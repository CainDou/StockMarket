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

#define MAX_BAR_COUNT		10500
#define MAX_DATA_COUNT		10500
#define MAX_MA_COUNT		4
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
	RecvMsg_RTTimeLine,
	RecvMsg_TodayData,
	RecvMsg_HisData,
	RecvMsg_HisPoint,
	RecvMsg_LastDayEma,
	RecvMsg_RTIndexMarket,
	RecvMsg_RTStockMarket,
	RecvMsg_HisIndexMarket,
	RecvMsg_HisStockMarket,
	RecvMsg_HisKline,
	RecvMsg_CloseInfo,
};

enum SendMsgType
{
	SendType_Connect = 0,
	SendType_ReConnect,
	SendType_GetHisData = 100,
	SendType_GetHisPoint,
	SendType_IndexMarket,
	SendType_StockMarket,
	SendType_HisPeriodKline,
	SendType_SubIns,
};


typedef struct _SortPara
{
	_SortPara() :nCol(0), nShowCol(0), nFlag(ST_NULL)
	{

	}
	int nCol;
	int nShowCol;
	int nFlag;
	int nDec;
}SortPara;

typedef struct _StockInfo
{
	SecurityID SecurityID;
	char SecurityName[16];
	char ExchangeID[8];
	char SimpleName[8];
	char SWL1ID[8];
	char SWL2ID[8];
	char ScaleID[8];
}StockInfo;

typedef struct _ReceiveInfo
{
	_ReceiveInfo() :MsgType(-1), nDataSize1(-1)
	{

	}
	int MsgType;
	char InsID[10];
	int Group;
	int Period;
	unsigned long nDataSize1;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long nDataSize2;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
}ReceiveInfo;

typedef struct _ReceiveIDInfo
{
	_ReceiveIDInfo() :MsgType(-1), NoUse3(-1)
	{

	}
	_ReceiveIDInfo(ReceiveInfo& ri)
	{
		memcpy_s(this, sizeof(ReceiveIDInfo),
			&ri, sizeof(ri));
	}
	int MsgType;
	char NoUse1[10];
	int ClientID;
	int NoUse2;
	unsigned long NoUse3;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long NoUse4;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
}ReceiveIDInfo;


enum SListHead
{
	SHead_Num = 0,
	SHead_ID,
	SHead_Name,
	SHead_LastPx,
	SHead_ChangePct,
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
	int  Group;
	int  Period;
}SendInfo;

typedef struct SendIDInfo
{
	int MsgType;
	char NoUse1[10];
	int  ClinetID;
	int  NoUse2;
}SendIDInfo_t;



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
	UpdateHisPoint,
	UpdateLastDayEma,
	UpdateIndexMarket,
	UpdateStockMarket,
	UpdateHisIndexMarket,
	UpdateHisStockMarket,
	UpdateHisKline,
	UpdateCloseInfo,
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
	MAINMSG_UpdatePic,
	MAINMSG_ProcFenShi,
	MAINMSG_ProcKline,
};


enum RpsGroup
{
	Group_SWL1 = 0,
	Group_SWL2,
	Group_Stock,
	Group_Count,
};


//通用的行情数据结构
typedef struct CommonStockMarket
{
	unsigned long long UpdateTime; // 行情时间 用作日线时只有日期没有时间
	SecurityID SecurityID; //证券代码
	int  State;//交易阶段代码
	double 	PreCloPrice; //昨收盘价
	long long	TradNum; //成交笔数
	long long	Volume; //成交总量
	double 	Turnover; //成交总金额
	double	LastPrice; //最新价
	double	OpenPrice; //今开盘价
	double	HighPrice; //最高价
	double	LowPrice; //最低价
	long long TotalOfferQty; //委托卖出总量（有效竞价范围内）
	double  WeightedAvgOfferPx; //加权平均卖出价格
	long long  TotalBidQty; //委托买入总量
	double  WeightedAvgBidPx; //加权平均买入价格
	double	AskPrice[10]; //卖一价
	long long	AskVolume[10]; //卖一量
	double	BidPrice[10]; //买一价
	long long	BidVolume[10]; //买一量
	int	NumOrdersB[10]; //买一委托笔数
	int	NumOrdersS[10]; //卖一委托笔数
};

