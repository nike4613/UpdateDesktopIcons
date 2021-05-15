#pragma once
#include <filesystem>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>

#include <nlohmann/json.hpp>

#include <guiddef.h>

#include "util.h"

namespace config
{
    using json = nlohmann::json;

    struct desktop_configuration
    {
        int index;
        GUID guid;

        std::filesystem::path real_directory;

        void set_rel_base(std::filesystem::path const& relBase);

        friend void to_json(json&, desktop_configuration const&);
        friend void from_json(json const&, desktop_configuration&);
    private:

        std::filesystem::path relative_base; // this isn't serialized
    };


    struct configuration
    {
        using ref = util::cell_ref<configuration>;

        std::optional<ref> by_guid(GUID const& guid) const noexcept;
        std::optional<ref> by_index(int index) const noexcept;

        desktop_configuration* get(std::size_t index, util::cell_ref_get_t);
        desktop_configuration const* get(std::size_t index, util::cell_ref_get_t) const;
        decltype(auto) operator[](ref val) { return val.in(this); }
        decltype(auto) operator[](ref val) const { return val.in(this); }

        friend void to_json(json&, configuration const&);
        friend void from_json(json const&, configuration&);

    private:
        std::vector<std::unique_ptr<desktop_configuration>> storage;
        std::unordered_map<GUID, ref> byGuid;
        std::unordered_map<int, ref> byIndex;
    };

}
