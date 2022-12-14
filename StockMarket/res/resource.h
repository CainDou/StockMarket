//stamp:231155fffb87642a
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#pragma once
#include <res.mgr/snamedvalue.h>
#define ROBJ_IN_CPP \
namespace SOUI \
{\
    const _R R;\
    const _UIRES UIRES;\
}
namespace SOUI
{
	class _UIRES{
		public:
		class _UIDEF{
			public:
			_UIDEF(){
				XML_INIT = _T("UIDEF:XML_INIT");
			}
			const TCHAR * XML_INIT;
		}UIDEF;
		class _LAYOUT{
			public:
			_LAYOUT(){
				XML_MAINWND = _T("LAYOUT:XML_MAINWND");
				dlg_keyboardElf = _T("LAYOUT:dlg_keyboardElf");
				dlg_maPara = _T("LAYOUT:dlg_maPara");
				dlg_login = _T("LAYOUT:dlg_login");
				dlg_macdPara = _T("LAYOUT:dlg_macdPara");
				dlg_BandPara = _T("LAYOUT:dlg_BandPara");
				dlg_emaPara = _T("LAYOUT:dlg_emaPara");
				wnd_work = _T("LAYOUT:wnd_work");
				page_workwnd = _T("LAYOUT:page_workwnd");
				dlg_subWindow = _T("LAYOUT:dlg_subWindow");
				dlg_newWindow = _T("LAYOUT:dlg_newWindow");
				dlg_openWindow = _T("LAYOUT:dlg_openWindow");
				dlg_stockFilter = _T("LAYOUT:dlg_stockFilter");
			}
			const TCHAR * XML_MAINWND;
			const TCHAR * dlg_keyboardElf;
			const TCHAR * dlg_maPara;
			const TCHAR * dlg_login;
			const TCHAR * dlg_macdPara;
			const TCHAR * dlg_BandPara;
			const TCHAR * dlg_emaPara;
			const TCHAR * wnd_work;
			const TCHAR * page_workwnd;
			const TCHAR * dlg_subWindow;
			const TCHAR * dlg_newWindow;
			const TCHAR * dlg_openWindow;
			const TCHAR * dlg_stockFilter;
		}LAYOUT;
		class _values{
			public:
			_values(){
				string = _T("values:string");
				color = _T("values:color");
				skin = _T("values:skin");
			}
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
		}values;
		class _IMG{
			public:
			_IMG(){
				default_scrollbar_png = _T("IMG:default_scrollbar_png");
				skin_lcex_header_arrow = _T("IMG:skin_lcex_header_arrow");
			}
			const TCHAR * default_scrollbar_png;
			const TCHAR * skin_lcex_header_arrow;
		}IMG;
		class _ICON{
			public:
			_ICON(){
				ICON_LOGO = _T("ICON:ICON_LOGO");
			}
			const TCHAR * ICON_LOGO;
		}ICON;
		class _smenu{
			public:
			_smenu(){
				menu_fenshi = _T("smenu:menu_fenshi");
				menu_kline = _T("smenu:menu_kline");
				menu_new = _T("smenu:menu_new");
			}
			const TCHAR * menu_fenshi;
			const TCHAR * menu_kline;
			const TCHAR * menu_new;
		}smenu;
	};
	const SNamedID::NAMEDVALUE namedXmlID[]={
		{L"_name_start",65535},
		{L"btn_BandCancel",65575},
		{L"btn_BandClose",65567},
		{L"btn_BandDefault",65576},
		{L"btn_BandOK",65574},
		{L"btn_BandSetDefault",65577},
		{L"btn_Day",65593},
		{L"btn_EMACancel",65582},
		{L"btn_EMAClose",65578},
		{L"btn_EMADefault",65583},
		{L"btn_EMAOK",65581},
		{L"btn_EMASetDefault",65584},
		{L"btn_FS",65587},
		{L"btn_ListConnect1",65594},
		{L"btn_ListConnect2",65595},
		{L"btn_M1",65588},
		{L"btn_M15",65590},
		{L"btn_M30",65591},
		{L"btn_M5",65589},
		{L"btn_M60",65592},
		{L"btn_MACDCancel",65564},
		{L"btn_MACDDefault",65565},
		{L"btn_MACDOK",65563},
		{L"btn_MACDSetDefault",65566},
		{L"btn_MaCancel",65551},
		{L"btn_MaClose",65545},
		{L"btn_MaDefault",65552},
		{L"btn_MaOK",65550},
		{L"btn_MaSetDefault",65553},
		{L"btn_MacdClose",65559},
		{L"btn_Market",65586},
		{L"btn_OK",65610},
		{L"btn_StockFilter",65596},
		{L"btn_cancel",65611},
		{L"btn_close",65537},
		{L"btn_kbElfClose",65542},
		{L"btn_login",65556},
		{L"btn_max",65538},
		{L"btn_min",65540},
		{L"btn_mulitWindow",65536},
		{L"btn_quit",65557},
		{L"btn_restore",65539},
		{L"cbx_ID",65618},
		{L"cbx_condition",65624},
		{L"cbx_func",65616},
		{L"cbx_index1",65620},
		{L"cbx_index2",65626},
		{L"cbx_open",65612},
		{L"cbx_period1",65622},
		{L"cbx_period2",65629},
		{L"chk_NewStock",65601},
		{L"chk_SBM",65599},
		{L"chk_ST",65598},
		{L"chk_STARM",65600},
		{L"chk_use",65613},
		{L"edit_BandParaK",65570},
		{L"edit_BandParaM1",65571},
		{L"edit_BandParaM2",65572},
		{L"edit_BandParaN1",65568},
		{L"edit_BandParaN2",65569},
		{L"edit_BandParaP",65573},
		{L"edit_EMAPara1",65579},
		{L"edit_EMAPara2",65580},
		{L"edit_MacdPara1",65560},
		{L"edit_MacdPara2",65561},
		{L"edit_MacdPara3",65562},
		{L"edit_kbElf",65543},
		{L"edit_maPara1",65546},
		{L"edit_maPara2",65547},
		{L"edit_maPara3",65548},
		{L"edit_maPara4",65549},
		{L"edit_name",65609},
		{L"edit_num",65630},
		{L"fenshiPic",65603},
		{L"klinePic",65604},
		{L"list_kbElf",65544},
		{L"login_ID",65554},
		{L"login_PassWord",65555},
		{L"ls_filter",65614},
		{L"ls_rps",65602},
		{L"text_Group",65585},
		{L"text_ID",65617},
		{L"text_ShowIndy",65597},
		{L"text_condition",65623},
		{L"text_func",65615},
		{L"text_index1",65619},
		{L"text_index2",65625},
		{L"text_num",65628},
		{L"text_period1",65621},
		{L"text_period2",65627},
		{L"text_windowName",65608},
		{L"txt_login",65558},
		{L"txt_title",65541},
		{L"wnd_SWL1",65605},
		{L"wnd_SWL2",65606},
		{L"wnd_Stock",65607}		};
	class _R{
	public:
		class _name{
		public:
		_name(){
			_name_start = namedXmlID[0].strName;
			btn_BandCancel = namedXmlID[1].strName;
			btn_BandClose = namedXmlID[2].strName;
			btn_BandDefault = namedXmlID[3].strName;
			btn_BandOK = namedXmlID[4].strName;
			btn_BandSetDefault = namedXmlID[5].strName;
			btn_Day = namedXmlID[6].strName;
			btn_EMACancel = namedXmlID[7].strName;
			btn_EMAClose = namedXmlID[8].strName;
			btn_EMADefault = namedXmlID[9].strName;
			btn_EMAOK = namedXmlID[10].strName;
			btn_EMASetDefault = namedXmlID[11].strName;
			btn_FS = namedXmlID[12].strName;
			btn_ListConnect1 = namedXmlID[13].strName;
			btn_ListConnect2 = namedXmlID[14].strName;
			btn_M1 = namedXmlID[15].strName;
			btn_M15 = namedXmlID[16].strName;
			btn_M30 = namedXmlID[17].strName;
			btn_M5 = namedXmlID[18].strName;
			btn_M60 = namedXmlID[19].strName;
			btn_MACDCancel = namedXmlID[20].strName;
			btn_MACDDefault = namedXmlID[21].strName;
			btn_MACDOK = namedXmlID[22].strName;
			btn_MACDSetDefault = namedXmlID[23].strName;
			btn_MaCancel = namedXmlID[24].strName;
			btn_MaClose = namedXmlID[25].strName;
			btn_MaDefault = namedXmlID[26].strName;
			btn_MaOK = namedXmlID[27].strName;
			btn_MaSetDefault = namedXmlID[28].strName;
			btn_MacdClose = namedXmlID[29].strName;
			btn_Market = namedXmlID[30].strName;
			btn_OK = namedXmlID[31].strName;
			btn_StockFilter = namedXmlID[32].strName;
			btn_cancel = namedXmlID[33].strName;
			btn_close = namedXmlID[34].strName;
			btn_kbElfClose = namedXmlID[35].strName;
			btn_login = namedXmlID[36].strName;
			btn_max = namedXmlID[37].strName;
			btn_min = namedXmlID[38].strName;
			btn_mulitWindow = namedXmlID[39].strName;
			btn_quit = namedXmlID[40].strName;
			btn_restore = namedXmlID[41].strName;
			cbx_ID = namedXmlID[42].strName;
			cbx_condition = namedXmlID[43].strName;
			cbx_func = namedXmlID[44].strName;
			cbx_index1 = namedXmlID[45].strName;
			cbx_index2 = namedXmlID[46].strName;
			cbx_open = namedXmlID[47].strName;
			cbx_period1 = namedXmlID[48].strName;
			cbx_period2 = namedXmlID[49].strName;
			chk_NewStock = namedXmlID[50].strName;
			chk_SBM = namedXmlID[51].strName;
			chk_ST = namedXmlID[52].strName;
			chk_STARM = namedXmlID[53].strName;
			chk_use = namedXmlID[54].strName;
			edit_BandParaK = namedXmlID[55].strName;
			edit_BandParaM1 = namedXmlID[56].strName;
			edit_BandParaM2 = namedXmlID[57].strName;
			edit_BandParaN1 = namedXmlID[58].strName;
			edit_BandParaN2 = namedXmlID[59].strName;
			edit_BandParaP = namedXmlID[60].strName;
			edit_EMAPara1 = namedXmlID[61].strName;
			edit_EMAPara2 = namedXmlID[62].strName;
			edit_MacdPara1 = namedXmlID[63].strName;
			edit_MacdPara2 = namedXmlID[64].strName;
			edit_MacdPara3 = namedXmlID[65].strName;
			edit_kbElf = namedXmlID[66].strName;
			edit_maPara1 = namedXmlID[67].strName;
			edit_maPara2 = namedXmlID[68].strName;
			edit_maPara3 = namedXmlID[69].strName;
			edit_maPara4 = namedXmlID[70].strName;
			edit_name = namedXmlID[71].strName;
			edit_num = namedXmlID[72].strName;
			fenshiPic = namedXmlID[73].strName;
			klinePic = namedXmlID[74].strName;
			list_kbElf = namedXmlID[75].strName;
			login_ID = namedXmlID[76].strName;
			login_PassWord = namedXmlID[77].strName;
			ls_filter = namedXmlID[78].strName;
			ls_rps = namedXmlID[79].strName;
			text_Group = namedXmlID[80].strName;
			text_ID = namedXmlID[81].strName;
			text_ShowIndy = namedXmlID[82].strName;
			text_condition = namedXmlID[83].strName;
			text_func = namedXmlID[84].strName;
			text_index1 = namedXmlID[85].strName;
			text_index2 = namedXmlID[86].strName;
			text_num = namedXmlID[87].strName;
			text_period1 = namedXmlID[88].strName;
			text_period2 = namedXmlID[89].strName;
			text_windowName = namedXmlID[90].strName;
			txt_login = namedXmlID[91].strName;
			txt_title = namedXmlID[92].strName;
			wnd_SWL1 = namedXmlID[93].strName;
			wnd_SWL2 = namedXmlID[94].strName;
			wnd_Stock = namedXmlID[95].strName;
		}
		 const wchar_t * _name_start;
		 const wchar_t * btn_BandCancel;
		 const wchar_t * btn_BandClose;
		 const wchar_t * btn_BandDefault;
		 const wchar_t * btn_BandOK;
		 const wchar_t * btn_BandSetDefault;
		 const wchar_t * btn_Day;
		 const wchar_t * btn_EMACancel;
		 const wchar_t * btn_EMAClose;
		 const wchar_t * btn_EMADefault;
		 const wchar_t * btn_EMAOK;
		 const wchar_t * btn_EMASetDefault;
		 const wchar_t * btn_FS;
		 const wchar_t * btn_ListConnect1;
		 const wchar_t * btn_ListConnect2;
		 const wchar_t * btn_M1;
		 const wchar_t * btn_M15;
		 const wchar_t * btn_M30;
		 const wchar_t * btn_M5;
		 const wchar_t * btn_M60;
		 const wchar_t * btn_MACDCancel;
		 const wchar_t * btn_MACDDefault;
		 const wchar_t * btn_MACDOK;
		 const wchar_t * btn_MACDSetDefault;
		 const wchar_t * btn_MaCancel;
		 const wchar_t * btn_MaClose;
		 const wchar_t * btn_MaDefault;
		 const wchar_t * btn_MaOK;
		 const wchar_t * btn_MaSetDefault;
		 const wchar_t * btn_MacdClose;
		 const wchar_t * btn_Market;
		 const wchar_t * btn_OK;
		 const wchar_t * btn_StockFilter;
		 const wchar_t * btn_cancel;
		 const wchar_t * btn_close;
		 const wchar_t * btn_kbElfClose;
		 const wchar_t * btn_login;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_mulitWindow;
		 const wchar_t * btn_quit;
		 const wchar_t * btn_restore;
		 const wchar_t * cbx_ID;
		 const wchar_t * cbx_condition;
		 const wchar_t * cbx_func;
		 const wchar_t * cbx_index1;
		 const wchar_t * cbx_index2;
		 const wchar_t * cbx_open;
		 const wchar_t * cbx_period1;
		 const wchar_t * cbx_period2;
		 const wchar_t * chk_NewStock;
		 const wchar_t * chk_SBM;
		 const wchar_t * chk_ST;
		 const wchar_t * chk_STARM;
		 const wchar_t * chk_use;
		 const wchar_t * edit_BandParaK;
		 const wchar_t * edit_BandParaM1;
		 const wchar_t * edit_BandParaM2;
		 const wchar_t * edit_BandParaN1;
		 const wchar_t * edit_BandParaN2;
		 const wchar_t * edit_BandParaP;
		 const wchar_t * edit_EMAPara1;
		 const wchar_t * edit_EMAPara2;
		 const wchar_t * edit_MacdPara1;
		 const wchar_t * edit_MacdPara2;
		 const wchar_t * edit_MacdPara3;
		 const wchar_t * edit_kbElf;
		 const wchar_t * edit_maPara1;
		 const wchar_t * edit_maPara2;
		 const wchar_t * edit_maPara3;
		 const wchar_t * edit_maPara4;
		 const wchar_t * edit_name;
		 const wchar_t * edit_num;
		 const wchar_t * fenshiPic;
		 const wchar_t * klinePic;
		 const wchar_t * list_kbElf;
		 const wchar_t * login_ID;
		 const wchar_t * login_PassWord;
		 const wchar_t * ls_filter;
		 const wchar_t * ls_rps;
		 const wchar_t * text_Group;
		 const wchar_t * text_ID;
		 const wchar_t * text_ShowIndy;
		 const wchar_t * text_condition;
		 const wchar_t * text_func;
		 const wchar_t * text_index1;
		 const wchar_t * text_index2;
		 const wchar_t * text_num;
		 const wchar_t * text_period1;
		 const wchar_t * text_period2;
		 const wchar_t * text_windowName;
		 const wchar_t * txt_login;
		 const wchar_t * txt_title;
		 const wchar_t * wnd_SWL1;
		 const wchar_t * wnd_SWL2;
		 const wchar_t * wnd_Stock;
		}name;

