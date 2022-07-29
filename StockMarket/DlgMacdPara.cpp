#include "stdafx.h"
#include "DlgMacdPara.h"
#include"IniFile.h"
#include<string>

int g_nDefaultMacdPara[3];

CDlgMacdPara::CDlgMacdPara() :SHostWnd(_T("LAYOUT:dlg_macdPara"))
{
}

CDlgMacdPara::CDlgMacdPara(RpsGroup rg, HWND hWnd) : SHostWnd(_T("LAYOUT:dlg_macdPara"))
{
	m_rgGroup = rg;
	m_hParWnd = hWnd;
}


CDlgMacdPara::~CDlgMacdPara()
{
}

void CDlgMacdPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CDlgMacdPara::OnClickButtonOk()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_MacdPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		m_nMacdPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
		m_nMacdPara[0] = -1;

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_MacdPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		m_nMacdPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
		m_nMacdPara[1] = -1;
	SEdit *para3 = FindChildByName2<SEdit>(L"edit_MacdPara3");
	strPara3 = para3->GetWindowTextW();
	if (strPara3 != L"")
		m_nMacdPara[2] = _wtoi(strPara3.GetBuffer(1));
	else
		m_nMacdPara[2] = -1;

	::SendMessageW(m_hParWnd, WM_FENSHI_MSG, 
		(WPARAM)m_nMacdPara, FSMSG_MACD);

	::SendMessageW(m_hParWnd, WM_KLINE_MSG, 
		(WPARAM)m_nMacdPara, KLINEMSG_MACD);
	OnClose();
}

void CDlgMacdPara::OnClickButtonCancel()
{
	OnClose();
}

void CDlgMacdPara::OnInit(EventArgs * e)
{
	CIniFile ini(L".//config//config.ini");
	g_nDefaultMacdPara[0] = ini.GetInt(L"DefaultPara", L"MACDPara1", 12);
	g_nDefaultMacdPara[1] = ini.GetInt(L"DefaultPara", L"MACDPara2", 26);
	g_nDefaultMacdPara[2] = ini.GetInt(L"DefaultPara", L"MACDPara3", 9);

}

void CDlgMacdPara::OnClickButtonDefault()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_MacdPara1");
	para1->SetWindowTextW(std::to_wstring(g_nDefaultMacdPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_MacdPara2");
	para2->SetWindowTextW(std::to_wstring(g_nDefaultMacdPara[1]).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_MacdPara3");
	para3->SetWindowTextW(std::to_wstring(g_nDefaultMacdPara[2]).c_str());
}

void CDlgMacdPara::OnClickButtonSetDefault()
{
	int tmpPara[3];
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_MacdPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		tmpPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数1", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_MacdPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		tmpPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数2", L"错误", MB_ICONWARNING);
		return;
	}
	SEdit *para3 = FindChildByName2<SEdit>(L"edit_MacdPara3");
	strPara3 = para3->GetWindowTextW();
	if (strPara3 != L"")
		tmpPara[2] = _wtoi(strPara3.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数3", L"错误", MB_ICONWARNING);
		return;
	}

	g_nDefaultMacdPara[0] = tmpPara[0];
	g_nDefaultMacdPara[1] = tmpPara[1];
	g_nDefaultMacdPara[2] = tmpPara[2];

	CIniFile ini(L".\\config\\config.ini");
	ini.WriteInt(L"DefaultPara", L"MACDPara1", g_nDefaultMacdPara[0]);
	ini.WriteInt(L"DefaultPara", L"MACDPara2", g_nDefaultMacdPara[1]);
	ini.WriteInt(L"DefaultPara", L"MACDPara3", g_nDefaultMacdPara[2]);

	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);

}

void CDlgMacdPara::SetEditText(const int * pPara)
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_MacdPara1");
	para1->SetWindowTextW(std::to_wstring(pPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_MacdPara2");
	para2->SetWindowTextW(std::to_wstring(pPara[1]).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_MacdPara3");
	para3->SetWindowTextW(std::to_wstring(pPara[2]).c_str());
}

void CDlgMacdPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
