#include "stdafx.h"
#include "KlineTarget.h"
#include <numeric>
#include <deque>
using std::vector;
using std::accumulate;
using std::map;
#define MAX_LEFT 240
enum eKLINEDATA
{
	eKD_Open = 0,
	eKD_High,
	eKD_Low,
	eKD_Close,
	eKD_Vol,
	eKD_Amo,
};
map<int, int>CKlineTarget::m_klineDataPosMap = map<int, int>();

vector<TargetInfo> CKlineTarget::m_targetInfo = { { eTarget_NetGrid,"NetGrid",{ "N1","N2","N3" },{ 1000,200 ,20 } ,{ 1000,200,20 },
{ "ma","up1", "up2", "up3", "up4", "dw1", "dw2", "dw3", "dw4" },{},{},2,eTarget_Main,TRUE } };

CKlineTarget::CKlineTarget()
{
}


CKlineTarget::~CKlineTarget()
{
}

void CKlineTarget::ClearData()
{
	m_KlineDateTimeVec.clear();
	Open.clear();
	High.clear();
	Low.clear();
	Close.clear();
	std::lock_guard<std::mutex> lk(m_mx);
	for (auto &it : m_targetInfoMap)
	{
		for (auto& dataVec : it.OutVec)
		{
			dataVec.clear();
		}
		for (auto& dataVec : it.tmpDataVec)
		{
			dataVec.clear();
		}
		it.MarketPostion.clear();
		it.CurrentPos.clear();
		it.tradeMap.clear();
		it.bFirst = TRUE;
	}

}

void CKlineTarget::UpdateData(const KlineType dataArr[], int nCount)
{
	ClearData();
	int maxSize = nCount + MAX_LEFT;
	m_KlineDateTimeVec.reserve(maxSize);
	Open.reserve(maxSize);
	High.reserve(maxSize);
	Low.reserve(maxSize);
	Close.reserve(maxSize);
	for (int i = 0; i < nCount; ++i)
	{
		auto &data = dataArr[i];
		m_KlineDateTimeVec.emplace_back((int64_t)(data.date) * 10000 + (int64_t)data.time / 100);
		Open.emplace_back(data.open);
		High.emplace_back(data.high);
		Low.emplace_back(data.low);
		Close.emplace_back(data.close);
	}
	CalcData();
}

void CKlineTarget::UpdateData(const KlineType & data)
{
	int64_t time = (int64_t)(data.date) * 10000 + (int64_t)data.time ;
	if (m_KlineDateTimeVec.empty() || m_KlineDateTimeVec.back() != time)
	{
		m_KlineDateTimeVec.emplace_back(time);
		Open.emplace_back(data.open);
		High.emplace_back(data.high);
		Low.emplace_back(data.low);
		Close.emplace_back(data.close);
	}
	else
	{
		Open.back() = data.open;
		High.back() = data.high;
		Low.back() = data.low;
		Close.back() = data.close;
	}
	CalcData();
}

void CKlineTarget::CalcData()
{
	std::lock_guard<std::mutex> lk(m_mx);
	if (m_KlineDateTimeVec.empty())
		return;
	for (int i = 0; i < m_targetInfoMap.size(); ++i)
	{
		if (m_targetInfoMap[i].info.strTargetName == "NetGrid")
			NetGrid(i);
	}
}

std::pair<double, double> CKlineTarget::GetMaxAndMin(int nPos, int nStart, int nEnd)
{
	std::lock_guard<std::mutex> lk(m_mx);
	double fMax = DBL_MIN;
	double fMin = DBL_MAX;
	if (m_targetInfoMap.size() > nPos)
	{
		auto& data = m_targetInfoMap[nPos].OutVec;
		for (int i = 0; i < data.size(); ++i)
		{
			for (int j = nStart; j < nEnd; ++j)
			{
				if (j < data[i].size() && !isnan(data[i][j]) && !isinf(data[i][j]))
				{
					fMax = max(data[i][j], fMax);
					fMin = min(data[i][j], fMin);
				}
			}
		}
	}
	return std::make_pair(fMax, fMin);
}

vector<double> CKlineTarget::GetData(int nPos, int nIndex)
{
	std::lock_guard<std::mutex> lk(m_mx);
	auto& data = m_targetInfoMap[nPos].OutVec;
	vector<double> res(data.size(), NAN);
	for (int i = 0; i < data.size(); ++i)
	{
		if (nIndex < data[i].size())
			res[i] = data[i][nIndex];
	}
	return res;
}

