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

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include "tinyxml.h"
//#include "ScintillaEditView.h"
#include "Scintilla.h"
#include "ScintillaRef.h"
#include "ToolBar.h"
#include "UserDefineLangReference.h"

const bool POS_VERTICAL = true;
const bool POS_HORIZOTAL = false;

const int UDD_SHOW   = 1; // 0000 0001 
const int UDD_DOCKED = 2; // 0000 0010

// 0 : 0000 0000 hide & undocked
// 1 : 0000 0001 show & undocked
// 2 : 0000 0010 hide & docked
// 3 : 0000 0011 show & docked

const int TAB_DRAWTOPBAR = 1;      // 0001
const int TAB_DRAWINACTIVETAB = 2; // 0010
const int TAB_DRAGNDROP = 4;       // 0100
const int TAB_REDUCE = 8;	       // 1000

enum LangType {L_TXT, L_PHP , L_C, L_CPP, L_CS, L_OBJC, L_JAVA, L_RC, L_HTML, L_XML, L_MAKEFILE, L_PASCAL, L_BATCH, L_INI, L_NFO,\
               L_USER, L_ASP, L_SQL, L_VB, L_JS, L_CSS, L_PERL, L_PYTHON, L_LUA, L_TEX, L_FORTRAN, L_BASH, L_FLASH, L_NSIS};

const int LANG_INDEX_INSTR = 0;
const int LANG_INDEX_INSTR2 = 1;
const int LANG_INDEX_TYPE = 2;
const int LANG_INDEX_TYPE2 = 3;

const bool SCIV_PRIMARY = false;
const bool SCIV_SECOND = true;

const char fontSizeStrs[][3] = {"", "8", "9", "10", "11", "12", "14", "16", "18", "20", "22", "24", "26", "28"};

const int MAX_LEN = 256;

static int strVal(const char *str, int base) {
	if (!str) return -1;
	if (!str[0]) return 0;

    char *finStr;
    int result = strtol(str, &finStr, base);
    if (*finStr != '\0')
        return -1;
    return result;
};

static int decStrVal(const char *str) {
    return strVal(str, 10);
};

static int hexStrVal(const char *str) {
    return strVal(str, 16);
};


static int getKwClassFromName(const char *str) {
	if (!strcmp("instre1", str)) return LANG_INDEX_INSTR;
	if (!strcmp("instre2", str)) return LANG_INDEX_INSTR2;
	if (!strcmp("type1", str)) return LANG_INDEX_TYPE;
	if (!strcmp("type2", str)) return LANG_INDEX_TYPE2;

	return -1;
};

const int FONTSTYLE_BOLD = 1;
const int FONTSTYLE_ITALIC = 2;
const int FONTSTYLE_UNDERLINE = 4;

//#include "colors.h"
struct Style
{
	int _styleID;
    const char *_styleDesc;

	COLORREF _fgColor;
	COLORREF _bgColor;
	const char *_fontName;
	int _fontStyle;
	int _fontSize;

	int _keywordClass;
	std::string *_keywords;

	Style():_styleID(-1), _fgColor(-1), _bgColor(-1), _fontName(NULL), _fontStyle(-1), _fontSize(-1), _keywordClass(-1), _keywords(NULL){};

	~Style(){
		if (_keywords) 
			delete _keywords;
	};

	Style(const Style & style) 
	{
		_styleID = style._styleID;
		_styleDesc = style._styleDesc;
		_fgColor = style._fgColor;
		_bgColor = style._bgColor;
		_fontName = style._fontName;
		_fontSize = style._fontSize;
		_fontStyle = style._fontStyle;
		_keywordClass = style._keywordClass;
		if (style._keywords)
			_keywords = new std::string(*(style._keywords));
		else
			_keywords = NULL;
	};

	Style & operator=(const Style & style) {
		if (this != &style)
		{
			this->_styleID = style._styleID;
			this->_styleDesc = style._styleDesc;
			this->_fgColor = style._fgColor;
			this->_bgColor = style._bgColor;
			this->_fontName = style._fontName;
			this->_fontSize = style._fontSize;
			this->_fontStyle = style._fontStyle;
			this->_keywordClass = style._keywordClass;

			if (!(this->_keywords) && style._keywords)
				this->_keywords = new std::string(*(style._keywords));
			else if (this->_keywords && style._keywords)
				this->_keywords->assign(*(style._keywords));
			else if (this->_keywords && !(style._keywords))
			{
				delete (this->_keywords);
				this->_keywords = NULL;
			}
		}
		return *this;
	};

