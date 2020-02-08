#pragma once


#include "KeyboardBitMap.h"


#define FLAG_DISABLE_KEYBD  (1UL <<  0) // blocks physical keyboard input
#define FLAG_DISABLE_MOUSE  (1UL <<  1) // blocks physical mouse input
#define FLAG_DISABLE_IME    (1UL <<  2) // turns off conversion mode of IME
#define FLAG_FORCE_LAYOUT   (1UL <<  3) // changes keyboard layout to the preferred one if available


#define MIN_MILLISECONDS_TO_CHANGE  1000ULL


namespace UiPathTeam
{
    namespace KeyboardExtension
    {
        class Ipc
        {
        public:

            static Ipc* Map(HANDLE&);
            void Unmap(HANDLE&);

            DWORD m_dwFlags;
            DWORD m_dwProcessId32;
            DWORD m_dwProcessId64;
            DWORD m_hWnd32;
            DWORD m_hWnd64;
            UINT m_WM_AGENT_WAKEUP;
            KeyboardBitMap m_ToggleSequece;
            ULONGLONG m_LastProcessed;
            DWORD m_dwFlagsToggled;
            LANGID m_PreferredLangId;
            LANGID m_LastLangId;
            LANGID m_ForcedLangId;

            inline void Clear();
            inline void SetFlags(DWORD, DWORD = 0);
            inline void ToggleFlags();
            inline bool CanChange();

        private:

            Ipc() {}
            Ipc(const Ipc&) {}
            void operator =(const Ipc&) {}
        };

        inline void Ipc::Clear()
        {
            m_dwFlags = 0;
            m_dwProcessId32 = 0;
            m_dwProcessId64 = 0;
            m_hWnd32 = 0;
            m_hWnd64 = 0;
            m_WM_AGENT_WAKEUP = RegisterWindowMessageW(L"WM_AGENT_WAKEUP");
            m_ToggleSequece.Clear();
            m_LastProcessed = 0;
            m_dwFlagsToggled = 0;
            m_PreferredLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            m_LastLangId = 0;
            m_ForcedLangId = 0;
        }

        inline void Ipc::SetFlags(DWORD dwSet, DWORD dwReset)
        {
            for (;;)
            {
                DWORD dwFlags1 = m_dwFlags;
                DWORD dwFlags2 = dwFlags1;
                dwFlags2 |= dwSet;
                dwFlags2 &= ~dwReset;
                if (InterlockedCompareExchange(&m_dwFlags, dwFlags2, dwFlags1) == dwFlags1)
                {
                    m_LastProcessed = 0;
                    m_dwFlagsToggled = 0;
                    break;
                }
            }
        }

        inline void Ipc::ToggleFlags()
        {
            m_dwFlagsToggled = InterlockedExchange(&m_dwFlags, m_dwFlagsToggled);
        }

        // This prevents the frequent toggle changes.
        inline bool Ipc::CanChange()
        {
            static LARGE_INTEGER freq; // initialized by zeroes
            if (freq.QuadPart == 0)
            {
                QueryPerformanceFrequency(&freq);
            }
            ULONGLONG last = m_LastProcessed;
            LARGE_INTEGER current;
            QueryPerformanceCounter(&current);
            if (last + (MIN_MILLISECONDS_TO_CHANGE * freq.QuadPart / 1000ULL) <= static_cast<ULONGLONG>(current.QuadPart))
            {
                if (InterlockedCompareExchange(&m_LastProcessed, current.QuadPart, last) == last)
                {
                    // semi-guarantees the exclusive change.
                    return true;
                }
            }
            return false;
        }
    }
}


#define AGENT_ENABLED   1
#define AGENT_DISABLED  2
