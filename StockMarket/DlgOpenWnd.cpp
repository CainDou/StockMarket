#include "stdafx.h"
#include "DlgOpenWnd.h"
#include "IniFile.h"

CDlgOpenWnd::CDlgOpenWnd(HWND hParWnd) :SHostWnd(_T("LAYOUT:dlg_openWindow"))
{
	m_hParWnd = hParWnd;
}


CDlgOpenWnd::~CDlgOpenWnd()
{
}

void SOUI::CDlgOpenWnd::OnInit(EventArgs * e)
{
	pCbx = FindChildByName2<SComboBox>(L"cbx_open");
	CIniFile ini(".//config//subWindow.ini");
	nDefaultWndCount = ini.GetIntA("SubWindowName", "DefaultCount", 0);
	nNameCount = ini.GetIntA("SubWindowName", "NameCount", 0);
	SStringA str;
	SStringA strName;
	int nItemCount = 0;
	for (int i = nNameCount - 1; i >= 0; --i)
	{
		strName = ini.GetStringA("SubWindowName",
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

void SOUI::CDlgOpenWnd::OnBtnOK()
{
	int nSel = pCbx->GetCurSel();
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_OpenWindow, ItemdNumMap[nSel]);
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
