// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "IniFile.h"
#include <fstream>
#include <process.h>
#include "DlgBandPara.h"
#include "DlgEmaPara.h"
#include "DlgMacdPara.h"
#include "DlgMaPara.h"
#include <helper/SMenu.h>
#include <queue>
#include "zlib.h"

using std::ifstream;
using std::ofstream;
using std::queue;

HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hLoginEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

HWND g_MainWnd;
#define SHOWDATACOUNT 2
#define TIMER_AUTOSAVE 1
#define MAX_TICK 6000
#define MAX_SUBPIC 3

uint64_t g_uTestTime;

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_bListInited = false;
	m_pPreSelBtn[Group_SWL1] = nullptr;
	m_pPreSelBtn[Group_SWL2] = nullptr;
	m_pPreSelBtn[Group_Stock] = nullptr;
	m_pList[Group_SWL1] = nullptr;
	m_pList[Group_SWL2] = nullptr;
	m_pList[Group_Stock] = nullptr;
	m_pDlgKbElf = nullptr;
	m_nLastOptWnd = 1;
	m_bListSWL2Conn = false;
	m_bListStockConn1 = false;
	m_bListStockConn2 = false;
	m_ListShowInd[Group_SWL1] = "";
	m_ListShowInd[Group_SWL2] = "";
	m_ListShowInd[Group_Stock] = "";
	m_pTxtSWL2Indy = nullptr;
	m_pTxtStockIndy = nullptr;
	todayDataBuffer = nullptr;
	todayDataSize = 0;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	g_MainWnd = m_hWnd;
	InitCommonSetting();
	InitConfig();
	InitLogFile();
	SetVectorSize();

	InitDataHandleMap();
	InitNetHandleMap();
	::InitializeCriticalSection(&m_csClose);

	tRpsCalc = thread(&CMainDlg::DataProc, this);
	m_RpsProcThreadID = *(unsigned*)&tRpsCalc.get_id();
	tDataProc = thread(&CMainDlg::DataProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	m_NetClinet.SetWndHandle(m_hWnd);
	tLogin = thread(&CMainDlg::Login, this);
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	ResetEvent(g_hLoginEvent);

	m_NetHandleFlag.clear();
	m_NetClinet.RegisterHandle(NetHandle);
	m_NetClinet.Start(m_uNetThreadID, this);
	SendInfo info;
	info.MsgType = SendType_Connect;
	strcpy(info.str, "StkMarket");
	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	WaitForSingleObject(g_hEvent, INFINITE);
	while (!m_bServerReady)
	{
		m_bServerReady = true;
		::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
			NULL, LoginMsg_WaitAndTry);
		WaitForSingleObject(g_hLoginEvent, INFINITE);
		ResetEvent(g_hLoginEvent);
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	while (!CheckInfoRecv())
	{
		m_NetHandleFlag.clear();
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	SStringW Info = L"登陆成功,开始程序初始化";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);

	InitShowConfig();
	InitList();
	InitSubPic();

	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);
	Info = L"处理当日历史数据，请等待...";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);
	WaitForSingleObject(g_hLoginEvent, INFINITE);


	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	ifile.open(L".\\config\\MainWnd.position", std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
	}
	ifile.close();

	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_HideWnd);
	SwitchToThisWindow(m_hWnd, FALSE);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CMainDlg::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case MAINMSG_UpdateList:
	{
		UpdateList();
		for (auto &it : m_pList)
			if (it->IsVisible())
				it->Invalidate();
	}
	break;
	case MAINMSG_UpdateListSingle:
	{
		int nGroup = (int)lp;
		UpdateList(nGroup);
		m_pList[nGroup]->Invalidate();
	}
	break;
	case MAINMSG_ShowPic:
	{
		auto info = m_pDlgKbElf->GetShowPicInfo();
		ShowSubStockPic(info.first, info.second);
	}
	break;
	case MAINMSG_UpdatePic:
	{
		int nGroup = (int)lp;
		m_pFenShiPic[nGroup]->Invalidate();
		m_pKlinePic[nGroup]->Invalidate();
	}
	break;
	case MAINMSG_ReInit:
		ReInit();
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnDataMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	FSMSG msg = (FSMSG)HIWORD(lp);
	char *StockID = (char*)wp;
	if (m_strSubStock[nGroup] == StockID)
	{
		switch (msg)
		{
		case DM_MARKET:
		{
			m_pFenShiPic[nGroup]->DataProc();
			m_pKlinePic[nGroup]->SetTodayMarketState(true);
		}
		break;
		case DM_HISKLINE:
			m_pKlinePic[nGroup]->SetHisKlineState(true);
			break;
		case DM_HISPOINT:
			m_pKlinePic[nGroup]->SetHisPointState(true);
			break;
		default:
			break;
		}
		if (m_pKlinePic[nGroup]->IsVisible()
			&& m_pKlinePic[nGroup]->GetDataReadyState())
			m_pKlinePic[nGroup]->DataProc();
	}
	delete[]StockID;
	return 0;
}

LRESULT CMainDlg::OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	FSMSG msg = (FSMSG)HIWORD(lp);
	SFenShiPic *pPic = m_pFenShiPic[nGroup];
	switch (msg)
	{
	case FSMSG_UPDATE:
		break;
	case FSMSG_EMA:
		pPic->SetEmaPara((int*)wp);
		pPic->ReProcEMA();
		pPic->Invalidate();
		ChangeWindowSetting(nGroup, "EMA", (void*)wp, true);
		break;
	case FSMSG_MACD:
		pPic->SetMacdPara((int*)wp);
		pPic->ReProcMacd();
		pPic->Invalidate();
		ChangeWindowSetting(nGroup, "MACD", (void*)wp, true);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	KLINEMSG msg = (KLINEMSG)HIWORD(lp);
	SKlinePic *pPic = m_pKlinePic[nGroup];
	switch (msg)
	{
	case KLINEMSG_UPDATE:
		break;
	case KLINEMSG_MA:
		pPic->SetMaPara((int*)wp);
		pPic->ReProcMAData();
		pPic->Invalidate();
		ChangeWindowSetting(nGroup, "MA", (void*)wp, true);
		break;
	case KLINEMSG_MACD:
		pPic->SetMacdPara((int*)wp);
		pPic->ReProcMacdData();
		pPic->Invalidate();
		ChangeWindowSetting(nGroup, "MACD", (void*)wp, true);
		break;
	case KLINEMSG_BAND:
		pPic->SetBandPara(*(BandPara_t*)wp);
		pPic->ReProcBandData();
		pPic->Invalidate();
		ChangeWindowSetting(nGroup, "Band", (void*)wp, true);
		break;
	default:
		break;
	}
	return 0;
}

void CMainDlg::OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	RpsGroup nowGroup = (RpsGroup)m_nLastOptWnd;
	SFenShiPic * pPic = m_pFenShiPic[nowGroup];
	bool bState = false;
	switch (nID)
	{
	case FM_Deal:
		pPic->SetDealState();
		pPic->Invalidate();
		bState = pPic->GetDealState();
		ChangeWindowSetting(nowGroup, "ShowTSCDeal", &bState);
		break;
	case FM_Volume:
		pPic->SetVolumeState();
		pPic->Invalidate();
		bState = pPic->GetVolumeState();
		ChangeWindowSetting(nowGroup, "ShowTSCVolume", &bState);
		break;
	case FM_MACD:
		pPic->SetMacdState();
		pPic->Invalidate();
		bState = pPic->GetMacdState();
		ChangeWindowSetting(nowGroup, "ShowTSCMACD", &bState);
		break;
	case FM_RPS:
		pPic->SetRpsState(SP_FULLMARKET);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_FULLMARKET);
		ChangeWindowSetting(nowGroup, "ShowTSCRPS", &bState);
		break;
	case FM_MacdPara:
	{
		CDlgMacdPara *pDlg = new CDlgMacdPara(nowGroup, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(pPic->GetMacdPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case FM_Avg:
		pPic->SetAvgState();
		pPic->Invalidate();
		bState = pPic->GetAvgState();
		ChangeWindowSetting(nowGroup, "ShowAvg", &bState);
		break;
	case FM_EMA:
		pPic->SetEmaState();
		pPic->Invalidate();
		bState = pPic->GetEmaState();
		ChangeWindowSetting(nowGroup, "ShowEMA", &bState);
		break;
	case FM_EmaPara:
	{
		CDlgEmaPara *pDlg = new CDlgEmaPara(nowGroup, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(pPic->GetEmaPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case FM_L1RPS:
		pPic->SetRpsState(SP_SWINDYL1);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_SWINDYL1);
		ChangeWindowSetting(nowGroup, "ShowTSCL1RPS", &bState);
		break;
	case FM_L2RPS:
		pPic->SetRpsState(SP_SWINDYL2);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_SWINDYL2);
		ChangeWindowSetting(nowGroup, "ShowTSCL2RPS", &bState);
		break;
	default:
		break;
	}
}

void CMainDlg::OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	RpsGroup nowGroup = (RpsGroup)m_nLastOptWnd;
	SKlinePic * pPic = m_pKlinePic[nowGroup];
	bool bState = false;
	switch (nID)
	{
	case KM_Deal:
		pPic->SetDealState();
		pPic->Invalidate();
		bState = pPic->GetDealState();
		ChangeWindowSetting(nowGroup, "ShowKlineDeal", &bState);
		break;
	case KM_MA:
		pPic->SetBandState(false, false);
		pPic->SetMaState();
		pPic->Invalidate();
		bState = pPic->GetMaState();
		ChangeWindowSetting(nowGroup, "ShowMA", &bState);
		break;
	case KM_Band:
		pPic->SetMaState(false, false);
		pPic->SetBandState();
		pPic->Invalidate();
		bState = pPic->GetBandState();
		ChangeWindowSetting(nowGroup, "ShowBand", &bState);
		break;
	case KM_Volume:
		pPic->SetVolumeState();
		pPic->Invalidate();
		bState = pPic->GetVolumeState();
		ChangeWindowSetting(nowGroup, "ShowKlineVolume", &bState);
		break;
	case KM_MACD:
		pPic->SetMacdState();
		pPic->Invalidate();
		bState = pPic->GetMacdState();
		ChangeWindowSetting(nowGroup, "ShowKlineMACD", &bState);
		break;
	case KM_RPS:
		pPic->SetRpsState(SP_FULLMARKET);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_FULLMARKET);
		ChangeWindowSetting(nowGroup, "ShowKlineRPS", &bState);
		break;
	case KM_MacdPara:
	{
		CDlgMacdPara *pDlg = new CDlgMacdPara(nowGroup, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(pPic->GetMacdPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_BandPara:
	{
		CDlgBandPara *pDlg = new CDlgBandPara(nowGroup, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(pPic->GetBandPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_MaPara:
	{
		CDlgMaPara *pDlg = new CDlgMaPara(nowGroup, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(pPic->GetMaPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_L1RPS:
		pPic->SetRpsState(SP_SWINDYL1);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_SWINDYL1);
		ChangeWindowSetting(nowGroup, "ShowKlineL1RPS", &bState);
		break;
	case KM_L2RPS:
		pPic->SetRpsState(SP_SWINDYL2);
		pPic->Invalidate();
		bState = pPic->GetRpsState(SP_SWINDYL2);
		ChangeWindowSetting(nowGroup, "ShowKlineL2RPS", &bState);
		break;

	default:
		break;
	}
}

void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

void CMainDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == VK_ESCAPE)
	{
		OnBtnShowTypeChange(m_nLastOptWnd, true);
	}
	else if (nChar == 229)
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hWnd, &rc);
			m_pDlgKbElf->SetWindowPos(NULL, rc.right - 320,
				rc.bottom - 370, 0, 0, SWP_NOSIZE);
			if (m_nLastOptWnd == Group_SWL1)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL1],
					false, &m_IndexVec, &m_SWInd1Vec);
			if (m_nLastOptWnd == Group_SWL2)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2],
					false, &m_IndexVec, &m_SWInd2Vec);
			if (m_nLastOptWnd == Group_Stock)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock],
					false, &m_StockVec);
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
			::GetWindowRect(m_hWnd, &rc);
			m_pDlgKbElf->SetWindowPos(NULL,
				rc.right - 320, rc.bottom - 370, 0, 0,
				SWP_NOSIZE);
			if (m_nLastOptWnd == Group_SWL1)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL1],
					false, &m_IndexVec, &m_SWInd1Vec);
			if (m_nLastOptWnd == Group_SWL2)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2],
					false, &m_IndexVec, &m_SWInd2Vec);
			if (m_nLastOptWnd == Group_Stock)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock],
					false, &m_StockVec);
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

void CMainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	InitWndRect();
	if (PointInWindow(point, m_RectMap[Group_SWL1]))
		m_nLastOptWnd = Group_SWL1;
	else if (PointInWindow(point, m_RectMap[Group_SWL2]))
		m_nLastOptWnd = Group_SWL2;
	else if (PointInWindow(point, m_RectMap[Group_Stock]))
		m_nLastOptWnd = Group_Stock;

}

void CMainDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	InitWndRect();
	if (PointInWindow(point, m_RectMap[Group_SWL1]))
		m_nLastOptWnd = Group_SWL1;
	else if (PointInWindow(point, m_RectMap[Group_SWL2]))
		m_nLastOptWnd = Group_SWL2;
	else if (PointInWindow(point, m_RectMap[Group_Stock]))
		m_nLastOptWnd = Group_Stock;
	if (m_pList[m_nLastOptWnd]->IsVisible())
		return;
	if (m_pFenShiPic[m_nLastOptWnd]->IsVisible())
	{
		SFenShiPic *pPic = m_pFenShiPic[m_nLastOptWnd];
		SMenu menu;
		menu.LoadMenuW(L"smenu:menu_fenshi");
		if (pPic->GetDealState())
			menu.CheckMenuItem(FM_Deal, MF_CHECKED);
		if (pPic->GetVolumeState())
			menu.CheckMenuItem(FM_Volume, MF_CHECKED);
		if (pPic->GetMacdState())
			menu.CheckMenuItem(FM_MACD, MF_CHECKED);
		if (pPic->GetRpsState(SP_FULLMARKET))
			menu.CheckMenuItem(FM_RPS, MF_CHECKED);
		if (pPic->GetAvgState())
			menu.CheckMenuItem(FM_Avg, MF_CHECKED);
		if (pPic->GetEmaState())
			menu.CheckMenuItem(FM_EMA, MF_CHECKED);
		int nSubPicNum = pPic->GetShowSubPicNum();
		for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		{
			if (pPic->GetRpsState(i))
				menu.CheckMenuItem(FM_EmaPara + i, MF_CHECKED);
		}
		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;
	}

	if (m_pKlinePic[m_nLastOptWnd]->IsVisible())
	{
		SKlinePic *pPic = m_pKlinePic[m_nLastOptWnd];
		SMenu menu;
		menu.LoadMenuW(L"smenu:menu_kline");
		if (pPic->GetDealState())
			menu.CheckMenuItem(KM_Deal, MF_CHECKED);
		if (pPic->GetVolumeState())
			menu.CheckMenuItem(KM_Volume, MF_CHECKED);
		if (pPic->GetMacdState())
			menu.CheckMenuItem(KM_MACD, MF_CHECKED);
		if (pPic->GetRpsState(SP_FULLMARKET))
			menu.CheckMenuItem(KM_RPS, MF_CHECKED);
		if (pPic->GetMaState())
			menu.CheckMenuItem(KM_MA, MF_CHECKED);
		if (pPic->GetBandState())
			menu.CheckMenuItem(KM_Band, MF_CHECKED);
		int nSubPicNum = pPic->GetShowSubPicNum();
		for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		{
			if (pPic->GetRpsState(i))
				menu.CheckMenuItem(KM_MaPara + i, MF_CHECKED);
		}

		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;

	}
}

BOOL CMainDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	InitWndRect();
	int nWnd = 0;
	ScreenToClient(&pt);
	static uint64_t tick = 0;
	uint64_t nowTick = GetTickCount64();
	if (nowTick - tick < 500)
		return FALSE;
	tick = nowTick;
	if (PointInWindow(pt, m_RectMap[Group_SWL1]))
		nWnd = Group_SWL1;
	else if (PointInWindow(pt, m_RectMap[Group_SWL2]))
		nWnd = Group_SWL2;
	else if (PointInWindow(pt, m_RectMap[Group_Stock]))
		nWnd = Group_Stock;
	if (m_pList[nWnd]->IsVisible())
		m_pList[nWnd]->SetFocus();
	else
	{
		SStringA strSubIns = m_strSubStock[nWnd];
		std::vector < pair<SStringA, int>>ListPosVec(
			m_ListPosVec[nWnd].begin(), m_ListPosVec[nWnd].end());
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
		ShowSubStockPic(ListPosVec[i].first, m_pList[nWnd]);
	}
	return 0;
}

void CMainDlg::SwitchPic2List(int nGroup)
{
	SColorListCtrlEx * pList = nullptr;
	m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
	m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
	m_pKlinePic[nGroup]->ClearTip();
	pList = m_pList[nGroup];
	pList->SetVisible(TRUE, TRUE);
	UpdateList(nGroup);
	pList->SetFocus();
	//CRect rc = m_pFenShiPic[nGroup]->GetClientRect();
	//SWindow::InvalidateRect(rc);
	pList->RequestRelayout();
}

void CMainDlg::SwitchList2Pic(int nGroup, int nPeriod)
{
	m_pList[nGroup]->SetVisible(FALSE, TRUE);
	m_pFenShiPic[nGroup]->SetVisible(FALSE, FALSE);
	m_pKlinePic[nGroup]->SetVisible(FALSE, FALSE);
	if (nPeriod == Period_FenShi)
	{
		m_pFenShiPic[nGroup]->SetVisible(TRUE, TRUE);
		m_pFenShiPic[nGroup]->SetFocus();
		m_pFenShiPic[nGroup]->RequestRelayout();

	}
	else
	{
		m_pKlinePic[nGroup]->SetVisible(TRUE, TRUE);
		m_pKlinePic[nGroup]->SetFocus();
		m_pKlinePic[nGroup]->RequestRelayout();
	}

}

void CMainDlg::ChangeWindowSetting(int nGroup, SStringA strKey,
	void* pValue, bool bComplex)
{
	SStringA strSection;
	strSection.Format("Group%d", nGroup);
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

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == TIMER_AUTOSAVE)
		SavePicConfig();
}

void CMainDlg::OnDestroy()
{

	SetMsgHandled(FALSE);
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		NULL, LoginMsg_Exit);
	tLogin.join();
	delete m_pDlgKbElf;
	m_pDlgKbElf = nullptr;
	m_NetClinet.Stop();
	SavePicConfig();
	WINDOWPLACEMENT wp = { sizeof(wp) };
	SendMsg(m_RpsProcThreadID, Msg_Exit, NULL, 0);
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	tRpsCalc.join();
	tDataProc.join();
	::GetWindowPlacement(m_hWnd, &wp);
	std::ofstream ofile;
	ofile.open(L".\\config\\MainWnd.position",
		std::ios::out | std::ios::binary);
	if (ofile.is_open())
		ofile.write((char*)&wp, sizeof(wp));
	ofile.close();
}

void CMainDlg::InitCommonSetting()
{
	m_PeriodVec.resize(7);
	m_PeriodVec[0] = 0;
	m_PeriodVec[1] = 1;
	m_PeriodVec[2] = 5;
	m_PeriodVec[3] = 15;
	m_PeriodVec[4] = 30;
	m_PeriodVec[5] = 60;
	m_PeriodVec[6] = 1440;
	m_bServerReady = true;
	for (int i = 0; i < 3; ++i)
	{
		m_PeriodArr[i] = Period_1Day;
		m_ListPeriodArr[i] = Period_1Day;
		m_bShowList[i] = true;
	}
	m_SubPicShowNameVec.resize(MAX_SUBPIC);
	for (int i = 0; i < MAX_SUBPIC; ++i)
		m_SubPicShowNameVec[i].reserve(SHOWDATACOUNT);
	m_SubPicShowNameVec[0].emplace_back("Point520");
	m_SubPicShowNameVec[0].emplace_back("Point2060");
	m_SubPicShowNameVec[1].emplace_back("L1Point520");
	m_SubPicShowNameVec[1].emplace_back("L1Point2060");
	m_SubPicShowNameVec[2].emplace_back("L2Point520");
	m_SubPicShowNameVec[2].emplace_back("L2Point2060");

	m_SubPicWndNameVec.reserve(MAX_SUBPIC);
	m_SubPicWndNameVec.emplace_back("全市场RPS");
	m_SubPicWndNameVec.emplace_back("1级行业RPS");
	m_SubPicWndNameVec.emplace_back("2级行业RPS");

}

