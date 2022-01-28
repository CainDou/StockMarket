#include "stdafx.h"
#include "SKlinePic.h"
#include <map>
#include <vector>
#include<gdiplus.h>
#include<unordered_map>
#include"DealList.h"
#include"PriceList.h"
#include "SSubPic.h"

using std::vector;
#define MOVE_ONESTEP		10		//每次平移的数据量
#define K_WIDTH_TOTAL		16		//k线占用总宽度(在x轴上)
//一些基本的界面框架信息
#define RC_LEFT				48
#define RC_RIGHT			40
#define RC_TOP				25
#define RC_BOTTOM			40

#define RC_MAX				5
#define RC_MIN				5
#define RC_RIGHT_BACK		50		//框内最后一根k线和框得距离

#define DATA_ERROR			-1234567
#define SDECIMAL			L"%.02f"

SKlinePic::SKlinePic()
{

	::InitializeCriticalSection(&m_cs);
	m_style.m_bBkgndBlend = 0;
	m_bFocusable = 1; //可以获取焦点
					  //	m_nKWidth = K_WIDTH_TOTAL;
	m_nKWidth = K_WIDTH_TOTAL;
	m_nMouseX = m_nMouseY = -1;
	m_bPaintInit = FALSE;
	m_pAll = nullptr;
	m_bShowMouseLine = false;
	m_nMove = 0;
	m_nTickPre = 0;
	m_nBandCount = 0;
	m_nMacdCount = 0;
	m_bDataInited = false;
	m_pBandData = nullptr;
	m_pMacdData = nullptr;
	m_nTradingDay = false;
	m_nPeriod = 1;
	m_pDealList = new CDealList;
	m_pPriceList = new CPriceList;
	m_strSubIns = "";
	m_pGroupDataType = nullptr;
	m_bReSetFirstLine = false;
	m_nFirst = 0;
	m_bKeyDown = false;
	m_preMovePoint.SetPoint(-1, -1);
	m_bClearTip = false;
	m_nPreX = -1;
	m_nPreY = -1;
}

SKlinePic::~SKlinePic()
{
	if (m_pAll)
	delete m_pAll;
	if (m_pPriceList)
		delete m_pPriceList;
	if (m_pDealList)
		delete m_pDealList;
	if (m_pTip)
		delete m_pTip;
}

void SKlinePic::InitShowPara()
{
	m_pTip = new SKlineTip(m_hParWnd);
	m_pTip->Create();

	m_bShowBandTarget = m_InitPara.bShowBandTarget;
	m_bShowVolume = m_InitPara.bShowKlineVolume;
	m_bShowMA = m_InitPara.bShowMA;
	m_nKWidth = m_InitPara.nWidth;
	m_bShowDeal = m_InitPara.bShowMarket;
	m_bShowMacd = m_InitPara.bShowKlineMACD;
	m_nMACDPara[0] = m_InitPara.nMACDPara[0];
	m_nMACDPara[1] = m_InitPara.nMACDPara[1];
	m_nMACDPara[2] = m_InitPara.nMACDPara[2];
	m_BandPara = m_InitPara.BandPara;
	m_nMAPara[0] = m_InitPara.nMAPara[0];
	m_nMAPara[1] = m_InitPara.nMAPara[1];
	m_nMAPara[2] = m_InitPara.nMAPara[2];
	m_nMAPara[3] = m_InitPara.nMAPara[3];

	m_bNoJiange = m_InitPara.bNoJiange;
}

void SKlinePic::SetShowData(SStringA subIns, vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_strSubIns = subIns;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
}

void SKlinePic::SetShowData(SStringA subIns, vector<CommonStockMarket>* pStkMarketVec)
{
	m_strSubIns = subIns;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
}

void SKlinePic::BandDataUpdate()
{
	if (m_pBandData == nullptr)
	{
		m_pBandData = new Band_t;
		ZeroMemory(m_pBandData, sizeof(Band_t));
	}
	int nSize = m_pAll->nTotal;
	if (nSize == 0)
		return;
	if (nSize >= m_nBandCount + 1)
	{
		for (int i = m_nBandCount; i < nSize; i++)
			m_nBandCount = ProcBandTargetData(i, m_pBandData);
	}
}

void SKlinePic::MACDDataUpdate()
{
	if (m_pMacdData == nullptr)
	{
		m_pMacdData = new MACDData_t;
		ZeroMemory(m_pMacdData, sizeof(MACDData_t));
	}

	int nSize = m_pAll->nTotal;
	if (nSize == 0)
		return;
	if (nSize >= m_nMacdCount + 1)
	{
		for (int i = m_nMacdCount; i < nSize; i++)
			m_nMacdCount = ProcMACDData(i, m_pMacdData);
	}
}

void SKlinePic::DrawMainUpperMarket(IRenderTarget * pRT, KlineType & data)
{
	SStringW strMarket;

	strMarket.Format(L"%s 日期:%d-%02d-%02d 时间:%02d:%02d:00 开:%.02f 高:%s 低:%s 收:%s 量:%%d",
		StrA2StrW(m_strSubIns), data.date / 10000, data.date % 10000 / 100, data.date % 100,
		data.time / 10000, data.time % 10000 / 100,
		data.open, data.high, data.low, data.close, data.vol);

	DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top), strMarket);
}

