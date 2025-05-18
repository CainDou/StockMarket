#include "stdafx.h"
#include "SFundFlowPriceVol.h"

#define HEADHEIGHT 20

#define PERTOTALHEIGHT 44
#define PREPICEIGHT  16
#define PRESUBITEMHEIGHT 20


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
#define LINEWIDTH 1

#define DATACOUNT 239


vector<SStringW> titleVec = { L"价格",L"类型",L"大单买",L"中单买",L"小单买",L"量图",
L"小单卖",L"中单卖",L"大单卖",L"总量",L"买率%" };

const int nSingleTextWidth = 60;

const int nDataCount = 4;


vector<SStringW> PvtText = { L"委托",L"撤单",L"主动成交",L"被动成交" };
vector<int> PvtType = { ePVT_Order ,ePVT_Delete ,ePVT_Trade ,ePVT_Trade };
vector<COLORREF> BuyColor = {ACTSMLBUYCOLOR,ACTMIDBUYCOLOR,ACTBIGBUYCOLOR};
vector<COLORREF> SellColor = { ACTSMLSELLCOLOR,ACTMIDSELLCOLOR,ACTBIGSELLCOLOR };

enum eVolType
{
	eVT_BigBuy,
	eVT_MidBuy,
	eVT_SmlBuy,
	eVT_Change,
	eVT_SmlSell = eVT_Change,
	eVT_MidSell,
	eVT_BigSell,
	eVT_Total,
	eVT_Count,
};


SFundFlowPriceVol::SFundFlowPriceVol()
{
	m_nOffset = 0;
	m_bInit = FALSE;
	m_ShowDataVec.resize(nDataCount);
	m_OrderPriceVolVec.resize(DATACOUNT);
	m_DeletePriceVolVec.resize(DATACOUNT);
	m_TradePriceVolVec.resize(DATACOUNT);
	m_nShowPriceType = ePVT_Trade;
	m_nShowTimeType = eSTT_AllTime;
}


SFundFlowPriceVol::~SFundFlowPriceVol()
{
}

void SFundFlowPriceVol::ClearData()
{
	std::lock_guard<std::mutex> lk(m_mx);
	m_OrderPriceVolVec.clear();
	m_DeletePriceVolVec.clear();
	m_TradePriceVolVec.clear();
	m_OrderPriceVolVec.resize(DATACOUNT);
	m_DeletePriceVolVec.resize(DATACOUNT);
	m_TradePriceVolVec.resize(DATACOUNT);

}

void SOUI::SFundFlowPriceVol::UpdateOrderPriVolData(OrderVolState * pData, int nDataCount)
{
	std::lock_guard<std::mutex> lk(m_mx);
	for (int i = 0; i < nDataCount; ++i)
		if (pData[i].nSid >= 0)
			m_OrderPriceVolVec[pData[i].nSid][pData[i].nPrice] = pData[i];

}

void SOUI::SFundFlowPriceVol::UpdateDeletePriVolData(DeleteVolState * pData, int nDataCount)
{
	std::lock_guard<std::mutex> lk(m_mx);
	for (int i = 0; i < nDataCount; ++i)
		if (pData[i].nSid >= 0)
			m_DeletePriceVolVec[pData[i].nSid][pData[i].nPrice] = pData[i];

}

void SOUI::SFundFlowPriceVol::UpdateTradePriVolData(TradeVolState * pData, int nDataCount)
{
	std::lock_guard<std::mutex> lk(m_mx);
	for (int i = 0; i < nDataCount; ++i)
		if (pData[i].nSid >= 0)
			m_TradePriceVolVec[pData[i].nSid][pData[i].nPrice] = pData[i];

}


void SFundFlowPriceVol::OnPaint(IRenderTarget * pRT)
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

