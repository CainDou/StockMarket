#include "stdafx.h"
#include "DlgTradeSysFilter.h"
#include <io.h>
#include <fstream>
#include "IniFile.h"
#include "SColorListCtrlEx.h"
#include <functional>
#include "WndSynHandler.h"
#include "NetWorkClient.h"
#include <helper/SMenu.h>

extern CWndSynHandler g_WndSyn;
extern HWND g_MainWnd;
enum eTradeSysDir
{
	eTSD_Null,
	eTSD_Long,
	eTSD_Short,
};



const int nPeriodCount = 6;
map<int, SStringW> periodNameMap = { { Period_1Min,L"1分" },{ Period_5Min,L"5分" },
{ Period_15Min,L"15分" } ,{ Period_30Min,L"30分" } ,{ Period_60Min,L"60分" } ,{ Period_1Day,L"日" } };

std::vector<int> nMonthDays = { 31,28,31,30,31,30,31,31,30,31,30,31 };


double fRatio = 0.05;
enum eResHeader
{
	eRH_Index,
	eRH_SecurityID,
	eRH_Name,
	eRH_LastPx,
	eRH_ChgPct,
	eRH_TradeSys,
	eRH_Dir,
	eRH_Period,
	eRH_EnterPx,
	eRH_EnterDate,
	eRH_ExitPx,
	eRH_ExitDate,
	eRH_PosYield,
	eRH_PosDate,
};

std::map<int, int> IntMap = { {eRH_Dir,offsetof(ShowDataType,ShowDataType::nDir)} ,{eRH_Period,offsetof(ShowDataType,ShowDataType::nPeriod) }
,{eRH_EnterDate,offsetof(ShowDataType,ShowDataType::nEnterDate) },{eRH_ExitDate,offsetof(ShowDataType,ShowDataType::nExitDate) },
{eRH_PosDate,offsetof(ShowDataType,ShowDataType::nPosDay) } };
std::map<int, int> DoubleMap = { {eRH_LastPx,offsetof(ShowDataType,ShowDataType::fLastPrice) } ,{eRH_ChgPct,offsetof(ShowDataType,ShowDataType::fChgPct) } ,
{eRH_EnterPx,offsetof(ShowDataType,ShowDataType::fEnterPrice) },{eRH_ExitPx,offsetof(ShowDataType,ShowDataType::fExitPrice) },
{eRH_PosYield,offsetof(ShowDataType,ShowDataType::fYeild) } };
std::map<int, int> StrMap = { { eRH_SecurityID,offsetof(ShowDataType,ShowDataType::strSecurityID) } ,
{eRH_Name,offsetof(ShowDataType,ShowDataType::strName) } ,{eRH_TradeSys,offsetof(ShowDataType,ShowDataType::strSys) } };
enum eConditonType
{
	eCond_Today,
	eCond_Open,
};

enum eSortDir
{
	eSD_Less,
	eSD_Greater,
};

CDlgTradeSysFilter::CDlgTradeSysFilter() :SHostWnd(_T("LAYOUT:dlg_tradeSysFilter"))
{
	m_bLayoutInited = FALSE;
	m_bIsValid = TRUE;
	m_bExit = FALSE;
	nHisRecvStartDate = 99999999;
	nHisRecvEndDate = 0;
	nPrePageCount = 0;
	nNowPage = 0;
	nSortCol = 0;
	nSortState = ST_NULL;
}


CDlgTradeSysFilter::~CDlgTradeSysFilter()
{
	m_bExit = TRUE;
	if (m_thread.joinable())
		m_thread.join();
}

void SOUI::CDlgTradeSysFilter::OnClose()
{
	m_bIsValid = FALSE;
	ShowWindow(SW_HIDE);
	SStringA strPosFile;
	strPosFile.Format(".\\config\\TradeSys.position");
	if (_access(strPosFile, 0) == 0)
		remove(strPosFile);
}

void SOUI::CDlgTradeSysFilter::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void SOUI::CDlgTradeSysFilter::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void SOUI::CDlgTradeSysFilter::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void SOUI::CDlgTradeSysFilter::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE, TRUE);
		pBtnMax->SetVisible(FALSE, TRUE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE, TRUE);
		pBtnMax->SetVisible(TRUE, TRUE);
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_SizeChange, NULL);

}

int SOUI::CDlgTradeSysFilter::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL SOUI::CDlgTradeSysFilter::OnInitDialog(EventArgs * e)
{
	m_bLayoutInited = TRUE;
	InitTradeSysName();
	InitControls();
	m_thread = std::thread(&CDlgTradeSysFilter::DataHandle, this);
	int uMsgThreadID = *(unsigned*)&m_thread.get_id();
	g_WndSyn.SetTradeSysResWnd(m_hWnd, uMsgThreadID);
	SendMsg(m_uParWndThreadID, Syn_ReSendRtTradeSysRes, nullptr, 0);
	return 0;
}

LRESULT SOUI::CDlgTradeSysFilter::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	switch (wp)
	{
	case TSRMsg_UpdateRtData:
		UpdateRtShowDataVec();
		UpdateRtData();
		break;
	case TSRMsg_UpdateHisData:
		UpdateHisShowDataVec();
		UpdateHisData();
		break;
	case TSRMsg_ChangeShowPara:
		if (m_pRadioRt->IsChecked())
		{
			UpdateRtShowDataVec();
			UpdateRtData();
		}
		else
		{
			UpdateHisShowDataVec();
			UpdateHisData();
		}
		break;
	case TSRMSG_SizeChange:
		ChangeResLsSize();
		break;
	case TSRMSG_ChangeItemCount:
		if (m_pRadioRt->IsChecked())
			UpdateRtData();
		else
			UpdateHisData();
		break;
	case TSRMSG_SortData:
		SortShowData();
		if (m_pRadioRt->IsChecked())
			UpdateRtData();
		else
			UpdateHisData();
		break;
	default:
		break;
	}
	return 0;
}

void SOUI::CDlgTradeSysFilter::OnDestroy()
{
	SetMsgHandled(FALSE);

	if (m_bIsValid)
	{
		SStringA strPosFile;
		strPosFile.Format(".\\config\\TradeSys.position");
		std::ofstream ofile(strPosFile);
		if (ofile.is_open())
		{
			WINDOWPLACEMENT wp = { sizeof(wp) };
			::GetWindowPlacement(m_hWnd, &wp);

			ofile.write((char*)&wp, sizeof(wp));
			ofile.close();
		}
		ShowWindow(SW_HIDE);
	}

}

