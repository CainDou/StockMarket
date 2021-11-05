#include "stdafx.h"
#include "DataProc.h"
#include <algorithm>


CDataProc::CDataProc()
{

}


CDataProc::~CDataProc()
{

}

bool CDataProc::CalcRps(TimeLineArrMap & comData)
{
	for (auto &TMap : comData)
	{
		const SStringA& StockID = TMap.first;
		map<SStringA, vector<CoreData>> &dataMap = TMap.second;
		auto & closeVec = dataMap["close"];
		if (closeVec.empty())
			continue;
		SetEMA(dataMap, closeVec.back(), 5, "EMA5");
		SetEMA(dataMap, closeVec.back(), 20, "EMA20");
		SetEMA(dataMap, closeVec.back(), 60, "EMA60");

		auto& EMA5Vec = dataMap["EMA5"];
		auto & EMA20Vec = dataMap["EMA20"];
		auto & EMA60Vec = dataMap["EMA60"];
		auto & DEA520Vec = dataMap["DEA520"];
		auto & DEA2060Vec = dataMap["DEA2060"];


		CoreData data(closeVec.back());
		data.value = EMA5Vec.back().value - EMA20Vec.back().value;
		SetDEA(dataMap, data, 10, "DEA520");
		data.value = DEA520Vec.back().value / EMA20Vec.back().value * 100;
		UpdateOnceTmData(comData, data, StockID, "RPS520");
		data.value = (EMA5Vec.back().value - EMA20Vec.back().value - DEA520Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD520");
		data.value = EMA20Vec.back().value - EMA60Vec.back().value;
		SetDEA(dataMap, data, 10, "DEA2060");
		data.value = DEA2060Vec.back().value / EMA60Vec.back().value * 100;
		UpdateOnceTmData(comData, data, StockID, "RPS2060");
		data.value = (EMA20Vec.back().value - EMA60Vec.back().value - DEA2060Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD2060");
	}
	return true;
}

bool CDataProc::CalcHisRps(TimeLineArrMap& comData)
{
	for (auto &TMap : comData)
	{

		const SStringA& StockID = TMap.first;
		auto &dataMap = TMap.second;
		auto & closeVec = dataMap["close"];
		if (closeVec.empty())
			continue;
		auto& EMA5Vec = dataMap["EMA5"];
		if (!EMA5Vec.empty())
			continue;

		auto & EMA20Vec = dataMap["EMA20"];
		auto & EMA60Vec = dataMap["EMA60"];

		auto &DEA520Vec = dataMap["DEA520"];
		auto &DEA2060Vec = dataMap["DEA2060"];

		auto & RPS520Vec = dataMap["RPS520"];
		auto & RPS2060Vec = dataMap["RPS2060"];


		for (int i = 0; i < closeVec.size(); ++i)
		{
			SetEMA(dataMap, closeVec[i], 5, "EMA5");
			SetEMA(dataMap, closeVec[i], 20, "EMA20");
			SetEMA(dataMap, closeVec[i], 60, "EMA60");

			CoreData data(closeVec[i]);
			data.value = EMA5Vec.back().value - EMA20Vec.back().value;
			SetDEA(dataMap, data, 10, "DEA520");
			data.value = DEA520Vec.back().value / EMA20Vec.back().value * 100;
			UpdateTmData(RPS520Vec, data);
			data.value = EMA20Vec.back().value - EMA60Vec.back().value;
			SetDEA(dataMap, data, 10, "DEA2060");
			data.value = DEA2060Vec.back().value / EMA60Vec.back().value * 100;
			UpdateTmData(RPS2060Vec, data);
		}
		CoreData data(EMA5Vec.back());
		data.value = (EMA5Vec.back().value - EMA20Vec.back().value - DEA520Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD520");
		data.value = (EMA20Vec.back().value - EMA60Vec.back().value - DEA2060Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD2060");


	}
	return true;
}

void CDataProc::UpdateTmData(TimeLineArrMap& comData, TimeLineData & data)
{
	auto & dataVec = comData[data.securityID][data.dataName];
	if (dataVec.empty())
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date < data.data.date)
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date == data.data.date)
	{
		if (dataVec.back().time < data.data.time)
			dataVec.emplace_back(data.data);
		else if (dataVec.back().time == data.data.time)
			dataVec.back() = data.data;
	}
}

void CDataProc::UpdateTmData(TimeLineArrMap & comData, CoreData & data, SStringA SecurityID, SStringA dataName)
{
	auto & dataVec = comData[SecurityID][SecurityID];
	if (dataVec.empty())
		dataVec.emplace_back(data);
	else if (dataVec.back().date < data.date)
		dataVec.emplace_back(data);
	else if (dataVec.back().date == data.date)
	{
		if (dataVec.back().time < data.time)
			dataVec.emplace_back(data);
		else if (dataVec.back().time == data.time)
			dataVec.back() = data;
	}

}

void CDataProc::UpdateTmData(vector<CoreData>& comData, TimeLineData & data)
{
	if (comData.empty())
		comData.emplace_back(data.data);
	else if (comData.back().date < data.data.date)
		comData.emplace_back(data.data);
	else if (comData.back().date == data.data.date)
	{
		if (comData.back().time <  data.data.time)
			comData.emplace_back(data.data);
		else if (comData.back().time == data.data.time)
			comData.back() = data.data;
	}
}

void CDataProc::UpdateTmData(vector<CoreData>& comData, CoreData & data)
{
	if (comData.empty())
		comData.emplace_back(data);
	else if (comData.back().date < data.date)
		comData.emplace_back(data);
	else if (comData.back().date == data.date)
	{
		if (comData.back().time <  data.time)
			comData.emplace_back(data);
		else if (comData.back().time == data.time)
			comData.back() = data;
	}

}

void CDataProc::UpdateClose(TimeLineArrMap& comData, TimeLineData & data, int nPeriod)
{
	auto & dataVec = comData[data.securityID][data.dataName];
	if (nPeriod != 1)
	{
		int timeLeft = (data.data.time % 100) % nPeriod;
		data.data.time -= timeLeft;
	}
	if (dataVec.empty())
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date < data.data.date)
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date == data.data.date)
	{
		if (dataVec.back().time < data.data.time)
			dataVec.emplace_back(data.data);
		else if (dataVec.back().time == data.data.time)
			dataVec.back() = data.data;
	}
}

