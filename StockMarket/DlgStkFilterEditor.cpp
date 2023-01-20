#include "stdafx.h"
#include "DlgStkFilterEditor.h"
#include "DlgFuncInsert.h"
#include <queue>
#include <stack>


CDlgStkFilterEditor::CDlgStkFilterEditor(HWND hParWnd, int nSubdiv) : 
	SHostWnd(_T("LAYOUT:dlg_FilterFrmlEditor"))
{
	m_bDirty = FALSE;
	m_pDlgInsert = nullptr;
	m_FrmlInfo = FrmlFullInfo{ "" };
	m_FrmlInfo.useType = eFU_Filter;
	m_bNew = TRUE;
	m_nSubdiv = nSubdiv;
	bChangeFromInit = FALSE;
	m_hParWnd = hParWnd;
	m_bFromStkFilter = FALSE;
}

SOUI::CDlgStkFilterEditor::CDlgStkFilterEditor(HWND hParWnd, FrmlFullInfo & info, BOOL bFromStkFilter) :
	SHostWnd(_T("LAYOUT:dlg_FilterFrmlEditor"))
{
	m_bDirty = FALSE;
	m_pDlgInsert = nullptr;
	m_bNew = FALSE;
	m_FrmlInfo = info;
	bChangeFromInit = TRUE;
	m_hParWnd = hParWnd;
	m_bFromStkFilter = bFromStkFilter;
}


CDlgStkFilterEditor::~CDlgStkFilterEditor()
{
}

BOOL CDlgStkFilterEditor::OnInitDialog(EventArgs * e)
{
	InitSubControl();
	CFrmlCompiler::InitSetting();
	if (m_bNew)
		m_pCbxType->SetCurSel(m_nSubdiv);
	else
	{
		SStringW tmp;
		m_pEditName->SetWindowTextW(StrA2StrW(m_FrmlInfo.name.c_str()));
		m_pEditDscp->SetWindowTextW(StrA2StrW(m_FrmlInfo.descption.c_str()));
		for (int i = 0; i < m_FrmlInfo.para.size(); ++i)
		{
			string paraName = m_FrmlInfo.para[i];
			m_ParaSet.insert(paraName);
			m_pEditParaName[i]->SetWindowTextW(StrA2StrW(paraName.c_str()));
			m_pEditParaMin[i]->SetWindowTextW(
				tmp.Format(L"%g", m_FrmlInfo.paraSetting[paraName].min));
			m_pEditParaMax[i]->SetWindowTextW(
				tmp.Format(L"%g", m_FrmlInfo.paraSetting[paraName].max));
			m_pEditParaDef[i]->SetWindowTextW(
				tmp.Format(L"%g", m_FrmlInfo.paraSetting[paraName].def));
		}
		m_pCbxType->SetCurSel(m_FrmlInfo.subdivision);
		m_pEditParam->SetWindowTextW(StrA2StrW(m_FrmlInfo.paraElf.c_str()));
		m_PEditExplain->SetWindowTextW(StrA2StrW(m_FrmlInfo.usage.c_str()));
		m_pReFrml->SetWindowTextW(StrA2StrW(m_FrmlInfo.code.c_str()));

	}
	return 0;
}

bool CDlgStkFilterEditor::OnEditFrmlChange(EventArgs * e)
{
	EventRENotify *pEvt = (EventRENotify*)e;
	if (pEvt->iNotify != EN_CHANGE)
		return TRUE;
	SCapsRichEdit* pEdit = (SCapsRichEdit*)pEvt->sender;
	if (bChangeFromInit)
		bChangeFromInit = FALSE;
	else
	{
		m_bDirty = TRUE;
		if (!pEdit->GetEditDirty())
			return TRUE;
		if (pEdit->GetImmInput())
		{
			pEdit->ClearDirty();
			return TRUE;
		}
	}
	SStringW str = pEdit->GetWindowText();
	str.MakeUpper();
	SStringW strTrans = TransFrml(str);
	m_pEditTrans->SetWindowTextW(strTrans);
	m_pTabCtrl->SetCurSel(0);

	if (str.IsEmpty())
	{
		pEdit->ClearDirty();
		return TRUE;
	}
	UpdateStrFormat(pEdit, str);
	pEdit->ClearDirty();
	return TRUE;
}


