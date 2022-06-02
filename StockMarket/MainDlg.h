// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "NetWorkClient.h"
#include "SColorListCtrlEx.h"
#include "Define.h"
#include "DataProc.h"
#include "SSubPic.h"
#include "DlgKbElf.h"
#include "SFenShiPic.h"
#include "SKlinePic.h"
#include <vector>
#include <map>
#include <unordered_map>
#include "DlgLogin.h"

using std::map;
using std::vector;
using std::unordered_map;


#pragma comment(lib,"ws2_32.lib")

class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void	OnClose();
	void	OnMaximize();
	void	OnRestore();
	void	OnMinimize();
	void	OnSize(UINT nType, CSize size);
	int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL	OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
	LRESULT OnFSMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
	LRESULT OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
	void	OnFSMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
	void	OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
	void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void    OnLButtonDown(UINT nFlags, CPoint point);
	void	OnRButtonUp(UINT nFlags, CPoint point);
	void	SwitchPic2List(int nGroup);
	void	SwitchList2Pic(int nGroup, int nPeriod);
	void	ChangeWindowSetting(int nGroup, SStringA strKey, 
		void* pValue, bool bComplex = false);
	void	OnTimer(UINT_PTR nIDEvent);
	void	OnDestroy();
public:
	void InitConfig();
	void InitWndRect();
	void InitSubPic();
	void InitShowConfig();
	void SavePicConfig();
	//列表处理
public:
	void InitList();
	bool OnListHeaderClick(EventArgs *pEvtBase);
	bool OnListHeaderSwap(EventArgs *pEvtBase);
	bool OnListDbClick(EventArgs *pEvtBase);
	bool OnListLClick(EventArgs *pEvtBase);

	void SortList(SColorListCtrlEx* pList, bool bSortCode = false);
	void UpdateList(/*InsIDType InsID*/);
	void UpdateListShowStock(int nGroup);
	int static __cdecl SortDouble(void *para1, const void* para2, const void*para3);
	int static __cdecl SortInt(void *para1, const void* para2, const void*para3);
	int static __cdecl SortStr(void *para1, const void* para2, const void*para3);

public:
	static bool ReceiveData(SOCKET socket, int size, char end, char *buffer, int offset = 0);
	static unsigned __stdcall NetHandle(void* para);
	static unsigned __stdcall DataHandle(void *para);
	static unsigned __stdcall LoginProcess(void* para);

	//辅助函数
public:
	bool PointInWindow(const CPoint& pt,const CRect& rc);
	void ShowSubStockPic(SStringA stockID, SColorListCtrlEx* pList);
	//bool GetHisData(int satrtDate, int nDay, char* msg);
	bool GetHisPoint(SStringA stockID, int nPeriod, int nGroup);
	bool GetMarket(SStringA stockID, int nGroup);
	bool GetHisKline(SStringA stockID, int nPeriod, int nGroup);
	int  GetGroupFromList(SColorListCtrlEx * pList);

	//自定义按钮处理
public:
	void OnBtnInd1MarketClicked();
	void OnBtnInd1FenShiClicked();
	void OnBtnInd1M1Clicked();
	void OnBtnInd1M5Clicked();
	void OnBtnInd1M15Clicked();
	void OnBtnInd1M30Clicked();
	void OnBtnInd1M60Clicked();
	void OnBtnInd1DayClicked();
	void OnBtnInd2MarketClicked();
	void OnBtnInd2FenShiClicked();
	void OnBtnInd2M1Clicked();
	void OnBtnInd2M5Clicked();
	void OnBtnInd2M15Clicked();
	void OnBtnInd2M30Clicked();
	void OnBtnInd2M60Clicked();
	void OnBtnInd2DayClicked();
	void OnBtnInd2ListConnectClicked();
	void OnBtnInd3MarketClicked();
	void OnBtnInd3FenShiClicked();
	void OnBtnInd3M1Clicked();
	void OnBtnInd3M5Clicked();
	void OnBtnInd3M15Clicked();
	void OnBtnInd3M30Clicked();
	void OnBtnInd3M60Clicked();
	void OnBtnInd3DayClicked();
	void OnBtnInd3ListConnect1Clicked();
	void OnBtnInd3ListConnect2Clicked();
	void OnBtnShowTypeChange(int nGroup, bool bFroceList = false);
	void OnBtnPeriedChange(int nGroup, int nPeriod);
	void SetSelectedPeriod(int nGroup,int nPeriod);
	void SetBtnState(SImageButton* nowBtn,SImageButton** preBtn);
	void SetBtnState(SImageButton* nowBtn, bool bSelected);
	void SetBtnState(int nGroup,int nPeriod,bool bSelected);


