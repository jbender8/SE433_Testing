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

#include "Parameters.h"
#include "ScintillaEditView.h"
#include <shlobj.h>

//#include <windows.h>


NppParameters * NppParameters::_pSelf = new NppParameters;

NppParameters::NppParameters():_nbLang(0), _nbFile(0), _nbMaxFile(10), _pXmlDoc(NULL),\
							   _pXmlUserDoc(NULL), _pXmlUserStylerDoc(NULL), _pXmlUserLangDoc(NULL),\
							   _nbUserLang(0), _transparentFuncAddr(NULL), _pXmlToolIconsDoc(NULL)
{
	for (int i = 0 ; i < NB_LANG ; _langList[i] = NULL, i++);
	char nppPath[MAX_LEN];
	char userPath[MAX_LEN];
	
	// Prepare for default path
	::GetModuleFileName(NULL, nppPath, sizeof(nppPath));
	PathRemoveFileSpec(nppPath);

	//SHGetSpecialFolderPath(NULL, userPath, CSIDL_APPDATA, TRUE);
	ITEMIDLIST *pidl;
	SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
	SHGetPathFromIDList(pidl, userPath);

	PathAppend(userPath, "Notepad++");
	if (!PathFileExists(userPath))
	{
		::CreateDirectory(userPath, NULL);
	}
	//-------------------------------------//
	// Transparent function for w2k and xp //
	//-------------------------------------//
	HMODULE hUser32 = ::GetModuleHandle("User32");
	if (hUser32)
		_transparentFuncAddr = (WNDPROC)GetProcAddress(hUser32, "SetLayeredWindowAttributes");

	//---------------------------------------//
	// langs.xml : for every user statically //
	//---------------------------------------//
	char langs_xml_path[MAX_LEN];
	strcpy(langs_xml_path, nppPath);
	
	PathAppend(langs_xml_path, "langs.xml");

	_pXmlDoc = new TiXmlDocument(langs_xml_path);
	bool loadOkay = _pXmlDoc->LoadFile();
	if (!loadOkay)
	{
		::MessageBox(NULL, "Load langs.xml failed!", "Configurator",MB_OK);
		delete _pXmlDoc;
		_pXmlDoc = NULL;
	}
	else
		getLangKeywordsFromXmlTree();

	//---------------------------//
	// config.xml : for per user //
	//---------------------------//
	char configPath[MAX_LEN];
	strcpy(configPath, userPath);
	PathAppend(configPath, "config.xml");

	if (!PathFileExists(configPath))
	{
		char srcConfigPath[MAX_LEN];
		strcpy(srcConfigPath, nppPath);
		PathAppend(srcConfigPath, "config.xml");

		::CopyFile(srcConfigPath, configPath, TRUE);
	}

	_pXmlUserDoc = new TiXmlDocument(configPath);
	loadOkay = _pXmlUserDoc->LoadFile();
	if (!loadOkay)
	{
		::MessageBox(NULL, "Load config.xml failed!", "Configurator",MB_OK);
		delete _pXmlUserDoc;
		_pXmlUserDoc = NULL;
	}
	else
		getUserParametersFromXmlTree();

	//----------------------------//
	// stylers.xml : for per user //
	//----------------------------//
	char stylerPath[MAX_LEN];
	strcpy(stylerPath, userPath);
	PathAppend(stylerPath, "stylers.xml");

	if (!PathFileExists(stylerPath))
	{
		char srcStylersPath[MAX_LEN];
		strcpy(srcStylersPath, nppPath);
		PathAppend(srcStylersPath, "stylers.xml");

		::CopyFile(srcStylersPath, stylerPath, TRUE);
	}

	_pXmlUserStylerDoc = new TiXmlDocument(stylerPath);
	loadOkay = _pXmlUserStylerDoc->LoadFile();
	if (!loadOkay)
	{
		::MessageBox(NULL, "Load stylers.xml failed!", "Configurator",MB_OK);
		delete _pXmlUserStylerDoc;
		_pXmlUserStylerDoc = NULL;
	}
	else
		getUserStylersFromXmlTree();

	//-----------------------------------//
	// userDefineLang.xml : for per user //
	//-----------------------------------//
	strcpy(_userDefineLangPath, userPath);
	PathAppend(_userDefineLangPath, "userDefineLang.xml");

	_pXmlUserLangDoc = new TiXmlDocument(_userDefineLangPath);
	loadOkay = _pXmlUserLangDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlUserLangDoc;
		_pXmlUserLangDoc = NULL;
	}
	else
		getUserDefineLangsFromXmlTree();	
	
	//-------------------------------//
	// nativeLang.xml : for per user //
	//-------------------------------//
	char nativeLangPath[MAX_LEN];
	strcpy(nativeLangPath, userPath);
	PathAppend(nativeLangPath, "nativeLang.xml");

	_pXmlNativeLangDoc = new TiXmlDocument(nativeLangPath);
	loadOkay = _pXmlNativeLangDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlNativeLangDoc;
		_pXmlNativeLangDoc = NULL;
	}	
	
	//---------------------------------//
	// toolbarIcons.xml : for per user //
	//---------------------------------//
	char toolbarIconsPath[MAX_LEN];
	strcpy(toolbarIconsPath, userPath);
	PathAppend(toolbarIconsPath, "toolbarIcons.xml");

	_pXmlToolIconsDoc = new TiXmlDocument(toolbarIconsPath);
	loadOkay = _pXmlToolIconsDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlToolIconsDoc;
		_pXmlToolIconsDoc = NULL;
	}
}

