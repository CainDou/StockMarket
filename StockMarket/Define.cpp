#include "stdafx.h"
#include "Define.h"
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

	return PostThreadMessage(uThreadId, MsgType, (WPARAM)buf, (LPARAM)BufLen);
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
