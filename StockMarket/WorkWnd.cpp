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
#include "DlgStockFilter.h"
#include <sstream>
#include "DlgSelTarget.h"
#include "DlgComboStockFilter.h"
#include "DlgHeaderSelect.h"
#include "DlgRehabFixedTime.h"

#define MAX_SUBPIC 3
#define SHOWDATACOUNT 2
#define MAX_TICK 6000
#define MIN_DIFF 0.000001

CWorkWnd::CWorkWnd() :SHostWnd(_T("LAYOUT:wnd_work"))
{
	m_pDlgKbElf = nullptr;
	m_uThreadID = INVALID_THREADID;
	m_bUseStockFilter = FALSE;
	m_bShowList = true;
	m_pFilterDataMap = nullptr;
	m_pTFMarketHash = nullptr;
	m_pL1IndyFilterDataMap = nullptr;
	m_pL2IndyFilterDataMap = nullptr;
	m_bFilterWnd = FALSE;
}


CWorkWnd::~CWorkWnd()
{
	if (INVALID_THREADID != m_uThreadID)
		SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_workThread.joinable())
		m_workThread.join();
	if (m_pDlgKbElf)
		m_pDlgKbElf->DestroyWindow();
	m_pPreSelBtn = nullptr;
}

void CWorkWnd::SetGroup(RpsGroup Group, HWND hParWnd)
{
	m_Group = Group;
	m_hParWnd = hParWnd;
	if (Group_SWL1 == m_Group)
	{
		m_pTextIndy->SetVisible(FALSE);
		m_pBtnConn1->SetVisible(FALSE);
		m_pBtnConn1->SetAttribute(L"size", L"0,22");
		m_pBtnConn2->SetVisible(FALSE);
		m_pBtnConn2->SetAttribute(L"size", L"0,22");
		m_pTextTitle->SetWindowTextW(L"申万一级行业");
		m_pCheckST->SetVisible(FALSE, TRUE);
		m_pCheckSBM->SetVisible(FALSE, TRUE);
		m_pCheckSTARM->SetVisible(FALSE, TRUE);
		m_pCheckNewStock->SetVisible(FALSE, TRUE);

	}
	else if (Group_SWL2 == m_Group)
	{
		m_pBtnConn1->SetWindowTextW(L"联");
		m_pBtnConn2->SetVisible(FALSE);
		m_pBtnConn2->SetAttribute(L"size", L"0,22");
		m_pTextTitle->SetWindowTextW(L"申万二级行业");
		m_pCheckST->SetVisible(FALSE, TRUE);
		m_pCheckSBM->SetVisible(FALSE, TRUE);
		m_pCheckSTARM->SetVisible(FALSE, TRUE);
		m_pCheckNewStock->SetVisible(FALSE, TRUE);

	}
	else if (Group_Stock == m_Group)
	{
		m_pTextTitle->SetWindowTextW(L"股票");
		m_tfNameVec.emplace_back("ABSR");
		m_tfNameVec.emplace_back("A2PBSR");
		m_tfNameVec.emplace_back("AABSR");
		m_tfNameVec.emplace_back("POCR");
		m_tfNameVec.emplace_back("VOLUME");
		m_tfNameVec.emplace_back("ASV");
		m_tfNameVec.emplace_back("ABV");
		m_tfNameVec.emplace_back("OPEN");
		m_tfNameVec.emplace_back("HIGH");
		m_tfNameVec.emplace_back("LOW");
		m_tfNameVec.emplace_back("AMOUNT");
		if (m_bFilterWnd)
		{
			m_pTextIndy->SetVisible(FALSE);
			m_pTextIndy->SetAttribute(L"size", L"0,22");
			m_pBtnConn1->SetVisible(FALSE);
			m_pBtnConn1->SetAttribute(L"size", L"0,22");
			m_pBtnConn2->SetVisible(FALSE);
			m_pBtnConn2->SetAttribute(L"size", L"0,22");
		}

	}

	//2023.5.4修改 将m_pDlgStockFilter 改为 m_pDlgCmbStockFilter
	//在保留新版本的情况下 将其对外接口隐藏 回退老版本

	m_pDlgCmbStockFilter = new CDlgComboStockFilter(m_uThreadID, m_Group);
	m_pDlgCmbStockFilter->Create(NULL);
	m_pDlgCmbStockFilter->CenterWindow(m_hParWnd);
	m_pDlgCmbStockFilter->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
}

void CWorkWnd::InitShowConfig(InitPara initPara)
{
	vector<SStringA> SubPicWndNameVec;
	SubPicWndNameVec.reserve(MAX_SUBPIC);
	SubPicWndNameVec.emplace_back("全市场RPS");
	SubPicWndNameVec.emplace_back("1级行业RPS");
	SubPicWndNameVec.emplace_back("2级行业RPS");

	m_InitPara = initPara;
	//if (Group_Stock == m_Group)
	//{
	m_pFenShiPic->InitSubPic(m_InitPara.nTSCPointWndNum);
	m_pKlinePic->InitSubPic(m_InitPara.nKlinePointWndNum);
	//}
	//else
	//{
	//	m_pFenShiPic->InitSubPic(1);
	//	m_pKlinePic->InitSubPic(1);
	//}
	m_pFenShiPic->SetRpsGroup(m_Group);
	m_pFenShiPic->InitShowPara(m_InitPara);
	m_pKlinePic->SetRpsGroup(m_Group);
	m_pKlinePic->SetParentHwnd(m_hWnd);
	m_pKlinePic->InitShowPara(m_InitPara);
	m_PicPeriod = m_ListPeriod = m_InitPara.Period;

	m_pPreSelBtn = m_pPeriodBtnMap[m_ListPeriod];
	m_pPreSelBtn->SetAttribute(L"colorText", L"#00ffffff");
	m_bListConn1 = initPara.Connect1;
	m_bListConn2 = initPara.Connect2;
	if (m_bListConn1)
		SetBtnState(m_pBtnConn1, true);
	if (m_bListConn2)
		SetBtnState(m_pBtnConn2, true);

	if (m_bListConn1 || m_bListConn2)
	{
		m_ListShowInd = m_InitPara.ShowIndy;
		SetListShowIndyStr(m_pTextIndy);
	}

	m_bUseStockFilter = initPara.UseStockFilter;
	m_bListShowST = initPara.ListShowST;
	m_bListShowSBM = initPara.ListShowSBM;
	m_bListShowSTARM = initPara.ListShowSTARM;
	m_bListShowNewStock = initPara.ListShowNewStock;
	m_pCheckST->SetCheck(m_bListShowST);
	m_pCheckSBM->SetCheck(m_bListShowSBM);
	m_pCheckSTARM->SetCheck(m_bListShowSTARM);
	m_pCheckNewStock->SetCheck(m_bListShowNewStock);

	SetBtnState(m_pBtnStockFilter, m_bUseStockFilter);
	if (m_InitPara.nKlineRehabType != eRT_NoRehab)
		m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");

}

void CWorkWnd::InitListConfig(map<int, BOOL>& titleShowMap, map<int, int>&titleOrderMap)
{
	m_TitleShowMap = titleShowMap;
	m_TitleOrderMap = titleOrderMap;
	SetListDataIsShow();
	SetListDataOrder();
}

InitPara CWorkWnd::OutPutInitPara()
{
	m_pFenShiPic->OutPutShowPara(m_InitPara);
	m_pKlinePic->OutPutShowPara(m_InitPara);
	m_InitPara.Period = m_ListPeriod;
	m_InitPara.Connect1 = m_bListConn1;
	m_InitPara.Connect2 = m_bListConn2;
	strcpy_s(m_InitPara.ShowIndy, m_ListShowInd);
	m_InitPara.UseStockFilter = m_bUseStockFilter;
	m_InitPara.ListShowST = m_bListShowST;
	m_InitPara.ListShowSBM = m_bListShowSBM;
	m_InitPara.ListShowSTARM = m_bListShowSTARM;
	m_InitPara.ListShowNewStock = m_bListShowNewStock;

	return m_InitPara;
}

void CWorkWnd::ClearData()
{
	m_InfoVec.clear();
}

void CWorkWnd::InitList()
{
	m_pList = FindChildByID2<SColorListCtrlEx>(R.id.ls_rps);
	m_pList->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK,
		Subscriber(&CWorkWnd::OnListDbClick, this));
	m_pList->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
		Subscriber(&CWorkWnd::OnListLClick, this));
	SHeaderCtrlEx * pHeader =
		(SHeaderCtrlEx *)m_pList->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(5);
	pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CWorkWnd::OnListHeaderClick, this));
	pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP,
		Subscriber(&CWorkWnd::OnListHeaderSwap, this));
	if (Group_Stock == m_Group)
	{
		m_pList->InsertColumn(SHead_ActBuySellRatio, L"主动量比%", 70);
		m_pList->InsertColumn(SHead_ActToPasBuySellRatio,
			L"转换主被动量比%", 70);
		m_pList->InsertColumn(SHead_AvgBuySellRatio, L"平均量比%", 70);
		m_pList->InsertColumn(SHead_POCRatio, L"最多成交价格比%", 70);
		m_pList->InsertColumn(SHead_Volume, L"成交量", 70);
		m_pList->InsertColumn(SHead_ActSellVolume, L"内盘", 70);
		m_pList->InsertColumn(SHead_ActBuyVolume, L"外盘", 70);
		m_pList->InsertColumn(SHead_Open, L"开盘", 70);
		m_pList->InsertColumn(SHead_High, L"最高", 70);
		m_pList->InsertColumn(SHead_Low, L"最低", 70);
		m_pList->InsertColumn(SHead_Amount, L"成交额", 70);
	}

	m_ListDataSortMap[SHead_ID] = eSDT_Int;
	m_ListDataSortMap[SHead_Name] = eSDT_String;
	m_ListDataSortMap[SHead_CloseRank520] = eSDT_Int;
	m_ListDataSortMap[SHead_CloseRank2060] = eSDT_Int;
	m_ListDataSortMap[SHead_AmountMACD520] = eSDT_BigDouble;
	m_ListDataSortMap[SHead_AmountRank520] = eSDT_Int;
	m_ListDataSortMap[SHead_AmountMACD2060] = eSDT_BigDouble;
	m_ListDataSortMap[SHead_AmountRank2060] = eSDT_Int;
	m_ListDataSortMap[SHead_Volume] = eSDT_Int;
	m_ListDataSortMap[SHead_ActSellVolume] = eSDT_Int;
	m_ListDataSortMap[SHead_ActBuyVolume] = eSDT_Int;
	m_ListDataSortMap[SHead_Amount] = eSDT_BigDouble;
	m_ListDataSortMap[SHead_CAVol] = eSDT_Int;
	m_ListDataSortMap[SHead_CAVolRank] = eSDT_Int;
	m_ListDataSortMap[SHead_CAAmo] = eSDT_BigDouble;
	m_ListDataSortMap[SHead_CAAmoRank] = eSDT_Int;

	m_ListDataDecMap[SHead_LastPx] = 2;
	m_ListDataDecMap[SHead_ChangePct] = 2;
	m_ListDataDecMap[SHead_ActBuySellRatio] = 2;
	m_ListDataDecMap[SHead_ActToPasBuySellRatio] = 2;
	m_ListDataDecMap[SHead_AvgBuySellRatio] = 2;
	m_ListDataDecMap[SHead_POCRatio] = 2;
	m_ListDataDecMap[SHead_Open] = 2;
	m_ListDataDecMap[SHead_High] = 2;
	m_ListDataDecMap[SHead_Low] = 2;

	m_bListInited = true;
	if (!m_bUseStockFilter)
		UpdateListShowStock();
	else
		UpdateListFilterShowStock();
	m_MouseWheelMap.hash = m_ListPosMap.hash;
	UpdateList();

}

void CWorkWnd::ReInitList()
{
	m_bListInited = false;
	UpdateListShowStock();
	m_bListInited = true;
	UpdateList();

}

void CWorkWnd::SetDataPoint(void * pData, int DataType)
{
	switch (DataType)
	{
	case DT_ListData:
		m_pListDataMap = (map<int, strHash<RtRps>>*)pData;
		break;
	case DT_TFMarket:
		m_pTFMarketHash = (map<int, strHash<TickFlowMarket>>*)pData;
		break;
	case DT_FilterData:
		m_pFilterDataMap = (map<int, strHash<map<string, double>>>*)pData;
		break;
	case DT_L1IndyFilterData:
		m_pL1IndyFilterDataMap = (map<int, strHash<map<string, double>>>*)pData;
		break;
	case DT_L2IndyFilterData:
		m_pL2IndyFilterDataMap = (map<int, strHash<map<string, double>>>*)pData;
		break;
	case DT_CallAction:
		m_pCallActionHash = (strHash<CAInfo>*)pData;
	break;
	default:
		break;
	}
}

void CWorkWnd::SetPicUnHandled()
{
	m_pFenShiPic->SetPicUnHandled();
	m_pKlinePic->SetPicUnHandled();
}

void CWorkWnd::ReSetPic()
{
	if (m_strSubStock != "")
		ShowPicWithNewID(m_strSubStock, true);
}


void CWorkWnd::SetListInfo(vector<StockInfo>& infoVec,
	strHash<SStringA>& StockNameMap)
{
	m_InfoVec = infoVec;
	for (auto &it : m_InfoVec)
		m_infoMap.hash[it.SecurityID] = it;
	m_StockName = StockNameMap;
	m_pDlgKbElf->SetStockInfo(m_InfoVec);
	m_pDlgCmbStockFilter->SetStockInfo(m_InfoVec);
}

//void CWorkWnd::UpdateTodayPointData(SStringA pointName,
//	vector<CoreData>& dataVec, bool bLast)
//{
//	m_PointData[m_PicPeriod][pointName] = dataVec;
//	if (bLast)
//		m_PointReadyMap[m_PicPeriod][pointName] = TRUE;
//}

