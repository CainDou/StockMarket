#include "stdafx.h"
#include "DlgFuncInsert.h"
#include "FrmlCompiler.h"
#include "SColorListCtrlEx.h"
#include "SCapsRichEdit.h"

CDlgFuncInsert::CDlgFuncInsert(HWND hWnd) :SHostWnd(_T("LAYOUT:dlg_FuncInsert"))
{
	m_hParWnd = hWnd;
	m_preFuncClass = FT_Count;
	m_bFindOver = TRUE;
	m_preFindStr = "";
}


CDlgFuncInsert::~CDlgFuncInsert()
{
}

BOOL SOUI::CDlgFuncInsert::OnInitDialog(EventArgs * e)
{
	m_FuncInfoMap = CFrmlCompiler::GetFuncInfo();
	m_pLcFunc = FindChildByName2<SColorListCtrlEx>(L"lc_func");
	m_pLcFunc->Invalidate();
	m_pLbClass = FindChildByName2<SListBox>(L"lb_class");
	m_pEditFinder = FindChildByName2<SCapsEdit>(L"edit_FindFunc");
	m_pEditDscp = FindChildByName2<SEdit>(L"edit_funcDscp");
	m_pLbClass->GetEventSet()->subscribeEvent(EventLBSelChanged::EventID,
		Subscriber(&CDlgFuncInsert::OnLbClassClicked, this));
	m_pLcFunc->GetEventSet()->subscribeEvent(EventLCSelChanged::EventID,
		Subscriber(&CDlgFuncInsert::OnLcFuncClicked, this));
	m_pLbClass->SetCurSel(0);
	

	return true;
}

void SOUI::CDlgFuncInsert::OnBtnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	ShowWindow(SW_HIDE);
}

void SOUI::CDlgFuncInsert::OnBtnOK()
{
	int nSel = m_pLcFunc->GetSelectedItem();
	if (-1 == nSel)
		return;
	SStringW strFunc = m_pLcFunc->GetSubItemText(nSel, 0);
	::EnableWindow(m_hParWnd, TRUE);
	::SendMessage(m_hParWnd, WM_FUNC_MSG, (WPARAM)&strFunc, FNCMsg_InsertFunc);
	ShowWindow(SW_HIDE);
}

void SOUI::CDlgFuncInsert::OnBtnFind()
{
	string str = StrW2StrA(m_pEditFinder->GetWindowTextW());
	if (str != m_preFindStr)
	{
		m_preFindStr = str;
		m_bFindOver = TRUE;
	}
	if (str.empty())
		return;
	if (m_bFindOver)
	{
		m_bFindOver = FALSE;
		m_FinderMap.clear();
		for (auto &it : m_FuncInfoMap)
		{
			for (int i = 0; i < it.second.size(); ++i)
			{
				if (string::npos != it.second[i].name.find(str))
					m_FinderMap[it.first].insert(i);
			}
		}
		if (m_FinderMap.empty())
		{
			SMessageBox(NULL, L"没有找到匹配项", L"通知", MB_OK);
			m_bFindOver = TRUE;
		}
		else
		{
			auto & topRes = m_FinderMap.begin();
			eFuncType funcClass = topRes->first;
			int nPos = *topRes->second.begin();
			UpdateList(funcClass);
			m_pLcFunc->SetSelectedItem(nPos);
			topRes->second.erase(nPos);
			if (topRes->second.empty())
				m_FinderMap.erase(funcClass);
		}
	}
	else
	{
		if (m_FinderMap.empty())
		{
			SMessageBox(NULL, L"查找完毕!", L"通知", MB_OK);
			m_bFindOver = TRUE;
		}
		else
		{
			auto & topRes = m_FinderMap.begin();
			eFuncType funcClass = topRes->first;
			int nPos = *topRes->second.begin();
			UpdateList(funcClass);
			m_pLcFunc->SetSelectedItem(nPos);
			topRes->second.erase(nPos);
			if (topRes->second.empty())
				m_FinderMap.erase(funcClass);
		}

	}
}

bool SOUI::CDlgFuncInsert::OnLbClassClicked(EventArgs * pEvtBase)
{
	EventLBSelChanged* pEvt = (EventLBSelChanged*)pEvtBase;
	SListBox* pList = (SListBox*)pEvt->sender;
	int nSel = pList->GetCurSel();
	UpdateList((eFuncType)nSel);
	return true;
}

bool SOUI::CDlgFuncInsert::OnLcFuncClicked(EventArgs * pEvtBase)
{
	EventLCSelChanged * pEvt = (EventLCSelChanged *)pEvtBase;
	SColorListCtrlEx* pList = (SColorListCtrlEx*)pEvt->sender;
	int nCurSel = pList->GetSelectedItem();
	if (m_preFuncClass == FT_None)
	{
		int nNowCount = 0;
		for (auto &it : m_FuncInfoMap)
		{
			if (it.second.size() + nNowCount <= nCurSel)
			{
				nNowCount += it.second.size();
				continue;
			}
			else
			{
				m_pEditDscp->SetWindowTextW(
					StrA2StrW(it.second[nCurSel - nNowCount].\
						description.c_str()));
				break;
			}
		}
	}
	else
		m_pEditDscp->SetWindowTextW(
			StrA2StrW(m_FuncInfoMap[m_preFuncClass][nCurSel].\
				description.c_str()));
	return true;
}

void SOUI::CDlgFuncInsert::UpdateList(eFuncType FtClass)
{
	if (FtClass == m_preFuncClass)
		return;
	m_pLcFunc->DeleteAllItems();
	//m_pLbClass->SetCurSel(FtClass);
	int nRow = 0;
	if (FT_None == FtClass)
	{
		for (auto &classVec : m_FuncInfoMap)
		{
			for (auto &funcInfo : classVec.second)
			{
				m_pLcFunc->InsertItem(nRow, StrA2StrW(funcInfo.name.c_str()));
				m_pLcFunc->SetSubItemText(nRow++, 1, StrA2StrW(funcInfo.frofile.c_str()));
			}
		}
	}
	else
	{
		auto &classVec = m_FuncInfoMap[FtClass];
		for (auto &funcInfo : classVec)
		{
			m_pLcFunc->InsertItem(nRow, StrA2StrW(funcInfo.name.c_str()));
			m_pLcFunc->SetSubItemText(nRow++, 1, StrA2StrW(funcInfo.frofile.c_str()));
		}

	}
	m_pLcFunc->RequestRelayout();

	m_preFuncClass = FtClass;
}

void SOUI::CDlgFuncInsert::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
