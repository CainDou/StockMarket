#include "stdafx.h"
#include "WndSynHandler.h"
#include "FrmlManager.h"
#include "IniFile.h"
#include "zlib.h"
#include <io.h>
#include "MD5.h"

HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hLoginEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#define SHOWDATACOUNT 2
#define TIMER_AUTOSAVE 1
#define MAX_TICK 6000
#define MAX_SUBPIC 3

#define MSG_SUCC 0x01010101
CWndSynHandler::CWndSynHandler()
{
	todayDataBuffer = nullptr;
	todayDataSize = 0;
	bExit = false;
	m_bFirstData = true;
	m_bCaUpdate = false;
}


CWndSynHandler::~CWndSynHandler()
{
	if (m_pLoginDlg)
		::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
			NULL, LoginMsg_Exit);
	m_NetClinet.Stop();
	SendMsg(m_RpsProcThreadID, Msg_Exit, NULL, 0);
	SendMsg(m_uMsgThreadID, Msg_Exit, NULL, 0);
	if (tRpsCalc.joinable())
		tRpsCalc.join();
	if (tMsgSyn.joinable())
		tMsgSyn.join();
	if (tLogin.joinable())
		tLogin.join();

}

void CWndSynHandler::Run()
{
	InitializeCriticalSection(&m_cs);
	InitializeCriticalSection(&m_csFilterData);

	InitCommonSetting();
	InitPointInfo();
	InitNetConfig();
	SetVectorSize();
	InitDataHandleMap();
	InitNetHandleMap();
	InitSynHandleMap();
	CFrmlManager::InitFrmlManage();
	tRpsCalc = thread(&CWndSynHandler::DataProc, this);
	m_RpsProcThreadID = *(unsigned*)&tRpsCalc.get_id();
	tMsgSyn = thread(&CWndSynHandler::MsgProc, this);
	m_uMsgThreadID = *(unsigned*)&tMsgSyn.get_id();
	m_NetClinet.SetWndHandle(m_hMain);
	if (!CheckCmdLine())
		exit(0);
	tLogin = thread(&CWndSynHandler::Login, this);
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	if (bExit)	exit(0);
	//ResetEvent(g_hLoginEvent);
	m_NetHandleFlag.clear();
	m_NetClinet.RegisterHandle(NetHandle);
	m_NetClinet.Start(m_uNetThreadID, this);
	SendInfo info;
	info.MsgType = ComSend_Connect;
	strcpy(info.str, "StkMarket");
	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	WaitForSingleObject(g_hEvent, INFINITE);
	while (!m_bServerReady)
	{
		m_bServerReady = true;
		::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
			NULL, LoginMsg_WaitAndTry);
		WaitForSingleObject(g_hLoginEvent, INFINITE);
		if (bExit)	exit(0);
		//ResetEvent(g_hLoginEvent);
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	while (!CheckInfoRecv())
	{
		m_NetHandleFlag.clear();
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	SStringW Info = L"登陆成功,开始程序初始化";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);

	//Info = L"处理当日历史数据，请等待...";
	//::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
	//	(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);
	//WaitForSingleObject(g_hLoginEvent, INFINITE);
	if (bExit)	exit(0);

	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_HideWnd);
}

void CWndSynHandler::Close()
{
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		NULL, LoginMsg_Exit);
}

void CWndSynHandler::InitCommonSetting()
{
	m_PeriodVec.resize(7);
	m_PeriodVec[0] = 0;
	m_PeriodVec[1] = 1;
	m_PeriodVec[2] = 5;
	m_PeriodVec[3] = 15;
	m_PeriodVec[4] = 30;
	m_PeriodVec[5] = 60;
	m_PeriodVec[6] = 1440;
	m_bServerReady = true;

	m_PointGetMsg[eRpsPoint] = SendType_GetHisRpsPoint;
	m_PointGetMsg[eSecPoint] = SendType_GetHisSecPoint;

	//m_comDataNameVec.emplace_back("close");
	//m_comDataNameVec.emplace_back("RPS520");
	//m_comDataNameVec.emplace_back("MACD520");
	//m_uniDataNameVec.emplace_back("Point520");
	//m_uniDataNameVec.emplace_back("Rank520");
	//m_comDataNameVec.emplace_back("RPS2060");
	//m_comDataNameVec.emplace_back("MACD2060");
	//m_uniDataNameVec.emplace_back("Point2060");
	//m_uniDataNameVec.emplace_back("Rank2060");

}

