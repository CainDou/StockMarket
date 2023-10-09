#include "stdafx.h"
#include "TradeSimulator.h"
#include "SColorListCtrlEx.h"
#include "STradeInfoPic.h"
#include "WndSynHandler.h"
#include <algorithm>
#include "SSpinButtonCtrlEx.h"
#include <locale>
#include "DlgTradeSetting.h"
#include "SMessageBoxTimer.h"
#include "DlgTrustConfirm.h"

#define MAX_NUM_LENGTH 9
#define MAX_TICK 6000
#define PRICELIMIT 0.02

extern CWndSynHandler g_WndSyn;

const wchar_t* strNumOfDigital[] = { L"亿",L"仟",L"佰",L"拾",L"万",L"仟",L"佰",L"拾",L"" };
const wchar_t* strBigNum[] = { L"零",L"壹",L"贰",L"叁",L"肆",L"伍",L"陆",L"柒",L"捌",L"玖" };

const long long llShMainMaxTrade = 1'000'000;
const long long llSzMainMaxTrade = 1'000'000;
const long long llSbmMaxTrade = 300'000;
const long long llStramMaxTrade = 100'000;
CTradeSimulator::CTradeSimulator() :SHostWnd(_T("LAYOUT:dlg_TradeSimulate"))
{
	m_bLogin = FALSE;
	m_nHisTrustStartDate = 0;
	m_nHisTrustEndDate = 0;
	m_nHisDealStartDate = 0;
	m_nHisDealEndDate = 0;
	m_bLayoutInited = FALSE;
	m_bDataInited = FALSE;
}


CTradeSimulator::~CTradeSimulator()
{
	SendMsg(m_DataThreadID, Msg_Exit, NULL, 0);
	if (tDataProc.joinable())
		tDataProc.join();
}

BOOL CTradeSimulator::OnInitDialog(EventArgs * e)
{
	m_bLayoutInited = TRUE;
	::InitializeCriticalSection(&m_csClose);
	::InitializeCriticalSection(&m_csDeal);
	::InitializeCriticalSection(&m_csAccount);
	::InitializeCriticalSection(&m_csPosition);
	::InitializeCriticalSection(&m_csTrust);
	::InitializeCriticalSection(&m_csMarket);

	g_WndSyn.SetTradeWnd(m_hWnd);
	tDataProc = thread(&CTradeSimulator::DataMsgProc, this);
	m_DataThreadID = *(unsigned*)&tDataProc.get_id();
	g_WndSyn.SetTradeDlgThreadID(m_DataThreadID);

	GetTradeSetting();
	InitControls();
	InitStrings();
	InitColors();
	InitDataHandleMap();

	return 0;
}

void CTradeSimulator::InitControls()
{
	m_pTabTrade = FindChildByName2<STabCtrl>(L"tab_ctrl");

	m_pEditBuyID = FindChildByName2<SEdit>(L"edit_buyId");
	m_pEditBuyID->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditTradeIDChange, this));
	m_pEditBuyPrice = FindChildByName2<SEdit>(L"edit_buyPrice");
	m_pEditBuyPrice->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditPriceChange, this));

	m_pEditBuyVol = FindChildByName2<SEdit>(L"edit_buyVol");
	m_pEditBuyVol->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditVolChange, this));
	m_pLbBuyID = FindChildByName2<SListBox>(L"lb_buyID");
	m_pLbBuyID->GetEventSet()->subscribeEvent(
		EventLButtonDown::EventID,
		Subscriber(&CTradeSimulator::OnLbIDLButtonDown, this));


	m_pEditSellID = FindChildByName2<SEdit>(L"edit_sellId");
	m_pEditSellID->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditTradeIDChange, this));
	m_pEditSellPrice = FindChildByName2<SEdit>(L"edit_sellPrice");
	m_pEditSellPrice->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditPriceChange, this));

	m_pEditSellVol = FindChildByName2<SEdit>(L"edit_sellVol");
	m_pEditSellVol->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditVolChange, this));
	m_pLbSellID = FindChildByName2<SListBox>(L"lb_sellID");
	m_pLbSellID->GetEventSet()->subscribeEvent(
		EventLButtonDown::EventID,
		Subscriber(&CTradeSimulator::OnLbIDLButtonDown, this));

	m_pTxtMaxBuy = FindChildByName2<SStatic>(L"text_maxBuy");
	m_pTxtMaxSell = FindChildByName2<SStatic>(L"text_maxSell");
	m_pTxtBuyVolInBig = FindChildByName2<SStatic>(L"txt_buyVolInBig");
	m_pTxtSellVolInBig = FindChildByName2<SStatic>(L"txt_sellVolInBig");

	m_IDListMap[m_pEditBuyID] = m_pLbBuyID;
	m_IDListMap[m_pEditSellID] = m_pLbSellID;

	m_volTextMap[m_pEditBuyVol] = m_pTxtBuyVolInBig;
	m_volTextMap[m_pEditSellVol] = m_pTxtSellVolInBig;

	m_pTxtTotalCapital = FindChildByName2<SStatic>(L"text_totalCapital");
	m_pTxtMarketValue = FindChildByName2<SStatic>(L"text_marketValue");
	m_pTxtProfit = FindChildByName2<SStatic>(L"text_profit");
	m_pTxtProfitChg = FindChildByName2<SStatic>(L"text_profitChg");
	m_pTxtAvaliableFunds = FindChildByName2<SStatic>(L"text_availaleFunds");

	m_pTradeInfo = FindChildByName2<STradeInfoPic>(L"tradeInfo");
	m_pTradeInfo->SetDataPoint(&m_marketVec);

	m_pLsPosition = FindChildByName2<SColorListCtrlEx>(L"ls_position");
	SHeaderCtrlEx *pHeader = (SHeaderCtrlEx *)m_pLsPosition->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(2);
	m_pEditPosIDSearch = FindChildByName2<SEdit>(L"edit_posIDSearch");
	m_pEditPosIDSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pTxtPosCount = FindChildByName2<SStatic>(L"text_posCount");
	m_editListMap[m_pEditPosIDSearch] = m_pLsPosition;
	m_StockIdInListMap[m_pLsPosition] = SPH_SecurityID;
	m_pLsPosition->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));

	m_pTxtTrustCount = FindChildByName2<SStatic>(L"text_trustCount");
	m_pEditTrustIDSearch = FindChildByName2<SEdit>(L"edit_trustIDSearch");
	m_pEditTrustIDSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pLsTrust = FindChildByName2<SColorListCtrlEx>(L"ls_trust");
	m_editListMap[m_pEditTrustIDSearch] = m_pLsTrust;
	m_StockIdInListMap[m_pLsTrust] = STH_SecurityID;
	m_pLsTrust->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));


	m_pTxtCancelCount = FindChildByName2<SStatic>(L"text_cancelCount");
	m_pLsCancel = FindChildByName2<SColorListCtrlEx>(L"ls_cancel");
	m_pChkSelectAll = FindChildByName2<SCheckBox>(L"chk_cancelAll");
	m_StockIdInListMap[m_pLsCancel] = SCH_SecurityID;
	m_pLsCancel->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));


	m_pTxtDealCount = FindChildByName2<SStatic>(L"text_dealCount");
	m_pEditDealIDSearch = FindChildByName2<SEdit>(L"edit_dealIDSearch");
	m_pEditDealIDSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pLsDeal = FindChildByName2<SColorListCtrlEx>(L"ls_deal");
	m_editListMap[m_pEditDealIDSearch] = m_pLsDeal;
	m_StockIdInListMap[m_pLsDeal] = SDH_SecurityID;
	m_pLsDeal->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));

	m_pTxtDealSumCount = FindChildByName2<SStatic>(L"text_dealSumCount");
	m_pEditDealSumSearch = FindChildByName2<SEdit>(L"edit_dealSumSearch");
	m_pEditDealSumSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pLsDealSum = FindChildByName2<SColorListCtrlEx>(L"ls_dealSummary");
	m_editListMap[m_pEditDealSumSearch] = m_pLsDealSum;
	m_StockIdInListMap[m_pLsDealSum] = SDSH_SecurityID;
	m_pLsDealSum->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));

	m_pDtpHisTrustStart = FindChildByName2<SDateTimePicker>(L"dtp_hisTrustStart");
	m_pDtpHisTrustEnd = FindChildByName2<SDateTimePicker>(L"dtp_hisTrustEnd");
	m_pEditHisTrustSearch = FindChildByName2<SEdit>(L"edit_hisTrustIDSearch");
	m_pEditHisTrustSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pTxtHisTrustCount = FindChildByName2<SStatic>(L"text_hisTrustCount");
	m_pLsHisTrust = FindChildByName2<SColorListCtrlEx>(L"ls_hisTrust");
	m_editListMap[m_pEditHisTrustSearch] = m_pLsHisTrust;
	m_StockIdInListMap[m_pLsHisTrust] = SHTH_SecurityID;
	m_pLsHisTrust->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));

	m_pDtpHisDealStart = FindChildByName2<SDateTimePicker>(L"dtp_hisDealStart");
	m_pDtpHisDealEnd = FindChildByName2<SDateTimePicker>(L"dtp_hisDealEnd");
	m_pEditHisDealSearch = FindChildByName2<SEdit>(L"edit_hisDealIDSearch");
	m_pEditHisDealSearch->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CTradeSimulator::OnEditSearchChange, this));
	m_pTxtHisDealCount = FindChildByName2<SStatic>(L"text_hisDealCount");
	m_pLsHisDeal = FindChildByName2<SColorListCtrlEx>(L"ls_hisDeal");
	m_editListMap[m_pEditHisDealSearch] = m_pLsHisDeal;
	m_StockIdInListMap[m_pLsHisDeal] = SHDH_SecurityID;
	m_pLsHisDeal->GetEventSet()->subscribeEvent(
		EventLCDbClick::EventID,
		Subscriber(&CTradeSimulator::OnListLDoubleClicked, this));

	m_pLsCancel->GetEventSet()->subscribeEvent(EventLCSelChanged::EventID,
		Subscriber(&CTradeSimulator::OnListCancelLBClicked, this));

	m_pEditBuyVol->SetWindowTextW(L"");
	m_pEditSellVol->SetWindowTextW(L"");
}

void CTradeSimulator::InitStrings()
{
	m_tradeDirectStrMap[eTD_Buy] = L"买入";
	m_tradeDirectStrMap[eTD_Sell] = L"卖出";
	m_tradeDirectStrMap[eTD_CancelBuy] = L"撤买";
	m_tradeDirectStrMap[eTD_CancelSell] = L"撤卖";

	m_trustStateStrMap[eTS_NotSubmit] = L"待报";
	m_trustStateStrMap[eTS_Submitted] = L"已报";
	m_trustStateStrMap[eTS_DealPart] = L"部成";
	m_trustStateStrMap[eTS_DealAll] = L"已成";
	m_trustStateStrMap[eTS_Canceled] = L"已撤";
	m_trustStateStrMap[eTS_Discarded] = L"废单";
	m_cancelSignStrMap[eCS_Normal] = L"正常";
	m_cancelSignStrMap[eCS_Canceled] = L"撤单";
}

void CTradeSimulator::InitColors()
{
	m_tradeDirectColorMap[eTD_Buy] = RGBA(255, 0, 0, 255);
	m_tradeDirectColorMap[eTD_Sell] = RGBA(0, 150, 255, 255);
	m_tradeDirectColorMap[eTD_CancelBuy] = RGBA(255, 0, 0, 255);
	m_tradeDirectColorMap[eTD_CancelSell] = RGBA(0, 150, 255, 255);

}

