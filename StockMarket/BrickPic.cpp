#include "stdafx.h"
#include "BrickPic.h"
#include "Define.h"
#include "DealList.h"
#include "PriceList.h"

#define MOVE_ONESTEP		10		//每次平移的数据量
#define K_WIDTH_TOTAL		16		//k线占用总宽度(在x轴上)
//一些基本的界面框架信息
#define RC_LEFT				0
#define RC_RIGHT			60
#define RC_TOP				25
#define RC_BOTTOM			40

#define RC_RIGHT_BACK		50		//框内最后一根k线和框得距离

#define DATA_ERROR			-1234567

#define	ZOOMWIDTH (m_nKWidth * 1.0 / m_fZoomRatio)
#define TOTALZOOMWIDTH ((m_nKWidth + m_nGap) * 1.0 / m_fZoomRatio)

#define  MAX_JIANGE 45
#define  MAX_KWIDTH 45
#define INFOHEIGHT 20
#define MAININFOCOUNT 2
#define SUBINFOCOUNT 1
#define TEXTHEIGHTMARGIN 5

constexpr double fRevolution = 1e-6;
const double fMaxGenTime = 10;

constexpr int MinSec = 60;
constexpr int HourSec = 60 * MinSec;
constexpr int DaySec = 24 * HourSec;
constexpr int MonthSec = 30 * DaySec;
constexpr int YearSec = 365 * DaySec;


using std::vector;
const std::vector<SStringW> strBrickType =
{ L"跳",L"%",L" 价格" };

SOUI::BrickPic::BrickPic() :
	m_fZoomRatio(1.0), m_nKWidth(9), m_nEnd(0), m_nFirst(0),
	m_PtMouse(-1, -1), m_nMove(-1), m_bReSetFirstLine(false),
	m_bKeyDown(false), m_nGap(2), m_nBrickType(eBT_Tick), m_fBrickSetting(50),
	m_nGroupDataType(1), m_bShowMouseLine(false), m_pStkMarketVec(nullptr),
	m_pHisRenkoVec(nullptr), m_bHalfPrice(false), m_bShowGenTime(true), m_nMainTarget(eBMT_Pendant),
	m_fPendantPara({ 2 })
{
	m_bFocusable = 1;
	m_pDealList.reset(new CDealList);
	m_pPriceList.reset(new CPriceList);
	m_bPaintInit = FALSE;
}

SOUI::BrickPic::~BrickPic()
{

}

void SOUI::BrickPic::SetShowIns(SStringA strIns, SStringA StockName)
{
	m_bDataInited = false;
	m_strSubIns = strIns;
	m_strStockName = StockName;
	m_pPriceList->ChangeShowStock(strIns, m_strStockName);
	m_pDealList->ChangeShowData(strIns);
}


void SOUI::BrickPic::InitShowPara(const InitPara_t& para)
{
	m_fZoomRatio = para.fBrickZoomRatio;
	m_fBrickSetting = para.fBrickSetting;
	m_nKWidth = para.nBrickWidth;
	m_nBrickType = para.nBrickType;
	m_nGap = para.nBrickJiange;
	m_bShowDeal = para.bShowBrickDeal;
	m_bShowGenTime = para.bShowBrickGenTime;
	m_nMainTarget = para.nBrickMainTarget;
	if (m_fPendantPara.empty())
		m_fPendantPara.emplace_back(para.fBrickPendantPara);
	else
		m_fPendantPara[0] = para.fBrickPendantPara;
}

void SOUI::BrickPic::OutPutShowPara(InitPara_t& para)
{
	para.fBrickZoomRatio = m_fZoomRatio;
	para.fBrickSetting = m_fBrickSetting;
	para.nBrickWidth = m_nKWidth;
	para.nBrickType = m_nBrickType;
	para.nBrickJiange = m_nGap;
	para.bShowBrickDeal = m_bShowDeal;
	para.bShowBrickGenTime = m_bShowGenTime;
	para.nBrickMainTarget = m_nMainTarget;
	para.fBrickPendantPara = m_fPendantPara[0];

}

void SOUI::BrickPic::SetDataPoint(vector<CommonStockMarket>* pStkMarketVec,
	vector<RenkoData>* pHisRenkoVec)
{
	m_pStkMarketVec = pStkMarketVec;
	m_pHisRenkoVec = pHisRenkoVec;
	m_pPriceList->SetDataPoint(m_pStkMarketVec);
	m_pDealList->SetDataPoint(m_pStkMarketVec);
}

void	SOUI::BrickPic::DataProc()
{
	m_bDataInited = false;
	m_nUsedTickCount = 0;
	DataInit();
	RenkoDataWithHis();
	RenkoDataUpdate();
	m_bDataInited = true;
}

void SOUI::BrickPic::ReProcData()
{
}

void SOUI::BrickPic::UpdateData()
{
	if (m_bDataInited)
		RenkoDataUpdate();
}

void BrickPic::ReProcKlineRehabData(eRehabType rehabType)
{
	m_bDataInited = false;
	m_rehabType = rehabType;
	m_calcRehabType = m_rehabType;
	m_nFTRehabTime = 0;
	m_nUsedTickCount = 0;
	RenkoDataWithHis();
	RenkoDataUpdate();
	m_bDataInited = true;
}

void BrickPic::ReProcKlineRehabData(FixedTimeRehab& frt)
{
	m_bDataInited = false;
	m_rehabType = frt.Type;
	m_nFTRehabTime = frt.nDate;
	if (eRT_Rehab_Cash_FixedTime == m_rehabType)
		m_calcRehabType = frt.bFrontRehab ? eRT_FrontRehab_Cash : eRT_BackRehab_Cash;
	else
		m_calcRehabType = frt.bFrontRehab ? eRT_FrontRehab_ReInv : eRT_BackRehab_ReInv;

	m_nUsedTickCount = 0;
	RenkoDataWithHis();
	RenkoDataUpdate();
	m_bDataInited = true;
}


void BrickPic::SetPriceListHalf(bool bHalf)
{
	m_bHalfPrice = bHalf;
	m_pPriceList->SetPriceListHalf(m_bHalfPrice);
}


void SOUI::BrickPic::OnDbClicked(UINT nFlags, CPoint point)
{
	m_bShowMouseLine = !m_bShowMouseLine;
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
	}
	Invalidate();
}

void BrickPic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (m_DateTimeVec.empty())
		return;
	switch (nChar)
	{
	case VK_UP:
	{
		if (m_bKeyDown)
		{
			m_bReSetFirstLine = true;
		}
		int nPicWidth = m_rcMain.right - m_rcMain.left - RC_RIGHT_BACK;

		double fNowDataCount = nPicWidth * 1.0 / TOTALZOOMWIDTH;
		int nMinDataCount = nPicWidth * 1.0 / (MAX_KWIDTH + MAX_JIANGE);
		if (int(fNowDataCount + 0.5) == nMinDataCount)
			break;
		int nNowDataCount = max(nMinDataCount, fNowDataCount / 1.5 + 0.5);
		SetPicBarWidth(nPicWidth, nNowDataCount);
		Invalidate();
		break;
	}
	break;
	case VK_DOWN:
	{
		if (m_bKeyDown)
		{
			m_bReSetFirstLine = true;
		}
		int nPicWidth = m_rcMain.right - m_rcMain.left - RC_RIGHT_BACK;

		double fNowDataCount = nPicWidth * 1.0 / TOTALZOOMWIDTH;
		if (int(fNowDataCount + 0.5) == MAX_DATA_COUNT)
			break;
		int nNowDataCount = min(MAX_DATA_COUNT, fNowDataCount * 1.5 + 0.5);
		SetPicBarWidth(nPicWidth, nNowDataCount);
		Invalidate();

	}
	break;
	case VK_LEFT:
	case VK_RIGHT:
	{
		if (!m_bKeyDown)
		{
			m_bKeyDown = true;
			if (m_bShowMouseLine)
			{
				int nx = GetXData(m_PtMouse.x);
				if (nx >= m_DateTimeVec.size())
					nx = m_DateTimeVec.size() - 1;
				m_nNowPosition = max(nx - m_nFirst, 0);
			}
			else
			{
				m_PtMouse.x = m_PtMouse.y = -1;
				if (nChar == VK_LEFT)
					m_nNowPosition = m_nEnd - m_nFirst - 1;
				else if (nChar == VK_RIGHT)
					m_nNowPosition = 0;
			}
			m_bShowMouseLine = true;
			Invalidate();
			break;
		}
		else
		{

			if (nChar == VK_LEFT)
				m_nNowPosition--;
			else if (nChar == VK_RIGHT)
				m_nNowPosition++;
			if (m_nNowPosition < 0)
			{
				m_nMove = max(0, m_nFirst - 1);
				m_nNowPosition = 0;
				Invalidate();
				break;
			}
			if (m_nNowPosition >= m_nEnd - m_nFirst)
			{
				m_nMove = m_nFirst + 1;
				if (m_nMove + m_nMaxKNum >= m_DateTimeVec.size())
					m_nMove = -1;
				m_nNowPosition = m_nEnd - m_nFirst - 1;
				Invalidate();
				break;
			}
		}
		CRect rc = GetClientRect();
		CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
		SPainter pa;
		SWindow::BeforePaint(pRT, pa);
		DrawKeyDownLine(pRT, true);
		AfterPaint(pRT, pa);
		ReleaseRenderTarget(pRT);

	}
	break;
	default:
		break;
	}
}

void SOUI::BrickPic::DataInit()
{
	ReSetData(0);
	m_fPriceTick = 0.01;
	m_nDecimal = 2;
	m_nUsedTickCount = 0;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_nTradingDay = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

}

