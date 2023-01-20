#pragma once
#include<set>
#include <fstream>
#include "FrmlManager.h"

namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgStockFilter : public SHostWnd
	{
	public:
		CDlgStockFilter(UINT uParThreadID,RpsGroup Group);
		~CDlgStockFilter();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	InitList(bool bUse, SFPlan& sfPlan);
		void	InitComboBox();
		void	InitFrmlCombox();
		void	InitStringMap();
		void	OutPutCondition(SFPlan& sfPlan);
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		CRect   GetLastCharRect(IRenderTarget*pRT, SStringW str,CRect clinetRC,int nLineHei);
		static void	SaveConditonsList(std::ofstream &fs, SFPlan &sfPlan);
		static void	ReadConditonsList(std::ifstream &fs, SFPlan &sfPlan);

	protected:
		void	ListAddItem(SFCondition& sf);
		bool	GetListItem(int nRow, SFCondition& sf);
		void	SearchFrml(int nDirect, SStringW strKey);
		void	ChangeParaSetting(int nSel);

	protected:
		bool	OnCbxFrmlChange(EventArgs * e);
		void	OnBtnAddConditon();
		void	OnBtnDeleteCondition();
		void	OnBtnFind();
		void	OnBtnUsage();
		void	OnBtnEdit();
		void	OnBtnRead();
		void	OnBtnSave();
		void	OnRadioCliecked();


	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Find", OnBtnFind)
			EVENT_NAME_COMMAND(L"btn_Usage", OnBtnUsage)
			EVENT_NAME_COMMAND(L"btn_Edit", OnBtnEdit)
			EVENT_NAME_COMMAND(L"btn_Add", OnBtnAddConditon)
			EVENT_NAME_COMMAND(L"btn_Delete", OnBtnDeleteCondition)
			EVENT_NAME_COMMAND(L"btn_Save", OnBtnSave)
			EVENT_NAME_COMMAND(L"btn_Read", OnBtnRead)
			EVENT_NAME_COMMAND(L"rdb_and", OnRadioCliecked)
			EVENT_NAME_COMMAND(L"rdb_or", OnRadioCliecked)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgStockFilter)
			MESSAGE_HANDLER(WM_FILTER_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()


	protected:
		SCheckBox*	  m_pCheckUse;
		SColorListCtrlEx* m_pList;
		SComboBox*	  m_pCbxFrml;
		SComboBox*	  m_pCbxPeriod;
		SRadioBox*	  m_pRdbAnd;
		SRadioBox*	  m_pRdbOr;
		SStatic*		m_pTextParaTitle;
		//SStatic*	  m_pTextParaStart[16];
		SEdit*		  m_pEditPara[16];
		//SStatic*	  m_pTextParaEnd[16];

	protected:
		HWND m_hParWnd;
		SStringA m_strParWnd;
		RpsGroup	m_Group;
		map<int, SStringW> m_PeriodMap;
		vector<FrmlFullInfo> m_FrmlVec;
		SFPlan m_sfPlan;
		SFPlan m_tmpSFPlan;
		UINT m_uParThreadID;
		BOOL m_bCondsChanged;
		BOOL m_bUseSF;
	};


}