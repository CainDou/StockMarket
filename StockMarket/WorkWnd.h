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
		void		SetListInfo(vector<StockInfo>& infoVec,
			strHash<SStringA>& StockNameMap);
		void		SetPreClose(strHash<double> &preCloseMap);
		void		SetParThreadID(UINT uThreadID);
		void		CloseWnd();
		void		OutputStockFilterPara(SFPlan &sfPlan);
		void		OutputComboStockFilterPara(vector<StockFilter>& sfVec);
		void		InitStockFilterPara(SFPlan &sfPlan);
		void		InitComboStockFilterPara(vector<StockFilter>& sfVec);
		void		SetPointInfo(map<ePointDataType, ShowPointInfo> &infoMap);
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
		void SetListDataIsShow();
		void SetListDataOrder();
		void UpdateListShowStock();
		void UpdateList();
		void UpdateRpsData(int nRow, sRps &rps, int nStart, int nEnd);
		void UpdateSecData(int nRow, sSection &sec, int nStart, int nEnd);
		void UpdateTFData(int nRow, TickFlowMarket& tfData,double fPreClose);
		void UpdateListFilterShowStock();
		void SortList(SColorListCtrlEx* pList, bool bSortCode = false);

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

		//辅助函数
		BOOL GetAttPara(char * msg, map<SStringA, SStringA>& paraMap);
		void ProcHisRpsPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName1, SStringA dataName2,char* attchMsg,int attMsgSize);
		void ProcHisSecPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName, char* attchMsg, int attMsgSize);


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
		//列表相关数据
	protected:
		bool		m_bListInited;
		BOOL		m_bUseStockFilter;
		//vector<vector<SStringA>> m_SubPicShowNameVec;
		map< ePointType, map<SStringA, vector<SStringA>>>m_SubPicShowNameVec;
		strHash<int>m_ListPosMap;
		strHash<int>m_MouseWheelMap;
		map<int, strHash<RtRps>> *m_pListDataMap;
		map<int, strHash<TickFlowMarket>> *m_pTFMarketHash;
		map<int, strHash<map<string, double>>>* m_pFilterDataMap;
		strHash<double> m_preCloseMap;
		strHash<SStringA> m_StockName;
		vector<SStringA> m_dataNameVec;
		vector<SStringA> m_tfNameVec;
		map<int, BOOL> m_TitleShowMap;
		map<int, int> m_TitleOrderMap;
		SFPlan m_sfPlan;
		vector<StockFilter> m_sfVec;
		vector<BOOL> m_frmlExistVec;
		map<ePointDataType, ShowPointInfo> m_pointInfoMap;
		map<int, int> m_SFPeriodMap;
		map<int, string> m_SFIndexMap;
		map<int, PCOMPAREFUNC> m_SFConditionMap;


		int			m_nDate;
		BOOL		m_bListShowST;
		BOOL		m_bListShowSBM;
		BOOL		m_bListShowSTARM;
		BOOL		m_bListShowNewStock;
		map<SListHead, eSortDataType> m_ListDataSortMap;
		map<SListHead, int> m_ListDataDecMap;

		//分析图数据
	protected:
		map<int, map<SStringA, vector<CoreData>>> m_PointData;
		vector<CommonIndexMarket>m_IndexMarketVec;
		vector<CommonStockMarket>m_StockMarketVec;
		map<int, vector<KlineType>>m_KlineMap;

		bool m_bMarketGet;
		map<int, map<SStringA,BOOL>>m_PointGetMap;
		map<int, bool>m_KlineGetMap;
		//map<SStringA,map<int, bool>>m_PointGetMap;
		//set<ShowPointInfo> m_PointUseMap;
		map<ePointType, int> m_PointDataCount;
		eMaType m_MaParaSet;
	protected:
		unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
		vector<StockInfo> m_InfoVec;
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


inline void CWorkWnd::SetPreClose(strHash<double>& preCloseMap)
{
	m_preCloseMap = preCloseMap;
}

inline void SOUI::CWorkWnd::SetParThreadID(UINT uThreadID)
{
	m_uParWndThreadID = uThreadID;
}

