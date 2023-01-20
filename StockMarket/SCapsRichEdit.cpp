#include "stdafx.h"
#include "SCapsRichEdit.h"
//#include <Imm.h>

#pragma comment(lib,"imm32.lib")

#define CTRL(_ch) (_ch - 'A' + 1)
SCapsRichEdit::SCapsRichEdit()
{
	m_bDirty = FALSE;
	m_bImm = FALSE;
}

SCapsRichEdit::~SCapsRichEdit()
{
}

BOOL SOUI::SCapsRichEdit::GetCharFormat(CHARFORMAT2W * pcf)
{
	*pcf = m_cfDef;
	return TRUE;
}

CHARFORMAT2W SOUI::SCapsRichEdit::SetCharFormat(CHARFORMAT2W * pcf)
{
	CHARFORMAT2W oldCf = m_cfDef;
	m_cfDef = *pcf;
	m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
	return oldCf;
}

BOOL SOUI::SCapsRichEdit::InsertStr(SStringW str)
{
	m_bDirty = TRUE;
	m_bImm = FALSE;
	ReplaceSel(str);
	EventRENotify evt(this);
	evt.iNotify = EN_CHANGE;
	FireEvent(evt);
	return 0;
}


void SOUI::SCapsRichEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HIMC hImc = ImmGetContext(GetContainer()->GetHostHwnd());
	DWORD dwCharCount = 1;
	dwCharCount = ImmGetCompositionString(hImc, GCS_COMPSTR, NULL, 0l);
	ImmReleaseContext(GetContainer()->GetHostHwnd(), hImc);
	if (dwCharCount > 0)
		m_bImm = TRUE;
	else
		m_bImm = FALSE;
	m_bDirty = TRUE;
	EventRENotify evt(this);
	evt.iNotify = EN_CHANGE;
	if (!m_bImm)
	{
		SHORT sState = ::GetKeyState(VK_CONTROL);
		if (HIBYTE(sState) == 0xff)
		{
			if(nChar=='V')
				FireEvent(evt);
		}
		else
		{
			if ((nChar >= VK_NUMPAD0 && nChar <= VK_DIVIDE)
				//||(nChar >='0'&& nChar <'9')
				|| (nChar >= VK_OEM_1 && nChar <= VK_OEM_8)
				|| nChar == VK_BACK || nChar == VK_RETURN
				|| isalnum(nChar) || nChar == VK_SPACE
				|| nChar == VK_PROCESSKEY)
				FireEvent(evt);
		}
	}
	else
		FireEvent(evt);

}



SOUI::SCapsEdit::SCapsEdit()
{
}

SOUI::SCapsEdit::~SCapsEdit()
{
}

void SOUI::SCapsEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		// Ctrl-Return generates Ctrl-J (LF), treat it as an ordinary return
	case CTRL('J'):
	case VK_RETURN:
		if (!(GetKeyState(VK_CONTROL) & 0x8000)
			&& !(m_dwStyle & ES_WANTRETURN))
			return;
		break;

	case VK_TAB:
		if (!m_fWantTab && !(GetKeyState(VK_CONTROL) & 0x8000))
			return;
		break;
	default:
		if (m_dwStyle&ES_NUMBER && !isdigit(nChar) && nChar != '-' && nChar != '.' && nChar != ',')
			return;
#ifndef _UNICODE
		if (m_byDbcsLeadByte == 0)
		{
			if (IsDBCSLeadByte(nChar))
			{
				m_byDbcsLeadByte = nChar;
				return;
			}
		}
		else
		{
			nChar = MAKEWORD(nChar, m_byDbcsLeadByte);
			m_pTxtHost->GetTextService()->TxSendMessage(WM_IME_CHAR, nChar, 0, NULL);
			m_byDbcsLeadByte = 0;
			return;
		}
#endif//_UNICODE
		break;
	}
	if (nChar >= 'a' && nChar <= 'z')
	{
		nChar -= 32;
		m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, nChar, GetCurMsg()->lParam, NULL);
	}
	else
		m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
}

void SOUI::SCapsEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//输入法下回车
	if (nChar == VK_PROCESSKEY &&
		((GetCurMsg()->lParam & 0xFF0000) >> 16) == 0x1c)
	{
		DWORD dwStart = 0, dwEnd = 0;
		SSendMessage(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		SStringW str = GetWindowTextW();
		SetWindowTextW(str.MakeUpper());
		SSendMessage(EM_SETSEL, dwStart, dwEnd);
	}

}
