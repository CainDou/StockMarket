
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


//class CDataProc;
namespace SOUI
{
	class CDealList;
	class CPriceList;
	class SSubTargetPic;

	class SFenShiPic : public SWindow
	{
		SOUI_CLASS_NAME(SFenShiPic, L"fenshiPic")	//����xml��ǩ

		// ���ݽӿ�
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
		//ͼ�δ���
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
		//LRESULT		OnMsg(UINT uMsg, WPARAM wp, LPARAM lp);
		void		OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);
		void		OnTimer(char cTimerID);
		void		OnMouseMove(UINT nFlags, CPoint point);
		void		OnMouseLeave();
		void		OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void		OnDbClickedFenshi(UINT nFlags, CPoint point);
		//���ݴ���
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
		void		HandleMissData(FENSHI_GROUP f1, int time);//��ȫ��©������
		//��ʾ���غ�����
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

		//������ʾ����
	protected:
		CRect       m_rcAll;		//���¿����
		CRect		m_rcMain;		//�Ͽ�����,K��
		CRect		m_rcVolume;		//�¿�����,ָ��
		CRect		m_rcMACD;		//MACDָ���
		CRect		m_rcImage;
		CPoint		m_preMovePt;
		std::set<int> m_timeSet;
		std::map<int, SStringW> m_virTimeLineMap;
		int			m_nAllLineNum;			//��ʱͼ��Ŀ����(����ά��)
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
		int			m_nMaxKNum;		//����ʾ��k���������
		int			m_nTickPre;		//�����һ��ˢ��ʱ��
		int			m_nKeyX;
		int			m_nKeyY;
		int			m_nNowPosition;		//���ڵ������ߵ�λ��
		int			m_nPaintTick;
		bool		m_bHalfPrice;

		//��������
	protected:
		CPriceList* m_pPriceList;
		CDealList*  m_pDealList;
		SSubTargetPic**    m_ppSubPic;	//rpsͼ
		int			m_nSubPicNum;
		int			m_nChangeNum;
		//����
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
		int			m_nLastVolume;	//��һ��ʱ�ν����Ľ�����
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