void CWorkWnd::CloseWnd()
{
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_workThread.joinable())
		m_workThread.join();
	m_uThreadID = INVALID_THREADID;
	m_pPreSelBtn = nullptr;
	m_pListDataMap = nullptr;
	m_pList->DeleteAllItems();
	m_PointData.clear();
	m_L1IndyPointData.clear();
	m_L2IndyPointData.clear();
	m_IndexMarketVec.clear();
	m_IndexMarketVec.shrink_to_fit();
	m_StockMarketVec.clear();
	m_StockMarketVec.shrink_to_fit();
	m_KlineMap.clear();
	m_preCloseMap.hash.clear();
	m_StockName.hash.clear();
	m_dataNameVec.clear();
	m_dataNameVec.shrink_to_fit();
	//m_PointReadyMap.clear();
	m_KlineGetMap.clear();
	m_PointGetMap.clear();
	m_L1IndyPointGetMap.clear();
	m_L2IndyPointGetMap.clear();
	m_SubPicShowNameVec.clear();
	//m_SubPicShowNameVec.shrink_to_fit();
	m_ListPosMap.hash.clear();
	m_MouseWheelMap.hash.clear();
}

void CWorkWnd::OutputStockFilterPara(SFPlan &sfPlan)
{
	m_pDlgStockFilter->OutPutCondition(sfPlan);
}

void CWorkWnd::OutputComboStockFilterPara(vector<StockFilter>& sfVec)
{
	m_pDlgCmbStockFilter->OutPutCondition(sfVec);
}

void CWorkWnd::InitStockFilterPara(SFPlan &sfPlan)
{
	m_sfPlan = sfPlan;
	if (m_sfPlan.condVec.empty())
	{
		m_bUseStockFilter = false;
		SetBtnState(m_pBtnStockFilter, m_bUseStockFilter);
	}
	m_pDlgStockFilter->InitList(m_bUseStockFilter, sfPlan);
}

void CWorkWnd::InitComboStockFilterPara(vector<StockFilter>& sfVec)
{
	m_sfVec = sfVec;
	m_pDlgCmbStockFilter->InitList(m_bUseStockFilter, sfVec);
}

void CWorkWnd::SetPointInfo(map<int, ShowPointInfo>& infoMap)
{
	if (m_Group == Group_Stock)
		m_pointInfoMap = infoMap;
	else
	{
		for (int i = eFullMarketPointStart; i < eFullMarketPointEnd; ++i)
			m_pointInfoMap[i] = infoMap[i];
		for (int i = eCAPointStart; i < eCAPointEnd; ++i)
			m_pointInfoMap[i] = infoMap[i];

	}
}

map<int, int> CWorkWnd::GetListTitleOrder()
{
	return m_TitleOrderMap;
}


void CWorkWnd::OnInit(EventArgs * e)
{
	::InitializeCriticalSection(&m_csClose);
	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

	m_pFenShiPic = FindChildByID2<SFenShiPic>(R.id.fenshiPic);
	m_pKlinePic = FindChildByID2<SKlinePic>(R.id.klinePic);
	m_pTextTitle = FindChildByID2<SStatic>(R.id.text_Group);
	m_pTextIndy = FindChildByID2<SStatic>(R.id.text_ShowIndy);
	m_pBtnMarket = FindChildByID2<SImageButton>(R.id.btn_Market);
	m_pCheckST = FindChildByID2<SCheckBox>(R.id.chk_ST);
	m_pCheckSBM = FindChildByID2<SCheckBox>(R.id.chk_SBM);
	m_pCheckSTARM = FindChildByID2<SCheckBox>(R.id.chk_STARM);
	m_pCheckNewStock = FindChildByID2<SCheckBox>(R.id.chk_NewStock);
	m_pBtnRehab = FindChildByID2<SImageButton>(R.id.btn_Rehab);
	m_pKlinePic->SetCaInfoData(&m_CallAction);
	m_pBtnRehab->SetVisible(FALSE, FALSE);
	m_pPeriodBtnMap[Period_FenShi] =
		FindChildByID2<SImageButton>(R.id.btn_FS);
	m_pPeriodBtnMap[Period_1Min] =
		FindChildByID2<SImageButton>(R.id.btn_M1);
	m_pPeriodBtnMap[Period_5Min] =
		FindChildByID2<SImageButton>(R.id.btn_M5);
	m_pPeriodBtnMap[Period_15Min] =
		FindChildByID2<SImageButton>(R.id.btn_M15);
	m_pPeriodBtnMap[Period_30Min] =
		FindChildByID2<SImageButton>(R.id.btn_M30);
	m_pPeriodBtnMap[Period_60Min] =
		FindChildByID2<SImageButton>(R.id.btn_M60);
	m_pPeriodBtnMap[Period_1Day] =
		FindChildByID2<SImageButton>(R.id.btn_Day);
	m_pBtnConn1 = FindChildByID2<SImageButton>(R.id.btn_ListConnect1);
	m_pBtnConn2 = FindChildByID2<SImageButton>(R.id.btn_ListConnect2);
	m_pBtnStockFilter = FindChildByID2<SImageButton>(R.id.btn_StockFilter);
	m_pBtnTitleSel = FindChildByID2<SImageButton>(R.id.btn_TitleSel);

	InitProcFucMap();
	InitNameVec();
	InitStockFilterFunc();
	m_workThread = thread(&CWorkWnd::DataProc, this);
	m_uThreadID = *(unsigned*)&m_workThread.get_id();
	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);

}

LRESULT CWorkWnd::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case WDMsg_UpdateList:
		if (m_bUseStockFilter)
			UpdateListFilterShowStock();
		UpdateList();
		if (m_pList->IsVisible())
			m_pList->Invalidate();
		break;
	case WDMsg_UpdatePic:
		if (m_pFenShiPic->IsVisible())
			m_pFenShiPic->Invalidate();
		if (m_pKlinePic->IsVisible())
			m_pKlinePic->Invalidate();
		break;
	case WDMsg_SubIns:
		if (m_pDlgKbElf->GetShowPicInfo() != m_strSubStock)
			ShowPicWithNewID(m_pDlgKbElf->GetShowPicInfo());
		else
			SetSelectedPeriod(m_ListPeriod);
		break;
	case WDMsg_ChangeShowList:
		if (m_bUseStockFilter)
			UpdateListFilterShowStock();
		else
			UpdateListShowStock();
		UpdateList();
		break;
	case WDMsg_SetFocus:
		CSimpleWnd::SetFocus();
		break;
	case WDMsg_ChangeStockFilter:
		m_bUseStockFilter = (BOOL)lp;
		if (m_bUseStockFilter)
		{
			SetBtnState(m_pBtnStockFilter, true);
			SetBtnState(m_pBtnConn1, false);
			SetBtnState(m_pBtnConn2, false);
			//m_pDlgStockFilter->OutPutCondition(m_sfPlan);
			m_pDlgCmbStockFilter->OutPutCondition(m_sfVec);
			m_bListConn1 = false;
			m_bListConn2 = false;
			m_ListShowInd = "";
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdateList, 0);
		}
		else
		{
			SetBtnState(m_pBtnStockFilter, false);
			UpdateListShowStock();
		}

		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SaveConfig, 0);
		break;
	case WDMsg_ChangePointTarget:
	{
		int pointData = (int)lp;
		auto pPointData = &m_PointData;
		if (pointData > eIndyMarketPointEnd)
		{
			if (pointData >= eL1IndyPointStart &&  pointData < eL1IndyPointEnd)
				pPointData = &m_L1IndyPointData;
			else if (pointData >= eL2IndyPointStart &&  pointData < eL2IndyPointEnd)
				pPointData = &m_L2IndyPointData;
		}


		auto &info = m_pointInfoMap[pointData];
		GetPointData(info, m_strSubStock, m_PicPeriod);
		int dataCount = m_PointDataCount[info.type];
		vector<vector<CoreData>*> tmpDataArr(dataCount);
		vector<BOOL> rightVec(dataCount);
		vector<SStringA> dataNameVec;
		for (int i = 0; i < dataCount; ++i)
		{
			SStringA dataName = info.srcDataName +
				m_SubPicShowNameVec[info.type][info.dataInRange][i];
			tmpDataArr[i] = &(*pPointData)[m_PicPeriod][dataName];
			dataNameVec.emplace_back(m_SubPicShowNameVec[info.type][info.dataInRange][i]);
			rightVec[i] = TRUE;
		}
		SStringA strTitle = "";
		if (Group_Stock == m_Group)
		{
			vector<SStringA> nameVec;
			GetBelongingIndyName(nameVec);
			if ("L1" == info.dataInRange || "L1" == info.IndyRange)
				strTitle.Format("行业:%s", nameVec[0]);
			else if ("L2" == info.dataInRange || "L2" == info.IndyRange)
				strTitle.Format("行业:%s", nameVec[1]);
			strTitle.Format("%s %s", info.showName, strTitle);
		}

		if (m_pFenShiPic->IsVisible())
		{
			m_pFenShiPic->SetSelPointWndInfo(info, strTitle);
			m_pFenShiPic->SetSubPicShowData(dataCount, tmpDataArr,
				rightVec, dataNameVec, m_strSubStock,
				m_StockName.hash[m_strSubStock]);

		}
		else if (m_pKlinePic->IsVisible())
		{
			m_pKlinePic->SetSelPointWndInfo(info, strTitle);
			m_pKlinePic->SetSubPicShowData(dataCount, tmpDataArr,
				rightVec, dataNameVec, m_strSubStock,
				m_StockName.hash[m_strSubStock]);
		}


	}
	break;
	case WDMsg_ChangeShowTilte:
		m_TitleShowMap = *(map<int, BOOL>*)lp;
		SetListDataIsShow();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SaveListConfig, 0);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CWorkWnd::OnFSMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case FSMSG_UPDATE:
		break;
	case FSMSG_EMA:
		m_pFenShiPic->SetEmaPara((int*)wp);
		::SendMsg(m_uThreadID, WW_FSEma, NULL, 0);
		break;
	case FSMSG_MACD:
		m_pFenShiPic->SetMacdPara((int*)wp);
		::SendMsg(m_uThreadID, WW_FSMacd, NULL, 0);
		break;
	default:
		break;
	}

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
	return 0;
}

LRESULT CWorkWnd::OnKlineMsg(UINT uMsg, WPARAM wp,
	LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case KLINEMSG_UPDATE:
		break;
	case KLINEMSG_MA:
		m_pKlinePic->SetMaPara((int*)wp, m_MaParaSet);
		::SendMsg(m_uThreadID, WW_KlineMa, NULL, 0);
		break;
	case KLINEMSG_MACD:
		m_pKlinePic->SetMacdPara((int*)wp);
		::SendMsg(m_uThreadID, WW_KlineMacd, NULL, 0);
		break;
	case KLINEMSG_BAND:
		m_pKlinePic->SetBandPara(*(BandPara_t*)wp);
		::SendMsg(m_uThreadID, WW_KlineBand, NULL, 0);
		break;
	case KLINEMSG_REHAB:
		m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");
		::SendMsg(m_uThreadID, WW_FixedTimeRehab, (char*)wp, sizeof(FixedTimeRehab));
		break;
	default:
		break;
	}

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
	return 0;
}

void CWorkWnd::OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case FM_Deal:
		m_pFenShiPic->SetDealState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetDealState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_Volume:
		m_pFenShiPic->SetVolumeState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetVolumeState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_MACD:
		m_pFenShiPic->SetMacdState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetMacdState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
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

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case FM_EMA:
		m_pFenShiPic->SetEmaState();
		m_pFenShiPic->Invalidate();
		bState = m_pFenShiPic->GetEmaState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
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
	case FM_PointWnd0:
	case FM_PointWnd1:
	case FM_PointWnd2:
	case FM_PointWnd3:
	case FM_PointWnd4:
	case FM_PointWnd5:
	case FM_PointWnd6:
	case FM_PointWnd7:
	case FM_PointWnd8:
	{
		int nWndNum = nID - FM_PointWnd0;
		int nOldWndNum = m_pFenShiPic->GetShowSubPicNum();
		vector<ShowPointInfo> infoVec = m_pFenShiPic->GetSubPicDataToGet(nWndNum, m_pointInfoMap);
		m_pFenShiPic->ReSetSubPic(nWndNum, infoVec);
		std::set<ShowPointInfo>pointGetSet;
		for (auto &it : infoVec)
		{
			if (pointGetSet.count(it) == 0)
			{
				GetPointData(it, m_strSubStock, Period_FenShi);
				pointGetSet.insert(it);
			}
		}
		SetFenShiShowData(infoVec, nOldWndNum);
		m_pFenShiPic->Invalidate();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	}

	//case FM_L1RPS:
	//	m_pFenShiPic->SetRpsState(SP_SWINDYL1);
	//	m_pFenShiPic->Invalidate();
	//	bState = m_pFenShiPic->GetRpsState(SP_SWINDYL1);
	//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
	//		WDMsg_SaveConfig, NULL);
	//	break;
	//case FM_L2RPS:
	//	m_pFenShiPic->SetRpsState(SP_SWINDYL2);
	//	m_pFenShiPic->Invalidate();
	//	bState = m_pFenShiPic->GetRpsState(SP_SWINDYL2);
	//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
	//		WDMsg_SaveConfig, NULL);
	//	break;
	default:
		break;
	}
}
void CWorkWnd::OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)

