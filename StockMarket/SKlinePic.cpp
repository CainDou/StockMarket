#include "stdafx.h"
#include "SKlinePic.h"
#include <map>
#include <vector>
#include<gdiplus.h>
#include<unordered_map>
#include"DealList.h"
#include"PriceList.h"
#include "SSubTargetPic.h"
#include <set>

using std::vector;
#define MOVE_ONESTEP		10		//每次平移的数据量
#define K_WIDTH_TOTAL		16		//k线占用总宽度(在x轴上)
//一些基本的界面框架信息
#define RC_LEFT				48
#define RC_RIGHT			48
#define RC_TOP				25
#define RC_BOTTOM			40

#define RC_MAX				5
#define RC_MIN				5
#define RC_RIGHT_BACK		50		//框内最后一根k线和框得距离

#define DATA_ERROR			-1234567
#define SDECIMAL			L"%.02f"

#define	ZOOMWIDTH (m_nKWidth * 1.0 / m_nZoomRatio)
#define TOTALZOOMWIDTH ((m_nKWidth + m_nJiange) * 1.0 / m_nZoomRatio)

#define MAX_SUBWINDOW 5

SKlinePic::SKlinePic()
{

	::InitializeCriticalSection(&m_cs);
	::InitializeCriticalSection(&m_csSub);

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
	m_ppSubPic = nullptr;
	m_strSubIns = "";
	m_bReSetFirstLine = false;
	m_nFirst = 0;
	m_nEnd = 0;
	m_bKeyDown = false;
	m_preMovePoint.SetPoint(-1, -1);
	m_bShowMA = true;
	m_bClearTip = false;
	m_nPreX = -1;
	m_nPreY = -1;
	//m_pbShowSubPic = nullptr;
	m_bTodayMarketReady = false;
	m_bHisKlineReady = false;
	m_bHisPointReady = false;
	m_nMAPara[0] = 5;
	m_nMAPara[1] = 10;
	m_nMAPara[2] = 20;
	m_nMAPara[3] = 60;
	m_pTip = nullptr;
	m_nZoomRatio = 1;
	//m_pTip = new SKlineTip(m_hParWnd);

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
	if (m_ppSubPic)
	{
		for (int i = 0; i < m_nSubPicNum; ++i)
			delete m_ppSubPic[i];
		delete[]m_ppSubPic;
	}
	//if (m_pbShowSubPic)
	//	delete[]m_pbShowSubPic;
}

void SKlinePic::InitSubPic(int nNum)
{

	m_nSubPicNum = nNum;
	m_ppSubPic = new SSubTargetPic*[nNum];
	for (int i = 0; i < nNum; ++i)
	{
		m_ppSubPic[i] = new SSubTargetPic;
		//m_ppSubPic[i]->SetSubPicName(picNameVec[i]);
	}
	//m_pbShowSubPic = new BOOL[nNum];
}

void SKlinePic::ReSetSubPic(int nNum, vector<ShowPointInfo>& infoVec)
{
	::EnterCriticalSection(&m_csSub);
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic*[nNum];
	if (nNum > m_nSubPicNum)
	{
		for (int i = 0; i < m_nSubPicNum; ++i)
			ppTmpSubPicArr[i] = m_ppSubPic[i];
		delete[]m_ppSubPic;
		m_ppSubPic = nullptr;
		m_ppSubPic = ppTmpSubPicArr;
		for (int i = m_nSubPicNum; i < nNum; ++i)
		{
			m_ppSubPic[i] = new SSubTargetPic;
			m_ppSubPic[i]->SetSubPicInfo(infoVec[i - m_nSubPicNum]);
			m_ppSubPic[i]->SetOffset(m_nMove);
			m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);
		}

	}
	else if (nNum < m_nSubPicNum)
	{
		for (int i = 0; i < nNum; ++i)
			ppTmpSubPicArr[i] = m_ppSubPic[i];
		for (int i = nNum; i < m_nSubPicNum; ++i)
			delete m_ppSubPic[i];
		delete[]m_ppSubPic;
		m_ppSubPic = nullptr;
		m_ppSubPic = ppTmpSubPicArr;
	}

	m_nSubPicNum = nNum;
	::LeaveCriticalSection(&m_csSub);

}

vector<ShowPointInfo> SOUI::SKlinePic::GetSubPicDataToGet(int nNum, map<ePointDataType, ShowPointInfo>& infoMap)
{
	::EnterCriticalSection(&m_csSub);
	vector<ShowPointInfo> infoVec;
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic*[nNum];
	if (nNum > m_nSubPicNum)
	{
		auto  tmpInfoMap = infoMap;
		for (int i = 0; i < m_nSubPicNum; ++i)
		{
			auto spi = m_ppSubPic[i]->GetSubPicInfo();
			for (auto &it : infoMap)
			{
				if (it.second == spi)
				{
					if (tmpInfoMap.size() > 1)
						tmpInfoMap.erase(it.first);
					break;
				}

			}
		}
		for (int i = m_nSubPicNum; i < nNum; ++i)
		{
			auto& infoPair = tmpInfoMap.begin();
			infoVec.emplace_back(infoPair->second);
			if (tmpInfoMap.size() > 1)
				tmpInfoMap.erase(infoPair->first);

		}
	}
	::LeaveCriticalSection(&m_csSub);
	return infoVec;
}


void SKlinePic::InitShowPara(InitPara_t para)
{
	m_pTip = new SKlineTip(m_hParWnd);
	m_pTip->Create();

	m_bShowBandTarget = para.bShowBandTarget;
	m_bShowVolume = para.bShowKlineVolume;
	m_bShowMA = para.bShowMA;
	m_nKWidth = para.nWidth;
	m_bShowDeal = para.bShowKlineDeal;
	m_bShowMacd = para.bShowKlineMACD;
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->SetSubPicInfo(para.KlinePonitWndInfo[i]);
	::LeaveCriticalSection(&m_csSub);

	m_nMACDPara[0] = para.nMACDPara[0];
	m_nMACDPara[1] = para.nMACDPara[1];
	m_nMACDPara[2] = para.nMACDPara[2];
	m_BandPara = para.BandPara;
	m_nMAPara[0] = para.nMAPara[0];
	m_nMAPara[1] = para.nMAPara[1];
	m_nMAPara[2] = para.nMAPara[2];
	m_nMAPara[3] = para.nMAPara[3];

	m_nJiange = para.nJiange;
}

void SKlinePic::OutPutShowPara(InitPara_t & para)
{
	para.bShowBandTarget = m_bShowBandTarget;
	para.bShowKlineVolume = m_bShowVolume;
	para.bShowMA = m_bShowMA;
	para.nWidth = m_nKWidth;
	para.bShowKlineDeal = m_bShowDeal;
	para.bShowKlineMACD = m_bShowMacd;
	::EnterCriticalSection(&m_csSub);
	para.nKlinePointWndNum = m_nSubPicNum;
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		para.KlinePonitWndInfo.resize(m_nSubPicNum);
		para.KlinePonitWndInfo[i] = m_ppSubPic[i]->GetSubPicInfo();
	}
	::LeaveCriticalSection(&m_csSub);

	para.nMACDPara[0] = m_nMACDPara[0];
	para.nMACDPara[1] = m_nMACDPara[1];
	para.nMACDPara[2] = m_nMACDPara[2];
	para.BandPara = m_BandPara;
	para.nMAPara[0] = m_nMAPara[0];
	para.nMAPara[1] = m_nMAPara[1];
	para.nMAPara[2] = m_nMAPara[2];
	para.nMAPara[3] = m_nMAPara[3];
	para.nJiange = m_nJiange;
}

void SKlinePic::SetShowData(SStringA subIns, SStringA StockName, vector<CommonIndexMarket>* pIdxMarketVec,
	map<int, vector<KlineType>>*pHisKlineMap)
{
	m_bDataInited = false;
	m_strSubIns = subIns;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pHisKlineMap = pHisKlineMap;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
	m_strStockName = StockName;
	::EnterCriticalSection(&m_csSub);

	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);
		m_ppSubPic[i]->SetOffset2Zero();
	}
	::LeaveCriticalSection(&m_csSub);

	m_pPriceList->SetShowData(subIns, m_strStockName, pIdxMarketVec);
	m_pDealList->SetShowData(subIns, pIdxMarketVec);
}

