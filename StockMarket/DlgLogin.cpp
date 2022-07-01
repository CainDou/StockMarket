#include "stdafx.h"
#include "DlgLogin.h"
#include<process.h>

extern HANDLE g_hLoginEvent;

SOUI::CDlgLogin::CDlgLogin() :SHostDialog(_T("LAYOUT:dlg_login"))
{
}

SOUI::CDlgLogin::CDlgLogin(HWND hParWnd, CNetWorkClient *pNetClient) :
	SHostDialog(_T("LAYOUT:dlg_login")), m_hParWnd(hParWnd), m_pNetClient(pNetClient)
{
}


SOUI::CDlgLogin::~CDlgLogin()
{
}


void SOUI::CDlgLogin::OnClickButtonOk()
{

	SButton * pButton = FindChildByName2<SButton>(L"btn_login");
	SEdit * pEditID = FindChildByName2<SEdit>(L"login_ID");
	pButton->EnableWindow(FALSE,TRUE);
	pButton->Invalidate();
	if (pEditID)
		strID = pEditID->GetWindowText();
	SEdit *pEditPsd = FindChildByName2<SEdit>(L"login_PassWord");
	if (pEditPsd)
		strPsd = pEditPsd->GetWindowText();
	if (m_pNetClient->GetState())
	{
		m_bLogin = true;
		SetEvent(g_hLoginEvent);
	}
	else
	{
		m_pLoginTxt->SetWindowTextW(L"正在连接服务器");
		m_pLoginTxt->Invalidate();
		TraceLog("开始连接服务器");
		m_pNetClient->SetWndHandle(m_hParWnd);
		//m_pNetClient->m_socket = INVALID_SOCKET;
		if (m_pNetClient->OnConnect(m_strIPAddr, m_nIPPort))
		{
			m_bLogin = true;
			SetEvent(g_hLoginEvent);
		}
		else
		{
			m_bLogin = false;
			m_pLoginTxt->SetWindowTextW(L"服务器连接失败，请重试");
			pButton->EnableWindow(TRUE);
			pButton->Invalidate();
		}
	}

}

void SOUI::CDlgLogin::OnClickButtonCancel()
{
	LoginInfo_t loginInfo = { 0 };
	loginInfo.loginwnd = m_hWnd;
	loginInfo.bLogin = false;

	//::PostMessage(m_hParWnd, WM_MAINMSG, (WPARAM)&loginInfo, MainMsg_Login);
	//CSimpleWnd::DestroyWindow();

	//m_bLogin = false;
	//SetEvent(g_hLoginEvent);
	TraceLog("关闭程序");
	m_pNetClient->Stop();
	//g_bExit = TRUE;
	exit(-1);
}

void SOUI::CDlgLogin::OnInit(EventArgs * e)
{
	m_pLoginTxt = FindChildByName2<SStatic>(L"txt_login");
	assert(m_pLoginTxt);
	//	m_pLoginTxt->SetWindowTextW(L"正在连接服务器");
	m_bLogin = false;
}

LRESULT SOUI::CDlgLogin::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nlp = (int)lp;
	if (nlp == LoginMsg_UpdateText)
	{
		wchar_t *pMsg = (wchar_t *)wp;
		if (m_pLoginTxt)
			m_pLoginTxt->SetWindowTextW(pMsg);
	}
	else if (nlp == LoginMsg_HideWnd)
		ShowWindow(SW_HIDE);
	else if (nlp == LoginMsg_WaitAndTry)
	{
		SButton * pButton = FindChildByName2<SButton>(L"btn_login");
		m_bLogin = false;
		m_pLoginTxt->SetWindowTextW(L"服务器正在初始化，请稍后重试...");
		pButton->EnableWindow(TRUE);
		pButton->Invalidate();
	}
	else if (nlp == LoginMsg_Exit)
		EndDialog(0);
	else if (nlp == LoginMsg_Reinit)
	{
		ShowWindow(SW_SHOW);
		OnClickButtonOk();
	}
	return 0;
}


void SOUI::CDlgLogin::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
