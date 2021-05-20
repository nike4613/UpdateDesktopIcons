#pragma once

#include <chrono>
#include <future>
#include <optional>
#include <utility>
#include <filesystem>

#include <wil/com.h>

#include "com.h"
#include "config.h"
#include "explore.h"
#include "reparse.h"
#include "util.h"
#include "IVirtualDesktop.h"

namespace app
{
    config::configuration maybe_first_setup(std::filesystem::path const& desktop, std::filesystem::path const& configFolder);

    struct Application : com::object<Application, IVirtualDesktopNotification>
    {
        // TODO: implement config reloading
        Application(config::configuration const& configuration, std::filesystem::path const& desktopPath) noexcept
            : config{ configuration }, desktopPath{ std::filesystem::absolute(desktopPath) } {}
        Application(config::configuration&& configuration, std::filesystem::path const& desktopPath) noexcept
            : config{ std::move(configuration) }, desktopPath{ std::filesystem::absolute(desktopPath) } {}

        HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(IVirtualDesktop*) noexcept override;
        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(IVirtualDesktop* destroyed, IVirtualDesktop* fallback) noexcept override;
        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(IVirtualDesktop* destroyed, IVirtualDesktop* fallback) noexcept override;
        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(IVirtualDesktop* destroyed, IVirtualDesktop* fallback) noexcept override;
        HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(IUnknown* view) noexcept override;
        HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(IVirtualDesktop* pDesktopOld, IVirtualDesktop* pDesktopNew) noexcept override;

        void initialize();
        void shutdown();

    private:
        void reinitialize();

        void match_config_to_desktops(IObjectArray* vdesks);

        void changed_to_desktop(GUID const&);
        void do_update_desktop(GUID const&);
        void config_updated();

        config::config_store config;
        std::filesystem::path desktopPath;

        com::unique_notification_registration ownNotifReg;
        wil::com_ptr<IVirtualDesktopManagerInternal> vdeskManager;
        std::unique_ptr<explore::explorer_tracker> explorerTracker; // a unique pointer specifically so it isn't running until initialize/after shutdown

        std::shared_future<std::pair<GUID, std::chrono::system_clock::time_point>> updatePromise;
        std::future<void> updateAsync;
    };
}