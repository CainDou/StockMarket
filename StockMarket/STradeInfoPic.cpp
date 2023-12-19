#include "stdafx.h"
#include "STradeInfoPic.h"

#define RC_PRH			20
#define MAX_LIST_NUM	300
#define VPOS(x) (m_rectPrice.top + RC_PRH * (x+1))
#define SPACE 5
#define RC_HEIGHT		20

STradeInfoPic::STradeInfoPic()
{
	m_strStock = "";
	m_strStockName = "";
	//	m_nIndex = -1;
	m_bInit = FALSE;
	m_bInsInited = FALSE;

}


STradeInfoPic::~STradeInfoPic()
{
}

void STradeInfoPic::ChangeShowStock(SStringA StockID, SStringA StockName)
{
	m_bInsInited = FALSE;
	m_strStock = StockID;
	m_strStockName = StockName;
	if (m_strStockName.Find("ST") != -1)
		m_fMaxChgPct = 0.05;
	else if (m_strStock[0] == '3' || m_strStock.Find("688") != -1)
		m_fMaxChgPct = 0.2;
	else
		m_fMaxChgPct = 0.1;

		m_bInsInited = TRUE;

}

void STradeInfoPic::SetDataPoint(vector<CommonStockMarket>* pStkMarketVec)
{
	m_pStkMarketVec = pStkMarketVec;

}



void STradeInfoPic::OnPaint(IRenderTarget * pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);

	if (!m_bInit)
	{
		m_bInit = TRUE;
		pRT->CreatePen(PS_SOLID, RGBA(255, 31, 31, 255), 2, &m_penRed);

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 15;// �����С
		_stprintf(lf.lfFaceName, L"%s", L"΢���ź�"); // ��������
		GETRENDERFACTORY->CreateFont(&m_pFont15, lf);
		lf.lfHeight = 20;// �����С
		GETRENDERFACTORY->CreateFont(&m_pFont20, lf);
		lf.lfHeight = 12;// �����С
		GETRENDERFACTORY->CreateFont(&m_pFont10, lf);
	}
	CRect rc;
	SWindow::GetClientRect(&rc);
	//pRT->FillSolidRect(rc, RGBA(0, 0, 0, 255));
	m_rectPrice.SetRect(rc.left,
		rc.top, rc.right, rc.top + 580);
	m_rectDeal.SetRect(rc.left,
		rc.top + 585, rc.right, rc.bottom + 30);

	if (m_bInsInited)
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
	AfterPaint(pRT, pa);


}

