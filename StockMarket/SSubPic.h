
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


#define MOVE_ONESTEP		10		//每次平移的数据量
#define K_WIDTH_TOTAL		16		//k线占用总宽度(在x轴上)
//一些基本的界面框架信息
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
	DWORD	nTime;		//计算后的最后一个时间
	int		nMin;		//此时的时间
	int		nCount;
}InsStockAllData_t;



namespace SOUI
{

	class SSubPic : public SWindow
	{
		SOUI_CLASS_NAME(SSubPic, L"subPic")	//定义xml标签

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
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		BOOL		IsInRect(int x, int y, int nMode = 0);	//是否在坐标中,0为全部,1为上方,2为下方
		SStringW	GetYPrice(int nY, BOOL bIsRight);
		void		DrawData(IRenderTarget * pRT);
		int			GetXPos(int n);		//获取id对应的x坐标
		int			GetXData(int nx); 	//获取鼠标下的数据id
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
		int			m_nMaxKNum;		//能显示的k线最大数量
		int			m_nTickPre;		//鼠标上一次刷新时间
		int			m_nKeyX;
		int			m_nKeyY;
		int			m_nNowPosition;		//现在的数据线的位置
		int			m_nAllLineNum;			//分时图条目总数(必须维持)
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


