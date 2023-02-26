#pragma once
#include<set>
#include<string>
#include <deque>
#include <regex>
#include <map>

using namespace std;

typedef double(*PFUNC)(map<int, double>&);

enum eFuncType
{
	FT_None = 0,
	FT_BarInfo,
	FT_Logic,
	FT_Select,
	FT_Math,
	FT_RtInfo,
	FT_Operation,
	FT_RPS,
	FT_FundFlow,
	FT_Count,
};

enum eGraError
{
	GE_None = 0,
	GE_UndefinePara,
	GE_NumBeforeLetter,
	GE_MissEnd,
	GE_ExpMissEnd,
	GE_IllegalGrammar,
	GE_UndefineSign,
	GE_ErrorCondition,
	GE_ErrorOut,
	GE_ErrorParaNum,
	GE_MissLeftParen,
	GE_MissRightParen,
};

enum eOptLevel
{
	OPT_LV0 = 0,
	OPT_LV1,
	OPT_LV2,
	OPT_LV3,
	OPT_LV4,
	OPT_LV5,
	OPT_LV6,
	OPT_LV7,
	OPT_LV8,
	OPT_LV9,
	OPT_LVERR = 999,

};

const int OPT_LVFUNC = OPT_LV6;
const int OPT_LVPAREN = OPT_LV7;
enum eSntcType
{
	Snt_NULL = 0,
	Snt_Assign = 0x01,			//��ֵ
	Snt_Out = 0x02,				//���
	Snt_LogicOut = 0x04,		//�߼����
};

enum eEleType
{
	ET_ILLEEGAL = 0,	//�Ƿ�
	ET_FUNC,			//����
	ET_SYSTEMVAR,		//ϵͳ����
	ET_USERVAR,			//�û�����
	ET_CONST,			//����
	ET_OPEATION,		//�����
	ET_ASSIGN,			//=
	ET_OUT,				//:
	ET_COMMA,			//,
	ET_SEMICOLON,		//;
	ET_LPAREN,			//(
	ET_RPAREN,			//)
	ET_EQUAL,			//==
	ET_UNEQUAL,			//!=
};

enum eCharType
{
	CT_EMPTY,		//��Ԫ��
	CT_CHAR,		//��ĸ
	CT_NUMBER,		//����
	CT_SIGN,		//����
	CT_BRACKET,		//����
	CT_COMMA,		//����
	CT_SEMICOLON,	//�ֺ�
	CT_EXPSTART,	//ע����俪ʼ
	CT_EXPEND,		//ע��������
};

typedef struct _ErrMsg
{
	int Error;
	int nLine;
	string strEle;
	string strSent;
}ErrMsg;

typedef struct _FuncInfo
{
	string name;
	string frofile;
	string description;
}FuncInfo;

typedef struct _CalcCell
{
	int ParaCount;
	string OptOrFunc;
	map<int, _CalcCell> FucPara;
	map<int, string> SysVar;
	map<int, string> UserVar;
	map<int, double> ConstVar;
	double Res(const map<string, double>& SysVarMap,
		const map<string, double>& UserVarMap) const;
	bool SaveToFile(ofstream & ofile, int nLevel);
	bool GetDataFromString(string& str,int nLevel);
protected:
	string GetLevelTag(string tag, int nLevel);
}CalcCell;

typedef struct _OutFrml
{
	int type;
	string paraName;
	CalcCell formula;
	//string srcSent;
}OutFrml;

typedef struct _CellType
{
	int type;
	int line;
	string element;
}ElementType;

