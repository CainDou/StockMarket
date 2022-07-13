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
#include "WndSynHandler.h"

using std::ifstream;
using std::ofstream;
using std::queue;

//HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//HANDLE g_hLoginEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

HWND g_MainWnd;
#define SHOWDATACOUNT 2
#define TIMER_AUTOSAVE 1
#define MAX_TICK 6000
#define MAX_SUBPIC 3

uint64_t g_uTestTime;

static CWndSynHandler g_WndSyn;

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
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
	g_WndSyn.SetMianWnd(m_hWnd);
	g_WndSyn.Run();
	InitConfig();
	InitLogFile();
	tDataProc = thread(&CMainDlg::MsgProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	InitWorkWnd();
	
	g_WndSyn.AddWnd(m_hWnd, m_DataThreadID);
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	ifile.open(L".\\config\\MainWnd.position", std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
	}
	ifile.close();
	SwitchToThisWindow(m_hWnd, FALSE);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CMainDlg::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case MAINMSG_ReInit:
		ReInit();
		break;
	default:
		break;
	}
	return 0;
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


void CMainDlg::OnDestroy()
{

	SetMsgHandled(FALSE);
	g_WndSyn.Close();
	SavePicConfig();
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	tDataProc.join();
	WINDOWPLACEMENT wp = { sizeof(wp) };
	::GetWindowPlacement(m_hWnd, &wp);
	std::ofstream ofile;
	ofile.open(L".\\config\\MainWnd.position",
		std::ios::out | std::ios::binary);
	if (ofile.is_open())
		ofile.write((char*)&wp, sizeof(wp));
	ofile.close();
}

void CMainDlg::InitWorkWnd()
{
	SRealWnd * pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL1");
	m_MainWndMap[Group_SWL1] = (CWorkWnd *)pRealWnd->GetData();
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL2");
	m_MainWndMap[Group_SWL2] = (CWorkWnd *)pRealWnd->GetData();
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_Stock");
	m_MainWndMap[Group_Stock] = (CWorkWnd *)pRealWnd->GetData();
	vector<vector<SStringA>> ListInsVec;
	unordered_map<SStringA, StockInfo, hash_SStringA> ListInfoMap;
	
}


void CMainDlg::InitConfig()
{
	CIniFile ini(".//config//WindowConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		InitPara initPara;
		SStringA strSection;
		strSection.Format("Group%d", i);
		initPara.bShowMA =
			ini.GetIntA(strSection, "ShowMA") == 0 ? false : true;
		initPara.bShowBandTarget =
			ini.GetIntA(strSection, "ShowBand") == 0 ? false : true;
		initPara.bShowAverage =
			ini.GetIntA(strSection, "ShowAvg") == 0 ? false : true;
		initPara.bShowEMA =
			ini.GetIntA(strSection, "ShowEMA") == 0 ? false : true;
		initPara.bShowTSCMACD =
			ini.GetIntA(strSection, "ShowTSCMACD") == 0 ? false : true;
		initPara.bShowTSCVolume =
			ini.GetIntA(strSection, "ShowTSCVolume") == 0 ? false : true;
		initPara.bShowKlineVolume =
			ini.GetIntA(strSection, "ShowKlineVolume") == 0 ? false : true;
		initPara.bShowKlineMACD =
			ini.GetIntA(strSection, "ShowKlineMACD") == 0 ? false : true;
		initPara.bShowTSCRPS[0] =
			ini.GetIntA(strSection, "ShowTSCRPS") == 0 ? false : true;
		initPara.bShowTSCRPS[1] =
			ini.GetIntA(strSection, "ShowTSCL1RPS") == 0 ? false : true;
		initPara.bShowTSCRPS[2] =
			ini.GetIntA(strSection, "ShowTSCL2RPS") == 0 ? false : true;
		initPara.bShowKlineRPS[0] =
			ini.GetIntA(strSection, "ShowKlineRPS") == 0 ? false : true;
		initPara.bShowKlineRPS[1] =
			ini.GetIntA(strSection, "ShowKlineL1RPS") == 0 ? false : true;
		initPara.bShowKlineRPS[2] =
			ini.GetIntA(strSection, "ShowKlineL2RPS") == 0 ? false : true;

		initPara.nWidth = ini.GetIntA(strSection, "Width", 16);
		initPara.bShowTSCDeal =
			ini.GetIntA(strSection, "ShowTSCDeal") == 0 ? false : true;
		initPara.bShowKlineDeal =
			ini.GetIntA(strSection, "ShowKlineDeal") == 0 ? false : true;
		initPara.nEMAPara[0] =
			ini.GetIntA(strSection, "EMAPara1", 12);
		initPara.nEMAPara[1] =
			ini.GetIntA(strSection, "EMAPara2", 26);
		initPara.nMACDPara[0] =
			ini.GetIntA(strSection, "MACDPara1", 12);
		initPara.nMACDPara[1] =
			ini.GetIntA(strSection, "MACDPara2", 26);
		initPara.nMACDPara[2] =
			ini.GetIntA(strSection, "MACDPara3", 9);
		initPara.nMAPara[0] =
			ini.GetIntA(strSection, "MAPara1", 5);
		initPara.nMAPara[1] =
			ini.GetIntA(strSection, "MAPara2", 10);
		initPara.nMAPara[2] =
			ini.GetIntA(strSection, "MAPara3", 20);
		initPara.nMAPara[3] =
			ini.GetIntA(strSection, "MAPara4", 60);
		initPara.nJiange =
			ini.GetIntA(strSection, "Jiange", 2);
		initPara.BandPara.N1 =
			ini.GetIntA(strSection, "BandN1", 8);
		initPara.BandPara.N2 =
			ini.GetIntA(strSection, "BandN2", 11);
		initPara.BandPara.K =
			ini.GetIntA(strSection, "BandK", 390);
		initPara.BandPara.M1 =
			ini.GetIntA(strSection, "BandM1", 8);
		initPara.BandPara.M2 =
			ini.GetIntA(strSection, "BandM2", 4);
		initPara.BandPara.P =
			ini.GetIntA(strSection, "BandP", 390);

		initPara.Period = ini.GetIntA(strSection, "Period", Period_1Day);
		initPara.Connect1 = ini.GetIntA(strSection, "ListConnent1", 0)
			== 0 ? false : true;
		initPara.Connect2 = ini.GetIntA(strSection, "ListConnent2", 0)
			== 0 ? false : true;
		SStringA strStock = ini.GetStringA(strSection, "ShowIndustry", "");
		strcpy_s(initPara.ShowIndy, strStock);

		m_MainWndMap[i]->InitShowConfig(initPara);
	}


}




