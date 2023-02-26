#include "stdafx.h"
#include "FrmlManager.h"
#include <direct.h>
#include <io.h>
#include <fstream>

set<string> CFrmlManager::m_FrmlName = set<string>();
map<string, FrmlFullInfo>  CFrmlManager::m_FrmlMap
= map<string, FrmlFullInfo>();
map<string, int> CFrmlManager::m_FrmlUseMap = map<string, int>();
CRITICAL_SECTION  CFrmlManager::m_cs = CRITICAL_SECTION();

CFrmlManager::CFrmlManager()
{
}


CFrmlManager::~CFrmlManager()
{
}

BOOL CFrmlManager::InitFrmlManage()
{
	::InitializeCriticalSection(&m_cs);
	GetFormulas();
	return 0;
}

BOOL CFrmlManager::CheckNameIsUseful(string strName)
{
	Lock();
	BOOL bUseful =  m_FrmlName.count(strName) == 0;
	UnLock();
	return bUseful;
}

BOOL CFrmlManager::AddNewFormula(FrmlFullInfo & info)
{
	Lock();
	m_FrmlMap[info.name] = info;
	m_FrmlName.insert(info.name);
	UnLock();
	return TRUE;
}

BOOL CFrmlManager::ChangeFormula(string oldName, FrmlFullInfo & info)
{
	Lock();
	m_FrmlMap.erase(oldName);
	m_FrmlName.erase(oldName);
	m_FrmlMap[info.name] = info;
	m_FrmlName.insert(info.name);
	UnLock();
	return TRUE;
}

BOOL CFrmlManager::DeleteFormula(string oldName)
{
	Lock();
	m_FrmlMap.erase(oldName);
	m_FrmlName.erase(oldName);
	UnLock();
	return TRUE;
}

BOOL CFrmlManager::SaveFormulas()
{
	if (_access(".\\formula", 0) != 0)
		_mkdir(".\\formula");
	string fileName = ".\\formula\\UserFrml.fi";
	ofstream ofile(fileName, ios::binary);
	if (ofile.is_open())
	{
		Lock();
		for (auto &it : m_FrmlMap)
				SaveFormula(ofile, it.second);
		ofile.close();
		UnLock();
	}
	else
		return FALSE;
	return TRUE;
}

BOOL CFrmlManager::GetFormulas()
{
	string fileName = ".\\formula\\UserFrml.fi";
	ifstream ifile(fileName, ios::binary);
	if (ifile.is_open())
	{
		char ch = 0;
		string tag = "";
		string revTag = "";
		string msg = "";
		bool bIsTag = false;
		bool bIsRevTag = false;
		bool bFirstChar = false;
		while (ifile.read(&ch, 1))
			msg += ch;
		size_t nStartPos = 0;
		size_t nEndPos = 0;
		while (nStartPos != string::npos)
		{
			nStartPos = msg.find("<info>", nEndPos);
			nEndPos = msg.find("</info>", nStartPos);
			if (nStartPos != string::npos && nEndPos != string::npos)
			{
				string funMsg = msg.substr(nStartPos + 6, nEndPos - nStartPos - 6);
				FrmlFullInfo info = { "" };
				if (GetFormula(funMsg, info))
				{
					Lock();
					m_FrmlMap[info.name] = info;
					m_FrmlName.insert(info.name);
					UnLock();
				}
			}
		}
	}
	else
		return FALSE;
	return TRUE;
}

map<string, FrmlFullInfo> CFrmlManager::GetFormulaMap()
{
	return m_FrmlMap;
}

const FrmlFullInfo & CFrmlManager::GetFormula(string name)
{
	return m_FrmlMap[name];
}

void CFrmlManager::IncreaseFrmlUseCount(string name)
{
	UnLock();
	m_FrmlUseMap[name]++;
	UnLock();
}

void CFrmlManager::DecreaseFrmlUseCount(string name)
{
	UnLock();
	m_FrmlUseMap[name]--;
	UnLock();
}

int CFrmlManager::GetFrmlUseCount(string name)
{
	if (m_FrmlUseMap.count(name) == 0)
		return 0;
	return m_FrmlUseMap[name];
}

