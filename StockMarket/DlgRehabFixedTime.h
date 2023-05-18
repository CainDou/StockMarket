#pragma once
namespace SOUI
{
	class CDlgRehabFixedTime : public SHostWnd
	{
	public:
		CDlgRehabFixedTime(HWND hWnd,eRehabType rehabType);
		~CDlgRehabFixedTime();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		HWND m_hParWnd;
		eRehabType m_RehabType;
		SDateTimePicker *m_pDtpRehabDate;
		SRadioBox *m_pRadioFrontRehab;
		SRadioBox *m_pRadioBackRehab;

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_Close", OnClose)
			EVENT_NAME_COMMAND(L"btn_OK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnClickButtonCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgRehabFixedTime)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};
}


