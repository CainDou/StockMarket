#include "stdafx.h"
#include "DlgBackTesting.h"
#include "SColorListCtrlEx.h"
#include "IniFile.h"
#include <algorithm>
typedef struct _testDataInfo
{
	int nPeriod;
	SStringA strIndex;
	bool operator <(const _testDataInfo& other)const;
	bool operator ==(const _testDataInfo& other) const;
}testData;


bool _testDataInfo::operator<(const _testDataInfo & other) const
{
	if (this->nPeriod == other.nPeriod)
		return this->strIndex < other.strIndex;
	return this->nPeriod < other.nPeriod;
}

bool _testDataInfo::operator==(const _testDataInfo & other) const
{
	return this->strIndex == other.strIndex && this->nPeriod == other.nPeriod;
}


CDlgBackTesting::CDlgBackTesting(set<StockFilter> sfSet, vector<StockInfo>& stockInfo) : SHostWnd(_T("LAYOUT:dlg_condBackTesting"))
{
	m_sfSet = sfSet;
	m_StockInfo = stockInfo;
	m_nLastStartDate = 0;
	m_nLastEndDate = 0;
	m_uCalcThreadID = INVALID_THREADID;
	m_uNetThreadID = INVALID_THREADID;
	m_bCalc = FALSE;
	m_bLastIsFinished = FALSE;
}

CDlgBackTesting::~CDlgBackTesting()
{
	m_NetClient.Stop();
	if (m_tCalc.joinable())
		m_tCalc.join();

}

BOOL CDlgBackTesting::OnInitDialog(EventArgs * e)
{
	m_hEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	m_pListAvgRes = FindChildByName2<SColorListCtrlEx>(L"lc_avgResult");
	m_pListDetailRes = FindChildByName2<SColorListCtrlEx>(L"lc_testResult");
	m_pDtpStartDate = FindChildByName2<SDateTimePicker>(L"dtp_testStart");
	m_pDtpEndDate = FindChildByName2<SDateTimePicker>(L"dtp_testEnd");
	m_pBtnCalc = FindChildByName2<SImageButton>(L"btn_testStart");
	m_pBtnExport = FindChildByName2<SImageButton>(L"btn_export");
	m_pTextState = FindChildByName2<SStatic>(L"text_TestState");

	SHeaderCtrlEx * pHeader =
		(SHeaderCtrlEx *)m_pListAvgRes->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(1);
	pHeader =
		(SHeaderCtrlEx *)m_pListDetailRes->GetWindow(GSW_FIRSTCHILD);
	pHeader->SetNoMoveCol(3);
	for (auto &it : m_StockInfo)
		m_StockInfoMap[it.SecurityID] = it;
	InitResTitle();
	InitNetConfig();
	InitNetHandleMap();
	InitConditonMap();
	return 0;
}

void CDlgBackTesting::InitResTitle()
{
	m_singleResTitle[SBTH_Num] = "序号";
	m_singleResTitle[SBTH_ID] = "股票代码";
	m_singleResTitle[SBTH_Date] = "日期";
	m_singleResTitle[SBTH_ROR1] = "一日收益率";
	m_singleResTitle[SBTH_ROR3] = "三日收益率";
	m_singleResTitle[SBTH_ROR5] = "五日收益率";
	m_singleResTitle[SBTH_ROR10] = "十日收益率";
	m_singleResTitle[SBTH_ROR1Over300] = "相对沪深300一日超额收益率";
	m_singleResTitle[SBTH_ROR3Over300] = "相对沪深300三日超额收益率";
	m_singleResTitle[SBTH_ROR5Over300] = "相对沪深300五日超额收益率";
	m_singleResTitle[SBTH_ROR10Over300] = "相对沪深300十日超额收益率";
	m_singleResTitle[SBTH_ROR1OverIndy1] = "相对一级行业一日超额收益率";
	m_singleResTitle[SBTH_ROR3OverIndy1] = "相对一级行业三日超额收益率";
	m_singleResTitle[SBTH_ROR5OverIndy1] = "相对一级行业五日超额收益率";
	m_singleResTitle[SBTH_ROR10OverIndy1] = "相对一级行业十日超额收益率";
	m_singleResTitle[SBTH_ROR1OverIndy2] = "相对二级行业一日超额收益率";
	m_singleResTitle[SBTH_ROR3OverIndy2] = "相对二级行业三日超额收益率";
	m_singleResTitle[SBTH_ROR5OverIndy2] = "相对二级行业五日超额收益率";
	m_singleResTitle[SBTH_ROR10OverIndy2] = "相对二级行业十日超额收益率";

	m_avgResTitle[SBTAH_DataCount] = "数据条数";
	m_avgResTitle[SBTAH_ROR1] = "一日收益率";
	m_avgResTitle[SBTAH_ROR3] = "三日收益率";
	m_avgResTitle[SBTAH_ROR5] = "五日收益率";
	m_avgResTitle[SBTAH_ROR10] = "十日收益率";
	m_avgResTitle[SBTAH_ROR1Over300] = "相对沪深300一日超额收益率";
	m_avgResTitle[SBTAH_ROR3Over300] = "相对沪深300三日超额收益率";
	m_avgResTitle[SBTAH_ROR5Over300] = "相对沪深300五日超额收益率";
	m_avgResTitle[SBTAH_ROR10Over300] = "相对沪深300十日超额收益率";
	m_avgResTitle[SBTAH_ROR1OverIndy1] = "相对一级行业一日超额收益率";
	m_avgResTitle[SBTAH_ROR3OverIndy1] = "相对一级行业三日超额收益率";
	m_avgResTitle[SBTAH_ROR5OverIndy1] = "相对一级行业五日超额收益率";
	m_avgResTitle[SBTAH_ROR10OverIndy1] = "相对一级行业十日超额收益率";
	m_avgResTitle[SBTAH_ROR1OverIndy2] = "相对二级行业一日超额收益率";
	m_avgResTitle[SBTAH_ROR3OverIndy2] = "相对二级行业三日超额收益率";
	m_avgResTitle[SBTAH_ROR5OverIndy2] = "相对二级行业五日超额收益率";
	m_avgResTitle[SBTAH_ROR10OverIndy2] = "相对二级行业十日超额收益率";

}