void CMainDlg::InitConfig()
{
	CIniFile ini(".//config//config.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);

	ini.SetFileNameA(".//config//WindowConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		SStringA strSection;
		strSection.Format("Group%d", i);
		m_arrInit[i].bShowMA =
			ini.GetIntA(strSection, "ShowMA") == 0 ? false : true;
		m_arrInit[i].bShowBandTarget =
			ini.GetIntA(strSection, "ShowBand") == 0 ? false : true;
		m_arrInit[i].bShowAverage =
			ini.GetIntA(strSection, "ShowAvg") == 0 ? false : true;
		m_arrInit[i].bShowEMA =
			ini.GetIntA(strSection, "ShowEMA") == 0 ? false : true;
		m_arrInit[i].bShowTSCMACD =
			ini.GetIntA(strSection, "ShowTSCMACD") == 0 ? false : true;
		m_arrInit[i].bShowTSCVolume =
			ini.GetIntA(strSection, "ShowTSCVolume") == 0 ? false : true;
		m_arrInit[i].bShowKlineVolume =
			ini.GetIntA(strSection, "ShowKlineVolume") == 0 ? false : true;
		m_arrInit[i].bShowKlineMACD =
			ini.GetIntA(strSection, "ShowKlineMACD") == 0 ? false : true;
		m_arrInit[i].bShowTSCRPS[0] =
			ini.GetIntA(strSection, "ShowTSCRPS") == 0 ? false : true;
		m_arrInit[i].bShowTSCRPS[1] =
			ini.GetIntA(strSection, "ShowTSCL1RPS") == 0 ? false : true;
		m_arrInit[i].bShowTSCRPS[2] =
			ini.GetIntA(strSection, "ShowTSCL2RPS") == 0 ? false : true;
		m_arrInit[i].bShowKlineRPS[0] =
			ini.GetIntA(strSection, "ShowKlineRPS") == 0 ? false : true;
		m_arrInit[i].bShowKlineRPS[1] =
			ini.GetIntA(strSection, "ShowKlineL1RPS") == 0 ? false : true;
		m_arrInit[i].bShowKlineRPS[2] =
			ini.GetIntA(strSection, "ShowKlineL2RPS") == 0 ? false : true;

		m_arrInit[i].nWidth = ini.GetIntA(strSection, "Width", 16);
		m_arrInit[i].bShowTSCDeal =
			ini.GetIntA(strSection, "ShowTSCDeal") == 0 ? false : true;
		m_arrInit[i].bShowKlineDeal =
			ini.GetIntA(strSection, "ShowKlineDeal") == 0 ? false : true;
		m_arrInit[i].nEMAPara[0] =
			ini.GetIntA(strSection, "EMAPara1", 12);
		m_arrInit[i].nEMAPara[1] =
			ini.GetIntA(strSection, "EMAPara2", 26);
		m_arrInit[i].nMACDPara[0] =
			ini.GetIntA(strSection, "MACDPara1", 12);
		m_arrInit[i].nMACDPara[1] =
			ini.GetIntA(strSection, "MACDPara2", 26);
		m_arrInit[i].nMACDPara[2] =
			ini.GetIntA(strSection, "MACDPara3", 9);
		m_arrInit[i].nMAPara[0] =
			ini.GetIntA(strSection, "MAPara1", 5);
		m_arrInit[i].nMAPara[1] =
			ini.GetIntA(strSection, "MAPara2", 10);
		m_arrInit[i].nMAPara[2] =
			ini.GetIntA(strSection, "MAPara3", 20);
		m_arrInit[i].nMAPara[3] =
			ini.GetIntA(strSection, "MAPara4", 60);
		m_arrInit[i].nJiange =
			ini.GetIntA(strSection, "Jiange", 2);
		m_arrInit[i].BandPara.N1 =
			ini.GetIntA(strSection, "BandN1", 8);
		m_arrInit[i].BandPara.N2 =
			ini.GetIntA(strSection, "BandN2", 11);
		m_arrInit[i].BandPara.K =
			ini.GetIntA(strSection, "BandK", 390);
		m_arrInit[i].BandPara.M1 =
			ini.GetIntA(strSection, "BandM1", 8);
		m_arrInit[i].BandPara.M2 =
			ini.GetIntA(strSection, "BandM2", 4);
		m_arrInit[i].BandPara.P =
			ini.GetIntA(strSection, "BandP", 390);
	}


}

void CMainDlg::InitWndRect()
{
	SWindow *pWindow = FindChildByName2<SWindow>(L"wnd_Ind1");
	m_RectMap[Group_SWL1] = pWindow->GetClientRect();
	pWindow = FindChildByName2<SWindow>(L"wnd_Ind2");
	m_RectMap[Group_SWL2] = pWindow->GetClientRect();
	pWindow = FindChildByName2<SWindow>(L"wnd_Ind3");
	m_RectMap[Group_Stock] = pWindow->GetClientRect();

}

void CMainDlg::InitSubPic()
{

	m_pFenShiPic[Group_SWL1] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind1");
	m_pFenShiPic[Group_SWL2] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind2");
	m_pFenShiPic[Group_Stock] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind3");
	m_pFenShiPic[Group_SWL1]->InitSubPic(1, m_SubPicWndNameVec);
	m_pFenShiPic[Group_SWL2]->InitSubPic(1, m_SubPicWndNameVec);
	m_pFenShiPic[Group_Stock]->InitSubPic(MAX_SUBPIC, m_SubPicWndNameVec);

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pFenShiPic[i]->SetRpsGroup((RpsGroup(i)));
		m_pFenShiPic[i]->InitShowPara(m_arrInit[i]);
	}

	m_pKlinePic[Group_SWL1] = FindChildByName2<SKlinePic>(L"klinePic_Ind1");
	m_pKlinePic[Group_SWL2] = FindChildByName2<SKlinePic>(L"klinePic_Ind2");
	m_pKlinePic[Group_Stock] = FindChildByName2<SKlinePic>(L"klinePic_Ind3");
	m_pKlinePic[Group_SWL1]->InitSubPic(1, m_SubPicWndNameVec);
	m_pKlinePic[Group_SWL2]->InitSubPic(1, m_SubPicWndNameVec);
	m_pKlinePic[Group_Stock]->InitSubPic(MAX_SUBPIC, m_SubPicWndNameVec);

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pKlinePic[i]->SetRpsGroup((RpsGroup(i)));
		m_pKlinePic[i]->SetParentHwnd(m_hWnd);
		m_pKlinePic[i]->InitShowPara(m_arrInit[i]);
	}


}

void CMainDlg::InitShowConfig()
{
	CIniFile ini(".//config//WindowConfig.ini");
	m_ListPeriodArr[Group_SWL1] = ini.GetIntA("Group0", "Period", Period_1Day);
	m_ListPeriodArr[Group_SWL2] = ini.GetIntA("Group1", "Period", Period_1Day);
	m_ListPeriodArr[Group_Stock] = ini.GetIntA("Group2", "Period", Period_1Day);

	m_PeriodArr[Group_SWL1] = m_ListPeriodArr[Group_SWL1];
	m_PeriodArr[Group_SWL2] = m_ListPeriodArr[Group_SWL2];
	m_PeriodArr[Group_Stock] = m_ListPeriodArr[Group_Stock];

	SStringW btnName;
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		if (Period_FenShi == m_ListPeriodArr[i])
			btnName.Format(L"btn_Ind%dFS", i + 1);
		else if (Period_1Day == m_ListPeriodArr[i])
			btnName.Format(L"btn_Ind%dDay", i + 1);
		else
			btnName.Format(L"btn_Ind%dM%d", i + 1, m_ListPeriodArr[i]);
		m_pPreSelBtn[i] = FindChildByName2<SImageButton>(btnName);
		m_pPreSelBtn[i]->SetAttribute(L"colorText", L"#00ffffff");
	}

	m_pTxtSWL2Indy = FindChildByName2<SStatic>(L"text_List2ShowIndy");
	m_pTxtStockIndy = FindChildByName2<SStatic>(L"text_List3ShowIndy");

	m_bListSWL2Conn = ini.GetIntA("Group1", "ListConnent", 0) == 0 ? false : true;
	if (m_bListSWL2Conn)
	{
		m_ListShowInd[Group_SWL2] = ini.GetStringA("Group1", "ShowIndustry", "");
		SImageButton * pBtn =
			FindChildByName2<SImageButton>(L"btn_Ind2ListConnect");
		SetBtnState(pBtn, true);
		SetListShowIndyStr(m_pTxtSWL2Indy, Group_SWL2);
	}
	m_bListStockConn1 = ini.GetIntA("Group2", "ListConnent1", 0) == 0 ? false : true;
	if (m_bListStockConn1)
	{
		SImageButton * pBtn =
			FindChildByName2<SImageButton>(L"btn_Ind3ListConnect1");
		SetBtnState(pBtn, true);
	}
	m_bListStockConn2 = ini.GetIntA("Group2", "ListConnent2", 0) == 0 ? false : true;
	if (m_bListStockConn2)
	{
		SImageButton * pBtn =
			FindChildByName2<SImageButton>(L"btn_Ind3ListConnect2");
		SetBtnState(pBtn, true);
	}
	if (m_bListStockConn1 || m_bListStockConn2)
		m_ListShowInd[Group_Stock] = ini.GetStringA("Group2", "ShowIndustry", "");
}

void CMainDlg::SavePicConfig()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pFenShiPic[i]->OutPutShowPara(m_arrInit[i]);
		m_pKlinePic[i]->OutPutShowPara(m_arrInit[i]);
	}

	CIniFile ini(".//config//WindowConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		SStringA strSection;
		strSection.Format("Group%d", i);
		ini.WriteIntA(strSection, "ShowMA", m_arrInit[i].bShowMA);
		ini.WriteIntA(strSection, "ShowBand", m_arrInit[i].bShowBandTarget);
		ini.WriteIntA(strSection, "ShowAvg", m_arrInit[i].bShowAverage);
		ini.WriteIntA(strSection, "ShowEMA", m_arrInit[i].bShowEMA);
		ini.WriteIntA(strSection, "ShowTSCMACD", m_arrInit[i].bShowTSCMACD);
		ini.WriteIntA(strSection, "ShowTSCVolume", m_arrInit[i].bShowTSCVolume);
		ini.WriteIntA(strSection, "ShowKlineVolume", m_arrInit[i].bShowKlineVolume);
		ini.WriteIntA(strSection, "ShowKlineMACD", m_arrInit[i].bShowKlineMACD);
		ini.WriteIntA(strSection, "ShowTSCRPS", m_arrInit[i].bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowTSCL1RPS", m_arrInit[i].bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowTSCL2RPS", m_arrInit[i].bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowKlineRPS", m_arrInit[i].bShowKlineRPS[0]);
		ini.WriteIntA(strSection, "ShowKlineL1RPS", m_arrInit[i].bShowKlineRPS[1]);
		ini.WriteIntA(strSection, "ShowKlineL2RPS", m_arrInit[i].bShowKlineRPS[2]);
		ini.WriteIntA(strSection, "Width", m_arrInit[i].nWidth);
		ini.WriteIntA(strSection, "ShowTSCDeal", m_arrInit[i].bShowTSCDeal);
		ini.WriteIntA(strSection, "ShowKlineDeal", m_arrInit[i].bShowKlineDeal);
		ini.WriteIntA(strSection, "EMAPara1", m_arrInit[i].nEMAPara[0]);
		ini.WriteIntA(strSection, "EMAPara2", m_arrInit[i].nEMAPara[1]);
		ini.WriteIntA(strSection, "MACDPara1", m_arrInit[i].nMACDPara[0]);
		ini.WriteIntA(strSection, "MACDPara2", m_arrInit[i].nMACDPara[1]);
		ini.WriteIntA(strSection, "MACDPara3", m_arrInit[i].nMACDPara[2]);
		ini.WriteIntA(strSection, "MAPara1", m_arrInit[i].nMAPara[0]);
		ini.WriteIntA(strSection, "MAPara2", m_arrInit[i].nMAPara[1]);
		ini.WriteIntA(strSection, "MAPara3", m_arrInit[i].nMAPara[2]);
		ini.WriteIntA(strSection, "MAPara4", m_arrInit[i].nMAPara[3]);
		ini.WriteIntA(strSection, "Jiange", m_arrInit[i].nJiange);
		ini.WriteIntA(strSection, "BandN1", m_arrInit[i].BandPara.N1);
		ini.WriteIntA(strSection, "BandN2", m_arrInit[i].BandPara.N2);
		ini.WriteIntA(strSection, "BandK", m_arrInit[i].BandPara.K);
		ini.WriteIntA(strSection, "BandM1", m_arrInit[i].BandPara.M1);
		ini.WriteIntA(strSection, "BandM2", m_arrInit[i].BandPara.M2);
		ini.WriteIntA(strSection, "BandP", m_arrInit[i].BandPara.P);
	}

}

void CMainDlg::SaveListConfig()
{
	CIniFile ini(".//config//WindowConfig.ini");

	ini.WriteIntA("Group0", "Period", m_ListPeriodArr[Group_SWL1]);
	ini.WriteIntA("Group1", "Period", m_ListPeriodArr[Group_SWL2]);
	ini.WriteIntA("Group2", "Period", m_ListPeriodArr[Group_Stock]);

	ini.WriteIntA("Group1", "ListConnent", m_bListSWL2Conn);
	ini.WriteStringA("Group1", "ShowIndustry", m_ListShowInd[Group_SWL2]);
	ini.WriteIntA("Group2", "ListConnent1", m_bListStockConn1);
	ini.WriteIntA("Group2", "ListConnent2", m_bListStockConn2);
	ini.WriteStringA("Group2", "ShowIndustry", m_ListShowInd[Group_Stock]);
}

