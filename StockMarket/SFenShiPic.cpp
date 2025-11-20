#include "stdafx.h"
#include "SFenShiPic.h"
#include <unordered_map>
#include <fstream>
#include"DealList.h"
#include"PriceList.h"
#include<windows.h>
#include<map>
#include<math.h>
#include "SSubTargetPic.h"
#include <helper/SMenu.h>
#include<set>

extern HWND g_MainWnd;


#define sDimical L"%.02f"
#define MAX_SUBWINDOW 5
#define INFOHEIGHT 20
#define UPPERMARGIN 25
#define BOTTOMMARGIN 35
#define DEALBOTTOMMARGIN 5
#define MAININFOCOUNT 2
#define TEXTTOPMARGIN 5
#define TARGETMARGIN 5


vector<COLORREF> colVec = {
	RGBA(255, 0, 255, 255),		//EMA1
	RGBA(0, 255, 0, 255),		//EMA2
	RGBA(200, 200, 200, 255),		//成交
	RGBA(255, 50, 50, 255),		//主买
	RGBA(50, 255, 50, 255),			//主卖
 RGBA(180, 120, 60, 255),		//订
RGBA(255, 150, 100, 255),		//订单买
RGBA(100, 255, 150, 255),		//订单卖
RGBA(50, 50, 50, 255),		//撤
RGBA(200, 100, 150, 255),		//撤单买
RGBA(50, 180, 150, 255),			//撤单卖
};

enum ePriceLineType
{
	ePLT_EMA1,
	ePLT_EMA2,
	ePLT_Trade,
	ePLT_TradeB,
	ePLT_TradeS,
	ePLT_Order,
	ePLT_OrderB,
	ePLT_OrderS,
	ePLT_Delete,
	ePLT_DeleteB,
	ePLT_DeleteS,
	ePLT_Count,
};

enum eFundFlowShotType
{
	eFFS_Null,
	eFFS_OrderVol,
	eFFS_DeleteVol,
	eFFS_OrderNum,
	eFFS_DeleteNum,
};

SFenShiPic::SFenShiPic()
{
	m_nTickPre = 0;
	m_bFocusable = 1; //可以获取焦点
	m_nMouseX = m_nMouseY = -1;
	m_nIndex = -1;
	m_nOldIndex = -1;
	m_pDataInfo = nullptr;

	m_nAllLineNum = 225;

	m_nPaintTick = GetTickCount();
	m_bDataInited = false;

	m_nSubPicNum = 0;
	m_nNowPosition = 0;
	m_bShowMouseLine = false;
	m_bKeyDown = false;
	m_bIsFirstKey = true;
	m_bShowDeal = true;
	m_bShowAvg = true;
	m_bPaintInit = FALSE;


	m_pDealList = new CDealList;
	m_pPriceList = new CPriceList;
	m_ppSubPic = nullptr;
	m_bIsIndex = false;
	m_preMovePt.SetPoint(-1, -1);

	m_nMACDPara[0] = 12;
	m_nMACDPara[1] = 26;
	m_nMACDPara[2] = 9;

	m_nEMAPara[0] = 12;
	m_nEMAPara[1] = 26;
	m_bHalfPrice = false;

	m_bShowOrderPrice = false;
	m_bShowOrderPriceDetail = false;
	m_bShowDeletePriceDetail = false;
	m_nFundFlowShowType = eFFS_Null;

	//::InitializeCriticalSection(&m_mxSub);
}



SFenShiPic::~SFenShiPic()
{
	if (m_pDataInfo)
		delete m_pDataInfo;
	if (m_pPriceList)
		delete m_pPriceList;
	if (m_pDealList)
		delete m_pDealList;
	if (m_ppSubPic)
	{
		for (int i = 0; i < m_nSubPicNum; ++i)
			delete m_ppSubPic[i];
		delete[]m_ppSubPic;
	}

}

void SFenShiPic::InitSubPic(int nNum)
{
	m_nSubPicNum = nNum;
	m_ppSubPic = new SSubTargetPic * [nNum];
	for (int i = 0; i < nNum; ++i)
		m_ppSubPic[i] = new SSubTargetPic;


}

void SFenShiPic::ReSetSubPic(int nNum, vector<ShowPointInfo>& infoVec)
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic * [nNum];
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

}

vector<ShowPointInfo> SFenShiPic::GetSubPicDataToGet(int nNum, map<int, ShowPointInfo>& infoMap)
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	vector<ShowPointInfo> infoVec;
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic * [nNum];
	if (nNum > m_nSubPicNum)
	{
		auto  tmpInfoMap = infoMap;
		for (int i = 0; i < m_nSubPicNum; ++i)
		{
			auto spi = m_ppSubPic[i]->GetSubPicInfo();
			for (auto& it : infoMap)
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
	return infoVec;
}

void SFenShiPic::SetDataPoint(vector<CommonIndexMarket>* pIdxMarketVec)
{
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_pTFMarketVec = nullptr;
	m_bIsIndex = true;
	m_pPriceList->SetDataPoint(pIdxMarketVec);
	m_pDealList->SetDataPoint(pIdxMarketVec);
}

void SFenShiPic::SetDataPoint(vector<CommonStockMarket>* pStkMarketVec, vector<TickFlowMarket>* pTFMarketVec)
{
	m_pStkMarketVec = pStkMarketVec;
	m_pTFMarketVec = pTFMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsIndex = false;
	m_pPriceList->SetDataPoint(pStkMarketVec);
	m_pDealList->SetDataPoint(pStkMarketVec);

}

void SOUI::SFenShiPic::SetFundFlowDataPoint(vector<OrderState>* pOrderStateVec, vector<DeleteState>* pDeleteStateVec,
	vector<TradeState>* pTradeStateVec)
{
	m_pOrderStateVec = pOrderStateVec;
	m_pDeleteStateVec = pDeleteStateVec;
	m_pTradeStateVec = pTradeStateVec;
}

void SFenShiPic::ChangeShowStock(SStringA subIns, SStringA StockName)
{
	m_bDataInited = false;
	m_strSubIns = subIns;
	m_strStockName = StockName;
	m_pPriceList->ChangeShowStock(subIns, m_strStockName);
	m_pDealList->ChangeShowData(subIns);
}


void SFenShiPic::SetSubPicShowData(int nIndex, bool nGroup)
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		m_ppSubPic[i]->SetShowData(nIndex, nGroup);
	}

}

void SFenShiPic::SetSubPicShowData(int nDataCount[],
	vector<vector<vector<CoreData>*>>& data, vector<vector<BOOL>> bRightVec,
	vector<vector<SStringA>> dataNameVec, SStringA StockID, SStringA StockName, int nStartWnd)
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = nStartWnd; i < m_nSubPicNum; ++i)
	{
		int nTargetCount = i - nStartWnd;
		m_ppSubPic[i]->SetShowData(
			nDataCount[nTargetCount], &data[nTargetCount][0],
			bRightVec[nTargetCount], dataNameVec[nTargetCount],
			StockID, StockName);
	}

}

void SFenShiPic::SetSubPicShowData(int nDataCount, vector<vector<CoreData>*>& data, vector<BOOL> bRightVec,
	vector<SStringA> dataNameVec, SStringA StockID, SStringA StockName)
{
	m_ppSubPic[m_nChangeNum]->SetShowData(
		nDataCount, &data[0],
		bRightVec, dataNameVec,
		StockID, StockName);

}




void SFenShiPic::InitShowPara(InitPara_t para)
{
	m_bShowDeal = para.bShowTSCDeal;
	m_bShowMacd = para.bShowTSCMACD;
	m_bShowVolume = para.bShowTSCVolume;
	m_bShowAvg = para.bShowAverage;
	m_bShowEMA = para.bShowEMA;
	m_bShowOrderPrice = para.bShowOrderPrice;
	m_bShowOrderPriceDetail = para.bShowOrderPriceDetail;
	m_bShowDeletePriceDetail = para.bShowDeletePriceDetail;
	m_nFundFlowShowType = para.nFundFlowShowType;

	//for (int i = 0; i < m_nSubPicNum;++i)
	//	m_pbShowSubPic[i] = para.bShowTSCRPS[i];
	{
		std::lock_guard<std::mutex> lock(m_mxSub);
		m_nSubPicNum = para.nTSCPointWndNum;
		m_ppSubPic = new SSubTargetPic*[m_nSubPicNum];
		for (int i = 0; i < m_nSubPicNum; ++i)
		{
			m_ppSubPic[i] = new SSubTargetPic;
			m_ppSubPic[i]->SetSubPicInfo(para.TSCPonitWndInfo[i]);

		}

	}

	m_nEMAPara[0] = para.nEMAPara[0];
	m_nEMAPara[1] = para.nEMAPara[1];
	m_nMACDPara[0] = para.nMACDPara[0];
	m_nMACDPara[1] = para.nMACDPara[1];
	m_nMACDPara[2] = para.nMACDPara[2];

}

void SFenShiPic::OutPutShowPara(InitPara_t& para)
{
	para.bShowTSCDeal = m_bShowDeal;
	para.bShowTSCMACD = m_bShowMacd;
	para.bShowTSCVolume = m_bShowVolume;
	para.bShowAverage = m_bShowAvg;
	para.bShowEMA = m_bShowEMA;
	para.bShowOrderPrice = m_bShowOrderPrice;
	para.bShowOrderPriceDetail = m_bShowOrderPriceDetail;
	para.bShowDeletePriceDetail = m_bShowDeletePriceDetail;
	para.nFundFlowShowType = m_nFundFlowShowType;
	std::lock_guard<std::mutex> lock(m_mxSub);
	para.nTSCPointWndNum = m_nSubPicNum;
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		para.TSCPonitWndInfo.resize(m_nSubPicNum);
		para.TSCPonitWndInfo[i] = m_ppSubPic[i]->GetSubPicInfo();
	}
	para.nEMAPara[0] = m_nEMAPara[0];
	para.nEMAPara[1] = m_nEMAPara[1];
	para.nMACDPara[0] = m_nMACDPara[0];
	para.nMACDPara[1] = m_nMACDPara[1];
	para.nMACDPara[2] = m_nMACDPara[2];

}





void SFenShiPic::OnPaint(IRenderTarget* pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	SWindow::GetClientRect(&m_rcAll);

	//m_bPaintInit = true;
	if (!m_bPaintInit)
	{
		m_bPaintInit = true;
	}

	{
		std::lock_guard<std::mutex> lock(m_mxSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->InitColorAndPen(pRT);
	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);

	m_rcAll.DeflateRect(RC_FSLEFT + 5, RC_FSTOP, RC_FSRIGHT, RC_FSBOTTOM);

	SetWindowRect();

	m_nMiddle = (m_rcMain.top + m_rcMain.bottom) / 2;
	m_nHeight = m_rcMain.bottom - m_rcMain.top - (INFOHEIGHT * MAININFOCOUNT);

	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(192, 192, 192, 255), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		pts[0].SetPoint(m_rcAll.left - RC_FSLEFT + 1, m_rcAll.top - UPPERMARGIN);
		pts[1].SetPoint(m_rcAll.left - RC_FSLEFT + 1, m_rcAll.bottom + BOTTOMMARGIN);
		pts[2].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_rcAll.bottom + BOTTOMMARGIN);
		pts[3].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_rcAll.top - UPPERMARGIN);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		if (m_bShowDeal)
		{
			pts[0].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcAll.top - UPPERMARGIN);
			pts[1].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcAll.bottom + BOTTOMMARGIN);
			pRT->DrawLines(pts, 2);
			pts[0].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_pPriceList->m_rect.bottom - DEALBOTTOMMARGIN);
			pts[1].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_pPriceList->m_rect.bottom - DEALBOTTOMMARGIN);
			pRT->DrawLines(pts, 2);
		}
		pRT->SelectObject(oldPen);
	}


	DrawArrow(pRT);
	DrawPrice(pRT);

	if (m_bDataInited)
	{
		DrawVirtualTimeLine(pRT);
		DrawData(pRT);
	}


	if (m_bShowDeal)
	{
		m_pPriceList->Paint(pRT);
		m_pDealList->Paint(pRT);
	}


	CPoint po(m_nMouseX, m_nMouseY);
	m_nMouseX = m_nMouseY = -1;
	{
		std::lock_guard<std::mutex> lock(m_mxSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
			m_ppSubPic[i]->SetMousePosDefault();
	}

	LONGLONG llTmp3 = GetTickCount64();
	if (m_bKeyDown)
		DrawKeyDownMouseLine(pRT, 0);
	else
		DrawMouse(pRT, po, TRUE);
	AfterPaint(pRT, pa);
}