void CTradeSimulator::InitDatas()
{
	m_SynThreadID = g_WndSyn.GetThreadID();
	m_tradeSynThreadID = g_WndSyn.GetTradeThreadID();
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	m_stockVec = ListInsVec[Group_Stock];
	for (auto &it : m_stockVec)
		m_stockHash.hash[it.SecurityID] = it;
	vector<map<int, strHash<RtRps>>> *pListData = g_WndSyn.GetListData();
	m_pListDataMap = &pListData->at(Group_Stock);
	m_preCloseMap = g_WndSyn.GetCloseMap();
	g_WndSyn.AddWnd(m_hWnd, m_DataThreadID);
	g_WndSyn.SetSubWndInfo(m_hWnd, m_hWnd, Group_Stock);
	m_bDataInited = TRUE;
	UpdateAccountInfo();
	UpdatePositionList(L"");
	UpdateTrustList(L"");
	UpdateCancelList();
	UpdateDealList(L"");
	SetTimer(1, 1000);
}

void CTradeSimulator::InitDataHandleMap()
{
	m_DataHandleMap[Syn_HisStockMarket] =
		&CTradeSimulator::OnUpdateHisStockMarket;
	m_DataHandleMap[Syn_RTStockMarket] =
		&CTradeSimulator::OnUpdateStockMarket;
	m_DataHandleMap[Syn_CloseInfo] =
		&CTradeSimulator::OnUpdateCloseInfo;
	m_DataHandleMap[TradeSyn_OnLogin]
		= &CTradeSimulator::OnTradeLogin;
	m_DataHandleMap[TradeSyn_OnLogout]
		= &CTradeSimulator::OnTradeLogout;
	m_DataHandleMap[TradeSyn_OnAccountInfo]
		= &CTradeSimulator::OnAccountInfo;
	m_DataHandleMap[TradeSyn_OnPosition]
		= &CTradeSimulator::OnPosition;
	m_DataHandleMap[TradeSyn_OnTrust]
		= &CTradeSimulator::OnTrust;
	m_DataHandleMap[TradeSyn_OnDeal]
		= &CTradeSimulator::OnDeal;
	m_DataHandleMap[TradeSyn_OnHisTrust]
		= &CTradeSimulator::OnHisTrust;
	m_DataHandleMap[TradeSyn_OnHisDeal]
		= &CTradeSimulator::OnHisDeal;
	m_DataHandleMap[TradeSyn_OnSubmitFeedback]
		= &CTradeSimulator::OnSubmitFeedback;
}


void CTradeSimulator::OnBtnClose()
{
	if (SMessageBox(m_hWnd, _T("确定要退出交易么？"), _T("警告"), MB_OKCANCEL) == IDOK)
	{
		LogOut();
		m_bLogin = FALSE;
		ShowWindow(SW_HIDE);
		ClearData();
	}
	else
		return;

}

void CTradeSimulator::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CTradeSimulator::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CTradeSimulator::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CTradeSimulator::OnBtnBuyAll()
{
	SetEditVol(m_pEditBuyVol, 1);
}

void CTradeSimulator::OnBtnBuyHalf()
{
	SetEditVol(m_pEditBuyVol, 2);
}

void CTradeSimulator::OnBtnBuyThird()
{
	SetEditVol(m_pEditBuyVol, 3);
}

void CTradeSimulator::OnBtnBuyQuarter()
{
	SetEditVol(m_pEditBuyVol, 4);
}

