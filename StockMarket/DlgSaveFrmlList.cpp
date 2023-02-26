#include "stdafx.h"
#include "DlgSaveFrmlList.h"
#include <fstream>
#include <set>


CDlgSaveFrmlList::CDlgSaveFrmlList(HWND hParWnd):SHostWnd(_T("LAYOUT:dlg_SaveFrmlList"))
{
	m_hParWnd = hParWnd;
}


CDlgSaveFrmlList::~CDlgSaveFrmlList()
{
}

BOOL SOUI::CDlgSaveFrmlList::OnInitDialog(EventArgs * e)
{
	m_pEditFileName = FindChildByID2<SEdit>(R.id.edit_fileName);
	return 0;
}

void SOUI::CDlgSaveFrmlList::OnBtnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	::SwitchToThisWindow(m_hParWnd,TRUE);
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgSaveFrmlList::OnBtnOK()
{
	string strFileName =StrW2StrA(m_pEditFileName->GetWindowTextW());
	if (strFileName.empty())
	{
		SMessageBox(m_hWnd, L"方案名称不能为空", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}
	std::ifstream ifile(".\\config\\FilterFrml.lst",std::ios::_Nocreate);
	std::set<string> listName;
	bool bReWrite = false;
	if (ifile.is_open())
	{
		string buffer;
		while (getline(ifile, buffer))
			listName.insert(buffer);
		if (listName.count(strFileName) != 0)
		{
			bReWrite = true;
			if (IDCANCEL == SMessageBox(m_hWnd,
				L"当前输入的方案名已经存在，是否覆盖？",
				L"警告", MB_OKCANCEL | MB_ICONWARNING))
				return;
		}
		ifile.close();
	}
	::SendMessage(m_hParWnd, WM_FILTER_MSG,
		(WPARAM)&strFileName, FilterMsg_SaveFrmlList);
	if (!bReWrite)
	{
		std::ofstream ofile(".\\config\\FilterFrml.lst", std::ios::app);
		if (ofile.is_open())
		{
			ofile << strFileName << std:: endl;
			ofile.close();
		}
	}
	OnBtnClose();

}

void SOUI::CDlgSaveFrmlList::OnBtnCancel()
{
	OnBtnClose();
}

void SOUI::CDlgSaveFrmlList::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