BOOL CFrmlManager::SaveFormula(ofstream & ofile, FrmlFullInfo & info)
{
	char nEnd = 0;
	ofile.write("<info>", strlen("<info>"));
	ofile.write("<name>", strlen("<name>"));
	ofile.write(info.name.c_str(), info.name.size());
	ofile.write("</name>", strlen("</name>"));
	ofile.write("<descption>", strlen("<descption>"));
	ofile.write(info.descption.c_str(), info.descption.size());
	ofile.write("</descption>", strlen("</descption>"));
	ofile.write("<para>", strlen("<para>"));
	for (auto &it : info.para)
	{
		ofile.write("<paraitem>", strlen("<paraitem>"));
		ofile.write(it.c_str(), it.size());
		ofile.write(&nEnd, 1);
		ofile.write((char*)&info.paraSetting[it].min, sizeof(info.paraSetting[it].min));
		ofile.write((char*)&info.paraSetting[it].max, sizeof(info.paraSetting[it].max));
		ofile.write((char*)&info.paraSetting[it].def, sizeof(info.paraSetting[it].def));
		ofile.write("</paraitem>", strlen("</paraitem>"));
	}
	ofile.write("</para>", strlen("</para>"));
	ofile.write("<code>", strlen("<code>"));
	ofile.write(info.code.c_str(), info.code.size());
	ofile.write("</code>", strlen("</code>"));
	ofile.write("<formula>", strlen("<formula>"));
	for (auto &it : info.formula)
	{
		ofile.write("<frmlitem>", strlen("<frmlitem>"));
		ofile.write((char*)&it.type, sizeof(it.type));
		ofile.write(it.paraName.c_str(), it.paraName.size());
		ofile.write(&nEnd, 1);
		it.formula.SaveToFile(ofile,0);
		ofile.write("</frmlitem>", strlen("</frmlitem>"));
	}
	ofile.write("</formula>", strlen("</formula>"));
	ofile.write("<paraelf>", strlen("<paraelf>"));
	ofile.write(info.paraElf.c_str(), info.paraElf.size());
	ofile.write("</paraelf>", strlen("</paraelf>"));
	ofile.write("<usage>", strlen("<usage>"));
	ofile.write(info.usage.c_str(), info.usage.size());
	ofile.write("</usage>", strlen("</usage>"));
	ofile.write("<usetype>", strlen("<usetype>"));
	ofile.write((char*)&info.useType, sizeof(info.useType));
	ofile.write("</usetype>", strlen("</usetype>"));
	ofile.write("<subdivision>", strlen("<subdivision>"));
	ofile.write((char*)&info.subdivision, sizeof(info.subdivision));
	ofile.write("</subdivision>", strlen("</subdivision>"));
	ofile.write("<sysfrml>", strlen("<sysfrml>"));
	ofile.write((char*)&info.sysFrml, sizeof(info.sysFrml));
	ofile.write("</sysfrml>", strlen("</sysfrml>"));
	ofile.write("</info>", strlen("</info>"));
	return TRUE;

}

BOOL CFrmlManager::GetFormula(string& str, FrmlFullInfo & info)
{
	info.name = GetStringData(str, "<name>", "</name>");
	info.descption = GetStringData(str, "<descption>", "</descption>");
	//获取参数设置
	string paraInfo = GetStringData(str, "<para>", "</para>");
	if (!paraInfo.empty())
	{
		size_t startPos = 0;
		size_t endPos = 0;
		while (true)
		{
			startPos = paraInfo.find("<paraitem>", endPos);
			if (startPos != string::npos)
			{
				startPos += strlen("<paraitem>");
				endPos = paraInfo.find('\0', startPos);
				string paraName = paraInfo.substr(startPos, endPos - startPos);
				info.para.emplace_back(paraName);
				info.paraSetting[paraName].min = *(double*)(paraInfo.c_str() + endPos + 1);
				info.paraSetting[paraName].max = *(double*)(paraInfo.c_str()
					+ endPos + sizeof(double) + 1);
				info.paraSetting[paraName].def = *(double*)(paraInfo.c_str()
					+ endPos + sizeof(double) * 2 + 1);
				endPos = paraInfo.find("</paraitem>", startPos);
			}
			else
				break;
		}
	}

	info.code = GetStringData(str, "<code>", "</code>");
	string formula = GetStringData(str, "<formula>", "</formula>");
	if (!formula.empty())
	{
		size_t startPos = 0;
		size_t endPos = 0;
		while (true)
		{
			startPos = formula.find("<frmlitem>", endPos);
			if (startPos != string::npos)
			{
				OutFrml of = { 0 };
				startPos += strlen("<frmlitem>");
				of.type = *(int*)(formula.c_str() + startPos);
				startPos += sizeof(int);
				endPos = formula.find('\0', startPos);
				of.paraName = formula.substr(startPos, endPos - startPos);
				startPos = endPos + 1;
				endPos = formula.find("</frmlitem>", startPos);
				string outFrmlStr = formula.substr(startPos, endPos - startPos);
				if (of.formula.GetDataFromString(outFrmlStr,0))
					info.formula.emplace_back(of);
			}
			else
				break;
		}

	}
	info.paraElf = GetStringData(str, "<paraelf>", "</paraelf>");
	info.usage = GetStringData(str, "<usage>", "</usage>");
	info.useType = *(eFrmlUse*)GetStringData(str, "<usetype>", "</usetype>").c_str();
	info.subdivision = *(int*)GetStringData(str, "<subdivision>", "</subdivision>").c_str();
	info.sysFrml = *(BOOL*)GetStringData(str, "<sysfrml>", "</sysfrml>").c_str();

	return TRUE;

}

BOOL CFrmlManager::WriteString(ofstream & ofile, string begTag, string endTag,
	char * msg, size_t size)
{
	ofile.write(begTag.c_str(), begTag.size());
	ofile.write(msg, size);
	ofile.write(endTag.c_str(), endTag.size());
	return TRUE;
}

string CFrmlManager::GetStringData(string & str, string begTag, string endTag)
{
	size_t nStartPos = str.find(begTag);
	size_t nEndPos = str.find(endTag, nStartPos);
	if (nStartPos != string::npos && nEndPos != string::npos)
		return str.substr(nStartPos + begTag.size(), nEndPos - nStartPos - begTag.size());
	return "";
}

BOOL CFrmlManager::InitSysFrml()
{
	return 0;
}

BOOL CFrmlManager::InitIndexConditionFrml()
{
	return 0;
}

BOOL CFrmlManager::InitFundamentalFrml()
{
	return 0;
}

BOOL CFrmlManager::InitRealTimeFrml()
{
	return 0;
}

BOOL CFrmlManager::InitTendencyFrml()
{
	return 0;
}

BOOL CFrmlManager::InitFormFrml()
{
	return 0;
}

BOOL CFrmlManager::InitOtherFrml()
{
	return 0;
}

void CFrmlManager::Lock()
{
	::EnterCriticalSection(&m_cs);
}

void CFrmlManager::UnLock()
{
	::LeaveCriticalSection(&m_cs);
}
