#include "pch.h"
#include "Debug.h"
#include "VirtualKeyMap.h"
#include "KeyboardBitMap.h"


#define DELIMITER L"+"


using namespace UiPathTeam;


static VirtualKeyMap s_map;


static PWSTR Trim(PWSTR psz)
{
    PWCHAR pR = psz;
    while (iswspace(*pR))
    {
        pR++;
    }
    PWCHAR pStart = pR;
    PWCHAR pEnd = NULL;
    while (*pR)
    {
        if (iswspace(*pR))
        {
            if (pEnd == NULL)
            {
                pEnd = pR;
            }
        }
        else
        {
            pEnd = NULL;
        }
        pR++;
    }
    if (pEnd != NULL)
    {
        *pEnd = L'\0';
    }
    return pStart;
}


int KeyboardBitMap::Parse(PCWSTR psz, int* piErrors)
{
    DBGFNC(L"KeyboardBitMap::Parse");
    DBGPUT(L"Started.");
    DBGPUT(psz != NULL ? L"input=\"%s\"" : L"input=NULL", psz);
    if (psz == NULL || *psz == L'\0')
    {
        if (piErrors != NULL)
        {
            *piErrors = 0;
        }
        DBGPUT(L"Ended. parsed=0 errors=0");
        return 0;
    }
    PWSTR pszCopy = _wcsdup(psz);
    if (pszCopy == NULL)
    {
        throw std::bad_alloc();
    }
    int iParsed = 0;
    int iErrors = 0;
    PWSTR context = NULL;
    PWSTR pszNext = wcstok_s(pszCopy, DELIMITER, &context);
    while (pszNext != NULL)
    {
        pszNext = Trim(pszNext);
        int code = s_map.Find(pszNext);
        DBGPUT(L"next=\"%s\" code=%d", pszNext, code);
        if (0 <= code && code <= 255)
        {
            Set(code);
            iParsed++;
        }
        else
        {
            PWSTR pStop = NULL;
            code = wcstoul(pszNext, &pStop, 0);
            if (*pStop != L'\0' || pszNext == pStop)
            {
                Set(code);
                iParsed++;
            }
            else
            {
                iErrors++;
            }
        }
        pszNext = wcstok_s(NULL, DELIMITER, &context);
    }
    if (piErrors != NULL)
    {
        *piErrors = iErrors;
    }
    free(pszCopy);
    DBGPUT(L"Ended. parsed=%d errors=%d", iParsed, iErrors);
    return iParsed;
}
