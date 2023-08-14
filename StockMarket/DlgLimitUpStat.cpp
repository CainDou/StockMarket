#include "stdafx.h"
#include "DlgLimitUpStat.h"
#include "SColorListCtrlEx.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <direct.h>
#include <io.h>
#define BUFFERSIZE 16384 //16k

#define MAXPLATE 10

extern HWND g_MainWnd;


CDlgLimitUpStat::CDlgLimitUpStat() :SHostWnd(_T("LAYOUT:dlg_LimitUpStat"))
{
	m_bLayoutInited = FALSE;
	m_bClose = FALSE;
	m_ShowPlate = "";
}


CDlgLimitUpStat::~CDlgLimitUpStat()
{
	if (!m_bClose)
		CloseWnd();
}

BOOL CDlgLimitUpStat::OnInitDialog(EventArgs * e)
{
	m_bLayoutInited = TRUE;
	for (int i = 0; i < DAYCOUNT; ++i)
		::InitializeCriticalSection(&m_cs[i]);
	::InitializeCriticalSection(&m_csShowPlate);
	//InitWindowPos();
	for (int i = 0; i < DAYCOUNT; ++i)
	{
		SStringW strName;
		m_text[i] = FindChildByName2<SStatic>(strName.Format(L"txt_day%d", i));
		m_lsPlate[i] = FindChildByName2<SColorListCtrlEx>(strName.Format(L"ls_plate%d", i));
		m_lsPlate[i]->GetEventSet()->subscribeEvent(EVT_LC_SELCHANGED,
			Subscriber(&CDlgLimitUpStat::OnListPlateLClick, this));

		m_lsLimitUpStock[i] = FindChildByName2<SColorListCtrlEx>(strName.Format(L"ls_luStock%d", i));
		m_lsLimitUpStock[i]->SetVisible(FALSE, TRUE);

	}
	InitNet();
	m_thread = std::thread(&CDlgLimitUpStat::DataGet, this);
	m_uThreadID = *(unsigned*)&m_thread.get_id();

	//GetPlateData(20230704);
	//m_dayVec.emplace_back(20230704);
	//UpdateShowPlate(0);
	//GetPlateLimitUpStock(20230704, "801123");


	return 0;
}

BOOL CDlgLimitUpStat::CloseWnd()
{
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_thread.joinable())
		m_thread.join();

	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	for (int i = 0; i < m_dayVec.size(); ++i)
	{
		for (auto&it : m_bUpdateMap)
		{
			if (it.first != nToday)
				SaveHisData(it.first);
		}
	}
	if (IsWindowVisible())
	{
		WINDOWPLACEMENT wp = { sizeof(wp) };
		::GetWindowPlacement(m_hWnd, &wp);
		std::ofstream ofile;
		ofile.open(".\\config\\LimitUp.position",
			std::ios::out | std::ios::binary);
		if (ofile.is_open())
			ofile.write((char*)&wp, sizeof(wp));
		ofile.close();
	}
	else
	{
		if (_access(".\\config\\LimitUp.position", 0) == 0)
			remove(".\\config\\LimitUp.position");
	}
	m_bClose = TRUE;

	return TRUE;
}

void CDlgLimitUpStat::InitWindowPos()
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strFileName;
	strFileName.Format(".\\config\\LimitUp.position");
	ifile.open(strFileName, std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
		ifile.close();
	}
	else
		CenterWindow(g_MainWnd);

}