void SFundFlowPriceVol::DrawPriceVol(IRenderTarget * pRT)
{
	DrawArrow(pRT);
	DrawHead1(pRT);
	DrawHead2(pRT);
	int nShowDataCount = CalcShowData();
	if (nShowDataCount == 0)
		return;
	SetMaxPaintData();
	int nItemWidth = LINEWIDTH * (nShowDataCount + 1) + nShowDataCount * PRESUBITEMHEIGHT;
	int nMaxNum = (m_rc.Height() - HEADHEIGHT * 2) / nItemWidth;
	int nPrice = m_StockTick.LastPrice * 100 + 0.5;
	int nOffset = 0;
	if (nMaxNum >= m_ShowPriceSet.size())
	{
		m_nOffset = 0;
		m_nSetOffset = 0;
	}
	else
	{
		int nPricePos = 0;
		int nCount = 0;
		for (auto&it = m_ShowPriceSet.rbegin(); it != m_ShowPriceSet.rend(); ++it)
		{
			if (*it == nPrice)
			{
				nPricePos = nCount;
				break;
			}
			++nCount;
		}
		int nHalfNum = nMaxNum / 2;
		m_nOffset = min(max(0, nPricePos - nHalfNum), m_ShowPriceSet.size() - nMaxNum);
		nOffset = m_nOffset + m_nSetOffset;
		if (nOffset < 0)
		{
			nOffset = 0;
			m_nSetOffset = nOffset - m_nOffset;
		}
		else if (nOffset > m_ShowPriceSet.size() - nMaxNum)
		{
			nOffset = m_ShowPriceSet.size() - nMaxNum;
			m_nSetOffset = nOffset - m_nOffset;
		}
	}
	int nDataCount = -1;
	int nTopHeight = m_rc.top + HEADHEIGHT * 2;
	for (auto&it = m_ShowPriceSet.rbegin(); it != m_ShowPriceSet.rend(); ++it)
	{
		++nDataCount;
		if (nDataCount < nOffset)
			continue;
		if (nDataCount - nOffset >= nMaxNum)
			break;
		DrawSingleData(pRT, *it,
			CRect(m_rc.left, nTopHeight, m_rc.right, nTopHeight + nItemWidth),
			*it == nPrice);
		nTopHeight += nItemWidth;
	}

}

void SFundFlowPriceVol::DrawArrow(IRenderTarget * pRT)
{
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(m_penGray, (IRenderObj**)&oldPen);
	CPoint pts[2];
	int nPricWidth = m_rc.Width() - (titleVec.size() - 1) * nSingleTextWidth;
	int nWidth = m_rc.left;
	for (int i = 0; i < titleVec.size(); ++i)
	{
		if (titleVec[i] != L"量图")
			nWidth += nSingleTextWidth;
		else
			nWidth += nPricWidth;
		pts[0] = { nWidth, m_rc.top +HEADHEIGHT };
		pts[1] = { nWidth, m_rc.bottom };
		pRT->DrawLines(pts, 2);
	}
	pRT->SelectObject(oldPen);

}

void SOUI::SFundFlowPriceVol::DrawHead1(IRenderTarget * pRT)
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
	pRT->DrawTextW(str.Format(L"%s %s 资金流价量分布", m_strStockName, m_strStock), wcslen(str),
		CRect(m_rc.left, m_rc.top, m_rc.right, nBottom),
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(oldColor);

}

void SFundFlowPriceVol::DrawHead2(IRenderTarget * pRT)
{
	CAutoRefPtr<IPen> oldPen;
	int nTop = m_rc.top + HEADHEIGHT;
	int nBottom = m_rc.top + HEADHEIGHT * 2;
	int nLeft = m_rc.left;
	int nPricWidth = m_rc.Width() - (titleVec.size() - 1) * nSingleTextWidth;

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
	
	bool bPastPic = false;
	for (int i = 0; i < titleVec.size(); ++i)
	{
		int nRight = nLeft;
		int nTextAlign = DT_RIGHT;
		if (titleVec[i] != L"量图")
		{
			nRight += nSingleTextWidth;
			if (bPastPic)
				nTextAlign = DT_LEFT;
		}
		else
		{
			nRight += nPricWidth;
			bPastPic = true;
			nTextAlign = DT_CENTER;
		}

		pRT->DrawTextW(titleVec[i], wcslen(titleVec[i]),
			CRect(nLeft, nTop, nRight, nBottom),
			nTextAlign | DT_VCENTER | DT_SINGLELINE);
		nLeft = nRight;
	}

	pRT->SetTextColor(oldColor);

}

