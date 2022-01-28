// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "IniFile.h"
#include <fstream>
#include <process.h>
using std::ifstream;
using std::ofstream;

HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#define SHOWDATACOUNT 2

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_bListInited = false;
	m_pPreSelBtn[Group_SWL1] = nullptr;
	m_pPreSelBtn[Group_SWL2] = nullptr;
	m_pPreSelBtn[Group_Stock] = nullptr;
	m_pList[Group_SWL1] = nullptr;
	m_pList[Group_SWL2] = nullptr;
	m_pList[Group_Stock] = nullptr;
	//m_pSubPic[Group_SWL1] = nullptr;
	//m_pSubPic[Group_SWL2] = nullptr;
	//m_pSubPic[Group_Stock] = nullptr;
	m_pDlgKbElf = nullptr;
	m_nLastOptWnd = 1;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	InitConfig();
	m_PeriodVec.resize(6);
	m_PeriodVec[0] = 1;
	m_PeriodVec[1] = 5;
	m_PeriodVec[2] = 15;
	m_PeriodVec[3] = 30;
	m_PeriodVec[4] = 60;
	m_PeriodVec[5] = 1440;

	m_hDataThread = (HANDLE)_beginthreadex(NULL, 0, DataHandle, this, NULL, &m_DataProcThreadID);
	m_NetClinet.SetWndHandle(m_hWnd);
	if (m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
	{
		m_NetClinet.RegisterHandle(NetHandle);
		m_NetClinet.Start(m_uNetThreadID, this);
		SendInfo info;
		info.MsgType = SendType_Connect;
		strcpy(info.str, "StkMarket");
		send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	}
	WaitForSingleObject(g_hEvent, INFINITE);

	SetVectorSize();

	InitList();
	InitSubPic();
	InitShowConfig();

	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);
	//m_pDlgKbElf->SetVisible(FALSE);

	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	ifile.open(L".\\config\\MainWnd.position", std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
	}
	ifile.close();

	return 0;
}
LRESULT CMainDlg::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int Msg = (int)wp;
	if (wp == MAINMSG_UpdateList)
	{
		UpdateList();
		for (auto &it : m_pList)
			if (it->IsVisible())
				it->Invalidate();
	}
	else if (wp == MAINMSG_ShowPic)
	{
		auto info = m_pDlgKbElf->GetShowPicInfo();
		ShowSubStockPic(info.first, info.second);
	}
	else if (wp == MAINMSG_UpdatePic)
	{
		int nGroup = (int)lp;
		m_pFenShiPic[nGroup]->Invalidate();
		m_pKlinePic[nGroup]->Invalidate();
	}
	return 0;
}

void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	if (nChar == VK_ESCAPE)
	{
		SwitchPic2List(m_nLastOptWnd);
		//if (m_pSubPic[Group_SWL1]->IsFocused()) SwitchPic2List(m_pSubPic[Group_SWL1]);
		//if (m_pSubPic[Group_SWL2]->IsFocused()) SwitchPic2List(m_pSubPic[Group_SWL2]);
		//if (m_pSubPic[Group_Stock]->IsFocused()) SwitchPic2List(m_pSubPic[Group_Stock]);
	}
	else if ((nChar >= 0x30 && nChar <= 0x39) ||
		(nChar >= 0x41 && nChar <= 0x5A) ||
		(nChar >= VK_NUMPAD0&&nChar <= VK_NUMPAD9))
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hWnd, &rc);
			m_pDlgKbElf->SetWindowPos(NULL, rc.right - 320, rc.bottom - 370, 0, 0, SWP_NOSIZE);
			if (m_nLastOptWnd == Group_SWL1)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL1], &m_IndexVec, &m_SWInd1Vec);
			if (m_nLastOptWnd == Group_SWL2)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_SWL2], &m_IndexVec, &m_SWInd2Vec);
			if (m_nLastOptWnd == Group_Stock)
				m_pDlgKbElf->SetStockInfo(m_pList[Group_Stock], &m_StockVec);
			if (nChar >= VK_NUMPAD0&&nChar <= VK_NUMPAD9)
				m_pDlgKbElf->strInput = (char)(nChar - 0x30);
			else
				m_pDlgKbElf->strInput = (char)nChar;
			m_pDlgKbElf->m_pEdit->SetWindowTextW(m_pDlgKbElf->strInput);
			m_pDlgKbElf->m_pEdit->SetFocus();
			m_pDlgKbElf->m_pEdit->SetSel(-1);
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
		}

	}
}

void CMainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	InitWndRect();
	if (PointInWindow(point, m_RectMap[Group_SWL1]))
		m_nLastOptWnd = Group_SWL1;
	else if (PointInWindow(point, m_RectMap[Group_SWL2]))
		m_nLastOptWnd = Group_SWL2;
	else if (PointInWindow(point, m_RectMap[Group_Stock]))
		m_nLastOptWnd = Group_Stock;
}

void CMainDlg::SwitchPic2List(int nGroup)
{
	SColorListCtrlEx * pList = nullptr;
	m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
	m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
	pList = m_pList[nGroup];
	pList->SetVisible(TRUE, TRUE);
	pList->SetFocus();
	CRect rc = m_pFenShiPic[nGroup]->GetClientRect();
	SWindow::InvalidateRect(rc);

}

