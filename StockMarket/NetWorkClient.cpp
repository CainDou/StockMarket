#include "stdafx.h"
#include "NetWorkClient.h"
#include <process.h>
#include <iphlpapi.h>
#pragma comment(lib,"iphlpapi.lib")
BYTE CNetWorkClient::m_uMac[8] = { 0,0,0,0,0,0,0,0 };
vector<string> CNetWorkClient::m_strIPAddr = vector<string>();
vector<int> CNetWorkClient::m_nIPPort = vector<int>();
extern const char* ClientName;

CNetWorkClient::CNetWorkClient()
{
	m_hWnd = NULL;
	m_bConnected = FALSE;
	m_pFnHandle = NULL;
	//m_uThreadID = 0;
	m_socket = INVALID_SOCKET;
	m_ClientID = INVALID_SOCKET;
	//m_hFunc = INVALID_HANDLE_VALUE;
	m_bExit = FALSE;
	m_bRun = FALSE;
	m_nAskID = 0;
	m_bMacAuthRes = FALSE;
	m_bMacAuthSend = FALSE;
	m_nConnectServer = -1;

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
	unsigned long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) < 0) //设置为非阻塞
		return FALSE;
	int nReturn = ::connect(m_socket, (sockaddr*)&m_remote, sizeof(m_remote));

	if (nReturn == 0)
	{
		mode = 0;
		if (ioctlsocket(m_socket, FIONBIO, &mode) == 0)
			return TRUE;
		else
			return FALSE;
	}

	//因为是非阻塞的，这个时候错误码应该是WSAEWOULDBLOCK，Linux下是EINPROGRESS
	if (nReturn < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
		return FALSE;

	fd_set writeset;
	FD_ZERO(&writeset);
	FD_SET(m_socket, &writeset);
	timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	nReturn = select(m_socket + 1, NULL, &writeset, NULL, &tv);
	if (nReturn <= 0) {
		if (m_socket != INVALID_SOCKET)
		{
			WSACleanup();
			return FALSE;
		}
	}
	else {
		mode = 0;
		if (ioctlsocket(m_socket, FIONBIO, &mode) == 0)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;

}

BOOL CNetWorkClient::OnConnect(LPCSTR lpIP, UINT uPort)
{
	if (lpIP == NULL)
	{
		if (m_socket == INVALID_SOCKET)
		{
			m_bConnected = TRUE;
		}
		else
		{
			::shutdown(m_socket, SD_BOTH);
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			m_bConnected = FALSE;
			//m_bMacAuthSend = FALSE;
			//m_bMacAuthRes = FALSE;
		}

	}
	else
	{
		if (m_socket == INVALID_SOCKET)
		{

			if (!ConnectInit(lpIP, uPort, m_hWnd))
			{
				m_bConnected = FALSE;
				m_socket = INVALID_SOCKET;
				return FALSE;
			}
			m_bConnected = TRUE;
		}
		else
		{
			::shutdown(m_socket, SD_BOTH);
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			m_bConnected = FALSE;
			//m_bMacAuthSend = FALSE;
			//m_bMacAuthRes = FALSE;
		}

	}
	return TRUE;
}

//BOOL CNetWorkClient::RegisterHandle(PFNNETHANDLE pFunc)
//{
//	m_pFnHandle = pFunc;
//	return TRUE;
//}

BOOL CNetWorkClient::ConnectServer()
{
	if (!GetConnectState())
	{
		if (!m_bMacAuthSend || m_bMacAuthRes)
		{
			int nServerCount = min(m_strIPAddr.size(), m_nIPPort.size());
			if (m_nConnectServer == -1)
			{
				int nServer = 0;
				for (; nServer < nServerCount; ++nServer)
				{
					if (OnConnect(m_strIPAddr[nServer].c_str(), m_nIPPort[nServer]))
					{
						if (!m_bMacAuthSend)
							MacAddrAuth();
						break;

					}
				}
				if (nServer >= nServerCount)
					return false;
				else
					m_nConnectServer = nServer;
			}
			else
			{
				if (OnConnect(m_strIPAddr[m_nConnectServer].c_str(), m_nIPPort[m_nConnectServer]))
				{
					if (m_bMacAuthRes)
						MacAddrAuth();
					SendInfo info;
					strcpy(info.str, ClientName);
					info.MsgType = ComSend_ReConnect;
					info.Group = m_ClientID;
					SendData((char*)&info, sizeof(info));
				}
			}
		}
	}
	return m_bConnected;
}

BOOL CNetWorkClient::Start(PFNNETHANDLE pFunc, void *para)
{
	if (!pFunc)
		return FALSE;

	m_thread = std::thread(pFunc, para);
	m_bRun = TRUE;

	return TRUE;
}

BOOL CNetWorkClient::Stop()
{
	m_bExit = TRUE;
	if (m_bRun)
	{
		OnConnect(NULL, NULL);
		if (m_thread.joinable())
			m_thread.join();
		//WaitForSingleObject(m_hFunc, INFINITE);

		//m_hFunc = INVALID_HANDLE_VALUE;
		m_bRun = FALSE;
	}
	return TRUE;
}


int CNetWorkClient::ReceiveData(char * buffer, int size, char end)
{
	if (!m_bConnected && !ConnectServer())
		return 0;
	char*p = buffer;
	int sizeLeft = size;
	while (sizeLeft > 0)
	{
		int ret = recv(m_socket, p, sizeLeft, 0);
		if (0 == ret)
		{
			OnConnect(NULL, NULL);
			return 0;
		}
		if (SOCKET_ERROR == ret)
		{
			//delete[] buffer;
			//buffer = nullptr;
			p = nullptr;
			return SOCKET_ERROR;
		}
		sizeLeft -= ret;
		p += ret;
	}
	p = nullptr;
	if (0 != end)
	{
		char cEnd;
		int ret = recv(m_socket, &cEnd, 1, 0);
		if (cEnd == end)
			return size;
		return size - 1;
	}
	return size;
}

int CNetWorkClient::SendDataWithID(char* msg, int size)
{
	int nID = m_nAskID++;
	int newSize = size + sizeof(nID);
	char* newMsg = new char[newSize];
	memcpy_s(newMsg, newSize, msg, size);
	memcpy_s(newMsg + size, newSize, &nID, sizeof(nID));
	if (send(m_socket, newMsg, newSize, 0) > 0)
		return nID;
	return -1;
}

BOOL CNetWorkClient::MacAddrAuth()
{
	m_bMacAuthSend = TRUE;
	SendInfo info = { 0 };
	info.MsgType = ComSend_MacAddr;
	for (int i = 0; i < 6; ++i)
		info.str[i] = (char)m_uMac[i];
	SendData((char*)&info, sizeof(info));
	ReceiveInfo recvInfo;
	if (ReceiveData((char*)&recvInfo, sizeof(recvInfo)) > 0)
		m_bMacAuthRes = ((ReceiveAuthInfo)recvInfo).Res;
	else
		m_bMacAuthRes = FALSE;
	m_bConnected = m_bMacAuthRes;
	return m_bMacAuthRes;
}

void CNetWorkClient::GetLocalMac()
{
	std::map < std::string, std::vector<std::string>> result;
	IP_ADAPTER_INFO* pAdpFree = NULL;
	IP_ADAPTER_INFO* pIpAdpInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	unsigned long ulBufLen = sizeof(IP_ADAPTER_INFO);
	int ret;
	if ((ret = GetAdaptersInfo(pIpAdpInfo, &ulBufLen)) == ERROR_BUFFER_OVERFLOW) {
		free(pIpAdpInfo);
		//分配实际所需要的内存空间
		pIpAdpInfo = (IP_ADAPTER_INFO*)malloc(ulBufLen);
		if (NULL == pIpAdpInfo) {
			return;
		}
	}
	if ((ret = GetAdaptersInfo(pIpAdpInfo, &ulBufLen)) == NO_ERROR) {
		pAdpFree = pIpAdpInfo;

		for (auto pAdapter = pIpAdpInfo; pAdapter; pAdapter = pAdapter->Next) {

			if (pAdapter->Type != MIB_IF_TYPE_ETHERNET)
				continue;
			if (pAdapter->AddressLength != 6)
				continue;
			if (strstr(pAdapter->Description, "Bluetooth") != nullptr)
				continue;
			//if (strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") == 0)
			//	continue;
			for (int i = 0; i < 8; ++i)
				m_uMac[i] = pAdapter->Address[i];
			break;
		}
	}
	if (pAdpFree) {
		free(pAdpFree);
	}

}