void CDlgLimitUpStat::InitNet(BOOL bReinit)
{
	//LPCTSTR lpszAgent = L"WinInetGet/0.1";
	if (bReinit)
	{
		InternetCloseHandle(hConnectRT);
		InternetCloseHandle(hConnectHis);
	}
	else
	{
		hInternet = InternetOpenA(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
			INTERNET_FLAG_KEEP_CONNECTION |
			INTERNET_FLAG_NO_AUTH |
			INTERNET_FLAG_NO_COOKIES |
			INTERNET_FLAG_NO_UI |
			//设置启用HTTPS
			INTERNET_FLAG_SECURE |
			INTERNET_FLAG_RELOAD |
			INTERNET_FLAG_NO_CACHE_WRITE;

	}

	hConnectRT = InternetConnectA(hInternet, "apphq.longhuvip.com",
		INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	hConnectHis = InternetConnectA(hInternet, "apphis.longhuvip.com",
		INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
}

void CDlgLimitUpStat::InitData()
{
	set<int>SavedDaySet;
	GetHisDataList(SavedDaySet);
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	int nDayCount = 0;
	int nYear = st.wYear;
	int nMonth = st.wMonth;
	int nDay = st.wDay;
	int nDayOfWeek = st.wDayOfWeek;
	while (nDayCount < DAYCOUNT)
	{
		int nDate = nYear * 10000 + nMonth * 100 + nDay;
		int nCount = nDayCount;
		if (nDayOfWeek != 0 && nDayOfWeek != 6) //去除周六周日
		{
			BOOL bGetData = FALSE;
			if (SavedDaySet.count(nDate))
			{
				ReadHisData(nDate);
				bGetData = TRUE;
			}
			else if (GetPlateData(nCount, nDate) > 0)
				bGetData = TRUE;

			if (bGetData)
			{
				m_dayVec.emplace_back(nDate);
				::SendMessage(m_hWnd, WM_LIMITUP_MSG, LUM_UpdatePlate, nDayCount);
				++nDayCount;
			}

		}

		--nDay;
		if (nDay == 0)
		{
			nMonth--;
			if (nMonth == 0)
			{
				nYear--;
				nMonth = 12;
			}
			if (nMonth == 2)
			{
				if (nYear % 100 != 0 && nYear % 4 == 0)
					nDay = 29;
				else if (nYear % 400 == 0)
					nDay = 29;
				else
					nDay = 28;
			}
			else if (nMonth == 4 || nMonth == 6 || nMonth == 9 || nMonth == 11)
				nDay = 30;
			else
				nDay = 31;
		}
		--nDayOfWeek;
		if (nDayOfWeek < 0)
			nDayOfWeek = 6;
	}
	for (auto &it : m_dayVec)
	{
		if (SavedDaySet.count(it) == 0)
		{
			if (it != nToday)
				SaveHisData(it);
		}
	}

	SaveHisDataList(nToday);

}

void CDlgLimitUpStat::OnBtnClose()
{
	ShowWindow(SW_HIDE);
}

void CDlgLimitUpStat::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CDlgLimitUpStat::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void CDlgLimitUpStat::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CDlgLimitUpStat::OnSize(UINT nType, CSize size)
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
}

void CDlgLimitUpStat::UpdateShowPlate(int nDataPos)
{
	int nDate = m_dayVec[nDataPos];
	SStringW str;
	m_text[nDataPos]->SetWindowTextW(
		str.Format(L"%04d-%02d-%02d", nDate / 10000,
			nDate % 10000 / 100, nDate % 100));
	::EnterCriticalSection(&m_cs[nDataPos]);
	auto plateVec = m_dayPlateMap[nDate];
	::LeaveCriticalSection(&m_cs[nDataPos]);
	for (int i = 0; i < MAXPLATE; ++i)
	{
		if (m_lsPlate[nDataPos]->GetItemCount() <= i)
			m_lsPlate[nDataPos]->InsertItem(i, L"");

		if (i < plateVec.size())
		{
			m_lsPlate[nDataPos]->SetSubItemText(i, 0, plateVec[i].strPlateName, RGBA(255, 255, 255, 255));
			m_lsPlate[nDataPos]->SetSubItemText(i, 1, str.Format(L"%d", plateVec[i].nCount), RGBA(255, 255, 255, 255));
		}
	}
	int nSel = m_lsPlate[nDataPos]->GetSelectedItem();
	m_lsPlate[nDataPos]->SetUserData(nDataPos);
}

void CDlgLimitUpStat::SaveHisData(int nDate)
{
	if (_access(".\\limitUp", 0) == -1)
		_mkdir(".\\limitUp");

	SStringA strFileName;
	strFileName.Format(".\\limitUp\\day_%d.DAT", nDate);
	std::ofstream ofile(strFileName, std::ios::binary);
	if (ofile.is_open())
	{
		auto &plateVec = m_dayPlateMap[nDate];
		for (auto &plInfo : plateVec)
		{
			ofile.write((char*)&plInfo, sizeof(plInfo));
			auto &luStockVec = m_LimitUpMap[nDate][plInfo.strPlateID];
			for (auto &luStock : luStockVec)
				ofile.write((char*)&luStock, sizeof(luStock));
		}
		ofile.close();
	}

}

void CDlgLimitUpStat::ReadHisData(int nDate)
{
	SStringA strFileName;
	strFileName.Format(".\\limitUp\\day_%d.DAT", nDate);
	std::ifstream ifile(strFileName, std::ios::binary);
	if (ifile.is_open())
	{
		PlateInfo plInfo = { "" };
		while (ifile.read((char*)&plInfo, sizeof(plInfo)))
		{
			m_dayPlateMap[nDate].emplace_back(plInfo);
			auto &limitUpVec = m_LimitUpMap[nDate][plInfo.strPlateID];
			int nCount = 0;
			while (nCount < plInfo.nCount)
			{
				LimitUpStock luStock = { "" };
				ifile.read((char*)&luStock, sizeof(luStock));
				limitUpVec.emplace_back(luStock);
				++nCount;
			}
		}
		ifile.close();
	}
}

void CDlgLimitUpStat::GetHisDataList(set<int>& hisDayVec)
{
	std::ifstream ifile(".\\limitUp\\DayList.DAT", std::ios::binary);
	if (ifile.is_open())
	{
		int nDate;
		while (ifile.read((char*)&nDate, sizeof(nDate)))
			hisDayVec.insert(nDate);
		ifile.close();
	}
}

void CDlgLimitUpStat::SaveHisDataList(int nToday)
{
	std::ofstream ofile(".\\limitUp\\DayList.DAT");
	set<SStringA> valuedFileName;
	if (ofile.is_open())
	{
		for (auto &nDate : m_dayVec)
			if (nToday != nDate)
			{
				ofile.write((char*)&nDate, sizeof(nDate));
				SStringA tmp;
				tmp.Format("day_%d.DAT", nDate);
				valuedFileName.insert(tmp);
			}
		ofile.close();
	}
	WIN32_FIND_DATAA FindFileData;
	SStringA DataPath = ".\\limitUp\\";
	HANDLE FileHandle = FindFirstFileA(DataPath + "day_*.DAT", &FindFileData);
	if (valuedFileName.count(FindFileData.cFileName) == 0)
		DeleteFileA(DataPath + FindFileData.cFileName);
	while (FindNextFileA(FileHandle, &FindFileData))
	{
		if (valuedFileName.count(FindFileData.cFileName) == 0)
			DeleteFileA(DataPath + FindFileData.cFileName);
	}



}

LRESULT CDlgLimitUpStat::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nMsgId = (int)wp;
	switch (nMsgId)
	{
	case LUM_UpdatePlate:
	{
		int nDataPos = (int)lp;
		UpdateShowPlate(nDataPos);
	}
	break;
	case LUM_UpdateToday:
		UpdateShowPlate(0);
		if (m_ShowPlate != "")
			UpdateShowLimitUpStock(0, m_ShowPlate);
		break;
	case LUM_UpdateShowPlate:
	{
		SStringA strPlate = *(SStringA*)lp;
		if (strPlate == m_ShowPlate)
		{
			for (int i = 0; i < DAYCOUNT; ++i)
				UpdateShowLimitUpStock(i, strPlate);
		}
	}
	break;
	default:
		break;
	}
	return 0;
}

bool CDlgLimitUpStat::OnListPlateLClick(EventArgs * pEvtBase)
{
	EventHeaderClick *pEvt = (EventHeaderClick*)pEvtBase;
	SColorListCtrlEx *pList = (SColorListCtrlEx*)pEvt->sender;
	int nSel = pList->GetSelectedItem();
	if (nSel < 0)
		return false;
	SStringW str = pList->GetSubItemText(nSel, 0);
	int nDayPos = pList->GetUserData();
	int nDate = m_dayVec[nDayPos];
	::EnterCriticalSection(&m_cs[nDayPos]);
	SStringA strPlate = m_dayPlateMap[nDate][nSel].strPlateID;
	::LeaveCriticalSection(&m_cs[nDayPos]);
	if (strPlate != m_ShowPlate)
	{
		::EnterCriticalSection(&m_csShowPlate);
		m_ShowPlate = strPlate;
		::LeaveCriticalSection(&m_csShowPlate);
		for (int i = 0; i < DAYCOUNT; ++i)
		{
			m_lsLimitUpStock[i]->SetVisible(FALSE, TRUE);
			m_lsLimitUpStock[i]->RequestRelayout();

		}
		SendMsg(m_uThreadID, GetPlateLimitUp,
			strPlate.GetBuffer(0), strPlate.GetLength() + 1);
	}
	//for (int i = 0; i < m_dayVec.size(); ++i)
	//	UpdateShowLimitUpStock(i, strPlate);

	return true;
}

void CDlgLimitUpStat::UpdateShowLimitUpStock(int nDataPos, SStringA strPlate)
{
	int nDate = m_dayVec[nDataPos];
	::EnterCriticalSection(&m_cs[nDataPos]);
	auto plateVec = m_dayPlateMap[nDate];
	::LeaveCriticalSection(&m_cs[nDataPos]);
	int nPos = -1;
	for (int i = 0; i < plateVec.size(); ++i)
	{
		if (plateVec[i].strPlateID == strPlate)
		{
			nPos = i;
			break;
		}
	}
	nPos = nPos >= MAXPLATE ? -1 : nPos;
	m_lsPlate[nDataPos]->SetSelectedItem(nPos);
	m_lsLimitUpStock[nDataPos]->DeleteAllItems();

	::EnterCriticalSection(&m_cs[nDataPos]);
	auto plateStockMap = m_LimitUpMap[nDate];
	::LeaveCriticalSection(&m_cs[nDataPos]);

	if (plateStockMap.count(strPlate))
	{
		auto &StockVec = plateStockMap[strPlate];
		if (!StockVec.empty())
		{
			for (int i = 0; i < StockVec.size(); ++i)
			{
				m_lsLimitUpStock[nDataPos]->InsertItem(i, StockVec[i].strStockName);
				m_lsLimitUpStock[nDataPos]->SetSubItemText(i, 1, StockVec[i].strLimitUp);
			}
			m_lsLimitUpStock[nDataPos]->SetVisible(TRUE, TRUE);
		}
		/*else*/
		//{
		//	m_lsLimitUpStock[nDataPos]->SetVisible(FALSE, TRUE);
		//	m_lsLimitUpStock[nDataPos]->RequestRelayout();
		//	
		//}
	}
	//else
	//{
	//	m_lsLimitUpStock[nDataPos]->SetVisible(FALSE, TRUE);
	//	m_lsLimitUpStock[nDataPos]->RequestRelayout();

	//}

}

int CDlgLimitUpStat::GetPlateData(int nPos, int nDate)
{
	SStringA strUrl = "/w1/api/index.php?Order=1&a=RealRankingInfo&st=15&apiv=w21&Type=1&c=ZhiShuRanking&ZSType=7";
	HINTERNET hRequest;
	string tmpRes;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nDate == nToday)
	{
		hRequest = HttpOpenRequestA(hConnectRT, "GET", strUrl,
			NULL, NULL, NULL, dwOpenRequestFlags, 0);
	}
	else
	{
		strUrl.Format("%s&Date=%04d-%02d-%02d", strUrl, nDate / 10000,
			nDate % 10000 / 100, nDate % 100);
		hRequest = HttpOpenRequestA(hConnectHis, "GET", strUrl,
			NULL, NULL, NULL, dwOpenRequestFlags, 0);
	}

	BOOL bResult = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
	if (!bResult) {
		return GetLastError();
	}
	DWORD dwBytesAvailable;
	BYTE messageBody[BUFFERSIZE];
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0)) {
		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, messageBody,
			dwBytesAvailable, &dwBytesRead);
		if (!bResult) {
			return -1;
		}
		if (dwBytesRead == 0)
			break;
		tmpRes.append((char*)messageBody, dwBytesRead);
	}
	if (tmpRes.find("MYSQL数据库链接出错") == string::npos)
	{
		GetPlateDataFromStr(nPos, nDate, tmpRes);
		return 1;
	}
	else
		return 0;


}

