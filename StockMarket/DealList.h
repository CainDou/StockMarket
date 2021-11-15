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
		void		DrawOneCode(IRenderTarget * pRT);		//画单个合约
		void		DrawIndexOneCode(IRenderTarget * pRT);
		void		DrawGroupCode(IRenderTarget * pRT);		//画组合合约

		std::vector<TICK_INFO>	m_vTick;		//显示用的数据

	};
}

