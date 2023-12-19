#include "stdafx.h"
#include "SPriceVolPic.h"

#define HEADHEIGHT 20

#define PERTOTALHEIGHT 44
#define PRESIDEHEIGHT  18

#define PRICEWIDTH 60
#define VOLWIDTH 60
#define BIGBUYVOLWIDTH 60
#define MIDBUYVOLWIDTH 60
#define SMLBUYVOLWIDTH 60
#define BIGSELLVOLWIDTH 60
#define MIDSELLVOLWIDTH 60
#define SMLSELLVOLWIDTH 60
#define ACTBUYRATIOWIDTH 60
#define DEALWIDTH 60

#define ACTBIGBUYCOLOR RGBA(0xFF,0,0,255)
#define ACTMIDBUYCOLOR RGBA(0xFF,0x50,0,255)
#define ACTSMLBUYCOLOR RGBA(0xFF,0,0x90,255)

#define ACTBIGSELLCOLOR RGBA(0,0xFF,0,255)
#define ACTMIDSELLCOLOR RGBA(0x00,0xA0,0,255)
#define ACTSMLSELLCOLOR RGBA(0xA0,0xA0,0,255)

#define PASBIGBUYCOLOR RGBA(0xFF,0,0xFF,255)
#define PASMIDBUYCOLOR RGBA(0xFF,0x50,0xFF,255)
#define PASSMLBUYCOLOR RGBA(0xFF,0x90,0xFF,255)

#define PASBIGSELLCOLOR RGBA(0,0xFF,0xFF,255)
#define PASMIDSELLCOLOR RGBA(0x50,0xE0,0xF0,255)
#define PASSMLSELLCOLOR RGBA(0x70,0xA0,0xE0,255)


#define MARGIN 1
#define ITEMMARGIN 2

const int TextWidth = 
PRICEWIDTH + VOLWIDTH + BIGBUYVOLWIDTH + MIDBUYVOLWIDTH +
SMLBUYVOLWIDTH + BIGSELLVOLWIDTH + MIDSELLVOLWIDTH + SMLSELLVOLWIDTH +
ACTBUYRATIOWIDTH + DEALWIDTH;

SPriceVolPic::SPriceVolPic()
{
	m_nOffset = 0;
	m_bInit = FALSE;
}


SPriceVolPic::~SPriceVolPic()
{
}


void SPriceVolPic::OnPaint(IRenderTarget * pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);

	if (!m_bInit)
	{
		m_bInit = TRUE;
		pRT->CreatePen(PS_DOT, RGBA(0x50, 0x50, 0x50, 255), 1, &m_penDotGray);
		pRT->CreatePen(PS_SOLID, RGBA(0x50, 0x50, 0x50, 255), 1, &m_penGray);
		//pRT->CreatePen(PS_DOT, RGBA(255, 255, 255, 255), 1, &m_penDotGray);
		//pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &m_penGray);

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
	pRT->SetAttribute(L"antiAlias", L"0", FALSE);

	SWindow::GetClientRect(&m_rc);
	if (IsVisible())
		DrawPriceVol(pRT);
	AfterPaint(pRT, pa);

}

void SPriceVolPic::DrawPriceVol(IRenderTarget * pRT)
{
	DrawArrow(pRT);
	DrawHead1(pRT);
	DrawHead2(pRT);
	SetMaxPaintData();
	m_cs.Enter();
	auto PriceVolMap(m_PriceVolMap);
	m_cs.Leave();
	int nMaxNum = (m_rc.Height() - HEADHEIGHT * 2) / PERTOTALHEIGHT;
	int nPrice = m_StockTick.LastPrice * 100 + 0.5;
	if (nMaxNum >= PriceVolMap.size())
		m_nOffset = 0;
	else
	{
		int nPricePos = 0;
		int nCount = 0;
		for (auto&it = PriceVolMap.rbegin(); it != PriceVolMap.rend(); ++it)
		{
			if (it->first == nPrice)
			{
				nPricePos = nCount;
				break;
			}
			++nCount;
		}
		int nHalfNum = nMaxNum / 2;
		m_nOffset = min(max(0, nPricePos - nHalfNum), PriceVolMap.size() - nMaxNum);
	}
	int nDataCount = -1;
	int nTopHeight = m_rc.top + HEADHEIGHT * 2;
	for (auto&it = PriceVolMap.rbegin(); it != PriceVolMap.rend(); ++it)
	{
		++nDataCount;
		if (nDataCount < m_nOffset)
			continue;
		if (nDataCount - m_nOffset >= nMaxNum)
			break;
		DrawSingleData(pRT, it->second, 
			CRect(m_rc.left, nTopHeight, m_rc.right, nTopHeight + PERTOTALHEIGHT),
			it->first == nPrice);
		nTopHeight += PERTOTALHEIGHT;
	}

}

