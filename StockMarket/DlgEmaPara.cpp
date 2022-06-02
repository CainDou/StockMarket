#include "stdafx.h"
#include "DlgEmaPara.h"
#include"IniFile.h"
#include<string>

int g_nDefaultEmaPara[2];

CDlgEmaPara::CDlgEmaPara() :SHostWnd(_T("LAYOUT:dlg_emaPara"))
{
}

SOUI::CDlgEmaPara::CDlgEmaPara(RpsGroup rg, HWND hWnd) : SHostWnd(_T("LAYOUT:dlg_emaPara"))
{
	m_rgGroup = rg;
	m_hParWnd = hWnd;
}


CDlgEmaPara::~CDlgEmaPara()
{
}

void SOUI::CDlgEmaPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgEmaPara::OnClickButtonOk()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_EMAPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		nPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
		nPara[0] = -1;

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_EMAPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		nPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
		nPara[1] = -1;

	LPARAM lp = MAKELPARAM(m_rgGroup, FSMSG_EMA);
	//SSendMessage(WM_FENSHI_MSG, (WPARAM)nPara, lp);

	::SendMessageW(m_hParWnd, WM_FENSHI_MSG, (WPARAM)nPara, lp);
	OnClose();
}

void SOUI::CDlgEmaPara::OnClickButtonCancel()
{
	OnClose();

}

void SOUI::CDlgEmaPara::OnInit(EventArgs * e)
{
	CIniFile ini(L".//config//config.ini");
	g_nDefaultEmaPara[0] = ini.GetInt(L"DefaultPara", L"EMAPara1", 12);
	g_nDefaultEmaPara[1] = ini.GetInt(L"DefaultPara", L"EMAPara2", 26);
}

void SOUI::CDlgEmaPara::OnClickButtonDefault()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_EMAPara1");
	para1->SetWindowTextW(std::to_wstring(g_nDefaultEmaPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_EMAPara2");
	para2->SetWindowTextW(std::to_wstring(g_nDefaultEmaPara[1]).c_str());
}

void SOUI::CDlgEmaPara::OnClickButtonSetDefault()
{
	int tmpPara[2];
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_EMAPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		tmpPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数1", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_EMAPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		tmpPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数2", L"错误", MB_ICONWARNING);
		return;
	}

	g_nDefaultEmaPara[0] = tmpPara[0];
	g_nDefaultEmaPara[1] = tmpPara[1];

	CIniFile ini(L".\\config\\config.ini");
	ini.WriteInt(L"DefaultPara", L"EMAPara1", g_nDefaultEmaPara[0]);
	ini.WriteInt(L"DefaultPara", L"EMAPara2", g_nDefaultEmaPara[1]);

	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);
}

void SOUI::CDlgEmaPara::SetEditText(const int * pPara)
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_EMAPara1");
	para1->SetWindowTextW(std::to_wstring(pPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_EMAPara2");
	para2->SetWindowTextW(std::to_wstring(pPara[1]).c_str());
}

void SOUI::CDlgEmaPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
