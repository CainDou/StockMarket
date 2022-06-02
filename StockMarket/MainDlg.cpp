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

using std::ifstream;
using std::ofstream;
using std::queue;

HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hLoginEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

HWND g_MainWnd;
#define SHOWDATACOUNT 2
#define TIMER_RECONNECT 1

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
	//m_pSubPic[Group_SWL1] = nullptr;
	//m_pSubPic[Group_SWL2] = nullptr;
	//m_pSubPic[Group_Stock] = nullptr;
	m_pDlgKbElf = nullptr;
	m_nLastOptWnd = 1;
	m_bListSWL2Conn = false;
	m_bListStockConn1 = false;
	m_bListStockConn2 = false;
	m_ListShowInd[Group_SWL1] = "";
	m_ListShowInd[Group_SWL2] = "";
	m_ListShowInd[Group_Stock] = "";

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
	InitConfig();
	InitLogFile();
	m_PeriodVec.resize(7);
	m_PeriodVec[0] = 0;
	m_PeriodVec[1] = 1;
	m_PeriodVec[2] = 5;
	m_PeriodVec[3] = 15;
	m_PeriodVec[4] = 30;
	m_PeriodVec[5] = 60;
	m_PeriodVec[6] = 1440;

	for (int i = 0; i < 3; ++i)
	{
		m_PeriodArr[i] = Period_1Day;
		m_ListPeriodArr[i] = Period_1Day;
		m_bShowList[i] = true;
	}

	::InitializeCriticalSection(&m_csClose);

	m_hDataThread = (HANDLE)_beginthreadex(NULL, 0, DataHandle,
		this, NULL, &m_DataProcThreadID);
	m_NetClinet.SetWndHandle(m_hWnd);
	_beginthreadex(NULL, 0, LoginProcess, this, 0, NULL);
	WaitForSingleObject(g_hLoginEvent, INFINITE);

	m_NetClinet.RegisterHandle(NetHandle);
	m_NetClinet.Start(m_uNetThreadID, this);
	SendInfo info;
	info.MsgType = SendType_Connect;
	strcpy(info.str, "StkMarket");
	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	WaitForSingleObject(g_hEvent, INFINITE);
	SStringW Info = L"登陆成功,开始程序初始化";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);

	SetVectorSize();

	InitList();
	InitSubPic();
	InitShowConfig();

	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);
	//m_pDlgKbElf->SetVisible(FALSE);
	ResetEvent(g_hLoginEvent);
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

	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_DestoryWnd);
	SwitchToThisWindow(m_hWnd, FALSE);

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
	case MAINMSG_ProcFenShi:
	{
		char *InsID = (char*)lp;
		for (int i = 0; i < 3; ++i)
		{
			if (m_strSubStock[i] == InsID)
			{
				m_pFenShiPic[i]->DataProc();
				m_pKlinePic[i]->DataProc();
			}
		}
	}
	break;
	case MAINMSG_ProcKline:
	{
	}
	break;
	default:
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	USHORT nGroup = LOWORD(lp);
	FSMSG msg = (FSMSG)HIWORD(lp);
	SFenShiPic *pPic = m_pFenShiPic[nGroup];
	switch (msg)
	{
	case FSMSG_PROCDATA:
		m_pFenShiPic[nGroup]->DataProc();
		break;
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
	case KLINEMSG_PROCDATA:
	{
		m_pKlinePic[nGroup]->SetHisKlineState(true);
		if (m_pKlinePic[nGroup]->GetDataReadyState())
			m_pKlinePic[nGroup]->DataProc();
	}
	break;
	case KLINEMSG_HISPOINT:
	{
		m_pKlinePic[nGroup]->SetHisPointState(true);
		if (m_pKlinePic[nGroup]->GetDataReadyState())
			m_pKlinePic[nGroup]->DataProc();
	}
	break;

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
		pPic->SetRpsState();
		pPic->Invalidate();
		bState = pPic->GetRpsState();
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
		pPic->SetRpsState();
		pPic->Invalidate();
		bState = pPic->GetRpsState();
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
					&m_IndexVec, &m_SWInd1Vec);
			if (m_nLastOptWnd == Group_SWL2)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2],
					&m_IndexVec, &m_SWInd2Vec);
			if (m_nLastOptWnd == Group_Stock)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock],
					&m_StockVec);
			//m_pDlgKbElf->m_pEdit->SetWindowTextW(m_pDlgKbElf->wstrInput);
			m_pDlgKbElf->wstrInput = L"";
			SStringW input = L"";
			if (m_nLastChar >= '0' && m_nLastChar <= '9')
				input = (char)m_nLastChar;
			OutputDebugStringW(input);
			OutputDebugStringW(L"\n");
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
					&m_IndexVec, &m_SWInd1Vec);
			if (m_nLastOptWnd == Group_SWL2)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2],
					&m_IndexVec, &m_SWInd2Vec);
			if (m_nLastOptWnd == Group_Stock)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock],
					&m_StockVec);
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
		if (pPic->GetRpsState())
			menu.CheckMenuItem(FM_RPS, MF_CHECKED);
		if (pPic->GetAvgState())
			menu.CheckMenuItem(FM_Avg, MF_CHECKED);
		if (pPic->GetEmaState())
			menu.CheckMenuItem(FM_EMA, MF_CHECKED);
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
		if (pPic->GetRpsState())
			menu.CheckMenuItem(KM_RPS, MF_CHECKED);
		if (pPic->GetMaState())
			menu.CheckMenuItem(KM_MA, MF_CHECKED);
		if (pPic->GetBandState())
			menu.CheckMenuItem(KM_Band, MF_CHECKED);
		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;

	}
}