void CTradeSimulator::OnBtnBuy()
{
	SStringA str = StrW2StrA(m_pEditBuyID->GetWindowTextW());
	if (m_stockHash.hash.count(str) == 0)
	{
		SMessageBox(m_hWnd, L"请输入正确的股票代码!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditBuyID->SetFocus();
		return;
	}
	SStringW strPrice = m_pEditBuyPrice->GetWindowTextW();
	int nDotPos = strPrice.Find('.');
	if (nDotPos < 0)
	{
		nDotPos = strPrice.GetLength();
		strPrice += '.';
	}
	int nFloatCount = strPrice.GetLength() - nDotPos - 1;
	for (int i = nFloatCount; i < 2; ++i)
		strPrice += '0';

	long long llIntPart = _wtoi64(strPrice.Left(nDotPos));
	long long llFloatPart = _wtoi64(strPrice.Right(2));
	long long llPrice = llIntPart * 100 + llFloatPart;
	if (llPrice == 0)
	{
		SMessageBox(m_hWnd, L"请输入价格!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditBuyPrice->SetFocus();
		return;
	}

	if (m_setting.remindLimitPrice || m_setting.remindCagePrice)
	{
		if (!CheckPriceIsLeagal(eTD_Buy, llPrice))
		{
			SMessageBox(m_hWnd, L"委托价格超出价格限制!", L"提示", MB_OK | MB_ICONWARNING);
			m_pEditBuyPrice->SetFocus();
			return;
		}
	}
	int nCount = _wtoi(m_pEditBuyVol->GetWindowTextW());
	BOOL bVolLeagl = TRUE;
	if (nCount < 100)
		bVolLeagl = FALSE;
	if (str.Left(3) == "688")
	{
		if (nCount < 200)
			bVolLeagl = FALSE;
	}
	else
	{
		if (nCount % 100 != 0)
			bVolLeagl = FALSE;
	}

	if (nCount > m_llMaxBuy)
		bVolLeagl = FALSE;


	if (!bVolLeagl)
	{
		SMessageBox(m_hWnd, L"请输入正确的数量!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditBuyVol->SetFocus();
		return;
	}

	TrustInfo info = { 0 };
	strcpy_s(info.Account, m_accInfo.ID);
	strcpy_s(info.SecurityID, str);
	strcpy_s(info.SecurityName, m_stockHash.hash[str].SecurityName);
	info.TrustPrice = TradeDouble(llPrice, 2);
	info.TrustVol = TradeDouble(nCount, 0);
	info.Direct = eTD_Buy;
	if (m_setting.trustConfirm)
	{
		CDlgTrustConfirm dlg(info);
		if (dlg.DoModal(m_hWnd) != IDOK)
			return;
	}
	SendMsg(m_tradeSynThreadID, TradeSyn_SubmitTrade,
		(char*)&info, sizeof(info));
}

void CTradeSimulator::OnBtnSellAll()
{
	SetEditVol(m_pEditSellVol, 1);
}

void CTradeSimulator::OnBtnSellHalf()
{
	SetEditVol(m_pEditSellVol, 2);
}

void CTradeSimulator::OnBtnSellThird()
{
	SetEditVol(m_pEditSellVol, 3);
}

void CTradeSimulator::OnBtnSellQuarter()
{
	SetEditVol(m_pEditSellVol, 4);
}

void CTradeSimulator::OnBtnSell()
{
	SStringA str = StrW2StrA(m_pEditSellID->GetWindowTextW());
	if (m_stockHash.hash.count(str) == 0)
	{
		SMessageBox(m_hWnd, L"请输入正确的股票代码!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditSellID->SetFocus();
		return;
	}

	SStringW strPrice = m_pEditSellPrice->GetWindowTextW();
	int nDotPos = strPrice.Find('.');
	if (nDotPos < 0)
	{
		nDotPos = strPrice.GetLength();
		strPrice += '.';
	}
	int nFloatCount = strPrice.GetLength() - nDotPos - 1;
	for (int i = nFloatCount; i < 2; ++i)
		strPrice += '0';

	long long llIntPart = _wtoi64(strPrice.Left(nDotPos));
	long long llFloatPart = _wtoi64(strPrice.Right(2));
	long long llPrice = llIntPart * 100 + llFloatPart;
	if (llPrice == 0)
	{
		SMessageBox(m_hWnd, L"请输入价格!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditSellPrice->SetFocus();
		return;
	}

	if (m_setting.remindLimitPrice || m_setting.remindCagePrice)
	{
		if (!CheckPriceIsLeagal(eTD_Sell, llPrice))
		{
			SMessageBox(m_hWnd, L"委托价格超出价格限制!", L"提示", MB_OK | MB_ICONWARNING);
			m_pEditBuyPrice->SetFocus();
			return;
		}
	}

	int nCount = _wtoi(m_pEditSellVol->GetWindowTextW());
	BOOL bVolLeagl = TRUE;
	if (str.Left(3) == "688")
	{
		if (nCount < 200)
		{
			::EnterCriticalSection(&m_csPosition);
			int nTotal = m_PosInfoMap[str].UsablePos.data;
			::LeaveCriticalSection(&m_csPosition);
			int nLeft = nTotal % 200;

			if (nCount != nLeft)
				bVolLeagl = FALSE;
		}
	}
	else
	{
		::EnterCriticalSection(&m_csPosition);
		int nTotal = m_PosInfoMap[str].UsablePos.data;
		::LeaveCriticalSection(&m_csPosition);
		int nLeft = nTotal % 100;
		if (nCount == 0)
			bVolLeagl = FALSE;
		else if (nCount % 100 != 0 && nCount % 100 != nLeft)
			bVolLeagl = FALSE;
	}
	if (nCount > m_llMaxSell)
		bVolLeagl = FALSE;

	if (!bVolLeagl)
	{
		SMessageBox(m_hWnd, L"请输入正确的数量!", L"错误", MB_OK | MB_ICONERROR);
		m_pEditSellVol->SetFocus();
		return;
	}
	TrustInfo info = { 0 };
	strcpy_s(info.Account, m_accInfo.ID);
	strcpy_s(info.SecurityID, str);
	strcpy_s(info.SecurityName, m_stockHash.hash[str].SecurityName);
	info.TrustPrice = TradeDouble(llPrice, 2);
	info.TrustVol = TradeDouble(nCount, 0);
	info.Direct = eTD_Sell;
	if (m_setting.trustConfirm)
	{
		CDlgTrustConfirm dlg(info);
		if (dlg.DoModal(m_hWnd) != IDOK)
			return;
	}

	SendMsg(m_tradeSynThreadID, TradeSyn_SubmitTrade,
		(char*)&info, sizeof(info));

}

void CTradeSimulator::OnBtnBuyCon()
{
}

void CTradeSimulator::OnBtnBuyUnCon()
{
}

void CTradeSimulator::OnBtnSellCon()
{
}

void CTradeSimulator::OnBtnSellUnCon()
{
}

void CTradeSimulator::OnBtnRefreshAcc()
{
	SStringA str;
	str.Format("ID:%s;", m_accInfo.ID);
	SendMsg(m_tradeSynThreadID, TradeSyn_QueryAccountInfo,
		str.GetBuffer(0), str.GetLength() + 1);

}

void CTradeSimulator::OnBtnPosIDSearch()
{
	SStringW str = m_pEditPosIDSearch->GetWindowTextW();
	UpdateListData(m_pLsPosition, str);
}

void CTradeSimulator::OnBtnPosDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_持仓_%d.csv", m_accInfo.AccountID, nDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName, std::ios::out);
		if (ofile.is_open())
		{
			SaveListData(m_pLsPosition, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}

void CTradeSimulator::OnBtnTrustIDSearch()
{
	SStringW str = m_pEditTrustIDSearch->GetWindowTextW();
	UpdateListData(m_pLsTrust, str);
}

void CTradeSimulator::OnBtnTrustDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_当日委托_%d.csv", m_accInfo.AccountID, nDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsTrust, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}

void CTradeSimulator::OnBtnDealIDSearch()
{
	SStringW str = m_pEditDealIDSearch->GetWindowTextW();
	UpdateListData(m_pLsDeal, str);
}

void CTradeSimulator::OnBtnDealDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_当日成交_%d.csv", m_accInfo.AccountID, nDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsDeal, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}

void CTradeSimulator::OnBtnDealSumSearch()
{
	SStringW str = m_pEditDealSumSearch->GetWindowTextW();
	UpdateListData(m_pLsDealSum, str);
}

void CTradeSimulator::OnBtnDealSumDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_成交汇总_%d.csv", m_accInfo.AccountID, nDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsDealSum, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}

void CTradeSimulator::OnChkCancelAll()
{

	BOOL bChceck = m_pChkSelectAll->IsChecked();
	int nTotalItem = m_pLsCancel->GetItemCount();

	for (int i = 0; i < nTotalItem; ++i)
		m_pLsCancel->SetCheckState(i, bChceck);
	m_pLsCancel->Invalidate();
}

void CTradeSimulator::OnBtnCancelMulti()
{
	if (m_setting.cancelConfirm)
	{
		if (SMessageBox(m_hWnd, L"是否要撤销选中委托？", L"提示", MB_OKCANCEL) != IDOK)
			return;
	}
	for (int i = 0; i < m_pLsCancel->GetItemCount(); ++i)
	{
		if (!m_pLsCancel->GetCheckState(i))
			continue;
		SStringA strStockID = StrW2StrA(m_pLsCancel->GetSubItemText(i, SCH_SecurityID));
		int nID = _wtoi(m_pLsCancel->GetSubItemText(i, SCH_ApplyID));
		SStringW strDirect = m_pLsCancel->GetSubItemText(i, SCH_Direct);
		TrustInfo info = { 0 };
		strcpy_s(info.Account, m_accInfo.ID);
		strcpy_s(info.SecurityID, strStockID);
		strcpy_s(info.SecurityName, m_stockHash.hash[info.SecurityID].SecurityName);

		info.ApplyID = nID;
		if (strDirect == L"买入")
			info.Direct = eTD_CancelBuy;
		else if (strDirect == L"卖出")
			info.Direct = eTD_CancelSell;
		SendMsg(m_tradeSynThreadID, TradeSyn_SubmitTrade,
			(char*)&info, sizeof(info));
	}

}

void CTradeSimulator::OnBtnCancelAll()
{
	if (m_setting.cancelConfirm)
	{
		if (SMessageBox(m_hWnd, L"是否要撤销所有委托？", L"提示", MB_OKCANCEL) != IDOK)
			return;
	}

	for (int i = 0; i < m_pLsCancel->GetItemCount(); ++i)
	{
		SStringA strStockID = StrW2StrA(m_pLsCancel->GetSubItemText(i, SCH_SecurityID));
		int nID = _wtoi(m_pLsCancel->GetSubItemText(i, SCH_ApplyID));
		SStringW strDirect = m_pLsCancel->GetSubItemText(i, SCH_Direct);
		TrustInfo info = { 0 };
		strcpy_s(info.Account, m_accInfo.ID);
		strcpy_s(info.SecurityID, strStockID);
		strcpy_s(info.SecurityName, m_stockHash.hash[info.SecurityID].SecurityName);
		info.ApplyID = nID;
		if (strDirect == L"买入")
			info.Direct = eTD_CancelBuy;
		else if (strDirect == L"卖出")
			info.Direct = eTD_CancelSell;
		SendMsg(m_tradeSynThreadID, TradeSyn_SubmitTrade,
			(char*)&info, sizeof(info));

	}
}

void CTradeSimulator::OnBtnCancekDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_可撤委托_%d.csv", m_accInfo.AccountID, nDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsCancel, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}


void CTradeSimulator::OnBtnHisTrustSearch()
{
	SYSTEMTIME st;
	m_pDtpHisTrustStart->GetTime(st);
	int nStartDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_pDtpHisTrustEnd->GetTime(st);
	int nEndDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nEndDate - nStartDate > 10000)
	{
		SMessageBox(m_hWnd, L"每次最多可以查询一年的时间！", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}

	m_nHisTrustStartDate = nStartDate;
	m_nHisTrustEndDate = nEndDate;
	if (m_nHisTrustDataStrat <= m_nHisTrustStartDate
		&&m_nHisTrustDataEnd >= m_nHisTrustEndDate)
	{
		SStringW str = m_pEditHisTrustSearch->GetWindowTextW();
		UpdateListData(m_pLsHisTrust, str);
	}
	else
	{
		SStringA str;
		str.Format("ID:%s,StartDate:%d,EndDate:%d;",
			m_accInfo.ID, m_nHisTrustStartDate, m_nHisTrustEndDate);
		SendMsg(m_tradeSynThreadID, TradeSyn_QueryHisTrust,
			str.GetBuffer(0), str.GetLength() + 1);
		m_nHisTrustDataStrat = m_nHisTrustStartDate;
		m_nHisTrustDataEnd = m_nHisTrustEndDate;

	}
}

void CTradeSimulator::OnBtnHisTrustDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_历史委托_%d-%d.csv", m_accInfo.AccountID, m_nHisTrustStartDate,
		m_nHisTrustDataEnd);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsHisTrust, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}


void CTradeSimulator::OnBtnHisDealSearch()
{
	SYSTEMTIME st;
	m_pDtpHisDealStart->GetTime(st);
	int nStartDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_pDtpHisDealEnd->GetTime(st);
	int nEndDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nEndDate - nStartDate > 10000)
	{
		SMessageBox(m_hWnd, L"每次最多可以查询一年的时间！", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}
	m_nHisDealStartDate = nStartDate;
	m_nHisDealEndDate = nEndDate;

	if (m_nHisDealDataStrat <= m_nHisDealStartDate
		&&m_nHisDealDataEnd >= m_nHisDealEndDate)
	{
		SStringW str = m_pEditHisDealSearch->GetWindowTextW();
		UpdateListData(m_pLsHisDeal, str);
	}
	else
	{
		SStringA str;
		str.Format("ID:%s,StartDate:%d,EndDate:%d;",
			m_accInfo.ID, m_nHisDealStartDate, m_nHisDealEndDate);
		SendMsg(m_tradeSynThreadID, TradeSyn_QueryHisDeal,
			str.GetBuffer(0), str.GetLength() + 1);
		m_nHisDealDataStrat = m_nHisDealStartDate;
		m_nHisDealDataEnd = m_nHisDealEndDate;
	}
}

void CTradeSimulator::OnBtnHisDealDownload()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"%d_历史成交_%d-%d.csv", m_accInfo.AccountID, m_nHisDealStartDate,
		m_nHisDealDataEnd);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		std::ofstream ofile(FileName);
		if (ofile.is_open())
		{
			SaveListData(m_pLsHisDeal, ofile);
			ofile.close();
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);
	}
}

bool CTradeSimulator::OnListLDoubleClicked(EventArgs * e)
{
	EventLCDbClick* pEvt = (EventLCDbClick*)e;
	SColorListCtrlEx* pList = (SColorListCtrlEx*)pEvt->sender;
	int nSel = pEvt->nCurSel;
	SStringW wstrStockID = pList->GetSubItemText(nSel, m_StockIdInListMap[pList]);
	SStringA strStockID = StrW2StrA(wstrStockID);
	int nTabSel = m_pTabTrade->GetCurSel();
	if (nTabSel == 0)
	{
		m_pEditBuyID->SetWindowTextW(wstrStockID);
		m_pLbBuyID->SetVisible(FALSE, TRUE);
		m_pEditBuyPrice->SetFocus();
	}
	else
	{
		m_pEditSellID->SetWindowTextW(wstrStockID);
		m_pLbSellID->SetVisible(FALSE, TRUE);
		m_pEditSellPrice->SetFocus();
	}
	::SendMessage(m_hWnd, WM_TRADE_MSG, (WPARAM)&strStockID, TSMsg_SetTradeStock);
	return true;
}

bool CTradeSimulator::OnListCancelLBClicked(EventArgs * e)
{
	EventLCSelChanged* pEvt = (EventLCSelChanged*)e;
	int nSel = pEvt->nNewSel;
	BOOL bChecked = m_pLsCancel->GetCheckState(nSel);
	BOOL bAllChecked = m_pChkSelectAll->IsChecked();
	if (bAllChecked)
	{
		if (!bChecked)
			m_pChkSelectAll->SetCheck(FALSE);
	}
	else
	{
		if (bChecked)
		{
			int nTotal = m_pLsCancel->GetItemCount();
			BOOL bSet = TRUE;
			for (int i = 0; i < nTotal; ++i)
			{
				if (!m_pLsCancel->GetCheckState(i))
				{
					bSet = FALSE;
					break;
				}
			}
			m_pChkSelectAll->SetCheck(bSet);
		}
	}
	return true;
}

bool CTradeSimulator::OnEditSearchChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	SColorListCtrlEx* pList = m_editListMap[pEdit];
	SStringW str = pEdit->GetWindowTextW();
	UpdateListData(pList, str);
	return true;
}

bool CTradeSimulator::OnEditTradeIDChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	SListBox* pList = m_IDListMap[pEdit];
	if (pEvt->iNotify == EN_KILLFOCUS)
	{
		if (pList->IsVisible())
			pList->SetVisible(FALSE, TRUE);
		return true;
	}
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	SStringW str = pEdit->GetWindowTextW();
	SStringA strID = StrW2StrA(str);
	if (str.IsEmpty())
	{
		pList->SetVisible(FALSE, TRUE);
		return true;
	}
	pList->DeleteAll();
	CRect rc = pList->GetClientRect();
	int nItemHeight = pList->GetItemHeight();
	int nMaxItem = rc.Height() / nItemHeight;
	int nCount = 0;
	for (auto& it : m_stockVec)
	{
		if (strstr(it.SecurityID, strID.GetBuffer(0)))
		{
			SStringW strShow;
			strShow.Format(L"%s  %s", StrA2StrW(it.SecurityID),
				StrA2StrW(it.SecurityName));
			pList->InsertString(nCount++, strShow);
			if (nCount == nMaxItem)
				break;
		}
	}
	if (nCount == 0)
		pList->SetVisible(FALSE, TRUE);
	else
	{
		pList->SetCurSel(0);
		m_bSetFirst = TRUE;
		pList->SetVisible(TRUE, TRUE);
	}
	return TRUE;
}

bool CTradeSimulator::OnEditVolChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	SStringW str = pEdit->GetWindowTextW();
	SStatic* pText = m_volTextMap[pEdit];
	int nLength = str.GetLength();
	if (!isdigit(str[nLength - 1]))
	{
		SStringW dstStr = L"";
		for (int i = 0; i < nLength - 1; ++i)
			dstStr += str[i];
		str = dstStr;
		pEdit->SetWindowTextW(str);
		pEdit->SetSel(-1);
	}

	SStringW strBigNum = str.IsEmpty() ? L"" : NumToBig(str);
	pText->SetWindowTextW(strBigNum);
	return true;
}

bool SOUI::CTradeSimulator::OnEditPriceChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;

	if (pEvt->iNotify == EN_KILLFOCUS)
	{
		SetMaxTradeVol(pEdit);
		return true;
	}
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	SStringW str = pEdit->GetWindowTextW();

	int nLength = str.GetLength();
	int nPos = str.Find('.');
	if (nPos >= 0)
	{
		if (nLength - nPos - 1 > 2)
		{
			SStringW dstStr = L"";
			for (int i = 0; i < nPos + 3; ++i)
				dstStr += str[i];
			str = dstStr;
			pEdit->SetWindowTextW(str);
			pEdit->SetSel(-1);
		}
	}
	return true;
}

bool CTradeSimulator::OnLbIDLButtonDown(EventArgs * e)
{
	EventLButtonDown* pEvt = (EventLButtonDown*)e;
	SListBox* pList = (SListBox*)pEvt->sender;
	int nSel = pList->GetCurSel();
	SStringW str = pList->GetText(nSel);
	SStringA strStockID = StrW2StrA(str).Left(6);
	if (pList == m_pLbBuyID)
	{
		m_pEditBuyID->SetWindowTextW(str.Left(6));
		pList->SetVisible(FALSE, TRUE);
		m_pEditBuyPrice->SetFocus();
	}
	else
	{
		m_pEditSellID->SetWindowTextW(str.Left(6));
		pList->SetVisible(FALSE, TRUE);
		m_pEditSellPrice->SetFocus();

	}
	::SendMessage(m_hWnd, WM_TRADE_MSG, (WPARAM)&strStockID, TSMsg_SetTradeStock);
	return true;
}