void SFenShiPic::DrawArrow(IRenderTarget* pRT)
{
	//	if (m_dataVec.empty())
	//		return;
	//画k线区
	int nLen = m_rcMain.bottom - m_rcMain.top;
	int nYoNum = 9;		//y轴标示数量 3 代表画两根线
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
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

	//k线区横向虚线

	COLORREF clRed = RGB(139, 0, 0);
	HDC pdc = pRT->GetDC();
	int width = m_nHeight / 8;
	for (size_t i = 0; i < nYoNum + 1; i++)
	{
		int nY = i >= MAININFOCOUNT ? m_rcMain.top + INFOHEIGHT * MAININFOCOUNT + width * (i - MAININFOCOUNT) :
			m_rcMain.top + INFOHEIGHT * (i + 1);

		//int nY = i<2? m_rcMain.top + 20 * (i+1): 
		//	m_rcMain.top + 40 + width * (i-1);
		CPoint pts[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pts[0].SetPoint(m_rcMain.left, nY);
			pts[1].SetPoint(m_rcMain.right, nY);

			if (i - 2 == nYoNum / 2)
				pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			else if (nY == m_rcMain.bottom)
				pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
			else
				pRT->CreatePen(PS_SOLID, RGBA(150, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pRT->DrawLines(pts, 2);
			pRT->SelectObject(oldPen);
		}
	}


	pRT->ReleaseDC(pdc);




	if (m_bShowVolume)
	{
		//画辅图区
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pts[0].SetPoint(m_rcVolume.left, m_rcVolume.top);
			pts[1].SetPoint(m_rcVolume.left, m_rcVolume.bottom);
			pts[2].SetPoint(m_rcVolume.right, m_rcVolume.bottom);
			pts[3].SetPoint(m_rcVolume.right, m_rcVolume.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
		}

		//副图区横向虚线
		pdc = pRT->GetDC();
		int nY = m_rcVolume.bottom - ((m_rcVolume.bottom - m_rcVolume.top) / 2);
		CPoint pt[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pRT->CreatePen(PS_SOLID, RGBA(150, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pt[0].SetPoint(m_rcMain.left, nY);
			pt[1].SetPoint(m_rcMain.right, nY);
			pRT->DrawLines(pt, 2);
			pRT->SelectObject(oldPen);
		}

		//标注

		pRT->ReleaseDC(pdc);
	}

	//画	MACD区
	if (m_bShowMacd)
		DrawTragetArrow(pRT, m_rcMACD);
	if (m_nFundFlowShowType != eFFS_Null)
		DrawTragetArrow(pRT, m_rcFundFlowVol);

	std::lock_guard<std::mutex> lock(m_mxSub);

	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		//if (m_pbShowSubPic[i])
		m_ppSubPic[i]->DrawArrow(pRT);
	}

}

void SFenShiPic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{

	int nLen = m_rcMain.right - m_rcMain.left;	//判断是否超出范围
	//判断最大最小值

	double fMax = -1000000000;
	double fMin = 1000000000;


	//fMax = m_pDataInfo->fMax;
	//fMin = m_pDataInfo->fMin;

	std::lock_guard<std::mutex> lock(m_mxData);

	for (unsigned i = 0; i < m_dataVec.size(); i++)
	{
		if (m_dataVec[i].close > fMax)
			fMax = m_dataVec[i].close;
		if (m_dataVec[i].close < fMin)
			fMin = m_dataVec[i].close;
	}

	int nFundFlowSize = min(m_dataVec.size(), 239);
	if (m_bShowOrderPrice)
	{
		for (unsigned i = 0; i < nFundFlowSize; i++)
		{
			if (m_pOrderStateVec->at(i).nSid >= 0)
			{
				auto& orderData = m_pOrderStateVec->at(i);
				fMax = max(fMax, orderData.fOrdPrice * 0.01);
				if (orderData.fOrdPrice > 0)
					fMin = min(fMin, orderData.fOrdPrice * 0.01);
			}
		}

	}
	if (m_bShowOrderPriceDetail)
	{
		for (unsigned i = 0; i < nFundFlowSize; i++)
		{
			if (m_pOrderStateVec->at(i).nSid >= 0)
			{
				auto& orderData = m_pOrderStateVec->at(i);
				fMax = max(fMax, max(orderData.fOrdPriceB * 0.01, orderData.fOrdPriceS * 0.01));
				if (orderData.fOrdPriceB > 0)
					fMin = min(fMin, orderData.fOrdPriceB * 0.01);
				if (orderData.fOrdPriceS > 0)
					fMin = min(fMin, orderData.fOrdPriceS * 0.01);
			}
		}

	}

	if (m_bShowDeletePriceDetail)
	{
		for (unsigned i = 0; i < nFundFlowSize; i++)
		{
			if (m_pDeleteStateVec->at(i).nSid >= 0)
			{
				auto& deleteData = m_pDeleteStateVec->at(i);
				fMax = max(fMax, max(deleteData.fDelPriceB * 0.01, deleteData.fDelPriceS * 0.01));
				if (deleteData.fDelPriceB > 0)
					fMin = min(fMin, deleteData.fDelPriceB * 0.01);
				if (deleteData.fDelPriceS > 0)
					fMin = min(fMin, deleteData.fDelPriceS * 0.01);
			}
		}

	}

	if (fMax > 100000000)
		fMax = m_pDataInfo->fPreClose + 1;
	if (fMin > 100000000)
		fMin = m_pDataInfo->fPreClose - 1;

	if (fMax < m_pDataInfo->fPreClose)
		fMax = m_pDataInfo->fPreClose + (m_pDataInfo->fPreClose - fMin);
	else if (fMin > m_pDataInfo->fPreClose)
		fMin = m_pDataInfo->fPreClose + (m_pDataInfo->fPreClose - fMax);
	else
	{
		if ((fMax - m_pDataInfo->fPreClose) > (m_pDataInfo->fPreClose - fMin))
			fMin = m_pDataInfo->fPreClose + (m_pDataInfo->fPreClose - fMax);
		else
			fMax = m_pDataInfo->fPreClose + (m_pDataInfo->fPreClose - fMin);
	}

	m_pDataInfo->fMax = fMax;
	m_pDataInfo->fMin = fMin;
	if (m_pDataInfo->fMax == fMin)
		m_pDataInfo->fMax = m_pDataInfo->fMax * 1.1;
	if (m_pDataInfo->fMax == 0)
		m_pDataInfo->fMax = 1;

	m_pDataInfo->dDelta = m_pDataInfo->fMax - m_pDataInfo->fPreClose;


}

void SFenShiPic::GetFuTuMaxDiff()		//判断副图坐标最大最小值和k线条数
{

	//判断最大最小值
	double fMax = -100000000;
	double fMin = 100000000;


	std::lock_guard<std::mutex> lock(m_mxData);

	int nDataNum = m_dataVec.size();
	if (nDataNum > m_nAllLineNum)
		nDataNum = m_nAllLineNum;
	for (int j = 0; j < nDataNum; j++)
	{
		if (m_dataVec[j].vol < fMin)
			fMin = m_dataVec[j].vol;
		if (m_dataVec[j].vol > fMax)
			fMax = m_dataVec[j].vol;
	}


	fMin = 0;

	m_pDataInfo->fMaxf = fMax;
	m_pDataInfo->fMinf = fMin;
	if (m_pDataInfo->fMaxf == fMin)
		m_pDataInfo->fMaxf = m_pDataInfo->fMaxf * 1.1;
	if (m_pDataInfo->fMaxf == 0)
		m_pDataInfo->fMaxf = 1;

	if (m_dataVec.empty())
		m_pDataInfo->fMaxf = 1;

	if (!m_bIsIndex)
		m_pDataInfo->fMaxf /= 100;



}

BOOL SFenShiPic::IsInRect(int x, int y, int nMode)	//是否在坐标中,0为全部,1为上方,2为下方
{
	CRect* prc;
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
	case 3:
		prc = &m_rcMACD;
	default:
		return FALSE;
	}
	if (x >= prc->left && x <= prc->right &&
		y >= prc->top && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

int SFenShiPic::GetFuTuYPos(double fDiff)	//获得附图y位置
{
	double fPos = m_pDataInfo->fMaxf - fDiff;
	fPos = fPos / (m_pDataInfo->fMaxf - m_pDataInfo->fMinf) *
		(m_rcVolume.bottom - m_rcVolume.top - RC_FSMIN);
	int nPos = (int)fPos;
	nPos = m_rcVolume.top + nPos + RC_FSMIN;
	return nPos;
}

SStringW SFenShiPic::GetFuTuYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	if (nY > m_rcVolume.bottom || nY < m_rcVolume.top)
		return strRet;

	int nDiff = m_rcVolume.bottom - nY;
	double fDiff = (double)nDiff /
		(m_rcVolume.Height() - RC_FSMIN) * m_pDataInfo->fMaxf;
	strRet.Format(L"%.0f", fDiff);
	return strRet;
}

int SFenShiPic::GetMACDYPos(double fDiff)
{
	double fPos = m_rcMACD.top + (1 - (fDiff / m_pDataInfo->fMaxMACD)) / 2 * (m_rcMACD.Height() - (INFOHEIGHT + TARGETMARGIN * 2)) + INFOHEIGHT + TARGETMARGIN;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SFenShiPic::GetMACDYPrice(int nY)
{
	int nWidth = (m_rcMACD.Height() - INFOHEIGHT) / 4;
	SStringW strRet; strRet.Empty();
	if (nY > m_rcMACD.bottom || nY < m_rcMACD.top)
		return strRet;
	double fDiff = ((double)(nWidth * 2 + INFOHEIGHT + m_rcMACD.top) - nY) / (m_rcMACD.Height() - (INFOHEIGHT + TARGETMARGIN * 2)) * 2 * m_pDataInfo->fMaxMACD;
	strRet.Format(L"%.2f", fDiff);
	return strRet;
}

int SOUI::SFenShiPic::GetFundFlowVolYPos(double fDiff)
{
	double fPos = m_nFundFlowVolMax - fDiff;
	fPos = fPos / m_nFundFlowVolMax *
		(m_rcFundFlowVol.bottom - m_rcFundFlowVol.top - TARGETMARGIN - INFOHEIGHT);
	int nPos = (int)fPos;
	nPos = m_rcFundFlowVol.top + nPos + TARGETMARGIN + INFOHEIGHT;
	return nPos;
}

SStringW SOUI::SFenShiPic::GetFundFlowVolYValue(int nY)
{
	int nWidth = (m_rcFundFlowVol.Height() - INFOHEIGHT) / 4;
	SStringW strRet; strRet.Empty();
	if (nY > m_rcFundFlowVol.bottom || nY < m_rcFundFlowVol.top)
		return strRet;
	int nDiff = m_rcFundFlowVol.bottom - nY;
	double fDiff = (double)nDiff /
		(m_rcFundFlowVol.Height() - (INFOHEIGHT + TARGETMARGIN)) * m_nFundFlowVolMax;
	strRet.Format(L"%.0f", fDiff);
	return strRet;
}

void SOUI::SFenShiPic::GetFundFlowMaxDiff()
{
	if (m_nFundFlowShowType == eFFS_Null)
		return;
	int nDataCount = min(m_dataVec.size(), 239);
	m_nFundFlowVolMax = 0;
	if (m_nFundFlowShowType == eFFS_OrderVol)
	{
		for (int i = 0; i < nDataCount; ++i)
		{
			auto& orderData = m_pOrderStateVec->at(i);
			if (orderData.nSid >= 0)
				m_nFundFlowVolMax = max(m_nFundFlowVolMax, orderData.nOrdVolume * 0.01);
		}
		return;
	}
	if (m_nFundFlowShowType == eFFS_DeleteVol)
	{
		for (int i = 0; i < nDataCount; ++i)
		{
			auto& deleteData = m_pDeleteStateVec->at(i);
			if (deleteData.nSid >= 0)
				m_nFundFlowVolMax = max(m_nFundFlowVolMax, deleteData.nDelVolume * 0.01);
		}
		return;
	}
	if (m_nFundFlowShowType == eFFS_OrderNum)
	{
		for (int i = 0; i < nDataCount; ++i)
		{
			auto& orderData = m_pOrderStateVec->at(i);
			if (orderData.nSid >= 0)
				m_nFundFlowVolMax = max(m_nFundFlowVolMax, orderData.nOrdNum);
		}
		return;
	}
	if (m_nFundFlowShowType == eFFS_DeleteNum)
	{
		for (int i = 0; i < nDataCount; ++i)
		{
			auto& deleteData = m_pDeleteStateVec->at(i);
			if (deleteData.nSid >= 0)
				m_nFundFlowVolMax = max(m_nFundFlowVolMax, deleteData.nDelNum);
		}
		return;
	}

}

int SFenShiPic::GetYPos(double fDiff)
{
	int nWidth = m_nHeight / 8;
	double fPos = m_rcMain.top + (1 - ((fDiff - m_pDataInfo->fPreClose) / m_pDataInfo->dDelta)) / 2 * (nWidth * 8) + INFOHEIGHT * MAININFOCOUNT;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SFenShiPic::GetYPrice(int nY, BOOL bIsPercent)
{
	SStringW strRet; strRet.Empty();
	int nWidth = m_nHeight / 8;
	int middle = nWidth * 4 + m_rcMain.top + INFOHEIGHT * MAININFOCOUNT;
	int nDiff = nY - middle;
	double fDiff = nDiff;
	double fPrice = m_pDataInfo->fPreClose - fDiff / (nWidth * 8) * 2 * m_pDataInfo->dDelta;
	if (bIsPercent)
		strRet.Format(L"%.2f%%", (fPrice - m_pDataInfo->fPreClose) / m_pDataInfo->fPreClose * 100);
	else
		strRet.Format(L"%.02f", fPrice);
	return strRet;
}

void SFenShiPic::OnMouseMove(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	int nTick = GetTickCount();
	if (nTick - m_nTickPre < 10 && m_nTickPre > 0)
		return;
	m_nTickPre = nTick;

	if (point == m_preMovePt)
		return;
	m_preMovePt = point;

	m_bIsFirstKey = true;

	CPoint p = point;

	if (m_bKeyDown)
	{
		//DrawKeyDownMouseLine(pRT, 0);
		m_bKeyDown = false;
		{
			std::lock_guard<std::mutex> lock(m_mxSub);
			for (int i = 0; i < m_nSubPicNum; ++i)
				m_ppSubPic[i]->SetMouseMove();
		}
		Invalidate();
		return;
	}

	CRect rc = GetClientRect();
	CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);

	DrawMouse(pRT, point);
	AfterPaint(pRT, pa);
	ReleaseRenderTarget(pRT);
}


void SFenShiPic::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	//SetMsgHandled(FALSE);
	SSendMessage(WM_COMMAND, MAKEWORD(nID, uNotifyCode), (LPARAM)wndCtl);
}

void SFenShiPic::OnTimer(char cTimerID)
{
	if (cTimerID == 1)	//刷新鼠标
	{
		if (m_bDataInited)
			UpdateData();
		Invalidate();
	}
}


void SFenShiPic::OnMouseLeave()
{
	SetMsgHandled(FALSE);
	Invalidate();		//主要是为了消除未来得及消除的鼠标线
}

void SFenShiPic::DrawVirtualTimeLine(IRenderTarget* pRT)
{
	if (m_virTimeLineMap.empty())
		return;
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);

	CAutoRefPtr<IPen> pen, oldPen;
	pRT->CreatePen(PS_DOT, RGBA(150, 0, 0, 0xFF), 2, &pen);
	pRT->SelectObject(pen, (IRenderObj**)&oldPen);

	for (auto& it : m_virTimeLineMap)
	{
		int x = GetXPos(it.first) + width;

		if (it.first != 0 && it.second != L"15:00" && it.second != L"15:15")
		{
			CPoint pt[2];
			pt[0].SetPoint(x, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT);
			pt[1].SetPoint(x, m_rcImage.bottom);
			pRT->DrawLines(pt, 2);
		}
		if (it.second == L"15:00" || it.second == L"15:15")
			DrawTextonPic(pRT, CRect(x - 35, m_rcImage.bottom + TEXTTOPMARGIN, x, m_rcImage.bottom + INFOHEIGHT), it.second, RGBA(255, 255, 255, 255), DT_SINGLELINE);
		else
			DrawTextonPic(pRT, CRect(x, m_rcImage.bottom + TEXTTOPMARGIN, x + 40, m_rcImage.bottom + INFOHEIGHT), it.second, RGBA(255, 255, 255, 255), DT_SINGLELINE);

	}
	pRT->SelectObject(oldPen);
}

void SFenShiPic::DrawUpperMarket(IRenderTarget* pRT, FENSHI_GROUP& data, int id)
{

	DrawMainUpperInfo(pRT, data);
	DrawMainPriceTragetInfo(pRT, data, id);
	DrawMacdUpperInfo(pRT, data);
	DrawFundFlowVolUpperInfo(pRT, id);
}

int SFenShiPic::GetXPos(int n) {	//获取id对应的x坐标
	double fx = m_rcMain.left + ((n + 0.5) * (double)(m_rcMain.Width() - 2) / (double)m_nAllLineNum + 0.5);
	int nx = (int)fx;
	if (nx < m_rcMain.left || nx > m_rcMain.right)
		nx = m_rcMain.left;
	return nx;
}

int SFenShiPic::GetXData(int nx) {	//获取鼠标下的数据id
	double fn = (double)(nx - m_rcMain.left) / ((double)(m_rcMain.Width() - 2) / (double)m_nAllLineNum) - 0.5;
	int n = (int)fn;
	if (n < 0 || n >= (int)m_dataVec.size())
		n = -1;
	return n;
}

void SFenShiPic::DrawTextonPic(IRenderTarget* pRT, CRect rc, SStringW str, COLORREF color, UINT uFormat, DWORD rop)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.right - rc.left, rc.bottom - rc.top);
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfHeight = 12;// 字体大小
	_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
	CAutoRefPtr<IFont> pFont;
	GETRENDERFACTORY->CreateFont(&pFont, lf);
	pMemRT->SelectObject(pFont);

	pMemRT->SetTextColor(color);				//字为白色
	pMemRT->DrawTextW(str, wcslen(str),
		CRect(0, 0, rc.right - rc.left, rc.bottom - rc.top),
		uFormat);
	pRT->BitBlt(rc, pMemRT, 0, 0, rop);

}