void SPriceVolPic::DrawArrow(IRenderTarget * pRT)
{
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penGray, (IRenderObj**)&oldPen);
	CPoint pts[2];
	int nWidth = m_rc.left + PRICEWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += VOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += BIGBUYVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += MIDBUYVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += SMLBUYVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	int nPicWidth = m_rc.Width() - TextWidth;
	nWidth += nPicWidth;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += SMLSELLVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += MIDSELLVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += BIGSELLVOLWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	nWidth += ACTBUYRATIOWIDTH;
	pts[0] = { nWidth, m_rc.top };
	pts[1] = { nWidth, m_rc.bottom };
	pRT->DrawLines(pts, 2);

	pRT->SelectObject(oldPen);

}

void SOUI::SPriceVolPic::DrawHead1(IRenderTarget * pRT)
{
	CAutoRefPtr<IPen> oldPen;
	int nBottom = m_rc.top + HEADHEIGHT;
	int nLeft = m_rc.left;
	pRT->SelectObject(m_penGray, (IRenderObj**)&oldPen);
	{
		CPoint pts[5];
		pts[0] = { m_rc.left, m_rc.top };
		pts[1] = { m_rc.right, m_rc.top };
		pts[2] = { m_rc.right, nBottom };
		pts[3] = { m_rc.left, nBottom };
		pts[4] = { m_rc.left, m_rc.top };
		pRT->DrawLines(pts, 5);
	}
	pRT->SelectObject(oldPen);
	SStringW str;
	COLORREF oldColor = pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(str.Format(L"%s %s 价量分布", m_strName,m_strStock), wcslen(str),
		CRect(m_rc.left, m_rc.top, m_rc.right, nBottom),
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(oldColor);

}

void SPriceVolPic::DrawHead2(IRenderTarget * pRT)
{
	CAutoRefPtr<IPen> oldPen;
	int nTop = m_rc.top + HEADHEIGHT;
	int nBottom = m_rc.top + HEADHEIGHT * 2;
	int nLeft = m_rc.left;
	pRT->SelectObject(m_penGray, (IRenderObj**)&oldPen);
	{
		CPoint pts[5];
		pts[0] = { m_rc.left, nTop };
		pts[1] = { m_rc.right, nTop };
		pts[2] = { m_rc.right, nBottom };
		pts[3] = { m_rc.left, nBottom };
		pts[4] = { m_rc.left, nTop };
		pRT->DrawLines(pts, 5);
	}
	pRT->SelectObject(oldPen);
	SStringW str;
	COLORREF oldColor = pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(str.Format(L"%s", L"价格"), wcslen(str),
		CRect(nLeft, nTop, nLeft + PRICEWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += PRICEWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"成交量"), wcslen(str),
		CRect(nLeft, nTop, nLeft + VOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += VOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"大单买入"), wcslen(str),
		CRect(nLeft, nTop, nLeft + BIGBUYVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += BIGBUYVOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"中单买入"), wcslen(str),
		CRect(nLeft, nTop, nLeft + MIDBUYVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += MIDBUYVOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"小单买入"), wcslen(str),
		CRect(nLeft, nTop, nLeft + SMLBUYVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += SMLBUYVOLWIDTH;

	int nPicWidth = m_rc.Width() - TextWidth;

	pRT->DrawTextW(str.Format(L"%s", L"成交量图"), wcslen(str),
		CRect(nLeft, nTop, nLeft + nPicWidth, nBottom),
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	nLeft += nPicWidth;

	pRT->DrawTextW(str.Format(L"%s", L"小单卖出"), wcslen(str),
		CRect(nLeft, nTop, nLeft + SMLSELLVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += SMLSELLVOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"中单卖出"), wcslen(str),
		CRect(nLeft, nTop, nLeft + MIDSELLVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += MIDSELLVOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"大单卖出"), wcslen(str),
		CRect(nLeft, nTop, nLeft + BIGSELLVOLWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += BIGSELLVOLWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"主买率"), wcslen(str),
		CRect(nLeft, nTop, nLeft + ACTBUYRATIOWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += ACTBUYRATIOWIDTH;

	pRT->DrawTextW(str.Format(L"%s", L"成交笔数"), wcslen(str),
		CRect(nLeft, nTop, nLeft + DEALWIDTH, nBottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += DEALWIDTH;

	pRT->SetTextColor(oldColor);

}

void SPriceVolPic::DrawSingleData(IRenderTarget * pRT, PriceVolInfo & info, CRect & rc, BOOL bNowPrice)
{
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penGray, (IRenderObj**)&oldPen);
	{
		CPoint pts[5];
		pts[0] = { rc.left, rc.top };
		pts[1] = { rc.right, rc.top };
		pts[2] = { rc.right, rc.bottom };
		pts[3] = { rc.left, rc.bottom };
		pts[4] = { rc.left, rc.top };
		pRT->DrawLines(pts, 5);
	}
	pRT->SelectObject(oldPen);

	int nVCenter = (rc.top + rc.bottom) / 2 /*+ 1*/;

	int nPicWidth = rc.Width() - TextWidth;
	int nBuyWidth = nPicWidth / 2 - MARGIN;
	int nSellWidth = nBuyWidth;
	int nPicTop = rc.top + ITEMMARGIN;
	int nPicBottom = rc.bottom - ITEMMARGIN;
	SStringW str;
	int nLeft = rc.left;
	double fPrice = info.nPriceMulti100 / 100.0;
	COLORREF oldColor = pRT->SetTextColor(GetTextColor(fPrice));
	if (bNowPrice)
		pRT->FillSolidRect(CRect(nLeft + MARGIN, rc.top + MARGIN,
			nLeft + PRICEWIDTH - MARGIN, rc.bottom - MARGIN),
			RGBA(0xA0, 0xA0, 0x00, 0xFF));
	pRT->DrawTextW(str.Format(L"%.02f", fPrice), wcslen(str),
		CRect(nLeft, rc.top, nLeft + PRICEWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += PRICEWIDTH;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	int64_t nTradeVol = info.nActBigBuyVol + info.nActMidBuyVol + info.nActSmallBuyVol +
		info.nPasBigBuyVol + info.nPasMidBuyVol + info.nPasSmallBuyVol;

	str = GetVolShowText(nTradeVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + VOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += VOLWIDTH;

	int nTwoDataLeft = nLeft;

	pRT->SetTextColor(ACTBIGBUYCOLOR);
	str = GetVolShowText(info.nActBigBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top , nLeft + BIGBUYVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	pRT->SetTextColor(PASBIGBUYCOLOR);
	str = GetVolShowText(info.nPasBigBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + BIGBUYVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += BIGBUYVOLWIDTH;

	pRT->SetTextColor(ACTMIDBUYCOLOR);
	str = GetVolShowText(info.nActMidBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + MIDBUYVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	pRT->SetTextColor(PASMIDBUYCOLOR);
	str = GetVolShowText(info.nPasMidBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + MIDBUYVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += MIDBUYVOLWIDTH;

	pRT->SetTextColor(ACTSMLBUYCOLOR);
	str = GetVolShowText(info.nActSmallBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + SMLBUYVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	pRT->SetTextColor(PASSMLBUYCOLOR);
	str = GetVolShowText(info.nPasSmallBuyVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + SMLBUYVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += SMLBUYVOLWIDTH;

	int PicCenter = nLeft + nBuyWidth;
	//主动买
	int nBuyRight = PicCenter;
	int nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nActSmallBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nPicTop,
		nBuyRight, nVCenter - MARGIN), ACTSMLBUYCOLOR);

	nBuyRight -= nWidth;
	nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nActMidBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nPicTop,
		nBuyRight, nVCenter - MARGIN), ACTMIDBUYCOLOR);

	nBuyRight -= nWidth;
	nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nActBigBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nPicTop,
		nBuyRight, nVCenter - MARGIN), ACTBIGBUYCOLOR);

	//被动买
	nBuyRight = PicCenter;
	nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nPasSmallBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nVCenter + MARGIN,
		nBuyRight, nPicBottom), PASSMLBUYCOLOR);

	nBuyRight -= nWidth;
	nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nPasMidBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nVCenter + MARGIN,
		nBuyRight, nPicBottom), PASMIDBUYCOLOR);

	nBuyRight -= nWidth;
	nWidth = nBuyWidth * 1.0 / m_MaxVol * info.nPasBigBuyVol;
	pRT->FillSolidRect(CRect(nBuyRight - nWidth, nVCenter + MARGIN,
		nBuyRight, nPicBottom), PASBIGBUYCOLOR);

	//被动卖
	int nSellLeft = PicCenter;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nPasSmallSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nPicTop,
		nSellLeft + nWidth, nVCenter - MARGIN), PASSMLSELLCOLOR);

	nSellLeft += nWidth;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nPasMidSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nPicTop,
		nSellLeft + nWidth, nVCenter - MARGIN), PASMIDSELLCOLOR);

	nSellLeft += nWidth;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nPasBigSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nPicTop,
		nSellLeft + nWidth, nVCenter - MARGIN), PASBIGSELLCOLOR);

	//主动卖
	nSellLeft = PicCenter;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nActSmallSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nVCenter + MARGIN,
		nSellLeft + nWidth, nPicBottom), ACTSMLSELLCOLOR);

	nSellLeft += nWidth;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nActMidSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nVCenter + MARGIN,
		nSellLeft + nWidth, nPicBottom), ACTMIDSELLCOLOR);

	nSellLeft += nWidth;
	nWidth = nSellWidth * 1.0 / m_MaxVol * info.nActBigSellVol;
	pRT->FillSolidRect(CRect(nSellLeft, nVCenter + MARGIN,
		nSellLeft + nWidth, nPicBottom), ACTBIGSELLCOLOR);

	nLeft += nPicWidth;

	pRT->SetTextColor(PASSMLSELLCOLOR);
	str = GetVolShowText(info.nPasSmallSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + SMLSELLVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(ACTSMLSELLCOLOR);
	str = GetVolShowText(info.nActSmallSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + SMLSELLVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += SMLSELLVOLWIDTH;

	pRT->SetTextColor(PASMIDSELLCOLOR);
	str = GetVolShowText(info.nPasMidSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + MIDSELLVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	pRT->SetTextColor(ACTMIDSELLCOLOR);
	str = GetVolShowText(info.nActMidSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + MIDSELLVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += MIDSELLVOLWIDTH;

	pRT->SetTextColor(PASBIGSELLCOLOR);
	str = GetVolShowText(info.nPasBigSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, rc.top, nLeft + BIGSELLVOLWIDTH, nVCenter),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(ACTBIGSELLCOLOR);
	str = GetVolShowText(info.nActBigSellVol);
	pRT->DrawTextW(str, wcslen(str),
		CRect(nLeft, nVCenter, nLeft + BIGSELLVOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += BIGSELLVOLWIDTH;
	int nTwoRight = nLeft;
	pRT->SelectObject(m_penDotGray, (IRenderObj**)&oldPen);
	{
		CPoint pts[2];
		pts[0] = { nTwoDataLeft, nVCenter };
		pts[1] = { nTwoRight, nVCenter };
		pRT->DrawLines(pts, 2);
	}
	pRT->SelectObject(oldPen);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	int64_t nActBuyVol = info.nActBigBuyVol + info.nActMidBuyVol + info.nActSmallBuyVol;
	int64_t nActSellVol = info.nActBigSellVol + info.nActMidSellVol + info.nActSmallSellVol;

	pRT->DrawTextW(str.Format(L"%.02f%%", nActBuyVol*1.0 / (nActBuyVol + nActSellVol) *100), wcslen(str),
		CRect(nLeft, rc.top, nLeft + ACTBUYRATIOWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	nLeft += ACTBUYRATIOWIDTH;
	int nTotalOrder = info.nActBigBuyOrder + info.nActMidBuyOrder + info.nActSmallBuyOrder +
		info.nPasBigBuyOrder + info.nPasMidBuyOrder + info.nPasSmallBuyOrder +
		info.nActBigSellOrder + info.nActMidSellOrder + info.nActSmallSellOrder +
		info.nPasBigSellOrder + info.nPasMidSellOrder + info.nPasSmallSellOrder;

	pRT->DrawTextW(str.Format(L"%d", nTotalOrder), wcslen(str),
		CRect(nLeft, rc.top, nLeft + VOLWIDTH, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

}

void SOUI::SPriceVolPic::SetMaxPaintData()
{
	m_cs.Enter();
	m_MaxVol = 0;
	for (auto&it : m_PriceVolMap)
		m_MaxVol = max(m_MaxVol,
			max(it.second.nActBigBuyVol + it.second.nActMidBuyVol + it.second.nActSmallBuyVol,
				it.second.nPasBigBuyVol + it.second.nPasMidBuyVol + it.second.nPasSmallBuyVol));
	m_cs.Leave();
}

COLORREF SPriceVolPic::GetTextColor(double price)
{
	if (price > 10000000 || price < 0)
		return RGBA(255, 255, 255, 255);
	if (price - m_StockTick.PreCloPrice > 0.000001)
		return RGBA(255, 31, 31, 255);
	else if (price - m_StockTick.PreCloPrice < -0.000001)
		return RGBA(0, 255, 0, 255);
	return RGBA(255, 255, 255, 255);
}

SStringW SOUI::SPriceVolPic::GetVolShowText(int nVol)
{
	SStringW str;
	nVol /= 100;
	if (nVol < 10000)
		str.Format(L"%d", nVol);
	else if(nVol < 100000000)
		str.Format(L"%.02f万", nVol*1.0 / 10000);
	else
		str.Format(L"%.02f亿", nVol*1.0 / 100000000);
	return str;

}