void CTradeSimulator::SetTradeStock(SStringA strStock)
{
	if (strStock == m_strStock)
		return;
	if (m_stockHash.hash.count(strStock) == 0)
		return;
	m_strStock = strStock;
	m_strStockName = m_stockHash.hash[m_strStock].SecurityName;
	DataGetInfo GetInfo;
	GetInfo.hWnd = m_hWnd;
	strcpy_s(GetInfo.StockID, strStock);
	GetInfo.Period = Period_FenShi;
	GetInfo.Group = Group_Stock;
	SendMsg(m_SynThreadID, Syn_GetMarket, (char*)&GetInfo, sizeof(GetInfo));

}

void CTradeSimulator::UpdateTradeInfo(BOOL bFirst)
{
	m_pTradeInfo->Invalidate();
	::EnterCriticalSection(&m_csMarket);
	auto market = m_marketVec.empty() ? CommonStockMarket() : m_marketVec.back();
	::LeaveCriticalSection(&m_csMarket);
	if (bFirst)
	{
		SStringW str;
		int nSel = m_pTabTrade->GetCurSel();
		if (nSel == 0)
		{
			double fPrice = 0;
			if (m_setting.buyPriceType < eBPT_LastPrice)
				fPrice = market.AskPrice[m_setting.buyPriceType];
			else if (m_setting.buyPriceType == eBPT_LastPrice)
				fPrice = market.LastPrice;
			if (fPrice != 0)
			{
				m_pEditBuyPrice->SetWindowTextW(str.Format(L"%.02f", fPrice));
				m_pEditBuyPrice->SetSel(-1);

				SetMaxTradeVol(m_pEditBuyPrice);
				if (m_setting.buyVolType == eBVT_Empty)
					m_pEditBuyVol->SetWindowTextW(L"");
				else
					SetEditVol(m_pEditBuyVol, m_setting.buyVolType);

				if (m_setting.buyVolFix > 0)
				{
					int nVol = m_setting.buyVolFix - m_setting.buyVolFix % 100;
					nVol = min(nVol, m_llMaxBuy);
					m_pEditBuyVol->SetWindowTextW(str.Format(L"%d", nVol));
				}

				if (m_setting.buyAmo > 0)
				{
					int nVol = m_setting.buyAmo / fPrice;
					nVol -= nVol % 100;
					nVol = min(nVol, m_llMaxBuy);
					m_pEditBuyVol->SetWindowTextW(str.Format(L"%d", nVol));
				}
			}
			else
			{
				m_pEditBuyPrice->SetWindowTextW(L"");
				m_pEditBuyVol->SetWindowTextW(L"");

			}
		}
		else
		{
			double fPrice = 0;
			if (m_setting.sellPriceType < eSPT_LastPrice)
				fPrice = market.BidPrice[m_setting.sellPriceType];
			else if (m_setting.sellPriceType == eSPT_LastPrice)
				fPrice = market.LastPrice;
			if (fPrice != 0)
				m_pEditSellPrice->SetWindowTextW(str.Format(L"%.02f", fPrice));
			else
				m_pEditSellPrice->SetWindowTextW(L"");
			m_pEditSellPrice->SetSel(-1);

			SetMaxTradeVol(m_pEditSellPrice);
			if (m_setting.sellVolType == eSVT_Empty)
				m_pEditSellVol->SetWindowTextW(L"");
			else
				SetEditVol(m_pEditSellVol, m_setting.sellVolType);
		}
	}

}

void CTradeSimulator::UpdateSubmitFeedback(SubmitFeedback sfb)
{
	if (sfb.Feedback == eSF_SubmitSucc)
	{
		int nSel = m_pTabTrade->GetCurSel();
		if (nSel == 0)
		{
			if (m_setting.afterTradeType == eATT_Clear)
			{
				m_pEditBuyID->SetWindowTextW(L"");
				m_pEditBuyPrice->SetWindowTextW(L"");
				m_pEditBuyVol->SetWindowTextW(L"");
			}
			else if (m_setting.afterTradeType == eATT_Id)
			{
				m_pEditBuyPrice->SetWindowTextW(L"");
				m_pEditBuyVol->SetWindowTextW(L"");
			}
			else if (m_setting.afterTradeType == eATT_IdPrice)
				m_pEditBuyVol->SetWindowTextW(L"");

		}
		else
		{
			if (m_setting.afterTradeType == eATT_Clear)
			{
				m_pEditSellID->SetWindowTextW(L"");
				m_pEditSellPrice->SetWindowTextW(L"");
				m_pEditSellVol->SetWindowTextW(L"");
			}
			else if (m_setting.afterTradeType == eATT_Id)
			{
				m_pEditSellPrice->SetWindowTextW(L"");
				m_pEditSellVol->SetWindowTextW(L"");
			}
			else if (m_setting.afterTradeType == eATT_IdPrice)
				m_pEditSellVol->SetWindowTextW(L"");

		}
		if (m_setting.showTrust)
		{
			SStringW str;
			SMessageBoxWithTimer(NULL, str.Format(L"报单成功,委托编号:%d", sfb.TrustID), L"提示", MB_OK, m_setting.windowTime);
		}
	}
	else if (sfb.Feedback == eSF_SubmitFail)
	{
		SStringW str;
		SMessageBoxWithTimer(NULL, str.Format(L"报单错误,委托编号:%d,请检查数量！", sfb.TrustID), L"提示", MB_OK, m_setting.windowTime);
	}
	else if (sfb.Feedback == eSF_SubmitTimeErr)
	{
		SStringW str;
		SMessageBoxWithTimer(NULL, str.Format(L"报单错误,委托编号:%d,非法的时间！", sfb.TrustID), L"提示", MB_OK, m_setting.windowTime);
	}
	else if (sfb.Feedback == eSF_InvalidPrice)
	{
		SStringW str;
		SMessageBoxWithTimer(NULL, str.Format(L"报单错误,委托编号:%d,非法的价格！", sfb.TrustID), L"提示", MB_OK, m_setting.windowTime);
	}
	else if (sfb.Feedback == eSF_CancelSucc)
	{
		SStringW str;
		SMessageBoxWithTimer(NULL, str.Format(L"撤单成功,委托编号:%d,撤销单号:%d", sfb.TrustID, sfb.ApplyID), L"提示", MB_OK, m_setting.windowTime);
	}
	else if (sfb.Feedback == eSF_CancelFail)
	{
		SStringW str;
		SMessageBoxWithTimer(NULL, str.Format(L"撤单失败,委托编号:%d,报单已经成交或撤销", sfb.TrustID), L"提示", MB_OK, m_setting.windowTime);
	}

}
void SOUI::CTradeSimulator::UpdateDealInfo(int nCount)
{
	if (m_setting.showDeal)
	{
		::EnterCriticalSection(&m_csDeal);
		for (int i = nCount; i < m_DealVec.size(); ++i)
		{
			SStringW str;
			SMessageBoxWithTimer(NULL, str.Format(L"委托%d成交,代码:%s,方向:%s,成交数量:%lld",
				m_DealVec[i].ApplyID,
				StrA2StrW(m_DealVec[i].SecurityID),
				m_tradeDirectStrMap[m_DealVec[i].Direct],
				m_DealVec[i].DealVol.data),
				L"成交提示", MB_OK, m_setting.windowTime);
		}
		::LeaveCriticalSection(&m_csDeal);
	}
}
void CTradeSimulator::SaveTradeSetting()
{
	CIniFile ini(".\\config\\trade.cfg");
	ini.WriteIntA("Setting", "buyPriceType", m_setting.buyPriceType);
	ini.WriteIntA("Setting", "buyVolType", m_setting.buyVolType);
	ini.WriteIntA("Setting", "sellPriceType", m_setting.sellPriceType);
	ini.WriteIntA("Setting", "sellVolType", m_setting.sellVolType);
	ini.WriteIntA("Setting", "maxVol", m_setting.maxVol);
	ini.WriteIntA("Setting", "remindLimitPrice", m_setting.remindLimitPrice);
	ini.WriteIntA("Setting", "remindCagePrice", m_setting.remindCagePrice);
	ini.WriteIntA("Setting", "buyVolFix", m_setting.buyVolFix);
	ini.WriteIntA("Setting", "buyAmo", m_setting.buyAmo);
	ini.WriteIntA("Setting", "afterTradeType", m_setting.afterTradeType);
	ini.WriteIntA("Setting", "clickVol", m_setting.clickVol);
	ini.WriteIntA("Setting", "windowTime", m_setting.windowTime);
	ini.WriteIntA("Setting", "showDeal", m_setting.showDeal);
	ini.WriteIntA("Setting", "cancelConfirm", m_setting.cancelConfirm);
	ini.WriteIntA("Setting", "trustConfirm", m_setting.trustConfirm);
	ini.WriteIntA("Setting", "showTrust", m_setting.showTrust);
	ini.WriteIntA("Setting", "changePageClean", m_setting.changePageClean);
}
void CTradeSimulator::GetTradeSetting()
{
	CIniFile ini(".\\config\\trade.cfg");
	m_setting.buyPriceType = ini.GetIntA("Setting", "buyPriceType", 0);
	m_setting.buyVolType = ini.GetIntA("Setting", "buyVolType", 0);
	m_setting.sellPriceType = ini.GetIntA("Setting", "sellPriceType", 0);
	m_setting.sellVolType = ini.GetIntA("Setting", "sellVolType", 0);
	m_setting.maxVol = ini.GetIntA("Setting", "maxVol", 1'000'000);
	m_setting.remindLimitPrice = ini.GetIntA("Setting", "remindLimitPrice", 1);
	m_setting.remindCagePrice = ini.GetIntA("Setting", "remindCagePrice", 1);
	m_setting.buyVolFix = ini.GetIntA("Setting", "buyVolFix", 0);
	m_setting.buyAmo = ini.GetIntA("Setting", "buyAmo", 0);
	m_setting.afterTradeType = ini.GetIntA("Setting", "afterTradeType", 0);
	m_setting.clickVol = ini.GetIntA("Setting", "clickVol", 0);
	m_setting.windowTime = ini.GetIntA("Setting", "windowTime", 5);
	m_setting.showDeal = ini.GetIntA("Setting", "showDeal", 0);
	m_setting.cancelConfirm = ini.GetIntA("Setting", "cancelConfirm", 0);
	m_setting.trustConfirm = ini.GetIntA("Setting", "trustConfirm", 1);
	m_setting.showTrust = ini.GetIntA("Setting", "showTrust", 1);
	m_setting.changePageClean = ini.GetIntA("Setting", "changePageClean", 0);
}
void CTradeSimulator::LogOut()
{
	SendMsg(m_tradeSynThreadID, TradeSyn_Logout,
		m_accInfo.ID, sizeof(m_accInfo.ID));
}

void SOUI::CTradeSimulator::OnSpinBuyPrice()
{
	SSpinButtonCtrlEx * pSpin = FindChildByName2<SSpinButtonCtrlEx>(L"spin_buyPrice");
	int nAction = pSpin->GetAction();
	SStringW str = m_pEditBuyPrice->GetWindowTextW();
	double fPrice = _wtof(str);
	if (nAction == 1)
	{
		fPrice -= 0.01;
		if (fPrice < 0)
			fPrice = 0;
	}
	else
	{
		fPrice += 0.01;
		if (fPrice > 99999.99)
			fPrice = 99999.99;
	}
	m_pEditBuyPrice->SetWindowTextW(str.Format(L"%0.2f", fPrice));
	m_pEditBuyPrice->SetSel(-1);
}