void CWndSynHandler::InitNetConfig()
{
	CIniFile ini(".//config//NetConfig.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);

}

void CWndSynHandler::InitPointInfo()
{
	ShowPointInfo spi;
	spi.IndyRange = "";
	spi.type = eRpsPoint;
	spi.dataInRange = "";
	spi.srcDataName = "close";
	spi.showName = "CRPS";
	spi.overallType = eRpsPoint_Close;
	m_pointInfoMap[eRpsPoint_Close] = spi;
	spi.dataInRange = "L1";
	spi.showName = "CRPS(一级行业中)";
	spi.overallType = eRpsPoint_L1_Close;
	m_pointInfoMap[eRpsPoint_L1_Close] = spi;
	spi.dataInRange = "L2";
	spi.showName = "CRPS(二级行业中)";
	spi.overallType = eRpsPoint_L2_Close;
	m_pointInfoMap[eRpsPoint_L2_Close] = spi;

	spi.srcDataName = "amount";
	spi.dataInRange = "";
	spi.IndyRange = "";
	spi.showName = "AMORPS";
	spi.overallType = eRpsPoint_Amount;
	m_pointInfoMap[eRpsPoint_Amount] = spi;
	spi.dataInRange = "L1";
	spi.showName = "AMORPS(一级行业中)";
	spi.overallType = eRpsPoint_L1_Amount;
	m_pointInfoMap[eRpsPoint_L1_Amount] = spi;
	spi.dataInRange = "L2";
	spi.showName = "AMORPS(二级行业中)";
	spi.overallType = eRpsPoint_L2_Amount;
	m_pointInfoMap[eRpsPoint_L2_Amount] = spi;

	spi.type = eSecPoint;
	spi.dataInRange = "";
	spi.IndyRange = "";
	spi.showName = "AMO截面分数";
	spi.overallType = eSecPoint_Amount;
	m_pointInfoMap[eSecPoint_Amount] = spi;
	spi.dataInRange = "L1";
	spi.overallType = eSecPoint_L1_Amount;
	spi.showName = "AMO截面分数(一级行业中)";
	m_pointInfoMap[eSecPoint_L1_Amount] = spi;
	spi.dataInRange = "L2";
	spi.overallType = eSecPoint_L2_Amount;
	spi.showName = "AMO截面分数(二级行业中)";
	m_pointInfoMap[eSecPoint_L2_Amount] = spi;

	//对应1级行业指标
	spi.type = eRpsPoint;
	spi.dataInRange = "";
	spi.IndyRange = "L1";
	spi.srcDataName = "close";
	spi.showName = "所属一级行业CRPS";
	spi.overallType = eL1Indy_RpsPoint_Close;
	m_pointInfoMap[eL1Indy_RpsPoint_Close] = spi;
	spi.srcDataName = "amount";
	spi.showName = "所属一级行业AMORPS";
	spi.overallType = eL1Indy_RpsPoint_Amount;
	m_pointInfoMap[eL1Indy_RpsPoint_Amount] = spi;
	spi.type = eSecPoint;
	spi.showName = "所属一级行业AMO截面分数";
	spi.overallType = eL1Indy_SecPoint_Amount;
	m_pointInfoMap[eL1Indy_SecPoint_Amount] = spi;


	spi.type = eRpsPoint;
	spi.dataInRange = "";
	spi.IndyRange = "L2";
	spi.srcDataName = "close";
	spi.showName = "所属二级行业CRPS";
	spi.overallType = eL2Indy_RpsPoint_Close;
	m_pointInfoMap[eL2Indy_RpsPoint_Close] = spi;
	spi.srcDataName = "amount";
	spi.showName = "所属二级行业AMORPS";
	spi.overallType = eL2Indy_RpsPoint_Amount;
	m_pointInfoMap[eL2Indy_RpsPoint_Amount] = spi;
	spi.type = eSecPoint;
	spi.showName = "所属二级行业AMO截面分数";
	spi.overallType = eL2Indy_SecPoint_Amount;
	m_pointInfoMap[eL2Indy_SecPoint_Amount] = spi;


	spi.IndyRange = "";
	spi.type = eSecPoint;
	spi.dataInRange = "";
	spi.srcDataName = "CaVol";
	spi.showName = "集竞VRPS";
	spi.overallType = eCAPoint_Volume;
	m_pointInfoMap[eCAPoint_Volume] = spi;
	spi.dataInRange = "L1";
	spi.showName = "集竞VRPS(一级行业中)";
	spi.overallType = eCAPoint_L1_Volume;
	m_pointInfoMap[eCAPoint_L1_Volume] = spi;
	spi.dataInRange = "L2";
	spi.showName = "集竞VRPS(二级行业中)";
	spi.overallType = eCAPoint_L2_Volume;
	m_pointInfoMap[eCAPoint_L2_Volume] = spi;

	spi.srcDataName = "CaAmo";
	spi.dataInRange = "";
	spi.IndyRange = "";
	spi.showName = "集竞AMORPS";
	spi.overallType = eCAPoint_Amount;
	m_pointInfoMap[eCAPoint_Amount] = spi;
	spi.dataInRange = "L1";
	spi.showName = "集竞AMORPS(一级行业中)";
	spi.overallType = eCAPoint_L1_Amount;
	m_pointInfoMap[eCAPoint_L1_Amount] = spi;
	spi.dataInRange = "L2";
	spi.showName = "集竞AMORPS(二级行业中)";
	spi.overallType = eCAPoint_L2_Amount;
	m_pointInfoMap[eCAPoint_L2_Amount] = spi;

}

bool CWndSynHandler::ReceiveData(SOCKET socket, int size, char end,
	char * buffer, int offset)
{
	char*p = buffer + offset;
	StockInfo stkInfo = { 0 };
	int sizeLeft = size;
	while (sizeLeft > 0)
	{
		int ret = recv(socket, p, sizeLeft, 0);
		if (SOCKET_ERROR == ret)
		{
			delete[] buffer;
			buffer = nullptr;
			p = nullptr;
			return 0;
		}
		sizeLeft -= ret;
		p += ret;
	}
	p = nullptr;
	char cEnd;
	int ret = recv(socket, &cEnd, 1, 0);
	if (cEnd == end)
		return true;
	return false;
}

unsigned CWndSynHandler::NetHandle(void * para)
{
	CWndSynHandler *pMd = (CWndSynHandler*)para;
	int nOffset = 0;
	ReceiveInfo recvInfo;
	BOOL bNeedConnect = false;
	//int c = 0;
	while (true)
	{
		if (pMd->m_NetClinet.GetExitState())
			return 0;
		if (pMd->RecvInfoHandle(bNeedConnect, nOffset, recvInfo))
		{
			auto pFuc = pMd->m_netHandleMap[recvInfo.MsgType];
			if (pFuc == nullptr)
				pFuc = &CWndSynHandler::OnNoDefineMsg;
			(pMd->*pFuc)(pMd->m_NetClinet.GetSocket(), recvInfo);
		}

	}
	return 0;
}

void CWndSynHandler::Login()
{
	m_pLoginDlg = new CDlgLogin(m_hMain, &m_NetClinet);
	m_pLoginDlg->SetIPInfo(m_strIPAddr, m_nIPPort);
	m_pLoginDlg->DoModal();
	bExit = true;
	SetEvent(g_hLoginEvent);
	//delete m_pLoginDlg;
	//m_pLoginDlg = nullptr;
}


void CWndSynHandler::DataProc()
{
	int MsgId;
	char *info;
	int msgLength;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			break;
		}
		auto pFuc = m_dataHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);;
		delete[]info;
		info = nullptr;
	}

}


void CWndSynHandler::MsgProc()
{
	int MsgId;
	char *info;
	int msgLength;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			break;
		}
		auto pFuc = m_synHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}
}


bool CWndSynHandler::GetHisPoint(int nMsgType, SStringA stockID, int nPeriod, int nGroup, SStringA attInfo)
{
	SendInfo info = { 0 };
	info.MsgType = nMsgType;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int attSize = attInfo.GetLength() + 1;
	int nSize = sizeof(info) + attSize + sizeof(attSize);
	char *msg = new char[nSize];
	memcpy_s(msg, nSize, &info, sizeof(info));
	int nOffset = sizeof(info);
	memcpy_s(msg + nOffset, nSize, &attSize, sizeof(attSize));
	nOffset += sizeof(attSize);
	memcpy_s(msg + nOffset, nSize, attInfo, attSize);
	int ret = send(m_NetClinet.GetSocket(), msg, nSize, 0);
	return ret > 0;
}

