#pragma once
class CDlgTrustConfirm : public SHostDialog
{
public:
	CDlgTrustConfirm(TrustInfo trustInfo);
	~CDlgTrustConfirm();

	void OnInit(EventArgs *e);

	void OnBtnOK();
	void OnBtnCancel();
	void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
protected:
	EVENT_MAP_BEGIN()
		EVENT_HANDLER(EventInit::EventID, OnInit)
		EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
		EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnCancel)

		EVENT_MAP_END()

		//HostWnd真实窗口消息处理
		BEGIN_MSG_MAP_EX(CDlgTrustConfirm)
		MSG_WM_KEYDOWN(OnKeyDown)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

protected:
	TrustInfo m_trustInfo;
};

