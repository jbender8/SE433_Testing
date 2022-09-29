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

#include <windows.h>
#include <ShellAPI.h>
#include "ScintillaEditView.h"
#include "Parameters.h"


// initialize the static variable
HINSTANCE ScintillaEditView::_hLib = ::LoadLibrary("SciLexer.DLL");
int ScintillaEditView::_refCount = 0;
UserDefineDialog ScintillaEditView::_userDefineDlg;

const int ScintillaEditView::_SC_MARGE_LINENUMBER = 0;
const int ScintillaEditView::_SC_MARGE_SYBOLE = 1;
const int ScintillaEditView::_SC_MARGE_FOLDER = 2;

const int ScintillaEditView::_MARGE_LINENUMBER_NB_CHIFFRE = 5;

/*
SC_MARKNUM_*     | Arrow               Plus/minus           Circle tree                 Box tree 
-------------------------------------------------------------------------------------------------------------
FOLDEROPEN       | SC_MARK_ARROWDOWN   SC_MARK_MINUS     SC_MARK_CIRCLEMINUS            SC_MARK_BOXMINUS 
FOLDER           | SC_MARK_ARROW       SC_MARK_PLUS      SC_MARK_CIRCLEPLUS             SC_MARK_BOXPLUS 
FOLDERSUB        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_VLINE                  SC_MARK_VLINE 
FOLDERTAIL       | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_LCORNERCURVE           SC_MARK_LCORNER 
FOLDEREND        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEPLUSCONNECTED    SC_MARK_BOXPLUSCONNECTED 
FOLDEROPENMID    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEMINUSCONNECTED   SC_MARK_BOXMINUSCONNECTED 
FOLDERMIDTAIL    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_TCORNERCURVE           SC_MARK_TCORNER 
*/

const int ScintillaEditView::_markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND, SC_MARKNUM_FOLDEROPENMID, SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS, SC_MARK_PLUS, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN, SC_MARK_ARROW, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS, SC_MARK_CIRCLEPLUS, SC_MARK_VLINE, SC_MARK_LCORNERCURVE, SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS, SC_MARK_BOXPLUS, SC_MARK_VLINE, SC_MARK_LCORNER, SC_MARK_BOXPLUSCONNECTED, SC_MARK_BOXMINUSCONNECTED, SC_MARK_TCORNER}
};

//const int MASK_RED   = 0xFF0000;
//const int MASK_GREEN = 0x00FF00;
//const int MASK_BLUE  = 0x0000FF;

void ScintillaEditView::init(HINSTANCE hInst, HWND hPere)
{
	if (!_hLib)
	{
		MessageBox( NULL, "can't not load the dynamic library", "SYS ERR : ", MB_OK | MB_ICONSTOP);
		throw int(106901);
	}

	Window::init(hInst, hPere);
   _hSelf = ::CreateWindowEx(
					WS_EX_CLIENTEDGE,\
					"Scintilla",\
					"Notepad++",\
					WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,\
					0, 0, 100, 100,\
					_hParent,\
					NULL,\
					_hInst,\
					NULL);
/*WS_EX_RTLREADING*/
	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(106901);
	}

	_pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(_hSelf, SCI_GETDIRECTFUNCTION, 0, 0);
	_pScintillaPtr = (SCINTILLA_PTR)::SendMessage(_hSelf, SCI_GETDIRECTPOINTER, 0, 0);

    _userDefineDlg.init(_hInst, _hParent, this);

	if (!_pScintillaFunc || !_pScintillaPtr)
	{
		systemMessage("System Err");
		throw int(106901);
	}

    execute(SCI_SETMARGINMASKN, _SC_MARGE_FOLDER, SC_MASK_FOLDERS);
    //execute(SCI_SETMARGINWIDTHN, _SC_MARGE_FOLDER, 16);
    showMargin(_SC_MARGE_FOLDER, true);

	//execute(SCI_SETFOLDMARGINCOLOUR, true, red);
	execute(SCI_SETFOLDMARGINHICOLOUR, false, red);

    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_FOLDER, true);
    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_SYBOLE, true);

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));
	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETFOLDFLAGS, 16, 0);

	for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
        defineMarker(_markersArray[FOLDER_TYPE][i], _markersArray[_folderStyle][i], white, black);

	_codepage = ::GetACP();
    _pParameter = NppParameters::getInstance();

};

