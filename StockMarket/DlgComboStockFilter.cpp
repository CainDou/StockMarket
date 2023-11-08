#include "stdafx.h"
#include "DlgComboStockFilter.h"
#include "SColorListCtrlEx.h"
#include "DlgBackTesting.h"
#include "DlgAddNew.h"

CDlgComboStockFilter::CDlgComboStockFilter(HWND hParWnd, UINT uParThreadID, RpsGroup Group)
	:SHostWnd(_T("LAYOUT:dlg_comboStockFilter"))
{
	m_hParWnd = hParWnd;
	m_uParThreadID = uParThreadID;
	m_Group = Group;
	m_pDlgBackTesting = nullptr;
	m_bHisChange = FALSE;
	m_bHisCalced = FALSE;
}


CDlgComboStockFilter::~CDlgComboStockFilter()
{
}

BOOL CDlgComboStockFilter::OnInitDialog(EventArgs * e)
{

	m_pCheckUse = FindChildByID2<SCheckBox>(R.id.chk_use);

	m_pList = FindChildByID2<SColorListCtrlEx>(R.id.ls_filter);
	//SHeaderCtrlEx * pHead = (SHeaderCtrlEx *)m_pList->GetChild(GSW_FIRSTCHILD);
	//pHead->SetNoMoveCol(1);
	m_pCbxFunc = FindChildByID2<SComboBox>(R.id.cbx_func);
	m_pTextID = FindChildByID2<SStatic>(R.id.text_ID);
	m_pCbxID = FindChildByID2<SComboBox>(R.id.cbx_ID);
	m_pTextPeriod1 = FindChildByID2<SStatic>(R.id.text_period1);
	m_pCbxPeriod1 = FindChildByID2<SComboBox>(R.id.cbx_period1);
	m_pTextIndex1 = FindChildByID2<SStatic>(R.id.text_index1);
	m_pCbxIndex1 = FindChildByID2<SComboBox>(R.id.cbx_index1);
	m_pTextCondition = FindChildByID2<SStatic>(R.id.text_condition);
	m_pCbxCondition = FindChildByID2<SComboBox>(R.id.cbx_condition);
	m_pTextIndex2 = FindChildByID2<SStatic>(R.id.text_index2);
	m_pCbxIndex2 = FindChildByID2<SComboBox>(R.id.cbx_index2);
	m_pTextPeriod2 = FindChildByID2<SStatic>(R.id.text_period2);
	m_pCbxPeriod2 = FindChildByID2<SComboBox>(R.id.cbx_period2);
	m_pTextNum = FindChildByID2<SStatic>(R.id.text_num);
	m_pEditNum = FindChildByID2<SEdit>(R.id.edit_num);
	m_pEditNum->SetAttribute(L"colorText", L"#000000FF");

	m_pListHis = FindChildByID2<SColorListCtrlEx>(R.id.ls_filterHis);
	m_pCbxHisFunc = FindChildByID2<SComboBox>(R.id.cbx_funcHis);
	m_pTextHisID = FindChildByID2<SStatic>(R.id.text_IDHis);
	m_pCbxHisID = FindChildByID2<SComboBox>(R.id.cbx_IDHis);
	m_pTextHisPeriod1 = FindChildByID2<SStatic>(R.id.text_period1His);
	m_pCbxHisPeriod1 = FindChildByID2<SComboBox>(R.id.cbx_period1His);
	m_pTextHisIndex1 = FindChildByID2<SStatic>(R.id.text_index1His);
	m_pCbxHisIndex1 = FindChildByID2<SComboBox>(R.id.cbx_index1His);
	m_pTextHisCondition = FindChildByID2<SStatic>(R.id.text_conditionHis);
	m_pCbxHisCondition = FindChildByID2<SComboBox>(R.id.cbx_conditionHis);
	m_pTextHisIndex2 = FindChildByID2<SStatic>(R.id.text_index2His);
	m_pCbxHisIndex2 = FindChildByID2<SComboBox>(R.id.cbx_index2His);
	m_pTextHisPeriod2 = FindChildByID2<SStatic>(R.id.text_period2His);
	m_pCbxHisPeriod2 = FindChildByID2<SComboBox>(R.id.cbx_period2His);
	m_pTextHisNum = FindChildByID2<SStatic>(R.id.text_numHis);
	m_pEditHisNum = FindChildByID2<SEdit>(R.id.edit_numHis);
	m_pEditHisNum->SetAttribute(L"colorText", L"#000000FF");

	m_pTextHisCntDayFront = FindChildByID2<SStatic>(R.id.text_countDayFront);
	m_pTextHisCntDayBack = FindChildByID2<SStatic>(R.id.text_countDayBack);
	m_pEditHisCntDay = FindChildByID2<SEdit>(R.id.edit_countDay);
	m_pEditHisCntDay->SetAttribute(L"colorText", L"#000000FF");

	m_pRadioExist = FindChildByID2<SRadioBox>(R.id.rdb_exist);
	m_pRadioForall = FindChildByID2<SRadioBox>(R.id.rdb_forall);

	m_pCheckHisUse = FindChildByID2<SCheckBox>(R.id.chk_useHis);
	m_pBtnSaveHis = FindChildByID2<SImageButton>(R.id.btn_saveHis);
	m_pTab = FindChildByID2<STabCtrl>(R.id.tab_stockFilter);
	m_pBtnRtFilter = FindChildByID2<SImageButton>(R.id.btn_rtFilter);
	m_pBtnHisFilter = FindChildByID2<SImageButton>(R.id.btn_hisFilter);
	m_pBtnRtFilter->SetAttribute(L"colorText", L"#FF0000FF");
	if (m_Group != Group_Stock)
	{
		SImageButton *btn = FindChildByName2<SImageButton>(L"btn_backTesting");
		if (btn)
			btn->SetVisible(FALSE, TRUE);
	}
	InitStringMap();
	InitHisStringMap();
	InitComboBox();
	InitHisComboBox();
	return TRUE;
}

void CDlgComboStockFilter::OnBtnClose()
{
	ShowWindow(SW_HIDE);
}

void CDlgComboStockFilter::OnBtnOK()
{
	int nSel = m_pCbxFunc->GetCurSel();
	if (SFF_Add == nSel)
		AddConditon();
	else if (SFF_Change == nSel)
		ChangeCondition();
	else if (SFF_Delete == nSel)
		DeleteCondition();
}

void CDlgComboStockFilter::OnBtnOKHis()
{
	int nSel = m_pCbxHisFunc->GetCurSel();
	if (SFF_Add == nSel)
		AddHisConditon();
	else if (SFF_Change == nSel)
		ChangeHisCondition();
	else if (SFF_Delete == nSel)
		DeleteHisCondition();

}

void CDlgComboStockFilter::OnBtnBackTesting()
{
	if (m_pDlgBackTesting == nullptr)
	{
		m_pDlgBackTesting = new CDlgBackTesting(m_hWnd, m_sfSet, m_stockInfo);
		m_pDlgBackTesting->Create(NULL);
		m_pDlgBackTesting->CenterWindow(m_hWnd);
		m_pDlgBackTesting->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
		m_pDlgBackTesting->ShowWindow(SW_SHOWDEFAULT);
	}
	else
	{
		if (m_pDlgBackTesting->IsWindowVisible())
			m_pDlgBackTesting->ShowWindow(SW_HIDE);
		else
		{
			m_pDlgBackTesting->ReSetCondition(m_sfSet);
			m_pDlgBackTesting->ShowWindow(SW_SHOW);
		}

	}

}

void CDlgComboStockFilter::OnBtnSetFliterName()
{
	CDlgAddNew * pDlg = new CDlgAddNew(m_hParWnd, WT_SetFliterName, m_strFilterName);
	pDlg->Create(NULL);
	pDlg->CenterWindow(m_hWnd);
	pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	pDlg->ShowWindow(SW_SHOWDEFAULT);
}

void CDlgComboStockFilter::OnBtnSaveHisFitler()
{
	BOOL bUsed = m_pCheckHisUse->IsChecked();
	m_pBtnSaveHis->EnableWindow(FALSE, TRUE);
	m_pCheckHisUse->EnableWindow(FALSE, TRUE);
	if (m_bHisChange)
	{
		m_hisSfSet = m_hisTmpSfSet;
		SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);

	}

	if (bUsed)
	{
		if (m_bHisChange || !m_bHisCalced)
		{
			m_bHisChange = FALSE;
			if (m_pListHis->GetItemCount() <= 0)
			{
				m_pCheckHisUse->SetCheck(FALSE);
				SMessageBox(m_hWnd, L"��ǰ��ʷѡ��������Ϊ��,�Զ��ر�!", L"��ʾ", MB_OK);
				bUsed = FALSE;
				SendMsg(m_uParThreadID, WW_HisFilterStartCalc,
					(char*)&bUsed, sizeof(bUsed));
				m_pCheckHisUse->EnableWindow(TRUE, TRUE);
				m_pBtnSaveHis->EnableWindow(TRUE, TRUE);

			}

			else if (SMessageBox(m_hWnd, L"�������ģ��Ƿ����¼�����������Ĺ�Ʊ!", L"��ʾ", MB_OKCANCEL) == IDOK)
			{
				SendMsg(m_uParThreadID, WW_HisFilterStartCalc,
					(char*)&bUsed, sizeof(bUsed));
				CalcHisStockFilter();
				return;
			}
		}
		else
		{
			m_pCheckHisUse->EnableWindow(TRUE, TRUE);
			m_pBtnSaveHis->EnableWindow(TRUE, TRUE);
		}
	}
	else
	{
		SendMsg(m_uParThreadID, WW_ChangeHisFiterState,
			(char*)&bUsed, sizeof(bUsed));
		m_pCheckHisUse->EnableWindow(TRUE, TRUE);
		m_pBtnSaveHis->EnableWindow(TRUE, TRUE);
	}

	m_bHisChange = FALSE;

}

void CDlgComboStockFilter::OnBtnRtFitler()
{
	m_pTab->SetCurSel(0);
	m_pBtnRtFilter->SetAttribute(L"colorText", L"#FF0000FF");
	m_pBtnHisFilter->SetAttribute(L"colorText", L"#00FFFFFF");

}

void CDlgComboStockFilter::OnBtnHisFitler()
{
	m_pTab->SetCurSel(1);
	m_pBtnHisFilter->SetAttribute(L"colorText", L"#FF0000FF");
	m_pBtnRtFilter->SetAttribute(L"colorText", L"#00FFFFFF");

}

void CDlgComboStockFilter::InitList(bool bUse, vector<StockFilter> &sfVec)
{
	SStringW strTmp;
	for (int i = 0; i < sfVec.size(); ++i)
	{
		ListAddItem(sfVec[i]);
		m_pCbxID->InsertItem(i, strTmp.Format(L"����%d", i + 1), NULL, 0);
		m_sfSet.insert(sfVec[i]);
	}
	m_pCheckUse->SetCheck(bUse);

}

void CDlgComboStockFilter::InitHisList(bool bUse, vector<HisStockFilter>& hsfVec)
{
	SStringW strTmp;
	for (int i = 0; i < hsfVec.size(); ++i)
	{
		HisListAddItem(hsfVec[i]);
		m_pCbxHisID->InsertItem(i, strTmp.Format(L"����%d", i + 1), NULL, 0);
		m_hisSfSet.insert(hsfVec[i]);
	}
	m_hisTmpSfSet = m_hisSfSet;
	m_pCheckHisUse->SetCheck(bUse);
}

void CDlgComboStockFilter::InitHisFilterRes()
{
	BOOL bUse = TRUE;
	m_pCheckHisUse->EnableWindow(FALSE, TRUE);
	m_pBtnSaveHis->EnableWindow(FALSE, TRUE);
	SendMsg(m_uParThreadID, WW_HisFilterStartCalc,
		(char*)&bUse, sizeof(bUse));
	CalcHisStockFilter();
}


