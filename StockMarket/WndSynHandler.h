#pragma once
#include "NetWorkClient.h"
#include "Define.h"
#include "DlgLogin.h"
#include <thread>
#include <map>
#include<vector>
#include<unordered_map>
#include<set>
using std::map;
using std::vector;
using std::unordered_map;
using std::thread;
using std::set;
#define MAX_ASK_ID 32767 //2^15 - 1

class CWndSynHandler
{
	typedef void(CWndSynHandler::*PDATAHANDLEFUNC)(int, const char*);
	typedef void(CWndSynHandler::*PNETHANDLEFUNC)(ReceiveInfo&);

public:
	CWndSynHandler();
	~CWndSynHandler();
	void Run();
	void Close();
	void SetMainWnd(HWND hWnd);
	void AddWnd(HWND hWnd,UINT threadID);
	void RemoveWnd(HWND hWnd);
	void GetListInsVec(vector<vector<StockInfo>> &ListInsVec,
		 strHash<SStringA>& StockNameVec);
	vector<map<int, strHash<RtRps>>>* GetListData();
	map<int, strHash<TickFlowMarket>>* GetTFMarket();
	vector<map<int, vector<vector<double>>>>* GetFilterData();
	vector<strHash<CAInfo>>* GetCallActionData();
	map<int, strHash<int>>* GetStockPos();
	void GetSubPicShowNameVec(vector<vector<SStringA>>& SubPicShowNameVec);
	UINT GetThreadID() const;
	strHash<double> GetCloseMap() const;
	void SetCmdLine(LPCTSTR lpstrCmdLine);
	map<int, ShowPointInfo> GetPointInfo();
	void SetSubWndInfo(HWND hSubWnd,HWND hWnd,int nGroup);
	//初始化函数
protected:
	void InitCommonSetting();
	void InitNetConfig();
	void InitPointInfo();
	void InitDataNameMap();
	void InitRpsDataMap(string strDataName,int nMarketStart,int nL1Start,int nL2Start);
	void InitSecDataMap(string strDataName, int nMarketStart, int nL1Start, int nL2Start);

	//辅助函数
protected:
	int GetHisPoint(int nMsgType,SStringA stockID, int nPeriod, int nGroup,SStringA attInfo);
	int GetMarket(SStringA stockID, int nGroup);
	int GetHisKline(SStringA stockID, int nPeriod, int nGroup);
	int GetHisCallAction(SStringA stockID, int nPeriod, int nGroup);

	void InitDataHandleMap();
	void InitNetHandleMap();
	void InitSynHandleMap();
	bool CheckInfoRecv();
	bool CheckCmdLine();
	bool GetAutoUpdateFile(SStringA strMD5);
	bool GetAutoUpdateFileVer(SStringA &strMD5);
	bool HandleRpsData(string strDataName, sRps& data, 
		vector<double>&filterDataMap);
	bool HandleSecData(string strDataName, sSection& data,
		vector<double>&filterDataMap);
	void UpdateRtRpsPointData(vector<RtPointData>& subDataVec,
		RtPointData& dstData, sRps &rpsData, SStringA strDataName, int nGroup);
	void UpdateRtSecPointData(vector<RtPointData>& subDataVec,
		RtPointData& dstData, sSection &secData, SStringA strDataName, int nGroup);
	void UpdateRtSecPointFromCAInfo(vector<RtPointData>& subDataVec,
		RtPointData& dstData, CAInfo& caInfo, int nGroup);
protected:
	//bool ReceiveData(SOCKET socket, int size, char end,
	//	char *buffer, int offset = 0);
	static unsigned __stdcall NetHandle(void* para);
	void Login();
	void DataProc();
	void MsgProc();
	void SetVectorSize();
	void ClearData();
	void ReInit();
	//Net具体处理
protected:
	bool RecvInfoHandle(BOOL & bNeedConnect, int &nOffset, ReceiveInfo &recvInfo);
	void OnMsgClientID( ReceiveInfo &recvInfo);
	void OnMsgStockInfo( ReceiveInfo &recvInfo);
	void OnMsgRTTimeLine( ReceiveInfo &recvInfo);
	void OnMsgTodayTimeLine( ReceiveInfo &recvInfo);
	void OnMsgHisRpsPoint( ReceiveInfo &recvInfo);
	void OnMsgLastDayEma( ReceiveInfo &recvInfo);
	void OnMsgRTIndexMarket( ReceiveInfo &recvInfo);
	void OnMsgRTStockMarket( ReceiveInfo &recvInfo);
	void OnMsgHisIndexMarket( ReceiveInfo &recvInfo);
	void OnMsgHisStockMarket( ReceiveInfo &recvInfo);
	void OnMsgHisKline( ReceiveInfo &recvInfo);
	void OnMsgCloseInfo( ReceiveInfo &recvInfo);
	void OnMsgWait( ReceiveInfo &recvInfo);
	void OnMsgReInit( ReceiveInfo &recvInfo);
	void OnMsgRTTFMarket( ReceiveInfo &recvInfo);
	void OnMsgRtRps( ReceiveInfo &recvInfo);
	void OnMsgHisSecPoint( ReceiveInfo &recvInfo);
	void OnMsgRehabInfo( ReceiveInfo &recvInfo);
	void OnMsgCallAction( ReceiveInfo &recvInfo);
	void OnMsgHisCallAction( ReceiveInfo &recvInfo);
	void OnNoDefineMsg( ReceiveInfo &recvInfo);