void ScintillaEditView::setStyle(int styleID, COLORREF fgColour, COLORREF bgColour, const char *fontName, int fontStyle, int fontSize)
{
    if (!((fgColour >> 24) & 0xFF))
	    execute(SCI_STYLESETFORE, styleID, fgColour);

    if (!((bgColour >> 24) & 0xFF))
	    execute(SCI_STYLESETBACK, styleID, bgColour);
    
    if ((!fontName)||(strcmp(fontName, "")))
		execute(SCI_STYLESETFONT, (WPARAM)styleID, (LPARAM)fontName);

    if ((fontStyle != -1) && (fontStyle != 0))
    {
        if (fontStyle & FONTSTYLE_BOLD)
            execute(SCI_STYLESETBOLD, (WPARAM)styleID, (LPARAM)true);
        if (fontStyle & FONTSTYLE_ITALIC)
            execute(SCI_STYLESETITALIC, (WPARAM)styleID, (LPARAM)true);
        if (fontStyle & FONTSTYLE_UNDERLINE)
            execute(SCI_STYLESETUNDERLINE, (WPARAM)styleID, (LPARAM)true);
    }

	if (fontSize > 0)
		execute(SCI_STYLESETSIZE, styleID, fontSize);

}


void ScintillaEditView::setXmlLexer(LangType type)
{

    execute(SCI_SETSTYLEBITS, 7, 0);

	if (type == L_XML)
	{
        execute(SCI_SETLEXER, SCLEX_HTML);
		for (int i = 0 ; i < 4 ; i++)
			execute(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(""));

        makeStyle("xml");
	}
	else if ((type == L_HTML) || (type == L_PHP) || (type == L_ASP))
	{

        execute(SCI_SETLEXER, SCLEX_XML);

        const char *htmlKeyWords =_pParameter->getWordList(L_HTML, LANG_INDEX_INSTR);
        execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(htmlKeyWords?htmlKeyWords:""));

		makeStyle("html");
		
        setEmbeddedJSLexer();
        setPhpEmbeddedLexer();
		setEmbeddedAspLexer();
	}

}

void ScintillaEditView::setEmbeddedJSLexer()
{
	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle("javascript", pKwArray);

	std::string keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])
		keywordList = pKwArray[LANG_INDEX_INSTR];

	execute(SCI_SETKEYWORDS, 1, (LPARAM)getCompleteKeywordList(keywordList, L_JS, LANG_INDEX_INSTR));

	execute(SCI_STYLESETEOLFILLED, SCE_HJ_DEFAULT, true);
	execute(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENT, true);
	execute(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENTDOC, true);
}

void ScintillaEditView::setPhpEmbeddedLexer()
{
	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle("php", pKwArray);

	std::string keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])
		keywordList = pKwArray[LANG_INDEX_INSTR];

	execute(SCI_SETKEYWORDS, 4, (LPARAM)getCompleteKeywordList(keywordList, L_PHP, LANG_INDEX_INSTR));

	execute(SCI_STYLESETEOLFILLED, SCE_HPHP_DEFAULT, true);
	execute(SCI_STYLESETEOLFILLED, SCE_HPHP_COMMENT, true);
}

void ScintillaEditView::setEmbeddedAspLexer()
{
	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle("asp", pKwArray);

	std::string keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])
		keywordList = pKwArray[LANG_INDEX_INSTR];

	execute(SCI_SETKEYWORDS, 2, (LPARAM)getCompleteKeywordList(keywordList, L_VB, LANG_INDEX_INSTR));

    execute(SCI_STYLESETEOLFILLED, SCE_HBA_DEFAULT, true);
}

