#include "stdafx.h"
#include"PriceList.h"
#include <unordered_map>
#include<map>

#define VPOS(x) (m_rect.top + RC_PRH * x)
#define SPACE 5

CPriceList::CPriceList()
{
	m_strSubIns = "";
	//	m_nIndex = -1;
	m_bInit = FALSE;
	m_bIsStockIndex = false;
	m_bInsInited = FALSE;
	
}

void CPriceList::SetShowData(SStringA StockID, SStringA StockName,
	vector<CommonStockMarket>* pStkMarketVec)
{
	m_bInsInited = FALSE;
	m_strSubIns = StockID;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
	m_strStockName = StockName;
	if (m_strStockName.Find("ST") != -1)
		m_fMaxChgPct = 0.05;
	else if (m_strSubIns[0] == '3' || m_strSubIns.Find("688") != -1)
		m_fMaxChgPct = 0.2;
	else
		m_fMaxChgPct = 0.1;
	m_bInsInited = TRUE;
}

void CPriceList::SetShowData(SStringA StockID, SStringA StockName,
	vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_bInsInited = FALSE;
	m_strSubIns = StockID;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
	m_strStockName = StockName;
	m_bInsInited = TRUE;
}

void CPriceList::SetIndyName(vector<SStringA>& nameVec)
{
	m_strL1Indy = nameVec[0];
	m_strL2Indy = nameVec[1];
}


CPriceList::~CPriceList()
{
}

void SOUI::CPriceList::Paint(IRenderTarget * pRT)
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

	if (m_bInsInited)
	{
		if (!m_bIsStockIndex)
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
			if (!m_bHalfPrice)
				DrawStock(pRT);
			else
				DrawStockHalf(pRT);
		}
		else
		{
			if (!m_pIdxMarketVec->empty())
				m_IndexTick = m_pIdxMarketVec->back();
			else
				m_IndexTick = CommonIndexMarket{ 0 };
			DrawIndex(pRT);
		}
	}

}


