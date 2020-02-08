#include "pch.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.Server.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


LRESULT CALLBACK Server::LowLevelKeyboardHook(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::LowLevelKeyboardHook%d", Platform::Is64bitProcess() ? 64 : 32);
    if (nCode < 0)
    {
        DBGPUT(L"nCode<0");
    }
    else if (nCode == HC_ACTION)
    {
        if (m_pInstance->OnKeybdHook(static_cast<UINT>(wParam), reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)))
        {
            return TRUE;
        }
    }
    else
    {
        DBGPUT(L"nCode>0");
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK Server::LowLevelMouseHook(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Server::LowLevelMouseHook%d", Platform::Is64bitProcess() ? 64 : 32);
    if (nCode < 0)
    {
        DBGPUT(L"nCode<0");
    }
    else if (nCode == HC_ACTION)
    {
        if (m_pInstance->OnMouseHook(static_cast<UINT>(wParam), reinterpret_cast<MSLLHOOKSTRUCT*>(lParam)))
        {
            return TRUE;
        }
    }
    else
    {
        DBGPUT(L"nCode>0");
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