void CMainDlg::InitConfig()
{
	CIniFile ini(".//config//config.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);
}

void CMainDlg::InitWndRect()
{
	SWindow *pWindow = FindChildByName2<SWindow>(L"wnd_Ind1");
	m_RectMap[Group_SWL1] = pWindow->GetClientRect();
	pWindow = FindChildByName2<SWindow>(L"wnd_Ind2");
	m_RectMap[Group_SWL2] = pWindow->GetClientRect();
	pWindow = FindChildByName2<SWindow>(L"wnd_Ind3");
	m_RectMap[Group_Stock] = pWindow->GetClientRect();

}

void CMainDlg::InitSubPic()
{

	m_pFenShiPic[Group_SWL1] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind1");
	m_pFenShiPic[Group_SWL2] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind2");
	m_pFenShiPic[Group_Stock] = FindChildByName2<SFenShiPic>(L"fenshiPic_Ind3");

	m_pKlinePic[Group_SWL1] = FindChildByName2<SKlinePic>(L"klinePic_Ind1");
	m_pKlinePic[Group_SWL2] = FindChildByName2<SKlinePic>(L"klinePic_Ind2");
	m_pKlinePic[Group_Stock] = FindChildByName2<SKlinePic>(L"klinePic_Ind3");

	//m_pSubPic[Group_SWL1] = FindChildByName2<SSubPic>(L"subPic_Ind1");
	//m_pSubPic[Group_SWL2] = FindChildByName2<SSubPic>(L"subPic_Ind2");
	//m_pSubPic[Group_Stock] = FindChildByName2<SSubPic>(L"subPic_Ind3");

	UpdateList();

}

void CMainDlg::InitShowConfig()
{
	CIniFile ini(".//config//config.ini");
	m_PeriodArr[Group_SWL1] = ini.GetIntA("List1", "Period", Period_NULL);
	m_PeriodArr[Group_SWL2] = ini.GetIntA("List2", "Period", Period_NULL);
	m_PeriodArr[Group_Stock] = ini.GetIntA("List3", "Period", Period_NULL);
	SStringW btnName;
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		if (Period_FenShi == m_PeriodArr[i])
			btnName.Format(L"btn_Ind%dFS", i + 1);
		else if (Period_1Day == m_PeriodArr[i])
			btnName.Format(L"btn_Ind%d1Day", i + 1);
		else if (Period_NULL == m_PeriodArr[i])
			btnName.Format(L"btn_Ind%dMarket", i + 1);
		else
			btnName.Format(L"btn_Ind%dM%d", i, m_PeriodArr[i]);
		m_pPreSelBtn[i] = FindChildByName2<SImageButton>(btnName);
		m_pPreSelBtn[i]->SetAttribute(L"colorText", L"#00ffffff");
	}

}

void CMainDlg::InitList()
{

	ResetEvent(g_hEvent);
	int i = 0;
	SStringW tmp;

	m_dataNameVec.emplace_back("close");
	m_dataNameVec.emplace_back("RPS520");
	m_dataNameVec.emplace_back("MACD520");
	m_dataNameVec.emplace_back("Point520");
	m_dataNameVec.emplace_back("Rank520");
	m_dataNameVec.emplace_back("RPS2060");
	m_dataNameVec.emplace_back("MACD2060");
	m_dataNameVec.emplace_back("Point2060");
	m_dataNameVec.emplace_back("Rank2060");

	m_comDataNameVec.emplace_back("close");
	m_comDataNameVec.emplace_back("RPS520");
	m_comDataNameVec.emplace_back("MACD520");
	m_uniDataNameVec.emplace_back("Point520");
	m_uniDataNameVec.emplace_back("Rank520");
	m_comDataNameVec.emplace_back("RPS2060");
	m_comDataNameVec.emplace_back("MACD2060");
	m_uniDataNameVec.emplace_back("Point2060");
	m_uniDataNameVec.emplace_back("Rank2060");


	m_pList[Group_SWL1] = FindChildByName2<SColorListCtrlEx>(L"List_Ind1");
	m_pList[Group_SWL2] = FindChildByName2<SColorListCtrlEx>(L"List_Ind2");
	m_pList[Group_Stock] = FindChildByName2<SColorListCtrlEx>(L"List_Ind3");

	m_pList[Group_SWL1]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));
	m_pList[Group_SWL2]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));
	m_pList[Group_Stock]->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));


	//StockInfo info;
	//strcpy_s(info.SecurityID, "000001");
	//strcpy_s(info.ExchangeID, "SSE");
	//strcpy_s(info.SecurityName, "test");
	//m_IndexVec.emplace_back(info);
	for (auto &it : m_IndexVec)
	{
		tmp.Format(L"%d", i + 1);
		m_pList[Group_SWL1]->InsertItem(i, tmp);
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_ID, StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL1]->SetSubItemText(i, SHead_Name, StrA2StrW(it.SecurityName));

		m_pList[Group_SWL2]->InsertItem(i, tmp);
		m_pList[Group_SWL2]->SetSubItemText(i, SHead_ID, StrA2StrW(it.SecurityID).Left(6));
		m_pList[Group_SWL2]->SetSubItemText(i, SHead_Name, StrA2StrW(it.SecurityName));

		m_ListPosVec[Group_SWL1][it.SecurityID] = i;
		m_ListPosVec[Group_SWL2][it.SecurityID] = i;
		m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
		m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
		i++;
	}

	int nInd1 = i;
	for (auto &it : m_SWInd1Vec)
	{
		tmp.Format(L"%d", nInd1 + 1);
		m_pList[Group_SWL1]->InsertItem(nInd1, tmp);
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_ID, StrA2StrW(it.SecurityID));
		m_pList[Group_SWL1]->SetSubItemText(nInd1, SHead_Name, StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL1][it.SecurityID] = nInd1;
		m_ListInsVec[Group_SWL1].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;

		nInd1++;
	}

	int nInd2 = i;
	for (auto &it : m_SWInd2Vec)
	{
		tmp.Format(L"%d", nInd2 + 1);
		m_pList[Group_SWL2]->InsertItem(nInd2, tmp);
		m_pList[Group_SWL2]->SetSubItemText(nInd2, SHead_ID, StrA2StrW(it.SecurityID));
		m_pList[Group_SWL2]->SetSubItemText(nInd2, SHead_Name, StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_SWL2][it.SecurityID] = nInd2;
		m_ListInsVec[Group_SWL2].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
		nInd2++;
	}


	int nStock = 0;
	for (auto &it : m_StockVec)
	{
		tmp.Format(L"%d", nStock + 1);
		m_pList[Group_Stock]->InsertItem(nStock, tmp);
		m_pList[Group_Stock]->SetSubItemText(nStock, SHead_ID, StrA2StrW(it.SecurityID));
		m_pList[Group_Stock]->SetSubItemText(nStock, SHead_Name, StrA2StrW(it.SecurityName));
		m_ListPosVec[Group_Stock][it.SecurityID] = nStock;
		m_ListInsVec[Group_Stock].emplace_back(it.SecurityID);
		m_StockNameMap[it.SecurityID] = it.SecurityName;
		nStock++;
	}

	SetEvent(g_hEvent);

	SHeaderCtrlEx * pHeaderInd1 = (SHeaderCtrlEx *)m_pList[Group_SWL1]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd1->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP, Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd2 = (SHeaderCtrlEx *)m_pList[Group_SWL2]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd2->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP, Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd3 = (SHeaderCtrlEx *)m_pList[Group_Stock]->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd3->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd3->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd3->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP, Subscriber(&CMainDlg::OnListHeaderSwap, this));


	//int nColCount = m_pList->GetColumnCount();
	//SStringW ListName;
	//ListName.Format(L"ListHeader");
	//CIniFile ini(L".//config//config.ini");
	//for (int i = 0; i < nColCount; ++i)
	//{
	//	m_FacShowVec[i] = ini.GetInt(ListName, tmp.Format(L"Col%dVisible", i), 1) == 1 ? true : false;
	//	m_pHeader->SetOriItemIndex(i, ini.GetInt(ListName, tmp.Format(L"Col%dOrder", i), i));
	//}

	//for (int i = 0; i < nColCount; ++i)
	//	m_pHeader->SetItemShowVisible(i, m_FacShowVec[i]);

	m_bListInited = true;
	UpdateList();

}

