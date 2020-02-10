#include "pch.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.h"
#include "UiPathTeam.KeyboardExtension.Server.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


bool Server::RegisterWndClass()
{
    WNDCLASSEX wcx = { 0 };
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = m_hModule;
    wcx.lpszClassName = WINDOW_CLASSNAME;
    m_atomWndClass = RegisterClassExW(&wcx);
    return m_atomWndClass != 0;
}


void Server::UnregisterWndClass()
{
    if (m_atomWndClass != 0)
    {
        UnregisterClassW(WINDOW_CLASSNAME, m_hModule);
        m_atomWndClass = 0;
    }
}


HWND Server::CreateWnd()
{
    return CreateWindowExW(0, WINDOW_CLASSNAME, WINDOW_NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, m_hModule, this);
}


void Server::MainLoop()
{
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessageW(&msg, NULL, 0, 0)) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (bRet < 0)
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"GetMessage: Failed. error=%lu", dwError);
    }
}


LRESULT CALLBACK Server::WindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Server::WindowProc%d", Platform::Is64bitProcess() ? 64 : 32);

    Server* pThis = reinterpret_cast<Server*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (pThis != NULL)
    {
        LRESULT lRet;
        switch (uMsg)
        {
        case WM_DESTROY:
            DBGPUT(L"WM_DESTROY: Started.");
            pThis->OnDestroy();
            DBGPUT(L"WM_DESTROY: Ended.");
            return 0;
        case WM_CLOSE:
            DBGPUT(L"WM_CLOSE");
            PostQuitMessage(0);
            return 0;
        case WM_APP_GET_BLOCK_INPUT:
            DBGPUT(L"WM_APP_GET_BLOCK_INPUT: Started.");
            lRet = BLOCK_FLAGS(pThis->m_bBlockKeybd ? 1 : 0, pThis->m_bBlockMouse ? 1 : 0);
            DBGPUT(L"WM_APP_GET_BLOCK_INPUT: Ended. return=%08lx", lRet);
            return lRet;
        case WM_APP_SET_BLOCK_INPUT:
            DBGPUT(L"WM_APP_SET_BLOCK_INPUT: Started.");
            pThis->m_pDesktopIpc->m_Paused = 0;
            lRet = BLOCK_FLAGS(pThis->m_bBlockKeybd ? 1 : 0, pThis->m_bBlockMouse ? 1 : 0);
            pThis->m_bBlockKeybd = (wParam & BLOCK_KEYBD) == BLOCK_KEYBD;
            pThis->m_bBlockMouse = (wParam & BLOCK_MOUSE) == BLOCK_MOUSE;
            DBGPUT(L"WM_APP_SET_BLOCK_INPUT: Ended. return=%08lx", lRet);
            return lRet;
        case WM_APP_START_AGENT:
            DBGPUT(L"WM_APP_START_AGENT: Started.");
            lRet = pThis->StartAgent(reinterpret_cast<HWND>(wParam));
            DBGPUT(L"WM_APP_START_AGENT: Ended. return=%lu", lRet);
            return lRet;
        case WM_APP_STOP_AGENT:
            DBGPUT(L"WM_APP_STOP_AGENT: Started.");
            pThis->StopAgent(reinterpret_cast<HWND>(wParam));
            DBGPUT(L"WM_APP_STOP_AGENT: Ended.");
            return 0;
        case WM_APP_GET_FLAGS:
            DBGPUT(L"WM_APP_GET_FLAGS: Started.");
            lRet = pThis->GetFlags(reinterpret_cast<HWND>(wParam));
            DBGPUT(L"WM_APP_GET_FLAGS: Ended. return=%08lx", lRet);
            return lRet;
        case WM_APP_SET_FLAGS:
            DBGPUT(L"WM_APP_SET_FLAGS: Started.");
            pThis->m_pDesktopIpc->m_Paused = 0;
            lRet = pThis->SetFlags(reinterpret_cast<HWND>(wParam), static_cast<DWORD>(lParam));
            DBGPUT(L"WM_APP_SET_FLAGS: Ended. return=%08lx", lRet);
            return lRet;
        case WM_APP_GET_KBD_LAYOUT:
            DBGPUT(L"WM_APP_GET_KBD_LAYOUT: Started.");
            lRet = pThis->m_pDesktopIpc->m_KeyboardLayoutSetting.m_PreferredLangId;
            DBGPUT(L"WM_APP_GET_KBD_LAYOUT: Ended. return=%04lx", lRet);
            return lRet;
        case WM_APP_SET_KBD_LAYOUT:
            DBGPUT(L"WM_APP_SET_KBD_LAYOUT: Started.");
            {
                pThis->m_pDesktopIpc->m_Paused = 0;
                lRet = pThis->m_pDesktopIpc->m_KeyboardLayoutSetting.m_PreferredLangId;
                KeyboardLayoutSetting next;
                next.m_PreferredLangId = static_cast<LANGID>(wParam);
                next.m_SelectedLangId = 0;
                InterlockedExchange(&pThis->m_pDesktopIpc->m_KeyboardLayoutSetting.m_LangIds, next.m_LangIds);
            }
            DBGPUT(L"WM_APP_SET_KBD_LAYOUT: Ended. return=%04lx", lRet);
            return lRet;
        default:
            break;
        }
    }
    else if (uMsg == WM_CREATE)
    {
        DBGPUT(L"WM_CREATE: Started.");
        LPCREATESTRUCTW pCS = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        pThis = reinterpret_cast<Server*>(pCS->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->OnCreate(hwnd);
        DBGPUT(L"WM_CREATE: Ended.");
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


void Server::OnCreate(HWND hwnd)
{
    m_hwndMessage = hwnd;
    if (Platform::Is64bitProcess())
    {
        InstallKeybdHook();
        InstallMouseHook();
        InterlockedExchange(&m_pServerIpc->m_dwProcessId64, GetCurrentProcessId());
        InterlockedExchange(&m_pServerIpc->m_hWnd64, static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hwnd)));
    }
    else
    {
        if (!Platform::IsWow64Process())
        {
            InstallKeybdHook();
            InstallMouseHook();
        }
        InterlockedExchange(&m_pServerIpc->m_dwProcessId32, GetCurrentProcessId());
        InterlockedExchange(&m_pServerIpc->m_hWnd32, static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hwnd)));
    }
}


