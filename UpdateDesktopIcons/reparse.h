#pragma once
#include "pch.h"

namespace reparse
{
    void set_needed_privilege();

    class reparse_folder
    {
        wil::unique_handle file;
    public:
        explicit reparse_folder(std::wstring const& path, bool readonly = true);
        reparse_folder(wil::unique_handle&& handle);
        reparse_folder(wil::unique_handle const& handle);
        reparse_folder(reparse_folder&& other);
        reparse_folder(reparse_folder const& other);

        DWORD attributes();
        bool is_valid();
    };
}