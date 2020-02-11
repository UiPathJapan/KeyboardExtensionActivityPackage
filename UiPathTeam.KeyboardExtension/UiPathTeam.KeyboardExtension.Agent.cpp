#include "pch.h"
#include "Debug.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.h"
#include "UiPathTeam.KeyboardExtension.Agent.h"


extern DWORD g_dwTlsIndex;


using namespace UiPathTeam;
using namespace UiPathTeam::KeyboardExtension;


Agent::Agent()
    : m_ulRefCount(1)
    , m_bReleasePending(false)
    , m_bOnCall(false)
    , m_bInitialized(false)
    , m_pAgentIpc()
    , m_pDesktopIpc()
    , m_pTfThreadMgr(NULL)
    , m_TfClientId(TF_CLIENTID_NULL)
    , m_pTfCompartmentMgr(NULL)
    , m_pTfCompartmentKeyboardOpenClose(NULL)
    , m_pTfCompartmentInputModeConversion(NULL)
    , m_CompartmentEventSinkCookieMap()
    , m_pInputProcessorProfiles(NULL)
    , m_dwLanguageProfileNotifySinkCookie(TF_INVALID_COOKIE)
    , m_JaState(STATE_COMPARTMENT_NONE)
    , m_KoState(STATE_COMPARTMENT_NONE)
    , m_ZhTwState(STATE_COMPARTMENT_NONE)
    , m_ZhCnState(STATE_COMPARTMENT_NONE)
    , m_ZhHkState(STATE_COMPARTMENT_NONE)
    , m_ZhSgState(STATE_COMPARTMENT_NONE)
    , m_ZhMoState(STATE_COMPARTMENT_NONE)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::ctor");
    DBGPUT(L"Started.");
    if (m_pAgentIpc.Map(GetCurrentThreadId()))
    {
        if (!m_pDesktopIpc.Map())
        {
            m_pAgentIpc.Unmap();
        }
    }
    DBGPUT(L"Ended.");
}


Agent::~Agent()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::dtor");
    DBGPUT(L"Started.");
    if (m_bInitialized)
    {
        Uninitialize();
    }
    m_pDesktopIpc.Unmap();
    m_pAgentIpc.Unmap();
    DBGPUT(L"Ended.");
}