void SOUI::CDlgTradeSysFilter::OnBtnClose()
{
	GetNative()->SendMessage(WM_CLOSE);
}

void CDlgTradeSysFilter::UpdateRtShowDataVec()
{
	std::lock_guard<std::mutex> lk(m_mxRT);
	m_ShowDataVec.clear();

	for (int i = 0; i < m_TradeSysInfoVec.size(); ++i)
	{
		BOOL bChecked = m_pListTradeSys->GetCheckState(i);
		if (bChecked)
		{
			auto &dataVec = m_TradeResVec[i];
			BOOL bToday = m_pChkCondToday->IsChecked();
			BOOL bOpen = m_pChkCondOpen->IsChecked();
			for (int j = 0; j < dataVec.size(); ++j)
			{
				if (m_StockInfo.hash.count(dataVec[j].SecurityID) == 0)
					continue;

				//判断周期
				if (m_RtShowPeriodSet.count(dataVec[j].nPeriod) == 0)
					continue;
				//判断方向
				if (m_RtShowDirSet.count(dataVec[j].nType) == 0)
					continue;
				//判断是否当日触发
				if (bToday && (dataVec[j].nEnterDate == nTradingDay || dataVec[j].nExitDate == nTradingDay))
					m_ShowDataVec.emplace_back(ChangeResToShow(dataVec[j]));
				else if (bOpen && dataVec[j].nExitDate == 0)
					m_ShowDataVec.emplace_back(ChangeResToShow(dataVec[j]));

			}
		}
	}
	SortShowData();
	nTotalPage = m_ShowDataVec.size() / nPrePageCount;
	if (m_ShowDataVec.size() % nPrePageCount != 0)
		++nTotalPage;
	SStringW str;
	m_pTxtTotalPage->SetWindowTextW(str.Format(L"/%d页", max(1, nTotalPage)));
}


void CDlgTradeSysFilter::UpdateRtData()
{
	//m_pListRes->DeleteAllItems();
	//std::lock_guard<std::mutex> lk(m_mxRT);
	if (nNowPage >= nTotalPage)
	{
		nNowPage = nTotalPage - 1;
		SStringW str;
		m_pEditNowPage->SetWindowTextW(str.Format(L"%d", max(1, nTotalPage)));
	}
	if (nNowPage < 0)
		nNowPage = 0;
	int nStartPos = nNowPage * nPrePageCount;
	int nEndPos = min(nStartPos + nPrePageCount, m_ShowDataVec.size());
	int nItemCount = nEndPos - nStartPos;
	int nNowItemCount = m_pListRes->GetItemCount();
	if (nNowItemCount < nItemCount)
	{
		for (int i = nNowItemCount; i < nItemCount; ++i)
			m_pListRes->InsertItem(i, L"");
	}
	else if (nNowItemCount > nItemCount)
	{
		for (int i = nNowItemCount-1; i >= nItemCount; --i)
			m_pListRes->DeleteItem(i);
	}

	for (int i = nStartPos; i < nEndPos; ++i)
	{
		//auto PosInfo = m_ShowDataVec[i];
		//UpdateListData(i, m_TradeResVec[PosInfo.first][PosInfo.second]);
		UpdateListData(i-nStartPos,i, m_ShowDataVec[i]);
	}
	m_pListRes->RequestRelayout();
	m_pListRes->Invalidate();
}

void SOUI::CDlgTradeSysFilter::UpdateHisShowDataVec()
{
	std::lock_guard<std::mutex> lk(m_mxHis);
	m_ShowDataVec.clear();
	SYSTEMTIME st;
	m_pDtpStartDate->GetTime(st);
	int nStartDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_pDtpEndDate->GetTime(st);
	int nEndDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	for (int i = 0; i < m_TradeSysInfoVec.size(); ++i)
	{
		BOOL bChecked = m_pListTradeSys->GetCheckState(i);
		if (bChecked)
		{
			auto &dataVec = m_TradeResVec[i];
			BOOL bToday = m_pChkCondToday->IsChecked();
			BOOL bOpen = m_pChkCondOpen->IsChecked();
			for (int j = 0; j < dataVec.size(); ++j)
			{
				if (m_StockInfo.hash.count(dataVec[j].SecurityID) == 0)
					continue;
				//判断周期
				if (m_RtShowPeriodSet.count(dataVec[j].nPeriod) == 0)
					continue;
				//判断方向
				if (m_RtShowDirSet.count(dataVec[j].nType) == 0)
					continue;
				//判断是否当日触发
				if ((dataVec[j].nEnterDate >= nStartDate&& dataVec[j].nEnterDate <= nEndDate)
					|| (dataVec[j].nExitDate >= nStartDate&& dataVec[j].nExitDate <= nEndDate))
					m_ShowDataVec.emplace_back(ChangeResToShow(dataVec[j]));
			}
		}
	}
	SortShowData();

	nTotalPage = m_ShowDataVec.size() / nPrePageCount;
	if (m_ShowDataVec.size() % nPrePageCount != 0)
		++nTotalPage;
	SStringW str;
	m_pTxtTotalPage->SetWindowTextW(str.Format(L"/%d页", max(1, nTotalPage)));

}

void SOUI::CDlgTradeSysFilter::UpdateHisData()
{
	//m_pListRes->DeleteAllItems();
	//std::lock_guard<std::mutex> lk(m_mxHis);
	if (nNowPage >= nTotalPage)
	{
		nNowPage = nTotalPage - 1;
		SStringW str;
		m_pEditNowPage->SetWindowTextW(str.Format(L"%d", max(1, nTotalPage)));
	}
	if (nNowPage < 0)
		nNowPage = 0;
	int nStartPos = nNowPage * nPrePageCount;
	int nEndPos = min(nStartPos + nPrePageCount, m_ShowDataVec.size());
	int nItemCount = nEndPos - nStartPos;
	int nNowItemCount = m_pListRes->GetItemCount();
	if (nNowItemCount < nItemCount)
	{
		for (int i = nNowItemCount; i < nItemCount; ++i)
			m_pListRes->InsertItem(i, L"");
	}
	else if (nNowItemCount > nItemCount)
	{
		for (int i = nNowItemCount - 1; i >= nItemCount; --i)
			m_pListRes->DeleteItem(i);
	}

	for (int i = nStartPos; i < nEndPos; ++i)
	{
		//auto PosInfo = m_ShowDataVec[i];
		//UpdateListData(i, m_HisTradeResVec[PosInfo.first][PosInfo.second]);
		UpdateListData(i-nStartPos,i, m_ShowDataVec[i]);

	}
	m_pListRes->RequestRelayout();
	m_pListRes->Invalidate();
}





