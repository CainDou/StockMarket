#pragma once
#include <core/swnd.h>
#include <memory>
#include <set>
namespace SOUI
{
	class CDealList;
	class CPriceList;
	class BrickPic : public SWindow
	{
		SOUI_CLASS_NAME(BrickPic, L"brickPic")	//定义xml标签
	public:
		BrickPic();
		~BrickPic();
		void		SetShowIns(SStringA strIns, SStringA StockName);
		void		InitShowPara(const InitPara_t& para);
		void		OutPutShowPara(InitPara_t& para);
		void		SetBrickPara(int nType, double fSetting);
		void		SetDataPoint(vector<CommonStockMarket>* pStkMarketVec,
			vector<RenkoData>* pHisRenkoVec);

		void		DataProc();
		void		ReProcData();
		void		UpdateData();
		void		ReProcKlineRehabData(eRehabType rehabType);
		void		ReProcKlineRehabData(FixedTimeRehab& frt);

		void		SetRehabInfo(vector<RehabInfo>& rehabVec);
		void		SetTodayMarketState(bool bReady);
		void		SetHisRenkoState(bool bReady);
		bool		GetDataReadyState();
		eRehabType	GetRehabType();
		void		SetPriceListHalf(bool bHalf);
		bool		GetDealState() const;
		void		SetDealState(bool bRevesered = true, bool bState = false);
		void		SetPicUnHandled();
		int			GetBrickType();
		double		GetBrickSetting();
	protected:
		void		OnDbClicked(UINT nFlags, CPoint point);
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	protected:
		void		DataInit();
		void		RenkoDataWithHis();
		void		RenkoDataUpdate();
		void 		ReSetData(int nCount);
		void		AddData(const RenkoData& data, int64_t time);
		void		AddData(double fPrice, int64_t time, bool bJump);
		void		AddBrickDataToEmpty(const RenkoData& data, int64_t time);
		double		GetBrickStep(double fClose) const;
		void		UpdatePrice(double fPrice, int64_t time, double& fUpper, double& fLower,
			double& fUpperStart, double& fLowerStart, bool bJump);
		void		ChangeLastDataTime(int64_t time);
		RenkoData FrontRehabCash(RenkoData& srcKline, int nDate = 0);
		RenkoData FrontRehabReInv(RenkoData& srcKline, int nDate = 0);
		RenkoData BackRehabCash(RenkoData& srcKline, int nDate = 0);
		RenkoData BackRehabReInv(RenkoData& srcKline, int nDate = 0);

		double FrontRehabCash(double fPrice, int nDate = 0);
		double FrontRehabReInv(double fPrice, int nDate = 0);
		double BackRehabCash(double fPrice, int nDate = 0);
		double BackRehabReInv(double fPrice, int nDate = 0);

