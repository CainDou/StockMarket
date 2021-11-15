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
		SOUI_CLASS_NAME(SKlinePic, L"klinePic")	//����xml��ǩ

	public:
		SKlinePic();
		~SKlinePic();

		void		InitShowPara();
		AllKPIC_INFO *m_pAll;

		MACDData_t  *m_pMacdData;

		CRect       m_rcAll;		//���¿����
		CRect		m_rcUpper;		//�Ͽ�����,K��
		CRect		m_rcLower;		//�¿�����,ָ��
		CRect       m_rcLower2;		//�¿�2���ָ꣬��2
		CRect		m_rcImage;

		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;


		int			m_nCAType;		//��ʾ���Ͼ�����Ϣ

		bool		m_bNoJiange;

		double		m_fPreSettlePrice;

		//	int		m_nCyc;			//��������
		int			m_nKWidth;		//����֮��Ŀ��
		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nKeyX;
		int			m_nKeyY;

		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nMove;		//����Ϊ����ƽ��,0Ϊ����
		bool		m_bHover;
		int			m_nTickPre;		//�����һ��ˢ��ʱ��
		int			m_nFirst;		//��ʼk��
		int			m_nEnd;			//����k��

		bool		bPaintAfetrDataProc;

		bool		m_bShowMouseLine;
		bool		m_bKeyDown;
		bool		m_bShowVolume;
		bool		m_bShowMacd;
	
		InitPara_t  m_InitPara;
		bool        m_bShowHighLow;		//��ʾ�ߵͼ�
		bool        m_bShowMA;			//��ʾMA
		bool        m_bShowBandTarget;	//��ʾ�����Ż�����ָ��


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

		int			m_nKlineType;	//0��һ���ӣ�1��5���ӣ�2��15���ӣ�3��30���ӣ�4��60���ӣ�5������

		int			m_nPeriod;

		int			m_nBandCount;

		int			m_nMacdCount;

		//int			m_nSubIns;
		InsIDType	m_strSubIns;

		bool		m_bShowDeal;

		bool		m_bSubInsisGroup;

		bool		m_bIsStockIndex;

		int				m_nGroupVolume;			//����������ʷ1����K�����ݵ����һ��
		int				m_nGroupTime;			//����������ʷ1����K�����һ����������ʱ��
		bool			m_bGroupIsWhole;		//��ʷ1����K�����һ���Ƿ�����

		int				m_nTradingDay;
		std::vector<GroupKlineType> m_Group1MinKline;							//��ʷ��Ϻ�Լ1����K��
		std::vector<GroupKlineType> m_GroupDayKline;							//��ʷ��Ϻ�Լ��K��


		BOOL    m_bPaintInit;	//������Ŀ��ʼ��
		CAutoRefPtr<IFont> m_pFont12;
		CAutoRefPtr<IPen> m_penRed, m_penGreen, m_penWhite, m_penGrey,m_penPurple,m_penYellow,m_penMAGreen,m_penGray,m_penDotGreen,m_penDotYellow,m_penDotRed;
		CAutoRefPtr<IBrush> m_bBrushGreen, m_bBrushGrey, m_bBrushBlack,m_bBrushDarkBlue;


		//	int		ReInit(int nKwidth);		//���½��г�ʼ��
		int			GetShowKlineNum(int nKwidth = 2);	//��ȡ��Ҫ��ʾ��k������
		//	void	SetLowerData(float *p[], BYTE *bft);													//��ͼ��Ϣ
		//	void	SetUpperMainZhiBiao(float *p[], bool* bShow);											//��ͼ����Ϣָ��
		//	void	SetUpperExtraData(DATA_FOR_SHOW *p, int nDecimal, int nDecimalXi);						//��ͼ����Ϣ
		//	void	SetUpperMainData(DATA_FOR_SHOW *p, int nTotal, int nDecimal, int nDecimalXi);			//��ͼ����Ϣ

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
		void		DrawTime(IRenderTarget * pRT,  BOOL bFromOnPaint=FALSE);	//��ʱ������
		void		GetMaxDiff();		//�ж����������Сֵ��k������
		BOOL		IsInRect(int x, int y, int nMode = 0);	//�Ƿ���������,0Ϊȫ��,1Ϊ�Ϸ�,2Ϊ�·�
		SStringW	GetYPrice(int nY);
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

		BOOL        ptIsInKlineRect(CPoint pt,int nDataCount,DATA_FOR_SHOW &data );
		BOOL		IsCATime(int nTime,int nDate);
		BOOL        IsFirstLine(int nTime);
		void		FileDataToSingle(bool bPreLineIsCA,const KLineDataType &data, int nCount,bool bMutiMin);

		void		AddCADataToFirstLine(KLineDataType &CALine, KLineDataType &FirstLine);

		bool		m_bHandleTdyFirstLine;

		CRITICAL_SECTION m_cs;



		void		GetFuTuMaxDiff();		//�жϸ�ͼ���������Сֵ��k������
		int			GetFuTuYPos(double fDiff);	//��ø�ͼyλ��
		SStringW	GetFuTuYPrice(int nY);		//��ø�ͼyλ�ü۸�

		void		GetMACDMaxDiff();
		int			GetMACDYPos(double fDiff);	//���MACDͼyλ��
		SStringW	GetMACDYPrice(int nY);		//���MACDͼyλ�ü۸�

		void		DrawTextonPic(IRenderTarget * pRT, CRect rc, SStringW str, COLORREF color = RGBA(255, 255, 255, 255),UINT uFormat=DT_SINGLELINE);
		void		DrawKeyDownMouseLine(IRenderTarget * pRT, UINT nChar);

		double		GetHighPrice(int n,int nPeriod,int nOffset=0);
		double		GetLowPrice(int n, int nPeriod,int nOffset=0);
		int			Count(double a[], double b[], int nType, int n, int nPeriod);
		bool		Cross(double a[],double b[],int nPos);
		int			ValueWhen(int a[],int b[],int nPos);



		//������Լ��K�����ݴ���
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

		void GroupHisKline1MinProc();			//��ʷ1����K�ߺϳ�
		void GroupHisDayKlineProc();

		//��û��ʵʱK�ߵ�����¸���K��
		void GroupUpdateNoKline( int nCount, int nPos, std::vector<GroupDataType>* pTickVec, int ntime, bool bAdd, bool bNewLine = true);



		void KlineMAProc( int nCount);



		bool TimeIsLastBar(int UpdateTime, int LastBarTime, int TickTime,int nPeriod);
		//��Ϻ�Լ��K�����ݴ���


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