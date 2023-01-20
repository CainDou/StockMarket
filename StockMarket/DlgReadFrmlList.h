#pragma once
namespace SOUI
{
	class SColorListCtrlEx;
	class CDlgReadFrmlList : public SHostWnd
	{
	public:
		CDlgReadFrmlList(HWND hParWnd);
		~CDlgReadFrmlList();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnCancel();
		void	OnBtnDelete();

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		HWND m_hParWnd;
		SColorListCtrlEx *m_pListPlanName;
		BOOL m_bDelete;
		vector<string> m_palnNameVec;
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
			EVENT_NAME_COMMAND(L"btn_delete", OnBtnDelete)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgReadFrmlList)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	};

}