void CDataProc::UpdateClose(vector<CoreData>& comData, TimeLineData & data, int nPeriod)
{
	if (nPeriod != 1)
	{
		int timeLeft = (data.data.time % 100) % nPeriod;
		data.data.time -= timeLeft;
	}
	if (comData.empty())
		comData.emplace_back(data.data);
	else if (comData.back().date < data.data.date)
		comData.emplace_back(data.data);
	else if (comData.back().date == data.data.date)
	{
		if (comData.back().time < data.data.time)
			comData.emplace_back(data.data);
		else if (comData.back().time == data.data.time)
			comData.back() = data.data;
	}

}

void CDataProc::UpdateHisData(TimeLineArrMap& comData, TimeLineData * dataArr, int dataCount, int Period)
{
	TimeLineArrMap dataMap;
	SStringA nowStockID = "";
	vector<CoreData> * pDataVec = nullptr;
	for (int j = 0; j < dataCount; ++j)
	{
		if (nowStockID != dataArr[j].securityID)
		{
			nowStockID = dataArr[j].securityID;
			pDataVec = &dataMap[nowStockID]["close"];
		}
		UpdateClose(*pDataVec, dataArr[j], Period);

	}

	for (auto &it : dataMap)
	{
		const SStringA &StockID = it.first;
		auto &comMap = comData[StockID];
		ClearCalcData(comMap);
		auto & dataVec = it.second["close"];
		comMap["close"].insert(comMap["close"].begin(), dataVec.begin(), dataVec.end());
	}
}