bool CMainDlg::OnListHeaderClick(EventArgs * pEvtBase)
{

	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SHeaderCtrlEx *pHeader = (SHeaderCtrlEx*)pEvt->sender;
	SHDITEM hditem;
	hditem.mask = SHDI_ORDER;
	pHeader->GetItem(pEvt->iItem, &hditem);
	int nShowOrder = pHeader->GetOriItemIndex(hditem.iOrder);

	if (hditem.iOrder == 0)
		return false;

	SColorListCtrlEx * pList = (SColorListCtrlEx *)pHeader->GetParent();
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	auto & sortPara = m_SortPara[nGroup];

	pHeader->SetItemSort(sortPara.nShowCol, ST_NULL);


	if (hditem.iOrder == 1)
	{
		sortPara.nCol = hditem.iOrder;
		sortPara.nShowCol = nShowOrder;
		sortPara.nFlag = 0;
	}
	else if (nShowOrder != sortPara.nShowCol)
	{
		sortPara.nCol = hditem.iOrder;
		sortPara.nShowCol = nShowOrder;
		sortPara.nFlag = 1;

	}
	else
		sortPara.nFlag = !sortPara.nFlag;
	if (hditem.iOrder != 1)
	{
		if (sortPara.nFlag == 0)
			pHeader->SetItemSort(sortPara.nShowCol, ST_UP);
		else
			pHeader->SetItemSort(sortPara.nShowCol, ST_DOWN);

	}
	SortList((SColorListCtrlEx*)pHeader->GetParent(), true);

	return true;
}

bool CMainDlg::OnListHeaderSwap(EventArgs * pEvtBase)
{
	EventHeaderItemSwap *pEvt = (EventHeaderItemSwap*)pEvtBase;
	SHeaderCtrlEx* pHead = (SHeaderCtrlEx*)pEvt->sender;
	int nColCount = pHead->GetItemCount();
	std::vector<int> Order(nColCount);
	SColorListCtrlEx * pList = (SColorListCtrlEx *)pHead->GetParent();
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	auto & sortPara = m_SortPara[nGroup];

	for (int i = 0; i < nColCount; ++i)
	{
		Order[i] = pHead->GetOriItemIndex(i);
		if (sortPara.nCol == i)
			sortPara.nShowCol = Order[i];
	}
	//CIniFile ini(L".//config//config.ini");
	//SStringW listName;
	//SStringW tmp;
	//listName.Format(L"ListHeader");
	//for (int i = 0; i < nColCount; ++i)
	//	ini.WriteInt(listName, tmp.Format(L"Col%dOrder", i), Order[i]);
	return true;
}

bool CMainDlg::OnListDbClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nGroup = GetGroupFromList(pList);
	if (nGroup == -1)
		return false;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	if (m_ListPosVec[nGroup].count(StockID) == 0)
		StockID += "I";
	m_strSubStock[nGroup] = StockID;
	ShowSubStockPic(StockID, pList);
	return true;
}


void CMainDlg::SortList(SColorListCtrlEx* pList, bool bSortCode)
{

	int nGroup = GetGroupFromList(pList);
	if (-1 == nGroup)
		return;
	auto &sortPara = m_SortPara[nGroup];

	if (!bSortCode)
	{
		if (m_SortPara[nGroup].nCol == 0
			|| m_SortPara[nGroup].nCol == 1
			|| m_SortPara[nGroup].nCol == 2)
		{
			pList->Invalidate();
			return;
		}
	}

	int colCount = pList->GetColumnCount();
	int SHead_Time = colCount - 1;
	switch (sortPara.nCol)
	{
	case SHead_ID:
		pList->SortItems(SortInt, &sortPara);
		break;
	case SHead_Name:
		pList->SortItems(SortStr, &sortPara);
		break;
	default:
		if (sortPara.nCol == SHead_LastPx || sortPara.nCol == SHead_ChangePct)
			sortPara.nDec = 2;
		else
			sortPara.nDec = 3;
		pList->SortItems(SortDouble, &sortPara);
		break;
	}
	SStringW tmp;

	m_bListInited = false;
	for (int i = 0; i < pList->GetItemCount(); i++)
	{
		tmp.Format(L"%d", i + 1);
		pList->SetSubItemText(i, 0, tmp);
		SStringA stockID = StrW2StrA(pList->GetSubItemText(i, 1));
		if (m_ListPosVec[nGroup].count(stockID) == 0)
			stockID += "I";
		m_ListPosVec[nGroup][stockID] = i;
	}
	m_bListInited = true;
}