void SKlinePic::SetShowData(SStringA subIns, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec,
	map<int, vector<KlineType>>*pHisKlineMap)
{
	KillTimer(1);
	m_bDataInited = false;
	m_strSubIns = subIns;
	m_pStkMarketVec = pStkMarketVec;
	m_pHisKlineMap = pHisKlineMap;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
	m_strStockName = StockName;
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);
		m_ppSubPic[i]->SetOffset2Zero();
	}
	::LeaveCriticalSection(&m_csSub);

	m_pPriceList->SetShowData(subIns, m_strStockName, pStkMarketVec);
	m_pDealList->SetShowData(subIns, pStkMarketVec);
}

void SKlinePic::SetSubPicShowData(int nIndex, bool nGroup)
{
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->SetShowData(nIndex, nGroup);
	::LeaveCriticalSection(&m_csSub);

}

void SKlinePic::SetSubPicShowData(int nDataCount[],
	vector<vector<vector<CoreData>*>>& data, vector<vector<BOOL>> bRightVec,
	vector<vector<SStringA>> dataNameVec, SStringA StockID, SStringA StockName,
	int nStartWnd)
{
	::EnterCriticalSection(&m_csSub);
	for (int i = nStartWnd; i < m_nSubPicNum; ++i)
	{
		int nTargetCount = i - nStartWnd;
		m_ppSubPic[i]->SetShowData(
			nDataCount[nTargetCount], &data[nTargetCount][0],
			bRightVec[nTargetCount], dataNameVec[nTargetCount],
			StockID, StockName);
	}
	::LeaveCriticalSection(&m_csSub);

}

void SOUI::SKlinePic::SetSubPicShowData(int nDataCount, vector<vector<CoreData>*>& data, vector<BOOL> bRightVec,
	vector<SStringA> dataNameVec, SStringA StockID, SStringA StockName)
{
	m_ppSubPic[m_nChangeNum]->SetShowData(
		nDataCount, &data[0],
		bRightVec, dataNameVec,
		StockID, StockName);
}

void SKlinePic::ReSetSubPicData(int nDataCount,
	vector<CoreData>* data[],
	vector<BOOL>& bRightVec)
{
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->SetOffset(m_nMove);
		m_ppSubPic[i]->ReSetShowData(nDataCount, data, bRightVec);
		m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);

	}
	::LeaveCriticalSection(&m_csSub);

}

void SKlinePic::SetParentHwnd(HWND hParWnd)
{
	m_hParWnd = hParWnd;
	m_pTip = new SKlineTip(m_hParWnd);
	m_pTip->Create();
}

void SKlinePic::SetTodayMarketState(bool bReady)
{
	m_bTodayMarketReady = bReady;
	if (!m_bTodayMarketReady)
	{
		m_bDataInited = false;
	}

}


void SKlinePic::SetHisKlineState(bool bReady)
{
	m_bHisKlineReady = bReady;
	if (!m_bHisKlineReady)
	{
		m_bDataInited = false;
	}
}

void SKlinePic::SetHisPointState(bool bReady)
{
	m_bHisPointReady = bReady;
	if (!m_bHisPointReady)
	{
		m_bDataInited = false;
	}
}

bool SOUI::SKlinePic::GetDataReadyState()
{
	return m_bTodayMarketReady&m_bHisKlineReady;
}

void SKlinePic::ClearTip()
{
	m_pTip->ClearTip();
}