bool CDataProc::RankPoint(TimeLineArrMap& comData, TimeLineArrMap& uniData, vector<SStringA>& StockIDVec)
{
	vector<pair<SStringA,CoreData>> rpsData;
	rpsData.reserve(StockIDVec.size());
	for (auto &stockID : StockIDVec)
		if (!comData[stockID]["RPS520"].empty())
			rpsData.emplace_back(make_pair(stockID,comData[stockID]["RPS520"].back()));
	RankPoint(rpsData, uniData, "RPS520", "Rank520", "Point520");
	rpsData.clear();
	for (auto &stockID : StockIDVec)
		if (!comData[stockID]["RPS2060"].empty())
			rpsData.emplace_back(make_pair(stockID, comData[stockID]["RPS2060"].back()));
	RankPoint(rpsData, uniData, "RPS2060", "Rank2060", "Point2060");
	return true;
}

bool CDataProc::RankPointHisData(TimeLineArrMap& comData, TimeLineArrMap& uniData, vector<SStringA>& StockIDVec)
{
	map<int64_t, vector<pair<SStringA,CoreData>>> rpsData;
	for (auto &it : StockIDVec)
	{
		ClearRankPointData(uniData[it]);
		auto &data520Vec = comData[it]["RPS520"];
		for (auto &itor : data520Vec)
		{
			int64_t datetime = (int64_t)itor.date * 10000 + (int64_t)itor.time;
			rpsData[datetime].emplace_back(make_pair(it,itor));
		}
	}
	for (auto &it : rpsData)
		RankPoint(it.second, uniData, "RPS520", "Rank520", "Point520");

	rpsData.clear();
	for (auto &it : StockIDVec)
	{
		auto &data2060Vec = comData[it]["RPS2060"];
		for (auto &itor : data2060Vec)
		{
			int64_t datetime = (int64_t)itor.date * 10000 + (int64_t)itor.time;
			rpsData[datetime].emplace_back(make_pair(it, itor));
		}
	}
	for (auto &it : rpsData)
		RankPoint(it.second, uniData, "RPS2060", "Rank2060", "Point2060");
	return true;
}

bool CDataProc::UpdateShowData(TimeLineArrMap& comData, TimeLineArrMap& uniData,
	TimeLineMap& ShowData, vector<SStringA>& comDataNameVec, vector<SStringA>& uniDataNameVec, vector<SStringA>& StockIDVec)
{
	for (auto &stockID : StockIDVec)
	{
		auto & ShowMap = ShowData[stockID];
		auto & comMap = comData[stockID];
		auto & uniMap = uniData[stockID];
		for (auto &dataName : comDataNameVec)
		{
			if (!comMap[dataName].empty())
				ShowMap[dataName] = comMap[dataName].back();
		}

		for (auto &dataName : uniDataNameVec)
		{
			if (!uniMap[dataName].empty())
				ShowMap[dataName] = uniMap[dataName].back();
		}
	}
	return true;
}

bool CDataProc::RankPoint(vector<pair<SStringA, CoreData>>& dataVec, TimeLineArrMap& uniData, SStringA dataName, SStringA rankName, SStringA pointName)
{
	sort(dataVec.begin(), dataVec.end(), 
		[&](const pair<SStringA, CoreData> & data1, const pair<SStringA, CoreData> & data2)
	{return data1.second.value > data2.second.value; });

	int i = 1;
	int rank = 1;
	double preValue = NAN;
	int size = dataVec.size();
	for (auto &it : dataVec)
	{
		CoreData rankData(it.second);
		CoreData pointData(it.second);
		auto & dataMap = uniData[it.first];
		if (it.second.value == preValue)
		{
			rankData.value = rank;
			pointData.value = (size - rank)* 1.0 / (size - 1) * 100;
			++i;
		}
		else
		{
			rank = i;
			preValue = it.second.value;
			rankData.value = rank;
			pointData.value = (size - rank)* 1.0 / (size - 1) * 100;
			++i;
		}
		//UpdateTmData(uniData, rankData);
		//UpdateTmData(uniData, pointData);
		UpdateTmData(dataMap[rankName], rankData);
		UpdateTmData(dataMap[pointName], pointData);

	}
	return true;
}

