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
	m_errTransMap[GE_None] = "无错误";
	m_errTransMap[GE_UndefinePara] = "未知的参数";
	m_errTransMap[GE_NumBeforeLetter] = "参数命名错误，只能以字母开头";
	m_errTransMap[GE_MissEnd] = "缺少结束符';'";
	m_errTransMap[GE_ExpMissEnd] = "注释语句缺少结尾'}'";
	m_errTransMap[GE_IllegalGrammar] = "语法错误";
	m_errTransMap[GE_UndefineSign] = "未知的符号";
	m_errTransMap[GE_ErrorCondition] = "判断条件错误";
	m_errTransMap[GE_ErrorOut] = "输出结果错误";
	m_errTransMap[GE_ErrorParaNum] = "函数参数数量错误";
	m_errTransMap[GE_MissLeftParen] = "缺少相匹配的左括号";
	m_errTransMap[GE_MissRightParen] = "缺少相匹配的右括号";

}

void CFrmlCompiler::InitFuncAndVarTrans()
{
	m_FuncTransMap["IF"] = "如果PARAM#1,返回PARAM#2,否则返回PARAM#3";
	m_FuncTransMap["ABS"] = "PARAM#1的绝对值";
	m_FuncTransMap["BETWEEN"] = "PARAM#1在PARAM#2和PARAM#3之间";
	m_FuncTransMap["COS"] = "PARAM#1的余弦";
	m_FuncTransMap["LN"] = "PARAM#1的自然对数";
	m_FuncTransMap["LOG"] = "PARAM#1的以10为底的对数";
	m_FuncTransMap["MAX"] = "PARAM#1和PARAM#2的最大值";
	m_FuncTransMap["MIN"] = "PARAM#1和PARAM#2的最小值";
	m_FuncTransMap["MOD"] = "PARAM#1和PARAM#2的模";
	m_FuncTransMap["POW"] = "PARAM#1的PARAM#2次方";
	m_FuncTransMap["NOT"] = "非PARAM#1";
	m_FuncTransMap["RAND"] = "取一个范围在1-PARAM#1的随机数";
	m_FuncTransMap["RANGE"] = "PARAM#1大于PARAM#2小于PARAM#3";
	m_FuncTransMap["REVERSE"] = "PARAM#1的相反数";
	m_FuncTransMap["SIN"] = "PARAM#1的正弦";
	m_FuncTransMap["SQRT"] = "PARAM#1的平方根";
	m_FuncTransMap["TAN"] = "PARAM#1的正切";



	m_FuncTransMap["||"] = "或";
	m_FuncTransMap["&&"] = "并且";
	m_FuncTransMap["AND"] = "并且";
	m_FuncTransMap["OR"] = "或";
	m_FuncTransMap["=="] = "等于";
	m_FuncTransMap["!="] = "不等于";
	m_FuncTransMap["LASTPX"] = "最新价";
	m_FuncTransMap["CHG"] = "涨跌幅";
	m_FuncTransMap["RPS520"] = "520周期相对强度";
	m_FuncTransMap["RPS2060"] = "2060周期相对强度";
	m_FuncTransMap["MACD520"] = "520周期MACD值";
	m_FuncTransMap["MACD2060"] = "2060周期MACD值";

	m_FuncTransMap["RANK520"] = "520周期强度排名";
	m_FuncTransMap["RANK2060"] = "2060周期强度排名";
	m_FuncTransMap["POINT520"] = "520周期强度分数";
	m_FuncTransMap["POINT2060"] = "2060周期强度分数";

	m_FuncTransMap["RANK520L1"] = "520周期强度在一级行业排名";
	m_FuncTransMap["RANK2060L1"] = "2060周期强度在一级行业排名";
	m_FuncTransMap["POINT520L1"] = "520周期强度在一级行业分数";
	m_FuncTransMap["POINT2060L1"] = "2060周期强度在一级行业分数";
	m_FuncTransMap["RANK520L2"] = "520周期强度在二级行业排名";
	m_FuncTransMap["RANK2060L2"] = "2060周期强度在二级行业排名";
	m_FuncTransMap["POINT520L2"] = "520周期强度在二级行业分数";
	m_FuncTransMap["POINT2060L2"] = "2060周期强度在二级行业分数";

	m_FuncTransMap["AMTRPS520"] = "520周期成交额的相对强度";
	m_FuncTransMap["AMTRPS2060"] = "2060周期成交额的相对强度";
	m_FuncTransMap["AMTMACD520"] = "520周期成交额的MACD值";
	m_FuncTransMap["AMTMACD2060"] = "2060周期成交额的MACD值";

	m_FuncTransMap["AMTRANK520"] = "520周期成交额的强度排名";
	m_FuncTransMap["AMTRANK2060"] = "2060周期成交额的强度排名";
	m_FuncTransMap["AMTPOINT520"] = "520周期成交额的强度分数";
	m_FuncTransMap["AMTPOINT2060"] = "2060周期成交额的强度分数";

	m_FuncTransMap["AMTRANK520L1"] = "520周期成交额的强度在一级行业排名";
	m_FuncTransMap["AMTRANK2060L1"] = "2060周期成交额的强度在一级行业排名";
	m_FuncTransMap["AMTPOINT520L1"] = "520周期成交额的强度在一级行业分数";
	m_FuncTransMap["AMTPOINT2060L1"] = "2060周期成交额的强度在一级行业分数";
	m_FuncTransMap["AMTRANK520L2"] = "520周期成交额的强度在二级行业排名";
	m_FuncTransMap["AMTRANK2060L2"] = "2060周期成交额的强度在二级行业排名";
	m_FuncTransMap["AMTPOINT520L2"] = "520周期成交额的强度在二级行业分数";
	m_FuncTransMap["AMTPOINT2060L2"] = "2060周期成交额的强度在二级行业分数";

	m_FuncTransMap["AMTRATIO"] = "成交额分位";

	m_FuncTransMap["VOL"] = "成交量";
	m_FuncTransMap["HIGH"] = "最高价";
	m_FuncTransMap["LOW"] = "最低价";
	m_FuncTransMap["OPEN"] = "开盘价";
	m_FuncTransMap["CLOSE"] = "收盘价";
	m_FuncTransMap["AMOUNT"] = "成交金额";
	m_FuncTransMap["POC"] = "最多成交价格";
	m_FuncTransMap["ABV"] = "主动买入量";
	m_FuncTransMap["ASV"] = "主动卖出量";
	m_FuncTransMap["ABO"] = "主动买入笔数";
	m_FuncTransMap["ASO"] = "主动卖出笔数";
	m_FuncTransMap["PBO"] = "被动买入笔数";
	m_FuncTransMap["PSO"] = "被动卖出笔数";



	m_FuncTransMap["ABSR"] = "主动量比";
	m_FuncTransMap["A2PBSR"] = "转换主被动量比";
	m_FuncTransMap["AABSR"] = "平均量比";
	m_FuncTransMap["POCR"] = "最多成交价格比";
	m_FuncTransMap[":"] = "输出PARAM#1:";
	m_FuncTransMap["="] = "赋值PARAM#1:";
}

