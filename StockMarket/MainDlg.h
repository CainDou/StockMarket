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
#include <thread>

using std::map;
using std::vector;
using std::unordered_map;
using std::thread;

#pragma comment(lib,"ws2_32.lib")

class CMainDlg : public SHostWnd
{
	typedef void(CMainDlg::*PDATAHANDLEFUNC)(int, const char*);
	typedef void(CMainDlg::*PNETHANDLEFUNC)(SOCKET, ReceiveInfo&);

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
	void	SwitchPic2List(int nGroup);
	void	SwitchList2Pic(int nGroup, int nPeriod);
	void	ChangeWindowSetting(int nGroup, SStringA strKey,
		void* pValue, bool bComplex = false);
	void	OnTimer(UINT_PTR nIDEvent);
	void	OnDestroy();
public:
	void InitCommonSetting();
	void InitConfig();
	void InitWndRect();
	void InitSubPic();
	void InitShowConfig();
	void SavePicConfig();
	void SaveListConfig();
	//列表处理
public:
	void InitList();
	bool OnListHeaderClick(EventArgs *pEvtBase);
	bool OnListHeaderSwap(EventArgs *pEvtBase);
	bool OnListDbClick(EventArgs *pEvtBase);
	bool OnListLClick(EventArgs *pEvtBase);

