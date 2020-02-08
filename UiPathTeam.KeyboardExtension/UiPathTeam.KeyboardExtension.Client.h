#pragma once


#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.h"
#include "UiPathTeam.KeyboardExtension.Ipc.h"


namespace UiPathTeam
{
    namespace KeyboardExtension
    {
        class Client
        {
        public:

            Client(HMODULE);
            ~Client();
            bool StartServer();
            void StopServer();
            bool StartAgent(HWND);
            void StopAgent(HWND);
            inline void NotifyAgents();
            inline void SetFlags(DWORD, DWORD);
            inline void SetToggleSequence(PCWSTR);
            inline void SetPreferredKeyboardLayout(LANGID);

            static Client* m_pInstance;

        private:

            Client(const Client&) {}
            void operator =(const Client&) {}
            bool StartServer(int bitness);
            bool BuildCommandLine(HMODULE hModule, int bitness, PWCHAR pCommandLine);

            HMODULE m_hModule;
            HANDLE m_hIpcMapping;
            Ipc* m_pIpcBlock;
        };

        inline void Client::NotifyAgents()
        {
            DBGPUT(L"PostMessage(64-bit::%08lx,WM_APP_NOTIFY_AGENTS)", m_pIpcBlock->m_hWnd64);
            PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpcBlock->m_hWnd64)), WM_APP_NOTIFY_AGENTS, 0, 0);
            DBGPUT(L"PostMessage(32-bit::%08lx,WM_APP_NOTIFY_AGENTS)", m_pIpcBlock->m_hWnd32);
            PostMessageW(reinterpret_cast<HWND>(static_cast<DWORD_PTR>(m_pIpcBlock->m_hWnd32)), WM_APP_NOTIFY_AGENTS, 0, 0);
        }

        inline void Client::SetFlags(DWORD dwSet, DWORD dwReset)
        {
            m_pIpcBlock->SetFlags(dwSet, dwReset);
            DBGPUT(L"m_pIpcBlock->m_dwFlags=%08lx", m_pIpcBlock->m_dwFlags);
        }

        inline void Client::SetToggleSequence(PCWSTR psz)
        {
            m_pIpcBlock->m_ToggleSequece.Clear();
            m_pIpcBlock->m_ToggleSequece.Parse(psz);
        }

        inline void Client::SetPreferredKeyboardLayout(LANGID langId)
        {
            m_pIpcBlock->m_PreferredLangId = langId;
            m_pIpcBlock->m_ForcedLangId = (LANGID)0; // reset
        }
    }
}


#define START_TIMEOUT 10000
#define SHUTDOWN_TIMEOUT 60000
#define SHUTDOWN_WAIT_INTERVAL 1000
