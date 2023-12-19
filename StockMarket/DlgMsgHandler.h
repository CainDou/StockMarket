#pragma once
#include "WorkWnd.h"

class CDlgMsgHandler
{
	typedef void(CDlgMsgHandler::*PDATAHANDLEFUNC)(int, const char*);

public:
	CDlgMsgHandler();
	~CDlgMsgHandler();
	UINT Init(HWND hwnd, vector<CWorkWnd*>& workWndVec,UINT SynID,BOOL bOnlyStock);
	BOOL Stop();
protected:
	void InitMsgHandleMap();
	void MsgProc();
	void OnUpdateList(int nMsgLength, const char* info);
	void OnGetMarket(int nMsgLength, const char* info);
	void OnGetKline(int nMsgLength, const char* info);
	void OnGetPoint(int nMsgLength, const char* info);
	void OnUpdatePoint(int nMsgLength, const char* info);
	//void OnTodayPoint(int nMsgLength, const char* info);
	void OnHisRpsPoint(int nMsgLength, const char* info);
	void OnRTIndexMarket(int nMsgLength, const char* info);
	void OnRTStockMarket(int nMsgLength, const char* info);
	void OnHisIndexMarket(int nMsgLength, const char* info);
	void OnHisStockMarket(int nMsgLength, const char* info);
	void OnHisKline(int nMsgLength, const char* info);
	void OnCloseInfo(int nMsgLength, const char* info);
	void OnChangeIndy(int nMsgLength, const char* info);
	void OnHisSecPoint(int nMsgLength, const char* info);
	void OnRehabInfo(int nMsgLength, const char* info);
	void OnHisCallAction(int nMsgLength, const char* info);
	void OnGetCallAction(int nMsgLength, const char* info);
	void OnHisTFBase(int nMsgLength, const char* info);
	void OnGetHisTFBase(int nMsgLength, const char* info);
	void OnTodayTFMarket(int nMsgLength, const char* info);
	void OnRTTFMarket(int nMsgLength, const char* info);
	void OnRTPriceVol(int nMsgLength, const char* info);
protected:
	HWND m_hWnd;
	unordered_map<int, PDATAHANDLEFUNC>m_MsgHandleMap;
	map<int, map<SStringA, vector<ExDataGetInfo>>> m_WndPointSubMap;
	vector<CWorkWnd*>m_WndVec;
	map<int, SStringA> m_WndSubMap;
	UINT m_SynThreadID;
	map<HWND, int> m_WndHandleMap;
	thread tMsgProc;
	UINT m_uThreadID;
	BOOL m_bOnlyStock;

};

