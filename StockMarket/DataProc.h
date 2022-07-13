#pragma once

#include "Define.h"
#include <map>
#include <vector>
#include<unordered_map>

using std::vector;
using std::map;
using std::unordered_map;
using std::pair;
using std::make_pair;

class CDataProc
{
public:
	CDataProc();
	~CDataProc();

public:
	bool CalcKline(vector<KlineType> &klineVec, CommonIndexMarket &indexMarket, int nPeriod, long long& LastVol);
	bool CalcKline(vector<KlineType> &klineVec, CommonStockMarket &indexMarket, int nPeriod, long long& LastVol, int nTradingDay);
	bool CalcMA(vector<MAType>& MaVec, const vector<KlineType>& klineVec, int Period[4]);
	bool CalcRps(TimeLineArrMap& comData);
	bool CalcHisRps(TimeLineArrMap& comData);
	bool SetPreEMAData(TimeLineArrMap& comData, const TimeLineData& data);
	void UpdateTmData(TimeLineArrMap& comData, const TimeLineData& data);
	void UpdateTmData(TimeLineArrMap& comData, CoreData& data, SStringA SecurityID, SStringA dataName);
	void UpdateTmData(vector<CoreData>& comData, const TimeLineData& data);
	static void UpdateTmData(vector<CoreData>& comData, CoreData& data);

	void UpdateClose(TimeLineArrMap& comData, const TimeLineData& data, int nPeriod);
	void UpdateClose(vector<CoreData>& comData, const TimeLineData& data, int nPeriod);
	void UpdateHisData(TimeLineArrMap& comData, TimeLineData* dataArr, int dataCount,int Period);

	bool RankPoint(TimeLineArrMap& comData, TimeLineArrMap& uniData,
		vector<SStringA>& StockIDVec,
		unordered_map<SStringA, StockInfo, hash_SStringA> &StockInfoMap);

	bool RankPointHisData(TimeLineArrMap& comData, TimeLineArrMap& uniData,
		vector<SStringA>& StockIDVec,
		unordered_map<SStringA,StockInfo,hash_SStringA> &StockInfoMap);

	bool UpdateShowData(TimeLineArrMap& comData, TimeLineArrMap& uniData,TimeLineMap& ShowData,
		vector<SStringA>& comDataNameVec , vector<SStringA>& uniDataNameVec, vector<SStringA>& StockIDVec);
	double  EMA(int nCount, double preEMA, double data) const;

	bool SetPeriodFenshiOpenEMAData(TimeLineArrMap & comData,
		const const TimeLineData& data);
protected:
	void SetEMA(map<SStringA, vector<CoreData>>& dataMap,const CoreData& close, int nCount, SStringA dataName);
	void SetDEA(map<SStringA, vector<CoreData>>& dataMap, const CoreData& close, int nCount, SStringA dataName);
	bool RankPoint(vector<pair<SStringA,CoreData>>& dataVec, TimeLineArrMap& uniData,
		SStringA dataName, SStringA rankName, SStringA pointName,
		unordered_map<SStringA, StockInfo, hash_SStringA> &StockInfoMap);
	bool ClearCalcData(map<SStringA, vector<CoreData>> &dataMap);
	bool ClearRankPointData(map<SStringA, vector<CoreData>> &dataMap);
	void UpdateOnceTmData(TimeLineArrMap& comData, CoreData& data, SStringA SecurityID, SStringA dataName);
	void UpdateOnceTmData(vector<CoreData>& comData, CoreData& data);
	vector<TimeLineData>  CreatePreEMAFromOpenData(const const TimeLineData& data);
};

inline double CDataProc::EMA(int nCount, double preEMA, double data) const
{
	return preEMA*(nCount - 1) / (nCount + 1) + data * 2 / (nCount + 1);
}




