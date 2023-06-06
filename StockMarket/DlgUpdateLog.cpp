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
		L"���¸���\n"
		L"ʱ��:20230606\n��������:\n"
		L"1�������˼��Ͼ��۳ɽ����ͳɽ�����״ͼ������ͼ�е���ʾ\n"
		L"2���ڼ��Ͼ�����״ͼ������MAָ�����ʾ�Ͳ�������\n"
		L"��ʷ����\n"
		L"ʱ��:20230602\n��������:\n"
		L"1���޸��˷��������ڼ���ʵʱ�Ľ������ʱ���ڶ�����ҵ�����ݴ������������һ����ҵ�����ݵ�����\n"
		L"2���ڷ������������˼��Ͼ�����ʷ���ݵĴ���ģ����Զ���ģ��\n"
		L"3������˼��Ͼ�����ʷ���ݵĴ���\n"
		L"4����rps�ͻ����е�����ͼ�������˼��Ͼ��۷������ݵ�ͼ����ʾ\n"
		L"ʱ��:20230526\n��������:\n"
		L"1���޸��˹�ƱK�����Ӹ�Ȩ���ܺ�ָ����K����ʾ������������\n"
		L"2����ѡ�����������˹�Ʊ����һ������ҵ��rps�ͽ������ݵĵ���\n"
		L"3���ڹ�Ʊ��ʱͼ��K��ͼ������������һ������ҵ��rpsָ���ͼ����ʾ\n"
		L"4�������˼��Ͼ���������ݵĽ�������ʾ������ѡ������������������ݵĵ���\n"
		L"5�������˸�����־����ʾ����");
	m_pLogEdit->SetWindowTextW(strLog);
}
