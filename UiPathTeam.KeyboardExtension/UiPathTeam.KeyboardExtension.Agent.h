#pragma once


#include "Debug.h"
#include "Platform.h"
#include "UiPathTeam.KeyboardExtension.h"
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
            inline bool RestoreKeyboardLayout();
            inline void ResetInputSettings();
            inline void SetInputSettings();
            inline void RestoreInputSettings();
            inline void ResetKeyboardOpenClose(BYTE&);
            inline void SetKeyboardOpenClose(BYTE&);
            inline void ResetInputModeConversion(BYTE&);
            inline void SetInputModeConversion(BYTE&);
            inline void RestoreKeyboardOpenClose(BYTE&);
            inline void RestoreInputModeConversion(BYTE&);

            ULONG m_ulRefCount;
            bool m_bReleasePending;
            bool m_bOnCall;
            bool m_bInitialized;
            AgentIpcPtr m_pAgentIpc;
            IpcPtr<DesktopIpc> m_pDesktopIpc;
            ITfThreadMgr* m_pTfThreadMgr;
            TfClientId m_TfClientId;
            ITfCompartmentMgr* m_pTfCompartmentMgr;
            ITfCompartment* m_pTfCompartmentKeyboardOpenClose;
            ITfCompartment* m_pTfCompartmentInputModeConversion;
            std::map<ITfCompartment*, DWORD> m_CompartmentEventSinkCookieMap;
            ITfInputProcessorProfiles *m_pInputProcessorProfiles;
            DWORD m_dwLanguageProfileNotifySinkCookie;
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
            if (!m_bOnCall && m_pAgentIpc)
            {
                // This prevents the code from being called recursively.
                m_bOnCall = true;
                OnCall2(pCWPS);
                m_bOnCall = false;
            }
        }

        inline void Agent::OnCall2(const CWPSTRUCT* pCWPS)
        {
            if (pCWPS->message == m_pDesktopIpc->m_WM_AGENT_WAKEUP)
            {
                if (pCWPS->wParam == AGENT_INITIALIZE)
                {
                    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::INITIALIZE");
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
                    DBGFNC(L"UiPathTeam::KeyboardExtension::Agent::UNINITIALIZE");
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

        inline bool Agent::ForceKeyboardLayout()
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

        inline bool Agent::RestoreKeyboardLayout()
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

        inline void Agent::ResetInputSettings()
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

        inline void Agent::SetInputSettings()
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

        inline void Agent::RestoreInputSettings()
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

        inline void Agent::ResetKeyboardOpenClose(BYTE& state)
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

        inline void Agent::SetKeyboardOpenClose(BYTE& state)
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

        inline void Agent::ResetInputModeConversion(BYTE& state)
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

        inline void Agent::SetInputModeConversion(BYTE& state)
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

        inline void Agent::RestoreKeyboardOpenClose(BYTE& state)
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

        inline void Agent::RestoreInputModeConversion(BYTE& state)
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
    }
}
