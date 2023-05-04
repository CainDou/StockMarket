#include "stdafx.h"
#include "FrmlCompiler.h"
#include <stack>
#include <sstream>
#include <queue>
#include <ios>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

map<string, PFUNC> FuncMap;
BOOL bFuncMapInited = FALSE;
BOOL CFrmlCompiler::m_bInited = FALSE;
map<eSntcType, regex> CFrmlCompiler::m_ruleMap = map<eSntcType, regex>();
map<string, int> CFrmlCompiler::m_FuncMap = map<string, int>();
set<string> CFrmlCompiler::m_OptSet = set<string>();
set<string> CFrmlCompiler::m_SystemVarSet = set<string>();
set<char> CFrmlCompiler::m_EmptySet = set<char>();
map<eGraError, string> CFrmlCompiler::m_errTransMap = map<eGraError, string>();
map<string, string> CFrmlCompiler::m_FuncTransMap = map<string, string>();
map<eFuncType, vector<FuncInfo>> CFrmlCompiler::m_FuncInfoMap = map<eFuncType, vector<FuncInfo>>();

CFrmlCompiler::CFrmlCompiler()
{
}


CFrmlCompiler::~CFrmlCompiler()
{
}

BOOL CFrmlCompiler::ExplainString(string & str, set<string> setPara,
	ErrMsg& Error, vector<OutFrml>& funcVec)
{
	m_paraMap.clear();
	m_charLineNo.clear();
	for (auto &it : setPara)
		m_paraMap[it] = 0;
	FindParaDefine(str);
	nLineNo = 1;
	int nLineStart = 0;
	bool bExplain = false;
	for (nCharNo = 0; nCharNo < str.length(); ++nCharNo)
	{
		if (!bExplain)
		{
			if (str[nCharNo] == '{')
			{
				bExplain = true;
				continue;
			}
			if (str[nCharNo] != ';')
				continue;
			OutFrml funcInfo = { 0 };
			string strLine = str.substr(nLineStart, nCharNo - nLineStart + 1);
			//funcInfo.srcSent = strLine;
			nLineStart = nCharNo + 1;
			Error.strSent = strLine;
			if (SentenceHandle(strLine, Error, funcInfo))
				funcVec.emplace_back(funcInfo);
			else
				return FALSE;
		}
		else
		{
			if (str[nCharNo] == '}')
				bExplain = false;
		}

	}
	if (nLineStart < str.length())
	{
		string strLine = str.substr(nLineStart, nCharNo - nLineStart + 1);
		Error.strSent = strLine;
		if (!LeftStrHandle(strLine, Error))
			return FALSE;
	}



	return TRUE;
}

eEleType CFrmlCompiler::GetStrEleType(const string & str)
{
	if (m_FuncMap.count(str))
		return ET_FUNC;
	else if (m_SystemVarSet.count(str))
		return ET_SYSTEMVAR;
	else if (m_OptSet.count(str))
		return ET_OPEATION;
	else
	{
		if (isdigit(str[0]))
		{
			bool bIsDigit = true;
			int nDotCount = 0;
			for (auto &it : str)
			{
				if (!isdigit(it) && it != '.')
				{
					bIsDigit = false;
					break;
				}
				else if (it == '.')
					++nDotCount;
			}
			if (bIsDigit && nDotCount <= 1)
				return ET_CONST;
		}
	}
	return ET_ILLEEGAL;

}

BOOL CFrmlCompiler::SpliteSenetence(const string & str, vector<ElementType>& eleVec)
{
	int nStartPos = 0;
	ElementType et = { 0 };
	string buffer = "";
	eCharType preCharType = CT_EMPTY;
	eCharType nowCharType = CT_EMPTY;
	int nBracLV = 0;
	for (int i = 0; i < str.length(); ++i)
	{
		et.line = nLineNo;
		nowCharType = GetCharType(str[i]);
		if (preCharType == CT_EXPSTART)
		{
			if (nowCharType == CT_EXPEND)
				preCharType = nowCharType;
			continue;
		}
		if (nowCharType != CT_CHAR && nowCharType != CT_SIGN && nowCharType != CT_NUMBER)
		{
			if (buffer != "")
			{
				et.element = buffer;
				et.type = GetElementType(buffer);
				eleVec.emplace_back(et);
			}
			if (nowCharType != CT_EMPTY && nowCharType != CT_EXPSTART)
			{
				buffer = str[i];
				et.element = buffer;
				et.type = GetElementType(buffer);
				eleVec.emplace_back(et);
			}
			buffer = "";
		}
		else
		{
			if (preCharType == CT_CHAR || preCharType == CT_SIGN || preCharType == CT_NUMBER)
			{
				if (nowCharType == preCharType || (preCharType + nowCharType == CT_CHAR + CT_NUMBER))
					buffer += str[i];
				else
				{
					if (buffer != "")
					{
						et.element = buffer;
						et.type = GetElementType(buffer);
						eleVec.emplace_back(et);
					}
					buffer = "";
					buffer += str[i];
				}

			}
			else
				buffer += str[i];
		}
		preCharType = nowCharType;
	}
	if (buffer != "")
	{
		et.element = buffer;
		et.type = GetElementType(buffer);
		eleVec.emplace_back(et);
	}

	return TRUE;
}

void CFrmlCompiler::InitSetting()
{
	if (!m_bInited)
	{
		InitRules();
		InitOptAndFunc();
		InitSystemVar();
		InitEmptyChar();
		InitErrorTrans();
		InitFuncAndVarTrans();
		InitFuncInfo();
		m_bInited = TRUE;
	}
}

void CFrmlCompiler::InitRules()
{
	m_ruleMap[Snt_Assign] = regex("^(\\s+)?.*[A-Z]{1}(\\w+)?(\\s+)?=.*;$");
	m_ruleMap[Snt_Out] = regex("^(\\s+)?.*[A-Z]{1}(\\w+)?(\\s+)?:.*;$");
}

void CFrmlCompiler::InitOptAndFunc()
{
	m_FuncMap["ABS"] = 1;
	m_FuncMap["BETWEEN"] = 3;
	m_FuncMap["COS"] = 1;
	m_FuncMap["EXP"] = 1;
	m_FuncMap["IF"] = 3;
	m_FuncMap["LN"] = 1;
	m_FuncMap["LOG"] = 2;
	m_FuncMap["MAX"] = 2;
	m_FuncMap["MIN"] = 2;
	m_FuncMap["MOD"] = 2;
	m_FuncMap["POW"] = 2;
	m_FuncMap["NOT"] = 1;
	m_FuncMap["RAND"] = 1;
	m_FuncMap["RANGE"] = 3;
	m_FuncMap["REVERSE"] = 1;
	m_FuncMap["SIN"] = 1;
	m_FuncMap["SQRT"] = 1;
	m_FuncMap["TAN"] = 1;
	m_OptSet.insert("+");
	m_OptSet.insert("-");
	m_OptSet.insert("*");
	m_OptSet.insert("/");
	m_OptSet.insert(">");
	m_OptSet.insert("<");
	m_OptSet.insert(">=");
	m_OptSet.insert("==");
	m_OptSet.insert("<=");
	m_OptSet.insert("||");
	m_OptSet.insert("&&");
	m_OptSet.insert("AND");
	m_OptSet.insert("OR");


}

