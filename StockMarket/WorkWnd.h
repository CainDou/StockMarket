#pragma once
#include<thread>
using std::thread;
namespace SOUI
{
	class SColorListCtrlEx;
	class SFenShiPic;
	class SKlinePic;
	class CDlgKbElf;
	class CDlgStockFilter;
	class CDlgComboStockFilter;

	class CWorkWnd : public SHostWnd
	{
		typedef void(CWorkWnd::*PDATAHANDLEFUNC)(int, const char*);
		typedef bool(CWorkWnd::*PCOMPAREFUNC)(double, double);
	public:
		CWorkWnd();
		~CWorkWnd();
		//外部接口
		void		SetGroup(RpsGroup Group, HWND hParWnd);
		void		InitShowConfig(InitPara initPara);
		void		InitListConfig(map<int, BOOL>& titleShowMap,map<int,int>&titleOrderMap);
		InitPara	OutPutInitPara();
		void		ClearData();
		void		ClearAllData();
		void		InitList();
		void		ReInitList();
		void		SetDataPoint(void* pData, int DataType);
		void		SetPicUnHandled();
		void		ReSetPic();
		//void		UpdateTodayPointData(SStringA pointName,
		//	vector<CoreData> &dataVec, bool bLast);
		unsigned	GetThreadID() const;
		RpsGroup	GetGroup() const;
		map<int, BOOL> GetListShowTitle() const;
		void		SetFilterWndNum(int nWndNum);
		void		SetListInfo(vector<StockInfo>& infoVec,
			strHash<SStringA>& StockNameMap);
		void		SetPreClose(strHash<double> &preCloseMap);
		void		SetParThreadID(UINT uThreadID);
		void		CloseWnd();
		void		OutputStockFilterPara(SFPlan &sfPlan);
		void		OutputComboStockFilterPara(vector<StockFilter>& sfVec);
		void		OutputComboHisStockFilterPara(vector<HisStockFilter>& sfVec);
		void		InitStockFilterPara(SFPlan &sfPlan);
		void		InitComboStockFilterPara(vector<StockFilter>& sfVec);
		void		InitComboHisStockFilterPara(vector<HisStockFilter>& hsfVec);

