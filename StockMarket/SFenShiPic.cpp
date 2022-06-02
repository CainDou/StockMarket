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


extern HWND g_MainWnd;


#define sDimical L"%.02f"
//RpsGroup SFenShiPic::m_rgClickGroup = Group_Count;

SFenShiPic::SFenShiPic()
{
	m_nTickPre = 0;
	m_bFocusable = 1; //可以获取焦点
	m_nMouseX = m_nMouseY = -1;
	m_nIndex = -1;
	m_nOldIndex = -1;
	m_pData = nullptr;

	m_nAllLineNum = 225;

	m_nPaintTick = GetTickCount();
	m_bDataInited = false;


	m_nNowPosition = 0;
	m_bShowMouseLine = false;
	m_bKeyDown = false;
	m_bIsFirstKey = true;
	m_bShowDeal = true;
	m_bShowSubPic = true;
	m_bShowAvg = true;
	m_bPaintInit = FALSE;


	m_pDealList = new CDealList;
	m_pPriceList = new CPriceList;
	m_pSubPic = new SSubTargetPic();
	m_bIsStockIndex = false;
	m_preMovePt.SetPoint(-1, -1);

	m_nMACDPara[0] = 12;
	m_nMACDPara[1] = 26;
	m_nMACDPara[2] = 9;

	m_nEMAPara[0] = 12;
	m_nEMAPara[1] = 26;
	//m_pRTBuffer = nullptr;
}



SFenShiPic::~SFenShiPic()
{
	delete m_pData;
	delete m_pDealList;
	delete m_pPriceList;
}

void SFenShiPic::SetShowData(SStringA subIns, SStringA StockName, vector<CommonIndexMarket>* pIdxMarketVec)
{
	KillTimer(1);
	m_bDataInited = false;
	m_strSubIns = subIns;
	m_pIdxMarketVec = pIdxMarketVec;
	m_pStkMarketVec = nullptr;
	m_bIsStockIndex = true;
	m_strStockName = StockName;
	m_pPriceList->SetShowData(subIns, m_strStockName, pIdxMarketVec);
	m_pDealList->SetShowData(subIns, pIdxMarketVec);
	//DataInit();
	//if(m_pData->fPreClose == 0)
	//	m_pData->fPreClose = fPreClose;
}

void SFenShiPic::SetShowData(SStringA subIns, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec)
{
	KillTimer(1);
	m_bDataInited = false;
	m_strSubIns = subIns;
	m_pStkMarketVec = pStkMarketVec;
	m_pIdxMarketVec = nullptr;
	m_bIsStockIndex = false;
	m_strStockName = StockName;
	m_pPriceList->SetShowData(subIns, m_strStockName, pStkMarketVec);
	m_pDealList->SetShowData(subIns, pStkMarketVec);
	//DataInit();
	//if (m_pData->fPreClose == 0)
	//	m_pData->fPreClose = fPreClose;

}

void SFenShiPic::SetSubPicShowData(int nIndex, bool nGroup)
{
	m_pSubPic->SetShowData(nIndex, nGroup);
}

void SFenShiPic::SetSubPicShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec,
	vector<SStringA> dataNameVec, SStringA StockID, SStringA StockName)
{
	m_pSubPic->SetShowData(nDataCount, data, bRightVec, dataNameVec, StockID, StockName);
}




void SFenShiPic::InitShowPara(InitPara_t para)
{
	m_bShowDeal = para.bShowTSCDeal;
	m_bShowMacd = para.bShowTSCMACD;
	m_bShowVolume = para.bShowTSCVolume;
	m_bShowAvg = para.bShowAverage;
	m_bShowEMA = para.bShowEMA;
	m_nEMAPara[0] = para.nEMAPara[0];
	m_nEMAPara[1] = para.nEMAPara[1];
	m_nMACDPara[0] = para.nMACDPara[0];
	m_nMACDPara[1] = para.nMACDPara[1];
	m_nMACDPara[2] = para.nMACDPara[2];

}

void SOUI::SFenShiPic::OutPutShowPara(InitPara_t & para)
{
	para.bShowTSCDeal = m_bShowDeal;
	para.bShowTSCMACD = m_bShowMacd;
	para.bShowTSCVolume = m_bShowVolume;
	para.bShowAverage = m_bShowAvg;
	para.bShowEMA = m_bShowEMA;
	para.nEMAPara[0] = m_nEMAPara[0];
	para.nEMAPara[1] = m_nEMAPara[1];
	para.nMACDPara[0] = m_nMACDPara[0];
	para.nMACDPara[1] = m_nMACDPara[1];
	para.nMACDPara[2] = m_nMACDPara[2];

}





void SFenShiPic::OnPaint(IRenderTarget * pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	SWindow::GetClientRect(&m_rcAll);

	//m_bPaintInit = true;
	if (!m_bPaintInit)
	{
		m_bPaintInit = true;
		m_pSubPic->InitColorAndPen(pRT);
	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);

	m_rcAll.DeflateRect(RC_FSLEFT + 5, RC_FSTOP, RC_FSRIGHT + 10, RC_FSBOTTOM);

	SetWindowRect();

	m_nMiddle = (m_rcMain.top + m_rcMain.bottom) / 2;
	m_nHeight = m_rcMain.bottom - m_rcMain.top - 20;

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
			pts[0].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_pPriceList->m_rect.bottom - 5);
			pts[1].SetPoint(m_rcAll.right + RC_FSLEFT + 1, m_pPriceList->m_rect.bottom - 5);
			pRT->DrawLines(pts, 2);
		}
		pRT->SelectObject(oldPen);
	}


	DrawArrow(pRT);


	if (m_bDataInited)
	{
		DrawVirtualTimeLine(pRT);
		DrawData(pRT);
	}

	//if (m_bShowSubPic)
	//	m_pSubPic->OnPaint(pRT);

	if (m_bShowDeal)
	{
		m_pPriceList->Paint(pRT);
		m_pDealList->Paint(pRT);
	}


	CPoint po(m_nMouseX, m_nMouseY);
	m_nMouseX = m_nMouseY = -1;
	m_pSubPic->SetMousePosDefault();
	LONGLONG llTmp3 = GetTickCount64();
	if (m_bKeyDown)
		DrawKeyDownMouseLine(pRT, 0);
	else
		DrawMouse(pRT, po, TRUE);
	//	LOG_W(L"鼠标绘制时间:%I64d,总时间:%I64d\0", llTmp3 - llTmp2, llTmp3 - llTmp);
	AfterPaint(pRT, pa);
}

