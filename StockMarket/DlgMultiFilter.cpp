#include "stdafx.h"
#include "DlgMultiFilter.h"
#include <fstream>
#include "WndSynHandler.h"
#include <helper/SMenu.h>
#include <io.h>
#include <direct.h>

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
	m_bIsValid = FALSE;
	ShowWindow(SW_HIDE);
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.position", m_strWindowName);
	if (_access(strPosFile, 0) == 0)
		remove(strPosFile);

	StopAndClearData();
	//CSimpleWnd::DestroyWindow();
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
	//InitWindowPos();
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
	if (m_bIsValid)
	{
		SStringA strPosFile;
		strPosFile.Format(".\\config\\%s.position", m_strWindowName);
		std::ofstream ofile(strPosFile);
		if (ofile.is_open())
		{
			WINDOWPLACEMENT wp = { sizeof(wp) };
			::GetWindowPlacement(m_hWnd, &wp);

			ofile.write((char*)&wp, sizeof(wp));
			ofile.close();
		}
		ShowWindow(SW_HIDE);
		StopAndClearData();
	}
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
	auto pFilterData = g_WndSyn.GetFilterData();
	auto pStockPos = g_WndSyn.GetStockPos();
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
		m_WndVec[i]->SetDataPoint(&pStockPos->at(Group_Stock), DT_StockPos);

		for (int j = Group_SWL1; j < Group_Stock; ++j)
			m_WndVec[i]->SetDataPoint(&pFilterData->at(j), DT_L1IndyFilterData + j);
		for (int j = Group_SWL1; j < Group_Stock; ++j)
			m_WndVec[i]->SetDataPoint(&pStockPos->at(j), DT_L1IndyIndexPos + j);

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
			ifile.close();
		}

		vector<HisStockFilter> hsfVec;
		strPath.Format(".//config//%s_HSF_%d.DAT", m_strWindowName, i);
		ifile.open(strPath, std::ios::binary | std::ios::_Nocreate);
		if (ifile.is_open())
		{
			int nSFParaSize = 0;
			ifile.read((char*)&nSFParaSize, sizeof(nSFParaSize));
			HisStockFilter hsfPara = { 0 };
			while (ifile.read((char*)&hsfPara, nSFParaSize))
				hsfVec.emplace_back(hsfPara);
			m_WndVec[i]->InitComboHisStockFilterPara(hsfVec);
			ifile.close();
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
		strSection.Format("Wnd%d", i);
		GetInitPara(ini, initPara, strSection);

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
		strSection.Format("Wnd%d", i);
		SaveInitPara(ini, initPara, strSection);

		SavePointWndInfo(ini, initPara, strSection);
	}

}

void CDlgMultiFilter::SaveListConfig()
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
	m_MsgHandleMap[WW_GetHisTFBase]
		= &CDlgMultiFilter::OnGetHisTFBase;

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
	m_MsgHandleMap[Syn_HisTFBase]
		= &CDlgMultiFilter::OnHisTFBase;
	m_MsgHandleMap[Syn_TodayTFMarket]
		= &CDlgMultiFilter::OnTodayTFMarket;
	m_MsgHandleMap[Syn_RTTFMarkt]
		= &CDlgMultiFilter::OnRTTFMarket;

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
	for (int i = 0; i < MAX_WNDNUM; ++i)
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
	for (int i = 0; i < MAX_WNDNUM; ++i)
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

void CDlgMultiFilter::OnHisTFBase(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_TodayTFMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));

}

void CDlgMultiFilter::OnGetHisTFBase(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetHisTFBase, info, nMsgLength);
}

void CDlgMultiFilter::OnTodayTFMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisTFBase,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMultiFilter::OnRTTFMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((TickFlowMarket*)info)[0].SecurityID;
	if (m_WndSubMap[Group_Stock] == strStock)
		SendMsg(m_WndVec[Group_Stock]->GetThreadID(), WW_RTTFMarket,
			info, nMsgLength);
}

void CDlgMultiFilter::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	SetShowWndNum(nID - FWM_Start + 1, FALSE);
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

void CDlgMultiFilter::OnBtnSetWndNum()
{
	SMenu menu;
	if (menu.LoadMenuW(L"smenu:menu_filterWndNum"))
	{
		SImageButton *pBtn = FindChildByName2<SImageButton>(L"btn_wndNum");
		menu.CheckMenuItem(FWM_Start + m_nShowWndNum - 1, MF_CHECKED);
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen(&rc);
		menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);
	}

}


void CDlgMultiFilter::SaveComboStockFilterPara(int nWndNum)
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

	vector<HisStockFilter> hsfVec;
	m_WndVec[nWndNum]->OutputComboHisStockFilterPara(hsfVec);
	strPath.Format(".//config//%s_HSF_%d.DAT", m_strWindowName, nWndNum);
	if (!hsfVec.empty())
	{
		std::ofstream ofile(strPath, std::ios::binary);
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

void CDlgMultiFilter::StopAndClearData()
{
	SendMsg(m_SynThreadID, Syn_RemoveWnd, (char*)&m_hWnd, sizeof(m_hWnd));
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();
	m_DataThreadID = INVALID_THREADID;

	SavePicConfig();
	SaveListConfig();

	//for (int i = 0; i < MAX_WNDNUM; ++i)
	//	m_WndVec[i]->DestroyWindow();

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
	if (!bFromInit)
		for (int i = m_nShowWndNum; i < nNum; ++i)
			m_WndVec[i]->UpdateList();


	m_nShowWndNum = nNum;
	return TRUE;
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

