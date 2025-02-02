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
    struct cell_ref_get_tag {};

    template<typename T>
    concept has_get_index = requires(T v)
    {
        v.get(std::declval<std::size_t>());
    };

    template<typename T>
    concept has_tagged_get = requires(T v)
    {
        v.get(std::declval<std::size_t>(), cell_ref_get_tag{});
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
        { return in(*owner); }

        decltype(auto) in(TOwner const& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>())))
            requires has_get_index<TOwner>
        { return owner.get(index); }

        decltype(auto) in(TOwner const& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>(), cell_ref_get_tag{})))
            requires has_tagged_get<TOwner>
        { return owner.get(index, cell_ref_get_tag{}); }

        decltype(auto) in(TOwner* owner) const noexcept(noexcept(in(*owner)))
        { return in(*owner); }

        decltype(auto) in(TOwner& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>())))
            requires has_get_index<TOwner>
        { return owner.get(index); }

        decltype(auto) in(TOwner& owner) const
            noexcept(noexcept(owner.get(std::declval<std::size_t>(), cell_ref_get_tag{})))
            requires has_tagged_get<TOwner>
        { return owner.get(index, cell_ref_get_tag{}); }

        friend bool operator==(cell_ref a, cell_ref b) noexcept
        { return a.index == b.index; }
        friend bool operator!=(cell_ref a, cell_ref b) noexcept
        { return a.index != b.index; }

    private:
        friend TOwner;
        cell_ref(std::size_t idx) noexcept : index{ idx } { }

        std::size_t index;
    };

    // This is effectively a unique_ptr that can copy its content
    template<typename T>
    struct copy_ptr
    {
        using element_type = T;
        using pointer = element_type*;

        copy_ptr() noexcept = default;
        copy_ptr(std::nullptr_t) noexcept : obj{ nullptr } {}
        copy_ptr(copy_ptr const& other) noexcept(noexcept(std::make_unique<T>(*other)))
            : obj{ std::make_unique<T>(*other) } {} // invoke T's copy ctor
        copy_ptr(copy_ptr&&) noexcept = default;

        copy_ptr(std::unique_ptr<T> const& obj) noexcept(noexcept(std::make_unique<T>(*obj)))
            : obj{ std::make_unique<T>(*obj) } {} // invoke T's copy ctor
        copy_ptr(std::unique_ptr<T>&& obj) noexcept
            : obj{ std::move(obj) } {}

        void reset(T* ptr = nullptr) noexcept(noexcept(obj.reset(ptr)))
        { obj.reset(ptr); }

        pointer get() const noexcept { return obj.get(); }

        std::add_lvalue_reference_t<T> operator*() const noexcept { return *obj; }
        pointer operator->() const noexcept { return obj.operator->(); }
    private:
        std::unique_ptr<T> obj;
    };

    struct relative_tag {};

    // although this stores only absolute paths, setting relative_to changes the real target
    // because it treats it as-if it stored a base and relative
    struct rel_path
    {
        rel_path() noexcept = default;
        rel_path(rel_path const&) noexcept = default;
        rel_path(rel_path&&) noexcept = default;
        rel_path(std::filesystem::path const& target, std::filesystem::path const& relativeTo = {}) noexcept
            : relTo{ relativeTo }
        {
            set_real_path(target);
        }
        rel_path(relative_tag, std::filesystem::path const& target, std::filesystem::path const& relativeTo = {}) noexcept
            : relTo{ relativeTo }
        {
            set_path(target);
        }

        rel_path& operator=(rel_path const&) noexcept = default;
        rel_path& operator=(rel_path&&) noexcept = default;

        std::filesystem::path path() const noexcept;
        std::filesystem::path real_path() const noexcept;

        void set_path(std::filesystem::path const&) noexcept;
        void set_real_path(std::filesystem::path const&) noexcept;

        std::filesystem::path relative_to() const noexcept;
        void relative_to(std::filesystem::path const&) noexcept;

    private:
        std::filesystem::path realTarget;
        std::filesystem::path relTo;
    };
}

template<>
struct fmt::formatter<GUID, char>
{
    bool useBraces = true;

    constexpr auto parse(format_parse_context& ctx)
    {
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == nullptr)
            return begin;

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
struct fmt::formatter<GUID, wchar_t>
{
    bool useBraces = true;

    constexpr auto parse(wformat_parse_context& ctx)
    {
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == nullptr)
            return begin;

        if (*begin == L'b')
        {
            useBraces = true;
            ++begin;
        }
        else if (*begin == L'e')
        {
            useBraces = false;
            ++begin;
        }

        if (*begin != L'}')
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
                FMT_COMPILE(L"{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}"),
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7])
            : format_to(
                ctx.out(),
                FMT_COMPILE(L"{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}"),
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    }
};

namespace util
{
    std::size_t hash_guid(GUID const& guid) noexcept;
}

template<>
struct std::hash<GUID>
{
    std::size_t operator()(GUID const& guid) const noexcept
    {
        return util::hash_guid(guid);
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

template<>
struct nlohmann::adl_serializer<util::rel_path>
{
    using json = nlohmann::json;
    static void to_json(json& j, util::rel_path const& p)
    {
        j = p.path().generic_string();
    }
    static void from_json(json const& j, util::rel_path& p)
    {
        p = util::rel_path{ j.get<std::filesystem::path>() };
    }
};