CRect SFenShiPic::GetTextDrawRect(IRenderTarget* pRT, SStringW str, CRect rc)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.right - rc.left, rc.bottom - rc.top);
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfHeight = 12;// 字体大小
	_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
	CAutoRefPtr<IFont> pFont;
	GETRENDERFACTORY->CreateFont(&pFont, lf);
	pMemRT->SelectObject(pFont);
	CRect dstRc(0, 0, rc.right - rc.left, rc.bottom - rc.top);
	pMemRT->DrawTextW(str, wcslen(str), dstRc, DT_CALCRECT);
	return dstRc;
}

void SFenShiPic::DrawEarserLine(IRenderTarget* pRT, CPoint pt, bool bVertical)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	if (bVertical)
		GETRENDERFACTORY->CreateRenderTarget(&pMemRT, m_rcImage.Width(), m_rcImage.Height());
	else
		GETRENDERFACTORY->CreateRenderTarget(&pMemRT, m_rcImage.Width(), m_rcImage.Height());

	CAutoRefPtr<IPen> pPen;
	pMemRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &pPen);
	pMemRT->SelectObject(pPen);


	CPoint pts[2];
	if (bVertical)
	{
		pts[0].SetPoint(pt.x, m_rcImage.top);

		pts[1].SetPoint(pt.x, m_rcImage.bottom);
		pMemRT->DrawLines(pts, 2);
		pRT->BitBlt(m_rcImage, pMemRT, 0, 0, SRCINVERT);

	}
	else
	{
		pts[0].SetPoint(m_rcImage.left, pt.y);
		pts[1].SetPoint(m_rcImage.right, pt.y);
		pMemRT->DrawLines(pts, 2);
		pRT->BitBlt(m_rcImage, pMemRT, 0, 0, SRCINVERT);


	}


}

void SFenShiPic::GetMACDMaxDiff()		//判断副图坐标最大最小值和k线条数
{

	//判断最大最小值

	GetMacdDiff();

	if (m_dataVec.empty())
	{
		m_pDataInfo->fMaxMACD = 1;
		m_pDataInfo->fMinMACD = 0;
	}

	double fMin = m_pDataInfo->fMinMACD;
	double fMax = m_pDataInfo->fMaxMACD;
	if (fMax == fMin)
		fMax = fMax * 1.1;
	if (fMax == fMin && fMax == 0)
		fMax = 1;

	fMax = std::fabs(fMax) > std::fabs(fMin) ? std::fabs(fMax) : std::fabs(fMin);
	fMin = -fMax;

	m_pDataInfo->fMaxMACD = fMax;
	m_pDataInfo->fMinMACD = fMin;


}

void SFenShiPic::InitVirTimeLineMap()
{
	m_virTimeLineMap.clear();
	m_virTimeLineMap[0] = L"09:30";
	m_virTimeLineMap[121] = L"13:00";
	m_virTimeLineMap[240] = L"15:00";

}

void SFenShiPic::DataProc()
{
	DataInit();
	m_bDataInited = true;
	if (m_bIsIndex)
		IndexDataUpdate();
	else
		StockDataUpdate();
	GetMaxDiff();
	GetFuTuMaxDiff();
	GetMACDMaxDiff();
	GetFundFlowMaxDiff();
}


void SFenShiPic::IndexDataUpdate()
{

	if (m_pIdxMarketVec->empty())
		return;

	double OpenPrice = m_pIdxMarketVec->back().OpenPrice;
	if (OpenPrice > 10000000)
		return;

	int TickSize = m_pIdxMarketVec->size();
	std::lock_guard<std::mutex> lock(m_mxData);
	if (m_pDataInfo->nCount == TickSize - 1)
		return;

	for (size_t i = m_pDataInfo->nCount; i < TickSize; ++i)
	{
		auto& tick = m_pIdxMarketVec->at(i);
		int time = tick.UpdateTime / 100;
		++m_pDataInfo->nCount;
		if (m_pDataInfo->nCount >= TickSize)
			m_pDataInfo->nCount = TickSize - 1;
		if (time < 925)
			continue;
		if (time == 1500 && i == 0)
			continue;

		if (time > 1500)
			break;

		int min = SetFenshiMin(time);
		if (-1 == min)
			min = time % 100;

		if (min != m_pDataInfo->nMin)
		{
			FENSHI_GROUP f1 = { 0 };
			if (TimeInGap(time))
			{
				SetFSData(f1, tick);
				SetGapMarketTime(f1, time);
				if (m_timeSet.count(f1.time) == 0)
					continue;
				if (!m_dataVec.empty())
					m_dataVec.pop_back();
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
				HandleMissData(f1, time);
			}
			else if (i != 0)
			{

				auto& preTick = m_pIdxMarketVec->at(i - 1);
				f1.time = preTick.UpdateTime / 100;
				if (f1.time > 925 && f1.time < 930)
					f1.time = 925;
				if (f1.time > 1129 && f1.time < 1300)
					f1.time = 1129;
				if (m_timeSet.count(f1.time) == 0)
					continue;
				SetFSData(f1, preTick);
				m_pDataInfo->nMin = min;
				if (!m_dataVec.empty())
					m_dataVec.pop_back();
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
				m_pDataInfo->nLastVolume = preTick.Volume;
				HandleMissData(f1, time);

				ZeroMemory(&f1, sizeof(f1));
				SetFSData(f1, tick);
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
			}

		}
		else
		{
			if (m_timeSet.count(time) == 0)
				continue;
			if (m_dataVec.empty())
				m_dataVec.emplace_back(FENSHI_GROUP());
			auto& f1 = m_dataVec.back();
			SetFSData(f1, tick);
			MACDHandle(f1, 1);
			HandleMissData(f1, time);
		}
		m_pDataInfo->fMax = tick.HighPrice;
		m_pDataInfo->fMin = tick.LowPrice;
	}
}

