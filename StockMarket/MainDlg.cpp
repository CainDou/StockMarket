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
#include "DlgFrmlManage.h"
#include "DlgStockFilter.h"
#include "DlgUpdateLog.h"
#include <ShellAPI.h>

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

const char g_strUpdateDate[] = "20230731";

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_pDlgLUS = nullptr;
	//m_pDlgMultiFilter = nullptr;
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
	InitWindowPos();
	InitMsgHandleMap();
	InitWorkWnd();
	g_WndSyn.AddWnd(m_hWnd, m_DataThreadID);
	OpenSubWindows();
	OpenFilterWindows();
	OpenLimitUpWindow();
	SwitchToThisWindow(m_hWnd, FALSE);
	SetTimer(TIMER_AUTOSAVE, 5000);
	ShowUpdateLog();
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
	{
		int nType = HIWORD(lp);
		int nNum = LOWORD(lp);
		if(nType == WT_SubWindow)
			OpenSubWindowWithNum(nNum);
		else if(nType == WT_FilterWindow)
			OpenFilterWindowWithNum(nNum);
	}
		break;
	case WDMsg_SaveStockFilter:
		SaveComboStockFilterPara(int(lp));
		break;
	case WDMsg_SaveListConfig:
		SaveListConfig();
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
	CloseFilterWindows();
	CloseLimitUpWindow();
	g_WndSyn.Close();
	SavePicConfig();
	SaveListConfig();
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
	m_nNewWndType = WT_SubWindow;
	if (menu.LoadMenuW(L"smenu:menu_new"))
	{
		SImageButton *pBtn = FindChildByID2<SImageButton>(R.id.btn_mulitWindow);
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen(&rc);
		menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);
	}
}

void CMainDlg::OnBtnFuncEdit()
{
	CDlgFrmlManage *pDlg = new CDlgFrmlManage(m_hWnd);
	pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
		| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
	pDlg->ShowWindow(SW_SHOW);
	::EnableWindow(m_hWnd, FALSE);
}

void CMainDlg::OnBtnMultiFilter()
{
	SMenu menu;
	m_nNewWndType = WT_FilterWindow;
	if (menu.LoadMenuW(L"smenu:menu_new"))
	{
		SImageButton *pBtn = FindChildByID2<SImageButton>(R.id.btn_mulitWindow);
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen(&rc);
		menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);
	}
}

void CMainDlg::OnBtnLimitUpStat()
{
	if (m_pDlgLUS == nullptr)
	{
		m_pDlgLUS = new CDlgLimitUpStat;
		m_pDlgLUS->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
			| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
		//m_pDlgLUS->CenterWindow(m_hWnd);
		//m_pDlgLUS->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		//	SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
		m_pDlgLUS->ShowWindow(SW_SHOW);

	}
	else
	{
		if(m_pDlgLUS->IsWindowVisible())
			m_pDlgLUS->ShowWindow(SW_HIDE);
		else
			m_pDlgLUS->ShowWindow(SW_SHOW);

	}

}

void CMainDlg::CloseSubWindows()
{
	vector<SStringA> ShowSubWindowVec;
	for (auto it : m_SubWndVec)
	{
		if (it->WindowIsValid())
			ShowSubWindowVec.emplace_back(it->GetWindowName());
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
			pDlg->ShowWindow(SW_SHOW);
			pDlg->InitWindowPos();
			m_SubWndVec.emplace_back(pDlg);
		}

	}
}

void CMainDlg::CloseFilterWindows()
{
	vector<SStringA> ShowFilterWindowVec;
	for (auto it : m_FilterWndVec)
	{
		if (it->WindowIsValid())
			ShowFilterWindowVec.emplace_back(it->GetWindowName());
		it->CSimpleWnd::DestroyWindow();
	}
	CIniFile ini(".//config//subWindow.ini");
	ini.WriteIntA("FilterWindows", "Count", ShowFilterWindowVec.size());
	SStringA str;
	for (int i = 0; i < ShowFilterWindowVec.size(); ++i)
		ini.WriteStringA("FilterWindows", str.Format("%d", i),
			ShowFilterWindowVec[i]);

}