	//接收到的数据处理
protected:
	void OnUpdateTFMarket(int nMsgLength, const char* info);
	void OnUpdateRtRps(int nMsgLength, const char* info);
	void OnClearData(int nMsgLength, const char* info);
	void OnUpdateCallAction(int nMsgLength, const char* info);
	//同步窗口数据处理
protected:
	void OnAddWnd(int nMsgLength, const char* info);
	void OnRemoveWnd(int nMsgLength, const char* info);
	void OnGetMarket(int nMsgLength, const char* info);
	void OnGetKline(int nMsgLength, const char* info);
	void OnGetPoint(int nMsgLength, const char* info);
	void OnUpdateList(int nMsgLength, const char* info);
	void OnUpdatePoint(int nMsgLength, const char* info);
	void OnHisRpsPoint(int nMsgLength, const char* info);
	void OnRTIndexMarket(int nMsgLength, const char* info);
	void OnRTStockMarket(int nMsgLength, const char* info);
	void OnHisIndexMarket(int nMsgLength, const char* info);
	void OnHisStockMarket(int nMsgLength, const char* info);
	void OnHisKline(int nMsgLength, const char* info);
	void OnCloseInfo(int nMsgLength, const char* info);
	void OnReinit(int nMsgLength, const char* info);	
	void OnHisSecPoint(int nMsgLength, const char* info);
	void OnRehabInfo(int nMsgLength, const char* info);
	void OnHisCallAction(int nMsgLength, const char* info);
	void OnGetCallAction(int nMsgLength, const char* info);

public:
	vector<SStringA> m_dataNameVec;
	//vector<SStringA> m_comDataNameVec;
	vector<SStringA> m_uniDataNameVec;
	map<int, ShowPointInfo>m_pointInfoMap;
	vector<vector<SStringA>> m_SubPicShowNameVec;
	map<int, strHash<StockInfo>> m_ListStockInfoMap;
	map<int, strHash<int>> m_StockPos;

	strHash<SStringA> m_StockName;
	vector<int> m_PeriodVec;
	map<int, strHash<TickFlowMarket>> m_TFMarketHash;
	vector<map<int, strHash<RtRps>>> m_RtRpsHash;
	vector<map<int,vector<vector<double>>>> m_FilterDataMap;
	//分组 - 周期 -股票Index 日期序列 数据ID 数据序列
	vector<strHash<CAInfo>> m_CallActionHash;
	//map<string, vector<string>> m_rpsDataNameMap;
	//map<string, vector<string>> m_secDataNameMap;