		void		SetPointInfo(map<int, ShowPointInfo> &infoMap);
		map<int, int> GetListTitleOrder();
		// 消息响应
	protected:
		void	OnInit(EventArgs *e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void	OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnTarSelMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnRehabMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnRButtonUp(UINT nFlags, CPoint point);
		void	OnLButtonUp(UINT nFlags, CPoint point);
		BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		void	SwitchPic2List();
		void	SwitchList2Pic(int nPeriod);
		void	DataProc();
		void	InitProcFucMap();
		void	InitNameVec();
		void	InitStockFilterFunc();
		//列表响应
	public:
		bool OnListHeaderClick(EventArgs *pEvtBase);
		bool OnListHeaderSwap(EventArgs *pEvtBase);
		bool OnListDbClick(EventArgs *pEvtBase);
		bool OnListLClick(EventArgs *pEvtBase);

		//列表辅助函数
	public:
		void InitSortItemMapping();
		void SetListDataIsShow();
		void SetListDataOrder();
		void UpdateListShowStock();
		void HandleListData();
		void UpdateList();
		void SetPriceListHalf(bool bHalf);
		void UpdateListRpsData(int nRow, sRps &rps, int nStart, int nEnd);
		void UpdateListSecData(int nRow, sSection &sec, int nStart, int nEnd);
		void UpdateListCAData(int nRow, CAInfo& caData);
		void UpdateListTFData(int nRow, TickFlowMarket& tfData,double fPreClose);
		void UpdateListFilterShowStock();
		void SortList(SColorListCtrlEx* pList, bool bSortCode = false);
		void SortListData(bool bSortCode=false);
		void SortCommonData(int nSortHeader,int nFlag);
		void SortOtherData(int nSortHeader, int nFlag);

		template<typename T1, typename T2>
		bool compareData(const T1& data1, const T1& data2, int nOffset, int nFlag);
		template<typename T>
		bool compareData(const pair<SStringA,T>& data1, const pair<SStringA, T>& data2, int nFlag);


		template<typename T>
		void ResetListStockOrder(vector<T>& dataVec);
		template<typename T>
		void ResetListStockOrder(vector<pair<SStringA,T>>& dataVec);

		template<typename T>
		void SortData(strHash<T>& listDataHash, int nSortHeader, int nOffset, int nFlag);

		template<typename T>
		void SortData(map<SStringA,T>& listDataMap, int nSortHeader, int nFlag);
		

		bool CheckStockFitDomain(StockInfo& si);
		bool CheckStockDataPass(SFCondition& sf,SStringA StockID);
		bool CheckCmbStockDataPass(StockFilter& sf, SStringA StockID);


		bool GreaterThan(double a, double b);
		bool EqualOrGreaterThan(double a, double b);
		bool Equal(double a, double b);
		bool EqualOrLessThan(double a, double b);
		bool LessThan(double a, double b);

		int static __cdecl SortDouble(void *para1,
			const void* para2, const void*para3);
		int static __cdecl SortInt(void *para1,
			const void* para2, const void*para3);
		int static __cdecl SortStr(void *para1,
			const void* para2, const void*para3);
		int static __cdecl SortBigDouble(void *para1,
			const void* para2, const void*para3);

		//按钮响应
	public:
		void OnBtnMarketClicked();
		void OnBtnFenShiClicked();
		void OnBtnM1Clicked();
		void OnBtnM5Clicked();
		void OnBtnM15Clicked();
		void OnBtnM30Clicked();
		void OnBtnM60Clicked();
		void OnBtnDayClicked();
		void OnBtnListConnect1Clicked();
		void OnBtnListConnect2Clicked();
		void OnBtnStockFilterClicked();
		void OnBtnTitleSelectClicked();
		void OnCheckST();
		void OnCheckSBM();
		void OnCheckSTARM();
		void OnCheckNewStock();
		void OnBtnRehab();

		//按钮辅助操作
		void SetBtnState(SImageButton* nowBtn, SImageButton** preBtn);
		void SetBtnState(SImageButton* nowBtn, bool bSelected);
		void SetBtnState(int nPeriod, bool bSelected);
		void OnBtnShowTypeChange(bool bFroceList = false);
		void OnBtnPeriedChange(int nPeriod);
		void SetListShowIndyStr(SStatic* pText);
		void SetFenShiShowData(vector<ShowPointInfo>&infoVec,int nStartWnd = 0);
		void SetKlineShowData(vector<ShowPointInfo>&infoVec,int nPeriod, BOOL bNeedReCalc,int nStartWnd = 0);
		void GetBelongingIndyName(vector<SStringA>& nameVec);

		//订阅功能
	public:
		void SetSelectedPeriod(int nPeriod);
		void ShowPicWithNewID(SStringA stockID,bool bForce = false);
		void SetDataFlagFalse();
		void GetPointData(ShowPointInfo &info, SStringA StockID,int nPeriod);
		//数据更新处理
		void UpdateTmData(vector<CoreData>& comData, CoreData& data);
		bool CheckDataIsGot(ShowPointInfo & info, int nPeriod);
	protected:
		//外部消息处理
		void OnUpdateListData(int nMsgLength, const char* info);
		void OnUpdatePoint(int nMsgLength, const char* info);
		//void OnUpdateTodayPoint(int nMsgLength, const char* info);
		void OnUpdateHisKline(int nMsgLength, const char* info);
		void OnUpdateHisRpsPoint(int nMsgLength, const char* info);
		void OnUpdateIndexMarket(int nMsgLength, const char* info);
		void OnUpdateStockMarket(int nMsgLength, const char* info);
		void OnUpdateHisIndexMarket(int nMsgLength, const char* info);
		void OnUpdateHisStockMarket(int nMsgLength, const char* info);
		void OnUpdateCloseInfo(int nMsgLength, const char* info);
		void OnChangeShowIndy(int nMsgLength, const char* info);
		void OnUpdateHisSecPoint(int nMsgLength, const char* info);
		void OnUpdateRehabInfo(int nMsgLength, const char* info);
		void OnUpdateHisCallAction(int nMsgLength, const char* info);
		void OnUpdateHisTFBase(int nMsgLength, const char* info);
		void OnUpdateTodayTFMarket(int nMsgLength, const char* info);
		void OnUpdateRTTFMarket(int nMsgLength, const char* info);

		//内部消息处理
		void OnFenShiEma(int nMsgLength, const char* info);
		void OnFenShiMacd(int nMsgLength, const char* info);
		void OnKlineMa(int nMsgLength, const char* info);
		void OnKlineMacd(int nMsgLength, const char* info);
		void OnKlineBand(int nMsgLength, const char* info);
		void OnChangeStockFilter(int nMsgLength, const char* info);
		void OnSaveStockFilter(int nMsgLength, const char* info);
		void OnChangeKlineRehab(int nMsgLength, const char* info);
		void OnFixedTimeRehab(int nMsgLength, const char* info);
		void OnChangeHisStockFilter(int nMsgLength, const char* info);
		void OnHisFilterStartCalc(int nMsgLength, const char* info);
		void OnHisFilterEndCalc(int nMsgLength, const char* info);

		//辅助函数
		BOOL GetAttPara(char * msg, map<SStringA, SStringA>& paraMap);
		void ProcHisRpsPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName1, SStringA dataName2,char* attchMsg,int attMsgSize);
		void ProcHisSecPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName, char* attchMsg, int attMsgSize);
		void ProcHisCAPointFromCAInfo();

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_ID_COMMAND(R.id.btn_Market, OnBtnMarketClicked)
			EVENT_ID_COMMAND(R.id.btn_FS, OnBtnFenShiClicked)
			EVENT_ID_COMMAND(R.id.btn_M1, OnBtnM1Clicked)
			EVENT_ID_COMMAND(R.id.btn_M5, OnBtnM5Clicked)
			EVENT_ID_COMMAND(R.id.btn_M15, OnBtnM15Clicked)
			EVENT_ID_COMMAND(R.id.btn_M30, OnBtnM30Clicked)
			EVENT_ID_COMMAND(R.id.btn_M60, OnBtnM60Clicked)
			EVENT_ID_COMMAND(R.id.btn_Day, OnBtnDayClicked)
			EVENT_ID_COMMAND(R.id.btn_ListConnect1, OnBtnListConnect1Clicked)
			EVENT_ID_COMMAND(R.id.btn_ListConnect2, OnBtnListConnect2Clicked)
			EVENT_ID_COMMAND(R.id.btn_StockFilter, OnBtnStockFilterClicked)
			EVENT_ID_COMMAND(R.id.btn_TitleSel, OnBtnTitleSelectClicked)
			EVENT_ID_COMMAND(R.id.chk_ST, OnCheckST)
			EVENT_ID_COMMAND(R.id.chk_SBM, OnCheckSBM)
			EVENT_ID_COMMAND(R.id.chk_STARM, OnCheckSTARM)
			EVENT_ID_COMMAND(R.id.chk_NewStock, OnCheckNewStock)
			EVENT_ID_COMMAND(R.id.btn_Rehab, OnBtnRehab)

			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(CWorkWnd)
			MESSAGE_HANDLER(WM_WINDOW_MSG, OnMsg)
			MESSAGE_HANDLER(WM_FENSHI_MSG, OnFSMsg)
			MESSAGE_HANDLER(WM_KLINE_MSG, OnKlineMsg)
			COMMAND_RANGE_HANDLER_EX(FM_Return, FM_End, OnFSMenuCmd)
			COMMAND_RANGE_HANDLER_EX(KM_Return, KM_End, OnKlineMenuCmd)
			COMMAND_RANGE_HANDLER_EX(TSM_Close, TSM_End, OnTarSelMenuCmd)
			COMMAND_RANGE_HANDLER_EX(RM_NoRehab, RM_End, OnRehabMenuCmd)

			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_RBUTTONUP(OnRButtonUp)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()


