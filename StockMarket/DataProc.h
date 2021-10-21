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
	bool CalcRps(map<SStringA,map<SStringA,vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>&uniData,
		vector<SStringA>& StockIDVec,map<SStringA,map<SStringA,TimeLineData>>& ShowData,vector<SStringA>& dataNameVec);
	void UpdateTmData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, TimeLineData& data);

protected:
	void SetEMA(map<SStringA, vector<TimeLineData>>& dataMap,const TimeLineData& close, int nCount, SStringA dataName);
	bool RankPoint(vector<TimeLineData>& dataVec, map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData, 
		SStringA dataName, SStringA rankName, SStringA pointName);
};

inline double CDataProc::EMA(int nCount, double preEMA, double data) const
{
	return preEMA*(nCount - 1) / (nCount + 1) + data * 2 / (nCount + 1);
}

