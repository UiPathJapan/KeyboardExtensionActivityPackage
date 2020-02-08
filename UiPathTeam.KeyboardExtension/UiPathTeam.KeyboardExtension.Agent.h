#pragma once


#include "Debug.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.Ipc.h"
#include <msctf.h>
#include <map>


#define STATE_COMPARTMENT_NONE   0
#define STATE_COMPARTMENT_RESET  1
#define STATE_COMPARTMENT_SET    2


namespace UiPathTeam
{
    namespace KeyboardExtension
    {
        class Agent
            : public ITfCompartmentEventSink
            , public ITfLanguageProfileNotifySink
        {
        public:

            Agent();
            ~Agent();
            void Initialize();
            void Uninitialize();
            inline void OnCall(const CWPSTRUCT*);
            inline void OnCall2(const CWPSTRUCT*);

            //IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
            virtual ULONG STDMETHODCALLTYPE AddRef(void);
            virtual ULONG STDMETHODCALLTYPE Release(void);

            //ITfCompartmentEventSink
            virtual HRESULT STDMETHODCALLTYPE OnChange(
                /* [in] */ __RPC__in REFGUID rguid);

            //ITfLanguageProfileNotifySink
            virtual HRESULT STDMETHODCALLTYPE OnLanguageChange(
                /* [in] */ LANGID langid,
                /* [out] */ __RPC__out BOOL *pfAccept);
            virtual HRESULT STDMETHODCALLTYPE OnLanguageChanged(void);

            static LRESULT CALLBACK CallWndProcHook(
                _In_ int    nCode,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam);

        private:

            Agent(const Agent&) {}
            void operator =(const Agent&) {}
            bool GetCompartment(const GUID&, ITfCompartment**);
            void UnadviseAllCompartmentEventSinks();
            HRESULT GetCompartmentLong(ITfCompartment*, LONG&);
            HRESULT SetCompartmentLong(ITfCompartment*, LONG);
            PCWSTR GetCompartmentName(ITfCompartment*) const;
            inline bool ForceKeyboardLayout();
            inline void RestoreKeyboardLayout();
            inline void ResetInputSettings();
            inline void RestoreInputSettings();
            inline void ResetKeyboardOpenClose(BYTE&);
            inline void ResetInputModeConversion(BYTE&);
            inline void RestoreKeyboardOpenClose(BYTE&);
            inline void RestoreInputModeConversion(BYTE&);

            ULONG m_ulRefCount;
            bool m_bReleasePending;
            bool m_bOnCall;
            bool m_bEnabled;
            bool m_bInitialized;
            HANDLE m_hIpcMapping;
            Ipc* m_pIpcBlock;
            ITfThreadMgr* m_pTfThreadMgr;
            TfClientId m_TfClientId;
            ITfCompartmentMgr* m_pTfCompartmentMgr;
            ITfCompartment* m_pTfCompartmentKeyboardOpenClose;
            ITfCompartment* m_pTfCompartmentInputModeConversion;
            std::map<ITfCompartment*, DWORD> m_CompartmentEventSinkCookieMap;
            LONG m_KeyboardOpenClose;
            LONG m_InputModeConversion;
            ITfInputProcessorProfiles *m_pInputProcessorProfiles;
            DWORD m_dwLanguageProfileNotifySinkCookie;
            LANGID m_LangId;
            BYTE m_JaState;
            BYTE m_KoState;
            BYTE m_ZhTwState;
            BYTE m_ZhCnState;
            BYTE m_ZhHkState;
            BYTE m_ZhSgState;
            BYTE m_ZhMoState;
        };

        inline void Agent::OnCall(const CWPSTRUCT* pCWPS)
        {
            if (m_pIpcBlock != NULL && !m_bOnCall)
            {
                // This prevents the code from being called recursively.
                m_bOnCall = true;
                OnCall2(pCWPS);
                m_bOnCall = false;
            }
        }

        inline void Agent::OnCall2(const CWPSTRUCT* pCWPS)
        {
            if (pCWPS->message == m_pIpcBlock->m_WM_AGENT_WAKEUP)
            {
                if (pCWPS->wParam == AGENT_ENABLED)
                {
                    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::ENABLED");
                    DBGPUT(L"Started.");
                    m_bEnabled = true;
                    if (!m_bInitialized)
                    {
                        Initialize();
                    }
                    DBGPUT(L"Ended.");
                    //FALLTHROUGH
                }
                else if (pCWPS->wParam == AGENT_DISABLED)
                {
                    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::DISABLED");
                    DBGPUT(L"Started.");
                    m_bEnabled = false;
                    if (m_bInitialized)
                    {
                        RestoreInputSettings();
                        RestoreKeyboardLayout();
                        Uninitialize();
                    }
                    DBGPUT(L"Ended.");
                    return;
                }
            }
            if (m_bEnabled)
            {
                if ((m_pIpcBlock->m_dwFlags & FLAG_DISABLE_IME) == 0)
                {
                    RestoreInputSettings();
                }

                if ((m_pIpcBlock->m_dwFlags & FLAG_FORCE_LAYOUT) == FLAG_FORCE_LAYOUT)
                {
                    ForceKeyboardLayout();
                }
                else
                {
                    RestoreKeyboardLayout();
                }

                if ((m_pIpcBlock->m_dwFlags & FLAG_DISABLE_IME) == FLAG_DISABLE_IME)
                {
                    ResetInputSettings();
                }
            }
        }