void SOUI::SKlinePic::DrawMainUpperMA(IRenderTarget * pRT, int nPos)
{
	SStringW strMarket;
	auto &arrMA = m_pAll->fMa;
	if (nPos >= m_nMAPara[0] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[0], arrMA[0][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[0]);
	DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

	if (nPos >= m_nMAPara[1] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[1], arrMA[1][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[1]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + 90, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	if (nPos >= m_nMAPara[2] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[2], arrMA[2][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[2]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + 180, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 0, 255, 255));

	if (nPos >= m_nMAPara[3] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[3], arrMA[3][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[3]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + 270, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
}

void SOUI::SKlinePic::DrawMacdUpperMarket(IRenderTarget * pRT, int nPos)
{
	SStringW strMarekt;
	strMarekt.Format(L"MACD(%d,%d,%d) DIF:%.2f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[nPos]);
	DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
		strMarekt, RGBA(255, 255, 255, 255));
	strMarekt.Format(L"DEA:%.2f", m_pMacdData->DEA[nPos]);
	DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
		strMarekt, RGBA(255, 255, 0, 255));
	strMarekt.Format(L"MACD:%.2f", m_pMacdData->MACD[nPos]);
	DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
		strMarekt, RGBA(255, 0, 255, 255));

}

void SOUI::SKlinePic::DrawMainUpperBand(IRenderTarget * pRT, int nPos)
{
	SStringW strMarket;
	if (m_pBandData->DataValid[nPos])
	{
		strMarket.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
		DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"多平位置:%.02f", m_pBandData->SellLong[nPos]);

		DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(100, 100, 100, 255));

		strMarket.Format(L"空平位置:%.02f", m_pBandData->BuyShort[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 110 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"上轨:%.02f", m_pBandData->UpperTrack1[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 220 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));

		strMarket.Format(L"下轨:%.02f", m_pBandData->LowerTrack1[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 310 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

		strMarket.Format(L"上轨K2:%.02f", m_pBandData->UpperTrack2[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 400 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));

		strMarket.Format(L"下轨K2:%.02f", m_pBandData->LowerTrack2[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 500 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

		strMarket.Format(L"状态:%d", m_pBandData->Status[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 600 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"仓位:%.2f", m_pBandData->Position[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + 640 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	}
	else
	{
		strMarket.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
		DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"多平位置:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(100, 100, 100, 255));

		strMarket.Format(L"空平位置:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 60 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"上轨:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 120 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));

		strMarket.Format(L"下轨:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 180 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

		strMarket.Format(L"上轨K2:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 240 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));

		strMarket.Format(L"下轨K2:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 300 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

		strMarket.Format(L"状态:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 360 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

		strMarket.Format(L"仓位:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + 420 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	}

}


int SKlinePic::GetShowKlineNum(int nKwidth)	//获取需要显示的k线数量
{
	int nSrcwidth = m_rcUpper.right - m_rcUpper.left - RC_RIGHT_BACK;	//判断是否超出范围
	nSrcwidth = nSrcwidth / nKwidth;
	if (nSrcwidth < 50)
		nSrcwidth = 50;
	if (nSrcwidth > MAX_DATA_COUNT)
		nSrcwidth = MAX_DATA_COUNT;
	return (nSrcwidth > 0 ? nSrcwidth : 1);
}


int SKlinePic::ProcBandTargetData(int nPos, Band_t *pBandData)
{

	auto& Data = m_pAll->data;
	if (nPos >= 1)
	{
		pBandData->nLastHighPoint1[nPos] = pBandData->nLastHighPoint1[nPos - 1];
		pBandData->nLastHighPoint2[nPos] = pBandData->nLastHighPoint2[nPos - 1];
		pBandData->nLastLowPoint1[nPos] = pBandData->nLastLowPoint1[nPos - 1];
		pBandData->nLastLowPoint2[nPos] = pBandData->nLastLowPoint2[nPos - 1];
	}
	else
	{
		pBandData->nLastLowPoint1[nPos] = 0;
		pBandData->nLastLowPoint2[nPos] = 0;
		pBandData->nLastLowPoint1[nPos] = 0;
		pBandData->nLastLowPoint2[nPos] = 0;
	}

	if (Data[nPos].high<GetHighPrice(nPos, m_BandPara.N1, 1) && GetHighPrice(nPos, m_BandPara.N1, 1)>GetHighPrice(nPos, m_BandPara.N1, 2))
		pBandData->nLastHighPoint1[nPos] = nPos;
	if (Data[nPos].high<GetHighPrice(nPos, m_BandPara.N1, 1) && GetHighPrice(nPos, m_BandPara.N1, 1) == GetHighPrice(nPos, m_BandPara.N1, 2)
		&& GetHighPrice(nPos, m_BandPara.N1, 2)>GetHighPrice(nPos, m_BandPara.N1, 3))
		pBandData->nLastHighPoint1[nPos] = nPos;
	int KK1 = nPos - pBandData->nLastHighPoint1[nPos];
	pBandData->dHigh1[nPos] = GetHighPrice(nPos, m_BandPara.N1, KK1 + 1);

	if (Data[nPos].low > GetLowPrice(nPos, m_BandPara.N1, 1) && GetLowPrice(nPos, m_BandPara.N1, 1) < GetLowPrice(nPos, m_BandPara.N1, 2))
		pBandData->nLastLowPoint1[nPos] = nPos;
	if (Data[nPos].low > GetLowPrice(nPos, m_BandPara.N1, 1) && GetLowPrice(nPos, m_BandPara.N1, 1) == GetLowPrice(nPos, m_BandPara.N1, 2)
		&& GetLowPrice(nPos, m_BandPara.N1, 2) < GetLowPrice(nPos, m_BandPara.N1, 3))
		pBandData->nLastLowPoint1[nPos] = nPos;
	int DD1 = nPos - pBandData->nLastLowPoint1[nPos];
	pBandData->dLow1[nPos] = GetLowPrice(nPos, m_BandPara.N1, DD1 + 1);

	pBandData->dPreHigh1[nPos] = pBandData->dHigh1[nPos - KK1 - 2];
	pBandData->dPreLow1[nPos] = pBandData->dLow1[nPos - DD1 - 2];
	double dPrePreHigh1 = pBandData->dPreHigh1[nPos - KK1 - 3];;
	double dPrePreLow1 = pBandData->dPreLow1[nPos - DD1 - 3];

	double BandHigh1 = (pBandData->dHigh1[nPos] > pBandData->dPreHigh1[nPos] && pBandData->dLow1[nPos] > pBandData->dPreLow1[nPos]) ?
		pBandData->dHigh1[nPos] : min(pBandData->dHigh1[nPos], pBandData->dPreHigh1[nPos]);
	double BandLow1 = (pBandData->dHigh1[nPos] < pBandData->dPreHigh1[nPos] && pBandData->dLow1[nPos] < pBandData->dPreLow1[nPos]) ?
		pBandData->dLow1[nPos] : max(pBandData->dLow1[nPos], pBandData->dPreLow1[nPos]);
	double MidHigh1 = pBandData->dHigh1[nPos] > pBandData->dPreLow1[nPos] ?
		min(pBandData->dHigh1[nPos], pBandData->dPreHigh1[nPos]) : min(dPrePreHigh1, pBandData->dPreHigh1[nPos]);
	double MidLow1 = pBandData->dLow1[nPos] < pBandData->dPreHigh1[nPos] ?
		max(pBandData->dLow1[nPos], pBandData->dPreLow1[nPos]) : max(dPrePreLow1, pBandData->dPreLow1[nPos]);
	double CoreHigh1 = (pBandData->dHigh1[nPos] > min(dPrePreHigh1, pBandData->dPreHigh1[nPos]) && pBandData->dHigh1[nPos] < max(dPrePreHigh1, pBandData->dPreHigh1[nPos]))
		? pBandData->dHigh1[nPos] : max(dPrePreHigh1, pBandData->dPreHigh1[nPos]);
	double CoreLow1 = (pBandData->dLow1[nPos]<max(pBandData->dPreLow1[nPos], dPrePreLow1) && pBandData->dLow1[nPos]>min(pBandData->dPreLow1[nPos], dPrePreLow1)) ?
		pBandData->dLow1[nPos] : min(pBandData->dPreLow1[nPos], dPrePreLow1);

	if (Data[nPos].high<GetHighPrice(nPos, m_BandPara.N2, 1) && GetHighPrice(nPos, m_BandPara.N2, 1)>GetHighPrice(nPos, m_BandPara.N2, 2))
		pBandData->nLastHighPoint2[nPos] = nPos;
	if (Data[nPos].high<GetHighPrice(nPos, m_BandPara.N2, 1) && GetHighPrice(nPos, m_BandPara.N2, 1) == GetHighPrice(nPos, m_BandPara.N2, 2)
		&& GetHighPrice(nPos, m_BandPara.N2, 2)>GetHighPrice(nPos, m_BandPara.N2, 3))
		pBandData->nLastHighPoint2[nPos] = nPos;
	int KK2 = nPos - pBandData->nLastHighPoint2[nPos];
	pBandData->dHigh2[nPos] = GetHighPrice(nPos, m_BandPara.N2, KK2 + 1);

	if (Data[nPos].low > GetLowPrice(nPos, m_BandPara.N2, 1) && GetLowPrice(nPos, m_BandPara.N2, 1) < GetLowPrice(nPos, m_BandPara.N2, 2))
		pBandData->nLastLowPoint2[nPos] = nPos;
	if (Data[nPos].low > GetLowPrice(nPos, m_BandPara.N2, 1) && GetLowPrice(nPos, m_BandPara.N2, 1) == GetLowPrice(nPos, m_BandPara.N2, 2)
		&& GetLowPrice(nPos, m_BandPara.N2, 2) < GetLowPrice(nPos, m_BandPara.N2, 3))
		pBandData->nLastLowPoint2[nPos] = nPos;
	int DD2 = nPos - pBandData->nLastLowPoint2[nPos];
	pBandData->dLow2[nPos] = GetLowPrice(nPos, m_BandPara.N2, DD2 + 1);

	pBandData->dPreHigh2[nPos] = pBandData->dHigh2[nPos - KK2 - 2];
	pBandData->dPreLow2[nPos] = pBandData->dLow2[nPos - DD2 - 2];
	double dPrePreHigh2 = pBandData->dPreHigh2[nPos - KK2 - 3];;
	double dPrePreLow2 = pBandData->dPreLow2[nPos - DD2 - 3];

	double BandHigh2 = (pBandData->dHigh2[nPos] > pBandData->dPreHigh2[nPos] && pBandData->dLow2[nPos] > pBandData->dPreLow2[nPos]) ?
		pBandData->dHigh2[nPos] : min(pBandData->dHigh2[nPos], pBandData->dPreHigh2[nPos]);
	double BandLow2 = (pBandData->dHigh2[nPos] < pBandData->dPreHigh2[nPos] && pBandData->dLow2[nPos] < pBandData->dPreLow2[nPos]) ?
		pBandData->dLow2[nPos] : max(pBandData->dLow2[nPos], pBandData->dPreLow2[nPos]);
	double MidHigh2 = pBandData->dHigh2[nPos] > pBandData->dPreLow2[nPos] ?
		min(pBandData->dHigh2[nPos], pBandData->dPreHigh2[nPos]) : min(dPrePreHigh2, pBandData->dPreHigh2[nPos]);
	double MidLow2 = pBandData->dLow2[nPos] < pBandData->dPreHigh2[nPos] ?
		max(pBandData->dLow2[nPos], pBandData->dPreLow2[nPos]) : max(dPrePreLow2, pBandData->dPreLow2[nPos]);
	double CoreHigh2 = (pBandData->dHigh2[nPos] > min(dPrePreHigh2, pBandData->dPreHigh2[nPos]) && pBandData->dHigh2[nPos] < max(dPrePreHigh2, pBandData->dPreHigh2[nPos]))
		? pBandData->dHigh2[nPos] : max(dPrePreHigh2, pBandData->dPreHigh2[nPos]);
	double CoreLow2 = (pBandData->dLow2[nPos]<max(pBandData->dPreLow2[nPos], dPrePreLow2) && pBandData->dLow2[nPos]>min(pBandData->dPreLow2[nPos], dPrePreLow2)) ?
		pBandData->dLow2[nPos] : min(pBandData->dPreLow2[nPos], dPrePreLow2);

	//定义上下轨和中间位置
	double nWidth = (max(MidHigh2, CoreHigh2) - min(MidLow2, CoreLow2)) / min(MidLow2, CoreLow2) * 10000;
	pBandData->SellLong[nPos] = nWidth > m_BandPara.K ? max(CoreLow2, BandLow2) : max(CoreLow1, BandLow1);
	pBandData->BuyShort[nPos] = nWidth > m_BandPara.K ? min(CoreHigh2, BandHigh2) : min(CoreHigh1, BandHigh1);
	pBandData->UpperTrack1[nPos] = nWidth > m_BandPara.K ? max(MidHigh2, CoreHigh2) : max(pBandData->dHigh2[nPos], CoreHigh2);
	pBandData->LowerTrack1[nPos] = nWidth > m_BandPara.K ? min(MidLow2, CoreLow2) : min(pBandData->dLow2[nPos], CoreLow2);
	if (pBandData->UpperTrack1[nPos] >= 0)
		pBandData->UpperTrack2[nPos] = pBandData->UpperTrack1[nPos] * (1000 + m_BandPara.P) / 1000;
	else
		pBandData->UpperTrack2[nPos] = pBandData->UpperTrack1[nPos] * (1000 - m_BandPara.P) / 1000;

	if (pBandData->LowerTrack1[nPos] >= 0)
		pBandData->LowerTrack2[nPos] = pBandData->LowerTrack1[nPos] * (1000 - m_BandPara.P) / 1000;
	else
		pBandData->LowerTrack2[nPos] = pBandData->LowerTrack1[nPos] * (1000 + m_BandPara.P) / 1000;


	if ((int)pBandData->BuyShort[nPos] == DATA_ERROR || (int)pBandData->SellLong[nPos] == DATA_ERROR
		|| (int)pBandData->UpperTrack1[nPos] == DATA_ERROR || (int)pBandData->UpperTrack2[nPos] == DATA_ERROR
		|| (int)pBandData->LowerTrack1[nPos] == DATA_ERROR || (int)pBandData->LowerTrack2[nPos] == DATA_ERROR
		|| (int)pBandData->BuyShort[nPos] == 0 || (int)pBandData->SellLong[nPos] == 0
		|| (int)pBandData->UpperTrack1[nPos] == 0 || (int)pBandData->UpperTrack2[nPos] == 0
		|| (int)pBandData->LowerTrack1[nPos] == 0 || (int)pBandData->LowerTrack2[nPos] == 0)
		pBandData->DataValid[nPos] = false;
	else
		pBandData->DataValid[nPos] = true;

	if (!pBandData->DataValid[nPos])
	{
		for (int i = 0; i < nPos; i++)
			pBandData->DataValid[i] = false;
	}

	//找出穿越点1,2,3,4 1只能变化成2, 4只能变化成3
	int Count = 0;
	for (int i = 0; i < m_BandPara.M1; i++)
	{
		int n = nPos - i;
		if (n < 0 || !m_pBandData->DataValid[n])
			break;
		if (Data[n].high > pBandData->UpperTrack1[n])
			Count++;
	}


	bool bCross = false;
	if (nPos >= 1 && m_pBandData->DataValid[nPos] && m_pBandData->DataValid[nPos - 1])
	{
		if (Data[nPos].high > pBandData->UpperTrack1[nPos] && Data[nPos - 1].high < pBandData->UpperTrack1[nPos - 1])
			pBandData->nLastCrossHigh[nPos] = nPos;
		else
			pBandData->nLastCrossHigh[nPos] = pBandData->nLastCrossHigh[nPos - 1];
		if (Data[nPos].high > pBandData->UpperTrack2[nPos] && Data[nPos - 1].high < pBandData->UpperTrack2[nPos - 1])
			bCross = true;
	}
	pBandData->CrossPoint1[nPos] = (Count == m_BandPara.M2 && (nPos - pBandData->nLastCrossHigh[nPos]) <= m_BandPara.M1 || bCross) ? 1 : 0;
	pBandData->CrossPoint2[nPos] = (nPos >= 1 && pBandData->SellLong[nPos] > Data[nPos].low&&pBandData->SellLong[nPos - 1] < Data[nPos - 1].low) ? 2 : 0;
	pBandData->CrossPoint3[nPos] = (nPos >= 1 && pBandData->BuyShort[nPos] < Data[nPos].high&&pBandData->BuyShort[nPos - 1] > Data[nPos - 1].high) ? 3 : 0;

	Count = 0;
	for (int i = 0; i < m_BandPara.M1; i++)
	{
		int n = nPos - i;
		if (n < 0 || !m_pBandData->DataValid[n])
			break;
		if (Data[n].low < pBandData->LowerTrack1[n])
			Count++;
	}

	bCross = false;
	if (nPos >= 1 && m_pBandData->DataValid[nPos] && m_pBandData->DataValid[nPos - 1])
	{
		if (Data[nPos].low < pBandData->LowerTrack1[nPos] && Data[nPos - 1].low > pBandData->LowerTrack1[nPos - 1])
			pBandData->nLastCrossLow[nPos] = nPos;
		else
			pBandData->nLastCrossLow[nPos] = pBandData->nLastCrossLow[nPos - 1];
		if (Data[nPos].low < pBandData->LowerTrack2[nPos] && Data[nPos - 1].low > pBandData->LowerTrack2[nPos - 1])
			bCross = true;
	}
	pBandData->CrossPoint4[nPos] = (Count == m_BandPara.M2 && (nPos - pBandData->nLastCrossLow[nPos]) <= m_BandPara.M1 || bCross) ? 4 : 0;

	int ConStatus14 = ValueWhen(pBandData->CrossPoint1, pBandData->CrossPoint4, nPos);
	int ConStatus12 = ValueWhen(pBandData->CrossPoint1, pBandData->CrossPoint2, nPos);
	int ConStatus34 = ValueWhen(pBandData->CrossPoint3, pBandData->CrossPoint4, nPos);

	pBandData->Status[nPos] = ConStatus14 == 1 ? ConStatus12 : ConStatus34;

	//状态仓位
	double Position1 = pBandData->Status[nPos] == 1 ? 100 : 0;
	double Position2 = pBandData->Status[nPos] == 2 ? 50 : Position1;
	double Position3 = pBandData->Status[nPos] == 3 ? 50 : Position2;
	double Position4 = pBandData->Status[nPos] == 4 ? 0 : Position3;
	//过渡仓位
	double Position5 = (pBandData->Status[nPos] == 1 && (pBandData->Status[nPos - 1] == 2 || pBandData->Status[nPos - 1] == 3)) ? 62.5 : Position4;
	double Position6 = (pBandData->Status[nPos] == 4 && (pBandData->Status[nPos - 1] == 2 || pBandData->Status[nPos - 1] == 3)) ? 37.5 : Position5;
	double Position7 = (pBandData->Status[nPos] == 2 && pBandData->Status[nPos - 1] == 1) ? 87.5 : Position6;
	pBandData->Position[nPos] = (pBandData->Status[nPos] == 3 && pBandData->Status[nPos - 1] == 4) ? 12.5 : Position7;

	pBandData->W2[nPos] = Data[nPos].open - Data[nPos].close;
	pBandData->BB1[nPos] = pBandData->Position[nPos] == 100 ? 1 : (pBandData->Position[nPos] == 0 ? 2 : (pBandData->Position[nPos] == 50 ? 3 : 4));

	return nPos;
}

int SKlinePic::ProcMACDData(int nPos, MACDData_t * pMacdData)
{
	auto& Data = m_pAll->data;
	int nSize = m_pAll->nTotal;
	if (nSize == 0)
		return 0;
	if (nPos == 0)
	{
		pMacdData->EMA12[nPos] = Data[nPos].close;
		pMacdData->EMA26[nPos] = Data[nPos].close;
		pMacdData->DIF[nPos] = 0;
		pMacdData->DEA[nPos] = 0;
		pMacdData->MACD[nPos] = 0;
	}
	else
	{
		pMacdData->EMA12[nPos] = m_dataHandler.EMA(m_nMACDPara[0], pMacdData->EMA12[nPos - 1], Data[nPos].close);
		pMacdData->EMA26[nPos] = m_dataHandler.EMA(m_nMACDPara[1], pMacdData->EMA26[nPos - 1], Data[nPos].close);
		pMacdData->DIF[nPos] = pMacdData->EMA12[nPos] - pMacdData->EMA26[nPos];
		pMacdData->DEA[nPos] = m_dataHandler.EMA(m_nMACDPara[2], pMacdData->DEA[nPos - 1], pMacdData->DIF[nPos]);
		pMacdData->MACD[nPos] = 2 * (pMacdData->DIF[nPos] - pMacdData->DEA[nPos]);
	}

	return nPos;
}

BOOL SKlinePic::CreateChildren(pugi::xml_node xmlNode)
{
	if (!__super::CreateChildren(xmlNode))
		return FALSE;
	m_pSubPic = nullptr;

	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
	while (pChild)
	{
		if (pChild->IsClass(SSubPic::GetClassName()))
		{
			m_pSubPic = (SSubPic*)pChild;
			break;
		}
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}
}

void SKlinePic::OnPaint(IRenderTarget * pRT)
{
	LONGLONG llTmp = GetTickCount64();
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);



	if (!m_bPaintInit)
	{
		m_bPaintInit = TRUE;

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 12;// 字体大小
		_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
		GETRENDERFACTORY->CreateFont(&m_pFont12, lf);

		COLORREF cl = RGBA(176, 196, 222, 100);
		pRT->CreatePen(PS_SOLID, cl, 1, &m_penGrey);
		pRT->CreateSolidColorBrush(cl, &m_bBrushGrey);
		pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &m_bBrushBlack);
		pRT->CreateSolidColorBrush(RGBA(100, 100, 255, 255), &m_bBrushDarkBlue);
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 255), 1, &m_penRed);
		pRT->CreatePen(PS_SOLID, RGBA(0, 255, 255, 255), 1, &m_penGreen);
		pRT->CreatePen(PS_SOLID, RGBA(0, 255, 0, 255), 1, &m_penMAGreen);
		pRT->CreatePen(PS_DOT, RGBA(0, 255, 0, 255), 1, &m_penDotGreen);

		pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &m_penWhite);
		pRT->CreateSolidColorBrush(RGBA(0, 255, 255, 255), &m_bBrushGreen);
		pRT->CreatePen(PS_SOLID, RGBA(255, 255, 000, 255), 1, &m_penYellow);
		pRT->CreatePen(PS_DOT, RGBA(255, 255, 000, 255), 1, &m_penDotYellow);
		pRT->CreatePen(PS_DOT, RGBA(255, 0, 000, 255), 1, &m_penDotRed);

		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 255, 255), 1, &m_penPurple);
		pRT->CreatePen(PS_SOLID, RGBA(100, 100, 100, 255), 1, &m_penGray);


	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);


	SWindow::GetClientRect(&m_rcAll);
	pRT->FillSolidRect(m_rcAll, RGBA(0, 0, 0, 255));
	m_rcAll.DeflateRect(RC_LEFT + 5, RC_TOP, RC_RIGHT + 10, RC_BOTTOM);
	if (!m_bShowDeal)
	{
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right, m_rcAll.bottom);
		if (m_bShowVolume&&m_bShowMacd)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5 * 2);
			m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 5, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5);
			m_rcLower2.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 4 / 5, m_rcImage.right, m_rcImage.bottom);

		}
		else if (m_bShowVolume)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
			m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
			m_rcLower2.SetRect(0, 0, 0, 0);
		}
		else if (m_bShowMacd)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
			m_rcLower2.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
			m_rcLower.SetRect(0, 0, 0, 0);

		}
		else
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom);
			m_rcLower.SetRect(0, 0, 0, 0);
			m_rcLower2.SetRect(0, 0, 0, 0);
		}

	}
	else
	{
		m_pPriceList->m_rect.SetRect(m_rcAll.right - 190, m_rcAll.top, m_rcAll.right + 30, m_rcAll.top + 250);
		m_pDealList->m_rect.SetRect(m_rcAll.right - 190, m_rcAll.top + 255, m_rcAll.right + 30, m_rcAll.bottom + 30);
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right - 240, m_rcAll.bottom);

		if (m_bShowVolume&&m_bShowMacd)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5 * 2);
			m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 5, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5);
			m_rcLower2.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 4 / 5, m_rcImage.right, m_rcImage.bottom);

		}
		else if (m_bShowVolume)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
			m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
			m_rcLower2.SetRect(0, 0, 0, 0);
		}
		else if (m_bShowMacd)
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
			m_rcLower2.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
			m_rcLower.SetRect(0, 0, 0, 0);

		}
		else
		{
			m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom);
			m_rcLower.SetRect(0, 0, 0, 0);
			m_rcLower2.SetRect(0, 0, 0, 0);
		}

	}




	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(192, 192, 192, 255), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		pts[0].SetPoint(m_rcAll.left - 49, m_rcAll.top - 25);
		pts[1].SetPoint(m_rcAll.left - 49, m_rcAll.bottom + 35);
		pts[2].SetPoint(m_rcAll.right + 41, m_rcAll.bottom + 35);
		pts[3].SetPoint(m_rcAll.right + 41, m_rcAll.top - 25);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		if (m_bShowDeal)
		{
			pts[0].SetPoint(m_rcImage.right + 41, m_rcAll.top - 25);
			pts[1].SetPoint(m_rcImage.right + 41, m_rcAll.bottom + 35);
			pRT->DrawLines(pts, 2);
			pts[0].SetPoint(m_rcImage.right + 41, m_pPriceList->m_rect.bottom - 5);
			pts[1].SetPoint(m_rcAll.right + 41, m_pPriceList->m_rect.bottom - 5);
			pRT->DrawLines(pts, 2);

		}

		pRT->SelectObject(oldPen);
	}


	if (m_bDataInited)
	{

		if (m_bShowBandTarget)
			BandDataUpdate();

		GetMaxDiff();

		if (m_bShowVolume)
			GetFuTuMaxDiff();

		if (m_bShowMacd)
		{
			MACDDataUpdate();
			GetMACDMaxDiff();
		}


		DrawTime(pRT, TRUE);

		DrawArrow(pRT);


		DrawData(pRT);

		CPoint po(m_nMouseX, m_nMouseY);
		m_nMouseX = m_nMouseY = -1;
		LONGLONG llTmp3 = GetTickCount64();
		if (m_bKeyDown)
			DrawKeyDownLine(pRT, m_bClearTip);
		else
			DrawMouse(pRT, po, TRUE);

		//if (m_bShowMouseLine)
		//{
		//	if (m_bKeyDown)
		//		DrawKeyDownLine(pRT);
		//	else
		//		DrawMouseLine(pRT, po);
		//}
		//DrawMouse(pRT, po, TRUE);

	}
	else
		DrawArrow(pRT);
	if (m_bShowDeal)
	{
		m_pPriceList->Paint(pRT);
		m_pDealList->Paint(pRT);
	}

	//	LOG_W(L"鼠标绘制时间:%I64d,总时间:%I64d\n", llTmp3 - llTmp2, llTmp3 - llTmp);
	AfterPaint(pRT, pa);
}

