#pragma once
#include<thread>
#include "SLpPriceVolPic.h"
namespace SOUI
{
	class CDlgLpPriceVol : public SHostWnd
	{
	#define MAX_PICNUM 6
		typedef void(CDlgLpPriceVol::*PDATAHANDLEFUNC)(int, const char*);

	public:
		CDlgLpPriceVol(vector<StockInfo>& stockInfoVec);
		~CDlgLpPriceVol();
		void	OnClose();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		BOOL	OnInitDialog(EventArgs* e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
		void	OnDestroy();
		void	OnTimer(UINT_PTR nIDEvent);
		void	OnBtnClose();
		void	OnLButtonUp(UINT nFlags, CPoint point);
		BOOL	WindowIsValid();
		BOOL	SetShowPicNum(int nNum);
		BOOL	GetLpPriceData(int nPeriod,int nDate);
		BOOL	CheckPicMinHeight();
	public:
		bool OnEditStockIDChange(EventArgs *e);
		void OnMouseMove(UINT nFlags, CPoint pt);
		void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
		bool OnLbStockIDLButtonDown(EventArgs * e);
		bool OnCbxPicNumChange(EventArgs * e);
		bool OnCbxDataTypeChange(EventArgs * e);
		bool OnCbxDataLevelChange(EventArgs * e);
		bool OnCbxBasePriceChange(EventArgs * e);
		bool OnEditBasePriceChange(EventArgs * e);
		bool OnCbxDiffTypeChange(EventArgs * e);
		bool OnCbxBaseWndChange(EventArgs * e);
		bool OnCbxPeriodChange(EventArgs * e);
		bool OnDtpDateChange(EventArgs * e);
	public:
		void InitWindowPos();
		void InitControls();
		void InitConfig();
		void SaveConfig();
		void SetPicDataDate(SYSTEMTIME st,int nPic);
		map<int, PeriodPriceVolInfo> CalcShowPic(vector<PeriodPriceVolInfo> &dataVec);
		void HandleShowPicWithDataType(map<int, PeriodPriceVolInfo>& ShowDataMap);
		BOOL HandleMaxPiantData(map<int, PeriodPriceVolInfo>& ShowDataMap);
		BOOL HandleDiffType(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd);
		BOOL HandleShowDiff(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd);
		BOOL HandleShowOverlapping(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd);
		BOOL HandleShowDeflate(map<int, PeriodPriceVolInfo>& ShowDataMap, int nPicWnd);
	protected:
		void InitDataHandleMap();
		void DataHandle();
		void OnUpdateLpPriceVol(int nMsgLength, const char* info);
		void ReCalcShowData(int nMsgLength, const char* info);
		void OnUpdateListData(int nMsgLength, const char* info);
		void ChangeShowDiff(int nMsgLength, const char* info);
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

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgLpPriceVol)
			MESSAGE_HANDLER(WM_LPPRICEVOL_MSG, OnMsg)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			//MSG_WM_NCCALCSIZE(OnNcCalcSize)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
		//各窗口显示和计算参数
		//子线程
	protected:
		BOOL m_bLayoutInited;
		BOOL m_bIsValid;
		SEdit *m_pEditStockID;
		SListBox* m_pLbStockID;
		SComboBox* m_pCbxPicNum;
		SComboBox* m_pCbxDataType;
		SComboBox* m_pCbxDataLevel;
		SComboBox* m_pCbxBasePrice;
		SEdit*	   m_pEditBasePrice;
		SComboBox* m_pCbxDiffType;
		SStatic*   m_pTextBaseWnd;
		SComboBox* m_pCbxBaseWnd;
		SGroup*	   m_pGrpPic[MAX_PICNUM];
		SComboBox* m_pCbxPeriod[MAX_PICNUM];
		SDateTimePicker *m_pDtpDate[MAX_PICNUM];
		SLpPriceVolPic* m_pLpPriceVolPic[MAX_PICNUM];
	protected:
		UINT m_SynThreadID;
		UINT m_uMsgThreadID;
		std::thread m_thread;
		vector<StockInfo> m_StockInfoVec;
		vector<vector<PeriodPriceVolInfo>> m_LpDataVec;
		vector<map<int, PeriodPriceVolInfo>> m_LpShowData;
		BOOL m_bSetFirst;
		CPoint m_ptMouse;
		int m_nPicPeriod[MAX_PICNUM];
		int m_nPicDate[MAX_PICNUM];
		int m_nDtpYearMonth[MAX_PICNUM];
		int	m_nShowPicNum;
		int m_nDataType;
		int m_nDataLevel;
		int	m_nBasePriceType;
		double m_fBasePrice;
		SStringA m_StockID; 
		SStringA m_StockName;
		unordered_map<int, PDATAHANDLEFUNC>m_DataHandleMap;
		int m_nMaxPrice;
		int m_nMinPrice;
		double m_fMaxVol;
		int m_nMinLevel;
		strHash<RtRps>* m_pShowRpsData;
		int m_nDiffType;
		int	m_nBaseWnd;


	};
	inline BOOL CDlgLpPriceVol::WindowIsValid()
	{
		return m_bIsValid;
	}


}
