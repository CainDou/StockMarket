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
	m_bInited = FALSE;
}


CDealList::~CDealList()
{
}



void CDealList::SetDataPoint(vector<CommonStockMarket>* pStkMarketVec)
{
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;

}

void CDealList::SetDataPoint(vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;

}


void CDealList::ChangeShowData(SStringA StockID)
{
	m_strSubID = StockID;
}


void CDealList::Paint(IRenderTarget * pRT)
{
	if (!m_bInited)
	{
		m_bInited = TRUE;
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 15;// 字体大小
		_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
		GETRENDERFACTORY->CreateFont(&m_pFont15, lf);

	}
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
	//std::wstringstream wss;
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);

	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

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
			else if(tick.Volume !=0)
				MarketVec.emplace_back(tick);
		}
		int i = 0;
		int j = MarketVec.size() > nLineCount ? 
			nLineCount - 1 : MarketVec.size() - 1;
		for ( j; j >= 0; --j,++i)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%02d:%02d:%02d", MarketVec[j].UpdateTime / 10'000, MarketVec[j].UpdateTime % 10'000 / 100, MarketVec[j].UpdateTime % 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (MarketVec[j].LastPrice > MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (MarketVec[j].LastPrice < MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%.02f", MarketVec[j].LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp),
				CRect(m_rect.left + nWidth, 
					m_rect.top + RC_HEIGHT * (i + 1), 
					m_rect.left + nWidth * 2,
					m_rect.top + RC_HEIGHT * (i + 2)), 
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			/*if (j > 0)
			_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
			else
			_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
			if (j != MarketVec.size() -1)
				_swprintf(szTmp, L"%.0f", 
					ceil((MarketVec[j].Volume -
						MarketVec[j+1].Volume) *1.0 / 100));
			else
				_swprintf(szTmp, L"%d", MarketVec[j].Volume / 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}

		//for (auto iter = m_pStkMarketVec->cbegin() + (nListNum - nLineCount); 
		//	iter != m_pStkMarketVec->cend(); iter++, i++)
		//{
		//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	_swprintf(szTmp, L"%02d:%02d:%02d",iter->UpdateTime / 10'000,iter->UpdateTime %10'000 /100, iter->UpdateTime %100);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		//	if (iter->LastPrice > iter->PreCloPrice)
		//		pRT->SetTextColor(RGBA(255, 0, 0, 255));
		//	else if (iter->LastPrice < iter->PreCloPrice)
		//		pRT->SetTextColor(RGBA(0, 255, 0, 255));
		//	else
		//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	_swprintf(szTmp, L"%.02f", iter->LastPrice);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	/*if (j > 0)
		//	_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
		//	else
		//	_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
		//	if (iter != m_pStkMarketVec->cbegin())
		//		_swprintf(szTmp, L"%.0f", ceil((iter->Volume - (iter - 1)->Volume) *1.0 / 100));
		//	else
		//		_swprintf(szTmp, L"%d", iter->Volume);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		//}
	}
	pRT->SelectObject(OldFont);

}

void CDealList::DrawIndexOneCode(IRenderTarget * pRT)		//画单个合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	size_t nListNum = m_pIdxMarketVec->size();
	CAutoRefPtr<IFont> OldFont;
	pRT->SelectObject(m_pFont15, (IRenderObj**)&OldFont);
	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"金额", wcslen(L"金额"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		vector<CommonIndexMarket> MarketVec;
		for (int i = m_pIdxMarketVec->size() - 1; i >= 0; --i)
		{
			auto &tick = m_pIdxMarketVec->at(i);
			if (i != 0)
			{
				auto &preTick = m_pIdxMarketVec->at(i - 1);
				if (tick.Turnover == preTick.Turnover
					|| tick.Turnover < preTick.Turnover)
					continue;
				MarketVec.emplace_back(tick);
				if (MarketVec.size() >= nLineCount + 1)
					break;
			}
			else if(tick.Turnover != 0)
				MarketVec.emplace_back(tick);
		}
		int i = 0;
		int j = MarketVec.size() > nLineCount ?
			nLineCount - 1 : MarketVec.size() - 1;
		for (j; j >= 0; --j, ++i)

		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%02d:%02d:%02d", MarketVec[j].UpdateTime / 10'000,
				MarketVec[j].UpdateTime % 10'000 / 100, 
				MarketVec[j].UpdateTime % 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (MarketVec[j].LastPrice >  MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (MarketVec[j].LastPrice <  MarketVec[j].PreCloPrice)
				pRT->SetTextColor(RGBA(0, 255, 0, 255));
			else
				pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			_swprintf(szTmp, L"%.02f", MarketVec[j].LastPrice);
			pRT->DrawTextW(szTmp, wcslen(szTmp), 
				CRect(m_rect.left + nWidth, 
					m_rect.top + RC_HEIGHT * (i + 1),
					m_rect.left + nWidth * 2, 
					m_rect.top + RC_HEIGHT * (i + 2)), 
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			/*if (j > 0)
			_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
			else
			_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
			double diff = 0;
			if (j != MarketVec.size() - 1)
				diff = MarketVec[j].Turnover - MarketVec[j + 1].Turnover;
			else
				diff = MarketVec[j].Turnover;
			if (diff > 1'000'000'000)
				_swprintf(szTmp, L"%.01f亿", diff / 100'000'000);
			else if(diff > 100'000'000)
				_swprintf(szTmp, L"%.02f亿", diff / 100'000'000);
			else if(diff > 1'000'000)
				_swprintf(szTmp, L"%.0f万", diff / 10000);
			else if (diff > 10'000)
				_swprintf(szTmp, L"%.02f万", diff / 10000);
			else
				_swprintf(szTmp, L"%.0f", diff );

				//_swprintf(szTmp, L"%d万", MarketVec[j].Turnover / 10000);
			pRT->DrawTextW(szTmp, wcslen(szTmp), 
				CRect(m_rect.left + nWidth * 2, 
					m_rect.top + RC_HEIGHT * (i + 1),
					m_rect.left + nWidth * 3,
					m_rect.top + RC_HEIGHT * (i + 2)), 
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
		//for (auto iter = m_pIdxMarketVec->cbegin() + (nListNum - nLineCount); iter != m_pIdxMarketVec->cend(); iter++, i++)
		//{
		//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	_swprintf(szTmp, L"%02d:%02d:%02d", iter->UpdateTime / 10'000, iter->UpdateTime % 10'000 / 100, iter->UpdateTime % 100);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		//	if (iter->LastPrice > iter->PreCloPrice)
		//		pRT->SetTextColor(RGBA(255, 0, 0, 255));
		//	else if (iter->LastPrice < iter->PreCloPrice)
		//		pRT->SetTextColor(RGBA(0, 255, 0, 255));
		//	else
		//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	_swprintf(szTmp, L"%.02f", iter->LastPrice);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		//	ZeroMemory(szTmp, sizeof(wchar_t) * 100);
		//	/*if (j > 0)
		//	_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0));
		//	else
		//	_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0));*/
		//	if (iter != m_pIdxMarketVec->cbegin())
		//	{
		//		int diff = (int)(iter->Turnover - (iter - 1)->Turnover);
		//		if(diff == 0)
		//			_swprintf(szTmp, L"%d", 0);
		//		else
		//			_swprintf(szTmp, L"%d万", (int)(iter->Turnover - (iter - 1)->Turnover) / 10000);
		//	}
		//	else
		//		_swprintf(szTmp, L"%d", 0);
		//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		//}
	}
	pRT->SelectObject(OldFont);
}