bool CWndSynHandler::GetMarket(SStringA stockID, int nGroup)
{
	SendInfo info = { 0 };
	strcpy_s(info.str, stockID);
	info.Group = nGroup;
	//发送订阅数据
	info.MsgType = SendType_SubIns;
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	//发送获取数据请求
	if (nGroup != Group_Stock)
		info.MsgType = SendType_IndexMarket;
	else
		info.MsgType = SendType_StockMarket;
	ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CWndSynHandler::GetHisKline(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_HisPeriodKline;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CWndSynHandler::GetHisCallAction(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_HisCallAction;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}


void CWndSynHandler::InitDataHandleMap()
{
	m_dataHandleMap[ClearOldData] =
		&CWndSynHandler::OnClearData;
	m_dataHandleMap[UpdateTFMarket] =
		&CWndSynHandler::OnUpdateTFMarket;
	m_dataHandleMap[UpdateRtRps] =
		&CWndSynHandler::OnUpdateRtRps;
	m_dataHandleMap[UpdateCallAction] =
		&CWndSynHandler::OnUpdateCallAction;

}

void CWndSynHandler::InitNetHandleMap()
{
	m_netHandleMap[RecvMsg_ClientID]
		= &CWndSynHandler::OnMsgClientID;
	m_netHandleMap[RecvMsg_StockInfo]
		= &CWndSynHandler::OnMsgStockInfo;
	m_netHandleMap[RecvMsg_RTTimeLine]
		= &CWndSynHandler::OnMsgRTTimeLine;
	m_netHandleMap[RecvMsg_TodayTimeLine]
		= &CWndSynHandler::OnMsgTodayTimeLine;
	m_netHandleMap[RecvMsg_HisRpsPoint]
		= &CWndSynHandler::OnMsgHisRpsPoint;
	m_netHandleMap[RecvMsg_LastDayEma]
		= &CWndSynHandler::OnMsgLastDayEma;
	m_netHandleMap[RecvMsg_RTIndexMarket]
		= &CWndSynHandler::OnMsgRTIndexMarket;
	m_netHandleMap[RecvMsg_RTStockMarket]
		= &CWndSynHandler::OnMsgRTStockMarket;
	m_netHandleMap[RecvMsg_HisIndexMarket]
		= &CWndSynHandler::OnMsgHisIndexMarket;
	m_netHandleMap[RecvMsg_HisStockMarket]
		= &CWndSynHandler::OnMsgHisStockMarket;
	m_netHandleMap[RecvMsg_HisKline]
		= &CWndSynHandler::OnMsgHisKline;
	m_netHandleMap[RecvMsg_CloseInfo]
		= &CWndSynHandler::OnMsgCloseInfo;
	m_netHandleMap[RecvMsg_Wait]
		= &CWndSynHandler::OnMsgWait;
	m_netHandleMap[RecvMsg_Reinit]
		= &CWndSynHandler::OnMsgReInit;
	m_netHandleMap[RecvMsg_RTTFMarket]
		= &CWndSynHandler::OnMsgRTTFMarket;
	m_netHandleMap[RecvMsg_RTRps]
		= &CWndSynHandler::OnMsgRtRps;
	m_netHandleMap[RecvMsg_HisSecPoint]
		= &CWndSynHandler::OnMsgHisSecPoint;
	m_netHandleMap[RecvMsg_RehabInfo]
		= &CWndSynHandler::OnMsgRehabInfo;
	m_netHandleMap[RecvMsg_CallAction]
		= &CWndSynHandler::OnMsgCallAction;
	m_netHandleMap[RecvMsg_HisCallAction]
		= &CWndSynHandler::OnMsgHisCallAction;

}

void CWndSynHandler::InitSynHandleMap()
{
	m_synHandleMap[Syn_RemoveWnd]
		= &CWndSynHandler::OnRemoveWnd;
	m_synHandleMap[Syn_ListData]
		= &CWndSynHandler::OnUpdateList;
	m_synHandleMap[Syn_GetMarket]
		= &CWndSynHandler::OnGetMarket;
	m_synHandleMap[Syn_GetKline]
		= &CWndSynHandler::OnGetKline;
	m_synHandleMap[Syn_GetPoint]
		= &CWndSynHandler::OnGetPoint;
	m_synHandleMap[Syn_Point]
		= &CWndSynHandler::OnUpdatePoint;
	m_synHandleMap[Syn_HisRpsPoint]
		= &CWndSynHandler::OnHisRpsPoint;
	m_synHandleMap[Syn_RTIndexMarket]
		= &CWndSynHandler::OnRTIndexMarket;
	m_synHandleMap[Syn_RTStockMarket]
		= &CWndSynHandler::OnRTStockMarket;
	m_synHandleMap[Syn_HisIndexMarket]
		= &CWndSynHandler::OnHisIndexMarket;
	m_synHandleMap[Syn_HisStockMarket]
		= &CWndSynHandler::OnHisStockMarket;
	m_synHandleMap[Syn_HisKline]
		= &CWndSynHandler::OnHisKline;
	m_synHandleMap[Syn_CloseInfo]
		= &CWndSynHandler::OnCloseInfo;
	m_synHandleMap[Syn_Reinit]
		= &CWndSynHandler::OnReinit;
	m_synHandleMap[Syn_HisSecPoint]
		= &CWndSynHandler::OnHisSecPoint;
	m_synHandleMap[Syn_RehabInfo]
		= &CWndSynHandler::OnRehabInfo;
	m_synHandleMap[Syn_HisCallAction]
		= &CWndSynHandler::OnHisCallAction;
	m_synHandleMap[Syn_GetCallAction]
		= &CWndSynHandler::OnGetCallAction;
}


bool CWndSynHandler::CheckInfoRecv()
{
	BOOL bReady =
		m_NetHandleFlag[RecvMsg_ClientID] &
		m_NetHandleFlag[RecvMsg_CloseInfo];
	if (bReady == TRUE)
	{
		if (MSG_SUCC == m_NetHandleFlag[RecvMsg_StockInfo])
		{
			//SendMsg(m_RpsProcThreadID, UpdateTodayData,
			//	todayDataBuffer, todayDataSize);
			TraceLog("所有信息接收完毕");
			delete[]todayDataBuffer;
			todayDataBuffer = nullptr;
			todayDataSize = 0;
			return TRUE;
		}
	}
	TraceLog("有信息接收错误，重新进行登陆请求");
	return FALSE;

}


bool CWndSynHandler::CheckCmdLine()
{
	TraceLog("cmdLine:%s", StrW2StrA(m_strCmdLine));
	if (m_strCmdLine == L"-NoCheck")
		return true;
	else
	{
		if (!m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
			return true;

		SStringA strMD5 = "";
		SStringA strFileMD5 = "";
		while (GetAutoUpdateFileVer(strMD5))
			break;
		ifstream ifile(".\\AutoUpdate.exe", std::ios::binary);
		if (ifile.is_open())
		{
			MD5 MD5(ifile);
			strFileMD5 = MD5.toString().c_str();
			ifile.close();
		}
		if (strFileMD5 == "" || strMD5 != strFileMD5)
		{
			while (GetAutoUpdateFile(strMD5))
				break;
		}
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcessA(NULL, "AutoUpdate", NULL, NULL, FALSE, 0,
			NULL, NULL, &si, &pi);
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}
	return false;
}


bool CWndSynHandler::GetAutoUpdateFile(SStringA strMD5)
{
	SendInfo si = { 0 };
	si.MsgType = ComSend_UpdateFile;
	::send(m_NetClinet.GetSocket(), (char*)&si, sizeof(si), 0);
	ReceiveInfo recvInfo;
	int nRecv = 0;
	while (true)
	{
		int nRet = ::recv(m_NetClinet.GetSocket(), (char*)&recvInfo + nRecv,
			sizeof(recvInfo) - nRecv, 0);
		nRecv += nRet;
		if (nRecv == sizeof(recvInfo))
			break;
	}
	bool bSuccess = false;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(m_NetClinet.GetSocket(), recvInfo.DataSize, '#', buffer))
	{
		MD5 md5(buffer, recvInfo.SrcDataSize);
		if (md5.toString().c_str() == strMD5)
		{
			std::ofstream ofile(".\\AutoUpdate.exe", std::ios::binary);
			if (ofile.is_open())
			{
				ofile.write(buffer, recvInfo.SrcDataSize);
				ofile.close();
				bSuccess = true;
			}
		}
	}
	delete[]buffer;
	buffer = nullptr;
	return bSuccess;

}

bool CWndSynHandler::GetAutoUpdateFileVer(SStringA &strMD5)
{
	SendInfo si = { 0 };
	si.MsgType = ComSend_UpdateFileVer;
	::send(m_NetClinet.GetSocket(), (char*)&si, sizeof(si), 0);
	ReceiveInfo recvInfo;
	int nRecv = 0;
	while (true)
	{
		int nRet = ::recv(m_NetClinet.GetSocket(), (char*)&recvInfo + nRecv,
			sizeof(recvInfo) - nRecv, 0);
		nRecv += nRet;
		if (nRecv == sizeof(recvInfo))
			break;
	}
	bool bSuccess = false;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(m_NetClinet.GetSocket(), recvInfo.DataSize, '#', buffer))
	{
		strMD5 = buffer;
		bSuccess = true;
	}
	delete[]buffer;
	buffer = nullptr;
	return bSuccess;
}

bool CWndSynHandler::HandleRpsData(SStringA strDataName,
	sRps& data, map<string, double>&filterDataMap)
{
	string strName = strDataName;

	filterDataMap[strName + "MACD520"] = data.fMacd520;
	filterDataMap[strName + "MACD2060"] = data.fMacd2060;
	filterDataMap[strName + "RPS520"] = data.fRps520;
	filterDataMap[strName + "RPS2060"] = data.fRps2060;
	filterDataMap[strName + "RANK520"] = data.nRank520;
	filterDataMap[strName + "RANK2060"] = data.nRank2060;
	filterDataMap[strName + "POINT520"] = data.fPoint520;
	filterDataMap[strName + "POINT2060"] = data.fPoint2060;
	filterDataMap[strName + "RANK520L1"] = data.nL1Rank520;
	filterDataMap[strName + "RANK2060L1"] = data.nL1Rank2060;
	filterDataMap[strName + "POINT520L1"] = data.fL1Point520;
	filterDataMap[strName + "POINT2060L1"] = data.fL1Point2060;
	filterDataMap[strName + "RANK520L2"] = data.nL2Rank520;
	filterDataMap[strName + "RANK2060L2"] = data.nL1Rank2060;
	filterDataMap[strName + "POINT520L2"] = data.fL2Point520;
	filterDataMap[strName + "POINT2060L2"] = data.fL2Point2060;
	return true;
}

bool CWndSynHandler::HandleSecData(SStringA strDataName, sSection & data, map<string, double>& filterDataMap)
{
	string strName = strDataName;

	filterDataMap[strName + "RANK"] = data.rank;
	filterDataMap[strName + "POINT"] = data.point;
	filterDataMap[strName + "RANKL1"] = data.rankL1;
	filterDataMap[strName + "POINTL1"] = data.pointL1;
	filterDataMap[strName + "RANKL2"] = data.rankL2;
	filterDataMap[strName + "POINTL2"] = data.pointL2;

	return true;
}

void CWndSynHandler::UpdateRtRpsPointData(vector<RtPointData>& subDataVec,
	RtPointData & dstData, sRps & rpsData, SStringA strDataName, int nGroup)
{
	dstData.data.value = rpsData.fPoint520;
	strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point520");
	subDataVec.emplace_back(dstData);
	dstData.data.value = rpsData.fPoint2060;
	strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point2060");
	subDataVec.emplace_back(dstData);
	if (nGroup == Group_Stock)
	{
		dstData.data.value = rpsData.fL1Point520;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point520L1");
		subDataVec.emplace_back(dstData);
		dstData.data.value = rpsData.fL1Point2060;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point2060L1");
		subDataVec.emplace_back(dstData);
		dstData.data.value = rpsData.fL2Point520;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point520L2");
		subDataVec.emplace_back(dstData);
		dstData.data.value = rpsData.fL2Point2060;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point2060L2");
		subDataVec.emplace_back(dstData);

	}

}

void CWndSynHandler::UpdateRtSecPointData(vector<RtPointData>& subDataVec,
	RtPointData & dstData, sSection & secData, SStringA strDataName, int nGroup)
{
	dstData.data.value = secData.point;
	strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "Point");
	subDataVec.emplace_back(dstData);
	if (nGroup == Group_Stock)
	{
		dstData.data.value = secData.pointL1;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "PointL1");
		subDataVec.emplace_back(dstData);
		dstData.data.value = secData.pointL2;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, strDataName + "PointL2");
		subDataVec.emplace_back(dstData);
	}

}