void ScintillaEditView::setUserLexer()
{
    execute(SCI_SETLEXER, SCLEX_USER);
	UserLangContainer & userLangContainer = *(_userDefineDlg._pCurrentUserLang);

	for (int i = 0 ; i < userLangContainer.getNbKeywordList() ; i++)
	{
		execute(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(userLangContainer._keywordLists[i]));
	}

	for (int i = 0 ; i < userLangContainer._styleArray.getNbStyler() ; i++)
	{
		Style & style = userLangContainer._styleArray.getStyler(i);
		setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
	}
}

void ScintillaEditView::setUserLexer(const char *userLangName)
{
	
    execute(SCI_SETLEXER, SCLEX_USER);

	UserLangContainer & userLangContainer = NppParameters::getInstance()->getULCFromName(userLangName);
		//getULCFromName(userLangName);

	for (int i = 0 ; i < userLangContainer.getNbKeywordList() ; i++)
	{
		execute(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(userLangContainer._keywordLists[i]));
	}

	for (int i = 0 ; i < userLangContainer._styleArray.getNbStyler() ; i++)
	{
		Style & style = userLangContainer._styleArray.getStyler(i);
		setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
	}
}

void ScintillaEditView::setCppLexer(LangType langType)
{
    const char *cppInstrs;
    const char *cppTypes;
    const char *doxygenKeyWords  = _pParameter->getWordList(L_CPP, LANG_INDEX_TYPE2);

	char *lexerName;
	switch (langType)
	{
		case L_C:
			lexerName = "c"; break;

		case L_CPP:
			lexerName = "cpp"; break;

		case L_JAVA:
			lexerName = "java"; break;

		case L_JS:
			lexerName = "javascript"; break;

		case L_RC:
			lexerName = "rc"; break;

		case L_CS:
			lexerName = "cs"; break;

		default:
			return;
	}

    execute(SCI_SETLEXER, SCLEX_CPP); 
	if (isCJK())
	{
		int charSet = codepage2CharSet();
		if (charSet)
			execute(SCI_STYLESETCHARACTERSET, SCE_C_STRING, charSet);
	}

	if ((langType != L_RC) && (langType != L_JS))
    {
        if (doxygenKeyWords)
            execute(SCI_SETKEYWORDS, 2, (LPARAM)doxygenKeyWords);
    }

	if (langType == L_JS)
	{
		LexerStyler *pStyler = (_pParameter->getLStylerArray()).getLexerStylerByName("javascript");	
		if (pStyler)
		{
			for (int i = 0 ; i < pStyler->getNbStyler() ; i++)
			{
				Style & style = pStyler->getStyler(i);
				int cppID = style._styleID; 
				switch (style._styleID)
				{
					case SCE_HJ_DEFAULT : cppID = SCE_C_DEFAULT; break;
					case SCE_HJ_WORD : cppID = SCE_C_IDENTIFIER; break;
					case SCE_HJ_SYMBOLS : cppID = SCE_C_OPERATOR; break;
					case SCE_HJ_COMMENT : cppID = SCE_C_COMMENT; break;
					case SCE_HJ_COMMENTLINE : cppID = SCE_C_COMMENTLINE; break;
					case SCE_HJ_COMMENTDOC : cppID = SCE_C_COMMENTDOC; break;
					case SCE_HJ_NUMBER : cppID = SCE_C_NUMBER; break;
					case SCE_HJ_KEYWORD : cppID = SCE_C_WORD; break;
					case SCE_HJ_DOUBLESTRING : cppID = SCE_C_STRING; break;
					case SCE_HJ_SINGLESTRING : cppID = SCE_C_CHARACTER; break;
					case SCE_HJ_REGEX : cppID = SCE_C_REGEX; break;
				}
				setStyle(cppID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
			}
		}
		execute(SCI_STYLESETEOLFILLED, SCE_C_DEFAULT, true);
		execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENTLINE, true);
		execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENT, true);
		execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENTDOC, true);
	}

	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(lexerName, pKwArray);

	std::string instr1("");
	if (pKwArray[LANG_INDEX_INSTR])
		instr1 = pKwArray[LANG_INDEX_INSTR];
	cppInstrs = getCompleteKeywordList(instr1, langType, LANG_INDEX_INSTR);

	std::string type1("");
	if (pKwArray[LANG_INDEX_TYPE])
		type1 = pKwArray[LANG_INDEX_TYPE];
	cppTypes = getCompleteKeywordList(type1, langType, LANG_INDEX_TYPE);

	execute(SCI_SETKEYWORDS, 0, (LPARAM)cppInstrs);
	execute(SCI_SETKEYWORDS, 1, (LPARAM)cppTypes);

}

