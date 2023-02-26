#include "stdafx.h"
#include "DlgStockFilter.h"
#include "SColorListCtrlEx.h"
#include "DlgStkFilterEditor.h"
#include "DlgFrmlFind.h"
#include "DlgSaveFrmlList.h"
#include "DlgReadFrmlList.h"
#include <sstream>
#include <io.h>
#include <direct.h>

#define INTERHEIGHT 2
CDlgStockFilter::CDlgStockFilter(UINT uParThreadID, RpsGroup Group)
	:SHostWnd(_T("LAYOUT:dlg_stockFilter"))
{
	m_uParThreadID = uParThreadID;
	m_Group = Group;
	m_bCondsChanged = FALSE;
	m_bUseSF = FALSE;
}


CDlgStockFilter::~CDlgStockFilter()
{
}

BOOL SOUI::CDlgStockFilter::OnInitDialog(EventArgs * e)
{
	m_pCheckUse = FindChildByID2<SCheckBox>(R.id.chk_use);

	m_pList = FindChildByID2<SColorListCtrlEx>(R.id.lc_cond);
	m_pCbxFrml = FindChildByID2<SComboBox>(R.id.cbx_frml);
	m_pCbxPeriod = FindChildByID2<SComboBox>(R.id.cbx_period);
	m_pRdbAnd = FindChildByID2<SRadioBox>(R.id.rdb_and);
	m_pRdbOr = FindChildByID2<SRadioBox>(R.id.rdb_or);
	m_pTextParaTitle = FindChildByID2<SStatic>(R.id.text_paratitle);
	//m_pTextParaTitle->SetAttribute(L"multiLines", L"1");
	SStringW tmpStr;
	for (int i = 0; i < 16; ++i)
		m_pEditPara[i] = FindChildByName2<SEdit>(tmpStr.Format(L"edit_para%d", i + 1));
	InitPeriodMap();
	InitComboBox();
	return TRUE;
}

void SOUI::CDlgStockFilter::OnBtnClose()
{
	if (m_bCondsChanged)
	{
		if (SMessageBox(m_hWnd, L"条件列表发生了改变，确定不保存直接退出吗?",
			L"警告", MB_ICONWARNING | MB_OKCANCEL) == IDOK)
		{
			m_bCondsChanged = FALSE;
			m_pList->DeleteAllItems();
			for (int i = 0; i < m_sfPlan.condVec.size(); ++i)
				ListAddItem(m_sfPlan.condVec[i]);
			if (m_sfPlan.state == CS_And)
				m_pRdbAnd->SetCheck(TRUE);
			else
				m_pRdbOr->SetCheck(TRUE);
			m_pCheckUse->SetCheck(m_bUseSF);
			m_tmpSFPlan = m_sfPlan;
			ShowWindow(SW_HIDE);
		}
	}
	else
	{
		m_pCheckUse->SetCheck(m_bUseSF);
		ShowWindow(SW_HIDE);
	}

}

void SOUI::CDlgStockFilter::OnBtnOK()
{
	BOOL bUsed = m_pCheckUse->IsChecked();
	if (bUsed)
	{
		if (m_pList->GetItemCount() == 0)
		{
			m_pCheckUse->SetCheck(FALSE);
			SMessageBox(m_hWnd, L"条件为空，请添加条件后重试！",
				L"错误", MB_ICONERROR | MB_OK);
			return;
		}
		else
		{
			for (auto &it : m_tmpSFPlan.condVec)
			{
				if (CFrmlManager::CheckNameIsUseful(it.frml))
				{
					SMessageBox(m_hWnd, L"组合条件中有不存在的公式！",
						L"错误", MB_ICONERROR | MB_OK);
					return;
				}

			}
		}
	}
	if (m_bUseSF)
	{
		for (auto &it : m_sfPlan.condVec)
			CFrmlManager::DecreaseFrmlUseCount(it.frml);
	}
	if (m_bCondsChanged)
	{
		m_sfPlan = m_tmpSFPlan;
		m_bCondsChanged = FALSE;
		SendMsg(m_uParThreadID, WW_SaveStockFilter, nullptr, 0);
	}
	if (bUsed)
	{
		for (auto &it : m_sfPlan.condVec)
			CFrmlManager::IncreaseFrmlUseCount(it.frml);
	}

	SendMsg(m_uParThreadID, WW_ChangeStockFilter,
		(char*)&bUsed, sizeof(bUsed));

	m_bUseSF = bUsed;
	ShowWindow(SW_HIDE);

}

