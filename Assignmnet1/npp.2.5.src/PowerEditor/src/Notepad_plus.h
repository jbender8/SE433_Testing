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

#ifndef NOTEPAD_PLUS_H
#define NOTEPAD_PLUS_H

#include "Window.h"
#include "ScintillaEditView.h"
#include "ToolBar.h"
#include "ImageListSet.h"
#include "DocTabView.h"

#include "StaticDialog.h"
#include "SplitterContainer.h"
#include "FindReplaceDlg.h"
#include "AboutDlg.h"
#include "RunDlg.h"
#include "UserDefineDialog.h"
#include "StatusBar.h"
#include "Parameters.h"
#include "lastRecentFileList.h"
#include "GoToLineDlg.h"
#include "WordStyleDlg.h"
#include "constant.h"

// WebControl bug
//#include "WebControl.h"

#define NOTEPAD_PP_CLASS_NAME	"Notepad++"
#define EXPLORER_PLUGIN_PATH    ".\\plugins\\Win32Explr.dll"

#define MENU 0x01
#define TOOLBAR 0x02

//#define WM_LOADFILEBYPATH WM_USER

const bool MODE_TRANSFER = true;
const bool MODE_CLONE = false;

const unsigned char DOCK_MASK = 1;
const unsigned char TWO_VIEWS_MASK = 2;

const int MAIN_VIEW = 0;
const int SUB_VIEW = 1;

const int STATUSBAR_DOC_TYPE = 0;
const int STATUSBAR_DOC_SIZE = 1;
const int STATUSBAR_CUR_POS = 2;
const int STATUSBAR_EOF_FORMAT = 3;
const int STATUSBAR_UNICODE_TYPE = 4;
const int STATUSBAR_TYPING_MODE = 5;


// pour la fonction changeDlgLang()
const int DLG_FIND = 0;
const int DLG_GOTOLINE = 1;
const int DLG_RUN = 2;

struct iconLocator {
	int listIndex;
	int iconIndex;
	std::string iconLocation;

	iconLocator(int iList, int iIcon, const std::string iconLoc) 
		: listIndex(iList), iconIndex(iIcon), iconLocation(iconLoc){};
};

class Notepad_plus : public Window
{
public:
	Notepad_plus();

	void init(HINSTANCE, HWND, const char *);

	// ATTENTION : the order of the destruction is very important
	// because if the parent's window hadle is destroyed before
	// the destruction of its childrens' windows handle, 
	// its childrens' windows handle will be destroyed automatically!
	virtual ~Notepad_plus(){
		(NppParameters::getInstance())->destroyInstance();
	};

	void killAllChildren() {
		_toolBar.destroy();

        if (_pMainSplitter)
        {
            _pMainSplitter->destroy();
            delete _pMainSplitter;
        }

        _mainDocTab.destroy();
        _subDocTab.destroy();

		_mainEditView.destroy();
        _subEditView.destroy();

        _subSplitter.destroy();
        _statusBar.destroy();

		_configStyleDlg.destroy();

		if (_findReplaceDlg.isCreated())
			_findReplaceDlg.destroy();

        if (_aboutDlg.isCreated())
			_aboutDlg.destroy();

		if (_runDlg.isCreated())
			_runDlg.destroy();

        if (_goToLineDlg.isCreated())
			_goToLineDlg.destroy();

        if (_hTabPopupMenu)
            DestroyMenu(_hTabPopupMenu);

        if (_hTabPopupDropMenu)
            DestroyMenu(_hTabPopupDropMenu);

        // WebControl bug
        //_webBrowser.destroy();

	};

	virtual void destroy() {
		::DestroyWindow(_hSelf);
	};

    static const char * getClassName() {
        return _className;
    };

	void setTitle(const char *title) const {
		::SetWindowText(_hSelf, _className);
	};
	
	void setTitleWith(const char *filePath);

