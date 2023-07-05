#include "stdafx.h"
#include "DlgMultiFilter.h"
#include <fstream>
#include "WndSynHandler.h"
#include <helper/SMenu.h>
#define TIMER_AUTOSAVE 1

extern CWndSynHandler g_WndSyn;
extern HWND g_MainWnd;

const int MAX_WNDNUM = 6;
const int DEFAULT_WNDNUM = 4;

CDlgMultiFilter::CDlgMultiFilter(SStringA strWndName) :SHostWnd(_T("LAYOUT:dlg_multiFilter"))
{
	m_bLayoutInited = FALSE;
	m_strWindowName = strWndName;
	m_bIsValid = TRUE;
	m_nShowWndNum = MAX_WNDNUM;

}


CDlgMultiFilter::~CDlgMultiFilter()
{
	if (INVALID_THREADID != m_DataThreadID)
		SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();

}


void CDlgMultiFilter::OnClose()
{
	if (IsVisible())
	{
		ShowWindow(SW_HIDE);
		m_bIsValid = FALSE;
		KillTimer(TIMER_AUTOSAVE);
		SendMsg(m_SynThreadID, Syn_RemoveWnd, (char*)&m_hWnd, sizeof(m_hWnd));
		SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
		if (tDataProc.joinable())
			tDataProc.join();
		m_DataThreadID = INVALID_THREADID;
		SavePicConfig();
		WINDOWPLACEMENT wp = { sizeof(wp) };
		::GetWindowPlacement(m_hWnd, &wp);
		std::ofstream ofile;
		SStringA strPosFile;
		strPosFile.Format(".\\config\\%s.position", m_strWindowName);
		ofile.open(strPosFile, std::ios::out | std::ios::binary);
		if (ofile.is_open())
			ofile.write((char*)&wp, sizeof(wp));
		ofile.close();
		for (auto &it : m_WndVec)
			it->CloseWnd();
	}

}

int CDlgMultiFilter::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CDlgMultiFilter::OnInitDialog(EventArgs* e)
{
	m_bLayoutInited = TRUE;
	SStatic *pTitl = FindChildByID2<SStatic>(R.id.text_windowName);
	pTitl->SetWindowTextW(StrA2StrW(m_strWindowName));
	CSimpleWnd::SetWindowTextW(StrA2StrW(m_strWindowName));
	m_SynThreadID = g_WndSyn.GetThreadID();
	tDataProc = thread(&CDlgMultiFilter::MsgProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	InitWindowPos();
	InitMsgHandleMap();
	InitWorkWnd();
	g_WndSyn.AddWnd(m_hWnd, m_DataThreadID);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CDlgMultiFilter::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
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

void CDlgMultiFilter::OnDestroy()
{

	SetMsgHandled(FALSE);
	SavePicConfig();
	SaveListConfig();
	WINDOWPLACEMENT wp = { sizeof(wp) };
	::GetWindowPlacement(m_hWnd, &wp);
	std::ofstream ofile;
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.position", m_strWindowName);
	ofile.open(strPosFile, std::ios::out | std::ios::binary);
	if (ofile.is_open())
		ofile.write((char*)&wp, sizeof(wp));
	ofile.close();
}

void CDlgMultiFilter::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == TIMER_AUTOSAVE)
		SavePicConfig();
}

void CDlgMultiFilter::InitWindowPos()
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strFileName;
	strFileName.Format(".\\config\\%s.position", m_strWindowName);
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

