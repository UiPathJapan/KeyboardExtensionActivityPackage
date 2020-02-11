#pragma once


#include "Debug.h"
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
            void OnCall(const CWPSTRUCT*);

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
            void OnCall2(const CWPSTRUCT*);
            bool ForceKeyboardLayout();
            bool RestoreKeyboardLayout();
            void ResetInputSettings();
            void SetInputSettings();
            void RestoreInputSettings();
            void ResetKeyboardOpenClose(BYTE&);
            void SetKeyboardOpenClose(BYTE&);
            void ResetInputModeConversion(BYTE&);
            void SetInputModeConversion(BYTE&);
            void RestoreKeyboardOpenClose(BYTE&);
            void RestoreInputModeConversion(BYTE&);

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
    }
}