void SKlinePic::DrawArrow(IRenderTarget * pRT)
{
	//画k线区
	int nLen = m_rcUpper.bottom - m_rcUpper.top;
	int nYoNum = 9;		//y轴标示数量 9 代表画8根线
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(m_rcUpper.left, m_rcUpper.top);
		pts[1].SetPoint(m_rcUpper.left, m_rcUpper.bottom);
		pts[2].SetPoint(m_rcUpper.right, m_rcUpper.bottom);
		pts[3].SetPoint(m_rcUpper.right, m_rcUpper.top);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		pRT->SelectObject(oldPen);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));
	//	pRT->DrawText(L"你好dfsdfsdf", -1, m_rcFit, DT_SINGLELINE | DT_LEFT);
	//	pRT->TextOut(m_rcFit.left + 10, m_rcFit.top + 10, L"你好", 4);

	//k线区横向虚线
	COLORREF clRed = RGB(255, 0, 0);
	HDC pdc = pRT->GetDC();
	//	SetBkColor(pdc, RGBA(255, 0, 0, 0xff));
	for (int i = 0; i < nYoNum - 1; i++)
	{
		int nY = m_rcUpper.top + ((m_rcUpper.bottom - m_rcUpper.top - 20) / (nYoNum - 1) * i) + 20;
		if (i == 0)
		{
			CPoint pts[2];
			{
				CAutoRefPtr<IPen> oldPen, pen;
				//	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
				pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
				pRT->SelectObject(pen, (IRenderObj**)&oldPen);
				//y轴	//x轴
				pts[0].SetPoint(m_rcUpper.left, nY);
				pts[1].SetPoint(m_rcUpper.right, nY);
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
			}
		}
		else
		{
			for (int j = m_rcUpper.left + 1; j < m_rcUpper.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线
		}
		//k线区y轴加轴标
		if (m_bDataInited)
		{

			SStringW s1 = GetYPrice(nY);
			double fPrice = _wtof(s1);
			if (fPrice<15000 && fPrice>-15000)
				DrawTextonPic(pRT, CRect(m_rcUpper.left - RC_LEFT + 2, nY - 9, m_rcImage.left, nY + 16), s1, RGBA(255, 0, 0, 255), DT_CENTER);
			//		pRT->TextOut(m_rcUpper.left - RC_LEFT + 8, nY - 6, s1, -1);
		}
	}
	pRT->ReleaseDC(pdc);

	/*
	//k线区横轴位置所在y坐标
	int nlo = m_rcUpper.bottom / nYoNum * (nYoNum - 1) - 5 - m_rcUpper.bottom;
	if (nlo > 15 || nlo < -15)
	{
	SStringW strf1 = GetYPrice(m_rcUpper.bottom);
	pRT->TextOut(m_rcUpper.left -RC_LEFT + 8 , m_rcUpper.bottom - 5, strf1, -1);
	}*/

	//画指标区
	if (m_bShowVolume)
	{
		{
			CAutoRefPtr<IPen> oldPen;
			//	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
			pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pts[0].SetPoint(m_rcLower.left, m_rcLower.top);
			pts[1].SetPoint(m_rcLower.left, m_rcLower.bottom);
			pts[2].SetPoint(m_rcLower.right, m_rcLower.bottom);
			pts[3].SetPoint(m_rcLower.right, m_rcLower.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
		}

		//副图区横向虚线
		pdc = pRT->GetDC();
		for (int i = 1; i < 2; i++)
		{
			int nY = m_rcLower.bottom - ((m_rcLower.bottom - m_rcLower.top - 5) / 2 * i);
			for (int j = m_rcLower.left + 1; j < m_rcLower.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线

													//标注

			if (m_bDataInited)
			{
				SStringW s1 = GetFuTuYPrice(nY);
				DrawTextonPic(pRT, CRect(m_rcUpper.left - RC_LEFT + 2, nY - 9, m_rcImage.left, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
			}
		}
		pRT->ReleaseDC(pdc);
	}

	//指标2
	if (m_bShowMacd)
	{
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pts[0].SetPoint(m_rcLower2.left, m_rcLower2.top);
			pts[1].SetPoint(m_rcLower2.left, m_rcLower2.bottom);
			pts[2].SetPoint(m_rcLower2.right, m_rcLower2.bottom);
			pts[3].SetPoint(m_rcLower2.right, m_rcLower2.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
		}

		//副图区横向虚线
		pdc = pRT->GetDC();
		int nWidthMacd = (m_rcLower2.Height() - 20) / 4;

		for (int i = 0; i < 4; i++)
		{
			int nY = m_rcLower2.top + 20 + nWidthMacd*i;
			CPoint pts[2];
			{
				CAutoRefPtr<IPen> pen, oldPen;
				if (i == 2)
					pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
				else
					pRT->CreatePen(PS_SOLID, RGBA(75, 0, 0, 0xFF), 2, &pen);
				pRT->SelectObject(pen, (IRenderObj**)&oldPen);
				//y轴	//x轴
				pts[0].SetPoint(m_rcLower2.left, nY);
				pts[1].SetPoint(m_rcLower2.right, nY);
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
			}
		}
		pRT->ReleaseDC(pdc);

	}
}



void SKlinePic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{
	int nLen = m_rcUpper.right - m_rcUpper.left - RC_RIGHT_BACK;	//判断是否超出范围
	int  nJiange = 2;
	if (m_bNoJiange)
		nJiange = 0;
	m_nMaxKNum = nLen / (m_nKWidth + nJiange);
	m_nFirst = 0;
	m_nEnd = m_pAll->nTotal;
	int nTotal = m_pAll->nTotal;
	if (nTotal > m_nMaxKNum)
		m_nFirst = nTotal - m_nMaxKNum;


	//开始计算左右偏移(鼠标控制)
	if (m_nMove > 0)
	{
		if (m_nFirst <= 0 && m_nEnd == nTotal)
			m_nMove = 0;
		if (nTotal < m_nMove)
			m_nMove = nTotal;
		else if (m_nFirst > m_nMove)
		{
			m_nFirst -= m_nMove;
			m_nEnd = nTotal - m_nMove;
		}
		else
		{
			m_nEnd -= m_nFirst;
			m_nFirst = 0;
		}
	}

	if (m_bReSetFirstLine)
	{
		int tmpFirst = m_nMouseLinePos - m_nMaxKNum / 2;
		int tmpEnd = tmpFirst + (m_nEnd - m_nFirst);
		if (tmpFirst >= 0 && tmpEnd < nTotal)
		{
			m_nFirst = tmpFirst;
			m_nEnd = tmpEnd;
			m_nMove = nTotal - m_nEnd - 1;
		}
		m_bReSetFirstLine = false;
	}

	//	}
	//判断最大最小值
	//	OutputDebugString("判断最大值\n");
	double fMax = -100000000000000;
	double fMin = 100000000000000;
	auto& data = m_pAll->data;
	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (data[j].low < fMin)
			fMin = data[j].low;
		if (data[j].high > fMax)
			fMax = data[j].high;
	}
	//看指标中的数值大小
	//	if (dk == 0)
	//	{

	if (m_bShowMA)
	{
		for (int nt = 0; nt < MAX_MA_COUNT; nt++)
		{
			//			if (!m_pAll->m_bShowZtLine[nt])
			//				continue;
			auto& maData = m_pAll->fMa[nt];
			int j;
			if (m_nFirst > 0)
				j = m_nFirst - 1;
			else
				j = m_nFirst;
			for (j; j < m_nEnd; j++)
			{
				if ((nt == 0 && j >= 5) || (nt == 1 && j >= 10)
					|| (nt == 2 && j >= 20 || (nt == 3 && j >= 60)))
				{
					if (maData[j] < fMin)
						fMin = maData[j];
					if (maData[j] > fMax)
						fMax = maData[j];
				}
			}
		}
	}
	else if (m_bShowBandTarget)
	{
		for (int j = m_nFirst; j < m_nEnd; j++)
		{
			if (!m_pBandData->DataValid[j])
				continue;

			if (m_pBandData->LowerTrack2[j] < fMin)
				fMin = m_pBandData->LowerTrack2[j];
			if (m_pBandData->UpperTrack2[j] > fMax)
				fMax = m_pBandData->UpperTrack2[j];
		}

	}

	m_pAll->fMax = fMax;
	m_pAll->fMin = fMin;

	if (m_pAll->fMax == fMin || (fMax - fMin) < 0.0001)
	{
		m_pAll->fMax = m_pAll->fMax * 1.05;
		m_pAll->fMin = m_pAll->fMin*0.94;
	}
	if (m_pAll->fMax == 0)
		m_pAll->fMax = 1;
}

void SKlinePic::GetFuTuMaxDiff()		//判断副图坐标最大最小值和k线条数
{
	//判断最大最小值
	//	OutputDebugString("判断最大值\n");
	double fMax = -100000000000000;
	double fMin = 0;

	auto &data = m_pAll->data;
	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (data[j].vol > fMax)
			fMax = data[j].vol;
	}


	m_pAll->fSubMax = fMax;
	m_pAll->fSubMin = fMin;
	if (m_pAll->fSubMax == fMin)
		m_pAll->fSubMax = m_pAll->fSubMax * 1.1;
	if (m_pAll->fSubMax == 0)
		m_pAll->fSubMax = 1;
}

BOOL SKlinePic::IsInRect(int x, int y, int nMode)	//是否在坐标中,0为全部,1为上方,2为下方
{
	CRect *prc;
	switch (nMode)
	{
	case 0:
		prc = &m_rcImage;
		break;
	case 1:
		prc = &m_rcUpper;
		break;
	case 2:
		prc = &m_rcLower;
		break;
	default:
		return FALSE;
	}
	if (x >= prc->left && x <= prc->right &&
		y >= prc->top  && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

int SKlinePic::GetFuTuYPos(double fDiff)	//获得附图y位置
{
	double fPos = m_rcLower.top + (1 - (fDiff / m_pAll->fSubMax))  * (m_rcLower.Height() - 5) + RC_MAX;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SKlinePic::GetFuTuYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	if (nY > m_rcLower.bottom || nY < m_rcLower.top)
		return strRet;
	double fDiff = ((double)m_rcLower.bottom - nY) / (m_rcLower.Height() - 5)  * m_pAll->fSubMax;
	strRet.Format(L"%.0f", fDiff);
	return strRet;

}

void SKlinePic::GetMACDMaxDiff()		//判断副图坐标最大最小值和k线条数
{
	//判断最大最小值
	//	OutputDebugString("判断最大值\n");
	double fMax = -100000000000000;
	double fMin = 100000000000000;

	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (m_pMacdData->DEA[j] < fMin)
			fMin = m_pMacdData->DEA[j];
		if (m_pMacdData->DEA[j] > fMax)
			fMax = m_pMacdData->DEA[j];
		if (m_pMacdData->DIF[j] < fMin)
			fMin = m_pMacdData->DIF[j];
		if (m_pMacdData->DIF[j] > fMax)
			fMax = m_pMacdData->DIF[j];
		if (m_pMacdData->MACD[j] < fMin)
			fMin = m_pMacdData->MACD[j];
		if (m_pMacdData->MACD[j] > fMax)
			fMax = m_pMacdData->MACD[j];
	}


	if (fMax == fMin)
		fMax = fMax * 1.1;
	if (fMax == fMin&&fMax == 0)
		fMax = 1;


	//	}
	if (fMax < 0)
		fMax = 0 - fMin;
	else if (fMin > 0)
		fMin = 0 - fMax;
	else
	{
		if (fMax > (0 - fMin))
			fMin = 0 - fMax;
		else
			fMax = 0 - fMin;
	}



	m_pMacdData->fMax = fMax;
	m_pMacdData->fMin = fMin;

}

int SKlinePic::GetMACDYPos(double fDiff)
{
	double fPos = m_rcLower2.top + (1 - (fDiff / m_pMacdData->fMax)) / 2 * (m_rcLower2.Height() - 30) + 25;
	int nPos = (int)fPos;
	return nPos;

}

SStringW SKlinePic::GetMACDYPrice(int nY)
{
	int nWidth = (m_rcLower2.Height() - 20) / 4;
	SStringW strRet; strRet.Empty();
	if (nY > m_rcLower2.bottom || nY < m_rcLower2.top)
		return strRet;
	double fDiff = ((double)(nWidth * 2 + 20 + m_rcLower2.top) - nY) / (m_rcLower2.Height() - 30) * 2 * m_pMacdData->fMax;
	strRet.Format(L"%.2f", fDiff);
	return strRet;
}

int SKlinePic::GetYPos(double fDiff)
{
	double fPos = fDiff - m_pAll->fMin;
	double fPriceDiff = m_pAll->fMax - m_pAll->fMin;
	int nHeight = m_rcUpper.bottom - m_rcUpper.top - RC_MAX - RC_MIN - 20;
	fPos = m_rcUpper.bottom - fPos / fPriceDiff*nHeight + 0.5 - RC_MIN;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SKlinePic::GetYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	int nHeight = m_rcUpper.bottom - RC_MIN - m_rcUpper.top - RC_MAX - 20;
	double fPriceDiff = m_pAll->fMax - m_pAll->fMin;
	double fDiff = m_pAll->fMin + (double)(m_rcUpper.bottom - nY - RC_MIN) / nHeight*fPriceDiff;
	strRet.Format(SDECIMAL, fDiff);
	return strRet;

}

SStringW SKlinePic::GetAllYPrice(int nY)
{
	if (nY >= m_rcUpper.top && nY <= m_rcUpper.bottom)
		return  GetYPrice(nY);
	if (nY >= m_rcLower.top && nY <= m_rcLower.bottom)
		return  GetFuTuYPrice(nY);
	if (nY >= m_rcLower2.top && nY <= m_rcLower2.bottom)
		return  GetMACDYPrice(nY);
	return SStringW(L"");
}

void SKlinePic::OnMouseMove(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	int nTick = GetTickCount();
	if (nTick - m_nTickPre < 10 && m_nTickPre > 0)
		return;
	m_nTickPre = nTick;

	if (!m_bDataInited)
		return;
	if (point == m_preMovePoint)
		return;
	m_preMovePoint = point;
	m_bKeyDown = false;

	CPoint p = point;

	CRect rc = GetClientRect();
	CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	DrawMouse(pRT, point);
	AfterPaint(pRT, pa);
	ReleaseRenderTarget(pRT);

	//	LOG_W(L"on---ms:%d\n", GetTickCount() - nTick);
}

void SKlinePic::OnLButtonDown(UINT nFlags, CPoint point)
{
	//	OutputDebugString(L"1");
	//	SWindow::SetTimer(1, 16);
}

void SKlinePic::OnTimer(char cTimerID)
{
	//	SMessageBox(NULL, L"触发定时器", NULL, NULL);
	if (cTimerID == 125)	//刷新鼠标
	{
		//		if (m_bDataInited && !bPaintAfetrDataProc)
		//			Invalidate();
		//		else if (m_bDataInited&&bPaintAfetrDataProc)
		//			KillTimer(125);
	}
}

int SKlinePic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//	SWindow::SetTimer(1, 1000);
	return 0;
}

void SKlinePic::OnMouseLeave()
{
	if (m_nMouseX != -1 || m_nMouseY != -1)
	{
		m_nMouseX = m_nMouseY = -1;
		Invalidate();		//主要是为了消除未来得及消除的鼠标线
	}
}

int SKlinePic::GetXData(int nx) {	//获取鼠标下的数据id

	int nJiange = 2;
	if (m_bNoJiange)
		nJiange = 0;
	float fn = (float)(nx - m_rcUpper.left) / (float)(m_nKWidth + nJiange);
	int n = (int)fn;
	if (n < 0)
		n = 0;
	n += m_nFirst;
	return n;

}

int SKlinePic::GetXPos(int nx)
{

	int nPos = nx * m_nKWidth + 1 + m_rcUpper.left;
	nPos = nPos + m_nKWidth / 2;
	return nPos;
}


void SKlinePic::DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color, UINT uFormat)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.right - rc.left, rc.bottom - rc.top);
	CAutoRefPtr<IFont> oldFont;
	pMemRT->SelectObject(m_pFont12, (IRenderObj**)&oldFont);

	pMemRT->SetTextColor(color);				//字为白色
	pMemRT->DrawTextW(str, wcslen(str), CRect(0, 0, rc.right - rc.left, rc.bottom - rc.top), uFormat);
	pRT->BitBlt(rc, pMemRT, 0, 0, SRCINVERT);

}

