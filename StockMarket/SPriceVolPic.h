#pragma once
namespace SOUI
{
	class SPriceVolPic : public SWindow
	{

		SOUI_CLASS_NAME(SPriceVolPic, L"priceVolPic")	//∂®“Âxml±Í«©

	public:
		SPriceVolPic();
		~SPriceVolPic();
		void	UpdateMarket(const CommonStockMarket& market);
		void	UpdateData(map<int, PriceVolInfo>& PriceVolMap);
		void	ChangeShowStock(SStringA subIns, SStringA StockName);
		void	OnPaint(IRenderTarget *pRT);
	protected:
		void		DrawPriceVol(IRenderTarget * pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawHead1(IRenderTarget * pRT);
		void		DrawHead2(IRenderTarget * pRT);
		void		DrawSingleData(IRenderTarget * pRT, PriceVolInfo &info, CRect& rc,BOOL bNowPrice);
		void		SetMaxPaintData();
		COLORREF	GetTextColor(double price);
		SStringW    GetVolShowText(int nVol);
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
		
		SStringW		m_strStock;
		SStringW		m_strStockName;
		float			m_fPreSettle;
		CommonStockMarket   m_StockTick;
		map<int, PriceVolInfo> m_PriceVolMap;
		CAutoRefPtr<IFont> m_pFont15, m_pFont20, m_pFont10;
		CAutoRefPtr<IPen> m_penDotGray,m_penGray;
		BOOL			m_bInit;
		int64_t			m_MaxVol;
		CRect			m_rc;
		int				m_nOffset;
		SCriticalSection m_cs;
	};

	inline void SPriceVolPic::UpdateMarket(const CommonStockMarket & market)
	{
		m_StockTick = market;
	}

	inline void SOUI::SPriceVolPic::UpdateData(map<int, PriceVolInfo>& PriceVolMap)
	{
		m_cs.Enter();
		m_PriceVolMap.swap(PriceVolMap);
		m_cs.Leave();
	}
	inline void SPriceVolPic::ChangeShowStock(SStringA subIns, SStringA StockName)
	{
		m_strStockName = StrA2StrW(StockName);
		m_strStock = StrA2StrW(subIns);
	}
}
