#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Ipc.h"


#define SERVER_MAPPING_NAME     L"Local\\S28CC044A-8F75-43F8-91C6-BF2FCBC0E848@v8"
#define DESKTOP_MAPPING_NAME    L"Local\\D28CC044A-8F75-43F8-91C6-BF2FCBC0E848@v8"
#define AGENT_MAPPING_NAME      L"Local\\A28CC044A-8F75-43F8-91C6-BF2FCBC0E848@v8"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


static LONG s_LastIndex = -1;


static void CloseHandle2(HANDLE& h)
{
    HANDLE h2 = reinterpret_cast<HANDLE>(InterlockedExchangePointer(reinterpret_cast<PVOID*>(&h), NULL));
    if (CloseHandle(h2))
    {
        DBGPUT(L"CloseHandle(%p)", h2);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"CloseHandle(%p): Failed. error=%lu", h2, dwError);
    }
}


template<class T>
static T* Map(HANDLE& hMapping, PCWSTR pszName)
{
    LARGE_INTEGER size;
    size.QuadPart = sizeof(T);
    hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, pszName);
    if (hMapping != NULL)
    {
        DBGPUT(L"CreateFileMapping: return=%p", hMapping);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"CreateFileMapping: Failed. error=%lu", dwError);
        return NULL;
    }

    T* pBlock = (T*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pBlock != NULL)
    {
        DBGPUT(L"MapViewOfFile(%p): return=%p", hMapping, pBlock);
        return pBlock;
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"MapViewOfFile(%p): Failed. error=%lu", hMapping, dwError);
        CloseHandle2(hMapping);
        return NULL;
    }
}


static void Unmap(void* pBlock, HANDLE& hMapping)
{
    if (UnmapViewOfFile(pBlock))
    {
        DBGPUT(L"UnmapViewOfFile(%p)", pBlock);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"UnmapViewOfFile(%p): Failed. error=%lu", pBlock, dwError);
    }

    CloseHandle2(hMapping);
}


ServerIpc* ServerIpc::Map(HANDLE& hMapping)
{
    return ::Map<ServerIpc>(hMapping, SERVER_MAPPING_NAME);
}


void ServerIpc::Unmap(HANDLE& hMapping)
{
    ::Unmap(this, hMapping);
}


void ServerIpc::Clear()
{
    m_dwProcessId32 = 0;
    m_dwProcessId64 = 0;
    m_hWnd32 = 0;
    m_hWnd64 = 0;
    m_ToggleSequece.Clear();
    m_LastProcessed = 0;
}


DesktopIpc* DesktopIpc::Map(HANDLE& hMapping)
{
    return ::Map<DesktopIpc>(hMapping, DESKTOP_MAPPING_NAME);
}


void DesktopIpc::Unmap(HANDLE& hMapping)
{
    ::Unmap(this, hMapping);
}


void DesktopIpc::Clear()
{
    m_WM_AGENT_WAKEUP = RegisterWindowMessageW(L"WM_AGENT_WAKEUP");
    m_Paused = 0;
    m_dwFlags = 0;
    m_KeyboardLayoutSetting.m_LangIds = 0;
    m_LastLangId = 0;
}


AgentIpc* AgentIpc::Map(HANDLE& hMapping, DWORD dwThreadId)
{
    WCHAR szName[MAX_PATH] = { 0 };
    _snwprintf_s(szName, _TRUNCATE, L"%s@%lu", AGENT_MAPPING_NAME, dwThreadId);
    return ::Map<AgentIpc>(hMapping, szName);
}


void AgentIpc::Unmap(HANDLE& hMapping)
{
    ::Unmap(this, hMapping);
}


void AgentIpc::Clear(DWORD dwThreadId, DWORD dwFlags)
{
    m_dwThreadId = dwThreadId;
    m_dwFlags = dwFlags;
    m_LangId = 0;
    m_KeyboardOpenClose = 0;
    m_InputModeConversion = 0;
    m_dwValidity = 0;
}
