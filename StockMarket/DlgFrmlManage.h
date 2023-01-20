#pragma once
#include "FrmlManager.h"
namespace SOUI
{
	class CDlgFrmlManage : public SHostWnd
	{
	public:
		CDlgFrmlManage(HWND hParWnd);
		~CDlgFrmlManage();

	protected:
		BOOL OnInitDialog(EventArgs* e);
		BOOL InitShowText();
		BOOL UpdateFrmlTree();
		bool OnTreeFrmlSelChanged(EventArgs* e);
		void OnBtnNew();
		void OnBtnChange();
		void OnBtnDelete();
		void OnBtnClose();
		LRESULT OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_New", OnBtnNew)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Change", OnBtnChange)
			EVENT_NAME_COMMAND(L"btn_Delete", OnBtnDelete)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgFrmlManage)
			MESSAGE_HANDLER(WM_FORMULA_MSG, OnMsg)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		STreeCtrl * m_pTreeFrml;
		map<eFrmlUse, SStringW> m_FrmlUseShowText;
		map<eStkFilterFrmlType, SStringW> m_FilterFrmlShowText;
		map<eFrmlUse, HSTREEITEM>m_FrmlUseNodeMap;
		map<int, HSTREEITEM>m_SubdivNodeMap;
		map<string, FrmlFullInfo> m_FrmlMap;
		map<HSTREEITEM, string> m_FrmlPosMap;
		string	 m_ChangeFrmlName;
		SButton*  m_pBtnNew;
		SButton*  m_pBtnChange;
		SButton*  m_pBtnDelete;
		HWND	  m_hParWnd;
	};

}

