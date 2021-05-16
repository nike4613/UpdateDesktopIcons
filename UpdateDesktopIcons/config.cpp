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
    auto arr = json::array({});
    for (auto ref : config)
    {
        auto desk = ref.in(config);
        if (desk != nullptr)
        {
            arr.push_back(*desk);
        }
    }

    j = json{
        {"version", 1},
        {"defaultDir", config.default_dir},
        {"desktopMap", arr}
    };
}

void config::from_json(json const& j, configuration& config)
{
    if (j.at("version").get<int>() != 1)
        throw std::runtime_error("invalid version");
    j.at("defaultDir").get_to(config.default_dir);

    for (auto const& el : j.at("desktopMap"))
    {
        auto desk = std::make_unique<desktop_configuration>();
        el.get_to(*desk);
        config.storage.emplace_back(desk);
    }
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

void config::configuration::rebuild_maps() noexcept
{
    // TODO:
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

void config::configuration::remove(ref value) noexcept
{
    if (value.index >= storage.size())
        return;

    auto cfg = std::move(storage[value.index]); // clear it
    byGuid.erase(cfg->guid); // we assume that there can only be one entry with each guid

    auto idxIt = byIndex.find(cfg->index);
    if (idxIt != byIndex.end() && idxIt->second == value) // only actually erase if the ref is the same
    {
        byIndex.erase(idxIt);
    }
}

void config::configuration::changed(ref value) noexcept
{
    if (value.index >= storage.size())
        return;

    auto config = storage[value.index].get();
    if (config == nullptr)
        return;

    std::erase_if(byGuid, [=](auto const& p) { return p.second == value && config->guid != p.first; });
    std::erase_if(byIndex, [=](auto const& p) { return p.second == value && config->index != p.first; });
    byGuid.try_emplace(config->guid, value);
    byIndex.try_emplace(config->index, value);
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
