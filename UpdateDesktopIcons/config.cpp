#include "pch.h"
#include <algorithm>
#include "config.h"

using namespace config;

void config::to_json(json& j, desktop_configuration const& config)
{
    j = json{
        {"index", config.index},
        {"guid", config.guid},
        {"path", config.real_directory}
    };
}

void config::from_json(json const& j, desktop_configuration& config)
{
    auto index = j.find("index");
    if (index != j.end())
    {
        index->get_to(config.index);
    }
    auto guid = j.find("guid");
    if (guid != j.end())
    {
        guid->get_to(config.guid);
    }
    j.at("path").get_to(config.real_directory);
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
        {"desktops", arr}
    };
}

void config::from_json(json const& j, configuration& config)
{
    if (j.at("version").get<int>() != 1)
        throw std::runtime_error("invalid version");
    j.at("defaultDir").get_to(config.default_dir);

    for (auto const& el : j.at("desktops"))
    {
        auto desk = std::make_unique<desktop_configuration>();
        el.get_to(*desk);
        config.storage.emplace_back(desk);
    }
}

void config::desktop_configuration::set_rel_base(std::filesystem::path const& relBase) noexcept
{
    real_directory.relative_to(relBase);
}

void config::configuration::set_rel_base(std::filesystem::path const& relBase) noexcept
{
    default_dir.relative_to(relBase);

    for (auto ref : *this)
    {
        ref.in(this)->set_rel_base(relBase);
    }
}

void config::configuration::rebuild_maps() noexcept
{
    byGuid.clear();
    byIndex.clear();

    for (auto ref : *this)
    {
        auto desk = ref.in(this);
        byGuid.try_emplace(desk->guid, ref);
        byIndex.try_emplace(desk->index, ref);
    }
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

desktop_configuration* config::configuration::get(std::size_t index, util::cell_ref_get_tag)
{
    return const_cast<desktop_configuration*>(const_cast<configuration const*>(this)->get(index, util::cell_ref_get_tag{}));
}

desktop_configuration const* config::configuration::get(std::size_t index, util::cell_ref_get_tag) const
{
    if (index < storage.size())
    {
        return storage[index].get();
    }

    return nullptr;
}

configuration const& config::config_store::get() const
{
    return config;
}

std::pair<std::unique_lock<std::mutex>, configuration*> config::config_store::lock()
{
    return std::pair(std::unique_lock(mutex), &config);
}