void SFenShiPic::DrawArrow(IRenderTarget * pRT)
{
	//	if (m_pData->d.empty())
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
	//	SetBkColor(pdc, RGBA(255, 0, 0, 0xff));
	int width = m_nHeight / 8;
	for (size_t i = 0; i < nYoNum; i++)
	{
		int nY = m_rcMain.top + 20 + width * i;
		CPoint pts[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pts[0].SetPoint(m_rcMain.left, nY);
			pts[1].SetPoint(m_rcMain.right, nY);

			if (i == 4)
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




	/*
	//k线区横轴位置所在y坐标
	int nlo = m_rcUpper.bottom / nYoNum * (nYoNum - 1) - 5 - m_rcUpper.bottom;
	if (nlo > 15 || nlo < -15)
	{
	SStringW strf1 = GetYPrice(m_rcUpper.bottom);
	pRT->TextOut(m_rcUpper.left -RC_FSLEFT + 8 , m_rcUpper.bottom - 5, strf1, -1);
	}*/
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
	//	SStringW s1 = GetFuTuYPrice(nY);
	//	pRT->TextOut(m_rcLower.left - RC_FSLEFT + 8, nY - 6, s1, -1);

	//	pRT->ReleaseDC(pdc);

	//画	MACD区
	if (m_bShowMacd)
	{
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pts[0].SetPoint(m_rcMACD.left, m_rcMACD.top);
			pts[1].SetPoint(m_rcMACD.left, m_rcMACD.bottom);
			pts[2].SetPoint(m_rcMACD.right, m_rcMACD.bottom);
			pts[3].SetPoint(m_rcMACD.right, m_rcMACD.top);
			pts[4] = pts[0];
			pRT->DrawLines(pts, 4);
			pRT->SelectObject(oldPen);
		}

		//MACD区横向虚线
		pdc = pRT->GetDC();
		int nWidthMacd = (m_rcMACD.Height() - 20) / 4;
		for (size_t i = 0; i < 4; i++)
		{
			int nY = m_rcMACD.top + 20 + nWidthMacd*i;
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
	}

	if (m_bShowSubPic)
		m_pSubPic->DrawArrow(pRT);
}

void SFenShiPic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{

	int nLen = m_rcMain.right - m_rcMain.left;	//判断是否超出范围
													//判断最大最小值
													//	OutputDebugString("判断最大值\0");

	double fMax = -1000000000;
	double fMin = 1000000000;


	fMax = m_pData->fMax;
	fMin = m_pData->fMin;


	for (unsigned i = 0; i < m_pData->d.size(); i++)
	{
		if (m_pData->d[i].close > fMax)
			fMax = m_pData->d[i].close;
		if (m_pData->d[i].close < fMin)
			fMin = m_pData->d[i].close;
	}

	if (fMax > 100000000)
		fMax = m_pData->fPreClose + 1;
	if (fMin > 100000000)
		fMin = m_pData->fPreClose - 1;

	if (fMax < m_pData->fPreClose)
		fMax = m_pData->fPreClose + (m_pData->fPreClose - fMin);
	else if (fMin > m_pData->fPreClose)
		fMin = m_pData->fPreClose + (m_pData->fPreClose - fMax);
	else
	{
		if ((fMax - m_pData->fPreClose) > (m_pData->fPreClose - fMin))
			fMin = m_pData->fPreClose + (m_pData->fPreClose - fMax);
		else
			fMax = m_pData->fPreClose + (m_pData->fPreClose - fMin);
	}

	m_pData->fMax = fMax;
	m_pData->fMin = fMin;
	if (m_pData->fMax == fMin)
		m_pData->fMax = m_pData->fMax * 1.1;
	if (m_pData->fMax == 0)
		m_pData->fMax = 1;

	m_pData->dDelta = m_pData->fMax - m_pData->fPreClose;


}

void SFenShiPic::GetFuTuMaxDiff()		//判断副图坐标最大最小值和k线条数
{

	//判断最大最小值
	double fMax = -100000000;
	double fMin = 100000000;



	int nDataNum = m_pData->d.size();
	if (nDataNum > m_nAllLineNum)
		nDataNum = m_nAllLineNum;
	for (int j = 0; j < nDataNum; j++)
	{
		if (m_pData->d[j].vol < fMin)
			fMin = m_pData->d[j].vol;
		if (m_pData->d[j].vol > fMax)
			fMax = m_pData->d[j].vol;
	}


	fMin = 0;

	m_pData->fMaxf = fMax;
	m_pData->fMinf = fMin;
	if (m_pData->fMaxf == fMin)
		m_pData->fMaxf = m_pData->fMaxf * 1.1;
	if (m_pData->fMaxf == 0)
		m_pData->fMaxf = 1;

	if (m_pData->d.empty())
		m_pData->fMaxf = 1;

	if (!m_bIsStockIndex)
		m_pData->fMaxf /= 100;



}

BOOL SFenShiPic::IsInRect(int x, int y, int nMode)	//是否在坐标中,0为全部,1为上方,2为下方
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
	case 3:
		prc = &m_rcMACD;
	default:
		return FALSE;
	}
	if (x >= prc->left && x <= prc->right &&
		y >= prc->top  && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

int SFenShiPic::GetFuTuYPos(double fDiff)	//获得附图y位置
{
	double fPos = m_pData->fMaxf - fDiff;
	fPos = fPos / (m_pData->fMaxf - m_pData->fMinf)*
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
		(m_rcVolume.Height() - RC_FSMIN)*m_pData->fMaxf;
	strRet.Format(L"%.0f", fDiff);
	return strRet;
}

int SFenShiPic::GetMACDYPos(double fDiff)
{
	double fPos = m_rcMACD.top + (1 - (fDiff / m_pData->fMaxMACD)) / 2 * (m_rcMACD.Height() - 30) + 25;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SFenShiPic::GetMACDYPrice(int nY)
{
	int nWidth = (m_rcMACD.Height() - 20) / 4;
	SStringW strRet; strRet.Empty();
	if (nY > m_rcMACD.bottom || nY < m_rcMACD.top)
		return strRet;
	double fDiff = ((double)(nWidth * 2 + 20 + m_rcMACD.top) - nY) / (m_rcMACD.Height() - 30) * 2 * m_pData->fMaxMACD;
	strRet.Format(L"%.2f", fDiff);
	return strRet;
}

int SFenShiPic::GetYPos(double fDiff)
{
	int nWidth = m_nHeight / 8;
	double fPos = m_rcMain.top + (1 - ((fDiff - m_pData->fPreClose) / m_pData->dDelta)) / 2 * (nWidth * 8) + 20;
	int nPos = (int)fPos;
	return nPos;
}

SStringW SFenShiPic::GetYPrice(int nY, BOOL bIsPercent)
{
	SStringW strRet; strRet.Empty();
	int nWidth = m_nHeight / 8;
	int middle = nWidth * 4 + m_rcMain.top + 20;
	int nDiff = nY - middle;
	double fDiff = nDiff;
	double fPrice = m_pData->fPreClose - fDiff / (nWidth * 8) * 2 * m_pData->dDelta;
	if (bIsPercent)
		strRet.Format(L"%.2f%%", (fPrice - m_pData->fPreClose) / m_pData->fPreClose * 100);
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
		m_pSubPic->SetMouseMove();
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

//LRESULT SFenShiPic::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp)
//{
//	static int times = 0;
//	++times;
//	SStringA str;
//	str.Format("group%d OnMsg触发了%d\n",m_rgGroup, times);
//	OutputDebugStringA(str);
//
//	USHORT nGroup = LOWORD(lp);
//	if ((RpsGroup)nGroup != m_rgGroup)
//	{
//		SetMsgHandled(FALSE);
//		return -1;
//	}
//	FSMSG msg = (FSMSG)HIWORD(lp);
//	switch (msg)
//	{
//	case FSMSG_PROCDATA:
//		DataProc();
//		break;
//	case FSMSG_UPDATE:
//		break;
//	case FSMSG_EMA:
//		ReProcEMA();
//		Invalidate();
//		break;
//	case FSMSG_MACD:
//		ReProcMacd();
//		Invalidate();
//		break;
//	default:
//		break;
//	}
//	return 0;
//}

void SFenShiPic::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	static int times = 0;
	++times;
	SStringA str;
	str.Format("触发了%d\n", times);
	OutputDebugStringA(str);
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

void SFenShiPic::DrawVirtualTimeLine(IRenderTarget * pRT)
{
	if (m_virTimeLineMap.empty())
		return;
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);

	CAutoRefPtr<IPen> pen, oldPen;
	pRT->CreatePen(PS_DOT, RGBA(150, 0, 0, 0xFF), 2, &pen);
	pRT->SelectObject(pen, (IRenderObj**)&oldPen);

	for (auto &it : m_virTimeLineMap)
	{
		int x = GetXPos(it.first) + width;

		if (it.first != 0 && it.second != L"15:00" && it.second != L"15:15")
		{
			CPoint pt[2];
			pt[0].SetPoint(x, m_rcImage.top + 20);
			pt[1].SetPoint(x, m_rcImage.bottom);
			pRT->DrawLines(pt, 2);
		}
		if (it.second == L"15:00" || it.second == L"15:15")
			DrawTextonPic(pRT, CRect(x - 35, m_rcImage.bottom + 15, x, m_rcImage.bottom + 35), it.second, RGBA(255, 255, 255, 255), DT_SINGLELINE);
		else
			DrawTextonPic(pRT, CRect(x, m_rcImage.bottom + 15, x + 40, m_rcImage.bottom + 35), it.second, RGBA(255, 255, 255, 255), DT_SINGLELINE);

	}
	pRT->SelectObject(oldPen);
}

void SFenShiPic::DrawUpperMarket(IRenderTarget * pRT, FENSHI_GROUP & data)
{
	SStringW strMarket;
	HDC hdc = pRT->GetDC();
	CSize size;
	size.cx = 0; size.cy = 0;

	strMarket.Format(L"%d-%02d-%02d %02d:%02d", data.date / 10000, data.date % 10000 / 100, data.date % 100, data.time / 100,
		data.time % 100 / 1);
	int left = 5;
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 0, 255));


	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"价格");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.close);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"均价");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.avg);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, GetColor(data.avg));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"涨跌");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(sDimical, data.close - m_pData->fPreClose);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"涨跌幅");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"%.02f%%", 100 * (data.close - m_pData->fPreClose) / m_pData->fPreClose);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, GetColor(data.close));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"交易量");
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"%d",m_bIsStockIndex?data.vol: data.vol / 100);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 255, 0, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"EMA%d:%.02f", m_nEMAPara[0], data.EMA1);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(255, 0, 255, 255));

	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;
	strMarket.Format(L"EMA%d:%.02f", m_nEMAPara[1], data.EMA2);
	DrawTextonPic(pRT, CRect(m_rcMain.left + left, m_rcMain.top + 5, m_rcMain.right, m_rcImage.top + 20), strMarket, RGBA(0, 255, 0, 255));
	if (m_bShowMacd)
	{
		left = 5;
		strMarket.Format(L"MACD(%d,%d,%d)", m_nMACDPara[0], m_nMACDPara[1], m_nMACDPara[2]);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + 5, m_rcMACD.left + 160, m_rcMACD.top + 20),
			strMarket, RGBA(255, 255, 255, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"DIF:%.02f", data.macd.dDIF);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + 5, (m_rcMACD.left + 240 > m_rcMACD.right ? m_rcMACD.right : m_rcMACD.left + 240), m_rcMACD.top + 20),
			strMarket, RGBA(255, 255, 255, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"DEA:%.02f", data.macd.dDEA);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + 5, (m_rcMACD.left + 240 > m_rcMACD.right ? m_rcMACD.right : m_rcMACD.left + 240), m_rcMACD.top + 20),
			strMarket, RGBA(255, 255, 0, 255));
		GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
		left += size.cx;
		strMarket.Format(L"MACD:%.3f", data.macd.dMACD);
		DrawTextonPic(pRT, CRect(m_rcMACD.left + left, m_rcMACD.top + 5, m_rcMACD.right, m_rcMACD.top + 20),
			strMarket, RGBA(255, 0, 255, 255));

	}
	pRT->ReleaseDC(hdc);
}

