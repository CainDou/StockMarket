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
}


CWorkWnd::~CWorkWnd()
{
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if(m_workThread.joinable())
		m_workThread.join();
}

void CWorkWnd::SetGroup(RpsGroup Group, bool bMain, HWND hParWnd)
{
	m_Group = Group;
	m_bInMain = bMain;
	m_hParWnd = hParWnd;
}

void CWorkWnd::InitShowConfig(InitPara initPara)
{
	m_InitPara = initPara;
	if (Group_Stock == m_Group)
		m_pFenShiPic->InitSubPic(MAX_SUBPIC, m_SubPicWndNameVec);
	else
		m_pFenShiPic->InitSubPic(1, m_SubPicWndNameVec);
	m_pFenShiPic->SetRpsGroup(m_Group);
	m_pFenShiPic->InitShowPara(m_InitPara);
	if (Group_Stock == m_Group)
		m_pKlinePic->InitSubPic(MAX_SUBPIC, m_SubPicWndNameVec);
	else
		m_pKlinePic->InitSubPic(1, m_SubPicWndNameVec);
	m_pKlinePic->SetRpsGroup(m_Group);
	m_pKlinePic->SetParentHwnd(m_hWnd);
	m_pKlinePic->InitShowPara(m_InitPara);
	m_PicPeriod = m_ListPeriod = m_InitPara.Period;

	SStringW btnName;
	if (Period_FenShi == m_ListPeriod)
		btnName.Format(L"btn_FS");
	else if (Period_1Day == m_ListPeriod)
		btnName.Format(L"btn_Day");
	else
		btnName.Format(L"btn_M%d", m_ListPeriod);
	m_pPreSelBtn = FindChildByName2<SImageButton>(btnName);
	m_pPreSelBtn->SetAttribute(L"colorText", L"#00ffffff");
	m_pTxtIndy = FindChildByName2<SStatic>(L"text_ShowIndy");
	m_bListConn1 = initPara.Connect1;
	m_bListConn2 = initPara.Connect2;
	if (m_bListConn1)
	{
		SImageButton * pBtn =
			FindChildByName2<SImageButton>(L"btn_ListConnect1");
		SetBtnState(pBtn, true);
	}
	if (m_bListConn2)
	{
		SImageButton * pBtn =
			FindChildByName2<SImageButton>(L"btn_ListConnect2");
		SetBtnState(pBtn, true);
	}

	if (m_bListConn1 || m_bListConn2)
	{
		m_ListShowInd = m_InitPara.ShowIndy;
		SetListShowIndyStr(m_pTxtIndy);
	}

}

InitPara SOUI::CWorkWnd::OutPutInitPara()
{
	m_pFenShiPic->OutPutShowPara(m_InitPara);
	m_pKlinePic->OutPutShowPara(m_InitPara);
	m_InitPara.Period = m_ListPeriod;
	m_InitPara.Connect1 = m_bListConn1;
	m_InitPara.Connect2 = m_bListConn2;
	strcpy_s(m_InitPara.ShowIndy, m_ListShowInd);
	return m_InitPara;
}

void SOUI::CWorkWnd::ClearData()
{
	m_ListInsVec.clear();
	m_StockVec.clear();
}

void SOUI::CWorkWnd::InitList()
{
	m_pList = FindChildByName2<SColorListCtrlEx>(L"ls_rps");
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
	UpdateList();

}

void SOUI::CWorkWnd::ReInitList()
{
	m_bListInited = false;
	UpdateListShowStock();
	m_bListInited = true;
	UpdateList();

}

void SOUI::CWorkWnd::SetDataPoint(void * pData, int DataType)
{
	switch (DataType)
	{
	case DT_ListData:
		m_pListDataMap = (map<int,TimeLineMap>*)pData;
		break;
	}
}

void SOUI::CWorkWnd::SetKbElfInfo(bool bFroceUpdate,
	vector<StockInfo>* stock1Vec, vector<StockInfo>* stock2Vec)
{
	m_pDlgKbElf->SetStockInfo(bFroceUpdate, stock1Vec, stock2Vec);
}

