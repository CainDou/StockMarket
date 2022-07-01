
#pragma once
#include "Define.h"
#include <core/swnd.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <map>


using std::unordered_map;
using std::string;
using std::vector;
using std::map;


#define MOVE_ONESTEP		10		//ÿ��ƽ�Ƶ�������
#define K_WIDTH_TOTAL		16		//k��ռ���ܿ��(��x����)
//һЩ�����Ľ�������Ϣ
#define RC_FSLEFT		50
#define RC_FSRIGHT		40
#define RC_FSTOP		25
#define RC_FSBOTTOM		40

#define RC_FSMAX		5
#define RC_FSMIN		5

typedef struct InStockData
{
	double close;
	DWORD	time;
}InStockData_t;

typedef struct InsStockAllData
{
	std::vector<InStockData> d;
	DWORD	nTime;		//���������һ��ʱ��
	int		nMin;		//��ʱ��ʱ��
	int		nCount;
}InsStockAllData_t;



namespace SOUI
{

	class SSubPic : public SWindow
	{
		SOUI_CLASS_NAME(SSubPic, L"subPic")	//����xml��ǩ

	public:
		SSubPic();
		~SSubPic();

		void		SetShowData(int nIndex,bool nGroup);
		void		SetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec,vector<SStringA> dataNameVec,
			SStringA StockID ,SStringA StockName);
		void		OnDbClickedFenshi(UINT nFlags, CPoint point);
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void		ReSetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec);
		SStringA	GetShowStock() const;
		void		SetOffset2Zero();

	protected:
		void		InitColorAndPen(IRenderTarget *pRT);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		GetMaxDiff();		//�ж����������Сֵ��k������
		BOOL		IsInRect(int x, int y, int nMode = 0);	//�Ƿ���������,0Ϊȫ��,1Ϊ�Ϸ�,2Ϊ�·�
		SStringW	GetYPrice(int nY, BOOL bIsRight);
		void		DrawData(IRenderTarget * pRT);
		int			GetXPos(int n);		//��ȡid��Ӧ��x����
		int			GetXData(int nx); 	//��ȡ����µ�����id
		int			GetYPos(double fDiff, BOOL bIsRight);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnTimer(char cTimerID);
		int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		void		OnMouseLeave();
		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str,COLORREF color=RGBA(255,255,255,255),UINT uFormat=DT_SINGLELINE);
		void		DrawEarserLine(IRenderTarget * pRT, CPoint rc, bool bVertical);
		void		HandleMissData(InStockData f1, int time);
		void		DataInit();
		void		DrawKeyDownMouseLine(IRenderTarget * pRT/*,UINT nChar*/);
		void		DrawTime(IRenderTarget * pRT, CRect rc, int date ,int time);
		void		DrawMouseData(IRenderTarget * pRT, int xPos);
	protected:

		int			m_nOffset;
		int			m_nFirst;
		int			m_nEnd;
		CRect		m_rcImage;
		double		m_fMaxL;
		double		m_fMinL;
		double		m_fMaxR;
		double		m_fMinR;
		double		m_fDeltaL;
		double		m_fDeltaR;
		vector<CoreData> **m_pData;
		vector<SStringA> m_dataNameVec;
		vector<BOOL> m_bRightArr;
		vector<COLORREF> m_colorVec;
		vector<CAutoRefPtr<IPen>>	m_penVec;
		BOOL		m_bPaintInit;
		SStringA	m_StockName;
		SStringA	m_StockID;
		int			m_nPaintTick;
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nTickPre;		//�����һ��ˢ��ʱ��
		int			m_nKeyX;
		int			m_nKeyY;
		int			m_nNowPosition;		//���ڵ������ߵ�λ��
		int			m_nAllLineNum;			//��ʱͼ��Ŀ����(����ά��)
		int			m_nIndex;
		int			m_nCount;
		int			m_nMiddle;
		int			m_nHeight;
		int			m_nShowDataCount;
		bool		m_bDataInited;
		bool		m_bIsFirstKey;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;

	public:
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
//			MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_LBUTTONDBLCLK(OnDbClickedFenshi)
			SOUI_MSG_MAP_END()

	};

	inline SStringA SOUI::SSubPic::GetShowStock() const
	{
		return m_StockID;
	}

	inline void SOUI::SSubPic::SetOffset2Zero()
	{
		m_nOffset = 0;
	}
}


