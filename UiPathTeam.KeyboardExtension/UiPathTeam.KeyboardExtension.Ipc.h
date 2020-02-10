#pragma once


#include "KeyboardBitMap.h"


#define MIN_MILLISECONDS_TO_CHANGE  1000ULL


namespace UiPathTeam
{
    namespace KeyboardExtension
    {
        template<class T>
        class IpcPtr
        {
        public:

            IpcPtr()
                : m_p(0)
                , m_h(0)
            {
            }

            ~IpcPtr()
            {
                Unmap();
            }

            bool Map()
            {
                m_p = T::Map(m_h);
                return m_p != 0;
            }

            void Unmap()
            {
                if (m_p)
                {
                    m_p->Unmap(m_h);
                    m_p = 0;
                }
            }

            operator bool()
            {
                return m_p != 0;
            }

            T* operator ->()
            {
                return m_p;
            }

        private:

            IpcPtr(const IpcPtr&) {}
            void operator =(const IpcPtr&) {}

            T* m_p;
            HANDLE m_h;
        };

        class ServerIpc
        {
        public:

            static ServerIpc* Map(HANDLE& hMapping);
            void Unmap(HANDLE&);
            void Clear();
            inline bool CanChange();

            DWORD m_dwProcessId32;
            DWORD m_dwProcessId64;
            DWORD m_hWnd32;
            DWORD m_hWnd64;
            KeyboardBitMap m_ToggleSequece;
            ULONGLONG m_LastProcessed;

        private:

            ServerIpc() {}
            ServerIpc(const ServerIpc&) {}
            void operator =(const ServerIpc&) {}
        };

        // This prevents the frequent toggle changes.
        inline bool ServerIpc::CanChange()
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

        union KeyboardLayoutSetting
        {
            struct
            {
                LANGID m_PreferredLangId;
                LANGID m_SelectedLangId;
            };
            DWORD m_LangIds;
        };

        class DesktopIpc
        {
        public:

            static DesktopIpc* Map(HANDLE&);
            void Unmap(HANDLE&);
            void Clear();

            UINT m_WM_AGENT_WAKEUP;
            BYTE m_Paused;
            DWORD m_dwFlags;
            KeyboardLayoutSetting m_KeyboardLayoutSetting;
            LANGID m_LastLangId;

        private:

            DesktopIpc() {}
            DesktopIpc(const DesktopIpc&) {}
            void operator =(const DesktopIpc&) {}
        };

        class AgentIpc
        {
        public:

            static AgentIpc* Map(HANDLE&, DWORD);
            void Unmap(HANDLE&);
            void Clear(DWORD, DWORD);

            DWORD m_dwThreadId;
            DWORD m_dwFlags;
            LANGID m_LangId;
            LONG m_KeyboardOpenClose;
            LONG m_InputModeConversion;

        private:

            AgentIpc() {}
            AgentIpc(const AgentIpc&) {}
            void operator =(const AgentIpc&) {}
        };

        class AgentIpcPtr
        {
        public:

            AgentIpcPtr()
                : m_p(0)
                , m_h(0)
            {
            }

            ~AgentIpcPtr()
            {
                Unmap();
            }

            bool Map(DWORD dwThreadId)
            {
                m_p = AgentIpc::Map(m_h, dwThreadId);
                return m_p != 0;
            }

            void Unmap()
            {
                if (m_p)
                {
                    m_p->Unmap(m_h);
                    m_p = 0;
                }
            }

            operator bool()
            {
                return m_p != 0;
            }

            AgentIpc* operator ->()
            {
                return m_p;
            }

        private:

            AgentIpcPtr(const AgentIpcPtr&) {}
            void operator =(const AgentIpcPtr&) {}

            AgentIpc* m_p;
            HANDLE m_h;
        };
    }
}