void CMainDlg::InitList()
{

	int i = 0;
	SStringW tmp;

	m_dataNameVec.emplace_back("close");
	m_dataNameVec.emplace_back("RPS520");
	m_dataNameVec.emplace_back("MACD520");
	m_dataNameVec.emplace_back("Point520");
	m_dataNameVec.emplace_back("Rank520");
	m_dataNameVec.emplace_back("RPS2060");
	m_dataNameVec.emplace_back("MACD2060");
	m_dataNameVec.emplace_back("Point2060");
	m_dataNameVec.emplace_back("Rank2060");

	m_comDataNameVec.emplace_back("close");
	m_comDataNameVec.emplace_back("RPS520");
	m_comDataNameVec.emplace_back("MACD520");
	m_uniDataNameVec.emplace_back("Point520");
	m_uniDataNameVec.emplace_back("Rank520");
	m_comDataNameVec.emplace_back("RPS2060");
	m_comDataNameVec.emplace_back("MACD2060");
	m_uniDataNameVec.emplace_back("Point2060");
	m_uniDataNameVec.emplace_back("Rank2060");


	m_pList[Group_SWL1] = FindChildByName2<SColorListCtrlEx>(L"List_Ind1");
	m_pList[Group_SWL2] = FindChildByName2<SColorListCtrlEx>(L"List_Ind2");
	m_pList[Group_Stock] = FindChildByName2<SColorListCtrlEx>(L"List_Ind3");

	m_pList[Group_SWL1]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK,
		Subscriber(&CMainDlg::OnListDbClick, this));
	m_pList[Group_SWL2]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK,
		Subscriber(&CMainDlg::OnListDbClick, this));
	m_pList[Group_Stock]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK,
		Subscriber(&CMainDlg::OnListDbClick, this));
	m_pList[Group_SWL1]->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
		Subscriber(&CMainDlg::OnListLClick, this));
	m_pList[Group_SWL2]->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
		Subscriber(&CMainDlg::OnListLClick, this));
	m_pList[Group_Stock]->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
		Subscriber(&CMainDlg::OnListLClick, this));


	for (auto &it : m_IndexVec)
	{
		tmp.Format(L"%d", i + 1);
		m_pList[Group_SWL1]->InsertItem(i, tmp);
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_ID,
			StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_Name,
			StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL1][it.SecurityID] = i;
		i++;
	}

	int nInd1 = i;
	for (auto &it : m_SWInd1Vec)
	{
		tmp.Format(L"%d", nInd1 + 1);
		m_pList[Group_SWL1]->InsertItem(nInd1, tmp);
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_ID,
			StrA2StrW(it.SecurityID));
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_Name,
			StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL1][it.SecurityID] = nInd1;
		m_StockNameMap[it.SecurityID] = it.SecurityName;

		nInd1++;
	}

	UpdateListShowStock(Group_SWL2);
	UpdateListShowStock(Group_Stock);

	SHeaderCtrlEx * pHeaderInd1 =
		(SHeaderCtrlEx *)m_pList[Group_SWL1]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd1->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP,
		Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd2 =
		(SHeaderCtrlEx *)m_pList[Group_SWL2]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd2->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP,
		Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd3 =
		(SHeaderCtrlEx *)m_pList[Group_Stock]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd3->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd3->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd3->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP,
		Subscriber(&CMainDlg::OnListHeaderSwap, this));


	m_bListInited = true;
	UpdateList();

}

bool CMainDlg::OnListHeaderClick(EventArgs * pEvtBase)
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
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	auto & sortPara = m_SortPara[nGroup];

	pHeader->SetItemSort(sortPara.nShowCol, ST_NULL);


	if (hditem.iOrder == 1)
	{
		sortPara.nCol = hditem.iOrder;
		sortPara.nShowCol = nShowOrder;
		sortPara.nFlag = 0;
	}
	else if (nShowOrder != sortPara.nShowCol)
	{
		sortPara.nCol = hditem.iOrder;
		sortPara.nShowCol = nShowOrder;
		sortPara.nFlag = 1;

	}
	else
		sortPara.nFlag = !sortPara.nFlag;
	if (hditem.iOrder != 1)
	{
		if (sortPara.nFlag == 0)
			pHeader->SetItemSort(sortPara.nShowCol, ST_UP);
		else
			pHeader->SetItemSort(sortPara.nShowCol, ST_DOWN);

	}
	SortList((SColorListCtrlEx*)pHeader->GetParent(), true);

	return true;
}

bool CMainDlg::OnListHeaderSwap(EventArgs * pEvtBase)
{
	EventHeaderItemSwap *pEvt = (EventHeaderItemSwap*)pEvtBase;
	SHeaderCtrlEx* pHead = (SHeaderCtrlEx*)pEvt->sender;
	int nColCount = pHead->GetItemCount();
	std::vector<int> Order(nColCount);
	SColorListCtrlEx * pList = (SColorListCtrlEx *)pHead->GetParent();
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	auto & sortPara = m_SortPara[nGroup];

	for (int i = 0; i < nColCount; ++i)
	{
		Order[i] = pHead->GetOriItemIndex(i);
		if (sortPara.nCol == i)
			sortPara.nShowCol = Order[i];
	}
	return true;
}

bool CMainDlg::OnListDbClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (m_ListPosVec[nGroup].count(StockID) == 0)
		StockID += "I";
	m_strSubStock[nGroup] = StockID;

	m_PeriodArr[nGroup] = m_ListPeriodArr[nGroup];
	ShowSubStockPic(StockID, pList);
	m_bShowList[nGroup] = false;
	return true;
}

bool CMainDlg::OnListLClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1 || nGroup == Group_Stock)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (StockID[0] == '0')
		StockID += "I";
	bool bUpdateConfig = false;
	if (nGroup == Group_SWL1)
	{
		if (m_bListSWL2Conn
			&&m_ListShowInd[Group_SWL2] != StockID)
		{
			m_ListShowInd[Group_SWL2] = StockID;
			UpdateListShowStock(Group_SWL2);
			bUpdateConfig = true;
		}
		if (m_bListStockConn1
			&&m_ListShowInd[Group_Stock] != StockID)
		{
			m_ListShowInd[Group_Stock] = StockID;
			UpdateListShowStock(Group_Stock);
			bUpdateConfig = true;
		}
	}
	else if (nGroup == Group_SWL2)
	{
		if (m_bListStockConn2
			&&m_ListShowInd[Group_Stock] != StockID)
		{
			m_ListShowInd[Group_Stock] = StockID;
			UpdateListShowStock(Group_Stock);
			bUpdateConfig = true;
		}
	}
	if (bUpdateConfig)
		SaveListConfig();
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
	return true;
}


void CMainDlg::SortList(SColorListCtrlEx* pList, bool bSortCode)
{

	int nGroup = GetGroupFromList(pList);
	if (-1 == nGroup)
		return;
	auto &sortPara = m_SortPara[nGroup];

	if (!bSortCode)
	{
		if (m_SortPara[nGroup].nCol == 0
			|| m_SortPara[nGroup].nCol == 1
			|| m_SortPara[nGroup].nCol == 2)
		{
			pList->Invalidate();
			return;
		}
	}

	int colCount = pList->GetColumnCount();
	int SHead_Time = colCount - 1;
	switch (sortPara.nCol)
	{
	case SHead_ID:
		pList->SortItems(SortInt, &sortPara);
		break;
	case SHead_Name:
		pList->SortItems(SortStr, &sortPara);
		break;
	default:
		if (sortPara.nCol == SHead_LastPx || sortPara.nCol == SHead_ChangePct)
			sortPara.nDec = 2;
		else
			sortPara.nDec = 3;
		pList->SortItems(SortDouble, &sortPara);
		break;
	}
	SStringW tmp;

	m_bListInited = false;
	for (int i = 0; i < pList->GetItemCount(); i++)
	{
		tmp.Format(L"%d", i + 1);
		pList->SetSubItemText(i, 0, tmp);
		SStringA stockID = StrW2StrA(pList->GetSubItemText(i, 1));
		if (m_ListPosVec[nGroup].count(stockID) == 0)
			stockID += "I";
		m_ListPosVec[nGroup][stockID] = i;
	}
	m_bListInited = true;
}

void CMainDlg::UpdateList()
{
	if (!m_bListInited)
		return;

	SStringW tmp;
	for (int i = Group_SWL1; i < Group_Count; ++i)
		UpdateList(i);
}

void CMainDlg::UpdateList(int nGroup)
{
	if (!m_bListInited)
		return;

	SStringW tmp;
	if (!m_pList[nGroup]->IsVisible())
		return;
	::EnterCriticalSection(&m_csClose);
	auto &ListMap = m_ListPosVec[nGroup];
	for (auto &it : ListMap)
	{
		SStringA StockID = it.first;
		auto & dataVec = m_listDataVec[nGroup][StockID];
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
			m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, tmp, cl);
			double chgPct = (fClose - fPreClose) / fPreClose * 100;
			if (!isnan(chgPct) && !isinf(chgPct) && fClose != 0)
				tmp.Format(L"%.2f", (fClose - fPreClose) / fPreClose * 100);
			else
				tmp = L"-";
			m_pList[nGroup]->SetSubItemText(it.second, SHead_ChangePct, tmp, cl);
		}
		else
		{
			if (dataVec["close"].value == 0)
				m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, L"0.00");
			else
				m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, L"-");
			for (int i = SHead_ChangePct; i < SHead_ItmeCount; ++i)
				m_pList[nGroup]->SetSubItemText(it.second, i, L"-");
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
				m_pList[nGroup]->SetSubItemText(it.second,
					i + SHead_ChangePct, tmp, cl);
			}
			else
				m_pList[nGroup]->SetSubItemText(it.second,
					i + SHead_ChangePct, L"-");
		}
	}
	SortList(m_pList[nGroup]);
	::LeaveCriticalSection(&m_csClose);

}

void CMainDlg::UpdateListShowStock(int nGroup)
{
	if (nGroup == Group_SWL1)
		return;
	SStringA strInd = m_ListShowInd[nGroup];
	vector<StockInfo> tmpInfoVec;
	if (nGroup == Group_SWL2)
	{
		tmpInfoVec = m_IndexVec;
		tmpInfoVec.insert(tmpInfoVec.end(),
			m_SWInd2Vec.begin(), m_SWInd2Vec.end());
	}
	else if (nGroup == Group_Stock)
		tmpInfoVec = m_StockVec;
	m_ListPosVec[Group_Stock].clear();
	m_pList[nGroup]->DeleteAllItems();
	int nCount = 0;
	SStringW tmp;
	for (auto &it : tmpInfoVec)
	{
		if (strInd == "" || strInd == it.ScaleID ||
			strInd == it.SWL1ID || strInd == it.SWL2ID)
		{
			tmp.Format(L"%d", nCount + 1);
			m_pList[nGroup]->InsertItem(nCount, tmp);
			SStringW strID = StrA2StrW(it.SecurityID);
			if (strID.GetLength() > 6)
				strID = strID.Left(6);
			m_pList[nGroup]->SetSubItemText(nCount, SHead_ID,
				strID);
			m_pList[nGroup]->SetSubItemText(nCount, SHead_Name,
				StrA2StrW(it.SecurityName));
			m_ListPosVec[nGroup][it.SecurityID] = nCount;
			m_StockNameMap[it.SecurityID] = it.SecurityName;
			nCount++;
		}
	}
	if (nGroup == Group_SWL2)
		SetListShowIndyStr(m_pTxtSWL2Indy, Group_SWL2);
	else if (nGroup == Group_Stock)
		SetListShowIndyStr(m_pTxtStockIndy, Group_Stock);

	UpdateList(nGroup);
	m_pList[nGroup]->UpdateLayout();
	//m_pList[nGroup]->RequestRelayout();
}

int CMainDlg::SortDouble(void * para1, const void * para2, const void * para3)
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

int CMainDlg::SortInt(void * para1, const void * para2, const void * para3)
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

int CMainDlg::SortStr(void * para1, const void * para2, const void * para3)
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

bool CMainDlg::ReceiveData(SOCKET socket, int size, char end,
	char * buffer, int offset)
{
	char*p = buffer + offset;
	StockInfo stkInfo = { 0 };
	int sizeLeft = size;
	while (sizeLeft > 0)
	{
		int ret = recv(socket, p, sizeLeft, 0);
		if (SOCKET_ERROR == ret)
		{
			delete[] buffer;
			buffer = nullptr;
			p = nullptr;
			return 0;
		}
		sizeLeft -= ret;
		p += ret;
	}
	p = nullptr;
	char cEnd;
	int ret = recv(socket, &cEnd, 1, 0);
	if (cEnd == end)
		return true;
	return false;
}

unsigned CMainDlg::NetHandle(void * para)
{
	CMainDlg *pMd = (CMainDlg*)para;
	UINT rpsThreadID = pMd->m_RpsProcThreadID;
	UINT dataThreadID = pMd->m_DataThreadID;

	int nOffset = 0;
	ReceiveInfo recvInfo;
	BOOL bNeedConnect = false;
	//int c = 0;
	while (true)
	{
		if (pMd->m_NetClinet.GetExitState())
			return 0;
		if (pMd->RecvInfoHandle(bNeedConnect, nOffset, recvInfo))
		{
			auto pFuc = pMd->m_netHandleMap[recvInfo.MsgType];
			if (pFuc == nullptr)
				pFuc = &CMainDlg::OnNoDefineMsg;
			(pMd->*pFuc)(pMd->m_NetClinet.GetSocket(), recvInfo);
		}

	}
	return 0;
}

