#include "stdafx.h"
#include "DlgTradeSetting.h"


CDlgTradeSetting::CDlgTradeSetting(HWND hParWnd, TradeSetting setting) : SHostWnd(_T("LAYOUT:dlg_tradeSetting"))
{
	m_hParWnd = hParWnd;
	m_setting = setting;
}


CDlgTradeSetting::~CDlgTradeSetting()
{
}

void CDlgTradeSetting::OnInit(EventArgs * e)
{
	InitControls();
}

void CDlgTradeSetting::OnBtnOK()
{
	m_setting.buyPriceType = m_pCbxBuyPrice->GetCurSel();
	m_setting.buyVolType = m_pCbxBuyVol->GetCurSel();
	m_setting.sellPriceType = m_pCbxSellPrice->GetCurSel();
	m_setting.sellVolType = m_pCbxSellVol->GetCurSel();
	m_setting.maxVol = _wtoi(m_pEditMaxVol->GetWindowTextW());
	if (m_setting.maxVol < 100 || m_setting.maxVol > 1'000'000)
	{
		SMessageBox(m_hWnd, L"最大买入量设置错误!\n需要在100-1'000'000之间!", L"错误", MB_OK);
		m_pTab->SetCurSel(0);
		m_pEditMaxVol->SetFocus();
		return;
	}
	m_setting.remindLimitPrice = m_pCbxLimitPrice->GetCurSel() == 0 ? true : false;
	m_setting.remindCagePrice = m_pCbxCagePrice->GetCurSel() == 0 ? true : false;
	m_setting.buyVolFix = _wtoi(m_pEditBuyVolFix->GetWindowTextW());
	if (m_setting.buyVolFix < 0)
		m_setting.buyVolFix = 0;
	m_setting.buyAmo = _wtof(m_pEditBuyAmo->GetWindowTextW());
	if (m_setting.buyAmo < 0)
		m_setting.buyAmo = 0;

	m_setting.afterTradeType = m_pCbxAfterTrade->GetCurSel();
	m_setting.clickVol = m_pCbxClickVol->GetCurSel() == 0 ? true : false;
	m_setting.windowTime = _wtoi(m_pEditWindowTime->GetWindowTextW());
	if (m_setting.windowTime < 0)
		m_setting.windowTime = 0;

	m_setting.showDeal = m_pCbxShowDeal->GetCurSel() == 0 ? true : false;
	m_setting.cancelConfirm = m_pCbxCancelConfirm->GetCurSel() == 0 ? true : false;
	m_setting.trustConfirm = m_pCbxTrustConfirm->GetCurSel() == 0 ? true : false;
	m_setting.showTrust = m_pCbxShowTrust->GetCurSel() == 0 ? true : false;
	m_setting.changePageClean = m_pCbxShowTrust->GetCurSel() == 0 ? true : false;

	::SendMessage(m_hParWnd, WM_TRADE_MSG, (WPARAM)&m_setting, TSMsg_ChangeSetting);
	CSimpleWnd::DestroyWindow();

}