//LRESULT SKlinePic::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp)
//{
//	USHORT nGroup = LOWORD(lp);
//	if ((RpsGroup)nGroup != m_rgGroup)
//	{
//		SetMsgHandled(FALSE);
//		return -1;
//	}
//	KLINEMSG msg = (KLINEMSG)HIWORD(lp);
//	switch (msg)
//	{
//	case KLINEMSG_PROCDATA:
//		DataProc();
//		break;
//	case KLINEMSG_UPDATE:
//		break;
//	case KLINEMSG_MA:
//		ReProcMAData();
//		Invalidate();
//		break;
//	case KLINEMSG_MACD:
//		m_nMacdCount = 0;
//		UpdateData();
//		Invalidate();
//		break;
//	case KLINEMSG_BAND:
//		if (m_pBandData)
//			ZeroMemory(m_pBandData, sizeof(*m_pBandData));
//		m_nBandCount = 0;
//		BandDataUpdate();
//		Invalidate();
//		break;
//	default:
//		break;
//	}
//	return 0;
//}

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

	strMarket.Format(L"%s 日期:%04d-%02d-%02d 时间:%02d:%02d 开:%.02f 高:%.02f 低:%.02f 收:%.02f",
		StrA2StrW(m_strStockName), data.date / 10000, data.date % 10000 / 100, data.date % 100,
		data.time / 100, data.time % 100,
		data.open, data.high, data.low, data.close);
	SStringW strVol;
	if (data.vol > 1'000'000'000)
		strVol.Format(L"%.01f亿", data.vol / 100'000'000);
	else if (data.vol > 100'000'000)
		strVol.Format(L"%.02f亿", data.vol / 100'000'000);
	else if (data.vol > 1'000'000)
		strVol.Format(L"%.0f万", data.vol / 10000);
	else if (data.vol > 10'000)
		strVol.Format(L"%.02f万", data.vol / 10000);
	else
		strVol.Format(L"%.0f", data.vol);

	SStringW strAmt;
	if (data.amount > 1'000'000'000)
		strAmt.Format(L"%.01f亿", data.amount / 100'000'000);
	else if (data.amount > 100'000'000)
		strAmt.Format(L"%.02f亿", data.amount / 100'000'000);
	else if (data.amount > 1'000'000)
		strAmt.Format(L"%.0f万", data.amount / 10000);
	else if (data.amount > 10'000)
		strAmt.Format(L"%.02f万", data.amount / 10000);
	else
		strAmt.Format(L"%.0f", data.amount);

	strMarket.Format(L"%s 量:%s 金额:%s", strMarket, strVol, strAmt);

	DrawTextonPic(pRT, CRect(m_rcImage.left, m_rcImage.top - 20, m_rcImage.right, m_rcImage.top), strMarket);
}

void SKlinePic::DrawMainUpperMA(IRenderTarget * pRT, int nPos)
{
	HDC hdc = pRT->GetDC();
	CSize size;
	size.cx = 0; size.cy = 0;
	int left = 0;
	SStringW strMarket;
	auto &arrMA = m_pAll->fMa;
	if (nPos >= m_nMAPara[0] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[0], arrMA[0][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[0]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	if (nPos >= m_nMAPara[1] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[1], arrMA[1][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[1]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	if (nPos >= m_nMAPara[2] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[2], arrMA[2][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[2]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 0, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	if (nPos >= m_nMAPara[3] - 1)
		strMarket.Format(L"MA%d:%.2f", m_nMAPara[3], arrMA[3][nPos]);
	else
		strMarket.Format(L"MA%d:-", m_nMAPara[3]);
	DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
	pRT->ReleaseDC(hdc);
}

void SKlinePic::DrawMacdUpperMarket(IRenderTarget * pRT, int nPos)
{
	SStringW strMarekt;
	strMarekt.Format(L"MACD(%d,%d,%d) DIF:%.2f", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2], m_pMacdData->DIF[nPos]);
	DrawTextonPic(pRT, CRect(m_rcMACD.left + 5, m_rcMACD.top + 5, m_rcMACD.left + 160, m_rcMACD.top + 20),
		strMarekt, RGBA(255, 255, 255, 255));
	strMarekt.Format(L"DEA:%.2f", m_pMacdData->DEA[nPos]);
	DrawTextonPic(pRT, CRect(m_rcMACD.left + 160, m_rcMACD.top + 5, (m_rcMACD.left + 240 > m_rcMACD.right ? m_rcMACD.right : m_rcMACD.left + 240), m_rcMACD.top + 20),
		strMarekt, RGBA(255, 255, 0, 255));
	strMarekt.Format(L"MACD:%.2f", m_pMacdData->MACD[nPos]);
	DrawTextonPic(pRT, CRect(m_rcMACD.left + 240, m_rcMACD.top + 5, m_rcMACD.right, m_rcMACD.top + 20),
		strMarekt, RGBA(255, 0, 255, 255));

}

void SKlinePic::DrawMainUpperBand(IRenderTarget * pRT, int nPos)
{
	SStringW strMarket;
	HDC hdc = pRT->GetDC();
	CSize size;
	size.cx = 0; size.cy = 0;
	int nLeft = 0;
	if (m_pBandData->DataValid[nPos])
	{
		strMarket.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"多平位置:%.02f", m_pBandData->SellLong[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(100, 100, 100, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"空平位置:%.02f", m_pBandData->BuyShort[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"上轨:%.02f", m_pBandData->UpperTrack1[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"下轨:%.02f", m_pBandData->LowerTrack1[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"上轨K2:%.02f", m_pBandData->UpperTrack2[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"下轨K2:%.02f", m_pBandData->LowerTrack2[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"状态:%d", m_pBandData->Status[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"仓位:%.2f", m_pBandData->Position[nPos]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	}
	else
	{
		strMarket.Format(L" 波段优化(%d,%d,%d,%d,%d,%d)", m_BandPara.N1, m_BandPara.N2, m_BandPara.K, m_BandPara.M1, m_BandPara.M2, m_BandPara.P);
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"多平位置:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(100, 100, 100, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"空平位置:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"上轨:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"下轨:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"上轨K2:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(0, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"下轨K2:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"状态:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		nLeft += size.cx;

		strMarket.Format(L"仓位:-");
		DrawTextonPic(pRT, CRect(m_rcImage.left + nLeft, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket, RGBA(255, 255, 0, 255));

	}
	pRT->ReleaseDC(hdc);

}


void SKlinePic::SetWindowRect()
{
	if (!m_bShowDeal)
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right, m_rcAll.bottom);
	else
	{
		if (m_bIsStockIndex)
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 180,
				m_rcAll.top, m_rcAll.right + 30, m_rcAll.top + 160);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 180,
				m_rcAll.top + 165, m_rcAll.right + 30, m_rcAll.bottom + 30);
		}
		else
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 180,
				m_rcAll.top, m_rcAll.right + 30, m_rcAll.top + 580);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 180,
				m_rcAll.top + 585, m_rcAll.right + 30, m_rcAll.bottom + 30);
		}
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top,
			m_rcAll.right - 240, m_rcAll.bottom);
	}

	vector<CRect*>pSubRect;
	if (m_bShowVolume)
		pSubRect.emplace_back(&m_rcVolume);
	else m_rcVolume.SetRectEmpty();

	if (m_bShowMacd)
		pSubRect.emplace_back(&m_rcMACD);
	else m_rcMACD.SetRectEmpty();

	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		pSubRect.emplace_back(m_ppSubPic[i]->GetPicRect());
	::LeaveCriticalSection(&m_csSub);

	int preBottom = m_rcImage.top;
	int nowBottom = m_rcImage.top +
		m_rcImage.Height() / (pSubRect.size() + 2) * 2;
	m_rcMain.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right,
		m_rcImage.top + m_rcImage.Height() / (pSubRect.size() + 2) * 2);
	preBottom = nowBottom;
	SStringA str;
	for (int i = 0; i < pSubRect.size(); ++i)
	{
		nowBottom = m_rcImage.top + m_rcImage.Height()
			/ (pSubRect.size() + 2) * (3 + i);
		pSubRect[i]->SetRect(m_rcImage.left, preBottom,
			m_rcImage.right, nowBottom);
		preBottom = nowBottom;

	}

}

int SKlinePic::GetShowKlineNum(int nKwidth)	//获取需要显示的k线数量
{
	int nSrcwidth = m_rcMain.right - m_rcMain.left - RC_RIGHT_BACK;	//判断是否超出范围
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
		pMacdData->EMA12[nPos] = EMA(m_nMACDPara[0], pMacdData->EMA12[nPos - 1], Data[nPos].close);
		pMacdData->EMA26[nPos] = EMA(m_nMACDPara[1], pMacdData->EMA26[nPos - 1], Data[nPos].close);
		pMacdData->DIF[nPos] = pMacdData->EMA12[nPos] - pMacdData->EMA26[nPos];
		pMacdData->DEA[nPos] = EMA(m_nMACDPara[2], pMacdData->DEA[nPos - 1], pMacdData->DIF[nPos]);
		pMacdData->MACD[nPos] = 2 * (pMacdData->DIF[nPos] - pMacdData->DEA[nPos]);
	}

	return nPos;
}

//BOOL SKlinePic::CreateChildren(pugi::xml_node xmlNode)
//{
//	if (!__super::CreateChildren(xmlNode))
//		return FALSE;
//	m_ppSubPic = nullptr;
//
//	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
//	while (pChild)
//	{
//		if (pChild->IsClass(SSubPic::GetClassName()))
//		{
//			m_ppSubPic = (SSubPic*)pChild;
//			break;
//		}
//		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
//	}
//}

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

		m_bPaintInit = true;

	}
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->InitColorAndPen(pRT);
	::LeaveCriticalSection(&m_csSub);

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);


	SWindow::GetClientRect(&m_rcAll);
	pRT->FillSolidRect(m_rcAll, RGBA(0, 0, 0, 255));
	m_rcAll.DeflateRect(RC_LEFT + 5, RC_TOP, RC_RIGHT + 10, RC_BOTTOM);
	SetWindowRect();
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(192, 192, 192, 255), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		pts[0].SetPoint(m_rcAll.left - RC_FSLEFT + 1, m_rcAll.top - 25);
		pts[1].SetPoint(m_rcAll.left - RC_FSLEFT + 1, m_rcAll.bottom + 35);
		pts[2].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_rcAll.bottom + 35);
		pts[3].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_rcAll.top - 25);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		if (m_bShowDeal)
		{
			pts[0].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcAll.top - 25);
			pts[1].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcAll.bottom + 35);
			pRT->DrawLines(pts, 2);
			pts[0].SetPoint(m_rcImage.right + RC_FSLEFT + 1,
				m_pPriceList->m_rect.bottom - 5);
			pts[1].SetPoint(m_rcAll.right + RC_FSLEFT + 1,
				m_pPriceList->m_rect.bottom - 5);
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
		::EnterCriticalSection(&m_csSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetMousePosDefault();
		::LeaveCriticalSection(&m_csSub);

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
	int nLen = m_rcMain.bottom - m_rcMain.top;
	int nYoNum = 9;		//y轴标示数量 9 代表画8根线
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(m_rcMain.left, m_rcMain.top);
		pts[1].SetPoint(m_rcMain.left, m_rcMain.bottom);
		pts[2].SetPoint(m_rcMain.right, m_rcMain.bottom);
		pts[3].SetPoint(m_rcMain.right, m_rcMain.top);
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
		int nY = m_rcMain.top + ((m_rcMain.bottom - m_rcMain.top - 20) / (nYoNum - 1) * i) + 20;
		if (i == 0)
		{
			CPoint pts[2];
			{
				CAutoRefPtr<IPen> oldPen, pen;
				//	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
				pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
				pRT->SelectObject(pen, (IRenderObj**)&oldPen);
				//y轴	//x轴
				pts[0].SetPoint(m_rcMain.left, nY);
				pts[1].SetPoint(m_rcMain.right, nY);
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
			}
		}
		else
		{
			for (int j = m_rcMain.left + 1; j < m_rcMain.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线
		}
		//k线区y轴加轴标
		if (m_bDataInited)
		{

			SStringW s1 = GetYPrice(nY);
			double fPrice = _wtof(s1);
			if (fPrice<15000 && fPrice>-15000)
				DrawTextonPic(pRT, CRect(m_rcMain.right - RC_LEFT + 2, nY - 9, m_rcImage.left, nY + 16), s1, RGBA(255, 0, 0, 255), DT_CENTER);
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
			pts[0].SetPoint(m_rcVolume.left, m_rcVolume.top);
			pts[1].SetPoint(m_rcVolume.left, m_rcVolume.bottom);
			pts[2].SetPoint(m_rcVolume.right, m_rcVolume.bottom);
			pts[3].SetPoint(m_rcVolume.right, m_rcVolume.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_LEFT + 2, m_rcVolume.top, m_rcImage.left, m_rcVolume.top + 18), L"成交量", RGBA(255, 0, 0, 255), DT_CENTER);
			DrawTextonPic(pRT, CRect(m_rcMain.right, m_rcVolume.top, m_rcImage.right + RC_RIGHT - 2, m_rcVolume.top + 18), L"金额", RGBA(255, 0, 0, 255), DT_CENTER);

		}

		//副图区横向虚线
		pdc = pRT->GetDC();
		for (int i = 1; i < 2; i++)
		{
			int nY = m_rcVolume.bottom - ((m_rcVolume.bottom - m_rcVolume.top - 5) / 2 * i);
			for (int j = m_rcVolume.left + 1; j < m_rcVolume.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线

													//标注

			if (m_bDataInited)
			{
				SStringW sl = GetFuTuYPrice(nY);
				DrawTextonPic(pRT, CRect(m_rcMain.left - RC_LEFT + 2, nY - 9, m_rcImage.left, nY + 9), sl, RGBA(255, 0, 0, 255), DT_CENTER);
				SStringW sr = GetFuTuYPrice(nY, true);
				DrawTextonPic(pRT, CRect(m_rcMain.right, nY - 9, m_rcImage.right + RC_RIGHT - 2, nY + 9), sr, RGBA(255, 255, 0, 255), DT_CENTER);

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
			pts[0].SetPoint(m_rcMACD.left, m_rcMACD.top);
			pts[1].SetPoint(m_rcMACD.left, m_rcMACD.bottom);
			pts[2].SetPoint(m_rcMACD.right, m_rcMACD.bottom);
			pts[3].SetPoint(m_rcMACD.right, m_rcMACD.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
		}

		//副图区横向虚线
		pdc = pRT->GetDC();
		int nWidthMacd = (m_rcMACD.Height() - 20) / 4;

		for (int i = 0; i < 4; i++)
		{
			int nY = m_rcMACD.top + 20 + nWidthMacd*i;
			CPoint pts[2];
			{
				CAutoRefPtr<IPen> pen, oldPen;
				if (i == 2)
					pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
				else
					pRT->CreatePen(PS_SOLID, RGBA(75, 0, 0, 0xFF), 2, &pen);
				pRT->SelectObject(pen, (IRenderObj**)&oldPen);
				//y轴	//x轴
				pts[0].SetPoint(m_rcMACD.left, nY);
				pts[1].SetPoint(m_rcMACD.right, nY);
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
			}
		}
		pRT->ReleaseDC(pdc);

	}

	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->DrawArrow(pRT);
	}
	::LeaveCriticalSection(&m_csSub);

}



void SKlinePic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{
	int nLen = m_rcMain.right - m_rcMain.left - RC_RIGHT_BACK;	//判断是否超出范围
	//int  nJiange = 2;
	//if (m_nJiange)
	//	nJiange = 0;
	m_nMaxKNum = nLen / TOTALZOOMWIDTH;
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
		int tmpFirst = max(m_nMouseLinePos - m_nMaxKNum / 2, 0);
		int tmpEnd = tmpFirst + (m_nEnd - m_nFirst);
		if (tmpFirst >= 0 && tmpEnd < nTotal)
		{
			m_nFirst = tmpFirst;
			m_nEnd = tmpEnd;
			m_nMove = nTotal - m_nEnd;
		}

		::EnterCriticalSection(&m_csSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetOffset(m_nMove);
		::LeaveCriticalSection(&m_csSub);

		m_bReSetFirstLine = false;
	}
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->SetShowNum(m_nEnd - m_nFirst);
	::LeaveCriticalSection(&m_csSub);

	//	}
	//判断最大最小值
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
	double fVolMax = -100000000000000;
	double fVolMin = 0;
	double fAmtMax = -100000000000000;
	double fAmtMin = 0;

	auto &data = m_pAll->data;
	for (int j = m_nFirst; j < m_nEnd; j++)
	{
		if (data[j].vol > fVolMax)
			fVolMax = data[j].vol;
		if (data[j].amount > fAmtMax)
			fAmtMax = data[j].amount;
	}


	m_pAll->fVolMax = fVolMax;
	m_pAll->fVolMin = fVolMin;
	m_pAll->fAmountMax = fAmtMax;
	m_pAll->fAmountMin = fAmtMin;

	if (m_pAll->fVolMax == m_pAll->fVolMin)
		m_pAll->fVolMax = m_pAll->fVolMax * 1.1;
	if (m_pAll->fVolMax == 0)
		m_pAll->fVolMax = 1;

	if (m_pAll->fAmountMax == m_pAll->fAmountMin)
		m_pAll->fAmountMax = m_pAll->fAmountMax * 1.1;
	if (m_pAll->fAmountMax == 0)
		m_pAll->fAmountMax = 1;

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
		prc = &m_rcMain;
		break;
	case 2:
		prc = &m_rcVolume;
		break;
	default:
		return FALSE;
	}
	if (x >= prc->left && x <= prc->right &&
		y >= prc->top  && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

int SKlinePic::GetFuTuYPos(double fDiff, bool bAmt)	//获得附图y位置
{
	double fPos = 0;
	if (!bAmt)
		fPos = m_rcVolume.top + (1 - (fDiff / m_pAll->fVolMax))  * (m_rcVolume.Height() - 5) + RC_MAX;
	else
		fPos = m_rcVolume.top + (1 - (fDiff / m_pAll->fAmountMax))  * (m_rcVolume.Height() - 5) + RC_MAX;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SKlinePic::GetFuTuYPrice(int nY, bool bAmt)
{
	SStringW strRet; strRet.Empty();
	if (!bAmt)
	{
		if (nY > m_rcVolume.bottom || nY < m_rcVolume.top)
			return strRet;
		double fDiff = ((double)m_rcVolume.bottom - nY) / (m_rcVolume.Height() - 5)  * m_pAll->fVolMax;
		if (fDiff > 1'000'000'000)
			strRet.Format(L"%.01f亿", fDiff / 100'000'000);
		else if (fDiff > 100'000'000)
			strRet.Format(L"%.02f亿", fDiff / 100'000'000);
		else if (fDiff > 1'000'000)
			strRet.Format(L"%.0f万", fDiff / 10000);
		else if (fDiff > 10'000)
			strRet.Format(L"%.02f万", fDiff / 10000);
		else
			strRet.Format(L"%.0f", fDiff);
	}
	else
	{
		if (nY > m_rcVolume.bottom || nY < m_rcVolume.top)
			return strRet;
		double fDiff = ((double)m_rcVolume.bottom - nY) / (m_rcVolume.Height() - 5)  * m_pAll->fAmountMax;
		if (fDiff > 1'000'000'000)
			strRet.Format(L"%.01f亿", fDiff / 100'000'000);
		else if (fDiff > 100'000'000)
			strRet.Format(L"%.02f亿", fDiff / 100'000'000);
		else if (fDiff > 1'000'000)
			strRet.Format(L"%.0f万", fDiff / 10000);
		else if (fDiff > 10'000)
			strRet.Format(L"%.02f万", fDiff / 10000);
		else
			strRet.Format(L"%.0f", fDiff);

	}
	return strRet;

}

void SKlinePic::GetMACDMaxDiff()		//判断副图坐标最大最小值和k线条数
{
	//判断最大最小值
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
	double fPos = m_rcMACD.top + (1 - (fDiff / m_pMacdData->fMax))
		/ 2 * (m_rcMACD.Height() - 30) + 25;
	int nPos = (int)fPos;
	return nPos;

}

SStringW SKlinePic::GetMACDYPrice(int nY)
{
	if (!m_pMacdData)
		return L"";
	int nWidth = (m_rcMACD.Height() - 20) / 4;
	SStringW strRet; strRet.Empty();
	if (nY > m_rcMACD.bottom || nY < m_rcMACD.top)
		return strRet;
	double fDiff = ((double)(nWidth * 2 + 20 + m_rcMACD.top) - nY)
		/ (m_rcMACD.Height() - 30) * 2 * m_pMacdData->fMax;
	strRet.Format(L"%.2f", fDiff);
	return strRet;
}

int SKlinePic::GetYPos(double fDiff)
{
	double fPos = fDiff - m_pAll->fMin;
	double fPriceDiff = m_pAll->fMax - m_pAll->fMin;
	int nHeight = m_rcMain.bottom - m_rcMain.top - RC_MAX - RC_MIN - 20;
	fPos = m_rcMain.bottom - fPos / fPriceDiff*nHeight + 0.5 - RC_MIN;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SKlinePic::GetYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	int nHeight = m_rcMain.bottom - RC_MIN - m_rcMain.top - RC_MAX - 20;
	double fPriceDiff = m_pAll->fMax - m_pAll->fMin;
	double fDiff = m_pAll->fMin + (double)(m_rcMain.bottom - nY - RC_MIN)
		/ nHeight*fPriceDiff;
	strRet.Format(SDECIMAL, fDiff);
	return strRet;

}

SStringW SKlinePic::GetAllYPrice(int nY)
{
	if (nY >= m_rcMain.top && nY <= m_rcMain.bottom)
		return  GetYPrice(nY);
	if (nY >= m_rcMACD.top && nY <= m_rcMACD.bottom)
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
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
		::EnterCriticalSection(&m_csSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetMouseMove();
		::LeaveCriticalSection(&m_csSub);

		Invalidate();
		return;
	}

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
}

void SKlinePic::OnTimer(char cTimerID)
{
	if (cTimerID == 1)	//刷新鼠标
	{
		if (m_bDataInited)
		{
			UpdateData();
		}
		Invalidate();
	}
}

int SKlinePic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//	SWindow::SetTimer(1, 1000);
	return 0;
}

void SKlinePic::OnMouseLeave()
{
	SetMsgHandled(FALSE);
	if (m_nMouseX != -1 || m_nMouseY != -1)
	{
		m_nMouseX = m_nMouseY = -1;
		Invalidate();		//主要是为了消除未来得及消除的鼠标线
	}
}

int SKlinePic::GetXData(int nx) {	//获取鼠标下的数据id

	//int nJiange = 2;
	//if (m_nJiange)
	//	nJiange = 0;
	float fn = (float)(nx - m_rcMain.left) /
		(float)TOTALZOOMWIDTH;
	int n = (int)fn;
	if (n < 0)
		n = 0;
	n += m_nFirst;
	return n;

}

int SKlinePic::GetXPos(int nx)
{
	//int nJiange = 2;
	//if (m_nJiange)
	//	nJiange = 0;

	int nPos = nx * TOTALZOOMWIDTH
		+ 1 + m_rcMain.left;
	nPos = nPos + ZOOMWIDTH / 2;
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
	if (m_pAll->nTotal == 0)
		return;

	//画鼠标线
	if (m_pAll->nTotal <= 0)
		return;
	//int  nJianGe = 2;
	//if (m_nJiange)
	//	nJianGe = 0;

	int nx = m_nMouseLinePos - m_nFirst;
	int x = nx * TOTALZOOMWIDTH
		+ 1 + m_rcMain.left;
	CPoint po;
	po.x = x + ZOOMWIDTH / 2;
	po.y = GetYPos(m_pAll->data[m_nMouseLinePos].close);

	DrawMouseLine(pRT, po);

	auto &data = m_pAll->data[m_nMouseLinePos];
	DrawBarInfo(pRT, m_nMouseLinePos);
	::EnterCriticalSection(&m_csSub);

	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->SetNowKeyDownLinePos(nx);
		m_ppSubPic[i]->DrawKeyDownMouseLine(pRT, TRUE);
	}
	::LeaveCriticalSection(&m_csSub);

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
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->DrawMouse(pRT, p, bFromOnPaint);;
	}
	::LeaveCriticalSection(&m_csSub);

	m_nMouseX = p.x;
	m_nMouseY = p.y;
}

void SKlinePic::DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint) //画竖线时间轴时间和标示数字
{
	//int nJiange = 2;
	//if (m_nJiange)
	//	nJiange = 0;

	int nXpre = 0;  //第一根竖线的x轴位置
	int nMaX = (m_nEnd - m_nFirst - 1)*TOTALZOOMWIDTH
		+ RC_LEFT + 1 + ZOOMWIDTH / 2;	//最后一个数据的位置
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
		int x = i * TOTALZOOMWIDTH
			+ 1 + m_rcMain.left + (ZOOMWIDTH / 2);
		int preX = (nPosMx[nPosCount - 1]) * TOTALZOOMWIDTH +
			1 + m_rcMain.left + (ZOOMWIDTH / 2);
		//加时间
		if ((x - nXpre - RC_LEFT >= 10 && x < nMaX - 200) || (i == 0 && m_nEnd - m_nFirst > 9) || i == m_nEnd - m_nFirst - 1)
		{
			//加时间
			SStringW strTime, strDate;
			strTime.Format(L"%02d:%02d", m_pAll->data[i + m_nFirst].time / 100,
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
	//int  nJianGe = 2;
	//if (m_nJiange)
	//	nJianGe = 0;
	if (m_pAll->nTotal <= 0)
		return;

	//	::EnterCriticalSection(&m_cs);

	if (m_bShowMacd)
	{
		for (int i = 0; i < 4; i++)
		{
			int nY = m_rcMACD.top + 20 + (m_rcMACD.Height() - 20) / 4 * i;

			//k线区y轴加轴标
			SStringW s1 = GetMACDYPrice(nY);

			DrawTextonPic(pRT, CRect(m_rcMACD.left - RC_LEFT + 5, nY - 9, m_rcMACD.left, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
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
	vector<CPoint> AmtLine(m_nEnd - m_nFirst);
	for (int i = 0; i < 4; ++i)
		MaLine[i] = new CPoint[m_nEnd - m_nFirst];

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		int nOffset = i + m_nFirst;
		x = i * TOTALZOOMWIDTH + 1 + m_rcMain.left;
		auto data = m_pAll->data[nOffset];
		int nVolume = m_pAll->data[nOffset].vol;
		double fAmount = m_pAll->data[nOffset].amount;
		//if (!(m_rgGroup != Group_Stock && m_strSubIns[0] == '0'))
		//	nVolume = ceil(nVolume / 100);
		//int nVolume1;
		//if (m_nFirst + i > 0)
		//	nVolume1 = m_pAll->data[nOffset - 1].vol;


		if (!m_bShowBandTarget)
		{
			yopen = GetYPos(data.open);
			yhigh = GetYPos(data.high);
			ylow = GetYPos(data.low);
			yclose = GetYPos(data.close);

			//加数值
			int nTimeTmp = 10 + 10;
			if (((i + 1) % nTimeTmp == 0 && m_nEnd - m_nFirst - i > 10 && x < m_rcMain.right - 50) || i == 0 || i == m_nEnd - m_nFirst - 1)
			{

				//加最后的数值
				if (i == m_nEnd - m_nFirst - 1)
				{
					SStringW strTemp;
					strTemp.Format(SDECIMAL, data.close);
					pRT->TextOut(x + ZOOMWIDTH + 2, yclose - 5, strTemp, -1);

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
				pts[0].SetPoint(x + ZOOMWIDTH / 2, yclose);
				pts[1].SetPoint(x + ZOOMWIDTH / 2, yhigh);
				pts[2].SetPoint(x + ZOOMWIDTH / 2, yopen);
				pts[3].SetPoint(x + ZOOMWIDTH / 2, ylow);
			}
			else if (data.close <= data.open)
			{
				if (data.close == data.open)
					pRT->SelectObject(m_penWhite);
				else
					pRT->SelectObject(m_penGreen);
				pts[0].SetPoint(x + ZOOMWIDTH / 2, yopen);
				pts[1].SetPoint(x + ZOOMWIDTH / 2, yhigh);
				pts[2].SetPoint(x + ZOOMWIDTH / 2, yclose);
				pts[3].SetPoint(x + ZOOMWIDTH / 2, ylow);
			}
			pRT->DrawLines(pts, 2);
			pRT->DrawLines(pts + 2, 2);
			if (data.close == data.open)
			{
				pts[0].SetPoint(x, yopen);
				pts[1].SetPoint(x + ZOOMWIDTH, yopen);
				pRT->DrawLines(pts, 2);
			}
			else
			{
				if (data.close >= data.open)
					pRT->DrawRectangle(CRect(x, yclose, x + ZOOMWIDTH,
						yopen == yclose ? yopen + 1 : yopen));
				else
					//pRT->FillSolidRect(CRect(x, yclose, x + m_nKWidth, 
					//	yopen == yclose ? (yopen - 1) : yopen), RGBA(0, 255, 255, 255));
					pRT->FillSolidRect(CRect(x, yopen == yclose ? yopen - 1 : yopen
						, x + ZOOMWIDTH, yclose)
						, RGBA(0, 255, 255, 255));

			}

			if (m_bShowMA)
			{
				auto & arrMA = m_pAll->fMa;
				for (int j = 0; j < 4; ++j)
					if (i + m_nFirst >= m_nMAPara[j] - 1)
						MaLine[j][i].SetPoint(x + ZOOMWIDTH / 2, GetYPos(arrMA[j][nOffset]));
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

				SellLongLine[i].SetPoint(x + ZOOMWIDTH / 2, ySellLong);

				BuyShortLine[i].SetPoint(x + ZOOMWIDTH / 2, yBuyShort);

				UpperLine1[i].SetPoint(x + ZOOMWIDTH / 2, yUpperTrack1);

				LowerLine1[i].SetPoint(x + ZOOMWIDTH / 2, yLowerTrack1);

				UpperLine2[i].SetPoint(x + ZOOMWIDTH / 2, yUpperTrack2);

				LowerLine2[i].SetPoint(x + ZOOMWIDTH / 2, yLowerTrack2);
			}
		}

		if (m_bShowVolume)
		{
			AmtLine[i].SetPoint(x + ZOOMWIDTH / 2, GetFuTuYPos(fAmount,true));
			if (nVolume != 0)
			{

				pRT->SelectObject(m_bBrushGreen);


				pRT->SelectObject(m_penRed);

				if (data.close > data.open)
					pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume),
						x + ZOOMWIDTH, m_rcVolume.bottom));
				else if (data.close == data.open&&m_nFirst + i > 0)
				{
					if (data.close >= p[i - 1].close)
						pRT->DrawRectangle(CRect(x, GetFuTuYPos(nVolume),
							x + ZOOMWIDTH, m_rcVolume.bottom));
					else
						pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume),
							x + ZOOMWIDTH, m_rcVolume.bottom), RGBA(0, 255, 255, 255));
				}
				else
					pRT->FillSolidRect(CRect(x, GetFuTuYPos(nVolume),
						x + ZOOMWIDTH, m_rcVolume.bottom), RGBA(0, 255, 255, 255));
			}
		}

		if (m_bShowMacd)
		{
			double yDIF = GetMACDYPos(m_pMacdData->DIF[nOffset]);
			double yDEA = GetMACDYPos(m_pMacdData->DEA[nOffset]);


			DIFLine[i].SetPoint(x + ZOOMWIDTH / 2, yDIF);

			DEALine[i].SetPoint(x + ZOOMWIDTH / 2, yDEA);


			int nWidthMacd = (m_rcMACD.Height() - 20) / 4;
			//MACD柱状图
			if (m_pMacdData->MACD[nOffset] != 0)
			{
				pts[0].SetPoint(x + ZOOMWIDTH / 2, m_rcMACD.top + 20 + 2 * nWidthMacd);
				pts[1].SetPoint(x + ZOOMWIDTH / 2, GetMACDYPos(m_pMacdData->MACD[i + m_nFirst]));

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
		vector<CAutoRefPtr<IPen>> m_penVec(4);
		m_penVec[0] = m_penWhite;
		m_penVec[1] = m_penYellow;
		m_penVec[2] = m_penPurple;
		m_penVec[3] = m_penMAGreen;

		for (int i = 0; i < 4; ++i)
		{
			if (m_nFirst >= m_nMAPara[i] - 1)
			{
				pRT->SelectObject(m_penVec[i]);
				pRT->DrawLines(MaLine[i], m_nEnd - m_nFirst);
			}
			else if (m_nEnd > m_nMAPara[i] - 1)
			{
				pRT->SelectObject(m_penVec[i]);
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
			int nOffset = i + m_nFirst;
			x = i * TOTALZOOMWIDTH + 1 + m_rcMain.left;
			auto data = m_pAll->data[nOffset];
			int nVolume = m_pAll->data[nOffset].vol;
			DrawBandData(pRT, nOffset, data, x, m_nJiange);
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


	if (m_bShowVolume)
	{
		pRT->SelectObject(m_penWhite);
		pRT->DrawLines(&AmtLine[0], m_nEnd - m_nFirst);
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
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->DrawData(pRT);
	}
	::LeaveCriticalSection(&m_csSub);

}


void SKlinePic::DrawBandData(IRenderTarget * pRT, int nDataPos,
	KlineType &data, int x, int nJiange)
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
		if (yOpen == yClose)
			yOpen = yOpen + 1;
		if (m_pBandData->BB1[nDataPos] == 2)
			pRT->FillSolidRect(CRect(x, yOpen, x + ZOOMWIDTH, yClose),
				RGBA(0, 255, 255, 255));
		else if (m_pBandData->BB1[nDataPos] == 1)
			pRT->FillSolidRect(CRect(x, yOpen, x + ZOOMWIDTH, yClose),
				RGBA(255, 0, 0, 255));
		else if (m_pBandData->BB1[nDataPos] == 3)
			pRT->FillSolidRect(CRect(x, yOpen, x + ZOOMWIDTH, yClose),
				RGBA(255, 255, 0, 255));
		else if (m_pBandData->BB1[nDataPos] == 4)
			pRT->FillSolidRect(CRect(x, yOpen, x + ZOOMWIDTH, yClose),
				RGBA(100, 100, 100, 255));

		CPoint pts[2];

		if (m_pBandData->W2[nDataPos] == 0)
		{
			pts[0].SetPoint(x, yOpen);
			pts[1].SetPoint(x + ZOOMWIDTH, yOpen);
			pRT->DrawLines(pts, 2);
		}

		pts[0].SetPoint(x + ZOOMWIDTH / 2, yOpen);
		pts[1].SetPoint(x + ZOOMWIDTH / 2, yHigh);
		pRT->DrawLines(pts, 2);

		pts[0].SetPoint(x + ZOOMWIDTH / 2, yClose);
		pts[1].SetPoint(x + ZOOMWIDTH / 2, yLow);
		pRT->DrawLines(pts, 2);

	}
	else if (m_pBandData->W2[nDataPos] < 0)
	{
		if (yOpen == yClose)
			yOpen = yOpen - 1;
		pRT->DrawRectangle(CRect(x, yClose, x + ZOOMWIDTH, yOpen));

		CPoint pts[2];
		pts[0].SetPoint(x + ZOOMWIDTH / 2, yClose + 1);
		pts[1].SetPoint(x + ZOOMWIDTH / 2, yHigh);
		pRT->DrawLines(pts, 2);

		pts[0].SetPoint(x + ZOOMWIDTH / 2, yOpen - 1);
		pts[1].SetPoint(x + ZOOMWIDTH / 2, yLow);
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
			KlData.time / 100, KlData.time % 100,
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
			KlData.time / 100, KlData.time % 100,
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
	//m_bWaitData = false;
	SingleDataProc();
	m_nBandCount = 0;
	m_nMacdCount = 0;
	m_bDataInited = true;
	UpdateData();
	//Invalidate();
	//SetTimer(1, 1000);
}


void SKlinePic::ReProcMAData()
{
	m_bDataInited = false;
	for (int i = 0; i < m_pAll->nTotal; i++)
		KlineMAProc(i);
	m_bDataInited = true;
}

void SKlinePic::ReProcMacdData()
{
	m_nMacdCount = 0;
	UpdateData();
}

void SKlinePic::ReProcBandData()
{
	if (m_pBandData)
		ZeroMemory(m_pBandData, sizeof(*m_pBandData));
	m_nBandCount = 0;
	BandDataUpdate();
}

void SKlinePic::ChangePeriod(int nPeriod, BOOL bNeedReCalc)
{
	if (m_nPeriod != nPeriod)
	{
		m_nPeriod = nPeriod;
		if (bNeedReCalc && GetDataReadyState())
			DataProc();
	}
}

void SKlinePic::SetBelongingIndy(vector<SStringA>& strNameVec, int nStartWnd)
{
	m_strL1Indy = strNameVec[0];
	m_strL2Indy = strNameVec[1];
	m_pPriceList->SetIndyName(strNameVec);
	::EnterCriticalSection(&m_csSub);
	for (int i = nStartWnd; i < m_nSubPicNum; ++i)
	{
		auto info = m_ppSubPic[i]->GetSubPicInfo();
		SStringA str;
		if ("L1" == info.range)
			str.Format("行业:%s", strNameVec[0]);
		else if ("L2" == info.range)
			str.Format("行业:%s", strNameVec[1]);
		str.Format("%s %s", info.showName, str);
		m_ppSubPic[i]->SetSubTitleInfo(str);
	}
	::LeaveCriticalSection(&m_csSub);

}

void SKlinePic::GetShowPointInfo(vector<ShowPointInfo>& infoVec)
{
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		infoVec.emplace_back(m_ppSubPic[i]->GetSubPicInfo());
	}
	::LeaveCriticalSection(&m_csSub);

}

BOOL SKlinePic::CheckTargetSelectIsClicked(CPoint pt)
{
	::EnterCriticalSection(&m_csSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		if (m_ppSubPic[i]->CheckIsSelectClicked(pt))
		{
			m_nChangeNum = i;
			return TRUE;
		}
	}
	::LeaveCriticalSection(&m_csSub);

	return FALSE;
}

void SOUI::SKlinePic::CloseSinglePointWnd()
{
	::EnterCriticalSection(&m_csSub);
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic*[m_nSubPicNum - 1];
	for (int i = 0; i < m_nChangeNum; ++i)
		ppTmpSubPicArr[i] = m_ppSubPic[i];
	for (int i = m_nChangeNum; i < m_nSubPicNum - 1; ++i)
		ppTmpSubPicArr[i] = m_ppSubPic[i + 1];
	delete m_ppSubPic[m_nChangeNum];
	delete[]m_ppSubPic;
	m_ppSubPic = ppTmpSubPicArr;
	m_nSubPicNum -= 1;
	::LeaveCriticalSection(&m_csSub);

	Invalidate();
}

void SOUI::SKlinePic::SetSelPointWndInfo(ShowPointInfo & info, SStringA strTitle)
{
	m_ppSubPic[m_nChangeNum]->SetSubPicInfo(info);
	m_ppSubPic[m_nChangeNum]->SetSubTitleInfo(strTitle);

}

void SKlinePic::DrawTickMainData(IRenderTarget * pRT)	//画主图tick
{
	LONGLONG llTmp1 = GetTickCount64();
	if (m_pAll == nullptr || m_pAll->nTotal <= 0)
		return;
	//int  nJianGe = 1;
	//if (m_nKWidth > 4)
	//	nJianGe = 2;

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
		x = i * ZOOMWIDTH + 1 + m_rcMain.left;
		yopen = GetYPos(p[i].open);
		yhigh = GetYPos(p[i].high);
		ylow = GetYPos(p[i].low);
		yclose = GetYPos(p[i].close);
		if (i == 0)
			ypre = yopen;
		//连接前和现开
		pts[nPNum++].SetPoint(x, yopen);
		//画开
		pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, yopen);
		//竖线
		pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, yhigh);
		pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, ylow);
		//画收
		pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, yclose);
		pts[nPNum++].SetPoint(x + ZOOMWIDTH, yclose);
		ypre = yclose;

		//加最后的数值
		if (i == m_nEnd - m_nFirst - 1)
		{
			SStringW strTemp;
			strTemp.Format(SDECIMAL, p[i].close);
			pRT->TextOut(x + ZOOMWIDTH + 2, yclose - 5, strTemp, -1);
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
	//int  nJianGe = 1;
	//if (m_nKWidth > 4)
	//	nJianGe = 2;

	CPoint pts[10000];
	int nPNum = 0, x = 0;
	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;

	pRT->SelectObject(m_penGreen, (IRenderObj**)&oldPen);
	pRT->SelectObject(m_bBrushGreen, (IRenderObj**)&bOldBrush);

	KlineType *p = m_pAll->data + m_nFirst;

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		x = i * ZOOMWIDTH + 1 + m_rcMain.left;
		int volPos = GetFuTuYPos(m_pAll->data[m_nFirst + i - 1].vol);
		if (ZOOMWIDTH > 2)
		{
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2 - 1, m_rcVolume.bottom);
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2 - 1, volPos);
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, volPos);
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, m_rcVolume.bottom);
		}
		else
		{
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, m_rcVolume.bottom);
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, volPos);
			pts[nPNum++].SetPoint(x + ZOOMWIDTH / 2, m_rcVolume.bottom);
		}

	}
	pRT->DrawLines(pts, nPNum);
	//	}
	//补红线
	pRT->SelectObject(m_penRed);
	pts[0].SetPoint(m_rcVolume.left, m_rcVolume.bottom);
	pts[1].SetPoint(m_rcVolume.right, m_rcVolume.bottom);
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
	::EnterCriticalSection(&m_csSub);

	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->SetMouseLineState(m_bShowMouseLine);
	::LeaveCriticalSection(&m_csSub);

	Invalidate();
}

