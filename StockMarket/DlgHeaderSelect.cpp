#include "stdafx.h"
#include "DlgHeaderSelect.h"
#include "AdpterFixTitle.h"
#include "Define.h"


CDlgHeaderSelect::CDlgHeaderSelect(HWND hWnd, map<int,BOOL> showMap) :SHostWnd(_T("LAYOUT:dlg_headerSelect"))
{
	m_hParWnd = hWnd;
	m_bShowMap = showMap;
}


CDlgHeaderSelect::~CDlgHeaderSelect()
{
}

BOOL CDlgHeaderSelect::OnInitDialog(EventArgs * e)
{
	m_pLvTitle = FindChildByName2<SListView>(L"lv_title");
	m_pLvTitle->GetEventSet()->subscribeEvent(EVT_LV_SELCHANGED,
		Subscriber(&CDlgHeaderSelect::OnListTitleClicked, this));
	m_pEditDscp = FindChildByName2<SEdit>(L"edit_dscp");

	InitTilteMap();
	InitTilteDscpMap();

	if (m_pLvTitle)
	{
		ILvAdapter *pAdapter = new CAdpterFixTitle(m_bShowMap.size(), m_bShowMap,m_titleMap);
		m_pLvTitle->SetAdapter(pAdapter);
		pAdapter->Release();
		//CAdpterFixTitle *pTitltAdapter = (CAdpterFixTitle*)m_pLvTitle->GetAdapter();
		//OutputDebugStringFormat("开始进行设置\n");
		//for (auto &it : m_bShowMap)
		//{
		//	pTitltAdapter->SetItemChecked(it.first - SHead_CloseRPS520, it.second);
		//	pTitltAdapter->SetItemStr(it.first - SHead_CloseRPS520, m_titleMap[it.first]);
		//}

	}
	return 0;
}

void CDlgHeaderSelect::InitTilteMap()
{
	m_titleMap[SHead_CloseRPS520] = L"收盘价rps520";
	m_titleMap[SHead_CloseMACD520] = L"收盘价MACD520";
	m_titleMap[SHead_ClosePoint520] = L"收盘价520分数";
	m_titleMap[SHead_CloseRank520] = L"收盘价520排名";
	m_titleMap[SHead_CloseRPS2060] = L"收盘价rps2060";
	m_titleMap[SHead_CloseMACD2060] = L"收盘价MACD2060";
	m_titleMap[SHead_ClosePoint2060] = L"收盘价2060分数";
	m_titleMap[SHead_CloseRank2060] = L"收盘价2060排名";
	m_titleMap[SHead_AmountRPS520] = L"成交额rps520";
	m_titleMap[SHead_AmountMACD520] = L"成交额MACD520";
	m_titleMap[SHead_AmountPoint520] = L"成交额520分数";
	m_titleMap[SHead_AmountRank520] = L"成交额520排名";
	m_titleMap[SHead_AmountRPS2060] = L"成交额rps2060";
	m_titleMap[SHead_AmountMACD2060] = L"成交额MACD2060";
	m_titleMap[SHead_AmountPoint2060] = L"成交额2060分数";
	m_titleMap[SHead_AmountRank2060] = L"成交额2060排名";
	m_titleMap[SHead_AmountPoint] = L"成交额截面分数";
	m_titleMap[SHead_AmountRank] = L"成交额截面排名";
	m_titleMap[SHead_CAVol] = L"集合竞价成交量";
	m_titleMap[SHead_CAVolPoint] = L"集合竞价成交量分数";
	m_titleMap[SHead_CAVolRank] = L"集合竞价成交量排名";
	m_titleMap[SHead_CAAmo] = L"集合竞价成交额";
	m_titleMap[SHead_CAAmoPoint] = L"集合竞价成交额分数";
	m_titleMap[SHead_CAAmoRank] = L"集合竞价成交额排名";
	m_titleMap[SHead_ActBuySellRatio] = L"主动量比%";
	m_titleMap[SHead_ActToPasBuySellRatio] = L"转换主被动量比%";
	m_titleMap[SHead_AvgBuySellRatio] = L"平均量比%";
	m_titleMap[SHead_POCRatio] = L"最多成交价格比%";
	m_titleMap[SHead_Volume] = L"成交量";
	m_titleMap[SHead_ActSellVolume] = L"内盘";
	m_titleMap[SHead_ActBuyVolume] = L"外盘";
	m_titleMap[SHead_Open] = L"开盘";
	m_titleMap[SHead_High] = L"最高";
	m_titleMap[SHead_Low] = L"最低";
	m_titleMap[SHead_Amount] = L"成交额";

}

