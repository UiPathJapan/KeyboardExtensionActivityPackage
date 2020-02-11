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
            DWORD GetBlockInput();
            DWORD SetBlockInput(DWORD);
            bool StartAgent(HWND);
            void StopAgent(HWND);
            DWORD GetFlags(HWND);
            DWORD SetFlags(HWND, DWORD);
            LANGID GetPreferredKeyboardLayout();
            LANGID SetPreferredKeyboardLayout(LANGID);
            void SetToggleSequence(PCWSTR);
            DWORD SetState(HWND, DWORD, DWORD);

            static Client* m_pInstance;

        private:

            Client(const Client&) {}
            void operator =(const Client&) {}
            bool StartServer(int bitness);
            bool BuildCommandLine(HMODULE hModule, int bitness, PWCHAR pCommandLine);

            HMODULE m_hModule;
            IpcPtr<ServerIpc> m_pIpc;
        };
    }
}


#define START_TIMEOUT 10000
#define SHUTDOWN_TIMEOUT 60000
#define SHUTDOWN_WAIT_INTERVAL 1000