void SKlinePic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	switch (nChar)
	{
	case VK_UP:
	{
		if (m_bKeyDown)
		{
			m_pTip->ClearTip();
			m_bReSetFirstLine = true;
		}
		if (m_nKWidth == 1 && m_nJiange < 2)
		{
			if (m_nZoomRatio > 1)
				--m_nZoomRatio;
			else
				++m_nJiange;

			//m_nJiange = false;
			::EnterCriticalSection(&m_csSub);
			for (int i = 0; i < m_nSubPicNum; ++i)
				m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);
			::LeaveCriticalSection(&m_csSub);

			Invalidate();
			break;
		}
		else if (m_nKWidth == 65)
			break;
		m_nKWidth += 2;
		if (m_nKWidth > 64)
			m_nKWidth = 65;
		::EnterCriticalSection(&m_csSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetShowWidth(m_nKWidth, 2);
		::LeaveCriticalSection(&m_csSub);

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
		if (m_nFirst == 0)
			break;

		if (m_nKWidth == 1)
		{
			//if (m_nJiange )
			//{
				//m_nJiange = true;
			if (m_nJiange > 0)
				--m_nJiange;
			else
				++m_nZoomRatio;
			::EnterCriticalSection(&m_csSub);
			for (int i = 0; i < m_nSubPicNum; ++i)
				m_ppSubPic[i]->SetShowWidth(m_nKWidth, m_nJiange, m_nZoomRatio);
			::LeaveCriticalSection(&m_csSub);

			Invalidate();
			//}
			break;
		}
		m_nKWidth -= 2;
		if (m_nKWidth < 1)
			m_nKWidth = 1;
		::EnterCriticalSection(&m_csSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetShowWidth(m_nKWidth, 2);
		::LeaveCriticalSection(&m_csSub);

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
				if (nx >= m_pAll->nTotal)
					nx = m_pAll->nTotal - 1;
				m_nMouseLinePos = max(nx, 0);
			}
			else
			{
				m_nMouseX = m_nMouseY = -1;
				if (nChar == VK_LEFT)
					m_nMouseLinePos = m_nEnd - 1;
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
				::EnterCriticalSection(&m_csSub);
				for (int i = 0; i < m_nSubPicNum; ++i)
					m_ppSubPic[i]->SetOffset(m_nMove);
				::LeaveCriticalSection(&m_csSub);

				m_bClearTip = true;
				Invalidate();
				break;
			}
			if (m_nMouseLinePos >= m_nFirst + m_nMaxKNum)
			{
				m_nMove--;
				::EnterCriticalSection(&m_csSub);
				for (int i = 0; i < m_nSubPicNum; ++i)
					m_ppSubPic[i]->SetOffset(m_nMove);
				::LeaveCriticalSection(&m_csSub);
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

	HDC hdc = pRT->GetDC();
	int  nMode = SetROP2(hdc, R2_NOTXORPEN);
	MoveToEx(hdc, m_nMouseX, m_rcMain.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcMain.bottom);	//竖线
	MoveToEx(hdc, p.x, m_rcMain.top + 20, NULL);			LineTo(hdc, p.x, m_rcMain.bottom);
	if (m_bShowVolume)
	{
		MoveToEx(hdc, m_nMouseX, m_rcVolume.top, NULL);	LineTo(hdc, m_nMouseX, m_rcVolume.bottom);
		MoveToEx(hdc, p.x, m_rcVolume.top, NULL);			LineTo(hdc, p.x, m_rcVolume.bottom);
	}
	if (m_bShowMacd)
	{
		MoveToEx(hdc, m_nMouseX, m_rcMACD.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcMACD.bottom);
		MoveToEx(hdc, p.x, m_rcMACD.top + 20, NULL);			LineTo(hdc, p.x, m_rcMACD.bottom);

	}
	MoveToEx(hdc, m_rcMain.left, m_nMouseY, NULL);	LineTo(hdc, m_rcMain.right, m_nMouseY);	//横线
	MoveToEx(hdc, m_rcMain.left, p.y, NULL);			LineTo(hdc, m_rcMain.right, p.y);

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
		str.Format(L"%04d-%02d-%02d  %02d:%02d", date / 10000,
			date % 10000 / 100,
			date % 100,
			time / 100,
			time % 100);
		DrawTextonPic(pRT, CRect(x - 35, m_rcImage.bottom + 2, x + 35, m_rcImage.bottom + 35), str, RGBA(255, 255, 255, 255), 0);
	}

}

void SKlinePic::DrawMovePrice(IRenderTarget * pRT, int y, bool bNew)
{
	pRT->SelectObject(m_bBrushBlack);
	if (bNew)
		pRT->FillRectangle(CRect(m_rcMain.right + 1, m_rcMain.top, m_rcMain.right + RC_RIGHT, m_rcMain.bottom /*+ RC_BOTTOM - 10*/));

	if (y >= m_rcVolume.top && y <= m_rcVolume.bottom)
	{
		SStringW sl = GetFuTuYPrice(y);
		DrawTextonPic(pRT, CRect(m_rcMain.left -RC_LEFT, y + -6, m_rcMain.left, y + 15), sl);
		SStringW sr = GetFuTuYPrice(y,true);
		DrawTextonPic(pRT, CRect(m_rcMain.right + 2, y + -6, m_rcMain.right + RC_RIGHT, y + 15), sr);

	}
	else
	{
		SStringW str = GetAllYPrice(y);
		if (str != L"")
			DrawTextonPic(pRT, CRect(m_rcMain.right + 2, y + -6, m_rcMain.right + RC_RIGHT, y + 15), str);

	}

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
	{
		pRT->FillRectangle(CRect(m_rcMACD.left + 1, m_rcMACD.top + 4, m_rcMACD.right, m_rcMACD.top + 19));
		DrawMacdUpperMarket(pRT, nDataPos);
	}
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
	m_nMove = 0;

	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_nTradingDay = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_bAddDay = false;

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
	auto &dataVec = m_pHisKlineMap->at(m_nPeriod);
	int count = 0;

	for (auto &data : dataVec)
	{
		m_pAll->data[count++] = data;
		KlineMAProc(count);
	}
	m_pAll->nTotal = count;

	//bool bStart = false;
	//std::vector<KlineType> recVec;
	//if (m_nPeriod != 1440)
	//{
	//	if (GetFileKlineData(m_strSubIns, recVec))
	//	{
	//		int VecSize = recVec.size();
	//		int count = 0;
	//		for (auto iter = recVec.begin(); iter != recVec.end(); iter++)
	//		{
	//			if (m_nPeriod == 1)
	//			{
	//				if (recVec.end() - iter < 4700)		//1分钟K线
	//				{
	//					m_pAll->data[count++] = *iter;
	//					KlineMAProc(count);
	//				}

	//			}
	//			else
	//			{

	//				if (recVec.end() - iter < 4700 * m_nPeriod)		//5分钟K线
	//				{
	//					if (GenerateMultiMinFromOne(count, *iter, m_nPeriod))
	//						KlineMAProc(++count);
	//				}

	//			}
	//		}
	//		m_pAll->nTotal = count;
	//		if (!recVec.empty())
	//			m_pAll->nLast1MinTime = recVec.back().time;
	//	}
	//}
	//else
	//{
	//	//处理日K线
	//	if (GetFileKlineData(m_strSubIns, recVec, true) && !recVec.empty())
	//	{
	//		int VecSize = recVec.size();
	//		int count = 0;
	//		for (auto iter = recVec.cbegin(); iter != recVec.cend(); iter++)
	//		{
	//			if (recVec.cend() - iter < 4700)		//1分钟K线
	//			{
	//				m_pAll->data[count++] = *iter;
	//				KlineMAProc(count);
	//			}
	//		}
	//		m_pAll->nTotal = count;
	//	}
	//}


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
		else if (m_nPeriod != 1440)
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
		if (TickVec[i].LastPrice == 0)
			continue;
		int ntime = TickVec[i].UpdateTime;
		if (!ProcKlineTime(ntime) || TickVec[i].LastPrice == 0)
			continue;
		if (ntime == 1500 && i + m_nUsedTickCount == 1)
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			StockTickToKline(m_pAll->nTotal, TickVec[i], true, ntime);
			m_pAll->nTotal++;
		}
		else if (ntime == m_pAll->data[m_pAll->nTotal - 1].time && m_nTradingDay == m_pAll->data[m_pAll->nTotal - 1].date)
			StockTickToKline(m_pAll->nTotal - 1, TickVec[i]);
		if (ntime == 1500)
		{
			auto &kline = m_pAll->data[m_pAll->nTotal - 1];
			kline.open = kline.high = kline.low = kline.close;
		}
		KlineMAProc(m_pAll->nTotal);
	}
}

