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
	ResetEvent(g_hLoginEvent);
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
		ResetEvent(g_hLoginEvent);
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

	Info = L"处理当日历史数据，请等待...";
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG,
		(WPARAM)Info.GetBuffer(1), LoginMsg_UpdateText);
	WaitForSingleObject(g_hLoginEvent, INFINITE);
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

	m_comDataNameVec.emplace_back("close");
	m_comDataNameVec.emplace_back("RPS520");
	m_comDataNameVec.emplace_back("MACD520");
	m_uniDataNameVec.emplace_back("Point520");
	m_uniDataNameVec.emplace_back("Rank520");
	m_comDataNameVec.emplace_back("RPS2060");
	m_comDataNameVec.emplace_back("MACD2060");
	m_uniDataNameVec.emplace_back("Point2060");
	m_uniDataNameVec.emplace_back("Rank2060");

}

void CWndSynHandler::InitNetConfig()
{
	CIniFile ini(".//config//NetConfig.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);

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

void CWndSynHandler::CalcHisData(const TimeLineData *dataArr,
	int nPeriod, int dataCount)
{
	auto & periodDataMap = m_commonDataMap[nPeriod];

	int tick = GetTickCount();
	SStringA nowStockID = "";
	vector<CoreData> * pDataVec = nullptr;

	for (int i = 0; i < dataCount; ++i)
	{
		if (nowStockID != dataArr[i].securityID)
		{
			nowStockID = dataArr[i].securityID;
			pDataVec = &periodDataMap[nowStockID]["close"];
		}
		if (dataArr[i].data.time == 925)
		{
			if (Period_FenShi == nPeriod)
				m_dataHandler.SetPeriodFenshiOpenEMAData(periodDataMap,
					dataArr[i]);
		}
		else
			m_dataHandler.UpdateClose(*pDataVec, dataArr[i], nPeriod);
		//pDataVec->emplace_back(dataArr[i]);
	}

	m_dataHandler.CalcHisRps(periodDataMap);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_SWL1][nPeriod], m_ListInsVec[Group_SWL1],
		m_ListStockInfoMap[Group_SWL1].hash);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_SWL2][nPeriod], m_ListInsVec[Group_SWL2],
		m_ListStockInfoMap[Group_SWL2].hash);
	m_dataHandler.RankPointHisData(periodDataMap,
		m_dataVec[Group_Stock][nPeriod], m_ListInsVec[Group_Stock],
		m_ListStockInfoMap[Group_Stock].hash);

	m_dataHandler.UpdateShowData(m_commonDataMap[nPeriod],
		m_dataVec[Group_SWL1][nPeriod], m_listDataMap[Group_SWL1][nPeriod],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL1]);
	m_dataHandler.UpdateShowData(m_commonDataMap[nPeriod],
		m_dataVec[Group_SWL2][nPeriod], m_listDataMap[Group_SWL2][nPeriod],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_SWL2]);
	m_dataHandler.UpdateShowData(m_commonDataMap[nPeriod],
		m_dataVec[Group_Stock][nPeriod], m_listDataMap[Group_Stock][nPeriod],
		m_comDataNameVec, m_uniDataNameVec,
		m_ListInsVec[Group_Stock]);

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


