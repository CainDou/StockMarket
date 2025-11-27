#include "stdafx.h"
#include "DlgMultiPrdAnly.h"
#include <fstream>
#include <io.h>
#include <helper/SMenu.h>
#include "SKlinePic.h"
#include "DlgRehabFixedTime.h"
#include "IniFile.h"
#include "WndSynHandler.h"
#include "DlgMacdPara.h"
#include "DlgMaPara.h"
#include "DlgBandPara.h"
#include "DlgChangePara.h"
#include "DlgKbElf.h"
#include <sstream>

using std::ifstream;
using std::ofstream;
extern HWND g_MainWnd;
extern CWndSynHandler g_WndSyn;


#define MAX_SUBPIC 3
#define SHOWDATACOUNT 2
#define MAX_TICK 6000
#define MIN_DIFF 0.000001

const std::map<int, int> PrdPosMap = {
	{Period_1Min,0},
	{Period_5Min,1},
	{Period_15Min,2},
	{Period_30Min,3},
	{Period_60Min,4},
	{Period_1Day,5},
};

const std::vector<int> PrdVec = { Period_1Min ,Period_5Min ,Period_15Min ,
Period_30Min ,Period_60Min ,Period_1Day };

enum eTimer
{
	eTimer_DelayUpdate = 1,
};

SOUI::CDlgMultiPrdAnly::CDlgMultiPrdAnly(vector<StockInfo>& stockInfoVec) :SHostWnd(_T("LAYOUT:dlg_multiPeriodAnalysis")),
m_bIsValid(TRUE), m_bLayoutInited(FALSE), m_StockInfoVec(stockInfoVec), m_rehabType(eRT_FrontRehab_Cash),
m_nCurMsgWnd(-1), m_bCAInfoGet(FALSE), m_Group(Group_Stock), m_pKlinePic{ nullptr,nullptr,nullptr,nullptr },
m_nPeriod{ Period_5Min,Period_15Min,Period_60Min,Period_1Day }, m_nNowKTParaChange(-1),
m_nLongestPrdWnd(-1), m_nNowCtrlWnd(-1), m_pDlgKbElf(nullptr)
{
}

SOUI::CDlgMultiPrdAnly::~CDlgMultiPrdAnly()
{
	OutputDebugStringFormat("多周期析构\n");
	if (m_pDlgKbElf)
	{
		m_pDlgKbElf->DestroyWindow();
		//delete m_pDlgKbElf;
		m_pDlgKbElf = NULL;
	}
}


void	SOUI::CDlgMultiPrdAnly::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void	SOUI::CDlgMultiPrdAnly::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void	SOUI::CDlgMultiPrdAnly::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void SOUI::CDlgMultiPrdAnly::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	SWindow* pBtnMax = FindChildByName(L"btn_max");
	SWindow* pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE, TRUE);
		pBtnMax->SetVisible(FALSE, TRUE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE, TRUE);
		pBtnMax->SetVisible(TRUE, TRUE);
	}
	SetTimer(eTimer_DelayUpdate, 1000);
}

int		SOUI::CDlgMultiPrdAnly::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}
BOOL	SOUI::CDlgMultiPrdAnly::OnInitDialog(EventArgs* e)
{
	m_bLayoutInited = TRUE;
	InitControls();
	m_pointInfoMap = g_WndSyn.GetPointInfo();
	SStringA strSubStockID = InitShowConfig(m_pointInfoMap);
	m_accRehabMap = g_WndSyn.GetAccRehabMap();
	InitDatas();
	m_pDlgKbElf = new CDlgKbElf(m_hWnd);
	m_pDlgKbElf->Create(NULL);
	vector<vector<StockInfo>> ListInsVec;
	strHash<SStringA> StockName;
	g_WndSyn.GetListInsVec(ListInsVec, StockName);
	SetStockInfo(ListInsVec[m_Group], StockName);
	InitDataProcFucMap();
	m_dataProcThread = thread(&CDlgMultiPrdAnly::ProcData, this);
	m_uThreadID = *(unsigned*)&m_dataProcThread.get_id();
	g_WndSyn.SetMultiPrdAnlyWnd(m_hWnd, m_uThreadID);
	Sleep(100);
	if (strSubStockID != "")
		SendMsg(m_uThreadID, MPA_SetStock, strSubStockID, strSubStockID.GetLength() + 1);
	return FALSE;
}

LRESULT SOUI::CDlgMultiPrdAnly::OnMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
	int Msg = (int)wp;
	return 0;
}

LRESULT SOUI::CDlgMultiPrdAnly::OnWindowMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
	int Msg = (int)wp;
	switch (Msg)
	{
	case WDMsg_UpdatePic:
	{
		static int64_t nTick = 0;
		int64_t nNowTick = GetTickCount64();
		if (lp < MAX_PIC_NUM && m_nNowCtrlWnd != lp)
		{
			if (m_pKlinePic[lp] && m_pKlinePic[lp]->IsVisible())
				m_pKlinePic[lp]->Invalidate();
		}
		else
		{
			if (lp == MAX_PIC_NUM && nNowTick - nTick < 500)
				break;
			nTick = nNowTick;
			if (m_pKlinePic[m_nNowCtrlWnd])
				m_pKlinePic[m_nNowCtrlWnd]->OutputShowDataTimeRange(m_nStartDate, m_nStartTime, m_nEndDate, m_nEndTime);
			else break;
			for (int i = 0; i < MAX_PIC_NUM; ++i)
			{
				if (m_nNowCtrlWnd != i && m_pKlinePic[i])
				{
					int nDstStartTime = m_nStartTime, nDstEndTime = m_nEndTime;
					ChangeShowTimeRangeByPeriod(nDstStartTime, nDstEndTime, m_nPeriod[m_nNowCtrlWnd], m_nPeriod[i]);
					m_pKlinePic[i]->SetShowDataTimeRange(m_nStartDate, nDstStartTime, m_nEndDate, nDstEndTime);
				}
				if (m_pKlinePic[i]->IsVisible())
					m_pKlinePic[i]->Invalidate();
			}
			if (!m_StockMarketVec.empty())
			{
				auto lastMarket = m_StockMarketVec.back();
				double fChg = lastMarket.LastPrice !=0? lastMarket.LastPrice - lastMarket.PreCloPrice:0;
				SStringW str;
				str.Format(L"%.02f %+.02f %+.02f%%", lastMarket.LastPrice!=0? lastMarket.LastPrice: lastMarket.PreCloPrice, 
					fChg, fChg / lastMarket.PreCloPrice * 100);
				m_pTxtPrice->SetAttribute(L"colorText", fChg > 0 ? L"#FF0000FF" : fChg < 0 ? L"#00FF00FF" : L"#FFFFFFFF");
				m_pTxtPrice->SetWindowTextW(str);

			}
			else
				m_pTxtPrice->SetWindowTextW(L"");

		}

	}
	break;
	case WDMsg_SubIns:
		if (m_pDlgKbElf->GetShowPicInfo() != m_strSubStockID)
		{
			SStringA strStockID = m_pDlgKbElf->GetShowPicInfo();
			SendMsg(m_uThreadID, MPA_SetStock, strStockID, strStockID.GetLength() + 1);
		}
		break;
	case WDMsg_SetFocus:
		CSimpleWnd::SetFocus();
		break;
	case WDMsg_ChangePointTarget:
	{
		int pointData = (int)lp;
		auto pPointData = &m_PointData;
		if (pointData > eIndyMarketPointEnd)
		{
			if (pointData >= eL1IndyPointStart && pointData < eL1IndyPointEnd)
				pPointData = &m_L1IndyPointData;
			else if (pointData >= eL2IndyPointStart && pointData < eL2IndyPointEnd)
				pPointData = &m_L2IndyPointData;
		}


		auto& info = m_pointInfoMap[pointData];
		GetPointData(info, m_strSubStockID, m_nPeriod[m_nCurMsgWnd]);
		int dataCount = m_PointDataCount[info.type];
		vector<vector<CoreData>*> tmpDataArr(dataCount);
		vector<BOOL> rightVec(dataCount);
		vector<SStringA> dataNameVec;
		for (int i = 0; i < dataCount; ++i)
		{
			SStringA dataName = info.srcDataName +
				m_SubPicShowNameVec[info.type][info.dataInRange][i];
			tmpDataArr[i] = &(*pPointData)[m_nPeriod[m_nCurMsgWnd]][dataName];
			dataNameVec.emplace_back(m_SubPicShowNameVec[info.type][info.dataInRange][i]);
			rightVec[i] = TRUE;
		}
		SStringA strTitle = "";
		if (Group_Stock == m_Group)
		{
			vector<SStringA> nameVec;
			GetBelongingIndyName(nameVec);
			if ("L1" == info.dataInRange || "L1" == info.IndyRange)
				strTitle.Format("行业:%s", nameVec[0]);
			else if ("L2" == info.dataInRange || "L2" == info.IndyRange)
				strTitle.Format("行业:%s", nameVec[1]);
			strTitle.Format("%s %s", info.showName, strTitle);
		}


		if (m_pKlinePic[m_nCurMsgWnd] && m_pKlinePic[m_nCurMsgWnd]->IsVisible())
		{
			m_pKlinePic[m_nCurMsgWnd]->SetSelPointWndInfo(info, strTitle);
			m_pKlinePic[m_nCurMsgWnd]->SetSubPicShowData(dataCount, tmpDataArr,
				rightVec, dataNameVec, m_strSubStockID,
				m_StockName.hash[m_strSubStockID]);
		}
	}
	break;
	case WDMsg_ChangeShowTitle:
	{
		SStringW str;
		str.Format(L"%s %s", m_strStockName, StrA2StrW(m_strSubStockID));
		m_pTxtInfo->SetWindowTextW(str);
		CIniFile ini(".\\config\\MultiPrdAnly.ini");
		ini.WriteStringA("Overall", "SubStockID", m_strSubStockID);
	}
	break;
	default:
		break;
	}
	return 0;
}

