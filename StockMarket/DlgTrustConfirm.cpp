#include "stdafx.h"
#include "DlgTrustConfirm.h"


CDlgTrustConfirm::CDlgTrustConfirm(TrustInfo trustInfo) :SHostDialog(_T("LAYOUT:dlg_tradeConfirm"))
{
	m_trustInfo = trustInfo;
}


CDlgTrustConfirm::~CDlgTrustConfirm()
{
}

void CDlgTrustConfirm::OnInit(EventArgs * e)
{
	SStatic *pText = FindChildByName2<SStatic>(L"txt_trade");
	if (pText)
	{
		SStringW str;
		str.Format(L"������Ϣ:\n��Ʊ����:%s\n��Ʊ����:%s\n����:%s\nί�м۸�:%.02f\nί������:%lld\nȷ��Ҫ����ί����",
			StrA2StrW(m_trustInfo.SecurityID),
			StrA2StrW(m_trustInfo.SecurityName),
			m_trustInfo.Direct == eTD_Buy ? L"����" : L"����",
			m_trustInfo.TrustPrice.GetDouble(),
			m_trustInfo.TrustVol.data);
		pText->SetWindowTextW(str);
	}
}

void CDlgTrustConfirm::OnBtnOK()
{
	EndDialog(IDOK);
}

void CDlgTrustConfirm::OnBtnCancel()
{
	EndDialog(IDCANCEL);
}