void STradeInfoPic::DrawPrice(IRenderTarget * pRT)
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
	strTmp = StrA2StrW(m_strStock);
	pRT->DrawTextW(strTmp, strTmp.GetLength(),
		CRect(m_rectPrice.left + 20, m_rectPrice.top,
			m_rectPrice.left + nWidth * 2 + 80, VPOS(1)),
		DT_CENTER | DT_BOTTOM | DT_SINGLELINE);


	strTmp = L"";
	//����
	pRT->DrawRectangle(CRect(m_rectPrice.left, VPOS(1),
		m_rectPrice.right, VPOS(2)));
	//point[0].SetPoint(m_rectPrice.left, VPOS(1) );
	//point[1].SetPoint(m_rectPrice.right + SPACE, VPOS(1));
	//pRT->DrawLines(point, 2);

	int left = m_rectPrice.left + 1;
	int right = m_rectPrice.left + 35;
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"ί��", wcslen(L"ί��"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	left = m_rectPrice.left + nWidth * 2 + 10;
	right = m_rectPrice.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"ί��", wcslen(L"ί��"),
		CRect(left, VPOS(1), right, VPOS(2)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	left = m_rectPrice.left;
	right = m_rectPrice.left + 40;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"��ʮ", wcslen(L"��ʮ"),
		CRect(left, VPOS(2), right, VPOS(3)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(3), right, VPOS(4)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(4), right, VPOS(5)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(5), right, VPOS(6)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(6), right, VPOS(7)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(7), right, VPOS(8)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(8), right, VPOS(9)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(9), right, VPOS(10)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(10), right, VPOS(11)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"��һ", wcslen(L"��һ"),
		CRect(left, VPOS(11), right, VPOS(12)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pRT->DrawTextW(L"��һ", wcslen(L"��һ"),
		CRect(left, VPOS(12), right, VPOS(13)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"���", wcslen(L"���"),
		CRect(left, VPOS(13), right, VPOS(14)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(14), right, VPOS(15)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(15), right, VPOS(16)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(16), right, VPOS(17)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(17), right, VPOS(18)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"����", wcslen(L"����"),
		CRect(left, VPOS(18), right, VPOS(19)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"���", wcslen(L"���"),
		CRect(left, VPOS(19),
			right, VPOS(20)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"���", wcslen(L"���"),
		CRect(left, VPOS(20), right, VPOS(21)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"��ʮ", wcslen(L"��ʮ"),
		CRect(left, VPOS(21), right, VPOS(22)),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	//����
	point[0].SetPoint(m_rectPrice.left, VPOS(12));
	point[1].SetPoint(m_rectPrice.right, VPOS(12));
	pRT->DrawLines(point, 2);

	int pxLeft = m_rectPrice.left + 30;
	int pxRight = m_rectPrice.left + nWidth * 2 - 5;
	int volLeft = m_rectPrice.left + nWidth * 2;
	int volRight = m_rectPrice.left + nWidth * 4 - 40;
	int chgLeft = m_rectPrice.left + nWidth * 4 - 40;
	int chgRight = m_rectPrice.right;

	//��
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
			_swprintf(szTmp, L"��");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(11), volLeft + 8, VPOS(12)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.AskPrice[0] > preAsk1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"��");
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
			_swprintf(szTmp, L"��");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(12), volLeft + 8, VPOS(13)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else if (m_StockTick.BidPrice[0] > preBid1)
		{
			pRT->SetTextColor(RGBA(255, 31, 31, 255));
			_swprintf(szTmp, L"��");
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(pxRight, VPOS(12), volLeft + 8, VPOS(13)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		pRT->SelectObject(OldFont);

	}



	//ί��ί��
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


	//����
	point[0].SetPoint(m_rectPrice.left, VPOS(22));
	point[1].SetPoint(m_rectPrice.right, VPOS(22));
	pRT->DrawLines(point, 2);

	//��һ��
	left = m_rectPrice.left;
	right = m_rectPrice.left + 35;

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"����", 2,
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_LEFT);
	pRT->DrawTextW(L"�ǵ�", 2,
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_LEFT);
	pRT->DrawTextW(L"�Ƿ�", 2,
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_LEFT);
	pRT->DrawTextW(L"��ͣ", 2,
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_LEFT);
	pRT->DrawTextW(L"����", 2,
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_LEFT);
	pRT->DrawTextW(L"����", 2,
		CRect(left, VPOS(27), right, VPOS(28)),
		DT_LEFT);

	left = m_rectPrice.left + nWidth * 2 + 10;
	right = m_rectPrice.left + nWidth * 2 + 40;
	pRT->DrawTextW(L"����", 2,
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_LEFT);
	pRT->DrawTextW(L"���", 2,
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_LEFT);
	pRT->DrawTextW(L"���", 2,
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_LEFT);
	pRT->DrawTextW(L"��ͣ", 2,
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_LEFT);
	pRT->DrawTextW(L"���", 2,
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_LEFT);

	//����
	left = m_rectPrice.left + 35;
	right = m_rectPrice.left + nWidth * 2;
	pRT->SetTextColor(GetTextColor(m_StockTick.LastPrice));
	if (m_StockTick.LastPrice > 10000000
		|| m_StockTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"��");	//����
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
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice);	//����
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(22), right, VPOS(23)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.02f",
			m_StockTick.LastPrice - m_StockTick.PreCloPrice);	//�ǵ�
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(23), right, VPOS(24)),
			DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%",
			(m_StockTick.LastPrice - m_StockTick.PreCloPrice)
			/ (m_StockTick.PreCloPrice) * 100);	//�Ƿ�
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(left, VPOS(24), right, VPOS(25)),
			DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 31, 31, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 + m_fMaxChgPct));	//��ͣr
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	m_StockTick.Volume /= 100;
	if (m_StockTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%.2f��", m_StockTick.Volume / 10'000.0);	//����
	else
		_swprintf(szTmp, L"%lld", m_StockTick.Volume);

	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_RIGHT);

	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//����
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(27), right, VPOS(28)),
		DT_RIGHT);

	left = m_rectPrice.left + nWidth * 2 + 40;
	right = m_rectPrice.left + nWidth * 4;
	//����
	pRT->SetTextColor(GetTextColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"��");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//����
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(22), right, VPOS(23)),
		DT_RIGHT);

	//���
	pRT->SetTextColor(GetTextColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"��");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(23), right, VPOS(24)),
		DT_RIGHT);

	//���
	pRT->SetTextColor(GetTextColor(m_StockTick.LowPrice));
	if (m_StockTick.LowPrice > 10000000 || m_StockTick.LowPrice < 0)
		_swprintf(szTmp, L"��");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(24), right, VPOS(25)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%.02f",
		m_StockTick.PreCloPrice * (1 - m_fMaxChgPct));	//��ͣ
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(25), right, VPOS(26)),
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	if (m_StockTick.Turnover > 100'000'000'000)
		_swprintf(szTmp, L"%.0f��", m_StockTick.Turnover / 100'000'000);
	else if (m_StockTick.Turnover > 100'000'000)
		_swprintf(szTmp, L"%.02f��", m_StockTick.Turnover / 100'000'000);
	else if (m_StockTick.Turnover > 1'000'000)
		_swprintf(szTmp, L"%.0f��", m_StockTick.Turnover / 10'000);
	else
		_swprintf(szTmp, L"%.0f", m_StockTick.Turnover);	//��ͣ
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(left, VPOS(26), right, VPOS(27)),
		DT_RIGHT);


	//����
	point[0].SetPoint(m_rectPrice.left, VPOS(27) + 22);
	point[1].SetPoint(m_rectPrice.right, VPOS(27) + 22);
	pRT->DrawLines(point, 2);

	//����
	for (int i = VPOS(22); i < VPOS(28); i += 3)
		pRT->SetPixel(m_rectPrice.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

}

void STradeInfoPic::DrawDeal(IRenderTarget * pRT)		//��������Լ
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rectDeal.Height();
	int nLineCount = (m_rectDeal.Height() - RC_HEIGHT) / RC_HEIGHT - 2;
	int nWidth = m_rectDeal.Width() / 3;
	size_t nListNum = m_pStkMarketVec->size();
	//std::wstringstream wss;
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);

	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"ʱ��", wcslen(L"ʱ��"),
			CRect(m_rectDeal.left, m_rectDeal.top,
				m_rectDeal.left + nWidth, m_rectDeal.top + RC_HEIGHT),
			DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"�۸�", wcslen(L"�۸�"),
			CRect(m_rectDeal.left + nWidth, m_rectDeal.top,
				m_rectDeal.left + nWidth * 2, m_rectDeal.top + RC_HEIGHT),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"����", wcslen(L"����"),
			CRect(m_rectDeal.left + nWidth * 2, m_rectDeal.top,
				m_rectDeal.left + nWidth * 3, m_rectDeal.top + RC_HEIGHT),
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		vector<CommonStockMarket> MarketVec;
		for (int i = m_pStkMarketVec->size() - 1; i >= 0; --i)
		{
			auto &tick = m_pStkMarketVec->at(i);
			if (i != 0)
			{
				auto &preTick = m_pStkMarketVec->at(i - 1);
				if (tick.Volume == preTick.Volume
					|| tick.Volume < preTick.Volume)
					continue;
				MarketVec.emplace_back(tick);
				if (MarketVec.size() >= nLineCount + 1)
					break;
			}
			else if (tick.Volume != 0)
				MarketVec.emplace_back(tick);
		}
		int i = 0;
		int j = MarketVec.size() > nLineCount ?
			nLineCount - 1 : MarketVec.size() - 1;
		for (j; j >= 0; --j, ++i)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%02d:%02d:%02d",
				MarketVec[j].UpdateTime / 10'000,
				MarketVec[j].UpdateTime % 10'000 / 100,
				MarketVec[j].UpdateTime % 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rectDeal.left, m_rectDeal.top + RC_HEIGHT * (i + 1),
					m_rectDeal.left + nWidth, m_rectDeal.top + RC_HEIGHT * (i + 2)),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (MarketVec[j].LastPrice > MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (MarketVec[j].LastPrice < MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%.02f", MarketVec[j].LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rectDeal.left + nWidth,
					m_rectDeal.top + RC_HEIGHT * (i + 1),
					m_rectDeal.left + nWidth * 2,
					m_rectDeal.top + RC_HEIGHT * (i + 2)),
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			if (j != MarketVec.size() - 1)
				_swprintf(szTmp, L"%.0f",
					ceil((MarketVec[j].Volume -
						MarketVec[j + 1].Volume) *1.0 / 100));
			else
				_swprintf(szTmp, L"%d", MarketVec[j].Volume / 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rectDeal.left + nWidth * 2,
				m_rectDeal.top + RC_HEIGHT * (i + 1), m_rectDeal.left + nWidth * 3,
				m_rectDeal.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}

	}
	pRT->SelectObject(OldFont);

}

COLORREF STradeInfoPic::GetTextColor(double price)
{
	if (price > 10000000 || price < 0)
		return RGBA(255, 255, 255, 255);
	if (price > m_StockTick.PreCloPrice)
		return RGBA(255, 31, 31, 255);
	else if (price < m_StockTick.PreCloPrice)
		return RGBA(0, 255, 0, 255);
	return RGBA(255, 255, 255, 255);

}