typedef struct CommonStockTick
{
	SecurityID SecurityID;	//证券代码
	unsigned long long TradTime;		//成交时间
	double TradPrice;		//成交价格
	long long TradVolume;	//成交数量
	double TradeMoney;		//成交金额
	long long TradeBuyNo;		//买方单号
	long long TradeSellNo;	//卖方单号
};

typedef struct CommonStockOrder
{
	SecurityID SecurityID; //证券代码
	long long OrderTime;	//委托时间
	long long OrderNo;	//订单号
	double OrderPrice;	//委托价格
	long long OrderVolume;	//委托量
	int Side;	//方向
};

typedef struct CommonIndexMarket
{
	int TradingDay;
	int UpdateTime;
	SecurityID SecurityID;
	double	PreCloPrice;	//昨收盘价
	double	LastPrice; //最新价
	double	OpenPrice; //今开盘价
	double	HighPrice; //最高价
	double	LowPrice; //最低价
	long long Volume;
	double Turnover;
	double Amplitude;	//振幅
	double Change;
	double ChangePct;
	double VolumeRatio;	//量比
};


typedef struct KlineType
{
	double open;	//开
	double high;	//高
	double low;		//低
	double close;	//收
	double vol;		//量
	int date;		//日期
	int time;		//时间
};

typedef struct MACDType
{
	double dEMA12;
	double dEMA26;
	double dDIF;
	double dDEA;
	double dMACD;
}MACD_t;


typedef struct Band
{
	bool DataValid[MAX_BAR_COUNT];
	int nLastHighPoint1[MAX_BAR_COUNT];
	int nLastHighPoint2[MAX_BAR_COUNT];
	int nLastLowPoint1[MAX_BAR_COUNT];
	int nLastLowPoint2[MAX_BAR_COUNT];
	int nLastCrossHigh[MAX_BAR_COUNT];
	int nLastCrossLow[MAX_BAR_COUNT];
	double UpperTrack1[MAX_BAR_COUNT];
	double UpperTrack2[MAX_BAR_COUNT];
	double LowerTrack1[MAX_BAR_COUNT];
	double LowerTrack2[MAX_BAR_COUNT];
	double SellLong[MAX_BAR_COUNT];
	double BuyShort[MAX_BAR_COUNT];
	int CrossPoint1[MAX_BAR_COUNT];
	int CrossPoint2[MAX_BAR_COUNT];
	int CrossPoint3[MAX_BAR_COUNT];
	int CrossPoint4[MAX_BAR_COUNT];
	int Status[MAX_BAR_COUNT];
	double W2[MAX_BAR_COUNT];
	int BB1[MAX_BAR_COUNT];
	double dHigh1[MAX_BAR_COUNT];
	double dHigh2[MAX_BAR_COUNT];
	double dPreHigh1[MAX_BAR_COUNT];
	double dPreHigh2[MAX_BAR_COUNT];
	double dLow1[MAX_BAR_COUNT];
	double dLow2[MAX_BAR_COUNT];
	double dPreLow1[MAX_BAR_COUNT];
	double dPreLow2[MAX_BAR_COUNT];
	double Position[MAX_BAR_COUNT];
}Band_t;

typedef struct BandPara
{
	int N1;
	int N2;
	int K;
	int M1;
	int M2;
	int P;
	BandPara() :N1(8), N2(11), K(390), M1(8), M2(4), P(15)
	{}
}BandPara_t;

typedef struct MACDDataType
{
	double EMA12[MAX_BAR_COUNT];
	double EMA26[MAX_BAR_COUNT];
	double DIF[MAX_BAR_COUNT];
	double DEA[MAX_BAR_COUNT];
	double MACD[MAX_BAR_COUNT];
	double fMax;
	double fMin;
}MACDData_t;

