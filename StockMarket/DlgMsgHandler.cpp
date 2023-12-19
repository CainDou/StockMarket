#include "stdafx.h"
#include "DlgMsgHandler.h"
#include <thread>

CDlgMsgHandler::CDlgMsgHandler()
{
}


CDlgMsgHandler::~CDlgMsgHandler()
{
	Stop();
}

UINT CDlgMsgHandler::Init(HWND hWnd, vector<CWorkWnd*>& workWndVec, UINT SynID, BOOL bOnlyStock)
{
	m_hWnd = hWnd;
	m_WndVec = workWndVec;
	m_SynThreadID = SynID;
	m_bOnlyStock = bOnlyStock;
	InitMsgHandleMap();
	tMsgProc = std::thread(&CDlgMsgHandler::MsgProc, this);
	m_uThreadID = *(unsigned*)&tMsgProc.get_id();
	for (int i = 0; i < m_WndVec.size(); ++i)
	{
		m_WndHandleMap[m_WndVec[i]->m_hWnd] = i;
		m_WndVec[i]->SetParThreadID(m_uThreadID);

	}
	return m_uThreadID;
}

BOOL CDlgMsgHandler::Stop()
{
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (tMsgProc.joinable())
		tMsgProc.join();
	return TRUE;
}


void CDlgMsgHandler::InitMsgHandleMap()
{
	m_MsgHandleMap[WW_ListData]
		= &CDlgMsgHandler::OnUpdateList;
	m_MsgHandleMap[WW_GetMarket]
		= &CDlgMsgHandler::OnGetMarket;
	m_MsgHandleMap[WW_GetKline]
		= &CDlgMsgHandler::OnGetKline;
	m_MsgHandleMap[WW_GetPoint]
		= &CDlgMsgHandler::OnGetPoint;
	m_MsgHandleMap[WW_GetCallAction]
		= &CDlgMsgHandler::OnGetCallAction;
	m_MsgHandleMap[WW_GetHisTFBase]
		= &CDlgMsgHandler::OnGetHisTFBase;

	m_MsgHandleMap[Syn_Point]
		= &CDlgMsgHandler::OnUpdatePoint;
	//m_MsgHandleMap[Syn_TodayPoint]
	//	= &CDlgMsgHandler::OnTodayPoint;
	m_MsgHandleMap[Syn_HisRpsPoint]
		= &CDlgMsgHandler::OnHisRpsPoint;
	m_MsgHandleMap[Syn_RTIndexMarket]
		= &CDlgMsgHandler::OnRTIndexMarket;
	m_MsgHandleMap[Syn_RTStockMarket]
		= &CDlgMsgHandler::OnRTStockMarket;
	m_MsgHandleMap[Syn_HisIndexMarket]
		= &CDlgMsgHandler::OnHisIndexMarket;
	m_MsgHandleMap[Syn_HisStockMarket]
		= &CDlgMsgHandler::OnHisStockMarket;
	m_MsgHandleMap[Syn_HisKline]
		= &CDlgMsgHandler::OnHisKline;
	m_MsgHandleMap[Syn_CloseInfo]
		= &CDlgMsgHandler::OnCloseInfo;
	m_MsgHandleMap[WW_ChangeIndy]
		= &CDlgMsgHandler::OnChangeIndy;
	m_MsgHandleMap[Syn_HisSecPoint]
		= &CDlgMsgHandler::OnHisSecPoint;
	m_MsgHandleMap[Syn_RehabInfo]
		= &CDlgMsgHandler::OnRehabInfo;
	m_MsgHandleMap[Syn_HisCallAction]
		= &CDlgMsgHandler::OnHisCallAction;
	m_MsgHandleMap[Syn_HisTFBase]
		= &CDlgMsgHandler::OnHisTFBase;
	m_MsgHandleMap[Syn_TodayTFMarket]
		= &CDlgMsgHandler::OnTodayTFMarket;
	m_MsgHandleMap[Syn_RTTFMarkt]
		= &CDlgMsgHandler::OnRTTFMarket;
	m_MsgHandleMap[Syn_RTPriceVol]
		= &CDlgMsgHandler::OnRTPriceVol;



}

void CDlgMsgHandler::MsgProc()
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
		auto pFuc = m_MsgHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}

}

void CDlgMsgHandler::OnUpdateList(int nMsgLength, const char * info)
{
	for (auto &it : m_WndVec)
		SendMsg(it->GetThreadID(), WW_ListData,
			info, nMsgLength);

}

void CDlgMsgHandler::OnGetMarket(int nMsgLength, const char * info)
{
	DataGetInfo *pDgInfo = (DataGetInfo *)info;
	m_WndSubMap[pDgInfo->Group] = pDgInfo->StockID;
	m_WndPointSubMap.clear();
	SendMsg(m_SynThreadID, Syn_GetMarket, info, nMsgLength);
}

