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
    , m_bEnabled(false)
    , m_bInitialized(false)
    , m_hIpcMapping(NULL)
    , m_pIpcBlock(NULL)
    , m_pTfThreadMgr(NULL)
    , m_TfClientId(TF_CLIENTID_NULL)
    , m_pTfCompartmentMgr(NULL)
    , m_pTfCompartmentKeyboardOpenClose(NULL)
    , m_pTfCompartmentInputModeConversion(NULL)
    , m_CompartmentEventSinkCookieMap()
    , m_KeyboardOpenClose(0)
    , m_InputModeConversion(0)
    , m_pInputProcessorProfiles(NULL)
    , m_dwLanguageProfileNotifySinkCookie(TF_INVALID_COOKIE)
    , m_LangId(0)
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
    m_pIpcBlock = Ipc::Map(m_hIpcMapping);
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
    if (m_pIpcBlock != NULL)
    {
        m_pIpcBlock->Unmap(m_hIpcMapping);
    }
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

    if (m_pIpcBlock == NULL)
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

    GetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, m_KeyboardOpenClose);

    GetCompartmentLong(m_pTfCompartmentInputModeConversion, m_InputModeConversion);

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (void**)&m_pInputProcessorProfiles);
    if (hr == S_OK)
    {
        DBGPUT(L"CoCreateInstance(TF_InputProcessorProfiles): %p", m_pInputProcessorProfiles);
        hr = m_pInputProcessorProfiles->GetCurrentLanguage(&m_LangId);
        if (hr == S_OK)
        {
            DBGPUT(L"TfInputProcessorProfiles::GetCurrentLanguage: %04x", m_LangId);
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

    if (m_pIpcBlock == NULL)
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
