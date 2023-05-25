#include "stdafx.h"
#include "DlgComboStockFilter.h"
#include "SColorListCtrlEx.h"

CDlgComboStockFilter::CDlgComboStockFilter(UINT uParThreadID, RpsGroup Group)
	:SHostWnd(_T("LAYOUT:dlg_comboStockFilter"))
{
	m_uParThreadID = uParThreadID;
	m_Group = Group;
}


CDlgComboStockFilter::~CDlgComboStockFilter()
{
}

BOOL SOUI::CDlgComboStockFilter::OnInitDialog(EventArgs * e)
{
	m_pCheckUse = FindChildByID2<SCheckBox>(R.id.chk_use);

	m_pList = FindChildByID2<SColorListCtrlEx>(R.id.ls_filter); 
	SHeaderCtrlEx * pHead = (SHeaderCtrlEx *)m_pList->GetChild(GSW_FIRSTCHILD);
	pHead->SetNoMoveCol(1);
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
	InitStringMap();
	InitComboBox();
	return TRUE;
}

void SOUI::CDlgComboStockFilter::OnBtnClose()
{
	ShowWindow(SW_HIDE);
}

void SOUI::CDlgComboStockFilter::OnBtnOK()
{
	int nSel = m_pCbxFunc->GetCurSel();
	if (SFF_Add == nSel)
		AddConditon();
	else if (SFF_Change == nSel)
		ChangeCondition();
	else if (SFF_Delete == nSel)
		DeleteCondition();
}

void SOUI::CDlgComboStockFilter::InitList(bool bUse, vector<StockFilter> &sfVec)
{
	SStringW strTmp;
	for (int i = 0; i < sfVec.size(); ++i)
	{
		ListAddItem(sfVec[i]);
		m_pCbxID->InsertItem(i, strTmp.Format(L"条件%d", i + 1), NULL, 0);
		m_sfSet.insert(sfVec[i]);
	}
	m_pCheckUse->SetCheck(bUse);

}

void SOUI::CDlgComboStockFilter::InitComboBox()
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
		if (m_IndexMap.count(i))
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
		if (m_IndexMap.count(i))
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

void SOUI::CDlgComboStockFilter::OnCheckUse()
{
	BOOL bUsed = m_pCheckUse->IsChecked();
	if (bUsed)
	{
		if (m_pList->GetItemCount() == 0)
		{
			m_pCheckUse->SetCheck(FALSE);
			SMessageBox(m_hWnd, L"条件为空，请添加条件后重试！",
				L"错误", MB_ICONERROR | MB_OK);
		}
		else
		{
			SendMsg(m_uParThreadID, WW_ChangeStockFilter,
				(char*)&bUsed, sizeof(bUsed));
		}
	}
	else
		SendMsg(m_uParThreadID, WW_ChangeStockFilter,
		(char*)&bUsed, sizeof(bUsed));

}

void SOUI::CDlgComboStockFilter::StopFilter()
{
	m_pCheckUse->SetCheck(FALSE);
}

