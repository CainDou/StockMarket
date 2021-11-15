#include "stdafx.h"
#include "SKlinePic.h"
#include <map>
#include <vector>
#include<gdiplus.h>
#include"DataProcFunc.h"
#include<unordered_map>
#include"DealList.h"
#include"PriceList.h"


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


//extern bool	g_bGroup1MinKlineChanged[MAXINSCOUNT];

extern std::map<InsIDType, bool>g_Group1MinKlineChangedMap;

extern CRITICAL_SECTION g_csKline;
extern CRITICAL_SECTION g_csGroupKline;
extern CRITICAL_SECTION g_csCAInfo;


extern std::map<InsIDType, std::vector<RestoreTickType>> g_TickHash;	//Tick数据哈希表
extern std::map<InsIDType, bool>g_UpdateHash;								//Tick数据是否更新
extern SStringWList g_arInsID;													//合约代码列表
extern SStringWList g_arInsName;												//合约名称列表
extern SStringWList g_arExcID;													//合约交易所代码表
extern std::map<InsIDType, bool>g_DownLoadHash;							//历史数据下载哈希表

extern int g_nGroupCount;
extern std::map<InsIDType, std::vector<GroupDataType>>g_GroupTickHash;		//组合合约tick数据哈希表
//extern std::vector<GroupInsType> g_GroupVec;									//组合合约表
extern std::map<InsIDType, std::vector<StockIndex_t>>g_StockIndexTickHash;

extern std::map<InsIDType, ComboInsType>g_GroupInsMap;

extern CRITICAL_SECTION g_csTick;
extern CRITICAL_SECTION g_csGroupTick;
//extern  bool g_bIsGroup[16];

//extern  int	g_nSubIns[16];
extern std::map<InsIDType, std::vector<KLineDataType>>g_KlineHash;		//K线数据哈希表
extern std::map<InsIDType, std::vector<GroupKlineType>>g_GroupKlineHash;	//组合合约k线数据哈希表
extern std::map<InsIDType, CallAutionData_t>g_CADataMap;


extern HWND g_MainWnd;

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
	m_nGroupVolume = 0;
	m_nGroupTime = 0;
	m_bGroupIsWhole = 0;
	m_nTradingDay = false;
	m_nPeriod = 1;
	m_pDealList = new CDealList;
	m_pPriceList = new CPriceList;
	m_bSubInsisGroup = false;
	m_strSubIns = "";
	m_pGroupDataType = nullptr;
	m_nFirst = 0;
}

SKlinePic::~SKlinePic()
{
	if (m_pAll);
		delete m_pAll;
	if(m_pPriceList)
		delete m_pPriceList;
	if(m_pDealList)
		delete m_pDealList;
	if(m_pTip)
		delete m_pTip;
}

void SOUI::SKlinePic::InitShowPara()
{
	m_pTip = new SKlineTip(m_hParWnd);
	m_pTip->Create();

	m_bShowBandTarget = m_InitPara.bShowBandTarget;
	m_bShowVolume = m_InitPara.bShowKlineVolume;
	m_bShowHighLow = m_InitPara.bShowHighLow;
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

	m_nCAType = m_InitPara.nCAType;

	m_bNoJiange = m_InitPara.bNoJiange;
	m_pPriceList->m_pGroupDataType = m_pGroupDataType;
	m_pDealList->m_pGroupDataType = m_pGroupDataType;
}

void SOUI::SKlinePic::BandDataUpdate()
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

void SOUI::SKlinePic::MACDDataUpdate()
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


int SOUI::SKlinePic::ProcBandTargetData(int nPos, Band_t *pBandData)
{

	DATA_FOR_SHOW* Data = m_pAll->m_Klines.pd;
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

int SOUI::SKlinePic::ProcMACDData(int nPos, MACDData_t * pMacdData)
{
	DATA_FOR_SHOW* Data = m_pAll->m_Klines.pd;
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
		pMacdData->EMA12[nPos] = GetEMA(m_nMACDPara[0], pMacdData->EMA12[nPos - 1], Data[nPos].close);
		pMacdData->EMA26[nPos] = GetEMA(m_nMACDPara[1], pMacdData->EMA26[nPos - 1], Data[nPos].close);
		pMacdData->DIF[nPos] = pMacdData->EMA12[nPos] - pMacdData->EMA26[nPos];
		pMacdData->DEA[nPos] = GetEMA(m_nMACDPara[2], pMacdData->DEA[nPos - 1], pMacdData->DIF[nPos]);
		pMacdData->MACD[nPos] = 2 * (pMacdData->DIF[nPos] - pMacdData->DEA[nPos]);
	}

	return nPos;
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
		bPaintAfetrDataProc = true;

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
		//		for (int dk = 0; dk < MAX_KLINE_COUNT; dk++)
		//		{
		//			if (!m_pAll->m_Klines.bShow)
		//				continue;

		DrawArrow(pRT);


		DrawData(pRT);
		//		}

		CPoint po(m_nMouseX, m_nMouseY);
		m_nMouseX = m_nMouseY = -1;
		LONGLONG llTmp3 = GetTickCount64();
		DrawMouse(pRT, po, TRUE);

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
			int nY;
			if (m_bSubInsisGroup)
				nY = (m_rcLower.bottom + m_rcLower.top) / 2;
			else
				nY = m_rcLower.bottom - ((m_rcLower.bottom - m_rcLower.top - 5) / 2 * i);
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

/*

四： GDI函数调用次数很重要，用上双缓冲和消除刷新背景之后，闪烁的问题可以认为是解决了，下面我们将讨论减少CPU使用率的问题。
GDI操作是比较浪费CPU资源的，比如频率的调用GDI函数，势必浪费CPU资源，这其中又特别是字符的打印操作，为此GDI提供PolyTextOut函数，
调用它一次，可以输出任意多条的字符，每一条都是独立设置输出位置的，这个函数的使用场合我举个例子，比如你在绘制一个坐标上的刻度值，
那么这个函数再好不过了。与此功能类似的还有PolylineTo、Polyline、PolyPolyline、Polygon、PolyPolygon等函数。
HDC hdc = pRT->GetDC();

LOGFONT logFont;
memset(&logFont, 0, sizeof(logFont));
// 字体大小
logFont.lfHeight = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
logFont.lfWidth = 0;    // 字体的宽度(默认)
logFont.lfWeight = 0;    // 字体的磅数(默认,FW_BOLD为加粗)
logFont.lfItalic = 0;    // 斜体
logFont.lfUnderline = 0; // 下划线
logFont.lfStrikeOut = 0; // 删除线
logFont.lfCharSet = 0;   // 字符集(默认)
// 字体名称
_stprintf(logFont.lfFaceName, TEXT("%s"), TEXT("宋体"));

HFONT hFont;
hFont = CreateFontIndirect(&(logFont));
SelectObject(hdc, hFont);
pRT->ReleaseDC(hdc);


CFont font1;
VERIFY(font1.CreatePointFont(96, "宋体", pdc));//为DC创建字体
CFont *oldfont = pdc->SelectObject(&font1);
int nMode = pdc->SetBkMode(TRANSPARENT);
pdc->SetTextColor(RGB(255, 255, 255));			//字为白色

//y轴加轴标
for (i = 1; i < nYoNum; i++)
{
CString f1 = GetYPrice(m_rcFit.bottom / nYoNum * i);
pdc->TextOut(1, m_rcFit.bottom / nYoNum * i - 5, f1);
}
//横轴位置所在y坐标
int nlo = m_rcFit.bottom / nYoNum * (nYoNum - 1) - 5 - m_rcFit.bottom + RC_BOTTOM;
if (nlo > 15 || nlo < -15)
{
CString strf1 = GetYPrice(m_rcFit.bottom - RC_BOTTOM);
pdc->TextOut(1, m_rcFit.bottom - RC_BOTTOM - 5, strf1);
}
}
*/


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
	//	}
		//判断最大最小值
		//	OutputDebugString("判断最大值\n");
	double fMax = -100000000000000;
	double fMin = 100000000000000;
	DATA_FOR_SHOW *p = m_pAll->m_Klines.pd;
	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (p[j].low < fMin)
			fMin = p[j].low;
		if (p[j].high > fMax)
			fMax = p[j].high;
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
			double* pf = m_pAll->m_Klines.fMa[nt];
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
					if (pf[j] < fMin)
						fMin = pf[j];
					if (pf[j] > fMax)
						fMax = pf[j];
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

	m_pAll->m_Klines.fMax = fMax;
	m_pAll->m_Klines.fMin = fMin;

	if (m_pAll->m_Klines.fMax == fMin || (fMax - fMin) < 0.0001)
	{
		m_pAll->m_Klines.fMax = m_pAll->m_Klines.fMax * 1.05;
		m_pAll->m_Klines.fMin = m_pAll->m_Klines.fMin*0.94;
	}
	if (m_pAll->m_Klines.fMax == 0)
		m_pAll->m_Klines.fMax = 1;


	nLen = m_rcUpper.bottom - m_rcUpper.top - RC_MAX - RC_MIN;
	double fDiff = 0;
	fDiff = m_pAll->m_Klines.fMax - m_pAll->m_Klines.fMin;
	fDiff *= m_pAll->m_Klines.nDecimalXi;
	int nDiff = (int)fDiff;
	if (nDiff == 0)
	{
		nDiff = (int)(fDiff * m_pAll->m_Klines.nDecimalXi);
		if (nDiff == 0)
		{
			nDiff = 1;
			//	LOG_W(L"nDiff异常!");
		}
	}
	m_pAll->m_Klines.nDivY = 0;
	while (nDiff > nLen && nLen > 0)
	{
		nDiff /= 10;
		m_pAll->m_Klines.nDivY++;
	}
	m_pAll->m_Klines.nMulY = nLen / nDiff;

}

void SKlinePic::GetFuTuMaxDiff()		//判断副图坐标最大最小值和k线条数
{
	//判断最大最小值
	//	OutputDebugString("判断最大值\n");
	double fMax = -100000000000000;
	double fMin = 100000000000000;

	//	for (int i = 0; i<MAX_FLINE_COUNT; i++)
	//	{
	//		if (m_pAll->m_Futu.bft[i] == 0)
	//			continue;
	int *p = m_pAll->m_Futu.ftl;
	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (p[j] < fMin)
			fMin = p[j];
		if (p[j] > fMax)
			fMax = p[j];
	}

	//	}
	if (!m_bSubInsisGroup)
		fMin = 0;
	else
	{
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
	}

	m_pAll->m_Futu.fMax = fMax;
	m_pAll->m_Futu.fMin = fMin;
	if (m_pAll->m_Futu.fMax == fMin)
		m_pAll->m_Futu.fMax = m_pAll->m_Futu.fMax * 1.1;
	if (m_pAll->m_Futu.fMax == 0)
		m_pAll->m_Futu.fMax = 1;


	int nLen = m_rcLower.bottom - m_rcLower.top - RC_MAX - RC_MIN;
	double fDiff = 0;
	fDiff = m_pAll->m_Futu.fMax - m_pAll->m_Futu.fMin;
	fDiff *= m_pAll->m_Futu.nDecimalXi;
	int nDiff = (int)fDiff;
	if (nDiff == 0)
	{
		nDiff = (int)(fDiff * m_pAll->m_Futu.nDecimalXi);
		if (nDiff == 0)
		{
			nDiff = 1;
			//	LOG_W(L"nDiff异常!");
		}
	}
	m_pAll->m_Futu.nDivY = 0;
	while (nDiff > nLen && nLen > 0)
	{
		nDiff /= 10;
		m_pAll->m_Futu.nDivY++;
	}
	m_pAll->m_Futu.nMulY = nLen / nDiff;
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
	if (!m_bSubInsisGroup)
	{
		double fPos = m_rcLower.top + (1 - (fDiff / m_pAll->m_Futu.fMax))  * (m_rcLower.Height() - 5) + RC_MAX;
		int nPos = (int)fPos;
		return nPos;
	}
	else
	{
		double fPos = m_rcLower.top + (1 - (fDiff / m_pAll->m_Futu.fMax)) / 2 * (m_rcLower.Height() - 10) + RC_MAX;
		int nPos = (int)fPos;
		return nPos;
	}
}

SStringW SKlinePic::GetFuTuYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	if (nY > m_rcLower.bottom || nY < m_rcLower.top)
		return strRet;
	if (!m_bSubInsisGroup)
	{
		double fDiff = ((double)m_rcLower.bottom - nY) / (m_rcLower.Height() - 5)  * m_pAll->m_Futu.fMax;
		strRet.Format(L"%.0f", fDiff);
		return strRet;

	}
	else
	{
		double fDiff = (double)((m_rcLower.top + m_rcLower.bottom) / 2 - nY) / (m_rcLower.Height() - 10) * 2 * m_pAll->m_Futu.fMax;
		strRet.Format(L"%.0f", fDiff);
		return strRet;
	}

}

void SOUI::SKlinePic::GetMACDMaxDiff()		//判断副图坐标最大最小值和k线条数
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

int SOUI::SKlinePic::GetMACDYPos(double fDiff)
{
	double fPos = m_rcLower2.top + (1 - (fDiff / m_pMacdData->fMax)) / 2 * (m_rcLower2.Height() - 30) + 25;
	int nPos = (int)fPos;
	return nPos;

}

SStringW SOUI::SKlinePic::GetMACDYPrice(int nY)
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
	double fPos = fDiff - m_pAll->m_Klines.fMin;
	double fPriceDiff = m_pAll->m_Klines.fMax - m_pAll->m_Klines.fMin;
	int nHeight = m_rcUpper.bottom - m_rcUpper.top - RC_MAX - RC_MIN - 20;
	fPos = m_rcUpper.bottom - fPos / fPriceDiff*nHeight + 0.5 - RC_MIN;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SKlinePic::GetYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	int nHeight = m_rcUpper.bottom - RC_MIN - m_rcUpper.top - RC_MAX - 20;
	double fPriceDiff = m_pAll->m_Klines.fMax - m_pAll->m_Klines.fMin;
	double fDiff = m_pAll->m_Klines.fMin + (double)(m_rcUpper.bottom - nY - RC_MIN) / nHeight*fPriceDiff;
	strRet.Format(m_pAll->m_Klines.sDecimal, fDiff);
	return strRet;

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
	if (point.x == m_nMouseX && point.y == m_nMouseY)
		return;

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

