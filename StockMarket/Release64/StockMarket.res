        ��  ��                  �      �� ��     0         �4   V S _ V E R S I O N _ I N F O     ���                                         �   S t r i n g F i l e I n f o   �   0 8 0 4 0 4 b 0   @   F i l e D e s c r i p t i o n     S t o c k M a r k e t   6   F i l e V e r s i o n     1 ,   0 ,   0 ,   1     8   I n t e r n a l N a m e   S t o c k M a r k e t   J   L e g a l C o p y r i g h t   C o p y r i g h t   ( C )   2 0 1 1     H   O r i g i n a l F i l e n a m e   S t o c k M a r k e t . e x e   8   P r o d u c t N a m e     S t o c k M a r k e t   :   P r o d u c t V e r s i o n   1 ,   0 ,   0 ,   1     D    V a r F i l e I n f o     $    T r a n s l a t i o n     ��  8   U I D E F   X M L _ I N I T         0         ﻿<?xml version="1.0" encoding="utf-8"?>
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
</UIDEF>  �  @   L A Y O U T   X M L _ M A I N W N D         0         ﻿<?xml version="1.0"?>
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
     <   L A Y O U T   P A G E _ L I S T         0         ﻿<include>
	<window pos ="5,5,%50,%60" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" >
	  <text pos="5,2" colorText="#FF0000FF">申万1级行业</text>
	  <color_listctrlex name="List_Ind1" cache="1" bkgndBlend="0" pos="5,20,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
	    <headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
		 <items>
		   <item width="50">序号</item>
			<item width="65">证券代码</item>
			<item width="130">证券名称</item>
			<item width="70">最新价</item>
			<item width="70">rps520</item>
			<item width="70">520分数</item>
			<item width="70">520排名</item>
			<item width="70">rps2060</item>
			<item width="70">2060分数</item>
			<item width="70">2060排名</item>
		  </items>
		</headerex>
	  </color_listctrlex>
	<fenshiPic  name="FSPic_Ind1" pos="5,20,-10,-5" visible="0" colorBKgnd="#000000" font="size:12"/>
	</window>
	<window pos ="%50,5,-5,%60" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" >
	  <text pos="5,2" colorText="#FF0000FF">申万2级行业</text>
	  <color_listctrlex name="List_Ind2" cache="1" bkgndBlend="0" pos="5,20,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
	    <headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
		 <items>
		   <item width="50">序号</item>
			<item width="65">证券代码</item>
			<item width="130">证券名称</item>
			<item width="70">最新价</item>
			<item width="70">rps520</item>
			<item width="70">520分数</item>
			<item width="70">520排名</item>
			<item width="70">rps2060</item>
			<item width="70">2060分数</item>
			<item width="70">2060排名</item>
		  </items>
		</headerex>
	  </color_listctrlex>
	<fenshiPic  name="FSPic_Ind2" pos="5,20,-10,-5" visible="0" colorBKgnd="#000000" font="size:12"/>
	</window>
	<window pos ="5,%60,-5,-5" colorBkgnd="#000000ff" colorText="#ffffff" colorBorder="#FF0000" margin="1,1,1,1" >
	  <text pos="5,2" colorText="#FF0000FF">全市场</text>
	  <color_listctrlex name="List_Ind3" cache="1" bkgndBlend="0" pos="5,20,-10,-5" hotTrack="1" itemHeight="20" headerHeight="30" font="underline:0,face:微软雅黑" sbSkin="skin_default_scrollbar_png" colorItemBkgnd="#101010FF" colorItemBkgnd2="#101010FF" colorBkgnd="#000000ff" colorText="#FFFF00" align="right" valign="middle" scrollSpeed="120">
	    <headerex align="left" itemSwapEnable="1" fixWidth="0" sortHeader="1" colorBkgnd="#00000000" colorText="#00ffffff" itemSkin="skin_ButtonPureBlack" sortSkin="skin_lcex_header_arrow">
		 <items>
		   <item width="50">序号</item>
			<item width="65">证券代码</item>
			<item width="130">证券名称</item>
			<item width="70">最新价</item>
			<item width="70">rps520</item>
			<item width="70">520分数</item>
			<item width="70">520排名</item>
			<item width="70">rps2060</item>
			<item width="70">2060分数</item>
			<item width="70">2060排名</item>
		  </items>
		</headerex>
	  </color_listctrlex>
	<fenshiPic  name="FSPic_Ind3" pos="5,20,-10,-5" visible="0" colorBKgnd="#000000" font="size:12"/>
	</window>