		//子控件
	protected:
		SImageButton* m_pBtnMarket;
		map<int, SImageButton*> m_pPeriodBtnMap;
		SImageButton* m_pBtnConn1;
		SImageButton* m_pBtnConn2;
		SImageButton* m_pBtnStockFilter;
		SImageButton* m_pBtnTitleSel;
		SStatic *m_pTextFilterName;
		SStatic *m_pTextIndy;
		SStatic *m_pTextTitle;
		SColorListCtrlEx* m_pList;
		SFenShiPic* m_pFenShiPic;
		SKlinePic* m_pKlinePic;
		SImageButton* m_pPreSelBtn;
		CDlgKbElf* m_pDlgKbElf;
		CDlgStockFilter *m_pDlgStockFilter;
		CDlgComboStockFilter *m_pDlgCmbStockFilter;
		SCheckBox*	  m_pCheckST;
		SCheckBox*	  m_pCheckSBM;
		SCheckBox*	  m_pCheckSTARM;
		SCheckBox*	  m_pCheckNewStock;
		SImageButton* m_pBtnRehab;
		SStatic*	  m_pTextCalcInfo;

		//子类
	protected:
		SStringA m_strSubStock;
		//显示和计算参数
	protected:
		RpsGroup	m_Group;
		InitPara_t	m_InitPara;
		BOOL		m_bListConn1;
		BOOL		m_bListConn2;
		SStringA	m_ListShowInd;
		int			m_PicPeriod;
		int			m_ListPeriod;
		bool		m_bShowList;
		SortPara	m_SortPara;
		SStringA	m_WndName;
		int			m_nWndNum;
		BOOL		m_bFilterWnd;	//是否是用在选股上
		//列表相关数据
	protected:
		bool		m_bListInited;
		BOOL		m_bUseStockFilter;
		BOOL		m_bUseHisStockFilter;
		//vector<vector<SStringA>> m_SubPicShowNameVec;
		map< ePointType, map<SStringA, vector<SStringA>>>m_SubPicShowNameVec;
		map<int,SStringA>m_ListPosMap;
		map<int, SStringA>m_MouseWheelMap;
		map<int, strHash<RtRps>> *m_pListDataMap;
		map<int, strHash<TickFlowMarket>> *m_pTFMarketHash;
		map<int, vector<vector<double>>>* m_pFilterDataMap;
		map<int, vector<vector<double>>>* m_pL1IndyFilterDataMap;
		map<int, vector<vector<double>>>* m_pL2IndyFilterDataMap;
		map<int, vector<map<int, map<int, vector<double>>>>>* m_pHisFilterDataMap;
		map<int, vector<map<int, map<int, vector<double>>>>>* m_pL1IndyHisFilterDataMap;
		map<int, vector<map<int, map<int, vector<double>>>>>* m_pL2IndyHisFilterDataMap;
		strHash<CAInfo>* m_pCallActionHash;
		strHash<int>* m_pStockPos;
		strHash<int>* m_pL1IndyIndexPos;
		strHash<int>* m_pL2IndyIndexPos;

