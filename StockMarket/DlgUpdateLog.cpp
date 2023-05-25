#include "stdafx.h"
#include "DlgUpdateLog.h"


CDlgUpdateLog::CDlgUpdateLog():SHostWnd(_T("LAYOUT:dlg_updateLog"))
{
}


CDlgUpdateLog::~CDlgUpdateLog()
{
}

BOOL CDlgUpdateLog::OnInitDialog(EventArgs * e)
{
	m_pLogEdit = FindChildByName2<SRichEdit>(L"re_Log");
	UpdateLog();
	return 0;
}

void CDlgUpdateLog::OnBtnOK()
{
	CSimpleWnd::DestroyWindow();
}

void CDlgUpdateLog::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void CDlgUpdateLog::UpdateLog()
{
	SStringW strLog;
	strLog.Format(L"最新更新 时间:20230526\n更新内容:\n"
		L"1、修复了股票K线添加复权功能后，指数的K线显示出现问题的情况\n"
		L"2、在选股器中添加了股票所在一二级行业的rps和截面数据的调用\n"
		L"3、在股票分时图和K线图中添加了所在一二级行业的rps指标的图形显示\n"
		L"4、添加了集合竞价相关数据的接收与显示，并在选股器中添加了相关数据的调用\n"
		L"5、添加了更新日志的显示功能");
	m_pLogEdit->SetWindowTextW(strLog);
}