void CMainDlg::SwitchPic2List(int nGroup)
{
	SColorListCtrlEx * pList = nullptr;
	m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
	m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
	m_pKlinePic[nGroup]->ClearTip();
	pList = m_pList[nGroup];
	pList->SetVisible(TRUE, TRUE);
	pList->SetFocus();
	CRect rc = m_pFenShiPic[nGroup]->GetClientRect();
	SWindow::InvalidateRect(rc);

}

void CMainDlg::SwitchList2Pic(int nGroup, int nPeriod)
{
	if (nPeriod == Period_FenShi)
		m_pFenShiPic[nGroup]->SetVisible(TRUE, TRUE);
	else
		m_pKlinePic[nGroup]->SetVisible(TRUE, TRUE);
	m_pList[nGroup]->SetVisible(FALSE, TRUE);
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
	if (nIDEvent == TIMER_RECONNECT)
	{
		if (m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
		{
			SendIDInfo_t info = { 0 };
			info.MsgType = SendType_ReConnect;
			info.ClinetID = m_NetClinet.GetClientID();
			::send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
			KillTimer(1);
		}
	}
}

void CMainDlg::OnDestroy()
{
	SetMsgHandled(FALSE);
	delete m_pDlgKbElf;
	m_pDlgKbElf = nullptr;
	m_NetClinet.Stop();
	SavePicConfig();
	WINDOWPLACEMENT wp = { sizeof(wp) };
	SendMsg(m_DataProcThreadID, Msg_Exit, NULL, 0);
	WaitForSingleObject(m_hDataThread, INFINITE);
	::GetWindowPlacement(m_hWnd, &wp);
	std::ofstream ofile;
	ofile.open(L".\\config\\MainWnd.position",
		std::ios::out | std::ios::binary);
	if (ofile.is_open())
		ofile.write((char*)&wp, sizeof(wp));
	ofile.close();
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
		m_arrInit[i].bShowTSCRPS =
			ini.GetIntA(strSection, "ShowTSCRPS") == 0 ? false : true;
		m_arrInit[i].bShowKlineRPS =
			ini.GetIntA(strSection, "ShowKlineRPS") == 0 ? false : true;
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
		m_arrInit[i].bNoJiange =
			ini.GetIntA(strSection, "Jiange") == 0 ? false : true;
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
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pFenShiPic[i]->SetRpsGroup((RpsGroup(i)));
		m_pFenShiPic[i]->InitShowPara(m_arrInit[i]);
	}

	m_pKlinePic[Group_SWL1] = FindChildByName2<SKlinePic>(L"klinePic_Ind1");
	m_pKlinePic[Group_SWL2] = FindChildByName2<SKlinePic>(L"klinePic_Ind2");
	m_pKlinePic[Group_Stock] = FindChildByName2<SKlinePic>(L"klinePic_Ind3");
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pKlinePic[i]->SetRpsGroup((RpsGroup(i)));
		m_pKlinePic[i]->SetParentHwnd(m_hWnd);
		m_pKlinePic[i]->InitShowPara(m_arrInit[i]);
	}


	//m_pSubPic[Group_SWL1] = FindChildByName2<SSubPic>(L"subPic_Ind1");
	//m_pSubPic[Group_SWL2] = FindChildByName2<SSubPic>(L"subPic_Ind2");
	//m_pSubPic[Group_Stock] = FindChildByName2<SSubPic>(L"subPic_Ind3");

	//UpdateList();

}

void CMainDlg::InitShowConfig()
{
	CIniFile ini(".//config//config.ini");
	m_ListPeriodArr[Group_SWL1] = ini.GetIntA("List1", "Period", Period_1Day);
	m_ListPeriodArr[Group_SWL2] = ini.GetIntA("List2", "Period", Period_1Day);
	m_ListPeriodArr[Group_Stock] = ini.GetIntA("List3", "Period", Period_1Day);

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
		//else if (Period_NULL == m_PeriodArr[i])
		//	btnName.Format(L"btn_Ind%dMarket", i + 1);
		else
			btnName.Format(L"btn_Ind%dM%d", i, m_ListPeriodArr[i]);
		m_pPreSelBtn[i] = FindChildByName2<SImageButton>(btnName);
		m_pPreSelBtn[i]->SetAttribute(L"colorText", L"#00ffffff");
	}

}