{
	bool bState = false;
	switch (nID)
	{
	case KM_Deal:
		m_pKlinePic->SetDealState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetDealState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_MA:
		m_pKlinePic->SetBandState(false, false);
		m_pKlinePic->SetMaState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMaState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_Band:
		m_pKlinePic->SetMaState(false, false);
		m_pKlinePic->SetBandState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetBandState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_Volume:
		m_pKlinePic->SetVolumeState();
		m_pKlinePic->Invalidate();
		//bState = m_pKlinePic->GetVolumeState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_MACD:
		m_pKlinePic->SetMacdState();
		m_pKlinePic->Invalidate();
		bState = m_pKlinePic->GetMacdState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
		//case KM_RPS:
		//	m_pKlinePic->SetRpsState(SP_FULLMARKET);
		//	m_pKlinePic->Invalidate();
		//	bState = m_pKlinePic->GetRpsState(SP_FULLMARKET);
		//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		//		WDMsg_SaveConfig, NULL);
		//	break;
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
		m_MaParaSet = eMa_Close;
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara(m_MaParaSet));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	//case KM_L1RPS:
	//	m_pKlinePic->SetRpsState(SP_SWINDYL1);
	//	m_pKlinePic->Invalidate();
	//	bState = m_pKlinePic->GetRpsState(SP_SWINDYL1);
	//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
	//		WDMsg_SaveConfig, NULL);
	//	break;
	//case KM_L2RPS:
	//	m_pKlinePic->SetRpsState(SP_SWINDYL2);
	//	m_pKlinePic->Invalidate();
	//	bState = m_pKlinePic->GetRpsState(SP_SWINDYL2);
	//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
	//		WDMsg_SaveConfig, NULL);
	//	break;
	case KM_PointWnd0:
	case KM_PointWnd1:
	case KM_PointWnd2:
	case KM_PointWnd3:
	case KM_PointWnd4:
	case KM_PointWnd5:
	case KM_PointWnd6:
	case KM_PointWnd7:
	case KM_PointWnd8:
	{
		int nWndNum = nID - KM_PointWnd0;
		int nOldWndNum = m_pKlinePic->GetShowSubPicNum();
		vector<ShowPointInfo> infoVec = m_pKlinePic->GetSubPicDataToGet(nWndNum, m_pointInfoMap);
		m_pKlinePic->ReSetSubPic(nWndNum, infoVec);

		std::set<ShowPointInfo>pointGetSet;
		for (auto &it : infoVec)
		{
			if (pointGetSet.count(it) == 0)
			{
				GetPointData(it, m_strSubStock, m_PicPeriod);
				pointGetSet.insert(it);
			}
		}
		SetKlineShowData(infoVec, m_PicPeriod, FALSE, nOldWndNum);
		m_pKlinePic->Invalidate();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	}
	case KM_Amount:
		m_pKlinePic->SetAmountState();
		m_pKlinePic->Invalidate();
		//bState = m_pKlinePic->GetVolumeState();


		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_VolMaPara:
	{
		m_MaParaSet = eMa_Volume;
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara(m_MaParaSet));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_AmoMaPara:
	{
		m_MaParaSet = eMa_Amount;
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara(m_MaParaSet));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_CAVol:
		m_pKlinePic->SetCAVolState();
		m_pKlinePic->Invalidate();
		//bState = m_pKlinePic->GetVolumeState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_CAAmo:
		m_pKlinePic->SetCAAmoState();
		m_pKlinePic->Invalidate();
		//bState = m_pKlinePic->GetVolumeState();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case KM_CAVolMaPara:
	{
		m_MaParaSet = eMa_CAVol;
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara(m_MaParaSet));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_CAAmoMaPara:
	{
		m_MaParaSet = eMa_CAAmo;
		CDlgMaPara *pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic->GetMaPara(m_MaParaSet));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;

	default:
		break;
	}
}
void CWorkWnd::OnTarSelMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	switch (nID)
	{
	case TSM_Close:
		if (m_pFenShiPic->IsVisible())
			m_pFenShiPic->CloseSinglePointWnd();
		else if (m_pKlinePic->IsVisible())
			m_pKlinePic->CloseSinglePointWnd();

		::PostMessage(m_hParWnd, WM_WINDOW_MSG,
			WDMsg_SaveConfig, NULL);
		break;
	case TSM_Select:
	{
		CDlgSelTarget *pDlg = new CDlgSelTarget(m_hWnd, m_pointInfoMap, m_PicPeriod);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	}
}

void CWorkWnd::OnRehabMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	int rehabType = nID - RM_NoRehab;
	if (rehabType != eRT_Rehab_Cash_FixedTime &&
		rehabType != eRT_Rehab_ReInv_FixedTime)
	{
		::SendMsg(m_uThreadID, WW_ChangeRehab, (char*)&rehabType, sizeof(rehabType));
		if (rehabType == eRT_NoRehab)
			m_pBtnRehab->SetAttribute(L"colorText", L"#c0c0c0ff");
		else
			m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");
	}
	else
	{
		CDlgRehabFixedTime *pDlg = new CDlgRehabFixedTime(m_hWnd, (eRehabType)rehabType);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
}

void CWorkWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
void CWorkWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
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
			m_pDlgKbElf->SetWindowPos(HWND_TOPMOST, rc.right - 320,
				rc.bottom - 370, 0, 0, SWP_NOSIZE);
			m_pDlgKbElf->ClearInput();
			SStringW input = L"";
			if (m_nLastChar >= '0' && m_nLastChar <= '9')
				input = (char)m_nLastChar;
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
			//SetMsgHandled(TRUE);
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
			m_pDlgKbElf->ClearInput();
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
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
		}

	}
	else if (nChar == VK_RETURN)
	{
		if (m_bShowList)
		{
			int nSel = m_pList->GetSelectedItem();
			if (nSel != -1)
			{
				SStringA StockID = StrW2StrA(
					m_pList->GetSubItemText(nSel, SHead_ID));
				if (m_ListPosMap.hash.count(StockID) == 0)
					StockID += "I";

				m_PicPeriod = m_ListPeriod;
				if (StockID != m_strSubStock)
					ShowPicWithNewID(StockID);
				else
					SetSelectedPeriod(m_PicPeriod);

			}
		}
		else
		{
			if (m_PicPeriod != Period_FenShi)
				OnBtnFenShiClicked();
			else
				OnBtnDayClicked();
		}
	}
}
void CWorkWnd::OnRButtonUp(UINT nFlags, CPoint point)
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
		int nWndNum = m_pFenShiPic->GetShowSubPicNum();
		menu.CheckMenuItem(FM_PointWnd0 + nWndNum, MF_CHECKED);
		if (m_pFenShiPic->GetAvgState())
			menu.CheckMenuItem(FM_Avg, MF_CHECKED);
		if (m_pFenShiPic->GetEmaState())
			menu.CheckMenuItem(FM_EMA, MF_CHECKED);
		//int nSubPicNum = m_pFenShiPic->GetShowSubPicNum();
		//for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		//{
		//	if (m_pFenShiPic->GetRpsState(i))
		//		menu.CheckMenuItem(FM_EmaPara + i, MF_CHECKED);
		//}
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
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_CHECKED);
			menu.CheckMenuItem(KM_Volume, MF_CHECKED);
			menu.CheckMenuItem(KM_Amount, MF_UNCHECKED);
		}
		else if (m_pKlinePic->GetAmountState())
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_CHECKED);
			menu.CheckMenuItem(KM_Volume, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Amount, MF_CHECKED);
		}
		else
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Volume, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Amount, MF_UNCHECKED);
		}

		UINT VolState = m_pKlinePic->GetCAVolState() ? MF_CHECKED : MF_UNCHECKED;
		UINT AmoState = m_pKlinePic->GetCAAmoState() ? MF_CHECKED : MF_UNCHECKED;
		UINT AllState = (VolState || AmoState) ? MF_CHECKED : MF_UNCHECKED;
		menu.CheckMenuItem(KM_CAVolOrAmo, AllState);
		menu.CheckMenuItem(KM_CAVol, VolState);
		menu.CheckMenuItem(KM_CAAmo, AmoState);

		if (m_pKlinePic->GetMacdState())
			menu.CheckMenuItem(KM_MACD, MF_CHECKED);
		int nWndNum = m_pKlinePic->GetShowSubPicNum();
		//if (m_pKlinePic->GetRpsState(SP_FULLMARKET))
		menu.CheckMenuItem(KM_PointWnd0 + nWndNum, MF_CHECKED);
		if (m_pKlinePic->GetMaState())
			menu.CheckMenuItem(KM_MA, MF_CHECKED);
		if (m_pKlinePic->GetBandState())
			menu.CheckMenuItem(KM_Band, MF_CHECKED);
		//int nSubPicNum = m_pKlinePic->GetShowSubPicNum();
		//for (int i = SP_SWINDYL1; i < nSubPicNum; ++i)
		//{
		//	if (m_pKlinePic->GetRpsState(i))
		//		menu.CheckMenuItem(KM_MaPara + i, MF_CHECKED);
		//}

		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
		return;

	}
}
void CWorkWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	BOOL bTargetSelClicked = FALSE;
	if (m_pFenShiPic->IsVisible())
		bTargetSelClicked = m_pFenShiPic->CheckTargetSelectIsClicked(point);
	else if (m_pKlinePic->IsVisible())
		bTargetSelClicked = m_pKlinePic->CheckTargetSelectIsClicked(point);
	if (bTargetSelClicked)
	{
		SMenu menu;
		menu.LoadMenuW(L"smenu:menu_target");
		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
	}
}
BOOL CWorkWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
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
		std::vector < pair<SStringA, int>>ListPosVec(
			m_MouseWheelMap.hash.begin(), m_MouseWheelMap.hash.end());
		std::sort(ListPosVec.begin(), ListPosVec.end(),
			[](const pair<SStringA, int> &a, const pair<SStringA, int> b)
		{return a.second < b.second; });
		int i = 0;
		for (i; i < ListPosVec.size(); ++i)
		{
			if (m_strSubStock == ListPosVec[i].first)
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
		ShowPicWithNewID(ListPosVec[i].first);
	}
	return 0;
}
void CWorkWnd::SwitchPic2List()
{
	m_pFenShiPic->SetVisible(FALSE, TRUE);
	m_pKlinePic->SetVisible(FALSE, TRUE);
	m_pKlinePic->ClearTip();
	m_pList->SetVisible(TRUE, TRUE);
	m_pTextIndy->SetVisible(TRUE, TRUE);
	m_pBtnTitleSel->SetVisible(TRUE, TRUE);
	UpdateList();
	m_pList->SetFocus();
	m_pList->RequestRelayout();
	m_bShowList = true;
	m_pBtnMarket->SetWindowTextW(L"行情");
	if (Group_Stock == m_Group)
	{
		m_pBtnRehab->SetVisible(FALSE, TRUE);
		m_pCheckST->SetVisible(TRUE, TRUE);
		m_pCheckSBM->SetVisible(TRUE, TRUE);
		m_pCheckSTARM->SetVisible(TRUE, TRUE);
		m_pCheckNewStock->SetVisible(TRUE, TRUE);
	}


}
void CWorkWnd::SwitchList2Pic(int nPeriod)
{
	m_pList->SetVisible(FALSE, TRUE);
	m_pFenShiPic->SetVisible(FALSE, FALSE);
	m_pKlinePic->SetVisible(FALSE, FALSE);
	m_pTextIndy->SetVisible(FALSE, TRUE);
	m_pBtnTitleSel->SetVisible(FALSE, TRUE);

	if (nPeriod == Period_FenShi)
	{
		if (m_Group == Group_Stock)
			m_pBtnRehab->SetVisible(FALSE, TRUE);
		m_pFenShiPic->SetVisible(TRUE, TRUE);
		m_pFenShiPic->SetFocus();
		m_pFenShiPic->RequestRelayout();

	}
	else
	{
		m_pBtnRehab->SetVisible(TRUE, TRUE);
		if (m_Group == Group_Stock);
		m_pKlinePic->SetVisible(TRUE, TRUE);
		m_pKlinePic->SetFocus();
		m_pKlinePic->RequestRelayout();
	}
	m_bShowList = false;
	m_pBtnMarket->SetWindowTextW(L"个股");
	m_pCheckST->SetVisible(FALSE, TRUE);
	m_pCheckSBM->SetVisible(FALSE, TRUE);
	m_pCheckSTARM->SetVisible(FALSE, TRUE);
	m_pCheckNewStock->SetVisible(FALSE, TRUE);

}

