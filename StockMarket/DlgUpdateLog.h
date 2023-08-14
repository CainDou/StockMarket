#pragma once
namespace SOUI
{
	class CDlgUpdateLog: public SHostWnd
	{
	public:
		CDlgUpdateLog();
		~CDlgUpdateLog();
		BOOL	OnInitDialog(EventArgs* e);
		void	OnBtnOK();
	protected:
		virtual void OnFinalMessage(HWND hWnd);
		void UpdateLog();

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)

			EVENT_MAP_END()
			BEGIN_MSG_MAP_EX(CDlgUpdateLog)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		SRichEdit* m_pLogEdit;
	};
}

