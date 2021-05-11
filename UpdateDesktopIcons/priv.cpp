#include "pch.h"
#include "priv.h"

priv::context::context() : procHandle{}
{
    THROW_IF_WIN32_BOOL_FALSE(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &procHandle));
}

void priv::context::set_attr(wchar_t const* privName, DWORD attr)
{
    TOKEN_PRIVILEGES priv{ 1, {} };
    THROW_IF_WIN32_BOOL_FALSE(LookupPrivilegeValue(nullptr, privName, &priv.Privileges[0].Luid));
    priv.Privileges[0].Attributes = attr;
    THROW_IF_WIN32_BOOL_FALSE(AdjustTokenPrivileges(procHandle.get(), false, &priv, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr));
}
