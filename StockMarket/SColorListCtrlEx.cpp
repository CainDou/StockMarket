#include "stdafx.h"
#include "SColorListCtrlEx.h"

#pragma warning(disable : 4267 4018)

#define ITEM_MARGIN 4

#define WM_SINGLE (WM_USER+161)
namespace SOUI
{
	//////////////////////////////////////////////////////////////////////////
	//  SColorListCtrlEx
	SColorListCtrlEx::SColorListCtrlEx()
		: m_nHeaderHeight(20)
		, m_nItemHeight(20)
		, m_pHeader(NULL)
		, m_nSelectItem(-1)
		, m_crItemBg(RGBA(255, 255, 255, 255))
		, m_crItemBg2(RGBA(226, 226, 226, 255))
		, m_crItemSelBg(RGBA(57, 145, 209, 255))
		, m_crItemHotBg(RGBA(57, 145, 209, 128))
		, m_crText(RGBA(0, 0, 0, 255))
		//   , m_crSelText(RGBA(255,255,0,255))
		, m_pItemSkin(NULL)
		, m_pIconSkin(NULL)
		, m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_CHECKBOX))
		, m_ptIcon(-1, -1)
		, m_ptText(-1, -1)
		, m_bHotTrack(FALSE)
		, m_bCheckBox(FALSE)
		, m_bMultiSelection(FALSE)
	{
		m_bClipClient = TRUE;
		m_bFocusable = TRUE;
		m_evtSet.addEvent(EVENTID(EventLCSelChanging));
		m_evtSet.addEvent(EVENTID(EventLCSelChanged));
		m_evtSet.addEvent(EVENTID(EventLCDbClick));
		m_evtSet.addEvent(EVENTID(EventLCItemDeleted));
	}

	SColorListCtrlEx::~SColorListCtrlEx()
	{
	}

	int SColorListCtrlEx::InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, LPARAM lParam)
	{
		SASSERT(m_pHeader);

		int nRet = m_pHeader->InsertItem(nIndex, pszText, nWidth, ST_NULL, lParam);
		for (int i = 0; i < GetItemCount(); i++)
		{
			m_arrItems[i].arSubItems->SetCount(GetColumnCount());
		}
		UpdateScrollBar();
		return nRet;
	}

	BOOL SColorListCtrlEx::CreateChildren(pugi::xml_node xmlNode)
	{
		//  listctrl的子控件只能是一个header控件
		if (!__super::CreateChildren(xmlNode))
			return FALSE;
		m_pHeader = NULL;

		SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
		while (pChild)
		{
			if (pChild->IsClass(SHeaderCtrlEx::GetClassName()))
			{
				m_pHeader = (SHeaderCtrlEx*)pChild;
				m_pHeader->InitText();
				break;
			}
			pChild = pChild->GetWindow(GSW_NEXTSIBLING);
		}
		if (!m_pHeader) return FALSE;

		SStringW strPos;
		strPos.Format(L"0,0,-0,%d", m_nHeaderHeight);
		m_pHeader->SetAttribute(L"pos", strPos, TRUE);

		m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemChanging::EventID, Subscriber(&SColorListCtrlEx::OnHeaderSizeChanging, this));
		m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemSwap::EventID, Subscriber(&SColorListCtrlEx::OnHeaderSwap, this));

		return TRUE;
	}

	int SColorListCtrlEx::InsertItem(int nItem, LPCTSTR pszText, int nImage)
	{
		if (GetColumnCount() == 0) return -1;
		if (nItem<0 || nItem>GetItemCount())
			nItem = GetItemCount();

		DXLVITEMEX lvi;
		lvi.dwData = 0;
		lvi.arSubItems = new ArrSubItemEx();
		lvi.arSubItems->SetCount(GetColumnCount());

		DXLVSUBITEMEX &subItem = lvi.arSubItems->GetAt(0);
		subItem.strText = _tcsdup(pszText);
		subItem.cchTextMax = _tcslen(pszText);
		subItem.nImage = nImage;

		m_arrItems.InsertAt(nItem, lvi);

		UpdateScrollBar();

		return nItem;
	}

	BOOL SColorListCtrlEx::SetItemData(int nItem, DWORD dwData)
	{
		if (nItem >= GetItemCount())
			return FALSE;

		m_arrItems[nItem].dwData = dwData;

		return TRUE;
	}

	DWORD SColorListCtrlEx::GetItemData(int nItem)
	{
		if (nItem >= GetItemCount())
			return 0;

		DXLVITEMEX& lvi = m_arrItems[nItem];

		return (DWORD)lvi.dwData;
	}

	BOOL SColorListCtrlEx::SetSubItem(int nItem, int nSubItem, const DXLVSUBITEMEX* plv)
	{
		if (nItem >= GetItemCount() || nSubItem >= GetColumnCount())
			return FALSE;
		DXLVSUBITEMEX & lvsi_dst = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
		if (plv->mask & S_LVIFEX_TEXT)
		{
			if (lvsi_dst.strText) free(lvsi_dst.strText);
			lvsi_dst.strText = _tcsdup(plv->strText);
			lvsi_dst.cchTextMax = _tcslen(plv->strText);
		}
		if (plv->mask&S_LVIFEX_IMAGE)
			lvsi_dst.nImage = plv->nImage;
		if (plv->mask&S_LVIFEX_INDENT)
			lvsi_dst.nIndent = plv->nIndent;
		RedrawItem(nItem);
		return TRUE;
	}

	BOOL SColorListCtrlEx::GetSubItem(int nItem, int nSubItem, DXLVSUBITEMEX* plv) const
	{
		if (nItem >= GetItemCount() || nSubItem >= GetColumnCount())
			return FALSE;

		const DXLVSUBITEMEX & lvsi_src = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
		if (plv->mask & S_LVIFEX_TEXT)
		{
			_tcscpy_s(plv->strText, plv->cchTextMax, lvsi_src.strText);
		}
		if (plv->mask&S_LVIFEX_IMAGE)
			plv->nImage = lvsi_src.nImage;
		if (plv->mask&S_LVIFEX_INDENT)
			plv->nIndent = lvsi_src.nIndent;
		return TRUE;
	}

	BOOL SColorListCtrlEx::SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText, COLORREF clr)
	{
		if (nItem < 0 || nItem >= GetItemCount())
			return FALSE;

		if (nSubItem < 0 || nSubItem >= GetColumnCount())
			return FALSE;

		DXLVSUBITEMEX &lvi = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
		if (lvi.strText)
		{
			free(lvi.strText);
		}

		if (clr != CR_INVALID)
		{
			lvi.clrref = clr;
		}
		lvi.strText = _tcsdup(pszText);
		lvi.cchTextMax = _tcslen(pszText);

		CRect rcItem = GetItemRect(nItem, nSubItem);
		InvalidateRect(rcItem);
		return TRUE;
	}

	BOOL SColorListCtrlEx::SetSubItemColor(int nItem, int nSubItem, COLORREF clr)
	{
		if (nItem < 0 || nItem >= GetItemCount())
			return FALSE;

		if (nSubItem < 0 || nSubItem >= GetColumnCount())
			return FALSE;

		if (clr == CR_INVALID)
			return FALSE;

		DXLVSUBITEMEX &lvi = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
		lvi.clrref = clr;

		CRect rcItem = GetItemRect(nItem, nSubItem);
		InvalidateRect(rcItem);
		return TRUE;
	}


	SStringT SColorListCtrlEx::GetSubItemText(int nItem, int nSubItem) const
	{
		if (nItem >= GetItemCount() || nSubItem >= GetColumnCount())
			return _T("");

		const DXLVSUBITEMEX & lvsi_src = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
		return lvsi_src.strText;
	}

	int SColorListCtrlEx::GetSelectedItem()
	{
		return m_nSelectItem;
	}

	void SColorListCtrlEx::SetSelectedItem(int nItem)
	{
		if (nItem != m_nSelectItem)
		{
			NotifySelChange(m_nSelectItem, nItem);
		}
	}

	int SColorListCtrlEx::GetItemCount() const
	{
		if (GetColumnCount() <= 0)
			return 0;

		return m_arrItems.GetCount();
	}

	BOOL SColorListCtrlEx::SetItemCount(int nItems, int nGrowBy)
	{
		int nOldCount = GetItemCount();
		if (nItems < nOldCount) return FALSE;

		BOOL bRet = m_arrItems.SetCount(nItems, nGrowBy);
		if (bRet)
		{
			for (int i = nOldCount; i < nItems; i++)
			{
				DXLVITEMEX & lvi = m_arrItems[i];
				lvi.arSubItems = new ArrSubItemEx;
				lvi.arSubItems->SetCount(GetColumnCount());
			}
		}
		UpdateScrollBar();

		return bRet;
	}

	CRect SColorListCtrlEx::GetListRect()
	{
		CRect rcList;

		GetClientRect(&rcList);
		rcList.top += m_nHeaderHeight;

		return rcList;
	}

	//////////////////////////////////////////////////////////////////////////
	//  更新滚动条
	void SColorListCtrlEx::UpdateScrollBar()
	{
		CSize szView;
		szView.cx = m_pHeader->GetTotalWidth();
		szView.cy = GetItemCount()*m_nItemHeight;

		CRect rcClient;
		SWindow::GetClientRect(&rcClient);//不计算滚动条大小
		rcClient.top += m_nHeaderHeight;

		CSize size = rcClient.Size();
		//  关闭滚动条
		m_wBarVisible = SSB_NULL;
		if (size.cy < szView.cy || (size.cy < szView.cy + GetSbWidth() && size.cx < szView.cx))
		{
			//  需要纵向滚动条
			m_wBarVisible |= SSB_VERT;
			m_siVer.nMin = 0;
			m_siVer.nMax = szView.cy - 1;
			m_siVer.nPage = GetCountPerPage(FALSE)*m_nItemHeight;
			if (size.cx - GetSbWidth() < szView.cx)
			{
				//  需要横向滚动条
				m_wBarVisible |= SSB_HORZ;
				m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0;//注意同时调整纵向滚动条page信息

				m_siHoz.nMin = 0;
				m_siHoz.nMax = szView.cx - 1;
				m_siHoz.nPage = size.cx - GetSbWidth() > 0 ? size.cx - GetSbWidth() : 0;
			}
			else
			{
				//  不需要横向滚动条
				m_siHoz.nPage = size.cx;
				m_siHoz.nMin = 0;
				m_siHoz.nMax = m_siHoz.nPage - 1;
				m_siHoz.nPos = 0;
				m_ptOrigin.x = 0;
			}
		}
		else
		{
			//  不需要纵向滚动条
			m_siVer.nPage = size.cy;
			m_siVer.nMin = 0;
			m_siVer.nMax = size.cy - 1;
			m_siVer.nPos = 0;
			m_ptOrigin.y = 0;

			if (size.cx < szView.cx)
			{

				//  需要横向滚动条
				m_wBarVisible |= SSB_HORZ;
				m_siHoz.nMin = 0;
				m_siHoz.nMax = szView.cx - 1;
				m_siHoz.nPage = size.cx;
			}
			else
			{
				//  不需要横向滚动条
				m_siHoz.nPage = size.cx;
				m_siHoz.nMin = 0;
				m_siHoz.nMax = m_siHoz.nPage - 1;
				m_siHoz.nPos = 0;
				m_ptOrigin.x = 0;
			}
		}

		SetScrollPos(TRUE, m_siVer.nPos, TRUE);
		SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

		//  重新计算客户区及非客户区
		SSendMessage(WM_NCCALCSIZE);

		//  根据需要调整原点位置
		if (HasScrollBar(FALSE) && m_ptOrigin.x + m_siHoz.nPage > szView.cx)
		{
			m_ptOrigin.x = szView.cx - m_siHoz.nPage;
		}

		if (HasScrollBar(TRUE) && m_ptOrigin.y + m_siVer.nPage > szView.cy)
		{
			m_ptOrigin.y = szView.cy - m_siVer.nPage;
		}

		Invalidate();
	}

	//更新表头位置
	void SColorListCtrlEx::UpdateHeaderCtrl()
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		CRect rcHeader(rcClient);
		rcHeader.bottom = rcHeader.top + m_nHeaderHeight;
		rcHeader.left -= m_ptOrigin.x;
		if (m_pHeader) m_pHeader->SetOffSet(-m_ptOrigin.x);

	}

	void SColorListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_nSelectItem < 0)
		{
			if (nChar == VK_DOWN)
				m_nSelectItem = GetTopIndex();
			if (nChar == VK_UP)
			{
				if (GetItemCount() < GetCountPerPage(false))
					m_nSelectItem = GetItemCount();
				else
					m_nSelectItem = GetTopIndex() + GetCountPerPage(false);

			}

		}
		int  nNewSelItem = -1;
		if (nChar == VK_DOWN && m_nSelectItem < GetItemCount() - 1)
			nNewSelItem = m_nSelectItem + 1;
		else if (nChar == VK_UP && m_nSelectItem > 0)
			nNewSelItem = m_nSelectItem - 1;


		if (nNewSelItem != -1)
		{
			EnsureVisible(nNewSelItem);
			SetSelectedItem(nNewSelItem);
		}

	}

	void SColorListCtrlEx::DeleteItem(int nItem)
	{
		if (nItem >= 0 && nItem < GetItemCount())
		{
			DXLVITEMEX &lvi = m_arrItems[nItem];

			EventLCItemDeleted evt2(this);
			evt2.nItem = nItem;
			evt2.dwData = lvi.dwData;
			FireEvent(evt2);

			for (int i = 0; i < GetColumnCount(); i++)
			{
				DXLVSUBITEMEX &lvsi = lvi.arSubItems->GetAt(i);
				if (lvsi.strText) free(lvsi.strText);
			}
			delete lvi.arSubItems;
			m_arrItems.RemoveAt(nItem);

			UpdateScrollBar();
		}
	}

	void SColorListCtrlEx::DeleteColumn(int iCol)
	{
		if (m_pHeader->DeleteItem(iCol))
		{
			int nColumnCount = m_pHeader->GetItemCount();

			for (int i = 0; i < GetItemCount(); i++)
			{
				DXLVITEMEX &lvi = m_arrItems[i];

				if (0 == nColumnCount)
				{
					EventLCItemDeleted evt2(this);
					evt2.nItem = i;
					evt2.dwData = lvi.dwData;
					FireEvent(evt2);
				}

				DXLVSUBITEMEX &lvsi = lvi.arSubItems->GetAt(iCol);
				if (lvsi.strText) free(lvsi.strText);
				m_arrItems[i].arSubItems->RemoveAt(iCol);
			}
			UpdateScrollBar();
		}
	}

	void SColorListCtrlEx::DeleteAllItems()
	{
		m_nSelectItem = -1;
		for (int i = 0; i < GetItemCount(); i++)
		{
			DXLVITEMEX &lvi = m_arrItems[i];

			EventLCItemDeleted evt2(this);
			evt2.nItem = i;
			evt2.dwData = lvi.dwData;
			FireEvent(evt2);

			for (int j = 0; j < GetColumnCount(); j++)
			{
				DXLVSUBITEMEX &lvsi = lvi.arSubItems->GetAt(j);
				if (lvsi.strText) free(lvsi.strText);
			}
			delete lvi.arSubItems;
		}
		m_arrItems.RemoveAll();

		//UpdateScrollBar();
	}


	CRect SColorListCtrlEx::GetItemRect(int nItem, int nSubItem)
	{
		if (!(nItem >= 0 && nItem < GetItemCount() && nSubItem >= 0 && nSubItem < GetColumnCount()))
			return CRect();

		CRect rcItem;
		rcItem.top = m_nItemHeight*nItem;
		rcItem.bottom = rcItem.top + m_nItemHeight;
		rcItem.left = 0;
		rcItem.right = 0;

		for (int nCol = 0; nCol < GetColumnCount(); nCol++)
		{
			SHDITEM hdi;
			hdi.mask = SHDI_WIDTH | SHDI_ORDER;

			m_pHeader->GetItem(nCol, &hdi);
			rcItem.left = rcItem.right;
			rcItem.right = rcItem.left + hdi.cx.toPixelSize(GetScale());
			if (hdi.iOrder == nSubItem)
				break;
		}

		CRect rcList = GetListRect();
		//  变换到窗口坐标
		rcItem.OffsetRect(rcList.TopLeft());
		//  根据原点坐标修正
		rcItem.OffsetRect(-m_ptOrigin);

		return rcItem;
	}

	//////////////////////////////////////////////////////////////////////////
	//  自动修改pt的位置为相对当前项的偏移量
	int SColorListCtrlEx::HitTest(const CPoint& pt)
	{
		CRect rcList = GetListRect();

		CPoint pt2 = pt;
		pt2.y -= rcList.top - m_ptOrigin.y;

		int nRet = pt2.y / m_nItemHeight;
		if (nRet >= GetItemCount())
		{
			nRet = -1;
		}

		return nRet;
	}

	void SColorListCtrlEx::RedrawItem(int nItem)
	{
		if (!IsVisible(TRUE))
			return;

		CRect rcList = GetListRect();

		int nTopItem = GetTopIndex();
		int nPageItems = (rcList.Height() + m_nItemHeight - 1) / m_nItemHeight;

		if (nItem >= nTopItem && nItem < GetItemCount() && nItem <= nTopItem + nPageItems)
		{
			CRect rcItem(0, 0, rcList.Width(), m_nItemHeight);
			rcItem.OffsetRect(0, m_nItemHeight*nItem - m_ptOrigin.y);
			rcItem.OffsetRect(rcList.TopLeft());
			CRect rcDC;
			rcDC.IntersectRect(rcItem, rcList);
			IRenderTarget *pRT = GetRenderTarget(&rcDC, OLEDC_PAINTBKGND);
			SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);

			DrawItem(pRT, rcItem, nItem);

			ReleaseRenderTarget(pRT);
		}
	}

	int SColorListCtrlEx::GetCountPerPage(BOOL bPartial)
	{
		CRect rcClient = GetListRect();

		// calculate number of items per control height (include partial item)
		div_t divHeight = div(rcClient.Height(), m_nItemHeight);

		// round up to nearest item count
		return (std::max)((int)(bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot), 1);
	}
	BOOL SColorListCtrlEx::SortItems(
		PFNLVCOMPAREEX pfnCompare,
		void * pContext
	)
	{
		if (m_nSelectItem > 0)
			m_arrItems[m_nSelectItem].checked = FALSE;
		qsort_s(m_arrItems.GetData(), m_arrItems.GetCount(), sizeof(DXLVITEMEX), pfnCompare, pContext);
		if (m_nSelectItem > 0)
			m_arrItems[m_nSelectItem].checked = TRUE;
		InvalidateRect(GetListRect());
		return TRUE;
	}

	void SColorListCtrlEx::OnPaint(IRenderTarget * pRT)
	{
		SPainter painter;
		BeforePaint(pRT, painter);
		CRect rcList = GetListRect();
		int nTopItem = GetTopIndex();
		pRT->PushClipRect(&rcList);
		CRect rcItem(rcList);

		rcItem.bottom = rcItem.top;
		rcItem.OffsetRect(0, -(m_ptOrigin.y%m_nItemHeight));
		for (int nItem = nTopItem; nItem <= (nTopItem + GetCountPerPage(TRUE)) && nItem < GetItemCount(); rcItem.top = rcItem.bottom, nItem++)
		{
			rcItem.bottom = rcItem.top + m_nItemHeight;

			DrawItem(pRT, rcItem, nItem);
		}
		pRT->PopClip();
		AfterPaint(pRT, painter);

	}

	BOOL SColorListCtrlEx::HitCheckBox(const CPoint& pt)
	{
		if (!m_bCheckBox)
			return FALSE;

		CRect rect = GetListRect();
		rect.left += ITEM_MARGIN;
		rect.OffsetRect(-m_ptOrigin.x, 0);

		CSize sizeSkin = m_pCheckSkin->GetSkinSize();
		int nOffsetX = 3;
		CRect rcCheck;
		rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
		rcCheck.OffsetRect(rect.left + nOffsetX, 0);

		if (pt.x >= rcCheck.left && pt.x <= rcCheck.right)
			return TRUE;
		return FALSE;
	}

	BOOL SColorListCtrlEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		SPanel::OnMouseWheel(nFlags, zDelta, pt);
		return 0;
	}

	void SColorListCtrlEx::EnsureVisible(int iItem)
	{
		if (iItem < 0 || iItem >= GetItemCount()) return;

		int iFirstVisible = GetTopIndex();
		int iLastVisible = iFirstVisible + GetCountPerPage(false);

		if (iItem >= iFirstVisible && iItem < iLastVisible)
		{
			if (iItem == iFirstVisible)
			{
				int pos = m_nItemHeight*iItem;
				OnScroll(TRUE, SB_THUMBPOSITION, pos);
			}
			else if (iItem == iLastVisible - 1)
			{
				if (iItem == GetItemCount() - 1)
					OnScroll(TRUE, SB_BOTTOM, 0);
				else
				{
					int pos = (iItem + 1)*m_nItemHeight - m_siVer.nPage;
					OnScroll(TRUE, SB_THUMBPOSITION, pos);
				}
			}

			return;
		}


		if (iItem < iFirstVisible)
		{//scroll up
			int pos = m_nItemHeight*iItem;
			OnScroll(TRUE, SB_THUMBPOSITION, pos);
		}
		else // if(iItem >= iLastVisible)
		{//scroll down
			if (iItem == GetItemCount() - 1)
			{
				OnScroll(TRUE, SB_BOTTOM, 0);
			}
			else
			{
				int pos = (iItem + 1)*m_nItemHeight - m_siVer.nPage;
				OnScroll(TRUE, SB_THUMBPOSITION, pos);
			}
		}
	}

	void SColorListCtrlEx::DrawItem(IRenderTarget * pRT, CRect rcItem, int nItem)
	{
		BOOL bTextColorChanged = FALSE;
		int nBgImg = 0;
		COLORREF crOldText = RGBA(0xFF, 0xFF, 0xFF, 0xFF);
		COLORREF crItemBg = m_crItemBg;
		COLORREF crText = m_crText;
		DXLVITEMEX lvItem = m_arrItems[nItem];
		CRect rcIcon, rcText;

		CRect rcClient;
		GetClientRect(&rcClient);

		if (nItem % 2)
		{
			//         if (m_pItemSkin != NULL)
			//             nBgImg = 1;
			//         else if (CR_INVALID != m_crItemBg2)
			//             crItemBg = m_crItemBg2;
			//上面的代码不要了，因为skin间隔效果没必要，只留下颜色间隔就好了
			if (CR_INVALID != m_crItemBg2)
				crItemBg = m_crItemBg2;
		}

		if (lvItem.checked)
		{//和下面那个if的条件分开，才会有sel和hot的区别
			if (m_pItemSkin != NULL)
				nBgImg = 2;
			else if (CR_INVALID != m_crItemSelBg)
				crItemBg = m_crItemSelBg;

			//	if (CR_INVALID != m_crSelText)
			//		crText = m_crSelText;
		}
		else if (m_bHotTrack && nItem == m_nHoverItem)
		{
			if (m_pItemSkin != NULL)
				nBgImg = 1;
			else if (CR_INVALID != m_crItemHotBg)
				crItemBg = m_crItemHotBg;

			//	if (CR_INVALID != m_crSelText)
			//		crText = m_crSelText;
		}

		//绘制背景
		//     if (m_pItemSkin != NULL)
		//         m_pItemSkin->Draw(pRT, rc, nBgImg);
		//     else if (CR_INVALID != crItemBg)
		//         pRT->FillSolidRect( rc, crItemBg);
		//上面的代码在某些时候，【指定skin的时候，会导致背景异常】，所以颠倒一下顺序
		if (CR_INVALID != crItemBg)//先画背景
			pRT->FillSolidRect(rcItem, crItemBg);

		if (m_pItemSkin != NULL)//有skin，则覆盖背景
			m_pItemSkin->Draw(pRT, rcItem, nBgImg);

		//  左边加上空白
		rcItem.left += ITEM_MARGIN;

		if (CR_INVALID != crText)
		{
			bTextColorChanged = TRUE;
			crOldText = pRT->SetTextColor(crText);
		}

		CRect rcCol(rcItem);
		rcCol.right = rcCol.left;
		rcCol.OffsetRect(-m_ptOrigin.x, 0);
		CRect rcNoMove(rcItem);
		rcNoMove.right = rcNoMove.left;
		int right = 0;
		bool bFirst = true;
		int nNoMoveCol = m_pHeader ? m_pHeader->GetNoMoveCol() : 0;
		for (int nCol = 0; nCol < GetColumnCount(); nCol++)
		{
			//if (m_pHeader)
			//	if (!m_pHeader->isItemShowVisble(nCol))
			//		continue;


			CRect rcVisiblePart;

			SHDITEM hdi;
			hdi.mask = SHDI_WIDTH | SHDI_ORDER;
			m_pHeader->GetItem(nCol, &hdi);
			if (m_pHeader->isItemShowVisble(nCol))
			{
				rcCol.left = rcCol.right;
				rcCol.right = rcCol.left + hdi.cx.toPixelSize(GetScale());
			}
			else
			{
				rcCol.left = rcCol.right;
				//rcCol.right = rcCol.left + hdi.cx.toPixelSize(GetScale());

			}
			//if (nNoMoveCol > 0)
			//{
				if (nCol < nNoMoveCol)
				{
					rcNoMove.left = rcNoMove.right;
					rcNoMove.right = rcNoMove.left + hdi.cx.toPixelSize(GetScale());
					CRect rcTmp(rcCol);
					rcCol = rcNoMove;
					rcNoMove = rcTmp;
					right = rcCol.right;
				}
				else if (rcCol.left < right) continue;
				else if (rcCol.left >= right && bFirst)
				{
					int  diff = rcCol.left - right;
					rcCol.left = right;
					rcCol.right -= diff;
					bFirst = false;
				}
			//}

			rcVisiblePart.IntersectRect(rcItem, rcCol);
			if (rcCol.right  > rcClient.right + ITEM_MARGIN) break;

			if (rcVisiblePart.IsRectEmpty())
				continue;

			// 绘制 checkbox
			if (nCol == 0 && m_bCheckBox && m_pCheckSkin)
			{
				CSize sizeSkin = m_pCheckSkin->GetSkinSize();
				int nOffsetX = 3;
				int nOffsetY = (m_nItemHeight - sizeSkin.cy) / 2;
				CRect rcCheck;
				rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
				rcCheck.OffsetRect(rcCol.left + nOffsetX, rcCol.top + nOffsetY);
				m_pCheckSkin->Draw(pRT, rcCheck, lvItem.checked ? 4 : 0);

				rcCol.left = sizeSkin.cx + 6 + rcCol.left;
			}

			DXLVSUBITEMEX& subItem = lvItem.arSubItems->GetAt(hdi.iOrder);

			if (subItem.nImage != -1 && m_pIconSkin)
			{
				int nOffsetX = m_ptIcon.x;
				int nOffsetY = m_ptIcon.y;
				CSize sizeSkin = m_pIconSkin->GetSkinSize();
				rcIcon.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);

				if (m_ptIcon.x == -1)
					nOffsetX = m_nItemHeight / 6;

				if (m_ptIcon.y == -1)
					nOffsetY = (m_nItemHeight - sizeSkin.cy) / 2;

				rcIcon.OffsetRect(rcCol.left + nOffsetX, rcCol.top + nOffsetY);
				m_pIconSkin->Draw(pRT, rcIcon, subItem.nImage);
			}

			UINT align = DT_SINGLELINE;
			align |= m_style.GetTextAlign();
			rcText = rcCol;
			rcText.right -= 5;

			if (m_ptText.x == -1)
				rcText.left = rcIcon.Width() > 0 ? rcIcon.right + m_nItemHeight / 6 : rcCol.left;
			else
				rcText.left = rcCol.left + m_ptText.x;

			if (m_ptText.y == -1)
				align |= DT_VCENTER;
			else
				rcText.top = rcCol.top + m_ptText.y;

			if (CR_INVALID != subItem.clrref)
			{
				pRT->SetTextColor(subItem.clrref);
			}
			else
			{
				pRT->SetTextColor(crText);
			}
			pRT->DrawText(subItem.strText, subItem.cchTextMax, rcText, align);

			if (nNoMoveCol > 0 && nCol < nNoMoveCol)
			{
				CRect rcTmp(rcCol);
				rcCol = rcNoMove;
				rcNoMove = rcTmp;
			}

		}

		if (bTextColorChanged)
			pRT->SetTextColor(crOldText);
	}

	void SColorListCtrlEx::OnDestroy()
	{
		DeleteAllItems();

		__super::OnDestroy();
	}

	int SColorListCtrlEx::GetColumnCount() const
	{
		if (!m_pHeader)
			return 0;

		return m_pHeader->GetItemCount();
	}

	int SColorListCtrlEx::GetTopIndex() const
	{
		return m_ptOrigin.y / m_nItemHeight;
	}

	void SColorListCtrlEx::NotifySelChange(int nOldSel, int nNewSel, BOOL checkBox)
	{
		EventLCSelChanging evt1(this);
		evt1.bCancel = FALSE;
		evt1.nOldSel = nOldSel;
		evt1.nNewSel = nNewSel;

		FireEvent(evt1);
		if (evt1.bCancel) return;

		if (checkBox) {
			DXLVITEMEX &newItem = m_arrItems[nNewSel];
			newItem.checked = newItem.checked ? FALSE : TRUE;
			m_nSelectItem = nNewSel;
			RedrawItem(nNewSel);
		}
		else {
			if ((m_bMultiSelection || m_bCheckBox) && GetKeyState(VK_CONTROL) < 0) {
				if (nNewSel != -1) {
					DXLVITEMEX &newItem = m_arrItems[nNewSel];
					newItem.checked = newItem.checked ? FALSE : TRUE;
					m_nSelectItem = nNewSel;
					RedrawItem(nNewSel);
				}
			}
			else if ((m_bMultiSelection || m_bCheckBox) && GetKeyState(VK_SHIFT) < 0) {
				if (nNewSel != -1) {
					if (nOldSel == -1)
						nOldSel = 0;

					int imax = (nOldSel > nNewSel) ? nOldSel : nNewSel;
					int imin = (imax == nOldSel) ? nNewSel : nOldSel;
					for (int i = 0; i < GetItemCount(); i++)
					{
						DXLVITEMEX &lvItem = m_arrItems[i];
						BOOL last = lvItem.checked;
						if (i >= imin && i <= imax) {
							lvItem.checked = TRUE;
						}
						else {
							lvItem.checked = FALSE;
						}
						if (last != lvItem.checked)
							RedrawItem(i);
					}
				}
			}
			else {
				m_nSelectItem = -1;
				for (int i = 0; i < GetItemCount(); i++)
				{
					DXLVITEMEX &lvItem = m_arrItems[i];
					if (i != nNewSel && lvItem.checked)
					{
						BOOL last = lvItem.checked;
						lvItem.checked = FALSE;
						if (last != lvItem.checked)
							RedrawItem(i);
					}
				}
				if (nNewSel != -1) {
					DXLVITEMEX &newItem = m_arrItems[nNewSel];
					newItem.checked = TRUE;
					m_nSelectItem = nNewSel;
					RedrawItem(nNewSel);
				}
			}
		}

		EventLCSelChanged evt2(this);
		evt2.nOldSel = nOldSel;
		evt2.nNewSel = nNewSel;
		FireEvent(evt2);
	}

	BOOL SColorListCtrlEx::OnScroll(BOOL bVertical, UINT uCode, int nPos)
	{
		BOOL bRet = __super::OnScroll(bVertical, uCode, nPos);

		if (bVertical)
		{
			m_ptOrigin.y = m_siVer.nPos;
		}
		else
		{
			m_ptOrigin.x = m_siHoz.nPos;
			//  处理列头滚动
			UpdateHeaderCtrl();
		}

		Invalidate();
		if (uCode == SB_THUMBTRACK)
			ScrollUpdate();

		return bRet;
	}

	void SColorListCtrlEx::OnLButtonDown(UINT nFlags, CPoint pt)
	{
		__super::OnLButtonDown(nFlags, pt);
		m_nHoverItem = HitTest(pt);
		BOOL hitCheckBox = HitCheckBox(pt);

		if (hitCheckBox)
			NotifySelChange(m_nSelectItem, m_nHoverItem, TRUE);
		else if (m_nHoverItem != m_nSelectItem && !m_bHotTrack)
			NotifySelChange(m_nSelectItem, m_nHoverItem);
		else if (m_nHoverItem != -1 || m_nSelectItem != -1)
			NotifySelChange(m_nSelectItem, m_nHoverItem);
		int nSelect = GetSelectedItem();
	}

	void SColorListCtrlEx::SetHoverSelected()
	{
		SetSelectedItem(m_nHoverItem);

	}

	void SColorListCtrlEx::OnLButtonDbClick(UINT nFlags, CPoint pt)
	{
		m_nHoverItem = HitTest(pt);
		if (m_nHoverItem != m_nSelectItem)
			NotifySelChange(m_nSelectItem, m_nHoverItem);

		EventLCDbClick evt2(this);
		evt2.nCurSel = m_nHoverItem;
		FireEvent(evt2);
	}

	void SColorListCtrlEx::OnLButtonUp(UINT nFlags, CPoint pt)
	{
		__super::OnLButtonUp(nFlags, pt);
	}


	void SColorListCtrlEx::UpdateChildrenPosition()
	{
		__super::UpdateChildrenPosition();
		UpdateHeaderCtrl();
	}

	void SColorListCtrlEx::OnSize(UINT nType, CSize size)
	{
		__super::OnSize(nType, size);
		UpdateScrollBar();
		UpdateHeaderCtrl();
	}

	bool SColorListCtrlEx::OnHeaderClick(EventArgs *pEvt)
	{
		return true;
	}

	bool SColorListCtrlEx::OnHeaderSizeChanging(EventArgs *pEvt)
	{
		UpdateScrollBar();
		InvalidateRect(GetListRect());

		return true;
	}

	bool SColorListCtrlEx::OnHeaderSwap(EventArgs *pEvt)
	{
		InvalidateRect(GetListRect());

		return true;
	}

	void SColorListCtrlEx::OnMouseMove(UINT nFlags, CPoint pt)
	{
		int nHoverItem = HitTest(pt);
		if (m_bHotTrack && nHoverItem != m_nHoverItem)
		{
			m_nHoverItem = nHoverItem;
			Invalidate();
		}
	}

	void SColorListCtrlEx::OnMouseLeave()
	{
		if (m_bHotTrack)
		{
			m_nHoverItem = -1;
			Invalidate();
		}
		__super::OnMouseLeave();
	}

	BOOL SColorListCtrlEx::GetCheckState(int nItem)
	{
		if (nItem >= GetItemCount())
			return FALSE;

		const DXLVITEMEX lvItem = m_arrItems[nItem];
		return lvItem.checked;
	}

	BOOL SColorListCtrlEx::SetCheckState(int nItem, BOOL bCheck)
	{
		if (!m_bCheckBox) return FALSE;

		if (nItem >= GetItemCount())
			return FALSE;

		DXLVITEMEX &lvItem = m_arrItems[nItem];
		lvItem.checked = bCheck;

		return TRUE;
	}

	int SColorListCtrlEx::GetCheckedItemCount()
	{
		int ret = 0;

		for (int i = 0; i < GetItemCount(); i++)
		{
			const DXLVITEMEX lvItem = m_arrItems[i];
			if (lvItem.checked)
				ret++;
		}

		return ret;
	}

	int SColorListCtrlEx::GetFirstCheckedItem()
	{
		int ret = -1;
		for (int i = 0; i < GetItemCount(); i++)
		{
			const DXLVITEMEX lvItem = m_arrItems[i];
			if (lvItem.checked) {
				ret = i;
				break;
			}
		}

		return ret;
	}

	int SColorListCtrlEx::GetLastCheckedItem()
	{
		int ret = -1;
		for (int i = GetItemCount() - 1; i >= 0; i--)
		{
			const DXLVITEMEX lvItem = m_arrItems[i];
			if (lvItem.checked) {
				ret = i;
				break;
			}
		}

		return ret;
	}

}//end of namespace 
