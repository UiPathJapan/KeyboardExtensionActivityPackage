#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


HRESULT STDMETHODCALLTYPE Agent::OnChange(
    /* [in] */ __RPC__in REFGUID rguid)
{
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
    {
        DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::OnChange(KEYBOARD_OPENCLOSE)");
        DBGPUT(L"Started.");
        GetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, m_KeyboardOpenClose);
        DBGPUT(L"Ended.");
        return S_OK;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
    {
        DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::OnChange(KEYBOARD_INPUTMODE_CONVERSION)");
        DBGPUT(L"Started.");
        GetCompartmentLong(m_pTfCompartmentInputModeConversion, m_InputModeConversion);
        DBGPUT(L"Ended.");
        return S_OK;
    }
    else
    {
        DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::OnChange");
        DBGPUT(L"Unexpected compartment.");
        return E_UNEXPECTED;
    }
}
