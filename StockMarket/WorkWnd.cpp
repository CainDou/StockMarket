#include "stdafx.h"
#include "WorkWnd.h"
#include"Define.h"
#include"SFenShiPic.h"
#include"SKlinePic.h"
#include"SColorListCtrlEx.h"
#include "DlgKbElf.h"
#include"DlgMacdPara.h"
#include"DlgBandPara.h"
#include"DlgMaPara.h"
#include"DlgEmaPara.h"
#include <helper/SMenu.h>
#include "IniFile.h"
#include "DataProc.h"

#define MAX_SUBPIC 3
#define SHOWDATACOUNT 2
#define MAX_TICK 6000

CWorkWnd::CWorkWnd() :SHostWnd(_T("LAYOUT:wnd_work"))
{
	m_pDlgKbElf = nullptr;
	m_uThreadID = INVALID_THREADID;
}


CWorkWnd::~CWorkWnd()
{
	if(INVALID_THREADID != m_uThreadID)
		SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_workThread.joinable())
		m_workThread.join();
	m_pPreSelBtn = nullptr;
}

void CWorkWnd::SetGroup(RpsGroup Group, HWND hParWnd)
{
	m_Group = Group;
	m_hParWnd = hParWnd;
	if (Group_SWL1 == m_Group)
	{
		m_pTextIndy->SetVisible(FALSE);
		m_pBtnConn1->SetVisible(FALSE);
		m_pBtnConn2->SetVisible(FALSE);
		m_pTextTitle->SetWindowTextW(L"申万一级行业");
	}
	else if (Group_SWL2 == m_Group)
	{
		m_pBtnConn1->SetWindowTextW(L"联");
		m_pBtnConn2->SetVisible(FALSE);
		m_pBtnConn2->SetAttribute(L"size", L"0,22");
		m_pTextTitle->SetWindowTextW(L"申万二级行业");
	}
	else if (Group_Stock == m_Group)
		m_pTextTitle->SetWindowTextW(L"股票");
}

void CWorkWnd::InitShowConfig(InitPara initPara)
{
	vector<SStringA> SubPicWndNameVec;
	SubPicWndNameVec.reserve(MAX_SUBPIC);
	SubPicWndNameVec.emplace_back("全市场RPS");
	SubPicWndNameVec.emplace_back("1级行业RPS");
	SubPicWndNameVec.emplace_back("2级行业RPS");

	m_InitPara = initPara;
	if (Group_Stock == m_Group)
	{
		m_pFenShiPic->InitSubPic(MAX_SUBPIC, SubPicWndNameVec);
		m_pKlinePic->InitSubPic(MAX_SUBPIC, SubPicWndNameVec);
	}
	else
	{
		m_pFenShiPic->InitSubPic(1, SubPicWndNameVec);
		m_pKlinePic->InitSubPic(1, SubPicWndNameVec);
	}
	m_pFenShiPic->SetRpsGroup(m_Group);
	m_pFenShiPic->InitShowPara(m_InitPara);
	m_pKlinePic->SetRpsGroup(m_Group);
	m_pKlinePic->SetParentHwnd(m_hWnd);
	m_pKlinePic->InitShowPara(m_InitPara);
	m_PicPeriod = m_ListPeriod = m_InitPara.Period;

	m_pPreSelBtn = m_pPeriodBtnMap[m_ListPeriod];
	m_pPreSelBtn->SetAttribute(L"colorText", L"#00ffffff");
	m_bListConn1 = initPara.Connect1;
	m_bListConn2 = initPara.Connect2;
	if (m_bListConn1)
		SetBtnState(m_pBtnConn1, true);
	if (m_bListConn2)
		SetBtnState(m_pBtnConn2, true);

	if (m_bListConn1 || m_bListConn2)
	{
		m_ListShowInd = m_InitPara.ShowIndy;
		SetListShowIndyStr(m_pTextIndy);
	}

}

InitPara CWorkWnd::OutPutInitPara()
{
	m_pFenShiPic->OutPutShowPara(m_InitPara);
	m_pKlinePic->OutPutShowPara(m_InitPara);
	m_InitPara.Period = m_ListPeriod;
	m_InitPara.Connect1 = m_bListConn1;
	m_InitPara.Connect2 = m_bListConn2;
	strcpy_s(m_InitPara.ShowIndy, m_ListShowInd);
	return m_InitPara;
}

void CWorkWnd::ClearData()
{
	m_InfoVec.clear();
}

void CWorkWnd::InitList()
{
	m_pList = FindChildByID2<SColorListCtrlEx>(R.id.ls_rps);
	m_pList->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK,
		Subscriber(&CWorkWnd::OnListDbClick, this));
	m_pList->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
		Subscriber(&CWorkWnd::OnListLClick, this));
	UpdateListShowStock();
	SHeaderCtrlEx * pHeader =
		(SHeaderCtrlEx *)m_pList->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(3);
	pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CWorkWnd::OnListHeaderClick, this));
	pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP,
		Subscriber(&CWorkWnd::OnListHeaderSwap, this));
	m_bListInited = true;
	UpdateList();

}

void CWorkWnd::ReInitList()
{
	m_bListInited = false;
	UpdateListShowStock();
	m_bListInited = true;
	UpdateList();

}

void CWorkWnd::SetDataPoint(void * pData, int DataType)
{
	switch (DataType)
	{
	case DT_ListData:
		m_pListDataMap = (map<int, TimeLineMap>*)pData;
		break;
	}
}

void CWorkWnd::SetPicUnHandled()
{
	m_pFenShiPic->SetPicUnHandled();
	m_pKlinePic->SetPicUnHandled();
}

void CWorkWnd::ReSetPic()
{
	if (m_strSubStock != "")
		ShowPicWithNewID(m_strSubStock,true);
}

void CWorkWnd::SetListInfo(vector<StockInfo>& infoVec,
	strHash<SStringA>& StockNameMap)
{
	m_InfoVec = infoVec;
	m_StockName = StockNameMap;
	m_pDlgKbElf->SetStockInfo(m_InfoVec);
}

void CWorkWnd::UpdateTodayPointData(SStringA pointName,
	vector<CoreData>& dataVec, bool bLast)
{
	m_PointData[m_PicPeriod][pointName] = dataVec;
	if (bLast)
		m_PointReadyMap[m_PicPeriod] = true;
}

void CWorkWnd::CloseWnd()
{
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_workThread.joinable())
		m_workThread.join();
	m_uThreadID = INVALID_THREADID;
	m_pPreSelBtn = nullptr;
	m_pListDataMap = nullptr;
	m_pList->DeleteAllItems();
	m_PointData.clear();
	m_IndexMarketVec.clear();
	m_IndexMarketVec.shrink_to_fit();
	m_StockMarketVec.clear();
	m_StockMarketVec.shrink_to_fit();
	m_KlineMap.clear();
	m_preCloseMap.hash.clear();
	m_StockName.hash.clear();
	m_dataNameVec.clear();
	m_dataNameVec.shrink_to_fit();
	m_PointReadyMap.clear();
	m_KlineGetMap.clear();
	m_PointGetMap.clear();
	m_SubPicShowNameVec.clear();
	m_SubPicShowNameVec.shrink_to_fit();
	m_ListPosMap.hash.clear();
}