void CDlgComboStockFilter::InitComboBox()
{
	for (int i = SFF_Add; i < SFF_Count; ++i)
		m_pCbxFunc->InsertItem(i, m_FuncMap[i], NULL, 0);
	m_pCbxFunc->SetCurSel(0);
	m_pCbxFunc->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxFuncChange, this));
	OnCbxFuncAdd();
	for (int i = SFP_D1; i < SFP_Count; ++i)
	{
		m_pCbxPeriod1->InsertItem(i, m_PeriodMap[i], NULL, 0);
		m_pCbxPeriod2->InsertItem(i, m_PeriodMap[i], NULL, 0);
	}
	m_pCbxPeriod1->SetCurSel(0);
	m_pCbxPeriod2->SetCurSel(0);
	int nCount = 0;
	for (int i = SFI_Start; i < SFI_Count; ++i)
	{
		if (m_IndexMap.count(i) )
		{
			m_pCbxIndex1->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index1PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L1IndyStart; i < SFI_L1IndyCount; ++i)
	{
		if (m_IndexMap.count(i))
		{
			m_pCbxIndex1->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index1PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L2IndyStart; i < SFI_L2IndyCount; ++i)
	{
		if (m_IndexMap.count(i))
		{
			m_pCbxIndex1->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index1PosMap[nCount] = i;
			++nCount;
		}
	}

	nCount = 0;
	for (int i = SFI_CloseRps520; i < SFI_Count; ++i)
	{
		if (m_IndexMap.count(i) && !m_bIndexOnlyIn1[i])
		{
			m_pCbxIndex2->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index2PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L1IndyStart; i < SFI_L1IndyCount; ++i)
	{
		if (m_IndexMap.count(i))
		{
			m_pCbxIndex2->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index2PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L2IndyStart; i < SFI_L2IndyCount; ++i)
	{
		if (m_IndexMap.count(i))
		{
			m_pCbxIndex2->InsertItem(nCount, m_IndexMap[i], NULL, 0);
			m_Index2PosMap[nCount] = i;
			++nCount;
		}
	}
	m_pCbxIndex1->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxIndex1Change, this));
	m_pCbxIndex1->SetCurSel(SFI_ChgPct);
	m_pCbxIndex2->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxIndex2Change, this));
	m_pCbxIndex2->SetCurSel(SFI_Num - 1);
	m_pCbxID->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxIDChange, this));

	for (int i = SFC_Greater; i < SFC_Count; ++i)
		m_pCbxCondition->InsertItem(i, m_ConditionMap[i], NULL, 0);
	m_pCbxCondition->SetCurSel(0);

}

void CDlgComboStockFilter::InitHisComboBox()
{
	for (int i = SFF_Add; i < SFF_Count; ++i)
		m_pCbxHisFunc->InsertItem(i, m_FuncMap[i], NULL, 0);
	m_pCbxHisFunc->SetCurSel(0);
	m_pCbxHisFunc->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxHisFuncChange, this));
	OnCbxHisFuncAdd();
	for (int i = SFP_D1; i < SFP_Count; ++i)
	{
		m_pCbxHisPeriod1->InsertItem(i, m_PeriodMap[i], NULL, 0);
		m_pCbxHisPeriod2->InsertItem(i, m_PeriodMap[i], NULL, 0);
	}
	m_pCbxHisPeriod1->SetCurSel(0);
	m_pCbxHisPeriod2->SetCurSel(0);
	int nCount = 0;
	for (int i = SFI_Start; i < SFI_Count; ++i)
	{
		if (m_hisIndexMap.count(i) && i != SFI_Num)
		{
			m_pCbxHisIndex1->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex1PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L1IndyStart; i < SFI_L1IndyCount; ++i)
	{
		if (m_hisIndexMap.count(i))
		{
			m_pCbxHisIndex1->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex1PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L2IndyStart; i < SFI_L2IndyCount; ++i)
	{
		if (m_hisIndexMap.count(i))
		{
			m_pCbxHisIndex1->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex1PosMap[nCount] = i;
			++nCount;
		}
	}

	nCount = 0;
	for (int i = SFI_CloseRps520; i < SFI_Count; ++i)
	{
		if (m_hisIndexMap.count(i))
		{
			m_pCbxHisIndex2->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex2PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L1IndyStart; i < SFI_L1IndyCount; ++i)
	{
		if (m_hisIndexMap.count(i))
		{
			m_pCbxHisIndex2->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex2PosMap[nCount] = i;
			++nCount;
		}
	}
	for (int i = SFI_L2IndyStart; i < SFI_L2IndyCount; ++i)
	{
		if (m_hisIndexMap.count(i))
		{
			m_pCbxHisIndex2->InsertItem(nCount, m_hisIndexMap[i], NULL, 0);
			m_hisIndex2PosMap[nCount] = i;
			++nCount;
		}
	}
	m_pCbxHisIndex1->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxHisIndex1Change, this));
	m_pCbxHisIndex1->SetCurSel(SFI_ChgPct);
	m_pCbxHisIndex2->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxHisIndex2Change, this));
	m_pCbxHisIndex2->SetCurSel(2);
	m_pCbxHisID->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
		Subscriber(&CDlgComboStockFilter::OnCbxHisIDChange, this));

	for (int i = SFC_Greater; i < SFC_Count; ++i)
		m_pCbxHisCondition->InsertItem(i, m_ConditionMap[i], NULL, 0);
	m_pCbxHisCondition->SetCurSel(0);

}


void CDlgComboStockFilter::OnCheckUse()
{
	BOOL bUsed = m_pCheckUse->IsChecked();
	SendMsg(m_uParThreadID, WW_ChangeStockFilter,
		(char*)&bUsed, sizeof(bUsed));

}

void CDlgComboStockFilter::OnCheckUseHis()
{
	m_pCheckHisUse->EnableWindow(FALSE, TRUE);
	m_pBtnSaveHis->EnableWindow(FALSE, TRUE);
	BOOL bUsed = m_pCheckHisUse->IsChecked();

	if (bUsed)
	{
		if (m_bHisChange || !m_bHisCalced)
		{
			m_hisSfSet = m_hisTmpSfSet;
			m_bHisChange = FALSE;
			if (m_pListHis->GetItemCount() <= 0)
			{
				m_pCheckHisUse->SetCheck(FALSE);
				SMessageBox(m_hWnd, L"��ǰ��ʷѡ��������Ϊ��,�Զ��ر�!", L"��ʾ", MB_OK);
				bUsed = FALSE;
				SendMsg(m_uParThreadID, WW_HisFilterStartCalc,
					(char*)&bUsed, sizeof(bUsed));
				m_pCheckHisUse->EnableWindow(TRUE, TRUE);
				m_pBtnSaveHis->EnableWindow(TRUE, TRUE);

			}
			else
			{
				SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);
				SendMsg(m_uParThreadID, WW_HisFilterStartCalc,
					(char*)&bUsed, sizeof(bUsed));
				CalcHisStockFilter();
				return;

			}
		}
		else
		{
			SendMsg(m_uParThreadID, WW_ChangeHisFiterState,
				(char*)&bUsed, sizeof(bUsed));
			m_pCheckHisUse->EnableWindow(TRUE, TRUE);
			m_pBtnSaveHis->EnableWindow(TRUE, TRUE);
		}

	}
	else
	{
		SendMsg(m_uParThreadID, WW_ChangeHisFiterState,
			(char*)&bUsed, sizeof(bUsed));
		m_pCheckHisUse->EnableWindow(TRUE, TRUE);
		m_pBtnSaveHis->EnableWindow(TRUE, TRUE);
	}

}


void CDlgComboStockFilter::StopFilter()
{
	m_pCheckUse->SetCheck(FALSE);
	m_pCheckHisUse->SetCheck(FALSE);
}

void CDlgComboStockFilter::SetStockInfo(vector<StockInfo>& stockInfo)
{
	m_stockInfo = stockInfo;
	m_HisFilterRes.resize(m_stockInfo.size(), FALSE);
}