void SOUI::CTradeSimulator::OnSpinSellPrice()
{
	SSpinButtonCtrlEx * pSpin = FindChildByName2<SSpinButtonCtrlEx>(L"spin_sellPrice");
	int nAction = pSpin->GetAction();
	SStringW str = m_pEditSellPrice->GetWindowTextW();
	double fPrice = _wtof(str);
	if (nAction == 1)
	{
		fPrice -= 0.01;
		if (fPrice < 0)
			fPrice = 0;
	}
	else
	{
		fPrice += 0.01;
		if (fPrice > 99999.99)
			fPrice = 99999.99;
	}
	m_pEditSellPrice->SetWindowTextW(str.Format(L"%0.2f", fPrice));
	m_pEditBuyPrice->SetSel(-1);

}

void CTradeSimulator::OnBtnTradeSetting()
{
	CDlgTradeSetting *pDlg = new CDlgTradeSetting(m_hWnd, m_setting);
	pDlg->Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP
		| WS_OVERLAPPED | WS_POPUP, 0, 0, 0, 0, 0);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
	pDlg->ShowWindow(SW_SHOW);
}

void SOUI::CTradeSimulator::OnTab()
{
	if (m_setting.changePageClean)
	{
		int nCurSel = m_pTabTrade->GetCurSel();
		if (nCurSel == 0)
		{
			m_pEditBuyID->SetWindowTextW(L"");
			m_pEditBuyPrice->SetWindowTextW(L"");
			m_pEditBuyVol->SetWindowTextW(L"");
		}
		else
		{
			m_pEditSellID->SetWindowTextW(L"");
			m_pEditSellPrice->SetWindowTextW(L"");
			m_pEditSellVol->SetWindowTextW(L"");
		}
	}
}


LRESULT CTradeSimulator::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)lp;
	switch (Msg)
	{
	case TSMsg_SetTradeStock:
		SetTradeStock(*(SStringA*)wp);
		break;
	case TSMsg_UpdateTradeInfo:
		UpdateTradeInfo((BOOL)wp);
		break;
	case TSMsg_UpdateAccountInfo:
		UpdateAccountInfo();
		break;
	case TSMsg_UpdatePosInfo:
		UpdatePositionList(StrA2StrW(m_strPosSearch));
		break;
	case TSMsg_UpdateTrustAndCancelInfo:
		UpdateTrustList(StrA2StrW(m_strTrustSearch));
		UpdateCancelList();
		break;
	case TSMsg_UpdateDealInfo:
		UpdateDealList(StrA2StrW(m_strDealSearch));
		UpdateDealSummary(StrA2StrW(m_strDealSumSearch));
		break;
	case TSMsg_UpdateHisTrust:
		UpdateHisTrustList(StrA2StrW(m_strHisTrustSearch));
		break;
	case TSMsg_UpdateHisDeal:
		UpdateHisDealList(StrA2StrW(m_strHisDealSearch));
		break;
	case TSMsg_UpdateSubmitFeedback:
		UpdateSubmitFeedback(*(SubmitFeedback*)wp);
		break;
	case TSMsg_ShowWindow:
		ShowWindow(SW_SHOW);
		break;
	case TSMsg_ChangeSetting:
		m_setting = *(TradeSetting*)wp;
		SaveTradeSetting();
		break;
	case TSMsg_ShowDeal:
		UpdateDealInfo((int)wp);
		break;
	default:
		break;
	}
	return 0;
}

void CTradeSimulator::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (m_pLbBuyID->IsVisible())
		m_pLbBuyID->SSendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
	if (m_pLbSellID->IsVisible())
		m_pLbSellID->SSendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
	if (nChar == VK_RETURN)
	{
		if (m_pEditBuyID->IsFocused())
		{
			if (m_pLbBuyID->IsVisible())
			{
				int nSel = m_pLbBuyID->GetCurSel();
				SStringW str = m_pLbBuyID->GetText(nSel);
				SStringA strStockID = StrW2StrA(str).Left(6);
				m_pEditBuyID->SetWindowTextW(str.Left(6));
				::SendMessage(m_hWnd, WM_TRADE_MSG, (WPARAM)&strStockID, TSMsg_SetTradeStock);
			}
			m_pEditBuyPrice->SetFocus();
		}
		else if (m_pEditBuyPrice->IsFocused())
			m_pEditBuyVol->SetFocus();
		else if (m_pEditBuyVol->IsFocused())
			OnBtnBuy();

		if (m_pEditSellID->IsFocused())
		{
			if (m_pLbSellID->IsVisible())
			{
				int nSel = m_pLbSellID->GetCurSel();
				SStringW str = m_pLbSellID->GetText(nSel);
				SStringA strStockID = StrW2StrA(str).Left(6);
				m_pEditSellID->SetWindowTextW(str.Left(6));
				::SendMessage(m_hWnd, WM_TRADE_MSG, (WPARAM)&strStockID, TSMsg_SetTradeStock);
			}
			m_pEditSellPrice->SetFocus();
		}
		else if (m_pEditSellPrice->IsFocused())
			m_pEditSellVol->SetFocus();
		else if (m_pEditSellVol->IsFocused())
			OnBtnSell();

	}

}

void CTradeSimulator::OnMouseMove(UINT nFlags, CPoint pt)
{
	SetMsgHandled(FALSE);
	if (m_bSetFirst)
	{
		m_bSetFirst = FALSE;
		m_ptMouse = pt;
	}

	if (pt != m_ptMouse)
	{
		if (m_pLbBuyID->IsVisible())
			m_pLbBuyID->SetCurSel(m_pLbBuyID->HitTest(pt));

		if (m_pLbSellID->IsVisible())
			m_pLbSellID->SetCurSel(m_pLbSellID->HitTest(pt));
	}
}

void CTradeSimulator::OnSize(UINT nType, CSize size)
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

void CTradeSimulator::OnTimer(UINT_PTR nID)
{
	if (nID == 1)
	{
		::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdatePosInfo);
		::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateAccountInfo);
	}
}

void CTradeSimulator::DataMsgProc()
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
		if (m_DataHandleMap.count(MsgId))
		{
			auto pFuc = m_DataHandleMap[MsgId];
			if (pFuc)
				(this->*pFuc)(msgLength, info);
		}
		delete[]info;
		info = nullptr;
	}

}

void CTradeSimulator::OnUpdateHisStockMarket(int nMsgLength, const char * info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(CommonStockMarket);
	CommonStockMarket * dataArr = (CommonStockMarket *)(info + nOffset);
	::EnterCriticalSection(&m_csMarket);
	m_marketVec.reserve(MAX_TICK);
	m_marketVec.resize(dataCount);
	memcpy_s(&m_marketVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	::LeaveCriticalSection(&m_csMarket);

	m_pTradeInfo->ChangeShowStock(m_strStock, m_strStockName);
	::PostMessage(m_hWnd, WM_TRADE_MSG, TRUE, TSMsg_UpdateTradeInfo);
}

void CTradeSimulator::OnUpdateStockMarket(int nMsgLength, const char * info)
{
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	::EnterCriticalSection(&m_csMarket);
	m_marketVec.emplace_back(*pStockData);
	::LeaveCriticalSection(&m_csMarket);
	::PostMessage(m_hWnd, WM_TRADE_MSG, FALSE, TSMsg_UpdateTradeInfo);

}

void CTradeSimulator::OnUpdateCloseInfo(int nMsgLength, const char * info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	strHash<double> preCloseMap;
	for (int i = 0; i < dataCount; ++i)
		preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
	::EnterCriticalSection(&m_csClose);
	m_preCloseMap.hash = preCloseMap.hash;
	::LeaveCriticalSection(&m_csClose);

}

void CTradeSimulator::OnTradeLogin(int nMsgLength, const char * info)
{
	m_bLogin = TRUE;
	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_ShowWindow);
}

void CTradeSimulator::OnTradeLogout(int nMsgLength, const char * info)
{
	m_bLogin = FALSE;
	ClearData();
}

void CTradeSimulator::OnAccountInfo(int nMsgLength, const char * info)
{
	::EnterCriticalSection(&m_csAccount);
	m_accInfo = *(AccInfo*)info;
	::LeaveCriticalSection(&m_csAccount);

	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateAccountInfo);
}

void CTradeSimulator::OnPosition(int nMsgLength, const char * info)
{
	int nPositionCount = nMsgLength / sizeof(PositionInfo);
	PositionInfo* pData = (PositionInfo*)info;
	::EnterCriticalSection(&m_csPosition);
	for (int i = 0; i < nPositionCount; ++i)
		m_PosInfoMap[pData[i].SecurityID] = pData[i];
	::LeaveCriticalSection(&m_csPosition);
	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdatePosInfo);
}

void CTradeSimulator::OnTrust(int nMsgLength, const char * info)
{
	int nTrustCount = nMsgLength / sizeof(TrustInfo);
	TrustInfo* pData = (TrustInfo*)info;
	::EnterCriticalSection(&m_csTrust);
	for (int i = 0; i < nTrustCount; ++i)
		m_TrustVec.emplace_back(pData[i]);
	::LeaveCriticalSection(&m_csTrust);
	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateTrustAndCancelInfo);
}

void CTradeSimulator::OnDeal(int nMsgLength, const char * info)
{
	int nDealCount = nMsgLength / sizeof(DealInfo);
	DealInfo* pData = (DealInfo*)info;
	::EnterCriticalSection(&m_csDeal);
	int nShowCount = m_DealVec.size();;
	for (int i = 0; i < nDealCount; ++i)
	{
		m_DealVec.emplace_back(pData[i]);
		auto &dealMap = m_DealSumMap[pData[i].SecurityID];
		if (dealMap.count(pData[i].Direct) == 0)
			dealMap[pData[i].Direct] = pData[i];
		else
		{
			auto &dealSum = dealMap[pData[i].Direct];
			dealSum.DealVol = dealSum.DealVol + pData[i].DealVol;
			dealSum.DealAmo = dealSum.DealAmo + pData[i].DealAmo;
			dealSum.DealPrice = dealSum.DealAmo / dealSum.DealVol;
		}
	}
	::LeaveCriticalSection(&m_csDeal);

	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateDealInfo);
	if (m_bLogin)
		::SendMessage(m_hWnd, WM_TRADE_MSG, nShowCount, TSMsg_ShowDeal);

}

void CTradeSimulator::OnHisTrust(int nMsgLength, const char * info)
{
	int nTrustCount = nMsgLength / sizeof(TrustInfo);
	m_HisTrustVec.resize(nTrustCount);
	memcpy_s(&m_HisTrustVec[0], nMsgLength, info, nMsgLength);
	std::sort(m_HisTrustVec.begin(), m_HisTrustVec.end(),
		[&](const TrustInfo& a, const TrustInfo& b)
	{
		if (a.Date == b.Date)
			return a.Time < b.Time;
		return a.Date < b.Date;
	});
	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateHisTrust);

}

void CTradeSimulator::OnHisDeal(int nMsgLength, const char * info)
{
	int nDealCount = nMsgLength / sizeof(DealInfo);
	m_HisDealVec.resize(nDealCount);
	memcpy_s(&m_HisDealVec[0], nMsgLength, info, nMsgLength);
	std::sort(m_HisDealVec.begin(), m_HisDealVec.end(),
		[&](const DealInfo& a, const DealInfo& b)
	{
		if (a.Date == b.Date)
			return a.Time < b.Time;
		return a.Date < b.Date;
	});
	::SendMessage(m_hWnd, WM_TRADE_MSG, NULL, TSMsg_UpdateHisDeal);

}