void SOUI::CWorkWnd::SetPicUnHandled()
{
	m_pFenShiPic->SetPicUnHandled();
	m_pKlinePic->SetPicUnHandled();
}

void SOUI::CWorkWnd::ReSetPic()
{
	if (m_strSubStock != "")
		ShowSubStockPic(m_strSubStock);
}

void SOUI::CWorkWnd::UpdateTodayPointData(SStringA pointName,
	vector<CoreData>& dataVec, bool bLast)
{
	m_PointData[m_PicPeriod][pointName] = dataVec;
	if (bLast)
		m_PointReadyMap[m_PicPeriod] = true;
}

void SOUI::CWorkWnd::OnInit(EventArgs * e)
{
	m_pFenShiPic = FindChildByName2<SFenShiPic>(L"fenshiPic");
	m_pKlinePic = FindChildByName2<SKlinePic>(L"klinePic");
	InitProcFucMap();
	m_workThread = thread(&CWorkWnd::DataProc,this);
	m_uThreadID = *(unsigned*)&m_workThread.get_id();
	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);

}

LRESULT SOUI::CWorkWnd::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case MAINMSG_UpdateList:
	{
		UpdateList();
		if (m_pList->IsVisible())
			m_pList->Invalidate();
	}
	break;
	case MAINMSG_UpdatePoint:
	{
		UpdateList();
		m_pList->Invalidate();
	}
	break;
	case MAINMSG_TodayPoint:
	{
		auto info = m_pDlgKbElf->GetShowPicInfo();
		ShowSubStockPic(info.first);
	}
	break;
	case MAINMSG_HisPoint:
	{
		int nGroup = (int)lp;
		m_pFenShiPic->Invalidate();
		m_pKlinePic->Invalidate();
	}
	break;
	default:
		break;
	}
	return 0;
}

LRESULT SOUI::CWorkWnd::OnDataMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	FSMSG msg = (FSMSG)HIWORD(lp);
	char *StockID = (char*)wp;
	if (m_strSubStock == StockID)
	{
		switch (msg)
		{
		case DM_MARKET:
		{
			m_pFenShiPic->DataProc();
			m_pKlinePic->SetTodayMarketState(true);
		}
		break;
		case DM_HISKLINE:
			m_pKlinePic->SetHisKlineState(true);
			break;
		case DM_HISPOINT:
			m_pKlinePic->SetHisPointState(true);
			break;
		default:
			break;
		}
		if (m_pKlinePic->IsVisible()
			&& m_pKlinePic->GetDataReadyState())
			m_pKlinePic->DataProc();
	}
	delete[]StockID;
	return 0;
}

LRESULT SOUI::CWorkWnd::OnFSMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	FSMSG msg = (FSMSG)HIWORD(lp);
	switch (msg)
	{
	case FSMSG_UPDATE:
		break;
	case FSMSG_EMA:
		m_pFenShiPic->SetEmaPara((int*)wp);
		m_pFenShiPic->ReProcEMA();
		m_pFenShiPic->Invalidate();
		ChangeWindowSetting("EMA", (void*)wp, true);
		break;
	case FSMSG_MACD:
		m_pFenShiPic->SetMacdPara((int*)wp);
		m_pFenShiPic->ReProcMacd();
		m_pFenShiPic->Invalidate();
		ChangeWindowSetting("MACD", (void*)wp, true);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT SOUI::CWorkWnd::OnKlineMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	KLINEMSG msg = (KLINEMSG)HIWORD(lp);
	switch (msg)
	{
	case KLINEMSG_UPDATE:
		break;
	case KLINEMSG_MA:
		m_pKlinePic->SetMaPara((int*)wp);
		m_pKlinePic->ReProcMAData();
		m_pKlinePic->Invalidate();
		ChangeWindowSetting("MA", (void*)wp, true);
		break;
	case KLINEMSG_MACD:
		m_pKlinePic->SetMacdPara((int*)wp);
		m_pKlinePic->ReProcMacdData();
		m_pKlinePic->Invalidate();
		ChangeWindowSetting("MACD", (void*)wp, true);
		break;
	case KLINEMSG_BAND:
		m_pKlinePic->SetBandPara(*(BandPara_t*)wp);
		m_pKlinePic->ReProcBandData();
		m_pKlinePic->Invalidate();
		ChangeWindowSetting("Band", (void*)wp, true);
		break;
	default:
		break;
	}
	return 0;
}

