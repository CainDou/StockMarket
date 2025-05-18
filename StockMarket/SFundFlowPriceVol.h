#pragma once

#include <mutex>
namespace SOUI
{
	enum eShowTimeType
	{
		eSTT_Last,
		eSTT_AllTime,
	};

	enum ePriceVolType
	{
		ePVT_Order = 1,
		ePVT_Delete = 2,
		ePVT_Trade = 4,
	};

	class SFundFlowPriceVol : public SWindow
	{
		typedef struct _ShowPV
		{
			int64_t nVolInfo[7];
			double fBuyRatio;
		}ShowPV;

		SOUI_CLASS_NAME(SFundFlowPriceVol, L"FundFlowPriVolPic")	//∂®“Âxml±Í«©
	public:
		SFundFlowPriceVol();
		~SFundFlowPriceVol();
		void	UpdateMarket(const CommonStockMarket& market);
		void	ChangeOffset(bool bAdd);
		void	ClearData();
		void	UpdateOrderPriVolData(OrderVolState* pData,int nDataCount);
		void	UpdateDeletePriVolData(DeleteVolState* pData, int nDataCount);
		void	UpdateTradePriVolData(TradeVolState* pData, int nDataCount);
		void	ChangeShowStock(SStringA subIns, SStringA StockName);
		void	ChangeShowTime(bool bAllTime);
		void	ChangeShowType(int nType, bool bShow);
		void	OnPaint(IRenderTarget *pRT);
		
	protected:
		void		DrawPriceVol(IRenderTarget * pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawHead1(IRenderTarget * pRT);
		void		DrawHead2(IRenderTarget * pRT);
		void		DrawSingleData(IRenderTarget * pRT, int nPrice,CRect& rc, BOOL bNowPrice);
		void		DrawDetailData(IRenderTarget * pRT, const ShowPV& data, CRect& rc, SStringW& strType);
		int			CalcShowData();
		int			CalcOrderShowPv(int id);
		int			CalcDeleteShowPv(int id);
		int			CalcTradeShowPv(int id);
		void		SetMaxPaintData();
		int			GetID(int nTime);
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
		int				m_nShowPriceType;
		int				m_nShowTimeType;
		CommonStockMarket   m_StockTick;
		vector<map<int, OrderVolState>> m_OrderPriceVolVec;
		vector<map<int, DeleteVolState>> m_DeletePriceVolVec;
		vector<map<int, TradeVolState>> m_TradePriceVolVec;
		vector<map<int, ShowPV>> m_ShowDataVec;
		set<int>m_ShowPriceSet;
		CAutoRefPtr<IFont> m_pFont15, m_pFont20, m_pFont10;
		CAutoRefPtr<IPen> m_penDotGray, m_penGray;
		BOOL			m_bInit;
		int64_t			m_MaxVol;
		CRect			m_rc;
		int				m_nOffset;
		std::mutex      m_mx;
		int				m_nSetOffset;
	};

	inline void SFundFlowPriceVol::UpdateMarket(const CommonStockMarket & market)
	{
		m_StockTick = market;
	}

	inline void SFundFlowPriceVol::ChangeOffset(bool bAdd)
	{
		if (bAdd)
			m_nSetOffset += 3;
		else
			m_nSetOffset-=3;
		Invalidate();
	}

	inline void SFundFlowPriceVol::ChangeShowStock(SStringA subIns, SStringA StockName)
	{
		m_strStockName = StrA2StrW(StockName);
		m_strStock = StrA2StrW(subIns);
		m_nSetOffset = 0;
	}
	inline void SFundFlowPriceVol::ChangeShowTime(bool bAllTime)
	{
		if (bAllTime) m_nShowTimeType = eSTT_AllTime;
		else m_nShowTimeType = eSTT_Last;
		Invalidate();
	}
	inline void SFundFlowPriceVol::ChangeShowType(int nType, bool bShow)
	{
		if (bShow)
			m_nShowPriceType |= nType;
		else
			m_nShowPriceType &= (~nType);
		Invalidate();
	}
}
