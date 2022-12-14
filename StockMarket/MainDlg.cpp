// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "IniFile.h"
#include <fstream>
#include "WndSynHandler.h"
#include <helper/SMenu.h>
#include "DlgAddNew.h"
#include "DlgOpenWnd.h"

using std::ifstream;
using std::ofstream;

//HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//HANDLE g_hLoginEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

HWND g_MainWnd;
#define SHOWDATACOUNT 2
#define TIMER_AUTOSAVE 1
#define MAX_TICK 6000
#define MAX_SUBPIC 3

uint64_t g_uTestTime;

CWndSynHandler g_WndSyn;

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();
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
	g_WndSyn.SetMainWnd(m_hWnd);
	g_WndSyn.SetCmdLine(m_strCmdLine);
	g_WndSyn.Run();
	m_SynThreadID = g_WndSyn.GetThreadID();
	InitLogFile();
	tDataProc = thread(&CMainDlg::MsgProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	InitMsgHandleMap();
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
	OpenSubWindows();
	SwitchToThisWindow(m_hWnd, FALSE);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CMainDlg::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case WDMsg_HideWindow:
		ShowWindow(SW_HIDE);
		break;
	case WDMsg_ReInit:
		ReInit();
		break;
	case WDMsg_SaveConfig:
		SavePicConfig();
		break;
	case WDMsg_NewWindow:
	case WDMsg_OpenWindow:
		OpenSubWindowWithNum(int(lp));
		break;
	case WDMsg_SaveStockFilter:
		SaveStockFilterPara(int(lp));
		break;
	case WDMsg_Exit:
		exit(0);
		break;
	default:
		break;
	}
	return 0;
}


void CMainDlg::OnDestroy()
{

	SetMsgHandled(FALSE);
	CloseSubWindows();
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

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == TIMER_AUTOSAVE)
		SavePicConfig();
}

void CMainDlg::OnBtnNew()
{
	SMenu menu;
	if (menu.LoadMenuW(L"smenu:menu_new"))
	{
		SImageButton *pBtn = FindChildByID2<SImageButton>(R.id.btn_mulitWindow);
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen(&rc);
		menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);
	}
}

void CMainDlg::CloseSubWindows()
{
	vector<SStringA> ShowSubWindowVec;
	for (auto it : m_SubWndVec)
	{
		if (it->WindowIsValid())
		{
			it->OnClose();
			ShowSubWindowVec.emplace_back(it->GetWindowName());
		}
		it->CSimpleWnd::DestroyWindow();
	}
	CIniFile ini(".//config//subWindow.ini");
	ini.WriteIntA("SubWindows", "Count", ShowSubWindowVec.size());
	SStringA str;
	for (int i = 0; i < ShowSubWindowVec.size(); ++i)
		ini.WriteStringA("SubWindows", str.Format("%d", i),
			ShowSubWindowVec[i]);
}

void CMainDlg::OpenSubWindows()
{
	CIniFile ini(".//config//subWindow.ini");
	int nWindowCount = ini.GetIntA("SubWindows", "Count", 0);
	SStringA strTmp;
	for (int i = 0; i < nWindowCount; ++i)
	{
		SStringA strWindowName =
			ini.GetStringA("SubWindows",
				strTmp.Format("%d", i), "");
		if (strWindowName != "")
		{
			CDlgSub * pDlg = new CDlgSub(strWindowName);
			pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
				| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
			//pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			//	SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
			SetSubWindowPos(pDlg, strWindowName);
			pDlg->ShowWindow(SW_SHOW);
			m_SubWndVec.emplace_back(pDlg);
		}

	}
}

void CMainDlg::SetSubWindowPos(CDlgSub* pDlg, SStringA strName)
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.position", strName);
	ifile.open(strPosFile, std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(pDlg->m_hWnd, &wp);
		ifile.close();
	}
	else
	{
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
	}
}

