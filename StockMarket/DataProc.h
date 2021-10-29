#pragma once

#include "Define.h"
#include <map>
#include <vector>

using std::vector;
using std::map;

class CDataProc
{
public:
	CDataProc();
	~CDataProc();

public:
	double  EMA(int nCount, double preEMA, double data) const;
	bool CalcRps(map<SStringA,map<SStringA,vector<TimeLineData>>>& comData);
	bool CalcHisRps(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData);
	void UpdateTmData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, TimeLineData& data);
	void UpdateHisData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, TimeLineData* dataArr, int dataCount);
	bool RankPoint(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>&uniData, 
		vector<SStringA>& StockIDVec);
	bool RankPointHisData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>&uniData,
		vector<SStringA>& StockIDVec);
	bool UpdateShowData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>&uniData, 
		map<SStringA,map<SStringA, TimeLineData>>&ShowData, vector<SStringA>& dataNameVec , vector<SStringA>& StockIDVec);
protected:
	void SetEMA(map<SStringA, vector<TimeLineData>>& dataMap,const TimeLineData& close, int nCount, SStringA dataName);
	void SetDEA(map<SStringA, vector<TimeLineData>>& dataMap, const TimeLineData& close, int nCount, SStringA dataName);
	bool RankPoint(vector<TimeLineData>& dataVec, map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData, 
		SStringA dataName, SStringA rankName, SStringA pointName);
	bool ClearCalcData(map<SStringA, vector<TimeLineData>> &dataMap);
	bool ClearRankPointData(map<SStringA, vector<TimeLineData>> &dataMap);
};

inline double CDataProc::EMA(int nCount, double preEMA, double data) const
{
	return preEMA*(nCount - 1) / (nCount + 1) + data * 2 / (nCount + 1);
}