typedef struct InitPara
{
	bool bShowMA;
	bool bShowBandTarget;
	bool bShowAverage;
	bool bShowEMA;
	bool bShowTSCMACD;
	bool bShowTSCVolume;
	bool bShowKlineVolume;
	bool bShowKlineMACD;
	bool bShowTSCRPS;
	bool bShowKlineRPS;
	int  nWidth;
	bool bShowTSCDeal;
	bool bShowKlineDeal;
	int  nEMAPara[2];
	int  nMACDPara[3];
	int	 nMAPara[4];
	bool bNoJiange;
	BandPara_t  BandPara;
	InitPara() :bShowMA(true), bShowBandTarget(false), bShowAverage(true),bShowEMA(true), 
		bShowTSCMACD(true), bShowTSCVolume(false), bShowKlineVolume(false),bShowTSCRPS(true),
		bShowKlineRPS(true), nWidth(9), bShowKlineMACD(true) , bShowTSCDeal(true),bShowKlineDeal(false),
		nEMAPara{ 12,26 }, nMACDPara{ 12,26,9 },  nMAPara{ 5,10,20,60 }, bNoJiange(false)
	{}
}InitPara_t;


typedef struct _KLINE_INFO {
	KlineType	pd[MAX_DATA_COUNT];	//data信息
	double			fMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	bool			bShow;				//是否显示

	double			fMax;
	double			fMin;
	int				nDivY;				//y轴显示系数1
	int				nMulY;				//y轴显示系数2
	int				nDecimal;			//小数位数
	TCHAR			sDecimal[10];		//用来格式化
	int				nDecimalXi;			//10的m_nDecimal次方
	int				nLast1MinTime;		//最新更新所用的1MinK线的时间
	int				nLastVolume;		//最后使用K线更新时累计的区间内成交量
	int				nLastTradingDay;	//最后使用K线更新的交易日
}KLINE_INFO;

typedef struct _FUTU_INFO {
	int			ftl[MAX_DATA_COUNT];	//副图线
	COLORREF		clft;					//

	double			fMax;
	double			fMin;
	int				nDivY;				//y轴显示系数1
	int				nMulY;				//y轴显示系数2
	int				nDecimal;			//小数位数
	TCHAR			sDecimal[10];		//用来格式化
	int				nDecimalXi;			//10的m_nDecimal次方
}FUTU_INFO;


typedef struct _FENSHI_GROUP {
	double		close;
	double		avg;
	int			vol;
	double		EMA1;
	double		EMA2;
	DWORD		time;
	DWORD		date;
	MACD_t      macd;
}FENSHI_GROUP;

typedef struct _FENSHI_ALLINFO
{
	std::vector<FENSHI_GROUP> d;	//收盘价格

	double			fMax;
	double			fMin;
	//	int				nDivY;				//y轴显示系数1
	//	int				nMulY;				//y轴显示系数2
	//	int				nDecimal;			//小数位数
	//TCHAR			sDecimal[10];		//用来格式化
	//									//	int				nDecimalXi;			//10的m_nDecimal次方

	double			fMaxf;
	double			fMinf;
	double			fMaxMACD;
	double			fMinMACD;
	double			dDelta;
	//int				nDivYf;				//y轴显示系数1
	//int				nMulYf;				//y轴显示系数2
	//int				nRatio;				//价格系数
	//int				nRatio2;			//合约2价格系数
	double			fPreClose;			//前收

	int				nLastVolume;		//上一个时段最后的数量
	int				nCount;				//已经计算过tick的数量
	int				nAllLineNum;		//分时图数据的数量

	//int				nInsType;			//期货类型 0--商品无夜盘 1--商品有夜盘23:00 2--商品有夜盘1:00  3--商品有夜盘2:30  4--股指  5--国债

	int				nMin;				//计算完的最后一个分钟
	int				nTime;				//计算完的最后一个时间

	//int				nGroupIns1Ratio;	//合约1系数
	//int				nGroupIns2Ratio;	//合约2系数
}FENSHI_INFO;

