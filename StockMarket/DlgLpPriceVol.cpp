#include "stdafx.h"
#include "DlgLpPriceVol.h"
#include "IniFile.h"
#include <fstream>
#include "WndSynHandler.h"
#include "DlgStockFilter.h"
#include <io.h>
#include <direct.h>



#define TIMER_AUTOSAVE 1

extern CWndSynHandler g_WndSyn;
extern HWND g_MainWnd;


CDlgLpPriceVol::CDlgLpPriceVol(vector<StockInfo>& stockInfoVec) :SHostWnd(_T("LAYOUT:dlg_lpPriceVol"))
{
	m_bLayoutInited = FALSE;
	m_bIsValid = TRUE;
	m_LpDataVec.resize(6);
	m_LpShowData.resize(6);
	m_StockInfoVec = stockInfoVec;
	m_nShowPicNum = 0;
	m_nMinPrice = INT_MAX;
	m_nMaxPrice = INT_MIN;
	ZeroMemory(m_nPicPeriod, sizeof(m_nPicPeriod));
	ZeroMemory(m_nPicDate, sizeof(m_nPicPeriod));
	m_nMinLevel = 0;
	m_fMaxVol = 0;
}


CDlgLpPriceVol::~CDlgLpPriceVol()
{
	SendMsg(m_uMsgThreadID, Msg_Exit, NULL, 0);
	if (m_thread.joinable())
		m_thread.join();

}

void CDlgLpPriceVol::OnClose()
{
	m_bIsValid = FALSE;
	ShowWindow(SW_HIDE);
	SStringA strPosFile;
	strPosFile.Format(".\\config\\LpPriceVol.position");
	if (_access(strPosFile, 0) == 0)
		remove(strPosFile);
}

int CDlgLpPriceVol::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CDlgLpPriceVol::OnInitDialog(EventArgs* e)
{
	m_bLayoutInited = TRUE;
	SStatic *pTitl = FindChildByID2<SStatic>(R.id.text_windowName);
	m_SynThreadID = g_WndSyn.GetThreadID();
	//InitWindowPos();
	InitControls();
	InitConfig();
	InitDataHandleMap();
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	m_pShowRpsData = &(pListData->at(Group_Stock)[Period_1Day]);
	m_thread = thread(&CDlgLpPriceVol::DataHandle, this);
	m_uMsgThreadID = *(unsigned*)&m_thread.get_id();
	g_WndSyn.SetLpPriceVolWnd(m_hWnd, m_uMsgThreadID);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CDlgLpPriceVol::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case LPDMsg_UpdatePic:
		m_pLpPriceVolPic[(int)lp]->Invalidate();
		break;
	case LPDMsg_ChangeDate:
	{
		Sleep(10);
		int nPic = (int)lp;
		SCalendarEx* pCal = (SCalendarEx*)m_pDtpDate[nPic]->GetChild(GSW_FIRSTCHILD);
		OutputDebugStringFormat("OnMsg中ca是否显示：%d", pCal->IsVisible());
	}
	break;
	default:
		break;
	}
	return 0;
}

LRESULT SOUI::CDlgLpPriceVol::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
	__super::OnNcCalcSize(bCalcValidRects, lParam);
	//SWindow::GetClientRect(&m_rcClient);
	OutputDebugStringFormat("bCalc:%d 重新计算size\n", bCalcValidRects);

	return 0;
}

void CDlgLpPriceVol::OnDestroy()
{
	SetMsgHandled(FALSE);

	if (m_bIsValid)
	{
		SStringA strPosFile;
		strPosFile.Format(".\\config\\LpPriceVol.position");
		std::ofstream ofile(strPosFile);
		if (ofile.is_open())
		{
			WINDOWPLACEMENT wp = { sizeof(wp) };
			::GetWindowPlacement(m_hWnd, &wp);

			ofile.write((char*)&wp, sizeof(wp));
			ofile.close();
		}
		ShowWindow(SW_HIDE);
	}
}

void CDlgLpPriceVol::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == TIMER_AUTOSAVE)
		SaveConfig();
}


bool CDlgLpPriceVol::OnEditStockIDChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	if (pEvt->iNotify == EN_KILLFOCUS)
	{
		if (m_pLbStockID->IsVisible())
			m_pLbStockID->SetVisible(FALSE, TRUE);
		return true;
	}
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	SStringW str = pEdit->GetWindowTextW();
	SStringA strID = StrW2StrA(str);
	if (str.IsEmpty())
	{
		m_pLbStockID->SetVisible(FALSE, TRUE);
		return true;
	}
	m_pLbStockID->DeleteAll();
	CRect rc = m_pLbStockID->GetClientRect();
	int nItemHeight = m_pLbStockID->GetItemHeight();
	int nMaxItem = rc.Height() / nItemHeight;
	int nCount = 0;
	for (auto& it : m_StockInfoVec)
	{
		if (strstr(it.SecurityID, strID.GetBuffer(0)))
		{
			SStringW strShow;
			strShow.Format(L"%s  %s", StrA2StrW(it.SecurityID),
				StrA2StrW(it.SecurityName));
			m_pLbStockID->InsertString(nCount++, strShow);
			if (nCount == nMaxItem)
				break;
		}
	}
	if (nCount == 0)
		m_pLbStockID->SetVisible(FALSE, TRUE);
	else
	{
		m_pLbStockID->SetCurSel(0);
		m_bSetFirst = TRUE;
		m_pLbStockID->SetVisible(TRUE, TRUE);
	}
	return TRUE;
}

void CDlgLpPriceVol::OnMouseMove(UINT nFlags, CPoint pt)
{
	SetMsgHandled(FALSE);
	if (m_bSetFirst)
	{
		m_bSetFirst = FALSE;
		m_ptMouse = pt;
	}

	if (pt != m_ptMouse)
	{
		if (m_pLbStockID->IsVisible())
			m_pLbStockID->SetCurSel(m_pLbStockID->HitTest(pt));
	}
}

void CDlgLpPriceVol::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (m_pLbStockID->IsVisible())
		m_pLbStockID->SSendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));

	if (nChar == VK_RETURN)
	{
		if (m_pEditStockID->IsFocused())
		{
			if (m_pLbStockID->IsVisible())
			{
				int nSel = m_pLbStockID->GetCurSel();
				SStringW str = m_pLbStockID->GetText(nSel);
				SStringA strStockID = StrW2StrA(str).Left(6);
				m_pEditStockID->SetWindowTextW(str.Left(6));
				m_pLbStockID->SetVisible(FALSE, TRUE);
				m_pEditStockID->KillFocus();
				if (strStockID != m_StockID)
				{
					int nTick = GetTickCount();
					m_StockID = strStockID;
					m_StockName = StrW2StrA(str.Right(str.GetLength() - 1 - 6));
					m_nMinPrice = INT_MAX;
					m_nMaxPrice = INT_MIN;
					m_fMaxVol = 0;

					for (int i = 0; i < MAX_PICNUM; ++i)
					{
						m_pLpPriceVolPic[i]->ChangeShowStock(m_StockID, m_StockName);
						m_LpDataVec[i].clear();
						m_pLpPriceVolPic[i]->ClearShowData();
						m_pLpPriceVolPic[i]->Invalidate();
					}

					set<pair<int, int>> DataGetSet;
					for (int i = 0; i < m_nShowPicNum; ++i)
						DataGetSet.insert(make_pair(m_nPicPeriod[i], m_nPicDate[i]));

					for (auto &it : DataGetSet)
						GetLpPriceData(it.first, it.second);
					nTick = GetTickCount() - nTick;
					OutputDebugStringFormat("处理耗时:%dms\n", nTick);
				}
			}
		}
		else if (m_pEditBasePrice->IsFocused())
			m_pEditBasePrice->KillFocus();
		else
		{
			for (int i = 0; i < MAX_PICNUM; ++i)
			{
				if (m_pDtpDate[i]->IsFocused())
				{
					m_pDtpDate[i]->KillFocus();
					SYSTEMTIME st;
					m_pDtpDate[i]->GetTime(st);
					SetPicDataDate(st, i);
				}
			}

		}
	}

}