void SOUI::BrickPic::RenkoDataWithHis()
{
	m_nFirst = m_nEnd = 0;
	m_nMove = -1;
	m_bTodayFirst = true;
	if (m_pHisRenkoVec == nullptr)
		return;
	auto& renkoVec = *m_pHisRenkoVec;
	ReSetData(renkoVec.size());
	for (int i = 0; i < renkoVec.size(); ++i)
	{
		if (renkoVec[i].date == 20260117) //该天非交易日 有错误数据需要剔除
			continue;

		RenkoData data;
		if (eRT_NoRehab == m_calcRehabType)
			data = renkoVec[i];
		else if (eRT_FrontRehab_Cash == m_calcRehabType)
			data = FrontRehabCash(renkoVec[i], m_nFTRehabTime);
		else if (eRT_FrontRehab_ReInv == m_calcRehabType)
			data = FrontRehabReInv(renkoVec[i], m_nFTRehabTime);
		else if (eRT_BackRehab_Cash == m_calcRehabType)
			data = BackRehabCash(renkoVec[i], m_nFTRehabTime);
		else if (eRT_BackRehab_ReInv == m_calcRehabType)
			data = BackRehabReInv(renkoVec[i], m_nFTRehabTime);
		int64_t datetime = (int64_t)(data.date) * 1'000'000 + (int64_t)data.time;
		AddData(data, datetime);
	}
	if (m_nMainTarget == eBMT_Pendant)
		m_PendantDataVec.resize(m_DateTimeVec.size());
	//for (int i = 0; i < m_DateTimeVec.size(); ++i)
	//{
	//	m_nNowDataPos = i;
	//	CalcData();
	//	m_nLastDataPos = m_nNowDataPos;
	//}
	m_nLastDataPos = 0;
	m_nNowDataPos = m_DateTimeVec.size() - 1;
	CalcData();
	m_nLastDataPos = m_nNowDataPos;

	m_bDataInited = true;
}

void SOUI::BrickPic::RenkoDataUpdate()
{
	if (m_pStkMarketVec == nullptr)
		return;
	vector<CommonStockMarket> TickVec(m_pStkMarketVec->begin() + m_nUsedTickCount, m_pStkMarketVec->end());
	if (TickVec.empty())
		return;
	int64_t nLastTime = m_DateTimeVec.empty() ? 0 : m_DateTimeVec.back();
	for (int i = 0; i < TickVec.size(); i++)
	{
		++m_nUsedTickCount;
		if (TickVec[i].Volume <= 0)
			continue;
		int64_t ntime = (int64_t)m_nTradingDay * 1'000'000 + TickVec[i].UpdateTime;
		if (ntime < nLastTime)
			continue;
		double fPrice = TickVec[i].LastPrice;
		if (eRT_FrontRehab_Cash == m_calcRehabType)
			fPrice = FrontRehabCash(fPrice, m_nFTRehabTime);
		else if (eRT_FrontRehab_ReInv == m_calcRehabType)
			fPrice = FrontRehabReInv(fPrice, m_nFTRehabTime);
		else if (eRT_BackRehab_Cash == m_calcRehabType)
			fPrice = BackRehabCash(fPrice, m_nFTRehabTime);
		else if (eRT_BackRehab_ReInv == m_calcRehabType)
			fPrice = BackRehabReInv(fPrice, m_nFTRehabTime);
		bool bJump = m_bTodayFirst ? true : false;
		m_bTodayFirst = false;
		AddData(fPrice, ntime, bJump);
	}
	if (m_nMainTarget == eBMT_Pendant)
	{
		while (m_PendantDataVec.size() < m_DateTimeVec.size())
			m_PendantDataVec.emplace_back(std::vector<double>());
	}
	m_nNowDataPos = m_DateTimeVec.size() - 1;
	CalcData();
	m_nLastDataPos = m_nNowDataPos;


	Invalidate();
}

void SOUI::BrickPic::ReSetData(int nCount)
{
	m_DateTimeVec.clear();
	Open.clear();
	High.clear();
	Low.clear();
	Close.clear();
	GenTime.clear();
	BarState.clear();
	m_BrickJumpPos.clear();
	m_fPreStart = 0;

	m_DateTimeVec.reserve(nCount + 600);
	Open.reserve(nCount + 600);
	High.reserve(nCount + 600);
	Low.reserve(nCount + 600);
	Close.reserve(nCount + 600);
	GenTime.reserve(nCount + 600);
	BarState.reserve(nCount + 600);

	if (m_nMainTarget == eBMT_Pendant)
	{
		m_PendantDataVec.clear();
		m_PendantDataVec.reserve(nCount + 600);
		//m_fPreHigh = INT_MIN;
		//m_fPreLow = DBL_MAX;
	}

}

void SOUI::BrickPic::AddData(const RenkoData& data, int64_t time)
{
	if (m_DateTimeVec.empty())
		AddBrickDataToEmpty(data, time);
	else
	{
		double fUpperStart = 0;
		double fLowerStart = 0;
		double fPreClose = Close.size() > 1 ? Close[Close.size() - 2] : Open.back();
		if (Open.size() > 1)
		{
			fUpperStart = max(Open[Open.size() - 2], fPreClose);
			fLowerStart = min(Open[Open.size() - 2], fPreClose);
		}
		else
			fUpperStart = fLowerStart = Open.back();
		double fUpper = fUpperStart + GetBrickStep(fUpperStart);
		double fLower = fLowerStart - GetBrickStep(fLowerStart);
		int64_t preTime = m_DateTimeVec.size() > 1 ? m_DateTimeVec[m_DateTimeVec.size() - 2] :
			m_DateTimeVec.back();
		if (m_fPreStart != data.fStart)
		{
			UpdatePrice(data.fStart, time, fUpper, fLower, fUpperStart,
				fLowerStart, data.fStart != Close.back(), preTime);
			m_fPreStart = data.fStart;
		}
		UpdatePrice(data.fEnd, time, fUpper, fLower, fUpperStart, fLowerStart, false, preTime);

	}
}

void SOUI::BrickPic::AddData(double fPrice, int64_t time, bool bJump)
{
	if (m_DateTimeVec.empty())
	{
		RenkoData data{ 0, 0, fPrice, fPrice };
		AddBrickDataToEmpty(data, time);
	}
	else
	{
		double fUpperStart = 0;
		double fLowerStart = 0;
		double fPreClose = Close.size() > 1 ? Close[Close.size() - 2] : Open.back();
		if (Open.size() > 1)
		{
			fUpperStart = max(Open[Open.size() - 2], fPreClose);
			fLowerStart = min(Open[Open.size() - 2], fPreClose);
		}
		else
			fUpperStart = fLowerStart = Open.back();
		int64_t preTime = m_DateTimeVec.size() > 1 ? m_DateTimeVec[m_DateTimeVec.size() - 2] :
			m_DateTimeVec.back();
		double fUpper = fUpperStart + GetBrickStep(fUpperStart);
		double fLower = fLowerStart - GetBrickStep(fLowerStart);
		UpdatePrice(fPrice, time, fUpper, fLower, fUpperStart, fLowerStart, bJump, preTime);
	}
}

void BrickPic::CalcData()
{
	if (m_DateTimeVec.empty())
		return;
	for (int i = m_nLastDataPos; i <= m_nNowDataPos; ++i)
	{
		if (m_nMainTarget == eBMT_Pendant)
			CalcPendant(i);
	}
}

void SOUI::BrickPic::AddBrickDataToEmpty(const RenkoData& data, int64_t time)
{
	std::vector<double> fPriceVec{ data.fStart, data.fEnd };
	double fOpen = data.fStart;
	m_fPreStart = data.fStart;
	Open.emplace_back(fOpen);
	Close.emplace_back(fOpen);
	High.emplace_back(fOpen);
	Low.emplace_back(fOpen);
	GenTime.emplace_back(0);
	BarState.emplace_back(0);
	m_DateTimeVec.emplace_back(time);
	double fUpperStart = fOpen;
	double fLowerStart = fOpen;
	double fUpper = fOpen + GetBrickStep(fUpperStart);
	double fLower = fOpen - GetBrickStep(fLowerStart);
	for (int i = 0; i < fPriceVec.size(); ++i)
		UpdatePrice(fPriceVec[i], time, fUpper, fLower, fUpperStart, fLowerStart, false, time);

}

double SOUI::BrickPic::GetBrickStep(double fClose) const
{
	if (m_nBrickType == eBT_Tick)
		return m_fBrickSetting * m_fPriceTick;
	else if (m_nBrickType == eBT_Ratio)
		return fClose * m_fBrickSetting * 0.01;
	else if (m_nBrickType == eBT_Price)
		return m_fBrickSetting;
	return 0.01;
}

void SOUI::BrickPic::UpdatePrice(double fPrice, int64_t time, double& fUpper, double& fLower,
	double& fUpperStart, double& fLowerStart, bool bJump, int64_t& preTime)
{
	if (fPrice == 0)
		return;
	double fHigh = High.back();
	double fLow = Low.back();
	double fUpperDiff = fPrice - fUpper;
	double fLowerDiff = fPrice - fLower;

	if (fUpperDiff >= 0 || abs(fUpperDiff) < fRevolution)
	{
		while (fUpperDiff >= 0 || abs(fUpperDiff) < fRevolution)
		{
			ChangeLastDataTime(time);
			if (Open.back() != fUpperStart)
				Open.back() = fUpperStart;
			else
				Low.back() = fUpperStart;
			Close.back() = fUpper;
			High.back() = fUpper;
			GenTime.back() = GetGenTime(time, preTime);
			BarState.back() = 1;
			preTime = time;
			if (bJump || GenTime.back() == 0)
				m_BrickJumpPos.insert(m_DateTimeVec.size() - 1);
			fLowerStart = fUpperStart;
			fLower = fLowerStart - GetBrickStep(fLowerStart);
			fUpperStart = fUpper;
			fUpper += GetBrickStep(fUpperStart);
			m_DateTimeVec.emplace_back(time);
			Open.emplace_back(fUpperStart);
			Close.emplace_back(fUpperStart);
			High.emplace_back(max(fPrice, fUpperStart));
			Low.emplace_back(fUpperStart);
			GenTime.emplace_back(0);
			BarState.emplace_back(0);
			fUpperDiff = fPrice - fUpper;
		}
	}
	else if (fLowerDiff <= 0 || abs(fLowerDiff) < fRevolution)
	{
		while (fLowerDiff <= 0 || abs(fLowerDiff) < fRevolution)
		{
			ChangeLastDataTime(time);

			if (Open.back() != fLowerStart)
				Open.back() = fLowerStart;
			else
				High.back() = fLowerStart;
			GenTime.back() = GetGenTime(time, preTime);
			preTime = time;
			if (bJump || GenTime.back() == 0)
				m_BrickJumpPos.insert(m_DateTimeVec.size() - 1);
			Open.back() = fLowerStart;
			Close.back() = fLower;
			Low.back() = fLower;
			BarState.back() = 1;
			fUpperStart = fLowerStart;
			fUpper = fUpperStart + GetBrickStep(fUpperStart);
			fLowerStart = fLower;
			fLower -= GetBrickStep(fLowerStart);
			m_DateTimeVec.emplace_back(time);
			Open.emplace_back(fLowerStart);
			Close.emplace_back(fLowerStart);
			Low.emplace_back(min(fPrice, fLowerStart));
			High.emplace_back(fLowerStart);
			GenTime.emplace_back(0);
			BarState.emplace_back(0);
			fLowerDiff = fPrice - fLower;
		}
	}
	else
	{
		ChangeLastDataTime(time);
		Close.back() = fPrice;
		if (fPrice > High.back())
			High.back() = fPrice;
		else if (fPrice < Low.back())
			Low.back() = fPrice;
		GenTime.back() = GetGenTime(time, preTime);
	}
}