	void SortList(SColorListCtrlEx* pList, bool bSortCode = false);
	void UpdateList(/*InsIDType InsID*/);
	void UpdateList(int nGroup);
	void UpdateListShowStock(int nGroup);
	int static __cdecl SortDouble(void *para1, const void* para2, const void*para3);
	int static __cdecl SortInt(void *para1, const void* para2, const void*para3);
	int static __cdecl SortStr(void *para1, const void* para2, const void*para3);

public:
	bool ReceiveData(SOCKET socket, int size, char end, char *buffer, int offset = 0);
	static unsigned __stdcall NetHandle(void* para);
	//static unsigned __stdcall RpsCalcHandle(void *para);
	//static unsigned __stdcall DataHandle(void *para);
	//static unsigned __stdcall LoginProcess(void* para);
	void Login();
	void DataProc();
	void CalcHisData(const TimeLineData *dataArr,
		int nPeriod, int dataCount);
	//辅助函数
public:
	bool PointInWindow(const CPoint& pt, const CRect& rc);
	void ShowSubStockPic(SStringA stockID, SColorListCtrlEx* pList);
	//bool GetHisData(int satrtDate, int nDay, char* msg);
	bool GetHisPoint(SStringA stockID, int nPeriod, int nGroup);
	bool GetMarket(SStringA stockID, int nGroup);
	bool GetHisKline(SStringA stockID, int nPeriod, int nGroup);
	int  GetGroupFromList(SColorListCtrlEx * pList);
	void InitDataHandleMap();
	void InitNetHandleMap();
	void SetFenShiShowData(int nGroup);
	void SetKlineShowData(int nGroup, int nPeriod,BOOL bNeedReCalc);
	void ProcHisPointFromMsg(ReceivePointInfo*pRecvInfo, const char* info,
		SStringA dataName1,SStringA dataName2);
	void GetBelongingIndyName(vector<SStringA>& nameVec);
	bool CheckInfoRecv();
	void SetPointDataCapacity();
	//Net具体处理
public:
	bool RecvInfoHandle(BOOL & bNeedConnect, int &nOffset, ReceiveInfo &recvInfo);
	void OnMsgClientID(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgStockInfo(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgRTTimeLine(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgTodayTimeLine(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgHisPoint(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgLastDayEma(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgRTIndexMarket(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgRTStockMarket(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgHisIndexMarket(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgHisStockMarket(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgHisKline(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgCloseInfo(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgWait(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnMsgReInit(SOCKET netSocket, ReceiveInfo &recvInfo);
	void OnNoDefineMsg(SOCKET netSocket, ReceiveInfo &recvInfo);

	//接收到的数据处理
public:
	void OnTimeLineUpdate(int nMsgLength, const char* info);
	void OnTodayTimeLineProc(int nMsgLength, const char* info);
	void OnUpdateSingleListData(int nMsgLength, const char* info);
	void OnUpdateHisPointData(int nMsgLength, const char* info);
	void OnUpdateLastDayEma(int nMsgLength, const char* info);
	void OnUpdateIndexMarket(int nMsgLength, const char* info);
	void OnUpdateStockMarket(int nMsgLength, const char* info);
	void OnUpdateHisIndexMarket(int nMsgLength, const char* info);
	void OnUpdateHisStockMarket(int nMsgLength, const char* info);
	void OnUpdateHisKline(int nMsgLength, const char* info);
	void OnUpdateCloseInfo(int nMsgLength, const char* info);

	//重新初始化
public:
	void ReInit();
	void ClearData();
	void ReInitList();
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
	void SetSelectedPeriod(int nGroup, int nPeriod);
	void SetBtnState(SImageButton* nowBtn, SImageButton** preBtn);
	void SetBtnState(SImageButton* nowBtn, bool bSelected);
	void SetBtnState(int nGroup, int nPeriod, bool bSelected);
	void SetListShowIndyStr(SStatic* pText,int nGroup);

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
		MESSAGE_HANDLER(WM_DATA_MSG, OnDataMsg)
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
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
	//各窗口显示和计算参数
private:
	vector<CRect>	m_RectMap;
	InitPara_t		m_arrInit[3];
	BOOL			m_bLayoutInited;
	BOOL			m_bListSWL2Conn;
	BOOL			m_bListStockConn1;
	BOOL			m_bListStockConn2;
	SStringA		m_ListShowInd[3];
	int				m_PeriodArr[3];		//分析图周期
	int				m_ListPeriodArr[3];	//列表周期
	bool			m_bShowList[3];
	bool			m_bTodayInit;
	SortPara		m_SortPara[3];
	bool			m_bListInited;

	//合约信息和相关信息
public:
	vector<StockInfo> m_StockVec;
	vector<StockInfo> m_SWInd1Vec;	//申万一级行业
	vector<StockInfo> m_SWInd2Vec;	//申万二级行业
	vector<StockInfo> m_IndexVec;	//指数
	vector<SStringA> m_dataNameVec;
	vector<SStringA> m_comDataNameVec;
	vector<SStringA> m_uniDataNameVec;
	vector<vector<SStringA>> m_SubPicShowNameVec;
	vector<SStringA> m_SubPicWndNameVec;
	unordered_map<SStringA, SStringA, hash_SStringA> m_StockNameMap;
	map<int, unordered_map<SStringA, StockInfo, hash_SStringA>> m_ListStockInfoMap;
	vector<int> m_PeriodVec;
	//数据接收标志
protected:
	map<int, BOOL> m_NetHandleFlag;
	char *todayDataBuffer;
	unsigned long todayDataSize;
	//处理函数哈希表
public:
	unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
	unordered_map<int, PNETHANDLEFUNC>m_netHandleMap;

	//K线、市场和打分数据
public:
	unordered_map<SStringA, double, hash_SStringA> m_preCloseMap;
	vector<vector<SStringA>> m_ListInsVec;
	vector<unordered_map<SStringA, int, hash_SStringA>> m_ListPosVec;
	vector<TimeLineMap>m_listDataVec;
	vector<map<int, TimeLineArrMap> >m_dataVec;
	vector<unordered_map<SStringA, map<int, BOOL>, hash_SStringA> >m_PointGetVec;
	unordered_map<SStringA, BOOL, hash_SStringA> m_MarketGetMap;
	unordered_map<SStringA, map<int, BOOL>, hash_SStringA> m_KlineGetMap;
	unordered_map<SStringA, vector<CommonStockMarket>, hash_SStringA> m_StkMarketMap;
	unordered_map<SStringA, vector<CommonIndexMarket>, hash_SStringA> m_IdxMarketMap;
	unordered_map<SStringA, map<int, vector<KlineType>>, hash_SStringA> m_KlineMap;
	unordered_map<SStringA, map<int, long long>, hash_SStringA>m_LastVolMap;
	map<int, TimeLineArrMap> m_commonDataMap;	// 用来保存通用的需要使用所有数据数据

	//子线程
protected:
	thread tLogin;
	thread tDataProc;
	thread tRpsCalc;
	UINT m_uNetThreadID;
	UINT m_RpsProcThreadID;
	UINT m_DataThreadID;

	//调用子类
protected:
	CDataProc m_dataHandler;
	SColorListCtrlEx* m_pList[3];
	SFenShiPic* m_pFenShiPic[3];
	SKlinePic* m_pKlinePic[3];
	SStringA m_strSubStock[3];
	CNetWorkClient m_NetClinet;
	SImageButton* m_pPreSelBtn[3];
	CDlgLogin* m_pLoginDlg;
	CDlgKbElf* m_pDlgKbElf;
	SStatic *m_pTxtSWL2Indy;
	SStatic *m_pTxtStockIndy;

	//其他通用数据
protected:
	//map<SColorListCtrlEx*, SortPara> m_SortPara;
	SStringA m_strIPAddr;
	int		m_nIPPort;
	bool m_bServerReady;
	CRITICAL_SECTION m_csClose;
	int m_nLastOptWnd;
	UINT m_nLastChar;

};