void SFundFlowPriceVol::DrawSingleData(IRenderTarget * pRT, int nPrice,  CRect& rc, BOOL bNowPrice)
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

	//int nPicTop = rc.top + ITEMMARGIN;
	//int nPicBottom = rc.bottom - ITEMMARGIN;
	int nTop = rc.top + MARGIN;
	SStringW str;
	int nLeft = rc.left;
	double fPrice = nPrice * 0.01;
	COLORREF oldColor = pRT->SetTextColor(GetTextColor(fPrice));
	if (bNowPrice)
		pRT->FillSolidRect(CRect(nLeft + MARGIN, rc.top + MARGIN,
			nLeft + nSingleTextWidth - MARGIN, rc.bottom - MARGIN),
			RGBA(0xA0, 0xA0, 0x00, 0xFF));
	pRT->DrawTextW(str.Format(L"%.02f", fPrice), wcslen(str),
		CRect(nLeft, rc.top, nLeft + nSingleTextWidth, rc.bottom),
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	nLeft += nSingleTextWidth;
	bool bNeedDrawDotLine = false;
	for (int i = 0; i < nDataCount; ++i)
	{
		if (m_nShowPriceType & PvtType[i])
		{
			if (bNeedDrawDotLine)
			{
				pRT->SelectObject(m_penDotGray, (IRenderObj**)&oldPen);
				CPoint pts[2];
				pts[0] = { nLeft, nTop };
				pts[1] = { rc.right, nTop };
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
				nTop += LINEWIDTH;
			}
			CRect rcDetail = { nLeft ,nTop,rc.right,nTop + PRESUBITEMHEIGHT };
			if (m_ShowDataVec[i].count(nPrice))
				DrawDetailData(pRT, m_ShowDataVec[i][nPrice], rcDetail, PvtText[i]);
			else
				DrawDetailData(pRT, ShowPV{ 0 }, rcDetail, PvtText[i]);
			nTop += PRESUBITEMHEIGHT;
			bNeedDrawDotLine = true;
		}

	}

	
}

void SOUI::SFundFlowPriceVol::DrawDetailData(IRenderTarget * pRT, const ShowPV & data, CRect & rc, SStringW & strType)
{
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	int nPicWidth = m_rc.Width() - (titleVec.size() - 1) * nSingleTextWidth;
	int nHalfWidth = nPicWidth / 2 - MARGIN;
	int nLeft = rc.left;
	int nTextAlign = DT_RIGHT;
	pRT->DrawTextW(strType, wcslen(strType),
		CRect(nLeft, rc.top, nLeft + nSingleTextWidth, rc.bottom),
		nTextAlign | DT_VCENTER | DT_SINGLELINE);
	SStringW str;
	nLeft += nSingleTextWidth;
	for (int i = 0; i < eVT_Count; ++i)
	{
		str = GetVolShowText(data.nVolInfo[i]);
		if (i < eVT_Change)
			pRT->SetTextColor(BuyColor[eVT_Change - i-1]);
		else if(i<eVT_Total)
			pRT->SetTextColor(SellColor[i - eVT_Change]);
		else
			pRT->SetTextColor(RGBA(255,255,255,255));

		if (i == eVT_Change)
		{
			nLeft += nPicWidth;
			nTextAlign = DT_LEFT;
		}
		pRT->DrawTextW(str, wcslen(str),
			CRect(nLeft, rc.top, nLeft + nSingleTextWidth, rc.bottom),
			nTextAlign | DT_VCENTER | DT_SINGLELINE);
		nLeft += nSingleTextWidth;
	}
	pRT->DrawTextW(str.Format(L"%.02f", data.fBuyRatio * 100), wcslen(str),
		CRect(nLeft, rc.top, nLeft + nSingleTextWidth, rc.bottom),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	nLeft = rc.left + (eVT_Change+1) * nSingleTextWidth;

	int PicCenter = nLeft + nHalfWidth;



	int nBuyRight = PicCenter;
	int nSellLeft = PicCenter;
	int nWidth = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (data.nVolInfo[eVT_Change - i - 1] > 0)
		{
			nWidth = nHalfWidth * 1.0 / m_MaxVol * data.nVolInfo[eVT_Change - i - 1];
			pRT->FillSolidRect(CRect(nBuyRight - nWidth, rc.top + ITEMMARGIN,
				nBuyRight, rc.bottom - ITEMMARGIN), BuyColor[i]);
			nBuyRight -= nWidth;
		}
		if (data.nVolInfo[eVT_Change + i] > 0)
		{
			nWidth = nHalfWidth * 1.0 / m_MaxVol * data.nVolInfo[eVT_Change + i];
			pRT->FillSolidRect(CRect(nSellLeft, rc.top + ITEMMARGIN,
				nSellLeft + nWidth, rc.bottom - ITEMMARGIN), SellColor[i]);
			nSellLeft += nWidth;
		}

	}
}