void Agent::Initialize()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::Initialize");
    DBGPUT(L"Started.");

    if (m_bInitialized)
    {
        DBGPUT(L"Ended.");
        return;
    }

    if (!m_pAgentIpc)
    {
        DBGPUT(L"Ended. IPC block unavailable.");
        return;
    }

    m_bInitialized = true;

    HRESULT hr;

    hr = CoInitialize(NULL);
    DBGPUT(L"CoInitialize: return=%08lx", hr);
    // Subsequent COM calls will fail if this failed...

    hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&m_pTfThreadMgr);
    if (hr == S_OK)
    {
        DBGPUT(L"CoCreateInstance(TF_ThreadMgr): %p", m_pTfThreadMgr);
    }
    else
    {
        Debug::Put(L"CoCreateInstance(TF_ThreadMgr): Failed. error=%08lx", hr);
        return;
    }

    hr = m_pTfThreadMgr->Activate(&m_TfClientId);
    if (hr == S_OK)
    {
        DBGPUT(L"TfThreadMgr::Activate: %d", (int)m_TfClientId);
    }
    else
    {
        Debug::Put(L"TfThreadMgr::Activate: Failed. error=%08lx", hr);
        return;
    }

    hr = m_pTfThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void**)&m_pTfCompartmentMgr);
    if (hr == S_OK)
    {
        DBGPUT(L"TfThreadMgr::QueryInterface(TfCompartmentMgr): %p", m_pTfCompartmentMgr);
    }
    else
    {
        Debug::Put(L"TfThreadMgr::QueryInterface(TfCompartmentMgr): Failed. error=%08lx", hr);
        return;
    }

    if (!GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &m_pTfCompartmentKeyboardOpenClose))
    {
        return;
    }

    if (!GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_pTfCompartmentInputModeConversion))
    {
        return;
    }

    m_pAgentIpc->m_dwValidity = 0;

    if (GetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, m_pAgentIpc->m_KeyboardOpenClose) == S_OK)
    {
        m_pAgentIpc->m_dwValidity |= VALIDITY_OPENCLOSE;
    }

    if (GetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion) == S_OK)
    {
        m_pAgentIpc->m_dwValidity |= VALIDITY_CONVERSION;
    }

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (void**)&m_pInputProcessorProfiles);
    if (hr == S_OK)
    {
        DBGPUT(L"CoCreateInstance(TF_InputProcessorProfiles): %p", m_pInputProcessorProfiles);
        hr = m_pInputProcessorProfiles->GetCurrentLanguage(&m_pAgentIpc->m_LangId);
        if (hr == S_OK)
        {
            DBGPUT(L"TfInputProcessorProfiles::GetCurrentLanguage: %04x", m_pAgentIpc->m_LangId);
            m_pAgentIpc->m_dwValidity |= VALIDITY_LANGID;
        }
        else
        {
            Debug::Put(L"TfInputProcessorProfiles::GetCurrentLanguage: Failed. error=%08lx", hr);
        }
        ITfSource *pSource = NULL;
        hr = m_pInputProcessorProfiles->QueryInterface(IID_ITfSource, (void**)&pSource);
        if (hr == S_OK)
        {
            DBGPUT(L"TfInputProcessorProfiles::QueryInterface(TfSource): %p", pSource);
            hr = pSource->AdviseSink(IID_ITfLanguageProfileNotifySink, (ITfLanguageProfileNotifySink*)this, &m_dwLanguageProfileNotifySinkCookie);
            pSource->Release();
            if (hr == S_OK)
            {
                DBGPUT(L"AdviseSink(ITfLanguageProfileNotifySink): %lu", m_dwLanguageProfileNotifySinkCookie);
            }
            else
            {
                Debug::Put(L"AdviseSink(ITfLanguageProfileNotifySink): Failed. error=%08lx", hr);
            }
        }
        else
        {
            Debug::Put(L"TfInputProcessorProfiles::QueryInterface(TfSource): Failed. error=%08lx", hr);
        }
    }
    else
    {
        Debug::Put(L"CoCreateInstance(TF_InputProcessorProfiles): Failed. error=%08lx", hr);
    }

    DBGPUT(L"Ended.");
}


void Agent::Uninitialize()
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::Uninitialize");
    DBGPUT(L"Started.");

    if (!m_bInitialized)
    {
        DBGPUT(L"Ended.");
        return;
    }

    m_bInitialized = false;

    if (!m_pAgentIpc)
    {
        DBGPUT(L"Ended. IPC block unavailable.");
        return;
    }

    HRESULT hr;

    if (m_pInputProcessorProfiles != NULL)
    {
        ITfSource *pSource = NULL;
        hr = m_pInputProcessorProfiles->QueryInterface(IID_ITfSource, (void**)&pSource);
        if (hr == S_OK)
        {
            hr = pSource->UnadviseSink(m_dwLanguageProfileNotifySinkCookie);
            if (hr == S_OK)
            {
                DBGPUT(L"TfSource::UnadviseSink(LanguageProfileNotifySink): Done.");
            }
            else
            {
                Debug::Put(L"TfSource::UnadviseSink(LanguageProfileNotifySink): Failed. error=%08lx", hr);
            }
            pSource->Release();
        }
        m_pInputProcessorProfiles->Release();
        m_pInputProcessorProfiles = NULL;
    }

    UnadviseAllCompartmentEventSinks();

    if (m_pTfCompartmentInputModeConversion != NULL)
    {
        m_pTfCompartmentInputModeConversion->Release();
        m_pTfCompartmentInputModeConversion = NULL;
    }

    if (m_pTfCompartmentKeyboardOpenClose != NULL)
    {
        m_pTfCompartmentKeyboardOpenClose->Release();
        m_pTfCompartmentKeyboardOpenClose = NULL;
    }

    if (m_pTfCompartmentMgr != NULL)
    {
        m_pTfCompartmentMgr->Release();
        m_pTfCompartmentMgr = NULL;
    }

    if (m_pTfThreadMgr != NULL)
    {
        if (m_TfClientId != TF_CLIENTID_NULL)
        {
            m_pTfThreadMgr->Deactivate();
            m_TfClientId = TF_CLIENTID_NULL;
        }
        m_pTfThreadMgr->Release();
        m_pTfThreadMgr = NULL;
    }

    CoUninitialize();

    DBGPUT(L"Ended.");
}


