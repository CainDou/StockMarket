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

BOOL SOUI::CDlgHeaderSelect::OnInitDialog(EventArgs * e)
{
	m_pLvTitle = FindChildByName2<SListView>(L"lv_title");
	InitTilteMap();
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

void SOUI::CDlgHeaderSelect::InitTilteMap()
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

void SOUI::CDlgHeaderSelect::OnBtnClose()
{
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgHeaderSelect::OnBtnOK()
{
	CAdpterFixTitle* pAdpter = (CAdpterFixTitle*)m_pLvTitle->GetAdapter();
	m_bShowMap = pAdpter->GetTitleShowMap();
	::SendMessage(m_hParWnd, WM_WINDOW_MSG, WDMsg_ChangeShowTilte, (LPARAM)&m_bShowMap);
	OnBtnClose();
}

BOOL SOUI::CDlgHeaderSelect::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SetMsgHandled(FALSE);
	if (!m_pLvTitle->IsFocused())
		m_pLvTitle->SetFocus();
	return TRUE;
}

void SOUI::CDlgHeaderSelect::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