void SKlinePic::DrawKeyDownLine(IRenderTarget * pRT, bool bClearTip)
{

	//画鼠标线
	if (m_pAll->nTotal <= 0)
		return;
	int  nJianGe = 2;
	if (m_bNoJiange)
		nJianGe = 0;

	int nx = m_nMouseLinePos - m_nFirst;
	int x = nx * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
	CPoint po;
	po.x = x + m_nKWidth / 2;
	po.y = GetYPos(m_pAll->data[m_nMouseLinePos].close);

	DrawMouseLine(pRT, po);

	auto &data = m_pAll->data[m_nMouseLinePos];
	DrawBarInfo(pRT, m_nMouseLinePos);
	if (bClearTip)
	{
		m_pTip->ClearTip();

		if (m_nMouseLinePos > 0)
			DrawMouseKlineInfo(pRT, data, po, m_nMouseLinePos, m_pAll->data[m_nMouseLinePos - 1].close);
		else
			DrawMouseKlineInfo(pRT, data, po);
		m_bClearTip = false;
	}

	DrawMovePrice(pRT, m_nMouseY, false);
	DrawMovePrice(pRT, po.y, true);

	DrawTime(pRT);

	if (IsInRect(m_nMouseX, m_nMouseY, 0))
		DrawMoveTime(pRT, m_nMouseX, 0, 0, false);

	DrawTime(pRT);

	if (IsInRect(po.x, po.y, 0))
		DrawMoveTime(pRT, po.x, data.date, data.time, true);

	m_nMouseX = po.x;
	m_nMouseY = po.y;

}


