#pragma once

//һЩ�����Ľ�������Ϣ

#define RC_HEIGHT		20
#define MAX_LIST_NUM	300

namespace SOUI
{
	class CDealList 
	{

	public:
		CDealList();
		~CDealList();

		int						m_nComboSelect;	//��ʾ�Ǹ�����
												//	double			m_fPreClose;			//ǰ��
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
		void		DrawStockOneCode(IRenderTarget * pRT);		//��������Լ
		void		DrawIndexOneCode(IRenderTarget * pRT);
		CAutoRefPtr<IFont> m_pFont15;
		std::vector<TICK_INFO>	m_vTick;		//��ʾ�õ�����

	};
}

