#pragma once
#include <charconv>
#include <algorithm>
#include <string_view>
#include <concepts>
#include <filesystem>

#include <fmt/format.h>
#include <fmt/compile.h>
#include <nlohmann/json.hpp>

#include <guiddef.h>

namespace util
{
    struct cell_ref_get_t {};

    template<typename T>
    concept has_get_index = requires(T v)
    {
        v.get(std::declval<std::size_t>());
    };

    template<typename T>
    concept has_tagged_get = requires(T v)
    {
        v.get(std::declval<std::size_t>(), cell_ref_get_t{});
    };

    template<typename T>
    concept cell_ref_owner = has_get_index<T> || has_tagged_get<T>;

    template<typename TOwner>
    //requires cell_ref_owner<TOwner>
    struct cell_ref
    {
        cell_ref() noexcept = default;
        cell_ref(cell_ref const&) noexcept = default;
        cell_ref(cell_ref&&) noexcept = default;

        decltype(auto) in(TOwner const* owner) const noexcept(noexcept(in(*owner)))
        {
            return in(*owner);
        }

        decltype(auto) in(TOwner const& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>())))
            requires has_get_index<TOwner>
        {
            return owner.get(index);
        }

        decltype(auto) in(TOwner const& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>(), cell_ref_get_t{})))
            requires has_tagged_get<TOwner>
        {
            return owner.get(index, cell_ref_get_t{});
        }

        decltype(auto) in(TOwner* owner) const noexcept(noexcept(in(*owner)))
        {
            return in(*owner);
        }

        decltype(auto) in(TOwner& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>())))
            requires has_get_index<TOwner>
        {
            return owner.get(index);
        }

        decltype(auto) in(TOwner& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>(), cell_ref_get_t{})))
            requires has_tagged_get<TOwner>
        {
            return owner.get(index, cell_ref_get_t{});
        }


    private:
        friend TOwner;
        cell_ref(std::size_t idx) noexcept : index{ idx } { }

        std::size_t index;
    };
}

template<>
struct fmt::formatter<GUID>
{
    bool useBraces = true;

    constexpr auto parse(format_parse_context& ctx)
    {
        auto begin = ctx.begin(), end = ctx.end();
        if (*begin == 'b')
        {
            useBraces = true;
            ++begin;
        }
        else if (*begin == 'e')
        {
            useBraces = false;
            ++begin;
        }

        if (*begin != '}')
            throw fmt::format_error("invalid format string");

        return begin;
    }

    template <typename FormatContext>
    constexpr auto format(GUID const& guid, FormatContext& ctx)
    {
        // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
          // ctx.out() is an output iterator to write to. 
        return 
            useBraces
                ? format_to(
                    ctx.out(),
                    FMT_COMPILE("{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}"),
                    guid.Data1, guid.Data2, guid.Data3,
                    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7])
                : format_to(
                    ctx.out(),
                    FMT_COMPILE("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}"),
                    guid.Data1, guid.Data2, guid.Data3,
                    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    }
};

template<>
struct nlohmann::adl_serializer<GUID>
{
    using json = nlohmann::json;
    static void to_json(json& j, GUID const& guid)
    {
        // just use fmt to give me something sane
        j = fmt::format(FMT_STRING("{0:e}"), guid);
    }
    static void from_json(json const& j, GUID& guid)
    {
        guid = {}; // zero it

        auto const text = j.get<std::string>();
        if (text.size() == 0) return; // nothing to do if we have no input

        auto begin = &*text.begin();
        auto end = begin + text.size();

        // ignore error codes because we want to be really lenient here; zeroing anything not present
        auto dash = std::find(begin, end, '-');
        std::from_chars(begin, dash, guid.Data1, 16);
        begin = std::min(++dash, end);
        dash = std::find(std::min(dash, end), end, '-');
        std::from_chars(begin, dash, guid.Data2, 16);
        begin = std::min(++dash, end);
        dash = std::find(std::min(dash, end), end, '-');
        std::from_chars(begin, dash, guid.Data3, 16);

        begin = std::min(++dash, end);
        dash = std::find(std::min(dash, end), end, '-');
        // here we need to treat it as 2 groups of 2
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[0], 16).ptr; // after this, begin is either at the start of the next group, or the dash/end
        begin = std::from_chars(begin, dash, guid.Data4[1], 16).ptr; // then we continue reading the last group

        begin = std::min(++dash, end);
        // now we do that again, except 6 times
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[2], 16).ptr;
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[3], 16).ptr;
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[4], 16).ptr;
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[5], 16).ptr;
        begin = std::from_chars(begin, std::min(begin + 2, end), guid.Data4[6], 16).ptr;
        begin = std::from_chars(begin, end, guid.Data4[7], 16).ptr; // since this is the last one, we read to the end of the string
    }
};

template<>
struct nlohmann::adl_serializer<std::filesystem::path>
{
    using json = nlohmann::json;
    using path = std::filesystem::path;
    static void to_json(json& j, path const& p)
    {
        j = p.generic_string();
    }
    static void from_json(json const& j, path& p)
    {
        p = path{ j.get<std::string>() };
    }
};