#pragma once
namespace SOUI
{
	class DlgPendantPara : public SHostWnd
	{
	public:

		DlgPendantPara(HWND hParWnd);
		~DlgPendantPara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs* e);
		void SetPara( double fPara);

		HWND m_hParWnd;

		int m_nMacdPara[3];


	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_Close", OnClose)
			EVENT_NAME_COMMAND(L"btn_OK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnClickButtonCancel)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(DlgPendantPara)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	};

}