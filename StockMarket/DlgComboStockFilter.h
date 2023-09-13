#pragma once
#include<set>
#include "NetWorkClient.h"
namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgBackTesting;
	class CDlgComboStockFilter : public SHostWnd
	{
	public:
		CDlgComboStockFilter(HWND hParWnd, UINT uParThreadID, RpsGroup Group);
		~CDlgComboStockFilter();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnOKHis();
		void	OnBtnBackTesting();
		void	OnBtnSetFliterName();
		void	OnBtnSaveHisFitler();
		void	OnBtnRtFitler();
		void	OnBtnHisFitler();
		void	SetFilterName(SStringA strFilterName);
		void	InitList(bool bUse, vector<StockFilter> &sfVec);
		void	InitHisList(bool bUse, vector<HisStockFilter> &sfVec);
		void	InitHisFilterRes();
		void	InitComboBox();
		void	InitHisComboBox();
		void	OnCheckUse();
		void	OnCheckUseHis();
		void	StopFilter();
		void	SetStockInfo(vector<StockInfo> &stockInfo);

		void	InitStringMap();
		void	InitHisStringMap();

		void	OutPutCondition(vector<StockFilter> &sfVec);
		void	OutPutHisCondition(vector<HisStockFilter> &sfVec);
		set<SStringA>	OutPutHisPassStock();

	protected:
		void	ListAddItem(StockFilter& sf);
		void	ListChangeItem(int nRow, StockFilter& sf);
		void	ListDeleteItem(int nRow);
		bool	GetListItem(int nRow, StockFilter& sf);

		void	HisListAddItem(HisStockFilter& hsf);
		void	HisListChangeItem(int nRow, HisStockFilter& hsf);
		void	HisListDeleteItem(int nRow);
		bool	GetHisListItem(int nRow, HisStockFilter& hsf);
		void	CalcHisStockFilter();
	protected:
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		bool	OnCbxFuncChange(EventArgs * e);
		bool	OnCbxIndex1Change(EventArgs * e);
		bool	OnCbxIndex2Change(EventArgs * e);
		bool	OnCbxIDChange(EventArgs * e);

		void	OnCbxFuncAdd();
		void	OnCbxFuncChange();
		void	OnCbxFuncDelete();

		void	AddConditon();
		void	ChangeCondition();
		void	DeleteCondition();

		bool	OnCbxHisFuncChange(EventArgs * e);
		bool	OnCbxHisIndex1Change(EventArgs * e);
		bool	OnCbxHisIndex2Change(EventArgs * e);
		bool	OnCbxHisIDChange(EventArgs * e);

		void	OnCbxHisFuncAdd();
		void	OnCbxHisFuncChange();
		void	OnCbxHisFuncDelete();

		void	AddHisConditon();
		void	ChangeHisCondition();
		void	DeleteHisCondition();



	protected:
		virtual void OnFinalMessage(HWND hWnd);


	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_OKHis", OnBtnOKHis)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"chk_use", OnCheckUse)
			EVENT_NAME_COMMAND(L"chk_useHis", OnCheckUseHis)
			EVENT_NAME_COMMAND(L"btn_saveHis", OnBtnSaveHisFitler)
			EVENT_NAME_COMMAND(L"btn_setFilterName", OnBtnSetFliterName)
			EVENT_NAME_COMMAND(L"btn_rtFilter", OnBtnRtFitler)
			EVENT_NAME_COMMAND(L"btn_hisFilter", OnBtnHisFitler)
			EVENT_NAME_COMMAND(L"btn_backTesting", OnBtnBackTesting)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgComboStockFilter)

			MESSAGE_HANDLER(WM_FILTER_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()


	protected:
		SCheckBox*	  m_pCheckUse;
		SColorListCtrlEx* m_pList;
		SComboBox*	  m_pCbxFunc;
		SComboBox*	  m_pCbxID;
		SComboBox*	  m_pCbxPeriod1;
		SComboBox*	  m_pCbxIndex1;
		SComboBox*	  m_pCbxCondition;
		SComboBox*	  m_pCbxIndex2;
		SComboBox*	  m_pCbxPeriod2;
		SStatic*	  m_pTextID;
		SStatic*	  m_pTextPeriod1;
		SStatic*	  m_pTextIndex1;
		SStatic*	  m_pTextCondition;
		SStatic*	  m_pTextIndex2;
		SStatic*	  m_pTextPeriod2;
		SStatic*	  m_pTextNum;
		SEdit*		  m_pEditNum;

		SCheckBox*	  m_pCheckHisUse;
		SColorListCtrlEx* m_pListHis;
		SComboBox*	  m_pCbxHisFunc;
		SComboBox*	  m_pCbxHisID;
		SComboBox*	  m_pCbxHisPeriod1;
		SComboBox*	  m_pCbxHisIndex1;
		SComboBox*	  m_pCbxHisCondition;
		SComboBox*	  m_pCbxHisIndex2;
		SComboBox*	  m_pCbxHisPeriod2;
		SStatic*	  m_pTextHisID;
		SStatic*	  m_pTextHisPeriod1;
		SStatic*	  m_pTextHisIndex1;
		SStatic*	  m_pTextHisCondition;
		SStatic*	  m_pTextHisIndex2;
		SStatic*	  m_pTextHisPeriod2;
		SStatic*	  m_pTextHisNum;
		SEdit*		  m_pEditHisNum;
		SStatic*	  m_pTextHisCntDayFront;
		SStatic*	  m_pTextHisCntDayBack;
		SEdit*		  m_pEditHisCntDay;
		SRadioBox*    m_pRadioExist;
		SRadioBox*    m_pRadioForall;
		SStatic*	  m_pTextHisLog;
		SImageButton* m_pBtnSaveHis;
		STabCtrl*	  m_pTab;
		SImageButton* m_pBtnRtFilter;
		SImageButton* m_pBtnHisFilter;

	protected:
		HWND m_hParWnd;
		SStringA m_strParWnd;
		RpsGroup	m_Group;
		map<int, SStringW> m_FuncMap;
		map<int, SStringW> m_PeriodMap;
		map<int, SStringW> m_IndexMap;
		map<int, int>m_Index1PosMap;
		map<int, int>m_Index2PosMap;
		map<int, SStringW> m_ConditionMap;
		map<int, SStringW> m_NumUint;
		map<SStringW, int> m_ReverseFuncMap;
		map<SStringW, int> m_ReversePeriodMap;
		map<SStringW, int> m_ReverseIndexMap;
		map<SStringW, int> m_ReverseConditionMap;
		std::set<StockFilter> m_sfSet;
		UINT m_uParThreadID;
		CDlgBackTesting* m_pDlgBackTesting;
		vector<StockInfo> m_stockInfo;
		SStringA m_strFilterName;
		CNetWorkClient m_NetClient;
		SStringA m_strIPAddr;
		int	m_nIPPort;

		map<int, SStringW> m_hisIndexMap;
		map<int, int>m_hisIndex1PosMap;
		map<int, int>m_hisIndex2PosMap;
		map<int, SStringW> m_JudgeTypeMap;
		map<SStringW, int> m_ReverseJudgeTypeMap;

		std::set<HisStockFilter> m_hisSfSet;
		std::set<HisStockFilter> m_hisTmpSfSet;
		vector<BOOL> m_HisFilterRes;
		BOOL m_bHisChange;
		BOOL m_bHisCalced;
		set<SStringA> m_hisFitStockSet;

	};

	inline void CDlgComboStockFilter::SetFilterName(SStringA strFilterName)
	{
		m_strFilterName = strFilterName;
	}

	inline	set<SStringA> CDlgComboStockFilter::OutPutHisPassStock()
	{
		return m_hisFitStockSet;
	}

}