void SOUI::CDlgBackTesting::InitConditonMap()
{
	m_ableCondSet.insert(SFI_ChgPct);
	m_ableCondSet.insert(SFI_ClosePoint520);
	m_ableCondSet.insert(SFI_ClosePoint2060);
	m_ableCondSet.insert(SFI_Num);
	m_ableCondSet.insert(SFI_Vol);
	m_ableCondSet.insert(SFI_Amount);
	m_ableCondSet.insert(SFI_AmountPoint520);
	m_ableCondSet.insert(SFI_AmountPoint2060);
	m_ableCondSet.insert(SFI_AmountPoint);

	m_ableCondSet.insert(SFI_ClosePoint520L1);
	m_ableCondSet.insert(SFI_ClosePoint2060L1);
	m_ableCondSet.insert(SFI_AmountPoint520L1);
	m_ableCondSet.insert(SFI_AmountPoint2060L1);
	m_ableCondSet.insert(SFI_AmountPointL1);

	m_ableCondSet.insert(SFI_ClosePoint520L2);
	m_ableCondSet.insert(SFI_ClosePoint2060L2);
	m_ableCondSet.insert(SFI_AmountPoint520L2);
	m_ableCondSet.insert(SFI_AmountPoint2060L2);
	m_ableCondSet.insert(SFI_AmountPointL2);

	m_ableCondSet.insert(SFI_Open);
	m_ableCondSet.insert(SFI_High);
	m_ableCondSet.insert(SFI_Low);
	m_ableCondSet.insert(SFI_Close);


	m_ableCondSet.insert(SFI_CAVol);
	m_ableCondSet.insert(SFI_CAVolPoint);
	m_ableCondSet.insert(SFI_CAVolPointL1);
	m_ableCondSet.insert(SFI_CAVolPointL2);

	m_ableCondSet.insert(SFI_CAAmo);
	m_ableCondSet.insert(SFI_CAAmoPoint);
	m_ableCondSet.insert(SFI_CAAmoPointL1);
	m_ableCondSet.insert(SFI_CAAmoPointL2);


	m_ableCondSet.insert(SFI_L1IndyClosePoint520);
	m_ableCondSet.insert(SFI_L1IndyClosePoint2060);
	m_ableCondSet.insert(SFI_L1IndyAmountPoint520);
	m_ableCondSet.insert(SFI_L1IndyAmountPoint2060);
	m_ableCondSet.insert(SFI_L1IndyAmountPoint);

	m_ableCondSet.insert(SFI_L2IndyClosePoint520);
	m_ableCondSet.insert(SFI_L2IndyClosePoint2060);
	m_ableCondSet.insert(SFI_L2IndyAmountPoint520);
	m_ableCondSet.insert(SFI_L2IndyAmountPoint2060);
	m_ableCondSet.insert(SFI_L2IndyAmountPoint);

	m_IndexNameMap[SFI_ChgPct] = "Kline";
	m_IndexNameMap[SFI_ClosePoint520] = "ClosePoint520";
	m_IndexNameMap[SFI_ClosePoint2060] = "ClosePoint2060";
	m_IndexNameMap[SFI_Num] = "Num";
	m_IndexNameMap[SFI_Vol] = "Kline";
	m_IndexNameMap[SFI_Amount] = "Kline";
	m_IndexNameMap[SFI_AmountPoint520] = "AmountPoint520";
	m_IndexNameMap[SFI_AmountPoint2060] = "AmountPoint2060";
	m_IndexNameMap[SFI_AmountPoint] = "AmountPoint";

	m_IndexNameMap[SFI_ClosePoint520L1] = "ClosePoint520L1";
	m_IndexNameMap[SFI_ClosePoint2060L1] = "ClosePoint2060L1";
	m_IndexNameMap[SFI_AmountPoint520L1] = "AmountPoint520L1";
	m_IndexNameMap[SFI_AmountPoint2060L1] = "AmountPoint2060L1";
	m_IndexNameMap[SFI_AmountPointL1] = "AmountPointL1";

	m_IndexNameMap[SFI_ClosePoint520L2] = "ClosePoint520L2";
	m_IndexNameMap[SFI_ClosePoint2060L2] = "ClosePoint2060L2";
	m_IndexNameMap[SFI_AmountPoint520L2] = "AmountPoint520L2";
	m_IndexNameMap[SFI_AmountPoint2060L2] = "AmountPoint2060L2";
	m_IndexNameMap[SFI_AmountPointL2] = "AmountPointL2";

	m_IndexNameMap[SFI_Open] = "Kline";
	m_IndexNameMap[SFI_High] = "Kline";
	m_IndexNameMap[SFI_Low] = "Kline";
	m_IndexNameMap[SFI_Close] = "Kline";


	m_IndexNameMap[SFI_CAVol] = "CAData";
	m_IndexNameMap[SFI_CAVolPoint] = "CAData";
	m_IndexNameMap[SFI_CAVolPointL1] = "CAData";
	m_IndexNameMap[SFI_CAVolPointL2] = "CAData";

	m_IndexNameMap[SFI_CAAmo] = "CAData";
	m_IndexNameMap[SFI_CAAmoPoint] = "CAData";
	m_IndexNameMap[SFI_CAAmoPointL1] = "CAData";
	m_IndexNameMap[SFI_CAAmoPointL2] = "CAData";


	m_IndexNameMap[SFI_L1IndyClosePoint520] = "L1IndyClosePoint520";
	m_IndexNameMap[SFI_L1IndyClosePoint2060] = "L1IndyClosePoint2060";
	m_IndexNameMap[SFI_L1IndyAmountPoint520] = "L1IndyAmountPoint520";
	m_IndexNameMap[SFI_L1IndyAmountPoint2060] = "L1IndyAmountPoint2060";
	m_IndexNameMap[SFI_L1IndyAmountPoint] = "L1IndyAmountPoint";

	m_IndexNameMap[SFI_L2IndyClosePoint520] = "L2IndyClosePoint520";
	m_IndexNameMap[SFI_L2IndyClosePoint2060] = "L2IndyClosePoint2060";
	m_IndexNameMap[SFI_L2IndyAmountPoint520] = "L2IndyAmountPoint520";
	m_IndexNameMap[SFI_L2IndyAmountPoint2060] = "L2IndyAmountPoint2060";
	m_IndexNameMap[SFI_L2IndyAmountPoint] = "L2IndyAmountPoint";

	m_RevIndexNameMap["ClosePoint520"] = SFI_ClosePoint520;
	m_RevIndexNameMap["ClosePoint2060"] = SFI_ClosePoint2060;
	m_RevIndexNameMap["AmountPoint520"] = SFI_AmountPoint520;
	m_RevIndexNameMap["AmountPoint2060"] = SFI_AmountPoint2060;
	m_RevIndexNameMap["AmountPoint"] = SFI_AmountPoint;

	m_RevIndexNameMap["ClosePoint520L1"] = SFI_ClosePoint520L1;
	m_RevIndexNameMap["ClosePoint2060L1"] = SFI_ClosePoint2060L1;
	m_RevIndexNameMap["AmountPoint520L1"] = SFI_AmountPoint520L1;
	m_RevIndexNameMap["AmountPoint2060L1"] = SFI_AmountPoint2060L1;
	m_RevIndexNameMap["AmountPointL1"] = SFI_AmountPointL1;

	m_RevIndexNameMap["ClosePoint520L2"] = SFI_ClosePoint520L2;
	m_RevIndexNameMap["ClosePoint2060L2"] = SFI_ClosePoint2060L2;
	m_RevIndexNameMap["AmountPoint520L2"] = SFI_AmountPoint520L2;
	m_RevIndexNameMap["AmountPoint2060L2"] = SFI_AmountPoint2060L2;
	m_RevIndexNameMap["AmountPointL2"] = SFI_AmountPointL2;


	m_RevIndexNameMap["L1IndyClosePoint520"] = SFI_L1IndyClosePoint520;
	m_RevIndexNameMap["L1IndyClosePoint2060"] = SFI_L1IndyClosePoint2060;
	m_RevIndexNameMap["L1IndyAmountPoint520"] = SFI_L1IndyAmountPoint520;
	m_RevIndexNameMap["L1IndyAmountPoint2060"] = SFI_L1IndyAmountPoint2060;
	m_RevIndexNameMap["L1IndyAmountPoint"] = SFI_L1IndyAmountPoint;

	m_RevIndexNameMap["L2IndyClosePoint520"] = SFI_L2IndyClosePoint520;
	m_RevIndexNameMap["L2IndyClosePoint2060"] = SFI_L2IndyClosePoint2060;
	m_RevIndexNameMap["L2IndyAmountPoint520"] = SFI_L2IndyAmountPoint520;
	m_RevIndexNameMap["L2IndyAmountPoint2060"] = SFI_L2IndyAmountPoint2060;
	m_RevIndexNameMap["L2IndyAmountPoint"] = SFI_L2IndyAmountPoint;

	m_SFPeriodMap[SFP_FS] = Period_1Min;
	m_SFPeriodMap[SFP_M1] = Period_1Min;
	m_SFPeriodMap[SFP_M5] = Period_5Min;
	m_SFPeriodMap[SFP_M15] = Period_15Min;
	m_SFPeriodMap[SFP_M30] = Period_30Min;
	m_SFPeriodMap[SFP_M60] = Period_60Min;
	m_SFPeriodMap[SFP_D1] = Period_1Day;
	m_SFPeriodMap[SFP_Null] = Period_1Day;


	m_SFConditionMap[SFC_Greater] =
		&CDlgBackTesting::GreaterThan;
	m_SFConditionMap[SFC_EqualOrGreater] =
		&CDlgBackTesting::EqualOrGreaterThan;
	m_SFConditionMap[SFC_Equal] =
		&CDlgBackTesting::Equal;
	m_SFConditionMap[SFC_EqualOrLess] =
		&CDlgBackTesting::EqualOrLessThan;
	m_SFConditionMap[SFC_Less] =
		&CDlgBackTesting::LessThan;

}