bool CWndSynHandler::GetHisPoint(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_GetHisPoint;
	info.Group = nGroup;
	info.Period = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
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


void CWndSynHandler::InitDataHandleMap()
{
	m_dataHandleMap[UpdateData] =
		&CWndSynHandler::OnTimeLineUpdate;
	m_dataHandleMap[UpdateTodayData] =
		&CWndSynHandler::OnTodayTimeLineProc;
	m_dataHandleMap[UpdateLastDayEma] =
		&CWndSynHandler::OnUpdateLastDayEma;
	m_dataHandleMap[ClearOldData] =
		&CWndSynHandler::OnClearData;
	m_dataHandleMap[UpdateTFMarket] =
		&CWndSynHandler::OnUpdateTFMarket;

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
	m_netHandleMap[RecvMsg_HisPoint]
		= &CWndSynHandler::OnMsgHisPoint;
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
	m_synHandleMap[Syn_HisPoint]
		= &CWndSynHandler::OnHisPoint;
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
}


bool CWndSynHandler::CheckInfoRecv()
{
	BOOL bReady =
		m_NetHandleFlag[RecvMsg_ClientID] &
		m_NetHandleFlag[RecvMsg_TodayTimeLine] &
		m_NetHandleFlag[RecvMsg_LastDayEma] &
		m_NetHandleFlag[RecvMsg_TodayTimeLine];
	if (bReady == TRUE)
	{
		if (MSG_SUCC == m_NetHandleFlag[RecvMsg_StockInfo])
		{
			SendMsg(m_RpsProcThreadID, UpdateTodayData,
				todayDataBuffer, todayDataSize);
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

void CWndSynHandler::SetPointDataCapacity()
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		auto &GroupStock = m_ListStockInfoMap[i];
		auto &GroupData = m_dataVec[i];
		for (auto &it : GroupStock.hash)
		{
			for (auto &period : m_PeriodVec)
			{
				auto &data = GroupData[period][it.first];
				int nCapatical = period == Period_FenShi ? 240 :
					(period <= Period_60Min ? 240 / period : 1);
				data["Point520"].reserve(nCapatical);
				data["Point2060"].reserve(nCapatical);
				if (Group_Stock == i)
				{
					data["L1Point520"].reserve(nCapatical);
					data["L1Point2060"].reserve(nCapatical);
					data["L2Point520"].reserve(nCapatical);
					data["L2Point2060"].reserve(nCapatical);
				}

			}
		}

	}
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

void CWndSynHandler::OnMsgHisPoint(SOCKET netSocket, ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	int offset = sizeof(recvInfo);
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer, offset))
		SendMsg(m_uMsgThreadID, Syn_HisPoint, buffer, totalSize);
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
	m_NetHandleFlag[RecvMsg_TodayTimeLine] = FALSE;
	char *buffer = new char[recvInfo.DataSize];
	if (ReceiveData(netSocket, recvInfo.DataSize, '#', buffer))
	{
		SendMsg(m_uMsgThreadID, Syn_CloseInfo,
			buffer, recvInfo.DataSize);
		m_NetHandleFlag[RecvMsg_TodayTimeLine] = TRUE;
		TraceLog("接收昨日收盘数据成功");
	}
	else
		TraceLog("接收昨日收盘数据失败");

	delete[]buffer;
	buffer = nullptr;

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
		SendMsg(m_RpsProcThreadID, UpdateTFMarket,
			buffer, recvInfo.DataSize);
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

void CWndSynHandler::OnTimeLineUpdate(int nMsgLength, const char * info) {
	if (!m_bTodayInit)
		return;
	int dataCount = nMsgLength / sizeof(TimeLineData);
	TimeLineData *dataArr = (TimeLineData *)info;
	for (int i = 0; i < m_PeriodVec.size(); ++i)
	{
		int Period = m_PeriodVec[i];
		auto & periodDataMap = m_commonDataMap[Period];
		for (int j = 0; j < dataCount; ++j)
		{
			if (dataArr[j].data.time == 925)
			{
				if (i == 0)
					m_dataHandler.SetPeriodFenshiOpenEMAData(
						periodDataMap, dataArr[j]);
			}
			else
				m_dataHandler.UpdateClose(
					periodDataMap, dataArr[j], m_PeriodVec[i]);
		}
		m_dataHandler.CalcRps(periodDataMap);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_SWL1][Period], m_ListInsVec[Group_SWL1],
			m_ListStockInfoMap[Group_SWL1].hash);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_SWL2][Period], m_ListInsVec[Group_SWL2],
			m_ListStockInfoMap[Group_SWL2].hash);
		m_dataHandler.RankPoint(periodDataMap,
			m_dataVec[Group_Stock][Period], m_ListInsVec[Group_Stock],
			m_ListStockInfoMap[Group_Stock].hash);
		m_dataHandler.UpdateShowData(m_commonDataMap[Period],
			m_dataVec[Group_SWL1][Period], m_listDataMap[Group_SWL1][Period],
			m_comDataNameVec, m_uniDataNameVec,
			m_ListInsVec[Group_SWL1]);
		m_dataHandler.UpdateShowData(m_commonDataMap[Period],
			m_dataVec[Group_SWL2][Period], m_listDataMap[Group_SWL2][Period],
			m_comDataNameVec, m_uniDataNameVec,
			m_ListInsVec[Group_SWL2]);
		m_dataHandler.UpdateShowData(m_commonDataMap[Period],
			m_dataVec[Group_Stock][Period], m_listDataMap[Group_Stock][Period],
			m_comDataNameVec, m_uniDataNameVec,
			m_ListInsVec[Group_Stock]);

	}
	
	::EnterCriticalSection(&m_csFilterData);
	::LeaveCriticalSection(&m_csFilterData);
	SendMsg(m_uMsgThreadID, Syn_Point, nullptr, 0);
	SendMsg(m_uMsgThreadID, Syn_ListData, nullptr, 0);

}

