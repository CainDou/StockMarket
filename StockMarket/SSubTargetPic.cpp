#include "stdafx.h"
#include "SSubTargetPic.h"
#include <unordered_map>
#include <fstream>
#include<windows.h>
#include<map>
#include<math.h>
#include<vector>

using std::vector;
//extern bool g_bDataInited;

#define MARGIN 20
#define MAX_LINE 10

#define MIN_LINE_NUM 60
#define MAX_LINE_NUM 5000
#define DEFAULT_LINE_NUM 240
#define ZOOMRATIO 1.5

#define RIGHT_BLANK 50
#define	ZOOMWIDTH (m_nWidth * 1.0 / m_nZoomRatio)
#define TOTALZOOMWIDTH ((m_nWidth + m_nJiange) * 1.0 / m_nZoomRatio)

SSubTargetPic::SSubTargetPic()
{
	m_nTickPre = 0;
	m_nMouseX = m_nMouseY = -1;
	m_nIndex = -1;
	m_pData = nullptr;

	m_nAllLineNum = DEFAULT_LINE_NUM;

	m_nPaintTick = GetTickCount();
	m_bDataInited = false;


	m_nNowPosition = 0;
	m_bShowMouseLine = false;
	m_bKeyDown = false;
	m_bIsFirstKey = true;
	m_bUseWidth = false;
	m_bPaintInit = FALSE;

	m_StockID = "";

	m_fMaxL = 100;
	m_fMinL = 0;
	m_fMaxR = 100;
	m_fMinR = 0;
	m_fDeltaL = 100;
	m_fDeltaR = 100;

	m_nOffset = 0;
	m_nFirst = 0;
	//m_pRTBuffer = nullptr;
}

SSubTargetPic::~SSubTargetPic()
{
	delete[]m_pData;
}

void SSubTargetPic::SetShowData(int nIndex, bool bGroup)
{
	m_nIndex = nIndex;
	m_bKeyDown = false;
	m_nNowPosition = 0;
	m_bIsFirstKey = true;
}

void SSubTargetPic::SetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec,
	vector<SStringA> dataNameVec, SStringA StockID, SStringA StockName)
{
	m_bDataInited = false;
	SetOffset2Zero();
	if (m_pData)
	{
		delete[]m_pData;
		m_pData = nullptr;
	}

	m_nShowDataCount = nDataCount;
	m_pData = new vector<CoreData>*[nDataCount];
	m_bRightArr = bRightVec;
	m_StockID = StockID;
	m_StockName = StockName;
	m_dataNameVec = dataNameVec;
	for (int i = 0; i < nDataCount; ++i)
		m_pData[i] = data[i];

	m_bDataInited = true;
}






void SSubTargetPic::InitColorAndPen(IRenderTarget *pRT)
{
	m_colorVec.reserve(MAX_LINE);
	m_penVec.resize(MAX_LINE);
	m_colorVec.emplace_back(RGBA(255, 255, 255, 255));
	m_colorVec.emplace_back(RGBA(255, 255, 50, 255));
	m_colorVec.emplace_back(RGBA(255, 0, 255, 255));
	m_colorVec.emplace_back(RGBA(0, 255, 0, 255));
	m_colorVec.emplace_back(RGBA(0, 255, 255, 255));
	m_colorVec.emplace_back(RGBA(100, 100, 100, 255));
	m_colorVec.emplace_back(RGBA(176, 196, 222, 255));
	m_colorVec.emplace_back(RGBA(100, 100, 255, 255));
	m_colorVec.emplace_back(RGBA(255, 0, 0, 255));
	m_colorVec.emplace_back(RGBA(0, 0, 255, 255));
	for (int i = 0; i < MAX_LINE; ++i)
		pRT->CreatePen(PS_SOLID, m_colorVec[i], 1, &m_penVec[i]);
}

void SSubTargetPic::OnPaint(IRenderTarget * pRT)
{
	if (!m_bPaintInit)
	{
		m_bPaintInit = true;
		InitColorAndPen(pRT);
	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);

	m_rcImage.DeflateRect(RC_FSLEFT, 0, RC_FSRIGHT, 0);



	DrawArrow(pRT);


	if (m_bDataInited)
	{
		GetMaxDiff();
		DrawData(pRT);
	}

	if (m_bKeyDown)
		DrawKeyDownMouseLine(pRT/*, 0*/);
	else
	{
		CPoint po(m_nMouseX, m_nMouseY);
		m_nMouseX = m_nMouseY = -1;
		DrawMouse(pRT, po, TRUE);
	}
}

