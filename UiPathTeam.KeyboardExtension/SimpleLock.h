#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class SimpleLock
    {
    public:

        typedef ULONG Type; // Initialize values of this type by zero

        inline SimpleLock(Type&);
        inline ~SimpleLock();

    private:

        SimpleLock(const SimpleLock& src) : m_value(src.m_value) {}
        void operator =(const SimpleLock&) {}

        volatile Type& m_value;
    };

    inline SimpleLock::SimpleLock(SimpleLock::Type& value)
        : m_value(value)
    {
        while (InterlockedCompareExchange(&m_value, 1, 0))
        {
            Sleep(0);
        }
    }

    inline SimpleLock::~SimpleLock()
    {
        InterlockedExchange(&m_value, 0);
    }
}
