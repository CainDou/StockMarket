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
		//把pbtn的指针放到SRealWnd的Data中保存，以便在窗口destroy时释放pbtn对象。
		pRealWnd->SetData(wndDlg);
		//返回成功创建后的窗口句柄
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

//不处理，返回FALSE
BOOL CSouiRealWndHandler::OnRealWndSize(SRealWnd *pRealWnd)
{
	return FALSE;
}

//不处理，返回FALSE
BOOL CSouiRealWndHandler::OnRealWndInit(SRealWnd *pRealWnd)
{
	return FALSE;
}