// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "IniFile.h"
#include <fstream>

using std::ifstream;
using std::ofstream;

HANDLE g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#define SHOWDATACOUNT 2

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_bListInited = false;

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
	//m_NetClinet.SetWndHandle(m_hWnd);
	//if (m_NetClinet.OnConnect(m_strIPAddr, m_nIPPort))
	//{
	//	m_NetClinet.RegisterHandle(NetHandle);
	//	m_NetClinet.Start(m_uNetThreadID, this);
	//	SendInfo info;
	//	info.MsgType = SendType_Connect;
	//	strcpy(info.str, "StockMarket");
	//	send(m_NetClinet.GetSocket(), (char*)&info, sizeof(info), 0);
	//}
	//WaitForSingleObject(g_hEvent, INFINITE);
	InitList();
	InitFSPic();
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
	return LRESULT();
}

void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(FALSE);
	switch (nChar)
	{
	case VK_ESCAPE:
	{		
		if (m_pFSPicInd1->IsFocused()) SwitchPic2List(m_pFSPicInd1);
		if (m_pFSPicInd2->IsFocused()) SwitchPic2List(m_pFSPicInd2);
		if (m_pFSPicInd3->IsFocused()) SwitchPic2List(m_pFSPicInd3);
	}
	break;
	default:
		break;
	}
}

void CMainDlg::SwitchPic2List(SFenShiPic * pPic)
{
	SColorListCtrlEx * pList=nullptr;
	for (auto &it : m_ListPicMap)
		if (it.second == pPic)
			pList = it.first;
	pPic->SetVisible(FALSE);
	CRect rc = pPic->GetClientRect();
	pList->SetVisible(TRUE);
	pList->SetFocus();
	SWindow::InvalidateRect(rc);
}

void CMainDlg::InitConfig()
{
	CIniFile ini(L".//config//config.ini");
}

