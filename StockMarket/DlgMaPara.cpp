#include "stdafx.h"
#include "DlgMaPara.h"
#include <string>
#include"IniFile.h"

int g_nDefaultMaPara[4];


CDlgMaPara::CDlgMaPara() :SHostWnd(_T("LAYOUT:dlg_MaPara"))
{
}

CDlgMaPara::CDlgMaPara(RpsGroup rg, HWND hWnd) : SHostWnd(_T("LAYOUT:dlg_MaPara"))
{
	m_rgGroup = rg;
	m_hParWnd = hWnd;
}


CDlgMaPara::~CDlgMaPara()
{
}

void CDlgMaPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CDlgMaPara::OnClickButtonOk()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_maPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		nPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
		nPara[0] = -1;

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_maPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		nPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
		nPara[1] = -1;

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_maPara3");
	strPara3 = para3->GetWindowTextW();
	if (strPara3 != L"")
		nPara[2] = _wtoi(strPara3.GetBuffer(1));
	else
		nPara[2] = -1;

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_maPara4");
	strPara4 = para4->GetWindowTextW();
	if (strPara2 != L"")
		nPara[3] = _wtoi(strPara4.GetBuffer(1));
	else
		nPara[3] = -1;
	::SendMessageW(m_hParWnd, WM_KLINE_MSG, (WPARAM)nPara, KLINEMSG_MA);
	OnClose();

}

void CDlgMaPara::OnClickButtonCancel()
{
	OnClose();
}

void CDlgMaPara::OnInit(EventArgs * e)
{
	CIniFile ini(L".//config//config.ini");
	g_nDefaultMaPara[0] = ini.GetInt(L"DefaultPara", L"MAPara1", 5);
	g_nDefaultMaPara[1] = ini.GetInt(L"DefaultPara", L"MAPara2", 10);
	g_nDefaultMaPara[2] = ini.GetInt(L"DefaultPara", L"MAPara3", 20);
	g_nDefaultMaPara[3] = ini.GetInt(L"DefaultPara", L"MAPara4", 60);

}

void CDlgMaPara::OnClickButtonDefault()
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_maPara1");
	para1->SetWindowTextW(std::to_wstring(g_nDefaultMaPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_maPara2");
	para2->SetWindowTextW(std::to_wstring(g_nDefaultMaPara[1]).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_maPara3");
	para3->SetWindowTextW(std::to_wstring(g_nDefaultMaPara[2]).c_str());

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_maPara4");
	para4->SetWindowTextW(std::to_wstring(g_nDefaultMaPara[3]).c_str());
}

void CDlgMaPara::OnClickButtonSetDefault()
{
	int tmpPara[4];
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_maPara1");
	strPara1 = para1->GetWindowTextW();
	if (strPara1 != L"")
		tmpPara[0] = _wtoi(strPara1.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入周期1", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_maPara2");
	strPara2 = para2->GetWindowTextW();
	if (strPara2 != L"")
		tmpPara[1] = _wtoi(strPara2.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入周期2", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_maPara3");
	strPara3 = para3->GetWindowTextW();
	if (strPara3 != L"")
		tmpPara[2] = _wtoi(strPara3.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入周期3", L"错误", MB_ICONWARNING);
		return;
	}

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_maPara4");
	strPara4 = para4->GetWindowTextW();
	if (strPara2 != L"")
		tmpPara[3] = _wtoi(strPara4.GetBuffer(1));
	else
	{
		SMessageBox(NULL, L"请输入周期4", L"错误", MB_ICONWARNING);
		return;
	}
	g_nDefaultMaPara[0] = tmpPara[0];
	g_nDefaultMaPara[1] = tmpPara[1];
	g_nDefaultMaPara[2] = tmpPara[2];
	g_nDefaultMaPara[3] = tmpPara[3];

	CIniFile ini(L".\\config\\config.ini");
	ini.WriteInt(L"DefaultPara", L"MaPara1", g_nDefaultMaPara[0]);
	ini.WriteInt(L"DefaultPara", L"MaPara2", g_nDefaultMaPara[1]);
	ini.WriteInt(L"DefaultPara", L"MaPara3", g_nDefaultMaPara[2]);
	ini.WriteInt(L"DefaultPara", L"MaPara4", g_nDefaultMaPara[3]);

	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);

}

void CDlgMaPara::SetEditText(const int * pPara)
{
	SEdit *para1 = FindChildByName2<SEdit>(L"edit_maPara1");
	para1->SetWindowTextW(std::to_wstring(pPara[0]).c_str());

	SEdit *para2 = FindChildByName2<SEdit>(L"edit_maPara2");
	para2->SetWindowTextW(std::to_wstring(pPara[1]).c_str());

	SEdit *para3 = FindChildByName2<SEdit>(L"edit_maPara3");
	para3->SetWindowTextW(std::to_wstring(pPara[2]).c_str());

	SEdit *para4 = FindChildByName2<SEdit>(L"edit_maPara4");
	para4->SetWindowTextW(std::to_wstring(pPara[3]).c_str());

}

void CDlgMaPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
