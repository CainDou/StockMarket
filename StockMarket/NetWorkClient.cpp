#include "stdafx.h"
#include "NetWorkClient.h"
#include <process.h>

CNetWorkClient::CNetWorkClient()
{
	m_hWnd = NULL;
	m_bConnected = FALSE;
	m_pFnHandle = NULL;
	m_uThreadID = 0;
	m_socket = INVALID_SOCKET;
	m_ClientID = INVALID_SOCKET;
	m_hFunc = INVALID_HANDLE_VALUE;
	m_bExit = FALSE;
}


CNetWorkClient::~CNetWorkClient()
{
}

BOOL CNetWorkClient::ConnectInit(LPCSTR pszRemoteAddr, u_short nPort, HWND hWnd)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
		return FALSE;

	int on = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on));
	ULONG uAddr = ::inet_addr(pszRemoteAddr);
	if (uAddr == INADDR_NONE)
	{
		hostent* pHost = ::gethostbyname(pszRemoteAddr);
		if (pHost == NULL)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return FALSE;
		}
		uAddr = ((struct in_addr*)*(pHost->h_addr_list))->S_un.S_addr;
	}

	ZeroMemory(&m_remote, sizeof(m_remote));
	m_remote.sin_addr.S_un.S_addr = uAddr;
	m_remote.sin_family = AF_INET;
	m_remote.sin_port = htons(nPort);

	int nReturn = ::connect(m_socket, (sockaddr*)&m_remote, sizeof(m_remote));

	if (nReturn == 0)
		return TRUE;
	else
		return FALSE;
}

BOOL CNetWorkClient::OnConnect(LPCSTR lpIP, UINT uPort)
{
	if (m_socket == INVALID_SOCKET)
	{

		if (!ConnectInit(lpIP, uPort, m_hWnd))
		{
			m_socket = INVALID_SOCKET;
			return FALSE;
		}
	}
	else
	{
		::shutdown(m_socket, SD_BOTH);
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	return TRUE;
}

BOOL CNetWorkClient::RegisterHandle(PFNNETHANDLE pFunc)
{
	m_pFnHandle = pFunc;
	return TRUE;
}

HANDLE CNetWorkClient::Start(UINT & ThreadID, void *para)
{
	if (!m_pFnHandle)
		return NULL;
	
	m_hFunc = (HANDLE)_beginthreadex(NULL, 0, m_pFnHandle, para, 0, &m_uThreadID);
	if (m_hFunc != INVALID_HANDLE_VALUE)
		m_bConnected = TRUE;
	ThreadID = m_uThreadID;
	return m_hFunc;
}

BOOL CNetWorkClient::Stop()
{
	m_bExit = TRUE;
	if (m_bConnected)
	{
		OnConnect(NULL, NULL);
		WaitForSingleObject(m_hFunc, INFINITE);
		m_hFunc = INVALID_HANDLE_VALUE;
		m_bConnected = FALSE;
	}
	return TRUE;
}
