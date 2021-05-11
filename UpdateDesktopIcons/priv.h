#pragma once
#include "pch.h"

namespace priv
{
    class context
    {
        wil::unique_handle procHandle;
    public:
        context();

        void set_attr(wchar_t const* privName, DWORD attr);
        void enable(wchar_t const* privName) { set_attr(privName, SE_PRIVILEGE_ENABLED); }
        void disable(wchar_t const* privName) { set_attr(privName, 0); }
    };
}