int SFenShiPic::GetXPos(int n) {	//获取id对应的x坐标
	double fx = m_rcMain.left + ((n + 0.5)*(double)(m_rcMain.Width() - 2) / (double)m_nAllLineNum + 0.5);
	int nx = (int)fx;
	if (nx < m_rcMain.left || nx > m_rcMain.right)
		nx = m_rcMain.left;
	return nx;
}

int SFenShiPic::GetXData(int nx) {	//获取鼠标下的数据id
	double fn = (double)(nx - m_rcMain.left) / ((double)(m_rcMain.Width() - 2) / (double)m_nAllLineNum) - 0.5;
	int n = (int)fn;
	if (n < 0 || n >= (int)m_pData->d.size())
		n = -1;
	return n;
}

void SFenShiPic::DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color, UINT uFormat)
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
	pMemRT->DrawTextW(str, wcslen(str), CRect(0, 0, rc.right - rc.left, rc.bottom - rc.top), uFormat);
	pRT->BitBlt(rc, pMemRT, 0, 0, SRCINVERT);

}

void SFenShiPic::DrawEarserLine(IRenderTarget * pRT, CPoint pt, bool bVertical)
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

		//	pMemRT->DrawTextW(L"Test", wcslen(L"Test"), CRect(m_rcImage.left, pt.y - 5, m_rcImage.right, pt.y + 5), DT_SINGLELINE);

		//	pRT->BitBlt(CRect(m_rcImage.left, pt.y-5, m_rcImage.right, pt.y+5), pMemRT, 0, 0, SRCINVERT);


	}


}

void SFenShiPic::GetMACDMaxDiff()		//判断副图坐标最大最小值和k线条数
{

	//判断最大最小值

	GetMacdDiff();

	if (m_pData->d.empty())
	{
		m_pData->fMaxMACD = 1;
		m_pData->fMinMACD = 0;
	}

	double fMin = m_pData->fMinMACD;
	double fMax = m_pData->fMaxMACD;
	if (fMax == fMin)
		fMax = fMax * 1.1;
	if (fMax == fMin&&fMax == 0)
		fMax = 1;

	fMax = std::fabs(fMax) > std::fabs(fMin) ? std::fabs(fMax) : std::fabs(fMin);
	fMin = -fMax;

	m_pData->fMaxMACD = fMax;
	m_pData->fMinMACD = fMin;


}

void SFenShiPic::InitVirTimeLineMap()
{
	m_virTimeLineMap.clear();
	m_virTimeLineMap[0] = L"09:30";
	m_virTimeLineMap[120] = L"13:00";
	m_virTimeLineMap[239] = L"15:00";

}

void SFenShiPic::DataProc()
{
	DataInit();
	if (m_bIsStockIndex)
	{
		m_bDataInited = true;
		IndexDataUpdate();
	}
	else
	{
		m_bDataInited = true;
		StockDataUpdate();
	}
}