void SFenShiPic::StockDataUpdate()
{

	if (m_pStkMarketVec->empty())
		return;

	double OpenPrice = m_pStkMarketVec->back().OpenPrice;
	if (OpenPrice > 10000000)
		return;

	int TickSize = m_pStkMarketVec->size();
	std::lock_guard<std::mutex> lock(m_mxData);

	if (m_pDataInfo->nCount == TickSize - 1)
		return;

	for (size_t i = m_pDataInfo->nCount; i < TickSize; ++i)
	{
		auto& tick = m_pStkMarketVec->at(i);
		int time = tick.UpdateTime / 100;
		++m_pDataInfo->nCount;
		if (m_pDataInfo->nCount >= TickSize)
			m_pDataInfo->nCount = TickSize - 1;
		if (time < 925)
			continue;
		if (time == 1500 && i == 0)
			continue;

		if (time > 1500)
			break;

		int min = SetFenshiMin(time);
		if (-1 == min)
			min = time % 100;

		if (min != m_pDataInfo->nMin)
		{
			FENSHI_GROUP f1 = { 0 };
			if (TimeInGap(time))
			{
				SetFSData(f1, tick);
				SetGapMarketTime(f1, time);
				if (m_timeSet.count(f1.time) == 0)
					continue;
				if (!m_dataVec.empty())
					m_dataVec.pop_back();
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
				HandleMissData(f1, time);
			}
			else if (i != 0)
			{

				auto& preTick = m_pStkMarketVec->at(i - 1);
				f1.time = preTick.UpdateTime / 100;
				if (f1.time > 925 && f1.time < 930)
					f1.time = 925;
				if (f1.time > 1129 && f1.time < 1300)
					f1.time = 1129;
				if (m_timeSet.count(f1.time) == 0)
					continue;
				SetFSData(f1, preTick);
				m_pDataInfo->nMin = min;
				if (!m_dataVec.empty())
					m_dataVec.pop_back();
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
				m_pDataInfo->nLastVolume = preTick.Volume;
				HandleMissData(f1, time);

				ZeroMemory(&f1, sizeof(f1));
				SetFSData(f1, tick);
				MACDHandle(f1);
				m_dataVec.emplace_back(f1);
			}

		}
		else
		{
			if (m_timeSet.count(time) == 0)
				continue;

			if (m_dataVec.empty())
			{
				FENSHI_GROUP f1 = { 0 };
				SetFSData(f1, tick);
				m_dataVec.emplace_back(f1);
			}
			else
				SetFSData(m_dataVec.back(), tick);
			auto& f1 = m_dataVec.back();
			MACDHandle(f1, 1);
			HandleMissData(f1, time);
		}
		m_pDataInfo->fMax = tick.HighPrice;
		m_pDataInfo->fMin = tick.LowPrice;
	}
}



void SFenShiPic::SetFSData(FENSHI_GROUP& f, CommonIndexMarket& market)
{
	f.close = market.LastPrice;
	f.avg = market.LastPrice;
	f.vol = market.Volume - m_pDataInfo->nLastVolume;

	f.date = market.TradingDay;
	f.time = market.UpdateTime / 100;
}

void SFenShiPic::SetFSData(FENSHI_GROUP& f, CommonStockMarket& market)
{
	f.close = market.LastPrice == 0 ? market.PreCloPrice : market.LastPrice;
	f.avg = market.Volume == 0 ? market.LastPrice : market.Turnover / market.Volume;
	f.vol = market.Volume - m_pDataInfo->nLastVolume;

	f.date = m_nTradingDay;
	f.time = market.UpdateTime / 100;
	if (f.time < 930) f.time = 925;
	if (f.time > 1129 && f.time < 1300) f.time = 1129;

}


void SFenShiPic::SetGapMarketTime(FENSHI_GROUP& f, int time)
{
	if (time == 1130)
		f.time = 1129;
	else if (time == 1500)
		f.time = 1459;
}


void SFenShiPic::DeleteLastData()
{
	//删除最后一条数据为交易时段最后一分钟的数据
	std::lock_guard<std::mutex> lock(m_mxData);
	if (!m_dataVec.empty() &&
		(m_dataVec.at(m_dataVec.size() - 1).time == 1129
			|| m_dataVec.at(m_dataVec.size() - 1).time == 1459))
		m_dataVec.pop_back();
}


void SFenShiPic::ReProcEMA()
{
	std::lock_guard<std::mutex> lock(m_mxData);
	for (size_t i = 0; i < m_dataVec.size(); i++)
	{
		if (i == 0)
		{
			m_dataVec[i].EMA1 = m_dataVec[i].close;
			m_dataVec[i].EMA2 = m_dataVec[i].close;
		}
		else
		{
			m_dataVec[i].EMA1 = EMA(m_nEMAPara[0], m_dataVec[i - 1].EMA1, m_dataVec[i - 1].close);
			m_dataVec[i].EMA2 = EMA(m_nEMAPara[1], m_dataVec[i - 1].EMA2, m_dataVec[i - 1].close);
		}
	}
	//	Invalidate();
}

void SFenShiPic::SetPriceListHalf(bool bHalf)
{
	m_bHalfPrice = bHalf;
	m_pPriceList->SetPriceListHalf(m_bHalfPrice);
}

void SFenShiPic::SetBelongingIndy(vector<SStringA>& strNameVec, int nStartWnd)
{
	m_strL1Indy = strNameVec[0];
	m_strL2Indy = strNameVec[1];
	m_pPriceList->SetIndyName(strNameVec);
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = nStartWnd; i < m_nSubPicNum; ++i)
	{
		auto info = m_ppSubPic[i]->GetSubPicInfo();
		SStringA str;
		if ("L1" == info.dataInRange)
			str.Format("行业:%s", strNameVec[0]);
		else if ("L2" == info.dataInRange)
			str.Format("行业:%s", strNameVec[1]);
		str.Format("%s %s", info.showName, str);
		m_ppSubPic[i]->SetSubTitleInfo(str);
	}

}

void SFenShiPic::GetShowPointInfo(vector<ShowPointInfo>& infoVec)
{
	std::lock_guard<std::mutex> lock(m_mxSub);

	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		//if (m_pbShowSubPic[i])
		infoVec.emplace_back(m_ppSubPic[i]->GetSubPicInfo());
	}


}

BOOL SFenShiPic::CheckTargetSelectIsClicked(CPoint pt)
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		if (m_ppSubPic[i]->CheckIsSelectClicked(pt))
		{
			m_nChangeNum = i;
			return TRUE;
		}
	}

	return FALSE;
}

void SFenShiPic::CloseSinglePointWnd()
{
	std::lock_guard<std::mutex> lock(m_mxSub);
	SSubTargetPic** ppTmpSubPicArr = new SSubTargetPic * [m_nSubPicNum - 1];
	for (int i = 0; i < m_nChangeNum; ++i)
		ppTmpSubPicArr[i] = m_ppSubPic[i];
	for (int i = m_nChangeNum; i < m_nSubPicNum - 1; ++i)
		ppTmpSubPicArr[i] = m_ppSubPic[i + 1];
	delete m_ppSubPic[m_nChangeNum];
	delete[]m_ppSubPic;
	m_ppSubPic = ppTmpSubPicArr;
	m_nSubPicNum -= 1;
	Invalidate();
}

void SFenShiPic::SetSelPointWndInfo(ShowPointInfo& info, SStringA strTitle)
{
	m_ppSubPic[m_nChangeNum]->SetSubPicInfo(info);
	m_ppSubPic[m_nChangeNum]->SetSubTitleInfo(strTitle);

}

bool SFenShiPic::TimeInGap(int time)
{
	if (time == 1130 || time == 1500)
		return true;
	return false;
}

bool SFenShiPic::TimeAfterGap(int time)
{
	if (time == 1300)
		return true;
	return false;
}


bool SFenShiPic::TimeAfterGapHandle(int time, FENSHI_GROUP& f)
{
	if (TimeAfterGap(time))
	{
		//如果最后一条数据是上一个交易时段最后一条
		//重新处理上一条数据
		std::lock_guard<std::mutex> lock(m_mxData);
		if (!m_dataVec.empty() && (m_dataVec.at(m_dataVec.size() - 1).time == 1129))
		{
			f.time = 1129;
			m_dataVec.pop_back();
		}
	}
	return true;
}

void SFenShiPic::ReProcMacd()
{

	std::lock_guard<std::mutex> lock(m_mxData);

	if (!m_dataVec.empty())
	{
		double fMax = -10000000;
		double fMin = 10000000;

		for (auto iter = m_dataVec.begin() + 1; iter != m_dataVec.end(); iter++)
		{
			iter->macd.dEMA12 = EMA(m_nMACDPara[0], (iter - 1)->macd.dEMA12, iter->close);
			iter->macd.dEMA26 = EMA(m_nMACDPara[1], (iter - 1)->macd.dEMA26, iter->close);
			iter->macd.dDIF = iter->macd.dEMA12 - iter->macd.dEMA26;
			iter->macd.dDEA = EMA(m_nMACDPara[2], (iter - 1)->macd.dDEA, iter->macd.dDIF);
			iter->macd.dMACD = (iter->macd.dDIF - iter->macd.dDEA) * 2;
			if (fMax < iter->macd.dDIF)
				fMax = iter->macd.dDIF;
			if (fMax < iter->macd.dDEA)
				fMax = iter->macd.dDEA;
			if (fMax < iter->macd.dMACD)
				fMax = iter->macd.dMACD;
			if (fMin > iter->macd.dDIF)
				fMin = iter->macd.dDIF;
			if (fMin > iter->macd.dDEA)
				fMin = iter->macd.dDEA;
			if (fMin > iter->macd.dDIF)
				fMin = iter->macd.dMACD;
		}
		m_pDataInfo->fMaxMACD = fMax;
		m_pDataInfo->fMinMACD = fMin;

	}

}

void SFenShiPic::UpdateData()
{
	if (m_pDataInfo == nullptr)
		return;
	if (!m_bDataInited)
		return;
	if (m_bIsIndex)
		IndexDataUpdate();
	else
		StockDataUpdate();
	GetMaxDiff();
	GetFuTuMaxDiff();
	GetMACDMaxDiff();
	GetFundFlowMaxDiff();
}

void SFenShiPic::SetWindowRect()
{
	if (!m_bShowDeal)
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top,
			m_rcAll.right, m_rcAll.bottom);
	else
	{
		if (m_bIsIndex)
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top, m_rcAll.right + 40, m_rcAll.top + 160);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top + 165, m_rcAll.right + 40, m_rcAll.bottom + 30);
		}
		else
		{
			if (m_bHalfPrice)
			{
				m_pPriceList->m_rect.SetRect(m_rcAll.right - 182,
					m_rcAll.top, m_rcAll.right + 40, m_rcAll.top + 375);
				m_pDealList->m_rect.SetRect(m_rcAll.right - 182,
					m_rcAll.top + 378, m_rcAll.right + 40, m_rcAll.bottom + 30);

			}
			else
			{
				m_pPriceList->m_rect.SetRect(m_rcAll.right - 182,
					m_rcAll.top, m_rcAll.right + 40, m_rcAll.top + 580);
				m_pDealList->m_rect.SetRect(m_rcAll.right - 182,
					m_rcAll.top + 585, m_rcAll.right + 40, m_rcAll.bottom + 30);

			}
		}
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top,
			m_rcAll.right - 240, m_rcAll.bottom);
	}

	vector<CRect*>pSubRect;
	if (m_bShowVolume)
		pSubRect.emplace_back(&m_rcVolume);
	else m_rcVolume.SetRectEmpty();
	if (m_nFundFlowShowType != eFFS_Null)
		pSubRect.emplace_back(&m_rcFundFlowVol);
	else m_rcFundFlowVol.SetRectEmpty();

	if (m_bShowMacd)
		pSubRect.emplace_back(&m_rcMACD);
	else m_rcMACD.SetRectEmpty();

	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		pSubRect.emplace_back(m_ppSubPic[i]->GetPicRect());

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

