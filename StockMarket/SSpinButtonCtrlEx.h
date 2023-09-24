#pragma once
#include <control\SSpinButtonCtrl.h>
namespace SOUI
{
	class SSpinButtonCtrlEx :
		public SSpinButtonCtrl
	{
		SOUI_CLASS_NAME(SSpinButtonCtrl, L"spinButtonEx")

	public:
		SSpinButtonCtrlEx();
		~SSpinButtonCtrlEx();

		int GetAction();
	};

	inline int SSpinButtonCtrlEx::GetAction()
	{
		return m_iActionBtn;
	}
}