bool CDlgLpPriceVol::OnLbStockIDLButtonDown(EventArgs * e)
{
	EventLButtonDown* pEvt = (EventLButtonDown*)e;
	SListBox* pList = (SListBox*)pEvt->sender;
	int nSel = pList->GetCurSel();
	SStringW str = pList->GetText(nSel);
	SStringA strStockID = StrW2StrA(str).Left(6);
	m_pEditStockID->SetWindowTextW(str.Left(6));
	pList->SetVisible(FALSE, TRUE);
	m_pEditStockID->KillFocus();
	if (strStockID != m_StockID)
	{
		m_StockID = strStockID;
		m_StockName = StrW2StrA(str).Right(str.GetLength() - 1 - 6);
		m_nMinPrice = INT_MAX;
		m_nMaxPrice = INT_MIN;
		m_fMaxVol = 0;
		for (int i = 0; i < MAX_PICNUM; ++i)
		{
			m_pLpPriceVolPic[i]->ChangeShowStock(m_StockID, m_StockName);
			m_LpDataVec[i].clear();
			m_pLpPriceVolPic[i]->ClearShowData();
			m_pLpPriceVolPic[i]->Invalidate();
		}

		set<pair<int, int>> DataGetSet;
		for (int i = 0; i < m_nShowPicNum; ++i)
			DataGetSet.insert(make_pair(m_nPicPeriod[i], m_nPicDate[i]));

		for (auto &it : DataGetSet)
			GetLpPriceData(it.first, it.second);
	}
	return true;
}

bool CDlgLpPriceVol::OnCbxPicNumChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	SetShowPicNum(pCbx->GetItemData(nSel));
	pCbx->KillFocus();
	return true;
}

bool CDlgLpPriceVol::OnCbxDataTypeChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nDataType = pCbx->GetItemData(nSel);
	if (nDataType == m_nDataType)
		return true;
	m_nDataType = nDataType;
	m_fMaxVol = 0;

	int nReCalcType = eLPCW_ShowWnd;
	for (int i = 0; i < m_nShowPicNum; ++i)
	{
		m_pLpPriceVolPic[i]->ClearShowData();
		m_pLpPriceVolPic[i]->Invalidate();
	}
	SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));
	pCbx->KillFocus();
	return true;
}

bool CDlgLpPriceVol::OnCbxDataLevelChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nDataLevel = pCbx->GetItemData(nSel);
	pCbx->KillFocus();
	if (nDataLevel == m_nDataLevel)
		return true;
	m_nDataLevel = nDataLevel;
	m_fMaxVol = 0;
	int nReCalcType = eLPCW_ShowWnd;
	for (int i = 0; i < m_nShowPicNum; ++i)
	{
		m_pLpPriceVolPic[i]->ClearShowData();
		m_pLpPriceVolPic[i]->Invalidate();
	}
	SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));
	return true;
}

bool CDlgLpPriceVol::OnCbxBasePriceChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int  nBasePrice = pCbx->GetItemData(nSel);
	pCbx->KillFocus();
	if (m_nBasePriceType == nSel && nBasePrice != 9999)
		return true;
	m_nBasePriceType = nSel;
	double fBasePrice = nBasePrice;
	if (nBasePrice == 9999)
	{
		m_pEditBasePrice->SetVisible(TRUE, TRUE);
		fBasePrice = _wtof(m_pEditBasePrice->GetWindowTextW());
	}
	else
		m_pEditBasePrice->SetVisible(FALSE, TRUE);

	if (fBasePrice == m_fBasePrice)
		return true;
	m_fBasePrice = fBasePrice;
	m_fMaxVol = 0;
	int nReCalcType = eLPCW_ShowWnd;
	for (int i = 0; i < m_nShowPicNum; ++i)
	{
		m_pLpPriceVolPic[i]->ClearShowData();
		m_pLpPriceVolPic[i]->Invalidate();
	}
	SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));

	return true;
}

bool CDlgLpPriceVol::OnEditBasePriceChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	if (pEvt->iNotify == EN_KILLFOCUS)
	{
		double fBasePrice = _wtof(pEdit->GetWindowTextW());
		if (fBasePrice != m_fBasePrice)
		{
			m_fBasePrice = fBasePrice;
			m_fMaxVol = 0;
			int nReCalcType = eLPCW_ShowWnd;
			for (int i = 0; i < m_nShowPicNum; ++i)
			{
				m_pLpPriceVolPic[i]->ClearShowData();
				m_pLpPriceVolPic[i]->Invalidate();
			}
			SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));

		}
	}

	return true;
}

bool CDlgLpPriceVol::OnCbxDiffTypeChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	if (m_nDiffType != pEvt->nCurSel)
	{
		m_nDiffType = pEvt->nCurSel;
		if (m_nDiffType > eLPCSDT_NULL)
		{
			m_pTextBaseWnd->SetVisible(TRUE,TRUE);
			m_pCbxBaseWnd->SetVisible(TRUE,TRUE);
		}
		else
		{
			m_pTextBaseWnd->SetVisible(FALSE, TRUE);
			m_pCbxBaseWnd->SetVisible(FALSE, TRUE);

		}
		int nReCalcType = eLPCW_ShowWnd;
		SendMsg(m_uMsgThreadID, Syn_ChangeShowDiff, (char*)&nReCalcType, sizeof(nReCalcType));
	}
	return true;
}

bool CDlgLpPriceVol::OnCbxBaseWndChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	if (m_nBaseWnd != pEvt->nCurSel)
	{
		m_nBaseWnd = pEvt->nCurSel;
		int nReCalcType = eLPCW_ShowWnd;
		SendMsg(m_uMsgThreadID, Syn_ChangeShowDiff, (char*)&nReCalcType, sizeof(nReCalcType));
	}
	return true;
}