void CKlineTarget::NetGrid(int nIndex)
{
	auto &ti = m_targetInfoMap[nIndex];
	int N1 = ti.info.nUsePara[0];
	int N2 = ti.info.nUsePara[1];
	int N3 = ti.info.nUsePara[2];
	auto &ma = ti.OutVec[0];
	auto& up1 = ti.OutVec[1];
	auto& up2 = ti.OutVec[2];
	auto& up3 = ti.OutVec[3];
	auto& up4 = ti.OutVec[4];
	auto& dw1 = ti.OutVec[5];
	auto& dw2 = ti.OutVec[6];
	auto& dw3 = ti.OutVec[7];
	auto& dw4 = ti.OutVec[8];
	auto& bd = ti.tmpDataVec[0];
	auto& bdMa = ti.tmpDataVec[1];
	auto& marketPos = ti.MarketPostion;
	auto& currentPos = ti.CurrentPos;
	auto& tradeMap = ti.tradeMap;
	auto &func = [&](int nPos)
	{
		int nPreMaketPos = nPos - 1 >= 0 ? marketPos[nPos - 1] : 0;
		int nPreCurrentState = nPos - 1 >= 0 ? nPreMaketPos * currentPos[nPos - 1] : 0;
		int nNowMarketPos = marketPos.back();
		int nNowCurrentPos = currentPos.back();
		bool bTrade = false;
		if (!bTrade&&Close[nPos - 2] < ma[nPos - 2] && Close[nPos - 1] >= ma[nPos - 1] && Close[nPos - 1] < up1[nPos - 1] && nPreMaketPos <= 0)
		{
			nNowCurrentPos = 1;
			nNowMarketPos = 1;
			ti.tradeMap[nPos] = TradeInfo{ "maB", (High[nPos] + Low[nPos]) / 2,true };
			bTrade = true;
		}
		if (!bTrade&&nPreCurrentState == 1)
		{
			if (High[nPos] >= up1[nPos - 1])
			{
				ti.tradeMap[nPos] = TradeInfo{"up1S", max(up1[nPos - 1], Low[nPos]),false};
				nNowCurrentPos = 1;
				nNowMarketPos = -1;
				bTrade = true;
			}
		}
		if (!bTrade&&nPreCurrentState == -1)
		{
			if (High[nPos] >= up2[nPos - 1])
			{
				ti.tradeMap[nPos] = TradeInfo{"up2S", max(up2[nPos - 1], Low[nPos]),false };
				nNowCurrentPos = 3;
				nNowMarketPos = -1;
				bTrade = true;
			}

		}
		if (!bTrade&&nPreCurrentState == -3)
		{
			if (Low[nPos] <= up1[nPos - 1])
			{
				ti.tradeMap[nPos] = TradeInfo{"up2SP", min(up1[nPos - 1], High[nPos]),true };
				nNowCurrentPos = 1;
				nNowMarketPos = -1;
				bTrade = true;
			}
			else if (High[nPos] >= up3[nPos - 1])
			{
				ti.tradeMap[nPos] = TradeInfo{"up3S", max(up3[nPos - 1], Low[nPos]),false };
				nNowCurrentPos = 7;
				nNowMarketPos = -1;
				bTrade = true;
			}
		}

		if (!bTrade&&nPreCurrentState == -7)
		{
			if (Low[nPos] <= up2[nPos - 1])
			{
				ti.tradeMap[nPos] = TradeInfo{"up3SP", min(up2[nPos - 1], High[nPos]),true };
				nNowCurrentPos = 3;
				nNowMarketPos = -1;
				bTrade = true;
			}
		}

		if (!bTrade&& Close[nPos - 1] < up4[nPos - 1] && Close[nPos] >= up4[nPos] && nPreMaketPos < 0)
		{
			nNowCurrentPos = 0;
			nNowMarketPos = 0;
			ti.tradeMap[nPos] = TradeInfo{"CAll", (High[nPos] + Low[nPos]) / 2,true};
			bTrade = true;
		}


		if (!bTrade&&Close[nPos - 2] > ma[nPos - 2] && Close[nPos - 1] <= ma[nPos - 1] && Close[nPos - 1] > dw1[nPos - 1] && nPreMaketPos >= 0)
		{
			nNowCurrentPos = 1;
			nNowMarketPos = -1;
			ti.tradeMap[nPos] = TradeInfo{"maS", (High[nPos] + Low[nPos]) / 2,false};
			bTrade = true;
		}
		if (!bTrade&&nPreCurrentState == -1)
		{
			if (Low[nPos] <= dw1[nPos - 1])
			{
				nNowCurrentPos = 1;
				nNowMarketPos = 1;
				ti.tradeMap[nPos] = TradeInfo{"dw1B", min(dw1[nPos - 1], High[nPos]),true };
				bTrade = true;
			}
		}
		if (!bTrade&&nPreCurrentState == 1)
		{
			if (Low[nPos] <= dw2[nPos - 1])
			{
				nNowCurrentPos = 3;
				nNowMarketPos = 1;
				ti.tradeMap[nPos] = TradeInfo{"dw2B", min(dw2[nPos - 1], High[nPos]),true };
			}
		}
		if (!bTrade&&nPreCurrentState == 3)
		{
			if (High[nPos] >= dw1[nPos - 1])
			{
				nNowCurrentPos = 1;
				nNowMarketPos = 1;
				ti.tradeMap[nPos] = TradeInfo{"dw2BP", max(dw1[nPos - 1], Low[nPos]),false };
				bTrade = true;
			}
			else if (Low[nPos] <= dw3[nPos - 1])
			{
				nNowCurrentPos = 7;
				nNowMarketPos = 1;
				ti.tradeMap[nPos] = TradeInfo{"dw3B", min(dw3[nPos - 1], High[nPos]),true };
				bTrade = true;
			}

		}

		if (!bTrade&&nPreCurrentState == 7)
		{
			if (High[nPos] >= dw2[nPos - 1])
			{
				nNowCurrentPos = 3;
				nNowMarketPos = 1;
				ti.tradeMap[nPos] = TradeInfo{"dw3BP", max(dw2[nPos - 1], Low[nPos]),false };
				bTrade = true;
			}

		}

		if (!bTrade&& Close[nPos - 1] > dw4[nPos - 1] && Close[nPos] <= dw4[nPos] && nPreMaketPos > 0)
		{
			nNowCurrentPos = 0;
			nNowMarketPos = 0;
			ti.tradeMap[nPos] = TradeInfo{"SAll", (High[nPos] + Low[nPos]) / 2,false};
			bTrade = true;
		}

		if (marketPos.size() == nPos)
			marketPos.emplace_back(nNowMarketPos);
		else
			marketPos[nPos] = nNowMarketPos;

		if (currentPos.size() == nPos)
			currentPos.emplace_back(nNowCurrentPos);
		else
			currentPos[nPos] = nNowCurrentPos;

	};
	if (ti.bFirst)
	{
		ti.bFirst = FALSE;
		ma = MA(Close, N1);
		bd = High - Low;
		bdMa = MA(bd, N2);
		up1 = ma + bdMa * (N3 / 4.0);
		up2 = ma + bdMa * (N3 / 2.0);
		up3 = ma + bdMa * (N3 * 3 / 4.0);
		up4 = ma + bdMa *N3;
		dw1 = ma - bdMa * (N3 / 4.0);
		dw2 = ma - bdMa * (N3 / 2.0);
		dw3 = ma - bdMa * (N3 * 3 / 4.0);
		dw4 = ma - bdMa *N3;
		marketPos.reserve(ma.size());
		marketPos.emplace_back(0);
		marketPos.emplace_back(0);
		currentPos.reserve(ma.size());
		currentPos.emplace_back(0);
		currentPos.emplace_back(0);
		for (int i = 2; i < ma.size(); ++i)
			func(i);
	}
	else
	{
		bool bNew = m_KlineDateTimeVec.size() > ma.size();
		if (bNew)
		{
			ma.emplace_back(MA(Close, N1, 0));
			bd.emplace_back(High.back() - Low.back());
			bdMa.emplace_back(MA(bd, N2, 0));
			up1.emplace_back(ma.back() + bdMa.back() *N3 / 4.0);
			up2.emplace_back(ma.back() + bdMa.back() *N3 / 2.0);
			up3.emplace_back(ma.back() + bdMa.back() *N3 * 3 / 4.0);
			up4.emplace_back(ma.back() + bdMa.back() *N3);
			dw1.emplace_back(ma.back() - bdMa.back() *N3 / 4.0);
			dw2.emplace_back(ma.back() - bdMa.back() *N3 / 2.0);
			dw3.emplace_back(ma.back() - bdMa.back() *N3 * 3 / 4.0);
			dw4.emplace_back(ma.back() - bdMa.back() *N3);
		}
		else
		{
			ma.back() = MA(Close, N1, 0);
			bd.back() = High.back() - Low.back();
			bdMa.back() = MA(bd, N2, 0);
			up1.back() = ma.back() + bdMa.back() *N3 / 4.0;
			up2.back() = ma.back() + bdMa.back() *N3 / 2.0;
			up3.back() = ma.back() + bdMa.back() *N3 * 3 / 4.0;
			up4.back() = ma.back() + bdMa.back() *N3;
			dw1.back() = ma.back() - bdMa.back() *N3 / 4.0;
			dw2.back() = ma.back() - bdMa.back() *N3 / 2.0;
			dw3.back() = ma.back() - bdMa.back() *N3 * 3 / 4.0;
			dw4.back() = ma.back() - bdMa.back() *N3;
		}
		func(ma.size() - 1);
	}
}

