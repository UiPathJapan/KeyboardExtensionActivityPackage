#include "pch.h"
#include "Debug.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


HRESULT STDMETHODCALLTYPE Agent::OnLanguageChange(
    /* [in] */ LANGID langid,
    /* [out] */ __RPC__out BOOL *pfAccept)
{
    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::OnLanguageChange(%04x)", langid);
    if (m_pDesktopIpc->m_KeyboardLayoutSetting.m_PreferredLangId != (LANGID)0
        && m_pDesktopIpc->m_KeyboardLayoutSetting.m_SelectedLangId != langid
        && m_pDesktopIpc->m_KeyboardLayoutSetting.m_SelectedLangId != (LANGID)0
        && m_pDesktopIpc->m_KeyboardLayoutSetting.m_SelectedLangId != (LANGID)-1
        && !m_pDesktopIpc->m_Paused)
    {
        *pfAccept = FALSE;
        DBGPUT(L"return=S_OK Accept=FALSE");
        return S_OK;
    }
    *pfAccept = TRUE;
    DBGPUT(L"return=S_OK Accept=TRUE");
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Agent::OnLanguageChanged(void)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::OnLanguageChanged");
    HRESULT hr = m_pInputProcessorProfiles->GetCurrentLanguage(&m_pAgentIpc->m_LangId);
    if (hr == S_OK)
    {
        DBGPUT(L"TfInputProcessorProfiles::GetCurrentLanguage: %04x", m_pAgentIpc->m_LangId);
    }
    else
    {
        Debug::Put(L"TfInputProcessorProfiles::GetCurrentLanguage: Failed. error=%08lx", hr);
    }
    return S_OK;
}
