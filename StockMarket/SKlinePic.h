#ifndef _SKLINE_PIC
#define _SKLINE_PIC

#pragma once
#include <core/swnd.h>
#include "SKlineTip.h"
namespace SOUI
{
	class CDealList;
	class CPriceList;

	class SKlinePic : public SWindow
	{
		SOUI_CLASS_NAME(SKlinePic, L"klinePic")	//定义xml标签

	public:
		SKlinePic();
		~SKlinePic();

		void		InitShowPara();
		AllKPIC_INFO *m_pAll;

		MACDData_t  *m_pMacdData;

		CRect       m_rcAll;		//上下框相加
		CRect		m_rcUpper;		//上框坐标,K线
		CRect		m_rcLower;		//下框坐标,指标
		CRect       m_rcLower2;		//下框2坐标，指标2
		CRect		m_rcImage;

		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;


		int			m_nCAType;		//显示集合竞价信息

		bool		m_bNoJiange;

		double		m_fPreSettlePrice;

		//	int		m_nCyc;			//按键缩放
		int			m_nKWidth;		//两线之间的宽度
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nKeyX;
		int			m_nKeyY;

		int			m_nMaxKNum;		//能显示的k线最大数量
		int			m_nMove;		//正数为向右平移,0为现在
		bool		m_bHover;
		int			m_nTickPre;		//鼠标上一次刷新时间
		int			m_nFirst;		//开始k线
		int			m_nEnd;			//结束k线

		bool		bPaintAfetrDataProc;

		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
	
		InitPara_t  m_InitPara;
		bool        m_bShowHighLow;		//显示高低加
		bool        m_bShowMA;			//显示MA
		bool        m_bShowBandTarget;	//显示波段优化交易指标


		void		BandDataUpdate();

		void		MACDDataUpdate();

		void		DrawBandData(IRenderTarget * pRT,int nDataPos, DATA_FOR_SHOW &Data,int x,int nJiange);

		void		DrawMouseKlineInfo(IRenderTarget * pRT, const DATA_FOR_SHOW &KlData, CPoint pt, const int &num = 0, const double &fPrePrice = 0);

		void		DataProc();

		void		ReProcMAData();

		enumKMode	m_nLineSym;

		Band_t		*m_pBandData;
		BandPara_t  m_BandPara;
		int			m_nMACDPara[3];
		int			m_nMAPara[4];

		int			*m_pGroupDataType;

		int			m_nKlineType;	//0是一分钟，1是5分钟，2是15分钟，3是30分钟，4是60分钟，5是日线

		int			m_nPeriod;

		int			m_nBandCount;

		int			m_nMacdCount;

		//int			m_nSubIns;
		InsIDType	m_strSubIns;

		bool		m_bShowDeal;

		bool		m_bSubInsisGroup;

		bool		m_bIsStockIndex;

		int				m_nGroupVolume;			//用来保存历史1分钟K线数据的最后一个
		int				m_nGroupTime;			//用来保存历史1分钟K线最后一根不完整的时间
		bool			m_bGroupIsWhole;		//历史1分钟K线最后一根是否完整

		int				m_nTradingDay;
		std::vector<GroupKlineType> m_Group1MinKline;							//历史组合合约1分钟K线
		std::vector<GroupKlineType> m_GroupDayKline;							//历史组合合约日K线


		BOOL    m_bPaintInit;	//绘制项目初始化
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey,m_penPurple,m_penYellow,m_penMAGreen,m_penGray,m_penDotGreen,m_penDotYellow,m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack,m_bBrushDarkBlue;


		//	int		ReInit(int nKwidth);		//重新进行初始化
		int			GetShowKlineNum(int nKwidth = 2);	//获取需要显示的k线数量
		//	void	SetLowerData(float *p[], BYTE *bft);													//副图信息
		//	void	SetUpperMainZhiBiao(float *p[], bool* bShow);											//主图主信息指标
		//	void	SetUpperExtraData(DATA_FOR_SHOW *p, int nDecimal, int nDecimalXi);						//主图副信息
		//	void	SetUpperMainData(DATA_FOR_SHOW *p, int nTotal, int nDecimal, int nDecimalXi);			//主图主信息

		void		OnKeyDown(UINT nChar);
		void		OnDbClickedKline(UINT nFlags, CPoint point);

		bool		m_bIsPart;


