/**
* Copyright (C) 2014-2050 SOUI�Ŷ�
* All rights reserved.
*
* @file       SColorListCtrlEx.h
* @brief
* @version    v1.0
* @author     soui
* @date       2014-07-06
*
* Describe    ListCtrl�ؼ�
*/
#pragma once
#include "core/SPanel.h"
#include "SHeaderCtrlEx.h"

namespace SOUI
{
	/**
	* @enum
	* @brief     ����
	*
	* Describe   ����
	*/
	enum {
		S_LVIFEX_TEXT = 0x01,
		S_LVIFEX_IMAGE = 0x02,
		S_LVIFEX_INDENT = 0x04,
	};

	/**
	* qsort_s
	* @brief     �ȽϺ���--����ָ��
	*
	* Describe   ����ָ��
	*/
	typedef int(__cdecl  *PFNLVCOMPAREEX)(void *, const void *, const void *);//ʹ�ÿ��������㷨�еıȽϺ���,�ο�qsort_s

																			  /**
																			  * @struct    _DXLVSUBITEMEX
																			  * @brief     ����ṹ
																			  *
																			  * @Describe  ����ṹ
																			  */
	typedef struct _DXLVSUBITEMEX
	{
		/**
		* _DXLVSUBITEMEX
		* @brief     ���캯��
		*
		* Describe   ���캯��
		*/
		_DXLVSUBITEMEX()
		{
			mask = 0;
			nImage = 0;
			strText = NULL;
			cchTextMax = 0;
			nIndent = 0;
			clrref = CR_INVALID;
		}

		UINT		mask;         /**<  */
		LPTSTR		strText;      /**< �ı� */
		int			cchTextMax;   /**< �ı������ */
		UINT		nImage;       /**< ͼ�� */
		int			nIndent;      /**< ���� */
		COLORREF	clrref;		  /**< ��ɫ*/
	} DXLVSUBITEMEX;

	typedef SArray<DXLVSUBITEMEX>   ArrSubItemEx; /**< ������������	 */

												  /**
												  * @struct    _DXLVITEM
												  * @brief     ��Ŀ�ṹ
												  *
												  * @Describe  ��Ŀ�ṹ
												  */
	typedef struct _DXLVITEMEX
	{
		/**
		* _DXLVITEM
		* @brief     ���캯��
		*
		* Describe   ���캯��
		*/
		_DXLVITEMEX()
		{
			dwData = 0;
			arSubItems = NULL;
			checked = FALSE;
		}

		ArrSubItemEx	*arSubItems;
		DWORD			dwData;
		BOOL			checked;
	} DXLVITEMEX;

	//////////////////////////////////////////////////////////////////////////
	//  SColorListCtrlEx
	class SColorListCtrlEx : public SPanel
	{
		SOUI_CLASS_NAME(SColorListCtrlEx, L"color_listctrlex")

	public:

	//	HWND			m_ParWnd;
		void			OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void			EnsureVisible(int iItem);

		/**
		* SColorListCtrlEx::SColorListCtrlEx
		* @brief    ���캯��
		*
		* Describe  ���캯��
		*/
		SColorListCtrlEx();