void SOUI::BrickPic::ChangeLastDataTime(int64_t time)
{
	if (m_DateTimeVec.back() != time)
		m_DateTimeVec.back() = time;
}

void BrickPic::CalcPendant(int nID)
{
	if (m_PendantDataVec[nID].empty())
	{
		m_PendantDataVec[nID].resize(5, 0);
		if (m_bPendantState != m_bPrePendantState)
		{
			m_bPrePendantState = m_bPendantState;
			m_bPendantStateChange = true;
		}
		else
			m_bPendantStateChange = false;
		m_bPrePassTurn = m_bPassTurn;

	}
	double fUpperStart = 0;
	double fLowerStart = 0;
	double fPreClose = nID > 0 ? Close[nID - 1] : Open.back();
	if (nID > 0)
	{
		fUpperStart = max(Open[nID - 1], fPreClose);
		fLowerStart = min(Open[nID - 1], fPreClose);
	}
	else
		fUpperStart = fLowerStart = Open.back();
	double fUpperStep = GetBrickStep(fUpperStart);
	double fLowerStep = GetBrickStep(fLowerStart);
	bool bBarComplete = BarState[nID] == 1;


	if (nID > 0)
	{
		bool bRealCalcState = m_bPrePendantState;
		//if (m_bPendantStateChange)
		//{
		//	if (m_bPrePendantState && Close[nID] < Open[nID]) //当前应该变为多头状态 但由于变相没有更改
		//		bRealCalcState = false;
		//	if(!m_bPrePendantState && Close[nID] > Open[nID])
		//		bRealCalcState = true;

		//}
		if (bRealCalcState) //多头状态
		{
			m_PendantDataVec[nID][0] = max(High[nID] - fUpperStep * m_fPendantPara[0], m_PendantDataVec[nID - 1][0]);
			m_PendantDataVec[nID][2] = m_PendantDataVec[nID][0];
			if (Close[nID] < Open[nID] && bBarComplete)
				m_bPassTurn = true;
			bool bNeedStop = false;
			if (Low[nID] < m_PendantDataVec[nID][2])
			{
				if (m_bPrePassTurn || bBarComplete)
					bNeedStop = true;
			}
			if (!bNeedStop) //没触及转折
			{
				m_PendantDataVec[nID][1] = min(Low[nID] + fLowerStep * m_fPendantPara[0], m_PendantDataVec[nID - 1][1]);
				//m_bPendantState = true;
			}
			else
			{
				m_bPendantState = false;
				m_bPassTurn = false;
				m_PendantDataVec[nID][1] = Low[nID] + fLowerStep * m_fPendantPara[0];
				m_PendantDataVec[nID][3] = 1;
				if (m_PendantDataVec[nID][4] == 0)
					m_PendantDataVec[nID][4] = m_PendantDataVec[nID][0];
			}
		}
		else
		{
			m_PendantDataVec[nID][1] = min(Low[nID] + fLowerStep * m_fPendantPara[0], m_PendantDataVec[nID - 1][1]);
			m_PendantDataVec[nID][2] = m_PendantDataVec[nID][1];
			if (Close[nID] > Open[nID] && bBarComplete)
				m_bPassTurn = true;
			bool bNeedStop = false;
			if (High[nID] > m_PendantDataVec[nID][2])
			{
				if (m_bPrePassTurn || bBarComplete)
					bNeedStop = true;
			}

			if (!bNeedStop) //没触及转折
			{
				m_PendantDataVec[nID][0] = max(High[nID] - fUpperStep * m_fPendantPara[0], m_PendantDataVec[nID - 1][0]);
				//m_bPendantState = false;
			}
			else
			{
				m_bPendantState = true;
				m_bPassTurn = false;
				m_PendantDataVec[nID][0] = High[nID] - fUpperStep * m_fPendantPara[0];
				m_PendantDataVec[nID][3] = -1;
				if (m_PendantDataVec[nID][4] == 0)
					m_PendantDataVec[nID][4] = m_PendantDataVec[nID][1];
			}

		}

	}
	else
	{
		m_PendantDataVec[nID][0] = max(High[nID] - fUpperStep * m_fPendantPara[0], Open[0]);
		m_PendantDataVec[nID][1] = min(Low[nID] + fLowerStep * m_fPendantPara[0], Open[0]);

		if (Close[0] > Open[0])
		{
			m_PendantDataVec[nID][2] = m_PendantDataVec[nID][0];
			m_bPrePendantState = m_bPendantState = true;
		}
		else
		{
			m_PendantDataVec[nID][2] = m_PendantDataVec[nID][1];
			m_bPrePendantState = m_bPendantState = false;
		}
		m_bPrePassTurn = m_bPassTurn = false;

	}

}

void BrickPic::ReProcMainTarget()
{
	if (m_nMainTarget == eBMT_Pendant)
		ReProcPendant();

}

void BrickPic::ReProcPendant()
{
	m_PendantDataVec.clear();
	m_PendantDataVec.resize(m_DateTimeVec.size());
	for (int i = 0; i < m_PendantDataVec.size(); ++i)
		CalcPendant(i);
}

double BrickPic::GetGenTime(int64_t nowTime, int64_t preTime)
{
	if (nowTime == preTime)
		return 0;
	auto nowTimeStamp = GetUtcTime(nowTime);
	auto preTimeStamp = GetUtcTime(preTime);
	return nowTimeStamp - preTimeStamp;
}

time_t BrickPic::GetUtcTime(int64_t datetime)
{
	struct tm tm_utc = { 0 };

	// 提取各个时间字段
	tm_utc.tm_year = datetime / 10000000000LL - 1900; // 年份减去 1900
	tm_utc.tm_mon = (datetime / 100000000LL) % 100 - 1; // 月份 0-11
	tm_utc.tm_mday = (datetime / 1000000LL) % 100;       // 日 1-31
	tm_utc.tm_hour = (datetime / 10000LL) % 100;         // 时 0-23
	tm_utc.tm_min = (datetime / 100LL) % 100;           // 分 0-59
	tm_utc.tm_sec = datetime % 100;                     // 秒 0-59

	return _mkgmtime(&tm_utc);
}

std::wstring BrickPic::FormatTime(int64_t time)
{
	std::wstring str;
	int nTimeLevel = 0;
	while (time > MinSec)
	{
		if (time >= YearSec)
		{
			int nYear = time / YearSec;
			time -= (int64_t)nYear * YearSec;
			str += std::to_wstring(nYear) + L"年";
			nTimeLevel++;
		}
		else if (time >= MonthSec)
		{
			int nMonth = time / MonthSec;
			time -= (int64_t)nMonth * MonthSec;
			str += std::to_wstring(nMonth) + L"月";
			nTimeLevel++;
		}
		else if (time >= DaySec)
		{
			int nDay = time / DaySec;
			time -= (int64_t)nDay * DaySec;
			str += std::to_wstring(nDay) + L"天";
			nTimeLevel++;
		}
		else if (time >= HourSec)
		{
			int nHour = time / HourSec;
			time -= (int64_t)nHour * HourSec;
			str += std::to_wstring(nHour) + L"小时";
			nTimeLevel++;
		}
		else if (time >= MinSec)
		{
			int nMin = time / MinSec;
			time -= (int64_t)nMin * MinSec;
			str += std::to_wstring(nMin) + L"分";
			nTimeLevel++;
		}
		if (nTimeLevel >= 3)
			break;
	}
	if (nTimeLevel < 3)
		str += std::to_wstring(time) + L"秒";
	return str;
}


RenkoData BrickPic::FrontRehabCash(RenkoData& srcKline, int nDate)
{
	if (srcKline.date < nDate)
		return srcKline;
	RenkoData dstKline(srcKline);
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (dstKline.date >= m_RehabInfo[i].exDivDate)
			break;
		if (m_RehabInfo[i].allotmentRatio == 0)
		{
			if (m_RehabInfo[i].perCashDiv != 0)
			{
				dstKline.fStart -= m_RehabInfo[i].perCashDiv;
				dstKline.fEnd -= m_RehabInfo[i].perCashDiv;
			}

			double fRatio = m_RehabInfo[i].perShareDivRatio + m_RehabInfo[i].perShareTransRatio;
			if (fRatio != 0)
			{
				fRatio += 1;
				dstKline.fStart /= fRatio;
				dstKline.fEnd /= fRatio;
			}
		}
		else
		{
			dstKline.fStart /= m_RehabInfo[i].adjFactor;
			dstKline.fEnd /= m_RehabInfo[i].adjFactor;
		}


	}
	return dstKline;
}

RenkoData BrickPic::FrontRehabReInv(RenkoData& srcKline, int nDate)
{
	if (srcKline.date < nDate)
		return srcKline;
	RenkoData dstKline(srcKline);
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (dstKline.date >= m_RehabInfo[i].exDivDate)
			break;
		dstKline.fStart /= m_RehabInfo[i].adjFactor;
		dstKline.fEnd /= m_RehabInfo[i].adjFactor;
	}
	return dstKline;
}

