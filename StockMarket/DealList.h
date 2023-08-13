#pragma once

//一些基本的界面框架信息

#define RC_HEIGHT		20
#define MAX_LIST_NUM	300

namespace SOUI
{
	class CDealList 
	{

	public:
		CDealList();
		~CDealList();

		int						m_nComboSelect;	//显示那个数据
												//	double			m_fPreClose;			//前收
//		int			m_nIndex;
		SStringA	m_strSubID;
		bool		m_bGroup;
		double		m_dPreSettle;
		double      m_dSettleRatio;
		void		SetDataPoint(vector<CommonStockMarket>* pStkMarketVec);
		void		SetDataPoint(vector<CommonIndexMarket>* pIdxMarketVec);
		void		ChangeShowData(SStringA StockID);
		CRect		m_rect;
		bool		m_bIsStockIndex;
		void		Paint(IRenderTarget *pRT);
		BOOL		m_bInited;
		int			*m_pGroupDataType;
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;

	protected:
		void		DrawStockOneCode(IRenderTarget * pRT);		//画单个合约
		void		DrawIndexOneCode(IRenderTarget * pRT);
		CAutoRefPtr<IFont> m_pFont15;
		std::vector<TICK_INFO>	m_vTick;		//显示用的数据

	};
}

