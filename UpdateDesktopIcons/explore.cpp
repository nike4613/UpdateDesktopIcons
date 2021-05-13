#include "pch.h"
#include "explore.h"

explore::explorer_tracker::explorer_tracker()
{
    WNDCLASS wc{};
    wc.lpfnWndProc = &wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"Explorer.exe restart tracker";

    winClass = explore::unique_window_class_atom{ RegisterClassW(&wc) };
    THROW_LAST_ERROR_IF_MSG(!winClass.is_valid(), "Failed to register window class");

    window = wil::unique_hwnd{
        CreateWindowExW(
            0,
            (LPCWSTR)winClass.get(),
            L"Explorer restart tracker",
            WS_BORDER,
            0, 0, 0, 0,
            HWND_MESSAGE,
            nullptr,
            GetModuleHandleW(nullptr),
            this)
    };
    THROW_LAST_ERROR_IF_MSG(!window.is_valid(), "Failed to create window");

    taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");
    THROW_LAST_ERROR_IF_MSG(taskbarCreatedMessage == 0, "Could not register TaskbarCreated message");
}

explore::explorer_tracker::~explorer_tracker()
{
    // the order of destruction here is *really* important
    window.reset();
    winClass.reset();
}

void explore::explorer_tracker::set_restart_handler(std::function<void()> handler)
{
    this->handler = handler;
}

void explore::explorer_tracker::start_tracking()
{
    ShowWindow(window.get(), SW_SHOW);
}

LRESULT CALLBACK explore::explorer_tracker::wnd_proc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam) try
{
    explore::explorer_tracker* obj;
    switch (uMsg)
    {
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
        RETURN_LAST_ERROR_IF_MSG(GetLastError() != 0, "Could not get explorer tracker instance from window");
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
    { // FIXME: why don't I recieve this message?
        if (handler)
        {
            handler();
        }
    }

    switch (uMsg)
    {
    case WM_NCCREATE:
        return TRUE; // need this to continue
    case WM_CREATE:
        return 0; // need this to continue
    }

    return 0;
}

void explore::detail::UnregisterWindowClassFunc(ATOM klass)
{
    THROW_IF_WIN32_BOOL_FALSE(UnregisterClassW((LPCWSTR)klass, GetModuleHandleW(nullptr)));
}