int SOUI::SFundFlowPriceVol::CalcShowData()
{
	int nID = GetID(m_StockTick.UpdateTime / 100);
	for (auto&it : m_ShowDataVec)
		it.clear();
	std::lock_guard<std::mutex> lk(m_mx);
	int nShowDataCount = 0;
	m_ShowPriceSet.clear();

	if (m_nShowPriceType & ePVT_Order)
		nShowDataCount +=CalcOrderShowPv(nID);
	if(m_nShowPriceType & ePVT_Delete)
		nShowDataCount += CalcDeleteShowPv(nID);
	if (m_nShowPriceType & ePVT_Trade)
		nShowDataCount += CalcTradeShowPv(nID);
	return nShowDataCount;
}

int SOUI::SFundFlowPriceVol::CalcOrderShowPv(int id)
{


	auto func = [this](auto& dataMap,auto& orderPvMap)
	{
		for (auto &it : orderPvMap)
		{
			auto& data = dataMap[it.first];
			m_ShowPriceSet.insert(it.first);
			data.nVolInfo[eVT_BigBuy] += (it.second.nOrdBL)*0.01;
			data.nVolInfo[eVT_MidBuy] += (it.second.nOrdBM)*0.01;
			data.nVolInfo[eVT_SmlBuy] += (it.second.nOrdBS)*0.01;
			data.nVolInfo[eVT_BigSell] += (it.second.nOrdSL)*0.01;
			data.nVolInfo[eVT_MidSell] += (it.second.nOrdSM)*0.01;
			data.nVolInfo[eVT_SmlSell] += (it.second.nOrdSS)*0.01;
			data.nVolInfo[eVT_Total] = data.nVolInfo[eVT_BigBuy] + data.nVolInfo[eVT_MidBuy] + data.nVolInfo[eVT_SmlBuy] +
				data.nVolInfo[eVT_BigSell] + data.nVolInfo[eVT_MidSell] + data.nVolInfo[eVT_SmlSell];
				data.fBuyRatio = (data.nVolInfo[eVT_BigBuy] + data.nVolInfo[eVT_MidBuy] +
					data.nVolInfo[eVT_SmlBuy])*1.0 / data.nVolInfo[eVT_Total];
		}
	};

	auto &dataMap = m_ShowDataVec[0];
	if (m_nShowTimeType == eSTT_Last)
	{
		auto &orderPvMap = m_OrderPriceVolVec[id];
		func(dataMap,orderPvMap);
	}
	else
	{
		for (int i = 0; i <= id; ++i)
		{
			auto &orderPvMap = m_OrderPriceVolVec[i];
			func(dataMap,orderPvMap);
		}
	}
	return 1;

}