void CDlgTradeSysFilter::InitTradeSysName()
{
	CIniFile ini(".\\config\\TradSys.ini");
	int nCount = ini.GetIntA("Overall", "Count", 0);
	SStringA strKey;
	for (int i = 0; i < nCount; ++i)
	{
		strKey.Format("%d", i);
		SStringA strName = ini.GetStringA("Overall", strKey, "");
		if (!strName.IsEmpty())
		{
			TradeSysInfo tsi = { "" };
			tsi.strName = strName;
			tsi.strSimpleName = ini.GetStringA(strName, "SimpleName", "");
			tsi.strShowName = ini.GetStringA(strName, "ShowName", "");
			SStringA strPeriod = ini.GetStringA(strName, "Period", "");
			SStringA strTmpPeriod = "";
			for (int j = 0; j < strTmpPeriod.GetLength(); ++j)
			{
				if (strTmpPeriod[j] != ',')
					strTmpPeriod += strTmpPeriod[j];
				else
				{
					int nPeriod = atoi(strTmpPeriod);
					tsi.Period.insert(nPeriod);
					strTmpPeriod.Empty();
				}
			}
			if (!strTmpPeriod.IsEmpty())
			{
				int nPeriod = atoi(strTmpPeriod);
				tsi.Period.insert(nPeriod);
			}
			m_TradeSys[tsi.strSimpleName] = m_TradeSys.size();
			m_TradeSysInfoVec.emplace_back(tsi);
		}
	}
}

void SOUI::CDlgTradeSysFilter::InitControls()
{
	m_pListRes = FindChildByName2<SColorListCtrlEx>(L"ls_res");
	m_pListRes->SetMouseWheelEnabled(FALSE);
	m_pListRes->GetEventSet()->subscribeEvent(EventLCMouseWheel::EventID,
		Subscriber(&CDlgTradeSysFilter::OnListResMouseWheel, this));
	m_pListRes->GetEventSet()->subscribeEvent(EventLCRClick::EventID,
		Subscriber(&CDlgTradeSysFilter::OnListResRClick, this));

	SHeaderCtrlEx * pHeader =
		(SHeaderCtrlEx *)m_pListRes->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(3);
	pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,
		Subscriber(&CDlgTradeSysFilter::OnListResHeaderClick, this));

	m_pListTradeSys = FindChildByName2<SColorListCtrlEx>(L"ls_filter");
	m_pListTradeSys->GetEventSet()->subscribeEvent(EventLCSelChanged::EventID,
		Subscriber(&CDlgTradeSysFilter::OnListTradeSysSelChanged, this));

	m_pChkPeriodAll = FindChildByName2<SCheckBox>(L"chk_allperiod");
	m_pChkPeriodDay = FindChildByName2<SCheckBox>(L"chk_day");
	m_pChkPeriod1Min = FindChildByName2<SCheckBox>(L"chk_1min");
	m_pChkPeriod5Min = FindChildByName2<SCheckBox>(L"chk_5min");
	m_pChkPeriod15Min = FindChildByName2<SCheckBox>(L"chk_15min");
	m_pChkPeriod30Min = FindChildByName2<SCheckBox>(L"chk_30min");
	m_pChkPeriod60Min = FindChildByName2<SCheckBox>(L"chk_60min");
	m_ChkPeriodMap = { { m_pChkPeriodDay,Period_1Day },
	{ m_pChkPeriod1Min ,Period_1Min},{ m_pChkPeriod5Min ,Period_5Min },{ m_pChkPeriod15Min ,Period_15Min },
	{ m_pChkPeriod30Min ,Period_30Min } ,{ m_pChkPeriod60Min ,Period_60Min } };

	m_pChkPeriodAll->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkAllPeriodClick, this));

	m_pChkPeriodDay->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));
	m_pChkPeriod1Min->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));
	m_pChkPeriod5Min->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));
	m_pChkPeriod15Min->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));
	m_pChkPeriod30Min->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));
	m_pChkPeriod60Min->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkPeriodClick, this));


	for (int i = 0; i < m_TradeSysInfoVec.size(); ++i)
	{
		SStringW strTmp;
		m_pListTradeSys->InsertItem(i, strTmp.Format(L"%d", i));
		m_pListTradeSys->SetSubItemText(i, 1, StrA2StrW(m_TradeSysInfoVec[i].strShowName));
		strTmp.Empty();
		for (auto &it : m_TradeSysInfoVec[i].Period)
		{
			strTmp += periodNameMap[it];
			strTmp += '|';
		}
		strTmp = strTmp.Left(strTmp.GetLength() - 1);
		m_pListTradeSys->SetSubItemText(i, 2, strTmp);
	}

	m_pChkDirAll = FindChildByName2<SCheckBox>(L"chk_alldir");
	m_pChkDirLong = FindChildByName2<SCheckBox>(L"chk_long");
	m_pChkDirShort = FindChildByName2<SCheckBox>(L"chk_short");
	m_pChkDirAll->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkAllDirClick, this));
	m_pChkDirLong->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkDirClick, this));
	m_pChkDirShort->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkDirClick, this));

	m_pRadioRt = FindChildByName2<SRadioBox>(L"rad_rt");
	m_pRadioHis = FindChildByName2<SRadioBox>(L"rad_his");

	m_pChkCondAll = FindChildByName2<SCheckBox>(L"chk_condAll");
	m_pChkCondToday = FindChildByName2<SCheckBox>(L"chk_today");
	m_pChkCondOpen = FindChildByName2<SCheckBox>(L"chk_open");
	m_pChkCondAll->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkAllCondClick, this));
	m_pChkCondToday->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkCondClick, this));
	m_pChkCondOpen->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
		Subscriber(&CDlgTradeSysFilter::OnChkCondClick, this));


	m_pTxtStartDate = FindChildByName2<SStatic>(L"txt_start");
	m_pDtpStartDate = FindChildByName2<SDateTimePicker>(L"dtp_start");
	m_pTxtEndDate = FindChildByName2<SStatic>(L"txt_end");
	m_pDtpEndDate = FindChildByName2<SDateTimePicker>(L"dtp_end");

	m_pDtpStartDate->GetEventSet()->subscribeEvent(EVT_DATETIME_CHANGED,
		Subscriber(&CDlgTradeSysFilter::OnDataTimeChanged, this));
	m_pDtpEndDate->GetEventSet()->subscribeEvent(EVT_DATETIME_CHANGED,
		Subscriber(&CDlgTradeSysFilter::OnDataTimeChanged, this));

	m_pTxtTotalPage = FindChildByName2<SStatic>(L"text_totalPage");
	m_pEditNowPage = FindChildByName2<SEdit>(L"edit_page");

	m_RtShowPeriodSet = set<int>{ Period_1Day };
	m_HisShowPeriodSet = set<int>{Period_1Day };
	m_RtShowDirSet = set<int>{ eTSD_Long,eTSD_Short };
	m_HisShowDirSet = set<int>{ eTSD_Long,eTSD_Short };
}

