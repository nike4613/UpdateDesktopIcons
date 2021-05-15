#include "pch.h"
#include "explore.h"

explore::explorer_tracker::explorer_tracker()
{
    WNDCLASS wc{};
    wc.lpfnWndProc = &wnd_proc;
    wc.hInstance = THROW_LAST_ERROR_IF_NULL(GetModuleHandleW(nullptr));
    wc.lpszClassName = L"Explorer.exe restart tracker";

    winClass = explore::unique_window_class_atom{ RegisterClassW(&wc) };
    THROW_LAST_ERROR_IF_MSG(!winClass.is_valid(), "Failed to register window class");

    taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");
    THROW_LAST_ERROR_IF_MSG(taskbarCreatedMessage == 0, "Could not register TaskbarCreated message");

    std::promise<HWND> hwndPromise;
    auto future = hwndPromise.get_future();

    msgLoopThread = std::jthread([](std::stop_token stoptok, ATOM klass, std::promise<HWND> promise, explore::explorer_tracker* self)
        {
            try
            {
                auto window = wil::unique_hwnd{
                    THROW_LAST_ERROR_IF_NULL(
                        CreateWindowExW(
                            WS_EX_TOOLWINDOW,
                            (LPCWSTR)klass,
                            L"Explorer restart tracker",
                            WS_BORDER,
                            0, 0, 0, 0,
                            0, // no parent
                            nullptr,
                            THROW_LAST_ERROR_IF_NULL(GetModuleHandleW(nullptr)),
                            self))
                };
                THROW_LAST_ERROR_IF_MSG(!window.is_valid(), "Failed to create window");

                auto hw = window.get();

                promise.set_value(hw);

                while (!stoptok.stop_requested())
                {
                    MSG msg;
                    if (PeekMessageW(&msg, hw, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessageW(&msg);
                    }

                    if (!SwitchToThread()) // yield exceution
                    {
                        // if there are no other threads ready to run, sleep a bit
                        Sleep(1);
                    }
                }
            }
            catch (...)
            {
                promise.set_exception(std::current_exception());
            }
        }, winClass.get(), std::move(hwndPromise), this);

    window = future.get();
}

explore::explorer_tracker::explorer_tracker(std::function<void()> handle)
    : explorer_tracker{}
{
    handler = std::move(handle);
}

explore::explorer_tracker::~explorer_tracker()
{
    msgLoopThread.request_stop();
    msgLoopThread.join();
    winClass.reset();
}

void explore::explorer_tracker::set_restart_handler(std::function<void()> handle)
{
    std::scoped_lock _{ handlerMut };
    handler = std::move(handle);
}

void explore::explorer_tracker::start_tracking()
{
    ShowWindow(window, SW_SHOW);
}

LRESULT CALLBACK explore::explorer_tracker::wnd_proc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam) try
{
    explore::explorer_tracker* obj;
    switch (uMsg)
    {
    case WM_GETMINMAXINFO:
    {
        // this is invoked *before* NCCREATE and CREATE for some reason
        auto mmInfo = (MINMAXINFO*)lParam;
        *mmInfo = {};
        return 0;
    }
    case WM_NCCREATE:
    case WM_CREATE: // idk why but GetWindowLongPtrW fails during WM_CREATE
        obj = reinterpret_cast<explore::explorer_tracker*>(
            reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams
            );
        SetLastError(0);
        SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)obj);
        RETURN_LAST_ERROR_IF_MSG(GetLastError() != 0, "Could not set user data on window");
        break;
    default:
        obj = reinterpret_cast<explore::explorer_tracker*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        RETURN_LAST_ERROR_IF_MSG(GetLastError() != 0 && obj == nullptr, "Could not get explorer tracker instance from window (msg %X)", uMsg);
        break;
    }

    if (obj == nullptr) return 0; // we don't have an object because we haven't configured that just yet

    // obj always contains our real object
    return obj->inst_wnd_proc(uMsg, wParam, lParam);
}
CATCH_RETURN(); // returns nonzero on failure

LRESULT explore::explorer_tracker::inst_wnd_proc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == taskbarCreatedMessage)
    {
        decltype(handler) handle;
        {
            std::scoped_lock lock{ handlerMut };
            handle = handler;
        }

        if (handle)
        {
            handle();
        }
    }

    switch (uMsg)
    {
    case WM_NCCREATE:
        return TRUE; // need this to continue
    case WM_CREATE:
        return 0; // need this to continue

    case WM_NCCALCSIZE:
        if ((BOOL)wParam)
        {
            auto params = (NCCALCSIZE_PARAMS*)lParam;
            params->rgrc[0] = {};
            params->rgrc[1] = {};
            params->rgrc[2] = {};
            params->lppos->x = 0;
            params->lppos->y = 0;
            params->lppos->cx = 0;
            params->lppos->cy = 0;
            params->lppos->flags = SWP_HIDEWINDOW;

            return 0;
        }
        else
        {
            auto rect = (RECT*)lParam;
            *rect = {};
            
            return 0;
        }
    }

    return 0;
}

void explore::detail::UnregisterWindowClassFunc(ATOM klass)
{
    THROW_IF_WIN32_BOOL_FALSE(UnregisterClassW((LPCWSTR)klass, GetModuleHandleW(nullptr)));
}
