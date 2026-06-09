#pragma once
//#include "Define.h"
#include <set>
#include <mutex>



class CKlineTarget
{
#define REF(data,pos) (* (double*)((char*)&(data) + (pos)))


	typedef struct _TradeInfo
	{
		SStringA strLabel;
		double fTradePrice;
		bool bBuy;
	}TradeInfo;

	typedef struct _TargetRes
	{
		TargetInfo info;
		std::vector<OperVec> OutVec;
		std::vector<OperVec> tmpDataVec;
		std::vector<int>MarketPostion;
		std::vector<int>CurrentPos;
		std::map<int, TradeInfo> tradeMap;
		BOOL bFirst;
	}TargetRes;



public:
	CKlineTarget();
	~CKlineTarget();

	void ClearData();
	void UpdateData(const KlineType dataArr[],int nCount);
	void UpdateData(const KlineType& data);
	void CalcData();
	std::pair<double, double> GetMaxAndMin(int nPos, int nStart, int nEnd);
	std::vector<double> GetData(int nPos, int nIndex);
	std::vector<OperVec> GetData(int nPos);
	std::map<int, TradeInfo> GetTradeSignal(int nPos);
	void AddTarget(TargetInfo&info);
	void ChangeTarget(int nIndex, TargetInfo&info);
	void RemoveTarget(int nIndex);
	TargetInfo GetTargetInfo(int nIndex);
	static const TargetInfo& GetTargetOrgInfo(int nTargetID);
	static void  ChangeTargetInfoDefPara(int nTargetID,std::vector<int>& paraVec);

public:
	static double EMA(LastData& preEMA, double data, int nCount, int nTime);
	static double EMA(double preEMA, double data, int nCount);
	static double MA(double* dataArr, int nDataCount, int nCalcCount);
	static double SMA(double preEMA, double data, int nCount, int nWeight);
	static double HHV(double* dataArr, int nCount);

	static OperVec EMA(OperVec&data, int nCount);
	static OperVec SMA(OperVec&data, int nCount,int nWeight);
	static OperVec MA(OperVec&data, int nCount);

protected:
	void NetGrid(int nIndex);

protected:
	double HHV(const OperVec& dataVec, int nCount, int nOffset);
	double LLV(const OperVec& dataVec, int nCount, int nOffset);
	double MA(const OperVec& dataVec, int nCount, int nOffset);
	double STD(const OperVec& dataVec, int nCount, int nOffset);
	double ADX(const OperVec& dataVec, int nCount, int nMACount, int nOffset);
	double RSI (const OperVec& dataVec, int nCount, int nOffset);
	double ATR(int nCount,int nOffset);
	OperVec CrossAbove(OperVec& data1,OperVec& data2);
	OperVec CrossDown(OperVec& data1, OperVec& data2);
	OperVec TryBuy(OperVec&& cond, int nSize,OperVec& LimitPrice);
	OperVec TrySell(OperVec&& cond, int nSize, OperVec& LimitPrice);
	OperVec TrySellShort(OperVec&& cond, int nSize, OperVec& LimitPrice);
	OperVec TryBuyToCover(OperVec&& cond, int nSize, OperVec& LimitPrice);
	OperVec HHV(OperVec&data,int nCount);
	OperVec LLV(OperVec&data, int nCount);
	OperVec STD(OperVec&data, int nCount);
	OperVec ADX(OperVec&data, int nCount);
	OperVec RSI(OperVec&data, int nCount);

	void UpdateData(std::vector<double >& dataVec, double fData,int nDataPos);

protected:
	std::vector<int64_t> m_KlineDateTimeVec;
	OperVec Open;
	OperVec High;
	OperVec Low;
	OperVec Close;
	std::vector<TargetRes> m_targetInfoMap;
	static std::map<int, int>m_klineDataPosMap;
	//std::vector<OperVec> m_OutVec;
	std::map<SStringA, OperVec> m_tmpDataVec;
	std::mutex m_mx;
	static vector<TargetInfo> m_targetInfo;
};


inline double CKlineTarget::EMA(double preEMA, double data, int nCount)
{
	return (preEMA * (nCount - 1) + data * 2) / (nCount + 1);
}

inline double CKlineTarget::MA(double * dataArr, int nDataCount, int nCalcCount)
{
	if (nDataCount < nCalcCount)return 0;
	double res = 0.0;
	for (int i = nDataCount - 1, j = 0; i >= 0 && j < nCalcCount; --i, ++j)
		res += dataArr[i];
	res /= nCalcCount;
	return res;
}

inline double CKlineTarget::SMA(double preEMA, double data, int nCount, int nWeight)
{
	return (preEMA * (nCount - nWeight) + data * nWeight) / nCount;
}

inline double CKlineTarget::HHV(double * dataArr, int nCount)
{
	double fMax = dataArr[0];
	for (int i = 1; i < nCount; ++i)
		fMax = max(fMax, dataArr[i]);
	return fMax;
}


inline void CKlineTarget::UpdateData(std::vector<double >& dataVec, double fData, int nDataPos)
{
	if (dataVec.size() >= nDataPos)
		dataVec.emplace_back(fData);
	else
		dataVec[nDataPos] = fData;
}


inline TargetInfo CKlineTarget::GetTargetInfo(int nIndex)
{
	if (m_targetInfoMap.size() > nIndex)
		return m_targetInfoMap[nIndex].info;
	return TargetInfo();
}

inline const TargetInfo & CKlineTarget::GetTargetOrgInfo(int nTargetID)
{
	if (m_targetInfo.size() > nTargetID)
		return m_targetInfo[nTargetID];
	return TargetInfo{ -1 };
}

inline void CKlineTarget::ChangeTargetInfoDefPara(int nTargetID, std::vector<int>& paraVec)
{
	if (m_targetInfo.size() > nTargetID)
		 m_targetInfo[nTargetID].nParaDefValue = paraVec;
}
