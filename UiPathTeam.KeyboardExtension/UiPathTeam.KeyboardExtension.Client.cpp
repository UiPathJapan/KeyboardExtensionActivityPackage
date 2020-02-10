#include "pch.h"
#include "Debug.h"
#include "Platform.h"
#include "KnownFolderPath.h"
#include "UiPathTeam.KeyboardExtension.h"
#include "UiPathTeam.KeyboardExtension.Client.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


Client* Client::m_pInstance = NULL;


Client::Client(HMODULE hModule)
    : m_hModule(hModule)
    , m_pIpc()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::ctor");
    DBGPUT(L"Started.");
    if (!m_pIpc.Map())
    {
        throw std::runtime_error("Client-Server IPC block unavailable.");
    }
    m_pIpc->Clear();
    InterlockedCompareExchangePointer(reinterpret_cast<void**>(&m_pInstance), this, NULL);
    DBGPUT(L"Ended.");
}


Client::~Client()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::dtor");
    DBGPUT(L"Started.");
    InterlockedCompareExchangePointer(reinterpret_cast<void**>(&m_pInstance), NULL, this);
    m_pIpc.Unmap();
    DBGPUT(L"Ended.");
}


// Creates processes of RUNDLL32 to host the server if they are not running.
bool Client::StartServer()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::StartServer");

    if (!m_pIpc)
    {
        Debug::Put(L"IPC block unavailable.");
        return false;
    }

    if (m_pIpc->m_dwProcessId32 != 0)
    {
        HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, m_pIpc->m_dwProcessId32);
        if (h != NULL)
        {
            CloseHandle(h);
            DBGPUT(L"Server32: PID=%lu", m_pIpc->m_dwProcessId32);
        }
        else
        {
            DWORD dwError = GetLastError();
            DBGPUT(L"OpenProcess(%lu): Failed. error=%lu", m_pIpc->m_dwProcessId32, dwError);
            InterlockedExchange(&m_pIpc->m_dwProcessId32, 0);
        }
    }

    bool b32 = m_pIpc->m_dwProcessId32 != 0 ? true : StartServer(32);

    if (Platform::Is32bit())
    {
        return b32;
    }

    if (m_pIpc->m_dwProcessId64 != 0)
    {
        HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, m_pIpc->m_dwProcessId64);
        if (h != NULL)
        {
            CloseHandle(h);
            DBGPUT(L"Server64: PID=%lu", m_pIpc->m_dwProcessId64);
        }
        else
        {
            DWORD dwError = GetLastError();
            DBGPUT(L"OpenProcess(%lu): Failed. error=%lu", m_pIpc->m_dwProcessId64, dwError);
            InterlockedExchange(&m_pIpc->m_dwProcessId64, 0);
        }
    }

    bool b64 = m_pIpc->m_dwProcessId64 != 0 ? true : StartServer(64);

    return b32 && b64;
}