RenkoData BrickPic::BackRehabCash(RenkoData& srcKline, int nDate)
{
	if (srcKline.date < nDate)
		return srcKline;
	RenkoData dstKline(srcKline);
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (dstKline.date < m_RehabInfo[i].exDivDate)
			continue;
		if (m_RehabInfo[i].allotmentRatio == 0)
		{
			double fRatio = m_RehabInfo[i].perShareDivRatio + m_RehabInfo[i].perShareTransRatio;
			if (fRatio != 0)
			{
				fRatio += 1;
				dstKline.fStart *= fRatio;
				dstKline.fEnd *= fRatio;
			}

			if (m_RehabInfo[i].perCashDiv != 0)
			{
				dstKline.fStart += m_RehabInfo[i].perCashDiv;
				dstKline.fEnd += m_RehabInfo[i].perCashDiv;
			}

		}
		else
		{
			dstKline.fStart *= m_RehabInfo[i].adjFactor;
			dstKline.fEnd *= m_RehabInfo[i].adjFactor;
		}

	}
	return dstKline;
}

RenkoData BrickPic::BackRehabReInv(RenkoData& srcKline, int nDate)
{
	if (srcKline.date < nDate)
		return srcKline;
	RenkoData dstKline(srcKline);
	for (int i = m_RehabInfo.size() - 1; i >= 0; --i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			continue;
		if (dstKline.date < m_RehabInfo[i].exDivDate)
			break;
		dstKline.fStart *= m_RehabInfo[i].adjFactor;
		dstKline.fEnd *= m_RehabInfo[i].adjFactor;
	}
	return dstKline;
}

double BrickPic::FrontRehabCash(double fPrice, int nDate)
{
	if (m_nTradingDay < nDate)
		return fPrice;
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (m_nTradingDay >= m_RehabInfo[i].exDivDate)
			break;
		if (m_RehabInfo[i].allotmentRatio == 0)
		{
			if (m_RehabInfo[i].perCashDiv != 0)
			{
				fPrice -= m_RehabInfo[i].perCashDiv;
			}

			double fRatio = m_RehabInfo[i].perShareDivRatio + m_RehabInfo[i].perShareTransRatio;
			if (fRatio != 0)
			{
				fRatio += 1;
				fPrice /= fRatio;
			}
		}
		else
		{
			fPrice /= m_RehabInfo[i].adjFactor;
		}


	}
	return fPrice;
}

double BrickPic::FrontRehabReInv(double fPrice, int nDate)
{
	if (m_nTradingDay < nDate)
		return fPrice;
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (m_nTradingDay >= m_RehabInfo[i].exDivDate)
			break;
		fPrice /= m_RehabInfo[i].adjFactor;
	}
	return fPrice;
}

double BrickPic::BackRehabCash(double fPrice, int nDate)
{
	if (m_nTradingDay < nDate)
		return fPrice;
	for (int i = 0; i < m_RehabInfo.size(); ++i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			break;
		if (m_nTradingDay < m_RehabInfo[i].exDivDate)
			continue;
		if (m_RehabInfo[i].allotmentRatio == 0)
		{
			double fRatio = m_RehabInfo[i].perShareDivRatio + m_RehabInfo[i].perShareTransRatio;
			if (fRatio != 0)
			{
				fRatio += 1;
				fPrice *= fRatio;
			}

			if (m_RehabInfo[i].perCashDiv != 0)
			{
				fPrice += m_RehabInfo[i].perCashDiv;
			}

		}
		else
		{
			fPrice *= m_RehabInfo[i].adjFactor;
		}

	}
	return fPrice;
}

double BrickPic::BackRehabReInv(double fPrice, int nDate)
{
	if (m_nTradingDay < nDate)
		return fPrice;
	for (int i = m_RehabInfo.size() - 1; i >= 0; --i)
	{
		if (m_RehabInfo[i].exDivDate < nDate)
			continue;
		if (m_nTradingDay < m_RehabInfo[i].exDivDate)
			break;
		fPrice *= m_RehabInfo[i].adjFactor;
	}
	return fPrice;
}

void BrickPic::SetWindowRect()
{
	if (!m_bShowDeal)
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top, m_rcAll.right, m_rcAll.bottom);
	else
	{
		if (m_bHalfPrice)
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top, m_rcAll.right + 40, m_rcAll.top + 375);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top + 378, m_rcAll.right + 40, m_rcAll.bottom + 30);

		}
		else
		{
			m_pPriceList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top, m_rcAll.right + 40, m_rcAll.top + 580);
			m_pDealList->m_rect.SetRect(m_rcAll.right - 182,
				m_rcAll.top + 585, m_rcAll.right + 40, m_rcAll.bottom + 30);

		}
		m_rcImage.SetRect(m_rcAll.left, m_rcAll.top,
			m_rcAll.right - 260, m_rcAll.bottom);
	}

	vector<CRect*>pSubRect;
	if (m_bShowGenTime)
		pSubRect.emplace_back(&m_rcGenTime);
	else m_rcGenTime.SetRectEmpty();



	int preBottom = m_rcImage.top;
	int nowBottom = m_rcImage.top +
		m_rcImage.Height() / (pSubRect.size() + 2) * 2;
	m_rcMain.SetRect(m_rcImage.left, m_rcImage.top, m_rcImage.right,
		m_rcImage.top + m_rcImage.Height() / (pSubRect.size() + 2) * 2);
	preBottom = nowBottom;
	SStringA str;
	for (int i = 0; i < pSubRect.size(); ++i)
	{
		nowBottom = m_rcImage.top + m_rcImage.Height()
			/ (pSubRect.size() + 2) * (3 + i);
		pSubRect[i]->SetRect(m_rcImage.left, preBottom,
			m_rcImage.right, nowBottom);
		preBottom = nowBottom;

	}

}


void SOUI::BrickPic::CalcShowRange()
{
	int nLen = m_rcMain.Width() - RC_RIGHT_BACK;	//判断是否超出范围
	m_nMaxKNum = nLen / TOTALZOOMWIDTH;
	int nTotal = m_DateTimeVec.size();
	if (m_bReSetFirstLine)
	{
		int nSelData = m_nNowPosition + m_nFirst;
		int nRightDataCount = nTotal - nSelData - 1;
		if (nRightDataCount > m_nMaxKNum / 2)
			m_nFirst = max(nSelData - m_nMaxKNum / 2, 0);
		else
			m_nFirst = max(nTotal - m_nMaxKNum, 0);
		m_nEnd = min(m_nFirst + m_nMaxKNum, nTotal);
		m_nMove = m_nFirst;
		m_nNowPosition = nSelData - m_nFirst;
		m_bReSetFirstLine = false;
	}

	if (m_nMove == -1)
	{
		m_nEnd = m_DateTimeVec.size();
		m_nFirst = max(0, nTotal - m_nMaxKNum);

	}
	if (m_nMove >= 0)
	{
		m_nFirst = m_nMove;
		m_nEnd = min(m_nFirst + m_nMaxKNum, nTotal);
	}
}

void SOUI::BrickPic::SetPicBarWidth(int nPicWidth, int nDataCount)
{
	double  fTotalWidth = nPicWidth * 1.0 / nDataCount;
	if (fTotalWidth < 1)
	{
		m_nKWidth = 1;
		m_nGap = 0;
		m_fZoomRatio = 1 / fTotalWidth;
	}
	else if (fTotalWidth < 2)
	{
		m_nKWidth = 1;
		m_nGap = 1;
		m_fZoomRatio = 1.0 * (m_nKWidth + m_nGap) * nDataCount / nPicWidth;
	}
	else if (fTotalWidth < 3)
	{
		m_nKWidth = 1;
		m_nGap = 2;
		m_fZoomRatio = 1.0 * (m_nKWidth + m_nGap) * nDataCount / nPicWidth;
	}
	else if (fTotalWidth < 47 + 2)
	{
		m_nGap = 2;
		m_nKWidth = fTotalWidth - m_nGap;
		if (m_nKWidth % 2 == 0)
			m_nKWidth -= 1;

		m_fZoomRatio = 1.0 * (m_nKWidth + m_nGap) * nDataCount / nPicWidth;
	}
	else
	{
		m_nKWidth = MAX_KWIDTH;
		m_nGap = fTotalWidth - m_nKWidth;
		//if (m_nGap % 2 == 0)
		//	m_nGap -= 1;
		m_fZoomRatio = 1.0 * (m_nKWidth + m_nGap) * nDataCount / nPicWidth;

	}
	m_nMove = -1;
}