void CDlgLimitUpStat::GetPlateDataFromStr(int nPos, int nDate, string& str)
{
	vector<PlateInfo> plVec;
	std::stringstream ss(str);
	string buffer;
	while (getline(ss, buffer, ']'))
	{
		if (buffer.empty())
			break;
		size_t size = buffer.find_last_of('[');
		buffer = buffer.substr(size + 1);
		std::stringstream bss(buffer);
		int nCount = 0;
		PlateInfo plInfo = { "" };
		while (getline(bss, buffer, ','))
		{
			size_t startPos = buffer.find('\"');
			if (startPos != string::npos)
			{
				size_t endPos = buffer.find_last_of('\"');
				buffer = buffer.substr(startPos + 1, endPos - startPos - 1);
			}

			if (nCount == 0)
				strcpy_s(plInfo.strPlateID, buffer.c_str());
			if (nCount == 1)
			{
				wcscpy_s(plInfo.strPlateName, Unescape(buffer));
				break;
			}
			++nCount;
		}
		plInfo.nCount = GetPlateLimitUpStock(nPos, nDate, plInfo.strPlateID);
		plVec.emplace_back(plInfo);
		if (plVec.size() == MAXPLATE)
			break;
	}

	::EnterCriticalSection(&m_cs[nPos]);
	m_dayPlateMap[nDate] = plVec;
	::LeaveCriticalSection(&m_cs[nPos]);

}