void SFenShiPic::IndexDataUpdate()
{

	if (!m_bDataInited)
		return;
	if (m_pIdxMarketVec->empty())
		return;

	double OpenPrice = m_pIdxMarketVec->back().OpenPrice;
	if (OpenPrice > 10000000)
		return;

	int TickSize = m_pIdxMarketVec->size();
	if (m_pData->nCount == TickSize - 1)
		return;

	for (size_t i = m_pData->nCount; i < TickSize; ++i)
	{
		auto &tick = m_pIdxMarketVec->at(i);
		int time = tick.UpdateTime / 100;
		++m_pData->nCount;
		if (m_pData->nCount >= TickSize)
			m_pData->nCount = TickSize - 1;
		if (time < 930)
			continue;
		if (time == 1500 && i == 0)
			continue;

		if (time > 1500)
			break;

		int min = SetFenshiMin(time);
		if (-1 == min)
			min = time % 100;

		if (min != m_pData->nMin)
		{
			FENSHI_GROUP f1 = { 0 };
			if (TimeInGap(time))
			{
				SetFSData(f1, tick);
				SetGapMarketTime(f1, time);
				if (m_timeSet.count(f1.time) == 0)
					continue;
				if (!m_pData->d.empty())
					m_pData->d.pop_back();
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
				HandleMissData(f1, time);
			}
			else if (i != 0)
			{

				auto &preTick = m_pIdxMarketVec->at(i - 1);
				f1.time = preTick.UpdateTime / 100;
				if (m_timeSet.count(f1.time) == 0)
					continue;
				SetFSData(f1, preTick);
				m_pData->nMin = min;
				if (!m_pData->d.empty())
					m_pData->d.pop_back();
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
				m_pData->nLastVolume = preTick.Volume;
				HandleMissData(f1, time);

				ZeroMemory(&f1, sizeof(f1));
				SetFSData(f1, tick);
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
			}

		}
		else
		{
			if (m_timeSet.count(time) == 0)
				continue;
			if (m_pData->d.empty())
				m_pData->d.emplace_back(FENSHI_GROUP());
			auto &f1 = m_pData->d.back();
			SetFSData(f1, tick);
			MACDHandle(f1, 1);
			HandleMissData(f1, time);
		}
		m_pData->fMax = tick.HighPrice;
		m_pData->fMin = tick.LowPrice;
	}
}

void SFenShiPic::StockDataUpdate()
{

	if (!m_bDataInited)
		return;
	if (m_pStkMarketVec->empty())
		return;

	double OpenPrice = m_pStkMarketVec->back().OpenPrice;
	if (OpenPrice > 10000000)
		return;

	int TickSize = m_pStkMarketVec->size();
	if (m_pData->nCount == TickSize - 1)
		return;

	for (size_t i = m_pData->nCount; i < TickSize; ++i)
	{
		auto &tick = m_pStkMarketVec->at(i);
		int time = tick.UpdateTime / 100;
		++m_pData->nCount;
		if (m_pData->nCount >= TickSize)
			m_pData->nCount = TickSize - 1;
		if (time < 930)
			continue;
		if (time == 1500 && i == 0)
			continue;

		if (time > 1500)
			break;

		int min = SetFenshiMin(time);
		if (-1 == min)
			min = time % 100;

		if (min != m_pData->nMin)
		{
			FENSHI_GROUP f1 = { 0 };
			if (TimeInGap(time))
			{
				SetFSData(f1, tick);
				SetGapMarketTime(f1, time);
				if (m_timeSet.count(f1.time) == 0)
					continue;
				if (!m_pData->d.empty())
					m_pData->d.pop_back();
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
				HandleMissData(f1, time);
			}
			else if (i != 0)
			{

				auto &preTick = m_pStkMarketVec->at(i - 1);
				f1.time = preTick.UpdateTime / 100;
				if (m_timeSet.count(f1.time) == 0)
					continue;
				SetFSData(f1, preTick);
				m_pData->nMin = min;
				if (!m_pData->d.empty())
					m_pData->d.pop_back();
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
				m_pData->nLastVolume = preTick.Volume;
				HandleMissData(f1, time);

				ZeroMemory(&f1, sizeof(f1));
				SetFSData(f1, tick);
				MACDHandle(f1);
				m_pData->d.emplace_back(f1);
			}

		}
		else
		{
			if (m_timeSet.count(time) == 0)
				continue;
			if (m_pData->d.empty())
				m_pData->d.emplace_back(FENSHI_GROUP());
			auto &f1 = m_pData->d.back();
			SetFSData(f1, tick);
			MACDHandle(f1, 1);
			HandleMissData(f1, time);
		}
		m_pData->fMax = tick.HighPrice;
		m_pData->fMin = tick.LowPrice;
	}
}



void SFenShiPic::SetFSData(FENSHI_GROUP & f, CommonIndexMarket & market)
{
	f.close = market.LastPrice;
	f.avg = market.LastPrice;
	f.vol = market.Volume - m_pData->nLastVolume;

	f.date = market.TradingDay;
	f.time = market.UpdateTime / 100;
}

void SFenShiPic::SetFSData(FENSHI_GROUP & f, CommonStockMarket & market)
{
	f.close = market.LastPrice;
	f.avg = market.Volume == 0 ? market.LastPrice : market.Turnover / market.Volume;
	f.vol = market.Volume - m_pData->nLastVolume;

	f.date = m_nTradingDay;
	f.time = market.UpdateTime / 100;

}


void SFenShiPic::SetGapMarketTime(FENSHI_GROUP & f, int time)
{
	if (time == 1130)
		f.time = 1129;
	else if (time == 1500)
		f.time = 1459;
}


void SFenShiPic::DeleteLastData()
{
	//删除最后一条数据为交易时段最后一分钟的数据
	if (!m_pData->d.empty() &&
		(m_pData->d.at(m_pData->d.size() - 1).time == 1129
			|| m_pData->d.at(m_pData->d.size() - 1).time == 1459))
		m_pData->d.pop_back();
}