void SSubTargetPic::DrawArrow(IRenderTarget * pRT)
{
	//画k线区
	m_nHeight = m_rcImage.bottom - m_rcImage.top - 20;

	int nYoNum = m_nHeight / 20;
	if (nYoNum % 2 == 0)
		nYoNum--;
	if (nYoNum > 9)
		nYoNum = 9;
	if (nYoNum == 1)
		nYoNum = 3;
	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(m_rcImage.left, m_rcImage.top);
		pts[1].SetPoint(m_rcImage.left, m_rcImage.bottom);
		pts[2].SetPoint(m_rcImage.right, m_rcImage.bottom);
		pts[3].SetPoint(m_rcImage.right, m_rcImage.top);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		pRT->SelectObject(oldPen);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));

	//k线区横向虚线

	COLORREF clRed = RGB(139, 0, 0);
	HDC pdc = pRT->GetDC();
	//	SetBkColor(pdc, RGBA(255, 0, 0, 0xff));
	double width = m_nHeight*1.0 / (nYoNum - 1) ;
	for (int i = 0; i < nYoNum; i++)
	{
		double fY = m_rcImage.top + 20 + width * i;
		int nY = round(fY);
		CPoint pts[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pts[0].SetPoint(m_rcImage.left, nY);
			pts[1].SetPoint(m_rcImage.right, nY);

			if (i == nYoNum / 2)
				pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			else if (nY == m_rcImage.bottom)
				pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
			else
				pRT->CreatePen(PS_SOLID, RGBA(150, 0, 0, 0xFF), 1, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldPen);
			//y轴	//x轴
			pRT->DrawLines(pts, 2);
			pRT->SelectObject(oldPen);
		}

	}


	pRT->ReleaseDC(pdc);


}

void SSubTargetPic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{

	int nLen = m_rcImage.right - m_rcImage.left;	//判断是否超出范围
													//判断最大最小值
	m_nFirst = 0;
	m_nEnd = min(m_pData[0]->size(), m_pData[1]->size());
	if (m_nEnd > m_nAllLineNum)
		m_nFirst = m_nEnd - m_nAllLineNum;

	if (m_nOffset > 0)
	{
		if (m_nFirst <= 0 && m_nEnd == m_pData[0]->size())
			m_nOffset = 0;
		if (m_pData[0]->size() < m_nOffset)
			m_nOffset = m_pData[0]->size();
		else if (m_nFirst > m_nOffset)
		{
			m_nFirst -= m_nOffset;
			m_nEnd = m_pData[0]->size() - m_nOffset;
		}
		else
		{
			m_nEnd -= m_nFirst;
			m_nFirst = 0;
		}

	}


	for (unsigned i = 0; i < m_nShowDataCount; i++)
	{
		if (m_bRightArr[i])
		{
			for (int j = m_nFirst; j < m_nEnd; ++j)
			{
				m_fMaxR = max(m_fMaxR, m_pData[i]->at(j).value);
				m_fMinR = min(m_fMinR, m_pData[i]->at(j).value);
			}
		}
		else
		{
			for (int j = m_nFirst; j < m_nEnd; ++j)
			{
				m_fMaxL = max(m_fMaxR, m_pData[i]->at(j).value);
				m_fMinL = min(m_fMinR, m_pData[i]->at(j).value);
			}
		}
	}
	m_fDeltaL = (m_fMaxL == MININT || m_fMinL == MAXINT) ? NAN : m_fMaxL - m_fMinL;
	m_fDeltaR = (m_fMaxR == MININT || m_fMinR == MAXINT) ? NAN : m_fMaxR - m_fMinR;
}


BOOL SSubTargetPic::IsInRect(int x, int y)
{
	CRect *prc;
	prc = &m_rcImage;
	if (x >= prc->left && x <= prc->right/*&&
		y >= prc->top  && y <= prc->bottom*/)
		return TRUE;
	return FALSE;
}

