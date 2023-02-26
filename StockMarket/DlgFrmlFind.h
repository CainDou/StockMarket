#pragma once
namespace SOUI
{
	class SCapsEdit;
	class CDlgFrmlFind : public SHostWnd
	{
	public:
		CDlgFrmlFind(HWND hParWnd);
		~CDlgFrmlFind();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnClose();
		void	OnBtnUpFind();
		void	OnBtnDownFind();

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		HWND m_hParWnd;
		SCapsEdit *m_pEditKeyWord;
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_upFind", OnBtnUpFind)
			EVENT_NAME_COMMAND(L"btn_downFind", OnBtnDownFind)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgFrmlFind)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};
}