LRESULT SOUI::CDlgMultiPrdAnly::OnKlineMsg(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
	return 0;
	BOOL bNeedSaveConfig = TRUE;
	switch (lp)
	{
	case KLINEMSG_UPDATE:
		break;
	case KLINEMSG_MA:
		m_pKlinePic[m_nCurMsgWnd]->SetMaPara((int*)wp, m_MaParaSet[m_nCurMsgWnd]);
		::SendMsg(m_uThreadID, MPA_KlineMa, NULL, 0);
		break;
	case KLINEMSG_MACD:
		m_pKlinePic[m_nCurMsgWnd]->SetMacdPara((int*)wp);
		::SendMsg(m_uThreadID, MPA_KlineMacd, NULL, 0);
		break;
	case KLINEMSG_BAND:
		m_pKlinePic[m_nCurMsgWnd]->SetBandPara(*(BandPara_t*)wp);
		::SendMsg(m_uThreadID, MPA_KlineBand, NULL, 0);
		break;
	case KLINEMSG_REHAB:
	{
		m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");
		::SendMsg(m_uThreadID, MPA_FixedTimeRehab, (char*)wp, sizeof(FixedTimeRehab));
		bNeedSaveConfig = FALSE;
		FixedTimeRehab& frt = *(FixedTimeRehab*)wp;
		CIniFile ini(".//config//MultiPrdAnly.ini");
		ini.WriteIntA("Overall", "RehabType", frt.Type);
		ini.WriteIntA("Overall", "FrontRehab", frt.bFrontRehab);
		ini.WriteIntA("Overall", "RehabTime", frt.nDate);
		m_rehabType = frt.Type;
	}
	break;
	case KLINEMSG_CHANGEPARA:
	{
		if (m_nNowKTParaChange >= 0)
		{
			std::vector<int>* pPara = (std::vector<int>*)wp;
			auto ti = m_pKlinePic[m_nCurMsgWnd]->GetTargetInfo(m_nNowKTParaChange);
			if (ti.nParaDefValue.size() == pPara->size())
			{
				ti.nUsePara.swap(*pPara);
				m_pKlinePic[m_nCurMsgWnd]->ChangeTargetInfo(m_nNowKTParaChange, ti);
				::SendMsg(m_uThreadID, MPA_ReCalcTarget, NULL, 0);
				m_nNowKTParaChange = -1;
			}
		}

	}
	break;
	case KLINEMSG_CHANGEDEFAULTPARA:
	{
		bNeedSaveConfig = FALSE;
		int nIndex = (int)wp;
		SStringA strTargetName = CKlineTarget::GetTargetOrgInfo(nIndex).strTargetName;
		CIniFile ini(".\\config\\config.ini");
		SStringA strPara = ini.GetStringA("DefaultPara", strTargetName, "");
		if (!strPara.IsEmpty())
		{
			vector<int> paraVec;
			SStringW strTmp;
			for (int i = 0; i < strPara.GetLength(); ++i)
			{
				if (strPara[i] != ',')
					strTmp += strPara[i];
				else
				{
					paraVec.emplace_back(_wtoi(strTmp));
					strTmp.Empty();
				}
			}
			if (!strTmp.IsEmpty())
				paraVec.emplace_back(_wtoi(strTmp));
			CKlineTarget::ChangeTargetInfoDefPara(nIndex, paraVec);
		}

	}
	break;
	default:
		bNeedSaveConfig = FALSE;
		break;
	}
	if (bNeedSaveConfig)
		SaveShowConfig(m_nCurMsgWnd);
	return 0;
}

void	SOUI::CDlgMultiPrdAnly::OnClose()
{
	SetMsgHandled(FALSE);
	m_bIsValid = FALSE;
	ShowWindow(SW_HIDE);
	if (_access(".\\config\\MultiPrdAnly.position", 0) == 0)
		remove(".\\config\\MultiPrdAnly.position");
	g_WndSyn.RemoveMultiPrdAnlyWnd(m_hWnd);
	::PostMessage(g_MainWnd, WM_WINDOW_MSG, WDMsg_RemoveMultiPrdAnlyWnd, NULL);
}


void	SOUI::CDlgMultiPrdAnly::OnDestroy()
{
	SetMsgHandled(FALSE);
	SendMsg(m_uThreadID, Msg_Exit, NULL, 0);
	if (m_dataProcThread.joinable())
		m_dataProcThread.join();
	if (m_bIsValid)
	{
		SStringA strPosFile;
		strPosFile.Format(".\\config\\MultiPrdAnly.position");
		std::ofstream ofile(strPosFile);
		if (ofile.is_open())
		{
			WINDOWPLACEMENT wp = { sizeof(wp) };
			::GetWindowPlacement(m_hWnd, &wp);

			ofile.write((char*)&wp, sizeof(wp));
			ofile.close();
		}
	}

}
void	SOUI::CDlgMultiPrdAnly::OnBtnClose()
{
	GetNative()->SendMessage(WM_CLOSE);

}
void	SOUI::CDlgMultiPrdAnly::OnBtnRehab()
{
	SMenu menu;
	menu.LoadMenuW(L"smenu:menu_rehab");
	for (int i = RM_NoRehab; i < RM_End; ++i)
		menu.CheckMenuItem(i, i - RM_NoRehab == m_rehabType ? MF_CHECKED : MF_UNCHECKED);
	CRect rc = m_pBtnRehab->GetWindowRect();
	ClientToScreen(&rc);
	menu.TrackPopupMenu(0, rc.right, rc.bottom, m_hWnd);

}

void SOUI::CDlgMultiPrdAnly::OnButtonDown(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	int nWndIndex = -1;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pKlinePic[i] && m_pKlinePic[i]->IsContainPoint(point, TRUE))
		{
			nWndIndex = i;
			break;
		}
	}
	if (nWndIndex >= 0)
		m_nCurMsgWnd = nWndIndex;

}

void	SOUI::CDlgMultiPrdAnly::OnRButtonUp(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	SMenu menu;
	int nWndIndex = -1;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pKlinePic[i] && m_pKlinePic[i]->IsContainPoint(point, TRUE))
		{
			nWndIndex = i;
			break;
		}
	}
	if (nWndIndex >= 0)
	{

		m_nCurMsgWnd = nWndIndex;
		OutputDebugStringFormat("当前窗口是%d\n", nWndIndex);
		menu.LoadMenuW(L"smenu:menu_kline");
		if (m_pKlinePic[nWndIndex]->GetDealState())
			menu.CheckMenuItem(KM_Deal, MF_CHECKED);
		if (m_pKlinePic[nWndIndex]->GetVolumeState())
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_CHECKED);
			menu.CheckMenuItem(KM_Volume, MF_CHECKED);
			menu.CheckMenuItem(KM_Amount, MF_UNCHECKED);
		}
		else if (m_pKlinePic[nWndIndex]->GetAmountState())
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_CHECKED);
			menu.CheckMenuItem(KM_Volume, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Amount, MF_CHECKED);
		}
		else
		{
			menu.CheckMenuItem(KM_VolOrAmo, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Volume, MF_UNCHECKED);
			menu.CheckMenuItem(KM_Amount, MF_UNCHECKED);
		}

		if (m_pKlinePic[nWndIndex]->GetIsTFBaseDataUsed())
		{
			int nType = m_pKlinePic[nWndIndex]->GetTickFlowDataType();
			menu.CheckMenuItem(KM_TFRatio + nType, MF_CHECKED);
		}

		UINT VolState = m_pKlinePic[nWndIndex]->GetCAVolState() ? MF_CHECKED : MF_UNCHECKED;
		UINT AmoState = m_pKlinePic[nWndIndex]->GetCAAmoState() ? MF_CHECKED : MF_UNCHECKED;
		UINT AllState = (VolState || AmoState) ? MF_CHECKED : MF_UNCHECKED;
		menu.CheckMenuItem(KM_CAVolOrAmo, AllState);
		menu.CheckMenuItem(KM_CAVol, VolState);
		menu.CheckMenuItem(KM_CAAmo, AmoState);

		menu.CheckMenuItem(KM_VolDiff, m_pKlinePic[nWndIndex]->GetBigVolDiffState() ? MF_CHECKED : MF_UNCHECKED);

		if (m_pKlinePic[nWndIndex]->GetMacdState())
			menu.CheckMenuItem(KM_MACD, MF_CHECKED);
		int nWndNum = m_pKlinePic[nWndIndex]->GetShowSubPicNum();
		//if (m_pKlinePic[m_nCurMsgWnd]->GetRpsState(SP_FULLMARKET))
		menu.CheckMenuItem(KM_PointWnd0 + nWndNum, MF_CHECKED);
		if (m_pKlinePic[nWndIndex]->GetMaState())
			menu.CheckMenuItem(KM_MA, MF_CHECKED);
		if (m_pKlinePic[nWndIndex]->GetBandState())
			menu.CheckMenuItem(KM_Band, MF_CHECKED);
		if (m_pKlinePic[nWndIndex]->GetMainTarget() == eMain_NetGrid)
			menu.CheckMenuItem(KM_NetGrid, MF_CHECKED);



		ClientToScreen(&point);
		menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);
	}
	return;
}

void	SOUI::CDlgMultiPrdAnly::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(TRUE);
	if (nChar == 229)
	{
		m_nLastChar = 0;

		for (int i = VK_NUMPAD0; i <= VK_NUMPAD9; ++i)
		{
			SHORT nRps = ::GetKeyState(i);
			if (HIBYTE(nRps) == 0xff)
			{
				m_nLastChar = i - 0x30;
				return;
			}
		}
		for (int i = '0'; i <= '9'; ++i)
		{
			SHORT nRps = ::GetKeyState(i);
			if (HIBYTE(nRps) == 0xff)
			{
				m_nLastChar = i;
				return;
			}
		}

	}
	if (nChar >= VK_LEFT && nChar <= VK_DOWN)
	{
		int nWndIndex = m_nCurMsgWnd != -1 ? m_nCurMsgWnd : m_nNowCtrlWnd;
		if (!m_pKlinePic[nWndIndex]) return;
		bool bNeedRePaint = (nChar == VK_LEFT || nChar == VK_RIGHT) ? false : true;
		if (!bNeedRePaint)
			bNeedRePaint = m_pKlinePic[nWndIndex]->CheckKeyLeftOrRightMoveChange(nChar == VK_LEFT);
		m_pKlinePic[nWndIndex]->SSendMessage(WM_KEYDOWN, (WPARAM)nChar, MAKELPARAM(nRepCnt, nFlags));
		if (nWndIndex != m_nNowCtrlWnd)
		{
			m_pChkCtrl[m_nNowCtrlWnd]->SetCheck(FALSE);
			m_nNowCtrlWnd = nWndIndex;
			m_pChkCtrl[m_nNowCtrlWnd]->SetCheck(TRUE);
			CIniFile ini(".//config//MultiPrdAnly.ini");
			ini.WriteIntA("Overall", "CtrlWnd", m_nNowCtrlWnd);
		}

		if (bNeedRePaint)
		{
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
			SaveShowConfig(nWndIndex);
		}
	}
}

