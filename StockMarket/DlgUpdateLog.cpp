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
	strLog.Format(
		L"最新更新\n"
		L"时间:2023-07-05\n更新内容:\n"
		L"1、优化了行情列表的更新和排序功能，使用表外排序和显示部分刷新以降低在同时存在多个窗口下的资源消耗和卡顿现象\n"
		L"2、修复了客户端在接受rps实时数据时，对非更新数据周期进行重复赋值，导致cpu占用过大的问题\n"
		L"历史更新\n"
		L"时间:2023-06-30\n更新内容:\n"
		L"1、添加了多窗口选股功能，能够在同一窗口中进行多重选股\n"
		L"2、添加了多窗口选股的自动保存和重启恢复功能\n"
		L"3、能够在一个窗口中选择监控的选股模块数量，数量为1-6\n"
		L"4、修复了原有的窗口多来在创建新窗口时闪退的问题\n"
		L"时间:2023-06-26\n更新内容:\n"
		L"1、修复了K线图切换到日线时，可能出现的程序闪退的问题\n"
		L"时间:2023-06-21\n更新内容:\n"
		L"1、添加了集合竞价成交量和成交额柱状图在日线图中的显示\n"
		L"2、在集合竞价柱状图中添加MA指标的显示和参数调整\n"
		L"3、添加了选股公式的收益回测功能，可以通过选股器打开界面，进行一次最多一年数据的回测\n"
		L"4、在公式回测模块内添加了测试结果导出为CSV文件的功能\n"
		L"5、修复了在在进行选股条件修改时，指标自动设置不匹配的问题\n"
		L"6、添加了列表头的双行显示，可以在相同宽度下完整显示更多数据\n"
		L"时间:2023-06-02\n更新内容:\n"
		L"1、修复了服务器端在计算实时的截面分数时，在二级行业的数据错误的引用了在一级行业中数据的问题\n"
		L"2、在服务器端添加了集合竞价历史数据的处理模块和自动化模块\n"
		L"3、完成了集合竞价历史数据的处理\n"
		L"4、在rps客户端中的日线图中添加了集合竞价分数数据的图形显示\n"
		L"时间:2023-05-26\n更新内容:\n"
		L"1、修复了股票K线添加复权功能后，指数的K线显示出现问题的情况\n"
		L"2、在选股器中添加了股票所在一二级行业的rps和截面数据的调用\n"
		L"3、在股票分时图和K线图中添加了所在一二级行业的rps指标的图形显示\n"
		L"4、添加了集合竞价相关数据的接收与显示，并在选股器中添加了相关数据的调用\n"
		L"5、添加了更新日志的显示功能");
	m_pLogEdit->SetWindowTextW(strLog);
}