void CDlgBackTesting::InitNetHandleMap()
{

	m_NetClient.SetWndHandle(m_hWnd);
	m_netHandleMap[RecvMsg_HisMultiData] =
		&CDlgBackTesting::OnMsgHisMultiData;
	m_netHandleMap[RecvMsg_HisIndexKline] =
		&CDlgBackTesting::OnMsgHisIndexKline;
	m_NetClient.RegisterHandle(NetHandle);
	m_NetClient.Start(m_uNetThreadID, this);

}

void CDlgBackTesting::OnBtnClose()
{
	ShowWindow(SW_HIDE);
}

void CDlgBackTesting::OnBtnCalc()
{

	SYSTEMTIME st;
	::GetLocalTime(&st);
	int nDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	m_pDtpStartDate->GetTime(st);
	int nStartDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	//获取数据进行前一个月和后一个月的数据冗余量
	int nStartGetDate = st.wMonth == 1 ? (st.wYear - 1) * 10000 + 1200 + st.wDay :
		st.wYear * 10000 + (st.wMonth - 1) * 100 + st.wDay;
	m_pDtpEndDate->GetTime(st);
	int nEndDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
	int nEndGetDate = st.wMonth == 12 ? (st.wYear + 1) * 10000 + 100 + st.wDay :
		st.wYear * 10000 + (st.wMonth + 1) * 100 + st.wDay;
	if (nStartDate > nDate)
	{
		SMessageBox(m_hWnd, L"起始时间大于当前时间!", L"错误", MB_OK | MB_ICONERROR);
		m_pDtpStartDate->SetFocus();
		return;
	}

	if (nEndDate > nDate)
	{
		SMessageBox(m_hWnd, L"结束时间大于当前时间!", L"错误", MB_OK | MB_ICONERROR);
		m_pDtpStartDate->SetFocus();
		return;
	}


	if (nStartDate > nEndDate)
	{
		SMessageBox(m_hWnd, L"起始时间应该小于等于结束时间!", L"错误", MB_OK | MB_ICONERROR);
		m_pDtpStartDate->SetFocus();
		return;
	}

	if (nEndDate - nStartDate > 10000)
	{
		SMessageBox(m_hWnd, L"超过最长时间限制，最长为1年!", L"错误", MB_OK | MB_ICONERROR);
		m_pDtpStartDate->SetFocus();
		return;
	}

	if (m_nLastStartDate == nStartDate && m_nLastEndDate == nEndDate && CheckCondtionIsSame())
	{
		SMessageBox(m_hWnd, L"当前设置条件下的测试已经完成，如要继续测试，请修改条件!", L"提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_nLastStartDate = nStartDate;
	m_nLastEndDate = nEndDate;
	m_pTextState->SetWindowTextW(L"开始回测...");
	CalcRes();
}

void CDlgBackTesting::OnBtnCancel()
{
	if (m_bCalc)
	{
		m_bCalc = FALSE;
		if (m_tCalc.joinable())
		{
			::SendMsg(m_uCalcThreadID, Msg_Exit, nullptr, 0);
			m_tCalc.join();
		}
		m_pTextState->SetWindowTextW(L"测试取消！");
		m_pBtnCalc->EnableWindow(TRUE, TRUE);

	}
	else
		OnBtnClose();
}

void CDlgBackTesting::OnBtnExport()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	SStringW FileName;
	FileName.Format(L"公式回测结果%d-%d.csv", m_nLastStartDate, m_nLastStartDate);
	HRESULT hr = SaveFile(FileName, szBuffer);
	if (SUCCEEDED(hr))
	{
		FileName = szBuffer;
		if (FileName.Find(L".csv") == -1)
			FileName += L".csv";
		ofstream ofile(FileName);
		if (ofile.is_open())
		{
			ofile << "平均回测结果" << endl;
			for (auto &it : m_avgResTitle)
			{
				ofile << it.second;
				if (it.first < SBTAH_ItemCount - 1)
					ofile << ",";
				else
					ofile << endl;
			}
			ofile << m_AvgRes.nCount << ",";
			SaveTestRes(ofile, m_AvgRes.res);

			ofile << "具体个股回测结果" << endl;
			for (auto &it : m_singleResTitle)
			{
				ofile << it.second;
				if (it.first < SBTH_ItemCount - 1)
					ofile << ",";
				else
					ofile << endl;
			}
			for (int i = 0; i < m_resVec.size(); ++i)
			{
				ofile << i + 1 << ",";
				ofile << m_resVec[i].StockID << ",";
				ofile << m_resVec[i].nDate << ",";
				SaveTestRes(ofile, m_resVec[i].res);
			}
			SMessageBox(m_hWnd, L"文件导出成功！", L"提示", MB_OK | MB_ICONINFORMATION);
		}
		else
			SMessageBox(m_hWnd, L"文件已经打开，导出失败！", L"提示", MB_OK | MB_ICONINFORMATION);

	}
}

void CDlgBackTesting::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CDlgBackTesting::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CDlgBackTesting::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CDlgBackTesting::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}

}

