#pragma once
namespace SOUI
{
	class CDlgChangePara : public SHostWnd
	{
	public:
		CDlgChangePara(TargetInfo& targetInfo);
		CDlgChangePara(TargetInfo& targetInfo, HWND hParWnd);
		~CDlgChangePara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void OnClickButtonDefault();
		void OnClickButtonSetDefault();
	protected:
		virtual void OnFinalMessage(HWND hWnd);
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_Close", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_OK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_Default", OnClickButtonDefault)
			EVENT_NAME_COMMAND(L"btn_SetDefault", OnClickButtonSetDefault)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgChangePara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		TargetInfo ti;
		HWND m_hParWnd;
		std::vector<SEdit*> m_pEditVec;
		std::vector<SStatic*>m_pTextVec;
		SWindow* m_pWindowPara;
	};
}
