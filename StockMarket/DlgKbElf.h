#pragma once
#include"SColorListCtrlEx.h"
#include"Define.h"
#include<map>
#include<vector>
#include<unordered_map>
#include <functional>

using std::vector;
using std::unordered_map;
using std::map;
using std::greater;

namespace SOUI
{
	class SFenShiPic;
	class CDlgKbElf : public SHostWnd
	{
	public:
		CDlgKbElf();
		CDlgKbElf(HWND hParWnd);
		~CDlgKbElf();
	public:
		SStringA GetShowPicInfo();
		bool SetStockInfo(vector<StockInfo>& stock1Vec);
		void ClearInput();
		void SetEditInput(SStringW wstrInput);
	protected:
		void OnClose();
		void OnInit(EventArgs *e);
		bool OnEditChange(EventArgs *e);
		bool OnDbClick(EventArgs *e);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		void SubscribeIns(SStringA SubIns);
		void OnTimer(UINT_PTR cTimerID);

		SStringW InputToUpper(SStringW inPut);
		SStringW InputToLower(SStringW inPut);

	protected:
		SStringW m_wstrInput;
		SStringA m_subIns;
		HWND m_hParWnd;
		std::map<int, SStringA>m_Row2InsMap;
		map<SStringA, StockInfo, greater<SStringA>> m_StockInfoMap;
		SEdit* m_pEdit;
		SColorListCtrlEx *m_pList;
		bool m_bFromSet;
	protected:
		//	virtual void OnFinalMessage(HWND hWnd);
		void AddFindItem(int &InsCount, std::pair<const SStringA, StockInfo> &it);
		virtual void OnFinalMessage(HWND hWnd);

	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_kbElfClose", OnClose)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgTickFlowKbElf)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_KEYUP(OnKeyUp)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}
inline SStringA CDlgKbElf::GetShowPicInfo()
{
	return m_subIns;
}

