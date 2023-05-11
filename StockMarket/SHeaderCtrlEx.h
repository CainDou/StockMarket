#pragma once
#include <control/SHeaderCtrl.h>
class SHeaderCtrlEx :
	public SHeaderCtrl
{
	SOUI_CLASS_NAME(SHeaderCtrlEx, L"headerex")

public:
	SHeaderCtrlEx();
	~SHeaderCtrlEx();

	bool SetOriItemIndex(int nCol, int iOrder);
	bool SetNoMoveCol(int nCount);
	bool SetOffSet(int offset);
	int GetNoMoveCol();
	void RedrawItem(int iItem);
	CRect GetItemRect(UINT iItem) const;
	bool isItemShowVisble(UINT iItem);
	void SetItemShowVisible(int iItem, bool visible);
	int GetTotalWidth();
	int GetItemWidth(int iItem, bool bFroced);
	
protected:
	void InitText();
	virtual void OnPaint(IRenderTarget * pRT);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnActivateApp(BOOL bActive, DWORD dwThreadID);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_ACTIVATEAPP(OnActivateApp)
		SOUI_MSG_MAP_END()

protected:
	SArray<SHDITEM> m_arrSrcItem;
	int m_nNoMoveCol;
	int nOffset;
	bool m_bInitText;
	SArray<BOOL> m_arrVisble;
};

inline int SHeaderCtrlEx::GetNoMoveCol()
{
	return m_nNoMoveCol;
}