void CDlgBackTesting::ConditionHandle()
{
	set<testData> useDataSet;
	testData td;
	td.strIndex = "Kline";
	td.nPeriod = Period_1Day;
	useDataSet.insert(td);
	m_sfVecByOrder.clear();
	for (auto &it : m_sfSet)
	{
		if (m_ableCondSet.count(it.index1) == 0)
			continue;
		if (m_ableCondSet.count(it.index2) == 0)
			continue;
		td.nPeriod = m_SFPeriodMap[it.period1];
		td.strIndex = m_IndexNameMap[it.index1];
		useDataSet.insert(td);
		td.nPeriod = m_SFPeriodMap[it.period2];
		td.strIndex = m_IndexNameMap[it.index2];
		useDataSet.insert(td);
		m_sfVecByOrder.emplace_back(it);
	}
	std::sort(m_sfVecByOrder.begin(), m_sfVecByOrder.end(),
		[&](const StockFilter& a, const StockFilter& b)
	{int aMax = max(m_SFPeriodMap[a.period1], m_SFPeriodMap[a.period2]);
	int bMax = max(m_SFPeriodMap[b.period1], m_SFPeriodMap[b.period2]);
	return aMax > bMax;	});
	m_strDataGetMsg.Empty();

	for (auto &it : useDataSet)
	{
		if (it.strIndex != "Num")
		{
			SStringA str;
			str.Format("dataName:%s,dataPeriod:%d;", it.strIndex, it.nPeriod);
			m_strDataGetMsg += str;
		}
	}

}

void CDlgBackTesting::CalcRes()
{
	m_bLastIsFinished = FALSE;
	if (!m_NetClient.GetState())
	{
		if (!m_NetClient.OnConnect(m_strIPAddr, m_nIPPort))
		{
			SMessageBox(m_hWnd, L"无法与服务器端连接获取数据，请重试!", L"警告", MB_OK | MB_ICONWARNING);
			return;
		}
	}
	m_pBtnExport->EnableWindow(FALSE, TRUE);
	m_pListDetailRes->DeleteAllItems();
	m_pListAvgRes->DeleteAllItems();

	m_pBtnCalc->EnableWindow(FALSE, TRUE);
	m_bCalc = TRUE;
	ConditionHandle();
	m_resVec.clear();
	m_AvgRes.clear();
	m_tCalc = thread(&CDlgBackTesting::TestingData, this);
	m_uCalcThreadID = *(unsigned*)&m_tCalc.get_id();
	m_nDataGetCount = 0;
	m_nFinishCount = 0;
	m_LastTestSf = m_sfSet;
	::SendMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_GetData, 0);

}

void CDlgBackTesting::UpdateList(int nRow)
{
	SStringW tmp;
	const auto & res = m_resVec[nRow];
	tmp.Format(L"%d", nRow + 1);
	m_pListDetailRes->InsertItem(nRow, tmp);
	m_pListDetailRes->SetSubItemText(nRow, SBTH_ID, StrA2StrW(res.StockID));
	tmp.Format(L"%04d-%02d-%02d", res.nDate / 10000, res.nDate % 10000 / 100, res.nDate % 100);
	m_pListDetailRes->SetSubItemText(nRow, SBTH_Date, tmp);
	double fDataArr[] = { res.res.RoR1,res.res.RoR3,res.res.RoR5,res.res.RoR10,
		res.res.RoR1Over300,res.res.RoR3Over300,res.res.RoR5Over300,res.res.RoR10Over300,
		res.res.RoR1OverIndy1,res.res.RoR3OverIndy1,res.res.RoR5OverIndy1,res.res.RoR10OverIndy1,
		res.res.RoR1OverIndy2,res.res.RoR3OverIndy2,res.res.RoR5OverIndy2,res.res.RoR10OverIndy2 };
	for (int i = 0; i < 16; ++i)
	{
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (!isnan(fDataArr[i]))
		{
			tmp.Format(L"%.02f", fDataArr[i]);
			if (fDataArr[i] > 0)
				cl = RGBA(255, 0, 0, 255);
			else if (fDataArr[i] < 0)
				cl = RGBA(0, 255, 0, 255);
		}
		else
			tmp.Format(L"-");
		m_pListDetailRes->SetSubItemText(nRow, SBTH_ROR1 + i, tmp, cl);

	}
	m_pListDetailRes->RequestRelayout();
	tmp.Format(L"%d", m_AvgRes.nCount[0]);
	if (m_pListAvgRes->GetItemCount() <= 0)
		m_pListAvgRes->InsertItem(0, tmp);
	else
		m_pListAvgRes->SetSubItemText(0, SBTAH_DataCount, tmp);
	double fAvgDataArr[] = { m_AvgRes.res.RoR1,m_AvgRes.res.RoR3,m_AvgRes.res.RoR5,m_AvgRes.res.RoR10,
		m_AvgRes.res.RoR1Over300,m_AvgRes.res.RoR3Over300,m_AvgRes.res.RoR5Over300,m_AvgRes.res.RoR10Over300,
		m_AvgRes.res.RoR1OverIndy1,m_AvgRes.res.RoR3OverIndy1,m_AvgRes.res.RoR5OverIndy1,m_AvgRes.res.RoR10OverIndy1,
		m_AvgRes.res.RoR1OverIndy2,m_AvgRes.res.RoR3OverIndy2,m_AvgRes.res.RoR5OverIndy2,m_AvgRes.res.RoR10OverIndy2 };

	for (int i = 0; i < 16; ++i)
	{
		COLORREF cl = RGBA(255, 255, 255, 255);
		if (!isnan(fAvgDataArr[i]))
		{
			tmp.Format(L"%.02f", fAvgDataArr[i]);
			if (fAvgDataArr[i] > 0)
				cl = RGBA(255, 0, 0, 255);
			else if (fAvgDataArr[i] < 0)
				cl = RGBA(0, 255, 0, 255);
		}
		else
			tmp.Format(L"-");

		m_pListAvgRes->SetSubItemText(0, SBTAH_ROR1 + i, tmp, cl);
	}
	m_pListAvgRes->RequestRelayout();

}

