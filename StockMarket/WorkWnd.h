#pragma once
#include<thread>
using std::thread;
namespace SOUI
{
	class SColorListCtrlEx;
	class SFenShiPic;
	class SKlinePic;
	class CDlgKbElf;
	class CWorkWnd : public SHostWnd
	{
		typedef void(CWorkWnd::*PDATAHANDLEFUNC)(int, const char*);
	public:
		CWorkWnd();
		~CWorkWnd();
		//外部接口
		void		SetGroup(RpsGroup Group,bool bMain,HWND hParWnd);
		void		InitShowConfig(InitPara initPara);
		InitPara	OutPutInitPara();
		void		ClearData();
		void		InitList();
		void		ReInitList();
		void		SetDataPoint(void* pData,int DataType);
		void		SetKbElfInfo(bool bFroceUpdate,
			vector<StockInfo>* stock1Vec, vector<StockInfo>* stock2Vec = nullptr);
		void		SetPicUnHandled();
		void		ReSetPic();
		void		UpdateTodayPointData(SStringA pointName, 
					vector<CoreData> &dataVec,bool bLast);
		unsigned	GetThreadID() const;
		RpsGroup	GetGroup() const;

		// 消息响应
	protected:
		void	OnInit(EventArgs *e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT	OnDataMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void	OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		void    OnLButtonDown(UINT nFlags, CPoint point);
		void	OnRButtonUp(UINT nFlags, CPoint point);
		BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		void	SwitchPic2List();
		void	SwitchList2Pic(int nPeriod);
		void	ChangeWindowSetting(SStringA strKey,
			void* pValue, bool bComplex = false);
		void	OnTimer(UINT_PTR nIDEvent);
		void	DataProc();
		void	InitProcFucMap();
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
		void SortList(SColorListCtrlEx* pList, bool bSortCode = false);
		int static __cdecl SortDouble(void *para1,
			const void* para2, const void*para3);
		int static __cdecl SortInt(void *para1,
			const void* para2, const void*para3);
		int static __cdecl SortStr(void *para1, 
			const void* para2, const void*para3);
		void SaveListConfig();

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
		void ShowSubStockPic(SStringA stockID);
		//数据更新处理
	protected:
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
		void ProcHisPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
			SStringA dataName1, SStringA dataName2);


	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_Market", OnBtnMarketClicked)
			EVENT_NAME_COMMAND(L"btn_FS", OnBtnFenShiClicked)
			EVENT_NAME_COMMAND(L"btn_M1", OnBtnM1Clicked)
			EVENT_NAME_COMMAND(L"btn_M5", OnBtnM5Clicked)
			EVENT_NAME_COMMAND(L"btn_M15", OnBtnM15Clicked)
			EVENT_NAME_COMMAND(L"btn_M30", OnBtnM30Clicked)
			EVENT_NAME_COMMAND(L"btn_M60", OnBtnM60Clicked)
			EVENT_NAME_COMMAND(L"btn_Day", OnBtnDayClicked)
			EVENT_NAME_COMMAND(L"btn_ListConnect1", OnBtnListConnect1Clicked)
			EVENT_NAME_COMMAND(L"btn_ListConnect2", OnBtnListConnect2Clicked)
		EVENT_MAP_END()

		BEGIN_MSG_MAP_EX(CWorkWnd)
			MESSAGE_HANDLER(WM_MAIN_MSG, OnMsg)
			MESSAGE_HANDLER(WM_DATA_MSG, OnDataMsg)
			MESSAGE_HANDLER(WM_FENSHI_MSG, OnFSMsg)
			MESSAGE_HANDLER(WM_KLINE_MSG, OnKlineMsg)
			COMMAND_RANGE_HANDLER_EX(FM_Return, FM_End, OnFSMenuCmd)
			COMMAND_RANGE_HANDLER_EX(KM_Return, KM_End, OnKlineMenuCmd)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_RBUTTONUP(OnRButtonUp)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

		//子类
	protected:
		SColorListCtrlEx* m_pList;
		SFenShiPic* m_pFenShiPic;
		SKlinePic* m_pKlinePic;
		SStringA m_strSubStock;
		SImageButton* m_pPreSelBtn;
		SStatic *m_pTxtIndy;
		CDlgKbElf* m_pDlgKbElf;
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
		bool		m_bInMain;
		SortPara	m_SortPara;
		SStringA	m_WndName;
		//列表相关数据
	protected:
		bool		m_bListInited;
		unordered_map<SStringA, StockInfo, hash_SStringA>	m_ListStockInfoMap;
		vector<vector<SStringA>> m_SubPicShowNameVec;
		vector<SStringA> m_SubPicWndNameVec;
		vector<SStringA>	m_ListInsVec;
		unordered_map<SStringA, int, hash_SStringA>	m_ListPosMap;
		map<int,TimeLineMap> *m_pListDataMap;
		unordered_map<SStringA, double, hash_SStringA> m_preCloseMap;
		unordered_map<SStringA, SStringA, hash_SStringA> m_StockNameMap;
		vector<SStringA> m_dataNameVec;
		//分析图数据
	protected:
		map<int, map<SStringA,vector<CoreData>>> m_PointData;
		vector<CommonIndexMarket>m_IndexMarketVec;
		vector<CommonStockMarket>m_StockMarketVec;
		map<int, vector<KlineType>>m_KlineMap;

		bool m_bMarketGet;
		map<int, bool>m_PointReadyMap;
		map<int, bool>m_KlineGetMap;
		map<int, bool>m_PointGetMap;
	protected:
		unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
		vector<StockInfo> m_StockVec;
		UINT m_nLastChar;
		HWND m_hParWnd;
		unsigned m_uThreadID;
		thread m_workThread;
	};
}

inline unsigned SOUI::CWorkWnd::GetThreadID() const
{
	return m_uThreadID;
}

inline RpsGroup SOUI::CWorkWnd::GetGroup() const
{
	return m_Group;
}
