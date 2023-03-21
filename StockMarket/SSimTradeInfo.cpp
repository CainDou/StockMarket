#include "stdafx.h"
#include "SSimTradeInfo.h"

#define SPACE 5
#define RC_PRH 20
#define VPOS(x) (m_rectPrice.top + RC_PRH * x)

SSimTradeInfo::SSimTradeInfo()
{
	m_bInit = FALSE;
	m_bDataInited = FALSE;
	m_nOffset = 0;
	m_bUpdateDeal = TRUE;
}


SSimTradeInfo::~SSimTradeInfo()
{
}

void SOUI::SSimTradeInfo::SetShowData(SStringA StockID, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec)
{
	m_bDataInited = FALSE;
	m_strStockID = StockID;
	m_pStkMarketVec = pStkMarketVec;
	m_strStockName = StockName;
	if (m_strStockName.Find("ST") != -1)
		m_fMaxChgPct = 0.05;
	else if (m_strStockID[0] == '3' || m_strStockID.Find("688") != -1)
		m_fMaxChgPct = 0.2;
	else
		m_fMaxChgPct = 0.1;
	m_bDataInited = TRUE;

}


void SOUI::SSimTradeInfo::OnPaint(IRenderTarget * pRT)
{
	if (!m_bInit)
	{
		m_bInit = TRUE;
		pRT->CreatePen(PS_SOLID, RGBA(255, 31, 31, 255), 2, &m_penRed);

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 15;// 字体大小
		_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
		GETRENDERFACTORY->CreateFont(&m_pFont15, lf);
		lf.lfHeight = 20;// 字体大小
		GETRENDERFACTORY->CreateFont(&m_pFont20, lf);
		lf.lfHeight = 12;// 字体大小
		GETRENDERFACTORY->CreateFont(&m_pFont10, lf);

	}
	CRect rc;
	SWindow::GetClientRect(&rc);
	m_rectPrice.SetRect(rc.left,
		rc.top, rc.right, rc.top + 580);
	m_rectDeal.SetRect(rc.left,
		rc.top + 585, rc.right, rc.bottom);

	if (m_bDataInited)
	{
		if (!m_pStkMarketVec->empty())
			m_StockTick = m_pStkMarketVec->back();
		else
			m_StockTick = CommonStockMarket{ 0 };
		if (m_pStkMarketVec->size() > 1)
			m_preStockTick =
			m_pStkMarketVec->at(m_pStkMarketVec->size() - 2);
		else
			m_preStockTick = CommonStockMarket{ 0 };
		DrawPrice(pRT);
		DrawDeal(pRT);

	}
}

void SOUI::SSimTradeInfo::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (pt.y >= m_rectPrice.top && pt.y <= m_rectPrice.bottom)
		m_bClickPrice = TRUE;
	else
		m_bClickPrice = FALSE;
}

void SOUI::SSimTradeInfo::OnDestroy()
{
	__super::OnDestroy();
}

void SOUI::SSimTradeInfo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

}

void SOUI::SSimTradeInfo::OnSize(UINT nType, CSize size)
{
	__super::OnSize(nType, size);
}