void	SOUI::CDlgMultiPrdAnly::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetMsgHandled(TRUE);
	if (nChar == 229)
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hWnd, &rc);
			m_pDlgKbElf->SetWindowPos(HWND_TOPMOST, rc.right - 320,
				rc.bottom - 370, 0, 0, SWP_NOSIZE);
			m_pDlgKbElf->ClearInput();
			SStringW input = L"";
			if (m_nLastChar >= '0' && m_nLastChar <= '9')
				input = (char)m_nLastChar;
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
			//SetMsgHandled(TRUE);
		}
	}
	else if ((nChar >= 0x30 && nChar <= 0x39) ||
		(nChar >= 0x41 && nChar <= 0x5A) ||
		(nChar >= VK_NUMPAD0 && nChar <= VK_NUMPAD9))
	{
		if (!m_pDlgKbElf->IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_hWnd, &rc);
			m_pDlgKbElf->SetWindowPos(NULL,
				rc.right - 320, rc.bottom - 370, 0, 0,
				SWP_NOSIZE);
			m_pDlgKbElf->ClearInput();
			SStringW input;
			if (nChar >= VK_NUMPAD0 && nChar <= VK_NUMPAD9)
				input = (char)(nChar - 0x30);
			else if (nChar >= 0x30 && nChar <= 0x39)
			{
				bool bShift = HIBYTE(GetKeyState(VK_SHIFT)) > 0 ? true : false;
				if (bShift)
				{
					if (nChar == 0x32) input = 0x40;
					else if (nChar == 0x36) input = 0x5E;
					else if (nChar == 0x37) input = 0x26;
					else if (nChar == 0x38) input = 0x2A;
					else if (nChar == 0x39) input = 0x28;
					else if (nChar == 0x30) input = 0x29;
					else
						input = nChar - 0x10;
				}
				else
					input = (char)nChar;
			}
			else
			{
				bool bCaps = LOBYTE(GetKeyState(VK_CAPITAL)) > 0 ? true : false;
				bool bShift = HIBYTE(GetKeyState(VK_SHIFT)) > 0 ? true : false;
				input = bCaps ^ bShift ?
					(char)nChar : (char)(nChar + 32);
			}
			m_pDlgKbElf->ShowWindow(SW_SHOWDEFAULT);
			m_pDlgKbElf->SetEditInput(input);
		}

	}
}

void SOUI::CDlgMultiPrdAnly::OnExitSizeMove(LPWINDOWPOS lpWndPos)
{
	SetMsgHandled(FALSE);
}

void	SOUI::CDlgMultiPrdAnly::OnTimer(UINT_PTR nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == eTimer_DelayUpdate)
	{
		::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
		KillTimer(eTimer_DelayUpdate);
	}
}

bool SOUI::CDlgMultiPrdAnly::OnCmbPrdChange(EventArgs* e)
{
	EventCBSelChange* pEvt = dynamic_cast<EventCBSelChange*>(e);
	int nWndIndex = -1;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pCbxPeriod[i] == pEvt->sender)
		{
			nWndIndex = i;
			break;
		}
	}
	if (nWndIndex == -1)
		return FALSE;
	int nPeriod = PrdVec[pEvt->nCurSel];
	if (nPeriod == m_nPeriod[nWndIndex])
		return FALSE;
	m_nPeriod[nWndIndex] = nPeriod;
	if (!m_pKlinePic[nWndIndex]) return false;
	if (m_KlineGetMap.count(nPeriod) == 0)
	{
		m_pKlinePic[nWndIndex]->SetHisKlineState(false);
		DataGetInfo GetInfo;
		GetInfo.hWnd = m_hWnd;
		strcpy_s(GetInfo.StockID, m_strSubStockID);
		GetInfo.Group = m_Group;
		GetInfo.Period = nPeriod;
		if (nPeriod == Period_1Day)
		{
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetCallAction,
				(char*)&GetInfo, sizeof(GetInfo));
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetTradeVol,
				(char*)&GetInfo, sizeof(GetInfo));
		}
		SendMsg(g_WndSyn.GetThreadID(), Syn_GetKline,
			(char*)&GetInfo, sizeof(GetInfo));
	}
	else
		m_pKlinePic[nWndIndex]->SetHisKlineState(true);
	if (m_pKlinePic[nWndIndex]->GetIsTFBaseDataUsed())
	{
		if (m_TFBaseGetMap[nPeriod] == 0)
		{
			m_pKlinePic[nWndIndex]->SetTFMarketState(false);
			DataGetInfo GetInfo;
			GetInfo.hWnd = m_hWnd;
			strcpy_s(GetInfo.StockID, m_strSubStockID);
			GetInfo.Group = m_Group;
			GetInfo.Period = nPeriod;
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetHisTFBase,
				(char*)&GetInfo, sizeof(GetInfo));
		}
		else
			m_pKlinePic[nWndIndex]->SetTFMarketState(true);

	}
	vector<ShowPointInfo> infoVec;
	m_pKlinePic[nWndIndex]->GetShowPointInfo(infoVec);
	for (auto& info : infoVec)
		GetPointData(info, m_strSubStockID, nPeriod);

	SetKlineShowData(nWndIndex, infoVec, nPeriod, TRUE);
	int nDstStartTime = m_nStartTime, nDstEndTime = m_nEndTime;
	ChangeShowTimeRangeByPeriod(nDstStartTime, nDstEndTime, m_nPeriod[m_nNowCtrlWnd], m_nPeriod[nWndIndex]);
	m_pKlinePic[nWndIndex]->SetShowDataTimeRange(m_nStartDate, nDstStartTime, m_nEndDate, nDstEndTime, true);
	m_pKlinePic[nWndIndex]->Invalidate();
	return FALSE;
}

bool SOUI::CDlgMultiPrdAnly::OnChkClicked(EventArgs* e)
{
	EventLButtonUp* pEvt = dynamic_cast<EventLButtonUp*>(e);
	int nWndIndex = -1;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pChkCtrl[i] == pEvt->sender)
		{
			nWndIndex = i;
			break;
		}
	}
	if (m_pChkCtrl[nWndIndex]->IsChecked())
	{
		m_pChkCtrl[m_nNowCtrlWnd]->SetCheck(FALSE);
		m_nNowCtrlWnd = nWndIndex;
	}
	else
		m_pChkCtrl[nWndIndex]->SetCheck(TRUE);
	CIniFile ini(".//config//MultiPrdAnly.ini");
	ini.WriteIntA("Overall", "CtrlWnd", m_nNowCtrlWnd);
	return false;
}

void	SOUI::CDlgMultiPrdAnly::InitWindowPos()
{
	WINDOWPLACEMENT wp;
	std::ifstream ifile;
	SStringA strFileName;
	strFileName.Format(".\\config\\MultiPrdAnly.position");
	ifile.open(strFileName, std::ios::in | std::ios::binary);
	if (ifile.is_open())
	{
		ifile.read((char*)&wp, sizeof(wp));
		::SetWindowPlacement(m_hWnd, &wp);
		ifile.close();
	}
	else
		CenterWindow(g_MainWnd);

}



void	SOUI::CDlgMultiPrdAnly::InitControls()
{
	m_pTxtInfo = FindChildByName2<SStatic>(L"txt_Info");
	m_pTxtPrice = FindChildByName2<SStatic>(L"txt_Price");
	m_pBtnRehab = FindChildByName2<SButton>(L"btn_Rehab");
	for (int i = 0; i < 4; i++)
	{
		SStringW strWndName;
		strWndName.Format(L"wnd%d", i + 1);
		m_pWnd[i] = FindChildByName2<SWindow>(strWndName);
		if (m_pWnd[i])
		{
			m_pCbxPeriod[i] = m_pWnd[i]->FindChildByName2<SComboBox>(L"cbx_period");
			m_pKlinePic[i] = m_pWnd[i]->FindChildByName2<SKlinePic>("klinePic");
			m_pChkCtrl[i] = m_pWnd[i]->FindChildByName2<SCheckBox>(L"chk_ctrl");
			if (m_pCbxPeriod[i])
			{
				if (i < 2)
					m_pCbxPeriod[i]->SetCurSel(i + 1);
				else
					m_pCbxPeriod[i]->SetCurSel(i + 2);
			}
			m_pCbxPeriod[i]->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE,
				Subscriber(&CDlgMultiPrdAnly::OnCmbPrdChange, this));
			m_pChkCtrl[i]->GetEventSet()->subscribeEvent(EVT_LBUTTONUP,
				Subscriber(&CDlgMultiPrdAnly::OnChkClicked, this));

			if (m_pKlinePic[i])
				m_pKlinePic[i]->SetDealState(false, false);
		}
	}
}

void	SOUI::CDlgMultiPrdAnly::InitDatas()
{
	m_PointDataCount[eRpsPoint] = 2;
	m_PointDataCount[eSecPoint] = 1;
	m_SubPicShowNameVec[eRpsPoint][""].emplace_back("Point520");
	m_SubPicShowNameVec[eRpsPoint][""].emplace_back("Point2060");
	m_SubPicShowNameVec[eRpsPoint]["L1"].emplace_back("Point520L1");
	m_SubPicShowNameVec[eRpsPoint]["L1"].emplace_back("Point2060L1");
	m_SubPicShowNameVec[eRpsPoint]["L2"].emplace_back("Point520L2");
	m_SubPicShowNameVec[eRpsPoint]["L2"].emplace_back("Point2060L2");

	m_SubPicShowNameVec[eSecPoint][""].emplace_back("Point");
	m_SubPicShowNameVec[eSecPoint]["L1"].emplace_back("PointL1");
	m_SubPicShowNameVec[eSecPoint]["L2"].emplace_back("PointL2");

}

