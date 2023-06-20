#include "souistd.h"
#include "stdafx.h"
#include "SHeaderCtrlEx.h"
#include <vector>
#include <algorithm>
#include "helper/DragWnd.h"

using std::vector;
#define ITEM_MARGIN 1


SHeaderCtrlEx::SHeaderCtrlEx()
{
	m_bInitText = false;
	m_nNoMoveCol = 0;
	nOffset = 0;
}


SHeaderCtrlEx::~SHeaderCtrlEx()
{
}

bool SHeaderCtrlEx::SetOriItemIndex(int nCol, int iOrder)
{
	//if (nCol == iOrder)
	//	return true;
	int nItemCount = m_arrItems.GetCount();
	if (nCol >= nItemCount || iOrder >= nItemCount)
		return false;
	//if (!m_bInitText)
	//	InitText();
	m_arrItems[nCol].iOrder = iOrder;
	m_arrItems[nCol].strText = m_arrSrcItem[iOrder].strText;
	m_arrItems[nCol].cx = m_arrSrcItem[iOrder].cx;
	return true;
}

bool SHeaderCtrlEx::SetNoMoveCol(int nCount)
{
	m_nNoMoveCol = nCount;
	return true;
}


bool SHeaderCtrlEx::SetOffSet(int offset)
{
	nOffset = offset;
	return true;
}

void SHeaderCtrlEx::InitText()
{
	int nItemCount = m_arrItems.GetCount();
	for (int i = 0; i < nItemCount; ++i)
	{
		m_arrSrcItem.InsertAt(i, m_arrItems[i]);
		m_arrVisble.InsertAt(i, m_arrItems[i].bVisible);
	}
	m_bInitText = true;

}

int SHeaderCtrlEx::InsertItem(int iItem, LPCTSTR pszText, int nWidth, SHDSORTFLAG stFlag, LPARAM lParam)
{
	return InsertItem(iItem, pszText, nWidth, SLayoutSize::px, stFlag, lParam);
}

int SHeaderCtrlEx::InsertItem(int iItem, LPCTSTR pszText, int nWidth, SLayoutSize::Unit unit, SHDSORTFLAG stFlag, LPARAM lParam)
{
	SASSERT(pszText);
	SASSERT(nWidth >= 0);
	if (iItem == -1) iItem = (int)m_arrItems.GetCount();
	SHDITEM item;
	item.mask = 0xFFFFFFFF;
	item.cx.setSize((float)nWidth, unit);
	item.strText.SetCtxProvider(this);
	item.strText.SetText(pszText);
	item.stFlag = stFlag;
	item.state = 0;
	item.iOrder = iItem;
	item.lParam = lParam;
	m_arrItems.InsertAt(iItem, item);
	m_arrSrcItem.InsertAt(iItem, item);
	m_arrVisble.InsertAt(iItem, item.bVisible);

	//需要更新列的序号
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		if (i == (size_t)iItem) continue;
		if (m_arrItems[i].iOrder >= iItem)
			m_arrItems[i].iOrder++;
		if (m_arrSrcItem[i].iOrder >= iItem)
			m_arrSrcItem[i].iOrder++;

	}

	for (size_t i = 0; i < GetItemCount(); i++)
	{
		if (i == (size_t)iItem) continue;
		if (m_arrItems[i].iOrder >= iItem)
			m_arrItems[i].iOrder++;
	}

	EventHeaderRelayout e(this);
	FireEvent(e);

	Invalidate();
	return iItem;
}