double CKlineTarget::HHV(const OperVec& dataVec, int nCount, int nOffset)
{
	int nSize = dataVec.size();
	if (nSize - nOffset < nCount)
		return 0;
	int nStart = nSize - nOffset - 1;
	int nTimes = 0;
	double res = 0;
	while (nTimes < nCount)
	{
		res = max(res, dataVec[nStart - nTimes]);
		++nTimes;
	}
	return res;
}


double CKlineTarget::LLV(const OperVec& dataVec, int nCount, int nOffset)
{
	int nSize = dataVec.size();
	if (nSize - nOffset < nCount)
		return 0;
	int nStart = nSize - nOffset - 1;
	int nTimes = 0;
	double res = INT_MAX;
	while (nTimes < nCount)
	{
		res = min(res, dataVec[nStart - nTimes]);
		++nTimes;
	}
	return res;
}


double CKlineTarget::MA(const OperVec& dataVec, int nCount, int nOffset)
{
	int nSize = dataVec.size();
	if (nSize - nOffset < nCount)
		return 0;
	int nStart = nSize - nOffset - 1;
	int nTimes = 0;
	double res = 0;

	while (nTimes < nCount)
	{
		res += dataVec[nStart - nTimes];
		++nTimes;
	}
	return res / nCount;
}