		class _id{
		public:
		const static int _name_start	=	65535;
		const static int btn_BandCancel	=	65575;
		const static int btn_BandClose	=	65567;
		const static int btn_BandDefault	=	65576;
		const static int btn_BandOK	=	65574;
		const static int btn_BandSetDefault	=	65577;
		const static int btn_Day	=	65593;
		const static int btn_EMACancel	=	65582;
		const static int btn_EMAClose	=	65578;
		const static int btn_EMADefault	=	65583;
		const static int btn_EMAOK	=	65581;
		const static int btn_EMASetDefault	=	65584;
		const static int btn_FS	=	65587;
		const static int btn_ListConnect1	=	65594;
		const static int btn_ListConnect2	=	65595;
		const static int btn_M1	=	65588;
		const static int btn_M15	=	65590;
		const static int btn_M30	=	65591;
		const static int btn_M5	=	65589;
		const static int btn_M60	=	65592;
		const static int btn_MACDCancel	=	65564;
		const static int btn_MACDDefault	=	65565;
		const static int btn_MACDOK	=	65563;
		const static int btn_MACDSetDefault	=	65566;
		const static int btn_MaCancel	=	65551;
		const static int btn_MaClose	=	65545;
		const static int btn_MaDefault	=	65552;
		const static int btn_MaOK	=	65550;
		const static int btn_MaSetDefault	=	65553;
		const static int btn_MacdClose	=	65559;
		const static int btn_Market	=	65586;
		const static int btn_OK	=	65610;
		const static int btn_StockFilter	=	65596;
		const static int btn_cancel	=	65611;
		const static int btn_close	=	65537;
		const static int btn_kbElfClose	=	65542;
		const static int btn_login	=	65556;
		const static int btn_max	=	65538;
		const static int btn_min	=	65540;
		const static int btn_mulitWindow	=	65536;
		const static int btn_quit	=	65557;
		const static int btn_restore	=	65539;
		const static int cbx_ID	=	65618;
		const static int cbx_condition	=	65624;
		const static int cbx_func	=	65616;
		const static int cbx_index1	=	65620;
		const static int cbx_index2	=	65626;
		const static int cbx_open	=	65612;
		const static int cbx_period1	=	65622;
		const static int cbx_period2	=	65629;
		const static int chk_NewStock	=	65601;
		const static int chk_SBM	=	65599;
		const static int chk_ST	=	65598;
		const static int chk_STARM	=	65600;
		const static int chk_use	=	65613;
		const static int edit_BandParaK	=	65570;
		const static int edit_BandParaM1	=	65571;
		const static int edit_BandParaM2	=	65572;
		const static int edit_BandParaN1	=	65568;
		const static int edit_BandParaN2	=	65569;
		const static int edit_BandParaP	=	65573;
		const static int edit_EMAPara1	=	65579;
		const static int edit_EMAPara2	=	65580;
		const static int edit_MacdPara1	=	65560;
		const static int edit_MacdPara2	=	65561;
		const static int edit_MacdPara3	=	65562;
		const static int edit_kbElf	=	65543;
		const static int edit_maPara1	=	65546;
		const static int edit_maPara2	=	65547;
		const static int edit_maPara3	=	65548;
		const static int edit_maPara4	=	65549;
		const static int edit_name	=	65609;
		const static int edit_num	=	65630;
		const static int fenshiPic	=	65603;
		const static int klinePic	=	65604;
		const static int list_kbElf	=	65544;
		const static int login_ID	=	65554;
		const static int login_PassWord	=	65555;
		const static int ls_filter	=	65614;
		const static int ls_rps	=	65602;
		const static int text_Group	=	65585;
		const static int text_ID	=	65617;
		const static int text_ShowIndy	=	65597;
		const static int text_condition	=	65623;
		const static int text_func	=	65615;
		const static int text_index1	=	65619;
		const static int text_index2	=	65625;
		const static int text_num	=	65628;
		const static int text_period1	=	65621;
		const static int text_period2	=	65627;
		const static int text_windowName	=	65608;
		const static int txt_login	=	65558;
		const static int txt_title	=	65541;
		const static int wnd_SWL1	=	65605;
		const static int wnd_SWL2	=	65606;
		const static int wnd_Stock	=	65607;
		}id;

		class _string{
		public:
		const static int title	=	0;
		const static int ver	=	1;
		}string;

		class _color{
		public:
		const static int blue	=	0;
		const static int gray	=	1;
		const static int green	=	2;
		const static int red	=	3;
		const static int white	=	4;
		}color;

	};

#ifdef R_IN_CPP
	 extern const _R R;
	 extern const _UIRES UIRES;
#else
	 extern const __declspec(selectany) _R & R = _R();
	 extern const __declspec(selectany) _UIRES & UIRES = _UIRES();
#endif//R_IN_CPP
}