void CWndSynHandler::UpdateRtSecPointFromCAInfo(vector<RtPointData>& subDataVec,
	RtPointData & dstData, CAInfo & caInfo, int nGroup)
{
	dstData.data.value = caInfo.VolPoint;
	strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaVolPoint");
	subDataVec.emplace_back(dstData);
	dstData.data.value = caInfo.AmoPoint;
	strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaAmoPoint");
	subDataVec.emplace_back(dstData);
	if (nGroup == Group_Stock)
	{

		dstData.data.value = caInfo.VolPointL1;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaVolPointL1");
		subDataVec.emplace_back(dstData);

		dstData.data.value = caInfo.VolPointL2;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaVolPointL2");
		subDataVec.emplace_back(dstData);

		dstData.data.value = caInfo.AmoPointL1;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaAmoPointL1");
		subDataVec.emplace_back(dstData);

		dstData.data.value = caInfo.AmoPointL2;
		strcpy_s(dstData.dataName, MAX_NAME_LENGTH, "CaAmoPointL2");
		subDataVec.emplace_back(dstData);

	}
}

bool CWndSynHandler::RecvInfoHandle(BOOL & bNeedConnect,
	int &nOffset, ReceiveInfo &recvInfo)
{
	if (bNeedConnect)
	{
		if (m_NetClinet.GetExitState())
			return 0;
		if (m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
		{
			SendIDInfo_t info = { 0 };
			info.MsgType = ComSend_ReConnect;
			info.ClinetID = m_NetClinet.GetClientID();
			::send(m_NetClinet.GetSocket(), (char*)&info,
				sizeof(info), 0);
			bNeedConnect = false;
		}
		else
			return false;
	}

	int ret = recv(m_NetClinet.GetSocket(),
		(char*)&recvInfo + nOffset,
		sizeof(recvInfo) - nOffset, 0);
	if (ret == 0)
	{
		nOffset = 0;
		m_NetClinet.OnConnect(NULL, NULL);
		bNeedConnect = true;
		TraceLog("与服务器断开连接");
		return false;
	}

	if (SOCKET_ERROR == ret)
	{
		//nOffset = 0;
		if (m_NetClinet.GetExitState())
			return false;
		int nError = WSAGetLastError();
		if (nError == WSAECONNRESET)
		{
			m_NetClinet.OnConnect(NULL, NULL);
			bNeedConnect = true;
		}
		return false;
	}

	if (ret + nOffset < sizeof(recvInfo))
	{
		nOffset += ret;
		return false;
	}
	nOffset = 0;

	return true;
}

void CWndSynHandler::OnMsgClientID(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_NetClinet.SetClientID(((ReceiveIDInfo)recvInfo).ClientID);
	m_NetHandleFlag[RecvMsg_ClientID] = TRUE;
	TraceLog("接收客户端ID成功");
}

void CWndSynHandler::OnMsgStockInfo(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	StockInfo stkInfo = { 0 };
	const ReceiveStockInfo &info = recvInfo;
	Sleep(10);
	if (StockInfo_Stock == info.InfoType)
		TraceLog("开始接收股票");
	else if (StockInfo_SWL1 == info.InfoType)
		TraceLog("开始接收申万1级行业信息");
	else if (StockInfo_SWL2 == info.InfoType)
		TraceLog("开始接收申万2级行业信息");
	else if (StockInfo_Index == info.InfoType)
		TraceLog("开始接收重要指数信息");

	if (ReceiveData(netSocket, info.DataSize, '#', buffer))
	{
		int size = info.DataSize / sizeof(stkInfo);
		switch (info.InfoType)
		{
		case StockInfo_Stock:
		{
			StockInfo *infoArr = (StockInfo *)buffer;
			for (int i = 0; i < size; ++i)
			{
				auto &data = infoArr[i];
				m_ListInsVec[Group_Stock].emplace_back(data.SecurityID);
				m_ListStockInfoMap[Group_Stock].hash[data.SecurityID] = data;
				m_ListInfoVec[Group_Stock].emplace_back(data);
				m_StockName.hash[data.SecurityID] = data.SecurityName;
			}
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[0] = 1;
			TraceLog("接收股票信息成功");
		}
		break;
		case StockInfo_SWL1:
		{
			StockInfo *infoArr = (StockInfo *)buffer;
			for (int i = 0; i < size; ++i)
			{
				auto &data = infoArr[i];
				m_ListInsVec[Group_SWL1].emplace_back(data.SecurityID);
				m_ListInfoVec[Group_SWL1].emplace_back(data);
				m_StockName.hash[data.SecurityID] = data.SecurityName;

			}
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[1] = 1;
			TraceLog("接收申万1级行业信息成功");

		}
		break;
		case StockInfo_SWL2:
		{
			StockInfo *infoArr = (StockInfo *)buffer;
			for (int i = 0; i < size; ++i)
			{
				auto &data = infoArr[i];
				m_ListInsVec[Group_SWL2].emplace_back(data.SecurityID);
				m_ListInfoVec[Group_SWL2].emplace_back(data);
				m_StockName.hash[data.SecurityID] = data.SecurityName;

			}
			TraceLog("接收申万2级行业信息成功");
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[2] = 1;
		}
		break;
		case StockInfo_Index:
		{
			StockInfo *infoArr = (StockInfo *)buffer;
			for (int i = 0; i < size; ++i)
			{
				auto &data = infoArr[i];
				m_ListInsVec[Group_SWL1].emplace_back(data.SecurityID);
				m_ListInsVec[Group_SWL2].emplace_back(data.SecurityID);
				m_ListInfoVec[Group_SWL1].emplace_back(data);
				m_ListInfoVec[Group_SWL2].emplace_back(data);
				m_StockName.hash[data.SecurityID] = data.SecurityName;
			}
			TraceLog("接收重要指数信息成功");
			((BYTE*)&m_NetHandleFlag[RecvMsg_StockInfo])[3] = 1;
		}
		break;
		default:
			break;
		}
	}
	else
		TraceLog("接收信息失败");
	delete[] buffer;
	buffer = nullptr;

}

void CWndSynHandler::OnMsgRTTimeLine(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	TimeLineData stkInfo = { 0 };
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_RpsProcThreadID, UpdateData,
			buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgTodayTimeLine(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_NetHandleFlag[RecvMsg_TodayTimeLine] = FALSE;
	TraceLog("开始接收当日历史数据");
	if (todayDataBuffer)
	{
		delete[]todayDataBuffer;
		todayDataBuffer = nullptr;
	}
	todayDataSize = recvInfo.DataSize + sizeof(recvInfo.SrcDataSize);
	todayDataBuffer = new char[todayDataSize];
	memcpy_s(todayDataBuffer, todayDataSize,
		&recvInfo.SrcDataSize, sizeof(recvInfo.SrcDataSize));
	if (ReceiveData(netSocket, recvInfo.DataSize, '#',
		todayDataBuffer + sizeof(recvInfo.SrcDataSize)))
	{
		m_NetHandleFlag[RecvMsg_TodayTimeLine] = TRUE;
		TraceLog("接收当日历史数据成功");
	}
	else
		TraceLog("接收当日历史数据失败");
	SetEvent(g_hEvent);
}

void CWndSynHandler::OnMsgHisRpsPoint(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisRpsPoint, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgLastDayEma(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	TraceLog("开始接收最后的EMA数据");
	m_NetHandleFlag[RecvMsg_LastDayEma] = FALSE;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		SendMsg(m_RpsProcThreadID, UpdateLastDayEma,
			buffer, recvInfo.DataSize);
		m_NetHandleFlag[RecvMsg_LastDayEma] = TRUE;
		TraceLog("接收最后的EMA数据成功");
	}
	else
		TraceLog("接收最后的EMA数据失败");
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgRTIndexMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_uMsgThreadID, Syn_RTIndexMarket, buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgRTStockMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
		SendMsg(m_uMsgThreadID, Syn_RTStockMarket, buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgHisIndexMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisIndexMarket, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgHisStockMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisStockMarket, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgHisKline(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisKline, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgCloseInfo(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	TraceLog("开始接收昨日收盘数据");
	m_NetHandleFlag[RecvMsg_CloseInfo] = FALSE;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		SendMsg(m_uMsgThreadID, Syn_CloseInfo,
			buffer, recvInfo.DataSize);
		m_NetHandleFlag[RecvMsg_CloseInfo] = TRUE;
		TraceLog("接收昨日收盘数据成功");
	}
	else
		TraceLog("接收昨日收盘数据失败");

	delete[]buffer;
	buffer = nullptr;
	//SetEvent(g_hEvent);

}

void CWndSynHandler::OnMsgWait(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	m_bServerReady = false;
	SetEvent(g_hEvent);
}

void CWndSynHandler::OnMsgReInit(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	SendMsg(m_uMsgThreadID, Syn_Reinit,
		NULL, 0);
}

void CWndSynHandler::OnMsgRTTFMarket(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	TimeLineData stkInfo = { 0 };
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		unsigned long  ulSize = recvInfo.DataSize;
		unsigned long ulRawDataSize = recvInfo.SrcDataSize;
		unsigned char * RawData = new unsigned char[ulRawDataSize];
		int nReturn = uncompress(RawData, &ulRawDataSize, (Bytef*)buffer, ulSize);
		if (nReturn == Z_OK)
			SendMsg(m_RpsProcThreadID, UpdateTFMarket,
			(char*)RawData, ulRawDataSize);
		delete[]RawData;
		RawData = nullptr;
	}
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnMsgRtRps(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	TimeLineData stkInfo = { 0 };
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		unsigned long  ulSize = recvInfo.DataSize;
		unsigned long ulRawDataSize = recvInfo.SrcDataSize;
		unsigned char * RawData = new unsigned char[ulRawDataSize];
		int nReturn = uncompress(RawData, &ulRawDataSize, (Bytef*)buffer, ulSize);
		if (nReturn == Z_OK)
			SendMsg(m_RpsProcThreadID, UpdateRtRps,
			(char*)RawData, ulRawDataSize);
		delete[]RawData;
		RawData = nullptr;

	}
	delete[]buffer;
	buffer = nullptr;

}

void CWndSynHandler::OnMsgHisSecPoint(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisSecPoint, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;

}

void CWndSynHandler::OnMsgRehabInfo(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_RehabInfo, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;

}

void CWndSynHandler::OnMsgCallAction(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	TimeLineData stkInfo = { 0 };
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		unsigned long  ulSize = recvInfo.DataSize;
		unsigned long ulRawDataSize = recvInfo.SrcDataSize;
		unsigned char * RawData = new unsigned char[ulRawDataSize];
		int nReturn = uncompress(RawData, &ulRawDataSize, (Bytef*)buffer, ulSize);
		if (nReturn == Z_OK)
			SendMsg(m_RpsProcThreadID, UpdateCallAction,
			(char*)RawData, ulRawDataSize);
		delete[]RawData;
		RawData = nullptr;
	}
	delete[]buffer;
	buffer = nullptr;

}

void CWndSynHandler::OnMsgHisCallAction(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisCallAction, buffer, totalSize);
	delete[]buffer;
	buffer = nullptr;
}

void CWndSynHandler::OnNoDefineMsg(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	ReceiveData(netSocket, recvInfo.DataSize, '#', buffer);
	delete[]buffer;
	buffer = nullptr;
}


void CWndSynHandler::OnUpdateTFMarket(int nMsgLength, const char * info)
{
	int dataCount = nMsgLength / sizeof(TickFlowMarket);
	TickFlowMarket* dataArr = (TickFlowMarket*)info;
	int nCount = 0;
	int nErrName = 0;
	for (int i = 0; i < dataCount; ++i)
		m_TFMarketHash[dataArr[i].nPeriod].hash[dataArr[i].SecurityID] = dataArr[i];
	for (auto &it : m_TFMarketHash)
	{
		for (auto& data : it.second.hash)
		{
			//if(data.second.nTime)
			auto &tfMarket = data.second;
			//更新选股器数据
			auto &filterMap = m_FilterDataMap[Group_Stock][it.first].hash[data.first];
			filterMap["LASTPX"] = tfMarket.fClose;
			double fPreClose = m_preCloseMap.hash[data.first];
			filterMap["CHG"] = (tfMarket.fClose - fPreClose) / fPreClose * 100;
			filterMap["VOL"] = tfMarket.nVolume;
			filterMap["HIGH"] = tfMarket.fHigh;
			filterMap["LOW"] = tfMarket.fLow;
			filterMap["OPEN"] = tfMarket.fOpen;
			filterMap["CLOSE"] = tfMarket.fClose;
			filterMap["AMOUNT"] = tfMarket.fAmount;
			filterMap["POC"] = tfMarket.fPOC;
			filterMap["ABV"] = tfMarket.ActBuyVol;
			filterMap["ASV"] = tfMarket.ActSellVol;
			filterMap["ABO"] = tfMarket.uActBuyOrderCount;
			filterMap["ASO"] = tfMarket.uActSellOrderCount;
			filterMap["PBO"] = tfMarket.uPasBuyOrderCount;
			filterMap["PSO"] = tfMarket.uPasSellOrderCount;
			filterMap["ABSR"] = tfMarket.ABSR;
			filterMap["A2PBSR"] = tfMarket.A2PBSR;
			filterMap["AABSR"] = tfMarket.AABSR;
			filterMap["POCR"] = tfMarket.POCR;
		}
	}

	if (m_bFirstData)
	{
		SetEvent(g_hEvent);
		m_bFirstData = false;
	}
	SendMsg(m_uMsgThreadID, Syn_Point, nullptr, 0);
	SendMsg(m_uMsgThreadID, Syn_ListData, nullptr, 0);
}

void CWndSynHandler::OnUpdateRtRps(int nMsgLength, const char * info)
{
	int dataCount = nMsgLength / sizeof(RtRps);
	RtRps* dataArr = (RtRps*)info;
	for (int i = 0; i < dataCount; ++i)
		m_RtRpsHash[dataArr[i].nGroup][dataArr[i].nPeriod].hash[dataArr[i].SecurityID] = dataArr[i];
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto &preiodFilterMap = m_FilterDataMap[i];

		for (auto &periodData : m_RtRpsHash[i])
		{
			auto &FilterMap = preiodFilterMap[periodData.first].hash;

			for (auto& data : periodData.second.hash)
			{
				auto& rpsData = data.second;
				CoreData cd = { 0 };
				cd.date = rpsData.nDate;
				cd.time = rpsData.nTime;
				auto &filter = FilterMap[data.first];
				HandleRpsData("close", rpsData.rpsClose, filter);
				HandleRpsData("amount", rpsData.rpsClose, filter);
				HandleSecData("amount", rpsData.secAmount, filter);
			}
		}
	}
}

