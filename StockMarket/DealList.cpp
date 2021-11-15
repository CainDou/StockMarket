#include "stdafx.h"
#include "DealList.h"
#include <unordered_map>
#include <sstream>
#include <map>

extern SStringWList g_arInsID;
extern SStringWList g_arInsName;
extern std::map<InsIDType, std::vector<RestoreTickType>> g_TickHash;
extern std::map<InsIDType, std::vector<KLineDataType>>g_KlineHash;
//extern std::vector<ComboInsType> g_ComboVec;
extern std::map<InsIDType, std::vector<GroupDataType>>g_GroupTickHash;
extern CRITICAL_SECTION g_csTick;
extern CRITICAL_SECTION g_csGroupTick;
extern std::map<InsIDType, std::vector<StockIndex_t>>g_StockIndexTickHash;
extern std::map<InsIDType, ComboInsType>g_GroupInsMap;


CDealList::CDealList()
{
	m_vTick.clear();
	m_bGroup = false;
	//m_nIndex = -1;
	m_strSubIns = "";
	m_bIsStockIndex = false;
	m_pGroupDataType = nullptr;
	/*
	int ntime = 150100100;
	ZeroMemory(m_list, sizeof(DEAL_SHOW_LIST) * MAX_LIST_NUM);
	for (int i=0;i<30;i++)
	{
	m_list[i].time = ntime;
	ntime += 100;
	m_list[i].price = rand() / 20 + 990;
	m_list[i].vol = i+1 + 1000000;
	}
	m_nListNum = 30;
	m_fPreClose = 1100;
	_swprintf(m_sDecimal, L"%%.0f");*/
}


CDealList::~CDealList()
{
}

//void SOUI::CDealList::SetShowData(int nIndex, bool bGroup)
//{
//	m_nIndex = nIndex;
//	m_bGroup = bGroup;
//	if (m_bGroup)
//	{
//		::EnterCriticalSection(&g_csTick);
//
//		double Ins1PreSett = 0, Ins2PreSett = 0;
//		if (isalpha(g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins1Num)[0]))
//			Ins1PreSett = g_TickHash[g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins1Num)].back().PreSettlementPrice;
//		else
//			Ins1PreSett = g_StockIndexTickHash[g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins1Num)].back().PreClosePrice;
//
//		if (isalpha(g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins2Num)[0]))
//			Ins2PreSett = g_TickHash[g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins2Num)].back().PreSettlementPrice;
//		else
//			Ins2PreSett = g_StockIndexTickHash[g_arInsID.GetAt(g_ComboVec[m_nIndex].Ins2Num)].back().PreClosePrice;
//		::LeaveCriticalSection(&g_csTick);
//
//		m_dPreSettle = g_ComboVec[m_nIndex].Ins1Ratio*Ins1PreSett -
//			g_ComboVec[m_nIndex].Ins2Ratio*Ins2PreSett;
//		m_dSettleRatio = (g_ComboVec[m_nIndex].Ins1Ratio*Ins1PreSett) /
//			(g_ComboVec[m_nIndex].Ins2Ratio*Ins2PreSett)*RATIOCOE;
//	}
//	else
//	{
//		if (isalpha(g_arInsID.GetAt(m_nIndex)[0]))
//			m_bIsStockIndex = false;
//		else
//			m_bIsStockIndex = true;
//	}
//
//}

void SOUI::CDealList::SetShowData(InsIDType strSubIns, bool bGroup)
{
	m_strSubIns = strSubIns;
	m_bGroup = bGroup;

	if (m_bGroup)
	{
		::EnterCriticalSection(&g_csTick);

		InsIDType InsID1 = g_GroupInsMap[strSubIns].Ins1;
		InsIDType InsID2 = g_GroupInsMap[strSubIns].Ins2;

		double Ins1PreSett = 0, Ins2PreSett = 0;
		if (isalpha(InsID1[0]))
		{
			m_bIsStockIndex = false;
			Ins1PreSett = g_TickHash[InsID1].back().PreSettlementPrice;
		}
		else
		{
			m_bIsStockIndex = true;
			Ins1PreSett = g_StockIndexTickHash[InsID1].back().PreClosePrice;
		}

		if (isalpha(InsID2[0]))
		{
			m_bIsStockIndex = false;
			Ins2PreSett = g_TickHash[InsID2].back().PreSettlementPrice;
		}
		else
		{
			m_bIsStockIndex &= true;
			Ins2PreSett = g_StockIndexTickHash[InsID2].back().PreClosePrice;
		}
		::LeaveCriticalSection(&g_csTick);

		m_dPreSettle = g_GroupInsMap[strSubIns].Ins1Ratio*Ins1PreSett -
			g_GroupInsMap[strSubIns].Ins2Ratio*Ins2PreSett;
		m_dSettleRatio = (g_GroupInsMap[strSubIns].Ins1Ratio*Ins1PreSett) /
			(g_GroupInsMap[strSubIns].Ins2Ratio*Ins2PreSett)*RATIOCOE;
	}
	else
	{
		if (isalpha(m_strSubIns[0]))
			m_bIsStockIndex = false;
		else
			m_bIsStockIndex = true;
	}

}

