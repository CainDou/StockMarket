#pragma once

//һЩ�����Ľ�������Ϣ

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
		int				m_nWorkMode;			//1:1������,2:5������
		float			m_fPreSettle;
		bool			m_bIsStockIndex;
		RestoreTickType	m_Tick;
		StockIndex_t	m_IndexTick;
		GroupDataType		m_Group;

		double				m_groupSettle;
		double				m_groupSettleRatio;
		//	double			m_fPreClose;			//ǰ��
		int				m_nListNum;				//��������
		int				m_nDecimal;				//С��λ��
		TCHAR			m_sDecimal[10];			//������ʽ��
		int				m_nDecimalXi;			//10��m_nDecimal�η�

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

