#pragma once
namespace SOUI
{

	class DlgBrickPara : public SHostWnd
	{
	public:

		DlgBrickPara(HWND hParWnd);
		~DlgBrickPara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs* e);
		void SetPara(int nType, double fPara);

		HWND m_hParWnd;

		int m_nMacdPara[3];


	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_Close", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_OK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnClickButtonCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(DlgBrickPara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	};

}