void SOUI::CDealList::Paint(IRenderTarget * pRT)
{
	if (m_strSubIns == "")
		return;
	SPainter pa;
	if (!m_bGroup)
	{
		if(!m_bIsStockIndex)
			DrawOneCode(pRT);
		else
			DrawIndexOneCode(pRT);
	}
	else
	{
		DrawGroupCode(pRT);
	}

}

void CDealList::DrawOneCode(IRenderTarget * pRT)		//画单个合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	::EnterCriticalSection(&g_csTick);
	auto nListNum = g_TickHash[m_strSubIns].size();
	std::vector<RestoreTickType> tickVec;
	if (nListNum >= 50)
		tickVec.assign(g_TickHash[m_strSubIns].cend() - 50, g_TickHash[m_strSubIns].cend());
	else
		tickVec.assign(g_TickHash[m_strSubIns].cend() - nListNum, g_TickHash[m_strSubIns].cend());
	::LeaveCriticalSection(&g_csTick);
	nListNum = tickVec.size();
	std::wstringstream wss;
	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		int i = 0;
		for (auto iter = tickVec.cbegin() + (nListNum - nLineCount); iter != tickVec.cend(); iter++, i++)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			wss.str(L"");
			wss << iter->UpdateTime;
			_swprintf(szTmp, L"%s.%d", wss.str().c_str(), iter->UpdateMillisec / 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (iter->LastPrice > iter->PreSettlementPrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (iter->LastPrice < iter->PreSettlementPrice)
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
			if (iter != tickVec.cbegin())
				_swprintf(szTmp, L"%d", iter->Volume - (iter - 1)->Volume);
			else
				_swprintf(szTmp, L"%d", 0);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}
}

void SOUI::CDealList::DrawIndexOneCode(IRenderTarget * pRT)		//画单个合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	::EnterCriticalSection(&g_csTick);
	int nListNum = g_StockIndexTickHash[m_strSubIns].size();
	std::vector<StockIndex_t> tickVec;
	if (nListNum >= 50)
		tickVec.assign(g_StockIndexTickHash[m_strSubIns].cend() - 50, g_StockIndexTickHash[m_strSubIns].cend());
	else
		tickVec.assign(g_StockIndexTickHash[m_strSubIns].cend() - nListNum, g_StockIndexTickHash[m_strSubIns].cend());
	::LeaveCriticalSection(&g_csTick);
	nListNum = tickVec.size();
	std::wstringstream wss;
	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"价格", wcslen(L"价格"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		int i = 0;
		for (auto iter = tickVec.cbegin() + (nListNum - nLineCount); iter != tickVec.cend(); iter++, i++)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			wss.str(L"");
			wss << iter->UpdateTime;
			_swprintf(szTmp, L"%s", wss.str().c_str());
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (iter->LastPrice > iter->PreClosePrice)
				pRT->SetTextColor(RGBA(255, 0, 0, 255));
			else if (iter->LastPrice < iter->PreClosePrice)
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
			if (iter != tickVec.cbegin())
				_swprintf(szTmp, L"%d万", (int)(iter->Turnover - (iter - 1)->Turnover)/10000);
			else
				_swprintf(szTmp, L"%d", 0);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}
}