void CFrmlCompiler::InitSystemVar()
{
	m_SystemVarSet.insert("LASTPX");
	m_SystemVarSet.insert("CHG");
	m_SystemVarSet.insert("MACD520");
	m_SystemVarSet.insert("MACD2060");
	m_SystemVarSet.insert("RPS520");
	m_SystemVarSet.insert("RPS2060");
	m_SystemVarSet.insert("RANK520");
	m_SystemVarSet.insert("RANK2060");
	m_SystemVarSet.insert("POINT520");
	m_SystemVarSet.insert("POINT2060");
	m_SystemVarSet.insert("RANK520L1");
	m_SystemVarSet.insert("RANK2060L1");
	m_SystemVarSet.insert("POINT520L1");
	m_SystemVarSet.insert("POINT2060L1");
	m_SystemVarSet.insert("RANK520L2");
	m_SystemVarSet.insert("RANK2060L2");
	m_SystemVarSet.insert("POINT520L2");
	m_SystemVarSet.insert("POINT2060L2");
	m_SystemVarSet.insert("AMTMACD520");
	m_SystemVarSet.insert("AMTMACD2060");
	m_SystemVarSet.insert("AMTRPS520");
	m_SystemVarSet.insert("AMTRPS2060");
	m_SystemVarSet.insert("AMTRANK520");
	m_SystemVarSet.insert("AMTRANK2060");
	m_SystemVarSet.insert("AMTPOINT520");
	m_SystemVarSet.insert("AMTPOINT2060");
	m_SystemVarSet.insert("AMTRANK520L1");
	m_SystemVarSet.insert("AMTRANK2060L1");
	m_SystemVarSet.insert("AMTPOINT520L1");
	m_SystemVarSet.insert("AMTPOINT2060L1");
	m_SystemVarSet.insert("AMTRANK520L2");
	m_SystemVarSet.insert("AMTRANK2060L2");
	m_SystemVarSet.insert("AMTPOINT520L2");
	m_SystemVarSet.insert("AMTPOINT2060L2");
	m_SystemVarSet.insert("AMTRATIO");
	m_SystemVarSet.insert("VOL");
	m_SystemVarSet.insert("HIGH");
	m_SystemVarSet.insert("LOW");
	m_SystemVarSet.insert("OPEN");
	m_SystemVarSet.insert("CLOSE");
	m_SystemVarSet.insert("AMOUNT");
	m_SystemVarSet.insert("POC");
	m_SystemVarSet.insert("ABV");
	m_SystemVarSet.insert("ASV");
	m_SystemVarSet.insert("ABO");
	m_SystemVarSet.insert("ASO");
	m_SystemVarSet.insert("PBO");
	m_SystemVarSet.insert("PSO");
	m_SystemVarSet.insert("ABSR");
	m_SystemVarSet.insert("A2PBSR");
	m_SystemVarSet.insert("AABSR");
	m_SystemVarSet.insert("POCR");
}

void CFrmlCompiler::InitEmptyChar()
{
	m_EmptySet.insert(' ');
	m_EmptySet.insert('\t');
	m_EmptySet.insert('\n');
	m_EmptySet.insert('\r');
}

void CFrmlCompiler::InitErrorTrans()
{
	m_errTransMap[GE_None] = "�޴���";
	m_errTransMap[GE_UndefinePara] = "δ֪�Ĳ���";
	m_errTransMap[GE_NumBeforeLetter] = "������������ֻ������ĸ��ͷ";
	m_errTransMap[GE_MissEnd] = "ȱ�ٽ�����';'";
	m_errTransMap[GE_ExpMissEnd] = "ע�����ȱ�ٽ�β'}'";
	m_errTransMap[GE_IllegalGrammar] = "�﷨����";
	m_errTransMap[GE_UndefineSign] = "δ֪�ķ���";
	m_errTransMap[GE_ErrorCondition] = "�ж���������";
	m_errTransMap[GE_ErrorOut] = "����������";
	m_errTransMap[GE_ErrorParaNum] = "����������������";
	m_errTransMap[GE_MissLeftParen] = "ȱ����ƥ���������";
	m_errTransMap[GE_MissRightParen] = "ȱ����ƥ���������";

}

void CFrmlCompiler::InitFuncAndVarTrans()
{
	m_FuncTransMap["IF"] = "���PARAM#1,����PARAM#2,���򷵻�PARAM#3";
	m_FuncTransMap["ABS"] = "PARAM#1�ľ���ֵ";
	m_FuncTransMap["BETWEEN"] = "PARAM#1��PARAM#2��PARAM#3֮��";
	m_FuncTransMap["COS"] = "PARAM#1������";
	m_FuncTransMap["LN"] = "PARAM#1����Ȼ����";
	m_FuncTransMap["LOG"] = "PARAM#1����10Ϊ�׵Ķ���";
	m_FuncTransMap["MAX"] = "PARAM#1��PARAM#2�����ֵ";
	m_FuncTransMap["MIN"] = "PARAM#1��PARAM#2����Сֵ";
	m_FuncTransMap["MOD"] = "PARAM#1��PARAM#2��ģ";
	m_FuncTransMap["POW"] = "PARAM#1��PARAM#2�η�";
	m_FuncTransMap["NOT"] = "��PARAM#1";
	m_FuncTransMap["RAND"] = "ȡһ����Χ��1-PARAM#1�������";
	m_FuncTransMap["RANGE"] = "PARAM#1����PARAM#2С��PARAM#3";
	m_FuncTransMap["REVERSE"] = "PARAM#1���෴��";
	m_FuncTransMap["SIN"] = "PARAM#1������";
	m_FuncTransMap["SQRT"] = "PARAM#1��ƽ����";
	m_FuncTransMap["TAN"] = "PARAM#1������";



	m_FuncTransMap["||"] = "��";
	m_FuncTransMap["&&"] = "����";
	m_FuncTransMap["AND"] = "����";
	m_FuncTransMap["OR"] = "��";
	m_FuncTransMap["=="] = "����";
	m_FuncTransMap["!="] = "������";
	m_FuncTransMap["LASTPX"] = "���¼�";
	m_FuncTransMap["CHG"] = "�ǵ���";
	m_FuncTransMap["RPS520"] = "520�������ǿ��";
	m_FuncTransMap["RPS2060"] = "2060�������ǿ��";
	m_FuncTransMap["MACD520"] = "520����MACDֵ";
	m_FuncTransMap["MACD2060"] = "2060����MACDֵ";

	m_FuncTransMap["RANK520"] = "520����ǿ������";
	m_FuncTransMap["RANK2060"] = "2060����ǿ������";
	m_FuncTransMap["POINT520"] = "520����ǿ�ȷ���";
	m_FuncTransMap["POINT2060"] = "2060����ǿ�ȷ���";

	m_FuncTransMap["RANK520L1"] = "520����ǿ����һ����ҵ����";
	m_FuncTransMap["RANK2060L1"] = "2060����ǿ����һ����ҵ����";
	m_FuncTransMap["POINT520L1"] = "520����ǿ����һ����ҵ����";
	m_FuncTransMap["POINT2060L1"] = "2060����ǿ����һ����ҵ����";
	m_FuncTransMap["RANK520L2"] = "520����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["RANK2060L2"] = "2060����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["POINT520L2"] = "520����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["POINT2060L2"] = "2060����ǿ���ڶ�����ҵ����";

	m_FuncTransMap["AMTRPS520"] = "520���ڳɽ�������ǿ��";
	m_FuncTransMap["AMTRPS2060"] = "2060���ڳɽ�������ǿ��";
	m_FuncTransMap["AMTMACD520"] = "520���ڳɽ����MACDֵ";
	m_FuncTransMap["AMTMACD2060"] = "2060���ڳɽ����MACDֵ";

	m_FuncTransMap["AMTRANK520"] = "520���ڳɽ����ǿ������";
	m_FuncTransMap["AMTRANK2060"] = "2060���ڳɽ����ǿ������";
	m_FuncTransMap["AMTPOINT520"] = "520���ڳɽ����ǿ�ȷ���";
	m_FuncTransMap["AMTPOINT2060"] = "2060���ڳɽ����ǿ�ȷ���";

	m_FuncTransMap["AMTRANK520L1"] = "520���ڳɽ����ǿ����һ����ҵ����";
	m_FuncTransMap["AMTRANK2060L1"] = "2060���ڳɽ����ǿ����һ����ҵ����";
	m_FuncTransMap["AMTPOINT520L1"] = "520���ڳɽ����ǿ����һ����ҵ����";
	m_FuncTransMap["AMTPOINT2060L1"] = "2060���ڳɽ����ǿ����һ����ҵ����";
	m_FuncTransMap["AMTRANK520L2"] = "520���ڳɽ����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["AMTRANK2060L2"] = "2060���ڳɽ����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["AMTPOINT520L2"] = "520���ڳɽ����ǿ���ڶ�����ҵ����";
	m_FuncTransMap["AMTPOINT2060L2"] = "2060���ڳɽ����ǿ���ڶ�����ҵ����";

	m_FuncTransMap["AMTRATIO"] = "�ɽ����λ";

	m_FuncTransMap["VOL"] = "�ɽ���";
	m_FuncTransMap["HIGH"] = "��߼�";
	m_FuncTransMap["LOW"] = "��ͼ�";
	m_FuncTransMap["OPEN"] = "���̼�";
	m_FuncTransMap["CLOSE"] = "���̼�";
	m_FuncTransMap["AMOUNT"] = "�ɽ����";
	m_FuncTransMap["POC"] = "���ɽ��۸�";
	m_FuncTransMap["ABV"] = "����������";
	m_FuncTransMap["ASV"] = "����������";
	m_FuncTransMap["ABO"] = "�����������";
	m_FuncTransMap["ASO"] = "������������";
	m_FuncTransMap["PBO"] = "�����������";
	m_FuncTransMap["PSO"] = "������������";



	m_FuncTransMap["ABSR"] = "��������";
	m_FuncTransMap["A2PBSR"] = "ת������������";
	m_FuncTransMap["AABSR"] = "ƽ������";
	m_FuncTransMap["POCR"] = "���ɽ��۸��";
	m_FuncTransMap[":"] = "���PARAM#1:";
	m_FuncTransMap["="] = "��ֵPARAM#1:";
}