void CDlgMultiFilter::InitWorkWnd()
{
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	vector<map<int, strHash<unordered_map<string, double>>>>* pFilterData =
		g_WndSyn.GetFilterData();
	strHash<double> preCloseMap(g_WndSyn.GetCloseMap());
	auto infoMap = g_WndSyn.GetPointInfo();
	vector<strHash<CAInfo>>* pCallAction = g_WndSyn.GetCallActionData();

	SStringW strWndName;
	m_WndVec.resize(MAX_WNDNUM);
	m_SrcWndVec.resize(MAX_WNDNUM);
	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		strWndName.Format(L"wnd%d", i + 1);
		SRealWnd * pRealWnd = FindChildByName2<SRealWnd>(strWndName);
		m_SrcWndVec[i] = pRealWnd;
		m_WndVec[i] = (CWorkWnd *)pRealWnd->GetData();
		m_WndVec[i]->SetFilterWndNum(i);
		m_WndVec[i]->SetGroup(Group_Stock, m_hWnd);

	}
	InitConfig(infoMap);
	InitComboStockFilter();
	SetShowWndNum(m_nShowWndNum, TRUE);

	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		m_WndHandleMap[m_WndVec[i]->m_hWnd] = i;
		g_WndSyn.SetSubWndInfo(m_WndVec[i]->m_hWnd, m_hWnd, Group_Stock);
		m_WndVec[i]->SetParThreadID(m_DataThreadID);
		m_WndVec[i]->SetListInfo(ListInsVec[Group_Stock], StockName);
		m_WndVec[i]->SetDataPoint(&pListData->at(Group_Stock), DT_ListData);
		m_WndVec[i]->SetDataPoint(&pFilterData->at(Group_Stock), DT_FilterData);
		m_WndVec[i]->SetPointInfo(infoMap);
		m_WndVec[i]->SetDataPoint(&pCallAction->at(Group_Stock), DT_CallAction);
		for (int j = Group_SWL1; j < Group_Stock; ++j)
			m_WndVec[i]->SetDataPoint(&pFilterData->at(j), DT_L1IndyFilterData + j);
		map<int, strHash<TickFlowMarket>> *pTFMarket = g_WndSyn.GetTFMarket();
		m_WndVec[i]->SetDataPoint(pTFMarket, DT_TFMarket);
		m_WndVec[i]->SetPreClose(preCloseMap);
		m_WndVec[i]->InitList();
	}
	InitListConfig();
}


void CDlgMultiFilter::InitComboStockFilter()
{
	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		vector<StockFilter> sfVec;
		SStringA strPath;
		strPath.Format(".//config//%s_SF_%d.DAT", m_strWindowName, i);
		std::ifstream ifile(strPath, std::ios::binary | std::ios::_Nocreate);
		if (ifile.is_open())
		{
			int nSFParaSize = 0;
			ifile.read((char*)&nSFParaSize, sizeof(nSFParaSize));
			StockFilter sfPara = { 0 };
			while (ifile.read((char*)&sfPara, nSFParaSize))
				sfVec.emplace_back(sfPara);
			m_WndVec[i]->InitComboStockFilterPara(sfVec);
		}
	}

}

void CDlgMultiFilter::InitPointWndInfo(CIniFile & ini, InitPara & initPara, SStringA strSection, map<int, ShowPointInfo> &pointMap)
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