void CDealList::DrawGroupCode(IRenderTarget * pRT)		//画组合合约
{
	wchar_t szTmp[100] = { 0 };
	int nHeightTotal = m_rect.Height();
	int nLineCount = m_rect.Height() / RC_HEIGHT - 1;
	int nWidth = m_rect.Width() / 3;
	::EnterCriticalSection(&g_csGroupTick);
	int nListNum = g_GroupTickHash[m_strSubIns].size();
	std::vector<GroupDataType> tickVec;
	if (nListNum >= 50)
		tickVec.assign(g_GroupTickHash[m_strSubIns].cend() - 50, g_GroupTickHash[m_strSubIns].cend());
	else
		tickVec.assign(g_GroupTickHash[m_strSubIns].cend() - nListNum, g_GroupTickHash[m_strSubIns].cend());
	::LeaveCriticalSection(&g_csGroupTick);
	nListNum = tickVec.size();

	std::wstringstream wss;

	if (nLineCount >= 0 && nWidth > 10)
	{
		pRT->SetTextColor(RGBA(255, 255, 0, 255));
		pRT->DrawTextW(L"时间", wcslen(L"时间"), CRect(m_rect.left, m_rect.top, m_rect.left + nWidth, m_rect.top + RC_HEIGHT), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		if (*m_pGroupDataType == 0)
			pRT->DrawTextW(L"价差", wcslen(L"价差"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		else if (*m_pGroupDataType == 1)
			pRT->DrawTextW(L"比值", wcslen(L"比值"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		else if (*m_pGroupDataType == 2)
			pRT->DrawTextW(L"涨跌幅差", wcslen(L"涨跌幅差"), CRect(m_rect.left + nWidth, m_rect.top, m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		pRT->DrawTextW(L"数量", wcslen(L"数量"), CRect(m_rect.left + nWidth * 2, m_rect.top, m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (nListNum < nLineCount)
			nLineCount = nListNum;
		int  i = 0;
		for (auto iter = tickVec.cbegin() + (nListNum - nLineCount); iter != tickVec.cend(); iter++, i++)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			wss.str(L"");
			wss << iter->UpdateTime;
			_swprintf(szTmp, L"%s.%d", wss.str().c_str(), iter->UpdateMillisec / 100);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 2)), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (*m_pGroupDataType == 0)
			{

				if (iter->dLml > 10000000)
				{
					pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"-");
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
				else
				{
					if (iter->dLml > m_dPreSettle)
						pRT->SetTextColor(RGBA(255, 0, 0, 255));
					else if (iter->dLml < m_dPreSettle)
						pRT->SetTextColor(RGBA(0, 255, 0, 255));
					else
						pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"%g", iter->dLml);
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
			}
			else if (*m_pGroupDataType == 1)
			{
				if (iter->dLdl > 10000000)
				{
					pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"-");
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
				else
				{
					if (iter->dLdl > m_dSettleRatio)
						pRT->SetTextColor(RGBA(255, 0, 0, 255));
					else if (iter->dLdl < m_dSettleRatio)
						pRT->SetTextColor(RGBA(0, 255, 0, 255));
					else
						pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"%.2f", iter->dLdl);
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
				/*
				if (j > 0)
				_swprintf(szTmp, L"%d", (p->totalvol - (p - 1)->totalvol >= 0 ? p->totalvol - (p - 1)->totalvol : 0) );
				else
				_swprintf(szTmp, L"%d", (p->totalvol - m_pTbl->pHq.nVolume >= 0 ? p->totalvol - m_pTbl->pHq.nVolume : 0) );*/

			}
			else if (*m_pGroupDataType == 2)
			{
				if (iter->dLpmLp > 10000000)
				{
					pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"-");
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
				else
				{
					if (iter->dLpmLp > 0)
						pRT->SetTextColor(RGBA(255, 0, 0, 255));
					else if (iter->dLpmLp < 0)
						pRT->SetTextColor(RGBA(0, 255, 0, 255));
					else
						pRT->SetTextColor(RGBA(255, 255, 255, 255));
					ZeroMemory(szTmp, sizeof(wchar_t) * 100);
					_swprintf(szTmp, L"%.2f", iter->dLpmLp);
					pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}

			}
			ZeroMemory(szTmp, sizeof(wchar_t) * 100);
			if (!m_bIsStockIndex)
			{
				if (iter != tickVec.cbegin())
					_swprintf(szTmp, L"%d", iter->Volume - (iter - 1)->Volume);
				else
					_swprintf(szTmp, L"%d", 0);
			}
			else
			{
				if (iter != tickVec.cbegin())
					_swprintf(szTmp, L"%d万", (int)(iter->Volume - (iter - 1)->Volume));
				else
					_swprintf(szTmp, L"%d", 0);

			}

			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_HEIGHT * (i + 1), m_rect.left + nWidth * 3 , m_rect.top + RC_HEIGHT * (i + 2)), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		}
	}
}