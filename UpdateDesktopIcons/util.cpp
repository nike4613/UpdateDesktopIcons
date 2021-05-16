#include "pch.h"
#include "util.h"

std::size_t util::hash_guid(GUID const& guid) noexcept
{
    // FIXME: this is retard
    auto bits = std::bit_cast<std::bitset<sizeof(GUID)* CHAR_BIT>>(guid);
    return std::hash<decltype(bits)>{}(bits);
}

std::filesystem::path util::rel_path::path() const noexcept
{
    return realTarget.lexically_proximate(relTo);
}

std::filesystem::path util::rel_path::real_path() const noexcept
{
    return realTarget;
}

void util::rel_path::set_path(std::filesystem::path const& newPath) noexcept
{
    realTarget = relTo / newPath;
}

void util::rel_path::set_real_path(std::filesystem::path const& newReal) noexcept
{
    realTarget = newReal;
}

std::filesystem::path util::rel_path::relative_to() const noexcept
{
    return relTo;
}

void util::rel_path::relative_to(std::filesystem::path const& newRel) noexcept
{
    auto origRel = path();
    relTo = newRel;
    realTarget = relTo / origRel;
}
