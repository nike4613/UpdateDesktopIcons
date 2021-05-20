#include "pch.h"
#include "app.h"
#include "util.h"
#include "config.h"

#include <fstream>
#include <shellapi.h>

namespace
{
    config::configuration get_default_config(std::filesystem::path const& configDir)
    {
        config::configuration config{};
        config.default_dir = { util::relative_tag{}, L"default", configDir };
        return config;
    }
}

config::configuration app::maybe_first_setup(std::filesystem::path const& desktop, std::filesystem::path const& configFolder)
{
    namespace fs = std::filesystem;

    constexpr std::wstring_view config_file_name = L"config.json";

    if (!fs::exists(desktop))
    {
        throw std::runtime_error("provided desktop folder doesn't exist; cannot continue");
    }

    bool doFirstSetup = false;
    if (!fs::exists(configFolder))
    {
        doFirstSetup = true;
        fs::create_directories(configFolder);
    }

    auto configPath = configFolder / config_file_name;
    if (!fs::exists(configPath))
    {
        doFirstSetup = true;
    }

    if (!doFirstSetup)
    {
        config::configuration config;
        try
        {
            std::ifstream fconfig(configPath);
            auto json = nlohmann::json::parse(fconfig, nullptr, false, true);
            fconfig.close();
            json.get_to(config);
            config.set_rel_base(configFolder);
            config.rebuild_maps();
        }
        catch (...)
        {
            LOG_CAUGHT_EXCEPTION();
            fmt::print(FMT_STRING("Exception while loading configuration; resetting to default"));
            config = get_default_config(configFolder);
            fs::create_directories(config.default_dir.real_path());
        }
        config.config_file = configPath;
        return config;
    }
    
    // if we're here, we do first setup

    auto config = get_default_config(configFolder);
    auto defaultDir = config.default_dir.real_path() / L"";
    fs::create_directories(defaultDir);

    // write our config to disk
    {
        std::ofstream fconfig(configPath);
        fconfig << nlohmann::json(config).dump(2);
        fconfig.flush();
        fconfig.close();
    }

    // move content of real desktop to default dir
    {
        std::wstring nullterm{ std::wstring_view{ L"\0", 1 } };
        auto fromStr = (desktop / L"").native() + L"*" + nullterm;
        auto toStr = defaultDir.native() + nullterm;
        SHFILEOPSTRUCTW opts{
            nullptr, // hwnd
            FO_MOVE, // wFunc
            fromStr.c_str(), // pFrom
            toStr.c_str(), // pTo
            FOF_RENAMEONCOLLISION // fFlags
        };
        auto result = SHFileOperationW(&opts);
        if (result != 0)
        {
            THROW_WIN32_MSG(result, fmt::format(FMT_STRING("SHFileOperationW returned {}"), result).c_str());
        }
        if (opts.fAnyOperationsAborted)
        {
            throw std::runtime_error("File move aborted during first time setup");
        }
    }

    config.config_file = configPath;
    return config;
}