void SOUI::CDlgMultiPrdAnly::InitDataProcFucMap()
{
	m_dataHandleMap[Syn_Point] =
		&CDlgMultiPrdAnly::OnUpdatePoint;
	m_dataHandleMap[Syn_HisRpsPoint] =
		&CDlgMultiPrdAnly::OnUpdateHisRpsPoint;
	//m_dataHandleMap[MPA_RTIndexMarket] =
	//	&CDlgMultiPrdAnly::OnUpdateIndexMarket;
	m_dataHandleMap[Syn_RTStockMarket] =
		&CDlgMultiPrdAnly::OnUpdateStockMarket;
	//m_dataHandleMap[MPA_HisIndexMarket] =
	//	&CDlgMultiPrdAnly::OnUpdateHisIndexMarket;
	m_dataHandleMap[Syn_HisStockMarket] =
		&CDlgMultiPrdAnly::OnUpdateHisStockMarket;
	m_dataHandleMap[Syn_HisKline] =
		&CDlgMultiPrdAnly::OnUpdateHisKline;
	//m_dataHandleMap[MPA_CloseInfo] =
	//	&CDlgMultiPrdAnly::OnUpdateCloseInfo;
	m_dataHandleMap[MPA_SetStock] =
		&CDlgMultiPrdAnly::OnKlineChangeStock;
	m_dataHandleMap[MPA_KlineMa] =
		&CDlgMultiPrdAnly::OnKlineMa;
	m_dataHandleMap[MPA_KlineMacd] =
		&CDlgMultiPrdAnly::OnKlineMacd;
	m_dataHandleMap[MPA_KlineBand] =
		&CDlgMultiPrdAnly::OnKlineBand;
	m_dataHandleMap[MPA_ReCalcTarget] =
		&CDlgMultiPrdAnly::OnKlineTargetReCalc;
	m_dataHandleMap[Syn_HisSecPoint] =
		&CDlgMultiPrdAnly::OnUpdateHisSecPoint;
	m_dataHandleMap[Syn_RehabInfo] =
		&CDlgMultiPrdAnly::OnUpdateRehabInfo;
	m_dataHandleMap[MPA_ChangeRehab] =
		&CDlgMultiPrdAnly::OnChangeKlineRehab;
	m_dataHandleMap[MPA_FixedTimeRehab] =
		&CDlgMultiPrdAnly::OnFixedTimeRehab;
	m_dataHandleMap[Syn_HisCallAction] =
		&CDlgMultiPrdAnly::OnUpdateHisCallAction;
	m_dataHandleMap[Syn_HisTFBase] =
		&CDlgMultiPrdAnly::OnUpdateHisTFBase;
	m_dataHandleMap[Syn_TodayTFMarket] =
		&CDlgMultiPrdAnly::OnUpdateTodayTFMarket;
	m_dataHandleMap[Syn_RTTFMarket] =
		&CDlgMultiPrdAnly::OnUpdateRTTFMarket;
	m_dataHandleMap[Syn_RTPriceVol] =
		&CDlgMultiPrdAnly::OnUpdateRTPriceVol;
	m_dataHandleMap[Syn_RTTradeVol] =
		&CDlgMultiPrdAnly::OnUpdateRTTradeVol;
	m_dataHandleMap[Syn_HisTradeVol] =
		&CDlgMultiPrdAnly::OnUpdateHisTradeVol;

}

SStringA SOUI::CDlgMultiPrdAnly::InitShowConfig(map<int, ShowPointInfo>& pointMap)
{
	BOOL bHasIniFile = _access(".//config//MultiPrdAnly.ini", 0) == 0;
	CIniFile ini(".//config//MultiPrdAnly.ini");
	int CloseMAPara[] = { 5,10,20,60,0,0 };
	int VolAmoMAPara[] = { 5,10,0,0,0,0 };
	int nDefaultKlineWidth[] = { 1,3,12,48 };
	m_rehabType = (eRehabType)ini.GetIntA("Overall", "RehabType", eRT_FrontRehab_Cash);
	if (m_rehabType != eRT_NoRehab)
		m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");
	m_nNowCtrlWnd = ini.GetIntA("Overall", "CtrlWnd", -1);
	int nLongestPrd = 0;
	SStringA strSubStockID = ini.GetStringA("Overall", "SubStockID", "");
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		InitPara initPara;
		if (bHasIniFile)
		{
			SStringA strSection;
			strSection.Format("Wnd%d", i);
			GetInitPara(ini, initPara, strSection);
			InitPointWndInfo(ini, initPara, strSection, pointMap);
		}
		else
		{
			initPara.bShowKlineDeal = false;
			initPara.Period = m_nPeriod[i];
			initPara.nWidth = nDefaultKlineWidth[i];
		}
		if (m_pKlinePic[i])
		{
			m_nPeriod[i] = initPara.Period;
			if (m_nPeriod[i] > nLongestPrd)
			{
				nLongestPrd = m_nPeriod[i];
				m_nLongestPrdWnd = i;
			}
			m_pKlinePic[i]->SetRpsGroup(m_Group);
			m_pKlinePic[i]->SetParentHwnd(m_hWnd);
			m_pKlinePic[i]->InitShowPara(initPara);
			//m_pKlinePic[i]->InitSubPic(initPara.nKlinePointWndNum);
			m_pKlinePic[i]->SetDataPoint(&m_StockMarketVec, &m_KlineMap,
				&m_RtTFMarketVec, &m_TFBaseMap);
			m_pKlinePic[i]->SetCaInfoData(&m_CallAction);
			m_pKlinePic[i]->SetTradeVolData(&m_TradeVolData);
			m_pKlinePic[i]->SetPriceListHalf(TRUE);
		}
		m_pCbxPeriod[i]->SetCurSel(PrdPosMap.at(m_nPeriod[i]));
	}
	if (m_nNowCtrlWnd == -1)
		m_nNowCtrlWnd = m_nLongestPrdWnd;
	if (!bHasIniFile)
	{
		ini.WriteIntA("Overall", "CtrlWnd", m_nNowCtrlWnd);
		for (int i = 0; i < MAX_PIC_NUM; ++i)
		{
			SStringA strSection;
			strSection.Format("Wnd%d", i);
			SaveShowConfig(i);
		}
	}

	m_pChkCtrl[m_nNowCtrlWnd]->SetCheck(TRUE);
	return strSubStockID;
}

void SOUI::CDlgMultiPrdAnly::OnRehabMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	int rehabType = nID - RM_NoRehab;
	if (rehabType != eRT_Rehab_Cash_FixedTime &&
		rehabType != eRT_Rehab_ReInv_FixedTime)
	{
		::SendMsg(m_uThreadID, MPA_ChangeRehab, (char*)&rehabType, sizeof(rehabType));
		if (rehabType == eRT_NoRehab)
			m_pBtnRehab->SetAttribute(L"colorText", L"#c0c0c0ff");
		else
			m_pBtnRehab->SetAttribute(L"colorText", L"#00ffffff");
		CIniFile ini(".//config//MultiPrdAnly.ini");
		ini.WriteIntA("Overall", "RehabType", rehabType);
		m_rehabType = (eRehabType)rehabType;
	}
	else
	{
		CDlgRehabFixedTime* pDlg = new CDlgRehabFixedTime(m_hWnd, (eRehabType)rehabType);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
}