void SKlinePic::DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint)
{
	if (!bFromOnPaint)
		m_pTip->ClearTip();

	if (p.x == m_nMouseX && p.y == m_nMouseY)
		return;
	if (!IsInRect(p.x, p.y, 0))
	{
		if (m_nMouseX != -1 || m_nMouseY != -1)
		{
			m_nMouseX = m_nMouseY = -1;
			if (!bFromOnPaint)
			{
				Invalidate();
			}
		}
		return;
	}

	if (m_bShowMouseLine)
		DrawMouseLine(pRT, p);


	//显示当前K线的具体信息和指标信息
	int nNum = GetXData(p.x);
	nNum = nNum < m_nEnd ? nNum : m_nEnd - 1;
	pRT->SelectObject(m_bBrushBlack);
	SStringW strTemp, strDot;
	if (nNum >= 0)
	{
		if (m_bShowMouseLine)
			DrawBarInfo(pRT, nNum);
		auto & data = m_pAll->data[nNum];
		if (0 != data.date && !bFromOnPaint&&ptIsInKlineRect(p, nNum - m_nFirst, data))
		{
			if (nNum > 0)
				DrawMouseKlineInfo(pRT, data, p, nNum, m_pAll->data[nNum - 1].close);
			else
				DrawMouseKlineInfo(pRT, data, p);
		}
	}

	//显示横坐标所在数值
	DrawMovePrice(pRT, m_nMouseY, false);
	DrawMovePrice(pRT, p.y, true);


	//显示纵坐标数值

	DrawTime(pRT);

	if (IsInRect(m_nMouseX, m_nMouseY, 0))
		DrawMoveTime(pRT, m_nMouseX, 0, 0, false);

	DrawTime(pRT);

	if (IsInRect(p.x, p.y, 0))
	{
		int nx = GetXData(p.x);
		auto &data = m_pAll->data[nx];
		if (nx >= 0 && nx < m_pAll->nTotal && data.date>0)
			DrawMoveTime(pRT, p.x, data.date, data.time, true);
	}

	m_nMouseX = p.x;
	m_nMouseY = p.y;
}

void SKlinePic::DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint) //画竖线时间轴时间和标示数字
{
	int nJiange = 2;
	if (m_bNoJiange)
		nJiange = 0;

	int nXpre = 0;  //第一根竖线的x轴位置
	int nMaX = (m_nEnd - m_nFirst - 1)*(m_nKWidth + nJiange) + RC_LEFT + 1 + (m_nKWidth / 2);	//最后一个数据的位置
	KlineType *p = m_pAll->data + m_nFirst;

	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;
	pRT->SelectObject(m_penDotRed, (IRenderObj**)&oldPen);
	CPoint pts[5];
	HDC pdc = pRT->GetDC();

	int nPosMx[20] = { 0 };
	int nPosCount = 1;

	int nNowDay = m_pAll->data[m_nFirst].date;
	int nNowTime = m_pAll->data[m_nFirst].time;

	int nLastPos = 0;

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		int x = i * (m_nKWidth + nJiange) + 1 + m_rcUpper.left + (m_nKWidth / 2);
		int preX = (nPosMx[nPosCount - 1]) * (m_nKWidth + nJiange) + 1 + m_rcUpper.left + (m_nKWidth / 2);
		//加时间
		if ((x - nXpre - RC_LEFT >= 10 && x < nMaX - 200) || (i == 0 && m_nEnd - m_nFirst > 9) || i == m_nEnd - m_nFirst - 1)
		{
			//加时间
			SStringW strTime, strDate;
			strTime.Format(L"%02d:%02d:%02d", m_pAll->data[i + m_nFirst].time / 10000,
				m_pAll->data[i + m_nFirst].time % 10000 / 100,
				m_pAll->data[i + m_nFirst].time % 100);
			//加日期
			strDate.Format(L"%04d-%02d-%02d", m_pAll->data[i + m_nFirst].date / 10000,
				m_pAll->data[i + m_nFirst].date % 10000 / 100,
				m_pAll->data[i + m_nFirst].date % 100);

			//			pRT->TextOut(x - 25, m_rcImage.bottom + 5, strTemp, -1);
			DrawTextonPic(pRT, CRect(x - 25, m_rcImage.bottom + 5, x + 35, m_rcImage.bottom + 20), strTime);
			//			pRT->TextOut(x - 25, m_rcImage.bottom + 5 + 10, strTemp, -1);
			DrawTextonPic(pRT, CRect(x - 35, m_rcImage.bottom + 5 + 15, x + 45, m_rcImage.bottom + 35), strDate);

			//加竖线
			//		CPen pen2(PS_SOLID,1,RGB(112,128,144));	//画笔调整为白色
			//			pRT->SelectObject(m_penDotRed);

			if (bFromOnPaint)
			{
				if (nXpre > 0 || m_nEnd - m_nFirst < 10)
				{
					pts[0].SetPoint(x, m_rcImage.top + 20);
					pts[1].SetPoint(x, m_rcImage.bottom);
					//					pRT->DrawLines(pts, 2);
					//					for (int j = m_rcImage.top + 19; j < m_rcImage.bottom; j += 3)
					//						::SetPixelV(pdc, x, j, RGB(255, 0, 0));		//	划虚线
				}

			}
			nXpre += 300;

		}
		pRT->ReleaseDC(pdc);
		pRT->SelectObject(oldPen);
	}
}

void SKlinePic::DrawData(IRenderTarget * pRT)
{
	int  nJianGe = 2;
	if (m_bNoJiange)
		nJianGe = 0;
	if (m_pAll->nTotal <= 0)
		return;

	//	::EnterCriticalSection(&m_cs);

	if (m_bShowMacd)
	{
		for (int i = 0; i < 4; i++)
		{
			int nY = m_rcLower2.top + 20 + (m_rcLower2.Height() - 20) / 4 * i;

			//k线区y轴加轴标
			SStringW s1 = GetMACDYPrice(nY);

			DrawTextonPic(pRT, CRect(m_rcLower2.left - RC_LEFT + 5, nY - 9, m_rcLower2.left, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
		}
	}


	CPoint pts[5];
	int x = 0, yopen = 0, yclose = 0, yhigh = 0, ylow = 0;
	int ypreUpperTrack1 = 0, ypreUpperTrack2 = 0, ypreSellLong = 0,
		ypreBuyShort = 0, ypreLowerTrack1 = 0, ypreLowerTrack2 = 0;
	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;

	int nValidNum = -1;

	pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
	pRT->SelectObject(m_bBrushGreen, (IRenderObj**)&bOldBrush);

	KlineType *p = m_pAll->data + m_nFirst;

	CPoint *UpperLine1 = new CPoint[m_nEnd - m_nFirst];
	CPoint *UpperLine2 = new CPoint[m_nEnd - m_nFirst];
	CPoint *LowerLine1 = new CPoint[m_nEnd - m_nFirst];
	CPoint *LowerLine2 = new CPoint[m_nEnd - m_nFirst];
	CPoint *SellLongLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *BuyShortLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *DIFLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *DEALine = new CPoint[m_nEnd - m_nFirst];
	CPoint **MaLine = new CPoint*[4];
	for (int i = 0; i < 4; ++i)
		MaLine[i] = new CPoint[m_nEnd - m_nFirst];

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		int nOffset = i + m_nFirst;
		x = i * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
		auto data = m_pAll->data[nOffset];
		int nVolume = m_pAll->data[nOffset].vol;
		int nVolume1;
		if (m_nFirst + i > 0)
			nVolume1 = m_pAll->data[nOffset - 1].vol;


		if (!m_bShowBandTarget)
		{
			yopen = GetYPos(data.open);
			yhigh = GetYPos(data.high);
			ylow = GetYPos(data.low);
			yclose = GetYPos(data.close);

			//加数值
			int nTimeTmp = 10 + 10;
			if (((i + 1) % nTimeTmp == 0 && m_nEnd - m_nFirst - i > 10 && x < m_rcUpper.right - 50) || i == 0 || i == m_nEnd - m_nFirst - 1)
			{

				//加最后的数值
				if (i == m_nEnd - m_nFirst - 1)
				{
					SStringW strTemp;
					strTemp.Format(SDECIMAL, data.close);
					pRT->TextOut(x + m_nKWidth + 2, yclose - 5, strTemp, -1);

					CPoint pt;
					GetCursorPos(&pt);
					if (!m_bShowMouseLine || (pt.x > m_rcImage.right || pt.x<m_rcImage.left || pt.y>m_rcImage.bottom || pt.y < m_rcImage.top))
					{
						DrawMainUpperMarket(pRT, data);
						if (m_bShowMA)
							DrawMainUpperMA(pRT, nOffset);
						if (m_bShowMacd)
							DrawMacdUpperMarket(pRT, nOffset);
					}

				}

			}
			if (data.close > data.open)			//高低线
			{
				pRT->SelectObject(m_penRed);
				pts[0].SetPoint(x + m_nKWidth / 2, yclose);
				pts[1].SetPoint(x + m_nKWidth / 2, yhigh);
				pts[2].SetPoint(x + m_nKWidth / 2, yopen);
				pts[3].SetPoint(x + m_nKWidth / 2, ylow);
			}
			else if (data.close <= data.open)
			{
				if (data.close == data.open)
					pRT->SelectObject(m_penWhite);
				else
					pRT->SelectObject(m_penGreen);
				pts[0].SetPoint(x + m_nKWidth / 2, yopen);
				pts[1].SetPoint(x + m_nKWidth / 2, yhigh);
				pts[2].SetPoint(x + m_nKWidth / 2, yclose);
				pts[3].SetPoint(x + m_nKWidth / 2, ylow);
			}
			pRT->DrawLines(pts, 2);
			pRT->DrawLines(pts + 2, 2);
			if (data.close == data.open)
			{
				pts[0].SetPoint(x + 2, yopen);
				pts[1].SetPoint(x + m_nKWidth - 1, yopen);
				pRT->DrawLines(pts, 2);
			}
			else
			{
				if (data.close >= data.open)
					pRT->DrawRectangle(CRect(x, yopen, x + m_nKWidth, yclose));
				else
					pRT->FillSolidRect(CRect(x, yclose, x + m_nKWidth, yopen == yclose ? (yopen - 1) : yopen), RGBA(0, 255, 255, 255));
			}

			if (m_bShowMA)
			{
				auto & arrMA = m_pAll->fMa;
				for (int j = 0; j < 4; ++j)
					if (i + m_nFirst >= m_nMAPara[j] - 1)
						MaLine[j][i].SetPoint(x + m_nKWidth / 2, GetYPos(arrMA[j][nOffset]));
			}

		}
		else
		{
			if (m_pBandData->DataValid[nOffset])
			{
				//画轨道
				int yUpperTrack1 = GetYPos(m_pBandData->UpperTrack1[nOffset]);
				int yUpperTrack2 = GetYPos(m_pBandData->UpperTrack2[nOffset]);
				int ySellLong = GetYPos(m_pBandData->SellLong[nOffset]);
				int yBuyShort = GetYPos(m_pBandData->BuyShort[nOffset]);
				int yLowerTrack1 = GetYPos(m_pBandData->LowerTrack1[nOffset]);
				int yLowerTrack2 = GetYPos(m_pBandData->LowerTrack2[nOffset]);

				if (nValidNum == -1 && m_pBandData->DataValid[nOffset])
					nValidNum = i;

				SellLongLine[i].SetPoint(x + m_nKWidth / 2, ySellLong);

				BuyShortLine[i].SetPoint(x + m_nKWidth / 2, yBuyShort);

				UpperLine1[i].SetPoint(x + m_nKWidth / 2, yUpperTrack1);

				LowerLine1[i].SetPoint(x + m_nKWidth / 2, yLowerTrack1);

				UpperLine2[i].SetPoint(x + m_nKWidth / 2, yUpperTrack2);

				LowerLine2[i].SetPoint(x + m_nKWidth / 2, yLowerTrack2);
			}
		}

		if (m_bShowVolume)
		{
			if (nVolume != 0)
			{
				pRT->SelectObject(m_bBrushGreen);


				pRT->SelectObject(m_penRed);

				if (data.close > data.open)
					pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom));
				else if (data.close == data.open&&m_nFirst + i > 0)
				{
					if (data.close >= p[i - 1].close)
						pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom));
					else
						pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom), RGBA(0, 255, 255, 255));
				}
				else
					pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom), RGBA(0, 255, 255, 255));
			}
		}

		if (m_bShowMacd)
		{
			double yDIF = GetMACDYPos(m_pMacdData->DIF[nOffset]);
			double yDEA = GetMACDYPos(m_pMacdData->DEA[nOffset]);


			DIFLine[i].SetPoint(x + m_nKWidth / 2, yDIF);

			DEALine[i].SetPoint(x + m_nKWidth / 2, yDEA);


			int nWidthMacd = (m_rcLower2.Height() - 20) / 4;
			//MACD柱状图
			if (m_pMacdData->MACD[nOffset] != 0)
			{
				pts[0].SetPoint(x + m_nKWidth / 2, m_rcLower2.top + 20 + 2 * nWidthMacd);
				pts[1].SetPoint(x + m_nKWidth / 2, GetMACDYPos(m_pMacdData->MACD[i + m_nFirst]));

				if (m_pMacdData->MACD[nOffset] > 0)
					pRT->SelectObject(m_penRed);
				else
					pRT->SelectObject(m_penGreen);
				pRT->DrawLines(pts, 2);
			}

		}

	}


	if (m_bShowMA)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (m_nFirst >= m_nMAPara[i] - 1)
			{
				pRT->SelectObject(m_penWhite);
				pRT->DrawLines(MaLine[i], m_nEnd - m_nFirst);
			}
			else if (m_nEnd > m_nMAPara[i] - 1)
			{
				pRT->SelectObject(m_penWhite);
				pRT->DrawLines(MaLine[i] - m_nFirst + m_nMAPara[i] - 1, m_nEnd - m_nMAPara[i] + 1);

			}
		}
	}


	if (m_bShowBandTarget)
	{
		if (nValidNum + m_nFirst < m_nEnd&&nValidNum != -1)
		{
			pRT->SelectObject(m_penGray);
			pRT->DrawLines(SellLongLine + nValidNum, m_nEnd - nValidNum - m_nFirst);

			pRT->SelectObject(m_penWhite);
			pRT->DrawLines(BuyShortLine + nValidNum, m_nEnd - nValidNum - m_nFirst);

			pRT->SelectObject(m_penMAGreen);
			pRT->DrawLines(UpperLine1 + nValidNum, m_nEnd - nValidNum - m_nFirst);

			pRT->SelectObject(m_penYellow);
			pRT->DrawLines(LowerLine1 + nValidNum, m_nEnd - nValidNum - m_nFirst);

			pRT->SelectObject(m_penDotGreen);
			pRT->DrawLines(UpperLine2 + nValidNum, m_nEnd - nValidNum - m_nFirst);

			pRT->SelectObject(m_penDotYellow);
			pRT->DrawLines(LowerLine2 + nValidNum, m_nEnd - nValidNum - m_nFirst);

		}


		for (int i = 0; i < m_nEnd - m_nFirst; i++)
		{
			int nOffset = m_nEnd - m_nFirst;
			auto data = m_pAll->data[nOffset];
			x = i * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
			DrawBandData(pRT, nOffset, data, x, nJianGe);
			if (i == m_nEnd - m_nFirst - 1)
			{
				int nVolume = m_pAll->data[nOffset].vol;
				CPoint pt;
				GetCursorPos(&pt);
				if (!m_bShowMouseLine || (pt.x > m_rcImage.right || pt.x<m_rcImage.left || pt.y>m_rcImage.bottom || pt.y < m_rcImage.top))
				{
					DrawMainUpperMarket(pRT, data);
					DrawMainUpperBand(pRT, nOffset);
					if (m_bShowMacd)
						DrawMacdUpperMarket(pRT, nOffset);
				}

			}

		}
	}

	if (m_bShowMacd)
	{
		pRT->SelectObject(m_penWhite);
		pRT->DrawLines(DIFLine, m_nEnd - m_nFirst);

		pRT->SelectObject(m_penYellow);
		pRT->DrawLines(DEALine, m_nEnd - m_nFirst);

	}

	pRT->SelectObject(oldPen);
	pRT->SelectObject(bOldBrush);

	delete[]UpperLine1;
	delete[]UpperLine2;
	delete[]LowerLine1;
	delete[]LowerLine2;
	delete[]SellLongLine;
	delete[]BuyShortLine;
	delete[]DIFLine;
	delete[]DEALine;
	for (int i = 0; i < 4; ++i)
		delete[]MaLine[i];
	delete[]MaLine;

	UpperLine1 = nullptr;
	UpperLine2 = nullptr;
	LowerLine1 = nullptr;
	LowerLine2 = nullptr;
	SellLongLine = nullptr;
	BuyShortLine = nullptr;
	DIFLine = nullptr;
	DEALine = nullptr;
	MaLine = nullptr;


}