void SOUI::BrickPic::OnPaint(IRenderTarget* pRT)
{
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);



	if (!m_bPaintInit)
	{
		m_bPaintInit = TRUE;

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 12;// 字体大小
		_stprintf(lf.lfFaceName, L"%s", L"微软雅黑"); // 字体名称
		GETRENDERFACTORY->CreateFont(&m_pFont12, lf);

		COLORREF cl = RGBA(176, 196, 222, 100);
		pRT->CreatePen(PS_SOLID, cl, 1, &m_penGrey);
		pRT->CreateSolidColorBrush(cl, &m_bBrushGrey);
		pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 255), &m_bBrushBlack);
		pRT->CreateSolidColorBrush(RGBA(50, 100, 255, 255), &m_bBrushDarkBlue);
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 255), 1, &m_penRed);
		pRT->CreatePen(PS_SOLID, RGBA(0, 255, 255, 255), 1, &m_penGreen);
		pRT->CreatePen(PS_SOLID, RGBA(0, 255, 0, 255), 1, &m_penMAGreen);
		pRT->CreatePen(PS_DOT, RGBA(0, 255, 0, 255), 1, &m_penDotGreen);

		pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255, 255), 1, &m_penWhite);
		pRT->CreateSolidColorBrush(RGBA(0, 255, 255, 255), &m_bBrushGreen);
		pRT->CreatePen(PS_SOLID, RGBA(255, 255, 000, 255), 1, &m_penYellow);
		pRT->CreatePen(PS_DOT, RGBA(255, 255, 000, 255), 1, &m_penDotYellow);
		pRT->CreatePen(PS_DOT, RGBA(255, 0, 000, 255), 1, &m_penDotRed);

		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 255, 255), 1, &m_penPurple);
		pRT->CreatePen(PS_SOLID, RGBA(100, 100, 100, 255), 1, &m_penGray);


	}

	pRT->SetAttribute(L"antiAlias", L"0", FALSE);


	SWindow::GetClientRect(&m_rcAll);
	pRT->FillSolidRect(m_rcAll, RGBA(0, 0, 0, 255));
	m_rcAll.DeflateRect(RC_LEFT + 5, RC_TOP, RC_RIGHT + 5, RC_BOTTOM);
	SetWindowRect();

	CPoint pts[5];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(192, 192, 192, 255), 2, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);
		pts[0].SetPoint(m_rcAll.left - 49, m_rcAll.top - 25);
		pts[1].SetPoint(m_rcAll.left - 49, m_rcAll.bottom + 35);
		pts[2].SetPoint(m_rcAll.right + RC_RIGHT + 1, m_rcAll.bottom + 35);
		pts[3].SetPoint(m_rcAll.right + RC_RIGHT + 1, m_rcAll.top - 25);
		pts[4] = pts[0];
		pRT->DrawLines(pts, 5);
		if (m_bShowDeal)
		{
			pts[0].SetPoint(m_rcImage.right + RC_RIGHT + 1, m_rcAll.top - 25);
			pts[1].SetPoint(m_rcImage.right + RC_RIGHT + 1, m_rcAll.bottom + 35);
			pRT->DrawLines(pts, 2);
			pts[0].SetPoint(m_rcImage.right + RC_RIGHT + 1, m_pPriceList->m_rect.bottom - 5);
			pts[1].SetPoint(m_rcAll.right + RC_RIGHT + 1, m_pPriceList->m_rect.bottom - 5);
			pRT->DrawLines(pts, 2);

		}

		pRT->SelectObject(oldPen);
	}


	if (m_bDataInited)
	{

		CalcShowRange();
		GetMaxDiff();

		DrawTime(pRT, TRUE);
		DrawArrow(pRT);
		//DrawPrice(pRT);
		DrawData(pRT);
		//		}

		CPoint po(m_PtMouse.x, m_PtMouse.y);
		m_PtMouse.x = m_PtMouse.y = -1;
		LONGLONG llTmp3 = GetTickCount64();
		if (m_bKeyDown)
			DrawKeyDownLine(pRT, true);
		else
			DrawMouse(pRT, po, TRUE);
	}
	else
		DrawArrow(pRT);
	if (m_bShowDeal && m_pStkMarketVec != nullptr)
	{
		m_pPriceList->Paint(pRT);
		m_pDealList->Paint(pRT);
	}
	AfterPaint(pRT, pa);
}

void SOUI::BrickPic::DrawArrow(IRenderTarget* pRT)
{
	int nLen = m_rcMain.bottom - m_rcMain.top;
	int nYoNum = 9;		//y轴标示数量 9 代表画8根线
	CPoint pts[4];
	{
		CAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 2, &pen);
		pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(m_rcMain.left, m_rcMain.top);
		pts[1].SetPoint(m_rcMain.right, m_rcMain.top);
		pts[2].SetPoint(m_rcMain.right, m_rcMain.bottom);
		pts[3].SetPoint(m_rcMain.left, m_rcMain.bottom);
		//pts[4] = pts[0];
		pRT->DrawLines(pts, 4);
		pRT->SelectObject(oldPen);
	}
	pRT->SetTextColor(RGBA(255, 0, 0, 255));

	//k线区横向虚线
	COLORREF clRed = RGB(255, 0, 0);
	HDC pdc = pRT->GetDC();
	for (int i = 0; i < nYoNum - 1; i++)
	{
		int nY = m_rcMain.top + ((m_rcMain.bottom - m_rcMain.top - INFOHEIGHT) / (nYoNum - 1) * i) + INFOHEIGHT;
		if (i == 0)
		{
			CPoint pts[2];
			{
				CAutoRefPtr<IPen> oldPen, pen;
				//	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
				pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
				pRT->SelectObject(pen, (IRenderObj**)&oldPen);
				//y轴	//x轴
				pts[0].SetPoint(m_rcMain.left, nY);
				pts[1].SetPoint(m_rcMain.right, nY);
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen);
			}
		}
		else
		{
			for (int j = m_rcMain.left + 1; j < m_rcMain.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线
		}
		//k线区y轴加轴标
		if (m_bDataInited)
		{

			SStringW s1 = GetYPrice(nY);
			double fPrice = _wtof(s1);
			if (fPrice<15000 && fPrice>-15000)
				DrawTextonPic(pRT, CRect(m_rcMain.right + 2, nY - 9, m_rcMain.right + RC_RIGHT, nY + 16), s1, RGBA(255, 0, 0, 255), DT_CENTER);
			//		pRT->TextOut(m_rcUpper.left - RC_LEFT + 8, nY - 6, s1, -1);
		}
	}
	pRT->ReleaseDC(pdc);
	if (m_bShowGenTime)
		DrawVolAmoArrow(pRT, m_rcGenTime);
}

void BrickPic::DrawVolAmoArrow(IRenderTarget* pRT, CRect& rc)
{
	COLORREF clRed = RGB(255, 0, 0);
	CPoint pts[4];
	{
		CAutoRefPtr<IPen> oldPen;
		//	pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 0xFF), 1, &pen);
		pRT->SelectObject(m_penRed, (IRenderObj**)&oldPen);
		//y轴	//x轴
		pts[0].SetPoint(rc.left, rc.top);
		pts[1].SetPoint(rc.right, rc.top);
		pts[2].SetPoint(rc.right, rc.bottom);
		pts[3].SetPoint(rc.left, rc.bottom);
		//pts[4] = pts[0];
		pRT->DrawLines(pts + 1, 3);
		pRT->SelectObject(oldPen);

	}

	//副图区横向虚线
	HDC pdc = pRT->GetDC();

	int nY = rc.top + INFOHEIGHT;
	CAutoRefPtr<IPen> pen, oldPen;
	pRT->CreatePen(PS_SOLID, RGBA(200, 0, 0, 0xFF), 2, &pen);
	pRT->SelectObject(pen, (IRenderObj**)&oldPen);
	//y轴	//x轴
	pts[0].SetPoint(rc.left, nY);
	pts[1].SetPoint(rc.right, nY);
	pRT->DrawLines(pts, 2);
	pRT->SelectObject(oldPen);

	SStringW sr;
	for (int i = 1; i <= 4; i++)
	{
		int nY = rc.bottom - ((rc.bottom - rc.top - INFOHEIGHT) / 4 * i);
		if (i == 4)
			nY -= 5;
		else
			for (int j = rc.left + 1; j < rc.right; j += 3)
				::SetPixelV(pdc, j, nY, clRed);		//	划虚线

		//标注

		if (m_bDataInited)
		{
			sr = GetPosGenTime(nY);

			DrawTextonPic(pRT, CRect(m_rcMain.right, nY - 9, m_rcImage.right + RC_RIGHT - 2, nY + 9),
				sr, RGBA(255, 0, 0, 255), DT_CENTER);

		}
	}
	pRT->ReleaseDC(pdc);
}

void SOUI::BrickPic::DrawMouse(IRenderTarget* pRT, CPoint p, BOOL bFromOnPaint)
{
	if (p.x == m_PtMouse.x && p.y == m_PtMouse.y)
		return;
	if (!IsInRect(p.x, p.y, 0))
	{
		if (m_PtMouse.x != -1 || m_PtMouse.y != -1)
		{
			m_PtMouse.x = m_PtMouse.y = -1;
			if (!bFromOnPaint)
			{
				Invalidate();
			}
		}
		return;
	}

	if (m_bShowMouseLine)
		DrawMouseLine(pRT, p);

	//显示当前K线的具体信息和指标信息
	int nNum = GetXData(p.x);
	nNum = nNum < m_nEnd ? nNum : m_nEnd - 1;
	pRT->SelectObject(m_bBrushBlack);
	if (nNum >= 0 && nNum < m_DateTimeVec.size())
	{
		if (m_bShowMouseLine)
			DrawBarInfo(pRT, nNum);
	}
	SStringW strTemp, strDot;

	//显示横坐标所在数值
	DrawPrice(pRT);
	DrawMovePrice(pRT, m_PtMouse.y, false);
	DrawPrice(pRT);
	DrawMovePrice(pRT, p.y, true);


	//显示纵坐标数值

	DrawTime(pRT);

	if (IsInRect(m_PtMouse.x, m_PtMouse.y, 0))
		DrawMoveTime(pRT, m_PtMouse.x, 0, 0, false);

	DrawTime(pRT);

	if (IsInRect(p.x, p.y, 0))
	{
		int nx = GetXData(p.x);
		if (nx >= 0 && nx < m_DateTimeVec.size())
			DrawMoveTime(pRT, p.x, m_DateTimeVec[nx] / 1'000'000, m_DateTimeVec[nx] % 1'000'000, true);
	}

	m_PtMouse.x = p.x;
	m_PtMouse.y = p.y;
}

void SOUI::BrickPic::DrawTime(IRenderTarget* pRT, BOOL bFromOnPaint)
{
	int nXpre = 0;  //第一根竖线的x轴位置
	int nMaX = (m_nEnd - m_nFirst - 1) * TOTALZOOMWIDTH
		+ RC_LEFT + 1 + ZOOMWIDTH / 2;	//最后一个数据的位置

	CAutoRefPtr<IPen> oldPen;
	CAutoRefPtr<IBrush> bOldBrush;
	pRT->SelectObject(m_penDotRed, (IRenderObj**)&oldPen);
	CPoint pts[5];
	HDC pdc = pRT->GetDC();

	int nPosMx[20] = { 0 };
	int nPosCount = 1;

	int nPerDate = 0;
	int nLength = 60;

	for (int i = 0; i < m_nEnd - m_nFirst; i++)
	{
		int x = i * TOTALZOOMWIDTH
			+ 1 + m_rcMain.left + (ZOOMWIDTH / 2);
		int preX = (nPosMx[nPosCount - 1]) * TOTALZOOMWIDTH +
			1 + m_rcMain.left + (ZOOMWIDTH / 2);
		//加时间
		if ((x - nXpre - RC_LEFT >= 40 && x < nMaX - 200) || (i == 0 && m_nEnd - m_nFirst > 9) || i == m_nEnd - m_nFirst - 1)
		{
			//加时间
			int nDate = m_DateTimeVec[i + m_nFirst] / 1'000'000;
			if (nDate != nPerDate)
			{
				nPerDate = nDate;
				if ((x - nXpre >= nLength && x < nMaX - nLength) || (i == 0))
				{
					nXpre = x;
					SStringW strDate;
					strDate.Format(L"%02d/%02d/%02d",
						nDate / 10000 % 100,
						nDate % 10000 / 100,
						nDate % 100);
					DrawTextonPic(pRT, CRect(x, m_rcImage.bottom + 5, x + 120, m_rcImage.bottom + 20), strDate);

				}

			}
		}
	}
	pRT->ReleaseDC(pdc);
	pRT->SelectObject(oldPen);

}