// Creates a process of RUNDLL32 to run RunServer function, which is the main procedure of the server. 
bool Client::StartServer(int bitness)
{
    STARTUPINFOW si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(STARTUPINFOW);

    WCHAR szCommandLine[MAX_PATH * 2];

    if (!BuildCommandLine(m_hModule, bitness, szCommandLine))
    {
        return false;
    }

    if (CreateProcessW(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        bool bRet = false;
        DBGPUT(L"CreateProcess(%s): PID=%lu", szCommandLine, pi.dwProcessId);
        for (int i = 0; i < START_TIMEOUT; i++)
        {
            if (bitness == 64)
            {
                bRet = m_pIpc->m_dwProcessId64 != 0;
            }
            else
            {
                bRet = m_pIpc->m_dwProcessId32 != 0;
            }
            if (bRet)
            {
                break;
            }
            DWORD dwRet = WaitForSingleObject(pi.hProcess, 1);
            if (dwRet == WAIT_OBJECT_0)
            {
                DBGPUT(L"Server%d exited.", bitness);
                break;
            }
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return bRet;
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"CreateProcess(%s): Failed. error=%lu", szCommandLine, dwError);
        return false;
    }
}


// Posts a WM_CLOSE to each server's message-only window to end the main procedure of the server.
// Monitors the termination of each server process for SHUTDOWN_TIMEOUT milliseconds at most.
// Finally, force-terminates the servers that are not ended.
void Client::StopServer()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::StopServer");

    if (!m_pIpc)
    {
        Debug::Put(L"IPC block unavailable.");
        return;
    }

    DWORD dwCount = 0;
    HANDLE hh[2];
    DWORD ii[2];

    if (m_pIpc->m_dwProcessId32 != 0)
    {
        hh[dwCount] = OpenProcess(SYNCHRONIZE, FALSE, m_pIpc->m_dwProcessId32);
        if (hh[dwCount] != NULL)
        {
            DBGPUT(L"OpenProcess(%lu): return=%p", m_pIpc->m_dwProcessId32, hh[dwCount]);
            ii[dwCount] = m_pIpc->m_dwProcessId32;
            dwCount++;
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"OpenProcess(%lu): Failed. error=%lu", m_pIpc->m_dwProcessId32, dwError);
        }
    }

    if (m_pIpc->m_dwProcessId64 != 0)
    {
        hh[dwCount] = OpenProcess(SYNCHRONIZE, FALSE, m_pIpc->m_dwProcessId64);
        if (hh[dwCount] != NULL)
        {
            DBGPUT(L"OpenProcess(%lu): return=%p", m_pIpc->m_dwProcessId64, hh[dwCount]);
            ii[dwCount] = m_pIpc->m_dwProcessId64;
            dwCount++;
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"OpenProcess(%lu): Failed. error=%lu", m_pIpc->m_dwProcessId64, dwError);
        }
    }

    if (m_pIpc->m_hWnd32 != 0)
    {
        DBGPUT(L"PostMessage(32bit::%08lx,WM_CLOSE)...", m_pIpc->m_hWnd32);
        PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_CLOSE, 0, 0);
    }

    if (m_pIpc->m_hWnd64 != 0)
    {
        DBGPUT(L"PostMessage(64bit::%08lx,WM_CLOSE)...", m_pIpc->m_hWnd64);
        PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_CLOSE, 0, 0);
    }

    for (int i = SHUTDOWN_TIMEOUT / SHUTDOWN_WAIT_INTERVAL; dwCount > 0 && i > 0; i--)
    {
        DWORD dwRet = WaitForMultipleObjects(dwCount, hh, FALSE, SHUTDOWN_WAIT_INTERVAL);
        if (dwRet >= WAIT_OBJECT_0 + 0 && dwRet < WAIT_OBJECT_0 + dwCount)
        {
            DWORD dwIndex = dwRet - WAIT_OBJECT_0;
            DBGPUT(L"WaitForMultipleObjects: PID=%lu", ii[dwIndex]);
            CloseHandle(hh[dwIndex]);
            memmove(&hh[dwIndex + 0], &hh[dwIndex + 1], sizeof(HANDLE) * (dwCount - (dwIndex + 1)));
            memmove(&ii[dwIndex + 0], &ii[dwIndex + 1], sizeof(DWORD) * (dwCount - (dwIndex + 1)));
            dwCount--;
        }
        else if (dwRet == WAIT_TIMEOUT)
        {
            continue;
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"WaitForMultipleObjects: Failed. error=%lu", dwError);
            break;
        }
    }

    for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        if (TerminateProcess(hh[dwIndex], 127))
        {
            Debug::Put(L"TerminateProcess(PID=%lu): Done.", ii[dwIndex]);
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"TerminateProcess(PID=%lu): Failed. error=%lu", ii[dwIndex], dwError);
        }
        CloseHandle(hh[dwIndex]);
    }

    InterlockedExchange(&m_pIpc->m_dwProcessId32, 0);
    InterlockedExchange(&m_pIpc->m_dwProcessId64, 0);
}


// Builds the command line string to get RUNDLL32 to run RunServer function.
// Passes the process ID of this program to RunServer as the 1st argument.
// The path of this DLL needs to be a short path name to exclude any spaces in it.
bool Client::BuildCommandLine(HMODULE hModule, int bitness, PWCHAR pCommandLine)
{
    WCHAR szPath[MAX_PATH];
    if (GetModuleFileNameW(hModule, szPath, MAX_PATH))
    {
        DBGPUT(L"GetModuleFileName: %s", szPath);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"GetModuleFileName: Failed. error=%lu", dwError);
        return false;
    }

    if (bitness > 0 && sizeof(void*) * 8 != (size_t)bitness)
    {
        WCHAR szPath2[MAX_PATH];
        if (GetLongPathNameW(szPath, szPath2, MAX_PATH))
        {
            DBGPUT(L"GetLongPathName: %s", szPath2);
        }
        else
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"GetLongPathName: Failed. error=%lu", dwError);
            return false;
        }
        PWCHAR pFirst = wcschr(szPath2, L'\\');
        if (pFirst == NULL)
        {
            Debug::Put(L"LongPathName: Malformed. (No directory separator)");
            return false;
        }
        PWCHAR pLast = wcsrchr(pFirst + 1, L'\\');
        if (pLast == NULL)
        {
            pFirst[1] = L'\0';
        }
        else
        {
            pLast[1] = L'\0';
        }
        _snwprintf_s(szPath, _TRUNCATE, L"%s%s.dll", szPath2, bitness == 64 ? DLLNAME64 : DLLNAME32);
        DBGPUT(L"%d-bit: %s", bitness, szPath);
    }

    WCHAR szShortPath[MAX_PATH];
    if (GetShortPathNameW(szPath, szShortPath, MAX_PATH))
    {
        DBGPUT(L"GetShortPathName: %s", szShortPath);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"GetShortPathName: Failed. error=%lu", dwError);
        return false;
    }

    WCHAR szRundll[MAX_PATH];
    _snwprintf_s(szRundll, MAX_PATH, _TRUNCATE, L"%sSystem32\\rundll32.exe", (PCWSTR)KnownFolderPath(FOLDERID_Windows));

    _snwprintf_s(pCommandLine, MAX_PATH * 2, _TRUNCATE, L"%s %s,RunServer %lu", szRundll, szShortPath, GetCurrentProcessId());

    return true;
}