		strHash<double> m_preCloseMap;
		strHash<SStringA> m_StockName;
		vector<SStringA> m_dataNameVec;
		vector<SStringA> m_tfNameVec;
		map<int, BOOL> m_TitleShowMap;
		map<int, int> m_TitleOrderMap;
		SFPlan m_sfPlan;
		vector<StockFilter> m_sfVec;
		vector<HisStockFilter> m_hisSfVec;
		vector<BOOL> m_frmlExistVec;
		map<int, ShowPointInfo> m_pointInfoMap;
		map<int, int> m_SFPeriodMap;
		map<int, int> m_SFIndexMap;
		map<int, PCOMPAREFUNC> m_SFConditionMap;
		map<int, int>m_ComonSortMap;
		map<int,int>m_RpsSortMap;
		map<int, int>m_TFSortMap;
		map<int, int>m_CASortMap;

		int			m_nDate;
		BOOL		m_bListShowST;
		BOOL		m_bListShowSBM;
		BOOL		m_bListShowSTARM;
		BOOL		m_bListShowNewStock;
		map<SListHead, eSortDataType> m_ListDataSortMap;
		map<SListHead, int> m_ListDataDecMap;

		strHash<RtRps> m_ListShowRpsData;
		strHash<TickFlowMarket> m_ListShowTFData;
		strHash<CAInfo> m_ListShowCAData;
		set<int> m_ListItemUpdateSet;

		BOOL m_bHisFilterChecked;
		BOOL m_bHisFilterCalcing;

