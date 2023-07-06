//stamp:428e90fdc64f704d
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
				dlg_FilterFrmlEditor = _T("LAYOUT:dlg_FilterFrmlEditor");
				dlg_FuncInsert = _T("LAYOUT:dlg_FuncInsert");
				dlg_FrmlManage = _T("LAYOUT:dlg_FrmlManage");
				dlg_FrmlFind = _T("LAYOUT:dlg_FrmlFind");
				dlg_SaveFrmlList = _T("LAYOUT:dlg_SaveFrmlList");
				dlg_ReadFrmlList = _T("LAYOUT:dlg_ReadFrmlList");
				dlg_messagebox = _T("LAYOUT:dlg_messagebox");
				dlg_targetSelect = _T("LAYOUT:dlg_targetSelect");
				dlg_comboStockFilter = _T("LAYOUT:dlg_comboStockFilter");
				dlg_headerSelect = _T("LAYOUT:dlg_headerSelect");
				dlg_rehabFixedTime = _T("LAYOUT:dlg_rehabFixedTime");
				dlg_updateLog = _T("LAYOUT:dlg_updateLog");
				dlg_condBackTesting = _T("LAYOUT:dlg_condBackTesting");
				dlg_multiFilter = _T("LAYOUT:dlg_multiFilter");
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
			const TCHAR * dlg_FilterFrmlEditor;
			const TCHAR * dlg_FuncInsert;
			const TCHAR * dlg_FrmlManage;
			const TCHAR * dlg_FrmlFind;
			const TCHAR * dlg_SaveFrmlList;
			const TCHAR * dlg_ReadFrmlList;
			const TCHAR * dlg_messagebox;
			const TCHAR * dlg_targetSelect;
			const TCHAR * dlg_comboStockFilter;
			const TCHAR * dlg_headerSelect;
			const TCHAR * dlg_rehabFixedTime;
			const TCHAR * dlg_updateLog;
			const TCHAR * dlg_condBackTesting;
			const TCHAR * dlg_multiFilter;
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
				png_treeicon = _T("IMG:png_treeicon");
				btn_help = _T("IMG:btn_help");
			}
			const TCHAR * default_scrollbar_png;
			const TCHAR * skin_lcex_header_arrow;
			const TCHAR * png_treeicon;
			const TCHAR * btn_help;
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
				menu_target = _T("smenu:menu_target");
				menu_rehab = _T("smenu:menu_rehab");
				menu_filterWndNum = _T("smenu:menu_filterWndNum");
			}
			const TCHAR * menu_fenshi;
			const TCHAR * menu_kline;
			const TCHAR * menu_new;
			const TCHAR * menu_target;
			const TCHAR * menu_rehab;
			const TCHAR * menu_filterWndNum;
		}smenu;
	};
	const SNamedID::NAMEDVALUE namedXmlID[]={
		{L"_name_start",65535},
		{L"btnSwitch",65744},
		{L"btn_Add",65642},
		{L"btn_BandCancel",65580},
		{L"btn_BandClose",65572},
		{L"btn_BandDefault",65581},
		{L"btn_BandOK",65579},
		{L"btn_BandSetDefault",65582},
		{L"btn_Cancel",65650},
		{L"btn_Change",65731},
		{L"btn_Day",65598},
		{L"btn_Delete",65643},
		{L"btn_EMACancel",65587},
		{L"btn_EMAClose",65583},
		{L"btn_EMADefault",65588},
		{L"btn_EMAOK",65586},
		{L"btn_EMASetDefault",65589},
		{L"btn_Edit",65623},
		{L"btn_EditFunc",65538},
		{L"btn_FS",65592},
		{L"btn_Find",65621},
		{L"btn_Insert",65717},
		{L"btn_ListConnect1",65599},
		{L"btn_ListConnect2",65600},
		{L"btn_M1",65593},
		{L"btn_M15",65595},
		{L"btn_M30",65596},
		{L"btn_M5",65594},
		{L"btn_M60",65597},
		{L"btn_MACDCancel",65569},
		{L"btn_MACDDefault",65570},
		{L"btn_MACDOK",65568},
		{L"btn_MACDSetDefault",65571},
		{L"btn_MaCancel",65558},
		{L"btn_MaClose",65548},
		{L"btn_MaDefault",65555},
		{L"btn_MaOK",65557},
		{L"btn_MaSetDefault",65556},
		{L"btn_MacdClose",65564},
		{L"btn_Market",65591},
		{L"btn_MultiFilter",65537},
		{L"btn_New",65730},
		{L"btn_OK",65617},
		{L"btn_Read",65644},
		{L"btn_Rehab",65608},
		{L"btn_Save",65645},
		{L"btn_StockFilter",65601},
		{L"btn_Test",65718},
		{L"btn_TitleSel",65602},
		{L"btn_Usage",65622},
		{L"btn_backTesting",65750},
		{L"btn_cancel",65618},
		{L"btn_close",65539},
		{L"btn_delete",65737},
		{L"btn_downFind",65734},
		{L"btn_export",65777},
		{L"btn_help",65543},
		{L"btn_kbElfClose",65545},
		{L"btn_login",65561},
		{L"btn_max",65540},
		{L"btn_min",65542},
		{L"btn_mulitWindow",65536},
		{L"btn_quit",65562},
		{L"btn_restore",65541},
		{L"btn_testStart",65776},
		{L"btn_upFind",65733},
		{L"btn_wndNum",65781},
		{L"button1st",65745},
		{L"button2nd",65746},
		{L"button3rd",65747},
		{L"cbx_ID",65755},
		{L"cbx_condition",65761},
		{L"cbx_frml",65620},
		{L"cbx_func",65753},
		{L"cbx_index1",65757},
		{L"cbx_index2",65763},
		{L"cbx_open",65619},
		{L"cbx_period",65641},
		{L"cbx_period1",65759},
		{L"cbx_period2",65766},
		{L"cbx_type",65716},
		{L"chk_NewStock",65607},
		{L"chk_SBM",65605},
		{L"chk_ST",65604},
		{L"chk_STARM",65606},
		{L"chk_title",65769},
		{L"chk_use",65649},
		{L"dtp_rehab",65770},
		{L"dtp_testEnd",65775},
		{L"dtp_testStart",65774},
		{L"edit_BandParaK",65575},
		{L"edit_BandParaM1",65576},
		{L"edit_BandParaM2",65577},
		{L"edit_BandParaN1",65573},
		{L"edit_BandParaN2",65574},
		{L"edit_BandParaP",65578},
		{L"edit_EMAPara1",65584},
		{L"edit_EMAPara2",65585},
		{L"edit_FindFunc",65728},
		{L"edit_FindTarget",65749},
		{L"edit_MacdPara1",65565},
		{L"edit_MacdPara2",65566},
		{L"edit_MacdPara3",65567},
		{L"edit_dscp",65651},
		{L"edit_explain",65723},
		{L"edit_fileName",65735},
		{L"edit_funcDscp",65727},
		{L"edit_kbElf",65546},
		{L"edit_keyWord",65732},
		{L"edit_maPara1",65549},
		{L"edit_maPara2",65550},
		{L"edit_maPara3",65551},
		{L"edit_maPara4",65552},
		{L"edit_maPara5",65553},
		{L"edit_maPara6",65554},
		{L"edit_name",65616},
		{L"edit_num",65767},
		{L"edit_para1",65625},
		{L"edit_para10",65634},
		{L"edit_para10def",65691},
		{L"edit_para10max",65690},
		{L"edit_para10min",65689},
		{L"edit_para10name",65688},
		{L"edit_para11",65635},
		{L"edit_para11def",65695},
		{L"edit_para11max",65694},
		{L"edit_para11min",65693},
		{L"edit_para11name",65692},
		{L"edit_para12",65636},
		{L"edit_para12def",65699},
		{L"edit_para12max",65698},
		{L"edit_para12min",65697},
		{L"edit_para12name",65696},
		{L"edit_para13",65637},
		{L"edit_para13def",65703},
		{L"edit_para13max",65702},
		{L"edit_para13min",65701},
		{L"edit_para13name",65700},
		{L"edit_para14",65638},
		{L"edit_para14def",65707},
		{L"edit_para14max",65706},
		{L"edit_para14min",65705},
		{L"edit_para14name",65704},
		{L"edit_para15",65639},
		{L"edit_para15def",65711},
		{L"edit_para15max",65710},
		{L"edit_para15min",65709},
		{L"edit_para15name",65708},
		{L"edit_para16",65640},
		{L"edit_para16def",65715},
		{L"edit_para16max",65714},
		{L"edit_para16min",65713},
		{L"edit_para16name",65712},
		{L"edit_para1def",65655},
		{L"edit_para1max",65654},
		{L"edit_para1min",65653},
		{L"edit_para1name",65652},
		{L"edit_para2",65626},
		{L"edit_para2def",65659},
		{L"edit_para2max",65658},
		{L"edit_para2min",65657},
		{L"edit_para2name",65656},
		{L"edit_para3",65627},
		{L"edit_para3def",65663},
		{L"edit_para3max",65662},
		{L"edit_para3min",65661},
		{L"edit_para3name",65660},
		{L"edit_para4",65628},
		{L"edit_para4def",65667},
		{L"edit_para4max",65666},
		{L"edit_para4min",65665},
		{L"edit_para4name",65664},
		{L"edit_para5",65629},
		{L"edit_para5def",65671},
		{L"edit_para5max",65670},
		{L"edit_para5min",65669},
		{L"edit_para5name",65668},
		{L"edit_para6",65630},
		{L"edit_para6def",65675},
		{L"edit_para6max",65674},
		{L"edit_para6min",65673},
		{L"edit_para6name",65672},
		{L"edit_para7",65631},
		{L"edit_para7def",65679},
		{L"edit_para7max",65678},
		{L"edit_para7min",65677},
		{L"edit_para7name",65676},
		{L"edit_para8",65632},
		{L"edit_para8def",65683},
		{L"edit_para8max",65682},
		{L"edit_para8min",65681},
		{L"edit_para8name",65680},
		{L"edit_para9",65633},
		{L"edit_para9def",65687},
		{L"edit_para9max",65686},
		{L"edit_para9min",65685},
		{L"edit_para9name",65684},
		{L"edit_param",65722},
		{L"edit_result",65721},
		{L"edit_trans",65720},
		{L"fenshiPic",65610},
		{L"img_icon_exclamation",65742},
		{L"img_icon_information",65740},
		{L"img_icon_question",65741},
		{L"klinePic",65611},
		{L"lb_class",65725},
		{L"lb_target",65748},
		{L"lc_avgResult",65778},
		{L"lc_cond",65646},
		{L"lc_func",65726},
		{L"lc_planName",65736},
		{L"lc_testResult",65780},
		{L"list_kbElf",65547},
		{L"login_ID",65559},
		{L"login_PassWord",65560},
		{L"ls_filter",65751},
		{L"ls_rps",65609},
		{L"lv_title",65768},
		{L"msgicon",65739},
		{L"msgtext",65743},
		{L"msgtitle",65738},
		{L"radio_BackRehab",65772},
		{L"radio_FrontRehab",65771},
		{L"rdb_and",65647},
		{L"rdb_or",65648},
		{L"re_Log",65773},
		{L"re_frml",65724},
		{L"tab_ctrl",65719},
		{L"tc_frml",65729},
		{L"text_Group",65590},
		{L"text_ID",65754},
		{L"text_ShowIndy",65603},
		{L"text_TestState",65779},
		{L"text_condition",65760},
		{L"text_func",65752},
		{L"text_index1",65756},
		{L"text_index2",65762},
		{L"text_num",65765},
		{L"text_paratitle",65624},
		{L"text_period1",65758},
		{L"text_period2",65764},
		{L"text_windowName",65615},
		{L"txt_login",65563},
		{L"txt_title",65544},
		{L"wnd1",65782},
		{L"wnd2",65783},
		{L"wnd3",65784},
		{L"wnd4",65785},
		{L"wnd5",65786},
		{L"wnd6",65787},
		{L"wnd_SWL1",65612},
		{L"wnd_SWL2",65613},
		{L"wnd_Stock",65614}		};
	class _R{
	public:
		class _name{
		public:
		_name(){
			_name_start = namedXmlID[0].strName;
			btnSwitch = namedXmlID[1].strName;
			btn_Add = namedXmlID[2].strName;
			btn_BandCancel = namedXmlID[3].strName;
			btn_BandClose = namedXmlID[4].strName;
			btn_BandDefault = namedXmlID[5].strName;
			btn_BandOK = namedXmlID[6].strName;
			btn_BandSetDefault = namedXmlID[7].strName;
			btn_Cancel = namedXmlID[8].strName;
			btn_Change = namedXmlID[9].strName;
			btn_Day = namedXmlID[10].strName;
			btn_Delete = namedXmlID[11].strName;
			btn_EMACancel = namedXmlID[12].strName;
			btn_EMAClose = namedXmlID[13].strName;
			btn_EMADefault = namedXmlID[14].strName;
			btn_EMAOK = namedXmlID[15].strName;
			btn_EMASetDefault = namedXmlID[16].strName;
			btn_Edit = namedXmlID[17].strName;
			btn_EditFunc = namedXmlID[18].strName;
			btn_FS = namedXmlID[19].strName;
			btn_Find = namedXmlID[20].strName;
			btn_Insert = namedXmlID[21].strName;
			btn_ListConnect1 = namedXmlID[22].strName;
			btn_ListConnect2 = namedXmlID[23].strName;
			btn_M1 = namedXmlID[24].strName;
			btn_M15 = namedXmlID[25].strName;
			btn_M30 = namedXmlID[26].strName;
			btn_M5 = namedXmlID[27].strName;
			btn_M60 = namedXmlID[28].strName;
			btn_MACDCancel = namedXmlID[29].strName;
			btn_MACDDefault = namedXmlID[30].strName;
			btn_MACDOK = namedXmlID[31].strName;
			btn_MACDSetDefault = namedXmlID[32].strName;
			btn_MaCancel = namedXmlID[33].strName;
			btn_MaClose = namedXmlID[34].strName;
			btn_MaDefault = namedXmlID[35].strName;
			btn_MaOK = namedXmlID[36].strName;
			btn_MaSetDefault = namedXmlID[37].strName;
			btn_MacdClose = namedXmlID[38].strName;
			btn_Market = namedXmlID[39].strName;
			btn_MultiFilter = namedXmlID[40].strName;
			btn_New = namedXmlID[41].strName;
			btn_OK = namedXmlID[42].strName;
			btn_Read = namedXmlID[43].strName;
			btn_Rehab = namedXmlID[44].strName;
			btn_Save = namedXmlID[45].strName;
			btn_StockFilter = namedXmlID[46].strName;
			btn_Test = namedXmlID[47].strName;
			btn_TitleSel = namedXmlID[48].strName;
			btn_Usage = namedXmlID[49].strName;
			btn_backTesting = namedXmlID[50].strName;
			btn_cancel = namedXmlID[51].strName;
			btn_close = namedXmlID[52].strName;
			btn_delete = namedXmlID[53].strName;
			btn_downFind = namedXmlID[54].strName;
			btn_export = namedXmlID[55].strName;
			btn_help = namedXmlID[56].strName;
			btn_kbElfClose = namedXmlID[57].strName;
			btn_login = namedXmlID[58].strName;
			btn_max = namedXmlID[59].strName;
			btn_min = namedXmlID[60].strName;
			btn_mulitWindow = namedXmlID[61].strName;
			btn_quit = namedXmlID[62].strName;
			btn_restore = namedXmlID[63].strName;
			btn_testStart = namedXmlID[64].strName;
			btn_upFind = namedXmlID[65].strName;
			btn_wndNum = namedXmlID[66].strName;
			button1st = namedXmlID[67].strName;
			button2nd = namedXmlID[68].strName;
			button3rd = namedXmlID[69].strName;
			cbx_ID = namedXmlID[70].strName;
			cbx_condition = namedXmlID[71].strName;
			cbx_frml = namedXmlID[72].strName;
			cbx_func = namedXmlID[73].strName;
			cbx_index1 = namedXmlID[74].strName;
			cbx_index2 = namedXmlID[75].strName;
			cbx_open = namedXmlID[76].strName;
			cbx_period = namedXmlID[77].strName;
			cbx_period1 = namedXmlID[78].strName;
			cbx_period2 = namedXmlID[79].strName;
			cbx_type = namedXmlID[80].strName;
			chk_NewStock = namedXmlID[81].strName;
			chk_SBM = namedXmlID[82].strName;
			chk_ST = namedXmlID[83].strName;
			chk_STARM = namedXmlID[84].strName;
			chk_title = namedXmlID[85].strName;
			chk_use = namedXmlID[86].strName;
			dtp_rehab = namedXmlID[87].strName;
			dtp_testEnd = namedXmlID[88].strName;
			dtp_testStart = namedXmlID[89].strName;
			edit_BandParaK = namedXmlID[90].strName;
			edit_BandParaM1 = namedXmlID[91].strName;
			edit_BandParaM2 = namedXmlID[92].strName;
			edit_BandParaN1 = namedXmlID[93].strName;
			edit_BandParaN2 = namedXmlID[94].strName;
			edit_BandParaP = namedXmlID[95].strName;
			edit_EMAPara1 = namedXmlID[96].strName;
			edit_EMAPara2 = namedXmlID[97].strName;
			edit_FindFunc = namedXmlID[98].strName;
			edit_FindTarget = namedXmlID[99].strName;
			edit_MacdPara1 = namedXmlID[100].strName;
			edit_MacdPara2 = namedXmlID[101].strName;
			edit_MacdPara3 = namedXmlID[102].strName;
			edit_dscp = namedXmlID[103].strName;
			edit_explain = namedXmlID[104].strName;
			edit_fileName = namedXmlID[105].strName;
			edit_funcDscp = namedXmlID[106].strName;
			edit_kbElf = namedXmlID[107].strName;
			edit_keyWord = namedXmlID[108].strName;
			edit_maPara1 = namedXmlID[109].strName;
			edit_maPara2 = namedXmlID[110].strName;
			edit_maPara3 = namedXmlID[111].strName;
			edit_maPara4 = namedXmlID[112].strName;
			edit_maPara5 = namedXmlID[113].strName;
			edit_maPara6 = namedXmlID[114].strName;
			edit_name = namedXmlID[115].strName;
			edit_num = namedXmlID[116].strName;
			edit_para1 = namedXmlID[117].strName;
			edit_para10 = namedXmlID[118].strName;
			edit_para10def = namedXmlID[119].strName;
			edit_para10max = namedXmlID[120].strName;
			edit_para10min = namedXmlID[121].strName;
			edit_para10name = namedXmlID[122].strName;
			edit_para11 = namedXmlID[123].strName;
			edit_para11def = namedXmlID[124].strName;
			edit_para11max = namedXmlID[125].strName;
			edit_para11min = namedXmlID[126].strName;
			edit_para11name = namedXmlID[127].strName;
			edit_para12 = namedXmlID[128].strName;
			edit_para12def = namedXmlID[129].strName;
			edit_para12max = namedXmlID[130].strName;
			edit_para12min = namedXmlID[131].strName;
			edit_para12name = namedXmlID[132].strName;
			edit_para13 = namedXmlID[133].strName;
			edit_para13def = namedXmlID[134].strName;
			edit_para13max = namedXmlID[135].strName;
			edit_para13min = namedXmlID[136].strName;
			edit_para13name = namedXmlID[137].strName;
			edit_para14 = namedXmlID[138].strName;
			edit_para14def = namedXmlID[139].strName;
			edit_para14max = namedXmlID[140].strName;
			edit_para14min = namedXmlID[141].strName;
			edit_para14name = namedXmlID[142].strName;
			edit_para15 = namedXmlID[143].strName;
			edit_para15def = namedXmlID[144].strName;
			edit_para15max = namedXmlID[145].strName;
			edit_para15min = namedXmlID[146].strName;
			edit_para15name = namedXmlID[147].strName;
			edit_para16 = namedXmlID[148].strName;
			edit_para16def = namedXmlID[149].strName;
			edit_para16max = namedXmlID[150].strName;
			edit_para16min = namedXmlID[151].strName;
			edit_para16name = namedXmlID[152].strName;
			edit_para1def = namedXmlID[153].strName;
			edit_para1max = namedXmlID[154].strName;
			edit_para1min = namedXmlID[155].strName;
			edit_para1name = namedXmlID[156].strName;
			edit_para2 = namedXmlID[157].strName;
			edit_para2def = namedXmlID[158].strName;
			edit_para2max = namedXmlID[159].strName;
			edit_para2min = namedXmlID[160].strName;
			edit_para2name = namedXmlID[161].strName;
			edit_para3 = namedXmlID[162].strName;
			edit_para3def = namedXmlID[163].strName;
			edit_para3max = namedXmlID[164].strName;
			edit_para3min = namedXmlID[165].strName;
			edit_para3name = namedXmlID[166].strName;
			edit_para4 = namedXmlID[167].strName;
			edit_para4def = namedXmlID[168].strName;
			edit_para4max = namedXmlID[169].strName;
			edit_para4min = namedXmlID[170].strName;
			edit_para4name = namedXmlID[171].strName;
			edit_para5 = namedXmlID[172].strName;
			edit_para5def = namedXmlID[173].strName;
			edit_para5max = namedXmlID[174].strName;
			edit_para5min = namedXmlID[175].strName;
			edit_para5name = namedXmlID[176].strName;
			edit_para6 = namedXmlID[177].strName;
			edit_para6def = namedXmlID[178].strName;
			edit_para6max = namedXmlID[179].strName;
			edit_para6min = namedXmlID[180].strName;
			edit_para6name = namedXmlID[181].strName;
			edit_para7 = namedXmlID[182].strName;
			edit_para7def = namedXmlID[183].strName;
			edit_para7max = namedXmlID[184].strName;
			edit_para7min = namedXmlID[185].strName;
			edit_para7name = namedXmlID[186].strName;
			edit_para8 = namedXmlID[187].strName;
			edit_para8def = namedXmlID[188].strName;
			edit_para8max = namedXmlID[189].strName;
			edit_para8min = namedXmlID[190].strName;
			edit_para8name = namedXmlID[191].strName;
			edit_para9 = namedXmlID[192].strName;
			edit_para9def = namedXmlID[193].strName;
			edit_para9max = namedXmlID[194].strName;
			edit_para9min = namedXmlID[195].strName;
			edit_para9name = namedXmlID[196].strName;
			edit_param = namedXmlID[197].strName;
			edit_result = namedXmlID[198].strName;
			edit_trans = namedXmlID[199].strName;
			fenshiPic = namedXmlID[200].strName;
			img_icon_exclamation = namedXmlID[201].strName;
			img_icon_information = namedXmlID[202].strName;
			img_icon_question = namedXmlID[203].strName;
			klinePic = namedXmlID[204].strName;
			lb_class = namedXmlID[205].strName;
			lb_target = namedXmlID[206].strName;
			lc_avgResult = namedXmlID[207].strName;
			lc_cond = namedXmlID[208].strName;
			lc_func = namedXmlID[209].strName;
			lc_planName = namedXmlID[210].strName;
			lc_testResult = namedXmlID[211].strName;
			list_kbElf = namedXmlID[212].strName;
			login_ID = namedXmlID[213].strName;
			login_PassWord = namedXmlID[214].strName;
			ls_filter = namedXmlID[215].strName;
			ls_rps = namedXmlID[216].strName;
			lv_title = namedXmlID[217].strName;
			msgicon = namedXmlID[218].strName;
			msgtext = namedXmlID[219].strName;
			msgtitle = namedXmlID[220].strName;
			radio_BackRehab = namedXmlID[221].strName;
			radio_FrontRehab = namedXmlID[222].strName;
			rdb_and = namedXmlID[223].strName;
			rdb_or = namedXmlID[224].strName;
			re_Log = namedXmlID[225].strName;
			re_frml = namedXmlID[226].strName;
			tab_ctrl = namedXmlID[227].strName;
			tc_frml = namedXmlID[228].strName;
			text_Group = namedXmlID[229].strName;
			text_ID = namedXmlID[230].strName;
			text_ShowIndy = namedXmlID[231].strName;
			text_TestState = namedXmlID[232].strName;
			text_condition = namedXmlID[233].strName;
			text_func = namedXmlID[234].strName;
			text_index1 = namedXmlID[235].strName;
			text_index2 = namedXmlID[236].strName;
			text_num = namedXmlID[237].strName;
			text_paratitle = namedXmlID[238].strName;
			text_period1 = namedXmlID[239].strName;
			text_period2 = namedXmlID[240].strName;
			text_windowName = namedXmlID[241].strName;
			txt_login = namedXmlID[242].strName;
			txt_title = namedXmlID[243].strName;
			wnd1 = namedXmlID[244].strName;
			wnd2 = namedXmlID[245].strName;
			wnd3 = namedXmlID[246].strName;
			wnd4 = namedXmlID[247].strName;
			wnd5 = namedXmlID[248].strName;
			wnd6 = namedXmlID[249].strName;
			wnd_SWL1 = namedXmlID[250].strName;
			wnd_SWL2 = namedXmlID[251].strName;
			wnd_Stock = namedXmlID[252].strName;
		}
		 const wchar_t * _name_start;
		 const wchar_t * btnSwitch;
		 const wchar_t * btn_Add;
		 const wchar_t * btn_BandCancel;
		 const wchar_t * btn_BandClose;
		 const wchar_t * btn_BandDefault;
		 const wchar_t * btn_BandOK;
		 const wchar_t * btn_BandSetDefault;
		 const wchar_t * btn_Cancel;
		 const wchar_t * btn_Change;
		 const wchar_t * btn_Day;
		 const wchar_t * btn_Delete;
		 const wchar_t * btn_EMACancel;
		 const wchar_t * btn_EMAClose;
		 const wchar_t * btn_EMADefault;
		 const wchar_t * btn_EMAOK;
		 const wchar_t * btn_EMASetDefault;
		 const wchar_t * btn_Edit;
		 const wchar_t * btn_EditFunc;
		 const wchar_t * btn_FS;
		 const wchar_t * btn_Find;
		 const wchar_t * btn_Insert;
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
		 const wchar_t * btn_MultiFilter;
		 const wchar_t * btn_New;
		 const wchar_t * btn_OK;
		 const wchar_t * btn_Read;
		 const wchar_t * btn_Rehab;
		 const wchar_t * btn_Save;
		 const wchar_t * btn_StockFilter;
		 const wchar_t * btn_Test;
		 const wchar_t * btn_TitleSel;
		 const wchar_t * btn_Usage;
		 const wchar_t * btn_backTesting;
		 const wchar_t * btn_cancel;
		 const wchar_t * btn_close;
		 const wchar_t * btn_delete;
		 const wchar_t * btn_downFind;
		 const wchar_t * btn_export;
		 const wchar_t * btn_help;
		 const wchar_t * btn_kbElfClose;
		 const wchar_t * btn_login;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_mulitWindow;
		 const wchar_t * btn_quit;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_testStart;
		 const wchar_t * btn_upFind;
		 const wchar_t * btn_wndNum;
		 const wchar_t * button1st;
		 const wchar_t * button2nd;
		 const wchar_t * button3rd;
		 const wchar_t * cbx_ID;
		 const wchar_t * cbx_condition;
		 const wchar_t * cbx_frml;
		 const wchar_t * cbx_func;
		 const wchar_t * cbx_index1;
		 const wchar_t * cbx_index2;
		 const wchar_t * cbx_open;
		 const wchar_t * cbx_period;
		 const wchar_t * cbx_period1;
		 const wchar_t * cbx_period2;
		 const wchar_t * cbx_type;
		 const wchar_t * chk_NewStock;
		 const wchar_t * chk_SBM;
		 const wchar_t * chk_ST;
		 const wchar_t * chk_STARM;
		 const wchar_t * chk_title;
		 const wchar_t * chk_use;
		 const wchar_t * dtp_rehab;
		 const wchar_t * dtp_testEnd;
		 const wchar_t * dtp_testStart;
		 const wchar_t * edit_BandParaK;
		 const wchar_t * edit_BandParaM1;
		 const wchar_t * edit_BandParaM2;
		 const wchar_t * edit_BandParaN1;
		 const wchar_t * edit_BandParaN2;
		 const wchar_t * edit_BandParaP;
		 const wchar_t * edit_EMAPara1;
		 const wchar_t * edit_EMAPara2;
		 const wchar_t * edit_FindFunc;
		 const wchar_t * edit_FindTarget;
		 const wchar_t * edit_MacdPara1;
		 const wchar_t * edit_MacdPara2;
		 const wchar_t * edit_MacdPara3;
		 const wchar_t * edit_dscp;
		 const wchar_t * edit_explain;
		 const wchar_t * edit_fileName;
		 const wchar_t * edit_funcDscp;
		 const wchar_t * edit_kbElf;
		 const wchar_t * edit_keyWord;
		 const wchar_t * edit_maPara1;
		 const wchar_t * edit_maPara2;
		 const wchar_t * edit_maPara3;
		 const wchar_t * edit_maPara4;
		 const wchar_t * edit_maPara5;
		 const wchar_t * edit_maPara6;
		 const wchar_t * edit_name;
		 const wchar_t * edit_num;
		 const wchar_t * edit_para1;
		 const wchar_t * edit_para10;
		 const wchar_t * edit_para10def;
		 const wchar_t * edit_para10max;
		 const wchar_t * edit_para10min;
		 const wchar_t * edit_para10name;
		 const wchar_t * edit_para11;
		 const wchar_t * edit_para11def;
		 const wchar_t * edit_para11max;
		 const wchar_t * edit_para11min;
		 const wchar_t * edit_para11name;
		 const wchar_t * edit_para12;
		 const wchar_t * edit_para12def;
		 const wchar_t * edit_para12max;
		 const wchar_t * edit_para12min;
		 const wchar_t * edit_para12name;
		 const wchar_t * edit_para13;
		 const wchar_t * edit_para13def;
		 const wchar_t * edit_para13max;
		 const wchar_t * edit_para13min;
		 const wchar_t * edit_para13name;
		 const wchar_t * edit_para14;
		 const wchar_t * edit_para14def;
		 const wchar_t * edit_para14max;
		 const wchar_t * edit_para14min;
		 const wchar_t * edit_para14name;
		 const wchar_t * edit_para15;
		 const wchar_t * edit_para15def;
		 const wchar_t * edit_para15max;
		 const wchar_t * edit_para15min;
		 const wchar_t * edit_para15name;
		 const wchar_t * edit_para16;
		 const wchar_t * edit_para16def;
		 const wchar_t * edit_para16max;
		 const wchar_t * edit_para16min;
		 const wchar_t * edit_para16name;
		 const wchar_t * edit_para1def;
		 const wchar_t * edit_para1max;
		 const wchar_t * edit_para1min;
		 const wchar_t * edit_para1name;
		 const wchar_t * edit_para2;
		 const wchar_t * edit_para2def;
		 const wchar_t * edit_para2max;
		 const wchar_t * edit_para2min;
		 const wchar_t * edit_para2name;
		 const wchar_t * edit_para3;
		 const wchar_t * edit_para3def;
		 const wchar_t * edit_para3max;
		 const wchar_t * edit_para3min;
		 const wchar_t * edit_para3name;
		 const wchar_t * edit_para4;
		 const wchar_t * edit_para4def;
		 const wchar_t * edit_para4max;
		 const wchar_t * edit_para4min;
		 const wchar_t * edit_para4name;
		 const wchar_t * edit_para5;
		 const wchar_t * edit_para5def;
		 const wchar_t * edit_para5max;
		 const wchar_t * edit_para5min;
		 const wchar_t * edit_para5name;
		 const wchar_t * edit_para6;
		 const wchar_t * edit_para6def;
		 const wchar_t * edit_para6max;
		 const wchar_t * edit_para6min;
		 const wchar_t * edit_para6name;
		 const wchar_t * edit_para7;
		 const wchar_t * edit_para7def;
		 const wchar_t * edit_para7max;
		 const wchar_t * edit_para7min;
		 const wchar_t * edit_para7name;
		 const wchar_t * edit_para8;
		 const wchar_t * edit_para8def;
		 const wchar_t * edit_para8max;
		 const wchar_t * edit_para8min;
		 const wchar_t * edit_para8name;
		 const wchar_t * edit_para9;
		 const wchar_t * edit_para9def;
		 const wchar_t * edit_para9max;
		 const wchar_t * edit_para9min;
		 const wchar_t * edit_para9name;
		 const wchar_t * edit_param;
		 const wchar_t * edit_result;
		 const wchar_t * edit_trans;
		 const wchar_t * fenshiPic;
		 const wchar_t * img_icon_exclamation;
		 const wchar_t * img_icon_information;
		 const wchar_t * img_icon_question;
		 const wchar_t * klinePic;
		 const wchar_t * lb_class;
		 const wchar_t * lb_target;
		 const wchar_t * lc_avgResult;
		 const wchar_t * lc_cond;
		 const wchar_t * lc_func;
		 const wchar_t * lc_planName;
		 const wchar_t * lc_testResult;
		 const wchar_t * list_kbElf;
		 const wchar_t * login_ID;
		 const wchar_t * login_PassWord;
		 const wchar_t * ls_filter;
		 const wchar_t * ls_rps;
		 const wchar_t * lv_title;
		 const wchar_t * msgicon;
		 const wchar_t * msgtext;
		 const wchar_t * msgtitle;
		 const wchar_t * radio_BackRehab;
		 const wchar_t * radio_FrontRehab;
		 const wchar_t * rdb_and;
		 const wchar_t * rdb_or;
		 const wchar_t * re_Log;
		 const wchar_t * re_frml;
		 const wchar_t * tab_ctrl;
		 const wchar_t * tc_frml;
		 const wchar_t * text_Group;
		 const wchar_t * text_ID;
		 const wchar_t * text_ShowIndy;
		 const wchar_t * text_TestState;
		 const wchar_t * text_condition;
		 const wchar_t * text_func;
		 const wchar_t * text_index1;
		 const wchar_t * text_index2;
		 const wchar_t * text_num;
		 const wchar_t * text_paratitle;
		 const wchar_t * text_period1;
		 const wchar_t * text_period2;
		 const wchar_t * text_windowName;
		 const wchar_t * txt_login;
		 const wchar_t * txt_title;
		 const wchar_t * wnd1;
		 const wchar_t * wnd2;
		 const wchar_t * wnd3;
		 const wchar_t * wnd4;
		 const wchar_t * wnd5;
		 const wchar_t * wnd6;
		 const wchar_t * wnd_SWL1;
		 const wchar_t * wnd_SWL2;
		 const wchar_t * wnd_Stock;
		}name;

		class _id{
		public:
		const static int _name_start	=	65535;
		const static int btnSwitch	=	65744;
		const static int btn_Add	=	65642;
		const static int btn_BandCancel	=	65580;
		const static int btn_BandClose	=	65572;
		const static int btn_BandDefault	=	65581;
		const static int btn_BandOK	=	65579;
		const static int btn_BandSetDefault	=	65582;
		const static int btn_Cancel	=	65650;
		const static int btn_Change	=	65731;
		const static int btn_Day	=	65598;
		const static int btn_Delete	=	65643;
		const static int btn_EMACancel	=	65587;
		const static int btn_EMAClose	=	65583;
		const static int btn_EMADefault	=	65588;
		const static int btn_EMAOK	=	65586;
		const static int btn_EMASetDefault	=	65589;
		const static int btn_Edit	=	65623;
		const static int btn_EditFunc	=	65538;
		const static int btn_FS	=	65592;
		const static int btn_Find	=	65621;
		const static int btn_Insert	=	65717;
		const static int btn_ListConnect1	=	65599;
		const static int btn_ListConnect2	=	65600;
		const static int btn_M1	=	65593;
		const static int btn_M15	=	65595;
		const static int btn_M30	=	65596;
		const static int btn_M5	=	65594;
		const static int btn_M60	=	65597;
		const static int btn_MACDCancel	=	65569;
		const static int btn_MACDDefault	=	65570;
		const static int btn_MACDOK	=	65568;
		const static int btn_MACDSetDefault	=	65571;
		const static int btn_MaCancel	=	65558;
		const static int btn_MaClose	=	65548;
		const static int btn_MaDefault	=	65555;
		const static int btn_MaOK	=	65557;
		const static int btn_MaSetDefault	=	65556;
		const static int btn_MacdClose	=	65564;
		const static int btn_Market	=	65591;
		const static int btn_MultiFilter	=	65537;
		const static int btn_New	=	65730;
		const static int btn_OK	=	65617;
		const static int btn_Read	=	65644;
		const static int btn_Rehab	=	65608;
		const static int btn_Save	=	65645;
		const static int btn_StockFilter	=	65601;
		const static int btn_Test	=	65718;
		const static int btn_TitleSel	=	65602;
		const static int btn_Usage	=	65622;
		const static int btn_backTesting	=	65750;
		const static int btn_cancel	=	65618;
		const static int btn_close	=	65539;
		const static int btn_delete	=	65737;
		const static int btn_downFind	=	65734;
		const static int btn_export	=	65777;
		const static int btn_help	=	65543;
		const static int btn_kbElfClose	=	65545;
		const static int btn_login	=	65561;
		const static int btn_max	=	65540;
		const static int btn_min	=	65542;
		const static int btn_mulitWindow	=	65536;
		const static int btn_quit	=	65562;
		const static int btn_restore	=	65541;
		const static int btn_testStart	=	65776;
		const static int btn_upFind	=	65733;
		const static int btn_wndNum	=	65781;
		const static int button1st	=	65745;
		const static int button2nd	=	65746;
		const static int button3rd	=	65747;
		const static int cbx_ID	=	65755;
		const static int cbx_condition	=	65761;
		const static int cbx_frml	=	65620;
		const static int cbx_func	=	65753;
		const static int cbx_index1	=	65757;
		const static int cbx_index2	=	65763;
		const static int cbx_open	=	65619;
		const static int cbx_period	=	65641;
		const static int cbx_period1	=	65759;
		const static int cbx_period2	=	65766;
		const static int cbx_type	=	65716;
		const static int chk_NewStock	=	65607;
		const static int chk_SBM	=	65605;
		const static int chk_ST	=	65604;
		const static int chk_STARM	=	65606;
		const static int chk_title	=	65769;
		const static int chk_use	=	65649;
		const static int dtp_rehab	=	65770;
		const static int dtp_testEnd	=	65775;
		const static int dtp_testStart	=	65774;
		const static int edit_BandParaK	=	65575;
		const static int edit_BandParaM1	=	65576;
		const static int edit_BandParaM2	=	65577;
		const static int edit_BandParaN1	=	65573;
		const static int edit_BandParaN2	=	65574;
		const static int edit_BandParaP	=	65578;
		const static int edit_EMAPara1	=	65584;
		const static int edit_EMAPara2	=	65585;
		const static int edit_FindFunc	=	65728;
		const static int edit_FindTarget	=	65749;
		const static int edit_MacdPara1	=	65565;
		const static int edit_MacdPara2	=	65566;
		const static int edit_MacdPara3	=	65567;
		const static int edit_dscp	=	65651;
		const static int edit_explain	=	65723;
		const static int edit_fileName	=	65735;
		const static int edit_funcDscp	=	65727;
		const static int edit_kbElf	=	65546;
		const static int edit_keyWord	=	65732;
		const static int edit_maPara1	=	65549;
		const static int edit_maPara2	=	65550;
		const static int edit_maPara3	=	65551;
		const static int edit_maPara4	=	65552;
		const static int edit_maPara5	=	65553;
		const static int edit_maPara6	=	65554;
		const static int edit_name	=	65616;
		const static int edit_num	=	65767;
		const static int edit_para1	=	65625;
		const static int edit_para10	=	65634;
		const static int edit_para10def	=	65691;
		const static int edit_para10max	=	65690;
		const static int edit_para10min	=	65689;
		const static int edit_para10name	=	65688;
		const static int edit_para11	=	65635;
		const static int edit_para11def	=	65695;
		const static int edit_para11max	=	65694;
		const static int edit_para11min	=	65693;
		const static int edit_para11name	=	65692;
		const static int edit_para12	=	65636;
		const static int edit_para12def	=	65699;
		const static int edit_para12max	=	65698;
		const static int edit_para12min	=	65697;
		const static int edit_para12name	=	65696;
		const static int edit_para13	=	65637;
		const static int edit_para13def	=	65703;
		const static int edit_para13max	=	65702;
		const static int edit_para13min	=	65701;
		const static int edit_para13name	=	65700;
		const static int edit_para14	=	65638;
		const static int edit_para14def	=	65707;
		const static int edit_para14max	=	65706;
		const static int edit_para14min	=	65705;
		const static int edit_para14name	=	65704;
		const static int edit_para15	=	65639;
		const static int edit_para15def	=	65711;
		const static int edit_para15max	=	65710;
		const static int edit_para15min	=	65709;
		const static int edit_para15name	=	65708;
		const static int edit_para16	=	65640;
		const static int edit_para16def	=	65715;
		const static int edit_para16max	=	65714;
		const static int edit_para16min	=	65713;
		const static int edit_para16name	=	65712;
		const static int edit_para1def	=	65655;
		const static int edit_para1max	=	65654;
		const static int edit_para1min	=	65653;
		const static int edit_para1name	=	65652;
		const static int edit_para2	=	65626;
		const static int edit_para2def	=	65659;
		const static int edit_para2max	=	65658;
		const static int edit_para2min	=	65657;
		const static int edit_para2name	=	65656;
		const static int edit_para3	=	65627;
		const static int edit_para3def	=	65663;
		const static int edit_para3max	=	65662;
		const static int edit_para3min	=	65661;
		const static int edit_para3name	=	65660;
		const static int edit_para4	=	65628;
		const static int edit_para4def	=	65667;
		const static int edit_para4max	=	65666;
		const static int edit_para4min	=	65665;
		const static int edit_para4name	=	65664;
		const static int edit_para5	=	65629;
		const static int edit_para5def	=	65671;
		const static int edit_para5max	=	65670;
		const static int edit_para5min	=	65669;
		const static int edit_para5name	=	65668;
		const static int edit_para6	=	65630;
		const static int edit_para6def	=	65675;
		const static int edit_para6max	=	65674;
		const static int edit_para6min	=	65673;
		const static int edit_para6name	=	65672;
		const static int edit_para7	=	65631;
		const static int edit_para7def	=	65679;
		const static int edit_para7max	=	65678;
		const static int edit_para7min	=	65677;
		const static int edit_para7name	=	65676;
		const static int edit_para8	=	65632;
		const static int edit_para8def	=	65683;
		const static int edit_para8max	=	65682;
		const static int edit_para8min	=	65681;
		const static int edit_para8name	=	65680;
		const static int edit_para9	=	65633;
		const static int edit_para9def	=	65687;
		const static int edit_para9max	=	65686;
		const static int edit_para9min	=	65685;
		const static int edit_para9name	=	65684;
		const static int edit_param	=	65722;
		const static int edit_result	=	65721;
		const static int edit_trans	=	65720;
		const static int fenshiPic	=	65610;
		const static int img_icon_exclamation	=	65742;
		const static int img_icon_information	=	65740;
		const static int img_icon_question	=	65741;
		const static int klinePic	=	65611;
		const static int lb_class	=	65725;
		const static int lb_target	=	65748;
		const static int lc_avgResult	=	65778;
		const static int lc_cond	=	65646;
		const static int lc_func	=	65726;
		const static int lc_planName	=	65736;
		const static int lc_testResult	=	65780;
		const static int list_kbElf	=	65547;
		const static int login_ID	=	65559;
		const static int login_PassWord	=	65560;
		const static int ls_filter	=	65751;
		const static int ls_rps	=	65609;
		const static int lv_title	=	65768;
		const static int msgicon	=	65739;
		const static int msgtext	=	65743;
		const static int msgtitle	=	65738;
		const static int radio_BackRehab	=	65772;
		const static int radio_FrontRehab	=	65771;
		const static int rdb_and	=	65647;
		const static int rdb_or	=	65648;
		const static int re_Log	=	65773;
		const static int re_frml	=	65724;
		const static int tab_ctrl	=	65719;
		const static int tc_frml	=	65729;
		const static int text_Group	=	65590;
		const static int text_ID	=	65754;
		const static int text_ShowIndy	=	65603;
		const static int text_TestState	=	65779;
		const static int text_condition	=	65760;
		const static int text_func	=	65752;
		const static int text_index1	=	65756;
		const static int text_index2	=	65762;
		const static int text_num	=	65765;
		const static int text_paratitle	=	65624;
		const static int text_period1	=	65758;
		const static int text_period2	=	65764;
		const static int text_windowName	=	65615;
		const static int txt_login	=	65563;
		const static int txt_title	=	65544;
		const static int wnd1	=	65782;
		const static int wnd2	=	65783;
		const static int wnd3	=	65784;
		const static int wnd4	=	65785;
		const static int wnd5	=	65786;
		const static int wnd6	=	65787;
		const static int wnd_SWL1	=	65612;
		const static int wnd_SWL2	=	65613;
		const static int wnd_Stock	=	65614;
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
