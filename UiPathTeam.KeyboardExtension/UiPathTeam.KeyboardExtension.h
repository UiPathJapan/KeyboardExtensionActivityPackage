#pragma once


#define WM_APP_GET_BLOCK_INPUT      (WM_APP+0)
#define WM_APP_SET_BLOCK_INPUT      (WM_APP+1)
#define WM_APP_START_AGENT          (WM_APP+2)
#define WM_APP_STOP_AGENT           (WM_APP+3)
#define WM_APP_GET_FLAGS            (WM_APP+4)
#define WM_APP_SET_FLAGS            (WM_APP+5)
#define WM_APP_GET_KBD_LAYOUT       (WM_APP+6)
#define WM_APP_SET_KBD_LAYOUT       (WM_APP+7)
#define WM_APP_GET_CURRENT_STATE    (WM_APP+8)


#define AGENT_INITIALIZE        1
#define AGENT_UNINITIALIZE      2
#define AGENT_GETSTATE          3


#define BLOCK_KEYBD         (1UL<<0)                // blocks physical keyboard input
#define BLOCK_MOUSE         (1UL<<1)                // blocks physical mouse input
#define BLOCK_FLAGS(k,m)    (((k)<<0)|((m)<<1))


#define RESET_IME           (1UL<<0)                // turns off IME
#define SET_IME             (1UL<<1)                // turns on IME


#define DLLNAME32 L"UiPathTeam.KeyboardExtension32"
#define DLLNAME64 L"UiPathTeam.KeyboardExtension64"