void CWorkWnd::DataProc()
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
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}

}
void CWorkWnd::InitProcFucMap()
{
	m_dataHandleMap[WW_ListData] =
		&CWorkWnd::OnUpdateListData;
	m_dataHandleMap[WW_Point] =
		&CWorkWnd::OnUpdatePoint;
	//m_dataHandleMap[WW_TodayPoint] =
	//	&CWorkWnd::OnUpdateTodayPoint;
	m_dataHandleMap[WW_HisRpsPoint] =
		&CWorkWnd::OnUpdateHisRpsPoint;
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
	m_dataHandleMap[WW_ChangeIndy] =
		&CWorkWnd::OnChangeShowIndy;
	m_dataHandleMap[WW_FSEma] =
		&CWorkWnd::OnFenShiEma;
	m_dataHandleMap[WW_FSMacd] =
		&CWorkWnd::OnFenShiMacd;
	m_dataHandleMap[WW_KlineMa] =
		&CWorkWnd::OnKlineMa;
	m_dataHandleMap[WW_KlineMacd] =
		&CWorkWnd::OnKlineMacd;
	m_dataHandleMap[WW_KlineBand] =
		&CWorkWnd::OnKlineBand;
	m_dataHandleMap[WW_ChangeStockFilter] =
		&CWorkWnd::OnChangeStockFilter;
	m_dataHandleMap[WW_SaveStockFilter] =
		&CWorkWnd::OnSaveStockFilter;
	m_dataHandleMap[WW_HisSecPoint] =
		&CWorkWnd::OnUpdateHisSecPoint;
	m_dataHandleMap[WW_RehabInfo] =
		&CWorkWnd::OnUpdateRehabInfo;
	m_dataHandleMap[WW_ChangeRehab] =
		&CWorkWnd::OnChangeKlineRehab;
	m_dataHandleMap[WW_FixedTimeRehab] =
		&CWorkWnd::OnFixedTimeRehab;
	m_dataHandleMap[WW_HisCallAction] =
		&CWorkWnd::OnUpdateHisCallAction;

}
void CWorkWnd::InitNameVec()
{
	m_dataNameVec.emplace_back("close");
	m_dataNameVec.emplace_back("RPS520");
	m_dataNameVec.emplace_back("MACD520");
	m_dataNameVec.emplace_back("Point520");
	m_dataNameVec.emplace_back("Rank520");
	m_dataNameVec.emplace_back("RPS2060");
	m_dataNameVec.emplace_back("MACD2060");
	m_dataNameVec.emplace_back("Point2060");
	m_dataNameVec.emplace_back("Rank2060");


	m_PointDataCount[eRpsPoint] = 2;
	m_PointDataCount[eSecPoint] = 1;

	m_SubPicShowNameVec[eRpsPoint][""].emplace_back("Point520");
	m_SubPicShowNameVec[eRpsPoint][""].emplace_back("Point2060");
	m_SubPicShowNameVec[eRpsPoint]["L1"].emplace_back("Point520L1");
	m_SubPicShowNameVec[eRpsPoint]["L1"].emplace_back("Point2060L1");
	m_SubPicShowNameVec[eRpsPoint]["L2"].emplace_back("Point520L2");
	m_SubPicShowNameVec[eRpsPoint]["L2"].emplace_back("Point2060L2");

	m_SubPicShowNameVec[eSecPoint][""].emplace_back("Point");
	m_SubPicShowNameVec[eSecPoint]["L1"].emplace_back("PointL1");
	m_SubPicShowNameVec[eSecPoint]["L2"].emplace_back("PointL2");

}
void CWorkWnd::InitStockFilterFunc()
{

	m_SFPeriodMap[SFP_FS] = Period_FenShi;
	m_SFPeriodMap[SFP_M1] = Period_1Min;
	m_SFPeriodMap[SFP_M5] = Period_5Min;
	m_SFPeriodMap[SFP_M15] = Period_15Min;
	m_SFPeriodMap[SFP_M30] = Period_30Min;
	m_SFPeriodMap[SFP_M60] = Period_60Min;
	m_SFPeriodMap[SFP_D1] = Period_1Day;
	m_SFPeriodMap[SFP_Null] = Period_1Day;



	m_SFIndexMap[SFI_ChgPct] = "CHG";
	m_SFIndexMap[SFI_CloseRps520] = "closeRPS520";
	m_SFIndexMap[SFI_CloseMacd520] = "closeMACD520";
	m_SFIndexMap[SFI_ClosePoint520] = "closePOINT520";
	m_SFIndexMap[SFI_CloseRank520] = "closeRANK520";
	m_SFIndexMap[SFI_CloseRps2060] = "closeRPS2060";
	m_SFIndexMap[SFI_CloseMacd2060] = "closeMACD2060";
	m_SFIndexMap[SFI_ClosePoint2060] = "closePOINT2060";
	m_SFIndexMap[SFI_CloseRank2060] = "closeRANK2060";
	m_SFIndexMap[SFI_Num] = "Num";
	m_SFIndexMap[SFI_ABSR] = "ABSR";
	m_SFIndexMap[SFI_A2PBSR] = "A2PBSR";
	m_SFIndexMap[SFI_AABSR] = "AABSR";
	m_SFIndexMap[SFI_POCR] = "POCR";
	m_SFIndexMap[SFI_Vol] = "VOL";
	m_SFIndexMap[SFI_Amount] = "AMOUNT";
	m_SFIndexMap[SFI_AmountRps520] = "amountRPS520";
	m_SFIndexMap[SFI_AmountMacd520] = "amountMACD520";
	m_SFIndexMap[SFI_AmountPoint520] = "amountPOINT520";
	m_SFIndexMap[SFI_AmountRank520] = "amountRANK520";
	m_SFIndexMap[SFI_AmountRps2060] = "amountRPS2060";
	m_SFIndexMap[SFI_AmountMacd2060] = "amountMACD2060";
	m_SFIndexMap[SFI_AmountPoint2060] = "amountPOINT2060";
	m_SFIndexMap[SFI_AmountRank2060] = "amountRANK2060";
	m_SFIndexMap[SFI_AmountPoint] = "amountPOINT";
	m_SFIndexMap[SFI_AmountRank] = "amountRANK";

	m_SFIndexMap[SFI_ClosePoint520L1] = "closePOINT520L1";
	m_SFIndexMap[SFI_CloseRank520L1] = "closeRANK520L1";
	m_SFIndexMap[SFI_ClosePoint2060L1] = "closePOINT2060L1";
	m_SFIndexMap[SFI_CloseRank2060L1] = "closeRANK2060L1";
	m_SFIndexMap[SFI_AmountPoint520L1] = "amountPOINT520L1";
	m_SFIndexMap[SFI_AmountRank520L1] = "amountRANK520L1";
	m_SFIndexMap[SFI_AmountPoint2060L1] = "amountPOINT2060L1";
	m_SFIndexMap[SFI_AmountRank2060L1] = "amountRANK2060L1";
	m_SFIndexMap[SFI_AmountPointL1] = "amountPOINTL1";
	m_SFIndexMap[SFI_AmountRankL1] = "amountRANKL1";

	m_SFIndexMap[SFI_ClosePoint520L2] = "closePOINT520L2";
	m_SFIndexMap[SFI_CloseRank520L2] = "closeRANK520L2";
	m_SFIndexMap[SFI_ClosePoint2060L2] = "closePOINT2060L2";
	m_SFIndexMap[SFI_CloseRank2060L2] = "closeRANK2060L2";
	m_SFIndexMap[SFI_AmountPoint520L2] = "amountPOINT520L2";
	m_SFIndexMap[SFI_AmountMacd2060L2] = "amountMACD2060L2";
	m_SFIndexMap[SFI_AmountPoint2060L2] = "amountPOINT2060L2";
	m_SFIndexMap[SFI_AmountRank2060L2] = "amountRANK2060L2";
	m_SFIndexMap[SFI_AmountPointL2] = "amountPOINTL2";
	m_SFIndexMap[SFI_AmountRankL2] = "amountRANKL2";

	m_SFIndexMap[SFI_ABV] = "ABV";
	m_SFIndexMap[SFI_ASV] = "ASV";
	m_SFIndexMap[SFI_ABO] = "ABO";
	m_SFIndexMap[SFI_ASO] = "ASO";
	m_SFIndexMap[SFI_PBO] = "PBO";
	m_SFIndexMap[SFI_PSO] = "PSO";


	m_SFIndexMap[SFI_Open] = "OPEN";
	m_SFIndexMap[SFI_High] = "HIGH";
	m_SFIndexMap[SFI_Low] = "LOW";
	m_SFIndexMap[SFI_Close] = "CLOSE";

	m_SFIndexMap[SFI_CAVol] = "CAVOL";
	m_SFIndexMap[SFI_CAVolPoint] = "CAVOLPOINT";
	m_SFIndexMap[SFI_CAVolRank] = "CAVOLRANK";
	m_SFIndexMap[SFI_CAVolPointL1] = "CAVOLPOINTL1";
	m_SFIndexMap[SFI_CAVolRankL1] = "CAVOLRANKL1";
	m_SFIndexMap[SFI_CAVolPointL2] = "CAVOLPOINTL2";
	m_SFIndexMap[SFI_CAVolRankL2] = "CAVOLRANKL2";

	m_SFIndexMap[SFI_CAAmo] = "CAAMO";
	m_SFIndexMap[SFI_CAAmoPoint] = "CAAMOPOINT";
	m_SFIndexMap[SFI_CAAmoRank] = "CAAMORANK";
	m_SFIndexMap[SFI_CAAmoPointL1] = "CAAMOPOINTL1";
	m_SFIndexMap[SFI_CAAmoRankL1] = "CAAMORANKL1";
	m_SFIndexMap[SFI_CAAmoPointL2] = "CAAMOPOINTL2";
	m_SFIndexMap[SFI_CAAmoRankL2] = "CAAMORANKL2";



	m_SFIndexMap[SFI_L1IndyCloseRps520] = "closeRPS520";
	m_SFIndexMap[SFI_L1IndyCloseMacd520] = "closeMACD520";
	m_SFIndexMap[SFI_L1IndyClosePoint520] = "closePOINT520";
	m_SFIndexMap[SFI_L1IndyCloseRank520] = "closeRANK520";
	m_SFIndexMap[SFI_L1IndyCloseRps2060] = "closeRPS2060";
	m_SFIndexMap[SFI_L1IndyCloseMacd2060] = "closeMACD2060";
	m_SFIndexMap[SFI_L1IndyClosePoint2060] = "closePOINT2060";
	m_SFIndexMap[SFI_L1IndyCloseRank2060] = "closeRANK2060";
	m_SFIndexMap[SFI_L1IndyAmountRps520] = "amountRPS520";
	m_SFIndexMap[SFI_L1IndyAmountMacd520] = "amountMACD520";
	m_SFIndexMap[SFI_L1IndyAmountPoint520] = "amountPOINT520";
	m_SFIndexMap[SFI_L1IndyAmountRank520] = "amountRANK520";
	m_SFIndexMap[SFI_L1IndyAmountRps2060] = "amountRPS2060";
	m_SFIndexMap[SFI_L1IndyAmountMacd2060] = "amountMACD2060";
	m_SFIndexMap[SFI_L1IndyAmountPoint2060] = "amountPOINT2060";
	m_SFIndexMap[SFI_L1IndyAmountRank2060] = "amountRANK2060";
	m_SFIndexMap[SFI_L1IndyAmountPoint] = "amountPOINT";
	m_SFIndexMap[SFI_L1IndyAmountRank] = "amountRANK";

	m_SFIndexMap[SFI_L2IndyCloseRps520] = "closeRPS520";
	m_SFIndexMap[SFI_L2IndyCloseMacd520] = "closeMACD520";
	m_SFIndexMap[SFI_L2IndyClosePoint520] = "closePOINT520";
	m_SFIndexMap[SFI_L2IndyCloseRank520] = "closeRANK520";
	m_SFIndexMap[SFI_L2IndyCloseRps2060] = "closeRPS2060";
	m_SFIndexMap[SFI_L2IndyCloseMacd2060] = "closeMACD2060";
	m_SFIndexMap[SFI_L2IndyClosePoint2060] = "closePOINT2060";
	m_SFIndexMap[SFI_L2IndyCloseRank2060] = "closeRANK2060";
	m_SFIndexMap[SFI_L2IndyAmountRps520] = "amountRPS520";
	m_SFIndexMap[SFI_L2IndyAmountMacd520] = "amountMACD520";
	m_SFIndexMap[SFI_L2IndyAmountPoint520] = "amountPOINT520";
	m_SFIndexMap[SFI_L2IndyAmountRank520] = "amountRANK520";
	m_SFIndexMap[SFI_L2IndyAmountRps2060] = "amountRPS2060";
	m_SFIndexMap[SFI_L2IndyAmountMacd2060] = "amountMACD2060";
	m_SFIndexMap[SFI_L2IndyAmountPoint2060] = "amountPOINT2060";
	m_SFIndexMap[SFI_L2IndyAmountRank2060] = "amountRANK2060";
	m_SFIndexMap[SFI_L2IndyAmountPoint] = "amountPOINT";
	m_SFIndexMap[SFI_L2IndyAmountRank] = "amountRANK";

	m_SFConditionMap[SFC_Greater] =
		&CWorkWnd::GreaterThan;
	m_SFConditionMap[SFC_EqualOrGreater] =
		&CWorkWnd::EqualOrGreaterThan;
	m_SFConditionMap[SFC_Equal] =
		&CWorkWnd::Equal;
	m_SFConditionMap[SFC_EqualOrLess] =
		&CWorkWnd::EqualOrLessThan;
	m_SFConditionMap[SFC_Less] =
		&CWorkWnd::LessThan;

}

bool CWorkWnd::OnListHeaderClick(EventArgs * pEvtBase)
{

	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SHeaderCtrlEx *pHeader = (SHeaderCtrlEx*)pEvt->sender;
	SHDITEM hditem;
	hditem.mask = SHDI_ORDER;
	pHeader->GetItem(pEvt->iItem, &hditem);
	int nShowOrder = pHeader->GetOriItemIndex(hditem.iOrder);
	if (hditem.iOrder == 0)
		return false;

	SColorListCtrlEx * pList =
		(SColorListCtrlEx *)pHeader->GetParent();

	pHeader->SetItemSort(m_SortPara.nShowCol, ST_NULL);
	if (m_ListDataDecMap.count((SListHead)hditem.iOrder) == 0)
		m_SortPara.nDec = 3;
	else
		m_SortPara.nDec = m_ListDataDecMap[(SListHead)hditem.iOrder];

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
bool CWorkWnd::OnListHeaderSwap(EventArgs * pEvtBase)
{
	EventHeaderItemSwap *pEvt = (EventHeaderItemSwap*)pEvtBase;
	SHeaderCtrlEx* pHead = (SHeaderCtrlEx*)pEvt->sender;
	int nColCount = pHead->GetItemCount();
	//SColorListCtrlEx * pList =
	//	(SColorListCtrlEx *)pHead->GetParent();

	for (int i = 0; i < nColCount; ++i)
	{
		m_TitleOrderMap[i] = pHead->GetOriItemIndex(i);
		if (m_SortPara.nCol == i)
		{
			if (m_SortPara.nShowCol != m_TitleOrderMap[i])
			{
				pHead->SetItemSort(m_SortPara.nShowCol, ST_NULL);
				m_SortPara.nShowCol = m_TitleOrderMap[i];
				if (m_SortPara.nFlag == 0)
					pHead->SetItemSort(m_SortPara.nShowCol, ST_UP);
				else
					pHead->SetItemSort(m_SortPara.nShowCol, ST_DOWN);

			}

		}
	}

	::SendMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SaveListConfig, 0);
	return true;
}
bool CWorkWnd::OnListDbClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID =
		StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (m_ListPosMap.hash.count(StockID) == 0)
		StockID += "I";

	m_PicPeriod = m_ListPeriod;
	if (StockID != m_strSubStock)
		ShowPicWithNewID(StockID);
	else
		SetSelectedPeriod(m_PicPeriod);
	return true;
}
bool CWorkWnd::OnListLClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	if (m_Group == Group_Stock)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA strID =
		StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (strID[0] == '0')
		strID += "I";
	int nGroup = m_Group;
	char msg[12] = "";
	memcpy_s(msg, 12, &nGroup, 4);
	memcpy_s(msg + 4, 12, strID, strID.GetLength() + 1);
	SendMsg(m_uParWndThreadID, WW_ChangeIndy, msg, 12);
	return true;
}
void CWorkWnd::SetListDataIsShow()
{
	SHeaderCtrlEx *pHead = (SHeaderCtrlEx *)m_pList->GetWindow(GSW_FIRSTCHILD);
	for (auto &it : m_TitleShowMap)
		pHead->SetItemShowVisible(m_TitleOrderMap[it.first], it.second);
	m_pList->UpdateScrollBar();
	m_pList->RequestRelayout();
	m_pList->Invalidate();
}
void CWorkWnd::SetListDataOrder()
{
	SHeaderCtrlEx *pHead = (SHeaderCtrlEx *)m_pList->GetWindow(GSW_FIRSTCHILD);
	for (auto &it : m_TitleOrderMap)
		pHead->SetOriItemIndex(it.first, it.second);
	m_pList->UpdateScrollBar();
	m_pList->RequestRelayout();
	m_pList->Invalidate();

}
void CWorkWnd::UpdateListShowStock()
{
	SStringA strInd = m_ListShowInd;
	m_ListPosMap.hash.clear();
	m_pList->DeleteAllItems();
	int nCount = 0;
	SStringW tmp;
	for (auto &it : m_InfoVec)
	{
		if (Group_Stock == m_Group)
			if (!CheckStockFitDomain(it))
				continue;

		if (strInd == "" || strInd == it.ScaleID ||
			strInd == it.SWL1ID || strInd == it.SWL2ID)
		{
			tmp.Format(L"%d", nCount + 1);
			m_pList->InsertItem(nCount, tmp);
			SStringW strID = StrA2StrW(it.SecurityID);
			if (strID.GetLength() > 6)
				strID = strID.Left(6);
			m_pList->SetSubItemText(nCount, SHead_ID,
				strID);
			m_pList->SetSubItemText(nCount, SHead_Name,
				StrA2StrW(it.SecurityName));
			m_ListPosMap.hash[it.SecurityID] = nCount;
			nCount++;
		}
	}
	SetListShowIndyStr(m_pTextIndy);

	UpdateList();
	//m_pList->UpdateLayout();
	m_pList->RequestRelayout();
}