void SHeaderCtrlEx::OnPaint(IRenderTarget * pRT)
{
	SPainter painter;
	BeforePaint(pRT, painter);
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcItem(rcClient.left, rcClient.top, rcClient.left, rcClient.bottom);
	CRect rcNoMove(rcClient.left, rcClient.top, rcClient.left, rcClient.bottom);
	rcItem.right = rcItem.left;
	rcItem.OffsetRect(nOffset, 0);
	rcNoMove.right = rcNoMove.left;
	bool bFirst = true;
	int right = rcItem.left;
	//if (!m_bInitText)
	//	InitText();

	for (UINT i = 0; i < m_arrItems.GetCount(); i++)
	{
		if (!m_arrVisble[i])continue;

		m_arrItems[i].bVisible = true;
		rcItem.left = rcItem.right;
		rcItem.right = rcItem.left + m_arrItems[i].cx.toPixelSize(GetScale());
		rcNoMove.left = rcNoMove.right;
		rcNoMove.right = rcNoMove.left + m_arrItems[i].cx.toPixelSize(GetScale());
		if (i < m_nNoMoveCol)
		{
			CRect rcTmp(rcItem);
			rcItem = rcNoMove;
			rcNoMove = rcTmp;
			right = rcItem.right;
		}
		else if (rcItem.left < right)
		{
			m_arrItems[i].bVisible = false;
			continue;
		}
		else if (rcItem.left >= right && bFirst)
		{
			int  diff = rcItem.left - right;
			rcItem.left = right;
			rcItem.right -= diff;
			bFirst = false;
		}
		if (rcItem.right > rcClient.right) break;
		DrawItem(pRT, rcItem, m_arrItems.GetData() + i);
		if (i < m_nNoMoveCol)
		{
			CRect rcTmp(rcItem);
			rcItem = rcNoMove;
			rcNoMove = rcTmp;
		}
	}

	//if (m_nNoMoveCol > 0)
	//{
	//}
	//else
	//{
	//	for (UINT i = 0; i < m_arrItems.GetCount(); i++)
	//	{
	//		if (!m_arrVisble[i]) continue;
	//		m_arrItems[i].bVisible = true;
	//		rcItem.left = rcItem.right;
	//		rcItem.right = rcItem.left + m_arrItems[i].cx.toPixelSize(GetScale());
	//		DrawItem(pRT, rcItem, m_arrItems.GetData() + i);
	//		if (rcItem.right >= rcClient.right) break;
	//	}
	//}
	if (rcItem.right > rcClient.right)
		DrawItem(pRT, rcItem, m_arrItems.GetData());
	AfterPaint(pRT, painter);
}

void SHeaderCtrlEx::DrawItem(IRenderTarget * pRT, CRect rcItem, const LPSHDITEM pItem)
{
	if (!pItem->bVisible) return;
	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_pSkinItem) m_pSkinItem->Draw(pRT, rcItem, pItem->state);
	if (rcItem.right > rcClient.right)
	{
		pRT->DrawText(L"...",3, rcItem, DT_LEFT);
		return;
	}

	UINT uTextAlign = m_style.GetTextAlign();
	if (m_bMultiLines)
		uTextAlign ^= DT_SINGLELINE;

	pRT->DrawText(pItem->strText.GetText(FALSE), pItem->strText.GetText(FALSE).GetLength(), rcItem, uTextAlign);
	if (pItem->stFlag == ST_NULL || !m_pSkinSort) return;
	CSize szSort = m_pSkinSort->GetSkinSize();
	CPoint ptSort;
	ptSort.y = rcItem.top + (rcItem.Height() - szSort.cy) / 2;

	if (uTextAlign&DT_RIGHT)
		ptSort.x = rcItem.left + 2;
	else
		ptSort.x = rcItem.right - szSort.cx - 2;

	if (m_pSkinSort) m_pSkinSort->Draw(pRT, CRect(ptSort, szSort), pItem->stFlag == ST_UP ? 0 : 1);
}

