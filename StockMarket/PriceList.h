#pragma once

//一些基本的界面框架信息

#define RC_PRH			25
#define MAX_LIST_NUM	300

namespace SOUI
{
	class CPriceList 
	{

	public:
		CPriceList();
//		void SetShowData(int nIndex, bool bGroup);
		void SetShowData(InsIDType strSubIns, bool bGroup);
		~CPriceList();

		InsIDType		m_strSubIns;
		bool			m_bGroup;
		int				m_nWorkMode;			//1:1档行情,2:5档行情
		float			m_fPreSettle;
		bool			m_bIsStockIndex;
		RestoreTickType	m_Tick;
		StockIndex_t	m_IndexTick;
		GroupDataType		m_Group;

		double				m_groupSettle;
		double				m_groupSettleRatio;
		//	double			m_fPreClose;			//前收
		int				m_nListNum;				//数据数量
		int				m_nDecimal;				//小数位数
		TCHAR			m_sDecimal[10];			//用来格式化
		int				m_nDecimalXi;			//10的m_nDecimal次方

		int				*m_pGroupDataType;

		BOOL			m_bInit;

		CRect			m_rect;

		CAutoRefPtr<IFont> m_pFont15, m_pFont20;
		CAutoRefPtr<IPen> m_penRed;
		void		Paint(IRenderTarget *pRT);

	protected:
		void		DrawModeOne(IRenderTarget * pRT);
		void		DrawIndexModeOne(IRenderTarget * pRT);
		void		DrawModeGroup(IRenderTarget * pRT);
		COLORREF	GetTextColor(double price);
		COLORREF    GetGroupColor(double price);
	};
}