void CDlgComboStockFilter::InitStringMap()
{
	m_FuncMap[SFF_Add] = L"����";
	m_FuncMap[SFF_Change] = L"�޸�";
	m_FuncMap[SFF_Delete] = L"ɾ��";

	m_ReverseFuncMap[L"����"] = SFF_Add;
	m_ReverseFuncMap[L"�޸�"] = SFF_Change;
	m_ReverseFuncMap[L"ɾ��"] = SFF_Delete;

	m_PeriodMap[SFP_Null] = L"-";
	m_PeriodMap[SFP_D1] = L"1��";
	m_PeriodMap[SFP_FS] = L"��ʱ";
	m_PeriodMap[SFP_M1] = L"1����";
	m_PeriodMap[SFP_M5] = L"5����";
	m_PeriodMap[SFP_M15] = L"15����";
	m_PeriodMap[SFP_M30] = L"30����";
	m_PeriodMap[SFP_M60] = L"60����";

	m_ReversePeriodMap[L"-"] = SFP_Null;
	m_ReversePeriodMap[L"1��"] = SFP_D1;
	m_ReversePeriodMap[L"��ʱ"] = SFP_FS;
	m_ReversePeriodMap[L"1����"] = SFP_M1;
	m_ReversePeriodMap[L"5����"] = SFP_M5;
	m_ReversePeriodMap[L"15����"] = SFP_M15;
	m_ReversePeriodMap[L"30����"] = SFP_M30;
	m_ReversePeriodMap[L"60����"] = SFP_M60;


	m_IndexMap[SFI_ChgPct] = L"�ǵ���";
	m_bOnlyInDay[SFI_ChgPct] = TRUE;
	m_IndexMap[SFI_CloseRps520] = L"���̼�RPS520";
	m_IndexMap[SFI_CloseMacd520] = L"���̼�Macd520";
	m_IndexMap[SFI_ClosePoint520] = L"���̼�520����";
	m_IndexMap[SFI_CloseRank520] = L"���̼�520����";
	m_IndexMap[SFI_CloseRps2060] = L"���̼�RPS2060";
	m_IndexMap[SFI_CloseMacd2060] = L"���̼�Macd2060";
	m_IndexMap[SFI_ClosePoint2060] = L"���̼�2060����";
	m_IndexMap[SFI_CloseRank2060] = L"���̼�2060����";
	m_IndexMap[SFI_Num] = L"��ֵ";
	m_IndexMap[SFI_ABSR] = L"��������";
	m_IndexMap[SFI_A2PBSR] = L"����ת��������";
	m_IndexMap[SFI_AABSR] = L"ƽ����������";
	m_IndexMap[SFI_POCR] = L"���ɽ��۱�";
	m_IndexMap[SFI_Vol] = L"�ɽ���(��)";
	m_IndexMap[SFI_Amount] = L"�ɽ���(��Ԫ)";
	m_IndexMap[SFI_AmountRps520] = L"�ɽ���RPS520";
	m_IndexMap[SFI_AmountMacd520] = L"�ɽ���Macd520";
	m_IndexMap[SFI_AmountPoint520] = L"�ɽ���520����";
	m_IndexMap[SFI_AmountRank520] = L"�ɽ���520����";
	m_IndexMap[SFI_AmountRps2060] = L"�ɽ���RPS2060";
	m_IndexMap[SFI_AmountMacd2060] = L"�ɽ���Macd2060";
	m_IndexMap[SFI_AmountPoint2060] = L"�ɽ���2060����";
	m_IndexMap[SFI_AmountRank2060] = L"�ɽ���2060����";
	m_IndexMap[SFI_AmountPoint] = L"�ɽ���������";
	m_IndexMap[SFI_AmountRank] = L"�ɽ����������";

	m_IndexMap[SFI_ClosePoint520L1] = L"���̼�520����(һ����ҵ��)";
	m_IndexMap[SFI_CloseRank520L1] = L"���̼�520����(һ����ҵ��)";
	m_IndexMap[SFI_ClosePoint2060L1] = L"���̼�2060����(һ����ҵ��)";
	m_IndexMap[SFI_CloseRank2060L1] = L"���̼�2060����(һ����ҵ��)";
	m_IndexMap[SFI_AmountPoint520L1] = L"�ɽ���520����(һ����ҵ��)";
	m_IndexMap[SFI_AmountRank520L1] = L"�ɽ���520����(һ����ҵ��)";
	m_IndexMap[SFI_AmountPoint2060L1] = L"�ɽ���2060����(һ����ҵ��)";
	m_IndexMap[SFI_AmountRank2060L1] = L"�ɽ���2060����(һ����ҵ��)";
	m_IndexMap[SFI_AmountPointL1] = L"�ɽ���������(һ����ҵ��)";
	m_IndexMap[SFI_AmountRankL1] = L"�ɽ����������(һ����ҵ��)";

	m_IndexMap[SFI_ClosePoint520L2] = L"���̼�520����(������ҵ��)";
	m_IndexMap[SFI_CloseRank520L2] = L"���̼�520����(������ҵ��)";
	m_IndexMap[SFI_ClosePoint2060L2] = L"���̼�2060����(������ҵ��)";
	m_IndexMap[SFI_CloseRank2060L2] = L"���̼�2060����(������ҵ��)";
	m_IndexMap[SFI_AmountPoint520L2] = L"�ɽ���520����(������ҵ��)";
	m_IndexMap[SFI_AmountRank520L2] = L"�ɽ���520����(������ҵ��)";
	m_IndexMap[SFI_AmountPoint2060L2] = L"�ɽ���2060����(������ҵ��)";
	m_IndexMap[SFI_AmountRank2060L2] = L"�ɽ���2060����(������ҵ��)";
	m_IndexMap[SFI_AmountPointL2] = L"�ɽ���������(������ҵ��)";
	m_IndexMap[SFI_AmountRankL2] = L"�ɽ����������(������ҵ��)";

	m_IndexMap[SFI_ABV] = L"����������";
	m_IndexMap[SFI_ASV] = L"����������";
	m_IndexMap[SFI_ABO] = L"�������뵥��";
	m_IndexMap[SFI_ASO] = L"������������";
	m_IndexMap[SFI_PBO] = L"�������뵥��";
	m_IndexMap[SFI_PSO] = L"������������";

	m_IndexMap[SFI_Open] = L"���̼�";
	m_IndexMap[SFI_High] = L"��߼�";
	m_IndexMap[SFI_Low] = L"��ͼ�";
	m_IndexMap[SFI_Close] = L"���̼�";


	m_IndexMap[SFI_CAVol] = L"���Ͼ��۳ɽ���";
	m_IndexMap[SFI_CAVolPoint] = L"���Ͼ��۳ɽ�������";
	m_IndexMap[SFI_CAVolRank] = L"���Ͼ��۳ɽ�������";
	m_IndexMap[SFI_CAVolPointL1] = L"���Ͼ��۳ɽ�������(һ����ҵ��)";
	m_IndexMap[SFI_CAVolRankL1] = L"���Ͼ��۳ɽ�������(һ����ҵ��)";
	m_IndexMap[SFI_CAVolPointL2] = L"���Ͼ��۳ɽ�������(������ҵ��)";
	m_IndexMap[SFI_CAVolRankL2] = L"���Ͼ��۳ɽ�������(������ҵ��)";

	m_IndexMap[SFI_CAAmo] = L"���Ͼ��۳ɽ���";
	m_IndexMap[SFI_CAAmoPoint] = L"���Ͼ��۳ɽ������";
	m_IndexMap[SFI_CAAmoRank] = L"���Ͼ��۳ɽ�������";
	m_IndexMap[SFI_CAAmoPointL1] = L"���Ͼ��۳ɽ������(һ����ҵ��)";
	m_IndexMap[SFI_CAAmoRankL1] = L"���Ͼ��۳ɽ�������(һ����ҵ��)";
	m_IndexMap[SFI_CAAmoPointL2] = L"���Ͼ��۳ɽ������(������ҵ��)";
	m_IndexMap[SFI_CAAmoRankL2] = L"���Ͼ��۳ɽ�������(������ҵ��)";


	m_IndexMap[SFI_L1IndyCloseRps520] = L"����һ����ҵ�����̼�RPS520";
	m_IndexMap[SFI_L1IndyCloseMacd520] = L"����һ����ҵ�����̼�Macd520";
	m_IndexMap[SFI_L1IndyClosePoint520] = L"����һ����ҵ�����̼�520����";
	m_IndexMap[SFI_L1IndyCloseRank520] = L"����һ����ҵ�����̼�520����";
	m_IndexMap[SFI_L1IndyCloseRps2060] = L"����һ����ҵ�����̼�RPS2060";
	m_IndexMap[SFI_L1IndyCloseMacd2060] = L"����һ����ҵ�����̼�Macd2060";
	m_IndexMap[SFI_L1IndyClosePoint2060] = L"����һ����ҵ�����̼�2060����";
	m_IndexMap[SFI_L1IndyCloseRank2060] = L"����һ����ҵ�����̼�2060����";
	m_IndexMap[SFI_L1IndyAmountRps520] = L"����һ����ҵ�ĳɽ���RPS520";
	m_IndexMap[SFI_L1IndyAmountMacd520] = L"����һ����ҵ�ĳɽ���Macd520";
	m_IndexMap[SFI_L1IndyAmountPoint520] = L"����һ����ҵ�ĳɽ���520����";
	m_IndexMap[SFI_L1IndyAmountRank520] = L"����һ����ҵ�ĳɽ���520����";
	m_IndexMap[SFI_L1IndyAmountRps2060] = L"����һ����ҵ�ĳɽ���RPS2060";
	m_IndexMap[SFI_L1IndyAmountMacd2060] = L"����һ����ҵ�ĳɽ���Macd2060";
	m_IndexMap[SFI_L1IndyAmountPoint2060] = L"����һ����ҵ�ĳɽ���2060����";
	m_IndexMap[SFI_L1IndyAmountRank2060] = L"����һ����ҵ�ĳɽ���2060����";
	m_IndexMap[SFI_L1IndyAmountPoint] = L"����һ����ҵ�ĳɽ���������";
	m_IndexMap[SFI_L1IndyAmountRank] = L"����һ����ҵ�ĳɽ����������";

	m_IndexMap[SFI_L2IndyCloseRps520] = L"���ڶ�����ҵ�����̼�RPS520";
	m_IndexMap[SFI_L2IndyCloseMacd520] = L"���ڶ�����ҵ�����̼�Macd520";
	m_IndexMap[SFI_L2IndyClosePoint520] = L"���ڶ�����ҵ�����̼�520����";
	m_IndexMap[SFI_L2IndyCloseRank520] = L"���ڶ�����ҵ�����̼�520����";
	m_IndexMap[SFI_L2IndyCloseRps2060] = L"���ڶ�����ҵ�����̼�RPS2060";
	m_IndexMap[SFI_L2IndyCloseMacd2060] = L"���ڶ�����ҵ�����̼�Macd2060";
	m_IndexMap[SFI_L2IndyClosePoint2060] = L"���ڶ�����ҵ�����̼�2060����";
	m_IndexMap[SFI_L2IndyCloseRank2060] = L"���ڶ�����ҵ�����̼�2060����";
	m_IndexMap[SFI_L2IndyAmountRps520] = L"���ڶ�����ҵ�ĳɽ���RPS520";
	m_IndexMap[SFI_L2IndyAmountMacd520] = L"���ڶ�����ҵ�ĳɽ���Macd520";
	m_IndexMap[SFI_L2IndyAmountPoint520] = L"���ڶ�����ҵ�ĳɽ���520����";
	m_IndexMap[SFI_L2IndyAmountRank520] = L"���ڶ�����ҵ�ĳɽ���520����";
	m_IndexMap[SFI_L2IndyAmountRps2060] = L"���ڶ�����ҵ�ĳɽ���RPS2060";
	m_IndexMap[SFI_L2IndyAmountMacd2060] = L"���ڶ�����ҵ�ĳɽ���Macd2060";
	m_IndexMap[SFI_L2IndyAmountPoint2060] = L"���ڶ�����ҵ�ĳɽ���2060����";
	m_IndexMap[SFI_L2IndyAmountRank2060] = L"���ڶ�����ҵ�ĳɽ���2060����";
	m_IndexMap[SFI_L2IndyAmountPoint] = L"���ڶ�����ҵ�ĳɽ���������";
	m_IndexMap[SFI_L2IndyAmountRank] = L"���ڶ�����ҵ�ĳɽ����������";

	m_IndexMap[SFI_VolMome] = L"�ɽ�����";
	m_IndexMap[SFI_AbsMome] = L"���Զ���";
	m_IndexMap[SFI_StructState] = L"�ṹ״̬";
	m_IndexMap[SFI_StrengthFilter] = L"ǿ��ѡ��";
	m_IndexMap[SFI_AbsStrength] = L"����ǿ��";
	m_IndexMap[SFI_FlatBoard] = L"һ�ְ�";
	m_IndexMap[SFI_PeriBottom] = L"Ǳ����";
	m_IndexMap[SFI_BottomStart] = L"�ײ�����";
	m_IndexMap[SFI_JumpOver] = L"ͻ�ƻز�";

	m_bIndexOnlyIn1[SFI_VolMome] = TRUE;
	m_bIndexOnlyIn1[SFI_AbsMome] = TRUE;
	m_bIndexOnlyIn1[SFI_StructState] = TRUE;
	m_bIndexOnlyIn1[SFI_StrengthFilter] = TRUE;
	m_bIndexOnlyIn1[SFI_AbsStrength] = TRUE;
	m_bIndexOnlyIn1[SFI_FlatBoard] = TRUE;
	m_bIndexOnlyIn1[SFI_PeriBottom] = TRUE;
	m_bIndexOnlyIn1[SFI_BottomStart] = TRUE;
	m_bIndexOnlyIn1[SFI_JumpOver] = TRUE;

	m_bOnlyInDay[SFI_StrengthFilter] = TRUE;
	m_bOnlyInDay[SFI_AbsStrength] = TRUE;
	m_bOnlyInDay[SFI_FlatBoard] = TRUE;
	m_bOnlyInDay[SFI_PeriBottom] = TRUE;
	m_bOnlyInDay[SFI_BottomStart] = TRUE;
	m_bOnlyInDay[SFI_JumpOver] = TRUE;

	m_bOnlyBool[SFI_VolMome] = TRUE;
	m_bOnlyBool[SFI_AbsMome] = TRUE;
	m_bOnlyBool[SFI_StructState] = TRUE;
	m_bOnlyBool[SFI_StrengthFilter] = TRUE;
	m_bOnlyBool[SFI_AbsStrength] = TRUE;
	m_bOnlyBool[SFI_FlatBoard] = TRUE;
	m_bOnlyBool[SFI_PeriBottom] = TRUE;
	m_bOnlyBool[SFI_BottomStart] = TRUE;
	m_bOnlyBool[SFI_JumpOver] = TRUE;



	m_ReverseIndexMap[L"�ǵ���"] = SFI_ChgPct;
	m_ReverseIndexMap[L"���̼�RPS520"] = SFI_CloseRps520;
	m_ReverseIndexMap[L"���̼�Macd520"] = SFI_CloseMacd520;
	m_ReverseIndexMap[L"���̼�520����"] = SFI_ClosePoint520;
	m_ReverseIndexMap[L"���̼�520����"] = SFI_CloseRank520;
	m_ReverseIndexMap[L"���̼�RPS2060"] = SFI_CloseRps2060;
	m_ReverseIndexMap[L"���̼�Macd2060"] = SFI_CloseMacd2060;
	m_ReverseIndexMap[L"���̼�2060����"] = SFI_ClosePoint2060;
	m_ReverseIndexMap[L"���̼�2060����"] = SFI_CloseRank2060;
	m_ReverseIndexMap[L"��ֵ"] = SFI_Num;
	m_ReverseIndexMap[L"��������"] = SFI_ABSR;
	m_ReverseIndexMap[L"����ת��������"] = SFI_A2PBSR;
	m_ReverseIndexMap[L"ƽ����������"] = SFI_AABSR;
	m_ReverseIndexMap[L"���ɽ��۱�"] = SFI_POCR;

	m_ReverseIndexMap[L"�ɽ���(��)"] = SFI_Vol;
	m_ReverseIndexMap[L"�ɽ���(��Ԫ)"] = SFI_Amount;

	m_ReverseIndexMap[L"�ɽ���RPS520"] = SFI_AmountRps520;
	m_ReverseIndexMap[L"�ɽ���Macd520"] = SFI_AmountMacd520;
	m_ReverseIndexMap[L"�ɽ���520����"] = SFI_AmountPoint520;
	m_ReverseIndexMap[L"�ɽ���520����"] = SFI_AmountRank520;
	m_ReverseIndexMap[L"�ɽ���RPS2060"] = SFI_AmountRps2060;
	m_ReverseIndexMap[L"�ɽ���Macd2060"] = SFI_AmountMacd2060;
	m_ReverseIndexMap[L"�ɽ���2060����"] = SFI_AmountPoint2060;
	m_ReverseIndexMap[L"�ɽ���2060����"] = SFI_AmountRank2060;
	m_ReverseIndexMap[L"�ɽ���������"] = SFI_AmountPoint;
	m_ReverseIndexMap[L"�ɽ����������"] = SFI_AmountRank;

	m_ReverseIndexMap[L"���̼�RPS520(һ����ҵ��)"] = SFI_CloseRps520L1;
	m_ReverseIndexMap[L"���̼�Macd520(һ����ҵ��)"] = SFI_CloseMacd520L1;
	m_ReverseIndexMap[L"���̼�520����(һ����ҵ��)"] = SFI_ClosePoint520L1;
	m_ReverseIndexMap[L"���̼�520����(һ����ҵ��)"] = SFI_CloseRank520L1;
	m_ReverseIndexMap[L"���̼�RPS2060(һ����ҵ��)"] = SFI_CloseRps2060L1;
	m_ReverseIndexMap[L"���̼�Macd2060(һ����ҵ��)"] = SFI_CloseMacd2060L1;
	m_ReverseIndexMap[L"���̼�2060����(һ����ҵ��)"] = SFI_ClosePoint2060L1;
	m_ReverseIndexMap[L"���̼�2060����(һ����ҵ��)"] = SFI_CloseRank2060L1;
	m_ReverseIndexMap[L"�ɽ���RPS520(һ����ҵ��)"] = SFI_AmountRps520L1;
	m_ReverseIndexMap[L"�ɽ���Macd520(һ����ҵ��)"] = SFI_AmountMacd520L1;
	m_ReverseIndexMap[L"�ɽ���520����(һ����ҵ��)"] = SFI_AmountPoint520L1;
	m_ReverseIndexMap[L"�ɽ���520����(һ����ҵ��)"] = SFI_AmountRank520L1;
	m_ReverseIndexMap[L"�ɽ���RPS2060(һ����ҵ��)"] = SFI_AmountRps2060L1;
	m_ReverseIndexMap[L"�ɽ���Macd2060(һ����ҵ��)"] = SFI_AmountMacd2060L1;
	m_ReverseIndexMap[L"�ɽ���2060����(һ����ҵ��)"] = SFI_AmountPoint2060L1;
	m_ReverseIndexMap[L"�ɽ���2060����(һ����ҵ��)"] = SFI_AmountRank2060L1;
	m_ReverseIndexMap[L"�ɽ���������(һ����ҵ��)"] = SFI_AmountPointL1;
	m_ReverseIndexMap[L"�ɽ����������(һ����ҵ��)"] = SFI_AmountRankL1;

	m_ReverseIndexMap[L"���̼�RPS520(������ҵ��)"] = SFI_CloseRps520L2;
	m_ReverseIndexMap[L"���̼�Macd520(������ҵ��)"] = SFI_CloseMacd520L2;
	m_ReverseIndexMap[L"���̼�520����(������ҵ��)"] = SFI_ClosePoint520L2;
	m_ReverseIndexMap[L"���̼�520����(������ҵ��)"] = SFI_CloseRank520L2;
	m_ReverseIndexMap[L"���̼�RPS2060(������ҵ��)"] = SFI_CloseRps2060L2;
	m_ReverseIndexMap[L"���̼�Macd2060(������ҵ��)"] = SFI_CloseMacd2060L2;
	m_ReverseIndexMap[L"���̼�2060����(������ҵ��)"] = SFI_ClosePoint2060L2;
	m_ReverseIndexMap[L"���̼�2060����(������ҵ��)"] = SFI_CloseRank2060L2;
	m_ReverseIndexMap[L"�ɽ���RPS520(������ҵ��)"] = SFI_AmountRps520L2;
	m_ReverseIndexMap[L"�ɽ���Macd520(������ҵ��)"] = SFI_AmountMacd520L2;
	m_ReverseIndexMap[L"�ɽ���520����(������ҵ��)"] = SFI_AmountPoint520L2;
	m_ReverseIndexMap[L"�ɽ���520����(������ҵ��)"] = SFI_AmountRank520L2;
	m_ReverseIndexMap[L"�ɽ���RPS2060(������ҵ��)"] = SFI_AmountRps2060L2;
	m_ReverseIndexMap[L"�ɽ���Macd2060(������ҵ��)"] = SFI_AmountMacd2060L2;
	m_ReverseIndexMap[L"�ɽ���2060����(������ҵ��)"] = SFI_AmountPoint2060L2;
	m_ReverseIndexMap[L"�ɽ���2060����(������ҵ��)"] = SFI_AmountRank2060L2;
	m_ReverseIndexMap[L"�ɽ���������(������ҵ��)"] = SFI_AmountPointL2;
	m_ReverseIndexMap[L"�ɽ����������(������ҵ��)"] = SFI_AmountRankL2;

	m_ReverseIndexMap[L"����������"] = SFI_ABV;
	m_ReverseIndexMap[L"����������"] = SFI_ASV;
	m_ReverseIndexMap[L"�������뵥��"] = SFI_ABO;
	m_ReverseIndexMap[L"������������"] = SFI_ASO;
	m_ReverseIndexMap[L"�������뵥��"] = SFI_PBO;
	m_ReverseIndexMap[L"������������"] = SFI_PSO;

	m_ReverseIndexMap[L"���̼�"] = SFI_Open;
	m_ReverseIndexMap[L"��߼�"] = SFI_High;
	m_ReverseIndexMap[L"��ͼ�"] = SFI_Low;
	m_ReverseIndexMap[L"���̼�"] = SFI_Close;

	m_ReverseIndexMap[L"���Ͼ��۳ɽ���"] = SFI_CAVol;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������"] = SFI_CAVolPoint;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������"] = SFI_CAVolRank;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(һ����ҵ��)"] = SFI_CAVolPointL1;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(һ����ҵ��)"] = SFI_CAVolRankL1;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(������ҵ��)"] = SFI_CAVolPointL2;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(������ҵ��)"] = SFI_CAVolRankL2;


	m_ReverseIndexMap[L"���Ͼ��۳ɽ���"] = SFI_CAAmo;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ������"] = SFI_CAAmoPoint;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������"] = SFI_CAAmoRank;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ������(һ����ҵ��)"] = SFI_CAAmoPointL1;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(һ����ҵ��)"] = SFI_CAAmoRankL1;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ������(������ҵ��)"] = SFI_CAAmoPointL2;
	m_ReverseIndexMap[L"���Ͼ��۳ɽ�������(������ҵ��)"] = SFI_CAAmoRankL2;



	m_ReverseIndexMap[L"����һ����ҵ�����̼�RPS520"] = SFI_L1IndyCloseRps520;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�Macd520"] = SFI_L1IndyCloseMacd520;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�520����"] = SFI_L1IndyClosePoint520;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�520����"] = SFI_L1IndyCloseRank520;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�RPS2060"] = SFI_L1IndyCloseRps2060;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�Macd2060"] = SFI_L1IndyCloseMacd2060;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�2060����"] = SFI_L1IndyClosePoint2060;
	m_ReverseIndexMap[L"����һ����ҵ�����̼�2060����"] = SFI_L1IndyCloseRank2060;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���RPS520"] = SFI_L1IndyAmountRps520;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���Macd520"] = SFI_L1IndyAmountMacd520;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���520����"] = SFI_L1IndyAmountPoint520;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���520����"] = SFI_L1IndyAmountRank520;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���RPS2060"] = SFI_L1IndyAmountRps2060;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���Macd2060"] = SFI_L1IndyAmountMacd2060;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���2060����"] = SFI_L1IndyAmountPoint2060;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���2060����"] = SFI_L1IndyAmountRank2060;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ���������"] = SFI_L1IndyAmountPoint;
	m_ReverseIndexMap[L"����һ����ҵ�ĳɽ����������"] = SFI_L1IndyAmountRank;

	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�RPS520"] = SFI_L2IndyCloseRps520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�Macd520"] = SFI_L2IndyCloseMacd520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�520����"] = SFI_L2IndyClosePoint520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�520����"] = SFI_L2IndyCloseRank520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�RPS2060"] = SFI_L2IndyCloseRps2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�Macd2060"] = SFI_L2IndyCloseMacd2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�2060����"] = SFI_L2IndyClosePoint2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�����̼�2060����"] = SFI_L2IndyCloseRank2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���RPS520"] = SFI_L2IndyAmountRps520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���Macd520"] = SFI_L2IndyAmountMacd520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���520����"] = SFI_L2IndyAmountPoint520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���520����"] = SFI_L2IndyAmountRank520;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���RPS2060"] = SFI_L2IndyAmountRps2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���Macd2060"] = SFI_L2IndyAmountMacd2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���2060����"] = SFI_L2IndyAmountPoint2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���2060����"] = SFI_L2IndyAmountRank2060;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ���������"] = SFI_L2IndyAmountPoint;
	m_ReverseIndexMap[L"���ڶ�����ҵ�ĳɽ����������"] = SFI_L2IndyAmountRank;

	m_ReverseIndexMap[L"�ɽ�����"] = SFI_VolMome;
	m_ReverseIndexMap[L"���Զ���"] = SFI_AbsMome;
	m_ReverseIndexMap[L"�ṹ״̬"] = SFI_StructState;
	m_ReverseIndexMap[L"ǿ��ѡ��"] = SFI_StrengthFilter;
	m_ReverseIndexMap[L"����ǿ��"] = SFI_AbsStrength;
	m_ReverseIndexMap[L"һ�ְ�"] = SFI_FlatBoard;
	m_ReverseIndexMap[L"Ǳ����"] = SFI_PeriBottom;
	m_ReverseIndexMap[L"�ײ�����"] = SFI_BottomStart;
	m_ReverseIndexMap[L"ͻ�ƻز�"] = SFI_JumpOver;


	m_ConditionMap[SFC_Greater] = L"����";
	m_ConditionMap[SFC_EqualOrGreater] = L"���ڵ���";
	m_ConditionMap[SFC_Equal] = L"����";
	m_ConditionMap[SFC_EqualOrLess] = L"С�ڵ���";
	m_ConditionMap[SFC_Less] = L"С��";

	m_ReverseConditionMap[L"����"] = SFC_Greater;
	m_ReverseConditionMap[L"���ڵ���"] = SFC_EqualOrGreater;
	m_ReverseConditionMap[L"����"] = SFC_Equal;
	m_ReverseConditionMap[L"С�ڵ���"] = SFC_EqualOrLess;
	m_ReverseConditionMap[L"С��"] = SFC_Less;



	m_NumUint[SFI_ChgPct] = L"(-100 ~ 100)";
	m_NumUint[SFI_ClosePoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_ClosePoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_ClosePoint520L1] = L"(0 ~ 100)";
	m_NumUint[SFI_ClosePoint2060L1] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint520L1] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint2060L1] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPointL1] = L"(0 ~ 100)";
	m_NumUint[SFI_ClosePoint520L2] = L"(0 ~ 100)";
	m_NumUint[SFI_ClosePoint2060L2] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint520L2] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPoint2060L2] = L"(0 ~ 100)";
	m_NumUint[SFI_AmountPointL2] = L"(0 ~ 100)";
	m_NumUint[SFI_CloseRank520] = L"(> 0)";
	m_NumUint[SFI_CloseRank2060] = L"(> 0)";
	m_NumUint[SFI_AmountRank520] = L"(> 0)";
	m_NumUint[SFI_AmountRank2060] = L"(> 0)";
	m_NumUint[SFI_AmountRank] = L"(> 0)";
	m_NumUint[SFI_CloseRank520L1] = L"(> 0)";
	m_NumUint[SFI_CloseRank2060L1] = L"(> 0)";
	m_NumUint[SFI_AmountRank520L1] = L"(> 0)";
	m_NumUint[SFI_AmountRank2060L1] = L"(> 0)";
	m_NumUint[SFI_AmountRankL1] = L"(> 0)";
	m_NumUint[SFI_CloseRank520L2] = L"(> 0)";
	m_NumUint[SFI_CloseRank2060L2] = L"(> 0)";
	m_NumUint[SFI_AmountRank520L2] = L"(> 0)";
	m_NumUint[SFI_AmountRank2060L2] = L"(> 0)";
	m_NumUint[SFI_AmountRankL2] = L"(> 0)";
	m_NumUint[SFI_ABSR] = L"(-100 ~ 100)";
	m_NumUint[SFI_A2PBSR] = L"(-100 ~ 100)";
	m_NumUint[SFI_AABSR] = L"(-100 ~ 100)";
	m_NumUint[SFI_POCR] = L"(-100 ~ 100)";
	m_NumUint[SFI_Vol] = L"(��λ:��)";
	m_NumUint[SFI_Amount] = L"(��λ:��Ԫ)";

	m_NumUint[SFI_CAVol] = L"(��λ:��)";
	m_NumUint[SFI_CAVolPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRank] = L"(> 0)";
	m_NumUint[SFI_CAVolPointL1] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRankL1] = L"(> 0)";
	m_NumUint[SFI_CAVolPointL2] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRankL2] = L"(> 0)";

	m_NumUint[SFI_CAAmo] = L"(��λ:��Ԫ)";
	m_NumUint[SFI_CAAmoPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRank] = L"(> 0)";
	m_NumUint[SFI_CAAmoPointL1] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRankL1] = L"(> 0)";
	m_NumUint[SFI_CAAmoPointL2] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRankL2] = L"(> 0)";

	m_NumUint[SFI_VolMome] = L"��0��1��1���� 0����������";
	m_NumUint[SFI_AbsMome] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_StructState] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_StrengthFilter] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_AbsStrength] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_FlatBoard] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_PeriBottom] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_BottomStart] = L"��0��1��1���� 0����������";;
	m_NumUint[SFI_JumpOver] = L"��0��1��1���� 0����������";;




	m_NumUint[SFI_L1IndyClosePoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_L1IndyClosePoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_L1IndyAmountPoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_L1IndyAmountPoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_L1IndyAmountPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_L2IndyClosePoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_L2IndyClosePoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_L2IndyAmountPoint520] = L"(0 ~ 100)";
	m_NumUint[SFI_L2IndyAmountPoint2060] = L"(0 ~ 100)";
	m_NumUint[SFI_L2IndyAmountPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_L1IndyCloseRank520] = L"(> 0)";
	m_NumUint[SFI_L1IndyCloseRank2060] = L"(> 0)";
	m_NumUint[SFI_L1IndyAmountRank520] = L"(> 0)";
	m_NumUint[SFI_L1IndyAmountRank2060] = L"(> 0)";
	m_NumUint[SFI_L1IndyAmountRank] = L"(> 0)";
	m_NumUint[SFI_L2IndyCloseRank520] = L"(> 0)";
	m_NumUint[SFI_L2IndyCloseRank2060] = L"(> 0)";
	m_NumUint[SFI_L2IndyAmountRank520] = L"(> 0)";
	m_NumUint[SFI_L2IndyAmountRank2060] = L"(> 0)";
	m_NumUint[SFI_L2IndyAmountRank] = L"(> 0)";
}

