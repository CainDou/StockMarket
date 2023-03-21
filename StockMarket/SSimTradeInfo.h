#pragma once
namespace SOUI
{
	class SSimTradeInfo:public SWindow
	{
		SOUI_CLASS_NAME(SButton, L"tradeinfo")
	public:
		SSimTradeInfo();
		~SSimTradeInfo();
		void SetShowData(SStringA StockID, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec);
	protected:
		void OnPaint(IRenderTarget *pRT);
		void OnLButtonDown(UINT nFlags, CPoint pt);
		void OnDestroy();
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnSize(UINT nType, CSize size);
		void DrawPrice(IRenderTarget * pRT);
		void DrawDeal(IRenderTarget * pRT);
		COLORREF GetPriceColor(double fPrice);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_LBUTTONDOWN(OnLButtonDown) //将双击消息处理为单击
			MSG_WM_LBUTTONDBLCLK(OnLButtonDown) //将双击消息处理为单击
			MSG_WM_RBUTTONDOWN(OnLButtonDown) //将双击消息处理为单击
			MSG_WM_RBUTTONDBLCLK(OnLButtonDown) //将双击消息处理为单击
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			SOUI_MSG_MAP_END()

	protected:
		BOOL		m_bInit;
		BOOL		m_bDataInited;
		SStringA	m_strStockID;
		SStringA	m_strStockName;
		CRect		m_rectPrice;
		CRect		m_rectDeal;
		CommonStockMarket   m_StockTick;
		CommonStockMarket   m_preStockTick;
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonStockMarket> m_DealMarketVec;
		double		m_fMaxChgPct;
		CAutoRefPtr<IFont> m_pFont15, m_pFont20, m_pFont10;
		CAutoRefPtr<IPen> m_penRed;
		BOOL		m_bClickPrice;
		int			m_nOffset;
		BOOL		m_bUpdateDeal;

	};

}