void SOUI::CDlgComboStockFilter::InitStringMap()
{
	m_FuncMap[SFF_Add] = L"新增";
	m_FuncMap[SFF_Change] = L"修改";
	m_FuncMap[SFF_Delete] = L"删除";

	m_ReverseFuncMap[L"新增"] = SFF_Add;
	m_ReverseFuncMap[L"修改"] = SFF_Change;
	m_ReverseFuncMap[L"删除"] = SFF_Delete;

	m_PeriodMap[SFP_Null] = L"-";
	m_PeriodMap[SFP_D1] = L"1日";
	m_PeriodMap[SFP_FS] = L"分时";
	m_PeriodMap[SFP_M1] = L"1分钟";
	m_PeriodMap[SFP_M5] = L"5分钟";
	m_PeriodMap[SFP_M15] = L"15分钟";
	m_PeriodMap[SFP_M30] = L"30分钟";
	m_PeriodMap[SFP_M60] = L"60分钟";

	m_ReversePeriodMap[L"-"] = SFP_Null;
	m_ReversePeriodMap[L"1日"] = SFP_D1;
	m_ReversePeriodMap[L"分时"] = SFP_FS;
	m_ReversePeriodMap[L"1分钟"] = SFP_M1;
	m_ReversePeriodMap[L"5分钟"] = SFP_M5;
	m_ReversePeriodMap[L"15分钟"] = SFP_M15;
	m_ReversePeriodMap[L"30分钟"] = SFP_M30;
	m_ReversePeriodMap[L"60分钟"] = SFP_M60;


	m_IndexMap[SFI_ChgPct] = L"涨跌幅";
	m_IndexMap[SFI_CloseRps520] = L"收盘价RPS520";
	m_IndexMap[SFI_CloseMacd520] = L"收盘价Macd520";
	m_IndexMap[SFI_ClosePoint520] = L"收盘价520分数";
	m_IndexMap[SFI_CloseRank520] = L"收盘价520排名";
	m_IndexMap[SFI_CloseRps2060] = L"收盘价RPS2060";
	m_IndexMap[SFI_CloseMacd2060] = L"收盘价Macd2060";
	m_IndexMap[SFI_ClosePoint2060] = L"收盘价2060分数";
	m_IndexMap[SFI_CloseRank2060] = L"收盘价2060排名";
	m_IndexMap[SFI_Num] = L"数值";
	m_IndexMap[SFI_ABSR] = L"主动量比";
	m_IndexMap[SFI_A2PBSR] = L"主动转被动量比";
	m_IndexMap[SFI_AABSR] = L"平均主动量比";
	m_IndexMap[SFI_POCR] = L"最多成交价比";
	m_IndexMap[SFI_Vol] = L"成交量(手)";
	m_IndexMap[SFI_Amount] = L"成交额(万元)";
	m_IndexMap[SFI_AmountRps520] = L"成交额RPS520";
	m_IndexMap[SFI_AmountMacd520] = L"成交额Macd520";
	m_IndexMap[SFI_AmountPoint520] = L"成交额520分数";
	m_IndexMap[SFI_AmountRank520] = L"成交额520排名";
	m_IndexMap[SFI_AmountRps2060] = L"成交额RPS2060";
	m_IndexMap[SFI_AmountMacd2060] = L"成交额Macd2060";
	m_IndexMap[SFI_AmountPoint2060] = L"成交额2060分数";
	m_IndexMap[SFI_AmountRank2060] = L"成交额2060排名";
	m_IndexMap[SFI_AmountPoint] = L"成交额截面分数";
	m_IndexMap[SFI_AmountRank] = L"成交额截面排名";

	m_IndexMap[SFI_ClosePoint520L1] = L"收盘价520分数(一级行业中)";
	m_IndexMap[SFI_CloseRank520L1] = L"收盘价520排名(一级行业中)";
	m_IndexMap[SFI_ClosePoint2060L1] = L"收盘价2060分数(一级行业中)";
	m_IndexMap[SFI_CloseRank2060L1] = L"收盘价2060排名(一级行业中)";
	m_IndexMap[SFI_AmountPoint520L1] = L"成交额520分数(一级行业中)";
	m_IndexMap[SFI_AmountRank520L1] = L"成交额520排名(一级行业中)";
	m_IndexMap[SFI_AmountPoint2060L1] = L"成交额2060分数(一级行业中)";
	m_IndexMap[SFI_AmountRank2060L1] = L"成交额2060排名(一级行业中)";
	m_IndexMap[SFI_AmountPointL1] = L"成交额截面分数(一级行业中)";
	m_IndexMap[SFI_AmountRankL1] = L"成交额截面排名(一级行业中)";

	m_IndexMap[SFI_ClosePoint520L2] = L"收盘价520分数(二级行业中)";
	m_IndexMap[SFI_CloseRank520L2] = L"收盘价520排名(二级行业中)";
	m_IndexMap[SFI_ClosePoint2060L2] = L"收盘价2060分数(二级行业中)";
	m_IndexMap[SFI_CloseRank2060L2] = L"收盘价2060排名(二级行业中)";
	m_IndexMap[SFI_AmountPoint520L2] = L"成交额520分数(二级行业中)";
	m_IndexMap[SFI_AmountRank520L2] = L"成交额520排名(二级行业中)";
	m_IndexMap[SFI_AmountPoint2060L2] = L"成交额2060分数(二级行业中)";
	m_IndexMap[SFI_AmountRank2060L2] = L"成交额2060排名(二级行业中)";
	m_IndexMap[SFI_AmountPointL2] = L"成交额截面分数(二级行业中)";
	m_IndexMap[SFI_AmountRankL2] = L"成交额截面排名(二级行业中)";

	m_IndexMap[SFI_ABV] = L"主动买入量";
	m_IndexMap[SFI_ASV] = L"主动卖出量";
	m_IndexMap[SFI_ABO] = L"主动买入单数";
	m_IndexMap[SFI_ASO] = L"主动卖出单数";
	m_IndexMap[SFI_PBO] = L"被动买入单数";
	m_IndexMap[SFI_PSO] = L"被动卖出单数";

	m_IndexMap[SFI_Open] = L"开盘价";
	m_IndexMap[SFI_High] = L"最高价";
	m_IndexMap[SFI_Low] = L"最低价";
	m_IndexMap[SFI_Close] = L"收盘价";


	m_IndexMap[SFI_CAVol] = L"集合竞价成交量";
	m_IndexMap[SFI_CAVolPoint] = L"集合竞价成交量分数";
	m_IndexMap[SFI_CAVolRank] = L"集合竞价成交量排名";
	m_IndexMap[SFI_CAVolPointL1] = L"集合竞价成交量分数(一级行业中)";
	m_IndexMap[SFI_CAVolRankL1] = L"集合竞价成交量排名(一级行业中)";
	m_IndexMap[SFI_CAVolPointL2] = L"集合竞价成交量分数(二级行业中)";
	m_IndexMap[SFI_CAVolRankL2] = L"集合竞价成交量排名(二级行业中)";

	m_IndexMap[SFI_CAAmo] = L"集合竞价成交额";
	m_IndexMap[SFI_CAAmoPoint] = L"集合竞价成交额分数";
	m_IndexMap[SFI_CAAmoRank] = L"集合竞价成交额排名";
	m_IndexMap[SFI_CAAmoPointL1] = L"集合竞价成交额分数(一级行业中)";
	m_IndexMap[SFI_CAAmoRankL1] = L"集合竞价成交额排名(一级行业中)";
	m_IndexMap[SFI_CAAmoPointL2] = L"集合竞价成交额分数(二级行业中)";
	m_IndexMap[SFI_CAAmoRankL2] = L"集合竞价成交额排名(二级行业中)";


	m_IndexMap[SFI_L1IndyCloseRps520] = L"所在一级行业的收盘价RPS520";
	m_IndexMap[SFI_L1IndyCloseMacd520] = L"所在一级行业的收盘价Macd520";
	m_IndexMap[SFI_L1IndyClosePoint520] = L"所在一级行业的收盘价520分数";
	m_IndexMap[SFI_L1IndyCloseRank520] = L"所在一级行业的收盘价520排名";
	m_IndexMap[SFI_L1IndyCloseRps2060] = L"所在一级行业的收盘价RPS2060";
	m_IndexMap[SFI_L1IndyCloseMacd2060] = L"所在一级行业的收盘价Macd2060";
	m_IndexMap[SFI_L1IndyClosePoint2060] = L"所在一级行业的收盘价2060分数";
	m_IndexMap[SFI_L1IndyCloseRank2060] = L"所在一级行业的收盘价2060排名";
	m_IndexMap[SFI_L1IndyAmountRps520] = L"所在一级行业的成交额RPS520";
	m_IndexMap[SFI_L1IndyAmountMacd520] = L"所在一级行业的成交额Macd520";
	m_IndexMap[SFI_L1IndyAmountPoint520] = L"所在一级行业的成交额520分数";
	m_IndexMap[SFI_L1IndyAmountRank520] = L"所在一级行业的成交额520排名";
	m_IndexMap[SFI_L1IndyAmountRps2060] = L"所在一级行业的成交额RPS2060";
	m_IndexMap[SFI_L1IndyAmountMacd2060] = L"所在一级行业的成交额Macd2060";
	m_IndexMap[SFI_L1IndyAmountPoint2060] = L"所在一级行业的成交额2060分数";
	m_IndexMap[SFI_L1IndyAmountRank2060] = L"所在一级行业的成交额2060排名";
	m_IndexMap[SFI_L1IndyAmountPoint] = L"所在一级行业的成交额截面分数";
	m_IndexMap[SFI_L1IndyAmountRank] = L"所在一级行业的成交额截面排名";

	m_IndexMap[SFI_L2IndyCloseRps520] = L"所在二级行业的收盘价RPS520";
	m_IndexMap[SFI_L2IndyCloseMacd520] = L"所在二级行业的收盘价Macd520";
	m_IndexMap[SFI_L2IndyClosePoint520] = L"所在二级行业的收盘价520分数";
	m_IndexMap[SFI_L2IndyCloseRank520] = L"所在二级行业的收盘价520排名";
	m_IndexMap[SFI_L2IndyCloseRps2060] = L"所在二级行业的收盘价RPS2060";
	m_IndexMap[SFI_L2IndyCloseMacd2060] = L"所在二级行业的收盘价Macd2060";
	m_IndexMap[SFI_L2IndyClosePoint2060] = L"所在二级行业的收盘价2060分数";
	m_IndexMap[SFI_L2IndyCloseRank2060] = L"所在二级行业的收盘价2060排名";
	m_IndexMap[SFI_L2IndyAmountRps520] = L"所在二级行业的成交额RPS520";
	m_IndexMap[SFI_L2IndyAmountMacd520] = L"所在二级行业的成交额Macd520";
	m_IndexMap[SFI_L2IndyAmountPoint520] = L"所在二级行业的成交额520分数";
	m_IndexMap[SFI_L2IndyAmountRank520] = L"所在二级行业的成交额520排名";
	m_IndexMap[SFI_L2IndyAmountRps2060] = L"所在二级行业的成交额RPS2060";
	m_IndexMap[SFI_L2IndyAmountMacd2060] = L"所在二级行业的成交额Macd2060";
	m_IndexMap[SFI_L2IndyAmountPoint2060] = L"所在二级行业的成交额2060分数";
	m_IndexMap[SFI_L2IndyAmountRank2060] = L"所在二级行业的成交额2060排名";
	m_IndexMap[SFI_L2IndyAmountPoint] = L"所在二级行业的成交额截面分数";
	m_IndexMap[SFI_L2IndyAmountRank] = L"所在二级行业的成交额截面排名";

	m_ReverseIndexMap[L"涨跌幅"] = SFI_ChgPct;
	m_ReverseIndexMap[L"收盘价RPS520"] = SFI_CloseRps520;
	m_ReverseIndexMap[L"收盘价Macd520"] = SFI_CloseMacd520;
	m_ReverseIndexMap[L"收盘价520分数"] = SFI_ClosePoint520;
	m_ReverseIndexMap[L"收盘价520排名"] = SFI_CloseRank520;
	m_ReverseIndexMap[L"收盘价RPS2060"] = SFI_CloseRps2060;
	m_ReverseIndexMap[L"收盘价Macd2060"] = SFI_CloseMacd2060;
	m_ReverseIndexMap[L"收盘价2060分数"] = SFI_ClosePoint2060;
	m_ReverseIndexMap[L"收盘价2060排名"] = SFI_CloseRank2060;
	m_ReverseIndexMap[L"数值"] = SFI_Num;
	m_ReverseIndexMap[L"主动量比"] = SFI_ABSR;
	m_ReverseIndexMap[L"主动转被动量比"] = SFI_A2PBSR;
	m_ReverseIndexMap[L"平均主动量比"] = SFI_AABSR;
	m_ReverseIndexMap[L"最多成交价比"] = SFI_POCR;

	m_ReverseIndexMap[L"成交量(手)"] = SFI_Vol;
	m_ReverseIndexMap[L"成交额(万元)"] = SFI_Amount;

	m_ReverseIndexMap[L"成交额RPS520"] = SFI_AmountRps520;
	m_ReverseIndexMap[L"成交额Macd520"] = SFI_AmountMacd520;
	m_ReverseIndexMap[L"成交额520分数"] = SFI_AmountPoint520;
	m_ReverseIndexMap[L"成交额520排名"] = SFI_AmountRank520;
	m_ReverseIndexMap[L"成交额RPS2060"] = SFI_AmountRps2060;
	m_ReverseIndexMap[L"成交额Macd2060"] = SFI_AmountMacd2060;
	m_ReverseIndexMap[L"成交额2060分数"] = SFI_AmountPoint2060;
	m_ReverseIndexMap[L"成交额2060排名"] = SFI_AmountRank2060;
	m_ReverseIndexMap[L"成交额截面分数"] = SFI_AmountPoint;
	m_ReverseIndexMap[L"成交额截面排名"] = SFI_AmountRank;

	m_ReverseIndexMap[L"收盘价RPS520(一级行业中)"] = SFI_CloseRps520L1;
	m_ReverseIndexMap[L"收盘价Macd520(一级行业中)"] = SFI_CloseMacd520L1;
	m_ReverseIndexMap[L"收盘价520分数(一级行业中)"] = SFI_ClosePoint520L1;
	m_ReverseIndexMap[L"收盘价520排名(一级行业中)"] = SFI_CloseRank520L1;
	m_ReverseIndexMap[L"收盘价RPS2060(一级行业中)"] = SFI_CloseRps2060L1;
	m_ReverseIndexMap[L"收盘价Macd2060(一级行业中)"] = SFI_CloseMacd2060L1;
	m_ReverseIndexMap[L"收盘价2060分数(一级行业中)"] = SFI_ClosePoint2060L1;
	m_ReverseIndexMap[L"收盘价2060排名(一级行业中)"] = SFI_CloseRank2060L1;
	m_ReverseIndexMap[L"成交额RPS520(一级行业中)"] = SFI_AmountRps520L1;
	m_ReverseIndexMap[L"成交额Macd520(一级行业中)"] = SFI_AmountMacd520L1;
	m_ReverseIndexMap[L"成交额520分数(一级行业中)"] = SFI_AmountPoint520L1;
	m_ReverseIndexMap[L"成交额520排名(一级行业中)"] = SFI_AmountRank520L1;
	m_ReverseIndexMap[L"成交额RPS2060(一级行业中)"] = SFI_AmountRps2060L1;
	m_ReverseIndexMap[L"成交额Macd2060(一级行业中)"] = SFI_AmountMacd2060L1;
	m_ReverseIndexMap[L"成交额2060分数(一级行业中)"] = SFI_AmountPoint2060L1;
	m_ReverseIndexMap[L"成交额2060排名(一级行业中)"] = SFI_AmountRank2060L1;
	m_ReverseIndexMap[L"成交额截面分数(一级行业中)"] = SFI_AmountPointL1;
	m_ReverseIndexMap[L"成交额截面排名(一级行业中)"] = SFI_AmountRankL1;

	m_ReverseIndexMap[L"收盘价RPS520(二级行业中)"] = SFI_CloseRps520L2;
	m_ReverseIndexMap[L"收盘价Macd520(二级行业中)"] = SFI_CloseMacd520L2;
	m_ReverseIndexMap[L"收盘价520分数(二级行业中)"] = SFI_ClosePoint520L2;
	m_ReverseIndexMap[L"收盘价520排名(二级行业中)"] = SFI_CloseRank520L2;
	m_ReverseIndexMap[L"收盘价RPS2060(二级行业中)"] = SFI_CloseRps2060L2;
	m_ReverseIndexMap[L"收盘价Macd2060(二级行业中)"] = SFI_CloseMacd2060L2;
	m_ReverseIndexMap[L"收盘价2060分数(二级行业中)"] = SFI_ClosePoint2060L2;
	m_ReverseIndexMap[L"收盘价2060排名(二级行业中)"] = SFI_CloseRank2060L2;
	m_ReverseIndexMap[L"成交额RPS520(二级行业中)"] = SFI_AmountRps520L2;
	m_ReverseIndexMap[L"成交额Macd520(二级行业中)"] = SFI_AmountMacd520L2;
	m_ReverseIndexMap[L"成交额520分数(二级行业中)"] = SFI_AmountPoint520L2;
	m_ReverseIndexMap[L"成交额520排名(二级行业中)"] = SFI_AmountRank520L2;
	m_ReverseIndexMap[L"成交额RPS2060(二级行业中)"] = SFI_AmountRps2060L2;
	m_ReverseIndexMap[L"成交额Macd2060(二级行业中)"] = SFI_AmountMacd2060L2;
	m_ReverseIndexMap[L"成交额2060分数(二级行业中)"] = SFI_AmountPoint2060L2;
	m_ReverseIndexMap[L"成交额2060排名(二级行业中)"] = SFI_AmountRank2060L2;
	m_ReverseIndexMap[L"成交额截面分数(二级行业中)"] = SFI_AmountPointL2;
	m_ReverseIndexMap[L"成交额截面排名(二级行业中)"] = SFI_AmountRankL2;

	m_ReverseIndexMap[L"主动买入量"] = SFI_ABV;
	m_ReverseIndexMap[L"主动卖出量"] = SFI_ASV;
	m_ReverseIndexMap[L"主动买入单数"] = SFI_ABO;
	m_ReverseIndexMap[L"主动卖出单数"] = SFI_ASO;
	m_ReverseIndexMap[L"被动买入单数"] = SFI_PBO;
	m_ReverseIndexMap[L"被动卖出单数"] = SFI_PSO;

	m_ReverseIndexMap[L"开盘价"] = SFI_Open;
	m_ReverseIndexMap[L"最高价"] = SFI_High;
	m_ReverseIndexMap[L"最低价"] =SFI_Low;
	m_ReverseIndexMap[L"收盘价"] = SFI_Close;

	m_ReverseIndexMap[L"集合竞价成交量"] = SFI_CAVol;
	m_ReverseIndexMap[L"集合竞价成交量分数"] = SFI_CAVolPoint;
	m_ReverseIndexMap[L"集合竞价成交量排名"] = SFI_CAVolRank;
	m_ReverseIndexMap[L"集合竞价成交量分数(一级行业中)"] = SFI_CAVolPointL1;
	m_ReverseIndexMap[L"集合竞价成交量排名(一级行业中)"] = SFI_CAVolRankL1;
	m_ReverseIndexMap[L"集合竞价成交量分数(二级行业中)"] = SFI_CAVolPointL2;
	m_ReverseIndexMap[L"集合竞价成交量排名(二级行业中)"] = SFI_CAVolRankL2;


	m_ReverseIndexMap[L"集合竞价成交额"] = SFI_CAAmo;
	m_ReverseIndexMap[L"集合竞价成交额分数"] = SFI_CAAmoPoint;
	m_ReverseIndexMap[L"集合竞价成交额排名"] = SFI_CAAmoRank;
	m_ReverseIndexMap[L"集合竞价成交额分数(一级行业中)"] = SFI_CAAmoPointL1;
	m_ReverseIndexMap[L"集合竞价成交额排名(一级行业中)"] = SFI_CAAmoRankL1;
	m_ReverseIndexMap[L"集合竞价成交额分数(二级行业中)"] = SFI_CAAmoPointL2;
	m_ReverseIndexMap[L"集合竞价成交额排名(二级行业中)"] = SFI_CAAmoRankL2;



	m_ReverseIndexMap[L"所在一级行业的收盘价RPS520"] = SFI_L1IndyCloseRps520;
	m_ReverseIndexMap[L"所在一级行业的收盘价Macd520"] = SFI_L1IndyCloseMacd520;
	m_ReverseIndexMap[L"所在一级行业的收盘价520分数"] = SFI_L1IndyClosePoint520;
	m_ReverseIndexMap[L"所在一级行业的收盘价520排名"] = SFI_L1IndyCloseRank520;
	m_ReverseIndexMap[L"所在一级行业的收盘价RPS2060"] = SFI_L1IndyCloseRps2060;
	m_ReverseIndexMap[L"所在一级行业的收盘价Macd2060"] = SFI_L1IndyCloseMacd2060;
	m_ReverseIndexMap[L"所在一级行业的收盘价2060分数"] = SFI_L1IndyClosePoint2060;
	m_ReverseIndexMap[L"所在一级行业的收盘价2060排名"] = SFI_L1IndyCloseRank2060;
	m_ReverseIndexMap[L"所在一级行业的成交额RPS520"] = SFI_L1IndyAmountRps520;
	m_ReverseIndexMap[L"所在一级行业的成交额Macd520"] = SFI_L1IndyAmountMacd520;
	m_ReverseIndexMap[L"所在一级行业的成交额520分数"] = SFI_L1IndyAmountPoint520;
	m_ReverseIndexMap[L"所在一级行业的成交额520排名"] = SFI_L1IndyAmountRank520;
	m_ReverseIndexMap[L"所在一级行业的成交额RPS2060"] = SFI_L1IndyAmountRps2060;
	m_ReverseIndexMap[L"所在一级行业的成交额Macd2060"] = SFI_L1IndyAmountMacd2060;
	m_ReverseIndexMap[L"所在一级行业的成交额2060分数"] = SFI_L1IndyAmountPoint2060;
	m_ReverseIndexMap[L"所在一级行业的成交额2060排名"] = SFI_L1IndyAmountRank2060;
	m_ReverseIndexMap[L"所在一级行业的成交额截面分数"] = SFI_L1IndyAmountPoint;
	m_ReverseIndexMap[L"所在一级行业的成交额截面排名"] = SFI_L1IndyAmountRank;

	m_ReverseIndexMap[L"所在二级行业的收盘价RPS520"] = SFI_L2IndyCloseRps520;
	m_ReverseIndexMap[L"所在二级行业的收盘价Macd520"] = SFI_L2IndyCloseMacd520;
	m_ReverseIndexMap[L"所在二级行业的收盘价520分数"] = SFI_L2IndyClosePoint520;
	m_ReverseIndexMap[L"所在二级行业的收盘价520排名"] = SFI_L2IndyCloseRank520;
	m_ReverseIndexMap[L"所在二级行业的收盘价RPS2060"] = SFI_L2IndyCloseRps2060;
	m_ReverseIndexMap[L"所在二级行业的收盘价Macd2060"] = SFI_L2IndyCloseMacd2060;
	m_ReverseIndexMap[L"所在二级行业的收盘价2060分数"] = SFI_L2IndyClosePoint2060;
	m_ReverseIndexMap[L"所在二级行业的收盘价2060排名"] = SFI_L2IndyCloseRank2060;
	m_ReverseIndexMap[L"所在二级行业的成交额RPS520"] = SFI_L2IndyAmountRps520;
	m_ReverseIndexMap[L"所在二级行业的成交额Macd520"] = SFI_L2IndyAmountMacd520;
	m_ReverseIndexMap[L"所在二级行业的成交额520分数"] = SFI_L2IndyAmountPoint520;
	m_ReverseIndexMap[L"所在二级行业的成交额520排名"] = SFI_L2IndyAmountRank520;
	m_ReverseIndexMap[L"所在二级行业的成交额RPS2060"] = SFI_L2IndyAmountRps2060;
	m_ReverseIndexMap[L"所在二级行业的成交额Macd2060"] = SFI_L2IndyAmountMacd2060;
	m_ReverseIndexMap[L"所在二级行业的成交额2060分数"] = SFI_L2IndyAmountPoint2060;
	m_ReverseIndexMap[L"所在二级行业的成交额2060排名"] = SFI_L2IndyAmountRank2060;
	m_ReverseIndexMap[L"所在二级行业的成交额截面分数"] = SFI_L2IndyAmountPoint;
	m_ReverseIndexMap[L"所在二级行业的成交额截面排名"] = SFI_L2IndyAmountRank;

	m_ConditionMap[SFC_Greater] = L"大于";
	m_ConditionMap[SFC_EqualOrGreater] = L"大于等于";
	m_ConditionMap[SFC_Equal] = L"等于";
	m_ConditionMap[SFC_EqualOrLess] = L"小于等于";
	m_ConditionMap[SFC_Less] = L"小于";

	m_ReverseConditionMap[L"大于"] = SFC_Greater;
	m_ReverseConditionMap[L"大于等于"] = SFC_EqualOrGreater;
	m_ReverseConditionMap[L"等于"] = SFC_Equal;
	m_ReverseConditionMap[L"小于等于"] = SFC_EqualOrLess;
	m_ReverseConditionMap[L"小于"] = SFC_Less;



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
	m_NumUint[SFI_Vol] = L"(单位:手)";
	m_NumUint[SFI_Amount] = L"(单位:万元)";

	m_NumUint[SFI_CAVol] = L"(单位:手)";
	m_NumUint[SFI_CAVolPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRank] = L"(> 0)";
	m_NumUint[SFI_CAVolPointL1] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRankL1] = L"(> 0)";
	m_NumUint[SFI_CAVolPointL2] = L"(0 ~ 100)";
	m_NumUint[SFI_CAVolRankL2] = L"(> 0)";

	m_NumUint[SFI_CAAmo] = L"(单位:万元)";
	m_NumUint[SFI_CAAmoPoint] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRank] = L"(> 0)";
	m_NumUint[SFI_CAAmoPointL1] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRankL1] = L"(> 0)";
	m_NumUint[SFI_CAAmoPointL2] = L"(0 ~ 100)";
	m_NumUint[SFI_CAAmoRankL2] = L"(> 0)";




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