void CMainDlg::InitFSPic()
{
	m_pFSPicInd1 = FindChildByName2<SFenShiPic>(L"FSPic_Ind1");
	m_pFSPicInd2 = FindChildByName2<SFenShiPic>(L"FSPic_Ind2");
	m_pFSPicInd3 = FindChildByName2<SFenShiPic>(L"FSPic_Ind3");
	m_ListPicMap[m_pListInd1] = m_pFSPicInd1;
	m_ListPicMap[m_pListInd2] = m_pFSPicInd2;
	m_ListPicMap[m_pListInd3] = m_pFSPicInd3;
	TimeLineData data;
	strcpy_s(data.securityID, "000001");
	data.date = 20211020;
	data.time = 930;
	strcpy_s(data.dataName, "close");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 10000 / 100.0 + 3000;
		m_commonDataMap["000001"]["close"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["close"] = m_commonDataMap["000001"]["close"].back();

	strcpy_s(data.dataName, "RPS520");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 200 / 100.0;
		m_commonDataMap["000001"]["RPS520"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["RPS520"] = m_commonDataMap["000001"]["RPS520"].back();

	strcpy_s(data.dataName, "RPS2060");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 200 / 100.0;
		m_commonDataMap["000001"]["RPS2060"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["RPS2060"] = m_commonDataMap["000001"]["RPS2060"].back();

	strcpy_s(data.dataName, "Rank520");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 100;
		m_dataMap[m_pListInd1]["000001"]["Rank520"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["Rank520"] = m_dataMap[m_pListInd1]["000001"]["Rank520"].back();

	strcpy_s(data.dataName, "Rank2060");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 100;
		m_dataMap[m_pListInd1]["000001"]["Rank2060"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["Rank2060"] = m_dataMap[m_pListInd1]["000001"]["Rank2060"].back();

	strcpy_s(data.dataName, "Point520");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 10000 /100.0;
		m_dataMap[m_pListInd1]["000001"]["Point520"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["Point520"] = m_dataMap[m_pListInd1]["000001"]["Point520"].back();

	strcpy_s(data.dataName, "Point2060");
	for (int i = 0; i < 500; ++i)
	{
		data.value = rand() % 10000 / 100.0;
		m_dataMap[m_pListInd1]["000001"]["Point2060"].emplace_back(data);
	}
	m_listDataMap[m_pListInd1]["000001"]["Point2060"] = m_dataMap[m_pListInd1]["000001"]["Point2060"].back();

	UpdateList();

}

void CMainDlg::InitList()
{
	int i = 0;
	SStringW tmp;
	m_dataNameVec.emplace_back("close");
	m_dataNameVec.emplace_back("RPS520");
	m_dataNameVec.emplace_back("Rank520");
	m_dataNameVec.emplace_back("Ponit520");
	m_dataNameVec.emplace_back("RPS2060");
	m_dataNameVec.emplace_back("Rank2060");
	m_dataNameVec.emplace_back("Ponit2060");

	m_pListInd1 = FindChildByName2<SColorListCtrlEx>(L"List_Ind1");
	m_pListInd2 = FindChildByName2<SColorListCtrlEx>(L"List_Ind2");
	m_pListInd3 = FindChildByName2<SColorListCtrlEx>(L"List_Ind3");

	m_pListInd1->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));
	m_pListInd2->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));
	m_pListInd3->GetEventSet()->subscribeEvent(EVT_LC_DBCLICK, Subscriber(&CMainDlg::OnListDbClick, this));

	StockInfo info;
	strcpy_s(info.SecurityID, "000001");
	strcpy_s(info.ExchangeID, "SSE");
	strcpy_s(info.SecurityName, "test");
	m_IndexVec.emplace_back(info);
	for (auto &it : m_IndexVec)
	{
		tmp.Format(L"%d", i + 1);
		m_pListInd1->InsertItem(i, tmp);
		m_pListInd1->SetSubItemText(i, 1, StrA2StrW(it.SecurityID));
		m_pListInd1->SetSubItemText(i, 2, StrA2StrW(it.SecurityName));

		m_pListInd2->InsertItem(i, tmp);
		m_pListInd2->SetSubItemText(i, 1, StrA2StrW(it.SecurityID));
		m_pListInd2->SetSubItemText(i, 2, StrA2StrW(it.SecurityName));

		m_ListPosMap[m_pListInd1][it.SecurityID] = i;
		m_ListPosMap[m_pListInd2][it.SecurityID] = i;
		i++;
	}

	int nInd1 = i;
	for (auto &it : m_SWInd1Vec)
	{
		tmp.Format(L"%d", nInd1 + 1);
		m_pListInd1->InsertItem(nInd1, tmp);
		m_pListInd1->SetSubItemText(nInd1, 1, StrA2StrW(it.SecurityID));
		m_pListInd1->SetSubItemText(nInd1, 2, StrA2StrW(it.SecurityName));
		m_ListPosMap[m_pListInd1][it.SecurityID] = nInd1;

		nInd1++;
	}

	int nInd2 = i;
	for (auto &it : m_SWInd2Vec)
	{
		tmp.Format(L"%d", nInd2 + 1);
		m_pListInd2->InsertItem(nInd2, tmp);
		m_pListInd2->SetSubItemText(nInd2, 1, StrA2StrW(it.SecurityID));
		m_pListInd2->SetSubItemText(nInd2, 2, StrA2StrW(it.SecurityName));
		m_ListPosMap[m_pListInd2][it.SecurityID] = nInd2;
		nInd2++;
	}



	SHeaderCtrlEx * pHeaderInd1 = (SHeaderCtrlEx *)m_pListInd1->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd1->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd1->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP, Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd2 = (SHeaderCtrlEx *)m_pListInd2->GetWindow(GSW_FIRSTCHILD);
	pHeaderInd2->SetNoMoveCol(3);
	//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(&CMainDlg::OnListHeaderClick, this));
	pHeaderInd2->GetEventSet()->subscribeEvent(EVT_HEADER_ITEMSWAP, Subscriber(&CMainDlg::OnListHeaderSwap, this));

	SHeaderCtrlEx * pHeaderInd3 = (SHeaderCtrlEx *)m_pListInd3->GetWindow(GSW_FIRSTCHILD);
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

	pHeader->SetItemSort(m_SortPara.nShowCol, ST_NULL);

	if (hditem.iOrder == 1)
	{
		m_SortPara.nCol = hditem.iOrder;
		m_SortPara.nShowCol = nShowOrder;
		m_SortPara.nFlag = 0;
	}
	else if (nShowOrder != m_SortPara.nShowCol)
	{
		m_SortPara.nCol = hditem.iOrder;
		m_SortPara.nShowCol = nShowOrder;
		m_SortPara.nFlag = 1;

	}
	else
		m_SortPara.nFlag = !m_SortPara.nFlag;
	if (hditem.iOrder != 1)
	{
		if (m_SortPara.nFlag == 0)
			pHeader->SetItemSort(m_SortPara.nShowCol, ST_UP);
		else
			pHeader->SetItemSort(m_SortPara.nShowCol, ST_DOWN);

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
	for (int i = 0; i < nColCount; ++i)
	{
		Order[i] = pHead->GetOriItemIndex(i);
		if (m_SortPara.nCol == i)
			m_SortPara.nShowCol = Order[i];
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
	int nSel = pList->GetSelectedItem();
	SStringA StockID = StrW2StrA(pList->GetSubItemText(nSel, SHead_ID));
	pList->SetVisible(FALSE);
	CRect rc = pList->GetClientRect();
	if(pList->HasScrollBar(TRUE))
		rc.right += 20;
	if (pList->HasScrollBar(FALSE))
		rc.bottom += 20;
	m_ListPicMap[pList]->SetVisible(TRUE);
	vector<TimeLineData>* tmpDataArr[SHOWDATACOUNT] = {
	&m_dataMap[pList][StockID]["Point520"],&m_dataMap[pList][StockID]["Point2060"] };
	vector<BOOL> rightVec(SHOWDATACOUNT);
	for (auto &it : rightVec)
		it = TRUE;
	m_ListPicMap[pList]->SetShowData(SHOWDATACOUNT, tmpDataArr, rightVec);
	m_ListPicMap[pList]->SetFocus();
	SWindow::InvalidateRect(rc);
	return true;
}


void CMainDlg::SortList(SColorListCtrlEx* pList, bool bSortCode)
{

	if (!bSortCode)
	{
		if (m_SortPara.nCol == 0
			|| m_SortPara.nCol == 1
			|| m_SortPara.nCol == 2)
		{
			pList->Invalidate();
			return;
		}
	}

	int colCount = pList->GetColumnCount();
	int SHead_Time = colCount - 1;
	switch (m_SortPara.nCol)
	{
	case SHead_ID:
		pList->SortItems(SortInt, &m_SortPara);
		break;
	case SHead_Name:
		pList->SortItems(SortStr, &m_SortPara);
		break;
	default:
		pList->SortItems(SortDouble, &m_SortPara);
		break;
	}
	SStringW tmp;

	m_bListInited = false;
	for (int i = 0; i < pList->GetItemCount(); i++)
	{
		tmp.Format(L"%d", i + 1);
		pList->SetSubItemText(i, 0, tmp);
		m_ListPosMap[pList][StrW2StrA(pList->GetSubItemText(i, 1))] = i;
	}
	m_bListInited = true;
}

void CMainDlg::UpdateList()
{
	if (!m_bListInited)
		return;

	SStringW tmp;
	for (auto &ListMap : m_ListPosMap)
	{
		for (auto &it : ListMap.second)
		{
			SStringA StockID = it.first;
			for (int i = 0; i < m_dataNameVec.size(); ++i)
			{
				if (m_listDataMap[ListMap.first][StockID].count(m_dataNameVec[i]) && !isnan(m_listDataMap[ListMap.first][StockID][m_dataNameVec[i]].value))
				{
					COLORREF cl = RGBA(255, 0, 0, 255);
					if (m_listDataMap[ListMap.first][StockID][m_dataNameVec[i]].value < 0)
						cl = RGBA(0, 255, 0, 255);
					else if (m_listDataMap[ListMap.first][StockID][m_dataNameVec[i]].value == 0)
						cl = RGBA(255, 255, 255, 255);
					tmp.Format(L"%.02f", m_listDataMap[ListMap.first][StockID][m_dataNameVec[i]].value);
					ListMap.first->SetSubItemText(it.second, i + 3, tmp, cl);
				}
				else
					ListMap.first->SetSubItemText(it.second, i + 3, L"-");
			}
		}
		SortList(ListMap.first);
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
				str1 = str1.Right(2);
				str2 = str2.Right(2);
				if ((_wtoi(str1) - _wtoi(str2)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str1) - _wtoi(str2);
					else
						return _wtoi(str2) - _wtoi(str1);
				}
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
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

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
				str1 = str1.Right(2);
				str2 = str2.Right(2);
				if ((_wtoi(str2) - _wtoi(str1)) != 0)
				{
					if (f1 > 0)
						return _wtoi(str2) - _wtoi(str1);
					else
						return _wtoi(str1) - _wtoi(str2);
				}
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
		else
		{
			if (f1 == 0 && f2 == 0)
			{
				const DXLVSUBITEMEX CodeItem1 = pPara1->arSubItems->GetAt(1);
				const DXLVSUBITEMEX CodeItem2 = pPara2->arSubItems->GetAt(1);

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

bool CMainDlg::ReceiveData(SOCKET socket, int size, char end, char * buffer)
{
	char*p = buffer;
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
		case Msg_StockInfo:
		{
			char *buffer = new char[recvInfo.nDataSize1];
			StockInfo stkInfo = { 0 };
			Sleep(10);
			if (ReceiveData(netSocket, recvInfo.nDataSize1, '#', buffer))
			{
				int size = recvInfo.nDataSize1 / sizeof(stkInfo);
				pMd->m_StockVec.reserve(size);
				for (int i = 0; i < size; ++i)
				{
					memcpy_s(&stkInfo, sizeof(stkInfo), buffer + i * sizeof(stkInfo), sizeof(stkInfo));
					pMd->m_StockVec.emplace_back(stkInfo);
				}

			}
			delete[] buffer;
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

	CMainDlg *pMd = (CMainDlg*)para;
	while (true)
	{
		MsgId = RecvMsg(0, &info, 0, 0);
		switch (MsgId)
		{
		case UpdateData:
		{
			TimeLineData &data = *(TimeLineData*)info;
			pMd->m_dataHandler.UpdateTmData(pMd->m_commonDataMap, data);
		}
		break;
		case UpdateRPS:
			pMd->m_dataHandler.CalcRps(pMd->m_commonDataMap, pMd->m_dataMap[pMd->m_pListInd1], pMd->m_ListInsMap[pMd->m_pListInd1],
				pMd->m_listDataMap[pMd->m_pListInd1], pMd->m_dataNameVec);
			pMd->m_dataHandler.CalcRps(pMd->m_commonDataMap, pMd->m_dataMap[pMd->m_pListInd2], pMd->m_ListInsMap[pMd->m_pListInd2],
				pMd->m_listDataMap[pMd->m_pListInd2], pMd->m_dataNameVec);
			pMd->m_dataHandler.CalcRps(pMd->m_commonDataMap, pMd->m_dataMap[pMd->m_pListInd3], pMd->m_ListInsMap[pMd->m_pListInd3],
				pMd->m_listDataMap[pMd->m_pListInd3], pMd->m_dataNameVec);
			break;
		default:
			break;
		}
	}

}
//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
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

