#pragma once

#include "FrmlCompiler.h"
#include "SCapsRichEdit.h"
#include "FrmlManager.h"



namespace SOUI
{
	class CDlgFuncInsert;
	class CDlgStkFilterEditor : public SHostWnd
	{
	public:
		CDlgStkFilterEditor(HWND hParWnd,int nSubdiv);
		CDlgStkFilterEditor(HWND hParWnd ,FrmlFullInfo &info,BOOL bFromStkFilter);
		~CDlgStkFilterEditor();
		FrmlFullInfo	GetFrmlInfo();

	protected:
		BOOL	OnInitDialog(EventArgs* e);
		bool	OnEditFrmlChange(EventArgs *e);
		virtual void OnFinalMessage(HWND hWnd);
	protected:
		void	InitSubControl();
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);
		//	文版编辑器功能
		BOOL	SetExplianType(SCapsRichEdit* pEdit,DWORD dwStart,DWORD dwEnd);
		BOOL	UpdateStrFormat(SCapsRichEdit* pEdit, SStringW& str);
		BOOL	SetStrFormatDefault(SCapsRichEdit* pEdit,DWORD dwStart, DWORD dwEnd);
		BOOL	SetEleColor(SCapsRichEdit* pEdit,eEleType et, DWORD dwStart, DWORD dwEnd);
		SStringW	TransFrml(SStringW& str);
		//函数测试
		BOOL	FrmlTest();
		BOOL	CheckParam();
		//其他功能
		BOOL	GenerateParaElf();
		SStringW GetFrmlTrans(int nFuncNum,map<int, string>& TransMsg, 
			map<int, int>&funcParaNumMap,map<int, map<int, vector<int>>>& funcParaMap);
	protected:
		void	OnBtnOK();
		void	OnBtnClose();
		void	OnBtnInsertFunc();
		void	OnBtnTest();

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Insert", OnBtnInsertFunc)
			EVENT_NAME_COMMAND(L"btn_Test", OnBtnTest)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgStkFilterEditor)
			MESSAGE_HANDLER(WM_FUNC_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		CDlgFuncInsert* m_pDlgInsert;
		SCapsEdit* m_pEditName;
		SCapsEdit* m_pEditDscp;
		SCapsEdit* m_pEditParaName[16];
		SCapsEdit* m_pEditParaMin[16];
		SCapsEdit* m_pEditParaMax[16];
		SCapsEdit* m_pEditParaDef[16];
		SComboBox* m_pCbxType;
		SCapsEdit* m_pEditTrans;
		SCapsEdit* m_pEditResult;
		SCapsEdit* m_pEditParam;
		SCapsEdit* m_PEditExplain;
		SCapsRichEdit* m_pReFrml;
		STabCtrl* m_pTabCtrl;
		CFrmlCompiler m_FrmlCmpl;
		FrmlFullInfo  m_FrmlInfo;
		set<string>	  m_ParaSet;
		BOOL m_bDirty;
		BOOL m_bNew;
		int m_nSubdiv;
		BOOL bChangeFromInit;
		HWND m_hParWnd;
		BOOL m_bFromStkFilter;

	};

	inline FrmlFullInfo CDlgStkFilterEditor::GetFrmlInfo()
	{
		return m_FrmlInfo;
	}
}