	void setKeywords(const char *str) {
		if (!_keywords)
			_keywords = new std::string(str);
		else
			*_keywords = str;
	};
};

const int MAX_STYLE = 30;

struct StyleArray
{
public:
    StyleArray() : _nbStyler(0){};

    StyleArray & operator=(const StyleArray & sa)
    {
        if (this != &sa)
        {
            this->_nbStyler = sa._nbStyler;
            for (int i = 0 ; i < _nbStyler ; i++)
            {
                this->_styleArray[i] = sa._styleArray[i];
            }
        }
        return *this;
    }

    int getNbStyler() {return _nbStyler;};
    Style & getStyler(int index) {return _styleArray[index];};

    bool hasEnoughSpace() {return (_nbStyler < MAX_STYLE);};
    void addStyler(int styleID, TiXmlNode *styleNode);

	void addStyler(int styleID, char *styleName) {
		_styleArray[_nbStyler]._styleID = styleID;
		_styleArray[_nbStyler]._styleDesc = styleName;
		_nbStyler++;
	};

    int getStylerIndexByID(int id) {
        for (int i = 0 ; i < _nbStyler ; i++)
            if (_styleArray[i]._styleID == id)
                return i;
        return -1;
    };

    int getStylerIndexByName(const char *name) const {
		if (!name)
			return -1;
        for (int i = 0 ; i < _nbStyler ; i++)
			if (!strcmp(_styleArray[i]._styleDesc, name))
                return i;
        return -1;
    };

protected:
	Style _styleArray[MAX_STYLE];
	int _nbStyler;
};

struct LexerStyler : public StyleArray
{
public :
    LexerStyler():StyleArray(){};

    LexerStyler & operator=(const LexerStyler & ls)
    {
        if (this != &ls)
        {
            *((StyleArray *)this) = ls;
            strcpy(this->_lexerName, ls._lexerName);
			strcpy(this->_lexerDesc, ls._lexerDesc);
			strcpy(this->_lexerUserExt, ls._lexerUserExt);
        }
        return *this;
    }

    void setLexerName(const char *lexerName) {
        strcpy(_lexerName, lexerName);
    };
	
	void setLexerDesc(const char *lexerDesc) {
        strcpy(_lexerDesc, lexerDesc);
    };

	void setLexerUserExt(const char *lexerUserExt) {
        strcpy(_lexerUserExt, lexerUserExt);
    };

    const char * getLexerName() const {return _lexerName;};
	const char * getLexerDesc() const {return _lexerDesc;};
    const char * getLexerUserExt() const {return _lexerUserExt;};
private :
	char _lexerName[16];
	char _lexerDesc[32];
	char _lexerUserExt[256];

};

const int MAX_LEXER_STYLE = 50;

struct LexerStylerArray
{
public :
	LexerStylerArray() : _nbLexerStyler(0){};

    LexerStylerArray & operator=(const LexerStylerArray & lsa)
    {
        if (this != &lsa)
        {
            this->_nbLexerStyler = lsa._nbLexerStyler;
            for (int i = 0 ; i < this->_nbLexerStyler ; i++)
                this->_lexerStylerArray[i] = lsa._lexerStylerArray[i];
        }
        return *this;
    }

    int getNbLexer() const {return _nbLexerStyler;};

    LexerStyler & getLexerFromIndex(int index)
    {
        return _lexerStylerArray[index];
    };

    const char * getLexerNameFromIndex(int index) const {return _lexerStylerArray[index].getLexerName();}
	const char * getLexerDescFromIndex(int index) const {return _lexerStylerArray[index].getLexerDesc();}

    LexerStyler * getLexerStylerByName(const char *lexerName) {
		if (!lexerName) return NULL;
        for (int i = 0 ; i < _nbLexerStyler ; i++)
        {
            if (!strcmp(_lexerStylerArray[i].getLexerName(), lexerName))
                return &(_lexerStylerArray[i]);
        }
        return NULL;
    };
    bool hasEnoughSpace() {return (_nbLexerStyler < MAX_LEXER_STYLE);};
    void addLexerStyler(const char *lexerName, const char *lexerDesc, const char *lexerUserExt, TiXmlNode *lexerNode);

private :
	LexerStyler _lexerStylerArray[MAX_LEXER_STYLE];
	int _nbLexerStyler;
};