void CTradeSimulator::OnSubmitFeedback(int nMsgLength, const char * info)
{
	SubmitFeedback* sfb = (SubmitFeedback*)info;
	::SendMessage(m_hWnd, WM_TRADE_MSG, (WPARAM)(char*)sfb, TSMsg_UpdateSubmitFeedback);
}

void CTradeSimulator::SaveListData(SColorListCtrlEx * pList, std::ofstream & ofile)
{
	SHeaderCtrlEx* pHeader = (SHeaderCtrlEx*)pList->GetWindow(GSW_FIRSTCHILD);
	int nItemCount = pHeader->GetItemCount();
	for (int i = 0; i < nItemCount; ++i)
	{
		SHDITEM si;
		si.mask |= SHDI_TEXT;
		pHeader->GetItem(i, &si);
		ofile << StrW2StrA(si.strText.GetText(TRUE));
		if (i != nItemCount - 1)
			ofile << ",";
		else
			ofile << std::endl;
	}
	int nListCount = pList->GetItemCount();
	for (int i = 0; i < nListCount; ++i)
	{
		for (int j = 0; j < nItemCount; ++j)
		{
			ofile << StrW2StrA(pList->GetSubItemText(i, j));
			if (j != nItemCount - 1)
				ofile << ",";
			else
				ofile << std::endl;
		}
	}
}

SStringW SOUI::CTradeSimulator::NumberWithSeparator(SStringW str)
{
	SStringW res;
	int nDotPos = str.Find('.');
	if (nDotPos == -1)
		nDotPos = str.GetLength();
	for (int i = 0; i < str.GetLength(); ++i)
	{
		res += str[i];
		int nPos = nDotPos - i;
		if (nPos > 3 && nPos % 3 == 1)
			res += ',';
	}
	return res;
}

bool SOUI::CTradeSimulator::CheckPriceIsLeagal(int nDirect, long long llPrice)
{
	::EnterCriticalSection(&m_csMarket);
	auto market = m_marketVec.back();
	::LeaveCriticalSection(&m_csMarket);

	double fLimitRatio = 0.1;
	if (market.SecurityID[0] == '3' || (market.SecurityID[0] == '6' &&
		market.SecurityID[1] == '8' && market.SecurityID[2] == '8'))
		fLimitRatio = 0.2;
	long long llUpLimit = market.PreCloPrice * (1 + fLimitRatio) * 100 + 0.5;
	long long llLowLimit = market.PreCloPrice * (1 - fLimitRatio) * 100 + 0.5;

	if (m_setting.remindLimitPrice)
	{
		if (llPrice > llUpLimit
			|| llPrice < llLowLimit)
			return false;
	}

	if (m_setting.remindCagePrice)
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		int nTime = st.wHour * 10000 + st.wMinute * 100 + st.wSecond;
		if (nTime < 92500 || nTime >= 145700 && nTime < 150000)
			return true;

		if (eTD_Buy == nDirect)
		{
			double fTrustLimit = market.AskPrice[0] != 0 ?
				max(market.AskPrice[0] * (1 + PRICELIMIT), market.AskPrice[0] + 0.1) : 0;
			long long llTrustLimit = fTrustLimit * 100 + 0.5;
			if (llTrustLimit == 0)
				llTrustLimit = llUpLimit;
			if (llPrice > llTrustLimit)
				return false;
		}
		else if (eTD_Sell == nDirect)
		{
			double fTrustLimit = market.BidPrice[0] != 0 ?
				min(market.BidPrice[0] * (1 - PRICELIMIT), market.BidPrice[0] - 0.1) : 0;
			long long llTrustLimit = fTrustLimit * 100 + 0.5;
			if (llTrustLimit == 0)
				llTrustLimit = llLowLimit;

			if (llPrice < llTrustLimit)
				return false;

		}

	}
	return true;

}


void CTradeSimulator::SetEditVol(SEdit * pEdit, int nDivisor)
{

	long long nMaxVol = pEdit == m_pEditBuyVol ? m_llMaxBuy / 100 / nDivisor :
		m_llMaxSell / 100 / nDivisor;
	nMaxVol *= 100;
	SStringW str;
	str.Format(L"%lld", nMaxVol);
	pEdit->SetWindowTextW(str);

	EventRENotify evt(pEdit);
	evt.iNotify = EN_CHANGE;
	pEdit->FireEvent(evt);
}


void CTradeSimulator::UpdateListData(SColorListCtrlEx * pList, SStringW str)
{
	if (pList == m_pLsPosition)
		UpdatePositionList(str);
	else if (pList == m_pLsTrust)
		UpdateTrustList(str);
	else if (pList == m_pLsCancel)
		UpdateCancelList();
	else if (pList == m_pLsDeal)
		UpdateDealList(str);
	else if (pList == m_pLsDealSum)
		UpdateDealSummary(str);
	else if (pList == m_pLsHisTrust)
		UpdateHisTrustList(str);
	else if (pList == m_pLsHisDeal)
		UpdateHisDealList(str);
}

void CTradeSimulator::UpdatePositionList(SStringW str)
{
	if (!m_bDataInited)
		return;
	SStringA strNew = StrW2StrA(str);
	if (m_strPosSearch != strNew)
	{
		m_pLsPosition->DeleteAllItems();
		m_PosInfoPosMap.clear();
		m_strPosSearch = strNew;
	}
	int nInsertPos = m_pLsPosition->GetItemCount();
	::EnterCriticalSection(&m_csPosition);
	auto posMap = m_PosInfoMap;
	::LeaveCriticalSection(&m_csPosition);
	for (auto &it : posMap)
	{
		auto& posInfo = it.second;
		if (str.IsEmpty() || strstr(posInfo.SecurityID, m_strPosSearch)
			|| strstr(posInfo.SecurityName, m_strPosSearch))
		{
			if (m_PosInfoPosMap.count(posInfo.SecurityID) == 0)
			{
				m_pLsPosition->InsertItem(nInsertPos, StrA2StrW(posInfo.SecurityID));
				m_pLsPosition->SetSubItemText(nInsertPos, SPH_SecurityName, StrA2StrW(posInfo.SecurityName));
				m_PosInfoPosMap[posInfo.SecurityID] = nInsertPos;
				++nInsertPos;
			}
			int nPos = m_PosInfoPosMap[posInfo.SecurityID];
			m_pLsPosition->SetSubItemText(nPos, SPH_YesterdayPos, str.Format(L"%.02f", posInfo.YestdayPos.GetDouble()));
			m_pLsPosition->SetSubItemText(nPos, SPH_TotalPos, str.Format(L"%.02f", posInfo.TotalPos.GetDouble()));
			m_pLsPosition->SetSubItemText(nPos, SPH_UsablePos, str.Format(L"%.02f", posInfo.UsablePos.GetDouble()));
			double fCostPrice = 0;
			if (posInfo.TotalPos.GetDouble() != 0)
				fCostPrice = posInfo.Cost.GetDouble() / posInfo.TotalPos.GetDouble();
			m_pLsPosition->SetSubItemText(nPos, SPH_CostPrice, str.Format(L"%.03f", fCostPrice));
			double fLastPrice = m_pListDataMap->count(Period_1Day) ?
				m_pListDataMap->at(Period_1Day).hash[posInfo.SecurityID].fPrice : 0;
			if (fLastPrice == 0)
				fLastPrice = m_preCloseMap.hash[posInfo.SecurityID];
			m_pLsPosition->SetSubItemText(nPos, SPH_LastPrice, str.Format(L"%.02f", fLastPrice));
			m_pLsPosition->SetSubItemText(nPos, SPH_Cost, str.Format(L"%.02f", posInfo.Cost.GetDouble()));
			double fMarketValue = posInfo.TotalPos.GetDouble() * fLastPrice;
			m_pLsPosition->SetSubItemText(nPos, SPH_MarketValue, str.Format(L"%.02f", fMarketValue));
			double fPL = fMarketValue - posInfo.Cost.GetDouble();
			COLORREF color = fPL == 0 ? RGBA(255, 255, 255, 255) : (fPL > 0 ? RGBA(255, 0, 0, 255) : RGBA(0, 225, 0, 255));
			m_pLsPosition->SetSubItemText(nPos, SPH_FloatingPL, str.Format(L"%.02f", fPL), color);
			double fPLPct = 0;
			if (posInfo.Cost.GetDouble() != 0)
				fPLPct = fPL / abs(posInfo.Cost.GetDouble()) * 100;
			m_pLsPosition->SetSubItemText(nPos, SPH_FloatingPLPct, str.Format(L"%.02f", fPLPct), color);
			m_pLsPosition->SetSubItemText(nPos, SPH_FreezedPos, str.Format(L"%.02f", posInfo.FreezedPos.GetDouble()));
			m_pLsPosition->SetSubItemText(nPos, SPH_InTransitPos, str.Format(L"%.02f", posInfo.InTransitPos.GetDouble()));
			m_pLsPosition->SetSubItemText(nPos, SPH_Account, str.Format(L"%d", posInfo.AccountID));
		}
	}
	m_pLsPosition->Invalidate();
	m_pTxtPosCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsPosition->GetItemCount()));
}