void CWndSynHandler::OnClearData(int nMsgLength, const char * info)
{
	m_bTodayInit = false;
	ClearData();
	SetVectorSize();
	SetEvent(g_hLoginEvent);
}

void CWndSynHandler::OnUpdateCallAction(int nMsgLength, const char * info)
{
	int dataCount = nMsgLength / sizeof(CAInfo);
	CAInfo* dataArr = (CAInfo*)info;
	int nCount = 0;
	int nErrName = 0;
	for (int i = 0; i < dataCount; ++i)
		m_CallActionHash[dataArr[i].group].hash[dataArr[i].SecurityID] = dataArr[i];
	m_bCaUpdate = true;
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		for (auto& data : m_CallActionHash[i].hash)
		{
			//if(data.second.nTime)
			auto &caInfo = data.second;
			//更新选股器数据
			auto &filterMap = m_FilterDataMap[i][Period_1Day].hash[data.first];
			filterMap["CAVOL"] = caInfo.Volume;
			filterMap["CAVOLPOINT"] = caInfo.VolPoint;
			filterMap["CAVOLRANK"] = caInfo.VolRank;
			filterMap["CAVOLPOINTL1"] = caInfo.VolPointL1;
			filterMap["CAVOLRANKL1"] = caInfo.VolRankL1;
			filterMap["CAVOLPOINTL2"] = caInfo.VolPointL2;
			filterMap["CAVOLRANKL2"] = caInfo.VolRankL2;
			filterMap["CAAMO"] = caInfo.Amount;
			filterMap["CAAMOPOINT"] = caInfo.AmoPoint;
			filterMap["CAAMORANK"] = caInfo.AmoRank;
			filterMap["CAAMOPOINTL1"] = caInfo.AmoPointL1;
			filterMap["CAAMORANKL1"] = caInfo.AmoRankL1;
			filterMap["CAAMOPOINTL2"] = caInfo.AmoPointL2;
			filterMap["CAAMORANKL2"] = caInfo.AmoRankL2;

		}

	}
}



