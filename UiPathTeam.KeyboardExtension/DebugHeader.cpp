#include "pch.h"
#include "SimpleLock.h"
#include "DebugHeader.h"


using namespace UiPathTeam;


#define DEFAULT_SEPARATOR L": "


static SimpleLock::Type s_Mutex;
static DWORD s_dwTlsIndex = TLS_OUT_OF_INDEXES;


DebugHeader& DebugHeader::Instance()
{
    if (s_dwTlsIndex == TLS_OUT_OF_INDEXES)
    {
        SimpleLock lock(s_Mutex);
        if (s_dwTlsIndex == TLS_OUT_OF_INDEXES)
        {
            s_dwTlsIndex = TlsAlloc();
            if (s_dwTlsIndex == TLS_OUT_OF_INDEXES)
            {
                throw std::runtime_error("Out of thread local indexes.");
            }
        }
    }

    DebugHeader* pH = (DebugHeader*)TlsGetValue(s_dwTlsIndex);
    if (pH == NULL)
    {
        pH = new DebugHeader();
        if (TlsSetValue(s_dwTlsIndex, pH) == FALSE)
        {
            throw std::runtime_error("Failed to set a value to thread local storage.");
        }
    }

    return *pH;
}


void DebugHeader::Destroy()
{
    SimpleLock lock(s_Mutex);

    if (s_dwTlsIndex == TLS_OUT_OF_INDEXES)
    {
        return;
    }

    DebugHeader* pH = (DebugHeader*)TlsGetValue(s_dwTlsIndex);
    if (pH == NULL)
    {
        return;
    }

    if (TlsSetValue(s_dwTlsIndex, NULL) == FALSE)
    {
        throw std::runtime_error("Failed to set a value to thread local storage.");
    }

    delete pH;
}


DebugHeader::DebugHeader()
    : m_stack()
    , m_pszSeparator(NULL)
{
}


DebugHeader::~DebugHeader()
{
    Clear();
    free(m_pszSeparator);
}


PCWSTR DebugHeader::Get() const
{
    if (m_stack.size() > 0)
    {
        return m_stack.top();
    }
    else
    {
        return NULL;
    }
}


void DebugHeader::Push(PCWSTR pszFormat, va_list argList)
{
    va_list argList2;
    va_copy(argList2, argList);
    int len = _vscwprintf(pszFormat, argList2);
    va_end(argList2);
    if (len < 0)
    {
        throw std::runtime_error("DebugHeader::Push: Failed to format.");
    }
    size_t size = len + 1;
    PWCHAR psz = (PWCHAR)calloc(size, sizeof(WCHAR));
    if (psz == NULL)
    {
        throw std::bad_alloc();
    }
    va_copy(argList2, argList);
    _vsnwprintf_s(psz, size, _TRUNCATE, pszFormat, argList2);
    va_end(argList2);
    m_stack.push(psz);
}


void DebugHeader::Pop()
{
    if (m_stack.size() > 0)
    {
        PWCHAR psz = m_stack.top();
        m_stack.pop();
        free(psz);
    }
}


void DebugHeader::Clear()
{
    while (m_stack.size() > 0)
    {
        PWCHAR psz = m_stack.top();
        m_stack.pop();
        free(psz);
    }
}


PCWSTR DebugHeader::GetSeparator() const
{
    return m_pszSeparator != NULL ? m_pszSeparator : DEFAULT_SEPARATOR;
}


void DebugHeader::SetSeparator(PCWSTR psz)
{
    free(InterlockedExchangePointer((PVOID*)&m_pszSeparator, NULL));
    if (psz != NULL)
    {
        m_pszSeparator = _wcsdup(psz);
        if (m_pszSeparator == NULL)
        {
            throw std::bad_alloc();
        }
    }
}
