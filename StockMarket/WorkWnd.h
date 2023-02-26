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
		InitPara	OutPutInitPara();
		void		ClearData();
		void		InitList();
		void		ReInitList();
		void		SetDataPoint(void* pData, int DataType);
		void		SetPicUnHandled();
		void		ReSetPic();
		void		UpdateTodayPointData(SStringA pointName,
			vector<CoreData> &dataVec, bool bLast);
		unsigned	GetThreadID() const;
		RpsGroup	GetGroup() const;
		void		SetListInfo(vector<StockInfo>& infoVec,
			strHash<SStringA>& StockNameMap);
		void		SetPreClose(strHash<double> &preCloseMap);
		void		SetParThreadID(UINT uThreadID);
		void		CloseWnd();
		void		OutputStockFilterPara(SFPlan &sfPlan);
		void		InitStockFilterPara(SFPlan &sfPlan);
		// 消息响应
	protected:
		void	OnInit(EventArgs *e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void	OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnRButtonUp(UINT nFlags, CPoint point);
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
		void UpdateListShowStock();
		void UpdateList();
		void UpdateListFilterShowStock();
		void SortList(SColorListCtrlEx* pList, bool bSortCode = false);

		bool CheckStockFitDomain(StockInfo& si);
		bool CheckStockDataPass(SFCondition& sf,SStringA StockID);
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
		void OnCheckST();
		void OnCheckSBM();
		void OnCheckSTARM();
		void OnCheckNewStock();

		//按钮辅助操作
		void SetBtnState(SImageButton* nowBtn, SImageButton** preBtn);
		void SetBtnState(SImageButton* nowBtn, bool bSelected);
		void SetBtnState(int nPeriod, bool bSelected);
		void OnBtnShowTypeChange(bool bFroceList = false);
		void OnBtnPeriedChange(int nPeriod);
		void SetListShowIndyStr(SStatic* pText);
		void SetFenShiShowData();
		void SetKlineShowData(int nPeriod, BOOL bNeedReCalc);
		void GetBelongingIndyName(vector<SStringA>& nameVec);

		//订阅功能
	public:
		void SetSelectedPeriod(int nPeriod);
		void ShowPicWithNewID(SStringA stockID,bool bForce = false);
		void SetDataFlagFalse();
		//数据更新处理
	protected:
		//外部消息处理
		void OnUpdateListData(int nMsgLength, const char* info);
		void OnUpdatePoint(int nMsgLength, const char* info);
		void OnUpdateTodayPoint(int nMsgLength, const char* info);
		void OnUpdateHisKline(int nMsgLength, const char* info);
		void OnUpdateHisPoint(int nMsgLength, const char* info);
		void OnUpdateIndexMarket(int nMsgLength, const char* info);
		void OnUpdateStockMarket(int nMsgLength, const char* info);
		void OnUpdateHisIndexMarket(int nMsgLength, const char* info);
		void OnUpdateHisStockMarket(int nMsgLength, const char* info);
		void OnUpdateCloseInfo(int nMsgLength, const char* info);
		void OnChangeShowIndy(int nMsgLength, const char* info);
		//内部消息处理
		void OnFenShiEma(int nMsgLength, const char* info);
		void OnFenShiMacd(int nMsgLength, const char* info);
		void OnKlineMa(int nMsgLength, const char* info);
		void OnKlineMacd(int nMsgLength, const char* info);
		void OnKlineBand(int nMsgLength, const char* info);
		void OnChangeStockFilter(int nMsgLength, const char* info);
		void OnSaveStockFilter(int nMsgLength, const char* info);


		void ProcHisPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName1, SStringA dataName2);


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
			EVENT_ID_COMMAND(R.id.chk_ST, OnCheckST)
			EVENT_ID_COMMAND(R.id.chk_SBM, OnCheckSBM)
			EVENT_ID_COMMAND(R.id.chk_STARM, OnCheckSTARM)
			EVENT_ID_COMMAND(R.id.chk_NewStock, OnCheckNewStock)

			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(CWorkWnd)
			MESSAGE_HANDLER(WM_WINDOW_MSG, OnMsg)
			MESSAGE_HANDLER(WM_FENSHI_MSG, OnFSMsg)
			MESSAGE_HANDLER(WM_KLINE_MSG, OnKlineMsg)
			COMMAND_RANGE_HANDLER_EX(FM_Return, FM_End, OnFSMenuCmd)
			COMMAND_RANGE_HANDLER_EX(KM_Return, KM_End, OnKlineMenuCmd)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_RBUTTONUP(OnRButtonUp)
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
		SStatic *m_pTextIndy;
		SStatic *m_pTextTitle;
		SColorListCtrlEx* m_pList;
		SFenShiPic* m_pFenShiPic;
		SKlinePic* m_pKlinePic;
		SImageButton* m_pPreSelBtn;
		CDlgKbElf* m_pDlgKbElf;
		CDlgStockFilter *m_pDlgStockFilter;
		SCheckBox*	  m_pCheckST;
		SCheckBox*	  m_pCheckSBM;
		SCheckBox*	  m_pCheckSTARM;
		SCheckBox*	  m_pCheckNewStock;

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
		vector<vector<SStringA>> m_SubPicShowNameVec;
		strHash<int>m_ListPosMap;
		strHash<int>m_MouseWheelMap;
		map<int, TimeLineMap> *m_pListDataMap;
		map<int, strHash<TickFlowMarket>> *m_pTFMarketHash;
		map<int, strHash<map<string, double>>>* m_pFilterDataMap;
		strHash<double> m_preCloseMap;
		strHash<SStringA> m_StockName;
		vector<SStringA> m_dataNameVec;
		vector<SStringA> m_tfNameVec;
		SFPlan m_sfPlan;
		vector<BOOL> m_frmlExistVec;
		//map<int, int> m_SFPeriodMap;
		//map<int, SStringA> m_SFIndexMap;
		//map<int, PCOMPAREFUNC> m_SFConditionMap;


		int			m_nDate;
		BOOL		m_bListShowST;
		BOOL		m_bListShowSBM;
		BOOL		m_bListShowSTARM;
		BOOL		m_bListShowNewStock;

		//分析图数据
	protected:
		map<int, map<SStringA, vector<CoreData>>> m_PointData;
		vector<CommonIndexMarket>m_IndexMarketVec;
		vector<CommonStockMarket>m_StockMarketVec;
		map<int, vector<KlineType>>m_KlineMap;

		bool m_bMarketGet;
		map<int, int>m_PointReadyMap;
		map<int, bool>m_KlineGetMap;
		map<int, bool>m_PointGetMap;
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


inline void CWorkWnd::SetPreClose(strHash<double>& preCloseMap)
{
	m_preCloseMap = preCloseMap;
}

inline void SOUI::CWorkWnd::SetParThreadID(UINT uThreadID)
{
	m_uParWndThreadID = uThreadID;
}