bool CDataProc::ClearCalcData(map<SStringA, vector<CoreData>>& dataMap)
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

bool CDataProc::ClearRankPointData(map<SStringA, vector<CoreData>>& dataMap)
{
	dataMap["Rank520"].clear();
	dataMap["Rank2060"].clear();
	dataMap["Point520"].clear();
	dataMap["Point2060"].clear();
	return true;
}

void CDataProc::UpdateOnceTmData(TimeLineArrMap & comData, CoreData & data, SStringA SecurityID, SStringA dataName)
{
	vector<CoreData>& dataVec = comData[SecurityID][dataName];
	if (dataVec.empty())
	{
		dataVec.reserve(1);
		dataVec.emplace_back(data);
	}
	else
		dataVec[0] = data;
}

void CDataProc::UpdateOnceTmData(vector<CoreData>& comData, CoreData & data)
{
	if (comData.empty())
	{
		comData.reserve(1);
		comData.emplace_back(data);
	}
	else
		comData[0] = data;
}

void CDataProc::SetEMA(map<SStringA, vector<CoreData>>& dataMap, const CoreData& close, int nCount, SStringA dataName)
{
	auto & dataVec = dataMap[dataName];
	if (dataVec.empty())
	{
		CoreData data(close);
		dataVec.reserve(2);
		dataVec.emplace_back(data);
		dataVec.emplace_back(data);

	}
	else
	{
		if (dataVec[1].time == close.time)
		{
			if (dataVec[0].time == dataVec[1].time)
				dataVec[1].value = close.value;
			else
				dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);
		}
		else
		{
			dataVec[0] = dataVec[1];
			dataVec[1] = close;
			dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);

		}
	}
	//	if (dataVec[0].time == dataVec[1].time)
	//	{

	//	}

	//	if (dataVec.back().time == data.time)
	//	{
	//		size_t size = dataVec.size();
	//		if (size == 1)
	//			dataVec[0] = data;
	//		else
	//		{
	//			data.value = EMA(nCount, dataVec[0].value, close.value);
	//			dataVec[1] = data;
	//		}
	//	}
	//	else
	//	{
	//		dataVec[0] = dataVec[1];
	//		data.value = EMA(nCount, dataVec.back().value, close.value);
	//		dataVec.emplace_back(data);
	//	}


}


void CDataProc::SetDEA(map<SStringA, vector<CoreData>>& dataMap, const CoreData & close, int nCount, SStringA dataName)
{

	auto & dataVec = dataMap[dataName];
	if (dataVec.empty())
	{
		CoreData data(close);
		data.value = 0;
		dataVec.reserve(2);
		dataVec.emplace_back(data);
		dataVec.emplace_back(data);

	}
	else
	{
		if (dataVec[1].time == close.time)
		{
			if (dataVec[0].time == dataVec[1].time)
				dataVec[1].value = 0;
			else
				dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);
		}
		else
		{
			dataVec[0] = dataVec[1];
			dataVec[1] = close;
			dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);
		}
	}

	//TimeLineData data(close);
	//strcpy_s(data.dataName, dataName);
	//auto & dataVec = dataMap[dataName];
	//if (dataVec.empty())
	//{
	//	data.value = 0;
	//	dataVec.emplace_back(data);
	//}
	//else
	//{
	//	if (dataVec.back().time == data.time)
	//	{
	//		size_t size = dataVec.size();
	//		if (size == 1)
	//			dataVec.back() = data;
	//		else
	//		{
	//			data.value = EMA(nCount, dataVec[size - 2].value, close.value);
	//			dataVec.back() = data;
	//		}
	//	}
	//	else
	//	{
	//		data.value = EMA(nCount, dataVec.back().value, close.value);
	//		dataVec.emplace_back(data);
	//	}

	//}

}

