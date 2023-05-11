#include "stdafx.h"
#include "DlgHeaderSelect.h"
#include "AdpterFixTitle.h"
#include "Define.h"


CDlgHeaderSelect::CDlgHeaderSelect(HWND hWnd, map<int,BOOL> showMap) :SHostWnd(_T("LAYOUT:dlg_headerSelect"))
{
	m_hParWnd = hWnd;
	m_bShowMap = showMap;
}


CDlgHeaderSelect::~CDlgHeaderSelect()
{
}

BOOL SOUI::CDlgHeaderSelect::OnInitDialog(EventArgs * e)
{
	m_pLvTitle = FindChildByName2<SListView>(L"lv_title");
	InitTilteMap();
	if (m_pLvTitle)
	{
		ILvAdapter *pAdapter = new CAdpterFixTitle(m_bShowMap.size(), m_bShowMap,m_titleMap);
		m_pLvTitle->SetAdapter(pAdapter);
		pAdapter->Release();
		//CAdpterFixTitle *pTitltAdapter = (CAdpterFixTitle*)m_pLvTitle->GetAdapter();
		//OutputDebugStringFormat("开始进行设置\n");
		//for (auto &it : m_bShowMap)
		//{
		//	pTitltAdapter->SetItemChecked(it.first - SHead_CloseRPS520, it.second);
		//	pTitltAdapter->SetItemStr(it.first - SHead_CloseRPS520, m_titleMap[it.first]);
		//}

	}
	return 0;
}

void SOUI::CDlgHeaderSelect::InitTilteMap()
{
	m_titleMap[SHead_CloseRPS520] = L"收盘价rps520";
	m_titleMap[SHead_CloseMACD520] = L"收盘价MACD520";
	m_titleMap[SHead_ClosePoint520] = L"收盘价520分数";
	m_titleMap[SHead_CloseRank520] = L"收盘价520排名";
	m_titleMap[SHead_CloseRPS2060] = L"收盘价rps2060";
	m_titleMap[SHead_CloseMACD2060] = L"收盘价MACD2060";
	m_titleMap[SHead_ClosePoint2060] = L"收盘价2060分数";
	m_titleMap[SHead_CloseRank2060] = L"收盘价2060排名";
	m_titleMap[SHead_AmountRPS520] = L"成交额rps520";
	m_titleMap[SHead_AmountMACD520] = L"成交额MACD520";
	m_titleMap[SHead_AmountPoint520] = L"成交额520分数";
	m_titleMap[SHead_AmountRank520] = L"成交额520排名";
	m_titleMap[SHead_AmountRPS2060] = L"成交额rps2060";
	m_titleMap[SHead_AmountMACD2060] = L"成交额MACD2060";
	m_titleMap[SHead_AmountPoint2060] = L"成交额2060分数";
	m_titleMap[SHead_AmountRank2060] = L"成交额2060排名";
	m_titleMap[SHead_AmountPoint] = L"成交额截面分数";
	m_titleMap[SHead_AmountRank] = L"成交额截面排名";
	m_titleMap[SHead_ActBuySellRatio] = L"主动量比%";
	m_titleMap[SHead_ActToPasBuySellRatio] = L"转换主被动量比%";
	m_titleMap[SHead_AvgBuySellRatio] = L"平均量比%";
	m_titleMap[SHead_POCRatio] = L"最多成交价格比%";
	m_titleMap[SHead_Volume] = L"成交量";
	m_titleMap[SHead_ActSellVolume] = L"内盘";
	m_titleMap[SHead_ActBuyVolume] = L"外盘";
	m_titleMap[SHead_Open] = L"开盘";
	m_titleMap[SHead_High] = L"最高";
	m_titleMap[SHead_Low] = L"最低";
	m_titleMap[SHead_Amount] = L"成交额";

}

void SOUI::CDlgHeaderSelect::OnBtnClose()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgHeaderSelect::OnBtnOK()
{
	CAdpterFixTitle* pAdpter = (CAdpterFixTitle*)m_pLvTitle->GetAdapter();
	m_bShowMap = pAdpter->GetTitleShowMap();
	::SendMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_ChangeShowTilte, (LPARAM)&m_bShowMap);
	OnBtnClose();
}

BOOL SOUI::CDlgHeaderSelect::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	if (!m_pLvTitle->IsFocused())
		m_pLvTitle->SetFocus();
	return TRUE;
}

void SOUI::CDlgHeaderSelect::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
