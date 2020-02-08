#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class KeyboardBitMap
    {
    public:

        inline KeyboardBitMap();
        inline KeyboardBitMap(const KeyboardBitMap&);
        inline KeyboardBitMap& operator =(const KeyboardBitMap&);
        inline void Clear();
        inline bool IsZero() const;
        inline bool IsSet(const KeyboardBitMap&) const;
        inline void Set(DWORD);
        inline void Reset(DWORD);
        int Parse(PCWSTR, int* = NULL);

    private:

        LONGLONG m_pressed[4];
    };

    inline KeyboardBitMap::KeyboardBitMap()
    {
        Clear();
    }

    inline KeyboardBitMap::KeyboardBitMap(const KeyboardBitMap& rhs)
    {
        m_pressed[0] = rhs.m_pressed[0];
        m_pressed[1] = rhs.m_pressed[1];
        m_pressed[2] = rhs.m_pressed[2];
        m_pressed[3] = rhs.m_pressed[3];
    }

    inline KeyboardBitMap& KeyboardBitMap::operator =(const KeyboardBitMap& rhs)
    {
        m_pressed[0] = rhs.m_pressed[0];
        m_pressed[1] = rhs.m_pressed[1];
        m_pressed[2] = rhs.m_pressed[2];
        m_pressed[3] = rhs.m_pressed[3];
        return *this;
    }

    inline void KeyboardBitMap::Clear()
    {
        m_pressed[0] = 0LL;
        m_pressed[1] = 0LL;
        m_pressed[2] = 0LL;
        m_pressed[3] = 0LL;
    }

    inline bool KeyboardBitMap::IsZero() const
    {
        return (m_pressed[0] | m_pressed[1] | m_pressed[2] | m_pressed[3]) == 0LL;
    }

    inline bool KeyboardBitMap::IsSet(const KeyboardBitMap& rhs) const
    {
        return (m_pressed[0] & rhs.m_pressed[0]) == rhs.m_pressed[0] &&
               (m_pressed[1] & rhs.m_pressed[1]) == rhs.m_pressed[1] &&
               (m_pressed[2] & rhs.m_pressed[2]) == rhs.m_pressed[2] &&
               (m_pressed[3] & rhs.m_pressed[3]) == rhs.m_pressed[3];
    }

    inline void KeyboardBitMap::Set(DWORD vk)
    {
        if (vk < 256)
        {
            m_pressed[vk / 64] |= (1LL << (vk % 64));
        }
    }

    inline void KeyboardBitMap::Reset(DWORD vk)
    {
        if (vk < 256)
        {
            m_pressed[vk / 64] &= ~(1LL << (vk % 64));
        }
    }
}