BOOL CDlgBackTesting::CheckCondtionIsSame()
{
	if (m_LastTestSf.size() != m_sfSet.size())
		return FALSE;
	for (auto &it : m_sfSet)
	{
		if (m_LastTestSf.count(it) == 0)
			return FALSE;
	}
	return TRUE;
}


LRESULT CDlgBackTesting::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nMsgId = (int)wp;
	switch (nMsgId)
	{
	case BTM_UpdateList:
		UpdateList((int)lp);
		break;
	case BTM_GetData:
	{
		if (m_nDataGetCount < m_StockInfo.size())
		{
			GetTestMultiData(m_StockInfo[m_nDataGetCount].SecurityID);
			++m_nDataGetCount;
		}
	}
	break;
	case BTM_SingleCalcFinish:
	{
		++m_nFinishCount;

		SStringW tmp;
		if (m_nFinishCount < m_StockInfo.size())
			tmp.Format(L"测试正在进行中，当前进度%d/%d...", m_nFinishCount, m_StockInfo.size());
		else
			tmp.Format(L"测试完成！");
		m_pTextState->SetWindowTextW(tmp);
		if (m_nFinishCount >= m_StockInfo.size())
			::PostMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_AllFinish, 0);
	}
	break;
	case BTM_AllFinish:
	{
		m_bCalc = FALSE;
		m_bLastIsFinished = TRUE;
		if (m_tCalc.joinable())
		{
			::SendMsg(m_uCalcThreadID, Msg_Exit, nullptr, 0);
			m_tCalc.join();
		}
		m_pBtnCalc->EnableWindow(TRUE, TRUE);
		m_pBtnExport->EnableWindow(TRUE, TRUE);

	}
	default:
		break;
	}
	return 0;
}

unsigned CDlgBackTesting::NetHandle(void * para)
{
	int nOffset = 0;
	ReceiveInfo recvInfo;
	BOOL bNeedConnect = false;
	CDlgBackTesting *pDlg = (CDlgBackTesting*)para;
	auto &NetClient = pDlg->m_NetClient;
	while (true)
	{
		if (NetClient.GetExitState())
			return 0;
		if (pDlg->RecvInfoHandle(bNeedConnect, nOffset, recvInfo))
		{
			auto pFuc = pDlg->m_netHandleMap[recvInfo.MsgType];
			if (pFuc == nullptr)
				pFuc = &CDlgBackTesting::OnNoDefineMsg;
			(pDlg->*pFuc)(recvInfo);
		}

	}
	return 0;
}

void CDlgBackTesting::InitNetConfig()
{
	CIniFile ini(".//config//NetConfig.ini");
	m_strIPAddr = ini.GetStringA("IP", "Addr", "");
	m_nIPPort = ini.GetIntA("IP", "Port", 0);

}


bool CDlgBackTesting::RecvInfoHandle(BOOL & bNeedConnect, int & nOffset, ReceiveInfo & recvInfo)
{
	if (bNeedConnect)
	{
		if (m_NetClient.GetExitState())
			return 0;
		if (m_NetClient.OnConnect(m_strIPAddr, m_nIPPort))
			bNeedConnect = false;
		else
			return false;
	}

	int ret = recv(m_NetClient.GetSocket(),
		(char*)&recvInfo + nOffset,
		sizeof(recvInfo) - nOffset, 0);
	if (ret == 0)
	{
		nOffset = 0;
		m_NetClient.OnConnect(NULL, NULL);
		bNeedConnect = true;
		TraceLog("与服务器断开连接");
		return false;
	}

	if (SOCKET_ERROR == ret)
	{
		//nOffset = 0;
		if (m_NetClient.GetExitState())
			return false;
		int nError = WSAGetLastError();
		if (nError == WSAECONNRESET)
		{
			m_NetClient.OnConnect(NULL, NULL);
			bNeedConnect = true;
		}
		return false;
	}

	if (ret + nOffset < sizeof(recvInfo))
	{
		nOffset += ret;
		return false;
	}
	nOffset = 0;

	return true;
}

void CDlgBackTesting::OnMsgHisMultiData(ReceiveInfo & recvInfo)
{
	int totalSize = recvInfo.DataSize + sizeof(recvInfo);
	char *buffer = new char[totalSize];
	memcpy_s(buffer, totalSize, &recvInfo, sizeof(recvInfo));
	m_NetClient.ReceiveData(buffer + sizeof(recvInfo), recvInfo.DataSize, '#');
	SendMsg(m_uCalcThreadID, BackTesting, buffer, recvInfo.DataSize);
	delete[]buffer;
	buffer = nullptr;
	::PostMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_GetData, 0);
}

void CDlgBackTesting::OnMsgHisIndexKline(ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	m_NetClient.ReceiveData(buffer, recvInfo.DataSize, '#');
	KlineType * klineData = (KlineType *)(buffer + sizeof(int));
	int nDataCount = recvInfo.DataSize / sizeof(KlineType);
	auto &dataMap = m_IndexData[recvInfo.InsID];
	for (int i = 0; i < nDataCount; ++i)
		dataMap[klineData[i].date] = klineData[i];
	delete[]buffer;
	buffer = nullptr;
	SetEvent(m_hEvent);

}

void CDlgBackTesting::OnNoDefineMsg(ReceiveInfo & recvInfo)
{
	char *buffer = new char[recvInfo.DataSize];
	m_NetClient.ReceiveData(buffer, recvInfo.DataSize, '#');
	delete[]buffer;
	buffer = nullptr;

}

void CDlgBackTesting::GetIndexKline(SStringA strIndexID)
{
	SendInfoWithDate info = { 0 };
	info.MsgType = SendType_IndexHisDayKline;
	info.StartDate = m_nLastStartDate;
	info.EndDate = m_nLastEndDate;
	strcpy_s(info.StockID, strIndexID);
	m_NetClient.SendDataWithID((char*)&info, sizeof(info));
}

void CDlgBackTesting::GetTestMultiData(SStringA StockID)
{
	SendInfoWithDate info = { 0 };
	info.MsgType = SendType_HisMultiData;

	info.StartDate = m_nLastStartDate;
	info.EndDate = m_nLastEndDate;
	strcpy_s(info.StockID, StockID);
	int attSize = m_strDataGetMsg.GetLength() + 1;
	int nSize = sizeof(info) + attSize + sizeof(attSize);
	char *msg = new char[nSize];
	memcpy_s(msg, nSize, &info, sizeof(info));
	int nOffset = sizeof(info);
	memcpy_s(msg + nOffset, nSize, &attSize, sizeof(attSize));
	nOffset += sizeof(attSize);
	memcpy_s(msg + nOffset, nSize, m_strDataGetMsg, attSize);
	BOOL bRet = m_NetClient.SendDataWithID(msg, nSize);
}


