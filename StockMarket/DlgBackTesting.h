#pragma once
#include<set>
#include "NetWorkClient.h"
#include <thread>
#include <fstream>

using std::set;
using std::thread;
using std::ofstream;
using std::endl;

#define MIN_DIFF 0.000001

typedef struct _backTestingData
{
	//map<日期,map<周期,map<数据代码,map<时间,数据>>>>
	map<int, map<int, map<int, map<int, double>>>> dataMap;
	map<int, double> rehabMap;
}BackTestingData;

typedef struct _TestRes
{
	double RoR1;
	double RoR3;
	double RoR5;
	double RoR10;
	double RoR1Over300;
	double RoR3Over300;
	double RoR5Over300;
	double RoR10Over300;
	double RoR1OverIndy1;
	double RoR3OverIndy1;
	double RoR5OverIndy1;
	double RoR10OverIndy1;
	double RoR1OverIndy2;
	double RoR3OverIndy2;
	double RoR5OverIndy2;
	double RoR10OverIndy2;
	_TestRes():RoR1(NAN),RoR3(NAN), RoR5(NAN),RoR10(NAN),
		RoR1Over300(NAN), RoR3Over300(NAN), RoR5Over300(NAN), RoR10Over300(NAN),
		RoR1OverIndy1(NAN), RoR3OverIndy1(NAN), RoR5OverIndy1(NAN), RoR10OverIndy1(NAN),
		RoR1OverIndy2(NAN), RoR3OverIndy2(NAN), RoR5OverIndy2(NAN), RoR10OverIndy2(NAN)
	{

	}
}TsetRes;

typedef struct _singleRes
{
	SStringA StockID;
	int nDate;
	TsetRes res;
}SingleRes;

typedef struct _avgRes
{
	int nCount;
	TsetRes res;
}AvgRes;

namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgBackTesting:public SHostWnd
	{

		typedef void(CDlgBackTesting::*PBTNETHANDLEFUNC)(ReceiveInfo&);
		typedef bool(CDlgBackTesting::*PCOMPAREFUNC)(double, double);

	public:
		CDlgBackTesting(set<StockFilter> sfSet, vector<StockInfo>& StockInfo);
		~CDlgBackTesting();
		BOOL	OnInitDialog(EventArgs* e);
		void	ReSetCondition(set<StockFilter> sfSet);
		void	InitResTitle();
		void	InitConditonMap();
		void	InitNetHandleMap();
		void	OnBtnClose();
		void	OnBtnCalc();
		void	OnBtnCancel();
		void	OnBtnExport();
		void	OnMaximize();
		void	OnRestore();
		void	OnMinimize();
		void	OnSize(UINT nType, CSize size);
		void	ConditionHandle();
		void	CalcRes();
		void	UpdateList(int nRow);
		BOOL	CheckCondtionIsSame();
	protected:
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);

		//数据通信
	protected:
		static unsigned __stdcall NetHandle(void* para);
		void InitNetConfig();
		bool RecvInfoHandle(BOOL & bNeedConnect, int &nOffset, ReceiveInfo &recvInfo);
		void OnMsgHisMultiData(ReceiveInfo &recvInfo);
		void OnMsgHisIndexKline(ReceiveInfo &recvInfo);
		void OnNoDefineMsg(ReceiveInfo &recvInfo);
		void GetIndexKline(SStringA strIndexID);
		void GetTestMultiData(SStringA StockID);
		//数据处理
	protected:
		void TestingData();
		void CalcData(char* msg,int nMsgLength);
		int GetDataFromMsg(char* msg, BackTestingData& data);
		bool CheckDataPass(StockFilter &sf, map<int, map<int, map<int, double>>>& dataMap);
		bool ProcFitDataRes(SStringA StockID, vector<int>& dataPassDate, BackTestingData& testDataMap);
		void ProcAvgRes(SingleRes &res);
		bool GreaterThan(double a, double b);
		bool EqualOrGreaterThan(double a, double b);
		bool Equal(double a, double b);
		bool EqualOrLessThan(double a, double b);
		bool LessThan(double a, double b);
		bool SaveTestRes(ofstream& ofile, TsetRes& res);
	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_testStart", OnBtnCalc)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
			EVENT_NAME_COMMAND(L"btn_export", OnBtnExport)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgBackTesting)
			MESSAGE_HANDLER(WM_BACKTESTING_MSG, OnMsg)
			MSG_WM_SIZE(OnSize)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SDateTimePicker *m_pDtpStartDate;
		SDateTimePicker *m_pDtpEndDate;
		SColorListCtrlEx* m_pListAvgRes;
		SColorListCtrlEx* m_pListDetailRes;
		SImageButton *m_pBtnCalc;
		SImageButton *m_pBtnExport;
		SStatic *m_pTextState;

	protected:
		set<StockFilter> m_sfSet;
		CNetWorkClient m_NetClient;
		unordered_map<int, PBTNETHANDLEFUNC>m_netHandleMap;
		vector<StockInfo> m_StockInfo;
		map<SStringA, StockInfo>m_StockInfoMap;
		SStringA m_strIPAddr;
		int	m_nIPPort;

		set<StockFilter> m_LastTestSf;
		int m_nLastStartDate;
		int m_nLastEndDate;
		thread m_tCalc;
		unsigned m_uCalcThreadID;
		unsigned m_uNetThreadID;
		BOOL m_bCalc;

		map<SStringA, map<int, KlineType>> m_IndexData;
		vector<StockFilter> m_sfVecByOrder;
		set<int> m_ableCondSet;
		map<int, SStringA> m_IndexNameMap;
		map<SStringA, int> m_RevIndexNameMap;
		map<int, int> m_SFPeriodMap;
		map<int, PCOMPAREFUNC> m_SFConditionMap;
		vector<SingleRes> m_resVec;
		AvgRes m_AvgRes;
		SStringA m_strDataGetMsg;
		HANDLE m_hEvent;
		int m_nDataGetCount;
		int m_nFinishCount;
		BOOL m_bLastIsFinished;
		map<int, SStringA> m_avgResTitle;
		map<int, SStringA> m_singleResTitle;

	};

	inline void CDlgBackTesting::ReSetCondition(set<StockFilter> sfSet)
	{
		m_sfSet = sfSet;
	}
	inline bool CDlgBackTesting::GreaterThan(double a, double b)
	{
		return a > b;
	}

	inline bool CDlgBackTesting::EqualOrGreaterThan(double a, double b)
	{
		return a >= b;
	}

	inline bool CDlgBackTesting::Equal(double a, double b)
	{
		return (a == b) || (abs(a - b) < MIN_DIFF);
	}

	inline bool CDlgBackTesting::EqualOrLessThan(double a, double b)
	{
		return a <= b;
	}

	inline bool CDlgBackTesting::LessThan(double a, double b)
	{
		return a < b;
	}

	
}