void NppParameters::setFontList(HWND hWnd)
{
	//---------------//
	// Sys font list //
	//---------------//
	LOGFONT lf;
	_fontlist.push_back("");

	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0]='\0';
	HDC hDC = ::GetDC(hWnd);
	::EnumFontFamiliesEx(hDC, 
						&lf, 
						(FONTENUMPROC) EnumFontFamExProc, 
						(LPARAM) &_fontlist, 0);
}

void NppParameters::getLangKeywordsFromXmlTree()
{
	TiXmlNode *root = _pXmlDoc->FirstChild("NotepadPlus");
		if (!root) return;
	feedKeyWordsParameters(root);
}

bool NppParameters::getUserStylersFromXmlTree()
{
	TiXmlNode *root = _pXmlUserStylerDoc->FirstChild("NotepadPlus");
		if (!root) return false;
	return feedStylerArray(root);
}

bool NppParameters::getUserParametersFromXmlTree()
{
	if (!_pXmlUserDoc)
		return false;

	TiXmlNode *root = _pXmlUserDoc->FirstChild("NotepadPlus");
	if (!root) return false;

	// GUI
	feedGUIParameters(root);

	//History
	//Prendre toutes les allocation familliale de ses enfants
	feedFileListParameters(root);

	//Abondonner (ou plus radicalement, tuer) sa femme et ses enfants
	TiXmlNode *node = root->FirstChildElement("History");
	root->RemoveChild(node);

	//Trouver une jeune fille qui n'a pas d'enfant
	TiXmlElement HistoryNode("History");

	//se marrier avec cette jeune fille 
	root->InsertEndChild(HistoryNode);
	return true;
}

bool NppParameters::getUserDefineLangsFromXmlTree()
{
	if (!_pXmlUserLangDoc)
		return false;
	
	TiXmlNode *root = _pXmlUserLangDoc->FirstChild("NotepadPlus");
	if (!root) 
		return false;

	feedUserLang(root);
}

void NppParameters::feedFileListParameters(TiXmlNode *node)
{
	TiXmlNode *historyRoot = node->FirstChildElement("History");
	if (!historyRoot) return;

	//int nbMaxFile;
	(historyRoot->ToElement())->Attribute("nbMaxFile", &_nbMaxFile);
	if ((_nbMaxFile < 0) || (_nbMaxFile > 30))
		_nbMaxFile = 10;

	for (TiXmlNode *childNode = historyRoot->FirstChildElement("File");
		childNode && (_nbFile < NB_MAX_LRF_FILE);
		childNode = childNode->NextSibling("File") )
	{
		_LRFileList[_nbFile] = new std::string((childNode->FirstChild())->Value());
		_nbFile++;
	}
}
const int loadFailed = 100;
const int missingName = 101;
void NppParameters::feedUserLang(TiXmlNode *node)
{
	for (TiXmlNode *childNode = node->FirstChildElement("UserLang");
		childNode && (_nbUserLang < NB_MAX_USER_LANG);
		childNode = childNode->NextSibling("UserLang") )
	{
		const char *name = (childNode->ToElement())->Attribute("name");
		const char *ext = (childNode->ToElement())->Attribute("ext");
		try {
			if (!name || !name[0] || !ext) throw int(missingName);

			_userLangArray[_nbUserLang] = new UserLangContainer(name, ext);
			_nbUserLang++;

			TiXmlNode *keywordListsRoot = childNode->FirstChildElement("KeywordLists");
			if (!keywordListsRoot) throw int(loadFailed);
			feedUserKeywordList(keywordListsRoot);

			TiXmlNode *stylesRoot = childNode->FirstChildElement("Styles");
			if (!stylesRoot) throw int(loadFailed);
			feedUserStyles(stylesRoot);

		} catch (int e) {
			if (e == loadFailed)
				delete _userLangArray[--_nbUserLang];
		}
	}
}

int NppParameters::addUserLangToEnd(const UserLangContainer & userLang, const char *newName)
{
	if (isExistingUserLangName(newName))
		return -1;
	_userLangArray[_nbUserLang] = new UserLangContainer();
	*(_userLangArray[_nbUserLang]) = userLang;
	strcpy(_userLangArray[_nbUserLang]->_name, newName);
	_nbUserLang++;
	return _nbUserLang-1;
}