void CPriceList::DrawStock(IRenderTarget * pRT)
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
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20, VPOS(-1),
			m_rect.left + nWidth * 2 + 80, VPOS(1) - SPACE),
		DT_CENTER | DT_TOP | DT_SINGLELINE);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);
	strTmp = StrA2StrW(m_strSubIns);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20, m_rect.top,
			m_rect.left + nWidth * 2 + 80, VPOS(1)),
		DT_CENTER | DT_BOTTOM | DT_SINGLELINE);


	strTmp = L"";
	//横线
	pRT->DrawRectangle(CRect(m_rect.left - 6, VPOS(1),
		m_rect.right+6, VPOS(2)));
	//point[0].SetPoint(m_rect.left, VPOS(1) );
	//point[1].SetPoint(m_rect.right + SPACE, VPOS(1));
	//pRT->DrawLines(point, 2);

	int left = m_rect.left - 5;
	int right = m_rect.left + 30;
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"委比", wcslen(L"委比"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	left = m_rect.left + nWidth * 2 + 10;
	right = m_rect.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"委差", wcslen(L"委差"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	left = m_rect.left - 5;
	right = m_rect.left + 40;

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
	point[0].SetPoint(m_rect.left - 5, VPOS(12));
	point[1].SetPoint(m_rect.right + 5, VPOS(12));
	pRT->DrawLines(point, 2);

	int pxLeft = m_rect.left + 30;
	int pxRight = m_rect.left + nWidth * 2 - 5;
	int volLeft = m_rect.left + nWidth * 2;
	int volRight = m_rect.left + nWidth * 4 - 30;
	int chgLeft = m_rect.left + nWidth * 4 - 30;
	int chgRight = m_rect.right+5;

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
		if (nVol > 0 || fPrice >0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((2 + i)), volRight, VPOS((3 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			AskVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetTextColor(fPrice));
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
		if (nVol > 0 || fPrice>0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((12 + i)), volRight, VPOS((13 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			BidVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetTextColor(fPrice));
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
		left = m_rect.left + 30;
		right = m_rect.left + nWidth * 2;
		_swprintf(szTmp, L"%.02f%%", diffRatio);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rect.left + nWidth * 2 + 40;
		right = m_rect.left + nWidth * 4;
		_swprintf(szTmp, L"%d", diff);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"-");
		left = m_rect.left + 30;
		right = m_rect.left + nWidth * 2;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rect.left + nWidth * 2 + 40;
		right = m_rect.left + nWidth * 4;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}


	//横线
	point[0].SetPoint(m_rect.left - 5, VPOS(22));
	point[1].SetPoint(m_rect.right, VPOS(22));
	pRT->DrawLines(point, 2);

	//第一列
	left = m_rect.left - 5;
	right = m_rect.left + 30;

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

	left = m_rect.left + nWidth * 2 + 10;
	right = m_rect.left + nWidth * 2 + 40;
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
	left = m_rect.left + 30;
	right = m_rect.left + nWidth * 2;
	pRT->SetTextColor(GetTextColor(m_StockTick.LastPrice));
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

	left = m_rect.left + nWidth * 2 + 40;
	right = m_rect.left + nWidth * 4;
	//开盘
	pRT->SetTextColor(GetTextColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_StockTick.LowPrice));
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
	point[0].SetPoint(m_rect.left - 5, VPOS(27) + 22);
	point[1].SetPoint(m_rect.right, VPOS(27) + 22);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = VPOS(22); i < VPOS(28); i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

}

void SOUI::CPriceList::DrawStockHalf(IRenderTarget * pRT)
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
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20, VPOS(-1),
			m_rect.left + nWidth * 2 + 80, VPOS(1) - SPACE),
		DT_CENTER | DT_TOP | DT_SINGLELINE);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);
	strTmp = StrA2StrW(m_strSubIns);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20, m_rect.top,
			m_rect.left + nWidth * 2 + 80, VPOS(1)),
		DT_CENTER | DT_BOTTOM | DT_SINGLELINE);


	strTmp = L"";
	//横线
	pRT->DrawRectangle(CRect(m_rect.left - 6, VPOS(1),
		m_rect.right + 6, VPOS(2)));
	//point[0].SetPoint(m_rect.left, VPOS(1) );
	//point[1].SetPoint(m_rect.right + SPACE, VPOS(1));
	//pRT->DrawLines(point, 2);

	int left = m_rect.left - 5;
	int right = m_rect.left + 30;
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"委比", wcslen(L"委比"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	left = m_rect.left + nWidth * 2 + 10;
	right = m_rect.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"委差", wcslen(L"委差"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	left = m_rect.left - 5;
	right = m_rect.left + 40;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	
	pRT->DrawTextW(L"卖五", wcslen(L"卖五"),
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖四", wcslen(L"卖四"),
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖三", wcslen(L"卖三"),
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖二", wcslen(L"卖二"),
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖一", wcslen(L"卖一"),
		CRect(left, VPOS(6), right, VPOS(7)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pRT->DrawTextW(L"买一", wcslen(L"买一"),
		CRect(left, VPOS(7), right, VPOS(8)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买二", wcslen(L"买二"),
		CRect(left, VPOS(8), right, VPOS(9)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买三", wcslen(L"买三"),
		CRect(left, VPOS(9), right, VPOS(10)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买四", wcslen(L"买四"),
		CRect(left, VPOS(10), right, VPOS(11)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买五", wcslen(L"买五"),
		CRect(left, VPOS(11), right, VPOS(12)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left - 5, VPOS(7));
	point[1].SetPoint(m_rect.right + 5, VPOS(7));
	pRT->DrawLines(point, 2);

	int pxLeft = m_rect.left + 30;
	int pxRight = m_rect.left + nWidth * 2 - 5;
	int volLeft = m_rect.left + nWidth * 2;
	int volRight = m_rect.left + nWidth * 4 - 30;
	int chgLeft = m_rect.left + nWidth * 4 - 30;
	int chgRight = m_rect.right + 5;

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

	for (int i = 0; i < 5; ++i)
	{
		int nVol = m_StockTick.AskVolume[5 - i] / 100;
		double fPrice = m_StockTick.AskPrice[5 - i];
		if (nVol > 0 || fPrice >0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((2 + i)), volRight, VPOS((3 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			AskVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetTextColor(fPrice));
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
				CRect(pxRight, VPOS(6), volLeft + 8, VPOS(7)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.AskPrice[0] > preAsk1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"▲");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(6), volLeft + 8, VPOS(7)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		pRT->SelectObject(OldFont);

	}


	for (int i = 0; i < 5; ++i)
	{
		int nVol = m_StockTick.BidVolume[i] / 100;
		double fPrice = m_StockTick.BidPrice[i];
		if (nVol > 0 || fPrice>0)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"%d", nVol);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(volLeft, VPOS((7 + i)), volRight, VPOS((8 + i))),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			BidVol += nVol;
			if (fPrice > 0)
			{
				pRT->SetTextColor(GetTextColor(fPrice));
				_swprintf(szTmp, L"%.02f", fPrice);
				pRT->DrawTextW(szTmp, wcslen(szTmp),
					CRect(pxLeft, VPOS((7 + i)), pxRight, VPOS((8 + i))),
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
						CRect(chgLeft, VPOS((7 + i)), chgRight, VPOS((8 + i))),
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
				CRect(pxRight, VPOS(7), volLeft + 8, VPOS(8)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.BidPrice[0] > preBid1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"▲");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(7), volLeft + 8, VPOS(8)),
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
		left = m_rect.left + 30;
		right = m_rect.left + nWidth * 2;
		_swprintf(szTmp, L"%.02f%%", diffRatio);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rect.left + nWidth * 2 + 40;
		right = m_rect.left + nWidth * 4;
		_swprintf(szTmp, L"%d", diff);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"-");
		left = m_rect.left + 30;
		right = m_rect.left + nWidth * 2;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		left = m_rect.left + nWidth * 2 + 40;
		right = m_rect.left + nWidth * 4;
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(1), right, VPOS(2)),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}


	//横线
	point[0].SetPoint(m_rect.left - 5, VPOS(12));
	point[1].SetPoint(m_rect.right, VPOS(12));
	pRT->DrawLines(point, 2);

	//第一列
	left = m_rect.left - 5;
	right = m_rect.left + 30;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2,
		CRect(left, VPOS(12), right, VPOS(13)),
		DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2,
		CRect(left, VPOS(13), right, VPOS(14)),
		DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2,
		CRect(left, VPOS(14), right, VPOS(15)),
		DT_LEFT);
	pRT->DrawTextW(L"涨停", 2,
		CRect(left, VPOS(15), right, VPOS(16)),
		DT_LEFT);
	pRT->DrawTextW(L"总手", 2,
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_LEFT);
	pRT->DrawTextW(L"昨收", 2,
		CRect(left, VPOS(17), right, VPOS(18)),
		DT_LEFT);

	left = m_rect.left + nWidth * 2 + 10;
	right = m_rect.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"开盘", 2,
		CRect(left, VPOS(12), right, VPOS(13)),
		DT_LEFT);
	pRT->DrawTextW(L"最高", 2,
		CRect(left, VPOS(13), right, VPOS(14)),
		DT_LEFT);
	pRT->DrawTextW(L"最低", 2,
		CRect(left, VPOS(14), right, VPOS(15)),
		DT_LEFT);
	pRT->DrawTextW(L"跌停", 2,
		CRect(left, VPOS(15), right, VPOS(16)),
		DT_LEFT);
	pRT->DrawTextW(L"金额", 2,
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_LEFT);

	//数据
	left = m_rect.left + 30;
	right = m_rect.left + nWidth * 2;
	pRT->SetTextColor(GetTextColor(m_StockTick.LastPrice));
	if (m_StockTick.LastPrice > 10000000
		|| m_StockTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(12), right, VPOS(13)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(13), right, VPOS(14)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(14), right, VPOS(15)),
			DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(12), right, VPOS(13)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.02f",
			m_StockTick.LastPrice - m_StockTick.PreCloPrice);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(13), right, VPOS(14)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%",
			(m_StockTick.LastPrice - m_StockTick.PreCloPrice)
			/ (m_StockTick.PreCloPrice) * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(14), right, VPOS(15)),
			DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 31, 31, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 + m_fMaxChgPct));	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(15), right, VPOS(16)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	m_StockTick.Volume /= 100;
	if (m_StockTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%.2f万", m_StockTick.Volume / 10'000.0);	//昨收
	else
		_swprintf(szTmp, L"%lld", m_StockTick.Volume);

	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_RIGHT);

	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(17), right, VPOS(18)),
		DT_RIGHT);

	left = m_rect.left + nWidth * 2 + 40;
	right = m_rect.left + nWidth * 4;
	//开盘
	pRT->SetTextColor(GetTextColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(12), right, VPOS(13)),
		DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(13), right, VPOS(14)),
		DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_StockTick.LowPrice));
	if (m_StockTick.LowPrice > 10000000 || m_StockTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(14), right, VPOS(15)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 - m_fMaxChgPct));	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(15), right, VPOS(16)),
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
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left - 5, VPOS(17) + 22);
	point[1].SetPoint(m_rect.right, VPOS(17) + 22);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = VPOS(12); i < VPOS(18); i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

}