void CDlgTradeSetting::OnBtnCancel()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgTradeSetting::InitControls()
{
	m_pTab = FindChildByName2<STabCtrl>(L"tab_ctrl");
	m_pCbxBuyPrice = FindChildByName2<SComboBox>(L"cbx_buyPrice");
	m_pCbxBuyPrice->InsertItem(0, L"卖一价", NULL, 0);
	m_pCbxBuyPrice->InsertItem(1, L"卖二价", NULL, 1);
	m_pCbxBuyPrice->InsertItem(2, L"卖三价", NULL, 2);
	m_pCbxBuyPrice->InsertItem(3, L"卖四价", NULL, 3);
	m_pCbxBuyPrice->InsertItem(4, L"卖五价", NULL, 4);
	m_pCbxBuyPrice->InsertItem(5, L"最新价", NULL, 5);
	m_pCbxBuyPrice->InsertItem(6, L"空", NULL, 6);
	m_pCbxBuyPrice->SetCurSel(m_setting.buyPriceType);

	m_pCbxBuyVol = FindChildByName2<SComboBox>(L"cbx_buyVol");
	m_pCbxBuyVol->InsertItem(0, L"空", NULL, 0);
	m_pCbxBuyVol->InsertItem(1, L"全仓", NULL, 1);
	m_pCbxBuyVol->InsertItem(2, L"1/2仓", NULL, 2);
	m_pCbxBuyVol->InsertItem(3, L"1/3仓", NULL, 3);
	m_pCbxBuyVol->InsertItem(4, L"1/4仓", NULL, 4);
	m_pCbxBuyVol->InsertItem(5, L"1/5仓", NULL, 5);
	m_pCbxBuyVol->InsertItem(6, L"1/6仓", NULL, 6);
	m_pCbxBuyVol->SetCurSel(m_setting.buyVolType);

	m_pCbxSellPrice = FindChildByName2<SComboBox>(L"cbx_sellPrice");
	m_pCbxSellPrice->InsertItem(0, L"买一价", NULL, 0);
	m_pCbxSellPrice->InsertItem(1, L"买二价", NULL, 1);
	m_pCbxSellPrice->InsertItem(2, L"买三价", NULL, 2);
	m_pCbxSellPrice->InsertItem(3, L"买四价", NULL, 3);
	m_pCbxSellPrice->InsertItem(4, L"买五价", NULL, 4);
	m_pCbxSellPrice->InsertItem(5, L"最新价", NULL, 5);
	m_pCbxSellPrice->InsertItem(6, L"空", NULL, 6);
	m_pCbxSellPrice->SetCurSel(m_setting.sellPriceType);

	m_pCbxSellVol = FindChildByName2<SComboBox>(L"cbx_sellVol");
	m_pCbxSellVol->InsertItem(0, L"空", NULL, 0);
	m_pCbxSellVol->InsertItem(1, L"全仓", NULL, 1);
	m_pCbxSellVol->InsertItem(2, L"1/2仓", NULL, 2);
	m_pCbxSellVol->InsertItem(3, L"1/3仓", NULL, 3);
	m_pCbxSellVol->InsertItem(4, L"1/4仓", NULL, 4);
	m_pCbxSellVol->InsertItem(5, L"1/5仓", NULL, 5);
	m_pCbxSellVol->InsertItem(6, L"1/6仓", NULL, 6);
	m_pCbxSellVol->SetCurSel(m_setting.sellVolType);

	SStringW str;
	m_pEditMaxVol = FindChildByName2<SEdit>(L"edit_maxVol");
	m_pEditMaxVol->SetWindowTextW(str.Format(L"%d", m_setting.maxVol));

	m_pCbxLimitPrice = FindChildByName2<SComboBox>(L"cbx_limitPrice");
	m_pCbxLimitPrice->InsertItem(0, L"是", NULL, 1);
	m_pCbxLimitPrice->InsertItem(1, L"否", NULL, 0);
	m_pCbxLimitPrice->SetCurSel(m_setting.remindLimitPrice ? 0 : 1);

	m_pCbxCagePrice = FindChildByName2<SComboBox>(L"cbx_cagePrice");
	m_pCbxCagePrice->InsertItem(0, L"是", NULL, 1);
	m_pCbxCagePrice->InsertItem(1, L"否", NULL, 0);
	m_pCbxCagePrice->SetCurSel(m_setting.remindCagePrice ? 0 : 1);

	m_pEditBuyVolFix = FindChildByName2<SEdit>(L"edit_buyVolFix");
	if(m_setting.buyVolFix != 0)
		m_pEditBuyVolFix->SetWindowTextW(str.Format(L"%d", m_setting.buyVolFix));

	m_pEditBuyAmo = FindChildByName2<SEdit>(L"edit_buyAmo");
	if (m_setting.buyAmo != 0)
		m_pEditBuyAmo->SetWindowTextW(str.Format(L"%g", m_setting.buyAmo));

	m_pCbxAfterTrade = FindChildByName2<SComboBox>(L"cbx_aftreTrade");
	m_pCbxAfterTrade->InsertItem(0, L"清空", NULL, 0);
	m_pCbxAfterTrade->InsertItem(1, L"保留代码", NULL, 1);
	m_pCbxAfterTrade->InsertItem(2, L"保留代码、价格", NULL, 2);
	m_pCbxAfterTrade->InsertItem(3, L"保留代码、价格、数量", NULL, 3);
	m_pCbxAfterTrade->SetCurSel(m_setting.afterTradeType);

	m_pCbxClickVol = FindChildByName2<SComboBox>(L"cbx_clickVol");
	if (m_pCbxClickVol)
	{
		m_pCbxClickVol->InsertItem(0, L"是", NULL, 1);
		m_pCbxClickVol->InsertItem(1, L"否", NULL, 0);
		m_pCbxClickVol->SetCurSel(m_setting.clickVol ? 0 : 1);
	}

	m_pEditWindowTime = FindChildByName2<SEdit>(L"edit_windowTime");
	if (m_pEditWindowTime)
	{
		if (m_setting.windowTime != 0)
			m_pEditWindowTime->SetWindowTextW(str.Format(L"%d", m_setting.windowTime));
	}

	m_pCbxShowDeal = FindChildByName2<SComboBox>(L"cbx_showDeal");
	m_pCbxShowDeal->InsertItem(0, L"是", NULL, 1);
	m_pCbxShowDeal->InsertItem(1, L"否", NULL, 0);
	m_pCbxShowDeal->SetCurSel(m_setting.showDeal ? 0 : 1);

	m_pCbxCancelConfirm = FindChildByName2<SComboBox>(L"cbx_cancelConfirm");
	m_pCbxCancelConfirm->InsertItem(0, L"是", NULL, 1);
	m_pCbxCancelConfirm->InsertItem(1, L"否", NULL, 0);
	m_pCbxCancelConfirm->SetCurSel(m_setting.cancelConfirm ? 0 : 1);

	m_pCbxTrustConfirm = FindChildByName2<SComboBox>(L"cbx_trustConfirm");
	if (m_pCbxTrustConfirm)
	{
		m_pCbxTrustConfirm->InsertItem(0, L"是", NULL, 1);
		m_pCbxTrustConfirm->InsertItem(1, L"否", NULL, 0);
		m_pCbxTrustConfirm->SetCurSel(m_setting.trustConfirm ? 0 : 1);

	}

	m_pCbxShowTrust = FindChildByName2<SComboBox>(L"cbx_showTrust");
	m_pCbxShowTrust->InsertItem(0, L"是", NULL, 1);
	m_pCbxShowTrust->InsertItem(1, L"否", NULL, 0);
	m_pCbxShowTrust->SetCurSel(m_setting.showTrust ? 0 : 1);

	m_pCbxChangePage = FindChildByName2<SComboBox>(L"cbx_changePage");
	m_pCbxChangePage->InsertItem(0, L"是", NULL, 1);
	m_pCbxChangePage->InsertItem(1, L"否", NULL, 0);
	m_pCbxChangePage->SetCurSel(m_setting.changePageClean ? 0 : 1);

}

void SOUI::CDlgTradeSetting::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
