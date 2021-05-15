#pragma once

#include <WinUser.h>

#include <wil/resource.h>
#include <functional>
#include <thread>
#include <mutex>

namespace explore
{
    namespace detail
    {
        void UnregisterWindowClassFunc(ATOM klass);
    }

    using unique_window_class_atom = wil::unique_any<ATOM, decltype(&detail::UnregisterWindowClassFunc), &detail::UnregisterWindowClassFunc> ;

    struct explorer_tracker
    {
        explorer_tracker();
        explorer_tracker(std::function<void()> handler);
        ~explorer_tracker();

        void set_restart_handler(std::function<void()> handler);
        void start_tracking();
    private:
        static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT inst_wnd_proc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        unique_window_class_atom winClass;
        std::function<void()> handler;
        std::mutex handlerMut;
        std::jthread msgLoopThread;
        HWND window;
        UINT taskbarCreatedMessage;
    };
}