void SOUI::SKlinePic::OnMouseLeave()
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

	//显示点文字
	int nNum = GetXData(p.x);
	pRT->SelectObject(m_bBrushBlack);
	SStringW strTemp, strDot;
	if (nNum >= 0&&nNum<m_pAll->nTotal)
	{
		auto pda = m_pAll->m_Klines.pd[nNum];
		int nVolume = m_pAll->m_Futu.ftl[nNum];
		double fMa1 = m_pAll->m_Klines.fMa[0][nNum];
		double fMa2 = m_pAll->m_Klines.fMa[1][nNum];
		double fMa3 = m_pAll->m_Klines.fMa[2][nNum];
		double fMa4 = m_pAll->m_Klines.fMa[3][nNum];

		if (m_bShowMouseLine)
		{
			if (0 != pda.date)
			{
				pRT->FillRectangle(CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top));
				pRT->FillRectangle(CRect(m_rcImage.left + 1, m_rcImage.top + 4, m_rcImage.right, m_rcImage.top + 19));

				SStringW strName;

				if (m_bSubInsisGroup)
				{
					if (*m_pGroupDataType == 0)
						strName.Format(L"%s-价差", g_GroupInsMap[m_strSubIns].ComboIns);
					else
						strName.Format(L"%s-比值", g_GroupInsMap[m_strSubIns].ComboIns);
				}
				else
					strName.Format(L"%s", StrA2StrW(m_strSubIns));

				strDot.Format(L"%s 日期:%%d-%%02d-%%02d 时间:%%02d:%%02d:00 开:%s 高:%s 低:%s 收:%s 量:%%d",
					strName.GetBuffer(1),
					m_pAll->m_Klines.sDecimal,
					m_pAll->m_Klines.sDecimal,
					m_pAll->m_Klines.sDecimal,
					m_pAll->m_Klines.sDecimal);
				strTemp.Format(strDot, pda.date / 10000, pda.date % 10000 / 100, pda.date % 100,
					pda.time / 10000, pda.time % 10000 / 100,
					pda.open, pda.high, pda.low, pda.close, nVolume);
				DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top), strTemp);

				if (m_bShowMA)
				{
					if (nNum >= 4)
						strTemp.Format(L"MA%d:%.2f", m_nMAPara[0], fMa1);
					else
						strTemp.Format(L"MA%d:-", m_nMAPara[0]);
					DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

					if (nNum >= 9)
						strTemp.Format(L"MA%d:%.2f", m_nMAPara[1], fMa2);
					else
						strTemp.Format(L"MA%d:-", m_nMAPara[1]);
					DrawTextonPic(pRT, CRect(m_rcImage.left + 90, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

					if (nNum >= 19)
						strTemp.Format(L"MA%d:%.2f", m_nMAPara[2], fMa3);
					else
						strTemp.Format(L"MA%d:-", m_nMAPara[2]);
					DrawTextonPic(pRT, CRect(m_rcImage.left + 180, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 0, 255, 255));

					if (nNum >= 59)
						strTemp.Format(L"MA%d:%.2f", m_nMAPara[3], fMa4);
					else
						strTemp.Format(L"MA%d:-", m_nMAPara[3]);
					DrawTextonPic(pRT, CRect(m_rcImage.left + 270, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));
				}
				else if (m_bShowBandTarget)
				{
					if (m_pBandData->DataValid[nNum])
					{
						strTemp.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
						DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"多平位置:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->SellLong[nNum]);

						DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(100, 100, 100, 255));

						strTemp.Format(L"空平位置:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->BuyShort[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 110 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"上轨:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->UpperTrack1[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 220 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->LowerTrack1[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 310 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"上轨K2:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->UpperTrack2[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 400 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨K2:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->LowerTrack2[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 500 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"状态:%d", m_pBandData->Status[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 600 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"仓位:%.2f", m_pBandData->Position[nNum]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 640 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));


					}
					else
					{
						strTemp.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
						DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"多平位置:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(100, 100, 100, 255));

						strTemp.Format(L"空平位置:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 60 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"上轨:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 120 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 180 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"上轨K2:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 240 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨K2:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 300 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"状态:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 360 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"仓位:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 420 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

					}

				}

				if (m_bShowMacd)
				{
					pRT->FillRectangle(CRect(m_rcLower2.left + 1, m_rcLower2.top + 4, m_rcLower2.right, m_rcLower2.top + 20));

					if ((m_pAll->nInstype&NationalDebt) == 0)
					{
						strTemp.Format(L"MACD(%d,%d,%d) DIF:%.2f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
							strTemp, RGBA(255, 255, 255, 255));
						strTemp.Format(L"DEA:%.2f", m_pMacdData->DEA[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
							strTemp, RGBA(255, 255, 0, 255));
						strTemp.Format(L"MACD:%.2f", m_pMacdData->MACD[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
							strTemp, RGBA(255, 0, 255, 255));

					}
					else
					{
						strTemp.Format(L"MACD(%d,%d,%d) DIF:%.3f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
							strTemp, RGBA(255, 255, 255, 255));
						strTemp.Format(L"DEA:%.3f", m_pMacdData->DEA[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
							strTemp, RGBA(255, 255, 0, 255));
						strTemp.Format(L"MACD:%.3f", m_pMacdData->MACD[nNum]);
						DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
							strTemp, RGBA(255, 0, 255, 255));

					}
				}



			}

		}
		if (0 != pda.date && !bFromOnPaint&&ptIsInKlineRect(p, nNum - m_nFirst, pda))
		{
			if (nNum > 0)
				DrawMouseKlineInfo(pRT, pda, p, nNum, m_pAll->m_Klines.pd[nNum - 1].close);
			else
				DrawMouseKlineInfo(pRT, pda, p);
		}

	}


	if (m_bShowMouseLine)
	{

		HDC hdc = pRT->GetDC();
		//	HPEN pen, oldPen;
		//	pen = CreatePen(PS_SOLID, 1, RGBA(255, 255, 255, 0xFF));
		//	oldPen = (HPEN)SelectObject(hdc, pen);
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
	}
	//显示横坐标所在数值

	pRT->SelectObject(m_bBrushBlack);
	//	pRT->FillRectangle(CRect(m_rcUpper.right + 1, m_rcUpper .top, m_rcUpper.right + RC_RIGHT, m_rcLower.bottom));

	SStringW s1;
	s1.Empty();
	if (m_nMouseY >= m_rcUpper.top && m_nMouseY <= m_rcUpper.bottom)	//十字在k线上
		s1 = GetYPrice(m_nMouseY);
	else if (m_nMouseY >= m_rcLower.top && m_nMouseY <= m_rcLower.bottom)	//十字在附图上
		s1 = GetFuTuYPrice(m_nMouseY);
	//	LOG_W(s1.GetBuffer(1));
	if (!s1.IsEmpty())
		DrawTextonPic(pRT, CRect(m_rcUpper.right + 2, m_nMouseY + -6, m_rcUpper.right + RC_RIGHT, m_nMouseY + 15), s1);


	pRT->FillRectangle(CRect(m_rcUpper.right + 1, m_rcUpper.top, m_rcUpper.right + RC_RIGHT, m_rcLower.bottom + RC_BOTTOM - 10));

	if (p.y >= m_rcUpper.top && p.y <= m_rcUpper.bottom)	//十字在k线上
		s1 = GetYPrice(p.y);
	else if (p.y >= m_rcLower.top && p.y <= m_rcLower.bottom)	//十字在附图上
		s1 = GetFuTuYPrice(p.y);
	//	LOG_W(s1.GetBuffer(1));
	if (!s1.IsEmpty())
		DrawTextonPic(pRT, CRect(m_rcUpper.right + 2, p.y - 6, m_rcUpper.right + RC_RIGHT, p.y + 15), s1);
	//	pRT->FillRectangle(CRect(m_rcUpper.right + 1, m_nMouseY + 2, m_rcUpper.right + RC_RIGHT, m_nMouseY + 22));


		/*
		//第二个k线的的鼠标显示
		if (m_pAll->m_Klines[1].bShow)
		{
		s1 = GetYPrice(p.y, 1);
		TextOut(hdc, m_rcUpper.right - 25, p.y - 12, s1, -1);
		}*/

		//显示纵坐标数值

	DrawTime(pRT, m_nKlineType);

	if (IsInRect(m_nMouseX, m_nMouseY, 0))
	{
		int nx = GetXData(m_nMouseX);
		int date = m_pAll->m_Klines.pd[nx].date;
		int time = m_pAll->m_Klines.pd[nx].time;
		if (nx >= 0 && date > 0)
		{
			s1.Format(L"%d-%02d-%02d  %02d:%02d:00", date / 10000,
				date % 10000 / 100,
				date % 100,
				time / 10000,
				time % 10000 / 100);
			pRT->FillRectangle(CRect(m_nMouseX - 40, m_rcImage.bottom + 2, m_nMouseX + 40, m_rcImage.bottom + 34));
			//			DrawTextonPic(pRT, CRect(m_nMouseX, m_rcImage.bottom + 2, m_nMouseX + 100, m_rcImage.bottom + 35), s1);
			//			SetROP2(hdc, nMode);
			//			InvalidateRect(CRect(m_rcImage.left - 38, m_rcImage.bottom + 2, m_rcImage.right + 38, m_rcImage.bottom + 35));

		}
	}

	DrawTime(pRT, m_nKlineType);

	if (IsInRect(p.x, p.y, 0))
	{
		int nx = GetXData(p.x);
		int date = m_pAll->m_Klines.pd[nx].date;
		int time = m_pAll->m_Klines.pd[nx].time;
		int nTotal = m_pAll->nTotal;

		if (nx >= 0 && nx < nTotal && date>0)
		{
			s1.Format(L"%d-%02d-%02d  %02d:%02d:00", date / 10000,
				date % 10000 / 100,
				date % 100,
				time / 10000,
				time % 10000 / 100);
			pRT->FillRectangle(CRect(p.x - 40, m_rcImage.bottom + 2, p.x + 40, m_rcImage.bottom + 34));
			DrawTextonPic(pRT, CRect(p.x - 35, m_rcImage.bottom + 2, p.x + +35, m_rcImage.bottom + 35), s1, RGBA(255, 255, 255, 255), 0);
			//			SetROP2(hdc, nMode);
			//			InvalidateRect(CRect(m_rcImage.left - 38, m_rcImage.bottom + 2, m_rcImage.right + 38, m_rcImage.bottom + 35));

		}
	}
	m_nMouseX = p.x;
	m_nMouseY = p.y;
	m_nKeyX = p.x;
	m_nKeyY = p.y;

}

void SKlinePic::DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint) //画竖线时间轴时间和标示数字
{
	int nJiange = 2;
	if (m_bNoJiange)
		nJiange = 0;

	int nXpre = 0;  //第一根竖线的x轴位置
	int nMaX = (m_nEnd - m_nFirst - 1)*(m_nKWidth + nJiange) + RC_LEFT + 1 + (m_nKWidth / 2);	//最后一个数据的位置
	DATA_FOR_SHOW *p = m_pAll->m_Klines.pd + m_nFirst;

	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;
	pRT->SelectObject(m_penDotRed, (IRenderObj**)&oldPen);
	CPoint pts[5];
	HDC pdc = pRT->GetDC();

	int nPosMx[20] = { 0 };
	int nPosCount = 1;

	bool bHasNight = (m_pAll->nInstype&ComNightEnd2300) || (m_pAll->nInstype&ComNightEnd100) || (m_pAll->nInstype&ComNightEnd230);

	int nNowDay = m_pAll->m_Klines.pd[m_nFirst].date;
	int nNowTime = m_pAll->m_Klines.pd[m_nFirst].time;

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
			strTime.Format(L"%02d:%02d:%02d", m_pAll->m_Klines.pd[i + m_nFirst].time / 10000,
				m_pAll->m_Klines.pd[i + m_nFirst].time % 10000 / 100,
				m_pAll->m_Klines.pd[i + m_nFirst].time % 100);
			//加日期
			strDate.Format(L"%04d-%02d-%02d", m_pAll->m_Klines.pd[i + m_nFirst].date / 10000,
				m_pAll->m_Klines.pd[i + m_nFirst].date % 10000 / 100,
				m_pAll->m_Klines.pd[i + m_nFirst].date % 100);

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
		//if (x - preX > 80)
		//{
		//	nPosMx[nPosCount] = i;
		//	nPosCount++;
		//}
		//else
		//{
		//	if (bHasNight)
		//	{
		//		if (nNowTime >= 90000 && nNowTime < 151500)
		//		{
		//			if (m_pAll->m_Klines.pd[m_nFirst + i].time >= 210000 || m_pAll->m_Klines.pd[m_nFirst + i].time < 23000
		//				|| (m_pAll->m_Klines.pd[m_nFirst + i].date != nNowDay&&
		//					m_pAll->m_Klines.pd[m_nFirst + i].time >= 90000 &&
		//					m_pAll->m_Klines.pd[m_nFirst + i].time < 151500))
		//			{
		//				nNowDay = m_pAll->m_Klines.pd[m_nFirst + i].date;
		//				nNowTime = m_pAll->m_Klines.pd[m_nFirst + i].time;
		//				nPosMx[nPosCount - 1] = i;
		//			}
		//		}
		//		else
		//		{
		//			if(m_pAll->m_Klines.pd[m_nFirst + i].date != nNowDay)
		//		}
		//	}
		//	else
		//	{
		//		if (m_pAll->m_Klines.pd[m_nFirst + i].date != nNowDay)
		//		{
		//			nPosMx[nPosCount - 1] = i;
		//			nNowDay = m_pAll->m_Klines.pd[m_nFirst + i].date;

		//		}
		//	}
		//}


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

	DATA_FOR_SHOW *p = m_pAll->m_Klines.pd + m_nFirst;

	CPoint *UpperLine1 = new CPoint[m_nEnd - m_nFirst];
	CPoint *UpperLine2 = new CPoint[m_nEnd - m_nFirst];
	CPoint *LowerLine1 = new CPoint[m_nEnd - m_nFirst];
	CPoint *LowerLine2 = new CPoint[m_nEnd - m_nFirst];
	CPoint *SellLongLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *BuyShortLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *DIFLine = new CPoint[m_nEnd - m_nFirst];
	CPoint *DEALine = new CPoint[m_nEnd - m_nFirst];
	CPoint *Ma5Line = new CPoint[m_nEnd - m_nFirst];
	CPoint *Ma10Line = new CPoint[m_nEnd - m_nFirst];
	CPoint *Ma20Line = new CPoint[m_nEnd - m_nFirst];
	CPoint *Ma60Line = new CPoint[m_nEnd - m_nFirst];

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		x = i * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
		auto data = m_pAll->m_Klines.pd[i + m_nFirst];
		int nVolume = m_pAll->m_Futu.ftl[m_nFirst + i];
		int nVolume1;
		if (m_nFirst + i > 0)
			nVolume1 = m_pAll->m_Futu.ftl[m_nFirst + i - 1];
		double fMa1 = m_pAll->m_Klines.fMa[0][i + m_nFirst];
		double fMa2 = m_pAll->m_Klines.fMa[1][i + m_nFirst];
		double fMa3 = m_pAll->m_Klines.fMa[2][i + m_nFirst];
		double fMa4 = m_pAll->m_Klines.fMa[3][i + m_nFirst];


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
					strTemp.Format(m_pAll->m_Klines.sDecimal, data.close);
					//	pRT->TextOut(x + 2, yclose - 5, strTemp,-1);
					//				if (dk == 0)
					pRT->TextOut(x + m_nKWidth + 2, yclose - 5, strTemp, -1);
					//		pRT->TextOut(m_rcUpper.left - RC_LEFT + 8, m_rcUpper.top - RC_TOP + 10, strTemp, -1);

					CPoint pt;
					GetCursorPos(&pt);
					if (!m_bShowMouseLine || (pt.x > m_rcImage.right || pt.x<m_rcImage.left || pt.y>m_rcImage.bottom || pt.y < m_rcImage.top))
					{
						SStringW strDot;

						SStringW strName;

						if (m_bSubInsisGroup)
						{
							if (*m_pGroupDataType == 0)
								strName.Format(L"%s-价差", g_GroupInsMap[m_strSubIns].ComboIns);
							else
								strName.Format(L"%s-比值", g_GroupInsMap[m_strSubIns].ComboIns);
						}
						else
							strName.Format(L"%s", StrA2StrW(m_strSubIns));

						strDot.Format(L"%s 日期:%%d-%%02d-%%02d 时间:%%02d:%%02d:00 开:%s 高:%s 低:%s 收:%s 量:%%d",
							strName.GetBuffer(1),
							m_pAll->m_Klines.sDecimal,
							m_pAll->m_Klines.sDecimal,
							m_pAll->m_Klines.sDecimal,
							m_pAll->m_Klines.sDecimal);

						strTemp.Format(strDot, data.date / 10000, data.date % 10000 / 100, data.date % 100,
							data.time / 10000, data.time % 10000 / 100,
							data.open, data.high, data.low, data.close, nVolume);
						DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top), strTemp);
						if (m_bShowMA)
						{
							if (m_nFirst + i >= 4)
								strTemp.Format(L"MA%d:%.2f", m_nMAPara[0], fMa1);
							else
								strTemp.Format(L"MA%d:-", m_nMAPara[0]);
							DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

							if (m_nFirst + i >= 9)
								strTemp.Format(L"MA%d:%.2f", m_nMAPara[1], fMa2);
							else
								strTemp.Format(L"MA%d:-", m_nMAPara[1]);
							DrawTextonPic(pRT, CRect(m_rcImage.left + 90, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

							if (m_nFirst + i >= 19)
								strTemp.Format(L"MA%d:%.2f", m_nMAPara[2], fMa3);
							else
								strTemp.Format(L"MA%d:-", m_nMAPara[2]);
							DrawTextonPic(pRT, CRect(m_rcImage.left + 180, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 0, 255, 255));

							if (m_nFirst + i >= 59)
								strTemp.Format(L"MA%d:%.2f", m_nMAPara[3], fMa4);
							else
								strTemp.Format(L"MA%d:-", m_nMAPara[3]);
							DrawTextonPic(pRT, CRect(m_rcImage.left + 270, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));
						}
						if (m_bShowMacd)
						{
							if ((m_pAll->nInstype&NationalDebt) == 0)
							{
								strTemp.Format(L"MACD(%d,%d,%d) DIF:%.2f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
									strTemp, RGBA(255, 255, 255, 255));
								strTemp.Format(L"DEA:%.2f", m_pMacdData->DEA[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
									strTemp, RGBA(255, 255, 0, 255));
								strTemp.Format(L"MACD:%.2f", m_pMacdData->MACD[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
									strTemp, RGBA(255, 0, 255, 255));

							}
							else
							{
								strTemp.Format(L"MACD(%d,%d,%d) DIF:%.3f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
									strTemp, RGBA(255, 255, 255, 255));
								strTemp.Format(L"DEA:%.3f", m_pMacdData->DEA[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
									strTemp, RGBA(255, 255, 0, 255));
								strTemp.Format(L"MACD:%.3f", m_pMacdData->MACD[i + m_nFirst]);
								DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
									strTemp, RGBA(255, 0, 255, 255));

							}
						}

					}

				}

			}
			if (m_pAll->nLineType == 1)
			{
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
				if (m_bShowHighLow)
				{
					pRT->DrawLines(pts, 2);
					pRT->DrawLines(pts + 2, 2);
				}
				if (data.close == data.open)
				{
					pts[0].SetPoint(x + 2, yopen);
					pts[1].SetPoint(x + m_nKWidth - 1, yopen);
					pRT->DrawLines(pts, 2);
				}
				else
				{
					if (data.close >= data.open)
					{
						pRT->DrawRectangle(CRect(x, yopen, x + m_nKWidth, yclose));
						//				pRT->SetPixel(x, yopen, RGBA(255, 255, 255, 255));
					}
					else
					{
						//if (m_nKWidth > 2)
						pRT->FillSolidRect(CRect(x, yclose, x + m_nKWidth, yopen==yclose?(yopen-1):yopen), RGBA(0, 255, 255, 255));
						//else
						//	pRT->FillSolidRect(CRect(x, yopen, x + m_nKWidth, yclose + 1), RGBA(0, 255, 255, 255));
					}
				}

			}

			if (m_bShowMA)
			{

				if (i + m_nFirst >= 4)
					Ma5Line[i].SetPoint(x + m_nKWidth / 2, GetYPos(fMa1));

				//MA10
				if (i + m_nFirst >= 9)
					Ma10Line[i].SetPoint(x + m_nKWidth / 2, GetYPos(fMa2));

				//MA20
				if (i + m_nFirst >= 19)
					Ma20Line[i].SetPoint(x + m_nKWidth / 2, GetYPos(fMa3));


				//MA60
				if (i + m_nFirst >= 59)
					Ma60Line[i].SetPoint(x + m_nKWidth / 2, GetYPos(fMa4));
			}

		}
		else
		{
			if (m_pBandData->DataValid[i + m_nFirst])
			{
				//画轨道
				int yUpperTrack1 = GetYPos(m_pBandData->UpperTrack1[i + m_nFirst]);
				int yUpperTrack2 = GetYPos(m_pBandData->UpperTrack2[i + m_nFirst]);
				int ySellLong = GetYPos(m_pBandData->SellLong[i + m_nFirst]);
				int yBuyShort = GetYPos(m_pBandData->BuyShort[i + m_nFirst]);
				int yLowerTrack1 = GetYPos(m_pBandData->LowerTrack1[i + m_nFirst]);
				int yLowerTrack2 = GetYPos(m_pBandData->LowerTrack2[i + m_nFirst]);

				if (nValidNum == -1 && m_pBandData->DataValid[i + m_nFirst])
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

				if (!m_bSubInsisGroup)
				{
					if (m_pAll->m_Futu.bft == 1 && m_nFirst + i > 0)
					{
						pRT->SelectObject(m_penWhite);
						pts[0].SetPoint(x + m_nKWidth / 2 - m_nKWidth, GetFuTuYPos(nVolume1));
						pts[1].SetPoint(x + m_nKWidth / 2, GetFuTuYPos(nVolume));
						pRT->DrawLines(pts, 2);
					}
					else if (m_pAll->m_Futu.bft == 2)
					{
						pRT->SelectObject(m_penRed);

						if (data.close > data.open)
							pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom));
						else if (data.close == data.open&&m_nFirst + i > 0)
						{
							if (data.close >= p[i - 1].close)
							{
								pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom));
							}
							else
							{
								//if (m_nKWidth > 2)
								pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, m_rcLower.bottom), RGBA(0, 255, 255, 255));
								//else
								//	pRT->FillRectangle(CRect(x + nJianGe, GetFuTuYPos(nVolume), x + m_nKWidth + nJianGe, m_rcLower.bottom));
							}
						}
						else
						{
							//if (m_nKWidth > 2)
							pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth,  m_rcLower.bottom), RGBA(0, 255, 255, 255));
							//else
							//	pRT->FillRectangle(CRect(x + nJianGe, GetFuTuYPos(nVolume), x + m_nKWidth + nJianGe, m_rcLower.bottom));
						}
					}
					else if (m_pAll->m_Futu.bft == 3)
					{

					}
				}
				else
				{
					if (m_pAll->m_Futu.bft == 1 && m_nFirst + i > 0)
					{
						pRT->SelectObject(m_penWhite);
						pts[0].SetPoint(x + m_nKWidth / 2 - m_nKWidth, GetFuTuYPos(nVolume1));
						pts[1].SetPoint(x + m_nKWidth / 2, GetFuTuYPos(nVolume));
						pRT->DrawLines(pts, 2);
					}
					else if (m_pAll->m_Futu.bft == 2)
					{
						pRT->SelectObject(m_penRed);
						int VPos = GetFuTuYPos(nVolume);
						int nBottom = (m_rcLower.bottom + m_rcLower.top) / 2;
						if (VPos == nBottom)
						{
							if (nVolume > 0)
								VPos--;
							else if (nVolume < 0)
								VPos++;
						}

						if (GetFuTuYPos(nVolume) >= (m_rcLower.bottom + m_rcLower.top) / 2)
						{
							if (data.close > data.open)
								pRT->DrawRectangle(CRect(x,VPos<nBottom?VPos: nBottom, x + m_nKWidth, VPos>nBottom ? VPos : nBottom));
							else if (data.close == data.open)
							{
								if (data.close >= p[i - 1].close)
								{
									pRT->SelectObject(m_penRed);
									pRT->DrawRectangle(CRect(x, VPos<nBottom ? VPos : nBottom, x + m_nKWidth, VPos>nBottom ? VPos : nBottom));
								}
								else
								{
									//if (m_nKWidth > 2)
									pRT->FillSolidRect(CRect(x, VPos<nBottom ? VPos : nBottom, x + m_nKWidth, VPos>nBottom ? VPos : nBottom), RGBA(0, 255, 255, 255));
									//else
									//	pRT->FillRectangle(CRect(x + nJianGe, (m_rcLower.bottom + m_rcLower.top) / 2, x + m_nKWidth + nJianGe, GetFuTuYPos(nVolume)));
								}
							}
							else
							{
								//if (m_nKWidth > 2)
								pRT->FillSolidRect(CRect(x, VPos<nBottom ? VPos : nBottom, x + m_nKWidth, VPos>nBottom ? VPos : nBottom), RGBA(0, 255, 255, 255));
								//else
								//	pRT->FillRectangle(CRect(x + nJianGe, (m_rcLower.bottom + m_rcLower.top) / 2, x + m_nKWidth + nJianGe, GetFuTuYPos(nVolume)));
							}

						}
						else
						{
							if (data.close > data.open)
								pRT->DrawRectangle(CRect(x, (m_rcLower.bottom + m_rcLower.top) / 2, x + m_nKWidth, GetFuTuYPos(nVolume)));
							else if (data.close == data.open&&m_nFirst + i > 0)
							{
								if (data.close >= p[i - 1].close)
								{
									pRT->DrawRectangle(CRect(x, (m_rcLower.bottom + m_rcLower.top) / 2, x + m_nKWidth, GetFuTuYPos(nVolume)));
								}
								else
								{
									//if (m_nKWidth > 2)
									pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, (m_rcLower.bottom + m_rcLower.top) / 2), RGBA(0, 255, 255, 255));
									//else
									//	pRT->FillRectangle(CRect(x + nJianGe, GetFuTuYPos(nVolume), x + m_nKWidth + nJianGe, (m_rcLower.bottom + m_rcLower.top) / 2));

								}
							}
							else
							{
								//if (m_nKWidth > 2)
								pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume), x + m_nKWidth, (m_rcLower.bottom + m_rcLower.top) / 2), RGBA(0, 255, 255, 255));
								//else
								//	pRT->FillRectangle(CRect(x + nJianGe, GetFuTuYPos(nVolume), x + m_nKWidth + nJianGe, (m_rcLower.bottom + m_rcLower.top) / 2));

							}

						}
					}
					else if (m_pAll->m_Futu.bft == 3)
					{

					}
				}
			}
		}

		if (m_bShowMacd)
		{
			double yDIF = GetMACDYPos(m_pMacdData->DIF[i + m_nFirst]);
			double yDEA = GetMACDYPos(m_pMacdData->DEA[i + m_nFirst]);


			DIFLine[i].SetPoint(x + m_nKWidth / 2, yDIF);

			DEALine[i].SetPoint(x + m_nKWidth / 2, yDEA);


			int nWidthMacd = (m_rcLower2.Height() - 20) / 4;
			//MACD柱状图
			if (m_pMacdData->MACD[i + m_nFirst] != 0)
			{
				pts[0].SetPoint(x + m_nKWidth / 2, m_rcLower2.top + 20 + 2 * nWidthMacd);
				pts[1].SetPoint(x + m_nKWidth / 2, GetMACDYPos(m_pMacdData->MACD[i + m_nFirst]));

				if (m_pMacdData->MACD[i + m_nFirst] > 0)
					pRT->SelectObject(m_penRed);
				else
					pRT->SelectObject(m_penGreen);
				pRT->DrawLines(pts, 2);
			}

		}

	}

	//	::LeaveCriticalSection(&m_cs);

	if (m_bShowMA)
	{
		if (m_nFirst >= 4)
		{
			pRT->SelectObject(m_penWhite);
			pRT->DrawLines(Ma5Line, m_nEnd - m_nFirst);
		}
		else if (m_nEnd > 4)
		{
			pRT->SelectObject(m_penWhite);
			pRT->DrawLines(Ma5Line - m_nFirst + 4, m_nEnd - 4);

		}
		if (m_nFirst >= 9)
		{
			pRT->SelectObject(m_penYellow);
			pRT->DrawLines(Ma10Line, m_nEnd - m_nFirst);
		}
		else if (m_nEnd > 9)

		{
			pRT->SelectObject(m_penYellow);
			pRT->DrawLines(Ma10Line - m_nFirst + 9, m_nEnd - 9);

		}


		if (m_nFirst >= 19)
		{
			pRT->SelectObject(m_penPurple);
			pRT->DrawLines(Ma20Line, m_nEnd - m_nFirst);
		}
		else if (m_nEnd > 19)
		{
			pRT->SelectObject(m_penPurple);
			pRT->DrawLines(Ma20Line - m_nFirst + 19, m_nEnd - 19);

		}


		if (m_nFirst >= 59)
		{
			pRT->SelectObject(m_penMAGreen);
			pRT->DrawLines(Ma60Line, m_nEnd - m_nFirst);
		}
		else if (m_nEnd > 59)

		{
			pRT->SelectObject(m_penMAGreen);
			pRT->DrawLines(Ma60Line - m_nFirst + 59, m_nEnd - 59);

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
			auto data = m_pAll->m_Klines.pd[i + m_nFirst];
			x = i * (m_nKWidth + nJianGe) + 1 + m_rcUpper.left;
			DrawBandData(pRT, i + m_nFirst, data, x, nJianGe);
			if (i == m_nEnd - m_nFirst - 1)
			{
				int nVolume = m_pAll->m_Futu.ftl[m_nFirst + i];
				CPoint pt;
				GetCursorPos(&pt);
				if (!m_bShowMouseLine || (pt.x > m_rcImage.right || pt.x<m_rcImage.left || pt.y>m_rcImage.bottom || pt.y < m_rcImage.top))
				{
					SStringW strDot, strTemp;
					SStringW strName;

					if (m_bSubInsisGroup)
					{
						if (*m_pGroupDataType == 0)
							strName.Format(L"%s-价差", g_GroupInsMap[m_strSubIns].ComboIns);
						else
							strName.Format(L"%s-比值", g_GroupInsMap[m_strSubIns].ComboIns);
					}
					else
						strName.Format(L"%s", StrA2StrW(m_strSubIns));

					strDot.Format(L"%s 日期:%%d-%%02d-%%02d 时间:%%02d:%%02d:00 开:%s 高:%s 低:%s 收:%s 量:%%d",
						strName.GetBuffer(1),
						m_pAll->m_Klines.sDecimal,
						m_pAll->m_Klines.sDecimal,
						m_pAll->m_Klines.sDecimal,
						m_pAll->m_Klines.sDecimal);
					strTemp.Format(strDot, data.date / 10000, data.date % 10000 / 100, data.date % 100,
						data.time / 10000, data.time % 10000 / 100,
						data.open, data.high, data.low, data.close, nVolume);
					DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top), strTemp);
					if (m_pBandData->DataValid[i + m_nFirst])
					{
						strTemp.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
						DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"多平位置:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->SellLong[i + m_nFirst]);

						DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(100, 100, 100, 255));

						strTemp.Format(L"空平位置:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->BuyShort[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 110 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"上轨:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->UpperTrack1[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 220 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->LowerTrack1[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 310 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"上轨K2:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->UpperTrack2[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 400 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨K2:%s", m_pAll->m_Klines.sDecimal);
						strTemp.Format(strTemp.GetBuffer(1), m_pBandData->LowerTrack2[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 500 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"状态:%d", m_pBandData->Status[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 600 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"仓位:%.2f", m_pBandData->Position[i + m_nFirst]);
						DrawTextonPic(pRT, CRect(m_rcImage.left + 640 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

					}
					else
					{
						strTemp.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
						DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"多平位置:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(100, 100, 100, 255));

						strTemp.Format(L"空平位置:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 60 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"上轨:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 120 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 180 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"上轨K2:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 240 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(0, 255, 0, 255));

						strTemp.Format(L"下轨K2:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 300 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

						strTemp.Format(L"状态:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 360 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp);

						strTemp.Format(L"仓位:-");
						DrawTextonPic(pRT, CRect(m_rcImage.left + 420 + 150, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strTemp, RGBA(255, 255, 0, 255));

					}
					if (m_bShowMacd)
					{
						if ((m_pAll->nInstype&NationalDebt) == 0)
						{
							strTemp.Format(L"MACD(%d,%d,%d) DIF:%.2f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
								strTemp, RGBA(255, 255, 255, 255));
							strTemp.Format(L"DEA:%.2f", m_pMacdData->DEA[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
								strTemp, RGBA(255, 255, 0, 255));
							strTemp.Format(L"MACD:%.2f", m_pMacdData->MACD[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
								strTemp, RGBA(255, 0, 255, 255));

						}
						else
						{
							strTemp.Format(L"MACD(%d,%d,%d) DIF:%.3f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 5, m_rcLower2.top + 5, m_rcLower2.left + 160, m_rcLower2.top + 20),
								strTemp, RGBA(255, 255, 255, 255));
							strTemp.Format(L"DEA:%.3f", m_pMacdData->DEA[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 160, m_rcLower2.top + 5, (m_rcLower2.left + 240 > m_rcLower2.right ? m_rcLower2.right : m_rcLower2.left + 240), m_rcLower2.top + 20),
								strTemp, RGBA(255, 255, 0, 255));
							strTemp.Format(L"MACD:%.3f", m_pMacdData->MACD[i + m_nFirst]);
							DrawTextonPic(pRT, CRect(m_rcLower2.left + 240, m_rcLower2.top + 5, m_rcLower2.right, m_rcLower2.top + 20),
								strTemp, RGBA(255, 0, 255, 255));

						}
					}

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

	UpperLine1 = nullptr;
	UpperLine2 = nullptr;
	LowerLine1 = nullptr;
	LowerLine2 = nullptr;
	SellLongLine = nullptr;
	BuyShortLine = nullptr;
	DIFLine = nullptr;
	DEALine = nullptr;


	if (m_pAll->nLineType == 3)
	{
		DrawTickMainData(pRT);
		DrawVolData(pRT);
	}

	LONGLONG llTmp2 = GetTickCount64();

}


void SOUI::SKlinePic::DrawBandData(IRenderTarget * pRT, int nDataPos, DATA_FOR_SHOW &data, int x, int nJiange)
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

void SOUI::SKlinePic::DrawMouseKlineInfo(IRenderTarget * pRT, const DATA_FOR_SHOW  &KlData, CPoint pt, const int &num, const double &fPrePrice)
{
	CRect rc{ pt.x,pt.y,pt.x + 100,pt.y + 150 };

	SStringW strFormat;
	strFormat.Format(L"%%s:%s", m_pAll->m_Klines.sDecimal);
	SStringW tmp;
	if (num != 0)
	{
		tmp.Format(L"%%02d-%%02d-%%02d %%02d:%%02d\n%s\n%s\n%s\n%s\n%s\n涨跌幅:%%.2f%%s\n", strFormat, strFormat, strFormat, strFormat, strFormat);
		tmp.Format(tmp, (KlData.date / 10000) % 100, (KlData.date / 100) % 100, KlData.date % 100, KlData.time / 10000, (KlData.time / 100) % 100,
			L"开盘", KlData.open,
			L"最高", KlData.high,
			L"最低", KlData.low,
			L"收盘", KlData.close,
			L"涨跌", KlData.close - fPrePrice,
			(KlData.close - fPrePrice) / fPrePrice * 100, L"%");
	}
	else
	{
		tmp.Format(L"%%02d-%%02d-%%02d %%02d:%%02d\n%s\n%s\n%s\n%s\n涨跌:-\n涨跌幅:-\n", strFormat, strFormat, strFormat, strFormat);
		tmp.Format(tmp, (KlData.date / 10000) % 100, (KlData.date / 100) % 100, KlData.date % 100, KlData.time / 10000, (KlData.time / 100) % 100,
			L"开盘", KlData.open,
			L"最高", KlData.high,
			L"最低", KlData.low,
			L"收盘", KlData.close);
	}
	m_pTip->UpdateTip(rc, tmp, GetScale());
	m_pTip->RelayEvent(pt);
}

void SOUI::SKlinePic::DataProc()
{
	bPaintAfetrDataProc = false;
	if (m_bSubInsisGroup)
		GroupDataProc();
	else
		SingleDataProc();
	m_nBandCount = 0;
	m_nMacdCount = 0;

	UpdateData();
	m_bDataInited = true;

	TraceLog("K线数据处理完成");

}

void SOUI::SKlinePic::ReProcMAData()
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

	DATA_FOR_SHOW *p = m_pAll->m_Klines.pd + m_nFirst;
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
			strTemp.Format(m_pAll->m_Klines.sDecimal, p[i].close);
			//	pRT->TextOut(x + 2, yclose - 5, strTemp,-1);
			pRT->TextOut(x + m_nKWidth + 2, yclose - 5, strTemp, -1);
			//		pRT->TextOut(m_rcUpper.left - RC_LEFT + 8, m_rcUpper.top - RC_TOP + 10, strTemp, -1);
		}
	}
	pRT->DrawLines(pts, nPNum);
	pRT->SelectObject(oldPen);
	pRT->SelectObject(bOldBrush);

	LONGLONG llTmp2 = GetTickCount64();
	//	LOG_W(L"tick:%I64d,总tick:%I64d\n", llTmp2 - llTmp1, llTmp2 - llTmp1);

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

	DATA_FOR_SHOW *p = m_pAll->m_Klines.pd + m_nFirst;

	//	for (int nt = 0; nt < MAX_FLINE_COUNT; nt++)
	//	{
	//		if (!m_pAll->m_Futu.bft[nt])
	//			continue;
	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		x = i * m_nKWidth + 1 + m_rcUpper.left;
		if (m_nKWidth > 2)
		{
			pts[nPNum++].SetPoint(x + m_nKWidth / 2 - 1, m_rcLower.bottom);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2 - 1, GetFuTuYPos(m_pAll->m_Futu.ftl[m_nFirst + i - 1]));
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, GetFuTuYPos(m_pAll->m_Futu.ftl[m_nFirst + i - 1]));
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, m_rcLower.bottom);
		}
		else
		{
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, m_rcLower.bottom);
			pts[nPNum++].SetPoint(x + m_nKWidth / 2, GetFuTuYPos(m_pAll->m_Futu.ftl[m_nFirst + i - 1]));
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
	//	LONGLONG llTmp2 = GetTickCount64();
	//	LOG_W(L"fu:%I64d\n", llTmp2 - llTmp1, llTmp2 - llTmp1);

}

void SKlinePic::OnDbClickedKline(UINT nFlags, CPoint point)
{
	m_bShowMouseLine = !m_bShowMouseLine;
	Invalidate();
}

void SKlinePic::OnKeyDown(UINT nChar)
{
	switch (nChar)
	{
	case VK_LEFT:
	{
		if (m_nFirst != 0)
			m_nMove++;
		Invalidate();
	}
	break;
	case VK_RIGHT:
	{
		m_nMove--;
		if (m_nMove < 0)
			m_nMove = 0;
		Invalidate();
	}
	break;
	default:
		break;
	}
}

void SKlinePic::DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar)
{

	//画鼠标线
	int nx = GetXData(m_nKeyX);
	if (nx > m_pAll->nTotal)
		nx = m_pAll->nTotal;
	CPoint po;
	po.y = GetYPos(_wtof(GetYPrice(nx).GetBuffer(1)));
	nx -= m_nFirst;
	po.x = GetXPos(nx);

	HDC hdc = pRT->GetDC();
	//	HPEN pen, oldPen;
	//	pen = CreatePen(PS_SOLID, 1, RGBA(255, 255, 255, 0xFF));
	//	oldPen = (HPEN)SelectObject(hdc, pen);
	CRect rcClient;
	GetClientRect(rcClient);
	int  nMode = SetROP2(hdc, R2_NOT);
	MoveToEx(hdc, po.x, m_rcImage.top + 19, NULL);			LineTo(hdc, po.x, m_rcImage.bottom);
	MoveToEx(hdc, m_rcImage.left, po.y, NULL);		LineTo(hdc, m_rcUpper.right, po.y);

	//	SelectObject(hdc, oldPen);
	SetROP2(hdc, nMode);
	pRT->ReleaseDC(hdc);


}

double SOUI::SKlinePic::GetHighPrice(int n, int nPeriod, int nOffset)
{
	int nSize = m_pAll->nTotal;
	if (nSize <= 0 || nSize < n + 1 || n - nOffset < 0)
		return DATA_ERROR;
	double fHigh = m_pAll->m_Klines.pd[n - nOffset].high;
	for (int i = 0; i < nPeriod; i++)
	{
		int nPos = n - nOffset - i;
		if (nPos < 0)
			break;
		if (fHigh < m_pAll->m_Klines.pd[nPos].high)
			fHigh = m_pAll->m_Klines.pd[nPos].high;
	}
	return fHigh;
}

double SOUI::SKlinePic::GetLowPrice(int n, int nPeriod, int nOffset)
{
	int nSize = m_pAll->nTotal;
	if (nSize <= 0 || nSize < n + 1 || n - nOffset < 0)
		return DATA_ERROR;
	double fLow = m_pAll->m_Klines.pd[n - nOffset].low;
	for (int i = 0; i < nPeriod; i++)
	{
		int nPos = n - nOffset - i;
		if (nPos < 0)
			break;
		if (fLow > m_pAll->m_Klines.pd[nPos].low)
			fLow = m_pAll->m_Klines.pd[nPos].low;
	}
	return fLow;
}

int SOUI::SKlinePic::Count(double a[], double b[], int nType, int n, int nPeriod)
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

bool SOUI::SKlinePic::Cross(double a[], double b[], int nPos)
{
	if (nPos > 0)
	{
		if (a[nPos] > b[nPos] && a[nPos - 1] < b[nPos - 1])
			return true;
	}

	return false;
}


int SOUI::SKlinePic::ValueWhen(int a[], int b[], int nPos)
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

void SOUI::SKlinePic::DataInit()
{
	if (m_pAll == nullptr)
		m_pAll = new AllKPIC_INFO;
	ZeroMemory(m_pAll, sizeof(AllKPIC_INFO));


	int nsize = sizeof(AllKPIC_INFO);
	m_pAll->nTotal = 0;
	m_pAll->nLineType = 1;
	::EnterCriticalSection(&g_csTick);
	if (!m_bSubInsisGroup)
	{
		if (isalpha(m_strSubIns[0]))
		{
			m_bIsStockIndex = false;
			if (!g_TickHash[m_strSubIns].empty())
				m_fPreSettlePrice = g_TickHash[m_strSubIns].back().PreSettlementPrice;
		}
		else
		{
			m_bIsStockIndex = false;
			if (!g_StockIndexTickHash[m_strSubIns].empty())
				m_fPreSettlePrice = g_StockIndexTickHash[m_strSubIns].back().PreClosePrice;
			m_bIsStockIndex = true;
		}

		m_pAll->nInstype = GetInsType(m_strSubIns);
		m_pAll->m_Klines.nDecimal = GetInsMinPrice(m_strSubIns);
	}
	else
	{

		InsIDType InsID1 = g_GroupInsMap[m_strSubIns].Ins1;
		InsIDType InsID2 = g_GroupInsMap[m_strSubIns].Ins2;
		m_pAll->nInstype = GetInsType(InsID1,InsID2);
		m_pAll->m_Klines.nDecimal = GetInsMinPrice(InsID1,InsID2);

		double fPrePrice1, fPrePrice2;
		if (isalpha(InsID1[0]))
		{
			if (!g_TickHash[InsID1].empty())
				fPrePrice1 = g_TickHash[InsID1].back().PreSettlementPrice;
		}
		else
		{
			if (!g_StockIndexTickHash[InsID1].empty())
				fPrePrice1 = g_StockIndexTickHash[InsID1].back().PreClosePrice;

		}

		if (isalpha(InsID2[0]))
		{
			if (!g_TickHash[InsID2].empty())
				fPrePrice2 = g_TickHash[InsID2].back().PreSettlementPrice;
		}
		else
		{
			if (!g_StockIndexTickHash[InsID2].empty())
				fPrePrice2 = g_StockIndexTickHash[InsID2].back().PreClosePrice;
		}
		if (*m_pGroupDataType == 0)
			m_fPreSettlePrice = fPrePrice1*g_GroupInsMap[m_strSubIns].Ins1Ratio - fPrePrice2*g_GroupInsMap[m_strSubIns].Ins2Ratio;
		else
			m_fPreSettlePrice = (fPrePrice1*g_GroupInsMap[m_strSubIns].Ins1Ratio) / (fPrePrice2*g_GroupInsMap[m_strSubIns].Ins2Ratio)*RATIOCOE;


	}
	::LeaveCriticalSection(&g_csTick);

	ZeroMemory(&m_Last1MinHisData, sizeof(m_Last1MinHisData));

	m_pAll->m_Klines.nDecimalXi = 1;
	m_pAll->m_Futu.bft = 2;
	if (*m_pGroupDataType == 0)
		_swprintf(m_pAll->m_Klines.sDecimal, L"%%.%df", m_pAll->m_Klines.nDecimal);
	else
		_swprintf(m_pAll->m_Klines.sDecimal, L"%%.%df", 2);

	m_pAll->m_Futu.nDecimal = 1;
	m_pAll->m_Futu.nDecimalXi = 1;
	m_bNeedAddCount = false;
	TraceLog("K线图数据初始化完成");
}

void SOUI::SKlinePic::SingleDataProc()
{
	DataInit();
	SingleDataWithHis();
	SingleDataNoHis();
	//	m_bDataInited = true;
	//	SingleDataUpdate();
	TraceLog("%s K线图处理完成", m_strSubIns.GetBuffer(1));

}

void SOUI::SKlinePic::SingleDataWithHis()
{

	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

	bool bStart = false;
	m_bHandleTdyFirstLine = false;
	std::vector<KLineDataType> recVec;
	if (m_nKlineType != 5)
	{
		if (GetFileKlineData(m_strSubIns, &recVec))
		{
			int VecSize = recVec.size();
			int count = 0;
			for (auto iter = recVec.begin(); iter != recVec.end(); iter++)
			{
				if (m_nKlineType == 0)
				{
					if (recVec.end() - iter < 4700)		//1分钟K线
					{
						if (IsCATime(iter->UpdateTime,iter->TradingDay))
						{
							if (m_nCAType == CA_Show)
							{
								AddCADataToFirstLine(*iter, *(iter + 1));
								continue;
							}
							else if (m_nCAType == CA_Hide)
								continue;
						}
						else if (nDate == iter->TradingDay && !m_bHandleTdyFirstLine)
						{
							m_bHandleTdyFirstLine = true;
							OutPutDebugStringFormat("开：%.2f 高:%.2f 低:%.2f 收:%.2f\n", iter->open_price, iter->high_price, iter->low_price, iter->close_price);

							if (IsFirstLine(iter->UpdateTime) && m_nCAType == CA_Show)
							{
								::EnterCriticalSection(&g_csCAInfo);
								if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
								{
									CallAutionData_t CaData = g_CADataMap[m_strSubIns];
									iter->open_price = CaData.fOpenPrice;
									if (iter->high_price < CaData.fOpenPrice)
										iter->high_price = CaData.fOpenPrice;
									if (iter->low_price > CaData.fOpenPrice)
										iter->low_price = CaData.fOpenPrice;
									iter->volume += CaData.nVolume;
								}
								::LeaveCriticalSection(&g_csCAInfo);
							}
						}
						m_pAll->m_Klines.pd[count].close = iter->close_price;
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
						m_pAll->m_Klines.pd[count].date = iter->TradingDay;
						m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
						m_pAll->m_Futu.ftl[count] = iter->volume;
						count++;
						KlineMAProc(count);
					}

				}
				else if (m_nKlineType == 1)
				{
					if (recVec.end() - iter < 4700 * 5)		//5分钟K线
					{
						int nLeft = (iter->UpdateTime % 10000) / 100 % 5;
						if (nLeft == 0)
						{

							bStart = true;
							if (IsCATime(iter->UpdateTime, iter->TradingDay))
							{
								if (m_nCAType == CA_Show)
								{
									AddCADataToFirstLine(*iter, *(iter + 1));
									continue;
								}
								else if (m_nCAType == CA_Hide)
									continue;
							}
							else if (nDate == iter->TradingDay && !m_bHandleTdyFirstLine)
							{
								m_bHandleTdyFirstLine = true;
								if (IsFirstLine(iter->UpdateTime) && m_nCAType == CA_Show)
								{
									::EnterCriticalSection(&g_csCAInfo);
									if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
									{
										CallAutionData_t CaData = g_CADataMap[m_strSubIns];
										iter->open_price = CaData.fOpenPrice;
										if (iter->high_price < CaData.fOpenPrice)
											iter->high_price = CaData.fOpenPrice;
										if (iter->low_price > CaData.fOpenPrice)
											iter->low_price = CaData.fOpenPrice;
										iter->volume += CaData.nVolume;
									}
									::LeaveCriticalSection(&g_csCAInfo);
								}
							}

							if (!(iter->UpdateTime == 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype))
							{
								if (m_pAll->m_Klines.nLastVolume != 0)
								{
									count++;
									KlineMAProc(count);
									m_pAll->m_Klines.nLastVolume = 0;
								}
								m_pAll->m_Klines.pd[count].close = iter->close_price;
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;
								m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
								m_pAll->m_Futu.ftl[count] += iter->volume;
								m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							}
						}
						else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
						{

							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

						}
						else if (bStart)
						{
							if (m_pAll->m_Klines.pd[count].open == 0)
							{
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							}
							if (m_pAll->m_Klines.pd[count].high < iter->high_price)
								m_pAll->m_Klines.pd[count].high = iter->high_price;
							if (m_pAll->m_Klines.pd[count].low > iter->low_price)
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							if (nLeft == 4)
							{
								if (m_pAll->m_Klines.pd[count].time == 0)
								{
									m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
									m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								}
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
							}
						}

					}

				}
				else if (m_nKlineType == 2)
				{
					if (recVec.end() - iter < 4700 * 15)		//15分钟K线
					{
						int nLeft = (iter->UpdateTime % 10000) / 100 % 15;
						if (nLeft == 0 || IsCATime(iter->UpdateTime, iter->TradingDay))
						{
							bStart = true;
							if (IsCATime(iter->UpdateTime, iter->TradingDay))
							{
								if (m_nCAType == CA_Show)
								{
									AddCADataToFirstLine(*iter, *(iter + 1));
									continue;
								}
								else if (m_nCAType == CA_Hide)
									continue;
							}
							else if (nDate == iter->TradingDay && !m_bHandleTdyFirstLine)
							{
								m_bHandleTdyFirstLine = true;
								if (IsFirstLine(iter->UpdateTime) && m_nCAType == CA_Show)
								{
									::EnterCriticalSection(&g_csCAInfo);
									if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
									{
										CallAutionData_t CaData = g_CADataMap[m_strSubIns];
										iter->open_price = CaData.fOpenPrice;
										if (iter->high_price < CaData.fOpenPrice)
											iter->high_price = CaData.fOpenPrice;
										if (iter->low_price > CaData.fOpenPrice)
											iter->low_price = CaData.fOpenPrice;
										iter->volume += CaData.nVolume;
									}
									::LeaveCriticalSection(&g_csCAInfo);
								}
							}

							if (!(iter->UpdateTime == 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype))
							{
								if (m_pAll->m_Klines.nLastVolume != 0)
								{
									count++;
									KlineMAProc(count);
									m_pAll->m_Klines.nLastVolume = 0;

								}
								m_pAll->m_Klines.pd[count].close = iter->close_price;
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;
								m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
								m_pAll->m_Futu.ftl[count] += iter->volume;
								m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							}
						}
						else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
						{

							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;

							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

						}
						else if (bStart)
						{
							if (m_pAll->m_Klines.pd[count].open == 0)
							{
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							}
							if (m_pAll->m_Klines.pd[count].high < iter->high_price)
								m_pAll->m_Klines.pd[count].high = iter->high_price;
							if (m_pAll->m_Klines.pd[count].low > iter->low_price)
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							if (nLeft == 14)
							{
								if (m_pAll->m_Klines.pd[count].time == 0)
								{
									m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 1400;
									m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								}

								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
							}
						}

					}

				}
				else if (m_nKlineType == 3)
				{
					if (recVec.end() - iter < 4700 * 30)		//30分钟K线
					{
						int nLeft = (iter->UpdateTime % 10000) / 100 % 30;
						if (nLeft == 0 || IsCATime(iter->UpdateTime, iter->TradingDay))
						{
							bStart = true;
							if (IsCATime(iter->UpdateTime, iter->TradingDay))
							{
								if (m_nCAType == CA_Show)
								{
									AddCADataToFirstLine(*iter, *(iter + 1));
									continue;
								}
								else if (m_nCAType == CA_Hide)
									continue;
							}
							else if (nDate == iter->TradingDay && !m_bHandleTdyFirstLine)
							{
								m_bHandleTdyFirstLine = true;
								if (IsFirstLine(iter->UpdateTime) && m_nCAType == CA_Show)
								{
									::EnterCriticalSection(&g_csCAInfo);
									if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
									{
										CallAutionData_t CaData = g_CADataMap[m_strSubIns];
										iter->open_price = CaData.fOpenPrice;
										if (iter->high_price < CaData.fOpenPrice)
											iter->high_price = CaData.fOpenPrice;
										if (iter->low_price > CaData.fOpenPrice)
											iter->low_price = CaData.fOpenPrice;
										iter->volume += CaData.nVolume;
									}
									::LeaveCriticalSection(&g_csCAInfo);
								}
							}
							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;

							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;

							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
						}
						else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
						{
							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;

							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;

							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

						}
						else if (bStart)
						{
							if (m_pAll->m_Klines.pd[count].open == 0)
							{
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;

							}
							if (m_pAll->m_Klines.pd[count].high < iter->high_price)
								m_pAll->m_Klines.pd[count].high = iter->high_price;
							if (m_pAll->m_Klines.pd[count].low > iter->low_price)
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							if (nLeft == 29
								|| (iter->UpdateTime == 101400 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
								|| (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt))
							{
								if (m_pAll->m_Klines.pd[count].time == 0)
								{
									if (iter->UpdateTime == 101400 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
										m_pAll->m_Klines.pd[count].time = 100000;
									else if (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt)
										m_pAll->m_Klines.pd[count].time = 150000;
									else
										m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 2900;
									m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								}
								count++;
								KlineMAProc(count);

								m_pAll->m_Klines.nLastVolume = 0;
							}
							if (iter->UpdateTime == 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
								m_pAll->m_Klines.nLastVolume = 0;
						}
					}
				}
				else if (m_nKlineType == 4)
				{
					if (recVec.end() - iter < 4700 * 50)		//60分钟K线
					{
						int nLeft = (iter->UpdateTime % 10000) / 100 % 60;
						if (nLeft == 0									//每个小时第一分钟
							|| ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt) && iter->UpdateTime == 93000)			//金融期货上午开盘时间
							|| (StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype&&iter->UpdateTime == 133000)
							|| IsCATime(iter->UpdateTime, iter->TradingDay))			//商品期货下午开盘时间
						{
							bStart = true;
							if (IsCATime(iter->UpdateTime, iter->TradingDay))
							{
								if (m_nCAType == CA_Show)
								{
									AddCADataToFirstLine(*iter, *(iter + 1));
									continue;
								}
								else if (m_nCAType == CA_Hide)
									continue;
							}
							else if (nDate == iter->TradingDay && !m_bHandleTdyFirstLine)
							{
								m_bHandleTdyFirstLine = true;
								if (IsFirstLine(iter->UpdateTime) && m_nCAType == CA_Show)
								{
									::EnterCriticalSection(&g_csCAInfo);
									if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
									{
										CallAutionData_t CaData = g_CADataMap[m_strSubIns];
										iter->open_price = CaData.fOpenPrice;
										if (iter->high_price < CaData.fOpenPrice)
											iter->high_price = CaData.fOpenPrice;
										if (iter->low_price > CaData.fOpenPrice)
											iter->low_price = CaData.fOpenPrice;
										iter->volume += CaData.nVolume;
									}
									::LeaveCriticalSection(&g_csCAInfo);
								}
							}

							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;

							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							if ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt) && iter->UpdateTime == 93000)
								m_pAll->m_Klines.pd[count].time = 90000;
							else if (StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype&&iter->UpdateTime == 133000)
								m_pAll->m_Klines.pd[count].time = 130000;
							else
								m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
						}
						else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
						{
							if (m_pAll->m_Klines.nLastVolume != 0)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
							}
							m_pAll->m_Klines.pd[count].close = iter->close_price;

							m_pAll->m_Klines.pd[count].open = iter->open_price;
							m_pAll->m_Klines.pd[count].high = iter->high_price;
							m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

						}
						else if (bStart)
						{
							if (m_pAll->m_Klines.pd[count].open == 0)
							{
								m_pAll->m_Klines.pd[count].open = iter->open_price;
								m_pAll->m_Klines.pd[count].high = iter->high_price;
								m_pAll->m_Klines.pd[count].low = iter->low_price;

							}
							if (m_pAll->m_Klines.pd[count].high < iter->high_price)
								m_pAll->m_Klines.pd[count].high = iter->high_price;
							if (m_pAll->m_Klines.pd[count].low > iter->low_price)
								m_pAll->m_Klines.pd[count].low = iter->low_price;
							m_pAll->m_Klines.pd[count].close = iter->close_price;
							m_pAll->m_Futu.ftl[count] += iter->volume;
							m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
							if (nLeft == 59							//每个小时最后一分钟
								|| iter->UpdateTime == 22900
								|| iter->UpdateTime == 112900										//上午收盘时间
								|| (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt))		//国债期货下午收盘时间
							{
								if (m_pAll->m_Klines.pd[count].time == 0)
								{
									if (iter->UpdateTime == 112900)
										m_pAll->m_Klines.pd[count].time = 110000;
									else if (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt)
										m_pAll->m_Klines.pd[count].time = 150000;
									else
										m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 5900;
									m_pAll->m_Klines.pd[count].date = iter->TradingDay;
								}

								count++;
								KlineMAProc(count);

								m_pAll->m_Klines.nLastVolume = 0;
							}
						}

						if (iter == recVec.end() - 1 && iter->UpdateTime == 101400)
							m_pAll->m_Klines.pd[count].close = iter->close_price;
					}
				}
			}
			m_pAll->nTotal = count;
			if (!recVec.empty())
				m_pAll->m_Klines.nLast1MinTime = recVec.back().UpdateTime;
			m_pAll->m_Klines.bShow = true;
		}
	}
	else
	{
		//处理日K线
		if (GetFileKlineData(m_strSubIns, &recVec, true) && !recVec.empty())
		{
			int VecSize = recVec.size();
			int count = 0;
			for (auto iter = recVec.cbegin(); iter != recVec.cend(); iter++)
			{
				if (recVec.cend() - iter < 4700)		//1分钟K线
				{
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
					m_pAll->m_Futu.ftl[count] = iter->volume;
					count++;
					KlineMAProc(count);
				}
			}
			m_pAll->nTotal = count;
		}
	}
	TraceLog("%s K线图历史数据处理完成", m_strSubIns.GetBuffer(1));

}

void SOUI::SKlinePic::SingleDataNoHis()
{

	::EnterCriticalSection(&g_csKline);
	std::vector<KLineDataType> KlineVec = g_KlineHash[m_strSubIns];
	::LeaveCriticalSection(&g_csKline);
	int nCount = KlineVec.size();

	if (KlineVec.empty())
	{
		if (m_pAll->m_Klines.nLastVolume != 0)
		{
			m_pAll->nTotal++;
			KlineMAProc(m_pAll->nTotal);
		}
		return;
	}

	int count = m_pAll->nTotal;

	bool bBeginIsFirst = false;
	if (IsFirstLine(KlineVec.cbegin()->UpdateTime) && m_nCAType == CA_Show)
		bBeginIsFirst = true;


	for (auto iter = KlineVec.begin(); iter != KlineVec.end(); iter++)
	{
		if (m_nKlineType == 0)				//1分钟K线
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{

				m_pAll->m_Klines.pd[count].close = iter->close_price;
				m_pAll->m_Klines.pd[count].open = iter->open_price;
				m_pAll->m_Klines.pd[count].high = iter->high_price;
				m_pAll->m_Klines.pd[count].low = iter->low_price;
				m_pAll->m_Klines.pd[count].date = iter->TradingDay;
				m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
				m_pAll->m_Futu.ftl[count] = iter->volume;
				if (bBeginIsFirst)
				{
					bBeginIsFirst = false;
					if (iter == KlineVec.begin())
					{
						::EnterCriticalSection(&g_csCAInfo);
						if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
						{
							CallAutionData_t CaData = g_CADataMap[m_strSubIns];
							m_pAll->m_Klines.pd[count].open = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[count].high < CaData.fOpenPrice)
								m_pAll->m_Klines.pd[count].high = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[count].low > CaData.fOpenPrice)
								m_pAll->m_Klines.pd[count].low = CaData.fOpenPrice;
							m_pAll->m_Futu.ftl[count] += CaData.nVolume;
						}
						::LeaveCriticalSection(&g_csCAInfo);
					}
				}

				count++;
				m_pAll->nTotal++;
				KlineMAProc(count);
			}

		}
		else if (m_nKlineType == 1)	//5分钟K线
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time + 400 || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{

				int nLeft = (iter->UpdateTime % 10000) / 100 % 5;
				if (nLeft == 0)
				{
					if (!(iter->UpdateTime == 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype))
					{
						if (m_pAll->m_Klines.nLastVolume != 0 && iter->UpdateTime != m_pAll->m_Klines.pd[count].time)
						{
							count++;
							m_pAll->m_Klines.nLastVolume = 0;
							m_pAll->nTotal++;
							KlineMAProc(count);
						}
						m_pAll->m_Klines.pd[count].close = iter->close_price;
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
						m_pAll->m_Klines.pd[count].date = iter->TradingDay;
						m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
						m_pAll->m_Futu.ftl[count] = iter->volume;
						if (bBeginIsFirst)
						{
							bBeginIsFirst = false;
							if (iter == KlineVec.begin())
							{
								if (iter == KlineVec.begin())
								{
									::EnterCriticalSection(&g_csCAInfo);
									if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
									{
										CallAutionData_t CaData = g_CADataMap[m_strSubIns];
										m_pAll->m_Klines.pd[count].open = CaData.fOpenPrice;
										if (m_pAll->m_Klines.pd[count].high < CaData.fOpenPrice)
											m_pAll->m_Klines.pd[count].high = CaData.fOpenPrice;
										if (m_pAll->m_Klines.pd[count].low > CaData.fOpenPrice)
											m_pAll->m_Klines.pd[count].low = CaData.fOpenPrice;
										m_pAll->m_Futu.ftl[count] += CaData.nVolume;
									}
									::LeaveCriticalSection(&g_csCAInfo);
								}
							}
						}
						m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

					}
				}
				else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
				{

					if (m_pAll->m_Klines.nLastVolume != 0)
					{
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

				}

				else
				{
					if (m_pAll->m_Klines.pd[count].open == 0)
					{
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					}
					if (m_pAll->m_Klines.pd[count].high < iter->high_price)
						m_pAll->m_Klines.pd[count].high = iter->high_price;
					if (m_pAll->m_Klines.pd[count].low > iter->low_price)
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
					if (nLeft == 4)
					{
						if (m_pAll->m_Klines.pd[count].time == 0)
						{
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 400;
						}
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
				}
				m_pAll->m_Klines.nLast1MinTime = iter->UpdateTime;
			}

		}
		else if (m_nKlineType == 2)			//15分钟K线
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time + 1400 || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{
				int nLeft = (iter->UpdateTime % 10000) / 100 % 15;
				if (nLeft == 0)
				{
					if (iter->UpdateTime / 100 % 5 == 0)
					{
						if (m_pAll->m_Klines.nLastVolume != 0 && iter->UpdateTime != m_pAll->m_Klines.pd[count].time)
						{
							count++;
							m_pAll->m_Klines.nLastVolume = 0;
							m_pAll->nTotal++;
							KlineMAProc(count);
						}
						m_pAll->m_Klines.pd[count].close = iter->close_price;
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
						m_pAll->m_Klines.pd[count].date = iter->TradingDay;
						m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
						m_pAll->m_Futu.ftl[count] = iter->volume;
						if (bBeginIsFirst)
						{
							bBeginIsFirst = false;
							if (iter == KlineVec.begin())
							{
								::EnterCriticalSection(&g_csCAInfo);
								if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
								{
									CallAutionData_t CaData = g_CADataMap[m_strSubIns];
									m_pAll->m_Klines.pd[count].open = CaData.fOpenPrice;
									if (m_pAll->m_Klines.pd[count].high < CaData.fOpenPrice)
										m_pAll->m_Klines.pd[count].high = CaData.fOpenPrice;
									if (m_pAll->m_Klines.pd[count].low > CaData.fOpenPrice)
										m_pAll->m_Klines.pd[count].low = CaData.fOpenPrice;
									m_pAll->m_Futu.ftl[count] += CaData.nVolume;
								}
								::LeaveCriticalSection(&g_csCAInfo);
							}
						}

						m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

					}
				}
				else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
				{

					if (m_pAll->m_Klines.nLastVolume != 0)
					{
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);

					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

				}
				else
				{
					if (m_pAll->m_Klines.pd[count].open == 0)
					{
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					}
					if (m_pAll->m_Klines.pd[count].high < iter->high_price)
						m_pAll->m_Klines.pd[count].high = iter->high_price;
					if (m_pAll->m_Klines.pd[count].low > iter->low_price)
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
					if (nLeft == 14)
					{
						if (m_pAll->m_Klines.pd[count].time == 0)
						{
							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
							m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 1400;
						}
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
				}
				m_pAll->m_Klines.nLast1MinTime = iter->UpdateTime;
			}

		}
		else if (m_nKlineType == 3)		//30分钟K线
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time + 2900 || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{

				int nLeft = (iter->UpdateTime % 10000) / 100 % 30;
				if (nLeft == 0)
				{
					if (m_pAll->m_Klines.nLastVolume != 0 && iter->UpdateTime != m_pAll->m_Klines.pd[count].time
						&&iter->UpdateTime != 91500)
					{
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
					m_pAll->m_Futu.ftl[count] = iter->volume;
					if (bBeginIsFirst)
					{
						bBeginIsFirst = false;
						if (iter == KlineVec.begin())
						{
							::EnterCriticalSection(&g_csCAInfo);
							if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
							{
								CallAutionData_t CaData = g_CADataMap[m_strSubIns];
								m_pAll->m_Klines.pd[count].open = CaData.fOpenPrice;
								if (m_pAll->m_Klines.pd[count].high < CaData.fOpenPrice)
									m_pAll->m_Klines.pd[count].high = CaData.fOpenPrice;
								if (m_pAll->m_Klines.pd[count].low > CaData.fOpenPrice)
									m_pAll->m_Klines.pd[count].low = CaData.fOpenPrice;
								m_pAll->m_Futu.ftl[count] += CaData.nVolume;
							}
							::LeaveCriticalSection(&g_csCAInfo);
						}
					}

					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
				}
				else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
				{
					if (m_pAll->m_Klines.nLastVolume != 0)
					{
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;

					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

				}

				else
				{
					if (m_pAll->m_Klines.pd[count].open == 0)
					{
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					}
					if (m_pAll->m_Klines.pd[count].high < iter->high_price)
						m_pAll->m_Klines.pd[count].high = iter->high_price;
					if (m_pAll->m_Klines.pd[count].low > iter->low_price)
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
					if (nLeft == 29 ||
						(iter->UpdateTime == 101400 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt))
					{
						if (m_pAll->m_Klines.pd[count].time == 0)
						{
							if (iter->UpdateTime == 101400
								&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
								m_pAll->m_Klines.pd[count].time = 100000;
							else if (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt)
								m_pAll->m_Klines.pd[count].time = 150000;
							else
								m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 2900;

							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
						}
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}
					if (iter->UpdateTime == 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						m_pAll->m_Klines.nLastVolume = 0;

				}
				m_pAll->m_Klines.nLast1MinTime = iter->UpdateTime;
			}

		}
		else if (m_nKlineType == 4)		//60分钟K线
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time + 5900 || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{
				int nLeft = (iter->UpdateTime % 10000) / 100 % 60;
				if (nLeft == 0									//每个小时第一分钟
					|| ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt) && iter->UpdateTime == 93000)			//金融期货上午开盘时间
					|| (StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype&&iter->UpdateTime == 133000))			//商品期货下午开盘时间
				{
					if (m_pAll->m_Klines.nLastVolume != 0 && iter->UpdateTime != m_pAll->m_Klines.pd[count].time
						&& iter->UpdateTime != 93000)
					{
						count++;
						m_pAll->nTotal++;
						m_pAll->m_Klines.nLastVolume = 0;
						KlineMAProc(count);
					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					if ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt) && iter->UpdateTime == 93000)
						m_pAll->m_Klines.pd[count].time = 90000;
					else if (StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype&&iter->UpdateTime == 133000)
						m_pAll->m_Klines.pd[count].time = 130000;
					else
						m_pAll->m_Klines.pd[count].time = iter->UpdateTime;
					m_pAll->m_Futu.ftl[count] = iter->volume;
					if (bBeginIsFirst)
					{
						bBeginIsFirst = false;
						if (iter == KlineVec.begin())
						{
							::EnterCriticalSection(&g_csCAInfo);
							if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
							{
								CallAutionData_t CaData = g_CADataMap[m_strSubIns];
								m_pAll->m_Klines.pd[count].open = CaData.fOpenPrice;
								if (m_pAll->m_Klines.pd[count].high < CaData.fOpenPrice)
									m_pAll->m_Klines.pd[count].high = CaData.fOpenPrice;
								if (m_pAll->m_Klines.pd[count].low > CaData.fOpenPrice)
									m_pAll->m_Klines.pd[count].low = CaData.fOpenPrice;
								m_pAll->m_Futu.ftl[count] += CaData.nVolume;
							}
							::LeaveCriticalSection(&g_csCAInfo);
						}
					}
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
				}
				else if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
				{
					if (m_pAll->m_Klines.nLastVolume != 0)
					{
						count++;
						m_pAll->nTotal++;
						m_pAll->m_Klines.nLastVolume = 0;
						KlineMAProc(count);
					}
					m_pAll->m_Klines.pd[count].close = iter->close_price;

					m_pAll->m_Klines.pd[count].open = iter->open_price;
					m_pAll->m_Klines.pd[count].high = iter->high_price;
					m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].date = iter->TradingDay;
					m_pAll->m_Klines.pd[count].time = iter->UpdateTime - nLeft * 100;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];

				}
				else
				{
					if (m_pAll->m_Klines.pd[count].open == 0)
					{
						m_pAll->m_Klines.pd[count].open = iter->open_price;
						m_pAll->m_Klines.pd[count].high = iter->high_price;
						m_pAll->m_Klines.pd[count].low = iter->low_price;

					}
					if (m_pAll->m_Klines.pd[count].high < iter->high_price)
						m_pAll->m_Klines.pd[count].high = iter->high_price;
					if (m_pAll->m_Klines.pd[count].low > iter->low_price)
						m_pAll->m_Klines.pd[count].low = iter->low_price;
					m_pAll->m_Klines.pd[count].close = iter->close_price;
					m_pAll->m_Futu.ftl[count] += iter->volume;
					m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[count];
					if (nLeft == 59							//每个小时最后一分钟
						|| iter->UpdateTime == 112900										//上午收盘时间
						|| (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt))		//国债期货下午收盘时间
					{
						if (m_pAll->m_Klines.pd[count].time == 0)
						{
							if (iter->UpdateTime == 112900)
								m_pAll->m_Klines.pd[count].time = 110000;
							else if (iter->UpdateTime == 151400 && m_pAll->nInstype == NationalDebt)
								m_pAll->m_Klines.pd[count].time = 150000;
							else
								m_pAll->m_Klines.pd[count].time = iter->UpdateTime - 5900;

							m_pAll->m_Klines.pd[count].date = iter->TradingDay;
						}
						count++;
						m_pAll->m_Klines.nLastVolume = 0;
						m_pAll->nTotal++;
						KlineMAProc(count);
					}

				}
				m_pAll->m_Klines.nLast1MinTime = iter->UpdateTime;
			}

		}
	}
	if (m_pAll->m_Klines.nLastVolume != 0)
	{
		m_pAll->m_Klines.pd[m_pAll->nTotal].close = KlineVec.back().close_price;
		m_pAll->nTotal++;
		KlineMAProc(m_pAll->nTotal);
	}
	TraceLog("%s K线图非历史数据处理完成", m_strSubIns.GetBuffer(1));

}

