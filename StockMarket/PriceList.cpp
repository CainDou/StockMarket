#include "stdafx.h"
#include"PriceList.h"
#include "common.h"
#include <unordered_map>
#include<map>
extern SStringWList g_arInsID;
extern SStringWList g_arInsName;
extern std::map<InsIDType, std::vector<RestoreTickType>> g_TickHash;
extern std::map<InsIDType, std::vector<KLineDataType>>g_KlineHash;
//extern std::vector<GroupInsType> g_GroupVec;
extern std::map<InsIDType, std::vector<GroupDataType>>g_GroupTickHash;
extern std::map<InsIDType, double> g_UpperLimitHash;
extern std::map<InsIDType, double> g_LowerLimitHash;
extern std::map<InsIDType, std::vector<StockIndex_t>>g_StockIndexTickHash;
extern std::map<InsIDType, ComboInsType>g_GroupInsMap;

extern CRITICAL_SECTION g_csTick;
extern CRITICAL_SECTION g_csGroupTick;

CPriceList::CPriceList()
{
	m_strSubIns = "";
//	m_nIndex = -1;
	m_bInit = FALSE;
	m_bGroup = FALSE;
	m_groupSettle = 0;
	m_bIsStockIndex = false;
	m_pGroupDataType = nullptr;
}

//void CPriceList::SetShowData(int nIndex, bool bGroup)
//{
//
//	m_nIndex = nIndex;
//	m_bGroup = bGroup;
//	if (m_bGroup)
//	{
//		::EnterCriticalSection(&g_csTick);
//		double Ins1PreSett = 0, Ins2PreSett = 0;
//		if (isalpha(g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins1Num)[0]))
//			Ins1PreSett = g_TickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins1Num)].back().PreSettlementPrice;
//		else
//			Ins1PreSett = g_StockIndexTickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins1Num)].back().PreClosePrice;
//
//		if (isalpha(g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins2Num)[0]))
//			Ins2PreSett = g_TickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins2Num)].back().PreSettlementPrice;
//		else
//			Ins2PreSett = g_StockIndexTickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins2Num)].back().PreClosePrice;
//		::LeaveCriticalSection(&g_csTick);
//
//		m_groupSettle = g_GroupVec[m_nIndex].Ins1Ratio*Ins1PreSett -
//			g_GroupVec[m_nIndex].Ins2Ratio*Ins2PreSett;
//		m_groupSettleRatio = (g_GroupVec[m_nIndex].Ins1Ratio*Ins1PreSett) /
//			(g_GroupVec[m_nIndex].Ins2Ratio*Ins2PreSett)*RATIOCOE;
//	}
//	else
//	{
//		if (isalpha(g_arInsID.GetAt(m_nIndex)[0]))
//			m_bIsStockIndex = false;
//		else
//			m_bIsStockIndex = true;
//	}
//	//	m_pTick = &g_TickHash[g_arInsID.GetAt(m_nIndex)].back();
//}

void SOUI::CPriceList::SetShowData(InsIDType strSubIns, bool bGroup)
{

	m_strSubIns = strSubIns;
	m_bGroup = bGroup;
	if (m_bGroup)
	{
		InsIDType InsID1 = g_GroupInsMap[strSubIns].Ins1;
		InsIDType InsID2 = g_GroupInsMap[strSubIns].Ins2;

		::EnterCriticalSection(&g_csTick);
		double Ins1PreSett = 0, Ins2PreSett = 0;
		int nGroupPos = atoi(m_strSubIns.Right(2));
		if (isalpha(InsID1[0]))
			Ins1PreSett = g_TickHash[InsID1].back().PreSettlementPrice;
		else
			Ins1PreSett = g_StockIndexTickHash[InsID1].back().PreClosePrice;

		if (isalpha(InsID2[0]))
			Ins2PreSett = g_TickHash[InsID2].back().PreSettlementPrice;
		else
			Ins2PreSett = g_StockIndexTickHash[InsID2].back().PreClosePrice;
		::LeaveCriticalSection(&g_csTick);

		m_groupSettle = g_GroupInsMap[strSubIns].Ins1Ratio*Ins1PreSett -
			g_GroupInsMap[strSubIns].Ins2Ratio*Ins2PreSett;
		m_groupSettleRatio = (g_GroupInsMap[strSubIns].Ins1Ratio*Ins1PreSett) /
			(g_GroupInsMap[strSubIns].Ins2Ratio*Ins2PreSett)*RATIOCOE;
	}
	else
	{
		if (isalpha(m_strSubIns[0]))
			m_bIsStockIndex = false;
		else
			m_bIsStockIndex = true;
	}
	//	m_pTick = &g_TickHash[g_arInsID.GetAt(m_nIndex)].back();
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

	if (m_strSubIns != "")
	{
		if (!m_bGroup)
		{
			if (!m_bIsStockIndex)
			{
				::EnterCriticalSection(&g_csTick);
				if(!g_TickHash[m_strSubIns].empty())
					m_Tick = g_TickHash[m_strSubIns].back();
				::LeaveCriticalSection(&g_csTick);
				DrawModeOne(pRT);
			}
			else
			{
				::EnterCriticalSection(&g_csTick);
				if (!g_StockIndexTickHash[m_strSubIns].empty())
					m_IndexTick = g_StockIndexTickHash[m_strSubIns].back();
				::LeaveCriticalSection(&g_csTick);
				DrawIndexModeOne(pRT);
			}
		}
		else
		{
			//			m_pTick = &g_TickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins1Num)].back();
			//			m_pTick2 = &g_TickHash[g_arInsID.GetAt(g_GroupVec[m_nIndex].Ins2Num)].back();
			::EnterCriticalSection(&g_csGroupTick);
			if (!g_GroupTickHash[m_strSubIns].empty())
				m_Group = g_GroupTickHash[m_strSubIns].back();
			::LeaveCriticalSection(&g_csGroupTick);
			DrawModeGroup(pRT);
		}
	}

}