void SOUI::CDlgStockFilter::InitList(bool bUse, SFPlan& sfPlan)
{
	m_sfPlan = sfPlan;
	m_tmpSFPlan = sfPlan;
	for (int i = 0; i < sfPlan.condVec.size(); ++i)
	{
		ListAddItem(sfPlan.condVec[i]);
		if (bUse)
			CFrmlManager::IncreaseFrmlUseCount(sfPlan.condVec[i].frml);
	}
	m_pCheckUse->SetCheck(bUse);
	if (sfPlan.state == CS_And)
		m_pRdbAnd->SetCheck(TRUE);
	else
		m_pRdbOr->SetCheck(TRUE);
}

void SOUI::CDlgStockFilter::InitComboBox()
{
	InitFrmlCombox();
	for (int i = SFP_D1; i < SFP_Count; ++i)
		m_pCbxPeriod->InsertItem(i, m_PeriodStrMap[m_PeriodMap[i]], NULL, 0);
	m_pCbxPeriod->SetCurSel(0);

}

void SOUI::CDlgStockFilter::InitFrmlCombox()
{
	string strSel = "";
	if (m_pCbxFrml->GetCount() > 0)
	{
		int nSel = m_pCbxFrml->GetCurSel();
		if (nSel >= 0)
			strSel = m_FrmlVec[nSel].name;
	}
	m_pCbxFrml->ResetContent();
	m_FrmlVec.clear();
	auto frmlMap = CFrmlManager::GetFormulaMap();
	int nSel = 0;
	int nItemCount = 0;
	for (auto &it : frmlMap)
	{
		if (eFU_Filter == it.second.useType)
		{
			SStringW strTmp;
			strTmp.Format(L"%s  -%s", StrA2StrW(it.second.name.c_str()),
				StrA2StrW(it.second.descption.c_str()));
			m_pCbxFrml->InsertItem(m_FrmlVec.size(), strTmp, NULL, 0);
			m_FrmlVec.emplace_back(it.second);
			if (it.second.name == strSel)
				nSel = nItemCount;
			++nItemCount;
		}
	}
	m_pCbxFrml->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgStockFilter::OnCbxFrmlChange, this));
	m_pCbxFrml->SetCurSel(nSel);
	ChangeParaSetting(nSel);
}



void SOUI::CDlgStockFilter::InitPeriodMap()
{


	m_PeriodStrMap[Period_End] = L"-";
	m_PeriodStrMap[Period_1Day] = L"1日";
	m_PeriodStrMap[Period_FenShi] = L"分时";
	m_PeriodStrMap[Period_1Min] = L"1分钟";
	m_PeriodStrMap[Period_5Min] = L"5分钟";
	m_PeriodStrMap[Period_15Min] = L"15分钟";
	m_PeriodStrMap[Period_30Min] = L"30分钟";
	m_PeriodStrMap[Period_60Min] = L"60分钟";

	m_PeriodMap[SFP_Null] = Period_End;
	m_PeriodMap[SFP_D1] = Period_1Day;
	m_PeriodMap[SFP_FS] = Period_FenShi;
	m_PeriodMap[SFP_M1] = Period_1Min;
	m_PeriodMap[SFP_M5] = Period_5Min;
	m_PeriodMap[SFP_M15] = Period_15Min;
	m_PeriodMap[SFP_M30] = Period_30Min;
	m_PeriodMap[SFP_M60] = Period_60Min;

	//m_ReversePeriodMap[L"-"] = SFP_Null;
	//m_ReversePeriodMap[L"1日"] = SFP_D1;
	//m_ReversePeriodMap[L"分时"] = SFP_FS;
	//m_ReversePeriodMap[L"1分钟"] = SFP_M1;
	//m_ReversePeriodMap[L"5分钟"] = SFP_M5;
	//m_ReversePeriodMap[L"15分钟"] = SFP_M15;
	//m_ReversePeriodMap[L"30分钟"] = SFP_M30;
	//m_ReversePeriodMap[L"60分钟"] = SFP_M60;

}

void SOUI::CDlgStockFilter::OutPutCondition(SFPlan& sfPlan)
{
	sfPlan = m_sfPlan;
}