void CDlgBackTesting::TestingData()
{
	int MsgId;
	char *info;
	int msgLength;

	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			m_uCalcThreadID = INVALID_THREADID;
			OutputDebugStringFormat("线程退出\n");
			break;
		}
		else if (BackTesting == MsgId)
			CalcData(info, msgLength);
		delete[]info;
		info = nullptr;
	}
}

void CDlgBackTesting::CalcData(char* msg, int nMsgLength)
{
	BackTestingData testDataMap;
	ReceiveInfo Info = *(ReceiveInfo*)msg;
	int nOffset = sizeof(ReceiveInfo);
	while (nOffset < nMsgLength)
	{
		int nMsgOffset = GetDataFromMsg(msg + nOffset, testDataMap);
		nOffset += nMsgOffset;
	}

	vector<int> dataPassDate;
	for (auto &it : testDataMap.dataMap)
	{
		int nDate = it.first;
		if (nDate >= m_nLastStartDate && nDate <= m_nLastEndDate)
		{
			bool bFit = true;
			for (auto &sf : m_sfVecByOrder)
			{
				if (!CheckDataPass(sf, it.second))
				{
					bFit = false;
					break;
				}
			}
			if (bFit)
				dataPassDate.emplace_back(nDate);
		}
	}
	ProcFitDataRes(Info.InsID, dataPassDate, testDataMap);
	::PostMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_SingleCalcFinish, (LPARAM)Info.InsID);
}

int CDlgBackTesting::GetDataFromMsg(char * msg, BackTestingData & data)
{
	int nOffset = 0;
	int nDataNameSize = *(int*)(msg + nOffset);
	nOffset += sizeof(nDataNameSize);
	char* dataName = new char[nDataNameSize];
	strcpy_s(dataName, nDataNameSize, msg + nOffset);
	nOffset += nDataNameSize;
	//周期
	int nPeriod = *(int*)(msg + nOffset);
	nOffset += sizeof(nPeriod);
	int nDataSize = *(int*)(msg + nOffset);

	nOffset += sizeof(nDataSize);
	char *strData = new char[nDataSize];
	memcpy_s(strData, nDataSize, msg + nOffset, nDataSize);
	nOffset += nDataSize;
	if (strcmp(dataName, "Rehab") == 0)
	{
		RehabInfo * rehabInfo = (RehabInfo *)strData;
		int nDataCount = nDataSize / sizeof(RehabInfo);
		for (int i = 0; i < nDataCount; ++i)
		{
			int nDate = rehabInfo[i].exDivDate;
			data.rehabMap[nDate] = rehabInfo[i].accumAdjFactor;
		}
	}
	else if (strcmp(dataName, "Kline") == 0)
	{
		KlineType * klineData = (KlineType*)strData;
		int nDataCount = nDataSize / sizeof(KlineType);
		int fPreClose = 0;
		for (int i = 0; i < nDataCount; ++i)
		{
			int nDate = klineData[i].date;
			int nTime = klineData[i].time;
			if (nDate >= m_nLastStartDate && nDate <= m_nLastEndDate)
			{
				data.dataMap[nDate][nPeriod][SFI_Vol][nTime] = klineData[i].vol;
				data.dataMap[nDate][nPeriod][SFI_Amount][nTime] = klineData[i].amount;
				data.dataMap[nDate][nPeriod][SFI_Open][nTime] = klineData[i].open;
				data.dataMap[nDate][nPeriod][SFI_High][nTime] = klineData[i].high;
				data.dataMap[nDate][nPeriod][SFI_Low][nTime] = klineData[i].low;
				data.dataMap[nDate][nPeriod][SFI_Close][nTime] = klineData[i].close;
			}
			if (nPeriod == Period_1Day)
			{
				double fClose = klineData[i].close;
				for (auto &it : data.rehabMap)
				{
					if (nDate < it.first)
						break;
					fClose = klineData[i].close * it.second;
				}
				if (fPreClose != 0)
				{
					data.dataMap[nDate][nPeriod][SFI_ChgPct][nTime] =
						(klineData[i].close - fPreClose) / fPreClose * 100;
				}
				if (nDate <m_nLastStartDate || nDate >m_nLastEndDate)
					data.dataMap[nDate][nPeriod][SFI_Close][nTime] = klineData[i].close;
				fPreClose = fClose;
			}
		}
	}
	else if (strcmp(dataName, "CAData") == 0)
	{
		CAInfo * caData = (CAInfo*)strData;
		int nDataCount = nDataSize / sizeof(CAInfo);
		for (int i = 0; i < nDataCount; ++i)
		{
			int nDate = caData[i].date;
			int nTime = 0;
			if (nDate >= m_nLastStartDate && nDate <= m_nLastEndDate)
			{
				data.dataMap[nDate][nPeriod][SFI_CAVol][nTime] = caData[i].Volume;
				data.dataMap[nDate][nPeriod][SFI_CAVolPoint][nTime] = caData[i].VolPoint;
				data.dataMap[nDate][nPeriod][SFI_CAVolPointL1][nTime] = caData[i].VolPointL1;
				data.dataMap[nDate][nPeriod][SFI_CAVolPointL2][nTime] = caData[i].VolPointL2;
				data.dataMap[nDate][nPeriod][SFI_CAAmo][nTime] = caData[i].AmoPoint;
				data.dataMap[nDate][nPeriod][SFI_CAAmoPoint][nTime] = caData[i].AmoPoint;
				data.dataMap[nDate][nPeriod][SFI_CAAmoPointL1][nTime] = caData[i].AmoPointL1;
				data.dataMap[nDate][nPeriod][SFI_CAAmoPointL2][nTime] = caData[i].AmoPointL2;

			}
		}

	}
	else
	{
		CoreData * crData = (CoreData*)strData;
		int nDataCount = nDataSize / sizeof(CoreData);
		int nDataIndex = m_RevIndexNameMap[dataName];
		for (int i = 0; i < nDataCount; ++i)
		{
			int nDate = crData[i].date;
			int nTime = crData[i].time;
			if (nDate >= m_nLastStartDate && nDate <= m_nLastEndDate)
				data.dataMap[nDate][nPeriod][nDataIndex][nTime] = crData[i].date;

		}

	}
	return nOffset;
}