void SKlinePic::DrawBandData(IRenderTarget * pRT, int nDataPos, KlineType &data, int x, int nJiange)
{
	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;
	pRT->SelectObject(m_penGrey, (IRenderObj**)&oldPen);
	pRT->SelectObject(m_bBrushGrey, (IRenderObj**)&bOldBrush);

	int yHigh = GetYPos(data.high);
	int yLow = GetYPos(data.low);
	int yOpen = GetYPos(data.open);
	int yClose = GetYPos(data.close);




	if (m_pBandData->BB1[nDataPos] == 1)
		pRT->SelectObject(m_penRed);
	else if (m_pBandData->BB1[nDataPos] == 2)
		pRT->SelectObject(m_penGreen);
	else if (m_pBandData->BB1[nDataPos] == 3)
		pRT->SelectObject(m_penYellow);
	else if (m_pBandData->BB1[nDataPos] == 4)
		pRT->SelectObject(m_penGray);

	if (m_pBandData->W2[nDataPos] >= 0)
	{
		if (m_pBandData->BB1[nDataPos] == 2)
			pRT->FillSolidRect(CRect(x, yOpen, x + m_nKWidth, yClose), RGBA(0, 255, 255, 255));
		else if (m_pBandData->BB1[nDataPos] == 1)
			pRT->FillSolidRect(CRect(x, yOpen, x + m_nKWidth, yClose), RGBA(255, 0, 0, 255));
		else if (m_pBandData->BB1[nDataPos] == 3)
			pRT->FillSolidRect(CRect(x, yOpen, x + m_nKWidth, yClose), RGBA(255, 255, 0, 255));
		else if (m_pBandData->BB1[nDataPos] == 4)
			pRT->FillSolidRect(CRect(x, yOpen, x + m_nKWidth, yClose), RGBA(100, 100, 100, 255));

		CPoint pts[2];

		if (m_pBandData->W2[nDataPos] == 0)
		{
			pts[0].SetPoint(x, yOpen);
			pts[1].SetPoint(x + m_nKWidth, yOpen);
			pRT->DrawLines(pts, 2);
		}

		pts[0].SetPoint(x + m_nKWidth / 2, yOpen);
		pts[1].SetPoint(x + m_nKWidth / 2, yHigh);
		pRT->DrawLines(pts, 2);

		pts[0].SetPoint(x + m_nKWidth / 2, yClose);
		pts[1].SetPoint(x + m_nKWidth / 2, yLow);
		pRT->DrawLines(pts, 2);

	}
	else if (m_pBandData->W2[nDataPos] < 0)
	{
		pRT->DrawRectangle(CRect(x, yClose, x + m_nKWidth, yOpen));

		CPoint pts[2];
		pts[0].SetPoint(x + m_nKWidth / 2, yClose + 1);
		pts[1].SetPoint(x + m_nKWidth / 2, yHigh);
		pRT->DrawLines(pts, 2);

		pts[0].SetPoint(x + m_nKWidth / 2, yOpen - 1);
		pts[1].SetPoint(x + m_nKWidth / 2, yLow);
		pRT->DrawLines(pts, 2);

	}

}

void SKlinePic::DrawMouseKlineInfo(IRenderTarget * pRT, const KlineType  &KlData, CPoint pt, const int &num, const double &fPrePrice)
{
	CRect rc{ pt.x,pt.y,pt.x + 100,pt.y + 150 };

	SStringW tmp;
	if (num != 0)
	{
		tmp.Format(L"%02d-%02d-%02d %02d:%02d\n开盘:%.02f\n"
			"最高:%.02f\n最低:%.02f\n收盘:%.02f\n涨跌:%.02f\n涨跌幅:%.02f%%\n",
			(KlData.date / 10000) % 100, (KlData.date / 100) % 100, KlData.date % 100,
			KlData.time / 10000, (KlData.time / 100) % 100,
			KlData.open,
			KlData.high,
			KlData.low,
			KlData.close,
			KlData.close - fPrePrice,
			(KlData.close - fPrePrice) / fPrePrice * 100);
	}
	else
	{
		tmp.Format(L"%02d-%02d-%02d %02d:%02d\n开盘:%.02f\n"
			"最高:%.02f\n最低:%.02f\n收盘:%.02f\n涨跌:-\n涨跌幅:-\n",
			(KlData.date / 10000) % 100, (KlData.date / 100) % 100, KlData.date % 100,
			KlData.time / 10000, (KlData.time / 100) % 100,
			KlData.open,
			KlData.high,
			KlData.low,
			KlData.close);
	}
	m_pTip->UpdateTip(rc, tmp, GetScale());
	m_pTip->RelayEvent(pt);
}

void SKlinePic::DataProc()
{
	SingleDataProc();
	m_nBandCount = 0;
	m_nMacdCount = 0;

	UpdateData();
	m_bDataInited = true;
}

void SKlinePic::ReProcMAData()
{
	m_bDataInited = false;
	for (int i = 0; i < m_pAll->nTotal; i++)
		KlineMAProc(i);
	m_bDataInited = true;
}

void SKlinePic::DrawTickMainData(IRenderTarget * pRT)	//画主图tick
{
	LONGLONG llTmp1 = GetTickCount64();
	if (m_pAll == nullptr || m_pAll->nTotal <= 0)
		return;
	int  nJianGe = 1;
	if (m_nKWidth > 4)
		nJianGe = 2;

	CPoint pts[10000];
	int nPNum = 0;
	int x = 0, yopen = 0, yclose = 0, yhigh = 0, ylow = 0, ypre = 0;
	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;

	pRT->SelectObject(m_penGreen, (IRenderObj**)&oldPen);
	pRT->SelectObject(m_bBrushGreen, (IRenderObj**)&bOldBrush);

	auto *p = m_pAll->data + m_nFirst;
	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		x = i * m_nKWidth + 1 + m_rcUpper.left;
		yopen = GetYPos(p[i].open);
		yhigh = GetYPos(p[i].high);
		ylow = GetYPos(p[i].low);
		yclose = GetYPos(p[i].close);
		if (i == 0)
			ypre = yopen;
		//连接前和现开
		pts[nPNum++].SetPoint(x, yopen);
		//画开
		pts[nPNum++].SetPoint(x + m_nKWidth / 2, yopen);
		//竖线
		pts[nPNum++].SetPoint(x + m_nKWidth / 2, yhigh);
		pts[nPNum++].SetPoint(x + m_nKWidth / 2, ylow);
		//画收
		pts[nPNum++].SetPoint(x + m_nKWidth / 2, yclose);
		pts[nPNum++].SetPoint(x + m_nKWidth, yclose);
		ypre = yclose;

		//加最后的数值
		if (i == m_nEnd - m_nFirst - 1)
		{
			SStringW strTemp;
			strTemp.Format(SDECIMAL, p[i].close);
			pRT->TextOut(x + m_nKWidth + 2, yclose - 5, strTemp, -1);
		}
	}
	pRT->DrawLines(pts, nPNum);
	pRT->SelectObject(oldPen);
	pRT->SelectObject(bOldBrush);

	LONGLONG llTmp2 = GetTickCount64();

}

