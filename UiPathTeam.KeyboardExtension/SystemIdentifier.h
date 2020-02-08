#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class SystemIdentifier
    {
    public:

        static SystemIdentifier LowIntegrity;
#if 0
        static SystemIdentifier Everyone;
        static SystemIdentifier AuthenticatedUser;
        static SystemIdentifier Administrators;
        static SystemIdentifier Users;
#endif

        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD);
#if 0
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
        SystemIdentifier(SID_IDENTIFIER_AUTHORITY, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
#endif
        ~SystemIdentifier();
        inline operator PSID() const;

    private:

        SystemIdentifier(const SystemIdentifier&) {}
        void operator =(const SystemIdentifier&) {}

        PSID m_pSid;
    };

    inline SystemIdentifier::operator PSID() const
    {
        return m_pSid;
    }
}
