#pragma once
#include "FrmlCompiler.h"
namespace SOUI
{
	class SColorListCtrlEx;
	class SCapsEdit;
	class CDlgFuncInsert : public SHostWnd
	{
	public:
		CDlgFuncInsert(HWND hWnd);
		~CDlgFuncInsert();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnFind();
		bool	OnLbClassClicked(EventArgs *pEvtBase);
		bool	OnLcFuncClicked(EventArgs *pEvtBase);
		void	UpdateList(eFuncType FtClass);

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Find", OnBtnFind)

			EVENT_MAP_END()
			BEGIN_MSG_MAP_EX(CDlgFuncInsert)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SListBox * m_pLbClass;
		SColorListCtrlEx* m_pLcFunc;
		SCapsEdit* m_pEditFinder;
		SEdit* m_pEditDscp;
	protected:
		HWND m_hParWnd;
		map<eFuncType, vector<FuncInfo>> m_FuncInfoMap;
		eFuncType m_preFuncClass;
		BOOL m_bFindOver;
		map<eFuncType, set<int>>m_FinderMap;
		string m_preFindStr;
	};

}