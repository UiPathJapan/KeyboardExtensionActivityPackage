#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Ipc.h"


#define MAPPING_NAME L"Local\\28CC044A-8F75-43F8-91C6-BF2FCBC0E848@v7"


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


Ipc* Ipc::Map(HANDLE& hMapping)
{
    LARGE_INTEGER size;
    size.QuadPart = sizeof(Ipc);
    hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, MAPPING_NAME);
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

    Ipc* pBlock = (Ipc*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
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


void Ipc::Unmap(HANDLE& hMapping)
{
    if (UnmapViewOfFile(this))
    {
        DBGPUT(L"UnmapViewOfFile(%p)", this);
    }
    else
    {
        DWORD dwError = GetLastError();
        Debug::Put(L"UnmapViewOfFile(%p): Failed. error=%lu", this, dwError);
    }

    CloseHandle2(hMapping);
}