void CMainDlg::SavePicConfig()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_pFenShiPic[i]->OutPutShowPara(m_arrInit[i]);
		m_pKlinePic[i]->OutPutShowPara(m_arrInit[i]);
	}

	CIniFile ini(".//config//config.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);

	ini.SetFileNameA(".//config//WindowConfig.ini");

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
		ini.WriteIntA(strSection, "ShowTSCRPS", m_arrInit[i].bShowTSCRPS);
		ini.WriteIntA(strSection, "ShowKlineRPS", m_arrInit[i].bShowKlineRPS);
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
		ini.WriteIntA(strSection, "Jiange", m_arrInit[i].bNoJiange);
		ini.WriteIntA(strSection, "BandN1", m_arrInit[i].BandPara.N1);
		ini.WriteIntA(strSection, "BandN2", m_arrInit[i].BandPara.N2);
		ini.WriteIntA(strSection, "BandK", m_arrInit[i].BandPara.K);
		ini.WriteIntA(strSection, "BandM1", m_arrInit[i].BandPara.M1);
		ini.WriteIntA(strSection, "BandM2", m_arrInit[i].BandPara.M2);
		ini.WriteIntA(strSection, "BandP", m_arrInit[i].BandPara.P);
	}

}

void CMainDlg::InitList()
{

	ResetEvent(g_hEvent);
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


	//StockInfo info;
	//strcpy_s(info.SecurityID, "000001");
	//strcpy_s(info.ExchangeID, "SSE");
	//strcpy_s(info.SecurityName, "test");
	//m_IndexVec.emplace_back(info);
	for (auto &it : m_IndexVec)
	{
		tmp.Format(L"%d", i + 1);
		m_pList[Group_SWL1]->InsertItem(i, tmp);
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_ID,
			StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_Name,
			StrA2StrW(it.SecurityName));

		m_pList[Group_SWL2]->InsertItem(i, tmp);
		m_pList[Group_SWL2]->SetSubItemText(i, SHead_ID,
			StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL2]->SetSubItemText(i, SHead_Name,
			StrA2StrW(it.SecurityName));

		m_ListPosVec[Group_SWL1][it.SecurityID] = i;
		m_ListPosVec[Group_SWL2][it.SecurityID] = i;
		m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
		m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
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
		m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;

		nInd1++;
	}

	int nInd2 = i;
	for (auto &it : m_SWInd2Vec)
	{
		tmp.Format(L"%d", nInd2 + 1);
		m_pList[Group_SWL2]->InsertItem(nInd2, tmp);
		m_pList[Group_SWL2]->SetSubItemText(nInd2, SHead_ID,
			StrA2StrW(it.SecurityID));
		m_pList[Group_SWL2]->SetSubItemText(nInd2, SHead_Name,
			StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL2][it.SecurityID] = nInd2;
		m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
		nInd2++;
	}


	int nStock = 0;
	for (auto &it : m_StockVec)
	{
		tmp.Format(L"%d", nStock + 1);
		m_pList[Group_Stock]->InsertItem(nStock, tmp);
		m_pList[Group_Stock]->SetSubItemText(nStock, SHead_ID,
			StrA2StrW(it.SecurityID));
		m_pList[Group_Stock]->SetSubItemText(nStock, SHead_Name,
			StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_Stock][it.SecurityID] = nStock;
		m_ListInsVec[Group_Stock].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
		nStock++;
	}

	SetEvent(g_hEvent);

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


	//int nColCount = m_pList->GetColumnCount();
	//SStringW ListName;
	//ListName.Format(L"ListHeader");
	//CIniFile ini(L".//config//config.ini");
	//for (int i = 0; i < nColCount; ++i)
	//{
	//	m_FacShowVec[i] = ini.GetInt(ListName, tmp.Format(L"Col%dVisible", i), 1) == 1 ? true : false;
	//	m_pHeader->SetOriItemIndex(i, ini.GetInt(ListName, tmp.Format(L"Col%dOrder", i), i));
	//}

	//for (int i = 0; i < nColCount; ++i)
	//	m_pHeader->SetItemShowVisible(i, m_FacShowVec[i]);

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
	//CIniFile ini(L".//config//config.ini");
	//SStringW listName;
	//SStringW tmp;
	//listName.Format(L"ListHeader");
	//for (int i = 0; i < nColCount; ++i)
	//	ini.WriteInt(listName, tmp.Format(L"Col%dOrder", i), Order[i]);
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

	//SStringW strBtn;
	//strBtn.Format(L"btn_Ind%dFS", nGroup + 1);
	//SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);
	m_PeriodArr[nGroup] = m_ListPeriodArr[nGroup];
	//SetBtnState(pBtn, &m_pPreSelBtn[nGroup]);
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
	if (nGroup == Group_SWL1)
	{
		if (m_bListSWL2Conn
			&&m_ListShowInd[Group_SWL2] != StockID)
		{
			m_ListShowInd[Group_SWL2] = StockID;
			UpdateListShowStock(Group_SWL2);
		}
		if (m_bListStockConn1
			&&m_ListShowInd[Group_Stock] != StockID)
		{
			m_ListShowInd[Group_Stock] = StockID;
			UpdateListShowStock(Group_Stock);
		}
	}
	else if (nGroup == Group_SWL2)
	{
		if (m_bListStockConn2
			&&m_ListShowInd[Group_Stock] != StockID)
		{
			m_ListShowInd[Group_Stock] = StockID;
			UpdateListShowStock(Group_Stock);
		}
	}
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
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
	int nGroup = -1;
	::EnterCriticalSection(&m_csClose);
	for (auto &ListMap : m_ListPosVec)
	{
		++nGroup;
		for (auto &it : ListMap)
		{
			SStringA StockID = it.first;
			auto & dataVec = m_listDataVec[nGroup][StockID];
			if (!isnan(dataVec["close"].value))
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
				m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, L"-");
				m_pList[nGroup]->SetSubItemText(it.second, SHead_ChangePct, L"-");

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
	}
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
			m_ListInsVec[nGroup].emplace_back(it.SecurityID);
			m_StockNameMap[it.SecurityID] = it.SecurityName;
			nCount++;
		}
	}
	//m_pList[nGroup]->Invalidate();
	m_pList[nGroup]->UpdateLayout();

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
	UINT dataThreadID = pMd->m_DataProcThreadID;
	int nOffset = 0;
	ReceiveInfo recvInfo;
	bool bNeedConnect = false;
	//int c = 0;
	while (true)
	{
		if (bNeedConnect)
		{
			if (pMd->m_NetClinet.GetExitState())
				return 0;
			if (pMd->m_NetClinet.OnConnect(pMd->m_strIPAddr, pMd->m_nIPPort))
			{
				SendIDInfo_t info = { 0 };
				info.MsgType = SendType_ReConnect;
				info.ClinetID = pMd->m_NetClinet.GetClientID();
				::send(pMd->m_NetClinet.GetSocket(), (char*)&info,
					sizeof(info), 0);
				bNeedConnect = false;
			}
			else
				continue;
		}

		SOCKET netSocket = pMd->m_NetClinet.GetSocket();
		int ret = recv(netSocket, (char*)&recvInfo + nOffset,
			sizeof(recvInfo) - nOffset, 0);
		TraceLog("接收数据大小%d", ret);
		//sprintf_s(buffer, "连接成功了%d",++c);
		//send(netSocket, buffer, 1024, 0);
		if (ret == 0)
		{
			nOffset = 0;
			pMd->m_NetClinet.OnConnect(NULL, NULL);
			bNeedConnect = true;
			TraceLog("与服务器断开连接");
			continue;
		}

		if (SOCKET_ERROR == ret)
		{
			//nOffset = 0;
			if (pMd->m_NetClinet.GetExitState())
				return 0;
			int nError = WSAGetLastError();
			if (nError == WSAECONNRESET)
			{
				pMd->m_NetClinet.OnConnect(NULL, NULL);
				bNeedConnect = true;
			}
			continue;
		}

		if (ret + nOffset < sizeof(recvInfo))
		{
			nOffset += ret;
			continue;
		}
		nOffset = 0;
		TraceLog("接收数据类型是%d", recvInfo.MsgType);

		switch (recvInfo.MsgType)
		{
		case RecvMsg_ClientID:
			pMd->m_NetClinet.SetClientID(((ReceiveIDInfo)recvInfo).ClientID);
			break;
		case RecvMsg_StockInfo:
		{
			TraceLog("接收股票信息");
			char *buffer = new char[recvInfo.nDataSize1];
			StockInfo stkInfo = { 0 };
			Sleep(10);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
			{
				int size = recvInfo.nDataSize1 / sizeof(stkInfo);
				switch (recvInfo.nDataSize2)
				{
				case StockInfo_Stock:
					pMd->m_StockVec.resize(size);
					memcpy_s(&pMd->m_StockVec[0], recvInfo.nDataSize1,
						buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_SWL1:
					pMd->m_SWInd1Vec.resize(size);
					memcpy_s(&pMd->m_SWInd1Vec[0], recvInfo.nDataSize1,
						buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_SWL2:
					pMd->m_SWInd2Vec.resize(size);
					memcpy_s(&pMd->m_SWInd2Vec[0], recvInfo.nDataSize1,
						buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_Index:
					pMd->m_IndexVec.resize(size);
					memcpy_s(&pMd->m_IndexVec[0], recvInfo.nDataSize1,
						buffer, recvInfo.nDataSize1);
					break;
				default:
					break;
				}
			}
			delete[] buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_RTTimeLine:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			TimeLineData stkInfo = { 0 };
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;

		}
		break;
		case RecvMsg_TodayData:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateTodayData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
			SetEvent(g_hEvent);
		}
		break;
		case RecvMsg_HisData:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateHisData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisPoint:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisPoint, buffer, totalSize);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_LastDayEma:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateLastDayEma, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_RTIndexMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateIndexMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_RTStockMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateStockMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisIndexMarket:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisIndexMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisStockMarket:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisStockMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisKline:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisKline, buffer, totalSize);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_CloseInfo:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateCloseInfo, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		default:
			break;
		}

	}
	return 0;
}
unsigned CMainDlg::DataHandle(void * para)
{
	int MsgId;
	char *info;
	int msgLength;
	bool bExit = false;

	CMainDlg *pMd = (CMainDlg*)para;
	CDataProc& dataHandler = pMd->m_dataHandler;
	auto& ListDataVec = pMd->m_listDataVec;
	auto& ListInsVec = pMd->m_ListInsVec;
	auto& dataVec = pMd->m_dataVec;
	auto &commonDataMap = pMd->m_commonDataMap;
	auto& periodVec = pMd->m_PeriodVec;
	auto& KlineMap = pMd->m_KlineMap;
	auto& LastVolMap = pMd->m_LastVolMap;
	auto& marketGetMap = pMd->m_MarketGetMap;
	auto& klineGetMap = pMd->m_KlineGetMap;
	bool bTodayInit = false;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nTradingDay = st.wYear * 10000 + st.wMonth + st.wDay;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		//OutputDebugString(L"接收到信息\n");
		//SStringW strInfo;
		//strInfo.Format(L"数据是%d\n", MsgId);
		//OutputDebugString(strInfo);

		switch (MsgId)
		{
		case UpdateData:
		{
			if (!bTodayInit)
				break;
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData *dataArr = (TimeLineData *)info;

			for (int i = 0; i < periodVec.size(); ++i)
			{
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];
				for (int j = 0; j < dataCount; ++j)
				{
					if (dataArr[j].data.time == 925)
					{
						if (i == 0)
							dataHandler.SetPeriodFenshiOpenEMAData(
								periodDataMap, dataArr[j]);
					}
					else
						dataHandler.UpdateClose(
							periodDataMap, dataArr[j], periodVec[i]);
				}
				dataHandler.CalcRps(periodDataMap);
				dataHandler.RankPoint(periodDataMap,
					dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPoint(periodDataMap,
					dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPoint(periodDataMap,
					dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}
			int ShowPeriod = pMd->m_ListPeriodArr[Group_SWL1];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_SWL1][ShowPeriod], ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL1]);
			//period = pMd->m_PeriodArr[Group_SWL2];
			ShowPeriod = pMd->m_ListPeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_SWL2][ShowPeriod], ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL2]);
			//period = pMd->m_PeriodArr[Group_Stock];
			ShowPeriod = pMd->m_ListPeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_Stock][ShowPeriod], ListDataVec[Group_Stock],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_Stock]);

			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
		}
		break;
		case UpdateTodayData:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData *dataArr = (TimeLineData *)info;
			WaitForSingleObject(g_hEvent, INFINITE);
			for (int i = 0; i < periodVec.size(); ++i)
			{
				SStringA str;
				str.Format("开始计算周期%d\n", periodVec[i]);
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];

				int tick = GetTickCount();
				SStringA nowStockID = "";
				vector<CoreData> * pDataVec = nullptr;

				for (int j = 0; j < dataCount; ++j)
				{
					if (nowStockID != dataArr[j].securityID)
					{
						nowStockID = dataArr[j].securityID;
						pDataVec = &commonDataMap[Period][nowStockID]["close"];
					}
					if (dataArr[j].data.time == 925)
					{
						if (i == 0)
							dataHandler.SetPeriodFenshiOpenEMAData(periodDataMap,
								dataArr[j]);
					}
					else
						dataHandler.UpdateClose(*pDataVec, dataArr[j], Period);
					//pDataVec->emplace_back(dataArr[i]);
				}

				dataHandler.CalcHisRps(periodDataMap);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}
			int ShowPeriod = pMd->m_ListPeriodArr[Group_SWL1];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_SWL1][ShowPeriod], ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL1]);
			ShowPeriod = pMd->m_ListPeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_SWL2][ShowPeriod], ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL2]);
			ShowPeriod = pMd->m_ListPeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[ShowPeriod],
				dataVec[Group_Stock][ShowPeriod], ListDataVec[Group_Stock],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec,
				ListInsVec[Group_Stock]);

			bTodayInit = true;
			SetEvent(g_hLoginEvent);
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
			for (auto &it : commonDataMap)
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
		break;
		case UpdateRPS:
			break;
		case UpdateHisData:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData* dataArr = (TimeLineData*)info;

			for (int i = 0; i < periodVec.size(); ++i)
			{
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];

				dataHandler.UpdateHisData(periodDataMap, dataArr, dataCount, Period);
				dataHandler.CalcHisRps(periodDataMap);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPointHisData(periodDataMap,
					dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}
			int period = pMd->m_PeriodArr[Group_SWL1];
			dataHandler.UpdateShowData(commonDataMap[period],
				dataVec[Group_SWL1][period],
				ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec,
				pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL1]);
			period = pMd->m_PeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[period],
				dataVec[Group_SWL2][period],
				ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec,
				pMd->m_uniDataNameVec,
				ListInsVec[Group_SWL2]);
			period = pMd->m_PeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[period],
				dataVec[Group_Stock][period],
				ListDataVec[Group_Stock],
				pMd->m_comDataNameVec,
				pMd->m_uniDataNameVec,
				ListInsVec[Group_Stock]);
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
			for (auto &it : commonDataMap)
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
		break;
		case UpdateSingleListData:
		{
			SColorListCtrlEx ** ppList = (SColorListCtrlEx **)info;
			int nGroup = pMd->GetGroupFromList(*ppList);
			if (-1 == nGroup)
				break;
			int period = pMd->m_PeriodArr[nGroup];
			dataHandler.UpdateShowData(commonDataMap[period],
				dataVec[nGroup][period],
				ListDataVec[nGroup],
				pMd->m_comDataNameVec,
				pMd->m_uniDataNameVec,
				ListInsVec[nGroup]);
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
		}
		break;
		case UpdateHisPoint:
		{
			ReceiveInfo recvInfo;
			memcpy_s(&recvInfo, sizeof(recvInfo), info, sizeof(recvInfo));
			int nOffset = sizeof(recvInfo);
			int nSize520 = recvInfo.nDataSize2 / sizeof(CoreData);
			int nSize2060 = (recvInfo.nDataSize1 - recvInfo.nDataSize2) /
				sizeof(CoreData);
			int nGroup = recvInfo.Group;
			int nPeriod = recvInfo.Period;
			//SColorListCtrlEx * pList = nullptr;
			//if (nGroup == Group_SWL1)
			//	pList = pMd->m_pList[Group_SWL1];
			//else if (nGroup == Group_SWL2)
			//	pList = pMd->m_pList[Group_SWL2];
			//else if (nGroup == Group_Stock)
			//	pList = Group_Stock;
			pMd->m_PointGetVec[nGroup][recvInfo.InsID][nPeriod] = TRUE;
			vector<CoreData> PointVec(nSize520);
			memcpy_s(&PointVec[0], recvInfo.nDataSize2,
				info + nOffset, recvInfo.nDataSize2);
			nOffset += recvInfo.nDataSize2;
			auto &Point520Vec = dataVec[nGroup][nPeriod][recvInfo.InsID]["Point520"];
			Point520Vec.insert(Point520Vec.begin(), PointVec.begin(), PointVec.end());
			PointVec.resize(nSize2060);
			memcpy_s(&PointVec[0], recvInfo.nDataSize1 - recvInfo.nDataSize2,
				info + nOffset, recvInfo.nDataSize1 - recvInfo.nDataSize2);
			auto &Point2060Vec = dataVec[nGroup][nPeriod][recvInfo.InsID]["Point2060"];
			Point2060Vec.insert(Point2060Vec.begin(), PointVec.begin(), PointVec.end());
			LPARAM lp = MAKELPARAM(nGroup, KLINEMSG_HISPOINT);
			::PostMessage(pMd->m_hWnd, WM_KLINE_MSG,
				NULL, lp);

		}
		break;
		case UpdateLastDayEma:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData* dataArr = (TimeLineData*)info;
			int preDataCount = dataCount / 6;
			int offset = 0;
			for (int i = 1; i < 7; ++i)	//分时图数据不需要设置
			{
				int nPeriod = periodVec[i];
				auto &comDataMap = commonDataMap[nPeriod];
				for (int j = 0; j < preDataCount; ++j)
				{
					auto &data = dataArr[offset];
					dataHandler.SetPreEMAData(comDataMap, data);
					++offset;
				}
			}

		}
		break;
		case UpdateIndexMarket:
		{
			CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
			SStringA SecurityID = pIndexData->SecurityID;
			if (marketGetMap.count(SecurityID))
				pMd->m_IdxMarketMap[SecurityID].emplace_back(*pIndexData);
		}
		break;
		case UpdateStockMarket:
		{
			CommonStockMarket* pStockData = (CommonStockMarket*)info;
			SStringA SecurityID = pStockData->SecurityID;
			if (marketGetMap.count(SecurityID))
				pMd->m_StkMarketMap[SecurityID].emplace_back(*pStockData);
		}
		break;
		case UpdateHisIndexMarket:
		{
			ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
			SStringA StockID = pRecvInfo->InsID;
			int nOffset = sizeof(*pRecvInfo);
			int dataCount = pRecvInfo->nDataSize1 / sizeof(CommonIndexMarket);
			CommonIndexMarket * dataArr = (CommonIndexMarket *)(info + nOffset);
			auto &MarketVec = pMd->m_IdxMarketMap[StockID];
			MarketVec.resize(dataCount);
			memcpy_s(&MarketVec[0], msgLength, dataArr, msgLength);
			marketGetMap[StockID] = TRUE;
			int nGroup = pRecvInfo->Group;
			LPARAM lp = MAKELPARAM(nGroup, FSMSG_PROCDATA);
			::PostMessage(pMd->m_hWnd, WM_FENSHI_MSG,
				NULL, lp);

			//char *InsID = new char[8];
			//::PostMessage(pMd->m_hWnd, WM_FENSHI_MSG,
			//	(WPARAM)&InfoVec[infoID], FSMSG_PROCDATA);
			//::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_ProcKline, (LPARAM)InsID);

		}
		break;
		case UpdateHisStockMarket:
		{
			ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
			SStringA StockID = pRecvInfo->InsID;
			int nOffset = sizeof(*pRecvInfo);
			int dataCount = pRecvInfo->nDataSize1 / sizeof(CommonStockMarket);
			CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
			auto &MarketVec = pMd->m_StkMarketMap[StockID];
			MarketVec.resize(dataCount);
			memcpy_s(&MarketVec[0], msgLength, dataArr, msgLength);
			marketGetMap[StockID] = TRUE;
			int nGroup = pRecvInfo->Group;
			LPARAM lp = MAKELPARAM(nGroup, FSMSG_PROCDATA);
			::PostMessage(pMd->m_hWnd, WM_FENSHI_MSG,
				NULL, lp);

			//int infoID = pMd->SetHisDataMsg(InfoVec, dataArr[0].SecurityID);
			//::PostMessage(pMd->m_hWnd, WM_FENSHI_MSG,
			//	(WPARAM)&InfoVec[infoID], FSMSG_PROCDATA);
			//::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_ProcKline, (LPARAM)InsID);

		}
		break;
		case UpdateHisKline:
		{
			ReceiveInfo recvInfo;
			memcpy_s(&recvInfo, sizeof(recvInfo), info, sizeof(recvInfo));
			SStringA StockID = recvInfo.InsID;
			int nOffset = sizeof(recvInfo);
			int nSize = recvInfo.nDataSize1 / sizeof(KlineType);
			int nGroup = recvInfo.Group;
			int nPeriod = recvInfo.Period;
			pMd->m_KlineGetMap[StockID][nPeriod] = TRUE;
			auto &KlineVec = KlineMap[StockID][nPeriod];
			KlineVec.resize(nSize);
			memcpy_s(&KlineVec[0], recvInfo.nDataSize1,
				info + nOffset, recvInfo.nDataSize1);
			//char *InsID = new char[8];
			LPARAM lp = MAKELPARAM(nGroup, KLINEMSG_PROCDATA);
			::PostMessage(pMd->m_hWnd, WM_KLINE_MSG,
				NULL, lp);


			//if (nGroup == Group_SWL1 || nGroup == Group_SWL2)
			//{
			//	auto &marketVec = pMd->m_IdxMarketMap[StockID];
			//	auto &LastVol = LastVolMap[StockID][nPeriod];
			//	for (auto &it : marketVec)
			//		dataHandler.CalcKline(KlineVec, it, nPeriod, LastVol);
			//}
			//else if (nGroup == Group_Stock)
			//{
			//	auto &marketVec = pMd->m_StkMarketMap[StockID];
			//	auto &LastVol = LastVolMap[StockID][nPeriod];
			//	for (auto &it : marketVec)
			//		dataHandler.CalcKline(KlineVec, it, nPeriod, LastVol, nTradingDay);
			//}
			//::PostMessage(pMd->m_hWnd, WM_KLINE_MSG, MAINMSG_UpdateList, 0);

		}
		break;
		case UpdateCloseInfo:
		{
			pair<char[8], double>preCloseData;
			int dataCount = msgLength / sizeof(preCloseData);
			pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
			auto &closeMap = pMd->m_preCloseMap;
			::EnterCriticalSection(&pMd->m_csClose);
			closeMap.clear();
			for (int i = 0; i < dataCount; ++i)
				closeMap[dataArr[i].first] = dataArr[i].second;
			::LeaveCriticalSection(&pMd->m_csClose);

		}
		break;
		case Msg_Exit:
			bExit = true;
			break;
		default:
			break;
		}
		delete[]info;
		info = nullptr;
		if (bExit)
			break;
	}
	return 0;
}

