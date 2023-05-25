#include "stdafx.h"
#include "Define.h"
#include<fstream>
#include<io.h>
#include <ShlObj.h>

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

int SendMsg(unsigned uThreadId, unsigned MsgType,
	const char *SendBuf, unsigned BufLen)
{
	if (uThreadId <= 0)
		return INVALID_THREADID;
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

TimePreiod & operator++(TimePreiod & tp)
{

	{
		switch (tp)
		{
		case Period_FenShi:
			tp = Period_1Min;
			break;
		case Period_1Min:
			tp = Period_5Min;
			break;
		case Period_5Min:
			tp = Period_15Min;
			break;
		case Period_15Min:
			tp = Period_30Min;
			break;
		case Period_30Min:
			tp = Period_60Min;
			break;
		case Period_60Min:
			tp = Period_1Day;
			break;
		default:
			tp = Period_End;
			break;
		}
		return tp;
	}
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

void OutputDebugStringFormat(char * fmt, ...)
{
	char tracebuf[SendBufLen * 10] = { 0 };
	va_list argp;
	int len = 0;

	va_start(argp, fmt);
	len = vsprintf(tracebuf, fmt, argp);
	va_end(argp);

	OutputDebugStringA(tracebuf);

}

HRESULT OpenFile(LPTSTR FileName, COMDLG_FILTERSPEC fileType[], size_t arrySize,
	LPCTSTR filePath)
{
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		if (filePath)
		{
			PIDLIST_ABSOLUTE pidl;
			hr = ::SHParseDisplayName(filePath, 0, &pidl, SFGAO_FOLDER, 0);
			if (SUCCEEDED(hr))
			{
				IShellItem* psi;
				hr = ::SHCreateShellItem(NULL, NULL, pidl, &psi);
				if (SUCCEEDED(hr))
				{
					pfd->SetFolder(psi);
				}
				ILFree(pidl);
			}
		}

		DWORD dwFlags;
		hr = pfd->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetFileTypes(arrySize, fileType);
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetFileTypeIndex(1);
					if (SUCCEEDED(hr))
					{

						hr = pfd->Show(NULL);
						if (SUCCEEDED(hr))
						{
							IShellItem *pSelItem;

							hr = pfd->GetResult(&pSelItem);
							if (SUCCEEDED(hr))
							{

								LPWSTR pszFilePath = NULL;
								hr = pSelItem->GetDisplayName(
									SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
								if (SUCCEEDED(hr))
								{
									wcscpy(FileName, pszFilePath);
									CoTaskMemFree(pszFilePath);
								}
								pSelItem->Release();
							}
						}
					}

				}


			}
		}
		pfd->Release();
	}
	return hr;
}

HRESULT SaveFile(LPCTSTR DefaultFileName, LPTSTR FileName,
	COMDLG_FILTERSPEC fileType[], size_t arrySize, LPCTSTR filePath)
{
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog,
		NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		if (filePath)
		{
			PIDLIST_ABSOLUTE pidl;
			hr = ::SHParseDisplayName(filePath, NULL,&pidl, SFGAO_FOLDER, NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem* psi;
				hr = ::SHCreateShellItem(NULL, NULL, pidl, &psi);
				if (SUCCEEDED(hr))
				{
					pfd->SetFolder(psi);
				}
				ILFree(pidl);
			}
		}
		DWORD dwFlags;

		hr = pfd->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetFileTypes(arrySize, fileType);
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetFileTypeIndex(1);
					if (SUCCEEDED(hr))
					{
						hr = pfd->SetFileName(DefaultFileName);
						if (SUCCEEDED(hr))
						{

							//hr=pfd->SetFolder()
							hr = pfd->Show(NULL);
							if (SUCCEEDED(hr))
							{
								IShellItem *pSelItem;

								hr = pfd->GetResult(&pSelItem);
								if (SUCCEEDED(hr))
								{

									LPWSTR pszFilePath = NULL;
									hr = pSelItem->GetDisplayName(
										SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
									if (SUCCEEDED(hr))
									{
										wcscpy(FileName, pszFilePath);
										CoTaskMemFree(pszFilePath);
									}
									pSelItem->Release();
								}
							}
						}
					}
				}

			}
		}
		pfd->Release();
	}
	return hr;
}

bool _UsedPointInfo::operator<(const _UsedPointInfo & other) const
{
	return this->overallType < other.overallType;
}

bool _UsedPointInfo::operator==(const _UsedPointInfo & other) const
{
	return other.overallType == this->overallType;
}

bool _StockFilterPara::operator<(const _StockFilterPara & other) const
{
	if (index1 != other.index1)
		return index1 < other.index1;
	if (period1 != other.period1)
		return period1 < other.period1;
	if (condition != other.condition)
		return condition < other.condition;
	if (index2 != other.index2)
		return index2 < other.index2;
	if (period2 != other.period2)
		return period2 < other.period2;
	return num < other.num;
}
