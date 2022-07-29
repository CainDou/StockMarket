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
	class SSubTargetPic;

	class SKlinePic : public SWindow
	{
		SOUI_CLASS_NAME(SKlinePic, L"klinePic")	//����xml��ǩ

	public:
		SKlinePic();
		~SKlinePic();


		//���ݽӿ�
		void		InitSubPic(int nNum, vector<SStringA> & picNameVec);
		void		InitShowPara(InitPara_t para);
		void		OutPutShowPara(InitPara_t& para);
		void		SetShowData(SStringA subIns, SStringA StockName, vector<CommonIndexMarket>* pIdxMarketVec,
					map<int, vector<KlineType>>*pHisKlineMap);
		void		SetShowData(SStringA subIns, SStringA StockName, vector<CommonStockMarket>* pStkMarketVec,
					map<int, vector<KlineType>>*pHisKlineMap);
		void		SetSubPicShowData(int nIndex, bool nGroup);
		void		SetSubPicShowData(int nDataCount[],
			vector<vector<vector<CoreData>*>>& data,
			vector<vector<BOOL>> bRightVec,
			vector<vector<SStringA>>, SStringA StockID,
			SStringA StockName);
		void		ReSetSubPicData(int nDataCount, vector<CoreData>* data[], vector<BOOL>& bRightVec);
		void		SetParentHwnd(HWND hParWnd);
		void		SetTodayMarketState(bool bReady);
		void		SetHisKlineState(bool bReady);
		void		SetHisPointState(bool bReady);
		bool		GetDataReadyState();
		void        ClearTip();
		//LRESULT		OnMsg(UINT uMsg, WPARAM wp, LPARAM lp);
		void		SetRpsGroup(RpsGroup rg);
		void		DataProc();
		void		UpdateData();
		void		ReProcMAData();
		void		ReProcMacdData();
		void		ReProcBandData();
		void		ChangePeriod(int nPeriod,BOOL bNeedReCalc);
		bool		GetDealState() const;
		bool		GetVolumeState() const;
		bool		GetMacdState() const;
		bool		GetRpsState(int nWndNum) const;
		bool		GetMaState() const;
		bool		GetBandState() const;
		void		SetDealState(bool bRevesered = true, bool bState = false);
		void		SetVolumeState(bool bRevesered = true, bool bState = false);
		void		SetMacdState(bool bRevesered = true, bool bState = false);
		void		SetRpsState(int nWndNum,
					bool bRevesered = true, bool bState = false);
		void		SetMaState(bool bRevesered = true, bool bState = false);
		void		SetBandState(bool bRevesered = true, bool bState = false);
		void		SetBandPara(BandPara_t& bandPara);
		void		SetMaPara(int maPara[]);
		void		SetMacdPara(int MacdPara[]);
		const int*  GetMaPara();
		const int*  GetMacdPara();
		BandPara_t	GetBandPara();
		void		SetPicUnHandled();
		int			GetShowSubPicNum() const;
		void		SetBelongingIndy(vector<SStringA>& strNameVec);

		// ͼ�δ�������
	protected:
		//virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint = FALSE);	//��ʱ������
		void		DrawData(IRenderTarget * pRT);
		void		DrawTickMainData(IRenderTarget * pRT); //����ͼtick
		void		DrawVolData(IRenderTarget * pRT);		//����ͼvol
		void		DrawMainUpperMarket(IRenderTarget *pRT, KlineType& data);
		void		DrawMainUpperMA(IRenderTarget *pRT, int nPos);
		void		DrawMacdUpperMarket(IRenderTarget *pRT, int nPos);
		void		DrawMainUpperBand(IRenderTarget *pRT, int nPos);
		void		DrawBandData(IRenderTarget * pRT, int nDataPos, KlineType &Data, int x, int nJiange);
		void		DrawMouseKlineInfo(IRenderTarget * pRT, const KlineType &KlData, CPoint pt, const int &num = 0, const double &fPrePrice = 0);
		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE);
		void		DrawKeyDownLine(IRenderTarget* pRT, bool ClearTip = false);
		void		DrawMouseLine(IRenderTarget * pRT, CPoint p);
		void		DrawMoveTime(IRenderTarget * pRT, int x, int date, int time, bool bNew);
		void		DrawMovePrice(IRenderTarget * pRT, int y, bool bNew);
		void		DrawBarInfo(IRenderTarget * pRT, int nDataPos);

		//ͼ�β�������
	protected:
		void		SetWindowRect();
		int			GetShowKlineNum(int nKwidth = 2);	//��ȡ��Ҫ��ʾ��k������
		BOOL		IsInRect(int x, int y, int nMode = 0);	//�Ƿ���������,0Ϊȫ��,1Ϊ�Ϸ�,2Ϊ�·�
		void		GetMaxDiff();		//�ж����������Сֵ��k������
		SStringW	GetYPrice(int nY);
		SStringW	GetAllYPrice(int nY);
		int			GetXData(int nx); 	//��ȡ����µ�����id
		int			GetXPos(int nx);
		int			GetYPos(double fDiff);
		void		GetFuTuMaxDiff();		//�жϸ�ͼ���������Сֵ��k������
		int			GetFuTuYPos(double fDiff);	//��ø�ͼyλ��
		SStringW	GetFuTuYPrice(int nY);		//��ø�ͼyλ�ü۸�
		void		GetMACDMaxDiff();
		int			GetMACDYPos(double fDiff);	//���MACDͼyλ��
		SStringW	GetMACDYPrice(int nY);		//���MACDͼyλ�ü۸�
		BOOL        ptIsInKlineRect(CPoint pt, int nDataCount, KlineType &data);
		//������Ӧ
	protected:
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnLButtonDown(UINT nFlags, CPoint point);
		void		OnTimer(char cTimerID);
		int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		void		OnMouseLeave();
		void		OnSize(UINT nType, CSize size);




		//�ؼ���Ӧ
	protected:
		void		OnDbClickedKline(UINT nFlags, CPoint point);
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		// ���ݴ���
	protected:
		void DataInit();

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
		int  ProcBandTargetData(int nPos, Band_t *BandData);
		int	 ProcMACDData(int nPos, MACDData_t  *MacdData);
		void ReProcKlineData(bool bSingleNeedProc = false);

		//�����Ż�������������
	protected:
		double		GetHighPrice(int n, int nPeriod, int nOffset = 0);
		double		GetLowPrice(int n, int nPeriod, int nOffset = 0);
		int			Count(double a[], double b[], int nType, int n, int nPeriod);
		bool		Cross(double a[], double b[], int nPos);
		int			ValueWhen(int a[], int b[], int nPos);


		//����
	protected:
		AllKPIC_INFO *m_pAll;
		MACDData_t  *m_pMacdData;
		Band_t		*m_pBandData;
		size_t			m_nUsedTickCount;
		vector<CommonStockMarket> *m_pStkMarketVec;
		vector<CommonIndexMarket> *m_pIdxMarketVec;
		map<int, vector<KlineType>> *m_pHisKlineMap;
		SStringA	m_strSubIns;
		SStringA	m_strStockName;
		SStringA	m_strL1Indy;
		SStringA	m_strL2Indy;
		bool		m_bAddDay;	//��������

		//������
	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		CDataProc	m_dataHandler;
		SKlineTip*  m_pTip;
		SSubTargetPic**	m_ppSubPic;
		int m_nSubPicNum;

		//��ʾ�ͼ������
	protected:
		BandPara_t  m_BandPara;
		int 		m_nZoomRatio;
		int			m_nMAPara[4];
		int			m_nMACDPara[3];
		bool		m_bDataInited;
		int			m_nKWidth;		//����֮��Ŀ���
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nPreX;
		int			m_nPreY;
		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nMove;		//����Ϊ����ƽ��,0Ϊ����
		int			m_nTickPre;		//�����һ��ˢ��ʱ��
		int			m_nFirst;		//��ʼk��
		int			m_nEnd;			//����k��
		int			m_nPeriod;
		int			m_nBandCount;
		int			m_nMacdCount;
		int			m_nTradingDay;
		int			m_nMouseLinePos;
		int			m_nJiange;
		bool		m_bReSetFirstLine;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
		bool		m_bClearTip;
		bool        m_bShowMA;			//��ʾMA
		bool        m_bShowBandTarget;	//��ʾ�����Ż�����ָ��
		bool		m_bShowDeal;
		bool		m_bIsStockIndex;
		BOOL		*m_pbShowSubPic;
		bool		m_bTodayMarketReady;
		bool		m_bHisKlineReady;
		bool		m_bHisPointReady;
		//��ͼ����
	protected:
		CRect       m_rcAll;		//���¿����
		CRect		m_rcMain;		//�Ͽ�����,K��
		CRect		m_rcVolume;		//�¿�����,ָ��
		CRect       m_rcMACD;		//�¿�2���ָ꣬��2
		CRect		m_rcImage;
		CPoint		m_preMovePoint;
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey, m_penPurple,
			m_penYellow, m_penMAGreen, m_penGray, m_penDotGreen, m_penDotYellow, m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack, m_bBrushDarkBlue;
		BOOL    m_bPaintInit;	//������Ŀ��ʼ��

	protected:
		CRITICAL_SECTION m_cs;
		HWND		m_hParWnd;
		RpsGroup	m_rgGroup;

		SOUI_MSG_MAP_BEGIN()
			//MESSAGE_HANDLER_EX(WM_KLINE_MSG, OnMsg)
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_KEYDOWN(OnKeyDown)
			//MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONDBLCLK(OnDbClickedKline)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			//MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_CREATE(OnCreate)
			SOUI_MSG_MAP_END()
	};

	inline void SKlinePic::SetRpsGroup(RpsGroup rg)
	{
		m_rgGroup = rg;
	}
	inline bool SKlinePic::GetDealState() const
	{
		return m_bShowDeal;
	}
	inline bool SKlinePic::GetVolumeState() const
	{
		return m_bShowVolume;
	}
	inline bool SKlinePic::GetMacdState() const
	{
		return m_bShowMacd;
	}
	inline bool SKlinePic::GetRpsState(int nWndNum) const
	{
		return m_pbShowSubPic[nWndNum];
	}
	inline bool SKlinePic::GetMaState() const
	{
		return m_bShowMA;
	}
	inline bool SKlinePic::GetBandState() const
	{
		return m_bShowBandTarget;
	}
	inline void SKlinePic::SetDealState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowDeal = !m_bShowDeal;
		else m_bShowDeal = bState;
	}
	inline void SKlinePic::SetVolumeState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowVolume = !m_bShowVolume;
		else m_bShowVolume = bState;
	}
	inline void SKlinePic::SetMacdState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowMacd = !m_bShowMacd;
		else m_bShowMacd = bState;
	}
	inline void SKlinePic::SetRpsState(int nWndNum,bool bRevesered, bool bState)
	{
		if (bRevesered) m_pbShowSubPic[nWndNum] = !m_pbShowSubPic[nWndNum];
		else m_pbShowSubPic[nWndNum] = bState;
	}
	inline void SKlinePic::SetMaState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowMA = !m_bShowMA;
		else m_bShowMA = bState;
	}
	inline void SKlinePic::SetBandState(bool bRevesered, bool bState)
	{
		if (bRevesered) m_bShowBandTarget = !m_bShowBandTarget;
		else m_bShowBandTarget = bState;
	}
	inline void SKlinePic::SetMaPara(int maPara[])
	{
		memcpy_s(m_nMAPara, sizeof(m_nMAPara), maPara, sizeof(m_nMAPara));
	}
	inline void SKlinePic::SetMacdPara(int MacdPara[])
	{
		memcpy_s(m_nMACDPara, sizeof(m_nMACDPara), MacdPara, sizeof(m_nMACDPara));
	}
	inline void SKlinePic::SetBandPara(BandPara_t& bandPara)
	{
		m_BandPara = bandPara;
	}
	inline const int * SKlinePic::GetMaPara()
	{
		return m_nMAPara;
	}

	inline const int * SKlinePic::GetMacdPara()
	{
		return m_nMACDPara;
	}

	inline BandPara_t SKlinePic::GetBandPara()
	{
		return m_BandPara;
	}

	inline void SKlinePic::SetPicUnHandled()
	{
		m_bDataInited = false;
	}

	inline int SKlinePic::GetShowSubPicNum() const
	{
		return m_nSubPicNum;
	}

}
#endif // !_SKLINE_PIC