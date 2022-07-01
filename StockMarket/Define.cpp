#include "stdafx.h"
#include "Define.h"
#include<fstream>
#include<io.h>
#define SendBufLen          64

char g_strLogFile[MAX_PATH] = "";
CRITICAL_SECTION g_csLogFile;

SStringW StrA2StrW(const SStringA &sstrSrcA)
{
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(sstrSrcA), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(sstrSrcA), -1, wstr, len);
	SStringW sstrDestW = wstr;
	delete[] wstr;

	return sstrDestW;
}

SStringA StrW2StrA(const SStringW &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, str, len, NULL, NULL);
	SStringA cstrDestA = str;
	delete[] str;
	return cstrDestA;
}

int SendMsg(unsigned uThreadId, unsigned MsgType, char *SendBuf, unsigned BufLen)
{
	if (uThreadId <= 0)
		return InvalidThreadId;
	char *buf;
	if (SendBuf == NULL
		|| BufLen <= 0)
	{
		buf = new char[5]();
		strcpy(buf, "null");
	}
	else
	{
		buf = new char[BufLen]();
		memcpy(buf, SendBuf, BufLen);
	}
	int ret = PostThreadMessage(uThreadId, MsgType, (WPARAM)buf, (LPARAM)BufLen);
	return ret;
}

int	RecvMsg(int  msgQId, char** buf, int& length, int timeout)
{
	MSG l_msg;
	if (GetMessage(&l_msg, 0, 0, 0))
	{
		*buf = (char *)l_msg.wParam;
		length = (int)l_msg.lParam;
	}
	return l_msg.message;
}

bool GetFileKlineData(SStringA InsID, vector<KlineType>& dataVec, bool bIsDay)
{
	return false;
}

void InitLogFile()
{
	::InitializeCriticalSection(&g_csLogFile);
	SStringA strPath = ".//trace";
	if (_access(strPath, 0) == -1)
		CreateDirectoryA(strPath, NULL);

	SYSTEMTIME st;
	::GetLocalTime(&st);

	int preYear = st.wYear;
	int preMonth = st.wMonth;
	int preDay = st.wDay;
	if (preDay <= 7)
	{
		preMonth--;
		if (preMonth == 0)
		{
			preYear--;
			preMonth = 12;
		}
		if (preMonth == 1 || preMonth == 3 || preMonth == 5 || preMonth == 7 || preMonth == 8 || preMonth == 10 || preMonth == 12)
			preDay += 24;
		else if (preMonth == 4 || preMonth == 6 || preMonth == 9 || preMonth == 11)
			preDay += 23;
		else if (preMonth == 2)
		{
			if (preYear % 4 == 0)
				preDay += 22;
			else
				preDay += 21;
		}
	}
	else
		preDay -= 7;
	int LastDeleteDate = preYear * 10000 + preMonth * 100 + preDay;
	WIN32_FIND_DATAA FindFileData;
	strPath += "//";
	HANDLE FindHandle = FindFirstFileA(strPath + "*.log", &FindFileData);
	vector<SStringA> logVec;
	logVec.emplace_back(FindFileData.cFileName);
	while (FindNextFileA(FindHandle, &FindFileData))
		logVec.emplace_back(FindFileData.cFileName);
	for (auto fileName : logVec)
	{
		if (atoi(fileName.Mid(6, 8)) <= LastDeleteDate)
			DeleteFileA(strPath + fileName);
	}

	sprintf_s(g_strLogFile, "%strace_%04d%02d%02d.log", strPath, st.wYear, st.wMonth, st.wDay);
}

void TraceLog(char * log, ...)
{

	char tracebuf[SendBufLen * 10] = { 0 };
	va_list argp;
	int len = 0;

	va_start(argp, log);
	len = vsprintf(tracebuf, log, argp);
	va_end(argp);

	SYSTEMTIME st;
	::GetLocalTime(&st);

	SStringA strbuf;
	strbuf.Format("[%02d:%02d:%02d] %s", st.wHour, st.wMinute, st.wSecond, tracebuf);

	std::ofstream ofile;
	::EnterCriticalSection(&g_csLogFile);
	if (strlen(g_strLogFile) == 0)
	{

	}
	if (strcmp(g_strLogFile, "") != 0)
	{
		ofile.open(g_strLogFile, std::ios::app);
		ofile << strbuf.GetBuffer(1) << std::endl;
		ofile.close();
	}
	::LeaveCriticalSection(&g_csLogFile);

}
