#pragma once

namespace SOUI
{
	class STradeInfoPic: public SWindow
	{

		SOUI_CLASS_NAME(STradeInfoPic, L"tradeInfo")	//定义xml标签

	public:
		STradeInfoPic();
		~STradeInfoPic();
		void	ChangeShowStock(SStringA StockID, SStringA StockName);
		void	SetDataPoint(vector<CommonStockMarket>* pStkMarketVec);
		void	OnPaint(IRenderTarget *pRT);
	protected:
		void		DrawPrice(IRenderTarget * pRT);
		void		DrawDeal(IRenderTarget * pRT);		//画单个合约
		COLORREF	GetTextColor(double price);
	protected:
		SOUI_MSG_MAP_BEGIN()
			//MESSAGE_HANDLER_EX(WM_FENSHI_MSG, OnMsg)
			//COMMAND_RANGE_HANDLER_EX(FM_Return,FM_End,OnMenuCmd)
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			//MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_CREATE(OnCreate)
			//MSG_WM_RBUTTONUP(OnRButtonUp)
			SOUI_MSG_MAP_END()

	protected:
		SStringA		m_strStock;
		SStringA		m_strStockName;
		float			m_fPreSettle;
		CommonStockMarket   m_StockTick;
		CommonStockMarket   m_preStockTick;
		vector<CommonStockMarket> *m_pStkMarketVec;
		double m_fMaxChgPct;
		BOOL			m_bInsInited;
		BOOL			m_bInit;
		CRect			m_rectPrice;
		CRect			m_rectDeal;

		CAutoRefPtr<IFont> m_pFont15, m_pFont20, m_pFont10;
		CAutoRefPtr<IPen> m_penRed;

	};

}