void CDlgMultiFilter::InitConfig(map<int, ShowPointInfo> &pointMap)
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.ini", m_strWindowName);
	CIniFile ini(strPosFile);
	int CloseMAPara[] = { 5,10,20,60,0,0 };
	int VolAmoMAPara[] = { 5,10,0,0,0,0 };

	m_nShowWndNum = ini.GetIntA("Overall", "WndNum", DEFAULT_WNDNUM);
	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		InitPara initPara;
		SStringA strSection;
		SStringA strKey;
		strSection.Format("Wnd%d", i);
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
		initPara.bShowKlineAmount =
			ini.GetIntA(strSection, "ShowKlineAmount", 0) == 0 ? false : true;
		initPara.bShowKlineCAVol =
			ini.GetIntA(strSection, "ShowKlineCAVol", 0) == 0 ? false : true;
		initPara.bShowKlineCAAmo =
			ini.GetIntA(strSection, "ShowKlineCAAmo", 0) == 0 ? false : true;
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
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			initPara.nMAPara[i] = ini.GetIntA(strSection,
				strKey.Format("MAPara%d", i + 1), CloseMAPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			initPara.nVolMaPara[i] = ini.GetIntA(strSection,
				strKey.Format("VolMAPara%d", i + 1), VolAmoMAPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			initPara.nAmoMaPara[i] = ini.GetIntA(strSection,
				strKey.Format("AmoMAPara%d", i + 1), VolAmoMAPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			initPara.nCAVolMaPara[i] = ini.GetIntA(strSection,
				strKey.Format("CAVolMAPara%d", i + 1), VolAmoMAPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			initPara.nCAAmoMaPara[i] = ini.GetIntA(strSection,
				strKey.Format("CAAmoMAPara%d", i + 1), VolAmoMAPara[i]);

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
		initPara.nKlineRehabType = ini.GetIntA(strSection, "KlineRehabType", 0);
		initPara.nKlineCalcRehabType = ini.GetIntA(strSection, "KlineCalcRehabType", 0);
		initPara.nKlineFTRehabDate = ini.GetIntA(strSection, "KlineFTRehabDate", 0);

		InitPointWndInfo(ini, initPara, strSection, pointMap);

		m_WndVec[i]->InitShowConfig(initPara);
	}


}

void CDlgMultiFilter::InitListConfig()
{
	SStringA strFile;
	strFile.Format(".\\config\\ListConfig_%s.ini", m_strWindowName);
	CIniFile ini(strFile);

	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		map<int, BOOL>showTitleMap;
		map<int, int>titleOrderMap;
		SStringA strSection;
		strSection.Format("Wnd%d", i);
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

		m_WndVec[i]->InitListConfig(showTitleMap, titleOrderMap);
	}

}

void CDlgMultiFilter::SavePointWndInfo(CIniFile & ini, InitPara & initPara, SStringA strSection)
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




void CDlgMultiFilter::SavePicConfig()
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.ini", m_strWindowName);
	CIniFile ini(strPosFile);
	ini.WriteIntA("Overall", "WndNum", m_nShowWndNum);

	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		auto initPara = m_WndVec[i]->OutPutInitPara();
		SStringA strSection;
		SStringA strKey;
		strSection.Format("Wnd%d", i);
		ini.WriteIntA(strSection, "ShowMA", initPara.bShowMA);
		ini.WriteIntA(strSection, "ShowBand", initPara.bShowBandTarget);
		ini.WriteIntA(strSection, "ShowAvg", initPara.bShowAverage);
		ini.WriteIntA(strSection, "ShowEMA", initPara.bShowEMA);
		ini.WriteIntA(strSection, "ShowTSCMACD", initPara.bShowTSCMACD);
		ini.WriteIntA(strSection, "ShowTSCVolume", initPara.bShowTSCVolume);
		ini.WriteIntA(strSection, "ShowKlineVolume", initPara.bShowKlineVolume);
		ini.WriteIntA(strSection, "ShowKlineAmount", initPara.bShowKlineAmount);
		ini.WriteIntA(strSection, "ShowKlineCAVol", initPara.bShowKlineCAVol);
		ini.WriteIntA(strSection, "ShowKlineCAAmo", initPara.bShowKlineCAAmo);
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
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			ini.WriteIntA(strSection, strKey.Format("MAPara%d", i + 1), initPara.nMAPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			ini.WriteIntA(strSection, strKey.Format("VolMAPara%d", i + 1), initPara.nVolMaPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			ini.WriteIntA(strSection, strKey.Format("AmoMAPara%d", i + 1), initPara.nAmoMaPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			ini.WriteIntA(strSection, strKey.Format("CAVolMAPara%d", i + 1), initPara.nCAVolMaPara[i]);
		for (int i = 0; i < MAX_MA_COUNT; ++i)
			ini.WriteIntA(strSection, strKey.Format("CAAmoMAPara%d", i + 1), initPara.nCAAmoMaPara[i]);

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
		ini.WriteIntA(strSection, "KlineRehabType", initPara.nKlineRehabType);
		ini.WriteIntA(strSection, "KlineCalcRehabType", initPara.nKlineCalcRehabType);
		ini.WriteIntA(strSection, "KlineFTRehabDate", initPara.nKlineFTRehabDate);

		SavePointWndInfo(ini, initPara, strSection);
	}

}

void SOUI::CDlgMultiFilter::SaveListConfig()
{
	SStringA strFile;
	strFile.Format(".\\config\\ListConfig_%s.ini", m_strWindowName);
	CIniFile ini(strFile);

	for (int i = 0; i < MAX_WNDNUM; ++i)
	{
		map<int, BOOL>showTitleMap = m_WndVec[i]->GetListShowTitle();
		map<int, int>titleOrderMap = m_WndVec[i]->GetListTitleOrder();
		SStringA strSection;
		strSection.Format("Wnd%d", i);
		ini.WriteIntA(strSection, "ShowItemCount", showTitleMap.size());
		SStringA strKey;
		for (auto &it : showTitleMap)
			ini.WriteIntA(strSection, strKey.Format("Show%d", it.first), it.second);
		for (auto &it : titleOrderMap)
			ini.WriteIntA(strSection, strKey.Format("Order%d", it.first), it.second);

	}

}


void CDlgMultiFilter::ReInit()
{
	ShowWindow(SW_HIDE);
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndVec[i]->SetPicUnHandled();
		m_WndVec[i]->ClearData();
	}
	ReInitWorkWnd();
	ShowWindow(SW_SHOW);
	//SwitchToThisWindow(m_hWnd, FALSE);

}


void CDlgMultiFilter::ReInitList()
{
	for (int i = 0; i < MAX_WNDNUM; ++i)
		m_WndVec[i]->ReInitList();
}

void CDlgMultiFilter::ReInitWorkWnd()
{
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndVec[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndVec[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		m_WndVec[i]->ReInitList();
		m_WndVec[i]->ReSetPic();
	}
}

void CDlgMultiFilter::InitMsgHandleMap()
{
	m_MsgHandleMap[WW_ListData]
		= &CDlgMultiFilter::OnUpdateList;
	m_MsgHandleMap[WW_GetMarket]
		= &CDlgMultiFilter::OnGetMarket;
	m_MsgHandleMap[WW_GetKline]
		= &CDlgMultiFilter::OnGetKline;
	m_MsgHandleMap[WW_GetPoint]
		= &CDlgMultiFilter::OnGetPoint;
	m_MsgHandleMap[WW_GetCallAction]
		= &CDlgMultiFilter::OnGetCallAction;

	m_MsgHandleMap[Syn_Point]
		= &CDlgMultiFilter::OnUpdatePoint;
	//m_MsgHandleMap[Syn_TodayPoint]
	//	= &CDlgMultiFilter::OnTodayPoint;
	m_MsgHandleMap[Syn_HisRpsPoint]
		= &CDlgMultiFilter::OnHisRpsPoint;
	m_MsgHandleMap[Syn_RTStockMarket]
		= &CDlgMultiFilter::OnRTStockMarket;
	m_MsgHandleMap[Syn_HisStockMarket]
		= &CDlgMultiFilter::OnHisStockMarket;
	m_MsgHandleMap[Syn_HisKline]
		= &CDlgMultiFilter::OnHisKline;
	m_MsgHandleMap[Syn_CloseInfo]
		= &CDlgMultiFilter::OnCloseInfo;
	m_MsgHandleMap[Syn_HisSecPoint]
		= &CDlgMultiFilter::OnHisSecPoint;
	m_MsgHandleMap[Syn_RehabInfo]
		= &CDlgMultiFilter::OnRehabInfo;
	m_MsgHandleMap[Syn_HisCallAction]
		= &CDlgMultiFilter::OnHisCallAction;

}

void CDlgMultiFilter::MsgProc()
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

void CDlgMultiFilter::OnUpdateList(int nMsgLength, const char * info)
{
	for (auto &it : m_WndVec)
		SendMsg(it->GetThreadID(), WW_ListData,
			info, nMsgLength);

}

void CDlgMultiFilter::OnGetMarket(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	m_WndSubMap[pDgInfo->Group] = pDgInfo->StockID;
	SendMsg(m_SynThreadID, Syn_GetMarket, info, nMsgLength);
}

void CDlgMultiFilter::OnGetKline(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetKline, info, nMsgLength);
}

void CDlgMultiFilter::OnGetPoint(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetPoint, info, nMsgLength);
}