void SOUI::CDlgComboStockFilter::OutPutCondition(vector<StockFilter>& sfVec)
{
	sfVec.clear();
	for (int i = 0; i < m_pList->GetItemCount(); ++i)
	{
		StockFilter sf = { 0 };
		if (GetListItem(i, sf))
			sfVec.emplace_back(sf);
	}

}

void SOUI::CDlgComboStockFilter::ListAddItem(StockFilter & sf)
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

void SOUI::CDlgComboStockFilter::ListChangeItem(int nRow, StockFilter & sf)
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

void SOUI::CDlgComboStockFilter::ListDeleteItem(int nRow)
{
	if (nRow >= m_pList->GetItemCount() || nRow < 0)
		return;
	m_pList->DeleteItem(nRow);
	SStringW strTmp;
	for (int i = nRow; i < m_pList->GetItemCount(); ++i)
		m_pList->SetSubItemText(i, SFLH_ID, strTmp.Format(L"%d", i + 1));
}

bool SOUI::CDlgComboStockFilter::GetListItem(int nRow, StockFilter& sf)
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


bool SOUI::CDlgComboStockFilter::OnCbxFuncChange(EventArgs * e)
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

bool SOUI::CDlgComboStockFilter::OnCbxIndex1Change(EventArgs * e)
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
	else
	{
		m_pTextPeriod1->SetVisible(TRUE, TRUE);
		m_pCbxPeriod1->SetVisible(TRUE, TRUE);
		m_pTextPeriod1->SetAttribute(L"pos", L"5,[5");
		m_pCbxPeriod1->SetAttribute(L"pos", L"5,[5,-5,@20");
		m_pCbxIndex2->EnableWindow(TRUE, TRUE);
	}
	int nIndex2Sel = m_pCbxIndex2->GetCurSel();
	if (m_Index2PosMap[nIndex2Sel] == SFI_Num)
	{
		SStringW strNum = L"数值";
		if (m_NumUint.count(m_Index1PosMap[nSel]))
			strNum += m_NumUint[m_Index1PosMap[nSel]];
		m_pTextNum->SetWindowTextW(strNum);
	}
	return true;

}

