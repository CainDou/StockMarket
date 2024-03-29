#include "stdafx.h"
#include "DataProc.h"
#include <algorithm>
#include <numeric>

using std::accumulate;

CDataProc::CDataProc()
{

}


CDataProc::~CDataProc()
{

}

bool CDataProc::CalcKline(vector<KlineType>& klineVec,
	CommonIndexMarket & indexMarket, 
	int nPeriod, long long& LastVol)
{
	int time = indexMarket.UpdateTime / 100;
	if (time == 925)
		time = 930;
	if (time >= 1130 && time < 1300)
		time = 1129;
	if (time >= 1500)
		time = 1459;

	if (nPeriod != 1)
	{
		if (nPeriod / 1440 == 0)
		{
			if (nPeriod != 60)
			{
				int timeLeft = (time % 100) % nPeriod;
				time -= timeLeft;
			}
			else
			{
				if (time >= 930 && time < 1030)
					time = 930;
				else if (time >= 1030 && time < 1130)
					time = 1030;
				else
				{
					int timeLeft = (time % 100) % nPeriod;
					time -= timeLeft;
				}
			}
		}
		else
			time = 0;
	}
	if (klineVec.empty() || klineVec.back().time != time
		|| klineVec.back().date != indexMarket.TradingDay)
	{
		KlineType data = { 0 };
		data.time = time;
		data.date = indexMarket.TradingDay;
		data.open = indexMarket.LastPrice;
		data.high = indexMarket.LastPrice;
		data.low = indexMarket.LastPrice;
		data.close = indexMarket.LastPrice;
		data.vol = indexMarket.Volume - LastVol;
		LastVol = indexMarket.Volume;
		klineVec.emplace_back(data);
	}
	else
	{
		auto &lastData = klineVec.back();
		lastData.close = indexMarket.LastPrice;
		lastData.high = max(indexMarket.LastPrice, lastData.high);
		lastData.low = min(indexMarket.LastPrice, lastData.low);
		lastData.vol += indexMarket.Volume - LastVol;
		LastVol = indexMarket.Volume;
	}
	return true;
}

bool CDataProc::CalcKline(vector<KlineType>& klineVec, 
	CommonStockMarket & indexMarket, int nPeriod, 
	long long& LastVol, int nTradingDay)
{
	int time = indexMarket.UpdateTime / 100;
	if (time == 925)
		time = 930;
	if (time >= 1130 && time < 1300)
		time = 1129;
	if (time >= 1500)
		time = 1459;
	if (nPeriod != 1)
	{
		if (nPeriod / 1440 == 0)
		{
			if (nPeriod != 60)
			{
				int timeLeft = (time % 100) % nPeriod;
				time -= timeLeft;
			}
			else
			{
				if (time >= 930 && time < 1030)
					time = 930;
				else if (time >= 1030 && time < 1130)
					time = 1030;
				else
				{
					int timeLeft = (time % 100) % nPeriod;
					time -= timeLeft;
				}
			}
		}
		else
			time = 0;
	}
	if (klineVec.empty() || klineVec.back().time != time 
		|| klineVec.back().date != nTradingDay)
	{
		KlineType data = { 0 };
		data.time = time;
		data.date = nTradingDay;
		data.open = indexMarket.LastPrice;
		data.high = indexMarket.LastPrice;
		data.low = indexMarket.LastPrice;
		data.close = indexMarket.LastPrice;
		data.vol = indexMarket.Volume - LastVol;
		LastVol = indexMarket.Volume;
		klineVec.emplace_back(data);
	}
	else
	{
		auto &lastData = klineVec.back();
		lastData.close = indexMarket.LastPrice;
		lastData.high = max(indexMarket.LastPrice, lastData.high);
		lastData.low = min(indexMarket.LastPrice, lastData.low);
		lastData.vol += indexMarket.Volume - LastVol;
		LastVol = indexMarket.Volume;
	}
	return true;
}

