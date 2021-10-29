#include "stdafx.h"
#include "SFenShiPic.h"
#include <unordered_map>
#include <fstream>
#include<windows.h>
#include<map>
#include<math.h>
#include<vector>

using std::vector;
//extern bool g_bDataInited;

#define MARGIN 20
#define PER_CHAR_WIDTH 7
#define MAX_LINE 10

#define MIN_LINE_NUM 60
#define MAX_LINE_NUM 5000
#define DEFAULT_LINE_NUM 240
#define ZOOMRATIO 1.5

#define RIGHT_BLANK 50

SFenShiPic::SFenShiPic()
{
	m_nTickPre = 0;
	m_bFocusable = 1; //可以获取焦点
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

	m_bPaintInit = FALSE;

	m_fMaxL = MININT;
	m_fMinL = MAXINT;
	m_fMaxR = MININT;
	m_fMinR = MAXINT;
	m_fDeltaL = NAN;
	m_fDeltaR = NAN;

	m_nOffset = 0;
	m_nFirst = 0;
	//m_pRTBuffer = nullptr;
}



SFenShiPic::~SFenShiPic()
{
	delete[]m_pData;
}

void SFenShiPic::SetShowData(int nIndex, bool bGroup)
{
	m_nIndex = nIndex;
	m_bKeyDown = false;
	m_nNowPosition = 0;
	m_bIsFirstKey = true;
}

void SOUI::SFenShiPic::SetShowData(int nDataCount, vector<TimeLineData>* data[], vector<BOOL>& bRightVec ,SStringA StockID, SStringA StockName)
{
	if (m_pData)
	{
		delete[]m_pData;
		m_pData = nullptr;
	}

	m_nShowDataCount = nDataCount;
	m_pData = new vector<TimeLineData>*[nDataCount];
	m_bRightArr = bRightVec;
	m_StockID = StockID;
	m_StockName = StockName;
	for (int i = 0; i < nDataCount; ++i)
		m_pData[i] = data[i];
	m_bDataInited = true;
}






void SOUI::SFenShiPic::InitColorAndPen(IRenderTarget *pRT)
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

void SFenShiPic::OnPaint(IRenderTarget * pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	SWindow::GetClientRect(&m_rcImage);

	if (!m_bPaintInit)
	{
		m_bPaintInit = true;
		InitColorAndPen(pRT);
	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);

	m_rcImage.DeflateRect(RC_FSLEFT + 5, RC_FSTOP, RC_FSRIGHT + 10, RC_FSBOTTOM);


	m_nHeight = m_rcImage.bottom - m_rcImage.top - 20;

	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(192, 192, 192, 255), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		pts[0].SetPoint(m_rcImage.left - RC_FSLEFT + 1, m_rcImage.top - 25);
		pts[1].SetPoint(m_rcImage.left - RC_FSLEFT + 1, m_rcImage.bottom + 35);
		pts[2].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcImage.bottom + 35);
		pts[3].SetPoint(m_rcImage.right + RC_FSLEFT + 1, m_rcImage.top - 25);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);

		pRT->SelectObject(oldPen);
	}


	DrawArrow(pRT);


	if (m_bDataInited)
	{
		GetMaxDiff();
		DrawData(pRT);
	}


	//CPoint po(m_nMouseX, m_nMouseY);
	//m_nMouseX = m_nMouseY = -1;
	//LONGLONG llTmp3 = GetTickCount64();
	if (m_bKeyDown)
		DrawKeyDownMouseLine(pRT/*, 0*/);
	else
	{
		CPoint po(m_nMouseX, m_nMouseY);
		m_nMouseX = m_nMouseY = -1;
		DrawMouse(pRT, po, TRUE);
	}
	//	LOG_W(L"鼠标绘制时间:%I64d,总时间:%I64d\0", llTmp3 - llTmp2, llTmp3 - llTmp);
	AfterPaint(pRT, pa);
}