void CWorkWnd::UpdateList()
{
	if (!IsVisible())
		return;
	if (!m_bListInited)
		return;
	if (!m_pList->IsVisible())
		return;
	SStringW tmp;
	if (m_pListDataMap->count(m_ListPeriod) == 0)
	{
		for (auto &it : m_ListPosMap.hash)
		{
			m_pList->SetSubItemText(it.second, SHead_LastPx, L"-");
			for (int i = SHead_ChangePct; i < SHead_CommonItmeCount; ++i)
				m_pList->SetSubItemText(it.second, i, L"-");
		}
		return;
	}
	auto ListData = m_pListDataMap->at(m_ListPeriod);
	auto TfData = m_pTFMarketHash ?
		m_pTFMarketHash->at(m_ListPeriod) : strHash<TickFlowMarket>();
	//int nFirstIndex = m_pList->GetTopIndex();
	//int nPerPageCount = m_pList->GetCountPerPage(TRUE);
	//int nLastIndex = nFirstIndex + nPerPageCount;
	m_pList->LockUpdate();
	for (auto &it : m_ListPosMap.hash)
	{
		SStringA StockID = it.first;
		//int nPos = it.second;
		//if (nPos < nFirstIndex || nPos > nLastIndex)
		//	continue;
		if (ListData.hash.count(StockID))
		{
			::EnterCriticalSection(&m_csClose);
			double fPreClose = m_preCloseMap.hash[StockID];
			::LeaveCriticalSection(&m_csClose);

			auto & rtData = ListData.hash[StockID];
			if (rtData.fPrice != 0)
			{
				COLORREF cl = RGBA(255, 255, 255, 255);
				if (rtData.fPrice > fPreClose)
					cl = RGBA(255, 0, 0, 255);
				else if (rtData.fPrice < fPreClose)
					cl = RGBA(0, 255, 0, 255);
				tmp.Format(L"%.2f", rtData.fPrice);
				m_pList->SetSubItemText(it.second, SHead_LastPx, tmp, cl);
				double chgPct = (rtData.fPrice - fPreClose) / fPreClose * 100;
				if (!isnan(chgPct) && !isinf(chgPct))
					tmp.Format(L"%.2f", (rtData.fPrice - fPreClose) / fPreClose * 100);
				else
					tmp = L"-";
				m_pList->SetSubItemText(it.second, SHead_ChangePct, tmp, cl);
				UpdateListRpsData(it.second, rtData.rpsClose, SHead_CloseRPS520, SHead_CloseRank2060);
				//if (isnan(rtData.rpsAmount.fMacd520) || isinf(rtData.rpsAmount.fMacd520))
				UpdateListRpsData(it.second, rtData.rpsAmount, SHead_AmountRPS520, SHead_AmountRank2060);
				UpdateListSecData(it.second, rtData.secAmount, SHead_AmountPoint, SHead_AmountRank);
				if (m_pCallActionHash->hash.count(StockID))
					UpdateListCAData(it.second, m_pCallActionHash->hash[StockID]);

				if (m_Group == Group_Stock)
				{
					if (TfData.hash.count(StockID))
						UpdateListTFData(it.second, TfData.hash[StockID], fPreClose);
				}

			}
			else
			{
				if (rtData.fPrice == 0)
					m_pList->SetSubItemText(it.second, SHead_LastPx, L"0.00");
				else
					m_pList->SetSubItemText(it.second, SHead_LastPx, L"-");
				for (int i = SHead_ChangePct; i < SHead_CommonItmeCount; ++i)
					m_pList->SetSubItemText(it.second, i, L"-");
				if (m_Group == Group_Stock)
				{
					for (int i = SHead_TickFlowStart; i < SHead_StockItemCount; ++i)
						m_pList->SetSubItemText(it.second, i, L"-");

				}

				continue;
			}
		}
		else
		{
			m_pList->SetSubItemText(it.second, SHead_LastPx, L"-");
			for (int i = SHead_ChangePct; i < SHead_CommonItmeCount; ++i)
				m_pList->SetSubItemText(it.second, i, L"-");
		}

	}
	m_pList->UnlockUpdate();
	SortList(m_pList);

}

void SOUI::CWorkWnd::SetPriceListHalf(bool bHalf)
{
	m_pFenShiPic->SetPriceListHalf(bHalf);
	m_pKlinePic->SetPriceListHalf(bHalf);

}

