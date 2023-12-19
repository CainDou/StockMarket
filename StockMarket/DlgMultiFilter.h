#pragma once
#include<thread>
#include"WorkWnd.h"
#include "IniFile.h"
#include <queue>
#include "DlgMsgHandler.h"

namespace SOUI
{
	class CDlgMultiFilter : public SHostWnd
	{
		typedef void(CDlgMultiFilter::*PDATAHANDLEFUNC)(int, const char*);

	public:
		CDlgMultiFilter(SStringA strWndName);
		~CDlgMultiFilter();
		void	OnClose();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		BOOL	OnInitDialog(EventArgs* e);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		void	OnDestroy();
		void	OnTimer(UINT_PTR nIDEvent);
		void	OnBtnClose();
		void	OnBtnSetWndNum();
		void	SaveComboStockFilterPara(int nWndNum);	
		void	StopAndClearData();
		BOOL	SetShowWndNum(int nNum, BOOL bFromInit);
		BOOL	WindowIsValid();
		SStringA GetWindowName();
	public:
		void InitWindowPos();
		void InitWorkWnd();
		void InitComboStockFilter();
		void InitPointWndInfo(CIniFile& ini, InitPara& initPara, SStringA strSection, map<int, ShowPointInfo> &pointMap);
		void InitConfig(map<int, ShowPointInfo> &pointMap);
		void InitListConfig();
		void SavePointWndInfo(CIniFile& ini, InitPara& initPara, SStringA strSection);
		void SavePicConfig();
		void SaveListConfig();
	public:
		void ReInit();
		void ReInitList();
		void ReInitWorkWnd();



	protected:
		void OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		//soui消息
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
			EVENT_NAME_COMMAND(L"btn_wndNum", OnBtnSetWndNum)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgMultiFilter)
			MESSAGE_HANDLER(WM_WINDOW_MSG, OnMsg)
			COMMAND_RANGE_HANDLER_EX(FWM_Start, FWM_End, OnMenuCmd)
			MSG_WM_TIMER(OnTimer)
			//MSG_WM_CREATE(OnCreate)
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
		SStringA		m_strWindowName;
		int				m_nShowWndNum;

		//子线程
	protected:
		vector<CWorkWnd*>m_WndVec;
		vector<SRealWnd*>m_SrcWndVec;
		UINT m_SynThreadID;
		BOOL m_bIsValid;
		CDlgMsgHandler m_MsgHandler;
	};
	inline BOOL CDlgMultiFilter::WindowIsValid()
	{
		return m_bIsValid;
	}
	inline SStringA CDlgMultiFilter::GetWindowName()
	{
		return m_strWindowName;
	}

	}