int SFenShiPic::SetFenshiMin(int nTime, bool bSetData)
{
	if (nTime >= 925 && nTime < 930)
		return 25;
	return -1;
}

void SFenShiPic::GetMacdDiff()
{
	std::lock_guard<std::mutex> lock(m_mxData);

	m_pDataInfo->fMinMACD = 10;
	m_pDataInfo->fMaxMACD = -10;
	for (size_t i = 0; i < m_dataVec.size(); i++)
	{
		if (m_dataVec[i].macd.dDEA < m_pDataInfo->fMinMACD)
			m_pDataInfo->fMinMACD = m_dataVec[i].macd.dDEA;
		if (m_dataVec[i].macd.dDEA > m_pDataInfo->fMaxMACD)
			m_pDataInfo->fMaxMACD = m_dataVec[i].macd.dDEA;
		if (m_dataVec[i].macd.dDIF < m_pDataInfo->fMinMACD)
			m_pDataInfo->fMinMACD = m_dataVec[i].macd.dDIF;
		if (m_dataVec[i].macd.dDIF > m_pDataInfo->fMaxMACD)
			m_pDataInfo->fMaxMACD = m_dataVec[i].macd.dDIF;
		if (m_dataVec[i].macd.dMACD < m_pDataInfo->fMinMACD)
			m_pDataInfo->fMinMACD = m_dataVec[i].macd.dMACD;
		if (m_dataVec[i].macd.dMACD > m_pDataInfo->fMaxMACD)
			m_pDataInfo->fMaxMACD = m_dataVec[i].macd.dMACD;
	}
}

void SFenShiPic::MACDHandle(FENSHI_GROUP& f1, int nOffset)
{
	if (m_dataVec.size() <= nOffset)
	{
		f1.EMA1 = f1.close;
		f1.EMA2 = f1.close;
		f1.macd.dEMA12 = f1.close;
		f1.macd.dEMA26 = f1.close;
		f1.macd.dDIF = 0;
		f1.macd.dDEA = 0;
		f1.macd.dMACD = 0;
	}
	else
	{
		size_t size = m_dataVec.size();
		auto& LastData = m_dataVec[size - nOffset - 1];
		f1.EMA1 = EMA(m_nEMAPara[0], LastData.macd.dEMA12, f1.close);
		f1.EMA2 = EMA(m_nEMAPara[1], LastData.macd.dEMA26, f1.close);
		f1.macd.dEMA12 = EMA(m_nMACDPara[0], LastData.macd.dEMA12, f1.close);
		f1.macd.dEMA26 = EMA(m_nMACDPara[1], LastData.macd.dEMA26, f1.close);
		f1.macd.dDIF = f1.macd.dEMA12 - f1.macd.dEMA26;
		f1.macd.dDEA = EMA(m_nMACDPara[2], LastData.macd.dDEA, f1.macd.dDIF);
		f1.macd.dMACD = 2 * (f1.macd.dDIF - f1.macd.dDEA);
	}

}

void SFenShiPic::HandleNoDataTime(FENSHI_GROUP f1)
{
	//std::lock_guard<std::mutex> lock(m_mxData);

	if (m_dataVec.size() == 1)
	{
		std::vector<int> timeVec(m_timeSet.begin(), m_timeSet.end());
		std::sort(timeVec.begin(), timeVec.end());
		std::vector<int> TodayTime;
		for (auto& it : timeVec)
		{
			if (it < f1.time)
				TodayTime.emplace_back(it);
		}
		if (!TodayTime.empty())
		{
			m_dataVec.clear();
			FENSHI_GROUP data = f1;
			data.close = data.avg = m_pDataInfo->fPreClose;
			data.vol = 0;
			for (auto& it : TodayTime)
			{
				data.time = it;
				MACDHandle(data);
				m_dataVec.emplace_back(data);
			}
			MACDHandle(f1);
			m_dataVec.emplace_back(f1);

		}
	}

}

void SFenShiPic::HandleMissData(FENSHI_GROUP f1, int time)//补全遗漏的数据
{
	HandleNoDataTime(f1);
	//std::lock_guard<std::mutex> lock(m_mxData);
	if (time - f1.time > 1)
	{
		std::vector<int> timeVec(m_timeSet.begin(), m_timeSet.end());
		std::sort(timeVec.begin(), timeVec.end());
		std::vector<int> TodayTime;
		for (auto& it : timeVec)
			if (it < time && it >f1.time)
				TodayTime.emplace_back(it);
		if (!TodayTime.empty())
		{
			FENSHI_GROUP data = f1;
			data.vol = 0;
			for (auto& it : TodayTime)
			{
				data.time = it;
				MACDHandle(data);
				m_dataVec.emplace_back(data);
			}

		}
	}


}

void SFenShiPic::DrawMouseLine(IRenderTarget* pRT, CPoint po)
{
	HDC hdc = pRT->GetDC();
	int  nMode = SetROP2(hdc, R2_NOTXORPEN);
	MoveToEx(hdc, m_nMouseX, m_rcMain.top + INFOHEIGHT * MAININFOCOUNT, NULL);	LineTo(hdc, m_nMouseX, m_rcMain.bottom);	//竖线
	MoveToEx(hdc, po.x, m_rcMain.top + INFOHEIGHT * MAININFOCOUNT, NULL);			LineTo(hdc, po.x, m_rcMain.bottom);
	if (m_bShowVolume)
	{
		MoveToEx(hdc, m_nMouseX, m_rcVolume.top, NULL);	LineTo(hdc, m_nMouseX, m_rcVolume.bottom);
		MoveToEx(hdc, po.x, m_rcVolume.top, NULL);			LineTo(hdc, po.x, m_rcVolume.bottom);
	}

	if (m_nFundFlowShowType != eFFS_Null)
	{
		MoveToEx(hdc, m_nMouseX, m_rcFundFlowVol.top + INFOHEIGHT, NULL);	LineTo(hdc, m_nMouseX, m_rcFundFlowVol.bottom);
		MoveToEx(hdc, po.x, m_rcFundFlowVol.top + INFOHEIGHT, NULL);			LineTo(hdc, po.x, m_rcFundFlowVol.bottom);

	}

	if (m_bShowMacd)
	{
		MoveToEx(hdc, m_nMouseX, m_rcMACD.top + INFOHEIGHT, NULL);	LineTo(hdc, m_nMouseX, m_rcMACD.bottom);
		MoveToEx(hdc, po.x, m_rcMACD.top + INFOHEIGHT, NULL);			LineTo(hdc, po.x, m_rcMACD.bottom);

	}
	MoveToEx(hdc, m_rcMain.left, m_nMouseY, NULL);	LineTo(hdc, m_rcMain.right, m_nMouseY);	//横线
	MoveToEx(hdc, m_rcMain.left, po.y, NULL);			LineTo(hdc, m_rcMain.right, po.y);

	SetROP2(hdc, nMode);

	pRT->ReleaseDC(hdc);
}

void SOUI::SFenShiPic::DrawFundFlowPriceLine(IRenderTarget* pRT, int nDataNum)
{
	vector<CPoint> OrderPriceLine;
	vector<CPoint> OrderPriceBLine;
	vector<CPoint> OrderPriceSLine;
	vector<CPoint> DeletePriceBLine;
	vector<CPoint> DeletePriceSLine;

	int nShowCount = 0;

	if (m_bShowOrderPrice)
	{
		OrderPriceLine.resize(nDataNum);
		nShowCount++;
	}
	if (m_bShowOrderPriceDetail)
	{
		OrderPriceBLine.resize(nDataNum);
		OrderPriceSLine.resize(nDataNum);
		nShowCount++;

	}
	if (m_bShowDeletePriceDetail)
	{
		DeletePriceBLine.resize(nDataNum);
		DeletePriceSLine.resize(nDataNum);
		nShowCount++;
	}
	if (nShowCount == 0)
		return;
	int yOrderPrice = 0, yOrderPriceB = 0, yOrderPriceS = 0, yDeletePriceB = 0, yDeletePriceS = 0;
	vector<CAutoRefPtr<IPen>> PriceLinePenVec(ePLT_Count);
	for (int i = 0; i < ePLT_Count; ++i)
		pRT->CreatePen(PS_SOLID, colVec[i], 1, &PriceLinePenVec[i]);
	yOrderPrice = yOrderPriceB = yOrderPriceS = yDeletePriceB = yDeletePriceS = GetYPos(m_dataVec[0].close);
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);
	for (int i = 0; i < nDataNum; ++i)
	{
		int x = GetXPos(i);
		int nDataPos = min(i, 238);
		if (m_bShowOrderPrice)
		{
			auto& orderData = m_pOrderStateVec->at(nDataPos);
			if (orderData.nSid >= 0 && orderData.fOrdPrice > 0)
				yOrderPrice = GetYPos(orderData.fOrdPrice * 0.01);
			OrderPriceLine[i].SetPoint(x + width, yOrderPrice);
		}

		if (m_bShowOrderPriceDetail)
		{
			auto& orderData = m_pOrderStateVec->at(nDataPos);
			if (orderData.nSid >= 0)
			{
				if (orderData.fOrdPriceB > 0)
					yOrderPriceB = GetYPos(orderData.fOrdPriceB * 0.01);
				if (orderData.fOrdPriceS > 0)
					yOrderPriceS = GetYPos(orderData.fOrdPriceS * 0.01);

			}
			OrderPriceBLine[i].SetPoint(x + width, yOrderPriceB);
			OrderPriceSLine[i].SetPoint(x + width, yOrderPriceS);

		}

		if (m_bShowDeletePriceDetail)
		{
			auto& DeleteData = m_pDeleteStateVec->at(nDataPos);
			if (DeleteData.nSid >= 0)
			{
				if (DeleteData.fDelPriceB > 0)
					yDeletePriceB = GetYPos(DeleteData.fDelPriceB * 0.01);
				if (DeleteData.fDelPriceS > 0)
					yDeletePriceS = GetYPos(DeleteData.fDelPriceS * 0.01);

			}
			DeletePriceBLine[i].SetPoint(x + width, yDeletePriceB);
			DeletePriceSLine[i].SetPoint(x + width, yDeletePriceS);

		}

	}
	if (m_bShowOrderPrice)
	{
		pRT->SelectObject(PriceLinePenVec[ePLT_Order]);
		pRT->DrawLines(&OrderPriceLine[0], nDataNum);

	}

	if (m_bShowOrderPriceDetail)
	{
		pRT->SelectObject(PriceLinePenVec[ePLT_OrderB]);
		pRT->DrawLines(&OrderPriceBLine[0], nDataNum);
		pRT->SelectObject(PriceLinePenVec[ePLT_OrderS]);
		pRT->DrawLines(&OrderPriceSLine[0], nDataNum);

	}

	if (m_bShowDeletePriceDetail)
	{
		pRT->SelectObject(PriceLinePenVec[ePLT_DeleteB]);
		pRT->DrawLines(&DeletePriceBLine[0], nDataNum);
		pRT->SelectObject(PriceLinePenVec[ePLT_DeleteS]);
		pRT->DrawLines(&DeletePriceSLine[0], nDataNum);
	}

}