void CWndSynHandler::OnTodayTimeLineProc(int nMsgLength, const char * info) {
	//typeof()
	unsigned long nSize = *(unsigned long*)info;
	unsigned char* buffer = new unsigned char[nSize];
	uncompress(buffer, &nSize,
		(unsigned char*)(info + 4), nMsgLength - 4);
	int dataCount = nSize / sizeof(TimeLineData);
	TimeLineData *dataArr = (TimeLineData *)buffer;
	//WaitForSingleObject(g_hEvent, INFINITE);
	SetPointDataCapacity();
	vector<thread> procThreadVec(m_PeriodVec.size());
	for (int i = 0; i < m_PeriodVec.size(); ++i)
	{
		int Period = m_PeriodVec[i];
		thread t(&CWndSynHandler::CalcHisData, this, dataArr,
			Period, dataCount);
		procThreadVec[i].swap(t);
	}
	for (auto &it : procThreadVec)
		if (it.joinable())
			it.join();
	delete[]buffer;
	buffer = nullptr;

	m_bTodayInit = true;
	SetEvent(g_hLoginEvent);
	SendMsg(m_uMsgThreadID, Syn_Point, nullptr, 0);
	SendMsg(m_uMsgThreadID, Syn_ListData, nullptr, 0);
	for (auto &it : m_commonDataMap)
	{
		for (auto &data : it.second)
		{
			data.second["RPS520"].clear();
			data.second["RPS520"].shrink_to_fit();
			data.second["RPS2060"].clear();
			data.second["RPS2060"].shrink_to_fit();
		}
	}

}

void CWndSynHandler::OnUpdateLastDayEma(int nMsgLength, const char* info) {
	int dataCount = nMsgLength / sizeof(TimeLineData);
	TimeLineData* dataArr = (TimeLineData*)info;
	int preDataCount = dataCount / 6;
	int offset = 0;
	for (int i = 1; i < 7; ++i)	//分时图数据不需要设置
	{
		int nPeriod = m_PeriodVec[i];
		auto &comDataMap = m_commonDataMap[nPeriod];
		for (int j = 0; j < preDataCount; ++j)
		{
			auto &data = dataArr[offset];
			m_dataHandler.SetPreEMAData(comDataMap, data);
			++offset;
		}
	}

}

