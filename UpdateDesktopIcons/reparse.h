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
        reparse_folder(wil::unique_handle&& handle) noexcept;
        reparse_folder(wil::unique_handle const& handle);
        reparse_folder(reparse_folder&& other) noexcept;
        reparse_folder(reparse_folder const& other);

        DWORD attributes();
        bool is_valid();

        bool is_junction();

        void set_junction_target(std::wstring_view subsName, std::wstring_view printName);
        struct junction_target {
            std::wstring substitute_name;
            std::wstring print_name;
        };
        junction_target get_junction_target();
    private:
        DWORD get_reparse_buffer(_Out_ REPARSE_GUID_DATA_BUFFER* buffer, DWORD bufferSize, bool throwIfTooSmall);
    };
}