void SOUI::CDlgMultiPrdAnly::OnKlineMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	bool bState = false;
	return;
	switch (nID)
	{
	case KM_Deal:
		m_pKlinePic[m_nCurMsgWnd]->SetDealState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		bState = m_pKlinePic[m_nCurMsgWnd]->GetDealState();
		break;
	case KM_MA:
		m_pKlinePic[m_nCurMsgWnd]->SetMainTarget(eMain_MA, vector<int>());
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		bState = m_pKlinePic[m_nCurMsgWnd]->GetMaState();
		break;
	case KM_Band:
		m_pKlinePic[m_nCurMsgWnd]->SetMainTarget(eMain_Band, vector<int>());

		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		break;
	case KM_Volume:
		m_pKlinePic[m_nCurMsgWnd]->SetVolumeState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		//bState = m_pKlinePic[m_nCurMsgWnd]->GetVolumeState();
		break;
	case KM_MACD:
		m_pKlinePic[m_nCurMsgWnd]->SetMacdState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		bState = m_pKlinePic[m_nCurMsgWnd]->GetMacdState();
		break;
		//case KM_RPS:
		//	m_pKlinePic[m_nCurMsgWnd]->SetRpsState(SP_FULLMARKET);
		//	m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		//	bState = m_pKlinePic[m_nCurMsgWnd]->GetRpsState(SP_FULLMARKET);
		//	::PostMessage(m_hParWnd, WM_WINDOW_MSG,
		//		WDMsg_SaveConfig, NULL);
		//	break;
	case KM_MacdPara:
	{
		CDlgMacdPara* pDlg = new CDlgMacdPara(m_Group, m_hWnd);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMacdPara());
		pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
	}
	break;
	case KM_NetGrid:
	{
		m_pKlinePic[m_nCurMsgWnd]->SetMainTarget(eMain_NetGrid,
			m_initPara[m_nCurMsgWnd].KlineMainTargetPara.count(eMain_NetGrid) ?
			m_initPara[m_nCurMsgWnd].KlineMainTargetPara[eMain_NetGrid] : vector<int>());
		::SendMsg(m_uThreadID, MPA_ReCalcTarget, NULL, 0);
	}
	break;
	case KM_ChangeMainPara:
	{
		int nMainPara = m_pKlinePic[m_nCurMsgWnd]->GetMainTarget();
		if (nMainPara == eMain_MA)
		{
			m_MaParaSet[m_nCurMsgWnd] = eMa_Close;
			CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
			pDlg->Create(NULL);
			pDlg->CenterWindow(m_hWnd);
			pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
			pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			pDlg->ShowWindow(SW_SHOWDEFAULT);
			::EnableWindow(m_hWnd, FALSE);
		}
		else if (nMainPara == eMain_Band)
		{
			CDlgBandPara* pDlg = new CDlgBandPara(m_Group, m_hWnd);
			pDlg->Create(NULL);
			pDlg->CenterWindow(m_hWnd);
			pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetBandPara());
			pDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			pDlg->ShowWindow(SW_SHOWDEFAULT);

		}
		else if (nMainPara >= eMain_NetGrid)
		{
			m_nNowKTParaChange = nMainPara - eMain_NetGrid;
			auto ti = m_pKlinePic[m_nCurMsgWnd]->GetTargetInfo(0);
			CDlgChangePara* pDlgPara = new CDlgChangePara(ti, m_hWnd);
			pDlgPara->Create(NULL);
			pDlgPara->CenterWindow(m_hWnd);
			if (ti.strParaName.size() <= 6)
				pDlgPara->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			else
			{
				int nExLevel = ti.strParaName.size() / 3 - 2;
				pDlgPara->SetWindowPos(NULL, 0, 0, 365, 200 + nExLevel * 40, SWP_NOMOVE);

			}
			pDlgPara->ShowWindow(SW_SHOWDEFAULT);
			::EnableWindow(m_hWnd, FALSE);

		}
	}
	break;
	case KM_PointWnd0:
	case KM_PointWnd1:
	case KM_PointWnd2:
	case KM_PointWnd3:
	case KM_PointWnd4:
	case KM_PointWnd5:
	case KM_PointWnd6:
	case KM_PointWnd7:
	case KM_PointWnd8:
	{
		int nWndNum = nID - KM_PointWnd0;
		int nOldWndNum = m_pKlinePic[m_nCurMsgWnd]->GetShowSubPicNum();
		vector<ShowPointInfo> infoVec = m_pKlinePic[m_nCurMsgWnd]->GetSubPicDataToGet(nWndNum, m_pointInfoMap);
		map<int, ShowPointInfo> m_pointInfoMap;
		m_pKlinePic[m_nCurMsgWnd]->ReSetSubPic(nWndNum, infoVec);

		std::set<ShowPointInfo>pointGetSet;
		for (auto& it : infoVec)
		{
			if (pointGetSet.count(it) == 0)
			{
				GetPointData(it, m_strSubStockID, m_nPeriod[m_nCurMsgWnd]);
				pointGetSet.insert(it);
			}
		}
		SetKlineShowData(m_nCurMsgWnd, infoVec, m_nPeriod[m_nCurMsgWnd], FALSE, nOldWndNum);
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		break;
	}
	case KM_Amount:
		m_pKlinePic[m_nCurMsgWnd]->SetAmountState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		//bState = m_pKlinePic[m_nCurMsgWnd]->GetVolumeState();
		break;
	case KM_VolMaPara:
	{
		m_MaParaSet[m_nCurMsgWnd] = eMa_Volume;
		CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_AmoMaPara:
	{
		m_MaParaSet[m_nCurMsgWnd] = eMa_Amount;
		CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_CAVol:
		m_pKlinePic[m_nCurMsgWnd]->SetCAVolState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		break;
	case KM_CAAmo:
		m_pKlinePic[m_nCurMsgWnd]->SetCAAmoState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		break;
	case KM_CAVolMaPara:
	{
		m_MaParaSet[m_nCurMsgWnd] = eMa_CAVol;
		CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_CAAmoMaPara:
	{
		m_MaParaSet[m_nCurMsgWnd] = eMa_CAAmo;
		CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;
	case KM_TFRatio:
	case KM_TFVol:
	case KM_TFOrder:
	case KM_TFAvgVol:
	{
		if (m_Group != Group_Stock)
			break;
		m_pKlinePic[m_nCurMsgWnd]->SetTickFlowDataType(nID - KM_TFRatio);
		if (m_pKlinePic[m_nCurMsgWnd]->GetIsTFBaseDataUsed())
		{
			if (m_TFBaseGetMap.count(m_nPeriod[m_nCurMsgWnd]) == 0)
			{
				m_pKlinePic[m_nCurMsgWnd]->SetTFMarketState(false);
				DataGetInfo GetInfo;
				GetInfo.hWnd = m_hWnd;
				strcpy_s(GetInfo.StockID, m_strSubStockID);
				GetInfo.Group = m_Group;
				GetInfo.Period = m_nPeriod[m_nCurMsgWnd];
				SendMsg(g_WndSyn.GetThreadID(), Syn_GetHisTFBase,
					(char*)&GetInfo, sizeof(GetInfo));
			}
			else
				m_pKlinePic[m_nCurMsgWnd]->SetTFMarketState(true);

		}
		::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);

	}
	break;
	case KM_VolDiff:
		if (m_Group != Group_Stock)
			break;
		m_pKlinePic[m_nCurMsgWnd]->SetBigVolDiffState();
		m_pKlinePic[m_nCurMsgWnd]->Invalidate();
		//bState = m_pKlinePic[m_nCurMsgWnd]->GetVolumeState();
		break;
	case KM_VolDiffPara:
	{
		if (m_Group != Group_Stock)
			break;
		m_MaParaSet[m_nCurMsgWnd] = eMa_VolDiff;
		CDlgMaPara* pDlg = new CDlgMaPara(m_Group, m_hWnd, m_MaParaSet[m_nCurMsgWnd]);
		pDlg->Create(NULL);
		pDlg->CenterWindow(m_hWnd);
		pDlg->SetEditText(m_pKlinePic[m_nCurMsgWnd]->GetMaPara(m_MaParaSet[m_nCurMsgWnd]));
		pDlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWDEFAULT);
		::EnableWindow(m_hWnd, FALSE);
	}
	break;

	default:
		break;
	}
	SaveShowConfig(m_nCurMsgWnd);

}

void SOUI::CDlgMultiPrdAnly::SaveShowConfig(int nWndIndex)
{
	CIniFile ini(".\\config\\MultiPrdAnly.ini");
	InitPara initPara;
	m_pKlinePic[nWndIndex]->OutPutShowPara(initPara);
	SStringA strSection;
	initPara.Period = m_nPeriod[nWndIndex];
	strSection.Format("Wnd%d", nWndIndex);
	SaveInitPara(ini, initPara, strSection);
	SavePointWndInfo(ini, initPara, strSection);
}

void SOUI::CDlgMultiPrdAnly::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void	SOUI::CDlgMultiPrdAnly::GetPointData(ShowPointInfo& info, SStringA StockID, int nPeriod)
{
	if (CheckDataIsGot(info, nPeriod))
		return;
	ExDataGetInfo GetInfo;
	GetInfo.nAskGroup = m_Group;

	//GetInfo.nAskGroup = m_Group;
	GetInfo.Type = info.type;
	GetInfo.hWnd = m_hWnd;
	GetInfo.Period = nPeriod;

	if (info.overallType < eCAPointEnd &&
		info.overallType >= eCAPointStart)
	{
		return;
		//if (m_PicPeriod != Period_1Day)
		//	return;
		//GetInfo.Group = m_Group;
		//strcpy_s(GetInfo.StockID, StockID);
		//SendMsg(m_uParWndThreadID, MPA_GetCallAction,
		//	(char*)&GetInfo, sizeof(GetInfo));
	}
	else
	{
		if (info.overallType < eIndyMarketPointEnd)
		{
			GetInfo.Group = m_Group;
			strcpy_s(GetInfo.StockID, StockID);
		}
		else if (info.overallType < eL1IndyPointEnd
			&& info.overallType >= eL1IndyPointStart)
		{
			GetInfo.Group = Group_SWL1;
			strcpy_s(GetInfo.StockID, m_infoMap.hash[StockID].SWL1ID);
		}
		else if (info.overallType < eL2IndyPointEnd
			&& info.overallType >= eL2IndyPointStart)
		{
			GetInfo.Group = Group_SWL2;
			strcpy_s(GetInfo.StockID, m_infoMap.hash[StockID].SWL2ID);
		}

		SStringA strExMsg;
		strExMsg.Format("dataName:%s,dataRange:%s", info.srcDataName, info.dataInRange);

		GetInfo.exMsg = new char[strExMsg.GetLength() + 1];
		strcpy_s(GetInfo.exMsg, strExMsg.GetLength() + 1, strExMsg);

		m_WndPointSubMap[GetInfo.nAskGroup][GetInfo.StockID].emplace_back(GetInfo);
		SendMsg(g_WndSyn.GetThreadID(), Syn_GetPoint, (char*)&GetInfo, sizeof(GetInfo));

	}

}

bool SOUI::CDlgMultiPrdAnly::CheckDataIsGot(ShowPointInfo& info, int nPeriod)
{
	if (info.overallType < eCAPointEnd &&
		info.overallType >= eCAPointStart)
	{
		if (m_bCAInfoGet)
			return true;
		else
			return false;
	}
	auto pPointGetMap = &m_PointGetMap;
	if (info.overallType < eL1IndyPointEnd
		&& info.overallType >= eL1IndyPointStart)
		pPointGetMap = &m_L1IndyPointGetMap;
	else if (info.overallType < eL2IndyPointEnd
		&& info.overallType >= eL2IndyPointStart)
		pPointGetMap = &m_L2IndyPointGetMap;
	auto& dataGetMap = (*pPointGetMap)[nPeriod];

	if (eRpsPoint == info.type)
	{
		SStringA dataName520 = info.srcDataName + "Point520" + info.dataInRange;
		SStringA dataName2060 = info.srcDataName + "Point2060" + info.dataInRange;
		if (dataGetMap[dataName520] == 0 ||
			dataGetMap[dataName2060] == 0)
			return false;
	}
	else if (eSecPoint == info.type)
	{
		SStringA dataName = info.srcDataName + "Point" + info.dataInRange;
		if (dataGetMap[dataName] == 0)
			return false;
	}
	return true;
}