void CWndSynHandler::OnUpdateTFMarket(int nMsgLength, const char * info)
{
	int dataCount = nMsgLength / sizeof(TickFlowMarket);
	TickFlowMarket* dataArr = (TickFlowMarket*)info;
	for (int i = 0; i < dataCount; ++i)
		m_TFMarketHash[dataArr[i].nPeriod].hash[dataArr[i].SecurityID] = dataArr[i];
	for (auto &it : m_TFMarketHash)
	{
		for (auto& data : it.second.hash)
		{
			auto &tfMarket = data.second;
			double fDelta1 =
				(tfMarket.ActBuyVol*1.0 - tfMarket.ActSellVol) /
				(tfMarket.ActBuyVol*1.0 + tfMarket.ActSellVol) * 100;
			auto &dataMap = m_listDataMap[Group_Stock];
			CoreData cd = { 0 };
			cd.time = tfMarket.nTime;
			cd.value = fDelta1;
			dataMap[it.first][data.first]["ABSR"] = cd;
			double fActBuyVol = tfMarket.ActBuyVol*1.0
				/ tfMarket.uActBuyOrderCount*tfMarket.uPasSellOrderCount;
			double fActSelVol = tfMarket.ActSellVol*1.0 /
				tfMarket.uActSellOrderCount*tfMarket.uPasBuyOrderCount;
			double fDelta2 = (fActBuyVol - fActSelVol)
				/ (fActBuyVol + fActSelVol) * 100;
			cd.value = fDelta2;
			dataMap[it.first][data.first]["A2PBSR"] = cd;


			double avgBuyVol = tfMarket.ActBuyVol * 1.0
				/ tfMarket.uActBuyOrderCount;
			double avgSellVol = tfMarket.ActSellVol * 1.0
				/ tfMarket.uActSellOrderCount;
			double avgRatio = (avgBuyVol - avgSellVol)
				/ (avgBuyVol + avgSellVol) * 100;
			cd.value = avgRatio;
			dataMap[it.first][data.first]["AABSR"] = cd;


			double fPocRatio = (tfMarket.fPOC - tfMarket.fPrice)
				/ tfMarket.fPrice * 100;
			cd.value = fPocRatio;
			dataMap[it.first][data.first]["POCR"] = cd;

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
	GetMarket(pDgInfo->StockID, pDgInfo->Group);
}

void CWndSynHandler::OnGetKline(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	GetHisKline(pDgInfo->StockID, pDgInfo->Period, pDgInfo->Group);
}

void CWndSynHandler::OnGetPoint(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	::EnterCriticalSection(&m_cs);
	auto &PointMap = m_dataVec[pDgInfo->Group]\
		[pDgInfo->Period][pDgInfo->StockID];
	map<SStringA, vector<CoreData>> *pDataMap
		= new map<SStringA, vector<CoreData>>(PointMap);
	::LeaveCriticalSection(&m_cs);
	char msg[16];
	memcpy_s(msg, 16, &pDgInfo->Group, 4);
	memcpy_s(msg + 4, 16, &pDgInfo->Period, 8);
	memcpy_s(msg + 8, 16, &pDataMap, 8);
	SendMsg(m_hWndMap[pDgInfo->hWnd], Syn_TodayPoint,
		msg, 16);
	if (Period_FenShi != pDgInfo->Period)
		GetHisPoint(pDgInfo->StockID, pDgInfo->Period, pDgInfo->Group);
}

void CWndSynHandler::OnUpdateList(int nMsgLength, const char * info)
{
	for (auto &it : m_hWndMap)
		SendMsg(it.second, Syn_ListData, NULL, 0);

}

void CWndSynHandler::OnUpdatePoint(int nMsgLength, const char * info)
{
	for (auto &wndSub : m_WndSubMap)
	{
		HWND hWnd = wndSub.first;
		for (auto &subInfo : wndSub.second)
		{
			vector<pair<int, pair<char[16], CoreData>>>subDataVec;
			::EnterCriticalSection(&m_cs);
			auto &dataMap = m_dataVec[subInfo.first];
			for (auto &periodDataPair : dataMap)
			{
				if (periodDataPair.second.count(subInfo.second))
				{
					pair<int, pair<char[16], CoreData>> data;
					data.first = periodDataPair.first;
					auto &tmDataMap = periodDataPair.second[subInfo.second];
					for (auto &dataPair : tmDataMap)
					{
						if (dataPair.first.Find("Point") != -1
							&& !dataPair.second.empty())
						{
							strcpy_s(data.second.first, dataPair.first);
							data.second.second = dataPair.second.back();
							subDataVec.emplace_back(data);
						}
					}

				}
			}
			if (!subDataVec.empty())
			{
				int length = 4 + subDataVec.size() *
					sizeof(pair<int, pair<char[16], CoreData>>);
				char* msg = new char[length];
				memcpy_s(msg, length, &subInfo.first, 4);
				memcpy_s(msg + 4, length, &subDataVec[0], length - 4);
				SendMsg(m_hWndMap[hWnd], Syn_Point, msg, length);
				delete[]msg;
				msg = nullptr;
			}
			::LeaveCriticalSection(&m_cs);
		}
	}
}

void CWndSynHandler::OnHisPoint(int nMsgLength, const char * info)
{
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SStringA strStock = pRecvInfo->InsID;
	int nGroup = pRecvInfo->Group;
	for (auto &it : m_WndSubMap)
	{
		auto &hWnd = it.first;
		if (it.second.count(nGroup)
			&& it.second[nGroup] == strStock)
			SendMsg(m_hWndMap[hWnd], Syn_HisPoint,
				info, nMsgLength);
	}
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
	SStringA strStock = pRecvInfo->InsID;
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

void CWndSynHandler::SetVectorSize()
{
	m_dataVec.resize(Group_Count);
	m_ListInsVec.resize(Group_Count);
	m_ListInfoVec.resize(Group_Count);
	m_listDataMap.resize(Group_Count);
}

void CWndSynHandler::ClearData()
{
	m_dataVec.clear();
	m_ListInsVec.clear();
	m_listDataMap.clear();
	m_ListInfoVec.clear();
	m_commonDataMap.clear();
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
		ResetEvent(g_hLoginEvent);
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
	WaitForSingleObject(g_hLoginEvent, INFINITE);
	::PostMessage(m_pLoginDlg->m_hWnd, WM_LOGIN_MSG, NULL, LoginMsg_HideWnd);

}