void CFrmlCompiler::InitFuncInfo()
{
	FuncInfo fi = { "" };
	fi.name = "ABS";
	fi.frofile = "����ֵ";
	fi.description = "����ֵ.\n�÷�:\nABS(X)����X�ľ���ֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "BETWEEN";
	fi.frofile = "����";
	fi.description = "����.\n�÷�:\n"\
		"BETWEEN(A,B,C)��ʾA����B��C֮��ʱ����1,B<A<C��C<A<B,���򷵻�0";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "COS";
	fi.frofile = "����ֵ";
	fi.description = "����ֵ.\n�÷�:\nCOS(X)����X������ֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "EXP";
	fi.frofile = "��Ȼָ��";
	fi.description = "��Ȼָ��.\n�÷�:\nEXP(X)����e��X����";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "IF";
	fi.frofile = "�����ж�";
	fi.description = "����������ͬ��ֵ.\n�÷�:\n"
		" IF(X,A,B)��X��Ϊ0�򷵻�A,���򷵻�B";
	m_FuncInfoMap[FT_Select].emplace_back(fi);
	fi.name = "LN";
	fi.frofile = "��Ȼ����";
	fi.description = "��Ȼ����.\n�÷�:\nLN(X)����X��eΪ�׵Ķ���";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "LOG";
	fi.frofile = "����";
	fi.description = "����.\n�÷�:\nLOG(X,A)����X��AΪ�׵Ķ���";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MAX";
	fi.frofile = "���ֵ";
	fi.description = "���ֵ.\n�÷�:\nMAX(A,B)����A��B�еĽϴ�ֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MIN";
	fi.frofile = "��Сֵ";
	fi.description = "��Сֵ.\n�÷�:\nMIN(A,B)����A��B�еĽ�Сֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MOD";
	fi.frofile = "ģ";
	fi.description = "ȡģ.\n�÷�:\nMOD(M,N)����M����N��ģ(M����N������)";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "POW";
	fi.frofile = "��";
	fi.description = "����.\n�÷�:\nPOW(M,N))����A��B����";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "NOT";
	fi.frofile = "ȡ��";
	fi.description = "���߼���.\n�÷�:\nNOT(X)���ط�X";
	m_FuncInfoMap[FT_Logic].emplace_back(fi);
	fi.name = "RAND";
	fi.frofile = "�����";
	fi.description = "ȡ�����.\n�÷�:\nRAND(N)����һ����Χ��1-N���������";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "RANGE";
	fi.frofile = "����";
	fi.description = "�ж���������.\n�÷�:\n"\
		"RANGE(A,B,C)��ʾA����BͬʱС��Cʱ����1,���򷵻�0";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "SIN";
	fi.frofile = "����ֵ";
	fi.description = "����ֵ.\n�÷�:\nSIN(X)����X������ֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "SQRT";
	fi.frofile = "ƽ����";
	fi.description = "��ƽ����.\n�÷�:\nSQRT(X)����X��ƽ����";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "TAN";
	fi.frofile = "����ֵ";
	fi.description = "������ֵ.\n�÷�:\nTAN(X)����X����ֵ";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "+";
	fi.frofile = "��";
	fi.description = "����: X + Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "-";
	fi.frofile = "��";
	fi.description = "����: X - Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "*";
	fi.frofile = "��";
	fi.description = "����: X * Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "/";
	fi.frofile = "����";
	fi.description = "����: X / Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "<";
	fi.frofile = "С��";
	fi.description = "����: X < Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ">";
	fi.frofile = "����";
	fi.description = "����: X > Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "<=";
	fi.frofile = "С�ڵ���";
	fi.description = "����: X <= Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ">=";
	fi.frofile = "���ڵ���";
	fi.description = "����: X >= Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "==";
	fi.frofile = "����";
	fi.description = "����: X == Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "!=";
	fi.frofile = "������";
	fi.description = "����: X != Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "{";
	fi.frofile = "ע�Ϳ�ʼ����";
	fi.description = "����: {ע�Ͳ���}";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "}";
	fi.frofile = "ע�ͽ�������";
	fi.description = "����: {ע�Ͳ���}";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "AND";
	fi.frofile = "����";
	fi.description = "����: X>Y AND Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "OR";
	fi.frofile = "����";
	fi.description = "����: X>Y OR Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "&&";
	fi.frofile = "����";
	fi.description = "����: X>Y && Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "||";
	fi.frofile = "����";
	fi.description = "����: X>Y || Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "(";
	fi.frofile = "����";
	fi.description = "����: A/(B+C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ")";
	fi.frofile = "������";
	fi.description = "����: A/(B+C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ",";
	fi.frofile = "����";
	fi.description = "����: IF(A,B,C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ":";
	fi.frofile = "���";
	fi.description = "����: A:B+C";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "=";
	fi.frofile = "��ֵ";
	fi.description = "����: A=B+C";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ";";
	fi.frofile = "������";
	fi.description = "����: A=B+C;";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "LASTPX";
	fi.frofile = "���¼�";
	fi.description = "���ص�ǰ�������¼�";
	m_FuncInfoMap[FT_RtInfo].emplace_back(fi);
	fi.name = "CHG";
	fi.frofile = "�ǵ���";
	fi.description = "���ص�ǰ�ǵ���";
	m_FuncInfoMap[FT_RtInfo].emplace_back(fi);
	fi.name = "VOL";
	fi.frofile = "�ɽ���";
	fi.description = "���ظ����ڳɽ���";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "HIGH";
	fi.frofile = "��߼�";
	fi.description = "���ظ�������߼�";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "LOW";
	fi.frofile = "��ͼ�";
	fi.description = "���ظ�������ͼ�";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "OPEN";
	fi.frofile = "���̼�";
	fi.description = "���ظ����ڿ��̼�";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "CLOSE";
	fi.frofile = "���̼�";
	fi.description = "���ظ��������̼�";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "AMOUNT";
	fi.frofile = "�ɽ���";
	fi.description = "���ظ����ڳɽ���";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "POC";
	fi.frofile = "���ɽ���";
	fi.description = "���ظ��������ɽ���";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABV";
	fi.frofile = "����������";
	fi.description = "���ظ�������������ɽ���";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ASV";
	fi.frofile = "����������";
	fi.description = "���ظ��������������ɽ���";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABO";
	fi.frofile = "�������붩����";
	fi.description = "���ظ�������������ɽ��Ķ�������";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ASO";
	fi.frofile = "��������������";
	fi.description = "���ظ����������ɽ��Ķ�������";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "PBO";
	fi.frofile = "�������붩����";
	fi.description = "���ظ����ڱ�������ɽ��Ķ�������";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "PSO";
	fi.frofile = "��������������";
	fi.description = "���ظ����ڱ��������ɽ��Ķ�������";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABSR";
	fi.frofile = "��������";
	fi.description = "���ظ�������������\n"\
		"��������: ��������ɽ��� / ���������ɽ��� * 100";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "A2PBSR";
	fi.frofile = "ת������������";
	fi.description = "���ظ�����ת������������\n"
		"ת������������: ת�������� / ת�������� * 100\n"\
		"ת��������: ���������� / �������붩���� * ��������������\n"\
		"ת��������: ���������� / �������������� * �������붩����";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "AABSR";
	fi.frofile = "ƽ����������";
	fi.description = "���ظ�����ƽ������\n"
		"ƽ����������: ƽ������������ / ƽ������������ * 100\n"\
		"ƽ������������: ���������� / �������붩����\n"\
		"ƽ������������: ���������� / ��������������";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "POCR";
	fi.frofile = "���ɽ��۸��";
	fi.description = "���ظ��������ɽ��۸��\n"
		"���ɽ��۸��: (���ɽ��۸� - ���¼�) / ���¼� * 100";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "MACD520";
	fi.frofile = "���̼۵�MACD(5,20,10)";
	fi.description = "���ظ����ڵ����̼۵�MACD(5,20,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "MACD2060";
	fi.frofile = "���̼۵�MACD(20,60,10)";
	fi.description = "���ظ����ڵ����̼۵�MACD(20,60,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RPS520";
	fi.frofile = "���̼۵�5��20ʱ�䴰�ڵ����ǿ��";
	fi.description = "���ظ����ڵ����̼۵Ļ���ʱ�䴰��Ϊ5�������ڵ����̼�Ϊ20�����ǿ��";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RPS2060";
	fi.frofile = "���̼۵�20��60ʱ�䴰�ڵ����ǿ��";
	fi.description = "���ظ����ڵ����̼۵Ļ���ʱ�䴰��Ϊ20�������ڵ����̼�Ϊ60�����ǿ��";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520";
	fi.frofile = "���̼۵�RPS520����ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060";
	fi.frofile = "���̼۵�RPS2060����ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520";
	fi.frofile = "���̼۵�RPS520�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060";
	fi.frofile = "���̼۵�RPS2060�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520L1";
	fi.frofile = "���̼۵�RPS520��һ����ҵ��ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060L1";
	fi.frofile = "���̼۵�RPS2060��һ����ҵ��ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520L1";
	fi.frofile = "���̼۵�RPS520��һ����ҵ�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060L1";
	fi.frofile = "���̼۵�RPS2060��һ����ҵ�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520L2";
	fi.frofile = "���̼۵�RPS520�ڶ�����ҵ��ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060L2";
	fi.frofile = "���̼۵�RPS2060�ڶ�����ҵ��ֵ����";
	fi.description = "���ظ����ڵ����̼۵�RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520L2";
	fi.frofile = "���̼۵�RPS520�ڶ�����ҵ�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060L2";
	fi.frofile = "���̼۵�RPS2060�ڶ�����ҵ�ķ���";
	fi.description = "���ظ����ڵ����̼۵�RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTMACD520";
	fi.frofile = "�ɽ����MACD(5,20,10)";
	fi.description = "���ظ����ڵĳɽ����MACD(5,20,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTMACD2060";
	fi.frofile = "�ɽ����MACD(20,60,10)";
	fi.description = "���ظ����ڵĳɽ����MACD(20,60,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRPS520";
	fi.frofile = "�ɽ����5��20ʱ�䴰�ڵ����ǿ��";
	fi.description = "���ظ����ڵĳɽ���Ļ���ʱ�䴰��Ϊ5�������ڵĳɽ���Ϊ20�����ǿ��";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRPS2060";
	fi.frofile = "�ɽ����20��60ʱ�䴰�ڵ����ǿ��";
	fi.description = "���ظ����ڵĳɽ���Ļ���ʱ�䴰��Ϊ20�������ڵĳɽ���Ϊ60�����ǿ��";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520";
	fi.frofile = "�ɽ����RPS520����ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060";
	fi.frofile = "�ɽ����RPS2060����ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520";
	fi.frofile = "�ɽ����RPS520�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060";
	fi.frofile = "�ɽ����RPS2060�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520L1";
	fi.frofile = "�ɽ����RPS520��һ����ҵ��ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060L1";
	fi.frofile = "�ɽ����RPS2060��һ����ҵ��ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520L1";
	fi.frofile = "�ɽ����RPS520��һ����ҵ�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L1";
	fi.frofile = "�ɽ����RPS2060��һ����ҵ�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520L2";
	fi.frofile = "�ɽ����RPS520�ڶ�����ҵ��ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS520����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060L2";
	fi.frofile = "�ɽ����RPS2060�ڶ�����ҵ��ֵ����";
	fi.description = "���ظ����ڵĳɽ����RPS2060����ֵ����";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520L2";
	fi.frofile = "�ɽ����RPS520�ڶ�����ҵ�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS520�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L2";
	fi.frofile = "�ɽ����RPS2060�ڶ�����ҵ�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L2";
	fi.frofile = "�ɽ����RPS2060�ڶ�����ҵ�ķ���";
	fi.description = "���ظ����ڵĳɽ����RPS2060�Ĺ�һ������(��һ��100��,���һ��0��)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRATIO";
	fi.frofile = "�ɽ���ķ�λ";
	fi.description = "���ظ����ڵĳɽ���ķ�λ�����ݷ�ΧΪ0-100\n����:����95Ϊ�ù�Ʊ�ɽ������95%�Ĺ�Ʊ�����ù�Ʊ�ɽ���Ϊǰ5%";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);

}