bool CDlgLpPriceVol::OnCbxPeriodChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nPeriod = pCbx->GetItemData(nSel);
	int nPic = -1;
	for (int i = 0; i < MAX_PICNUM; ++i)
	{
		if (m_pCbxPeriod[i] == pCbx)
		{
			nPic = i;
			break;
		}
	}
	pCbx->KillFocus();
	if (nPic < 0)
		return true;
	if (nPeriod != eLPCT_Date && nPeriod == m_nPicPeriod[nPic])
		return true;
	int nDate = nPeriod;
	if (nPeriod == eLPCT_Date)
	{
		SYSTEMTIME st;
		m_pDtpDate[nPic]->GetTime(st);
		nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
		GetLocalTime(&st);
		int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
		m_pDtpDate[nPic]->SetVisible(TRUE, TRUE);
		m_nPicPeriod[nPic] = eLPCT_Date;
		if (nDate == nToday)
			return true;
	}
	else
		m_pDtpDate[nPic]->SetVisible(FALSE, TRUE);

	m_LpDataVec[nPic].clear();
	m_pLpPriceVolPic[nPic]->ClearShowData();
	m_pLpPriceVolPic[nPic]->Invalidate();

	if (m_nPicPeriod[nPic] != nPeriod ||
		(m_nPicPeriod[nPic] == eLPCT_Date && m_nPicDate[nPic] != nDate))
	{
		GetLpPriceData(nPeriod, nDate);
		m_nPicPeriod[nPic] = nPeriod;
		m_nPicDate[nPic] = nDate;
	}

	return false;
}

bool CDlgLpPriceVol::OnDtpDateChange(EventArgs * e)
{
	EventDateTimeChanged *pEvt = (EventDateTimeChanged*)e;
	SDateTimePicker *pDtp = (SDateTimePicker *)pEvt->sender;
	//int nChildCount = pDtp->GetChildrenCount();
	//for (int i = 0; i < nChildCount; ++i)
	//{
	//	SCalendarEx* pCal = (SCalendarEx*)pDtp->GetChild(GSW_FIRSTCHILD);
	//	BOOL bVisible = pCal->IsVisible();
	//	OutputDebugStringFormat("日历存在:%d\n", bVisible);

	//}



	int nPic = -1;
	for (int i = 0; i < MAX_PICNUM; ++i)
	{
		if (m_pDtpDate[i] == pDtp)
		{
			nPic = i;
			break;
		}
	}

	int nYearMonth = pEvt->newTime.wYear * 100 + pEvt->newTime.wMonth;
	if (nYearMonth == m_nDtpYearMonth[nPic])
	{
		OutputDebugStringFormat("点击日期 进行计算\n");
		SetPicDataDate(pEvt->newTime, nPic);

	}
	else
	{
		OutputDebugStringFormat("点击其他\n");
		m_nDtpYearMonth[nPic] = nYearMonth;

	}
	//pDtp->
	//SYSTEMTIME st = pEvt->newTime;
	//int nPic = -1;
	//for (int i = 0; i < MAX_PICNUM; ++i)
	//{
	//	if (m_pDtpDate[i] == pDtp)
	//	{
	//		nPic = i;
	//		break;
	//	}
	//}
	//if (nPic < 0)
	//	return true;
	//int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	//if (nDate == m_nPicDate[nPic])
	//	return true;
	////GetLpPriceData(eLPCT_Date, nDate);
	//m_nPicDate[nPic] = nDate;
	return true;
}

void CDlgLpPriceVol::InitWindowPos()
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strFileName;
	strFileName.Format(".\\config\\LpPriceVol.position");
	ifile.open(strFileName, std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
		ifile.close();
	}
	else
		CenterWindow(g_MainWnd);
}

void CDlgLpPriceVol::InitControls()
{
	m_pEditStockID = FindChildByName2<SEdit>(L"edit_stockID");
	m_pCbxPicNum = FindChildByName2<SComboBox>(L"cbx_picNum");
	m_pCbxDataType = FindChildByName2<SComboBox>(L"cbx_dataType");
	m_pCbxDataLevel = FindChildByName2<SComboBox>(L"cbx_dataLevel");
	m_pCbxBasePrice = FindChildByName2<SComboBox>(L"cbx_basePrice");
	m_pEditBasePrice = FindChildByName2<SEdit>(L"edit_basePrice");
	m_pLbStockID = FindChildByName2<SListBox>(L"lb_stockID");
	m_pCbxDiffType = FindChildByName2<SComboBox>(L"cbx_diffType");
	m_pTextBaseWnd = FindChildByName2<SStatic>(L"txt_baseWnd");
	m_pCbxBaseWnd = FindChildByName2<SComboBox>(L"cbx_baseWnd");

	if (m_pEditStockID)
	{
		m_pEditStockID->GetEventSet()->subscribeEvent(
			EventRENotify::EventID,
			Subscriber(&CDlgLpPriceVol::OnEditStockIDChange, this));

	}

	int nPos = 0;
	if (m_pCbxPicNum)
	{
		m_pCbxPicNum->InsertItem(nPos++, L"1", NULL, 1);
		m_pCbxPicNum->InsertItem(nPos++, L"2", NULL, 2);
		m_pCbxPicNum->InsertItem(nPos++, L"3", NULL, 3);
		m_pCbxPicNum->InsertItem(nPos++, L"4", NULL, 4);
		m_pCbxPicNum->InsertItem(nPos++, L"6", NULL, 6);
		m_pCbxPicNum->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxPicNumChange, this));
	}


	nPos = 0;
	if (m_pCbxDataType)
	{
		m_pCbxDataType->InsertItem(nPos++, L"数值", NULL, 0);
		m_pCbxDataType->InsertItem(nPos++, L"比例", NULL, 1);
		m_pCbxDataType->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxDataTypeChange, this));

	}
	nPos = 0;

	if (m_pCbxDataLevel)
	{
		m_pCbxDataLevel->InsertItem(nPos++, L"自适应", NULL, 0);
		m_pCbxDataLevel->InsertItem(nPos++, L"10", NULL, 10);
		m_pCbxDataLevel->InsertItem(nPos++, L"20", NULL, 20);
		m_pCbxDataLevel->InsertItem(nPos++, L"50", NULL, 50);
		m_pCbxDataLevel->InsertItem(nPos++, L"100", NULL, 100);
		m_pCbxDataLevel->InsertItem(nPos++, L"1分钱", NULL, 9999);
		m_pCbxDataLevel->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxDataLevelChange, this));

	}
	nPos = 0;

	if (m_pCbxBasePrice)
	{
		m_pCbxBasePrice->InsertItem(nPos++, L"无", NULL, -1);
		m_pCbxBasePrice->InsertItem(nPos++, L"最新价", NULL, -2);
		m_pCbxBasePrice->InsertItem(nPos++, L"0", NULL, 0);
		m_pCbxBasePrice->InsertItem(nPos++, L"1", NULL, 1);
		m_pCbxBasePrice->InsertItem(nPos++, L"2", NULL, 2);
		m_pCbxBasePrice->InsertItem(nPos++, L"5", NULL, 5);
		m_pCbxBasePrice->InsertItem(nPos++, L"10", NULL, 10);
		m_pCbxBasePrice->InsertItem(nPos++, L"自定义", NULL, 9999);
		m_pCbxBasePrice->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxBasePriceChange, this));

	}

	if (m_pEditBasePrice)
		m_pEditBasePrice->GetEventSet()->subscribeEvent(
			EventRENotify::EventID,
			Subscriber(&CDlgLpPriceVol::OnEditBasePriceChange, this));

	if (m_pCbxDiffType)
	{
		m_pCbxDiffType->InsertItem(nPos++, L"无", NULL, 0);
		m_pCbxDiffType->InsertItem(nPos++, L"只显示不同数据", NULL, 1);
		m_pCbxDiffType->InsertItem(nPos++, L"与指定窗口叠加", NULL, 2);
		m_pCbxDiffType->InsertItem(nPos++, L"与指定窗口做差", NULL, 3);
		m_pCbxDiffType->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxDiffTypeChange, this));

	}

	if (m_pCbxBaseWnd)
	{
		m_pCbxBaseWnd->InsertItem(nPos++, L"无", NULL, 0);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口1", NULL, 1);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口2", NULL, 2);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口3", NULL, 3);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口4", NULL, 4);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口5", NULL, 5);
		m_pCbxBaseWnd->InsertItem(nPos++, L"窗口6", NULL, 6);
		m_pCbxBaseWnd->GetEventSet()->subscribeEvent(
			EventCBSelChange::EventID,
			Subscriber(&CDlgLpPriceVol::OnCbxBaseWndChange, this));

	}


	if (m_pLbStockID)
	{
		m_pLbStockID->GetEventSet()->subscribeEvent(
			EventLButtonDown::EventID,
			Subscriber(&CDlgLpPriceVol::OnLbStockIDLButtonDown, this));

	}
	SStringW str;
	for (int i = 0; i < MAX_PICNUM; ++i)
	{
		m_pGrpPic[i] = FindChildByName2<SGroup>(str.Format(L"group_lp%d", i + 1));
		m_pCbxPeriod[i] = FindChildByName2<SComboBox>(str.Format(L"cbx_period%d", i + 1));
		if (m_pCbxPeriod[i])
		{
			nPos = 0;
			m_pCbxPeriod[i]->InsertItem(nPos, L"全时段", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"1个月前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"3个月前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"6个月前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"1年前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"2年前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"3年前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"5年前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"10年前", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->InsertItem(nPos, L"自定义", NULL, nPos);
			++nPos;
			m_pCbxPeriod[i]->GetEventSet()->subscribeEvent(
				EventCBSelChange::EventID,
				Subscriber(&CDlgLpPriceVol::OnCbxPeriodChange, this));
		}
		m_pDtpDate[i] = FindChildByName2<SDateTimePicker>(str.Format(L"dtp_date%d", i + 1));
		if (m_pDtpDate[i])
			m_pDtpDate[i]->GetEventSet()->subscribeEvent(
				EventDateTimeChanged::EventID,
				Subscriber(&CDlgLpPriceVol::OnDtpDateChange, this));

		m_pLpPriceVolPic[i] = FindChildByName2<SLpPriceVolPic>(str.Format(L"lpPiceVol%d", i + 1));

	}
}