void CMainDlg::OpenSubWindowWithNum(int nWndNum)
{
	CIniFile ini(".//config//subWindow.ini");
	SStringA str;
	SStringA strName = ini.GetStringA("SubWindowName",
		str.Format("%d", nWndNum), "");
	CDlgSub * pDlg = new CDlgSub(strName);
	pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
		| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
	SetSubWindowPos(pDlg, strName);
	pDlg->ShowWindow(SW_SHOW);
	m_SubWndVec.emplace_back(pDlg);
	//SwitchToThisWindow(pDlg->m_hWnd, FALSE);
}


void CMainDlg::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	switch (nID)
	{
	case WDM_New:
	{
		CDlgAddNew * pDlg = new CDlgAddNew(m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case WDM_Open:
	{
		CDlgOpenWnd * pDlg = new CDlgOpenWnd(m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);

	}
	break;
	default:
		break;
	}
}

void CMainDlg::SaveStockFilterPara(int nGroup)
{
	vector<StockFilter> sfVec;
	m_WndMap[nGroup]->OutputStockFilterPara(sfVec);
	SStringA strPath;
	strPath.Format(".//config//Main_SF_%d.DAT", nGroup);
	if (!sfVec.empty())
	{
		ofstream ofile(strPath, std::ios::binary);
		if (ofile.is_open())
		{
			int paraSize = sizeof(StockFilter);
			ofile.write((char*)&paraSize, sizeof(paraSize));
			ofile.write((char*)&sfVec[0], paraSize * sfVec.size());
			ofile.close();
		}
	}
	else
		DeleteFileA(strPath);

}

void CMainDlg::InitWorkWnd()
{
	SRealWnd * pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL1");
	m_WndMap[Group_SWL1] = (CWorkWnd *)pRealWnd->GetData();
	m_WndMap[Group_SWL1]->SetGroup(Group_SWL1, m_hWnd);
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL2");
	m_WndMap[Group_SWL2] = (CWorkWnd *)pRealWnd->GetData();
	m_WndMap[Group_SWL2]->SetGroup(Group_SWL2, m_hWnd);
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_Stock");
	m_WndMap[Group_Stock] = (CWorkWnd *)pRealWnd->GetData();
	m_WndMap[Group_Stock]->SetGroup(Group_Stock, m_hWnd);
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	vector<map<int, TimeLineMap>> *pListData = g_WndSyn.GetListData();
	strHash<double> preCloseMap(g_WndSyn.GetCloseMap());
	InitConfig();
	InitStockFilter();
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndMap[i]->SetParThreadID(m_DataThreadID);
		m_WndMap[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndMap[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		if (i == Group_Stock)
		{
			map<int, strHash<TickFlowMarket>> *pTFMarket = g_WndSyn.GetTFMarket();
			m_WndMap[i]->SetDataPoint(pTFMarket, DT_TFMarket);
		}
		m_WndMap[i]->SetPreClose(preCloseMap);
		m_WndMap[i]->InitList();
	}

}

void CMainDlg::InitStockFilter()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		vector<StockFilter> sfVec;
		SStringA strPath;
		strPath.Format(".//config//Main_SF_%d.DAT", i);
		ifstream ifile(strPath, std::ios::binary | std::ios::_Nocreate);
		if (ifile.is_open())
		{
			int nSFParaSize = 0;
			ifile.read((char*)&nSFParaSize, sizeof(nSFParaSize));
			StockFilter sfPara = { 0 };
			while (ifile.read((char*)&sfPara, nSFParaSize))
				sfVec.emplace_back(sfPara);
			m_WndMap[i]->InitStockFilterPara(sfVec);
		}

	}

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
			ini.GetIntA(strSection, "ShowMA", 1) == 0 ? false : true;
		initPara.bShowBandTarget =
			ini.GetIntA(strSection, "ShowBand") == 0 ? false : true;
		initPara.bShowAverage =
			ini.GetIntA(strSection, "ShowAvg", 1) == 0 ? false : true;
		initPara.bShowEMA =
			ini.GetIntA(strSection, "ShowEMA", 1) == 0 ? false : true;
		initPara.bShowTSCMACD =
			ini.GetIntA(strSection, "ShowTSCMACD", 1) == 0 ? false : true;
		initPara.bShowTSCVolume =
			ini.GetIntA(strSection, "ShowTSCVolume", 1) == 0 ? false : true;
		initPara.bShowKlineVolume =
			ini.GetIntA(strSection, "ShowKlineVolume", 1) == 0 ? false : true;
		initPara.bShowKlineMACD =
			ini.GetIntA(strSection, "ShowKlineMACD", 1) == 0 ? false : true;
		initPara.bShowTSCRPS[0] =
			ini.GetIntA(strSection, "ShowTSCRPS", 1) == 0 ? false : true;
		initPara.bShowTSCRPS[1] =
			ini.GetIntA(strSection, "ShowTSCL1RPS", 1) == 0 ? false : true;
		initPara.bShowTSCRPS[2] =
			ini.GetIntA(strSection, "ShowTSCL2RPS", 1) == 0 ? false : true;
		initPara.bShowKlineRPS[0] =
			ini.GetIntA(strSection, "ShowKlineRPS", 1) == 0 ? false : true;
		initPara.bShowKlineRPS[1] =
			ini.GetIntA(strSection, "ShowKlineL1RPS", 1) == 0 ? false : true;
		initPara.bShowKlineRPS[2] =
			ini.GetIntA(strSection, "ShowKlineL2RPS", 1) == 0 ? false : true;

		initPara.nWidth = ini.GetIntA(strSection, "Width", 16);
		initPara.bShowTSCDeal =
			ini.GetIntA(strSection, "ShowTSCDeal", 1) == 0 ? false : true;
		initPara.bShowKlineDeal =
			ini.GetIntA(strSection, "ShowKlineDeal", 1) == 0 ? false : true;
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
		initPara.UseStockFilter = ini.GetIntA(strSection, "StockFilter", 0)
			== 0 ? false : true;
		initPara.ListShowST = ini.GetIntA(strSection, "ListShowST", 1)
			== 0 ? false : true;
		initPara.ListShowSBM = ini.GetIntA(strSection, "ListShowSBM", 1)
			== 0 ? false : true;
		initPara.ListShowSTARM = ini.GetIntA(strSection, "ListShowSTARM", 1)
			== 0 ? false : true;
		initPara.ListShowNewStock = ini.GetIntA(strSection, "ListShowNewSotck", 1)
			== 0 ? false : true;

		m_WndMap[i]->InitShowConfig(initPara);
	}


}




