//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef IDC_STATIC
#define IDC_STATIC	-1
#endif

#define	IDI_M30ICON				100
#define	IDR_MENU1				101
#define	IDR_ACCELERATOR1		102

#define	IDI_NEW_OFF_ICON      201
#define	IDI_OPEN_OFF_ICON     202
#define	IDI_CLOSE_OFF_ICON    203
#define	IDI_CLOSEALL_OFF_ICON 204
#define	IDI_SAVE_OFF_ICON     205
#define	IDI_SAVEALL_OFF_ICON  206
#define	IDI_CUT_OFF_ICON      207   
#define	IDI_COPY_OFF_ICON     208   
#define	IDI_PASTE_OFF_ICON    209   
#define	IDI_UNDO_OFF_ICON     210   
#define	IDI_REDO_OFF_ICON     211   
#define	IDI_FIND_OFF_ICON     212   
#define	IDI_REPLACE_OFF_ICON  213
#define	IDI_ZOOMIN_OFF_ICON   214
#define	IDI_ZOOMOUT_OFF_ICON  215
#define	IDI_VIEW_UD_DLG_OFF_ICON 216
#define	IDI_PRINT_OFF_ICON    217
#define	IDI_VIEW_ALL_CHAR_ON_ICON  218
#define	IDI_VIEW_INDENT_ON_ICON 219
#define	IDI_VIEW_WRAP_ON_ICON 220

#define	IDI_NEW_ON_ICON      301
#define	IDI_OPEN_ON_ICON     302
#define	IDI_CLOSE_ON_ICON    303
#define	IDI_CLOSEALL_ON_ICON 304
#define	IDI_SAVE_ON_ICON     305
#define	IDI_SAVEALL_ON_ICON  306
#define	IDI_CUT_ON_ICON      307
#define	IDI_COPY_ON_ICON     308
#define	IDI_PASTE_ON_ICON    309
#define	IDI_UNDO_ON_ICON     310
#define	IDI_REDO_ON_ICON     311
#define	IDI_FIND_ON_ICON     312
#define	IDI_REPLACE_ON_ICON  313
#define	IDI_ZOOMIN_ON_ICON   314
#define	IDI_ZOOMOUT_ON_ICON  315
#define	IDI_VIEW_UD_DLG_ON_ICON 316
#define	IDI_PRINT_ON_ICON    317
#define	IDI_VIEW_ALL_CHAR_OFF_ICON  318
#define	IDI_VIEW_INDENT_OFF_ICON 319
#define	IDI_VIEW_WRAP_OFF_ICON 320

//#define	IDI_NEW_DISABLE_ICON  401
//#define	IDI_OPEN_ON_ICON      402
#define	IDI_SAVE_DISABLE_ICON   403  
#define	IDI_SAVEALL_DISABLE_ICON 404
//#define	IDI_CLOSE_ON_ICON       405
//#define	IDI_CLOSEALL_ON_ICON    406
#define	IDI_CUT_DISABLE_ICON      407
#define	IDI_COPY_DISABLE_ICON     408
#define	IDI_PASTE_DISABLE_ICON   409
#define	IDI_UNDO_DISABLE_ICON    410
#define	IDI_REDO_DISABLE_ICON    411


#define	IDI_SAVED_ICON       501
#define	IDI_UNSAVED_ICON     502
#define	IDI_READONLY_ICON     503

#define	IDC_MY_CUR     1402
#define	IDC_UP_ARROW  1403
#define	IDC_DRAG_TAB    1404
#define	IDC_DRAG_INTERDIT_TAB 1405
#define	IDC_DRAG_PLUS_TAB 1406

#define	IDR_SAVEALL			1500
#define	IDR_CLOSEFILE		1501
#define	IDR_CLOSEALL		1502
#define	IDR_FIND				1503
#define	IDR_REPLACE			1504
#define	IDR_ZOOMIN			1505
#define	IDR_ZOOMOUT		1506
#define	IDR_WRAP			1507
#define	IDR_INVISIBLECHAR	1508
#define	IDR_INDENTGUIDE		1509
#define	IDR_SHOWPANNEL		1510

