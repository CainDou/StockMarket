#pragma once


#define InvalidThreadId       0

#include<unordered_map>
#include<string>
#include<vector>
#include<map>

using std::unordered_map;
using std::hash;
using std::string;
using std::vector;
using std::map;


typedef char SecurityID[8];


struct hash_SStringA
{
	size_t operator()(const SStringA& str) const
	{
		return hash<string>()((string)str);
	}
};


//typedef struct _TimeLineData
//{
//	char dataName[24];
//	SecurityID securityID;
//	double value;
//	int time;
//	int date;
//}TimeLineData;

typedef struct _coreData
{
	double value;
	int time;
	int date;
}CoreData;

typedef struct  _TimeLineData
{
	CoreData data;
	char dataName[12];
	SecurityID securityID;
}TimeLineData;


typedef unordered_map<SStringA, map<SStringA, CoreData>, hash_SStringA> TimeLineMap;
typedef unordered_map<SStringA, map<SStringA, vector<CoreData>>, hash_SStringA> TimeLineArrMap;


enum RecvMsgType
{
	RecvMsg_StockInfo = 182433,
	RecvMsg_UpdateIndex,
	RecvMsg_TodayData,
	RecvMsg_HisData,
};

enum SendMsgType
{
	SendType_Connect = 0,
	SendType_GetHisData = 100,
};


typedef struct _SortPara
{
	_SortPara() :nCol(0), nShowCol(0), nFlag(ST_NULL)
	{

	}
	int nCol;
	int nShowCol;
	int nFlag;
}SortPara;

typedef struct _StockInfo
{
	SecurityID SecurityID;
	char SecurityName[16];
	char ExchangeID[8];
}StockInfo;

typedef struct _ReceiveInfo
{
	_ReceiveInfo() :MsgType(-1), nDataSize1(-1)
	{

	}
	int MsgType;
	char InsID[10];
	int StartDate;
	int EndDate;
	unsigned long nDataSize1;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long nDataSize2;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
}ReceiveInfo;

enum SListHead
{
	SHead_Num = 0,
	SHead_ID,
	SHead_Name,
	SHead_LastPx,
	SHead_RPS520,
	SHead_MACD520,
	SHead_Point520,
	SHead_Rank520,
	SHead_RPS2060,
	SHead_MACD2060,
	SHead_Point2060,
	SHead_Rank2060,
};

typedef struct SendInfo
{
	int MsgType;
	char str[10];
	int  StartDate;
	int  EndDate;
}SendInfo;


SStringW StrA2StrW(const SStringA &sstrSrcA);

SStringA StrW2StrA(const SStringW &cstrSrcW);

int SendMsg(unsigned uThreadId, unsigned MsgType, char *SendBuf, unsigned BufLen);
int	RecvMsg(int  msgQId, char** buf, int& length, int timeout);

enum DataProcType
{
	UpdateData=10000,
	UpdateTodayData,
	UpdateRPS,
	UpdateHisData,
	UpdateSingleListData,
	Msg_Exit = 88888,
};

enum StockInfoType
{
	StockInfo_Stock,
	StockInfo_SWL1,
	StockInfo_SWL2,
	StockInfo_Index,
};

enum MAINMSG
{
	MAINMSG_UpdateList,
	MAINMSG_ShowPic,
};

typedef struct sendInfo
{
	int MsgType;
	char InsID[10];
	int  StartDate;
	int  EndDate;
}ReceiveInfo_t;
