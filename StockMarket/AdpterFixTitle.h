#pragma once
#include <helper/SAdapterBase.h>
class CAdpterFixTitle : public SAdapterBase
{
public:
	CAdpterFixTitle(int nSize, map<int, BOOL>&bShowMap, map<int, SStringW>& titleMap);
	~CAdpterFixTitle();

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate);

	BOOL bItemChecked(int nPos);
	void SetItemChecked(int nPos,BOOL bChecked);
	void SetItemStr(int nPos,SStringW strItem);
	bool OnCheckClick(EventCmd *pEvt);
	map<int, BOOL> GetTitleShowMap();

	virtual int getCount();

protected:
	vector<SCheckBox*> m_chkVec;
	map<int, BOOL> m_bShowMap;
	map<int, SStringW>m_titleMap;
	int nItemSize;

};

inline map<int, BOOL> CAdpterFixTitle::GetTitleShowMap()
{
	return m_bShowMap;
}