LRESULT CALLBACK Agent::CallWndProcHook(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    Debug::Function x(L"UiPathTeam::KeyboardExtension::Agent::CallWndProcHook%d", Platform::Is64bitProcess() ? 64 : 32);
    if (nCode < 0)
    {
        DBGPUT(L"nCode<0");
    }
    else if (nCode == HC_ACTION)
    {
        try
        {
            if (g_dwTlsIndex != TLS_OUT_OF_INDEXES)
            {
                Agent* pContext = reinterpret_cast<Agent*>(TlsGetValue(g_dwTlsIndex));
                if (pContext == NULL)
                {
                    pContext = new Agent();
                    TlsSetValue(g_dwTlsIndex, pContext);
                }
                pContext->OnCall(reinterpret_cast<CWPSTRUCT*>(lParam));
            }
        }
        catch (std::runtime_error ex)
        {
            Debug::Put(L"%hs", ex.what());
        }
        catch (std::bad_alloc)
        {
            Debug::Put(L"Out of memory.");
        }
        catch (...)
        {
            Debug::Put(L"Unhandled exception caught.");
        }
    }
    else
    {
        DBGPUT(L"nCode>0");
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


void Agent::OnCall(const CWPSTRUCT* pCWPS)
{
    if (!m_bOnCall && m_pAgentIpc)
    {
        // This prevents the code from being called recursively.
        m_bOnCall = true;
        OnCall2(pCWPS);
        m_bOnCall = false;
    }
}


void Agent::OnCall2(const CWPSTRUCT* pCWPS)
{
    if (pCWPS->message == m_pDesktopIpc->m_WM_AGENT_WAKEUP)
    {
        if (pCWPS->wParam == AGENT_INITIALIZE)
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::AGENT_INITIALIZE");
            DBGPUT(L"Started.");
            if (!m_bInitialized)
            {
                Initialize();
            }
            DBGPUT(L"Ended.");
            //FALLTHROUGH
        }
        else if (pCWPS->wParam == AGENT_UNINITIALIZE)
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::AGENT_UNINITIALIZE");
            DBGPUT(L"Started.");
            if (m_bInitialized)
            {
                RestoreInputSettings();
                RestoreKeyboardLayout();
                Uninitialize();
            }
            DBGPUT(L"Ended.");
            return;
        }
        else if (pCWPS->wParam == AGENT_GET_STATE)
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::AGENT_GET_STATE");
            DBGPUT(L"Started.");
            if (m_bInitialized)
            {
                m_pAgentIpc->m_LangId = 0;
                m_pAgentIpc->m_KeyboardOpenClose = 0;
                m_pAgentIpc->m_InputModeConversion = 0;
                m_pAgentIpc->m_dwValidity = 0;
                if (m_pTfCompartmentKeyboardOpenClose)
                {
                    if (GetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, m_pAgentIpc->m_KeyboardOpenClose) == S_OK)
                    {
                        m_pAgentIpc->m_dwValidity |= VALIDITY_OPENCLOSE;
                    }
                }
                if (m_pTfCompartmentInputModeConversion)
                {
                    if (GetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion) == S_OK)
                    {
                        m_pAgentIpc->m_dwValidity |= VALIDITY_CONVERSION;
                    }
                }
                if (m_pInputProcessorProfiles)
                {
                    HRESULT hr = m_pInputProcessorProfiles->GetCurrentLanguage(&m_pAgentIpc->m_LangId);
                    if (hr == S_OK)
                    {
                        DBGPUT(L"TfInputProcessorProfiles::GetCurrentLanguage: %04x", m_pAgentIpc->m_LangId);
                        m_pAgentIpc->m_dwValidity |= VALIDITY_LANGID;
                    }
                    else
                    {
                        Debug::Put(L"TfInputProcessorProfiles::GetCurrentLanguage: Failed. error=%08lx", hr);
                    }
                }
            }
            DBGPUT(L"Ended.");
            return;
        }
        else if (pCWPS->wParam == AGENT_SET_OPENCLOSE)
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::AGENT_SET_OPENCLOSE");
            DBGPUT(L"Started.");
            if (m_bInitialized)
            {
                if (m_pTfCompartmentKeyboardOpenClose)
                {
                    SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, (static_cast<DWORD>(pCWPS->lParam) >> 15) & 1);
                }
            }
            DBGPUT(L"Ended.");
            return;
        }
        else if (pCWPS->wParam == AGENT_SET_CONVERSION)
        {
            DBGFNC(L"UiPathTeam::KeyboardExtension::AGENT_SET_CONVERSION");
            DBGPUT(L"Started.");
            if (m_bInitialized)
            {
                if (m_pTfCompartmentInputModeConversion)
                {
                    SetCompartmentLong(m_pTfCompartmentInputModeConversion, (static_cast<DWORD>(pCWPS->lParam) >> 0) & 0x7fff);
                }
            }
            DBGPUT(L"Ended.");
            return;
        }
    }
    if (m_bInitialized)
    {
        if ((m_pAgentIpc->m_dwFlags & (RESET_IME | SET_IME)) == 0 || m_pDesktopIpc->m_Paused)
        {
            RestoreInputSettings();
        }

        if (m_pDesktopIpc->m_KeyboardLayoutSetting.m_PreferredLangId && !m_pDesktopIpc->m_Paused)
        {
            ForceKeyboardLayout();
        }
        else
        {
            RestoreKeyboardLayout();
        }

        if ((m_pAgentIpc->m_dwFlags & RESET_IME) != 0 && !m_pDesktopIpc->m_Paused)
        {
            ResetInputSettings();
        }
        else if ((m_pAgentIpc->m_dwFlags & SET_IME) != 0 && !m_pDesktopIpc->m_Paused)
        {
            SetInputSettings();
        }
    }
}