void CDlgLpPriceVol::InitConfig()
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\LpPriceVol.ini");
	CIniFile ini(strPosFile);
	if (m_pCbxPicNum)
	{
		int nPicNum = ini.GetIntA("CommonSetting", "PicNum", 0);
		m_pCbxPicNum->SetCurSel(nPicNum);
	}
	if (m_pCbxDataType)
	{
		int nDataType = ini.GetIntA("CommonSetting", "DataType", 0);
		m_pCbxDataType->SetCurSel(nDataType);
		m_nDataType = m_pCbxDataType->GetItemData(nDataType);
	}
	if (m_pCbxDataLevel)
	{
		int nDataLevel = ini.GetIntA("CommonSetting", "DataLevel", 0);
		m_pCbxDataLevel->SetCurSel(nDataLevel);
		m_nDataLevel = m_pCbxDataLevel->GetItemData(nDataLevel);

	}
	if (m_pCbxBasePrice)
	{
		int nBasePriceType = ini.GetIntA("CommonSetting", "BasePrice", 0);
		m_pCbxBasePrice->SetCurSel(nBasePriceType);
		if (m_nBasePriceType == eLPCBPT_User)
		{
			SStringA strPrice = ini.GetStringA("CommonSetting", "UserPrice", 0);
			m_pEditBasePrice->SetWindowTextW(StrA2StrW(strPrice));
			m_pEditBasePrice->EnableWindow(TRUE);
			m_fBasePrice = atof(strPrice);
		}
		else m_fBasePrice = m_pCbxBasePrice->GetItemData(nBasePriceType);
	}

	if (m_pCbxDiffType)
	{
		m_nDiffType = ini.GetIntA("CommonSetting", "DiffType", eLPCSDT_NULL);
		m_pCbxDiffType->SetCurSel(m_nDiffType);
		if (m_pCbxBaseWnd)
		{
			m_nBaseWnd = ini.GetIntA("CommonSetting", "BaseWnd", 0);
			m_pCbxBaseWnd->SetCurSel(m_nDiffType);
			if (m_nDiffType > eLPCSDT_NULL)
			{
				m_pTextBaseWnd->SetVisible(TRUE);
				m_pCbxBaseWnd->SetVisible(TRUE);
			}

		}



	}

	for (int i = 0; i < MAX_PICNUM; ++i)
	{
		SStringA strSection;
		strSection.Format("PicSetting%d", i + 1);
		m_nPicPeriod[i] = ini.GetIntA(strSection, "PeriodType", 0);
		m_pCbxPeriod[i]->SetCurSel(m_nPicPeriod[i]);
		if (m_nPicPeriod[i] != eLPCT_Date)
			m_nPicDate[i] = m_nPicPeriod[i];
		else
		{
			m_nPicDate[i] = ini.GetIntA(strSection, "Date", 0);
			if (m_nPicDate[i] != 0)
			{
				SYSTEMTIME st;
				st.wYear = m_nPicDate[i] / 10000;
				st.wMonth = m_nPicDate[i] / 100 % 100;
				st.wDay = m_nPicDate[i] % 100;
				m_pDtpDate[i]->SetTime(st);
			}
		}
		SYSTEMTIME st;
		m_pDtpDate[i]->GetTime(st);
		m_nDtpYearMonth[i] = st.wYear * 100 + st.wMonth;
		if (m_pLpPriceVolPic[i])
			m_pLpPriceVolPic[i]->SetCalcPara(m_nDataType, m_nDataLevel, m_fBasePrice);
	}
	SetShowPicNum(m_pCbxPicNum->GetItemData(m_pCbxPicNum->GetCurSel()));
}