bool CDlgBackTesting::CheckDataPass(StockFilter & sf, map<int, map<int, map<int, double>>>& dataMap)
{
	int nPeriod1 = m_SFPeriodMap[sf.period1];
	int nPeriod2 = m_SFPeriodMap[sf.period2];

	auto &timeData1 = dataMap[nPeriod1][sf.index1];
	if (sf.index2 == SFI_Num)
	{
		double data2 = sf.num;
		for (auto &it : timeData1)
		{
			double fData1 = it.second;
			if (sf.index1 == SFI_Amount || sf.index1 == SFI_CAAmo)
				fData1 /= 10000;
			if ((this->*m_SFConditionMap[sf.condition])(fData1, data2))
				return true;
		}
	}
	else
	{
		auto &timeData2 = dataMap[nPeriod2][sf.index2];
		auto &longPeriodData = nPeriod1 < nPeriod2 ? timeData2 : timeData1;
		auto &shortPeriodData = nPeriod1 < nPeriod2 ? timeData1 : timeData2;
		auto itShort = shortPeriodData.begin();
		for (auto &dataLong : longPeriodData)
		{
			int nNowTime = dataLong.first == 0 ? 1500 : dataLong.first;
			for (itShort; itShort != shortPeriodData.end(); itShort++)
			{
				int nNowShortTime = itShort->first == 0 ? 1500 : itShort->first;

				double fData1 = nPeriod1 < nPeriod2 ? itShort->second : dataLong.second;
				double fData2 = nPeriod1 < nPeriod2 ? dataLong.second : itShort->second;
				if (sf.index1 == SFI_Amount || sf.index1 == SFI_CAAmo)
					fData1 /= 10000;
				if (sf.index2 == SFI_Amount || sf.index2 == SFI_CAAmo)
					fData2 /= 10000;

				if ((this->*m_SFConditionMap[sf.condition])(fData1, fData2))
					return true;
				if (nNowShortTime == nNowTime)
					break;
			}
		}
	}
	return false;
}

bool CDlgBackTesting::ProcFitDataRes(SStringA StockID, vector<int>& dataPassDate,
	BackTestingData& testDataMap)
{
	if (!dataPassDate.empty())
	{
		const auto& stockInfo = m_StockInfoMap[StockID];
		if (m_IndexData.count("000300I") == 0)
		{
			ResetEvent(m_hEvent);
			GetIndexKline("000300I");
			WaitForSingleObject(m_hEvent, INFINITE);
		}
		if (m_IndexData.count(stockInfo.SWL1ID) == 0)
		{
			ResetEvent(m_hEvent);
			GetIndexKline(stockInfo.SWL1ID);
			WaitForSingleObject(m_hEvent, INFINITE);
		}
		if (m_IndexData.count(stockInfo.SWL2ID) == 0)
		{
			ResetEvent(m_hEvent);
			GetIndexKline(stockInfo.SWL2ID);
			WaitForSingleObject(m_hEvent, INFINITE);
		}
		//以收盘价为锚点进行日期运算操作
		int nDataCount = 0;
		map<int, int> DayCountMap;
		map<int, SingleRes> singleResMap;
		for (auto &it : dataPassDate)
		{
			DayCountMap[it] = 0;
			singleResMap[it] = SingleRes();
			auto& res = singleResMap[it];
			res.nDate = it;
			res.StockID = StockID;
		}
		for (auto &it : testDataMap.dataMap)
		{
			int nDate = it.first;
			for (auto &dayCount : DayCountMap)
			{
				auto &res = singleResMap[dayCount.first];
				if (nDate > dayCount.first)
					dayCount.second++;
				if (dayCount.second == 1 || dayCount.second == 3 || dayCount.second == 5 || dayCount.second == 10)
				{
					double fNowClose = it.second[Period_1Day][SFI_Close][0];
					double fFitDayClose = testDataMap.dataMap[dayCount.first][Period_1Day][SFI_Close][0];
					for (auto &rehab : testDataMap.rehabMap)
					{
						if (nDate < rehab.first)
							break;
						fNowClose = it.second[Period_1Day][SFI_Close][0] * rehab.second;
						if (dayCount.first >= rehab.first)
							fFitDayClose = testDataMap.dataMap[dayCount.first][Period_1Day][SFI_Close][0] * rehab.second;
					}
					double f300NowClose = m_IndexData["000300I"][nDate].close;
					double f300FitDayClose = m_IndexData["000300I"][dayCount.first].close;
					double fIndy1NowClose = m_IndexData[stockInfo.SWL1ID][nDate].close;
					double fIndy1FitDayClose = m_IndexData[stockInfo.SWL1ID][dayCount.first].close;
					double fIndy2NowClose = m_IndexData[stockInfo.SWL2ID][nDate].close;
					double fIndy2FitDayClose = m_IndexData[stockInfo.SWL2ID][dayCount.first].close;
					if (dayCount.second == 1)
					{
						res.res.RoR1 = (fNowClose - fFitDayClose) / fFitDayClose * 100;
						res.res.RoR1Over300 = (f300NowClose - f300FitDayClose) / f300FitDayClose * 100;
						res.res.RoR1Over300 = res.res.RoR1 - res.res.RoR1Over300;
						res.res.RoR1OverIndy1 = (fIndy1NowClose - fIndy1FitDayClose) / fIndy1FitDayClose * 100;
						res.res.RoR1OverIndy1 = res.res.RoR1 - res.res.RoR1OverIndy1;
						res.res.RoR1OverIndy2 = (fIndy2NowClose - fIndy2FitDayClose) / fIndy2FitDayClose * 100;
						res.res.RoR1OverIndy2 = res.res.RoR1 - res.res.RoR1OverIndy2;
					}
					else if (dayCount.second == 3)
					{
						res.res.RoR3 = (fNowClose - fFitDayClose) / fFitDayClose * 100;
						res.res.RoR3Over300 = (f300NowClose - f300FitDayClose) / f300FitDayClose * 100;
						res.res.RoR3Over300 = res.res.RoR3 - res.res.RoR3Over300;
						res.res.RoR3OverIndy1 = (fIndy1NowClose - fIndy1FitDayClose) / fIndy1FitDayClose * 100;
						res.res.RoR3OverIndy1 = res.res.RoR3 - res.res.RoR3OverIndy1;
						res.res.RoR3OverIndy2 = (fIndy2NowClose - fIndy2FitDayClose) / fIndy2FitDayClose * 100;
						res.res.RoR3OverIndy2 = res.res.RoR3 - res.res.RoR3OverIndy2;
					}
					else if (dayCount.second == 5)
					{
						res.res.RoR5 = (fNowClose - fFitDayClose) / fFitDayClose * 100;
						res.res.RoR5Over300 = (f300NowClose - f300FitDayClose) / f300FitDayClose * 100;
						res.res.RoR5Over300 = res.res.RoR5 - res.res.RoR5Over300;
						res.res.RoR5OverIndy1 = (fIndy1NowClose - fIndy1FitDayClose) / fIndy1FitDayClose * 100;
						res.res.RoR5OverIndy1 = res.res.RoR5 - res.res.RoR5OverIndy1;
						res.res.RoR5OverIndy2 = (fIndy2NowClose - fIndy2FitDayClose) / fIndy2FitDayClose * 100;
						res.res.RoR5OverIndy2 = res.res.RoR5 - res.res.RoR5OverIndy2;
					}
					else if (dayCount.second == 10)
					{
						res.res.RoR10 = (fNowClose - fFitDayClose) / fFitDayClose * 100;
						res.res.RoR10Over300 = (f300NowClose - f300FitDayClose) / f300FitDayClose * 100;
						res.res.RoR10Over300 = res.res.RoR10 - res.res.RoR10Over300;
						res.res.RoR10OverIndy1 = (fIndy1NowClose - fIndy1FitDayClose) / fIndy1FitDayClose * 100;
						res.res.RoR10OverIndy1 = res.res.RoR10 - res.res.RoR10OverIndy1;
						res.res.RoR10OverIndy2 = (fIndy2NowClose - fIndy2FitDayClose) / fIndy2FitDayClose * 100;
						res.res.RoR10OverIndy2 = res.res.RoR10 - res.res.RoR10OverIndy2;
						m_resVec.emplace_back(res);
						ProcAvgRes(res);
						::SendMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_UpdateList, m_resVec.size() - 1);
					}
				}
			}

		}
		for (auto &dayCount : DayCountMap)
		{
			if (dayCount.second < 10)
			{
				auto &res = singleResMap[dayCount.first];
				m_resVec.emplace_back(res);
				ProcAvgRes(res);
				::SendMessage(m_hWnd, WM_BACKTESTING_MSG, BTM_UpdateList, m_resVec.size() - 1);
			}
		}
		return true;
	}
	return false;
}

