#include "pch.h"
#include "LowIntegrityImpersonator.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"
#include "UiPathTeam.KeyboardExtension.Server.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


Server* Server::m_pInstance = NULL;


Server::Server(HMODULE hModule)
    : m_hModule(hModule)
    , m_pServerIpc()
    , m_pDesktopIpc()
    , m_hExitEvent(NULL)
    , m_hClientProcessWatcher(NULL)
    , m_hClientProcess(NULL)
    , m_hKeybdHook(NULL)
    , m_hMouseHook(NULL)
    , m_AgentMap()
    , m_atomWndClass(0)
    , m_hwndMessage(NULL)
    , m_pressedKeys()
    , m_bBlockKeybd(false)
    , m_bBlockMouse(false)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Server::ctor");
    DBGPUT(L"Started.");
    Initialize();
    InterlockedCompareExchangePointer(reinterpret_cast<void**>(&m_pInstance), this, NULL);
    DBGPUT(L"Ended.");
}


Server::~Server()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Server::dtor");
    DBGPUT(L"Started.");
    InterlockedCompareExchangePointer(reinterpret_cast<void**>(&m_pInstance), NULL, this);
    Uninitialize();
    DBGPUT(L"Ended.");
}


void Server::Initialize()
{
    if (!m_pServerIpc.Map())
    {
        throw std::runtime_error("Client-Server IPC block unavailable.");
    }

    {
        // Temporarily impersonate the low integrity so that even Internet Explorer in the protected mode can map this IPC block.
        LowIntegrityImpersonator x;
        if (!m_pDesktopIpc.Map())
        {
            throw std::runtime_error("Server-AnyAgent IPC block unavailable.");
        }
    }
    m_pDesktopIpc->Clear();

    m_hExitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (m_hExitEvent == NULL)
    {
        Uninitialize();
        throw std::runtime_error("Event object unavailable.");
    }
}


void Server::Uninitialize()
{
    if (m_hExitEvent != NULL)
    {
        SetEvent(m_hExitEvent);
        DWORD dwCount = 0;
        HANDLE hh[16];
        if (m_hClientProcessWatcher != NULL)
        {
            hh[dwCount++] = m_hClientProcessWatcher;
        }
        if (dwCount > 0)
        {
            WaitForMultipleObjects(dwCount, hh, TRUE, 10000);
            for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                CloseHandle(hh[dwIndex]);
            }
        }
        CloseHandle(m_hExitEvent);
    }

    if (m_hClientProcess != NULL)
    {
        if (CloseHandle(m_hClientProcess))
        {
            DBGPUT(L"CloseHandle(%p)", m_hClientProcess);
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"CloseHandle(%p): Failed. error=%lu", m_hClientProcess, dwError);
        }
    }

    m_pDesktopIpc.Unmap();

    m_pServerIpc.Unmap();
}


void Server::Run()
{
    HWND hWnd = NULL;

    try
    {
        if (!RegisterWndClass())
        {
            Debug::Put(L"RegisterClassEx: Failed.");
            goto done;
        }

        hWnd = CreateWnd();
        if (hWnd == NULL)
        {
            Debug::Put(L"CreateWindowEx(HWND_MESSAGE): Failed.");
            goto done;
        }

        MainLoop();
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

done:

    if (hWnd != NULL)
    {
        DestroyWindow(hWnd);
    }

    UnregisterWndClass();
}


bool Server::RegisterClient(DWORD dwProcessId)
{
    m_hClientProcess = OpenProcess(SYNCHRONIZE, FALSE, dwProcessId);
    if (m_hClientProcess != NULL)
    {
        DBGPUT(L"OpenProcess(%lu): return=%p", dwProcessId, m_hClientProcess);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"OpenProcess(%lu): Failed. error=%lu", dwProcessId, dwError);
        return false;
    }

    m_hClientProcessWatcher = CreateThread(NULL, 0, WatchClientProcess, this, 0, NULL);
    if (m_hClientProcessWatcher != NULL)
    {
        return true;
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"CreateThread: Failed. error=%lu", dwError);
        return false;
    }
}


DWORD WINAPI Server::WatchClientProcess(
    _In_ LPVOID lpParameter
)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Server::WatchClientProcess");
    Server* pThis = reinterpret_cast<Server*>(lpParameter);
    HANDLE hh[2];
    hh[0] = pThis->m_hExitEvent;
    hh[1] = pThis->m_hClientProcess;
    DWORD dwRet = WaitForMultipleObjects(2, hh, FALSE, INFINITE);
    if (dwRet == WAIT_OBJECT_0 + 0)
    {
        DBGPUT(L"Server is going down.");
        return 0;
    }
    else if (dwRet == WAIT_OBJECT_0 + 1)
    {
        DBGPUT(L"Client exited.");
        PostMessageW(pThis->m_hwndMessage, WM_CLOSE, 0, 0);
        return 1;
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"WaitForMultipleObjects: Failed. return=%lu error=%lu", dwRet, dwError);
        return 2;
    }
}


