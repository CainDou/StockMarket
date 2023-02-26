#pragma once
#include "control\SRichEdit.h"
namespace SOUI
{
	class SCapsRichEdit :public SRichEdit
	{
		SOUI_CLASS_NAME(SCapsRichEdit, L"capsrichedit")	//定义xml标签

	public:
		SCapsRichEdit();
		~SCapsRichEdit();
		BOOL GetCharFormat(CHARFORMAT2W* pcf);
		CHARFORMAT2W SetCharFormat(CHARFORMAT2W* pcf);
		BOOL InsertStr(SStringW str);
		BOOL GetEditDirty();
		void ClearDirty();
		BOOL GetImmInput();

	protected:
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	protected:
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_KEYUP(OnKeyUp)
			//MSG_WM_KEYDOWN(OnKeyDown)
		SOUI_MSG_MAP_END()


	protected:
		BOOL m_bDirty;
		BOOL m_bImm;
	};


	class SCapsEdit :public SEdit
	{

		SOUI_CLASS_NAME(SCapsRichEdit, L"capsedit")	//定义xml标签

	public:
		SCapsEdit();
		~SCapsEdit();

	protected:
		void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	protected:
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_CHAR(OnChar)
			SOUI_MSG_MAP_END()

	};

	inline BOOL SCapsRichEdit::GetEditDirty()
	{
		return m_bDirty;
	}

	inline void SCapsRichEdit::ClearDirty()
	{
		m_bDirty = FALSE;
	}
	inline BOOL SCapsRichEdit::GetImmInput()
	{
		return m_bImm;
	}
}

