#include "stdafx.h"
#include "DlgPendantPara.h"

SOUI::DlgPendantPara::DlgPendantPara(HWND hParWnd) : SHostWnd(_T("LAYOUT:dlg_pendantPara"))
{
	m_hParWnd = hParWnd;
}

SOUI::DlgPendantPara::~DlgPendantPara()
{
}

void SOUI::DlgPendantPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}
void SOUI::DlgPendantPara::OnInit(EventArgs* e)
{

}


void SOUI::DlgPendantPara::OnClickButtonOk()
{
	SEdit* para1 = FindChildByName2<SEdit>(L"edit_para");
	SStringW strPara = para1->GetWindowTextW();

	double fSetting = _wtof(strPara.GetBuffer(1));
	::SendMessageW(m_hParWnd, WM_BRICK_MSG, (WPARAM)fSetting, BRICKMSG_CHANGEPENDANT);
	OnClose();
}

void SOUI::DlgPendantPara::SetPara(double fPara)
{
	SEdit* editPara = FindChildByName2<SEdit>(L"edit_para");
	SStringW strTmp;
	editPara->SetWindowTextW(strTmp.Format(L"%g", fPara));
}

void SOUI::DlgPendantPara::OnClickButtonCancel()
{
	OnClose();
}

void SOUI::DlgPendantPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