void CTradeSimulator::UpdateTrustList(SStringW str)
{
	SStringA strNew = StrW2StrA(str);
	if (m_strTrustSearch != strNew)
	{
		m_pLsTrust->DeleteAllItems();
		m_TrustInfoPosMap.clear();
		m_strTrustSearch = strNew;
	}
	::EnterCriticalSection(&m_csTrust);
	auto TrustVec = m_TrustVec;
	::LeaveCriticalSection(&m_csTrust);
	for (int i = 0; i < TrustVec.size(); ++i)
	{
		auto& TrustInfo = TrustVec[i];
		if (str.IsEmpty() || strstr(TrustInfo.SecurityID, m_strTrustSearch)
			|| strstr(TrustInfo.SecurityName, m_strTrustSearch))
		{
			if (m_TrustInfoPosMap.count(TrustInfo.TrustID) == 0)
			{
				int nPos = m_pLsTrust->InsertItem(0, str.Format(L"%d", TrustInfo.Date));
				m_pLsTrust->SetSubItemText(nPos, STH_ApplyID, str.Format(L"%d", TrustInfo.ApplyID));
				m_pLsTrust->SetSubItemText(nPos, STH_SecurityID, StrA2StrW(TrustInfo.SecurityID));
				m_pLsTrust->SetSubItemText(nPos, STH_SecurityName, StrA2StrW(TrustInfo.SecurityName));
				m_pLsTrust->SetSubItemText(nPos, STH_Direct,
					m_tradeDirectStrMap[TrustInfo.Direct], m_tradeDirectColorMap[TrustInfo.Direct]);
				for (auto &it : m_TrustInfoPosMap)
					++it.second;
				m_TrustInfoPosMap[TrustInfo.TrustID] = nPos;
			}
			int nPos = m_TrustInfoPosMap[TrustInfo.TrustID];
			m_pLsTrust->SetSubItemText(nPos, STH_TrustTime, str.Format(L"%02d:%02d:%02d",
				TrustInfo.Time / 10000, TrustInfo.Time / 100 % 100, TrustInfo.Time % 100));
			m_pLsTrust->SetSubItemText(nPos, STH_TrustState, m_trustStateStrMap[TrustInfo.State]);
			m_pLsTrust->SetSubItemText(nPos, STH_TrustPrice, str.Format(L"%.02f", TrustInfo.TrustPrice.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_TrustVol, str.Format(L"%.0f", TrustInfo.TrustVol.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_DealPrice, str.Format(L"%.02f", TrustInfo.DealPrice.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_DealVol, str.Format(L"%.0f", TrustInfo.DealVol.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_DealAmo, str.Format(L"%.03f", TrustInfo.DealAmo.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_CancelVol, str.Format(L"%.0f", TrustInfo.CancelVol.GetDouble()));
			m_pLsTrust->SetSubItemText(nPos, STH_CancelSign, m_cancelSignStrMap[TrustInfo.CancelSign]);
			m_pLsTrust->SetSubItemText(nPos, STH_TrustID, str.Format(L"%d", TrustInfo.TrustID));
			m_pLsTrust->SetSubItemText(nPos, STH_Account, str.Format(L"%d", TrustInfo.AccountID));
		}
	}
	m_pLsTrust->Invalidate();
	m_pTxtTrustCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsTrust->GetItemCount()));

}

void CTradeSimulator::UpdateCancelList()
{
	m_cancellableSet.clear();
	::EnterCriticalSection(&m_csTrust);
	auto TrustVec = m_TrustVec;
	::LeaveCriticalSection(&m_csTrust);

	for (int i = 0; i < TrustVec.size(); ++i)
	{
		auto& trustInfo = TrustVec[i];
		if (trustInfo.State == eTS_Submitted ||
			trustInfo.State == eTS_DealPart)
			m_cancellableSet.insert(trustInfo.ApplyID);
		else if (m_cancellableSet.count(trustInfo.ApplyID))
			m_cancellableSet.erase(trustInfo.ApplyID);
	}

	int nNowRow = 0;
	while (nNowRow < m_pLsCancel->GetItemCount())
	{
		int nApplyID = _wtoi(m_pLsCancel->GetSubItemText(nNowRow, SCH_ApplyID));
		if (m_cancellableSet.count(nApplyID) == 0)
		{
			m_pLsCancel->DeleteItem(nNowRow);
			m_CancelInfoPosMap.erase(nApplyID);
			for (auto &it : m_CancelInfoPosMap)
				if (it.second >= nNowRow)
					--it.second;
		}
		else
			++nNowRow;
	}
	SStringW str;
	for (int i = 0; i < TrustVec.size(); ++i)
	{
		auto& TrustInfo = TrustVec[i];
		if (m_cancellableSet.count(TrustInfo.ApplyID))
		{
			if (m_CancelInfoPosMap.count(TrustInfo.ApplyID) == 0)
			{

				int nPos = m_pLsCancel->InsertItem(0, str.Format(L"%02d:%02d:%02d",
					TrustInfo.Time / 10000, TrustInfo.Time / 100 % 100, TrustInfo.Time % 100));
				m_pLsCancel->SetSubItemText(nPos, SCH_ApplyID, str.Format(L"%d", TrustInfo.ApplyID));

				m_pLsCancel->SetSubItemText(nPos, SCH_SecurityID, StrA2StrW(TrustInfo.SecurityID));
				m_pLsCancel->SetSubItemText(nPos, SCH_SecurityName, StrA2StrW(TrustInfo.SecurityName));
				m_pLsCancel->SetSubItemText(nPos, SCH_Direct,
					m_tradeDirectStrMap[TrustInfo.Direct], m_tradeDirectColorMap[TrustInfo.Direct]);
				for (auto &it : m_CancelInfoPosMap)
					++it.second;
				m_CancelInfoPosMap[TrustInfo.ApplyID] = nPos;
			}
			int nPos = m_CancelInfoPosMap[TrustInfo.ApplyID];
			m_pLsCancel->SetSubItemText(nPos, SCH_TrustState, m_trustStateStrMap[TrustInfo.State]);
			m_pLsCancel->SetSubItemText(nPos, SCH_TrustPrice, str.Format(L"%.02f", TrustInfo.TrustPrice.GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_TrustVol, str.Format(L"%.0f", TrustInfo.TrustVol.GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_DealPrice, str.Format(L"%.02f", TrustInfo.DealPrice.GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_DealVol, str.Format(L"%.0f", TrustInfo.DealVol.GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_DealAmo, str.Format(L"%.03f", TrustInfo.DealAmo.GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_CancellableVol, str.Format(L"%.0f", (TrustInfo.TrustVol - TrustInfo.DealVol).GetDouble()));
			m_pLsCancel->SetSubItemText(nPos, SCH_CancelSign, m_cancelSignStrMap[TrustInfo.CancelSign]);
			m_pLsCancel->SetSubItemText(nPos, SCH_TrustID, str.Format(L"%d", TrustInfo.TrustID));
			m_pLsCancel->SetSubItemText(nPos, SCH_Account, str.Format(L"%d", TrustInfo.AccountID));
		}
	}
	m_pLsCancel->Invalidate();
	m_pTxtCancelCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsCancel->GetItemCount()));

}

void CTradeSimulator::UpdateDealList(SStringW str)
{
	SStringA strNew = StrW2StrA(str);
	m_pLsDeal->DeleteAllItems();
	m_strDealSearch = strNew;
	::EnterCriticalSection(&m_csDeal);
	auto DealVec = m_DealVec;
	::LeaveCriticalSection(&m_csDeal);

	for (int i = 0; i < DealVec.size(); ++i)
	{
		auto& DealInfo = DealVec[i];
		if (str.IsEmpty() || strstr(DealInfo.SecurityID, m_strDealSearch)
			|| strstr(DealInfo.SecurityName, m_strDealSearch))
		{
			int nPos = m_pLsDeal->InsertItem(0, str.Format(L"%02d:%02d:%02d",
				DealInfo.Time / 10000, DealInfo.Time / 100 % 100, DealInfo.Time % 100));
			m_pLsDeal->SetSubItemText(nPos, SDH_ApplyID, str.Format(L"%d", DealInfo.ApplyID));
			m_pLsDeal->SetSubItemText(nPos, SDH_SecurityID, StrA2StrW(DealInfo.SecurityID));
			m_pLsDeal->SetSubItemText(nPos, SDH_SecurityName, StrA2StrW(DealInfo.SecurityName));
			m_pLsDeal->SetSubItemText(nPos, SDH_Direct,
				m_tradeDirectStrMap[DealInfo.Direct], m_tradeDirectColorMap[DealInfo.Direct]);
			m_pLsDeal->SetSubItemText(nPos, SDH_DealPrice, str.Format(L"%.02f", DealInfo.DealPrice.GetDouble()));
			m_pLsDeal->SetSubItemText(nPos, SDH_DealVol, str.Format(L"%.0f", DealInfo.DealVol.GetDouble()));
			m_pLsDeal->SetSubItemText(nPos, SDH_DealAmo, str.Format(L"%.03f", DealInfo.DealAmo.GetDouble()));
			m_pLsDeal->SetSubItemText(nPos, SDH_TrustID, str.Format(L"%d", DealInfo.TrustID));
			m_pLsDeal->SetSubItemText(nPos, SDH_Account, str.Format(L"%d", DealInfo.AccountID));
		}
	}
	m_pLsDeal->Invalidate();
	m_pTxtDealCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsDeal->GetItemCount()));

}

void CTradeSimulator::UpdateDealSummary(SStringW str)
{
	SStringA strNew = StrW2StrA(str);
	if (m_strDealSumSearch != strNew)
	{
		m_pLsDealSum->DeleteAllItems();
		m_DealSumPosMap.clear();
		m_strDealSumSearch = strNew;
	}
	::EnterCriticalSection(&m_csDeal);
	auto DealSumMap = m_DealSumMap;
	::LeaveCriticalSection(&m_csDeal);

	for (auto &IDMap : DealSumMap)
	{
		SStringA SecurityID = IDMap.first;
		for (auto &DirectMap : IDMap.second)
		{
			int nDirect = DirectMap.first;
			auto &DealSum = DirectMap.second;
			if (str.IsEmpty() || strstr(DealSum.SecurityID, m_strDealSumSearch)
				|| strstr(DealSum.SecurityName, m_strDealSumSearch))
			{
				auto &dealPos = m_DealSumPosMap[DealSum.SecurityID];
				if (dealPos.count(DealSum.Direct) == 0)
				{
					int nPos = m_pLsDealSum->InsertItem(0, StrA2StrW(DealSum.SecurityID));
					m_pLsDealSum->SetSubItemText(nPos, SDSH_SecurityName, StrA2StrW(DealSum.SecurityName));
					m_pLsDealSum->SetSubItemText(nPos, SDSH_Direct,
						m_tradeDirectStrMap[DealSum.Direct], m_tradeDirectColorMap[DealSum.Direct]);
					for (auto &it : m_DealSumPosMap)
					{
						for (auto &posMap : it.second)
							posMap.second++;
					}
					m_DealSumPosMap[SecurityID][DealSum.Direct] = nPos;
				}
				int nPos = dealPos[DealSum.Direct];
				m_pLsDealSum->SetSubItemText(nPos, SDSH_DealVol, str.Format(L"%.0f", DealSum.DealVol.GetDouble()));
				m_pLsDealSum->SetSubItemText(nPos, SDSH_DealAvgPrice,
					str.Format(L"%.03f", DealSum.DealAmo.GetDouble() / DealSum.DealVol.GetDouble()));
				m_pLsDealSum->SetSubItemText(nPos, SDSH_DealAmo, str.Format(L"%.02f", DealSum.DealAmo.GetDouble()));
				m_pLsDealSum->SetSubItemText(nPos, SDSH_Account, str.Format(L"%d", DealSum.AccountID));
			}

		}
	}

	m_pLsDealSum->Invalidate();
	m_pTxtDealSumCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsDealSum->GetItemCount()));

}

void CTradeSimulator::UpdateHisTrustList(SStringW str)
{
	SStringA strNew = StrW2StrA(str);
	if (m_strHisTrustSearch != strNew)
	{
		m_pLsHisTrust->DeleteAllItems();
		m_HisTrustPosMap.clear();
		m_strHisTrustSearch = strNew;
	}
	for (int i = 0; i < m_HisTrustVec.size(); ++i)
	{
		auto& TrustInfo = m_HisTrustVec[i];
		if (TrustInfo.Date < m_nHisTrustStartDate ||
			TrustInfo.Date > m_nHisTrustEndDate)
			continue;
		if (str.IsEmpty() || strstr(TrustInfo.SecurityID, m_strHisTrustSearch)
			|| strstr(TrustInfo.SecurityName, m_strHisTrustSearch))
		{
			if (m_HisTrustPosMap.count(TrustInfo.ApplyID) == 0)
			{
				int nPos = m_pLsHisTrust->InsertItem(0, str.Format(L"%d", TrustInfo.Date));
				m_pLsHisTrust->SetSubItemText(nPos, SHTH_TrustTime, str.Format(L"%02d:%02d:%02d",
					TrustInfo.Time / 10000, TrustInfo.Time / 100 % 100, TrustInfo.Time % 100));
				m_pLsHisTrust->SetSubItemText(nPos, SHTH_ApplyID, str.Format(L"%d", TrustInfo.ApplyID));

				m_pLsHisTrust->SetSubItemText(nPos, SHTH_SecurityID, StrA2StrW(TrustInfo.SecurityID));
				m_pLsHisTrust->SetSubItemText(nPos, SHTH_SecurityName, StrA2StrW(TrustInfo.SecurityName));
				m_pLsHisTrust->SetSubItemText(nPos, SHTH_Direct,
					m_tradeDirectStrMap[TrustInfo.Direct], m_tradeDirectColorMap[TrustInfo.Direct]);
				for (auto &it : m_HisTrustPosMap)
					++it.second;
				m_HisTrustPosMap[TrustInfo.ApplyID] = nPos;
			}
			int nPos = m_HisTrustPosMap[TrustInfo.ApplyID];
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_TrustState, m_trustStateStrMap[TrustInfo.State]);
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_TrustPrice, str.Format(L"%.02f", TrustInfo.TrustPrice.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_TrustVol, str.Format(L"%.0f", TrustInfo.TrustVol.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_DealPrice, str.Format(L"%.02f", TrustInfo.DealPrice.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_DealVol, str.Format(L"%.0f", TrustInfo.DealVol.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_DealAmo, str.Format(L"%.03f", TrustInfo.DealAmo.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_CancelVol, str.Format(L"%.0f", TrustInfo.CancelVol.GetDouble()));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_CancelSign, m_cancelSignStrMap[TrustInfo.CancelSign]);
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_TrustID, str.Format(L"%d", TrustInfo.TrustID));
			m_pLsHisTrust->SetSubItemText(nPos, SHTH_Account, str.Format(L"%d", TrustInfo.AccountID));
		}
	}
	m_pLsHisTrust->Invalidate();
	m_pTxtHisTrustCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsHisTrust->GetItemCount()));
}

void CTradeSimulator::UpdateHisDealList(SStringW str)
{
	SStringA strNew = StrW2StrA(str);
	if (m_strHisDealSearch != strNew)
	{
		m_pLsHisDeal->DeleteAllItems();
		m_strHisDealSearch = strNew;
	}
	for (int i = 0; i < m_HisDealVec.size(); ++i)
	{
		auto& DealInfo = m_HisDealVec[i];
		if (DealInfo.Date < m_nHisDealStartDate ||
			DealInfo.Date > m_nHisDealEndDate)
			continue;
		if (str.IsEmpty() || strstr(DealInfo.SecurityID, m_strHisDealSearch)
			|| strstr(DealInfo.SecurityName, m_strHisDealSearch))
		{
			int nPos = m_pLsHisDeal->InsertItem(0, str.Format(L"%02d:%02d:%02d",
				DealInfo.Time / 10000, DealInfo.Time / 100 % 100, DealInfo.Time % 100));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_DealTime, str.Format(L"%02d:%02d:%02d",
				DealInfo.Time / 10000, DealInfo.Time / 100 % 100, DealInfo.Time % 100));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_ApplyID, str.Format(L"%d", DealInfo.ApplyID));

			m_pLsHisDeal->SetSubItemText(nPos, SHDH_SecurityID, StrA2StrW(DealInfo.SecurityID));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_SecurityName, StrA2StrW(DealInfo.SecurityName));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_Direct,
				m_tradeDirectStrMap[DealInfo.Direct], m_tradeDirectColorMap[DealInfo.Direct]);
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_DealPrice, str.Format(L"%.02f", DealInfo.DealPrice.GetDouble()));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_DealVol, str.Format(L"%.0f", DealInfo.DealVol.GetDouble()));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_DealAmo, str.Format(L"%.03f", DealInfo.DealAmo.GetDouble()));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_SettleAmp, str.Format(L"%.3f", DealInfo.SettleAmo.GetDouble()));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_TrustID, str.Format(L"%d", DealInfo.TrustID));
			m_pLsHisDeal->SetSubItemText(nPos, SHDH_Account, str.Format(L"%d", DealInfo.AccountID));
		}
	}
	m_pLsHisDeal->Invalidate();
	m_pTxtHisDealCount->SetWindowTextW(str.Format(L"共%d条记录", m_pLsHisDeal->GetItemCount()));

}