void CMainDlg::UpdateList()
{
	if (!m_bListInited)
		return;

	SStringW tmp;
	int nGroup = -1;
	for (auto &ListMap : m_ListPosVec)
	{
		++nGroup;
		for (auto &it : ListMap)
		{
			SStringA StockID = it.first;
			auto & dataVec = m_listDataVec[nGroup][StockID];
			if (!isnan(dataVec["close"].value))
			{
				double fClose = dataVec["close"].value;
				double fPreClose = m_preCloseMap[StockID];
				COLORREF cl = RGBA(255, 255, 255, 255);
				if (fClose > fPreClose)
					cl = RGBA(255, 0, 0, 255);
				else if (fClose < fPreClose)
					cl = RGBA(0, 255, 0, 255);
				tmp.Format(L"%.2f", fClose);
				m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, tmp, cl);
				tmp.Format(L"%.2f", (fClose - fPreClose) / fPreClose *100);
				m_pList[nGroup]->SetSubItemText(it.second, SHead_ChangePct, tmp, cl);
			}
			else
			{
				m_pList[nGroup]->SetSubItemText(it.second, SHead_LastPx, L"-");
				m_pList[nGroup]->SetSubItemText(it.second, SHead_ChangePct, L"-");

			}
			for (int i = 1; i < m_dataNameVec.size(); ++i)
			{
				if (dataVec.count(m_dataNameVec[i]) && !isnan(dataVec[m_dataNameVec[i]].value))
				{
					COLORREF cl = RGBA(255, 255, 0, 255);
					if (i + SHead_ChangePct == SHead_Point520 || i + SHead_ChangePct == SHead_Point2060)
					{
						if (dataVec[m_dataNameVec[i]].value >= 80)
							cl = RGBA(255, 0, 0, 255);
						else if (dataVec[m_dataNameVec[i]].value < 60)
							cl = RGBA(0, 255, 0, 255);
						else
							cl = RGBA(255, 255, 255, 255);
					}
					else if (i + SHead_ChangePct == SHead_RPS520 || i + SHead_ChangePct == SHead_RPS2060
						|| i + SHead_ChangePct == SHead_MACD520 || i + SHead_ChangePct == SHead_MACD2060)
					{
						if (dataVec[m_dataNameVec[i]].value > 0)
							cl = RGBA(255, 0, 0, 255);
						else if (dataVec[m_dataNameVec[i]].value < 0)
							cl = RGBA(0, 255, 0, 255);
						else
							cl = RGBA(255, 255, 255, 255);
					}

					if (i + SHead_ChangePct == SHead_Rank520 || i + SHead_ChangePct == SHead_Rank2060)
						tmp.Format(L"%.0f", dataVec[m_dataNameVec[i]].value);
					else
						tmp.Format(L"%.03f", dataVec[m_dataNameVec[i]].value);
					m_pList[nGroup]->SetSubItemText(it.second, i + SHead_ChangePct, tmp, cl);
				}
				else
					m_pList[nGroup]->SetSubItemText(it.second, i + SHead_ChangePct, L"-");
			}
		}
		SortList(m_pList[nGroup]);
	}

}

int CMainDlg::SortDouble(void * para1, const void * para2, const void * para3)
{
	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);


	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;

	int nDec = pData->nDec;

	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);

	}


	if (pData->nFlag == 0)
	{
		if (str1 == L"-")
			str1 = L"100000.00";
		if (str2 == L"-")
			str2 = L"100000.00";
		double f1 = _wtof(str1);
		double f2 = _wtof(str2);
		if (f1*f2 < 0)
		{
			if (f1 - f2 > 0)
				return 1;
			else
				return -1;
		}
		else if (f1*f2 > 0)
		{
			int n1 = _wtoi(str1);
			int n2 = _wtoi(str2);
			if (n1 != n2)
				return n1 - n2;
			else
			{
				str1 = str1.Right(nDec);
				str2 = str2.Right(nDec);
				if ((_wtoi(str1) - _wtoi(str2)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str1) - _wtoi(str2);
					else
						return _wtoi(str2) - _wtoi(str1);
				}
				else
				{
					const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
					const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

					SStringW Code1 = CodeItem1.strText;
					SStringW Code2 = CodeItem2.strText;
					return Code1.Compare(Code2);
				}
			}
		}
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

				SStringW Code1 = CodeItem1.strText;
				SStringW Code2 = CodeItem2.strText;
				return Code1.Compare(Code2);
			}
			else
			{
				if (f1 - f2 > 0)
					return 1;
				else
					return -1;
			}
		}
	}
	else
	{
		if (str1 == L"-")
			str1 = L"-100000.00";
		if (str2 == L"-")
			str2 = L"-100000.00";
		double f1 = _wtof(str1);
		double f2 = _wtof(str2);
		if (f1*f2 < 0)
		{
			if (f2 - f1 > 0)
				return 1;
			else
				return -1;
		}
		else if (f1*f2 > 0)
		{
			int n1 = 0;
			int	n2 = 0;
			n1 = _wtoi(str1);
			n2 = _wtoi(str2);

			if (n1 != n2)
				return n2 - n1;
			else
			{
				str1 = str1.Right(nDec);
				str2 = str2.Right(nDec);
				if ((_wtoi(str2) - _wtoi(str1)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str2) - _wtoi(str1);
					else
						return _wtoi(str1) - _wtoi(str2);
				}
				else
				{
					const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
					const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

					SStringW Code1 = CodeItem1.strText;
					SStringW Code2 = CodeItem2.strText;
					return Code1.Compare(Code2);
				}
			}

		}
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(SHead_ID);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(SHead_ID);

				SStringW Code1 = CodeItem1.strText;
				SStringW Code2 = CodeItem2.strText;
				return Code1.Compare(Code2);
			}
			else
			{
				if (f2 - f1 > 0)
					return 1;
				else
					return -1;
			}
		}
	}
}

int CMainDlg::SortInt(void * para1, const void * para2, const void * para3)
{

	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);

	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;

	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);

	}


	if (pData->nFlag == 0)
	{
		if (str1 == L"-")
			str1 = L"99999999999";
		if (str2 == L"-")
			str2 = L"99999999999";

		if (str1 == str2)
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1(CodeItem1.strText);
			SStringW Code2(CodeItem2.strText);
			return Code1.Compare(Code2);

		}

		if ((_wtoll(str1) - _wtoll(str2)) != 0)
			return _wtoll(str1) - _wtoll(str2);
		else
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1 = CodeItem1.strText;
			SStringW Code2 = CodeItem2.strText;
			return Code1.Compare(Code2);
		}
	}
	else
	{
		if (str1 == L"-")
			str1 = L"-99999999999";
		if (str2 == L"-")
			str2 = L"-99999999999";

		if (str1 == str2)
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1(CodeItem1.strText);
			SStringW Code2(CodeItem2.strText);
			return Code1.Compare(Code2);

		}

		if ((_wtoll(str2) - _wtoll(str1)) != 0)
			return _wtoll(str2) - _wtoll(str1);
		else
		{
			const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
			const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

			SStringW Code1 = CodeItem1.strText;
			SStringW Code2 = CodeItem2.strText;
			return Code1.Compare(Code2);
		}
	}
}

int CMainDlg::SortStr(void * para1, const void * para2, const void * para3)
{
	SortPara *pData = (SortPara*)para1;
	const DXLVITEMEX* pPara1 = (const DXLVITEMEX*)para2;
	const DXLVITEMEX* pPara2 = (const DXLVITEMEX*)para3;
	const DXLVSUBITEMEX subItem1 = pPara1->arSubItems->GetAt(pData->nCol);
	const DXLVSUBITEMEX subItem2 = pPara2->arSubItems->GetAt(pData->nCol);

	SStringW str1 = subItem1.strText;
	SStringW str2 = subItem2.strText;
	if (str1 == str2)
	{
		const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
		const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

		SStringW Code1 = CodeItem1.strText;
		SStringW Code2 = CodeItem2.strText;
		return Code1.Compare(Code2);
	}


	if (pData->nFlag == 0)
		return str1.Compare(str2);
	else
		return str2.Compare(str1);
}