void CWorkWnd::OnInit(EventArgs * e)
{
	::InitializeCriticalSection(&m_csClose);
	m_pFenShiPic = FindChildByID2<SFenShiPic>(R.id.fenshiPic);
	m_pKlinePic = FindChildByID2<SKlinePic>(R.id.klinePic);
	m_pTextTitle = FindChildByID2<SStatic>(R.id.text_Group);
	m_pTextIndy = FindChildByID2<SStatic>(R.id.text_ShowIndy);
	m_pBtnMarket = FindChildByID2<SImageButton>(R.id.btn_Market);
	m_pPeriodBtnMap[Period_FenShi] = 
		FindChildByID2<SImageButton>(R.id.btn_FS);
	m_pPeriodBtnMap[Period_1Min] =
		FindChildByID2<SImageButton>(R.id.btn_M1);
	m_pPeriodBtnMap[Period_5Min] =
		FindChildByID2<SImageButton>(R.id.btn_M5);
	m_pPeriodBtnMap[Period_15Min] =
		FindChildByID2<SImageButton>(R.id.btn_M15);
	m_pPeriodBtnMap[Period_30Min] =
		FindChildByID2<SImageButton>(R.id.btn_M30);
	m_pPeriodBtnMap[Period_60Min] =
		FindChildByID2<SImageButton>(R.id.btn_M60);
	m_pPeriodBtnMap[Period_1Day] =
		FindChildByID2<SImageButton>(R.id.btn_Day);
	m_pBtnConn1 = FindChildByID2<SImageButton>(R.id.btn_ListConnect1);
	m_pBtnConn2 = FindChildByID2<SImageButton>(R.id.btn_ListConnect2);

	InitProcFucMap();
	InitNameVec();
	m_workThread = thread(&CWorkWnd::DataProc, this);
	m_uThreadID = *(unsigned*)&m_workThread.get_id();
	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);

}

LRESULT CWorkWnd::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case WDMsg_UpdateList:
		UpdateList();
		if (m_pList->IsVisible())
			m_pList->Invalidate();
		break;
	case WDMsg_UpdatePic:
		if (m_pFenShiPic->IsVisible())
			m_pFenShiPic->Invalidate();
		if (m_pKlinePic->IsVisible())
			m_pKlinePic->Invalidate();
		break;
	case WDMsg_SubIns:
		ShowPicWithNewID(m_pDlgKbElf->GetShowPicInfo());
		break;
	case WDMsg_ChangeIndy:
		UpdateListShowStock();
		break;
	case WDMsg_SetFocus:
		CSimpleWnd::SetFocus();
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CWorkWnd::OnFSMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case FSMSG_UPDATE:
		break;
	case FSMSG_EMA:
		m_pFenShiPic->SetEmaPara((int*)wp);
		::SendMsg(m_uThreadID, WW_FSEma, NULL, 0);
		break;
	case FSMSG_MACD:
		m_pFenShiPic->SetMacdPara((int*)wp);
		::SendMsg(m_uThreadID, WW_FSMacd, NULL, 0);
		break;
	default:
		break;
	}
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
	return 0;
}

LRESULT CWorkWnd::OnKlineMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case KLINEMSG_UPDATE:
		break;
	case KLINEMSG_MA:
		m_pKlinePic->SetMaPara((int*)wp);
		::SendMsg(m_uThreadID, WW_KlineMa, NULL, 0);
		break;
	case KLINEMSG_MACD:
		m_pKlinePic->SetMacdPara((int*)wp);
		::SendMsg(m_uThreadID, WW_KlineMacd, NULL, 0);
		break;
	case KLINEMSG_BAND:
		m_pKlinePic->SetBandPara(*(BandPara_t*)wp);
		::SendMsg(m_uThreadID, WW_KlineBand, NULL, 0);
		break;
	default:
		break;
	}
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
	return 0;
}

