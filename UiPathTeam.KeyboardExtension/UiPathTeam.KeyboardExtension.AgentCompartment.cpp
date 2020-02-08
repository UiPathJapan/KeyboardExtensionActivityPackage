#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


static PCWSTR GetGuidText(const GUID& rguid)
{
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE)) return L"KEYBOARD_OPENCLOSE";
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION)) return L"KEYBOARD_INPUTMODE_CONVERSION";
    return L"?";
}


bool Agent::GetCompartment(const GUID& rguid, ITfCompartment** ppCompartment)
{
    HRESULT hr;

    ITfCompartment* pCompartment = NULL;
    hr = m_pTfCompartmentMgr->GetCompartment(rguid, &pCompartment);
    if (hr == S_OK)
    {
        Debug::Put(L"TfCompartmentMgr::GetCompartment(%s): %p", GetGuidText(rguid), pCompartment);
    }
    else
    {
        Debug::Put(L"TfCompartmentMgr::GetCompartment(%s): Failed. error=%08lx", GetGuidText(rguid), hr);
        return false;
    }

    ITfSource *pSource = NULL;
    hr = pCompartment->QueryInterface(IID_ITfSource, (void**)&pSource);
    if (hr == S_OK)
    {
        Debug::Put(L"TfCompartment(%s)::QueryInterface(TfSource): %p", GetGuidText(rguid), pSource);
    }
    else
    {
        Debug::Put(L"TfCompartment(%s)::QueryInterface(TfSource): Failed. error=%08lx", GetGuidText(rguid), hr);
        pCompartment->Release();
        return false;
    }

    DWORD dwCookie = TF_INVALID_COOKIE;
    hr = pSource->AdviseSink(IID_ITfCompartmentEventSink, (ITfCompartmentEventSink*)this, &dwCookie);
    pSource->Release();
    if (hr == S_OK)
    {
        Debug::Put(L"AdviseSink(ITfCompartmentEventSink): %lu", dwCookie);
        m_CompartmentEventSinkCookieMap.insert(std::pair<ITfCompartment*, DWORD>(pCompartment, dwCookie));
        pCompartment->AddRef();
        *ppCompartment = pCompartment;
        return true;
    }
    else
    {
        Debug::Put(L"AdviseSink(ITfCompartmentEventSink): Failed. error=%08lx", hr);
        pCompartment->Release();
        return false;
    }
}


void Agent::UnadviseAllCompartmentEventSinks()
{
    for (std::map<ITfCompartment*, DWORD>::const_iterator iter = m_CompartmentEventSinkCookieMap.begin(); iter != m_CompartmentEventSinkCookieMap.end(); iter++)
    {
        ITfCompartment* pCompartment = iter->first;
        DWORD dwCookie = iter->second;
        if (dwCookie != TF_INVALID_COOKIE && pCompartment != NULL)
        {
            ITfSource *pSource = NULL;
            HRESULT hr = pCompartment->QueryInterface(IID_ITfSource, (void**)&pSource);
            if (hr == S_OK)
            {
                pSource->UnadviseSink(dwCookie);
                pSource->Release();
            }
            pCompartment->Release();
        }
    }
    m_CompartmentEventSinkCookieMap.clear();
}


HRESULT Agent::GetCompartmentLong(ITfCompartment* pCompartment, LONG& rValue)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::GetCompartmentLong(%s)", GetCompartmentName(pCompartment));

    if (pCompartment == NULL)
    {
        DBGPUT(L"TfCompartment not available.");
        return E_FAIL;
    }

    DBGPUT(L"Started.");

    VARIANT value;
    VariantInit(&value);

    HRESULT hr = m_pTfCompartmentKeyboardOpenClose->GetValue(&value);
    if (hr == S_OK)
    {
        if (value.vt == VT_I4)
        {
            DBGPUT(L"Ended. return=%ld", value.lVal);
            rValue = value.lVal;
        }
        else
        {
            DBGPUT(L"Ended. Unexpected type=%d", value.vt);
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        DBGPUT(L"Ended with error=%08lx", hr);
    }
    return hr;
}

HRESULT Agent::SetCompartmentLong(ITfCompartment* pCompartment, LONG settingValue)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::SetCompartmentLong(%s,%ld)", GetCompartmentName(pCompartment), settingValue);

    if (pCompartment == NULL)
    {
        DBGPUT(L"TfCompartment not available.");
        return E_POINTER;
    }

    DBGPUT(L"Started.");

    VARIANT value;
    value.vt = VT_I4;
    value.lVal = settingValue;

    HRESULT hr = pCompartment->SetValue(m_TfClientId, &value);
    if (hr == S_OK)
    {
        DBGPUT(L"Ended successfully.");
    }
    else
    {
        DBGPUT(L"Ended with error=%08lx", hr);
    }

    return hr;
}


PCWSTR Agent::GetCompartmentName(ITfCompartment* pCompartment) const
{
    if (pCompartment == m_pTfCompartmentKeyboardOpenClose) return L"KEYBOARD_OPENCLOSE";
    if (pCompartment == m_pTfCompartmentInputModeConversion) return L"KEYBOARD_INPUTMODE_CONVERSION";
    return L"?";
}