//used by Objective-C and Actionscript
void ScintillaEditView::setObjCLexer(LangType langType) 
{
    execute(SCI_SETLEXER, SCLEX_CPP);
	const char *doxygenKeyWords = _pParameter->getWordList(L_CPP, LANG_INDEX_TYPE2);

	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	const char *lexerName = "objc";
	if (langType == L_FLASH)
		lexerName = "actionscript";

	makeStyle(lexerName, pKwArray);
	
	std::string objcInstr1Kwl("");
	if (pKwArray[LANG_INDEX_INSTR])
		objcInstr1Kwl = pKwArray[LANG_INDEX_INSTR];
	const char *objcInstrs = getCompleteKeywordList(objcInstr1Kwl, langType, LANG_INDEX_INSTR);
	
	std::string objcInstr2Kwl("");
	if (pKwArray[LANG_INDEX_INSTR2])
		objcInstr2Kwl = pKwArray[LANG_INDEX_INSTR2];
	const char *objCDirective = getCompleteKeywordList(objcInstr2Kwl, langType, LANG_INDEX_INSTR2);

	std::string objcTypeKwl("");
	if (pKwArray[LANG_INDEX_TYPE])
		objcTypeKwl = pKwArray[LANG_INDEX_TYPE];
	const char *objcTypes = getCompleteKeywordList(objcTypeKwl, langType, LANG_INDEX_TYPE);
	
	std::string objcType2Kwl("");
	if (pKwArray[LANG_INDEX_TYPE2])
		objcType2Kwl = pKwArray[LANG_INDEX_TYPE2];
	const char *objCQualifier = getCompleteKeywordList(objcType2Kwl, langType, LANG_INDEX_TYPE2);
	
	execute(SCI_SETKEYWORDS, 0, (LPARAM)objcInstrs);
    execute(SCI_SETKEYWORDS, 1, (LPARAM)objcTypes);
	execute(SCI_SETKEYWORDS, 2, (LPARAM)(doxygenKeyWords?doxygenKeyWords:""));
	execute(SCI_SETKEYWORDS, 3, (LPARAM)objCDirective);
	execute(SCI_SETKEYWORDS, 4, (LPARAM)objCQualifier);
}


void ScintillaEditView::setLexer(int lexerID, LangType langType, const char *lexerName, int whichList)
{
	execute(SCI_SETLEXER, lexerID);

	const char *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	
	makeStyle(lexerName, pKwArray);

	if (whichList & LIST_0)
	{
		std::string instrList("");
		if (pKwArray[LANG_INDEX_INSTR])
			instrList = pKwArray[LANG_INDEX_INSTR];
		execute(SCI_SETKEYWORDS, 0, (LPARAM)getCompleteKeywordList(instrList, langType, LANG_INDEX_INSTR));
	}	
	
	if (whichList & LIST_1)
	{
		std::string instr2List("");
		if (pKwArray[LANG_INDEX_INSTR2])
			instr2List = pKwArray[LANG_INDEX_INSTR2];
		execute(SCI_SETKEYWORDS, 1, (LPARAM)getCompleteKeywordList(instr2List, langType, LANG_INDEX_INSTR2));
	}

	if (whichList & LIST_2)
	{
		std::string typeList("");
		if (pKwArray[LANG_INDEX_TYPE])
			typeList = pKwArray[LANG_INDEX_TYPE];
		execute(SCI_SETKEYWORDS, 2, (LPARAM)getCompleteKeywordList(typeList, langType, LANG_INDEX_TYPE));
	}

	if (whichList & LIST_3)
	{
		std::string type2List("");
		if (pKwArray[LANG_INDEX_TYPE2])
			type2List = pKwArray[LANG_INDEX_TYPE2];
		execute(SCI_SETKEYWORDS, 3, (LPARAM)getCompleteKeywordList(type2List, langType, LANG_INDEX_TYPE2));
	}
}

