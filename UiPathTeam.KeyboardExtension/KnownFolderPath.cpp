#include "pch.h"
#include "KnownFolderPath.h"


using namespace UiPathTeam;


PWSTR KnownFolderPath::Get(const KNOWNFOLDERID& rid, DWORD dwFlags)
{
    PWSTR psz2;
    PWSTR psz1 = NULL;
    HRESULT hr = SHGetKnownFolderPath(rid, dwFlags, NULL, &psz1);
    if (hr == S_OK)
    {
        size_t len = wcslen(psz1);
        psz2 = (PWSTR)malloc((len + 2) * sizeof(WCHAR));
        if (!psz2)
        {
            CoTaskMemFree(psz1);
            throw std::bad_alloc();
        }
        memcpy(psz2, psz1, len * sizeof(WCHAR));
        if (len > 0 && psz1[len - 1] != L'\\')
        {
            psz2[len + 0] = L'\\';
            psz2[len + 1] = L'\0';
        }
        else
        {
            psz2[len] = L'\0';
        }
        CoTaskMemFree(psz1);
    }
    else if (hr == E_FAIL)
    {
        throw std::runtime_error("KnownFolderPath::Get failed. (KF_CATEGORY_VIRTUAL)");
    }
    else if (hr == E_INVALIDARG)
    {
        throw std::runtime_error("KnownFolderPath::Get failed. (not exist)");
    }
    else
    {
        throw std::runtime_error("KnownFolderPath::Get failed. (unknown reason)");
    }
    return psz2;
}


KnownFolderPath::KnownFolderPath(const KNOWNFOLDERID& rid, DWORD dwFlags)
    : m_psz(Get(rid, dwFlags))
{
}


KnownFolderPath::KnownFolderPath(const KnownFolderPath& rSrc)
    : m_psz(NULL)
{
    operator =(rSrc);
}


KnownFolderPath::~KnownFolderPath()
{
    free(m_psz);
}


void KnownFolderPath::operator =(const KnownFolderPath& rSrc)
{
    free(InterlockedExchangePointer((PVOID*)&m_psz, NULL));
    m_psz = _wcsdup(rSrc);
    if (!m_psz)
    {
        throw std::bad_alloc();
    }
}
