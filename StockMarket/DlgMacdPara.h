#pragma once
namespace SOUI
{
	class CDlgMacdPara : public SHostWnd
	{
	public:
		CDlgMacdPara();
		CDlgMacdPara(RpsGroup rg, HWND hWnd);
		~CDlgMacdPara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void OnClickButtonDefault();
		void OnClickButtonSetDefault();
		void SetEditText(const int *pPara);

		SStringW strPara1;
		SStringW strPara2;
		SStringW strPara3;

		RpsGroup m_rgGroup;
		HWND m_hParWnd;

		int m_nMacdPara[3];


	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_MacdClose", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_MACDOK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_MACDCancel", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_MACDDefault", OnClickButtonDefault)
			EVENT_NAME_COMMAND(L"btn_MACDSetDefault", OnClickButtonSetDefault)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgMacdPara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}

