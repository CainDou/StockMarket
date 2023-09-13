#pragma once
#include"NetWorkClient.h"

namespace SOUI
{
	class CDlgRegister : public SHostWnd
	{
	public:
		CDlgRegister(HWND hParWnd, eAccountOpt optType);
		~CDlgRegister();
		void OnBtnClose();
		void OnBtnOK();
		void OnInit(EventArgs *e);
		bool OnEditIDChange(EventArgs *e);
		bool OnEditSrcPsdChange(EventArgs *e);
		bool OnEditPsdChange(EventArgs *e);
		bool OnEditPsdCfmChange(EventArgs *e);

	protected:
		virtual void OnFinalMessage(HWND hWnd);
		LRESULT  OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgRegister)
			MESSAGE_HANDLER(WM_ACCOUNT_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	protected:
		SEdit* m_pEditID;
		SEdit* m_pEditSrcPsd;
		SEdit* m_pEditPsd;
		SEdit* m_pEditPsdCfm;
		SStatic* m_pTxtIDInfo;
		SStatic* m_pTxtSrcPsdInfo;
		SStatic* m_pTxtPsdInfo;
		SStatic* m_pTxtPsdCfmInfo;
		SStatic* m_pTxtSrcPsd;
		SStatic* m_pTxtPsd;
		SStatic* m_pTitle;

	protected:
		bool m_bIDValid;
		bool m_bPsdValid;
		bool m_bPsdCfmSame;
		eAccountOpt m_optType;
		unsigned m_uSynThreadID;

		HWND m_hParWnd;
	};

}