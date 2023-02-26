#include "stdafx.h"
#include "DlgFrmlFind.h"
#include "SCapsRichEdit.h"


CDlgFrmlFind::CDlgFrmlFind(HWND hParWnd) :SHostWnd(_T("LAYOUT:dlg_FrmlFind"))
{
	m_hParWnd = hParWnd;
}


CDlgFrmlFind::~CDlgFrmlFind()
{
}

BOOL SOUI::CDlgFrmlFind::OnInitDialog(EventArgs * e)
{
	m_pEditKeyWord = FindChildByID2<SCapsEdit>(R.id.edit_keyWord);
	return 0;
}


void SOUI::CDlgFrmlFind::OnBtnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgFrmlFind::OnBtnUpFind()
{
	SStringW strKeyWord = m_pEditKeyWord->GetWindowTextW();
	std::pair<int, SStringW> info = std::make_pair(SD_Up, strKeyWord);
	::SendMessage(m_hParWnd, WM_FILTER_MSG, (WPARAM)&info, FilterMsg_Search);
}

void SOUI::CDlgFrmlFind::OnBtnDownFind()
{
	SStringW strKeyWord = m_pEditKeyWord->GetWindowTextW();
	std::pair<int, SStringW> info = std::make_pair(SD_Down, strKeyWord);
	::SendMessage(m_hParWnd, WM_FILTER_MSG, (WPARAM)&info, FilterMsg_Search);
}

void SOUI::CDlgFrmlFind::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