void CPriceList::DrawModeOne(IRenderTarget * pRT)
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
	_swprintf(szTmp, L"%d", m_Tick.AskVolume1);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	_swprintf(szTmp, L"%d", m_Tick.BidVolume1);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2, m_rect.left + nWidth * 4 - 10, m_rect.top + (RC_PRH + 5) * 3), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	//买一卖一价
	pRT->SetTextColor(GetTextColor(m_Tick.AskPrice1));
	if (m_Tick.AskPrice1 > 10000000)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_Tick.AskPrice1);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + RC_PRH + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH + 5) * 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	pRT->SetTextColor(GetTextColor(m_Tick.BidPrice1));
	if (m_Tick.BidPrice1 > 10000000)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_Tick.BidPrice1);
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

	pRT->SetTextColor(GetTextColor(m_Tick.LastPrice));
	if (m_Tick.LastPrice > 10000000 || m_Tick.LastPrice < 0)
	{
		_swprintf(szTmp, L"—");	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	else
	{
		_swprintf(szTmp, L"%g", m_Tick.LastPrice);	//最新
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%g", m_Tick.LastPrice - m_Tick.PreSettlementPrice);	//涨跌
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		_swprintf(szTmp, L"%.2f%%", (m_Tick.LastPrice - m_Tick.PreSettlementPrice) / (m_Tick.PreSettlementPrice) * 100);	//涨幅
		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));
	_swprintf(szTmp, L"%g", g_UpperLimitHash[m_strSubIns]);	//涨停r
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%g", m_Tick.PreClosePrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_Tick.OpenPrice));
	if (m_Tick.OpenPrice > 10000000 || m_Tick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_Tick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_Tick.HighestPrice));
	if (m_Tick.HighestPrice > 10000000 || m_Tick.HighestPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_Tick.HighestPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_Tick.LowestPrice));
	if (m_Tick.LowestPrice > 10000000 || m_Tick.LowestPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_Tick.LowestPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(0, 255, 0, 255));
	_swprintf(szTmp, L"%g", g_LowerLimitHash[m_strSubIns]);	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"%g", m_Tick.PreSettlementPrice);	//昨结
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
		_swprintf(szTmp, L"%g", m_IndexTick.LastPrice);	//最新
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
	_swprintf(szTmp, L"%g", m_IndexTick.PreClosePrice);	//昨收
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);

	//开盘
	pRT->SetTextColor(GetTextColor(m_IndexTick.OpenPrice));
	if (m_IndexTick.OpenPrice > 10000000 || m_IndexTick.OpenPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_IndexTick.OpenPrice);	//开盘
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

	//最高
	pRT->SetTextColor(GetTextColor(m_IndexTick.HighestPrice));
	if (m_IndexTick.HighestPrice > 10000000 || m_IndexTick.HighestPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_IndexTick.HighestPrice);
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 4, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

	//最低
	pRT->SetTextColor(GetTextColor(m_IndexTick.LowestPrice));
	if (m_IndexTick.LowestPrice > 10000000 || m_IndexTick.LowestPrice < 0)
		_swprintf(szTmp, L"—");
	else
		_swprintf(szTmp, L"%g", m_IndexTick.LowestPrice);
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

