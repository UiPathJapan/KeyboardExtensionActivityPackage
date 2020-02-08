#include "pch.h"
#include "Debug.h"
#include "SystemIdentifier.h"
#include "LowIntegrityImpersonator.h"


using namespace UiPathTeam;


LowIntegrityImpersonator::LowIntegrityImpersonator()
    : m_hToken1(NULL)
    , m_hToken2(NULL)
    , m_bRevert(false)
{
    DBGFNC(L"LowIntegrityImpersonator::ctor");
    DBGPUT(L"Started.");

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &m_hToken1))
    {
        DBGPUT(L"OpenProcessToken");
    }
    else
    {
        DBGPUT(L"OpenProcessToken: Failed. error=%lu", GetLastError());
        return;
    }

    if (DuplicateTokenEx(m_hToken1, 0, NULL, SecurityImpersonation, TokenPrimary, &m_hToken2))
    {
        DBGPUT(L"DuplicateTokenEx(SecurityImpersonation,TokenPrimary)");
    }
    else
    {
        DBGPUT(L"DuplicateTokenEx(SecurityImpersonation,TokenPrimary): Failed. error=%lu", GetLastError());
        return;
    }

    TOKEN_MANDATORY_LABEL tml = { 0 };
    tml.Label.Attributes = SE_GROUP_INTEGRITY;
    tml.Label.Sid = SystemIdentifier::LowIntegrity;
    if (SetTokenInformation(m_hToken2, TokenIntegrityLevel, &tml, sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(tml.Label.Sid)))
    {
        DBGPUT(L"SetTokenInformation(TokenIntegrityLevel)");
    }
    else
    {
        DBGPUT(L"SetTokenInformation(TokenIntegrityLevel): Failed. error=%lu", GetLastError());
        return;
    }

    if (ImpersonateLoggedOnUser(m_hToken2))
    {
        DBGPUT(L"ImpersonateLoggedOnUser");
        m_bRevert = true;
    }
    else
    {
        DBGPUT(L"ImpersonateLoggedOnUser: Failed. error=%lu", GetLastError());
    }

    DBGPUT(L"Ended.");
}


LowIntegrityImpersonator::~LowIntegrityImpersonator()
{
    DBGFNC(L"LowIntegrityImpersonator::dtor");
    DBGPUT(L"Started.");

    if (m_bRevert)
    {
        if (RevertToSelf())
        {
            DBGPUT(L"RevertToSelf");
        }
        else
        {
            DBGPUT(L"RevertToSelf: Failed. error=%lu", GetLastError());
        }
    }

    if (m_hToken2 != NULL)
    {
        if (CloseHandle(m_hToken2))
        {
            DBGPUT(L"CloseHandle(%p)", m_hToken2);
        }
        else
        {
            DBGPUT(L"CloseHandle(%p): Failed. error=%lu", m_hToken2, GetLastError());
        }
    }

    if (m_hToken1 != NULL)
    {
        if (CloseHandle(m_hToken1))
        {
            DBGPUT(L"CloseHandle(%p)", m_hToken1);
        }
        else
        {
            DBGPUT(L"CloseHandle(%p): Failed. error=%lu", m_hToken1, GetLastError());
        }
    }

    DBGPUT(L"Ended.");
}