void SFenShiPic::ReProcEMA()
{
	for (size_t i = 0; i < m_pData->d.size(); i++)
	{
		if (i == 0)
		{
			m_pData->d[i].EMA1 = m_pData->d[i].close;
			m_pData->d[i].EMA2 = m_pData->d[i].close;
		}
		else
		{
			m_pData->d[i].EMA1 = m_dataHandler.EMA(m_nEMAPara[0], m_pData->d[i - 1].EMA1, m_pData->d[i - 1].close);
			m_pData->d[i].EMA2 = m_dataHandler.EMA(m_nEMAPara[1], m_pData->d[i - 1].EMA2, m_pData->d[i - 1].close);
		}
	}
	//	Invalidate();
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


bool SFenShiPic::TimeAfterGapHandle(int time, FENSHI_GROUP &f)
{
	if (TimeAfterGap(time))
	{
		//如果最后一条数据是上一个交易时段最后一条
		//重新处理上一条数据
		if (!m_pData->d.empty() && (m_pData->d.at(m_pData->d.size() - 1).time == 1129))
		{
			f.time = 1129;
			m_pData->d.pop_back();
		}
	}
	return true;
}

void SFenShiPic::ReProcMacd()
{


	if (!m_pData->d.empty())
	{
		double fMax = -10000000;
		double fMin = 10000000;

		for (auto iter = m_pData->d.begin() + 1; iter != m_pData->d.end(); iter++)
		{
			iter->macd.dEMA12 = m_dataHandler.EMA(m_nMACDPara[0], (iter - 1)->macd.dEMA12, iter->close);
			iter->macd.dEMA26 = m_dataHandler.EMA(m_nMACDPara[1], (iter - 1)->macd.dEMA26, iter->close);
			iter->macd.dDIF = iter->macd.dEMA12 - iter->macd.dEMA26;
			iter->macd.dDEA = m_dataHandler.EMA(m_nMACDPara[2], (iter - 1)->macd.dDEA, iter->macd.dDIF);
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
		m_pData->fMaxMACD = fMax;
		m_pData->fMinMACD = fMin;

	}

}

void SFenShiPic::UpdateData()
{
	if (m_pData == nullptr)
		return;
	if (m_bIsStockIndex)
		IndexDataUpdate();
	else
		StockDataUpdate();
	GetMaxDiff();
	GetFuTuMaxDiff();
	GetMACDMaxDiff();
	//	SSendMessage(WM_PAINT);
	//	Invalidate();
}

void SFenShiPic::SetWindowRect()
{
	if (!m_bShowDeal)
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right, m_rcAll.bottom);
	else
	{
		if (m_bIsStockIndex)
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 180, m_rcAll.top, m_rcAll.right + 30, m_rcAll.top + 180);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 180, m_rcAll.top + 185, m_rcAll.right + 30, m_rcAll.bottom + 30);
		}
		else
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 180, m_rcAll.top, m_rcAll.right + 30, m_rcAll.top + 560);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 180, m_rcAll.top + 565, m_rcAll.right + 30, m_rcAll.bottom + 30);
		}
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right - 240, m_rcAll.bottom);
	}

	CRect *pSubRect[3];
	int nShowCount = 0;
	if (m_bShowVolume)
	{
		pSubRect[nShowCount] = &m_rcVolume;
		nShowCount++;
	}
	else m_rcVolume.SetRectEmpty();

	if (m_bShowMacd)
	{
		pSubRect[nShowCount] = &m_rcMACD;
		nShowCount++;
	}
	else m_rcMACD.SetRectEmpty();

	if (m_bShowSubPic)
	{
		pSubRect[nShowCount] = m_pSubPic->GetPicRect();
		nShowCount++;
	}
	else
		m_pSubPic->SetPicRect(CRect(0, 0, 0, 0));
	int preBottom = m_rcImage.top;
	int nowBottom = m_rcImage.top + m_rcImage.Height() / (nShowCount + 2) * 2;
	m_rcMain.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.top + m_rcImage.Height() / (nShowCount + 2) * 2);
	preBottom = nowBottom;
	for (int i = 0; i < nShowCount; ++i)
	{
		nowBottom = m_rcImage.top + m_rcImage.Height() / (nShowCount + 2) * (3 + i);
		pSubRect[i]->SetRect(m_rcImage.left, preBottom, m_rcImage.right, nowBottom);
		preBottom = nowBottom;
	}

	//if (m_bShowMacd&&m_bShowVolume)
	//{
	//	m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5 * 2);
	//	m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 5, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 5);
	//	m_rcMACD.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 4 / 5, m_rcImage.right, m_rcImage.bottom);
	//}
	//else if (m_bShowVolume)
	//{
	//	m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
	//	m_rcLower.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
	//	m_rcMACD.SetRect(0, 0, 0, 0);
	//}
	//else if (m_bShowMacd)
	//{
	//	m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom - (m_rcImage.bottom - m_rcImage.top) / 4);
	//	m_rcLower.SetRect(0, 0, 0, 0);
	//	m_rcMACD.SetRect(m_rcImage.left, m_rcImage.top + (m_rcImage.bottom - m_rcImage.top) * 3 / 4, m_rcImage.right, m_rcImage.bottom);
	//}
	//else
	//{
	//	m_rcUpper.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right, m_rcImage.bottom);
	//	m_rcLower.SetRect(0, 0, 0, 0);
	//	m_rcMACD.SetRect(0, 0, 0, 0);
	//}
}

int SFenShiPic::SetFenshiMin(int nTime, bool bSetData)
{
	if (nTime >= 925 && nTime < 930)
		return 30;
	return -1;
}

void SFenShiPic::GetMacdDiff()
{

	m_pData->fMinMACD = 10;
	m_pData->fMaxMACD = -10;
	for (size_t i = 0; i < m_pData->d.size(); i++)
	{
		if (m_pData->d[i].macd.dDEA < m_pData->fMinMACD)
			m_pData->fMinMACD = m_pData->d[i].macd.dDEA;
		if (m_pData->d[i].macd.dDEA > m_pData->fMaxMACD)
			m_pData->fMaxMACD = m_pData->d[i].macd.dDEA;
		if (m_pData->d[i].macd.dDIF < m_pData->fMinMACD)
			m_pData->fMinMACD = m_pData->d[i].macd.dDIF;
		if (m_pData->d[i].macd.dDIF > m_pData->fMaxMACD)
			m_pData->fMaxMACD = m_pData->d[i].macd.dDIF;
		if (m_pData->d[i].macd.dMACD < m_pData->fMinMACD)
			m_pData->fMinMACD = m_pData->d[i].macd.dMACD;
		if (m_pData->d[i].macd.dMACD > m_pData->fMaxMACD)
			m_pData->fMaxMACD = m_pData->d[i].macd.dMACD;
	}
}

void SFenShiPic::MACDHandle(FENSHI_GROUP & f1, int nOffset)
{
	if (m_pData->d.size() <= nOffset)
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
		size_t size = m_pData->d.size();
		auto &LastData = m_pData->d[size - nOffset - 1];
		f1.EMA1 = m_dataHandler.EMA(m_nEMAPara[0], LastData.macd.dEMA12, f1.close);
		f1.EMA2 = m_dataHandler.EMA(m_nEMAPara[1], LastData.macd.dEMA26, f1.close);
		f1.macd.dEMA12 = m_dataHandler.EMA(m_nMACDPara[0], LastData.macd.dEMA12, f1.close);
		f1.macd.dEMA26 = m_dataHandler.EMA(m_nMACDPara[1], LastData.macd.dEMA26, f1.close);
		f1.macd.dDIF = f1.macd.dEMA12 - f1.macd.dEMA26;
		f1.macd.dDEA = m_dataHandler.EMA(m_nMACDPara[2], LastData.macd.dDEA, f1.macd.dDIF);
		f1.macd.dMACD = 2 * (f1.macd.dDIF - f1.macd.dDEA);
	}

}

void SFenShiPic::HandleNoDataTime(FENSHI_GROUP f1)
{
	if (m_pData->d.size() == 1)
	{
		std::vector<int> timeVec(m_timeSet.begin(), m_timeSet.end());
		std::sort(timeVec.begin(), timeVec.end());
		std::vector<int> YeseterdayTime;
		std::vector<int> TodayTime;
		if (f1.time >= 2100)
		{
			for (auto &it : timeVec)
			{
				if (it < f1.time && it >= 2100)
					YeseterdayTime.emplace_back(it);
			}

		}
		else
		{
			for (auto &it : timeVec)
			{
				if (it >= 2100)
					YeseterdayTime.emplace_back(it);
				else if (it < f1.time)
					TodayTime.emplace_back(it);
			}
		}
		m_pData->d.clear();
		FENSHI_GROUP data = f1;
		data.close = data.avg = m_pData->fPreClose;
		data.vol = 0;
		for (auto &it : YeseterdayTime)
		{
			data.time = it;
			MACDHandle(data);
			m_pData->d.emplace_back(data);
		}
		for (auto &it : TodayTime)
		{
			data.time = it;
			MACDHandle(data);
			m_pData->d.emplace_back(data);
		}
		MACDHandle(f1);
		m_pData->d.emplace_back(f1);
	}

}

void SFenShiPic::HandleMissData(FENSHI_GROUP f1, int time)//补全遗漏的数据
{
	HandleNoDataTime(f1);

	if (time - f1.time != 1)
	{
		std::vector<int> timeVec(m_timeSet.begin(), m_timeSet.end());
		std::sort(timeVec.begin(), timeVec.end());
		std::vector<int> YeseterdayTime;
		std::vector<int> TodayTime;
		if (time >= 2100)
		{
			for (auto &it : timeVec)
			{
				if (it < time && it >f1.time && it >= 2100)
					YeseterdayTime.emplace_back(it);
			}

		}
		else if (f1.time >= 2100)
		{
			for (auto &it : timeVec)
			{
				if (it > f1.time)
					YeseterdayTime.emplace_back(it);
				else if (it < time)
					TodayTime.emplace_back(it);
			}
		}
		else
		{
			for (auto &it : timeVec)
				if (it < time && it >f1.time)
					YeseterdayTime.emplace_back(it);
		}
		FENSHI_GROUP data = f1;
		data.vol = 0;
		for (auto &it : YeseterdayTime)
		{
			data.time = it;
			MACDHandle(data);
			m_pData->d.emplace_back(data);
		}
		for (auto &it : TodayTime)
		{
			data.time = it;
			MACDHandle(data);
			m_pData->d.emplace_back(data);
		}
	}


}