void CWorkWnd::UpdateListRpsData(int nRow, sRps & rps, int nStart, int nEnd)
{
	vector<double> dataVec(SHead_CloseRank2060 - SHead_CloseRPS520 + 1);
	dataVec[0] = rps.fRps520;
	dataVec[1] = rps.fMacd520;
	dataVec[2] = rps.fPoint520;
	dataVec[3] = rps.nRank520;
	dataVec[4] = rps.fRps2060;
	dataVec[5] = rps.fMacd2060;
	dataVec[6] = rps.fPoint2060;
	dataVec[7] = rps.nRank2060;
	SStringW tmp;
	for (int i = nStart; i <= nEnd; ++i)
	{
		int nOffset = (i - SHead_CloseRPS520) %
			(SHead_CloseRank2060 - SHead_CloseRPS520 + 1) + SHead_CloseRPS520;
		COLORREF cl = RGBA(255, 255, 0, 255);

		if (nOffset == SHead_ClosePoint520
			|| nOffset == SHead_ClosePoint2060)
		{
			if (dataVec[i - nStart] >= 80)
				cl = RGBA(255, 0, 0, 255);
			else if (dataVec[i - nStart] < 60)
				cl = RGBA(0, 255, 0, 255);
			else
				cl = RGBA(255, 255, 255, 255);
		}
		else if (nOffset == SHead_CloseRPS520
			|| nOffset == SHead_CloseRPS2060
			|| nOffset == SHead_CloseMACD520
			|| nOffset == SHead_CloseMACD2060)
		{
			if (dataVec[i - nStart] > 0)
				cl = RGBA(255, 0, 0, 255);
			else if (dataVec[i - nStart] < 0)
				cl = RGBA(0, 255, 0, 255);
			else
				cl = RGBA(255, 255, 255, 255);
		}

		if (nOffset == SHead_CloseRank520 ||
			nOffset == SHead_CloseRank2060)
			tmp.Format(L"%.0f", dataVec[i - nStart]);
		else if (m_ListDataSortMap[(SListHead)i] == eSDT_BigDouble)
		{
			double fAbsData = abs(dataVec[i - nStart]);
			if (fAbsData > 1'000'000'000)
				tmp.Format(L"%.01f亿", fAbsData / 100'000'000);
			else if (fAbsData > 100'000'000)
				tmp.Format(L"%.02f亿", fAbsData / 100'000'000);
			else if (fAbsData > 1'000'000)
				tmp.Format(L"%.0f万", fAbsData / 10000);
			else if (fAbsData > 10'000)
				tmp.Format(L"%.02f万", fAbsData / 10000);
			else
				tmp.Format(L"%.0f", fAbsData);

		}
		else
			tmp.Format(L"%.03f", dataVec[i - nStart]);
		m_pList->SetSubItemText(nRow,
			i, tmp, cl);
	}

}

void CWorkWnd::UpdateListSecData(int nRow, sSection &sec, int nStart, int nEnd)
{
	vector<double> dataVec(SHead_AmountRank - SHead_AmountPoint + 1);
	dataVec[0] = sec.point;
	dataVec[1] = sec.rank;
	SStringW tmp;
	for (int i = nStart; i <= nEnd; ++i)
	{
		int nOffset = (i - SHead_AmountPoint) %
			(SHead_AmountRank - SHead_AmountPoint + 1) + SHead_AmountPoint;
		COLORREF cl = RGBA(255, 255, 0, 255);

		if (nOffset == SHead_AmountPoint)
		{
			if (dataVec[i - nStart] >= 80)
				cl = RGBA(255, 0, 0, 255);
			else if (dataVec[i - nStart] < 60)
				cl = RGBA(0, 255, 0, 255);
			else
				cl = RGBA(255, 255, 255, 255);
		}

		if (nOffset == SHead_AmountRank)
			tmp.Format(L"%.0f", dataVec[i - nStart]);
		else
			tmp.Format(L"%.03f", dataVec[i - nStart]);
		m_pList->SetSubItemText(nRow,
			i, tmp, cl);
	}

}

void CWorkWnd::UpdateListCAData(int nRow, CAInfo& caData)
{
	vector<double> dataVec(SHead_CAAmoRank - SHead_CAVol + 1);
	dataVec[0] = caData.Volume;
	dataVec[1] = caData.VolPoint;
	dataVec[2] = caData.VolRank;
	dataVec[3] = caData.Amount;
	dataVec[4] = caData.AmoPoint;
	dataVec[5] = caData.AmoRank;
	SStringW tmp;
	for (int i = SHead_CAVol; i <= SHead_CAAmoRank; ++i)
	{
		COLORREF cl = RGBA(255, 255, 0, 255);

		if (i == SHead_CAVolPoint
			|| i == SHead_CAAmoPoint)
		{
			if (dataVec[i - SHead_CAVol] >= 80)
				cl = RGBA(255, 0, 0, 255);
			else if (dataVec[i - SHead_CAVol] < 60)
				cl = RGBA(0, 255, 0, 255);
			else
				cl = RGBA(255, 255, 255, 255);
		}
		if (i == SHead_CAVol ||
			i == SHead_CAVolRank ||
			i == SHead_CAAmoRank)
			tmp.Format(L"%.0f", dataVec[i - SHead_CAVol]);
		else if (m_ListDataSortMap[(SListHead)i] == eSDT_BigDouble)
		{
			double fAbsData = abs(dataVec[i - SHead_CAVol]);
			if (fAbsData > 1'000'000'000)
				tmp.Format(L"%.01f亿", fAbsData / 100'000'000);
			else if (fAbsData > 100'000'000)
				tmp.Format(L"%.02f亿", fAbsData / 100'000'000);
			else if (fAbsData > 1'000'000)
				tmp.Format(L"%.0f万", fAbsData / 10000);
			else if (fAbsData > 10'000)
				tmp.Format(L"%.02f万", fAbsData / 10000);
			else
				tmp.Format(L"%.0f", fAbsData);

		}
		else
			tmp.Format(L"%.03f", dataVec[i - SHead_CAVol]);
		m_pList->SetSubItemText(nRow,
			i, tmp, cl);
	}
}

void CWorkWnd::UpdateListTFData(int nRow, TickFlowMarket& tfData, double fPreClose)
{
	vector<double> dataVec(SHead_StockItemCount - SHead_TickFlowStart);
	dataVec[0] = tfData.ABSR;
	dataVec[1] = tfData.A2PBSR;
	dataVec[2] = tfData.AABSR;
	dataVec[3] = tfData.POCR;
	dataVec[4] = tfData.nVolume;
	dataVec[5] = tfData.ActSellVol;
	dataVec[6] = tfData.ActBuyVol;
	dataVec[7] = tfData.fOpen;
	dataVec[8] = tfData.fHigh;
	dataVec[9] = tfData.fLow;
	dataVec[10] = tfData.fAmount;
	SStringW tmp;
	for (int i = 0; i < dataVec.size(); ++i)
	{
		COLORREF cl = RGBA(255, 255, 0, 255);
		if (i + SHead_TickFlowStart == SHead_ActSellVolume)
			cl = RGBA(0, 255, 0, 255);
		else if (i + SHead_TickFlowStart == SHead_ActBuyVolume)
			cl = RGBA(255, 0, 0, 255);
		else if (i + SHead_TickFlowStart == SHead_Open ||
			i + SHead_TickFlowStart == SHead_High ||
			i + SHead_TickFlowStart == SHead_Low)
		{
			if (dataVec[i] > fPreClose)
				cl = RGBA(255, 0, 0, 255);
			else if (dataVec[i] < fPreClose)
				cl = RGBA(0, 255, 0, 255);
			else
				cl = RGBA(255, 255, 255, 255);
		}
		if (i + SHead_TickFlowStart == SHead_ActBuyVolume ||
			i + SHead_TickFlowStart == SHead_ActSellVolume ||
			i + SHead_TickFlowStart == SHead_Volume)
		{
			m_pList->SetSubItemText(nRow,
				i + SHead_TickFlowStart,
				tmp.Format(L"%.0f", dataVec[i]), cl);
		}
		else if (i + SHead_TickFlowStart == SHead_Amount)
		{
			double fAmount = dataVec[i];
			if (fAmount > 1'000'000'000)
				tmp.Format(L"%.01f亿", fAmount / 100'000'000);
			else if (fAmount > 100'000'000)
				tmp.Format(L"%.02f亿", fAmount / 100'000'000);
			else if (fAmount > 1'000'000)
				tmp.Format(L"%.0f万", fAmount / 10000);
			else if (fAmount > 10'000)
				tmp.Format(L"%.02f万", fAmount / 10000);
			else
				tmp.Format(L"%.0f", fAmount);
			m_pList->SetSubItemText(nRow,
				i + SHead_TickFlowStart,
				tmp, cl);

		}
		else
		{
			if (!isnan(dataVec[i]) && !isinf(dataVec[i]))
			{
				m_pList->SetSubItemText(nRow,
					i + SHead_TickFlowStart,
					tmp.Format(L"%.02f", dataVec[i]), cl);

			}
			else
				m_pList->SetSubItemText(nRow,
					i + SHead_TickFlowStart,
					L"-", cl);

		}


	}

}

void CWorkWnd::UpdateListFilterShowStock()
{
	m_ListPosMap.hash.clear();
	//m_pList->DeleteAllItems();
	int nSelect = m_pList->GetSelectedItem();
	int nNowItemCount = m_pList->GetItemCount();
	int nCount = 0;
	SStringW tmp;
	for (auto &it : m_InfoVec)
	{
		if (Group_Stock == m_Group)
			if (!CheckStockFitDomain(it))
				continue;

		//2023.5.4 修改回退版本
		//bool bPassed = m_sfPlan.state == CS_And ? true : false;
		bool bPassed = true;
		for (auto &sf : m_sfVec)
		{
			if (!CheckCmbStockDataPass(sf, it.SecurityID))
			{
				bPassed = false;
				break;
			}
		}
		//for (auto &sf : m_sfPlan.condVec)
		//{
		//	if (m_sfPlan.state == CS_And)
		//	{
		//		if (!CheckStockDataPass(sf, it.SecurityID))
		//		{
		//			bPassed = false;
		//			break;
		//		}
		//	}
		//	else
		//	{
		//		if (CheckStockDataPass(sf, it.SecurityID))
		//		{
		//			bPassed = true;
		//			break;
		//		}
		//	}
		//}
		if (bPassed)
		{
			if (nCount >= nNowItemCount)
			{
				tmp.Format(L"%d", nCount + 1);
				m_pList->InsertItem(nCount, tmp);
				SStringW strID = StrA2StrW(it.SecurityID);
				if (strID.GetLength() > 6)
					strID = strID.Left(6);
				m_pList->SetSubItemText(nCount, SHead_ID,
					strID);
				m_pList->SetSubItemText(nCount, SHead_Name,
					StrA2StrW(it.SecurityName));
				m_ListPosMap.hash[it.SecurityID] = nCount;
			}
			else
			{
				tmp.Format(L"%d", nCount + 1);
				m_pList->SetSubItemText(nCount, SHead_ID, tmp);
				SStringW strID = StrA2StrW(it.SecurityID);
				if (strID.GetLength() > 6)
					strID = strID.Left(6);
				m_pList->SetSubItemText(nCount, SHead_ID,
					strID);
				m_pList->SetSubItemText(nCount, SHead_Name,
					StrA2StrW(it.SecurityName));
				m_ListPosMap.hash[it.SecurityID] = nCount;

			}
			nCount++;
		}
	}
	while (nNowItemCount > nCount)
	{
		m_pList->DeleteItem(nCount);
		--nNowItemCount;
	}
	m_pList->SetSelectedItem(nSelect < nCount ? nSelect : -1);
	SetListShowIndyStr(m_pTextIndy);

	m_pList->RequestRelayout();
}

void CWorkWnd::SortList(SColorListCtrlEx * pList, bool bSortCode)
{

	if (!bSortCode)
	{
		if (m_SortPara.nCol == 0
			|| m_SortPara.nCol == 1
			|| m_SortPara.nCol == 2)
		{
			m_MouseWheelMap.hash = m_ListPosMap.hash;
			pList->Invalidate();
			return;
		}
	}

	int colCount = pList->GetColumnCount();
	//int SHead_Time = colCount - 1;
	auto sortDataType = m_ListDataSortMap[(SListHead)m_SortPara.nCol];
	switch (sortDataType)
	{
	case eSDT_Double:
		pList->SortItems(SortDouble, &m_SortPara);
		break;
	case eSDT_Int:
		pList->SortItems(SortInt, &m_SortPara);
		break;
	case eSDT_String:
		pList->SortItems(SortStr, &m_SortPara);
		break;
	case eSDT_BigDouble:
		pList->SortItems(SortBigDouble, &m_SortPara);
		break;
	default:
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
		if (m_ListPosMap.hash.count(stockID) == 0)
			stockID += "I";
		m_ListPosMap.hash[stockID] = i;
	}
	m_MouseWheelMap.hash = m_ListPosMap.hash;
	m_bListInited = true;
}

bool CWorkWnd::CheckStockFitDomain(StockInfo & si)
{
	if (!m_bListShowST)
	{
		if (NULL != strstr(si.SecurityName, "ST"))
			return false;
	}
	//检测创业板
	if (!m_bListShowSBM)
	{
		if (si.SecurityID[0] == '3')
			return false;
	}
	//检测科创板
	if (!m_bListShowSTARM)
	{
		if (si.SecurityID[0] == '6' &&
			si.SecurityID[1] == '8' &&
			si.SecurityID[2] == '8')
			return false;
	}
	//检测新股
	if (!m_bListShowNewStock)
	{
		if (m_nDate - si.IPODate < 10000)
			return false;
	}
	return true;
}

bool CWorkWnd::CheckStockDataPass(SFCondition& sf, SStringA StockID)
{
	if (m_pFilterDataMap == nullptr)
		return false;
	if (m_pFilterDataMap->count(sf.nPeriod) == 0)
		return false;
	if (m_pFilterDataMap->at(sf.nPeriod).hash.count(StockID) == 0)
		return false;
	const auto &sysVar = m_pFilterDataMap->at(sf.nPeriod).hash[StockID];
	const auto & frml = CFrmlManager::GetFormula(sf.frml);
	map<string, double> uservar;
	//map<string, double> testVar;
	for (int i = 0; i < frml.para.size(); ++i)
		uservar[frml.para[i]] = sf.paraVec[i];
	for (auto &it : frml.formula)
	{
		if (it.type == Snt_Assign)
		{
			uservar[it.paraName] = it.formula.Res(sysVar, uservar);
		}
		else if (it.formula.Res(sysVar, uservar))
		{
			//OutputDebugStringFormat("Stock:%s 符合该条件\n", StockID);
			return true;
		}

	}

	return false;
}

bool CWorkWnd::CheckCmbStockDataPass(StockFilter & sf, SStringA StockID)
{
	if (m_pFilterDataMap == nullptr)
		return false;

	int nPeriod1 = m_SFPeriodMap[sf.period1];
	int nPeriod2 = m_SFPeriodMap[sf.period2];
	SStringA StockID1 = "";
	SStringA StockID2 = "";

	map<int, strHash<map<string, double>>> *pFilter1 = nullptr;
	if (sf.index1 >= SFI_Start && sf.index1 < SFI_Count)
	{
		pFilter1 = m_pFilterDataMap;
		StockID1 = StockID;
	}
	else if (sf.index1 >= SFI_L1IndyStart && sf.index1 < SFI_L1IndyCount)
	{
		pFilter1 = m_pL1IndyFilterDataMap;
		StockID1 = m_infoMap.hash[StockID].SWL1ID;
	}
	else if (sf.index1 >= SFI_L2IndyStart && sf.index1 < SFI_L2IndyCount)
	{
		pFilter1 = m_pL2IndyFilterDataMap;
		StockID1 = m_infoMap.hash[StockID].SWL2ID;
	}
	map<int, strHash<map<string, double>>> *pFilter2 = nullptr;
	if (sf.index2 >= SFI_Start && sf.index2 < SFI_Count)
	{
		pFilter2 = m_pFilterDataMap;
		StockID2 = StockID;
	}
	else if (sf.index2 >= SFI_L1IndyStart && sf.index2 < SFI_L1IndyCount)
	{
		pFilter2 = m_pL1IndyFilterDataMap;
		StockID2 = m_infoMap.hash[StockID].SWL1ID;
	}
	else if (sf.index2 >= SFI_L2IndyStart && sf.index2 < SFI_L2IndyCount)
	{
		pFilter2 = m_pL2IndyFilterDataMap;
		StockID2 = m_infoMap.hash[StockID].SWL2ID;
	}

	if (pFilter1 == nullptr || pFilter2 == nullptr)
		return false;

	if (pFilter1->count(nPeriod1) == 0)
		return false;
	if (pFilter2->count(nPeriod2) == 0)
		return false;

	if (pFilter1->at(nPeriod1).hash.count(StockID1) == 0)
		return false;
	if (pFilter2->at(nPeriod2).hash.count(StockID2) == 0)
		return false;

	double data1 = pFilter1->at(nPeriod1).hash[StockID1][m_SFIndexMap[sf.index1]];
	double data2 = sf.index2 == SFI_Num ? sf.num :
		pFilter2->at(nPeriod2).hash[StockID2][m_SFIndexMap[sf.index2]];
	if (sf.index1 == SFI_Amount || sf.index1 == SFI_CAAmo)
		data1 /= 10000;

	if (sf.index2 == SFI_Amount || sf.index1 == SFI_CAAmo)
		data2 /= 10000;
	return (this->*m_SFConditionMap[sf.condition])(data1, data2);
}

bool CWorkWnd::GreaterThan(double a, double b)
{
	return a > b;
}

bool CWorkWnd::EqualOrGreaterThan(double a, double b)
{
	return a >= b;
}

bool CWorkWnd::Equal(double a, double b)
{
	return (a == b) || (abs(a - b) < MIN_DIFF);
}

bool CWorkWnd::EqualOrLessThan(double a, double b)
{
	return a <= b;
}

bool CWorkWnd::LessThan(double a, double b)
{
	return a < b;
}


int CWorkWnd::SortDouble(void * para1, const void * para2,
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
				const DXLVSUBITEMEX CodeItem1 =
					pPara1->arSubItems->GetAt(SHead_ID);
				const DXLVSUBITEMEX CodeItem2 =
					pPara2->arSubItems->GetAt(SHead_ID);

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

int CWorkWnd::SortInt(void * para1, const void * para2,
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

int CWorkWnd::SortStr(void * para1, const void * para2,
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

int CWorkWnd::SortBigDouble(void * para1, const void * para2, const void * para3)
{
	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);

	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;

	int64_t n1;
	if (str1.Right(1) == L"亿")
		n1 = _wtof(str1) * 100'000'000;
	else if (str1.Right(1) == L"万")
		n1 = _wtof(str1) * 10'000;
	else
		n1 = _wtof(str1);
	int64_t n2;
	if (str2.Right(1) == L"亿")
		n2 = _wtof(str2) * 100'000'000;
	else if (str2.Right(1) == L"万")
		n2 = _wtof(str2) * 10'000;
	else
		n2 = _wtof(str2);
	int64_t diff = n1 - n2;
	if (diff > 0)
		return pData->nFlag == 0 ? 1 : -1;
	else if (diff < 0)
		return pData->nFlag == 0 ? -1 : 1;
	else
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);
	}
}


void CWorkWnd::OnBtnMarketClicked()
{
	OnBtnShowTypeChange();
}

void CWorkWnd::OnBtnFenShiClicked()
{
	OnBtnPeriedChange(Period_FenShi);
}

void CWorkWnd::OnBtnM1Clicked()
{
	OnBtnPeriedChange(Period_1Min);
}

void CWorkWnd::OnBtnM5Clicked()
{
	OnBtnPeriedChange(Period_5Min);

}

void CWorkWnd::OnBtnM15Clicked()
{
	OnBtnPeriedChange(Period_15Min);

}

void CWorkWnd::OnBtnM30Clicked()
{
	OnBtnPeriedChange(Period_30Min);

}

void CWorkWnd::OnBtnM60Clicked()
{
	OnBtnPeriedChange(Period_60Min);
}

void CWorkWnd::OnBtnDayClicked()
{
	OnBtnPeriedChange(Period_1Day);

}

void CWorkWnd::OnBtnListConnect1Clicked()
{
	m_bListConn1 = !m_bListConn1;
	if (!m_bListConn1)
		m_ListShowInd = "";
	SetBtnState(m_pBtnConn1, m_bListConn1);
	if (m_bUseStockFilter)
	{
		m_bUseStockFilter = FALSE;
		m_pDlgCmbStockFilter->StopFilter();
		SetBtnState(m_pBtnStockFilter, m_bUseStockFilter);
	}

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::OnBtnListConnect2Clicked()
{
	m_bListConn2 = !m_bListConn2;
	if (!m_bListConn2)
		m_ListShowInd = "";
	SetBtnState(m_pBtnConn2, m_bListConn2);
	if (m_bUseStockFilter)
	{
		m_bUseStockFilter = FALSE;
		m_pDlgCmbStockFilter->StopFilter();
		SetBtnState(m_pBtnStockFilter, m_bUseStockFilter);
	}

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::OnBtnStockFilterClicked()
{
	//2023.5.4修改 将m_pDlgStockFilter替换为m_pDlgCmbStockFilter
	if (m_pDlgCmbStockFilter->IsWindowVisible())
		m_pDlgCmbStockFilter->ShowWindow(SW_HIDE);
	else
	{
		m_pDlgCmbStockFilter->CenterWindow(m_hParWnd);
		//m_pDlgCmbStockFilter->InitFrmlCombox();
		m_pDlgCmbStockFilter->ShowWindow(SW_SHOW);
	}
}

void CWorkWnd::OnBtnTitleSelectClicked()
{
	CDlgHeaderSelect *pDlg = new CDlgHeaderSelect(m_hWnd, m_TitleShowMap);
	pDlg->Create(NULL);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
	//::EnableWindow(m_hWnd, FALSE);
}

void CWorkWnd::OnCheckST()
{
	m_bListShowST = m_pCheckST->IsChecked();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_ChangeShowList, 0);
}

void CWorkWnd::OnCheckSBM()
{
	m_bListShowSBM = m_pCheckSBM->IsChecked();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_ChangeShowList, 0);
}

void CWorkWnd::OnCheckSTARM()
{
	m_bListShowSTARM = m_pCheckSTARM->IsChecked();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_ChangeShowList, 0);

}

void CWorkWnd::OnCheckNewStock()
{
	m_bListShowNewStock = m_pCheckNewStock->IsChecked();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_ChangeShowList, 0);
}

void CWorkWnd::OnBtnRehab()
{
	SMenu menu;
	menu.LoadMenuW(L"smenu:menu_rehab");
	eRehabType rehabType = m_pKlinePic->GetRehabType();
	for (int i = RM_NoRehab; i < RM_End; ++i)
		menu.CheckMenuItem(i, i - RM_NoRehab == rehabType ? MF_CHECKED : MF_UNCHECKED);
	CRect rc = m_pBtnRehab->GetWindowRect();
	ClientToScreen(&rc);
	menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);

}

void CWorkWnd::SetBtnState(SImageButton * nowBtn, SImageButton ** preBtn)
{
	if (*preBtn)
		(*preBtn)->SetAttribute(L"colorText", L"#c0c0c0ff");
	nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	*preBtn = nowBtn;
}

void CWorkWnd::SetBtnState(SImageButton * nowBtn, bool bSelected)
{
	if (bSelected)
		nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	else
		nowBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void CWorkWnd::SetBtnState(int nPeriod, bool bSelected)
{
	SImageButton * pBtn = m_pPeriodBtnMap[nPeriod];
	if (bSelected)
	{
		pBtn->SetAttribute(L"colorText", L"#00ffffff");
		m_pPreSelBtn = pBtn;
	}
	else
		pBtn->SetAttribute(L"colorText", L"#c0c0c0ff");
}

void CWorkWnd::OnBtnShowTypeChange(bool bFroceList)
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

				::PostMessage(m_hParWnd, WM_WINDOW_MSG,
					WDMsg_SaveConfig, NULL);
			}
			return;
		}
	}
	if (m_bShowList)
	{
		SetBtnState(m_ListPeriod, false);
		SetBtnState(m_PicPeriod, true);
		SetSelectedPeriod(m_ListPeriod);
	}
	else
	{
		SetBtnState(m_PicPeriod, false);
		SetBtnState(m_ListPeriod, true);
		SwitchPic2List();
	}
}

