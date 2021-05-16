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

        void remove(ref value) noexcept;
        void changed(ref value) noexcept;

        desktop_configuration* get(std::size_t index, util::cell_ref_get_t);
        desktop_configuration const* get(std::size_t index, util::cell_ref_get_t) const;
        decltype(auto) operator[](ref val) { return val.in(this); }
        decltype(auto) operator[](ref val) const { return val.in(this); }

        friend void to_json(json&, configuration const&);
        friend void from_json(json const&, configuration&);

        struct iterator;
        using const_iterator = iterator const;
        iterator begin() noexcept { return iterator{ 0 }; }
        iterator end() noexcept { return iterator{ storage.size() }; }
        const_iterator begin() const noexcept { return iterator{ 0 }; }
        const_iterator end() const noexcept { return iterator{ storage.size() }; }

    private:
        std::vector<util::copy_ptr<desktop_configuration>> storage;
        std::unordered_map<GUID, ref> byGuid;
        std::unordered_map<int, ref> byIndex;

    public:
        struct iterator
        {
            iterator() noexcept = default;
            iterator(iterator const&) noexcept = default;
            iterator(iterator&&) noexcept = default;

            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = ref;
            using reference = ref;
            using difference_type = std::ptrdiff_t;

            ref operator*() const noexcept { return { index }; }
            iterator& operator++() noexcept { ++index; return *this; }
            iterator operator++(int) noexcept { return iterator{ index++ }; }
            iterator& operator--() noexcept { --index; return *this; }
            iterator operator--(int) noexcept { return iterator{ index-- }; }

            /*
            friend difference_type operator-(iterator const& a, iterator const& b) noexcept
            {
                return a.index - b.index;
            }
            friend iterator operator-(iterator const& a, difference_type dist) noexcept
            {
                return iterator{ a.index - dist };
            }
            iterator& operator-=(difference_type dist) noexcept
            {
                index -= dist;
                return *this;
            }
            friend iterator operator+(iterator const& a, difference_type dist) noexcept
            {
                return iterator{ a.index + dist };
            }
            friend iterator operator+(difference_type dist, iterator const& a) noexcept
            {
                return iterator{ a.index + dist };
            }
            iterator& operator-=(difference_type dist) noexcept
            {
                index += dist;
                return *this;
            }

            reference operator[](difference_type dist) noexcept
            {
                return { index + dist };
            }
            */

            friend std::strong_ordering operator<=>(iterator const& a, iterator const& b) noexcept
            {
                return a.index <=> b.index;
            }
        private:
            friend struct configuration;
            explicit iterator(std::size_t idx) noexcept : index{ idx } {}
            std::size_t index;
        };
        //static_assert(std::bidirectional_iterator<iterator>, "fix yer iterator idiot");
    };

}
