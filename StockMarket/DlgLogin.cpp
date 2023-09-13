#include "stdafx.h"
#include "DlgLogin.h"
#include<process.h>
#include "IniFile.h"
#include "WndSynHandler.h"
#include "MD5.h"
#include "DlgRegister.h"

extern HANDLE g_hLoginEvent;
extern HANDLE g_hEvent;
extern CWndSynHandler g_WndSyn;

CDlgLogin::CDlgLogin() :SHostDialog(_T("LAYOUT:dlg_login"))
{
	m_bRelogin = false;
}

CDlgLogin::CDlgLogin(HWND hParWnd, CNetWorkClient *pNetClient) :
	SHostDialog(_T("LAYOUT:dlg_login")), m_hParWnd(hParWnd), m_pNetClient(pNetClient)
{
	m_bRelogin = false;
}


CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::OnClickButtonLogin()
{
	if (!m_pNetClient->GetConnectState())
	{
		if (!m_pNetClient->OnConnect(m_strIPAddr, m_nIPPort))
		{
			SMessageBox(m_hWnd, L"连接服务器失败，请稍后再试...", L"提示", MB_OK);
			return;
		}
	}
	if (m_pEditAccout->GetWindowTextW().IsEmpty())
	{
		SMessageBox(m_hWnd, L"请输入账号!", L"提示", MB_OK);
		m_pEditAccout->SetFocus();
		return;
	}
	if (m_pEditPsd->GetWindowTextW().IsEmpty())
	{
		SMessageBox(m_hWnd, L"请输入密码!", L"提示", MB_OK);
		m_pEditPsd->SetFocus();
		return;
	}

	SStringA strLoginInfo;
	MD5 md5Handler;
	SStringA strID = StrW2StrA(m_pEditAccout->GetWindowTextW());
	SStringA strPsd = StrW2StrA(m_pEditPsd->GetWindowTextW());
	md5Handler.update(string(strPsd.GetBuffer(0)));
	SStringA strPsdMd5 = md5Handler.toString().c_str();
	strLoginInfo.Format("ID:%s,PsdMd5:%s;", strID, strPsdMd5);
	m_strLastID = strID;
	m_strLastPsd = strPsd;
	SaveConfig();
	SButton * pButton = FindChildByName2<SButton>(L"btn_login");
	pButton->EnableWindow(FALSE, TRUE);
	pButton = FindChildByName2<SButton>(L"btn_market");
	pButton->EnableWindow(FALSE, TRUE);

	SendMsg(m_SynThreadID, TradeSyn_Login,
		strLoginInfo.GetBuffer(0), strLoginInfo.GetLength() + 1);


}


void CDlgLogin::OnClickButtonMarket()
{

	if (!m_pNetClient->GetConnectState())
	{
		if (!m_pNetClient->OnConnect(m_strIPAddr, m_nIPPort))
		{
			SMessageBox(m_hWnd, L"连接服务器失败，请稍后再试...", L"提示", MB_OK);
			return;
		}
	}

	SButton * pButton = FindChildByName2<SButton>(L"btn_login");
	pButton->EnableWindow(FALSE, TRUE);
	pButton = FindChildByName2<SButton>(L"btn_market");
	pButton->EnableWindow(FALSE, TRUE);
	SetEvent(g_hLoginEvent);

}