void CWorkWnd::OnBtnPeriedChange(int nPeriod)
{
	SImageButton * pBtn = m_pPeriodBtnMap[nPeriod];
	if (pBtn == m_pPreSelBtn)
		return;
	SetBtnState(pBtn, &m_pPreSelBtn);
	if (m_bShowList)
	{
		m_ListPeriod = nPeriod;
		UpdateList();
	}
	else
		SetSelectedPeriod(nPeriod);

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::SetListShowIndyStr(SStatic * pText)
{
	if (m_bUseStockFilter)
		pText->SetWindowTextW(L"当前分类:选股器");
	else
	{
		if (m_ListShowInd == "")
			pText->SetWindowTextW(L"当前分类:全市场");
		else
		{
			SStringW strIndy = StrA2StrW(m_StockName.hash[m_ListShowInd]);
			SStringW strText;
			pText->SetWindowTextW(strText.Format(L"当前分类:%s", strIndy));
		}
	}
}

void CWorkWnd::SetFenShiShowData(vector<ShowPointInfo>&infoVec, int nStartWnd)
{
	SStringA stockID = m_strSubStock;
	//int nShowNum = m_pFenShiPic->GetShowSubPicNum();
	vector<SStringA> nameVec;
	GetBelongingIndyName(nameVec);
	m_pFenShiPic->SetBelongingIndy(nameVec, nStartWnd);
	int nShowNum = infoVec.size();
	if (nShowNum > 0)
	{
		vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
		vector<vector<BOOL>> rightVec(nShowNum);
		vector<vector<SStringA>> dataNameVec(nShowNum);
		int *dataCount = new int[nShowNum];
		for (int i = 0; i < nShowNum; ++i)
		{
			auto &info = infoVec[i];
			auto pPointData = &m_PointData;
			if (info.overallType > eIndyMarketPointEnd)
			{
				if (info.overallType >= eL1IndyPointStart &&  info.overallType < eL1IndyPointEnd)
					pPointData = &m_L1IndyPointData;
				else if (info.overallType >= eL2IndyPointStart &&  info.overallType < eL2IndyPointEnd)
					pPointData = &m_L2IndyPointData;
			}

			dataCount[i] = m_PointDataCount[info.type];
			tmpDataArr[i].resize(dataCount[i]);
			rightVec[i].resize(dataCount[i]);
			for (int j = 0; j < dataCount[i]; ++j)
			{
				SStringA dataName = info.srcDataName +
					m_SubPicShowNameVec[info.type][info.dataInRange][j];
				tmpDataArr[i][j] = &(*pPointData)[Period_FenShi][dataName];
				dataNameVec[i].emplace_back(m_SubPicShowNameVec[info.type][info.dataInRange][j]);
				rightVec[i][j] = TRUE;
			}
		}

		m_pFenShiPic->SetSubPicShowData(dataCount,
			tmpDataArr, rightVec, dataNameVec,
			stockID, m_StockName.hash[stockID], nStartWnd);

		delete[]dataCount;
		dataCount = nullptr;
	}


}


void CWorkWnd::SetKlineShowData(vector<ShowPointInfo>&infoVec, int nPeriod, BOOL bNeedReCalc, int nStartWnd)
{
	SStringA stockID = m_strSubStock;
	vector<SStringA> nameVec;
	GetBelongingIndyName(nameVec);
	m_pKlinePic->SetBelongingIndy(nameVec, nStartWnd);
	int nShowNum = infoVec.size();
	if (nShowNum > 0)
	{
		vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
		vector<vector<BOOL>> rightVec(nShowNum);
		vector<vector<SStringA>> dataNameVec(nShowNum);
		int *dataCount = new int[nShowNum];
		for (int i = 0; i < nShowNum; ++i)
		{
			auto &info = infoVec[i];
			auto pPointData = &m_PointData;
			if (info.overallType > eIndyMarketPointEnd)
			{
				if (info.overallType >= eL1IndyPointStart &&  info.overallType < eL1IndyPointEnd)
					pPointData = &m_L1IndyPointData;
				else if (info.overallType >= eL2IndyPointStart &&  info.overallType < eL2IndyPointEnd)
					pPointData = &m_L2IndyPointData;
			}


			dataCount[i] = m_PointDataCount[info.type];
			tmpDataArr[i].resize(dataCount[i]);
			rightVec[i].resize(dataCount[i]);
			for (int j = 0; j < dataCount[i]; ++j)
			{
				SStringA dataName = info.srcDataName +
					m_SubPicShowNameVec[info.type][info.dataInRange][j];
				tmpDataArr[i][j] = &(*pPointData)[nPeriod][dataName];
				dataNameVec[i].emplace_back(m_SubPicShowNameVec[info.type][info.dataInRange][j]);
				rightVec[i][j] = TRUE;
			}
		}

		m_pKlinePic->SetSubPicShowData(dataCount,
			tmpDataArr, rightVec, dataNameVec,
			stockID, m_StockName.hash[stockID], nStartWnd);

		delete[]dataCount;
		dataCount = nullptr;


	}

	m_pKlinePic->ChangePeriod(nPeriod, bNeedReCalc);


}

void CWorkWnd::GetBelongingIndyName(vector<SStringA>& nameVec)
{
	nameVec.resize(2);
	SStringA stockID = m_strSubStock;
	//stockID = stockID.Left(6);
	const auto &info = m_infoMap.hash[m_strSubStock];
	nameVec[0] = m_StockName.hash[info.SWL1ID];
	nameVec[1] = m_StockName.hash[info.SWL2ID];
}

void CWorkWnd::SetSelectedPeriod(int nPeriod)
{
	if (m_PicPeriod == nPeriod
		&&m_strSubStock != "")
	{
		SwitchList2Pic(nPeriod);
		return;
	}
	m_PicPeriod = nPeriod;
	SStringA& StockID = m_strSubStock;
	if (StockID == "")
	{
		for (auto &it : m_ListPosMap.hash)
		{
			if (it.second == 0)
			{
				StockID = it.first;
				ShowPicWithNewID(StockID);
				return;
			}
		}
	}

	if (Period_FenShi != nPeriod)
	{

		if (m_KlineGetMap.count(nPeriod) == 0)
		{
			m_pKlinePic->SetHisKlineState(false);
			DataGetInfo GetInfo;
			GetInfo.hWnd = m_hWnd;
			strcpy_s(GetInfo.StockID, StockID);
			GetInfo.Group = m_Group;
			GetInfo.Period = nPeriod;
			if (nPeriod == Period_1Day)
				SendMsg(m_uParWndThreadID, WW_GetCallAction,
				(char*)&GetInfo, sizeof(GetInfo));

			SendMsg(m_uParWndThreadID, WW_GetKline,
				(char*)&GetInfo, sizeof(GetInfo));
		}
		else
			m_pKlinePic->SetHisKlineState(true);
		vector<ShowPointInfo> infoVec;
		m_pKlinePic->GetShowPointInfo(infoVec);
		for (auto &info : infoVec)
			GetPointData(info, StockID, nPeriod);

		SetKlineShowData(infoVec, nPeriod, TRUE);
	}
	SwitchList2Pic(nPeriod);
}

void CWorkWnd::ShowPicWithNewID(SStringA StockID, bool bForce)
{
	if (!bForce && m_strSubStock == StockID)
		return;
	SetDataFlagFalse();
	m_PicPeriod = m_bShowList ?
		m_ListPeriod : m_PicPeriod;
	m_strSubStock = StockID;

	SStringA StockName = m_StockName.hash[StockID];
	if (m_Group != Group_Stock)
	{
		m_pFenShiPic->SetShowData(StockID, StockName, &m_IndexMarketVec);
		m_pKlinePic->SetShowData(StockID, StockName,
			&m_IndexMarketVec, &m_KlineMap);
	}
	else
	{
		m_pFenShiPic->SetShowData(StockID, StockName, &m_StockMarketVec);
		m_pKlinePic->SetShowData(StockID, StockName,
			&m_StockMarketVec, &m_KlineMap);
	}
	//获取分时数据
	DataGetInfo GetInfo;
	GetInfo.hWnd = m_hWnd;
	strcpy_s(GetInfo.StockID, StockID);
	GetInfo.Group = m_Group;
	GetInfo.Period = Period_FenShi;
	SendMsg(m_uParWndThreadID, WW_GetMarket,
		(char*)&GetInfo, sizeof(GetInfo));
	vector<ShowPointInfo>infoVec;
	m_pFenShiPic->GetShowPointInfo(infoVec);
	for (auto &info : infoVec)
		GetPointData(info, StockID, Period_FenShi);
	SetFenShiShowData(infoVec);

	//获取当前订阅数据
	if (Period_FenShi != m_PicPeriod)
	{
		GetInfo.Period = m_PicPeriod;
		if (Period_1Day == m_PicPeriod)
			SendMsg(m_uParWndThreadID, WW_GetCallAction,
			(char*)&GetInfo, sizeof(GetInfo));

		SendMsg(m_uParWndThreadID, WW_GetKline,
			(char*)&GetInfo, sizeof(GetInfo));
		vector<ShowPointInfo>infoVec;
		m_pKlinePic->GetShowPointInfo(infoVec);
		for (auto &info : infoVec)
			GetPointData(info, StockID, m_PicPeriod);

		SetKlineShowData(infoVec, m_PicPeriod, FALSE);
	}
	SwitchList2Pic(m_PicPeriod);
}

void CWorkWnd::SetDataFlagFalse()
{
	m_bMarketGet = false;
	m_bCAInfoGet = false;
	m_PointGetMap.clear();
	m_L1IndyPointGetMap.clear();
	m_L2IndyPointGetMap.clear();
	m_CallAction.clear();
	//m_PointGetMap.clear();
	m_KlineGetMap.clear();
	m_pKlinePic->SetTodayMarketState(false);
	m_pKlinePic->SetHisKlineState(false);
	m_pKlinePic->SetHisPointState(false);
	m_pKlinePic->SetHisCAInfoState(false);

}

void CWorkWnd::GetPointData(ShowPointInfo & info, SStringA StockID, int nPeriod)
{
	if (CheckDataIsGot(info, nPeriod))
		return;
	ExDataGetInfo GetInfo;
	GetInfo.nAskGroup = m_Group;

	//GetInfo.nAskGroup = m_Group;
	GetInfo.Type = info.type;
	GetInfo.hWnd = m_hWnd;
	GetInfo.Period = nPeriod;

	if (info.overallType < eCAPointEnd&&
		info.overallType >= eCAPointStart)
	{
		return;
		//if (m_PicPeriod != Period_1Day)
		//	return;
		//GetInfo.Group = m_Group;
		//strcpy_s(GetInfo.StockID, StockID);
		//SendMsg(m_uParWndThreadID, WW_GetCallAction,
		//	(char*)&GetInfo, sizeof(GetInfo));
	}
	else
	{
		if (info.overallType < eIndyMarketPointEnd)
		{
			GetInfo.Group = m_Group;
			strcpy_s(GetInfo.StockID, StockID);
		}
		else if (info.overallType < eL1IndyPointEnd
			&& info.overallType >= eL1IndyPointStart)
		{
			GetInfo.Group = Group_SWL1;
			strcpy_s(GetInfo.StockID, m_infoMap.hash[StockID].SWL1ID);
		}
		else if (info.overallType < eL2IndyPointEnd
			&& info.overallType >= eL2IndyPointStart)
		{
			GetInfo.Group = Group_SWL2;
			strcpy_s(GetInfo.StockID, m_infoMap.hash[StockID].SWL2ID);
		}

		SStringA strExMsg;
		strExMsg.Format("dataName:%s,dataRange:%s", info.srcDataName, info.dataInRange);

		GetInfo.exMsg = new char[strExMsg.GetLength() + 1];
		strcpy_s(GetInfo.exMsg, strExMsg.GetLength() + 1, strExMsg);
		SendMsg(m_uParWndThreadID, WW_GetPoint,
			(char*)&GetInfo, sizeof(GetInfo));

	}


}

void CWorkWnd::UpdateTmData(vector<CoreData>& comData, CoreData & data)
{
	if (comData.empty())
		comData.emplace_back(data);
	else if (comData.back().date < data.date)
		comData.emplace_back(data);
	else if (comData.back().date == data.date)
	{
		if (comData.back().time < data.time)
			comData.emplace_back(data);
		else if (comData.back().time == data.time)
			comData.back() = data;
	}
}

bool CWorkWnd::CheckDataIsGot(ShowPointInfo & info, int nPeriod)
{
	if (info.overallType < eCAPointEnd&&
		info.overallType >= eCAPointStart)
	{
		if (m_bCAInfoGet)
			return true;
		else
			return false;
	}
	auto pPointGetMap = &m_PointGetMap;
	if (info.overallType < eL1IndyPointEnd
		&& info.overallType >= eL1IndyPointStart)
		pPointGetMap = &m_L1IndyPointGetMap;
	else if (info.overallType < eL2IndyPointEnd
		&& info.overallType >= eL2IndyPointStart)
		pPointGetMap = &m_L2IndyPointGetMap;
	auto &dataGetMap = (*pPointGetMap)[nPeriod];

	if (eRpsPoint == info.type)
	{
		SStringA dataName520 = info.srcDataName + "Point520" + info.dataInRange;
		SStringA dataName2060 = info.srcDataName + "Point2060" + info.dataInRange;
		if (dataGetMap[dataName520] == 0 ||
			dataGetMap[dataName2060] == 0)
			return false;
	}
	else if (eSecPoint == info.type)
	{
		SStringA dataName = info.srcDataName + "Point" + info.dataInRange;
		if (dataGetMap[dataName] == 0)
			return false;
	}
	return true;
}

void CWorkWnd::OnUpdateListData(int nMsgLength, const char * info)
{
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdateList, NULL);
}