void CWndSynHandler::OnAddWnd(int nMsgLength, const char * info)
{
	pair<HWND, UINT> wndInfo = *(pair<HWND, UINT>*)info;
	AddWnd(wndInfo.first, wndInfo.second);
}

void CWndSynHandler::OnRemoveWnd(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	RemoveWnd(hWnd);
}

void CWndSynHandler::OnGetMarket(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	m_WndSubMap[pDgInfo->hWnd][pDgInfo->Group] = pDgInfo->StockID;
	m_WndPointSubMap[pDgInfo->hWnd][pDgInfo->Group].clear();
	GetMarket(pDgInfo->StockID, pDgInfo->Group);
}

void CWndSynHandler::OnGetKline(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	GetHisKline(pDgInfo->StockID, pDgInfo->Period, pDgInfo->Group);
}

void CWndSynHandler::OnGetPoint(int nMsgLength, const char * info)
{
	ExDataGetInfo *pDgInfo = (ExDataGetInfo *)info;
	m_WndPointSubMap[pDgInfo->hWnd][pDgInfo->nAskGroup][pDgInfo->StockID].emplace_back(*pDgInfo);
	GetHisPoint(m_PointGetMsg[pDgInfo->Type], pDgInfo->StockID,
		pDgInfo->Period, pDgInfo->Group, pDgInfo->exMsg);
}