enum eBracketType
{
	BT_FUNC,
	BT_PARA,
};
void   InitFuncMap();
double IF(map<int, double>& paraMap);
double ITSELF(map<int, double>& paraMap);
double ABS(map<int, double>& paraMap);
double BETWEEN(map<int, double>& paraMap);
double COS(map<int, double>& paraMap);
double EXP(map<int, double>& paraMap);
double LN(map<int, double>& paraMap);
double LOG(map<int, double>& paraMap);
double MAX(map<int, double>& paraMap);
double MIN(map<int, double>& paraMap);
double MOD(map<int, double>& paraMap);
double POW(map<int, double>& paraMap);
double NOT(map<int, double>& paraMap);
double RAND(map<int, double>& paraMap);
double RANGE(map<int, double>& paraMap);
double REVERSE(map<int, double>& paraMap);
double SIN(map<int, double>& paraMap);
double SQRT(map<int, double>& paraMap);
double TAN(map<int, double>& paraMap);
double ADD(map<int, double>& paraMap);
double MINUS(map<int, double>& paraMap);
double MULTIPLY(map<int, double>& paraMap);
double DIVIDE(map<int, double>& paraMap);
double AND(map<int, double>& paraMap);
double OR(map<int, double>& paraMap);
double BETTERTHAN(map<int, double>& paraMap);
double BETTEROREQUAL(map<int, double>& paraMap);
double EQUAL(map<int, double>& paraMap);
double LESSTHAN(map<int, double>& paraMap);
double LESSOREQUAL(map<int, double>& paraMap);
double UNEQUAL(map<int, double>& paraMap);

class CFrmlCompiler
{
public:
	CFrmlCompiler();
	~CFrmlCompiler();
public:
	BOOL ExplainString(string &str, std::set<string>  setPara,
		ErrMsg& Error, vector<OutFrml>& funcVec);
	eEleType GetStrEleType(const string& str);
	BOOL SpliteSenetence(const string& str, vector<ElementType>& eleVec);
	static void InitSetting();
	static string TransError(eGraError err);
	static string TransFuncAndVar(string str,int & nParaNum);
	static const map<eFuncType, vector<FuncInfo>>& GetFuncInfo();
protected:
	static void InitRules();
	static void InitOptAndFunc();
	static void InitSystemVar();
	static void InitEmptyChar();
	static void InitErrorTrans();
	static void InitFuncAndVarTrans();
	static void InitFuncInfo();
	BOOL CheckParaIsLegal(const string& paraName, ErrMsg& Error, int nParaPos);
	void FindParaDefine(const string& srcStr);
	BOOL SentenceHandle(const string& str, ErrMsg& Error, OutFrml& outFunc);
	BOOL LeftStrHandle(const string& str, ErrMsg& Error);
	BOOL SpliteSenetence(const string& str, ErrMsg& Error, vector<ElementType>& eleVec);
	BOOL ElementOrderHandle(map<int, vector<int>>& OptOrder, vector<ElementType>& eleVec,
		ErrMsg& Error);
	BOOL GenerateFunc(OutFrml& outFunc, vector<ElementType>& eleVec, ErrMsg& Error);
	eEleType GetElementType(const string& str, ErrMsg& Error, int nParaPos);
	BOOL GrammarCheck(const ElementType& et, eEleType preEleType, ErrMsg &Error);
	eCharType GetCharType(char ch);
	int	 GetOptLevel(string opt);
	eEleType GetElementType(const string& str);
protected:
	static map<eSntcType, regex> m_ruleMap;
	static map<string, int> m_FuncMap;
	static set<string> m_OptSet;
	static set<string> m_SystemVarSet;
	static set<char> m_EmptySet;
	static map<eGraError, string> m_errTransMap;
	static map<string, string> m_FuncTransMap;
	static map<eFuncType, vector<FuncInfo>> m_FuncInfoMap;
	static BOOL m_bInited;
	map<string, int> m_paraMap;
	map<int, int> m_charLineNo;
	int nLineNo;
	int nCharNo;
};

inline string CFrmlCompiler::TransError(eGraError err)
{
	return m_errTransMap[err];
}

inline string CFrmlCompiler::TransFuncAndVar(string str,int & nParaNum)
{
	nParaNum = -1;
	if (m_FuncTransMap.count(str))
	{
		if (m_FuncMap.count(str))
			nParaNum = m_FuncMap[str];
		return m_FuncTransMap[str];
	}
	return "";
}

inline const map<eFuncType, vector<FuncInfo>>& CFrmlCompiler::GetFuncInfo()
{
	return m_FuncInfoMap;
}
