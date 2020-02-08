#pragma once


#include <Windows.h>
#include <stdarg.h>


namespace UiPathTeam
{
    class Debug
    {
    public:

        static void Put(PCWSTR, ...);
        static void VPut(PCWSTR, va_list);

        class Function
        {
        public:

            Function(PCWSTR, ...);
            ~Function();

        private:

            Function(const Function&) {}
            void operator =(const Function&) {}
        };
    };
}


#ifdef _DEBUG
#define DBGPUT(...) UiPathTeam::Debug::Put(__VA_ARGS__)
#define DBGFNC(...) UiPathTeam::Debug::Function dbgfnc_v_1_noname_20180701(__VA_ARGS__)
#else //_DEBUG
#define DBGPUT(...) (void)0
#define DBGFNC(...) (void)0
#endif //_DEBUG
