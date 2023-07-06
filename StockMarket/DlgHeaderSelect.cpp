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
		//OutputDebugStringFormat("��ʼ��������\n");
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
	m_titleMap[SHead_CloseRPS520] = L"���̼�rps520";
	m_titleMap[SHead_CloseMACD520] = L"���̼�MACD520";
	m_titleMap[SHead_ClosePoint520] = L"���̼�520����";
	m_titleMap[SHead_CloseRank520] = L"���̼�520����";
	m_titleMap[SHead_CloseRPS2060] = L"���̼�rps2060";
	m_titleMap[SHead_CloseMACD2060] = L"���̼�MACD2060";
	m_titleMap[SHead_ClosePoint2060] = L"���̼�2060����";
	m_titleMap[SHead_CloseRank2060] = L"���̼�2060����";
	m_titleMap[SHead_AmountRPS520] = L"�ɽ���rps520";
	m_titleMap[SHead_AmountMACD520] = L"�ɽ���MACD520";
	m_titleMap[SHead_AmountPoint520] = L"�ɽ���520����";
	m_titleMap[SHead_AmountRank520] = L"�ɽ���520����";
	m_titleMap[SHead_AmountRPS2060] = L"�ɽ���rps2060";
	m_titleMap[SHead_AmountMACD2060] = L"�ɽ���MACD2060";
	m_titleMap[SHead_AmountPoint2060] = L"�ɽ���2060����";
	m_titleMap[SHead_AmountRank2060] = L"�ɽ���2060����";
	m_titleMap[SHead_AmountPoint] = L"�ɽ���������";
	m_titleMap[SHead_AmountRank] = L"�ɽ����������";
	m_titleMap[SHead_CAVol] = L"���Ͼ��۳ɽ���";
	m_titleMap[SHead_CAVolPoint] = L"���Ͼ��۳ɽ�������";
	m_titleMap[SHead_CAVolRank] = L"���Ͼ��۳ɽ�������";
	m_titleMap[SHead_CAAmo] = L"���Ͼ��۳ɽ���";
	m_titleMap[SHead_CAAmoPoint] = L"���Ͼ��۳ɽ������";
	m_titleMap[SHead_CAAmoRank] = L"���Ͼ��۳ɽ�������";
	m_titleMap[SHead_ActBuySellRatio] = L"��������%";
	m_titleMap[SHead_ActToPasBuySellRatio] = L"ת������������%";
	m_titleMap[SHead_AvgBuySellRatio] = L"ƽ������%";
	m_titleMap[SHead_POCRatio] = L"���ɽ��۸��%";
	m_titleMap[SHead_Volume] = L"�ɽ���";
	m_titleMap[SHead_ActSellVolume] = L"����";
	m_titleMap[SHead_ActBuyVolume] = L"����";
	m_titleMap[SHead_Open] = L"����";
	m_titleMap[SHead_High] = L"���";
	m_titleMap[SHead_Low] = L"���";
	m_titleMap[SHead_Amount] = L"�ɽ���";

}

