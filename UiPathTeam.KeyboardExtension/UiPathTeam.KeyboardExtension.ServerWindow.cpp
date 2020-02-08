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
        case WM_APP_START_AGENT:
            DBGPUT(L"WM_APP_START_AGENT: Started.");
            pThis->StartAgent(reinterpret_cast<HWND>(wParam));
            DBGPUT(L"WM_APP_START_AGENT: Ended.");
            return 0;
        case WM_APP_STOP_AGENT:
            DBGPUT(L"WM_APP_STOP_AGENT: Started.");
            pThis->StopAgent(reinterpret_cast<HWND>(wParam));
            DBGPUT(L"WM_APP_STOP_AGENT: Ended.");
            return 0;
        case WM_APP_NOTIFY_AGENTS:
            DBGPUT(L"WM_APP_NOTIFY_AGENTS: Started.");
            pThis->NotifyAgents();
            DBGPUT(L"WM_APP_NOTIFY_AGENTS: Ended.");
            return 0;
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
    InstallKeybdHook();
    InstallMouseHook();
    if (Platform::Is64bitProcess())
    {
        InterlockedExchange(&m_pIpcBlock->m_dwProcessId64, GetCurrentProcessId());
        InterlockedExchange(&m_pIpcBlock->m_hWnd64, static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hwnd)));
    }
    else
    {
        InterlockedExchange(&m_pIpcBlock->m_dwProcessId32, GetCurrentProcessId());
        InterlockedExchange(&m_pIpcBlock->m_hWnd32, static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hwnd)));
    }
}


void Server::OnDestroy()
{
    m_hwndMessage = NULL;
    StopAgent(NULL);
    if (Platform::Is64bitProcess())
    {
        InterlockedExchange(&m_pIpcBlock->m_dwProcessId64, 0);
        InterlockedExchange(&m_pIpcBlock->m_hWnd64, 0);

    }
    else
    {
        InterlockedExchange(&m_pIpcBlock->m_dwProcessId32, 0);
        InterlockedExchange(&m_pIpcBlock->m_hWnd32, 0);
    }
    UninstallMouseHook();
    UninstallKeybdHook();
}


void Server::StartAgent(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::StartAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    DWORD dwProcessId = 0;
    DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
    std::map<DWORD, HHOOK>::iterator iter = m_CallWndProcHookMap.find(dwThreadId);
    if (iter == m_CallWndProcHookMap.end())
    {
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        if (!InstallCallWndProcHook(dwThreadId))
        {
            DBGPUT(L"Ended.");
            return;
        }
    }
    SendMessageTimeoutW(hwnd, m_pIpcBlock->m_WM_AGENT_WAKEUP, AGENT_ENABLED, 0, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
    DBGPUT(L"Ended.");
}


void Server::StopAgent(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::StopAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        std::map<DWORD, HHOOK>::iterator iter = m_CallWndProcHookMap.find(dwThreadId);
        if (iter != m_CallWndProcHookMap.end())
        {
            DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
            SendMessageTimeoutW(hwnd, m_pIpcBlock->m_WM_AGENT_WAKEUP, AGENT_DISABLED, 0, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
            UninstallCallWndProcHook(iter);
        }
    }
    else
    {
        NotifyAgents(AGENT_DISABLED);
        UninstallAllCallWndProcHooks();
    }
    DBGPUT(L"Ended.");
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
    std::map<DWORD, HHOOK>::iterator iter = pThis->m_CallWndProcHookMap.find(dwThreadId);
    if (iter != pThis->m_CallWndProcHookMap.end())
    {
        DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP,%lu,%lu) Started.", hwnd, dwThreadId, dwProcessId, pSWUS->wParam, pSWUS->lParam);
        SendMessageTimeoutW(hwnd, pThis->m_pIpcBlock->m_WM_AGENT_WAKEUP, pSWUS->wParam, pSWUS->lParam, SMTO_NOTIMEOUTIFNOTHUNG, WAKEUP_TIMEOUT, NULL);
        DBGPUT(L"SendMessage(%p::%lu::%lu,WM_AGENT_WAKEUP,%lu,%lu) Ended.", hwnd, dwThreadId, dwProcessId, pSWUS->wParam, pSWUS->lParam);
    }
    return TRUE;
}