void SFenShiPic::DrawArrow(IRenderTarget * pRT)
{
	//	if (m_pData->d.empty())
	//		return;
	//画k线区
	int nLen = m_rcImage.bottom - m_rcImage.top;
	int nYoNum = 9;		//y轴标示数量 3 代表画两根线
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
	int width = m_nHeight / 8;
	for (int i = 0; i < nYoNum; i++)
	{
		int nY = m_rcImage.top + 20 + width * i;
		CPoint pts[2];
		{
			CAutoRefPtr<IPen> pen, oldPen;
			pts[0].SetPoint(m_rcImage.left, nY);
			pts[1].SetPoint(m_rcImage.right, nY);

			if (i == 4)
				pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
			else if (nY == m_rcImage.bottom)
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


}

void SFenShiPic::GetMaxDiff()		//判断坐标最大最小值和k线条数
{

	int nLen = m_rcImage.right - m_rcImage.left;	//判断是否超出范围
	//判断最大最小值
	//	OutputDebugString("判断最大值\0");
	m_nFirst = 0;
	m_nEnd = m_pData[0]->size();
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


BOOL SFenShiPic::IsInRect(int x, int y, int nMode)	//是否在坐标中,0为全部,1为上方,2为下方
{
	CRect *prc;
	switch (nMode)
	{
	case 0:
		prc = &m_rcImage;
		break;
	default:
		return FALSE;
	}
	if (x >= prc->left && x <= prc->right &&
		y >= prc->top  && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

int SFenShiPic::GetYPos(double fDiff, BOOL bIsRight)
{
	int nWidth = m_nHeight / 8;
	double fPos;
	if (bIsRight)
		fPos = m_rcImage.top + ((m_fMaxR - fDiff) / m_fDeltaR) * (nWidth * 8) + MARGIN;
	else
		fPos = m_rcImage.top + ((m_fMaxL - fDiff) / m_fDeltaL) * (nWidth * 8) + MARGIN;

	int nPos = (int)fPos;
	return nPos;
}

SStringW SFenShiPic::GetYPrice(int nY, BOOL bIsRight)
{
	SStringW strRet; strRet.Empty();
	if (bIsRight&&isnan(m_fDeltaR))
		return strRet;
	if (!bIsRight&&isnan(m_fDeltaL))
		return strRet;
	int nWidth = m_nHeight / 8;
	int middle = m_rcImage.top + 20;
	int nDiff = nY - m_rcImage.top - MARGIN;
	double fDiff = nDiff;
	double fPrice;
	if (bIsRight)
		fPrice = m_fMaxR - fDiff / (nWidth * 8) * m_fDeltaR;
	else
		fPrice = m_fMaxR - fDiff / (nWidth * 8) * m_fDeltaR;
	strRet.Format(L"%.02f", fPrice);
	return strRet;
}

void SFenShiPic::OnMouseMove(UINT nFlags, CPoint point)
{
	int nTick = GetTickCount();
	if (nTick - m_nTickPre < 10 && m_nTickPre > 0)
		return;
	m_nTickPre = nTick;

	if (point.x == m_nMouseX && point.y == m_nMouseY)
		return;


	m_bIsFirstKey = true;

	CPoint p = point;

	CRect rc = GetClientRect();
	CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
		Invalidate();
		return;
	}

	DrawMouse(pRT, point);
	AfterPaint(pRT, pa);
	ReleaseRenderTarget(pRT);
}

void SFenShiPic::OnTimer(char cTimerID)
{
	if (cTimerID == 1)	//刷新鼠标
	{
	}
}

int SFenShiPic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return 0;
}

void SOUI::SFenShiPic::OnMouseLeave()
{
	Invalidate();		//主要是为了消除未来得及消除的鼠标线
}

int SFenShiPic::GetXPos(int n) {	//获取id对应的x坐标
	double fx = m_rcImage.left + ((n + 0.5)*(double)(m_rcImage.Width()- RIGHT_BLANK - 2) / (double)m_nAllLineNum + 0.5);
	int nx = (int)fx;
	if (nx < m_rcImage.left || nx > m_rcImage.right)
		nx = m_rcImage.left;
	return nx;
}

int SFenShiPic::GetXData(int nx) {	//获取鼠标下的数据id
	double fn = (double)(nx - m_rcImage.left) / ((double)(m_rcImage.Width() - RIGHT_BLANK - 2) / (double)m_nAllLineNum) - 0.5;
	int n = (int)fn;
	if (n < 0 || n >= (int)m_pData[0]->size())
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

void SOUI::SFenShiPic::DrawEarserLine(IRenderTarget * pRT, CPoint pt, bool bVertical)
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




void SOUI::SFenShiPic::HandleMissData(InStockData f1, int time)//补全遗漏的数据
{
}

void SOUI::SFenShiPic::DrawTime(IRenderTarget * pRT, CRect rc, int date, int time)
{
	SStringW str;
	str.Format(L"%d-%02d-%02d      %02d:%02d", date / 10000, date % 10000 / 100, date % 100,
		time / 100, time % 100);
	DrawTextonPic(pRT, rc, str, RGBA(255, 255, 255, 255), 0);

}

void SOUI::SFenShiPic::DrawMouseData(IRenderTarget * pRT, int xPos)
{
	SStringW sl;
	int left = m_rcImage.left + 5;
	int top = m_rcImage.top + 5;
	int bottom = m_rcImage.top + MARGIN;
	int right = m_rcImage.right;
	sl.Format(L"%s %s", StrA2StrW(m_StockID), StrA2StrW(m_StockName));
	DrawTextonPic(pRT, CRect(left, top, right, bottom), sl, RGBA(255, 255, 0, 255));
	left += sl.GetLength()*PER_CHAR_WIDTH *1.5;

	if (xPos < 0 || xPos>=m_nAllLineNum)
		return;
	TimeLineData* pData = new TimeLineData[m_nShowDataCount];
	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		if (xPos >= 0 && xPos < m_pData[0]->size())
			pData[i] = m_pData[i]->at(xPos);
		else if (xPos == -1 && !m_pData[i]->empty())
			pData[i] = m_pData[i]->at(m_pData[i]->size() - 1);
	}
	sl.Format(L"%d-%02d-%02d %02d:%02d", pData[0].date / 10000, pData[0].date % 10000 / 100, pData[0].date % 100,
		pData[0].time / 100, pData[0].time % 100);
	DrawTextonPic(pRT, CRect(left, top, right, bottom), sl, RGBA(255, 255, 0, 255));
	left += 110;
	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		sl = StrA2StrW(pData[i].dataName);
		DrawTextonPic(pRT, CRect(left, top, right, bottom), sl, m_colorVec[i]);
		left += sl.GetLength()*PER_CHAR_WIDTH;
		sl.Format(L"%.2f", pData[i].value);
		DrawTextonPic(pRT, CRect(left, top, right, bottom), sl, m_colorVec[i]);
		left += sl.GetLength()*PER_CHAR_WIDTH;
	}
	delete[]pData;
	pData = nullptr;
}


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
		MoveToEx(hdc, m_nMouseX, m_rcImage.top + 20, NULL);	LineTo(hdc, m_nMouseX, m_rcImage.bottom);
		MoveToEx(hdc, m_rcImage.left, m_nMouseY, NULL);	LineTo(hdc, m_rcImage.right, m_nMouseY);

		MoveToEx(hdc, po.x, m_rcImage.top + 20, NULL);			LineTo(hdc, po.x, m_rcImage.bottom);
		MoveToEx(hdc, m_rcImage.left, po.y, NULL);			LineTo(hdc, m_rcImage.right, po.y);
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
			sl = GetYPrice(m_nMouseY, FALSE);
			sr = GetYPrice(m_nMouseY, TRUE);
		}
		if (!sr.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.right + 1, m_nMouseY - 10, m_rcImage.right + RC_FSLEFT - 2, m_nMouseY + 10), sr);
		if (!sl.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.left - RC_FSLEFT + 2, m_nMouseY - 10, m_rcImage.left - 1, m_nMouseY + 10), sl);




		sr.Empty(); sl.Empty();
		if (po.y >= m_rcImage.top && po.y <= m_rcImage.bottom)	//十字在k线上
		{
			sl = GetYPrice(po.y, FALSE);
			sr = GetYPrice(po.y, TRUE);
		}

		if (!sr.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.right + 1, po.y - 10, m_rcImage.right + RC_FSLEFT - 2, po.y + 10), sr);
		if (!sl.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.left - RC_FSLEFT + 2, po.y - 10, m_rcImage.left - 1, po.y + 10), sl);

		//显示纵坐标数值
		int left = max(0, m_pData[0]->size() - m_nAllLineNum);
		int dataPosMouse = -1;
		if (IsInRect(m_nMouseX, m_nMouseY, 0))
		{
			int nx = GetXData(m_nMouseX);
			dataPosMouse = nx >= 0 ? nx + m_nFirst : -1;
			if (dataPosMouse >= 0 && dataPosMouse < m_pData[0]->size())
			{
				const TimeLineData &data = m_pData[0]->at(dataPosMouse);
				DrawTime(pRT, CRect(m_nMouseX - 35, m_rcImage.bottom + 2, m_nMouseX + 35, m_rcImage.bottom + 40), data.date, data.time);
			}

		}
		int dataPosPoint = -1;
		if (IsInRect(po.x, po.y, 0))
		{
			int nx = GetXData(po.x);
			dataPosPoint = nx >= 0 ? nx + m_nFirst : -1;
			if (dataPosPoint >= 0 && dataPosPoint < m_pData[0]->size())
			{
				const TimeLineData &data = m_pData[0]->at(dataPosPoint);
				DrawTime(pRT, CRect(po.x - 35, m_rcImage.bottom + 2, po.x + 35, m_rcImage.bottom + 40), data.date, data.time);
			}

		}
		//在左上角添加此时刻具体行情


		if (IsInRect(m_nMouseX, m_nMouseY, 0) && m_bShowMouseLine)
			DrawMouseData(pRT, dataPosMouse);
		if (IsInRect(po.x, po.y, 0) && m_bShowMouseLine)
			DrawMouseData(pRT, dataPosPoint);
	}
	m_nMouseX = po.x;
	m_nMouseY = po.y;

}