void SOUI::CDlgTradeSysFilter::DataHandle()
{
	int MsgId;
	char *info;
	int msgLength;
	while (!m_bExit)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		switch (MsgId)
		{
		case Syn_TradeSysRes:
		{
			int nDataCount = msgLength / sizeof(TradeSysRes);
			std::lock_guard<std::mutex>lk(m_mxRT);
			m_TradeResVec.clear();
			m_TradeResVec.resize(m_TradeSys.size());
			TradeSysRes* pData = (TradeSysRes*)info;
			for (int i = 0; i < nDataCount; ++i)
			{
				if (m_TradeSys.count(pData[i].TsSimple))
					m_TradeResVec[m_TradeSys[pData[i].TsSimple]].emplace_back(pData[i]);
			}
			::PostMessageW(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_UpdateRtData, NULL);
		}
		break;
		case Syn_HisTradeSysRes:
		{
			ReceiveInfoWithDate* pRecvInfo = (ReceiveInfoWithDate*)info;
			int nDataCount = pRecvInfo->srcDataSize / sizeof(TradeSysRes);
			if (nHisRecvStartDate > pRecvInfo->nStartDate)
				nHisRecvStartDate = pRecvInfo->nStartDate;
			if (nHisRecvEndDate < pRecvInfo->nEndDate)
				nHisRecvEndDate = pRecvInfo->nEndDate;
			if (nDataCount > 0)
			{
				std::lock_guard<std::mutex>lk(m_mxHis);
				TradeSysRes* pData = (TradeSysRes*)info;
				SStringA strSmpName = pData[0].TsSimple;
				if (m_TradeSys.count(strSmpName))
				{
					int nPos = m_TradeSys[strSmpName];
					for (int i = 0; i < nDataCount; ++i)
						m_HisTradeResVec[nPos].emplace_back(pData[i]);
				}
			}
			::PostMessageW(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_UpdateHisData, NULL);
		}
		break;
		case Syn_SelfSelChange:
		{
			m_SelfSelStockMap.clear();
			int nDataCount = msgLength / sizeof(SelfSelStockInfo);
			SelfSelStockInfo* pData = (SelfSelStockInfo*)info;
			for (int i = 0; i < nDataCount; ++i)
				m_SelfSelStockMap[pData[i].SecurityID] = pData[i];
			::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ChangeItemCount, NULL);

		}
		default:
			break;
		}
		delete[]info;
		info = nullptr;
	}

}

void SOUI::CDlgTradeSysFilter::UpdateListData(int nID, TradeSysRes & data)
{
	int nIndex = m_pListRes->GetItemCount();
	double fRehab = m_accRehabMap.count(data.SecurityID) ? m_accRehabMap[data.SecurityID] : 1;
	auto &stockInfo = m_StockInfo.hash[data.SecurityID];
	SStringW strTmp;
	m_pListRes->InsertItem(nIndex, strTmp.Format(L"%d", nID + 1));
	m_pListRes->SetSubItemText(nIndex, eRH_SecurityID, StrA2StrW(data.SecurityID));
	m_pListRes->SetSubItemText(nIndex, eRH_Name, StrA2StrW(stockInfo.SecurityName));
	double fLastPrice = g_WndSyn.GetStockLastPrice(data.SecurityID);
	double fPreClose = m_PreCloseMap.hash.count(data.SecurityID) ? m_PreCloseMap.hash[data.SecurityID] : 0;
	if (fLastPrice == 0) fLastPrice = fPreClose;
	if (fPreClose != 0)
	{
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (fLastPrice > fPreClose)
			cl = RGBA(255, 0, 0, 255);
		else if (fLastPrice < fPreClose)
			cl = RGBA(0, 255, 0, 255);

		m_pListRes->SetSubItemText(nIndex, eRH_LastPx, strTmp.Format(L"%.02f", fLastPrice), cl);
		m_pListRes->SetSubItemText(nIndex, eRH_ChgPct, strTmp.Format(L"%.02f", (fLastPrice - fPreClose) / fPreClose * 100), cl);
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_ChgPct, L"-");
		if (fLastPrice != 0)
			m_pListRes->SetSubItemText(nIndex, eRH_LastPx, strTmp.Format(L"%.02f", fLastPrice));
		else
			m_pListRes->SetSubItemText(nIndex, eRH_LastPx, L"-");

	}
	int nInfoPos = m_TradeSys[data.TsSimple];
	m_pListRes->SetSubItemText(nIndex, eRH_TradeSys, StrA2StrW(m_TradeSysInfoVec[nInfoPos].strShowName));
	m_pListRes->SetSubItemText(nIndex, eRH_Dir, data.nType == eTSD_Long ? L"多头" : L"空头");
	m_pListRes->SetSubItemText(nIndex, eRH_Period, periodNameMap[data.nPeriod]);
	m_pListRes->SetSubItemText(nIndex, eRH_EnterPx, strTmp.Format(L"%.03f", data.fEnterPrice / fRehab));
	m_pListRes->SetSubItemText(nIndex, eRH_EnterDate, strTmp.Format(L"%d", data.nEnterDate));
	if (data.nExitDate != 0)
	{
		m_pListRes->SetSubItemText(nIndex, eRH_EnterPx, strTmp.Format(L"%.03f", data.fExitPrice / fRehab));
		m_pListRes->SetSubItemText(nIndex, eRH_EnterDate, strTmp.Format(L"%d", data.nExitDate));
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_ExitPx, L"-");
		m_pListRes->SetSubItemText(nIndex, eRH_ExitDate, L"-");
	}
	int nExitDate = data.nExitDate;
	if (nExitDate == 0)
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		nExitDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	}
	int nPosDate = GetDaysBetween2Date(data.nEnterDate, nExitDate);
	double fExitPrice = data.fExitPrice != 0 ? data.fExitPrice : fLastPrice * fRehab;
	if (nPosDate != 0)
	{
		double fYield = (fExitPrice - data.fEnterPrice) / data.fEnterPrice * 100;
		if (data.nType == eTSD_Short)
		{
			double fInterest = data.fEnterPrice * fRatio * nPosDate / 365;
			fYield = (data.fEnterPrice - fExitPrice - fInterest) / data.fEnterPrice * 100;
		}
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (fYield > 0)
			cl = RGBA(255, 0, 0, 255);
		else if (fYield < 0)
			cl = RGBA(0, 255, 0, 255);
		m_pListRes->SetSubItemText(nIndex, eRH_PosYield, strTmp.Format(L"%.02f", fYield), cl);
		m_pListRes->SetSubItemText(nIndex, eRH_PosDate, strTmp.Format(L"%d", nPosDate));
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_PosYield, L"-");
		m_pListRes->SetSubItemText(nIndex, eRH_PosDate, L"0");
	}
}

