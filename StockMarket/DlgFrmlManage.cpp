#include "stdafx.h"
#include "DlgFrmlManage.h"
#include "DlgStkFilterEditor.h"


CDlgFrmlManage::CDlgFrmlManage(HWND hParWnd) :SHostWnd(_T("LAYOUT:dlg_FrmlManage"))
{
	m_hParWnd = hParWnd;
}


CDlgFrmlManage::~CDlgFrmlManage()
{
}

BOOL SOUI::CDlgFrmlManage::OnInitDialog(EventArgs * e)
{
	m_pBtnNew = FindChildByName2<SButton>(L"btn_New");
	m_pBtnChange = FindChildByName2<SButton>(L"btn_Change");
	m_pBtnDelete = FindChildByName2<SButton>(L"btn_Delete");
	m_pTreeFrml = FindChildByName2<STreeCtrl>(L"tc_frml");
	m_pTreeFrml->GetEventSet()->subscribeEvent(EventTCSelChanged::EventID,
		Subscriber(&CDlgFrmlManage::OnTreeFrmlSelChanged, this));

	InitShowText();
	UpdateFrmlTree();
	return 0;
}

BOOL SOUI::CDlgFrmlManage::InitShowText()
{
	m_FrmlUseShowText[eFU_Target] = L"技术指标公式";
	m_FrmlUseShowText[eFU_Filter] = L"条件选股公式";
	m_FilterFrmlShowText[SFFT_IndexCondition] = L"指标条件";
	m_FilterFrmlShowText[SFFT_Fundamental] = L"基本面";
	m_FilterFrmlShowText[SFFT_RealTime] = L"即时盘中";
	m_FilterFrmlShowText[SFFT_Tendency] = L"走势特征";
	m_FilterFrmlShowText[SFFT_Form] = L"形态特征";
	m_FilterFrmlShowText[SFFT_Other] = L"其他类型";
	return TRUE;
}

BOOL SOUI::CDlgFrmlManage::UpdateFrmlTree()
{
	//插入类别结点
	m_pTreeFrml->RemoveAllItems();
	m_FrmlUseNodeMap.clear();
	m_SubdivNodeMap.clear();
	m_FrmlPosMap.clear();

	auto RootItem = m_pTreeFrml->GetRootItem();
	m_FrmlUseNodeMap[eFU_Target] =
		m_pTreeFrml->InsertItem(m_FrmlUseShowText[eFU_Target], 0, 0, STVI_ROOT, STVI_LAST);
	m_pTreeFrml->SetItemData(m_FrmlUseNodeMap[eFU_Target], 0);
	m_FrmlUseNodeMap[eFU_Filter] =
		m_pTreeFrml->InsertItem(m_FrmlUseShowText[eFU_Filter], 0, 0,
			STVI_ROOT, m_FrmlUseNodeMap[eFU_Target]);
	m_pTreeFrml->SetItemData(m_FrmlUseNodeMap[eFU_Filter], 0);

	HSTREEITEM preItem = STVI_LAST;
	for (int i = SFFT_IndexCondition; i < SFFT_Count; ++i)
	{
		m_SubdivNodeMap[i] =
			m_pTreeFrml->InsertItem(m_FilterFrmlShowText[(eStkFilterFrmlType)i], 0, 0,
				m_FrmlUseNodeMap[eFU_Filter], preItem);
		m_pTreeFrml->SetItemData(m_SubdivNodeMap[i], 1);
		preItem = m_SubdivNodeMap[i];

	}

	m_FrmlMap = CFrmlManager::GetFormulaMap();
	map<eFrmlUse, map<int, HSTREEITEM>> preItemMap;
	for (auto &it : m_FrmlMap)
	{
		HSTREEITEM preItem = preItemMap[it.second.useType].count(it.second.subdivision) == 0 ?
			STVI_LAST : preItemMap[it.second.useType][it.second.subdivision];

		SStringW strDscp;
		auto &parNode = m_SubdivNodeMap[it.second.subdivision];
		strDscp.Format(L"%s\t -%s", StrA2StrW(it.first.c_str()),
			StrA2StrW(it.second.descption.c_str()));
		preItem = m_pTreeFrml->InsertItem(strDscp, 2, 2, parNode, preItem);
		m_pTreeFrml->SetItemData(preItem, 2);
		m_FrmlPosMap[preItem] = it.first;
		preItemMap[it.second.useType][it.second.subdivision] = preItem;
	}

	return TRUE;
}