void SHeaderCtrlEx::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (IsItemHover(m_dwHitTest))
	{
		if (m_bDragging)
		{//拖动表头项
			if (m_bItemSwapEnable)
			{
				CDragWnd::EndDrag();
				DeleteObject(m_hDragImg);
				m_hDragImg = NULL;

				m_arrItems[LOWORD(m_dwHitTest)].state = 0;//normal

				if (m_dwDragTo != m_dwHitTest && IsItemHover(m_dwDragTo))
				{
					//SHDITEM t = m_arrItems[LOWORD(m_dwHitTest)];
					//m_arrItems.RemoveAt(LOWORD(m_dwHitTest));
					//int nPos = LOWORD(m_dwDragTo);
					//if (nPos > LOWORD(m_dwHitTest)) nPos--;//要考虑将自己移除的影响
					//m_arrItems.InsertAt(LOWORD(m_dwDragTo), t);
					int nHit = LOWORD(m_dwHitTest);
					int nDragTo = LOWORD(m_dwDragTo);
					SHDITEM tHit = m_arrItems[LOWORD(m_dwHitTest)];
					SHDITEM tDragTo = m_arrItems[LOWORD(m_dwDragTo)];
					SetOriItemIndex(LOWORD(m_dwHitTest), tDragTo.iOrder);
					SetOriItemIndex(LOWORD(m_dwDragTo), tHit.iOrder);

					//int nTmpOrder = tHit.iOrder;
					//tHit.iOrder = tDragTo.iOrder;
					//tDragTo.iOrder = nTmpOrder;

					//发消息通知宿主表项位置发生变化
					EventHeaderItemSwap evt(this);
					evt.iOldIndex = LOWORD(m_dwHitTest);
					evt.iNewIndex = LOWORD(m_dwDragTo);
					FireEvent(evt);

					EventHeaderRelayout e(this);
					FireEvent(e);
				}
				m_dwHitTest = HitTest(pt);
				m_dwDragTo = (DWORD)-1;
				Invalidate();
			}
		}
		else
		{//点击表头项
			if (m_bSortHeader)
			{
				m_arrItems[LOWORD(m_dwHitTest)].state = 1;//hover
				RedrawItem(LOWORD(m_dwHitTest));
				EventHeaderClick evt(this);
				evt.iItem = LOWORD(m_dwHitTest);
				FireEvent(evt);
			}
		}
	}
	else if (m_dwHitTest != -1)
	{//调整表头宽度，发送一个调整完成消息
		EventHeaderItemChanged evt(this);
		evt.iItem = LOWORD(m_dwHitTest);
		evt.nWidth = m_arrItems[evt.iItem].cx.toPixelSize(GetScale());
		FireEvent(evt);

		EventHeaderRelayout e(this);
		FireEvent(e);
	}
	m_bDragging = FALSE;
	ReleaseCapture();

}

void SHeaderCtrlEx::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bDragging || nFlags&MK_LBUTTON)
	{
		if (!m_bDragging)
		{
			if (IsItemHover(m_dwHitTest) && m_bItemSwapEnable &&LOWORD(m_dwHitTest) >= m_nNoMoveCol)
			{
				m_dwDragTo = m_dwHitTest;
				CRect rcItem = GetItemRect(LOWORD(m_dwHitTest));
				DrawDraggingState(m_dwDragTo);
				m_hDragImg = CreateDragImage(LOWORD(m_dwHitTest));
				CPoint pt = m_ptClick - rcItem.TopLeft();
				CDragWnd::BeginDrag(m_hDragImg, pt, 0, 128, LWA_ALPHA | LWA_COLORKEY);
				m_bDragging = TRUE;
			}
		}
		if (IsItemHover(m_dwHitTest))
		{
			if (m_bItemSwapEnable && LOWORD(m_dwHitTest) >= m_nNoMoveCol)
			{
				DWORD dwDragTo = HitTest(pt);
				CPoint pt2(pt.x, m_ptClick.y);
				ClientToScreen(GetContainer()->GetHostHwnd(), &pt2);
				if (IsItemHover(dwDragTo) && m_dwDragTo != dwDragTo)
				{
					m_dwDragTo = dwDragTo;
					DrawDraggingState(dwDragTo);
				}
				CDragWnd::DragMove(pt2);
			}
		}
		else if (m_dwHitTest != -1)
		{//调节宽度
			if (!m_bFixWidth)
			{
				int cxNew = m_nAdjItemOldWidth + pt.x - m_ptClick.x;
				if (cxNew < 0) cxNew = 0;
				if (m_arrItems[LOWORD(m_dwHitTest)].cx.unit == SLayoutSize::px)
					m_arrItems[LOWORD(m_dwHitTest)].cx.setSize((float)cxNew, SLayoutSize::px);
				else if (m_arrItems[LOWORD(m_dwHitTest)].cx.unit == SLayoutSize::dp)
					m_arrItems[LOWORD(m_dwHitTest)].cx.setSize(cxNew * 1.0f / GetScale(), SLayoutSize::dp);
				// TODO: dip 和 sp 的处理（AYK）

				Invalidate();
				GetContainer()->UpdateWindow();//立即更新窗口
											   //发出调节宽度消息
				EventHeaderItemChanging evt(this);
				evt.iItem = LOWORD(m_dwHitTest);
				evt.nWidth = cxNew;
				FireEvent(evt);

				EventHeaderRelayout e(this);
				FireEvent(e);
			}
		}
	}
	else
	{
		DWORD dwHitTest = HitTest(pt);
		if (dwHitTest != m_dwHitTest)
		{
			if (m_bSortHeader)
			{
				if (IsItemHover(m_dwHitTest))
				{
					WORD iHover = LOWORD(m_dwHitTest);
					m_arrItems[iHover].state = 0;
					//RedrawItem(iHover);
				}
				if (IsItemHover(dwHitTest))
				{
					WORD iHover = LOWORD(dwHitTest);
					m_arrItems[iHover].state = 1;//hover
					//RedrawItem(iHover);
				}
			}
			m_dwHitTest = dwHitTest;
		}
	}

}