void CMainDlg::Login()
{
	m_pLoginDlg = new CDlgLogin(m_hWnd, &m_NetClinet);
	m_pLoginDlg->SetIPInfo(m_strIPAddr, m_nIPPort);
	m_pLoginDlg->DoModal();
}


void CMainDlg::DataProc()
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

void CMainDlg::CalcHisData(const TimeLineData *dataArr, int nPeriod, int dataCount)
{
	auto & periodDataMap = m_commonDataMap[nPeriod];

	int tick = GetTickCount();
	SStringA nowStockID = "";
	vector<CoreData> * pDataVec = nullptr;

	for (int i = 0; i < dataCount; ++i)
	{
		if (nowStockID != dataArr[i].securityID)
		{
			nowStockID = dataArr[i].securityID;
			pDataVec = &m_commonDataMap[nPeriod][nowStockID]["close"];
		}
		if (dataArr[i].data.time == 925)
		{
			if (Period_FenShi == nPeriod)
				m_dataHandler.SetPeriodFenshiOpenEMAData(periodDataMap,
					dataArr[i]);
		}
		else
			m_dataHandler.UpdateClose(*pDataVec, dataArr[i], nPeriod);
		//pDataVec->emplace_back(dataArr[i]);
	}

	m_dataHandler.CalcHisRps(periodDataMap);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_SWL1][nPeriod], m_ListInsVec[Group_SWL1],
		m_ListStockInfoMap[Group_SWL1]);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_SWL2][nPeriod], m_ListInsVec[Group_SWL2],
		m_ListStockInfoMap[Group_SWL2]);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_Stock][nPeriod], m_ListInsVec[Group_Stock],
		m_ListStockInfoMap[Group_Stock]);
}


bool CMainDlg::PointInWindow(const CPoint& pt, const CRect& rc)
{
	if (pt.x >= rc.left && pt.x <= rc.right
		&&pt.y >= rc.top&&pt.y <= rc.bottom)
		return true;
	return false;
}
void CMainDlg::ShowSubStockPic(SStringA stockID, SColorListCtrlEx * pList)
{
	int nGroup = GetGroupFromList(pList);
	if (-1 == nGroup)
		return;

	BOOL bNeedMarket = TRUE;
	SStringA ShowStockID = stockID;
	m_strSubStock[nGroup] = ShowStockID;
	if (m_MarketGetMap.count(ShowStockID) == 0)
	{
		m_pKlinePic[nGroup]->SetTodayMarketState(false);
		GetMarket(ShowStockID, nGroup);
	}
	else
	{
		m_pKlinePic[nGroup]->SetTodayMarketState(true);
		bNeedMarket = FALSE;
	}


	SStringA StockName = m_StockNameMap[ShowStockID];



	SFenShiPic *pFenShiPic = m_pFenShiPic[nGroup];
	SKlinePic *pKlinePic = m_pKlinePic[nGroup];

	int nPeriod = m_bShowList[nGroup] ?
		m_ListPeriodArr[nGroup] : m_PeriodArr[nGroup];

	if (ShowStockID[0] == '8' || ShowStockID.GetLength() == 7)
	{
		pFenShiPic->SetShowData(ShowStockID, StockName, &m_IdxMarketMap[ShowStockID]);
		pKlinePic->SetShowData(ShowStockID, StockName,
			&m_IdxMarketMap[ShowStockID], &m_KlineMap[ShowStockID]);
	}
	else
	{
		pFenShiPic->SetShowData(ShowStockID, StockName, &m_StkMarketMap[ShowStockID]);
		pKlinePic->SetShowData(ShowStockID, StockName,
			&m_StkMarketMap[ShowStockID], &m_KlineMap[ShowStockID]);
	}


	if (Period_FenShi != nPeriod)
	{
		if (m_KlineGetMap[stockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisKlineState(false);
			GetHisKline(stockID, nPeriod, nGroup);
		}
		else
			m_pKlinePic[nGroup]->SetHisKlineState(true);

		if (m_PointGetVec[nGroup][stockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisPointState(false);
			GetHisPoint(stockID, nPeriod, nGroup);
		}
		else
			m_pKlinePic[nGroup]->SetHisPointState(true);
		SetKlineShowData(nGroup, nPeriod, FALSE);
	}


	SetFenShiShowData(nGroup);

	if (!bNeedMarket)
	{
		pFenShiPic->DataProc();
		if (Period_FenShi != nPeriod
			&&pKlinePic->GetDataReadyState())
			pKlinePic->DataProc();

	}
	SwitchList2Pic(nGroup, nPeriod);
	m_bShowList[nGroup] = false;
	//SWindow::InvalidateRect(rc);
}


bool CMainDlg::GetHisPoint(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_GetHisPoint;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CMainDlg::GetMarket(SStringA stockID, int nGroup)
{
	SendInfo info = { 0 };
	strcpy_s(info.str, stockID);
	info.Group = nGroup;
	//发送订阅数据
	info.MsgType = SendType_SubIns;
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	//发送获取数据请求
	if (nGroup != Group_Stock)
		info.MsgType = SendType_IndexMarket;
	else
		info.MsgType = SendType_StockMarket;
	ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	g_uTestTime = GetTickCount64();
	return ret > 0;
}

bool CMainDlg::GetHisKline(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_HisPeriodKline;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

int CMainDlg::GetGroupFromList(SColorListCtrlEx * pList)
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		if (m_pList[i] == pList)
			return i;
	}
	return -1;
}

void CMainDlg::InitDataHandleMap()
{
	m_dataHandleMap[UpdateData] =
		&CMainDlg::OnTimeLineUpdate;
	m_dataHandleMap[UpdateTodayData] =
		&CMainDlg::OnTodayTimeLineProc;
	m_dataHandleMap[UpdateSingleListData] =
		&CMainDlg::OnUpdateSingleListData;
	m_dataHandleMap[UpdateHisPoint] =
		&CMainDlg::OnUpdateHisPointData;
	m_dataHandleMap[UpdateLastDayEma] =
		&CMainDlg::OnUpdateLastDayEma;
	m_dataHandleMap[UpdateIndexMarket] =
		&CMainDlg::OnUpdateIndexMarket;
	m_dataHandleMap[UpdateStockMarket] =
		&CMainDlg::OnUpdateStockMarket;
	m_dataHandleMap[UpdateHisIndexMarket] =
		&CMainDlg::OnUpdateHisIndexMarket;
	m_dataHandleMap[UpdateHisStockMarket] =
		&CMainDlg::OnUpdateHisStockMarket;
	m_dataHandleMap[UpdateHisKline] =
		&CMainDlg::OnUpdateHisKline;
	m_dataHandleMap[UpdateCloseInfo] =
		&CMainDlg::OnUpdateCloseInfo;

}

void CMainDlg::InitNetHandleMap()
{
	m_netHandleMap[RecvMsg_ClientID]
		= &CMainDlg::OnMsgClientID;
	m_netHandleMap[RecvMsg_StockInfo]
		= &CMainDlg::OnMsgStockInfo;
	m_netHandleMap[RecvMsg_RTTimeLine]
		= &CMainDlg::OnMsgRTTimeLine;
	m_netHandleMap[RecvMsg_TodayTimeLine]
		= &CMainDlg::OnMsgTodayTimeLine;
	m_netHandleMap[RecvMsg_HisPoint]
		= &CMainDlg::OnMsgHisPoint;
	m_netHandleMap[RecvMsg_LastDayEma]
		= &CMainDlg::OnMsgLastDayEma;
	m_netHandleMap[RecvMsg_RTIndexMarket]
		= &CMainDlg::OnMsgRTIndexMarket;
	m_netHandleMap[RecvMsg_RTStockMarket]
		= &CMainDlg::OnMsgRTStockMarket;
	m_netHandleMap[RecvMsg_HisIndexMarket]
		= &CMainDlg::OnMsgHisIndexMarket;
	m_netHandleMap[RecvMsg_HisStockMarket]
		= &CMainDlg::OnMsgHisStockMarket;
	m_netHandleMap[RecvMsg_HisKline]
		= &CMainDlg::OnMsgHisKline;
	m_netHandleMap[RecvMsg_CloseInfo]
		= &CMainDlg::OnMsgCloseInfo;
	m_netHandleMap[RecvMsg_Wait]
		= &CMainDlg::OnMsgWait;
	m_netHandleMap[RecvMsg_Reinit]
		= &CMainDlg::OnMsgReInit;

}

void CMainDlg::SetFenShiShowData(int nGroup)
{
	SStringA stockID = m_strSubStock[nGroup];
	int nShowNum = m_pFenShiPic[nGroup]->GetShowSubPicNum();
	vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
	vector<vector<BOOL>> rightVec(nShowNum);
	if (Group_Stock == nGroup)
	{
		vector<SStringA> nameVec;
		GetBelongingIndyName(nameVec);
		m_pFenShiPic[nGroup]->SetBelongingIndy(nameVec);
	}
	int *dataCount = new int[nShowNum];
	for (int i = 0; i < nShowNum; ++i)
	{
		tmpDataArr[i].resize(SHOWDATACOUNT);
		rightVec[i].resize(SHOWDATACOUNT);
		dataCount[i] = SHOWDATACOUNT;
		for (int j = 0; j < SHOWDATACOUNT; ++j)
		{
			tmpDataArr[i][j] = &m_dataVec[nGroup][Period_FenShi]\
				[stockID][m_SubPicShowNameVec[i][j]];
			rightVec[i][j] = TRUE;
		}
	}


	m_pFenShiPic[nGroup]->SetSubPicShowData(dataCount,
		tmpDataArr, rightVec, m_SubPicShowNameVec,
		stockID, m_StockNameMap[stockID]);

	delete[]dataCount;
	dataCount = nullptr;
}

void CMainDlg::SetKlineShowData(int nGroup, int nPeriod, BOOL bNeedReCalc)
{
	SStringA stockID = m_strSubStock[nGroup];
	int nShowNum = m_pKlinePic[nGroup]->GetShowSubPicNum();
	vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
	vector<vector<BOOL>> rightVec(nShowNum);
	if (Group_Stock == nGroup)
	{
		vector<SStringA> nameVec;
		GetBelongingIndyName(nameVec);
		m_pKlinePic[nGroup]->SetBelongingIndy(nameVec);
	}
	int *dataCount = new int[nShowNum];
	for (int i = 0; i < nShowNum; ++i)
	{
		tmpDataArr[i].resize(SHOWDATACOUNT);
		rightVec[i].resize(SHOWDATACOUNT);
		dataCount[i] = SHOWDATACOUNT;
		for (int j = 0; j < SHOWDATACOUNT; ++j)
		{
			tmpDataArr[i][j] = &m_dataVec[nGroup][nPeriod]\
				[stockID][m_SubPicShowNameVec[i][j]];
			rightVec[i][j] = TRUE;
		}
	}


	m_pKlinePic[nGroup]->SetSubPicShowData(dataCount,
		tmpDataArr, rightVec, m_SubPicShowNameVec,
		stockID, m_StockNameMap[stockID]);

	delete[]dataCount;
	dataCount = nullptr;

	m_pKlinePic[nGroup]->ChangePeriod(nPeriod, bNeedReCalc);

}

void CMainDlg::ProcHisPointFromMsg(ReceivePointInfo * pRecvInfo,
	const char* info, SStringA dataName1, SStringA dataName2)
{
	int nOffset = 0;
	int nSize520 = pRecvInfo->FirstDataSize / sizeof(CoreData);
	int nSize2060 = (pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize) /
		sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_PointGetVec[nGroup][pRecvInfo->InsID][nPeriod] = TRUE;
	vector<CoreData> PointVec(nSize520);
	memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->FirstDataSize);
	nOffset += pRecvInfo->FirstDataSize;
	auto &Point520Vec = m_dataVec[nGroup][nPeriod]\
		[pRecvInfo->InsID][dataName1];
	Point520Vec.insert(Point520Vec.begin(),
		PointVec.begin(), PointVec.end());
	PointVec.resize(nSize2060);
	memcpy_s(&PointVec[0], pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize);
	auto &Point2060Vec = m_dataVec[nGroup][nPeriod]\
		[pRecvInfo->InsID][dataName2];
	Point2060Vec.insert(Point2060Vec.begin(),
		PointVec.begin(), PointVec.end());
}

void CMainDlg::GetBelongingIndyName(vector<SStringA>& nameVec)
{
	nameVec.resize(2);
	SStringA stockID = m_strSubStock[Group_Stock];
	auto &info = m_ListStockInfoMap[Group_Stock][stockID];
	nameVec[0] = m_StockNameMap[info.SWL1ID];
	nameVec[1] = m_StockNameMap[info.SWL2ID];
}

bool CMainDlg::CheckInfoRecv()
{
	BOOL bReady =
		m_NetHandleFlag[RecvMsg_ClientID] &
		m_NetHandleFlag[RecvMsg_TodayTimeLine] &
		m_NetHandleFlag[RecvMsg_LastDayEma] &
		m_NetHandleFlag[RecvMsg_TodayTimeLine];
	if (bReady == TRUE)
	{
		if (0x01010101 == m_NetHandleFlag[RecvMsg_StockInfo])
		{
			SendMsg(m_RpsProcThreadID, UpdateTodayData,
				todayDataBuffer, todayDataSize);
			TraceLog("所有信息接收完毕");
			delete[]todayDataBuffer;
			todayDataBuffer = nullptr;
			todayDataSize = 0;
			return TRUE;
		}
	}
	TraceLog("有信息接收错误，重新进行登陆请求");
	return FALSE;

}

void CMainDlg::SetPointDataCapacity()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto &GroupStock = m_ListStockInfoMap[i];
		auto &GroupData = m_dataVec[i];
		for (auto &it : GroupStock)
		{
			for (auto &period : m_PeriodVec)
			{
				auto &data = GroupData[period][it.first];
				int nCapatical = period == Period_FenShi ? 240 :
					(period <= Period_60Min ? 240 / period : 1);
				data["Point520"].reserve(nCapatical);
				data["Point2060"].reserve(nCapatical);
				if (Group_Stock == i)
				{
					data["L1Point520"].reserve(nCapatical);
					data["L1Point2060"].reserve(nCapatical);
					data["L2Point520"].reserve(nCapatical);
					data["L2Point2060"].reserve(nCapatical);
				}

			}
		}

	}
}

