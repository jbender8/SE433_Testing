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

#ifndef SCINTILLA_EDIT_VIEW_H
#define SCINTILLA_EDIT_VIEW_H

#include <vector>
#include "Window.h"
#include "Scintilla.h"
#include "ScintillaRef.h"
#include "SciLexer.h"
#include "Buffer.h"
#include "colors.h"
#include "SysMsg.h"
#include "UserDefineDialog.h"

class NppParameters;

#define NB_WORD_LIST 4
#define WORD_LIST_LEN 256

typedef int (* SCINTILLA_FUNC) (void*, int, int, int);
typedef void * SCINTILLA_PTR;
typedef std::vector<Buffer> buf_vec_t;

#define DEF_SCINTILLA               1000
#define WM_DOCK_USERDEFINE_DLG      (WM_USER + DEF_SCINTILLA + 1)
#define WM_UNDOCK_USERDEFINE_DLG    (WM_USER + DEF_SCINTILLA + 2)
#define WM_CLOSE_USERDEFINE_DLG		(WM_USER + DEF_SCINTILLA + 3)
#define WM_REMOVE_USERLANG		    (WM_USER + DEF_SCINTILLA + 4)
#define WM_RENAME_USERLANG			(WM_USER + DEF_SCINTILLA + 5)

#define LINEDRAW_FONT  "LINEDRAW.TTF"
//const bool DOCK = true;
//const bool UNDOCK = false;
const int NB_FOLDER_STATE = 7;

// Codepage
const int CP_CHINESE_TRADITIONAL = 950;
const int CP_CHINESE_SIMPLIFIED = 936;
const int CP_JAPANESE = 932;
const int CP_KOREAN = 949;

//wordList
#define LIST_0 1
#define LIST_1 2
#define LIST_2 4
#define LIST_3 8

const bool dirUp = true;
const bool dirDown = false;

class ScintillaEditView : public Window
{
public:
	ScintillaEditView()
		: Window(), _pScintillaFunc(NULL),_pScintillaPtr(NULL),
		  _currentIndex(0),_MSLineDrawFont(0), _folderStyle(FOLDER_STYLE_BOX)
	{
		++_refCount;
	};

	virtual ~ScintillaEditView()
	{
		--_refCount;

		if ((!_refCount)&&(_hLib))
		{
			::FreeLibrary(_hLib);
			if (_MSLineDrawFont)
			{
				::RemoveFontResource(LINEDRAW_FONT);
			}
		}
	};
	virtual void destroy()
	{
		removeAllUnusedDocs();
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	};

	virtual void init(HINSTANCE hInst, HWND hPere);

