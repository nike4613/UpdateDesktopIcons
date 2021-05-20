#include "pch.h"
#include "app.h"
#include "priv.h"
#include "IVirtualDesktop.h"
#include "util.h"

#include <shlobj_core.h>

#include <fstream>

#include <nlohmann/json.hpp>

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopCreated(IVirtualDesktop* desktop) noexcept
{
    GUID guid;
    RETURN_IF_FAILED(desktop->GetID(&guid));

    fmt::print(FMT_STRING("Created {}\n"), guid);

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

HRESULT STDMETHODCALLTYPE app::Application::VirtualDesktopDestroyed(IVirtualDesktop* destroyed, IVirtualDesktop*) noexcept try
{
    GUID guid;
    RETURN_IF_FAILED(destroyed->GetID(&guid));

    fmt::print(FMT_STRING("Destroyed {}\n"), guid);

    bool modified = false;
    {
        auto const& [_lock, config] = this->config.lock(); // we write to the config

        if (auto opt = config->by_guid(guid); opt)
        {
            auto desk = opt->in(config);
            desk->removed = true; // flag the entry as being removed
            modified = true;
        }
    }

    if (modified)
    {
        config_updated();
    }

    return S_OK;
}
CATCH_RETURN();

HRESULT STDMETHODCALLTYPE app::Application::CurrentVirtualDesktopChanged(IVirtualDesktop*, IVirtualDesktop* pDesktopNew) noexcept try
{
    GUID guid;
    RETURN_IF_FAILED(pDesktopNew->GetID(&guid));

    changed_to_desktop(guid);

    return S_OK;
}
CATCH_RETURN();

namespace fs = std::filesystem;

void app::Application::initialize()
{
    reparse::set_needed_privilege();

    { // set our own priviledges
        priv::context ctx;
        ctx.enable(SE_BACKUP_NAME);
        ctx.enable(SE_RESTORE_NAME);
    }

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
    {
        auto const& [_lock, config] = this->config.lock(); // because we write to the config

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

namespace
{
    void save_reg_value(HKEY key, wchar_t const* valueName, fs::path const& file);
    void load_reg_value(HKEY key, wchar_t const* valueName, DWORD type, fs::path const& file);
}

void app::Application::do_update_desktop(GUID const& guid)
{
    fmt::print(FMT_STRING("Updating desktop {}\n"), guid);

    // figure out what target to set
    fs::path targetPath;
    fs::path configDir;
    {
        auto const& [_lock, config] = this->config.get(); // we only read from the config

        configDir = config->config_file.parent_path();

        if (auto opt = config->by_guid(guid); opt)
        {
            // we found a decl for it
            auto deskCfg = opt->in(config);

            targetPath = deskCfg->real_directory.real_path();
        }
        else
        {
            // we don't have a decl for it, use the default
            targetPath = config->default_dir.real_path();
        }
    }

    { // set junction target
        reparse::reparse_folder desktop{ this->desktopPath, /* readonly */ false };

        auto reparseTargetPath = fs::absolute(targetPath).native();
        desktop.set_junction_target(L"\\??\\" + reparseTargetPath, reparseTargetPath);
    }

    if (lastDesktop != GUID{} && lastDesktop != guid)
    { // try to save/restore desktop registry keys
        try
        {
            auto getFilen = [](GUID const& guid) { return fmt::format(FMT_STRING(L"{0:b}"), guid); };

            auto oldFile = configDir / getFilen(lastDesktop);
            auto newFile = configDir / getFilen(guid);

            // FIXME: this doesn't actually work, I think because the desktop is overwriting it
            wil::unique_hkey regKey;
            THROW_IF_WIN32_ERROR(RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\Shell\\Bags\\1\\Desktop", 0, KEY_READ | KEY_WRITE, &regKey));
            save_reg_value(regKey.get(), L"IconLayouts", oldFile);
            if (fs::exists(newFile))
            {
                load_reg_value(regKey.get(), L"IconLayouts", REG_BINARY, newFile);
            }
        }
        CATCH_LOG();
    }
    lastDesktop = guid;

    // perform a change notify
    SHChangeNotify(
        SHCNE_UPDATEDIR,
        SHCNF_PATH,
        fs::absolute(this->desktopPath).native().c_str(),
        nullptr
    );
}

namespace
{
    void save_reg_value(HKEY key, wchar_t const* valueName, fs::path const& file)
    {
        DWORD size;
        THROW_IF_WIN32_ERROR(RegQueryValueExW(key, valueName, nullptr, nullptr, nullptr, &size));
        auto data = std::make_unique<std::byte[]>(size);
        THROW_IF_WIN32_ERROR(RegQueryValueExW(key, valueName, nullptr, nullptr, (LPBYTE)data.get(), &size));

        std::ofstream ofile(file, std::ios::out | std::ios::binary);
        ofile.write(reinterpret_cast<char const*>(&data[0]), size);
        ofile.flush();
        ofile.close();
    }

    void load_reg_value(HKEY key, wchar_t const* valueName, DWORD type, fs::path const& file)
    {
        std::size_t size;
        std::unique_ptr<std::byte[]> data;
        {
            std::ifstream ifile(file, std::ios::in | std::ios::binary);
            size = ifile.seekg(0, std::ios::end).tellg();
            data = std::make_unique<std::byte[]>(size);
            ifile.seekg(0, std::ios::beg).read(reinterpret_cast<char*>(&data[0]), size);
        }

        THROW_IF_WIN32_ERROR(RegSetValueExW(key, valueName, 0, type, (LPBYTE)data.get(), size));
    }
}


void app::Application::config_updated()
{
    // TODO: pull this file writing somewhere else
    auto const& [_lock, config] = this->config.get();
    std::ofstream fconfig(config->config_file);
    fconfig << nlohmann::json(*config).dump(2);
    fconfig.flush();
    fconfig.close();
}