void CDlgHeaderSelect::InitTilteDscpMap()
{
	m_titleDscpMap[SHead_CloseRPS520] = L"收盘价rps520:\n"
		L"计算说明:\n"
		L"收盘价RPS520 = (EMA(C,5) - EMA(C,20)) / EMA(C,20) * 100";

	m_titleDscpMap[SHead_CloseMACD520] = L"收盘价MACD520:\n"
		L"计算说明:\n"
		L"收盘价MACD520 = (DIF-DEA)*2\n "
		L"DIF = EMA(C,5) - EMA(C,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_ClosePoint520] = L"收盘价520分数:\n"
		L"计算说明:\n"
		L"收盘价520分数 = (总股票数量 - 收盘价520排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_CloseRank520] = L"收盘价520排名:\n"
		L"计算说明:\n"
		L"收盘价520排名 = 对RPS520进行降序排列后的排名\n";

	m_titleDscpMap[SHead_CloseRPS2060] = L"收盘价rps2060:\n"
		L"计算说明:\n"
		L"收盘价RPS2060 = (EMA(C,5) - EMA(C,20)) / EMA(C,20) * 100\n";

	m_titleDscpMap[SHead_CloseMACD2060] = L"收盘价MACD2060:\n"
		L"计算说明:\n"
		L"收盘价MACD2060 = (DIF-DEA)*2\n "
		L"DIF = EMA(C,5) - EMA(C,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_ClosePoint2060] = L"收盘价2060分数:\n"
		L"计算说明:\n"
		L"收盘价2060分数 = (总股票数量 - 收盘价2060排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_CloseRank2060] = L"收盘价2060排名:\n"
		L"计算说明:\n"
		L"收盘价2060排名 = 对RPS2060进行降序排列后的排名\n";


	m_titleDscpMap[SHead_AmountRPS520] = L"成交额rps520:\n"
		L"计算说明:\n"
		L"成交额RPS520 = (EMA(AMO,5) - EMA(AMO,20)) / EMA(AMO,20) * 100\n";

	m_titleDscpMap[SHead_AmountMACD520] = L"成交额MACD520:\n"
		L"计算说明:\n"
		L"成交额MACD520 = (DIF-DEA)*2\n "
		L"DIF = EMA(AMO,5) - EMA(AMO,20)\n "
		L"DEA = EMA(DIF,10)\n";

	m_titleDscpMap[SHead_AmountPoint520] = L"成交额520分数:\n"
		L"计算说明:\n"
		L"成交额520分数 = (总股票数量 - 成交额520排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank520] = L"成交额520排名:\n"
		L"计算说明:\n"
		L"成交额520排名 = 对RPS520进行降序排列后的排名\n";

	m_titleDscpMap[SHead_AmountRPS2060] = L"成交额rps2060:\n"
		L"计算说明:\n"
		L"成交额RPS2060 = (EMA(AMO,5) - EMA(AMO,20)) / EMA(AMO,20) * 100\n";

	m_titleDscpMap[SHead_AmountMACD2060] = L"成交额MACD2060:\n"
		L"计算说明:\n"
		L"成交额MACD2060 = (DIF-DEA)*2\n "
		L"DIF = EMA(AMO,5) - EMA(AMO,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_AmountPoint2060] = L"成交额2060分数:\n"
		L"计算说明:\n"
		L"成交额2060分数 = (总股票数量 - 成交额2060排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank2060] = L"成交额2060排名:\n"
		L"计算说明:\n"
		L"成交额2060排名 = 对RPS2060进行降序排列后的排名\n";

	m_titleDscpMap[SHead_AmountPoint] = L"成交额截面分数:\n"
		L"计算说明:\n"
		L"成交额截面分数 = (总股票数量 - 成交额截面排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank] = L"成交额截面排名:\n"
		L"计算说明:\n"
		L"成交额截面排名 = 对成交额进行降序排列后的排名\n";

	m_titleDscpMap[SHead_CAVol] = L"集合竞价成交量:\n"
		L"计算说明:\n"
		L"集合竞价成交量 = 在集合竞价期间的成交成交量(单位:手)\n";

	m_titleDscpMap[SHead_CAVolPoint] = L"集合竞价成交量分数:\n"
		L"计算说明:\n"
		L"集合竞价成交量分数 = (总股票数量 - 集合竞价成交量排名) / (总股票数量 -1) * 100 \n";

	m_titleDscpMap[SHead_CAVolRank] = L"集合竞价成交量排名:\n"
		L"计算说明:\n"
		L"集合竞价成交量排名 = 对集合竞价成交量进行降序排列后的排名\n";

	m_titleDscpMap[SHead_CAAmo] = L"集合竞价成交额:\n"
		L"计算说明:\n"
		L"集合竞价成交额 = 在集合竞价期间的成交成交额(单位:元)\n";

	m_titleDscpMap[SHead_CAAmoPoint] = L"集合竞价成交额分数:\n"
		L"计算说明:\n"
		L"集合竞价成交额分数 = (总股票数额 - 集合竞价成交额排名) / (总股票数额 -1) * 100 \n";

	m_titleDscpMap[SHead_CAAmoRank] = L"集合竞价成交额排名:\n"
		L"计算说明:\n"
		L"集合竞价成交额排名 = 对集合竞价成交额进行降序排列后的排名\n";


	m_titleDscpMap[SHead_ActBuySellRatio] = L"主动量比%:\n"
		L"计算说明:\n"
		L"主动量比 = (主动买入量 - 主动卖出量) / (主动买入量 + 主动卖出量) * 100\n";

	m_titleDscpMap[SHead_ActToPasBuySellRatio] = L"转换主被动量比%:\n"
		L"计算说明:\n"
		L"转换主被动量比 = (转换买入量 - 转换卖出量) / (转换买入量 + 转换卖出量) * 100\n"
		L"转换买入量 = 主动买入量 / 主动买入笔数 * 被动卖出笔数\n"
		L"转换卖出量 = 主动卖出量 / 主动卖出笔数 * 被动买入笔数\n";

	m_titleDscpMap[SHead_AvgBuySellRatio] = L"平均量比%:\n"
		L"计算说明:\n"
		L"主动量比 = (平均主动买入量 - 平均主动卖出量) / (平均主动买入量 + 平均主动卖出量) * 100\n"
		L"平均主动买入量 = 主动买入量 / 主动买入笔数\n"
		L"平均主动卖出量 = 主动卖出量 / 主动卖出笔数\n";

	m_titleDscpMap[SHead_POCRatio] = L"最多成交价格比%:\n"
		L"计算说明:\n"
		L"最多成交价格比 = (成交量最多的价格 - 最新价格) / 最新价格 * 100\n";

	m_titleDscpMap[SHead_Volume] = L"成交量:\n"
		L"计算说明:\n"
		L"成交量 = 当前周期内的成交数量(单位:手)";

	m_titleDscpMap[SHead_ActSellVolume] = L"内盘:\n"
		L"计算说明:\n"
		L"内盘 = 当前周期内主动卖出的数量(单位:手)";

	m_titleDscpMap[SHead_ActBuyVolume] = L"外盘:\n"
		L"计算说明:\n"
		L"外盘 = 当前周期内主动买入的数量(单位:手)";

	m_titleDscpMap[SHead_Open] = L"开盘:\n"
		L"计算说明:\n"
		L"开盘 = 当前周期的开盘价格";

	m_titleDscpMap[SHead_High] = L"最高:\n"
		L"计算说明:\n"
		L"最高 = 当前周期的最高价格";

	m_titleDscpMap[SHead_Low] = L"最低:\n"
		L"计算说明:\n"
		L"最低 = 当前周期的最低价格";

	m_titleDscpMap[SHead_Amount] = L"成交额:\n"
		L"计算说明:\n"
		L"成交额 = 当前周期内的成交金额(单位:元)";

}

void CDlgHeaderSelect::OnBtnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CDlgHeaderSelect::OnBtnOK()
{
	CAdpterFixTitle* pAdpter = (CAdpterFixTitle*)m_pLvTitle->GetAdapter();
	m_bShowMap = pAdpter->GetTitleShowMap();
	::SendMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_ChangeShowTilte, (LPARAM)&m_bShowMap);
	OnBtnClose();
}

BOOL CDlgHeaderSelect::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	if (!m_pLvTitle->IsFocused())
		m_pLvTitle->SetFocus();
	return TRUE;
}

bool CDlgHeaderSelect::OnListTitleClicked(EventArgs * e)
{
	EventLVSelChanged *pEvt = (EventLVSelChanged*)e;
	int nSel = m_pLvTitle->GetSel();
	if (nSel < 0)
		return false;
	m_pEditDscp->SetWindowTextW(m_titleDscpMap[nSel + SHead_CloseRPS520]);
	return true;
}

void CDlgHeaderSelect::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
