#pragma once
#include<map>
#include<set>
using std::set;
namespace SOUI
{
	class CDlgOpenWnd :public SHostWnd
	{
	public:
		CDlgOpenWnd(HWND hParWnd,int nWndType);
		~CDlgOpenWnd();
		void OnInit(EventArgs* e);
		void InitStrings();
		void OnBtnOK();
		void OnBtnCancel();
	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		SStringA m_strNewWndName;
		SStringW m_strDefaultName;
		int nDefaultWndCount;
		int nNameCount;
		set<SStringA> NameSet;
		map<int, int> ItemdNumMap;
		HWND m_hParWnd;
		SComboBox *pCbx;
		int m_nWndType;
		map<int, SStringA> m_WndTypeSection;
		SStringA m_strSection;

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgOpenWnd)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};
}