bool CMainDlg::ReceiveData(SOCKET socket, int size, char end, char * buffer, int offset)
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

unsigned CMainDlg::NetHandle(void * para)
{
	CMainDlg *pMd = (CMainDlg*)para;
	UINT dataThreadID = pMd->m_DataProcThreadID;
	//int c = 0;
	while (true)
	{
		SOCKET netSocket = pMd->m_NetClinet.GetSocket();
		if (netSocket == INVALID_SOCKET)
			return 0;
		//sprintf_s(buffer, "连接成功了%d",++c);
		//send(netSocket, buffer, 1024, 0);
		ReceiveInfo recvInfo;
		int ret = recv(netSocket, (char*)&recvInfo, sizeof(recvInfo), 0);
		if (ret == 0)
		{
			pMd->m_NetClinet.OnConnect(NULL, NULL);
			pMd->m_NetClinet.SetState(FALSE);
			pMd->m_NetClinet.OnConnect(pMd->m_strIPAddr, pMd->m_nIPPort);
			netSocket = pMd->m_NetClinet.GetSocket();
		}

		if (SOCKET_ERROR == ret)
			return 0;

		switch (recvInfo.MsgType)
		{
		case RecvMsg_StockInfo:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			StockInfo stkInfo = { 0 };
			Sleep(10);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
			{
				int size = recvInfo.nDataSize1 / sizeof(stkInfo);
				switch (recvInfo.nDataSize2)
				{
				case StockInfo_Stock:
					pMd->m_StockVec.resize(size);
					memcpy_s(&pMd->m_StockVec[0], recvInfo.nDataSize1, buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_SWL1:
					pMd->m_SWInd1Vec.resize(size);
					memcpy_s(&pMd->m_SWInd1Vec[0], recvInfo.nDataSize1, buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_SWL2:
					pMd->m_SWInd2Vec.resize(size);
					memcpy_s(&pMd->m_SWInd2Vec[0], recvInfo.nDataSize1, buffer, recvInfo.nDataSize1);
					break;
				case StockInfo_Index:
					pMd->m_IndexVec.resize(size);
					memcpy_s(&pMd->m_IndexVec[0], recvInfo.nDataSize1, buffer, recvInfo.nDataSize1);
					break;
				default:
					break;
				}
			}
			delete[] buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_UpdateIndex:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			TimeLineData stkInfo = { 0 };
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;

		}
		break;
		case RecvMsg_TodayData:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateTodayData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
			SetEvent(g_hEvent);
		}
		break;
		case RecvMsg_HisData:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateHisData, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisPoint:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisPoint, buffer, totalSize);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_LastDayEma:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateLastDayEma, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_RTIndexMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateIndexMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_RTStockMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateStockMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_IndexMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateHisIndexMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_StockMarket:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateHisStockMarket, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_HisKline:
		{
			int totalSize = recvInfo.nDataSize1 + sizeof(recvInfo);
			char *buffer = new char[totalSize];
			memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
			int offset = sizeof(recvInfo);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer, offset))
				SendMsg(dataThreadID, UpdateHisKline, buffer, totalSize);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		case RecvMsg_CloseInfo:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
				SendMsg(dataThreadID, UpdateCloseInfo, buffer, recvInfo.nDataSize1);
			delete[]buffer;
			buffer = nullptr;
		}
		break;
		default:
			break;
		}

	}
	return 0;
}
unsigned CMainDlg::DataHandle(void * para)
{
	int MsgId;
	char *info;
	int msgLength;
	bool bExit = false;

	CMainDlg *pMd = (CMainDlg*)para;
	CDataProc& dataHandler = pMd->m_dataHandler;
	auto& ListDataVec = pMd->m_listDataVec;
	auto& ListInsVec = pMd->m_ListInsVec;
	auto& dataVec = pMd->m_dataVec;
	auto &commonDataMap = pMd->m_commonDataMap;
	auto& periodVec = pMd->m_PeriodVec;
	auto& KlineMap = pMd->m_KlineMap;
	auto& LastVolMap = pMd->m_LastVolMap;
	auto& marketGetMap = pMd->m_MarketGetMap;
	auto& klineGetMap = pMd->m_KlineGetMap;
	bool bTodayInit = false;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nTradingDay = st.wYear * 10000 + st.wMonth + st.wDay;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		switch (MsgId)
		{
		case UpdateData:
		{
			if (!bTodayInit)
				break;
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData *dataArr = (TimeLineData *)info;

			for (int i = 0; i < periodVec.size(); ++i)
			{
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];
				for (int j = 0; j < dataCount; ++j)
					dataHandler.UpdateClose(periodDataMap, dataArr[j], periodVec[i]);
				dataHandler.CalcRps(periodDataMap);
				dataHandler.RankPoint(periodDataMap, dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPoint(periodDataMap, dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPoint(periodDataMap, dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}

			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_SWL1][Period_1Day], ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL1]);
			//period = pMd->m_PeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_SWL2][Period_1Day], ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL2]);
			//period = pMd->m_PeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_Stock][Period_1Day], ListDataVec[Group_Stock],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_Stock]);

			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
		}
		break;
		case UpdateTodayData:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData *dataArr = (TimeLineData *)info;
			WaitForSingleObject(g_hEvent, INFINITE);
			for (int i = 0; i < periodVec.size(); ++i)
			{
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];

				int tick = GetTickCount();
				SStringA nowStockID = "";
				vector<CoreData> * pDataVec = nullptr;
				for (int j = 0; j < dataCount; ++j)
				{
					if (nowStockID != dataArr[j].securityID)
					{
						nowStockID = dataArr[j].securityID;
						pDataVec = &commonDataMap[Period][nowStockID]["close"];
					}
					dataHandler.UpdateClose(*pDataVec, dataArr[j], Period);
					//pDataVec->emplace_back(dataArr[i]);

				}


				dataHandler.CalcHisRps(periodDataMap);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}
			//int period = pMd->m_PeriodArr[Group_SWL1];
			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_SWL1][Period_1Day], ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL1]);
			//period = pMd->m_PeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_SWL2][Period_1Day], ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL2]);
			//period = pMd->m_PeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[Period_1Day], dataVec[Group_Stock][Period_1Day], ListDataVec[Group_Stock],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_Stock]);

			bTodayInit = true;
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
			for (auto &it : commonDataMap)
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
		break;
		case UpdateRPS:
			break;
		case UpdateHisData:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData* dataArr = (TimeLineData*)info;

			for (int i = 0; i < periodVec.size(); ++i)
			{
				int Period = periodVec[i];
				auto & periodDataMap = commonDataMap[Period];

				dataHandler.UpdateHisData(periodDataMap, dataArr, dataCount, Period);
				dataHandler.CalcHisRps(periodDataMap);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_SWL1][Period], ListInsVec[Group_SWL1]);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_SWL2][Period], ListInsVec[Group_SWL2]);
				dataHandler.RankPointHisData(periodDataMap, dataVec[Group_Stock][Period], ListInsVec[Group_Stock]);
			}
			int period = pMd->m_PeriodArr[Group_SWL1];
			dataHandler.UpdateShowData(commonDataMap[period], dataVec[Group_SWL1][period], ListDataVec[Group_SWL1],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL1]);
			period = pMd->m_PeriodArr[Group_SWL2];
			dataHandler.UpdateShowData(commonDataMap[period], dataVec[Group_SWL2][period], ListDataVec[Group_SWL2],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_SWL2]);
			period = pMd->m_PeriodArr[Group_Stock];
			dataHandler.UpdateShowData(commonDataMap[period], dataVec[Group_Stock][period], ListDataVec[Group_Stock],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[Group_Stock]);
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
			for (auto &it : commonDataMap)
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
		break;
		case UpdateSingleListData:
		{
			SColorListCtrlEx ** ppList = (SColorListCtrlEx **)info;
			int nGroup = pMd->GetGroupFromList(*ppList);
			if (-1 == nGroup)
				break;
			int period = pMd->m_PeriodArr[nGroup];
			dataHandler.UpdateShowData(commonDataMap[period], dataVec[nGroup][period], ListDataVec[nGroup],
				pMd->m_comDataNameVec, pMd->m_uniDataNameVec, ListInsVec[nGroup]);
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);
		}
		break;
		case UpdateHisPoint:
		{
			ReceiveInfo recvInfo;
			memcpy_s(&recvInfo, sizeof(recvInfo), info, sizeof(recvInfo));
			int nOffset = sizeof(recvInfo);
			int nSize520 = recvInfo.nDataSize2 / sizeof(CoreData);
			int nSize2060 = (recvInfo.nDataSize1 - recvInfo.nDataSize2) / sizeof(CoreData);
			int nGroup = recvInfo.StartDate;
			int nPeriod = recvInfo.EndDate;
			//SColorListCtrlEx * pList = nullptr;
			//if (nGroup == Group_SWL1)
			//	pList = pMd->m_pList[Group_SWL1];
			//else if (nGroup == Group_SWL2)
			//	pList = pMd->m_pList[Group_SWL2];
			//else if (nGroup == Group_Stock)
			//	pList = Group_Stock;
			pMd->m_PointGetVec[nGroup][recvInfo.InsID][nPeriod] = TRUE;
			vector<CoreData> PointVec(nSize520);
			memcpy_s(&PointVec[0], recvInfo.nDataSize2, info + nOffset, recvInfo.nDataSize2);
			nOffset += recvInfo.nDataSize2;
			auto &Point520Vec = dataVec[nGroup][nPeriod][recvInfo.InsID]["Point520"];
			Point520Vec.insert(Point520Vec.begin(), PointVec.begin(), PointVec.end());
			PointVec.resize(nSize2060);
			memcpy_s(&PointVec[0], recvInfo.nDataSize1 - recvInfo.nDataSize2, info + nOffset, recvInfo.nDataSize1 - recvInfo.nDataSize2);
			auto &Point2060Vec = dataVec[nGroup][nPeriod][recvInfo.InsID]["Point2060"];
			Point2060Vec.insert(Point2060Vec.begin(), PointVec.begin(), PointVec.end());
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdatePic, (LPARAM)nGroup);

		}
		break;
		case UpdateLastDayEma:
		{
			int dataCount = msgLength / sizeof(TimeLineData);
			TimeLineData* dataArr = (TimeLineData*)info;
			int preDataCount = dataCount / 6;
			int offset = 0;
			for (int i = 0; i < 6; ++i)
			{
				int nPeriod = periodVec[i];
				auto &comDataMap = commonDataMap[nPeriod];
				for (int j = 0; j < preDataCount; ++j)
				{
					auto &data = dataArr[offset];
					dataHandler.SetPreEMAData(comDataMap, data);
					++offset;
				}
			}

		}
		break;
		case UpdateIndexMarket:
		{
			CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
			SStringA SecurityID = pIndexData->SecurityID;
			if (marketGetMap.count(SecurityID))
				pMd->m_IdxMarketMap[SecurityID].emplace_back(*pIndexData);
			for (auto &it : periodVec)
			{
				if (pMd->m_KlineGetMap[SecurityID].count(it) == 0)
					continue;
				dataHandler.CalcKline(KlineMap[SecurityID][it], *pIndexData, it, LastVolMap[SecurityID][it]);
			}
		}
		break;
		case UpdateStockMarket:
		{
			CommonStockMarket* pStockData = (CommonStockMarket*)info;
			SStringA SecurityID = pStockData->SecurityID;
			if (marketGetMap.count(SecurityID))
				pMd->m_StkMarketMap[SecurityID].emplace_back(*pStockData);
			for (auto &it : periodVec)
			{
				if (pMd->m_KlineGetMap[SecurityID].count(it) == 0)
					continue;
				dataHandler.CalcKline(KlineMap[SecurityID][it], *pStockData, it, LastVolMap[SecurityID][it], nTradingDay);
			}
		}
		break;
		case UpdateHisIndexMarket:
		{
			int dataCount = msgLength / sizeof(CommonIndexMarket);
			CommonIndexMarket * dataArr = (CommonIndexMarket *)info;
			SStringA StockID = dataArr[0].SecurityID;
			auto &MarketVec = pMd->m_IdxMarketMap[StockID];
			MarketVec.resize(dataCount);
			memcpy_s(&MarketVec[0], msgLength, dataArr, msgLength);
			marketGetMap[StockID] = TRUE;
		}
		break;
		case UpdateHisStockMarket:
		{
			int dataCount = msgLength / sizeof(CommonStockMarket);
			CommonStockMarket * dataArr = (CommonStockMarket *)info;
			SStringA StockID = dataArr[0].SecurityID;
			auto &MarketVec = pMd->m_StkMarketMap[StockID];
			MarketVec.resize(dataCount);
			memcpy_s(&MarketVec[0], msgLength, dataArr, msgLength);
			marketGetMap[StockID] = TRUE;
		}
		break;
		case UpdateHisKline:
		{
			ReceiveInfo recvInfo;
			memcpy_s(&recvInfo, sizeof(recvInfo), info, sizeof(recvInfo));
			SStringA StockID = recvInfo.InsID;
			int nOffset = sizeof(recvInfo);
			int nSize = recvInfo.nDataSize1 / sizeof(KlineType);
			int nGroup = recvInfo.StartDate;
			int nPeriod = recvInfo.EndDate;
			pMd->m_KlineGetMap[StockID][nPeriod] = TRUE;
			auto &KlineVec = KlineMap[StockID][nPeriod];
			KlineVec.resize(nSize);
			memcpy_s(&KlineVec[0], recvInfo.nDataSize1, info + nOffset, recvInfo.nDataSize1);
			if (nGroup == Group_SWL1 || nGroup == Group_SWL2)
			{
				auto &marketVec = pMd->m_IdxMarketMap[StockID];
				auto &LastVol = LastVolMap[StockID][nPeriod];
				for (auto &it : marketVec)
					dataHandler.CalcKline(KlineVec, it, nPeriod, LastVol);
			}
			else if (nGroup == Group_Stock)
			{
				auto &marketVec = pMd->m_StkMarketMap[StockID];
				auto &LastVol = LastVolMap[StockID][nPeriod];
				for (auto &it : marketVec)
					dataHandler.CalcKline(KlineVec, it, nPeriod, LastVol, nTradingDay);
			}
			::PostMessage(pMd->m_hWnd, WM_MAIN_MSG, MAINMSG_UpdateList, 0);

		}
		break;
		case UpdateCloseInfo:
		{
			pair<char[8], double>preCloseData;
			int dataCount = msgLength / sizeof(preCloseData);
			pair<char[8], double> * dataArr = (pair<char[8], double> *)info;
			auto &closeMap = pMd->m_preCloseMap;
			closeMap.clear();
			for (int i = 0; i < dataCount; ++i)
				closeMap[dataArr[i].first] = dataArr[i].second;
		}
		break;
		case Msg_Exit:
			bExit = true;
			break;
		default:
			break;
		}
		delete[]info;
		info = nullptr;
		if (bExit)
			break;
	}
	return 0;
}