void CMainDlg::SavePicConfig()
{
	CIniFile ini(".//config//WindowConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto initPara = m_MainWndMap[i]->OutPutInitPara();
		SStringA strSection;
		strSection.Format("Group%d", i);
		ini.WriteIntA(strSection, "ShowMA", initPara.bShowMA);
		ini.WriteIntA(strSection, "ShowBand", initPara.bShowBandTarget);
		ini.WriteIntA(strSection, "ShowAvg", initPara.bShowAverage);
		ini.WriteIntA(strSection, "ShowEMA", initPara.bShowEMA);
		ini.WriteIntA(strSection, "ShowTSCMACD", initPara.bShowTSCMACD);
		ini.WriteIntA(strSection, "ShowTSCVolume", initPara.bShowTSCVolume);
		ini.WriteIntA(strSection, "ShowKlineVolume", initPara.bShowKlineVolume);
		ini.WriteIntA(strSection, "ShowKlineMACD", initPara.bShowKlineMACD);
		ini.WriteIntA(strSection, "ShowTSCRPS", initPara.bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowTSCL1RPS", initPara.bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowTSCL2RPS", initPara.bShowTSCRPS[0]);
		ini.WriteIntA(strSection, "ShowKlineRPS", initPara.bShowKlineRPS[0]);
		ini.WriteIntA(strSection, "ShowKlineL1RPS", initPara.bShowKlineRPS[1]);
		ini.WriteIntA(strSection, "ShowKlineL2RPS", initPara.bShowKlineRPS[2]);
		ini.WriteIntA(strSection, "Width", initPara.nWidth);
		ini.WriteIntA(strSection, "ShowTSCDeal", initPara.bShowTSCDeal);
		ini.WriteIntA(strSection, "ShowKlineDeal", initPara.bShowKlineDeal);
		ini.WriteIntA(strSection, "EMAPara1", initPara.nEMAPara[0]);
		ini.WriteIntA(strSection, "EMAPara2", initPara.nEMAPara[1]);
		ini.WriteIntA(strSection, "MACDPara1", initPara.nMACDPara[0]);
		ini.WriteIntA(strSection, "MACDPara2", initPara.nMACDPara[1]);
		ini.WriteIntA(strSection, "MACDPara3", initPara.nMACDPara[2]);
		ini.WriteIntA(strSection, "MAPara1", initPara.nMAPara[0]);
		ini.WriteIntA(strSection, "MAPara2", initPara.nMAPara[1]);
		ini.WriteIntA(strSection, "MAPara3", initPara.nMAPara[2]);
		ini.WriteIntA(strSection, "MAPara4", initPara.nMAPara[3]);
		ini.WriteIntA(strSection, "Jiange", initPara.nJiange);
		ini.WriteIntA(strSection, "BandN1", initPara.BandPara.N1);
		ini.WriteIntA(strSection, "BandN2", initPara.BandPara.N2);
		ini.WriteIntA(strSection, "BandK", initPara.BandPara.K);
		ini.WriteIntA(strSection, "BandM1", initPara.BandPara.M1);
		ini.WriteIntA(strSection, "BandM2", initPara.BandPara.M2);
		ini.WriteIntA(strSection, "BandP", initPara.BandPara.P);
		ini.WriteIntA(strSection, "Period", initPara.Period);
		ini.WriteIntA(strSection, "ListConnent1", initPara.Connect1);
		ini.WriteIntA(strSection, "ListConnent2", initPara.Connect2);
		ini.WriteStringA(strSection, "ShowIndustry", initPara.ShowIndy);

	}

}


void CMainDlg::ReInit()
{
	ShowWindow(SW_HIDE);
	ReInitList();
	for (int i = Group_SWL1; i < Group_Count; ++i)
		m_MainWndMap[i]->ReSetPic();
	ShowWindow(SW_SHOW);
	SwitchToThisWindow(m_hWnd, FALSE);

}


void CMainDlg::ReInitList()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
		m_MainWndMap[i]->ReInitList();
}

void CMainDlg::MsgProc()
{
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