		vector<BOOL> m_StockPassHisVec;
		BOOL m_bHisFitlterDataReady;
		map<HisStockFilter, vector<BOOL>> m_SingleHsfRes;
		//分析图数据
	protected:
		map<int, map<SStringA, vector<CoreData>>> m_PointData;
		map<int, map<SStringA, vector<CoreData>>> m_L1IndyPointData;
		map<int, map<SStringA, vector<CoreData>>> m_L2IndyPointData;
		vector<CommonIndexMarket>m_IndexMarketVec;
		vector<CommonStockMarket>m_StockMarketVec;
		map<int, vector<KlineType>>m_KlineMap;
		map<int, vector<TFBaseMarket>>m_TFBaseMap;
		map<int, vector<TickFlowMarket>>m_RtTFMarketVec;
		vector<CAInfo>m_CallAction;

		bool m_bMarketGet;
		bool m_bCAInfoGet;
		map<int, map<SStringA,BOOL>>m_PointGetMap;
		map<int, map<SStringA, BOOL>>m_L1IndyPointGetMap;
		map<int, map<SStringA, BOOL>>m_L2IndyPointGetMap;
		map<int, bool>m_KlineGetMap;
		map<int, bool>m_TFBaseGetMap;
		//map<SStringA,map<int, bool>>m_PointGetMap;
		//set<ShowPointInfo> m_PointUseMap;
		map<ePointType, int> m_PointDataCount;
		eMaType m_MaParaSet;
	protected:
		unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
		vector<StockInfo> m_InfoVec;
		strHash<StockInfo>m_infoMap;
		UINT m_nLastChar;
		HWND m_hParWnd;
		unsigned m_uThreadID;
		thread m_workThread;
		unsigned m_uParWndThreadID;
		CRITICAL_SECTION m_csClose;
	};
}

inline unsigned CWorkWnd::GetThreadID() const
{
	return m_uThreadID;
}

inline RpsGroup CWorkWnd::GetGroup() const
{
	return m_Group;
}

inline map<int, BOOL> SOUI::CWorkWnd::GetListShowTitle() const
{
	return m_TitleShowMap;
}

inline void CWorkWnd::SetFilterWndNum(int nWndNum)
{
	m_nWndNum = nWndNum;
	m_bFilterWnd = TRUE;
}


inline void CWorkWnd::SetPreClose(strHash<double>& preCloseMap)
{
	m_preCloseMap = preCloseMap;
}

inline void CWorkWnd::SetParThreadID(UINT uThreadID)
{
	m_uParWndThreadID = uThreadID;
}

template<typename T1,typename T2>
bool CWorkWnd::compareData(const T1& data1, const T1& data2, int nOffset, int nFlag)
{
	T2 cmpData1= *((T2*)((BYTE*)&data1 + nOffset));
	T2 cmpData2 = *((T2*)((BYTE*)&data2 + nOffset));
	bool bData1Valid = !isinf<double>(cmpData1) && !isnan<double>(cmpData1);
	bool bData2Valid = !isinf<double>(cmpData2) && !isnan<double>(cmpData2);
	if (bData1Valid && bData2Valid)
	{
		if (cmpData1 != cmpData2)
		{
			if(nFlag == SD_Greater)
				return cmpData1 < cmpData2;
			else if(nFlag == SD_Less)
				return cmpData1 > cmpData2;
		}
		else
			return strcmp(data1.SecurityID, data2.SecurityID) < 0;

	}
	else
	{
		if (bData1Valid)
			return true;
		else if (bData2Valid)
			return false;
		return strcmp(data1.SecurityID, data2.SecurityID) < 0;
	}
	return false;

}

template<typename T>
bool CWorkWnd::compareData(const pair<SStringA, T>& data1, const pair<SStringA, T>& data2, int nFlag)
{
	T cmpData1 = data1.second;
	T cmpData2 = data2.second;
	bool bData1Valid = !isinf<double>(cmpData1) && !isnan<double>(cmpData1);
	bool bData2Valid = !isinf<double>(cmpData2) && !isnan<double>(cmpData2);
	if (bData1Valid && bData2Valid)
	{
		if (cmpData1 != cmpData2)
		{
			if (nFlag == SD_Greater)
				return cmpData1 < cmpData2;
			else if (nFlag == SD_Less)
				return cmpData1 > cmpData2;
		}
		else
			return data1.first < data2.first;

	}
	else
	{
		if (bData1Valid)
			return true;
		else if (bData2Valid)
			return false;
		return data1.first < data2.first < 0;
	}
	return false;
}