DWORD Client::GetBlockInput()
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::GetBlockInput");
    DBGPUT(L"Started.");
    LRESULT lRet;
    if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_GET_BLOCK_INPUT)", m_pIpc->m_hWnd64);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_GET_BLOCK_INPUT, 0, 0);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_GET_BLOCK_INPUT)", m_pIpc->m_hWnd32);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_GET_BLOCK_INPUT, 0, 0);
    }
    DBGPUT(L"Ended. return=%08lx", lRet);
    return static_cast<DWORD>(lRet);
}


DWORD Client::SetBlockInput(DWORD dwFlags)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::SetBlockInput");
    DBGPUT(L"Started. flags=%08lx", dwFlags);
    LRESULT lRet;
    if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_SET_BLOCK_INPUT,%08lx)", m_pIpc->m_hWnd64, dwFlags);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_SET_BLOCK_INPUT, dwFlags, 0);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_SET_BLOCK_INPUT,%08lx)", m_pIpc->m_hWnd32, dwFlags);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_SET_BLOCK_INPUT, dwFlags, 0);
    }
    DBGPUT(L"Ended. return=%08lx", lRet);
    return static_cast<DWORD>(lRet);
}


bool Client::StartAgent(HWND hwnd)
{
    LRESULT lRet = 0;
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::StartAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);

    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        int bitness = Platform::GetProcessBitness(dwProcessId);
        if (bitness == 64)
        {
            DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_START_AGENT,%p)", m_pIpc->m_hWnd64, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_START_AGENT, reinterpret_cast<WPARAM>(hwnd), 0);
        }
        else if (bitness == 32)
        {
            DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_START_AGENT,%p)", m_pIpc->m_hWnd32, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_START_AGENT, reinterpret_cast<WPARAM>(hwnd), 0);
        }
    }

    DBGPUT(L"Ended. return=%s", lRet ? L"true" : L"false");
    return lRet ? true : false;
}


void Client::StopAgent(HWND hwnd)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Client::StopAgent");
    DBGPUT(L"Started. hwnd=%p", hwnd);

    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        int bitness = Platform::GetProcessBitness(dwProcessId);
        if (bitness == 64)
        {
            DBGPUT(L"PostMessage(64-bit::%08lx,WM_APP_STOP_AGENT)", m_pIpc->m_hWnd64);
            PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_STOP_AGENT, reinterpret_cast<WPARAM>(hwnd), 0);
        }
        else if (bitness == 32)
        {
            DBGPUT(L"PostMessage(32-bit::%08lx,WM_APP_STOP_AGENT)", m_pIpc->m_hWnd32);
            PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_STOP_AGENT, reinterpret_cast<WPARAM>(hwnd), 0);
        }
    }
    else
    {
        if (m_pIpc->m_hWnd64)
        {
            DBGPUT(L"PostMessage(64-bit::%08lx,WM_APP_STOP_AGENT)", m_pIpc->m_hWnd64);
            PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_STOP_AGENT, 0, 0);
        }
        DBGPUT(L"PostMessage(32-bit::%08lx,WM_APP_STOP_AGENT)", m_pIpc->m_hWnd32);
        PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_STOP_AGENT, 0, 0);
    }

    DBGPUT(L"Ended.");
}


DWORD Client::GetFlags(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::GetFlags");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    LRESULT lRet = (LRESULT)-1;
    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        int bitness = Platform::GetProcessBitness(dwProcessId);
        if (bitness == 64)
        {
            DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_GET_FLAGS,%p)", m_pIpc->m_hWnd64, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_GET_FLAGS, reinterpret_cast<WPARAM>(hwnd), 0);
        }
        else if (bitness == 32)
        {
            DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_GET_FLAGS,%p)", m_pIpc->m_hWnd32, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_GET_FLAGS, reinterpret_cast<WPARAM>(hwnd), 0);
        }
    }
    else if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_GET_FLAGS)", m_pIpc->m_hWnd64);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_GET_FLAGS, 0, 0);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_GET_FLAGS)", m_pIpc->m_hWnd32);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_GET_FLAGS, 0, 0);
    }
    DBGPUT(L"Ended. return=%08lx", lRet);
    return static_cast<DWORD>(lRet);
}