protected:
	void SetVectorSize();

protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_Ind1Market", OnBtnInd1MarketClicked)
		EVENT_NAME_COMMAND(L"btn_Ind1FS", OnBtnInd1FenShiClicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M1", OnBtnInd1M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M5", OnBtnInd1M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M15", OnBtnInd1M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M30", OnBtnInd1M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M60", OnBtnInd1M60Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1Day", OnBtnInd1DayClicked)
		EVENT_NAME_COMMAND(L"btn_Ind2Market", OnBtnInd2MarketClicked)
		EVENT_NAME_COMMAND(L"btn_Ind2FS", OnBtnInd2FenShiClicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M1", OnBtnInd2M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M5", OnBtnInd2M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M15", OnBtnInd2M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M30", OnBtnInd2M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M60", OnBtnInd2M60Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2Day", OnBtnInd2DayClicked)
		EVENT_NAME_COMMAND(L"btn_Ind2ListConnect", OnBtnInd2ListConnectClicked)
		EVENT_NAME_COMMAND(L"btn_Ind3Market", OnBtnInd3MarketClicked)
		EVENT_NAME_COMMAND(L"btn_Ind3FS", OnBtnInd3FenShiClicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M1", OnBtnInd3M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M5", OnBtnInd3M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M15", OnBtnInd3M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M30", OnBtnInd3M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M60", OnBtnInd3M60Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3Day", OnBtnInd3DayClicked)
		EVENT_NAME_COMMAND(L"btn_Ind3ListConnect1", OnBtnInd3ListConnect1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3ListConnect2", OnBtnInd3ListConnect2Clicked)

	EVENT_MAP_END()
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_MAIN_MSG, OnMsg)
		MESSAGE_HANDLER(WM_FENSHI_MSG, OnFSMsg)
		MESSAGE_HANDLER(WM_KLINE_MSG, OnKlineMsg)
		COMMAND_RANGE_HANDLER_EX(FM_Return, FM_End, OnFSMenuCmd)
		COMMAND_RANGE_HANDLER_EX(KM_Return, KM_End, OnKlineMenuCmd)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;	
	BOOL			m_bListSWL2Conn;
	BOOL			m_bListStockConn1;
	BOOL			m_bListStockConn2;
	SStringA		m_ListShowInd[3];
