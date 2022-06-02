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
	unsigned long nDataSize1;		//snap����ʱ stockIndex�����ݴ�С;kline����ʱ ѹ�������ݴ�С
	unsigned long nDataSize2;		//snap����ʱ futures�����ݴ�С;kline����ʱ ԭʼ���ݴ�С
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
	unsigned long NoUse3;		//snap����ʱ stockIndex�����ݴ�С;kline����ʱ ѹ�������ݴ�С
	unsigned long NoUse4;		//snap����ʱ futures�����ݴ�С;kline����ʱ ԭʼ���ݴ�С
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


//ͨ�õ��������ݽṹ
typedef struct CommonStockMarket
{
	unsigned long long UpdateTime; // ����ʱ�� ��������ʱֻ������û��ʱ��
	SecurityID SecurityID; //֤ȯ����
	int  State;//���׽׶δ���
	double 	PreCloPrice; //�����̼�
	long long	TradNum; //�ɽ�����
	long long	Volume; //�ɽ�����
	double 	Turnover; //�ɽ��ܽ��
	double	LastPrice; //���¼�
	double	OpenPrice; //���̼�
	double	HighPrice; //��߼�
	double	LowPrice; //��ͼ�
	long long TotalOfferQty; //ί��������������Ч���۷�Χ�ڣ�
	double  WeightedAvgOfferPx; //��Ȩƽ�������۸�
	long long  TotalBidQty; //ί����������
	double  WeightedAvgBidPx; //��Ȩƽ������۸�
	double	AskPrice[10]; //��һ��
	long long	AskVolume[10]; //��һ��
	double	BidPrice[10]; //��һ��
	long long	BidVolume[10]; //��һ��
	int	NumOrdersB[10]; //��һί�б���
	int	NumOrdersS[10]; //��һί�б���
};

typedef struct CommonStockTick
{
	SecurityID SecurityID;	//֤ȯ����
	unsigned long long TradTime;		//�ɽ�ʱ��
	double TradPrice;		//�ɽ��۸�
	long long TradVolume;	//�ɽ�����
	double TradeMoney;		//�ɽ����
	long long TradeBuyNo;		//�򷽵���
	long long TradeSellNo;	//��������
};

typedef struct CommonStockOrder
{
	SecurityID SecurityID; //֤ȯ����
	long long OrderTime;	//ί��ʱ��
	long long OrderNo;	//������
	double OrderPrice;	//ί�м۸�
	long long OrderVolume;	//ί����
	int Side;	//����
};

typedef struct CommonIndexMarket
{
	int TradingDay;
	int UpdateTime;
	SecurityID SecurityID;
	double	PreCloPrice;	//�����̼�
	double	LastPrice; //���¼�
	double	OpenPrice; //���̼�
	double	HighPrice; //��߼�
	double	LowPrice; //��ͼ�
	long long Volume;
	double Turnover;
	double Amplitude;	//���
	double Change;
	double ChangePct;
	double VolumeRatio;	//����
};


typedef struct KlineType
{
	double open;	//��
	double high;	//��
	double low;		//��
	double close;	//��
	double vol;		//��
	int date;		//����
	int time;		//ʱ��
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
	KlineType	pd[MAX_DATA_COUNT];	//data��Ϣ
	double			fMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	bool			bShow;				//�Ƿ���ʾ

	double			fMax;
	double			fMin;
	int				nDivY;				//y����ʾϵ��1
	int				nMulY;				//y����ʾϵ��2
	int				nDecimal;			//С��λ��
	TCHAR			sDecimal[10];		//������ʽ��
	int				nDecimalXi;			//10��m_nDecimal�η�
	int				nLast1MinTime;		//���¸������õ�1MinK�ߵ�ʱ��
	int				nLastVolume;		//���ʹ��K�߸���ʱ�ۼƵ������ڳɽ���
	int				nLastTradingDay;	//���ʹ��K�߸��µĽ�����
}KLINE_INFO;

typedef struct _FUTU_INFO {
	int			ftl[MAX_DATA_COUNT];	//��ͼ��
	COLORREF		clft;					//

	double			fMax;
	double			fMin;
	int				nDivY;				//y����ʾϵ��1
	int				nMulY;				//y����ʾϵ��2
	int				nDecimal;			//С��λ��
	TCHAR			sDecimal[10];		//������ʽ��
	int				nDecimalXi;			//10��m_nDecimal�η�
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
	std::vector<FENSHI_GROUP> d;	//���̼۸�

	double			fMax;
	double			fMin;
	//	int				nDivY;				//y����ʾϵ��1
	//	int				nMulY;				//y����ʾϵ��2
	//	int				nDecimal;			//С��λ��
	//TCHAR			sDecimal[10];		//������ʽ��
	//									//	int				nDecimalXi;			//10��m_nDecimal�η�

	double			fMaxf;
	double			fMinf;
	double			fMaxMACD;
	double			fMinMACD;
	double			dDelta;
	//int				nDivYf;				//y����ʾϵ��1
	//int				nMulYf;				//y����ʾϵ��2
	//int				nRatio;				//�۸�ϵ��
	//int				nRatio2;			//��Լ2�۸�ϵ��
	double			fPreClose;			//ǰ��

	int				nLastVolume;		//��һ��ʱ����������
	int				nCount;				//�Ѿ������tick������
	int				nAllLineNum;		//��ʱͼ���ݵ�����

	//int				nInsType;			//�ڻ����� 0--��Ʒ��ҹ�� 1--��Ʒ��ҹ��23:00 2--��Ʒ��ҹ��1:00  3--��Ʒ��ҹ��2:30  4--��ָ  5--��ծ

	int				nMin;				//����������һ������
	int				nTime;				//����������һ��ʱ��

	//int				nGroupIns1Ratio;	//��Լ1ϵ��
	//int				nGroupIns2Ratio;	//��Լ2ϵ��
}FENSHI_INFO;

typedef struct _AllKPIC_INFO {
	KlineType		data[MAX_DATA_COUNT];	//data��Ϣ
	double			fMa[MAX_MA_COUNT][MAX_DATA_COUNT];
	double			fMax;
	double			fMin;
	double			fSubMax;
	double			fSubMin;
	int				nLast1MinTime;		//���¸������õ�1MinK�ߵ�ʱ��
	int				nLastVolume;		//���ʹ��K�߸���ʱ�ۼƵ������ڳɽ���
	int				nLastTradingDay;	//���ʹ��K�߸��µĽ�����
	int				nTotal;
	int		nKwidth;		//k�߿��
	int		nMoveNow;		//У�����move,��ֹmove���ޱ��
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
	int			id;				//֤ȯ�ڲ����
	int			date;			//����(20170101)
	int			time;			//ʱ��(93501500)		
	double		newp;			//���¼�
	int			vol;			//������
	int			totalvol;		//�ɽ�������
	double		abp[10];		//��5��5��

	_PARAM_TICK_INFO() :id(0), date(0),
		time(0), newp(0), vol(0), totalvol(0)
	{
		ZeroMemory(abp, sizeof(int) * 10);
	}

	int Compare(const _PARAM_TICK_INFO &x)	//1����,0����,-1С��
	{
		return this->Compare(x.date, x.time);
	}

	int Compare(int date, int time)	//1����,0����,-1С��
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