DWORD Client::SetFlags(HWND hwnd, DWORD dwFlags)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::SetFlags");
    DBGPUT(L"Started. hwnd=%p set=%08lx", hwnd, dwFlags);
    LRESULT lRet = (LRESULT)-1;
    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        int bitness = Platform::GetProcessBitness(dwProcessId);
        if (bitness == 64)
        {
            DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_SET_IME_FLAGS,%p,%08lx)", m_pIpc->m_hWnd64, hwnd, dwFlags);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_SET_FLAGS, reinterpret_cast<WPARAM>(hwnd), dwFlags);
        }
        else if (bitness == 32)
        {
            DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_SET_IME_FLAGS,%p,%08lx)", m_pIpc->m_hWnd32, hwnd, dwFlags);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_SET_FLAGS, reinterpret_cast<WPARAM>(hwnd), dwFlags);
        }
    }
    else if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_SET_IME_FLAGS,0,%08lx)", m_pIpc->m_hWnd64, dwFlags);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_SET_FLAGS, 0, dwFlags);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_SET_IME_FLAGS,0,%08lx)", m_pIpc->m_hWnd32, dwFlags);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_SET_FLAGS, 0, dwFlags);
    }
    DBGPUT(L"Ended. return=%08lx", lRet);
    return static_cast<DWORD>(lRet);
}


LANGID Client::GetPreferredKeyboardLayout()
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::GetPreferredKeyboardLayout");
    DBGPUT(L"Started.");
    LANGID langId;
    if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_GET_KBD_LAYOUT)", m_pIpc->m_hWnd64);
        langId = (LANGID)SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_GET_KBD_LAYOUT, 0, 0);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_GET_KBD_LAYOUT)", m_pIpc->m_hWnd32);
        langId = (LANGID)SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_GET_KBD_LAYOUT, 0, 0);
    }
    DBGPUT(L"Ended. return=%04x", langId);
    return langId;

}


LANGID Client::SetPreferredKeyboardLayout(LANGID langId)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::SetPreferredKeyboardLayout");
    DBGPUT(L"Started. lang=%04x", langId);
    LRESULT lRet;
    if (m_pIpc->m_hWnd64)
    {
        DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_SET_KBD_LAYOUT,%04x)", m_pIpc->m_hWnd64, langId);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_SET_KBD_LAYOUT, langId, 0);
    }
    else
    {
        DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_SET_KBD_LAYOUT,%04x)", m_pIpc->m_hWnd32, langId);
        lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_SET_KBD_LAYOUT, langId, 0);
    }
    DBGPUT(L"Ended. return=%04lx", lRet);
    return static_cast<LANGID>(lRet);
}


void Client::SetToggleSequence(PCWSTR psz)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::SetToggleSequence");
    DBGPUT(L"Started. sequence=%s", psz);
    m_pIpc->m_ToggleSequece.Clear();
    m_pIpc->m_ToggleSequece.Parse(psz);
    DBGPUT(L"Ended.");
}


DWORD Client::GetState(HWND hwnd)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Client::GetState");
    DBGPUT(L"Started. hwnd=%p", hwnd);
    LRESULT lRet = 0;
    if (hwnd != NULL)
    {
        DWORD dwProcessId = 0;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        DBGPUT(L"TID=%lu PID=%lu", dwThreadId, dwProcessId);
        int bitness = Platform::GetProcessBitness(dwProcessId);
        if (bitness == 64)
        {
            DBGPUT(L"SendMessage(64-bit::%08lx,WM_APP_GET_CURRENT_STATE,%p)", m_pIpc->m_hWnd64, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd64)), WM_APP_GET_CURRENT_STATE, reinterpret_cast<WPARAM>(hwnd), 0);
        }
        else if (bitness == 32)
        {
            DBGPUT(L"SendMessage(32-bit::%08lx,WM_APP_GET_CURRENT_STATE,%p)", m_pIpc->m_hWnd32, hwnd);
            lRet = SendMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpc->m_hWnd32)), WM_APP_GET_CURRENT_STATE, reinterpret_cast<WPARAM>(hwnd), 0);
        }
    }
    DBGPUT(L"Ended. return=%08lx", lRet);
    return static_cast<DWORD>(lRet);
}
