#pragma once
namespace SOUI
{
	class CDlgHeaderSelect : public SHostWnd
	{
	public:
		CDlgHeaderSelect(HWND hWnd,map<int,BOOL>showMap);
		~CDlgHeaderSelect();
		BOOL	OnInitDialog(EventArgs* e);
		void	InitTilteMap();
		void	InitTilteDscpMap();
		void	OnBtnClose();
		void	OnBtnOK();
		BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		bool	OnListTitleClicked(EventArgs* e);

	protected:
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInitDialog)
			EVENT_NAME_COMMAND(L"btn_OK", OnBtnOK)
			EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
			EVENT_NAME_COMMAND(L"btn_Cancel", OnBtnClose)

			EVENT_MAP_END()
			BEGIN_MSG_MAP_EX(CDlgHeaderSelect)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		HWND m_hParWnd;
		SListView* m_pLvTitle;
		SEdit* m_pEditDscp;
		map<int,BOOL> m_bShowMap;
		map<int,SStringW>m_titleMap;
		map<int, SStringW>m_titleDscpMap;
	};
}