void Server::OnDestroy()
{
    m_hwndMessage = NULL;
    StopAgent(NULL);
    if (Platform::Is64bitProcess())
    {
        InterlockedExchange(&m_pServerIpc->m_dwProcessId64, 0);
        InterlockedExchange(&m_pServerIpc->m_hWnd64, 0);
    }
    else
    {
        InterlockedExchange(&m_pServerIpc->m_dwProcessId32, 0);
        InterlockedExchange(&m_pServerIpc->m_hWnd32, 0);
    }
    UninstallMouseHook();
    UninstallKeybdHook();
}


bool Server::StartAgent(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::StartAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    DWORD dwProcessId = 0;
    DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
    std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.find(dwThreadId);
    if (iter == m_AgentMap.end())
    {
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        if (!InstallCallWndProcHook(dwThreadId))
        {
            DBGPUT(L"Ended. return=false");
            return false;
        }
    }
    SendMessageTimeoutW(hwnd, m_pDesktopIpc->m_WM_AGENT_WAKEUP, AGENT_INITIALIZE, 0, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
    DBGPUT(L"Ended. return=true");
    return true;
}


void Server::StopAgent(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::StopAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.find(dwThreadId);
        if (iter != m_AgentMap.end())
        {
            DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
            SendMessageTimeoutW(hwnd, m_pDesktopIpc->m_WM_AGENT_WAKEUP, AGENT_UNINITIALIZE, 0, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
            UninstallCallWndProcHook(iter);
        }
    }
    else
    {
        NotifyAgents(AGENT_UNINITIALIZE);
        UninstallAllCallWndProcHooks();
    }
    DBGPUT(L"Ended.");
}


DWORD Server::GetFlags(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::GetFlags");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    DWORD dwFlags = (DWORD)-1;
    if (hwnd)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.find(dwThreadId);
        if (iter != m_AgentMap.end())
        {
            dwFlags = iter->second->m_pIpc->m_dwFlags;
        }
    }
    else
    {
        dwFlags = m_pDesktopIpc->m_dwFlags;
    }
    DBGPUT(L"Ended. return=%08lx", dwFlags);
    return dwFlags;
}


DWORD Server::SetFlags(HWND hwnd, DWORD dwFlags)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::SetFlags");
    DBGPUT(L"Started. hwnd=%p flags=%08lx", hwnd, dwFlags);
    DWORD dwFlags0 = (DWORD)-1;
    if (hwnd)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.find(dwThreadId);
        if (iter != m_AgentMap.end())
        {
            dwFlags0 = InterlockedExchange(&iter->second->m_pIpc->m_dwFlags, dwFlags);
            if (dwFlags != dwFlags0)
            {
                DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP) Started.", hwnd, dwThreadId, dwProcessId);
                SendMessageTimeoutW(hwnd, m_pDesktopIpc->m_WM_AGENT_WAKEUP, 0, 0, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
                DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP) Ended.", hwnd, dwThreadId, dwProcessId);
            }
        }
    }
    else
    {
        dwFlags0 = InterlockedExchange(&m_pDesktopIpc->m_dwFlags, dwFlags);
        for (std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.begin(); iter != m_AgentMap.end(); iter++)
        {
            InterlockedExchange(&iter->second->m_pIpc->m_dwFlags, dwFlags);
        }
        NotifyAgents();
    }
    DBGPUT(L"Ended. return=%08lx", dwFlags0);
    return dwFlags0;
}


typedef struct
{
    Server* pThis;
    WPARAM wParam;
    LPARAM lParam;
} ServerWakeUpStruct;


void Server::NotifyAgents(WPARAM wParam, LPARAM lParam)
{
    ServerWakeUpStruct swus = { this, wParam, lParam };
    EnumWindows(SendWakeUpMessage, reinterpret_cast<LPARAM>(&swus));
}


BOOL CALLBACK Server::SendWakeUpMessage(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    ServerWakeUpStruct* pSWUS = reinterpret_cast<ServerWakeUpStruct*>(lParam);
    Server* pThis = pSWUS->pThis;
    DWORD dwProcessId = 0;
    DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
    std::map<DWORD, AgentHook*>::iterator iter = pThis->m_AgentMap.find(dwThreadId);
    if (iter != pThis->m_AgentMap.end())
    {
        DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP,%lu,%lu) Started.", hwnd, dwThreadId, dwProcessId, pSWUS->wParam, pSWUS->lParam);
        SendMessageTimeoutW(hwnd, pThis->m_pDesktopIpc->m_WM_AGENT_WAKEUP, pSWUS->wParam, pSWUS->lParam, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
        DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP,%lu,%lu) Ended.", hwnd, dwThreadId, dwProcessId, pSWUS->wParam, pSWUS->lParam);
    }
    return TRUE;
}
