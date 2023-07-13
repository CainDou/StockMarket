#pragma once
#include "wininet.h"
#include <cstdlib>
#include <thread>
#pragma comment(lib,"wininet.lib")
#define DAYCOUNT 10
namespace SOUI
{
	typedef struct _PlateInfo
	{
		SStringA strPlateID;
		SStringW strPlateName;
		int nCount;
	}PlateInfo;

	typedef struct _LimitUpStock
	{
		SStringA strStockID;
		SStringW strStockName;
		SStringW strLimitUp;
		int nCountDay;
		int nLimitUpDay;
	}LimitUpStock;


	class SColorListCtrlEx;

	class CDlgLimitUpStat :public SHostWnd
	{
	public:
		CDlgLimitUpStat();
		~CDlgLimitUpStat();
		BOOL	OnInitDialog(EventArgs* e);
		BOOL	CloseWnd();
		void	InitWindowPos();
		void	InitNet();
		void	InitData();
		void	OnBtnClose();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		void	UpdateShowPlate(int nDataPos);

	protected:
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		bool	OnListPlateLClick(EventArgs * pEvtBase);
		void	UpdateShowLimitUpStock(int nDataPos, SStringA strPlate);
		int		GetPlateData(int nPos,int nDate); 
		void	GetPlateDataFromStr(int nPos, int nDate,string& str);
		int		GetPlateLimitUpStock(int nPos, int nDate, SStringA strPlateID);
		void	GetPlateLimitUpDataFromMsg(int nPos, int nDate, string& str, SStringA strPlateID);
		void	DataGet();
		void	OnTimer(char cTimerID);
		SStringW Unescape(const string& input);
		virtual void OnFinalMessage(HWND hWnd);
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgLimitUpStat)
			MESSAGE_HANDLER(WM_LIMITUP_MSG, OnMsg)
			//MESSAGE_HANDLER(WM_BACKTESTING_MSG, OnMsg)
			MSG_WM_TIMER_EX(OnTimer);
			MSG_WM_SIZE(OnSize)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SStatic* m_text[DAYCOUNT];
		SColorListCtrlEx* m_lsPlate[DAYCOUNT];
		SColorListCtrlEx* m_lsLimitUpStock[DAYCOUNT];
		BOOL	 m_bLayoutInited;

		HINTERNET hInternet;
		HINTERNET hConnectRT;
		HINTERNET hConnectHis;
		DWORD dwOpenRequestFlags;
		vector<int> m_dayVec;
		map<int, vector<PlateInfo>> m_dayPlateMap;
		map<int, map<SStringA, vector<LimitUpStock>>> m_LimitUpMap;

		unsigned m_uThreadID;
		std::thread m_thread;
		SStringA m_ShowPlate;

		CRITICAL_SECTION m_cs[DAYCOUNT];
	};

}