void CDlgComboStockFilter::InitHisStringMap()
{
	m_hisIndexMap[SFI_ChgPct] = L"�ǵ���";
	m_hisIndexMap[SFI_ClosePoint520] = L"���̼�520����";
	m_hisIndexMap[SFI_ClosePoint2060] = L"���̼�2060����";
	m_hisIndexMap[SFI_Num] = L"��ֵ";
	m_hisIndexMap[SFI_ABSR] = L"��������";
	m_hisIndexMap[SFI_A2PBSR] = L"����ת��������";
	m_hisIndexMap[SFI_AABSR] = L"ƽ����������";
	m_hisIndexMap[SFI_POCR] = L"���ɽ��۱�";
	m_hisIndexMap[SFI_Vol] = L"�ɽ���(��)";
	m_hisIndexMap[SFI_Amount] = L"�ɽ���(��Ԫ)";
	m_hisIndexMap[SFI_AmountPoint520] = L"�ɽ���520����";
	m_hisIndexMap[SFI_AmountPoint2060] = L"�ɽ���2060����";
	m_hisIndexMap[SFI_AmountPoint] = L"�ɽ���������";

	m_hisIndexMap[SFI_ClosePoint520L1] = L"���̼�520����(һ����ҵ��)";
	m_hisIndexMap[SFI_ClosePoint2060L1] = L"���̼�2060����(һ����ҵ��)";
	m_hisIndexMap[SFI_AmountPoint520L1] = L"�ɽ���520����(һ����ҵ��)";
	m_hisIndexMap[SFI_AmountPoint2060L1] = L"�ɽ���2060����(һ����ҵ��)";
	m_hisIndexMap[SFI_AmountPointL1] = L"�ɽ���������(һ����ҵ��)";

	m_hisIndexMap[SFI_ClosePoint520L2] = L"���̼�520����(������ҵ��)";
	m_hisIndexMap[SFI_ClosePoint2060L2] = L"���̼�2060����(������ҵ��)";
	m_hisIndexMap[SFI_AmountPoint520L2] = L"�ɽ���520����(������ҵ��)";
	m_hisIndexMap[SFI_AmountPoint2060L2] = L"�ɽ���2060����(������ҵ��)";
	m_hisIndexMap[SFI_AmountPointL2] = L"�ɽ���������(������ҵ��)";

	m_hisIndexMap[SFI_ABV] = L"����������";
	m_hisIndexMap[SFI_ASV] = L"����������";
	m_hisIndexMap[SFI_ABO] = L"�������뵥��";
	m_hisIndexMap[SFI_ASO] = L"������������";
	m_hisIndexMap[SFI_PBO] = L"�������뵥��";
	m_hisIndexMap[SFI_PSO] = L"������������";

	m_hisIndexMap[SFI_Open] = L"���̼�";
	m_hisIndexMap[SFI_High] = L"��߼�";
	m_hisIndexMap[SFI_Low] = L"��ͼ�";
	m_hisIndexMap[SFI_Close] = L"���̼�";


	m_hisIndexMap[SFI_CAVol] = L"���Ͼ��۳ɽ���";
	m_hisIndexMap[SFI_CAVolPoint] = L"���Ͼ��۳ɽ�������";
	m_hisIndexMap[SFI_CAVolPointL1] = L"���Ͼ��۳ɽ�������(һ����ҵ��)";
	m_hisIndexMap[SFI_CAVolPointL2] = L"���Ͼ��۳ɽ�������(������ҵ��)";

	m_hisIndexMap[SFI_CAAmo] = L"���Ͼ��۳ɽ���";
	m_hisIndexMap[SFI_CAAmoPoint] = L"���Ͼ��۳ɽ������";
	m_hisIndexMap[SFI_CAAmoPointL1] = L"���Ͼ��۳ɽ������(һ����ҵ��)";
	m_hisIndexMap[SFI_CAAmoPointL2] = L"���Ͼ��۳ɽ������(������ҵ��)";


	m_hisIndexMap[SFI_L1IndyClosePoint520] = L"����һ����ҵ�����̼�520����";
	m_hisIndexMap[SFI_L1IndyClosePoint2060] = L"����һ����ҵ�����̼�2060����";
	m_hisIndexMap[SFI_L1IndyAmountPoint520] = L"����һ����ҵ�ĳɽ���520����";
	m_hisIndexMap[SFI_L1IndyAmountPoint2060] = L"����һ����ҵ�ĳɽ���2060����";
	m_hisIndexMap[SFI_L1IndyAmountPoint] = L"����һ����ҵ�ĳɽ���������";

	m_hisIndexMap[SFI_L2IndyClosePoint520] = L"���ڶ�����ҵ�����̼�520����";
	m_hisIndexMap[SFI_L2IndyClosePoint2060] = L"���ڶ�����ҵ�����̼�2060����";
	m_hisIndexMap[SFI_L2IndyAmountPoint520] = L"���ڶ�����ҵ�ĳɽ���520����";
	m_hisIndexMap[SFI_L2IndyAmountPoint2060] = L"���ڶ�����ҵ�ĳɽ���2060����";
	m_hisIndexMap[SFI_L2IndyAmountPoint] = L"���ڶ�����ҵ�ĳɽ���������";

	m_JudgeTypeMap[eJT_Exist] = L"����";
	m_JudgeTypeMap[eJT_Forall] = L"ȫ������";

	m_ReverseJudgeTypeMap[L"����"] = eJT_Exist;
	m_ReverseJudgeTypeMap[L"ȫ������"] = eJT_Forall;

}