void SKlinePic::DrawVolData(IRenderTarget * pRT)
{
	//	LONGLONG llTmp1 = GetTickCount64();
	if (m_pAll == nullptr || m_pAll->nTotal <= 0)
		return;
	int  nJianGe = 1;
	if (m_nKWidth > 4)
		nJianGe = 2;

	CPoint pts[10000];
	int nPNum = 0, x = 0;
	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;

	pRT->SelectObject(m_penGreen, (IRenderObj**)&oldPen);
	pRT->SelectObject(m_bBrushGreen, (IRenderObj**)&bOldBrush);

	KlineType *p = m_pAll->data + m_nFirst;

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		x = i * m_nKWidth + 1 + m_rcUpper.left;
		int volPos = GetFuTuYPos(m_pAll->data[m_nFirst + i - 1].vol);
		if (m_nKWidth > 2)
		{
			pts[nPNum++].SetPoint(x + m_nKWidth / 2 - 1, m_rcLower.bottom);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2 - 1, volPos);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, volPos);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, m_rcLower.bottom);
		}
		else
		{
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, m_rcLower.bottom);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, volPos);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, m_rcLower.bottom);
		}

	}
	pRT->DrawLines(pts, nPNum);
	//	}
	//补红线
	pRT->SelectObject(m_penRed);
	pts[0].SetPoint(m_rcLower.left, m_rcLower.bottom);
	pts[1].SetPoint(m_rcLower.right, m_rcLower.bottom);
	pRT->DrawLines(pts, 2);

	pRT->SelectObject(oldPen);
	pRT->SelectObject(bOldBrush);

}

void SKlinePic::OnDbClickedKline(UINT nFlags, CPoint point)
{
	m_bShowMouseLine = !m_bShowMouseLine;
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
		m_nMouseX = m_nPreX;
		m_nMouseY = m_nPreY;
	}
	Invalidate();
}

void SKlinePic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_UP:
	{
		if (m_bKeyDown)
		{
			m_pTip->ClearTip();
			m_bReSetFirstLine = true;
		}
		if (m_nKWidth == 1 && m_bNoJiange)
		{
			m_bNoJiange = false;
			Invalidate();
			break;
		}
		else if (m_nKWidth == 65)
			break;
		m_nKWidth += 2;
		if (m_nKWidth > 64)
			m_nKWidth = 65;

		Invalidate();

	}
	break;
	case VK_DOWN:
	{
		if (m_bKeyDown)
		{
			m_pTip->ClearTip();
			m_bReSetFirstLine = true;
		}

		if (m_nKWidth == 1)
		{
			if (!m_bNoJiange)
			{
				m_bNoJiange = true;
				Invalidate();
			}
			break;
		}
		m_nKWidth -= 2;
		if (m_nKWidth < 1)
			m_nKWidth = 1;

		Invalidate();

	}
	break;
	case VK_LEFT:
	case VK_RIGHT:
	{
		if (!m_bKeyDown)
		{
			m_bKeyDown = true;
			m_nPreX = m_nMouseX;
			m_nPreY = m_nMouseY;
			if (m_bShowMouseLine)
			{
				int nx = GetXData(m_nMouseX);
				if (nx > m_pAll->nTotal)
					nx = m_pAll->nTotal;
				m_nMouseLinePos = nx;
			}
			else
			{
				m_nMouseX = m_nMouseY = -1;
				if (nChar == VK_LEFT)
					m_nMouseLinePos = m_pAll->nTotal - 1;
				else if (nChar == VK_RIGHT)
					m_nMouseLinePos = m_nFirst;
			}
			m_bShowMouseLine = true;
			Invalidate();
			break;
		}
		else
		{
			if (nChar == VK_LEFT)
				m_nMouseLinePos--;
			else if (nChar == VK_RIGHT)
				m_nMouseLinePos++;
			if (m_nMouseLinePos < 0)
				m_nMouseLinePos = 0;
			else if (m_nMouseLinePos >= m_pAll->nTotal)
				m_nMouseLinePos = m_pAll->nTotal - 1;

			if (m_nMouseLinePos < m_nFirst)
			{
				m_nMove++;
				m_bClearTip = true;
				Invalidate();
				break;
			}
			if (m_nMouseLinePos >= m_nFirst + m_nMaxKNum)
			{
				m_nMove--;
				m_bClearTip = true;

				Invalidate();
				break;
			}

		}
		CRect rc = GetClientRect();
		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
		SPainter pa;
		SWindow::BeforePaint(pRT, pa);
		DrawKeyDownLine(pRT, true);
		AfterPaint(pRT, pa);
		ReleaseRenderTarget(pRT);

	}
	break;
	default:
		break;
	}

}

void SKlinePic::DrawMouseLine(IRenderTarget * pRT, CPoint p)
{
	//画鼠标线
	//OutPutDebugStringFormat("前点 %d,%d 后点%d,%d\n", m_nMouseX, m_nMouseY, p.x, p.y);
	HDC hdc = pRT->GetDC();
	CRect rcClient;
	GetClientRect(rcClient);
	int  nMode = SetROP2(hdc, R2_NOTXORPEN);
	if (!m_bShowMacd)
	{
		MoveToEx(hdc, m_nMouseX, m_rcImage.top + 19, NULL);	LineTo(hdc, m_nMouseX, m_rcImage.bottom);
		MoveToEx(hdc, p.x, m_rcImage.top + 19, NULL);			LineTo(hdc, p.x, m_rcImage.bottom);
		MoveToEx(hdc, m_rcImage.left, m_nMouseY, NULL);	LineTo(hdc, m_rcImage.right, m_nMouseY);
		MoveToEx(hdc, m_rcImage.left, p.y, NULL);			LineTo(hdc, m_rcImage.right, p.y);

	}
	else
	{
		MoveToEx(hdc, m_nMouseX, m_rcImage.top + 19, NULL);	LineTo(hdc, m_nMouseX, m_rcLower2.top);
		MoveToEx(hdc, p.x, m_rcImage.top + 19, NULL);			LineTo(hdc, p.x, m_rcLower2.top);
		MoveToEx(hdc, m_nMouseX, m_rcLower2.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcLower2.bottom);
		MoveToEx(hdc, p.x, m_rcLower2.top + 20, NULL);			LineTo(hdc, p.x, m_rcLower2.bottom);
		MoveToEx(hdc, m_rcImage.left, m_nMouseY, NULL);	LineTo(hdc, m_rcImage.right, m_nMouseY);
		MoveToEx(hdc, m_rcImage.left, p.y, NULL);			LineTo(hdc, m_rcImage.right, p.y);

	}
	//	SelectObject(hdc, oldPen);
	SetROP2(hdc, nMode);

	pRT->ReleaseDC(hdc);

	//m_nMouseX = p.x;
	//m_nMouseY = p.y;

}

void SKlinePic::DrawMoveTime(IRenderTarget * pRT, int x, int date, int time, bool bNew)
{
	pRT->FillRectangle(CRect(x - 40, m_rcImage.bottom + 2, x + 40, m_rcImage.bottom + 34));

	if (bNew)
	{
		SStringW str;
		str.Format(L"%d-%02d-%02d  %02d:%02d:00", date / 10000,
			date % 10000 / 100,
			date % 100,
			time / 10000,
			time % 10000 / 100);
		DrawTextonPic(pRT, CRect(x - 35, m_rcImage.bottom + 2, x + 35, m_rcImage.bottom + 35), str, RGBA(255, 255, 255, 255), 0);
	}

}

void SKlinePic::DrawMovePrice(IRenderTarget * pRT, int y, bool bNew)
{
	pRT->SelectObject(m_bBrushBlack);
	if (bNew)
		pRT->FillRectangle(CRect(m_rcUpper.right + 1, m_rcUpper.top, m_rcUpper.right + RC_RIGHT, m_rcLower.bottom + RC_BOTTOM - 10));

	SStringW str = GetAllYPrice(y);
	if (str != L"")
		DrawTextonPic(pRT, CRect(m_rcUpper.right + 2, y + -6, m_rcUpper.right + RC_RIGHT, y + 15), str);

}

void SKlinePic::DrawBarInfo(IRenderTarget * pRT, int nDataPos)
{
	pRT->FillRectangle(CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top));
	pRT->FillRectangle(CRect(m_rcImage.left + 1, m_rcImage.top + 4, m_rcImage.right, m_rcImage.top + 19));

	auto & data = m_pAll->data[nDataPos];
	DrawMainUpperMarket(pRT, data);

	if (m_bShowMA)
		DrawMainUpperMA(pRT, nDataPos);
	if (m_bShowBandTarget)
		DrawMainUpperBand(pRT, nDataPos);

	if (m_bShowMacd)
		DrawMacdUpperMarket(pRT, nDataPos);
}

double SKlinePic::GetHighPrice(int n, int nPeriod, int nOffset)
{
	int nSize = m_pAll->nTotal;
	if (nSize <= 0 || nSize < n + 1 || n - nOffset < 0)
		return DATA_ERROR;
	double fHigh = m_pAll->data[n - nOffset].high;
	for (int i = 0; i < nPeriod; i++)
	{
		int nPos = n - nOffset - i;
		if (nPos < 0)
			break;
		if (fHigh < m_pAll->data[nPos].high)
			fHigh = m_pAll->data[nPos].high;
	}
	return fHigh;
}

double SKlinePic::GetLowPrice(int n, int nPeriod, int nOffset)
{
	int nSize = m_pAll->nTotal;
	if (nSize <= 0 || nSize < n + 1 || n - nOffset < 0)
		return DATA_ERROR;
	double fLow = m_pAll->data[n - nOffset].low;
	for (int i = 0; i < nPeriod; i++)
	{
		int nPos = n - nOffset - i;
		if (nPos < 0)
			break;
		if (fLow > m_pAll->data[nPos].low)
			fLow = m_pAll->data[nPos].low;
	}
	return fLow;
}

int SKlinePic::Count(double a[], double b[], int nType, int n, int nPeriod)
{
	int nSize = m_pAll->nTotal;
	if (nSize <= 0 || nSize < n + 1)
		return DATA_ERROR;
	int nCount = 0;
	if (nType == 0)
	{
		for (int i = 0; i < nPeriod; i++)
		{
			int nPos = n - i;
			if (nPos < 0)
				break;
			if (a[nPos] > b[nPos])
				nCount++;
		}
	}
	else if (nType == 1)
	{
		for (int i = 0; i < nPeriod; i++)
		{
			int nPos = n - i;
			if (nPos < 0)
				break;
			if (a[nPos] < b[nPos])
				nCount++;
		}

	}
	return nCount;
}

bool SKlinePic::Cross(double a[], double b[], int nPos)
{
	if (nPos > 0)
	{
		if (a[nPos] > b[nPos] && a[nPos - 1] < b[nPos - 1])
			return true;
	}

	return false;
}


int SKlinePic::ValueWhen(int a[], int b[], int nPos)
{
	if (nPos > 0 && m_pBandData->DataValid[nPos])
	{
		if (a[nPos] + b[nPos] > 0)
			return a[nPos] + b[nPos];
		else
			return ValueWhen(a, b, nPos - 1);
	}
	else
		return 0;
}

void SKlinePic::DataInit()
{
	if (m_pAll == nullptr)
		m_pAll = new AllKPIC_INFO;
	ZeroMemory(m_pAll, sizeof(AllKPIC_INFO));


	int nsize = sizeof(AllKPIC_INFO);
	m_pAll->nTotal = 0;

	m_nUsedTickCount = 0;

}

bool SKlinePic::GenerateMultiMinFromOne(int nCount, KlineType & data, int nPeriod)
{
	auto &target = m_pAll->data[nCount];
	int time = 0;
	if (nPeriod != 60)
	{
		int left = data.time % 100 % nPeriod;
		time = left == 0 ? data.time : data.time - left + nPeriod;
		if (time % 100 == 60)
			time = ((time / 100) + 1) * 100;
	}
	else
	{
		if (time >= 930 && time <= 1030)
			time = 1030;
		if (time > 1030 && time <= 1130)
			time = 1130;
		if (time >= 1300 && time <= 1400)
			time = 1400;
		if (time > 1400 && time <= 1500)
			time = 1500;
	}
	if (time != target.time)
	{
		if (target.date == 0)
		{
			target = data;
			return false;
		}
		else
		{
			m_pAll->data[++nCount] = data;
			return true;
		}
	}

	target.close = data.close;
	target.high = max(target.high, data.high);
	target.low = max(target.low, data.low);
	target.vol += data.vol;
	return false;

}