bool SOUI::CDlgComboStockFilter::OnCbxIndex2Change(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = pEvt->nCurSel;
	int nIndex1Sel = m_pCbxIndex1->GetCurSel();
	if (SFI_Num == m_Index2PosMap[nSel])
	{
		SStringW strNum = L"数值";
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

bool SOUI::CDlgComboStockFilter::OnCbxIDChange(EventArgs * e)
{
	EventCBSelChange *pEvt = (EventCBSelChange*)e;
	SComboBox *pCbx = (SComboBox *)pEvt->sender;
	int nSel = m_pCbxFunc->GetCurSel();
	if (SFF_Change == nSel)
	{
		int nSel = pEvt->nCurSel;
		StockFilter sf;
		GetListItem(nSel, sf);
		m_pCbxIndex1->SetCurSel(sf.index1);
		m_pCbxIndex2->SetCurSel(sf.index2);
		m_pCbxPeriod1->SetCurSel(sf.period1);
		m_pCbxCondition->SetCurSel(sf.condition);
		if (sf.index2 == SFI_Num)
		{
			SStringW strNum = L"数值";
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

void SOUI::CDlgComboStockFilter::OnCbxFuncAdd()
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

void SOUI::CDlgComboStockFilter::OnCbxFuncChange()
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

void SOUI::CDlgComboStockFilter::OnCbxFuncDelete()
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

void SOUI::CDlgComboStockFilter::AddConditon()
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
		SMessageBox(m_hWnd, L"已包含相同条件，添加失败", L"警告", MB_OK | MB_ICONERROR);
		return;
	}
	m_sfSet.insert(sf);
	int nIDCount = m_pList->GetItemCount();
	ListAddItem(sf);
	SStringW strTmp;
	m_pCbxID->InsertItem(nIDCount,
		strTmp.Format(L"条件%d", nIDCount + 1), NULL, 0);
	SendMsg(m_uParThreadID, WW_SaveStockFilter, NULL, 0);
	if (m_pCheckUse->IsChecked())
	{
		BOOL bUse = TRUE;
		SendMsg(m_uParThreadID, WW_ChangeStockFilter,
			(char*)&bUse, sizeof(bUse));
	}
}

void SOUI::CDlgComboStockFilter::ChangeCondition()
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
		SMessageBox(m_hWnd, L"已包含相同条件，修改失败", L"警告", MB_OK | MB_ICONERROR);
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

void SOUI::CDlgComboStockFilter::DeleteCondition()
{
	if (m_pList->GetItemCount() == 0)
	{
		SMessageBox(m_hWnd, L"当前无条件可以删除", L"错误", MB_OK | MB_ICONERROR);
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
			SMessageBox(m_hWnd, L"当前选股器无条件，自动关闭!", L"提示", MB_OK);

		}

	}


}

void SOUI::CDlgComboStockFilter::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