void CDlgComboStockFilter::OutPutCondition(vector<StockFilter>& sfVec)
{
	sfVec.clear();
	for (int i = 0; i < m_pList->GetItemCount(); ++i)
	{
		StockFilter sf = { 0 };
		if (GetListItem(i, sf))
			sfVec.emplace_back(sf);
	}

}

void CDlgComboStockFilter::OutPutHisCondition(vector<HisStockFilter>& sfVec)
{
	sfVec.clear();
	for (int i = 0; i < m_pListHis->GetItemCount(); ++i)
	{
		HisStockFilter hsf = { 0 };
		if (GetHisListItem(i, hsf))
			sfVec.emplace_back(hsf);
	}
}


void CDlgComboStockFilter::ListAddItem(StockFilter & sf)
{
	int nID = m_pList->GetItemCount() + 1;
	SStringW strTmp;
	strTmp.Format(L"%d", nID);
	m_pList->InsertItem(nID - 1, strTmp);
	m_pList->SetSubItemText(nID - 1, SFLH_Index1, m_IndexMap[sf.index1]);
	m_pList->SetSubItemText(nID - 1, SFLH_Period1, m_PeriodMap[sf.period1]);
	m_pList->SetSubItemText(nID - 1, SFLH_Condition, m_ConditionMap[sf.condition]);
	m_pList->SetSubItemText(nID - 1, SFLH_Index2, m_IndexMap[sf.index2]);
	if (sf.index2 == SFI_Num)
	{
		strTmp.Format(L"%.02f", sf.num);
		m_pList->SetSubItemText(nID - 1, SFLH_Period2OrNum, strTmp);
	}
	else
		m_pList->SetSubItemText(nID - 1, SFLH_Period2OrNum, m_PeriodMap[sf.period2]);
}

