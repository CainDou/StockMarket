#include "stdafx.h"
#include "DlgKbElf.h"
#include<set>
#include"IniFile.h"

CDlgKbElf::CDlgKbElf()
{
}

CDlgKbElf::CDlgKbElf(HWND hParWnd) : SHostWnd(_T("LAYOUT:dlg_keyboardElf"))
{
	m_hParWnd = hParWnd;
	m_bFromSet = false;
}

CDlgKbElf::~CDlgKbElf()
{
}

void CDlgKbElf::OnClose()
{
	ShowWindow(SW_HIDE);

}

void CDlgKbElf::OnInit(EventArgs * e)
{
	m_pList = FindChildByName2<SColorListCtrlEx>(L"list_kbElf");
	m_pEdit = FindChildByName2<SEdit>(L"edit_kbElf");
	if (m_pEdit)
	{
		m_pEdit->GetEventSet()->subscribeEvent(
			EventRENotify::EventID,
			Subscriber(&CDlgKbElf::OnEditChange, this));
	}
	if (m_pList)
	{
		m_pList->GetEventSet()->subscribeEvent(
			EVT_LC_DBCLICK,
			Subscriber(&CDlgKbElf::OnDbClick, this));
	}

	m_pEdit->SetWindowTextW(L"");
	SetTimer(2, 500);

}


bool CDlgKbElf::SetStockInfo(vector<StockInfo>& stockVec)
{
	m_StockInfoMap.clear();
	for (auto &it : stockVec)
		m_StockInfoMap[it.SecurityID] = it;
	return true;
}

void CDlgKbElf::ClearInput()
{
	m_wstrInput = L"";
}

void CDlgKbElf::SetEditInput(SStringW wstrInput)
{
	m_pEdit->SetWindowTextW(wstrInput);
	m_pEdit->SetFocus();
	m_pEdit->SetSel(-1);
	if (wstrInput != L"")
	{
		m_bFromSet = true;
		m_wstrInput = wstrInput;
	}
}

bool CDlgKbElf::OnEditChange(EventArgs * e)
{
	if (m_pList == nullptr)
		return false;
	if (!IsWindowVisible())
		return false;
	SStringW wstr = m_pEdit->GetWindowTextW();

	if (m_bFromSet)
	{
		wstr = m_wstrInput;
		m_pEdit->SetWindowTextW(m_wstrInput);
		m_pEdit->SetSel(-1);
		m_bFromSet = false;
	}
	else if (m_wstrInput == wstr)
		return false;
	m_pList->DeleteAllItems();
	m_wstrInput = wstr;
	int InsCount = -1;
	m_Row2InsMap.clear();
	if (m_wstrInput == L"")
	{
		ShowWindow(SW_HIDE);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SetFocus, NULL);
	}
	else
	{
		SStringW wstr = InputToUpper(m_wstrInput);
		int len = wstr.GetLength();
		bool bID = true;
		bool bName = true;
		bool bSimpleName = true;
		SStringA strInput = StrW2StrA(wstr);
		for (int i = 0; i < len; ++i)
		{
			if (wstr[i] >= '0' && wstr[i] <= '9')
				bName = false;
			else if (wstr[i] >= 'A' && wstr[i] < 'Z')
				bID = false;
			else if (wstr[i] < 128)
			{
				bID = false;
				if (i > 2)
					bName = false;
			}
			else
			{
				bID = false;
				bSimpleName = false;
			}
		}


		for (auto &it : m_StockInfoMap)
		{
			if (bID)
			{
				if (it.first.Find(strInput) != -1)
				{
					AddFindItem(InsCount, it);
					continue;
				}
			}
			if (bSimpleName)
			{
				SStringA strSimpleName = it.second.SimpleName;
				if (strSimpleName.Find(strInput) != -1)
				{
					AddFindItem(InsCount, it);
					continue;
				}

			}
			if (bName)
			{
				SStringA strName = it.second.SecurityName;
				SStringA strNameNoSpace = "";
				if (strName.Find(' ') != -1)
				{
					SStringW wstrName = StrA2StrW(strName);
					wstrName.Remove(' ');
					strNameNoSpace = StrW2StrA(wstrName);
				}
				if (strName.Find(strInput) != -1
					|| strNameNoSpace.Find(strInput) != -1)
					AddFindItem(InsCount, it);

			}
		}


		if (InsCount == -1)
			m_pList->InsertItem(0, L"无匹配项");

		SetTimer(1, 50);

	}

	return true;
}

