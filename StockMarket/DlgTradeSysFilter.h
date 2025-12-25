#pragma once
#include<thread>
#include <mutex>
typedef struct _TradeSysInfo
{
	SStringA strName;
	SStringA strSimpleName;
	SStringA strShowName;
	set<int> Period;
}TradeSysInfo;

typedef struct _ShowDataType
{
	SStringW strSecurityID;
	SStringW strName;
	double fLastPrice;
	double fChgPct;
	SStringW strSys;
	int nDir;
	int nPeriod;
	double fEnterPrice;
	int nEnterDate;
	double fExitPrice;
	int nExitDate;
	double fYeild;
	int nPosDay;
}ShowDataType;

namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgTradeSysFilter : public SHostWnd
	{
	public:
		CDlgTradeSysFilter();
		~CDlgTradeSysFilter();
		void	OnClose();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		BOOL	OnInitDialog(EventArgs* e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void	OnDestroy();
		//void	OnTimer(UINT_PTR nIDEvent);
		void	OnBtnClose();
		//void	OnLButtonUp(UINT nFlags, CPoint point);
		BOOL	WindowIsValid();
		void	UpdateRtShowDataVec();
		void	UpdateRtData();
		void	UpdateHisShowDataVec();
		void	UpdateHisData();
		void    SetMainMsgThreadID(unsigned nID);
		void	SetTradingDay(int nDate);
		void	SetPreCloseData(strHash<double>& PreCloseMap);
		void    SetStockInfo(strHash<StockInfo> &stockInfo);
		void	SetRehabInfo(map<SStringA, double>& accRehabMap);
		void	SetSelfSelStock(map<SStringA, SelfSelStockInfo>& selfSelStock);

	protected:
		void InitTradeSysName();
		void InitControls();
		void DataHandle();
		void UpdateListData(int nID,TradeSysRes& data);
		void UpdateListData(int nIndex,int nID, ShowDataType& data);
		int GetDaysBetween2Date(int nBegin, int nEnd);
		void ChangeResLsSize();
		ShowDataType ChangeResToShow(TradeSysRes& data);
		void OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);

	protected:
		bool OnChkAllPeriodClick(EventArgs* arg);
		bool OnChkPeriodClick(EventArgs* arg);
		bool OnChkAllDirClick(EventArgs* arg);
		bool OnChkDirClick(EventArgs* arg);
		bool OnChkAllCondClick(EventArgs* arg);
		bool OnChkCondClick(EventArgs* arg);
		bool OnDataTimeChanged(EventArgs* arg);
		bool OnListResHeaderClick(EventArgs * pEvtBase);
		bool OnListResMouseWheel(EventArgs * pEvtBase);
		bool OnListResRClick(EventArgs* arg);
		void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
		void OnRadioDateRt();
		void OnRadioDateHis();
		void OnBtnPageUp();
		void OnBtnPageDown();
		void OnBtnOk();
		bool OnListTradeSysSelChanged(EventArgs * e);
		void GetHisTradeSysRes(int nStartDate, int nEndDate,bool bClearData);
		void OnBtnAddAllStockToSelfSel();
		void SortShowData();
		template<typename T>
		static bool compareData(const ShowDataType& data1, const ShowDataType& data2, int nOffset, int nFlag);

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
			EVENT_NAME_COMMAND(L"rad_rt", OnRadioDateRt)
			EVENT_NAME_COMMAND(L"rad_his", OnRadioDateHis)
			EVENT_NAME_COMMAND(L"btn_pageUp", OnBtnPageUp)
			EVENT_NAME_COMMAND(L"btn_pageDown", OnBtnPageDown)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOk)
			EVENT_NAME_COMMAND(L"btn_addSelf", OnBtnAddAllStockToSelfSel)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgTradeSysFilter)
			MESSAGE_HANDLER(WM_TRADESYSRES_MSG, OnMsg)
			COMMAND_RANGE_HANDLER_EX(ASSM_Strat, ASSM_End, OnMenuCmd)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SColorListCtrlEx* m_pListRes;
		SColorListCtrlEx*  m_pListTradeSys;
		SCheckBox* m_pChkPeriodAll;
		SCheckBox* m_pChkPeriodDay;
		SCheckBox* m_pChkPeriod1Min;
		SCheckBox* m_pChkPeriod5Min;
		SCheckBox* m_pChkPeriod15Min;
		SCheckBox* m_pChkPeriod30Min;
		SCheckBox* m_pChkPeriod60Min;

		SCheckBox* m_pChkDirAll;
		SCheckBox* m_pChkDirLong;
		SCheckBox* m_pChkDirShort;
		
		SRadioBox* m_pRadioRt;
		SRadioBox* m_pRadioHis;

		SCheckBox* m_pChkCondAll;
		SCheckBox* m_pChkCondTdOpen;
		SCheckBox* m_pChkCondTdClose;
		SCheckBox* m_pChkCondNotClose;

		SStatic* m_pTxtStartDate;
		SDateTimePicker* m_pDtpStartDate;
		SStatic* m_pTxtEndDate;
		SDateTimePicker* m_pDtpEndDate;

		SStatic* m_pTxtTotalPage;
		SEdit* m_pEditNowPage;
		
	protected:
		strHash<StockInfo> m_StockInfo;
		strHash<double> m_PreCloseMap;
		map<SStringA, int> m_TradeSys;
		map<SStringA, double> m_accRehabMap;
		vector<vector<TradeSysRes>> m_TradeResVec;
		vector<vector<TradeSysRes>> m_HisTradeResVec;
		vector<TradeSysInfo> m_TradeSysInfoVec;
		map<SStringA, SelfSelStockInfo>m_SelfSelStockMap;
		set<int> m_RtShowPeriodSet;
		set<int> m_HisShowPeriodSet;
		set<int> m_RtShowDirSet;
		set<int> m_HisShowDirSet;
		map<SCheckBox*, int> m_ChkPeriodMap;
		int nTradingDay;
		int nHisRecvStartDate;
		int nHisRecvEndDate;
		int nTotalPage;
		int nNowPage;
		int nPrePageCount;
		vector<ShowDataType> m_ShowDataVec;
		int nSortCol;
		int nSortState;
	protected:
		std::thread m_thread;
		int m_uMsgThreadID;
		std::mutex m_mxRT;
		std::mutex m_mxHis;
		unsigned m_uParWndThreadID;
	protected:
		BOOL m_bIsValid;
		BOOL m_bLayoutInited;
		//BOOL m_bExit;
	};
	inline void SOUI::CDlgTradeSysFilter::SetMainMsgThreadID(unsigned nID)
	{
		m_uParWndThreadID = nID;
	}

	inline void CDlgTradeSysFilter::SetTradingDay(int nDate)
	{
		nTradingDay = nDate;
	}

	inline void CDlgTradeSysFilter::SetPreCloseData(strHash<double>& PreCloseMap)
	{
		m_PreCloseMap.hash.swap(PreCloseMap.hash);
	}

	inline void CDlgTradeSysFilter::SetStockInfo(strHash<StockInfo> &stockInfo)
	{
		m_StockInfo.hash.swap(stockInfo.hash);
	}

	inline void CDlgTradeSysFilter::SetRehabInfo(map<SStringA, double>& accRehabMap)
	{
		m_accRehabMap.swap(accRehabMap);
	}

	inline void CDlgTradeSysFilter::SetSelfSelStock(map<SStringA, SelfSelStockInfo>& selfSelStock)
	{
		m_SelfSelStockMap = selfSelStock;
	}

	inline BOOL CDlgTradeSysFilter::WindowIsValid()
	{
		return m_bIsValid;
	}


	template<typename T>
	inline bool CDlgTradeSysFilter::compareData(const ShowDataType & data1, const ShowDataType & data2, int nOffset, int nFlag)
	{
		T& cmpData1 = *((T*)((BYTE*)&data1 + nOffset));
		T& cmpData2 = *((T*)((BYTE*)&data2 + nOffset));
		if (cmpData1 != cmpData2)
		{
			if (nFlag == SD_Greater)
				return cmpData1 < cmpData2;
			else if (nFlag == SD_Less)
				return cmpData1 > cmpData2;
		}
		else if (data1.nDir != data2.nDir)
			return data1.nDir < data2.nDir;
		else if (data1.nPeriod != data2.nPeriod)
			return data1.nPeriod > data2.nPeriod;
		else if (data1.strSecurityID != data2.strSecurityID)
			return data1.strSecurityID < data2.strSecurityID;
		else
			return data1.strSys < data2.strSys;

		return false;
	}

}