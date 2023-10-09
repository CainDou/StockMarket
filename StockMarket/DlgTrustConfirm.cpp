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
		str.Format(L"交易信息:\n股票代码:%s\n股票名称:%s\n方向:%s\n委托价格:%.02f\n委托数量:%lld\n确定要进行委托吗？",
			StrA2StrW(m_trustInfo.SecurityID),
			StrA2StrW(m_trustInfo.SecurityName),
			m_trustInfo.Direct == eTD_Buy ? L"买入" : L"卖出",
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

