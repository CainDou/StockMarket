#include "stdafx.h"
#include "SHeaderCtrlEx.h"
#include <vector>
#include <algorithm>

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
	if (nCol == iOrder)
		return true;
	int nItemCount = m_arrItems.GetCount();
	if (nCol >= nItemCount || iOrder >= nItemCount)
		return false;
	if (!m_bInitText)
	{
		for (int i = 0; i < nItemCount; ++i)
		{
			m_arrSrcItem.InsertAt(i, m_arrItems[i]);
			m_arrVisble.InsertAt(i, m_arrItems[i].bVisible);
		}
		m_bInitText = true;
	}
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
	if (!m_bInitText)
	{
		for (int i = 0; i < m_arrItems.GetCount(); ++i)
		{
			m_arrSrcItem.InsertAt(i, m_arrItems[i]);
			m_arrVisble.InsertAt(i, m_arrItems[i].bVisible);
		}
		m_bInitText = true;
	}
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
	if (rcItem.right < rcClient.right)
	{
		rcItem.left = rcItem.right;
		rcItem.right = rcClient.right;
		if (m_pSkinItem) m_pSkinItem->Draw(pRT, rcItem, 3);
	}
	AfterPaint(pRT, painter);
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
	m_arrItems[iItem].bVisible = visible;
	m_arrVisble[iItem] = visible;
	Invalidate();
	//发出调节宽度消息
	EventHeaderItemChanged evt(this);
	evt.iItem = iItem;
	evt.nWidth = GetItemWidth(iItem);
	FireEvent(evt);
}