bool CDlgKbElf::OnDbClick(EventArgs * e)
{
	int nSel = m_pList->GetSelectedItem();
	if (nSel >= 0)
		SubscribeIns(m_Row2InsMap[nSel]);
	else if (m_Row2InsMap.find(0) != m_Row2InsMap.end())
		SubscribeIns(m_Row2InsMap[0]);
	ShowWindow(SW_HIDE);
	return true;
}

void CDlgKbElf::SubscribeIns(SStringA SubIns)
{

	m_subIns = SubIns;
	::PostMessageW(m_hParWnd, WM_WINDOW_MSG, WDMsg_SubIns, NULL);
}

void CDlgKbElf::OnTimer(UINT_PTR cTimerID)
{
	if (cTimerID == 1)
	{
		if (m_pList->GetItemCount() > 0)
			m_pList->SetSelectedItem(0);
		KillTimer(1);
	}

	if (cTimerID == 2)
	{
		if (IsWindowVisible())
		{
			if (!m_pList->IsFocused() && !m_pEdit->IsFocused())
			{

				ShowWindow(SW_HIDE);
				//::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SetFocus, NULL);
			}
		}
	}
}

SStringW CDlgKbElf::InputToUpper(SStringW inPut)
{
	int nLength = inPut.GetLength();
	SStringW Upperstr = L"";
	for (int i = 0; i < nLength; i++)
	{
		if (isalpha(inPut[i]))
			Upperstr += toupper(inPut[i]);
		else
			Upperstr += inPut[i];
	}
	return Upperstr;
}

SStringW CDlgKbElf::InputToLower(SStringW inPut)
{
	int nLength = inPut.GetLength();
	SStringW Upperstr = L"";
	for (int i = 0; i < nLength; i++)
	{
		if (isalpha(inPut[i]))
			Upperstr += tolower(inPut[i]);
		else
			Upperstr += inPut[i];
	}
	return Upperstr;
}

void CDlgKbElf::AddFindItem(int & InsCount,
	std::pair<const SStringA, StockInfo>& it)
{
	InsCount++;
	SStringW StockIns = StrA2StrW(it.first);
	if (StockIns.Find('I') != -1)
		StockIns = StockIns.Left(6);
	m_pList->InsertItem(InsCount, StockIns);
	m_pList->SetSubItemText(InsCount, 1,
		StrA2StrW(it.second.SecurityName));
	SStringW StockClass = L"A股";
	if ((it.first[0] == '0'&&it.first.GetLength() == 7)
		|| it.first[0] == '8')
		StockClass = L"指数";
	m_pList->SetSubItemText(InsCount, 2, StockClass);
	m_Row2InsMap[InsCount] = it.first;

}

void SOUI::CDlgKbElf::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}


void CDlgKbElf::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == VK_DOWN || nChar == VK_UP)
	{
		int nSelectedItem = m_pList->GetSelectedItem();
		if (nSelectedItem < 0)
		{
			if (nChar == VK_DOWN)
				nSelectedItem = m_pList->GetTopIndex();
			if (nChar == VK_UP)
			{
				if (m_pList->GetItemCount() < m_pList->GetCountPerPage(false))
					nSelectedItem = m_pList->GetItemCount();
				else
					nSelectedItem = m_pList->GetTopIndex() + m_pList->GetCountPerPage(false);

			}

		}
		int  nNewSelItem = -1;
		if (nChar == VK_DOWN && nSelectedItem < m_pList->GetItemCount() - 1)
			nNewSelItem = nSelectedItem + 1;
		else if (nChar == VK_UP &&  m_pList->GetSelectedItem() > 0)
			nNewSelItem = nSelectedItem - 1;


		if (nNewSelItem != -1)
		{
			m_pList->EnsureVisible(nNewSelItem);
			m_pList->SetSelectedItem(nNewSelItem);
		}

	}

}

void CDlgKbElf::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == VK_RETURN)
	{
		int nSel = m_pList->GetSelectedItem();
		if (nSel >= 0 && !m_Row2InsMap.empty())
			SubscribeIns(m_Row2InsMap[nSel]);
		m_pEdit->SetWindowTextW(L"");
		ShowWindow(SW_HIDE);
	}
	else if (nChar == VK_BACK && m_pEdit->GetWindowText() == L""
		|| nChar == VK_ESCAPE)
	{
		ShowWindow(SW_HIDE);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SetFocus, NULL);
	}
}
