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
	//InitWindowPos();
	InitWorkWnd();
	UINT uMsgThreadID = m_MsgHandler.Init(m_hWnd, m_WndVec, m_SynThreadID,FALSE);
	g_WndSyn.AddWnd(m_hWnd, uMsgThreadID);
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
	m_WndVec.resize(Group_Count);
	SRealWnd * pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL1");
	m_WndVec[Group_SWL1] = (CWorkWnd *)pRealWnd->GetData();
	m_WndVec[Group_SWL1]->SetGroup(Group_SWL1, m_hWnd);
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_SWL2");
	m_WndVec[Group_SWL2] = (CWorkWnd *)pRealWnd->GetData();
	m_WndVec[Group_SWL2]->SetGroup(Group_SWL2, m_hWnd);
	pRealWnd = FindChildByName2<SRealWnd>(L"wnd_Stock");
	m_WndVec[Group_Stock] = (CWorkWnd *)pRealWnd->GetData();
	m_WndVec[Group_Stock]->SetGroup(Group_Stock, m_hWnd);
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
		g_WndSyn.SetSubWndInfo(m_WndVec[i]->m_hWnd, m_hWnd, i);
		m_WndVec[i]->SetListInfo(ListInsVec[i], StockName);
		m_WndVec[i]->SetDataPoint(&pListData->at(i), DT_ListData);
		m_WndVec[i]->SetDataPoint(&pFilterData->at(i), DT_FilterData);
		m_WndVec[i]->SetPointInfo(infoMap);
		m_WndVec[i]->SetDataPoint(&pCallAction->at(i), DT_CallAction);
		m_WndVec[i]->SetDataPoint(&pStockPos->at(i), DT_StockPos);

		//添加上级行业的选股器数据
		for (int j = Group_SWL1; j < i; ++j)
			m_WndVec[i]->SetDataPoint(&pFilterData->at(j), DT_L1IndyFilterData + j);

		for (int j = Group_SWL1; j < i; ++j)
			m_WndVec[i]->SetDataPoint(&pStockPos->at(j), DT_L1IndyIndexPos + j);
		if (i == Group_Stock)
		{
			map<int, strHash<TickFlowMarket>> *pTFMarket = g_WndSyn.GetTFMarket();
			m_WndVec[i]->SetDataPoint(pTFMarket, DT_TFMarket);
		}
		m_WndVec[i]->SetPreClose(preCloseMap);
		m_WndVec[i]->InitList();
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
			m_WndVec[i]->InitStockFilterPara(sfPlan);
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

		m_WndVec[i]->InitShowConfig(initPara);
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

		m_WndVec[i]->InitListConfig(showTitleMap, titleOrderMap);
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
		auto initPara = m_WndVec[i]->OutPutInitPara();
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
		map<int, BOOL>showTitleMap = m_WndVec[i]->GetListShowTitle();
		map<int, int>titleOrderMap = m_WndVec[i]->GetListTitleOrder();
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
		m_WndVec[i]->SetPicUnHandled();
		m_WndVec[i]->ClearData();
	}
	ReInitWorkWnd();
	ShowWindow(SW_SHOW);
	//SwitchToThisWindow(m_hWnd, FALSE);

}


void CDlgSub::ReInitList()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
		m_WndVec[i]->ReInitList();
}

void CDlgSub::ReInitWorkWnd()
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
	m_WndVec[nGroup]->OutputStockFilterPara(sfPlan);
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
	m_WndVec[nGroup]->OutputComboStockFilterPara(sfVec);
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
	m_WndVec[nGroup]->OutputComboHisStockFilterPara(hsfVec);
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

void CDlgSub::StopAndClearData()
{
	SendMsg(m_SynThreadID, Syn_RemoveWnd, (char*)&m_hWnd, sizeof(m_hWnd));
	m_MsgHandler.Stop();

	SavePicConfig();
	SaveListConfig();

	for (int i = 0; i < Group_Count; ++i)
		m_WndVec[i]->DestroyWindow();

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