int SSubTargetPic::GetYPos(double fDiff, BOOL bIsRight)
{
	//int nWidth = m_nHeight / 8;
	double fPos;
	if (bIsRight)
		fPos = m_rcImage.top + ((m_fMaxR - fDiff) / m_fDeltaR) * m_nHeight + MARGIN;
	else
		fPos = m_rcImage.top + ((m_fMaxL - fDiff) / m_fDeltaL) * m_nHeight + MARGIN;

	int nPos = (int)fPos;
	return nPos;
}

SStringW SSubTargetPic::GetYPrice(int nY, BOOL bIsRight)
{
	SStringW strRet; strRet.Empty();
	if (bIsRight&&isnan(m_fDeltaR))
		return strRet;
	if (!bIsRight&&isnan(m_fDeltaL))
		return strRet;
	//int nWidth = m_nHeight / 8;
	int middle = m_rcImage.top + 20;
	int nDiff = nY - m_rcImage.top - MARGIN;
	double fDiff = nDiff;
	double fPrice;
	if (bIsRight)
		fPrice = m_fMaxR - fDiff / m_nHeight * m_fDeltaR;
	else
		fPrice = m_fMaxR - fDiff / m_nHeight * m_fDeltaR;
	strRet.Format(L"%.02f", fPrice);
	return strRet;
}


void SSubTargetPic::OnTimer(char cTimerID)
{
	if (cTimerID == 1)	//刷新鼠标
	{
	}
}

int SSubTargetPic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return 0;
}


int SSubTargetPic::GetXPos(int n) {	//获取id对应的x坐标
	if (!m_bUseWidth)
	{
		double fx = m_rcImage.left + ((n + 0.5)*(double)(m_rcImage.Width() - 2) / (double)m_nAllLineNum + 0.5);
		int nx = (int)fx;
		if (nx < m_rcImage.left || nx > m_rcImage.right)
			nx = m_rcImage.left;
		return nx;
	}
	else
	{
		int nPos = n *TOTALZOOMWIDTH + 1 + m_rcImage.left;
		nPos += ZOOMWIDTH / 2;
		return nPos;
	}
}

int SSubTargetPic::GetXData(int nx) {	//获取鼠标下的数据id
	if (!m_bUseWidth)
	{
		double fn = (double)(nx - m_rcImage.left) / ((double)(m_rcImage.Width() - 2) / (double)m_nAllLineNum) - 0.5;
		int n = (int)fn;
		if (n < 0 || n >= (int)m_pData[0]->size())
			n = -1;
		return n;
	}
	else
	{
		float fn = (float)(nx - m_rcImage.left) / (float)TOTALZOOMWIDTH;
		int n = (int)fn;
		if (n < 0)
			n = 0;
		n += m_nFirst;
		return n;
	}
}

void SSubTargetPic::DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color, UINT uFormat)
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

void SSubTargetPic::DrawEarserLine(IRenderTarget * pRT, CPoint pt, bool bVertical)
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




void SSubTargetPic::HandleMissData(InStockData f1, int time)//补全遗漏的数据
{
}



void SSubTargetPic::DrawMouseData(IRenderTarget * pRT, int xPos)
{
	pRT->FillRectangle(CRect(m_rcImage.left,
		m_rcImage.top,
		m_rcImage.right,
		m_rcImage.top + 19));

	SStringW sl;
	int left = m_rcImage.left + 5;
	int top = m_rcImage.top + 5;
	int bottom = m_rcImage.top + MARGIN;
	int right = m_rcImage.right;



	CoreData* pData = new CoreData[m_nShowDataCount];
	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		if (xPos >= 0 && xPos < m_pData[0]->size())
			pData[i] = m_pData[i]->at(xPos);
		else if (xPos == -1 && !m_pData[i]->empty())
			pData[i] = m_pData[i]->at(m_pData[i]->size() - 1);
	}
	//sl.Format(L"%4d-%02d-%02d %02d:%02d", pData[0].date / 10000,
	//	pData[0].date % 10000 / 100, pData[0].date % 100,
	//	pData[0].time / 100, pData[0].time % 100);

	DrawTextonPic(pRT, CRect(left, top, right, bottom), m_strPicName, m_colorVec[0]);
	if (xPos < 0 || xPos >= m_pData[0]->size())
		return;
	HDC hdc = pRT->GetDC();
	CSize size = { 0 };

	GetTextExtentPoint32(hdc, m_strPicName, m_strPicName.GetLength(), &size);
	left += size.cx;

	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		sl = StrA2StrW(m_dataNameVec[i]);
		sl.Format(L"%s:%.02f", sl, pData[i].value);
		DrawTextonPic(pRT, CRect(left, top, right, bottom), sl, m_colorVec[i]);
		GetTextExtentPoint32(hdc, sl, sl.GetLength(), &size);
		left += size.cx;
	}
	delete[]pData;
	pData = nullptr;
	pRT->ReleaseDC(hdc);
}