void CWorkWnd::OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case FM_Deal:
		m_pFenShiPic->SetDealState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetDealState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_Volume:
		m_pFenShiPic->SetVolumeState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetVolumeState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_MACD:
		m_pFenShiPic->SetMacdState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetMacdState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_RPS:
		m_pFenShiPic->SetRpsState(SP_FULLMARKET);
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetRpsState(SP_FULLMARKET);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_MacdPara:
	{
		CDlgMacdPara *pDlg = new CDlgMacdPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pFenShiPic->GetMacdPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case FM_Avg:
		m_pFenShiPic->SetAvgState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetAvgState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_EMA:
		m_pFenShiPic->SetEmaState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetEmaState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_EmaPara:
	{
		CDlgEmaPara *pDlg = new CDlgEmaPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pFenShiPic->GetEmaPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case FM_L1RPS:
		m_pFenShiPic->SetRpsState(SP_SWINDYL1);
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetRpsState(SP_SWINDYL1);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_L2RPS:
		m_pFenShiPic->SetRpsState(SP_SWINDYL2);
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetRpsState(SP_SWINDYL2);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	default:
		break;
	}
}
void CWorkWnd::OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case KM_Deal:
		m_pKlinePic->SetDealState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetDealState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_MA:
		m_pKlinePic->SetBandState(false, false);
		m_pKlinePic->SetMaState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMaState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_Band:
		m_pKlinePic->SetMaState(false, false);
		m_pKlinePic->SetBandState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetBandState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_Volume:
		m_pKlinePic->SetVolumeState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetVolumeState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_MACD:
		m_pKlinePic->SetMacdState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMacdState();
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_RPS:
		m_pKlinePic->SetRpsState(SP_FULLMARKET);
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetRpsState(SP_FULLMARKET);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_MacdPara:
	{
		CDlgMacdPara *pDlg = new CDlgMacdPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMacdPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_BandPara:
	{
		CDlgBandPara *pDlg = new CDlgBandPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetBandPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_MaPara:
	{
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_L1RPS:
		m_pKlinePic->SetRpsState(SP_SWINDYL1);
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetRpsState(SP_SWINDYL1);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_L2RPS:
		m_pKlinePic->SetRpsState(SP_SWINDYL2);
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetRpsState(SP_SWINDYL2);
		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;

	default:
		break;
	}
}
void CWorkWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == 229)
	{
		m_nLastChar = 0;

		for (int i = VK_NUMPAD0; i <= VK_NUMPAD9; ++i)
		{
			SHORT nRps = ::GetKeyState(i);
			if (HIBYTE(nRps) == 0xff)
			{
				m_nLastChar = i - 0x30;
				return;
			}
		}
		for (int i = '0'; i <= '9'; ++i)
		{
			SHORT nRps = ::GetKeyState(i);
			if (HIBYTE(nRps) == 0xff)
			{
				m_nLastChar = i;
				return;
			}
		}

	}
}
void CWorkWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == VK_ESCAPE)
	{
		OnBtnShowTypeChange(true);
	}
	else if (nChar == 229)
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hParWnd, &rc);
			m_pDlgKbElf->SetWindowPos(HWND_TOPMOST, rc.right - 320,
				rc.bottom - 370, 0, 0, SWP_NOSIZE);
			m_pDlgKbElf->ClearInput();
			SStringW input = L"";
			if (m_nLastChar >= '0' && m_nLastChar <= '9')
				input = (char)m_nLastChar;
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
			//SetMsgHandled(TRUE);
		}
	}
	else if ((nChar >= 0x30 && nChar <= 0x39) ||
		(nChar >= 0x41 && nChar <= 0x5A) ||
		(nChar >= VK_NUMPAD0&&nChar <= VK_NUMPAD9))
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hParWnd, &rc);
			m_pDlgKbElf->SetWindowPos(NULL,
				rc.right - 320, rc.bottom - 370, 0, 0,
				SWP_NOSIZE);
			m_pDlgKbElf->ClearInput();
			SStringW input;
			if (nChar >= VK_NUMPAD0&&nChar <= VK_NUMPAD9)
				input = (char)(nChar - 0x30);
			else if (nChar >= 0x30 && nChar <= 0x39)
			{
				bool bShift = HIBYTE(GetKeyState(VK_SHIFT)) > 0 ? true : false;
				if (bShift)
				{
					if (nChar == 0x32) input = 0x40;
					else if (nChar == 0x36) input = 0x5E;
					else if (nChar == 0x37) input = 0x26;
					else if (nChar == 0x38) input = 0x2A;
					else if (nChar == 0x39) input = 0x28;
					else if (nChar == 0x30) input = 0x29;
					else
						input = nChar - 0x10;
				}
				else
					input = (char)nChar;
			}
			else
			{
				bool bCaps = LOBYTE(GetKeyState(VK_CAPITAL)) > 0 ? true : false;
				bool bShift = HIBYTE(GetKeyState(VK_SHIFT)) > 0 ? true : false;
				input = bCaps ^ bShift ?
					(char)nChar : (char)(nChar + 32);
			}
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
		}

	}
}
void CWorkWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	if (m_pList->IsVisible())
		return;
	if (m_pFenShiPic->IsVisible())
	{
		SMenu menu;
		menu.LoadMenuW(L"smenu:menu_fenshi");
		if (m_pFenShiPic->GetDealState())
			menu.CheckMenuItem(FM_Deal, MF_CHECKED);
		if (m_pFenShiPic->GetVolumeState())
			menu.CheckMenuItem(FM_Volume, MF_CHECKED);
		if (m_pFenShiPic->GetMacdState())
			menu.CheckMenuItem(FM_MACD, MF_CHECKED);
		if (m_pFenShiPic->GetRpsState(SP_FULLMARKET))
			menu.CheckMenuItem(FM_RPS, MF_CHECKED);
		if (m_pFenShiPic->GetAvgState())
			menu.CheckMenuItem(FM_Avg, MF_CHECKED);
		if (m_pFenShiPic->GetEmaState())
			menu.CheckMenuItem(FM_EMA, MF_CHECKED);
		int nSubPicNum = m_pFenShiPic->GetShowSubPicNum();
		for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		{
			if (m_pFenShiPic->GetRpsState(i))
				menu.CheckMenuItem(FM_EmaPara + i, MF_CHECKED);
		}
		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;
	}

	if (m_pKlinePic->IsVisible())
	{
		SMenu menu;
		menu.LoadMenuW(L"smenu:menu_kline");
		if (m_pKlinePic->GetDealState())
			menu.CheckMenuItem(KM_Deal, MF_CHECKED);
		if (m_pKlinePic->GetVolumeState())
			menu.CheckMenuItem(KM_Volume, MF_CHECKED);
		if (m_pKlinePic->GetMacdState())
			menu.CheckMenuItem(KM_MACD, MF_CHECKED);
		if (m_pKlinePic->GetRpsState(SP_FULLMARKET))
			menu.CheckMenuItem(KM_RPS, MF_CHECKED);
		if (m_pKlinePic->GetMaState())
			menu.CheckMenuItem(KM_MA, MF_CHECKED);
		if (m_pKlinePic->GetBandState())
			menu.CheckMenuItem(KM_Band, MF_CHECKED);
		int nSubPicNum = m_pKlinePic->GetShowSubPicNum();
		for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		{
			if (m_pKlinePic->GetRpsState(i))
				menu.CheckMenuItem(KM_MaPara + i, MF_CHECKED);
		}

		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;

	}
}
BOOL CWorkWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	ScreenToClient(&pt);
	static uint64_t tick = 0;
	uint64_t nowTick = GetTickCount64();
	if (nowTick - tick < 500)
		return FALSE;
	tick = nowTick;
	if (m_pList->IsVisible())
		m_pList->SetFocus();
	else
	{
		std::vector < pair<SStringA, int>>ListPosVec(
			m_ListPosMap.hash.begin(), m_ListPosMap.hash.end());
		std::sort(ListPosVec.begin(), ListPosVec.end(),
			[](const pair<SStringA, int> &a, const pair<SStringA, int> b)
		{return a.second < b.second; });
		int i = 0;
		for (i; i < ListPosVec.size(); ++i)
		{
			if (m_strSubStock == ListPosVec[i].first)
				break;
		}
		if (zDelta < 0)
			++i;
		else
			--i;
		if (i < 0)
			i = ListPosVec.size() - 1;
		if (i >= ListPosVec.size())
			i = 0;
		ShowPicWithNewID(ListPosVec[i].first);
	}
	return 0;
}
void CWorkWnd::SwitchPic2List()
{
	m_pFenShiPic->SetVisible(FALSE, TRUE);
	m_pKlinePic->SetVisible(FALSE, TRUE);
	m_pKlinePic->ClearTip();
	m_pList->SetVisible(TRUE, TRUE);
	UpdateList();
	m_pList->SetFocus();
	m_pList->RequestRelayout();

}
void CWorkWnd::SwitchList2Pic(int nPeriod)
{
	m_pList->SetVisible(FALSE, TRUE);
	m_pFenShiPic->SetVisible(FALSE, FALSE);
	m_pKlinePic->SetVisible(FALSE, FALSE);
	if (nPeriod == Period_FenShi)
	{
		m_pFenShiPic->SetVisible(TRUE, TRUE);
		m_pFenShiPic->SetFocus();
		m_pFenShiPic->RequestRelayout();

	}
	else
	{
		m_pKlinePic->SetVisible(TRUE, TRUE);
		m_pKlinePic->SetFocus();
		m_pKlinePic->RequestRelayout();
	}
}