void	SOUI::CDlgMultiPrdAnly::SetKlineShowData(int nWndIndex, vector<ShowPointInfo>& infoVec,
	int nPeriod, BOOL bNeedReCalc, int nStartWnd)
{
	SStringA stockID = m_strSubStockID;
	vector<SStringA> nameVec;
	GetBelongingIndyName(nameVec);
	m_pKlinePic[nWndIndex]->SetBelongingIndy(nameVec, nStartWnd);
	int nShowNum = infoVec.size();
	if (nShowNum > 0)
	{
		vector<vector<vector<CoreData>*>> tmpDataArr(nShowNum);
		vector<vector<BOOL>> rightVec(nShowNum);
		vector<vector<SStringA>> dataNameVec(nShowNum);
		int* dataCount = new int[nShowNum];
		for (int i = 0; i < nShowNum; ++i)
		{
			auto& info = infoVec[i];
			auto pPointData = &m_PointData;
			if (info.overallType > eIndyMarketPointEnd)
			{
				if (info.overallType >= eL1IndyPointStart && info.overallType < eL1IndyPointEnd)
					pPointData = &m_L1IndyPointData;
				else if (info.overallType >= eL2IndyPointStart && info.overallType < eL2IndyPointEnd)
					pPointData = &m_L2IndyPointData;
			}


			dataCount[i] = m_PointDataCount[info.type];
			tmpDataArr[i].resize(dataCount[i]);
			rightVec[i].resize(dataCount[i]);
			for (int j = 0; j < dataCount[i]; ++j)
			{
				SStringA dataName = info.srcDataName +
					m_SubPicShowNameVec[info.type][info.dataInRange][j];
				tmpDataArr[i][j] = &(*pPointData)[nPeriod][dataName];
				dataNameVec[i].emplace_back(m_SubPicShowNameVec[info.type][info.dataInRange][j]);
				rightVec[i][j] = TRUE;
			}
		}

		m_pKlinePic[nWndIndex]->SetSubPicShowData(dataCount,
			tmpDataArr, rightVec, dataNameVec,
			stockID, m_StockName.hash[stockID], nStartWnd);

		delete[]dataCount;
		dataCount = nullptr;


	}

	m_pKlinePic[nWndIndex]->ChangePeriod(nPeriod, bNeedReCalc);

}

void SOUI::CDlgMultiPrdAnly::SetStockInfo(vector<StockInfo>& infoVec, strHash<SStringA>& StockNameMap)
{
	m_StockPassSet.resize(infoVec.size());
	for (auto& it : infoVec)
		m_infoMap.hash[it.SecurityID] = it;
	m_StockName = StockNameMap;
	m_pDlgKbElf->SetStockInfo(infoVec);

}

void SOUI::CDlgMultiPrdAnly::ChangeShowStock(SStringA StockID)
{
	if (m_strSubStockID == StockID)
		return;

	m_nCurMsgWnd = -1;

	SetDataFlagFalse();
	SStringA StockName = m_StockName.hash[StockID];
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		if (m_pKlinePic[i])
			m_pKlinePic[i]->ChangeShowStock(StockID, StockName);
	//获取分时数据
	DataGetInfo GetInfo;
	GetInfo.hWnd = m_hWnd;
	strcpy_s(GetInfo.oldStockID, m_strSubStockID);
	strcpy_s(GetInfo.StockID, StockID);
	GetInfo.Group = m_Group;
	GetInfo.Period = Period_FenShi;
	m_strSubStockID = StockID;
	m_strStockName = StrA2StrW(m_StockName.hash[m_strSubStockID]);
	PostMessage(WM_WINDOW_MSG, WDMsg_ChangeShowTitle, NULL);
	SendMsg(g_WndSyn.GetThreadID(), Syn_GetMarket,
		(char*)&GetInfo, sizeof(GetInfo));
	vector<ShowPointInfo>infoVec;
	//获取当前订阅数据

	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (!m_pKlinePic[i])
			continue;
		GetInfo.Period = m_nPeriod[i];
		if (Period_1Day == m_nPeriod[i])
		{
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetCallAction,
				(char*)&GetInfo, sizeof(GetInfo));
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetTradeVol,
				(char*)&GetInfo, sizeof(GetInfo));
		}

		SendMsg(g_WndSyn.GetThreadID(), Syn_GetKline,
			(char*)&GetInfo, sizeof(GetInfo));

		if (m_pKlinePic[i]->GetIsTFBaseDataUsed())
		{
			DataGetInfo GetInfo;
			GetInfo.hWnd = m_hWnd;
			strcpy_s(GetInfo.StockID, StockID);
			GetInfo.Group = m_Group;
			GetInfo.Period = m_nPeriod[i];
			SendMsg(g_WndSyn.GetThreadID(), Syn_GetHisTFBase,
				(char*)&GetInfo, sizeof(GetInfo));

		}

		vector<ShowPointInfo>infoVec;
		m_pKlinePic[i]->GetShowPointInfo(infoVec);
		for (auto& info : infoVec)
			GetPointData(info, StockID, m_nPeriod[i]);
		SetKlineShowData(i, infoVec, m_nPeriod[i], FALSE);
	}

}

void	SOUI::CDlgMultiPrdAnly::SetDataFlagFalse()
{
	m_bMarketGet = false;
	m_bCAInfoGet = false;
	m_bTradeVolGet = false;
	m_PointGetMap.clear();
	m_L1IndyPointGetMap.clear();
	m_L2IndyPointGetMap.clear();
	m_CallAction.clear();
	m_TradeVolData.clear();
	//m_PointGetMap.clear();

	m_KlineGetMap.clear();
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pKlinePic[i])
		{
			m_pKlinePic[i]->SetTodayMarketState(false);
			m_pKlinePic[i]->SetHisKlineState(false);
			m_pKlinePic[i]->SetHisPointState(false);
			m_pKlinePic[i]->SetHisCAInfoState(false);
			m_pKlinePic[i]->SetTFMarketState(false);

		}
	}
}

void SOUI::CDlgMultiPrdAnly::ProcData()
{
	int MsgId;
	char* info;
	int msgLength;
	while (true)
	{
		MsgId = RecvMsg(0, &info, msgLength, 0);
		if (MsgId == Msg_Exit)
		{
			delete[]info;
			info = nullptr;
			break;
		}
		auto pFuc = m_dataHandleMap[MsgId];
		if (pFuc)
			(this->*pFuc)(msgLength, info);
		delete[]info;
		info = nullptr;
	}

}

void SOUI::CDlgMultiPrdAnly::ChangeShowTimeRangeByPeriod(int& nStartTime, int& nEndTime, int nSrcPrd, int nDstPrd)
{
	if (nSrcPrd == nDstPrd)
		return;
	if (nSrcPrd == Period_1Day)
	{
		nStartTime = 930 + nDstPrd;
		if (nStartTime > 959)
			nStartTime += 40;
		nEndTime = 1500;
	}
	else if (nDstPrd == Period_1Day)
	{
		nStartTime = nEndTime = 0;
	}
	else
	{

		if (nDstPrd > nSrcPrd)
		{
			int nRatio = nDstPrd / nSrcPrd;
			int nStartMinCount = GetTradeMinCount(nStartTime);
			int nDstPos = (nStartMinCount / nSrcPrd + nRatio - 1) / nRatio;
			nStartTime = TradeMinCountToTime(nDstPos * nDstPrd);
			int nEndMinCount = GetTradeMinCount(nEndTime);
			nDstPos = (nEndMinCount / nSrcPrd + nRatio - 1) / nRatio;
			nEndTime = TradeMinCountToTime(nDstPos * nDstPrd);
		}
		else if (nDstPrd < nSrcPrd)
		{
			if (nDstPrd == Period_60Min)
				nDstPrd = 100;
			nStartTime = nStartTime - nDstPrd + nSrcPrd;
			if (nStartTime % 100 >= 60)
				nStartTime -= 40;
		}
	}
}

int SOUI::CDlgMultiPrdAnly::GetTradeMinCount(int nTime)
{
	int nMinCount = 0;
	int nHour = nTime / 100;
	nMinCount = nHour >= 13 ? 120 + (nHour - 13) * 60 : (nHour - 9) * 60 - 30;
	return nMinCount + nTime % 100;
}

int SOUI::CDlgMultiPrdAnly::TradeMinCountToTime(int nTradeMinCount)
{
	int nTime = 0;
	if (nTradeMinCount <= 120)
	{
		int nHour = (nTradeMinCount + 30) / 60 + 9;
		int nMin = (nTradeMinCount + 30) % 60;
		nTime = nHour * 100 + nMin;
	}
	else
	{
		int nHour = (nTradeMinCount - 120) / 60 + 13;
		int nMin = (nTradeMinCount - 120) % 60;
		nTime = nHour * 100 + nMin;
	}
	return nTime;
}


void CDlgMultiPrdAnly::GetBelongingIndyName(vector<SStringA>& nameVec)
{
	nameVec.resize(2);
	SStringA stockID = m_strSubStockID;
	//stockID = stockID.Left(6);
	const auto& info = m_infoMap.hash[m_strSubStockID];
	nameVec[0] = m_StockName.hash[info.SWL1ID];
	nameVec[1] = m_StockName.hash[info.SWL2ID];
}

void CDlgMultiPrdAnly::OnUpdatePoint(int nMsgLength, const char* info)
{
	int nDataCount = nMsgLength / sizeof(RtPointData);
	RtPointData* dataArr = (RtPointData*)info;
	for (int i = 0; i < nDataCount; ++i)
	{
		auto pPointData = &m_PointData;
		auto pPointGetMap = &m_PointGetMap;
		if (dataArr[i].stockID != m_strSubStockID)
		{
			if (strcmp(dataArr[i].stockID, m_infoMap.hash[m_strSubStockID].SWL1ID) == 0)
			{
				pPointData = &m_L1IndyPointData;
				pPointGetMap = &m_L1IndyPointGetMap;
			}
			else if (strcmp(dataArr[i].stockID, m_infoMap.hash[m_strSubStockID].SWL2ID) == 0)
			{
				pPointData = &m_L2IndyPointData;
				pPointGetMap = &m_L2IndyPointGetMap;
			}
		}

		if ((*pPointGetMap)[dataArr[i].period][dataArr[i].dataName])
		{
			UpdateTmData((*pPointData)[dataArr[i].period]\
				[dataArr[i].dataName],
				dataArr[i].data);
		}
	}
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		m_pKlinePic[i]->UpdateData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
}