void SOUI::SFenShiPic::DrawFundFlowVol(IRenderTarget* pRT, int nDataNum)
{
	if (m_nFundFlowShowType == eFFS_Null)
		return;
	CAutoRefPtr<IPen>penRed, penBlue;
	pRT->CreatePen(PS_SOLID, RGBA(255, 20, 0, 255), 1, &penRed);
	pRT->CreatePen(PS_SOLID, RGBA(0, 255, 255, 255), 1, &penBlue);
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);
	for (int i = 0; i < nDataNum; ++i)
	{
		if (i >= 238 && i <= 239)
			continue;

		int x = GetXPos(i);
		int nDataPos = min(i, 238);
		CPoint ptsSell[2];
		CPoint ptsBuy[2];
		bool bDrawSell = false;
		bool bDrawBuy = false;
		int nBottom = m_rcFundFlowVol.bottom - 1;
		switch (m_nFundFlowShowType)
		{
		case eFFS_OrderVol:
		{
			auto& orderData = m_pOrderStateVec->at(nDataPos);
			if (orderData.nSid >= 0)
			{
				if (orderData.nOrdVolumeS > 0)
				{
					bDrawSell = true;
					ptsSell[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(orderData.nOrdVolumeS * 0.01);
					ptsSell[1].SetPoint(x + width, nBottom);
				}
				if (orderData.nOrdVolumeB > 0)
				{
					bDrawBuy = true;
					ptsBuy[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(orderData.nOrdVolume * 0.01);
					ptsBuy[1].SetPoint(x + width, nBottom);
				}
			}
		}
		break;
		case eFFS_DeleteVol:
		{
			auto& deleteData = m_pDeleteStateVec->at(nDataPos);
			if (deleteData.nSid >= 0)
			{
				if (deleteData.nDelVolumeS > 0)
				{
					bDrawSell = true;
					ptsSell[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(deleteData.nDelVolumeS * 0.01);
					ptsSell[1].SetPoint(x + width, nBottom);
				}
				if (deleteData.nDelVolumeB > 0)
				{
					bDrawBuy = true;
					ptsBuy[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(deleteData.nDelVolume * 0.01);
					ptsBuy[1].SetPoint(x + width, nBottom);
				}
			}
		}
		break;
		case eFFS_OrderNum:
		{
			auto& orderData = m_pOrderStateVec->at(nDataPos);
			if (orderData.nSid >= 0)
			{
				if (orderData.nOrdNumS > 0)
				{
					bDrawSell = true;
					ptsSell[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(orderData.nOrdNumS);
					ptsSell[1].SetPoint(x + width, nBottom);
				}
				if (orderData.nOrdNumB > 0)
				{
					bDrawBuy = true;
					ptsBuy[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(orderData.nOrdNum);
					ptsBuy[1].SetPoint(x + width, nBottom);
				}
			}
		}
		break;
		case eFFS_DeleteNum:
		{
			auto& deleteData = m_pDeleteStateVec->at(nDataPos);
			if (deleteData.nSid >= 0)
			{
				if (deleteData.nDelNumS > 0)
				{
					bDrawSell = true;
					ptsSell[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(deleteData.nDelNumS);
					ptsSell[1].SetPoint(x + width, nBottom);
				}
				if (deleteData.nDelNumB > 0)
				{
					bDrawBuy = true;
					ptsBuy[0].SetPoint(x + width, nBottom);
					nBottom = GetFundFlowVolYPos(deleteData.nDelNum);
					ptsBuy[1].SetPoint(x + width, nBottom);
				}
			}
		}
		break;
		default:
			break;
		}
		if (bDrawSell)
		{
			pRT->SelectObject(penBlue);
			pRT->DrawLines(ptsSell, 2);
		}

		if (bDrawBuy)
		{
			pRT->SelectObject(penRed);
			pRT->DrawLines(ptsBuy, 2);

		}
	}

}

void SOUI::SFenShiPic::DrawTragetArrow(IRenderTarget* pRT, CRect& rc)
{
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(rc.left, rc.top);
		pts[1].SetPoint(rc.left, rc.bottom);
		pts[2].SetPoint(rc.right, rc.bottom);
		pts[3].SetPoint(rc.right, rc.top);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 4);
		pRT->SelectObject(oldPen);
	}

	//MACD区横向虚线
	HDC pdc = pRT->GetDC();
	int nWidth = (rc.Height() - INFOHEIGHT) / 4;
	for (size_t i = 0; i < 4; i++)
	{
		int nY = rc.top + INFOHEIGHT + nWidth * i;
		CPoint pts[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			if (i == 2)
				pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
			else
				pRT->CreatePen(PS_SOLID, RGBA(150, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pts[0].SetPoint(m_rcMain.left, nY);
			pts[1].SetPoint(m_rcMain.right, nY);
			pRT->DrawLines(pts, 2);
			pRT->SelectObject(oldPen);
		}
	}
	pRT->ReleaseDC(pdc);

}

void SOUI::SFenShiPic::DrawMainUpperInfo(IRenderTarget* pRT, FENSHI_GROUP& data)
{
	SStringW strMarket;
	HDC hdc = pRT->GetDC();
	CSize size;
	size.cx = 0; size.cy = 0;
	CAutoRefPtr<IBrush> bBrushDarkBlue;
	pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &bBrushDarkBlue);
	pRT->SelectObject(bBrushDarkBlue);

	pRT->FillRectangle(CRect(m_rcImage.left + 1, m_rcImage.top + 4, m_rcImage.right - 1, m_rcImage.top + INFOHEIGHT - 1));

	strMarket.Format(L"%d-%02d-%02d %02d:%02d", data.date / 10000, data.date % 10000 / 100, data.date % 100, data.time / 100,
		data.time % 100 / 1);
	int left = 5;
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 0, 255));


	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"价格");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.close);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"均价");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.avg);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, GetColor(data.avg));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"涨跌");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.close - m_pDataInfo->fPreClose);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"涨跌幅");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"%.02f%%", 100 * (data.close - m_pDataInfo->fPreClose) / m_pDataInfo->fPreClose);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"交易量");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"%d", m_bIsIndex ? data.vol : data.vol / 100);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT), strMarket, RGBA(255, 255, 0, 255));

	pRT->ReleaseDC(hdc);
}

void SOUI::SFenShiPic::DrawMainPriceTragetInfo(IRenderTarget* pRT, FENSHI_GROUP& data, int id)
{
	SStringW strMarket;
	HDC hdc = pRT->GetDC();
	pRT->FillRectangle(CRect(m_rcImage.left + 1, m_rcImage.top + INFOHEIGHT + 4, m_rcImage.right - 1, m_rcImage.top + INFOHEIGHT * 2 - 1));
	CSize size;
	size.cx = 0; size.cy = 0;
	int left = 5;
	if (m_bShowEMA)
	{
		strMarket.Format(L"EMA%d:%.02f", m_nEMAPara[0], data.EMA1);
		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_EMA1]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"EMA%d:%.02f", m_nEMAPara[1], data.EMA2);
		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_EMA2]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;

	}


	id = min(id, 238);



	if (m_bShowOrderPrice)
	{
		auto& orderData = m_pOrderStateVec->at(id);
		if (orderData.nSid >= 0)
			strMarket.Format(L"委托:%.02f", orderData.fOrdPrice * 0.01);
		else
			strMarket.Format(L"委托:-");

		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_Order]);
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
	}


	if (m_bShowOrderPriceDetail)
	{
		auto& orderData = m_pOrderStateVec->at(id);
		if (orderData.nSid >= 0)
			strMarket.Format(L"委买:%.02f", orderData.fOrdPriceB * 0.01);
		else
			strMarket.Format(L"委买:-");

		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_OrderB]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		if (orderData.nSid >= 0)
			strMarket.Format(L"委卖:%.02f", orderData.fOrdPriceS * 0.01);
		else
			strMarket.Format(L"委卖:-");

		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_OrderS]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;


	}



	if (m_bShowDeletePriceDetail)
	{
		auto& DeleteData = m_pDeleteStateVec->at(id);
		if (DeleteData.nSid >= 0)
			strMarket.Format(L"撤买:%.02f", DeleteData.fDelPriceB * 0.01);
		else
			strMarket.Format(L"撤买:-");

		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_DeleteB]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		if (DeleteData.nSid >= 0)
			strMarket.Format(L"撤卖:%.02f", DeleteData.fDelPriceS * 0.01);
		else
			strMarket.Format(L"撤卖:-");

		DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + INFOHEIGHT + TEXTTOPMARGIN, m_rcMain.right, m_rcImage.top + INFOHEIGHT * MAININFOCOUNT),
			strMarket, colVec[ePLT_DeleteS]);

		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;

	}

	pRT->ReleaseDC(hdc);
}

void SOUI::SFenShiPic::DrawMacdUpperInfo(IRenderTarget* pRT, FENSHI_GROUP& data)
{
	if (m_bShowMacd)
	{
		SStringW strMarket;
		HDC hdc = pRT->GetDC();
		CSize size;
		pRT->FillRectangle(CRect(m_rcMACD.left + 1, m_rcMACD.top + 4, m_rcMACD.right, m_rcMACD.top + INFOHEIGHT - 1));
		int left = 5;
		strMarket.Format(L"MACD(%d,%d,%d)", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2]);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + TEXTTOPMARGIN, m_rcMACD.left + 160, m_rcMACD.top + INFOHEIGHT),
			strMarket, RGBA(255, 255, 255, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"DIF:%.02f", data.macd.dDIF);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + TEXTTOPMARGIN, (m_rcMACD.left + 240 > m_rcMACD.right ? m_rcMACD.right : m_rcMACD.left + 240), m_rcMACD.top + INFOHEIGHT),
			strMarket, RGBA(255, 255, 255, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"DEA:%.02f", data.macd.dDEA);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + TEXTTOPMARGIN, (m_rcMACD.left + 240 > m_rcMACD.right ? m_rcMACD.right : m_rcMACD.left + 240), m_rcMACD.top + INFOHEIGHT),
			strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"MACD:%.3f", data.macd.dMACD);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + TEXTTOPMARGIN, m_rcMACD.right, m_rcMACD.top + INFOHEIGHT),
			strMarket, RGBA(255, 0, 255, 255));
		pRT->ReleaseDC(hdc);

	}

}

void SOUI::SFenShiPic::DrawFundFlowVolUpperInfo(IRenderTarget* pRT, int id)
{
	if (m_nFundFlowShowType == eFFS_Null)
		return;
	vector<SStringW> strMarket;
	HDC hdc = pRT->GetDC();
	pRT->FillRectangle(CRect(m_rcFundFlowVol.left + 1, m_rcFundFlowVol.top + 4, m_rcFundFlowVol.right, m_rcFundFlowVol.top + INFOHEIGHT - 1));
	CSize size;
	size.cx = 0; size.cy = 0;
	int left = 5;
	id = min(id, 238);
	SStringW str;
	switch (m_nFundFlowShowType)
	{
	case eFFS_OrderVol:
	{
		auto orderData = m_pOrderStateVec->at(id);
		if (orderData.nSid >= 0)
		{
			str.Format(L"委托量:%.0f", orderData.nOrdVolume * 0.01);
			strMarket.emplace_back(str);
			str.Format(L"委买量:%.0f", orderData.nOrdVolumeB * 0.01);
			strMarket.emplace_back(str);
			str.Format(L"委卖量:%.0f", orderData.nOrdVolumeS * 0.01);
			strMarket.emplace_back(str);
		}
		else
		{
			strMarket.emplace_back(L"委托量:-");
			strMarket.emplace_back(L"委买量:-");
			strMarket.emplace_back(L"委卖量:-");
		}
	}
	break;
	case eFFS_DeleteVol:
	{
		auto deleteData = m_pDeleteStateVec->at(id);
		if (deleteData.nSid >= 0)
		{
			str.Format(L"撤单量:%.0f", deleteData.nDelVolume * 0.01);
			strMarket.emplace_back(str);
			str.Format(L"撤买量:%.0f", deleteData.nDelVolumeB * 0.01);
			strMarket.emplace_back(str);
			str.Format(L"撤卖量:%.0f", deleteData.nDelVolumeS * 0.01);
			strMarket.emplace_back(str);
		}
		else
		{
			strMarket.emplace_back(L"撤单量:-");
			strMarket.emplace_back(L"撤买量:-");
			strMarket.emplace_back(L"撤卖量:-");
		}
	}
	break;
	case eFFS_OrderNum:
	{
		auto orderData = m_pOrderStateVec->at(id);
		if (orderData.nSid >= 0)
		{
			str.Format(L"委托单:%d", orderData.nOrdNum);
			strMarket.emplace_back(str);
			str.Format(L"委买单:%d", orderData.nOrdNumB);
			strMarket.emplace_back(str);
			str.Format(L"委卖单:%d", orderData.nOrdNumS);
			strMarket.emplace_back(str);
		}
		else
		{
			strMarket.emplace_back(L"委托单:-");
			strMarket.emplace_back(L"委买单:-");
			strMarket.emplace_back(L"委卖单:-");
		}
	}
	break;
	case eFFS_DeleteNum:
	{
		auto deleteData = m_pDeleteStateVec->at(id);
		if (deleteData.nSid >= 0)
		{
			str.Format(L"撤单单:%d", deleteData.nDelNum);
			strMarket.emplace_back(str);
			str.Format(L"撤买单:%d", deleteData.nDelNumB);
			strMarket.emplace_back(str);
			str.Format(L"撤卖单:%d", deleteData.nDelNumS);
			strMarket.emplace_back(str);
		}
		else
		{
			strMarket.emplace_back(L"撤单单:-");
			strMarket.emplace_back(L"撤买单:-");
			strMarket.emplace_back(L"撤卖单:-");
		}
	}
	break;
	default:
		break;
	}
	COLORREF color[] = { RGBA(255,255,255,255),RGBA(255,0,0,255),RGBA(0,255,255,255) };
	for (int i = 0; i < strMarket.size(); ++i)
	{
		DrawTextonPic(pRT, CRect(m_rcFundFlowVol.left + left, m_rcFundFlowVol.top + TEXTTOPMARGIN, m_rcFundFlowVol.right, m_rcFundFlowVol.top + INFOHEIGHT),
			strMarket[i], color[i]);
		GetTextExtentPoint32(hdc, strMarket[i], strMarket[i].GetLength(), &size);
		left += size.cx;
	}

	pRT->ReleaseDC(hdc);

}