int CDlgLimitUpStat::GetPlateLimitUpStock(int nPos, int nDate, SStringA strPlateID)
{

	HINTERNET hRequest;
	string tmpRes;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	if (nDate == nToday)
	{
		SStringA strUrl =
			"/w1/api/index.php?Order=1&st=50&a=ZhiShuStockList_W8&c=ZhiShuRanking&old=1&apiv=w21&Type=6&PlateID=%s";
		strUrl.Format(strUrl, strPlateID);
		hRequest = HttpOpenRequestA(hConnectRT, "GET", strUrl,
			NULL, NULL, NULL, dwOpenRequestFlags, 0);
	}
	else
	{
		SStringA strUrl =
			"/w1/api/index.php?st=50&Index=0&old=1&Order=1&a=ZhiShuStockList_W8&apiv=w26&Type=6&c=ZhiShuRanking";
		strUrl.Format("%s&PlateID=%s&Date=%04d-%02d-%02d", strUrl, strPlateID,
			nDate / 10000, nDate % 10000 / 100, nDate % 100);
		hRequest = HttpOpenRequestA(hConnectHis, "GET", strUrl,
			NULL, NULL, NULL, dwOpenRequestFlags, 0);
	}

	BOOL bResult = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
	if (!bResult) {
		return GetLastError();
	}
	DWORD dwBytesAvailable;
	BYTE messageBody[BUFFERSIZE];
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0)) {
		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, messageBody,
			dwBytesAvailable, &dwBytesRead);
		if (!bResult) {
			return -1;
		}
		if (dwBytesRead == 0)
			break;
		tmpRes.append((char*)messageBody, dwBytesRead);
	}
	if (tmpRes.find("MYSQL数据库链接出错") == string::npos)
	{
		if (tmpRes.find("list") != string::npos)
		{
			size_t countPos = tmpRes.rfind("\"Count\"");
			size_t countStart = 0;
			size_t countEnd = 0;
			for (auto i = countPos; i < tmpRes.size(); ++i)
			{
				if (tmpRes[i] == ':')
					countStart = i + 1;
				else if (tmpRes[i] == ',')
				{
					countEnd = i;
					break;
				}
			}

			int nCount = countEnd > countStart ?
				atoi(tmpRes.substr(countStart, countEnd - countStart).c_str()) : 0;
			if (nCount > 0)
			{
				GetPlateLimitUpDataFromMsg(nPos, nDate, tmpRes, strPlateID);
				int nTotal = m_LimitUpMap[nDate][strPlateID].size();
				for (int i = nTotal - 1; i >= 0; --i)
				{
					if (m_LimitUpMap[nDate][strPlateID][i].nLimitUpDay == 0)
						--nTotal;
					else
						break;
				}
				return nTotal;
			}
		}
	}
	return 0;


}