void SKlinePic::StockMarketMultMinUpdate(int nPeriod)
{
	vector<CommonStockMarket> TickVec(m_pStkMarketVec->begin() + m_nUsedTickCount, m_pStkMarketVec->end());
	for (size_t i = 0; i < TickVec.size(); ++i)
	{
		++m_nUsedTickCount;
		if (TickVec[i].LastPrice == 0)
			continue;
		int ntime = TickVec[i].UpdateTime;
		if (!ProcKlineTime(ntime) || TickVec[i].LastPrice == 0)
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
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
		kline.vol = (tick.Volume - m_pAll->nLastVolume) / 100;
		kline.amount = tick.Turnover - m_pAll->fLastAmount;
		kline.time = time;
		kline.date = m_nTradingDay;
	}
	else
	{
		kline.close = tick.LastPrice;
		kline.high = max(tick.LastPrice, kline.high);
		kline.low = min(tick.LastPrice, kline.low);
		kline.vol += (tick.Volume - m_pAll->nLastVolume) / 100;
		kline.amount += tick.Turnover - m_pAll->fLastAmount;

	}
	m_pAll->nLastVolume = tick.Volume;
	m_pAll->fLastAmount = tick.Turnover;
}

void SKlinePic::StockTickToDayKline(int nCount, CommonStockMarket & tick)
{
	auto &kline = m_pAll->data[nCount];
	kline.close = tick.LastPrice;
	kline.open = tick.OpenPrice;
	kline.high = tick.HighPrice;
	kline.low = tick.LowPrice;
	kline.vol = tick.Volume / 100;
	kline.amount = tick.Turnover;
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
		if (TickVec[i].LastPrice == 0)
			continue;
		if (TickVec[i].TradingDay < m_nTradingDay)
			continue;
		int ntime = TickVec[i].UpdateTime/* / 100*/;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
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
		if (TickVec[i].LastPrice == 0)
			continue;
		if (TickVec[i].TradingDay < m_nTradingDay)
			continue;
		int ntime = TickVec[i].UpdateTime /*/ 100*/;
		if (!ProcKlineTime(ntime))
			continue;
		if (m_pAll->nTotal == 0 || ntime > m_pAll->data[m_pAll->nTotal - 1].time ||
			m_nTradingDay > m_pAll->data[m_pAll->nTotal - 1].date)
		{
			//int nLeft = ntime % 100 % nPeriod;
			//ntime -= nLeft;
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
	if (tick.TradingDay < m_nTradingDay)
		return;
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
		kline.amount = tick.Turnover - m_pAll->fLastAmount;
		if (m_strSubIns[0] != '0')
			kline.vol /= 100;
		kline.time = time;
		kline.date = m_nTradingDay;
	}
	else
	{
		kline.close = tick.LastPrice;
		kline.high = max(tick.LastPrice, kline.high);
		kline.low = min(tick.LastPrice, kline.low);
		if (m_strSubIns[0] != '0')
			kline.vol += (tick.Volume - m_pAll->nLastVolume) / 100;
		else
			kline.vol += (tick.Volume - m_pAll->nLastVolume);
		kline.amount += tick.Turnover - m_pAll->fLastAmount;

	}
	m_pAll->nLastVolume = tick.Volume;
	m_pAll->fLastAmount = tick.Turnover;
}

