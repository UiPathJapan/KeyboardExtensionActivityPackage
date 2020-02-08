#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


HRESULT STDMETHODCALLTYPE Agent::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::QueryInterface");
    if (!ppvObject)
    {
        Debug::Put(L"return=E_POINTER");
        return E_POINTER;
    }
    else if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = (ITfCompartmentEventSink*)this;
        m_ulRefCount++;
        DBGPUT(L"return=S_OK (IUnknown)");
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObject = (ITfCompartmentEventSink*)this;
        m_ulRefCount++;
        DBGPUT(L"return=S_OK (ITfCompartmentEventSink)");
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ITfLanguageProfileNotifySink))
    {
        *ppvObject = (ITfLanguageProfileNotifySink*)this;
        m_ulRefCount++;
        DBGPUT(L"return=S_OK (ITfLanguageProfileNotifySink)");
        return S_OK;
    }
    else
    {
        Debug::Put(L"return=E_NOINTERFACE");
        return E_NOINTERFACE;
    }
}


ULONG STDMETHODCALLTYPE Agent::AddRef(void)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::AddRef");
    ++m_ulRefCount;
    DBGPUT(L"return=%lu", m_ulRefCount);
    return m_ulRefCount;
}


ULONG STDMETHODCALLTYPE Agent::Release(void)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::Release");
    DBGPUT(L"Started.");
    if (--m_ulRefCount == 0)
    {
        if (m_bReleasePending)
        {
            DBGPUT(L"Being released now...");
        }
        else
        {
            m_bReleasePending = true;
            delete this;
        }
        DBGPUT(L"Ended. return=0");
        return 0;
    }
    DBGPUT(L"Ended. return=%ld", m_ulRefCount);
    return m_ulRefCount;
}
