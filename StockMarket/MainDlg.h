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
#include "WorkWnd.h"

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
	void	ChangeWindowSetting(int nGroup, SStringA strKey,
		void* pValue, bool bComplex = false);
	void	OnDestroy();
public:
	void InitWorkWnd();
	void InitConfig();
	void SavePicConfig();
public:
	void ReInit();
	void ReInitList();
	//自定义按钮处理

protected:
	void MsgProc();

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


	//子线程
protected:
	thread tDataProc;
	UINT m_DataThreadID;
	map<int, CWorkWnd*>m_MainWndMap;
};
