#pragma once
#include<thread>
#include <fstream>
namespace SOUI
{
	class SColorListCtrlEx;
	class STradeInfoPic;
	class CTradeSimulator :public SHostWnd
	{
		typedef void(CTradeSimulator::*PDATAHANDLEFUNC)(int, const char*);

	public:
		CTradeSimulator();
		~CTradeSimulator();
		BOOL	OnInitDialog(EventArgs* e);
		void	InitControls();
		void	InitStrings();
		void	InitColors();
		void	InitDatas();
		void	InitDataHandleMap();
		void	OnBtnClose();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnBtnBuyAll();
		void	OnBtnBuyHalf();
		void	OnBtnBuyThird();
		void	OnBtnBuyQuarter();
		void	OnBtnBuy();
		void	OnBtnSellAll();
		void	OnBtnSellHalf();
		void	OnBtnSellThird();
		void	OnBtnSellQuarter();
		void	OnBtnSell();
		void	OnBtnBuyCon();
		void	OnBtnBuyUnCon();
		void	OnBtnSellCon();
		void	OnBtnSellUnCon();
		void	OnBtnRefreshAcc();
		void	OnBtnPosIDSearch();
		void	OnBtnPosDownload();
		void	OnBtnTrustIDSearch();
		void	OnBtnTrustDownload();
		void	OnBtnDealIDSearch();
		void	OnBtnDealDownload();
		void	OnBtnDealSumSearch();
		void	OnBtnDealSumDownload();
		void	OnChkCancelAll();
		void	OnBtnCancelMulti();
		void	OnBtnCancelAll();
		void	OnBtnCancekDownload();
		void	OnBtnHisTrustSearch();
		void	OnBtnHisTrustDownload();
		void	OnBtnHisDealSearch();
		void	OnBtnHisDealDownload();
		bool	OnListLDoubleClicked(EventArgs* e);
		bool	OnListCancelLBClicked(EventArgs* e);
		bool	OnEditSearchChange(EventArgs *e);
		bool	OnEditTradeIDChange(EventArgs *e);
		bool	OnEditVolChange(EventArgs *e);
		bool	OnEditPriceChange(EventArgs *e);
		bool	OnLbIDLButtonDown(EventArgs *e);
		void	SetTradeStock(SStringA strStock);
		void	UpdateTradeInfo(BOOL bFirst);
		void	UpdateSubmitFeedback(SubmitFeedback sfb);
		void	UpdateDealInfo(int nCount);
		void	SaveTradeSetting();
		void	GetTradeSetting();
		BOOL	IsLogin();
		void	LogOut();
		void	OnSpinBuyPrice();
		void	OnSpinSellPrice();
		void	OnBtnTradeSetting();
		void	OnTab();
	protected:
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
		void OnMouseMove(UINT nFlags, CPoint pt);
		void OnSize(UINT nType, CSize size);
		void OnTimer(UINT_PTR nID);

