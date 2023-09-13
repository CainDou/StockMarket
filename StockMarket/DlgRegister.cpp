#include "stdafx.h"
#include "DlgRegister.h"
#include "WndSynHandler.h"

extern CWndSynHandler g_WndSyn;

CDlgRegister::CDlgRegister(HWND hParWnd, eAccountOpt optType) :SHostWnd(_T("LAYOUT:dlg_register"))
{
	m_hParWnd = hParWnd;
	m_optType = optType;
}


CDlgRegister::~CDlgRegister()
{
}

void CDlgRegister::OnBtnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void CDlgRegister::OnBtnOK()
{

	SStringA strSendInfo;
	if (m_optType == eAO_Register)
	{
		if (!m_bIDValid)
		{
			SMessageBox(m_hWnd, L"账号不符合要求！", L"错误", MB_OK);
			m_pEditID->SetFocus();
			return;
		}

		if (!m_bPsdValid)
		{
			SMessageBox(m_hWnd, L"密码不符合要求！", L"错误", MB_OK);
			m_pEditPsd->SetFocus();
			return;
		}

		if (!m_bPsdCfmSame)
		{
			SMessageBox(m_hWnd, L"两次密码不一样！", L"错误", MB_OK);
			m_pEditPsdCfm->SetFocus();
			return;
		}

		SStringA strID = StrW2StrA(m_pEditID->GetWindowTextW());
		SStringA strPsd = StrW2StrA(m_pEditPsd->GetWindowTextW());
		strSendInfo.Format("ID:%s,Psd:%s;", strID, strPsd);
		SendMsg(m_uSynThreadID, TradeSyn_Register,
			strSendInfo.GetBuffer(0), strSendInfo.GetLength()+1);

	}
	else
	{
		if (!m_bPsdValid)
		{
			SMessageBox(m_hWnd, L"密码不符合要求！", L"错误", MB_OK);
			m_pEditPsd->SetFocus();
			return;
		}

		if (!m_bPsdCfmSame)
		{
			SMessageBox(m_hWnd, L"两次密码不一样！", L"错误", MB_OK);
			m_pEditPsdCfm->SetFocus();
			return;
		}
		SStringA strID = StrW2StrA(m_pEditID->GetWindowTextW());
		SStringA strSrcPsd = StrW2StrA(m_pEditSrcPsd->GetWindowTextW());
		SStringA strPsd = StrW2StrA(m_pEditPsd->GetWindowTextW());
		strSendInfo.Format("ID:%s,SrcPsd:%s,Psd:%s;", strID, strSrcPsd, strPsd);
		SendMsg(m_uSynThreadID, TradeSyn_ChangePsd,
			strSendInfo.GetBuffer(0), strSendInfo.GetLength() + 1);

	}

}

void CDlgRegister::OnInit(EventArgs * e)
{
	m_pEditID = FindChildByName2<SEdit>(L"edit_ID");
	m_pEditSrcPsd = FindChildByName2<SEdit>(L"edit_SrcPsd");
	m_pEditPsd = FindChildByName2<SEdit>(L"edit_Psd");
	m_pEditPsdCfm = FindChildByName2<SEdit>(L"edit_PsdConfirm");
	m_pTxtIDInfo = FindChildByName2<SStatic>(L"txt_IDInfo");
	m_pTxtSrcPsdInfo = FindChildByName2<SStatic>(L"txt_SrcPsdInfo");
	m_pTxtPsdInfo = FindChildByName2<SStatic>(L"txt_PsdInfo");
	m_pTxtPsdCfmInfo = FindChildByName2<SStatic>(L"txt_PsdCfmInfo");
	m_pTxtSrcPsd = FindChildByName2<SStatic>(L"txt_srcPsd");
	m_pTxtPsd = FindChildByName2<SStatic>(L"txt_psd");
	m_pTitle = FindChildByName2<SStatic>(L"txt_title");

	m_pEditID->GetEventSet()->subscribeEvent(EventRENotify::EventID,
		Subscriber(&CDlgRegister::OnEditIDChange, this));

	m_pEditSrcPsd->GetEventSet()->subscribeEvent(EventRENotify::EventID,
		Subscriber(&CDlgRegister::OnEditSrcPsdChange, this));

	m_pEditPsd->GetEventSet()->subscribeEvent(EventRENotify::EventID,
		Subscriber(&CDlgRegister::OnEditPsdChange, this));

	m_pEditPsdCfm->GetEventSet()->subscribeEvent(EventRENotify::EventID,
		Subscriber(&CDlgRegister::OnEditPsdCfmChange, this));


	if (m_optType == eAO_Register)
	{
		m_pTxtSrcPsd->SetAttribute(L"pos", L"10,[0,@0,@0");
		m_pEditSrcPsd->SetAttribute(L"pos", L"[5,{0,@0,@0");
		m_pTxtSrcPsdInfo->SetAttribute(L"pos", L"[5,{0,@0,@0");
	}
	else if (m_optType == eAO_ChangePsd)
	{
		m_pTitle->SetWindowTextW(L"修改密码");
		m_pTxtPsd->SetWindowTextW(L"新密码");
	}
	m_uSynThreadID = g_WndSyn.GetTradeThreadID();
	g_WndSyn.SetAccountWnd(m_hWnd);
}

