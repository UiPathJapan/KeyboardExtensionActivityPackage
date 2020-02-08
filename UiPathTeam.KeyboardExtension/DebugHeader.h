#pragma once


#include <Windows.h>
#include <stdarg.h>
#include <stack>


namespace UiPathTeam
{
    class DebugHeader
    {
    public:

        static DebugHeader& Instance();
        static void Destroy();

        ~DebugHeader();
        PCWSTR Get() const;
        void Push(PCWSTR, va_list);
        void Pop();
        void Clear();
        PCWSTR GetSeparator() const;
        void SetSeparator(PCWSTR);

    private:

        DebugHeader();
        DebugHeader(const DebugHeader&) {}
        void operator =(const DebugHeader&) {}

        std::stack<PWCHAR> m_stack;
        PWSTR m_pszSeparator;
    };
}