	protected:
		void DataMsgProc();
		void OnUpdateHisStockMarket(int nMsgLength, const char* info);
		void OnUpdateStockMarket(int nMsgLength, const char* info);
		void OnUpdateCloseInfo(int nMsgLength, const char* info);
		void OnTradeLogin(int nMsgLength, const char* info);
		void OnTradeLogout(int nMsgLength, const char* info);
		void OnAccountInfo(int nMsgLength, const char* info);
		void OnPosition(int nMsgLength, const char* info);
		void OnTrust(int nMsgLength, const char* info);
		void OnDeal(int nMsgLength, const char* info);
		void OnHisTrust(int nMsgLength, const char* info);
		void OnHisDeal(int nMsgLength, const char* info);
		void OnSubmitFeedback(int nMsgLength, const char* info);
		void SaveListData(SColorListCtrlEx* pList,std::ofstream &ofile);
		SStringW NumberWithSeparator(SStringW str);
		bool CheckPriceIsLeagal(int nDirect, long long llPrice);
	protected:
		void SetEditVol(SEdit* pEdit, int nDivisor);
		void UpdateListData(SColorListCtrlEx* pList, SStringW str);
		void UpdatePositionList(SStringW str);
		void UpdateTrustList(SStringW str);
		void UpdateCancelList();
		void UpdateDealList(SStringW str);
		void UpdateDealSummary(SStringW str);
		void UpdateHisTrustList(SStringW str);
		void UpdateHisDealList(SStringW str);
		void UpdateAccountInfo();
		void SetMaxTradeVol(SEdit* pEdit);
		SStringW NumToBig(SStringW strNum);
		void ClearData();
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
			EVENT_NAME_COMMAND(L"btn_buyCon", OnBtnBuyCon)
			EVENT_NAME_COMMAND(L"btn_buyUnCon", OnBtnBuyUnCon)
			EVENT_NAME_COMMAND(L"btn_buyAll", OnBtnBuyAll)
			EVENT_NAME_COMMAND(L"btn_buyHalf", OnBtnBuyHalf)
			EVENT_NAME_COMMAND(L"btn_buyThird", OnBtnBuyThird)
			EVENT_NAME_COMMAND(L"btn_buyQuarter", OnBtnBuyQuarter)
			EVENT_NAME_COMMAND(L"btn_buy", OnBtnBuy)
			EVENT_NAME_COMMAND(L"btn_sellCon", OnBtnSellCon)
			EVENT_NAME_COMMAND(L"btn_sellUnCon", OnBtnSellUnCon)
			EVENT_NAME_COMMAND(L"btn_sellAll", OnBtnSellAll)
			EVENT_NAME_COMMAND(L"btn_sellHalf", OnBtnSellHalf)
			EVENT_NAME_COMMAND(L"btn_sellThird", OnBtnSellThird)
			EVENT_NAME_COMMAND(L"btn_sellQuarter", OnBtnSellQuarter)
			EVENT_NAME_COMMAND(L"btn_sell", OnBtnSell)
			EVENT_NAME_COMMAND(L"btn_refreshAcc", OnBtnRefreshAcc)
			EVENT_NAME_COMMAND(L"btn_posIDSearch", OnBtnPosIDSearch)
			EVENT_NAME_COMMAND(L"btn_posDownload", OnBtnPosDownload)
			EVENT_NAME_COMMAND(L"btn_trustIDSearch", OnBtnTrustIDSearch)
			EVENT_NAME_COMMAND(L"btn_trustDownload", OnBtnTrustDownload)
			EVENT_NAME_COMMAND(L"btn_cancelMultiple", OnBtnCancelMulti)
			EVENT_NAME_COMMAND(L"btn_cancelAll", OnBtnCancelAll)
			EVENT_NAME_COMMAND(L"btn_cancelDownload", OnBtnCancekDownload)
			EVENT_NAME_COMMAND(L"chk_cancelAll", OnChkCancelAll)
			EVENT_NAME_COMMAND(L"btn_dealIDSearch", OnBtnDealIDSearch)
			EVENT_NAME_COMMAND(L"btn_dealDownload", OnBtnDealDownload)
			EVENT_NAME_COMMAND(L"btn_dealSumSearch", OnBtnDealSumSearch)
			EVENT_NAME_COMMAND(L"btn_dealSumDownload", OnBtnDealSumDownload)
			EVENT_NAME_COMMAND(L"btn_hisTrustIDSearch", OnBtnHisTrustSearch)
			EVENT_NAME_COMMAND(L"btn_hisTrustDownload", OnBtnHisTrustDownload)
			EVENT_NAME_COMMAND(L"btn_hisDealIDSearch", OnBtnHisDealSearch)
			EVENT_NAME_COMMAND(L"btn_hisDealDownload", OnBtnHisDealDownload)
			EVENT_NAME_COMMAND(L"spin_buyPrice", OnSpinBuyPrice)
			EVENT_NAME_COMMAND(L"spin_sellPrice", OnSpinSellPrice)
			EVENT_NAME_COMMAND(L"btn_buySetting", OnBtnTradeSetting)
			EVENT_NAME_COMMAND(L"btn_sellSetting", OnBtnTradeSetting)
			EVENT_NAME_COMMAND(L"tab_ctrl", OnTab)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CTradeSimulator)
			MESSAGE_HANDLER(WM_TRADE_MSG, OnMsg)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_SIZE(OnSize)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	protected:

		STabCtrl* m_pTabTrade;

		SEdit* m_pEditBuyID;
		SEdit* m_pEditBuyPrice;
		SEdit* m_pEditBuyVol;
		SEdit* m_pEditSellID;
		SEdit* m_pEditSellPrice;
		SEdit* m_pEditSellVol;
		SStatic* m_pTxtMaxBuy;
		SStatic* m_pTxtMaxSell;
		SListBox* m_pLbBuyID;
		SListBox* m_pLbSellID;
		SStatic* m_pTxtBuyVolInBig;
		SStatic* m_pTxtSellVolInBig;

		SStatic* m_pTxtTotalCapital;
		SStatic* m_pTxtMarketValue;
		SStatic* m_pTxtProfit;
		SStatic* m_pTxtProfitChg;
		SStatic* m_pTxtAvaliableFunds;