void CDlgMultiFilter::OnUpdatePoint(int nMsgLength, const char * info)
{
	for(int i=0;i<MAX_WNDNUM;++i)
	SendMsg(m_WndVec[i]->GetThreadID(), WW_Point,
		info + 4, nMsgLength - 4);
}

//void CDlgMultiFilter::OnTodayPoint(int nMsgLength, const char * info)
//{
//	int nGroup = *(int*)info;
//	SendMsg(m_WndVec[nGroup]->GetThreadID(), WW_TodayPoint,
//		info + 4, nMsgLength - 4);
//}

void CDlgMultiFilter::OnHisRpsPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisRpsPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}


void CDlgMultiFilter::OnRTStockMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	for(int i = 0;i<MAX_WNDNUM;++i)
	if (m_WndSubMap[i] == strStock)
		SendMsg(m_WndVec[i]->GetThreadID(), WW_RTStockMarket,
			info, nMsgLength);
}


void CDlgMultiFilter::OnHisStockMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisStockMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnHisKline(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisKline,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnCloseInfo(int nMsgLength, const char * info)
{
	for (int i = 0; i < Group_Count; ++i)
		SendMsg(m_WndVec[i]->GetThreadID(), WW_CloseInfo,
			info, nMsgLength);
}


void CDlgMultiFilter::OnHisSecPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisSecPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnRehabInfo(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_RehabInfo,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnHisCallAction(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisCallAction,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnGetCallAction(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetCallAction, info, nMsgLength);
}

void CDlgMultiFilter::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	SetShowWndNum(nID - FWM_Start + 1,FALSE);
}

void CDlgMultiFilter::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

//TODO:消息映射
void CDlgMultiFilter::OnBtnClose()
{
	GetNative()->SendMessage(WM_CLOSE);
}

void SOUI::CDlgMultiFilter::OnBtnSetWndNum()
{
	SMenu menu;
	if (menu.LoadMenuW(L"smenu:menu_filterWndNum"))
	{
		SImageButton *pBtn = FindChildByName2<SImageButton>(L"btn_wndNum");
		menu.CheckMenuItem(FWM_Start + m_nShowWndNum -1, MF_CHECKED);
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen(&rc);
		menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);
	}

}


