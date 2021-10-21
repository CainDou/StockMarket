#include "stdafx.h"
#include "DataProc.h"
#include <algorithm>


CDataProc::CDataProc()
{

}


CDataProc::~CDataProc()
{

}

bool CDataProc::CalcRps(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>&uniData, 
	vector<SStringA>& StockIDVec, map<SStringA, map<SStringA, TimeLineData>>& ShowData, vector<SStringA>& dataNameVec)
{
	for (auto &TMap : comData)
	{
		const SStringA& StockID = TMap.first;
		map<SStringA, vector<TimeLineData>> &dataMap = TMap.second;
		if (dataMap["close"].empty())
			continue;
		SetEMA(dataMap, dataMap["close"].back(), 5, "EMA5");
		SetEMA(dataMap, dataMap["close"].back(), 20, "EMA20");
		SetEMA(dataMap, dataMap["close"].back(), 60, "EMA60");
		
		TimeLineData data(dataMap["close"].back());
		strcpy_s(data.dataName, "RPS520");
		data.value = dataMap["EMA5"].back().value / dataMap["EMA20"].back().value;
		UpdateTmData(comData, data);
		strcpy_s(data.dataName, "RPS2060");
		data.value = dataMap["EMA20"].back().value / dataMap["EMA60"].back().value;
		UpdateTmData(comData, data);
	}
	vector<TimeLineData> rpsData;
	rpsData.reserve(StockIDVec.size());
	for (auto &stockID:StockIDVec)
		rpsData.emplace_back(comData[stockID]["RPS250"].back());
	RankPoint(rpsData, uniData, "RPS520", "Rank520", "Point520");
	rpsData.clear();
	for (auto &stockID : StockIDVec)
		rpsData.emplace_back(comData[stockID]["RPS2060"].back());
	RankPoint(rpsData, uniData, "RPS2060", "Rank2060", "Point2060");
	for (auto &stockID : StockIDVec)
	{
		for (auto &dataName : dataNameVec)
		{
			if (comData[stockID].count(dataName))
				ShowData[stockID][dataName] = comData[stockID][dataName].back();
			else if(uniData[stockID].count(dataName))
				ShowData[stockID][dataName] = uniData[stockID][dataName].back();
		}
	}
	return true;
}

void CDataProc::UpdateTmData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, TimeLineData & data)
{
	if (comData[data.securityID][data.dataName].empty() || comData[data.securityID][data.dataName].back().time != data.time)
		comData[data.securityID][data.dataName].emplace_back(data);
	else
		comData[data.securityID][data.dataName].back() = data;
}

bool CDataProc::RankPoint(vector<TimeLineData>& dataVec, map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData, 
	SStringA dataName, SStringA rankName, SStringA pointName)
{
	sort(dataVec.begin(), dataVec.end(), [&](const TimeLineData & data1, const TimeLineData & data2) {return data1.value > data2.value; });
	int i = 1;
	int rank = 1;
	double preValue = NAN;
	int size = dataVec.size();
	for (auto &it : dataVec)
	{
		uniData[it.securityID][rankName].emplace_back(it);
		uniData[it.securityID][pointName].emplace_back(it);
		if (it.value == preValue)
		{
			uniData[it.securityID][rankName].back().value = rank;
			uniData[it.securityID][pointName].back().value = (size - rank - 1)* 1.0 / (size - 1)*100;
			++i;
		}
		else
		{
			rank = i;
			preValue = it.value;
			uniData[it.securityID][rankName].back().value = i;
			uniData[it.securityID][rankName].back().value = (size - rank - 1)* 1.0 / (size - 1)*100;
			++i;
		}
	}
	return true;
}

void CDataProc::SetEMA(map<SStringA, vector<TimeLineData>>& dataMap, const TimeLineData & close, int nCount, SStringA dataName)
{
	TimeLineData data(close);
	strcpy_s(data.dataName, dataName);
	if (dataMap[dataName].empty())
		dataMap[dataName].emplace_back(data);
	else
	{
		if (dataMap[dataName].back().time == data.time)
		{
			size_t size = dataMap[dataName].size();
			if (size == 1)
				dataMap[dataName].back() = data;
			else
			{
				data.value = EMA(nCount, dataMap[dataName][size - 2].value, close.value);
				dataMap[dataName].back() = data;
			}
		}
		else
		{
			data.value = EMA(nCount, dataMap[dataName].back().value, close.value);
			dataMap[dataName].emplace_back(data);
		}

	}

}

