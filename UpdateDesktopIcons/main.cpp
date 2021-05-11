#include "pch.h"
#include "priv.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;


bool FolderHasReparsePoint(wil::unique_handle const& fileHandle)
{
    BY_HANDLE_FILE_INFORMATION info{};
    THROW_IF_WIN32_BOOL_FALSE(GetFileInformationByHandle(fileHandle.get(), &info));
    return (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

bool FolderHasReparsePoint(winrt::hstring const& folderName)
{
    wil::unique_handle handle{
        CreateFile(folderName.c_str(), GENERIC_READ, FILE_SHARE_READ, 
            nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0)
    };
    THROW_LAST_ERROR_IF_MSG(!handle.is_valid(), "Folder: '%ws'", folderName.c_str());
    return FolderHasReparsePoint(handle);
}

void configure_backup_priv()
{
    priv::context ctx;
    ctx.enable(SE_BACKUP_NAME);
    ctx.enable(SE_RESTORE_NAME);
}

void do_main(winrt::hstring const& folderName)
{
    configure_backup_priv();

    auto result = FolderHasReparsePoint(folderName);
    if (result)
    {
        std::fputws(L"Folder is a reparse point.\r\n", stdout);
    }
    else
    {
        std::fputws(L"Folder is not a reparse point.\r\n", stdout);
    }
}

int wmain(int argc, wchar_t const* const* argv) try
{
    wil::SetResultLoggingCallback([](wil::FailureInfo const& failure) noexcept
    {
        constexpr std::size_t sizeOfLogMessageWithNul = 2048;

        wchar_t logMessage[sizeOfLogMessageWithNul];
        if (SUCCEEDED(wil::GetFailureLogString(logMessage, sizeOfLogMessageWithNul, failure)))
        {
            std::fputws(logMessage, stderr);
        }
    });

    winrt::init_apartment();

    std::span<wchar_t const* const> const args(argv, argc);

    if (args.size() != 2)
    {
        std::fputws(L"Please specify a folder path.\r\n", stderr);
        return -1;
    }

    do_main(args[1]);

    return 0;
}
CATCH_RETURN();
