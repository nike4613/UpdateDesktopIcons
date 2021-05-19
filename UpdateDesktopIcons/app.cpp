#include "pch.h"
#include "app.h"
#include "IVirtualDesktop.h"

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopCreated(IVirtualDesktop*) noexcept
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopDestroyBegin(IVirtualDesktop*, IVirtualDesktop*) noexcept
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopDestroyFailed(IVirtualDesktop*, IVirtualDesktop*) noexcept
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE app::Application::ViewVirtualDesktopChanged(IUnknown*) noexcept
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopDestroyed(IVirtualDesktop* destroyed, IVirtualDesktop*) noexcept
{
    // TODO: do something with the config

    return S_OK;
}

HRESULT STDMETHODCALLTYPE app::Application::CurrentVirtualDesktopChanged(IVirtualDesktop*, IVirtualDesktop* pDesktopNew) noexcept try
{
    GUID guid;
    THROW_IF_FAILED(pDesktopNew->GetID(&guid));

    changed_to_desktop(guid);

    return S_OK;
}
CATCH_RETURN();

void app::Application::initialize()
{
    auto shell = wil::CoCreateInstance<CImmersiveShell, IServiceProvider>(CLSCTX_LOCAL_SERVER); // local server because its hosted in explorer.exe
    THROW_IF_FAILED(shell->QueryService<IVirtualDesktopManagerInternal>(CLSID_CVirtualDesktopManagerInternal, &vdeskManager));

    wil::com_ptr<IVirtualDesktopNotificationService> vdnotifService;
    THROW_IF_FAILED(shell->QueryService<IVirtualDesktopNotificationService>(CLSID_CVirtualDesktopNotificationService, &vdnotifService));

    ownNotifReg = com::register_virtual_desktop_notification(vdnotifService, this);

    explorerTracker = std::make_unique<explore::explorer_tracker>([this] { try { this->reinitialize(); } CATCH_LOG(); });

    wil::com_ptr<IVirtualDesktop> current;
    THROW_IF_FAILED(vdeskManager->GetCurrentDesktop(&current));
    GUID guid;
    THROW_IF_FAILED(current->GetID(&guid));

    do_update_desktop(guid);
}

void app::Application::shutdown()
{
    explorerTracker.reset();
    ownNotifReg.reset();
    vdeskManager.reset();
}

void app::Application::reinitialize()
{
    // our reinit code is identical to our init code
    initialize();
}

void app::Application::changed_to_desktop(GUID const&)
{
    // TODO: de-bounce, then call do_update_desktop
}

void app::Application::do_update_desktop(GUID const& guid)
{
    // TODO: change desktop target
}

void app::Application::config_updated()
{
    // TODO: save updated config
}
