#pragma once


#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Ipc.h"
#include <map>


namespace UiPathTeam
{
    namespace KeyboardExtension
    {
        class AgentHook
        {
        public:

            AgentHook(DWORD, HMODULE, DWORD);
            ~AgentHook();

            AgentIpcPtr m_pIpc;
            HHOOK m_hCallWndProcHook;
        };

        class Server
        {
        public:

            Server(HMODULE);
            ~Server();
            bool RegisterClient(DWORD);
            void Run();

        private:

            Server(const Server&) {}
            void operator =(const Server&) {}
            void Initialize();
            void Uninitialize();
            bool RegisterWndClass();
            void UnregisterWndClass();
            HWND CreateWnd();
            void MainLoop();
            void OnCreate(HWND);
            void OnDestroy();
            bool StartAgent(HWND);
            void StopAgent(HWND);
            DWORD GetFlags(HWND);
            DWORD SetFlags(HWND, DWORD);
            void NotifyAgents(WPARAM = 0, LPARAM = 0);
            void InstallKeybdHook();
            void InstallMouseHook();
            bool InstallCallWndProcHook(DWORD);
            void UninstallKeybdHook();
            void UninstallMouseHook();
            void UninstallCallWndProcHook(std::map<DWORD, AgentHook*>::iterator&);
            void UninstallAllCallWndProcHooks();
            inline bool OnKeybdHook(UINT, const KBDLLHOOKSTRUCT*);
            inline bool OnMouseHook(UINT, const MSLLHOOKSTRUCT*);

            static LRESULT CALLBACK WindowProc(
                _In_ HWND   hwnd,
                _In_ UINT   uMsg,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam
            );

            static LRESULT CALLBACK LowLevelKeyboardHook(
                _In_ int    nCode,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam
            );

            static LRESULT CALLBACK LowLevelMouseHook(
                _In_ int    nCode,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam
            );

            static BOOL CALLBACK SendWakeUpMessage(
                _In_ HWND   hwnd,
                _In_ LPARAM lParam
            );

            static DWORD WINAPI WatchClientProcess(
                _In_ LPVOID lpParameter
            );

            static Server* m_pInstance;

            HMODULE m_hModule;
            IpcPtr<ServerIpc> m_pServerIpc;
            IpcPtr<DesktopIpc> m_pDesktopIpc;
            HANDLE m_hExitEvent;
            HANDLE m_hClientProcessWatcher;
            HANDLE m_hClientProcess;
            HHOOK m_hKeybdHook;
            HHOOK m_hMouseHook;
            std::map<DWORD, AgentHook*> m_AgentMap;
            ATOM m_atomWndClass;
            HWND m_hwndMessage;
            KeyboardBitMap m_pressedKeys;
            bool m_bBlockKeybd;
            bool m_bBlockMouse;
        };

        inline bool Server::OnKeybdHook(UINT uMsg, const KBDLLHOOKSTRUCT* pKHS)
        {
            if ((pKHS->flags & (LLKHF_INJECTED | LLKHF_LOWER_IL_INJECTED)) != 0)
            {
                //DBGPUT(L"Injected.");
                return false;
            }
            if ((pKHS->flags & LLKHF_UP) == 0)
            {
                m_pressedKeys.Set(pKHS->vkCode);
                if (!m_pServerIpc->m_ToggleSequece.IsZero() && m_pressedKeys.IsSet(m_pServerIpc->m_ToggleSequece))
                {
                    DBGPUT(L"Toggle key sequence detected.");
                    if (m_pServerIpc->CanChange())
                    {
                        m_pDesktopIpc->m_Paused ^= 1;
                        NotifyAgents();
                    }
                }
                if (m_bBlockKeybd && !m_pDesktopIpc->m_Paused)
                {
                    //DBGPUT(L"Blocked.");
                    return true;
                }
            }
            else
            {
                m_pressedKeys.Reset(pKHS->vkCode);
            }
            //DBGPUT(L"Allowed.");
            return false;
        }

        inline bool Server::OnMouseHook(UINT uMsg, const MSLLHOOKSTRUCT* pMHS)
        {
            if ((pMHS->flags & (LLMHF_INJECTED | LLMHF_LOWER_IL_INJECTED)) != 0)
            {
                //DBGPUT(L"Injected.");
                return false;
            }
            if (m_bBlockMouse && !m_pDesktopIpc->m_Paused)
            {
                //DBGPUT(L"Blocked.");
                return true;
            }
            //DBGPUT(L"Allowed.");
            return false;
        }
    }
}


#define WINDOW_CLASSNAME L"KeyboardMouseHookClass"
#define WINDOW_NAME L"KeyboardMouseHookWindow"

#define WAKEUP_TIMEOUT 1000