double CKlineTarget::STD(const OperVec& dataVec, int nCount, int nOffset)
{
	int nSize = dataVec.size();
	if (nSize - nOffset < nCount)
		return 0;
	if (nCount < 2)
		return 0;

	int nStart = nSize - nOffset - 1;
	int nTimes = 0;
	vector<double> singleDataVec;
	singleDataVec.reserve(nCount);
	while (singleDataVec.size() < nCount)
		singleDataVec.emplace_back(dataVec[nStart - nTimes++]);



	double fMean = accumulate(singleDataVec.begin(), singleDataVec.end(), 0.0) / nCount;
	double res = accumulate(singleDataVec.begin(), singleDataVec.end(), 0.0,
		[&](double sum, double data) {double diff = data - fMean;
	return sum + diff * diff; });

	return sqrt(res / (nCount - 1));
}


double CKlineTarget::ADX(const OperVec& dataVec, int nCount, int nMACount, int nOffset)
{
	int64_t nSize = dataVec.size();
	if (nSize - nOffset < 0 || nSize - nOffset <= nMACount)
		return 0;
	vector<double> fPDIVec(nMACount);
	vector<double> fMDIVec(nMACount);
	//先计算当前最后日期的
	double fDMP = 0;
	double fDMM = 0;
	double fMTR = 0;
	vector<double>fTR(nCount);
	vector<double> fMPChange(nCount);
	vector < double> fMMChange(nCount);
	auto nEnd = nSize - nOffset;
	int64_t nStart = max(int64_t(1), nEnd - nCount);
	for (auto i = nStart; i < nEnd; ++i)
	{
		auto& preData = dataVec[i - 1];
		fTR[i - nStart] = max(max((High[i] - Low[i]), abs(High[i] - Close[i - 1])), abs(Close[i - 1] - Low[i]));
		fMTR += fTR[i - nStart];
		double fHighDiff = High[i] - High[i - 1];
		double fLowDiff = Low[i - 1] - Low[i];
		fMPChange[i - nStart] = fHighDiff > 0 && fHighDiff > fLowDiff ? fHighDiff : 0;
		fMMChange[i - nStart] = fLowDiff > 0 && fLowDiff > fHighDiff ? fLowDiff : 0;
		fDMP += fMPChange[i - nStart];
		fDMM += fMMChange[i - nStart];
	}
	fPDIVec[0] = fDMP * 100 / fMTR;
	fMDIVec[0] = fDMM * 100 / fMTR;


	for (int i = 1; i < nMACount; ++i)
	{
		nEnd = nSize - nOffset - i;
		int nLastIndex = ((nEnd - nStart - i + 1) % nCount + nCount) % nCount;
		fMTR -= fTR[nLastIndex];
		fDMP -= fMPChange[nLastIndex];
		fDMM -= fMMChange[nLastIndex];
		nStart--;
		if (nStart >= 1)
		{
			fTR[nLastIndex] = max(max((High[nStart] - Low[nStart]), abs(High[nStart] - Close[nStart - 1])), abs(Close[nStart - 1] - Low[nStart]));
			double fHighDiff = High[nStart] - High[nStart - 1];
			double fLowDiff = Low[nStart - 1] - Low[nStart];
			fMPChange[nLastIndex] = fHighDiff > 0 && fHighDiff > fLowDiff ? fHighDiff : 0;
			fMMChange[nLastIndex] = fLowDiff > 0 && fLowDiff > fHighDiff ? fLowDiff : 0;
			fMTR += fTR[nLastIndex];
			fDMP += fMPChange[nLastIndex];
			fDMM += fMMChange[nLastIndex];
		}

		fPDIVec[i] = fDMP * 100 / fMTR;
		fMDIVec[i] = fDMM * 100 / fMTR;
	}
	double fADX = 0;
	for (int i = 0; i < nMACount; ++i)
		fADX += (abs(fMDIVec[i] - fPDIVec[i]) / (fMDIVec[i] + fPDIVec[i])) * 100;
	fADX /= nMACount;
	return fADX;
}