struct NppGUI
{
	NppGUI() : _toolBarStatus(TB_LARGE), _statusBarShow(true), \
		       _tabStatus(TAB_DRAWTOPBAR | TAB_DRAWINACTIVETAB | TAB_DRAGNDROP),\
	           _splitterPos(POS_HORIZOTAL), _userDefineDlgStatus(UDD_DOCKED), _tabSize(8),\
			   _tabReplacedBySpace(false) {
		_appPos.left = 0;
		_appPos.top = 0;
		_appPos.right = 700;
		_appPos.bottom = 500;
	};
	toolBarStatusType _toolBarStatus;		// small, large ou hide
	bool _statusBarShow;		// show ou hide

	// 1er  bit : draw top bar; 
	// 2nd  bit : draw inactive tabs
	// 3eme bit : enable drag&drop
	// 4eme bit : reduce the height
	// 0:don't draw; 1:draw top bar 2:draw inactive tabs 3:draw both 7:draw both+drag&drop
	int _tabStatus;

	bool _splitterPos;			// horizontal ou vertical
	int _userDefineDlgStatus;	// (hide||show) && (docked||undocked)

	int _tabSize;
	bool _tabReplacedBySpace;

	RECT _appPos;

	void setTabReplacedBySpace(bool b) {_tabReplacedBySpace = b;};
	
};

struct ScintillaViewParams
{
	ScintillaViewParams() : _lineNumberMarginShow(true), _bookMarkMarginShow(true), \
		                    _currentLineHilitingShow(true), _folderStyle(FOLDER_STYLE_BOX), \
	                        _indentGuideLineShow(true), _doWrap(false){};
	bool _lineNumberMarginShow;
	bool _bookMarkMarginShow;
	folderStyle  _folderStyle; //"simple", "arrow", "circle" and "box"
	bool _indentGuideLineShow;
	bool _currentLineHilitingShow;
	bool _doWrap;
	int _edgeMode;
	//int _edgeNbColumn;
};

const int NB_LIST = 20;
const int NB_MAX_LRF_FILE = 30;
const int NB_MAX_USER_LANG = 30;
const int LANG_NAME_LEN = 32;

struct Lang
{
	LangType _langID;
	char _langName[LANG_NAME_LEN];
	const char *_defaultExtList;
	const char *_langKeyWordList[NB_LIST];

	Lang() {for (int i = 0 ; i < NB_LIST ; _langKeyWordList[i] = NULL ,i++);};
	Lang(LangType langID, const char *name) : _langID(langID){
		_langName[0] = '\0';
		if (name)
			strcpy(_langName, name);
		for (int i = 0 ; i < NB_LIST ; _langKeyWordList[i] = NULL ,i++);
	};
	~Lang() {};
	void setDefaultExtList(const char *extLst){
		_defaultExtList = extLst;
	};
	
	const char * getDefaultExtList() const {
		return _defaultExtList;
	};
	
	void setWords(const char *words, int index) {
		_langKeyWordList[index] = words;
	};

	const char * getWords(int index) const {
		return _langKeyWordList[index];
	};

	LangType getLangID() const {return _langID;};
	const char * getLangName() const {return _langName;};
};

class UserLangContainer
{
friend class ScintillaEditView;
friend class NppParameters;

friend class SharedParametersDialog;
friend class FolderStyleDialog;
friend class KeyWordsStyleDialog;
friend class CommentStyleDialog;
friend class SymbolsStyleDialog;
friend class UserDefineDialog;

public :
	UserLangContainer(){
		strcpy(_name, "new user define");
		_ext[0] = '\0';

		for (int i = 0 ; i < nbKeywodList ; i++)
			*_keywordLists[i] = '\0';
	};
	UserLangContainer(const char *name, const char *ext){
		strcpy(_name, name);
		strcpy(_ext, ext);
		for (int i = 0 ; i < nbKeywodList ; i++)
			*_keywordLists[i] = '\0';
	};