        inline bool Agent::ForceKeyboardLayout()
        {
            if (m_LangId == m_pIpcBlock->m_ForcedLangId)
            {
                // Nothing needs to be done.
                return true;
            }
            if (m_pInputProcessorProfiles == NULL)
            {
                return false;
            }
            if (m_pIpcBlock->m_ForcedLangId == (LANGID)0)
            {
                LANGID selected = (LANGID)-1;
                LANGID* pLangIds = NULL;
                ULONG ulLangCount = 0;
                HRESULT hr = m_pInputProcessorProfiles->GetLanguageList(&pLangIds, &ulLangCount);
                if (hr == S_OK)
                {
                    DBGPUT(L"TfInputProcessorProfiles::GetLanguageList: count=%lu", ulLangCount);
                    for (ULONG index = 0; index < ulLangCount; index++)
                    {
                        DBGPUT(L"LANGID=%04x", pLangIds[index]);
                        if (pLangIds[index] == m_pIpcBlock->m_PreferredLangId)
                        {
                            selected = pLangIds[index];
                            break;
                        }
                        else if (selected == (LANGID)-1 && PRIMARYLANGID(pLangIds[index]) == PRIMARYLANGID(m_pIpcBlock->m_PreferredLangId))
                        {
                            selected = pLangIds[index];
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
                }
                InterlockedCompareExchange16(reinterpret_cast<SHORT*>(&m_pIpcBlock->m_ForcedLangId), selected, 0);
                DBGPUT(L"ForcedLangId=%04x", m_pIpcBlock->m_ForcedLangId);
            }
            if (m_pIpcBlock->m_ForcedLangId != (LANGID)-1)
            {
                LANGID current = m_LangId;
                HRESULT hr = m_pInputProcessorProfiles->ChangeCurrentLanguage(m_pIpcBlock->m_ForcedLangId);
                if (hr == S_OK)
                {
                    DBGPUT(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x)", m_pIpcBlock->m_ForcedLangId);
                    InterlockedCompareExchange16(reinterpret_cast<SHORT*>(&m_pIpcBlock->m_LastLangId), current, 0);
                    return true;
                }
                else
                {
                    Debug::Put(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Failed. error=%08lx", m_pIpcBlock->m_ForcedLangId, hr);
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        inline void Agent::RestoreKeyboardLayout()
        {
            if (m_pIpcBlock->m_LastLangId == 0)
            {
                // Nothing needs to be done because the layout was never changed.
            }
            else if (m_LangId != m_pIpcBlock->m_LastLangId && m_pInputProcessorProfiles != NULL)
            {
                HRESULT hr = m_pInputProcessorProfiles->ChangeCurrentLanguage(m_pIpcBlock->m_LastLangId);
                if (hr == S_OK)
                {
                    DBGPUT(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Done.", m_pIpcBlock->m_LastLangId);
                }
                else
                {
                    Debug::Put(L"InputProcessorProfiles::ChangeCurrentLanguage(%04x): Failed. error=%08lx", m_pIpcBlock->m_LastLangId, hr);
                }
            }
        }

        inline void Agent::ResetInputSettings()
        {
            switch (PRIMARYLANGID(m_LangId))
            {
            case LANG_JAPANESE:
                ResetKeyboardOpenClose(m_JaState);
                break;
            case LANG_KOREAN: // turns off Hangul conversion mode
                ResetInputModeConversion(m_KoState);
                break;
            case LANG_CHINESE: // Experimental Support (further investigation is needed)
                switch (SUBLANGID(m_LangId))
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

        inline void Agent::RestoreInputSettings()
        {
            switch (PRIMARYLANGID(m_LangId))
            {
            case LANG_JAPANESE:
                RestoreKeyboardOpenClose(m_JaState);
                break;
            case LANG_KOREAN:
                RestoreInputModeConversion(m_KoState);
                break;
            case LANG_CHINESE: // Experimental Support (further investigation is needed)
                switch (SUBLANGID(m_LangId))
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

        inline void Agent::ResetKeyboardOpenClose(BYTE& state)
        {
            if (state == STATE_COMPARTMENT_NONE)
            {
                state = m_KeyboardOpenClose == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
            }
            if (m_KeyboardOpenClose != 0)
            {
                SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 0); // turns off IME
            }
        }

        inline void Agent::ResetInputModeConversion(BYTE& state)
        {
            if (state == STATE_COMPARTMENT_NONE)
            {
                state = (m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == 0 ? STATE_COMPARTMENT_RESET : STATE_COMPARTMENT_SET;
            }
            if ((m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == TF_CONVERSIONMODE_NATIVE)
            {
                SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_InputModeConversion & ~TF_CONVERSIONMODE_NATIVE); // turns off NATIVE conversion mode
            }
        }

        inline void Agent::RestoreKeyboardOpenClose(BYTE& state)
        {
            if (state != STATE_COMPARTMENT_NONE)
            {
                if (state == STATE_COMPARTMENT_SET && m_KeyboardOpenClose == 0)
                {
                    SetCompartmentLong(m_pTfCompartmentKeyboardOpenClose, 1); // turns on IME
                }
                state = STATE_COMPARTMENT_NONE;
            }
        }

        inline void Agent::RestoreInputModeConversion(BYTE& state)
        {
            if (state != STATE_COMPARTMENT_NONE)
            {
                if (state == STATE_COMPARTMENT_SET && (m_InputModeConversion & TF_CONVERSIONMODE_NATIVE) == 0)
                {
                    SetCompartmentLong(m_pTfCompartmentInputModeConversion, m_InputModeConversion | TF_CONVERSIONMODE_NATIVE); // turns on NATIVE conversion mode
                }
                state = STATE_COMPARTMENT_NONE;
            }
        }
    }
}