	// For filtering the modeless Dialog message
	bool isDlgsMsg(MSG *msg) const {
		if (_findReplaceDlg.isCreated())
		{
			if (::IsDialogMessage(_findReplaceDlg.getHSelf(), msg))
				return true;
		}
		if (_aboutDlg.isCreated())
		{
			if (::IsDialogMessage(_aboutDlg.getHSelf(), msg))
				return true;
		}

		if (_runDlg.isCreated())
		{
			if (::IsDialogMessage(_runDlg.getHSelf(), msg))
				return true;
		}

		if (_goToLineDlg.isCreated())
		{
			if (::IsDialogMessage(_goToLineDlg.getHSelf(), msg))
				return true;
		}

		return false;
	};
    bool doOpen(const char *fileName);

	void saveScintillaParams(bool whichOne) {
		ScintillaViewParams svp;
		ScintillaEditView *pView = (whichOne == SCIV_PRIMARY)?&_mainEditView:&_subEditView;

		svp._lineNumberMarginShow = pView->hasMarginShowed(ScintillaEditView::_SC_MARGE_LINENUMBER); 
		svp._bookMarkMarginShow = pView->hasMarginShowed(ScintillaEditView::_SC_MARGE_SYBOLE);
		svp._indentGuideLineShow = pView->isShownIndentGuide();
		svp._folderStyle = pView->getFolderStyle();
		svp._currentLineHilitingShow = pView->isCurrentLineHiLiting();
		svp._doWrap = pView->isWrap();
		svp._edgeMode = pView->execute(SCI_GETEDGEMODE);
		//svp._edgeNbColumn = pView->execute(SCI_GETEDGECOLUMN);
		(NppParameters::getInstance())->writeScintillaParams(svp, whichOne);
	};

	void saveGUIParams(){
		NppGUI & nppGUI = (NppGUI &)(NppParameters::getInstance())->getNppGUI();
		nppGUI._statusBarShow = _statusBar.isVisible();
		nppGUI._toolBarStatus = _toolBar.getState();

		nppGUI._tabStatus = (TabBar::doDragNDropOrNot()?TAB_DRAWTOPBAR:0) | (TabBar::drawTopBar()?TAB_DRAGNDROP:0) | (TabBar::drawInactiveTab()?TAB_DRAWINACTIVETAB:0) | (_toReduceTabBar?TAB_REDUCE:0);
		nppGUI._splitterPos = _subSplitter.isVertical()?POS_VERTICAL:POS_HORIZOTAL;
		UserDefineDialog *udd = _pEditView->getUserDefineDlg();
		bool b = udd->isDocked();
		nppGUI._userDefineDlgStatus = (b?UDD_DOCKED:0) | (udd->isVisible()?UDD_SHOW:0);
		//ADD RECT
		RECT rc;
		if (::IsIconic(_hSelf))
			::ShowWindow(_hSelf, SW_SHOWNORMAL);
		::GetWindowRect(_hSelf, &rc);
		nppGUI._appPos.left = rc.left;
		nppGUI._appPos.top = rc.top;
		nppGUI._appPos.right = rc.right - rc.left;
		nppGUI._appPos.bottom = rc.bottom - rc.top;
		(NppParameters::getInstance())->writeGUIParams();
	};
	void saveUserDefineLangs() {
		if (ScintillaEditView::getUserDefineDlg()->isDirty())
		//if (true)
			(NppParameters::getInstance())->writeUserDefinedLang();
	};

	void changeDlgLang(int dlgID);
	void changeConfigLang();
	void changeUserDefineLang();
	void changeMenuLang();
	const char * getNativeTip(int btnID);
	void changeToolBarIcons();

private:
	static const char _className[32];
	char _nppPath[MAX_PATH];
    Window *_pMainWindow;
	TiXmlNode *_nativeLang, *_toolIcons;

    unsigned char _mainWindowStatus;

    DocTabView _mainDocTab;
    DocTabView _subDocTab;
    DocTabView *_pDocTab;

    ScintillaEditView _mainEditView;
    ScintillaEditView _subEditView;
    ScintillaEditView *_pEditView;

    SplitterContainer *_pMainSplitter;
    SplitterContainer _subSplitter;