void CDlgStkFilterEditor::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void CDlgStkFilterEditor::InitSubControl()
{
	m_pEditName = FindChildByName2<SCapsEdit>(L"edit_name");
	if (m_bFromStkFilter)
		m_pEditName->EnableWindow(FALSE, TRUE);
	m_pEditDscp = FindChildByName2<SCapsEdit>(L"edit_dscp");
	SStringW strTmp;
	for (int i = 0; i < 16; ++i)
	{
		m_pEditParaName[i] = FindChildByName2<SCapsEdit>(strTmp.Format(L"edit_para%dname", i + 1));
		m_pEditParaMin[i] = FindChildByName2<SCapsEdit>(strTmp.Format(L"edit_para%dmin", i + 1));
		m_pEditParaMax[i] = FindChildByName2<SCapsEdit>(strTmp.Format(L"edit_para%dmax", i + 1));
		m_pEditParaDef[i] = FindChildByName2<SCapsEdit>(strTmp.Format(L"edit_para%ddef", i + 1));
	}
	m_pCbxType = FindChildByName2<SComboBox>(L"cbx_type");
	m_pEditTrans = FindChildByName2<SCapsEdit>(L"edit_trans");
	m_pEditResult = FindChildByName2<SCapsEdit>(L"edit_result");
	m_pEditParam = FindChildByName2<SCapsEdit>(L"edit_param");
	m_PEditExplain = FindChildByName2<SCapsEdit>(L"edit_explain");
	m_pReFrml = FindChildByName2<SCapsRichEdit>(L"re_frml");
	m_pTabCtrl = FindChildByName2<STabCtrl>(L"tab_ctrl");
	m_pReFrml->GetEventSet()->subscribeEvent(
		EventRENotify::EventID,
		Subscriber(&CDlgStkFilterEditor::OnEditFrmlChange, this));
	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	m_pReFrml->GetCharFormat(&cf);
	cf.dwMask |= CFM_ALL2;
	cf.dwEffects |= CFE_ALLCAPS;
	cf.dwMask &= ~CFM_BACKCOLOR;
	m_pReFrml->SetCharFormat(&cf);
}

LRESULT CDlgStkFilterEditor::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case FNCMsg_InsertFunc:
	{
		SStringW str = *(SStringW*)wp;
		m_pReFrml->InsertStr(str);
	}
	break;
	default:
		break;
	}
	return 0;
}

BOOL CDlgStkFilterEditor::SetExplianType(SCapsRichEdit* pEdit, DWORD dwStart, DWORD dwEnd)
{
	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	pEdit->GetCharFormat(&cf);
	cf.crTextColor = RGB(100, 100, 100);
	pEdit->SSendMessage(EM_SETSEL, dwStart, dwEnd);
	pEdit->SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	return TRUE;
}