void SOUI::SSimTradeInfo::DrawPrice(IRenderTarget * pRT)
{
	CPoint point[5];
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rectPrice.Height();
	int nWidth = (m_rectPrice.Width() - 10) / 4;
	if (nWidth < 10)
		return;
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penRed);
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont20, (IRenderObj**)&OldFont);

	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rectPrice.left + 20, VPOS(-1),
			m_rectPrice.left + nWidth * 2 + 80, VPOS(1) - SPACE),
		DT_CENTER | DT_TOP | DT_SINGLELINE);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);
	strTmp = StrA2StrW(m_strStockID);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rectPrice.left + 20, m_rectPrice.top,
			m_rectPrice.left + nWidth * 2 + 80, VPOS(1)),
		DT_CENTER | DT_BOTTOM | DT_SINGLELINE);


	strTmp = L"";
	//横线
	pRT->DrawRectangle(CRect(m_rectPrice.left - 6, VPOS(1),
		m_rectPrice.right + 6, VPOS(2)));
	//point[0].SetPoint(m_rectPrice.left, VPOS(1) );
	//point[1].SetPoint(m_rectPrice.right + SPACE, VPOS(1));
	//pRT->DrawLines(point, 2);

	int left = m_rectPrice.left - 5;
	int right = m_rectPrice.left + 30;
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"委比", wcslen(L"委比"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	left = m_rectPrice.left + nWidth * 2 + 10;
	right = m_rectPrice.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"委差", wcslen(L"委差"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	left = m_rectPrice.left - 5;
	right = m_rectPrice.left + 40;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"卖十", wcslen(L"卖十"),
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖九", wcslen(L"卖九"),
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖八", wcslen(L"卖八"),
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖七", wcslen(L"卖七"),
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖六", wcslen(L"卖六"),
		CRect(left, VPOS(6), right, VPOS(7)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖五", wcslen(L"卖五"),
		CRect(left, VPOS(7), right, VPOS(8)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖四", wcslen(L"卖四"),
		CRect(left, VPOS(8), right, VPOS(9)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖三", wcslen(L"卖三"),
		CRect(left, VPOS(9), right, VPOS(10)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖二", wcslen(L"卖二"),
		CRect(left, VPOS(10), right, VPOS(11)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖一", wcslen(L"卖一"),
		CRect(left, VPOS(11), right, VPOS(12)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pRT->DrawTextW(L"买一", wcslen(L"买一"),
		CRect(left, VPOS(12), right, VPOS(13)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买二", wcslen(L"买二"),
		CRect(left, VPOS(13), right, VPOS(14)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买三", wcslen(L"买三"),
		CRect(left, VPOS(14), right, VPOS(15)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买四", wcslen(L"买四"),
		CRect(left, VPOS(15), right, VPOS(16)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买五", wcslen(L"买五"),
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买六", wcslen(L"买六"),
		CRect(left, VPOS(17), right, VPOS(18)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买七", wcslen(L"买七"),
		CRect(left, VPOS(18), right, VPOS(19)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买八", wcslen(L"买八"),
		CRect(left, VPOS(19),
			right, VPOS(20)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买九", wcslen(L"买九"),
		CRect(left, VPOS(20), right, VPOS(21)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买十", wcslen(L"买十"),
		CRect(left, VPOS(21), right, VPOS(22)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	//横线
	point[0].SetPoint(m_rectPrice.left - 5, VPOS(12));
	point[1].SetPoint(m_rectPrice.right + 5, VPOS(12));
	pRT->DrawLines(point, 2);

	int pxLeft = m_rectPrice.left + 30;
	int pxRight = m_rectPrice.left + nWidth * 2 - 5;
	int volLeft = m_rectPrice.left + nWidth * 2;
	int volRight = m_rectPrice.left + nWidth * 4 - 30;
	int chgLeft = m_rectPrice.left + nWidth * 4 - 30;
	int chgRight = m_rectPrice.right + 5;

	//量
	int BidVol = 0;
	int AskVol = 0;
	std::map<double, int> preAskMap;
	std::map<double, int> preBidMap;
	if (m_preStockTick.UpdateTime != 0)
	{
		for (int i = 0; i < 10; ++i)
		{
			if (m_preStockTick.AskPrice[i] > 0 &&
				m_preStockTick.AskVolume[i] > 0)
				preAskMap[m_preStockTick.AskPrice[i]]
				= m_preStockTick.AskVolume[i] / 100;
			if (m_preStockTick.BidPrice[i] > 0 &&
				m_preStockTick.BidVolume[i] > 0)
				preBidMap[m_preStockTick.BidPrice[i]]
				= m_preStockTick.BidVolume[i] / 100;
		}
	}

	for (int i = 0; i < 10; ++i)
	{
		int nVol = m_StockTick.AskVolume[9 - i] / 100;
		double fPrice = m_StockTick.AskPrice[9 - i];
		if (nVol > 0 || fPrice > 0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((2 + i)), volRight, VPOS((3 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			AskVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetPriceColor(fPrice));
				_swprintf(szTmp, L"%.02f", fPrice);
				pRT->DrawTextW(szTmp, wcslen(szTmp),
					CRect(pxLeft, VPOS((2 + i)), pxRight, VPOS((3 + i))),
					DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				int preVol = 0;
				if (preAskMap.count(fPrice))
					preVol = preAskMap[fPrice];
				if (nVol != preVol)
				{
					int diff = nVol - preVol;
					if (diff > 0)
						pRT->SetTextColor(RGBA(255, 31, 31, 255));
					else
						pRT->SetTextColor(RGBA(0, 255, 0, 255));
					_swprintf(szTmp, L"%+d", diff);
					pRT->SelectObject(m_pFont10, (IRenderObj**)&OldFont);
					pRT->DrawTextW(szTmp, wcslen(szTmp),
						CRect(chgLeft, VPOS((2 + i)), chgRight, VPOS((3 + i))),
						DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
					pRT->SelectObject(OldFont);
				}
			}

		}
	}

	double preAsk1 = m_preStockTick.AskPrice[0];
	if (preAsk1 > 0 && m_StockTick.AskPrice[0] > 0)
	{
		pRT->SelectObject(m_pFont10, (IRenderObj**)&OldFont);
		if (m_StockTick.AskPrice[0] < preAsk1)
		{
			pRT->SetTextColor(RGBA(0, 255, 0, 255));
			_swprintf(szTmp, L"▼");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(11), volLeft + 8, VPOS(12)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.AskPrice[0] > preAsk1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"▲");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(11), volLeft + 8, VPOS(12)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		pRT->SelectObject(OldFont);

	}


	for (int i = 0; i < 10; ++i)
	{
		int nVol = m_StockTick.BidVolume[i] / 100;
		double fPrice = m_StockTick.BidPrice[i];
		if (nVol > 0 || fPrice > 0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((12 + i)), volRight, VPOS((13 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			BidVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetPriceColor(fPrice));
				_swprintf(szTmp, L"%.02f", fPrice);
				pRT->DrawTextW(szTmp, wcslen(szTmp),
					CRect(pxLeft, VPOS((12 + i)), pxRight, VPOS((13 + i))),
					DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				int preVol = 0;
				if (preBidMap.count(fPrice))
					preVol = preBidMap[fPrice];
				if (nVol != preVol)
				{
					int diff = nVol - preVol;
					if (diff > 0)
						pRT->SetTextColor(RGBA(255, 31, 31, 255));
					else
						pRT->SetTextColor(RGBA(0, 255, 0, 255));
					_swprintf(szTmp, L"%+d", diff);
					pRT->SelectObject(m_pFont10, (IRenderObj**)&OldFont);
					pRT->DrawTextW(szTmp, wcslen(szTmp),
						CRect(chgLeft, VPOS((12 + i)), chgRight, VPOS((13 + i))),
						DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
					pRT->SelectObject(OldFont);
				}

			}

		}
	}

	double preBid1 = m_preStockTick.BidPrice[0];
	if (preBid1 > 0 && m_StockTick.BidPrice[0] > 0)
	{
		pRT->SelectObject(m_pFont10, (IRenderObj**)&OldFont);
		if (m_StockTick.BidPrice[0] < preBid1)
		{
			pRT->SetTextColor(RGBA(0, 255, 0, 255));
			_swprintf(szTmp, L"▼");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(12), volLeft + 8, VPOS(13)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.BidPrice[0] > preBid1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"▲");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(12), volLeft + 8, VPOS(13)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		pRT->SelectObject(OldFont);

	}



	//委比委差
	if (BidVol + AskVol > 0)
	{
		int diff = BidVol - AskVol;
		double diffRatio = diff*100.0 / (BidVol + AskVol);

		if (diff > 0)
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
		else if (diff < 0)
			pRT->SetTextColor(RGBA(0, 255, 0, 255));
		else
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
		left = m_rectPrice.left + 30;
		right = m_rectPrice.left + nWidth * 2;
		_swprintf(szTmp, L"%.02f%%", diffRatio);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rectPrice.left + nWidth * 2 + 40;
		right = m_rectPrice.left + nWidth * 4;
		_swprintf(szTmp, L"%d", diff);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"-");
		left = m_rectPrice.left + 30;
		right = m_rectPrice.left + nWidth * 2;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rectPrice.left + nWidth * 2 + 40;
		right = m_rectPrice.left + nWidth * 4;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}


	//横线
	point[0].SetPoint(m_rectPrice.left - 5, VPOS(22));
	point[1].SetPoint(m_rectPrice.right, VPOS(22));
	pRT->DrawLines(point, 2);

	//第一列
	left = m_rectPrice.left - 5;
	right = m_rectPrice.left + 30;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2,
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2,
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2,
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_LEFT);
	pRT->DrawTextW(L"涨停", 2,
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_LEFT);
	pRT->DrawTextW(L"总手", 2,
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_LEFT);
	pRT->DrawTextW(L"昨收", 2,
		CRect(left, VPOS(27), right, VPOS(28)),
		DT_LEFT);

	left = m_rectPrice.left + nWidth * 2 + 10;
	right = m_rectPrice.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"开盘", 2,
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_LEFT);
	pRT->DrawTextW(L"最高", 2,
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_LEFT);
	pRT->DrawTextW(L"最低", 2,
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_LEFT);
	pRT->DrawTextW(L"跌停", 2,
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_LEFT);
	pRT->DrawTextW(L"金额", 2,
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_LEFT);

	//数据
	left = m_rectPrice.left + 30;
	right = m_rectPrice.left + nWidth * 2;
	pRT->SetTextColor(GetPriceColor(m_StockTick.LastPrice));
	if (m_StockTick.LastPrice > 10000000
		|| m_StockTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(22), right, VPOS(23)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(23), right, VPOS(24)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(24), right, VPOS(25)),
			DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(22), right, VPOS(23)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.02f",
			m_StockTick.LastPrice - m_StockTick.PreCloPrice);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(23), right, VPOS(24)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%",
			(m_StockTick.LastPrice - m_StockTick.PreCloPrice)
			/ (m_StockTick.PreCloPrice) * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(24), right, VPOS(25)),
			DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 31, 31, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 + m_fMaxChgPct));	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	m_StockTick.Volume /= 100;
	if (m_StockTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%.2f万", m_StockTick.Volume / 10'000.0);	//昨收
	else
		_swprintf(szTmp, L"%lld", m_StockTick.Volume);

	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_RIGHT);

	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(27), right, VPOS(28)),
		DT_RIGHT);

	left = m_rectPrice.left + nWidth * 2 + 40;
	right = m_rectPrice.left + nWidth * 4;
	//开盘
	pRT->SetTextColor(GetPriceColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_RIGHT);

	//最高
	pRT->SetTextColor(GetPriceColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_RIGHT);

	//最低
	pRT->SetTextColor(GetPriceColor(m_StockTick.LowPrice));
	if (m_StockTick.LowPrice > 10000000 || m_StockTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 - m_fMaxChgPct));	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	if (m_StockTick.Turnover > 100'000'000'000)
		_swprintf(szTmp, L"%.0f亿", m_StockTick.Turnover / 100'000'000);
	else if (m_StockTick.Turnover > 100'000'000)
		_swprintf(szTmp, L"%.02f亿", m_StockTick.Turnover / 100'000'000);
	else if (m_StockTick.Turnover > 1'000'000)
		_swprintf(szTmp, L"%.0f万", m_StockTick.Turnover / 10'000);
	else
		_swprintf(szTmp, L"%.0f", m_StockTick.Turnover);	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_RIGHT);


	//横线
	point[0].SetPoint(m_rectPrice.left - 5, VPOS(27) + 22);
	point[1].SetPoint(m_rectPrice.right, VPOS(27) + 22);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = VPOS(22); i < VPOS(28); i += 3)
		pRT->SetPixel(m_rectPrice.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

}

void SOUI::SSimTradeInfo::DrawDeal(IRenderTarget * pRT)
{
	if (!m_bUpdateDeal)
		return;
	//画单个合约
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rectDeal.Height();
	int nLineCount = m_rectDeal.Height() / RC_PRH - 1;
	int nWidth = m_rectDeal.Width() / 3;
	//std::wstringstream wss;
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);

	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"),
			CRect(m_rectDeal.left, m_rectDeal.top,
				m_rectDeal.left + nWidth, m_rectDeal.top + RC_PRH),
			DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"),
			CRect(m_rectDeal.left + nWidth, m_rectDeal.top,
				m_rectDeal.left + nWidth * 2, m_rectDeal.top + RC_PRH),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"),
			CRect(m_rectDeal.left + nWidth * 2, m_rectDeal.top,
				m_rectDeal.left + nWidth * 3, m_rectDeal.top + RC_PRH),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (m_nOffset == 0)
		{
			m_DealMarketVec.clear();
			for (int i = m_pStkMarketVec->size() - 1; i >= 0; --i)
			{
				auto &tick = m_pStkMarketVec->at(i);
				if (i != 0)
				{
					auto &preTick = m_pStkMarketVec->at(i - 1);
					if (tick.Volume == preTick.Volume
						|| tick.Volume < preTick.Volume)
						continue;
					m_DealMarketVec.emplace_back(tick);
					if (m_DealMarketVec.size() >= nLineCount + 1)
						break;
				}
				else if (tick.Volume != 0)
					m_DealMarketVec.emplace_back(tick);
			}
		}
		int nMaxOffset = m_DealMarketVec.size() % nLineCount == 0 ?
			m_DealMarketVec.size() / nLineCount : m_DealMarketVec.size() / nLineCount + 1;
		if (m_nOffset + 1 >= nMaxOffset)
			m_nOffset = nMaxOffset - 1;

		int i = 0;
		int nStart = nLineCount * (m_nOffset + 1) -1;
		int j = nStart >= m_DealMarketVec.size() ?
			m_DealMarketVec.size() - 1 : nStart;
		for (i,j; i<nLineCount; --j, ++i)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%02d:%02d:%02d",
				m_DealMarketVec[j].UpdateTime / 10'000,
				m_DealMarketVec[j].UpdateTime % 10'000 / 100,
				m_DealMarketVec[j].UpdateTime % 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rectDeal.left, m_rectDeal.top + RC_PRH * (i + 1),
					m_rectDeal.left + nWidth, m_rectDeal.top + RC_PRH * (i + 2)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (m_DealMarketVec[j].LastPrice > m_DealMarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (m_DealMarketVec[j].LastPrice < m_DealMarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%.02f", m_DealMarketVec[j].LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rectDeal.left + nWidth,
					m_rectDeal.top + RC_PRH * (i + 1),
					m_rectDeal.left + nWidth * 2,
					m_rectDeal.top + RC_PRH * (i + 2)),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			if (j != m_DealMarketVec.size() - 1)
				_swprintf(szTmp, L"%.0f",
					ceil((m_DealMarketVec[j].Volume -
						m_DealMarketVec[j + 1].Volume) *1.0 / 100));
			else
				_swprintf(szTmp, L"%d", m_DealMarketVec[j].Volume / 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rectDeal.left + nWidth * 2, m_rectDeal.top + RC_PRH * (i + 1),
					m_rectDeal.left + nWidth * 3, m_rectDeal.top + RC_PRH * (i + 2)),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
		if (m_nOffset > 0)
			m_bUpdateDeal = FALSE;
	}
	pRT->SelectObject(OldFont);

}

COLORREF SOUI::SSimTradeInfo::GetPriceColor(double fPrice)
{
	if (fPrice > 10000000 || fPrice < 0)
		return RGBA(255, 255, 255, 255);
	if (fPrice > m_StockTick.PreCloPrice)
		return RGBA(255, 31, 31, 255);
	else if (fPrice < m_StockTick.PreCloPrice)
		return RGBA(0, 255, 0, 255);
	return RGBA(255, 255, 255, 255);
}