int SOUI::SFundFlowPriceVol::CalcDeleteShowPv(int id)
{
	auto func = [this](auto& dataMap, auto& deletePvMap)
	{
		for (auto &it : deletePvMap)
		{
			auto& data = dataMap[it.first];
			m_ShowPriceSet.insert(it.first);
			data.nVolInfo[eVT_BigBuy] += (it.second.nDelBL)*0.01;
			data.nVolInfo[eVT_MidBuy] += (it.second.nDelBM)*0.01;
			data.nVolInfo[eVT_SmlBuy] += (it.second.nDelBS)*0.01;
			data.nVolInfo[eVT_BigSell] += (it.second.nDelSL)*0.01;
			data.nVolInfo[eVT_MidSell] += (it.second.nDelSM)*0.01;
			data.nVolInfo[eVT_SmlSell] += (it.second.nDelSS)*0.01;
			data.nVolInfo[eVT_Total] = data.nVolInfo[eVT_BigBuy] + data.nVolInfo[eVT_MidBuy] + data.nVolInfo[eVT_SmlBuy] +
				data.nVolInfo[eVT_BigSell] + data.nVolInfo[eVT_MidSell] + data.nVolInfo[eVT_SmlSell];
			data.fBuyRatio = (data.nVolInfo[eVT_BigBuy] + data.nVolInfo[eVT_MidBuy] +
				data.nVolInfo[eVT_SmlBuy])*1.0 / data.nVolInfo[eVT_Total];
		}
	};
	auto &dataMap = m_ShowDataVec[1];
	if (m_nShowTimeType == eSTT_Last)
	{
		auto &deletePvMap = m_DeletePriceVolVec[id];
		func(dataMap,deletePvMap);
	}
	else
	{
		for (int i = 0; i <= id; ++i)
		{
			auto &deletePvMap = m_DeletePriceVolVec[i];
			func(dataMap,deletePvMap);
		}
	}
	return 1;

}

int SOUI::SFundFlowPriceVol::CalcTradeShowPv(int id)
{
	auto func = [this](auto& actDataMap, auto& pasDataMap,auto&tradePvMap)
	{
		for (auto &it : tradePvMap)
		{
			auto& actData = actDataMap[it.first];
			auto& pasData = pasDataMap[it.first];
			m_ShowPriceSet.insert(it.first);

			actData.nVolInfo[eVT_BigBuy] += (it.second.nTradABLU + it.second.nTradABLD) * 0.01;
			actData.nVolInfo[eVT_MidBuy] += (it.second.nTradABMU + it.second.nTradABMD)*0.01;
			actData.nVolInfo[eVT_SmlBuy] += (it.second.nTradABSU + it.second.nTradABSD)*0.01;
			actData.nVolInfo[eVT_BigSell] += (it.second.nTradASLU + it.second.nTradASLD)*0.01;
			actData.nVolInfo[eVT_MidSell] += (it.second.nTradASMU + it.second.nTradASMD)*0.01;
			actData.nVolInfo[eVT_SmlSell] += (it.second.nTradASSU + it.second.nTradASSD)*0.01;

			pasData.nVolInfo[eVT_BigBuy] += (it.second.nTradPBLU + it.second.nTradPBLD)*0.01;
			pasData.nVolInfo[eVT_MidBuy] += (it.second.nTradPBMU + it.second.nTradPBMD)*0.01;
			pasData.nVolInfo[eVT_SmlBuy] += (it.second.nTradPBSU + it.second.nTradPBSD)*0.01;
			pasData.nVolInfo[eVT_BigSell] += (it.second.nTradPSLU + it.second.nTradPSLD)*0.01;
			pasData.nVolInfo[eVT_MidSell] += (it.second.nTradPSMU + it.second.nTradPSMD)*0.01;
			pasData.nVolInfo[eVT_SmlSell] += (it.second.nTradPSSU + it.second.nTradPSSD)*0.01;

			actData.nVolInfo[eVT_Total] = actData.nVolInfo[eVT_BigBuy] + actData.nVolInfo[eVT_MidBuy] + actData.nVolInfo[eVT_SmlBuy] +
				actData.nVolInfo[eVT_BigSell] + actData.nVolInfo[eVT_MidSell] + actData.nVolInfo[eVT_SmlSell];
			actData.fBuyRatio = (actData.nVolInfo[eVT_BigBuy] + actData.nVolInfo[eVT_MidBuy] + actData.nVolInfo[eVT_SmlBuy])*1.0 / actData.nVolInfo[eVT_Total];
			pasData.nVolInfo[eVT_Total] = pasData.nVolInfo[eVT_BigBuy] + pasData.nVolInfo[eVT_MidBuy] + pasData.nVolInfo[eVT_SmlBuy] +
				pasData.nVolInfo[eVT_BigSell] + pasData.nVolInfo[eVT_MidSell] + pasData.nVolInfo[eVT_SmlSell];
			pasData.fBuyRatio = (pasData.nVolInfo[eVT_BigBuy] + pasData.nVolInfo[eVT_MidBuy] + pasData.nVolInfo[eVT_SmlBuy])*1.0/ pasData.nVolInfo[eVT_Total];

		}
	};
	auto &actDataMap = m_ShowDataVec[2];
	auto &PasDataMap = m_ShowDataVec[3];
	if (m_nShowTimeType == eSTT_Last)
	{
		auto &tradePvMap = m_TradePriceVolVec[id];
		func(actDataMap, PasDataMap, tradePvMap);
	}
	else
	{
		for (int i = 0; i <= id; ++i)
		{
			auto &tradePvMap = m_TradePriceVolVec[i];
			func(actDataMap, PasDataMap, tradePvMap);
		}
	}
	return 2;
}