void CDlgLpPriceVol::SaveConfig()
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\LpPriceVol.ini");
	CIniFile ini(strPosFile);
	if (m_pCbxPicNum)
		ini.WriteIntA("CommonSetting", "PicNum", m_pCbxPicNum->GetCurSel());
	if (m_pCbxDataType)
		ini.WriteIntA("CommonSetting", "DataType", m_pCbxDataType->GetCurSel());
	if (m_pCbxDataLevel)
		ini.WriteIntA("CommonSetting", "DataLevel", m_pCbxDataLevel->GetCurSel());
	if (m_pCbxBasePrice)
	{
		ini.WriteIntA("CommonSetting", "BasePrice", m_pCbxBasePrice->GetCurSel());
		if (m_pCbxBasePrice->GetItemData(m_pCbxBasePrice->GetCurSel()) == 9999)
		{
			ini.WriteStringA("CommonSetting", "UserPrice", StrW2StrA(m_pEditBasePrice->GetWindowTextW()));
		}
	}

	if (m_pCbxDiffType)
	{
		ini.WriteIntA("CommonSetting", "DiffType", m_nDiffType);
		ini.GetIntA("CommonSetting", "BaseWnd", m_nDiffType);
	}

	for (int i = 0; i < MAX_PICNUM; ++i)
	{
		SStringA strSection;
		strSection.Format("PicSetting%d", i + 1);
		ini.WriteIntA(strSection, "PeriodType", m_nPicPeriod[i]);
		ini.WriteIntA(strSection, "Date", m_nPicDate[i]);
	}

}

void CDlgLpPriceVol::SetPicDataDate(SYSTEMTIME st, int nPic)
{
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nDate == m_nPicDate[nPic])
		return;
	GetLpPriceData(eLPCT_Date, nDate);
	m_nPicDate[nPic] = nDate;

}

map<int, PeriodPriceVolInfo> CDlgLpPriceVol::CalcShowPic(vector<PeriodPriceVolInfo>& dataVec)
{
	if (dataVec.empty())
		return map<int, PeriodPriceVolInfo>();
	map<int, PeriodPriceVolInfo> ShowDataMap;
	if (m_nDataLevel == 9999)
	{
		for (auto &it : dataVec)
			ShowDataMap[it.nPriceMulti100] = it;
	}
	else
	{
		int nLevel = m_nDataLevel;
		if (m_nDataLevel == 0)
			nLevel = m_nMinLevel;

		if (m_nBasePriceType == eLPCBPT_NULL)
		{
			int nMaxDiff = m_nMaxPrice - m_nMinPrice + 1;
			double fPerDiff = nMaxDiff * 1.0 / nLevel;
			int nIndex = 0;
			for (int i = 0; i < nLevel; ++i)
			{
				int nPriceMax = m_nMinPrice + (i + 1) *fPerDiff + 0.5;
				ShowDataMap[nPriceMax - 1] = PeriodPriceVolInfo(nPriceMax - 1, dataVec[0].nPeriodType);
				for (; nIndex < dataVec.size(); ++nIndex)
				{
					if (dataVec[nIndex].nPriceMulti100 < nPriceMax)
						ShowDataMap[nPriceMax - 1] += dataVec[nIndex];
					else
						break;
				}
			}
		}
		else
		{
			if (m_nBasePriceType == eLPCBPT_LastPx)
			{
				if (m_pShowRpsData->hash.count(m_StockID))
					m_fBasePrice = m_pShowRpsData->hash[m_StockID].fPrice;
				else
					m_fBasePrice = 0;
			}
			nLevel /= 2;
			int nBasePrice = m_fBasePrice * 100 + 0.5;
			int nMaxDiff = max(abs(m_nMaxPrice - nBasePrice), abs(nBasePrice - m_nMinPrice));
			nMaxDiff++;
			double fPerDiff = nMaxDiff * 1.0 / nLevel;
			int nIndex = 0;
			int nReverseID = dataVec.size() - 1;
			vector<PeriodPriceVolInfo> lowerVec;
			vector<PeriodPriceVolInfo> upperVec;

			for (auto &it : dataVec)
			{
				if (it.nPriceMulti100 >= nBasePrice)
					upperVec.emplace_back(it);
				else
					lowerVec.emplace_back(it);
			}
			int nIndexLow = lowerVec.size() - 1;
			int nIndexUp = 0;

			for (int i = 0; i < nLevel; ++i)
			{
				int nPriceMax = nBasePrice + (i + 1) *fPerDiff + 0.5;
				int nPriceMinShow = nBasePrice - i *fPerDiff + 0.5;
				int nPriceMin = nBasePrice - (i + 1) *fPerDiff + 0.5;
				for (; nIndexUp < upperVec.size(); ++nIndexUp)
				{
					if (dataVec[nIndexUp].nPriceMulti100 < nPriceMax)
						ShowDataMap[nPriceMax - 1] += upperVec[nIndexUp];
					else
						break;
				}
				for (; nIndexLow >= 0; --nIndexLow)
				{
					if (dataVec[nIndexLow].nPriceMulti100 >= nPriceMin)
						ShowDataMap[nPriceMinShow] += lowerVec[nIndexLow];
					else
						break;
				}

				if (ShowDataMap.count(nPriceMax - 1))
					ShowDataMap[nPriceMax - 1].nPriceMulti100 = nPriceMax - 1;
				else
					ShowDataMap[nPriceMax - 1] = PeriodPriceVolInfo(nPriceMax - 1, m_nDataType);
				if (ShowDataMap.count(nPriceMinShow))
					ShowDataMap[nPriceMinShow].nPriceMulti100 = nPriceMinShow;
				else
					ShowDataMap[nPriceMinShow] = PeriodPriceVolInfo(nPriceMinShow, m_nDataType);;

			}

		}
	}
	HandleShowPicWithDataType(ShowDataMap);
	return ShowDataMap;
}

void CDlgLpPriceVol::HandleShowPicWithDataType(map<int, PeriodPriceVolInfo>& ShowDataMap)
{
	if (m_nDataType == eLPCDT_Ratio)
	{
		double fTotalOrder = 0;
		double fTotalVol = 0;
		for (auto &it : ShowDataMap)
		{
			fTotalVol += it.second.fActBigBuyVol + it.second.fActMidBuyVol + it.second.fActSmallBuyVol +
				it.second.fPasBigBuyVol + it.second.fPasMidBuyVol + it.second.fPasSmallBuyVol;

			fTotalOrder += it.second.fActBigBuyOrder + it.second.fActMidBuyOrder + it.second.fActSmallBuyOrder +
				it.second.fPasBigBuyOrder + it.second.fPasMidBuyOrder + it.second.fPasSmallBuyOrder;
		}

		for (auto &it : ShowDataMap)
		{
			it.second.fActBigBuyVol /= (fTotalVol / 100);
			it.second.fActMidBuyVol /= (fTotalVol / 100);
			it.second.fActSmallBuyVol /= (fTotalVol / 100);
			it.second.fPasBigBuyVol /= (fTotalVol / 100);
			it.second.fPasMidBuyVol /= (fTotalVol / 100);
			it.second.fPasSmallBuyVol /= (fTotalVol / 100);
			it.second.fActBigSellVol /= (fTotalVol / 100);
			it.second.fActMidSellVol /= (fTotalVol / 100);
			it.second.fActSmallSellVol /= (fTotalVol / 100);
			it.second.fPasBigSellVol /= (fTotalVol / 100);
			it.second.fPasMidSellVol /= (fTotalVol / 100);
			it.second.fPasSmallSellVol /= (fTotalVol / 100);

			it.second.fActBigBuyOrder /= (fTotalOrder / 100);
			it.second.fActMidBuyOrder /= (fTotalOrder / 100);
			it.second.fActSmallBuyOrder /= (fTotalOrder / 100);
			it.second.fPasBigBuyOrder /= (fTotalOrder / 100);
			it.second.fPasMidBuyOrder /= (fTotalOrder / 100);
			it.second.fPasSmallBuyOrder /= (fTotalOrder / 100);
			it.second.fActBigSellOrder /= (fTotalOrder / 100);
			it.second.fActMidSellOrder /= (fTotalOrder / 100);
			it.second.fActSmallSellOrder /= (fTotalOrder / 100);
			it.second.fPasBigSellOrder /= (fTotalOrder / 100);
			it.second.fPasMidSellOrder /= (fTotalOrder / 100);
			it.second.fPasSmallSellOrder /= (fTotalOrder / 100);

		}
	}

}

