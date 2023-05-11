#pragma once
#include<set>
using std::set;
#include"FrmlCompiler.h"

typedef struct _paraRange
{
	double min;
	double max;
	double def;
}paraRage;

enum eFrmlUse
{
	eFU_Target,
	eFU_Filter,
};

enum eStkFilterFrmlType
{
	SFFT_IndexCondition = 0,
	SFFT_Fundamental,
	SFFT_RealTime,
	SFFT_Tendency,
	SFFT_Form,
	SFFT_Other,
	SFFT_Count,
};

typedef struct _FrmlFullInfoType
{
	string name;
	string descption;
	vector<string> para;
	map<string, paraRage> paraSetting;
	string code;
	vector<OutFrml> formula;
	string paraElf;
	string usage;
	eFrmlUse useType;
	int subdivision;
	BOOL sysFrml;
}FrmlFullInfo;


class CFrmlManager
{
public:
	CFrmlManager();
	~CFrmlManager();
	static BOOL InitFrmlManage();
	static BOOL CheckNameIsUseful(string strName);
	static BOOL AddNewFormula(FrmlFullInfo& info);
	static BOOL ChangeFormula(string oldName, FrmlFullInfo& info);
	static BOOL DeleteFormula(string oldName);
	static BOOL SaveFormulas();
	static BOOL GetFormulas();
	static map<string, FrmlFullInfo> GetFormulaMap();
	static const FrmlFullInfo & GetFormula(string name);
	static void IncreaseFrmlUseCount(string name);
	static void DecreaseFrmlUseCount(string name);
	static int GetFrmlUseCount(string name);

protected:
	static BOOL SaveFormula(ofstream &ofile, FrmlFullInfo& info);
	static BOOL GetFormula(string& str, FrmlFullInfo& info);
	static BOOL WriteString(ofstream& ofile,string begTag,string endTag,char* msg,size_t size);
	static string GetStringData(string& str, string begTag, string endTag);
	static BOOL InitSysFrml();
	static BOOL InitIndexConditionFrml();
	static BOOL InitFundamentalFrml();
	static BOOL InitRealTimeFrml();
	static BOOL InitTendencyFrml();
	static BOOL InitFormFrml();
	static BOOL InitOtherFrml();
	static void Lock();
	static void UnLock();
protected:
	static set<string> m_FrmlName;
	static map<string, FrmlFullInfo> m_FrmlMap;
	static map<string, int> m_FrmlUseMap;
	static CRITICAL_SECTION m_cs;
};


/*
SFFT_RealTime,
SFFT_Tendency,
SFFT_Form,
SFFT_Other,
SFFT_Count,
*/