BOOL CFrmlCompiler::CheckParaIsLegal(const string & paraName, ErrMsg & Error, int nParaPos)
{
	Error.nLine = nLineNo;

	if (paraName[0] >= '0' && paraName[0] <= '9')
	{
		Error.Error = GE_NumBeforeLetter;
		return FALSE;
	}

	if (m_paraMap.count(paraName) == 0
		|| nParaPos < m_paraMap[paraName])
	{
		Error.Error = GE_UndefinePara;
		return FALSE;
	}

	Error.nLine = 0;
	return TRUE;
}

void CFrmlCompiler::FindParaDefine(const string & srcStr)
{
	regex rule("[A-Z]{1}(\\w+)?(\\s+)?(:|=)");
	sregex_iterator pos(srcStr.cbegin(), srcStr.cend(), rule);
	sregex_iterator end;
	for (; pos != end; ++pos)
	{
		string buffer = pos->str();
		int nStart = 0;
		int nEnd = buffer.size() - 1;
		bool bFind = false;
		while (!bFind)
		{
			if (isalpha(buffer[nStart]))
				bFind = true;
			else
			{
				bFind = false;
				++nStart;
			}
			if (isalnum(buffer[nEnd]))
				bFind &= true;
			else
			{
				bFind = false;
				--nEnd;
			}
		}
		string para = buffer.substr(nStart, nEnd - nStart + 1);
		if (m_paraMap.count(para))
			continue;
		int nPos = srcStr.find(buffer);
		m_paraMap[para] = nPos + nEnd;
	}
}

BOOL CFrmlCompiler::SentenceHandle(const string & str, ErrMsg & Error, OutFrml& outFunc)
{
	string testStr = str;
	//�滻�����з�
	for (auto &it : testStr)
		if (it == '\r' || it == '\n')
			it = ' ';
	for (auto &it : m_ruleMap)
	{
		smatch sm;
		if (regex_match(testStr, sm, it.second))
			outFunc.type |= it.first;
	}
	if (!(outFunc.type & Snt_Assign) && !(outFunc.type & Snt_Out))
		outFunc.type |= Snt_LogicOut;
	vector<ElementType> eleVec;
	if (SpliteSenetence(str, Error, eleVec))
	{
		if (GenerateFunc(outFunc, eleVec, Error))
			return TRUE;
	}
	return FALSE;
}

BOOL CFrmlCompiler::LeftStrHandle(const string & str, ErrMsg & Error)
{
	bool bExplain = false;
	for (int i = 0; i < str.length(); ++i)
	{
		if (str[i] == '\n')
		{
			++nLineNo;
			continue;
		}

		if (!bExplain)
		{
			if (str[i] == '{')
			{
				bExplain = true;
				continue;
			}
			if (str[i] != ' ' && str[i] != '\n'
				&& str[i] != '\r' && str[i] != '\t')
			{
				Error.Error = GE_MissEnd;
				Error.nLine = nLineNo;
				Error.strEle = str;
			}
			return FALSE;
		}
		else
		{
			if (str[i] == '}')
				bExplain = false;
		}
	}
	if (bExplain)
	{
		Error.Error = GE_ExpMissEnd;
		Error.nLine = nLineNo;
		Error.strEle = str;
		return FALSE;
	}
	return TRUE;
}

