#pragma once

//一些基本的界面框架信息

#define RC_PRH			20
#define MAX_LIST_NUM	300

namespace SOUI
{
	class CPriceList 
	{

	public:
		CPriceList();
//		void SetShowData(int nIndex, bool bGroup);
		void		SetShowData(SStringA StockID, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec);
		void		SetShowData(SStringA StockID, SStringA StockName, vector<CommonIndexMarket>* pIdxMarketVec);
		void		SetIndyName(vector<SStringA>& nameVec);
		~CPriceList();

		SStringA		m_strSubIns;
		SStringA		m_strStockName;
		SStringA		m_strL1Indy;
		SStringA		m_strL2Indy;
		int				m_nWorkMode;			//1:1档行情,2:5档行情
		float			m_fPreSettle;
		bool			m_bIsStockIndex;
		CommonIndexMarket	m_IndexTick;
		CommonStockMarket   m_StockTick;
		CommonIndexMarket	m_preIndexTick;
		CommonStockMarket   m_preStockTick;

		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;
		double m_fMaxChgPct;
		
		//	double			m_fPreClose;			//前收
		int				m_nListNum;				//数据数量

		BOOL			m_bInsInited;

		BOOL			m_bInit;

		CRect			m_rect;

		CAutoRefPtr<IFont> m_pFont15, m_pFont20,m_pFont10;
		CAutoRefPtr<IPen> m_penRed;
		void		Paint(IRenderTarget *pRT);

	protected:
		void		DrawStockModeOne(IRenderTarget * pRT);
		void		DrawIndexModeOne(IRenderTarget * pRT);
		COLORREF	GetTextColor(double price);
	};
}

