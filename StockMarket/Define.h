#pragma once


#define INVALID_THREADID 0

#include<unordered_map>
#include<string>
#include<vector>
#include<map>
#include<set>
#include "IniFile.h"

using std::unordered_map;
using std::hash;
using std::string;
using std::vector;
using std::map;
using std::set;
using std::pair;


typedef char SecurityID[8];

#define MAX_BAR_COUNT		10500
#define MAX_DATA_COUNT		10500
#define MAX_MA_COUNT		6

#define MAX_NAME_LENGTH		32


struct hash_SStringA
{
	size_t operator()(const SStringA& str) const
	{
		return hash<string>()((string)(const char*)str);
	}
};

template<typename type>
struct strHash
{
public:
	unordered_map<SStringA, type, hash_SStringA> hash;
};

template<typename type>
struct strVecHash
{
	unordered_map<SStringA, vector<type>,
		hash_SStringA> vecHash;
};


template<typename type>
struct strMapHash
{
	unordered_map<SStringA, map<SStringA, type>, hash_SStringA> mapHash;
};

template<typename type>
struct strMapVecHash
{
	unordered_map<SStringA, map<SStringA, vector<type>>, hash_SStringA> mapVecHash;
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


//typedef unordered_map<SStringA, map<SStringA, CoreData>, hash_SStringA> TimeLineMap;
//typedef unordered_map<SStringA, map<SStringA, vector<CoreData>>, hash_SStringA> TimeLineArrMap;


enum RecvMsgType
{
	RecvMsg_StockInfo = 182433,
	RecvMsg_RTTimeLine,
	RecvMsg_TodayTimeLine,
	RecvMsg_NoUse,
	RecvMsg_HisRpsPoint,
	RecvMsg_LastDayEma,
	RecvMsg_RTIndexMarket,
	RecvMsg_RTStockMarket,
	RecvMsg_HisIndexMarket,
	RecvMsg_HisStockMarket,
	RecvMsg_HisKline,
	RecvMsg_CloseInfo,
	RecvMsg_Wait,
	RecvMsg_Reinit,
	RecvMsg_RTTFMarket,
	RecvMsg_RTRps,
	RecvMsg_HisSecPoint,
	RecvMsg_RehabInfo,
	RecvMsg_CallAction,
	RecvMsg_HisCallAction,
	RecvMsg_HisMultiData,
	RecvMsg_HisIndexKline,
	RecvMsg_HisMultiDataForHSF,
	RecvMsg_HisTFBase,
	RecvMsg_TodayTFMarket,

};

enum SendMsgType
{
	SendType_MsgNoUse = 100,
	SendType_GetHisRpsPoint,
	SendType_IndexMarket,
	SendType_StockMarket,
	SendType_HisPeriodKline,
	SendType_SubIns,
	SendType_GetHisSecPoint,
	SendType_HisCallAction,
	SendType_HisMultiData,
	SendType_IndexHisDayKline,
	SendType_HisTFBase,
};

enum ComSendMsgType
{
	ComSend_Connect = 0,
	ComSend_ReConnect,
	ComSend_FileVer = 1000,
	ComSend_File,
	ComSend_UpdateFile,
	ComSend_UpdateFileVer,
};

enum BackTestingMsgType
{
	BTM_UpdateList = 0,
	BTM_GetData,
	BTM_SingleCalcFinish,
	BTM_AllFinish,
	BTM_GetHisData,
	BTM_SingleHisCalcFinish,
	BTM_AllHisFinish,

};

enum LimitUpMsgType
{
	LUM_UpdatePlate = 0,
	LUM_UpdateToday,
	LUM_UpdateShowPlate,
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
	int	 IPODate;
}StockInfo;

typedef struct _ReceiveInfo
{
	_ReceiveInfo() :MsgType(-1), DataSize(-1)
	{

	}
	int MsgType;
	char InsID[10];
	int Group;
	int Period;
	unsigned long DataSize;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long SrcDataSize;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
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

typedef struct _ReceiveStockInfo
{
	_ReceiveStockInfo() :MsgType(-1), DataSize(-1)
	{

	}
	_ReceiveStockInfo(ReceiveInfo& ri)
	{
		memcpy_s(this, sizeof(ReceiveStockInfo),
			&ri, sizeof(ri));
	}
	int MsgType;
	char NoUse1[10];
	int ClientID;
	int NoUse2;
	unsigned long DataSize;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long InfoType;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
}ReceiveStockInfo;

typedef struct _ReceivePointInfo
{
	_ReceivePointInfo() :MsgType(-1), TotalDataSize(-1)
	{

	}
	_ReceivePointInfo(ReceiveInfo& ri)
	{
		memcpy_s(this, sizeof(ReceivePointInfo),
			&ri, sizeof(ri));
	}
	int MsgType;
	char Message[10];
	int Group;
	int Period;
	unsigned long TotalDataSize;		//snap数据时 stockIndex的数据大小;kline数据时 压缩后数据大小
	unsigned long FirstDataSize;		//snap数据时 futures的数据大小;kline数据时 原始数据大小
}ReceivePointInfo;


enum SListHead
{
	SHead_Num = 0,
	SHead_ID,
	SHead_Name,
	SHead_LastPx,
	SHead_ChangePct,
	SHead_CloseRPS520,
	SHead_CloseMACD520,
	SHead_ClosePoint520,
	SHead_CloseRank520,
	SHead_CloseRPS2060,
	SHead_CloseMACD2060,
	SHead_ClosePoint2060,
	SHead_CloseRank2060,
	SHead_AmountRPS520,
	SHead_AmountMACD520,
	SHead_AmountPoint520,
	SHead_AmountRank520,
	SHead_AmountRPS2060,
	SHead_AmountMACD2060,
	SHead_AmountPoint2060,
	SHead_AmountRank2060,
	SHead_AmountPoint,
	SHead_AmountRank,
	SHead_CAVol,
	SHead_CAVolPoint,
	SHead_CAVolRank,
	SHead_CAAmo,
	SHead_CAAmoPoint,
	SHead_CAAmoRank,
	SHead_CommonItmeCount,
	SHead_TickFlowStart = SHead_CommonItmeCount,
	SHead_ActBuySellRatio = SHead_TickFlowStart,
	SHead_ActToPasBuySellRatio,
	SHead_AvgBuySellRatio,
	SHead_POCRatio,
	SHead_Volume,
	SHead_ActSellVolume,
	SHead_ActBuyVolume,
	SHead_Open,
	SHead_High,
	SHead_Low,
	SHead_Amount,
	SHead_ActBuyOrder,
	SHead_ActSellOrder,
	SHead_AvgActBuyNum,
	SHead_AvgActSellNum,
	SHead_StockItemCount,
};

enum SBackTestingListHead
{
	SBTH_Num = 0,
	SBTH_ID,
	SBTH_Date,
	SBTH_ROR1,
	SBTH_ROR3,
	SBTH_ROR5,
	SBTH_ROR10,
	SBTH_ROR1Over300,
	SBTH_ROR3Over300,
	SBTH_ROR5Over300,
	SBTH_ROR10Over300,
	SBTH_ROR1OverIndy1,
	SBTH_ROR3OverIndy1,
	SBTH_ROR5OverIndy1,
	SBTH_ROR10OverIndy1,
	SBTH_ROR1OverIndy2,
	SBTH_ROR3OverIndy2,
	SBTH_ROR5OverIndy2,
	SBTH_ROR10OverIndy2,
	SBTH_ItemCount,
};

enum SBackTestingAvgListHead
{
	SBTAH_DataCount = 0,
	SBTAH_ROR1,
	SBTAH_ROR3,
	SBTAH_ROR5,
	SBTAH_ROR10,
	SBTAH_ROR1Over300,
	SBTAH_ROR3Over300,
	SBTAH_ROR5Over300,
	SBTAH_ROR10Over300,
	SBTAH_ROR1OverIndy1,
	SBTAH_ROR3OverIndy1,
	SBTAH_ROR5OverIndy1,
	SBTAH_ROR10OverIndy1,
	SBTAH_ROR1OverIndy2,
	SBTAH_ROR3OverIndy2,
	SBTAH_ROR5OverIndy2,
	SBTAH_ROR10OverIndy2,
	SBTAH_ItemCount,
};


const int g_nListNewItemStart = SHead_CAVol;
const int g_nListNewItemEnd = SHead_CAAmoRank;


typedef struct _SendInfo
{
	int MsgType;
	char str[10];
	int  Group;
	int  Period;
}SendInfo;

typedef struct _SendIDInfo
{
	int MsgType;
	char NoUse1[10];
	int  ClinetID;
	int  NoUse2;
}SendIDInfo;


typedef struct _SendTradeInfo
{
	int MsgType;
	char Acc[10];
	int  ClinetID;
	int  NoUse2;
}SendTradeInfo;


typedef struct _SendInfoWithDate
{
	int MsgType;
	char StockID[10];
	int  StartDate;
	int  EndDate;
}SendInfoWithDate;



SStringW StrA2StrW(const SStringA &sstrSrcA);

SStringA StrW2StrA(const SStringW &cstrSrcW);

int SendMsg(unsigned uThreadId, unsigned MsgType, const char *SendBuf, unsigned BufLen);
int	RecvMsg(int  msgQId, char** buf, int& length, int timeout);

HRESULT OpenFile(LPTSTR FileName);
HRESULT SaveFile(LPCTSTR DefaultFileName, LPTSTR FileName);


enum DataProcType
{
	UpdateData = 10000,
	UpdateTodayData,
	UpdateLastDayEma,
	ClearOldData,
	UpdateTFMarket,
	UpdateRtRps,
	UpdateCallAction,
	BackTesting,
	GetTodayLimitUp,
	GetPlateLimitUp,
	ReInitLimitUp,
	limitUpReconnect,
	CheckHisFilterPass,
	Msg_ReInit = 77777,
	Msg_Exit = 88888,
};

enum SynMsg
{
	Syn_AddWnd = 20000,
	Syn_RemoveWnd,
	Syn_ListData,
	Syn_GetMarket,
	Syn_GetKline,
	Syn_GetPoint,
	Syn_Point,
	Syn_NoUse,
	Syn_HisRpsPoint,
	Syn_RTIndexMarket,
	Syn_RTStockMarket,
	Syn_HisIndexMarket,
	Syn_HisStockMarket,
	Syn_HisKline,
	Syn_CloseInfo,
	Syn_Reinit,
	Syn_FilterData,
	Syn_GetSecPoint,
	Syn_HisSecPoint,
	Syn_RehabInfo,
	Syn_HisCallAction,
	Syn_GetCallAction,
	Syn_HisTFBase,
	Syn_GetHisTFBase,
	Syn_TodayTFMarket,
	Syn_RTTFMarkt,
	Syn_GetTradeMarket,
	Syn_RTBuyStockMarket,
	Syn_RTSellStockMarket,
	Syn_HisBuyStockMarket,
	Syn_HisSellStockMarket,

	Syn_ReLogin,

};



enum WorkWndMsg
{
	WW_ListData = Syn_ListData,
	WW_GetMarket,
	WW_GetKline,
	WW_GetPoint,
	WW_Point,
	WW_NoUse,
	WW_HisRpsPoint,
	WW_RTIndexMarket,
	WW_RTStockMarket,
	WW_HisIndexMarket,
	WW_HisStockMarket,
	WW_HisKline,
	WW_CloseInfo,
	WW_Reinit,
	WW_ChangeIndy,
	WW_OuterMsgEnd,
	WW_FSEma,
	WW_FSMacd,
	WW_KlineMa,
	WW_KlineMacd,
	WW_KlineBand,
	WW_InnerMsgEnd,
	WW_ChangeStockFilter,
	WW_SaveStockFilter,
	WW_HisSecPoint,
	WW_RehabInfo,
	WW_ChangeRehab,
	WW_FixedTimeRehab,
	WW_HisCallAction,
	WW_GetCallAction,
	WW_ChangeHisFiterState,
	WW_HisFilterStartCalc,
	WW_HisFilterEndCalc,
	WW_HisTFBase,
	WW_GetHisTFBase,
	WW_TodayTFMarket,
	WW_RTTFMarket,

};



enum StockInfoType
{
	StockInfo_Stock,
	StockInfo_SWL1,
	StockInfo_SWL2,
	StockInfo_Index,
};

enum WDMSG
{
	WDMsg_UpdateListData,
	WDMsg_UpdatePic,
	WDMsg_SubIns,
	WDMsg_HideWindow,
	WDMsg_ReInit,
	WDMsg_ChangeShowList,
	WDMsg_SetFocus,
	WDMsg_SaveConfig,
	WDMsg_NewWindow,
	WDMsg_OpenWindow,
	WDMsg_ChangeStockFilter,
	WDMsg_SaveStockFilter,
	WDMsg_ChangePointTarget,
	WDMsg_ChangeShowTilte,
	WDMsg_SaveListConfig,
	WDMsg_UpdateList,
	WDMsg_SetFilterName,
	WDMsg_ChangeHisStockFilter,
	WDMsg_HisFilterStartCalc,
	WDMsg_HisFilterEndCalc,

	WDMsg_Exit,

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


typedef struct _KlineWithAmount
{
	int date;		//日期
	int time;		//时间
	double open;	//开
	double high;	//高
	double low;		//低
	double close;	//收
	double vol;		//量
	double amount;	//金额
}KlineType;

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

typedef struct _TFDataType
{
	double	ABSR[MAX_BAR_COUNT];
	double	A2PBSR[MAX_BAR_COUNT];
	double	AABSR[MAX_BAR_COUNT];
	int		ActBuyVol[MAX_BAR_COUNT];
	int		ActSellVol[MAX_BAR_COUNT];
	double	AvgActBuyVol[MAX_BAR_COUNT];
	double	AvgActSellVol[MAX_BAR_COUNT];
	int		ActBuyOrder[MAX_BAR_COUNT];
	int		ActSellOrder[MAX_BAR_COUNT];
	double	fMaxRatio;
	double	fMinRatio;
	int		nMaxActVol;
	int		nMinActVol;
	int		nMaxActOrder;
	int		nMinActOrder;
	double	fMaxAvgVol;
	double	fMinAvgVol;

}TFData;



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
	KlineType		srcData[MAX_DATA_COUNT]; //原始未复权数据
	KlineType		data[MAX_DATA_COUNT];	//复权后的K线
	double			fMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	double			fVolMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	double			fAmoMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	double			fMax;
	double			fMin;
	double			fVolMax;
	double			fVolMin;
	double			fAmountMax;
	double			fAmountMin;
	double			fLastAmount;
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
		fVolMax = 0;
		fVolMin = 0;
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
	Period_End = 65535,
};

TimePreiod& operator ++(TimePreiod &tp);

void InitLogFile();
void TraceLog(char* log, ...);

enum LoginMsg
{
	LoginMsg_UpdateText = 0,
	LoginMsg_HideWnd,
	LoginMsg_WaitAndTry,
	LoginMsg_Exit,
	LoginMsg_Reinit,
	LoginMsg_TradeLoginFeedBack,
	LoginMsg_ReLogin,

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
	FSMSG_UPDATE = 0,
	FSMSG_EMA,
	FSMSG_MACD,
	FSMSG_COUNT,
};

enum KLINEMSG
{
	KLINEMSG_UPDATE = 0,
	KLINEMSG_MA,
	KLINEMSG_MACD,
	KLINEMSG_BAND,
	KLINEMSG_REHAB,
};

enum FSMenu
{
	FM_Return = 100,
	FM_Deal,
	FM_Volume,
	FM_MACD,
	//FM_RPS,
	FM_PonitWndNum,
	FM_MacdPara,
	FM_Avg,
	FM_EMA,
	FM_EmaPara,
	//FM_L1RPS,
	//FM_L2RPS,
	FM_PointWnd0,
	FM_PointWnd1,
	FM_PointWnd2,
	FM_PointWnd3,
	FM_PointWnd4,
	FM_PointWnd5,
	FM_PointWnd6,
	FM_PointWnd7,
	FM_PointWnd8,
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
	//KM_RPS,
	KM_PonitWndNum,
	KM_MacdPara,
	KM_BandPara,
	KM_MaPara,
	//KM_L1RPS,
	//KM_L2RPS,
	KM_PointWnd0,
	KM_PointWnd1,
	KM_PointWnd2,
	KM_PointWnd3,
	KM_PointWnd4,
	KM_PointWnd5,
	KM_PointWnd6,
	KM_PointWnd7,
	KM_PointWnd8,
	KM_VolOrAmo,
	KM_Amount,
	KM_VolMaPara,
	KM_AmoMaPara,
	KM_CAVolOrAmo,
	KM_CAVol,
	KM_CAAmo,
	KM_CAVolMaPara,
	KM_CAAmoMaPara,
	KM_TickFlow,
	KM_TFRatio,
	KM_TFVol,
	KM_TFOrder,
	KM_TFAvgVol,
	KM_End,
};

enum WDMenu
{
	WDM_Return = 300,
	WDM_New,
	WDM_Open,
	WDM_End,
};

//指标选择
enum TSMenu
{
	TSM_Close = 400,
	TSM_Select,
	TSM_End,
};

enum ReceiveCommonMsgType
{
	RecvMsg_ClientID = 170000,
	RecvMsg_ClientFileVer,
	RecvMsg_ClientFile,
	RecvMsg_UpdateFile,
	RecvMsg_UpdateFileVer,
};


enum eSubPic
{
	SP_FULLMARKET = 0,
	SP_SWINDYL1,
	SP_SWINDYL2,
	SP_COUNT,
};

typedef struct _DataGetInfo
{
	HWND hWnd;
	char StockID[8];
	int	 Group;
	int Period;
}DataGetInfo;


enum
{
	DT_ListData = 0,
	DT_TFMarket,
	DT_FilterData,
	DT_IndexMarket,
	DT_StockMarket,
	DT_Kline,
	DT_Point,
	DT_L1IndyFilterData,
	DT_L2IndyFilterData,
	DT_CallAction,
	DT_StockPos,
	DT_L1IndyIndexPos,
	DT_L2IndyIndexPos,
};


enum SF_FUNC
{
	SFF_Add = 0,
	SFF_Change,
	SFF_Delete,
	SFF_Count,
};

enum SF_PERIOD
{
	SFP_Null = -1,
	SFP_D1 = 0,
	SFP_FS,
	SFP_M1,
	SFP_M5,
	SFP_M15,
	SFP_M30,
	SFP_M60,
	SFP_Count,
};

enum SF_INDEX
{
	SFI_Start = 0,
	SFI_ChgPct = SFI_Start,
	SFI_CloseRps520,
	SFI_CloseMacd520,
	SFI_ClosePoint520,
	SFI_CloseRank520,
	SFI_CloseRps2060,
	SFI_CloseMacd2060,
	SFI_ClosePoint2060,
	SFI_CloseRank2060,
	SFI_Num,
	SFI_ABSR,
	SFI_A2PBSR,
	SFI_AABSR,
	SFI_POCR,
	SFI_Vol,
	SFI_Amount,
	SFI_AmountRps520,
	SFI_AmountMacd520,
	SFI_AmountPoint520,
	SFI_AmountRank520,
	SFI_AmountRps2060,
	SFI_AmountMacd2060,
	SFI_AmountPoint2060,
	SFI_AmountRank2060,
	SFI_AmountPoint,
	SFI_AmountRank,
	//在一级行业和二级行业中的计算数据
	SFI_CloseRps520L1,
	SFI_CloseMacd520L1,
	SFI_ClosePoint520L1,
	SFI_CloseRank520L1,
	SFI_CloseRps2060L1,
	SFI_CloseMacd2060L1,
	SFI_ClosePoint2060L1,
	SFI_CloseRank2060L1,
	SFI_AmountRps520L1,
	SFI_AmountMacd520L1,
	SFI_AmountPoint520L1,
	SFI_AmountRank520L1,
	SFI_AmountRps2060L1,
	SFI_AmountMacd2060L1,
	SFI_AmountPoint2060L1,
	SFI_AmountRank2060L1,
	SFI_AmountPointL1,
	SFI_AmountRankL1,
	SFI_CloseRps520L2,
	SFI_CloseMacd520L2,
	SFI_ClosePoint520L2,
	SFI_CloseRank520L2,
	SFI_CloseRps2060L2,
	SFI_CloseMacd2060L2,
	SFI_ClosePoint2060L2,
	SFI_CloseRank2060L2,
	SFI_AmountRps520L2,
	SFI_AmountMacd520L2,
	SFI_AmountPoint520L2,
	SFI_AmountRank520L2,
	SFI_AmountRps2060L2,
	SFI_AmountMacd2060L2,
	SFI_AmountPoint2060L2,
	SFI_AmountRank2060L2,
	SFI_AmountPointL2,
	SFI_AmountRankL2,
	SFI_ABV,
	SFI_ASV,
	SFI_ABO,
	SFI_ASO,
	SFI_PBO,
	SFI_PSO,

	SFI_Open,
	SFI_High,
	SFI_Low,
	SFI_Close,

	SFI_CAVol,
	SFI_CAVolPoint,
	SFI_CAVolRank,
	SFI_CAVolPointL1,
	SFI_CAVolRankL1,
	SFI_CAVolPointL2,
	SFI_CAVolRankL2,

	SFI_CAAmo,
	SFI_CAAmoPoint,
	SFI_CAAmoRank,
	SFI_CAAmoPointL1,
	SFI_CAAmoRankL1,
	SFI_CAAmoPointL2,
	SFI_CAAmoRankL2,

	//对应一级行业和二级行业的数据

	SFI_LastPx,

	SFI_Count,

};
enum SF_L1INDYINDEX
{
	SFI_L1IndyStart = 1000,
	SFI_L1IndyCloseRps520 = SFI_L1IndyStart,
	SFI_L1IndyCloseMacd520,
	SFI_L1IndyClosePoint520,
	SFI_L1IndyCloseRank520,
	SFI_L1IndyCloseRps2060,
	SFI_L1IndyCloseMacd2060,
	SFI_L1IndyClosePoint2060,
	SFI_L1IndyCloseRank2060,
	SFI_L1IndyAmountRps520,
	SFI_L1IndyAmountMacd520,
	SFI_L1IndyAmountPoint520,
	SFI_L1IndyAmountRank520,
	SFI_L1IndyAmountRps2060,
	SFI_L1IndyAmountMacd2060,
	SFI_L1IndyAmountPoint2060,
	SFI_L1IndyAmountRank2060,
	SFI_L1IndyAmountPoint,
	SFI_L1IndyAmountRank,
	SFI_L1IndyCount,

};

enum SF_L2INDYINDEX
{
	SFI_L2IndyStart = 2000,
	SFI_L2IndyCloseRps520 = SFI_L2IndyStart,
	SFI_L2IndyCloseMacd520,
	SFI_L2IndyClosePoint520,
	SFI_L2IndyCloseRank520,
	SFI_L2IndyCloseRps2060,
	SFI_L2IndyCloseMacd2060,
	SFI_L2IndyClosePoint2060,
	SFI_L2IndyCloseRank2060,
	SFI_L2IndyAmountRps520,
	SFI_L2IndyAmountMacd520,
	SFI_L2IndyAmountPoint520,
	SFI_L2IndyAmountRank520,
	SFI_L2IndyAmountRps2060,
	SFI_L2IndyAmountMacd2060,
	SFI_L2IndyAmountPoint2060,
	SFI_L2IndyAmountRank2060,
	SFI_L2IndyAmountPoint,
	SFI_L2IndyAmountRank,
	SFI_L2IndyCount,
};

enum SF_CONDITION
{
	SFC_Greater = 0,
	SFC_EqualOrGreater,
	SFC_Equal,
	SFC_EqualOrLess,
	SFC_Less,
	SFC_Count,
};

typedef struct _StockFilterCondition
{
	string frml;
	vector<double> paraVec;
	int nPeriod;
}SFCondition;

enum ConditionsState
{
	CS_And = 0,
	CS_Or,
};

typedef struct _StockFiltrePlan
{
	vector<SFCondition> condVec;
	ConditionsState state;
}SFPlan;

enum SF_LISTHEAD
{
	SFLH_ID = 0,
	SFLH_Index1,
	SFLH_Period1,
	SFLH_Condition,
	SFLH_Index2,
	SFLH_Period2OrNum,
	SFLH_CountDay,
	SFLH_JudgeType,
};

enum SF_DOMAIN
{
	SFD_ST = 0,
	SFD_SBM,
	SFD_STARM,
	SFD_NewStock,
	SFD_Count,
};

typedef struct _TickFlowMarket
{
	SecurityID SecurityID;
	int nDelta;						//K线的Delta值
	uint64_t nVolume;						//K线的总成交量
	int nPeriod;
	double fPrice;
	double fPOC;						//POC价格
	double fOpen;
	double fHigh;
	double fLow;
	double fClose;
	double fAmount;
	uint32_t nTime;						//时间
	uint64_t ActBuyVol;
	uint64_t ActSellVol;
	uint32_t uActBuyOrderCount;	//股票主动买入订单数 或 期货多开量
	uint32_t uActSellOrderCount;	//股票主动卖出订单数 或 期货空开量
	uint32_t uPasBuyOrderCount;	//股票被动买入订单数 或 期货空平量
	uint32_t uPasSellOrderCount;	//股票被动卖出订单数 或 期货多平量
	double ABSR;
	double A2PBSR;
	double AABSR;
	double POCR;
}TickFlowMarket;

void OutputDebugStringFormat(char *fmt, ...);

enum FUNCMSG
{
	FNCMsg_InsertFunc = 0,
};


enum FORMULAMSG
{
	FrmlMsg_AddFrml = 0,
	FrmlMsg_ChangeFrml,
};

enum FILTERMSG
{
	FilterMsg_ReinitFrml = 0,
	FilterMsg_Search,
	FilterMsg_UpdateFrml,
	FilterMsg_SaveFrmlList,
	FilterMsg_ReadFrmlList,

};

enum SerachDir
{
	SD_Up = 0,
	SD_Down
};

HRESULT OpenFile(LPTSTR FileName, COMDLG_FILTERSPEC fileType[], size_t arrySize, LPCTSTR filePath);
HRESULT SaveFile(LPCTSTR DefaultFileName, LPTSTR FileName,
	COMDLG_FILTERSPEC fileType[], size_t arrySize, LPCTSTR filePath);

typedef struct _rpsdata
{
	double fMacd520;
	double fMacd2060;
	double fRps520;
	double fRps2060;
	double fPoint520;
	double fPoint2060;
	int	nRank520;
	int nRank2060;
	double fL1Point520;
	double fL1Point2060;
	int	nL1Rank520;
	int nL1Rank2060;
	double fL2Point520;
	double fL2Point2060;
	int	nL2Rank520;
	int nL2Rank2060;
}sRps;

typedef struct _sectionData
{
	double point;
	double pointL1;
	double pointL2;
	int rank;
	int rankL1;
	int rankL2;
}sSection;

typedef struct _RtRpsData
{
	SecurityID SecurityID;
	int nGroup;
	double fPrice;
	int nDate;
	int nTime;
	int nPeriod;
	sRps rpsClose;
	sRps rpsAmount;
	sSection secAmount;
}RtRps;

inline double EMA(int nCount, double preEMA, double data)
{
	return preEMA*(nCount - 1) / (nCount + 1) + data * 2 / (nCount + 1);
}

enum ePointType
{
	eRpsPoint = 0,
	eSecPoint,
};

typedef struct _UsedPointInfo
{
	int overallType;
	ePointType type;
	SStringA srcDataName;
	SStringA dataInRange;		//打分数据所在的范围
	SStringA IndyRange;			//行业范围
	SStringA showName;
	bool operator <(const _UsedPointInfo& other) const;
	bool operator ==(const _UsedPointInfo& other) const;
}ShowPointInfo;



// 具体的打分数据类型
enum ePointDataType
{
	ePointStart,
	eFullMarketPointStart = ePointStart,
	eRpsPoint_Close = eFullMarketPointStart,
	eRpsPoint_Amount,
	eSecPoint_Amount,
	eFullMarketPointEnd,
	eIndyMarketPointStart = eFullMarketPointEnd,
	eRpsPoint_L1_Close = eIndyMarketPointStart,
	eRpsPoint_L2_Close,
	eRpsPoint_L1_Amount,
	eRpsPoint_L2_Amount,
	eSecPoint_L1_Amount,
	eSecPoint_L2_Amount,
	eIndyMarketPointEnd,
	ePointEnd = eIndyMarketPointEnd,
};

enum eL1IndyPointDataType
{
	eL1IndyPointStart = 100,
	eL1Indy_RpsPoint_Close = eL1IndyPointStart,
	eL1Indy_RpsPoint_Amount,
	eL1Indy_SecPoint_Amount,
	eL1IndyPointEnd,
};

enum eL2IndyPointDataType
{
	eL2IndyPointStart = 200,
	eL2Indy_RpsPoint_Close = eL2IndyPointStart,
	eL2Indy_RpsPoint_Amount,
	eL2Indy_SecPoint_Amount,
	eL2IndyPointEnd,
};

enum eCAPointDataType
{
	eCAPointStart = 300,
	eCAFullMarketPointStart = eCAPointStart,
	eCAPoint_Volume = eCAFullMarketPointStart,
	eCAPoint_Amount,
	eCAPointFullMarketEnd,
	eCAIndyMarketPointStart = eCAPointFullMarketEnd,
	eCAPoint_L1_Volume = eCAIndyMarketPointStart,
	eCAPoint_L2_Volume,
	eCAPoint_L1_Amount,
	eCAPoint_L2_Amount,
	eCAIndyMarketPointEnd,
	eCAPointEnd = eCAIndyMarketPointEnd,
};


typedef struct _ExDataDetInfo :public DataGetInfo
{
	int nAskGroup;	//请求的窗口周期
	ePointType Type;
	char* exMsg;
}ExDataGetInfo;


typedef struct InitPara
{
	bool bShowMA;
	bool bShowBandTarget;
	bool bShowAverage;
	bool bShowEMA;
	bool bShowTSCMACD;
	bool bShowTSCVolume;
	bool bShowKlineVolume;
	bool bShowKlineAmount;
	bool bShowKlineCAVol;
	bool bShowKlineCAAmo;
	bool bShowKlineMACD;
	bool bShowTSCRPS[3];
	bool bShowKlineRPS[3];
	int  nWidth;
	bool bShowTSCDeal;
	bool bShowKlineDeal;
	int  nEMAPara[2];
	int  nMACDPara[3];
	int	 nMAPara[MAX_MA_COUNT];
	int	 nVolMaPara[MAX_MA_COUNT];
	int  nAmoMaPara[MAX_MA_COUNT];
	int	 nCAVolMaPara[MAX_MA_COUNT];
	int	 nCAAmoMaPara[MAX_MA_COUNT];
	int	 nJiange;
	BandPara_t  BandPara;
	int Period;
	bool Connect1;
	bool Connect2;
	char ShowIndy[8];
	bool UseStockFilter;
	bool ListShowST;
	bool ListShowSBM;
	bool ListShowSTARM;
	bool ListShowNewStock;
	int nTSCPointWndNum;
	int nKlinePointWndNum;
	int nKlineRehabType;		//用于显示的复权类型
	int nKlineCalcRehabType;	//用于内部计算的复权类型
	int nKlineFTRehabDate;
	bool UseHisStockFilter;
	bool bKlineUseTickFlowData;
	int	 nKlineTickFlowDataType;
	vector<ShowPointInfo> TSCPonitWndInfo;
	vector<ShowPointInfo> KlinePonitWndInfo;
	SStringA strFilterName;

	InitPara() :bShowMA(true), bShowBandTarget(false),
		bShowAverage(true), bShowEMA(true),
		bShowTSCMACD(true), bShowTSCVolume(false),
		bShowKlineVolume(false), bShowKlineAmount(false),
		bShowKlineCAVol(false), bShowKlineCAAmo(false),
		bShowTSCRPS{ false,false,false },
		bShowKlineRPS{ false ,false,false }, nWidth(9),
		bShowKlineMACD(true), bShowTSCDeal(true), bShowKlineDeal(false),
		nEMAPara{ 12,26 }, nMACDPara{ 12,26,9 },
		nMAPara{ 5,10,20,60 }, nJiange(2), Period(Period_1Day),
		Connect1(false), Connect2(false), ShowIndy(""), UseStockFilter(false),
		ListShowST(true), ListShowSBM(true),
		ListShowSTARM(true), ListShowNewStock(true), nKlineRehabType(0),
		nKlineCalcRehabType(0), nKlineFTRehabDate(0),UseHisStockFilter(false), 
		bKlineUseTickFlowData(false), nKlineTickFlowDataType(0),strFilterName("")
	{}
}InitPara_t;

enum eSortDataType
{
	eSDT_Double = 0,
	eSDT_Int,
	eSDT_String,
	eSDT_BigDouble,
	eSDT_Uint64,
};

typedef struct _StockFilterPara
{
	bool operator <(const _StockFilterPara& other) const;
	bool operator ==(const _StockFilterPara& other) const;
	double num;
	int index1;
	int period1;
	int condition;
	int index2;
	int period2;
}StockFilter;

typedef struct _HisStockFilterPara
{
	bool operator <(const _HisStockFilterPara& other) const;

	StockFilter sf;
	int countDay;
	int type;
}HisStockFilter;

enum eJudgeType
{
	eJT_Exist = 0,
	eJT_Forall,
};

enum eMaType
{
	eMa_Close = 0,
	eMa_Volume,
	eMa_Amount,
	eMa_CAVol,
	eMa_CAAmo,
};

typedef struct _sReHab
{
	int exDivDate;
	double perCashDiv;			//每股现金分红
	double perShareDivRatio;	//每股送股
	double perShareTransRatio;	//每股转增股
	double allotmentRatio;		//每股配股比例
	double allotmentPrice;		//配股价
	double adjFactor;			//后复权因子
	double accumAdjFactor;		//累计后复权因子
}RehabInfo;

enum eRehabType
{
	eRT_NoRehab = 0,
	eRT_FrontRehab_Cash,
	eRT_BackRehab_Cash,
	eRT_Rehab_Cash_FixedTime,
	eRT_FrontRehab_ReInv,
	eRT_BackRehab_ReInv,
	eRT_Rehab_ReInv_FixedTime,

};

enum RehabMenu
{
	RM_NoRehab = 500,
	RM_FrontRehab_Cash,
	RM_BackRehab_Cash,
	RM_Rehab_Cash_FixedTime,
	RM_FrontRehab_ReInv,
	RM_BackRehab_ReInv,
	RM_Rehab_ReInv_FixedTime,
	RM_End,

};

enum FilterWndMenu
{
	FWM_Start = 600,
	FWM_WndNum1 = FWM_Start,
	FWM_WndNum2,
	FWM_WndNum3,
	FWM_WndNum4,
	FWM_WndNum5,
	FWM_WndNum6,
	FWM_End,
};

typedef struct _sFixedTimeRehab
{
	int nDate;
	BOOL bFrontRehab;
	eRehabType Type;
}FixedTimeRehab;


typedef struct _RtPointData
{
	char dataName[MAX_NAME_LENGTH];
	CoreData data;
	char stockID[8];
	int period;
}RtPointData;

typedef struct _CAInfo
{
	SecurityID SecurityID;
	int date;
	int group;
	double Volume;
	double VolPoint;
	double VolPointL1;
	double VolPointL2;
	double Amount;
	double AmoPoint;
	double AmoPointL1;
	double AmoPointL2;
	int VolRank;
	int VolRankL1;
	int VolRankL2;
	int AmoRank;
	int AmoRankL1;
	int AmoRankL2;
}CAInfo;

enum _volAmoType
{
	VAT_Volume,
	VAT_Amount,
	VAT_CAVol,
	VAT_CAAmo,
};

enum _WndType
{
	WT_SubWindow,
	WT_FilterWindow,
	WT_SetFliterName,
};

enum SortDirect
{
	SD_Greater = 0,
	SD_Less,
};

enum eRpsData
{
	eRps_RPS520 = 0,
	eRps_MACD520,
	eRps_POINT520,
	eRps_RANK520,
	eRps_RPS2060,
	eRps_MACD2060,
	eRps_POINT2060,
	eRps_RANK2060,
	eRps_POINT520L1,
	eRps_RANK520L1,
	eRps_POINT2060L1,
	eRps_RANK2060L1,
	eRps_RANK520L2,
	eRps_RANK2060L2,
	eRps_POINT520L2,
	eRps_POINT2060L2,
	eRps_DataCount,
};

enum eSecData
{
	eSec_POINT,
	eSec_RANK,
	eSec_POINTL1,
	eSec_RANKL1,
	eSec_POINTL2,
	eSec_RANKL2,
	eSec_DataCount,

};

enum eComboFilterMsg
{
	CFMsg_FinishHis = 0,
};

void GetInitPara(CIniFile& ini, InitPara& para, SStringA strSection);
void SaveInitPara(CIniFile& ini, InitPara& para, SStringA strSection);

typedef struct _TickFlowBaseMarket
{
	SecurityID SecurityID;
	int nDate;
	int nTime;						//时间
	int nPeriod;
	double fPrice;
	double fPOC;						//POC价格
	uint64_t ActBuyVol;
	uint64_t ActSellVol;
	uint32_t uActBuyOrderCount;	//股票主动买入订单数 或 期货多开量
	uint32_t uActSellOrderCount;	//股票主动卖出订单数 或 期货空开量
	uint32_t uPasBuyOrderCount;	//股票被动买入订单数 或 期货空平量
	uint32_t uPasSellOrderCount;	//股票被动卖出订单数 或 期货多平量

}TFBaseMarket;

enum eDataFigureType
{
	eDFT_SolidLine=0,
	eDFT_DotLine,
	eDFT_Rect,
};

enum eFigureColorType
{
	eFCT_Appoint,//指定颜色
	eFCT_Default,//默认颜色
	eFCT_PriceJudge,//根据价格判定
};

enum eTFDataType
{
	eTFDT_Ratio,
	eTFDT_Vol,
	eTFDT_Order,
	eTFDT_AvgVol,
};

typedef struct _TradeDouble
{
	_TradeDouble();
	_TradeDouble(long long llData, long long llDigital);
	long long data;
	long long digital;
	double GetDouble() const;
	_TradeDouble operator +(const _TradeDouble& other);
	_TradeDouble operator -(const _TradeDouble& other);
	_TradeDouble operator *(const _TradeDouble& other);
	_TradeDouble operator /(const _TradeDouble& other);

}TradeDouble;

typedef struct _tradeAccInfo
{
	char ID[16];
	TradeDouble totalCaptical;
	TradeDouble marketValue;
	TradeDouble marketCost;
	TradeDouble usableCaptical;
	TradeDouble freezedCaptical;
	int AccountID;
}AccInfo;


typedef struct _PositionInfo
{
	char SecurityID[8];
	char SecurityName[16];
	TradeDouble YestdayPos;
	TradeDouble TotalPos;
	TradeDouble UsablePos;
	TradeDouble Cost;
	TradeDouble InTransitPos;
	TradeDouble FreezedPos;
	int AccountID;
	char Account[16];
}PositionInfo;

typedef struct _TrustInfo
{
	char SecurityID[8];
	char SecurityName[16];
	int Date;
	int Time;
	int TrustID;
	int ApplyID;
	int Direct;
	int State;
	TradeDouble TrustVol;
	TradeDouble TrustPrice;
	TradeDouble DealVol;
	TradeDouble DealPrice;
	TradeDouble DealAmo;
	TradeDouble CancelVol;
	int CancelSign;
	int AccountID;
	char Account[16];
}TrustInfo;

typedef struct _DealInfo
{
	char SecurityID[8];
	char SecurityName[16];
	int Date;
	int Time;
	int TrustID;
	int ApplyID;
	int Direct;
	int State;
	TradeDouble DealVol;
	TradeDouble DealPrice;
	TradeDouble DealAmo;
	TradeDouble SettleAmo;
	int AccountID;
	char Account[16];
}DealInfo;

enum SPostionHead
{
	SPH_SecurityID=0,
	SPH_SecurityName,
	SPH_YesterdayPos,
	SPH_TotalPos,
	SPH_UsablePos,
	SPH_CostPrice,
	SPH_LastPrice,
	SPH_Cost,
	SPH_MarketValue,
	SPH_FloatingPL,
	SPH_FloatingPLPct,
	SPH_FreezedPos,
	SPH_InTransitPos,
	SPH_Account,
	SPH_Count,
};

enum STrustHead
{
	STH_TrustDate,
	STH_TrustTime,
	STH_ApplyID,
	STH_SecurityID,
	STH_SecurityName,
	STH_Direct,
	STH_TrustState,
	STH_TrustVol,
	STH_TrustPrice,
	STH_DealVol,
	STH_DealPrice,
	STH_DealAmo,
	STH_CancelVol,
	STH_CancelSign,
	STH_TrustID,
	STH_Account,
	STH_Count,
};

enum SCancelHead
{
	SCH_TrustTime,
	SCH_ApplyID,
	SCH_SecurityID,
	SCH_SecurityName,
	SCH_Direct,
	SCH_TrustState,
	SCH_TrustPrice,
	SCH_TrustVol,
	SCH_DealPrice,
	SCH_DealVol,
	SCH_DealAmo,
	SCH_CancellableVol,
	SCH_CancelSign,
	SCH_TrustID,
	SCH_Account,
	SCH_Count,
};

enum SDealHead
{
	SDH_DealTime,
	SDH_ApplyID,
	SDH_SecurityID,
	SDH_SecurityName,
	SDH_Direct,
	SDH_DealPrice,
	SDH_DealVol,
	SDH_DealAmo,
	SDH_TrustID,
	SDH_Account,
	SDH_Count,
};

enum SDealSumHead
{
	SDSH_SecurityID,
	SDSH_SecurityName,
	SDSH_Direct,
	SDSH_DealVol,
	SDSH_DealAvgPrice,
	SDSH_DealAmo,
	SDSH_Account,
	SDSH_Count,
};


enum SHisTrustHead
{
	SHTH_TrustDate,
	SHTH_TrustTime,
	SHTH_ApplyID,
	SHTH_SecurityID,
	SHTH_SecurityName,
	SHTH_Direct,
	SHTH_TrustState,
	SHTH_TrustPrice,
	SHTH_TrustVol,
	SHTH_DealPrice,
	SHTH_DealVol,
	SHTH_DealAmo,
	SHTH_CancelVol,
	SHTH_CancelSign,
	SHTH_TrustID,
	SHTH_Account,
	SHTH_Count,
};

enum SHisDealHead
{
	SHDH_DealDate,
	SHDH_DealTime,
	SHDH_ApplyID,
	SHDH_SecurityID,
	SHDH_SecurityName,
	SHDH_Direct,
	SHDH_DealPrice,
	SHDH_DealVol,
	SHDH_DealAmo,
	SHDH_SettleAmp,
	SHDH_TrustID,
	SHDH_Account,
	SHDH_Count,
};

enum eTradeDircect
{
	eTD_Buy = 0,
	eTD_Sell,
	eTD_CancelBuy,
	eTD_CancelSell,
};

enum eTrustState
{
	eTS_NotSubmit = 0,
	eTS_Submitted,
	eTS_DealPart,
	eTS_DealAll,
	eTS_Canceled,
	eTS_Discarded,
};

enum eCancelSign
{
	eCS_Normal,
	eCS_Canceled,
};

enum TradeSimulatorMsg
{
	TSMsg_SetTradeStock,
	TSMsg_UpdateBuyTradeInfo,
	TSMsg_UpdateSellTradeInfo,
	TSMsg_UpdateAccountInfo,
	TSMsg_UpdatePosInfo,
	TSMsg_UpdateTrustAndCancelInfo,
	TSMsg_UpdateDealInfo,
	TSMsg_UpdateHisTrust,
	TSMsg_UpdateHisDeal,
	TSMsg_UpdateSubmitFeedback,
	TSMsg_ShowWindow,
	TSMsg_ChangeSetting,
	TSMsg_ShowDeal,

};

enum TradeAccountMsg
{
	TAMsg_Register = 0,
	TAMsg_ChangePsd,
};

enum AccountFeedBack
{
	AFB_RegisterSuccess=0,
	AFB_AccountExist,
	AFB_ChangePsdSuccess,
	AFB_NotPair,
};

enum TradeLoginFeedBack
{
	TLFB_Success,
	TLFB_Unsuccess,
};


enum eAccountOpt
{
	eAO_Register,
	eAO_ChangePsd,
};

enum TradeSendMsgType
{
	TradeSendMsg_Start = 200,
	TradeSendMsg_Register,
	TradeSendMsg_ChangePsd,
	TradeSendMsg_Login,
	TradeSendMsg_Logout,
	TradeSendMsg_QueryAccountInfo,
	TradeSendMsg_QueryPostion,
	TradeSendMsg_QueryHisTrust,
	TradeSendMsg_QueryHisDeal,
	TradeSendMsg_SubmitTrade,
	TradeSendMsg_End,
};

enum TradeReceiveMsgType
{
	TradeRecvMsg_Register = 190000,
	TradeRecvMsg_ChangePsd,
	TradeRecvMsg_Login,
	TradeRecvMsg_Logout,
	TradeRecvMsg_AccountInfo,
	TradeRecvMsg_Position,
	TradeRecvMsg_Trust,
	TradeRecvMsg_Deal,
	TradeRecvMsg_HisTrust,
	TradeRecvMsg_HisDeal,
	TradeRecvMsg_SubmitFeedback,

};


enum TradeSynMsg
{
	TradeSyn_Register = 30000,
	TradeSyn_ChangePsd,
	TradeSyn_Login,
	TradeSyn_Logout,
	TradeSyn_QueryAccountInfo,
	TradeSyn_QueryPosition,
	TradeSyn_QueryHisTrust,
	TradeSyn_QueryHisDeal,
	TradeSyn_SubmitTrade,

	TradeSyn_OnRegister,
	TradeSyn_OnChangePsd,
	TradeSyn_OnLogin,
	TradeSyn_OnLogout,
	TradeSyn_OnAccountInfo,
	TradeSyn_OnPosition,
	TradeSyn_OnTrust,
	TradeSyn_OnDeal,
	TradeSyn_OnHisTrust,
	TradeSyn_OnHisDeal,
	TradeSyn_OnSubmitFeedback,
};

enum TradeMsg
{
	TradeMsg_Login,
	TradeMsg_Logout,
	TradeMsg_AccountInfo,
	TradeMsg_Position,
	TradeMsg_Trust,
	TradeMsg_Deal,
	TradeMsg_HisTrust,
	TradeMsg_HisDeal,
};

typedef struct _SubmitFeedback
{
	char Account[16];
	int TrustID;
	int Feedback;
	int ApplyID;
}SubmitFeedback;

enum eSubmitFeedback
{
	eSF_SubmitSucc,
	eSF_SubmitFail,
	eSF_SubmitTimeErr,
	eSF_InvalidPrice,
	eSF_CancelSucc,
	eSF_CancelFail,
};

typedef struct _tradeSetting
{
	double buyAmo;
	int buyPriceType;
	int buyVolType;
	int sellPriceType;
	int sellVolType;
	int maxVol;
	int buyVolFix;
	int afterTradeType;
	int windowTime;
	bool remindLimitPrice;
	bool remindCagePrice;
	bool clickVol;
	bool showDeal;
	bool cancelConfirm;
	bool trustConfirm;
	bool showTrust;
	bool changePageClean;
	_tradeSetting() :buyAmo(0),buyPriceType(0),buyVolType(0),
		sellPriceType(0), sellVolType(0), maxVol(1'000'000),
		buyVolFix(0), afterTradeType(0), windowTime(5),
		remindLimitPrice(1), remindCagePrice(1), clickVol(false),
		showDeal(false), cancelConfirm(false), trustConfirm(true),
		showTrust(true), changePageClean(false)
	{

	}
}TradeSetting;

enum eBuyPriceType
{
	eBPT_Ask1 = 0,
	eBPT_Ask2,
	eBPT_Ask3,
	eBPT_Ask4,
	eBPT_Ask5,
	eBPT_LastPrice,
	eBPT_Empty,
};

enum eBuyVolType
{
	eBVT_Empty = 0,
	eBVT_All,
	eBVT_Half,
	eBVT_OneThird,
	eBVT_Quarter,
	eBVT_OneFifth,
	eBVT_OneSixth,
};

enum eSellPriceType
{
	eSPT_Bid1 = 0,
	eSPT_Bid2,
	eSPT_Bid3,
	eSPT_Bid4,
	eSPT_Bid5,
	eSPT_LastPrice,
	eSPT_Empty,
};

enum eSellVolType
{
	eSVT_Empty = 0,
	eSVT_All,
	eSVT_Half,
	eSVT_OneThird,
	eSVT_Quarter,
	eSVT_OneFifth,
	eSVT_OneSixth,
};

enum eAfterTradeType
{
	eATT_Clear,
	eATT_Id,
	eATT_IdPrice,
	eATT_IdPriceVol,
};