typedef struct _AllKPIC_INFO {
	KlineType		data[MAX_DATA_COUNT];	//data信息
	double			fMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	double			fMax;
	double			fMin;
	double			fSubMax;
	double			fSubMin;
	int				nLast1MinTime;		//最新更新所用的1MinK线的时间
	int				nLastVolume;		//最后使用K线更新时累计的区间内成交量
	int				nLastTradingDay;	//最后使用K线更新的交易日
	int				nTotal;
	int		nKwidth;		//k线宽度
	int		nMoveNow;		//校正后的move,防止move无限变大
	_AllKPIC_INFO() {
		clear();
	}

	void clear() {
		ZeroMemory(&data, sizeof(data));
		ZeroMemory(fMa, sizeof(fMa));
		fMax = 0;
		fMin = 0;
		fSubMax = 0;
		fSubMin = 0;
		nTotal = 0;
		nLast1MinTime = 0;
		nLastVolume = 0;
		nTotal = 0;
		nLastTradingDay = 0;
		nMoveNow = 0;
	}

}AllKPIC_INFO;

typedef struct MAType
{
	double MA1;
	double MA2;
	double MA3;
	double MA4;
	int date;
	int time;
};

typedef struct EMAType
{
	double EMA1;
	double EMA2;
};

bool GetFileKlineData(SStringA InsID, vector<KlineType> &dataVec, bool bIsDay = false);

typedef struct _PARAM_TICK_INFO {
	int			id;				//证券内部编号
	int			date;			//日期(20170101)
	int			time;			//时间(93501500)		
	double		newp;			//最新价
	int			vol;			//最新量
	int			totalvol;		//成交总数量
	double		abp[10];		//卖5买5价

	_PARAM_TICK_INFO() :id(0), date(0),
		time(0), newp(0), vol(0), totalvol(0)
	{
		ZeroMemory(abp, sizeof(int) * 10);
	}

	int Compare(const _PARAM_TICK_INFO &x)	//1大于,0等于,-1小于
	{
		return this->Compare(x.date, x.time);
	}

	int Compare(int date, int time)	//1大于,0等于,-1小于
	{
		if (this->date > date)
			return 1;
		else if (this->date < date)
			return -1;
		else if (this->time > time)
			return 1;
		else if (this->time < time)
			return -1;
		else
			return 0;
	}
}TICK_INFO, *LPTICK_INFO;


enum TimePreiod
{
	Period_FenShi = 0,
	Period_1Min = 1,
	Period_5Min = 5,
	Period_15Min = 15,
	Period_30Min = 30,
	Period_60Min = 60,
	Period_1Day = 1440,
	//Period_NULL = 65535,
};

void InitLogFile();
void TraceLog(char* log, ...);

enum LoginMsg
{
	LoginMsg_UpdateText = 0,
	LoginMsg_DestoryWnd,
};

typedef struct LoginInfo
{
	HWND loginwnd;
	bool bLogin;
	wchar_t ID[17];
	wchar_t psd[17];
}LoginInfo_t;

enum FSMSG
{
	FSMSG_PROCDATA = 0,
	FSMSG_UPDATE,
	FSMSG_EMA,
	FSMSG_MACD,
	FSMSG_COUNT,
};

enum KLINEMSG
{
	KLINEMSG_PROCDATA = 0,
	KLINEMSG_HISPOINT,
	KLINEMSG_UPDATE,
	KLINEMSG_MA,
	KLINEMSG_MACD,
	KLINEMSG_BAND,
};

enum FSMenu
{
	FM_Return = 100,
	FM_Deal,
	FM_Volume,
	FM_MACD,
	FM_RPS,
	FM_MacdPara,
	FM_Avg,
	FM_EMA,
	FM_EmaPara,
	FM_End,
};

enum KlineMenu
{
	KM_Return = 200,
	KM_Deal,
	KM_MA,
	KM_Band,
	KM_Volume,
	KM_MACD,
	KM_RPS,
	KM_MacdPara,
	KM_BandPara,
	KM_MaPara,
	KM_End,
};

enum ReceiveCommonMsgType
{
	RecvMsg_ClientID = 170000,
};
