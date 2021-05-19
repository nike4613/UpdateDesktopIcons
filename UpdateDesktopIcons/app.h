#pragma once

#include <wil/com.h>

#include "com.h"
#include "config.h"
#include "explore.h"
#include "reparse.h"
#include "util.h"
#include "IVirtualDesktop.h"

namespace app
{
    struct Application : com::object<Application, IVirtualDesktopNotification>
    {
        // TODO: implement config reloading
        Application(config::configuration const& configuration) noexcept
            : config{ configuration } {}
        Application(config::configuration&& configuration) noexcept
            : config{ std::move(configuration) } {}

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

        com::unique_notification_registration ownNotifReg;
        wil::com_ptr<IVirtualDesktopManagerInternal> vdeskManager;
        //explore::explorer_tracker explorerTracker{ [this] { try { this->reinitialize(); } CATCH_LOG(); } };
        std::unique_ptr<explore::explorer_tracker> explorerTracker; // a unique pointer specifically so it isn't running until initialize/after shutdown
    };
}