void SOUI::CDlgTradeSysFilter::UpdateListData(int nIndex,int nID, ShowDataType & data)
{
	SStringW strTmp;
	m_pListRes->SetSubItemText(nIndex,eRH_Index, strTmp.Format(L"%d", nID + 1));
	if (m_SelfSelStockMap.count(StrW2StrA(data.strSecurityID)))
	{
		m_pListRes->SetSubItemText(nIndex, eRH_SecurityID, data.strSecurityID,RGBA(0,225,225,255));
		m_pListRes->SetSubItemText(nIndex, eRH_Name, data.strName, RGBA(0, 225, 225, 255));
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_SecurityID, data.strSecurityID,RGBA(255,255,0,255));
		m_pListRes->SetSubItemText(nIndex, eRH_Name, data.strName, RGBA(255, 255, 0, 255));
	}
	if (data.fLastPrice != 0)
	{
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (data.fChgPct > 0)
			cl = RGBA(255, 0, 0, 255);
		else if (data.fChgPct < 0)
			cl = RGBA(0, 255, 0, 255);

		m_pListRes->SetSubItemText(nIndex, eRH_LastPx, strTmp.Format(L"%.02f", data.fLastPrice), cl);
		m_pListRes->SetSubItemText(nIndex, eRH_ChgPct, strTmp.Format(L"%.02f", data.fChgPct), cl);
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_ChgPct, L"-");
		m_pListRes->SetSubItemText(nIndex, eRH_LastPx, L"-");
	}
	m_pListRes->SetSubItemText(nIndex, eRH_TradeSys, data.strSys);
	m_pListRes->SetSubItemText(nIndex, eRH_Dir, data.nDir == eTSD_Long ? L"多头" : L"空头");
	m_pListRes->SetSubItemText(nIndex, eRH_Period, periodNameMap[data.nPeriod]);
	m_pListRes->SetSubItemText(nIndex, eRH_EnterPx, strTmp.Format(L"%.03f", data.fEnterPrice));
	m_pListRes->SetSubItemText(nIndex, eRH_EnterDate, strTmp.Format(L"%d", data.nEnterDate));
	if (data.nExitDate != 0)
	{
		m_pListRes->SetSubItemText(nIndex, eRH_EnterPx, strTmp.Format(L"%.03f", data.fExitPrice));
		m_pListRes->SetSubItemText(nIndex, eRH_EnterDate, strTmp.Format(L"%d", data.nExitDate));
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_ExitPx, L"-");
		m_pListRes->SetSubItemText(nIndex, eRH_ExitDate, L"-");
	}
	if (data.nPosDay > 0)
	{
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (data.fYeild > 0)
			cl = RGBA(255, 0, 0, 255);
		else if (data.fYeild < 0)
			cl = RGBA(0, 255, 0, 255);
		m_pListRes->SetSubItemText(nIndex, eRH_PosYield, strTmp.Format(L"%.02f", data.fYeild), cl);
		m_pListRes->SetSubItemText(nIndex, eRH_PosDate, strTmp.Format(L"%d", data.nPosDay));
	}
	else
	{
		m_pListRes->SetSubItemText(nIndex, eRH_PosYield, L"-");
		m_pListRes->SetSubItemText(nIndex, eRH_PosDate, L"0");
	}
}

int SOUI::CDlgTradeSysFilter::GetDaysBetween2Date(int nBegin, int nEnd)
{
	int nMonthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	int nYear1 = nBegin / 10000;
	int nMonth1 = nBegin % 10000 / 100;
	int nDay1 = nBegin % 100;
	int nYear2 = nEnd / 10000;
	int nMonth2 = nEnd % 10000 / 100;
	int nDay2 = nEnd % 100;
	int nTotalDays = 0;
	for (int i = nYear1; i < nYear2; ++i)
	{
		if (i % 4 == 0 && (i % 100 != 0 || i % 400 == 0))
			nTotalDays += 366;
		else nTotalDays += 365;
	}
	int nPastDays1 = nDay1;
	for (int i = 0; i < nMonth1; ++i)
		nPastDays1 += nMonthDays[i];
	if (nYear1 % 4 == 0 && (nYear1 % 100 != 0 || nYear1 % 400 == 0))
		nPastDays1++;
	int nPastDays2 = nDay2;
	for (int i = 0; i < nMonth2; ++i)
		nPastDays2 += nMonthDays[i];
	if (nYear2 % 4 == 0 && (nYear2 % 100 != 0 || nYear2 % 400 == 0))
		nPastDays2++;
	nTotalDays += (nPastDays2 - nPastDays1);

	return nTotalDays;
}

void SOUI::CDlgTradeSysFilter::ChangeResLsSize()
{
	if (m_pListRes)
	{
		int nNowItemCount = m_pListRes->GetCountPerPage(FALSE);
		if (nPrePageCount != nNowItemCount)
		{
			nPrePageCount = nNowItemCount;
			nTotalPage = m_ShowDataVec.size() / nPrePageCount;
			if (m_ShowDataVec.size() % nPrePageCount != 0)
				++nTotalPage;
			SStringW str;
			m_pTxtTotalPage->SetWindowTextW(str.Format(L"/%d页", max(1, nTotalPage)));
			::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ChangeItemCount, NULL);
		}
	}
}