    HMENU _hTabPopupMenu, _hTabPopupDropMenu;

	ToolBar	_toolBar;
	IconList _docTabIconList;
    StatusBar _statusBar;
	bool _toReduceTabBar;

	// Dialog
	FindReplaceDlg _findReplaceDlg;
    AboutDlg _aboutDlg;
	RunDlg _runDlg;
    GoToLineDlg _goToLineDlg;
	GlobalStyleDlg _configStyleDlg;

	int _nbLRFile;
	LastRecentFileList _lastRecentFileList;

	std::vector<iconLocator> _customIconVect;

    // WebControl bug
	//WebControl _webBrowser;

	//Plugin for explorer
	HMODULE _hExplorer;
    HWND (*_doExplorer)(HINSTANCE,  HWND );

	static LRESULT CALLBACK Notepad_plus_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	LRESULT runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	void notify(SCNotification *notification);
	void command(int id);
	void fileNew(){
		setTitleWith(_pDocTab->newDoc(NULL));

		updateStatusBar();
		dynamicCheckMenuAndTB();
	};

	void fileOpen();
	bool fileClose();
	bool fileCloseAll();
	bool fileCloseAllButCurrent();

	void hideCurrentView();

	int doSaveOrNot(const char *fn) {
		char phrase[512] = "Save file \"";
		strcat(strcat(phrase, fn), "\" ?");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL);
	};
	
	int doReloadOrNot(const char *fn) {
		char phrase[512] = "The file \"";
		strcat(strcat(phrase, fn), "\" is modified by another program. Reload this file?");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
	};

	int doCloseOrNot(const char *fn) {
		char phrase[512] = "The file \"";
		strcat(strcat(phrase, fn), "\" doesn't exist anymore. Keep this file in editor ? (Yes keep it, No remove it)");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
	};
	
	bool fileSave();
	bool fileSaveAll();
	bool fileSaveAs();
	void filePrint(bool showDialog);
	bool doSave(const char *filename, UniMode mode);
	void enableMenu(int cmdID, bool doEnable) const {
		int flag = doEnable?MF_ENABLED | MF_BYCOMMAND:MF_DISABLED | MF_GRAYED | MF_BYCOMMAND;
		::EnableMenuItem(::GetMenu(_hSelf), cmdID, flag);
	}
	void enableCommand(int cmdID, bool doEnable, int which) const;
	void checkClipboard();
	void checkDocState();
	void checkUndoState();
	void dropFiles(HDROP hdrop);
	void checkModifiedDocument();
	void reload(const char *fileName);

    void docGotoAnotherEditView(bool mode);
    void dockUserDlg();
    void undockUserDlg();

    void getToolBarClientRect(RECT &rc) const;
    void getMainClientRect(RECT & rc) const;
    void getStatusBarClientRect(RECT & rc) const;

    void switchEditViewTo(int gid) {
        _pDocTab = (gid == MAIN_VIEW)?&_mainDocTab:&_subDocTab;
        _pEditView = (gid == MAIN_VIEW)?&_mainEditView:&_subEditView;
		_pEditView->beSwitched();
        _pEditView->getFocus();

        checkDocState();
        setTitleWith(_pEditView->getCurrentTitle());
		setLangStatus(_pEditView->getCurrentDocType());
		updateStatusBar();
		dynamicCheckMenuAndTB();
    };
	
	void dynamicCheckMenuAndTB() const;

	void enableConvertMenuItems(formatType f) const {
		enableCommand(IDM_FORMAT_TODOS, (f != WIN_FORMAT), MENU);
		enableCommand(IDM_FORMAT_TOUNIX, (f != UNIX_FORMAT), MENU);
		enableCommand(IDM_FORMAT_TOMAC, (f != MAC_FORMAT), MENU);
	};

	void checkUnicodeMenuItems(UniMode um) const {
		int id = -1;
		switch (um)
		{
			case uni8Bit : id = IDM_FORMAT_ANSI; break;
			case uniUTF8 : id = IDM_FORMAT_UTF_8; break;
			case uni16BE : id = IDM_FORMAT_UCS_2BE; break;
			case uni16LE : id = IDM_FORMAT_UCS_2LE; break;
		}
		if (id != -1)
		{
			::CheckMenuRadioItem(::GetMenu(_hSelf), IDM_FORMAT_ANSI, IDM_FORMAT_UCS_2LE, id, MF_BYCOMMAND);

			//if (um != uni8Bit)
			checkMenuItem(IDM_FORMAT_AS_UTF_8, FALSE);
			enableCommand(IDM_FORMAT_AS_UTF_8, (um == uni8Bit), MENU);
		}
		else
		{
			::CheckMenuRadioItem(::GetMenu(_hSelf), IDM_FORMAT_ANSI, IDM_FORMAT_UCS_2LE, IDM_FORMAT_ANSI, MF_BYCOMMAND);
			enableCommand(IDM_FORMAT_AS_UTF_8, true, MENU);
			checkMenuItem(IDM_FORMAT_AS_UTF_8, true);
		}
	};

    int getCurrentView() const {
        return (_pEditView == &_mainEditView)?MAIN_VIEW:SUB_VIEW;
    };

	int getNonCurrentView() const {
        return (_pEditView == &_mainEditView)?SUB_VIEW:MAIN_VIEW;
    };

    DocTabView * getNonCurrentDocTab() {
        return (_pDocTab == &_mainDocTab)?&_subDocTab:&_mainDocTab;
    };

    ScintillaEditView * getNonCurrentEditView() {
        return (_pEditView == &_mainEditView)?&_subEditView:&_mainEditView;
    };

    void synchronise();

    void setLangStatus(LangType langType);

	void setDisplayFormat(formatType f) {
		std::string str;
		switch (f)
		{
			case MAC_FORMAT :
				str = "MAC";
				break;
			case UNIX_FORMAT :
				str = "UNIX";
				break;
			default :
				str = "Dos\\Windows";
		}
		_statusBar.setText(str.c_str(), STATUSBAR_EOF_FORMAT);
	};

	void setUniModeText(UniMode um)
	{
		char *uniModeText;
		switch (um)
		{
			case uniUTF8:
				uniModeText = "UTF-8"; break;
			case uni16BE:
				uniModeText = "UCS-2 Big Ending"; break;
			case uni16LE:
				uniModeText = "UCS-2 little Ending"; break;
			case uniCookie:
				uniModeText = "ANSI as UTF-8"; break;
			default :
				uniModeText = "ANSI";
		}
		_statusBar.setText(uniModeText, STATUSBAR_UNICODE_TYPE);
	};

	int langTypeToCommandID(LangType lt) const {
		int id;
		switch (lt)
		{
			case L_C :	
				id = IDM_LANG_C; break;
			
			case L_CPP :
				id = IDM_LANG_CPP; break;

			case L_JAVA :
				id = IDM_LANG_JAVA;	break;

			case L_CS :
				id = IDM_LANG_CS; break;

			case L_OBJC :
				id = IDM_LANG_OBJC;	break;

			case L_HTML :
				id = IDM_LANG_HTML;	break;

			case L_XML : 
				id = IDM_LANG_XML; break;

			case L_JS :
				id = IDM_LANG_JS; break;

			case L_PHP :
				id = IDM_LANG_PHP; break;

			case L_ASP :
				id = IDM_LANG_ASP; break;

			case L_CSS :
				id = IDM_LANG_CSS; break;

			case L_LUA :
				id = IDM_LANG_LUA; break;

			case L_PERL :
				id = IDM_LANG_PERL; break;

			case L_PYTHON :
				id = IDM_LANG_PYTHON; break;

			case L_BATCH :
				id = IDM_LANG_BATCH; break;

			case L_PASCAL :
				id = IDM_LANG_PASCAL; break;

			case L_MAKEFILE :
				id = IDM_LANG_MAKEFILE;	break;

			case L_INI :
				id = IDM_LANG_INI; break;

			case L_NFO :
				id = IDM_LANG_ASCII; break;

			case L_RC :
				id = IDM_LANG_RC; break;

			case L_TEX :
				id = IDM_LANG_TEX; break;

			case L_FORTRAN : 
				id = IDM_LANG_FORTRAN; break;

			case L_BASH : 
				id = IDM_LANG_SH; break;

			case L_FLASH :
				id = IDM_LANG_FLASH; break;

			case L_NSIS :
				id = IDM_LANG_NSIS; break;

			case L_USER :
				id = IDM_LANG_USER; break;

            case L_SQL :
                id = IDM_LANG_SQL; break;

            case L_VB :
                id = IDM_LANG_VB; break;

			case L_TXT :
				id = IDM_LANG_TEXT;	break;

            
			default :
				id = IDM_LANG_TEXT;
		}
		return id;
	};

	void checkLangsMenu(int id) const {
		if (id == -1)
		{
			id = langTypeToCommandID(_pEditView->getCurrentDocType());
			if (id == IDM_LANG_USER)
			{
				if (_pEditView->getCurrentBuffer().isUserDefineLangExt())
					return;
			}
		}
		::CheckMenuRadioItem(::GetMenu(_hSelf), IDM_LANG_C, IDM_LANG_USER_LIMIT, id, MF_BYCOMMAND);
	}
    void setLanguage(int id, LangType langType) {
        _pEditView->setCurrentDocType(langType);
        setLangStatus(langType);
		checkLangsMenu(id);
    };

    int getToolBarState() const {
        HMENU hMenu = ::GetMenu(_hSelf);

        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_HIDE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_HIDE;
        
        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_REDUCE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_REDUCE;

        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_ENLARGE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_ENLARGE;

        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_STANDARD, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_STANDARD;

		return -1;
    };

    int getFolderMarginStyle() const {
        HMENU hMenu = ::GetMenu(_hSelf);

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_SIMPLE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_SIMPLE;
        
        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_ARROW, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_ARROW;

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_CIRCLE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_CIRCLE;

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_BOX, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_BOX;

		return 0;
    };

    void checkToolBarMenu(int id2Check) const {
        ::CheckMenuRadioItem(::GetMenu(_hSelf), IDM_VIEW_TOOLBAR_HIDE, IDM_VIEW_TOOLBAR_STANDARD, id2Check, MF_BYCOMMAND);
    };

	void checkFolderMarginStyleMenu(int id2Check) const {
		::CheckMenuRadioItem(::GetMenu(_hSelf), IDM_VIEW_FOLDERMAGIN_SIMPLE, IDM_VIEW_FOLDERMAGIN_BOX, id2Check, MF_BYCOMMAND);
	};

    int checkStatusBar() const {
        HMENU hMenu = ::GetMenu(_hSelf);
        int check = (::GetMenuState(hMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND) == MF_CHECKED)?MF_UNCHECKED:MF_CHECKED;
        ::CheckMenuItem(hMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND | check);
        return check;
    };

    int getFolderMaginStyleIDFrom(folderStyle fStyle) const {
        switch (fStyle)
        {
            case FOLDER_STYLE_SIMPLE : return IDM_VIEW_FOLDERMAGIN_SIMPLE;
            case FOLDER_STYLE_ARROW : return IDM_VIEW_FOLDERMAGIN_ARROW;
            case FOLDER_STYLE_CIRCLE : return IDM_VIEW_FOLDERMAGIN_CIRCLE;
            case FOLDER_STYLE_BOX : return IDM_VIEW_FOLDERMAGIN_BOX;
			default : return FOLDER_TYPE;
        }
        //return 
    };

	void checkMenuItem(int itemID, bool willBeChecked) const {
		::CheckMenuItem(::GetMenu(_hSelf), itemID, MF_BYCOMMAND | (willBeChecked?MF_CHECKED:MF_UNCHECKED));
	};
	void charAdded(char chAdded);
	void MaintainIndentation(char ch);

    void bookmarkAdd(int lineno) const {
		if (lineno == -1)
			lineno = _pEditView->getCurrentLineNumber();
		if (!bookmarkPresent(lineno))
			_pEditView->execute(SCI_MARKERADD, lineno, MARK_SYMBOLE);
	};
    void bookmarkDelete(int lineno) const {
		if (lineno == -1)
			lineno = _pEditView->getCurrentLineNumber();
		if ( bookmarkPresent(lineno))
			_pEditView->execute(SCI_MARKERDELETE, lineno, MARK_SYMBOLE);
	};
    bool bookmarkPresent(int lineno) const {
		if (lineno == -1)
			lineno = _pEditView->getCurrentLineNumber();
		int state = _pEditView->execute(SCI_MARKERGET, lineno);
		return state & (1 << MARK_SYMBOLE);
	};
    void bookmarkToggle(int lineno) const {
		if (lineno == -1)
			lineno = _pEditView->getCurrentLineNumber();

		if (bookmarkPresent(lineno))
			bookmarkDelete(lineno);
		else
    		bookmarkAdd(lineno);
	};
    void bookmarkNext(bool forwardScan);
	void bookmarkClearAll() const {
		_pEditView->execute(SCI_MARKERDELETEALL, MARK_SYMBOLE);
	};

    void findMatchingBracePos(int & braceAtCaret, int & braceOpposite);
    void braceMatch();
   
    void activateNextDoc(bool direction);

	void updateStatusBar() {
        char strLnCol[64];
		sprintf(strLnCol, "Ln : %d    Col : %d    Sel : %d", 
            (_pEditView->getCurrentLineNumber() + 1), \
			(_pEditView->getCurrentColumnNumber() + 1),\
			(_pEditView->getSelectedByteNumber()));

        _statusBar.setText(strLnCol, STATUSBAR_CUR_POS);

		char strDonLen[64];
		sprintf(strDonLen, "%d bytes", _pEditView->getCurrentDocLen());
		_statusBar.setText(strDonLen, STATUSBAR_DOC_SIZE);

		setDisplayFormat((_pEditView->getCurrentBuffer()).getFormat());
		setUniModeText(_pEditView->getCurrentBuffer().getUnicodeMode());
        _statusBar.setText(_pEditView->execute(SCI_GETOVERTYPE) ? "OVR" : "INS", STATUSBAR_TYPING_MODE);
    };
	void showAutoComp();
	void getApiFileName(LangType langType, std::string &fn);

	void foldAll() {
		_pEditView->execute(SCI_COLOURISE, 0, -1);
		int maxLine = _pEditView->execute(SCI_GETLINECOUNT);
		bool expanding = true;
		for (int lineSeek = 0; lineSeek < maxLine; lineSeek++) 
		{
			if (_pEditView->execute(SCI_GETFOLDLEVEL, lineSeek) & SC_FOLDLEVELHEADERFLAG) 
			{
				expanding = !_pEditView->execute(SCI_GETFOLDEXPANDED, lineSeek);
				break;
			}
		}
		for (int line = 0; line < maxLine; line++) 
		{
			int level = _pEditView->execute(SCI_GETFOLDLEVEL, line);
			if ((level & SC_FOLDLEVELHEADERFLAG) && (SC_FOLDLEVELBASE == (level & SC_FOLDLEVELNUMBERMASK))) 
			{
				if (expanding) 
				{
					_pEditView->execute(SCI_SETFOLDEXPANDED, line, 1);
					_pEditView->expand(line, true, false, 0, level);
					line--;
				} 
				else 
				{
					int lineMaxSubord = _pEditView->execute(SCI_GETLASTCHILD, line, -1);
					_pEditView->execute(SCI_SETFOLDEXPANDED, line, 0);
					if (lineMaxSubord > line)
						_pEditView->execute(SCI_HIDELINES, line + 1, lineMaxSubord);
				}
			}
		}
	}
	void changeStyleCtrlsLang(HWND hDlg, int *idArray, const char **translatedText);
};

#endif //NOTEPAD_PLUS_H
