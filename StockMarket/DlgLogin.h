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
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void SetIPInfo(SStringA strIPAddr, int nIPPort);

		SStringW strID;
		SStringW strPsd;

		SStringA m_strIPAddr;
		int m_nIPPort;

		SStatic * m_pLoginTxt;
		HWND	  m_hParWnd;

		bool m_bLogin;

		CNetWorkClient * m_pNetClient;
	protected:
		virtual void OnFinalMessage(HWND hWnd);
		LRESULT  OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);


	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_login", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_quit", OnClickButtonCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgLogin)
			MESSAGE_HANDLER(WM_LOGIN_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()


	};

	inline void SOUI::CDlgLogin::SetIPInfo(SStringA strIPAddr, int nIPPort)
	{
		m_strIPAddr = strIPAddr;
		m_nIPPort = nIPPort;
	}
}