	map<string, vector<int>> m_rpsDataNameMap;
	map<string, vector<int>> m_secDataNameMap;

protected:
	map<int, BOOL> m_NetHandleFlag;
	char *todayDataBuffer;
	unsigned long todayDataSize;
	bool			m_bTodayInit;
	bool			m_bFirstData;
	bool			m_bCaUpdate;
	map<ePointType, int> m_PointGetMsg;

protected:
	thread tLogin;
	thread tRpsCalc;
	thread tMsgSyn;
	UINT m_uNetThreadID;
	UINT m_RpsProcThreadID;
	UINT m_uMsgThreadID;
	//处理函数哈希表
protected:
	unordered_map<int, PDATAHANDLEFUNC>m_dataHandleMap;
	unordered_map<int, PNETHANDLEFUNC>m_netHandleMap;
	unordered_map<int, PDATAHANDLEFUNC>m_synHandleMap;

public:
	strHash<double>m_preCloseMap;
	vector<vector<SStringA>> m_ListInsVec;
	vector<vector<StockInfo>> m_ListInfoVec;
	map<int,map<int, unordered_map<string, map<string, double>>>> m_allDataHash;

	//调用子类
protected:
	CNetWorkClient m_NetClient;
	CDlgLogin* m_pLoginDlg;

protected:
	HWND m_hMain;
	map<HWND,UINT> m_hWndMap;	//一级窗口的工作线程号
	map<HWND, SStringA> m_WndSubMap;
	map<HWND, HWND> m_hSubWndMap; //工作窗口的窗口句柄与之对应的一级窗口句柄
	map<HWND, set<SStringA>> m_WndPointSubMap; //窗口的打分数据信息
	map<HWND, int> m_SubWndGroup; //工作窗口的分组
	map<HWND, set<int>>m_SubWndGetInfoMap;
	SStringA m_strIPAddr;
	int		m_nIPPort;
	bool m_bServerReady;
	bool bExit;
	CRITICAL_SECTION m_cs;
	CRITICAL_SECTION m_csFilterData;
	SStringW m_strCmdLine;
};

inline void CWndSynHandler::SetMainWnd(HWND hWnd)
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

inline void CWndSynHandler::GetListInsVec(
	vector<vector<StockInfo>>& ListInsVec, 
	strHash<SStringA>&StockName)
{
	ListInsVec = m_ListInfoVec;
	StockName = m_StockName;
}

inline vector<map<int, strHash<RtRps>>>* CWndSynHandler::GetListData()
{
	return &m_RtRpsHash;
}

inline map<int, strHash<TickFlowMarket>>* CWndSynHandler::GetTFMarket()
{
	return &m_TFMarketHash;
}

inline vector<map<int, vector<vector<double>>>>* CWndSynHandler::GetFilterData()
{
	return &m_FilterDataMap;
}

inline vector<strHash<CAInfo>>* CWndSynHandler::GetCallActionData()
{
	return &m_CallActionHash;
}

inline map<int, strHash<int>>* CWndSynHandler::GetStockPos()
{
	return &m_StockPos;
}

inline void CWndSynHandler::GetSubPicShowNameVec(
	vector<vector<SStringA>>& SubPicShowNameVec)
{
	SubPicShowNameVec = m_SubPicShowNameVec;
}

inline UINT CWndSynHandler::GetThreadID() const
{
	return m_uMsgThreadID;
}

inline strHash<double> CWndSynHandler::GetCloseMap() const
{
	return m_preCloseMap;
}

inline void CWndSynHandler::SetCmdLine(LPCTSTR lpstrCmdLine)
{
	m_strCmdLine = lpstrCmdLine;
}

inline map<int, ShowPointInfo> CWndSynHandler::GetPointInfo()
{
	return m_pointInfoMap;
}

inline void CWndSynHandler::SetSubWndInfo(HWND hSubWnd, HWND hWnd, int nGroup)
{
	m_hSubWndMap[hSubWnd]= hWnd;
	m_SubWndGroup[hSubWnd] = nGroup;
}

