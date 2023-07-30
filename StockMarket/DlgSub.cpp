#include "stdafx.h"
#include "DlgSub.h"
#include "IniFile.h"
#include <fstream>
#include "WndSynHandler.h"
#include "DlgStockFilter.h"
#include <io.h>
#include <direct.h>



#define TIMER_AUTOSAVE 1

extern CWndSynHandler g_WndSyn;
extern HWND g_MainWnd;


CDlgSub::CDlgSub(SStringA strWndName) :SHostWnd(_T("LAYOUT:dlg_subWindow"))
{
	m_bLayoutInited = FALSE;
	m_strWindowName = strWndName;
	m_bIsValid = TRUE;
}


CDlgSub::~CDlgSub()
{
	if (INVALID_THREADID != m_DataThreadID)
		SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();
}

void CDlgSub::OnClose()
{
	m_bIsValid = FALSE;
	ShowWindow(SW_HIDE);
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.position", m_strWindowName);
	if (_access(strPosFile, 0) == 0)
		remove(strPosFile);

	StopAndClearData();
}

int CDlgSub::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CDlgSub::OnInitDialog(EventArgs* e)
{
	m_bLayoutInited = TRUE;
	SStatic *pTitl = FindChildByID2<SStatic>(R.id.text_windowName);
	pTitl->SetWindowTextW(StrA2StrW(m_strWindowName));
	CSimpleWnd::SetWindowTextW(StrA2StrW(m_strWindowName));
	m_SynThreadID = g_WndSyn.GetThreadID();
	tDataProc = thread(&CDlgSub::MsgProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	//InitWindowPos();
	InitMsgHandleMap();
	InitWorkWnd();
	g_WndSyn.AddWnd(m_hWnd, m_DataThreadID);
	SetTimer(TIMER_AUTOSAVE, 5000);
	return 0;
}
LRESULT CDlgSub::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
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

void CDlgSub::OnDestroy()
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

void CDlgSub::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == TIMER_AUTOSAVE)
		SavePicConfig();
}


void CDlgSub::InitWindowPos()
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

void CDlgSub::InitWorkWnd()
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
	auto pFilterData =g_WndSyn.GetFilterData();
	auto pStockPos = g_WndSyn.GetStockPos();

	strHash<double> preCloseMap(g_WndSyn.GetCloseMap());
	auto infoMap = g_WndSyn.GetPointInfo();
	vector<strHash<CAInfo>>* pCallAction = g_WndSyn.GetCallActionData();

	InitConfig(infoMap);
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

void CDlgSub::InitStockFilter()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		SFPlan sfPlan;
		SStringA strPath;
		strPath.Format(".//filter//%s_SF_%d.sfl", m_strWindowName, i);
		std::ifstream ifile(strPath, std::ios::_Nocreate);
		if (ifile.is_open())
		{
			CDlgStockFilter::ReadConditonsList(ifile, sfPlan);
			m_WndMap[i]->InitStockFilterPara(sfPlan);
			ifile.close();
		}

	}

}

void CDlgSub::InitComboStockFilter()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
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
			m_WndMap[i]->InitComboStockFilterPara(sfVec);
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
			m_WndMap[i]->InitComboHisStockFilterPara(hsfVec);
			ifile.close();
		}

	}

}

void CDlgSub::InitPointWndInfo(CIniFile & ini, InitPara & initPara, SStringA strSection, map<int, ShowPointInfo> &pointMap)
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


void CDlgSub::InitConfig(map<int, ShowPointInfo> &pointMap)
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.ini", m_strWindowName);
	CIniFile ini(strPosFile);
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

void CDlgSub::InitListConfig()
{
	SStringA strFile;
	strFile.Format(".\\config\\ListConfig_%s.ini", m_strWindowName);
	CIniFile ini(strFile);

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

void CDlgSub::SavePointWndInfo(CIniFile & ini, InitPara & initPara, SStringA strSection)
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




void CDlgSub::SavePicConfig()
{
	SStringA strPosFile;
	strPosFile.Format(".\\config\\%s.ini", m_strWindowName);
	CIniFile ini(strPosFile);
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto initPara = m_WndMap[i]->OutPutInitPara();
		SStringA strSection;
		strSection.Format("Group%d", i);
		SaveInitPara(ini, initPara, strSection);
		SavePointWndInfo(ini, initPara, strSection);
	}

}

void CDlgSub::SaveListConfig()
{
	SStringA strFile;
	strFile.Format(".\\config\\ListConfig_%s.ini", m_strWindowName);
	CIniFile ini(strFile);

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


void CDlgSub::ReInit()
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


void CDlgSub::ReInitList()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
		m_WndMap[i]->ReInitList();
}

