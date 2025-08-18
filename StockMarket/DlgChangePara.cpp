#include "stdafx.h"
#include "DlgChangePara.h"
#include "IniFile.h"



SOUI::CDlgChangePara::CDlgChangePara(TargetInfo & targetInfo) :SHostWnd(_T("LAYOUT:dlg_changeTargetPara"))
{
	ti = targetInfo;
	m_pWindowPara = nullptr;
}

SOUI::CDlgChangePara::CDlgChangePara(TargetInfo & targetInfo, HWND hParWnd) : SHostWnd(_T("LAYOUT:dlg_changeTargetPara"))
{
	ti = targetInfo;
	m_hParWnd = hParWnd;
	m_pWindowPara = nullptr;
}

CDlgChangePara::~CDlgChangePara()
{
	//if (m_pWindowPara)
	//{
	//	for (auto &it : m_pTextVec)
	//		m_pWindowPara->DestroyChild(it);
	//	for (auto &it : m_pEditVec)
	//		m_pWindowPara->DestroyChild(it);

	//}
}

void SOUI::CDlgChangePara::OnClose()
{
	::EnableWindow(m_hParWnd, TRUE);
	CSimpleWnd::DestroyWindow();
}

void SOUI::CDlgChangePara::OnClickButtonOk()
{
	std::vector<int>paraVec;
	for (int i = 0; i < ti.strParaName.size(); ++i)
	{
		SStringW strTmp = m_pEditVec[i]->GetWindowTextW();
		int nPara = !strTmp.IsEmpty() ? _wtoi(strTmp) : -1;
		paraVec.emplace_back(nPara);
	}
	::SendMessage(m_hParWnd, WM_KLINE_MSG, (WPARAM)&paraVec, KLINEMSG_CHANGEPARA);
	OnClose();
}

void SOUI::CDlgChangePara::OnClickButtonCancel()
{
	OnClose();
}

void SOUI::CDlgChangePara::OnInit(EventArgs * e)
{
	CIniFile ini(".//config//config.ini");
	SStringA strSection = "DefaultPara";
	SStringA strKey;
	SStringA strPara = ini.GetStringA(strSection, ti.strTargetName, "");
	if (!strPara.IsEmpty())
	{
		SStringW strTmp;
		ti.nParaDefValue.clear();
		for (int j = 0; j < strPara.GetLength(); ++j)
		{
			if (strPara[j] != ',')
				strTmp += strPara[j];
			else
			{
				ti.nParaDefValue.emplace_back(_wtoi(strTmp));
				strTmp.Empty();
			}
		}
		if (!strTmp.IsEmpty())
			ti.nParaDefValue.emplace_back(_wtoi(strTmp));
	}

	m_pWindowPara = FindChildByName2<SWindow>(L"wnd_para");
	if (m_pWindowPara)
	{
		if (ti.strParaName.empty())
		{
			SStatic* pText = new SStatic;
			SApplication::getSingleton().SetSwndDefAttr(pText);
			m_pWindowPara->InsertChild(pText);
			pText->SSendMessage(WM_CREATE);
			pText->SetWindowTextW(L"无需要设置的参数");

			m_pTextVec.emplace_back(pText);
			SButton*pBtnDef = FindChildByName2<SButton>(L"btn_Default");
			pBtnDef->EnableWindow(FALSE, TRUE);
			SButton*pBtnSetDef = FindChildByName2<SButton>(L"btn_SetDefault");
			pBtnSetDef->EnableWindow(FALSE, TRUE);

		}
		else
		{
			for (int i = 0; i < ti.strParaName.size(); ++i)
			{
				SStatic* pText = new SStatic;
				SApplication::getSingleton().SetSwndDefAttr(pText);
				m_pWindowPara->InsertChild(pText);
				pText->SSendMessage(WM_CREATE);
				int nLeft = 27 + i % 3 * 110;
				int nTop = 10 + i / 3 * 40;
				SStringW tmp;
				pText->SetAttribute(L"pos", tmp.Format(L"%d,%d", nLeft, nTop));
				pText->SetAttribute(L"size", L"30,24");
				pText->SetWindowTextW(StrA2StrW(ti.strParaName[i]));
				m_pTextVec.emplace_back(pText);
				SEdit* pEdit = new SEdit();
				SApplication::getSingleton().SetSwndDefAttr(pEdit);
				m_pWindowPara->InsertChild(pEdit);
				pEdit->SSendMessage(WM_CREATE);
				nLeft = nLeft + 30;
				pEdit->SetAttribute(L"pos", tmp.Format(L"%d,%d", nLeft, nTop));
				pEdit->SetAttribute(L"size", L"48,24");
				pEdit->SetAttribute(L"colorBkgnd", L"#FFFFFF");
				pEdit->SetAttribute(L"colorText", L"#000000");
				pEdit->SetWindowTextW(tmp.Format(L"%d", ti.nUsePara[i]));
				m_pEditVec.emplace_back(pEdit);
			}
		}
	}
}

void SOUI::CDlgChangePara::OnClickButtonDefault()
{
	SStringW strTmp;
	for (int i = 0; i < ti.nParaDefValue.size(); ++i)
		m_pEditVec[i]->SetWindowTextW(strTmp.Format(L"%d", ti.nParaDefValue[i]));
}

void SOUI::CDlgChangePara::OnClickButtonSetDefault()
{
	SStringW strTmp;
	std::vector<int> tmpPara;
	for (int i = 0; i < ti.strParaName.size(); ++i)
	{
		SStringW strPara = m_pEditVec[i]->GetWindowTextW();
		if (strPara != L"")
			tmpPara.emplace_back(_wtoi(strPara));
		else
		{
			SMessageBox(NULL, strTmp.Format(L"请输入参数%s的值", StrA2StrW(ti.strParaName[i])),
				L"错误", MB_ICONWARNING);
			return;
		}
	}
	ti.nParaDefValue = tmpPara;

	CIniFile ini(L".\\config\\config.ini");
	SStringW strRes;
	strTmp.Empty();
	for (int i = 0; i < tmpPara.size(); ++i)
	{
		strRes += strTmp.Format(L"%d", tmpPara[i]);
		if (i != tmpPara.size() - 1)
			strRes += ',';
	}
	ini.WriteString(L"DefaultPara", StrA2StrW(ti.strTargetName), strRes);

	::PostMessageW(m_hParWnd, WM_KLINE_MSG, (WPARAM)ti.nTargetIndex, KLINEMSG_CHANGEDEFAULTPARA);
	SMessageBox(NULL, L"设置默认参数成功", L"通知", MB_OK);

}

void SOUI::CDlgChangePara::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