void ScintillaEditView::defineDocType(LangType typeDoc)
{
	//setStyle(STYLE_DEFAULT, black, white, "Verdana", 0, 9);
    
    StyleArray & stylers = _pParameter->getMiscStylerArray();
    int iStyleDefault = stylers.getStylerIndexByID(STYLE_DEFAULT);
    if (iStyleDefault != -1)
    {
        Style & styleDefault = stylers.getStyler(iStyleDefault);
	    setStyle(styleDefault._styleID, styleDefault._fgColor, styleDefault._bgColor, styleDefault._fontName, styleDefault._fontStyle, styleDefault._fontSize);
    }

    execute(SCI_STYLECLEARALL);

	//StyleArray & stylers = _pParameter->getMiscStylerArray();
    int iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE);
    if (iFind != -1)
    {
        Style & styleFind = stylers.getStyler(iFind);
	    setStyle(styleFind._styleID, styleFind._fgColor, styleFind._bgColor, styleFind._fontName, styleFind._fontStyle, styleFind._fontSize);
    }

	//execute(SCI_STYLESETFORE, SCE_UNIVERSAL_FOUND_STYLE, yellow);
	//execute(SCI_STYLESETBACK, SCE_UNIVERSAL_FOUND_STYLE, red);
    
    int caretWidth = 1;
    
	
    // surtout, surtout ne pas faire SCI_SETCODEPAGE ici
    // ca rame a mort!!!
	if (isCJK())
	{
		if (getCurrentBuffer()._unicodeMode == uni8Bit)
			execute(SCI_SETCODEPAGE, _codepage);
	}

	execute(SCI_SETSTYLEBITS, 5);

	showMargin(_SC_MARGE_FOLDER, isNeededFolderMarge(typeDoc));
	switch (typeDoc)
	{
		case L_C :
		case L_CPP :
		case L_JS:
		case L_JAVA :
		case L_RC :
		case L_CS :
            setCppLexer(typeDoc); break;

		case L_FLASH :
        case L_OBJC :
            setObjCLexer(typeDoc); break;
		
	    case L_PHP :
		case L_ASP :
		case L_HTML :
		case L_XML :
			setXmlLexer(typeDoc); break;

		case L_CSS :
			setCssLexer(); break;

		case L_LUA :
			setLuaLexer(); break;

		case L_MAKEFILE :
			setMakefileLexer(); break;

		case L_INI :
			setIniLexer(); break;
			
        case L_USER :
			if (_buffers[_currentIndex]._userLangExt[0])
				setUserLexer(_buffers[_currentIndex]._userLangExt); 
			else
				setUserLexer();
			break;

        case L_NFO :
			if (!_MSLineDrawFont)
				_MSLineDrawFont = ::AddFontResource(LINEDRAW_FONT);
			if (_MSLineDrawFont)
			{
				LexerStyler *pStyler = (_pParameter->getLStylerArray()).getLexerStylerByName("nfo");
				COLORREF bg = black;
				COLORREF fg = liteGrey;
				if (pStyler)
				{
                    
                    int i = pStyler->getStylerIndexByName("DEFAULT");
                    if (i != -1)
                    {
                        Style & style = pStyler->getStyler(i);
                        bg = style._bgColor;
                        fg = style._fgColor;
                    }
				}

				setStyle(STYLE_DEFAULT, fg, bg, "MS LineDraw");
				execute(SCI_STYLECLEARALL);
			}
			break;

		case L_SQL :
			setSqlLexer(); break;

		case L_VB :
			setVBLexer(); break;

		case L_PASCAL :
			setPascalLexer(); break;

		case L_PERL :
			setPerlLexer(); break;

		case L_PYTHON :
			setPythonLexer(); break;

		case L_BATCH :
			setBatchLexer(); break;

		case L_TEX : 
			setTeXLexer(); break;

		case L_NSIS :
			setNsisLexer();break;

		case L_BASH :
			setBashLexer();break;

		case L_FORTRAN : 
			setFortranLexer();break;

		case L_TXT :
		default :
			execute(SCI_SETLEXER, (_codepage == CP_CHINESE_TRADITIONAL)?SCLEX_MAKEFILE:SCLEX_NULL); break;

	}
	//All the global styles should put here
	int index = stylers.getStylerIndexByID(STYLE_INDENTGUIDE);
	if (index != -1)
    {
        Style & style = stylers.getStyler(index);
	    setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
    }

	index = stylers.getStylerIndexByID(STYLE_BRACELIGHT);
	if (index != -1)
    {
        Style & style = stylers.getStyler(index);
	    setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
    }

	index = stylers.getStylerIndexByID(STYLE_BRACEBAD);
	if (index != -1)
    {
        Style & style = stylers.getStyler(index);
	    setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
    }

	execute(SCI_SETTABWIDTH, ((NppParameters::getInstance())->getNppGUI())._tabSize);
	execute(SCI_SETUSETABS, !((NppParameters::getInstance())->getNppGUI())._tabReplacedBySpace);

    execute(SCI_COLOURISE, 0, -1);
}

