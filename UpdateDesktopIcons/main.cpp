#include "pch.h"
#include "priv.h"
#include "reparse.h"
#include "IVirtualDesktop.h"
#include "com.h"

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

    std::vector<wil::com_ptr<IVirtualDesktop>> vdesktopObjOwner;
    std::map<IVirtualDesktop*, UINT> indexMap;

    UINT count;
    THROW_IF_FAILED(vdesks->GetCount(&count));
    printf("%d Desktops:\n", count);
    vdesktopObjOwner.reserve(count);
    for (UINT i = 0; i < count; i++)
    {
        wil::com_ptr<IVirtualDesktop> desk;
        THROW_IF_FAILED(vdesks->GetAt(i, __uuidof(IVirtualDesktop), desk.put_void()));

        GUID guid;
        THROW_IF_FAILED(desk->GetID(&guid));

        printf("%d { Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX} }\n",
            i,
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        indexMap[desk.get()] = i;
        vdesktopObjOwner.emplace_back(std::move(desk));
    }

    wil::com_ptr<IVirtualDesktopNotificationService> vdnotifService;
    THROW_IF_FAILED(ishell->QueryService<IVirtualDesktopNotificationService>(__uuidof(CVirtualDesktopNotificationService), &vdnotifService));
    
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
            //wil::com_ptr<IVirtualDesktop> from{ pDesktopOld }, to{ pDesktopNew };

            auto fromIdx = indexMap.at(pDesktopOld);
            auto toIdx = indexMap.at(pDesktopNew);

            printf("Changed from %d to %d\n", fromIdx, toIdx);

            return S_OK;
        }
        CATCH_RETURN();
    };

    using unique_vd_reg_cookie = wil::unique_com_token<IVirtualDesktopNotificationService, DWORD,
        decltype(&IVirtualDesktopNotificationService::Unregister), &IVirtualDesktopNotificationService::Unregister>;

    /*wil::com_ptr<IVirtualDesktopNotification> reciever;
    reciever.attach(winrt::make<VDNotifReciever>(vdesktopObjOwner, indexMap).detach());*/
    auto reciever = VDNotifReciever::make<>(vdesktopObjOwner, indexMap);
    unique_vd_reg_cookie regCookie{ vdnotifService.get() };
    THROW_IF_FAILED(vdnotifService->Register(reciever.get(), &regCookie));

    std::fputws(L"Now printing all virtual desktop changes.\n", stdout);
    std::fputws(L"Press enter to exit.\n", stdout);
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
