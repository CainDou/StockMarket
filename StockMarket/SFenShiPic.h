
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


//class CDataProc;
namespace SOUI
{
	class CDealList;
	class CPriceList;
	class SSubTargetPic;

	class SFenShiPic : public SWindow
	{
		SOUI_CLASS_NAME(SFenShiPic, L"fenshiPic")	//定义xml标签

		// 数据接口
	public:
		SFenShiPic();
		~SFenShiPic();
		void		InitSubPic(int nNum/*,vector<SStringA> & picNameVec*/);
		void		ReSetSubPic(int nNum, vector<ShowPointInfo>& infoVec);
		vector<ShowPointInfo> GetSubPicDataToGet(int nNum, map<int, ShowPointInfo>& infoMap);
		void		SetDataPoint(vector<CommonIndexMarket>* pIdxMarketVec);
		void		SetDataPoint(vector<CommonStockMarket>* pStkMarketVec,
					vector<TickFlowMarket>* pTFMarketVec);
		void		ChangeShowStock(SStringA subIns, SStringA StockName);
		void		SetSubPicShowData(int nIndex, bool nGroup);
		void		SetSubPicShowData(int nDataCount[],
					vector<vector<vector<CoreData>*>>& data,
					vector<vector<BOOL>> bRightVec,
					vector<vector<SStringA>>dataNameVec, SStringA StockID,
					SStringA StockName,int nStartWnd = 0);
		void		SetSubPicShowData(int nDataCount,
			vector<vector<CoreData>*>& data,
			vector<BOOL> bRightVec,
			vector<SStringA> dataNameVec, SStringA StockID,
			SStringA StockName);