void SOUI::SKlinePic::SingleDataUpdate()
{
	if (!m_bIsStockIndex)
	{
		if (m_nKlineType == 0)
			SingleFutures1MinUpdate();
		else if (m_nKlineType < 5)
			SingleFuturesMultMinUpdate(m_nPeriod);
		else
			SingleFuturesDayUpdate();
	}
	else
	{
		if (m_nKlineType == 0)
			SingleIndex1MinUpdate();
		else if (m_nKlineType < 5)
			SingleIndexMultMinUpdate(m_nPeriod);
		else
			SingleIndexDayUpdate();

	}
}

void SOUI::SKlinePic::SingleFutures1MinUpdate()
{

	std::vector<KLineDataType>* pKlineVec = &g_KlineHash[m_strSubIns];
	std::vector<RestoreTickType>* pTickVec = &g_TickHash[m_strSubIns];
	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csTick);
	if (pKlineVec->empty() && !pTickVec->empty())		//没有接收过实时K线
	{

		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| (ntime < 90000))
		{
			::LeaveCriticalSection(&g_csTick);
			return;
		}

		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
		{
			//		OutputDebugString(L"1Min:情况1\n");

			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || ntime < 90000 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
					break;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time&& atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime == 150000 && NationalDebt != m_pAll->nInstype))
					break;
				//if (ntime == 85900 || (ntime == 92900 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
				//{
				//	if (m_nCAType == CA_Show)
				//	{
				//		m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).OpenPrice;
				//		m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).OpenPrice;
				//		m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).OpenPrice;
				//	}
				//	else
				//		break;
				//}
				//else
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;

				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				//if (ntime == 85900)
				//	m_pAll->m_Klines.pd[nDataCount].time = 90000;
				//else if (ntime == 92900 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt))
				//	m_pAll->m_Klines.pd[nDataCount].time = 93000;
				//else
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;

			}

			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}

			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}
		}
		else if (ntime == m_pAll->m_Klines.pd[nDataCount - 1].time)
		{
			//		OutputDebugString(L"1Min:情况2\n");
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
					break;
				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount - 1].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);

				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}

			KlineMAProc(m_pAll->nTotal);
		}
	}
	else if (!pTickVec->empty())				 			//接收过实时K线
	{
		::EnterCriticalSection(&g_csKline);
		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time)			//新的时间大于最后一条K线的时间，直接添加
		{
			m_pAll->m_Klines.pd[nDataCount].close = pKlineVec->back().close_price;
			m_pAll->m_Klines.pd[nDataCount].open = pKlineVec->back().open_price;
			m_pAll->m_Klines.pd[nDataCount].high = pKlineVec->back().high_price;
			m_pAll->m_Klines.pd[nDataCount].low = pKlineVec->back().low_price;
			m_pAll->m_Klines.pd[nDataCount].date = pKlineVec->back().TradingDay;
			m_pAll->m_Klines.pd[nDataCount].time = pKlineVec->back().UpdateTime;
			m_pAll->m_Futu.ftl[nDataCount] = pKlineVec->back().volume;
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}
			nDataCount++;
			m_pAll->nTotal++;
			KlineMAProc(m_pAll->nTotal);

		}
		else if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.pd[nDataCount - 1].time)	//新的时间等于最后一条K线的时间，先覆盖，在添加
		{
			//		OutputDebugString(L"1Min:情况4\n");
			m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
			m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->back().open_price;
			m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->back().high_price;
			m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->back().low_price;
			m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
			m_pAll->m_Klines.pd[nDataCount - 1].time = pKlineVec->back().UpdateTime;
			m_pAll->m_Futu.ftl[nDataCount - 1] = pKlineVec->back().volume;

			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}


			KlineMAProc(m_pAll->nTotal);
			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);

					return;
				}

				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype && m_pAll->m_Klines.pd[nDataCount - 1].time == 101400)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;

				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}
		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{
			//		OutputDebugString(L"1Min:情况5\n");

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}

				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 2].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount - 1].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}

			KlineMAProc(m_pAll->nTotal);
		}
		::LeaveCriticalSection(&g_csKline);

	}
	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::SingleIndex1MinUpdate()
{

	std::vector<KLineDataType>* pKlineVec = &g_KlineHash[m_strSubIns];
	std::vector<StockIndex_t>* pTickVec = &g_StockIndexTickHash[m_strSubIns];
	//	auto pTickVec = &g_TickHash[g_arInsID[m_nSubIns]];
	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csTick);
	if (pKlineVec->empty() && !pTickVec->empty())		//没有接收过实时K线
	{
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| (ntime < 93000))
		{
			::LeaveCriticalSection(&g_csTick);
			return;
		}

		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
		{
			//		OutputDebugString(L"1Min:情况1\n");
			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime > 150000 || ntime < 93000)
					break;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time&& atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
					break;
				if (ntime == 113000)
					break;
				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = (pTickVec->back().Turnover/10000 - pTickVec->at(i - 1).Turnover)/10000;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Turnover/10000 - pTickVec->at(i).Turnover/10000;

			}

			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}

			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}
		}
		else if (ntime == m_pAll->m_Klines.pd[nDataCount - 1].time)
		{
			//		OutputDebugString(L"1Min:情况2\n");
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime > 150000 || ntime < 93000)
					break;
				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if (ntime == 113000)
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount - 1].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Turnover - pTickVec->at(i - 1).Turnover;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Turnover - pTickVec->at(i).Turnover;
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}

			KlineMAProc(m_pAll->nTotal);
		}
	}
	else if (!pTickVec->empty())							//接收过实时K线
	{
		::EnterCriticalSection(&g_csKline);
		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time)			//新的时间大于最后一条K线的时间，直接添加
		{
			m_pAll->m_Klines.pd[nDataCount].close = pKlineVec->back().close_price;
			m_pAll->m_Klines.pd[nDataCount].open = pKlineVec->back().open_price;
			m_pAll->m_Klines.pd[nDataCount].high = pKlineVec->back().high_price;
			m_pAll->m_Klines.pd[nDataCount].low = pKlineVec->back().low_price;
			m_pAll->m_Klines.pd[nDataCount].date = pKlineVec->back().TradingDay;
			m_pAll->m_Klines.pd[nDataCount].time = pKlineVec->back().UpdateTime;
			m_pAll->m_Futu.ftl[nDataCount] = pKlineVec->back().volume;
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}

			nDataCount++;
			m_pAll->nTotal++;
			KlineMAProc(m_pAll->nTotal);

		}
		else if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.pd[nDataCount - 1].time)	//新的时间等于最后一条K线的时间，先覆盖，在添加
		{
			//		OutputDebugString(L"1Min:情况4\n");

			m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
			m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->back().open_price;
			m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->back().high_price;
			m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->back().low_price;
			m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
			m_pAll->m_Klines.pd[nDataCount - 1].time = pKlineVec->back().UpdateTime;
			m_pAll->m_Futu.ftl[nDataCount - 1] = pKlineVec->back().volume;
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}

			KlineMAProc(m_pAll->nTotal);
			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);

					return;
				}

				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype && m_pAll->m_Klines.pd[nDataCount - 1].time == 101400)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;

				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}
		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{
			//		OutputDebugString(L"1Min:情况5\n");

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}

				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 2].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount - 1].time = ntime;
				if (ntime == 90000 || (ntime == 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - 0;
				else if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
					m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}
			KlineMAProc(m_pAll->nTotal);
		}
		::LeaveCriticalSection(&g_csKline);

	}
	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::SingleFuturesMultMinUpdate(int nPeriod)
{

	std::vector<KLineDataType>* pKlineVec = &g_KlineHash[m_strSubIns];
	std::vector<RestoreTickType>* pTickVec = &g_TickHash[m_strSubIns];

	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csTick);
	if (pKlineVec->empty())		//没有接收过实时K线
	{
		if (pTickVec->empty())
			return;
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| ntime < 90000)
		{
			::LeaveCriticalSection(&g_csTick);
			return;
		}

		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100 || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date) //新的一条5分钟K线
		{
			ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (0 != nDataCount && (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100 || atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date))
			{
				m_pAll->m_Klines.pd[nDataCount].high = -100000000;
				m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime >= 151500 || ntime < 90000 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
						break;
					if (ntime <= (m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100) && atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					m_pAll->m_Klines.pd[nDataCount].time = ntime;
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}

				if (m_pAll->m_Klines.pd[nDataCount].close != 0)
				{
					m_pAll->nTotal++;
					KlineMAProc(m_pAll->nTotal);
				}
			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
					m_pAll->m_Klines.nLastVolume = 0;
				}


				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (0 == nDataCount || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
					{
						if (ntime < 90000 || ntime>151500 || (ntime < 93000 && (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)))
							break;
					}
					else if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;


					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					if (bNewKline)
						m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					if (bNewKline)
					{

						m_pAll->m_Klines.pd[nDataCount].time = ntime;

						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}


					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}

				m_pAll->m_Futu.ftl[nDataCount] += m_pAll->m_Klines.nLastVolume;
				if (!m_bHandleTdyFirstLine)
				{
					if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
					{
						::EnterCriticalSection(&g_csCAInfo);
						if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
						{
							CallAutionData_t CaData = g_CADataMap[m_strSubIns];
							m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
								m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
								m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
							m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
						}
						::LeaveCriticalSection(&g_csCAInfo);
					}

				}
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}

		}
		else if (ntime >= m_pAll->m_Klines.pd[nDataCount - 1].time) //最后一条K线并不完整
		{
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
				{

					::LeaveCriticalSection(&g_csTick);

					return;
				}

				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);

				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			m_pAll->m_Futu.ftl[nDataCount - 1] += m_pAll->m_Klines.nLastVolume;
			if (!m_bHandleTdyFirstLine)
			{
				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}
			}
			KlineMAProc(m_pAll->nTotal);
		}

	}
	else							//接收过实时K线
	{
		::EnterCriticalSection(&g_csKline);
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100)			//新的时间大于最后一条K线的时间，直接添加 
		{
			int ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)	//最早一条tick时间在下一条K线之前
			{

				m_pAll->m_Klines.pd[nDataCount].high = -10000000;
				m_pAll->m_Klines.pd[nDataCount].low = 100000000;
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					m_pAll->m_Klines.pd[nDataCount].time = ntime;
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);

				}
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个1处新增一条K线", m_nSubIns, nPeriod);

			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
					m_pAll->m_Klines.nLastVolume = 0;
				}
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					if (bNewKline)
						m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					if (bNewKline)
					{
						m_pAll->m_Klines.pd[nDataCount].time = ntime;
						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}
					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				m_pAll->m_Futu.ftl[nDataCount] += m_pAll->m_Klines.nLastVolume;
				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}

				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个2处新增一条K线", m_nSubIns, nPeriod);

			}


		}
		else if (TimeIsLastBar(pKlineVec->back().UpdateTime, m_pAll->m_Klines.pd[nDataCount - 1].time, ntime, nPeriod))//新的时间等于最后一条K线的时间，先覆盖，在添加
		{

			int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
			if (nPeriod != 60)
			{
				if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.nLast1MinTime && (ntime == 113000 || ntime == 112900
					|| ntime == 150000 || ntime == 151500 || ntime == 101400 || ntime == 101500))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}
			}
			else
			{
				if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.nLast1MinTime && (ntime == 113000 || ntime == 112900 || ntime == 150000 || ntime == 151500))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}
			}


			if (pKlineVec->front().UpdateTime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
			{
				m_pAll->m_Futu.ftl[nDataCount - 1] = 0;
				for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
				{
					if (pKlineVec->at(i).UpdateTime < m_pAll->m_Klines.pd[nDataCount - 1].time)
						break;
					m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->at(i).open_price;
					m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
					if (pKlineVec->at(i).high_price > m_pAll->m_Klines.pd[nDataCount - 1].high)
						m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->at(i).high_price;
					if (pKlineVec->at(i).low_price < m_pAll->m_Klines.pd[nDataCount - 1].low)
						m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->at(i).low_price;
					m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
					m_pAll->m_Futu.ftl[nDataCount - 1] += pKlineVec->at(i).volume;
				}

			}
			else
			{
				m_pAll->m_Futu.ftl[nDataCount - 1] = m_pAll->m_Klines.nLastVolume;
				for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
				{
					if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->at(i).open_price;
					m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
					if (pKlineVec->at(i).high_price > m_pAll->m_Klines.pd[nDataCount - 1].high)
						m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->at(i).high_price;
					if (pKlineVec->at(i).low_price < m_pAll->m_Klines.pd[nDataCount - 1].low)
						m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->at(i).low_price;
					m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
					m_pAll->m_Futu.ftl[nDataCount - 1] += pKlineVec->at(i).volume;
				}
			}

			m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;
			m_pAll->m_Klines.nLastVolume = 0;
			KlineMAProc(m_pAll->nTotal);


			//新增一条K线
			m_pAll->m_Klines.pd[nDataCount].low = 100000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
				if (0 != nLeft)
					m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;

				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个3处新增一条K线", m_nSubIns, nPeriod);

			}

		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{

			for (int i = pKlineVec->size() - 1; i >= 0; i--)
			{
				if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				m_pAll->m_Klines.nLastVolume += pKlineVec->at(i).volume;
				if (IsFirstLine(pKlineVec->at(i).UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
						m_pAll->m_Klines.nLastVolume += g_CADataMap[m_strSubIns].nVolume;
					::LeaveCriticalSection(&g_csCAInfo);
				}
			}


			m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);
				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
				}
				::LeaveCriticalSection(&g_csCAInfo);
			}

			m_pAll->m_Futu.ftl[nDataCount - 1] += m_pAll->m_Klines.nLastVolume;
			KlineMAProc(m_pAll->nTotal);

		}
		::LeaveCriticalSection(&g_csKline);
	}

	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::SingleIndexMultMinUpdate(int nPeriod)
{

	std::vector<KLineDataType>* pKlineVec = &g_KlineHash[m_strSubIns];
	std::vector<StockIndex_t>* pTickVec = &g_StockIndexTickHash[m_strSubIns];

	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csTick);
	if (pKlineVec->empty() && !pTickVec->empty())		//没有接收过实时K线
	{
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| ntime < 93000)
		{
			::LeaveCriticalSection(&g_csTick);
			return;
		}

		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100 || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date) //新的一条5分钟K线
		{
			ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (0 != nDataCount && (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100 || atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date))
			{
				//			OutputDebugString(L"5Min:情况1\n");
				m_pAll->m_Klines.pd[nDataCount].high = -100000000;
				m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime > 150000 || ntime < 93000)
						break;
					if (ntime <= (m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100) && atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
						break;
					if (ntime == 113000)
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					m_pAll->m_Klines.pd[nDataCount].time = ntime;
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;

					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}

				if (m_pAll->m_Klines.pd[nDataCount].close != 0)
				{
					m_pAll->nTotal++;
					KlineMAProc(m_pAll->nTotal);
				}

			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
					m_pAll->m_Klines.nLastVolume = 0;
				}


				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (0 == nDataCount || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
					{
						if (ntime < 93000 || ntime>150000)
							break;
					}
					else if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if (ntime == 113000)
						break;

					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					if (bNewKline)
						m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;

					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					if (bNewKline)
					{
						m_pAll->m_Klines.pd[nDataCount].time = ntime;
						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}


					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				m_pAll->m_Futu.ftl[nDataCount] += m_pAll->m_Klines.nLastVolume;
				if (!m_bHandleTdyFirstLine)
				{
					if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
					{
						::EnterCriticalSection(&g_csCAInfo);
						if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
						{
							CallAutionData_t CaData = g_CADataMap[m_strSubIns];
							m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
								m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
							if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
								m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
							m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
						}
						::LeaveCriticalSection(&g_csCAInfo);
					}

				}

				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}

		}
		else if (ntime >= m_pAll->m_Klines.pd[nDataCount - 1].time) //最后一条K线并不完整
		{
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
				{

					::LeaveCriticalSection(&g_csTick);

					return;
				}

				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);

				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			m_pAll->m_Futu.ftl[nDataCount - 1] += m_pAll->m_Klines.nLastVolume;
			if (!m_bHandleTdyFirstLine)
			{
				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount - 1] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}
			}
			KlineMAProc(m_pAll->nTotal);
		}

	}
	else if (!pTickVec->empty())							//接收过实时K线
	{
		::EnterCriticalSection(&g_csKline);
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100)			//新的时间大于最后一条K线的时间，直接添加 
		{
			int ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)	//最早一条tick时间在下一条K线之前
			{

				m_pAll->m_Klines.pd[nDataCount].high = -10000000;
				m_pAll->m_Klines.pd[nDataCount].low = 100000000;
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					m_pAll->m_Klines.pd[nDataCount].time = ntime;
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个1处新增一条K线", m_nSubIns, nPeriod);

			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
					m_pAll->m_Klines.nLastVolume = 0;
				}
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
					if (bNewKline)
						m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
						m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
					if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
						m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
					m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
					if (bNewKline)
					{
						m_pAll->m_Klines.pd[nDataCount].time = ntime;
						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}
					if (i > 0)
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
					else
						m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
				}
				m_pAll->m_Futu.ftl[nDataCount] += m_pAll->m_Klines.nLastVolume;
				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);
					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
					{
						CallAutionData_t CaData = g_CADataMap[m_strSubIns];
						m_pAll->m_Klines.pd[nDataCount].open = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].high < CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].high = CaData.fOpenPrice;
						if (m_pAll->m_Klines.pd[nDataCount].low > CaData.fOpenPrice)
							m_pAll->m_Klines.pd[nDataCount].low = CaData.fOpenPrice;
						m_pAll->m_Futu.ftl[nDataCount] += CaData.nVolume;
					}
					::LeaveCriticalSection(&g_csCAInfo);
				}
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个2处新增一条K线", m_nSubIns, nPeriod);

			}


		}
		else if (TimeIsLastBar(pKlineVec->back().UpdateTime, m_pAll->m_Klines.pd[nDataCount - 1].time, ntime, nPeriod))//新的时间等于最后一条K线的时间，先覆盖，在添加
		{

			int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
			if (nPeriod != 60)
			{
				if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.nLast1MinTime && (ntime == 113000 || ntime == 112900
					|| ntime == 150000 || ntime == 151500 || ntime == 101400 || ntime == 101500))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}
			}
			else
			{
				if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.nLast1MinTime && (ntime == 113000 || ntime == 112900 || ntime == 150000 || ntime == 151500))
				{
					::LeaveCriticalSection(&g_csKline);
					::LeaveCriticalSection(&g_csTick);
					return;
				}
			}


			if (pKlineVec->front().UpdateTime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
			{
				m_pAll->m_Futu.ftl[nDataCount - 1] = 0;
				for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
				{
					if (pKlineVec->at(i).UpdateTime < m_pAll->m_Klines.pd[nDataCount - 1].time)
						break;
					m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->at(i).open_price;
					m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
					if (pKlineVec->at(i).high_price > m_pAll->m_Klines.pd[nDataCount - 1].high)
						m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->at(i).high_price;
					if (pKlineVec->at(i).low_price < m_pAll->m_Klines.pd[nDataCount - 1].low)
						m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->at(i).low_price;
					m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
					m_pAll->m_Futu.ftl[nDataCount - 1] += pKlineVec->at(i).volume;
				}

			}
			else
			{
				m_pAll->m_Futu.ftl[nDataCount - 1] = m_pAll->m_Klines.nLastVolume;
				for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
				{
					if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					m_pAll->m_Klines.pd[nDataCount - 1].open = pKlineVec->at(i).open_price;
					m_pAll->m_Klines.pd[nDataCount - 1].close = pKlineVec->back().close_price;
					if (pKlineVec->at(i).high_price > m_pAll->m_Klines.pd[nDataCount - 1].high)
						m_pAll->m_Klines.pd[nDataCount - 1].high = pKlineVec->at(i).high_price;
					if (pKlineVec->at(i).low_price < m_pAll->m_Klines.pd[nDataCount - 1].low)
						m_pAll->m_Klines.pd[nDataCount - 1].low = pKlineVec->at(i).low_price;
					m_pAll->m_Klines.pd[nDataCount - 1].date = pKlineVec->back().TradingDay;
					m_pAll->m_Futu.ftl[nDataCount - 1] += pKlineVec->at(i).volume;
				}
			}

			m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;
			m_pAll->m_Klines.nLastVolume = 0;
			KlineMAProc(m_pAll->nTotal);


			//新增一条K线
			m_pAll->m_Klines.pd[nDataCount].low = 100000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
				m_pAll->m_Klines.pd[nDataCount].open = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount].high)
					m_pAll->m_Klines.pd[nDataCount].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount].low)
					m_pAll->m_Klines.pd[nDataCount].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount].date = atoi(pTickVec->back().TradingDay);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
				if (0 != nLeft)
					m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;

				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  单个3处新增一条K线", m_nSubIns, nPeriod);

			}

		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{
			for (int i = pKlineVec->size() - 1; i >= 0; i--)
			{
				if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				m_pAll->m_Klines.nLastVolume += pKlineVec->at(i).volume;
				if (IsFirstLine(pKlineVec->at(i).UpdateTime) && m_nCAType == CA_Show)
				{
					::EnterCriticalSection(&g_csCAInfo);

					if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
						m_pAll->m_Klines.nLastVolume += g_CADataMap[m_strSubIns].nVolume;
					::LeaveCriticalSection(&g_csCAInfo);

				}
			}
			m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
				if (pTickVec->at(i).LastPrice > m_pAll->m_Klines.pd[nDataCount - 1].high)
					m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->at(i).LastPrice;
				if (pTickVec->at(i).LastPrice < m_pAll->m_Klines.pd[nDataCount - 1].low)
					m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->at(i).LastPrice;
				m_pAll->m_Klines.pd[nDataCount - 1].date = atoi(pTickVec->back().TradingDay);
				if (i > 0)
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i - 1).Volume;
				else
					m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume - pTickVec->at(i).Volume;
			}
			if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
			{
				::EnterCriticalSection(&g_csCAInfo);

				if (g_CADataMap.find(m_strSubIns) != g_CADataMap.end())
				{
					CallAutionData_t CaData = g_CADataMap[m_strSubIns];
					m_pAll->m_Klines.pd[nDataCount - 1].open = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].high < CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].high = CaData.fOpenPrice;
					if (m_pAll->m_Klines.pd[nDataCount - 1].low > CaData.fOpenPrice)
						m_pAll->m_Klines.pd[nDataCount - 1].low = CaData.fOpenPrice;
				}
				::LeaveCriticalSection(&g_csCAInfo);

			}

			m_pAll->m_Futu.ftl[nDataCount - 1] += m_pAll->m_Klines.nLastVolume;
			KlineMAProc(m_pAll->nTotal);

		}
		::LeaveCriticalSection(&g_csKline);
	}

	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::SingleFuturesDayUpdate()
{

	std::vector<RestoreTickType>* pTickVec = &g_TickHash[m_strSubIns];

	::EnterCriticalSection(&g_csTick);

	int nDay = atoi(pTickVec->back().TradingDay);
	int nDataCount = m_pAll->nTotal;
	if (pTickVec->back().OpenPrice > 100000000)
	{
		::LeaveCriticalSection(&g_csTick);
		return;
	}


	if (nDataCount == 0 || nDay > m_pAll->m_Klines.pd[nDataCount - 1].date)
	{

		m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
		m_pAll->m_Klines.pd[nDataCount].open = pTickVec->back().OpenPrice;
		m_pAll->m_Klines.pd[nDataCount].high = pTickVec->back().HighestPrice;
		m_pAll->m_Klines.pd[nDataCount].low = pTickVec->back().LowestPrice;
		m_pAll->m_Klines.pd[nDataCount].date = nDay;
		m_pAll->m_Klines.pd[nDataCount].time = 0;
		m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume;
		m_pAll->nTotal++;
		KlineMAProc(m_pAll->nTotal);

	}
	else
	{

		m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->back().OpenPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->back().HighestPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->back().LowestPrice;
		m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume;
		KlineMAProc(m_pAll->nTotal);

	}
	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::SingleIndexDayUpdate()
{

	std::vector<StockIndex_t>* pTickVec = &g_StockIndexTickHash[m_strSubIns];

	::EnterCriticalSection(&g_csTick);

	int nDay = atoi(pTickVec->back().TradingDay);
	int nDataCount = m_pAll->nTotal;
	if (pTickVec->back().OpenPrice == 0)
	{
		::LeaveCriticalSection(&g_csTick);
		return;
	}

	if (nDataCount == 0||nDay > m_pAll->m_Klines.pd[nDataCount - 1].date)
	{

		m_pAll->m_Klines.pd[nDataCount].close = pTickVec->back().LastPrice;
		m_pAll->m_Klines.pd[nDataCount].open = pTickVec->back().OpenPrice;
		m_pAll->m_Klines.pd[nDataCount].high = pTickVec->back().HighestPrice;
		m_pAll->m_Klines.pd[nDataCount].low = pTickVec->back().LowestPrice;
		m_pAll->m_Klines.pd[nDataCount].date = nDay;
		m_pAll->m_Klines.pd[nDataCount].time = 0;
		m_pAll->m_Futu.ftl[nDataCount] = pTickVec->back().Volume;
		m_pAll->nTotal++;
		KlineMAProc(m_pAll->nTotal);

	}
	else
	{

		m_pAll->m_Klines.pd[nDataCount - 1].close = pTickVec->back().LastPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].open = pTickVec->back().OpenPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].high = pTickVec->back().HighestPrice;
		m_pAll->m_Klines.pd[nDataCount - 1].low = pTickVec->back().LowestPrice;
		m_pAll->m_Futu.ftl[nDataCount - 1] = pTickVec->back().Volume;
		KlineMAProc(m_pAll->nTotal);

	}
	::LeaveCriticalSection(&g_csTick);
}

