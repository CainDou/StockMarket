
#pragma once
#include <core/swnd.h>


#define MOVE_ONESTEP		10		//ÿ��ƽ�Ƶ�������
#define K_WIDTH_TOTAL		16		//k��ռ���ܿ��(��x����)
//һЩ�����Ľ�������Ϣ
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
		SOUI_CLASS_NAME(SFenShiPic, L"fenshiPic")	//����xml��ǩ

		// ���ݽӿ�
	public:
		SFenShiPic();
		~SFenShiPic();

		void SetShowData(SStringA subIns, vector<CommonIndexMarket>* pIdxMarketVec);
		void SetShowData(SStringA subIns, vector<CommonStockMarket>* pStkMarketVec);
		void InitShowPara();

		int			m_nAllLineNum;			//��ʱͼ��Ŀ����(����ά��)

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

		CRect       m_rcAll;		//���¿����
		CRect		m_rcUpper;		//�Ͽ�����,K��
		CRect		m_rcLower;		//�¿�����,ָ��
		CRect		m_rcMACD;		//MACDָ���
		CRect		m_rcFutNStcok;
		CRect		m_rcImage;

		int			m_nMouseX;
		int			m_nMouseY;
		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nTickPre;		//�����һ��ˢ��ʱ��

		int			m_nKeyX;
		int			m_nKeyY;

		int			m_nLastVolume;	//��һ��ʱ�ν����Ľ�����


		InitPara_t  m_InitPara;

		int			m_nNowPosition;		//���ڵ������ߵ�λ��

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

		//ͼ�δ���
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

		//ͼ��������ݳ�ʼ���ͼ����ȡ
	protected:
		void		DataInit();
		void		SingleInit();
		void		SetShowTime();
		void		GetMaxDiff();		//�ж����������Сֵ��k������
		int			GetXPos(int n);		//��ȡid��Ӧ��x����
		int			GetXData(int nx); 	//��ȡ����µ�����id
		int			GetYPos(double fDiff);
		BOOL		IsInRect(int x, int y, int nMode = 0);	//�Ƿ���������,0Ϊȫ��,1Ϊ�Ϸ�,2Ϊ�·�
		SStringW	GetYPrice(int nY, BOOL bIsPercent);
		void		GetFuTuMaxDiff();			//�жϸ�ͼ���������Сֵ��k������
		int			GetFuTuYPos(double fDiff);	//��ø�ͼyλ��
		SStringW	GetFuTuYPrice(int nY);		//��ø�ͼyλ�ü۸�
		int			GetMACDYPos(double fDiff);	//���MACDͼyλ��
		SStringW	GetMACDYPrice(int nY);		//���MACDͼyλ�ü۸�
		void        InitVirTimeLineMap();
		void		GetMACDMaxDiff();			//�ж�MACDָ��ͼ�����ֵ��Сֵ
		void		GetMacdDiff();
		void		SetWindowRect();
		COLORREF	GetColor(double dPrice);

		//��Ϣ��Ӧ
	protected:
		void		OnTimer(char cTimerID);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnMouseLeave();
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void		OnDbClickedFenshi(UINT nFlags, CPoint point);

		//���ݴ���
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
		void		HandleMissData(FENSHI_GROUP f1, int time);//��ȫ��©������

		//��ʾ����
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
		SSubPic*    m_pSubPic;	//rpsͼ
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