bool CMainDlg::RecvInfoHandle(BOOL & bNeedConnect, int &nOffset, ReceiveInfo &recvInfo)
{
	if (bNeedConnect)
	{
		if (m_NetClinet.GetExitState())
			return 0;
		if (m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
		{
			SendIDInfo_t info = { 0 };
			info.MsgType = SendType_ReConnect;
			info.ClinetID = m_NetClinet.GetClientID();
			::send(m_NetClinet.GetSocket(), (char*)&info,
				sizeof(info), 0);
			bNeedConnect = false;
		}
		else
			return false;
	}

	int ret = recv(m_NetClinet.GetSocket(),
		(char*)&recvInfo + nOffset,
		sizeof(recvInfo) - nOffset, 0);
	if (ret == 0)
	{
		nOffset = 0;
		m_NetClinet.OnConnect(NULL, NULL);
		bNeedConnect = true;
		TraceLog("与服务器断开连接");
		return false;
	}

	if (SOCKET_ERROR == ret)
	{
		//nOffset = 0;
		if (m_NetClinet.GetExitState())
			return false;
		int nError = WSAGetLastError();
		if (nError == WSAECONNRESET)
		{
			m_NetClinet.OnConnect(NULL, NULL);
			bNeedConnect = true;
		}
		return false;
	}

	if (ret + nOffset < sizeof(recvInfo))
	{
		nOffset += ret;
		return false;
	}
	nOffset = 0;

	return true;
}

void CMainDlg::OnMsgClientID(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_NetClinet.SetClientID(((ReceiveIDInfo)recvInfo).ClientID);
	m_NetHandleFlag[RecvMsg_ClientID] = TRUE;
	TraceLog("接收客户端ID成功");
}

void CMainDlg::OnMsgStockInfo(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	StockInfo stkInfo = { 0 };
	const ReceiveStockInfo &info = recvInfo;
	Sleep(10);
	if (StockInfo_Stock == info.InfoType)
		TraceLog("开始接收股票");
	else if (StockInfo_SWL1 == info.InfoType)
		TraceLog("开始接收申万1级行业信息");
	else if (StockInfo_SWL2 == info.InfoType)
		TraceLog("开始接收申万2级行业信息");
	else if (StockInfo_Index == info.InfoType)
		TraceLog("开始接收重要指数信息");

	if (ReceiveData(netSocket, info.DataSize, '#', buffer))
	{
		int size = info.DataSize / sizeof(stkInfo);
		switch (info.InfoType)
		{
		case StockInfo_Stock:
			m_StockVec.resize(size);
			memcpy_s(&m_StockVec[0], info.DataSize,
				buffer, info.DataSize);
			for (auto &it : m_StockVec)
			{
				m_ListInsVec[Group_Stock].emplace_back(it.SecurityID);
				m_StockNameMap[it.SecurityID] = it.SecurityName;
				m_ListStockInfoMap[Group_Stock][it.SecurityID] = it;
			}
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[0] = 1;
			TraceLog("接收股票信息成功");
			break;
		case StockInfo_SWL1:
			m_SWInd1Vec.resize(size);
			memcpy_s(&m_SWInd1Vec[0], info.DataSize,
				buffer, info.DataSize);
			for (auto &it : m_SWInd1Vec)
			{
				m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
				m_StockNameMap[it.SecurityID] = it.SecurityName;
			}
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[1] = 1;
			TraceLog("接收申万1级行业信息成功");
			break;
		case StockInfo_SWL2:
			m_SWInd2Vec.resize(size);
			memcpy_s(&m_SWInd2Vec[0], info.DataSize,
				buffer, info.DataSize);
			for (auto &it : m_SWInd2Vec)
			{
				m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
				m_StockNameMap[it.SecurityID] = it.SecurityName;
			}
			TraceLog("接收申万2级行业信息成功");
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[2] = 1;
			break;
		case StockInfo_Index:
			m_IndexVec.resize(size);
			memcpy_s(&m_IndexVec[0], info.DataSize,
				buffer, info.DataSize);
			for (auto &it : m_IndexVec)
			{
				m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
				m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
				m_StockNameMap[it.SecurityID] = it.SecurityName;
			}
			TraceLog("接收重要指数信息成功");
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[3] = 1;
			break;
		default:
			break;
		}
	}
	else
		TraceLog("接收信息失败");
	delete[] buffer;
	buffer = nullptr;

}

void CMainDlg::OnMsgRTTimeLine(SOCKET netSocket, ReceiveInfo & recvInfo) {
	char *buffer = new char[recvInfo.DataSize];
	TimeLineData stkInfo = { 0 };
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_RpsProcThreadID, UpdateData,
			buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgTodayTimeLine(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_NetHandleFlag[RecvMsg_TodayTimeLine] = FALSE;
	TraceLog("开始接收当日历史数据");
	if (todayDataBuffer)
	{
		delete[]todayDataBuffer;
		todayDataBuffer = nullptr;
	}
	todayDataSize = recvInfo.DataSize + sizeof(recvInfo.SrcDataSize);
	todayDataBuffer = new char[todayDataSize];
	memcpy_s(todayDataBuffer, todayDataSize,
		&recvInfo.SrcDataSize, sizeof(recvInfo.SrcDataSize));
	if (ReceiveData(netSocket, recvInfo.DataSize, '#',
		todayDataBuffer + sizeof(recvInfo.SrcDataSize)))
	{
		m_NetHandleFlag[RecvMsg_TodayTimeLine] = TRUE;
		TraceLog("接收当日历史数据成功");
	}
	else
		TraceLog("接收当日历史数据失败");
	SetEvent(g_hEvent);
}

void CMainDlg::OnMsgHisPoint(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_DataThreadID, UpdateHisPoint, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgLastDayEma(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	TraceLog("开始接收最后的EMA数据");
	m_NetHandleFlag[RecvMsg_LastDayEma] = FALSE;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		SendMsg(m_DataThreadID, UpdateLastDayEma,
			buffer, recvInfo.DataSize);
		m_NetHandleFlag[RecvMsg_LastDayEma] = TRUE;
		TraceLog("接收最后的EMA数据成功");
	}
	else
		TraceLog("接收最后的EMA数据失败");
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgRTIndexMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_DataThreadID, UpdateIndexMarket, buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgRTStockMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_DataThreadID, UpdateStockMarket, buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgHisIndexMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_DataThreadID, UpdateHisIndexMarket, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgHisStockMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_DataThreadID, UpdateHisStockMarket, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgHisKline(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_DataThreadID, UpdateHisKline, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnMsgCloseInfo(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	TraceLog("开始接收昨日收盘数据");
	m_NetHandleFlag[RecvMsg_TodayTimeLine] = FALSE;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		SendMsg(m_DataThreadID, UpdateCloseInfo,
			buffer, recvInfo.DataSize);
		m_NetHandleFlag[RecvMsg_TodayTimeLine] = TRUE;
		TraceLog("接收昨日收盘数据成功");
	}
	else
		TraceLog("接收昨日收盘数据失败");

	delete[]buffer;
	buffer = nullptr;

}

void CMainDlg::OnMsgWait(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_bServerReady = false;
	SetEvent(g_hEvent);
}

void CMainDlg::OnMsgReInit(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_ReInit, NULL);
}

void CMainDlg::OnNoDefineMsg(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	ReceiveData(netSocket, recvInfo.DataSize, '#', buffer);
	delete[]buffer;
	buffer = nullptr;
}

void CMainDlg::OnTimeLineUpdate(int nMsgLength, const char * info) {
	if (!m_bTodayInit)
		return;
	int dataCount = nMsgLength / sizeof(TimeLineData);
	TimeLineData *dataArr = (TimeLineData *)info;

	for (int i = 0; i < m_PeriodVec.size(); ++i)
	{
		int Period = m_PeriodVec[i];
		auto & periodDataMap = m_commonDataMap[Period];
		for (int j = 0; j < dataCount; ++j)
		{
			if (dataArr[j].data.time == 925)
			{
				if (i == 0)
					m_dataHandler.SetPeriodFenshiOpenEMAData(
						periodDataMap, dataArr[j]);
			}
			else
				m_dataHandler.UpdateClose(
					periodDataMap, dataArr[j], m_PeriodVec[i]);
		}
		m_dataHandler.CalcRps(periodDataMap);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_SWL1][Period], m_ListInsVec[Group_SWL1],
			m_ListStockInfoMap[Group_SWL1]);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_SWL2][Period], m_ListInsVec[Group_SWL2],
			m_ListStockInfoMap[Group_SWL2]);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_Stock][Period], m_ListInsVec[Group_Stock],
			m_ListStockInfoMap[Group_Stock]);
	}
	int ShowPeriod = m_ListPeriodArr[Group_SWL1];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_SWL1][ShowPeriod], m_listDataVec[Group_SWL1],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL1]);
	ShowPeriod = m_ListPeriodArr[Group_SWL2];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_SWL2][ShowPeriod], m_listDataVec[Group_SWL2],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL2]);
	ShowPeriod = m_ListPeriodArr[Group_Stock];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_Stock][ShowPeriod], m_listDataVec[Group_Stock],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_Stock]);

	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
}

