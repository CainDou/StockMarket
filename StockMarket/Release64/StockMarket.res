        ��  ��                  �      �� ��     0         �4   V S _ V E R S I O N _ I N F O     ���                                         �   S t r i n g F i l e I n f o   �   0 8 0 4 0 4 b 0   @   F i l e D e s c r i p t i o n     S t o c k M a r k e t   0   F i l e V e r s i o n     1 . 0 . 0 . 2   8   I n t e r n a l N a m e   S t o c k M a r k e t   J   L e g a l C o p y r i g h t   C o p y r i g h t   ( C )   2 0 1 1     H   O r i g i n a l F i l e n a m e   S t o c k M a r k e t . e x e   8   P r o d u c t N a m e     S t o c k M a r k e t   4   P r o d u c t V e r s i o n   1 . 0 . 0 . 2   D    V a r F i l e I n f o     $    T r a n s l a t i o n     ��  8   U I D E F   X M L _ I N I T         0         ﻿<?xml version="1.0" encoding="utf-8"?>
<UIDEF>
  <font face="宋体" size="15"/>

  <string src="values:string" />
  <skin src="values:skin"/>
  <color src="values:color"/>


  <style>
    <class name="normalbtn" font="" colorText="#385e8b" colorTextDisable="#91a7c0" textMode="0x25" cursor="hand" margin-x="0"/>
    <class name="cls_btn_weblink" cursor="hand" colorText="#1e78d5" colorTextHover="#1e78d5" font="italic:1" fontHover="underline:1,italic:1" />
  </style>

  <objattr>
  </objattr>
</UIDEF>  �  @   L A Y O U T   X M L _ M A I N W N D         0         <?xml version="1.0"?>
<SOUI name="mainWindow" title="@string/title" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="1400" height="850" margin="1,5,5,5" resizable="1" wndType="appMain" appWnd="1" translucent="0" colorBkgnd="#000000ff">
	<root cache="1">
		<caption pos="0,0,-0,30" show="1" font="adding:0" colorBorder="#000000ff" colorBkgnd="#101010ff" cache="1">
			<icon pos="10,8" src="ICON_LOGO:16" />
			<text pos="29,9" colorText="#00FFFF">@string/title</text>
			<imgbtn name="btn_close" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			<imgbtn name="btn_max" skin="_skin.sys.btn.maximize" pos="-83,0" animate="1" />
			<imgbtn name="btn_restore" skin="_skin.sys.btn.restore" pos="-83,0" show="0" animate="1" />
			<imgbtn name="btn_min" skin="_skin.sys.btn.minimize" pos="-121,0" animate="1" />
		</caption>
		<window pos="0,30,-0,-0" colorBkgnd="#000000ff" cache="1">
			<include src="layout:page_list" />
		</window>
	</root>
</SOUI>
  6,  <   L A Y O U T   P A G E _ L I S T         0         ﻿<?xml version="1.0"?>
