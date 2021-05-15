#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
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
    private:
        friend void to_json(json&, desktop_configuration const&);

        std::filesystem::path relative_base; // this isn't serialized
    };

    void to_json(json& j, desktop_configuration const& config);
    void from_json(json const& j, desktop_configuration& config);

    class configuration
    {

    };
}
