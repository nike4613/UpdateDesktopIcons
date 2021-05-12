#include "pch.h"
#include "priv.h"
#include "reparse.h"
#include "IVirtualDesktop.h"

void do_read(std::wstring_view folderName)
{
    reparse::set_needed_privilege();
    reparse::reparse_folder folder{ folderName };
    if (!folder.is_valid())
    {
        std::fputws(L"Folder is not a reparse point.\r\n", stdout);
    }
    else
    {
        std::fputws(L"Folder is a reparse point.\r\n", stdout);
        if (folder.is_junction())
        {
            std::fputws(L"Folder is a mount point/junction.\r\n", stdout);
            auto const& [subs, print] = folder.get_junction_target();
            std::fputws(L"SubstituteName: ", stdout);
            std::fputws(subs.c_str(), stdout);
            std::fputws(L"\r\nPrintName: ", stdout);
            std::fputws(print.c_str(), stdout);
            std::fputws(L"\r\n", stdout);
        }
    }
}

void do_write(std::wstring_view folderName, std::wstring_view newPath)
{
    reparse::set_needed_privilege();
    reparse::reparse_folder folder{ folderName, /* readonly */ false };
    if (!folder.is_valid())
    {
        std::fputws(L"Making folder a junction.\r\n", stdout);
    }
    else
    {
        std::fputws(L"Changing junction target.\r\n", stdout);
    }

    auto fullName = reparse::reparse_folder{ newPath }.full_path();
    //auto fullName = newPath;

    folder.set_junction_target(fullName, newPath);
}

void do_watch_vdesk()
{
    auto ishell = wil::CoCreateInstance<CImmersiveShell, IServiceProvider>(CLSCTX_LOCAL_SERVER);
    
    wil::com_ptr<IVirtualDesktopManagerInternal> vdmgr;
    THROW_IF_FAILED(ishell->QueryService<IVirtualDesktopManagerInternal>(__uuidof(CVirtualDesktopManagerInternal), &vdmgr));

    wil::com_ptr<IObjectArray> vdesks;
    THROW_IF_FAILED(vdmgr->GetDesktops(&vdesks));

    UINT count;
    THROW_IF_FAILED(vdesks->GetCount(&count));
    printf("%d Desktops:\n", count);
    for (UINT i = 0; i < count; i++)
    {
        wil::com_ptr<IVirtualDesktop> desk;
        THROW_IF_FAILED(vdesks->GetAt(i, __uuidof(IVirtualDesktop), desk.put_void()));

        GUID guid;
        THROW_IF_FAILED(desk->GetID(&guid));

        printf("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
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

    //winrt::init_apartment();
    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    std::span<wchar_t const* const> const args(argv, argc);

    if (args.size() < 2)
    {
        std::fputws(L"Usage: app <folder to operate on> [<new target>]\r\n", stderr);
        std::fputws(L"       app vdesk\r\n", stderr);
        return -1;
    }

    if (args.size() == 2)
    {
        if (std::wstring_view{ args[1] } == std::wstring_view{ L"vdesk" })
        {
            do_watch_vdesk();
        }
        else
        {
            do_read(args[1]);
        }
    }
    else if (args.size() >= 3)
    {
        do_write(args[1], args[2]);
    }

    return 0;
}
CATCH_RETURN();