void SOUI::CWorkWnd::OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case FM_Deal:
		m_pFenShiPic->SetDealState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetDealState();
		ChangeWindowSetting(m_Group, "ShowTSCDeal", &bState);
		break;
	case FM_Volume:
		m_pFenShiPic->SetVolumeState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetVolumeState();
		ChangeWindowSetting(m_Group, "ShowTSCVolume", &bState);
		break;
	case FM_MACD:
		m_pFenShiPic->SetMacdState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetMacdState();
		ChangeWindowSetting(m_Group, "ShowTSCMACD", &bState);
		break;
	case FM_RPS:
		m_pFenShiPic->SetRpsState(SP_FULLMARKET);
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetRpsState(SP_FULLMARKET);
		ChangeWindowSetting(m_Group, "ShowTSCRPS", &bState);
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
		ChangeWindowSetting(m_Group, "ShowAvg", &bState);
		break;
	case FM_EMA:
		m_pFenShiPic->SetEmaState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetEmaState();
		ChangeWindowSetting(m_Group, "ShowEMA", &bState);
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
		ChangeWindowSetting(m_Group, "ShowTSCL1RPS", &bState);
		break;
	case FM_L2RPS:
		m_pFenShiPic->SetRpsState(SP_SWINDYL2);
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetRpsState(SP_SWINDYL2);
		ChangeWindowSetting(m_Group, "ShowTSCL2RPS", &bState);
		break;
	default:
		break;
	}
}
void SOUI::CWorkWnd::OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case KM_Deal:
		m_pKlinePic->SetDealState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetDealState();
		ChangeWindowSetting(m_Group, "ShowKlineDeal", &bState);
		break;
	case KM_MA:
		m_pKlinePic->SetBandState(false, false);
		m_pKlinePic->SetMaState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMaState();
		ChangeWindowSetting(m_Group, "ShowMA", &bState);
		break;
	case KM_Band:
		m_pKlinePic->SetMaState(false, false);
		m_pKlinePic->SetBandState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetBandState();
		ChangeWindowSetting(m_Group, "ShowBand", &bState);
		break;
	case KM_Volume:
		m_pKlinePic->SetVolumeState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetVolumeState();
		ChangeWindowSetting(m_Group, "ShowKlineVolume", &bState);
		break;
	case KM_MACD:
		m_pKlinePic->SetMacdState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMacdState();
		ChangeWindowSetting(m_Group, "ShowKlineMACD", &bState);
		break;
	case KM_RPS:
		m_pKlinePic->SetRpsState(SP_FULLMARKET);
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetRpsState(SP_FULLMARKET);
		ChangeWindowSetting(m_Group, "ShowKlineRPS", &bState);
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
		ChangeWindowSetting(m_Group, "ShowKlineL1RPS", &bState);
		break;
	case KM_L2RPS:
		m_pKlinePic->SetRpsState(SP_SWINDYL2);
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetRpsState(SP_SWINDYL2);
		ChangeWindowSetting(m_Group, "ShowKlineL2RPS", &bState);
		break;

	default:
		break;
	}
}
void SOUI::CWorkWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
void SOUI::CWorkWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
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
			m_pDlgKbElf->SetWindowPos(NULL, rc.right - 320,
				rc.bottom - 370, 0, 0, SWP_NOSIZE);
			m_pDlgKbElf->wstrInput = L"";
			SStringW input = L"";
			if (m_nLastChar >= '0' && m_nLastChar <= '9')
				input = (char)m_nLastChar;
			m_pDlgKbElf->m_pEdit->SetWindowTextW(input);

			m_pDlgKbElf->m_pEdit->SetFocus();
			m_pDlgKbElf->m_pEdit->SetSel(-1);
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
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
			m_pDlgKbElf->wstrInput = L"";
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
			m_pDlgKbElf->m_pEdit->SetWindowTextW(input);
			m_pDlgKbElf->m_pEdit->SetFocus();
			m_pDlgKbElf->m_pEdit->SetSel(-1);
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
		}

	}
}
void SOUI::CWorkWnd::OnLButtonDown(UINT nFlags, CPoint point)
{

}
void SOUI::CWorkWnd::OnRButtonUp(UINT nFlags, CPoint point)
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
BOOL SOUI::CWorkWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	int nWnd = 0;
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
		SStringA strSubIns = m_strSubStock[nWnd];
		std::vector < pair<SStringA, int>>ListPosVec(
			m_ListPosMap.begin(), m_ListPosMap.end());
		std::sort(ListPosVec.begin(), ListPosVec.end(),
			[](const pair<SStringA, int> &a, const pair<SStringA, int> b)
		{return a.second < b.second; });
		int i = 0;
		for (i; i < ListPosVec.size(); ++i)
		{
			if (strSubIns == ListPosVec[i].first)
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
		ShowSubStockPic(ListPosVec[i].first);
	}
	return 0;
}
void SOUI::CWorkWnd::SwitchPic2List()
{
	m_pFenShiPic->SetVisible(FALSE, TRUE);
	m_pKlinePic->SetVisible(FALSE, TRUE);
	m_pKlinePic->ClearTip();
	m_pList->SetVisible(TRUE, TRUE);
	UpdateList();
	m_pList->SetFocus();
	m_pList->RequestRelayout();

}
void SOUI::CWorkWnd::SwitchList2Pic(int nPeriod)
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
void SOUI::CWorkWnd::ChangeWindowSetting(SStringA strKey, void * pValue, bool bComplex)
{
	SStringA strSection;
	strSection.Format("Group%d", m_Group);
	CIniFile ini(".//config//WindowConfig.ini");
	if (!bComplex)
		ini.WriteIntA(strSection, strKey, (int)pValue);
	else
	{
		if (strKey.Find("MACD") != -1)
		{
			int * pMacd = (int *)pValue;
			for (int i = 0; i < 3; ++i)
				ini.WriteIntA(strSection,
					strKey.Format("MACDPara%d", i + 1), pMacd[i]);
		}
		else if (strKey.Find("EMA") != -1)
		{
			int * pEma = (int *)pValue;
			for (int i = 0; i < 2; ++i)
				ini.WriteIntA(strSection,
					strKey.Format("EMAPara%d", i + 1), pEma[i]);
		}
		else if (strKey.Find("MA") != -1)
		{
			int * pMa = (int *)pValue;
			for (int i = 0; i < 4; ++i)
				ini.WriteIntA(strSection,
					strKey.Format("MAPara%d", i + 1), pMa[i]);
		}
		else if (strKey.Find("Band") != -1)
		{
			BandPara_t * pBand = (BandPara_t *)pValue;
			ini.WriteIntA(strSection, "BandN1", pBand->N1);
			ini.WriteIntA(strSection, "BandN2", pBand->N2);
			ini.WriteIntA(strSection, "BandK", pBand->K);
			ini.WriteIntA(strSection, "BandM1", pBand->M1);
			ini.WriteIntA(strSection, "BandM2", pBand->M2);
			ini.WriteIntA(strSection, "BandP", pBand->P);
		}
	}
}
void SOUI::CWorkWnd::OnTimer(UINT_PTR nIDEvent)
{
}
void SOUI::CWorkWnd::DataProc()
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
			(this->*pFuc)(msgLength, info);;
		delete[]info;
		info = nullptr;
	}

}
void SOUI::CWorkWnd::InitProcFucMap()
{
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

}
bool SOUI::CWorkWnd::OnListHeaderClick(EventArgs * pEvtBase)
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
bool SOUI::CWorkWnd::OnListHeaderSwap(EventArgs * pEvtBase)
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
bool SOUI::CWorkWnd::OnListDbClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (m_ListPosMap.count(StockID) == 0)
		StockID += "I";
	m_strSubStock = StockID;

	m_PicPeriod = m_ListPeriod;
	ShowSubStockPic(StockID);
	m_bShowList = false;
	return true;
}
bool SOUI::CWorkWnd::OnListLClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	if (m_Group == Group_Stock)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (StockID[0] == '0')
		StockID += "I";
	::PostMessage(m_hParWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
	return true;
}
void SOUI::CWorkWnd::UpdateListShowStock()
{
	if (!m_bListInited)
		return;
	if (!m_pList->IsVisible())
		return;
	SStringW tmp;
	auto ListData = m_pListDataMap->at(m_ListPeriod);
	for (auto &it : m_ListPosMap)
	{
		SStringA StockID = it.first;
		if (ListData.count(StockID))
		{
			auto & dataVec = ListData[StockID];
			if (!isnan(dataVec["close"].value) && dataVec["close"].value != 0)
			{
				double fClose = dataVec["close"].value;
				double fPreClose = m_preCloseMap[StockID];
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

void SOUI::CWorkWnd::UpdateList()
{
	if (!m_bListInited)
		return;
	if (!m_pList->IsVisible())
		return;
	SStringW tmp;
	auto ListData = m_pListDataMap->at(m_ListPeriod);
	for (auto &it : m_ListPosMap)
	{
		SStringA StockID = it.first;
		if (ListData.count(StockID))
		{
			auto & dataVec = ListData[StockID];
			if (!isnan(dataVec["close"].value) && dataVec["close"].value != 0)
			{
				double fClose = dataVec["close"].value;
				double fPreClose = m_preCloseMap[StockID];
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

void SOUI::CWorkWnd::SortList(SColorListCtrlEx * pList, bool bSortCode)
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
		if (m_ListPosMap.count(stockID) == 0)
			stockID += "I";
		m_ListPosMap[stockID] = i;
	}
	m_bListInited = true;
}

int SOUI::CWorkWnd::SortDouble(void * para1, const void * para2,
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

int SOUI::CWorkWnd::SortInt(void * para1, const void * para2,
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

int SOUI::CWorkWnd::SortStr(void * para1, const void * para2,
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

void SOUI::CWorkWnd::SaveListConfig()
{
}

void SOUI::CWorkWnd::OnBtnMarketClicked()
{
	OnBtnShowTypeChange();
}

void SOUI::CWorkWnd::OnBtnFenShiClicked()
{
	OnBtnPeriedChange(Period_FenShi);
}

void SOUI::CWorkWnd::OnBtnM1Clicked()
{
	OnBtnPeriedChange(Period_1Min);
}

void SOUI::CWorkWnd::OnBtnM5Clicked()
{
	OnBtnPeriedChange(Period_5Min);

}

void SOUI::CWorkWnd::OnBtnM15Clicked()
{
	OnBtnPeriedChange(Period_15Min);

}

void SOUI::CWorkWnd::OnBtnM30Clicked()
{
	OnBtnPeriedChange(Period_30Min);

}

void SOUI::CWorkWnd::OnBtnM60Clicked()
{
	OnBtnPeriedChange(Period_60Min);

}

void SOUI::CWorkWnd::OnBtnDayClicked()
{
	OnBtnPeriedChange(Period_1Day);

}

void SOUI::CWorkWnd::OnBtnListConnect1Clicked()
{
	m_bListConn1 = !m_bListConn1;
	if (!m_bListConn1)
		m_ListShowInd = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_ListConnect1");
	SetBtnState(pBtn, m_bListConn1);
	::PostMessage(m_hWnd, WM_MAIN_MSG,
		MAINMSG_UpdatePoint, Group_Stock);
	SaveListConfig();
}

void SOUI::CWorkWnd::OnBtnListConnect2Clicked()
{
	m_bListConn2 = !m_bListConn2;
	if (!m_bListConn2)
		m_ListShowInd = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_ListConnect2");
	SetBtnState(pBtn, m_bListConn2);
	::PostMessage(m_hWnd, WM_MAIN_MSG,
		MAINMSG_UpdatePoint, Group_Stock);
}

void SOUI::CWorkWnd::SetBtnState(SImageButton * nowBtn, SImageButton ** preBtn)
{
	if (*preBtn)
		(*preBtn)->SetAttribute(L"colorText", L"#c0c0c0ff");
	nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	*preBtn = nowBtn;
}

void SOUI::CWorkWnd::SetBtnState(SImageButton * nowBtn, bool bSelected)
{
	if (bSelected)
		nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	else
		nowBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void SOUI::CWorkWnd::SetBtnState(int nPeriod, bool bSelected)
{
	SStringW strBtn;
	if (nPeriod == Period_FenShi)
		strBtn.Format(L"btnFS");
	else if (nPeriod == Period_1Day)
		strBtn.Format(L"btn_Day");
	else
		strBtn.Format(L"btn_M%d", nPeriod);
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	if (bSelected)
	{
		pBtn->SetAttribute(L"colorText", L"#00ffffff");
		m_pPreSelBtn = pBtn;
	}
	else
		pBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void SOUI::CWorkWnd::OnBtnShowTypeChange(bool bFroceList)
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
				SaveListConfig();
			}
			return;
		}
	}
	SStringW strBtn = L"btn_Market";
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	//if (pBtn == m_pPreSelBtn[Group_SWL1])
	//	return;
	if (m_bShowList)
	{
		pBtn->SetWindowTextW(L"个股");
		SetBtnState(m_ListPeriod, false);
		SetBtnState(m_PicPeriod, true);
		SetSelectedPeriod(m_ListPeriod);
	}
	else
	{
		pBtn->SetWindowTextW(L"行情");
		SetBtnState(m_PicPeriod, false);
		SetBtnState(m_ListPeriod, true);
		SwitchPic2List();
	}
	m_bShowList = !m_bShowList;
	SaveListConfig();
}

void SOUI::CWorkWnd::OnBtnPeriedChange(int nPeriod)
{
	SStringW strBtn;
	if (nPeriod == Period_FenShi)
		strBtn.Format(L"btn_FS");
	else if (nPeriod == Period_1Day)
		strBtn.Format(L"btn_Day");
	else
		strBtn.Format(L"btn_M%d", nPeriod);
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	if (pBtn == m_pPreSelBtn)
		return;
	SetBtnState(pBtn, &m_pPreSelBtn);
	//SetSelectedPeriod(nGroup, nGroup);
	if (m_bShowList)
	{
		m_ListPeriod = nPeriod;
		//SendMsg(m_RpsProcThreadID, UpdateSingleListData,
		//	(char*)&m_pList, sizeof(m_pList));
	}
	else
		SetSelectedPeriod(nPeriod);
	SaveListConfig();
}

void SOUI::CWorkWnd::SetListShowIndyStr(SStatic * pText)
{
	if (m_ListShowInd == "")
		pText->SetWindowTextW(L"当前分类:全市场");
	else
	{
		SStringW strIndy = StrA2StrW(m_StockNameMap[m_ListShowInd]);
		SStringW strText;
		pText->SetWindowTextW(strText.Format(L"当前分类:%s", strIndy));
	}
}

void SOUI::CWorkWnd::SetFenShiShowData()
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
		stockID, m_StockNameMap[stockID]);

	delete[]dataCount;
	dataCount = nullptr;

}

void SOUI::CWorkWnd::SetKlineShowData(int nPeriod, BOOL bNeedReCalc)
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
			tmpDataArr[i][j] = &m_PointData[Period_FenShi]
				[m_SubPicShowNameVec[i][j]];

			rightVec[i][j] = TRUE;
		}
	}


	m_pKlinePic->SetSubPicShowData(dataCount,
		tmpDataArr, rightVec, m_SubPicShowNameVec,
		stockID, m_StockNameMap[stockID]);

	delete[]dataCount;
	dataCount = nullptr;

	m_pKlinePic->ChangePeriod(nPeriod, bNeedReCalc);

}

void SOUI::CWorkWnd::GetBelongingIndyName(vector<SStringA>& nameVec)
{
	nameVec.resize(2);
	SStringA stockID = m_strSubStock;
	auto &info = m_ListStockInfoMap[stockID];
	nameVec[0] = m_StockNameMap[info.SWL1ID];
	nameVec[1] = m_StockNameMap[info.SWL2ID];
}

void SOUI::CWorkWnd::SetSelectedPeriod(int nPeriod)
{
	if (m_ListPeriod == nPeriod
		&&m_strSubStock != "")
	{
		SwitchList2Pic(nPeriod);
		return;
	}
	m_PicPeriod = nPeriod;
	SStringA& StockID = m_strSubStock;
	if (StockID == "")
	{
		for (auto &it : m_ListPosMap)
		{
			if (it.second == 0)
			{
				StockID = it.first;
				ShowSubStockPic(StockID);
				break;
			}
		}
	}

	BOOL bWaitData = FALSE;
	if (!m_bMarketGet)
	{
		m_pKlinePic->SetHisKlineState(false);
		DataGetInfo* pGetInfo = new DataGetInfo;
		pGetInfo->hWnd = m_hParWnd;
		strcpy_s(pGetInfo->StockID, StockID);
		pGetInfo->Group = m_Group;
		pGetInfo->Period = nPeriod;
		::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadMarket,
			(LPARAM)pGetInfo);
		bWaitData = TRUE;
	}
	else
		m_pKlinePic->SetHisKlineState(true);


	if (Period_FenShi != nPeriod)
	{

		if (m_KlineGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisKlineState(false);
			DataGetInfo* pGetInfo = new DataGetInfo;
			pGetInfo->hWnd = m_hParWnd;
			strcpy_s(pGetInfo->StockID, StockID);
			pGetInfo->Group = m_Group;
			pGetInfo->Period = nPeriod;
			::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadKline,
				(LPARAM)pGetInfo);
		}
		else
			m_pKlinePic->SetHisKlineState(true);

		if (m_PointGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisPointState(false);
			DataGetInfo* pGetInfo = new DataGetInfo;
			pGetInfo->hWnd = m_hParWnd;
			strcpy_s(pGetInfo->StockID, StockID);
			pGetInfo->Group = m_Group;
			pGetInfo->Period = nPeriod;
			::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadPoint,
				(LPARAM)pGetInfo);
		}
		else
			m_pKlinePic->SetHisPointState(true);

		SetKlineShowData(nPeriod, TRUE);
	}
	SwitchList2Pic(nPeriod);

}

