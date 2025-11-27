#pragma once
#include <thread>
namespace SOUI
{
	class SKlinePic;
	class CDlgKbElf;
	class CDlgMultiPrdAnly : public SHostWnd
	{
		typedef void(CDlgMultiPrdAnly::* PDATAHANDLEFUNC)(int, const char*);

	public:
		CDlgMultiPrdAnly(vector<StockInfo>& stockInfoVec);
		virtual ~CDlgMultiPrdAnly();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		BOOL	OnInitDialog(EventArgs* e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
		LRESULT OnWindowMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
		LRESULT OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
		void	OnDestroy();
		void	OnBtnClose();
		void	OnBtnRehab();
		void	OnButtonDown(UINT nFlags, CPoint point);
		void	OnRButtonUp(UINT nFlags, CPoint point);
		void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		void	OnExitSizeMove(LPWINDOWPOS lpWndPos);
		void	OnTimer(UINT_PTR nIDEvent);
		bool	OnCmbPrdChange(EventArgs* e);
		bool	OnChkClicked(EventArgs* e);
		void	InitWindowPos();
		//void	CloseWnd();
		UINT	GetThreadID() const { return m_uThreadID; }

	protected:
		void		InitControls();
		void		InitDatas();
		void		InitDataProcFucMap();
		SStringA	InitShowConfig(map<int, ShowPointInfo>& pointMap);
		void		OnRehabMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void		OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void		SaveShowConfig(int nWndIndex);
		void		GetPointData(ShowPointInfo& info, SStringA StockID, int nPeriod);
		bool		CheckDataIsGot(ShowPointInfo& info, int nPeriod);
		void		SetKlineShowData(int nWndIndex, vector<ShowPointInfo>& infoVec, int nPeriod, BOOL bNeedReCalc, int nStartWnd = 0);
		void		GetBelongingIndyName(vector<SStringA>& nameVec);
		void		SetStockInfo(vector<StockInfo>& infoVec, strHash<SStringA>& StockNameMap);
		void		ChangeShowStock(SStringA stockID);
		void		SetDataFlagFalse();
		void 		ProcData();
		void		ChangeShowTimeRangeByPeriod(int& nStartTime,int& nEndTime,int nSrcPrd,int nDstPrd);
		int			GetTradeMinCount(int nTime);
		int			TradeMinCountToTime(int nTradeMinCount);
		void 		OnClose();	
		//消息处理函数
	protected:
		//外部消息
		void OnUpdatePoint(int nMsgLength, const char* info);
		void OnUpdateHisKline(int nMsgLength, const char* info);
		void OnUpdateHisRpsPoint(int nMsgLength, const char* info);
		//void OnUpdateIndexMarket(int nMsgLength, const char* info);
		void OnUpdateStockMarket(int nMsgLength, const char* info);
		//void OnUpdateHisIndexMarket(int nMsgLength, const char* info);
		void OnUpdateHisStockMarket(int nMsgLength, const char* info);
		//void OnUpdateCloseInfo(int nMsgLength, const char* info);
		void OnUpdateHisSecPoint(int nMsgLength, const char* info);
		void OnUpdateRehabInfo(int nMsgLength, const char* info);
		void OnUpdateHisCallAction(int nMsgLength, const char* info);
		void OnUpdateHisTFBase(int nMsgLength, const char* info);
		void OnUpdateTodayTFMarket(int nMsgLength, const char* info);
		void OnUpdateRTTFMarket(int nMsgLength, const char* info);
		void OnUpdateRTPriceVol(int nMsgLength, const char* info);
		void OnUpdateRTTradeVol(int nMsgLength, const char* info);
		void OnUpdateHisTradeVol(int nMsgLength, const char* info);

		//内部消息
		void OnKlineChangeStock(int nMsgLength, const char* info);
		void OnKlineMa(int nMsgLength, const char* info);
		void OnKlineMacd(int nMsgLength, const char* info);
		void OnKlineBand(int nMsgLength, const char* info);
		void OnKlineTargetReCalc(int nMsgLength, const char* info);
		void OnChangeKlineRehab(int nMsgLength, const char* info);
		void OnFixedTimeRehab(int nMsgLength, const char* info);

		//消息辅助函数
		BOOL GetAttPara(char* msg, map<SStringA, SStringA>& paraMap);
		void ProcHisRpsPointFromMsg(ReceivePointInfo* pRecvInfo, const char* info,
			SStringA dataName1, SStringA dataName2, char* attchMsg, int attMsgSize);
		void ProcHisSecPointFromMsg(ReceivePointInfo* pRecvInfo, const char* info,
			SStringA dataName, char* attchMsg, int attMsgSize);
		void ProcHisCAPointFromCAInfo();
		void UpdateTmData(vector<CoreData>& comData, CoreData& data);


	protected:
		virtual void OnFinalMessage(HWND hWnd);
	protected:
		//soui消息
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
			EVENT_NAME_COMMAND(L"btn_Rehab", OnBtnRehab)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgMultiPrdAnly)
			MESSAGE_HANDLER(WM_MULTPRDANALY_MSG, OnMsg)
			MESSAGE_HANDLER(WM_WINDOW_MSG, OnWindowMsg)
			MESSAGE_HANDLER(WM_KLINE_MSG, OnKlineMsg)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			MSG_WM_WINDOWPOSCHANGED(OnExitSizeMove)
			MSG_WM_LBUTTONDOWN(OnButtonDown)
			COMMAND_RANGE_HANDLER_EX(RM_NoRehab, RM_End, OnRehabMenuCmd)
			COMMAND_RANGE_HANDLER_EX(KM_Return, KM_End, OnKlineMenuCmd)
			MSG_WM_RBUTTONUP(OnRButtonUp)
			//MSG_WM_NCCALCSIZE(OnNcCalcSize)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	protected:
		static const int MAX_PIC_NUM = 4;
		BOOL m_bLayoutInited;
		BOOL m_bIsValid;
		SWindow* m_pWnd[MAX_PIC_NUM];
		SKlinePic* m_pKlinePic[MAX_PIC_NUM];
		SComboBox* m_pCbxPeriod[MAX_PIC_NUM];
		SCheckBox* m_pChkCtrl[MAX_PIC_NUM];
		SButton* m_pBtnRehab;
		SStatic* m_pTxtInfo;
		SStatic* m_pTxtPrice;
		CDlgKbElf* m_pDlgKbElf;