void SOUI::SKlinePic::GroupToData(int nCount, const GroupKlineType & kline, int nTimeOffset)
{
	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].close = kline.close_price;
		m_pAll->m_Klines.pd[nCount].open = kline.open_price;
		m_pAll->m_Klines.pd[nCount].high = kline.high_price;
		m_pAll->m_Klines.pd[nCount].low = kline.low_price;
	}
	else
	{
		m_pAll->m_Klines.pd[nCount].close = kline.close_price_r;
		m_pAll->m_Klines.pd[nCount].open = kline.open_price_r;
		m_pAll->m_Klines.pd[nCount].high = kline.high_price_r;
		m_pAll->m_Klines.pd[nCount].low = kline.low_price_r;
	}
	m_pAll->m_Klines.pd[nCount].date = kline.TradingDay;
	m_pAll->m_Klines.pd[nCount].time = kline.UpdateTime - nTimeOffset;
	m_pAll->m_Futu.ftl[nCount] = kline.volume;
	m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[nCount];

}

void SOUI::SKlinePic::GroupUpdateWithHad(int nCount, const GroupKlineType & kline)
{
	if (*m_pGroupDataType == 0)
	{
		if (m_pAll->m_Klines.pd[nCount].open == 0)
		{
			m_pAll->m_Klines.pd[nCount].open = kline.open_price;
			m_pAll->m_Klines.pd[nCount].high = kline.high_price;
			m_pAll->m_Klines.pd[nCount].low = kline.low_price;
		}
		if (m_pAll->m_Klines.pd[nCount].high < kline.high_price)
			m_pAll->m_Klines.pd[nCount].high = kline.high_price;
		if (m_pAll->m_Klines.pd[nCount].low > kline.low_price)
			m_pAll->m_Klines.pd[nCount].low = kline.low_price;
		m_pAll->m_Klines.pd[nCount].close = kline.close_price;

	}
	else
	{
		if (m_pAll->m_Klines.pd[nCount].open == 0)
		{
			m_pAll->m_Klines.pd[nCount].open = kline.open_price_r;
			m_pAll->m_Klines.pd[nCount].high = kline.high_price_r;
			m_pAll->m_Klines.pd[nCount].low = kline.low_price_r;
		}
		if (m_pAll->m_Klines.pd[nCount].high < kline.high_price_r)
			m_pAll->m_Klines.pd[nCount].high = kline.high_price_r;
		if (m_pAll->m_Klines.pd[nCount].low > kline.low_price_r)
			m_pAll->m_Klines.pd[nCount].low = kline.low_price_r;
		m_pAll->m_Klines.pd[nCount].close = kline.close_price_r;

	}
	m_pAll->m_Futu.ftl[nCount] += kline.volume;
	m_pAll->m_Klines.nLastVolume = m_pAll->m_Futu.ftl[nCount];
}