void CDlgComboStockFilter::ListChangeItem(int nRow, StockFilter & sf)
{
	if (nRow >= m_pList->GetItemCount() || nRow < 0)
		return;
	SStringW strTmp;
	m_pList->SetSubItemText(nRow, SFLH_Index1, m_IndexMap[sf.index1]);
	m_pList->SetSubItemText(nRow, SFLH_Period1, m_PeriodMap[sf.period1]);
	m_pList->SetSubItemText(nRow, SFLH_Condition, m_ConditionMap[sf.condition]);
	m_pList->SetSubItemText(nRow, SFLH_Index2, m_IndexMap[sf.index2]);
	if (sf.index2 == SFI_Num)
	{
		strTmp.Format(L"%.02f", sf.num);
		m_pList->SetSubItemText(nRow, SFLH_Period2OrNum, strTmp);
	}
	else
		m_pList->SetSubItemText(nRow, SFLH_Period2OrNum, m_PeriodMap[sf.period2]);

}

void CDlgComboStockFilter::ListDeleteItem(int nRow)
{
	if (nRow >= m_pList->GetItemCount() || nRow < 0)
		return;
	m_pList->DeleteItem(nRow);
	SStringW strTmp;
	for (int i = nRow; i < m_pList->GetItemCount(); ++i)
		m_pList->SetSubItemText(i, SFLH_ID, strTmp.Format(L"%d", i + 1));
}

bool CDlgComboStockFilter::GetListItem(int nRow, StockFilter& sf)
{
	if (nRow >= m_pList->GetItemCount() || nRow < 0)
		return false;
	SStringW strTmp = m_pList->GetSubItemText(nRow, SFLH_Index1);
	sf.index1 = m_ReverseIndexMap[strTmp];
	strTmp = m_pList->GetSubItemText(nRow, SFLH_Period1);
	sf.period1 = m_ReversePeriodMap[strTmp];
	strTmp = m_pList->GetSubItemText(nRow, SFLH_Condition);
	sf.condition = m_ReverseConditionMap[strTmp];
	strTmp = m_pList->GetSubItemText(nRow, SFLH_Index2);
	sf.index2 = m_ReverseIndexMap[strTmp];
	strTmp = m_pList->GetSubItemText(nRow, SFLH_Period2OrNum);
	if (SFI_Num == sf.index2)
		sf.num = _wtof(strTmp);
	else
		sf.period2 = m_ReversePeriodMap[strTmp];
	return true;
}

void CDlgComboStockFilter::HisListAddItem(HisStockFilter & hsf)
{
	int nID = m_pListHis->GetItemCount() + 1;
	SStringW strTmp;
	strTmp.Format(L"%d", nID);
	m_pListHis->InsertItem(nID - 1, strTmp);
	m_pListHis->SetSubItemText(nID - 1, SFLH_Index1, m_IndexMap[hsf.sf.index1]);
	m_pListHis->SetSubItemText(nID - 1, SFLH_Period1, m_PeriodMap[hsf.sf.period1]);
	m_pListHis->SetSubItemText(nID - 1, SFLH_Condition, m_ConditionMap[hsf.sf.condition]);
	m_pListHis->SetSubItemText(nID - 1, SFLH_Index2, m_IndexMap[hsf.sf.index2]);
	if (hsf.sf.index2 == SFI_Num)
	{
		strTmp.Format(L"%.02f", hsf.sf.num);
		m_pListHis->SetSubItemText(nID - 1, SFLH_Period2OrNum, strTmp);
	}
	else
		m_pListHis->SetSubItemText(nID - 1, SFLH_Period2OrNum, m_PeriodMap[hsf.sf.period2]);
	m_pListHis->SetSubItemText(nID - 1, SFLH_CountDay, strTmp.Format(L"%d", hsf.countDay));
	m_pListHis->SetSubItemText(nID - 1, SFLH_JudgeType, m_JudgeTypeMap[hsf.type]);

}

void CDlgComboStockFilter::HisListChangeItem(int nRow, HisStockFilter & hsf)
{
	if (nRow >= m_pListHis->GetItemCount() || nRow < 0)
		return;
	SStringW strTmp;
	m_pListHis->SetSubItemText(nRow, SFLH_Index1, m_IndexMap[hsf.sf.index1]);
	m_pListHis->SetSubItemText(nRow, SFLH_Period1, m_PeriodMap[hsf.sf.period1]);
	m_pListHis->SetSubItemText(nRow, SFLH_Condition, m_ConditionMap[hsf.sf.condition]);
	m_pListHis->SetSubItemText(nRow, SFLH_Index2, m_IndexMap[hsf.sf.index2]);
	if (hsf.sf.index2 == SFI_Num)
	{
		strTmp.Format(L"%.02f", hsf.sf.num);
		m_pListHis->SetSubItemText(nRow, SFLH_Period2OrNum, strTmp);
	}
	else
		m_pListHis->SetSubItemText(nRow, SFLH_Period2OrNum, m_PeriodMap[hsf.sf.period2]);
	m_pListHis->SetSubItemText(nRow, SFLH_CountDay, strTmp.Format(L"%d", hsf.countDay));
	m_pListHis->SetSubItemText(nRow, SFLH_JudgeType, m_JudgeTypeMap[hsf.type]);

}

void CDlgComboStockFilter::HisListDeleteItem(int nRow)
{
	if (nRow >= m_pListHis->GetItemCount() || nRow < 0)
		return;
	m_pListHis->DeleteItem(nRow);
	SStringW strTmp;
	for (int i = nRow; i < m_pListHis->GetItemCount(); ++i)
		m_pListHis->SetSubItemText(i, SFLH_ID, strTmp.Format(L"%d", i + 1));
}

bool CDlgComboStockFilter::GetHisListItem(int nRow, HisStockFilter & hsf)
{
	if (nRow >= m_pListHis->GetItemCount() || nRow < 0)
		return false;
	SStringW strTmp = m_pListHis->GetSubItemText(nRow, SFLH_Index1);
	hsf.sf.index1 = m_ReverseIndexMap[strTmp];
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_Period1);
	hsf.sf.period1 = m_ReversePeriodMap[strTmp];
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_Condition);
	hsf.sf.condition = m_ReverseConditionMap[strTmp];
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_Index2);
	hsf.sf.index2 = m_ReverseIndexMap[strTmp];
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_Period2OrNum);
	if (SFI_Num == hsf.sf.index2)
		hsf.sf.num = _wtof(strTmp);
	else
		hsf.sf.period2 = m_ReversePeriodMap[strTmp];
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_CountDay);
	hsf.countDay = _wtof(strTmp);
	strTmp = m_pListHis->GetSubItemText(nRow, SFLH_JudgeType);
	hsf.type = m_ReverseJudgeTypeMap[strTmp];
	return true;
}

void CDlgComboStockFilter::CalcHisStockFilter()
{
	if (m_pDlgBackTesting == nullptr)
	{
		m_pDlgBackTesting = new CDlgBackTesting(m_hWnd, m_sfSet, m_stockInfo);
		m_pDlgBackTesting->Create(NULL);
		m_pDlgBackTesting->CenterWindow(m_hWnd);
		m_pDlgBackTesting->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
		m_pDlgBackTesting->ShowWindow(SW_HIDE);
	}
	if (m_hisSfSet.empty())
		::SendMessage(m_hWnd, WM_FILTER_MSG, CFMsg_FinishHis, NULL);
	else
		m_pDlgBackTesting->CalcHisStockFilter(m_hisSfSet);
}


LRESULT CDlgComboStockFilter::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nMsg = (int)wp;
	switch (nMsg)
	{
	case CFMsg_FinishHis:
	{
		m_bHisChange = FALSE;
		m_bHisCalced = TRUE;
		BOOL bUsed = TRUE;
		m_hisFitStockSet = m_pDlgBackTesting->GetHisFilterPassStock();
		SendMsg(m_uParThreadID, WW_HisFilterEndCalc,
			(char*)&bUsed, sizeof(bUsed));
		m_pCheckHisUse->EnableWindow(TRUE, TRUE);
		m_pBtnSaveHis->EnableWindow(TRUE, TRUE);

	}
	break;
	default:
		break;
	}
	return 0;
}

bool CDlgComboStockFilter::OnCbxFuncChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	if (SFF_Add == nSel)
		OnCbxFuncAdd();
	if (SFF_Change == nSel)
		OnCbxFuncChange();
	if (SFF_Delete == nSel)
		OnCbxFuncDelete();
	return true;
}

bool CDlgComboStockFilter::OnCbxIndex1Change(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	if (SFI_ChgPct == nSel)
	{
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
		m_pTextPeriod1->SetVisible(FALSE, TRUE);
		m_pCbxPeriod1->SetVisible(FALSE, TRUE);
		m_pTextPeriod1->SetAttribute(L"pos", L"5,[0,5,[0");
		m_pCbxPeriod1->SetAttribute(L"pos", L"5,[0,5,[0");
		m_pTextPeriod2->SetVisible(FALSE, TRUE);
		m_pCbxPeriod2->SetVisible(FALSE, TRUE);
		int nNumPos = SFI_Num;
		for (auto &it : m_Index2PosMap)
		{
			if (it.second == SFI_Num)
			{
				nNumPos = it.first;
				break;
			}
		}
		m_pCbxIndex2->SetCurSel(nNumPos);
		m_pCbxIndex2->EnableWindow(FALSE, TRUE);
	}
	else if (m_bOnlyBool[m_Index1PosMap[nSel]])
	{
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
		m_pTextPeriod1->SetVisible(TRUE, TRUE);
		m_pCbxPeriod1->SetVisible(TRUE, TRUE);
		m_pTextPeriod1->SetAttribute(L"pos", L"5,[5");
		m_pCbxPeriod1->SetAttribute(L"pos", L"5,[5,-5,@20");
		m_pTextPeriod2->SetVisible(FALSE, TRUE);
		m_pCbxPeriod2->SetVisible(FALSE, TRUE);
		int nNumPos = SFI_Num;
		for (auto &it : m_Index2PosMap)
		{
			if (it.second == SFI_Num)
			{
				nNumPos = it.first;
				break;
			}
		}

		m_pCbxIndex2->SetCurSel(nNumPos);
		m_pCbxIndex2->EnableWindow(FALSE, TRUE);
		m_pCbxCondition->SetCurSel(SFC_Equal);
		m_pCbxCondition->EnableWindow(FALSE, TRUE);

	}
	else
	{
		m_pTextPeriod1->SetVisible(TRUE, TRUE);
		m_pCbxPeriod1->SetVisible(TRUE, TRUE);
		m_pTextPeriod1->SetAttribute(L"pos", L"5,[5");
		m_pCbxPeriod1->SetAttribute(L"pos", L"5,[5,-5,@20");
		m_pCbxIndex2->EnableWindow(TRUE, TRUE);
		m_pCbxCondition->EnableWindow(TRUE, TRUE);
	}

	if (m_bOnlyInDay[m_Index1PosMap[nSel]])
	{
		m_pCbxPeriod1->SetCurSel(SFP_D1);
		m_pCbxPeriod1->EnableWindow(FALSE, TRUE);
	}
	else
		m_pCbxPeriod1->EnableWindow(TRUE, TRUE);



	int nIndex2Sel = m_pCbxIndex2->GetCurSel();
	if (m_Index2PosMap[nIndex2Sel] == SFI_Num)
	{
		SStringW strNum = L"��ֵ";
		if (m_NumUint.count(m_Index1PosMap[nSel]))
			strNum += m_NumUint[m_Index1PosMap[nSel]];
		m_pTextNum->SetWindowTextW(strNum);
	}
	return true;

}