void CWorkWnd::DataProc()
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
		auto pFuc = m_dataHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}

}
void CWorkWnd::InitProcFucMap()
{
	m_dataHandleMap[WW_ListData] =
		&CWorkWnd::OnUpdateListData;
	m_dataHandleMap[WW_Point] =
		&CWorkWnd::OnUpdatePoint;
	m_dataHandleMap[WW_TodayPoint] =
		&CWorkWnd::OnUpdateTodayPoint;
	m_dataHandleMap[WW_HisPoint] =
		&CWorkWnd::OnUpdateHisPoint;
	m_dataHandleMap[WW_RTIndexMarket] =
		&CWorkWnd::OnUpdateIndexMarket;
	m_dataHandleMap[WW_RTStockMarket] =
		&CWorkWnd::OnUpdateStockMarket;
	m_dataHandleMap[WW_HisIndexMarket] =
		&CWorkWnd::OnUpdateHisIndexMarket;
	m_dataHandleMap[WW_HisStockMarket] =
		&CWorkWnd::OnUpdateHisStockMarket;
	m_dataHandleMap[WW_HisKline] =
		&CWorkWnd::OnUpdateHisKline;
	m_dataHandleMap[WW_CloseInfo] =
		&CWorkWnd::OnUpdateCloseInfo;
	m_dataHandleMap[WW_ChangeIndy] =
		&CWorkWnd::OnChangeShowIndy;
	m_dataHandleMap[WW_FSEma] =
		&CWorkWnd::OnFenShiEma;
	m_dataHandleMap[WW_FSMacd] =
		&CWorkWnd::OnFenShiMacd;
	m_dataHandleMap[WW_KlineMa] =
		&CWorkWnd::OnKlineMa;
	m_dataHandleMap[WW_KlineMacd] =
		&CWorkWnd::OnKlineMacd;
	m_dataHandleMap[WW_KlineBand] =
		&CWorkWnd::OnKlineBand;

}
void CWorkWnd::InitNameVec()
{
	m_dataNameVec.emplace_back("close");
	m_dataNameVec.emplace_back("RPS520");
	m_dataNameVec.emplace_back("MACD520");
	m_dataNameVec.emplace_back("Point520");
	m_dataNameVec.emplace_back("Rank520");
	m_dataNameVec.emplace_back("RPS2060");
	m_dataNameVec.emplace_back("MACD2060");
	m_dataNameVec.emplace_back("Point2060");
	m_dataNameVec.emplace_back("Rank2060");

	m_SubPicShowNameVec.resize(MAX_SUBPIC);
	for (int i = 0; i < MAX_SUBPIC; ++i)
		m_SubPicShowNameVec[i].reserve(SHOWDATACOUNT);
	m_SubPicShowNameVec[0].emplace_back("Point520");
	m_SubPicShowNameVec[0].emplace_back("Point2060");
	m_SubPicShowNameVec[1].emplace_back("L1Point520");
	m_SubPicShowNameVec[1].emplace_back("L1Point2060");
	m_SubPicShowNameVec[2].emplace_back("L2Point520");
	m_SubPicShowNameVec[2].emplace_back("L2Point2060");


}
bool CWorkWnd::OnListHeaderClick(EventArgs * pEvtBase)
{

	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SHeaderCtrlEx *pHeader = (SHeaderCtrlEx*)pEvt->sender;
	SHDITEM hditem;
	hditem.mask = SHDI_ORDER;
	pHeader->GetItem(pEvt->iItem, &hditem);
	int nShowOrder = pHeader->GetOriItemIndex(hditem.iOrder);

	if (hditem.iOrder == 0)
		return false;

	SColorListCtrlEx * pList = (SColorListCtrlEx *)pHeader->GetParent();

	pHeader->SetItemSort(m_SortPara.nShowCol, ST_NULL);


	if (hditem.iOrder == 1)
	{
		m_SortPara.nCol = hditem.iOrder;
		m_SortPara.nShowCol = nShowOrder;
		m_SortPara.nFlag = 0;
	}
	else if (nShowOrder != m_SortPara.nShowCol)
	{
		m_SortPara.nCol = hditem.iOrder;
		m_SortPara.nShowCol = nShowOrder;
		m_SortPara.nFlag = 1;

	}
	else
		m_SortPara.nFlag = !m_SortPara.nFlag;
	if (hditem.iOrder != 1)
	{
		if (m_SortPara.nFlag == 0)
			pHeader->SetItemSort(m_SortPara.nShowCol, ST_UP);
		else
			pHeader->SetItemSort(m_SortPara.nShowCol, ST_DOWN);

	}
	SortList(pList, true);

	return true;
}
bool CWorkWnd::OnListHeaderSwap(EventArgs * pEvtBase)
{
	EventHeaderItemSwap *pEvt = (EventHeaderItemSwap*)pEvtBase;
	SHeaderCtrlEx* pHead = (SHeaderCtrlEx*)pEvt->sender;
	int nColCount = pHead->GetItemCount();
	std::vector<int> Order(nColCount);
	SColorListCtrlEx * pList = (SColorListCtrlEx *)pHead->GetParent();

	for (int i = 0; i < nColCount; ++i)
	{
		Order[i] = pHead->GetOriItemIndex(i);
		if (m_SortPara.nCol == i)
			m_SortPara.nShowCol = Order[i];
	}
	return true;
}
bool CWorkWnd::OnListDbClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (m_ListPosMap.hash.count(StockID) == 0)
		StockID += "I";
	//m_strSubStock = StockID;

	m_PicPeriod = m_ListPeriod;
	ShowPicWithNewID(StockID);
	m_bShowList = false;
	return true;
}
bool CWorkWnd::OnListLClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	if (m_Group == Group_Stock)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA strID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (strID[0] == '0')
		strID += "I";
	int nGroup = m_Group;
	char msg[12] = "";
	memcpy_s(msg, 12, &nGroup, 4);
	memcpy_s(msg + 4, 12, strID, strID.GetLength() + 1);
	SendMsg(m_uParWndThreadID, WW_ChangeIndy, msg, 12);
	return true;
}
void CWorkWnd::UpdateListShowStock()
{
	SStringA strInd = m_ListShowInd;
	m_ListPosMap.hash.clear();
	m_pList->DeleteAllItems();
	int nCount = 0;
	SStringW tmp;
	for (auto &it : m_InfoVec)
	{
		if (strInd == "" || strInd == it.ScaleID ||
			strInd == it.SWL1ID || strInd == it.SWL2ID)
		{
			tmp.Format(L"%d", nCount + 1);
			m_pList->InsertItem(nCount, tmp);
			SStringW strID = StrA2StrW(it.SecurityID);
			if (strID.GetLength() > 6)
				strID = strID.Left(6);
			m_pList->SetSubItemText(nCount, SHead_ID,
				strID);
			m_pList->SetSubItemText(nCount, SHead_Name,
				StrA2StrW(it.SecurityName));
			m_ListPosMap.hash[it.SecurityID] = nCount;
			nCount++;
		}
	}
	SetListShowIndyStr(m_pTextIndy);

	UpdateList();
	//m_pList->UpdateLayout();
	m_pList->RequestRelayout();
}