void CDlgMultiPrdAnly::OnUpdateHisKline(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nSize = pRecvInfo->SrcDataSize / sizeof(KlineType);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	m_KlineGetMap[nPeriod] = TRUE;
	auto& KlineVec = m_KlineMap[nPeriod];
	KlineVec.resize(nSize);
	memcpy_s(&KlineVec[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_nPeriod[i] == nPeriod)
		{
			m_pKlinePic[i]->SetHisKlineState(true);
			if (m_pKlinePic[i]->GetDataReadyState())
				m_pKlinePic[i]->DataProc();
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);
		}
	}
}

void CDlgMultiPrdAnly::OnUpdateHisRpsPoint(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo*)(info + nOffset);

	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char* msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	pRecvInfo->TotalDataSize -=
		(nAttMsgSize + sizeof(nAttMsgSize) + sizeof(nMsgID));
	ProcHisRpsPointFromMsg(pRecvInfo, info + nOffset,
		"Point520", "Point2060", msg, nAttMsgSize);
	int nPeriod = pRecvInfo->Period;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_nPeriod[i] == nPeriod)
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);
	}

}


//void CDlgMultiPrdAnly::OnUpdateIndexMarket(int nMsgLength, const char* info)
//{
//	CommonIndexMarket* pIndexData = (CommonIndexMarket*)info;
//	SStringA SecurityID = pIndexData->SecurityID;
//	m_IndexMarketVec.emplace_back(*pIndexData);
//	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
//}

void CDlgMultiPrdAnly::OnUpdateStockMarket(int nMsgLength, const char* info)
{
	CommonStockMarket* pStockData = (CommonStockMarket*)info;
	SStringA SecurityID = pStockData->SecurityID;
	m_StockMarketVec.emplace_back(*pStockData);
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		m_pKlinePic[i]->UpdateData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
}

//void CDlgMultiPrdAnly::OnUpdateHisIndexMarket(int nMsgLength, const char* info)
//{
//	ReceiveInfo* pRecvInfo = (ReceiveInfo*)info;
//	nOffset += sizeof(*pRecvInfo);
//	int nMsgID = *(int*)(info + nOffset);
//	nOffset += sizeof(nMsgID);
//	int dataCount = pRecvInfo->SrcDataSize / sizeof(CommonIndexMarket);
//	CommonIndexMarket* dataArr = (CommonIndexMarket*)(info + nOffset);
//	m_IndexMarketVec.reserve(MAX_TICK);
//	m_IndexMarketVec.resize(dataCount);
//	memcpy_s(&m_IndexMarketVec[0], pRecvInfo->SrcDataSize,
//		dataArr, pRecvInfo->SrcDataSize);
//	m_bMarketGet = TRUE;
//	m_pFenShiPic->DataProc();
//	m_pKlinePic->SetTodayMarketState(true);
//	if (m_pKlinePic->GetDataReadyState())
//		m_pKlinePic->DataProc();
//	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, NULL);
//}

void CDlgMultiPrdAnly::OnUpdateHisStockMarket(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(CommonStockMarket);
	CommonStockMarket* dataArr = (CommonStockMarket*)(info + nOffset);
	m_StockMarketVec.reserve(MAX_TICK);
	m_StockMarketVec.resize(dataCount);
	memcpy_s(&m_StockMarketVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	m_bMarketGet = TRUE;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pKlinePic[i])
		{
			m_pKlinePic[i]->SetTodayMarketState(true);
			if (m_pKlinePic[i]->GetDataReadyState())
				m_pKlinePic[i]->DataProc();
		}
	}
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
}

//void CDlgMultiPrdAnly::OnUpdateCloseInfo(int nMsgLength, const char* info)
//{
//	pair<char[8], double>preCloseData;
//	int dataCount = nMsgLength / sizeof(preCloseData);
//	pair<char[8], double>* dataArr = (pair<char[8], double> *)info;
//	strHash<double> preCloseMap;
//	for (int i = 0; i < dataCount; ++i)
//		preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
//	m_preCloseMap.hash = preCloseMap.hash;
//	//for (int i = 0; i < dataCount; ++i)
//	//	m_preCloseMap.hash[dataArr[i].first] = dataArr[i].second;
//
//}

void CDlgMultiPrdAnly::OnUpdateHisSecPoint(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceivePointInfo* pRecvInfo = (ReceivePointInfo*)(info + nOffset);

	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nAttMsgSize = *(int*)(info + nOffset);
	nOffset += sizeof(nAttMsgSize);
	char* msg = new char[nAttMsgSize + 1];
	memcpy_s(msg, nAttMsgSize + 1, info + nOffset, nAttMsgSize);
	pRecvInfo->TotalDataSize -=
		(nAttMsgSize + sizeof(nAttMsgSize) + sizeof(nMsgID));
	//ReceivePointInfo *pRecvInfo1 =
	//	(ReceivePointInfo *)(info + nOffset);
	//nOffset += sizeof(*pRecvInfo1);
	ProcHisSecPointFromMsg(pRecvInfo, info + nOffset,
		"Point", msg, nAttMsgSize);
	int nPeriod = pRecvInfo->Period;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_nPeriod[i] == nPeriod)
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);
	}
}

void CDlgMultiPrdAnly::OnUpdateRehabInfo(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int dataCount = pRecvInfo->SrcDataSize / sizeof(RehabInfo);
	RehabInfo* dataArr = (RehabInfo*)(info + nOffset);
	vector<RehabInfo> rehabInfoVec;
	rehabInfoVec.resize(dataCount);
	memcpy_s(&rehabInfoVec[0], pRecvInfo->SrcDataSize,
		dataArr, pRecvInfo->SrcDataSize);
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		if (m_pKlinePic[i])
			m_pKlinePic[i]->SetRehabInfo(rehabInfoVec);

}

void SOUI::CDlgMultiPrdAnly::OnUpdateHisCallAction(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nSize = pRecvInfo->SrcDataSize / sizeof(CAInfo);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_CallAction.resize(nSize);
	memcpy_s(&m_CallAction[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	ProcHisCAPointFromCAInfo();
	m_bCAInfoGet = TRUE;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_pKlinePic[i])
		{
			m_pKlinePic[i]->SetHisCAInfoState(true);
			if (m_nPeriod[i] == Period_1Day)
				::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);
		}
	}
}


void CDlgMultiPrdAnly::OnUpdateHisTFBase(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nSize = pRecvInfo->SrcDataSize / sizeof(TFBaseMarket);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	auto& TFBaseVec = m_TFBaseMap[nPeriod];
	TFBaseVec.resize(nSize);
	memcpy_s(&TFBaseVec[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
}

void CDlgMultiPrdAnly::OnUpdateTodayTFMarket(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nSize = pRecvInfo->SrcDataSize / sizeof(TickFlowMarket);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	auto& TFMarkteVec = m_RtTFMarketVec[nPeriod];
	TFMarkteVec.resize(nSize);
	memcpy_s(&TFMarkteVec[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	m_TFBaseGetMap[nPeriod] = TRUE;

	if (nPeriod != Period_FenShi)
	{
		for (int i = 0; i < MAX_PIC_NUM; ++i)
		{
			if (m_nPeriod[i] == nPeriod && m_pKlinePic[i])
			{
				m_pKlinePic[i]->SetTFMarketState(true);
				m_pKlinePic[i]->UpdateData();
				::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);
			}
		}
	}


}

void CDlgMultiPrdAnly::OnUpdateRTTFMarket(int nMsgLength, const char* info)
{
	TickFlowMarket* pTickFlow = (TickFlowMarket*)info;
	int nDataCount = nMsgLength / sizeof(TickFlowMarket);
	for (int i = 0; i < nDataCount; ++i)
	{
		int nPeriod = pTickFlow[i].nPeriod;
		if (m_TFBaseGetMap[nPeriod])
		{
			if (m_RtTFMarketVec[nPeriod].empty())
				m_RtTFMarketVec[nPeriod].emplace_back(pTickFlow[i]);
			else
			{
				if (m_RtTFMarketVec[nPeriod].back().nTime != pTickFlow[i].nTime)
					m_RtTFMarketVec[nPeriod].emplace_back(pTickFlow[i]);
				else
					m_RtTFMarketVec[nPeriod].back() = pTickFlow[i];
			}
		}

	}
}

void CDlgMultiPrdAnly::OnUpdateRTPriceVol(int nMsgLength, const char* info)
{
	PriceVolInfo* pPriceVol = (PriceVolInfo*)(info);
	int nDataCount = nMsgLength / sizeof(PriceVolInfo);
	map<int, PriceVolInfo> priceVol;
	for (int i = 0; i < nDataCount; ++i)
		priceVol[pPriceVol[i].nPriceMulti100] = pPriceVol[i];
}

void SOUI::CDlgMultiPrdAnly::OnUpdateRTTradeVol(int nMsgLength, const char* info)
{
	TradeVol* pPriceVol = (TradeVol*)(info);
	int nDataCount = nMsgLength / sizeof(TradeVol);
	for (int i = 0; i < nDataCount; ++i)
	{
		if (m_TradeVolData.empty() || m_TradeVolData.back().nDate != pPriceVol[i].nDate)
			m_TradeVolData.emplace_back(pPriceVol[i]);
		else
			m_TradeVolData.back() = pPriceVol[i];

	}

	for (int i = 0; i < MAX_PIC_NUM; ++i)
		if (m_pKlinePic[i] && m_pKlinePic[i]->GetBigVolDiffState() && m_nPeriod[i] == Period_1Day)
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);

}

void SOUI::CDlgMultiPrdAnly::OnUpdateHisTradeVol(int nMsgLength, const char* info)
{
	int nOffset = sizeof(HWND);
	ReceiveInfo* pRecvInfo = (ReceiveInfo*)(info + nOffset);
	nOffset += sizeof(*pRecvInfo);
	int nMsgID = *(int*)(info + nOffset);
	nOffset += sizeof(nMsgID);
	int nSize = pRecvInfo->SrcDataSize / sizeof(TradeVol);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	m_TradeVolData.resize(nSize);
	memcpy_s(&m_TradeVolData[0], pRecvInfo->SrcDataSize,
		info + nOffset, pRecvInfo->SrcDataSize);
	m_bTradeVolGet = TRUE;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
	{
		if (m_nPeriod[i] == Period_1Day && m_pKlinePic[i])
		{
			m_pKlinePic[i]->SetHisVolDiffState(true);
			::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, i);

		}
	}

}

void SOUI::CDlgMultiPrdAnly::OnKlineChangeStock(int nMsgLength, const char* info)
{
	SStringA strStockID = info;
	ChangeShowStock(strStockID);
}

void CDlgMultiPrdAnly::OnKlineMa(int nMsgLength, const char* info)
{
	m_pKlinePic[m_nCurMsgWnd]->ReProcMAData(m_MaParaSet[m_nCurMsgWnd]);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, m_nCurMsgWnd);
}