void CMainDlg::OpenFilterWindows()
{
	CIniFile ini(".//config//subWindow.ini");
	int nWindowCount = ini.GetIntA("FilterWindows", "Count", 0);
	SStringA strTmp;
	for (int i = 0; i < nWindowCount; ++i)
	{
		SStringA strWindowName =
			ini.GetStringA("FilterWindows",
				strTmp.Format("%d", i), "");
		if (strWindowName != "")
		{
			CDlgMultiFilter * pDlg = new CDlgMultiFilter(strWindowName);

			pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
				| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
			//pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			//	SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
			//SetOtherWindowPos(pDlg, strWindowName);
			pDlg->ShowWindow(SW_SHOW);
			pDlg->InitWindowPos();
			m_FilterWndVec.emplace_back(pDlg);
		}

	}

}

void CMainDlg::CloseLimitUpWindow()
{
	CIniFile ini(".//config//subWindow.ini");
	if (m_pDlgLUS != nullptr && m_pDlgLUS->IsWindowVisible())
	{
		m_pDlgLUS->CloseWnd();
		ini.WriteIntA("LimitWindow", "Show", TRUE);
	}
	else
	{
		ini.WriteIntA("LimitWindow", "Show", FALSE);
	}

}

void CMainDlg::OpenLimitUpWindow()
{
	CIniFile ini(".//config//subWindow.ini");
	BOOL bShow = ini.GetIntA("LimitWindow", "Show", FALSE);
	if (bShow)
	{
		m_pDlgLUS = new CDlgLimitUpStat;
		m_pDlgLUS->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
			| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
		m_pDlgLUS->ShowWindow(SW_SHOW);
		m_pDlgLUS->InitWindowPos();
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
	pDlg->ShowWindow(SW_SHOW);
	m_SubWndVec.emplace_back(pDlg);
	//SwitchToThisWindow(pDlg->m_hWnd, FALSE);
}

void CMainDlg::OpenFilterWindowWithNum(int nWndNum)
{
	CIniFile ini(".//config//subWindow.ini");
	SStringA str;
	SStringA strName = ini.GetStringA("FilterWindowName",
		str.Format("%d", nWndNum), "");
	CDlgMultiFilter * pDlg = new CDlgMultiFilter(strName);
	pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
		| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
	pDlg->ShowWindow(SW_SHOW);
	m_FilterWndVec.emplace_back(pDlg);
}


void CMainDlg::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	switch (nID)
	{
	case WDM_New:
	{
		CDlgAddNew * pDlg = new CDlgAddNew(m_hWnd,m_nNewWndType);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case WDM_Open:
	{
		CDlgOpenWnd * pDlg = new CDlgOpenWnd(m_hWnd, m_nNewWndType);
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
	SFPlan sfPlan;
	m_WndMap[nGroup]->OutputStockFilterPara(sfPlan);
	SStringA strPath;
	strPath.Format(".//filter//Main_SF_%d.sfl", nGroup);
	if (!sfPlan.condVec.empty())
	{
		ofstream ofile(strPath, std::ios::binary);
		if (ofile.is_open())
		{
			CDlgStockFilter::SaveConditonsList(ofile, sfPlan);
			ofile.close();
		}
	}
	else
		DeleteFileA(strPath);

}

void CMainDlg::SaveComboStockFilterPara(int nGroup)
{
	vector<StockFilter> sfVec;
	m_WndMap[nGroup]->OutputComboStockFilterPara(sfVec);
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

	vector<HisStockFilter> hsfVec;
	m_WndMap[nGroup]->OutputComboHisStockFilterPara(hsfVec);
	strPath.Format(".//config//Main_HSF_%d.DAT", nGroup);
	if (!hsfVec.empty())
	{
		ofstream ofile(strPath, std::ios::binary);
		if (ofile.is_open())
		{
			int paraSize = sizeof(HisStockFilter);
			ofile.write((char*)&paraSize, sizeof(paraSize));
			ofile.write((char*)&hsfVec[0], paraSize * hsfVec.size());
			ofile.close();
		}
	}
	else
		DeleteFileA(strPath);

}

void CMainDlg::ShowUpdateLog()
{
	CIniFile ini(".//config//UpdateLog.ini");
	BOOL bShowLog = ini.GetIntA("UpdateLog", g_strUpdateDate, 0);
	if (!bShowLog)
	{
		ini.WriteIntA("UpdateLog", g_strUpdateDate, 1);
		CDlgUpdateLog* pDlg = new CDlgUpdateLog;
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
}

void CMainDlg::InitWindowPos()
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strFileName = ".\\config\\MainWnd.position";
	ifile.open(strFileName, std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
		ifile.close();
	}
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
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	auto pFilterData = g_WndSyn.GetFilterData();
	auto pStockPos = g_WndSyn.GetStockPos();
	strHash<double> preCloseMap(g_WndSyn.GetCloseMap());
	auto infoMap = g_WndSyn.GetPointInfo();
	vector<strHash<CAInfo>>* pCallAction = g_WndSyn.GetCallActionData();
	InitConfig(infoMap);
	//2023.5.4 修改将新版的选股器函数调用改为老版本
	//InitStockFilter();
	InitComboStockFilter();


	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndHandleMap[m_WndMap[i]->m_hWnd] = i;
		g_WndSyn.SetSubWndInfo(m_WndMap[i]->m_hWnd, m_hWnd, i);
		m_WndMap[i]->SetParThreadID(m_DataThreadID);
		m_WndMap[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndMap[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		m_WndMap[i]->SetDataPoint(&pFilterData->at(i), DT_FilterData);
		m_WndMap[i]->SetPointInfo(infoMap);
		m_WndMap[i]->SetDataPoint(&pCallAction->at(i), DT_CallAction);
		m_WndMap[i]->SetDataPoint(&pStockPos->at(i), DT_StockPos);

		//添加上级行业的选股器数据
		for (int j = Group_SWL1; j < i; ++j)
			m_WndMap[i]->SetDataPoint(&pFilterData->at(j), DT_L1IndyFilterData + j);

		for (int j = Group_SWL1; j < i; ++j)
			m_WndMap[i]->SetDataPoint(&pStockPos->at(j), DT_L1IndyIndexPos + j);

		if (i == Group_Stock)
		{
			map<int, strHash<TickFlowMarket>> *pTFMarket = g_WndSyn.GetTFMarket();
			m_WndMap[i]->SetDataPoint(pTFMarket, DT_TFMarket);
		}
		m_WndMap[i]->SetPreClose(preCloseMap);
		m_WndMap[i]->InitList();
	}
	InitListConfig();

}

void CMainDlg::InitStockFilter()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		SFPlan sfPlan;
		SStringA strPath;
		strPath.Format(".//filter//Main_SF_%d.sfl", i);
		ifstream ifile(strPath, std::ios::_Nocreate);
		if (ifile.is_open())
		{
			CDlgStockFilter::ReadConditonsList(ifile, sfPlan);
			m_WndMap[i]->InitStockFilterPara(sfPlan);
			ifile.close();
		}

	}

}

void CMainDlg::InitComboStockFilter()
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
			m_WndMap[i]->InitComboStockFilterPara(sfVec);
			ifile.close();
		}

		vector<HisStockFilter> hsfVec;
		strPath.Format(".//config//Main_HSF_%d.DAT", i);
		ifile.open(strPath, std::ios::binary | std::ios::_Nocreate);
		if (ifile.is_open())
		{
			int nSFParaSize = 0;
			ifile.read((char*)&nSFParaSize, sizeof(nSFParaSize));
			HisStockFilter hsfPara = { 0 };
			while (ifile.read((char*)&hsfPara, nSFParaSize))
				hsfVec.emplace_back(hsfPara);
			m_WndMap[i]->InitComboHisStockFilterPara(hsfVec);
			ifile.close();
		}

	}

}