void SOUI::SKlinePic::GroupDayUpdateWithTick(int nCount, const GroupDataType & tick)
{
	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].close = tick.dLml;
		m_pAll->m_Klines.pd[nCount].open = tick.dOmo;
		m_pAll->m_Klines.pd[nCount].high = tick.dHighest;
		m_pAll->m_Klines.pd[nCount].low = tick.dLowest;
	}
	else
	{
		m_pAll->m_Klines.pd[nCount].close = tick.dLdl;
		m_pAll->m_Klines.pd[nCount].open = tick.dOdo;
		m_pAll->m_Klines.pd[nCount].high = tick.dHighestRatio;
		m_pAll->m_Klines.pd[nCount].low = tick.dLowestRatio;
	}
	if (m_pAll->m_Klines.pd[nCount].open > m_pAll->m_Klines.pd[nCount].high)
		m_pAll->m_Klines.pd[nCount].high = m_pAll->m_Klines.pd[nCount].open;
	if (m_pAll->m_Klines.pd[nCount].open < m_pAll->m_Klines.pd[nCount].low)
		m_pAll->m_Klines.pd[nCount].low = m_pAll->m_Klines.pd[nCount].open;
	m_pAll->m_Klines.pd[nCount].date = atoi(tick.TradingDay);
	m_pAll->m_Klines.pd[nCount].time = 0;
	m_pAll->m_Futu.ftl[nCount] = tick.Volume;

}

void SOUI::SKlinePic::GroupUpdateWithTick(int nCount, std::vector<GroupDataType>* pTickVec, int nPos, bool bNewLine)
{


	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].close = pTickVec->back().dLml;
		if (bNewLine)
			m_pAll->m_Klines.pd[nCount].open = pTickVec->at(nPos).dLml;
		if (pTickVec->at(nPos).dLml > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pTickVec->at(nPos).dLml;
		if (pTickVec->at(nPos).dLml < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pTickVec->at(nPos).dLml;

	}
	else
	{
		m_pAll->m_Klines.pd[nCount].close = pTickVec->back().dLdl;
		if (bNewLine)
			m_pAll->m_Klines.pd[nCount].open = pTickVec->at(nPos).dLdl;
		if (pTickVec->at(nPos).dLdl > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pTickVec->at(nPos).dLdl;
		if (pTickVec->at(nPos).dLdl < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pTickVec->at(nPos).dLdl;

	}
	m_pAll->m_Klines.pd[nCount].date = atoi(pTickVec->at(nPos).TradingDay);

	if (nPos > 0)
		m_pAll->m_Futu.ftl[nCount] = pTickVec->back().Volume - pTickVec->at(nPos - 1).Volume;
	else
		m_pAll->m_Futu.ftl[nCount] = pTickVec->back().Volume - pTickVec->at(nPos).Volume;


}

void SOUI::SKlinePic::GroupUpdateWithKline(int nCount, GroupKlineType & kline)
{
	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].close = kline.close_price;
		m_pAll->m_Klines.pd[nCount].open = kline.open_price;
		m_pAll->m_Klines.pd[nCount].high = kline.high_price;
		m_pAll->m_Klines.pd[nCount].low = kline.low_price;
	}
	else
	{
		m_pAll->m_Klines.pd[nCount].close = kline.close_price_r;
		m_pAll->m_Klines.pd[nCount].open = kline.open_price_r;
		m_pAll->m_Klines.pd[nCount].high = kline.high_price_r;
		m_pAll->m_Klines.pd[nCount].low = kline.low_price_r;
	}
	m_pAll->m_Klines.pd[nCount].date = kline.TradingDay;
	m_pAll->m_Klines.pd[nCount].time = kline.UpdateTime;
	m_pAll->m_Futu.ftl[nCount] = kline.volume;

}