void CWorkWnd::UpdateList()
{
	if (!m_bListInited)
		return;
	if (!m_pList->IsVisible())
		return;
	SStringW tmp;
	auto ListData = m_pListDataMap->at(m_ListPeriod);
	for (auto &it : m_ListPosMap.hash)
	{
		SStringA StockID = it.first;
		if (ListData.count(StockID))
		{
			auto & dataVec = ListData[StockID];
			if (!isnan(dataVec["close"].value) && dataVec["close"].value != 0)
			{
				double fClose = dataVec["close"].value;
				double fPreClose = m_preCloseMap.hash[StockID];
				COLORREF cl = RGBA(255, 255, 255, 255);
				if (fClose > fPreClose)
					cl = RGBA(255, 0, 0, 255);
				else if (fClose < fPreClose && fClose != 0)
					cl = RGBA(0, 255, 0, 255);
				tmp.Format(L"%.2f", fClose);
				m_pList->SetSubItemText(it.second, SHead_LastPx, tmp, cl);
				double chgPct = (fClose - fPreClose) / fPreClose * 100;
				if (!isnan(chgPct) && !isinf(chgPct) && fClose != 0)
					tmp.Format(L"%.2f", (fClose - fPreClose) / fPreClose * 100);
				else
					tmp = L"-";
				m_pList->SetSubItemText(it.second, SHead_ChangePct, tmp, cl);
			}
			else
			{
				if (dataVec["close"].value == 0)
					m_pList->SetSubItemText(it.second, SHead_LastPx, L"0.00");
				else
					m_pList->SetSubItemText(it.second, SHead_LastPx, L"-");
				for (int i = SHead_ChangePct; i < SHead_ItmeCount; ++i)
					m_pList->SetSubItemText(it.second, i, L"-");
				continue;
			}
			for (int i = 1; i < m_dataNameVec.size(); ++i)
			{
				if (dataVec.count(m_dataNameVec[i])
					&& !isnan(dataVec[m_dataNameVec[i]].value))
				{
					COLORREF cl = RGBA(255, 255, 0, 255);
					if (i + SHead_ChangePct == SHead_Point520
						|| i + SHead_ChangePct == SHead_Point2060)
					{
						if (dataVec[m_dataNameVec[i]].value >= 80)
							cl = RGBA(255, 0, 0, 255);
						else if (dataVec[m_dataNameVec[i]].value < 60)
							cl = RGBA(0, 255, 0, 255);
						else
							cl = RGBA(255, 255, 255, 255);
					}
					else if (i + SHead_ChangePct == SHead_RPS520
						|| i + SHead_ChangePct == SHead_RPS2060
						|| i + SHead_ChangePct == SHead_MACD520
						|| i + SHead_ChangePct == SHead_MACD2060)
					{
						if (dataVec[m_dataNameVec[i]].value > 0)
							cl = RGBA(255, 0, 0, 255);
						else if (dataVec[m_dataNameVec[i]].value < 0)
							cl = RGBA(0, 255, 0, 255);
						else
							cl = RGBA(255, 255, 255, 255);
					}

					if (i + SHead_ChangePct == SHead_Rank520 ||
						i + SHead_ChangePct == SHead_Rank2060)
						tmp.Format(L"%.0f", dataVec[m_dataNameVec[i]].value);
					else
						tmp.Format(L"%.03f", dataVec[m_dataNameVec[i]].value);
					m_pList->SetSubItemText(it.second,
						i + SHead_ChangePct, tmp, cl);
				}
				else
					m_pList->SetSubItemText(it.second,
						i + SHead_ChangePct, L"-");
			}

		}
	}
	SortList(m_pList);

}

void CWorkWnd::SortList(SColorListCtrlEx * pList, bool bSortCode)
{

	if (!bSortCode)
	{
		if (m_SortPara.nCol == 0
			|| m_SortPara.nCol == 1
			|| m_SortPara.nCol == 2)
		{
			pList->Invalidate();
			return;
		}
	}

	int colCount = pList->GetColumnCount();
	int SHead_Time = colCount - 1;
	switch (m_SortPara.nCol)
	{
	case SHead_ID:
		pList->SortItems(SortInt, &m_SortPara);
		break;
	case SHead_Name:
		pList->SortItems(SortStr, &m_SortPara);
		break;
	default:
		if (m_SortPara.nCol == SHead_LastPx || m_SortPara.nCol == SHead_ChangePct)
			m_SortPara.nDec = 2;
		else
			m_SortPara.nDec = 3;
		pList->SortItems(SortDouble, &m_SortPara);
		break;
	}
	SStringW tmp;

	m_bListInited = false;
	for (int i = 0; i < pList->GetItemCount(); i++)
	{
		tmp.Format(L"%d", i + 1);
		pList->SetSubItemText(i, 0, tmp);
		SStringA stockID = StrW2StrA(pList->GetSubItemText(i, 1));
		if (m_ListPosMap.hash.count(stockID) == 0)
			stockID += "I";
		m_ListPosMap.hash[stockID] = i;
	}
	m_bListInited = true;
}