void SOUI::BrickPic::GetMaxDiff()
{
	m_fMaxY = INT_MIN;
	m_fMinY = DBL_MAX;
	m_nMaxVol = INT_MIN;
	m_nMinVol = 0;
	m_fMaxGenTime = 0;
	m_fMinGenTime = 0;

	for (int i = m_nFirst; i < m_nEnd; ++i)
	{
		if (High[i] > m_fMaxY)
			m_fMaxY = High[i];
		if (Low[i] < m_fMinY)
			m_fMinY = Low[i];
		if (GenTime[i] > m_fMaxGenTime)
			m_fMaxGenTime = GenTime[i];
	}
	m_fMaxGenTime *= exp(1.05);

	if (m_nMainTarget == eBMT_Pendant)
	{
		for (int i = m_nFirst; i < m_nEnd; ++i)
		{
			for (int j = 0; j < m_fPendantPara.size(); ++j)
			{
				if (m_fPendantPara[j] > 0 && i < m_PendantDataVec.size() && j < m_PendantDataVec[i].size()
					&& !isnan(m_PendantDataVec[i][j]))
				{
					if (m_PendantDataVec[i][j] > m_fMaxY)
						m_fMaxY = m_PendantDataVec[i][j];
					if (m_PendantDataVec[i][j] < m_fMinY)
						m_fMinY = m_PendantDataVec[i][j];
				}
			}
		}

	}

	double fDiff = m_fMaxY - m_fMinY;
	if (fDiff == 0)
		fDiff = 1;
	m_fMaxY += fDiff / 8;
	m_fMinY -= fDiff / 8;
}

BOOL SOUI::BrickPic::IsInRect(int x, int y, int nMode)
{
	CRect* prc;
	switch (nMode)
	{
	case 0:
		prc = &m_rcImage;
		break;
	case 1:
		prc = &m_rcMain;
		break;
	case 2:
		//prc = &m_rcVolume;
		break;
	case 3:
	{
		//CRect rc = m_rcMain;
		//if (!m_rcMACD.IsRectNull())
		//	rc.bottom = m_rcMACD.bottom;
		//else if (!m_rcVolume.IsRectNull())
		//	rc.bottom = m_rcVolume.bottom;
		//prc = &rc;
	}
	default:
		return FALSE;
	}

	if (x >= prc->left && x <= prc->right &&
		y >= prc->top && y <= prc->bottom)
		return TRUE;
	return FALSE;
}

SStringW SOUI::BrickPic::GetYPrice(int nY)
{
	SStringW strRet; strRet.Empty();
	int nHeight = m_rcMain.bottom - m_rcMain.top - INFOHEIGHT;
	double fPriceDiff = m_fMaxY - m_fMinY;
	double fDiff = m_fMinY + (double)(m_rcMain.bottom - nY)
		/ nHeight * fPriceDiff;
	if (fDiff < 1000)
		strRet.Format(L"%.02f", fDiff);
	else
		strRet.Format(L"%.0f", fDiff);

	return strRet;
}

SStringW SOUI::BrickPic::GetPosGenTime(int nY)
{
	SStringW strRet; strRet.Empty();
	if (nY > m_rcGenTime.bottom || nY < m_rcGenTime.top)
		return strRet;
	double fDiff = ((double)m_rcGenTime.bottom - nY) / (m_rcGenTime.Height() - INFOHEIGHT) * log(m_fMaxGenTime);
	strRet.Format(L"%.02f", fDiff);
	return strRet;
}

void SOUI::BrickPic::DrawData(IRenderTarget* pRT)
{
	if (m_DateTimeVec.empty())
		return;
	if (m_nEnd - m_nFirst <= 0)
		return;
	DrawBarInfo(pRT, m_nEnd - 1);
	std::vector<std::vector<CPoint>>MainTargetLine;
	SetMainLineSize(MainTargetLine, m_nEnd - m_nFirst);
	for (int i = 0; i < m_nEnd - m_nFirst; ++i)
	{
		int nOffset = i + m_nFirst;

		int nX = i * TOTALZOOMWIDTH + 1 + m_rcMain.left;
		int nXMid = nX + ZOOMWIDTH * 0.5;
		DrawBrick(pRT, i, nX);
		AddDataToMainTargetLine(MainTargetLine, i, nX);
		if (m_bShowGenTime)
			DrawBarChartData(pRT, m_rcGenTime, log(GenTime[nOffset] + 1), log(m_fMaxGenTime + 1), log(m_fMinGenTime + 1), nX, i);

	}
	if (m_nMainTarget == eBMT_Pendant)
		DrawPendant(pRT, MainTargetLine);

}

int SOUI::BrickPic::GetXData(int nx)
{
	float fn = (float)(nx - m_rcMain.left) /
		(float)TOTALZOOMWIDTH;
	int n = (int)fn;
	if (n < 0)
		n = 0;
	n += m_nFirst;
	return n;
}

int SOUI::BrickPic::GetXPos(int nx)
{
	int nPos = nx * TOTALZOOMWIDTH
		+ 1 + m_rcMain.left;
	nPos = nPos + ZOOMWIDTH / 2;
	return nPos;
}

int SOUI::BrickPic::GetYPos(double fDiff)
{
	double fPos = fDiff - m_fMinY;
	double fPriceDiff = m_fMaxY - m_fMinY;
	int nHeight = m_rcMain.bottom - m_rcMain.top - INFOHEIGHT;
	fPos = m_rcMain.bottom - fPos / fPriceDiff * nHeight + 0.5;
	int nPos = (int)fPos;
	return nPos;
}

int SOUI::BrickPic::GetYPos(double fY, const CRect& rc, double fMax, double fMin, int nInfoCount)
{
	int height = rc.Height() - nInfoCount * INFOHEIGHT;
	double fDelta = fMax - fMin;
	double fPos = rc.top + nInfoCount * INFOHEIGHT + (1 - ((fY - fMin) / fDelta)) * height;
	return fPos;
}

void SOUI::BrickPic::OnMouseMove(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);

	if (!m_bDataInited)
		return;
	if (point == m_PtMouse)
		return;
	if (m_bKeyDown)
	{
		m_bKeyDown = false;
		m_PtMouse = point;
		Invalidate();
		return;
	}

	CPoint p = point;

	CRect rc = GetClientRect();
	CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rc, 3, 0);
	SPainter pa;
	SWindow::BeforePaint(pRT, pa);
	DrawMouse(pRT, point);
	AfterPaint(pRT, pa);
	ReleaseRenderTarget(pRT);
}

void SOUI::BrickPic::OnMouseLeave()
{
	SetMsgHandled(FALSE);
	if (m_PtMouse.x != -1 || m_PtMouse.y != -1)
	{
		m_PtMouse.x = m_PtMouse.y = -1;
		Invalidate();		//主要是为了消除未来得及消除的鼠标线
	}

}

void SOUI::BrickPic::OnSize(UINT nType, CSize size)
{
	if (size.cx < 350)
		m_nKWidth = 8;
	else if (size.cx < 500)
		m_nKWidth = 12;
	else
		m_nKWidth = K_WIDTH_TOTAL;
}

void SOUI::BrickPic::DrawTextonPic(IRenderTarget* pRT, CRect rc, SStringW str,
	COLORREF color, UINT uFormat, DWORD rop)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.right - rc.left, rc.bottom - rc.top);
	CAutoRefPtr<IFont> oldFont;
	pMemRT->SelectObject(m_pFont12, (IRenderObj**)&oldFont);

	pMemRT->SetTextColor(color);				//字为白色
	CRect newRc(0, 0, rc.right - rc.left, rc.bottom - rc.top);
	pMemRT->DrawTextW(str, wcslen(str), newRc, uFormat);
	pRT->BitBlt(rc, pMemRT, 0, 0, rop);

}

void SOUI::BrickPic::DrawKeyDownLine(IRenderTarget* pRT, bool ClearTip)
{
	if (m_DateTimeVec.size() == 0)
		return;


	int nx = m_nNowPosition + m_nFirst;
	int x = m_nNowPosition * TOTALZOOMWIDTH
		+ 1 + m_rcMain.left;
	CPoint po;
	po.x = x + ZOOMWIDTH / 2;
	po.y = GetYPos(Close[nx]);
	DrawMouseLine(pRT, po);
	DrawBarInfo(pRT, nx);

	DrawPrice(pRT);
	DrawMovePrice(pRT, m_PtMouse.y, false);
	DrawPrice(pRT);
	DrawMovePrice(pRT, po.y, true);

	DrawTime(pRT);
	if (IsInRect(m_PtMouse.x, m_PtMouse.y, 0))
		DrawMoveTime(pRT, m_PtMouse.x, 0, 0, false);
	DrawTime(pRT);

	if (IsInRect(po.x, po.y, 0) && nx < m_DateTimeVec.size())
		DrawMoveTime(pRT, po.x, m_DateTimeVec[nx] / 1'000'000,
			m_DateTimeVec[nx] % 1'000'000, true);

	m_PtMouse = po;
}

