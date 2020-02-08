#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class Platform
    {
    public:

        static bool Is64bit();
        static bool Is32bit();
        static bool Is64bitProcess();
        static bool Is32bitProcess();
        static bool IsWow64Process();
        static DWORD Is64bitProcess(HANDLE, bool&);
        static DWORD Is32bitProcess(HANDLE, bool&);
        static DWORD IsWow64Process(HANDLE, bool&);
        static int GetProcessBitness(DWORD);
    };
}