COLORREF SFenShiPic::GetColor(double dPrice)
{
	if (dPrice > m_pData->fPreClose)
		return RGBA(255, 0, 0, 255);
	else if (dPrice < m_pData->fPreClose)
		return RGBA(0, 255, 0, 255);
	else
		return RGBA(255, 255, 255, 255);
}

//BOOL SFenShiPic::CreateChildren(pugi::xml_node xmlNode)
//{
//	if (!__super::CreateChildren(xmlNode))
//		return FALSE;
//	m_pSubPic = nullptr;
//
//	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
//	while (pChild)
//	{
//		if (pChild->IsClass(SSubPic::GetClassName()))
//		{
//			m_pSubPic = (SSubPic*)pChild;
//			break;
//		}
//		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
//	}
//}


void SFenShiPic::DrawMouse(IRenderTarget * pRT, CPoint po, BOOL bFromOnPaint)
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
	{

		HDC hdc = pRT->GetDC();
		int  nMode = SetROP2(hdc, R2_NOTXORPEN);
		MoveToEx(hdc, m_nMouseX, m_rcMain.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcMain.bottom);	//竖线
		MoveToEx(hdc, po.x, m_rcMain.top + 20, NULL);			LineTo(hdc, po.x, m_rcMain.bottom);
		if (m_bShowVolume)
		{
			MoveToEx(hdc, m_nMouseX, m_rcVolume.top, NULL);	LineTo(hdc, m_nMouseX, m_rcVolume.bottom);
			MoveToEx(hdc, po.x, m_rcVolume.top, NULL);			LineTo(hdc, po.x, m_rcVolume.bottom);
		}
		if (m_bShowMacd)
		{
			MoveToEx(hdc, m_nMouseX, m_rcMACD.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcMACD.bottom);
			MoveToEx(hdc, po.x, m_rcMACD.top + 20, NULL);			LineTo(hdc, po.x, m_rcMACD.bottom);

		}
		MoveToEx(hdc, m_rcMain.left, m_nMouseY, NULL);	LineTo(hdc, m_rcMain.right, m_nMouseY);	//横线
		MoveToEx(hdc, m_rcMain.left, po.y, NULL);			LineTo(hdc, m_rcMain.right, po.y);

		SetROP2(hdc, nMode);

		pRT->ReleaseDC(hdc);
	}
	//显示横坐标所在数值
	CAutoRefPtr<IBrush> bBrushDarkBlue;
	pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &bBrushDarkBlue);
	pRT->SelectObject(bBrushDarkBlue);

	if (m_bDataInited)
	{
		SStringW sl, sr;
		sr.Empty(); sl.Empty();
		if (m_nMouseY >= m_rcMain.top && m_nMouseY <= m_rcMain.bottom)	//十字在k线上
		{
			sl = GetYPrice(m_nMouseY, FALSE);
			sr = GetYPrice(m_nMouseY, TRUE);

		}
		else if (m_nMouseY >= m_rcVolume.top && m_nMouseY <= m_rcVolume.bottom)	//十字在附图上
			sr = GetFuTuYPrice(m_nMouseY);
		else if (m_nMouseY >= m_rcMACD.top && m_nMouseY <= m_rcMACD.bottom)
			sr = GetMACDYPrice(m_nMouseY);
		if (!sr.IsEmpty())
		{
			DrawTextonPic(pRT, CRect(m_rcMain.right + 1, m_nMouseY - 10, m_rcMain.right + RC_FSLEFT - 2, m_nMouseY + 10), sr);
			//				pRT->FillRectangle(CRect(m_rcUpper.right + 1, m_nMouseY + 2, m_rcUpper.right + RC_FSLEFT+1, m_nMouseY + 22));
		}
		if (!sl.IsEmpty())
		{
			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 2, m_nMouseY - 10, m_rcMain.left - 1, m_nMouseY + 10), sl);
			//				pRT->FillRectangle(CRect(m_rcUpper.left - RC_FSLEFT+1, m_nMouseY + 2, m_rcUpper.left, m_nMouseY + 22));
		}




		sr.Empty(); sl.Empty();
		if (po.y >= m_rcMain.top && po.y <= m_rcMain.bottom)	//十字在k线上
		{
			sl = GetYPrice(po.y, FALSE);
			sr = GetYPrice(po.y, TRUE);
		}
		else if (po.y >= m_rcVolume.top && po.y <= m_rcVolume.bottom)	//十字在附图上
			sr = GetFuTuYPrice(po.y);
		else if (po.y >= m_rcMACD.top && po.y <= m_rcMACD.bottom)
			sr = GetMACDYPrice(po.y);

		if (!sr.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcMain.right + 1, po.y - 10, m_rcMain.right + RC_FSLEFT - 2, po.y + 10), sr);
		if (!sl.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 2, po.y - 10, m_rcMain.left - 1, po.y + 10), sl);

		//显示纵坐标数值

		if (IsInRect(m_nMouseX, m_nMouseY, 0))
		{
			int nx = GetXData(m_nMouseX);

			if (nx >= 0 && m_pData->d[nx].date > 0)
			{
				FENSHI_GROUP *p = &(m_pData->d[nx]);
				sl.Format(L"%02d-%02d  %02d:%02d", p->date % 10000 / 100, p->date % 100, p->time / 100,
					p->time % 100);
				DrawTextonPic(pRT, CRect(m_nMouseX, m_rcImage.bottom, m_nMouseX + 80, m_rcImage.bottom + 20), sl, RGBA(255, 255, 255, 255), 0);
			}

		}
		if (IsInRect(po.x, po.y, 0))
		{
			int nx = GetXData(po.x);

			if (nx >= 0 && m_pData->d[nx].date > 0)
			{
				FENSHI_GROUP *p = &(m_pData->d[nx]);
				sl.Format(L"%02d-%02d  %02d:%02d", p->date % 10000 / 100, p->date % 100, p->time / 100,
					p->time % 100);
				DrawTextonPic(pRT, CRect(po.x, m_rcImage.bottom, po.x + 80, m_rcImage.bottom + 20), sl, RGBA(255, 255, 255, 255), 0);
			}

		}
		//在左上角添加此时刻具体行情


		if (IsInRect(m_nMouseX, m_nMouseY, 0) && m_bShowMouseLine)
		{
			int nx = GetXData(m_nMouseX);
			FENSHI_GROUP p = { 0 };
			if (nx >= 0 && m_pData->d[nx].date > 0)
				p = (m_pData->d[nx]);
			else if (nx == -1 && !m_pData->d.empty())
				p = (m_pData->d.at(m_pData->d.size() - 1));

			if (p.date != 0)
				DrawUpperMarket(pRT, p);



		}
		if (IsInRect(po.x, po.y, 0) && m_bShowMouseLine)
		{
			int nx = GetXData(po.x);
			FENSHI_GROUP p = { 0 };
			if (nx >= 0 && m_pData->d[nx].date > 0)
				p = (m_pData->d[nx]);
			else if (nx == -1 && !m_pData->d.empty())
				p = (m_pData->d.at(m_pData->d.size() - 1));
			if (p.date != 0)
				DrawUpperMarket(pRT, p);

		}

	}
	if (m_bShowSubPic)
		m_pSubPic->DrawMouse(pRT, po, bFromOnPaint);;
	m_nMouseX = po.x;
	m_nMouseY = po.y;

}