bool CDataProc::CalcMA(vector<MAType>& MaVec, 
	const vector<KlineType>& klineVec, int Period[4])
{
	auto size = klineVec.size();
	MAType data = { 0 };
	data.date = klineVec.back().date;
	data.time = klineVec.back().time;
	data.MA1 = size < Period[0] ? NAN : 
		accumulate(klineVec.begin() + size - Period[0],
			klineVec.end(), 0.0,
		[&](double a, const KlineType &b)
	{return a + b.close; }) / Period[0];
	data.MA2 = size < Period[1] ? NAN : 
		accumulate(klineVec.begin() + size - Period[1], 
			klineVec.end(), 0.0,
		[&](double a, const KlineType &b) 
	{return a + b.close; }) / Period[1];
	data.MA3 = size < Period[2] ? NAN : 
		accumulate(klineVec.begin() + size - Period[2],
			klineVec.end(), 0.0,
		[&](double a, const KlineType &b)
	{return a + b.close; }) / Period[2];
	data.MA4 = size < Period[3] ? NAN : 
		accumulate(klineVec.begin() + size - Period[3],
			klineVec.end(), 0.0,
		[&](double a, const KlineType &b) 
	{return a + b.close; }) / Period[3];
	return true;
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
		data.value = (EMA5Vec.back().value -
			EMA20Vec.back().value - DEA520Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD520");

		data.value = EMA20Vec.back().value - EMA60Vec.back().value;
		SetDEA(dataMap, data, 10, "DEA2060");
		data.value = DEA2060Vec.back().value / EMA60Vec.back().value * 100;
		UpdateOnceTmData(comData, data, StockID, "RPS2060");
		data.value = (EMA20Vec.back().value - 
			EMA60Vec.back().value - DEA2060Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD2060");
		auto lastData(closeVec.back());
		closeVec.clear();
		closeVec.emplace_back(lastData);

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
		//if (!EMA5Vec.empty())
		//	continue;

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
		data.value = (EMA5Vec.back().value -
			EMA20Vec.back().value - DEA520Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD520");
		data.value = (EMA20Vec.back().value -
			EMA60Vec.back().value - DEA2060Vec.back().value) * 2;
		UpdateOnceTmData(comData, data, StockID, "MACD2060");
		auto lastData (closeVec.back());
		closeVec.clear();
		closeVec.emplace_back(lastData);
		closeVec.shrink_to_fit();
	}
	return true;
}

bool CDataProc::SetPreEMAData(TimeLineArrMap & comData, const TimeLineData& data)
{
	auto &dataMap = comData[data.securityID];
	auto &dataVec = dataMap[data.dataName];

	if (dataVec.empty())
	{
		dataVec.reserve(2);
		dataVec.emplace_back(data.data);
		dataVec.emplace_back(data.data);
	}
	else
	{
		dataVec.clear();
		dataVec[0] = data.data;
		dataVec[1] = data.data;
	}
	return true;
}

void CDataProc::UpdateTmData(TimeLineArrMap& comData, const TimeLineData & data)
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

void CDataProc::UpdateTmData(TimeLineArrMap & comData, CoreData & data,
	SStringA SecurityID, SStringA dataName)
{
	auto & dataVec = comData[SecurityID][dataName];
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

void CDataProc::UpdateTmData(vector<CoreData>& comData,
	const TimeLineData & data)
{
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

void CDataProc::UpdateTmData(vector<CoreData>& comData, CoreData & data)
{
	if (comData.empty())
		comData.emplace_back(data);
	else if (comData.back().date < data.date)
		comData.emplace_back(data);
	else if (comData.back().date == data.date)
	{
		if (comData.back().time < data.time)
			comData.emplace_back(data);
		else if (comData.back().time == data.time)
			comData.back() = data;
	}

}

void CDataProc::UpdateClose(TimeLineArrMap& comData,
	const TimeLineData & data,
	int nPeriod)
{
	if (data.data.value == 0)
		return;


	auto & dataVec = comData[data.securityID][data.dataName];
	int time = data.data.time;

	if (nPeriod != Period_FenShi)
	{
		if (time < 930)
			return;

		if (time != 1130 && time != 1500)
			++time;

		if (nPeriod / Period_1Day == 0)
		{
			if (nPeriod == Period_60Min)
			{
				if (time > 930 && time <= 1030)
					time = 1030;
				else if (time > 1030 && time <= 1130)
					time = 1130;
				else if (time % 100 != 0)
					time = (time / 100 + 1) * 100;
			}
			else if (nPeriod != Period_1Min)
			{
				int nLeft = time % 100 % nPeriod;
				if (nLeft != 0)
					time = time + nPeriod - nLeft;
			}
			if (time % 100 == 60)
				time += 40;
		}
		else
			time = 0;
	}
	if (dataVec.empty())
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date < data.data.date)
		dataVec.emplace_back(data.data);
	else if (dataVec.back().date == data.data.date)
	{
		if (dataVec.back().time < time)
			dataVec.emplace_back(data.data);
		else if (dataVec.back().time == time)
			dataVec.back() = data.data;
	}
	dataVec.back().time = time;
}

void CDataProc::UpdateClose(vector<CoreData>& comData,
	const TimeLineData & data, int nPeriod)
{
	if (data.data.value == 0)
		return;

	int time = data.data.time;

	if (nPeriod != Period_FenShi)
	{
		if (time < 930)
			return;

		if (time != 1130 && time != 1500)
			++time;

		if (nPeriod / Period_1Day == 0)
		{
			if (nPeriod == Period_60Min)
			{
				if (time > 930 && time <= 1030)
					time = 1030;
				else if (time > 1030 && time <= 1130)
					time = 1130;
				else if (time % 100 != 0)
					time = (time / 100 + 1) * 100;
			}
			else if (nPeriod != Period_1Min)
			{
				int nLeft = time % 100 % nPeriod;
				if (nLeft != 0)
					time = time + nPeriod - nLeft;
			}
			if (time % 100 == 60)
				time += 40;
		}
		else
			time = 0;
	}
	if (comData.empty())
		comData.emplace_back(data.data);
	else if (comData.back().date < data.data.date)
		comData.emplace_back(data.data);
	else if (comData.back().date == data.data.date)
	{
		if (comData.back().time < time)
			comData.emplace_back(data.data);
		else if (comData.back().time == time)
			comData.back() = data.data;
	}
	comData.back().time = time;

}

void CDataProc::UpdateHisData(TimeLineArrMap& comData, 
	TimeLineData * dataArr, int dataCount, int Period)
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
		comMap["close"].insert(comMap["close"].begin(), 
			dataVec.begin(), dataVec.end());
	}
}

bool CDataProc::RankPoint(TimeLineArrMap& comData,
	TimeLineArrMap& uniData,
	vector<SStringA>& StockIDVec,
	unordered_map<SStringA, StockInfo, hash_SStringA> &StockInfoMap)
{
	vector<pair<SStringA, CoreData>> rpsData;
	rpsData.reserve(StockIDVec.size());
	for (auto &stockID : StockIDVec)
	{
		auto &dataVec = comData[stockID]["RPS520"];
		if (!dataVec.empty())
			rpsData.emplace_back(make_pair(stockID, dataVec.back()));
	}
	RankPoint(rpsData, uniData,
		"RPS520", "Rank520", "Point520", StockInfoMap);
	rpsData.clear();
	for (auto &stockID : StockIDVec)
	{
		auto &dataVec = comData[stockID]["RPS2060"];
		if (!dataVec.empty())
			rpsData.emplace_back(make_pair(stockID, dataVec.back()));
	}
	RankPoint(rpsData, uniData,
		"RPS2060", "Rank2060", "Point2060", StockInfoMap);
	return true;
}

bool CDataProc::RankPointHisData(TimeLineArrMap& comData,
	TimeLineArrMap& uniData,
	vector<SStringA>& StockIDVec,
	unordered_map<SStringA, StockInfo, hash_SStringA> &StockInfoMap)
{
	map<int64_t, vector<pair<SStringA, CoreData>>> rpsData;
	for (auto &it : StockIDVec)
	{
		ClearRankPointData(uniData[it]);
		auto &data520Vec = comData[it]["RPS520"];
		for (auto &itor : data520Vec)
		{
			int64_t datetime = (int64_t)itor.date * 10000 + (int64_t)itor.time;
			rpsData[datetime].emplace_back(make_pair(it, itor));
		}
	}
	for (auto &it : rpsData)
		RankPoint(it.second, uniData,
			"RPS520", "Rank520", "Point520", StockInfoMap);

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
		RankPoint(it.second, uniData,
			"RPS2060", "Rank2060", "Point2060", StockInfoMap);
	return true;
}

bool CDataProc::UpdateShowData(TimeLineArrMap& comData, TimeLineArrMap& uniData,
	TimeLineMap& ShowData, vector<SStringA>& comDataNameVec,
	vector<SStringA>& uniDataNameVec, vector<SStringA>& StockIDVec)
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


bool CDataProc::SetPeriodFenshiOpenEMAData(TimeLineArrMap & comData,
	const TimeLineData & data)
{
	vector<TimeLineData> tmVec = CreatePreEMAFromOpenData(data);
	for (int i = 0; i < tmVec.size(); ++i)
		SetPreEMAData(comData, tmVec[i]);
	return true;
}

bool CDataProc::RankPoint(vector<pair<SStringA, CoreData>>& dataVec,
	TimeLineArrMap& uniData, SStringA dataName,
	SStringA rankName, SStringA pointName,
	unordered_map<SStringA, StockInfo, hash_SStringA> &StockInfoMap)
{
	sort(dataVec.begin(), dataVec.end(),
		[&](const pair<SStringA, CoreData> & data1,
			const pair<SStringA, CoreData> & data2)
	{return data1.second.value > data2.second.value; });

	unordered_map<SStringA, int, hash_SStringA> IndCountMap;
	SStringA SWL1PointName = "L1" + pointName;
	SStringA SWL2PointName = "L2" + pointName;
	vector<CoreData> SWL1PointVec;
	vector<CoreData> SWL2PointVec;
	int i = 1;
	int rank = 1;
	double preValue = NAN;
	int size = dataVec.size();
	SWL1PointVec.reserve(size);
	SWL2PointVec.reserve(size);
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
		UpdateOnceTmData(dataMap[rankName], rankData);
		UpdateTmData(dataMap[pointName], pointData);
		if (!StockInfoMap.empty())
		{
			//SW1����ҵ
			const auto &info = StockInfoMap[it.first];
			if (strcmp(info.SWL1ID, "") == 0)
				continue;
			int &l1Count = IndCountMap[info.SWL1ID];
			pointData.value = ++l1Count;
			//UpdateTmData(dataMap[SWL1PointName], pointData);
			SWL1PointVec.emplace_back(pointData);
			//SWL2
			int &l2Count = IndCountMap[info.SWL2ID];
			pointData.value = ++l2Count;
			//UpdateTmData(dataMap[SWL2PointName], pointData);
			SWL2PointVec.emplace_back(pointData);
		}

	}
	int nDataCount = 0;
	if (!IndCountMap.empty())
	{
		for (auto &it : dataVec)
		{
			const auto &info = StockInfoMap[it.first];
			if (strcmp(info.SWL1ID, "") == 0)
				continue;
			auto & dataMap = uniData[it.first];
			size_t size = IndCountMap[info.SWL1ID];
			//auto &L1Data = dataMap[SWL1PointName].back();
			//L1Data.value = (size - L1Data.value) *1.0 / (size -1) * 100;
			//size = IndCountMap[info.SWL2ID];
			//auto &L2Data = dataMap[SWL2PointName].back();
			//L2Data.value = (size - L2Data.value) *1.0 / (size - 1) * 100;
			auto &L1Data = SWL1PointVec[nDataCount];
			L1Data.value = (size - L1Data.value) *1.0 / (size - 1) * 100;
			UpdateTmData(dataMap[SWL1PointName], L1Data);
			size = IndCountMap[info.SWL2ID];
			auto &L2Data = SWL2PointVec[nDataCount];
			L2Data.value = (size - L2Data.value) *1.0 / (size - 1) * 100;
			UpdateTmData(dataMap[SWL2PointName], L2Data);
			++nDataCount;
		}
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

void CDataProc::UpdateOnceTmData(TimeLineArrMap & comData, 
	CoreData & data, SStringA SecurityID, SStringA dataName)
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

vector<TimeLineData> CDataProc::CreatePreEMAFromOpenData(const TimeLineData & data)
{
	vector<TimeLineData> tmVec;
	tmVec.reserve(5);
	int i = 0;
	while (i < 5)
	{
		tmVec.emplace_back(data);
		++i;
	}
	strcpy_s(tmVec[0].dataName, "EMA5");
	strcpy_s(tmVec[1].dataName, "EMA20");
	strcpy_s(tmVec[2].dataName, "EMA60");
	strcpy_s(tmVec[3].dataName, "DEA520");
	strcpy_s(tmVec[4].dataName, "DEA2060");
	tmVec[3].data.value = 0;
	tmVec[4].data.value = 0;
	return tmVec;
}

void CDataProc::SetEMA(map<SStringA, vector<CoreData>>& dataMap,
	const CoreData& close, int nCount, SStringA dataName)
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
		if (dataVec[1].date != close.date
			|| dataVec[1].time != close.time)
		{
			dataVec[0] = dataVec[1];
			dataVec[1] = close;
			dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);
		}
		else
		{
			if (dataVec[0].time == dataVec[1].time
				&& dataVec[0].time != 0)
				dataVec[1].value = close.value;
			else
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


void CDataProc::SetDEA(map<SStringA, vector<CoreData>>& dataMap, 
	const CoreData & close, int nCount, SStringA dataName)
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
		if (dataVec[1].date != close.date
			|| dataVec[1].time != close.time)
		{
			dataVec[0] = dataVec[1];
			dataVec[1] = close;
			dataVec[1].value = EMA(nCount, dataVec[0].value, close.value);
		}
		else
		{
			if (dataVec[0].time == dataVec[1].time && dataVec[0].time != 0)
				dataVec[1].value = close.value;
			else
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