LRESULT SOUI::CDlgStockFilter::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	FILTERMSG msg = (FILTERMSG)lp;
	switch (msg)
	{
	case FilterMsg_ReinitFrml:
		InitFrmlCombox();
		break;
	case FilterMsg_Search:
	{
		pair<int, SStringW>* pSearchMsg = (pair<int, SStringW>*)wp;
		SearchFrml(pSearchMsg->first, pSearchMsg->second);
	}
	break;
	case FilterMsg_UpdateFrml:
	{
		FrmlFullInfo * pFrml = (FrmlFullInfo *)wp;
		for (auto &it : m_FrmlVec)
		{
			if (it.name == pFrml->name)
			{
				it = *pFrml;
				break;
			}
		}
	}
	break;
	case FilterMsg_SaveFrmlList:
	{
		if (_access(".\\filter", 0) != 0)
			_mkdir(".\\filter");
		string  strPlanName = *(string *)wp;
		strPlanName = ".\\filter\\" + strPlanName + ".sfl";
		std::ofstream ofile(strPlanName);
		if (ofile.is_open())
			SaveConditonsList(ofile, m_tmpSFPlan);
	}
	break;
	case FilterMsg_ReadFrmlList:
	{
		if (_access(".\\filter", 0) != 0)
			_mkdir(".\\filter");
		string  strPlanName = *(string *)wp;
		strPlanName = ".\\filter\\" + strPlanName + ".sfl";
		std::ifstream ifile(strPlanName);
		if (ifile.is_open())
		{
			ReadConditonsList(ifile, m_tmpSFPlan);
			m_pList->DeleteAllItems();
			for (int i = 0; i < m_tmpSFPlan.condVec.size(); ++i)
				ListAddItem(m_tmpSFPlan.condVec[i]);
			m_bCondsChanged = TRUE;
		}
	}
	break;

	default:
		break;
	}
	return 0;
}

CRect SOUI::CDlgStockFilter::GetLastCharRect(IRenderTarget *pRT, SStringW str,
	CRect clinetRC, int nLineHei)
{
	CRect rc = clinetRC;
	int nLine = 0;
	SStringW tmpStr = L"";
	CSize szChar;
	for (int i = 0; i < str.GetLength(); ++i)
	{
		if (str[i] != '\n')
			tmpStr += str[i];
		else
		{
			tmpStr = L"";
			++nLine;
		}
	}
	if (tmpStr.IsEmpty())
	{
		rc.top += nLine *(nLineHei + INTERHEIGHT);
		rc.right = rc.left;
		rc.bottom = rc.top + nLineHei;
	}
	else
	{
		pRT->MeasureText(tmpStr, tmpStr.GetLength(), &szChar);
		int nWidth = szChar.cx;
		if (nWidth < clinetRC.Width())
		{
			rc.top += nLine *(nLineHei + INTERHEIGHT);
			rc.right = rc.left + nWidth;
			rc.bottom = rc.top + nLineHei;
		}
		else
		{
			int nNeedLine = nWidth / clinetRC.Width();
			nLine += nNeedLine;
			int nPos = tmpStr.GetLength() - 1;
			WCHAR* buffer = tmpStr.GetBuffer(tmpStr.GetLength() + 1);
			pRT->MeasureText(buffer + nPos,
				tmpStr.GetLength() - nPos, &szChar);
			int nNewWidth = szChar.cx;
			while ((nWidth - nNewWidth) / clinetRC.Width() == nNeedLine)
			{
				--nPos;
				pRT->MeasureText(buffer + nPos,
					tmpStr.GetLength() - nPos, &szChar);
				nNewWidth = szChar.cx;
			}
			rc.top += nLine *(nLineHei + INTERHEIGHT);
			rc.right = rc.left + nNewWidth;
			rc.bottom = rc.top + nLineHei;
		}
	}
	return rc;
}

void SOUI::CDlgStockFilter::ListAddItem(SFCondition & sf)
{
	int nID = m_pList->GetItemCount();
	SStringW strItem;
	strItem.Format(L"%s(", StrA2StrW(sf.frml.c_str()));
	for (int i = 0; i < sf.paraVec.size(); ++i)
	{
		if (i == 0)
			strItem.Format(L"%s%g", strItem, sf.paraVec[i]);
		else
			strItem.Format(L"%s,%g", strItem, sf.paraVec[i]);
	}
	strItem.Format(L"%s) %s", strItem, m_PeriodStrMap[sf.nPeriod]);
	m_pList->InsertItem(nID, strItem);
	m_pList->RequestRelayout();
	//m_sfVec.emplace_back(sf);

}