bool CMainDlg::PointInWindow(const CPoint& pt, const CRect& rc)
{
	if (pt.x >= rc.left && pt.x <= rc.right
		&&pt.y >= rc.top&&pt.y <= rc.bottom)
		return true;
	return false;
}
void CMainDlg::ShowSubStockPic(SStringA stockID, SColorListCtrlEx * pList)
{
	int nGroup = GetGroupFromList(pList);
	if (-1 == nGroup)
		return;
	SStringA ShowStockID = stockID;
	if (ShowStockID.Find('I') != -1)
		ShowStockID = ShowStockID.Left(6);
	pList->SetVisible(FALSE);
	CRect rc = pList->GetClientRect();
	if (pList->HasScrollBar(TRUE))
		rc.right += 20;
	if (pList->HasScrollBar(FALSE))
		rc.bottom += 20;

	SFenShiPic *pFenShiPic = m_pFenShiPic[nGroup];
	SKlinePic *pKlinePic = m_pKlinePic[nGroup];

	int& nPeriod = m_PeriodArr[nGroup];
	if (Period_FenShi == nPeriod || Period_NULL == nPeriod)
	{
		pFenShiPic->SetVisible(TRUE);
		nPeriod = Period_FenShi;
		pFenShiPic->SetFocus();
	}
	else
	{
		pKlinePic->SetVisible(TRUE);
		if (m_PointGetVec[nGroup][stockID].count(nPeriod) == 0)
		{
			if (pList == m_pList[Group_SWL1])
				GetHisPoint(stockID, nPeriod, Group_SWL1);
			else if (pList == m_pList[Group_SWL2])
				GetHisPoint(stockID, nPeriod, Group_SWL2);
			else if (pList == m_pList[Group_Stock])
				GetHisPoint(stockID, nPeriod, Group_Stock);
		}
		pKlinePic->SetFocus();
	}
	vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
		&m_dataVec[nGroup][nPeriod][stockID]["Point520"],
		&m_dataVec[nGroup][nPeriod][stockID]["Point2060"]
	};

	vector<BOOL> rightVec(SHOWDATACOUNT);
	for (auto &it : rightVec)
		it = TRUE;
	vector<SStringA> dataNameVec;
	dataNameVec.emplace_back("Point520");
	dataNameVec.emplace_back("Point2060");
	pFenShiPic->m_pSubPic->SetShowData(SHOWDATACOUNT, tmpDataArr, rightVec, dataNameVec, ShowStockID, m_StockNameMap[stockID]);
	pKlinePic->m_pSubPic->SetShowData(SHOWDATACOUNT, tmpDataArr, rightVec, dataNameVec, ShowStockID, m_StockNameMap[stockID]);

	SWindow::InvalidateRect(rc);
}