bool CDlgComboStockFilter::OnCbxIndex2Change(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nIndex1Sel = m_pCbxIndex1->GetCurSel();
	if (SFI_Num == m_Index2PosMap[nSel])
	{
		SStringW strNum = L"��ֵ";
		if (m_NumUint.count(nIndex1Sel))
			strNum += m_NumUint[nIndex1Sel];
		m_pTextNum->SetWindowTextW(strNum);
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
		m_pTextPeriod2->SetVisible(FALSE, TRUE);
		m_pCbxPeriod2->SetVisible(FALSE, TRUE);
	}
	else
	{
		m_pTextNum->SetVisible(FALSE, TRUE);
		m_pEditNum->SetVisible(FALSE, TRUE);
		m_pTextPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxPeriod2->SetVisible(TRUE, TRUE);
	}
	return true;
}

bool CDlgComboStockFilter::OnCbxIDChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = m_pCbxFunc->GetCurSel();
	if (SFF_Change == nSel)
	{
		int nSel = pEvt->nCurSel;
		StockFilter sf;
		GetListItem(nSel, sf);
		int nIndex1ID = sf.index1;
		int nIndex2ID = sf.index2;
		for (auto &it : m_Index1PosMap)
		{
			if (sf.index1 == it.second)
			{
				nIndex1ID = it.first;
				break;
			}
		}
		for (auto &it : m_Index2PosMap)
		{
			if (sf.index2 == it.second)
			{
				nIndex2ID = it.first;
				break;
			}
		}

		m_pCbxIndex1->SetCurSel(nIndex1ID);
		m_pCbxIndex2->SetCurSel(nIndex2ID);
		m_pCbxPeriod1->SetCurSel(sf.period1);
		m_pCbxCondition->SetCurSel(sf.condition);
		if (sf.index2 == SFI_Num)
		{
			SStringW strNum = L"��ֵ";
			if (m_NumUint.count(sf.index1))
				strNum += m_NumUint[sf.index1];
			m_pTextNum->SetWindowTextW(strNum);
			m_pTextNum->SetVisible(TRUE, TRUE);
			m_pEditNum->SetVisible(TRUE, TRUE);
			m_pTextPeriod2->SetVisible(FALSE, TRUE);
			m_pCbxPeriod2->SetVisible(FALSE, TRUE);
			m_pEditNum->SetWindowTextW(strNum.Format(L"%.02f", sf.num));
		}
		else
		{
			m_pTextNum->SetVisible(FALSE, TRUE);
			m_pEditNum->SetVisible(FALSE, TRUE);
			m_pTextPeriod2->SetVisible(TRUE, TRUE);
			m_pCbxPeriod2->SetVisible(TRUE, TRUE);
			m_pCbxPeriod2->SetCurSel(sf.period2);
		}

	}

	return true;
}

void CDlgComboStockFilter::OnCbxFuncAdd()
{
	m_pTextID->SetAttribute(L"pos", L"5,[0,5,[0");
	m_pCbxID->SetAttribute(L"pos", L"5,[0,5,[0");
	m_pTextID->SetVisible(FALSE);
	m_pCbxID->SetVisible(FALSE);
	m_pTextIndex1->SetVisible(TRUE);
	m_pCbxIndex1->SetVisible(TRUE);
	m_pTextPeriod1->SetVisible(TRUE);
	m_pCbxPeriod1->SetVisible(TRUE);
	m_pTextCondition->SetVisible(TRUE);
	m_pCbxCondition->SetVisible(TRUE);
	m_pTextIndex2->SetVisible(TRUE);
	m_pCbxIndex2->SetVisible(TRUE);
	if (m_pCbxIndex2->GetCurSel() == SFI_Num - 1)
	{
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pTextPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxPeriod2->SetVisible(TRUE, TRUE);
	}

}

void CDlgComboStockFilter::OnCbxFuncChange()
{
	m_pTextID->SetAttribute(L"pos", L"5,[5");
	m_pCbxID->SetAttribute(L"pos", L"5,[5,-5,@20");

	m_pTextID->SetVisible(TRUE);
	m_pCbxID->SetVisible(TRUE);
	m_pTextIndex1->SetVisible(TRUE);
	m_pCbxIndex1->SetVisible(TRUE);
	m_pTextPeriod1->SetVisible(TRUE);
	m_pCbxPeriod1->SetVisible(TRUE);
	m_pTextCondition->SetVisible(TRUE);
	m_pCbxCondition->SetVisible(TRUE);
	m_pTextIndex2->SetVisible(TRUE);
	m_pCbxIndex2->SetVisible(TRUE);
	if (m_pCbxIndex2->GetCurSel() == SFI_Num - 1)
	{
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pTextPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxPeriod2->SetVisible(TRUE, TRUE);
	}

}

void CDlgComboStockFilter::OnCbxFuncDelete()
{
	m_pTextID->SetAttribute(L"pos", L"5,[5");
	m_pCbxID->SetAttribute(L"pos", L"5,[5,-5,@20");
	m_pTextID->SetVisible(TRUE);
	m_pCbxID->SetVisible(TRUE);
	m_pTextIndex1->SetVisible(FALSE);
	m_pCbxIndex1->SetVisible(FALSE);
	m_pTextPeriod1->SetVisible(FALSE);
	m_pCbxPeriod1->SetVisible(FALSE);
	m_pTextCondition->SetVisible(FALSE);
	m_pCbxCondition->SetVisible(FALSE);
	m_pTextIndex2->SetVisible(FALSE);
	m_pCbxIndex2->SetVisible(FALSE);
	m_pTextPeriod2->SetVisible(FALSE);
	m_pCbxPeriod2->SetVisible(FALSE);
	m_pTextNum->SetVisible(FALSE);
	m_pEditNum->SetVisible(FALSE);

}