void NppParameters::removeUserLang(int index)
{
	if (index >= _nbUserLang )
		return;
	delete _userLangArray[index];
	for (int i = index ; i < (_nbUserLang - 1) ; i++)
		_userLangArray[i] = _userLangArray[i+1];
	_nbUserLang--;
}

int NppParameters::getIndexFromKeywordListName(const char *name)
{
	if (!name) return -1;
	if (!strcmp(name, "Folder+"))	return 1;
	else if (!strcmp(name, "Folder-"))	return 2;
	else if (!strcmp(name, "Operators"))return 3;
	else if (!strcmp(name, "Comment"))	return 4;
	else if (!strcmp(name, "Words1"))	return 5;
	else if (!strcmp(name, "Words2"))	return 6;
	else if (!strcmp(name, "Words3"))	return 7;
	else if (!strcmp(name, "Words4"))	return 8;
	else return -1;
}

void NppParameters::feedUserKeywordList(TiXmlNode *node)
{
	for (TiXmlNode *childNode = node->FirstChildElement("Keywords");
		childNode ;
		childNode = childNode->NextSibling("Keywords"))
	{
		const char *keywordsName = (childNode->ToElement())->Attribute("name");
		int i = getIndexFromKeywordListName(keywordsName);
		if (i != -1)
		{
			TiXmlNode *valueNode = childNode->FirstChild();
			const char *kwl = (valueNode)?valueNode->Value():"";
			strcpy(_userLangArray[_nbUserLang - 1]->_keywordLists[i], kwl);
		}
	}
}

void NppParameters::feedUserStyles(TiXmlNode *node)
{
	for (TiXmlNode *childNode = node->FirstChildElement("WordsStyle");
		childNode ;
		childNode = childNode->NextSibling("WordsStyle"))
	{
		int id;
		const char *styleIDStr = (childNode->ToElement())->Attribute("styleID", &id);
		if (styleIDStr)
		{
			_userLangArray[_nbUserLang - 1]->_styleArray.addStyler(id, childNode);
		}
	}
}

bool NppParameters::feedStylerArray(TiXmlNode *node)
{
    TiXmlNode *styleRoot = node->FirstChildElement("LexerStyles");
    if (!styleRoot) return false;

    // For each lexer
    for (TiXmlNode *childNode = styleRoot->FirstChildElement("LexerType");
		 childNode ;
		 childNode = childNode->NextSibling("LexerType") )
    {
     	if (!_lexerStylerArray.hasEnoughSpace()) return false;

	    TiXmlElement *element = childNode->ToElement();
	    const char *lexerName = element->Attribute("name");
		const char *lexerDesc = element->Attribute("desc");
		const char *lexerUserExt = element->Attribute("ext");
	    if (lexerName)
	    {
		    _lexerStylerArray.addLexerStyler(lexerName, lexerDesc, lexerUserExt, childNode);
	    }
    }

    // The global styles for all lexers
    TiXmlNode *globalStyleRoot = node->FirstChildElement("GlobalStyles");
    if (!globalStyleRoot) return false;

    for (TiXmlNode *childNode = globalStyleRoot->FirstChildElement("WidgetStyle");
		 childNode ;
		 childNode = childNode->NextSibling("WidgetStyle") )
    {
     	if (!_widgetStyleArray.hasEnoughSpace()) return false;

	    TiXmlElement *element = childNode->ToElement();
	    const char *styleIDStr = element->Attribute("styleID");

        int styleID = -1;
		if ((styleID = decStrVal(styleIDStr)) != -1)
		{
		    _widgetStyleArray.addStyler(styleID, childNode);
        }
    }

	return true;
}

void LexerStylerArray::addLexerStyler(const char *lexerName, const char *lexerDesc, const char *lexerUserExt , TiXmlNode *lexerNode)
{
    LexerStyler & ls = _lexerStylerArray[_nbLexerStyler++];
    ls.setLexerName(lexerName);
	if (lexerDesc)
		ls.setLexerDesc(lexerDesc);
	ls.setLexerUserExt(lexerUserExt);
    
    for (TiXmlNode *childNode = lexerNode->FirstChildElement("WordsStyle");
		 childNode ;
		 childNode = childNode->NextSibling("WordsStyle") )
    {
	        
		if (!ls.hasEnoughSpace()) return;

		TiXmlElement *element = childNode->ToElement();
		const char *styleIDStr = element->Attribute("styleID");
		
		if (styleIDStr)
		{
			int styleID = -1;
			if ((styleID = decStrVal(styleIDStr)) != -1)
			{
				ls.addStyler(styleID, childNode);
			}
		}
    }
}

