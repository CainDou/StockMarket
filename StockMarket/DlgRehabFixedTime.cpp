#include "stdafx.h"
#include "DlgRehabFixedTime.h"


CDlgRehabFixedTime::CDlgRehabFixedTime(HWND hWnd,eRehabType rehabType) :SHostWnd(_T("LAYOUT:dlg_rehabFixedTime"))
{
	m_hParWnd = hWnd;
	m_RehabType = rehabType;
}


CDlgRehabFixedTime::~CDlgRehabFixedTime()
{
}

void SOUI::CDlgRehabFixedTime::OnClose()
{ 
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgRehabFixedTime::OnClickButtonOk()
{
	FixedTimeRehab ftr = { 0 };
	SYSTEMTIME st;
	m_pDtpRehabDate->GetTime(st);
	ftr.nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	ftr.bFrontRehab = m_pRadioFrontRehab->IsChecked();
	ftr.Type = m_RehabType;
	::SendMessage(m_hParWnd, WM_KLINE_MSG, (WPARAM)&ftr, KLINEMSG_REHAB);
	OnClose();
}

void SOUI::CDlgRehabFixedTime::OnClickButtonCancel()
{
	OnClose();
}

void SOUI::CDlgRehabFixedTime::OnInit(EventArgs * e)
{
	m_pDtpRehabDate = FindChildByName2<SDateTimePicker>(L"dtp_rehab");
	m_pRadioFrontRehab = FindChildByName2<SRadioBox>(L"radio_FrontRehab");
	m_pRadioBackRehab = FindChildByName2<SRadioBox>(L"radio_BackRehab");
}

void SOUI::CDlgRehabFixedTime::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
