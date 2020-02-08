#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class LowIntegrityImpersonator
    {
    public:

        LowIntegrityImpersonator();
        ~LowIntegrityImpersonator();

    private:

        LowIntegrityImpersonator(const LowIntegrityImpersonator&) {}
        void operator =(const LowIntegrityImpersonator&) {}

        HANDLE m_hToken1;
        HANDLE m_hToken2;
        bool m_bRevert;
    };
}