<include>
	<window name="wnd_Ind1" pos="5,5,%50,%40" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" font="face:微软雅黑">
		<text pos="5,3" colorText="#FF0000FF">申万1级行业</text>
		<imgbtn pos="[5,0" size="30,22" tip="市场行情" animate="0" text="行情" name="btn_Ind1Market" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="分时图" animate="0" text="分时" name="btn_Ind1FS" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="1分钟图" animate="0" text="1M" name="btn_Ind1M1" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="5分钟图" animate="0" text="5M" name="btn_Ind1M5" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="15分钟图" animate="0" text="15M" name="btn_Ind1M15" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="30分钟图" animate="0" text="30M" name="btn_Ind1M30" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="60分钟图" animate="0" text="60M" name="btn_Ind1M60" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="日频图" animate="0" text="日" name="btn_Ind1Day" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<color_listctrlex name="List_Ind1" cache="1" bkgndBlend="0" pos="5,25,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
			<headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
				<items>
					<item width="50">序号</item>
					<item width="65">证券代码</item>
					<item width="110">证券名称</item>
					<item width="70">最新价</item>
					<item width="60">涨幅%</item>
					<item width="65">rps520</item>
					<item width="80">MACD520</item>
					<item width="65">520分数</item>
					<item width="60">520排名</item>
					<item width="70">rps2060</item>
					<item width="90">MACD2060</item>
					<item width="70">2060分数</item>
					<item width="65">2060排名</item>
				</items>
			</headerex>
		</color_listctrlex>
		<fenshiPic name="fenshiPic_Ind1" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</fenshiPic>
		<klinePic name="klinePic_Ind1" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</klinePic>
	</window>
	<window name="wnd_Ind2" pos="5,%40,%50,-5" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" font="face:微软雅黑">
		<text pos="5,3" colorText="#FF0000FF">申万2级行业</text>
		<imgbtn pos="[5,0" size="30,22" tip="市场行情" animate="0" text="行情" name="btn_Ind2Market" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="分时图" animate="0" text="分时" name="btn_Ind2FS" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="1分钟图" animate="0" text="1M" name="btn_Ind2M1" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="5分钟图" animate="0" text="5M" name="btn_Ind2M5" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="15分钟图" animate="0" text="15M" name="btn_Ind2M15" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="30分钟图" animate="0" text="30M" name="btn_Ind2M30" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="60分钟图" animate="0" text="60M" name="btn_Ind2M60" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="日频图" animate="0" text="日" name="btn_Ind2Day" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="行情列表联动" animate="0" text="联" name="btn_Ind2ListConnect" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<text pos="[5,3" colorText="#c0c0c0ff" name = "text_List2ShowIndy">当前分类:全市场</text>
		<color_listctrlex name="List_Ind2" cache="1" bkgndBlend="0" pos="5,25,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
			<headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
				<items>
					<item width="50">序号</item>
					<item width="65">证券代码</item>
					<item width="110">证券名称</item>
					<item width="70">最新价</item>
					<item width="60">涨幅%</item>
					<item width="65">rps520</item>
					<item width="80">MACD520</item>
					<item width="65">520分数</item>
					<item width="60">520排名</item>
					<item width="70">rps2060</item>
					<item width="90">MACD2060</item>
					<item width="70">2060分数</item>
					<item width="65">2060排名</item>
				</items>
			</headerex>
		</color_listctrlex>
		<fenshiPic name="fenshiPic_Ind2" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</fenshiPic>
		<klinePic name="klinePic_Ind2" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</klinePic>
	</window>
	<window name="wnd_Ind3" pos="%50,5,-5,-5" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" font="face:微软雅黑">
		<text pos="5,3" colorText="#FF0000FF">全市场</text>
		<imgbtn pos="[5,0" size="30,22" tip="市场行情" animate="0" text="行情" name="btn_Ind3Market" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="分时图" animate="0" text="分时" name="btn_Ind3FS" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="1分钟图" animate="0" text="1M" name="btn_Ind3M1" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="5分钟图" animate="0" text="5M" name="btn_Ind3M5" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="30,22" tip="15分钟图" animate="0" text="15M" name="btn_Ind3M15" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="30分钟图" animate="0" text="30M" name="btn_Ind3M30" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="35,22" tip="60分钟图" animate="0" text="60M" name="btn_Ind3M60" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="日频图" animate="0" text="日" name="btn_Ind3Day" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="申万1级行情列表联动" animate="0" text="联1" name="btn_Ind3ListConnect1" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<imgbtn pos="[0,0" size="25,22" tip="申万2级行情列表联动" animate="0" text="联2" name="btn_Ind3ListConnect2" skin="skin_ButtonGrayBlack" msgTransparent="0" colorText="#c0c0c0ff" font="face:微软雅黑,size:13" valign="bottom" />
		<text pos="[5,3" colorText="#c0c0c0ff" name = "text_List3ShowIndy">当前分类:全市场</text>

		<color_listctrlex name="List_Ind3" cache="1" bkgndBlend="0" pos="5,25,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
			<headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
				<items>
					<item width="50">序号</item>
					<item width="65">证券代码</item>
					<item width="110">证券名称</item>
					<item width="70">最新价</item>
					<item width="60">涨幅%</item>
					<item width="65">rps520</item>
					<item width="80">MACD520</item>
					<item width="65">520分数</item>
					<item width="60">520排名</item>
					<item width="70">rps2060</item>
					<item width="90">MACD2060</item>
					<item width="70">2060分数</item>
					<item width="65">2060排名</item>
				</items>
			</headerex>
		</color_listctrlex>
		<fenshiPic name="fenshiPic_Ind3" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</fenshiPic>
		<klinePic name="klinePic_Ind3" pos="5,25,-10,-5" visible="0" colorBKgnd="#000000" font="size:12">
			<!--	<subPic  pos="5,%75,-0,-40" visible="1" colorBKgnd="#000000" font="size:12"/> -->
		</klinePic>
	</window>