double CKlineTarget::RSI(const OperVec& dataVec, int nCount, int nOffset)
{
	auto nSize = dataVec.size();
	if (nSize - nOffset < 1)
		return 0;
	double SMAMAX = 0;
	double SMAABS = 0;
	for (int i = 1; i < nSize - nOffset; ++i)
	{
		double fDiff = dataVec[i] - dataVec[i - 1];
		if (i - 1 == 0)
		{
			SMAMAX = max(fDiff, 0.0);
			SMAABS = abs(fDiff);
		}
		else
		{
			SMAMAX = SMA(SMAMAX, max(fDiff, 0.0), nCount, 1);
			SMAABS = SMA(SMAMAX, abs(fDiff), nCount, 1);
		}
	}
	return SMAMAX / SMAABS * 100;
}


double CKlineTarget::ATR(int nCount, int nOffset)
{
	auto nSize = m_KlineDateTimeVec.size();
	if (nSize - nOffset <= nCount)
		return 0;
	double fATR = 0;
	auto nEnd = nSize - nOffset;
	auto nStart = nEnd - nCount;
	for (auto i = nStart; i < nEnd; ++i)
	{
		fATR += max(max((High[i] - Low[i]), abs(High[i] - Close[i - 1])), abs(Close[i - 1] - Low[i]));
	}
	return fATR / nCount;
}

OperVec CKlineTarget::CrossAbove(OperVec & data1, OperVec & data2)
{
	OperVec res;
	res.reserve(data1.size());
	res.emplace_back(0);
	for (int i = 1; i < data1.size(); ++i)
	{
		if (data1[i - 1] < data2[i - 1] && data1[i] >= data2[i])
			res.emplace_back(1);
		else
			res.emplace_back(0);
	}
	return res;
}

OperVec CKlineTarget::CrossDown(OperVec & data1, OperVec & data2)
{
	OperVec res;
	res.reserve(data1.size());
	res.emplace_back(0);
	for (int i = 1; i < data1.size(); ++i)
	{
		if (data1[i - 1] > data2[i - 1] && data1[i] <= data2[i])
			res.emplace_back(1);
		else
			res.emplace_back(0);
	}
	return res;
}

