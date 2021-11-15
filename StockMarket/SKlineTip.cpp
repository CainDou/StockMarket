#include "stdafx.h"
#include "SKlineTip.h"


namespace SOUI
{

#define TIMERID_DELAY    1
#define TIMERID_SPAN     2

#define MARGIN_TIP        5

	SKlineTip::SKlineTip(HWND hParWnd) :m_nDelay(100), m_nShowSpan(5000), m_font(0)
	{
		m_hParWnd = hParWnd;
	}

	SKlineTip::~SKlineTip()
	{
		if (m_font) DeleteObject(m_font);
	}

	BOOL SKlineTip::Create()
	{
		HWND hWnd = CSimpleWnd::Create(_T("klinetip"), WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, 0, 0, 0, 0, NULL, NULL);
		if (!hWnd) return FALSE;

		LOGFONT lf;
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
		lf.lfHeight = -15;
		_tcscpy(lf.lfFaceName, _T("微软雅黑"));
		m_font = CreateFontIndirect(&lf);

		return TRUE;
	}

	void SKlineTip::RelayEvent(CPoint pt)
	{
			if (!m_rcTarget.PtInRect(pt))
			{
				OnTimer(TIMERID_SPAN);//hide tip
			}
			else if (!IsWindowVisible() && !m_strTip.IsEmpty())
			{
				KillTimer(TIMERID_DELAY);
				SetTimer(TIMERID_DELAY, m_nDelay);
				::ClientToScreen(m_hParWnd, &pt);
				SetWindowPos(0, pt.x+15, pt.y+20, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
			}
	}

	void SKlineTip::UpdateTip( CRect rc, LPCTSTR pszTip, int nScale)
	{

		m_rcTarget = rc;
		m_strTip = pszTip;
		if (m_nScale != nScale)
		{
			if (m_font) DeleteObject(m_font);
			LOGFONT lf;
			GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
			lf.lfHeight = -15 * nScale / 100;
			_tcscpy(lf.lfFaceName, _T("微软雅黑"));
			m_font = CreateFontIndirect(&lf);
			m_nScale = nScale;
		}
		m_strTip.Replace(_T("\\n"), _T("\n"));

		if (IsWindowVisible())
		{
			ShowTip(TRUE);
		}
	}

	void SKlineTip::ClearTip()
	{
		ShowTip(FALSE);
	}


	void SKlineTip::ShowTip(BOOL bShow)
	{
		if (!bShow)
		{
			ShowWindow(SW_HIDE);
			m_rcTarget.SetRect(0, 0, 0, 0);
			m_strTip = _T("");
		}
		else if (!m_strTip.IsEmpty())
		{
			HDC hdc = ::GetDC(NULL);
			CRect rcText(0, 0, 500, 1000);
			HFONT oldFont = (HFONT)SelectObject(hdc, m_font);
			DrawText(hdc, m_strTip, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
			SelectObject(hdc, oldFont);
			::ReleaseDC(NULL, hdc);
			CRect rcWnd;
			GetWindowRect(&rcWnd);

			//增加对多显示器的支持			
			HMONITOR hMonitor;
			MONITORINFO mi;
			hMonitor = MonitorFromRect(&rcWnd, MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			int cx = mi.rcMonitor.right;
			int cy = mi.rcMonitor.bottom;
			/*增加部分结束*/


			rcWnd.right = rcWnd.left + rcText.right + 2 * MARGIN_TIP;
			rcWnd.bottom = rcWnd.top + rcText.bottom + 2 * MARGIN_TIP;
			//去掉下面这两行
			if (rcWnd.right>cx) rcWnd.OffsetRect(cx - rcWnd.right, 0);
			if (rcWnd.bottom>cy) rcWnd.OffsetRect(0, cy - rcWnd.bottom);
			SetWindowPos(HWND_TOPMOST, rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		}
	}

	void SKlineTip::OnTimer(UINT_PTR idEvent)
	{
		switch (idEvent)
		{
		case TIMERID_DELAY:
			KillTimer(TIMERID_DELAY);
			ShowTip(TRUE);
			SetTimer(TIMERID_SPAN, m_nShowSpan);
			break;
		case TIMERID_SPAN:
			ShowTip(FALSE);
			KillTimer(TIMERID_SPAN);
			break;
		}
	}

	void SKlineTip::OnPaint(HDC dc)
	{
		PAINTSTRUCT ps;
		dc = ::BeginPaint(m_hWnd, &ps);

		CRect rc;
		GetClientRect(&rc);
		HBRUSH br = CreateSolidBrush(RGBA(220,220,200,200));
		HGDIOBJ hOld = SelectObject(dc, br);
		Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(dc, hOld);
		DeleteObject(br);

		rc.DeflateRect(MARGIN_TIP, MARGIN_TIP);
		SetBkMode(dc, TRANSPARENT);
		HGDIOBJ hOldFont = SelectObject(dc, m_font);
		::DrawText(dc, m_strTip, -1, &rc, DT_WORDBREAK);
		SelectObject(dc, hOldFont);

		::EndPaint(m_hWnd, &ps);
	}


	void SKlineTip::OnFinalMessage(HWND hWnd)
	{
		CSimpleWnd::OnFinalMessage(hWnd);
		delete this;
	}
}//namespace SOUI