void SHeaderCtrlEx::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	if (m_bDragging)
	{
		if (m_bSortHeader && m_dwHitTest != -1)
		{
			m_arrItems[LOWORD(m_dwHitTest)].state = 0;//normal
		}
		m_dwHitTest = (DWORD)-1;

		CDragWnd::EndDrag();
		DeleteObject(m_hDragImg);
		m_hDragImg = NULL;
		m_bDragging = FALSE;
		ReleaseCapture();
		Invalidate();
	}
}



void SHeaderCtrlEx::RedrawItem(int iItem)
{
	CRect rcItem = GetItemRect(iItem);
	IRenderTarget *pRT = GetRenderTarget(rcItem, OLEDC_PAINTBKGND);
	DrawItem(pRT, rcItem, m_arrItems.GetData() + iItem);
	ReleaseRenderTarget(pRT);
}

CRect SHeaderCtrlEx::GetItemRect(UINT iItem) const
{
	CRect    rcClient;
	GetClientRect(&rcClient);
	if (!m_arrItems[iItem].bVisible)
		return CRect();
	CRect rcItem(rcClient.left, rcClient.top, rcClient.left, rcClient.bottom);
	CRect rcNoMove(rcClient.left, rcClient.top, rcClient.left, rcClient.bottom);
	rcItem.right = rcItem.left;
	rcItem.OffsetRect(nOffset, 0);
	rcNoMove.right = rcNoMove.left;
	int right;
	bool bFirst = true;
	for (UINT i = 0; i <= iItem && i < m_arrItems.GetCount(); i++)
	{
		if (!m_arrItems[i].bVisible) continue;
		rcItem.left = rcItem.right;
		rcItem.right = rcItem.left + m_arrItems[i].cx.toPixelSize(GetScale());
		rcNoMove.left = rcNoMove.right;
		rcNoMove.right = rcNoMove.left + m_arrItems[i].cx.toPixelSize(GetScale());
		if (i < m_nNoMoveCol)
		{
			CRect rcTmp(rcItem);
			rcItem = rcNoMove;
			rcNoMove = rcTmp;
			right = rcItem.right;
		}
		else if (rcItem.left < right)
		{
			continue;
		}
		else if (rcItem.left >= right && bFirst)
		{
			int  diff = rcItem.left - right;
			rcItem.left = right;
			rcItem.right -= diff;
			bFirst = false;
		}
		if (rcItem.right > rcClient.right) break;

	}
	return rcItem;
}

bool SHeaderCtrlEx::isItemShowVisble(UINT iItem)
{
	if (iItem >= m_arrVisble.GetCount())
		return false;
	return m_arrVisble[iItem];
}

void SHeaderCtrlEx::SetItemShowVisible(int iItem, bool visible)
{
	SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
	//if (!m_bInitText)
	//	InitText();
	m_arrItems[iItem].bVisible = visible;
	m_arrVisble[iItem] = visible;
	Invalidate();
	//发出调节宽度消息
	EventHeaderItemChanged evt(this);
	evt.iItem = iItem;
	evt.nWidth = GetItemWidth(iItem, false);
	FireEvent(evt);
}

int SHeaderCtrlEx::GetTotalWidth()
{
	int nRet = 0;
	for (UINT i = 0; i < m_arrVisble.GetCount(); i++)
	{
		if (m_arrVisble[i])
			nRet += GetItemWidth(i, true);
	}
	return nRet;
}

int SHeaderCtrlEx::GetItemWidth(int iItem, bool bFroced)
{
	if (iItem < 0 || (UINT)iItem >= m_arrItems.GetCount()) return -1;
	if (!bFroced)
		if (!m_arrItems[iItem].bVisible) return 0;
	return m_arrItems[iItem].cx.toPixelSize(GetScale());
}