void CDlgLimitUpStat::GetPlateLimitUpDataFromMsg(int nPos, int nDate, string & str, SStringA strPlateID)
{
	std::stringstream ss(str);
	string buffer;
	vector<LimitUpStock> limitUpVec;
	limitUpVec.clear();
	while (getline(ss, buffer, ']'))
	{
		if (buffer.empty())
			break;
		size_t size = buffer.find_last_of('[');
		buffer = buffer.substr(size + 1);
		std::stringstream bss(buffer);
		int nCount = 0;
		LimitUpStock luInfo = { "" };
		while (getline(bss, buffer, ','))
		{
			if (nCount > 1 && nCount < 23)
			{
				++nCount;
				continue;
			}
			size_t startPos = buffer.find('\"');
			if (startPos != string::npos)
			{
				size_t endPos = buffer.find_last_of('\"');
				buffer = buffer.substr(startPos + 1, endPos - startPos - 1);
			}

			if (nCount == 0)
				strcpy_s(luInfo.strStockID, buffer.c_str());
			if (nCount == 1)
				wcscpy_s(luInfo.strStockName, Unescape(buffer));
			if (nCount == 23)
			{
				if (buffer.empty())
					continue;
				SStringW strlimitUp = Unescape(buffer);
				if (strlimitUp.Find(L"昨日") != -1)
				{
					wcscpy_s(luInfo.strLimitUp, strlimitUp);
					luInfo.nCountDay = 1;
					luInfo.nLimitUpDay = 0;
				}
				else if (strlimitUp.Find(L"首板") != -1)
				{
					wcscpy_s(luInfo.strLimitUp, strlimitUp);
					luInfo.nCountDay = 1;
					luInfo.nLimitUpDay = 1;
				}
				else if (strlimitUp.Find(L"连板") != -1)
				{
					wcscpy_s(luInfo.strLimitUp, strlimitUp);
					luInfo.nCountDay = luInfo.nLimitUpDay = _wtoi(strlimitUp);
				}
				else if (strlimitUp.Find(L"天") != -1 &&
					strlimitUp.Find(L"板") != -1)
				{
					wcscpy_s(luInfo.strLimitUp, strlimitUp);
					luInfo.nCountDay = _wtoi(strlimitUp);
					int nLimitDayPos = strlimitUp.Find(L"天");
					luInfo.nLimitUpDay = _wtoi(strlimitUp.Right(strlimitUp.GetLength() - nLimitDayPos - 1));
					break;
				}
			}
			++nCount;
		}
		if (wcscmp(luInfo.strLimitUp, L"") != 0)
			limitUpVec.emplace_back(luInfo);
	}
	std::stable_sort(limitUpVec.begin(), limitUpVec.end(),
		[&](const LimitUpStock& data1, const LimitUpStock& data2)
	{
		if (data1.nLimitUpDay == data2.nLimitUpDay)
			return data1.nCountDay < data2.nCountDay;
		return data1.nLimitUpDay > data2.nLimitUpDay;
	});
	::EnterCriticalSection(&m_cs[nPos]);
	m_LimitUpMap[nDate][strPlateID] = limitUpVec;
	::LeaveCriticalSection(&m_cs[nPos]);

}