void CMainDlg::InitPointWndInfo(CIniFile& ini, InitPara & initPara, SStringA strSection, map<int, ShowPointInfo> &pointMap)
{
	vector<SStringA> strRangeVec;
	strRangeVec.emplace_back("");
	strRangeVec.emplace_back("L1");
	strRangeVec.emplace_back("L2");
	vector<SStringA> strShowNameVec;
	strShowNameVec.emplace_back("");
	strShowNameVec.emplace_back("一级行业");
	strShowNameVec.emplace_back("二级行业");
	initPara.nTSCPointWndNum = ini.GetIntA(strSection, "TSCPointWndNum", -1);
	if (initPara.nTSCPointWndNum == -1)
	{
		int nCount = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (initPara.bShowTSCRPS[i])
			{
				++nCount;
				if (i == 0)
					initPara.TSCPonitWndInfo.emplace_back(pointMap[eRpsPoint_Close]);
				else
					initPara.TSCPonitWndInfo.emplace_back(pointMap[eRpsPoint_L1_Close + i]);
			}
		}
		initPara.nTSCPointWndNum = nCount;
	}
	else if (initPara.nTSCPointWndNum > 0)
	{
		for (int i = 0; i < initPara.nTSCPointWndNum; ++i)
		{
			SStringA tmp;
			int overallType = ini.GetIntA(strSection, tmp.Format("TSCPoint%dOverallType", i), -1);
			if (overallType == -1)
			{
				int type = (ePointType)ini.GetIntA(strSection, tmp.Format("TSCPoint%dType", i), 0);
				SStringA srcDataName = ini.GetStringA(strSection, tmp.Format("TSCPoint%dSrcName", i), "");
				SStringA dataInRange = ini.GetStringA(strSection, tmp.Format("TSCPoint%dRange", i), "");
				for (auto &it : pointMap)
				{
					if (it.first > eIndyMarketPointEnd)
						continue;
					if (it.second.type == type &&
						it.second.dataInRange == dataInRange &&
						it.second.srcDataName == srcDataName)
					{
						initPara.TSCPonitWndInfo.emplace_back(it.second);
						break;
					}
				}
			}
			else
				initPara.TSCPonitWndInfo.emplace_back(pointMap[overallType]);
		}
	}
	initPara.nKlinePointWndNum = ini.GetIntA(strSection, "KlinePointWndNum", -1);
	if (initPara.nKlinePointWndNum == -1)
	{
		int nCount = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (initPara.bShowKlineRPS[i])
			{
				++nCount;
				++nCount;
				if (i == 0)
					initPara.KlinePonitWndInfo.emplace_back(pointMap[eRpsPoint_Close]);
				else
					initPara.KlinePonitWndInfo.emplace_back(pointMap[eRpsPoint_L1_Close + i]);
			}
		}
		initPara.nKlinePointWndNum = nCount;
	}
	else if (initPara.nKlinePointWndNum > 0)
	{
		for (int i = 0; i < initPara.nKlinePointWndNum; ++i)
		{
			SStringA tmp;
			int overallType = ini.GetIntA(strSection, tmp.Format("KlinePoint%dOverallType", i), -1);
			if (overallType == -1)
			{
				int type = (ePointType)ini.GetIntA(strSection, tmp.Format("KlinePoint%dType", i), 0);
				SStringA srcDataName = ini.GetStringA(strSection, tmp.Format("KlinePoint%dSrcName", i), "");
				SStringA dataInRange = ini.GetStringA(strSection, tmp.Format("KlinePoint%dRange", i), "");
				for (auto &it : pointMap)
				{
					if (it.first > eIndyMarketPointEnd)
						continue;
					if (it.second.type == type &&
						it.second.dataInRange == dataInRange &&
						it.second.srcDataName == srcDataName)
					{
						initPara.KlinePonitWndInfo.emplace_back(it.second);
						break;
					}
				}
			}
			else
				initPara.KlinePonitWndInfo.emplace_back(pointMap[overallType]);
		}
	}

}