COLORREF SFenShiPic::GetColor(double dPrice)
{
	if (dPrice > m_pDataInfo->fPreClose)
		return RGBA(255, 0, 0, 255);
	else if (dPrice < m_pDataInfo->fPreClose)
		return RGBA(0, 255, 0, 255);
	else
		return RGBA(255, 255, 255, 255);
}



void SFenShiPic::DrawMouse(IRenderTarget* pRT, CPoint po, BOOL bFromOnPaint)
{
	if (po.x == m_nMouseX && po.y == m_nMouseY)
		return;
	if (!IsInRect(po.x, po.y, 0))
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
		DrawMouseLine(pRT, po);
	//显示横坐标所在数值
	CAutoRefPtr<IBrush> bBrushDarkBlue;
	pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &bBrushDarkBlue);
	pRT->SelectObject(bBrushDarkBlue);

	if (m_bDataInited)
	{

		DrawPrice(pRT);
		DrawMovePrice(pRT, m_nMouseY, false);
		DrawPrice(pRT);
		DrawMovePrice(pRT, po.y, true);

		//显示纵坐标数值
		DrawVirtualTimeLine(pRT);

		if (IsInRect(m_nMouseX, m_nMouseY, 0))
			DrawMoveTime(pRT, m_nMouseX, 0, 0, false);
		DrawVirtualTimeLine(pRT);
		if (IsInRect(po.x, po.y, 0))
		{
			int nx = GetXData(po.x);
			if (nx >= 0 && nx < m_dataVec.size() && m_dataVec[nx].date > 0)
				DrawMoveTime(pRT, po.x, m_dataVec[nx].date, m_dataVec[nx].time, true);
		}
		//在左上角添加此时刻具体行情


		if (IsInRect(m_nMouseX, m_nMouseY, 0) && m_bShowMouseLine)
		{
			int nx = GetXData(m_nMouseX);
			FENSHI_GROUP p = { 0 };
			{
				std::lock_guard<std::mutex> lock(m_mxData);
				if (nx >= 0 && nx < m_dataVec.size() && m_dataVec[nx].date > 0)
					p = (m_dataVec[nx]);
				else if (nx == -1 && !m_dataVec.empty())
				{
					p = (m_dataVec.at(m_dataVec.size() - 1));
					nx = m_dataVec.size() - 1;
				}
			}
			if (p.date != 0)
				DrawUpperMarket(pRT, p, nx);



		}
		if (IsInRect(po.x, po.y, 0) && m_bShowMouseLine)
		{
			int nx = GetXData(po.x);
			FENSHI_GROUP p = { 0 };
			{
				std::lock_guard<std::mutex> lock(m_mxData);
				if (nx >= 0 && nx < m_dataVec.size() && m_dataVec[nx].date > 0)
					p = (m_dataVec[nx]);
				else if (nx == -1 && !m_dataVec.empty())
				{
					p = (m_dataVec.at(m_dataVec.size() - 1));
					nx = m_dataVec.size() - 1;
				}
			}
			if (p.date != 0)
				DrawUpperMarket(pRT, p, nx);

		}

	}
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		//if (m_pbShowSubPic[i])
		m_ppSubPic[i]->DrawMouse(pRT, po, bFromOnPaint);
	}

	m_nMouseX = po.x;
	m_nMouseY = po.y;

}



void SFenShiPic::DrawData(IRenderTarget* pRT)
{
	int nDataNum = m_dataVec.size();
	if (nDataNum == 0)
		return;

	CPoint pts[5];
	int x = 0, yavg = 0, yclose = 0, ypreavg = 0, ypreclose = 0;
	int yDIF = 0, yDEA = 0, ypreDIF = 0, ypreDEA = 0;
	int yEMA1 = 0, yEMA2 = 0, yPreEMA1 = 0, yPreEMA2 = 0;
	int yFutStock = 0;
	CAutoRefPtr<IPen> penWhite, penYellow, oldPen, penRed, penBlue, penGreen, penPurple;
	CAutoRefPtr<IBrush> bBrush, bOldBrush;
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);



	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 50, 255), 1, &penYellow);
	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &penWhite);
	pRT->CreatePen(PS_SOLID, RGBA(255, 20, 0, 255), 1, &penRed);
	pRT->CreatePen(PS_SOLID, RGBA(0, 255, 255, 255), 1, &penBlue);
	pRT->CreatePen(PS_SOLID, RGBA(0, 255, 0, 255), 1, &penGreen);
	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 255, 255), 1, &penPurple);
	pRT->CreateSolidColorBrush(RGBA(0, 255, 255, 255), &bBrush);

	pRT->SelectObject(penWhite, (IRenderObj**)&oldPen);
	pRT->SelectObject(bBrush, (IRenderObj**)&bOldBrush);



	if (nDataNum > m_nAllLineNum)
		nDataNum = m_nAllLineNum;
	int nFutStockDataNum = 0;


	CPoint* MainLine = new CPoint[nDataNum];
	CPoint* AvgLine = new CPoint[nDataNum];
	CPoint* EMA1Line = new CPoint[nDataNum];
	CPoint* EMA2Line = new CPoint[nDataNum];
	CPoint* DIFLine = new CPoint[nDataNum];
	CPoint* DEALine = new CPoint[nDataNum];


	std::vector<FENSHI_GROUP> dataVec;
	{
		std::lock_guard<std::mutex> lock(m_mxData);
		dataVec.assign(m_dataVec.begin(),m_dataVec.begin()+nDataNum);
	}

	for (size_t i = 0; i < nDataNum; i++)
	{

		x = GetXPos(i);
		int xpre = GetXPos(i - 1);
		yavg = GetYPos(dataVec[i].avg);
		yclose = GetYPos(dataVec[i].close);
		yDEA = GetMACDYPos(dataVec[i].macd.dDEA);
		yDIF = GetMACDYPos(dataVec[i].macd.dDIF);
		yEMA1 = GetYPos(dataVec[i].EMA1);
		yEMA2 = GetYPos(dataVec[i].EMA2);

		if (i == 0)
		{
			ypreclose = yclose;
			ypreavg = yavg;
			ypreDEA = yDEA;
			ypreDIF = yDIF;
			yPreEMA1 = yEMA1;
			yPreEMA2 = yEMA2;

		}
		//加最后的数值
		if (i == nDataNum - 1)
		{
			SStringW strTemp;
			strTemp.Format(L"%s %s", StrA2StrW(m_strStockName), StrA2StrW(m_strSubIns));
			DrawTextonPic(pRT, CRect(m_rcMain.left, m_rcMain.top - INFOHEIGHT, m_rcMain.right, m_rcMain.top), strTemp, RGBA(255, 255, 0, 255));

			strTemp.Format(L"Price:%.02f", dataVec[i].close);
			DrawTextonPic(pRT, CRect(m_rcMain.right - 160, m_rcMain.top - INFOHEIGHT, m_rcMain.right - 80, m_rcMain.top),
				strTemp, RGBA(255, 255, 255, 255));
			strTemp.Format(L"Avg:%.02f", dataVec[i].avg);
			DrawTextonPic(pRT, CRect(m_rcMain.right - 80, m_rcMain.top - INFOHEIGHT, m_rcMain.right, m_rcMain.top),
				strTemp, RGBA(255, 255, 0, 255));
			CPoint pt;
			GetCursorPos(&pt);
			if (!m_bShowMouseLine)
				DrawUpperMarket(pRT, dataVec[i], i);

		}

		MainLine[i].SetPoint(x + width, yclose);

		AvgLine[i].SetPoint(x + width, yavg);

		//画量
		if (m_bShowVolume)
		{
			pRT->SelectObject(penYellow);

			int vol = m_bIsIndex ?
				dataVec[i].vol : dataVec[i].vol / 100;
			if (dataVec[i].vol != 0)
			{

				pts[0].SetPoint(x + width, m_rcVolume.bottom - 1);
				pts[1].SetPoint(x + width, GetFuTuYPos(vol));
				pRT->DrawLines(pts, 2);
			}
			if (i == nDataNum - 1)
			{
				SStringW strTemp;
				strTemp.Format(L"Volume:%d", vol);

				pRT->TextOut(m_rcVolume.right - 100, m_rcVolume.top - RC_FSTOP + 30, strTemp, -1);
			}

		}

		//画EMA
		if (m_bShowEMA)
		{

			EMA1Line[i].SetPoint(x + width, yEMA1);

			EMA2Line[i].SetPoint(x + width, yEMA2);
		}


		//画MACD图

		if (m_bShowMacd)
		{

			DIFLine[i].SetPoint(x + width, yDIF);


			DEALine[i].SetPoint(x + width, yDEA);

			int nWidthMacd = (m_rcMACD.Height() - INFOHEIGHT) / 4;
			//MACD柱状图
			if (dataVec[i].macd.dMACD != 0)
			{
				pts[0].SetPoint(x + width, m_rcMACD.top + INFOHEIGHT + 2 * nWidthMacd);
				pts[1].SetPoint(x + width, GetMACDYPos(dataVec[i].macd.dMACD));

				if (dataVec[i].macd.dMACD > 0)
					pRT->SelectObject(penRed);
				else
					pRT->SelectObject(penBlue);
				pRT->DrawLines(pts, 2);
			}

		}

	}

	pRT->SelectObject(penWhite);
	pRT->DrawLines(MainLine, nDataNum);

	if (m_bShowAvg)
	{
		pRT->SelectObject(penYellow);
		pRT->DrawLines(AvgLine, nDataNum);

	}

	if (m_bShowEMA)
	{
		pRT->SelectObject(penPurple);
		pRT->DrawLines(EMA1Line, nDataNum);

		pRT->SelectObject(penGreen);
		pRT->DrawLines(EMA2Line, nDataNum);

	}


	if (m_bShowMacd)
	{
		pRT->SelectObject(penWhite);
		pRT->DrawLines(DIFLine, nDataNum);

		pRT->SelectObject(penYellow);
		pRT->DrawLines(DEALine, nDataNum);
	}


	DrawFundFlowPriceLine(pRT, nDataNum);

	DrawFundFlowVol(pRT, nDataNum);

	delete[]MainLine;
	delete[]AvgLine;
	delete[]EMA1Line;
	delete[]EMA2Line;
	delete[]DIFLine;
	delete[]DEALine;

	MainLine = nullptr;
	AvgLine = nullptr;
	EMA1Line = nullptr;
	EMA2Line = nullptr;
	DIFLine = nullptr;
	DEALine = nullptr;

	pRT->SelectObject(oldPen);
	pRT->SelectObject(bOldBrush);

	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
	{
		//if (m_pbShowSubPic[i])
		m_ppSubPic[i]->DrawData(pRT);
	}

}