void CDlgLimitUpStat::DataGet()
{
	int MsgId;
	char *info;
	int msgLength;
	InitData();
	SetTimer(1, 10000);
	SetTimer(2, 60 * 5 * 1000);
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			m_uThreadID = INVALID_THREADID;
			break;
		}
		else if (GetTodayLimitUp == MsgId)
		{
			SYSTEMTIME st;
			::GetLocalTime(&st);
			int nToday = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
			int nTime = st.wHour * 100 + st.wMinute;
			if (st.wDayOfWeek != 0 && st.wDayOfWeek != 6)
			{
				if ((nTime >= 915 && nTime <= 1200) || (nTime >= 1300 && nTime <= 1530))
				{
					GetPlateData(0, nToday);
					::EnterCriticalSection(&m_csShowPlate);
					SStringA strShowPlate = m_ShowPlate;
					::LeaveCriticalSection(&m_csShowPlate);
					if (strShowPlate != "")
					{
						BOOL bHasData = FALSE;
						::EnterCriticalSection(&m_cs[0]);
						if (m_LimitUpMap[nToday].count(strShowPlate))
							bHasData = TRUE;
						::LeaveCriticalSection(&m_cs[0]);
						if (!bHasData)
						{
							PlateInfo plInfo = { "" };
							strcpy_s(plInfo.strPlateID, strShowPlate);
							plInfo.nCount = GetPlateLimitUpStock(0, nToday, strShowPlate);
						}
					}

					::SendMessage(m_hWnd, WM_LIMITUP_MSG, LUM_UpdateToday, NULL);

				}
			}

		}
		else if (GetPlateLimitUp == MsgId)
		{
			SStringA strPlateID = info;
			for (int i = 0; i < m_dayVec.size(); ++i)
			{
				int nDate = m_dayVec[i];
				auto &plateStockMap = m_LimitUpMap[nDate];
				if (plateStockMap.count(strPlateID) == 0)
				{
					PlateInfo plInfo;
					strcpy_s(plInfo.strPlateID, strPlateID);
					plInfo.nCount = GetPlateLimitUpStock(i, nDate, strPlateID);
					m_dayPlateMap[nDate].emplace_back(plInfo);
					m_bUpdateMap[nDate] = TRUE;
				}

			}
			::SendMessage(m_hWnd, WM_LIMITUP_MSG, LUM_UpdateShowPlate, (LPARAM)&strPlateID);
		}
		else if (ReInitLimitUp == MsgId)
			InitData();
		else if (limitUpReconnect == MsgId)
			InitNet(TRUE);
		delete[]info;
		info = nullptr;
	}
}

void CDlgLimitUpStat::OnTimer(char cTimerID)
{
	if (cTimerID == 1)
	{
		if (IsWindowVisible())
			SendMsg(m_uThreadID, GetTodayLimitUp, nullptr, 0);
	}
	else if (cTimerID == 2)
		SendMsg(m_uThreadID, limitUpReconnect, nullptr, 0);

}

SStringW CDlgLimitUpStat::Unescape(const string & input)
{
	SStringW result;
	for (size_t i = 0; i < input.length(); ) {
		if (input[i] == '\\' && input[i + 1] == 'u') {
			string code = input.substr(i + 2, 4);
			wchar_t unicode = stoi(code, nullptr, 16);
			result += unicode;
			i += 6;
		}
		else {
			result += input[i++];
		}
	}
	return result;
}

void CDlgLimitUpStat::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;

}