void SKlinePic::SingleDataProc()
{
	DataInit();
	SingleDataWithHis();
	SingleDataUpdate();
}

void SKlinePic::SingleDataWithHis()
{

	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

	bool bStart = false;
	std::vector<KlineType> recVec;
	if (m_nPeriod != 1440)
	{
		if (GetFileKlineData(m_strSubIns, recVec))
		{
			int VecSize = recVec.size();
			int count = 0;
			for (auto iter = recVec.begin(); iter != recVec.end(); iter++)
			{
				if (m_nPeriod == 1)
				{
					if (recVec.end() - iter < 4700)		//1分钟K线
					{
						m_pAll->data[count++] = *iter;
						KlineMAProc(count);
					}

				}
				else
				{
					
					if (recVec.end() - iter < 4700 * m_nPeriod)		//5分钟K线
					{
						if (GenerateMultiMinFromOne(count, *iter, m_nPeriod))
							KlineMAProc(++count);
					}

				}
			}
			m_pAll->nTotal = count;
			if (!recVec.empty())
				m_pAll->nLast1MinTime = recVec.back().time;
		}
	}
	else
	{
		//处理日K线
		if (GetFileKlineData(m_strSubIns, recVec, true) && !recVec.empty())
		{
			int VecSize = recVec.size();
			int count = 0;
			for (auto iter = recVec.cbegin(); iter != recVec.cend(); iter++)
			{
				if (recVec.cend() - iter < 4700)		//1分钟K线
				{
					m_pAll->data[count++] = *iter;
					KlineMAProc(count);
				}
			}
			m_pAll->nTotal = count;
		}
	}


}

void SKlinePic::SingleDataUpdate()
{
	if (!m_bIsStockIndex)
	{
		if (m_nPeriod == 1)
			StockMarket1MinUpdate();
		else if (m_nPeriod != 1440)
			StockMarketMultMinUpdate(m_nPeriod);
		else
			StockMarketDayUpdate();
	}
	else
	{
		if (m_nPeriod == 1)
			IndexMarket1MinUpdate();
		else if (m_nPeriod != 0)
			IndexMarketMultMinUpdate(m_nPeriod);
		else
			IndexMarketDayUpdate();
	}
}


void SKlinePic::StockMarket1MinUpdate()
{
	vector<CommonStockMarket> TickVec(m_pStkMarketVec->begin() + m_nUsedTickCount, m_pStkMarketVec->end());
	if (TickVec.empty())
		return;
	for (size_t i = 0; i < TickVec.size(); ++i)
	{
		++m_nUsedTickCount;
		int ntime = (TickVec[i].UpdateTime / 100'000) * 100;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time || m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			StockTickToKline(m_pAll->nTotal, TickVec[i], true, ntime);
			m_pAll->nTotal++;
		}
		else if (ntime == m_pAll->data[m_pAll->nTotal - 1].time && m_nTradingDay == m_pAll->data[m_pAll->nTotal - 1].date)
			StockTickToKline(m_pAll->nTotal - 1, TickVec[i]);
		KlineMAProc(m_pAll->nTotal);
	}
}

void SKlinePic::StockMarketMultMinUpdate(int nPeriod)
{
	vector<CommonStockMarket> TickVec(m_pStkMarketVec->begin() + m_nUsedTickCount, m_pStkMarketVec->end());
	for (size_t i = 0; i < TickVec.size(); ++i)
	{
		++m_nUsedTickCount;
		int ntime = (TickVec[i].UpdateTime / 100'000) * 100;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time + (nPeriod - 1) * 100 ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			int nLeft = (ntime % 10000) / 100 % nPeriod;
			ntime -= nLeft * 100;
			StockTickToKline(m_pAll->nTotal, TickVec[i], true, ntime);
			m_pAll->nTotal++;
		}
		else if (ntime >= m_pAll->data[m_pAll->nTotal - 1].time &&
			m_nTradingDay == m_pAll->data[m_pAll->nTotal - 1].date) //最后一条K线并不完整
			StockTickToKline(m_pAll->nTotal - 1, TickVec[i]);
		KlineMAProc(m_pAll->nTotal);
	}

}

void SKlinePic::StockMarketDayUpdate()
{

	if (m_pStkMarketVec->empty())
		return;
	auto tick = m_pStkMarketVec->back();
	int nDataCount = m_pAll->nTotal;
	if (tick.OpenPrice > 10000000 || tick.OpenPrice == 0)
		return;
	if (!m_bAddDay)
	{
		StockTickToDayKline(nDataCount, tick);
		m_pAll->nTotal++;
		m_bAddDay = true;
	}
	else
		StockTickToDayKline(nDataCount - 1, tick);
	KlineMAProc(m_pAll->nTotal);
}

void SKlinePic::StockTickToKline(int nCount, CommonStockMarket & tick, bool bNewLine, int time)
{
	auto &kline = m_pAll->data[nCount];
	if (bNewLine)
	{
		kline.close = tick.LastPrice;
		kline.open = tick.LastPrice;
		kline.high = tick.LastPrice;
		kline.low = tick.LastPrice;
		kline.vol = tick.Volume - m_pAll->nLastVolume;
		kline.time = time;
		kline.date = m_nTradingDay;
	}
	else
	{
		kline.close = tick.LastPrice;
		kline.high = max(tick.LastPrice, kline.high);
		kline.low = min(tick.LastPrice, kline.low);
		kline.vol += tick.Volume - m_pAll->nLastVolume;
	}
	m_pAll->nLastVolume = tick.Volume;
}

void SKlinePic::StockTickToDayKline(int nCount, CommonStockMarket & tick)
{
	auto &kline = m_pAll->data[nCount];
	kline.close = tick.LastPrice;
	kline.open = tick.OpenPrice;
	kline.high = tick.HighPrice;
	kline.low = tick.LowPrice;
	kline.vol = tick.Volume;
	kline.date = m_nTradingDay;
	kline.time = 0;
}

void SKlinePic::IndexMarket1MinUpdate()
{
	vector<CommonIndexMarket> TickVec(m_pIdxMarketVec->begin() + m_nUsedTickCount, m_pIdxMarketVec->end());

	if (TickVec.empty())
		return;

	for (size_t i = 0; i < TickVec.size(); ++i)
	{
		++m_nUsedTickCount;
		int ntime = (TickVec[i].UpdateTime / 100'000) * 100;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time || m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			IndexTickToKline(m_pAll->nTotal, TickVec[i], true, ntime);
			m_pAll->nTotal++;
		}
		else if (ntime == m_pAll->data[m_pAll->nTotal - 1].time && m_nTradingDay == m_pAll->data[m_pAll->nTotal - 1].date)
			IndexTickToKline(m_pAll->nTotal - 1, TickVec[i]);
		KlineMAProc(m_pAll->nTotal);
	}
}

void SKlinePic::IndexMarketMultMinUpdate(int nPeriod)
{
	vector<CommonIndexMarket> TickVec(m_pIdxMarketVec->begin() + m_nUsedTickCount, m_pIdxMarketVec->end());
	for (size_t i = 0; i < TickVec.size(); ++i)
	{
		++m_nUsedTickCount;
		int ntime = (TickVec[i].UpdateTime / 100'000) * 100;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time + (nPeriod - 1) * 100 ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			int nLeft = (ntime % 10000) / 100 % nPeriod;
			ntime -= nLeft * 100;
			IndexTickToKline(m_pAll->nTotal, TickVec[i], true, ntime);
			m_pAll->nTotal++;
		}
		else if (ntime >= m_pAll->data[m_pAll->nTotal - 1].time &&
			m_nTradingDay == m_pAll->data[m_pAll->nTotal - 1].date) //最后一条K线并不完整
			IndexTickToKline(m_pAll->nTotal - 1, TickVec[i]);
		KlineMAProc(m_pAll->nTotal);
	}

}

void SKlinePic::IndexMarketDayUpdate()
{

	if (m_pIdxMarketVec->empty())
		return;
	auto tick = m_pIdxMarketVec->back();
	int nDataCount = m_pAll->nTotal;
	if (tick.OpenPrice > 10000000 || tick.OpenPrice == 0)
		return;
	if (!m_bAddDay)
	{
		IndexTickToDayKline(nDataCount, tick);
		m_pAll->nTotal++;
		m_bAddDay = true;
	}
	else
		IndexTickToDayKline(nDataCount - 1, tick);
	KlineMAProc(m_pAll->nTotal);
}

void SKlinePic::IndexTickToKline(int nCount, CommonIndexMarket & tick, bool bNewLine, int time)
{
	auto &kline = m_pAll->data[nCount];
	if (bNewLine)
	{
		kline.close = tick.LastPrice;
		kline.open = tick.LastPrice;
		kline.high = tick.LastPrice;
		kline.low = tick.LastPrice;
		kline.vol = tick.Volume - m_pAll->nLastVolume;
		kline.time = time;
		kline.date = m_nTradingDay;
	}
	else
	{
		kline.close = tick.LastPrice;
		kline.high = max(tick.LastPrice, kline.high);
		kline.low = min(tick.LastPrice, kline.low);
		kline.vol += tick.Volume - m_pAll->nLastVolume;
	}
	m_pAll->nLastVolume = tick.Volume;
}

void SKlinePic::IndexTickToDayKline(int nCount, CommonIndexMarket & tick)
{
	auto &kline = m_pAll->data[nCount];
	kline.close = tick.LastPrice;
	kline.open = tick.OpenPrice;
	kline.high = tick.HighPrice;
	kline.low = tick.LowPrice;
	kline.vol = tick.Volume;
	kline.date = m_nTradingDay;
	kline.time = 0;
}





bool SKlinePic::ProcKlineTime(int & time)
{
	if (time < 925 || (time > 1130 && time < 1300) || time>1500)
		return false;
	if (time > 925 && time < 930)
		time = 930;
	if (time == 1500)
		time = 1459;
	if (time == 1130)
		time = 1129;
	return true;
}


void SKlinePic::KlineMAProc(int nCount)
{
	if (nCount >= m_nMAPara[0])
	{
		double Ma1Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[0] - 1; j--)
			Ma1Sum += m_pAll->data[j].close;
		m_pAll->fMa[0][nCount - 1] = Ma1Sum / (double)m_nMAPara[0];
	}

	if (nCount >= m_nMAPara[1])
	{
		double Ma2Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[1] - 1; j--)
			Ma2Sum += m_pAll->data[j].close;
		m_pAll->fMa[1][nCount - 1] = Ma2Sum / (double)m_nMAPara[1];
	}

	if (nCount >= m_nMAPara[2])
	{
		double Ma3Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[2] - 1; j--)
			Ma3Sum += m_pAll->data[j].close;
		m_pAll->fMa[2][nCount - 1] = Ma3Sum / (double)m_nMAPara[2];
	}

	if (nCount >= m_nMAPara[3])
	{
		double Ma4Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[3] - 1; j--)
		{
			Ma4Sum += m_pAll->data[j].close;
		}
		m_pAll->fMa[3][nCount - 1] = Ma4Sum / (double)m_nMAPara[3];
	}

}

void SKlinePic::ReProcKlineData(bool bSingleNeedProc)
{
	if (!bSingleNeedProc)
		return;
	m_bDataInited = false;
	m_pAll->clear();
	m_nUsedTickCount = 0;
	SingleDataWithHis();
	SingleDataUpdate();

	m_bDataInited = true;
}

void SKlinePic::UpdateData()
{
	if (m_pAll == nullptr)
		return;
	SingleDataUpdate();
}

void SKlinePic::OnSize(UINT nType, CSize size)
{
	if (size.cx < 350)
		m_nKWidth = 8;
	else if (size.cx < 500)
		m_nKWidth = 12;
	else
		m_nKWidth = K_WIDTH_TOTAL;
}

BOOL SKlinePic::ptIsInKlineRect(CPoint pt, int nDataCount, KlineType &data)
{
	int	nJianGe = 2;
	if (m_bNoJiange)
		nJianGe = 0;

	int nTop = GetYPos(data.high);
	int nBottom = GetYPos(data.low);
	int nLeft = nDataCount * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
	int nRight = nLeft + m_nKWidth;

	if (pt.x >= nLeft&&pt.x <= nRight&&pt.y >= nTop&&pt.y <= nBottom)
		return TRUE;
	return FALSE;

}