void SOUI::SKlinePic::GroupUpdateWithKline(int nCount, std::vector<GroupKlineType>* pKlineVec, int nPos)
{
	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].open = pKlineVec->at(nPos).open_price;
		m_pAll->m_Klines.pd[nCount].close = pKlineVec->back().close_price;
		if (pKlineVec->at(nPos).high_price > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pKlineVec->at(nPos).high_price;
		if (pKlineVec->at(nPos).low_price < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pKlineVec->at(nPos).low_price;
	}
	else
	{
		m_pAll->m_Klines.pd[nCount].open = pKlineVec->at(nPos).open_price_r;
		m_pAll->m_Klines.pd[nCount].close = pKlineVec->back().close_price_r;
		if (pKlineVec->at(nPos).high_price_r > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pKlineVec->at(nPos).high_price_r;
		if (pKlineVec->at(nPos).low_price_r < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pKlineVec->at(nPos).low_price_r;

	}
	m_pAll->m_Klines.pd[nCount].date = pKlineVec->back().TradingDay;
	m_pAll->m_Futu.ftl[nCount] += pKlineVec->at(nPos).volume;

}

void SOUI::SKlinePic::GroupDataProc()
{
	DataInit();
	if (m_nKlineType == 5)
	{
		GroupHisDayKlineProc();
		GroupDataWithHis();
	}
	else
	{
		GroupHisKline1MinProc();
		GroupDataWithHis();
		GroupDataNoHis();
	}
	//	m_bDataInited = true;
	//	GroupDataUpdate();
	TraceLog("组合:%s K线图数据处理完成", m_strSubIns);

}

void SOUI::SKlinePic::GroupDataWithHis()
{
	bool bStart = false;
	int VecSize = m_Group1MinKline.size();
	int count = 0;
	int i = 0;
	if (m_nKlineType != 5)
	{
		for (i; i < VecSize; i++)
		{
			if (!m_bGroupIsWhole&&m_Group1MinKline[i].UpdateTime == m_nGroupTime&&m_Group1MinKline[i].TradingDay == m_nTradingDay)
				break;
			if (m_nKlineType == 0)
			{
				if (VecSize - i < 4700)		//1分钟K线
				{
					GroupToData(count, m_Group1MinKline[i]);
					count++;
					KlineMAProc(count);
				}
			}
			else if (m_nKlineType == 1)
			{
				if (VecSize - i < 4700 * 5)		//5分钟K线
				{
					int nLeft = m_Group1MinKline[i].UpdateTime % 500;
					if (!bStart&&nLeft == 0)
					{
						bStart = true;
					}
					if (bStart)
					{
						if (m_Group1MinKline[i].UpdateTime - nLeft != m_pAll->m_Klines.pd[count].time
							|| m_Group1MinKline[i].UpdateTime == 0)
						{

							if (m_bNeedAddCount)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
							GroupToData(count, m_Group1MinKline[i], nLeft);
							m_bNeedAddCount = true;
						}
						else
						{
							GroupUpdateWithHad(count, m_Group1MinKline[i]);
							if (nLeft == 400)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
						}

					}

				}

			}
			else if (m_nKlineType == 2)
			{
				if (VecSize - i < 4700 * 15)		//15分钟K线
				{
					int nLeft = (m_Group1MinKline[i].UpdateTime % 10000) / 100 % 15;
					if (!bStart&&nLeft == 0)
					{
						bStart = true;
					}

					if (bStart)
					{
						if (m_Group1MinKline[i].UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time
							|| m_Group1MinKline[i].UpdateTime == 0)
						{

							if (m_bNeedAddCount)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
							GroupToData(count, m_Group1MinKline[i], nLeft * 100);
							m_bNeedAddCount = true;
						}
						else
						{
							GroupUpdateWithHad(count, m_Group1MinKline[i]);
							if (nLeft == 14)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
						}

					}

				}

			}
			else if (m_nKlineType == 3)
			{
				if (VecSize - i < 4700 * 30)		//30分钟K线
				{
					int nLeft = (m_Group1MinKline[i].UpdateTime % 10000) / 100 % 30;
					if (!bStart && (nLeft == 0))
					{
						bStart = true;
					}

					if (bStart)
					{
						if (m_Group1MinKline[i].UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time
							|| m_Group1MinKline[i].UpdateTime == 0)
						{
							if (m_bNeedAddCount)
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
							GroupToData(count, m_Group1MinKline[i], nLeft * 100);
							m_bNeedAddCount = true;
						}
						else if (bStart)
						{
							GroupUpdateWithHad(count, m_Group1MinKline[i]);
							if (nLeft == 29
								|| (m_Group1MinKline[i].UpdateTime == 101400
									&& (StockIndex & m_pAll->nInstype) == 0 && (NationalDebt & m_pAll->nInstype) == 0)
								|| (m_Group1MinKline[i].UpdateTime == 151400
									&& (m_pAll->nInstype&NationalDebt) != 0))
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
							if (m_Group1MinKline[i].UpdateTime == 101500
								&& (StockIndex & m_pAll->nInstype) == 0 && (NationalDebt & m_pAll->nInstype) == 0)
								m_pAll->m_Klines.nLastVolume = 0;
						}

					}
				}

			}
			else if (m_nKlineType == 4)
			{
				if (VecSize - i < 4700 * 50)		//60分钟K线
				{
					int time = m_Group1MinKline[i].UpdateTime;
					int nLeft = (m_Group1MinKline[i].UpdateTime % 10000) / 100 % 60;
					if (!bStart && (nLeft == 0									//每个小时第一分钟
						|| (StockIndex == m_pAll->nInstype || NationalDebt == m_pAll->nInstype || (NationalDebt | StockIndex) == m_pAll->nInstype
							&&m_Group1MinKline[i].UpdateTime == 93000)			//股指期货上午开盘时间
						|| (StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype&&m_Group1MinKline[i].UpdateTime == 133000)))			//商品期货下午开盘时间
					{
						bStart = true;
					}

					if (bStart)
					{
						if (m_Group1MinKline[i].UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time
							|| m_Group1MinKline[i].UpdateTime == 0)
						{
							if (m_bNeedAddCount)
							{
								count++;
								KlineMAProc(count);

								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
							GroupToData(count, m_Group1MinKline[i], nLeft * 100);
							m_bNeedAddCount = true;
						}
						else if (bStart)
						{
							GroupUpdateWithHad(count, m_Group1MinKline[i]);
							if (nLeft == 59							//每个小时最后一分钟
								|| m_Group1MinKline[i].UpdateTime == 22900
								|| m_Group1MinKline[i].UpdateTime == 112900										//上午收盘时间
								|| (m_Group1MinKline[i].UpdateTime == 151400 && (m_pAll->nInstype&NationalDebt) != 0))		//国债期货下午收盘时间
							{
								count++;
								KlineMAProc(count);
								m_pAll->m_Klines.nLastVolume = 0;
								m_bNeedAddCount = false;
							}
						}
					}
				}

			}
		}
		m_pAll->nTotal = count;
		if (!m_bGroupIsWhole&&i > 0)
			m_pAll->m_Klines.nLast1MinTime = m_Group1MinKline[i - 1].UpdateTime;
		else if (!m_Group1MinKline.empty())
			m_pAll->m_Klines.nLast1MinTime = m_Group1MinKline.back().UpdateTime;
		m_pAll->m_Klines.bShow = true;
	}
	else
	{
		VecSize = m_GroupDayKline.size();
		for (i = 0; i < VecSize; i++)
		{
			if (VecSize - i < 4700)		//日K线
			{
				GroupToData(count, m_GroupDayKline[i]);
				count++;
				KlineMAProc(count);
			}

		}
		m_pAll->nTotal = count;
		m_GroupDayKline.clear();
		m_GroupDayKline.shrink_to_fit();

	}
	TraceLog("组合:%s K线图历史数据处理完成", m_strSubIns);
}

void SOUI::SKlinePic::GroupDataNoHis()
{
	if (m_nKlineType == 5)
		return;
	if (g_GroupKlineHash[m_strSubIns].empty())
	{
		if (m_bNeedAddCount)
		{
			m_pAll->nTotal++;
			KlineMAProc(m_pAll->nTotal);
			m_bNeedAddCount = false;
		}
		if (!m_Group1MinKline.empty())
			m_Last1MinHisData = m_Group1MinKline.back();
		m_Group1MinKline.clear();
		m_Group1MinKline.shrink_to_fit();

		return;
	}

	int count = m_pAll->nTotal;

	::EnterCriticalSection(&g_csGroupKline);
	//对登录时刻的k线进行处理
	if (!g_Group1MinKlineChangedMap[m_strSubIns])
	{
		if (!m_Group1MinKline.empty())
		{
			if (m_nGroupTime == g_GroupKlineHash[m_strSubIns].front().UpdateTime && !g_Group1MinKlineChangedMap[m_strSubIns])
			{

				GroupFirstKlineProc(m_strSubIns, m_bGroupIsWhole, m_Group1MinKline.back());

				g_Group1MinKlineChangedMap[m_strSubIns] = true;
			}

			if (!g_Group1MinKlineChangedMap[m_strSubIns])
			{
				int nHisKlineCount = m_Group1MinKline.size();
				for (int j = nHisKlineCount - 2; j >= 0; j--)
				{
					if (m_Group1MinKline[j].UpdateTime > g_GroupKlineHash[m_strSubIns].front().UpdateTime)
						continue;
					if (m_Group1MinKline[j].TradingDay != g_GroupKlineHash[m_strSubIns].front().TradingDay)
						break;
					if (m_Group1MinKline[j].UpdateTime == g_GroupKlineHash[m_strSubIns].front().UpdateTime)
					{
						GroupFirstKlineProc(m_strSubIns, true, j);
						break;
					}
				}
			}
			if (!m_Group1MinKline.empty())
				m_Last1MinHisData = m_Group1MinKline.back();
		}
		g_Group1MinKlineChangedMap[m_strSubIns] = true;
	}

	m_Group1MinKline.clear();
	m_Group1MinKline.shrink_to_fit();

	std::vector<GroupKlineType> KlineVec = g_GroupKlineHash[m_strSubIns];

	::LeaveCriticalSection(&g_csGroupKline);

	bool bBeginIsFirst = false;
	if (IsFirstLine(KlineVec.cbegin()->UpdateTime) && m_nCAType == CA_Show)
		bBeginIsFirst = true;


	for (auto iter = KlineVec.begin(); iter != KlineVec.end(); iter++)
	{
		//1分钟K线
		if (m_nKlineType == 0)
		{
			if (iter->UpdateTime > m_pAll->m_Klines.pd[count - 1].time || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
			{

				GroupToData(count, *iter);
				if (bBeginIsFirst)
				{
					bBeginIsFirst = false;
					if (iter == KlineVec.begin())
						GroupCAInfoProc(count);
				}
				count++;
				m_pAll->nTotal++;
				KlineMAProc(count);
			}

		}
		else
		{
			if (m_nPeriod > 1 && m_nPeriod <= 60)
			{
				if (iter->UpdateTime >= m_pAll->m_Klines.pd[count - 1].time + (m_nPeriod - 1) * 100 || iter->TradingDay > m_pAll->m_Klines.pd[count - 1].date)
				{
					int nLeft = (iter->UpdateTime % 10000) / 100 % m_nPeriod;
					if (iter->UpdateTime - nLeft * 100 != m_pAll->m_Klines.pd[count].time)
					{
						if (m_bNeedAddCount)
						{
							count++;
							m_pAll->nTotal++;
							KlineMAProc(count);
							m_pAll->m_Klines.nLastVolume = 0;
							m_bNeedAddCount = false;
						}
						GroupToData(count, *iter, nLeft * 100);
						if (bBeginIsFirst)
						{
							bBeginIsFirst = false;
							if (iter == KlineVec.begin())
								GroupCAInfoProc(count);
						}
						m_bNeedAddCount = true;
					}
					else
					{
						GroupUpdateWithHad(count, *iter);
						if (nLeft == m_nPeriod - 1
							|| (iter->UpdateTime == 101400 && (StockIndex&m_pAll->nInstype) == 0 && (NationalDebt & m_pAll->nInstype) == 0 && m_nPeriod != 60)
							|| (iter->UpdateTime == 151400 && (m_pAll->nInstype&NationalDebt) != 0)
							|| iter->UpdateTime == 112900)

						{
							count++;
							m_pAll->m_Klines.nLastVolume = 0;
							m_pAll->nTotal++;
							KlineMAProc(count);
							m_bNeedAddCount = false;
						}
					}
					m_pAll->m_Klines.nLast1MinTime = iter->UpdateTime;
				}

			}

		}

	}
	if (m_bNeedAddCount)
	{
		m_pAll->nTotal++;
		KlineMAProc(m_pAll->nTotal);
	}

	TraceLog("组合:%s K线图非历史数据处理完成", m_strSubIns);

}

void SOUI::SKlinePic::GroupDataUpdate()
{

	{
		if (m_nKlineType == 0)
			Group1MinUpdate();
		else if (m_nKlineType < 5)
			GroupMultMinUpdate(m_nPeriod);
		else
			GroupDayUpdate();
	}
}

void SOUI::SKlinePic::Group1MinUpdate()
{

	std::vector<GroupKlineType>* pKlineVec = &g_GroupKlineHash[m_strSubIns];
	std::vector<GroupDataType>* pTickVec = &g_GroupTickHash[m_strSubIns];
	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csGroupTick);

	if (pKlineVec->empty())		//没有接收过实时K线
	{
		const int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| (ntime < 90000))
		{
			::LeaveCriticalSection(&g_csGroupTick);
			return;
		}
		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
		{
			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				const int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || ntime < 90000 || (ntime < 93000
					&& (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt || m_pAll->nInstype == (StockIndex | NationalDebt))))
					break;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time&& atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime == 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateNoKline(nDataCount, i, pTickVec, ntime, true);

			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
				GroupCAInfoProc(nDataCount);

			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}
		}
		else if (ntime == m_pAll->m_Klines.pd[nDataCount - 1].time)
		{
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				const int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || (ntime < 93000
					&& (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt || m_pAll->nInstype == (StockIndex | NationalDebt))))
					break;
				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time&&ntime != 85900)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateNoKline(nDataCount - 1, i, pTickVec, ntime, false);
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
				GroupCAInfoProc(nDataCount - 1);

			KlineMAProc(m_pAll->nTotal);
		}
	}
	else							//接收过实时K线
	{
		::EnterCriticalSection(&g_csGroupKline);
		if (!g_Group1MinKlineChangedMap[m_strSubIns])
		{
			if (m_Last1MinHisData.TradingDay != 0)
			{
				if (pKlineVec->back().UpdateTime == m_Last1MinHisData.UpdateTime)
				{

					GroupFirstKlineProc(m_strSubIns, m_bGroupIsWhole, m_Last1MinHisData);

					g_Group1MinKlineChangedMap[m_strSubIns] = true;
				}
				else if (pKlineVec->back().UpdateTime > m_Last1MinHisData.UpdateTime || pKlineVec->back().TradingDay> m_Last1MinHisData.TradingDay)
					g_Group1MinKlineChangedMap[m_strSubIns] = true;

			}
			else
				g_Group1MinKlineChangedMap[m_strSubIns] = true;
		}

		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time)			//新的时间大于最后一条K线的时间，直接添加
		{
			if (pKlineVec->back().UpdateTime >= 150000 && m_pAll->nInstype != NationalDebt || pKlineVec->back().UpdateTime >= 151500)
			{
				::LeaveCriticalSection(&g_csGroupKline);
				::LeaveCriticalSection(&g_csGroupTick);
				return;
			}
			GroupUpdateWithKline(nDataCount, pKlineVec->back());
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
				GroupCAInfoProc(nDataCount);
			nDataCount++;
			m_pAll->nTotal++;
			KlineMAProc(m_pAll->nTotal);
		}
		else if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.pd[nDataCount - 1].time)	//新的时间等于最后一条K线的时间，先覆盖，在添加
		{
			//			OutputDebugString(L"Group1Min:情况4\n");
			//判断本条K线时间是否和历史K线的最后一条时间相等

			GroupUpdateWithKline(nDataCount - 1, pKlineVec->back());
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
				GroupCAInfoProc(nDataCount - 1);
			KlineMAProc(m_pAll->nTotal);

			if ((m_pAll->m_Klines.pd[nDataCount - 1].time == 145900 && m_pAll->nInstype != NationalDebt) || m_pAll->m_Klines.pd[nDataCount - 1].time == 151400)
			{
				::LeaveCriticalSection(&g_csGroupKline);
				::LeaveCriticalSection(&g_csGroupTick);
				return;
			}

			m_pAll->m_Klines.pd[nDataCount].high = -100000000;
			m_pAll->m_Klines.pd[nDataCount].low = 1000000000;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				const int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
				{
					::LeaveCriticalSection(&g_csGroupKline);
					::LeaveCriticalSection(&g_csGroupTick);
					return;
				}
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateWithTick(nDataCount, pTickVec, i);
				m_pAll->m_Klines.pd[nDataCount].time = ntime;
			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
			}

		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				const int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500 || (ntime < 93000
					&& (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt || m_pAll->nInstype == (StockIndex | NationalDebt))))
					break;
				if (ntime <= m_pAll->m_Klines.pd[nDataCount - 2].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateWithTick(nDataCount - 1, pTickVec, i);
				m_pAll->m_Klines.pd[nDataCount - 1].time = ntime;
			}
			if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
				GroupCAInfoProc(nDataCount - 1);
			KlineMAProc(m_pAll->nTotal);
		}
		::LeaveCriticalSection(&g_csGroupKline);
	}
	::LeaveCriticalSection(&g_csGroupTick);
}

void SOUI::SKlinePic::GroupMultMinUpdate(int nPeriod)
{

	std::vector<GroupKlineType>* pKlineVec = &g_GroupKlineHash[m_strSubIns];
	std::vector<GroupDataType>* pTickVec = &g_GroupTickHash[m_strSubIns];
	int nDataCount = m_pAll->nTotal;

	::EnterCriticalSection(&g_csGroupTick);

	if (pKlineVec->empty())		//没有接收过实时K线
	{
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (ntime >= 151500
			|| (101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
			|| ntime == 113000
			|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype)
			|| ntime < 90000)
		{
			::LeaveCriticalSection(&g_csGroupTick);
			return;
		}
		if (nDataCount == 0 || ntime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100 || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date) //新的一条多分钟K线
		{
			ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (0 != nDataCount && (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100 || atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date))
			{
				m_pAll->m_Klines.pd[nDataCount].high = -100000000;
				m_pAll->m_Klines.pd[nDataCount].low = 1000000000;
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime >= 151500 || ntime < 85900 || (ntime < 93000
						&& (m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt || m_pAll->nInstype == (StockIndex | NationalDebt))))
						break;
					if (ntime <= m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100 && atoi(pTickVec->back().TradingDay) == m_pAll->m_Klines.pd[nDataCount - 1].date)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					GroupUpdateNoKline(nDataCount, i, pTickVec, ntime, true);
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
				}

				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
					GroupCAInfoProc(nDataCount);


				if (m_pAll->m_Klines.pd[nDataCount].close != 0)
				{
					m_pAll->nTotal++;
					KlineMAProc(m_pAll->nTotal);
				}
			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].high = -10000000;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
				}

				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (0 == nDataCount || atoi(pTickVec->back().TradingDay) > m_pAll->m_Klines.pd[nDataCount - 1].date)
					{
						if (ntime < 90000 || ntime>151500)
							break;
					}
					else if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					GroupUpdateNoKline(nDataCount, i, pTickVec, ntime, true, bNewKline);
					if (bNewKline)
					{
						m_pAll->m_Klines.pd[nDataCount].time = ntime;

						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}
				}
				if (!m_bHandleTdyFirstLine)
				{
					if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount].time) && m_nCAType == CA_Show)
						GroupCAInfoProc(nDataCount);
				}

				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);

			}
		}
		else if (ntime >= m_pAll->m_Klines.pd[nDataCount - 1].time) //最后一条K线并不完整
		{

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime >= 151500)
					break;
				if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateWithTick(nDataCount - 1, pTickVec, i, false);
			}
			if (!m_bHandleTdyFirstLine)
			{
				if (IsFirstLine(m_pAll->m_Klines.pd[nDataCount - 1].time) && m_nCAType == CA_Show)
					GroupCAInfoProc(nDataCount - 1);
			}
			KlineMAProc(m_pAll->nTotal);

		}
	}
	else							//接收过实时K线
	{
		::EnterCriticalSection(&g_csGroupKline);
		int ntime = GetTime(pTickVec->back().UpdateTime) / 1000;
		if (!g_Group1MinKlineChangedMap[m_strSubIns])
		{
			if (m_Last1MinHisData.TradingDay != 0)
			{
				if (pKlineVec->back().UpdateTime == m_Last1MinHisData.UpdateTime)
				{
					GroupFirstKlineProc(m_strSubIns, m_bGroupIsWhole, m_Last1MinHisData);
					g_Group1MinKlineChangedMap[m_strSubIns] = true;
				}
				else if (pKlineVec->back().UpdateTime > m_Last1MinHisData.UpdateTime || pKlineVec->back().TradingDay > m_Last1MinHisData.TradingDay)
					g_Group1MinKlineChangedMap[m_strSubIns] = true;
			}
			else
				g_Group1MinKlineChangedMap[m_strSubIns] = true;

		}
		if (pKlineVec->back().UpdateTime > m_pAll->m_Klines.pd[nDataCount - 1].time + (nPeriod - 1) * 100)			//新的时间大于最后一条K线的时间，直接添加 
		{
			if (pKlineVec->back().UpdateTime >= 150000 && m_pAll->nInstype != NationalDebt || pKlineVec->back().UpdateTime >= 151500)
			{
				::LeaveCriticalSection(&g_csGroupKline);
				::LeaveCriticalSection(&g_csGroupTick);
				return;
			}

			int ntime = GetTime(pTickVec->front().UpdateTime) / 1000;
			if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)	//最早一条tick时间在下一条K线之前
			{
				m_pAll->m_Klines.pd[nDataCount].high = -10000000;
				m_pAll->m_Klines.pd[nDataCount].low = 100000000;

				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime < m_pAll->m_Klines.pd[nDataCount - 1].time + nPeriod * 100)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					GroupUpdateWithTick(nDataCount, pTickVec, i);
					m_pAll->m_Klines.pd[nDataCount].time = ntime;
					int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
					if (0 != nLeft)
						m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
				}

				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
					GroupCAInfoProc(nDataCount);

				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//		TraceLog("subIns:%d  period:%d  组合1处新增一条K线", m_nSubIns, nPeriod);
			}
			else
			{
				bool bNewKline = false;
				if (0 == m_pAll->m_Klines.pd[nDataCount].open)
				{
					bNewKline = true;
					m_pAll->m_Klines.pd[nDataCount].high = -10000000;
					m_pAll->m_Klines.pd[nDataCount].low = 100000000;
				}
				for (int i = pTickVec->size() - 1; i >= 0; i--)
				{
					ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
					if (ntime <= m_pAll->m_Klines.nLast1MinTime)
						break;
					if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
						|| ntime == 113000
						|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
						break;
					GroupUpdateWithTick(nDataCount, pTickVec, i, bNewKline);
					if (bNewKline)
					{
						m_pAll->m_Klines.pd[nDataCount].time = ntime;
						int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
						if (0 != nLeft)
							m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;
					}
				}
				m_pAll->m_Futu.ftl[nDataCount] += m_pAll->m_Klines.nLastVolume;
				if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
					GroupCAInfoProc(nDataCount);
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//			TraceLog("subIns:%d  period:%d  组合2处新增一条K线", m_nSubIns, nPeriod);
			}

		}
		else if (TimeIsLastBar(pKlineVec->back().UpdateTime, m_pAll->m_Klines.pd[nDataCount - 1].time, ntime, nPeriod))//新的时间等于最后一条K线的时间，先覆盖，在添加
		{

			if (pKlineVec->back().UpdateTime == m_pAll->m_Klines.nLast1MinTime && (ntime == 113000 || ntime == 112900 ||
				ntime == 150000 || ntime == 151500 || ntime == 101400 || ntime == 101500))
			{
				::LeaveCriticalSection(&g_csGroupKline);
				::LeaveCriticalSection(&g_csGroupTick);
				return;
			}

			if (!((pKlineVec->back().UpdateTime == 101400 && ntime > 101500 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)		//不将交易暂停时刻的K线重复修改，导致数量错误
				|| (pKlineVec->back().UpdateTime == 112900 && ntime > 113000)))

			{
				if (pKlineVec->front().UpdateTime <= m_pAll->m_Klines.pd[nDataCount - 1].time)
				{

					m_pAll->m_Futu.ftl[nDataCount - 1] = 0;
					for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
					{
						if (pKlineVec->at(i).UpdateTime < m_pAll->m_Klines.pd[nDataCount - 1].time)
							break;
						GroupUpdateWithKline(nDataCount - 1, pKlineVec, i);
					}
				}
				else
				{

					m_pAll->m_Futu.ftl[nDataCount - 1] = m_pAll->m_Klines.nLastVolume;
					for (int i = pKlineVec->size() - 1; i >= 0; i--)		//修改最后一条K线
					{
						if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
							break;
						GroupUpdateWithKline(nDataCount - 1, pKlineVec, i);

					}
				}

				m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;
				m_pAll->m_Klines.nLastVolume = 0;

				KlineMAProc(m_pAll->nTotal);
			}

			//新增一条K线

			m_pAll->m_Klines.pd[nDataCount].high = -10000000;
			m_pAll->m_Klines.pd[nDataCount].low = 100000000;
			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateWithTick(nDataCount, pTickVec, i);

				m_pAll->m_Klines.pd[nDataCount].time = ntime;
				int nLeft = (m_pAll->m_Klines.pd[nDataCount].time % 10000) / 100 % nPeriod;
				if (0 != nLeft)
					m_pAll->m_Klines.pd[nDataCount].time -= nLeft * 100;

			}
			if (m_pAll->m_Klines.pd[nDataCount].close != 0)
			{
				m_pAll->nTotal++;
				KlineMAProc(m_pAll->nTotal);
				//			TraceLog("subIns:%d  period:%d  组合3处新增一条K线", m_nSubIns, nPeriod);
			}
		}
		else																	//新的时间小于最后一条K线的时间，修改最后一条
		{
			for (int i = pKlineVec->size() - 1; i >= 0; i--)
			{
				if (pKlineVec->at(i).UpdateTime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				m_pAll->m_Klines.nLastVolume += pKlineVec->at(i).volume;
				if (IsFirstLine(pKlineVec->at(i).UpdateTime) && m_nCAType == CA_Show)
					m_pAll->m_Klines.nLastVolume += GroupCAInfoProc(0, false, false);
			}
			m_pAll->m_Klines.nLast1MinTime = pKlineVec->back().UpdateTime;

			for (int i = pTickVec->size() - 1; i >= 0; i--)
			{
				int ntime = GetTime(pTickVec->at(i).UpdateTime) / 1000;
				if (ntime <= m_pAll->m_Klines.nLast1MinTime)
					break;
				if ((101500 == ntime&& StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype)
					|| ntime == 113000
					|| (ntime >= 150000 && NationalDebt != m_pAll->nInstype))
					break;
				GroupUpdateWithTick(nDataCount - 1, pTickVec, i, false);
			}
			if (IsFirstLine(pKlineVec->back().UpdateTime) && m_nCAType == CA_Show)
				m_pAll->m_Klines.nLastVolume += GroupCAInfoProc(nDataCount - 1, false, true);

			m_pAll->m_Futu.ftl[nDataCount - 1] += m_pAll->m_Klines.nLastVolume;
			KlineMAProc(m_pAll->nTotal);

		}


		::LeaveCriticalSection(&g_csGroupKline);
	}

	::LeaveCriticalSection(&g_csGroupTick);
}

