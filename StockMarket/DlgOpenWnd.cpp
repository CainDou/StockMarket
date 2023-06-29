#include "stdafx.h"
#include "DlgOpenWnd.h"
#include "IniFile.h"

CDlgOpenWnd::CDlgOpenWnd(HWND hParWnd,int nWndType) :SHostWnd(_T("LAYOUT:dlg_openWindow"))
{
	m_hParWnd = hParWnd;
	m_nWndType = nWndType;
}


CDlgOpenWnd::~CDlgOpenWnd()
{
}

void SOUI::CDlgOpenWnd::OnInit(EventArgs * e)
{
	InitStrings();
	m_strSection = m_WndTypeSection[m_nWndType];
	pCbx = FindChildByName2<SComboBox>(L"cbx_open");
	CIniFile ini(".//config//subWindow.ini");
	nNameCount = ini.GetIntA(m_strSection, "NameCount", 0);
	SStringA str;
	SStringA strName;
	int nItemCount = 0;
	for (int i = nNameCount - 1; i >= 0; --i)
	{
		strName = ini.GetStringA(m_strSection,
			str.Format("%d", i), "");
		if (NameSet.count(strName) == 0)
		{
			NameSet.insert(strName);
			ItemdNumMap[nItemCount] = i;
			pCbx->InsertItem(nItemCount, StrA2StrW(strName), 0, 0);
			++nItemCount;
		}
	}
	pCbx->SetCurSel(0);
}

void CDlgOpenWnd::InitStrings()
{
	m_WndTypeSection[WT_SubWindow] = "SubWindowName";
	m_WndTypeSection[WT_FilterWindow] = "FilterWindowName";
}

void SOUI::CDlgOpenWnd::OnBtnOK()
{
	int nSel = pCbx->GetCurSel();
	LPARAM lp = MAKELPARAM(ItemdNumMap[nSel], m_nWndType);
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_OpenWindow, lp);
	CSimpleWnd::DestroyWindow();

}

void SOUI::CDlgOpenWnd::OnBtnCancel()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgOpenWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