	UserLangContainer & operator=(const UserLangContainer & ulc) {
		if (this != &ulc)
        {
			strcpy(this->_name, ulc._name);
			strcpy(this->_ext, ulc._ext);
			this->_styleArray = ulc._styleArray;
			for (int i = 0 ; i < nbKeywodList ; i++)
				strcpy(this->_keywordLists[i], ulc._keywordLists[i]);
		}
		return *this;
	};

	int getNbKeywordList() {return nbKeywodList;};
	char * getName() {return _name;};

private:
	char _name[langNameLenMax];
	char _ext[extsLenMax];

	StyleArray _styleArray;
	char _keywordLists[nbKeywodList][max_char];
};

const int NB_LANG = 32;

const bool DUP = true;
const bool FREE = false;

class NppParameters 
{
public:
    static NppParameters * getInstance() {return _pSelf;};
	static LangType getLangIDFromStr(const char *langName);
    void destroyInstance(){
		if (_pXmlDoc != NULL)
		{
			delete _pXmlDoc;
		}

		if (_pXmlUserDoc != NULL)
		{
			_pXmlUserDoc->SaveFile();
			delete _pXmlUserDoc;
		}
		if (_pXmlUserStylerDoc)
			delete _pXmlUserStylerDoc;

		if (_pXmlUserLangDoc)
			delete _pXmlUserLangDoc;

		if (_pXmlNativeLangDoc)
			delete _pXmlNativeLangDoc;

		if (_pXmlToolIconsDoc)
			delete _pXmlToolIconsDoc;

		delete _pSelf;
	};

	const NppGUI & getNppGUI() const {
        return _nppGUI;
    };

    const char * getWordList(LangType langID, int typeIndex) const {
    	Lang *pLang = getLangFromID(langID);
	    if (!pLang) return NULL;

        return pLang->getWords(typeIndex);
    };

	Lang * getLangFromID(LangType langID) const {
		for (int i = 0 ; i < _nbLang ; i++)
		{
			if ((_langList[i]->_langID == langID) || (!_langList[i]))
				return _langList[i];
		}
		return NULL;
	};

	Lang * getLangFromIndex(int i) const {
		if (i >= _nbLang) return NULL;
		return _langList[i];
	};

	const char * getLangExtFromName(const char *langName) const {
		for (int i = 0 ; i < _nbLang ; i++)
		{
			if (!strcmp(_langList[i]->_langName, langName))
				return _langList[i]->_defaultExtList;
		}
		return NULL;
	};

	int getNbLRFile() const {return _nbFile;};

	std::string *getLRFile(int index) const {
		return _LRFileList[index];
	};

	void setNbMaxFile(int nb) {
		_nbMaxFile = nb;
	};

	void setNbColumnEdge(int nb) {
		_nbColumnEdge = nb;
	};

	int getNbMaxFile() const {return _nbMaxFile;};
	int getNbColumnEdge() const {
		return _nbColumnEdge;
	};

    const ScintillaViewParams & getSVP(bool whichOne) const {
        return _svp[whichOne];
    };

	void writeNbHistoryFile(int nb) {
		TiXmlNode *historyNode = (_pXmlUserDoc->FirstChild("NotepadPlus"))->FirstChildElement("History");
		(historyNode->ToElement())->SetAttribute("nbMaxFile", nb);
	};

	void writeHistory(const char *fullpath);

	TiXmlNode * getChildElementByAttribut(TiXmlNode *pere, const char *childName, 
											 const char *attributName, const char *attributVal) const {
		for (TiXmlNode *childNode = pere->FirstChildElement(childName);
			childNode ;
			childNode = childNode->NextSibling(childName))
		{
			TiXmlElement *element = childNode->ToElement();
			const char *val = element->Attribute(attributName);
			if (val)
			{
				if (!strcmp(val, attributVal))
					return childNode;
			}
		}
		return NULL;
	};

