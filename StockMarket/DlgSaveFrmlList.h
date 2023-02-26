#pragma once
namespace SOUI
{
	class CDlgSaveFrmlList : public SHostWnd
	{
	public:
		CDlgSaveFrmlList(HWND hParWnd);
		~CDlgSaveFrmlList();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnOK();
		void	OnBtnCancel();

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		HWND m_hParWnd;
		SEdit *m_pEditFileName;
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgSaveFrmlList)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	};

}