		/**
		* SColorListCtrlEx::~SColorListCtrlEx
		* @brief    ��������
		*
		* Describe  ��������
		*/
		virtual ~SColorListCtrlEx();
		/**
		* SColorListCtrlEx::InsertColumn
		* @brief    ����һ��
		* @param    int nIndex -- ����
		* @param    LPCTSTR pszText -- ����
		* @param    int nWidth -- ���
		* @param    LPARAM lParam -- ���Ӳ���
		*
		* Describe  ����һ��
		*/
		int           InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, LPARAM lParam = 0);
		/**
		* SColorListCtrlEx::InsertItem
		* @brief    ������Ŀ
		* @param    int nIndex -- ����
		* @param    LPCTSTR pszText -- ����
		* @param    int nImage -- ͼ��
		*
		* Describe  ������Ŀ
		*/
		int             InsertItem(int nItem, LPCTSTR pszText, int nImage = -1);
		/**
		* SColorListCtrlEx::SetItemData
		* @brief    ���ø�������
		* @param    int nItem -- ����
		* @param    DWORD dwData-- ��������
		*
		* Describe  ���ø�������
		*/
		BOOL            SetItemData(int nItem, DWORD dwData);
		/**
		* SColorListCtrlEx::GetItemData
		* @brief    ��ȡ��������
		* @param    int nItem -- ����
		* @return   ����DWORD
		*
		* Describe  ��ȡ��������
		*/
		DWORD           GetItemData(int nItem);
		/**
		* SColorListCtrlEx::SetSubItem
		* @brief    ��������
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @param    const DXLVSUBITEMEX* plv --
		* @return   ����BOOL
		*
		* Describe  ��������
		*/
		BOOL            SetSubItem(int nItem, int nSubItem, const DXLVSUBITEMEX* plv);
		/**
		* SColorListCtrlEx::GetSubItem
		* @brief    ��ȡ����
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @param    DXLVSUBITEMEX* plv --
		* @return   ����BOOL
		*
		* Describe  ��ȡ��������
		*/
		BOOL            GetSubItem(int nItem, int nSubItem, DXLVSUBITEMEX* plv) const;
		/**
		* SColorListCtrlEx::SetSubItemText
		* @brief    ���������ı�
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @param    LPCTSTR pszText -- �ı�
		* @param    COLORREF clr -- ��ɫ(Ĭ��ΪĬ��ɫ)
		* @return   ����BOOL
		*
		* Describe  ���������ı�
		*/
		BOOL            SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText, COLORREF clr = CR_INVALID);

		/**
		* SColorListCtrlEx::SetSubItemColor
		* @brief    ����������ɫ
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @param    COLORREF clr -- ��ɫ
		* @return   ����BOOL
		*
		* Describe  ����������ɫ
		*/
		BOOL SetSubItemColor(int nItem, int nSubItem, COLORREF clr);

		/**
		* GetSubItemText
		* @brief    ��ȡ�����ı�
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @return   SOUI::SStringT -- ����ַ���
		* Describe
		*/
		SStringT        GetSubItemText(int nItem, int nSubItem) const;

		/**
		* SColorListCtrlEx::GetSelectedItem
		* @brief    ��ȡѡ����
		* @return   ����int
		*
		* Describe  ����ѡ����
		*/
		int             GetSelectedItem();
		/**
		* SColorListCtrlEx::SetSelectedItem
		* @brief    ����ѡ����
		* @param    int nItem -- ����
		*
		* Describe  ����ѡ����
		*/
		void            SetSelectedItem(int nItem);
		/**
		* SColorListCtrlEx::GetItemCount
		* @brief    ��ȡ��Ŀ����
		* @return   ����int
		*
		* Describe  ��ȡ��Ŀ����
		*/
		int             GetItemCount() const;
		/**
		* SColorListCtrlEx::SetItemCount
		* @brief    ������Ŀ��
		* @param    int nItems -- ����
		* @param    int nGrowBy --
		* @return   ����BOOL
		*
		* Describe  ������Ŀ��
		*/
		BOOL            SetItemCount(int nItems, int nGrowBy);
		/**
		* SColorListCtrlEx::GetColumnCount
		* @brief    ��ȡ������
		* @return   ����int
		*
		* Describe  ��ȡ������
		*/
		int             GetColumnCount() const;
		/**
		* SColorListCtrlEx::GetCountPerPage
		* @brief    ��ȡÿҳ��Ŀ����
		* @return   ����int
		*
		* Describe  ��ȡÿҳ��Ŀ����
		*/
		int             GetCountPerPage(BOOL bPartial);
		/**
		* SColorListCtrlEx::DeleteItem
		* @brief    ɾ��ָ����
		* @param    int nItem -- ����
		*
		* Describe  ɾ��ָ����
		*/
		void            DeleteItem(int nItem);
		/**
		* SColorListCtrlEx::DeleteColumn
		* @brief    ɾ��ָ����
		* @param    int iCol -- ����
		*
		* Describe  ɾ��ָ����
		*/
		void            DeleteColumn(int iCol);
		/**
		* SColorListCtrlEx::DeleteAllItems
		* @brief    ɾ��������
		*
		* Describe  ɾ��������
		*/
		void            DeleteAllItems();

		/**
		* SColorListCtrlEx::CreateChildren
		* @brief    ��������
		* @param    pugi::xml_node xmlNode -- xml�ļ�
		*
		* Describe  ͨ������xml�ļ�����
		*/
		virtual BOOL    CreateChildren(pugi::xml_node xmlNode);

		/**
		* SColorListCtrlEx::HitTest
		* @brief    ��ȡ��������Ϣ
		* @param    CPoint &pt -- ����
		*
		* Describe
		*/
		int             HitTest(const CPoint& pt);
		/**
		* SColorListCtrlEx::SortItems
		* @brief    ����
		* @param    PFNLVCOMPAREEX pfnCompare -- �ȽϺ���
		* @param    void * pContext -- �Ƚ�����
		* @return   ����BOOL
		*
		* Describe  ����
		*/
		BOOL            SortItems(PFNLVCOMPAREEX pfnCompare, void * pContext);

		/**
		* SColorListCtrlEx::GetCheckState
		* @brief    ��ȡĳһ���Ƿ�ѡ��
		* @param    int nItem -- ָ����һ��
		* @return   ����ѡ��״̬
		*
		* Describe  ��ȡĳһ���Ƿ�ѡ��
		*/
		BOOL            GetCheckState(int nItem);

		/**
		* SColorListCtrlEx::SetCheckState
		* @brief   ����ĳһ�е�ѡ��״̬
		* @param   int nItem -- ָ����
		* @param   BOOL bCheck -- ״̬
		* @return  ���غ���ִ���Ƿ�ɹ�
		*
		* Describe ����ĳһ�е�ѡ��״̬
		*/
		BOOL            SetCheckState(int nItem, BOOL bCheck);

		/**
		* SColorListCtrlEx::GetCheckedItemCount
		* @brief   ��ȡѡ���еļ���
		* @return  ���ؼ���
		*
		* Describe ��ȡѡ���еļ���
		*/
		int             GetCheckedItemCount();


		int             GetFirstCheckedItem();
		int             GetLastCheckedItem();
		VOID            EnableMultiSelection(BOOL enable) { m_bMultiSelection = enable; }
		VOID            EnableCheckBox(BOOL enable) { m_bCheckBox = enable; }
		VOID            EnableHotTrack(BOOL enable) { m_bHotTrack = enable; }

		/**
		* SColorListCtrlEx::GetTopIndex
		* @brief    ��ȡ�ö���ѡ������
		* @return   ����int
		*
		* Describe  ����ѡ����
		*/
		int             GetTopIndex() const;

		/**
		* SColorListCtrlEx::OnLButtonUp
		* @brief    ���̧��
		* @param    UINT nFlags -- ��־
		* @param    CPoint pt -- ����
		*
		* Describe  ��Ϣ��Ӧ����
		*/
		void    OnLButtonUp(UINT nFlags, CPoint pt);

		/**
		* SColorListCtrlEx::OnLButtonDown
		* @brief    �������
		* @param    UINT nFlags -- ��־
		* @param    CPoint pt -- ����
		*
		* Describe  ��Ϣ��Ӧ����
		*/
		void    OnLButtonDown(UINT nFlags, CPoint pt);

		void	SetHoverSelected();


		/**
		* SColorListCtrlEx::UpdateScrollBar
		* @brief    ���¹�����
		*
		* Describe  ���¹�����
		*/
		void            UpdateScrollBar();


	protected:

		/**
		* SColorListCtrlEx::GetItemRect
		* @brief    ��ȡitemλ��
		* @param    int nItem -- ����
		* @param    int nSubItem -- ��������
		* @return   ����int
		*
		* Describe  ����ѡ����
		*/
		CRect           GetItemRect(int nItem, int nSubItem = 0);

		/**
		* SColorListCtrlEx::DrawItem
		* @brief    ����
		* @param    IRenderTarget *pRT -- �����豸
		* @param    CRect &rc -- Ŀ������
		* @param    int iItem -- ѡ��
		*
		* Describe  ����
		*/
		virtual void    DrawItem(IRenderTarget *pRT, CRect rcItem, int nItem);
		/**
		* SColorListCtrlEx::RedrawItem
		* @brief    �ػ�ѡ��
		* @param    int iItem  -- ����
		*
		* Describe  �ػ�ѡ��
		*/
		void            RedrawItem(int nItem);

		/**
		* SColorListCtrlEx::NotifySelChange
		* @brief    �޸�ѡ����
		* @param    int nOldSel -- ��ѡ����
		* @param    int nNewSel -- ��ѡ����
		*
		* Describe  �޸�ѡ����
		*/
		void            NotifySelChange(int nOldSel, int nNewSel, BOOL checkBox = FALSE);
		/**
		* SColorListCtrlEx::OnPaint
		* @brief    ����
		* @param    IRenderTarget *pRT -- �����豸
		*
		* Describe  ��Ϣ��Ӧ����
		*/
		void            OnPaint(IRenderTarget *pRT);
		/**
		* SColorListCtrlEx::OnDestroy
		* @brief    ����
		*
		* Describe  ����
		*/
		void            OnDestroy();
		/**
		* SColorListCtrlEx::OnHeaderClick
		* @brief    �б�ͷ�����¼� --
		* @param    EventArgs *pEvt
		*
		* Describe  �б�ͷ�����¼�
		*/
		bool            OnHeaderClick(EventArgs *pEvt);
		/**
		* SColorListCtrlEx::OnHeaderSizeChanging
		* @brief    �б�ͷ��С�ı�
		* @param    EventArgs *pEvt --
		*
		* Describe  �б�ͷ��С�ı�
		*/
		bool            OnHeaderSizeChanging(EventArgs *pEvt);
		/**
		* SColorListCtrlEx::OnHeaderSwap
		* @brief    �б�ͷ����
		* @param    EventArgs *pEvt --
		*
		* Describe  �б�ͷ����
		*/
		bool            OnHeaderSwap(EventArgs *pEvt);
		/**
		* SColorListCtrlEx::OnScroll
		* @brief    �����¼�
		* @param    BOOL bVertical -- �Ƿ��Ǵ�ֱ
		* @param    UINT uCode -- ��������
		* @param    int nPos -- ����λ��
		*
		* Describe  ���ڹ����¼�
		*/
		virtual BOOL    OnScroll(BOOL bVertical, UINT uCode, int nPos);

		/**
		* SListBox::OnLButtonDbClick
		* @brief    ���˫��
		* @param    UINT nFlags -- ��־
		* @param    CPoint pt -- ����
		*
		* Describe  ��Ϣ��Ӧ����
		*/
		void OnLButtonDbClick(UINT nFlags, CPoint pt);


		/**
		* OnMouseMove
		* @brief    ��������ƶ�
		* @param    UINT nFlags --  ��־
		* @param    CPoint pt --  ����
		* @return   void
		* Describe
		*/
		void    OnMouseMove(UINT nFlags, CPoint pt);

		/**
		* OnMouseLeave
		* @brief    ��������뿪��Ϣ
		* @return   void
		* Describe
		*/
		void   OnMouseLeave();

		BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

		/**
		* SColorListCtrlEx::OnSize
		* @brief    ��Ϣ��Ӧ����
		* @param    UINT nType --
		* @param    CSize size --
		*
		* Describe  ��ȡĳ�������
		*/
		void    OnSize(UINT nType, CSize size);
		/**
		* SColorListCtrlEx::UpdateChildrenPosition
		* @brief    ��������λ��
		*
		* Describe  ��������λ��
		*/
		virtual void    UpdateChildrenPosition();
		/**
		* SColorListCtrlEx::GetListRect
		* @brief    ��ȡlistλ��
		* @return   ����CRect
		*
		* Describe  ��ȡlistλ��
		*/
		CRect           GetListRect();
		/**
		* SColorListCtrlEx::UpdateHeaderCtrl
		* @brief    �����б�ͷ�ؼ�
		*
		* Describe  �����б�ͷ�ؼ�
		*/
		void            UpdateHeaderCtrl();


		BOOL            HitCheckBox(const CPoint& pt);




	protected:
		int             m_nHeaderHeight;  /**< �б�ͷ�߶� */
		int             m_nItemHeight;  /**< ��Ŀ�߶� */

		int             m_nSelectItem;  /**< ѡ���� */
		int             m_nHoverItem;  /**< Hover״̬�� */
		BOOL            m_bHotTrack;  /**<  */

		CPoint          m_ptIcon;  /**< ͼ��λ�� */
		CPoint          m_ptText;  /**< �ı�λ�� */

		COLORREF        m_crItemBg;  /**< ����ɫ */
		COLORREF        m_crItemBg2;  /**< ����ɫ */
		COLORREF        m_crItemSelBg;  /**< ѡ�б���ɫ */
		COLORREF        m_crItemHotBg;  /**< Hot����ɫ */
		COLORREF        m_crText;  /**< �ı���ɫ */
								   //       COLORREF        m_crSelText;  /**< ѡ���ı���ɫ */

		ISkinObj*    m_pItemSkin;  /**< */
		ISkinObj*    m_pIconSkin;  /**< */
		ISkinObj*    m_pCheckSkin; /**< */
		BOOL        m_bCheckBox;
		BOOL        m_bMultiSelection;

	protected:
		typedef SArray<DXLVITEMEX> ArrLvItemEx;  /**< ����item���� */

		SHeaderCtrlEx*	m_pHeader;  /**< �б�ͷ�ؼ� */
		ArrLvItemEx     m_arrItems;  /**< */
		CPoint          m_ptOrigin;  /**< */



	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"headerHeight", m_nHeaderHeight, FALSE)
			ATTR_INT(L"itemHeight", m_nItemHeight, FALSE)
			ATTR_INT(L"checkBox", m_bCheckBox, TRUE)
			ATTR_INT(L"multiSelection", m_bMultiSelection, TRUE)
			ATTR_SKIN(L"itemSkin", m_pItemSkin, TRUE)
			ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
			ATTR_SKIN(L"checkSkin", m_pCheckSkin, TRUE)
			ATTR_COLOR(L"colorItemBkgnd", m_crItemBg, FALSE)
			ATTR_COLOR(L"colorItemBkgnd2", m_crItemBg2, FALSE)
			ATTR_COLOR(L"colorItemHotBkgnd", m_crItemHotBg, FALSE)
			ATTR_COLOR(L"colorItemSelBkgnd", m_crItemSelBg, FALSE)
			ATTR_COLOR(L"colorText", m_crText, FALSE)
			//        ATTR_COLOR(L"colorSelText", m_crSelText, FALSE)
			ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
			ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
			ATTR_INT(L"text-x", m_ptText.x, FALSE)
			ATTR_INT(L"text-y", m_ptText.y, FALSE)
			ATTR_INT(L"hotTrack", m_bHotTrack, FALSE)
			SOUI_ATTRS_END()

			SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
	//		MSG_WM_KEYDOWN(OnKeyDown)
			SOUI_MSG_MAP_END()
	};

}//end of namespace
