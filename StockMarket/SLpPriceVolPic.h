#pragma once

#define HEADHEIGHT 20

#define PERTOTALHEIGHT 44

namespace SOUI
{
	class SLpPriceVolPic : public SWindow
	{
		SOUI_CLASS_NAME(SLpPriceVolPic, L"lpPiceVolPic")	//∂®“Âxml±Í«©

	public:
		SLpPriceVolPic();
		~SLpPriceVolPic();
		void	UpdateMarket(const CommonStockMarket& market);
		void	UpdateData(map<int, PeriodPriceVolInfo>& PriceVolMap,int nMaxPrice,int nMinPrice,double fMaxVol);
		void	UpdateCmpData(map<int, PeriodPriceVolInfo>& PriceVolMap);
		void	SetCalcPara(int nDataType, int nDataLevel, double fBasePrice);
		void	UpdateDataRange(int nMaxPrice, int nMinPrice, double fMaxVol);
		void	ChangeShowStock(SStringA subIns, SStringA StockName);
		void	OnPaint(IRenderTarget *pRT);
		BOOL	IsDataInited();
		BOOL	IsShowParaFit(int nDataType, int nDataLevel, double fBasePrice);
		void	ClearShowData();
	protected:
		void		DrawPriceVol(IRenderTarget * pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawHead1(IRenderTarget * pRT);
		void		DrawHead2(IRenderTarget * pRT);
		void		DrawSingleData(IRenderTarget * pRT, PeriodPriceVolInfo &info,  CRect& rc, BOOL bNowPrice);
		void		DrawCmpSingleData(IRenderTarget * pRT, PeriodPriceVolInfo &info, CRect& rc, BOOL bNowPrice);
		void		SetMaxPaintData();
		COLORREF	GetTextColor(double price);
		SStringW    GetVolShowText(double  fVolRatio);

	protected:
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_CREATE(OnCreate)
			SOUI_MSG_MAP_END()
	protected:
		map<int, PeriodPriceVolInfo> m_ShowDataMap;
		map<int, PeriodPriceVolInfo> m_ShowCmpDataMap;

		SStringW		m_strStock;
		SStringW		m_strStockName;
		float			m_fPreSettle;
		CommonStockMarket   m_StockTick;
		CAutoRefPtr<IFont> m_pFont15, m_pFont20, m_pFont10;
		CAutoRefPtr<IPen> m_penDotGray, m_penGray;
		BOOL			m_bInit;
		double			m_MaxVol;
		CRect			m_rc;
		int				m_nOffset;
		int				m_nDataType;
		int				m_nDataLevel;
		double			m_fBasePrice;
		int				m_nMaxPrice;
		int				m_nMinPrice;
		BOOL			m_bDataInited;
	};


	inline void SLpPriceVolPic::UpdateMarket(const CommonStockMarket & market)
	{
		m_StockTick = market;
	}

	inline void SOUI::SLpPriceVolPic::UpdateData(map<int, PeriodPriceVolInfo>& PriceVolMap, int nMaxPrice, int nMinPrice, double fMaxVol)
	{
		m_ShowDataMap = PriceVolMap;
		m_nMaxPrice = nMaxPrice;
		m_nMinPrice = nMinPrice;
		m_MaxVol = fMaxVol;
		m_bDataInited = TRUE;
		m_ShowCmpDataMap.clear();
	}

	inline void SLpPriceVolPic::UpdateCmpData(map<int, PeriodPriceVolInfo>& PriceVolMap)
	{
		m_ShowCmpDataMap = PriceVolMap;

	}

	inline void SLpPriceVolPic::SetCalcPara(int nDataType, int nDataLevel, double fBasePrice)
	{
		m_nDataType = nDataType;
		m_nDataLevel = nDataLevel;
		m_fBasePrice = fBasePrice;
	}

	inline void SLpPriceVolPic::UpdateDataRange(int nMaxPrice, int nMinPrice,double fMaxVol)
	{
		m_nMaxPrice = nMaxPrice;
		m_nMinPrice = nMinPrice;
		m_MaxVol = fMaxVol;
	}

	inline void SLpPriceVolPic::ChangeShowStock(SStringA subIns, SStringA StockName)
	{
		m_strStockName = StrA2StrW(StockName);
		m_strStock = StrA2StrW(subIns);
		m_ShowDataMap.clear();
		m_bDataInited = FALSE;
	}

	inline BOOL SOUI::SLpPriceVolPic::IsDataInited()
	{
		return m_bDataInited;
	}
	inline BOOL SLpPriceVolPic::IsShowParaFit(int nDataType, int nDataLevel, double fBasePrice)
	{
		if (m_nDataType != nDataType) return FALSE;
		if (m_nDataLevel != m_nDataLevel) return FALSE;
		if (m_fBasePrice != fBasePrice) return FALSE;
		return TRUE;
	}
	inline void SLpPriceVolPic::ClearShowData()
	{
		m_ShowDataMap.clear();
	}
}

