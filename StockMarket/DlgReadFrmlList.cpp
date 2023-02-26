#include "stdafx.h"
#include "DlgReadFrmlList.h"
#include "SColorListCtrlEx.h"
#include <fstream>


CDlgReadFrmlList::CDlgReadFrmlList(HWND hParWnd) :SHostWnd(_T("LAYOUT:dlg_ReadFrmlList"))
{
	m_hParWnd = hParWnd;
	m_bDelete = FALSE;
}


CDlgReadFrmlList::~CDlgReadFrmlList()
{
}

BOOL SOUI::CDlgReadFrmlList::OnInitDialog(EventArgs * e)
{
	m_pListPlanName = FindChildByID2<SColorListCtrlEx>(R.id.lc_planName);
	std::ifstream ifile(".\\config\\FilterFrml.lst", std::ios::_Nocreate);
	if (ifile.is_open())
	{
		string buffer;
		while (getline(ifile, buffer))
		{
			m_pListPlanName->InsertItem(m_palnNameVec.size(), StrA2StrW(buffer.c_str()));
			m_palnNameVec.emplace_back(buffer);
		}
		ifile.close();
	}
	if (m_pListPlanName->GetItemCount() != 0)
		m_pListPlanName->SetSelectedItem(0);
	return 0;
}

void SOUI::CDlgReadFrmlList::OnBtnClose()
{
	if (m_bDelete)
	{
		std::ofstream ofile(".\\config\\FilterFrml.lst");
		if (ofile.is_open())
		{
			for(auto &it:m_palnNameVec)
				ofile << it<< std::endl;
			ofile.close();
		}
	}
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgReadFrmlList::OnBtnOK()
{
	int nSel = m_pListPlanName->GetSelectedItem();
	if (nSel < 0)
		return;
	string strFileName = m_palnNameVec[nSel];
	::SendMessage(m_hParWnd, WM_FILTER_MSG,
		(WPARAM)&strFileName, FilterMsg_ReadFrmlList);
	OnBtnClose();
}

void SOUI::CDlgReadFrmlList::OnBtnCancel()
{
	OnBtnClose();
}

void SOUI::CDlgReadFrmlList::OnBtnDelete()
{
	int nSel = m_pListPlanName->GetSelectedItem();
	if (nSel < 0)
		return;
	SStringW str;
	str.Format(L"确定要删除方案:%s吗?", m_pListPlanName->GetSubItemText(nSel, 0));
	if (SMessageBox(m_hWnd, str, L"提示", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
	{
		string planName = ".\\filter\\";
		planName += m_palnNameVec[nSel];
		planName += ".sfl";
		remove(planName.c_str());
		for (int i = nSel; i < m_palnNameVec.size() - 1; ++i)
			m_palnNameVec[i] = m_palnNameVec[i + 1];
		m_palnNameVec.pop_back();
		m_pListPlanName->DeleteItem(nSel);
		m_bDelete = TRUE;
		m_pListPlanName->RequestRelayout();
	}
}

void SOUI::CDlgReadFrmlList::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
