#include "stdafx.h"
#include "DataProc.h"
#include <algorithm>


CDataProc::CDataProc()
{

}


CDataProc::~CDataProc()
{

}

bool CDataProc::CalcRps(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData)
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
		data.value = dataMap["EMA5"].back().value - dataMap["EMA20"].back().value;
		SetDEA(dataMap, data, 10, "DEA520");
		strcpy_s(data.dataName, "RPS520");
		data.value = dataMap["DEA520"].back().value / dataMap["EMA20"].back().value *100;
		UpdateTmData(comData, data);
		data.value = dataMap["EMA20"].back().value - dataMap["EMA60"].back().value;
		SetDEA(dataMap, data, 10, "DEA2060");
		strcpy_s(data.dataName, "RPS2060");
		data.value = dataMap["DEA2060"].back().value / dataMap["EMA60"].back().value * 100;
		UpdateTmData(comData, data);
	}
	return true;
}

bool CDataProc::CalcHisRps(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData)
{
	for (auto &TMap : comData)
	{

		const SStringA& StockID = TMap.first;
		map<SStringA, vector<TimeLineData>> &dataMap = TMap.second;
		if (dataMap["close"].empty())
			continue;
		if (!dataMap["EMA5"].empty())
			continue;
		for (int i = 0; i < dataMap["close"].size(); ++i)
		{
			SetEMA(dataMap, dataMap["close"][i], 5, "EMA5");
			SetEMA(dataMap, dataMap["close"][i], 20, "EMA20");
			SetEMA(dataMap, dataMap["close"][i], 60, "EMA60");
			TimeLineData data(dataMap["close"][i]);
			data.value = dataMap["EMA5"][i].value - dataMap["EMA20"][i].value;
			SetDEA(dataMap, data, 10, "DEA520");
			strcpy_s(data.dataName, "RPS520");
			data.value = dataMap["DEA520"][i].value / dataMap["EMA20"][i].value * 100;
			UpdateTmData(comData, data);
			data.value = dataMap["EMA20"][i].value - dataMap["EMA60"][i].value;
			SetDEA(dataMap, data, 10, "DEA2060");
			strcpy_s(data.dataName, "RPS2060");
			data.value = dataMap["DEA2060"][i].value / dataMap["EMA60"][i].value * 100;
			UpdateTmData(comData, data);
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

void CDataProc::UpdateHisData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, TimeLineData * dataArr,int dataCount)
{
	map<SStringA, map<SStringA, vector<TimeLineData>>> dataMap;
	for (int i = 0; i < dataCount; ++i)
		UpdateTmData(dataMap, dataArr[i]);
	for (auto &it : dataMap)
	{
		const SStringA &StockID = it.first;
		ClearCalcData(it.second);
		vector<TimeLineData>& dataVec = it.second["close"];
		comData[StockID]["close"].insert(comData[StockID]["close"].begin(), dataVec.begin(), dataVec.end());
	}
}

bool CDataProc::RankPoint(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, 
	map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData, vector<SStringA>& StockIDVec)
{
	vector<TimeLineData> rpsData;
	rpsData.reserve(StockIDVec.size());
	for (auto &stockID : StockIDVec)
		if (!comData[stockID]["RPS520"].empty())
			rpsData.emplace_back(comData[stockID]["RPS520"].back());
	RankPoint(rpsData, uniData, "RPS520", "Rank520", "Point520");
	rpsData.clear();
	for (auto &stockID : StockIDVec)
		if (!comData[stockID]["RPS2060"].empty())
			rpsData.emplace_back(comData[stockID]["RPS2060"].back());
	RankPoint(rpsData, uniData, "RPS2060", "Rank2060", "Point2060");
	return true;
}

bool CDataProc::RankPointHisData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, 
	map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData, vector<SStringA>& StockIDVec)
{
	map<int64_t, vector<TimeLineData>> rps520Data;
	map<int64_t, vector<TimeLineData>> rps2060Data;
	for (auto &it : StockIDVec)
	{
		ClearRankPointData(uniData[it]);
		vector<TimeLineData> &data520Vec = comData[it]["RPS520"];
		for (auto &itor : data520Vec)
		{
			int64_t datetime = (int64_t)itor.date * 10000 + (int64_t)itor.time;
			rps520Data[datetime].emplace_back(itor);
		}
		vector<TimeLineData> &data2060Vec = comData[it]["RPS2060"];
		for (auto &itor : data2060Vec)
		{
			int64_t datetime = (int64_t)itor.date * 10000 + (int64_t)itor.time;
			rps2060Data[datetime].emplace_back(itor);
		}
	}
	for (auto &it : rps520Data)
		RankPoint(it.second, uniData, "RPS520", "Rank520", "Point520");
	for (auto &it : rps2060Data)
		RankPoint(it.second, uniData, "RPS2060", "Rank2060", "Point2060");
	return true;
}

bool CDataProc::UpdateShowData(map<SStringA, map<SStringA, vector<TimeLineData>>>& comData, map<SStringA, map<SStringA, vector<TimeLineData>>>& uniData,
	map<SStringA, map<SStringA, TimeLineData>>& ShowData, vector<SStringA>& dataNameVec, vector<SStringA>& StockIDVec)
{
	for (auto &stockID : StockIDVec)
	{
		for (auto &dataName : dataNameVec)
		{
			if (!comData[stockID][dataName].empty())
				ShowData[stockID][dataName] = comData[stockID][dataName].back();
			else if (!uniData[stockID][dataName].empty())
				ShowData[stockID][dataName] = uniData[stockID][dataName].back();
		}
	}
	return true;
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
		TimeLineData rankData(it);
		TimeLineData pointData(it);
		strcpy_s(rankData.dataName, rankName);
		strcpy_s(pointData.dataName, pointName);
		if (it.value == preValue)
		{
			rankData.value = rank;
			pointData.value = (size - rank)* 1.0 / (size - 1) * 100;
			++i;
		}
		else
		{
			rank = i;
			preValue = it.value;
			rankData.value = rank;
			pointData.value = (size - rank)* 1.0 / (size - 1) * 100;
			++i;
		}
		UpdateTmData(uniData, rankData);
		UpdateTmData(uniData, pointData);

	}
	return true;
}

bool CDataProc::ClearCalcData(map<SStringA, vector<TimeLineData>>& dataMap)
{
	dataMap["EMA5"].clear();
	dataMap["EMA20"].clear();
	dataMap["EMA60"].clear();
	dataMap["DEA520"].clear();
	dataMap["DEA2060"].clear();
	dataMap["RPS520"].clear();
	dataMap["RPS2060"].clear();

	return true;
}

bool CDataProc::ClearRankPointData(map<SStringA, vector<TimeLineData>>& dataMap)
{
	dataMap["Rank520"].clear();
	dataMap["Rank2060"].clear();
	dataMap["Point520"].clear();
	dataMap["Point2060"].clear();
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

void CDataProc::SetDEA(map<SStringA, vector<TimeLineData>>& dataMap, const TimeLineData & close, int nCount, SStringA dataName)
{
	TimeLineData data(close);
	strcpy_s(data.dataName, dataName);
	if (dataMap[dataName].empty())
	{
		data.value = 0;
		dataMap[dataName].emplace_back(data);
	}
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