void CWndSynHandler::OnUpdateList(int nMsgLength, const char * info)
{

	for (auto &it : m_hWndMap)
		SendMsg(it.second, Syn_ListData, NULL, 0);

}

void CWndSynHandler::OnUpdatePoint(int nMsgLength, const char * info)
{
	for (auto &wndSub : m_WndPointSubMap)
	{
		HWND hWnd = wndSub.first;
		for (auto &subInfo : wndSub.second)
		{
			vector<RtPointData>subDataVec;
			::EnterCriticalSection(&m_cs);
			for (auto &getInfo : subInfo.second)
			{
				SStringA StockID = getInfo.first;
				for (int i = Group_SWL1; i <= subInfo.first; ++i)
				{
					auto &dataMap = m_RtRpsHash[i];
					for (auto &periodDataPair : dataMap)
					{
						if (periodDataPair.second.hash.count(StockID))
						{
							if (periodDataPair.second.hash.count(StockID))
							{
								RtPointData data;
								strcpy_s(data.stockID, StockID);
								data.period = periodDataPair.first;
								auto &rpsData = periodDataPair.second.hash[StockID];
								data.data.date = rpsData.nDate;
								data.data.time = rpsData.nTime;
								UpdateRtRpsPointData(subDataVec, data, rpsData.rpsClose, "close", subInfo.first);
								UpdateRtRpsPointData(subDataVec, data, rpsData.rpsAmount, "amount", subInfo.first);
								UpdateRtSecPointData(subDataVec, data, rpsData.secAmount, "amount", subInfo.first);

							}
						}
					}
					//处理集合竞价数据
					if (i == subInfo.first && m_bCaUpdate)
					{
						auto& caInfo = m_CallActionHash[subInfo.first].hash[StockID];
						RtPointData data;
						strcpy_s(data.stockID, StockID);
						data.period = Period_1Day;
						UpdateRtSecPointFromCAInfo(subDataVec, data, caInfo, subInfo.first);
					}

				}
			}
			::LeaveCriticalSection(&m_cs);

			if (!subDataVec.empty())
			{
				int length = 4 + subDataVec.size() *
					sizeof(pair<int, pair<char[MAX_NAME_LENGTH], CoreData>>);
				char* msg = new char[length];
				memcpy_s(msg, length, &subInfo.first, 4);
				memcpy_s(msg + 4, length, &subDataVec[0], length - 4);
				SendMsg(m_hWndMap[hWnd], Syn_Point, msg, length);
				delete[]msg;
				msg = nullptr;
			}
		}
	}
	m_bCaUpdate = false;
}