void CDlgLogin::OnClickButtonCancel()
{
	if (!m_bRelogin)
	{
		LoginInfo_t loginInfo = { 0 };
		loginInfo.loginwnd = m_hWnd;
		loginInfo.bLogin = false;

		SetEvent(g_hEvent);
		TraceLog("关闭程序");
		m_pNetClient->Stop();
		//g_bExit = TRUE;
		EndDialog(-1);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
	//exit(0);
}

void CDlgLogin::OnClickButtonIDList()
{
	if (m_pLbIDList->IsVisible())
		m_pLbIDList->SetVisible(TRUE, TRUE);
	else
		m_pLbIDList->SetVisible(FALSE, TRUE);

}

void CDlgLogin::OnInit(EventArgs * e)
{
	m_pLoginTxt = FindChildByName2<SStatic>(L"txt_login");
	m_pEditAccout = FindChildByName2<SEdit>(L"edit_ID");
	m_pEditPsd = FindChildByName2<SEdit>(L"edit_Psd");
	m_pChkRember = FindChildByName2<SCheckBox>(L"chk_rember");
	m_pChkAutoLogin = FindChildByName2<SCheckBox>(L"chk_autoLogin");
	m_pLbIDList = FindChildByName2<SListBox>(L"lb_IDList");
	m_pLbIDList->GetEventSet()->subscribeEvent(EventLButtonDown::EventID,
		Subscriber(&CDlgLogin::OnLbIDLButtonDown, this));
	InitConfig();
	for (auto &it : m_IDSet)
		m_pLbIDList->InsertString(0, StrA2StrW(it));
	m_pEditAccout->SetWindowTextW(StrA2StrW(m_strLastID));
	if (m_bRemberPsd)
		m_pEditPsd->SetWindowTextW(StrA2StrW(m_strLastPsd));
	m_pChkRember->SetCheck(m_bRemberPsd);
	m_pChkAutoLogin->SetCheck(m_bAutoLogin);
	m_SynThreadID = g_WndSyn.GetTradeThreadID();

	m_pNetClient->SetWndHandle(m_hParWnd);
	//m_pNetClient->m_socket = INVALID_SOCKET;
	if (!m_pNetClient->GetConnectState())
		if (!m_pNetClient->OnConnect(m_strIPAddr, m_nIPPort))
			m_pLoginTxt->SetWindowTextW(L"服务器连接失败...");
	if (m_bAutoLogin)
		OnClickButtonLogin();

}

LRESULT CDlgLogin::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nMsg = (int)lp;
	if (nMsg == LoginMsg_UpdateText)
	{
		wchar_t *pMsg = (wchar_t *)wp;
		if (m_pLoginTxt)
			m_pLoginTxt->SetWindowTextW(pMsg);
	}
	else if (nMsg == LoginMsg_HideWnd)
	{
		SButton * pButton = FindChildByName2<SButton>(L"btn_login");
		pButton->EnableWindow(TRUE, TRUE);
		pButton = FindChildByName2<SButton>(L"btn_market");
		pButton->EnableWindow(FALSE, TRUE);
		ShowWindow(SW_HIDE);

	}
	else if (nMsg == LoginMsg_WaitAndTry)
	{
		SButton * pButton = FindChildByName2<SButton>(L"btn_login");
		m_pLoginTxt->SetWindowTextW(L"服务器正在初始化，请稍后重试...");
		pButton->EnableWindow(TRUE);
		pButton->Invalidate();
	}
	else if (nMsg == LoginMsg_Exit)
		EndDialog(0);
	else if (nMsg == LoginMsg_Reinit)
	{
		ShowWindow(SW_SHOW);
		OnClickButtonMarket();
	}
	else if (nMsg == LoginMsg_TradeLoginFeedBack)
	{
		int nFeedback = (int)wp;
		if (TLFB_Success == nFeedback)
		{
			if (!m_bRelogin)
			{
				m_pLoginTxt->SetWindowTextW(L"账号登陆成功,获取行情数据...");
				OnClickButtonMarket();
			}
			else
			{
				ShowWindow(SW_HIDE);
				SButton * pButton = FindChildByName2<SButton>(L"btn_login");
				pButton->EnableWindow(TRUE, TRUE);

			}

		}
		else if (TLFB_Unsuccess == nFeedback)
		{
			SMessageBox(m_hWnd, L"账号密码不匹配，请检查后再试!", L"提示", MB_OK | MB_ICONINFORMATION);
			SButton * pButton = FindChildByName2<SButton>(L"btn_login");
			pButton->EnableWindow(TRUE, TRUE);
			pButton = FindChildByName2<SButton>(L"btn_market");
			pButton->EnableWindow(TRUE, TRUE);

		}
	}
	else if (nMsg == LoginMsg_ReLogin)
	{
		ShowWindow(SW_SHOW);
		m_bRelogin = TRUE;
	}
	return 0;
}

void CDlgLogin::InitConfig()
{
	CIniFile ini(".\\config\\login.cfg");
	int nIDCount = ini.GetIntA("IDList", "Count", 0);
	SStringA str;
	for (int i = 0; i < nIDCount; ++i)
	{
		str = ini.GetStringA("IDList", str.Format("%d", i), "");
		if (!str.IsEmpty())
			m_IDSet.insert(str);
	}
	m_bRemberPsd = ini.GetIntA("Setting", "RemberPsd", 0) == 0 ? false : true;
	m_bAutoLogin = ini.GetIntA("Setting", "AutoLogin", 0) == 0 ? false : true;

	m_strLastID = ini.GetStringA("LastLogin", "ID", "");
	m_strLastPsd = ini.GetStringA("LastLogin", "Psd", "");
}

void CDlgLogin::SaveConfig()
{
	CIniFile ini(".\\config\\login.cfg");
	ini.WriteIntA("IDList", "Count", m_IDSet.size());
	SStringA str;
	int nPos = 0;
	SStringA strNowID = StrW2StrA(m_pEditAccout->GetWindowTextW());
	m_IDSet.insert(strNowID);
	for (auto &it : m_IDSet)
		ini.WriteStringA("IDList", str.Format("%d", nPos++), it);
	ini.WriteIntA("Setting", "RemberPsd", m_pChkRember->IsChecked());
	ini.WriteIntA("Setting", "AutoLogin", m_pChkAutoLogin->IsChecked());

	ini.WriteStringA("LastLogin", "ID", m_strLastID);
	ini.WriteStringA("LastLogin", "Psd", m_strLastPsd);
}

bool CDlgLogin::OnLbIDLButtonDown(EventArgs * e)
{
	EventLButtonDown* pEvt = (EventLButtonDown*)e;
	SListBox* pList = (SListBox*)pEvt->sender;
	if (pList->GetCount() <= 0)
	{
		pList->SetVisible(FALSE, TRUE);
		return true;
	}
	int nSel = pList->GetCurSel();
	SStringW str = pList->GetText(nSel);
	pList->SetVisible(FALSE, TRUE);
	m_pEditAccout->SetWindowTextW(str);
	m_pEditPsd->SetWindowTextW(L"");
	m_pEditPsd->SetFocus();
	return true;
}

void CDlgLogin::OnBtnRegister()
{
	CDlgRegister * pDlg = new CDlgRegister(m_hWnd, eAO_Register);
	pDlg->Create(NULL);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);

	::EnableWindow(m_hWnd, FALSE);

}

void CDlgLogin::OnBtnChangePsd()
{
	CDlgRegister * pDlg = new CDlgRegister(m_hWnd, eAO_ChangePsd);
	pDlg->Create(NULL);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);

	::EnableWindow(m_hWnd, FALSE);

}


void CDlgLogin::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
