#include "stdafx.h"
#include "DlgBrickPara.h"

SOUI::DlgBrickPara::DlgBrickPara(HWND hParWnd) : SHostWnd(_T("LAYOUT:dlg_brickPara"))
{
	m_hParWnd = hParWnd;
}

SOUI::DlgBrickPara::~DlgBrickPara()
{
}

void SOUI::DlgBrickPara::OnClose()
{
	CSimpleWnd::DestroyWindow();
}
void SOUI::DlgBrickPara::OnInit(EventArgs* e)
{

}


void SOUI::DlgBrickPara::OnClickButtonOk()
{
	SEdit* para1 = FindChildByName2<SEdit>(L"edit_para");
	SStringW strPara = para1->GetWindowTextW();
	SComboBox* pCmb = FindChildByName2<SComboBox>(L"cbx_type");

	std::pair<int, double>* pPara = new std::pair<int, double>;
	pPara->first = pCmb->GetCurSel();
	double fSetting = _wtof(strPara.GetBuffer(1));
	if (pPara->first == 0)
		pPara->second = max(1, fSetting);
	else
		pPara->second = max(0.01, fSetting);
	::SendMessageW(m_hParWnd, WM_BRICK_MSG, (WPARAM)pPara, BRICKMSG_CHANGEPARA);
	OnClose();
}

void SOUI::DlgBrickPara::SetPara(int nType, double fPara)
{
	SComboBox* pCmb = FindChildByName2<SComboBox>(L"cbx_type");
	pCmb->SetCurSel(nType);
	SEdit* editPara = FindChildByName2<SEdit>(L"edit_para");
	SStringW strTmp;
	editPara->SetWindowTextW(strTmp.Format(L"%g", fPara));
}

void SOUI::DlgBrickPara::OnClickButtonCancel()
{
	OnClose();
}

void SOUI::DlgBrickPara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