void SOUI::SKlinePic::GroupDayUpdate()
{

	std::vector<GroupDataType>* pTickVec = &g_GroupTickHash[m_strSubIns];
	::EnterCriticalSection(&g_csGroupTick);
	int nDay = atoi(pTickVec->back().TradingDay);
	int nDataCount = m_pAll->nTotal;
	if (pTickVec->back().dOmo > 10000000)
	{
		::LeaveCriticalSection(&g_csGroupTick);
		return;
	}

	if (nDataCount == 0 || nDay > m_pAll->m_Klines.pd[nDataCount - 1].date)
	{
		GroupDayUpdateWithTick(nDataCount, pTickVec->back());
		m_pAll->nTotal++;
		KlineMAProc(m_pAll->nTotal);
	}
	else
	{
		GroupDayUpdateWithTick(nDataCount - 1, pTickVec->back());
		KlineMAProc(m_pAll->nTotal);
	}
	::LeaveCriticalSection(&g_csGroupTick);
}

void SOUI::SKlinePic::GroupFirstKlineProc(InsIDType GroupInsID, bool bHisIsWhole, int nPos)
{
	if (bHisIsWhole)
	{
		g_GroupKlineHash[GroupInsID].front().open_price = m_Group1MinKline[nPos].open_price;
		g_GroupKlineHash[GroupInsID].front().high_price = m_Group1MinKline[nPos].high_price;
		g_GroupKlineHash[GroupInsID].front().low_price = m_Group1MinKline[nPos].low_price;
		g_GroupKlineHash[GroupInsID].front().close_price = m_Group1MinKline[nPos].close_price;
		g_GroupKlineHash[GroupInsID].front().open_price_r = m_Group1MinKline[nPos].open_price_r;
		g_GroupKlineHash[GroupInsID].front().high_price_r = m_Group1MinKline[nPos].high_price_r;
		g_GroupKlineHash[GroupInsID].front().low_price_r = m_Group1MinKline[nPos].low_price_r;
		g_GroupKlineHash[GroupInsID].front().close_price_r = m_Group1MinKline[nPos].close_price_r;
		g_GroupKlineHash[GroupInsID].front().volume = m_Group1MinKline[nPos].volume;

	}
	else
	{
		g_GroupKlineHash[GroupInsID].front().open_price = m_Group1MinKline[nPos].open_price;
		if (m_Group1MinKline[nPos].high_price > g_GroupKlineHash[GroupInsID].front().high_price)
			g_GroupKlineHash[GroupInsID].front().high_price = m_Group1MinKline[nPos].high_price;
		if (m_Group1MinKline[nPos].low_price < g_GroupKlineHash[GroupInsID].front().low_price)
			g_GroupKlineHash[GroupInsID].front().low_price = m_Group1MinKline[nPos].low_price;
		g_GroupKlineHash[GroupInsID].front().open_price_r = m_Group1MinKline[nPos].open_price_r;
		if (m_Group1MinKline[nPos].high_price_r > g_GroupKlineHash[GroupInsID].front().high_price_r)
			g_GroupKlineHash[GroupInsID].front().high_price_r = m_Group1MinKline[nPos].high_price_r;
		if (m_Group1MinKline[nPos].low_price_r < g_GroupKlineHash[GroupInsID].front().low_price_r)
			g_GroupKlineHash[GroupInsID].front().low_price_r = m_Group1MinKline[nPos].low_price_r;
		g_GroupKlineHash[GroupInsID].front().volume -= m_nGroupVolume;
	}

}

void SOUI::SKlinePic::GroupFirstKlineProc(InsIDType GroupInsID, bool bHisIsWhole, const GroupKlineType & KlineData)
{
	if (bHisIsWhole)
	{
		g_GroupKlineHash[GroupInsID].front().open_price = KlineData.open_price;
		g_GroupKlineHash[GroupInsID].front().high_price = KlineData.high_price;
		g_GroupKlineHash[GroupInsID].front().low_price = KlineData.low_price;
		g_GroupKlineHash[GroupInsID].front().close_price = KlineData.close_price;
		g_GroupKlineHash[GroupInsID].front().open_price_r = KlineData.open_price_r;
		g_GroupKlineHash[GroupInsID].front().high_price_r = KlineData.high_price_r;
		g_GroupKlineHash[GroupInsID].front().low_price_r = KlineData.low_price_r;
		g_GroupKlineHash[GroupInsID].front().close_price_r = KlineData.close_price_r;
		g_GroupKlineHash[GroupInsID].front().volume = KlineData.volume;

	}
	else
	{
		g_GroupKlineHash[GroupInsID].front().open_price = KlineData.open_price;
		if (KlineData.high_price > g_GroupKlineHash[GroupInsID].front().high_price)
			g_GroupKlineHash[GroupInsID].front().high_price = KlineData.high_price;
		if (KlineData.low_price < g_GroupKlineHash[GroupInsID].front().low_price)
			g_GroupKlineHash[GroupInsID].front().low_price = KlineData.low_price;
		g_GroupKlineHash[GroupInsID].front().open_price_r = KlineData.open_price_r;
		if (KlineData.high_price_r > g_GroupKlineHash[GroupInsID].front().high_price_r)
			g_GroupKlineHash[GroupInsID].front().high_price_r = KlineData.high_price_r;
		if (KlineData.low_price_r < g_GroupKlineHash[GroupInsID].front().low_price_r)
			g_GroupKlineHash[GroupInsID].front().low_price_r = KlineData.low_price_r;
		g_GroupKlineHash[GroupInsID].front().volume -= m_nGroupVolume;
	}

}

int SOUI::SKlinePic::GroupCAInfoProc(int nCount, bool bHandleVolume, bool bHandlePrice)
{
	InsIDType InsID1 = g_GroupInsMap[m_strSubIns].Ins1;
	InsIDType InsID2 = g_GroupInsMap[m_strSubIns].Ins2;
	::EnterCriticalSection(&g_csCAInfo);
	if (g_CADataMap.find(InsID1) != g_CADataMap.end()
		&& g_CADataMap.find(InsID2) != g_CADataMap.end())
	{
		CallAutionData_t Ins1CaData = g_CADataMap[InsID1];
		CallAutionData_t Ins2CaData = g_CADataMap[InsID2];
		::LeaveCriticalSection(&g_csCAInfo);
		if (bHandlePrice)
		{
			if (*m_pGroupDataType == 0)
				m_pAll->m_Klines.pd[nCount].open = Ins1CaData.fOpenPrice*g_GroupInsMap[m_strSubIns].Ins1Ratio - Ins2CaData.fOpenPrice*g_GroupInsMap[m_strSubIns].Ins1Ratio;
			else
				m_pAll->m_Klines.pd[nCount].open = Ins1CaData.fOpenPrice*g_GroupInsMap[m_strSubIns].Ins1Ratio / Ins2CaData.fOpenPrice*g_GroupInsMap[m_strSubIns].Ins1Ratio*RATIOCOE;

			if (m_pAll->m_Klines.pd[nCount].high < m_pAll->m_Klines.pd[nCount].open)
				m_pAll->m_Klines.pd[nCount].high = m_pAll->m_Klines.pd[nCount].open;
			if (m_pAll->m_Klines.pd[nCount].low > m_pAll->m_Klines.pd[nCount].open)
				m_pAll->m_Klines.pd[nCount].low = m_pAll->m_Klines.pd[nCount].open;

		}
		if (bHandleVolume)
			m_pAll->m_Futu.ftl[nCount] += Ins1CaData.nVolume - Ins2CaData.nVolume;
		return Ins1CaData.nVolume - Ins2CaData.nVolume;
	}
	::LeaveCriticalSection(&g_csCAInfo);
	return 0;
}

