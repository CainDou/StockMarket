#pragma once
#include <core\Swnd.h>
namespace SOUI
{
	class SKlineTip :
		public CSimpleWnd
	{
	public:
		SKlineTip(HWND hParWnd);
		~SKlineTip();

		BOOL Create();

	public://IToolTip
		/*virtual*/ void RelayEvent(CPoint pt);

		/*virtual*/ void UpdateTip( CRect rc, LPCTSTR pszTip, int nScale);

		/*virtual*/ void ClearTip();

		//void RelayEvent(const MSG *pMsg);
		//BOOL PreTranslateMessage(MSG* pMsg);
	protected:
		virtual void OnFinalMessage(HWND hWnd);

		void OnTimer(UINT_PTR idEvent);
		void OnPaint(HDC dc);
		void ShowTip(BOOL bShow);

		BEGIN_MSG_MAP_EX(SKlineTip)
			MSG_WM_PAINT(OnPaint)
			MSG_WM_TIMER(OnTimer)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		int            m_nDelay;
		int            m_nShowSpan;
		SStringT       m_strTip;
		int            m_nScale;
		CRect          m_rcTarget;
		HFONT          m_font;

		HWND		   m_hParWnd;
	};

}