BOOL CFrmlCompiler::SpliteSenetence(const string & str, ErrMsg & Error, vector<ElementType>& eleVec)
{
	int nStartPos = 0;
	int nCharPos = nCharNo - str.length();
	ElementType et = { 0 };
	string buffer = "";
	eCharType preCharType = CT_EMPTY;
	eCharType nowCharType = CT_EMPTY;
	int nBracLV = 0;
	for (int i = 0; i < str.length(); ++i)
	{
		m_charLineNo[++nCharPos] = nLineNo;
		if (str[i] == '\n')
			++nLineNo;
		et.line = nLineNo;
		nowCharType = GetCharType(str[i]);
		if (preCharType == CT_EXPSTART)
		{
			if (nowCharType == CT_EXPEND)
				preCharType = nowCharType;
			continue;
		}
		if (nowCharType != CT_CHAR && nowCharType != CT_SIGN && nowCharType != CT_NUMBER)
		{
			if (buffer != "")
			{
				et.element = buffer;
				et.type = GetElementType(buffer, Error, nCharPos - 1);
				if (et.type == ET_ILLEEGAL)
					return FALSE;
				eleVec.emplace_back(et);
			}
			if (nowCharType != CT_EMPTY && nowCharType != CT_EXPSTART)
			{
				buffer = str[i];
				et.element = buffer;
				et.type = GetElementType(buffer, Error, nCharPos);
				if (ET_LPAREN == et.type)
					++nBracLV;
				else if (ET_RPAREN == et.type)
					--nBracLV;
				if (nBracLV < 0)
				{
					Error.Error = GE_MissLeftParen;
					Error.nLine = et.line;
					Error.strEle = et.element;
					return FALSE;
				}

				if (et.type == ET_ILLEEGAL)
					return FALSE;
				eleVec.emplace_back(et);
			}
			buffer = "";
		}
		else
		{
			if (preCharType == CT_CHAR || preCharType == CT_SIGN || preCharType == CT_NUMBER)
			{
				if (nowCharType == preCharType || (preCharType + nowCharType == CT_CHAR + CT_NUMBER))
					buffer += str[i];
				else
				{
					if (buffer != "")
					{
						et.element = buffer;
						et.type = GetElementType(buffer, Error, nCharPos - 1);
						if (et.type == ET_ILLEEGAL)
							return FALSE;
						eleVec.emplace_back(et);
					}
					buffer = "";
					buffer += str[i];
				}

			}
			else
				buffer += str[i];
		}
		preCharType = nowCharType;
	}
	if (0 != nBracLV)
	{
		Error.Error = GE_MissRightParen;
		Error.nLine = et.line;
		Error.strEle = et.element;
		return FALSE;
	}
	return TRUE;
}

BOOL CFrmlCompiler::ElementOrderHandle(map<int, vector<int>>& OptOrder, vector<ElementType>& eleVec,
	ErrMsg & Error)
{
	eEleType preEleType = ET_ILLEEGAL;
	stack<int> BracStack;
	int nPreOrderLV = 0;
	for (int i = 0; i < eleVec.size(); ++i)
	{
		if (!GrammarCheck(eleVec[i], preEleType, Error))
			return FALSE;
		if (eleVec[i].type == ET_FUNC)
		{
			int nNowBarcLv = BracStack.empty() ? 0 : BracStack.top();
			nPreOrderLV = OPT_LVFUNC + nNowBarcLv;
			OptOrder[nPreOrderLV].emplace_back(i);
		}
		else if (eleVec[i].type == ET_OPEATION ||
			eleVec[i].type == ET_ASSIGN ||
			eleVec[i].type == ET_OUT ||
			eleVec[i].type == ET_EQUAL ||
			eleVec[i].type == ET_UNEQUAL)
		{
			int nOptOrder = GetOptLevel(eleVec[i].element);
			if (OPT_LVERR == nOptOrder)
			{
				Error.Error = GE_UndefineSign;
				Error.nLine = eleVec[i].line;
				Error.strEle = eleVec[i].element;
				return FALSE;
			}
			int nNowBarcLv = BracStack.empty() ? 0 : BracStack.top();
			nPreOrderLV = nOptOrder + nNowBarcLv;
			OptOrder[nPreOrderLV].emplace_back(i);
		}
		else if (eleVec[i].type == ET_LPAREN)
			BracStack.push(nPreOrderLV + OPT_LVPAREN);
		else if (eleVec[i].type == ET_RPAREN)
			BracStack.pop();
		preEleType = (eEleType)eleVec[i].type;
	}
	return TRUE;
}

