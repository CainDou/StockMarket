// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <com-cfg.h>
#include "resource.h"
#define R_IN_CPP	//定义这个开关来
#include "res\resource.h"
using namespace SOUI;

#include"Define.h"
#define WM_WINDOW_MSG (WM_USER+155)
#define WM_LOGIN_MSG (WM_USER+156)
#define WM_FENSHI_MSG (WM_USER+157)
#define WM_KLINE_MSG (WM_USER+158)
#define WM_DATA_MSG (WM_USER+159)
#define WM_FUNC_MSG (WM_USER+160)
#define WM_FORMULA_MSG (WM_USER+161)
#define WM_FILTER_MSG (WM_USER+162)
#define WM_BACKTESTING_MSG (WM_USER+163)
#define WM_LIMITUP_MSG (WM_USER+164)