void CMainDlg::OnTodayTimeLineProc(int nMsgLength, const char * info) {
	//typeof()
	unsigned long nSize = *(unsigned long*)info;
	unsigned char* buffer = new unsigned char[nSize];
	uncompress(buffer, &nSize,
		(unsigned char*)(info + 4), nMsgLength - 4);
	int dataCount = nSize / sizeof(TimeLineData);
	TimeLineData *dataArr = (TimeLineData *)buffer;
	//WaitForSingleObject(g_hEvent, INFINITE);
	SetPointDataCapacity();
	vector<thread> procThreadVec(m_PeriodVec.size());
	for (int i = 0; i < m_PeriodVec.size(); ++i)
	{
		int Period = m_PeriodVec[i];
		thread t(&CMainDlg::CalcHisData, this, dataArr,
			Period, dataCount);
		procThreadVec[i].swap(t);
		//auto & periodDataMap = m_commonDataMap[Period];

		//int tick = GetTickCount();
		//SStringA nowStockID = "";
		//vector<CoreData> * pDataVec = nullptr;

		//for (int j = 0; j < dataCount; ++j)
		//{
		//	if (nowStockID != dataArr[j].securityID)
		//	{
		//		nowStockID = dataArr[j].securityID;
		//		pDataVec = &m_commonDataMap[Period][nowStockID]["close"];
		//	}
		//	if (dataArr[j].data.time == 925)
		//	{
		//		if (i == 0)
		//			m_dataHandler.SetPeriodFenshiOpenEMAData(periodDataMap,
		//				dataArr[j]);
		//	}
		//	else
		//		m_dataHandler.UpdateClose(*pDataVec, dataArr[j], Period);
		//	//pDataVec->emplace_back(dataArr[i]);
		//}

		//m_dataHandler.CalcHisRps(periodDataMap);
		//m_dataHandler.RankPointHisData(periodDataMap,
		//	m_dataVec[Group_SWL1][Period], m_ListInsVec[Group_SWL1],
		//	m_ListStockInfoMap[Group_SWL1]);
		//m_dataHandler.RankPointHisData(periodDataMap,
		//	m_dataVec[Group_SWL2][Period], m_ListInsVec[Group_SWL2],
		//	m_ListStockInfoMap[Group_SWL2]);
		//m_dataHandler.RankPointHisData(periodDataMap,
		//	m_dataVec[Group_Stock][Period], m_ListInsVec[Group_Stock],
		//	m_ListStockInfoMap[Group_Stock]);
	}
	for (auto &it : procThreadVec)
		if (it.joinable())
			it.join();
	delete[]buffer;
	buffer = nullptr;
	int ShowPeriod = m_ListPeriodArr[Group_SWL1];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_SWL1][ShowPeriod], m_listDataVec[Group_SWL1],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL1]);
	ShowPeriod = m_ListPeriodArr[Group_SWL2];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_SWL2][ShowPeriod], m_listDataVec[Group_SWL2],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL2]);
	ShowPeriod = m_ListPeriodArr[Group_Stock];
	m_dataHandler.UpdateShowData(m_commonDataMap[ShowPeriod],
		m_dataVec[Group_Stock][ShowPeriod], m_listDataVec[Group_Stock],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_Stock]);

	m_bTodayInit = true;
	SetEvent(g_hLoginEvent);
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
	for (auto &it : m_commonDataMap)
	{
		for (auto &data : it.second)
		{
			data.second["RPS520"].clear();
			data.second["RPS520"].shrink_to_fit();
			data.second["RPS2060"].clear();
			data.second["RPS2060"].shrink_to_fit();
		}
	}

}

void CMainDlg::OnUpdateSingleListData(int nMsgLength, const char* info)
{
	SColorListCtrlEx ** ppList = (SColorListCtrlEx **)info;
	int nGroup = GetGroupFromList(*ppList);
	if (-1 == nGroup)
		return;
	int period = m_ListPeriodArr[nGroup];
	m_dataHandler.UpdateShowData(m_commonDataMap[period],
		m_dataVec[nGroup][period],
		m_listDataVec[nGroup],
		m_comDataNameVec,
		m_uniDataNameVec,
		m_ListInsVec[nGroup]);
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateListSingle, nGroup);
}

void CMainDlg::OnUpdateHisPointData(int nMsgLength, const char* info) {
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

void CMainDlg::OnUpdateLastDayEma(int nMsgLength, const char* info) {
	int dataCount = nMsgLength / sizeof(TimeLineData);
	TimeLineData* dataArr = (TimeLineData*)info;
	int preDataCount = dataCount / 6;
	int offset = 0;
	for (int i = 1; i < 7; ++i)	//分时图数据不需要设置
	{
		int nPeriod = m_PeriodVec[i];
		auto &comDataMap = m_commonDataMap[nPeriod];
		for (int j = 0; j < preDataCount; ++j)
		{
			auto &data = dataArr[offset];
			m_dataHandler.SetPreEMAData(comDataMap, data);
			++offset;
		}
	}

}

void CMainDlg::OnUpdateIndexMarket(int nMsgLength, const char* info) {
	CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
	SStringA SecurityID = pIndexData->SecurityID;
	if (m_MarketGetMap.count(SecurityID))
		m_IdxMarketMap[SecurityID].emplace_back(*pIndexData);
}

void CMainDlg::OnUpdateStockMarket(int nMsgLength, const char* info) {
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	if (m_MarketGetMap.count(SecurityID))
		m_StkMarketMap[SecurityID].emplace_back(*pStockData);
}

void CMainDlg::OnUpdateHisIndexMarket(int nMsgLength, const char* info) {
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonIndexMarket);
	CommonIndexMarket * dataArr = (CommonIndexMarket *)(info + nOffset);
	auto &MarketVec = m_IdxMarketMap[StockID];
	MarketVec.reserve(MAX_TICK);
	MarketVec.resize(dataCount);
	SStringA str;
	memcpy_s(&MarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_MarketGetMap[StockID] = TRUE;
	int nGroup = pRecvInfo->Group;
	LPARAM lp = MAKELPARAM(nGroup, DM_MARKET);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void CMainDlg::OnUpdateHisStockMarket(int nMsgLength, const char* info) {
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int dataCount = pRecvInfo->DataSize / sizeof(CommonStockMarket);
	CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
	auto &MarketVec = m_StkMarketMap[StockID];
	MarketVec.reserve(MAX_TICK);
	MarketVec.resize(dataCount);
	SStringA str;
	memcpy_s(&MarketVec[0], pRecvInfo->DataSize,
		dataArr, pRecvInfo->DataSize);
	m_MarketGetMap[StockID] = TRUE;
	int nGroup = pRecvInfo->Group;
	LPARAM lp = MAKELPARAM(nGroup, DM_MARKET);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void CMainDlg::OnUpdateHisKline(int nMsgLength, const char* info) {
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	char *StockID = new char[8];
	strcpy_s(StockID, 8, pRecvInfo->InsID);
	int nOffset = sizeof(*pRecvInfo);
	int nSize = pRecvInfo->DataSize / sizeof(KlineType);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_KlineGetMap[StockID][nPeriod] = TRUE;
	auto &KlineVec = m_KlineMap[StockID][nPeriod];
	KlineVec.resize(nSize);
	memcpy_s(&KlineVec[0], pRecvInfo->DataSize,
		info + nOffset, pRecvInfo->DataSize);
	//char *InsID = new char[8];
	LPARAM lp = MAKELPARAM(nGroup, DM_HISKLINE);
	::PostMessage(m_hWnd, WM_DATA_MSG,
		(WPARAM)StockID, lp);
}

void CMainDlg::OnUpdateCloseInfo(int nMsgLength, const char* info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	auto &closeMap = m_preCloseMap;
	::EnterCriticalSection(&m_csClose);
	closeMap.clear();
	for (int i = 0; i < dataCount; ++i)
		closeMap[dataArr[i].first] = dataArr[i].second;
	::LeaveCriticalSection(&m_csClose);
}

void CMainDlg::ReInit()
{
	m_bListInited = false;
	m_bTodayInit = false;
	ShowWindow(SW_HIDE);
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pFenShiPic[i]->SetPicUnHandled();
		m_pKlinePic[i]->SetPicUnHandled();
	}
	ResetEvent(g_hEvent);
	ResetEvent(g_hLoginEvent);
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		NULL, LoginMsg_Reinit);
	SStringW strInfo = L"重新初始化程序";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	m_NetHandleFlag.clear();
	ClearData();
	SetVectorSize();
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	ResetEvent(g_hLoginEvent);
	SendInfo info;
	info.MsgType = SendType_Connect;
	strcpy(info.str, "StkMarket");
	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	WaitForSingleObject(g_hEvent, INFINITE);
	while (!m_bServerReady)
	{
		m_bServerReady = true;
		::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
			NULL, LoginMsg_WaitAndTry);
		WaitForSingleObject(g_hLoginEvent, INFINITE);
		ResetEvent(g_hLoginEvent);
		SendInfo info;
		info.MsgType = SendType_Connect;
		strcpy(info.str, "StkMarket");
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	while (!CheckInfoRecv())
	{
		m_NetHandleFlag.clear();
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}

	strInfo = L"获取信息成功";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	ReInitList();
	strInfo = L"处理当日历史数据，请等待...";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	if (m_nLastOptWnd == Group_SWL1)
		m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL1],
			false, &m_IndexVec, &m_SWInd1Vec);
	if (m_nLastOptWnd == Group_SWL2)
		m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2],
			false, &m_IndexVec, &m_SWInd2Vec);
	if (m_nLastOptWnd == Group_Stock)
		m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock],
			false, &m_StockVec);
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		if (m_strSubStock[i] != "")
			ShowSubStockPic(m_strSubStock[i], m_pList[i]);
	}
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_HideWnd);
	ShowWindow(SW_SHOW);
	SwitchToThisWindow(m_hWnd, FALSE);

}

void CMainDlg::ClearData()
{
	m_dataVec.clear();
	m_RectMap.clear();
	m_PointGetVec.clear();
	m_ListPosVec.clear();
	m_ListInsVec.clear();
	m_listDataVec.clear();
	m_StockVec.clear();
	m_SWInd1Vec.clear();
	m_SWInd2Vec.clear();
	m_IndexVec.clear();
	m_StockNameMap.clear();
	m_commonDataMap.clear();
	m_MarketGetMap.clear();
	m_KlineGetMap.clear();
	m_StkMarketMap.clear();
	m_IdxMarketMap.clear();
	m_KlineMap.clear();
	m_LastVolMap.clear();
}

void CMainDlg::ReInitList()
{

	int i = 0;
	SStringW tmp;
	m_pList[Group_SWL1]->DeleteAllItems();
	for (auto &it : m_IndexVec)
	{
		tmp.Format(L"%d", i + 1);
		m_pList[Group_SWL1]->InsertItem(i, tmp);
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_ID,
			StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_Name,
			StrA2StrW(it.SecurityName));

		m_ListPosVec[Group_SWL1][it.SecurityID] = i;
		i++;
	}

	int nInd1 = i;
	for (auto &it : m_SWInd1Vec)
	{
		tmp.Format(L"%d", nInd1 + 1);
		m_pList[Group_SWL1]->InsertItem(nInd1, tmp);
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_ID,
			StrA2StrW(it.SecurityID));
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_Name,
			StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL1][it.SecurityID] = nInd1;
		//m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;

		nInd1++;
	}

	UpdateListShowStock(Group_SWL2);
	UpdateListShowStock(Group_Stock);

	m_bListInited = true;
	UpdateList();

}