OperVec CKlineTarget::TryBuy(OperVec && cond, int nSize, OperVec& LimitPrice)
{
	OperVec res;
	res.reserve(cond.size());
	//市价
	if (LimitPrice.empty())
	{
		for (auto &it : cond)
		{
			if (it == TRUE)
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	else
	{
		for (int i = 0; i < cond.size(); ++i)
		{
			if (cond[i] == TRUE && Low[i] <= LimitPrice[i])
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	return res;
}

OperVec CKlineTarget::TrySell(OperVec && cond, int nSize, OperVec& LimitPrice)
{
	OperVec res;
	res.reserve(cond.size());
	//市价
	if (LimitPrice.empty())
	{
		for (auto &it : cond)
		{
			if (it == TRUE)
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	else
	{
		for (int i = 0; i < cond.size(); ++i)
		{
			if (cond[i] == TRUE && Low[i] >= LimitPrice[i])
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	return res;
}

OperVec CKlineTarget::TrySellShort(OperVec && cond, int nSize, OperVec & LimitPrice)
{
	OperVec res;
	res.reserve(cond.size());
	//市价
	if (LimitPrice.empty())
	{
		for (auto &it : cond)
		{
			if (it == TRUE)
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	else
	{
		for (int i = 0; i < cond.size(); ++i)
		{
			if (cond[i] == TRUE && Low[i] >= LimitPrice[i])
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	return res;
}

OperVec CKlineTarget::TryBuyToCover(OperVec && cond, int nSize, OperVec & LimitPrice)
{
	OperVec res;
	res.reserve(cond.size());
	//市价
	if (LimitPrice.empty())
	{
		for (auto &it : cond)
		{
			if (it == TRUE)
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	else
	{
		for (int i = 0; i < cond.size(); ++i)
		{
			if (cond[i] == TRUE && Low[i] <= LimitPrice[i])
				res.emplace_back(nSize);
			else
				res.emplace_back(0);
		}
	}
	return res;
}

OperVec CKlineTarget::EMA(OperVec & data, int nCount)
{
	OperVec res;
	res.reserve(data.size());
	res.emplace_back(data[0]);
	for (int i = 1; i < data.size(); ++i)
		res.emplace_back(EMA(res[i - 1], data[i], nCount));
	return res;
}

OperVec CKlineTarget::SMA(OperVec & data, int nCount, int nWeight)
{
	OperVec res;
	res.reserve(data.size());
	res.emplace_back(data[0]);
	for (int i = 1; i < data.size(); ++i)
		res.emplace_back(SMA(res[i - 1], data[i], nCount, nWeight));
	return res;
}

OperVec CKlineTarget::MA(OperVec & data, int nCount)
{
	OperVec res;
	if (data.size() < nCount)
	{
		vector<double> tmpVec(data.size(), NAN);
		res.swap(tmpVec);
	}
	else
	{
		res.reserve(data.size());
		double fSum = 0;
		for (int i = 0; i < data.size(); ++i)
		{
			fSum += data[i];
			if (i >= nCount - 1)
			{
				if (i - nCount >= 0)
					fSum -= data[i - nCount];
				res.emplace_back(fSum / nCount);
			}
			else
				res.emplace_back(NAN);
		}
	}
	return res;
}


OperVec CKlineTarget::HHV(OperVec & data, int nCount)
{
	OperVec res;
	if (data.size() < nCount)
	{
		vector<double> tmpVec(data.size(), NAN);
		res.swap(tmpVec);
	}
	else
	{
		res.reserve(data.size());
		std::deque<int> qMax;
		for (int i = 0; i < data.size(); ++i)
		{
			while (!qMax.empty() && data[qMax.back()] <= data[i])
				qMax.pop_back();

			if (qMax.front() == i - nCount)
				qMax.pop_front();

			if (i >= nCount - 1)
				res.emplace_back(data[qMax.front()]);
			else
				res.emplace_back(NAN);
		}
	}
	return res;
}

OperVec CKlineTarget::LLV(OperVec & data, int nCount)
{
	OperVec res;
	if (data.size() < nCount)
	{
		vector<double> tmpVec(data.size(), NAN);
		res.swap(tmpVec);
	}
	else
	{
		res.reserve(data.size());
		std::deque<int> qMin;
		for (int i = 0; i < data.size(); ++i)
		{
			while (!qMin.empty() && data[qMin.back()] >= data[i])
				qMin.pop_back();

			if (qMin.front() == i - nCount)
				qMin.pop_front();

			if (i >= nCount - 1)
				res.emplace_back(data[qMin.front()]);
			else
				res.emplace_back(NAN);
		}
	}
	return res;
}


OperVec CKlineTarget::STD(OperVec & data, int nCount)
{
	OperVec res;
	if (nCount < 2 || data.size() < nCount)
	{
		vector<double> tmpVec(data.size(), NAN);
		res.swap(tmpVec);
	}
	else
	{
		res.reserve(data.size());
		double fSum = 0;
		for (int i = 0; i < data.size(); ++i)
		{
			fSum += data[i];
			if (i >= nCount - 1)
			{
				if (i - nCount >= 0)
					fSum -= data[i - nCount];
				double fMean = fSum / nCount;
				double fMeanDiffSum = 0;
				for (int j = i - nCount + 1; j <= i; ++j)
				{
					double fDiff = data[j] - fMean;
					fMeanDiffSum += fDiff*fDiff;
				}
				res.emplace_back(sqrt(fMeanDiffSum / (nCount - 1)));
			}
			else
				res.emplace_back(NAN);
		}
	}
	return res;
}

OperVec CKlineTarget::ADX(OperVec & data, int nCount)
{
	return OperVec();
}

OperVec CKlineTarget::RSI(OperVec & data, int nCount)
{
	OperVec res;
	if (data.size() < 2)
	{
		vector<double> tmpVec(data.size(), NAN);
		res.swap(tmpVec);
	}
	else
	{
		double fSum = 0;
		res.reserve(data.size());
		res.emplace_back(NAN);
		double fDiff = data[1] - data[0];
		double SMAMAX = max(fDiff, 0.0);
		double SMAABS = abs(fDiff);
		res.emplace_back(SMAMAX / SMAABS * 100);
		for (int i = 2; i < data.size(); ++i)
		{
			double fDiff = data[i] - data[i - 1];
			SMAMAX = SMA(SMAMAX, max(fDiff, 0.0), nCount, 1);
			SMAABS = SMA(SMAMAX, abs(fDiff), nCount, 1);
			res.emplace_back(SMAMAX / SMAABS * 100);
		}
	}
	return res;
}


vector<OperVec> CKlineTarget::GetData(int nPos)
{
	std::lock_guard<std::mutex> lk(m_mx);
	if (m_targetInfoMap.size() <= nPos)
		return vector<OperVec>();
	else
		return m_targetInfoMap[nPos].OutVec;
}

std::map<int, CKlineTarget::TradeInfo> CKlineTarget::GetTradeSignal(int nPos)
{
	std::lock_guard<std::mutex> lk(m_mx);
	if (m_targetInfoMap.size() <= nPos)
		return std::map<int, TradeInfo>();
	return m_targetInfoMap[nPos].tradeMap;
}

void CKlineTarget::AddTarget(TargetInfo & info)
{
	m_targetInfoMap.emplace_back(TargetRes{ info });
	auto& tr = m_targetInfoMap.back();
	tr.OutVec.resize(info.strOutName.size());
	tr.tmpDataVec.resize(info.nTmpParaCount);
	tr.bFirst = TRUE;
	//CalcData();
}

void CKlineTarget::ChangeTarget(int nIndex, TargetInfo & info)
{
	auto &tr = m_targetInfoMap[nIndex];
	tr.info = info;
	tr.OutVec.clear();
	tr.tmpDataVec.clear();
	tr.bFirst = TRUE;
	tr.OutVec.resize(info.strOutName.size());
	tr.tmpDataVec.resize(info.nTmpParaCount);
	tr.MarketPostion.clear();
	tr.CurrentPos.clear();
	tr.tradeMap.clear();
	//CalcData();
}

void CKlineTarget::RemoveTarget(int nIndex)
{
	if (m_targetInfoMap.size() > nIndex)
		m_targetInfoMap.erase(m_targetInfoMap.begin() + nIndex);
}


double CKlineTarget::EMA(LastData & preEMA, double data, int nCount, int nTime)
{
	if (preEMA.nTime != nTime)
	{
		preEMA.perData = preEMA.nowData;
		preEMA.nTime = nTime;
	}
	preEMA.nowData = EMA(preEMA.perData, data, nCount);
	return preEMA.nowData;
}