void SFenShiPic::DrawData(IRenderTarget * pRT)
{
	CPoint pts[5];
	int x = 0, yavg = 0, yclose = 0, ypreavg = 0, ypreclose = 0;
	int yDIF = 0, yDEA = 0, ypreDIF = 0, ypreDEA = 0;
	int yEMA1 = 0, yEMA2 = 0, yPreEMA1 = 0, yPreEMA2 = 0;
	int yFutStock = 0;
	CAutoRefPtr<IPen> penWhite, penYellow, oldPen, penRed, penBlue, penGreen, penPurple;
	CAutoRefPtr<IBrush> bBrush, bOldBrush;
	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);


	int nYoNum = 9;		//y轴标示数量 3 代表画两根线

	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 50, 255), 1, &penYellow);
	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &penWhite);
	pRT->CreatePen(PS_SOLID, RGBA(255, 20, 0, 255), 1, &penRed);
	pRT->CreatePen(PS_SOLID, RGBA(0, 255, 255, 255), 1, &penBlue);
	pRT->CreatePen(PS_SOLID, RGBA(0, 255, 0, 255), 1, &penGreen);
	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 255, 255), 1, &penPurple);
	pRT->CreateSolidColorBrush(RGBA(0, 255, 255, 255), &bBrush);

	pRT->SelectObject(penWhite, (IRenderObj**)&oldPen);
	pRT->SelectObject(bBrush, (IRenderObj**)&bOldBrush);


	//k线区加坐标
	int nWidth = m_nHeight / 8;
	for (size_t i = 0; i < nYoNum; i++)
	{
		int nY = m_rcMain.top + 20 + nWidth * i;

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
			int nY = m_rcMACD.top + 20 + (m_rcMACD.Height() - 20) / 4 * i;

			//k线区y轴加轴标
			SStringW s1 = GetMACDYPrice(nY);

			DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 5, nY - 9, m_rcMain.left, nY + 9), s1, RGBA(255, 255, 255, 255), DT_CENTER);
		}
	}


	int nDataNum = m_pData->d.size();
	if (nDataNum > m_nAllLineNum)
		nDataNum = m_nAllLineNum;
	int nFutStockDataNum = 0;


	CPoint *MainLine = new CPoint[nDataNum];
	CPoint* AvgLine = new CPoint[nDataNum];
	CPoint* EMA1Line = new CPoint[nDataNum];
	CPoint* EMA2Line = new CPoint[nDataNum];
	CPoint* DIFLine = new CPoint[nDataNum];
	CPoint* DEALine = new CPoint[nDataNum];


	for (size_t i = 0; i < nDataNum; i++)
	{

		x = GetXPos(i);
		int xpre = GetXPos(i - 1);
		yavg = GetYPos(m_pData->d[i].avg);
		yclose = GetYPos(m_pData->d[i].close);
		yDEA = GetMACDYPos(m_pData->d[i].macd.dDEA);
		yDIF = GetMACDYPos(m_pData->d[i].macd.dDIF);
		yEMA1 = GetYPos(m_pData->d[i].EMA1);
		yEMA2 = GetYPos(m_pData->d[i].EMA2);

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
			DrawTextonPic(pRT, CRect(m_rcMain.left, m_rcMain.top - 20, m_rcMain.right, m_rcMain.top), strTemp, RGBA(255, 255, 0, 255));

			strTemp.Format(L"Price:%.02f", m_pData->d[i].close);
			DrawTextonPic(pRT, CRect(m_rcMain.right - 160, m_rcMain.top - 20, m_rcMain.right - 80, m_rcMain.top),
				strTemp, RGBA(255, 255, 255, 255));
			strTemp.Format(L"Avg:%.02f", m_pData->d[i].avg);
			DrawTextonPic(pRT, CRect(m_rcMain.right - 80, m_rcMain.top - 20, m_rcMain.right, m_rcMain.top),
				strTemp, RGBA(255, 255, 0, 255));
			CPoint pt;
			GetCursorPos(&pt);
			//			if (!m_bShowMouseLine||(pt.x>m_rcImage.right||pt.x<m_rcImage.left||pt.y>m_rcImage.bottom||pt.y<m_rcImage.top))
			if (!m_bShowMouseLine)
				DrawUpperMarket(pRT, m_pData->d[i]);

		}

		MainLine[i].SetPoint(x + width, yclose);

		AvgLine[i].SetPoint(x + width, yavg);

		//画量
		if (m_bShowVolume)
		{
			pRT->SelectObject(penYellow);
			//		pRT->DrawRectangle(CRect(x + width, GetFuTuYPos(m_pData->d[i].vol), x + width, m_rcLower.bottom - 1));

			int vol = m_bIsStockIndex ? 
				m_pData->d[i].vol : m_pData->d[i].vol / 100;
			if (m_pData->d[i].vol != 0)
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

			int nWidthMacd = (m_rcMACD.Height() - 20) / 4;
			//MACD柱状图
			if (m_pData->d[i].macd.dMACD != 0)
			{
				pts[0].SetPoint(x + width, m_rcMACD.top + 20 + 2 * nWidthMacd);
				pts[1].SetPoint(x + width, GetMACDYPos(m_pData->d[i].macd.dMACD));

				if (m_pData->d[i].macd.dMACD > 0)
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

	if (m_bShowSubPic)
		m_pSubPic->DrawData(pRT);

}

void SFenShiPic::OnDbClickedFenshi(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	m_bShowMouseLine = !m_bShowMouseLine;
	m_bKeyDown = FALSE;
	m_pSubPic->SetMouseLineState(m_bShowMouseLine);
	Invalidate();
}

void SFenShiPic::OnRButtonUp(UINT nFlags, CPoint point)
{
	//LPARAM lp = MAKEWORD(Group_SWL1)
	//m_rgClickGroup = m_rgGroup;
	SMenu menu;
	menu.LoadMenuW(L"smenu:menu_fenshi");
	if (m_bShowDeal)
		menu.CheckMenuItem(FM_Deal, MF_CHECKED);
	if (m_bShowVolume)
		menu.CheckMenuItem(FM_Volume, MF_CHECKED);
	if (m_bShowMacd)
		menu.CheckMenuItem(FM_MACD, MF_CHECKED);
	if (m_bShowSubPic)
		menu.CheckMenuItem(FM_RPS, MF_CHECKED);
	if (m_bShowAvg)
		menu.CheckMenuItem(FM_Avg, MF_CHECKED);
	if (m_bShowEMA)
		menu.CheckMenuItem(FM_EMA, MF_CHECKED);
	ClientToScreen(g_MainWnd, &point);
	menu.TrackPopupMenu(0, point.x, point.y, g_MainWnd);
}

void SFenShiPic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	switch (nChar)
	{
	case VK_LEFT:
	{
		m_bShowMouseLine = true;
		m_bKeyDown = true;

		CRect rc = GetClientRect();
		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
		SPainter pa;
		SWindow::BeforePaint(pRT, pa);
		DrawKeyDownMouseLine(pRT, VK_LEFT);
		AfterPaint(pRT, pa);
		ReleaseRenderTarget(pRT);
	}
	break;
	case VK_RIGHT:
	{
		m_bShowMouseLine = true;
		m_bKeyDown = true;

		CRect rc = GetClientRect();
		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
		SPainter pa;
		SWindow::BeforePaint(pRT, pa);
		//		DrawKeyDownMouseLine(pRT);


		DrawKeyDownMouseLine(pRT, VK_RIGHT);
		AfterPaint(pRT, pa);
		ReleaseRenderTarget(pRT);
	}
	break;
	default:
		break;
	}
	//m_pSubPic->OnKeyUp(nChar, nRepCnt, nFlags);
	//Invalidate();
}

