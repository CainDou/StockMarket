#pragma once
class CDlgTrustConfirm : public SHostDialog
{
public:
	CDlgTrustConfirm(TrustInfo trustInfo);
	~CDlgTrustConfirm();

	void OnInit(EventArgs *e);

	void OnBtnOK();
	void OnBtnCancel();

protected:
	EVENT_MAP_BEGIN()
		EVENT_HANDLER(EventInit::EventID, OnInit)
		EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
		EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnCancel)

		EVENT_MAP_END()

		//HostWnd��ʵ������Ϣ����
		BEGIN_MSG_MAP_EX(CDlgTrustConfirm)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

protected:
	TrustInfo m_trustInfo;

};

