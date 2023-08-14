
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
#define RC_FSRIGHT		50
#define RC_FSTOP		25
#define RC_FSBOTTOM		40

#define RC_FSMAX		5
#define RC_FSMIN		5

#define RC_KLLEFT		0
#define RC_KLRIGHT		48

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

	class SSubTargetPic
	{

	public:
		SSubTargetPic();
		~SSubTargetPic();

		void		SetSubPicInfo(ShowPointInfo showInfo);
		void		SetSubTitleInfo(SStringA strTitle);
		ShowPointInfo GetSubPicInfo();
		void		SetShowData(int nIndex, bool nGroup);
		void		SetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec, vector<SStringA> dataNameVec,
			SStringA StockID, SStringA StockName);
		void		OnDbClicked(UINT nFlags, CPoint point);
		void		ReSetShowData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec);
		SStringA	GetShowStock() const;
		void		SetOffset2Zero();
		CRect*		GetPicRect();
		void		SetPicRect(CRect rc);
		void		Paint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawPrice(IRenderTarget * pRT);
		void		DrawMovePrice(IRenderTarget * pRT, int y, bool bNew);
		void		InitColorAndPen(IRenderTarget *pRT);
		void		DrawData(IRenderTarget * pRT);
		void		SetMouseLineState(bool bShow);
		void		SetMousePosDefault();
		void		SetMouseMove();
		void		SetNowKeyDownLinePos(int nPos);
		void		SetShowWidth(int width, int nJiange,int nZoomRatio = 1);
		void		SetOffset(int nOffset);
		void		SetShowNum(int nNum);
		void		DrawKeyDownMouseLine(IRenderTarget * pRT,BOOL bDoubleFlash = FALSE);
		BOOL		CheckIsSelectClicked(CPoint pt);
	protected:
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		BOOL		IsInRect(int x, int y);	
		SStringW	GetYPrice(int nY, BOOL bIsRight);
		int			GetXPos(int n);		//获取id对应的x坐标
		int			GetXData(int nx); 	//获取鼠标下的数据id
		int			GetYPos(double fDiff, BOOL bIsRight);
		void		OnTimer(char cTimerID);
		int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255), 
			UINT uFormat = DT_SINGLELINE,int nSize=12, DWORD rop = SRCINVERT);
		void		DrawEarserLine(IRenderTarget * pRT, CPoint rc, bool bVertical);
		void		HandleMissData(InStockData f1, int time);
		void		DrawMouseData(IRenderTarget * pRT, int xPos);
		
	protected:
		int			m_nOffset;
		int			m_nFirst;
		int			m_nEnd;
		int			m_nZoomRatio;
		CRect		m_rcImage;
		double		m_fMaxL;
		double		m_fMinL;
		double		m_fMaxR;
		double		m_fMinR;
		double		m_fDeltaL;
		double		m_fDeltaR;
		SStringW	m_strTitle;
		ShowPointInfo m_ShowInfo;
		vector<CoreData> **m_pData;
		vector<SStringA> m_dataNameVec;
		vector<BOOL> m_bRightArr;
		vector<COLORREF> m_colorVec;
		vector<std::pair<int, int>>* m_pDateTimeVec;
		vector<CAutoRefPtr<IPen>>	m_dotPenVec;
		vector<CAutoRefPtr<IPen>>	m_penVec;
		vector<CAutoRefPtr<IBrush>>	m_brushVec;
		vector<vector<double>> m_dataVec;
		vector<int>m_dataFigureType;
		vector<COLORREF>m_UsedColor;
		vector<int> m_dataColorType;
		vector<vector<BOOL>>m_judgeVec;

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
		int			m_nWidth;
		int			m_nJiange;
		int			m_nShowDataCount;
		bool		m_bDataInited;
		bool		m_bIsFirstKey;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bUseWidth;
		CRect		m_rcTargetSel;
		BOOL		m_bPenInit;
		int			nRectWidth;
	};

	inline void SSubTargetPic::SetSubPicInfo(ShowPointInfo showInfo)
	{
		m_ShowInfo = showInfo;
	}

	inline void SSubTargetPic::SetSubTitleInfo(SStringA strTitle)
	{
		m_strTitle = StrA2StrW(strTitle);
	}

	inline ShowPointInfo SSubTargetPic::GetSubPicInfo()
	{
		return m_ShowInfo;
	}

	inline SStringA SSubTargetPic::GetShowStock() const
	{
		return m_StockID;
	}

	inline void SSubTargetPic::SetOffset2Zero()
	{
		m_nOffset = 0;
	}
	inline CRect * SSubTargetPic::GetPicRect()
	{
		return &m_rcImage;
	}
	inline void SSubTargetPic::SetPicRect(CRect rc)
	{
		m_rcImage = rc;
	}
	inline void SSubTargetPic::SetMouseLineState(bool bShow)
	{
		m_bShowMouseLine = bShow;
	}
	inline void SSubTargetPic::SetMousePosDefault()
	{
		m_nMouseX = m_nMouseY = -1;
	}
	inline void SSubTargetPic::SetMouseMove()
	{
		m_bIsFirstKey = true;
		m_bKeyDown = false;
	}
	inline void SSubTargetPic::SetNowKeyDownLinePos(int nPos)
	{
		m_nNowPosition = nPos;
	}
	inline void SSubTargetPic::SetShowWidth(int nWidth, int nJiange,int nZoomRatio)
	{
		 m_nWidth= nWidth;
		 m_nJiange = nJiange;
		 m_nZoomRatio = nZoomRatio;
		 m_bUseWidth = true;
	}
	inline void SSubTargetPic::SetOffset(int nOffset)
	{
		m_nOffset = nOffset;
	}
	inline void SSubTargetPic::SetShowNum(int nNum)
	{
		m_nAllLineNum = nNum;
	}
}


