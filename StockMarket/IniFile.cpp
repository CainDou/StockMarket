#include "stdafx.h"
#include "IniFile.h"


CIniFile::CIniFile()
{

}

CIniFile::~CIniFile()
{

}

CIniFile::CIniFile(LPCTSTR lpszName)
{
	m_strFileName = lpszName;
}

UINT CIniFile::GetInt(LPCTSTR lpSection, LPCTSTR lpKeyName, int nDefault)
{
	return GetPrivateProfileInt(lpSection, lpKeyName, nDefault, m_strFileName);
}

SStringW CIniFile::GetString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPCTSTR lpDefault)
{
	const int nBufSize = 1024;
	SStringW strBuf;
	GetPrivateProfileString(lpSection, lpKeyName, lpDefault, strBuf.GetBuffer(nBufSize), nBufSize, m_strFileName);
	strBuf.ReleaseBuffer();
	return strBuf;
}

BOOL CIniFile::GetStruct(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPVOID lpStruct, UINT uSize)
{
	return GetPrivateProfileStruct(lpszSection, lpszKeyName, lpStruct, uSize, m_strFileName);
}

BOOL CIniFile::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszKeyName, int nValue)
{
	SStringW strBuf;
	strBuf.Format(_T("%d"), nValue);
	return WritePrivateProfileString(lpszSection, lpszKeyName, strBuf, m_strFileName);
}

BOOL CIniFile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPCTSTR lpString)
{
	return WritePrivateProfileString(lpszSection, lpszKeyName, lpString, m_strFileName);
}


BOOL CIniFile::WriteStruct(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPVOID lpStruct, UINT uSize)
{
	return WritePrivateProfileStruct(lpszSection, lpszKeyName, lpStruct, uSize, m_strFileName);
}

void CIniFile::SetFileName(LPCTSTR lpszFileName)
{
	m_strFileName = lpszFileName;
}

BOOL CIniFile::DeleteKey(LPCTSTR lpszSection, LPCTSTR lpszKeyName)
{
	return WritePrivateProfileString(lpszSection, lpszKeyName, NULL, m_strFileName);
}

BOOL CIniFile::GetSection(LPCTSTR lpszSection, SStringWList *pStrArray)
{
	const int nBufSize = 32767;
	TCHAR *pBuf = new TCHAR[nBufSize];
	BOOL bResult = GetPrivateProfileSection(lpszSection, pBuf, nBufSize, m_strFileName);
	TCHAR *p = pBuf;
	while (*p)
	{
		pStrArray->Add(p);
		p += lstrlen(p) + 1;
	}
	delete[]pBuf;
	return bResult;
}

BOOL CIniFile::WriteSection(LPCTSTR lpszSection, SStringWList *pStrArray)
{
	TCHAR *pBuf;
	int nSize = 0;
	unsigned i = 0;
	for (i = 0; i<pStrArray->GetCount(); i++)
		nSize += pStrArray->GetAt(i).GetLength() + 1;
	nSize++;
	pBuf = new TCHAR[nSize];
	TCHAR *p = pBuf;
	for (unsigned i = 0; i<pStrArray->GetCount(); i++)
	{
		lstrcpy(p, pStrArray->GetAt(i));
		p += pStrArray->GetAt(i).GetLength() + 1;
	}
	*p++ = '\0';
	BOOL bResult = WritePrivateProfileSection(lpszSection, pBuf, m_strFileName);
	delete[] pBuf;
	return bResult;
}

CIniFile::CIniFile(LPCSTR lpszName)
{
	m_strFileNameA = lpszName;

}


BOOL CIniFile::DeleteKeyA(LPCSTR lpszSection, LPCSTR lpszKeyName)
{
	return WritePrivateProfileStringA(lpszSection, lpszKeyName, NULL, m_strFileNameA);
}

void CIniFile::SetFileNameA(LPCSTR lpszFileName)
{
	m_strFileNameA = lpszFileName;
}

BOOL CIniFile::WriteStructA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPVOID lpStruct, UINT uSize)
{
	return WritePrivateProfileStructA(lpszSection, lpszKeyName, lpStruct, uSize, m_strFileNameA);
}

BOOL CIniFile::WriteStringA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPCSTR lpString)
{
	return WritePrivateProfileStringA(lpszSection, lpszKeyName, lpString, m_strFileNameA);
}

BOOL CIniFile::WriteIntA(LPCSTR lpszSection, LPCSTR lpszKeyName, int nValue)
{
	SStringA strBuf;
	strBuf.Format("%d", nValue);
	return WritePrivateProfileStringA(lpszSection, lpszKeyName, strBuf, m_strFileNameA);
}

BOOL CIniFile::GetStructA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPVOID lpStruct, UINT uSize)
{
	return GetPrivateProfileStructA(lpszSection, lpszKeyName, lpStruct, uSize, m_strFileNameA);
}

SStringA CIniFile::GetStringA(LPCSTR lpSection, LPCSTR lpKeyName, LPCSTR lpDefault)
{
	const int nBufSize = 1024;
	SStringA strBuf;
	GetPrivateProfileStringA(lpSection, lpKeyName, lpDefault, strBuf.GetBuffer(nBufSize), nBufSize, m_strFileNameA);
	strBuf.ReleaseBuffer();
	return strBuf;
}

UINT CIniFile::GetIntA(LPCSTR lpSection, LPCSTR lpKeyName, int nDefault)
{
	return GetPrivateProfileIntA(lpSection, lpKeyName, nDefault, m_strFileNameA);
}