	void writeScintillaParams(const ScintillaViewParams & svp, bool whichOne) {
		const char *pViewName = (whichOne == SCIV_PRIMARY)?"ScintillaPrimaryView":"ScintillaSecondaryView";
		TiXmlNode *configsRoot = (_pXmlUserDoc->FirstChild("NotepadPlus"))->FirstChildElement("GUIConfigs");
		TiXmlNode *scintNode = getChildElementByAttribut(configsRoot, "GUIConfig", "name", pViewName);
		if (scintNode)
		{
			(scintNode->ToElement())->SetAttribute("lineNumberMargin", svp._lineNumberMarginShow?"show":"hide");
			(scintNode->ToElement())->SetAttribute("bookMarkMargin", svp._bookMarkMarginShow?"show":"hide");
			(scintNode->ToElement())->SetAttribute("indentGuideLine", svp._indentGuideLineShow?"show":"hide");
			const char *pFolderStyleStr = (svp._folderStyle == FOLDER_STYLE_SIMPLE)?"simple":
											(svp._folderStyle == FOLDER_STYLE_ARROW)?"arrow":
												(svp._folderStyle == FOLDER_STYLE_CIRCLE)?"circle":"box";
			(scintNode->ToElement())->SetAttribute("folderMarkStyle", pFolderStyleStr);
			(scintNode->ToElement())->SetAttribute("currentLineHilitingShow", svp._currentLineHilitingShow?"show":"hide");
			(scintNode->ToElement())->SetAttribute("Wrap", svp._doWrap?"yes":"no");
			char *edgeStr = NULL;
			if (svp._edgeMode == EDGE_NONE)
				edgeStr = "no";
			else if (svp._edgeMode == EDGE_LINE)
				edgeStr = "line";
			else
				edgeStr = "background";
			(scintNode->ToElement())->SetAttribute("edge", edgeStr);
			//(scintNode->ToElement())->SetAttribute("edgeNbColumn", svp._edgeNbColumn);
		}
		else {/*create one*/}
	};

	void writeGUIParams();

	void writeStyles(LexerStylerArray & lexersStylers, StyleArray & globalStylers);

    LexerStylerArray & getLStylerArray() {return _lexerStylerArray;};
    StyleArray & getGlobalStylers() {return _widgetStyleArray;};

    StyleArray & getMiscStylerArray() {return _widgetStyleArray;};

    COLORREF getCurLineHilitingColour() {
		int i = _widgetStyleArray.getStylerIndexByName("Current line background colour");
        if (i == -1) return i;
        Style & style = _widgetStyleArray.getStyler(i);
        return style._bgColor;
    };
    void setCurLineHilitingColour(COLORREF colour2Set) {
        int i = _widgetStyleArray.getStylerIndexByName("Current line background colour");
        if (i == -1) return;
        Style & style = _widgetStyleArray.getStyler(i);
        style._bgColor = colour2Set;
    };

	void setFontList(HWND hWnd);
	const std::vector<std::string> & getFontList() const {return _fontlist;};
	
	int getNbUserLang() const {return _nbUserLang;};
	UserLangContainer & getULCFromIndex(int i) {return *_userLangArray[i];};
	UserLangContainer & getULCFromName(const char *userLangName) {
		for (int i = 0 ; i < _nbUserLang ; i++)
			if (!strcmp(userLangName, _userLangArray[i]->_name))
				return *_userLangArray[i];
		//return ;
	};
	void writeUserDefinedLang() {
		if (!_pXmlUserLangDoc)
		{
			//do the treatment
			_pXmlUserLangDoc = new TiXmlDocument(_userDefineLangPath);
		}

		//before remove the branch, we allocate and copy the char * which will be destroyed
		stylerStrOp(DUP);

		TiXmlNode *root = _pXmlUserLangDoc->FirstChild("NotepadPlus");
		if (root) 
		{
			_pXmlUserLangDoc->RemoveChild(root);
		}
		
		_pXmlUserLangDoc->InsertEndChild(TiXmlElement("NotepadPlus"));

		root = _pXmlUserLangDoc->FirstChild("NotepadPlus");

		for (int i = 0 ; i < _nbUserLang ; i++)
		{
			insertUserLang2Tree(root, _userLangArray[i]);
		}
		_pXmlUserLangDoc->SaveFile();
		stylerStrOp(FREE);
	};
	bool isExistingUserLangName(const char *newName) const {
		if ((!newName) || (!newName[0]))
			return true;

		for (int i = 0 ; i < _nbUserLang ; i++)
		{
			if (!strcmp(_userLangArray[i]->_name, newName))
				return true;
		}
		return false;
	};