void SFenShiPic::DrawData(IRenderTarget * pRT)
{
	CPoint pts[5];
	int x = 0;
	CAutoRefPtr<IPen> penWhite, penYellow, oldPen, penRed, penBlue, penGreen, penPurple;
	CAutoRefPtr<IBrush> bBrush, bOldBrush;
	int width = int((m_rcImage.Width() - RIGHT_BLANK) / m_nAllLineNum / 2 + 0.5);


	int nYoNum = 9;		//y轴标示数量 3 代表画两根线

	pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &penWhite);
	pRT->SelectObject(penWhite, (IRenderObj**)&oldPen);


	//k线区加坐标
	int nWidth = m_nHeight / 8;
	for (int i = 0; i < nYoNum; i++)
	{
		int nY = m_rcImage.top + 20 + nWidth * i;

		//k线区y轴加轴标
		SStringW s1 = GetYPrice(nY, FALSE);
		if (!s1.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.left - RC_FSLEFT, nY - 9, m_rcImage.left, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
		s1 = GetYPrice(nY, TRUE);
		if (!s1.IsEmpty())
			DrawTextonPic(pRT, CRect(m_rcImage.right + 3, nY - 9, m_rcImage.right + RC_FSRIGHT, nY + 9), s1, RGBA(255, 0, 0, 255), DT_CENTER);
	}


	int nDataNum = m_nEnd - m_nFirst;
	//int nEnd = m_pData[0]->size() - m_nOffset;
	//int nStart = nEnd - nDataNum;
	vector<vector<CPoint>> LineVec(m_nShowDataCount, vector<CPoint>(nDataNum));
	for (int i = 0; i < nDataNum; i++)
	{

		x = GetXPos(i);
		for (int j = 0; j < m_nShowDataCount; ++j)
			LineVec[j][i].SetPoint(x + width, GetYPos(m_pData[j]->at(i + m_nFirst).value, m_bRightArr[j]));
		//加最后的数值
	}

	if (!m_bShowMouseLine)
		DrawMouseData(pRT, m_nEnd - 1);

	for (int i = 0; i < m_nShowDataCount; ++i)
	{
		pRT->SelectObject(m_penVec[i]);
		pRT->DrawLines(&LineVec[i][0], nDataNum);
	}

	pRT->SelectObject(oldPen);

}

