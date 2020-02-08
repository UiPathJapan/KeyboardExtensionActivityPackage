#pragma once


#include <Windows.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <map>


namespace UiPathTeam
{
    class VirtualKeyMap
    {
    public:

        inline VirtualKeyMap();
        inline int Find(PCWSTR psz) const;
        inline PCWSTR Find(int) const;

    private:

        VirtualKeyMap(const VirtualKeyMap&) {}
        void operator =(const VirtualKeyMap&) {}

        class Comparator
        {
        public:

            bool operator ()(PCWSTR s1, PCWSTR s2) const
            {
                return _wcsicmp(s1, s2) < 0;
            }
        };

        typedef std::map<PCWSTR, int, Comparator> TextCodeMap;
        typedef std::pair<PCWSTR, int> TextCodePair;

        TextCodeMap m_map;
    };

    inline VirtualKeyMap::VirtualKeyMap()
        : m_map()
    {
#define ADD(x) m_map.insert(TextCodePair(L#x, VK_##x))
        ADD(LBUTTON);
        ADD(RBUTTON);
        ADD(CANCEL);
        ADD(MBUTTON);
        m_map.insert(TextCodePair(L"XBUTTON1", VK_XBUTTON1));
        m_map.insert(TextCodePair(L"XBUTTON2", VK_XBUTTON2));
        ADD(BACK);
        ADD(TAB);
        ADD(CLEAR);
        ADD(RETURN);
        ADD(SHIFT);
        ADD(CONTROL);
        ADD(MENU);
        ADD(PAUSE);
        ADD(CAPITAL);
        ADD(KANA);
        ADD(JUNJA);
        ADD(FINAL);
        ADD(KANJI);
        ADD(ESCAPE);
        ADD(CONVERT);
        ADD(NONCONVERT);
        ADD(ACCEPT);
        ADD(MODECHANGE);
        ADD(SPACE);
        ADD(PRIOR);
        ADD(NEXT);
        ADD(END);
        ADD(HOME);
        ADD(LEFT);
        ADD(UP);
        ADD(RIGHT);
        ADD(DOWN);
        ADD(SELECT);
        ADD(PRINT);
        ADD(EXECUTE);
        ADD(SNAPSHOT);
        ADD(INSERT);
        m_map.insert(TextCodePair(L"DELETE", VK_DELETE));
        ADD(HELP);
        ADD(LWIN);
        ADD(RWIN);
        ADD(APPS);
        ADD(SLEEP);
        ADD(NUMPAD0);
        ADD(NUMPAD1);
        ADD(NUMPAD2);
        ADD(NUMPAD3);
        ADD(NUMPAD4);
        ADD(NUMPAD5);
        ADD(NUMPAD6);
        ADD(NUMPAD7);
        ADD(NUMPAD8);
        ADD(NUMPAD9);
        ADD(MULTIPLY);
        m_map.insert(TextCodePair(L"ADD", VK_ADD));
        ADD(SEPARATOR);
        ADD(SUBTRACT);
        m_map.insert(TextCodePair(L"DECIMAL", VK_DECIMAL));
        ADD(DIVIDE);
        ADD(F1);
        ADD(F2);
        ADD(F3);
        ADD(F4);
        ADD(F5);
        ADD(F6);
        ADD(F7);
        ADD(F8);
        ADD(F9);
        ADD(F10);
        ADD(F11);
        ADD(F12);
        ADD(F13);
        ADD(F14);
        ADD(F15);
        ADD(F16);
        ADD(F17);
        ADD(F18);
        ADD(F19);
        ADD(F20);
        ADD(F21);
        ADD(F22);
        ADD(F23);
        ADD(F24);
        ADD(NAVIGATION_VIEW);
        ADD(NAVIGATION_MENU);
        ADD(NAVIGATION_UP);
        ADD(NAVIGATION_DOWN);
        ADD(NAVIGATION_LEFT);
        ADD(NAVIGATION_RIGHT);
        ADD(NAVIGATION_ACCEPT);
        ADD(NAVIGATION_CANCEL);
        ADD(NUMLOCK);
        ADD(SCROLL);
        ADD(OEM_NEC_EQUAL);
        ADD(OEM_FJ_JISHO);
        ADD(OEM_FJ_MASSHOU);
        ADD(OEM_FJ_TOUROKU);
        ADD(OEM_FJ_LOYA);
        ADD(OEM_FJ_ROYA);
        ADD(LSHIFT);
        ADD(RSHIFT);
        ADD(LCONTROL);
        ADD(RCONTROL);
        ADD(LMENU);
        ADD(RMENU);
        ADD(BROWSER_BACK);
        ADD(BROWSER_FORWARD);
        ADD(BROWSER_REFRESH);
        ADD(BROWSER_STOP);
        ADD(BROWSER_SEARCH);
        ADD(BROWSER_FAVORITES);
        ADD(BROWSER_HOME);
        ADD(VOLUME_MUTE);
        ADD(VOLUME_DOWN);
        ADD(VOLUME_UP);
        ADD(MEDIA_NEXT_TRACK);
        ADD(MEDIA_PREV_TRACK);
        ADD(MEDIA_STOP);
        ADD(MEDIA_PLAY_PAUSE);
        ADD(LAUNCH_MAIL);
        ADD(LAUNCH_MEDIA_SELECT);
        ADD(LAUNCH_APP1);
        ADD(LAUNCH_APP2);
        ADD(OEM_1);
        ADD(OEM_PLUS);
        ADD(OEM_COMMA);
        ADD(OEM_MINUS);
        ADD(OEM_PERIOD);
        ADD(OEM_2);
        ADD(OEM_3);
        ADD(GAMEPAD_A);
        ADD(GAMEPAD_B);
        ADD(GAMEPAD_X);
        ADD(GAMEPAD_Y);
        ADD(GAMEPAD_RIGHT_SHOULDER);
        ADD(GAMEPAD_LEFT_SHOULDER);
        ADD(GAMEPAD_LEFT_TRIGGER);
        ADD(GAMEPAD_RIGHT_TRIGGER);
        ADD(GAMEPAD_DPAD_UP);
        ADD(GAMEPAD_DPAD_DOWN);
        ADD(GAMEPAD_DPAD_LEFT);
        ADD(GAMEPAD_DPAD_RIGHT);
        ADD(GAMEPAD_MENU);
        ADD(GAMEPAD_VIEW);
        ADD(GAMEPAD_LEFT_THUMBSTICK_BUTTON);
        ADD(GAMEPAD_RIGHT_THUMBSTICK_BUTTON);
        ADD(GAMEPAD_LEFT_THUMBSTICK_UP);
        ADD(GAMEPAD_LEFT_THUMBSTICK_DOWN);
        ADD(GAMEPAD_LEFT_THUMBSTICK_RIGHT);
        ADD(GAMEPAD_LEFT_THUMBSTICK_LEFT);
        ADD(GAMEPAD_RIGHT_THUMBSTICK_UP);
        ADD(GAMEPAD_RIGHT_THUMBSTICK_DOWN);
        ADD(GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
        ADD(GAMEPAD_RIGHT_THUMBSTICK_LEFT);
        ADD(OEM_4);
        ADD(OEM_5);
        ADD(OEM_6);
        ADD(OEM_7);
        ADD(OEM_8);
        ADD(OEM_AX);
        ADD(OEM_102);
        ADD(ICO_HELP);
        ADD(ICO_00);
        ADD(PROCESSKEY);
        ADD(ICO_CLEAR);
        ADD(PACKET);
        ADD(OEM_RESET);
        ADD(OEM_JUMP);
        ADD(OEM_PA1);
        ADD(OEM_PA2);
        ADD(OEM_PA3);
        ADD(OEM_WSCTRL);
        ADD(OEM_CUSEL);
        ADD(OEM_ATTN);
        ADD(OEM_FINISH);
        ADD(OEM_COPY);
        ADD(OEM_AUTO);
        ADD(OEM_ENLW);
        ADD(OEM_BACKTAB);
        ADD(ATTN);
        ADD(CRSEL);
        ADD(EXSEL);
        ADD(EREOF);
        ADD(PLAY);
        ADD(ZOOM);
        ADD(NONAME);
        ADD(PA1);
        ADD(OEM_CLEAR);
#undef ADD
        // Digits
        for (int c = 0x30; c <= 0x39; c++)
        {
            static WCHAR szBuf[0x39 - 0x30 + 1][8];
            _snwprintf_s(szBuf[c - 0x30], _TRUNCATE, L"VK_%c", c);
            m_map.insert(TextCodePair(szBuf[c - 0x30], c));
        }
        // Alphabets
        for (int c = 0x41; c <= 0x5A; c++)
        {
            static WCHAR szBuf[0x5A - 0x41 + 1][8];
            _snwprintf_s(szBuf[c - 0x41], _TRUNCATE, L"VK_%c", c);
            m_map.insert(TextCodePair(szBuf[c - 0x41], c));
        }
    }

    inline int VirtualKeyMap::Find(PCWSTR psz) const
    {
        TextCodeMap::const_iterator iter = m_map.find(psz);
        if (iter != m_map.end())
        {
            return iter->second;
        }
        else
        {
            return -1;
        }
    }

    inline PCWSTR VirtualKeyMap::Find(int code) const
    {
        for (TextCodeMap::const_iterator iter = m_map.begin(); iter != m_map.end(); iter++)
        {
            if (iter->second == code)
            {
                return iter->first;
            }
        }
        return NULL;
    }
}
