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
		L"时间:2023-09-25\n更新内容:\n"
		L"1、添加了带有价格下单的报单系统和模拟成交系统\n"
		L"2、添加了当日成交汇总功能\n"
		L"3、修复了卖出报单存在的数量问题\n"
		L"4、修复了历史成交记录被覆盖的问题\n"
		L"--------------------------------------------------------------\n"
		L"历史更新\n"
		L"时间:2023-09-14\n更新内容:\n"
		L"1、在服务器端完成了交易账户的注册和密码功能\n"
		L"2、在服务器端添加了交易账户的登陆校验和反馈功能\n"
		L"3、在服务器端添加了模拟交易的报单响应与交易撮合\n"
		L"4、修复了rps客户端在进行交易时出现的显示及操作问题\n"
		L"时间:2023-08-28\n更新内容:\n"
		L"1、在服务器端添加了股票列表的手动更新功能\n"
		L"2、完成了模拟交易界面的设计与控件功能的响应\n"
		L"3、完成了模拟交易窗口的内部数据通信与功能联动\n"
		L"4、完成了模拟交易窗口与服务器之间的数据请求与接收\n"
		L"时间:2023-08-14\n更新内容:\n"
		L"1、修改了服务器端的订单流数据处理，添加了对各个时段数据在内存中的存储与传输\n"
		L"2、完成了订单流历史数据的处理与存储\n"
		L"3、在rps客户端添加了订单流历史数据的传输与处理，添加了各数据的历史图形显示\n"
		L"4、修复了集合竞价成交量的图形无法显示MA线的问题\n"
		L"5、优化了K线图中各项数据的数据对齐功能，防止数据显示在错误的位置\n"
		L"时间:2023-07-31\n更新内容:\n"
		L"1、添加了涨停统计数据的自动缓存功能，提高页面初次使用时的速度\n"
		L"2、修复了rps客户端在登出时，服务器端可能的死锁，导致后续rps客户端无法成功登陆的问题\n"
		L"3、修复了rps客户端在窗口最小化时主程序退出后，再次登陆子窗口显示过小的问题\n"
		L"4、优化了rps客户端的实时数据在内存中的存储结构，极大的减少了占用的内存\n"
		L"5、在行情列表中添加了主动买/卖单笔数和平均每单的手数\n"
		L"6、在rps客户端的选股器模块中添加了新的功能，能够使用历史数据进行选股\n"
		L"时间:2023-07-17\n更新内容:\n"
		L"1、在选股器界面中添加了选股方案的命名按钮，点击后可以设置当前窗口的选股方案名称\n"
		L"2、修复了在使用成交额强度相关数据进行选股时，选股错误的问题\n"
		L"3、修复了在一二级行业窗口中，对涨幅进行排序时无变化的问题\n"
		L"时间:2023-07-14\n更新内容:\n"
		L"1、修复了rps客户端在查看指数的分析图时，在获取的打分数据为空时，导致程序崩溃的问题\n"
		L"2、添加了热门板块的涨停统计功能\n"
		L"时间:2023-07-07\n更新内容:\n"
		L"1、优化了行情列表的更新和排序功能，使用表外排序和显示部分刷新以降低在同时存在多个窗口下的资源消耗和卡顿现象\n"
		L"2、修复了客户端在接受rps实时数据时，对非更新数据周期进行重复赋值，导致cpu占用过大的问题\n"
		L"3、在列表数据设置界面中添加了列表中的各项数据的计算公式介绍\n"
		L"4、在主界面添加了帮助按钮，该按钮在最小化左侧，点击后将打开程序说明文档\n"
		L"5、修复了在回测进行中，点击取消后，程序可能卡死的问题\n"
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