void CTradeSimulator::UpdateAccountInfo()
{
	SStringW str;
	::EnterCriticalSection(&m_csAccount);
	auto acc = m_accInfo;
	::LeaveCriticalSection(&m_csAccount);
	::EnterCriticalSection(&m_csPosition);
	auto posMap = m_PosInfoMap;
	::LeaveCriticalSection(&m_csPosition);
	acc.marketValue = TradeDouble(0, 2);
	for (auto &it : posMap)
	{
		auto& posInfo = it.second;
		double fCostPrice = 0;
		double fLastPrice = m_pListDataMap->count(Period_1Day) ?
			m_pListDataMap->at(Period_1Day).hash[posInfo.SecurityID].fPrice : 0;
		if (fLastPrice == 0)
			fLastPrice = m_preCloseMap.hash[posInfo.SecurityID];
		double fMarketValue = posInfo.TotalPos.GetDouble() * fLastPrice;
		acc.marketValue = acc.marketValue + TradeDouble(long long(fMarketValue * 100 + 0.5), 2);
	}
	acc.totalCaptical = acc.usableCaptical + acc.marketValue + acc.freezedCaptical;

	str.Format(L"%.02f", acc.totalCaptical.GetDouble());

	m_pTxtTotalCapital->SetWindowTextW(NumberWithSeparator(str.Format(L"%.02f", acc.totalCaptical.GetDouble())));
	m_pTxtMarketValue->SetWindowTextW(NumberWithSeparator(str.Format(L"%.02f", acc.marketValue.GetDouble())));
	double fProfit = (acc.marketValue - acc.marketCost).GetDouble();
	if (fProfit > 0)
	{
		m_pTxtProfit->SetAttribute(L"colorText", L"#FF0000FF");
		m_pTxtProfitChg->SetAttribute(L"colorText", L"#FF0000FF");
	}
	else if (fProfit < 0)
	{
		m_pTxtProfit->SetAttribute(L"colorText", L"#00FF00FF");
		m_pTxtProfitChg->SetAttribute(L"colorText", L"#00FF00FF");
	}
	else
	{
		m_pTxtProfit->SetAttribute(L"colorText", L"#FFFFFFFF");
		m_pTxtProfitChg->SetAttribute(L"colorText", L"#FFFFFFFF");
	}

	m_pTxtProfit->SetWindowTextW(NumberWithSeparator(str.Format(L"%.02f", fProfit)));

	double fProfitChg = fProfit / (acc.marketCost + acc.usableCaptical + acc.freezedCaptical).GetDouble() * 100;
	m_pTxtProfitChg->SetWindowTextW(NumberWithSeparator(str.Format(L"%.02f", fProfitChg)));
	m_pTxtAvaliableFunds->SetWindowTextW(NumberWithSeparator(str.Format(L"%.02f", acc.usableCaptical.GetDouble())));

}

void CTradeSimulator::SetMaxTradeVol(SEdit * pEdit)
{
	SStringW str = pEdit->GetWindowTextW();
	double fPrice = _wtof(str);
	if (pEdit == m_pEditBuyPrice)
	{
		if (fPrice != 0)
		{
			SStringA SecurityID = StrW2StrA(m_pEditBuyID->GetWindowTextW());

			m_llMaxBuy = m_accInfo.usableCaptical.GetDouble() / fPrice;
			if (SecurityID.Left(3) != "688")
			{
				m_llMaxBuy -= m_llMaxBuy % 100;
				if (SecurityID[0] == '6')
					m_llMaxBuy = min(m_llMaxBuy, llShMainMaxTrade);
				else if (SecurityID[0] == '0')
					m_llMaxBuy = min(m_llMaxBuy, llSzMainMaxTrade);
				else if (SecurityID[0] == '3')
					m_llMaxBuy = min(m_llMaxBuy, llSbmMaxTrade);
			}
			else
				m_llMaxBuy = min(m_llMaxBuy, llStramMaxTrade);
			m_llMaxBuy = min(m_llMaxBuy, m_setting.maxVol);
			str.Format(L"最大可买:%lld", m_llMaxBuy);
		}
		else
		{
			m_llMaxBuy = 0;
			str.Format(L"最大可买:--");
		}
		m_pTxtMaxBuy->SetWindowTextW(str);

	}
	else if (pEdit == m_pEditSellPrice)
	{
		SStringA SecurityID = StrW2StrA(m_pEditSellID->GetWindowTextW());
		m_llMaxSell = m_PosInfoMap.count(SecurityID) ? m_PosInfoMap[SecurityID].UsablePos.data : 0;
		if (SecurityID.Left(3) != "688")
		{
			if (SecurityID[0] == '6')
				m_llMaxSell = min(m_llMaxSell, llShMainMaxTrade);
			else if (SecurityID[0] == '0')
				m_llMaxSell = min(m_llMaxSell, llSzMainMaxTrade);
			else if (SecurityID[0] == '3')
				m_llMaxSell = min(m_llMaxSell, llSbmMaxTrade);
		}
		else
			m_llMaxSell = min(m_llMaxSell, llStramMaxTrade);
		m_llMaxSell = min(m_llMaxSell, m_setting.maxVol);

		str.Format(L"最大可卖:%lld", m_llMaxSell);
		m_pTxtMaxSell->SetWindowTextW(str);

	}
}


SStringW CTradeSimulator::NumToBig(SStringW strNum)
{
	SStringW strRes;
	vector<int> NumVec(MAX_NUM_LENGTH, 0);
	int nDiff = MAX_NUM_LENGTH - strNum.GetLength();
	for (int i = 0; i < strNum.GetLength(); ++i)
	{
		int nOffset = i + nDiff;
		NumVec[nOffset] = strNum[i] - '0';
	}
	bool bFindFirst = false;
	bool bPreIsZero = false;
	bool bUseWan = false;
	for (int i = 0; i < MAX_NUM_LENGTH; ++i)
	{
		if (!bFindFirst)
		{
			if (NumVec[i] == 0)
				continue;
			bFindFirst = true;
		}
		if (NumVec[i] == 0)
		{
			if (wcscmp(strNumOfDigital[i], L"亿") == 0)
			{
				bPreIsZero = false;
				strRes += strNumOfDigital[i];
			}
			else if (wcscmp(strNumOfDigital[i], L"万") == 0)
			{
				bPreIsZero = false;
				if (bUseWan)
					strRes += strNumOfDigital[i];
			}
			else
				bPreIsZero = true;
		}
		else
		{
			if (bPreIsZero)
			{
				strRes += strBigNum[0];
				bPreIsZero = false;
			}
			if (i >= 1 && i <= 4)
				bUseWan = true;

			strRes += strBigNum[NumVec[i]];
			strRes += strNumOfDigital[i];
		}
	}
	return strRes;
}

void CTradeSimulator::ClearData()
{
	m_PosInfoMap.clear();
	m_PosInfoPosMap.clear();
	m_pEditPosIDSearch->SetWindowTextW(L"");
	m_strPosSearch = "";
	m_pLsPosition->DeleteAllItems();

	m_TrustVec.clear();
	m_TrustInfoPosMap.clear();
	m_pEditTrustIDSearch->SetWindowTextW(L"");
	m_strTrustSearch = "";
	m_pLsTrust->DeleteAllItems();

	m_DealVec.clear();
	m_pEditDealIDSearch->SetWindowTextW(L"");
	m_strDealSearch = "";
	m_pLsDeal->DeleteAllItems();

	m_DealSumMap.clear();
	m_DealSumPosMap.clear();
	m_pEditDealSumSearch->SetWindowTextW(L"");
	m_strDealSumSearch = "";
	m_pLsDealSum->DeleteAllItems();

	m_cancellableSet.clear();
	m_CancelInfoPosMap.clear();
	m_pLsCancel->DeleteAllItems();


	m_HisTrustVec.clear();
	m_pEditHisTrustSearch->SetWindowTextW(L"");
	m_strHisTrustSearch = "";
	m_pLsHisTrust->DeleteAllItems();

	m_HisDealVec.clear();
	m_pEditHisDealSearch->SetWindowTextW(L"");
	m_strHisDealSearch = "";
	m_pLsHisDeal->DeleteAllItems();



	m_nHisTrustDataStrat = 0;
	m_nHisTrustDataEnd = 0;
	m_nHisDealDataStrat = 0;
	m_nHisDealDataEnd = 0;

	m_nHisTrustStartDate = 0;
	m_nHisTrustEndDate = 0;
	m_nHisDealStartDate = 0;
	m_nHisDealEndDate = 0;

}