void CMainDlg::InitConfig(map<int, ShowPointInfo> &pointMap)
{
	CIniFile ini(".//config//WindowConfig.ini");
	int CloseMAPara[] = { 5,10,20,60,0,0 };
	int VolAmoMAPara[] = { 5,10,0,0,0,0 };


	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		InitPara initPara;
		SStringA strSection;
		strSection.Format("Group%d", i);
		GetInitPara(ini, initPara, strSection);

		InitPointWndInfo(ini, initPara, strSection, pointMap);
		m_WndMap[i]->InitShowConfig(initPara);
	}


}

void CMainDlg::InitListConfig()
{
	CIniFile ini(".//config//ListConfig.ini");
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		map<int, BOOL>showTitleMap;
		map<int, int>titleOrderMap;
		SStringA strSection;
		strSection.Format("Group%d", i);
		int nShowItemCount = ini.GetIntA(strSection, "ShowItemCount", 0);
		if (nShowItemCount == 0)
		{
			if (i != Group_Stock)
			{
				for (int j = SHead_CloseRPS520; j < SHead_CommonItmeCount; ++j)
					showTitleMap[j] = TRUE;
				for (int j = SHead_ID; j < SHead_CommonItmeCount; ++j)
					titleOrderMap[j] = j;

			}
			else
			{
				for (int j = SHead_CloseRPS520; j < SHead_StockItemCount; ++j)
					showTitleMap[j] = TRUE;
				for (int j = SHead_ID; j < SHead_StockItemCount; ++j)
					titleOrderMap[j] = j;
			}
		}
		else
		{
			int nTotalItemCount = Group_Stock ==
				i ? SHead_StockItemCount : SHead_CommonItmeCount;
			if (nShowItemCount + SHead_CloseRPS520 < nTotalItemCount)
			{
				SStringA strKey;
				for (int j = 0; j < nShowItemCount; ++j)
				{
					int nIndex = j + SHead_CloseRPS520;
					if (nIndex >= g_nListNewItemStart)
						nIndex += (g_nListNewItemEnd - g_nListNewItemStart + 1);
					showTitleMap[nIndex] =
						ini.GetIntA(strSection, strKey.Format("Show%d", j + SHead_CloseRPS520), TRUE);
				}
				//默认新添加的都显示
				for (int j = g_nListNewItemStart; j <= g_nListNewItemEnd; ++j)
					showTitleMap[j] = TRUE;

				for (int j = 0; j < nShowItemCount + SHead_CloseRPS520; ++j)
				{
					int nIndex = j;
					if (nIndex >= g_nListNewItemStart)
						nIndex += (g_nListNewItemEnd - g_nListNewItemStart + 1);
					int nOrderShow = ini.GetIntA(strSection, strKey.Format("Order%d", j), j);
					if (nOrderShow >= g_nListNewItemStart)
						nOrderShow += (g_nListNewItemEnd - g_nListNewItemStart + 1);
					titleOrderMap[nIndex] = nOrderShow;

				}
				//默认新添加的都在原有位置
				for (int j = g_nListNewItemStart; j <= g_nListNewItemEnd; ++j)
					titleOrderMap[j] = j;

			}
			else
			{
				SStringA strKey;
				for (int j = 0; j < nShowItemCount; ++j)
					showTitleMap[j + SHead_CloseRPS520] =
					ini.GetIntA(strSection, strKey.Format("Show%d", j + SHead_CloseRPS520), TRUE);
				for (int j = 0; j < nShowItemCount + SHead_CloseRPS520; ++j)
					titleOrderMap[j] = ini.GetIntA(strSection, strKey.Format("Order%d", j), j);
			}
		}

		m_WndMap[i]->InitListConfig(showTitleMap, titleOrderMap);
	}

}

