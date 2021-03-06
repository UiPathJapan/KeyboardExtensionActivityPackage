#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"
#include "UiPathTeam.KeyboardExtension.Client.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


HMODULE g_hModule;
DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;


#ifdef _DEBUG
WCHAR g_szExeFileName[MAX_PATH];
#endif //_DEBUG


static void FreeAgent()
{
    if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
    {
        // Nothing to do
        return;
    }

    Agent* pContext = reinterpret_cast<Agent*>(TlsGetValue(g_dwTlsIndex));
    if (pContext == NULL)
    {
        // Nothing to do
        return;
    }

    try
    {
        pContext->Uninitialize();
        pContext->Release();
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

    TlsSetValue(g_dwTlsIndex, NULL);
}


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::DLL_PROCESS_ATTACH");
            DBGPUT(L"Started.");
            g_hModule = hModule;
            g_dwTlsIndex = TlsAlloc();
#ifdef _DEBUG
            if (GetModuleFileNameW(NULL, g_szExeFileName, MAX_PATH))
            {
                DBGPUT(L"Executable=%s", g_szExeFileName);
            }
#endif //_DEBUG
            DBGPUT(L"Ended.");
            break;
        }

        case DLL_THREAD_ATTACH:
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::DLL_THREAD_ATTACH");
            DBGPUT(L"Started.");
            DBGPUT(L"Ended.");
            break;
        }

        case DLL_THREAD_DETACH:
        {
            Debug::Function x(L"UiPathTeam::KeyboardExtension::DLL_THREAD_DETACH");
            DBGPUT(L"Started.");
            FreeAgent();
            DBGPUT(L"Ended.");
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            Debug::Function x(L"UiPathTeam::KeyboardExtension::DLL_PROCESS_DETACH");
            DBGPUT(L"Started.");
            FreeAgent();
            TlsFree(g_dwTlsIndex);
            delete KeyboardExtension::Client::m_pInstance;
            DBGPUT(L"Ended. Executable=%s", g_szExeFileName[0] ? g_szExeFileName : L"(unknown)");
            break;
        }
    }
    return TRUE;
}