bool CMainDlg::GetHisData(int satrtDate, int nDay, char* msg)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_GetHisData;
	info.StartDate = satrtDate;
	info.EndDate = nDay;
	strcpy_s(info.str, msg);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CMainDlg::GetHisPoint(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_GetHisPoint;
	info.StartDate = nGroup;
	info.EndDate = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CMainDlg::GetMarket(SStringA stockID, int nGroup)
{
	SendInfo info = { 0 };
	if (nGroup != Group_Stock)
		info.MsgType = SendType_IndexMarket;
	else
		info.MsgType = SendType_StockMarket;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

bool CMainDlg::GetHisKline(SStringA stockID, int nPeriod, int nGroup)
{
	SendInfo info = { 0 };
	info.MsgType = SendType_HisPeriodKline;
	info.StartDate = nGroup;
	info.EndDate = nPeriod;
	strcpy_s(info.str, stockID);
	int ret = send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	return ret > 0;
}

int CMainDlg::GetGroupFromList(SColorListCtrlEx * pList)
{
	for (int i = Group_SWL1; i < Group_Count; ++i)
	{
		if (m_pList[i] == pList)
			return i;
	}
	return -1;
}


void CMainDlg::OnBtnInd1MarketClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1Market");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_NULL);
}

void CMainDlg::OnBtnInd1FenShiClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1FS");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_FenShi);
}