	const char * getLangNameFromExt(char *ext) {
		if ((!ext) || (!ext[0]))
			return NULL;

		for (int i = 0 ; i < _nbUserLang ; i++)
		{
			if (!strcmp(_userLangArray[i]->_ext, ext))
				return _userLangArray[i]->_name;
		}
		return NULL;
	};
	int addUserLangToEnd(const UserLangContainer & userLang, const char *newName);
	void removeUserLang(int index);
	TiXmlDocument * getNativeLang() const {return _pXmlNativeLangDoc;};
	TiXmlDocument * getToolIcons() const {return _pXmlToolIconsDoc;};

	bool isTransparentAvailable() const {
		return (bool)_transparentFuncAddr;
	};

	void SetTransparent(HWND hwnd, int percent) {
		//WNDPROC transparentFunc = (NppParameters::getInstance())->getTransparentFunc();
		if (!_transparentFuncAddr) return;

		::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | /*WS_EX_LAYERED*/0x00080000);

		//bAlpha = 255 / 100 * iAlphaPercent;
				
		_transparentFuncAddr(hwnd, 0, percent, 0x00000002); 
	};

	void removeTransparent(HWND hwnd) {
		::SetWindowLong(hwnd, GWL_EXSTYLE,  ::GetWindowLong(hwnd, GWL_EXSTYLE) & ~/*WS_EX_LAYERED*/0x00080000);
	};
	
	void setDefLang(LangType langType) {_defLangType = langType;};
	LangType getDefLang() {return _defLangType;};

private:
    NppParameters();
	~NppParameters() {
		for (int i = 0 ; i < _nbLang ; i++)
			delete _langList[i];
		for (int i = 0 ; i < _nbFile ; i++)
			delete _LRFileList[i];
		for (int i = 0 ; i < _nbUserLang ; i++)
			delete _userLangArray[i];
	};
    static NppParameters *_pSelf;

	TiXmlDocument *_pXmlDoc, *_pXmlUserDoc, *_pXmlUserStylerDoc, *_pXmlUserLangDoc, *_pXmlNativeLangDoc, *_pXmlToolIconsDoc;

	NppGUI _nppGUI;
	ScintillaViewParams _svp[2];
	Lang *_langList[NB_LANG];
	int _nbLang;

	std::string *_LRFileList[NB_MAX_LRF_FILE];
	int _nbFile;
	int _nbMaxFile;
	int _nbColumnEdge;

	UserLangContainer *_userLangArray[NB_MAX_USER_LANG];
	int _nbUserLang;
	char _userDefineLangPath[MAX_LEN];

	LangType _defLangType;

    // All Styles (colours & fonts)
	LexerStylerArray _lexerStylerArray;
    StyleArray _widgetStyleArray;

	std::vector<std::string> _fontlist;

	WNDPROC _transparentFuncAddr;

	static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam) {
		std::vector<std::string> *pStrVect = (std::vector<std::string> *)lParam;
		pStrVect->push_back((char*)lpelfe->elfFullName);

		return 1; // I want to get all fonts
	};

	void getLangKeywordsFromXmlTree();
	bool getUserParametersFromXmlTree();
	bool getUserStylersFromXmlTree();
	bool getUserDefineLangsFromXmlTree();

	void feedGUIParameters(TiXmlNode *node);
	void feedKeyWordsParameters(TiXmlNode *node);
	void feedFileListParameters(TiXmlNode *node);
    void feedScintillaParam(bool whichOne, TiXmlNode *node);

    bool feedStylerArray(TiXmlNode *node);
    void getAllWordStyles(char *lexerName, TiXmlNode *lexerNode);

	void feedUserLang(TiXmlNode *node);
	int getIndexFromKeywordListName(const char *name);
	void feedUserStyles(TiXmlNode *node);
	void feedUserKeywordList(TiXmlNode *node);


	
	int getIndexFromStr(const char *indexName) const;
    void writeStyle2Element(Style & style2Wite, Style & style2Sync, TiXmlElement *element);

	void insertUserLang2Tree(TiXmlNode *node, UserLangContainer *userLang);
	void stylerStrOp(bool op);
	
};

#endif //PARAMETERS_H