#define IDM 40000

#define	IDM_FILE       (IDM + 1000)
	#define	IDM_FILE_NEW                     		(IDM_FILE + 1)
	#define	IDM_FILE_OPEN                    		(IDM_FILE + 2)
	#define	IDM_FILE_CLOSE                   		(IDM_FILE + 3)
	#define	IDM_FILE_CLOSEALL              		(IDM_FILE + 4)
	#define	IDM_FILE_CLOSEALL_BUT_CURRENT   (IDM_FILE + 5)
	#define	IDM_FILE_SAVE                    		(IDM_FILE + 6) 
	#define	IDM_FILE_SAVEALL               		(IDM_FILE + 7) 
	#define	IDM_FILE_SAVEAS		   		(IDM_FILE + 8)
	#define	IDM_FILE_ASIAN_LANG	   		(IDM_FILE + 9)  
	#define	IDM_FILE_PRINT		   		(IDM_FILE + 10)
	#define	IDM_FILE_EXIT			   		(IDM_FILE + 11)
   
#define	IDM_EDIT       (IDM + 2000)
	#define	IDM_EDIT_CUT					(IDM_EDIT + 1) 
	#define	IDM_EDIT_COPY				(IDM_EDIT + 2)
	#define	IDM_EDIT_UNDO				(IDM_EDIT + 3)
	#define	IDM_EDIT_REDO				(IDM_EDIT + 4)
	#define	IDM_EDIT_PASTE				(IDM_EDIT + 5)
	#define	IDM_EDIT_DELETE				(IDM_EDIT + 6)
	#define	IDM_EDIT_SELECTALL          (IDM_EDIT + 7)
	
	#define	IDM_EDIT_INS_TAB            (IDM_EDIT + 8)
	#define	IDM_EDIT_RMV_TAB            (IDM_EDIT + 9)
	#define	IDM_EDIT_DUP_LINE           (IDM_EDIT + 10)
	#define	IDM_EDIT_TRANSPOSE_LINE     (IDM_EDIT + 11)
	#define	IDM_EDIT_SPLIT_LINES        (IDM_EDIT + 12)
	#define	IDM_EDIT_JOIN_LINES         (IDM_EDIT + 13)
	#define	IDM_EDIT_LINE_UP            (IDM_EDIT + 14)
	#define	IDM_EDIT_LINE_DOWN          (IDM_EDIT + 15)
	        
#define	IDM_SEARCH       (IDM + 3000)

	#define	IDM_SEARCH_FIND	                		(IDM_SEARCH + 1)
	#define	IDM_SEARCH_FINDNEXT				(IDM_SEARCH + 2)
	#define	IDM_SEARCH_REPLACE              		(IDM_SEARCH + 3)
	#define	IDM_SEARCH_GOTOLINE				(IDM_SEARCH + 4)
	#define	IDM_SEARCH_TOGGLE_BOOKMARK		(IDM_SEARCH + 5)
	#define	IDM_SEARCH_NEXT_BOOKMARK		(IDM_SEARCH + 6)
	#define	IDM_SEARCH_PREV_BOOKMARK		(IDM_SEARCH + 7)
	#define	IDM_SEARCH_CLEAR_BOOKMARKS		(IDM_SEARCH + 8)

			
