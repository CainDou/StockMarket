#pragma once
#include "NetWorkClient.h"
#include "Define.h"
#include "DlgLogin.h"
#include <thread>
#include <map>
#include<vector>
#include<unordered_map>
#include<set>
#include "DataProc.h"

using std::map;
using std::vector;
using std::unordered_map;
using std::thread;
using std::set;

class CWndSynHandler
{
	typedef void(CWndSynHandler::*PDATAHANDLEFUNC)(int, const char*);
	typedef void(CWndSynHandler::*PNETHANDLEFUNC)(SOCKET, ReceiveInfo&);

public:
	CWndSynHandler();
	~CWndSynHandler();
	void Run();
	void Close();
	void SetMianWnd(HWND hWnd);
	void AddWnd(HWND hWnd,UINT threadID);
	void RemoveWnd(HWND hWnd);
	void GetListInsVec(vector<vector<SStringA>> &ListInsVec,
		unordered_map<SStringA, StockInfo, hash_SStringA>& ListInfoMap);

	//初始化函数
public:
	void InitCommonSetting();
	void InitConfig();

	//辅助函数
public:
	bool GetHisPoint(SStringA stockID, int nPeriod, int nGroup);
	bool GetMarket(SStringA stockID, int nGroup);
	bool GetHisKline(SStringA stockID, int nPeriod, int nGroup);
	void InitDataHandleMap();
	void InitNetHandleMap();
	void InitSynHandleMap();
	bool CheckInfoRecv();
	void SetPointDataCapacity();

public:
	bool ReceiveData(SOCKET socket, int size, char end,
		char *buffer, int offset = 0);
	static unsigned __stdcall NetHandle(void* para);
	void Login();
	void DataProc();
	void CalcHisData(const TimeLineData *dataArr,
		int nPeriod, int dataCount);
	void MsgProc();
	void SetVectorSize();
	void ClearData();
	void ReInit();
	//Net具体处理
protected:
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
protected:
	void OnTimeLineUpdate(int nMsgLength, const char* info);
	void OnTodayTimeLineProc(int nMsgLength, const char* info);
	void OnUpdateLastDayEma(int nMsgLength, const char* info);

	//同步窗口数据处理
protected:
	void OnAddWnd(int nMsgLength, const char* info);
	void OnRemoveWnd(int nMsgLength, const char* info);
	void OnUpdatePoint(int nMsgLength, const char* info);
	void OnGetMarket(int nMsgLength, const char* info);
	void OnGetKline(int nMsgLength, const char* info);
	void OnGetPoint(int nMsgLength, const char* info);
	void OnHisPoint(int nMsgLength, const char* info);
	void OnRTIndexMarket(int nMsgLength, const char* info);
	void OnRTStockMarket(int nMsgLength, const char* info);
	void OnHisIndexMarket(int nMsgLength, const char* info);
	void OnHisStockMarket(int nMsgLength, const char* info);
	void OnHisKline(int nMsgLength, const char* info);
	void OnCloseInfo(int nMsgLength, const char* info);
	void OnReinit(int nMsgLength, const char* info);
public:
	vector<SStringA> m_dataNameVec;
	vector<SStringA> m_comDataNameVec;
	vector<SStringA> m_uniDataNameVec;
	vector<vector<SStringA>> m_SubPicShowNameVec;
	vector<SStringA> m_SubPicWndNameVec;
	unordered_map<SStringA, SStringA, hash_SStringA> m_StockNameMap;
	map<int, unordered_map<SStringA, StockInfo, hash_SStringA>> m_ListStockInfoMap;
	vector<int> m_PeriodVec;

protected:
	map<int, BOOL> m_NetHandleFlag;
	char *todayDataBuffer;
	unsigned long todayDataSize;
	bool			m_bTodayInit;

protected:
	thread tLogin;
	thread tDataProc;
	thread tRpsCalc;
	thread tMsgSyn;
	UINT m_uNetThreadID;
	UINT m_RpsProcThreadID;
	UINT m_DataThreadID;
	UINT m_uMsgThreadID;
	//处理函数哈希表
protected:
	unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
	unordered_map<int, PNETHANDLEFUNC>m_netHandleMap;
	unordered_map<int, PDATAHANDLEFUNC>m_synHandleMap;

public:
	unordered_map<SStringA, double, hash_SStringA> m_preCloseMap;
	vector<vector<SStringA>> m_ListInsVec;
	vector<map<int, TimeLineMap>>m_listDataMap;
	vector<map<int, TimeLineArrMap> >m_dataVec;
	map<int, TimeLineArrMap> m_commonDataMap;	// 用来保存通用的需要使用所有数据数据

	//调用子类
protected:
	CDataProc m_dataHandler;
	CNetWorkClient m_NetClinet;
	CDlgLogin* m_pLoginDlg;

protected:
	HWND m_hMain;
	map<HWND,UINT> m_hWndMap;
	map<HWND, map<int, SStringA>> m_WndSubMap;
	SStringA m_strIPAddr;
	int		m_nIPPort;
	bool m_bServerReady;
	CRITICAL_SECTION m_cs;

};

inline void CWndSynHandler::SetMianWnd(HWND hWnd)
{
	m_hMain = hWnd;
}

inline void CWndSynHandler::AddWnd(HWND hWnd, UINT threadID)
{
	m_hWndMap[hWnd] = threadID;
}

inline void CWndSynHandler::RemoveWnd(HWND hWnd)
{
	m_hWndMap.erase(hWnd);
	m_WndSubMap.erase(hWnd);
}

inline void CWndSynHandler::GetListInsVec(vector<vector<SStringA>>& ListInsVec,
	unordered_map<SStringA, StockInfo, hash_SStringA>& ListInfoMap)
{
	ListInsVec = m_ListInsVec;
}