//void CPriceList::DrawModeGroup(IRenderTarget * pRT)
//{
//	CPoint point[5];
//	wchar_t szTmp[100] = { 0 };
//	int nHeightTotal = m_rect.Height();
//	int nWidth = (m_rect.Width() - 10) / 4;
//	if (nWidth < 10)
//		return;
//	CAutoRefPtr<IPen> oldPen;
//	pRT->SelectObject(m_penRed);
//	CAutoRefPtr<IFont> OldFont;
//	pRT->SelectObject(m_pFont20, (IRenderObj**)&OldFont);
//
//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
//	SStringW strTmp;
//	strTmp.Format(L"%s", g_GroupVec[m_nIndex].GroupInsID);
//
//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left, m_rect.top, m_rect.right, m_rect.top + RC_PRH - 10), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	//pRT->DrawTextW(m_Tick.sInstrumentID, wcslen(m_Tick.sInstrumentID), CRect(m_rect.left, m_rect.top + RC_PRH, m_rect.right, m_rect.top + RC_PRH * 2), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	strTmp.Format(L"%s", g_GroupVec[m_nIndex].GroupIns);
//	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left, m_rect.top + RC_PRH - 10, m_rect.right, m_rect.top + RC_PRH + 15), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//
//	pRT->SelectObject(OldFont);
//
//	//横线
//	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH + 20);
//	point[1].SetPoint(m_rect.right, m_rect.top + RC_PRH + 20);
//	pRT->DrawLines(point, 2);
//
//
//
//	//填入数值
//	pRT->SetTextColor(RGBA(255, 255, 0, 255));
//
//	pRT->DrawTextW(L"比值", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 1 + 20 + 5, m_rect.left + 45, m_rect.top + (RC_PRH) * 2 + 20 + 5), DT_LEFT);
//	pRT->DrawTextW(L"价差", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 1 + 20 + 5, m_rect.left + nWidth * 2 + 55, m_rect.top + (RC_PRH) * 2 + 20 + 5), DT_LEFT);
//
//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
//
//	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.left + 45, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_LEFT);
//	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.left + 45, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_LEFT);
//
//	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.left + nWidth * 2 + 55, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_LEFT);
//	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.left + nWidth * 2 + 55, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_LEFT);
//
//	//比值
//	if (m_Group.dLdl > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_RIGHT);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dLdl));
//		_swprintf(szTmp, L"%.2f", m_Group.dLdl);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_RIGHT);
//		_swprintf(szTmp, L"%.2f", m_Group.dLdl - m_groupSettleRatio);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_RIGHT);
//
//	}
//
//	//价差
//	if (m_Group.dLml > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.right - 5, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_RIGHT);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.right - 5, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_RIGHT);
//
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dLml));
//		_swprintf(szTmp, L"%g", m_Group.dLml);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 2 + 20 + 5, m_rect.right - 5, m_rect.top + (RC_PRH) * 3 + 20 + 5), DT_RIGHT);
//		_swprintf(szTmp, L"%g", m_Group.dLml - m_groupSettle);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 3 + 20 + 5, m_rect.right - 5, m_rect.top + (RC_PRH) * 4 + 20 + 5), DT_RIGHT);
//	}
//
//
//
//	//横线
//	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH * 4 + 18+10);
//	point[1].SetPoint(m_rect.right, m_rect.top + RC_PRH * 4 + 18+10);
//	pRT->DrawLines(point, 2);
//
//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
//	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"昨结", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 7 + 20 + 15, m_rect.left + 30, m_rect.top + (RC_PRH) * 8 + 20 + 15), DT_LEFT);
//
//	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_LEFT);
//	pRT->DrawTextW(L"昨结", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 7 + 20 + 15, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 8 + 20 + 15), DT_LEFT);
//
//	//比值最高
//	if (m_Group.dHighestRatio > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dHighestRatio));
//		_swprintf(szTmp, L"%g", m_Group.dHighestRatio);	//最新
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_RIGHT);
//	}
//
//	//比值最低
//	if (m_Group.dLowestRatio > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dLowestRatio));
//		_swprintf(szTmp, L"%g", m_Group.dLowestRatio);	//最新
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_RIGHT);
//	}
//
//	//开盘
//	if (m_Group.dOdo > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dOdo));
//		_swprintf(szTmp, L"%g", m_Group.dOdo);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_RIGHT);
//	}
//
//	//结算
//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
//	_swprintf(szTmp, L"%g", m_groupSettleRatio);	//昨结
//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20 + 15, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20 + 15), DT_RIGHT);
//
//
//	if (m_Group.dHighest > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dHighest));
//		_swprintf(szTmp, L"%g", m_Group.dHighest);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20 + 15), DT_RIGHT);
//	}
//
//	if (m_Group.dLowest > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_RIGHT);
//
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dLowest));
//		_swprintf(szTmp, L"%g", m_Group.dLowest);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20 + 15), DT_RIGHT);
//
//	}
//
//
//	//开盘
//	if (m_Group.dOmo > 10000000)
//	{
//		pRT->SetTextColor(RGBA(255, 255, 255, 255));
//		_swprintf(szTmp, L"—");
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_RIGHT);
//	}
//	else
//	{
//		pRT->SetTextColor(GetGroupColor(m_Group.dOmo));
//		_swprintf(szTmp, L"%g", m_Group.dOmo);
//		pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 7 + 20 + 15), DT_RIGHT);
//	}
//
//	//结算
//	pRT->SetTextColor(RGBA(255, 255, 255, 255));
//	_swprintf(szTmp, L"%g", m_groupSettle);	//昨结
//	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20 + 15, m_rect.right - 5, m_rect.top + (RC_PRH) * 8 + 20 + 15), DT_RIGHT);
//
//
//
//	//横线
//	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH) * 8 + 17 + 15);
//	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH) * 8 + 17 + 15);
//	pRT->DrawLines(point, 2);
//
//	//竖线
//	for (int i = m_rect.top + RC_PRH * 1 + 20; i < m_rect.top + (RC_PRH) * 8 + 17 + 15; i += 3)
//		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
//}
void SOUI::CPriceList::DrawModeGroup(IRenderTarget * pRT)
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
	SStringW strTmp=g_GroupInsMap[m_strSubIns].ComboInsID;

	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left, m_rect.top-20, m_rect.right, m_rect.top + RC_PRH-20), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	strTmp = g_GroupInsMap[m_strSubIns].ComboIns;
	pRT->DrawTextW(strTmp, strTmp.GetLength(), CRect(m_rect.left, m_rect.top+RC_PRH - 20, m_rect.right, m_rect.top + RC_PRH - 20+ RC_PRH), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//pRT->DrawTextW(m_Tick.sInstrumentID, wcslen(m_Tick.sInstrumentID), CRect(m_rect.left, m_rect.top + RC_PRH, m_rect.right, m_rect.top + RC_PRH * 2), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pRT->SelectObject(OldFont);

	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH + 5);
	point[1].SetPoint(m_rect.right+5, m_rect.top + RC_PRH + 5);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));


	//填入数值

	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 1 + 20, m_rect.left + 45, m_rect.top + (RC_PRH) * 2 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 2 + 20, m_rect.left + 45, m_rect.top + (RC_PRH) * 3 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨跌幅", 3, CRect(m_rect.left, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + 45, m_rect.top + (RC_PRH) * 4 + 20), DT_LEFT);

	//卖一


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + RC_PRH * 4 + 18);
	point[1].SetPoint(m_rect.right+5, m_rect.top + RC_PRH * 4 + 18);
	pRT->DrawLines(point, 2);

	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	pRT->DrawTextW(L"最新", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨跌", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"开盘", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"昨结", 2, CRect(m_rect.left, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + 30, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	pRT->DrawTextW(L"最高", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20), DT_LEFT);
	pRT->DrawTextW(L"最低", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20), DT_LEFT);
	pRT->DrawTextW(L"涨停", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20), DT_LEFT);
	pRT->DrawTextW(L"跌停", 2, CRect(m_rect.left + nWidth * 2 + 10, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 8 + 20), DT_LEFT);

	//最新
	if (*m_pGroupDataType == 0)
	{

		if (m_Group.dLml > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 1 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 2 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 2 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 3 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);

		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dLml));
			_swprintf(szTmp, L"%g", m_Group.dLml);	//最新
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 1 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 2 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

			double fDelta = m_Group.dLml - m_groupSettle;
			if (fDelta > -0.000001&&fDelta < 0.000001)
				fDelta = 0;
			_swprintf(szTmp, L"%g", fDelta);	//涨跌
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 2 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 3 + 20), DT_RIGHT);
			_swprintf(szTmp, L"%.2f%%", m_groupSettle>0?fDelta/ m_groupSettle*100: fDelta /(0- m_groupSettle) * 100);	//涨跌幅
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

		}
	}
	else if (*m_pGroupDataType == 1)
	{
		if (m_Group.dLdl > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 1 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 2 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dLdl));
			_swprintf(szTmp, L"%.2f", m_Group.dLdl);	//最新
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 1 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 2 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 4 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);

			double fDelta = m_Group.dLdl - m_groupSettleRatio;
			if (fDelta > -0.000001&&fDelta < 0.000001)
				fDelta = 0;
			_swprintf(szTmp, L"%.2f", fDelta);	//涨跌
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 5 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 2 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 3 + 20), DT_RIGHT);
			_swprintf(szTmp, L"%.2f%%", fDelta / m_groupSettleRatio *100);	//涨跌幅
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 3 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 4 + 20), DT_RIGHT);

		}

	}
	//开盘
	if (*m_pGroupDataType == 0)
	{
		if (m_Group.dOmo > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dOmo));
			_swprintf(szTmp, L"%g", m_Group.dOmo);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
		}
	}
	else if (*m_pGroupDataType == 1)
	{
		if (m_Group.dOdo > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dOdo));
			_swprintf(szTmp, L"%.2f", m_Group.dOdo);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 6 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
		}

	}
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	if (*m_pGroupDataType == 0)
		_swprintf(szTmp, L"%g", m_groupSettle);	//昨结
	else if (*m_pGroupDataType == 1)
		_swprintf(szTmp, L"%g", m_groupSettleRatio);	//昨结
	else if (*m_pGroupDataType == 2)
		_swprintf(szTmp, L"%d", 0);	//昨结

	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + 30, m_rect.top + (RC_PRH) * 7 + 20, m_rect.left + nWidth * 2, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);


	if (*m_pGroupDataType == 0)
	{
		if (m_Group.dHighest > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dHighest));
			_swprintf(szTmp, L"%g", m_Group.dHighest);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		}

		if (m_Group.dLowest > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);

		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dLowest));
			_swprintf(szTmp, L"%g", m_Group.dLowest);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);

		}
	}
	else if (*m_pGroupDataType == 1)
	{
		if (m_Group.dHighestRatio > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dHighestRatio));
			_swprintf(szTmp, L"%g", m_Group.dHighestRatio);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 4 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 5 + 20), DT_RIGHT);
		}

		if (m_Group.dLowestRatio > 10000000)
		{
			pRT->SetTextColor(RGBA(255, 255, 255, 255));
			_swprintf(szTmp, L"—");
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);

		}
		else
		{
			pRT->SetTextColor(GetGroupColor(m_Group.dLowestRatio));
			_swprintf(szTmp, L"%g", m_Group.dLowestRatio);
			pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 5 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 6 + 20), DT_RIGHT);

		}
	}


	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"—");	//涨停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 6 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 7 + 20), DT_RIGHT);
	pRT->SetTextColor(RGBA(255, 255, 255, 255));
	_swprintf(szTmp, L"—");	//跌停
	pRT->DrawTextW(szTmp, wcslen(szTmp), CRect(m_rect.left + nWidth * 2 + 40, m_rect.top + (RC_PRH) * 7 + 20, m_rect.right - 5, m_rect.top + (RC_PRH) * 8 + 20), DT_RIGHT);


	//横线
	point[0].SetPoint(m_rect.left, m_rect.top + (RC_PRH) * 8 + 17);
	point[1].SetPoint(m_rect.right, m_rect.top + (RC_PRH) * 8 + 17);
	pRT->DrawLines(point, 2);

	//竖线
	for (int i = m_rect.top + RC_PRH * 1 + 5; i < m_rect.top + (RC_PRH) * 8 + 17; i += 3)
		pRT->SetPixel(m_rect.left + nWidth * 2 + 5, i, RGB(0, 0, 255));
}


COLORREF CPriceList::GetTextColor(double price)
{
	if (!m_bIsStockIndex)
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_Tick.PreSettlementPrice)
			return RGBA(255, 0, 0, 255);
		else if (price < m_Tick.PreSettlementPrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
	else
	{
		if (price > 10000000 || price < 0)
			return RGBA(255, 255, 255, 255);
		if (price > m_IndexTick.PreClosePrice)
			return RGBA(255, 0, 0, 255);
		else if (price < m_IndexTick.PreClosePrice)
			return RGBA(0, 255, 0, 255);
		return RGBA(255, 255, 255, 255);

	}
}

COLORREF CPriceList::GetGroupColor(double price)
{
	double fDelta;
	if (*m_pGroupDataType == 0)
		fDelta = price - m_groupSettle;
	else if (*m_pGroupDataType == 1)
		fDelta = price - m_groupSettleRatio;
	else
		fDelta = price;
	if (fDelta > -0.000001&&fDelta < 0.000001)
		return RGBA(255, 255, 255, 255);
	else if (fDelta > 0)
		return RGBA(255, 0, 0, 255);
	else
		return RGBA(0, 255, 0, 255);
}