void SKlinePic::IndexTickToDayKline(int nCount, CommonIndexMarket & tick)
{
	auto &kline = m_pAll->data[nCount];
	kline.close = tick.LastPrice;
	kline.open = tick.OpenPrice;
	kline.high = tick.HighPrice;
	kline.low = tick.LowPrice;
	kline.vol = m_strSubIns[0] == '0' ? tick.Volume : tick.Volume / 100;
	kline.amount = tick.Turnover;
	kline.date = m_nTradingDay;
	kline.time = 0;
}





bool SKlinePic::ProcKlineTime(int & time)
{
	//if (time % 100 != 0)
	//	time /= 100;
	//else
	//{
	//	time /= 100;
	//	time--;
	//	if(time % 100 == 59)
	//		time -=40;
	//}
	time /= 100;
	if (time < 925 || (time > 1130 && time < 1300) || time > 1500)
		return false;
	if (time >= 925 && time < 931)
		time = 930;
	if (time != 1130 && time != 1500)
		++time;

	if (m_nPeriod != 1 && m_nPeriod != 1440)
	{
		if (m_nPeriod == 60)
		{
			if (time > 930 && time <= 1030)
				time = 1030;
			else if (time > 1030 && time <= 1130)
				time = 1130;
			else if (time % 100 != 0)
				time = (time / 100 + 1) * 100;
		}
		else
		{
			int nLeft = time % 100 % m_nPeriod;
			if (nLeft != 0)
				time = time + m_nPeriod - nLeft;
		}
	}
	if (time % 100 == 60)
		time += 40;

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
	if (!m_bDataInited)
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
	//int	nJianGe = 2;
	//if (m_nJiange)
	//	nJianGe = 0;

	int nTop = GetYPos(data.high);
	int nBottom = GetYPos(data.low);
	int nLeft = nDataCount * TOTALZOOMWIDTH
		+ 1 + m_rcMain.left;
	int nRight = nLeft + ZOOMWIDTH;

	if (pt.x >= nLeft&&pt.x <= nRight&&pt.y >= nTop&&pt.y <= nBottom)
		return TRUE;
	return FALSE;

}