void SSubTargetPic::DrawMouse(IRenderTarget * pRT, CPoint po, BOOL bFromOnPaint)
{
	if (po.x == m_nMouseX && po.y == m_nMouseY)
		return;
	if (!IsInRect(po.x, po.y))
	{
		if (m_nMouseX != -1 || m_nMouseY != -1)
		{
			m_nMouseX = m_nMouseY = -1;
			//if (!bFromOnPaint)
			//{
			//	Invalidate();
			//}
		}
		return;
	}


	if (m_bShowMouseLine)
	{
		HDC hdc = pRT->GetDC();
		int  nMode = SetROP2(hdc, R2_NOTXORPEN);
		MoveToEx(hdc, m_nMouseX, m_rcImage.top + 20, NULL);
		LineTo(hdc, m_nMouseX, m_rcImage.bottom);
		//MoveToEx(hdc, m_rcImage.left, m_nMouseY, NULL);	
		//LineTo(hdc, m_rcImage.right, m_nMouseY);

		MoveToEx(hdc, po.x, m_rcImage.top + 20, NULL);
		LineTo(hdc, po.x, m_rcImage.bottom);
		//MoveToEx(hdc, m_rcImage.left, po.y, NULL);			
		//LineTo(hdc, m_rcImage.right, po.y);
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
		if (m_nMouseY >= m_rcImage.top && m_nMouseY <= m_rcImage.bottom)	//十字在k线上
		{
			//sl = GetYPrice(m_nMouseY, FALSE);
			sr = GetYPrice(m_nMouseY, TRUE);
		}
		if (!sr.IsEmpty())
			DrawTextonPic(pRT,
				CRect(m_rcImage.right + 1,
					m_nMouseY - 10,
					m_rcImage.right + RC_FSLEFT - 2,
					m_nMouseY + 10),
				sr);
		if (!sl.IsEmpty())
			DrawTextonPic(pRT,
				CRect(m_rcImage.left - RC_FSLEFT + 2,
					m_nMouseY - 10,
					m_rcImage.left - 1,
					m_nMouseY + 10),
				sl);




		sr.Empty(); sl.Empty();
		if (po.y >= m_rcImage.top && po.y <= m_rcImage.bottom)	//十字在k线上
		{
			//sl = GetYPrice(po.y, FALSE);
			sr = GetYPrice(po.y, TRUE);
		}

		if (!sr.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.right + 1, po.y - 10, m_rcImage.right + RC_FSLEFT - 2, po.y + 10), sr);
		if (!sl.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.left - RC_FSLEFT + 2, po.y - 10, m_rcImage.left - 1, po.y + 10), sl);

		//显示纵坐标数值
		int left = max(0, m_pData[0]->size() - m_nAllLineNum);
		int dataPosMouse = -1;
		if (IsInRect(m_nMouseX, m_nMouseY) && m_nMouseX <= m_rcImage.right)
		{
			int nx = GetXData(m_nMouseX);
			if (!m_bUseWidth)
				dataPosMouse = nx >= 0 ? nx + m_nFirst : -1;
			else
				dataPosMouse = nx >= 0 ? nx : -1;

		}
		int dataPosPoint = -1;
		if (IsInRect(po.x, po.y) && po.x <= m_rcImage.right)
		{
			int nx = GetXData(po.x);
			if (!m_bUseWidth)
				dataPosPoint = nx >= 0 ? nx + m_nFirst : -1;
			else
				dataPosPoint = nx >= 0 ? nx : -1;

		}
		//在左上角添加此时刻具体行情


		if (IsInRect(m_nMouseX, m_nMouseY) && m_bShowMouseLine)
		{
			if (m_nMouseX > m_rcImage.right)
				DrawMouseData(pRT, m_nEnd - 1);
			else
				DrawMouseData(pRT, dataPosMouse);
		}
		if (IsInRect(po.x, po.y) && m_bShowMouseLine)
		{
			if (po.x > m_rcImage.right)
				DrawMouseData(pRT, m_nEnd - 1);
			else
				DrawMouseData(pRT, dataPosPoint);
		}
	}
	m_nMouseX = po.x;
	m_nMouseY = po.y;

}