void SOUI::CWorkWnd::ShowSubStockPic(SStringA StockID)
{
	BOOL bNeedMarket = TRUE;
	int nPeriod = m_bShowList ?
		m_ListPeriod : m_PicPeriod;
	m_strSubStock = StockID;
	if (!m_bMarketGet)
	{
		m_pKlinePic->SetTodayMarketState(false);
		m_pKlinePic->SetHisKlineState(false);
		DataGetInfo* pGetInfo = new DataGetInfo;
		pGetInfo->hWnd = m_hParWnd;
		strcpy_s(pGetInfo->StockID, StockID);
		pGetInfo->Group = m_Group;
		pGetInfo->Period = nPeriod;
		::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadMarket,
			(LPARAM)pGetInfo);
	}
	else
	{
		m_pKlinePic->SetTodayMarketState(true);
		bNeedMarket = FALSE;
	}


	SStringA StockName = m_StockNameMap[StockID];



	SFenShiPic *pFenShiPic = m_pFenShiPic;
	SKlinePic *pKlinePic = m_pKlinePic;


	if (StockID[0] == '8' || StockID.GetLength() == 7)
	{
		pFenShiPic->SetShowData(StockID, StockName, &m_IndexMarketVec);
		pKlinePic->SetShowData(StockID, StockName,
			&m_IndexMarketVec, &m_KlineMap);
	}
	else
	{
		pFenShiPic->SetShowData(StockID, StockName, &m_StockMarketVec);
		pKlinePic->SetShowData(StockID, StockName,
			&m_StockMarketVec, &m_KlineMap);
	}


	if (Period_FenShi != nPeriod)
	{
		if (m_KlineGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisKlineState(false);
			DataGetInfo* pGetInfo = new DataGetInfo;
			pGetInfo->hWnd = m_hParWnd;
			strcpy_s(pGetInfo->StockID, StockID);
			pGetInfo->Group = m_Group;
			pGetInfo->Period = nPeriod;
			::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadKline,
				(LPARAM)pGetInfo);
		}
		else
			m_pKlinePic->SetHisKlineState(true);

		if (m_PointGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisPointState(false);
			DataGetInfo* pGetInfo = new DataGetInfo;
			pGetInfo->hWnd = m_hParWnd;
			strcpy_s(pGetInfo->StockID, StockID);
			pGetInfo->Group = m_Group;
			pGetInfo->Period = nPeriod;
			::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_DownloadPoint,
				(LPARAM)pGetInfo);
		}
		else
			m_pKlinePic->SetHisPointState(true);
		SetKlineShowData(nPeriod, FALSE);
	}


	SetFenShiShowData();

	if (!bNeedMarket)
	{
		pFenShiPic->DataProc();
		if (Period_FenShi != nPeriod
			&&pKlinePic->GetDataReadyState())
			pKlinePic->DataProc();

	}
	SwitchList2Pic(nPeriod);
	m_bShowList = false;
	//SWindow::InvalidateRect(rc);
}