ShowDataType SOUI::CDlgTradeSysFilter::ChangeResToShow(TradeSysRes & data)
{
	double fRehab = m_accRehabMap.count(data.SecurityID) ? m_accRehabMap[data.SecurityID] : 1;
	auto &stockInfo = m_StockInfo.hash[data.SecurityID];
	SStringW strTmp;
	ShowDataType sdt = { L"" };
	sdt.strSecurityID = StrA2StrW(data.SecurityID);
	sdt.strName = StrA2StrW(stockInfo.SecurityName);
	sdt.fLastPrice = g_WndSyn.GetStockLastPrice(data.SecurityID);
	double fPreClose = m_PreCloseMap.hash.count(data.SecurityID) ? m_PreCloseMap.hash[data.SecurityID] : 0;
	if (sdt.fLastPrice == 0) sdt.fLastPrice = fPreClose;
	if (fPreClose != 0)
		sdt.fChgPct = (sdt.fLastPrice - fPreClose) / fPreClose * 100;
	int nInfoPos = m_TradeSys[data.TsSimple];
	sdt.strSys = StrA2StrW(m_TradeSysInfoVec[nInfoPos].strShowName);
	sdt.nPeriod = data.nPeriod;
	sdt.nDir = data.nType;
	sdt.fEnterPrice = data.fEnterPrice / fRehab;
	sdt.nEnterDate = data.nEnterDate;
	sdt.fExitPrice = data.fExitPrice / fRehab;
	sdt.nExitDate = data.nExitDate;
	int nExitDate = data.nExitDate;
	if (nExitDate == 0)
		nExitDate = nTradingDay;
	sdt.nPosDay = GetDaysBetween2Date(data.nEnterDate, nExitDate);
	double fExitPrice = data.fExitPrice != 0 ? data.fExitPrice : sdt.fLastPrice * fRehab;
	if (sdt.nPosDay > 0)
	{
		sdt.fYeild = (fExitPrice - data.fEnterPrice) / data.fEnterPrice * 100;
		if (data.nType == eTSD_Short)
		{
			double fInterest = data.fEnterPrice * fRatio * sdt.nPosDay / 365;
			sdt.fYeild = (data.fEnterPrice - fExitPrice - fInterest) / data.fEnterPrice * 100;
		}
	}
	return sdt;
}

void SOUI::CDlgTradeSysFilter::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	switch (nID)
	{
	case ASSM_AddSel:
	{
		int nSel = m_pListRes->GetSelectedItem();
		int nIndex = nPrePageCount * nNowPage + nSel;
		if (nIndex >= 0 && nIndex < m_ShowDataVec.size())
		{
			SStringA strSecurityID =StrW2StrA( m_ShowDataVec[nIndex].strSecurityID);
			g_WndSyn.AddSelfSelStock(strSecurityID);
		}
	}
	break;
	case ASSM_AddAll:
	{
		OnBtnAddAllStockToSelfSel();
	}
	break;
	}
}