void SOUI::SKlinePic::GroupHisKline1MinProc()
{
	InsIDType InsID1 = g_GroupInsMap[m_strSubIns].Ins1;
	InsIDType InsID2 = g_GroupInsMap[m_strSubIns].Ins2;
	int nRatio1 = g_GroupInsMap[m_strSubIns].Ins1Ratio;
	int nRatio2 = g_GroupInsMap[m_strSubIns].Ins2Ratio;
	bool bIns1Hot = false;
	m_nGroupTime = -1;
	::EnterCriticalSection(&g_csTick);

	//初始化日期信息
	int nToday;
	long long Ins1OpenInterest = 0, Ins2OpenInterest = 0;
	if (isalpha(InsID1[0]))
	{
		Ins1OpenInterest = g_TickHash[InsID1].back().OpenInterest;
		nToday = atoi(g_TickHash[InsID1].back().TradingDay);
	}
	else
	{
		Ins1OpenInterest = 0;
		nToday = atoi(g_StockIndexTickHash[InsID1].back().TradingDay);
	}

	if (isalpha(InsID2[0]))
		Ins2OpenInterest = g_TickHash[InsID2].back().OpenInterest;
	else
		Ins2OpenInterest = 0;
	::LeaveCriticalSection(&g_csTick);

	m_bHandleTdyFirstLine = true;
	if (m_nCAType == CA_Show)
		m_bHandleTdyFirstLine = false;

	if (Ins1OpenInterest >= Ins2OpenInterest)
		bIns1Hot = true;
	else
		bIns1Hot = false;
	std::vector<HisTickType> recVec;
	std::vector<HisTickType> tmpTickVec;
	std::vector<KLineDataType> klineVec;
	std::vector<KLineDataType> tmpKlineVec;
	m_nTradingDay = nToday;
	m_Group1MinKline.clear();
	if (GetFileKlineData(InsID1, &klineVec) && GetFileKlineData(InsID2, &tmpKlineVec))
	{
		unsigned i = 0, j = 0;
		GroupKlineType dfs;
		KLineDataType kl1;
		KLineDataType kl2;
		for (i; i < klineVec.size(); i++)
		{

			if (IsCATime(klineVec[i].UpdateTime, klineVec[i].TradingDay))
			{
				if (m_nCAType == CA_Show)
				{
					AddCADataToFirstLine(klineVec[i], klineVec[i + 1]);
					continue;
				}
				else if (m_nCAType == CA_Hide)
					continue;
			}

			int date = klineVec[i].TradingDay;
			if (date == nToday&&klineVec[i].UpdateTime >= 90000)//今天早上开盘后的数据不进行处理
				break;
			for (; j < tmpKlineVec.size(); j++)
			{
				if (IsCATime(tmpKlineVec[j].UpdateTime, tmpKlineVec[j].TradingDay))
				{
					if (m_nCAType == CA_Show)
					{
						AddCADataToFirstLine(tmpKlineVec[j], tmpKlineVec[j + 1]);
						continue;
					}
					else if (m_nCAType == CA_Hide)
						continue;
				}

				if (klineVec[i].TradingDay > tmpKlineVec[j].TradingDay)
					continue;
				else if (klineVec[i].TradingDay < tmpKlineVec[j].TradingDay)
					break;


				if (klineVec[i].UpdateTime > tmpKlineVec[j].UpdateTime)
					continue;
				if (klineVec[i].UpdateTime == tmpKlineVec[j].UpdateTime)		//两者时间相等
				{
					kl1 = klineVec[i];
					kl2 = tmpKlineVec[j];
				}
				else
				{
					kl1 = klineVec[i];
					kl2 = tmpKlineVec[j - 1];
				}
				ZeroMemory(&dfs, sizeof(dfs));
				dfs.TradingDay = kl1.TradingDay;
				dfs.UpdateTime = kl1.UpdateTime;
				double tmpHigh = 0, tmpLow = 0, tmpHigh_r = 0, tmpLow_r = 0;
				dfs.open_price = nRatio1*kl1.open_price - nRatio2*kl2.open_price;
				dfs.close_price = nRatio1*kl1.close_price - nRatio2*kl2.close_price;
				tmpHigh = dfs.open_price;
				tmpLow = dfs.close_price;
				dfs.open_price_r = (nRatio1*kl1.open_price) / (nRatio2*kl2.open_price)*RATIOCOE;
				dfs.close_price_r = (nRatio1*kl1.close_price) /( nRatio2*kl2.close_price)*RATIOCOE;
				tmpHigh_r = dfs.open_price_r;
				tmpLow_r = dfs.close_price_r;
				if (tmpHigh < tmpLow)
				{
					dfs.high_price = tmpLow;
					dfs.low_price = tmpHigh;
				}
				else
				{
					dfs.high_price = tmpHigh;
					dfs.low_price = tmpLow;
				}
				if (tmpHigh_r < tmpLow_r)
				{
					dfs.high_price_r = tmpLow_r;
					dfs.low_price_r = tmpHigh_r;
				}
				else
				{
					dfs.high_price_r = tmpHigh_r;
					dfs.low_price_r = tmpLow_r;
				}

				dfs.volume = kl1.volume - kl2.volume;

				if (!std::isinf(dfs.open_price_r) && !std::isinf(dfs.high_price_r) && !std::isinf(dfs.low_price_r) && !std::isinf(dfs.close_price_r))
					m_Group1MinKline.push_back(dfs);

				break;
			}
		}
		m_bGroupIsWhole = true;
	}
	if (bIns1Hot ? GetFileTickData(InsID1, &recVec, InsID2, &tmpTickVec) :
		GetFileTickData(InsID2, &recVec, InsID1, &tmpTickVec))			//获取今日的K线
	{
		m_bGroupIsWhole = false;
		GroupKlineType dfs;
		ZeroMemory(&dfs, sizeof(dfs));
		dfs.high_price = -100000000;
		dfs.low_price = 100000000;
		dfs.high_price_r = -100000000;
		dfs.low_price_r = 100000000;

		int date;
		if (isalpha(InsID1[0]))
			date = atoi(g_TickHash[InsID1].back().TradingDay);
		else
			date = atoi(g_StockIndexTickHash[InsID1].back().TradingDay);

		unsigned i, j = 0;
		int nOffset = 0;
		int Ins1LastVolume = 0, Ins2LastVolume = 0;
		if ((m_pAll->nInstype&StockIndex) == 0 && (m_pAll->nInstype&NationalDebt) == 0)
			nOffset = 32400;
		else
			nOffset = 34200;
		for (i = 0; i < recVec.size(); i++)
		{
			int Ins1Time = GetTime(recVec[i].UpdateTime) / 1000;
			int Ins1Offset = GetTimeOffset(recVec[i].UpdateTime);
			int Ins2Time = GetTime(tmpTickVec[j].UpdateTime) / 1000;
			int Ins2Offset = GetTimeOffset(tmpTickVec[j].UpdateTime);

			//			if (Ins1Time == 85900)
			//				Ins1Offset = 32400;
			if (Ins1Time == 101500
				&& (StockIndex &m_pAll->nInstype) == 0
				&& (NationalDebt&m_pAll->nInstype) == 0)
			{
				Ins1Time = 101400;
				Ins1Offset = 36840;
			}
			else if (Ins1Time == 113000)
			{
				Ins1Time = 112900;
				Ins1Offset = 41340;
			}

			if ((Ins1Time > 150000 && Ins1Time < 170000 && (NationalDebt&m_pAll->nInstype) == 0) || (Ins1Time > 151500 && Ins1Time < 170000))
				break;


			if (Ins1Time < 90000 || Ins1Time>170000
				|| (Ins1Time < 93000 && (m_pAll->nInstype == StockIndex
					|| m_pAll->nInstype == NationalDebt
					|| m_pAll->nInstype == (NationalDebt | StockIndex))))
			{
				Ins1LastVolume = recVec[i].Volume;
				continue;
			}
			while (Ins2Time < 90000 || Ins2Time>170000
				|| (Ins2Time < 93000 && (m_pAll->nInstype == StockIndex
					|| m_pAll->nInstype == NationalDebt
					|| m_pAll->nInstype == (NationalDebt | StockIndex))))
			{
				j++;
				Ins2LastVolume = tmpTickVec[j - 1].Volume;
				Ins2Time = GetTime(tmpTickVec[j].UpdateTime) / 1000;
				Ins2Offset = GetTimeOffset(tmpTickVec[j].UpdateTime);
			}

			//if (Ins2Time == 85900)
			//	Ins2Offset = 32400;
			if (Ins2Time == 101500
				&& (StockIndex &m_pAll->nInstype) == 0
				&& (NationalDebt&m_pAll->nInstype) == 0)
			{
				Ins2Time = 101400;
				Ins2Offset = 36840;
			}
			else if (Ins2Time == 113000)
			{
				Ins2Time = 112900;
				Ins2Offset = 41340;
			}
			if (Ins2Time > 150000 && (m_pAll->nInstype& NationalDebt) == 0 || Ins1Time > 151500)
				break;


			if (((Ins1Time == 90000 && m_pAll->nInstype == ComNoNight)
				|| (Ins1Time == 93000
					&& (m_pAll->nInstype == StockIndex
						|| m_pAll->nInstype == NationalDebt
						|| m_pAll->nInstype == (NationalDebt | StockIndex))))
				&& dfs.open_price == 0)
			{
				m_bHandleTdyFirstLine = true;
				double Ins1OpenPrice = 0, Ins2OpenPrice = 0;
				if (m_nCAType == CA_Show)
				{
					if (isalpha(InsID1[0]))
						Ins1OpenPrice = g_TickHash[InsID1].back().OpenPrice;
					else
						Ins1OpenPrice = g_StockIndexTickHash[InsID1].back().OpenPrice;

					if (isalpha(InsID2[0]))
						Ins2OpenPrice = g_TickHash[InsID2].back().OpenPrice;
					else
						Ins2OpenPrice = g_StockIndexTickHash[InsID2].back().OpenPrice;
					Ins1LastVolume = 0;
					Ins2LastVolume = 0;
				}
				else if (m_nCAType == CA_Hide)
				{
					if (bIns1Hot)
					{
						Ins1OpenPrice = recVec[i].LastPrice;
						Ins2OpenPrice = tmpTickVec[j].LastPrice;
					}
					else
					{
						Ins2OpenPrice = recVec[i].LastPrice;
						Ins1OpenPrice = tmpTickVec[j].LastPrice;
					}
				}

				dfs.open_price = nRatio1*Ins1OpenPrice -
					nRatio2*Ins2OpenPrice;
				dfs.open_price_r = (nRatio1*Ins1OpenPrice) /
					(nRatio2*Ins2OpenPrice)*RATIOCOE;

				//			OutPutDebugStringFormat("第一根K线的开盘价是:%.2f\n", dfs.open_price_r);

				dfs.TradingDay = date;
				dfs.UpdateTime = Ins1Time;
				if (dfs.open_price > dfs.high_price)
					dfs.high_price = dfs.open_price;
				if (dfs.open_price < dfs.low_price)
					dfs.low_price = dfs.open_price;
				if (dfs.open_price_r > dfs.high_price_r)
					dfs.high_price_r = dfs.open_price_r;
				if (dfs.open_price_r < dfs.low_price_r)
					dfs.low_price_r = dfs.open_price_r;

			}
			else if (Ins1Time == 90000 && dfs.open_price == 0)
			{
				m_bHandleTdyFirstLine = true;
				if (bIns1Hot)
				{
					dfs.open_price = nRatio1*recVec[i].LastPrice - nRatio2*tmpTickVec[j].LastPrice;
					dfs.open_price_r = (nRatio1*recVec[i].LastPrice) / (nRatio2*tmpTickVec[j].LastPrice)*RATIOCOE;
				}
				else
				{
					dfs.open_price = nRatio1*tmpTickVec[j].LastPrice - nRatio2*recVec[i].LastPrice;
					dfs.open_price_r = (nRatio1*tmpTickVec[j].LastPrice) / (nRatio2*recVec[i].LastPrice)*RATIOCOE;
				}

				dfs.TradingDay = date;
				dfs.UpdateTime = Ins1Time;
				if (dfs.open_price > dfs.high_price)
					dfs.high_price = dfs.open_price;
				if (dfs.open_price < dfs.low_price)
					dfs.low_price = dfs.open_price;
				if (dfs.open_price_r > dfs.high_price_r)
					dfs.high_price_r = dfs.open_price_r;
				if (dfs.open_price_r < dfs.low_price_r)
					dfs.low_price_r = dfs.open_price_r;
			}
			if (nOffset != Ins1Offset&&Ins1Offset >= Ins2Offset)
			{
				if (i > 0 && j > 0)
				{
					if ((Ins1Time == 150000 && (m_pAll->nInstype&NationalDebt) == 0) || Ins1Time == 151500)
					{
						if (bIns1Hot)
						{
							dfs.close_price = nRatio1*recVec[i].LastPrice - nRatio2*tmpTickVec[j].LastPrice;
							dfs.close_price_r = (nRatio1*recVec[i].LastPrice) / (nRatio2*tmpTickVec[j].LastPrice)*RATIOCOE;
							dfs.volume = (recVec[i].Volume - tmpTickVec[j].Volume) - (Ins1LastVolume - Ins2LastVolume);
						}
						else
						{
							dfs.close_price = nRatio1*tmpTickVec[j].LastPrice - nRatio2*recVec[i].LastPrice;
							dfs.close_price_r = (nRatio1*tmpTickVec[j].LastPrice) / (nRatio2*recVec[i].LastPrice)*RATIOCOE;
							dfs.volume = (tmpTickVec[j].Volume - recVec[i].Volume) - (Ins2LastVolume - Ins1LastVolume);
						}
						m_bGroupIsWhole = true;

					}
					else
					{
						if (bIns1Hot)
						{
							dfs.close_price = nRatio1*recVec[i - 1].LastPrice - nRatio2*tmpTickVec[j - 1].LastPrice;
							dfs.close_price_r = (nRatio1*recVec[i - 1].LastPrice) / (nRatio2*tmpTickVec[j - 1].LastPrice)*RATIOCOE;
							dfs.volume = (recVec[i - 1].Volume - tmpTickVec[j - 1].Volume) - (Ins1LastVolume - Ins2LastVolume);
						}
						else
						{
							dfs.close_price = nRatio1*tmpTickVec[j - 1].LastPrice - nRatio2*recVec[i - 1].LastPrice;
							dfs.close_price_r = (nRatio1*tmpTickVec[j - 1].LastPrice) / (nRatio2*recVec[i - 1].LastPrice)*RATIOCOE;
							dfs.volume = (tmpTickVec[j - 1].Volume - recVec[i - 1].Volume) - (Ins2LastVolume - Ins1LastVolume);
						}

					}


					if (dfs.close_price > dfs.high_price)
						dfs.high_price = dfs.close_price;
					if (dfs.close_price < dfs.low_price)
						dfs.low_price = dfs.close_price;
					if (dfs.close_price_r > dfs.high_price_r)
						dfs.high_price_r = dfs.close_price_r;
					if (dfs.close_price_r < dfs.low_price_r)
						dfs.low_price_r = dfs.close_price_r;


					if (!std::isinf(dfs.open_price_r) && !std::isinf(dfs.high_price_r) && !std::isinf(dfs.low_price_r) && !std::isinf(dfs.close_price_r))
						m_Group1MinKline.push_back(dfs);

					Ins1LastVolume = recVec[i - 1].Volume;
					Ins2LastVolume = tmpTickVec[j - 1].Volume;
					if (!m_Group1MinKline.empty())
						m_nGroupTime = m_Group1MinKline.back().UpdateTime;
				}
				else
				{
					if (bIns1Hot)
					{
						dfs.close_price = nRatio1*recVec[i].LastPrice - nRatio2*tmpTickVec[j].LastPrice;
						dfs.close_price_r = (nRatio1*recVec[i].LastPrice) / (nRatio2*tmpTickVec[j].LastPrice)*RATIOCOE;
						dfs.volume = (recVec[i].Volume - tmpTickVec[j].Volume) - (Ins1LastVolume - Ins2LastVolume);
					}
					else
					{
						dfs.close_price = nRatio1*tmpTickVec[j].LastPrice - nRatio2*recVec[i].LastPrice;
						dfs.close_price_r = (nRatio1*tmpTickVec[j].LastPrice) / (nRatio2*recVec[i].LastPrice)*RATIOCOE;
						dfs.volume = (tmpTickVec[j].Volume - recVec[i].Volume) - (Ins2LastVolume - Ins1LastVolume);
					}
					if (dfs.close_price > dfs.high_price)
						dfs.high_price = dfs.close_price;
					if (dfs.close_price < dfs.low_price)
						dfs.low_price = dfs.close_price;
					if (dfs.close_price_r > dfs.high_price_r)
						dfs.high_price_r = dfs.close_price_r;
					if (dfs.close_price_r < dfs.low_price_r)
						dfs.low_price_r = dfs.close_price_r;

					if (!std::isinf(dfs.open_price_r) && !std::isinf(dfs.high_price_r) && !std::isinf(dfs.low_price_r) && !std::isinf(dfs.close_price_r))
						m_Group1MinKline.push_back(dfs);

					if (!m_Group1MinKline.empty())
						m_nGroupTime = m_Group1MinKline.back().UpdateTime;

					Ins1LastVolume = recVec[i].Volume;
					Ins2LastVolume = tmpTickVec[j].Volume;

				}

				nOffset = Ins1Offset;
				if (!((Ins1Time == 150000 && (m_pAll->nInstype&NationalDebt) == 0) || Ins1Time == 151500))
				{
					ZeroMemory(&dfs, sizeof(dfs));
					double fDelta, fRatio;
					if (bIns1Hot)
					{
						fDelta = nRatio1*recVec[i].LastPrice - nRatio2*tmpTickVec[j].LastPrice;
						fRatio = (nRatio1*recVec[i].LastPrice) / (nRatio2*tmpTickVec[j].LastPrice)*RATIOCOE;

					}
					else
					{
						fDelta = nRatio1*tmpTickVec[j].LastPrice - nRatio2*recVec[i].LastPrice;
						fRatio = (nRatio1*tmpTickVec[j].LastPrice) / (nRatio2*recVec[i].LastPrice)*RATIOCOE;

					}
					dfs.open_price = fDelta;
					dfs.high_price = fDelta;
					dfs.low_price = fDelta;
					dfs.open_price_r = fRatio;
					dfs.high_price_r = fRatio;
					dfs.low_price_r = fRatio;

					dfs.TradingDay = date;
					dfs.UpdateTime = Ins1Time;
					if (j + 1 < tmpTickVec.size() && i + 1 < recVec.size())
					{
						if (GetTimeOffset(recVec[i].UpdateTime, true) < GetTimeOffset(recVec[i + 1].UpdateTime, true))
						{
							if (GetTimeOffset(tmpTickVec[j + 1].UpdateTime, true) <= GetTimeOffset(recVec[i + 1].UpdateTime, true))
							{
								do
								{
									if (j + 1 < tmpTickVec.size())
										j++;
								} while (GetTimeOffset(tmpTickVec[j].UpdateTime, true) < GetTimeOffset(recVec[i + 1].UpdateTime, true));
							}
						}
						else if (GetTimeOffset(recVec[i].UpdateTime, true) == GetTimeOffset(recVec[i + 1].UpdateTime, true))
						{
							while (j + 1 < tmpTickVec.size() && GetTimeOffset(tmpTickVec[j + 1].UpdateTime, true) <= GetTimeOffset(recVec[i + 1].UpdateTime, true))
								j++;
						}
					}
					if (i == recVec.size() - 1)
					{
						if (bIns1Hot)
							dfs.volume = Ins1LastVolume - Ins2LastVolume;
						else
							dfs.volume = Ins2LastVolume - Ins1LastVolume;

						dfs.close_price = fDelta;
						dfs.close_price_r = fRatio;
						if (!m_Group1MinKline.empty() && m_Group1MinKline.back().UpdateTime != dfs.UpdateTime)
						{
							if (!std::isinf(dfs.open_price_r) && !std::isinf(dfs.high_price_r) && !std::isinf(dfs.low_price_r) && !std::isinf(dfs.close_price_r))
								m_Group1MinKline.push_back(dfs);
							if (!m_Group1MinKline.empty())
								m_nGroupTime = m_Group1MinKline.back().UpdateTime;
							m_nGroupVolume = dfs.volume;
						}
					}
				}

			}
			else
			{
				double fDelta, fRatio;
				if (bIns1Hot)
				{
					fDelta = nRatio1*recVec[i].LastPrice - nRatio2*tmpTickVec[j].LastPrice;
					fRatio = (nRatio1*recVec[i].LastPrice) / (nRatio2*tmpTickVec[j].LastPrice)*RATIOCOE;

				}
				else
				{
					fDelta = nRatio1*tmpTickVec[j].LastPrice - nRatio2*recVec[i].LastPrice;
					fRatio = (nRatio1*tmpTickVec[j].LastPrice) / (nRatio2*recVec[i].LastPrice)*RATIOCOE;
				}
				if (fDelta > dfs.high_price)
					dfs.high_price = fDelta;
				if (fDelta < dfs.low_price)
					dfs.low_price = fDelta;
				if (fRatio > dfs.high_price_r)
					dfs.high_price_r = fRatio;
				if (fRatio < dfs.low_price_r)
					dfs.low_price_r = fRatio;
				if (j + 1 < tmpTickVec.size() && i + 1 < recVec.size())
				{
					if (GetTimeOffset(recVec[i].UpdateTime, true) < GetTimeOffset(recVec[i + 1].UpdateTime, true))
					{
						if (GetTimeOffset(tmpTickVec[j + 1].UpdateTime, true) <= GetTimeOffset(recVec[i + 1].UpdateTime, true))
						{
							do
							{
								if (j + 1 < tmpTickVec.size())
									j++;
								else
									break;
							} while (GetTimeOffset(tmpTickVec[j].UpdateTime, true) < GetTimeOffset(recVec[i + 1].UpdateTime, true));
						}
					}
					else if (GetTimeOffset(recVec[i].UpdateTime, true) == GetTimeOffset(recVec[i + 1].UpdateTime, true))
					{
						while (j + 1 < tmpTickVec.size() && GetTimeOffset(tmpTickVec[j + 1].UpdateTime, true) <= GetTimeOffset(recVec[i + 1].UpdateTime, true))
							j++;
					}
				}
				if (i == recVec.size() - 1)
				{
					if (bIns1Hot)
						dfs.volume = Ins1LastVolume - Ins2LastVolume;
					else
						dfs.volume = Ins2LastVolume - Ins1LastVolume;
					dfs.close_price = fDelta;
					dfs.close_price_r = fRatio;
					if (!m_Group1MinKline.empty() && m_Group1MinKline.back().UpdateTime != dfs.UpdateTime)
					{
						if (!std::isinf(dfs.open_price_r) && !std::isinf(dfs.high_price_r) && !std::isinf(dfs.low_price_r) && !std::isinf(dfs.close_price_r))
							m_Group1MinKline.push_back(dfs);
						m_nGroupTime = dfs.UpdateTime;
						m_nGroupVolume = dfs.volume;
					}
				}
			}
		}
		if (!m_Group1MinKline.empty() && m_Group1MinKline.back().TradingDay < nToday)
			m_bGroupIsWhole = true;
	}
	TraceLog("组合:%s 合成历史1分钟K线数据处理完成", m_strSubIns);
}

void SOUI::SKlinePic::GroupHisDayKlineProc()
{

	std::vector<KLineDataType> klineVec;
	std::vector<KLineDataType> tmpKlineVec;
	m_GroupDayKline.clear();
	InsIDType InsID1 = g_GroupInsMap[m_strSubIns].Ins1;
	InsIDType InsID2 = g_GroupInsMap[m_strSubIns].Ins2;
	int nRatio1 = g_GroupInsMap[m_strSubIns].Ins1Ratio;
	int nRatio2 = g_GroupInsMap[m_strSubIns].Ins1Ratio;

	if (GetFileKlineData(InsID1, &klineVec, true) && GetFileKlineData(InsID2, &tmpKlineVec, true))
	{
		unsigned i = 0, j = 0;
		GroupKlineType dfs;
		for (i; i < klineVec.size(); i++)
		{
			for (; j < tmpKlineVec.size(); j++)
			{
				if (klineVec[i].TradingDay > tmpKlineVec[j].TradingDay)
					continue;
				else if (klineVec[i].TradingDay < tmpKlineVec[j].TradingDay)
					break;
				ZeroMemory(&dfs, sizeof(dfs));
				dfs.TradingDay = klineVec[i].TradingDay;
				dfs.UpdateTime = 0;
				double tmpHigh, tmpLow, tmpHigh_r, tmpLow_r;
				dfs.open_price = nRatio1*klineVec[i].open_price - nRatio2*tmpKlineVec[j].open_price;
				dfs.close_price = nRatio1*klineVec[i].close_price - nRatio2*tmpKlineVec[j].close_price;
				tmpHigh = dfs.open_price;
				tmpLow = dfs.close_price;

				dfs.open_price_r = (nRatio1*klineVec[i].open_price) / (nRatio2*tmpKlineVec[j].open_price)*RATIOCOE;
				dfs.close_price_r = (nRatio1*klineVec[i].close_price) / (nRatio2*tmpKlineVec[j].close_price)*RATIOCOE;
				tmpHigh_r = dfs.open_price_r;
				tmpLow_r = dfs.close_price_r;

				if (tmpHigh < tmpLow)
				{
					dfs.high_price = tmpLow;
					dfs.low_price = tmpHigh;
				}
				else
				{
					dfs.high_price = tmpHigh;
					dfs.low_price = tmpLow;
				}
				if (tmpHigh_r < tmpLow_r)
				{
					dfs.high_price_r = tmpLow_r;
					dfs.low_price_r = tmpHigh_r;
				}
				else
				{
					dfs.high_price_r = tmpHigh_r;
					dfs.low_price_r = tmpLow_r;
				}

				//if (dfs.high_price < dfs.open_price) dfs.high_price = dfs.open_price;
				//if (dfs.high_price < dfs.close_price) dfs.high_price = dfs.close_price;
				//if (dfs.low_price > dfs.open_price) dfs.low_price = dfs.open_price;
				//if (dfs.low_price > dfs.close_price) dfs.low_price = dfs.close_price;
				//if (dfs.high_price_r < dfs.open_price_r) dfs.high_price_r = dfs.open_price_r;
				//if (dfs.high_price_r < dfs.close_price_r) dfs.high_price_r = dfs.close_price_r;
				//if (dfs.low_price_r > dfs.open_price_r) dfs.low_price_r = dfs.open_price_r;
				//if (dfs.low_price_r > dfs.close_price_r) dfs.low_price_r = dfs.close_price_r;

				dfs.volume = klineVec[i].volume - tmpKlineVec[j].volume;
				m_GroupDayKline.push_back(dfs);
				break;
			}
		}
	}
	TraceLog("组合:%s 合成历史日K线数据处理完成", m_strSubIns);
}

void SOUI::SKlinePic::GroupUpdateNoKline(int nCount, int nPos, std::vector<GroupDataType>* pTickVec, int ntime, bool bAdd, bool bNewLine)
{
	if (*m_pGroupDataType == 0)
	{
		m_pAll->m_Klines.pd[nCount].close = pTickVec->back().dLml;
		if (bNewLine)
			m_pAll->m_Klines.pd[nCount].open = pTickVec->at(nPos).dLml;
		if (pTickVec->at(nPos).dLml > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pTickVec->at(nPos).dLml;
		if (pTickVec->at(nPos).dLml < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pTickVec->at(nPos).dLml;

	}
	else
	{
		m_pAll->m_Klines.pd[nCount].close = pTickVec->back().dLdl;
		if (bNewLine)
			m_pAll->m_Klines.pd[nCount].open = pTickVec->at(nPos).dLdl;

		if (pTickVec->at(nPos).dLdl > m_pAll->m_Klines.pd[nCount].high)
			m_pAll->m_Klines.pd[nCount].high = pTickVec->at(nPos).dLdl;
		if (pTickVec->at(nPos).dLdl < m_pAll->m_Klines.pd[nCount].low)
			m_pAll->m_Klines.pd[nCount].low = pTickVec->at(nPos).dLdl;
	}
	m_pAll->m_Klines.pd[nCount].date = atoi(pTickVec->at(nPos).TradingDay);
	m_pAll->m_Klines.pd[nCount].time = ntime;


	if (nPos > 0)
		m_pAll->m_Futu.ftl[nCount] = pTickVec->back().Volume - pTickVec->at(nPos - 1).Volume;
	else
		m_pAll->m_Futu.ftl[nCount] = pTickVec->back().Volume - pTickVec->at(nPos).Volume;

}

void SOUI::SKlinePic::KlineMAProc(int nCount)
{
	if (nCount >= m_nMAPara[0])
	{
		double Ma1Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[0] - 1; j--)
		{
			Ma1Sum += m_pAll->m_Klines.pd[j].close;
		}
		m_pAll->m_Klines.fMa[0][nCount - 1] = Ma1Sum / (double)m_nMAPara[0];
	}

	if (nCount >= m_nMAPara[1])
	{
		double Ma2Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[1] - 1; j--)
		{
			Ma2Sum += m_pAll->m_Klines.pd[j].close;
		}
		m_pAll->m_Klines.fMa[1][nCount - 1] = Ma2Sum / (double)m_nMAPara[1];
	}

	if (nCount >= m_nMAPara[2])
	{
		double Ma3Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[2] - 1; j--)
		{
			Ma3Sum += m_pAll->m_Klines.pd[j].close;
		}
		m_pAll->m_Klines.fMa[2][nCount - 1] = Ma3Sum / (double)m_nMAPara[2];
	}

	if (nCount >= m_nMAPara[3])
	{
		double Ma4Sum = 0;
		for (int j = nCount - 1; j > nCount - m_nMAPara[3] - 1; j--)
		{
			Ma4Sum += m_pAll->m_Klines.pd[j].close;
		}
		m_pAll->m_Klines.fMa[3][nCount - 1] = Ma4Sum / (double)m_nMAPara[3];
	}

}

void SOUI::SKlinePic::ReProcKlineData(bool bSingleNeedProc)
{
	if (!bSingleNeedProc && !m_bSubInsisGroup)
		return;
	m_bDataInited = false;
	int nDigit = m_pAll->m_Klines.nDecimal;
	ZeroMemory(&m_pAll->m_Klines, sizeof(m_pAll->m_Klines));
	ZeroMemory(&m_pAll->m_Futu, sizeof(m_pAll->m_Futu));
	m_pAll->nTotal = 0;
	m_pAll->m_Klines.nDecimalXi = 1;
	m_pAll->m_Futu.bft = 2;
	m_pAll->m_Klines.nDecimal= nDigit;
	if (*m_pGroupDataType == 0)
		_swprintf(m_pAll->m_Klines.sDecimal, L"%%.%df", m_pAll->m_Klines.nDecimal);
	else if (m_pAll->nInstype == NationalDebt)
		wcscpy_s(m_pAll->m_Klines.sDecimal, L"%.3f");
	else
		wcscpy_s(m_pAll->m_Klines.sDecimal, L"%.2f");
	wcscpy_s(m_pAll->m_Futu.sDecimal, L"%.0f");
	m_pAll->m_Futu.nDecimal = 1;
	m_pAll->m_Futu.nDecimalXi = 1;
	m_bNeedAddCount = false;
	ZeroMemory(&m_Last1MinHisData, sizeof(m_Last1MinHisData));

	if (m_bSubInsisGroup)
	{
		if (m_nKlineType == 5)
			GroupHisDayKlineProc();
		else
			GroupHisKline1MinProc();
		GroupDataWithHis();
		GroupDataNoHis();
	}
	else
	{
		SingleDataWithHis();
		SingleDataNoHis();
	}
	m_bDataInited = true;

	//	Invalidate();
}

void SOUI::SKlinePic::UpdateData()
{


	if (m_pAll == nullptr)
		return;
	//	::EnterCriticalSection(&m_cs);
	if (!m_bSubInsisGroup)
		SingleDataUpdate();
	else
		GroupDataUpdate();




	//	::LeaveCriticalSection(&m_cs);
	//	Invalidate();

		//if (m_bShowBandTarget)
		//	BandDataUpdate();


		//GetMaxDiff(m_nKlineType);
		//if (m_bShowVolume)
		//	GetFuTuMaxDiff();

		//if (m_bShowMacd)
		//{
		//	MACDDataUpdate();
		//	GetMACDMaxDiff();
		//}
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

BOOL SOUI::SKlinePic::ptIsInKlineRect(CPoint pt, int nDataCount, DATA_FOR_SHOW &data)
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

BOOL SOUI::SKlinePic::IsCATime(int nTime, int nDate)
{
	if (m_pAll->nInstype == ComNoNight&&nTime == 85500)
		return TRUE;
	else if ((m_pAll->nInstype == ComNightEnd2300 || m_pAll->nInstype == ComNightEnd100 || m_pAll->nInstype == ComNightEnd230)
		&& nTime == 205500)
		return TRUE;
	else if ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)
		&& (nTime == 92500 || nTime == 91000))
	{
		if (m_pAll->nInstype == StockIndex)
		{
			if (nTime == 92500 && nDate > 20160101)
				return TRUE;

			if (nTime == 91000 && nDate < 20160101)
				return TRUE;
		}

		if (m_pAll->nInstype == NationalDebt)
		{
			if (nTime == 92500 && nDate >= 20200720)
				return TRUE;

			if (nTime == 91000 && nDate < 20200720)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL SOUI::SKlinePic::IsFirstLine(int nTime)
{
	if (m_pAll->nInstype == ComNoNight&&nTime == 90000)
		return TRUE;
	else if ((m_pAll->nInstype == StockIndex || m_pAll->nInstype == NationalDebt)
		&& (nTime == 93000 || nTime == 90000))
		return TRUE;
	return FALSE;
}

void SOUI::SKlinePic::AddCADataToFirstLine(KLineDataType & CALine, KLineDataType & FirstLine)
{
	FirstLine.open_price = CALine.open_price;
	if (FirstLine.high_price < CALine.high_price)
		FirstLine.high_price = CALine.high_price;
	if (FirstLine.low_price > CALine.low_price)
		FirstLine.low_price = CALine.low_price;
	FirstLine.volume += CALine.volume;
}

bool SOUI::SKlinePic::TimeIsLastBar(int UpdateTime, int LastBarTime, int TickTime, int nPeriod)
{
	if (UpdateTime == LastBarTime + (nPeriod - 1) * 100)
		return true;
	else if (UpdateTime == 151400)
		return true;
	else if (UpdateTime == 112900 && LastBarTime < 130000)
		return true;
	else if (nPeriod == 15 || nPeriod == 30)
	{
		if ((UpdateTime == 101400 && StockIndex != m_pAll->nInstype&&NationalDebt != m_pAll->nInstype
			&&LastBarTime == 100000 && TickTime > 101500))
			return true;
		else
			return false;
	}
	else
		return false;


}