void CMainDlg::SavePicConfig()
{
	CIniFile ini(".//config//WindowConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto initPara = m_WndMap[i]->OutPutInitPara();
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
		ini.WriteIntA(strSection, "StockFilter", initPara.UseStockFilter);
		ini.WriteIntA(strSection, "ListShowST", initPara.ListShowST);
		ini.WriteIntA(strSection, "ListShowSBM", initPara.ListShowSBM);
		ini.WriteIntA(strSection, "ListShowSTARM", initPara.ListShowSTARM);
		ini.WriteIntA(strSection, "ListShowNewSotck", initPara.ListShowNewStock);
	}

}


void CMainDlg::ReInit()
{
	ShowWindow(SW_HIDE);
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndMap[i]->SetPicUnHandled();
		m_WndMap[i]->ClearData();
	}
	ReInitWorkWnd();
	ShowWindow(SW_SHOW);
	//SwitchToThisWindow(m_hWnd, FALSE);

}


void CMainDlg::ReInitList()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
		m_WndMap[i]->ReInitList();
}

void CMainDlg::ReInitWorkWnd()
{
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	vector<map<int, TimeLineMap>> *pListData = g_WndSyn.GetListData();
	//strHash<double> preCloseMap(g_WndSyn.GetCloseMap());
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndMap[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndMap[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		//m_WndMap[i]->SetPreClose(preCloseMap);
		m_WndMap[i]->ReInitList();
		m_WndMap[i]->ReSetPic();
	}
}

void CMainDlg::InitMsgHandleMap()
{
	m_MsgHandleMap[WW_ListData]
		= &CMainDlg::OnUpdateList;
	m_MsgHandleMap[WW_GetMarket]
		= &CMainDlg::OnGetMarket;
	m_MsgHandleMap[WW_GetKline]
		= &CMainDlg::OnGetKline;
	m_MsgHandleMap[WW_GetPoint]
		= &CMainDlg::OnGetPoint;
	m_MsgHandleMap[Syn_Point]
		= &CMainDlg::OnUpdatePoint;
	m_MsgHandleMap[Syn_TodayPoint]
		= &CMainDlg::OnTodayPoint;
	m_MsgHandleMap[Syn_HisPoint]
		= &CMainDlg::OnHisPoint;
	m_MsgHandleMap[Syn_RTIndexMarket]
		= &CMainDlg::OnRTIndexMarket;
	m_MsgHandleMap[Syn_RTStockMarket]
		= &CMainDlg::OnRTStockMarket;
	m_MsgHandleMap[Syn_HisIndexMarket]
		= &CMainDlg::OnHisIndexMarket;
	m_MsgHandleMap[Syn_HisStockMarket]
		= &CMainDlg::OnHisStockMarket;
	m_MsgHandleMap[Syn_HisKline]
		= &CMainDlg::OnHisKline;
	m_MsgHandleMap[Syn_CloseInfo]
		= &CMainDlg::OnCloseInfo;
	m_MsgHandleMap[WW_ChangeIndy]
		= &CMainDlg::OnChangeIndy;

}

void CMainDlg::MsgProc()
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
		auto pFuc = m_MsgHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}

}

