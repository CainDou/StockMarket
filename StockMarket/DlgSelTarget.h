#pragma once
#include "Define.h"
namespace SOUI
{
	class CDlgSelTarget : public SHostWnd
	{
	public:
		CDlgSelTarget(HWND hWnd, map<int, ShowPointInfo>& pointMap,int nPeriod);
		~CDlgSelTarget();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnFind();
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
			BEGIN_MSG_MAP_EX(CDlgSelTarget)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SListBox * m_pLbTarget;
		SEdit* m_pEditFinder;
	protected:
		HWND m_hParWnd;
		map<int, ShowPointInfo> m_PointInfoMap;
		map<int, int> m_itemPointData;
		BOOL m_bFindOver;
		map<int, int>m_FinderMap;
		SStringA m_preFindStr;
		int m_nPeriod;

	};

}

