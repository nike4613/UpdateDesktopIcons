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

namespace
{
    // requires null-terminated
    wil::unique_handle try_get_file_handle(std::wstring_view path, bool readonly)
    {
        wil::unique_handle handle {
            CreateFileW(path.data(), // we require null terminated input
                readonly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
                readonly ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0)
        };
        THROW_LAST_ERROR_IF_MSG(!handle.is_valid(), "Folder: '%ws'", path.data());
        return handle;
    }
}

reparse::reparse_folder::reparse_folder(std::wstring const& path, bool readonly)
{
    file = try_get_file_handle(path, readonly);
}

reparse::reparse_folder::reparse_folder(std::wstring_view path, bool readonly)
{
    std::wstring pathData;
    if (path.data()[path.size()] != 0)
    {
        pathData = path; // force a copy to null terminate it
    }
    file = try_get_file_handle(pathData.size() != 0 ? std::wstring_view{ pathData } : path, readonly);
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

struct REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            BYTE PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            BYTE PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
};

namespace
{
    auto f_get_junction_target(REPARSE_DATA_BUFFER const* data)
    {
        auto subsView = std::wstring_view{
            reinterpret_cast<wchar_t const*>(&data->MountPointReparseBuffer.PathBuffer[0] + data->MountPointReparseBuffer.SubstituteNameOffset),
            data->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t)
        };
        auto printView = std::wstring_view{
            reinterpret_cast<wchar_t const*>(&data->MountPointReparseBuffer.PathBuffer[0] + data->MountPointReparseBuffer.PrintNameOffset),
            data->MountPointReparseBuffer.PrintNameLength / sizeof(wchar_t)
        };
        return std::make_pair(subsView, printView);
    }
}

reparse::reparse_folder::junction_target reparse::reparse_folder::get_junction_target()
{
    auto dataBuffer = std::make_unique<BYTE[]>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    auto guidData = new(dataBuffer.get()) REPARSE_GUID_DATA_BUFFER();
    auto data = new(dataBuffer.get()) REPARSE_DATA_BUFFER();
    // i think this is technically UB, since now 2 objects have the same addr

    auto realSize = get_reparse_buffer(guidData, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, true);
    if (data->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
        throw std::runtime_error("Reparse point not a moint point");

    auto [subsView, printView] = f_get_junction_target(data);

    return {
        std::wstring{ subsView },
        std::wstring{ printView }
    };
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

namespace
{
    auto build_junction_buffer(std::wstring_view subsName, std::wstring_view printName)
    {
        auto const bufferHeaderSize = FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer);
        auto const dataHeaderSize = FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) - bufferHeaderSize;
        auto const subsSize = subsName.size() * sizeof(wchar_t);
        auto const printSize = printName.size() * sizeof(wchar_t);
        auto const dataSize = dataHeaderSize + subsSize + printSize + 4 /* null bytes */;
        auto const fullSize = dataSize + bufferHeaderSize;

        if (fullSize > MAXIMUM_REPARSE_DATA_BUFFER_SIZE)
        {
            throw std::runtime_error("Generated buffer too large");
        }

        auto bufferStorage = std::make_unique<BYTE[]>(fullSize);
        std::memset(bufferStorage.get(), 0, fullSize);
        auto dataPtr = new(bufferStorage.get()) REPARSE_DATA_BUFFER
        {
            IO_REPARSE_TAG_MOUNT_POINT,
            (USHORT)dataSize,
            0
        };
        dataPtr->MountPointReparseBuffer = {
            /* substitute name offset */ 0,
            /* substitute name size */  (USHORT)subsSize,
            /* print name offset */     (USHORT)(subsSize + 2), // +2 for the null char
            /* print name size */       (USHORT)printSize
        };
        // copy in substitute name data
        std::memcpy(&dataPtr->MountPointReparseBuffer.PathBuffer[0], subsName.data(), subsSize);
        // copy in print name data
        std::memcpy(&dataPtr->MountPointReparseBuffer.PathBuffer[subsSize + 2], printName.data(), printSize);

#if _DEBUG
        auto [subsView, printView] = f_get_junction_target(dataPtr);
        assert(subsView == subsName);
        assert(printView == printName);
#endif

        auto deleter = [storage = std::move(bufferStorage)](auto*) mutable
        {
            storage.reset(nullptr);
        };
        return std::make_pair(std::unique_ptr<REPARSE_DATA_BUFFER, decltype(deleter)>{ dataPtr, std::move(deleter) }, fullSize);
    }
}

void reparse::reparse_folder::set_junction_target(std::wstring_view substituteName, std::wstring_view printName)
{
    auto [junctionBuf_, size] = std::move(build_junction_buffer(substituteName, printName));
    auto junctionBuf = std::move(junctionBuf_);

    THROW_IF_WIN32_BOOL_FALSE(DeviceIoControl(file.get(), FSCTL_SET_REPARSE_POINT,
        junctionBuf.get(), size, nullptr, 0, nullptr, nullptr));
}