		void		SetRpsGroup(RpsGroup rg);
		void		InitShowPara(InitPara_t para);
		void		OutPutShowPara(InitPara_t &para);
		void		DataProc();
		void		ReProcMacd();
		void		UpdateData();
		void		ReProcEMA();
		bool		GetDealState() const;
		bool		GetVolumeState() const;
		bool		GetMacdState() const;
		//bool		GetRpsState(int nWndNum) const;
		bool		GetAvgState() const;
		bool		GetEmaState() const;
		void		SetDealState(bool bRevesered = true,bool bState = false);
		void		SetVolumeState(bool bRevesered = true, bool bState = false);
		void		SetMacdState(bool bRevesered = true, bool bState = false);
		//void		SetRpsState(int nWndNum, bool bRevesered = true,
		//			bool bState = false);
		void		SetAvgState(bool bRevesered = true, bool bState = false);
		void		SetEmaState(bool bRevesered = true, bool bState = false);
		void		SetEmaPara(int EmaPara[]);
		void		SetMacdPara(int MacdPara[]);
		const int*	GetEmaPara();
		const int*  GetMacdPara();
		void		SetPicUnHandled();
		int			GetShowSubPicNum() const;
		void		SetPriceListHalf(bool bHalf);
		void		SetBelongingIndy(vector<SStringA>& strNameVec,int nStartWnd=0);
		void		GetShowPointInfo(vector<ShowPointInfo> &infoVec);
		BOOL		CheckTargetSelectIsClicked(CPoint pt);
		void		CloseSinglePointWnd();
		void		SetSelPointWndInfo(ShowPointInfo& info,SStringA strTitle);
		//static RpsGroup GetClickGroup();
		//图形处理
	protected:
		//virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawData(IRenderTarget * pRT);
		void		DrawVirtualTimeLine(IRenderTarget * pRT);
		void		DrawUpperMarket(IRenderTarget * pRT, FENSHI_GROUP &data);
		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str,
					COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE,
					DWORD rop = SRCINVERT);
		CRect		GetTextDrawRect(IRenderTarget * pRT, SStringW str, CRect rc);
		void		DrawEarserLine(IRenderTarget * pRT, CPoint rc, bool bVertical);
		void		DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar);
		void		DrawPrice(IRenderTarget * pRT);
		void		DrawMovePrice(IRenderTarget * pRT, int y, bool bNew);
		void		DrawMoveTime(IRenderTarget * pRT, int x, int date, int time, bool bNew);
		void		DrawMouseLine(IRenderTarget * pRT, CPoint po);

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
		//LRESULT		OnMsg(UINT uMsg, WPARAM wp, LPARAM lp);
		void		OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void		OnTimer(char cTimerID);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnMouseLeave();
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void		OnDbClickedFenshi(UINT nFlags, CPoint point);
		//数据处理
	protected:

		//CAutoRefPtr<IRenderTarget> m_pRTBuffer;
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
		//显示开关和设置
	protected:
		BOOL		m_bPaintInit;
		bool		m_bDataInited;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
		bool		m_bShowAvg;
		bool		m_bShowEMA;
		bool		m_bShowDeal;
		//BOOL*		m_pbShowSubPic;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bIsIndex;
		bool		m_bIsFirstKey;

		//具体显示参数
	protected:
		CRect       m_rcAll;		//上下框相加
		CRect		m_rcMain;		//上框坐标,K线
		CRect		m_rcVolume;		//下框坐标,指标
		CRect		m_rcMACD;		//MACD指标框
		CRect		m_rcImage;
		CPoint		m_preMovePt;
		std::set<int> m_timeSet;
		std::map<int, SStringW> m_virTimeLineMap;
		int			m_nAllLineNum;			//分时图条目总数(必须维持)
		int			m_nIndex;
		int			m_nOldIndex;
		int			m_nCount;
		int			m_nMiddle;
		int			m_nHeight;
		int			m_nMin;
		int			m_nEMAPara[2];
		int			m_nMACDPara[3];
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nMaxKNum;		//能显示的k线最大数量
		int			m_nTickPre;		//鼠标上一次刷新时间
		int			m_nKeyX;
		int			m_nKeyY;
		int			m_nNowPosition;		//现在的数据线的位置
		int			m_nPaintTick;
		bool		m_bHalfPrice;

		//调用子类
	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		SSubTargetPic**    m_ppSubPic;	//rps图
		int			m_nSubPicNum;
		int			m_nChangeNum;
		//数据
	protected:
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;
		vector<TickFlowMarket>* m_pTFMarketVec;
		FENSHI_INFO *m_pData;
		SStringA    m_strSubIns;
		SStringA	m_strStockName;
		SStringA	m_strL1Indy;
		SStringA	m_strL2Indy;
		int			m_nTradingDay;
		int			m_nLastVolume;	//上一个时段结束的交易量
		RpsGroup	m_rgGroup;
		CRITICAL_SECTION m_csSub;
	//private:
	//	static RpsGroup m_rgClickGroup;

		SOUI_MSG_MAP_BEGIN()
			//MESSAGE_HANDLER_EX(WM_FENSHI_MSG, OnMsg)
			//COMMAND_RANGE_HANDLER_EX(FM_Return,FM_End,OnMenuCmd)
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			//MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_LBUTTONDBLCLK(OnDbClickedFenshi)
			//MSG_WM_RBUTTONUP(OnRButtonUp)
			SOUI_MSG_MAP_END()
	};

	inline void SFenShiPic::SetRpsGroup(RpsGroup rg)
	{
		m_rgGroup = rg;
	}

	inline bool SFenShiPic::GetDealState() const
	{
		return m_bShowDeal;
	}
	inline bool SFenShiPic::GetVolumeState() const
	{
		return m_bShowVolume;
	}
	inline bool SFenShiPic::GetMacdState() const
	{
		return m_bShowMacd;
	}
	//inline bool SFenShiPic::GetRpsState(int nWndNum) const
	//{
	//	return m_pbShowSubPic[nWndNum];
	//}
	inline bool SFenShiPic::GetAvgState() const
	{
		return m_bShowAvg;
	}
	inline bool SFenShiPic::GetEmaState() const
	{
		return m_bShowEMA;
	}
	inline void SFenShiPic::SetDealState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowDeal = !m_bShowDeal;
		else m_bShowDeal = bState;
	}
	inline void SFenShiPic::SetVolumeState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowVolume = !m_bShowVolume;
		else m_bShowVolume = bState;
		if (!m_bShowVolume) m_rcVolume.SetRectEmpty();
	}
	inline void SFenShiPic::SetMacdState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowMacd = !m_bShowMacd;
		else m_bShowMacd = bState;
		if (!m_bShowMacd) m_rcMACD.SetRectEmpty();

	}
	//inline void SFenShiPic::SetRpsState(int nWndNum,bool bRevesered, bool bState)
	//{
	//	if (bRevesered) m_pbShowSubPic[nWndNum] = !m_pbShowSubPic[nWndNum];
	//	else m_pbShowSubPic[nWndNum] = bState;
	//}
	inline void SFenShiPic::SetAvgState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowAvg = !m_bShowAvg;
		else m_bShowAvg = bState;
	}
	inline void SFenShiPic::SetEmaState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowEMA = !m_bShowEMA;
		else m_bShowEMA = bState;
	}
	inline void SFenShiPic::SetEmaPara(int EmaPara[])
	{
		memcpy_s(m_nEMAPara, sizeof(m_nEMAPara), EmaPara, sizeof(m_nEMAPara));
	}
	inline void SFenShiPic::SetMacdPara(int MacdPara[])
	{
		memcpy_s(m_nMACDPara, sizeof(m_nMACDPara), MacdPara, sizeof(m_nMACDPara));
	}
	inline const int * SFenShiPic::GetEmaPara()
	{
		return m_nEMAPara;
	}
	inline const int * SFenShiPic::GetMacdPara()
	{
		return m_nMACDPara;
	}
	inline void SFenShiPic::SetPicUnHandled()
	{
		m_bDataInited = false;
	}
	inline int SFenShiPic::GetShowSubPicNum() const
	{
		return m_nSubPicNum;
	}


	//inline RpsGroup SFenShiPic::GetClickGroup()
	//{
	//	return m_rgClickGroup;
	//}
}