char * ScintillaEditView::attatchDefaultDoc(int nb)
{
	char title[10];
	char nb_str[4];
	strcat(strcpy(title, UNTITLED_STR), _itoa(nb, nb_str, 10));

	// get the doc pointer attached (by default) on the view Scintilla
	Document doc = execute(SCI_GETDOCPOINTER, 0, 0);

	// create the entry for our list
	_buffers.push_back(Buffer(doc, title));

	// set current index to 0
	_currentIndex = 0;

	return _buffers[_currentIndex]._fullPathName;
}


int ScintillaEditView::findDocIndexByName(const char *fn) const
{
	int index = -1;
	for (int i = 0 ; i < int(_buffers.size()) ; i++)
	{
		if (!strcmp(_buffers[i]._fullPathName, fn))
		{
			index = i;
			break;
		}
	}
	return index;
}

//! \brief this method activates the doc and the corresponding sub tab
//! \brief return the index of previeus current doc
char * ScintillaEditView::activateDocAt(int index)
{
	// before activating another document, we get the current position
	// from the Scintilla view then save it to the current document
	saveCurrentPos();
	Position & prevDocPos = _buffers[_currentIndex]._pos;

	// increase current doc ref count to 2 
	execute(SCI_ADDREFDOCUMENT, 0, _buffers[_currentIndex]._doc);

	// change the doc, this operation will decrease 
	// the ref count of old current doc to 1
	execute(SCI_SETDOCPOINTER, 0, _buffers[index]._doc);
	
	// reset all for another doc
	//execute(SCI_CLEARDOCUMENTSTYLE);
    //bool isDocTypeDiff = (_buffers[_currentIndex]._lang != _buffers[index]._lang);
	_currentIndex = index;
	
    //if (isDocTypeDiff)
    defineDocType(_buffers[_currentIndex]._lang);

	restoreCurrentPos(prevDocPos);

	execute(SCI_SETEOLMODE, _buffers[_currentIndex]._format);

    //execute(SCI_SETEOLMODE, _buffers[_currentIndex]._format);
	
    return _buffers[_currentIndex]._fullPathName;
}

// this method creates a new doc ,and adds it into 
// the end of the doc list and a last sub tab, then activate it
// it returns the name of this created doc (that's the current doc also)
char * ScintillaEditView::createNewDoc(const char *fn)
{
	Document newDoc = execute(SCI_CREATEDOCUMENT);
	_buffers.push_back(Buffer(newDoc, fn));
	_buffers[_buffers.size()-1].checkIfReadOnlyFile();
	return activateDocAt(int(_buffers.size())-1);
}

char * ScintillaEditView::createNewDoc(int nbNew)
{
	char title[10];
	char nb[4];
	strcat(strcpy(title, UNTITLED_STR), _itoa(nbNew, nb, 10));
	return createNewDoc(title);
}