void SOUI::CWorkWnd::OnUpdateListData(int nMsgLength, const char * info)
{

}

void SOUI::CWorkWnd::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nDataCount = nMsgLength / + sizeof(pair<int,pair<char[16], CoreData>>);
	pair<int, pair<char[16], CoreData>> *dataArr =
		(pair<int, pair<char[16], CoreData>> *)info;
	for (int i = 0; i < nDataCount; ++i)
	{
		if(m_PointReadyMap[dataArr[i].first])
			CDataProc::UpdateTmData(m_PointData[dataArr[i].first]\
				[dataArr[i].second.first],
				dataArr[i].second.second);
	}

}

void SOUI::CWorkWnd::OnUpdateTodayPoint(int nMsgLength, const char * info)
{
}

void SOUI::CWorkWnd::OnUpdateHisKline(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int nSize = pRecvInfo->DataSize / sizeof(KlineType);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_KlineGetMap[nPeriod] = TRUE;
	auto &KlineVec = m_KlineMap[nPeriod];
	KlineVec.resize(nSize);
	memcpy_s(&KlineVec[0], pRecvInfo->DataSize,
		info + nOffset, pRecvInfo->DataSize);
	//char *InsID = new char[8];
	LPARAM lp = MAKELPARAM(nGroup, DM_HISKLINE);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void SOUI::CWorkWnd::OnUpdateHisPoint(int nMsgLength, const char * info)
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

	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	LPARAM lp = MAKELPARAM(nGroup, DM_HISPOINT);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}