bool CDlgRegister::OnEditIDChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	if (m_optType == eAO_Register)
	{
		bool bValid = true;
		SStringW str = pEdit->GetWindowTextW();
		SStringA strID = StrW2StrA(str);
		if (str.IsEmpty())
		{
			m_pTxtIDInfo->SetAttribute(L"colorText", L"#000000FF");
			m_pTxtIDInfo->SetWindowTextW(L"8-16位字母或数字");

		}
		if (str.GetLength() < 8)
		{
			bValid = false;
			m_pTxtIDInfo->SetAttribute(L"colorText", L"#FF0000FF");
			m_pTxtIDInfo->SetWindowTextW(L"账号长度过短");
		}
		for (int i = 0; i < str.GetLength(); ++i)
		{
			if (str[i] != '!' && (str[i]<'#' || str[i] >'~'))
			{
				bValid = false;
				m_pTxtIDInfo->SetAttribute(L"colorText", L"#FF0000FF");
				m_pTxtIDInfo->SetWindowTextW(L"输入字符不符合要求");
				break;
			}
		}
		m_bIDValid = bValid;
		if (m_bIDValid)
		{
			m_pTxtIDInfo->SetAttribute(L"colorText", L"#00FF00FF");
			m_pTxtIDInfo->SetWindowTextW(L"账号符合规则");
		}
	}
	return true;
}

bool CDlgRegister::OnEditSrcPsdChange(EventArgs * e)
{
	return true;
}

bool CDlgRegister::OnEditPsdChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	bool bValid = true;
	SStringW str = pEdit->GetWindowTextW();
	SStringA strID = StrW2StrA(str);
	if (str.IsEmpty())
	{
		m_pTxtPsdInfo->SetAttribute(L"colorText", L"#000000FF");
		m_pTxtPsdInfo->SetWindowTextW(L"8-16位字母、数字、符号");
		return true;
	}
	if (str.GetLength() < 8)
	{
		bValid = false;
		m_pTxtPsdInfo->SetAttribute(L"colorText", L"#FF0000FF");
		m_pTxtPsdInfo->SetWindowTextW(L"密码长度过短");
	}
	for (int i = 0; i < str.GetLength(); ++i)
	{
		if (!isalnum(str[i]))
		{
			bValid = false;
			m_pTxtPsdInfo->SetAttribute(L"colorText", L"#FF0000FF");
			m_pTxtPsdInfo->SetWindowTextW(L"输入字符不符合要求");
			break;
		}
	}
	m_bPsdValid = bValid;
	if (m_bPsdValid)
	{
		m_pTxtPsdInfo->SetAttribute(L"colorText", L"#00FF00FF");
		m_pTxtPsdInfo->SetWindowTextW(L"可以使用该密码");

	}

	return true;
}

bool CDlgRegister::OnEditPsdCfmChange(EventArgs * e)
{
	EventRENotify* pEvt = (EventRENotify*)e;
	SEdit* pEdit = (SEdit*)pEvt->sender;
	if (pEvt->iNotify != EN_CHANGE)
		return true;
	bool bValid = true;
	SStringW str = pEdit->GetWindowTextW();
	SStringA strPsdCfm = StrW2StrA(str);
	if (str.IsEmpty())
	{
		m_pTxtPsdCfmInfo->SetAttribute(L"colorText", L"#000000FF");
		m_pTxtPsdCfmInfo->SetWindowTextW(L"8-16位字母、数字、符号");
		return true;
	}
	SStringA strPsd = StrW2StrA(m_pEditPsd->GetWindowTextW());
	if (strPsd != strPsdCfm)
	{
		m_bPsdCfmSame = false;
		m_pTxtPsdCfmInfo->SetAttribute(L"colorText", L"#FF0000FF");
		m_pTxtPsdCfmInfo->SetWindowTextW(L"两次密码不同！");
		return true;
	}
	m_bPsdCfmSame = true;
	m_pTxtPsdCfmInfo->SetAttribute(L"colorText", L"#00FF00FF");
	m_pTxtPsdCfmInfo->SetWindowTextW(L"两次密码相同!");

	return true;
}

void CDlgRegister::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

LRESULT CDlgRegister::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int nFeedback = (int)wp;

	if (lp == TAMsg_Register)
	{
		if (AFB_RegisterSuccess == nFeedback)
			SMessageBox(m_hWnd, L"注册成功!", L"提示", MB_OK | MB_ICONINFORMATION);
		else if (AFB_AccountExist == nFeedback)
			SMessageBox(m_hWnd, L"当前账号已存在!", L"提示", MB_OK | MB_ICONINFORMATION);
	}
	else if(lp == TAMsg_ChangePsd)
	{
		if (AFB_ChangePsdSuccess == nFeedback)
			SMessageBox(m_hWnd, L"密码修改成功!", L"提示", MB_OK | MB_ICONINFORMATION);
		else if (AFB_NotPair == nFeedback)
			SMessageBox(m_hWnd, L"账号密码不匹配，密码修改失败!", L"提示", MB_OK | MB_ICONINFORMATION);

	}
	return 0;
}
