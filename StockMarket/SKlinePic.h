#ifndef _SKLINE_PIC
#define _SKLINE_PIC

#pragma once
#include <core/swnd.h>
#include "SKlineTip.h"
#include "DataProc.h"

namespace SOUI
{
	class CDealList;
	class CPriceList;
	class SSubPic;

	class SKlinePic : public SWindow
	{
		SOUI_CLASS_NAME(SKlinePic, L"klinePic")	//定义xml标签

	public:
		SKlinePic();
		~SKlinePic();

		void InitShowPara();
		void SetShowData(SStringA subIns, vector<CommonIndexMarket>* pIdxMarketVec);
		void SetShowData(SStringA subIns, vector<CommonStockMarket>* pStkMarketVec);




		int			m_nKWidth;		//两线之间的宽度
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nPreX;
		int			m_nPreY;

		int			m_nMaxKNum;		//能显示的k线最大数量
		int			m_nMove;		//正数为向右平移,0为现在
		int			m_nTickPre;		//鼠标上一次刷新时间
		int			m_nFirst;		//开始k线
		int			m_nEnd;			//结束k线

		bool		m_bNoJiange;
		bool		m_bReSetFirstLine;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
		bool		m_bClearTip;

		bool        m_bShowMA;			//显示MA
		bool        m_bShowBandTarget;	//显示波段优化交易指标






		int			*m_pGroupDataType;

		int			m_nPeriod;

		int			m_nBandCount;

		int			m_nMacdCount;

		SStringA	m_strSubIns;

		bool		m_bShowDeal;


		bool		m_bIsStockIndex;

		int			m_nTradingDay;




		//	int		ReInit(int nKwidth);		//重新进行初始化






		HWND		m_hParWnd;

		// 图形处理
	protected:
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint = FALSE);	//画时间纵轴
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		BOOL		IsInRect(int x, int y, int nMode = 0);	//是否在坐标中,0为全部,1为上方,2为下方
		SStringW	GetYPrice(int nY);
		SStringW	GetAllYPrice(int nY);
		void		DrawData(IRenderTarget * pRT);
		void		DrawTickMainData(IRenderTarget * pRT); //画主图tick
		void		DrawVolData(IRenderTarget * pRT);		//画附图vol
		int			GetXData(int nx); 	//获取鼠标下的数据id
		int			GetXPos(int nx);
		int			GetYPos(double fDiff);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnLButtonDown(UINT nFlags, CPoint point);
		void		OnTimer(char cTimerID);
		int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		void		OnMouseLeave();
		void		OnSize(UINT nType, CSize size);
		int			GetShowKlineNum(int nKwidth = 2);	//获取需要显示的k线数量

		void		DrawMainUpperMarket(IRenderTarget *pRT, KlineType& data);
		void		DrawMainUpperMA(IRenderTarget *pRT, int nPos);
		void		DrawMacdUpperMarket(IRenderTarget *pRT, int nPos);
		void		DrawMainUpperBand(IRenderTarget *pRT, int nPos);
		void		DrawBandData(IRenderTarget * pRT, int nDataPos, KlineType &Data, int x, int nJiange);
		void		DrawMouseKlineInfo(IRenderTarget * pRT, const KlineType &KlData, CPoint pt, const int &num = 0, const double &fPrePrice = 0);
		BOOL        ptIsInKlineRect(CPoint pt, int nDataCount, KlineType &data);





		void		GetFuTuMaxDiff();		//判断副图坐标最大最小值和k线条数
		int			GetFuTuYPos(double fDiff);	//获得附图y位置
		SStringW	GetFuTuYPrice(int nY);		//获得附图y位置价格

		void		GetMACDMaxDiff();
		int			GetMACDYPos(double fDiff);	//获得MACD图y位置
		SStringW	GetMACDYPrice(int nY);		//获得MACD图y位置价格

		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE);
		void		DrawKeyDownLine(IRenderTarget* pRT, bool ClearTip = false);
		void		DrawMouseLine(IRenderTarget * pRT, CPoint p);
		void		DrawMoveTime(IRenderTarget * pRT, int x, int date, int time, bool bNew);
		void		DrawMovePrice(IRenderTarget * pRT, int y, bool bNew);
		void		DrawBarInfo(IRenderTarget * pRT, int nDataPos);


		int			m_nMouseLinePos;
		CPoint		m_preMovePoint;


		//控件响应
	protected:
		void		OnDbClickedKline(UINT nFlags, CPoint point);
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		// 数据处理
	protected:
		void DataInit();
		void DataProc();
		void UpdateData();

		bool GenerateMultiMinFromOne(int nCount, KlineType & data, int nPeriod);

		void SingleDataProc();
		void SingleDataWithHis();
		void SingleDataUpdate();

		void StockMarket1MinUpdate();
		void StockMarketMultMinUpdate(int nPeriod);
		void StockMarketDayUpdate();
		void StockTickToKline(int nCount, CommonStockMarket & tick, bool bNewLine = false, int time = 0);
		void StockTickToDayKline(int nCount, CommonStockMarket & tick);

		void IndexMarket1MinUpdate();
		void IndexMarketMultMinUpdate(int nPeriod);
		void IndexMarketDayUpdate();
		void IndexTickToKline(int nCount, CommonIndexMarket & tick, bool bNewLine = false, int time = 0);
		void IndexTickToDayKline(int nCount, CommonIndexMarket & tick);

		bool ProcKlineTime(int & time);

		void KlineMAProc(int nCount);
		void BandDataUpdate();
		void MACDDataUpdate();
		int ProcBandTargetData(int nPos, Band_t *BandData);
		int	ProcMACDData(int nPos, MACDData_t  *MacdData);
		void ReProcKlineData(bool bSingleNeedProc = false);
		void ReProcMAData();

		//波段优化辅助处理函数
	protected:
		double		GetHighPrice(int n, int nPeriod, int nOffset = 0);
		double		GetLowPrice(int n, int nPeriod, int nOffset = 0);
		int			Count(double a[], double b[], int nType, int n, int nPeriod);
		bool		Cross(double a[], double b[], int nPos);
		int			ValueWhen(int a[], int b[], int nPos);


		//数据
	protected:
		AllKPIC_INFO *m_pAll;
		MACDData_t  *m_pMacdData;
		Band_t		*m_pBandData;
		size_t			m_nUsedTickCount;
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;
		bool			m_bAddDay;	//添加日线

		//调用类
	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		CDataProc	m_dataHandler;
		SKlineTip*  m_pTip;
	public:
		SSubPic*	m_pSubPic;

		//显示和计算参数
	protected:
		BandPara_t  m_BandPara;
		InitPara_t  m_InitPara;
		int			m_nMAPara[4];
		int			m_nMACDPara[3];
		bool		m_bDataInited;

		//绘图参数
	protected:
		CRect       m_rcAll;		//上下框相加
		CRect		m_rcUpper;		//上框坐标,K线
		CRect		m_rcLower;		//下框坐标,指标
		CRect       m_rcLower2;		//下框2坐标，指标2
		CRect		m_rcImage;
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey, m_penPurple,
			m_penYellow, m_penMAGreen, m_penGray, m_penDotGreen, m_penDotYellow, m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack, m_bBrushDarkBlue;
		BOOL    m_bPaintInit;	//绘制项目初始化

	protected:
		CRITICAL_SECTION m_cs;

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			//MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			//			MSG_WM_LBUTTONDBLCLK(OnDbClickedKline)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			//			MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_CREATE(OnCreate)
			//			MSG_WM_SIZE(OnSize)
			SOUI_MSG_MAP_END()
	};
}
#endif // !_SKLINE_PIC