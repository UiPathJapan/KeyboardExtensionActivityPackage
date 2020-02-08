#include "pch.h"
#include "SystemIdentifier.h"


using namespace UiPathTeam;


SystemIdentifier SystemIdentifier::LowIntegrity(SECURITY_MANDATORY_LABEL_AUTHORITY, SECURITY_MANDATORY_LOW_RID);
#if 0
SystemIdentifier SystemIdentifier::Everyone(SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID);
SystemIdentifier SystemIdentifier::AuthenticatedUser(SECURITY_NT_AUTHORITY, SECURITY_AUTHENTICATED_USER_RID);
SystemIdentifier SystemIdentifier::Administrators(SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
SystemIdentifier SystemIdentifier::Users(SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS);
#endif


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 1, dwSubAuth0, 0, 0, 0, 0, 0, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(1) failed.");
    }
}


#if 0
SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 2, dwSubAuth0, dwSubAuth1, 0, 0, 0, 0, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(2) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 3, dwSubAuth0, dwSubAuth1, dwSubAuth2, 0, 0, 0, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(3) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 4, dwSubAuth0, dwSubAuth1, dwSubAuth2, dwSubAuth3, 0, 0, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(4) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3, DWORD dwSubAuth4)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 5, dwSubAuth0, dwSubAuth1, dwSubAuth2, dwSubAuth3, dwSubAuth4, 0, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(5) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3, DWORD dwSubAuth4, DWORD dwSubAuth5)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 6, dwSubAuth0, dwSubAuth1, dwSubAuth2, dwSubAuth3, dwSubAuth4, dwSubAuth5, 0, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(6) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3, DWORD dwSubAuth4, DWORD dwSubAuth5, DWORD dwSubAuth6)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 7, dwSubAuth0, dwSubAuth1, dwSubAuth2, dwSubAuth3, dwSubAuth4, dwSubAuth5, dwSubAuth6, 0, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(7) failed.");
    }
}


SystemIdentifier::SystemIdentifier(SID_IDENTIFIER_AUTHORITY auth, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3, DWORD dwSubAuth4, DWORD dwSubAuth5, DWORD dwSubAuth6, DWORD dwSubAuth7)
    : m_pSid(NULL)
{
    if (!AllocateAndInitializeSid(&auth, 8, dwSubAuth0, dwSubAuth1, dwSubAuth2, dwSubAuth3, dwSubAuth4, dwSubAuth5, dwSubAuth6, dwSubAuth7, &m_pSid))
    {
        throw std::runtime_error("Allocate SID(8) failed.");
    }
}
#endif


SystemIdentifier::~SystemIdentifier()
{
    if (m_pSid)
    {
        FreeSid(m_pSid);
    }
}