	LRESULT execute(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const {
		return _pScintillaFunc(_pScintillaPtr, static_cast<int>(Msg), static_cast<int>(wParam), static_cast<int>(lParam));
	};
	
	void defineDocType(LangType typeDoc);

    void setCurrentDocType(LangType typeDoc) {
        if ((_buffers[_currentIndex]._lang == typeDoc) && (typeDoc != L_USER))
            return;
		if (typeDoc == L_USER)
			_buffers[_currentIndex]._userLangExt[0] = '\0';

        _buffers[_currentIndex]._lang = typeDoc;
        defineDocType(typeDoc);
    };

	void setCurrentDocUserType(const char *userLangName) {
		strcpy(_buffers[_currentIndex]._userLangExt, userLangName);
        _buffers[_currentIndex]._lang = L_USER;
        defineDocType(L_USER);
    };

	char * attatchDefaultDoc(int nb);

	int findDocIndexByName(const char *fn) const;
	char * activateDocAt(int index);
	char * createNewDoc(const char *fn);
	char * createNewDoc(int nbNew);
	int getCurrentDocIndex() const {return _currentIndex;};
	const char * getCurrentTitle() const {return _buffers[_currentIndex]._fullPathName;};
	int setCurrentTitle(const char *fn) {
		_buffers[_currentIndex].setFileName(fn);
		defineDocType(_buffers[_currentIndex]._lang);
		return _currentIndex;
	};
	int closeCurrentDoc(int & i2Activate);
    void closeDocAt(int i2Close);

	void removeAllUnusedDocs();

	void getText(char *dest, int start, int end);

    void getText(char *dest, int len) const {
	    execute(SCI_GETTEXT, len, reinterpret_cast<LPARAM>(dest));
    };
	void setCurrentDocState(bool isDirty) {
		_buffers[_currentIndex]._isDirty = isDirty;
	};
	
	bool isCurrentDocDirty() const {
		return _buffers[_currentIndex]._isDirty;
	};

    void setCurrentDocReadOnly(bool isReadOnly) {
        _buffers[_currentIndex]._isReadOnly = isReadOnly;
    };
	
    bool isCurrentBufReadOnly() {
		return _buffers[_currentIndex]._isReadOnly;
	};

	bool isAllDocsClean() const {
		for (int i = 0 ; i < static_cast<int>(_buffers.size()) ; i++)
			if (_buffers[i]._isDirty)
				return false;
		return true;
	};

	int getNbDoc() const {
		return static_cast<int>(_buffers.size());
	};

	void saveCurrentPos()
	{
		int topLine = static_cast<int>(execute(SCI_GETFIRSTVISIBLELINE));
		_buffers[_currentIndex]._pos._fistVisibleLine = topLine;

		_buffers[_currentIndex]._pos._startPos = int(execute(SCI_GETSELECTIONSTART));
		_buffers[_currentIndex]._pos._endPos = int(execute(SCI_GETSELECTIONEND));
	};

	void restoreCurrentPos(const Position & prevPos)
	{		
		int scroll2Top = 0 - (int(execute(SCI_GETLINECOUNT)) + 1);
		execute(SCI_LINESCROLL, 0, scroll2Top);
		
		execute(SCI_LINESCROLL, 0, _buffers[_currentIndex]._pos._fistVisibleLine);
		execute(SCI_SETSELECTIONSTART, _buffers[_currentIndex]._pos._startPos);
		execute(SCI_SETSELECTIONEND, _buffers[_currentIndex]._pos._endPos);
	};

	Buffer & getBufferAt(int index) {
		if (index >= int(_buffers.size()))
			throw int(3615);
		return _buffers[index];
	};

	void updateCurrentBufTimeStamp() {
		_buffers[_currentIndex].updatTimeStamp();
	};

	int getLineStrAt(int nbLine, char *buf, bool activatReturn) {
		int nbChar = int(execute(SCI_LINELENGTH, nbLine));
		execute(SCI_GETLINE, nbLine, (LPARAM)buf);
		if (!activatReturn)
			nbChar -= 2;    // eliminate '\n' (0D 0A)

		buf[nbChar] = '\0';
		return nbChar;
	};

	int getCurrentDocLen() const {
		return int(execute(SCI_GETLENGTH));
	};

	CharacterRange getSelection() const {
		CharacterRange crange;
		crange.cpMin = long(execute(SCI_GETSELECTIONSTART));
		crange.cpMax = long(execute(SCI_GETSELECTIONEND));
		return crange;
	};

    LangType getCurrentDocType() const {
        return _buffers[_currentIndex]._lang;
    };

    void doUserDefineDlg(bool willBeShown = true) {
        _userDefineDlg.doDialog(willBeShown);
    };

    static UserDefineDialog * getUserDefineDlg() {return &_userDefineDlg;};

    //void changeUserDefineDlgStyle() {_userDefineDlg.changeStyle();};

    void setCaretColorWidth(int color, int width = 1) const {
        execute(SCI_SETCARETFORE, color);
        execute(SCI_SETCARETWIDTH, width);
    };

    int addBuffer(Buffer & buffer) {
        _buffers.push_back(buffer);
        return (int(_buffers.size()) - 1);
    };

    Buffer & getCurrentBuffer() {
        return getBufferAt(_currentIndex);
    };

	void beSwitched() {
		_userDefineDlg.setScintilla(this);
	};

    //Marge memeber and method
    static const int _SC_MARGE_LINENUMBER;
    static const int _SC_MARGE_SYBOLE;
    static const int _SC_MARGE_FOLDER;

    static const int _MARGE_LINENUMBER_NB_CHIFFRE;

    void showMargin(int witchMarge, bool willBeShowed = true) {
        if (witchMarge == _SC_MARGE_LINENUMBER)
            setLineNumberWidth(willBeShowed);
        else
            execute(SCI_SETMARGINWIDTHN, witchMarge, willBeShowed?16:0);
    };

    bool hasMarginShowed(int witchMarge) {
        return (bool)execute(SCI_GETMARGINWIDTHN, witchMarge, 0);
    };
    
    void marginClick(int position, int modifiers);

    void setMakerStyle(folderStyle style) {
        if (_folderStyle == style)
            return;
        _folderStyle = style;
        for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
            defineMarker(_markersArray[FOLDER_TYPE][i], _markersArray[style][i], white, black);
    };

    folderStyle getFolderStyle() {return _folderStyle;};

	void showWSAndTab(bool willBeShowed = true) {
		execute(SCI_SETVIEWWS, willBeShowed?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE);
	};

	void showEOL(bool willBeShowed = true) {
		execute(SCI_SETVIEWEOL, willBeShowed);
	};

	void showInvisibleChars(bool willBeShowed = true) {
		showWSAndTab(willBeShowed);
		showEOL(willBeShowed);
	};

	bool isInvisibleCharsShown(bool willBeShowed = true) {
		return bool(execute(SCI_GETVIEWWS));
	};

	void showIndentGuideLine(bool willBeShowed = true) {
		execute(SCI_SETINDENTATIONGUIDES, (WPARAM)willBeShowed);
	};

	bool isShownIndentGuide() {
		return bool(execute(SCI_GETINDENTATIONGUIDES));
	};
    void wrap(bool willBeWrapped = true) {
        execute(SCI_SETWRAPMODE, (WPARAM)willBeWrapped);
    };

    bool isWrap() {
        return (execute(SCI_GETWRAPMODE) == SC_WRAP_WORD);
    };

    void sortBuffer(int destIndex, int scrIndex) {
		// Do nothing if there's no change of the position
		if (scrIndex == destIndex)
			return;

        Buffer buf2Insert = _buffers.at(scrIndex);
        _buffers.erase(_buffers.begin() + scrIndex);
        _buffers.insert(_buffers.begin() + destIndex, buf2Insert);
    };

	int getSelectedTextCount() {
		CharacterRange range = getSelection();
		return (range.cpMax - range.cpMin);
	};

	char * getSelectedText(char * txt, int size) {
		CharacterRange range = getSelection();
		if (size < (range.cpMax - range.cpMin))
			return NULL;
		getText(txt, range.cpMin, range.cpMax);
		return txt;
	};

	int getCurrentLineNumber()const {
		return execute(SCI_LINEFROMPOSITION, execute(SCI_GETCURRENTPOS));
	};

	int getCurrentColumnNumber() const {
        return execute(SCI_GETCOLUMN, execute(SCI_GETCURRENTPOS));
    };

	int getSelectedByteNumber() const {
		int start = execute(SCI_GETSELECTIONSTART);
		int end = execute(SCI_GETSELECTIONEND);
		return (start < end)?end-start:start-end;
    };

	int getLineLength(int line) const {
		return execute(SCI_GETLINEENDPOSITION, line) - execute(SCI_POSITIONFROMLINE, line);
	};

	int getLineIndent(int line) const {
		return execute(SCI_GETLINEINDENTATION, line);
	};

	void setLineIndent(int line, int indent) const;

    void setLineNumberWidth(bool willBeShowed = true) {
        // The 4 here allows for spacing: 1 poxel on left and 3 on right.
        int pixelWidth = (willBeShowed)?(8 + _MARGE_LINENUMBER_NB_CHIFFRE * execute(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8")):0;
        execute(SCI_SETMARGINWIDTHN, 0, pixelWidth);
    };

    void setCurrentIndex(int index2Set) {_currentIndex = index2Set;};

	void setCurrentLineHiLiting(bool isHiliting, COLORREF bgColor) const {
		execute(SCI_SETCARETLINEVISIBLE, isHiliting);
		if (!isHiliting)
			return;
		execute(SCI_SETCARETLINEBACK, bgColor);
	};

	bool isCurrentLineHiLiting() const {
		return execute(SCI_GETCARETLINEVISIBLE);
	};

	inline void makeStyle(const char *lexerName, const char **keywordArray = NULL);

	void performGlobalStyles();

	void expand(int &line, bool doExpand, bool force = false, int visLevels = 0, int level = -1);
	void removeUserLang(const char *name) {
		for (int i = 0 ; i < _buffers.size() ; i++)
		{
			if ((_buffers[i]._lang == L_USER) && (!strcmp(name, _buffers[i]._userLangExt)))
			{
				_buffers[i]._userLangExt[0] = '\0';
			}
		}
	};
	void renameUserLang(const char *oldName, const char *newName) {
		for (int i = 0 ; i < _buffers.size() ; i++)
		{
			if ((_buffers[i]._lang == L_USER) && (!strcmp(oldName, _buffers[i]._userLangExt)))
			{
				strcpy(_buffers[i]._userLangExt, newName);
			}
		}
	};

	
		
	void currentLineUp() const {
		int currentLine = getCurrentLineNumber();
		if (currentLine == 0)
			return;
		currentLine--;
		execute(SCI_LINETRANSPOSE);
		execute(SCI_GOTOLINE, currentLine);
	};

	void currentLineDown() const {
		int currentLine = getCurrentLineNumber();
		if (currentLine == (execute(SCI_GETLINECOUNT) - 1))
			return;
		currentLine++;
		execute(SCI_GOTOLINE, currentLine);
		execute(SCI_LINETRANSPOSE);
	};

private:
	static HINSTANCE _hLib;
	static int _refCount;
    static UserDefineDialog _userDefineDlg;

    static const int _markersArray[][NB_FOLDER_STATE];

	SCINTILLA_FUNC _pScintillaFunc;
	SCINTILLA_PTR  _pScintillaPtr;

	// the current active buffer index of _buffers
	int _currentIndex;

	// the list of docs
	buf_vec_t _buffers;

	// For the file nfo
	int _MSLineDrawFont;	
	folderStyle _folderStyle;

    NppParameters *_pParameter;

	int _codepage;

	//void setStyle(int whichStyle, COLORREF fore, COLORREF back=white, const char *face=0, int size=-1) const;
	//void setFont(int which, const char *fontName, bool isBold=false, bool isItalic=false) const;
    void setStyle(int styleID, COLORREF fgColor, COLORREF bgColor = -1, const char *fontName = NULL, int fontStyle = -1, int fontSize = 0);
    void setCppLexer(LangType type);
	void setXmlLexer(LangType type);
	void setUserLexer();
	void setUserLexer(const char *userLangName);
	void setEmbeddedJSLexer();
    void setPhpEmbeddedLexer();
    void setEmbeddedAspLexer();
	void setCssLexer() {
		setLexer(SCLEX_CSS, L_CSS, "css", LIST_0);
	};

	void setLuaLexer() {
		setLexer(SCLEX_LUA, L_LUA, "lua", LIST_0 | LIST_1 | LIST_2 | LIST_3);
	};

	void setMakefileLexer() {
		execute(SCI_SETLEXER, SCLEX_MAKEFILE);
		makeStyle("makefile");
	};

	void setIniLexer() {
		execute(SCI_SETLEXER, SCLEX_PROPERTIES);
		execute(SCI_STYLESETEOLFILLED, SCE_PROPS_SECTION, true);
		makeStyle("ini");
	};

    void setObjCLexer(LangType type);

	void setSqlLexer() {
		setLexer(SCLEX_SQL, L_SQL, "sql", LIST_0);
	};

	void setBashLexer() {
		setLexer(SCLEX_BASH, L_BASH, "bash", LIST_0);
	};

	void setVBLexer() {
		setLexer(SCLEX_VB, L_VB, "vb", LIST_0);
	};

	void setPascalLexer() {
		setLexer(SCLEX_PASCAL, L_PASCAL, "pascal", LIST_0);
	};

	void setPerlLexer() {
		setLexer(SCLEX_PERL, L_PERL, "perl", LIST_0);
	};

	void setPythonLexer() {
		setLexer(SCLEX_PYTHON, L_PYTHON, "python", LIST_0);
	};

	void setBatchLexer() {
		setLexer(SCLEX_BATCH, L_BATCH, "batch", LIST_0);
	};

	void setTeXLexer() {
		execute(SCI_SETLEXER, SCLEX_TEX);
		makeStyle("tex");
	};

	void setNsisLexer() {
		setLexer(SCLEX_NSIS, L_NSIS, "nsis", LIST_0 | LIST_1 | LIST_2 | LIST_3);
	};

	void setFortranLexer() {
		setLexer(SCLEX_F77, L_FORTRAN, "fortran", LIST_0 | LIST_1 | LIST_2);
	};

    void defineMarker(int marker, int markerType, COLORREF fore, COLORREF back) {
	    execute(SCI_MARKERDEFINE, marker, markerType);
	    execute(SCI_MARKERSETFORE, marker, fore);
	    execute(SCI_MARKERSETBACK, marker, back);
    };

	bool isNeededFolderMarge(LangType typeDoc) const {
		switch (typeDoc)
		{
			case L_NFO:
			case L_BATCH:
			case L_TXT:
			case L_MAKEFILE:
            case L_SQL:
			case L_TEX:
				return false;
			default:
				return true;
		}
	};
	bool isCJK() const {
		return ((_codepage == CP_CHINESE_TRADITIONAL) || (_codepage == CP_CHINESE_SIMPLIFIED) || 
			    (_codepage == CP_JAPANESE) || (_codepage == CP_KOREAN));
	};

	int codepage2CharSet() const {
		switch (_codepage)	
		{
			case CP_CHINESE_TRADITIONAL : return SC_CHARSET_CHINESEBIG5;
			case CP_CHINESE_SIMPLIFIED : return SC_CHARSET_GB2312;
			case CP_KOREAN : return SC_CHARSET_HANGUL;
			case CP_JAPANESE : return SC_CHARSET_SHIFTJIS;
			default : return 0;
		}
	};
	
	const char * getCompleteKeywordList(std::string & kwl, LangType langType, int keywordIndex);
	//void setLexer(int lexID, LangType langType, const char *lexName, int kwIndex);
	void setLexer(int lexerID, LangType langType, const char *lexerName, int whichList);
};

#endif //SCINTILLA_EDIT_VIEW_H