void SFenShiPic::DataInit()
{
	if (m_pData == nullptr)
	{
		m_pData = new FENSHI_INFO;
		ZeroMemory(m_pData, sizeof(m_pData));
	}
	else
	{
		delete m_pData;
		m_pData = new FENSHI_INFO;
		ZeroMemory(m_pData, sizeof(m_pData));
	}

	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_nTradingDay = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

	SingleInit();

}

void SFenShiPic::SingleInit()
{
	m_pData->nCount = 0;
	m_pData->nLastVolume = 0;
	m_pData->nTime = 930;
	m_pData->nMin = 30;
	int nDigit = 0;

	if (m_bIsStockIndex)
		m_bShowAvg = false;

	InitVirTimeLineMap();
	SetShowTime();
	m_pData->nAllLineNum = 240;
	m_nAllLineNum = m_pData->nAllLineNum;
	m_pData->d.reserve(m_pData->nAllLineNum + 20);

	m_pData->fMaxMACD = -100;
	m_pData->fMinMACD = 100;

	m_bDataInited = true;

	SetTimer(1, 1000);
	if (m_bIsStockIndex)
	{
		if (m_pIdxMarketVec->empty())
			return;
		m_pData->fPreClose = m_pIdxMarketVec->back().PreCloPrice;
		m_pData->fMax = m_pIdxMarketVec->back().HighPrice;
		m_pData->fMin = m_pIdxMarketVec->back().LowPrice;

	}
	else
	{
		if (m_pStkMarketVec->empty())
			return;

		m_pData->fPreClose = m_pStkMarketVec->back().PreCloPrice;
		m_pData->fMax = m_pStkMarketVec->back().HighPrice;
		m_pData->fMin = m_pStkMarketVec->back().LowPrice;
	}
	m_pData->nMin = 30;
}


void SFenShiPic::SetShowTime()
{
	m_timeSet.clear();

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

void SFenShiPic::DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar)
{
	if (m_pData->d.empty())
		return;

	if (m_bIsFirstKey)
	{
		m_nNowPosition = GetXData(m_nMouseX);
		if (m_nNowPosition > m_pData->d.size() - 1)
			m_nNowPosition = m_pData->d.size() - 1;
		else if (m_nNowPosition < 0)
			m_nNowPosition = 0;
		m_bIsFirstKey = false;
		Invalidate();
		return;
	}

	int width = int(m_rcMain.Width() / m_nAllLineNum / 2 + 0.5);

	//画鼠标线
	CPoint po;
	po.x = GetXPos(m_nNowPosition) + width;
	po.y = GetYPos(m_pData->d[m_nNowPosition].close);
	HDC hdc = pRT->GetDC();
	//	HPEN pen, oldPen;
	//	pen = CreatePen(PS_SOLID, 1, RGBA(255, 255, 255, 0xFF));
	//	oldPen = (HPEN)SelectObject(hdc, pen);
	CRect rcClient;
	GetClientRect(rcClient);
	int  nMode = SetROP2(hdc, R2_NOT);
	//	MoveToEx(hdc, po.x + width, m_rcImage.top + 20, NULL);			LineTo(hdc, po.x + width, m_rcImage.bottom);
	//	MoveToEx(hdc, m_rcImage.left, po.y, NULL);		LineTo(hdc, m_rcUpper.right, po.y);

	if (!m_bShowMacd)
	{
		MoveToEx(hdc, po.x, m_rcImage.top + 20, NULL);			LineTo(hdc, po.x, m_rcImage.bottom);
		MoveToEx(hdc, m_rcImage.left, po.y, NULL);			LineTo(hdc, m_rcImage.right, po.y);

	}
	else
	{
		MoveToEx(hdc, po.x, m_rcImage.top + 20, NULL);			LineTo(hdc, po.x, m_rcMACD.top);
		MoveToEx(hdc, po.x, m_rcMACD.top + 20, NULL);			LineTo(hdc, po.x, m_rcMACD.bottom);
		MoveToEx(hdc, m_rcImage.left, po.y, NULL);			LineTo(hdc, m_rcImage.right, po.y);

	}


	//	SelectObject(hdc, oldPen);
	SetROP2(hdc, nMode);
	pRT->ReleaseDC(hdc);

	//显示横坐标轴数值

	FENSHI_GROUP p = m_pData->d[m_nNowPosition];


	SStringW sl, sr;

	CAutoRefPtr<IBrush> bBrushDarkBlue;
	pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &bBrushDarkBlue);
	pRT->SelectObject(bBrushDarkBlue);
	sr.Empty(); sl.Empty();
	sl.Format(L"%.2f", p.close);
	sr.Format(L"%.2f%%", 100 * (p.close - m_pData->fPreClose) / m_pData->fPreClose);
	if (!sr.IsEmpty())
	{
		//		pRT->FillRectangle(CRect(m_rcUpper.right + 1, po.y + 2, m_rcUpper.right + 40, po.y + 22));
		DrawTextonPic(pRT, CRect(m_rcMain.right + 1, po.y + 2, m_rcMain.right + 40, po.y + 22), sr);
		//		InvalidateRect(CRect(m_rcUpper.right + 1, po.y + 2, m_rcUpper.right + 40, po.y + 22));

	}
	if (!sl.IsEmpty())
	{
		//		pRT->FillRectangle(CRect(m_rcImage.left - RC_FSLEFT+1, m_rcImage.top, m_rcImage.left, m_rcImage.bottom));
		DrawTextonPic(pRT, CRect(m_rcMain.left - RC_FSLEFT + 1, po.y + 2, m_rcMain.left - 1, po.y + 22), sl);
		//		InvalidateRect(CRect(m_rcImage.left - RC_FSLEFT+1, m_rcImage.top, m_rcImage.left, m_rcImage.bottom));
	}

	//显示纵坐标轴数值
	sl.Format(L"%02d-%02d  %02d:%02d", p.date % 10000 / 100, p.date % 100, p.time / 100,
		p.time % 100);
	DrawTextonPic(pRT, CRect(po.x, m_rcImage.bottom, po.x + 80, m_rcImage.bottom + 20), sl, RGBA(255, 255, 255, 255), 0);


	//左上角显示行情
	DrawUpperMarket(pRT, p);
	m_pSubPic->SetNowKeyDownLinePos(m_nNowPosition);
	m_pSubPic->DrawKeyDownMouseLine(pRT);

	if (nChar == VK_LEFT)
	{
		if (-1 == m_nNowPosition)
			m_nNowPosition = m_pData->d.size() - 1;
		else if (0 == m_nNowPosition)
			m_nNowPosition = m_pData->d.size() - 1;
		else
			m_nNowPosition--;
		DrawKeyDownMouseLine(pRT, 0);
		//m_pSubPic->SetNowKeyDownLinePos(m_nNowPosition);
		//m_pSubPic->DrawKeyDownMouseLine(pRT);

	}
	else if (nChar == VK_RIGHT)
	{
		if (-1 == m_nNowPosition)
			m_nNowPosition = 0;
		else if (m_pData->d.size() - 1 == m_nNowPosition)
			m_nNowPosition = 0;
		else
			m_nNowPosition++;
		DrawKeyDownMouseLine(pRT, 0);
		//m_pSubPic->SetNowKeyDownLinePos(m_nNowPosition);
		//m_pSubPic->DrawKeyDownMouseLine(pRT);

	}

	//Invalidate();
}