BOOL CDlgLpPriceVol::HandleMaxPiantData(map<int, PeriodPriceVolInfo>& ShowDataMap)
{
	BOOL bChange = FALSE;
	for (auto&it : ShowDataMap)
	{
		double tmpMax = max(it.second.fActBigBuyVol + it.second.fActMidBuyVol + it.second.fActSmallBuyVol,
			it.second.fPasBigBuyVol + it.second.fPasMidBuyVol + it.second.fPasSmallBuyVol);
		if (tmpMax > m_fMaxVol)
		{
			m_fMaxVol = tmpMax;
			bChange = TRUE;
		}
	}
	return bChange;
}

BOOL CDlgLpPriceVol::HandleDiffType(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd)
{
	if (m_nDiffType == eLPCSDT_NULL)
		return FALSE;
	if (m_nDiffType != eLPCSDT_ShowDiff && m_nBaseWnd == 0)
		return FALSE;
	if (m_nDiffType == eLPCSDT_ShowDiff)
		return HandleShowDiff(ShowDataMap, nPicWnd);
	if (m_nDiffType == eLPCSDT_Overlapping)
		return HandleShowOverlapping(ShowDataMap, nPicWnd);
	if (m_nDiffType == eLPCSDT_Deflate)
		return HandleShowDeflate(ShowDataMap, nPicWnd);



}

BOOL CDlgLpPriceVol::HandleShowDiff(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd)
{
	if (m_nBaseWnd == 0)
	{
		for (auto &it : ShowDataMap)
		{
			BOOL bIsSame = TRUE;
			for (int i = 0; i < m_nShowPicNum; ++i)
			{
				if (!it.second.IsDataSame(m_LpShowData[i][it.first]))
				{
					bIsSame = FALSE;
					break;
				}
			}
			if (bIsSame)
				it.second = PeriodPriceVolInfo(it.second.nPriceMulti100,
					it.second.nPeriodType);
		}
		return TRUE;
	}
	else
	{
		if (m_nBaseWnd == nPicWnd + 1)
			return TRUE;
		for (auto &it : ShowDataMap)
		{
			if (it.second.IsDataSame(m_LpShowData[m_nBaseWnd - 1][it.first]))
				it.second = PeriodPriceVolInfo(it.second.nPriceMulti100,
					it.second.nPeriodType);
		}
	}
	return FALSE;
}

BOOL SOUI::CDlgLpPriceVol::HandleShowOverlapping(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd)
{
	if (m_nBaseWnd == 0)
		return FALSE;
	if (m_nBaseWnd == nPicWnd + 1)
		return TRUE;
	ShowDataMap = m_LpShowData[m_nBaseWnd - 1];
	return FALSE;
}

BOOL CDlgLpPriceVol::HandleShowDeflate(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd)
{
	if (m_nShowPicNum == 0)
		return FALSE;
	if (m_nBaseWnd == nPicWnd + 1)
		return TRUE;
	for (auto &it : ShowDataMap)
		it.second.AbsDiff(m_LpShowData[m_nBaseWnd - 1][it.first]);
	return FALSE;
}

void CDlgLpPriceVol::InitDataHandleMap()
{
	m_DataHandleMap[Syn_LpPriceVol] = &CDlgLpPriceVol::
		OnUpdateLpPriceVol;
	m_DataHandleMap[Syn_ReCalcLpPriceVol] = &CDlgLpPriceVol::
		ReCalcShowData;
	m_DataHandleMap[Syn_ListData] = &CDlgLpPriceVol::
		OnUpdateListData;
	m_DataHandleMap[Syn_ChangeShowDiff] = &CDlgLpPriceVol::
		ChangeShowDiff;

}

void CDlgLpPriceVol::DataHandle()
{
	int MsgId;
	char *info;
	int msgLength;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			break;
		}
		if (m_DataHandleMap.count(MsgId))
		{
			auto pFuc = m_DataHandleMap[MsgId];
			if (pFuc)
				(this->*pFuc)(msgLength, info);
		}
		delete[]info;
		info = nullptr;
	}
}

void CDlgLpPriceVol::OnUpdateLpPriceVol(int nMsgLength, const char * info)
{
	ReceiveLpPriVolInfo* pLpInfo = (ReceiveLpPriVolInfo*)info;
	if (m_StockID != pLpInfo->Message)
		return;
	set<int>PicSet;
	for (int i = 0; i < m_nShowPicNum; ++i)
	{
		if (m_nPicPeriod[i] == pLpInfo->PeriodType)
		{
			if (pLpInfo->PeriodType != eLPCT_Date || m_nPicDate[i] == pLpInfo->Date)
			{
				PicSet.insert(i);
			}
		}
	}

	if (PicSet.empty())
		return;
	bool bRangeChange = false;

	int nOffset = sizeof(*pLpInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	PeriodPriceVolInfo* dataArr = (PeriodPriceVolInfo*)(info + nOffset);
	int nDataSize = (nMsgLength - nOffset) / sizeof(PeriodPriceVolInfo);
	if (nDataSize > 0)
	{
		if (dataArr[0].nPriceMulti100 < m_nMinPrice)
		{
			m_nMinPrice = dataArr[0].nPriceMulti100;
			bRangeChange = true;
		}

		if (dataArr[nDataSize - 1].nPriceMulti100 > m_nMaxPrice)
		{
			m_nMaxPrice = dataArr[nDataSize - 1].nPriceMulti100;
			bRangeChange = true;
		}

	}
	vector<PeriodPriceVolInfo> dataVec(nDataSize);
	memcpy_s(&dataVec[0], nMsgLength - nOffset, dataArr, nMsgLength - nOffset);
	map<int, PeriodPriceVolInfo> ShowDatMap = CalcShowPic(dataVec);

	BOOL bDiffChange = FALSE;
	BOOL bMaxVolChange = FALSE;
	if (m_nDataType == eLPCDT_Num)
		bMaxVolChange = HandleMaxPiantData(ShowDatMap);

	for (auto &nPic : PicSet)
	{
		m_LpShowData[nPic] = ShowDatMap;
		m_LpDataVec[nPic].assign(dataVec.begin(), dataVec.end());
	}


	for (auto &nPic : PicSet)
	{
		auto tmpShowDataMap = ShowDatMap;
		bDiffChange = HandleDiffType(tmpShowDataMap, nPic);
		m_pLpPriceVolPic[nPic]->SetCalcPara(m_nDataType, m_nDataLevel, m_fBasePrice);
		if(m_nDiffType != eLPCSDT_Overlapping)
			m_pLpPriceVolPic[nPic]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
		else 
		{
			m_pLpPriceVolPic[nPic]->UpdateData(m_LpShowData[nPic], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
			if (m_nBaseWnd != nPic + 1)
				m_pLpPriceVolPic[nPic]->UpdateCmpData(tmpShowDataMap);
		}

		::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, nPic);

	}

	if (bDiffChange)
	{
		for (int i = 0; i < m_nShowPicNum; ++i)
		{
			if (PicSet.count(i) == 0)
			{
				auto tmpShowDataMap = m_LpShowData[i];
				HandleDiffType(tmpShowDataMap, i);
				if (m_nDiffType != eLPCSDT_Overlapping)
					m_pLpPriceVolPic[i]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
				else
				{
					m_pLpPriceVolPic[i]->UpdateData(m_LpShowData[i], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
					if (m_nBaseWnd != i + 1)
						m_pLpPriceVolPic[i]->UpdateCmpData(tmpShowDataMap);
				}
				::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, i);
			}
		}

	}
	else if (bRangeChange || bMaxVolChange)
	{
		for (int i = 0; i < m_nShowPicNum; ++i)
		{
			if (PicSet.count(i) == 0)
			{
				m_pLpPriceVolPic[i]->UpdateDataRange(m_nMaxPrice, m_nMinPrice, m_fMaxVol);
				::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, i);

			}
		}
	}


}

