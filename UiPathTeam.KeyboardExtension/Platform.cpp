#include "pch.h"
#include "Debug.h"
#include "Platform.h"


using namespace UiPathTeam;


bool Platform::Is64bit()
{
    return Is64bitProcess() || IsWow64Process();
}


bool Platform::Is32bit()
{
    return Is32bitProcess() && !IsWow64Process();
}


bool Platform::Is64bitProcess()
{
    return sizeof(void*) == 8;
}


bool Platform::Is32bitProcess()
{
    return sizeof(void*) == 4;
}


bool Platform::IsWow64Process()
{
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS fnIsWow64Process;

    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"), "IsWow64Process");

    if (fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"IsWow64Process(CurrentProcess): Failed. error=%lu", dwError);
            throw std::runtime_error("IsWow64Process failed with current process handle.");
        }
    }

    return bIsWow64;
}


DWORD Platform::Is64bitProcess(HANDLE hProcess, bool& bRet)
{
    if (Is32bit())
    {
        bRet = false;
        return ERROR_SUCCESS;
    }
    else
    {
        DWORD dwError = IsWow64Process(hProcess, bRet);
        if (dwError == ERROR_SUCCESS)
        {
            bRet = !bRet;
        }
        return dwError;
    }
}


DWORD Platform::Is32bitProcess(HANDLE hProcess, bool& bRet)
{
    if (Is32bit())
    {
        bRet = true;
        return ERROR_SUCCESS;
    }
    else
    {
        return IsWow64Process(hProcess, bRet);
    }
}


DWORD Platform::IsWow64Process(HANDLE hProcess, bool& bRet)
{
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS fnIsWow64Process;

    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"), "IsWow64Process");

    if (fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(hProcess, &bIsWow64))
        {
            DWORD dwError = GetLastError();
            Debug::Put(L"IsWow64Process: Failed. error=%lu", dwError);
            if (dwError == ERROR_SUCCESS)
            {
                throw std::runtime_error("IsWow64Process failed with successful status.");
            }
            return dwError;
        }
        bRet = bIsWow64 ? true : false;
        return ERROR_SUCCESS;
    }
    else
    {
        bRet = false;
        return ERROR_SUCCESS;
    }
}


int Platform::GetProcessBitness(DWORD dwProcessId)
{
    int bitness = 0;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (hProcess != NULL)
    {
        DBGPUT(L"OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,%lu): return=%p", dwProcessId, hProcess);
        bool bIs64bit;
        DWORD dwRet = Platform::Is64bitProcess(hProcess, bIs64bit);
        if (dwRet == ERROR_SUCCESS)
        {
            DBGPUT(L"Platform::Is64bitProcess: return=%s", bIs64bit ? L"true" : L"false");
            bitness = bIs64bit ? 64 : 32;
        }
        else
        {
            DBGPUT(L"Platform::Is64bitProcess: Failed. error=%lu", dwRet);
        }
        CloseHandle(hProcess);
        DBGPUT(L"CloseHandle(%p)", hProcess);
    }
    else
    {
        DWORD dwError = GetLastError();
        DBGPUT(L"OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,%lu): Failed. error=%lu", dwProcessId, dwError);
    }
    return bitness;
}
