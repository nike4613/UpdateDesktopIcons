#include "pch.h"
#include "reparse.h"
#include "priv.h"

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

reparse::reparse_folder::reparse_folder(wil::unique_handle&& handle)
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

reparse::reparse_folder::reparse_folder(reparse_folder&& other)
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
