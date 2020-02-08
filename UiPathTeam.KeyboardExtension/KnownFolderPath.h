#pragma once


#include <Windows.h>
#include <Shlobj.h>


namespace UiPathTeam
{
    // Note: The path ends with a folder separator.
    class KnownFolderPath
    {
    public:

        static PWSTR Get(const KNOWNFOLDERID&, DWORD = 0);

        KnownFolderPath(const KNOWNFOLDERID&, DWORD = 0);
        KnownFolderPath(const KnownFolderPath&);
        ~KnownFolderPath();
        inline operator PCWSTR() const;
        void operator =(const KnownFolderPath&);

    private:

        PWSTR m_psz;
    };

    inline KnownFolderPath::operator PCWSTR() const
    {
        return m_psz;
    }
}