template<typename T>
void CWorkWnd::ResetListStockOrder(vector<T>& dataVec)
{
	set<SStringA> showStockSet;
	for (auto &it : m_ListPosMap)
		showStockSet.insert(it.second);
	for (int i = 0; i < dataVec.size(); ++i)
	{
		auto &stockInfo = m_infoMap.hash[dataVec[i].SecurityID];
		m_pList->SetSubItemText(i, SHead_ID, StrA2StrW(stockInfo.SecurityID));
		m_pList->SetSubItemText(i, SHead_Name, StrA2StrW(stockInfo.SecurityName));
		m_ListPosMap[i] = stockInfo.SecurityID;
		showStockSet.erase(stockInfo.SecurityID);
	}
	int nCount = dataVec.size();
	for (auto &it : showStockSet)
	{
		auto &stockInfo = m_infoMap.hash[it];
		m_pList->SetSubItemText(nCount, SHead_ID, StrA2StrW(stockInfo.SecurityID));
		m_pList->SetSubItemText(nCount, SHead_Name, StrA2StrW(stockInfo.SecurityName));
		m_ListPosMap[nCount++] = stockInfo.SecurityID;
	}

}

template<typename T>
void CWorkWnd::ResetListStockOrder(vector<pair<SStringA,T>>& dataVec)
{
	set<SStringA> showStockSet;
	for (auto &it : m_ListPosMap)
		showStockSet.insert(it.second);
	for (int i = 0; i < dataVec.size(); ++i)
	{
		auto &stockInfo = m_infoMap.hash[dataVec[i].first];
		m_pList->SetSubItemText(i, SHead_ID, StrA2StrW(stockInfo.SecurityID));
		m_pList->SetSubItemText(i, SHead_Name, StrA2StrW(stockInfo.SecurityName));
		m_ListPosMap[i] = stockInfo.SecurityID;
		showStockSet.erase(stockInfo.SecurityID);
	}
	int nCount = dataVec.size();
	for (auto &it : showStockSet)
	{
		auto &stockInfo = m_infoMap.hash[it];
		m_pList->SetSubItemText(nCount, SHead_ID, StrA2StrW(stockInfo.SecurityID));
		m_pList->SetSubItemText(nCount, SHead_Name, StrA2StrW(stockInfo.SecurityName));
		m_ListPosMap[nCount++] = stockInfo.SecurityID;
	}
}

template<typename T>
void CWorkWnd::SortData(strHash<T>& listDataHash, int nSortHeader, int nOffset, int nFlag)
{
	vector<T>dataVec;
	dataVec.reserve(listDataHash.hash.size());
	for (auto &it : listDataHash.hash)
		dataVec.emplace_back(it.second);

	sort(dataVec.begin(), dataVec.end(),
		[&](const T & data1, const T& data2)
	{
		int nSortType = m_ListDataSortMap[(SListHead)nSortHeader];
		if (eSDT_Int == nSortType)
			return compareData<T, int>(data1, data2, nOffset, nFlag);
		else if (eSDT_Double == nSortType || eSDT_BigDouble == nSortType)
			return compareData<T, double>(data1, data2, nOffset, nFlag);
		else if(eSDT_Uint64 == nSortType)
			return compareData<T, uint64_t>(data1, data2, nOffset, nFlag);

		return false;
	});

	ResetListStockOrder(dataVec);

}

template<typename T>
void CWorkWnd::SortData(map<SStringA, T>& listDataMap, int nSortHeader,  int nFlag)
{
	vector<pair<SStringA, T>>dataVec(listDataMap.begin(), listDataMap.end());
	sort(dataVec.begin(), dataVec.end(),
		[&](const pair<SStringA,T> & data1, const pair<SStringA, T>& data2)
	{
		return compareData(data1, data2, nFlag);
	});
	ResetListStockOrder(dataVec);

}
