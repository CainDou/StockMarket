#pragma once
namespace SOUI
{
	class CDlgEmaPara : public SHostWnd
	{
	public:
		CDlgEmaPara();
		CDlgEmaPara(RpsGroup rg,HWND hWnd);
		~CDlgEmaPara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void OnClickButtonDefault();
		void OnClickButtonSetDefault();
		void SetEditText(const int *pPara);
	protected:
		virtual void OnFinalMessage(HWND hWnd);
		SStringW strPara1;
		SStringW strPara2;


		int nPara[2];

		RpsGroup m_rgGroup;
		HWND m_hParWnd;

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_EMAClose", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_EMAOK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_EMACancel", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_EMADefault", OnClickButtonDefault)
			EVENT_NAME_COMMAND(L"btn_EMASetDefault", OnClickButtonSetDefault)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgEmaPara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}

