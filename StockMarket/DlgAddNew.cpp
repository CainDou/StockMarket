#include "stdafx.h"
#include "DlgAddNew.h"
#include "IniFile.h"


CDlgAddNew::CDlgAddNew(HWND hParWnd, int nWndType, SStringA strOldName) :SHostWnd(_T("LAYOUT:dlg_newWindow"))
{
	m_hParWnd = hParWnd;
	m_nWndType = nWndType;
	m_strOldName = strOldName;

}


CDlgAddNew::~CDlgAddNew()
{
}

void SOUI::CDlgAddNew::OnInit(EventArgs * e)
{
	if (m_nWndType == WT_SubWindow || m_nWndType == WT_FilterWindow)
	{
		InitStrings();
		CIniFile ini(".//config//subWindow.ini");
		m_strSection = m_WndTypeSection[m_nWndType];
		m_strDef = m_WndTypeDefName[m_nWndType];
		nDefaultWndCount = ini.GetIntA(m_strSection, "DefaultCount", 0);
		nNameCount = ini.GetIntA(m_strSection, "NameCount", 0);
		SStringA str;
		SStringA strName;
		for (int i = 0; i < nNameCount; ++i)
		{
			strName = ini.GetStringA(m_strSection,
				str.Format("%d", i), "");
			if (strName != "")
				UsedNameSet.insert(strName);
		}
		m_strDefaultName.Format(L"%s%d", m_strDef, nDefaultWndCount + 1);
	}
	else if (m_nWndType == WT_SetFliterName)
	{
		m_strDefaultName = StrA2StrW(m_strOldName);
		SStatic *pTitle = FindChildByName2<SStatic>(L"txt_title");
		if (pTitle)
			pTitle->SetWindowTextW(L"设置选股方案名称");
	}
	m_pEdit = FindChildByName2<SEdit>(L"edit_name");
	m_pEdit->SetWindowTextW(m_strDefaultName);
	m_pEdit->SetFocus();
	//m_pEdit->SetSel(0,1);

}

void SOUI::CDlgAddNew::InitStrings()
{
	m_WndTypeSection[WT_SubWindow] = "SubWindowName";
	m_WndTypeSection[WT_FilterWindow] = "FilterWindowName";
	m_WndTypeDefName[WT_SubWindow] = L"副窗口";
	m_WndTypeDefName[WT_FilterWindow] = L"选股窗口";

}

void CDlgAddNew::OnBtnOK()
{
	SStringW strEdit = m_pEdit->GetWindowTextW();
	if (m_nWndType == WT_SubWindow || m_nWndType == WT_FilterWindow)
	{
		if (strEdit == L"")
		{
			SMessageBox(m_hWnd, L"窗口名称不能为空", L"错误", MB_ICONERROR | MB_OK);
			return;
		}
		m_strNewWndName = StrW2StrA(strEdit);
		if (UsedNameSet.count(m_strNewWndName))
		{
			if (IDCANCEL == SMessageBox(m_hWnd, L"已经具有相同名称的子窗口，"
				"继续操作可能会覆盖原有配置，是否继续", L"警告",
				MB_ICONWARNING | MB_OKCANCEL))
				return;
		}
		CIniFile ini(".//config//subWindow.ini");

		if (strEdit == m_strDefaultName)
		{
			++nDefaultWndCount;
			ini.WriteIntA(m_strSection, "DefaultCount", nDefaultWndCount);
		}
		++nNameCount;
		ini.WriteIntA(m_strSection, "NameCount", nNameCount);
		SStringA str;
		ini.WriteStringA(m_strSection,
			str.Format("%d", nNameCount - 1), m_strNewWndName);
		LPARAM lp = MAKELPARAM(nNameCount - 1, m_nWndType);
		::PostMessageW(m_hParWnd, WM_WINDOW_MSG, WDMsg_NewWindow, lp);
	}
	else if(m_nWndType == WT_SetFliterName)
	{
		SStringA strFilterName = StrW2StrA(strEdit);
		::SendMessageW(m_hParWnd, WM_WINDOW_MSG, WDMsg_SetFilterName, (LPARAM)&strFilterName);
		CSimpleWnd::DestroyWindow();
	}

	
}

void SOUI::CDlgAddNew::OnBtnCancel()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgAddNew::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