int CWorkWnd::SortDouble(void * para1, const void * para2,
	const void * para3)
{
	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);


	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;

	int nDec = pData->nDec;

	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);

	}


	if (pData->nFlag == 0)
	{
		if (str1 == L"-")
			str1 = L"100000.00";
		if (str2 == L"-")
			str2 = L"100000.00";
		double f1 = _wtof(str1);
		double f2 = _wtof(str2);
		if (f1*f2 < 0)
		{
			if (f1 - f2 > 0)
				return 1;
			else
				return -1;
		}
		else if (f1*f2 > 0)
		{
			int n1 = _wtoi(str1);
			int n2 = _wtoi(str2);
			if (n1 != n2)
				return n1 - n2;
			else
			{
				str1 = str1.Right(nDec);
				str2 = str2.Right(nDec);
				if ((_wtoi(str1) - _wtoi(str2)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str1) - _wtoi(str2);
					else
						return _wtoi(str2) - _wtoi(str1);
				}
				else
				{
					const DXLVSUBITEMEX CodeItem1 =
						pPara1->arSubItems->GetAt(SHead_ID);
					const DXLVSUBITEMEX CodeItem2 =
						pPara2->arSubItems->GetAt(SHead_ID);

					SStringW Code1 = CodeItem1.strText;
					SStringW Code2 = CodeItem2.strText;
					return Code1.Compare(Code2);
				}
			}
		}
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

				SStringW Code1 = CodeItem1.strText;
				SStringW Code2 = CodeItem2.strText;
				return Code1.Compare(Code2);
			}
			else
			{
				if (f1 - f2 > 0)
					return 1;
				else
					return -1;
			}
		}
	}
	else
	{
		if (str1 == L"-")
			str1 = L"-100000.00";
		if (str2 == L"-")
			str2 = L"-100000.00";
		double f1 = _wtof(str1);
		double f2 = _wtof(str2);
		if (f1*f2 < 0)
		{
			if (f2 - f1 > 0)
				return 1;
			else
				return -1;
		}
		else if (f1*f2 > 0)
		{
			int n1 = 0;
			int	n2 = 0;
			n1 = _wtoi(str1);
			n2 = _wtoi(str2);

			if (n1 != n2)
				return n2 - n1;
			else
			{
				str1 = str1.Right(nDec);
				str2 = str2.Right(nDec);
				if ((_wtoi(str2) - _wtoi(str1)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str2) - _wtoi(str1);
					else
						return _wtoi(str1) - _wtoi(str2);
				}
				else
				{
					const DXLVSUBITEMEX CodeItem1 =
						pPara1->arSubItems->GetAt(SHead_ID);
					const DXLVSUBITEMEX CodeItem2 =
						pPara2->arSubItems->GetAt(SHead_ID);

					SStringW Code1 = CodeItem1.strText;
					SStringW Code2 = CodeItem2.strText;
					return Code1.Compare(Code2);
				}
			}

		}
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

				SStringW Code1 = CodeItem1.strText;
				SStringW Code2 = CodeItem2.strText;
				return Code1.Compare(Code2);
			}
			else
			{
				if (f2 - f1 > 0)
					return 1;
				else
					return -1;
			}
		}
	}
}

int CWorkWnd::SortInt(void * para1, const void * para2,
	const void * para3)
{

	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);

	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;

	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);

	}


	if (pData->nFlag == 0)
	{
		if (str1 == L"-")
			str1 = L"99999999999";
		if (str2 == L"-")
			str2 = L"99999999999";

		if (str1 == str2)
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1(CodeItem1.strText);
			SStringW Code2(CodeItem2.strText);
			return Code1.Compare(Code2);

		}

		if ((_wtoll(str1) - _wtoll(str2)) != 0)
			return _wtoll(str1) - _wtoll(str2);
		else
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1 = CodeItem1.strText;
			SStringW Code2 = CodeItem2.strText;
			return Code1.Compare(Code2);
		}
	}
	else
	{
		if (str1 == L"-")
			str1 = L"-99999999999";
		if (str2 == L"-")
			str2 = L"-99999999999";

		if (str1 == str2)
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1(CodeItem1.strText);
			SStringW Code2(CodeItem2.strText);
			return Code1.Compare(Code2);

		}

		if ((_wtoll(str2) - _wtoll(str1)) != 0)
			return _wtoll(str2) - _wtoll(str1);
		else
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1 = CodeItem1.strText;
			SStringW Code2 = CodeItem2.strText;
			return Code1.Compare(Code2);
		}
	}
}

int CWorkWnd::SortStr(void * para1, const void * para2,
	const void * para3)
{
	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);

	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;
	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);
	}


	if (pData->nFlag == 0)
		return str1.Compare(str2);
	else
		return str2.Compare(str1);
}


void CWorkWnd::OnBtnMarketClicked()
{
	OnBtnShowTypeChange();
}

void CWorkWnd::OnBtnFenShiClicked()
{
	OnBtnPeriedChange(Period_FenShi);
}

void CWorkWnd::OnBtnM1Clicked()
{
	OnBtnPeriedChange(Period_1Min);
}

void CWorkWnd::OnBtnM5Clicked()
{
	OnBtnPeriedChange(Period_5Min);

}

void CWorkWnd::OnBtnM15Clicked()
{
	OnBtnPeriedChange(Period_15Min);

}

void CWorkWnd::OnBtnM30Clicked()
{
	OnBtnPeriedChange(Period_30Min);

}

void CWorkWnd::OnBtnM60Clicked()
{
	OnBtnPeriedChange(Period_60Min);
}

void CWorkWnd::OnBtnDayClicked()
{
	OnBtnPeriedChange(Period_1Day);

}