void CDlgMultiPrdAnly::OnKlineMacd(int nMsgLength, const char* info)
{
	m_pKlinePic[m_nCurMsgWnd]->ReProcMacdData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, m_nCurMsgWnd);
}

void CDlgMultiPrdAnly::OnKlineBand(int nMsgLength, const char* info)
{
	m_pKlinePic[m_nCurMsgWnd]->ReProcBandData();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, m_nCurMsgWnd);
}

void SOUI::CDlgMultiPrdAnly::OnKlineTargetReCalc(int nMsgLength, const char* info)
{
	m_pKlinePic[m_nCurMsgWnd]->CalcTarget();
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, m_nCurMsgWnd);
}


void CDlgMultiPrdAnly::OnChangeKlineRehab(int nMsgLength, const char* info)
{
	int rehabType = *(int*)info;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		m_pKlinePic[i]->ReProcKlineRehabData((eRehabType)rehabType);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);
}

void CDlgMultiPrdAnly::OnFixedTimeRehab(int nMsgLength, const char* info)
{
	FixedTimeRehab& frt = *(FixedTimeRehab*)info;
	for (int i = 0; i < MAX_PIC_NUM; ++i)
		m_pKlinePic[i]->ReProcKlineRehabData(frt);
	::PostMessage(m_hWnd, WM_WINDOW_MSG, WDMsg_UpdatePic, MAX_PIC_NUM);

}


BOOL CDlgMultiPrdAnly::GetAttPara(char* msg, map<SStringA, SStringA>& paraMap)
{
	std::stringstream ss(msg);
	std::string buffer = "";
	while (getline(ss, buffer, ','))
	{
		size_t nPos = buffer.find(":");
		if (nPos != string::npos)
		{
			SStringA strName = buffer.substr(0, nPos).c_str();
			SStringA strData = buffer.substr(nPos + 1).c_str();
			paraMap[strName] = strData;
		}
	}

	return TRUE;
}

void CDlgMultiPrdAnly::ProcHisRpsPointFromMsg(ReceivePointInfo* pRecvInfo,
	const char* info, SStringA dataName1, SStringA dataName2,
	char* attchMsg, int attMsgSize)
{

	int nOffset = 0;
	int nSize520 = pRecvInfo->TotalDataSize / 2;
	int nSize2060 = nSize520;
	int nCount520 = nSize520 / sizeof(CoreData);
	int nCount2060 = nSize2060 / sizeof(CoreData);
	//int nSize520 = pRecvInfo->FirstDataSize / sizeof(CoreData);
	//int nSize2060 = (pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize) /
	//	sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	//vector<CoreData> PointVec(nSize520);
	//memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
	//	info + nOffset, pRecvInfo->FirstDataSize);
	map<SStringA, SStringA>attPara;
	GetAttPara(attchMsg, attPara);
	SStringA strDataName = attPara["dataName"];
	SStringA strRange = attPara["dataRange"];

	auto pPointData = &m_PointData;
	auto pPointGetMap = &m_PointGetMap;
	if (pRecvInfo->Message != m_strSubStockID)
	{
		if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStockID].SWL1ID) == 0)
		{
			pPointData = &m_L1IndyPointData;
			pPointGetMap = &m_L1IndyPointGetMap;
		}
		else if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStockID].SWL2ID) == 0)
		{
			pPointData = &m_L2IndyPointData;
			pPointGetMap = &m_L2IndyPointGetMap;
		}
	}


	nOffset += attMsgSize;
	SStringA point520Name = strDataName + dataName1 + strRange;
	auto& Point520Vec = (*pPointData)[nPeriod][point520Name];
	Point520Vec.resize(nCount520);
	memcpy_s(&Point520Vec[0], nSize520,
		info + nOffset, nSize520);

	//Point520Vec.insert(Point520Vec.begin(),
	//	PointVec.begin(), PointVec.end());
	//PointVec.resize(nSize2060);
	//memcpy_s(&PointVec[0], pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize,
	//	info + nOffset, pRecvInfo->TotalDataSize - pRecvInfo->FirstDataSize);
	SStringA point2060Name = strDataName + dataName2 + strRange;
	nOffset += nSize520;
	auto& Point2060Vec = (*pPointData)[nPeriod][strDataName + dataName2 + strRange];
	Point2060Vec.resize(nCount2060);
	memcpy_s(&Point2060Vec[0], nSize2060,
		info + nOffset, nSize2060);
	(*pPointGetMap)[nPeriod][point520Name] = TRUE;
	(*pPointGetMap)[nPeriod][point2060Name] = TRUE;

	//Point2060Vec.insert(Point2060Vec.begin(),
	//	PointVec.begin(), PointVec.end());
}

void CDlgMultiPrdAnly::ProcHisSecPointFromMsg(ReceivePointInfo* pRecvInfo,
	const char* info, SStringA dataName, char* attchMsg, int attMsgSize)
{
	int nOffset = 0;
	int nSize = pRecvInfo->FirstDataSize / sizeof(CoreData);
	int nGroup = pRecvInfo->Group;
	int nPeriod = pRecvInfo->Period;
	map<SStringA, SStringA>attPara;
	GetAttPara(attchMsg, attPara);
	SStringA strDataName = attPara["dataName"];
	SStringA strRange = attPara["dataRange"];
	SStringA pointName = strDataName + dataName + strRange;

	auto pPointData = &m_PointData;
	auto pPointGetMap = &m_PointGetMap;
	if (pRecvInfo->Message != m_strSubStockID)
	{
		if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStockID].SWL1ID) == 0)
		{
			pPointData = &m_L1IndyPointData;
			pPointGetMap = &m_L1IndyPointGetMap;
		}
		else if (strcmp(pRecvInfo->Message, m_infoMap.hash[m_strSubStockID].SWL2ID) == 0)
		{
			pPointData = &m_L2IndyPointData;
			pPointGetMap = &m_L2IndyPointGetMap;
		}
	}

	nOffset += attMsgSize;
	auto& PointVec = (*pPointData)[nPeriod][pointName];
	PointVec.resize(nSize);
	memcpy_s(&PointVec[0], pRecvInfo->FirstDataSize,
		info + nOffset, pRecvInfo->FirstDataSize);

	(*pPointGetMap)[nPeriod][pointName] = TRUE;
}

void CDlgMultiPrdAnly::ProcHisCAPointFromCAInfo()
{
	auto& CAVolPointVec = m_PointData[Period_1Day]["CaVolPoint"];
	auto& CAVolPointL1Vec = m_PointData[Period_1Day]["CaVolPointL1"];
	auto& CAVolPointL2Vec = m_PointData[Period_1Day]["CaVolPointL2"];
	auto& CAAmoPointVec = m_PointData[Period_1Day]["CaAmoPoint"];
	auto& CAAmoPointL1Vec = m_PointData[Period_1Day]["CaAmoPointL1"];
	auto& CAAmoPointL2Vec = m_PointData[Period_1Day]["CaAmoPointL2"];
	CAVolPointVec.reserve(m_CallAction.size());
	CAVolPointL1Vec.reserve(m_CallAction.size());
	CAVolPointL2Vec.reserve(m_CallAction.size());
	CAAmoPointVec.reserve(m_CallAction.size());
	CAAmoPointL1Vec.reserve(m_CallAction.size());
	CAAmoPointL2Vec.reserve(m_CallAction.size());

	for (auto& it : m_CallAction)
	{
		CoreData data = { 0 };
		data.date = it.date;
		data.value = it.VolPoint;
		CAVolPointVec.emplace_back(data);
		data.value = it.VolPointL1;
		CAVolPointL1Vec.emplace_back(data);
		data.value = it.VolPointL2;
		CAVolPointL2Vec.emplace_back(data);
		data.value = it.AmoPoint;
		CAAmoPointVec.emplace_back(data);
		data.value = it.AmoPointL1;
		CAAmoPointL1Vec.emplace_back(data);
		data.value = it.AmoPointL2;
		CAAmoPointL2Vec.emplace_back(data);

	}
	m_PointGetMap[Period_1Day]["CaVolPoint"] = TRUE;
	m_PointGetMap[Period_1Day]["CaVolPointL1"] = TRUE;
	m_PointGetMap[Period_1Day]["CaVolPointL2"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPoint"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPointL1"] = TRUE;
	m_PointGetMap[Period_1Day]["CaAmoPointL2"] = TRUE;

}

void CDlgMultiPrdAnly::UpdateTmData(vector<CoreData>& comData, CoreData& data)
{
	if (comData.empty())
		comData.emplace_back(data);
	else if (comData.back().date < data.date)
		comData.emplace_back(data);
	else if (comData.back().date == data.date)
	{
		if (comData.back().time < data.time)
			comData.emplace_back(data);
		else if (comData.back().time == data.time)
			comData.back() = data;
	}
}
