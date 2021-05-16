#include "pch.h"
#include "util.h"

std::size_t util::hash_guid(GUID const& guid) noexcept
{
    // FIXME: this is retard
    auto bits = std::bit_cast<std::bitset<sizeof(GUID)* CHAR_BIT>>(guid);
    return std::hash<decltype(bits)>{}(bits);
}