void StyleArray::addStyler(int styleID, TiXmlNode *styleNode)
{
	_styleArray[_nbStyler]._styleID = styleID;
	
	if (styleNode)
	{
		TiXmlElement *element = styleNode->ToElement();
		
		// Pour _fgColor, _bgColor :
		// RGB() | (result & 0xFF000000) c'est pour le cas de -1 (0xFFFFFFFF)
		// retourné par hexStrVal(str)
		const char *str = element->Attribute("name");
		if (str)
		{
			_styleArray[_nbStyler]._styleDesc = str;
		}

		str = element->Attribute("fgColor");
		if (str)
		{
			unsigned long result = hexStrVal(str);
			_styleArray[_nbStyler]._fgColor = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
	            
		}
		
		str = element->Attribute("bgColor");
		if (str)
		{
			unsigned long result = hexStrVal(str);
			_styleArray[_nbStyler]._bgColor = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
		}
		
		str = element->Attribute("fontName");
		_styleArray[_nbStyler]._fontName = str;
		
		str = element->Attribute("fontStyle");
		if (str)
		{
			_styleArray[_nbStyler]._fontStyle = decStrVal(str);
		}
		
		str = element->Attribute("fontSize");
		if (str)
		{
			_styleArray[_nbStyler]._fontSize = decStrVal(str);
		}

		str = element->Attribute("keywordClass");
		if (str)
		{
			_styleArray[_nbStyler]._keywordClass = getKwClassFromName(str);
		}

		TiXmlNode *v = styleNode->FirstChild();
		if (v)
		{
			//const char *keyWords = v->Value();
			//if (keyWords)
				_styleArray[_nbStyler]._keywords = new std::string(v->Value());
		}
	}
	_nbStyler++;
}

void NppParameters::writeHistory(const char *fullpath)
{
	TiXmlNode *historyNode = (_pXmlUserDoc->FirstChild("NotepadPlus"))->FirstChildElement("History");
	TiXmlElement recentFileNode("File");
	TiXmlText fileNameFullPath(fullpath);
	recentFileNode.InsertEndChild(fileNameFullPath);

	(historyNode->ToElement())->InsertEndChild(recentFileNode);
}

// 3 restes : L_H, L_MAKEFILE, L_USER
LangType NppParameters::getLangIDFromStr(const char *langName)
{
	if (!strcmp("c", langName))	return L_C;
	if (!strcmp("cpp", langName)) return L_CPP;
	if (!strcmp("java", langName)) return L_JAVA;
	if (!strcmp("cs", langName)) return L_CS;
	if (!strcmp("objc", langName)) return L_OBJC;
	if (!strcmp("rc", langName)) return L_RC;
	if (!strcmp("html", langName)) return L_HTML;
	if (!strcmp("javascript", langName)) return L_JS;
	if (!strcmp("php", langName)) return L_PHP;
	if (!strcmp("vb", langName)) return L_VB;
    if (!strcmp("sql", langName)) return L_SQL;
	if (!strcmp("xml", langName)) return L_XML;
	if (!strcmp("asp", langName)) return L_ASP;
	if (!strcmp("perl", langName)) return L_PERL;
	if (!strcmp("pascal", langName)) return L_PASCAL;
	if (!strcmp("python", langName)) return L_PYTHON;
	if (!strcmp("css", langName)) return L_CSS;
	if (!strcmp("lua", langName)) return L_LUA;
	if (!strcmp("batch", langName)) return L_BATCH;
	if (!strcmp("ini", langName)) return L_INI;
	if (!strcmp("nfo", langName)) return L_NFO;

	if (!strcmp("tex", langName)) return L_TEX;
	if (!strcmp("fortran", langName)) return L_FORTRAN;
	if (!strcmp("bash", langName)) return L_BASH;
	if (!strcmp("actionscript", langName)) return L_FLASH;	
	if (!strcmp("nsis", langName)) return L_NSIS;	
	return L_TXT;
}

int NppParameters::getIndexFromStr(const char *indexName) const
{
	std::string str(indexName);

	if (str == "instre1")
		return LANG_INDEX_INSTR;
	else if (str == "instre2")
		return LANG_INDEX_INSTR2;
	else if (str == "type1")
		return LANG_INDEX_TYPE;
	else if (str == "type2")
		return LANG_INDEX_TYPE2;
	else
		return -1;
}

void NppParameters::feedKeyWordsParameters(TiXmlNode *node)
{
	
	TiXmlNode *langRoot = node->FirstChildElement("Languages");
	if (!langRoot) return;

	for (TiXmlNode *langNode = langRoot->FirstChildElement("Language");
		langNode ;
		langNode = langNode->NextSibling("Language") )
	{
		if (_nbLang < NB_LANG)
		{
			TiXmlElement *element = langNode->ToElement();
			const char *name = element->Attribute("name");
			if (name)
			{
				_langList[_nbLang] = new Lang(getLangIDFromStr(name), name);
				_langList[_nbLang]->setDefaultExtList(element->Attribute("ext"));

				for (TiXmlNode *kwNode = langNode->FirstChildElement("Keywords");
					kwNode ;
					kwNode = kwNode->NextSibling("Keywords") )
				{
					const char *indexName = (kwNode->ToElement())->Attribute("name");
					TiXmlNode *kwVal = kwNode->FirstChild();
					const char *keyWords = "";
					if ((indexName) && (kwVal))
						keyWords = kwVal->Value();

					int i = getIndexFromStr(indexName);

					if (i != -1)
						_langList[_nbLang]->setWords(keyWords, i);
				}
				_nbLang++;
			}
		}
	}
}