void SOUI::CPriceList::DrawIndex(IRenderTarget * pRT)
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
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20,
			m_rect.top - 5,
			m_rect.left + nWidth * 2 + 80,
			m_rect.top + RC_PRH),
		DT_CENTER | DT_TOP | DT_SINGLELINE);
	pRT->SelectObject(OldFont);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);
	strTmp = StrA2StrW(m_strSubIns);
	if (strTmp.GetLength() == 7)
		strTmp = strTmp.Left(6);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rect.left + 20, m_rect.top,
			m_rect.left + nWidth * 2 + 80, VPOS(2) - SPACE),
		DT_CENTER | DT_BOTTOM | DT_SINGLELINE);

	strTmp = L"";
	//横线
	point[0].SetPoint(m_rect.left, VPOS(2) - SPACE);
	point[1].SetPoint(m_rect.right + 5, VPOS(2) - SPACE);
	pRT->DrawLines(point, 2);

	//第一列数据
	int left = m_rect.left;
	int right = m_rect.left + 30;
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2,
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2,
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2,
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_LEFT);
	pRT->DrawTextW(L"总手", 2,
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_LEFT);
	pRT->DrawTextW(L"昨收", 2,
		CRect(left, VPOS(6), right, VPOS(7)),
		DT_LEFT);

	//第二列
	left = m_rect.left + nWidth * 2 + 10;
	right = m_rect.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"开盘", 2,
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_LEFT);
	pRT->DrawTextW(L"最高", 2,
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_LEFT);
	pRT->DrawTextW(L"最低", 2,
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_LEFT);
	pRT->DrawTextW(L"金额", 2,
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_LEFT);

	pRT->SetTextColor(GetTextColor(m_IndexTick.LastPrice));
	left = m_rect.left + 30;
	right = m_rect.left + nWidth * 2;
	if (m_IndexTick.LastPrice > 10000000
		|| m_IndexTick.LastPrice <= 0)
	{
		_swprintf(szTmp, L"—");
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(2), right, VPOS(3)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(3), right, VPOS(4)),
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(4), right, VPOS(5)),
			DT_RIGHT);
	}
	else
	{
		//最新价
		_swprintf(szTmp, L"%.02f", m_IndexTick.LastPrice);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(2), right, VPOS(3)),
			DT_RIGHT);
		//涨跌
		double change = m_IndexTick.LastPrice
			- m_IndexTick.PreCloPrice;
		_swprintf(szTmp, L"%.2f", change);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(3), right, VPOS(4)),
			DT_RIGHT);
		//涨幅
		_swprintf(szTmp, L"%.2f%%",
			change / m_IndexTick.PreCloPrice * 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(4), right, VPOS(5)),
			DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	if (m_IndexTick.SecurityID[0] == '8')
		m_IndexTick.Volume /= 100;
	if (m_IndexTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%d万", m_IndexTick.Volume / 10'000);
	else
		_swprintf(szTmp, L"%d", m_IndexTick.Volume);

	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_RIGHT);

	//昨收
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%.02f", m_IndexTick.PreCloPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(6), right, VPOS(7)),
		DT_RIGHT);

	left = m_rect.left + nWidth * 2 + 40;
	right = m_rect.left + nWidth * 4;
	//开盘
	pRT->SetTextColor(GetTextColor(m_IndexTick.OpenPrice));
	if (m_IndexTick.OpenPrice > 10000000 || m_IndexTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_IndexTick.HighPrice));
	if (m_IndexTick.HighPrice > 10000000 || m_IndexTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_IndexTick.LowPrice));
	if (m_IndexTick.LowPrice > 10000000 || m_IndexTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	//金额
	if (m_IndexTick.Turnover > 10'000'000'000)
		_swprintf(szTmp, L"%.0f亿",
			m_IndexTick.Turnover / 100'000'000);
	else if (m_IndexTick.Turnover > 100'000'000)
		_swprintf(szTmp, L"%.02f亿",
			m_IndexTick.Turnover / 100'000'000);
	else if (m_IndexTick.Turnover > 1'000'000)
		_swprintf(szTmp, L"%.02f万",
			m_IndexTick.Turnover / 10'000);
	else
		_swprintf(szTmp, L"%.0f", m_IndexTick.Turnover);	//跌停

	//_swprintf(szTmp, L"-");	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	//横线
	point[0].SetPoint(m_rect.left, VPOS(7));
	point[1].SetPoint(m_rect.right, VPOS(7));
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = VPOS(2); i < VPOS(7); i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5,
			i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

}



COLORREF CPriceList::GetTextColor(double price)
{
	if (!m_bIsStockIndex)
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_StockTick.PreCloPrice)
			return RGBA(255, 31, 31, 255);
		else if (price < m_StockTick.PreCloPrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
	else
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_IndexTick.PreCloPrice)
			return RGBA(255, 31, 31, 255);
		else if (price < m_IndexTick.PreCloPrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
}