bool CDlgBackTesting::SaveTestRes(ofstream & ofile, TestRes & res)
{
	if (!isnan(res.RoR1))
		ofile << res.RoR1 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR3))
		ofile << res.RoR3 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR5))
		ofile << res.RoR5 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR10))
		ofile << res.RoR10 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR1Over300))
		ofile << res.RoR1Over300 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR3Over300))
		ofile << res.RoR3Over300 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR5Over300))
		ofile << res.RoR5Over300 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR10Over300))
		ofile << res.RoR10Over300 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR1OverIndy1))
		ofile << res.RoR1OverIndy1 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR3OverIndy1))
		ofile << res.RoR3OverIndy1 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR5OverIndy1))
		ofile << res.RoR5OverIndy1 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR10OverIndy1))
		ofile << res.RoR10OverIndy1 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR1OverIndy2))
		ofile << res.RoR1OverIndy2 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR3OverIndy2))
		ofile << res.RoR3OverIndy2 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR5OverIndy2))
		ofile << res.RoR5OverIndy2 << ",";
	else
		ofile << "-" << ",";
	if (!isnan(res.RoR10OverIndy2))
		ofile << res.RoR10OverIndy2 << endl;
	else
		ofile << "-" << endl;
	return true;
}


void CDlgBackTesting::ProcAvgRes(SingleRes & res)
{
	int nCCount = 0;
	m_AvgRes.nCount[nCCount++]++;

	if (!isnan(res.res.RoR1))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR1 = res.res.RoR1;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR1 = (m_AvgRes.res.RoR1 * m_AvgRes.nCount[nCCount] + res.res.RoR1)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR3))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR3 = res.res.RoR3;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR3 = (m_AvgRes.res.RoR3 * m_AvgRes.nCount[nCCount] + res.res.RoR3)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR5))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR5 = res.res.RoR5;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR5 = (m_AvgRes.res.RoR5 * m_AvgRes.nCount[nCCount] + res.res.RoR5)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR10))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR10 = res.res.RoR10;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR10 = (m_AvgRes.res.RoR10 * m_AvgRes.nCount[nCCount] + res.res.RoR10)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR1Over300))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR1Over300 = res.res.RoR1Over300;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR1Over300 = (m_AvgRes.res.RoR1Over300 * m_AvgRes.nCount[nCCount] + res.res.RoR1Over300)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR3Over300))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR3Over300 = res.res.RoR3Over300;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR3Over300 = (m_AvgRes.res.RoR3Over300 * m_AvgRes.nCount[nCCount] + res.res.RoR3Over300)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR5Over300))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR5Over300 = res.res.RoR5Over300;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR5Over300 = (m_AvgRes.res.RoR5Over300 * m_AvgRes.nCount[nCCount] + res.res.RoR5Over300)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR10Over300))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR10Over300 = res.res.RoR10Over300;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR10Over300 = (m_AvgRes.res.RoR10Over300 * m_AvgRes.nCount[nCCount] + res.res.RoR10Over300)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;



	if (!isnan(res.res.RoR1OverIndy1))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR1OverIndy1 = res.res.RoR1OverIndy1;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR1OverIndy1 = (m_AvgRes.res.RoR1OverIndy1 * m_AvgRes.nCount[nCCount] + res.res.RoR1OverIndy1)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR3OverIndy1))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR3OverIndy1 = res.res.RoR3OverIndy1;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR3OverIndy1 = (m_AvgRes.res.RoR3OverIndy1 * m_AvgRes.nCount[nCCount] + res.res.RoR3OverIndy1)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR5OverIndy1))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR5OverIndy1 = res.res.RoR5OverIndy1;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR5OverIndy1 = (m_AvgRes.res.RoR5OverIndy1 * m_AvgRes.nCount[nCCount] + res.res.RoR5OverIndy1)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}	
	nCCount++;

	if (!isnan(res.res.RoR10OverIndy1))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR10OverIndy1 = res.res.RoR10OverIndy1;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR10OverIndy1 = (m_AvgRes.res.RoR10OverIndy1 * m_AvgRes.nCount[nCCount] + res.res.RoR10OverIndy1)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR1OverIndy2))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR1OverIndy2 = res.res.RoR1OverIndy2;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR1OverIndy2 = (m_AvgRes.res.RoR1OverIndy2 * m_AvgRes.nCount[nCCount] + res.res.RoR1OverIndy2)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR3OverIndy2))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR3OverIndy2 = res.res.RoR3OverIndy2;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR3OverIndy2 = (m_AvgRes.res.RoR3OverIndy2 * m_AvgRes.nCount[nCCount] + res.res.RoR3OverIndy2)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR5OverIndy2))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR5OverIndy2 = res.res.RoR5OverIndy2;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR5OverIndy2 = (m_AvgRes.res.RoR5OverIndy2 * m_AvgRes.nCount[nCCount] + res.res.RoR5OverIndy2)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

	if (!isnan(res.res.RoR10OverIndy2))
	{
		if (m_AvgRes.nCount[nCCount] == 0)
		{
			m_AvgRes.res.RoR10OverIndy2 = res.res.RoR10OverIndy2;
			m_AvgRes.nCount[nCCount]++;
		}
		else
		{
			m_AvgRes.res.RoR10OverIndy2 = (m_AvgRes.res.RoR10OverIndy2 * m_AvgRes.nCount[nCCount] + res.res.RoR10OverIndy2)
				/ (m_AvgRes.nCount[nCCount] + 1);
			m_AvgRes.nCount[nCCount]++;
		}
	}
	nCCount++;

}

void CDlgBackTesting::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

