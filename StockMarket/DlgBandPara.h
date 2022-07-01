#pragma once

namespace SOUI
{
	class CDlgBandPara : public SHostWnd
	{
	public:
		CDlgBandPara();
		CDlgBandPara(RpsGroup rg, HWND hWnd);
		~CDlgBandPara();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void OnClickButtonDefault();
		void OnClickButtonSetDefault();
		void SetEditText(BandPara_t para);

		SStringW strParaN1;
		SStringW strParaN2;
		SStringW strParaK;
		SStringW strParaM1;
		SStringW strParaM2;
		SStringW strParaP;

		RpsGroup m_rgGroup;
		BandPara_t m_BandPara;
		HWND m_hParWnd;

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_BandClose", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_BandOK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_BandDefault", OnClickButtonDefault)
			EVENT_NAME_COMMAND(L"btn_BandCancel", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_BandSetDefault", OnClickButtonSetDefault)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgBandPara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}
