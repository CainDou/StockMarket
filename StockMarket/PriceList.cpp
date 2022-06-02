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
	m_bInsInited = FALSE;
	//OutputDebugString(L"赋值不是stockIndex\n");
}

void CPriceList::SetShowData(SStringA StockID, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec)
{
	m_bInsInited = FALSE;
	m_strSubIns = StockID;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
	m_strStockName = StockName;
	if (m_strSubIns[0] == '3' || m_strSubIns.Find("688") != -1)
		m_fMaxChgPct = 0.2;
	else
		m_fMaxChgPct = 0.1;
	m_bInsInited = TRUE;
}

void CPriceList::SetShowData(SStringA StockID, SStringA StockName, vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_bInsInited = FALSE;
	m_strSubIns = StockID;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
	m_strStockName = StockName;
	m_bInsInited = TRUE;
}


CPriceList::~CPriceList()
{
}

void SOUI::CPriceList::Paint(IRenderTarget * pRT)
{
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

	if (m_bInsInited)
	{
		if (!m_bIsStockIndex)
		{
			if (!m_pStkMarketVec->empty())
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
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + 40, m_rect.top, m_rect.left + nWidth * 2 + 80, m_rect.top + RC_PRH), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	strTmp = L"";
	//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top, m_rect.right, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SelectObject(OldFont);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH + 4);
	point[1].SetPoint(m_rect.right + 5, m_rect.top + RC_PRH + 4);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"卖十", wcslen(L"卖十"), CRect(m_rect.left + 10, m_rect.top + RC_PRH, m_rect.left + 40, m_rect.top + RC_PRH * 2), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖九", wcslen(L"卖九"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 2, m_rect.left + 40, m_rect.top + RC_PRH * 3), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖八", wcslen(L"卖八"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 3, m_rect.left + 40, m_rect.top + RC_PRH * 4), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖七", wcslen(L"卖七"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 4, m_rect.left + 40, m_rect.top + RC_PRH * 5), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖六", wcslen(L"卖六"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 5, m_rect.left + 40, m_rect.top + RC_PRH * 6), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖五", wcslen(L"卖五"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 6, m_rect.left + 40, m_rect.top + RC_PRH * 7), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖四", wcslen(L"卖四"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 7, m_rect.left + 40, m_rect.top + RC_PRH * 8), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖三", wcslen(L"卖三"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 8, m_rect.left + 40, m_rect.top + RC_PRH * 9), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖二", wcslen(L"卖二"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 9, m_rect.left + 40, m_rect.top + RC_PRH * 10), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"卖一", wcslen(L"卖一"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 10, m_rect.left + 40, m_rect.top + RC_PRH * 11), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pRT->DrawTextW(L"买一", wcslen(L"买一"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 11, m_rect.left + 40, m_rect.top + RC_PRH * 12), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买二", wcslen(L"买二"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 12, m_rect.left + 40, m_rect.top + RC_PRH * 13), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买三", wcslen(L"买三"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 13, m_rect.left + 40, m_rect.top + RC_PRH * 14), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买四", wcslen(L"买四"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 14, m_rect.left + 40, m_rect.top + RC_PRH * 15), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买五", wcslen(L"买五"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 15, m_rect.left + 40, m_rect.top + RC_PRH * 16), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买六", wcslen(L"买六"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 16, m_rect.left + 40, m_rect.top + RC_PRH * 17), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买七", wcslen(L"买七"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 17, m_rect.left + 40, m_rect.top + RC_PRH * 18), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买八", wcslen(L"买八"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 18, m_rect.left + 40, m_rect.top + RC_PRH * 19), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买九", wcslen(L"买九"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 19, m_rect.left + 40, m_rect.top + RC_PRH * 20), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pRT->DrawTextW(L"买十", wcslen(L"买十"), CRect(m_rect.left + 10, m_rect.top + RC_PRH * 20, m_rect.left + 40, m_rect.top + RC_PRH * 21), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH * 11);
	point[1].SetPoint(m_rect.right + 5, m_rect.top + RC_PRH * 11);
	pRT->DrawLines(point, 2);

	//量
	if (m_StockTick.AskVolume[9] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[9] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH + 5,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[9]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[9]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[8] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[8] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 2,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[8]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[8]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 2,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[7] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[7] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 3,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 4), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[7]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[7]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 3,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 4), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[6] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[6] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 4,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 5), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[6]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[6]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 4,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 5), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[5] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[5] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 5,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 6), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[5]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[5]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 5,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 6), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[4] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[4] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 6,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 7), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[4]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[4]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 6,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 7), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.AskVolume[3] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[3] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 7,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 8), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[3]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[3]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 7,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 8), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[2] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[2] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 8,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 9), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[2]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[2]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 8,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 9), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[1] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[1] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 9,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 10), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[1]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[1]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 9,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 10), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}
	if (m_StockTick.AskVolume[0] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.AskVolume[0] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 10,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 11), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[0]));
		_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[0]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 10,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 11), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}



	if (m_StockTick.BidVolume[0] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[0] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 11,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 12), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[0]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[0]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 11,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 12), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[1] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[1] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 12,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 13), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[1]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[1]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 12,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 13), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[2] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[2] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 13,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 14), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[2]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[2]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 13,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 14), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[3] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[3] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 14,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 15), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[3]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[3]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 14,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 15), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[4] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[4] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 15,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 16), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[4]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[4]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 15,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 16), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[5] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[5] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 16,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 17), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[5]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[5]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 16,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 17), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[6] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[6] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 17,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 18), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[6]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[6]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 17,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 18), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[7] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[7] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 18,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 19), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[7]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[7]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 18,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 19), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[8] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[8] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 19,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 20), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[8]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[8]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 19,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 20), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	if (m_StockTick.BidVolume[9] > 0)
	{
		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		_swprintf(szTmp, L"%d", m_StockTick.BidVolume[9] / 100);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 20,
			m_rect.left + nWidth * 4 - 10, m_rect.top + RC_PRH * 21), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[9]));
		_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[9]);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 20,
			m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 21), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	}

	//买一卖一价
	//pRT->SetTextColor(GetTextColor(m_StockTick.AskPrice[0]));
	//_swprintf(szTmp, L"%.02f", m_StockTick.AskPrice[0]);
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5,
	//	m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//pRT->SetTextColor(GetTextColor(m_StockTick.BidPrice[0]));
	//_swprintf(szTmp, L"%.02f", m_StockTick.BidPrice[0]);
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH * 2,
	//	m_rect.left + nWidth * 2, m_rect.top + RC_PRH * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH * 21);
	point[1].SetPoint(m_rect.right, m_rect.top + RC_PRH * 21);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 21 , m_rect.left + 30, m_rect.top + (RC_PRH) * 22), DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 22, m_rect.left + 30, m_rect.top + (RC_PRH) * 23), DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 23, m_rect.left + 30, m_rect.top + (RC_PRH) * 24), DT_LEFT);
	pRT->DrawTextW(L"涨停", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 24, m_rect.left + 30, m_rect.top + (RC_PRH) * 25), DT_LEFT);
	pRT->DrawTextW(L"总手", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 25, m_rect.left + 30, m_rect.top + (RC_PRH) * 26), DT_LEFT);
	pRT->DrawTextW(L"昨收", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 26, m_rect.left + 30, m_rect.top + (RC_PRH) * 27), DT_LEFT);

	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 21, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 22), DT_LEFT);
	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 22, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 23), DT_LEFT);
	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 23, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 24), DT_LEFT);
	pRT->DrawTextW(L"跌停", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 24, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 25), DT_LEFT);
	pRT->DrawTextW(L"金额", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 25, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 26), DT_LEFT);

	pRT->SetTextColor(GetTextColor(m_StockTick.LastPrice));
	if (m_StockTick.LastPrice > 10000000 || m_StockTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 21, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 22), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 22, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 23), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 23, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 24), DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 21, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 22), DT_RIGHT);
		_swprintf(szTmp, L"%.02f", m_StockTick.LastPrice - m_StockTick.PreCloPrice);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 22, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 23), DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%", (m_StockTick.LastPrice - m_StockTick.PreCloPrice) / (m_StockTick.PreCloPrice) * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 23, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 24), DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice * (1 + m_fMaxChgPct));	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 24, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 25), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	m_StockTick.Volume /= 100;
	if (m_StockTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%.2f万", m_StockTick.Volume / 10'000.0);	//昨收
	else
		_swprintf(szTmp, L"%lld", m_StockTick.Volume);	//昨收

	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 25, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 26), DT_RIGHT);

	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 26, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 27), DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_StockTick.OpenPrice));
	if (m_StockTick.OpenPrice > 10000000 || m_StockTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 21, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 22), DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_StockTick.HighPrice));
	if (m_StockTick.HighPrice > 10000000 || m_StockTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 22, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 23), DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_StockTick.LowPrice));
	if (m_StockTick.LowPrice > 10000000 || m_StockTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_StockTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 23, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 24), DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%.02f", m_StockTick.PreCloPrice * (1 - m_fMaxChgPct));	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 24, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 25), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	//	pRT->DrawTextW(L"--", 2, CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH)* 25, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH)* 26), DT_RIGHT);
	if (m_StockTick.Turnover > 100'000'000'000)
		_swprintf(szTmp, L"%.0f亿", m_StockTick.Turnover / 100'000'000);	//跌停
	else if (m_StockTick.Turnover > 100'000'000)
		_swprintf(szTmp, L"%.02f亿", m_StockTick.Turnover / 100'000'000);	//跌停
	else if (m_StockTick.Turnover > 1'000'000)
		_swprintf(szTmp, L"%.02f万", m_StockTick.Turnover / 10'000);	//跌停
	else
		_swprintf(szTmp, L"%.0f", m_StockTick.Turnover);	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 25, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 26), DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH) * 26 + 22);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH) * 26 + 22);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = m_rect.top + RC_PRH * 21; i < m_rect.top + (RC_PRH) * 26; i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

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
	SStringW strTmp = StrA2StrW(m_strStockName);
	pRT->DrawTextW(strTmp, strTmp.GetLength(), 
		CRect(m_rect.left + 40, 
			m_rect.top-5, 
			m_rect.left + nWidth * 2 + 80, 
			m_rect.top + RC_PRH), 
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	strTmp = L"";
	//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top, m_rect.right, m_rect.top + RC_PRH + 5), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SelectObject(OldFont);
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) + 4);
	point[1].SetPoint(m_rect.right + 5, m_rect.top + (RC_PRH + 5) + 4);
	pRT->DrawLines(point, 2);

	//pRT->SetTextColor(RGBA(255, 255, 255, 255));
	//pRT->DrawTextW(L"卖", wcslen(L"卖"), CRect(m_rect.left + 10, m_rect.top + RC_PRH + 5, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	//pRT->DrawTextW(L"买", wcslen(L"买"), CRect(m_rect.left + 10, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 3), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	////横线
	//point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 2);
	//point[1].SetPoint(m_rect.right + 5, m_rect.top + (RC_PRH + 5) * 2);
	//pRT->DrawLines(point, 2);

	////量
	//_swprintf(szTmp, L"-");
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//_swprintf(szTmp, L"-");
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	////买一卖一价
	//pRT->SetTextColor(RGBA(255, 255, 0, 0));
	//_swprintf(szTmp, L"-");
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//_swprintf(szTmp, L"-");
	//pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	////横线
	//point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 3);
	//point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH + 5) * 3);
	//pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2, 
		CRect(m_rect.left, 
			m_rect.top + (RC_PRH + 5) * 1 + 20, 
			m_rect.left + 30, 
			m_rect.top + (RC_PRH + 5) * 2 + 20), 
		DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, 
		CRect(m_rect.left, 
			m_rect.top + (RC_PRH + 5) * 2 + 20, 
			m_rect.left + 30, 
			m_rect.top + (RC_PRH + 5) * 3 + 20),
		DT_LEFT);
	pRT->DrawTextW(L"涨幅", 2, 
		CRect(m_rect.left, 
			m_rect.top + (RC_PRH + 5) * 3 + 20, 
			m_rect.left + 30,
			m_rect.top + (RC_PRH + 5) * 4 + 20), 
		DT_LEFT);
	pRT->DrawTextW(L"总手", 2,
		CRect(m_rect.left, 
			m_rect.top + (RC_PRH + 5) * 4 + 20, 
			m_rect.left + 30,
			m_rect.top + (RC_PRH + 5) * 5 + 20),
		DT_LEFT);
	pRT->DrawTextW(L"昨收", 2,
		CRect(m_rect.left, 
			m_rect.top + (RC_PRH + 5) * 5 + 20,
			m_rect.left + 30, 
			m_rect.top + (RC_PRH + 5) * 6 + 20),
		DT_LEFT);

	pRT->DrawTextW(L"开盘", 2, 
		CRect(m_rect.left + nWidth * 2 + 10,
			m_rect.top + (RC_PRH + 5) * 1 + 20,
			m_rect.left + nWidth * 2 + 40, 
			m_rect.top + (RC_PRH + 5) * 2 + 20), 
		DT_LEFT);
	pRT->DrawTextW(L"最高", 2, 
		CRect(m_rect.left + nWidth * 2 + 10,
			m_rect.top + (RC_PRH + 5) * 2 + 20, 
			m_rect.left + nWidth * 2 + 40,
			m_rect.top + (RC_PRH + 5) * 3 + 20),
		DT_LEFT);
	pRT->DrawTextW(L"最低", 2,
		CRect(m_rect.left + nWidth * 2 + 10,
			m_rect.top + (RC_PRH + 5) * 3 + 20,
			m_rect.left + nWidth * 2 + 40, 
			m_rect.top + (RC_PRH + 5) * 4 + 20), 
		DT_LEFT);
	pRT->DrawTextW(L"金额", 2, 
		CRect(m_rect.left + nWidth * 2 + 10,
			m_rect.top + (RC_PRH + 5) * 4 + 20,
			m_rect.left + nWidth * 2 + 40,
			m_rect.top + (RC_PRH + 5) * 5 + 20),
		DT_LEFT);

	pRT->SetTextColor(GetTextColor(m_IndexTick.LastPrice));
	if (m_IndexTick.LastPrice > 10000000 || m_IndexTick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp),
			CRect(m_rect.left + 30,
				m_rect.top + (RC_PRH + 5) * 1 + 20, 
				m_rect.left + nWidth * 2,
				m_rect.top + (RC_PRH + 5) * 2 + 20), 
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), 
			CRect(m_rect.left + 30,
				m_rect.top + (RC_PRH + 5) * 2 + 20, 
				m_rect.left + nWidth * 2, 
				m_rect.top + (RC_PRH + 5) * 3 + 20), 
			DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), 
			CRect(m_rect.left + 30, 
				m_rect.top + (RC_PRH + 5) * 3 + 20, 
				m_rect.left + nWidth * 2, 
				m_rect.top + (RC_PRH + 5) * 4 + 20), 
			DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%.02f", m_IndexTick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), 
			CRect(m_rect.left + 30, 
				m_rect.top + (RC_PRH + 5) * 1 + 20, 
				m_rect.left + nWidth * 2, 
				m_rect.top + (RC_PRH + 5) * 2 + 20),
			DT_RIGHT);
		double change = m_IndexTick.LastPrice - m_IndexTick.PreCloPrice;
		_swprintf(szTmp, L"%.2f", change);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp), 
			CRect(m_rect.left + 30,
				m_rect.top + (RC_PRH + 5) * 2 + 20, 
				m_rect.left + nWidth * 2,
				m_rect.top + (RC_PRH + 5) * 3 + 20),
			DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%", change / m_IndexTick.PreCloPrice * 100);//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp), 
			CRect(m_rect.left + 30, 
				m_rect.top + (RC_PRH + 5) * 3 + 20,
				m_rect.left + nWidth * 2, 
				m_rect.top + (RC_PRH + 5) * 4 + 20),
			DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	if (m_IndexTick.SecurityID[0] == '8')
		m_IndexTick.Volume /= 100;
	if (m_IndexTick.Volume > 1'000'000)
		_swprintf(szTmp, L"%d万", m_IndexTick.Volume / 10'000);	//涨停r
	else
		_swprintf(szTmp, L"%d", m_IndexTick.Volume);	//涨停r

	pRT->DrawTextW(szTmp, wcslen(szTmp), 
		CRect(m_rect.left + 30,
			m_rect.top + (RC_PRH + 5) * 4 + 20, 
			m_rect.left + nWidth * 2, 
			m_rect.top + (RC_PRH + 5) * 5 + 20), 
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%.02f", m_IndexTick.PreCloPrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp),
		CRect(m_rect.left + 30, 
			m_rect.top + (RC_PRH + 5) * 5 + 20, 
			m_rect.left + nWidth * 2, 
			m_rect.top + (RC_PRH + 5) * 6 + 20),
		DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_IndexTick.OpenPrice));
	if (m_IndexTick.OpenPrice > 10000000 || m_IndexTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), 
		CRect(m_rect.left + nWidth * 2 + 40, 
			m_rect.top + (RC_PRH + 5) * 1 + 20, 
			m_rect.left + nWidth * 4, 
			m_rect.top + (RC_PRH + 5) * 2 + 20),
		DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_IndexTick.HighPrice));
	if (m_IndexTick.HighPrice > 10000000 || m_IndexTick.HighPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.HighPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), 
		CRect(m_rect.left + nWidth * 2 + 40, 
			m_rect.top + (RC_PRH + 5) * 2 + 20,
			m_rect.left + nWidth * 4, 
			m_rect.top + (RC_PRH + 5) * 3 + 20),
		DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_IndexTick.LowPrice));
	if (m_IndexTick.LowPrice > 10000000 || m_IndexTick.LowPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%.02f", m_IndexTick.LowPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), 
		CRect(m_rect.left + nWidth * 2 + 40,
			m_rect.top + (RC_PRH + 5) * 3 + 20, 
			m_rect.left + nWidth * 4, 
			m_rect.top + (RC_PRH + 5) * 4 + 20), 
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));

	//金额
	if (m_IndexTick.Turnover > 10'000'000'000)
		_swprintf(szTmp, L"%.0f亿", m_IndexTick.Turnover / 100'000'000);	
	else if (m_IndexTick.Turnover > 100'000'000)
		_swprintf(szTmp, L"%.02f亿", m_IndexTick.Turnover / 100'000'000);	//跌停
	else if (m_IndexTick.Turnover > 1'000'000)
		_swprintf(szTmp, L"%.02f万", m_IndexTick.Turnover / 10'000);	//跌停
	else
		_swprintf(szTmp, L"%.0f", m_IndexTick.Turnover);	//跌停

	//_swprintf(szTmp, L"-");	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), 
		CRect(m_rect.left + nWidth * 2 + 40, 
			m_rect.top + (RC_PRH + 5) * 4 + 20,
			m_rect.left + nWidth * 4, 
			m_rect.top + (RC_PRH + 5) * 5 + 20), 
		DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
	//	pRT->DrawTextW(L"--", 2, CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH + 5)* 7 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH + 5)* 8 + 20), DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH + 5) * 6 + 17);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH + 5) * 6 + 17);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = m_rect.top + (RC_PRH + 5) * 1; i < m_rect.top + (RC_PRH + 5) * 6 + 17; i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
	pRT->SelectObject(OldFont);

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