// return the index to close then (argument) the index to activate
int ScintillaEditView::closeCurrentDoc(int & i2Activate)
{
	int oldCurrent = _currentIndex;

    Position & prevDocPos = _buffers[_currentIndex]._pos;

	// if the file 2 delete is the last one
	if (_currentIndex == int(_buffers.size()) - 1)
    {
		// if current index is 0, ie. the current is the only one
		if (!_currentIndex)
		{
			_currentIndex = 0;
		}
		// the current is NOT the only one and it is the last one,
		// we set it to the index which precedes it
		else
			_currentIndex -= 1;
    }
	// else the next current index will be the same,
	// we do nothing

	// get the iterator and calculate its position with the old current index value
	buf_vec_t::iterator posIt = _buffers.begin() + oldCurrent;

	// erase the position given document from our list
	_buffers.erase(posIt);

	// set another document, so the ref count of old active document owned
	// by Scintilla view will be decreased to 0 by SCI_SETDOCPOINTER message
	execute(SCI_SETDOCPOINTER, 0, _buffers[_currentIndex]._doc);

	defineDocType(_buffers[_currentIndex]._lang);
	restoreCurrentPos(prevDocPos);
	
    i2Activate = _currentIndex;
	
	return oldCurrent;
}

void ScintillaEditView::closeDocAt(int i2Close)
{
    execute(SCI_RELEASEDOCUMENT, 0, _buffers[i2Close]._doc);

	// get the iterator and calculate its position with the old current index value
	buf_vec_t::iterator posIt = _buffers.begin() + i2Close;

	// erase the position given document from our list
	_buffers.erase(posIt);

    _currentIndex -= (i2Close < _currentIndex)?1:0;
}

void ScintillaEditView::removeAllUnusedDocs()
{
	// unreference all docs  from list of Scintilla
	// by sending SCI_RELEASEDOCUMENT message
	for (int i = 0 ; i < int(_buffers.size()) ; i++)
		if (i != _currentIndex)
			execute(SCI_RELEASEDOCUMENT, 0, _buffers[i]._doc);
	
	// remove all docs except the current doc from list
	_buffers.clear();
}