bool SOUI::CDlgStockFilter::GetListItem(int nRow, SFCondition& sf)
{
	if (nRow >= m_pList->GetItemCount())
		return false;
	sf = m_tmpSFPlan.condVec[nRow];
	return true;
}

void SOUI::CDlgStockFilter::SearchFrml(int nDirect, SStringW strKey)
{
	int nSel = m_pCbxFrml->GetCurSel();
	string strSearch = StrW2StrA(strKey);
	if (SD_Up == nDirect)
	{
		for (int i = nSel - 1; i >= 0; --i)
		{
			if (m_FrmlVec[i].name.find(strSearch) != string::npos
				|| m_FrmlVec[i].descption.find(strSearch) != string::npos)
			{
				m_pCbxFrml->SetCurSel(i);
				ChangeParaSetting(i);
				return;
			}
		}
		for (int i = m_pCbxFrml->GetCount() - 1; i > nSel; --i)
		{
			if (m_FrmlVec[i].name.find(strSearch) != string::npos
				|| m_FrmlVec[i].descption.find(strSearch) != string::npos)
			{
				m_pCbxFrml->SetCurSel(i);
				ChangeParaSetting(i);
				return;
			}
		}

	}
	else
	{
		for (int i = nSel + 1; i < m_pCbxFrml->GetCount(); ++i)
		{
			if (m_FrmlVec[i].name.find(strSearch) != string::npos
				|| m_FrmlVec[i].descption.find(strSearch) != string::npos)
			{
				m_pCbxFrml->SetCurSel(i);
				ChangeParaSetting(i);
				return;
			}
		}
		for (int i = 0; i < nSel; ++i)
		{
			if (m_FrmlVec[i].name.find(strSearch) != string::npos
				|| m_FrmlVec[i].descption.find(strSearch) != string::npos)
			{
				m_pCbxFrml->SetCurSel(i);
				ChangeParaSetting(i);
				return;
			}
		}

	}
}

void SOUI::CDlgStockFilter::ChangeParaSetting(int nSel)
{
	if (nSel >= m_FrmlVec.size())
		return;
	auto &frml = m_FrmlVec[nSel];

	string str = frml.paraElf;

	size_t pos = -1;
	map<size_t, int>PosParaMap;
	map<size_t, int>ParaLengthMap;

	while ((pos = str.find("PARAM#", pos + 1)) != string::npos)
	{
		string strNum = "";
		for (int i = pos + 6; i < pos + 8 && i < str.size(); ++i)
		{
			if (str[i] >= '0' && str[i] <= '9')
				strNum += str[i];
		}
		if (!strNum.empty())
		{
			int nWidth = strNum.size();
			int nNum = atoi(strNum.c_str());
			if (nNum > 16)
			{
				nNum = strNum[0] - '0';
				nWidth = 1;
			}
			nNum -= 1;
			if (nNum < 0)
				continue;

			PosParaMap[pos] = nNum;
			ParaLengthMap[pos] = 6 + nWidth;
		}
	}
	int nOffset = 0;
	map<int, CRect> EditRcMap;
	IRenderTarget* pRT = m_pTextParaTitle->GetRenderTarget();
	SIZE szChar;
	pRT->MeasureText(_T("A"), 1, &szChar);
	int nLineHei = szChar.cy;
	CRect clientRc = m_pTextParaTitle->GetClientRect();
	int nLine = 1;
	for (auto &it : PosParaMap)
	{
		string tmpStr = str.substr(it.first + nOffset, ParaLengthMap[it.first]);
		SStringW tmpWstr = StrA2StrW(tmpStr.c_str());
		pRT->MeasureText(tmpWstr, tmpWstr.GetLength(), &szChar);
		int nWidth = szChar.cx;
		tmpStr = str.substr(0, it.first + nOffset);
		tmpWstr = StrA2StrW(tmpStr.c_str());
		CRect rc = GetLastCharRect(pRT, tmpWstr, clientRc, nLineHei);
		if (clientRc.right - rc.right < nWidth)
		{
			tmpStr += '\n';
			tmpStr += str.substr(it.first + nOffset);
			str = tmpStr;
			++nOffset;
			rc.top = rc.bottom;
			rc.bottom += nLineHei;
			rc.left = clientRc.left + 1;
			rc.right = rc.left + nWidth;
		}
		else
		{
			rc.left = rc.right + 1;
			rc.right += nWidth;
		}
		EditRcMap[it.second] = rc;
	}
	m_pTextParaTitle->ReleaseRenderTarget(pRT);
	m_pTextParaTitle->SetWindowTextW(StrA2StrW(str.c_str()));
	for (int i = 0; i < 16; ++i)
	{
		if (EditRcMap.count(i) == 0)
			m_pEditPara[i]->SetVisible(FALSE, TRUE);
		else
		{
			m_pEditPara[i]->SetVisible(TRUE, TRUE);
			m_pEditPara[i]->Move(EditRcMap[i]);
			SStringW tmp;
			m_pEditPara[i]->SetWindowTextW(tmp.Format(L"%g",
				frml.paraSetting[frml.para[i]].def));
		}
	}
	//bool bFirst = true;
	//int nPara = 0;
	//while (getline(ss, buffer))
	//{
	//	if (bFirst)
	//	{
	//		if (buffer.find("PARAM#") == string::npos)
	//		{
	//			m_pTextParaTitle->SetAttribute(L"pos", L"5,20");
	//			m_pTextParaTitle->SetWindowTextW(StrA2StrW(buffer.c_str()));
	//		}
	//		else
	//		{
	//			m_pTextParaTitle->SetAttribute(L"pos", L"5,16,5,16");
	//			auto pos = buffer.find("#PARAM", 0);
	//			int nPara = buffer[pos + 6] - '0';
	//			string strStart = buffer.substr(0, pos);
	//			m_pTextParaStart[nPara]->SetWindowTextW(StrA2StrW(buffer.c_str()));
	//		}
	//		bFirst = false;
	//	}
	//}

}

