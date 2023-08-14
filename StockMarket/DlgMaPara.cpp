#include "stdafx.h"
#include "DlgMaPara.h"
#include <string>
#include"IniFile.h"


CDlgMaPara::CDlgMaPara() :SHostWnd(_T("LAYOUT:dlg_MaPara"))
{
}

CDlgMaPara::CDlgMaPara(RpsGroup rg, HWND hWnd, eMaType maType) : SHostWnd(_T("LAYOUT:dlg_MaPara"))
{
	m_rgGroup = rg;
	m_hParWnd = hWnd;
	m_maType = maType;
}


CDlgMaPara::~CDlgMaPara()
{
}

void CDlgMaPara::InitConfigName()
{
	m_MaConfigName[eMa_Close] = "";
	m_MaConfigName[eMa_Volume] = "Vol";
	m_MaConfigName[eMa_Amount] = "Amo";
	m_MaConfigName[eMa_CAVol] = "CAVol";
	m_MaConfigName[eMa_CAAmo] = "CAAmo";

}

void CDlgMaPara::OnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void CDlgMaPara::OnClickButtonOk()
{
	for (int i = 0; i < MAX_MA_COUNT; ++i)
	{
		SStringW strName;
		SEdit *para = FindChildByName2<SEdit>(strName.Format(L"edit_maPara%d", i + 1));
		strPara[i] = para->GetWindowTextW();
		if (strPara[i] != L"")
			nPara[i] = _wtoi(strPara[i].GetBuffer(1));
		else
			nPara[i] = 0;
	}
	::SendMessageW(m_hParWnd, WM_KLINE_MSG, (WPARAM)nPara, KLINEMSG_MA);
	OnClose();

}

void CDlgMaPara::OnClickButtonCancel()
{
	OnClose();
}

void CDlgMaPara::OnInit(EventArgs * e)
{
	InitConfigName();
	CIniFile ini(".//config//config.ini");
	SStringA strSection = "DefaultPara";
	SStringA strKey;
	int tmpDefaultPara[MAX_MA_COUNT] = { 5,10,20,60,0,0 };
	if (m_maType != eMa_Close)
	{
		tmpDefaultPara[2] = 0;
		tmpDefaultPara[3] = 0;
		strSection += m_MaConfigName[m_maType];
	}

	for (int i = 0; i < MAX_MA_COUNT; ++i)
		m_nDefaultPara[0] = ini.GetIntA(strSection, strKey.Format("MAPara%d", i + 1), tmpDefaultPara[i]);

}

void CDlgMaPara::OnClickButtonDefault()
{
	for (int i = 0; i < MAX_MA_COUNT; ++i)
	{
		SStringW strName;
		SEdit *para = FindChildByName2<SEdit>(strName.Format(L"edit_maPara%d", i + 1));
		para->SetWindowTextW(std::to_wstring(m_nDefaultPara[i]).c_str());;
	}

}

void CDlgMaPara::OnClickButtonSetDefault()
{
	for (int i = 0; i < MAX_MA_COUNT; ++i)
	{
		SStringW strName;
		SEdit *para = FindChildByName2<SEdit>(strName.Format(L"edit_maPara%d", i + 1));
		strPara[i] = para->GetWindowTextW();
		if (strPara[i] != L"")
			m_nDefaultPara[i] = _wtoi(strPara[i].GetBuffer(1));
		else
			m_nDefaultPara[i] = 0;

	}

	CIniFile ini(L".\\config\\config.ini");
	SStringW strSection = L"DefaultPara";
	SStringW strKey;
	if (m_maType == eMa_Volume)
		strSection += L"Vol";
	else if (m_maType == eMa_Amount)
		strSection += L"Amo";
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		m_nDefaultPara[0] = ini.WriteInt(strSection, strKey.Format(L"MAPara%d", i + 1), m_nDefaultPara[i]);

	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);

}

void CDlgMaPara::SetEditText(const int * pPara)
{
	for (int i = 0; i < MAX_MA_COUNT; ++i)
	{
		SStringW strName;
		SEdit *para = FindChildByName2<SEdit>(strName.Format(L"edit_maPara%d", i + 1));
		para->SetWindowTextW(std::to_wstring(pPara[i]).c_str());
	}

}

void CDlgMaPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
