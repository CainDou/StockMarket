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
		void OnClose();
		void OnInit(EventArgs *e);
		LRESULT OnActive(UINT wlParam, BOOL whParam, HWND lParam);
		bool SetStockInfo(SColorListCtrlEx* pPic, vector<StockInfo>* stock1Vec, vector<StockInfo>* stock2Vec=nullptr);
		bool OnEditChange(EventArgs *e);
		bool OnDbClick(EventArgs *e);
		bool OnKeyDown(EventArgs *e);

		std::pair<SStringA, SColorListCtrlEx*> GetShowPicInfo();

		void SubscribeIns(SStringA SubIns);
		void OnTimer(char cTimerID);

		SStringW InputToUpper(SStringW inPut);
		SStringW InputToLower(SStringW inPut);

		SStringW wstrInput;

		SStringA m_subIns;

		HWND m_hParWnd;

		int m_nGroup;
		std::map<int, SStringA>m_Row2InsMap;

		map<SStringA, StockInfo, greater<SStringA>> m_StockInfoMap;
		SColorListCtrlEx* m_useList;

		SEdit* m_pEdit;
		SColorListCtrlEx *m_pList;
	protected:
		//	virtual void OnFinalMessage(HWND hWnd);
		void AddFindItem(int &InsCount, std::pair<const SStringA, StockInfo> &it);
	protected:
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID, OnInit)
			EVENT_NAME_COMMAND(L"btn_kbElfClose", OnClose)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CDlgTickFlowKbElf)
			MSG_WM_TIMER(OnTimer)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	};

}
