#include "pch.h"
#include "priv.h"
#include "reparse.h"
#include "IVirtualDesktop.h"
#include "com.h"
#include "explore.h"
#include "util.h"
#include "config.h"

void do_read(std::wstring_view folderName)
{
    reparse::set_needed_privilege();
    reparse::reparse_folder folder{ folderName };
    if (!folder.is_valid())
    {
        fmt::print(L"Folder is not a reparse point.\r\n");
    }
    else
    {
        fmt::print(L"Folder is a reparse point.\r\n");
        if (folder.is_junction())
        {
            fmt::print(L"Folder is a mount point/junction.\r\n");
            auto const& [subs, print] = folder.get_junction_target();
            fmt::print(FMT_STRING(L"SubstituteName: {}\r\n"), subs);
            fmt::print(FMT_STRING(L"PrintName: {}\r\n"), print);
        }
    }
}

void do_write(std::wstring_view folderName, std::wstring_view newPath)
{
    reparse::set_needed_privilege();
    reparse::reparse_folder folder{ folderName, /* readonly */ false };
    if (!folder.is_valid())
    {
        fmt::print(L"Making folder a junction.\r\n");
    }
    else
    {
        fmt::print(L"Changing junction target.\r\n");
    }

    auto fullName = reparse::reparse_folder{ newPath }.full_path();

    folder.set_junction_target(fullName, newPath);
}

void do_watch_vdesk()
{
    auto ishell = wil::CoCreateInstance<CImmersiveShell, IServiceProvider>(CLSCTX_LOCAL_SERVER);
    
    wil::com_ptr<IVirtualDesktopManagerInternal> vdmgr;
    THROW_IF_FAILED(ishell->QueryService<IVirtualDesktopManagerInternal>(CLSID_CVirtualDesktopManagerInternal, &vdmgr));

    wil::com_ptr<IObjectArray> vdesks;
    THROW_IF_FAILED(vdmgr->GetDesktops(&vdesks));

    std::vector<wil::com_ptr<IVirtualDesktop>> vdesktopObjOwner;
    std::map<IVirtualDesktop*, UINT> indexMap;

    UINT count;
    THROW_IF_FAILED(vdesks->GetCount(&count));
    fmt::print(FMT_STRING("{} Desktops:\n"), count);
    vdesktopObjOwner.reserve(count);
    for (UINT i = 0; i < count; i++)
    {
        wil::com_ptr<IVirtualDesktop> desk;
        THROW_IF_FAILED(vdesks->GetAt(i, IID_IVirtualDesktop, desk.put_void()));

        GUID guid;
        THROW_IF_FAILED(desk->GetID(&guid));

        fmt::print(FMT_STRING("{} {{ Guid = {} }}\n"), i, guid);

        indexMap[desk.get()] = i;
        vdesktopObjOwner.emplace_back(std::move(desk));
    }

    wil::com_ptr<IVirtualDesktopNotificationService> vdnotifService;
    THROW_IF_FAILED(ishell->QueryService<IVirtualDesktopNotificationService>(CLSID_CVirtualDesktopNotificationService, &vdnotifService));
    
    struct VDNotifReciever : com::object<VDNotifReciever, IVirtualDesktopNotification>
    {
        std::vector<wil::com_ptr<IVirtualDesktop>> deskOwner;
        std::map<IVirtualDesktop*, UINT> indexMap;

        VDNotifReciever(std::vector<wil::com_ptr<IVirtualDesktop>> vec, std::map<IVirtualDesktop*, UINT> map)
           : deskOwner{ std::move(vec) }, indexMap{ std::move(map) }
        { }

        ~VDNotifReciever()
        {
        }

        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(IVirtualDesktop*, IVirtualDesktop*) noexcept override { return S_OK; }
        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(IVirtualDesktop*, IVirtualDesktop*) noexcept override { return S_OK; }
        HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(IUnknown*) noexcept override { return S_OK; }
        HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(IVirtualDesktop*) noexcept override { return S_OK;  }
        HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(IVirtualDesktop*, IVirtualDesktop*) noexcept override { return S_OK; }

        HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(IVirtualDesktop* pDesktopOld, IVirtualDesktop* pDesktopNew) noexcept override try
        {
            auto fromIdx = indexMap.at(pDesktopOld);
            auto toIdx = indexMap.at(pDesktopNew);

            fmt::print(FMT_STRING("Changed from {} to {}\n"), fromIdx, toIdx);

            return S_OK;
        }
        CATCH_RETURN();
    };

    auto reciever = VDNotifReciever::make<>(vdesktopObjOwner, indexMap);
    auto cookie = com::register_virtual_desktop_notification(vdnotifService, reciever);

    fmt::print(L"Now printing all virtual desktop changes.\n");
    fmt::print(L"Press enter to exit.\n");
    auto unused = getc(stdin);
}

void do_explore()
{
    explore::explorer_tracker tracker{ [] { fmt::print(L"Explorer restarted.\n"); } };

    fmt::print(L"Tracking restarts of Explorer.\n");
    tracker.start_tracking();
    fmt::print(L"Press enter to exit.\n");
    auto unused = getc(stdin);
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

    /*
    auto guid = IID_IAuthenticate;
    fmt::print(FMT_STRING("{0:e}\n"), guid);
    nlohmann::json test_j = guid;
    fmt::print(FMT_STRING("{}\n"), test_j.dump(2));
    auto guid2 = test_j.get<GUID>();
    assert(guid == guid2);
    auto guid3 = nlohmann::json("").get<GUID>();
    auto guid4 = nlohmann::json("-").get<GUID>();
    auto guid5 = nlohmann::json("--").get<GUID>();
    auto guid6 = nlohmann::json("ace").get<GUID>();
    auto guid7 = nlohmann::json("ace-").get<GUID>();
    auto guid8 = nlohmann::json("ace-01").get<GUID>();
    auto guid9 = nlohmann::json("ace-01-").get<GUID>();
    auto guida = nlohmann::json("ace-01-5").get<GUID>();
    auto guidb = nlohmann::json("ace-01-5f").get<GUID>();
    auto guidc = nlohmann::json("ace-01-5f-").get<GUID>();
    auto guidd = nlohmann::json("ace-01-5f-034").get<GUID>();
    auto guide = nlohmann::json("ace-01-5f-034-782-7").get<GUID>();
    */

    if (args.size() < 2)
    {
        fmt::print(stderr, FMT_STRING(L"Usage: {} <folder to operate on> [<new target>]\r\n"), args[0]);
        fmt::print(stderr, FMT_STRING(L"       {} vdesk\r\n"), args[0]);
        fmt::print(stderr, FMT_STRING(L"       {} explore\r\n"), args[0]);
        return -1;
    }

    if (args.size() == 2)
    {
        if (std::wstring_view{ args[1] } == std::wstring_view{ L"vdesk" })
        {
            do_watch_vdesk();
        }
        else if (std::wstring_view{ args[1] } == std::wstring_view{ L"explore" })
        {
            do_explore();
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
