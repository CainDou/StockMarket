#pragma once
class CIniFile
{
public:
	BOOL WriteSection(LPCTSTR lpszSection, SStringWList*pStrArray);
	BOOL GetSection(LPCTSTR lpszSection, SStringWList*pStrArray);
	BOOL DeleteKey(LPCTSTR lpszSection, LPCTSTR lpszKeyName);
	void SetFileName(LPCTSTR lpszFileName);
	BOOL WriteStruct(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPVOID lpStruct, UINT uSize);
	BOOL WriteString(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPCTSTR lpString);
	BOOL WriteInt(LPCTSTR lpszSection, LPCTSTR lpszKeyName, int nValue);
	BOOL GetStruct(LPCTSTR lpszSection, LPCTSTR lpszKeyName, LPVOID lpStruct, UINT uSize);
	SStringW GetString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPCTSTR lpDefault = _T(""));
	UINT GetInt(LPCTSTR lpSection, LPCTSTR lpKeyName, int nDefault = 0);
	CIniFile(LPCTSTR lpszName);
	CIniFile(LPCSTR lpszName);
	CIniFile();
	virtual ~CIniFile();
	BOOL DeleteKeyA(LPCSTR lpszSection, LPCSTR lpszKeyName);
	void SetFileNameA(LPCSTR lpszFileName);
	BOOL WriteStructA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPVOID lpStruct, UINT uSize);
	BOOL WriteStringA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPCSTR lpString);
	BOOL WriteIntA(LPCSTR lpszSection, LPCSTR lpszKeyName, int nValue);
	BOOL GetStructA(LPCSTR lpszSection, LPCSTR lpszKeyName, LPVOID lpStruct, UINT uSize);
	SStringA GetStringA(LPCSTR lpSection, LPCSTR lpKeyName, LPCSTR lpDefault = (""));
	UINT GetIntA(LPCSTR lpSection, LPCSTR lpKeyName, int nDefault = 0);

protected:
	SStringW m_strFileName;
	SStringA m_strFileNameA;
};

