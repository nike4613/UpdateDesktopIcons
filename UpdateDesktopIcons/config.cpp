#include "pch.h"
#include "config.h"

void config::to_json(json& j, desktop_configuration const& config)
{
    j = json{
        {"index", config.index},
        {"guid", config.guid},
        {"directory", config.real_directory.lexically_proximate(config.relative_base)}
    };
}

void config::from_json(json const& j, desktop_configuration& config)
{
    j.at("index").get_to(config.index);
    auto guid = j.find("guid");
    if (guid != j.end())
    {
        guid->get_to(config.guid);
    }
    j.at("directory").get_to(config.real_directory);
}

void config::desktop_configuration::set_rel_base(std::filesystem::path const& relBase)
{
    auto realdir = std::move(real_directory);
    auto oldRelbase = std::move(relative_base);

    if (!oldRelbase.empty())
    {
        realdir = realdir.lexically_proximate(oldRelbase);
    }

    relative_base = relBase;
    real_directory = relBase / realdir;
}