void SFenShiPic::OnDbClickedFenshi(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	m_bShowMouseLine = !m_bShowMouseLine;
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
		m_nMouseX = m_nKeyX;
		m_nMouseY = m_nKeyY;
	}
	std::lock_guard<std::mutex> lock(m_mxSub);
	for (int i = 0; i < m_nSubPicNum; ++i)
		m_ppSubPic[i]->SetMouseLineState(m_bShowMouseLine);

	Invalidate();
}



void SFenShiPic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
	{
		if (!m_bKeyDown)
		{
			m_bKeyDown = true;
			m_nKeyX = m_nMouseX;
			m_nKeyY = m_nMouseY;
			//if (m_bShowMouseLine)
			//{
			int nx = GetXData(m_nMouseX);
			if (nx >= m_dataVec.size())
				nx = m_dataVec.size() - 1;
			m_nNowPosition = max(nx, 0);
			m_bShowMouseLine = true;
			Invalidate();
			break;
		}
		else
		{
			if (nChar == VK_LEFT)
				m_nNowPosition--;
			else if (nChar == VK_RIGHT)
				m_nNowPosition++;
			if (m_nNowPosition < 0)
				m_nNowPosition = m_dataVec.size() - 1;
			else if (m_nNowPosition >= m_dataVec.size())
				m_nNowPosition = 0;
		}
		CRect rc = GetClientRect();
		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
		SPainter pa;
		SWindow::BeforePaint(pRT, pa);
		DrawKeyDownMouseLine(pRT, true);
		AfterPaint(pRT, pa);
		ReleaseRenderTarget(pRT);

	}
	break;
	default:
		break;
	}
	//m_ppSubPic->OnKeyUp(nChar, nRepCnt, nFlags);
	//Invalidate();
}

void SFenShiPic::DataInit()
{
	std::lock_guard<std::mutex> lock(m_mxData);

	if (m_pDataInfo == nullptr)
	{
		m_pDataInfo = new FENSHI_INFO;
		ZeroMemory(m_pDataInfo, sizeof(FENSHI_INFO));
	}
	else
	{
		delete m_pDataInfo;
		m_pDataInfo = new FENSHI_INFO;
		ZeroMemory(m_pDataInfo, sizeof(FENSHI_INFO));
	}
	m_dataVec.clear();
	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_nTradingDay = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	SingleInit();

}

void SFenShiPic::SingleInit()
{
	m_pDataInfo->nCount = 0;
	m_pDataInfo->nLastVolume = 0;
	m_pDataInfo->nTime = 925;
	m_pDataInfo->nMin = 25;
	int nDigit = 0;

	if (m_bIsIndex)
		m_bShowAvg = false;

	InitVirTimeLineMap();
	SetShowTime();
	m_pDataInfo->nAllLineNum = 241;
	m_nAllLineNum = m_pDataInfo->nAllLineNum;
	m_dataVec.reserve(m_pDataInfo->nAllLineNum + 20);

	m_pDataInfo->fMaxMACD = -100;
	m_pDataInfo->fMinMACD = 100;

	m_bDataInited = true;

	if (m_bIsIndex)
	{
		if (m_pIdxMarketVec->empty())
			return;
		m_pDataInfo->fPreClose = m_pIdxMarketVec->back().PreCloPrice;
		m_pDataInfo->fMax = m_pIdxMarketVec->back().HighPrice;
		m_pDataInfo->fMin = m_pIdxMarketVec->back().LowPrice;

	}
	else
	{
		if (m_pStkMarketVec->empty())
			return;

		m_pDataInfo->fPreClose = m_pStkMarketVec->back().PreCloPrice;
		m_pDataInfo->fMax = m_pStkMarketVec->back().HighPrice;
		m_pDataInfo->fMin = m_pStkMarketVec->back().LowPrice;
	}
	m_pDataInfo->nMin = 25;
}


void SFenShiPic::SetShowTime()
{
	m_timeSet.clear();
	m_timeSet.insert(925);
	for (int i = 930; i < 1130; ++i)
	{
		if (i % 100 == 60)
			i = (i / 100 + 1) * 100;
		m_timeSet.insert(i);
	}
	for (int i = 1300; i <= 1459; ++i)
	{
		if (i % 100 == 60)
			i = (i / 100 + 1) * 100;
		m_timeSet.insert(i);
	}
}

void SFenShiPic::DrawKeyDownMouseLine(IRenderTarget* pRT, UINT nChar)
{
	if (m_dataVec.empty())
		return;
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);


	//画鼠标线

	CPoint po;
	po.x = GetXPos(m_nNowPosition) + width;
	po.y = GetYPos(m_dataVec[m_nNowPosition].close);
	DrawMouseLine(pRT, po);

	//显示横坐标轴数值
	FENSHI_GROUP p = m_dataVec[m_nNowPosition];
	DrawUpperMarket(pRT, p, m_nNowPosition);

	{
		std::lock_guard<std::mutex> lock(m_mxSub);
		for (int i = 0; i < m_nSubPicNum; ++i)
		{
			m_ppSubPic[i]->SetNowKeyDownLinePos(m_nNowPosition);
			m_ppSubPic[i]->DrawKeyDownMouseLine(pRT, TRUE);
		}
	}

	//DrawVirtualTimeLine(pRT);
	DrawPrice(pRT);
	DrawMovePrice(pRT, m_nMouseY, false);
	DrawPrice(pRT);
	DrawMovePrice(pRT, po.y, true);

	DrawVirtualTimeLine(pRT);
	DrawMoveTime(pRT, m_nMouseX, 0, 0, false);
	DrawVirtualTimeLine(pRT);
	DrawMoveTime(pRT, po.x, p.date, p.time, true);

	//左上角显示行情
	m_nMouseX = po.x;
	m_nMouseY = po.y;


	//Invalidate();
}

void SFenShiPic::DrawPrice(IRenderTarget* pRT)
{
	if (!m_bDataInited)
		return;
	//k线区加坐标
	int nYoNum = 9;		//y轴标示数量 3 代表画两根线

	int nWidth = m_nHeight / 8;
	for (size_t i = 0; i < nYoNum; i++)
	{
		int nY = m_rcMain.top + INFOHEIGHT * MAININFOCOUNT + nWidth * i;

		//k线区y轴加轴标
		SStringW s1 = GetYPrice(nY, 0);
		if (i == 4)		//中间刻度线
			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 255, 255, 255), DT_CENTER);
		else if (i < 4)
			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
		else
		{
			if (nY != m_rcMain.bottom)
				DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(0, 255, 0, 255), DT_CENTER);
		}
	}

	//副图区加坐标
	if (m_bShowVolume)
	{
		int nY = m_rcVolume.bottom - ((m_rcVolume.bottom - m_rcVolume.top) / 2);
		SStringW s1;
		s1 = GetFuTuYPrice(nY);

		DrawTextonPic(pRT, CRect(m_rcVolume.left - RC_FSLEFT + 2, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 255, 255, 255), DT_CENTER);
	}
	//MACD区加坐标
	if (m_bShowMacd)
	{
		for (size_t i = 0; i < 4; i++)
		{
			int nY = m_rcMACD.top + INFOHEIGHT + (m_rcMACD.Height() - INFOHEIGHT) / 4 * i;

			//k线区y轴加轴标
			SStringW s1 = GetMACDYPrice(nY);

			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 5, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 255, 255, 255), DT_CENTER);
		}
	}

	if (m_nFundFlowShowType != eFFS_Null)
	{
		for (size_t i = 0; i < 4; i++)
		{
			int nY = m_rcFundFlowVol.top + INFOHEIGHT + (m_rcFundFlowVol.Height() - INFOHEIGHT) / 4 * i;

			//k线区y轴加轴标
			SStringW s1 = GetMACDYPrice(nY);

			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 5, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 255, 255, 255), DT_CENTER);
		}
	}

}

void SFenShiPic::DrawMovePrice(IRenderTarget* pRT, int y, bool bNew)
{
	CRect rcLeft(m_rcMain.left - RC_FSLEFT + 1, y - 15, m_rcMain.left - 1, y);
	CRect rcRight(m_rcMain.right + 1, y - 15, m_rcMain.right + RC_FSRIGHT - 1, y);
	bool bUseLeft = y >= m_rcMain.top + INFOHEIGHT * MAININFOCOUNT && y < m_rcMain.bottom;
	if (bNew)
	{
		SStringW sl, sr;
		if (y >= m_rcMain.top + INFOHEIGHT * MAININFOCOUNT && y < m_rcMain.bottom)
		{
			sl = GetYPrice(y, FALSE);
			sr = GetYPrice(y, TRUE);
		}
		else if (y >= m_rcVolume.top + INFOHEIGHT && y <= m_rcVolume.bottom)
			sr = GetFuTuYPrice(y);
		else if (y >= m_rcMACD.top + INFOHEIGHT && y <= m_rcMACD.bottom)
			sr = GetMACDYPrice(y);
		else if (y >= m_rcFundFlowVol.top + INFOHEIGHT && y <= m_rcFundFlowVol.bottom)
			sr = GetFundFlowVolYValue(y);

		if (sr != L"")
		{
			CAutoRefPtr<IBrush>bBrushDarkBlue, oldBrush;
			pRT->CreateSolidColorBrush(RGBA(50, 100, 150, 255), &bBrushDarkBlue);
			pRT->SelectObject(bBrushDarkBlue, (IRenderObj**)&oldBrush);
			pRT->FillRectangle(rcRight);
			pRT->SelectObject(oldBrush);
			rcRight.left += 1;
			rcRight.right -= 1;
			DrawTextonPic(pRT, rcRight, sr,
				RGBA(255, 255, 255, 255), DT_SINGLELINE | DT_CENTER, MERGECOPY);
		}
		if (sl != L"")
		{
			CAutoRefPtr<IBrush>bBrushDarkBlue, oldBrush;
			pRT->CreateSolidColorBrush(RGBA(50, 100, 150, 255), &bBrushDarkBlue);
			pRT->SelectObject(bBrushDarkBlue, (IRenderObj**)&oldBrush);
			pRT->FillRectangle(rcLeft);
			pRT->SelectObject(oldBrush);
			rcLeft.left += 1;
			rcLeft.right -= 1;
			DrawTextonPic(pRT, rcLeft, sl,
				RGBA(255, 255, 255, 255), DT_SINGLELINE | DT_CENTER, MERGECOPY);

		}
	}
	else
	{
		if (bUseLeft)
			pRT->FillRectangle(rcLeft);
		pRT->FillRectangle(rcRight);

	}
}

void SFenShiPic::DrawMoveTime(IRenderTarget* pRT, int x, int date, int time, bool bNew)
{

	CRect rc(x + 2, m_rcImage.bottom + 2, x + 50, m_rcImage.bottom + INFOHEIGHT);
	if (bNew)
	{
		if (date == 0)
			return;
		SStringW str;
		str.Format(L"%02d:%02d",
			time / 100,
			time % 100);
		CRect drawRc = GetTextDrawRect(pRT, str, rc);
		drawRc.SetRect(x, m_rcImage.bottom + 2, x + drawRc.right + 4, m_rcImage.bottom + INFOHEIGHT);
		CAutoRefPtr<IBrush>bBrushDarkBlue, oldBrush;
		pRT->CreateSolidColorBrush(RGBA(50, 100, 150, 255), &bBrushDarkBlue);
		pRT->SelectObject(bBrushDarkBlue, (IRenderObj**)&oldBrush);
		pRT->FillRectangle(drawRc);
		pRT->SelectObject(oldBrush);
		drawRc.left += 2;
		DrawTextonPic(pRT, drawRc,
			str, RGBA(255, 255, 255, 255), DT_SINGLELINE | DT_VCENTER, MERGECOPY);

	}
	else
	{
		rc.left -= 2;
		rc.right = min(rc.right, m_rcImage.right + RC_FSRIGHT - 1);
		pRT->FillRectangle(rc);
	}
}


