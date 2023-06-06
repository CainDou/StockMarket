#pragma once
namespace SOUI
{
	class CDlgMaPara : public SHostWnd
	{
	public:
		CDlgMaPara();
		CDlgMaPara(RpsGroup rg, HWND hWnd, eMaType maType);
		~CDlgMaPara();
		void InitConfigName();
		void OnClose();
		void OnClickButtonOk();
		void OnClickButtonCancel();
		void OnInit(EventArgs *e);
		void OnClickButtonDefault();
		void OnClickButtonSetDefault();
		void SetEditText(const int *pPara);

	protected:
		virtual void OnFinalMessage(HWND hWnd);
		SStringW strPara[MAX_MA_COUNT];

		int nPara[MAX_MA_COUNT];
		int m_nDefaultPara[MAX_MA_COUNT];

		map<eMaType, SStringA> m_MaConfigName;

		RpsGroup m_rgGroup;
		HWND m_hParWnd;
		eMaType m_maType;
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			//		EVENT_ID_HANDLER(R.id.cbx_lines, EventCBSelChange::EventID, OnCbxChange)
			EVENT_NAME_COMMAND(L"btn_MaClose", OnClose)
			//	EVENT_ID_COMMAND(R.id.btn_menu, OnBtnMenu)
			EVENT_NAME_COMMAND(L"btn_MaOK", OnClickButtonOk)
			EVENT_NAME_COMMAND(L"btn_MaCancel", OnClickButtonCancel)
			EVENT_NAME_COMMAND(L"btn_MaDefault", OnClickButtonDefault)
			EVENT_NAME_COMMAND(L"btn_MaSetDefault", OnClickButtonSetDefault)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgMaPara)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}

