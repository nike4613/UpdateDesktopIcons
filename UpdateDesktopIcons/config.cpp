#include "pch.h"
#include <algorithm>
#include "config.h"

using namespace config;

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

void config::to_json(json& j, configuration const& config)
{
    // TODO:
}

void config::from_json(json const& j, configuration& config)
{
    // TODO:
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

auto config::configuration::by_guid(GUID const& guid) const noexcept -> std::optional<ref>
{
    auto result = byGuid.find(guid);
    if (result != byGuid.end())
    {
        return result->second;
    }
    else
    {
        return std::nullopt;
    }
}

auto config::configuration::by_index(int index) const noexcept -> std::optional<ref>
{
    auto result = byIndex.find(index);
    if (result != byIndex.end())
    {
        return result->second;
    }
    else
    {
        return std::nullopt;
    }
}

desktop_configuration* config::configuration::get(std::size_t index, util::cell_ref_get_t)
{
    return const_cast<desktop_configuration*>(const_cast<configuration const*>(this)->get(index, util::cell_ref_get_t{}));
}

desktop_configuration const* config::configuration::get(std::size_t index, util::cell_ref_get_t) const
{
    if (index < storage.size())
    {
        return storage[index].get();
    }

    return nullptr;
}