	protected:
		void		CalcShowRange();
		void        SetPicBarWidth(int nPicWidth, int nDataCount);
		void		OnPaint(IRenderTarget* pRT);
		void		DrawArrow(IRenderTarget* pRT);
		void		DrawMouse(IRenderTarget* pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawTime(IRenderTarget* pRT, BOOL bFromOnPaint = FALSE);	//画时间纵轴
		void		GetMaxDiff();		//判断坐标最大最小值和k线条数
		BOOL		IsInRect(int x, int y, int nMode = 0);	//是否在坐标中,0为全部,1为上方,2为下方
		SStringW	GetYPrice(int nY);
		void		DrawData(IRenderTarget* pRT);
		int			GetXData(int nx); 	//获取鼠标下的数据id
		int			GetXPos(int nx);
		int			GetYPos(double fDiff);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnMouseLeave();
		void		OnSize(UINT nType, CSize size);
		void		DrawTextonPic(IRenderTarget* pRT, CRect rc, SStringW str,
			COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE, DWORD rop = SRCINVERT);
		void		DrawKeyDownLine(IRenderTarget* pRT, bool ClearTip = false);
		void		DrawMouseLine(IRenderTarget* pRT, CPoint p);
		CRect		GetTextDrawRect(IRenderTarget* pRT, SStringW str, CRect rc);
		void		DrawMoveTime(IRenderTarget* pRT, int x, int date, int time, bool bNew);
		void        DrawPrice(IRenderTarget* pRT);
		void		DrawMovePrice(IRenderTarget* pRT, int y, bool bNew);
		void		DrawBarInfo(IRenderTarget* pRT, int nDataPos);
		void		DrawUpperMarket(IRenderTarget* pRT, int nX);
		void		DrawBrick(IRenderTarget* pRT, int nPos, int nX);


	protected:
		std::unique_ptr<CPriceList> m_pPriceList;
		std::unique_ptr<CDealList>  m_pDealList;
		BOOL    m_bPaintInit;	//绘制项目初始化
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey, m_penPurple, m_penYellow,
			m_penMAGreen, m_penGray, m_penDotGreen, m_penDotYellow, m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack, m_bBrushDarkBlue;
		CRect       m_rcAll;		//上下框相加
		CRect		m_rcImage;
		CRect		m_rcMain;

		bool		m_bShowDeal;
		std::vector<int64_t> m_DateTimeVec;
		std::set<int> m_BrickJumpPos;
		CPoint m_PtMouse;
		int m_nLeftMouseMove;
		bool m_bDataInited;
		bool m_bShowMouseLine;
		bool m_bReSetFirstLine;
		bool m_bKeyDown;
		int m_nType;
		bool m_bMini;
		double m_fPriceTick;
		int m_nBrickType;
		int m_nBrickStart;
		double m_fBrickSetting;
		double m_fPreStart;
		int m_nKeyX;
		int m_nKeyY;
		int m_nDataCount; //当前K线的数据数量
		int m_nNowDataPos; //当前K线的数据id
		int m_nLastDataPos; //上一条K线的数据id
		int m_nNowPosition; //当前鼠标所在的K线位置
		double m_fZoomRatio; //缩放比例
		int m_nKWidth; //K线宽度
		int m_nMaxKNum;
		int m_nEnd;
		int m_nFirst;
		int m_nGap;
		int m_nMove;
		int m_nUsedTickCount;
		int64_t m_nLastTickTime;
		int m_nGroupDataType;
		bool m_bTodayMarketReady;
		bool m_bHisRenkoReady;
		bool		m_bHalfPrice;

	protected:

		SStringA	m_strSubIns;
		SStringA	m_strStockName;
		int				m_nTradingDay;
		HWND		m_hParWnd;
		OperVec Open;
		OperVec High;
		OperVec Low;
		OperVec Close;
		vector<RehabInfo> m_RehabInfo;
		vector<CommonStockMarket>* m_pStkMarketVec;
		vector<RenkoData>* m_pHisRenkoVec;
		int	 m_nFTRehabTime;
		eRehabType m_rehabType;		//用于显示的复权类型
		eRehabType m_calcRehabType;	//用于计算的复权类型
		double m_fMaxY;
		double m_fMinY;
		int m_nMaxVol;
		int m_nMinVol;
		int m_nDecimal;
		bool m_bTodayFirst;

	protected:
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_LBUTTONDBLCLK(OnDbClicked)
			MSG_WM_CREATE(OnCreate)
			SOUI_MSG_MAP_END()


	};
	inline void BrickPic::SetBrickPara(int nType, double fSetting)
	{
		m_nBrickType = nType;
		m_fBrickSetting = fSetting;
	}
	inline void BrickPic::SetRehabInfo(vector<RehabInfo>& rehabVec)
	{
		m_RehabInfo = rehabVec;
	}

	inline void BrickPic::SetTodayMarketState(bool bReady)
	{
		m_bTodayMarketReady = bReady;
		if (!m_bTodayMarketReady)
			m_bDataInited = false;
	}

	inline void BrickPic::SetHisRenkoState(bool bReady)
	{
		m_bHisRenkoReady = bReady;
		if (!m_bHisRenkoReady)
			m_bDataInited = false;
	}
	inline bool BrickPic::GetDataReadyState()
	{
		return m_bTodayMarketReady && m_bHisRenkoReady;
	}
	inline eRehabType BrickPic::GetRehabType()
	{
		return m_rehabType;
	}
	inline bool BrickPic::GetDealState() const
	{
		return m_bShowDeal;
	}
	inline void BrickPic::SetDealState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowDeal = !m_bShowDeal;
		else m_bShowDeal = bState;
	}

	inline void BrickPic::SetPicUnHandled()
	{
		m_bDataInited = false;
	}

	inline int BrickPic::GetBrickType()
	{
		return m_nBrickType;
	}

	inline double BrickPic::GetBrickSetting()
	{
		return m_fBrickSetting;
	}

}