void CWorkWnd::OnBtnListConnect1Clicked()
{
	m_bListConn1 = !m_bListConn1;
	if (!m_bListConn1)
		m_ListShowInd = "";
	SetBtnState(m_pBtnConn1, m_bListConn1);
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::OnBtnListConnect2Clicked()
{
	m_bListConn2 = !m_bListConn2;
	if (!m_bListConn2)
		m_ListShowInd = "";
	SetBtnState(m_pBtnConn2, m_bListConn2);
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::SetBtnState(SImageButton * nowBtn, SImageButton ** preBtn)
{
	if (*preBtn)
		(*preBtn)->SetAttribute(L"colorText", L"#c0c0c0ff");
	nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	*preBtn = nowBtn;
}

void CWorkWnd::SetBtnState(SImageButton * nowBtn, bool bSelected)
{
	if (bSelected)
		nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	else
		nowBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void CWorkWnd::SetBtnState(int nPeriod, bool bSelected)
{
	SImageButton * pBtn = m_pPeriodBtnMap[nPeriod];
	if (bSelected)
	{
		pBtn->SetAttribute(L"colorText", L"#00ffffff");
		m_pPreSelBtn = pBtn;
	}
	else
		pBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void CWorkWnd::OnBtnShowTypeChange(bool bFroceList)
{
	if (bFroceList)
	{
		if (m_bShowList)
		{
			if (m_Group != Group_SWL1
				&&m_ListShowInd != "")
			{
				m_ListShowInd = "";
				UpdateListShowStock();
				::PostMessage(m_hParWnd, WM_WINDOW_MSG,
					WDMsg_SaveConfig, NULL);
			}
			return;
		}
	}
	if (m_bShowList)
	{
		m_pBtnMarket->SetWindowTextW(L"个股");
		SetBtnState(m_ListPeriod, false);
		SetBtnState(m_PicPeriod, true);
		SetSelectedPeriod(m_ListPeriod);
	}
	else
	{
		m_pBtnMarket->SetWindowTextW(L"行情");
		SetBtnState(m_PicPeriod, false);
		SetBtnState(m_ListPeriod, true);
		SwitchPic2List();
	}
	m_bShowList = !m_bShowList;
}

void CWorkWnd::OnBtnPeriedChange(int nPeriod)
{
	SImageButton * pBtn = m_pPeriodBtnMap[nPeriod];
	if (pBtn == m_pPreSelBtn)
		return;
	SetBtnState(pBtn, &m_pPreSelBtn);
	if (m_bShowList)
	{
		m_ListPeriod = nPeriod;
		UpdateList();
	}
	else
		SetSelectedPeriod(nPeriod);
	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::SetListShowIndyStr(SStatic * pText)
{
	if (m_ListShowInd == "")
		pText->SetWindowTextW(L"当前分类:全市场");
	else
	{
		SStringW strIndy = StrA2StrW(m_StockName.hash[m_ListShowInd]);
		SStringW strText;
		pText->SetWindowTextW(strText.Format(L"当前分类:%s", strIndy));
	}
}

void CWorkWnd::SetFenShiShowData()
{
	SStringA stockID = m_strSubStock;
	int nShowNum = m_pFenShiPic->GetShowSubPicNum();
	vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
	vector<vector<BOOL>> rightVec(nShowNum);
	if (Group_Stock == m_Group)
	{
		vector<SStringA> nameVec;
		GetBelongingIndyName(nameVec);
		m_pFenShiPic->SetBelongingIndy(nameVec);
	}
	int *dataCount = new int[nShowNum];
	for (int i = 0; i < nShowNum; ++i)
	{
		tmpDataArr[i].resize(SHOWDATACOUNT);
		rightVec[i].resize(SHOWDATACOUNT);
		dataCount[i] = SHOWDATACOUNT;
		for (int j = 0; j < SHOWDATACOUNT; ++j)
		{
			tmpDataArr[i][j] = &m_PointData[Period_FenShi]
				[m_SubPicShowNameVec[i][j]];
			//tmpDataArr[i][j] = &m_dataVec[Period_FenShi]\
			//	[stockID][m_SubPicShowNameVec[i][j]];

			rightVec[i][j] = TRUE;
		}
	}


	m_pFenShiPic->SetSubPicShowData(dataCount,
		tmpDataArr, rightVec, m_SubPicShowNameVec,
		stockID, m_StockName.hash[stockID]);

	delete[]dataCount;
	dataCount = nullptr;

}

void CWorkWnd::SetKlineShowData(int nPeriod, BOOL bNeedReCalc)
{
	SStringA stockID = m_strSubStock;
	int nShowNum = m_pKlinePic->GetShowSubPicNum();
	vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
	vector<vector<BOOL>> rightVec(nShowNum);
	if (Group_Stock == m_Group)
	{
		vector<SStringA> nameVec;
		GetBelongingIndyName(nameVec);
		m_pKlinePic->SetBelongingIndy(nameVec);
	}
	int *dataCount = new int[nShowNum];
	for (int i = 0; i < nShowNum; ++i)
	{
		tmpDataArr[i].resize(SHOWDATACOUNT);
		rightVec[i].resize(SHOWDATACOUNT);
		dataCount[i] = SHOWDATACOUNT;
		for (int j = 0; j < SHOWDATACOUNT; ++j)
		{
			//tmpDataArr[i][j] = &m_dataVec[nPeriod]\
			//	[stockID][m_SubPicShowNameVec[i][j]];
			tmpDataArr[i][j] = &m_PointData[nPeriod]
				[m_SubPicShowNameVec[i][j]];

			rightVec[i][j] = TRUE;
		}
	}


	m_pKlinePic->SetSubPicShowData(dataCount,
		tmpDataArr, rightVec, m_SubPicShowNameVec,
		stockID, m_StockName.hash[stockID]);

	delete[]dataCount;
	dataCount = nullptr;

	m_pKlinePic->ChangePeriod(nPeriod, bNeedReCalc);

}

void CWorkWnd::GetBelongingIndyName(vector<SStringA>& nameVec)
{
	nameVec.resize(2);
	SStringA stockID = m_strSubStock;
	//stockID = stockID.Left(6);
	for (auto &it : m_InfoVec)
	{
		if (stockID == it.SecurityID)
		{
			nameVec[0] = m_StockName.hash[it.SWL1ID];
			nameVec[1] = m_StockName.hash[it.SWL2ID];
			break;
		}
	}
}

void CWorkWnd::SetSelectedPeriod(int nPeriod)
{
	if (m_PicPeriod == nPeriod
		&&m_strSubStock != "")
	{
		SwitchList2Pic(nPeriod);
		return;
	}
	m_PicPeriod = nPeriod;
	SStringA& StockID = m_strSubStock;
	if (StockID == "")
	{
		for (auto &it : m_ListPosMap.hash)
		{
			if (it.second == 0)
			{
				StockID = it.first;
				ShowPicWithNewID(StockID);
				return;
			}
		}
	}

	if (Period_FenShi != nPeriod)
	{

		if (m_KlineGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisKlineState(false);
			DataGetInfo GetInfo;
			GetInfo.hWnd = m_hParWnd;
			strcpy_s(GetInfo.StockID, StockID);
			GetInfo.Group = m_Group;
			GetInfo.Period = nPeriod;
			SendMsg(m_uParWndThreadID, WW_GetKline,
				(char*)&GetInfo, sizeof(GetInfo));
		}
		else
			m_pKlinePic->SetHisKlineState(true);

		if (m_PointGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisPointState(false);
			DataGetInfo GetInfo;
			GetInfo.hWnd = m_hParWnd;
			strcpy_s(GetInfo.StockID, StockID);
			GetInfo.Group = m_Group;
			GetInfo.Period = nPeriod;
			SendMsg(m_uParWndThreadID, WW_GetPoint,
				(char*)&GetInfo, sizeof(GetInfo));
		}
		else
			m_pKlinePic->SetHisPointState(true);

		SetKlineShowData(nPeriod, TRUE);
	}
	SwitchList2Pic(nPeriod);

}

void CWorkWnd::ShowPicWithNewID(SStringA StockID,bool bForce)
{
	if (!bForce && m_strSubStock == StockID)
		return;
	SetDataFlagFalse();
	m_PicPeriod = m_bShowList ?
		m_ListPeriod : m_PicPeriod;
	m_strSubStock = StockID;

	SStringA StockName = m_StockName.hash[StockID];
	if(m_Group!=Group_Stock)
	{
		m_pFenShiPic->SetShowData(StockID, StockName, &m_IndexMarketVec);
		m_pKlinePic->SetShowData(StockID, StockName,
			&m_IndexMarketVec, &m_KlineMap);
	}
	else
	{
		m_pFenShiPic->SetShowData(StockID, StockName, &m_StockMarketVec);
		m_pKlinePic->SetShowData(StockID, StockName,
			&m_StockMarketVec, &m_KlineMap);
	}
	//获取分时数据
	DataGetInfo GetInfo;
	GetInfo.hWnd = m_hParWnd;
	strcpy_s(GetInfo.StockID, StockID);
	GetInfo.Group = m_Group;
	GetInfo.Period = Period_FenShi;
	SendMsg(m_uParWndThreadID, WW_GetMarket,
		(char*)&GetInfo, sizeof(GetInfo));
	SendMsg(m_uParWndThreadID, WW_GetPoint,
		(char*)&GetInfo, sizeof(GetInfo));
	SetFenShiShowData();

	//获取当前订阅数据
	if (Period_FenShi != m_PicPeriod)
	{
		GetInfo.Period = m_PicPeriod;
		SendMsg(m_uParWndThreadID, WW_GetKline,
			(char*)&GetInfo, sizeof(GetInfo));
		SendMsg(m_uParWndThreadID, WW_GetPoint,
			(char*)&GetInfo, sizeof(GetInfo));
		SetKlineShowData(m_PicPeriod, FALSE);
	}
	SwitchList2Pic(m_PicPeriod);
	m_bShowList = false;
}

void CWorkWnd::SetDataFlagFalse()
{
	m_bMarketGet = false;
	m_PointReadyMap.clear();
	m_PointGetMap.clear();
	m_KlineGetMap.clear();
	m_pKlinePic->SetTodayMarketState(false);
	m_pKlinePic->SetHisKlineState(false);
	m_pKlinePic->SetHisPointState(false);
}

void CWorkWnd::OnUpdateListData(int nMsgLength, const char * info)
{
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdateList, NULL);
}

void CWorkWnd::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nDataCount = nMsgLength / sizeof(pair<int, pair<char[16], CoreData>>);
	pair<int, pair<char[16], CoreData>> *dataArr =
		(pair<int, pair<char[16], CoreData>> *)info;
	for (int i = 0; i < nDataCount; ++i)
	{
		if (m_PointReadyMap[dataArr[i].first])
			CDataProc::UpdateTmData(m_PointData[dataArr[i].first]\
				[dataArr[i].second.first],
				dataArr[i].second.second);
	}
	m_pFenShiPic->UpdateData();
	m_pKlinePic->UpdateData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateTodayPoint(int nMsgLength, const char * info)
{
	int nPeriod = *(int*)info;
	map<SStringA, vector<CoreData>> *pDataMap =
		*(map<SStringA, vector<CoreData>>**)(info + 4);
	auto & pointMap = m_PointData[nPeriod];
	for (auto &it : *pDataMap)
	{
		if (it.first.Find("Point") != -1)
			pointMap[it.first] = it.second;
	}
	delete pDataMap;
	pDataMap = nullptr;
	m_PointReadyMap[nPeriod] = true;
}

void CWorkWnd::OnUpdateHisKline(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nSize = pRecvInfo->DataSize / sizeof(KlineType);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_KlineGetMap[nPeriod] = TRUE;
	auto &KlineVec = m_KlineMap[nPeriod];
	KlineVec.resize(nSize);
	memcpy_s(&KlineVec[0], pRecvInfo->DataSize,
		info + nOffset, pRecvInfo->DataSize);
	m_pKlinePic->SetHisKlineState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	if (pRecvInfo->Group != Group_Stock)
		ProcHisPointFromMsg(pRecvInfo, info + sizeof(*pRecvInfo),
			"Point520", "Point2060");
	else
	{
		int nOffset = sizeof(*pRecvInfo);
		ReceivePointInfo *pRecvInfo1 =
			(ReceivePointInfo *)(info + nOffset);
		nOffset += sizeof(*pRecvInfo1);
		ProcHisPointFromMsg(pRecvInfo1, info + nOffset,
			"Point520", "Point2060");
		nOffset += pRecvInfo1->TotalDataSize;
		ReceivePointInfo *pRecvInfo2 =
			(ReceivePointInfo *)(info + nOffset);
		nOffset += sizeof(*pRecvInfo2);
		ProcHisPointFromMsg(pRecvInfo2, info + nOffset,
			"L1Point520", "L1Point2060");
		nOffset += pRecvInfo2->TotalDataSize;
		ReceivePointInfo *pRecvInfo3 =
			(ReceivePointInfo *)(info + nOffset);
		nOffset += sizeof(*pRecvInfo3);
		ProcHisPointFromMsg(pRecvInfo3, info + nOffset,
			"L2Point520", "L2Point2060");
	}
	m_pKlinePic->SetHisPointState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);

}


void CWorkWnd::OnUpdateIndexMarket(int nMsgLength, const char * info)
{
	CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
	SStringA SecurityID = pIndexData->SecurityID;
	m_IndexMarketVec.emplace_back(*pIndexData);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateStockMarket(int nMsgLength, const char * info)
{
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	m_StockMarketVec.emplace_back(*pStockData);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisIndexMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonIndexMarket);
	CommonIndexMarket * dataArr = (CommonIndexMarket *)(info + nOffset);
	m_IndexMarketVec.reserve(MAX_TICK);
	m_IndexMarketVec.resize(dataCount);
	memcpy_s(&m_IndexMarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_bMarketGet = TRUE;
	m_pFenShiPic->DataProc();
	m_pKlinePic->SetTodayMarketState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisStockMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonStockMarket);
	CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
	m_StockMarketVec.reserve(MAX_TICK);
	m_StockMarketVec.resize(dataCount);
	memcpy_s(&m_StockMarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_bMarketGet = TRUE;
	m_pFenShiPic->DataProc();
	m_pKlinePic->SetTodayMarketState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateCloseInfo(int nMsgLength, const char * info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	auto &closeMap = m_preCloseMap;
	::EnterCriticalSection(&m_csClose);
	closeMap.hash.clear();
	for (int i = 0; i < dataCount; ++i)
		closeMap.hash[dataArr[i].first] = dataArr[i].second;
	::LeaveCriticalSection(&m_csClose);
}

void CWorkWnd::OnChangeShowIndy(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SStringA IndexID = info + 4;
	if (Group_SWL1 == nGroup
		&& m_bListConn1)
		m_ListShowInd = IndexID;
	if(Group_SWL2 == nGroup
		&& m_bListConn2)
		m_ListShowInd = IndexID;
	::PostMessage(m_hWnd, WM_WINDOW_MSG,
		WDMsg_ChangeIndy, NULL);
}

void CWorkWnd::OnFenShiEma(int nMsgLength, const char * info)
{
	m_pFenShiPic->ReProcEMA();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnFenShiMacd(int nMsgLength, const char * info)
{
	m_pFenShiPic->ReProcMacd();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineMa(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcMAData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineMacd(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcMacdData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineBand(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcBandData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::ProcHisPointFromMsg(ReceivePointInfo * pRecvInfo,
	const char * info, SStringA dataName1, SStringA dataName2)
{
	int nOffset = 0;
	int nSize520 = pRecvInfo->FirstDataSize / sizeof(CoreData);
	int nSize2060 = (pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize) /
		sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_PointGetMap[nPeriod] = TRUE;
	vector<CoreData> PointVec(nSize520);
	memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->FirstDataSize);
	nOffset += pRecvInfo->FirstDataSize;
	auto &Point520Vec = m_PointData[nPeriod][dataName1];
	Point520Vec.insert(Point520Vec.begin(),
		PointVec.begin(), PointVec.end());
	PointVec.resize(nSize2060);
	memcpy_s(&PointVec[0], pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize);
	auto &Point2060Vec = m_PointData[nPeriod][dataName2];
	Point2060Vec.insert(Point2060Vec.begin(),
		PointVec.begin(), PointVec.end());
}