void SSubTargetPic::DrawData(IRenderTarget * pRT)
{
	GetMaxDiff();

	CPoint pts[5];
	int x = 0;
	CAutoRefPtr<IPen> penWhite, penYellow, oldPen,
		penRed, penBlue, penGreen, penPurple;
	CAutoRefPtr<IBrush> bBrush, bOldBrush;
	if (m_nAllLineNum == 0)
		return;
	int width = int(m_rcImage.Width() / m_nAllLineNum / 2 + 0.5);


	m_nHeight = m_rcImage.bottom - m_rcImage.top - 20;

	int nYoNum = m_nHeight / 20;
	if (nYoNum % 2 == 0)
		nYoNum--;
	if (nYoNum > 9)
		nYoNum = 9;
	if (nYoNum == 1)
		nYoNum = 3;

	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &penWhite);
	pRT->SelectObject(penWhite, (IRenderObj**)&oldPen);

	double nWidth = m_nHeight * 1.0 / (nYoNum - 1);
	for (int i = 0; i < nYoNum; i++)
	{
		double fY = m_rcImage.top + 20 + nWidth * i;
		int nY = round(fY);
		//k线区y轴加轴标
		SStringW s1 = GetYPrice(nY, FALSE);
		if (!s1.IsEmpty())
			DrawTextonPic(pRT,
				CRect(m_rcImage.left - RC_FSLEFT,
					nY - 9,
					m_rcImage.left,
					nY + 9), s1,
				RGBA(255, 0, 0, 255),
				DT_CENTER);
		//s1 = GetYPrice(nY, TRUE);
		//if (!s1.IsEmpty())
		//	DrawTextonPic(pRT,
		//		CRect(m_rcImage.right + 3,
		//			nY - 9, 
		//			m_rcImage.right
		//			+ RC_FSRIGHT, 
		//			nY + 9),
		//		s1, 
		//		RGBA(255, 0, 0, 255),
		//		DT_CENTER);
	}
	int nDataNum = m_bUseWidth ? m_nAllLineNum : m_nEnd - m_nFirst;
	vector<vector<CPoint>> LineVec(m_nShowDataCount, vector<CPoint>(nDataNum));
	int nLineStartPos = 0;
	if (!m_bUseWidth)
	{
		for (int i = 0; i < nDataNum; i++)
		{

			x = GetXPos(i);
			for (int j = 0; j < m_nShowDataCount; ++j)
				LineVec[j][i].SetPoint(x + width,
					GetYPos(m_pData[j]->at(i + m_nFirst).value, m_bRightArr[j]));
			//加最后的数值
		}
	}
	else
	{
		m_nEnd = min(m_pData[0]->size(), m_pData[1]->size()) - m_nOffset;
		if (m_nEnd <= 0)
			return;
		m_nFirst = m_nEnd - m_nAllLineNum;
		nLineStartPos = m_nFirst >= 0 ? 0 : 0 - m_nFirst;
		nDataNum = m_nAllLineNum - nLineStartPos;
		for (int i = 0; i < m_nAllLineNum; i++)
		{
			int nOffset = i + m_nFirst;
			if (nOffset < 0)
				continue;
			x = i * TOTALZOOMWIDTH + 1 + m_rcImage.left;
			for (int j = 0; j < m_nShowDataCount; ++j)
				LineVec[j][i].SetPoint(x + ZOOMWIDTH / 2,
					GetYPos(m_pData[j]->at(nOffset).value, m_bRightArr[j]));
			//加最后的数值
		}

	}


	if (!m_bShowMouseLine)
		DrawMouseData(pRT, m_nEnd - 1);

	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		pRT->SelectObject(m_penVec[i]);
		pRT->DrawLines(&LineVec[i][nLineStartPos], nDataNum);
	}

	pRT->SelectObject(oldPen);

}