bool SOUI::CDlgFrmlManage::OnTreeFrmlSelChanged(EventArgs * e)
{
	EventTCSelChanged* pEvt = (EventTCSelChanged*)e;
	STreeCtrl * pTree = (STreeCtrl *)pEvt->sender;
	HSTREEITEM selItem = pTree->GetSelectedItem();

	int nLevel = pTree->GetItemData(selItem);
	if (nLevel == 0)
	{
		m_pBtnNew->EnableWindow(FALSE, TRUE);
		m_pBtnChange->EnableWindow(FALSE, TRUE);
		m_pBtnDelete->EnableWindow(FALSE, TRUE);
	}
	else if (nLevel == 1)
	{
		m_pBtnNew->EnableWindow(TRUE, TRUE);
		m_pBtnChange->EnableWindow(FALSE, TRUE);
		m_pBtnDelete->EnableWindow(FALSE, TRUE);
	}
	else if (nLevel == 2)
	{
		m_pBtnNew->EnableWindow(TRUE, TRUE);
		m_pBtnChange->EnableWindow(TRUE, TRUE);
		m_pBtnDelete->EnableWindow(TRUE, TRUE);

	}
	return TRUE;
}

void SOUI::CDlgFrmlManage::OnBtnNew()
{
	HSTREEITEM selItem = m_pTreeFrml->GetSelectedItem();
	int nLevel = m_pTreeFrml->GetItemData(selItem);
	if (1 == nLevel)
	{
		for (auto &it : m_SubdivNodeMap)
		{
			if (it.second == selItem)
			{
				CDlgStkFilterEditor *pDlg = new CDlgStkFilterEditor(m_hWnd, it.first);
				pDlg->Create(m_hWnd);
				pDlg->CenterWindow(m_hWnd);
				pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				pDlg->ShowWindow(SW_SHOWDEFAULT);
				::EnableWindow(m_hWnd, FALSE);
				break;
			}
		}
	}
	else if (2 == nLevel)
	{
		string frmlName = m_FrmlPosMap[selItem];
		if (m_FrmlMap[frmlName].useType == eFU_Filter)
		{
			CDlgStkFilterEditor *pDlg = 
				new CDlgStkFilterEditor(m_hWnd, m_FrmlMap[frmlName].subdivision);
			pDlg->Create(m_hWnd);
			pDlg->CenterWindow(m_hWnd);
			pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			pDlg->ShowWindow(SW_SHOWDEFAULT);
			::EnableWindow(m_hWnd, FALSE);
		}
	}
}

void SOUI::CDlgFrmlManage::OnBtnChange()
{
	HSTREEITEM selItem = m_pTreeFrml->GetSelectedItem();
	string frmlName = m_FrmlPosMap[selItem];
	m_ChangeFrmlName = frmlName;
	if (m_FrmlMap[frmlName].useType == eFU_Filter)
	{
		CDlgStkFilterEditor *pDlg =
			new CDlgStkFilterEditor(m_hWnd, m_FrmlMap[frmlName],FALSE);
		pDlg->Create(m_hWnd);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
}

void SOUI::CDlgFrmlManage::OnBtnDelete()
{
	HSTREEITEM selItem = m_pTreeFrml->GetSelectedItem();
	string frmlName = m_FrmlPosMap[selItem];
	if (CFrmlManager::GetFrmlUseCount(frmlName) <= 0)
	{
		if (SMessageBox(m_hWnd, L"确定要删除当前公吗?",
			L"提示", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
			CFrmlManager::DeleteFormula(frmlName);
		else
			return;
	}
	else
	{
		SMessageBox(m_hWnd, L"当前公式正在使用，无法删除！",
			L"错误", MB_ICONERROR | MB_OK);
		return;
	}
	UpdateFrmlTree();
}

void SOUI::CDlgFrmlManage::OnBtnClose()
{
	CFrmlManager::SaveFormulas();
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();

}

LRESULT SOUI::CDlgFrmlManage::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	switch (lp)
	{
	case FrmlMsg_AddFrml:
	{
		FrmlFullInfo FrmlInfo = *(FrmlFullInfo*)wp;
		CFrmlManager::AddNewFormula(FrmlInfo);
		UpdateFrmlTree();
	}
	break;
	case FrmlMsg_ChangeFrml:
	{
		FrmlFullInfo FrmlInfo = *(FrmlFullInfo*)wp;
		CFrmlManager::ChangeFormula(m_ChangeFrmlName,FrmlInfo);
		m_ChangeFrmlName = "";
		UpdateFrmlTree();
	}
	break;

	default:
		break;
	}
	return 0;
}