void CDlgLpPriceVol::ReCalcShowData(int nMsgLength, const char * info)
{
	int nCalcWndType = *(int*)info;
	set<int> calcWndSet;
	if (nCalcWndType < eLPCW_ShowWnd)
		calcWndSet.insert(nCalcWndType);
	else if (nCalcWndType == eLPCW_ShowWnd)
		for (int i = 0; i < m_nShowPicNum; ++i)
			calcWndSet.insert(i);
	else if (nCalcWndType == eLPCW_AllWnd)
		for (int i = 0; i < MAX_PICNUM; ++i)
			calcWndSet.insert(i);

	BOOL bMaxVolChange = FALSE;
	for (auto &it : calcWndSet)
	{
		m_LpShowData[it] = CalcShowPic(m_LpDataVec[it]);
		if (m_nDataType == eLPCDT_Num)
			bMaxVolChange = HandleMaxPiantData(m_LpShowData[it]);
	}

	BOOL bDiffChange = FALSE;
	for (auto &it : calcWndSet)
	{
		auto tmpShowDataMap = m_LpShowData[it];
		bDiffChange =HandleDiffType(tmpShowDataMap, it);
		if (m_nDiffType != eLPCSDT_Overlapping)
			m_pLpPriceVolPic[it]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
		else
		{
			m_pLpPriceVolPic[it]->UpdateData(m_LpShowData[it], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
			if (m_nBaseWnd != it + 1)
				m_pLpPriceVolPic[it]->UpdateCmpData(tmpShowDataMap);
		}
		m_pLpPriceVolPic[it]->SetCalcPara(m_nDataType, m_nDataLevel, m_fBasePrice);
		m_pLpPriceVolPic[it]->UpdateDataRange(m_nMaxPrice, m_nMinPrice, m_fMaxVol);
		::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, it);
	}

	if (bDiffChange)
	{
		for (int i = 0; i < m_nShowPicNum; ++i)
		{

			if (calcWndSet.count(i) == 0)
			{
				auto tmpShowDataMap = m_LpShowData[i];
				HandleDiffType(tmpShowDataMap, i);
				if (m_nDiffType != eLPCSDT_Overlapping)
					m_pLpPriceVolPic[i]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
				else
				{
					m_pLpPriceVolPic[i]->UpdateData(m_LpShowData[i], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
					if (m_nBaseWnd != i + 1)
						m_pLpPriceVolPic[i]->UpdateCmpData(tmpShowDataMap);
				}
				::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, i);
			}
		}

	}
	else if (bMaxVolChange)
	{
		for (int i = 0; i < m_nShowPicNum; ++i)
		{
			if (calcWndSet.count(i) == 0)
			{

				m_pLpPriceVolPic[i]->UpdateDataRange(m_nMaxPrice, m_nMinPrice, m_fMaxVol);
				::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, i);
			}
		}
	}
}

void SOUI::CDlgLpPriceVol::OnUpdateListData(int nMsgLength, const char * info)
{
	if (m_nBasePriceType == eLPCBPT_LastPx)
	{
		double fPrice = 0;
		if (m_pShowRpsData->hash.count(m_StockID))
			fPrice = m_pShowRpsData->hash[m_StockID].fPrice;
		else
			fPrice = 0;
		if (m_fBasePrice != fPrice)
		{
			int nCalcWndType = eLPCW_ShowWnd;
			ReCalcShowData(sizeof(nCalcWndType), (char*)& nCalcWndType);
		}
	}
}

void CDlgLpPriceVol::ChangeShowDiff(int nMsgLength, const char * info)
{
	int nCalcWndType = *(int*)info;
	set<int> calcWndSet;
	if (nCalcWndType < eLPCW_ShowWnd)
		calcWndSet.insert(nCalcWndType);
	else if (nCalcWndType == eLPCW_ShowWnd)
		for (int i = 0; i < m_nShowPicNum; ++i)
			calcWndSet.insert(i);
	else if (nCalcWndType == eLPCW_AllWnd)
		for (int i = 0; i < MAX_PICNUM; ++i)
			calcWndSet.insert(i);
	BOOL bDiffChange = FALSE;
	for (auto &it : calcWndSet)
	{
		auto tmpShowDataMap = m_LpShowData[it];
		bDiffChange = HandleDiffType(tmpShowDataMap, it);
		if (m_nDiffType != eLPCSDT_Overlapping)
			m_pLpPriceVolPic[it]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
		else
		{
			m_pLpPriceVolPic[it]->UpdateData(m_LpShowData[it], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
			if (m_nBaseWnd != it + 1)
				m_pLpPriceVolPic[it]->UpdateCmpData(tmpShowDataMap);
		}
		m_pLpPriceVolPic[it]->SetCalcPara(m_nDataType, m_nDataLevel, m_fBasePrice);
		m_pLpPriceVolPic[it]->UpdateDataRange(m_nMaxPrice, m_nMinPrice, m_fMaxVol);
		::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, it);
	}
	if (bDiffChange)
	{
		for (int i = 0; i < m_nShowPicNum; ++i)
		{

			if (calcWndSet.count(i) == 0)
			{
				auto tmpShowDataMap = m_LpShowData[i];
				HandleDiffType(tmpShowDataMap, i);
				if (m_nDiffType != eLPCSDT_Overlapping)
					m_pLpPriceVolPic[i]->UpdateData(tmpShowDataMap, m_nMaxPrice, m_nMinPrice, m_fMaxVol);
				else
				{
					m_pLpPriceVolPic[i]->UpdateData(m_LpShowData[i], m_nMaxPrice, m_nMinPrice, m_fMaxVol);
					if (m_nBaseWnd != i + 1)
						m_pLpPriceVolPic[i]->UpdateCmpData(tmpShowDataMap);
				}
				::PostMessageW(m_hWnd, WM_LPPRICEVOL_MSG, LPDMsg_UpdatePic, i);
			}
		}

	}

}



