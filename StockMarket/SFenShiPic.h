
#pragma once
#include <core/swnd.h>


#define MOVE_ONESTEP		10		//每次平移的数据量
#define K_WIDTH_TOTAL		16		//k线占用总宽度(在x轴上)
//一些基本的界面框架信息
#define RC_FSLEFT		50
#define RC_FSRIGHT		50
#define RC_FSTOP		25
#define RC_FSBOTTOM		40

#define RC_FSMAX		5
#define RC_FSMIN		5

#include<set>
#include<map>
#include"DataProc.h"


//class CDataProc;
namespace SOUI
{
	class CDealList;
	class CPriceList;
	class SSubPic;

	class SFenShiPic : public SWindow
	{
		SOUI_CLASS_NAME(SFenShiPic, L"fenshiPic")	//定义xml标签

		// 数据接口
	public:
		SFenShiPic();
		~SFenShiPic();

		void SetShowData(SStringA subIns, vector<CommonIndexMarket>* pIdxMarketVec);
		void SetShowData(SStringA subIns, vector<CommonStockMarket>* pStkMarketVec);
		void InitShowPara();

		int			m_nAllLineNum;			//分时图条目总数(必须维持)

		int			m_nIndex;
		int			m_nOldIndex;
		int			m_nCount;

		bool		m_bShowMouseLine;
		bool		m_bKeyDown;

		int			m_nMiddle;
		int			m_nHeight;
		int			m_nMin;



		int			m_nEMAPara[2];
		int			m_nMACDPara[3];

		int			m_nTradingDay;


		FENSHI_INFO *m_pData;
		std::set<int> m_timeSet;
		std::map<int, SStringW> m_virTimeLineMap;

		CRect       m_rcAll;		//上下框相加
		CRect		m_rcUpper;		//上框坐标,K线
		CRect		m_rcLower;		//下框坐标,指标
		CRect		m_rcMACD;		//MACD指标框
		CRect		m_rcFutNStcok;
		CRect		m_rcImage;

		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nMaxKNum;		//能显示的k线最大数量
		int			m_nTickPre;		//鼠标上一次刷新时间

		int			m_nKeyX;
		int			m_nKeyY;

		int			m_nLastVolume;	//上一个时段结束的交易量


		InitPara_t  m_InitPara;

		int			m_nNowPosition;		//现在的数据线的位置

		bool		m_bIsStockIndex;

		bool		m_bIsFirstKey;

		//bool		m_bIsPart;


		//	int			m_nSubIns;

		SStringA    m_strSubIns;

		CPoint		m_preMovePt;

		bool		m_bDataInited;


		int			m_nPaintTick;

		void		ReProcMacd();
		void		UpdateData();
		void		ReProcEMA();



		int			m_nWndNum;

		//图形处理
	protected:
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawData(IRenderTarget * pRT);
		void		DrawVirtualTimeLine(IRenderTarget * pRT);
		void		DrawUpperMarket(IRenderTarget * pRT, FENSHI_GROUP &data);
		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE);
		void		DrawEarserLine(IRenderTarget * pRT, CPoint rc, bool bVertical);
		void		DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar);

		//图形相关数据初始化和计算获取
	protected:
		void		DataInit();
		void		SingleInit();
		void		SetShowTime();
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		int			GetXPos(int n);		//获取id对应的x坐标
		int			GetXData(int nx); 	//获取鼠标下的数据id
		int			GetYPos(double fDiff);
		BOOL		IsInRect(int x, int y, int nMode = 0);	//是否在坐标中,0为全部,1为上方,2为下方
		SStringW	GetYPrice(int nY, BOOL bIsPercent);
		void		GetFuTuMaxDiff();			//判断副图坐标最大最小值和k线条数
		int			GetFuTuYPos(double fDiff);	//获得附图y位置
		SStringW	GetFuTuYPrice(int nY);		//获得附图y位置价格
		int			GetMACDYPos(double fDiff);	//获得MACD图y位置
		SStringW	GetMACDYPrice(int nY);		//获得MACD图y位置价格
		void        InitVirTimeLineMap();
		void		GetMACDMaxDiff();			//判断MACD指标图中最大值最小值
		void		GetMacdDiff();
		void		SetWindowRect();
		COLORREF	GetColor(double dPrice);

		//消息响应
	protected:
		void		OnTimer(char cTimerID);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnMouseLeave();
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void		OnDbClickedFenshi(UINT nFlags, CPoint point);

		//数据处理
	protected:

		//CAutoRefPtr<IRenderTarget> m_pRTBuffer;
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;
		void		DataProc();
		void		StockDataUpdate();
		void		IndexDataUpdate();
		void		SetFSData(FENSHI_GROUP &f, CommonIndexMarket & market);
		void		SetFSData(FENSHI_GROUP &f, CommonStockMarket & market);
		void		SetGapMarketTime(FENSHI_GROUP &f, int time);
		void		DeleteLastData();
		bool		TimeInGap(int time);
		bool		TimeAfterGap(int time);
		bool		TimeAfterGapHandle(int time, FENSHI_GROUP &f);
		int			SetFenshiMin(int nTime, bool bSetData = false);
		void		MACDHandle(FENSHI_GROUP &f1, int nOffset = 0);
		void		HandleNoDataTime(FENSHI_GROUP f1);
		void		HandleMissData(FENSHI_GROUP f1, int time);//补全遗漏的数据

		//显示开关
	protected:
		BOOL		m_bPaintInit;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
		bool		m_bShowAvg;
		bool		m_bShowEMA;
		bool		m_bShowDeal;

	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		SSubPic*    m_pSubPic;	//rps图
		CDataProc   m_dataHandler;




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
}