void SOUI::CDlgStockFilter::SaveConditonsList(ofstream &fs, SFPlan &sfPlan)
{
	char cEnd = 0;
	for (auto &it : sfPlan.condVec)
	{
		fs.write("<condition>", strlen("<condition>"));
		fs.write(it.frml.c_str(), it.frml.size());
		fs.write(&cEnd, 1);
		int nParaSize = it.paraVec.size();
		fs.write((char*)&nParaSize, sizeof(nParaSize));
		for (auto &para : it.paraVec)
			fs.write((char*)&para, sizeof(para));
		fs.write((char*)&it.nPeriod, sizeof(it.nPeriod));
		fs.write("</condition>", strlen("</condition>"));
	}
	fs.write("<state>", strlen("<state>"));
	fs.write((char*)&sfPlan.state, sizeof(sfPlan.state));
	fs.write("</state>", strlen("</state>"));

}

void SOUI::CDlgStockFilter::ReadConditonsList(ifstream &fs, SFPlan &sfPlan)
{
	char ch = 0;
	string msg = "";
	while (fs.read(&ch, 1))
		msg += ch;
	size_t nStartPos = 0;
	size_t nEndPos = 0;
	sfPlan.condVec.clear();
	while (nStartPos != string::npos)
	{
		nStartPos = msg.find("<condition>", nEndPos);
		if (nStartPos != string::npos)
		{
			nEndPos = msg.find("</condition>", nStartPos);
			SFCondition sf;
			string condMsg = msg.substr(nStartPos + 11, nEndPos - nStartPos - 11);
			size_t pos = 0;
			for (pos; pos < condMsg.size(); ++pos)
			{
				if (condMsg[pos] == '\0')
				{
					sf.frml = condMsg.substr(0, pos);
					break;
				}
			}
			++pos;
			int nParaSize = *(int*)(condMsg.c_str() + pos);
			pos += sizeof(nParaSize);
			sf.paraVec.reserve(nParaSize);
			for (int i = 0; i < nParaSize; ++i)
			{
				sf.paraVec.emplace_back(*(double*)(condMsg.c_str() + pos));
				pos += sizeof(double);
			}
			sf.nPeriod = *(int*)(condMsg.c_str() + pos);
			sfPlan.condVec.emplace_back(sf);
		}
	}
	nStartPos = msg.find("<state>", nEndPos);
	nEndPos = msg.find("</state>", nStartPos);
	sfPlan.state = *(ConditionsState*)(msg.c_str() + nStartPos + 7);


}


