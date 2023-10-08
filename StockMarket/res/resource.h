//stamp:4bde473fc148d40f
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
				dlg_TradeSimulate = _T("LAYOUT:dlg_TradeSimulate");
				dlg_targetSelect = _T("LAYOUT:dlg_targetSelect");
				dlg_comboStockFilter = _T("LAYOUT:dlg_comboStockFilter");
				dlg_headerSelect = _T("LAYOUT:dlg_headerSelect");
				dlg_rehabFixedTime = _T("LAYOUT:dlg_rehabFixedTime");
				dlg_updateLog = _T("LAYOUT:dlg_updateLog");
				dlg_condBackTesting = _T("LAYOUT:dlg_condBackTesting");
				dlg_multiFilter = _T("LAYOUT:dlg_multiFilter");
				dlg_limitUpStat = _T("LAYOUT:dlg_limitUpStat");
				dlg_register = _T("LAYOUT:dlg_register");
				dlg_tradeConfirm = _T("LAYOUT:dlg_tradeConfirm");
				dlg_tradeSetting = _T("LAYOUT:dlg_tradeSetting");
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
			const TCHAR * dlg_TradeSimulate;
			const TCHAR * dlg_targetSelect;
			const TCHAR * dlg_comboStockFilter;
			const TCHAR * dlg_headerSelect;
			const TCHAR * dlg_rehabFixedTime;
			const TCHAR * dlg_updateLog;
			const TCHAR * dlg_condBackTesting;
			const TCHAR * dlg_multiFilter;
			const TCHAR * dlg_limitUpStat;
			const TCHAR * dlg_register;
			const TCHAR * dlg_tradeConfirm;
			const TCHAR * dlg_tradeSetting;
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
				png_vscroll = _T("IMG:png_vscroll");
				png_hscroll = _T("IMG:png_hscroll");
				png_newscroll = _T("IMG:png_newscroll");
				open_menu_png = _T("IMG:open_menu_png");
				download_png = _T("IMG:download_png");
			}
			const TCHAR * default_scrollbar_png;
			const TCHAR * skin_lcex_header_arrow;
			const TCHAR * png_treeicon;
			const TCHAR * btn_help;
			const TCHAR * png_vscroll;
			const TCHAR * png_hscroll;
			const TCHAR * png_newscroll;
			const TCHAR * open_menu_png;
			const TCHAR * download_png;
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
		{L"btnSwitch",65755},
		{L"btn_Add",65653},
		{L"btn_BandCancel",65589},
		{L"btn_BandClose",65581},
		{L"btn_BandDefault",65590},
		{L"btn_BandOK",65588},
		{L"btn_BandSetDefault",65591},
		{L"btn_Cancel",65661},
		{L"btn_Change",65742},
		{L"btn_Day",65608},
		{L"btn_Delete",65654},
		{L"btn_EMACancel",65596},
		{L"btn_EMAClose",65592},
		{L"btn_EMADefault",65597},
		{L"btn_EMAOK",65595},
		{L"btn_EMASetDefault",65598},
		{L"btn_Edit",65634},
		{L"btn_EditFunc",65540},
		{L"btn_FS",65602},
		{L"btn_Find",65632},
		{L"btn_IDList",65562},
		{L"btn_Insert",65728},
		{L"btn_LimitUpStat",65538},
		{L"btn_ListConnect1",65609},
		{L"btn_ListConnect2",65610},
		{L"btn_M1",65603},
		{L"btn_M15",65605},
		{L"btn_M30",65606},
		{L"btn_M5",65604},
		{L"btn_M60",65607},
		{L"btn_MACDCancel",65578},
		{L"btn_MACDDefault",65579},
		{L"btn_MACDOK",65577},
		{L"btn_MACDSetDefault",65580},
		{L"btn_MaCancel",65560},
		{L"btn_MaClose",65550},
		{L"btn_MaDefault",65557},
		{L"btn_MaOK",65559},
		{L"btn_MaSetDefault",65558},
		{L"btn_MacdClose",65573},
		{L"btn_Market",65601},
		{L"btn_MultiFilter",65537},
		{L"btn_New",65741},
		{L"btn_OK",65628},
		{L"btn_OKHis",65887},
		{L"btn_Read",65655},
		{L"btn_Rehab",65618},
		{L"btn_Save",65656},
		{L"btn_StockFilter",65611},
		{L"btn_Test",65729},
		{L"btn_TitleSel",65612},
		{L"btn_TradSimulate",65539},
		{L"btn_Usage",65633},
		{L"btn_backTesting",65844},
		{L"btn_buy",65773},
		{L"btn_buyAll",65767},
		{L"btn_buyCon",65760},
		{L"btn_buyHalf",65768},
		{L"btn_buyQuarter",65770},
		{L"btn_buySetting",65772},
		{L"btn_buyThird",65769},
		{L"btn_buyUnCon",65761},
		{L"btn_cancel",65629},
		{L"btn_cancelAll",65809},
		{L"btn_cancelDownload",65811},
		{L"btn_cancelMultiple",65808},
		{L"btn_changePsd",65571},
		{L"btn_close",65541},
		{L"btn_dealDownload",65817},
		{L"btn_dealIDSearch",65815},
		{L"btn_dealSumDownload",65822},
		{L"btn_dealSumSearch",65820},
		{L"btn_delete",65748},
		{L"btn_downFind",65745},
		{L"btn_export",65897},
		{L"btn_help",65545},
		{L"btn_hisDealDownload",65836},
		{L"btn_hisDealIDSearch",65834},
		{L"btn_hisFilter",65841},
		{L"btn_hisTrustDownload",65829},
		{L"btn_hisTrustIDSearch",65827},
		{L"btn_kbElfClose",65547},
		{L"btn_login",65567},
		{L"btn_market",65568},
		{L"btn_max",65542},
		{L"btn_min",65544},
		{L"btn_mulitWindow",65536},
		{L"btn_posDownload",65801},
		{L"btn_posIDSearch",65799},
		{L"btn_quit",65569},
		{L"btn_refreshAcc",65796},
		{L"btn_register",65570},
		{L"btn_restore",65543},
		{L"btn_rtFilter",65840},
		{L"btn_saveHis",65863},
		{L"btn_sell",65789},
		{L"btn_sellAll",65783},
		{L"btn_sellCon",65776},
		{L"btn_sellHalf",65784},
		{L"btn_sellQuarter",65786},
		{L"btn_sellSetting",65788},
		{L"btn_sellThird",65785},
		{L"btn_sellUnCon",65777},
		{L"btn_setFilterName",65842},
		{L"btn_testStart",65896},
		{L"btn_trustDownload",65806},
		{L"btn_trustIDSearch",65804},
		{L"btn_upFind",65744},
		{L"btn_wndNum",65901},
		{L"button1st",65756},
		{L"button2nd",65757},
		{L"button3rd",65758},
		{L"cbx_ID",65849},
		{L"cbx_IDHis",65869},
		{L"cbx_aftreTrade",65966},
		{L"cbx_buyPrice",65948},
		{L"cbx_buyVol",65950},
		{L"cbx_cagePrice",65960},
		{L"cbx_cancelConfirm",65974},
		{L"cbx_changePage",65980},
		{L"cbx_clickVol",65968},
		{L"cbx_condition",65855},
		{L"cbx_conditionHis",65875},
		{L"cbx_frml",65631},
		{L"cbx_func",65847},
		{L"cbx_funcHis",65867},
		{L"cbx_index1",65851},
		{L"cbx_index1His",65871},
		{L"cbx_index2",65857},
		{L"cbx_index2His",65877},
		{L"cbx_limitPrice",65958},
		{L"cbx_open",65630},
		{L"cbx_period",65652},
		{L"cbx_period1",65853},
		{L"cbx_period1His",65873},
		{L"cbx_period2",65860},
		{L"cbx_period2His",65880},
		{L"cbx_sellPrice",65952},
		{L"cbx_sellVol",65954},
		{L"cbx_showDeal",65972},
		{L"cbx_showTrust",65978},
		{L"cbx_trustConfirm",65976},
		{L"cbx_type",65727},
		{L"chk_NewStock",65617},
		{L"chk_SBM",65615},
		{L"chk_ST",65614},
		{L"chk_STARM",65616},
		{L"chk_autoLogin",65565},
		{L"chk_cancelAll",65813},
		{L"chk_rember",65564},
		{L"chk_title",65889},
		{L"chk_use",65660},
		{L"chk_useHis",65862},
		{L"dtp_hisDealEnd",65832},
		{L"dtp_hisDealStart",65831},
		{L"dtp_hisTrustEnd",65825},
		{L"dtp_hisTrustStart",65824},
		{L"dtp_rehab",65890},
		{L"dtp_testEnd",65895},
		{L"dtp_testStart",65894},
		{L"edit_BandParaK",65584},
		{L"edit_BandParaM1",65585},
		{L"edit_BandParaM2",65586},
		{L"edit_BandParaN1",65582},
		{L"edit_BandParaN2",65583},
		{L"edit_BandParaP",65587},
		{L"edit_EMAPara1",65593},
		{L"edit_EMAPara2",65594},
		{L"edit_FindFunc",65739},
		{L"edit_FindTarget",65839},
		{L"edit_ID",65561},
		{L"edit_MacdPara1",65574},
		{L"edit_MacdPara2",65575},
		{L"edit_MacdPara3",65576},
		{L"edit_Psd",65563},
		{L"edit_PsdConfirm",65944},
		{L"edit_SrcPsd",65940},
		{L"edit_buyAmo",65964},
		{L"edit_buyId",65759},
		{L"edit_buyPrice",65762},
		{L"edit_buyVol",65764},
		{L"edit_buyVolFix",65962},
		{L"edit_countDay",65883},
		{L"edit_dealIDSearch",65814},
		{L"edit_dealSumSearch",65819},
		{L"edit_dscp",65662},
		{L"edit_explain",65734},
		{L"edit_fileName",65746},
		{L"edit_funcDscp",65738},
		{L"edit_hisDealIDSearch",65833},
		{L"edit_hisTrustIDSearch",65826},
		{L"edit_kbElf",65548},
		{L"edit_keyWord",65743},
		{L"edit_maPara1",65551},
		{L"edit_maPara2",65552},
		{L"edit_maPara3",65553},
		{L"edit_maPara4",65554},
		{L"edit_maPara5",65555},
		{L"edit_maPara6",65556},
		{L"edit_maxVol",65956},
		{L"edit_name",65627},
		{L"edit_num",65861},
		{L"edit_numHis",65881},
		{L"edit_para1",65636},
		{L"edit_para10",65645},
		{L"edit_para10def",65702},
		{L"edit_para10max",65701},
		{L"edit_para10min",65700},
		{L"edit_para10name",65699},
		{L"edit_para11",65646},
		{L"edit_para11def",65706},
		{L"edit_para11max",65705},
		{L"edit_para11min",65704},
		{L"edit_para11name",65703},
		{L"edit_para12",65647},
		{L"edit_para12def",65710},
		{L"edit_para12max",65709},
		{L"edit_para12min",65708},
		{L"edit_para12name",65707},
		{L"edit_para13",65648},
		{L"edit_para13def",65714},
		{L"edit_para13max",65713},
		{L"edit_para13min",65712},
		{L"edit_para13name",65711},
		{L"edit_para14",65649},
		{L"edit_para14def",65718},
		{L"edit_para14max",65717},
		{L"edit_para14min",65716},
		{L"edit_para14name",65715},
		{L"edit_para15",65650},
		{L"edit_para15def",65722},
		{L"edit_para15max",65721},
		{L"edit_para15min",65720},
		{L"edit_para15name",65719},
		{L"edit_para16",65651},
		{L"edit_para16def",65726},
		{L"edit_para16max",65725},
		{L"edit_para16min",65724},
		{L"edit_para16name",65723},
		{L"edit_para1def",65666},
		{L"edit_para1max",65665},
		{L"edit_para1min",65664},
		{L"edit_para1name",65663},
		{L"edit_para2",65637},
		{L"edit_para2def",65670},
		{L"edit_para2max",65669},
		{L"edit_para2min",65668},
		{L"edit_para2name",65667},
		{L"edit_para3",65638},
		{L"edit_para3def",65674},
		{L"edit_para3max",65673},
		{L"edit_para3min",65672},
		{L"edit_para3name",65671},
		{L"edit_para4",65639},
		{L"edit_para4def",65678},
		{L"edit_para4max",65677},
		{L"edit_para4min",65676},
		{L"edit_para4name",65675},
		{L"edit_para5",65640},
		{L"edit_para5def",65682},
		{L"edit_para5max",65681},
		{L"edit_para5min",65680},
		{L"edit_para5name",65679},
		{L"edit_para6",65641},
		{L"edit_para6def",65686},
		{L"edit_para6max",65685},
		{L"edit_para6min",65684},
		{L"edit_para6name",65683},
		{L"edit_para7",65642},
		{L"edit_para7def",65690},
		{L"edit_para7max",65689},
		{L"edit_para7min",65688},
		{L"edit_para7name",65687},
		{L"edit_para8",65643},
		{L"edit_para8def",65694},
		{L"edit_para8max",65693},
		{L"edit_para8min",65692},
		{L"edit_para8name",65691},
		{L"edit_para9",65644},
		{L"edit_para9def",65698},
		{L"edit_para9max",65697},
		{L"edit_para9min",65696},
		{L"edit_para9name",65695},
		{L"edit_param",65733},
		{L"edit_posIDSearch",65798},
		{L"edit_result",65732},
		{L"edit_sellId",65775},
		{L"edit_sellPrice",65778},
		{L"edit_sellVol",65780},
		{L"edit_trans",65731},
		{L"edit_trustIDSearch",65803},
		{L"edit_windowTime",65970},
		{L"fenshiPic",65620},
		{L"img_icon_exclamation",65753},
		{L"img_icon_information",65751},
		{L"img_icon_question",65752},
		{L"klinePic",65621},
		{L"lb_IDList",65566},
		{L"lb_buyID",65774},
		{L"lb_class",65736},
		{L"lb_sellID",65790},
		{L"lb_target",65838},
		{L"lc_avgResult",65898},
		{L"lc_cond",65657},
		{L"lc_func",65737},
		{L"lc_planName",65747},
		{L"lc_testResult",65900},
		{L"list_kbElf",65549},
		{L"ls_cancel",65812},
		{L"ls_deal",65818},
		{L"ls_dealSummary",65823},
		{L"ls_filter",65845},
		{L"ls_filterHis",65865},
		{L"ls_hisDeal",65837},
		{L"ls_hisTrust",65830},
		{L"ls_luStock0",65910},
		{L"ls_luStock1",65913},
		{L"ls_luStock2",65916},
		{L"ls_luStock3",65919},
		{L"ls_luStock4",65922},
		{L"ls_luStock5",65925},
		{L"ls_luStock6",65928},
		{L"ls_luStock7",65931},
		{L"ls_luStock8",65934},
		{L"ls_luStock9",65937},
		{L"ls_plate0",65909},
		{L"ls_plate1",65912},
		{L"ls_plate2",65915},
		{L"ls_plate3",65918},
		{L"ls_plate4",65921},
		{L"ls_plate5",65924},
		{L"ls_plate6",65927},
		{L"ls_plate7",65930},
		{L"ls_plate8",65933},
		{L"ls_plate9",65936},
		{L"ls_position",65802},
		{L"ls_rps",65619},
		{L"ls_trust",65807},
		{L"lv_title",65888},
		{L"msgicon",65750},
		{L"msgtext",65754},
		{L"msgtitle",65749},
		{L"radio_BackRehab",65892},
		{L"radio_FrontRehab",65891},
		{L"rdb_and",65658},
		{L"rdb_exist",65885},
		{L"rdb_forall",65886},
		{L"rdb_or",65659},
		{L"re_Log",65893},
		{L"re_frml",65735},
		{L"spin_buy",65765},
		{L"spin_buyPrice",65763},
		{L"spin_sell",65781},
		{L"spin_sellPrice",65779},
		{L"tab_ctrl",65730},
		{L"tab_stockFilter",65843},
		{L"tc_frml",65740},
		{L"text_FilterName",65599},
		{L"text_Group",65600},
		{L"text_ID",65848},
		{L"text_IDHis",65868},
		{L"text_ShowIndy",65613},
		{L"text_TestState",65899},
		{L"text_availaleFunds",65795},
		{L"text_cancelCount",65810},
		{L"text_condition",65854},
		{L"text_conditionHis",65874},
		{L"text_countDayBack",65884},
		{L"text_countDayFront",65882},
		{L"text_dealCount",65816},
		{L"text_dealSumCount",65821},
		{L"text_func",65846},
		{L"text_funcHis",65866},
		{L"text_hisDealCount",65835},
		{L"text_hisInfo",65864},
		{L"text_hisTrustCount",65828},
		{L"text_index1",65850},
		{L"text_index1His",65870},
		{L"text_index2",65856},
		{L"text_index2His",65876},
		{L"text_marketValue",65792},
		{L"text_maxBuy",65766},
		{L"text_maxSell",65782},
		{L"text_num",65859},
		{L"text_numHis",65879},
		{L"text_paratitle",65635},
		{L"text_period1",65852},
		{L"text_period1His",65872},
		{L"text_period2",65858},
		{L"text_period2His",65878},
		{L"text_posCount",65800},
		{L"text_profit",65793},
		{L"text_profitChg",65794},
		{L"text_totalCapital",65791},
		{L"text_trustCount",65805},
		{L"text_windowName",65626},
		{L"tradeInfo",65797},
		{L"txt_IDInfo",65938},
		{L"txt_PsdCfmInfo",65945},
		{L"txt_PsdInfo",65943},
		{L"txt_SrcPsdInfo",65941},
		{L"txt_aftreTrade",65965},
		{L"txt_buyAmo",65963},
		{L"txt_buyPrice",65947},
		{L"txt_buyVol",65949},
		{L"txt_buyVolFix",65961},
		{L"txt_buyVolInBig",65771},
		{L"txt_cagePrice",65959},
		{L"txt_cancelConfirm",65973},
		{L"txt_changePage",65979},
		{L"txt_clickVol",65967},
		{L"txt_day0",65908},
		{L"txt_day1",65911},
		{L"txt_day2",65914},
		{L"txt_day3",65917},
		{L"txt_day4",65920},
		{L"txt_day5",65923},
		{L"txt_day6",65926},
		{L"txt_day7",65929},
		{L"txt_day8",65932},
		{L"txt_day9",65935},
		{L"txt_hisFilterCalc",65622},
		{L"txt_limitPrice",65957},
		{L"txt_login",65572},
		{L"txt_maxVol",65955},
		{L"txt_psd",65942},
		{L"txt_sellPrice",65951},
		{L"txt_sellVol",65953},
		{L"txt_sellVolInBig",65787},
		{L"txt_showDeal",65971},
		{L"txt_srcPsd",65939},
		{L"txt_title",65546},
		{L"txt_trade",65946},
		{L"txt_trustConfirm",65975},
		{L"txt_trustInfo",65977},
		{L"txt_windowTime",65969},
		{L"wnd1",65902},
		{L"wnd2",65903},
		{L"wnd3",65904},
		{L"wnd4",65905},
		{L"wnd5",65906},
		{L"wnd6",65907},
		{L"wnd_SWL1",65623},
		{L"wnd_SWL2",65624},
		{L"wnd_Stock",65625}		};
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
			btn_IDList = namedXmlID[21].strName;
			btn_Insert = namedXmlID[22].strName;
			btn_LimitUpStat = namedXmlID[23].strName;
			btn_ListConnect1 = namedXmlID[24].strName;
			btn_ListConnect2 = namedXmlID[25].strName;
			btn_M1 = namedXmlID[26].strName;
			btn_M15 = namedXmlID[27].strName;
			btn_M30 = namedXmlID[28].strName;
			btn_M5 = namedXmlID[29].strName;
			btn_M60 = namedXmlID[30].strName;
			btn_MACDCancel = namedXmlID[31].strName;
			btn_MACDDefault = namedXmlID[32].strName;
			btn_MACDOK = namedXmlID[33].strName;
			btn_MACDSetDefault = namedXmlID[34].strName;
			btn_MaCancel = namedXmlID[35].strName;
			btn_MaClose = namedXmlID[36].strName;
			btn_MaDefault = namedXmlID[37].strName;
			btn_MaOK = namedXmlID[38].strName;
			btn_MaSetDefault = namedXmlID[39].strName;
			btn_MacdClose = namedXmlID[40].strName;
			btn_Market = namedXmlID[41].strName;
			btn_MultiFilter = namedXmlID[42].strName;
			btn_New = namedXmlID[43].strName;
			btn_OK = namedXmlID[44].strName;
			btn_OKHis = namedXmlID[45].strName;
			btn_Read = namedXmlID[46].strName;
			btn_Rehab = namedXmlID[47].strName;
			btn_Save = namedXmlID[48].strName;
			btn_StockFilter = namedXmlID[49].strName;
			btn_Test = namedXmlID[50].strName;
			btn_TitleSel = namedXmlID[51].strName;
			btn_TradSimulate = namedXmlID[52].strName;
			btn_Usage = namedXmlID[53].strName;
			btn_backTesting = namedXmlID[54].strName;
			btn_buy = namedXmlID[55].strName;
			btn_buyAll = namedXmlID[56].strName;
			btn_buyCon = namedXmlID[57].strName;
			btn_buyHalf = namedXmlID[58].strName;
			btn_buyQuarter = namedXmlID[59].strName;
			btn_buySetting = namedXmlID[60].strName;
			btn_buyThird = namedXmlID[61].strName;
			btn_buyUnCon = namedXmlID[62].strName;
			btn_cancel = namedXmlID[63].strName;
			btn_cancelAll = namedXmlID[64].strName;
			btn_cancelDownload = namedXmlID[65].strName;
			btn_cancelMultiple = namedXmlID[66].strName;
			btn_changePsd = namedXmlID[67].strName;
			btn_close = namedXmlID[68].strName;
			btn_dealDownload = namedXmlID[69].strName;
			btn_dealIDSearch = namedXmlID[70].strName;
			btn_dealSumDownload = namedXmlID[71].strName;
			btn_dealSumSearch = namedXmlID[72].strName;
			btn_delete = namedXmlID[73].strName;
			btn_downFind = namedXmlID[74].strName;
			btn_export = namedXmlID[75].strName;
			btn_help = namedXmlID[76].strName;
			btn_hisDealDownload = namedXmlID[77].strName;
			btn_hisDealIDSearch = namedXmlID[78].strName;
			btn_hisFilter = namedXmlID[79].strName;
			btn_hisTrustDownload = namedXmlID[80].strName;
			btn_hisTrustIDSearch = namedXmlID[81].strName;
			btn_kbElfClose = namedXmlID[82].strName;
			btn_login = namedXmlID[83].strName;
			btn_market = namedXmlID[84].strName;
			btn_max = namedXmlID[85].strName;
			btn_min = namedXmlID[86].strName;
			btn_mulitWindow = namedXmlID[87].strName;
			btn_posDownload = namedXmlID[88].strName;
			btn_posIDSearch = namedXmlID[89].strName;
			btn_quit = namedXmlID[90].strName;
			btn_refreshAcc = namedXmlID[91].strName;
			btn_register = namedXmlID[92].strName;
			btn_restore = namedXmlID[93].strName;
			btn_rtFilter = namedXmlID[94].strName;
			btn_saveHis = namedXmlID[95].strName;
			btn_sell = namedXmlID[96].strName;
			btn_sellAll = namedXmlID[97].strName;
			btn_sellCon = namedXmlID[98].strName;
			btn_sellHalf = namedXmlID[99].strName;
			btn_sellQuarter = namedXmlID[100].strName;
			btn_sellSetting = namedXmlID[101].strName;
			btn_sellThird = namedXmlID[102].strName;
			btn_sellUnCon = namedXmlID[103].strName;
			btn_setFilterName = namedXmlID[104].strName;
			btn_testStart = namedXmlID[105].strName;
			btn_trustDownload = namedXmlID[106].strName;
			btn_trustIDSearch = namedXmlID[107].strName;
			btn_upFind = namedXmlID[108].strName;
			btn_wndNum = namedXmlID[109].strName;
			button1st = namedXmlID[110].strName;
			button2nd = namedXmlID[111].strName;
			button3rd = namedXmlID[112].strName;
			cbx_ID = namedXmlID[113].strName;
			cbx_IDHis = namedXmlID[114].strName;
			cbx_aftreTrade = namedXmlID[115].strName;
			cbx_buyPrice = namedXmlID[116].strName;
			cbx_buyVol = namedXmlID[117].strName;
			cbx_cagePrice = namedXmlID[118].strName;
			cbx_cancelConfirm = namedXmlID[119].strName;
			cbx_changePage = namedXmlID[120].strName;
			cbx_clickVol = namedXmlID[121].strName;
			cbx_condition = namedXmlID[122].strName;
			cbx_conditionHis = namedXmlID[123].strName;
			cbx_frml = namedXmlID[124].strName;
			cbx_func = namedXmlID[125].strName;
			cbx_funcHis = namedXmlID[126].strName;
			cbx_index1 = namedXmlID[127].strName;
			cbx_index1His = namedXmlID[128].strName;
			cbx_index2 = namedXmlID[129].strName;
			cbx_index2His = namedXmlID[130].strName;
			cbx_limitPrice = namedXmlID[131].strName;
			cbx_open = namedXmlID[132].strName;
			cbx_period = namedXmlID[133].strName;
			cbx_period1 = namedXmlID[134].strName;
			cbx_period1His = namedXmlID[135].strName;
			cbx_period2 = namedXmlID[136].strName;
			cbx_period2His = namedXmlID[137].strName;
			cbx_sellPrice = namedXmlID[138].strName;
			cbx_sellVol = namedXmlID[139].strName;
			cbx_showDeal = namedXmlID[140].strName;
			cbx_showTrust = namedXmlID[141].strName;
			cbx_trustConfirm = namedXmlID[142].strName;
			cbx_type = namedXmlID[143].strName;
			chk_NewStock = namedXmlID[144].strName;
			chk_SBM = namedXmlID[145].strName;
			chk_ST = namedXmlID[146].strName;
			chk_STARM = namedXmlID[147].strName;
			chk_autoLogin = namedXmlID[148].strName;
			chk_cancelAll = namedXmlID[149].strName;
			chk_rember = namedXmlID[150].strName;
			chk_title = namedXmlID[151].strName;
			chk_use = namedXmlID[152].strName;
			chk_useHis = namedXmlID[153].strName;
			dtp_hisDealEnd = namedXmlID[154].strName;
			dtp_hisDealStart = namedXmlID[155].strName;
			dtp_hisTrustEnd = namedXmlID[156].strName;
			dtp_hisTrustStart = namedXmlID[157].strName;
			dtp_rehab = namedXmlID[158].strName;
			dtp_testEnd = namedXmlID[159].strName;
			dtp_testStart = namedXmlID[160].strName;
			edit_BandParaK = namedXmlID[161].strName;
			edit_BandParaM1 = namedXmlID[162].strName;
			edit_BandParaM2 = namedXmlID[163].strName;
			edit_BandParaN1 = namedXmlID[164].strName;
			edit_BandParaN2 = namedXmlID[165].strName;
			edit_BandParaP = namedXmlID[166].strName;
			edit_EMAPara1 = namedXmlID[167].strName;
			edit_EMAPara2 = namedXmlID[168].strName;
			edit_FindFunc = namedXmlID[169].strName;
			edit_FindTarget = namedXmlID[170].strName;
			edit_ID = namedXmlID[171].strName;
			edit_MacdPara1 = namedXmlID[172].strName;
			edit_MacdPara2 = namedXmlID[173].strName;
			edit_MacdPara3 = namedXmlID[174].strName;
			edit_Psd = namedXmlID[175].strName;
			edit_PsdConfirm = namedXmlID[176].strName;
			edit_SrcPsd = namedXmlID[177].strName;
			edit_buyAmo = namedXmlID[178].strName;
			edit_buyId = namedXmlID[179].strName;
			edit_buyPrice = namedXmlID[180].strName;
			edit_buyVol = namedXmlID[181].strName;
			edit_buyVolFix = namedXmlID[182].strName;
			edit_countDay = namedXmlID[183].strName;
			edit_dealIDSearch = namedXmlID[184].strName;
			edit_dealSumSearch = namedXmlID[185].strName;
			edit_dscp = namedXmlID[186].strName;
			edit_explain = namedXmlID[187].strName;
			edit_fileName = namedXmlID[188].strName;
			edit_funcDscp = namedXmlID[189].strName;
			edit_hisDealIDSearch = namedXmlID[190].strName;
			edit_hisTrustIDSearch = namedXmlID[191].strName;
			edit_kbElf = namedXmlID[192].strName;
			edit_keyWord = namedXmlID[193].strName;
			edit_maPara1 = namedXmlID[194].strName;
			edit_maPara2 = namedXmlID[195].strName;
			edit_maPara3 = namedXmlID[196].strName;
			edit_maPara4 = namedXmlID[197].strName;
			edit_maPara5 = namedXmlID[198].strName;
			edit_maPara6 = namedXmlID[199].strName;
			edit_maxVol = namedXmlID[200].strName;
			edit_name = namedXmlID[201].strName;
			edit_num = namedXmlID[202].strName;
			edit_numHis = namedXmlID[203].strName;
			edit_para1 = namedXmlID[204].strName;
			edit_para10 = namedXmlID[205].strName;
			edit_para10def = namedXmlID[206].strName;
			edit_para10max = namedXmlID[207].strName;
			edit_para10min = namedXmlID[208].strName;
			edit_para10name = namedXmlID[209].strName;
			edit_para11 = namedXmlID[210].strName;
			edit_para11def = namedXmlID[211].strName;
			edit_para11max = namedXmlID[212].strName;
			edit_para11min = namedXmlID[213].strName;
			edit_para11name = namedXmlID[214].strName;
			edit_para12 = namedXmlID[215].strName;
			edit_para12def = namedXmlID[216].strName;
			edit_para12max = namedXmlID[217].strName;
			edit_para12min = namedXmlID[218].strName;
			edit_para12name = namedXmlID[219].strName;
			edit_para13 = namedXmlID[220].strName;
			edit_para13def = namedXmlID[221].strName;
			edit_para13max = namedXmlID[222].strName;
			edit_para13min = namedXmlID[223].strName;
			edit_para13name = namedXmlID[224].strName;
			edit_para14 = namedXmlID[225].strName;
			edit_para14def = namedXmlID[226].strName;
			edit_para14max = namedXmlID[227].strName;
			edit_para14min = namedXmlID[228].strName;
			edit_para14name = namedXmlID[229].strName;
			edit_para15 = namedXmlID[230].strName;
			edit_para15def = namedXmlID[231].strName;
			edit_para15max = namedXmlID[232].strName;
			edit_para15min = namedXmlID[233].strName;
			edit_para15name = namedXmlID[234].strName;
			edit_para16 = namedXmlID[235].strName;
			edit_para16def = namedXmlID[236].strName;
			edit_para16max = namedXmlID[237].strName;
			edit_para16min = namedXmlID[238].strName;
			edit_para16name = namedXmlID[239].strName;
			edit_para1def = namedXmlID[240].strName;
			edit_para1max = namedXmlID[241].strName;
			edit_para1min = namedXmlID[242].strName;
			edit_para1name = namedXmlID[243].strName;
			edit_para2 = namedXmlID[244].strName;
			edit_para2def = namedXmlID[245].strName;
			edit_para2max = namedXmlID[246].strName;
			edit_para2min = namedXmlID[247].strName;
			edit_para2name = namedXmlID[248].strName;
			edit_para3 = namedXmlID[249].strName;
			edit_para3def = namedXmlID[250].strName;
			edit_para3max = namedXmlID[251].strName;
			edit_para3min = namedXmlID[252].strName;
			edit_para3name = namedXmlID[253].strName;
			edit_para4 = namedXmlID[254].strName;
			edit_para4def = namedXmlID[255].strName;
			edit_para4max = namedXmlID[256].strName;
			edit_para4min = namedXmlID[257].strName;
			edit_para4name = namedXmlID[258].strName;
			edit_para5 = namedXmlID[259].strName;
			edit_para5def = namedXmlID[260].strName;
			edit_para5max = namedXmlID[261].strName;
			edit_para5min = namedXmlID[262].strName;
			edit_para5name = namedXmlID[263].strName;
			edit_para6 = namedXmlID[264].strName;
			edit_para6def = namedXmlID[265].strName;
			edit_para6max = namedXmlID[266].strName;
			edit_para6min = namedXmlID[267].strName;
			edit_para6name = namedXmlID[268].strName;
			edit_para7 = namedXmlID[269].strName;
			edit_para7def = namedXmlID[270].strName;
			edit_para7max = namedXmlID[271].strName;
			edit_para7min = namedXmlID[272].strName;
			edit_para7name = namedXmlID[273].strName;
			edit_para8 = namedXmlID[274].strName;
			edit_para8def = namedXmlID[275].strName;
			edit_para8max = namedXmlID[276].strName;
			edit_para8min = namedXmlID[277].strName;
			edit_para8name = namedXmlID[278].strName;
			edit_para9 = namedXmlID[279].strName;
			edit_para9def = namedXmlID[280].strName;
			edit_para9max = namedXmlID[281].strName;
			edit_para9min = namedXmlID[282].strName;
			edit_para9name = namedXmlID[283].strName;
			edit_param = namedXmlID[284].strName;
			edit_posIDSearch = namedXmlID[285].strName;
			edit_result = namedXmlID[286].strName;
			edit_sellId = namedXmlID[287].strName;
			edit_sellPrice = namedXmlID[288].strName;
			edit_sellVol = namedXmlID[289].strName;
			edit_trans = namedXmlID[290].strName;
			edit_trustIDSearch = namedXmlID[291].strName;
			edit_windowTime = namedXmlID[292].strName;
			fenshiPic = namedXmlID[293].strName;
			img_icon_exclamation = namedXmlID[294].strName;
			img_icon_information = namedXmlID[295].strName;
			img_icon_question = namedXmlID[296].strName;
			klinePic = namedXmlID[297].strName;
			lb_IDList = namedXmlID[298].strName;
			lb_buyID = namedXmlID[299].strName;
			lb_class = namedXmlID[300].strName;
			lb_sellID = namedXmlID[301].strName;
			lb_target = namedXmlID[302].strName;
			lc_avgResult = namedXmlID[303].strName;
			lc_cond = namedXmlID[304].strName;
			lc_func = namedXmlID[305].strName;
			lc_planName = namedXmlID[306].strName;
			lc_testResult = namedXmlID[307].strName;
			list_kbElf = namedXmlID[308].strName;
			ls_cancel = namedXmlID[309].strName;
			ls_deal = namedXmlID[310].strName;
			ls_dealSummary = namedXmlID[311].strName;
			ls_filter = namedXmlID[312].strName;
			ls_filterHis = namedXmlID[313].strName;
			ls_hisDeal = namedXmlID[314].strName;
			ls_hisTrust = namedXmlID[315].strName;
			ls_luStock0 = namedXmlID[316].strName;
			ls_luStock1 = namedXmlID[317].strName;
			ls_luStock2 = namedXmlID[318].strName;
			ls_luStock3 = namedXmlID[319].strName;
			ls_luStock4 = namedXmlID[320].strName;
			ls_luStock5 = namedXmlID[321].strName;
			ls_luStock6 = namedXmlID[322].strName;
			ls_luStock7 = namedXmlID[323].strName;
			ls_luStock8 = namedXmlID[324].strName;
			ls_luStock9 = namedXmlID[325].strName;
			ls_plate0 = namedXmlID[326].strName;
			ls_plate1 = namedXmlID[327].strName;
			ls_plate2 = namedXmlID[328].strName;
			ls_plate3 = namedXmlID[329].strName;
			ls_plate4 = namedXmlID[330].strName;
			ls_plate5 = namedXmlID[331].strName;
			ls_plate6 = namedXmlID[332].strName;
			ls_plate7 = namedXmlID[333].strName;
			ls_plate8 = namedXmlID[334].strName;
			ls_plate9 = namedXmlID[335].strName;
			ls_position = namedXmlID[336].strName;
			ls_rps = namedXmlID[337].strName;
			ls_trust = namedXmlID[338].strName;
			lv_title = namedXmlID[339].strName;
			msgicon = namedXmlID[340].strName;
			msgtext = namedXmlID[341].strName;
			msgtitle = namedXmlID[342].strName;
			radio_BackRehab = namedXmlID[343].strName;
			radio_FrontRehab = namedXmlID[344].strName;
			rdb_and = namedXmlID[345].strName;
			rdb_exist = namedXmlID[346].strName;
			rdb_forall = namedXmlID[347].strName;
			rdb_or = namedXmlID[348].strName;
			re_Log = namedXmlID[349].strName;
			re_frml = namedXmlID[350].strName;
			spin_buy = namedXmlID[351].strName;
			spin_buyPrice = namedXmlID[352].strName;
			spin_sell = namedXmlID[353].strName;
			spin_sellPrice = namedXmlID[354].strName;
			tab_ctrl = namedXmlID[355].strName;
			tab_stockFilter = namedXmlID[356].strName;
			tc_frml = namedXmlID[357].strName;
			text_FilterName = namedXmlID[358].strName;
			text_Group = namedXmlID[359].strName;
			text_ID = namedXmlID[360].strName;
			text_IDHis = namedXmlID[361].strName;
			text_ShowIndy = namedXmlID[362].strName;
			text_TestState = namedXmlID[363].strName;
			text_availaleFunds = namedXmlID[364].strName;
			text_cancelCount = namedXmlID[365].strName;
			text_condition = namedXmlID[366].strName;
			text_conditionHis = namedXmlID[367].strName;
			text_countDayBack = namedXmlID[368].strName;
			text_countDayFront = namedXmlID[369].strName;
			text_dealCount = namedXmlID[370].strName;
			text_dealSumCount = namedXmlID[371].strName;
			text_func = namedXmlID[372].strName;
			text_funcHis = namedXmlID[373].strName;
			text_hisDealCount = namedXmlID[374].strName;
			text_hisInfo = namedXmlID[375].strName;
			text_hisTrustCount = namedXmlID[376].strName;
			text_index1 = namedXmlID[377].strName;
			text_index1His = namedXmlID[378].strName;
			text_index2 = namedXmlID[379].strName;
			text_index2His = namedXmlID[380].strName;
			text_marketValue = namedXmlID[381].strName;
			text_maxBuy = namedXmlID[382].strName;
			text_maxSell = namedXmlID[383].strName;
			text_num = namedXmlID[384].strName;
			text_numHis = namedXmlID[385].strName;
			text_paratitle = namedXmlID[386].strName;
			text_period1 = namedXmlID[387].strName;
			text_period1His = namedXmlID[388].strName;
			text_period2 = namedXmlID[389].strName;
			text_period2His = namedXmlID[390].strName;
			text_posCount = namedXmlID[391].strName;
			text_profit = namedXmlID[392].strName;
			text_profitChg = namedXmlID[393].strName;
			text_totalCapital = namedXmlID[394].strName;
			text_trustCount = namedXmlID[395].strName;
			text_windowName = namedXmlID[396].strName;
			tradeInfo = namedXmlID[397].strName;
			txt_IDInfo = namedXmlID[398].strName;
			txt_PsdCfmInfo = namedXmlID[399].strName;
			txt_PsdInfo = namedXmlID[400].strName;
			txt_SrcPsdInfo = namedXmlID[401].strName;
			txt_aftreTrade = namedXmlID[402].strName;
			txt_buyAmo = namedXmlID[403].strName;
			txt_buyPrice = namedXmlID[404].strName;
			txt_buyVol = namedXmlID[405].strName;
			txt_buyVolFix = namedXmlID[406].strName;
			txt_buyVolInBig = namedXmlID[407].strName;
			txt_cagePrice = namedXmlID[408].strName;
			txt_cancelConfirm = namedXmlID[409].strName;
			txt_changePage = namedXmlID[410].strName;
			txt_clickVol = namedXmlID[411].strName;
			txt_day0 = namedXmlID[412].strName;
			txt_day1 = namedXmlID[413].strName;
			txt_day2 = namedXmlID[414].strName;
			txt_day3 = namedXmlID[415].strName;
			txt_day4 = namedXmlID[416].strName;
			txt_day5 = namedXmlID[417].strName;
			txt_day6 = namedXmlID[418].strName;
			txt_day7 = namedXmlID[419].strName;
			txt_day8 = namedXmlID[420].strName;
			txt_day9 = namedXmlID[421].strName;
			txt_hisFilterCalc = namedXmlID[422].strName;
			txt_limitPrice = namedXmlID[423].strName;
			txt_login = namedXmlID[424].strName;
			txt_maxVol = namedXmlID[425].strName;
			txt_psd = namedXmlID[426].strName;
			txt_sellPrice = namedXmlID[427].strName;
			txt_sellVol = namedXmlID[428].strName;
			txt_sellVolInBig = namedXmlID[429].strName;
			txt_showDeal = namedXmlID[430].strName;
			txt_srcPsd = namedXmlID[431].strName;
			txt_title = namedXmlID[432].strName;
			txt_trade = namedXmlID[433].strName;
			txt_trustConfirm = namedXmlID[434].strName;
			txt_trustInfo = namedXmlID[435].strName;
			txt_windowTime = namedXmlID[436].strName;
			wnd1 = namedXmlID[437].strName;
			wnd2 = namedXmlID[438].strName;
			wnd3 = namedXmlID[439].strName;
			wnd4 = namedXmlID[440].strName;
			wnd5 = namedXmlID[441].strName;
			wnd6 = namedXmlID[442].strName;
			wnd_SWL1 = namedXmlID[443].strName;
			wnd_SWL2 = namedXmlID[444].strName;
			wnd_Stock = namedXmlID[445].strName;
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
		 const wchar_t * btn_IDList;
		 const wchar_t * btn_Insert;
		 const wchar_t * btn_LimitUpStat;
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
		 const wchar_t * btn_OKHis;
		 const wchar_t * btn_Read;
		 const wchar_t * btn_Rehab;
		 const wchar_t * btn_Save;
		 const wchar_t * btn_StockFilter;
		 const wchar_t * btn_Test;
		 const wchar_t * btn_TitleSel;
		 const wchar_t * btn_TradSimulate;
		 const wchar_t * btn_Usage;
		 const wchar_t * btn_backTesting;
		 const wchar_t * btn_buy;
		 const wchar_t * btn_buyAll;
		 const wchar_t * btn_buyCon;
		 const wchar_t * btn_buyHalf;
		 const wchar_t * btn_buyQuarter;
		 const wchar_t * btn_buySetting;
		 const wchar_t * btn_buyThird;
		 const wchar_t * btn_buyUnCon;
		 const wchar_t * btn_cancel;
		 const wchar_t * btn_cancelAll;
		 const wchar_t * btn_cancelDownload;
		 const wchar_t * btn_cancelMultiple;
		 const wchar_t * btn_changePsd;
		 const wchar_t * btn_close;
		 const wchar_t * btn_dealDownload;
		 const wchar_t * btn_dealIDSearch;
		 const wchar_t * btn_dealSumDownload;
		 const wchar_t * btn_dealSumSearch;
		 const wchar_t * btn_delete;
		 const wchar_t * btn_downFind;
		 const wchar_t * btn_export;
		 const wchar_t * btn_help;
		 const wchar_t * btn_hisDealDownload;
		 const wchar_t * btn_hisDealIDSearch;
		 const wchar_t * btn_hisFilter;
		 const wchar_t * btn_hisTrustDownload;
		 const wchar_t * btn_hisTrustIDSearch;
		 const wchar_t * btn_kbElfClose;
		 const wchar_t * btn_login;
		 const wchar_t * btn_market;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_mulitWindow;
		 const wchar_t * btn_posDownload;
		 const wchar_t * btn_posIDSearch;
		 const wchar_t * btn_quit;
		 const wchar_t * btn_refreshAcc;
		 const wchar_t * btn_register;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_rtFilter;
		 const wchar_t * btn_saveHis;
		 const wchar_t * btn_sell;
		 const wchar_t * btn_sellAll;
		 const wchar_t * btn_sellCon;
		 const wchar_t * btn_sellHalf;
		 const wchar_t * btn_sellQuarter;
		 const wchar_t * btn_sellSetting;
		 const wchar_t * btn_sellThird;
		 const wchar_t * btn_sellUnCon;
		 const wchar_t * btn_setFilterName;
		 const wchar_t * btn_testStart;
		 const wchar_t * btn_trustDownload;
		 const wchar_t * btn_trustIDSearch;
		 const wchar_t * btn_upFind;
		 const wchar_t * btn_wndNum;
		 const wchar_t * button1st;
		 const wchar_t * button2nd;
		 const wchar_t * button3rd;
		 const wchar_t * cbx_ID;
		 const wchar_t * cbx_IDHis;
		 const wchar_t * cbx_aftreTrade;
		 const wchar_t * cbx_buyPrice;
		 const wchar_t * cbx_buyVol;
		 const wchar_t * cbx_cagePrice;
		 const wchar_t * cbx_cancelConfirm;
		 const wchar_t * cbx_changePage;
		 const wchar_t * cbx_clickVol;
		 const wchar_t * cbx_condition;
		 const wchar_t * cbx_conditionHis;
		 const wchar_t * cbx_frml;
		 const wchar_t * cbx_func;
		 const wchar_t * cbx_funcHis;
		 const wchar_t * cbx_index1;
		 const wchar_t * cbx_index1His;
		 const wchar_t * cbx_index2;
		 const wchar_t * cbx_index2His;
		 const wchar_t * cbx_limitPrice;
		 const wchar_t * cbx_open;
		 const wchar_t * cbx_period;
		 const wchar_t * cbx_period1;
		 const wchar_t * cbx_period1His;
		 const wchar_t * cbx_period2;
		 const wchar_t * cbx_period2His;
		 const wchar_t * cbx_sellPrice;
		 const wchar_t * cbx_sellVol;
		 const wchar_t * cbx_showDeal;
		 const wchar_t * cbx_showTrust;
		 const wchar_t * cbx_trustConfirm;
		 const wchar_t * cbx_type;
		 const wchar_t * chk_NewStock;
		 const wchar_t * chk_SBM;
		 const wchar_t * chk_ST;
		 const wchar_t * chk_STARM;
		 const wchar_t * chk_autoLogin;
		 const wchar_t * chk_cancelAll;
		 const wchar_t * chk_rember;
		 const wchar_t * chk_title;
		 const wchar_t * chk_use;
		 const wchar_t * chk_useHis;
		 const wchar_t * dtp_hisDealEnd;
		 const wchar_t * dtp_hisDealStart;
		 const wchar_t * dtp_hisTrustEnd;
		 const wchar_t * dtp_hisTrustStart;
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
		 const wchar_t * edit_ID;
		 const wchar_t * edit_MacdPara1;
		 const wchar_t * edit_MacdPara2;
		 const wchar_t * edit_MacdPara3;
		 const wchar_t * edit_Psd;
		 const wchar_t * edit_PsdConfirm;
		 const wchar_t * edit_SrcPsd;
		 const wchar_t * edit_buyAmo;
		 const wchar_t * edit_buyId;
		 const wchar_t * edit_buyPrice;
		 const wchar_t * edit_buyVol;
		 const wchar_t * edit_buyVolFix;
		 const wchar_t * edit_countDay;
		 const wchar_t * edit_dealIDSearch;
		 const wchar_t * edit_dealSumSearch;
		 const wchar_t * edit_dscp;
		 const wchar_t * edit_explain;
		 const wchar_t * edit_fileName;
		 const wchar_t * edit_funcDscp;
		 const wchar_t * edit_hisDealIDSearch;
		 const wchar_t * edit_hisTrustIDSearch;
		 const wchar_t * edit_kbElf;
		 const wchar_t * edit_keyWord;
		 const wchar_t * edit_maPara1;
		 const wchar_t * edit_maPara2;
		 const wchar_t * edit_maPara3;
		 const wchar_t * edit_maPara4;
		 const wchar_t * edit_maPara5;
		 const wchar_t * edit_maPara6;
		 const wchar_t * edit_maxVol;
		 const wchar_t * edit_name;
		 const wchar_t * edit_num;
		 const wchar_t * edit_numHis;
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
		 const wchar_t * edit_posIDSearch;
		 const wchar_t * edit_result;
		 const wchar_t * edit_sellId;
		 const wchar_t * edit_sellPrice;
		 const wchar_t * edit_sellVol;
		 const wchar_t * edit_trans;
		 const wchar_t * edit_trustIDSearch;
		 const wchar_t * edit_windowTime;
		 const wchar_t * fenshiPic;
		 const wchar_t * img_icon_exclamation;
		 const wchar_t * img_icon_information;
		 const wchar_t * img_icon_question;
		 const wchar_t * klinePic;
		 const wchar_t * lb_IDList;
		 const wchar_t * lb_buyID;
		 const wchar_t * lb_class;
		 const wchar_t * lb_sellID;
		 const wchar_t * lb_target;
		 const wchar_t * lc_avgResult;
		 const wchar_t * lc_cond;
		 const wchar_t * lc_func;
		 const wchar_t * lc_planName;
		 const wchar_t * lc_testResult;
		 const wchar_t * list_kbElf;
		 const wchar_t * ls_cancel;
		 const wchar_t * ls_deal;
		 const wchar_t * ls_dealSummary;
		 const wchar_t * ls_filter;
		 const wchar_t * ls_filterHis;
		 const wchar_t * ls_hisDeal;
		 const wchar_t * ls_hisTrust;
		 const wchar_t * ls_luStock0;
		 const wchar_t * ls_luStock1;
		 const wchar_t * ls_luStock2;
		 const wchar_t * ls_luStock3;
		 const wchar_t * ls_luStock4;
		 const wchar_t * ls_luStock5;
		 const wchar_t * ls_luStock6;
		 const wchar_t * ls_luStock7;
		 const wchar_t * ls_luStock8;
		 const wchar_t * ls_luStock9;
		 const wchar_t * ls_plate0;
		 const wchar_t * ls_plate1;
		 const wchar_t * ls_plate2;
		 const wchar_t * ls_plate3;
		 const wchar_t * ls_plate4;
		 const wchar_t * ls_plate5;
		 const wchar_t * ls_plate6;
		 const wchar_t * ls_plate7;
		 const wchar_t * ls_plate8;
		 const wchar_t * ls_plate9;
		 const wchar_t * ls_position;
		 const wchar_t * ls_rps;
		 const wchar_t * ls_trust;
		 const wchar_t * lv_title;
		 const wchar_t * msgicon;
		 const wchar_t * msgtext;
		 const wchar_t * msgtitle;
		 const wchar_t * radio_BackRehab;
		 const wchar_t * radio_FrontRehab;
		 const wchar_t * rdb_and;
		 const wchar_t * rdb_exist;
		 const wchar_t * rdb_forall;
		 const wchar_t * rdb_or;
		 const wchar_t * re_Log;
		 const wchar_t * re_frml;
		 const wchar_t * spin_buy;
		 const wchar_t * spin_buyPrice;
		 const wchar_t * spin_sell;
		 const wchar_t * spin_sellPrice;
		 const wchar_t * tab_ctrl;
		 const wchar_t * tab_stockFilter;
		 const wchar_t * tc_frml;
		 const wchar_t * text_FilterName;
		 const wchar_t * text_Group;
		 const wchar_t * text_ID;
		 const wchar_t * text_IDHis;
		 const wchar_t * text_ShowIndy;
		 const wchar_t * text_TestState;
		 const wchar_t * text_availaleFunds;
		 const wchar_t * text_cancelCount;
		 const wchar_t * text_condition;
		 const wchar_t * text_conditionHis;
		 const wchar_t * text_countDayBack;
		 const wchar_t * text_countDayFront;
		 const wchar_t * text_dealCount;
		 const wchar_t * text_dealSumCount;
		 const wchar_t * text_func;
		 const wchar_t * text_funcHis;
		 const wchar_t * text_hisDealCount;
		 const wchar_t * text_hisInfo;
		 const wchar_t * text_hisTrustCount;
		 const wchar_t * text_index1;
		 const wchar_t * text_index1His;
		 const wchar_t * text_index2;
		 const wchar_t * text_index2His;
		 const wchar_t * text_marketValue;
		 const wchar_t * text_maxBuy;
		 const wchar_t * text_maxSell;
		 const wchar_t * text_num;
		 const wchar_t * text_numHis;
		 const wchar_t * text_paratitle;
		 const wchar_t * text_period1;
		 const wchar_t * text_period1His;
		 const wchar_t * text_period2;
		 const wchar_t * text_period2His;
		 const wchar_t * text_posCount;
		 const wchar_t * text_profit;
		 const wchar_t * text_profitChg;
		 const wchar_t * text_totalCapital;
		 const wchar_t * text_trustCount;
		 const wchar_t * text_windowName;
		 const wchar_t * tradeInfo;
		 const wchar_t * txt_IDInfo;
		 const wchar_t * txt_PsdCfmInfo;
		 const wchar_t * txt_PsdInfo;
		 const wchar_t * txt_SrcPsdInfo;
		 const wchar_t * txt_aftreTrade;
		 const wchar_t * txt_buyAmo;
		 const wchar_t * txt_buyPrice;
		 const wchar_t * txt_buyVol;
		 const wchar_t * txt_buyVolFix;
		 const wchar_t * txt_buyVolInBig;
		 const wchar_t * txt_cagePrice;
		 const wchar_t * txt_cancelConfirm;
		 const wchar_t * txt_changePage;
		 const wchar_t * txt_clickVol;
		 const wchar_t * txt_day0;
		 const wchar_t * txt_day1;
		 const wchar_t * txt_day2;
		 const wchar_t * txt_day3;
		 const wchar_t * txt_day4;
		 const wchar_t * txt_day5;
		 const wchar_t * txt_day6;
		 const wchar_t * txt_day7;
		 const wchar_t * txt_day8;
		 const wchar_t * txt_day9;
		 const wchar_t * txt_hisFilterCalc;
		 const wchar_t * txt_limitPrice;
		 const wchar_t * txt_login;
		 const wchar_t * txt_maxVol;
		 const wchar_t * txt_psd;
		 const wchar_t * txt_sellPrice;
		 const wchar_t * txt_sellVol;
		 const wchar_t * txt_sellVolInBig;
		 const wchar_t * txt_showDeal;
		 const wchar_t * txt_srcPsd;
		 const wchar_t * txt_title;
		 const wchar_t * txt_trade;
		 const wchar_t * txt_trustConfirm;
		 const wchar_t * txt_trustInfo;
		 const wchar_t * txt_windowTime;
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
		const static int btnSwitch	=	65755;
		const static int btn_Add	=	65653;
		const static int btn_BandCancel	=	65589;
		const static int btn_BandClose	=	65581;
		const static int btn_BandDefault	=	65590;
		const static int btn_BandOK	=	65588;
		const static int btn_BandSetDefault	=	65591;
		const static int btn_Cancel	=	65661;
		const static int btn_Change	=	65742;
		const static int btn_Day	=	65608;
		const static int btn_Delete	=	65654;
		const static int btn_EMACancel	=	65596;
		const static int btn_EMAClose	=	65592;
		const static int btn_EMADefault	=	65597;
		const static int btn_EMAOK	=	65595;
		const static int btn_EMASetDefault	=	65598;
		const static int btn_Edit	=	65634;
		const static int btn_EditFunc	=	65540;
		const static int btn_FS	=	65602;
		const static int btn_Find	=	65632;
		const static int btn_IDList	=	65562;
		const static int btn_Insert	=	65728;
		const static int btn_LimitUpStat	=	65538;
		const static int btn_ListConnect1	=	65609;
		const static int btn_ListConnect2	=	65610;
		const static int btn_M1	=	65603;
		const static int btn_M15	=	65605;
		const static int btn_M30	=	65606;
		const static int btn_M5	=	65604;
		const static int btn_M60	=	65607;
		const static int btn_MACDCancel	=	65578;
		const static int btn_MACDDefault	=	65579;
		const static int btn_MACDOK	=	65577;
		const static int btn_MACDSetDefault	=	65580;
		const static int btn_MaCancel	=	65560;
		const static int btn_MaClose	=	65550;
		const static int btn_MaDefault	=	65557;
		const static int btn_MaOK	=	65559;
		const static int btn_MaSetDefault	=	65558;
		const static int btn_MacdClose	=	65573;
		const static int btn_Market	=	65601;
		const static int btn_MultiFilter	=	65537;
		const static int btn_New	=	65741;
		const static int btn_OK	=	65628;
		const static int btn_OKHis	=	65887;
		const static int btn_Read	=	65655;
		const static int btn_Rehab	=	65618;
		const static int btn_Save	=	65656;
		const static int btn_StockFilter	=	65611;
		const static int btn_Test	=	65729;
		const static int btn_TitleSel	=	65612;
		const static int btn_TradSimulate	=	65539;
		const static int btn_Usage	=	65633;
		const static int btn_backTesting	=	65844;
		const static int btn_buy	=	65773;
		const static int btn_buyAll	=	65767;
		const static int btn_buyCon	=	65760;
		const static int btn_buyHalf	=	65768;
		const static int btn_buyQuarter	=	65770;
		const static int btn_buySetting	=	65772;
		const static int btn_buyThird	=	65769;
		const static int btn_buyUnCon	=	65761;
		const static int btn_cancel	=	65629;
		const static int btn_cancelAll	=	65809;
		const static int btn_cancelDownload	=	65811;
		const static int btn_cancelMultiple	=	65808;
		const static int btn_changePsd	=	65571;
		const static int btn_close	=	65541;
		const static int btn_dealDownload	=	65817;
		const static int btn_dealIDSearch	=	65815;
		const static int btn_dealSumDownload	=	65822;
		const static int btn_dealSumSearch	=	65820;
		const static int btn_delete	=	65748;
		const static int btn_downFind	=	65745;
		const static int btn_export	=	65897;
		const static int btn_help	=	65545;
		const static int btn_hisDealDownload	=	65836;
		const static int btn_hisDealIDSearch	=	65834;
		const static int btn_hisFilter	=	65841;
		const static int btn_hisTrustDownload	=	65829;
		const static int btn_hisTrustIDSearch	=	65827;
		const static int btn_kbElfClose	=	65547;
		const static int btn_login	=	65567;
		const static int btn_market	=	65568;
		const static int btn_max	=	65542;
		const static int btn_min	=	65544;
		const static int btn_mulitWindow	=	65536;
		const static int btn_posDownload	=	65801;
		const static int btn_posIDSearch	=	65799;
		const static int btn_quit	=	65569;
		const static int btn_refreshAcc	=	65796;
		const static int btn_register	=	65570;
		const static int btn_restore	=	65543;
		const static int btn_rtFilter	=	65840;
		const static int btn_saveHis	=	65863;
		const static int btn_sell	=	65789;
		const static int btn_sellAll	=	65783;
		const static int btn_sellCon	=	65776;
		const static int btn_sellHalf	=	65784;
		const static int btn_sellQuarter	=	65786;
		const static int btn_sellSetting	=	65788;
		const static int btn_sellThird	=	65785;
		const static int btn_sellUnCon	=	65777;
		const static int btn_setFilterName	=	65842;
		const static int btn_testStart	=	65896;
		const static int btn_trustDownload	=	65806;
		const static int btn_trustIDSearch	=	65804;
		const static int btn_upFind	=	65744;
		const static int btn_wndNum	=	65901;
		const static int button1st	=	65756;
		const static int button2nd	=	65757;
		const static int button3rd	=	65758;
		const static int cbx_ID	=	65849;
		const static int cbx_IDHis	=	65869;
		const static int cbx_aftreTrade	=	65966;
		const static int cbx_buyPrice	=	65948;
		const static int cbx_buyVol	=	65950;
		const static int cbx_cagePrice	=	65960;
		const static int cbx_cancelConfirm	=	65974;
		const static int cbx_changePage	=	65980;
		const static int cbx_clickVol	=	65968;
		const static int cbx_condition	=	65855;
		const static int cbx_conditionHis	=	65875;
		const static int cbx_frml	=	65631;
		const static int cbx_func	=	65847;
		const static int cbx_funcHis	=	65867;
		const static int cbx_index1	=	65851;
		const static int cbx_index1His	=	65871;
		const static int cbx_index2	=	65857;
		const static int cbx_index2His	=	65877;
		const static int cbx_limitPrice	=	65958;
		const static int cbx_open	=	65630;
		const static int cbx_period	=	65652;
		const static int cbx_period1	=	65853;
		const static int cbx_period1His	=	65873;
		const static int cbx_period2	=	65860;
		const static int cbx_period2His	=	65880;
		const static int cbx_sellPrice	=	65952;
		const static int cbx_sellVol	=	65954;
		const static int cbx_showDeal	=	65972;
		const static int cbx_showTrust	=	65978;
		const static int cbx_trustConfirm	=	65976;
		const static int cbx_type	=	65727;
		const static int chk_NewStock	=	65617;
		const static int chk_SBM	=	65615;
		const static int chk_ST	=	65614;
		const static int chk_STARM	=	65616;
		const static int chk_autoLogin	=	65565;
		const static int chk_cancelAll	=	65813;
		const static int chk_rember	=	65564;
		const static int chk_title	=	65889;
		const static int chk_use	=	65660;
		const static int chk_useHis	=	65862;
		const static int dtp_hisDealEnd	=	65832;
		const static int dtp_hisDealStart	=	65831;
		const static int dtp_hisTrustEnd	=	65825;
		const static int dtp_hisTrustStart	=	65824;
		const static int dtp_rehab	=	65890;
		const static int dtp_testEnd	=	65895;
		const static int dtp_testStart	=	65894;
		const static int edit_BandParaK	=	65584;
		const static int edit_BandParaM1	=	65585;
		const static int edit_BandParaM2	=	65586;
		const static int edit_BandParaN1	=	65582;
		const static int edit_BandParaN2	=	65583;
		const static int edit_BandParaP	=	65587;
		const static int edit_EMAPara1	=	65593;
		const static int edit_EMAPara2	=	65594;
		const static int edit_FindFunc	=	65739;
		const static int edit_FindTarget	=	65839;
		const static int edit_ID	=	65561;
		const static int edit_MacdPara1	=	65574;
		const static int edit_MacdPara2	=	65575;
		const static int edit_MacdPara3	=	65576;
		const static int edit_Psd	=	65563;
		const static int edit_PsdConfirm	=	65944;
		const static int edit_SrcPsd	=	65940;
		const static int edit_buyAmo	=	65964;
		const static int edit_buyId	=	65759;
		const static int edit_buyPrice	=	65762;
		const static int edit_buyVol	=	65764;
		const static int edit_buyVolFix	=	65962;
		const static int edit_countDay	=	65883;
		const static int edit_dealIDSearch	=	65814;
		const static int edit_dealSumSearch	=	65819;
		const static int edit_dscp	=	65662;
		const static int edit_explain	=	65734;
		const static int edit_fileName	=	65746;
		const static int edit_funcDscp	=	65738;
		const static int edit_hisDealIDSearch	=	65833;
		const static int edit_hisTrustIDSearch	=	65826;
		const static int edit_kbElf	=	65548;
		const static int edit_keyWord	=	65743;
		const static int edit_maPara1	=	65551;
		const static int edit_maPara2	=	65552;
		const static int edit_maPara3	=	65553;
		const static int edit_maPara4	=	65554;
		const static int edit_maPara5	=	65555;
		const static int edit_maPara6	=	65556;
		const static int edit_maxVol	=	65956;
		const static int edit_name	=	65627;
		const static int edit_num	=	65861;
		const static int edit_numHis	=	65881;
		const static int edit_para1	=	65636;
		const static int edit_para10	=	65645;
		const static int edit_para10def	=	65702;
		const static int edit_para10max	=	65701;
		const static int edit_para10min	=	65700;
		const static int edit_para10name	=	65699;
		const static int edit_para11	=	65646;
		const static int edit_para11def	=	65706;
		const static int edit_para11max	=	65705;
		const static int edit_para11min	=	65704;
		const static int edit_para11name	=	65703;
		const static int edit_para12	=	65647;
		const static int edit_para12def	=	65710;
		const static int edit_para12max	=	65709;
		const static int edit_para12min	=	65708;
		const static int edit_para12name	=	65707;
		const static int edit_para13	=	65648;
		const static int edit_para13def	=	65714;
		const static int edit_para13max	=	65713;
		const static int edit_para13min	=	65712;
		const static int edit_para13name	=	65711;
		const static int edit_para14	=	65649;
		const static int edit_para14def	=	65718;
		const static int edit_para14max	=	65717;
		const static int edit_para14min	=	65716;
		const static int edit_para14name	=	65715;
		const static int edit_para15	=	65650;
		const static int edit_para15def	=	65722;
		const static int edit_para15max	=	65721;
		const static int edit_para15min	=	65720;
		const static int edit_para15name	=	65719;
		const static int edit_para16	=	65651;
		const static int edit_para16def	=	65726;
		const static int edit_para16max	=	65725;
		const static int edit_para16min	=	65724;
		const static int edit_para16name	=	65723;
		const static int edit_para1def	=	65666;
		const static int edit_para1max	=	65665;
		const static int edit_para1min	=	65664;
		const static int edit_para1name	=	65663;
		const static int edit_para2	=	65637;
		const static int edit_para2def	=	65670;
		const static int edit_para2max	=	65669;
		const static int edit_para2min	=	65668;
		const static int edit_para2name	=	65667;
		const static int edit_para3	=	65638;
		const static int edit_para3def	=	65674;
		const static int edit_para3max	=	65673;
		const static int edit_para3min	=	65672;
		const static int edit_para3name	=	65671;
		const static int edit_para4	=	65639;
		const static int edit_para4def	=	65678;
		const static int edit_para4max	=	65677;
		const static int edit_para4min	=	65676;
		const static int edit_para4name	=	65675;
		const static int edit_para5	=	65640;
		const static int edit_para5def	=	65682;
		const static int edit_para5max	=	65681;
		const static int edit_para5min	=	65680;
		const static int edit_para5name	=	65679;
		const static int edit_para6	=	65641;
		const static int edit_para6def	=	65686;
		const static int edit_para6max	=	65685;
		const static int edit_para6min	=	65684;
		const static int edit_para6name	=	65683;
		const static int edit_para7	=	65642;
		const static int edit_para7def	=	65690;
		const static int edit_para7max	=	65689;
		const static int edit_para7min	=	65688;
		const static int edit_para7name	=	65687;
		const static int edit_para8	=	65643;
		const static int edit_para8def	=	65694;
		const static int edit_para8max	=	65693;
		const static int edit_para8min	=	65692;
		const static int edit_para8name	=	65691;
		const static int edit_para9	=	65644;
		const static int edit_para9def	=	65698;
		const static int edit_para9max	=	65697;
		const static int edit_para9min	=	65696;
		const static int edit_para9name	=	65695;
		const static int edit_param	=	65733;
		const static int edit_posIDSearch	=	65798;
		const static int edit_result	=	65732;
		const static int edit_sellId	=	65775;
		const static int edit_sellPrice	=	65778;
		const static int edit_sellVol	=	65780;
		const static int edit_trans	=	65731;
		const static int edit_trustIDSearch	=	65803;
		const static int edit_windowTime	=	65970;
		const static int fenshiPic	=	65620;
		const static int img_icon_exclamation	=	65753;
		const static int img_icon_information	=	65751;
		const static int img_icon_question	=	65752;
		const static int klinePic	=	65621;
		const static int lb_IDList	=	65566;
		const static int lb_buyID	=	65774;
		const static int lb_class	=	65736;
		const static int lb_sellID	=	65790;
		const static int lb_target	=	65838;
		const static int lc_avgResult	=	65898;
		const static int lc_cond	=	65657;
		const static int lc_func	=	65737;
		const static int lc_planName	=	65747;
		const static int lc_testResult	=	65900;
		const static int list_kbElf	=	65549;
		const static int ls_cancel	=	65812;
		const static int ls_deal	=	65818;
		const static int ls_dealSummary	=	65823;
		const static int ls_filter	=	65845;
		const static int ls_filterHis	=	65865;
		const static int ls_hisDeal	=	65837;
		const static int ls_hisTrust	=	65830;
		const static int ls_luStock0	=	65910;
		const static int ls_luStock1	=	65913;
		const static int ls_luStock2	=	65916;
		const static int ls_luStock3	=	65919;
		const static int ls_luStock4	=	65922;
		const static int ls_luStock5	=	65925;
		const static int ls_luStock6	=	65928;
		const static int ls_luStock7	=	65931;
		const static int ls_luStock8	=	65934;
		const static int ls_luStock9	=	65937;
		const static int ls_plate0	=	65909;
		const static int ls_plate1	=	65912;
		const static int ls_plate2	=	65915;
		const static int ls_plate3	=	65918;
		const static int ls_plate4	=	65921;
		const static int ls_plate5	=	65924;
		const static int ls_plate6	=	65927;
		const static int ls_plate7	=	65930;
		const static int ls_plate8	=	65933;
		const static int ls_plate9	=	65936;
		const static int ls_position	=	65802;
		const static int ls_rps	=	65619;
		const static int ls_trust	=	65807;
		const static int lv_title	=	65888;
		const static int msgicon	=	65750;
		const static int msgtext	=	65754;
		const static int msgtitle	=	65749;
		const static int radio_BackRehab	=	65892;
		const static int radio_FrontRehab	=	65891;
		const static int rdb_and	=	65658;
		const static int rdb_exist	=	65885;
		const static int rdb_forall	=	65886;
		const static int rdb_or	=	65659;
		const static int re_Log	=	65893;
		const static int re_frml	=	65735;
		const static int spin_buy	=	65765;
		const static int spin_buyPrice	=	65763;
		const static int spin_sell	=	65781;
		const static int spin_sellPrice	=	65779;
		const static int tab_ctrl	=	65730;
		const static int tab_stockFilter	=	65843;
		const static int tc_frml	=	65740;
		const static int text_FilterName	=	65599;
		const static int text_Group	=	65600;
		const static int text_ID	=	65848;
		const static int text_IDHis	=	65868;
		const static int text_ShowIndy	=	65613;
		const static int text_TestState	=	65899;
		const static int text_availaleFunds	=	65795;
		const static int text_cancelCount	=	65810;
		const static int text_condition	=	65854;
		const static int text_conditionHis	=	65874;
		const static int text_countDayBack	=	65884;
		const static int text_countDayFront	=	65882;
		const static int text_dealCount	=	65816;
		const static int text_dealSumCount	=	65821;
		const static int text_func	=	65846;
		const static int text_funcHis	=	65866;
		const static int text_hisDealCount	=	65835;
		const static int text_hisInfo	=	65864;
		const static int text_hisTrustCount	=	65828;
		const static int text_index1	=	65850;
		const static int text_index1His	=	65870;
		const static int text_index2	=	65856;
		const static int text_index2His	=	65876;
		const static int text_marketValue	=	65792;
		const static int text_maxBuy	=	65766;
		const static int text_maxSell	=	65782;
		const static int text_num	=	65859;
		const static int text_numHis	=	65879;
		const static int text_paratitle	=	65635;
		const static int text_period1	=	65852;
		const static int text_period1His	=	65872;
		const static int text_period2	=	65858;
		const static int text_period2His	=	65878;
		const static int text_posCount	=	65800;
		const static int text_profit	=	65793;
		const static int text_profitChg	=	65794;
		const static int text_totalCapital	=	65791;
		const static int text_trustCount	=	65805;
		const static int text_windowName	=	65626;
		const static int tradeInfo	=	65797;
		const static int txt_IDInfo	=	65938;
		const static int txt_PsdCfmInfo	=	65945;
		const static int txt_PsdInfo	=	65943;
		const static int txt_SrcPsdInfo	=	65941;
		const static int txt_aftreTrade	=	65965;
		const static int txt_buyAmo	=	65963;
		const static int txt_buyPrice	=	65947;
		const static int txt_buyVol	=	65949;
		const static int txt_buyVolFix	=	65961;
		const static int txt_buyVolInBig	=	65771;
		const static int txt_cagePrice	=	65959;
		const static int txt_cancelConfirm	=	65973;
		const static int txt_changePage	=	65979;
		const static int txt_clickVol	=	65967;
		const static int txt_day0	=	65908;
		const static int txt_day1	=	65911;
		const static int txt_day2	=	65914;
		const static int txt_day3	=	65917;
		const static int txt_day4	=	65920;
		const static int txt_day5	=	65923;
		const static int txt_day6	=	65926;
		const static int txt_day7	=	65929;
		const static int txt_day8	=	65932;
		const static int txt_day9	=	65935;
		const static int txt_hisFilterCalc	=	65622;
		const static int txt_limitPrice	=	65957;
		const static int txt_login	=	65572;
		const static int txt_maxVol	=	65955;
		const static int txt_psd	=	65942;
		const static int txt_sellPrice	=	65951;
		const static int txt_sellVol	=	65953;
		const static int txt_sellVolInBig	=	65787;
		const static int txt_showDeal	=	65971;
		const static int txt_srcPsd	=	65939;
		const static int txt_title	=	65546;
		const static int txt_trade	=	65946;
		const static int txt_trustConfirm	=	65975;
		const static int txt_trustInfo	=	65977;
		const static int txt_windowTime	=	65969;
		const static int wnd1	=	65902;
		const static int wnd2	=	65903;
		const static int wnd3	=	65904;
		const static int wnd4	=	65905;
		const static int wnd5	=	65906;
		const static int wnd6	=	65907;
		const static int wnd_SWL1	=	65623;
		const static int wnd_SWL2	=	65624;
		const static int wnd_Stock	=	65625;
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