#define IDM_VIEW	(IDM + 4000)
	#define	IDM_VIEW_TOOLBAR_HIDE			(IDM_VIEW + 1)
	#define	IDM_VIEW_TOOLBAR_REDUCE			(IDM_VIEW + 2)	
	#define	IDM_VIEW_TOOLBAR_ENLARGE		(IDM_VIEW + 3)
	#define	IDM_VIEW_TOOLBAR_STANDARD		(IDM_VIEW + 4)
	#define	IDM_VIEW_REDUCETABBAR			(IDM_VIEW + 5)
	#define	IDM_VIEW_LOCKTABBAR				(IDM_VIEW + 6) 
	#define	IDM_VIEW_DRAWTABBAR_TOPBAR   	(IDM_VIEW + 7)
	#define	IDM_VIEW_DRAWTABBAR_INACIVETAB	(IDM_VIEW + 8) 
	#define	IDM_VIEW_STATUSBAR        		(IDM_VIEW + 9)  
	#define	IDM_VIEW_TOGGLE_FOLDALL			(IDM_VIEW + 10)
	#define	IDM_VIEW_USER_DLG				(IDM_VIEW + 11)
	#define	IDM_VIEW_LINENUMBER             (IDM_VIEW + 12)
	#define	IDM_VIEW_SYMBOLMARGIN           (IDM_VIEW + 13)
	#define	IDM_VIEW_FOLDERMAGIN            (IDM_VIEW + 14)
	#define	IDM_VIEW_FOLDERMAGIN_SIMPLE     (IDM_VIEW + 15)
	#define	IDM_VIEW_FOLDERMAGIN_ARROW      (IDM_VIEW + 16)
    #define	IDM_VIEW_FOLDERMAGIN_CIRCLE     (IDM_VIEW + 17)
	#define	IDM_VIEW_FOLDERMAGIN_BOX        (IDM_VIEW + 18)
	#define	IDM_VIEW_ALL_CHARACTERS		 	(IDM_VIEW + 19)
	#define	IDM_VIEW_INDENT_GUIDE		 	(IDM_VIEW + 20)
	#define	IDM_VIEW_CURLINE_HILITING		(IDM_VIEW + 21)
	#define	IDM_VIEW_WRAP					(IDM_VIEW + 22)
	#define	IDM_VIEW_ZOOMIN			 		(IDM_VIEW + 23)
	#define	IDM_VIEW_ZOOMOUT			 	(IDM_VIEW + 24)
	#define	IDM_VIEW_TAB_SPACE		        (IDM_VIEW + 25)
	#define	IDM_VIEW_EOL			        (IDM_VIEW + 26)
	#define	IDM_VIEW_EDGELINE		        (IDM_VIEW + 27)
	#define	IDM_VIEW_EDGEBACKGROUND	        (IDM_VIEW + 28)

#define	IDM_FORMAT  (IDM + 5000)                          
	#define	 IDM_FORMAT_TODOS			(IDM_FORMAT + 1)
	#define	 IDM_FORMAT_TOUNIX		(IDM_FORMAT + 2)
	#define	 IDM_FORMAT_TOMAC		 	(IDM_FORMAT + 3)
	//NEW
	#define     IDM_FORMAT_ANSI 			(IDM_FORMAT + 4)
	#define     IDM_FORMAT_UTF_8			(IDM_FORMAT + 5)
	#define     IDM_FORMAT_UCS_2BE		(IDM_FORMAT + 6)
	#define     IDM_FORMAT_UCS_2LE	    (IDM_FORMAT + 7)
	#define     IDM_FORMAT_AS_UTF_8	(IDM_FORMAT + 8)
    //WEN
	
#define	IDM_LANG 	(IDM + 6000)
	#define	IDM_LANGSTYLE_CONFIG_DLG	(IDM_LANG + 1)
	#define	IDM_LANG_C 		    	(IDM_LANG + 2)
	#define	IDM_LANG_CPP 		(IDM_LANG + 3)
	#define	IDM_LANG_JAVA 		(IDM_LANG + 4)
	#define	IDM_LANG_HTML 		(IDM_LANG + 5)		
	#define	IDM_LANG_XML		(IDM_LANG + 6)
	#define	IDM_LANG_JS			(IDM_LANG + 7)
	#define	IDM_LANG_PHP		(IDM_LANG + 8) 
	#define	IDM_LANG_ASP		(IDM_LANG + 9)
	#define	IDM_LANG_CSS        (IDM_LANG + 10)
	#define	IDM_LANG_PASCAL		(IDM_LANG + 11)
	#define	IDM_LANG_PYTHON		(IDM_LANG + 12)
	#define	IDM_LANG_PERL		(IDM_LANG + 13)
	#define	IDM_LANG_OBJC		(IDM_LANG + 14) 
	#define	IDM_LANG_ASCII		(IDM_LANG + 15)
	#define	IDM_LANG_TEXT		(IDM_LANG + 16)
	#define	IDM_LANG_RC			(IDM_LANG + 17)
	#define	IDM_LANG_MAKEFILE	(IDM_LANG + 18)
	#define	IDM_LANG_INI		(IDM_LANG + 19)
	#define	IDM_LANG_SQL		(IDM_LANG + 20)
	#define	IDM_LANG_VB   		(IDM_LANG + 21)
	#define	IDM_LANG_BATCH  	(IDM_LANG + 22)
    #define	IDM_LANG_CS         (IDM_LANG + 23)
    #define	IDM_LANG_LUA         (IDM_LANG + 24)
    #define	IDM_LANG_TEX         (IDM_LANG + 25)
    #define	IDM_LANG_FORTRAN         (IDM_LANG + 26)
    #define	IDM_LANG_SH         (IDM_LANG + 27)
    #define	IDM_LANG_FLASH         (IDM_LANG + 28)
    #define	IDM_LANG_NSIS         (IDM_LANG + 29)
	
	#define	IDM_LANG_USER		(IDM_LANG + 50)     //46050

    #define	IDM_LANG_USER_LIMIT		(IDM_LANG + 80)  //46080
    
