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

HRESULT OpenFile(LPTSTR FileName)
{
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;
		if (SUCCEEDED(hr))
		{

			hr = pfd->GetOptions(&dwFlags);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
				if (SUCCEEDED(hr))
				{
					if (SUCCEEDED(hr))
					{
						COMDLG_FILTERSPEC fileType[] =
						{
							{ L"配置文件", L"*.ini" },
							{ L"所有文件",L"*.*" },
						};
						hr = pfd->SetFileTypes(ARRAYSIZE(fileType), fileType);
						if (SUCCEEDED(hr))
						{
							hr = pfd->SetFileTypeIndex(1);
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
										hr = pSelItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
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
		}
		pfd->Release();
	}
	return hr;
}

HRESULT SaveFile(LPCTSTR DefaultFileName, LPTSTR FileName)
{
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;
		if (SUCCEEDED(hr))
		{

			hr = pfd->GetOptions(&dwFlags);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
				if (SUCCEEDED(hr))
				{
					if (SUCCEEDED(hr))
					{

						COMDLG_FILTERSPEC fileType[] =
						{
							{ L"CSV(逗号分隔符)", L"*.csv" },
							{ L"所有文件",L"*.*" },
						};
						hr = pfd->SetFileTypes(ARRAYSIZE(fileType), fileType);
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
										UINT fileType = 0;
										pfd->GetFileTypeIndex(&fileType);

										if (SUCCEEDED(hr))
										{

											LPWSTR pszFilePath = NULL;
											hr = pSelItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
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
			}
		}
		pfd->Release();
	}
	return hr;
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

void GetInitPara(CIniFile & ini, InitPara & para,SStringA strSection)
{

	int CloseMAPara[] = { 5,10,20,60,0,0 };
	int VolAmoMAPara[] = { 5,10,0,0,0,0 };

	SStringA strKey;
	para.bShowMA =
		ini.GetIntA(strSection, "ShowMA", 1) == 0 ? false : true;
	para.bShowBandTarget =
		ini.GetIntA(strSection, "ShowBand") == 0 ? false : true;
	para.bShowAverage =
		ini.GetIntA(strSection, "ShowAvg", 1) == 0 ? false : true;
	para.bShowEMA =
		ini.GetIntA(strSection, "ShowEMA", 1) == 0 ? false : true;
	para.bShowTSCMACD =
		ini.GetIntA(strSection, "ShowTSCMACD", 1) == 0 ? false : true;
	para.bShowTSCVolume =
		ini.GetIntA(strSection, "ShowTSCVolume", 1) == 0 ? false : true;
	para.bShowKlineVolume =
		ini.GetIntA(strSection, "ShowKlineVolume", 1) == 0 ? false : true;
	para.bShowKlineAmount =
		ini.GetIntA(strSection, "ShowKlineAmount", 0) == 0 ? false : true;
	para.bShowKlineCAVol =
		ini.GetIntA(strSection, "ShowKlineCAVol", 0) == 0 ? false : true;
	para.bShowKlineCAAmo =
		ini.GetIntA(strSection, "ShowKlineCAAmo", 0) == 0 ? false : true;

	para.bShowKlineMACD =
		ini.GetIntA(strSection, "ShowKlineMACD", 1) == 0 ? false : true;
	para.bShowTSCRPS[0] =
		ini.GetIntA(strSection, "ShowTSCRPS", 1) == 0 ? false : true;
	para.bShowTSCRPS[1] =
		ini.GetIntA(strSection, "ShowTSCL1RPS", 1) == 0 ? false : true;
	para.bShowTSCRPS[2] =
		ini.GetIntA(strSection, "ShowTSCL2RPS", 1) == 0 ? false : true;
	para.bShowKlineRPS[0] =
		ini.GetIntA(strSection, "ShowKlineRPS", 1) == 0 ? false : true;
	para.bShowKlineRPS[1] =
		ini.GetIntA(strSection, "ShowKlineL1RPS", 1) == 0 ? false : true;
	para.bShowKlineRPS[2] =
		ini.GetIntA(strSection, "ShowKlineL2RPS", 1) == 0 ? false : true;

	para.nWidth = ini.GetIntA(strSection, "Width", 16);
	para.bShowTSCDeal =
		ini.GetIntA(strSection, "ShowTSCDeal", 1) == 0 ? false : true;
	para.bShowKlineDeal =
		ini.GetIntA(strSection, "ShowKlineDeal", 1) == 0 ? false : true;
	para.nEMAPara[0] =
		ini.GetIntA(strSection, "EMAPara1", 12);
	para.nEMAPara[1] =
		ini.GetIntA(strSection, "EMAPara2", 26);
	para.nMACDPara[0] =
		ini.GetIntA(strSection, "MACDPara1", 12);
	para.nMACDPara[1] =
		ini.GetIntA(strSection, "MACDPara2", 26);
	para.nMACDPara[2] =
		ini.GetIntA(strSection, "MACDPara3", 9);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		para.nMAPara[i] = ini.GetIntA(strSection,
			strKey.Format("MAPara%d", i + 1), CloseMAPara[i]);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		para.nVolMaPara[i] = ini.GetIntA(strSection,
			strKey.Format("VolMAPara%d", i + 1), VolAmoMAPara[i]);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		para.nAmoMaPara[i] = ini.GetIntA(strSection,
			strKey.Format("AmoMAPara%d", i + 1), VolAmoMAPara[i]);
	for (int i = 0; i<MAX_MA_COUNT; ++i)
		para.nCAVolMaPara[i] = ini.GetIntA(strSection,
			strKey.Format("CAVolMAPara%d", i + 1), VolAmoMAPara[i]);
	for (int i = 0; i<MAX_MA_COUNT; ++i)
		para.nCAAmoMaPara[i] = ini.GetIntA(strSection,
			strKey.Format("CAAmoMAPara%d", i + 1), VolAmoMAPara[i]);

	para.nJiange =
		ini.GetIntA(strSection, "Jiange", 2);
	para.BandPara.N1 =
		ini.GetIntA(strSection, "BandN1", 8);
	para.BandPara.N2 =
		ini.GetIntA(strSection, "BandN2", 11);
	para.BandPara.K =
		ini.GetIntA(strSection, "BandK", 390);
	para.BandPara.M1 =
		ini.GetIntA(strSection, "BandM1", 8);
	para.BandPara.M2 =
		ini.GetIntA(strSection, "BandM2", 4);
	para.BandPara.P =
		ini.GetIntA(strSection, "BandP", 390);

	para.Period = ini.GetIntA(strSection, "Period", Period_1Day);
	para.Connect1 = ini.GetIntA(strSection, "ListConnent1", 0)
		== 0 ? false : true;
	para.Connect2 = ini.GetIntA(strSection, "ListConnent2", 0)
		== 0 ? false : true;
	SStringA strStock = ini.GetStringA(strSection, "ShowIndustry", "");
	strcpy_s(para.ShowIndy, strStock);
	para.UseStockFilter = ini.GetIntA(strSection, "StockFilter", 0)
		== 0 ? false : true;
	para.ListShowST = ini.GetIntA(strSection, "ListShowST", 1)
		== 0 ? false : true;
	para.ListShowSBM = ini.GetIntA(strSection, "ListShowSBM", 1)
		== 0 ? false : true;
	para.ListShowSTARM = ini.GetIntA(strSection, "ListShowSTARM", 1)
		== 0 ? false : true;
	para.ListShowNewStock = ini.GetIntA(strSection, "ListShowNewSotck", 1)
		== 0 ? false : true;
	para.nKlineRehabType = ini.GetIntA(strSection, "KlineRehabType", 0);
	para.nKlineCalcRehabType = ini.GetIntA(strSection, "KlineCalcRehabType", 0);
	para.nKlineFTRehabDate = ini.GetIntA(strSection, "KlineFTRehabDate", 0);
	para.strFilterName = ini.GetStringA(strSection, "FilterName", "");
	para.UseHisStockFilter = ini.GetIntA(strSection, "HisStockFilter", 0);
	para.bKlineUseTickFlowData = ini.GetIntA(strSection, "KlineUseTickFlowData", 0);
	para.nKlineTickFlowDataType = ini.GetIntA(strSection, "KlineTickFlowDataType", 0);

}

void SaveInitPara(CIniFile & ini, InitPara & para, SStringA strSection)
{
	SStringA strKey = "";
	ini.WriteIntA(strSection, "ShowMA", para.bShowMA);
	ini.WriteIntA(strSection, "ShowBand", para.bShowBandTarget);
	ini.WriteIntA(strSection, "ShowAvg", para.bShowAverage);
	ini.WriteIntA(strSection, "ShowEMA", para.bShowEMA);
	ini.WriteIntA(strSection, "ShowTSCMACD", para.bShowTSCMACD);
	ini.WriteIntA(strSection, "ShowTSCVolume", para.bShowTSCVolume);
	ini.WriteIntA(strSection, "ShowKlineVolume", para.bShowKlineVolume);
	ini.WriteIntA(strSection, "ShowKlineAmount", para.bShowKlineAmount);
	ini.WriteIntA(strSection, "ShowKlineCAVol", para.bShowKlineCAVol);
	ini.WriteIntA(strSection, "ShowKlineCAAmo", para.bShowKlineCAAmo);

	ini.WriteIntA(strSection, "ShowKlineMACD", para.bShowKlineMACD);
	ini.WriteIntA(strSection, "ShowTSCRPS", para.bShowTSCRPS[0]);
	ini.WriteIntA(strSection, "ShowTSCL1RPS", para.bShowTSCRPS[0]);
	ini.WriteIntA(strSection, "ShowTSCL2RPS", para.bShowTSCRPS[0]);
	ini.WriteIntA(strSection, "ShowKlineRPS", para.bShowKlineRPS[0]);
	ini.WriteIntA(strSection, "ShowKlineL1RPS", para.bShowKlineRPS[1]);
	ini.WriteIntA(strSection, "ShowKlineL2RPS", para.bShowKlineRPS[2]);
	ini.WriteIntA(strSection, "Width", para.nWidth);
	ini.WriteIntA(strSection, "ShowTSCDeal", para.bShowTSCDeal);
	ini.WriteIntA(strSection, "ShowKlineDeal", para.bShowKlineDeal);
	ini.WriteIntA(strSection, "EMAPara1", para.nEMAPara[0]);
	ini.WriteIntA(strSection, "EMAPara2", para.nEMAPara[1]);
	ini.WriteIntA(strSection, "MACDPara1", para.nMACDPara[0]);
	ini.WriteIntA(strSection, "MACDPara2", para.nMACDPara[1]);
	ini.WriteIntA(strSection, "MACDPara3", para.nMACDPara[2]);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		ini.WriteIntA(strSection, strKey.Format("MAPara%d", i + 1), para.nMAPara[i]);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		ini.WriteIntA(strSection, strKey.Format("VolMAPara%d", i + 1), para.nVolMaPara[i]);
	for (int i = 0; i < MAX_MA_COUNT; ++i)
		ini.WriteIntA(strSection, strKey.Format("AmoMAPara%d", i + 1), para.nAmoMaPara[i]);
	for (int i = 0; i<MAX_MA_COUNT; ++i)
		ini.WriteIntA(strSection, strKey.Format("CAVolMAPara%d", i + 1), para.nCAVolMaPara[i]);
	for (int i = 0; i<MAX_MA_COUNT; ++i)
		ini.WriteIntA(strSection, strKey.Format("CAAmoMAPara%d", i + 1), para.nCAAmoMaPara[i]);

	ini.WriteIntA(strSection, "Jiange", para.nJiange);
	ini.WriteIntA(strSection, "BandN1", para.BandPara.N1);
	ini.WriteIntA(strSection, "BandN2", para.BandPara.N2);
	ini.WriteIntA(strSection, "BandK", para.BandPara.K);
	ini.WriteIntA(strSection, "BandM1", para.BandPara.M1);
	ini.WriteIntA(strSection, "BandM2", para.BandPara.M2);
	ini.WriteIntA(strSection, "BandP", para.BandPara.P);
	ini.WriteIntA(strSection, "Period", para.Period);
	ini.WriteIntA(strSection, "ListConnent1", para.Connect1);
	ini.WriteIntA(strSection, "ListConnent2", para.Connect2);
	ini.WriteStringA(strSection, "ShowIndustry", para.ShowIndy);
	ini.WriteIntA(strSection, "StockFilter", para.UseStockFilter);
	ini.WriteIntA(strSection, "ListShowST", para.ListShowST);
	ini.WriteIntA(strSection, "ListShowSBM", para.ListShowSBM);
	ini.WriteIntA(strSection, "ListShowSTARM", para.ListShowSTARM);
	ini.WriteIntA(strSection, "ListShowNewSotck", para.ListShowNewStock);
	ini.WriteIntA(strSection, "KlineRehabType", para.nKlineRehabType);
	ini.WriteIntA(strSection, "KlineCalcRehabType", para.nKlineCalcRehabType);
	ini.WriteIntA(strSection, "KlineFTRehabDate", para.nKlineFTRehabDate);

	ini.WriteStringA(strSection, "FilterName", para.strFilterName);
	ini.WriteIntA(strSection, "HisStockFilter", para.UseHisStockFilter);

	ini.WriteIntA(strSection, "KlineUseTickFlowData", para.bKlineUseTickFlowData);
	ini.WriteIntA(strSection, "KlineTickFlowDataType", para.nKlineTickFlowDataType);

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

bool _StockFilterPara::operator==(const _StockFilterPara & other) const
{
	return memcmp(this,&other,sizeof(_StockFilterPara)) == 0;
}

bool _HisStockFilterPara::operator<(const _HisStockFilterPara & other) const
{
	if (sf == other.sf)
	{
		if (countDay != other.countDay)
			return countDay < other.countDay;
		return type < other.type;
	}
	return sf < other.sf;
}
_TradeDouble::_TradeDouble()
{
	data = 0;
	digital = 0;
}

_TradeDouble::_TradeDouble(long long llData, long long llDigital)
{
	data = llData;
	digital = llDigital;
}

double _TradeDouble::GetDouble() const
{
	return data / pow(10,digital);
}

_TradeDouble _TradeDouble::operator+(const _TradeDouble & other)
{
	_TradeDouble res;
	res.digital = max(digital, other.digital);
	auto nDiff1 = res.digital - digital;
	auto nDiff2 = res.digital - other.digital;
	res.data = data *  (long long)pow(10, nDiff1) +
		other.data *  (long long)pow(10, nDiff2);
	return res;
}

_TradeDouble _TradeDouble::operator-(const _TradeDouble & other)
{
	_TradeDouble res;
	res.digital = max(digital, other.digital);
	auto nDiff1 = res.digital - digital;
	auto nDiff2 = res.digital - other.digital;
	res.data = data *  (long long)pow(10, nDiff1) -
		other.data *  (long long)pow(10, nDiff2);
	return res;
}

_TradeDouble _TradeDouble::operator*(const _TradeDouble & other)
{
	_TradeDouble res;
	res.digital = digital + other.digital;
	res.data = data * other.data;
	return res;
}

_TradeDouble _TradeDouble::operator/(const _TradeDouble & other)
{
	_TradeDouble res;
	res.digital = digital - other.digital;
	if (data % other.data == 0)
		res.data = data / other.data;
	else
	{
		double tmpData = GetDouble() / other.GetDouble();
		res.digital = max(digital, other.digital + 1);
		res.data = tmpData * pow(10, res.digital);
	}
	return res;
}

_PeridoPriceVol::_PeridoPriceVol()
{
	ZeroMemory(this, sizeof(*this));
}

_PeridoPriceVol::_PeridoPriceVol(int nPrice, int nType)
{
	ZeroMemory(this, sizeof(*this));
	this->nPriceMulti100 = nPrice;
	this->nPeriodType = nType;
}

_PeridoPriceVol::_PeridoPriceVol(const PriceVolInfo & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->nPriceMulti100 = other.nPriceMulti100;
	this->fActBigBuyVol = other.nActBigBuyVol;
	this->fActBigBuyOrder = other.nActBigBuyOrder;
	this->fActMidBuyVol = other.nActMidBuyVol;
	this->fActMidBuyOrder = other.nActMidBuyOrder;
	this->fActSmallBuyVol = other.nActSmallBuyVol;
	this->fActSmallBuyOrder = other.nActSmallBuyOrder;
	this->fActBigSellVol = other.nActBigSellVol;
	this->fActBigSellOrder = other.nActBigSellOrder;
	this->fActMidSellVol = other.nActMidSellVol;
	this->fActMidSellOrder = other.nActMidSellOrder;
	this->fActSmallSellVol = other.nActSmallSellVol;
	this->fActSmallSellOrder = other.nActSmallSellOrder;
	this->fPasBigBuyVol = other.nPasBigBuyVol;
	this->fPasBigBuyOrder = other.nPasBigBuyOrder;
	this->fPasMidBuyVol = other.nPasMidBuyVol;
	this->fPasMidBuyOrder = other.nPasMidBuyOrder;
	this->fPasSmallBuyVol = other.nPasSmallBuyVol;
	this->fPasSmallBuyOrder = other.nPasSmallBuyOrder;
	this->fPasBigSellVol = other.nPasBigSellVol;
	this->fPasBigSellOrder = other.nPasBigSellOrder;
	this->fPasMidSellVol = other.nPasMidSellVol;
	this->fPasMidSellOrder = other.nPasMidSellOrder;
	this->fPasSmallSellVol = other.nPasSmallSellVol;
	this->fPasSmallSellOrder = other.nPasSmallSellOrder;
}

_PeridoPriceVol & _PeridoPriceVol::operator+=(const _priceVol & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->fActBigBuyVol += other.nActBigBuyVol;
	this->fActBigBuyOrder += other.nActBigBuyOrder;
	this->fActMidBuyVol += other.nActMidBuyVol;
	this->fActMidBuyOrder += other.nActMidBuyOrder;
	this->fActSmallBuyVol += other.nActSmallBuyVol;
	this->fActSmallBuyOrder += other.nActSmallBuyOrder;
	this->fActBigSellVol += other.nActBigSellVol;
	this->fActBigSellOrder += other.nActBigSellOrder;
	this->fActMidSellVol += other.nActMidSellVol;
	this->fActMidSellOrder += other.nActMidSellOrder;
	this->fActSmallSellVol += other.nActSmallSellVol;
	this->fActSmallSellOrder += other.nActSmallSellOrder;
	this->fPasBigBuyVol += other.nPasBigBuyVol;
	this->fPasBigBuyOrder += other.nPasBigBuyOrder;
	this->fPasMidBuyVol += other.nPasMidBuyVol;
	this->fPasMidBuyOrder += other.nPasMidBuyOrder;
	this->fPasSmallBuyVol += other.nPasSmallBuyVol;
	this->fPasSmallBuyOrder += other.nPasSmallBuyOrder;
	this->fPasBigSellVol += other.nPasBigSellVol;
	this->fPasBigSellOrder += other.nPasBigSellOrder;
	this->fPasMidSellVol += other.nPasMidSellVol;
	this->fPasMidSellOrder += other.nPasMidSellOrder;
	this->fPasSmallSellVol += other.nPasSmallSellVol;
	this->fPasSmallSellOrder += other.nPasSmallSellOrder;
	return *this;
}

_PeridoPriceVol & _PeridoPriceVol::operator-=(const _priceVol & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->fActBigBuyVol -= other.nActBigBuyVol;
	this->fActBigBuyOrder -= other.nActBigBuyOrder;
	this->fActMidBuyVol -= other.nActMidBuyVol;
	this->fActMidBuyOrder -= other.nActMidBuyOrder;
	this->fActSmallBuyVol -= other.nActSmallBuyVol;
	this->fActSmallBuyOrder -= other.nActSmallBuyOrder;
	this->fActBigSellVol -= other.nActBigSellVol;
	this->fActBigSellOrder -= other.nActBigSellOrder;
	this->fActMidSellVol -= other.nActMidSellVol;
	this->fActMidSellOrder -= other.nActMidSellOrder;
	this->fActSmallSellVol -= other.nActSmallSellVol;
	this->fActSmallSellOrder -= other.nActSmallSellOrder;
	this->fPasBigBuyVol -= other.nPasBigBuyVol;
	this->fPasBigBuyOrder -= other.nPasBigBuyOrder;
	this->fPasMidBuyVol -= other.nPasMidBuyVol;
	this->fPasMidBuyOrder -= other.nPasMidBuyOrder;
	this->fPasSmallBuyVol -= other.nPasSmallBuyVol;
	this->fPasSmallBuyOrder -= other.nPasSmallBuyOrder;
	this->fPasBigSellVol -= other.nPasBigSellVol;
	this->fPasBigSellOrder -= other.nPasBigSellOrder;
	this->fPasMidSellVol -= other.nPasMidSellVol;
	this->fPasMidSellOrder -= other.nPasMidSellOrder;
	this->fPasSmallSellVol -= other.nPasSmallSellVol;
	this->fPasSmallSellOrder -= other.nPasSmallSellOrder;
	return *this;
}

_PeridoPriceVol & _PeridoPriceVol::operator+=(const _PeridoPriceVol & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->fActBigBuyVol += other.fActBigBuyVol;
	this->fActBigBuyOrder += other.fActBigBuyOrder;
	this->fActMidBuyVol += other.fActMidBuyVol;
	this->fActMidBuyOrder += other.fActMidBuyOrder;
	this->fActSmallBuyVol += other.fActSmallBuyVol;
	this->fActSmallBuyOrder += other.fActSmallBuyOrder;
	this->fActBigSellVol += other.fActBigSellVol;
	this->fActBigSellOrder += other.fActBigSellOrder;
	this->fActMidSellVol += other.fActMidSellVol;
	this->fActMidSellOrder += other.fActMidSellOrder;
	this->fActSmallSellVol += other.fActSmallSellVol;
	this->fActSmallSellOrder += other.fActSmallSellOrder;
	this->fPasBigBuyVol += other.fPasBigBuyVol;
	this->fPasBigBuyOrder += other.fPasBigBuyOrder;
	this->fPasMidBuyVol += other.fPasMidBuyVol;
	this->fPasMidBuyOrder += other.fPasMidBuyOrder;
	this->fPasSmallBuyVol += other.fPasSmallBuyVol;
	this->fPasSmallBuyOrder += other.fPasSmallBuyOrder;
	this->fPasBigSellVol += other.fPasBigSellVol;
	this->fPasBigSellOrder += other.fPasBigSellOrder;
	this->fPasMidSellVol += other.fPasMidSellVol;
	this->fPasMidSellOrder += other.fPasMidSellOrder;
	this->fPasSmallSellVol += other.fPasSmallSellVol;
	this->fPasSmallSellOrder += other.fPasSmallSellOrder;
	return *this;
}

_PeridoPriceVol & _PeridoPriceVol::operator-=(const _PeridoPriceVol & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->fActBigBuyVol -= other.fActBigBuyVol;
	this->fActBigBuyOrder -= other.fActBigBuyOrder;
	this->fActMidBuyVol -= other.fActMidBuyVol;
	this->fActMidBuyOrder -= other.fActMidBuyOrder;
	this->fActSmallBuyVol -= other.fActSmallBuyVol;
	this->fActSmallBuyOrder -= other.fActSmallBuyOrder;
	this->fActBigSellVol -= other.fActBigSellVol;
	this->fActBigSellOrder -= other.fActBigSellOrder;
	this->fActMidSellVol -= other.fActMidSellVol;
	this->fActMidSellOrder -= other.fActMidSellOrder;
	this->fActSmallSellVol -= other.fActSmallSellVol;
	this->fActSmallSellOrder -= other.fActSmallSellOrder;
	this->fPasBigBuyVol -= other.fPasBigBuyVol;
	this->fPasBigBuyOrder -= other.fPasBigBuyOrder;
	this->fPasMidBuyVol -= other.fPasMidBuyVol;
	this->fPasMidBuyOrder -= other.fPasMidBuyOrder;
	this->fPasSmallBuyVol -= other.fPasSmallBuyVol;
	this->fPasSmallBuyOrder -= other.fPasSmallBuyOrder;
	this->fPasBigSellVol -= other.fPasBigSellVol;
	this->fPasBigSellOrder -= other.fPasBigSellOrder;
	this->fPasMidSellVol -= other.fPasMidSellVol;
	this->fPasMidSellOrder -= other.fPasMidSellOrder;
	this->fPasSmallSellVol -= other.fPasSmallSellVol;
	this->fPasSmallSellOrder -= other.fPasSmallSellOrder;
	return *this;
}

_PeridoPriceVol & _PeridoPriceVol::AbsDiff(const _PeridoPriceVol & other)
{
	strcpy_s(this->SecurityID, other.SecurityID);
	this->fActBigBuyVol = abs(this->fActBigBuyVol - other.fActBigBuyVol);
	this->fActBigBuyOrder = abs(this->fActBigBuyOrder - other.fActBigBuyOrder);
	this->fActMidBuyVol = abs(this->fActMidBuyVol - other.fActMidBuyVol);
	this->fActMidBuyOrder = abs(this->fActMidBuyOrder - other.fActMidBuyOrder);
	this->fActSmallBuyVol = abs(this->fActSmallBuyVol - other.fActSmallBuyVol);
	this->fActSmallBuyOrder = abs(this->fActSmallBuyOrder - other.fActSmallBuyOrder);
	this->fActBigSellVol = abs(this->fActBigSellVol - other.fActBigSellVol);
	this->fActBigSellOrder = abs(this->fActBigSellOrder - other.fActBigSellOrder);
	this->fActMidSellVol = abs(this->fActMidSellVol - other.fActMidSellVol);
	this->fActMidSellOrder = abs(this->fActMidSellOrder - other.fActMidSellOrder);
	this->fActSmallSellVol = abs(this->fActSmallSellVol - other.fActSmallSellVol);
	this->fActSmallSellOrder = abs(this->fActSmallSellOrder - other.fActSmallSellOrder);
	this->fPasBigBuyVol = abs(this->fPasBigBuyVol - other.fPasBigBuyVol);
	this->fPasBigBuyOrder = abs(this->fPasBigBuyOrder - other.fPasBigBuyOrder);
	this->fPasMidBuyVol = abs(this->fPasMidBuyVol - other.fPasMidBuyVol);
	this->fPasMidBuyOrder = abs(this->fPasMidBuyOrder - other.fPasMidBuyOrder);
	this->fPasSmallBuyVol = abs(this->fPasSmallBuyVol - other.fPasSmallBuyVol);
	this->fPasSmallBuyOrder = abs(this->fPasSmallBuyOrder - other.fPasSmallBuyOrder);
	this->fPasBigSellVol = abs(this->fPasBigSellVol - other.fPasBigSellVol);
	this->fPasBigSellOrder = abs(this->fPasBigSellOrder - other.fPasBigSellOrder);
	this->fPasMidSellVol = abs(this->fPasMidSellVol - other.fPasMidSellVol);
	this->fPasMidSellOrder = abs(this->fPasMidSellOrder - other.fPasMidSellOrder);
	this->fPasSmallSellVol = abs(this->fPasSmallSellVol - other.fPasSmallSellVol);
	this->fPasSmallSellOrder = abs(this->fPasSmallSellOrder - other.fPasSmallSellOrder);
	return *this;
	// TODO: 在此处插入 return 语句
}

BOOL _PeridoPriceVol::IsDataSame(const _PeridoPriceVol & other)
{
	int nOffset = offsetof(_PeridoPriceVol, fActBigBuyVol);
	int nSzie = sizeof(*this) - nOffset;
	const void *pThis = (char*)this + nOffset;
	const void *pOther = (char*)&other + nOffset;
	return memcmp(pThis, pOther, nSzie) == 0;
}