void CDlgSub::ReInitWorkWnd()
{
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		m_WndMap[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndMap[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		m_WndMap[i]->ReInitList();
		m_WndMap[i]->ReSetPic();
	}
}

void CDlgSub::InitMsgHandleMap()
{
	m_MsgHandleMap[WW_ListData]
		= &CDlgSub::OnUpdateList;
	m_MsgHandleMap[WW_GetMarket]
		= &CDlgSub::OnGetMarket;
	m_MsgHandleMap[WW_GetKline]
		= &CDlgSub::OnGetKline;
	m_MsgHandleMap[WW_GetPoint]
		= &CDlgSub::OnGetPoint;
	m_MsgHandleMap[WW_GetCallAction]
		= &CDlgSub::OnGetCallAction;

	m_MsgHandleMap[Syn_Point]
		= &CDlgSub::OnUpdatePoint;
	//m_MsgHandleMap[Syn_TodayPoint]
	//	= &CDlgSub::OnTodayPoint;
	m_MsgHandleMap[Syn_HisRpsPoint]
		= &CDlgSub::OnHisRpsPoint;
	m_MsgHandleMap[Syn_RTIndexMarket]
		= &CDlgSub::OnRTIndexMarket;
	m_MsgHandleMap[Syn_RTStockMarket]
		= &CDlgSub::OnRTStockMarket;
	m_MsgHandleMap[Syn_HisIndexMarket]
		= &CDlgSub::OnHisIndexMarket;
	m_MsgHandleMap[Syn_HisStockMarket]
		= &CDlgSub::OnHisStockMarket;
	m_MsgHandleMap[Syn_HisKline]
		= &CDlgSub::OnHisKline;
	m_MsgHandleMap[Syn_CloseInfo]
		= &CDlgSub::OnCloseInfo;
	m_MsgHandleMap[WW_ChangeIndy]
		= &CDlgSub::OnChangeIndy;
	m_MsgHandleMap[Syn_HisSecPoint]
		= &CDlgSub::OnHisSecPoint;
	m_MsgHandleMap[Syn_RehabInfo]
		= &CDlgSub::OnRehabInfo;
	m_MsgHandleMap[Syn_HisCallAction]
		= &CDlgSub::OnHisCallAction;

}

void CDlgSub::MsgProc()
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

void CDlgSub::OnUpdateList(int nMsgLength, const char * info)
{
	for (auto &it : m_WndMap)
		SendMsg(it.second->GetThreadID(), WW_ListData,
			info, nMsgLength);

}

void CDlgSub::OnGetMarket(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	m_WndSubMap[pDgInfo->Group] = pDgInfo->StockID;
	SendMsg(m_SynThreadID, Syn_GetMarket, info, nMsgLength);
}

void CDlgSub::OnGetKline(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetKline, info, nMsgLength);
}

void CDlgSub::OnGetPoint(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetPoint, info, nMsgLength);
}

void CDlgSub::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_Point,
		info + 4, nMsgLength - 4);
}

//void CDlgSub::OnTodayPoint(int nMsgLength, const char * info)
//{
//	int nGroup = *(int*)info;
//	SendMsg(m_WndMap[nGroup]->GetThreadID(), WW_TodayPoint,
//		info + 4, nMsgLength - 4);
//}

void CDlgSub::OnHisRpsPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisRpsPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnRTIndexMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	if (m_WndSubMap[Group_SWL1] == strStock)
		SendMsg(m_WndMap[Group_SWL1]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
	if (m_WndSubMap[Group_SWL2] == strStock)
		SendMsg(m_WndMap[Group_SWL2]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
}

void CDlgSub::OnRTStockMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	if (m_WndSubMap[Group_Stock] == strStock)
		SendMsg(m_WndMap[Group_Stock]->GetThreadID(), WW_RTStockMarket,
			info, nMsgLength);
}

void CDlgSub::OnHisIndexMarket(int nMsgLength, const char * info)
{

	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisIndexMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnHisStockMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisStockMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnHisKline(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisKline,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnCloseInfo(int nMsgLength, const char * info)
{
	for (int i = 0; i < Group_Count; ++i)
		SendMsg(m_WndMap[i]->GetThreadID(), WW_CloseInfo,
			info, nMsgLength);
}

void CDlgSub::OnChangeIndy(int nMsgLength, const char * info)
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

void CDlgSub::OnHisSecPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisSecPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnRehabInfo(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_RehabInfo,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnHisCallAction(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndMap[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisCallAction,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgSub::OnGetCallAction(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetCallAction, info, nMsgLength);
}

void CDlgSub::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

//TODO:消息映射
void CDlgSub::OnBtnClose()
{
	GetNative()->SendMessage(WM_CLOSE);
}

void CDlgSub::SaveStockFilterPara(int nGroup)
{
	SFPlan sfPlan;
	m_WndMap[nGroup]->OutputStockFilterPara(sfPlan);
	if (!sfPlan.condVec.empty())
	{
		SStringA strPath;
		strPath.Format(".//filter//%s_SF_%d.sfl", m_strWindowName,nGroup);
		std::ofstream ofile(strPath);
		if (ofile.is_open())
		{
			CDlgStockFilter::SaveConditonsList(ofile, sfPlan);
			ofile.close();
		}
	}
}

void CDlgSub::SaveComboStockFilterPara(int nGroup)
{
	vector<StockFilter> sfVec;
	m_WndMap[nGroup]->OutputComboStockFilterPara(sfVec);
	SStringA strPath;
	strPath.Format(".//config//%s_SF_%d.DAT", m_strWindowName, nGroup);
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
	strPath.Format(".//config//%s_HSF_%d.DAT", m_strWindowName, nGroup);
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

void SOUI::CDlgSub::StopAndClearData()
{
	SendMsg(m_SynThreadID, Syn_RemoveWnd, (char*)&m_hWnd, sizeof(m_hWnd));
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();
	m_DataThreadID = INVALID_THREADID;

	SavePicConfig();
	SaveListConfig();

	for (int i = 0; i < Group_Count; ++i)
		m_WndMap[i]->DestroyWindow();

}

void CDlgSub::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CDlgSub::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CDlgSub::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CDlgSub::OnSize(UINT nType, CSize size)
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

