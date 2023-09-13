#pragma once
#include"NetWorkClient.h"

namespace SOUI
{
	class CDlgLogin : public SHostDialog
	{
	public:
		CDlgLogin();
		CDlgLogin(HWND hParWnd, CNetWorkClient *pNetClient);
		~CDlgLogin();
		void OnClickButtonLogin();
		void OnClickButtonMarket();
		void OnClickButtonCancel();
		void OnClickButtonIDList();
		void OnInit(EventArgs *e);
		void SetIPInfo(SStringA strIPAddr, int nIPPort);
		void InitConfig();
		void SaveConfig();
		bool OnLbIDLButtonDown(EventArgs *e);
		void OnBtnRegister();
		void OnBtnChangePsd();

	protected:
		virtual void OnFinalMessage(HWND hWnd);
		LRESULT  OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_market", OnClickButtonMarket)
			EVENT_NAME_COMMAND(L"btn_quit", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_close", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_IDList", OnClickButtonIDList)
			EVENT_NAME_COMMAND(L"btn_register", OnBtnRegister)
			EVENT_NAME_COMMAND(L"btn_changePsd", OnBtnChangePsd)
			EVENT_NAME_COMMAND(L"btn_login", OnClickButtonLogin)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgLogin)
			MESSAGE_HANDLER(WM_LOGIN_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	protected:
		SStringW strID;
		SStringW strPsd;

		SStringA m_strIPAddr;
		int m_nIPPort;

		SStatic * m_pLoginTxt;
		HWND	  m_hParWnd;

		BOOL m_bRelogin;

		CNetWorkClient * m_pNetClient;
		set<SStringA> m_IDSet;
		bool m_bAutoLogin;
		bool m_bRemberPsd;
		SStringA m_strLastID;
		SStringA m_strLastPsd;
		unsigned m_SynThreadID;
	protected:
		SEdit* m_pEditAccout;
		SEdit* m_pEditPsd;
		SCheckBox * m_pChkRember;
		SCheckBox * m_pChkAutoLogin;
		SListBox* m_pLbIDList;

	};

	inline void SOUI::CDlgLogin::SetIPInfo(SStringA strIPAddr, int nIPPort)
	{
		m_strIPAddr = strIPAddr;
		m_nIPPort = nIPPort;
	}
}