void CMainDlg::OnBtnInd1MarketClicked()
{
	//SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1Market");
	////if (pBtn == m_pPreSelBtn[Group_SWL1])
	////	return;
	//if (m_bShowList[Group_SWL1])
	//{
	//	pBtn->SetWindowTextW(L"个股");
	//	SetBtnState(Group_SWL1, m_ListPeriodArr[Group_SWL1], false);
	//	SetBtnState(Group_SWL1, m_PeriodArr[Group_SWL1], true);
	//	SetSelectedPeriod(Group_SWL1, m_PeriodArr[Group_SWL1]);
	//}
	//else
	//{
	//	pBtn->SetWindowTextW(L"行情");
	//	SetBtnState(Group_SWL1, m_PeriodArr[Group_SWL1], false);
	//	SetBtnState(Group_SWL1, m_ListPeriodArr[Group_SWL1], true);
	//	SwitchPic2List(Group_SWL1);
	//}
	//m_bShowList[Group_SWL1] = !m_bShowList[Group_SWL1];

	OnBtnShowTypeChange(Group_SWL1);
	//if (!m_bShowList[0])
	//{
	//}
	//else
	//{

	//}
	//SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	////SetSelectedPeriod(Group_SWL1, Period_NULL);
	//m_bShowList[0] = !m_bShowList[0];

}

void CMainDlg::OnBtnInd1FenShiClicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_FenShi);
}

void CMainDlg::OnBtnInd1M1Clicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_1Min);
}

void CMainDlg::OnBtnInd1M5Clicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_5Min);
}

void CMainDlg::OnBtnInd1M15Clicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_15Min);
}

void CMainDlg::OnBtnInd1M30Clicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_30Min);
}
void CMainDlg::OnBtnInd1M60Clicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_60Min);
}

void CMainDlg::OnBtnInd1DayClicked()
{
	OnBtnPeriedChange(Group_SWL1, Period_1Day);
}

void CMainDlg::OnBtnInd2MarketClicked()
{
	//SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2Market");
	//if (m_bShowList[1])
	//	pBtn->SetWindowTextW(L"个股");
	//else
	//	pBtn->SetWindowTextW(L"行情");
	//SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	////SetSelectedPeriod(Group_SWL2, Period_NULL);
	//SwitchPic2List(Group_SWL2);
	OnBtnShowTypeChange(Group_SWL2);

}

void CMainDlg::OnBtnInd2FenShiClicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_FenShi);
}

void CMainDlg::OnBtnInd2M1Clicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_1Min);

}

void CMainDlg::OnBtnInd2M5Clicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_5Min);

}

void CMainDlg::OnBtnInd2M15Clicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_15Min);
}

void CMainDlg::OnBtnInd2M30Clicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_30Min);
}

void CMainDlg::OnBtnInd2M60Clicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_60Min);
}

void CMainDlg::OnBtnInd2DayClicked()
{
	OnBtnPeriedChange(Group_SWL2, Period_1Day);
}

void CMainDlg::OnBtnInd2ListConnectClicked()
{
	m_bListSWL2Conn = !m_bListSWL2Conn;
	if (!m_bListSWL2Conn)
		m_ListShowInd[Group_SWL2] = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_Ind2ListConnect");
	SetBtnState(pBtn, m_bListSWL2Conn);
	::PostMessage(m_hWnd, WM_MAIN_MSG,
		MAINMSG_UpdateListSingle, Group_SWL2);
	SaveListConfig();
}

void CMainDlg::OnBtnInd3MarketClicked()
{
	//SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3Market");
	//if (m_bShowList[2])
	//	pBtn->SetWindowTextW(L"个股");
	//else
	//	pBtn->SetWindowTextW(L"行情");
	//SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	////SetSelectedPeriod(Group_Stock, Period_NULL);
	//SwitchPic2List(Group_Stock);
	OnBtnShowTypeChange(Group_Stock);

}

void CMainDlg::OnBtnInd3FenShiClicked()
{
	OnBtnPeriedChange(Group_Stock, Period_FenShi);
}

void CMainDlg::OnBtnInd3M1Clicked()
{
	OnBtnPeriedChange(Group_Stock, Period_1Min);
}

void CMainDlg::OnBtnInd3M5Clicked()
{
	OnBtnPeriedChange(Group_Stock, Period_5Min);
}
void CMainDlg::OnBtnInd3M15Clicked()
{
	OnBtnPeriedChange(Group_Stock, Period_15Min);
}

void CMainDlg::OnBtnInd3M30Clicked()
{
	OnBtnPeriedChange(Group_Stock, Period_30Min);
}

void CMainDlg::OnBtnInd3M60Clicked()
{
	OnBtnPeriedChange(Group_Stock, Period_60Min);
}

void CMainDlg::OnBtnInd3DayClicked()
{
	OnBtnPeriedChange(Group_Stock, Period_1Day);
}

void CMainDlg::OnBtnInd3ListConnect1Clicked()
{
	m_bListStockConn1 = !m_bListStockConn1;
	if (!m_bListStockConn1)
		m_ListShowInd[Group_Stock] = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_Ind3ListConnect1");
	SetBtnState(pBtn, m_bListStockConn1);
	::PostMessage(m_hWnd, WM_MAIN_MSG,
		MAINMSG_UpdateListSingle, Group_Stock);
	SaveListConfig();

}

void CMainDlg::OnBtnInd3ListConnect2Clicked()
{
	m_bListStockConn2 = !m_bListStockConn2;
	if (!m_bListStockConn2)
		m_ListShowInd[Group_Stock] = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_Ind3ListConnect2");
	SetBtnState(pBtn, m_bListStockConn2);
	::PostMessage(m_hWnd, WM_MAIN_MSG,
		MAINMSG_UpdateListSingle, Group_Stock);
}

void CMainDlg::OnBtnShowTypeChange(int nGroup, bool bFroceList)
{
	if (bFroceList)
	{
		if (m_bShowList[nGroup])
		{
			if (nGroup != Group_SWL1
				&&m_ListShowInd[nGroup] != "")
			{
				m_ListShowInd[nGroup] = "";
				UpdateListShowStock(nGroup);
				SaveListConfig();
			}
			return;
		}
	}
	SStringW strBtn;
	strBtn.Format(L"btn_Ind%dMarket", nGroup + 1);
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	//if (pBtn == m_pPreSelBtn[Group_SWL1])
	//	return;
	if (m_bShowList[nGroup])
	{
		pBtn->SetWindowTextW(L"个股");
		SetBtnState(nGroup, m_ListPeriodArr[nGroup], false);
		SetBtnState(nGroup, m_PeriodArr[nGroup], true);
		SetSelectedPeriod(nGroup, m_PeriodArr[nGroup]);
	}
	else
	{
		pBtn->SetWindowTextW(L"行情");
		SetBtnState(nGroup, m_PeriodArr[nGroup], false);
		SetBtnState(nGroup, m_ListPeriodArr[nGroup], true);
		SwitchPic2List(nGroup);
	}
	m_bShowList[nGroup] = !m_bShowList[nGroup];
	SaveListConfig();
}

void CMainDlg::OnBtnPeriedChange(int nGroup, int nPeriod)
{
	SStringW strBtn;
	if (nPeriod == Period_FenShi)
		strBtn.Format(L"btn_Ind%dFS", nGroup + 1);
	else if (nPeriod == Period_1Day)
		strBtn.Format(L"btn_Ind%dDay", nGroup + 1);
	else
		strBtn.Format(L"btn_Ind%dM%d", nGroup + 1, nPeriod);
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	if (pBtn == m_pPreSelBtn[nGroup])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[nGroup]);
	//SetSelectedPeriod(nGroup, nGroup);
	if (m_bShowList[nGroup])
	{
		m_ListPeriodArr[nGroup] = nPeriod;
		SendMsg(m_RpsProcThreadID, UpdateSingleListData,
			(char*)&m_pList[nGroup], sizeof(m_pList[nGroup]));
	}
	else
		SetSelectedPeriod(nGroup, nPeriod);
	SaveListConfig();
}

void CMainDlg::SetSelectedPeriod(int nGroup, int nPeriod)
{
	if (m_PeriodArr[nGroup] == nPeriod
		&&m_strSubStock[nGroup] != "")
	{
		SwitchList2Pic(nGroup, nPeriod);
		return;
	}
	m_PeriodArr[nGroup] = nPeriod;
	//SendMsg(m_RpsProcThreadID, UpdateSingleListData,
	//	(char*)&m_pList[nGroup], sizeof(m_pList[nGroup]));
	//SSubPic * pPic = m_ppSubPic[nGroup];
	SStringA& StockID = m_strSubStock[nGroup];
	if (StockID == "")
	{
		for (auto &it : m_ListPosVec[nGroup])
		{
			if (it.second == 0)
			{
				StockID = it.first;
				ShowSubStockPic(StockID, m_pList[nGroup]);
				break;
			}
		}
	}

	BOOL bWaitData = FALSE;
	if (m_MarketGetMap.count(StockID) == 0)
	{
		m_pKlinePic[nGroup]->SetHisKlineState(false);
		GetMarket(StockID, nGroup);
		bWaitData = TRUE;
	}
	else
		m_pKlinePic[nGroup]->SetHisKlineState(true);


	if (Period_FenShi != nPeriod)
	{

		if (m_KlineGetMap[StockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisKlineState(false);
			GetHisKline(StockID, nPeriod, nGroup);
		}
		else
			m_pKlinePic[nGroup]->SetHisKlineState(true);

		if (m_PointGetVec[nGroup][StockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisPointState(false);
			GetHisPoint(StockID, nPeriod, nGroup);
		}
		else
			m_pKlinePic[nGroup]->SetHisPointState(true);

		SetKlineShowData(nGroup, nPeriod, TRUE);
		//vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
		//	&m_dataVec[nGroup][nPeriod][StockID]["Point520"],
		//	&m_dataVec[nGroup][nPeriod][StockID]["Point2060"] };
		//vector<BOOL> rightVec(SHOWDATACOUNT);
		//for (auto &it : rightVec)
		//	it = TRUE;

		//m_pKlinePic[nGroup]->SetSubPicShowData
		//(SHOWDATACOUNT, tmpDataArr, rightVec,
		//	dataNameVec, StockID, m_StockNameMap[StockID]);
		//m_pKlinePic[nGroup]->ChangePeriod(nPeriod, TRUE);
	}
	SwitchList2Pic(nGroup, nPeriod);

}

void CMainDlg::SetBtnState(SImageButton * nowBtn, SImageButton** preBtn)
{
	if (*preBtn)
		(*preBtn)->SetAttribute(L"colorText", L"#c0c0c0ff");
	nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	*preBtn = nowBtn;
}

void CMainDlg::SetBtnState(SImageButton * nowBtn, bool bSelected)
{
	if (bSelected)
		nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	else
		nowBtn->SetAttribute(L"colorText", L"#c0c0c0ff");

}

void CMainDlg::SetBtnState(int nGroup, int nPeriod, bool bSelected)
{
	SStringW strBtn;
	if (nPeriod == Period_FenShi)
		strBtn.Format(L"btn_Ind%dFS", nGroup + 1);
	else if (nPeriod == Period_1Day)
		strBtn.Format(L"btn_Ind%dDay", nGroup + 1);
	else
		strBtn.Format(L"btn_Ind%dM%d", nGroup + 1, nPeriod);
	SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	if (bSelected)
	{
		pBtn->SetAttribute(L"colorText", L"#00ffffff");
		m_pPreSelBtn[nGroup] = pBtn;
	}
	else
		pBtn->SetAttribute(L"colorText", L"#c0c0c0ff");

}

void CMainDlg::SetListShowIndyStr(SStatic * pText, int nGroup)
{
	if (m_ListShowInd[nGroup] == "")
		pText->SetWindowTextW(L"当前分类:全市场");
	else
	{
		SStringW strIndy = StrA2StrW(m_StockNameMap[m_ListShowInd[nGroup]]);
		SStringW strText;
		pText->SetWindowTextW(strText.Format(L"当前分类:%s", strIndy));
	}
}

void CMainDlg::SetVectorSize()
{
	m_dataVec.resize(Group_Count);
	m_RectMap.resize(Group_Count);
	m_PointGetVec.resize(Group_Count);
	m_ListPosVec.resize(Group_Count);
	m_ListInsVec.resize(Group_Count);
	m_listDataVec.resize(Group_Count);
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	if (SMessageBox(m_hWnd, _T("确定要退出么？"), _T("警告"), MB_OKCANCEL) == IDOK)
	{
		CSimpleWnd::DestroyWindow();
	}
	else
		return;
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