void SFenShiPic::OnDbClickedFenshi(UINT nFlags, CPoint point)
{
	m_bShowMouseLine = !m_bShowMouseLine;
	Invalidate();
}

void SFenShiPic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
			Invalidate();
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
		Invalidate();
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
				m_nNowPosition = min(m_nNowPosition, m_nAllLineNum -1);
			}
			Invalidate();
		}
		break;
	case VK_RIGHT:
	{
		m_bKeyDown = true;

		if (m_bIsFirstKey)
		{
			if(m_bShowMouseLine)
				m_nNowPosition =GetXData(m_nMouseX);
			else
				m_nNowPosition = min(m_pData[0]->size() - 1, m_nAllLineNum - 1);
			m_bIsFirstKey = false;
			m_bShowMouseLine = true;
			Invalidate();
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
		Invalidate();
	}
	break;
	case VK_DOWN:
		if (m_nAllLineNum < MAX_LINE_NUM&&m_nAllLineNum < m_pData[0]->size())
		{
			m_nAllLineNum *= ZOOMRATIO;
			m_nAllLineNum = min(m_nAllLineNum, MAX_LINE_NUM);
			//if(m_nAllLineNum > DEFAULT_LINE_NUM*5)
			//	m_nAllLineNum = min(m_nAllLineNum, m_pData[0]->size());
			if (m_bKeyDown)
			{
				m_nNowPosition *= ZOOMRATIO;
				if (m_nNowPosition >= m_nAllLineNum)
					m_nNowPosition = m_nAllLineNum - 1;
			}

			Invalidate();
		}
		break;
	default:
		break;
	}
}

