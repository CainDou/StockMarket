#pragma once
#include<set>
namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgBackTesting;
	class CDlgComboStockFilter : public SHostWnd
	{
	public:
		CDlgComboStockFilter(HWND hParWnd,UINT uParThreadID, RpsGroup Group);
		~CDlgComboStockFilter();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnBackTesting();
		void	OnBtnSetFliterName();
		void	SetFilterName(SStringA strFilterName);
		void	InitList(bool bUse, vector<StockFilter> &sfVec);
		void	InitComboBox();
		void	OnCheckUse();
		void	StopFilter();
		void	SetStockInfo(vector<StockInfo> &stockInfo);

		void	InitStringMap();
		void	OutPutCondition(vector<StockFilter> &sfVec);
	protected:
		void	ListAddItem(StockFilter& sf);
		void	ListChangeItem(int nRow, StockFilter& sf);
		void	ListDeleteItem(int nRow);
		bool	GetListItem(int nRow, StockFilter& sf);
	protected:
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

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"chk_use", OnCheckUse)
			EVENT_NAME_COMMAND(L"btn_backTesting", OnBtnBackTesting)
			EVENT_NAME_COMMAND(L"btn_setFilterName", OnBtnSetFliterName)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgComboStockFilter)
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
	};

	inline void CDlgComboStockFilter::SetFilterName(SStringA strFilterName)
	{
		m_strFilterName = strFilterName;
	}
}