</include>
  �  H   L A Y O U T   D L G _ K E Y B O A R D E L F         0         <?xml version="1.0"?>
<SOUI name="键盘精灵" title="布局" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="300" height="350" margin="1,5,5,5" wndType="8" toolWindow="1">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(0,0,0)" colorText="#ffffff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<text pos="5,4" name="txt_title">键盘精灵</text>
				<imgbtn name="btn_kbElfClose" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			</caption>
			<edit name="edit_kbElf" pos="4,32" size="292,24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000ff" autoSel="1" notifyChange="1" />
			<color_listctrlex name="list_kbElf" pos="4,56" size="292, 280" hotTrack="0" itemHeight="20" headerHeight="0" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#ffffffff">
				<headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1">
					<items>
						<item width="80">代码</item>
						<item width="150">名称</item>
						<item width="45">分类</item>
					</items>
				</headerex>
			</color_listctrlex>
		</window>
	</root>
</SOUI>
�  <   L A Y O U T   D L G _ M A P A R A       0         <?xml version="1.0"?>
<SOUI name="window" title="布局" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="365" height="200" margin="1,5,5,5" wndType="normal" toolWindow="0">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(70,70,70)" colorText="#ffffff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<text pos="5,4" name="txt_title">MA参数</text>
				<imgbtn name="btn_MaClose" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			</caption>
			<edit name="edit_maPara1" pos="80,48" size="64, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="40,48" size="40, 24">周期1</text>
			<edit name="edit_maPara2" pos="261,48" size="64, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="221,48" size="40, 24">周期2</text>
			<edit name="edit_maPara3" pos="80,96" size="64, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="40,96" size="40, 24">周期3</text>
			<edit name="edit_maPara4" pos="261,96" size="64, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="221,96" size="40, 24">周期4</text>
			<button name="btn_MaOK" pos="40,170" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">确定</button>
			<button name="btn_MaCancel" pos="225,170" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">取消</button>
			<button name="btn_MaDefault" pos="40,138" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">默认参数</button>
			<button name="btn_MaSetDefault" pos="225,138" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">设置默认参数</button>
		</window>
	</root>
