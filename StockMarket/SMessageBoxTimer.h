#pragma once
namespace SOUI
{

	INT_PTR  SMessageBoxWithTimer(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int nTime = 0,int nScale = 100);

	class SMessageBoxTimer:public SMessageBoxImpl
	{
	public:
		SMessageBoxTimer();
		~SMessageBoxTimer();
		INT_PTR MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int nTime = 0,int nScale = 100);
		BOOL OnInitDialog(HWND wnd, LPARAM lInitParam);

		void OnTimer(UINT_PTR nID);
	protected:
		BEGIN_MSG_MAP_EX(SMessageBoxTimer)
			MSG_WM_INITDIALOG(OnInitDialog)
			MSG_WM_TIMER(OnTimer)
			CHAIN_MSG_MAP(SMessageBoxImpl)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}

