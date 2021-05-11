#include "pch.h"
#include "reparse.h"
#include "priv.h"

#include <winioctl.h>

void reparse::set_needed_privilege()
{
    priv::context ctx;
    ctx.enable(SE_BACKUP_NAME);
    ctx.enable(SE_RESTORE_NAME);
}

reparse::reparse_folder::reparse_folder(std::wstring const& path, bool readonly)
{
    wil::unique_handle handle{
        CreateFileW(path.c_str(), 
            readonly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
            readonly ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0)
    };
    THROW_LAST_ERROR_IF_MSG(!handle.is_valid(), "Folder: '%ws'", path.c_str());
    file = std::move(handle);
}

reparse::reparse_folder::reparse_folder(wil::unique_handle&& handle) noexcept
    : file{std::move(handle)}
{
    // TODO: check this has correct access modes, if possible
}

reparse::reparse_folder::reparse_folder(wil::unique_handle const& handle)
{
    THROW_IF_WIN32_BOOL_FALSE(DuplicateHandle(
        GetCurrentProcess(), handle.get(),
        GetCurrentProcess(), &file,
        0 /* ignored because of options */, false, DUPLICATE_SAME_ACCESS));
}

reparse::reparse_folder::reparse_folder(reparse_folder&& other) noexcept
    : file{std::move(other.file)}
{
}

reparse::reparse_folder::reparse_folder(reparse_folder const& other)
    : reparse_folder(other.file)
{
}

DWORD reparse::reparse_folder::attributes()
{
    BY_HANDLE_FILE_INFORMATION info{};
    THROW_IF_WIN32_BOOL_FALSE(GetFileInformationByHandle(file.get(), &info));
    return info.dwFileAttributes;
}

bool reparse::reparse_folder::is_valid()
{
    return file.is_valid() && (attributes() & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

bool reparse::reparse_folder::is_junction()
{
    // since I only actually care about the tag for this, I can just throw the type on stack no problem
    REPARSE_GUID_DATA_BUFFER buffer;
    get_reparse_buffer(&buffer, sizeof(buffer), false);
    return buffer.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT;
}

DWORD reparse::reparse_folder::get_reparse_buffer(_Out_ REPARSE_GUID_DATA_BUFFER* buffer, DWORD bufferSize, bool throwIfTooSmall)
{
    DWORD read;
    if (throwIfTooSmall)
    {
        THROW_IF_WIN32_BOOL_FALSE(DeviceIoControl(file.get(), FSCTL_GET_REPARSE_POINT,
            nullptr, 0, buffer, bufferSize, &read, nullptr));
    }
    else
    {
        THROW_LAST_ERROR_IF(!DeviceIoControl(file.get(), FSCTL_GET_REPARSE_POINT,
            nullptr, 0, buffer, bufferSize, &read, nullptr) && GetLastError() != ERROR_MORE_DATA);
    }

    return read;
}