void SOUI::BrickPic::DrawMouseLine(IRenderTarget* pRT, CPoint p)
{
	HDC hdc = pRT->GetDC();
	int  nMode = SetROP2(hdc, R2_NOTXORPEN);
	MoveToEx(hdc, m_PtMouse.x, m_rcMain.top + INFOHEIGHT, NULL);	LineTo(hdc, m_PtMouse.x, m_rcMain.bottom);	//竖线
	MoveToEx(hdc, p.x, m_rcMain.top + INFOHEIGHT, NULL);			LineTo(hdc, p.x, m_rcMain.bottom);

	MoveToEx(hdc, m_rcMain.left, m_PtMouse.y, NULL);	LineTo(hdc, m_rcMain.right, m_PtMouse.y);	//横线
	MoveToEx(hdc, m_rcMain.left, p.y, NULL);			LineTo(hdc, m_rcMain.right, p.y);

	if (m_bShowGenTime)
	{
		MoveToEx(hdc, m_PtMouse.x, m_rcGenTime.top + INFOHEIGHT, NULL);	LineTo(hdc, m_PtMouse.x, m_rcGenTime.bottom);	//竖线
		MoveToEx(hdc, p.x, m_rcGenTime.top + INFOHEIGHT, NULL);			LineTo(hdc, p.x, m_rcGenTime.bottom);
	}

	SetROP2(hdc, nMode);

	pRT->ReleaseDC(hdc);
}

CRect SOUI::BrickPic::GetTextDrawRect(IRenderTarget* pRT, SStringW str, CRect rc)
{
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.right - rc.left, rc.bottom - rc.top);
	CAutoRefPtr<IFont> oldFont;
	pMemRT->SelectObject(m_pFont12, (IRenderObj**)&oldFont);
	CRect dstRc(0, 0, rc.right - rc.left, rc.bottom - rc.top);

	pMemRT->DrawTextW(str, wcslen(str), dstRc, DT_CALCRECT);
	return dstRc;
}

void SOUI::BrickPic::DrawMoveTime(IRenderTarget* pRT, int x, int date, int time, bool bNew)
{
	CRect rc(x + 2, m_rcImage.bottom + 2, x + 120, m_rcImage.bottom + 20);
	if (bNew)
	{
		if (date == 0)
			return;
		SStringW str;
		str.Format(L"%02d/%02d/%02d  %02d:%02d:%02d", date / 10000 % 100,
			date % 10000 / 100,
			date % 100,
			time / 10000,
			time % 10000 / 100,
			time % 100);

		CRect drawRc = GetTextDrawRect(pRT, str, rc);
		drawRc.SetRect(x, m_rcImage.bottom + 2, x + drawRc.right + 4, m_rcImage.bottom + 20);
		CAutoRefPtr<IBrush> oldBrush;
		pRT->SelectObject(m_bBrushDarkBlue, (IRenderObj**)&oldBrush);
		pRT->FillRectangle(drawRc);
		pRT->SelectObject(oldBrush);
		drawRc.left += 2;
		DrawTextonPic(pRT, drawRc,
			str, RGBA(255, 255, 255, 255), DT_SINGLELINE | DT_VCENTER, MERGECOPY);

	}
	else
	{
		rc.left -= 2;
		rc.right = min(rc.right, m_rcImage.right + RC_RIGHT - 1);
		pRT->FillRectangle(rc);
	}

}

void SOUI::BrickPic::DrawPrice(IRenderTarget* pRT)
{
	int nLen = m_rcMain.bottom - m_rcMain.top;
	int nYoNum = 9;		//y轴标示数量 9 代表画8根线
	for (int i = 0; i < nYoNum - 1; i++)
	{
		int nY = m_rcMain.top + ((m_rcMain.bottom - m_rcMain.top - INFOHEIGHT) / (nYoNum - 1) * i) + INFOHEIGHT;
		//k线区y轴加轴标
		if (m_bDataInited)

		{
			SStringW s1 = GetYPrice(nY);
			double fPrice = _wtof(s1);
			DrawTextonPic(pRT, CRect(m_rcMain.right + 2, nY - 9, m_rcMain.right + RC_RIGHT, nY + 16),
				s1, RGBA(255, 0, 0, 255), DT_CENTER);

		}
	}
	if (m_bShowGenTime)
		DrawVolAmoPrice(pRT, m_rcGenTime);

}


void BrickPic::DrawVolAmoPrice(IRenderTarget* pRT, CRect& rc)
{
	for (int i = 1; i < 5; i++)
	{
		int nY = rc.top + INFOHEIGHT;
		{
			int nY = rc.bottom - ((rc.bottom - rc.top - INFOHEIGHT) / 4 * i);
			if (i == 4)
				nY -= 5;													//标注

			if (m_bDataInited)
			{
				SStringW sr = GetPosGenTime(nY);
				DrawTextonPic(pRT, CRect(rc.right, nY - 9,
					rc.right + RC_RIGHT - 2, nY + 9),
					sr, RGBA(255, 0, 0, 255), DT_CENTER);
			}
		}
	}
}
void SOUI::BrickPic::DrawMovePrice(IRenderTarget* pRT, int y, bool bNew)
{
	CRect rc(m_rcMain.right + 1, y - 15, m_rcMain.right + RC_RIGHT - 1, y);
	if (bNew)
	{
		SStringW sr;
		if (y >= m_rcMain.top + INFOHEIGHT && y <= m_rcMain.bottom)
			sr = GetYPrice(y);
		if (y >= m_rcGenTime.top + INFOHEIGHT && y <= m_rcGenTime.bottom)
			sr = GetPosGenTime(y);
		if (sr != L"")
		{
			CAutoRefPtr<IBrush> oldBrush;
			pRT->SelectObject(m_bBrushDarkBlue, (IRenderObj**)&oldBrush);
			pRT->FillRectangle(rc);
			pRT->SelectObject(oldBrush);
			rc.left += 1;
			rc.right -= 1;
			DrawTextonPic(pRT, rc, sr,
				RGBA(255, 255, 255, 255), DT_SINGLELINE | DT_CENTER, MERGECOPY);
		}
	}
	else
		pRT->FillRectangle(rc);
}

void SOUI::BrickPic::DrawBarInfo(IRenderTarget* pRT, int nDataPos)
{
	CAutoRefPtr<IBrush> oldBrush;
	pRT->SelectObject(m_bBrushBlack, (IRenderObj**)&oldBrush);
	pRT->FillRectangle(CRect(m_rcImage.left, m_rcImage.top - INFOHEIGHT, m_rcImage.right, m_rcImage.top));
	pRT->FillRectangle(CRect(m_rcImage.left + 1, m_rcImage.top + 4, m_rcImage.right, m_rcImage.top + INFOHEIGHT));
	DrawUpperMarket(pRT, nDataPos);
	if (m_nMainTarget == eBMT_Pendant)
		DrawMainUpperPendant(pRT, nDataPos, m_PendantDataVec[nDataPos]);
	if (m_bShowGenTime)
		DrawGenTimeUppaerInfo(pRT, GenTime[nDataPos]);
	pRT->SelectObject(oldBrush);

}


void SOUI::BrickPic::DrawUpperMarket(IRenderTarget* pRT, int nX)
{
	SStringW strMarket;
	int nDate = m_DateTimeVec[nX] / 1000000;
	int nTime = m_DateTimeVec[nX] % 1000000;

	strMarket.Format(L"%s 砖型图(%g%s) 日期:%04d-%02d-%02d 时间:%02d:%02d:%02d 开:%.02f 高:%.02f 低:%.02f 收:%.02f 生成时间:%s",
		StrA2StrW(m_strSubIns),
		m_fBrickSetting, strBrickType.at(m_nBrickType),
		nDate / 10000, nDate % 10000 / 100, nDate % 100,
		nTime / 10000, nTime / 100 % 100, nTime % 100,
		Open[nX], High[nX], Low[nX], Close[nX], FormatTime(GenTime[nX]).c_str());
	DrawTextonPic(pRT, CRect(m_rcImage.left + 5, m_rcImage.top - INFOHEIGHT, m_rcImage.right, m_rcImage.top), strMarket);
}

void SOUI::BrickPic::DrawMainUpperPendant(IRenderTarget* pRT, int nX, const vector<double>& data)
{
	HDC hdc = pRT->GetDC();

	SStringW strMarket = L"吊灯止损";
	CSize size;
	size.cx = 0; size.cy = 0;
	int left = 5;
	DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket,
		RGBA(255, 255, 255, 255));
	GetTextExtentPoint32(hdc, strMarket, strMarket.GetLength(), &size);
	left += size.cx;

	if (m_fPendantPara.size() > 0 && m_fPendantPara[0] > 0)
	{
		DWORD dwColor = RGBA(255, 255, 255, 255);
		if (data.size() > 2 && !isnan(data[2]))
		{
			if (data[2] == data[0])
			{
				strMarket.Format(L"多头止损(%%g砖):%%.0%df", m_nDecimal);
				strMarket.Format(strMarket, m_fPendantPara[0], data[0]);
			}
			else if (data[2] == data[1])
			{
				strMarket.Format(L"空头止损(%%g砖):%%.0%df", m_nDecimal);
				strMarket.Format(strMarket, m_fPendantPara[0], data[0]);
				dwColor = RGBA(0xFF, 0xFF, 000, 0xFF);
			}
		}
		else
			strMarket.Format(L"止损(%g砖):-", m_fPendantPara[0]);
		DrawTextonPic(pRT, CRect(m_rcImage.left + left, m_rcImage.top + 5, m_rcImage.right - 1, m_rcImage.top + 19), strMarket,
			dwColor);
	}

}

void SOUI::BrickPic::DrawGenTimeUppaerInfo(IRenderTarget* pRT, double fTime)
{
	pRT->FillRectangle(CRect(m_rcGenTime.left + 1, m_rcGenTime.top + 4, m_rcGenTime.right, m_rcGenTime.top + INFOHEIGHT));
	HDC hdc = pRT->GetDC();
	CSize size;
	size.cx = 0; size.cy = 0;
	int left = 5;
	SStringW strMarket;
	strMarket.Format(L"生成时间:%s(%.02f)", FormatTime(fTime).c_str(), log(fTime + 1));
	DrawTextonPic(pRT,
		CRect(m_rcGenTime.left + left, m_rcGenTime.top + 5,
			m_rcGenTime.right - 1, m_rcGenTime.top + 19),
		strMarket);

	pRT->ReleaseDC(hdc);
}