void CMainDlg::OnUpdateList(int nMsgLength, const char * info)
{
	for (auto &it : m_WndMap)
		SendMsg(it.second->GetThreadID(), WW_ListData,
			info, nMsgLength);

}

void CMainDlg::OnGetMarket(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	m_WndSubMap[pDgInfo->Group] = pDgInfo->StockID;
	SendMsg(m_SynThreadID, Syn_GetMarket, info, nMsgLength);
}

void CMainDlg::OnGetKline(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetKline, info, nMsgLength);
}

void CMainDlg::OnGetPoint(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetPoint, info, nMsgLength);
}

void CMainDlg::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_Point,
		info + 4, nMsgLength - 4);
}

void CMainDlg::OnTodayPoint(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_TodayPoint,
		info + 4, nMsgLength - 4);
}

void CMainDlg::OnHisPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	int nGroup = pRecvInfo->Group;
	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_HisPoint,
		info, nMsgLength);
}

void CMainDlg::OnRTIndexMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	if (m_WndSubMap[Group_SWL1] == strStock)
		SendMsg(m_WndMap[Group_SWL1]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
	if (m_WndSubMap[Group_SWL2] == strStock)
		SendMsg(m_WndMap[Group_SWL2]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
}

void CMainDlg::OnRTStockMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	if (m_WndSubMap[Group_Stock] == strStock)
		SendMsg(m_WndMap[Group_Stock]->GetThreadID(), WW_RTStockMarket,
			info, nMsgLength);
}

void CMainDlg::OnHisIndexMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	if (m_WndSubMap[nGroup] == strStock)
		SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_HisIndexMarket,
			info, nMsgLength);
}

void CMainDlg::OnHisStockMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	if (m_WndSubMap[nGroup] == strStock)
		SendMsg(m_WndMap[Group_Stock]->GetThreadID(), WW_HisStockMarket,
			info, nMsgLength);
}

void CMainDlg::OnHisKline(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	if (m_WndSubMap[nGroup] == strStock)
		SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_HisKline,
			info, nMsgLength);
}

void CMainDlg::OnCloseInfo(int nMsgLength, const char * info)
{
	for (int i = 0; i < Group_Count; ++i)
		SendMsg(m_WndMap[i]->GetThreadID(), WW_CloseInfo,
			info, nMsgLength);
}

void CMainDlg::OnChangeIndy(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	if (Group_SWL1 == nGroup)
	{
		for (int i = Group_SWL2; i < Group_Count; ++i)
			SendMsg(m_WndMap[i]->GetThreadID(), WW_ChangeIndy,
				info, nMsgLength);
	}
	else if (Group_SWL2 == nGroup)
		SendMsg(m_WndMap[Group_Stock]->GetThreadID(), WW_ChangeIndy,
			info, nMsgLength);
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

void CMainDlg::SetCmdLine(LPCTSTR lpstrCmdLine)
{
	m_strCmdLine = lpstrCmdLine;
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

