#include "stdafx.h"
#include "DlgSelTarget.h"


CDlgSelTarget::CDlgSelTarget(HWND hWnd, map<int, ShowPointInfo>& pointMap, int nPeriod) 
	:SHostWnd(_T("LAYOUT:dlg_targetSelect"))
{
	m_hParWnd = hWnd;
	m_bFindOver = TRUE;
	m_preFindStr = "";
	m_PointInfoMap = pointMap;
	m_nPeriod = nPeriod;
}


CDlgSelTarget::~CDlgSelTarget()
{
}

BOOL CDlgSelTarget::OnInitDialog(EventArgs * e)
{
	m_pLbTarget = FindChildByName2<SListBox>(L"lb_target");
	m_pEditFinder = FindChildByName2<SEdit>(L"edit_FindTarget");
	int nIndex = 0;
	for (auto &it : m_PointInfoMap)
	{
		if (m_nPeriod != Period_1Day 
			&& it.first >= eCAPointStart
			&&it.first < eCAPointEnd)
			continue;
		m_itemPointData[nIndex] = it.first;
		m_pLbTarget->InsertString(nIndex, StrA2StrW(it.second.showName));
		++nIndex;
	}
	m_pLbTarget->SetCurSel(0);
	return 0;
}

void SOUI::CDlgSelTarget::OnBtnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgSelTarget::OnBtnOK()
{
	int nSel = m_pLbTarget->GetCurSel();
	::SendMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_ChangePointTarget, (LPARAM)m_itemPointData[nSel]);
	OnBtnClose();
}

void SOUI::CDlgSelTarget::OnBtnFind()
{
	SStringA str = StrW2StrA(m_pEditFinder->GetWindowTextW());
	if (str != m_preFindStr)
	{
		m_preFindStr = str;
		m_bFindOver = TRUE;
	}
	if (str.IsEmpty())
		return;
	if (m_bFindOver)
	{
		m_bFindOver = FALSE;
		m_FinderMap.clear();
		int nIndex = 0;
		for (auto &it : m_PointInfoMap)
		{
			if (it.second.showName.Find(str) >= 0)
				m_FinderMap[it.first] = nIndex;
			++nIndex;
		}
		if (m_FinderMap.empty())
		{
			SMessageBox(NULL, L"没有找到匹配项", L"通知", MB_OK);
			m_bFindOver = TRUE;
		}
		else
		{
			auto & topRes = m_FinderMap.begin();
			m_pLbTarget->SetCurSel(topRes->second);
			m_FinderMap.erase(topRes->first);
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
			m_pLbTarget->SetCurSel(topRes->second);
			m_FinderMap.erase(topRes->first);
		}

	}
}

void SOUI::CDlgSelTarget::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