void SOUI::CWorkWnd::OnUpdateIndexMarket(int nMsgLength, const char * info)
{
	CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
	SStringA SecurityID = pIndexData->SecurityID;
	m_IndexMarketVec.emplace_back(*pIndexData);
}

void SOUI::CWorkWnd::OnUpdateStockMarket(int nMsgLength, const char * info)
{
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	m_StockMarketVec.emplace_back(*pStockData);
}

void SOUI::CWorkWnd::OnUpdateHisIndexMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonIndexMarket);
	CommonIndexMarket * dataArr = (CommonIndexMarket *)(info + nOffset);
	m_IndexMarketVec.reserve(MAX_TICK);
	m_IndexMarketVec.resize(dataCount);
	SStringA str;
	memcpy_s(&m_IndexMarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_bMarketGet = TRUE;
	int nGroup = pRecvInfo->Group;
	LPARAM lp = MAKELPARAM(nGroup, DM_MARKET);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void SOUI::CWorkWnd::OnUpdateHisStockMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonStockMarket);
	CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
	m_StockMarketVec.reserve(MAX_TICK);
	m_StockMarketVec.resize(dataCount);
	SStringA str;
	memcpy_s(&m_StockMarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_bMarketGet = TRUE;
	int nGroup = pRecvInfo->Group;
	LPARAM lp = MAKELPARAM(nGroup, DM_MARKET);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void SOUI::CWorkWnd::OnUpdateCloseInfo(int nMsgLength, const char * info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	auto &closeMap = m_preCloseMap;
	//::EnterCriticalSection(&m_csClose);
	closeMap.clear();
	for (int i = 0; i < dataCount; ++i)
		closeMap[dataArr[i].first] = dataArr[i].second;
	//::LeaveCriticalSection(&m_csClose);
}

void SOUI::CWorkWnd::ProcHisPointFromMsg(ReceivePointInfo * pRecvInfo,
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