void CDlgHeaderSelect::InitTilteDscpMap()
{
	m_titleDscpMap[SHead_CloseRPS520] = L"���̼�rps520:\n"
		L"����˵��:\n"
		L"���̼�RPS520 = (EMA(C,5) - EMA(C,20)) / EMA(C,20) * 100";

	m_titleDscpMap[SHead_CloseMACD520] = L"���̼�MACD520:\n"
		L"����˵��:\n"
		L"���̼�MACD520 = (DIF-DEA)*2\n "
		L"DIF = EMA(C,5) - EMA(C,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_ClosePoint520] = L"���̼�520����:\n"
		L"����˵��:\n"
		L"���̼�520���� = (�ܹ�Ʊ���� - ���̼�520����) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_CloseRank520] = L"���̼�520����:\n"
		L"����˵��:\n"
		L"���̼�520���� = ��RPS520���н������к������\n";

	m_titleDscpMap[SHead_CloseRPS2060] = L"���̼�rps2060:\n"
		L"����˵��:\n"
		L"���̼�RPS2060 = (EMA(C,5) - EMA(C,20)) / EMA(C,20) * 100\n";

	m_titleDscpMap[SHead_CloseMACD2060] = L"���̼�MACD2060:\n"
		L"����˵��:\n"
		L"���̼�MACD2060 = (DIF-DEA)*2\n "
		L"DIF = EMA(C,5) - EMA(C,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_ClosePoint2060] = L"���̼�2060����:\n"
		L"����˵��:\n"
		L"���̼�2060���� = (�ܹ�Ʊ���� - ���̼�2060����) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_CloseRank2060] = L"���̼�2060����:\n"
		L"����˵��:\n"
		L"���̼�2060���� = ��RPS2060���н������к������\n";


	m_titleDscpMap[SHead_AmountRPS520] = L"�ɽ���rps520:\n"
		L"����˵��:\n"
		L"�ɽ���RPS520 = (EMA(AMO,5) - EMA(AMO,20)) / EMA(AMO,20) * 100\n";

	m_titleDscpMap[SHead_AmountMACD520] = L"�ɽ���MACD520:\n"
		L"����˵��:\n"
		L"�ɽ���MACD520 = (DIF-DEA)*2\n "
		L"DIF = EMA(AMO,5) - EMA(AMO,20)\n "
		L"DEA = EMA(DIF,10)\n";

	m_titleDscpMap[SHead_AmountPoint520] = L"�ɽ���520����:\n"
		L"����˵��:\n"
		L"�ɽ���520���� = (�ܹ�Ʊ���� - �ɽ���520����) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank520] = L"�ɽ���520����:\n"
		L"����˵��:\n"
		L"�ɽ���520���� = ��RPS520���н������к������\n";

	m_titleDscpMap[SHead_AmountRPS2060] = L"�ɽ���rps2060:\n"
		L"����˵��:\n"
		L"�ɽ���RPS2060 = (EMA(AMO,5) - EMA(AMO,20)) / EMA(AMO,20) * 100\n";

	m_titleDscpMap[SHead_AmountMACD2060] = L"�ɽ���MACD2060:\n"
		L"����˵��:\n"
		L"�ɽ���MACD2060 = (DIF-DEA)*2\n "
		L"DIF = EMA(AMO,5) - EMA(AMO,20)\n "
		L"DEA = EMA(DIF,10)\n ";

	m_titleDscpMap[SHead_AmountPoint2060] = L"�ɽ���2060����:\n"
		L"����˵��:\n"
		L"�ɽ���2060���� = (�ܹ�Ʊ���� - �ɽ���2060����) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank2060] = L"�ɽ���2060����:\n"
		L"����˵��:\n"
		L"�ɽ���2060���� = ��RPS2060���н������к������\n";

	m_titleDscpMap[SHead_AmountPoint] = L"�ɽ���������:\n"
		L"����˵��:\n"
		L"�ɽ��������� = (�ܹ�Ʊ���� - �ɽ����������) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_AmountRank] = L"�ɽ����������:\n"
		L"����˵��:\n"
		L"�ɽ���������� = �Գɽ�����н������к������\n";

	m_titleDscpMap[SHead_CAVol] = L"���Ͼ��۳ɽ���:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ��� = �ڼ��Ͼ����ڼ�ĳɽ��ɽ���(��λ:��)\n";

	m_titleDscpMap[SHead_CAVolPoint] = L"���Ͼ��۳ɽ�������:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ������� = (�ܹ�Ʊ���� - ���Ͼ��۳ɽ�������) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_CAVolRank] = L"���Ͼ��۳ɽ�������:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ������� = �Լ��Ͼ��۳ɽ������н������к������\n";

	m_titleDscpMap[SHead_CAAmo] = L"���Ͼ��۳ɽ���:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ��� = �ڼ��Ͼ����ڼ�ĳɽ��ɽ���(��λ:Ԫ)\n";

	m_titleDscpMap[SHead_CAAmoPoint] = L"���Ͼ��۳ɽ������:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ������ = (�ܹ�Ʊ���� - ���Ͼ��۳ɽ�������) / (�ܹ�Ʊ���� -1) * 100 \n";

	m_titleDscpMap[SHead_CAAmoRank] = L"���Ͼ��۳ɽ�������:\n"
		L"����˵��:\n"
		L"���Ͼ��۳ɽ������� = �Լ��Ͼ��۳ɽ�����н������к������\n";


	m_titleDscpMap[SHead_ActBuySellRatio] = L"��������%:\n"
		L"����˵��:\n"
		L"�������� = (���������� - ����������) / (���������� + ����������) * 100\n";

	m_titleDscpMap[SHead_ActToPasBuySellRatio] = L"ת������������%:\n"
		L"����˵��:\n"
		L"ת������������ = (ת�������� - ת��������) / (ת�������� + ת��������) * 100\n"
		L"ת�������� = ���������� / ����������� * ������������\n"
		L"ת�������� = ���������� / ������������ * �����������\n";

	m_titleDscpMap[SHead_AvgBuySellRatio] = L"ƽ������%:\n"
		L"����˵��:\n"
		L"�������� = (ƽ������������ - ƽ������������) / (ƽ������������ + ƽ������������) * 100\n"
		L"ƽ������������ = ���������� / �����������\n"
		L"ƽ������������ = ���������� / ������������\n";

	m_titleDscpMap[SHead_POCRatio] = L"���ɽ��۸��%:\n"
		L"����˵��:\n"
		L"���ɽ��۸�� = (�ɽ������ļ۸� - ���¼۸�) / ���¼۸� * 100\n";

	m_titleDscpMap[SHead_Volume] = L"�ɽ���:\n"
		L"����˵��:\n"
		L"�ɽ��� = ��ǰ�����ڵĳɽ�����(��λ:��)";

	m_titleDscpMap[SHead_ActSellVolume] = L"����:\n"
		L"����˵��:\n"
		L"���� = ��ǰ��������������������(��λ:��)";

	m_titleDscpMap[SHead_ActBuyVolume] = L"����:\n"
		L"����˵��:\n"
		L"���� = ��ǰ�������������������(��λ:��)";

	m_titleDscpMap[SHead_Open] = L"����:\n"
		L"����˵��:\n"
		L"���� = ��ǰ���ڵĿ��̼۸�";

	m_titleDscpMap[SHead_High] = L"���:\n"
		L"����˵��:\n"
		L"��� = ��ǰ���ڵ���߼۸�";

	m_titleDscpMap[SHead_Low] = L"���:\n"
		L"����˵��:\n"
		L"��� = ��ǰ���ڵ���ͼ۸�";

	m_titleDscpMap[SHead_Amount] = L"�ɽ���:\n"
		L"����˵��:\n"
		L"�ɽ��� = ��ǰ�����ڵĳɽ����(��λ:Ԫ)";

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
