#pragma once
#include<WinSock2.h>

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

protected:
	SOCKET m_socket;
	HWND m_hWnd;
	sockaddr_in m_remote;
	BOOL m_bConnected;
	UINT m_uThreadID;
	HANDLE m_hFunc;
	PFNNETHANDLE m_pFnHandle;
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
	return m_bConnected;
}

inline BOOL CNetWorkClient::SetState(BOOL bState)
{
	m_bConnected = FALSE;
	return TRUE;
}

