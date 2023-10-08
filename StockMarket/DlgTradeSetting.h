#pragma once
namespace SOUI
{
	class CDlgTradeSetting : public SHostWnd
	{
	public:
		CDlgTradeSetting(HWND hParWnd,TradeSetting setting);
		~CDlgTradeSetting();
		void OnInit(EventArgs* e);
		void OnBtnOK();
		void OnBtnCancel();
	protected:
		void InitControls();
		virtual void OnFinalMessage(HWND hWnd);
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnCancel)

			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgTradeSetting)
			//	COMMAND_ID_HANDLER_EX(102, OnMenuCmd)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	protected:
		STabCtrl *m_pTab;
		SComboBox *m_pCbxBuyPrice;
		SComboBox *m_pCbxBuyVol;
		SComboBox *m_pCbxSellPrice;
		SComboBox *m_pCbxSellVol;
		SEdit *m_pEditMaxVol;
		SComboBox *m_pCbxLimitPrice;
		SComboBox *m_pCbxCagePrice;
		SEdit *m_pEditBuyVolFix;
		SEdit *m_pEditBuyAmo;
		SComboBox *m_pCbxAfterTrade;
		SComboBox *m_pCbxClickVol;
		SEdit *m_pEditWindowTime;
		SComboBox *m_pCbxShowDeal;
		SComboBox *m_pCbxCancelConfirm;
		SComboBox *m_pCbxTrustConfirm;
		SComboBox *m_pCbxShowTrust;
		SComboBox *m_pCbxChangePage;
	protected:
		HWND m_hParWnd;
		TradeSetting m_setting;
	};
}