BOOL CFrmlCompiler::GenerateFunc(OutFrml & outFunc, vector<ElementType>& eleVec, ErrMsg & Error)
{
	map<int, CalcCell> calMap;
	map<int, int>lBarcPos;
	map<int, eBracketType>lBarcType;
	stack<int>lBracStack;
	map<int, int>rBarcPos;
	map<int, int>rBrackPair;
	eEleType preEleType = ET_ILLEEGAL;
	eEleType preCalcEleType = ET_ILLEEGAL;
	int	nPreClacElePos = 0;
	CalcCell *pLastCell = nullptr;
	map<int, int>CalcEleRange;
	map<int, int>calcEleRangeRev;
	map<int, vector<int>> OptOrder;
	if (!ElementOrderHandle(OptOrder, eleVec, Error))
		return FALSE;
	if (OptOrder.empty())
	{
		int nBracLV = 0;
		CalcCell cc = { 0 };
		for (auto& it : eleVec)
		{
			if (ET_SYSTEMVAR == it.type)
				cc.SysVar[cc.ParaCount++] = it.element;
			else if (ET_USERVAR == it.type)
				cc.UserVar[cc.ParaCount++] = it.element;
			else if (ET_CONST == it.type)
				cc.ConstVar[cc.ParaCount++] = atof(it.element.c_str());
			else if (ET_LPAREN == it.type)
				++nBracLV;
			else if (ET_RPAREN == it.type)
				--nBracLV;
			else if (ET_SEMICOLON == it.type)
				break;
			else
			{
				Error.Error = GE_IllegalGrammar;
				Error.nLine = it.line;
				Error.strEle = it.element;
				return FALSE;

			}

		}
		if (cc.ParaCount == 1)
			outFunc.formula = cc;
		else
		{
			Error.Error = GE_IllegalGrammar;
			Error.nLine = eleVec.back().line;
			Error.strEle = eleVec.back().element;
			return FALSE;
		}

	}
	else
	{
		int nBracketLevel = 0;
		vector < pair<int, vector<int>>> OptOrderVec(OptOrder.cbegin(), OptOrder.cend());
		sort(OptOrderVec.begin(), OptOrderVec.end(),
			[](const pair<int, vector<int>>& a, const pair<int, vector<int>>& b)
		{return a.first > b.first; });
		for (auto &it : OptOrderVec)
		{
			for (auto &pos : it.second)
			{
				switch (eleVec[pos].type)
				{
				case ET_FUNC:
				{
					CalcCell cc = { 0 };
					cc.OptOrFunc = eleVec[pos].element;
					int nBracLV = 0;
					for (int i = pos + 1; i < eleVec.size(); ++i)
					{
						if (CalcEleRange.count(i))
						{
							cc.FucPara[cc.ParaCount++] = calMap[i];
							i = CalcEleRange[i];
						}
						else
						{
							if (ET_SYSTEMVAR == eleVec[i].type)
								cc.SysVar[cc.ParaCount++] = eleVec[i].element;
							else if (ET_USERVAR == eleVec[i].type)
								cc.UserVar[cc.ParaCount++] = eleVec[i].element;
							else if (ET_CONST == eleVec[i].type)
								cc.ConstVar[cc.ParaCount++] = atof(eleVec[i].element.c_str());
							else if (ET_LPAREN == eleVec[i].type)
								++nBracLV;
							else if (ET_RPAREN == eleVec[i].type)
							{
								--nBracLV;
								if (0 == nBracLV)
								{
									CalcEleRange[pos] = i;
									calcEleRangeRev[i] = pos;
									break;
								}

							}
							else if (ET_COMMA != eleVec[i].type)
							{
								Error.Error = GE_IllegalGrammar;
								Error.nLine = eleVec[i].line;
								Error.strEle = eleVec[i].element;
								return FALSE;
							}

						}
					}
					if (cc.ParaCount != m_FuncMap[eleVec[pos].element])
					{
						Error.Error = GE_ErrorParaNum;
						Error.nLine = eleVec[pos].line;
						Error.strEle = eleVec[pos].element;
						return FALSE;
					}
					calMap[pos] = cc;
					pLastCell = &calMap[pos];
				}
				break;
				case ET_OPEATION:
				case ET_EQUAL:
				case ET_UNEQUAL:
				{
					CalcCell cc = { 0 };
					cc.OptOrFunc = eleVec[pos].element;
					int nBracLV = 0;
					int nStartPos = 0;
					int nEndPos = eleVec.size() - 1;
					for (int i = pos - 1; i >= 0; --i)
					{
						if (calcEleRangeRev.count(i))
						{
							i = calcEleRangeRev[i];
							cc.FucPara[cc.ParaCount++] = calMap[i];
							nStartPos = i;
							break;
						}
						else
						{
							if (ET_SYSTEMVAR == eleVec[i].type
								|| ET_USERVAR == eleVec[i].type
								|| ET_CONST == eleVec[i].type)
							{
								if (ET_SYSTEMVAR == eleVec[i].type)
									cc.SysVar[cc.ParaCount++] = eleVec[i].element;
								else if (ET_USERVAR == eleVec[i].type)
									cc.UserVar[cc.ParaCount++] = eleVec[i].element;
								else if (ET_CONST == eleVec[i].type)
									cc.ConstVar[cc.ParaCount++] = atof(eleVec[i].element.c_str());
								if (0 == nBracLV)
								{
									nStartPos = i;
									break;
								}
							}
							else if (ET_LPAREN == eleVec[i].type)
							{
								--nBracLV;
								if (0 == nBracLV)
								{
									nStartPos = i;
									break;
								}
							}
							else if (ET_RPAREN == eleVec[i].type)
								++nBracLV;
							else
							{
								Error.Error = GE_IllegalGrammar;
								Error.nLine = eleVec[i].line;
								Error.strEle = eleVec[i].element;
								return FALSE;
							}

						}
					}
					nBracLV = 0;
					for (int i = pos + 1; i < eleVec.size(); ++i)
					{
						if (CalcEleRange.count(i))
						{
							cc.FucPara[cc.ParaCount++] = calMap[i];
							i = CalcEleRange[i];
							nEndPos = i;
							break;
						}
						else
						{
							if (ET_SYSTEMVAR == eleVec[i].type
								|| ET_USERVAR == eleVec[i].type
								|| ET_CONST == eleVec[i].type)
							{
								if (ET_SYSTEMVAR == eleVec[i].type)
									cc.SysVar[cc.ParaCount++] = eleVec[i].element;
								else if (ET_USERVAR == eleVec[i].type)
									cc.UserVar[cc.ParaCount++] = eleVec[i].element;
								else if (ET_CONST == eleVec[i].type)
									cc.ConstVar[cc.ParaCount++] = atof(eleVec[i].element.c_str());
								if (0 == nBracLV)
								{
									nEndPos = i;
									break;
								}

							}
							else if (ET_LPAREN == eleVec[i].type)
								++nBracLV;
							else if (ET_RPAREN == eleVec[i].type)
							{
								--nBracLV;
								if (0 == nBracLV)
								{
									nEndPos = i;
									break;
								}

							}
							else
							{
								Error.Error = GE_IllegalGrammar;
								Error.nLine = eleVec[i].line;
								Error.strEle = eleVec[i].element;
								return FALSE;
							}

						}
					}
					cc.ParaCount = cc.FucPara.size() + cc.SysVar.size()
						+ cc.UserVar.size() + cc.ConstVar.size();
					CalcEleRange[nStartPos] = nEndPos;
					calcEleRangeRev[nEndPos] = nStartPos;
					calMap[nStartPos] = cc;
					pLastCell = &calMap[nStartPos];
				}
				break;
				case ET_ASSIGN:
				case ET_OUT:
				{
					CalcCell cc = { 0 };
					outFunc.paraName = eleVec[pos - 1].element;
					int nOutCount = 0;
					int nBracLV = 0;
					for (int i = pos + 1; i < eleVec.size(); ++i)
					{
						if (CalcEleRange.count(i))
						{
							++nOutCount;
							outFunc.formula = calMap[i];
							i = CalcEleRange[i];
							if (0 == nBracLV)
								break;
						}
						else
						{
							if (ET_SYSTEMVAR == eleVec[i].type)
								cc.SysVar[cc.ParaCount++] = eleVec[i].element;
							else if (ET_USERVAR == eleVec[i].type)
								cc.UserVar[cc.ParaCount++] = eleVec[i].element;
							else if (ET_CONST == eleVec[i].type)
								cc.ConstVar[cc.ParaCount++] = atof(eleVec[i].element.c_str());
							else if (ET_LPAREN == eleVec[i].type)
								++nBracLV;
							else if (ET_RPAREN == eleVec[i].type)
							{
								--nBracLV;
								if (0 == nBracLV)
									break;
							}

						}

					}
					if (1 != nOutCount)
					{
						if (cc.ParaCount == 1)
							outFunc.formula = cc;
						else
						{
							Error.Error = GE_ErrorOut;
							Error.nLine = eleVec[pos].line;
							Error.strEle = eleVec[pos].element;

							return FALSE;
						}
					}
					pLastCell = nullptr;
				}
				break;
				default:
					break;
				}
			}
		}
		if (pLastCell)
			outFunc.formula = *pLastCell;
	}
	return TRUE;
}

eEleType CFrmlCompiler::GetElementType(const string & str, ErrMsg & Error, int nParaPos)
{
	eEleType et = ET_ILLEEGAL;
	Error.strEle = str;
	if (isalnum(str[0]))
	{
		if (isalpha(str[0]))
		{
			if (m_FuncMap.count(str))
				et = ET_FUNC;
			else if (m_OptSet.count(str))
				et = ET_OPEATION;
			else if (m_SystemVarSet.count(str))
				et = ET_SYSTEMVAR;
			else if (CheckParaIsLegal(str, Error, nParaPos))
				et = ET_USERVAR;
		}
		else
		{
			int i = 0;
			int nDotCount = 0;
			for (i; i < str.length(); ++i)
			{
				if (str[i] == '.' && i != str.length() - 1)
				{
					++nDotCount;
					continue;
				}
				if (str[i]<'0' || str[i] >'9')
					break;
			}
			if (i == str.length())
				et = ET_CONST;
			else
			{
				Error.nLine = nLineNo;
				Error.Error = GE_NumBeforeLetter;
				Error.strEle = str;
			}
		}
	}
	else
	{
		if (m_OptSet.count(str))
			et = ET_OPEATION;
		else if (str == "=")
			et = ET_ASSIGN;
		else if (str == ":")
			et = ET_OUT;
		else if (str == "==")
			et = ET_EQUAL;
		else if (str == "!=")
			et = ET_UNEQUAL;
		else if (str == ",")
			et = ET_COMMA;
		else if (str == ";")
			et = ET_SEMICOLON;
		else if (str == "(")
			et = ET_LPAREN;
		else if (str == ")")
			et = ET_RPAREN;
	}
	return et;
}