void SOUI::SFundFlowPriceVol::SetMaxPaintData()
{
	m_MaxVol = 0;
	for(auto &dataMap: m_ShowDataVec)
	for (auto&it : dataMap)
		m_MaxVol = max(m_MaxVol,
			max(it.second.nVolInfo[eVT_BigBuy] + it.second.nVolInfo[eVT_MidBuy] + it.second.nVolInfo[eVT_SmlBuy],
				it.second.nVolInfo[eVT_BigSell] + it.second.nVolInfo[eVT_MidSell] + it.second.nVolInfo[eVT_SmlSell]));
}

int SOUI::SFundFlowPriceVol::GetID(int nTime)
{
	if (nTime < 930)
		return 0;
	if (nTime > 1500)
		return 238;
	if (nTime >= 1457)
		nTime = 1457;
	else if (nTime >= 1130 && nTime < 1300)
		nTime = 1300;
	int nSid = 0;
	int nHour = nTime / 100;
	int nMin = nTime - nHour * 100;
	int nHourDiff = nHour - (nHour > 11 ? 11 : 9);
	int nMinDiff = nMin - (nHour > 11 ? -1 : 29);
	nSid = nHourDiff * 60 + nMinDiff;
	return nSid;
}

COLORREF SFundFlowPriceVol::GetTextColor(double price)
{
	if (price > 10000000 || price < 0)
		return RGBA(255, 255, 255, 255);
	if (price - m_StockTick.PreCloPrice > 0.000001)
		return RGBA(255, 31, 31, 255);
	else if (price - m_StockTick.PreCloPrice < -0.000001)
		return RGBA(0, 255, 0, 255);
	return RGBA(255, 255, 255, 255);
}

SStringW SOUI::SFundFlowPriceVol::GetVolShowText(int nVol)
{
	SStringW str;
	nVol /= 100;
	if (nVol < 10000)
		str.Format(L"%d", nVol);
	else if (nVol < 100000000)
		str.Format(L"%.02f万", nVol*1.0 / 10000);
	else
		str.Format(L"%.02f亿", nVol*1.0 / 100000000);
	return str;

}