</SOUI>
 [  <   L A Y O U T   D L G _ L O G I N         0         ﻿<SOUI name="dlg_login" title="登陆" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="400" height="300" margin="1,5,5,5" wndType="normal" appWnd="0">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(220,220,210)" colorText="#000000ff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<icon pos="10,8" src="ICON_LOGO:16" />
				<text pos="29,9" name="txt_title">登陆</text>
			</caption>
			<edit name="login_ID" pos="148,56" size="144, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" maxBuf="16" />
			<text pos="100,56" size="32, 24">账号</text>
			<edit name="login_PassWord" pos="148,90" size="144, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" password="1" maxBuf="16" />
			<text pos="100,90" size="32, 24">密码</text>
			<imgbtn name="btn_login" pos="128,160" size="56, 24" tip="" animate="1">登陆</imgbtn>
			<imgbtn name="btn_quit" pos="220,160" size="56, 24" tip="" animate="1">取消</imgbtn>
			<text name="txt_login" pos="64,224" size="304, 64" multiLines="1" valign="top">欢迎使用</text>
		</window>
	</root>
</SOUI>
   @   L A Y O U T   D L G _ M A C D P A R A       0         <?xml version="1.0"?>
<SOUI name="dlg_macdPara" title="MACD参数" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="365" height="180" margin="1,5,5,5" wndType="normal" toolWindow="0">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(70,70,70)" colorText="#ffffff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<text pos="5,4" name="txt_title">MACD参数</text>
				<imgbtn name="btn_MacdClose" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			</caption>
			<edit name="edit_MacdPara1" pos="60,60" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="16,60" size="40, 24">参数1</text>
			<edit name="edit_MacdPara2" pos="180,60" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="136,60" size="40, 24">参数2</text>
			<edit name="edit_MacdPara3" pos="300,60" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="256,60" size="40, 24">参数3</text>
			<button name="btn_MACDOK" pos="50,140" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">确定</button>
			<button name="btn_MACDCancel" pos="230,140" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">取消</button>
			<button name="btn_MACDDefault" pos="50,100" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">默认参数</button>
			<button name="btn_MACDSetDefault" pos="230,100" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">设置默认参数</button>
		</window>
	</root>
</SOUI>
�  @   L A Y O U T   D L G _ B A N D P A R A       0         <?xml version="1.0"?>
<SOUI name="dlg_BandPara" title="波段优化参数" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="365" height="188" margin="1,5,5,5" wndType="normal" toolWindow="0">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(70,70,70)" colorText="#ffffff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<text pos="5,4" name="txt_title">波段优化参数</text>
				<imgbtn name="btn_BandClose" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			</caption>
			<edit name="edit_BandParaN1" pos="52,50" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="32,50" size="20, 24">N1</text>
			<edit name="edit_BandParaN2" pos="172,50" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="152,50" size="20, 24">N2</text>
			<edit name="edit_BandParaK" pos="284,50" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="264,50" size="20, 24">K</text>
			<edit name="edit_BandParaM1" pos="52,90" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="32,90" size="20, 24">M1</text>
			<edit name="edit_BandParaM2" pos="172,90" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="152,90" size="20, 24">M2</text>
			<edit name="edit_BandParaP" pos="284,90" size="48, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" />
			<text pos="264,90" size="20, 24">P</text>
			<button name="btn_BandOK" pos="50,155" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">确定</button>
			<button name="btn_BandCancel" pos="200,155" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">取消</button>
			<button name="btn_BandDefault" pos="50,122" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">默认参数</button>
			<button name="btn_BandSetDefault" pos="200,122" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">设置默认参数</button>
		</window>
	</root>
</SOUI>
   s  @   L A Y O U T   D L G _ E M A P A R A         0         ﻿<SOUI name="dlg_emaPara" title="EMA参数" bigIcon="ICON_LOGO:32" smallIcon="ICON_LOGO:16" width="365" height="180" margin="1,5,5,5" wndType="normal" toolWindow="0">
	<root cache="1">
		<window pos="0,0,-0,-0" colorBkgnd="rgb(70,70,70)" colorText="#ffffff">
			<caption pos="0,0,-0,30" focusable="1" drawfocusrect="0">
				<text pos="5,4" name="txt_title">EMA参数</text>
				<imgbtn name="btn_EMAClose" skin="_skin.sys.btn.close" pos="-45,0" tip="close" animate="1" />
			</caption>
			<edit name="edit_EMAPara1" pos="92,60" size="72, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" number="1" />
			<text pos="48,60" size="40, 24">参数1</text>
			<edit name="edit_EMAPara2" pos="244,60" size="72, 24" colorBkgnd="#FFFFFF" cueText="" colorText="#000000" number="1" />
			<text pos="200,60" size="40, 24">参数2</text>
			<button name="btn_EMAOK" pos="50,140" size="100, 24" colorText="#000000" skin="_skin.sys.btn.normal">确定</button>
			<button name="btn_EMACancel" pos="220,140" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">取消</button>
			<button name="btn_EMADefault" pos="50,100" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">默认参数</button>
			<button name="btn_EMASetDefault" pos="220,100" size="100,24" colorText="#000000" skin="_skin.sys.btn.normal">设置默认参数</button>
		</window>
	</root>
</SOUI>
 q   4   V A L U E S   S T R I N G       0         ﻿<?xml version="1.0" encoding="utf-8"?>
  <string>
    <title value="RPS"/>
    <ver value="1.0"/>
  </string>
   �   4   V A L U E S   C O L O R         0         ﻿<?xml version="1.0" encoding="utf-8"?>

<color>
  <red value="rgb(255,0,0)"/>
  <green value="rgb(0,255,0)"/>
  <blue value="rgb(0,0,255)"/>
  <white value="rgb(255,255,255)"/>
  <gray value="#808080"/>
</color> 
    f  0   V A L U E S   S K I N       0         ﻿<?xml version="1.0" encoding="utf-8"?>

<skin>
  	<imglist name="skin_traymenu_icons" src="img:traymenu_icons" states="9" />
	<ColorRect name="skin_Black" src="" normal="#000000ff" hover="#000000ff" pushdown="#000000ff" disable="#000000ff" />
	<Button name="skin_ButtonBlack" src="" colorUp="#000000ff" colorDown="#000000ff" colorUpHover="#505050ff" colorDownHover="#505050ff" colorUpPush="#000000ff" colorDownPush="#000000ff" colorUpDisable="#000000ff" colorDownDisable="#000000ff" />
	<scrollbar name="skin_bb_scrollbar" src="img:default.scrollbar" margin="2" />
	<scrollbar name="skin_default_scrollbar_png" src="IMG:default_scrollbar_png" margin="2" />
	<Button name="skin_ButtonClass" src="" colorBorder="#000000ff" colorBorderHover="#000000ff" colorBorderPush="#000000ff" colorBorderDisable="#000000ff" colorUp="#000000ff" colorDown="#000000ff" colorUpHover="#000000ff" colorDownHover="#000000ff" colorUpPush="#000000ff" colorDownPush="#000000ff" colorUpDisable="#7d9ec0ff" colorDownDisable="#7d9ec0ff" />
	<Button name="skin_ButtonPureBlack" src="" colorUp="#000000ff" colorDown="#000000ff" colorUpHover="#000000ff" colorDownHover="#000000ff" colorUpPush="#000000ff" colorDownPush="#000000ff" colorUpDisable="#000000ff" colorDownDisable="#000000ff" colorBorder="#ff0000ff" />
	<Button name="skin_ButtonGrayBlack" src="" colorUp="#000000ff" colorDown="#000000ff" colorUpDisable="#000000ff" colorDownDisable="#000000ff" colorUpHover="#505050ff" colorDownHover="#505050ff" colorUpPush="#505050ff" colorDownPush="#505050ff" />
	<imgframe name="skin_lcex_header_arrow" src="img:skin_lcex_header_arrow" states="2" />
	</skin>
  n  L   I M G   D E F A U L T _ S C R O L L B A R _ P N G       0         �PNG

   IHDR   �   0   t�e   PLTE���>��>�ր��>��pppD�����L�����|ż�   tRNS �Ć�F�   �IDATx^�1��0D����H��R���@]�\ �T�s�8�^[l`�	ʸ�f��=�t)�,�;D �Mҁsm���f� �	���ܤEP�
 cJ��y�9^e A�JTTTS-[�?����I�kst
Iw���A������lQ]-`h�97i4�(��qd�Ww��A�JT���l�����~���,̛~�v��4d�j����sn�"h ��s%��<���2��D%**�闽��#��{�}}yN-M    IEND�B`�  +  P   I M G   S K I N _ L C E X _ H E A D E R _ A R R O W         0         �PNG

   IHDR         �Ao�   sRGB ���   gAMA  ���a   	pHYs  �  ��o�d   �IDAT(Sm��1�5PQqqr'�s��,�DGE�,��� 4�
�1N���IQ�{~a��jVʯ���,ܝ��@��^�S�,�O4�[_���������U��
�@lk$Ϯ`J9���:��5�c�0.H�S�%q�� [���MP� �jN�S2GJJi�NK�����E�Q6��۷��_:���"[�Tg��    IEND�B`� �%      �� ��             (   0   `           $  �  �                                                                                                                   �  � �	!�0�=8�`2�-��0��0��*��0��3�i(
�=� �& �  �                                                                                                                     
 � (�  �/�H4��%r�O�*�
�
�
�
�&�N�$s�/��%
�: �
 �  �                                                                                                                  � 2�  �0��&��   �   �   �   �   �   �   �   ��	0�+��*�R �/�  �                                                                                                              � 3� #�+���   ���.�
9�@�	5�'��   �   ��+��&
�; � �                                                                                                              � 3� #�+��
�
8�&y�6��2�j3�V5�S2�[6�|,��&�   �   �	2�0�� �,�  �                                                                                                          � 2� �4��.��6�r.�5� � �	 � � �0�8/���   ��(��%�&  �  �     �� �� �� 	�� 	�� 	�� �� ��  ��                      ��  �� �� �� 	�� 	�� 	�� 	�� ��  ��       � )�  �2�L2�C � �  �  �          � $�  �3�o-�   �  �!j�*�: � �     �� ��:��V��W��W��I�� ��  ��                  ��  ��  �� ��@��W��W��W��U��1�� ��           �  � � � �  �                   � "�  �
3�g	4�   �   �!g�-�= �  �     �� ��3qC2��7$��9'��{qk��� )�� ��              ��  ��  �� �� 3rg|�8&��7%��7$��aT����M�� 	��                                                �  �  �$�#1��	�   ��$v�+
�0 � �     �� ��Aw*����	��j^���� *�� ��              ��  ��  �� ��8p4!������*��~Z��� %�� ��                                        �  �  � �)	�-6ʅC�   �   �
�2��"��9�  �     �� ��@w+����!��k_���� *�� ��          ��  ��  �� 	�� CYK�� 
��	�� 
��\O���� A�� ��  ��                                   �  � �
'�$6�^.��	6��   � �X�4�W �
%�  �     �� ��@w+����!��k_���� *�� ��          ��  �� �� $�[�+������;(�ᦢ,h�� ��  ��                               �  � �	%�#6�Z0��M��   �   � �B�5Ʌ"� � �         �� ��@w+����!��k_���� *�� ��      ��  ��  �� ��WC2������&��xnr��� -�� ��                           �  �  ��4�O2��P��   �   �   ��T�5Ȇ&�' � �             �� ��@w+����!��k_���� *�� ��      ��  �� �� 2qf}�$��	��	��O?�Ⱥ�N�� ��  ��                       � ' �  �)	�,7Ё$g�
�   �   �   ��-�+��6�d!� � �  �             �� ��@w+����!��k_���� *�� ��  ��  ��  �� ��6n6#������2�ꘓ@x�� �� ��                        �  �  �)
�10��9��   �   ��+�&v�4�.�7 � � �                     �� ��@w+����!��k_���� *�� ��  ��  �� 
�� DVH��
��	��#��j_���� 7�� ��  ��                       �   � "�4��.�   �   � �#�'t�5ʂ/�<� �
 �  �                         �� ��@w+����!��k_���� *��  ��  �� �� -~ui�)������E5�Բ�Z�� ��  ��                        � ! �  �1�J!`� �   ��K�4��1�B� �  � 
 �                                 �� ��@w+����!��k`���� -�� �� �� 0��G}6$������.�S��� $�� ��                            � (�  �6�}'�   �   �E�9�v"�  �	 �  �                                         �� ��@w+���� 
��`S����9o��Dl��Evrg{�6#��	����&��ma���� :�� ��  ��                      4 .
�  �;���   ��/��)	�)
 � �                                                 �� ��@w+����	��'��+��+��*��#����	��.��pe~���L�� WZ  ��  �k� �I�ڧ�vԹԭӺӫ̨̧�g�1�s� -�  �9���   ��.��$�I�  �              �  �  � � � �  �         �� ��@w+����	������������	��	��4!��mb����.l�� 1�� ��  �p� �4�	�������������������Jґ� )�  �8ɇ#�   ��'s�*�J �
	 �  � ( �  �	 � �)	�*/�I"� � �         �� ��@w+����	��'��,��+��+��,��&������!��7$��wms���C�� �q� �3�	�������������������Jӑ� !�  �2�UP�   �   � �*��0�i(
�5*�$*�#*	�)2�>6�g0��<��.�:  � �         �� ��@w+���� 
��aT����7z��@y��?y��>{we�WH��-������)��qg|��� 9�� �/�	�������������������Jӑ�  �  �&�$2��#�   �   ��J� i�(��*��#u�!e�?��-}�4�;  � �         �� ��@w+����!��k`���� 0�� �� �� �� �� ?�~[�2������0�핏Ez�� �4�	�������������������Jӑ�   � &�  �.�<.��(� �   �   �����   �   �  �-|�4�<  � �         �� ��@w+����!��k_���� )��  ��  ��  ��  �� �� 1siy�$��	��	��\N���� 7�Z��������������������Jӑ�      � ��  �
-�62��!a�#�
� �   �   �   �  ���1��1�8  � �         �� ��@w+����!��k_���� *�� ��      ��  ��  �� ��)b4 ������?.�ڸ�P�}]������������������Jӑ�          � % �  �"�0�J6ȅ*��"r�`�Y� a�!q�'��6��1�Y"� � �         �� ��@w+����!��k_���� *�� ��          ��  �� ��L<*������9&�䰯^J������������������Jӑ�                  �  � ��'�#,�20�<2�?0�<,�3(�&� �	 � �   �         �� ��@w+����!��k_���� *�� ��          ��  �� ��P;(������9&�屯`F������������������Jӑ�                             
 �  �  � �	 � � �
 �  �   �                     �� ��@w+����!��k_���� *��  ��  ��  ��  �� �� ��;s/������;(�ಱWÄS����������������w�P� ���	��	��	_	_                                                                     �� ��@w+����!��k`���� ,�� �� �� �� �� %��ZSE�� 
��	����O@���� @�gz������������������������������ԮԿ�����i�ZK#K                                                            �� ��@w+���� ��bU����2k��=g��=g��Bk��Evyoo�D4��"��	����(���za��� "�=�	������������������������������������������������Ȫ�Ԉ^�j<<                                                 �� ��@w+����	��(��.��.��.��,��*��$����	����"��\N����I�� 
�-����������������������������������������������������������v��((H'H                                             �� ��@w+������������������������*��_R����W�� � � �3�	�������������������������������������������������������������v�� 
./                                         �� ��9v7%��+��,��,��,��,��,��,��+��.��9'��WI����X���C�� �� �x� �4�	�����������������|�P���N�	�L�	�g�ٌ�2��s��������������������T�]���                                         �� ��O��7z��>|��>|��>|��>|��>|��>|��>{��Ax��;n��X��;�� �� 	��  ��  �k� �4�	�������������������Jӑ�             �(� ��� ۔�8��������������������) *L(L                                     �� �� �� �� �� �� �� �� �� �� �� �� �� 
�� ��  ��          �k� �4�	�������������������Jӑ�                 �Z� ��� ��g����������������|L|2�V�                                                                                                             �k� �4�	�������������������Jӑ�                     Հ� �~�#�����������������n�a���                                                                                                             �k� �4�	�������������������Jӑ�                     �k� �R�
����������������Ւ�i��                                                                                                             �k� �4�	�������������������Jӑ�                     �k� �S�����������������ە�i��                                                                                                             �k� �4�	�������������������Jӑ�                     Ղ� ��%����������������В�Yݦ�                                                                                                             �k� �4�	�������������������Jӑ�                 �I� ��� ��l����������������ы�)ԍ�                                                                                                             �k� �4�	�������������������Iӑ�             �|� �2���Q���������������������K�
�v�                                                                                                             �k� �4�	����������������Ӝ�v��<��?��?إ�F٥�s�������������������������U��� �F�                                                                                                             �k� �4�	���������������������������������������������������������������H��m�                                                                                                                 �k� �4�	�����������������������������������������������������������p�ي�                                                                                                                     �k� �8�	���������������������������������������������������ߝ�C�>��c�                                                                                                                         �k� �R�ۓ�N��j��h��h��h��h��h��h��hݗ�eߝ�@ք��3��e�                                                                                                                                                                                                                                                         ���� ?  ����   ����   ����   ����   ����   ��|   ���   ���   ���   ���   ��    ��    ��    �� ?   ��    �?��    ?��    p`�    �`?�     `     `      `      p    � x    � |    � ~    � �   � ��      ?�      �      �      �      �    `~�   ��  ����  ����  ����  ����  ����  ���|�  ��� �  ��� �  ��� �  ��� �  ��� �  ������     0   �� I C O N _ L O G O       0            00     �%   �  <   S M E N U   M E N U _ F E N S H I       0         ﻿<menu iconSkin="skin_small_icons" itemHeight="26" iconMargin="4" textMargin="8" maxWidth="190" contextHelpId="1">
  <item id="101">盘口信息</item>
  <sep/>
  <item id="102" check="0">成交量</item>
  <sep/>
  <item id="103" check="0">MACD</item>
  <sep/>
  <item id="104" >RPS</item>
  <sep/>
  <item id="105" >MACD参数</item>
  <sep/>
  <item id="106" check="0">均线</item>
  <sep/>
  <item id="107" check="0">EMA线</item>
  <sep/>
  <item id="108" >EMA参数</item>
  <sep/>
  <item id="109" >申万1级RPS</item>
  <sep/>
  <item id="110" >申万2级RPS</item>
  <sep/>
  <item id="100" >返回</item>
</menu>�  <   S M E N U   M E N U _ K L I N E         0         ﻿<menu iconSkin="skin_small_icons" itemHeight="26" iconMargin="4" textMargin="8" maxWidth="190" contextHelpId="1">
  <item id="201">盘口信息</item>
  <sep/>
  <item id="202" check="0">MA</item>
  <sep/>
  <item id="203" check="0">波段优化</item>
  <sep/>
    <item id="204" check="0">成交量</item>
  <sep/>
  <item id="205" check="0">MACD</item>
  <sep/>
    <item id="206" check="0">RPS</item>
  <sep/>
  <item   text="修改参数"  contextHelpId="2">
    <item id="207">MACD参数</item>
    <item id="208">波段优化参数</item>
    <item id="209">MA参数</item>
  </item>
  <item id="210" >申万1级RPS</item>
  <sep/>
  <item id="211" >申万2级RPS</item>
  <sep/>
  <item id="200" >返回</item>
</menu>