#include "stdafx.h"
#include "DealList.h"
#include <unordered_map>
#include <sstream>
#include <map>


CDealList::CDealList()
{
	m_vTick.clear();
	m_bGroup = false;
	//m_nIndex = -1;
	m_strSubID = "";
	m_bIsStockIndex = false;
	m_pGroupDataType = nullptr;
}


CDealList::~CDealList()
{
}



void CDealList::SetShowData(SStringA StockID, vector<CommonStockMarket>* pStkMarketVec)
{
	m_strSubID = StockID;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
}

void CDealList::SetShowData(SStringA StockID, vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_strSubID = StockID;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
}

void CDealList::Paint(IRenderTarget * pRT)
{
	if (m_strSubID == "")
		return;
	SPainter pa;
	if (!m_bGroup)
	{
		if (!m_bIsStockIndex)
			DrawStockOneCode(pRT);
		else
			DrawIndexOneCode(pRT);
	}

}

void CDealList::DrawStockOneCode(IRenderTarget * pRT)		//画单个合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	size_t nListNum = m_pStkMarketVec->size();
	std::wstringstream wss;
	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		int i = 0;
		for (auto iter = m_pStkMarketVec->cbegin() + (nListNum - nLineCount); iter != m_pStkMarketVec->cend(); iter++, i++)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			wss.str(L"");
			wss << iter->UpdateTime;
			_swprintf(szTmp, L"%s", wss.str().c_str());
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (iter->LastPrice > iter->PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (iter->LastPrice < iter->PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%g", iter->LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			/*if (j > 0)
			_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
			else
			_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
			if (iter != m_pStkMarketVec->cbegin())
				_swprintf(szTmp, L"%d", iter->Volume - (iter - 1)->Volume);
			else
				_swprintf(szTmp, L"%d", iter->Volume);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}
}

void CDealList::DrawIndexOneCode(IRenderTarget * pRT)		//画单个合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	size_t nListNum = m_pIdxMarketVec->size();
	std::wstringstream wss;
	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		int i = 0;
		for (auto iter = m_pIdxMarketVec->cbegin() + (nListNum - nLineCount); iter != m_pIdxMarketVec->cend(); iter++, i++)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			wss.str(L"");
			wss << iter->UpdateTime;
			_swprintf(szTmp, L"%s", wss.str().c_str());
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (iter->LastPrice > iter->PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (iter->LastPrice < iter->PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%g", iter->LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			/*if (j > 0)
			_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
			else
			_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
			if (iter != m_pIdxMarketVec->cbegin())
				_swprintf(szTmp, L"%d万", (int)(iter->Turnover - (iter - 1)->Turnover) / 10000);
			else
				_swprintf(szTmp, L"%d", 0);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}
}