void CWndSynHandler::OnHisRpsPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->Message;
	int nGroup = pRecvInfo->Group;
	int nOffset = sizeof(*pRecvInfo);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char *msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	for (auto &it : m_WndPointSubMap)
	{
		auto &hWnd = it.first;
		for (int i = Group_SWL1; i < Group_Count; ++i)
		{
			if (it.second.count(i)
				&& it.second[i].count(strStock))
			{
				auto &infoVec = it.second[i][strStock];
				for (auto getInfo = infoVec.begin(); getInfo < infoVec.end(); ++getInfo)
				{
					if (getInfo->Period == pRecvInfo->Period &&
						strcmp(getInfo->exMsg, msg) == 0)
					{
						SendMsg(m_hWndMap[hWnd], Syn_HisRpsPoint,
							info, nMsgLength);
						infoVec.erase(getInfo);
						break;
					}
				}
			}
		}
	}
	delete[]msg;
}

void CWndSynHandler::OnRTIndexMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(Group_SWL1)
			&& it.second[Group_SWL1] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_RTIndexMarket,
				info, nMsgLength);
		if (it.second.count(Group_SWL2)
			&& it.second[Group_SWL2] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_RTIndexMarket,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnRTStockMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(Group_Stock)
			&& it.second[Group_Stock] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_RTStockMarket,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnHisIndexMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(nGroup)
			&& it.second[nGroup] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_HisIndexMarket,
				info, nMsgLength);
	}
}

void CWndSynHandler::OnHisStockMarket(int nMsgLength, const char * info)
{
	ReceiveInfo* pRecvInfo = (ReceiveInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(nGroup)
			&& it.second[nGroup] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_HisStockMarket,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnHisKline(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->Message;
	int nGroup = pRecvInfo->Group;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(nGroup)
			&& it.second[nGroup] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_HisKline,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnCloseInfo(int nMsgLength, const char * info)
{
	pair<char[8], double>preCloseData;
	int dataCount = nMsgLength / sizeof(preCloseData);
	pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
	//::EnterCriticalSection(&m_csClose);
	m_preCloseMap.hash.clear();
	for (int i = 0; i < dataCount; ++i)
		m_preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
	for (auto &it : m_hWndMap)
		SendMsg(m_hWndMap[it.first], Syn_CloseInfo,
			info, nMsgLength);
}

void CWndSynHandler::OnReinit(int nMsgLength, const char * info)
{
	ReInit();
	for (auto &it : m_hWndMap)
	{
		::PostMessage(it.first, WM_WINDOW_MSG,
			WDMsg_ReInit, NULL);
	}
}

void CWndSynHandler::OnHisSecPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->Message;
	int nGroup = pRecvInfo->Group;
	int nOffset = sizeof(*pRecvInfo);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char *msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	for (auto &it : m_WndPointSubMap)
	{
		auto &hWnd = it.first;
		for (int i = Group_SWL1; i < Group_Count; ++i)
		{
			if (it.second.count(i)
				&& it.second[i].count(strStock))
			{
				auto &infoVec = it.second[i][strStock];
				for (auto getInfo = infoVec.begin(); getInfo < infoVec.end(); ++getInfo)
				{
					if (getInfo->Period == pRecvInfo->Period &&
						strcmp(getInfo->exMsg, msg) == 0)
					{
						SendMsg(m_hWndMap[hWnd], Syn_HisSecPoint,
							info, nMsgLength);
						infoVec.erase(getInfo);
						break;
					}
				}
			}
		}
	}
}

void CWndSynHandler::OnRehabInfo(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->Message;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(Group_Stock)
			&& it.second[Group_Stock] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_RehabInfo,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnHisCallAction(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->Message;
	int nGroup = pRecvInfo->Group;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(nGroup)
			&& it.second[nGroup] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_HisCallAction,
				info, nMsgLength);
	}

}

void CWndSynHandler::OnGetCallAction(int nMsgLength, const char * info)
{
	ExDataGetInfo *pDgInfo = (ExDataGetInfo *)info;
	GetHisCallAction(pDgInfo->StockID, pDgInfo->Period, pDgInfo->Group);
}



void CWndSynHandler::SetVectorSize()
{
	m_ListInsVec.resize(Group_Count);
	m_ListInfoVec.resize(Group_Count);
	m_FilterDataMap.resize(Group_Count);
	m_RtRpsHash.resize(Group_Count);
	m_CallActionHash.resize(Group_Count);
	for (auto period = Period_FenShi; period < Period_End; ++period)
		m_TFMarketHash[period] = strHash<TickFlowMarket>();
}

void CWndSynHandler::ClearData()
{
	m_ListInsVec.clear();
	m_ListInfoVec.clear();
	m_FilterDataMap.clear();
	m_TFMarketHash.clear();
	m_RtRpsHash.clear();
}

void CWndSynHandler::ReInit()
{
	ResetEvent(g_hEvent);
	ResetEvent(g_hLoginEvent);
	for (auto &it : m_hWndMap)
		::PostMessage(it.first, WM_WINDOW_MSG,
			WDMsg_HideWindow, NULL);
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		NULL, LoginMsg_Reinit);
	SStringW strInfo = L"重新初始化程序";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	m_NetHandleFlag.clear();
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	ResetEvent(g_hLoginEvent);
	SendMsg(m_RpsProcThreadID, ClearOldData, NULL, 0);
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	ResetEvent(g_hLoginEvent);
	m_bFirstData = true;
	SendInfo info;
	info.MsgType = ComSend_Connect;
	strcpy(info.str, "StkMarket");
	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	WaitForSingleObject(g_hEvent, INFINITE);
	while (!m_bServerReady)
	{
		m_bServerReady = true;
		::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
			NULL, LoginMsg_WaitAndTry);
		WaitForSingleObject(g_hLoginEvent, INFINITE);
		//ResetEvent(g_hLoginEvent);
		SendInfo info;
		info.MsgType = ComSend_Connect;
		strcpy(info.str, "StkMarket");
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	while (!CheckInfoRecv())
	{
		m_NetHandleFlag.clear();
		ResetEvent(g_hEvent);
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
		WaitForSingleObject(g_hEvent, INFINITE);
	}

	strInfo = L"获取信息成功";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	strInfo = L"处理当日历史数据，请等待...";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)strInfo.GetBuffer(1), LoginMsg_UpdateText);
	//WaitForSingleObject(g_hLoginEvent, INFINITE);
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_HideWnd);

}