void SOUI::SFenShiPic::DataInit()
{
	m_fMaxL = MININT;
	m_fMinL = MAXINT;
	m_fMaxR = MININT;
	m_fMinR = MAXINT;
	m_fDeltaL = NAN;
	m_fDeltaR = NAN;
}

void SFenShiPic::DrawKeyDownMouseLine(IRenderTarget * pRT/*, UINT nChar*/)
{

	int left = m_pData[0]->size() - m_nAllLineNum;
	left = max(left , 0);
	int dataPos = m_nNowPosition + left - m_nOffset;

	//画鼠标线
	const TimeLineData &data = m_pData[0]->at(dataPos);
	CPoint nowPoint;
	nowPoint.x = GetXPos(m_nNowPosition);
	nowPoint.y = GetYPos(data.value, m_bRightArr[0]);

	HDC hdc = pRT->GetDC();
	CRect rcClient;
	GetClientRect(rcClient);
	int  nMode = SetROP2(hdc, R2_NOT);
	MoveToEx(hdc, nowPoint.x, m_rcImage.top + MARGIN, NULL);			LineTo(hdc, nowPoint.x, m_rcImage.bottom);
	MoveToEx(hdc, m_rcImage.left, nowPoint.y, NULL);			LineTo(hdc, m_rcImage.right, nowPoint.y);
	SetROP2(hdc, nMode);
	pRT->ReleaseDC(hdc);

	//显示横坐标轴数值

	SStringW sl, sr;
	sl = GetYPrice(nowPoint.y, FALSE);
	sr = GetYPrice(nowPoint.y, TRUE);
	if (!sl.IsEmpty())
		DrawTextonPic(pRT, CRect(m_rcImage.left - RC_FSLEFT + 2, nowPoint.y + 2, m_rcImage.left - 1, nowPoint.y + 22), sl);
	if (!sr.IsEmpty())
		DrawTextonPic(pRT, CRect(m_rcImage.right + 1, nowPoint.y + 2, m_rcImage.right + RC_FSRIGHT, nowPoint.y + 22), sr);

	DrawTime(pRT, CRect(nowPoint.x - 35, m_rcImage.bottom + 2, nowPoint.x + 35, m_rcImage.bottom + 40), data.date, data.time);


	//左上角显示行情
	DrawMouseData(pRT, dataPos);
}


