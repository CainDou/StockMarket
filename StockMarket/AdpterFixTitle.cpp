#include "stdafx.h"
#include "AdpterFixTitle.h"
#include "Define.h"


CAdpterFixTitle::CAdpterFixTitle(int nSize, map<int, BOOL>&bShowMap, map<int, SStringW>& titleMap)
{
	nItemSize = nSize;
	m_bShowMap = bShowMap;
	m_titleMap = titleMap;
	m_chkVec.resize(nItemSize);
}


CAdpterFixTitle::~CAdpterFixTitle()
{
}

void CAdpterFixTitle::getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
{
	if (pItem->GetChildrenCount() == 0)
	{
		pItem->InitFromXml(xmlTemplate);
	}

	SCheckBox *pChk = pItem->FindChildByName2<SCheckBox>(L"chk_title");
	int nTitle = position + SHead_CloseRPS520;
	pChk->GetRoot()->SetUserData(nTitle);
	pChk->SetCheck(m_bShowMap[nTitle]);
	pChk->SetWindowTextW(m_titleMap[nTitle]);
	pChk->GetEventSet()->subscribeEvent(&CAdpterFixTitle::OnCheckClick, this);

}

BOOL CAdpterFixTitle::bItemChecked(int nPos)
{
	return m_chkVec[nPos]->IsChecked();
}

void CAdpterFixTitle::SetItemChecked(int nPos, BOOL bChecked)
{
	m_chkVec[nPos]->SetCheck(bChecked);
}

void CAdpterFixTitle::SetItemStr(int nPos, SStringW strItem)
{
	m_chkVec[nPos]->SetWindowTextW(strItem);
}

bool CAdpterFixTitle::OnCheckClick(EventCmd * pEvt)
{
	SCheckBox* pChk = sobj_cast<SCheckBox>(pEvt->sender);
	int nTitlePos = pChk->GetRoot()->GetUserData();
	m_bShowMap[nTitlePos] = pChk->IsChecked();
	EventLVSelChanged evt();
	return true;
}

int CAdpterFixTitle::getCount()
{
	return nItemSize;
}