		bool		m_bDataInited;

		bool		m_bNeedAddCount;
		int 		ProcBandTargetData(int nPos,Band_t *BandData);

		int			ProcMACDData(int nPos, MACDData_t  *MacdData);

		void		ReProcKlineData(bool bSingleNeedProc = false);

		void		UpdateData();

		SKlineTip*  m_pTip;

		HWND		m_hParWnd;

	protected:
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawTime(IRenderTarget * pRT,  BOOL bFromOnPaint=FALSE);	//画时间纵轴
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		BOOL		IsInRect(int x, int y, int nMode = 0);	//是否在坐标中,0为全部,1为上方,2为下方
		SStringW	GetYPrice(int nY);
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

		BOOL        ptIsInKlineRect(CPoint pt,int nDataCount,DATA_FOR_SHOW &data );
		BOOL		IsCATime(int nTime,int nDate);
		BOOL        IsFirstLine(int nTime);
		void		FileDataToSingle(bool bPreLineIsCA,const KLineDataType &data, int nCount,bool bMutiMin);

		void		AddCADataToFirstLine(KLineDataType &CALine, KLineDataType &FirstLine);

		bool		m_bHandleTdyFirstLine;

		CRITICAL_SECTION m_cs;



		void		GetFuTuMaxDiff();		//判断副图坐标最大最小值和k线条数
		int			GetFuTuYPos(double fDiff);	//获得附图y位置
		SStringW	GetFuTuYPrice(int nY);		//获得附图y位置价格

		void		GetMACDMaxDiff();
		int			GetMACDYPos(double fDiff);	//获得MACD图y位置
		SStringW	GetMACDYPrice(int nY);		//获得MACD图y位置价格

		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255),UINT uFormat=DT_SINGLELINE);
		void		DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar);

		double		GetHighPrice(int n,int nPeriod,int nOffset=0);
		double		GetLowPrice(int n, int nPeriod,int nOffset=0);
		int			Count(double a[], double b[], int nType, int n, int nPeriod);
		bool		Cross(double a[],double b[],int nPos);
		int			ValueWhen(int a[],int b[],int nPos);



		//单个合约的K线数据处理
		void DataInit();

		void SingleDataProc();
		void SingleDataWithHis();
		void SingleDataNoHis();
		void SingleDataUpdate();

		void SingleFutures1MinUpdate();
		void SingleIndex1MinUpdate();
		void SingleFuturesMultMinUpdate(int nPeriod);
		void SingleIndexMultMinUpdate(int nPeriod);
		void SingleFuturesDayUpdate();
		void SingleIndexDayUpdate();


		void GroupToData( int nCount, const GroupKlineType &kline, int nTimeOffset = 0);
		void GroupUpdateWithHad( int nCount, const GroupKlineType &kline);
		void GroupDayUpdateWithTick( int nCount, const GroupDataType &tick);
		void GroupUpdateWithTick( int nCount, std::vector<GroupDataType>* pTickVec, int nPos, bool bNewLine = true);

		void GroupUpdateWithKline( int nCount, GroupKlineType &kline);
		void GroupUpdateWithKline( int nCount, std::vector<GroupKlineType>* pKlineVec, int nPos);
		GroupKlineType m_Last1MinHisData;

		void GroupDataProc();
		void GroupDataWithHis();
		void GroupDataNoHis();
		void GroupDataUpdate();

		void Group1MinUpdate();
		void GroupMultMinUpdate(int nPeriod);
		void GroupDayUpdate();

		void GroupFirstKlineProc(InsIDType GroupInsID,bool bHisIsWhole, int nPos);
		void GroupFirstKlineProc(InsIDType GroupInsID, bool bHisIsWhole, const GroupKlineType& KlineData);

		int  GroupCAInfoProc(int nCount, bool bHandleVolume=true, bool bHandlePrice=true);

		void GroupHisKline1MinProc();			//历史1分钟K线合成
		void GroupHisDayKlineProc();

		//在没有实时K线的情况下更新K线
		void GroupUpdateNoKline( int nCount, int nPos, std::vector<GroupDataType>* pTickVec, int ntime, bool bAdd, bool bNewLine = true);



		void KlineMAProc( int nCount);



		bool TimeIsLastBar(int UpdateTime, int LastBarTime, int TickTime,int nPeriod);
		//组合合约的K线数据处理


		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
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