#define	IDM_ABOUT 	(IDM  + 7000)
#define	IDM_TRY         (IDM + 7001)

#define	IDM_SETTING    (IDM + 8000)
	#define	IDM_SETTING_TAB_SIZE   	       (IDM_SETTING + 1)
	#define	IDM_SETTING_TAB_REPLCESPACE  (IDM_SETTING + 2)
    #define	IDM_SETTING_HISTORY_SIZE  (IDM_SETTING + 3)

	#define	IDM_SETTING_EDGE_SIZE  (IDM_SETTING + 4)
	//#define	IDM_SETTING_EDGE_COLOUR  (IDM_SETTING + 5)
	
#define	IDM_EXECUTE  (IDM + 9000)      

#define  IDC_DOC_GOTO_ANOTHER_VIEW  		10001
#define  IDC_DOC_CLONE_TO_ANOTHER_VIEW  	10002
#define	IDC_PREV_DOC	10003
#define	IDC_NEXT_DOC	10004


#define	IDD_FILEVIEW_DIALOG				1000
#define IDC_BUTTON_PRINT                1001

#define IDD_CREATE_DIRECTORY			1100
#define IDC_STATIC_CURRENT_FOLDER       1101
#define IDC_EDIT_NEW_FOLDER             1102

#define	IDD_INSERT_INPUT_TEXT			1200
#define	IDC_EDIT_INPUT_VALUE			1201
#define	IDC_STATIC_INPUT_TITLE			1202
#define	IDC_ICON_INPUT_ICON				1203

#define	IDR_M30_MENU					1500
#define	IDR_M30_ACCELERATORS		1501

// #define	IDD_FIND_REPLACE_DLG		1600

#define	IDD_ABOUTBOX 1700
#define	IDC_LICENCE_EDIT 1701
#define	IDC_HOME_ADDR		1702
#define	IDC_EMAIL_ADDR		1703


//#define	IDD_USER_DEFINE_BOX 				 1800

#define	IDD_GOLINE		2000
#define	ID_GOLINE_EDIT	(IDD_GOLINE + 1)
#define	ID_CURRLINE		(IDD_GOLINE + 2)
#define	ID_LASTLINE		(IDD_GOLINE + 3)
#define	ID_URHERE_STATIC           (IDD_GOLINE + 4)
#define	ID_UGO_STATIC                 (IDD_GOLINE + 5)
#define	ID_NOMORETHAN_STATIC   (IDD_GOLINE + 6)
//#define	IDD_COLOUR_POPUP   2100

// See WordStyleDlgRes.h
//#define	IDD_STYLER_DLG	2200
//#define IDD_GLOBAL_STYLER_DLG	2300

#define	IDD_VALUE_DLG       2400
#define	IDC_VALUE_STATIC  2401
#define	IDC_VALUE_EDIT      2402

#define	IDD_SETTING_DLG    2500

#define	IDC_AUTOCOMPLETE 3000
