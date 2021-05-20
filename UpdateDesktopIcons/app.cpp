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

    if (!explorerTracker)
    { // don't recreate the tracker on reinitialize
        explorerTracker = std::make_unique<explore::explorer_tracker>([this] { try { this->reinitialize(); } CATCH_LOG(); });
    }

    wil::com_ptr<IObjectArray> desktops;
    THROW_IF_FAILED(vdeskManager->GetDesktops(&desktops));
    match_config_to_desktops(desktops.get());

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

void app::Application::match_config_to_desktops(IObjectArray* vdesks)
{
    auto const& [_lock, config] = this->config.lock();

    for (auto ref : *config)
    {
        ref.in(config)->removed = true;
    }

    UINT count;
    THROW_IF_FAILED(vdesks->GetCount(&count));
    for (UINT i = 0; i < count; i++)
    {
        wil::com_ptr<IVirtualDesktop> desk;
        THROW_IF_FAILED(vdesks->GetAt(i, IID_IVirtualDesktop, desk.put_void()));

        GUID guid;
        THROW_IF_FAILED(desk->GetID(&guid));

        if (auto dconf = config->by_guid(guid); dconf)
        {
            auto conf = dconf->in(config);
            conf->index = i;
            conf->removed = false;
            config->changed(*dconf);
        }
        else if (auto dconf = config->by_index(i); dconf && dconf->in(config)->guid == GUID{})
        { // only set guid if it hasn't already been set
            auto conf = dconf->in(config);
            conf->guid = guid;
            conf->removed = false;
            config->changed(*dconf);
        }
        else
        {
            // this desktop isn't mapped
            continue;
        }
    }

    config_updated();
}

void app::Application::changed_to_desktop(GUID const& guid)
{
    using namespace std::chrono_literals;

    fmt::print(FMT_STRING("Desktop changed to {}\n"), guid);

    // FIXME: I am almost certain there is a better way to do this

    std::promise<std::pair<GUID, std::chrono::system_clock::time_point>> promise; // reset our promise
    promise.set_value({ guid, std::chrono::system_clock::now() + 500ms });
    updatePromise = promise.get_future().share();

    if (!updateAsync.valid() || updateAsync.wait_for(0ms) == std::future_status::ready)
    {
        updateAsync = std::async(std::launch::async,
        [this]()
        {
            try
            {
                GUID guid{};
                GUID guid2{};
                auto future = updatePromise;
                auto result = future.get();
                do
                {
                    std::this_thread::sleep_until(result.second);
                    future = updatePromise;
                    result = future.get();
                    guid2 = guid;
                    guid = result.first;
                } while (guid != guid2);

                // we know konw our guid, and have debounced
                do_update_desktop(guid);
            }
            CATCH_LOG();
        });
    }
}

void app::Application::do_update_desktop(GUID const& guid)
{
    fmt::print(FMT_STRING("Updating desktop {}\n"), guid);
    // TODO: change desktop target
}

void app::Application::config_updated()
{
    // TODO: save updated config
}