void ScintillaEditView::getText(char *dest, int start, int end) 
{
	TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = dest;
	execute(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

void ScintillaEditView::marginClick(int position, int modifiers)
{
	int lineClick = int(execute(SCI_LINEFROMPOSITION, position, 0));
	int levelClick = int(execute(SCI_GETFOLDLEVEL, lineClick, 0));
	if (levelClick & SC_FOLDLEVELHEADERFLAG)
    {
		if (modifiers & SCMOD_SHIFT)
        {
			// Ensure all children visible
			execute(SCI_SETFOLDEXPANDED, lineClick, 1);
			expand(lineClick, true, true, 100, levelClick);
		}
        else if (modifiers & SCMOD_CTRL) 
        {
			if (execute(SCI_GETFOLDEXPANDED, lineClick, 0)) 
            {
				// Contract this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 0);
				expand(lineClick, false, true, 0, levelClick);
			} 
            else 
            {
				// Expand this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 1);
				expand(lineClick, true, true, 100, levelClick);
			}
		} 
        else 
        {
			// Toggle this line
			execute(SCI_TOGGLEFOLD, lineClick, 0);
		}
	}
}

void ScintillaEditView::expand(int &line, bool doExpand, bool force, int visLevels, int level)
{
	int lineMaxSubord = int(execute(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK));
	line++;
	while (line <= lineMaxSubord)
    {
		if (force) 
        {
			if (visLevels > 0)
				execute(SCI_SHOWLINES, line, line);
			else
				execute(SCI_HIDELINES, line, line);
		} 
        else 
        {
			if (doExpand)
				execute(SCI_SHOWLINES, line, line);
		}
		int levelLine = level;
		if (levelLine == -1)
			levelLine = int(execute(SCI_GETFOLDLEVEL, line, 0));
		if (levelLine & SC_FOLDLEVELHEADERFLAG)
        {
			if (force) 
            {
				if (visLevels > 1)
					execute(SCI_SETFOLDEXPANDED, line, 1);
				else
					execute(SCI_SETFOLDEXPANDED, line, 0);
				expand(line, doExpand, force, visLevels - 1);
			} 
            else
            {
				if (doExpand)
                {
					if (!execute(SCI_GETFOLDEXPANDED, line, 0))
						execute(SCI_SETFOLDEXPANDED, line, 1);

					expand(line, true, force, visLevels - 1);
				} 
                else 
                {
					expand(line, false, force, visLevels - 1);
				}
			}
		}
        else
        {
			line++;
		}
	}
}

void ScintillaEditView::makeStyle(const char *lexerName, const char **keywordArray)
{
	LexerStyler *pStyler = (_pParameter->getLStylerArray()).getLexerStylerByName(lexerName);
	if (pStyler)
	{
		for (int i = 0 ; i < pStyler->getNbStyler() ; i++)
		{
			Style & style = pStyler->getStyler(i);
			setStyle(style._styleID, style._fgColor, style._bgColor, style._fontName, style._fontStyle, style._fontSize);
			if (keywordArray)
			{
				if ((style._keywordClass != -1) && (style._keywords))
					keywordArray[style._keywordClass] = style._keywords->c_str();
			}
		}
	}
}

void ScintillaEditView::performGlobalStyles() 
{
	StyleArray & stylers = _pParameter->getMiscStylerArray();

	int i = stylers.getStylerIndexByName("Current line background colour");
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		execute(SCI_SETCARETLINEBACK, style._bgColor);
	}

	i = stylers.getStylerIndexByName("Mark colour");
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		execute(SCI_MARKERSETFORE, 1, style._fgColor);
		execute(SCI_MARKERSETBACK, 1, style._bgColor);
	}

    COLORREF selectColorBack = grey;

	i = stylers.getStylerIndexByName("Selected text colour");
	if (i != -1)
    {
        Style & style = stylers.getStyler(i);
		selectColorBack = style._bgColor;
		execute(SCI_SETSELBACK, 1, selectColorBack);
    }

    COLORREF caretColor = black;
	i = stylers.getStylerIndexByID(SCI_SETCARETFORE);
	if (i != -1)
    {
        Style & style = stylers.getStyler(i);
        caretColor = style._fgColor;
    }
    execute(SCI_SETCARETFORE, caretColor);

	COLORREF edgeColor = liteGrey;
	i = stylers.getStylerIndexByName("Edge colour");
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		edgeColor = style._fgColor;
	}
	execute(SCI_SETEDGECOLOUR, edgeColor);
}

void ScintillaEditView::setLineIndent(int line, int indent) const {
	if (indent < 0)
		return;
	CharacterRange crange = getSelection();
	int posBefore = execute(SCI_GETLINEINDENTPOSITION, line);
	execute(SCI_SETLINEINDENTATION, line, indent);
	int posAfter = execute(SCI_GETLINEINDENTPOSITION, line);
	int posDifference = posAfter - posBefore;
	if (posAfter > posBefore) {
		// Move selection on
		if (crange.cpMin >= posBefore) {
			crange.cpMin += posDifference;
		}
		if (crange.cpMax >= posBefore) {
			crange.cpMax += posDifference;
		}
	} else if (posAfter < posBefore) {
		// Move selection back
		if (crange.cpMin >= posAfter) {
			if (crange.cpMin >= posBefore)
				crange.cpMin += posDifference;
			else
				crange.cpMin = posAfter;
		}
		if (crange.cpMax >= posAfter) {
			if (crange.cpMax >= posBefore)
				crange.cpMax += posDifference;
			else
				crange.cpMax = posAfter;
		}
	}
	execute(SCI_SETSEL, crange.cpMin, crange.cpMax);
}

const char * ScintillaEditView::getCompleteKeywordList(std::string & kwl, LangType langType, int keywordIndex)
{
	kwl += " ";
	const char *defKwl = _pParameter->getWordList(langType, keywordIndex);
	kwl += defKwl?defKwl:"";
	return kwl.c_str();
}