void CDlgLpPriceVol::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

//TODO:消息映射
void CDlgLpPriceVol::OnBtnClose()
{
	GetNative()->SendMessage(WM_CLOSE);
}

void CDlgLpPriceVol::OnLButtonUp(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	if (m_pEditBasePrice->IsFocused())
	{
		CRect rc = m_pEditBasePrice->GetWindowRect();
		if (rc.PtInRect(point))
			return;
		m_pEditBasePrice->KillFocus();
	}
	else
	{
		for (int i = 0; i < MAX_PICNUM; ++i)
		{
			if (m_pDtpDate[i]->IsFocused())
			{
				CRect rc = m_pDtpDate[i]->GetWindowRect();
				if (rc.PtInRect(point))
					return;
				m_pDtpDate[i]->KillFocus();
				SYSTEMTIME st;
				m_pDtpDate[i]->GetTime(st);
				SetPicDataDate(st, i);
				break;
			}
		}

	}
}


BOOL CDlgLpPriceVol::SetShowPicNum(int nNum)
{
	if (nNum == m_nShowPicNum)
		return FALSE;
	if (1 == nNum)
	{
		m_pGrpPic[0]->SetAttribute(L"pos", L"5,5,-5,-5");
	}
	else if (2 == nNum)
	{
		m_pGrpPic[0]->SetAttribute(L"pos", L"5,5,%50,-5");
		m_pGrpPic[1]->SetAttribute(L"pos", L"%50,5,-5,-5");
	}
	else if (3 == nNum)
	{
		m_pGrpPic[0]->SetAttribute(L"pos", L"5,5,%33,-5");
		m_pGrpPic[1]->SetAttribute(L"pos", L"%33,5,%67,-5");
		m_pGrpPic[2]->SetAttribute(L"pos", L"%67,5,-5,-5");
	}
	else if (4 == nNum)
	{
		m_pGrpPic[0]->SetAttribute(L"pos", L"5,5,%50,%50");
		m_pGrpPic[1]->SetAttribute(L"pos", L"%50,5,-5,%50");
		m_pGrpPic[2]->SetAttribute(L"pos", L"5,%50,%50,-5");
		m_pGrpPic[3]->SetAttribute(L"pos", L"%50,%50,-5,-5");
	}
	else if (6 == nNum)
	{
		m_pGrpPic[0]->SetAttribute(L"pos", L"5,5,%33,%50");
		m_pGrpPic[1]->SetAttribute(L"pos", L"%33,5,%67,%50");
		m_pGrpPic[2]->SetAttribute(L"pos", L"%67,5,-5,%50");
		m_pGrpPic[3]->SetAttribute(L"pos", L"5,%50,%33,-5");
		m_pGrpPic[4]->SetAttribute(L"pos", L"%33,%50,%67,-5");
		m_pGrpPic[5]->SetAttribute(L"pos", L"%67,%50,-5,-5");

	}
	set<pair<int, int>> DataGetSet;
	m_nShowPicNum = nNum;

	if (CheckPicMinHeight() && m_nDataLevel == 0)
	{
		int nReCalcType = eLPCW_ShowWnd;
		for (int i = 0; i < m_nShowPicNum; ++i)
		{
			m_pLpPriceVolPic[i]->ClearShowData();
			m_pLpPriceVolPic[i]->Invalidate();
		}
		SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));
	}

	for (int i = 0; i < nNum; ++i)
	{
		if (!m_pLpPriceVolPic[i]->IsDataInited())
			DataGetSet.insert(make_pair(m_nPicPeriod[i], m_nPicDate[i]));
		else if (!m_pLpPriceVolPic[i]->IsShowParaFit(m_nDataType, m_nDataLevel, m_fBasePrice))
		{
			m_pLpPriceVolPic[i]->ClearShowData();
			m_pLpPriceVolPic[i]->Invalidate();
			SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&i, sizeof(i));
		}
		m_pGrpPic[i]->SetVisible(TRUE, TRUE);
	}

	for (auto &it : DataGetSet)
		GetLpPriceData(it.first, it.second);

	for (int i = nNum; i < MAX_PICNUM; ++i)
		m_pGrpPic[i]->SetVisible(FALSE, TRUE);

	SStringW str;
	if (m_pCbxBaseWnd)
	{
		int nCount = m_pCbxBaseWnd->GetCount();
		int nSel = m_pCbxBaseWnd->GetCurSel();
		if (nCount < m_nShowPicNum + 1)
		{
			for (int i = nCount; i < m_nShowPicNum + 1; ++i)
				m_pCbxBaseWnd->InsertItem(i,str.Format(L"窗口%d", i), NULL, i);
		}
		else if(nCount >m_nShowPicNum + 1)
		{
			for (int i = nCount - 1; i > m_nShowPicNum; --i)
				m_pCbxBaseWnd->DeleteString(i);
			if (nSel > m_nShowPicNum + 1)
				m_pCbxBaseWnd->SetCurSel(0);
		}

	}

	return TRUE;
}

BOOL CDlgLpPriceVol::GetLpPriceData(int nPeriod, int nDate)
{
	if (m_StockID == "")
		return FALSE;
	LpDataGetInfo GetInfo;
	GetInfo.hWnd = m_hWnd;
	strcpy_s(GetInfo.StockID, m_StockID);
	GetInfo.PeriodType = nPeriod;
	GetInfo.Date = nDate;
	SendMsg(m_SynThreadID, Syn_GetLpPriceVol,
		(char*)&GetInfo, sizeof(GetInfo));
	return TRUE;
}

BOOL CDlgLpPriceVol::CheckPicMinHeight()
{
	CRect rc = GetClientRect();
	int nHeight = rc.Height() - 65 - 5;
	if (m_nShowPicNum > 3)
		nHeight /= 2;
	nHeight -= (5 + 20 + 5);
	BOOL bChange = FALSE;
	int nLevel = (nHeight - HEADHEIGHT * 2) / PERTOTALHEIGHT;
	if (nLevel != m_nMinLevel)
	{
		m_nMinLevel = nLevel;
		bChange = TRUE;
	}
	return bChange;
}


void CDlgLpPriceVol::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CDlgLpPriceVol::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CDlgLpPriceVol::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CDlgLpPriceVol::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE, TRUE);
		pBtnMax->SetVisible(FALSE, TRUE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE, TRUE);
		pBtnMax->SetVisible(TRUE, TRUE);
	}

	CRect rc = GetClientRect();
	OutputDebugStringFormat("整个窗口的高度:%d\n", rc.Height());

	if (CheckPicMinHeight() && m_nDataLevel == 0)
	{
		int nReCalcType = eLPCW_ShowWnd;
		for (int i = 0; i < m_nShowPicNum; ++i)
		{
			m_pLpPriceVolPic[i]->ClearShowData();
			m_pLpPriceVolPic[i]->Invalidate();
		}
		SendMsg(m_uMsgThreadID, Syn_ReCalcLpPriceVol, (char*)&nReCalcType, sizeof(nReCalcType));
	}
}