void Server::InstallKeybdHook()
{
    if (m_hKeybdHook == NULL)
    {
        m_hKeybdHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardHook, m_hModule, 0);
        if (m_hKeybdHook != NULL)
        {
            DBGPUT(L"SetWindowsHookEx(WH_KEYBOARD_LL)");
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"SetWindowsHookEx(WH_KEYBOARD_LL): Failed. error=%lu", dwError);
        }
    }
}


void Server::InstallMouseHook()
{
    if (m_hMouseHook == NULL)
    {
        m_hMouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseHook, m_hModule, 0);
        if (m_hMouseHook != NULL)
        {
            DBGPUT(L"SetWindowsHookEx(WH_MOUSE_LL)");
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"SetWindowsHookEx(WH_MOUSE_LL): Failed. error=%lu", dwError);
        }
    }
}


bool Server::InstallCallWndProcHook(DWORD dwThreadId)
{
    AgentHook* pAgent = new AgentHook(dwThreadId, m_hModule, m_pDesktopIpc->m_dwFlags);

    if (pAgent->m_pIpc && pAgent->m_hCallWndProcHook != NULL)
    {
        m_AgentMap.insert(std::pair<DWORD, AgentHook*>(dwThreadId, pAgent));
        return true;
    }
    else
    {
        delete pAgent;
        return false;
    }
}


void Server::UninstallKeybdHook()
{
    if (m_hKeybdHook != NULL)
    {
        if (UnhookWindowsHookEx(m_hKeybdHook))
        {
            DBGPUT(L"UnhookWindowsHookEx(WH_KEYBOARD_LL)");
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"UnhookWindowsHookEx(WH_KEYBOARD_LL): Failed. error=%lu", dwError);
        }
        m_hKeybdHook = NULL;
    }
}


void Server::UninstallMouseHook()
{
    if (m_hMouseHook != NULL)
    {
        if (UnhookWindowsHookEx(m_hMouseHook))
        {
            DBGPUT(L"UnhookWindowsHookEx(WH_MOUSE_LL)");
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"UnhookWindowsHookEx(WH_MOUSE_LL): Failed. error=%lu", dwError);
        }
        m_hMouseHook = NULL;
    }
}


void Server::UninstallCallWndProcHook(std::map<DWORD, AgentHook*>::iterator& iter)
{
    DWORD dwThreadId = iter->first;
    AgentHook* pAgent = iter->second;
    iter->second = NULL;
    delete pAgent;
    m_AgentMap.erase(iter);
}


void Server::UninstallAllCallWndProcHooks()
{
    for (std::map<DWORD, AgentHook*>::iterator iter = m_AgentMap.begin(); iter != m_AgentMap.end(); iter++)
    {
        DWORD dwThreadId = iter->first;
        AgentHook* pAgent = iter->second;
        iter->second = NULL;
        delete pAgent;
    }
    m_AgentMap.clear();
}


AgentHook::AgentHook(DWORD dwThreadId, HMODULE hModule, DWORD dwFlags)
    : m_pIpc()
    , m_hCallWndProcHook(NULL)
{
    {
        // Temporarily impersonate the low integrity so that even Internet Explorer in the protected mode can map this IPC block.
        LowIntegrityImpersonator x;
        if (!m_pIpc.Map(dwThreadId))
        {
            return;
        }
    }
    m_pIpc->Clear(dwThreadId, dwFlags);

    m_hCallWndProcHook = SetWindowsHookExW(WH_CALLWNDPROC, Agent::CallWndProcHook, hModule, dwThreadId);
    if (m_hCallWndProcHook != NULL)
    {
        DBGPUT(L"SetWindowsHookEx(WH_CALLWNDPROC,%lu): return=%p", dwThreadId, m_hCallWndProcHook);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"SetWindowsHookEx(WH_CALLWNDPROC): Failed. error=%lu", dwError);
    }
}


AgentHook::~AgentHook()
{
    if (m_hCallWndProcHook != NULL)
    {
        if (UnhookWindowsHookEx(m_hCallWndProcHook))
        {
            DBGPUT(L"UnhookWindowsHookEx(WH_CALLWNDPROC::%p::%lu)", m_hCallWndProcHook, m_pIpc->m_dwThreadId);
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"UnhookWindowsHookEx(WH_CALLWNDPROC::%p::%lu): Failed. error=%lu", m_hCallWndProcHook, m_pIpc->m_dwThreadId, dwError);
        }
    }

    m_pIpc.Unmap();
}
