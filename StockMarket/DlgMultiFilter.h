#pragma once
#include<thread>
#include"WorkWnd.h"

namespace SOUI
{
	class CDlgMultiFilter : public SHostWnd
	{
		typedef void(CDlgMultiFilter::*PDATAHANDLEFUNC)(int, const char*);

	public:
		CDlgMultiFilter();
		~CDlgMultiFilter();
	};

}

