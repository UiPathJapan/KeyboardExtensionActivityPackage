#include "pch.h"
#include "Debug.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"
#include "UiPathTeam.KeyboardExtension.Client.h"
#include "UiPathTeam.KeyboardExtension.Server.h"


extern HMODULE g_hModule;


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


extern "C" bool __stdcall StartServer(void)
{
    bool bRet = false;
    Debug::Function x(L"UiPathTeam::KeyboardExtension::StartServer");
    DBGPUT(L"Started.");
    try
    {
        if (Client::m_pInstance == NULL)
        {
            new Client(g_hModule);
        }
        bRet = Client::m_pInstance->StartServer();
    }
    catch (std::runtime_error ex)
    {
        Debug::Put(L"%hs", ex.what());
    }
    catch (std::bad_alloc)
    {
        Debug::Put(L"Out of memory.");
    }
    catch (...)
    {
        Debug::Put(L"Unhandled exception caught.");
    }
    DBGPUT(L"Ended. return=%s", bRet ? L"true" : L"false");
    return bRet;
}


extern "C" void __stdcall StopServer(void)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::StopServer");
    DBGPUT(L"Started.");
    if (Client::m_pInstance != NULL)
    {
        try
        {
            Client::m_pInstance->StopServer();
            delete Client::m_pInstance;
        }
        catch (std::runtime_error ex)
        {
            Debug::Put(L"%hs", ex.what());
        }
        catch (std::bad_alloc)
        {
            Debug::Put(L"Out of memory.");
        }
        catch (...)
        {
            Debug::Put(L"Unhandled exception caught.");
        }
    }
    DBGPUT(L"Ended.");
}


extern "C" void __stdcall SetFlags(DWORD dwSet, DWORD dwReset)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::SetFlags");
    DBGPUT(L"Started. set=%08lx reset=%08lx", dwSet, dwReset);
    if (Client::m_pInstance != NULL)
    {
        Client::m_pInstance->SetFlags(dwSet, dwReset);
        Client::m_pInstance->NotifyAgents();
    }
    DBGPUT(L"Ended.");
}


extern "C" void __stdcall SetToggleSequence(PCWSTR psz)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::SetToggleSequence");
    DBGPUT(psz != NULL ? L"Started. sequence=\"%s\"" : L"Started. sequence=NULL", psz);
    if (Client::m_pInstance != NULL)
    {
        Client::m_pInstance->SetToggleSequence(psz);
    }
    DBGPUT(L"Ended.");
}


extern "C" void __stdcall SetPreferredKeyboardLayout(LANGID langId)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::SetPreferredKeyboardLayout");
    DBGPUT(L"Started. langId=%04x", langId);
    if (Client::m_pInstance != NULL)
    {
        Client::m_pInstance->SetPreferredKeyboardLayout(langId);
        Client::m_pInstance->NotifyAgents();
    }
    DBGPUT(L"Ended.");
}


extern "C" bool __stdcall StartAgent(HWND hwnd)
{
    bool bRet = false;
    Debug::Function x(L"UiPathTeam::KeyboardExtension::StartAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    try
    {
        if (Client::m_pInstance == NULL)
        {
            new Client(g_hModule);
        }
        bRet = Client::m_pInstance->StartAgent(hwnd);
    }
    catch (std::runtime_error ex)
    {
        Debug::Put(L"%hs", ex.what());
    }
    catch (std::bad_alloc)
    {
        Debug::Put(L"Out of memory.");
    }
    catch (...)
    {
        Debug::Put(L"Unhandled exception caught.");
    }
    DBGPUT(L"Ended. return=%s", bRet ? L"true" : L"false");
    return bRet;
}


extern "C" void __stdcall StopAgent(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::StopAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    if (Client::m_pInstance != NULL)
    {
        Client::m_pInstance->StopAgent(hwnd);
    }
    DBGPUT(L"Ended.");
}


// This is the server main procedure to be passed to RUNDLL32.
extern "C" void CALLBACK RunServer(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::RunServer%d", Platform::Is64bitProcess() ? 64 : 32);

    DBGPUT(L"Started. CmdLine=\"%hs\"", lpszCmdLine);

    try
    {
        Server server(g_hModule);

        PCHAR pStop = lpszCmdLine;
        DWORD dwClientPid = strtoul(lpszCmdLine, &pStop, 10);
        if (pStop > lpszCmdLine && *pStop == '\0')
        {
            if (!server.RegisterClient(dwClientPid))
            {
                throw std::runtime_error("No valid client process handle available.");
            }
        }
        else
        {
            throw std::runtime_error("No valid client process ID.");
        }

        server.Run();
    }
    catch (std::runtime_error ex)
    {
        Debug::Put(L"%hs", ex.what());
    }
    catch (std::bad_alloc)
    {
        Debug::Put(L"Out of memory.");
    }
    catch (...)
    {
        Debug::Put(L"Unhandled exception caught.");
    }

    DBGPUT(L"Ended.");
}