void CFrmlCompiler::InitFuncInfo()
{
	FuncInfo fi = { "" };
	fi.name = "ABS";
	fi.frofile = "绝对值";
	fi.description = "绝对值.\n用法:\nABS(X)返回X的绝对值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "BETWEEN";
	fi.frofile = "介于";
	fi.description = "介于.\n用法:\n"\
		"BETWEEN(A,B,C)表示A处于B和C之间时返回1,B<A<C或C<A<B,否则返回0";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "COS";
	fi.frofile = "余弦值";
	fi.description = "余弦值.\n用法:\nCOS(X)返回X的余弦值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "EXP";
	fi.frofile = "自然指数";
	fi.description = "自然指数.\n用法:\nEXP(X)返回e的X次幂";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "IF";
	fi.frofile = "条件判断";
	fi.description = "根据条件求不同的值.\n用法:\n"
		" IF(X,A,B)若X不为0则返回A,否则返回B";
	m_FuncInfoMap[FT_Select].emplace_back(fi);
	fi.name = "LN";
	fi.frofile = "自然对数";
	fi.description = "自然对数.\n用法:\nLN(X)返回X以e为底的对数";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "LOG";
	fi.frofile = "对数";
	fi.description = "对数.\n用法:\nLOG(X,A)返回X以A为底的对数";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MAX";
	fi.frofile = "最大值";
	fi.description = "最大值.\n用法:\nMAX(A,B)返回A和B中的较大值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MIN";
	fi.frofile = "最小值";
	fi.description = "最小值.\n用法:\nMIN(A,B)返回A和B中的较小值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "MOD";
	fi.frofile = "模";
	fi.description = "取模.\n用法:\nMOD(M,N)返回M关于N的模(M除以N的余数)";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "POW";
	fi.frofile = "幂";
	fi.description = "求幂.\n用法:\nPOW(M,N))返回A的B次幂";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "NOT";
	fi.frofile = "取反";
	fi.description = "求逻辑非.\n用法:\nNOT(X)返回非X";
	m_FuncInfoMap[FT_Logic].emplace_back(fi);
	fi.name = "RAND";
	fi.frofile = "随机数";
	fi.description = "取随机数.\n用法:\nRAND(N)返回一个范围在1-N的随机整数";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "RANGE";
	fi.frofile = "区间";
	fi.description = "判断在区间内.\n用法:\n"\
		"RANGE(A,B,C)表示A大于B同时小于C时返回1,否则返回0";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "SIN";
	fi.frofile = "正弦值";
	fi.description = "正弦值.\n用法:\nSIN(X)返回X的正弦值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "SQRT";
	fi.frofile = "平方根";
	fi.description = "求平方根.\n用法:\nSQRT(X)返回X的平方根";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "TAN";
	fi.frofile = "正切值";
	fi.description = "求正切值.\n用法:\nTAN(X)返回X的切值";
	m_FuncInfoMap[FT_Math].emplace_back(fi);
	fi.name = "+";
	fi.frofile = "加";
	fi.description = "例如: X + Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "-";
	fi.frofile = "减";
	fi.description = "例如: X - Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "*";
	fi.frofile = "乘";
	fi.description = "例如: X * Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "/";
	fi.frofile = "除以";
	fi.description = "例如: X / Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "<";
	fi.frofile = "小于";
	fi.description = "例如: X < Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ">";
	fi.frofile = "大于";
	fi.description = "例如: X > Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "<=";
	fi.frofile = "小于等于";
	fi.description = "例如: X <= Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ">=";
	fi.frofile = "大于等于";
	fi.description = "例如: X >= Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "==";
	fi.frofile = "等于";
	fi.description = "例如: X == Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "!=";
	fi.frofile = "不等于";
	fi.description = "例如: X != Y";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "{";
	fi.frofile = "注释开始符号";
	fi.description = "例如: {注释部分}";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "}";
	fi.frofile = "注释结束符号";
	fi.description = "例如: {注释部分}";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "AND";
	fi.frofile = "并且";
	fi.description = "例如: X>Y AND Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "OR";
	fi.frofile = "或者";
	fi.description = "例如: X>Y OR Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "&&";
	fi.frofile = "并且";
	fi.description = "例如: X>Y && Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "||";
	fi.frofile = "或者";
	fi.description = "例如: X>Y || Y>Z";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "(";
	fi.frofile = "括号";
	fi.description = "例如: A/(B+C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ")";
	fi.frofile = "反括号";
	fi.description = "例如: A/(B+C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ",";
	fi.frofile = "逗号";
	fi.description = "例如: IF(A,B,C)";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ":";
	fi.frofile = "输出";
	fi.description = "例如: A:B+C";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "=";
	fi.frofile = "赋值";
	fi.description = "例如: A=B+C";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = ";";
	fi.frofile = "语句结束";
	fi.description = "例如: A=B+C;";
	m_FuncInfoMap[FT_Operation].emplace_back(fi);
	fi.name = "LASTPX";
	fi.frofile = "最新价";
	fi.description = "返回当前行情最新价";
	m_FuncInfoMap[FT_RtInfo].emplace_back(fi);
	fi.name = "CHG";
	fi.frofile = "涨跌幅";
	fi.description = "返回当前涨跌幅";
	m_FuncInfoMap[FT_RtInfo].emplace_back(fi);
	fi.name = "VOL";
	fi.frofile = "成交量";
	fi.description = "返回该周期成交量";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "HIGH";
	fi.frofile = "最高价";
	fi.description = "返回该周期最高价";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "LOW";
	fi.frofile = "最低价";
	fi.description = "返回该周期最低价";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "OPEN";
	fi.frofile = "开盘价";
	fi.description = "返回该周期开盘价";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "CLOSE";
	fi.frofile = "收盘价";
	fi.description = "返回该周期收盘价";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "AMOUNT";
	fi.frofile = "成交额";
	fi.description = "返回该周期成交额";
	m_FuncInfoMap[FT_BarInfo].emplace_back(fi);
	fi.name = "POC";
	fi.frofile = "最多成交价";
	fi.description = "返回该周期最多成交价";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABV";
	fi.frofile = "主动买入量";
	fi.description = "返回该周期主动买入成交量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ASV";
	fi.frofile = "主动卖出量";
	fi.description = "返回该周期主动卖出成交量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABO";
	fi.frofile = "主动买入订单量";
	fi.description = "返回该周期主动买入成交的订单数量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ASO";
	fi.frofile = "主动卖出订单量";
	fi.description = "返回该周期卖出成交的订单数量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "PBO";
	fi.frofile = "被动买入订单量";
	fi.description = "返回该周期被动买入成交的订单数量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "PSO";
	fi.frofile = "被动卖出订单量";
	fi.description = "返回该周期被动卖出成交的订单数量";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "ABSR";
	fi.frofile = "主动量比";
	fi.description = "返回该周期主动量比\n"\
		"主动量比: 主动买入成交量 / 主动卖出成交量 * 100";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "A2PBSR";
	fi.frofile = "转换主被动量比";
	fi.description = "返回该周期转换主被动量比\n"
		"转换主被动量比: 转换买入量 / 转换卖出量 * 100\n"\
		"转换买入量: 主动买入量 / 主动买入订单数 * 被动卖出订单数\n"\
		"转换卖出量: 主动卖出量 / 主动卖出订单数 * 被动买入订单数";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "AABSR";
	fi.frofile = "平均主动量比";
	fi.description = "返回该周期平均量比\n"
		"平均主动量比: 平均主动买入量 / 平均主动卖出量 * 100\n"\
		"平均主动买入量: 主动买入量 / 主动买入订单数\n"\
		"平均主动卖出量: 主动卖出量 / 主动卖出订单数";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "POCR";
	fi.frofile = "最多成交价格比";
	fi.description = "返回该周期最多成交价格比\n"
		"最多成交价格比: (最多成交价格 - 最新价) / 最新价 * 100";
	m_FuncInfoMap[FT_FundFlow].emplace_back(fi);
	fi.name = "MACD520";
	fi.frofile = "收盘价的MACD(5,20,10)";
	fi.description = "返回该周期的收盘价的MACD(5,20,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "MACD2060";
	fi.frofile = "收盘价的MACD(20,60,10)";
	fi.description = "返回该周期的收盘价的MACD(20,60,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RPS520";
	fi.frofile = "收盘价的5、20时间窗口的相对强度";
	fi.description = "返回该周期的收盘价的滑动时间窗口为5、长周期的收盘价为20的相对强度";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RPS2060";
	fi.frofile = "收盘价的20、60时间窗口的相对强度";
	fi.description = "返回该周期的收盘价的滑动时间窗口为20、长周期的收盘价为60的相对强度";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520";
	fi.frofile = "收盘价的RPS520的数值排名";
	fi.description = "返回该周期的收盘价的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060";
	fi.frofile = "收盘价的RPS2060的数值排名";
	fi.description = "返回该周期的收盘价的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520";
	fi.frofile = "收盘价的RPS520的分数";
	fi.description = "返回该周期的收盘价的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060";
	fi.frofile = "收盘价的RPS2060的分数";
	fi.description = "返回该周期的收盘价的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520L1";
	fi.frofile = "收盘价的RPS520在一级行业数值排名";
	fi.description = "返回该周期的收盘价的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060L1";
	fi.frofile = "收盘价的RPS2060在一级行业数值排名";
	fi.description = "返回该周期的收盘价的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520L1";
	fi.frofile = "收盘价的RPS520在一级行业的分数";
	fi.description = "返回该周期的收盘价的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060L1";
	fi.frofile = "收盘价的RPS2060在一级行业的分数";
	fi.description = "返回该周期的收盘价的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK520L2";
	fi.frofile = "收盘价的RPS520在二级行业数值排名";
	fi.description = "返回该周期的收盘价的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "RANK2060L2";
	fi.frofile = "收盘价的RPS2060在二级行业数值排名";
	fi.description = "返回该周期的收盘价的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT520L2";
	fi.frofile = "收盘价的RPS520在二级行业的分数";
	fi.description = "返回该周期的收盘价的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "POINT2060L2";
	fi.frofile = "收盘价的RPS2060在二级行业的分数";
	fi.description = "返回该周期的收盘价的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTMACD520";
	fi.frofile = "成交额的MACD(5,20,10)";
	fi.description = "返回该周期的成交额的MACD(5,20,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTMACD2060";
	fi.frofile = "成交额的MACD(20,60,10)";
	fi.description = "返回该周期的成交额的MACD(20,60,10)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRPS520";
	fi.frofile = "成交额的5、20时间窗口的相对强度";
	fi.description = "返回该周期的成交额的滑动时间窗口为5、长周期的成交额为20的相对强度";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRPS2060";
	fi.frofile = "成交额的20、60时间窗口的相对强度";
	fi.description = "返回该周期的成交额的滑动时间窗口为20、长周期的成交额为60的相对强度";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520";
	fi.frofile = "成交额的RPS520的数值排名";
	fi.description = "返回该周期的成交额的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060";
	fi.frofile = "成交额的RPS2060的数值排名";
	fi.description = "返回该周期的成交额的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520";
	fi.frofile = "成交额的RPS520的分数";
	fi.description = "返回该周期的成交额的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060";
	fi.frofile = "成交额的RPS2060的分数";
	fi.description = "返回该周期的成交额的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520L1";
	fi.frofile = "成交额的RPS520在一级行业数值排名";
	fi.description = "返回该周期的成交额的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060L1";
	fi.frofile = "成交额的RPS2060在一级行业数值排名";
	fi.description = "返回该周期的成交额的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520L1";
	fi.frofile = "成交额的RPS520在一级行业的分数";
	fi.description = "返回该周期的成交额的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L1";
	fi.frofile = "成交额的RPS2060在一级行业的分数";
	fi.description = "返回该周期的成交额的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK520L2";
	fi.frofile = "成交额的RPS520在二级行业数值排名";
	fi.description = "返回该周期的成交额的RPS520的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRANK2060L2";
	fi.frofile = "成交额的RPS2060在二级行业数值排名";
	fi.description = "返回该周期的成交额的RPS2060的数值排名";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT520L2";
	fi.frofile = "成交额的RPS520在二级行业的分数";
	fi.description = "返回该周期的成交额的RPS520的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L2";
	fi.frofile = "成交额的RPS2060在二级行业的分数";
	fi.description = "返回该周期的成交额的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTPOINT2060L2";
	fi.frofile = "成交额的RPS2060在二级行业的分数";
	fi.description = "返回该周期的成交额的RPS2060的归一化分数(第一名100分,最后一名0分)";
	m_FuncInfoMap[FT_RPS].emplace_back(fi);
	fi.name = "AMTRATIO";
	fi.frofile = "成交额的分位";
	fi.description = "返回该周期的成交额的分位，数据范围为0-100\n例如:返回95为该股票成交额大于95%的股票，即该股票成交额为前5%";
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
	//替换掉换行符
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
	//系统参数
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
	//自定义参数
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
	// 常数参数
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