void SOUI::BrickPic::DrawBrick(IRenderTarget* pRT, int nPos, int nX)
{
	int nOffset = nPos + m_nFirst;
	int yhigh = GetYPos(High[nOffset]);
	int ylow = GetYPos(Low[nOffset]);
	int yopen = GetYPos(Open[nOffset]);
	int yclose = GetYPos(Close[nOffset]);
	DWORD dwColor = Close[nOffset] > Open[nOffset] ? RGBA(255, 0, 0, 255) : Close[nOffset] == Open[nOffset] ?
		RGBA(255, 255, 255, 255) : RGBA(0, 255, 255, 255);
	auto dwPen = Close[nOffset] > Open[nOffset] ? m_penRed : Close[nOffset] == Open[nOffset] ?
		m_penWhite : m_penGreen;
	CAutoRefPtr<IPen> oldPen;
	pRT->SelectObject(dwPen, (IRenderObj**)&oldPen);
	if (ZOOMWIDTH >= 1)
	{

		CRect rcBar;
		rcBar.left = nX;
		rcBar.top = min(yopen, yclose);
		rcBar.right = nX + ZOOMWIDTH;
		rcBar.bottom = max(yopen, yclose);
		CPoint pts[4];
		if (yhigh != rcBar.top)
		{
			pts[0].SetPoint(nX + ZOOMWIDTH / 2, yhigh);
			pts[1].SetPoint(nX + ZOOMWIDTH / 2, rcBar.top);
			pRT->DrawLines(pts, 2);
		}
		if (ylow != rcBar.bottom)
		{
			pts[0].SetPoint(nX + ZOOMWIDTH / 2, ylow);
			pts[1].SetPoint(nX + ZOOMWIDTH / 2, rcBar.bottom);
			pRT->DrawLines(pts, 2);
		}

		if (yopen == yclose)
		{
			pts[0].SetPoint(nX, yopen);
			pts[1].SetPoint(nX + ZOOMWIDTH, yopen);
			pRT->DrawLines(pts, 2);
		}
		else
		{
			if (m_BrickJumpPos.count(nOffset) == 0)
				pRT->FillSolidRect(rcBar, dwColor);
			else
				pRT->DrawRectangle(rcBar);


		}


	}
	else
	{
		CPoint pt[2];
		pt[0].SetPoint(nX, yhigh);
		pt[1].SetPoint(nX, ylow);
		pRT->DrawLines(pt, 2);

	}
	pRT->SelectObject(oldPen);

}

void SOUI::BrickPic::SetMainLineSize(std::vector<std::vector<CPoint>>& MainLine, int nDataCount)
{
	if (m_nMainTarget == eBMT_Pendant)
	{
		MainLine.resize(3);
		for (auto& lineVec : MainLine)
			lineVec.resize(nDataCount);
	}
}

void SOUI::BrickPic::AddDataToMainTargetLine(std::vector<std::vector<CPoint>>& MainLine, int nPos, int nX)
{
	int nOffset = nPos + m_nFirst;

	if (m_nMainTarget == eBMT_Pendant)
	{
		if (m_PendantDataVec.size() > nOffset)
		{
			auto& pendant = m_PendantDataVec[nOffset];
			if (pendant[2] == pendant[0])
			{
				MainLine[0][nPos] = { int(nX + ZOOMWIDTH * 0.5), GetYPos(pendant[2]) };
			}
			else
			{
				MainLine[1][nPos] = { int(nX + ZOOMWIDTH * 0.5), GetYPos(pendant[2]) };

			}
			if (pendant[3] == 1)
			{
				MainLine[2][nPos] = { int(nX + ZOOMWIDTH * 0.5), GetYPos(Low[nOffset]) };
			}
			else if (pendant[3] == -1)
			{
				MainLine[2][nPos] = { -int(nX + ZOOMWIDTH * 0.5), GetYPos(High[nOffset]) };
			}

		}
	}
}

void SOUI::BrickPic::DrawBarChartData(IRenderTarget* pRT, const CRect& rc, double data,
	double fMax, double fMin, int nX, int nShowPos)
{
	int nDataOffset = nShowPos + m_nFirst;

	if (data != 0)
	{
		//auto& kline = m_dataVec[nDataOffset];

		int nMinY = GetYPos(fMin, rc, fMax, fMin, SUBINFOCOUNT);
		int nMaxY = GetYPos(data, rc, fMax, fMin, SUBINFOCOUNT);

		if (ZOOMWIDTH >= 1)
		{
			CRect rcBar;
			rcBar.left = nX;
			rcBar.top = nMaxY;
			rcBar.right = nX + ZOOMWIDTH;
			rcBar.bottom = nMinY;

			if (Close[nDataOffset] > Open[nDataOffset])
				pRT->FillSolidRect(rcBar, RGBA(255, 0, 0, 255));
			else if (Close[nDataOffset] == Open[nDataOffset] && nDataOffset > 0)
			{
				if (Close[nDataOffset] >= Close[nDataOffset - 1])
					pRT->FillSolidRect(rcBar, RGBA(255, 0, 0, 255));
				else
				{
					pRT->FillSolidRect(rcBar, RGBA(0, 255, 255, 255));
				}
			}
			else
			{
				pRT->FillSolidRect(rcBar, RGBA(0, 255, 255, 255));
			}

		}
		else
		{
			CPoint pts[2];
			pts[0].SetPoint(nX + ZOOMWIDTH / 2, nMaxY);
			pts[1].SetPoint(nX + ZOOMWIDTH / 2, nMinY);
			if (Close[nDataOffset] > Open[nDataOffset])
				pRT->SelectObject(m_penRed);
			else if (Close[nDataOffset] < Open[nDataOffset])
				pRT->SelectObject(m_penGreen);
			else
			{
				if (nDataOffset >= 1)
				{
					if (Close[nDataOffset] >= Close[nDataOffset - 1])
						pRT->SelectObject(m_penRed);
					else
						pRT->SelectObject(m_penGreen);
				}
				else
					pRT->SelectObject(m_penGreen);

			}
			pRT->DrawLines(pts, 2);

		}
	}
}

void SOUI::BrickPic::DrawPendant(IRenderTarget* pRT,
	std::vector<std::vector<CPoint>>& MaLine)
{
	DWORD dwLong = RGBA(255, 255, 255, 255);
	DWORD dwShort = RGBA(0xFF, 0xFF, 000, 0xFF);
	DWORD dwGreen = RGBA(0, 255, 255, 255);
	DWORD dwRed = RGBA(255, 0, 0, 255);;

	int nR = ZOOMWIDTH >= 2 ? min(max(ZOOMWIDTH / 2 - 1, 2), 3) : 1;
	//int nWidth = min(nR * 2,nR*1.5+1);
	int nWidth = 1;
	vector<CPoint> LineVec;
	LineVec.reserve(m_nEnd - m_nFirst);
	bool bLong = MaLine[0][0].x != 0;
	LineVec.emplace_back(bLong ? MaLine[0][0] : MaLine[1][0]);
	for (int i = 1; i < m_nEnd - m_nFirst; ++i)
	{
		if (bLong)
		{
			if (MaLine[0][i].x != 0)
				LineVec.emplace_back(MaLine[0][i]);
			else
			{
				//DrawLine(pRender, LineVec, ColorVec[0]);
				for (auto& pt : LineVec)
				{
					CRect rc = { pt.x - nR,pt.y - nR,pt.x + nR,pt.y + nR };
					pRT->FillSolidEllipse(&rc, dwLong);
				}
				LineVec.clear();
				//LineVec.emplace_back(MaLine[0][i - 1]);
				LineVec.emplace_back(MaLine[1][i]);
				bLong = false;
			}
		}
		else
		{
			if (MaLine[1][i].x != 0)
				LineVec.emplace_back(MaLine[1][i]);
			else
			{
				//DrawLine(pRender, LineVec, ColorVec[1]);
				for (auto& pt : LineVec)
				{
					CRect rc = { pt.x - nR,pt.y - nR,pt.x + nR,pt.y + nR };
					pRT->FillSolidEllipse(&rc, dwShort);
				}

				LineVec.clear();
				//LineVec.emplace_back(MaLine[1][i - 1]);
				LineVec.emplace_back(MaLine[0][i]);
				bLong = true;
			}
		}
	}
	if (!LineVec.empty())
	{
		auto dwColor = bLong ? dwLong : dwShort;
		for (auto& pt : LineVec)
		{
			CRect rc = { pt.x - nR,pt.y - nR,pt.x + nR,pt.y + nR };
			pRT->FillSolidEllipse(&rc, dwColor);

		}

	}
	for (int i = 0; i < m_nEnd - m_nFirst; ++i)
	{
		if (MaLine[2][i].x > 0)
		{
			DrawTextonPic(pRT, CRect(MaLine[2][i].x - 5, MaLine[2][i].y + 5, MaLine[2][i].x + 5,
				MaLine[2][i].y + 15), L"▲", dwGreen, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			SStringW strPrice;
			strPrice.Format(strPrice.Format(L"%%.0%df", m_nDecimal), m_PendantDataVec[m_nFirst + i][4]);
			SIZE sz = { 0 };
			pRT->MeasureText(strPrice, strPrice.GetLength(), &sz);
			int nWidth = sz.cx / 2 + 1;
			DrawTextonPic(pRT, CRect(MaLine[2][i].x - nWidth, MaLine[2][i].y + 15, MaLine[2][i].x + nWidth,
				MaLine[2][i].y + 15 + sz.cy), strPrice, dwGreen, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
		else if (MaLine[2][i].x < 0)
		{

			DrawTextonPic(pRT, CRect(-MaLine[2][i].x - 5, MaLine[2][i].y - 15, -MaLine[2][i].x + 5,
				MaLine[2][i].y - 5), L"▼", dwRed, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			SStringW strPrice;
			strPrice.Format(strPrice.Format(L"%%.0%df", m_nDecimal), m_PendantDataVec[m_nFirst + i][4]);
			SIZE sz = { 0 };
			pRT->MeasureText(strPrice, strPrice.GetLength(), &sz);
			int nWidth = sz.cx / 2 + 1;
			DrawTextonPic(pRT, CRect(-MaLine[2][i].x - nWidth, MaLine[2][i].y - 15 - sz.cy, -MaLine[2][i].x + nWidth,
				MaLine[2][i].y - 15), strPrice, dwRed, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}
}

