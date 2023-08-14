// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Define.h"
#include <vector>
#include <map>
#include <unordered_map>
#include "DlgLogin.h"
#include <thread>
#include "WorkWnd.h"
#include "DlgSub.h"
#include "IniFile.h"
#include "DlgMultiFilter.h"
#include "DlgLimitUpStat.h"

using std::map;
using std::vector;
using std::unordered_map;
using std::thread;

#pragma comment(lib,"ws2_32.lib")

class CMainDlg : public SHostWnd
{
	typedef void(CMainDlg::*PDATAHANDLEFUNC)(int, const char*);

public:
	CMainDlg();
	~CMainDlg();

	void	OnClose();
	void	OnMaximize();
	void	OnRestore();
	void	OnMinimize();
	void	OnBtnHelp();
	void	SetCmdLine(LPCTSTR lpstrCmdLine);
	void	OnSize(UINT nType, CSize size);
	int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL	OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
	void	OnDestroy();
	void	OnTimer(UINT_PTR nIDEvent);
	void	OnBtnNew();
	void	OnBtnFuncEdit();
	void	OnBtnMultiFilter();
	void	OnBtnLimitUpStat();
	void	CloseSubWindows();
	void	OpenSubWindows();
	void	CloseFilterWindows();
	void	OpenFilterWindows();
	void	CloseLimitUpWindow();
	void	OpenLimitUpWindow();
	void	OpenSubWindowWithNum(int nWndNum);
	void	OpenFilterWindowWithNum(int nWndNum);
	void	OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
	void	SaveStockFilterPara(int nGroup);
	void	SaveComboStockFilterPara(int nGroup);
	void	ShowUpdateLog();

public:
	void InitWindowPos();
	void InitWorkWnd();
	void InitStockFilter();
	void InitComboStockFilter();
	void InitPointWndInfo(CIniFile& ini, InitPara& initPara, SStringA strSection,map<int,ShowPointInfo> &pointMap);
	void InitConfig(map<int, ShowPointInfo> &pointMap);
	void InitListConfig();
	void SavePointWndInfo(CIniFile& ini, InitPara& initPara, SStringA strSection);
	void SavePicConfig();
	void SaveListConfig();
public:
	void ReInit();
	void ReInitList();
	void ReInitWorkWnd();
	//自定义按钮处理

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

protected:
	//soui消息
	EVENT_MAP_BEGIN()			
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_help", OnBtnHelp)
		EVENT_ID_COMMAND(R.id.btn_mulitWindow, OnBtnNew)
		EVENT_NAME_COMMAND(L"btn_EditFunc", OnBtnFuncEdit)
		EVENT_NAME_COMMAND(L"btn_MultiFilter", OnBtnMultiFilter)
		EVENT_NAME_COMMAND(L"btn_LimitUpStat", OnBtnLimitUpStat)

		EVENT_MAP_END()

		//HostWnd真实窗口消息处理
		BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_WINDOW_MSG, OnMsg)
		COMMAND_RANGE_HANDLER_EX(WDM_Return,WDM_End,OnMenuCmd)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
	//各窗口显示和计算参数
private:
	BOOL			m_bLayoutInited;
	unordered_map<int, PDATAHANDLEFUNC>m_MsgHandleMap;
	SStringW		m_strCmdLine;
	//子线程
protected:
	thread tDataProc;
	UINT m_DataThreadID;
	map<int, CWorkWnd*>m_WndMap;
	map<HWND, int> m_WndHandleMap;
	map<int, SStringA> m_WndSubMap;
	UINT m_SynThreadID;
	vector<CDlgSub*> m_SubWndVec;
	vector<CDlgMultiFilter*>m_FilterWndVec;
	map<int, map<SStringA, vector<ExDataGetInfo>>> m_WndPointSubMap;
	int m_nNewWndType;
	CDlgLimitUpStat *m_pDlgLUS;
	//CDlgMultiFilter *m_pDlgMultiFilter;
};