bool SOUI::CDlgStockFilter::OnCbxFrmlChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	ChangeParaSetting(nSel);
	return true;
}


void SOUI::CDlgStockFilter::OnBtnAddConditon()
{
	int nSel = m_pCbxFrml->GetCurSel();
	auto &frml = m_FrmlVec[nSel];
	SFCondition sf;
	sf.frml = frml.name;
	for (int i = 0; i < frml.para.size(); ++i)
	{
		if (m_pEditPara[i]->IsVisible())
			sf.paraVec.emplace_back(_wtof(m_pEditPara[i]->GetWindowTextW()));
		else
			sf.paraVec.emplace_back(frml.paraSetting[frml.para[i]].def);
	}
	sf.nPeriod = m_PeriodMap[m_pCbxPeriod->GetCurSel()];
	ListAddItem(sf);
	m_tmpSFPlan.condVec.emplace_back(sf);
	m_pList->SetSelectedItem(m_pList->GetItemCount() - 1);
	m_bCondsChanged = TRUE;
}

void SOUI::CDlgStockFilter::OnBtnDeleteCondition()
{
	int nSel = m_pList->GetSelectedItem();
	if (nSel < 0)
		return;
	m_pList->DeleteItem(nSel);
	m_pList->RequestRelayout();
	if (nSel > 0)
		m_pList->SetSelectedItem(nSel - 1);
	else
		m_pList->SetSelectedItem(0);
	for (int i = nSel; i < m_tmpSFPlan.condVec.size() - 1; ++i)
		m_tmpSFPlan.condVec[i] = m_tmpSFPlan.condVec[i + 1];
	m_tmpSFPlan.condVec.pop_back();
	m_bCondsChanged = TRUE;

}

void SOUI::CDlgStockFilter::OnBtnFind()
{
	int nSel = m_pCbxFrml->GetCurSel();
	CDlgFrmlFind *pDlg = new CDlgFrmlFind(m_hWnd);
	pDlg->Create(m_hWnd);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
	::EnableWindow(m_hWnd, FALSE);
}

void SOUI::CDlgStockFilter::OnBtnUsage()
{
	int nSel = m_pCbxFrml->GetCurSel();
	SStringW strUsage = StrA2StrW(m_FrmlVec[nSel].usage.c_str());
	SStringW title;
	title.Format(L"[%s]指标用法", StrA2StrW(m_FrmlVec[nSel].name.c_str()));
	SMessageBox(m_hWnd, strUsage, title, MB_OK);
}

void SOUI::CDlgStockFilter::OnBtnEdit()
{
	int nSel = m_pCbxFrml->GetCurSel();
	CDlgStkFilterEditor *pDlg = new CDlgStkFilterEditor(m_hWnd, m_FrmlVec[nSel], TRUE);
	pDlg->Create(m_hWnd);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
	::EnableWindow(m_hWnd, FALSE);

}

void SOUI::CDlgStockFilter::OnBtnRead()
{
	CDlgReadFrmlList *pDlg = new CDlgReadFrmlList(m_hWnd);
	pDlg->Create(m_hWnd);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
	::EnableWindow(m_hWnd, FALSE);

}

void SOUI::CDlgStockFilter::OnBtnSave()
{
	if (0 == m_pList->GetItemCount())
	{
		SMessageBox(m_hWnd, L"当前条件为空，请添加条件后再试!", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}
	CDlgSaveFrmlList *pDlg = new CDlgSaveFrmlList(m_hWnd);
	pDlg->Create(m_hWnd);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
	::EnableWindow(m_hWnd, FALSE);
}

void SOUI::CDlgStockFilter::OnRadioAndCliecked()
{
	if (m_pRdbAnd->IsChecked())
		return;
	m_tmpSFPlan.state = CS_And;
	m_bCondsChanged = TRUE;

}

void SOUI::CDlgStockFilter::OnRadioOrCliecked()
{
	if (m_pRdbOr->IsChecked())
		return;
	m_tmpSFPlan.state = CS_Or;
	m_bCondsChanged = TRUE;
}

void SOUI::CDlgStockFilter::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