public:
	CNetWorkClient m_NetClinet;
	vector<StockInfo> m_StockVec;
	vector<StockInfo> m_SWInd1Vec;	//申万一级行业
	vector<StockInfo> m_SWInd2Vec;	//申万二级行业
	vector<StockInfo> m_IndexVec;	//指数
	vector<SStringA> m_dataNameVec;
	vector<SStringA> m_comDataNameVec;
	vector<SStringA> m_uniDataNameVec;
	unordered_map<SStringA, SStringA, hash_SStringA> m_StockNameMap;
	unordered_map<SStringA, double, hash_SStringA> m_preCloseMap;
	//map<SColorListCtrlEx*, vector<SStringA>> m_ListInsMap;
	//map<SColorListCtrlEx*, unordered_map<SStringA, int, hash_SStringA>> m_ListPosMap;
	//map<SColorListCtrlEx*, TimeLineMap>m_listDataMap;
	//map<SColorListCtrlEx*, map<int,TimeLineArrMap> >m_dataMap;
	vector<vector<SStringA>> m_ListInsVec;
	vector<unordered_map<SStringA, int, hash_SStringA>> m_ListPosVec;
	vector<TimeLineMap>m_listDataVec;
	vector<map<int, TimeLineArrMap> >m_dataVec;

	//map<SColorListCtrlEx*, int> m_PeriodMap;

	int m_PeriodArr[3];		//分析图周期
	int m_ListPeriodArr[3];	//列表周期
	bool m_bShowList[3];
	//map<SColorListCtrlEx*, CRect> m_RectMap;
	//map<SColorListCtrlEx*, unordered_map<SStringA,map<int, BOOL>,hash_SStringA> >m_PointGetMap;
	vector<CRect> m_RectMap;
	vector<unordered_map<SStringA, map<int, BOOL>, hash_SStringA> >m_PointGetVec;

	unordered_map<SStringA,  BOOL, hash_SStringA> m_MarketGetMap;
	unordered_map<SStringA, map<int, BOOL>, hash_SStringA> m_KlineGetMap;


	unordered_map<SStringA, vector<CommonStockMarket>, hash_SStringA> m_StkMarketMap;
	unordered_map<SStringA, vector<CommonIndexMarket>, hash_SStringA> m_IdxMarketMap;

	unordered_map<SStringA, map<int,vector<KlineType>>, hash_SStringA> m_KlineMap;
	InitPara_t m_arrInit[3];
	//map<SColorListCtrlEx*, unordered_map<SStringA, map<int, vector<MACDType>>, hash_SStringA >> m_KlineMACDMap;
	//map<SColorListCtrlEx*, unordered_map<SStringA, map<int, vector<MAType>>, hash_SStringA>>m_KlineMAMap;
	//map<SColorListCtrlEx*, unordered_map<SStringA, vector<MACDType>, hash_SStringA>> m_FSMACDMap;
	//map<SColorListCtrlEx*, unordered_map<SStringA, vector<EMAType>, hash_SStringA>> m_FSMAMap;

	//vector<unordered_map<SStringA, map<int, vector<MACDType>>, hash_SStringA >> m_KlineMACDVec;
	//vector<unordered_map<SStringA, map<int, vector<MAType>>, hash_SStringA>>m_KlineMAVec;
	//vector<unordered_map<SStringA, vector<MACDType>, hash_SStringA>> m_FSMACDVec;
	//vector<unordered_map<SStringA, vector<EMAType>, hash_SStringA>> m_FSMAVec;

	unordered_map<SStringA, map<int, long long>, hash_SStringA>m_LastVolMap;
	SColorListCtrlEx* m_pList[3];
	SFenShiPic* m_pFenShiPic[3];
	SKlinePic* m_pKlinePic[3];
	SStringA m_strSubStock[3];
	//SSubPic* m_pSubPic[3];
	SImageButton* m_pPreSelBtn[3];

	map<int, TimeLineArrMap> m_commonDataMap;	// 用来保存通用的需要使用所有数据数据
	SStringA m_strIPAddr;
	CDataProc m_dataHandler;
	int		m_nIPPort;
	CDlgLogin* m_pLoginDlg;
	//map<SColorListCtrlEx *, SSubPic*> m_ListPicMap;
	//SColorListCtrlEx *m_pListInd1;
	//SColorListCtrlEx *m_pListInd2;
	//SColorListCtrlEx *m_pListInd3;
	//SSubPic* m_pSubPicInd1;
	//SSubPic* m_pSubPicInd2;
	//SSubPic* m_pSubPicInd3;
	UINT m_DataProcThreadID;
	vector<int> m_PeriodVec;
	CDlgKbElf* m_pDlgKbElf;
	CRITICAL_SECTION m_csClose;
	int m_nLastOptWnd;
	UINT m_nLastChar;

protected:
	//map<SColorListCtrlEx*, SortPara> m_SortPara;
	SortPara m_SortPara[3];

	vector<HANDLE> m_hSubThreadVec;
	HANDLE m_hDataThread;
	UINT m_uNetThreadID;
	bool m_bListInited;

};