BOOL CFrmlCompiler::GrammarCheck(const ElementType& et, eEleType preEleType, ErrMsg &Error)
{
	BOOL bLegal = TRUE;
	switch (et.type)
	{
	case ET_FUNC:
		if (ET_SYSTEMVAR == preEleType || ET_USERVAR == preEleType
			|| ET_CONST == preEleType || ET_RPAREN == preEleType)
			bLegal = FALSE;
		break;
	case ET_SYSTEMVAR:
	case ET_USERVAR:
	case ET_CONST:
		if (ET_FUNC == preEleType || ET_SYSTEMVAR == preEleType
			|| ET_USERVAR == preEleType || ET_CONST == preEleType
			|| ET_RPAREN == preEleType)
			bLegal = FALSE;
		break;
	case ET_OPEATION:
	case ET_COMMA:
	case ET_EQUAL:
	case ET_UNEQUAL:
	case ET_SEMICOLON:
		if (ET_SYSTEMVAR != preEleType && ET_USERVAR != preEleType
			&& ET_CONST != preEleType && ET_RPAREN != preEleType)
			bLegal = FALSE;
		break;
	case ET_ASSIGN:
	case ET_OUT:
		if (ET_USERVAR != preEleType)
			bLegal = FALSE;
		break;
	case ET_LPAREN:
		if (ET_SYSTEMVAR == preEleType || ET_USERVAR == preEleType
			|| ET_CONST == preEleType || ET_OUT == ET_RPAREN)
			bLegal = FALSE;
		break;
	case ET_RPAREN:
		if (ET_SYSTEMVAR != preEleType && ET_USERVAR != preEleType
			&& ET_CONST != preEleType && ET_COMMA != preEleType
			&& ET_OUT != ET_RPAREN)
			bLegal = FALSE;
		break;
	default:
		break;
	}
	if (!bLegal)
	{
		Error.Error = GE_IllegalGrammar;
		Error.nLine = et.line;
		Error.strEle = et.element;
		return FALSE;
	}
	return TRUE;
}

eCharType CFrmlCompiler::GetCharType(char ch)
{
	eCharType CharType = CT_SIGN;
	if (isalnum(ch))
	{
		if (isalpha(ch))
			CharType = CT_CHAR;
		else
			CharType = CT_NUMBER;
	}
	else if (ch == '.')
		CharType = CT_NUMBER;
	else if (ch == ' ' || ch == '\n'
		|| ch == '\r' || ch == '\t')
		CharType = CT_EMPTY;
	else if (ch == ',')
		CharType = CT_COMMA;
	else if (ch == ';')
		CharType = CT_SEMICOLON;
	else if (ch == '(' || ch == ')')
		CharType = CT_BRACKET;
	else if (ch == '{')
		CharType = CT_EXPSTART;
	else if (ch == '}')
		CharType = CT_EXPEND;
	else
		CharType = CT_SIGN;
	return CharType;
}

int CFrmlCompiler::GetOptLevel(string opt)
{
	if (opt.length() == 1)
	{
		switch (opt[0])
		{
		case ':':
		case '=':
			return OPT_LV0;
		case '>':
		case '<':
			return OPT_LV3;
		case '+':
		case '-':
			return OPT_LV4;
		case '*':
		case '/':
			return OPT_LV5;
		default:
			break;
		}
	}
	else
	{
		if (opt == ">=" || opt == "<="
			|| opt == "==" || opt == "!=")
			return OPT_LV3;
		else if (opt == "||" || opt == "OR")
			return OPT_LV1;
		else if (opt == "&&" || opt == "AND")
			return OPT_LV2;
	}
	return OPT_LVERR;
}

eEleType CFrmlCompiler::GetElementType(const string & str)
{
	eEleType et = ET_ILLEEGAL;
	if (isalnum(str[0]))
	{
		if (isalpha(str[0]))
		{
			if (m_FuncMap.count(str))
				et = ET_FUNC;
			else if (m_OptSet.count(str))
				et = ET_OPEATION;
			else if (m_SystemVarSet.count(str))
				et = ET_SYSTEMVAR;
			else
				et = ET_USERVAR;
		}
		else
			et = ET_CONST;
	}
	else
	{
		if (m_OptSet.count(str))
			et = ET_OPEATION;
		else if (str == "=")
			et = ET_ASSIGN;
		else if (str == ":")
			et = ET_OUT;
		else if (str == "==")
			et = ET_EQUAL;
		else if (str == "!=")
			et = ET_UNEQUAL;
		else if (str == ",")
			et = ET_COMMA;
		else if (str == ";")
			et = ET_SEMICOLON;
		else if (str == "(")
			et = ET_LPAREN;
		else if (str == ")")
			et = ET_RPAREN;
	}
	return et;
}

double _CalcCell::Res(const map<string, double>& SysVarMap, const map<string, double>& UserVarMap) const
{
	if (!bFuncMapInited)
	{
		InitFuncMap();
		bFuncMapInited = TRUE;
	}
	map<int, double> varMap;
	for (auto &it : FucPara)
		varMap[it.first] = it.second.Res(SysVarMap, UserVarMap);
	for (auto &it : SysVar)
	{
		if (SysVarMap.count(it.second) == 0)
			return 0;
		varMap[it.first] = SysVarMap.at(it.second);
	}
	for (auto &it : UserVar)
		varMap[it.first] = UserVarMap.at(it.second);
	for (auto &it : ConstVar)
		varMap[it.first] = it.second;
	return (*FuncMap[OptOrFunc])(varMap);
}