void CWorkWnd::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nDataCount = nMsgLength / sizeof(RtPointData);
	RtPointData *dataArr = (RtPointData*)info;
	for (int i = 0; i < nDataCount; ++i)
	{
		auto pPointData = &m_PointData;
		auto pPointGetMap = &m_PointGetMap;
		if (dataArr[i].stockID != m_strSubStock)
		{
			if (strcmp(dataArr[i].stockID, m_infoMap.hash[m_strSubStock].SWL1ID) == 0)
			{
				pPointData = &m_L1IndyPointData;
				pPointGetMap = &m_L1IndyPointGetMap;
			}
			else if (strcmp(dataArr[i].stockID, m_infoMap.hash[m_strSubStock].SWL2ID) == 0)
			{
				pPointData = &m_L2IndyPointData;
				pPointGetMap = &m_L2IndyPointGetMap;
			}
		}

		if ((*pPointGetMap)[dataArr[i].period][dataArr[i].dataName])
		{
			UpdateTmData((*pPointData)[dataArr[i].period]\
				[dataArr[i].dataName],
				dataArr[i].data);
		}
	}
	m_pFenShiPic->UpdateData();
	m_pKlinePic->UpdateData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

//void CWorkWnd::OnUpdateTodayPoint(int nMsgLength, const char * info)
//{
//	int nPeriod = *(int*)info;
//	map<SStringA, vector<CoreData>> *pDataMap =
//		*(map<SStringA, vector<CoreData>>**)(info + 4);
//	auto & pointMap = m_PointData[nPeriod];
//	for (auto &it : *pDataMap)
//	{
//		if (it.first.Find("Point") != -1)
//			pointMap[it.first] = it.second;
//	}
//	delete pDataMap;
//	pDataMap = nullptr;
//	m_PointReadyMap[nPeriod][] = true;
//}

void CWorkWnd::OnUpdateHisKline(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nSize = pRecvInfo->SrcDataSize / sizeof(KlineType);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	m_KlineGetMap[nPeriod] = TRUE;
	auto &KlineVec = m_KlineMap[nPeriod];
	KlineVec.resize(nSize);
	memcpy_s(&KlineVec[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	m_pKlinePic->SetHisKlineState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisRpsPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;

	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char *msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	pRecvInfo->TotalDataSize -= (nAttMsgSize + sizeof(nAttMsgSize));
	//ReceivePointInfo *pRecvInfo1 =
	//	(ReceivePointInfo *)(info + nOffset);
	//nOffset += sizeof(*pRecvInfo1);
	ProcHisRpsPointFromMsg(pRecvInfo, info + nOffset,
		"Point520", "Point2060", msg, nAttMsgSize);
	//nOffset += pRecvInfo1->TotalDataSize;
	//if (pRecvInfo->Group == Group_Stock)
	//{
	//	ReceivePointInfo *pRecvInfo2 =
	//		(ReceivePointInfo *)(info + nOffset);
	//	nOffset += sizeof(*pRecvInfo2);
	//	ProcHisRpsPointFromMsg(pRecvInfo2, info + nOffset,
	//		"L1Point520", "L1Point2060");
	//	nOffset += pRecvInfo2->TotalDataSize;
	//	ReceivePointInfo *pRecvInfo3 =
	//		(ReceivePointInfo *)(info + nOffset);
	//	nOffset += sizeof(*pRecvInfo3);
	//	ProcHisRpsPointFromMsg(pRecvInfo3, info + nOffset,
	//		"L2Point520", "L2Point2060");
	//}
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);

}


void CWorkWnd::OnUpdateIndexMarket(int nMsgLength, const char * info)
{
	CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
	SStringA SecurityID = pIndexData->SecurityID;
	m_IndexMarketVec.emplace_back(*pIndexData);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateStockMarket(int nMsgLength, const char * info)
{
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	m_StockMarketVec.emplace_back(*pStockData);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisIndexMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(CommonIndexMarket);
	CommonIndexMarket * dataArr = (CommonIndexMarket *)(info + nOffset);
	m_IndexMarketVec.reserve(MAX_TICK);
	m_IndexMarketVec.resize(dataCount);
	memcpy_s(&m_IndexMarketVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	m_bMarketGet = TRUE;
	m_pFenShiPic->DataProc();
	m_pKlinePic->SetTodayMarketState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateHisStockMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(CommonStockMarket);
	CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
	m_StockMarketVec.reserve(MAX_TICK);
	m_StockMarketVec.resize(dataCount);
	memcpy_s(&m_StockMarketVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	m_bMarketGet = TRUE;
	m_pFenShiPic->DataProc();
	m_pKlinePic->SetTodayMarketState(true);
	if (m_pKlinePic->GetDataReadyState())
		m_pKlinePic->DataProc();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateCloseInfo(int nMsgLength, const char * info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	strHash<double> preCloseMap;
	for (int i = 0; i < dataCount; ++i)
		preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
	::EnterCriticalSection(&m_csClose);
	m_preCloseMap.hash = preCloseMap.hash;
	//for (int i = 0; i < dataCount; ++i)
	//	m_preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
	::LeaveCriticalSection(&m_csClose);

}

void CWorkWnd::OnChangeShowIndy(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SStringA IndexID = info + 4;
	if (Group_SWL1 == nGroup
		&& m_bListConn1)
		m_ListShowInd = IndexID;
	if (Group_SWL2 == nGroup
		&& m_bListConn2)
		m_ListShowInd = IndexID;
	::PostMessage(m_hWnd, WM_WINDOW_MSG,
		WDMsg_ChangeShowList, NULL);
}

void CWorkWnd::OnUpdateHisSecPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;

	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char *msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	pRecvInfo->TotalDataSize -= (nAttMsgSize + sizeof(nAttMsgSize));
	//ReceivePointInfo *pRecvInfo1 =
	//	(ReceivePointInfo *)(info + nOffset);
	//nOffset += sizeof(*pRecvInfo1);
	ProcHisSecPointFromMsg(pRecvInfo, info + nOffset,
		"Point", msg, nAttMsgSize);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnUpdateRehabInfo(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(RehabInfo);
	RehabInfo * dataArr = (RehabInfo *)(info + nOffset);
	vector<RehabInfo> rehabInfoVec;
	rehabInfoVec.resize(dataCount);
	memcpy_s(&rehabInfoVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	m_pKlinePic->SetRehabInfo(rehabInfoVec);

}

void SOUI::CWorkWnd::OnUpdateHisCallAction(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	int nOffset = sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nSize = pRecvInfo->SrcDataSize / sizeof(CAInfo);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_CallAction.resize(nSize);
	memcpy_s(&m_CallAction[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	ProcHisCAPointFromCAInfo();
	m_bCAInfoGet = TRUE;
	m_pKlinePic->SetHisCAInfoState(true);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnFenShiEma(int nMsgLength, const char * info)
{
	m_pFenShiPic->ReProcEMA();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnFenShiMacd(int nMsgLength, const char * info)
{
	m_pFenShiPic->ReProcMacd();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineMa(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcMAData(m_MaParaSet);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineMacd(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcMacdData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnKlineBand(int nMsgLength, const char * info)
{
	m_pKlinePic->ReProcBandData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnChangeStockFilter(int nMsgLength, const char * info)
{
	BOOL bUse = *(BOOL*)info;
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_ChangeStockFilter, bUse);

	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		WDMsg_SaveConfig, NULL);
}

void CWorkWnd::OnSaveStockFilter(int nMsgLength, const char * info)
{
	if(m_bFilterWnd)
		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SaveStockFilter, m_nWndNum);
	else
		::PostMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_SaveStockFilter, m_Group);
}

void CWorkWnd::OnChangeKlineRehab(int nMsgLength, const char * info)
{
	int rehabType = *(int*)info;
	m_pKlinePic->ReProcKlineRehabData((eRehabType)rehabType);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
}

void CWorkWnd::OnFixedTimeRehab(int nMsgLength, const char * info)
{
	FixedTimeRehab &frt = *(FixedTimeRehab*)info;
	m_pKlinePic->ReProcKlineRehabData(frt);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);

}



BOOL CWorkWnd::GetAttPara(char * msg, map<SStringA, SStringA>& paraMap)
{
	stringstream ss(msg);
	string buffer = "";
	while (getline(ss, buffer, ','))
	{
		size_t nPos = buffer.find(":");
		if (nPos != string::npos)
		{
			SStringA strName = buffer.substr(0, nPos).c_str();
			SStringA strData = buffer.substr(nPos + 1).c_str();
			paraMap[strName] = strData;
		}
	}

	return TRUE;
}

void CWorkWnd::ProcHisRpsPointFromMsg(ReceivePointInfo * pRecvInfo,
	const char * info, SStringA dataName1, SStringA dataName2,
	char* attchMsg, int attMsgSize)
{

	int nOffset = 0;
	int nSize520 = pRecvInfo->FirstDataSize / sizeof(CoreData);
	int nSize2060 = (pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize) /
		sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	//vector<CoreData> PointVec(nSize520);
	//memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
	//	info + nOffset, pRecvInfo->FirstDataSize);
	map<SStringA, SStringA>attPara;
	GetAttPara(attchMsg, attPara);
	SStringA strDataName = attPara["dataName"];
	SStringA strRange = attPara["dataRange"];

	auto pPointData = &m_PointData;
	auto pPointGetMap = &m_PointGetMap;
	if (pRecvInfo->Message != m_strSubStock)
	{
		if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStock].SWL1ID) == 0)
		{
			pPointData = &m_L1IndyPointData;
			pPointGetMap = &m_L1IndyPointGetMap;
		}
		else if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStock].SWL2ID) == 0)
		{
			pPointData = &m_L2IndyPointData;
			pPointGetMap = &m_L2IndyPointGetMap;
		}
	}


	nOffset += attMsgSize;
	SStringA point520Name = strDataName + dataName1 + strRange;
	auto &Point520Vec = (*pPointData)[nPeriod][point520Name];
	Point520Vec.resize(nSize520);
	memcpy_s(&Point520Vec[0], pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->FirstDataSize);

	//Point520Vec.insert(Point520Vec.begin(),
	//	PointVec.begin(), PointVec.end());
	//PointVec.resize(nSize2060);
	//memcpy_s(&PointVec[0], pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize,
	//	info + nOffset, pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize);
	SStringA point2060Name = strDataName + dataName2 + strRange;
	nOffset += pRecvInfo->FirstDataSize;
	auto &Point2060Vec = (*pPointData)[nPeriod][strDataName + dataName2 + strRange];
	Point2060Vec.resize(nSize2060);
	memcpy_s(&Point2060Vec[0], pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize);
	(*pPointGetMap)[nPeriod][point520Name] = TRUE;
	(*pPointGetMap)[nPeriod][point2060Name] = TRUE;

	//Point2060Vec.insert(Point2060Vec.begin(),
	//	PointVec.begin(), PointVec.end());
}

void CWorkWnd::ProcHisSecPointFromMsg(ReceivePointInfo * pRecvInfo,
	const char * info, SStringA dataName, char * attchMsg, int attMsgSize)
{
	int nOffset = 0;
	int nSize = pRecvInfo->FirstDataSize / sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	map<SStringA, SStringA>attPara;
	GetAttPara(attchMsg, attPara);
	SStringA strDataName = attPara["dataName"];
	SStringA strRange = attPara["dataRange"];
	SStringA pointName = strDataName + dataName + strRange;

	auto pPointData = &m_PointData;
	auto pPointGetMap = &m_PointGetMap;
	if (pRecvInfo->Message != m_strSubStock)
	{
		if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStock].SWL1ID) == 0)
		{
			pPointData = &m_L1IndyPointData;
			pPointGetMap = &m_L1IndyPointGetMap;
		}
		else if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStock].SWL2ID) == 0)
		{
			pPointData = &m_L2IndyPointData;
			pPointGetMap = &m_L2IndyPointGetMap;
		}
	}

	nOffset += attMsgSize;
	auto &PointVec = (*pPointData)[nPeriod][pointName];
	PointVec.resize(nSize);
	memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->FirstDataSize);

	(*pPointGetMap)[nPeriod][pointName] = TRUE;
}

void CWorkWnd::ProcHisCAPointFromCAInfo()
{
	auto &CAVolPointVec = m_PointData[Period_1Day]["CaVolPoint"];
	auto &CAVolPointL1Vec = m_PointData[Period_1Day]["CaVolPointL1"];
	auto &CAVolPointL2Vec = m_PointData[Period_1Day]["CaVolPointL2"];
	auto &CAAmoPointVec = m_PointData[Period_1Day]["CaAmoPoint"];
	auto &CAAmoPointL1Vec = m_PointData[Period_1Day]["CaAmoPointL1"];
	auto &CAAmoPointL2Vec = m_PointData[Period_1Day]["CaAmoPointL2"];
	CAVolPointVec.reserve(m_CallAction.size());
	CAVolPointL1Vec.reserve(m_CallAction.size());
	CAVolPointL2Vec.reserve(m_CallAction.size());
	CAAmoPointVec.reserve(m_CallAction.size());
	CAAmoPointL1Vec.reserve(m_CallAction.size());
	CAAmoPointL2Vec.reserve(m_CallAction.size());

	for (auto &it : m_CallAction)
	{
		CoreData data = { 0 };
		data.date = it.date;
		data.value = it.VolPoint;
		CAVolPointVec.emplace_back(data);
		data.value = it.VolPointL1;
		CAVolPointL1Vec.emplace_back(data);
		data.value = it.VolPointL2;
		CAVolPointL2Vec.emplace_back(data);
		data.value = it.AmoPoint;
		CAAmoPointVec.emplace_back(data);
		data.value = it.AmoPointL1;
		CAAmoPointL1Vec.emplace_back(data);
		data.value = it.AmoPointL2;
		CAAmoPointL2Vec.emplace_back(data);

	}
	m_PointGetMap[Period_1Day]["CaVolPoint"] = TRUE;
	m_PointGetMap[Period_1Day]["CaVolPointL1"] = TRUE;
	m_PointGetMap[Period_1Day]["CaVolPointL2"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPoint"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPointL1"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPointL2"] = TRUE;

}