void NppParameters::feedGUIParameters(TiXmlNode *node)
{
	TiXmlNode *GUIRoot = node->FirstChildElement("GUIConfigs");
	if (!GUIRoot) return;

	for (TiXmlNode *childNode = GUIRoot->FirstChildElement("GUIConfig");
		childNode ;
		childNode = childNode->NextSibling("GUIConfig") )
	{
		TiXmlElement *element = childNode->ToElement();
		const char *nm = element->Attribute("name");
		if (!nm) return;

		const char *val;

		if (!strcmp(nm, "ToolBar"))
		{
			val = (childNode->FirstChild())->Value();
			if (!val) return;

			if (!strcmp(val, "hide"))
				_nppGUI._toolBarStatus = TB_HIDE;
			else if (!strcmp(val, "small"))
				_nppGUI._toolBarStatus = TB_SMALL;
			else if (!strcmp(val, "large"))
				_nppGUI._toolBarStatus = TB_LARGE;
			else if (!strcmp(val, "standard"))
				_nppGUI._toolBarStatus = TB_STANDARD;
		}
		else if (!strcmp(nm, "StatusBar"))
		{
			val = (childNode->FirstChild())->Value();
			if (!val) return;

			if (!strcmp(val, "hide"))
				_nppGUI._statusBarShow = false;
			else if (!strcmp(val, "show"))
				_nppGUI._statusBarShow = true;
		}
		else if (!strcmp(nm, "TabBar"))
		{
			bool isFailed = false;
			int oldValue = _nppGUI._tabStatus;
			val = element->Attribute("dragAndDrop");
			if (val)
			{
				if (!strcmp(val, "yes"))
					_nppGUI._tabStatus = TAB_DRAGNDROP;
				else if (!strcmp(val, "no"))
					_nppGUI._tabStatus = 0;
				else
					isFailed = true;
			}

			val = element->Attribute("drawTopBar");
			if (val)
			{
				if (!strcmp(val, "yes"))
					_nppGUI._tabStatus |= TAB_DRAWTOPBAR;
				else if (!strcmp(val, "no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}
			val = element->Attribute("drawInactiveTab");
			if (val)
			{
				if (!strcmp(val, "yes"))
					_nppGUI._tabStatus |= TAB_DRAWINACTIVETAB;
				else if (!strcmp(val, "no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}
			val = element->Attribute("reduce");
			if (val)
			{
				if (!strcmp(val, "yes"))
					_nppGUI._tabStatus |= TAB_REDUCE;
				else if (!strcmp(val, "no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}
			if (isFailed)
				_nppGUI._tabStatus = oldValue;
		}

		else if (!strcmp(nm, "ScintillaViewsSplitter"))
		{
			val = (childNode->FirstChild())->Value();
			if (!val) return;

			if (!strcmp(val, "vertical"))
				_nppGUI._splitterPos = POS_VERTICAL;
			else if (!strcmp(val, "horizontal"))
				_nppGUI._splitterPos = POS_HORIZOTAL;
		}
		else if (!strcmp(nm, "UserDefineDlg"))
		{
			bool isFailed = false;
			int oldValue = _nppGUI._userDefineDlgStatus;

			val = (childNode->FirstChild())->Value();
			if (val) 
			{
				if (!strcmp(val, "hide"))
					_nppGUI._userDefineDlgStatus = 0;
				else if (!strcmp(val, "show"))
					_nppGUI._userDefineDlgStatus = UDD_SHOW;
				else 
					isFailed = true;
			}
			val = element->Attribute("position");
			if (val) 
			{
				if (!strcmp(val, "docked"))
					_nppGUI._userDefineDlgStatus |= UDD_DOCKED;
				else if (!strcmp(val, "undocked"))
					_nppGUI._userDefineDlgStatus |= 0;
				else 
					isFailed = true;
			}
			if (isFailed)
				_nppGUI._userDefineDlgStatus = oldValue;
		}
		else if (!strcmp(nm, "TabSetting"))
		{
			val = element->Attribute("size");
			if (val)
				_nppGUI._tabSize = decStrVal(val);

			if ((_nppGUI._tabSize == -1) || (_nppGUI._tabSize == 0))
				_nppGUI._tabSize = 8;
			//bool isFailed = false;
			val = element->Attribute("replaceBySpace");
			if (val)
				_nppGUI._tabReplacedBySpace = (!strcmp(val, "yes"));
		}
		else if (!strcmp(nm, "AppPosition"))
		{
			RECT oldRect = _nppGUI._appPos;
			bool fuckUp = true;
			int i;

			if (element->Attribute("x", &i))
			{
				_nppGUI._appPos.left = i;

				if (element->Attribute("y", &i))
				{
					_nppGUI._appPos.top = i;

					if (element->Attribute("width", &i))
					{
						_nppGUI._appPos.right = i;

						if (element->Attribute("height", &i))
						{
							_nppGUI._appPos.bottom = i;
							fuckUp = false;
						}
					}
				}
			}

			if (fuckUp)
				_nppGUI._appPos = oldRect;
		}

		else if (!strcmp(nm, "EdgeLine"))
		{
			int nbColumn;
			if (element->Attribute("nbColumn", &nbColumn))
				_nbColumnEdge = nbColumn;
		}

		else if (!strcmp(nm, "ScintillaPrimaryView"))
		{
			feedScintillaParam(SCIV_PRIMARY, element);
		}
		else if (!strcmp(nm, "ScintillaSecondaryView"))
		{
			feedScintillaParam(SCIV_SECOND, element);
		}
	}
}
void NppParameters::feedScintillaParam(bool whichOne, TiXmlNode *node)
{
    TiXmlElement *element = node->ToElement();

    // Line Number Margin
    const char *nm = element->Attribute("lineNumberMargin");
	if (nm) 
	{
		if (!strcmp(nm, "show"))
			_svp[whichOne]._lineNumberMarginShow = true;
		else if (!strcmp(nm, "hide"))
			_svp[whichOne]._lineNumberMarginShow = false;
	}

	// Bookmark Margin
	nm = element->Attribute("bookMarkMargin");
	if (nm) 
	{

		if (!strcmp(nm, "show"))
			_svp[whichOne]._bookMarkMarginShow = true;
		else if (!strcmp(nm, "hide"))
			_svp[whichOne]._bookMarkMarginShow = false;
	}

    // Indent GuideLine 
    nm = element->Attribute("indentGuideLine");
	if (nm)
	{
		if (!strcmp(nm, "show"))
			_svp[whichOne]._indentGuideLineShow = true;
		else if (!strcmp(nm, "hide"))
			_svp[whichOne]._indentGuideLineShow= false;
	}

    // Folder Mark Style
    nm = element->Attribute("folderMarkStyle");
	if (nm)
	{
		if (!strcmp(nm, "box"))
			_svp[whichOne]._folderStyle = FOLDER_STYLE_BOX;
		else if (!strcmp(nm, "circle"))
			_svp[whichOne]._folderStyle = FOLDER_STYLE_CIRCLE;
		else if (!strcmp(nm, "arrow"))
			_svp[whichOne]._folderStyle = FOLDER_STYLE_ARROW;
		else if (!strcmp(nm, "simple"))
			_svp[whichOne]._folderStyle = FOLDER_STYLE_SIMPLE;
	}

    // Current Line Highlighting State
    nm = element->Attribute("currentLineHilitingShow");
	if (nm)
	{
		if (!strcmp(nm, "show"))
			_svp[whichOne]._currentLineHilitingShow = true;
		else if (!strcmp(nm, "hide"))
			_svp[whichOne]._currentLineHilitingShow = false;
	}

	// Do Wrap
    nm = element->Attribute("Wrap");
	if (nm)
	{
		if (!strcmp(nm, "yes"))
			_svp[whichOne]._doWrap = true;
		else if (!strcmp(nm, "no"))
			_svp[whichOne]._doWrap = false;
	}

	// Do Edge
    nm = element->Attribute("edge");
	if (nm)
	{
		if (!strcmp(nm, "background"))
			_svp[whichOne]._edgeMode = EDGE_BACKGROUND;
		else if (!strcmp(nm, "line"))
			_svp[whichOne]._edgeMode = EDGE_LINE;
		else
			_svp[whichOne]._edgeMode = EDGE_NONE;
	}
}

void NppParameters::writeGUIParams()
{
	TiXmlNode *GUIRoot = (_pXmlUserDoc->FirstChild("NotepadPlus"))->FirstChildElement("GUIConfigs");
	if (!GUIRoot) 
	{
		return;
	}
	for (TiXmlNode *childNode = GUIRoot->FirstChildElement("GUIConfig");
		childNode ;
		childNode = childNode->NextSibling("GUIConfig"))
	{
		TiXmlElement *element = childNode->ToElement();
		const char *nm = element->Attribute("name");
		if (!nm) {return;}

		if (!strcmp(nm, "ToolBar"))
		{
			const char *pStr = _nppGUI._toolBarStatus == TB_HIDE?"hide":(_nppGUI._toolBarStatus == TB_SMALL?"small":(_nppGUI._toolBarStatus == TB_STANDARD?"standard":"large"));
			(childNode->FirstChild())->SetValue(pStr);
		}
		else if (!strcmp(nm, "StatusBar"))
		{
			const char *pStr = _nppGUI._statusBarShow?"show":"hide";
			(childNode->FirstChild())->SetValue(pStr);
		}
		else if (!strcmp(nm, "TabBar"))
		{
			const char *pStr = (_nppGUI._tabStatus & TAB_DRAWTOPBAR)?"yes":"no";
			element->SetAttribute("dragAndDrop", pStr);

			pStr = (_nppGUI._tabStatus & TAB_DRAGNDROP)?"yes":"no";
			element->SetAttribute("drawTopBar", pStr);

			pStr = (_nppGUI._tabStatus & TAB_DRAWINACTIVETAB)?"yes":"no";
			element->SetAttribute("drawInactiveTab", pStr);

			pStr = (_nppGUI._tabStatus & TAB_REDUCE)?"yes":"no";
			element->SetAttribute("reduce", pStr);
		}
		else if (!strcmp(nm, "ScintillaViewsSplitter"))
		{
			const char *pStr = _nppGUI._splitterPos == POS_VERTICAL?"vertical":"horizontal";
			(childNode->FirstChild())->SetValue(pStr);
		}
		else if (!strcmp(nm, "UserDefineDlg"))
		{
			const char *pStr = _nppGUI._userDefineDlgStatus & UDD_SHOW?"show":"hide";
			(childNode->FirstChild())->SetValue(pStr);
			
			pStr = (_nppGUI._userDefineDlgStatus & UDD_DOCKED)?"docked":"undocked";
			element->SetAttribute("position", pStr);
		}
		else if (!strcmp(nm, "TabSetting"))
		{
			const char *pStr = _nppGUI._tabReplacedBySpace?"yes":"no";
			element->SetAttribute("replaceBySpace", pStr);
			element->SetAttribute("size", _nppGUI._tabSize);
		}
		else if (!strcmp(nm, "AppPosition"))
		{
			element->SetAttribute("x", _nppGUI._appPos.left);
			element->SetAttribute("y", _nppGUI._appPos.top);
			element->SetAttribute("width", _nppGUI._appPos.right);
			element->SetAttribute("height", _nppGUI._appPos.bottom);
		}
		else if (!strcmp(nm, "EdgeLine"))
		{
			element->SetAttribute("nbColumn", _nbColumnEdge);
		}
	}
}

int RGB2int(COLORREF color) {
    return (((((DWORD)color) & 0x0000FF) << 16) | ((((DWORD)color) & 0x00FF00)) | ((((DWORD)color) & 0xFF0000) >> 16));
};

void NppParameters::writeStyles(LexerStylerArray & lexersStylers, StyleArray & globalStylers)
{
	TiXmlNode *lexersRoot = (_pXmlUserStylerDoc->FirstChild("NotepadPlus"))->FirstChildElement("LexerStyles");
	for (TiXmlNode *childNode = lexersRoot->FirstChildElement("LexerType");
		childNode ;
		childNode = childNode->NextSibling("LexerType"))
	{
		TiXmlElement *element = childNode->ToElement();
		const char *nm = element->Attribute("name");
		
		LexerStyler *pLs = _lexerStylerArray.getLexerStylerByName(nm);
        LexerStyler *pLs2 = lexersStylers.getLexerStylerByName(nm);

		const char *extStr = pLs->getLexerUserExt();
		//pLs2->setLexerUserExt(extStr);
	    element->SetAttribute("ext", extStr);


		if (pLs) 
		{
			for (TiXmlNode *grChildNode = childNode->FirstChildElement("WordsStyle");
					grChildNode ;
					grChildNode = grChildNode->NextSibling("WordsStyle"))
			{
				TiXmlElement *grElement = grChildNode->ToElement();
                const char *styleName = grElement->Attribute("name");

                int i = pLs->getStylerIndexByName(styleName);
                if (i != -1)
				{
                    Style & style = pLs->getStyler(i);
                    Style & style2Sync = pLs2->getStyler(i);

                    writeStyle2Element(style, style2Sync, grElement);
                }
			}
		}
	}

	TiXmlNode *globalStylesRoot = (_pXmlUserStylerDoc->FirstChild("NotepadPlus"))->FirstChildElement("GlobalStyles");

    for (TiXmlNode *childNode = globalStylesRoot->FirstChildElement("WidgetStyle");
		childNode ;
		childNode = childNode->NextSibling("WidgetStyle"))
    {
        TiXmlElement *pElement = childNode->ToElement();
        const char *styleName = pElement->Attribute("name");
        int i = _widgetStyleArray.getStylerIndexByName(styleName);

        if (i != -1)
		{
            Style & style = _widgetStyleArray.getStyler(i);
            Style & style2Sync = globalStylers.getStyler(i);

            writeStyle2Element(style, style2Sync, pElement);
        }
    }

	_pXmlUserStylerDoc->SaveFile();
}

void NppParameters::writeStyle2Element(Style & style2Wite, Style & style2Sync, TiXmlElement *element)
{
    const char *styleName = element->Attribute("name");

    if (HIBYTE(HIWORD(style2Wite._fgColor)) != 0xFF)
    {
        int rgbVal = RGB2int(style2Wite._fgColor);
	    char fgStr[7];
	    sprintf(fgStr, "%.6X", rgbVal);
	    element->SetAttribute("fgColor", fgStr);
    }

    if (HIBYTE(HIWORD(style2Wite._bgColor)) != 0xFF)
    {
        int rgbVal = RGB2int(style2Wite._bgColor);
	    char bgStr[7];
	    sprintf(bgStr, "%.6X", rgbVal);
	    element->SetAttribute("bgColor", bgStr);
    }

    if (style2Wite._fontName)
    {
        const char *oldFontName = element->Attribute("fontName");
        if (strcmp(oldFontName, style2Wite._fontName))
        {
		    element->SetAttribute("fontName", style2Wite._fontName);
            style2Sync._fontName = style2Wite._fontName = element->Attribute("fontName");
        }
    }

    if (style2Wite._fontSize != -1)
    {
        if (!style2Wite._fontSize)
            element->SetAttribute("fontSize", "");
        else
		    element->SetAttribute("fontSize", style2Wite._fontSize);
    }

    if (style2Wite._fontStyle != -1)
    {
	    element->SetAttribute("fontStyle", style2Wite._fontStyle);
    }

	
	if (style2Wite._keywords)
    {	
		TiXmlNode *teteDeNoeud = element->LastChild();

		if (teteDeNoeud)
			teteDeNoeud->SetValue(style2Wite._keywords->c_str());
		else 
			element->InsertEndChild(TiXmlText(style2Wite._keywords->c_str()));
    }
}

void NppParameters::insertUserLang2Tree(TiXmlNode *node, UserLangContainer *userLang) 
{
	TiXmlElement *rootElement = (node->InsertEndChild(TiXmlElement("UserLang")))->ToElement();

	rootElement->SetAttribute("name", userLang->_name);
	rootElement->SetAttribute("ext", userLang->_ext);

	TiXmlElement *kwlElement = (rootElement->InsertEndChild(TiXmlElement("KeywordLists")))->ToElement();
	char kwn[8][16] = {"Folder+","Folder-","Operators","Comment","Words1","Words2","Words3","Words4"};
	for (int i = 0 ; i < 8 ; i++)
	{
		TiXmlElement *kwElement = (kwlElement->InsertEndChild(TiXmlElement("Keywords")))->ToElement();
		kwElement->SetAttribute("name", kwn[i]);
		kwElement->InsertEndChild(TiXmlText(userLang->_keywordLists[i+1]));
	}

	TiXmlElement *styleRootElement = (rootElement->InsertEndChild(TiXmlElement("Styles")))->ToElement();

	for (int i = 0 ; i < userLang->_styleArray.getNbStyler() ; i++)
	{
		TiXmlElement *styleElement = (styleRootElement->InsertEndChild(TiXmlElement("WordsStyle")))->ToElement();
		Style style2Wite = userLang->_styleArray.getStyler(i);

		styleElement->SetAttribute("name", style2Wite._styleDesc);

		styleElement->SetAttribute("styleID", style2Wite._styleID);

		//if (HIBYTE(HIWORD(style2Wite._fgColor)) != 0xFF)
		{
			int rgbVal = RGB2int(style2Wite._fgColor);
			char fgStr[7];
			sprintf(fgStr, "%.6X", rgbVal);
			styleElement->SetAttribute("fgColor", fgStr);
		}

		//if (HIBYTE(HIWORD(style2Wite._bgColor)) != 0xFF)
		{
			int rgbVal = RGB2int(style2Wite._bgColor);
			char bgStr[7];
			sprintf(bgStr, "%.6X", rgbVal);
			styleElement->SetAttribute("bgColor", bgStr);
		}

		if (style2Wite._fontName)
		{
			styleElement->SetAttribute("fontName", style2Wite._fontName);
		}

		if (style2Wite._fontStyle == -1)
		{
			styleElement->SetAttribute("fontStyle", "0");
		}
		else
		{
			styleElement->SetAttribute("fontStyle", style2Wite._fontStyle);
		}

		if (style2Wite._fontSize != -1)
		{
			if (!style2Wite._fontSize)
				styleElement->SetAttribute("fontSize", "");
			else
				styleElement->SetAttribute("fontSize", style2Wite._fontSize);
		}
	}
}

void NppParameters::stylerStrOp(bool op) 
{
	for (int i = 0 ; i < _nbUserLang ; i++)
	{
		int nbStyler = _userLangArray[i]->_styleArray.getNbStyler();
		for (int j = 0 ; j < nbStyler ; j++)
		{
			Style & style = _userLangArray[i]->_styleArray.getStyler(j);
			
			if (op == DUP)
			{
				char *str = new char[strlen(style._styleDesc) + 1];
				style._styleDesc = strcpy(str, style._styleDesc);
				if (style._fontName)
				{
					str = new char[strlen(style._fontName) + 1];
					style._fontName = strcpy(str, style._fontName);
				}
				else
				{
					str = new char[2];
					str[0] = str[1] = '\0';
					style._fontName = str;
				}
			}
			else
			{
				delete [] style._styleDesc;
				delete [] style._fontName;
			}
		}
	}
}

