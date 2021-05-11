#include "pch.h"
#include "priv.h"
#include "reparse.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

void do_main(std::wstring const& folderName)
{
    reparse::set_needed_privilege();
    reparse::reparse_folder folder{ folderName };
    if (folder.is_valid())
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
