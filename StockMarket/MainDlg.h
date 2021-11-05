// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "NetWorkClient.h"
#include "SColorListCtrlEx.h"
#include "Define.h"
#include "DataProc.h"
#include "SFenShiPic.h"
#include "DlgKbElf.h"
#include <vector>
#include <map>
#include <unordered_map>

using std::map;
using std::vector;
using std::unordered_map;


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
	void    OnLButtonDown(UINT nFlags, CPoint point);
	void	SwitchPic2List(SFenShiPic *pPic);
public:
	void InitConfig();
	void InitWndRect();
	void InitFSPic();
	void InitShowConfig();
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

	//辅助函数
public:
	bool PointInWindow(const CPoint& pt,const CRect& rc);
	void ShowSubStockPic(SStringA stockID, SColorListCtrlEx* pList);
	bool GetHisData(int satrtDate, int nDay, char* msg);
	//自定义按钮处理
public:
	void OnBtnInd1M1Clicked();
	void OnBtnInd1M5Clicked();
	void OnBtnInd1M15Clicked();
	void OnBtnInd1M30Clicked();
	void OnBtnInd1M60Clicked();
	void OnBtnInd2M1Clicked();
	void OnBtnInd2M5Clicked();
	void OnBtnInd2M15Clicked();
	void OnBtnInd2M30Clicked();
	void OnBtnInd2M60Clicked();
	void OnBtnInd3M1Clicked();
	void OnBtnInd3M5Clicked();
	void OnBtnInd3M15Clicked();
	void OnBtnInd3M30Clicked();
	void OnBtnInd3M60Clicked();
	void SetSelectedPeriod(SColorListCtrlEx* pPic,int nPeriod);
	void SetBtnState(SImageButton* nowBtn,SImageButton** preBtn);
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_Ind1M1", OnBtnInd1M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M5", OnBtnInd1M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M15", OnBtnInd1M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M30", OnBtnInd1M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind1M60", OnBtnInd1M60Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M1", OnBtnInd2M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M5", OnBtnInd2M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M15", OnBtnInd2M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M30", OnBtnInd2M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind2M60", OnBtnInd2M60Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M1", OnBtnInd3M1Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M5", OnBtnInd3M5Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M15", OnBtnInd3M15Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M30", OnBtnInd3M30Clicked)
		EVENT_NAME_COMMAND(L"btn_Ind3M60", OnBtnInd3M60Clicked)

	EVENT_MAP_END()
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_MAIN_MSG, OnMsg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
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
	vector<SStringA> m_dataNameVec;;
	vector<SStringA> m_comDataNameVec;
	vector<SStringA> m_uniDataNameVec;
	unordered_map<SStringA, SStringA, hash_SStringA> m_StockNameMap;
	map<SColorListCtrlEx*, vector<SStringA>> m_ListInsMap;
	map<SColorListCtrlEx*, unordered_map<SStringA, int, hash_SStringA>> m_ListPosMap;
	map<SColorListCtrlEx*, TimeLineMap>m_listDataMap;
	map<SColorListCtrlEx*, map<int,TimeLineArrMap> >m_dataMap;
	map<SColorListCtrlEx*, int> m_PeriodMap;
	map<SColorListCtrlEx*, CRect> m_RectMap;

	map<int, TimeLineArrMap> m_commonDataMap;	// 用来保存通用的需要使用所有数据数据
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
	UINT m_DataProcThreadID;
	vector<int> m_PeriodVec;
	SImageButton* m_pPreSelBtn1;
	SImageButton* m_pPreSelBtn2;
	SImageButton* m_pPreSelBtn3;
	CDlgKbElf* m_pDlgKbElf;
	int m_nLastOptWnd;

protected:
	map<SColorListCtrlEx*, SortPara> m_SortPara;
	vector<HANDLE> m_hSubThreadVec;
	HANDLE m_hDataThread;
	UINT m_uNetThreadID;
	bool m_bListInited;

};