void CDlgMsgHandler::OnGetKline(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetKline, info, nMsgLength);
}

void CDlgMsgHandler::OnGetPoint(int nMsgLength, const char * info)
{
	ExDataGetInfo *pDgInfo = (ExDataGetInfo *)info;
	m_WndPointSubMap[pDgInfo->nAskGroup][pDgInfo->StockID].emplace_back(*pDgInfo);
	SendMsg(m_SynThreadID, Syn_GetPoint, info, nMsgLength);
}

void CDlgMsgHandler::OnUpdatePoint(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	SendMsg(m_WndVec[nGroup]->GetThreadID(), WW_Point,
		info + 4, nMsgLength - 4);
}

void CDlgMsgHandler::OnHisRpsPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisRpsPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnRTIndexMarket(int nMsgLength, const char * info)
{
	if (m_bOnlyStock)
		return;
	SStringA strStock = ((CommonIndexMarket*)info)->SecurityID;
	if (m_WndSubMap[Group_SWL1] == strStock)
		SendMsg(m_WndVec[Group_SWL1]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
	if (m_WndSubMap[Group_SWL2] == strStock)
		SendMsg(m_WndVec[Group_SWL2]->GetThreadID(), WW_RTIndexMarket,
			info, nMsgLength);
}

void CDlgMsgHandler::OnRTStockMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((CommonStockMarket*)info)->SecurityID;
	int nStart = m_bOnlyStock ? 0 : Group_Stock;
	for (int i = 0; i < m_WndVec.size(); ++i)
	{
		if (m_WndSubMap[i] == strStock)
			SendMsg(m_WndVec[i]->GetThreadID(), WW_RTStockMarket,
				info, nMsgLength);
	}
}

void CDlgMsgHandler::OnHisIndexMarket(int nMsgLength, const char * info)
{
	if (m_bOnlyStock)
		return;
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisIndexMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnHisStockMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisStockMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnHisKline(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisKline,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnCloseInfo(int nMsgLength, const char * info)
{
	for (int i = 0; i < m_WndVec.size(); ++i)
		SendMsg(m_WndVec[i]->GetThreadID(), WW_CloseInfo,
			info, nMsgLength);
}

void CDlgMsgHandler::OnChangeIndy(int nMsgLength, const char * info)
{
	int nGroup = *(int*)info;
	if (Group_SWL1 == nGroup)
	{
		for (int i = Group_SWL2; i < Group_Count; ++i)
			SendMsg(m_WndVec[i]->GetThreadID(), WW_ChangeIndy,
				info, nMsgLength);
	}
	else if (Group_SWL2 == nGroup)
		SendMsg(m_WndVec[Group_Stock]->GetThreadID(), WW_ChangeIndy,
			info, nMsgLength);
}

void CDlgMsgHandler::OnHisSecPoint(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisSecPoint,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnRehabInfo(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_RehabInfo,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnHisCallAction(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisCallAction,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnGetCallAction(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetCallAction, info, nMsgLength);
}

void CDlgMsgHandler::OnHisTFBase(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_HisTFBase,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnGetHisTFBase(int nMsgLength, const char * info)
{
	SendMsg(m_SynThreadID, Syn_GetHisTFBase, info, nMsgLength);
}

void CDlgMsgHandler::OnTodayTFMarket(int nMsgLength, const char * info)
{
	HWND hWnd = *(HWND*)info;
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo *)info;
	SendMsg(m_WndVec[m_WndHandleMap[hWnd]]->GetThreadID(), WW_TodayTFMarket,
		info + sizeof(hWnd), nMsgLength - sizeof(hWnd));
}

void CDlgMsgHandler::OnRTTFMarket(int nMsgLength, const char * info)
{
	SStringA strStock = ((TickFlowMarket*)info)[0].SecurityID;
	int nStart = m_bOnlyStock ? 0 : Group_Stock;
	for (int i = 0; i < m_WndVec.size(); ++i)
		if (m_WndSubMap[i] == strStock)
			SendMsg(m_WndVec[i]->GetThreadID(), WW_RTTFMarket,
				info, nMsgLength);

}

void CDlgMsgHandler::OnRTPriceVol(int nMsgLength, const char * info)
{
	SStringA strStock = ((PriceVolInfo*)info)[0].SecurityID;
	int nStart = m_bOnlyStock ? 0 : Group_Stock;
	for (int i = 0; i < m_WndVec.size(); ++i)
		if (m_WndSubMap[i] == strStock)
			SendMsg(m_WndVec[i]->GetThreadID(), WW_RTPriceVol,
				info, nMsgLength);
}