void CMainDlg::SavePointWndInfo(CIniFile & ini, InitPara & initPara, SStringA strSection)
{
	ini.WriteIntA(strSection, "TSCPointWndNum", initPara.nTSCPointWndNum);
	for (int i = 0; i < initPara.nTSCPointWndNum; ++i)
	{
		auto& spi = initPara.TSCPonitWndInfo[i];
		SStringA tmp;
		ini.WriteIntA(strSection, tmp.Format("TSCPoint%dOverallType", i), spi.overallType);
	}
	ini.WriteIntA(strSection, "KlinePointWndNum", initPara.nKlinePointWndNum);
	for (int i = 0; i < initPara.nKlinePointWndNum; ++i)
	{
		auto& spi = initPara.KlinePonitWndInfo[i];
		SStringA tmp;
		ini.WriteIntA(strSection, tmp.Format("KlinePoint%dOverallType", i), spi.overallType);
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
		SaveInitPara(ini, initPara,strSection);
		SavePointWndInfo(ini, initPara, strSection);
	}

}

void CMainDlg::SaveListConfig()
{
	CIniFile ini(".//config//ListConfig.ini");

	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		map<int, BOOL>showTitleMap = m_WndMap[i]->GetListShowTitle();
		map<int, int>titleOrderMap = m_WndMap[i]->GetListTitleOrder();
		SStringA strSection;
		strSection.Format("Group%d", i);
		ini.WriteIntA(strSection, "ShowItemCount", showTitleMap.size());
		SStringA strKey;
		for (auto &it : showTitleMap)
			ini.WriteIntA(strSection, strKey.Format("Show%d", it.first), it.second);
		for (auto &it : titleOrderMap)
			ini.WriteIntA(strSection, strKey.Format("Order%d", it.first), it.second);

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
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
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
	m_MsgHandleMap[WW_GetCallAction]
		= &CMainDlg::OnGetCallAction;

	m_MsgHandleMap[Syn_Point]
		= &CMainDlg::OnUpdatePoint;
	//m_MsgHandleMap[Syn_TodayPoint]
	//	= &CMainDlg::OnTodayPoint;
	m_MsgHandleMap[Syn_HisRpsPoint]
		= &CMainDlg::OnHisRpsPoint;
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
	m_MsgHandleMap[Syn_HisSecPoint]
		= &CMainDlg::OnHisSecPoint;
	m_MsgHandleMap[Syn_RehabInfo]
		= &CMainDlg::OnRehabInfo;
	m_MsgHandleMap[Syn_HisCallAction]
		= &CMainDlg::OnHisCallAction;

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
	m_WndPointSubMap.clear();
	SendMsg(m_SynThreadID, Syn_GetMarket, info, nMsgLength);
}

