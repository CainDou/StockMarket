#pragma once
#include<thread>
#include"WorkWnd.h"
#include "DlgMsgHandler.h"
class CIniFile;
namespace SOUI
{
	class CDlgSub : public SHostWnd
	{
		typedef void(CDlgSub::*PDATAHANDLEFUNC)(int, const char*);

	public:
		CDlgSub(SStringA strWndName);
		~CDlgSub();
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
		void	SaveStockFilterPara(int nGroup);
		void	SaveComboStockFilterPara(int nGroup);
		void	StopAndClearData();
		BOOL	WindowIsValid();
		SStringA GetWindowName();

	public:
		void InitWindowPos();
		void InitWorkWnd();
		void InitStockFilter();
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
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		//soui消息
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgSub)
			MESSAGE_HANDLER(WM_WINDOW_MSG, OnMsg)
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

		//子线程
	protected:
		vector<CWorkWnd*>m_WndVec;
		UINT m_SynThreadID;
		CDlgMsgHandler m_MsgHandler;
		BOOL m_bIsValid;
	};
	inline BOOL CDlgSub::WindowIsValid()
	{
		return m_bIsValid;
	}
	inline SStringA CDlgSub::GetWindowName()
	{
		return m_strWindowName;
	}
}