</include>  q   4   V A L U E S   S T R I N G       0         ﻿<?xml version="1.0" encoding="utf-8"?>
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
�@lk$Ϯ`J9���:��5�c�0.H�S�%q�� [���MP� �jN�S2GJJi�NK�����E�Q6��۷��_:���"[�Tg��    IEND�B`� (B      �� ��             (   @   �                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  EGGEGGEGGEGGEGGEGGEGGEGGEGGEGGEGGEGGEGG	EGGEGG                                                                                                                                                                                            EGGEGG
EGGEGG"EGG1EGG@EGGMEGGVEGG[EGG\EGG[EGGVEGGNEGGCEGG4EGG%EGGEGGEGGEGG                                                                                                                                                                    EGGEGGEGGEGG"EGG7EGGOZJTryId��Gp��@zն*t��:���0�Zm�{K�qLחpX�aN�eXM|EGGUEGG=EGG(EGGEGG
EGG                                                                                                                                                    EGGEGG	EGGEGG-EGGJEGGjsD^��@t��:������������������������mH���.���8��=�ЁJ�sJہdR�MKIwEGGTEGG6EGGEGGEGG                                                                                                                                    EGGEGGEGGEGG.EGGRWHQ��Eo��.������ ��� ��� ��� ��� ��� ��� ��� ��� ���'w���0���0���/���-���/���4�߀?�rI�mZM�EGG_EGG9EGGEGG
EGG                                                                                                                        EGGEGGEGG%EGGJTIP��Ix��"��� ��� ��� ��������������������������� ������pE���.���2���2���2���1���/���.��8��V�p^O�EGGYEGG/EGGEGGEGG            :AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA                                            EGGEGGEGGEGG5EGGg�Oq��$��� ��� ��� ������������������������������������ ���N]���,���2���2���2���2���2���1���0���+��8��uP�RNJ~EGGCEGGEGG
EGG:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA                                EGGEGGEGGEGGC\LV��G������ ��� ������������������������������������������ ���4n���-���2���2���2���2���2���2���2���2���0���-�ۀB�zfX�EGGVEGG'DGG>CC:AA:AA	:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA	:AA:AA                            EGGEGGEGGEGGMtLa��"��� ��� ������������������������������������������������ ���&y���2���1���2���2���2���2���2���2���2���2���2���-��9��qX�EGGdEGG.AEE<BB:AA:AA�4qr�7gH:AA:AA:AA:AA�9a5�3t��9a4:AA:AA:AA                            EGGEGGEGGP|Ll����� ��� �� ������������������������������������������������ ���~���5���1���2���2���2���2���2���2���2���2���2���2���/���.��zW�EGGeCFF.>CC:AA�5ox����(�8cP:AA':AA':AA(�4r�����4s�:AA :AA:AA                        EGGEGGEGGLWPz�i7��]'��x'��������� ��������������������������������������������� �����~5���0���2���2���2���2���2���2���2���2���2���2���2���0���-��{Y�DFFO?DD)�5ox�5o��4y��3y��5o��8ef�;YL�5o��3v��6x��3v��4q�}<T':AA                    EGGEGGEGGDYUr�S1��I&��M,��H,��F.��t(������ �� ��������������������������������������� �����~6���0���2���2���2���2���2���2���2���2���2���2���2���2���,��B�ROMl�8h��j]��sX�ŘD�ЅN��sX��Wg��Ep��sX��sX�;��sX��sX��4r�:AA                EGGEGGEGG7SS`�[=��M&��Q+��Q+��Q+��Q+��G-��Q.��������� ������������������������������������ ���~���5���1���2���2���2���2���2���2���2���2���2���2���2���2���.�݃E�JJHl�<^[�Nk��Wg�ЅN��sX��Wg��Ep��;u��Wg��Wg���@��Wg��Wg��6kX:AA                EGG	EGG(EGGlgT��I!��Q+��Q+��Q+��Q+��Q+��Q+��L+��G/��������� ��������������������������������� ���'x���2���1���2���2���2���2���2���2���2���2���2���2���2���2��*��}Y�DFFW?DD*�8eL�5o��sX��Wg�7ee�;ZJn=P=�7fd�3v�ŘD��3v��7h]:AA:AA            EGGEGGEGGRgZ��H ��P*��Q+��Q+��Q+��Q+��Q+��Q+��Q+��O+��D.��������������������������������������� ���8l���-���2���2���2���2���2���2���2���2���2���2���2���2���.��9�jaY�DFF:>CC:AA�6jY�Wg��Ep�9^?:AA#:AA#:AA$�5ox�j]��5ov:AA:AA:AA
            EGGEGG7ZYg�Z8��L'��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��O+��G.������ ������������������������������ ���TZ���,���2���2���2���2���2���2���2���2���2���2���2���2��~)��}`�EGGVDFF=CC:AA:AA�5mS�8c6:AA:AA:AA:AA:AA�4rp:AA:AA:AA:AA        EGGEGGEGG_fS��J"��R+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��N+��Q.������ ������������������������ ������uA���/���2���2���2���2���2���2���2���2���2���2���2��)�ͅQ�JKJsEGG+CFF:AA:AA:AA	:AA:AA:AA	:AA	:AA	:AA	:AA
:AA:AA	:AA:AA:AA        EGGEGG:YUr�Q-��O)��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��H-��q)������ ��������������� ������ ���2q���/���2���2���2���2���2���2���2���2���2���2���2���,��D�e\T�EGG:EGGEGG:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA    EGGEGGEGG\jY��I"��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��P+��E/������ ������ ��� ������������|��u>���*���0���2���2���2���2���2���2���2���2���2���,��@�n_S�EGGEEGGEGG            :AA:AA:AAEKKEKKEKKEKKEKKEKKEKK                EGGEGG0TT`�X8��N(��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��R+��I,��t)������������)w�6e�v5Y�k0P�nQKǉfD֮sI��;���0���/���2���2���1���1���1���1��~)�ޅG�m^R�EGGIEGGIKK	PTTPTTPTTPTTPTTPTTPTTPTT
PTTPTTPTTPTT
PTTPTTPTT            EGGEGGIkb��K#��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��O+��W*������1v�c>R�CB@�c^Y�tle�yqi�e_Z�CB@�577~NHB��lN��~8��y*��x/��u.��t.��u,��v+��yN�[RL�CEEIEHH!LOOPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTT        EGGEGGEGGcfQ��K#��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��K(��]0���Fp�9;:�rkd�~tk�YOH�B:3�;3,�YOH�~tk�phb�*,,a577x[QH��m?�oE��rS��tX��qU�x`N�577�466R799-AEENRRPTT"PTT(PTT+PTT)PTT&PTT$PTT%PTT*PTT1PTT7S+�hTƣR5�3PTTPTTPTTPTT    EGGEGG-PP\�Y9��O(��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��P)��M(��e\��799~vng�YOH�
�
�
�
�
��oe]�nf`� ""gjd^��}u�oe]�YOH�QHA�YOH�wld��zr�le_�+..6=@@,LPP0S)�uT��UƺT��R8�WPTT<PTT=T��U��V��V��V��V߯R5�2PTTPTTPTT    EGG
EGG=_W��R+��P*��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��L%��bS��>@@v]YT|oe]�
�
�3+%�YOH�JA:�$�
���{r�a[V�wld�$�
�
�,$�
�
�,$��{r�LJGl;:EJS��V��V��U��V��V��T��R=}iV��U��U��U��U��V�S)�JPTTPTTPTT    EGGEGGKf[��M%��Q+��Q+��Q+��Q*��Q)��R)��R)��R)��Q)��Q*��Q+��Q+��Q+��Q+��M&��]>��LLR�9;;Ec]Xhoe]��oe]�}tl�^XS�ha[уyq�,$�
�QHA��{r�$��oe]���y��xp�{r�B:3�
�JA:�tle�H��S��U��U��U��U��U��V��T ��V�U��U��U��V�T��PTT/PTTPTTPTT    EGGEGGXhW��J#��Q(��S$��T%��T*��T0��Q2��N3��P2��T0��S)��T%��S%��Q(��Q+��M'��aS��DFFY577%VQM>~tk�~tk�rjc�Yg�WRM�YOH�
�3+%�h^V�
�QHA�jc]�]UFC?��{s�$���w�F��P��U��VߩVޣV߯U��U��V߯R7��U��U��U��VޣPTTOPTT0PTTPTT        EGGEGGabH��O ��Q6��HT��>m��7}��6���1���-���2���5���8{��@j��IP��R3��R"��V3��XWk�DFF;/11QMHAEB?RGTiQLH�wog�B:3�
�;3,�QHA�
�wld�CA=�@4Pnf`�JA:�
��{r�J��O	��U��S)��PTT�S0��V��U��V��R:��U��U��U��VޣPTTNPTT/PTTPTT        EGGEGGgU8��@c��/���)���'���(���)���*���*���*���(���(���(���(���1���BW��b?��GHK{DFF)%''
(Ab[V�zqiӍ�y�h^V�3+%�
�
�`WO�QHA�
��{r�++)`.>kd]�QHA�
�oe]�W+��O	��U��R7��PTT~R<~�V��U��U��S+��U��U��U��VޣPTTIPTT-PTTPTT        EGGEGGjP|��!���)���+���+���+���+���+���+���+���+���+���+���+���)���$���Us��EGGjDFF!
VQLgwng�`WO�;3,�
�
�
�
�JA:��{r�QHA�
�oe]�MIE�C6Rnf`�JA:�
��{r�I��O	��U��S+��PTTsR;�V��U��U��S+��U��U��U��VޣPTTGPTT+PTTPTT        EGG EGGjD���$���+���+���+���+���+���+���+���+���+���+���+���+���+���%���D���EGGgACC"2xoh�B:3�
�
�
�
�;3,�oe]�nf`�FC?�oe]�
�JA:�oga�  fXEB?��{r�$�
���y�F��P��U��T$��PTTfR9�uV��U��U��S+��U��U��U��VޣPTTLPTT/PTTPTT        EGGEGGiD���&���+���+���+���+���+���+���+���+���+���+���+���+���+���%���J���EGGp@BB'NJEhwld�
�
�3+%�h^V���y�zqi�SOJ�X^�yq�,$�
�YOH���y���y��{r�B:3�
�B:3�vjn�J
��R��U��T"��PTT\R7�mV��U��U��S*��S+��U��U��VޣPTTQPTT3PTTPTTPTT    EGGEGGd>���)���+���+���+���+���+���+���+���+���+���+���+���+���+���(���9���KRV�@BB5&qib�QHA�
�$��xp�NJE�t[MVQMw=;8be^X�wld�$�
�
�
�
�
�3+%��{r�VGf�N��S��U��T �{PTTQPTTOV��U��U��T$��U��U��U��V��S5�nPTT5PTTPTTPTT    EGGEGG\N���$���+���+���+���+���+���+���+���+���+���+���+���+���+���*���)���[q��ACCK%''.ld^�YOH�
�,$�|sk��x itle�oe]�~tk�?<9_nf`��~u�oe]�`WO�QHA�`WO�wld��xp�e`Z�?&pfQ��T��U��T�sPTTAPTT@VޯU��U��UƥU��U��U��U��V��S1�MPTTPTT	PTT    EGGEGGRS��%���+���+���+���+���+���+���+���+���+���+���+���+���+���+���"���Y���CEEo133;A?<^~tk�
�
�h^V��yq�zqj�v�YOH�
�oe]�[VPo48ZUPrmf_�umf�ha[�QMIc ""8/224C/mATرU��V߯R;�>PTT.PTT-U�tV�V��T��U��U��U��U��U��U�xPTTPTT
PTT    EGGEGGCUt��&���+���+���+���+���+���+���+���+���+���+���+���+���+���+���)���,���Yjw�<>>Z$&&Eule�QHA�
�
�$�3+%��
�
�oe]�`ZUh!##(-//./11@.00I+--D'))5"$$%'**9<<HLLQ3�1U�dR5�2PTTPTTPTTPTTS(�HT&�WR;�NT�}UˏVՙVߩV�S)�@PTTPTTPTT    EGGEGG4Pan�0���)���+���+���+���+���+���+���+���+���+���+���+���+���-���.���$���B���HSW�688[;:8avmf�B:3�
�
�
�
�$�`WO�ypi�$&&38::8BDDT`fF�cjD�ILEuACCSACC1BDDHKKOSSPTTPTTPTTPTTPTTPTTPTTPTTPTTPTT&PTT/PTT4PTT4QEn5S,�=PTTPTTPTTPTT    EGGEGG%GKLr?���&���+���+���+���+���+���+���+���+���+���+���*���.���(��������� ���-���FZ_�688hROK�vng�wld�QHA�YOH�oe]��}t�nf`�'))G8::OBDDu��JҚ�������)�L�RTK�EGGHFHH$ILLNQQ	PTT	PTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTT	PTTPTT    EGGEGGEGGVZ���"���+���+���+���+���+���+���+���+���+���-���-��������� ��� ��� ��� ���$���Kt}�;==�022qKHE�mf`�g`[�TPL�(**_133`;==oY\H���E宻������������	���-�ptS�EGG\EGG+FHHKNNKNNIKKGJJHJJKNNPTTPTTPTTPTTPTT
PTT
PTT
PTT	PTTPTTPTT            EGGEGG<^o~�)���*���+���+���+���+���+���+���+���-���)������ ��� ��� ��� ��� ��� ��� ������.���>x��BSV�:@@�9;;�;=<�GIB�inP���M��)���������������������	�����I�EGGhEGG.EGGEGGEGGEGGEGGEGGEGGEGG                                                EGGEGG%GJJpI���$���+���+���+���+���+���+���-���)������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ���������K���J���8��*���������������������������������������C�EGGkEGG.EGGQTP��VZhnHVEGGHEGG8EGG"EGGEGGEGG                                    EGGEGGEGGJ]z��"���+���+���+���+���+���,���)������ ������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��������,���	���������������������������������������������	���K�EGGgEGG.EGG'��pz��+霧@�koQ�EGGhEGGGEGG+EGGEGG
EGG                                EGGEGG+JOR|?���&���+���+���+���+���*������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���m҈���������������������������������������������������������Y�EGG[EGG4EGGI��W������#���:�|�F�NQHEGGVEGG7EGG!EGGEGGEGG                    EGGEGGEGGK_y��#���+���+���*���-������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��������A����������������������������������������������������������orZ�EGGLEGG>JLKp��H������������/�@�ejL�EGGgEGGFEGG*EGGEGG
EGG                EGGEGG'FHHnP���!���+���-���!������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���'�����������������������������������������������������������������<�MNMvEGG:EGGFdgT���%���������������"���A�{�L�JMGzEGGOEGG,EGGEGG            EGGEGGEGG=\fm�5���%���,������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���Dε����������������������������������������������������������������}�[�EGGIEGG+EGGS��U������������������������-�D�rwRwEGG(EGG
                EGGEGGEGGR`z��%���!������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���YѠ������������������������������������������������������������������@�GIH^EGG"EGG(EGGi��O���������������������������E�EGG:EGG                    EGGEGG%EGGbT}����� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���hя������������������������������������������������������������������%�TVOkEGGEGGEGG6TWO���3������������������������l�EGG6EGG                    EGGEGGEGG-EGGmD��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���kЇ������������������������������������������������������������������8�MOLYEGGEGGEGGEGGHtxY���������������������P�EGGgEGG#EGG                        EGGEGGEGG1FJLs7������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���hю���������������������������������������������������������������1�dhN�EGG7EGG    EGGEGG EGG]��W���������������&�fhZ�EGG;EGGEGG                            EGGEGGEGG2EGGnB������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���[П������������������������������������������������������������;�bfN�EGGDEGGEGG    EGGEGG
EGG.HJIu��I������������W�EGGXEGGEGG                                    EGGEGGEGG.EGGdI������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ���Gг���������������������������������������������������������Q�TWL�EGG@EGGEGGEGGEGGEGGEGGEGGEGG=ceU���%������<�ORM}EGG/EGG
                                            EGGEGGEGG&EGGTPnt�4������ ��� ��� ��� ��� ��� ��� ��� ��� ���)�����������������������������������������������������.���P�EGGgEGG4EGGEGGEGGEGGEGGEGGEGGEGGEGGEGGO��\������f�EGGFEGGEGG                                                EGGEGG	EGGEGG>EGGoU���%��� ��� ��� ��� ��� ��� ��� ��� ��������<������������������������������������������#���O�[_L�EGGNEGG&EGGEGGEGGEGG7XRvQ}o�eJKNEGG    EGGEGG"HJIB��k�MOL@EGGEGG                                                            EGGEGGEGG(EGGLGTWG���/������ ��� ��� ��� ��� ��� ���rс���	���������������������������������(���L�gkO�EGG\EGG4EGGEGGEGGEGG+EGGW\S��\=��rf��EGG$EGG    EGGEGG
EGGEGGEGGEGG
EGG                                                                EGGEGGEGGEGG,EGGLFIJrGz��:������	������ ��� ��������%������������������������,���:耈E�X[J�EGGYEGG7EGGEGGEGGEGGEGG;EGGp`Q��O*��_A��JLO^EGG!EGG        EGGEGGEGGEGGEGG                                                                            EGGEGGEGGEGG&EGG>EGGXEMOvHnw�=���>���=���$���A���+﮺�4?蘡>↎A�tyP�TXK�EGGaEGGFEGG.EGGEGGEGGEGGEGGEGGCPLh�`I��O(��H ��k^��EGGNEGGEGG                                                                                                                EGGEGGEGGEGGEGG&EGG7EGGFEGGSEGG^EGGeCKLlFHGkIKElEGGfEGG_EGGVEGGIEGG;EGG+EGGEGGEGGEGG        EGG	EGG(j]��S2��M&��N(��X5��XWh�EGG5EGG
                                                                                                                                EGGEGGEGGEGGEGGEGGEGGEGGEGG EGGEGGEGGEGGEGGEGG
EGGEGG                    EGG
GHK+dN��S1��L&��H!��kX��EGGbEGGEGG                                                                                                                                                                                                                        EGGEGGEGGGXSu�_G��R+��d\��EGG>EGG                                                                                                                                                                                                                            EGGEGG
EGGEGGAOLdjgR��WToSEGG!EGG                                                                                                                                                                                                                                    EGGEGGEGGEGG&EGGEGGEGG                                                                                                                                                                                                                                        EGGEGGEGGEGGEGG                                                        ������������������� ������ �����   ����   ����   ����   ���      �       �       �       �       �       �       �       �       �       �       �       �     ��      �                                                                                                                             �     ��      ��      ?�      �      �       �       �       �       �       �       �      �      ��     ��    ��    ��    ��    ����  ����� �����������������������������?��%      �� ��             (   0   `                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          EGGEGGEGGEGG+EGG8EGG?EGG@EGG=EGG6EGG*EGGEGG
EGG                                                                                                                                EGGEGG
EGGEGG;nI^p�Iq��?y��-|��*���.���i\�}F�tSԏjQ�iXJkEGG:EGGEGG	EGG                                                                                                            EGGEGGEGG(^IUc�=k��.{��!������ ��� ��� ��� ���"w���3���-���7�߀?�zN�hL�[QJ_EGG)EGGEGG                                                                                                EGGEGG VIQ^�Et��"���	��� ��� ��� ��� ������ ��� ��� ���]Q���,���/���.���-���1�߀@�sT�\RJaEGG EGG            :AA:AA:AA:AA:AA:AA:AA:AA:AA:AA                                    EGGEGG:�Hl��)������ ��� ������������������������ ���;i���-���2���2���2���1���/���/�ڀB��iS�EGG:EGGEGG:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA                        EGGEGGPHMS�D|����� ��� ������������������������������ ���%w���1���2���2���2���2���2���2���.���4��xV�[RKZDGG<BB:AA:AA�4sm:AA:AA:AA:AA�4sm:AA:AA:AA                        EGGWK_e�5��� ��� ~����������������������������������� ������6���1���2���2���2���2���2���2���/���,��M�]SLdAEE:AA�4qs����5pu:AA#:AA#�5pu����4qs:AA:AA                    EGGLLW[bF��]%��v$��������� ������������������������������ ������|6���0���2���2���2���2���2���2���2���0���*��Q�FHGD�8b9�4q�����5o��5m}�7fc�5o�����5p��6lV:AA                EGG
EGGHcP��H#��L+��G-��H/���#���	��� ��������������������������� ������7���0���2���2���2���2���2���2���2���2���.��=�yNWt�Ep��sX�ŘD��sX��j]��Wg��sX�ŘD��sX��Wg��5pN            EGGEGG4dX��L%��P*��R+��R+��O+��@/��h)������ ������������������������ ��� |���4���1���2���2���2���2���2���2���2���3��*�ŀO�CFFE�8a8�4q��sX��5p��5m{�7fb�5p��sX��4q��6lU:AA            EGGe_��P+��L&��Q+��Q+��Q+��Q+��Q+��C.��c)���
��� ��������������������� ���1p���/���2���2���2���2���2���2���2���3���/���/��s`�BFF#:AA�8b7�Ep�9`::AA:AA�9`:�Ep�8b7:AA:AA
        EGGLLRPhR��I!��Q+��Q+��Q+��Q+��Q+��Q+��R+��F-��s'������ ������������������ ���O[���,���2���2���2���2���2���2���2���1��~'���\�EGG?@DD:AA	:AA�7h/:AA:AA:AA:AA�7h/:AA:AA	:AA        EGGaW��L%��P)��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��B/������ ��� ��� ��� ��� ��� ������t=���.���2���2���2���2���2���2���1��)�҃K�b\VfEGG:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA    EGGFHIIdN��I"��R,��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��J,��U-������ �����������}��X[��2���.���/���2���2���2���2���0��*�؂E�k^SzEGGEGG            EKKPTTPTTPTTPTTLPPPTT        EGGEGGc^�S/��P*��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��E/������{��1l�sJ\�sbb�qda�aVL�rXA��oJ��:��}+��~0��|0��|-��|-��N�eYOwGII NRRPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTTPTT    EGGVUiDeX��L#��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��P*��C(���;��uX`�~un�oe]�QHA�QHA�wld�xoh�VRN�`QD��rJ�vY��xa��sX��l[�MHC|799.FJJPTTPTT'PTT)PTT(PTT&PTT(PTT0S-�RT��UʎR7�(PTTPTT    EGGa\��`I��L%��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��Q+��J$��WD��WKX��{r�3+%�
�
�
�
�B:3�tle�rkd�~tk�QHA�B:3�QHA�YOH��{r�le_�D3e>UƥU��V	��UҿT&�pS*�hU��V��V��U��U�hPTTPTT    EGG"dW��[:��N'��Q+��Q*��Q)��R)��R)��Q)��Q+��Q+��Q+��O(��S0��]Z}�c^Yooe]�
�QHA�wld�wld�3+%�
�h^V�oe]�
�$�JA:�;3,�
�3+%��wo�J��T��U��U��U��V��UŗU��U��U��U��T�nPTTPTT    EGG1dR��V1��P!��T%��S+��R2��O3��P3��S0��S)��T%��S&��L#��cO��BDDGg`ZIwld�oe]�tle�A>;�OKG�~tk�
�JA:�3+%�$��{r�pha�yph�oe]�
�oe]�Z*��S��U��V�U��U��V߯U��U��U��UǓQCrDPTTPTT    EGG;\=��U0��GL��=l��5��3���.���0���3���8y��@c��L?��R%��cZ��ACC"`ZTN[UPpHEAsjc\�vn�YOH�
�QHA��JA:�oga�:L��y�
�`WO�`9��R��UɝR=|�V߾U��V��S,��U��U��UȒPTT9PTTPTT    EGGANX��8y��(���'���(���)���*���*���(���(���'���(���Gg��ZQw�@BB^XS[xoh�wld�QHA�,$�
��wld�
�YOH�f_Y�+?��y�
�YOH�eA��R��UÙPTT~VߺU��U��S,��U��U��UɑPTT5PTTPTT    EGGCC���*���(���+���+���+���+���+���+���+���+���%���6���J\p�:<<zqi�B:3�
�
�
�;3,�~tk�zqj�3+%�B:3�skd�RMI�a[U�~tk�
�oe]�X*��S��UƕPTTkVߺU��U��S,��U��U��UɑPTT8PTTPTT    EGG@?���-���*���+���+���+���+���+���+���+���+���'���4���Sgv�8::b\Vdoe]�
�$�h^V���y�{rj�UPK�_YS�`WO�
�QHA�oe]�h^V�,$�$��{r�O��T��UɑPTTZV	ٟU��U��T(��U��U��UǓPTT<PTTPTT    EGG:C���.���)���+���+���+���+���+���+���+���+���*���'���Zw��=??(oha�`WO�
�YOH�[VQ�qd^X��{r��{r�vmf�`WO�3+%�3+%�3+%�;3,�wld�nY{�P��U��U͌PTTIU̎U��U��V��U��U��V��U�yPTTPTT    EGG/N���2���(���+���+���+���+���+���+���+���+���,���"���T���CEEL.��w��;3,��~v��vn�wld�,$�oe]�ZTOqmf_��zr݄zr�yqހwo�e_ZuC"�UTټV��T �\PTT1T"�_V��V�V��U��U��U��VޣPTTPTT    EGG Sx��9���'���+���+���+���+���+���+���+���+���+���+���0���\u��355;vmf�YOH�
��,$�
�
�wld�\VQT-//(355@133D*,,0#%%144EHHS �?U�PPTTPTTPTTT�XS.�TS'�iUˏVޣV�U�pPTTPTT    EGGXn~yG���%���+���+���+���+���+���+���+���+���.���)������*���Imt�9:9Jypi�~tk�B:3�,$�B:3�~tk�|sk�466:hmG���8ނ�<�mrK�EGG>HJJOSS
PTTPTTPTTPTTPTTPTTPTTPTT%PTT+PTT+S,�=PTTPTTPTT    EGGT_f>Z���#���+���+���+���+���+���+���-���-��������� ��� ������H~��8::bUQMyypiÄzr�yqi�VRNw9;;]v}J���+���
���������E�Z]KpFHH JMMPTTMQQMQQPTTPTTPTTPTTPTTPTTPTTPTT
PTTPTT    EGGEGG[jw�0���)���+���+���+���+���.���*������ ��� ��� ��� ��� ������(���B���Ago�Z`C�dhD�~�NǦ�A���������������������-�dhG�EGG$EGG`dZdjL#EGGEGGEGG        PTTPTTPTT                EGGEGGBP���!���,���+���+���-���*������ ��� ��� ��� ��� ��� ��� ��� ��� ���f͓���(��� ���������������������������
���$�ilJ�EGG"EGG��\���;�WZKgEGG1EGGEGG                                EGG[r��)���(���+���,���+������ ��� ��� ��� ��� ��� ��� ��� ��� ���&��������
������������������������������������,�hkV�EGG%��as��4���*���>�hmJ�EGGAEGGEGG	EGG                    EGGEGGDT������+���-������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ���]љ���
������������������������������������������A�NPNQEGG1��g���������#���9�{�H�X\HbEGG.EGGEGGEGG            EGGbqz�8���&���$������ ��� ��� ��� ��� ��� ��� ��� ��� ��������j���	������������������������������������������	���c�EGG(EGG=��L���������������5���A�lqI�[_LBEGG            EGGEGG.a��'������ ��� ��� ��� ��� ��� ��� ��� ��� ��� ���
�����P���
���������������������������������������������9�JLJCEGGRUK]��=������������������)���I�EGG"                EGGEGGAK}����� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��������D������������������������������������������������"�]_UYEGGEGGru[���%���������������9�h�EGG                    EGGGMOO>��� ��� ��� ��� ��� ��� ��� ��� ��� ��� ��������B������������������������������������������������D�EGG3EGGEGGEGG/��T���������������`�EGG0EGG                        EGGFNPO6������ ��� ��� ��� ��� ��� ��� ��� ��������M���������������������������������������������H�MPIPEGG        EGG	EGGI��R���������J�EGGFEGG                                EGGEGGBE������ ��� ��� ��� ��� ��� ��� ��������`���	������������������������������������%���Q�EGGCEGGEGGEGGEGGEGGEGGbdVy��&���#�ps[�EGG                                        EGG	EGG.Jrz�4������ ��� ��� ��� ��� ��� ���jщ���	���������������������������������A�puM�EGG/EGG	EGGEGG]W~8lh�EGGEGGEGG&��b���b�EGG1EGG                                            EGGEGGFMOHK��4������ ��� ��� ��� ���7����������������������������� ���>�|�N�PSIMEGGEGGEGGEGG=^P��kXʾ_]m4EGG    EGGLNKPRMEGG
                                                        EGGEGGEGGDKt}�8���&��������������e������������!���+���;퍘=�ntI�EGGEEGGEGGEGGEGGIHW[YG��N(��pd��EGG$EGG                                                                                    EGGEGGEGG+EKLIDksxB~��9���3���x�<ǅ�3Ā�B�w}A�fkIwIKHIEGG+EGGEGG        EGG]R��[A��J#��[<��WVfsEGG                                                                                                    EGGEGGEGGEGGEGG"EGG#EGG"EGGEGGEGGEGG                    EGGdN��L%��G��jZ��EGG6EGG                                                                                                                                                                    EGGIITG^S��Z=��aZ��EGG                                                                                                                                                                            EGG
OMa?dQ��]X|FEGG	                                                                                                                                                                            EGGEGG
EGGEGGEGG                                        ������  ������  �� ��  �� ��  ��  �  ��  8  �       �       �       �       �       �       �       �       �       �                                                                                                            �  �    ?  �      �       �       �       �       �       �     �      �      ��   �  ��   �  �� �  �� �  �����  ������  ������  �      �� ��             (       @                                                              EGGEGGEGG
EGGEGGEGGEGGEGGEGGEGGEGGEGG                                                                        EGGEGGEGGEGG#EGG8EGGLEGG\EGGeEGGgEGGbEGGVEGGEEGG1EGGEGGEGGEGG    :AA:AA:AA:AA:AA:AA:AA:AA                EGGEGGEGGEGG(EGGGEGGkyC`��Bw��,������0|��zI��{K�jL�eVL�EGG`EGG=EGG DGG@DD:AA:AA:AA	:AA	:AA	:AA	:AA:AA            EGGEGG
EGGEGG@FGHq�Cn��$������ ��� ��� ��� ���H[���)���0��8��xF�cN�EGG`EGG5CFF?DD:AA:AA:AA:AA:AA:AA:AA        EGGEGG
EGG$EGGS|Ke��'������ ��� ��� ��������� ���$v���0���1���0���.���2��yM�[SL�DFFFBEE#�=V$�4t~}<T'}<T'�4t~�;X :AA    EGGEGG
EGG%EGG[�Fw����� ~�� ������������������ ������6���0���2���2���1���-�ށB�s^P�CFFC�6o�����4r��4r�����4t~�:[    EGGEGGEGGU`U��[(��u"������ ������������������ ������}7���0���2���2���2���2���+���D�E_��g[�ŘD��j]��Wg�ŘD��sX��3uzEGGEGGEGGC]Y~�R/��K(��G-��I.��������� ������������ ������6���0���2���2���2���2���-��A�OLIf�8i}�Wg��5pu�6me�Wg��4t}�:\EGG	EGG,NOV{]?��K$��Q+��Q+��N+��D/������ ������������ ���'t���0���2���2���2���2���1���*��z_�DGGPBFF%�6lU=DD=DD�6lU<CC<BBEGGEGGOdV��K$��Q+��Q+��Q+��Q+��N+��I.������}��~��{�����HR���+��1��1��1��1��{*��R�GIIsFII9FKK"EKKEKK DJJCIIBHHAGGEGG)NNU{]>��M&��Q+��Q+��Q+��Q+��Q+��G-��n#���{��s��@n�Rf�j@��r0��q*��u.��u.��u,��tF�PNJ�FHHMJNN0MQQ(LQQ)LQQ(LPP#JOOINNINNEGG@aZ��O(��Q+��Q+��Q+��Q+��Q+��Q+��K)��T*���_s�{r�h^V�`WO��{r�ncZ�kQ��{i��|j��ue�XUQ�>@@`I1{VR-�SQHiAR>{GT�nVդT�OOSSPTTEGGTcS��L$��Q*��R)��R)��Q)��Q*��Q+��I&��cU��~tk��$��$��{r�h^V�3+%�3+%�B:3��{r�M!��Q��U��V��V��U��U��V�S)�8PTTEGGc`B��O ��R,��Q2��O3��R/��S'��O ��[@��SPO�wld�YOH��{r�~v�$�RIB�
�oe]�~tk�3+%�YOH�g@��N	��S��U��V�U��U��U�}PTT'PTTEGGkSD��:h��3���0���.���1���7w��AJ��`O��688yKHE{phbƍ�y�`WO��NE>�3+%�zqj�VRM�`WO�;3,�w]��L	��Q��U��V��U��U��T�uPTT+PTTEGGo9���%���)���*���*���)���(���'���K^��244qumf�JA:�
�
�JA:�j`X�,$��vn�kd^�YOH�JA:�oO��L	��Q��U��V��U��U��T�uPTT,PTTEGGm6���)���+���+���+���+���+���&���Lr��XUQ�oe]�
�`WO���y�jc\ցwo�,$�;3,�JA:��oe]�[)��N��Q��U��V��V��U��U��PTT*PTTEGGe?���'���+���+���+���+���+���$���I���HHF�~tk�
�wld���y�JA:�wld��zr�oe]�oe]�~tk�t^�M	��QѸQ'�nV��V��U��U��V��T&�EPTTEGGXJ���%���+���+���+���+���+���,���+���Hgt�ske�QHA�
�
�$�~tk�BB@�`^O�[XR�LJH|>-a]L�|L-�GOSS2T#�aU�qVۜVߩV߯S0�+PTTEGGEYz��$���+���+���+���,���-������������Eox�qjd�~tk�oe]��|t�lg_�|�9������)�inH�BDDnDFFCILL-LOO-LPP2MQQ/ORR)QAu*S0�+PTTPTT
EGG/R[b�6���'���+���,���,������ ��� ���������"���I{��twO恇>奰.�������������}�<�FHHrHLKHrzMWHJJHHJJGIKK8JMM'LOOMQQPTTEGGEGGYS���!���,���,������ ��� ��� ��� ���������s�j���������������������������I�EGG{��`���4�~�E�JLHvFHHWFII9FIIHJJEGGEGG5V`g�7���(������ ��� ��� ��� ��� ��� ��������(���������������������������
�ux`�FHH���C������-�B�aeH�EGGIEGGEGGEGGEGGOb}��!������ ��� ��� ��� ��� ��� ���1�����������������������������������9�HJI�]`P���'������������B�EGG4    EGGEGG%EGGcD��� ��� ��� ��� ��� ��� ��� ���=ϼ���������������������������������#�TVOwEGGh�X������������[�EGG:    EGGEGGEGG-EGGj5������ ��� ��� ��� ��� ���9��������������������������������$�|�M�EGGLEGG;EGGh��U���	���4�TVPzEGG,        EGGEGGEGG.EGGdE������ ��� ��� ��� ���%�����������������������������3�ouL�EGGeEGG=EGG)EGG<TVPm��7⒘`�EGGDEGG            EGGEGGEGG&EGGQHel�<������ ��� ��������=������������������.�I�KNH�EGGmEGG_aO��xp�TEGG&EGG,nq]HOQM4EGGEGG                EGGEGG	EGGEGG6EGGZIbg�;���.��� ���O��������)�2옢D�xC�NQHxEGGTEGGLNKakV;��cE��YYgJEGGEGGEGGEGGEGGEGG                        EGGEGGEGGEGG0EGGFEGGYEHHhBQTtMRFvKMFpEGGcEGGSEGG>EGG(EGGEGG3cIջE��pa��EGG<EGGEGGEGGEGG                                        EGGEGGEGGEGGEGGEGG#EGG$EGG"EGGEGGEGGEGGEGG	EGG!VNzWfOͭ`Y�bEGG'EGG
                                                                        EGGEGGEGG                EGGEGGEGG!EGG*EGG EGGEGG                    � ��  �   �   �   �   �                                                                       �   �   �   �   �   �  �  ���h      �� ��             (                                                 EGGEGGEGG9EGGJEGGLEGG@EGG'EGGEGG                    EGGEGGJGJJ�<i��(|�����Ah�Ӂ?�nH�dUJnDGG'@DD:AA	:AA	:AAEGGEGG]Jgx�)|�� ��� ��� ���~���2���-��;��lR�nAR;�4sn�4r^�6i-EGGUTdkZ;��x������ ��� ������}8���0���,��;��:o�����"���,}�NOWGfU��H"��C.��f(���������6r���.���0��+��}Z�sDVc�1y��1y�;e5d[��U0��P&��R*��?,���=��wld�wld��v_��zf��sj�`@��S��j!��f��O)�0ZA��Q+��O3��N+��^A��wld�;3,�3+%�F=7�;3,�B:3�tW��U��U��U��U�oAb��2��.���,���Og��ga[�YOH�,$�.& �{rj�;3,�w^��U��V��V��S)�K6���)���*���&���O���wld�;3,�wld�dZS�B:3�YOH�i@��V��U��U��U�nL~��*���+���,���!���c{}�QHA�B:3��{r��{c�o_r�S��R*�kS�zVޯS(�,\q��=���'������ ������d�����_���4�������dhT���Q�jpFK;j[HKK%EGG-_{����� ��� ��� ���p�{���������������C섇b��� ���3熏A�EGGEGG@6��� ��� ��� �����i���������������/�EGGu��L������[�    EGGEGG@>������ ���fϊ���
������!���7�TVP�[U{]\]XW��`�ehYH        EGGEGG'FX\`9���;��΅�8҃�<�ejF{EGGX[Aȷrb��EGG EGGEGG	            EGGEGGEGGEGG%EGG'EGGEGGEGG _H��d\�XEGG        �  �                                               �   �   �  >   0   �� I C O N _ L O G O       0            @@     (B   00     �%          �        h     