	protected:
		unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
		vector<StockInfo> m_StockInfoVec;
		eRehabType m_rehabType;
		int m_nFTRehabTime;
		UINT m_uThreadID;
		int m_nPeriod[MAX_PIC_NUM];
		InitPara m_initPara[MAX_PIC_NUM];
		eMaType m_MaParaSet[MAX_PIC_NUM];
		map<int, ShowPointInfo> m_pointInfoMap;
		map<int, map<SStringA, vector<ExDataGetInfo>>> m_WndPointSubMap;
		map<int, map<SStringA, BOOL>>m_PointGetMap;
		map<int, map<SStringA, BOOL>>m_L1IndyPointGetMap;
		map<int, map<SStringA, BOOL>>m_L2IndyPointGetMap;
		map<int, map<SStringA, vector<CoreData>>> m_PointData;
		map<int, map<SStringA, vector<CoreData>>> m_L1IndyPointData;
		map<int, map<SStringA, vector<CoreData>>> m_L2IndyPointData;
		vector<CommonIndexMarket>m_IndexMarketVec;
		vector<CommonStockMarket>m_StockMarketVec;
		map<int, vector<KlineType>>m_KlineMap;
		map<int, vector<TFBaseMarket>>m_TFBaseMap;
		map<int, vector<TickFlowMarket>>m_RtTFMarketVec;

		vector<CAInfo>m_CallAction;
		vector<TradeVol> m_TradeVolData;
		map<eSubTargetType, int> m_PointDataCount;
		map< eSubTargetType, map<SStringA, vector<SStringA>>>m_SubPicShowNameVec;

		map<int, bool>m_KlineGetMap;
		map<int, bool>m_TFBaseGetMap;

		map<SStringA, double> m_accRehabMap;
		strHash<StockInfo>	m_infoMap;
		int m_nCurMsgWnd;
		bool m_bMarketGet;
		bool m_bCAInfoGet;
		bool m_bTradeVolGet;
		SStringA m_strSubStockID;
		SStringW m_strStockName;
		strHash<SStringA> m_StockName;
		vector<BOOL> m_StockPassSet;
		RpsGroup m_Group;
		UINT m_nLastChar;
		int m_nNowKTParaChange;
		std::thread m_dataProcThread;
		int m_nLongestPrdWnd;
		int m_nNowCtrlWnd;
		int m_nStartDate;
		int m_nStartTime;
		int m_nEndDate;
		int m_nEndTime;
	};
}