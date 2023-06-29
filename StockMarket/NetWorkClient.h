#pragma once
#include<WinSock2.h>
#include <atomic>
/**
* PFNNETHANDLE
* @brief     接收函数--函数指针
*
* Describe   函数指针
*/
typedef unsigned int(__stdcall *PFNNETHANDLE)(void*);

class CNetWorkClient
{
public:
	CNetWorkClient();
	~CNetWorkClient();
	BOOL SetWndHandle(HWND hwnd);
	BOOL ConnectInit(LPCSTR pszRemoteAddr, u_short nPort, HWND hWnd);
	BOOL OnConnect(LPCSTR lpIP, UINT uPort);
	BOOL RegisterHandle(PFNNETHANDLE pFunc);
	HANDLE Start(UINT &ThreadID,void* para);
	BOOL   Stop();
	SOCKET GetSocket() const;
	BOOL   GetState() const;
	BOOL   SetState(BOOL bState);
	BOOL   SetClientID(SOCKET s);
	SOCKET GetClientID() const;
	BOOL   GetExitState() const;
	BOOL	SendData(char* msg, int size);
	BOOL   ReceiveData(char * buffer, int size, char end = 0);
	int   SendDataWithID(char* msg, int size);
protected:
	SOCKET m_socket;
	SOCKET m_ClientID;
	HWND m_hWnd;
	sockaddr_in m_remote;
	BOOL m_bConnected;
	BOOL m_bRun;
	BOOL m_bExit;
	UINT m_uThreadID;
	HANDLE m_hFunc;
	PFNNETHANDLE m_pFnHandle;
	std::atomic_int m_nAskID;
};

inline BOOL CNetWorkClient::SetWndHandle(HWND hWnd)
{
	m_hWnd = hWnd;
	return TRUE;
}


inline SOCKET CNetWorkClient::GetSocket() const
{
	return m_socket;
}

inline BOOL CNetWorkClient::GetState() const
{
	return m_bConnected && m_bRun;
}

inline BOOL CNetWorkClient::SetState(BOOL bState)
{
	m_bConnected = FALSE;
	return TRUE;
}

inline BOOL CNetWorkClient::SetClientID(SOCKET s)
{
	m_ClientID = s;
	return TRUE;
}

inline SOCKET CNetWorkClient::GetClientID() const
{
	return m_ClientID;
}

inline BOOL CNetWorkClient::GetExitState() const
{
	return m_bExit;
}

inline BOOL CNetWorkClient::SendData(char * msg, int size)
{
	return send(m_socket, msg, size, 0) > 0;
}