void CDlgComboStockFilter::AddConditon()
{
	StockFilter sf = { 0 };
	sf.index1 = m_Index1PosMap[m_pCbxIndex1->GetCurSel()];
	if (SFI_ChgPct == sf.index1)
		sf.period1 = SFP_Null;
	else
		sf.period1 = m_pCbxPeriod1->GetCurSel();
	sf.condition = m_pCbxCondition->GetCurSel();
	sf.index2 = m_Index2PosMap[m_pCbxIndex2->GetCurSel()];
	if (sf.index2 != SFI_Num)
		sf.period2 = m_pCbxPeriod2->GetCurSel();
	else
		sf.num = _wtof(m_pEditNum->GetWindowTextW());
	if (m_sfSet.count(sf))
	{
		SMessageBox(m_hWnd, L"�Ѱ�����ͬ���������ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	m_sfSet.insert(sf);
	int nIDCount = m_pList->GetItemCount();
	ListAddItem(sf);
	SStringW strTmp;
	m_pCbxID->InsertItem(nIDCount,
		strTmp.Format(L"����%d", nIDCount + 1), NULL, 0);
	SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);
	if (m_pCheckUse->IsChecked())
	{
		BOOL bUse = TRUE;
		SendMsg(m_uParThreadID, WW_ChangeStockFilter,
			(char*)&bUse, sizeof(bUse));
	}
}

void CDlgComboStockFilter::ChangeCondition()
{

	StockFilter sf = { 0 };
	sf.index1 = m_Index1PosMap[m_pCbxIndex1->GetCurSel()];
	if (SFI_ChgPct == sf.index1)
		sf.period1 = SFP_Null;
	else
		sf.period1 = m_pCbxPeriod1->GetCurSel();
	sf.condition = m_pCbxCondition->GetCurSel();
	sf.index2 = m_Index2PosMap[m_pCbxIndex2->GetCurSel()];
	if (sf.index2 != SFI_Num)
		sf.period2 = m_pCbxPeriod2->GetCurSel();
	else
		sf.num = _wtof(m_pEditNum->GetWindowTextW());
	if (m_sfSet.count(sf))
	{
		SMessageBox(m_hWnd, L"�Ѱ�����ͬ�������޸�ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	m_sfSet.insert(sf);
	int nID = m_pCbxID->GetCurSel();
	StockFilter oriSf = { 0 };
	GetListItem(nID, oriSf);
	m_sfSet.erase(oriSf);
	ListChangeItem(nID, sf);
	SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);
	if (m_pCheckUse->IsChecked())
	{
		BOOL bUse = TRUE;
		SendMsg(m_uParThreadID, WW_ChangeStockFilter,
			(char*)&bUse, sizeof(bUse));
	}
}

void CDlgComboStockFilter::DeleteCondition()
{
	if (m_pList->GetItemCount() == 0)
	{
		SMessageBox(m_hWnd, L"��ǰ����������ɾ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	int nID = m_pCbxID->GetCurSel();
	StockFilter oriSf = { 0 };
	GetListItem(nID, oriSf);
	m_sfSet.erase(oriSf);
	ListDeleteItem(nID);
	int nItemCount = m_pCbxID->GetCount();
	m_pCbxID->DeleteString(nItemCount - 1);
	if (nID >= m_pCbxID->GetCount())
		m_pCbxID->SetCurSel(m_pCbxID->GetCount() - 1);
	if (m_pCbxID->GetCount() == 0)
		m_pCbxID->SetWindowTextW(L"");
	m_pList->RequestRelayout();
	SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);
	if (m_pCheckUse->IsChecked())
	{

		if (m_pList->GetItemCount() > 0)
		{
			BOOL bUse = TRUE;
			SendMsg(m_uParThreadID, WW_ChangeStockFilter,
				(char*)&bUse, sizeof(bUse));
		}
		else
		{
			m_pCheckUse->SetCheck(FALSE);
			BOOL bUse = FALSE;
			SendMsg(m_uParThreadID, WW_ChangeStockFilter,
				(char*)&bUse, sizeof(bUse));
			SMessageBox(m_hWnd, L"��ǰѡ�������������Զ��ر�!", L"��ʾ", MB_OK);

		}

	}


}

bool CDlgComboStockFilter::OnCbxHisFuncChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	if (SFF_Add == nSel)
		OnCbxHisFuncAdd();
	if (SFF_Change == nSel)
		OnCbxHisFuncChange();
	if (SFF_Delete == nSel)
		OnCbxHisFuncDelete();
	return true;
}

bool CDlgComboStockFilter::OnCbxHisIndex1Change(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	if (SFI_ChgPct == nSel)
	{
		m_pTextHisNum->SetVisible(TRUE, TRUE);
		m_pEditHisNum->SetVisible(TRUE, TRUE);
		m_pTextHisPeriod1->SetVisible(FALSE, TRUE);
		m_pCbxHisPeriod1->SetVisible(FALSE, TRUE);
		m_pTextHisPeriod1->SetAttribute(L"pos", L"5,[0,5,[0");
		m_pCbxHisPeriod1->SetAttribute(L"pos", L"5,[0,5,[0");
		m_pTextHisPeriod2->SetVisible(FALSE, TRUE);
		m_pCbxHisPeriod2->SetVisible(FALSE, TRUE);
		int nNumPos = SFI_Num;
		for (auto &it : m_hisIndex2PosMap)
		{
			if (it.second == SFI_Num)
			{
				nNumPos = it.first;
				break;
			}
		}
		m_pCbxHisIndex2->SetCurSel(nNumPos);
		m_pCbxHisIndex2->EnableWindow(FALSE, TRUE);
	}
	else
	{
		m_pTextHisPeriod1->SetVisible(TRUE, TRUE);
		m_pCbxHisPeriod1->SetVisible(TRUE, TRUE);
		m_pTextHisPeriod1->SetAttribute(L"pos", L"5,[5");
		m_pCbxHisPeriod1->SetAttribute(L"pos", L"5,[5,-5,@20");
		m_pCbxHisIndex2->EnableWindow(TRUE, TRUE);
	}
	int nIndex2Sel = m_pCbxHisIndex2->GetCurSel();
	if (m_hisIndex2PosMap[nIndex2Sel] == SFI_Num)
	{
		SStringW strNum = L"��ֵ";
		if (m_NumUint.count(m_hisIndex1PosMap[nSel]))
			strNum += m_NumUint[m_hisIndex1PosMap[nSel]];
		m_pTextHisNum->SetWindowTextW(strNum);
	}
	return true;
}

bool CDlgComboStockFilter::OnCbxHisIndex2Change(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nIndex1Sel = m_pCbxHisIndex1->GetCurSel();
	if (SFI_Num == m_hisIndex2PosMap[nSel])
	{
		SStringW strNum = L"��ֵ";
		if (m_NumUint.count(nIndex1Sel))
			strNum += m_NumUint[nIndex1Sel];
		m_pTextHisNum->SetWindowTextW(strNum);
		m_pTextHisNum->SetVisible(TRUE, TRUE);
		m_pEditHisNum->SetVisible(TRUE, TRUE);
		m_pTextHisPeriod2->SetVisible(FALSE, TRUE);
		m_pCbxHisPeriod2->SetVisible(FALSE, TRUE);
	}
	else
	{
		m_pTextHisNum->SetVisible(FALSE, TRUE);
		m_pEditHisNum->SetVisible(FALSE, TRUE);
		m_pTextHisPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxHisPeriod2->SetVisible(TRUE, TRUE);
	}
	return true;
}

bool CDlgComboStockFilter::OnCbxHisIDChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = m_pCbxHisFunc->GetCurSel();
	if (SFF_Change == nSel)
	{
		int nSel = pEvt->nCurSel;
		HisStockFilter hsf;
		GetHisListItem(nSel, hsf);
		int nIndex1ID = hsf.sf.index1;
		int nIndex2ID = hsf.sf.index2;
		for (auto &it : m_hisIndex1PosMap)
		{
			if (hsf.sf.index1 == it.second)
			{
				nIndex1ID = it.first;
				break;
			}
		}
		for (auto &it : m_hisIndex2PosMap)
		{
			if (hsf.sf.index2 == it.second)
			{
				nIndex2ID = it.first;
				break;
			}
		}

		m_pCbxHisIndex1->SetCurSel(nIndex1ID);
		m_pCbxHisIndex2->SetCurSel(nIndex2ID);
		m_pCbxHisPeriod1->SetCurSel(hsf.sf.period1);
		m_pCbxHisCondition->SetCurSel(hsf.sf.condition);
		if (hsf.sf.index2 == SFI_Num)
		{
			SStringW strNum = L"��ֵ";
			if (m_NumUint.count(hsf.sf.index1))
				strNum += m_NumUint[hsf.sf.index1];
			m_pTextHisNum->SetWindowTextW(strNum);
			m_pTextHisNum->SetVisible(TRUE, TRUE);
			m_pEditHisNum->SetVisible(TRUE, TRUE);
			m_pTextHisPeriod2->SetVisible(FALSE, TRUE);
			m_pCbxHisPeriod2->SetVisible(FALSE, TRUE);
			m_pEditHisNum->SetWindowTextW(strNum.Format(L"%.02f", hsf.sf.num));
		}
		else
		{
			m_pTextHisNum->SetVisible(FALSE, TRUE);
			m_pEditHisNum->SetVisible(FALSE, TRUE);
			m_pTextHisPeriod2->SetVisible(TRUE, TRUE);
			m_pCbxHisPeriod2->SetVisible(TRUE, TRUE);
			m_pCbxHisPeriod2->SetCurSel(hsf.sf.period2);
		}

	}

	return true;
}

void CDlgComboStockFilter::OnCbxHisFuncAdd()
{
	m_pTextHisID->SetAttribute(L"pos", L"5,[0,5,[0");
	m_pCbxHisID->SetAttribute(L"pos", L"5,[0,5,[0");
	m_pTextHisID->SetVisible(FALSE);
	m_pCbxHisID->SetVisible(FALSE);
	m_pTextHisIndex1->SetVisible(TRUE);
	m_pCbxHisIndex1->SetVisible(TRUE);
	m_pTextHisPeriod1->SetVisible(TRUE);
	m_pCbxHisPeriod1->SetVisible(TRUE);
	m_pTextHisCondition->SetVisible(TRUE);
	m_pCbxHisCondition->SetVisible(TRUE);
	m_pTextHisIndex2->SetVisible(TRUE);
	m_pCbxHisIndex2->SetVisible(TRUE);
	m_pTextHisCntDayFront->SetVisible(TRUE);
	m_pTextHisCntDayBack->SetVisible(TRUE);
	m_pEditHisCntDay->SetVisible(TRUE);
	m_pRadioExist->SetVisible(TRUE);
	m_pRadioForall->SetVisible(TRUE);

	if (m_pCbxHisIndex2->GetCurSel() == SFI_Num - 1)
	{
		m_pTextHisNum->SetVisible(TRUE, TRUE);
		m_pEditHisNum->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pTextHisPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxHisPeriod2->SetVisible(TRUE, TRUE);
	}
}

void CDlgComboStockFilter::OnCbxHisFuncChange()
{
	m_pTextHisID->SetAttribute(L"pos", L"5,[5");
	m_pCbxHisID->SetAttribute(L"pos", L"5,[5,-5,@20");

	m_pTextHisID->SetVisible(TRUE);
	m_pCbxHisID->SetVisible(TRUE);
	m_pTextHisIndex1->SetVisible(TRUE);
	m_pCbxHisIndex1->SetVisible(TRUE);
	m_pTextHisPeriod1->SetVisible(TRUE);
	m_pCbxHisPeriod1->SetVisible(TRUE);
	m_pTextHisCondition->SetVisible(TRUE);
	m_pCbxHisCondition->SetVisible(TRUE);
	m_pTextHisIndex2->SetVisible(TRUE);
	m_pCbxHisIndex2->SetVisible(TRUE);
	m_pTextHisCntDayFront->SetVisible(TRUE);
	m_pTextHisCntDayBack->SetVisible(TRUE);
	m_pEditHisCntDay->SetVisible(TRUE);
	m_pRadioExist->SetVisible(TRUE);
	m_pRadioForall->SetVisible(TRUE);

	if (m_pCbxHisIndex2->GetCurSel() == SFI_Num - 1)
	{
		m_pTextNum->SetVisible(TRUE, TRUE);
		m_pEditNum->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pTextPeriod2->SetVisible(TRUE, TRUE);
		m_pCbxHisPeriod2->SetVisible(TRUE, TRUE);
	}
}

void CDlgComboStockFilter::OnCbxHisFuncDelete()
{
	m_pTextHisID->SetAttribute(L"pos", L"5,[5");
	m_pCbxHisID->SetAttribute(L"pos", L"5,[5,-5,@20");

	m_pTextHisID->SetVisible(TRUE);
	m_pCbxHisID->SetVisible(TRUE);
	m_pTextHisIndex1->SetVisible(FALSE);
	m_pCbxHisIndex1->SetVisible(FALSE);
	m_pTextHisPeriod1->SetVisible(FALSE);
	m_pCbxHisPeriod1->SetVisible(FALSE);
	m_pTextHisCondition->SetVisible(FALSE);
	m_pCbxHisCondition->SetVisible(FALSE);
	m_pTextHisIndex2->SetVisible(FALSE);
	m_pCbxHisIndex2->SetVisible(FALSE);
	m_pTextHisCntDayFront->SetVisible(FALSE);
	m_pTextHisCntDayBack->SetVisible(FALSE);
	m_pEditHisCntDay->SetVisible(FALSE);
	m_pRadioExist->SetVisible(FALSE);
	m_pRadioForall->SetVisible(FALSE);

}

void CDlgComboStockFilter::AddHisConditon()
{
	HisStockFilter hsf = { 0 };
	hsf.sf.index1 = m_hisIndex1PosMap[m_pCbxHisIndex1->GetCurSel()];
	if (SFI_ChgPct == hsf.sf.index1)
		hsf.sf.period1 = SFP_Null;
	else
		hsf.sf.period1 = m_pCbxHisPeriod1->GetCurSel();
	hsf.sf.condition = m_pCbxHisCondition->GetCurSel();
	hsf.sf.index2 = m_hisIndex2PosMap[m_pCbxHisIndex2->GetCurSel()];
	if (hsf.sf.index2 != SFI_Num)
		hsf.sf.period2 = m_pCbxHisPeriod2->GetCurSel();
	else
		hsf.sf.num = _wtof(m_pEditHisNum->GetWindowTextW());
	hsf.countDay = _wtof(m_pEditHisCntDay->GetWindowTextW());
	hsf.type = m_pRadioExist->IsChecked() ? eJT_Exist : eJT_Forall;

	if (m_hisTmpSfSet.count(hsf))
	{
		SMessageBox(m_hWnd, L"�Ѱ�����ͬ���������ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	m_bHisChange = TRUE;
	m_bHisCalced = FALSE;
	m_hisTmpSfSet.insert(hsf);
	int nIDCount = m_pListHis->GetItemCount();
	HisListAddItem(hsf);
	SStringW strTmp;
	m_pCbxHisID->InsertItem(nIDCount,
		strTmp.Format(L"����%d", nIDCount + 1), NULL, 0);
}

void CDlgComboStockFilter::ChangeHisCondition()
{

	HisStockFilter hsf = { 0 };
	hsf.sf.index1 = m_hisIndex1PosMap[m_pCbxHisIndex1->GetCurSel()];
	if (SFI_ChgPct == hsf.sf.index1)
		hsf.sf.period1 = SFP_Null;
	else
		hsf.sf.period1 = m_pCbxHisPeriod1->GetCurSel();
	hsf.sf.condition = m_pCbxHisCondition->GetCurSel();
	hsf.sf.index2 = m_hisIndex2PosMap[m_pCbxHisIndex2->GetCurSel()];
	if (hsf.sf.index2 != SFI_Num)
		hsf.sf.period2 = m_pCbxHisPeriod2->GetCurSel();
	else
		hsf.sf.num = _wtof(m_pEditHisNum->GetWindowTextW());

	hsf.countDay = _wtof(m_pEditHisCntDay->GetWindowTextW());
	hsf.type = m_pRadioExist->IsChecked() ? eJT_Exist : eJT_Forall;

	if (m_hisTmpSfSet.count(hsf))
	{
		SMessageBox(m_hWnd, L"�Ѱ�����ͬ�������޸�ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	m_bHisChange = TRUE;
	m_bHisCalced = FALSE;

	m_hisTmpSfSet.insert(hsf);
	int nID = m_pCbxHisID->GetCurSel();
	HisStockFilter oriSf = { 0 };
	GetHisListItem(nID, oriSf);
	m_hisTmpSfSet.erase(oriSf);
	HisListChangeItem(nID, hsf);
}

void CDlgComboStockFilter::DeleteHisCondition()
{
	if (m_pListHis->GetItemCount() == 0)
	{
		SMessageBox(m_hWnd, L"��ǰ����������ɾ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	m_bHisChange = TRUE;
	m_bHisCalced = FALSE;

	int nID = m_pCbxHisID->GetCurSel();
	HisStockFilter oriSf = { 0 };
	GetHisListItem(nID, oriSf);
	m_hisTmpSfSet.erase(oriSf);
	HisListDeleteItem(nID);
	int nItemCount = m_pCbxHisID->GetCount();
	m_pCbxHisID->DeleteString(nItemCount - 1);
	if (nID >= m_pCbxHisID->GetCount())
		m_pCbxHisID->SetCurSel(m_pCbxID->GetCount() - 1);
	if (m_pCbxHisID->GetCount() == 0)
		m_pCbxHisID->SetWindowTextW(L"");
	m_pListHis->RequestRelayout();

}


void CDlgComboStockFilter::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}