void CMainDlg::OnGetKline(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetKline, info, nMsgLength);
}

void CMainDlg::OnGetPoint(int nMsgLength, const char * info)
{
	ExDataGetInfo *pDgInfo = (ExDataGetInfo *)info;
	m_WndPointSubMap[pDgInfo->nAskGroup][pDgInfo->StockID].emplace_back(*pDgInfo);
	SendMsg(m_SynThreadID, Syn_GetPoint, info, nMsgLength);
}

void CMainDlg::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_Point,
		info + 4, nMsgLength - 4);
}

//void CMainDlg::OnTodayPoint(int nMsgLength, const char * info)
//{
//	int nGroup = *(int*)info;
//	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_TodayPoint,
//		info + 4, nMsgLength - 4);
//}

void CMainDlg::OnHisRpsPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisRpsPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
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

	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisIndexMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CMainDlg::OnHisStockMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisStockMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CMainDlg::OnHisKline(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisKline,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
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

void CMainDlg::OnHisSecPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisSecPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CMainDlg::OnRehabInfo(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_RehabInfo,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CMainDlg::OnHisCallAction(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisCallAction,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CMainDlg::OnGetCallAction(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetCallAction, info, nMsgLength);
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

void CMainDlg::OnBtnHelp()
{
	ShellExecute(nullptr, _T("open"), _T(".\\相对强度rps客户端使用说明.pdf"), _T(""), _T(""), SW_SHOW);

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

