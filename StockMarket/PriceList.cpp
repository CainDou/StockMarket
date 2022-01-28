#include "stdafx.h"
#include"PriceList.h"
#include <unordered_map>
#include<map>
extern CRITICAL_SECTION g_csGroupTick;

CPriceList::CPriceList()
{
	m_strSubIns = "";
//	m_nIndex = -1;
	m_bInit = FALSE;
	m_bIsStockIndex = false;
}

void CPriceList::SetShowData(SStringA StockID, vector<CommonStockMarket>* pStkMarketVec)
{
	m_strSubIns = StockID;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
	if (m_strSubIns[0] == '3' || m_strSubIns.Find("688") != -1)
		m_fMaxChgPct = 0.2;
	else
		m_fMaxChgPct = 0.1;
}

void CPriceList::SetShowData(SStringA StockID, vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_strSubIns = StockID;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
}


CPriceList::~CPriceList()
{
}

void SOUI::CPriceList::Paint(IRenderTarget * pRT)
{

	return;
	if (!m_bInit)
	{
		m_bInit = TRUE;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 255), 2, &m_penRed);

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 15;// 字体大小
		_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
		GETRENDERFACTORY->CreateFont(&m_pFont15, lf);
		lf.lfHeight = 20;// 字体大小
		GETRENDERFACTORY->CreateFont(&m_pFont20, lf);

	}

			if (!m_bIsStockIndex)
			{
				if(!m_pStkMarketVec->empty())
					m_StockTick = m_pStkMarketVec->back();
				DrawStockModeOne(pRT);
			}
			else
			{
				if (!m_pIdxMarketVec->empty())
					m_IndexTick = m_pIdxMarketVec->back();
				DrawIndexModeOne(pRT);
			}

}


void CPriceList::DrawStockModeOne(IRenderTarget * pRT)
{
	CPoint point[5];
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nWidth = (m_rect.Width() - 10) / 4;
	if (nWidth < 10)
		return;
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penRed);
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont20, (IRenderObj**)&OldFont);

	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	SStringW strTmp =StrA2StrW(m_strSubIns);
	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + 40, m_rect.top, m_rect.left + nWidth * 2 + 80, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	strTmp = L"";
	//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top, m_rect.right, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SelectObject(OldFont);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH + 4);
	point[1].SetPoint(m_rect.right+5, m_rect.top + RC_PRH + 4);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"卖", wcslen(L"卖"), CRect(m_rect.left + 10, m_rect.top + RC_PRH + 5, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买", wcslen(L"买"), CRect(m_rect.left + 10, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 3), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 2);
	point[1].SetPoint(m_rect.right+5, m_rect.top + (RC_PRH + 5) * 2);
	pRT->DrawLines(point, 2);

	//量
	_swprintf(szTmp, L"%d", m_StockTick.AskVolume[0]);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	_swprintf(szTmp, L"%d", m_StockTick.BidVolume[0]);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//买一卖一价
	pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[0]));
	if (m_StockTick.AskPrice[0] > 10000000)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[0]);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[0]));
	if (m_StockTick.BidPrice[0] > 10000000)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[0]);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 3);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH + 5) * 3);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨停", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"昨收", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20), DT_LEFT);
	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"跌停", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"昨结", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	pRT->SetTextColor(GetTextColor(m_StockTick.LastPrice));
	if (m_StockTick.LastPrice > 10000000 || m_StockTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice - m_StockTick.PreCloPrice);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%", (m_StockTick.LastPrice - m_StockTick.PreCloPrice) / (m_StockTick.PreCloPrice) * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice * (1+ m_fMaxChgPct));	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_StockTick.LowPrice));
	if (m_StockTick.LowPrice > 10000000 || m_StockTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice * (1-m_fMaxChgPct));	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//昨结
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);
	//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	//	pRT->DrawTextW(L"--", 2, CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH)* 7 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH)* 8 + 20), DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH) * 8 + 17);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH) * 8 + 17);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = m_rect.top + (RC_PRH + 5) * 3; i < m_rect.top + (RC_PRH) * 8 + 17; i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
}

void SOUI::CPriceList::DrawIndexModeOne(IRenderTarget * pRT)
{
	CPoint point[5];
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nWidth = (m_rect.Width() - 10) / 4;
	if (nWidth < 10)
		return;
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penRed);
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont20, (IRenderObj**)&OldFont);

	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	SStringW strTmp = StrA2StrW(m_strSubIns);
	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + 40, m_rect.top, m_rect.left + nWidth * 2 + 80, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	strTmp = L"";
	//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top, m_rect.right, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SelectObject(OldFont);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH + 4);
	point[1].SetPoint(m_rect.right+5, m_rect.top + RC_PRH + 4);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"卖", wcslen(L"卖"), CRect(m_rect.left + 10, m_rect.top + RC_PRH + 5, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买", wcslen(L"买"), CRect(m_rect.left + 10, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 3), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 2);
	point[1].SetPoint(m_rect.right+5, m_rect.top + (RC_PRH + 5) * 2);
	pRT->DrawLines(point, 2);

	//量
	_swprintf(szTmp, L"-");
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	_swprintf(szTmp, L"-");
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//买一卖一价
	pRT->SetTextColor(RGBA(255, 255, 0, 0));
	_swprintf(szTmp, L"-");
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	_swprintf(szTmp, L"-");
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 3);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH + 5) * 3);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨停", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"昨收", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20), DT_LEFT);
	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"跌停", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"昨结", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	pRT->SetTextColor(GetTextColor(m_IndexTick.LastPrice));
	if (m_IndexTick.LastPrice > 10000000 || m_IndexTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_IndexTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%.2f", m_IndexTick.Change);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%", m_IndexTick.ChangePct * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));
	_swprintf(szTmp, L"-");	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%.02f", m_IndexTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_IndexTick.OpenPrice));
	if (m_IndexTick.OpenPrice > 10000000 || m_IndexTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_IndexTick.HighPrice));
	if (m_IndexTick.HighPrice > 10000000 || m_IndexTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_IndexTick.LowPrice));
	if (m_IndexTick.LowPrice > 10000000 || m_IndexTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"-");	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"-");	//昨结
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);
	//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	//	pRT->DrawTextW(L"--", 2, CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH)* 7 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH)* 8 + 20), DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH) * 8 + 17);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH) * 8 + 17);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = m_rect.top + (RC_PRH + 5) * 3; i < m_rect.top + (RC_PRH) * 8 + 17; i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
}



COLORREF CPriceList::GetTextColor(double price)
{
	if (!m_bIsStockIndex)
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_StockTick.PreCloPrice)
			return RGBA(255, 0, 0, 255);
		else if (price < m_StockTick.PreCloPrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
	else
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_IndexTick.PreCloPrice)
			return RGBA(255, 0, 0, 255);
		else if (price < m_IndexTick.PreCloPrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
}