void SSubTargetPic::OnDbClicked(UINT nFlags, CPoint point)
{
	m_bShowMouseLine = !m_bShowMouseLine;
}

void SSubTargetPic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_bDataInited)
		return;
	switch (nChar)
	{
	case VK_LEFT:
	{
		m_bKeyDown = true;

		if (m_bIsFirstKey)
		{
			if (m_bShowMouseLine)
				m_nNowPosition = GetXData(m_nMouseX);
			else
				m_nNowPosition = min(m_pData[0]->size() - 1, m_nAllLineNum - 1);
			m_bIsFirstKey = false;
			m_bShowMouseLine = true;
			return;
		}

		if (0 == m_nNowPosition)
		{
			int size = m_pData[0]->size();
			if (size - m_nOffset > m_nAllLineNum)
				++m_nOffset;
		}
		else
			m_nNowPosition--;
	}
	break;
	case VK_UP:
		if (m_nAllLineNum > MIN_LINE_NUM)
		{
			m_nAllLineNum /= ZOOMRATIO;
			m_nAllLineNum = max(m_nAllLineNum, MIN_LINE_NUM);
			if (m_bKeyDown)
			{
				m_nNowPosition /= ZOOMRATIO;
				m_nNowPosition = min(m_nNowPosition, m_nAllLineNum - 1);
			}
		}
		break;
	case VK_RIGHT:
	{
		m_bKeyDown = true;

		if (m_bIsFirstKey)
		{
			if (m_bShowMouseLine)
				m_nNowPosition = GetXData(m_nMouseX);
			else
				m_nNowPosition = min(m_pData[0]->size() - 1, m_nAllLineNum - 1);
			m_bIsFirstKey = false;
			m_bShowMouseLine = true;
			return;
		}

		int size = min(m_pData[0]->size(), m_nAllLineNum);
		if (size == m_nNowPosition + 1)
		{
			--m_nOffset;
			m_nOffset = max(0, m_nOffset);
		}
		else
			++m_nNowPosition;
	}
	break;
	case VK_DOWN:
		if (m_nAllLineNum < MAX_LINE_NUM)
		{
			m_nAllLineNum *= ZOOMRATIO;
			m_nAllLineNum = min(m_nAllLineNum, MAX_LINE_NUM);
			if (m_bKeyDown)
			{
				m_nNowPosition *= ZOOMRATIO;
				if (m_nNowPosition >= m_nAllLineNum)
					m_nNowPosition = m_nAllLineNum - 1;
			}

		}
		break;
	default:
		break;
	}
}

void SSubTargetPic::ReSetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec)
{
	m_bDataInited = false;
	if (m_pData)
	{
		delete[]m_pData;
		m_pData = nullptr;
	}

	m_nShowDataCount = nDataCount;
	m_pData = new vector<CoreData>*[nDataCount];
	m_bRightArr = bRightVec;
	for (int i = 0; i < nDataCount; ++i)
		m_pData[i] = data[i];

	m_bDataInited = true;

}


void SSubTargetPic::DrawKeyDownMouseLine(IRenderTarget * pRT, BOOL bDoubleFlash)
{
	m_bKeyDown = true;
	m_bShowMouseLine = true;
	int left = m_pData[0]->size() - m_nAllLineNum;
	left = max(left, 0);
	int dataPos = m_nNowPosition + left - m_nOffset;
	//画鼠标线

	CPoint nowPoint;
	nowPoint.x = GetXPos(m_nNowPosition);
	//nowPoint.y = GetYPos(data.value, m_bRightArr[0]);
	SStringA strInfo;
	HDC hdc = pRT->GetDC();


	int  nMode = SetROP2(hdc, R2_NOT);

	if (bDoubleFlash)
	{
		MoveToEx(hdc, m_nMouseX, m_rcImage.top + MARGIN, NULL);
		LineTo(hdc, m_nMouseX, m_rcImage.bottom);
	}

	MoveToEx(hdc, nowPoint.x, m_rcImage.top + MARGIN, NULL);
	LineTo(hdc, nowPoint.x, m_rcImage.bottom);



	SetROP2(hdc, nMode);
	pRT->ReleaseDC(hdc);


	//左上角显示行情
	DrawMouseData(pRT, dataPos);
	m_nMouseX = nowPoint.x;
}



