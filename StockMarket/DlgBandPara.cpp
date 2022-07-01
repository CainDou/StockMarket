#include "stdafx.h"
#include "DlgBandPara.h"
#include"IniFile.h"
#include<string>

BandPara_t g_DefaultBandPara;

CDlgBandPara::CDlgBandPara() :SHostWnd(_T("LAYOUT:dlg_BandPara"))
{
}

SOUI::CDlgBandPara::CDlgBandPara(RpsGroup rg, HWND hWnd) : SHostWnd(_T("LAYOUT:dlg_BandPara"))
{
	m_rgGroup = rg;
	m_hParWnd = hWnd;
}


CDlgBandPara::~CDlgBandPara()
{
}

void SOUI::CDlgBandPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgBandPara::OnClickButtonOk()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_BandParaN1");
	strParaN1 = para1->GetWindowTextW();
	if (strParaN1 != L"")
		m_BandPara.N1 = _wtoi(strParaN1.GetBuffer(1));
	else
		m_BandPara.N1 = -1;

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_BandParaN2");
	strParaN2 = para2->GetWindowTextW();
	if (strParaN2 != L"")
		m_BandPara.N2 = _wtoi(strParaN2.GetBuffer(1));
	else
		m_BandPara.N2 = -1;


	SEdit *para3 = FindChildByName2<SEdit>(L"edit_BandParaK");
	strParaK = para3->GetWindowTextW();
	if (strParaK != L"")
		m_BandPara.K = _wtoi(strParaK.GetBuffer(1));
	else
		m_BandPara.K = -1;

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_BandParaM1");
	strParaM1 = para4->GetWindowTextW();
	if (strParaM1 != L"")
		m_BandPara.M1 = _wtoi(strParaM1.GetBuffer(1));
	else
		m_BandPara.M1 = -1;

	SEdit *para5 = FindChildByName2<SEdit>(L"edit_BandParaM2");
	strParaM2 = para5->GetWindowTextW();
	if (strParaM2 != L"")
		m_BandPara.M2 = _wtoi(strParaM2.GetBuffer(1));
	else
		m_BandPara.M2 = -1;

	SEdit *para6 = FindChildByName2<SEdit>(L"edit_BandParaP");
	strParaP = para6->GetWindowTextW();
	if (strParaP != L"")
		m_BandPara.P = _wtoi(strParaP.GetBuffer(1));
	else
		m_BandPara.P = -1;
	LPARAM lp = MAKELPARAM(m_rgGroup, KLINEMSG_BAND);
	//SSendMessage(WM_KLINE_MSG, (WPARAM)&m_BandPara, lp);

	::SendMessageW(m_hParWnd, WM_KLINE_MSG, (WPARAM)&m_BandPara, lp);
	OnClose();

}

void SOUI::CDlgBandPara::OnClickButtonCancel()
{
	OnClose();

}

void SOUI::CDlgBandPara::OnInit(EventArgs * e)
{
	CIniFile ini(L".//config//config.ini");

	g_DefaultBandPara.N1 = ini.GetInt(L"DefaultPara", L"BandParaN1", 8);
	g_DefaultBandPara.N2 = ini.GetInt(L"DefaultPara", L"BandParaN2", 11);
	g_DefaultBandPara.K = ini.GetInt(L"DefaultPara", L"BandParaK", 390);
	g_DefaultBandPara.M1 = ini.GetInt(L"DefaultPara", L"BandParaM1", 8);
	g_DefaultBandPara.M2 = ini.GetInt(L"DefaultPara", L"BandParaM2", 4);
	g_DefaultBandPara.P = ini.GetInt(L"DefaultPara", L"BandParaP", 15);

}

void SOUI::CDlgBandPara::OnClickButtonDefault()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_BandParaN1");
	para1->SetWindowTextW(std::to_wstring(g_DefaultBandPara.N1).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_BandParaN2");
	para2->SetWindowTextW(std::to_wstring(g_DefaultBandPara.N2).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_BandParaK");
	para3->SetWindowTextW(std::to_wstring(g_DefaultBandPara.K).c_str());

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_BandParaM1");
	para4->SetWindowTextW(std::to_wstring(g_DefaultBandPara.M1).c_str());

	SEdit *para5 = FindChildByName2<SEdit>(L"edit_BandParaM2");
	para5->SetWindowTextW(std::to_wstring(g_DefaultBandPara.M2).c_str());

	SEdit *para6 = FindChildByName2<SEdit>(L"edit_BandParaP");
	para6->SetWindowTextW(std::to_wstring(g_DefaultBandPara.P).c_str());

}

void SOUI::CDlgBandPara::OnClickButtonSetDefault()
{
	BandPara_t tmpBandPara;
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_BandParaN1");
	strParaN1 = para1->GetWindowTextW();
	if (strParaN1 != L"")
		tmpBandPara.N1 = _wtoi(strParaN1.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数N1", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_BandParaN2");
	strParaN2 = para2->GetWindowTextW();
	if (strParaN2 != L"")
		tmpBandPara.N2 = _wtoi(strParaN2.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数N2", L"错误", MB_ICONWARNING);
		return;
	}


	SEdit *para3 = FindChildByName2<SEdit>(L"edit_BandParaK");
	strParaK = para3->GetWindowTextW();
	if (strParaK != L"")
		tmpBandPara.K = _wtoi(strParaK.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数K", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_BandParaM1");
	strParaM1 = para4->GetWindowTextW();
	if (strParaM1 != L"")
		tmpBandPara.M1 = _wtoi(strParaM1.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数M1", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para5 = FindChildByName2<SEdit>(L"edit_BandParaM2");
	strParaM2 = para5->GetWindowTextW();
	if (strParaM2 != L"")
		tmpBandPara.M2 = _wtoi(strParaM2.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数M2", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para6 = FindChildByName2<SEdit>(L"edit_BandParaP");
	strParaP = para6->GetWindowTextW();
	if (strParaP != L"")
		tmpBandPara.P = _wtoi(strParaP.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入参数P", L"错误", MB_ICONWARNING);
		return;
	}

	g_DefaultBandPara = tmpBandPara;

	CIniFile ini(L".\\config\\config.ini");
	ini.WriteInt(L"DefaultPara", L"BandParaN1", g_DefaultBandPara.N1);
	ini.WriteInt(L"DefaultPara", L"BandParaN2", g_DefaultBandPara.N2);
	ini.WriteInt(L"DefaultPara", L"BandParaK", g_DefaultBandPara.K);
	ini.WriteInt(L"DefaultPara", L"BandParaM1", g_DefaultBandPara.M1);
	ini.WriteInt(L"DefaultPara", L"BandParaM2", g_DefaultBandPara.M2);
	ini.WriteInt(L"DefaultPara", L"BandParaP", g_DefaultBandPara.P);

	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);
}

void SOUI::CDlgBandPara::SetEditText(BandPara_t para)
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_BandParaN1");
	para1->SetWindowTextW(std::to_wstring(para.N1).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_BandParaN2");
	para2->SetWindowTextW(std::to_wstring(para.N2).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_BandParaK");
	para3->SetWindowTextW(std::to_wstring(para.K).c_str());

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_BandParaM1");
	para4->SetWindowTextW(std::to_wstring(para.M1).c_str());

	SEdit *para5 = FindChildByName2<SEdit>(L"edit_BandParaM2");
	para5->SetWindowTextW(std::to_wstring(para.M2).c_str());

	SEdit *para6 = FindChildByName2<SEdit>(L"edit_BandParaP");
	para6->SetWindowTextW(std::to_wstring(para.P).c_str());
}

void SOUI::CDlgBandPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