unsigned CMainDlg::LoginProcess(void * para)
{
	CMainDlg *pThis = (CMainDlg*)para;
	pThis->m_pLoginDlg = new CDlgLogin(pThis->m_hWnd, &pThis->m_NetClinet);
	pThis->m_pLoginDlg->SetIPInfo(pThis->m_strIPAddr, pThis->m_nIPPort);
	pThis->m_pLoginDlg->DoModal();
	return 0;
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
		GetMarket(ShowStockID, nGroup);
		OutputDebugStringA("获取数据\n");
	}
	else
		bNeedMarket = FALSE;


	SStringA StockName = m_StockNameMap[ShowStockID];

	//if (ShowStockID.Find('I') != -1)
	//	ShowStockID = ShowStockID.Left(6);
	pList->SetVisible(FALSE);
	CRect rc = pList->GetClientRect();
	if (pList->HasScrollBar(TRUE))
		rc.right += 20;
	if (pList->HasScrollBar(FALSE))
		rc.bottom += 20;


	SFenShiPic *pFenShiPic = m_pFenShiPic[nGroup];
	SKlinePic *pKlinePic = m_pKlinePic[nGroup];
	vector<SStringA> dataNameVec;
	dataNameVec.emplace_back("Point520");
	dataNameVec.emplace_back("Point2060");

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


	if (Period_FenShi == nPeriod/* || Period_NULL == nPeriod*/)
	{
		//if (Period_NULL == nPeriod)
		//{
		SStringW strBtn;
		strBtn.Format(L"btn_Ind%dFS", nGroup + 1);
		SImageButton * pBtn = FindChildByName2<SImageButton>(strBtn);

		SetBtnState(pBtn, &m_pPreSelBtn[nGroup]);
		//}

		pFenShiPic->SetVisible(TRUE);
		pFenShiPic->SetFocus();

	}
	else
	{
		pKlinePic->SetVisible(TRUE);
		if (m_KlineGetMap[stockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisKlineState(false);
			GetHisKline(stockID, nPeriod, nGroup);
		}

		if (m_PointGetVec[nGroup][stockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisPointState(false);
			GetHisPoint(stockID, nPeriod, nGroup);
		}
		m_pKlinePic[nGroup]->ChangePeriod(nPeriod, FALSE);
		m_pKlinePic[nGroup]->SetFocus();
		vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
			&m_dataVec[nGroup][nPeriod][stockID]["Point520"],
			&m_dataVec[nGroup][nPeriod][stockID]["Point2060"]
		};

		vector<BOOL> rightVec(SHOWDATACOUNT);
		for (auto &it : rightVec)
			it = TRUE;
		pKlinePic->SetSubPicShowData(SHOWDATACOUNT, tmpDataArr,
			rightVec, dataNameVec,
			ShowStockID, m_StockNameMap[stockID]);

	}

	//设置分时初始数据
	vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
		&m_dataVec[nGroup][Period_FenShi][stockID]["Point520"],
		&m_dataVec[nGroup][Period_FenShi][stockID]["Point2060"] };
	vector<BOOL> rightVec(SHOWDATACOUNT);
	for (auto &it : rightVec)
		it = TRUE;
	pFenShiPic->SetSubPicShowData(SHOWDATACOUNT, tmpDataArr, rightVec, dataNameVec,
		ShowStockID, m_StockNameMap[stockID]);

	if (!bNeedMarket)
	{
		OutputDebugStringA("不需要处理获取数据");
		if (Period_FenShi == nPeriod)
			pFenShiPic->DataProc();
		else if (pKlinePic->GetDataReadyState())
			pKlinePic->DataProc();

	}
	m_bShowList[nGroup] = false;
	SWindow::InvalidateRect(rc);
}

