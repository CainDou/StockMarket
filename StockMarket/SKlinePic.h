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
		SOUI_CLASS_NAME(SKlinePic, L"klinePic")	//����xml��ǩ

	public:
		SKlinePic();
		~SKlinePic();

		void InitShowPara();
		void SetShowData(SStringA subIns, vector<CommonIndexMarket>* pIdxMarketVec);
		void SetShowData(SStringA subIns, vector<CommonStockMarket>* pStkMarketVec);




		int			m_nKWidth;		//����֮��Ŀ��
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nPreX;
		int			m_nPreY;

		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nMove;		//����Ϊ����ƽ��,0Ϊ����
		int			m_nTickPre;		//�����һ��ˢ��ʱ��
		int			m_nFirst;		//��ʼk��
		int			m_nEnd;			//����k��

		bool		m_bNoJiange;
		bool		m_bReSetFirstLine;
		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
		bool		m_bClearTip;

		bool        m_bShowMA;			//��ʾMA
		bool        m_bShowBandTarget;	//��ʾ�����Ż�����ָ��






		int			*m_pGroupDataType;

		int			m_nPeriod;

		int			m_nBandCount;

		int			m_nMacdCount;

		SStringA	m_strSubIns;

		bool		m_bShowDeal;


		bool		m_bIsStockIndex;

		int			m_nTradingDay;




		//	int		ReInit(int nKwidth);		//���½��г�ʼ��






		HWND		m_hParWnd;

		// ͼ�δ���
	protected:
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		void		OnPaint(IRenderTarget *pRT);
		void		DrawArrow(IRenderTarget * pRT);
		void		DrawMouse(IRenderTarget * pRT, CPoint p, BOOL bFromOnPaint = FALSE);
		void		DrawTime(IRenderTarget * pRT, BOOL bFromOnPaint = FALSE);	//��ʱ������
		void		GetMaxDiff();		//�ж����������Сֵ��k������
		BOOL		IsInRect(int x, int y, int nMode = 0);	//�Ƿ���������,0Ϊȫ��,1Ϊ�Ϸ�,2Ϊ�·�
		SStringW	GetYPrice(int nY);
		SStringW	GetAllYPrice(int nY);
		void		DrawData(IRenderTarget * pRT);
		void		DrawTickMainData(IRenderTarget * pRT); //����ͼtick
		void		DrawVolData(IRenderTarget * pRT);		//����ͼvol
		int			GetXData(int nx); 	//��ȡ����µ�����id
		int			GetXPos(int nx);
		int			GetYPos(double fDiff);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnLButtonDown(UINT nFlags, CPoint point);
		void		OnTimer(char cTimerID);
		int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		void		OnMouseLeave();
		void		OnSize(UINT nType, CSize size);
		int			GetShowKlineNum(int nKwidth = 2);	//��ȡ��Ҫ��ʾ��k������

		void		DrawMainUpperMarket(IRenderTarget *pRT, KlineType& data);
		void		DrawMainUpperMA(IRenderTarget *pRT, int nPos);
		void		DrawMacdUpperMarket(IRenderTarget *pRT, int nPos);
		void		DrawMainUpperBand(IRenderTarget *pRT, int nPos);
		void		DrawBandData(IRenderTarget * pRT, int nDataPos, KlineType &Data, int x, int nJiange);
		void		DrawMouseKlineInfo(IRenderTarget * pRT, const KlineType &KlData, CPoint pt, const int &num = 0, const double &fPrePrice = 0);
		BOOL        ptIsInKlineRect(CPoint pt, int nDataCount, KlineType &data);





		void		GetFuTuMaxDiff();		//�жϸ�ͼ���������Сֵ��k������
		int			GetFuTuYPos(double fDiff);	//��ø�ͼyλ��
		SStringW	GetFuTuYPrice(int nY);		//��ø�ͼyλ�ü۸�

		void		GetMACDMaxDiff();
		int			GetMACDYPos(double fDiff);	//���MACDͼyλ��
		SStringW	GetMACDYPrice(int nY);		//���MACDͼyλ�ü۸�

		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255), UINT uFormat = DT_SINGLELINE);
		void		DrawKeyDownLine(IRenderTarget* pRT, bool ClearTip = false);
		void		DrawMouseLine(IRenderTarget * pRT, CPoint p);
		void		DrawMoveTime(IRenderTarget * pRT, int x, int date, int time, bool bNew);
		void		DrawMovePrice(IRenderTarget * pRT, int y, bool bNew);
		void		DrawBarInfo(IRenderTarget * pRT, int nDataPos);


		int			m_nMouseLinePos;
		CPoint		m_preMovePoint;


		//�ؼ���Ӧ
	protected:
		void		OnDbClickedKline(UINT nFlags, CPoint point);
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		// ���ݴ���
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

		//�����Ż�����������
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
		bool			m_bAddDay;	//�������

		//������
	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		CDataProc	m_dataHandler;
		SKlineTip*  m_pTip;
	public:
		SSubPic*	m_pSubPic;

		//��ʾ�ͼ������
	protected:
		BandPara_t  m_BandPara;
		InitPara_t  m_InitPara;
		int			m_nMAPara[4];
		int			m_nMACDPara[3];
		bool		m_bDataInited;

		//��ͼ����
	protected:
		CRect       m_rcAll;		//���¿����
		CRect		m_rcUpper;		//�Ͽ�����,K��
		CRect		m_rcLower;		//�¿�����,ָ��
		CRect       m_rcLower2;		//�¿�2���ָ꣬��2
		CRect		m_rcImage;
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey, m_penPurple,
			m_penYellow, m_penMAGreen, m_penGray, m_penDotGreen, m_penDotYellow, m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack, m_bBrushDarkBlue;
		BOOL    m_bPaintInit;	//������Ŀ��ʼ��

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