BOOL CDlgStkFilterEditor::UpdateStrFormat(SCapsRichEdit* pEdit, SStringW & str)
{
	int nStart = 0;
	BOOL bInExplain = FALSE;
	int nPos = 0;
	DWORD dwStart = 0, dwEnd = 0;
	BOOL bIsInPara = FALSE;
	pEdit->SSendMessage(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	SetStrFormatDefault(pEdit, 0, str.GetLength());
	int nLineCount = 0;
	for (nPos; nPos < str.GetLength(); ++nPos)
	{
		if (str[nPos] == '\n')
			nLineCount++;
		if (bInExplain)
		{
			if (str[nPos] == '}')
			{
				bInExplain = FALSE;
				SetExplianType(pEdit, nStart - nLineCount, nPos + 1 - nLineCount);
				SetStrFormatDefault(pEdit, nPos + 1 - nLineCount, nPos + 1 - nLineCount);
			}
		}
		else
		{
			if (str[nPos] == '{')
			{
				bInExplain = TRUE;
				bIsInPara = FALSE;
				nStart = nPos;
			}
			else
			{
				if (isalnum(str[nPos]) || str[nPos] == '.')
				{
					if (!bIsInPara)
					{
						bIsInPara = TRUE;
						nStart = nPos;
					}
				}
				else
				{
					if (bIsInPara)
					{
						bIsInPara = FALSE;
						string tmpStr = StrW2StrA(str.Mid(nStart, nPos - nStart));
						eEleType et = m_FrmlCmpl.GetStrEleType(tmpStr);
						if (et != ET_ILLEEGAL)
							SetEleColor(pEdit, et, nStart - nLineCount, nPos - nLineCount);
					}
				}
			}
		}
	}
	if (bInExplain)
	{
		SetExplianType(pEdit, nStart - nLineCount, str.GetLength() - nLineCount);
		SetExplianType(pEdit, str.GetLength() - nLineCount, str.GetLength() - nLineCount);
	}
	else
	{
		if (bIsInPara)
		{
			string tmpStr = StrW2StrA(str.Mid(nStart, nPos - nStart));
			eEleType et = m_FrmlCmpl.GetStrEleType(tmpStr);
			if (et != ET_ILLEEGAL)
				SetEleColor(pEdit, et, nStart - nLineCount, nPos - nLineCount);
		}
		pEdit->SSendMessage(EM_SETSEL, dwStart, dwEnd);
		SetStrFormatDefault(pEdit, dwEnd, dwEnd);
	}
	return TRUE;
}

BOOL CDlgStkFilterEditor::SetStrFormatDefault(SCapsRichEdit* pEdit, DWORD dwStart, DWORD dwEnd)
{
	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	pEdit->GetCharFormat(&cf);
	cf.crTextColor = RGB(0, 0, 0);
	pEdit->SSendMessage(EM_SETSEL, dwStart, dwEnd);
	pEdit->SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	return TRUE;
}

BOOL CDlgStkFilterEditor::SetEleColor(SCapsRichEdit* pEdit, eEleType et, DWORD dwStart, DWORD dwEnd)
{
	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	pEdit->GetCharFormat(&cf);
	if (ET_FUNC == et)
		cf.crTextColor = RGB(255, 100, 100);
	else if (ET_SYSTEMVAR == et)
		cf.crTextColor = RGB(100, 200, 240);
	else if (ET_OPEATION == et)
		cf.crTextColor = RGB(100, 100, 255);
	else if (ET_CONST == et)
		cf.crTextColor = RGB(255, 100, 255);

	pEdit->SSendMessage(EM_SETSEL, dwStart, dwEnd);
	pEdit->SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	return TRUE;
}

SStringW CDlgStkFilterEditor::TransFrml(SStringW & str)
{
	vector<ElementType> eleVec;
	string strFrml = StrW2StrA(str);
	m_FrmlCmpl.SpliteSenetence(strFrml, eleVec);
	vector<int> TransOrder;
	map<int, string> TransMsg;
	stack<pair<int, string>> FuncQue;
	map<int, string> FuncMap;
	stack<int> LParenQue;
	map<int, map<int, vector<int>>> funcParaMap;
	map<int, int> funcParaNumMap;
	set<int> FuncParen;
	map<int, int> FuncParaCount;
	bool bPreEleIsFunc = false;
	SStringW strTrans = L"";
	for (int i = 0; i < eleVec.size(); ++i)
	{
		auto nowFunc = FuncQue.empty() ? pair<int, string>{0, ""} : FuncQue.top();
		if (eleVec[i].type == ET_LPAREN)
		{
			LParenQue.push(i);
			if (bPreEleIsFunc)
			{
				FuncParen.insert(i);
				bPreEleIsFunc = false;
			}
			else
			{
				TransMsg[i] = "(";
				if (nowFunc.second != "")
					funcParaMap[nowFunc.first][FuncParaCount[nowFunc.first]].emplace_back(i);
				else
					TransOrder.emplace_back(i);
			}
		}
		else if (eleVec[i].type == ET_RPAREN)
		{
			int nLParen = LParenQue.empty() ? -1 : LParenQue.top();
			LParenQue.pop();
			if (!FuncParen.count(nLParen))
			{
				TransMsg[i] = ")";
				if (nowFunc.second != "")
					funcParaMap[nowFunc.first][FuncParaCount[nowFunc.first]].emplace_back(i);
				else
					TransOrder.emplace_back(i);
			}
			else
				FuncQue.pop();
		}
		else if (eleVec[i].type == ET_SEMICOLON)
		{
			TransMsg[i] = "\n";
			TransOrder.emplace_back(i);
			while (!FuncQue.empty())
				FuncQue.pop();
			while (!LParenQue.empty())
				LParenQue.pop();

		}
		else if (eleVec[i].type == ET_COMMA)
		{
			if (nowFunc.second != "")
				FuncParaCount[nowFunc.first]++;
		}
		else if (eleVec[i].type == ET_FUNC)
		{
			bPreEleIsFunc = true;
			if (nowFunc.second != "")
				funcParaMap[nowFunc.first][FuncParaCount[nowFunc.first]].emplace_back(i);
			else
				TransOrder.emplace_back(i);
			FuncMap[i] = eleVec[i].element;
			FuncQue.push(make_pair(i, eleVec[i].element));
			int nParaNum = 0;
			TransMsg[i] = m_FrmlCmpl.TransFuncAndVar(eleVec[i].element, nParaNum);
			funcParaNumMap[i] = nParaNum;
		}
		else
		{
			if (eleVec[i].type == ET_SYSTEMVAR || eleVec[i].type == ET_USERVAR
				|| eleVec[i].type == ET_CONST || eleVec[i].type == ET_OPEATION)
			{
				int nParaNum = 0;
				if (nowFunc.second != "")
					funcParaMap[nowFunc.first][FuncParaCount[nowFunc.first]].emplace_back(i);
				else
					TransOrder.emplace_back(i);
				string strTrans = m_FrmlCmpl.TransFuncAndVar(eleVec[i].element, nParaNum);
				TransMsg[i] = strTrans == "" ? eleVec[i].element : strTrans;
			}
			else if (eleVec[i].type == ET_ASSIGN || eleVec[i].type == ET_OUT)
			{
				int nParaNum;
				string strTrans = m_FrmlCmpl.TransFuncAndVar(eleVec[i].element, nParaNum);
				int nPos = strTrans.find("PARAM#1");
				if (i != 0 && (eleVec[i - 1].type == ET_SYSTEMVAR ||
					eleVec[i - 1].type == ET_CONST ||
					eleVec[i - 1].type == ET_USERVAR))
				{
					strTrans = strTrans.replace(nPos, sizeof("PARAM#1") - 1, TransMsg[i - 1]);
					TransOrder.back() = i;
				}
				else
				{
					strTrans = strTrans.replace(nPos, sizeof("PARAM#1") - 1, "");
					TransOrder.emplace_back(i);
				}
				TransMsg[i] = strTrans;

			}
			else
			{
				if (nowFunc.second != "")
					funcParaMap[nowFunc.first][FuncParaCount[nowFunc.first]].emplace_back(i);
				else
					TransOrder.emplace_back(i);
				TransMsg[i] = eleVec[i].element;
			}

		}
	}
	for (auto &it : TransOrder)
	{
		if (FuncMap.count(it))
			strTrans += GetFrmlTrans(it, TransMsg, funcParaNumMap, funcParaMap);
		else
			strTrans += StrA2StrW(TransMsg[it].c_str());
	}
	return strTrans;
}

BOOL CDlgStkFilterEditor::FrmlTest()
{
	m_pTabCtrl->SetCurSel(1);

	SStringW wstrName = m_pEditName->GetWindowTextW();
	if (wstrName.IsEmpty())
	{
		SMessageBox(NULL, L"公式名称不能为空！", L"错误", MB_OK | MB_ICONERROR);
		m_pEditName->SetFocus();
		return FALSE;
	}
	else if (isdigit(wstrName[0]))
	{
		SMessageBox(NULL, L"公式名称不能以数字开头！", L"错误", MB_OK | MB_ICONERROR);
		m_pEditName->SetFocus();
		return FALSE;
	}
	string strName = StrW2StrA(wstrName);
	if (strName != m_FrmlInfo.name &&!CFrmlManager::CheckNameIsUseful(strName))
	{
		SMessageBox(NULL, L"当前公式名已经存在，请重新命名！", L"错误", MB_OK | MB_ICONERROR);
		m_pEditName->SetFocus();
		return FALSE;
	}
	if (!CheckParam())
		return FALSE;

	string strFrml = StrW2StrA(m_pReFrml->GetWindowTextW().MakeUpper());
	ErrMsg Error = { 0 };
	vector<OutFrml> formula;
	if (m_FrmlCmpl.ExplainString(strFrml, m_ParaSet, Error, formula))
	{
		if (formula.empty())
		{
			m_pEditResult->SetWindowTextW(L"测试未通过!\n错误原因:无编译内容");
			return FALSE;
		}

		int nOutCount = 0;
		for (auto &it : formula)
		{
			if ((it.type & Snt_Out) || (it.type & Snt_LogicOut))
				nOutCount++;
		}
		if (nOutCount < 1)
		{
			m_pEditResult->SetWindowTextW(L"测试未通过!\n错误原因:缺少输出语句");
			return FALSE;
		}
		else if (nOutCount > 1)
		{
			m_pEditResult->
				SetWindowTextW(L"测试未通过!\n错误原因:输出语句太多，条件选股公式只能有一条输出语句");
			return FALSE;
		}
		else
		{
			m_pEditResult->SetWindowTextW(L"测试通过!");
			m_FrmlInfo.formula = formula;
			return TRUE;
		}
	}
	else
	{
		SStringW wstrRes;
		wstrRes.Format(L"测试未通过!\n错误语句:%s\n错误元素:%s\n错误原因:%s",
			StrA2StrW(Error.strSent.c_str()),
			StrA2StrW(Error.strEle.c_str()),
			StrA2StrW(m_FrmlCmpl.TransError((eGraError)Error.Error).c_str()));
		m_pEditResult->SetWindowTextW(wstrRes);
		return FALSE;
	}
	return FALSE;
}

BOOL CDlgStkFilterEditor::CheckParam()
{
	//m_FrmlInfo.para.clear();
	//m_FrmlInfo.paraSetting.clear();
	//m_ParaSet.clear();
	set<string> paraSet;
	vector<string> paraVec;
	map<string, paraRage>paraSetting;
	SStringW strErr;
	for (int i = 0; i < 16; ++i)
	{
		SStringW strName = m_pEditParaName[i]->GetWindowTextW();
		if (strName.IsEmpty())
			break;
		if (!isalpha(strName[0]))
		{
			SMessageBox(NULL,
				strErr.Format(L"参数%d错误!参数只能以字母开头", i + 1),
				L"错误", MB_ICONERROR | MB_OK);
			m_pEditParaName[i]->SetFocus();
			return FALSE;
		}
		for (int j = 1; j < strName.GetLength(); ++j)
		{
			if (!((strName[j] >= '0' && strName[j] <= '9') ||
				strName[j] >= 'A' && strName[j] <= 'Z'))
			{
				SMessageBox(NULL,
					strErr.Format(L"参数%d错误!参数中只能含有字母和数字", i + 1),
					L"错误", MB_ICONERROR | MB_OK);
				m_pEditParaName[i]->SetFocus();
				return FALSE;
			}
		}
		paraRage pr = { 0 };
		SStringW str = m_pEditParaMin[i]->GetWindowTextW();
		if (str.IsEmpty())
		{
			pr.min = 0;
			m_pEditParaMin[i]->SetWindowTextW(L"0");
		}
		else
			pr.min = _wtof(str);
		str = m_pEditParaMax[i]->GetWindowTextW();
		if (str.IsEmpty())
		{
			pr.max = 0;
			m_pEditParaMax[i]->SetWindowTextW(L"0");
		}
		else
			pr.max = _wtof(str);
		str = m_pEditParaDef[i]->GetWindowTextW();
		if (str.IsEmpty())
		{
			pr.def = 0;
			m_pEditParaDef[i]->SetWindowTextW(L"0");
		}
		else
			pr.def = _wtof(str);
		if (pr.max <= pr.min)
		{
			SMessageBox(NULL,
				strErr.Format(L"参数%d错误!\n最大值设定错误,应大于最小值", i + 1),
				L"错误", MB_ICONERROR | MB_OK);
			m_pEditParaMax[i]->SetFocus();
			return FALSE;
		}
		if (pr.def<pr.min || pr.def > pr.max)
		{
			SMessageBox(NULL,
				strErr.Format(L"参数%d错误!\n缺省值是定错误,应在最大值与最小值之间", i + 1),
				L"错误", MB_ICONERROR | MB_OK);
			m_pEditParaDef[i]->SetFocus();
			return FALSE;
		}
		string paraName = StrW2StrA(strName);
		paraVec.emplace_back(paraName);
		paraSetting[paraName] = pr;
		paraSet.insert(paraName);
	}
	bool bParaChanged = false;
	bool bAskChange = false;
	if (m_FrmlInfo.para.size() != paraVec.size())
	{
		bParaChanged = true;
		if (m_FrmlInfo.para.size() != 0)
			bAskChange = true;
	}
	if (!bParaChanged)
	{
		for (int i = 0; i < m_FrmlInfo.para.size(); ++i)
		{
			if (m_FrmlInfo.para[i] != paraVec[i])
			{
				bParaChanged = true;
				bAskChange = true;
				break;
			}
		}
	}
	m_FrmlInfo.para = paraVec;
	m_FrmlInfo.paraSetting = paraSetting;
	m_ParaSet = paraSet;
	if (bParaChanged)
	{
		int nRes = 0;
		if (bAskChange)
			nRes = SMessageBox(NULL, L"参数改变，是否重新生成参数精灵", L"提示", MB_OKCANCEL);
		if (!bAskChange || nRes == IDOK)
			GenerateParaElf();
	}
	return TRUE;
}

BOOL CDlgStkFilterEditor::GenerateParaElf()
{
	string paraElf = "";
	if (m_FrmlInfo.para.empty())
		paraElf = "此公式无参数需要设置";
	else
	{
		paraElf = "请设置参数:\n";
		for (int i = 0; i < m_FrmlInfo.para.size(); ++i)
		{
			char str[1024];
			string paraName = m_FrmlInfo.para[i];
			sprintf_s(str, "%s:PARAM#%d(%.02f--%.02f)\n",
				paraName.c_str(), i + 1,
				m_FrmlInfo.paraSetting[paraName].min,
				m_FrmlInfo.paraSetting[paraName].max);
			paraElf.append(str);
		}
	}
	m_FrmlInfo.paraElf = paraElf;
	m_pEditParam->SetWindowTextW(StrA2StrW(paraElf.c_str()));
	return TRUE;
}

SStringW CDlgStkFilterEditor::GetFrmlTrans(int nFuncNum, map<int, string>& TransMsg,
	map<int, int>& funcParaNumMap, map<int, map<int, vector<int>>>& funcParaMap)
{
	SStringW str = StrA2StrW(TransMsg[nFuncNum].c_str());
	map<int, SStringW> paraTrans;
	auto &paraMap = funcParaMap[nFuncNum];
	for (int i = 0; i < funcParaNumMap[nFuncNum]; ++i)
	{
		if (paraMap.count(i) == 0 || paraMap[i].empty())
			paraTrans[i] = L"?";
		else
		{
			SStringW tmpStr;
			for (int j = 0; j < paraMap[i].size(); ++j)
			{
				if (funcParaNumMap.count(paraMap[i][j]))
				{
					tmpStr += GetFrmlTrans(paraMap[i][j],
						TransMsg, funcParaNumMap, funcParaMap);
				}
				else
					tmpStr += StrA2StrW(TransMsg[paraMap[i][j]].c_str());
			}
			paraTrans[i] = tmpStr;
		}
	}
	SStringW tmp;
	for (auto &it : paraTrans)
		str.Replace(tmp.Format(L"PARAM#%d", it.first + 1), it.second);
	return str;
}

void CDlgStkFilterEditor::OnBtnOK()
{
	if (FrmlTest())
	{
		m_FrmlInfo.name = StrW2StrA(m_pEditName->GetWindowTextW());
		m_FrmlInfo.descption = StrW2StrA(m_pEditDscp->GetWindowTextW());
		m_FrmlInfo.code = StrW2StrA(m_pReFrml->GetWindowTextW());
		m_FrmlInfo.usage = StrW2StrA(m_PEditExplain->GetWindowTextW());
		m_FrmlInfo.useType = eFU_Filter;
		m_FrmlInfo.subdivision = m_pCbxType->GetCurSel();
		if (!m_bFromStkFilter)
		{
			if (m_bNew)
				::SendMessage(m_hParWnd, WM_FORMULA_MSG, (WPARAM)&m_FrmlInfo, FrmlMsg_AddFrml);
			else
				::SendMessage(m_hParWnd, WM_FORMULA_MSG, (WPARAM)&m_FrmlInfo, FrmlMsg_ChangeFrml);
		}
		else
		{
			CFrmlManager::ChangeFormula(m_FrmlInfo.name, m_FrmlInfo);
			::SendMessage(m_hParWnd, WM_FILTER_MSG, (WPARAM)&m_FrmlInfo, FilterMsg_UpdateFrml);
		}

		::EnableWindow(m_hParWnd, TRUE);
		CSimpleWnd::DestroyWindow();
	}
}

void CDlgStkFilterEditor::OnBtnClose()
{
	if (m_bDirty)
	{
		if (SMessageBox(m_hWnd, L"公式内容已经修改，确定要退出么?", L"提示",
			MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		{
			::EnableWindow(m_hParWnd, TRUE);
			CSimpleWnd::DestroyWindow();
		}
	}
	else
	{
		::EnableWindow(m_hParWnd, TRUE);
		CSimpleWnd::DestroyWindow();
	}

}

void CDlgStkFilterEditor::OnBtnInsertFunc()
{
	if (m_pDlgInsert == nullptr)
	{
		m_pDlgInsert = new CDlgFuncInsert(m_hWnd);
		m_pDlgInsert->Create(m_hWnd);
		m_pDlgInsert->CenterWindow(m_hWnd);
		m_pDlgInsert->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		m_pDlgInsert->ShowWindow(SW_SHOWDEFAULT);
		m_pDlgInsert->UpdateList(FT_None);
		::EnableWindow(m_hWnd, FALSE);

	}
	else {
		m_pDlgInsert->ShowWindow(SW_SHOW);
		::EnableWindow(m_hWnd, FALSE);
	}
}

void CDlgStkFilterEditor::OnBtnTest()
{
	FrmlTest();
}