void CMainDlg::OnBtnInd1M1Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1M1");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_1Min);
}

void CMainDlg::OnBtnInd1M5Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1M5");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_5Min);
}

void CMainDlg::OnBtnInd1M15Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1M15");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_15Min);

}

void CMainDlg::OnBtnInd1M30Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1M30");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_30Min);

}
void CMainDlg::OnBtnInd1M60Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1M60");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_60Min);
}

void CMainDlg::OnBtnInd1DayClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind1Day");
	if (pBtn == m_pPreSelBtn[Group_SWL1])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL1]);
	SetSelectedPeriod(Group_SWL1, Period_1Day);
}

void CMainDlg::OnBtnInd2MarketClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2Market");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_NULL);
}

void CMainDlg::OnBtnInd2FenShiClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2FS");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_FenShi);
}

void CMainDlg::OnBtnInd2M1Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2M1");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_1Min);

}

void CMainDlg::OnBtnInd2M5Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2M5");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_5Min);

}

void CMainDlg::OnBtnInd2M15Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2M15");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_15Min);

}

void CMainDlg::OnBtnInd2M30Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2M30");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_30Min);

}

void CMainDlg::OnBtnInd2M60Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2M60");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_60Min);
}

void CMainDlg::OnBtnInd2DayClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind2Day");
	if (pBtn == m_pPreSelBtn[Group_SWL2])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_SWL2]);
	SetSelectedPeriod(Group_SWL2, Period_1Day);
}

void CMainDlg::OnBtnInd3MarketClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3Market");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_NULL);
}

void CMainDlg::OnBtnInd3FenShiClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3FS");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_FenShi);
}

void CMainDlg::OnBtnInd3M1Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3M1");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_1Min);
}

void CMainDlg::OnBtnInd3M5Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3M5");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_5Min);

}
void CMainDlg::OnBtnInd3M15Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3M15");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_15Min);

}

void CMainDlg::OnBtnInd3M30Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3M30");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_30Min);

}

void CMainDlg::OnBtnInd3M60Clicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3M60");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_60Min);
}

void CMainDlg::OnBtnInd3DayClicked()
{
	SImageButton * pBtn = FindChildByName2<SImageButton>(L"btn_Ind3Day");
	if (pBtn == m_pPreSelBtn[Group_Stock])
		return;
	SetBtnState(pBtn, &m_pPreSelBtn[Group_Stock]);
	SetSelectedPeriod(Group_Stock, Period_1Day);
}

void CMainDlg::SetSelectedPeriod(int nGroup, int nPeriod)
{
	m_PeriodArr[nGroup] = nPeriod;
	SendMsg(m_DataProcThreadID, UpdateSingleListData, (char*)&m_pList[nGroup], sizeof(m_pList[nGroup]));
	//SSubPic * pPic = m_pSubPic[nGroup];
	SStringA& StockID = m_strSubStock[nGroup];
	if (StockID == "")
	{
		for (auto &it : m_ListPosVec[nGroup])
		{
			if (it.second == 0)
			{
				StockID = it.first;
				ShowSubStockPic(StockID, m_pList[nGroup]);
				break;
			}
		}
	}

	if (m_MarketGetMap.count(StockID) == 0)
		GetMarket(StockID, nGroup);

	if (nPeriod == Period_NULL)
	{
		m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
		m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
		//m_pSubPic[nGroup]->SetVisible(FALSE, TRUE);
		m_pList[nGroup]->SetVisible(TRUE, TRUE);
		return;
	}
	else if (nPeriod == Period_FenShi)
	{
		m_pKlinePic[nGroup]->SetVisible(FALSE, TRUE);
		m_pFenShiPic[nGroup]->SetVisible(TRUE, TRUE);
		m_pList[nGroup]->SetVisible(FALSE, TRUE);

	}
	else
	{

		if (m_KlineGetMap[StockID].count(nPeriod) == 0)
			GetHisKline(StockID, nPeriod, nGroup);

		if (m_PointGetVec[nGroup][StockID].count(nPeriod) == 0)
			GetHisPoint(StockID, nPeriod, nGroup);

		vector<CoreData>* tmpDataArr[SHOWDATACOUNT] = {
			&m_dataVec[nGroup][nPeriod][StockID]["Point520"],
			&m_dataVec[nGroup][nPeriod][StockID]["Point2060"] };
		vector<BOOL> rightVec(SHOWDATACOUNT);
		for (auto &it : rightVec)
			it = TRUE;

		m_pKlinePic[nGroup]->m_pSubPic->SetOffset2Zero();
		m_pKlinePic[nGroup]->m_pSubPic->ReSetShowData(2, tmpDataArr, rightVec);
		m_pKlinePic[nGroup]->m_pSubPic->SetFocus();

		m_pKlinePic[nGroup]->SetVisible(TRUE, TRUE);
		m_pFenShiPic[nGroup]->SetVisible(FALSE, TRUE);
		m_pList[nGroup]->SetVisible(FALSE, TRUE);

	}

}

void CMainDlg::SetBtnState(SImageButton * nowBtn, SImageButton** preBtn)
{
	if (*preBtn)
		(*preBtn)->SetAttribute(L"colorText", L"#c0c0c0ff");
	nowBtn->SetAttribute(L"colorText", L"#00ffffff");
	*preBtn = nowBtn;
}

void CMainDlg::SetVectorSize()
{
	m_dataVec.resize(Group_Count);
	m_RectMap.resize(Group_Count);
	m_PointGetVec.resize(Group_Count);
	m_ListPosVec.resize(Group_Count);
	m_ListInsVec.resize(Group_Count);
	m_listDataVec.resize(Group_Count);
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	if (SMessageBox(m_hWnd, _T("确定要退出么？"), _T("警告"), MB_OKCANCEL) == IDOK)
	{
		delete m_pDlgKbElf;
		m_pDlgKbElf = nullptr;
		m_NetClinet.Stop();
		WINDOWPLACEMENT wp = { sizeof(wp) };
		SendMsg(m_DataProcThreadID, Msg_Exit, NULL, 0);
		WaitForSingleObject(m_hDataThread, INFINITE);
		::GetWindowPlacement(m_hWnd, &wp);
		std::ofstream ofile;
		ofile.open(L".\\config\\MainWnd.position", std::ios::out | std::ios::binary);
		if (ofile.is_open())
			ofile.write((char*)&wp, sizeof(wp));
		ofile.close();
		CSimpleWnd::DestroyWindow();
	}
	else
		return;
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