bool _CalcCell::SaveToFile(ofstream & ofile, int nLevel)
{
	char cEnd = 0;
	string tag = "";
	tag = GetLevelTag("<calccell>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	ofile.write((char*)&ParaCount, sizeof(ParaCount));
	ofile.write(OptOrFunc.c_str(), OptOrFunc.size());
	tag = GetLevelTag("<funcpara>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	for (auto &it : FucPara)
	{
		tag = GetLevelTag("<funcparaitem>", nLevel);
		ofile.write(tag.c_str(), tag.size());
		ofile.write((char*)&it.first, sizeof(it.first));
		it.second.SaveToFile(ofile,nLevel + 1);
		tag = GetLevelTag("</funcparaitem>", nLevel);
		ofile.write(tag.c_str(), tag.size());
	}
	tag = GetLevelTag("</funcpara>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	tag = GetLevelTag("<sysvar>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	for (auto &it : SysVar)
	{
		ofile.write((char*)&it.first, sizeof(it.first));
		ofile.write(it.second.c_str(), it.second.size());
		ofile.write(&cEnd, 1);
	}
	tag = GetLevelTag("</sysvar>", nLevel);
	ofile.write(tag.c_str(), tag.size());

	tag = GetLevelTag("<uservar>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	for (auto &it : UserVar)
	{
		ofile.write((char*)&it.first, sizeof(it.first));
		ofile.write(it.second.c_str(), it.second.size());
		ofile.write(&cEnd, 1);
	}
	tag = GetLevelTag("</uservar>", nLevel);
	ofile.write(tag.c_str(), tag.size());

	tag = GetLevelTag("<constvar>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	for (auto &it : ConstVar)
	{
		ofile.write((char*)&it.first, sizeof(it.first));
		ofile.write((char*)&it.second, sizeof(it.second));
	}
	tag = GetLevelTag("</constvar>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	tag = GetLevelTag("</calccell>", nLevel);
	ofile.write(tag.c_str(), tag.size());
	return true;
}

bool _CalcCell::GetDataFromString(string& str, int nLevel)
{
	char cEnd = 0;
	string tag = "";
	size_t startPos = 0;
	size_t endPos = 0;
	tag = GetLevelTag("<calccell>", nLevel);
	startPos = str.find(tag) + tag.size();
	ParaCount = *(int*)(str.c_str() + startPos);
	startPos += sizeof(int);
	tag = GetLevelTag("<funcpara>", nLevel);
	endPos = str.find(tag, startPos);
	OptOrFunc = str.substr(startPos, endPos - startPos);
	startPos = endPos + tag.size();
	tag = GetLevelTag("</funcpara>", nLevel);
	endPos = str.find(tag, startPos);
	string funcParaInfo = str.substr(startPos, endPos - startPos);
	if (!funcParaInfo.empty())
	{
		size_t subStart = 0;
		size_t subEnd = 0;
		while (true)
		{
			tag = GetLevelTag("<funcparaitem>", nLevel);
			subStart = funcParaInfo.find(tag, subEnd);
			if (subStart != string::npos)
			{
				subStart += tag.size();
				int nOrder = *(int*)(funcParaInfo.c_str() + subStart);
				subStart += sizeof(int);
				tag = GetLevelTag("</funcparaitem>", nLevel);
				subEnd = funcParaInfo.find(tag, subStart);
				CalcCell cc = { 0 };
				if (cc.GetDataFromString(funcParaInfo.substr(subStart, subEnd - subStart), nLevel + 1))
					FucPara[nOrder] = cc;
			}
			else
				break;
		}
	}
	//ϵͳ����
	tag = GetLevelTag("<sysvar>", nLevel);
	startPos = str.find(tag,endPos) + tag.size();
	tag = GetLevelTag("</sysvar>", nLevel);
	endPos = str.find(tag, startPos);
	string sysVarInfo = str.substr(startPos, endPos - startPos);
	if (!sysVarInfo.empty())
	{
		size_t subStart = 0;
		size_t subEnd = 0;
		while (subStart<sysVarInfo.size())
		{
			int nOrder =  *(int*)(sysVarInfo.c_str() + subStart);
			subStart += sizeof(int);
			subEnd = sysVarInfo.find('\0', subStart);
			string varName = sysVarInfo.substr(subStart, subEnd - subStart);
			SysVar[nOrder] = varName;
			subStart = subEnd + 1;
		}
	}
	//�Զ������
	tag = GetLevelTag("<uservar>", nLevel);
	startPos = str.find(tag, endPos) + tag.size();
	tag = GetLevelTag("</uservar>", nLevel);
	endPos = str.find(tag, startPos);
	string userVarInfo = str.substr(startPos, endPos - startPos);
	if (!userVarInfo.empty())
	{
		size_t subStart = 0;
		size_t subEnd = 0;
		while (subStart<userVarInfo.size())
		{
			int nOrder = *(int*)(userVarInfo.c_str() + subStart);
			subStart += sizeof(int);
			subEnd = userVarInfo.find('\0', subStart);
			string varName = userVarInfo.substr(subStart, subEnd - subStart);
			UserVar[nOrder] = varName;
			subStart = subEnd + 1;
		}
	}
	// ��������
	tag = GetLevelTag("<constvar>", nLevel);
	startPos = str.find(tag, endPos) + tag.size();
	tag = GetLevelTag("</constvar>", nLevel);
	endPos = str.find(tag, startPos);
	string constVarInfo = str.substr(startPos, endPos - startPos);
	if (!constVarInfo.empty())
	{
		size_t subStart = 0;
		size_t subEnd = 0;
		while (subStart<constVarInfo.size())
		{
			int nOrder = *(int*)(constVarInfo.c_str() + subStart);
			subStart += sizeof(int);
			double fvar = *(double*)(constVarInfo.c_str() + subStart);
			subStart += sizeof(double);
			ConstVar[nOrder] = fvar;
		}
	}
	tag = GetLevelTag("</calccell>", nLevel);
	startPos = str.find(tag);
	if(startPos != string::npos)
		return true;
	return false;
}

string _CalcCell::GetLevelTag(string tag, int nLevel)
{
	string strLevel = to_string(nLevel);
	strLevel += '>';
	return tag.replace(tag.size() - 1, 1, strLevel);
}

void InitFuncMap()
{
	FuncMap[""] = &ITSELF;
	FuncMap["IF"] = &IF;
	FuncMap["ABS"] = &ABS;
	FuncMap["BETWEEN"] = &BETWEEN;
	FuncMap["COS"] = &COS;
	FuncMap["EXP"] = &EXP;
	FuncMap["LN"] = &LN;
	FuncMap["LOG"] = &LOG;
	FuncMap["MAX"] = &MAX;
	FuncMap["MIN"] = &MIN;
	FuncMap["MOD"] = &MOD;
	FuncMap["POW"] = &POW;
	FuncMap["NOT"] = &NOT;
	FuncMap["RAND"] = &RAND;
	FuncMap["RANGE"] = &RANGE;
	FuncMap["REVERSE"] = &REVERSE;
	FuncMap["SIN"] = &SIN;
	FuncMap["SQRT"] = &SQRT;
	FuncMap["TAN"] = &TAN;
	FuncMap["+"] = &ADD;
	FuncMap["-"] = &MINUS;
	FuncMap["*"] = &MULTIPLY;
	FuncMap["/"] = &DIVIDE;
	FuncMap[">"] = &BETTERTHAN;
	FuncMap["<"] = &LESSTHAN;
	FuncMap[">="] = &BETTEROREQUAL;
	FuncMap["<="] = &LESSOREQUAL;
	FuncMap["||"] = &OR;
	FuncMap["&&"] = &AND;
	FuncMap["AND"] = &AND;
	FuncMap["OR"] = &OR;
	FuncMap["=="] = &EQUAL;
	FuncMap["!="] = &UNEQUAL;

}

double IF(map<int, double>& paraMap)
{
	if (paraMap[0])
		return paraMap[1];
	else
		return paraMap[2];
}

double ITSELF(map<int, double>& paraMap)
{
	return paraMap[0];
}

double ABS(map<int, double>& paraMap)
{
	return abs(paraMap[0]);
}

double BETWEEN(map<int, double>& paraMap)
{
	if (paraMap[0] > paraMap[1] && paraMap[0] < paraMap[2])
		return TRUE;
	else if (paraMap[0] < paraMap[1] && paraMap[0] > paraMap[2])
		return TRUE;
	return FALSE;
}

double COS(map<int, double>& paraMap)
{
	return cos(paraMap[0]);
}

double EXP(map<int, double>& paraMap)
{
	return exp(paraMap[0]);
}

double LN(map<int, double>& paraMap)
{
	return log(paraMap[0]);
}

double LOG(map<int, double>& paraMap)
{
	return log(paraMap[0]) / log(paraMap[1]);
}

double MAX(map<int, double>& paraMap)
{
	return max(paraMap[0], paraMap[1]);
}

double MIN(map<int, double>& paraMap)
{
	return min(paraMap[0], paraMap[1]);
}

double MOD(map<int, double>& paraMap)
{
	return (int)round(paraMap[0]) % (int)round(paraMap[1]);
}

double POW(map<int, double>& paraMap)
{
	return pow(paraMap[0], paraMap[1]);
}

double NOT(map<int, double>& paraMap)
{
	return !paraMap[0];
}

double RAND(map<int, double>& paraMap)
{
	int Tick = ::GetTickCount();
	srand(Tick);
	return rand() % (int)paraMap[0] + 1;
}

double RANGE(map<int, double>& paraMap)
{
	if (paraMap[0] >= paraMap[1] && paraMap[0] <= paraMap[2])
		return TRUE;
	return FALSE;
}

double REVERSE(map<int, double>& paraMap)
{
	return 0 - paraMap[0];
}

double SIN(map<int, double>& paraMap)
{
	return sin(paraMap[0]);
}

double SQRT(map<int, double>& paraMap)
{
	return sqrt(paraMap[0]);
}

double TAN(map<int, double>& paraMap)
{
	return tan(paraMap[0]);
}

double ADD(map<int, double>& paraMap)
{
	return paraMap[0] + paraMap[1];
}

double MINUS(map<int, double>& paraMap)
{
	return paraMap[0] - paraMap[1];
}

double MULTIPLY(map<int, double>& paraMap)
{
	return paraMap[0] * paraMap[1];
}

double DIVIDE(map<int, double>& paraMap)
{
	return paraMap[0] / paraMap[1];
}

double AND(map<int, double>& paraMap)
{
	return paraMap[0] && paraMap[1];
}

double OR(map<int, double>& paraMap)
{
	return paraMap[0] || paraMap[1];
}

double BETTERTHAN(map<int, double>& paraMap)
{
	return paraMap[0] > paraMap[1];
}

double BETTEROREQUAL(map<int, double>& paraMap)
{
	return paraMap[0] >= paraMap[1];
}

double EQUAL(map<int, double>& paraMap)
{
	return paraMap[0] == paraMap[1];
}

double LESSTHAN(map<int, double>& paraMap)
{
	return paraMap[0] < paraMap[1];
}

double LESSOREQUAL(map<int, double>& paraMap)
{
	return paraMap[0] <= paraMap[1];
}

double UNEQUAL(map<int, double>& paraMap)
{
	return paraMap[0] != paraMap[1];
}

double NEG(map<int, double>& paraMap)
{
	return 0.0;
}
