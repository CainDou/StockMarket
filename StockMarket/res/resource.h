//stamp:17f9bc1365398ce5
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
				page_list = _T("LAYOUT:page_list");
				dlg_keyboardElf = _T("LAYOUT:dlg_keyboardElf");
				dlg_maPara = _T("LAYOUT:dlg_maPara");
				dlg_login = _T("LAYOUT:dlg_login");
				dlg_macdPara = _T("LAYOUT:dlg_macdPara");
				dlg_BandPara = _T("LAYOUT:dlg_BandPara");
				dlg_emaPara = _T("LAYOUT:dlg_emaPara");
			}
			const TCHAR * XML_MAINWND;
			const TCHAR * page_list;
			const TCHAR * dlg_keyboardElf;
			const TCHAR * dlg_maPara;
			const TCHAR * dlg_login;
			const TCHAR * dlg_macdPara;
			const TCHAR * dlg_BandPara;
			const TCHAR * dlg_emaPara;
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
			}
			const TCHAR * menu_fenshi;
			const TCHAR * menu_kline;
		}smenu;
	};
	const SNamedID::NAMEDVALUE namedXmlID[]={
		{L"List_Ind1",65549},
		{L"List_Ind2",65561},
		{L"List_Ind3",65573},
		{L"_name_start",65535},
		{L"btn_BandCancel",65610},
		{L"btn_BandClose",65602},
		{L"btn_BandDefault",65611},
		{L"btn_BandOK",65609},
		{L"btn_BandSetDefault",65612},
		{L"btn_EMACancel",65617},
		{L"btn_EMAClose",65613},
		{L"btn_EMADefault",65618},
		{L"btn_EMAOK",65616},
		{L"btn_EMASetDefault",65619},
		{L"btn_Ind1Day",65548},
		{L"btn_Ind1FS",65542},
		{L"btn_Ind1M1",65543},
		{L"btn_Ind1M15",65545},
		{L"btn_Ind1M30",65546},
		{L"btn_Ind1M5",65544},
		{L"btn_Ind1M60",65547},
		{L"btn_Ind1Market",65541},
		{L"btn_Ind2Day",65560},
		{L"btn_Ind2FS",65554},
		{L"btn_Ind2M1",65555},
		{L"btn_Ind2M15",65557},
		{L"btn_Ind2M30",65558},
		{L"btn_Ind2M5",65556},
		{L"btn_Ind2M60",65559},
		{L"btn_Ind2Market",65553},
		{L"btn_Ind3Day",65572},
		{L"btn_Ind3FS",65566},
		{L"btn_Ind3M1",65567},
		{L"btn_Ind3M15",65569},
		{L"btn_Ind3M30",65570},
		{L"btn_Ind3M5",65568},
		{L"btn_Ind3M60",65571},
		{L"btn_Ind3Market",65565},
		{L"btn_MACDCancel",65599},
		{L"btn_MACDDefault",65600},
		{L"btn_MACDOK",65598},
		{L"btn_MACDSetDefault",65601},
		{L"btn_MaCancel",65586},
		{L"btn_MaClose",65580},
		{L"btn_MaDefault",65587},
		{L"btn_MaOK",65585},
		{L"btn_MaSetDefault",65588},
		{L"btn_MacdClose",65594},
		{L"btn_close",65536},
		{L"btn_kbElfClose",65577},
		{L"btn_login",65591},
		{L"btn_max",65537},
		{L"btn_min",65539},
		{L"btn_quit",65592},
		{L"btn_restore",65538},
		{L"edit_BandParaK",65605},
		{L"edit_BandParaM1",65606},
		{L"edit_BandParaM2",65607},
		{L"edit_BandParaN1",65603},
		{L"edit_BandParaN2",65604},
		{L"edit_BandParaP",65608},
		{L"edit_EMAPara1",65614},
		{L"edit_EMAPara2",65615},
		{L"edit_MacdPara1",65595},
		{L"edit_MacdPara2",65596},
		{L"edit_MacdPara3",65597},
		{L"edit_kbElf",65578},
		{L"edit_maPara1",65581},
		{L"edit_maPara2",65582},
		{L"edit_maPara3",65583},
		{L"edit_maPara4",65584},
		{L"fenshiPic_Ind1",65550},
		{L"fenshiPic_Ind2",65562},
		{L"fenshiPic_Ind3",65574},
		{L"klinePic_Ind1",65551},
		{L"klinePic_Ind2",65563},
		{L"klinePic_Ind3",65575},
		{L"list_kbElf",65579},
		{L"login_ID",65589},
		{L"login_PassWord",65590},
		{L"txt_login",65593},
		{L"txt_title",65576},
		{L"wnd_Ind1",65540},
		{L"wnd_Ind2",65552},
		{L"wnd_Ind3",65564}		};
	class _R{
	public:
		class _name{
		public:
		_name(){
			List_Ind1 = namedXmlID[0].strName;
			List_Ind2 = namedXmlID[1].strName;
			List_Ind3 = namedXmlID[2].strName;
			_name_start = namedXmlID[3].strName;
			btn_BandCancel = namedXmlID[4].strName;
			btn_BandClose = namedXmlID[5].strName;
			btn_BandDefault = namedXmlID[6].strName;
			btn_BandOK = namedXmlID[7].strName;
			btn_BandSetDefault = namedXmlID[8].strName;
			btn_EMACancel = namedXmlID[9].strName;
			btn_EMAClose = namedXmlID[10].strName;
			btn_EMADefault = namedXmlID[11].strName;
			btn_EMAOK = namedXmlID[12].strName;
			btn_EMASetDefault = namedXmlID[13].strName;
			btn_Ind1Day = namedXmlID[14].strName;
			btn_Ind1FS = namedXmlID[15].strName;
			btn_Ind1M1 = namedXmlID[16].strName;
			btn_Ind1M15 = namedXmlID[17].strName;
			btn_Ind1M30 = namedXmlID[18].strName;
			btn_Ind1M5 = namedXmlID[19].strName;
			btn_Ind1M60 = namedXmlID[20].strName;
			btn_Ind1Market = namedXmlID[21].strName;
			btn_Ind2Day = namedXmlID[22].strName;
			btn_Ind2FS = namedXmlID[23].strName;
			btn_Ind2M1 = namedXmlID[24].strName;
			btn_Ind2M15 = namedXmlID[25].strName;
			btn_Ind2M30 = namedXmlID[26].strName;
			btn_Ind2M5 = namedXmlID[27].strName;
			btn_Ind2M60 = namedXmlID[28].strName;
			btn_Ind2Market = namedXmlID[29].strName;
			btn_Ind3Day = namedXmlID[30].strName;
			btn_Ind3FS = namedXmlID[31].strName;
			btn_Ind3M1 = namedXmlID[32].strName;
			btn_Ind3M15 = namedXmlID[33].strName;
			btn_Ind3M30 = namedXmlID[34].strName;
			btn_Ind3M5 = namedXmlID[35].strName;
			btn_Ind3M60 = namedXmlID[36].strName;
			btn_Ind3Market = namedXmlID[37].strName;
			btn_MACDCancel = namedXmlID[38].strName;
			btn_MACDDefault = namedXmlID[39].strName;
			btn_MACDOK = namedXmlID[40].strName;
			btn_MACDSetDefault = namedXmlID[41].strName;
			btn_MaCancel = namedXmlID[42].strName;
			btn_MaClose = namedXmlID[43].strName;
			btn_MaDefault = namedXmlID[44].strName;
			btn_MaOK = namedXmlID[45].strName;
			btn_MaSetDefault = namedXmlID[46].strName;
			btn_MacdClose = namedXmlID[47].strName;
			btn_close = namedXmlID[48].strName;
			btn_kbElfClose = namedXmlID[49].strName;
			btn_login = namedXmlID[50].strName;
			btn_max = namedXmlID[51].strName;
			btn_min = namedXmlID[52].strName;
			btn_quit = namedXmlID[53].strName;
			btn_restore = namedXmlID[54].strName;
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
			fenshiPic_Ind1 = namedXmlID[71].strName;
			fenshiPic_Ind2 = namedXmlID[72].strName;
			fenshiPic_Ind3 = namedXmlID[73].strName;
			klinePic_Ind1 = namedXmlID[74].strName;
			klinePic_Ind2 = namedXmlID[75].strName;
			klinePic_Ind3 = namedXmlID[76].strName;
			list_kbElf = namedXmlID[77].strName;
			login_ID = namedXmlID[78].strName;
			login_PassWord = namedXmlID[79].strName;
			txt_login = namedXmlID[80].strName;
			txt_title = namedXmlID[81].strName;
			wnd_Ind1 = namedXmlID[82].strName;
			wnd_Ind2 = namedXmlID[83].strName;
			wnd_Ind3 = namedXmlID[84].strName;
		}
		 const wchar_t * List_Ind1;
		 const wchar_t * List_Ind2;
		 const wchar_t * List_Ind3;
		 const wchar_t * _name_start;
		 const wchar_t * btn_BandCancel;
		 const wchar_t * btn_BandClose;
		 const wchar_t * btn_BandDefault;
		 const wchar_t * btn_BandOK;
		 const wchar_t * btn_BandSetDefault;
		 const wchar_t * btn_EMACancel;
		 const wchar_t * btn_EMAClose;
		 const wchar_t * btn_EMADefault;
		 const wchar_t * btn_EMAOK;
		 const wchar_t * btn_EMASetDefault;
		 const wchar_t * btn_Ind1Day;
		 const wchar_t * btn_Ind1FS;
		 const wchar_t * btn_Ind1M1;
		 const wchar_t * btn_Ind1M15;
		 const wchar_t * btn_Ind1M30;
		 const wchar_t * btn_Ind1M5;
		 const wchar_t * btn_Ind1M60;
		 const wchar_t * btn_Ind1Market;
		 const wchar_t * btn_Ind2Day;
		 const wchar_t * btn_Ind2FS;
		 const wchar_t * btn_Ind2M1;
		 const wchar_t * btn_Ind2M15;
		 const wchar_t * btn_Ind2M30;
		 const wchar_t * btn_Ind2M5;
		 const wchar_t * btn_Ind2M60;
		 const wchar_t * btn_Ind2Market;
		 const wchar_t * btn_Ind3Day;
		 const wchar_t * btn_Ind3FS;
		 const wchar_t * btn_Ind3M1;
		 const wchar_t * btn_Ind3M15;
		 const wchar_t * btn_Ind3M30;
		 const wchar_t * btn_Ind3M5;
		 const wchar_t * btn_Ind3M60;
		 const wchar_t * btn_Ind3Market;
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
		 const wchar_t * btn_close;
		 const wchar_t * btn_kbElfClose;
		 const wchar_t * btn_login;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_quit;
		 const wchar_t * btn_restore;
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
		 const wchar_t * fenshiPic_Ind1;
		 const wchar_t * fenshiPic_Ind2;
		 const wchar_t * fenshiPic_Ind3;
		 const wchar_t * klinePic_Ind1;
		 const wchar_t * klinePic_Ind2;
		 const wchar_t * klinePic_Ind3;
		 const wchar_t * list_kbElf;
		 const wchar_t * login_ID;
		 const wchar_t * login_PassWord;
		 const wchar_t * txt_login;
		 const wchar_t * txt_title;
		 const wchar_t * wnd_Ind1;
		 const wchar_t * wnd_Ind2;
		 const wchar_t * wnd_Ind3;
		}name;

		class _id{
		public:
		const static int List_Ind1	=	65549;
		const static int List_Ind2	=	65561;
		const static int List_Ind3	=	65573;
		const static int _name_start	=	65535;
		const static int btn_BandCancel	=	65610;
		const static int btn_BandClose	=	65602;
		const static int btn_BandDefault	=	65611;
		const static int btn_BandOK	=	65609;
		const static int btn_BandSetDefault	=	65612;
		const static int btn_EMACancel	=	65617;
		const static int btn_EMAClose	=	65613;
		const static int btn_EMADefault	=	65618;
		const static int btn_EMAOK	=	65616;
		const static int btn_EMASetDefault	=	65619;
		const static int btn_Ind1Day	=	65548;
		const static int btn_Ind1FS	=	65542;
		const static int btn_Ind1M1	=	65543;
		const static int btn_Ind1M15	=	65545;
		const static int btn_Ind1M30	=	65546;
		const static int btn_Ind1M5	=	65544;
		const static int btn_Ind1M60	=	65547;
		const static int btn_Ind1Market	=	65541;
		const static int btn_Ind2Day	=	65560;
		const static int btn_Ind2FS	=	65554;
		const static int btn_Ind2M1	=	65555;
		const static int btn_Ind2M15	=	65557;
		const static int btn_Ind2M30	=	65558;
		const static int btn_Ind2M5	=	65556;
		const static int btn_Ind2M60	=	65559;
		const static int btn_Ind2Market	=	65553;
		const static int btn_Ind3Day	=	65572;
		const static int btn_Ind3FS	=	65566;
		const static int btn_Ind3M1	=	65567;
		const static int btn_Ind3M15	=	65569;
		const static int btn_Ind3M30	=	65570;
		const static int btn_Ind3M5	=	65568;
		const static int btn_Ind3M60	=	65571;
		const static int btn_Ind3Market	=	65565;
		const static int btn_MACDCancel	=	65599;
		const static int btn_MACDDefault	=	65600;
		const static int btn_MACDOK	=	65598;
		const static int btn_MACDSetDefault	=	65601;
		const static int btn_MaCancel	=	65586;
		const static int btn_MaClose	=	65580;
		const static int btn_MaDefault	=	65587;
		const static int btn_MaOK	=	65585;
		const static int btn_MaSetDefault	=	65588;
		const static int btn_MacdClose	=	65594;
		const static int btn_close	=	65536;
		const static int btn_kbElfClose	=	65577;
		const static int btn_login	=	65591;
		const static int btn_max	=	65537;
		const static int btn_min	=	65539;
		const static int btn_quit	=	65592;
		const static int btn_restore	=	65538;
		const static int edit_BandParaK	=	65605;
		const static int edit_BandParaM1	=	65606;
		const static int edit_BandParaM2	=	65607;
		const static int edit_BandParaN1	=	65603;
		const static int edit_BandParaN2	=	65604;
		const static int edit_BandParaP	=	65608;
		const static int edit_EMAPara1	=	65614;
		const static int edit_EMAPara2	=	65615;
		const static int edit_MacdPara1	=	65595;
		const static int edit_MacdPara2	=	65596;
		const static int edit_MacdPara3	=	65597;
		const static int edit_kbElf	=	65578;
		const static int edit_maPara1	=	65581;
		const static int edit_maPara2	=	65582;
		const static int edit_maPara3	=	65583;
		const static int edit_maPara4	=	65584;
		const static int fenshiPic_Ind1	=	65550;
		const static int fenshiPic_Ind2	=	65562;
		const static int fenshiPic_Ind3	=	65574;
		const static int klinePic_Ind1	=	65551;
		const static int klinePic_Ind2	=	65563;
		const static int klinePic_Ind3	=	65575;
		const static int list_kbElf	=	65579;
		const static int login_ID	=	65589;
		const static int login_PassWord	=	65590;
		const static int txt_login	=	65593;
		const static int txt_title	=	65576;
		const static int wnd_Ind1	=	65540;
		const static int wnd_Ind2	=	65552;
		const static int wnd_Ind3	=	65564;
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