void SOUI::CDlgMultiFilter::SaveComboStockFilterPara(int nWndNum)
{
	vector<StockFilter> sfVec;
	m_WndVec[nWndNum]->OutputComboStockFilterPara(sfVec);
	SStringA strPath;
	strPath.Format(".//config//%s_SF_%d.DAT", m_strWindowName, nWndNum);
	if (!sfVec.empty())
	{
		std::ofstream ofile(strPath, std::ios::binary);
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


BOOL CDlgMultiFilter::SetShowWndNum(int nNum, BOOL bFromInit)
{
	
	if (nNum == m_nShowWndNum && !bFromInit)
		return FALSE;

	if (1 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,-5,-5");
		m_WndVec[0]->SetPriceListHalf(false);
	}
	else if (2 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,%50,-5");
		m_SrcWndVec[1]->SetAttribute(L"pos", L"%50,5,-5,-5");
		for (int i = 0; i < nNum; ++i)
			m_WndVec[i]->SetPriceListHalf(false);
	}
	else if (3 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,%33,-5");
		m_SrcWndVec[1]->SetAttribute(L"pos", L"%33,5,%67,-5");
		m_SrcWndVec[2]->SetAttribute(L"pos", L"%67,5,-5,-5");
		for (int i = 0; i < nNum; ++i)
			m_WndVec[i]->SetPriceListHalf(false);
	}
	else if (4 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,%50,%50");
		m_SrcWndVec[1]->SetAttribute(L"pos", L"%50,5,-5,%50");
		m_SrcWndVec[2]->SetAttribute(L"pos", L"5,%50,%50,-5");
		m_SrcWndVec[3]->SetAttribute(L"pos", L"%50,%50,-5,-5");
		for (int i = 0; i < nNum; ++i)
			m_WndVec[i]->SetPriceListHalf(true);
	}
	else if (5 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,%50,%50");
		m_SrcWndVec[1]->SetAttribute(L"pos", L"%50,5,-5,%50");
		m_SrcWndVec[2]->SetAttribute(L"pos", L"5,%50,%33,-5");
		m_SrcWndVec[3]->SetAttribute(L"pos", L"%33,%50,%67,-5");
		m_SrcWndVec[4]->SetAttribute(L"pos", L"%67,%50,-5,-5");
		for (int i = 0; i < nNum; ++i)
			m_WndVec[i]->SetPriceListHalf(true);
	}
	else if (6 == nNum)
	{
		m_SrcWndVec[0]->SetAttribute(L"pos", L"5,5,%33,%50");
		m_SrcWndVec[1]->SetAttribute(L"pos", L"%33,5,%67,%50");
		m_SrcWndVec[2]->SetAttribute(L"pos", L"%67,5,-5,%50");
		m_SrcWndVec[3]->SetAttribute(L"pos", L"5,%50,%33,-5");
		m_SrcWndVec[4]->SetAttribute(L"pos", L"%33,%50,%67,-5");
		m_SrcWndVec[5]->SetAttribute(L"pos", L"%67,%50,-5,-5");
		for (int i = 0; i < nNum; ++i)
			m_WndVec[i]->SetPriceListHalf(true);

	}
	for (int i = 0; i < nNum; ++i)
	{
		m_SrcWndVec[i]->SetVisible(TRUE, TRUE);
		m_WndVec[i]->SetVisible(TRUE, TRUE);
	}
	for (int i = nNum; i < MAX_WNDNUM; ++i)
	{
		m_SrcWndVec[i]->SetVisible(FALSE, TRUE);
		m_WndVec[i]->SetVisible(FALSE, TRUE);

	}
	if(!bFromInit)
		for(int i =m_nShowWndNum;i<nNum;++i)
			m_WndVec[i]->UpdateList();


	m_nShowWndNum = nNum;
}

void CDlgMultiFilter::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CDlgMultiFilter::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CDlgMultiFilter::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CDlgMultiFilter::OnSize(UINT nType, CSize size)
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

