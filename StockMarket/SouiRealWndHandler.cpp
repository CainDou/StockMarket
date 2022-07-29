#include "stdafx.h"
#include "SouiRealWndHandler.h"
#include "WorkWnd.h"


CSouiRealWndHandler::CSouiRealWndHandler()
{
}


CSouiRealWndHandler::~CSouiRealWndHandler()
{
}

HWND CSouiRealWndHandler::OnRealWndCreate(SRealWnd *pRealWnd)
{
	const SRealWndParam &param = pRealWnd->GetRealWndParam();
	if (param.m_strClassName == L"workwnd")
	{
		CWorkWnd *wndDlg = new CWorkWnd();
		wndDlg->Create(pRealWnd->GetContainer()->GetHostHwnd(),
			WS_CHILD | WS_VISIBLE /*| BS_PUSHBUTTON*/, 0, 0, 0, 0, 0);
		//��pbtn��ָ��ŵ�SRealWnd��Data�б��棬�Ա��ڴ���destroyʱ�ͷ�pbtn����
		pRealWnd->SetData(wndDlg);
		//���سɹ�������Ĵ��ھ��
		return wndDlg->m_hWnd;
	}
	else
	{
		return 0;
	}
}

void CSouiRealWndHandler::OnRealWndDestroy(SRealWnd *pRealWnd)
{
	const SRealWndParam &param = pRealWnd->GetRealWndParam();
	if (param.m_strClassName == L"workwnd")
	{
		CWorkWnd *pbtn = (CWorkWnd*)pRealWnd->GetData();
		if (pbtn)
		{
			pbtn->DestroyWindow();
			delete pbtn;
		}
	}

}

//����������FALSE
BOOL CSouiRealWndHandler::OnRealWndSize(SRealWnd *pRealWnd)
{
	return FALSE;
}

//����������FALSE
BOOL CSouiRealWndHandler::OnRealWndInit(SRealWnd *pRealWnd)
{
	return FALSE;
}