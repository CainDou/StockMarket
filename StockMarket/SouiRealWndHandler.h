#pragma once
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{
	class CSouiRealWndHandler :public TObjRefImpl2<IRealWndHandler, CSouiRealWndHandler>
	{
	public:
		CSouiRealWndHandler();
		~CSouiRealWndHandler();
		virtual HWND OnRealWndCreate(SRealWnd *pRealWnd);
		virtual void OnRealWndDestroy(SRealWnd *pRealWnd);
		virtual BOOL OnRealWndInit(SRealWnd *pRealWnd);
		virtual BOOL OnRealWndSize(SRealWnd *pRealWnd);
	};
}