//bool CMainDlg::GetHisData(int satrtDate, int nDay, char* msg)
//{
//	SendInfo info = { 0 };
//	info.MsgType = SendType_GetHisData;
//	info.StartDate = satrtDate;
//	info.EndDate = nDay;
//	strcpy_s(info.str, msg);
//	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
//	return ret > 0;
//}

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
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, NULL);
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
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, NULL);
}

void CMainDlg::OnBtnInd3ListConnect2Clicked()
{
	m_bListStockConn2 = !m_bListStockConn2;
	if (!m_bListStockConn2)
		m_ListShowInd[Group_Stock] = "";
	SImageButton * pBtn =
		FindChildByName2<SImageButton>(L"btn_Ind3ListConnect2");
	SetBtnState(pBtn, m_bListStockConn2);
	::PostMessage(m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, NULL);
}

void CMainDlg::OnBtnShowTypeChange(int nGroup, bool bFroceList)
{
	if (bFroceList)
	{
		if (m_bShowList[nGroup])
			return;
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
		m_ListPeriodArr[nGroup] = nPeriod;
	else
		SetSelectedPeriod(nGroup, nPeriod);

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
	SendMsg(m_DataProcThreadID, UpdateSingleListData,
		(char*)&m_pList[nGroup], sizeof(m_pList[nGroup]));
	//SSubPic * pPic = m_pSubPic[nGroup];
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

	if (m_MarketGetMap.count(StockID) == 0)
		GetMarket(StockID, nGroup);

	if (nPeriod == Period_FenShi)
	{
		m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
		m_pFenShiPic[nGroup]->SetVisible(TRUE, TRUE);
		m_pList[nGroup]->SetVisible(FALSE, TRUE);
		m_pFenShiPic[nGroup]->SetFocus();
	}
	else
	{

		if (m_KlineGetMap[StockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisKlineState(false);
			GetHisKline(StockID, nPeriod, nGroup);
		}

		if (m_PointGetVec[nGroup][StockID].count(nPeriod) == 0)
		{
			m_pKlinePic[nGroup]->SetHisPointState(false);
			GetHisPoint(StockID, nPeriod, nGroup);
		}

		vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
			&m_dataVec[nGroup][nPeriod][StockID]["Point520"],
			&m_dataVec[nGroup][nPeriod][StockID]["Point2060"] };
		vector<BOOL> rightVec(SHOWDATACOUNT);
		for (auto &it : rightVec)
			it = TRUE;

		m_pKlinePic[nGroup]->ReSetSubPicData(2, tmpDataArr, rightVec);
		m_pKlinePic[nGroup]->ChangePeriod(nPeriod, TRUE);
		m_pKlinePic[nGroup]->SetVisible(TRUE, TRUE);
		m_pKlinePic[nGroup]->SetFocus();
		m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
		m_pList[nGroup]->SetVisible(FALSE, TRUE);
	}

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