bool Agent::ForceKeyboardLayout()
{
    while (true)
    {
        KeyboardLayoutSetting current = m_pDesktopIpc->m_KeyboardLayoutSetting;
        if (m_pAgentIpc->m_LangId == current.m_SelectedLangId)
        {
            // Nothing needs to be done.
            return true;
        }
        if (m_pInputProcessorProfiles == NULL)
        {
            return false;
        }
        if (!current.m_PreferredLangId)
        {
            return RestoreKeyboardLayout();
        }
        if (!current.m_SelectedLangId)
        {
            KeyboardLayoutSetting next;
            next.m_PreferredLangId = current.m_PreferredLangId;
            next.m_SelectedLangId = (LANGID)-1;
            LANGID* pLangIds = NULL;
            ULONG ulLangCount = 0;
            HRESULT hr = m_pInputProcessorProfiles->GetLanguageList(&pLangIds, &ulLangCount);
            if (hr == S_OK)
            {
                DBGPUT(L"TfInputProcessorProfiles::GetLanguageList: count=%lu", ulLangCount);
                for (ULONG index = 0; index < ulLangCount; index++)
                {
                    DBGPUT(L"LANGID=%04x", pLangIds[index]);
                    if (pLangIds[index] == next.m_PreferredLangId)
                    {
                        next.m_SelectedLangId = pLangIds[index];
                        break;
                    }
                    else if (next.m_SelectedLangId == (LANGID)-1 && PRIMARYLANGID(pLangIds[index]) == PRIMARYLANGID(next.m_PreferredLangId))
                    {
                        next.m_SelectedLangId = pLangIds[index];
                    }
                }
                if (pLangIds)
                {
                    CoTaskMemFree(pLangIds);
                }
            }
            else
            {
                Debug::Put(L"TfInputProcessorProfiles::GetLanguageList: Failed. error=%08lx", hr);
                return false;
            }
            if (InterlockedCompareExchange(&m_pDesktopIpc->m_KeyboardLayoutSetting.m_LangIds, next.m_LangIds, current.m_LangIds) != current.m_LangIds)
            {
                DBGPUT(L"Layout settings changed while processing. Retrying...");
                continue;
            }
            DBGPUT(L"ForcedLangId=%04x", next.m_SelectedLangId);
            current.m_LangIds = next.m_LangIds;
        }
        if (current.m_SelectedLangId != (LANGID)-1)
        {
            LANGID last = m_pAgentIpc->m_LangId;
            HRESULT hr = m_pInputProcessorProfiles->ChangeCurrentLanguage(current.m_SelectedLangId);
            if (hr == S_OK)
            {
                DBGPUT(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x)", current.m_SelectedLangId);
                InterlockedCompareExchange16(reinterpret_cast<SHORT*>(&m_pDesktopIpc->m_LastLangId), last, 0);
                return true;
            }
            else
            {
                Debug::Put(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Failed. error=%08lx", current.m_SelectedLangId, hr);
                return false;
            }
        }
        else
        {
            return false;
        }
        //NEVER REACH HERE
    }
}


bool Agent::RestoreKeyboardLayout()
{
    if (!m_pDesktopIpc->m_LastLangId || m_pAgentIpc->m_LangId == m_pDesktopIpc->m_LastLangId)
    {
        // Nothing needs to be done.
        return true;
    }
    else if (m_pInputProcessorProfiles != NULL)
    {
        HRESULT hr = m_pInputProcessorProfiles->ChangeCurrentLanguage(m_pDesktopIpc->m_LastLangId);
        if (hr == S_OK)
        {
            DBGPUT(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Done.", m_pDesktopIpc->m_LastLangId);
            return true;
        }
        else
        {
            Debug::Put(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Failed. error=%08lx", m_pDesktopIpc->m_LastLangId, hr);
            return false;
        }
    }
    else
    {
        return false;
    }
}


void Agent::ResetInputSettings()
{
    switch (PRIMARYLANGID(m_pAgentIpc->m_LangId))
    {
    case LANG_JAPANESE:
        ResetKeyboardOpenClose(m_JaState);
        break;
    case LANG_KOREAN: // turns off Hangul conversion mode
        ResetInputModeConversion(m_KoState);
        break;
    case LANG_CHINESE: // Experimental Support (further investigation is needed)
        switch (SUBLANGID(m_pAgentIpc->m_LangId))
        {
        case SUBLANG_CHINESE_TRADITIONAL: // Chinese (Taiwan) 0x0404 zh-TW
            ResetKeyboardOpenClose(m_ZhTwState);
            break;
        case SUBLANG_CHINESE_SIMPLIFIED: // Chinese (PR China) 0x0804 zh-CN
            ResetKeyboardOpenClose(m_ZhCnState);
            break;
        case SUBLANG_CHINESE_HONGKONG: // Chinese (Hong Kong S.A.R., P.R.C.) 0x0c04 zh-HK
            ResetKeyboardOpenClose(m_ZhHkState);
            break;
        case SUBLANG_CHINESE_SINGAPORE: // Chinese (Singapore) 0x1004 zh-SG
            ResetKeyboardOpenClose(m_ZhSgState);
            break;
        case SUBLANG_CHINESE_MACAU: // Chinese (Macau S.A.R.) 0x1404 zh-MO
            ResetKeyboardOpenClose(m_ZhMoState);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


void Agent::SetInputSettings()
{
    switch (PRIMARYLANGID(m_pAgentIpc->m_LangId))
    {
    case LANG_JAPANESE:
        SetKeyboardOpenClose(m_JaState);
        break;
    case LANG_KOREAN: // turns on Hangul conversion mode
        SetInputModeConversion(m_KoState);
        break;
    case LANG_CHINESE: // Experimental Support (further investigation is needed)
        switch (SUBLANGID(m_pAgentIpc->m_LangId))
        {
        case SUBLANG_CHINESE_TRADITIONAL: // Chinese (Taiwan) 0x0404 zh-TW
            SetKeyboardOpenClose(m_ZhTwState);
            break;
        case SUBLANG_CHINESE_SIMPLIFIED: // Chinese (PR China) 0x0804 zh-CN
            SetKeyboardOpenClose(m_ZhCnState);
            break;
        case SUBLANG_CHINESE_HONGKONG: // Chinese (Hong Kong S.A.R., P.R.C.) 0x0c04 zh-HK
            SetKeyboardOpenClose(m_ZhHkState);
            break;
        case SUBLANG_CHINESE_SINGAPORE: // Chinese (Singapore) 0x1004 zh-SG
            SetKeyboardOpenClose(m_ZhSgState);
            break;
        case SUBLANG_CHINESE_MACAU: // Chinese (Macau S.A.R.) 0x1404 zh-MO
            SetKeyboardOpenClose(m_ZhMoState);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


void Agent::RestoreInputSettings()
{
    switch (PRIMARYLANGID(m_pAgentIpc->m_LangId))
    {
    case LANG_JAPANESE:
        RestoreKeyboardOpenClose(m_JaState);
        break;
    case LANG_KOREAN:
        RestoreInputModeConversion(m_KoState);
        break;
    case LANG_CHINESE: // Experimental Support (further investigation is needed)
        switch (SUBLANGID(m_pAgentIpc->m_LangId))
        {
        case SUBLANG_CHINESE_TRADITIONAL: // Chinese (Taiwan) 0x0404 zh-TW
            RestoreKeyboardOpenClose(m_ZhTwState);
            break;
        case SUBLANG_CHINESE_SIMPLIFIED: // Chinese (PR China) 0x0804 zh-CN
            RestoreKeyboardOpenClose(m_ZhCnState);
            break;
        case SUBLANG_CHINESE_HONGKONG: // Chinese (Hong Kong S.A.R., P.R.C.) 0x0c04 zh-HK
            RestoreKeyboardOpenClose(m_ZhHkState);
            break;
        case SUBLANG_CHINESE_SINGAPORE: // Chinese (Singapore) 0x1004 zh-SG
            RestoreKeyboardOpenClose(m_ZhSgState);
            break;
        case SUBLANG_CHINESE_MACAU: // Chinese (Macau S.A.R.) 0x1404 zh-MO
            RestoreKeyboardOpenClose(m_ZhMoState);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


void Agent::ResetKeyboardOpenClose(BYTE& state)
{
    if (state == STATE_COMPARTMENT_NONE)
    {
        state = m_pAgentIpc->m_KeyboardOpenClose == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
    }
    if (m_pAgentIpc->m_KeyboardOpenClose != 0)
    {
        SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 0); // turns off IME
    }
}


void Agent::SetKeyboardOpenClose(BYTE& state)
{
    if (state == STATE_COMPARTMENT_NONE)
    {
        state = m_pAgentIpc->m_KeyboardOpenClose == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
    }
    if (m_pAgentIpc->m_KeyboardOpenClose == 0)
    {
        SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 1); // turns on IME
    }
}


void Agent::ResetInputModeConversion(BYTE& state)
{
    if (state == STATE_COMPARTMENT_NONE)
    {
        state = (m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
    }
    if ((m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == TF_CONVERSIONMODE_NATIVE)
    {
        SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion & ~TF_CONVERSIONMODE_NATIVE); // turns off NATIVE conversion mode
    }
}


void Agent::SetInputModeConversion(BYTE& state)
{
    if (state == STATE_COMPARTMENT_NONE)
    {
        state = (m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
    }
    if ((m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) != TF_CONVERSIONMODE_NATIVE)
    {
        SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion | TF_CONVERSIONMODE_NATIVE); // turns on NATIVE conversion mode
    }
}


void Agent::RestoreKeyboardOpenClose(BYTE& state)
{
    if (state != STATE_COMPARTMENT_NONE)
    {
        if (state == STATE_COMPARTMENT_SET && m_pAgentIpc->m_KeyboardOpenClose == 0)
        {
            SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 1); // turns on IME
        }
        else if (state == STATE_COMPARTMENT_RESET && m_pAgentIpc->m_KeyboardOpenClose == 1)
        {
            SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 0); // turns off IME
        }
        state = STATE_COMPARTMENT_NONE;
    }
}


void Agent::RestoreInputModeConversion(BYTE& state)
{
    if (state != STATE_COMPARTMENT_NONE)
    {
        if (state == STATE_COMPARTMENT_SET && (m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == 0)
        {
            SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion | TF_CONVERSIONMODE_NATIVE); // turns on NATIVE conversion mode
        }
        else if (state == STATE_COMPARTMENT_RESET && (m_pAgentIpc->m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) != 0)
        {
            SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_pAgentIpc->m_InputModeConversion & ~TF_CONVERSIONMODE_NATIVE); // turns off NATIVE conversion mode
        }
        state = STATE_COMPARTMENT_NONE;
    }
}