bool SOUI::CDlgTradeSysFilter::OnChkAllPeriodClick(EventArgs * arg)
{
	BOOL bChecked = m_pChkPeriodAll->IsChecked();
	if (bChecked)
	{
		m_pChkPeriodDay->SetCheck(TRUE);
		m_pChkPeriod1Min->SetCheck(TRUE);
		m_pChkPeriod5Min->SetCheck(TRUE);
		m_pChkPeriod15Min->SetCheck(TRUE);
		m_pChkPeriod30Min->SetCheck(TRUE);
		m_pChkPeriod60Min->SetCheck(TRUE);
		if (m_pRadioRt->IsChecked())
			m_RtShowPeriodSet = set<int>{ Period_FenShi,Period_1Min,Period_5Min,Period_15Min,
			Period_30Min,Period_60Min,Period_1Day };
		else
			m_HisShowPeriodSet = set<int>{ Period_FenShi,Period_1Min,Period_5Min,Period_15Min,
			Period_30Min,Period_60Min,Period_1Day };

	}
	else
	{
		m_pChkPeriodDay->SetCheck(FALSE);
		m_pChkPeriod1Min->SetCheck(FALSE);
		m_pChkPeriod5Min->SetCheck(FALSE);
		m_pChkPeriod15Min->SetCheck(FALSE);
		m_pChkPeriod30Min->SetCheck(FALSE);
		m_pChkPeriod60Min->SetCheck(FALSE);
		if (m_pRadioRt->IsChecked())
			m_RtShowPeriodSet.clear();
		else
			m_HisShowPeriodSet.clear();

	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnChkPeriodClick(EventArgs * arg)
{
	SCheckBox* pChk = dynamic_cast<SCheckBox*> (arg->sender);
	BOOL bChecked = pChk->IsChecked();
	auto &ShowPeriodSet = m_pRadioRt->IsChecked() ? m_RtShowPeriodSet : m_HisShowPeriodSet;
	if (!bChecked)
	{
		ShowPeriodSet.erase(m_ChkPeriodMap[pChk]);
		m_pChkPeriodAll->SetCheck(FALSE);
	}
	else
	{
		ShowPeriodSet.insert(m_ChkPeriodMap[pChk]);
		if (ShowPeriodSet.size() == nPeriodCount)
			m_pChkPeriodAll->SetCheck(TRUE);
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnChkAllDirClick(EventArgs * arg)
{
	BOOL bChecked = m_pChkDirAll->IsChecked();
	auto &showDirSet = m_pRadioRt->IsChecked() ? m_RtShowDirSet : m_HisShowDirSet;
	if (bChecked)
	{
		m_pChkDirLong->SetCheck(TRUE);
		m_pChkDirShort->SetCheck(TRUE);
		showDirSet = set<int>{ eTSD_Long,eTSD_Short };
	}
	else
	{
		m_pChkDirLong->SetCheck(FALSE);
		m_pChkDirShort->SetCheck(FALSE);
		showDirSet.clear();
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnChkDirClick(EventArgs * arg)
{
	SCheckBox* pChk = dynamic_cast<SCheckBox*> (arg->sender);
	BOOL bChecked = pChk->IsChecked();
	auto &showDirSet = m_pRadioRt->IsChecked() ? m_RtShowDirSet : m_HisShowDirSet;
	showDirSet.clear();
	if (m_pChkDirLong->IsChecked())
		showDirSet.insert(eTSD_Long);
	if (m_pChkDirShort->IsChecked())
		showDirSet.insert(eTSD_Short);
	if (showDirSet.size() == 2)
		m_pChkDirAll->SetCheck(TRUE);
	else
		m_pChkDirAll->SetCheck(FALSE);

	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnChkAllCondClick(EventArgs * arg)
{
	BOOL bChecked = m_pChkCondAll->IsChecked();
	if (bChecked)
	{
		m_pChkCondToday->SetCheck(TRUE);
		m_pChkCondOpen->SetCheck(TRUE);
	}
	else
	{
		m_pChkCondToday->SetCheck(FALSE);
		m_pChkCondOpen->SetCheck(FALSE);
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnChkCondClick(EventArgs * arg)
{
	SCheckBox* pChk = dynamic_cast<SCheckBox*> (arg->sender);
	BOOL bChecked = pChk->IsChecked();
	if (!bChecked)
	{
		m_pChkCondAll->SetCheck(FALSE);
	}
	else
	{
		if (m_pChkCondToday->IsChecked() && m_pChkCondOpen->IsChecked())
			m_pChkCondAll->SetCheck(TRUE);
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnDataTimeChanged(EventArgs * arg)
{
	EventDateTimeChanged* evt = dynamic_cast<EventDateTimeChanged*>(arg);
	SYSTEMTIME st;
	m_pDtpStartDate->GetTime(st);
	int nStartDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_pDtpEndDate->GetTime(st);
	int nEndDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nStartDate > nEndDate)
	{
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
		return true;
	}
	if (nStartDate >= nTradingDay)
	{
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
		return true;
	}
	if (nStartDate >= nHisRecvStartDate && nEndDate <= nHisRecvEndDate)
	{
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
		return true;
	}
	if (nStartDate > nHisRecvEndDate || nEndDate < nHisRecvStartDate ||
		(nStartDate <nHisRecvStartDate &&  nEndDate > nHisRecvEndDate))
	{
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ClearRes, NULL);
		GetHisTradeSysRes(nStartDate, nEndDate, true);
	}
	else if (nStartDate < nHisRecvStartDate)
	{
		nEndDate = nHisRecvStartDate - 1;
		int nDay = nEndDate % 100;
		if (nDay == 0)
		{
			int nYear = nEndDate / 10000;
			int nMonth = nEndDate % 10000 / 100;
			nMonth -= 1;
			if (nMonth == 0)
			{
				nYear -= 1;
				nMonth = 12;
			}
			nDay = nMonthDays[nMonth - 1];
			if (nMonth == 2 && (nYear % 4 == 0 && nYear % 100 != 0 || nYear % 400 == 0))
				nDay += 1;
			nEndDate = nYear * 10000 + nMonth * 100 + nDay;
		}
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ClearRes, NULL);
		GetHisTradeSysRes(nStartDate, nEndDate, true);
	}
	else if (nEndDate > nHisRecvEndDate)
	{
		nStartDate = nHisRecvEndDate + 1;
		int nDay = nEndDate % 100;
		int nMonth = nEndDate % 10000 / 100;
		int nYear = nEndDate / 10000;
		int nTotalDays = nMonthDays[nMonth + 1];
		if (nMonth == 2 && (nYear % 4 == 0 && nYear % 100 != 0 || nYear % 400 == 0))
			nTotalDays += 1;
		if (nDay > nTotalDays)
		{
			nDay = 1;
			nMonth += 1;
			if (nMonth > 12)
			{
				nMonth = 1;
				nYear += 1;
			}

		}
		nStartDate = nYear * 10000 + nMonth * 100 + nDay;
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ClearRes, NULL);
		GetHisTradeSysRes(nStartDate, nEndDate, true);

	}
	return true;
}


bool SOUI::CDlgTradeSysFilter::OnListResHeaderClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SHeaderCtrlEx *pHeader = (SHeaderCtrlEx*)pEvt->sender;
	SHDITEM hditem;
	hditem.mask = SHDI_ORDER;
	pHeader->GetItem(pEvt->iItem, &hditem);
	SColorListCtrlEx * pList =
		(SColorListCtrlEx *)pHeader->GetParent();
	pHeader->SetItemSort(nSortCol, ST_NULL);
	if (hditem.iOrder != nSortCol)
	{
		nSortCol = hditem.iOrder;
		nSortState = SD_Less;
	}
	else
	{
		if (nSortCol != 0)
			nSortState = !nSortState;
		else
			return true;
	}
	if (hditem.iOrder != 0)
	{
		if (nSortState == SD_Less)
			pHeader->SetItemSort(nSortCol, ST_DOWN);
		else if (nSortState == SD_Greater)
			pHeader->SetItemSort(nSortCol, ST_UP);
	}
	::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_SortData, NULL);
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnListResMouseWheel(EventArgs * pEvtBase)
{
	EventLCMouseWheel * pEvt = dynamic_cast<EventLCMouseWheel *>(pEvtBase);
	if (pEvt->zDelta < 0)
		OnBtnPageDown();
	else
		OnBtnPageUp();
	return true;
}

bool SOUI::CDlgTradeSysFilter::OnListResRClick(EventArgs * arg)
{
	EventLCRClick * pEvt = dynamic_cast<EventLCRClick *>(arg);
	SMenu menu;
	menu.LoadMenuW(L"smenu:menu_addselfsel");
	CPoint pt = pEvt->pt;
	ClientToScreen(&pt);
	menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
	return true;
}

void SOUI::CDlgTradeSysFilter::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (m_pEditNowPage->IsFocused() && nChar == VK_RETURN)
		OnBtnOk();
}

void SOUI::CDlgTradeSysFilter::OnRadioDateRt()
{
	m_pChkCondAll->SetVisible(TRUE, TRUE);
	m_pChkCondToday->SetVisible(TRUE, TRUE);
	m_pChkCondOpen->SetVisible(TRUE, TRUE);

	m_pTxtStartDate->SetVisible(FALSE, TRUE);
	m_pTxtEndDate->SetVisible(FALSE, TRUE);
	m_pDtpStartDate->SetVisible(FALSE, TRUE);
	m_pDtpEndDate->SetVisible(FALSE, TRUE);
	for (auto &it : m_ChkPeriodMap)
	{
		if (m_RtShowPeriodSet.count(it.second))
			it.first->SetCheck(TRUE);
		else
			it.first->SetCheck(FALSE);
	}
	if (m_RtShowPeriodSet.size() == nPeriodCount)
		m_pChkPeriodAll->SetCheck(TRUE);
	else
		m_pChkPeriodAll->SetCheck(FALSE);

	if (m_RtShowDirSet.count(eTSD_Long))
		m_pChkDirLong->SetCheck(TRUE);
	else
		m_pChkDirLong->SetCheck(FALSE);


	if (m_RtShowDirSet.count(eTSD_Short))
		m_pChkDirShort->SetCheck(TRUE);
	else
		m_pChkDirShort->SetCheck(FALSE);

	if (m_RtShowDirSet.size() == 2)
		m_pChkDirAll->SetCheck(TRUE);
	else
		m_pChkDirAll->SetCheck(FALSE);

	UpdateRtShowDataVec();
	UpdateRtData();

}

void SOUI::CDlgTradeSysFilter::OnRadioDateHis()
{
	m_pChkCondAll->SetVisible(FALSE,TRUE);
	m_pChkCondToday->SetVisible(FALSE, TRUE);
	m_pChkCondOpen->SetVisible(FALSE, TRUE);

	m_pTxtStartDate->SetVisible(TRUE, TRUE);
	m_pTxtEndDate->SetVisible(TRUE, TRUE);
	m_pDtpStartDate->SetVisible(TRUE, TRUE);
	m_pDtpEndDate->SetVisible(TRUE, TRUE);

	for (auto &it : m_ChkPeriodMap)
	{
		if (m_HisShowPeriodSet.count(it.second))
			it.first->SetCheck(TRUE);
		else
			it.first->SetCheck(FALSE);
	}
	if (m_HisShowPeriodSet.size() == nPeriodCount)
		m_pChkPeriodAll->SetCheck(TRUE);
	else
		m_pChkPeriodAll->SetCheck(FALSE);

	if (m_HisShowDirSet.count(eTSD_Long))
		m_pChkDirLong->SetCheck(TRUE);
	else
		m_pChkDirLong->SetCheck(FALSE);


	if (m_HisShowDirSet.count(eTSD_Short))
		m_pChkDirShort->SetCheck(TRUE);
	else
		m_pChkDirShort->SetCheck(FALSE);

	if (m_HisShowDirSet.size() == 2)
		m_pChkDirAll->SetCheck(TRUE);
	else
		m_pChkDirAll->SetCheck(FALSE);

	UpdateHisShowDataVec();
	UpdateHisData();

}

void SOUI::CDlgTradeSysFilter::OnBtnPageUp()
{
	int nNewPage = max(0, nNowPage - 1);
	if (nNewPage != nNowPage)
	{
		nNowPage = nNewPage;
		SStringW strTmp;
		m_pEditNowPage->SetWindowTextW(strTmp.Format(L"%d", nNewPage + 1));
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ChangeItemCount, NULL);
	}
}

void SOUI::CDlgTradeSysFilter::OnBtnPageDown()
{
	int nNewPage = min(nNowPage + 1, nTotalPage - 1);
	if (nNewPage != nNowPage)
	{
		nNowPage = nNewPage;
		SStringW strTmp;
		m_pEditNowPage->SetWindowTextW(strTmp.Format(L"%d", nNewPage + 1));
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ChangeItemCount, NULL);
	}

}

void SOUI::CDlgTradeSysFilter::OnBtnOk()
{
	SStringW str = m_pEditNowPage->GetWindowTextW();
	int nPage = _wtof(str) - 1;
	if (nPage < 0)
		nPage = 0;
	if (nPage > nTotalPage - 1)
		nPage = nTotalPage - 1;
	if (nPage != nNowPage)
	{
		nNowPage = nPage;
		::PostMessage(m_hWnd, WM_TRADESYSRES_MSG, TSRMSG_ChangeItemCount, NULL);
	}
}

bool SOUI::CDlgTradeSysFilter::OnListTradeSysSelChanged(EventArgs * e)
{
	SendMessage(WM_TRADESYSRES_MSG, TSRMsg_ChangeShowPara, NULL);
	return true;
}

void SOUI::CDlgTradeSysFilter::GetHisTradeSysRes(int nStartDate, int nEndDate, bool bClearData)
{
	SendInfoWithDate info = { 0 };
	info.MsgType = SendType_HisTradeSysRes;
	info.StartDate = nStartDate;
	info.EndDate = nEndDate;
	if (bClearData)
	{
		m_HisTradeResVec.clear();
		for (auto& it : m_TradeSys)
			m_HisTradeResVec.emplace_back(vector<TradeSysRes>());
		nHisRecvStartDate = 99999999;
		nHisRecvEndDate = 0;
	}
	for (auto &it : m_TradeSysInfoVec)
	{
		strcpy_s(info.StockID, it.strSimpleName);
		SendMsg(m_uParWndThreadID, Syn_GetHisTradeSysRes, (char*)&info, sizeof(info));
	}
}

void SOUI::CDlgTradeSysFilter::OnBtnAddAllStockToSelfSel()
{
	std::set<SStringA> StockSet;
	auto &dataVec = m_pRadioRt->IsChecked() ? m_TradeResVec : m_HisTradeResVec;
	for (auto &it : m_ShowDataVec)
	{
		//auto& data = dataVec[it.first][it.second];
		StockSet.insert(StrW2StrA(it.strSecurityID));
	}
	if (!StockSet.empty())
		g_WndSyn.AddSelfSelStock(StockSet);
}

void SOUI::CDlgTradeSysFilter::SortShowData()
{
	std::sort(m_ShowDataVec.begin(), m_ShowDataVec.end(), [](const ShowDataType& a, const ShowDataType& b)
	{
		if (a.nEnterDate != b.nEnterDate)
			return a.nEnterDate > b.nEnterDate;
		else if (a.nExitDate != b.nExitDate)
			return a.nExitDate > b.nExitDate;
		return a.strSecurityID < b.strSecurityID;
	});

	if (nSortCol != eRH_Index)
	{
		if (IntMap.count(nSortCol))
		{
			int nOffset = IntMap[nSortCol];
			int nFlag = nSortState;
			std::sort(m_ShowDataVec.begin(), m_ShowDataVec.end(), [nOffset, nFlag](const ShowDataType& a, const ShowDataType& b)
			{
				return compareData<int>(a, b, nOffset, nFlag);
			});
		}
		else if (DoubleMap.count(nSortCol))
		{
			int nOffset = DoubleMap[nSortCol];
			int nFlag = nSortState;
			std::sort(m_ShowDataVec.begin(), m_ShowDataVec.end(), [nOffset, nFlag](const ShowDataType& a, const ShowDataType& b)
			{
				return compareData<double >(a, b, nOffset, nFlag);
			});

		}
		else if (StrMap.count(nSortCol))
		{
			int nOffset = StrMap[nSortCol];
			int nFlag = nSortState;
			std::sort(m_ShowDataVec.begin(), m_ShowDataVec.end(), [nOffset, nFlag](const ShowDataType& a, const ShowDataType& b)
			{
				return compareData<SStringW >(a, b, nOffset, nFlag);
			});
		}

	}
}


void CDlgTradeSysFilter::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

