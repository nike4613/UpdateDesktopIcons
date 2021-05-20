#pragma once
#include <wil/resource.h>
#include <string>
#include <string_view>
#include <filesystem>

namespace reparse
{
    void set_needed_privilege();

    class reparse_folder
    {
        wil::unique_handle file;
    public:
        explicit reparse_folder(std::wstring const& path, bool readonly = true);
        explicit reparse_folder(std::wstring_view path, bool readonly = true);
        explicit reparse_folder(std::filesystem::path const& path, bool readonly = true);
        reparse_folder(wil::unique_handle&& handle) noexcept;
        reparse_folder(wil::unique_handle const& handle);
        reparse_folder(reparse_folder&& other) noexcept;
        reparse_folder(reparse_folder const& other);

        unsigned long attributes();
        std::wstring full_path();
        bool is_valid();

        bool is_junction();

        struct junction_target {
            std::wstring substitute_name;
            std::wstring print_name;
        };
        junction_target get_junction_target();
        void set_junction_target(std::wstring_view substituteName, std::wstring_view printName);
    private:
        using DWORD = unsigned long; // makes VS happy to find the def
        DWORD get_reparse_buffer(_Out_ REPARSE_GUID_DATA_BUFFER* buffer, DWORD bufferSize, bool throwIfTooSmall);
    };
}