		STradeInfoPic* m_pTradeInfo;

		SColorListCtrlEx* m_pLsPosition;
		SEdit* m_pEditPosIDSearch;
		SStatic* m_pTxtPosCount;

		SStatic* m_pTxtTrustCount;
		SEdit* m_pEditTrustIDSearch;
		SColorListCtrlEx* m_pLsTrust;

		SStatic* m_pTxtCancelCount;
		SColorListCtrlEx* m_pLsCancel;
		SCheckBox* m_pChkSelectAll;

		SStatic* m_pTxtDealCount;
		SEdit* m_pEditDealIDSearch;
		SColorListCtrlEx* m_pLsDeal;

		SStatic* m_pTxtDealSumCount;
		SEdit* m_pEditDealSumSearch;
		SColorListCtrlEx* m_pLsDealSum;


		SDateTimePicker* m_pDtpHisTrustStart;
		SDateTimePicker* m_pDtpHisTrustEnd;
		SEdit* m_pEditHisTrustSearch;
		SStatic* m_pTxtHisTrustCount;
		SColorListCtrlEx* m_pLsHisTrust;

		SDateTimePicker* m_pDtpHisDealStart;
		SDateTimePicker* m_pDtpHisDealEnd;
		SEdit* m_pEditHisDealSearch;
		SStatic* m_pTxtHisDealCount;
		SColorListCtrlEx* m_pLsHisDeal;

		map<SEdit*, SColorListCtrlEx*>m_editListMap;
		map<SEdit*, SListBox*>m_IDListMap;
		map<SEdit*, SStatic*>m_volTextMap;
		map<SColorListCtrlEx*, int>m_StockIdInListMap;

		//市场行情数据
	protected:
		SStringA m_strStock;
		SStringA m_strStockName;
		vector<CommonStockMarket> m_marketVec;
		vector<StockInfo> m_stockVec;
		strHash<StockInfo> m_stockHash;
		map<int, strHash<RtRps>> *m_pListDataMap;
		strHash<double> m_preCloseMap;
		//显示对照数据
	protected:
		map<int, SStringW>m_tradeDirectStrMap;
		map<int, COLORREF>m_tradeDirectColorMap;
		map<int, SStringW>m_trustStateStrMap;
		map<int, SStringW>m_cancelSignStrMap;
		TradeSetting m_setting;
		//交易数据
	protected:
		AccInfo m_accInfo;
		map<SStringA,PositionInfo> m_PosInfoMap;
		vector<TrustInfo> m_TrustVec;
		vector<DealInfo> m_DealVec;
		map<SStringA, map<int, DealInfo>> m_DealSumMap;
		vector<TrustInfo> m_HisTrustVec;
		vector<DealInfo> m_HisDealVec;

		SStringA m_strPosSearch;
		SStringA m_strTrustSearch;
		SStringA m_strDealSearch;
		SStringA m_strDealSumSearch;
		SStringA m_strHisTrustSearch;
		SStringA m_strHisDealSearch;

		map<SStringA, int>m_PosInfoPosMap;
		map<int, int>m_TrustInfoPosMap;
		map<SStringA, map<int, int>>m_DealSumPosMap;
		map<int, int>m_CancelInfoPosMap;
		map<int, int>m_HisTrustPosMap;

		set<int>m_cancellableSet;

		int m_nHisTrustDataStrat;
		int m_nHisTrustDataEnd;
		int m_nHisDealDataStrat;
		int m_nHisDealDataEnd;

		int m_nHisTrustStartDate;
		int m_nHisTrustEndDate;
		int m_nHisDealStartDate;
		int m_nHisDealEndDate;

		long long m_llMaxBuy;
		long long m_llMaxSell;
	protected:
		BOOL m_bLogin;
		UINT m_SynThreadID;
		UINT m_tradeSynThreadID;
		BOOL m_bDataInited;
		CRITICAL_SECTION m_csClose;
		CRITICAL_SECTION m_csTrust;
		CRITICAL_SECTION m_csDeal;
		CRITICAL_SECTION m_csPosition;
		CRITICAL_SECTION m_csAccount;
		CRITICAL_SECTION m_csMarket;

		unordered_map<int, PDATAHANDLEFUNC>m_DataHandleMap;
		std::thread tDataProc;
		UINT m_DataThreadID;
	protected:
		BOOL m_bLayoutInited;
		BOOL m_bSetFirst;
		CPoint m_ptMouse;

	};
	inline BOOL CTradeSimulator::IsLogin()
	{
		return m_bLogin;
	}


}


