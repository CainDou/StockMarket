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
		InsIDType	m_strSubIns;
//		int			m_nIndex;
		bool		m_bGroup;
		double		m_dPreSettle;
		double      m_dSettleRatio;
		void		SetShowData(int nIndex, bool bGroup);
		void		SetShowData(InsIDType strSubIns,bool bGroup);
		CRect		m_rect;
		bool		m_bIsStockIndex;
		void		Paint(IRenderTarget *pRT);

		int			*m_pGroupDataType;

	protected:
		void		DrawOneCode(IRenderTarget * pRT);		//��������Լ
		void		DrawIndexOneCode(IRenderTarget * pRT);
		void		DrawGroupCode(IRenderTarget * pRT);		//����Ϻ�Լ

		std::vector<TICK_INFO>	m_vTick;		//��ʾ�õ�����

	};
}

