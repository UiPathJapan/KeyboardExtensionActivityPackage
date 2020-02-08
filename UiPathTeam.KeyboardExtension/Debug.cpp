#include "pch.h"
#include "DebugHeader.h"
#include "Debug.h"


using namespace UiPathTeam;


void Debug::Put(PCWSTR pszFormat, ...)
{
    va_list argList;
    va_start(argList, pszFormat);
    VPut(pszFormat, argList);
    va_end(argList);
}


void Debug::VPut(PCWSTR pszFormat, va_list argList)
{
    va_list argList2;
    va_copy(argList2, argList);
    int len2 = _vscwprintf(pszFormat, argList2);
    va_end(argList2);
    if (len2 > 0)
    {
        DebugHeader& dh = DebugHeader::Instance();
        PCWSTR pszHeader = dh.Get();
        PCWSTR pszSeparator = dh.GetSeparator();
        int len1 = pszHeader ? (int)(wcslen(pszHeader) + wcslen(pszSeparator)) : 0;
        size_t size = len1 + len2 + 2;
        PWCHAR pBuf;
        WCHAR buf[512];
        if (size <= sizeof(buf))
        {
            pBuf = buf;
        }
        else
        {
            pBuf = (PWCHAR)malloc(size * sizeof(WCHAR));
            if (pBuf == NULL)
            {
                pBuf = buf;
                size = sizeof(buf);
            }
        }
        int len = 0;
        if (pszHeader != NULL)
        {
            len += _snwprintf_s(pBuf, size, _TRUNCATE, L"%s%s", pszHeader, pszSeparator);
        }
        va_copy(argList2, argList);
        len += _vsnwprintf_s(pBuf + len, size - len, _TRUNCATE, pszFormat, argList2);
        va_end(argList2);
        if (len + 2 <= (int)size && pBuf[len - 1] != L'\n')
        {
            pBuf[len + 0] = L'\n';
            pBuf[len + 1] = L'\0';
        }
        OutputDebugStringW(pBuf);
        if (pBuf != buf)
        {
            free(pBuf);
        }
    }
    else if (len2 < 0)
    {
        Put(L"!FORMAT ERROR!");
    }
}


Debug::Function::Function(PCWSTR pszFormat, ...)
{
    va_list argList;
    va_start(argList, pszFormat);
    DebugHeader::Instance().Push(pszFormat, argList);
    va_end(argList);
}


Debug::Function::~Function()
{
    DebugHeader::Instance().Pop();
}
