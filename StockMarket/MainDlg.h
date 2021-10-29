// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "NetWorkClient.h"
#include "SColorListCtrlEx.h"
#include "Define.h"
#include "DataProc.h"
#include "SFenShiPic.h"
#include <vector>
#include <map>

using std::map;
using std::vector;


#pragma comment(lib,"ws2_32.lib")

class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
	void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void	SwitchPic2List(SFenShiPic *pPic);
public:
	void InitConfig();
	void InitFSPic();
	//列表处理
public:
	void InitList();
	bool OnListHeaderClick(EventArgs *pEvtBase);
	bool OnListHeaderSwap(EventArgs *pEvtBase);
	bool OnListDbClick(EventArgs *pEvtBase);

	void SortList(SColorListCtrlEx* pList, bool bSortCode = false);
	void UpdateList(/*InsIDType InsID*/);
	int static __cdecl SortDouble(void *para1, const void* para2, const void*para3);
	int static __cdecl SortInt(void *para1, const void* para2, const void*para3);
	int static __cdecl SortStr(void *para1, const void* para2, const void*para3);

public:
	static bool ReceiveData(SOCKET socket, int size, char end, char *buffer);
	static unsigned __stdcall NetHandle(void* para);
	static unsigned __stdcall DataHandle(void *para);


protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
	EVENT_MAP_END()
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_MAIN_MSG, OnMsg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KEYDOWN(OnKeyDown)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;	

public:
	CNetWorkClient m_NetClinet;
	vector<StockInfo> m_StockVec;
	vector<StockInfo> m_SWInd1Vec;	//申万一级行业
	vector<StockInfo> m_SWInd2Vec;	//申万二级行业
	vector<StockInfo> m_IndexVec;	//指数
	vector<SStringA> m_dataNameVec;
	map<SStringA, SStringA> m_StockNameMap;
	map<SColorListCtrlEx*, vector<SStringA>> m_ListInsMap;
	map<SColorListCtrlEx*,map<SStringA, int>> m_ListPosMap;
	map<SColorListCtrlEx*, map<SStringA, map<SStringA, TimeLineData>>>m_listDataMap;
	map<SColorListCtrlEx*, map<SStringA, map<SStringA, vector<TimeLineData>>>>m_dataMap;
	map<SStringA, map<SStringA, vector<TimeLineData>>> m_commonDataMap;
	SStringA m_strIPAddr;
	CDataProc m_dataHandler;
	int		m_nIPPort;
	map<SColorListCtrlEx *, SFenShiPic*> m_ListPicMap;
	SColorListCtrlEx *m_pListInd1;
	SColorListCtrlEx *m_pListInd2;
	SColorListCtrlEx *m_pListInd3;
	SFenShiPic* m_pFSPicInd1;
	SFenShiPic* m_pFSPicInd2;
	SFenShiPic* m_pFSPicInd3;


protected:
	map<SColorListCtrlEx*, SortPara> m_SortPara;
	UINT m_uNetThreadID;
	bool m_bListInited;

};
