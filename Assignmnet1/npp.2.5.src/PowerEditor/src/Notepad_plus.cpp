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

#include <shlwapi.h>
#include "Notepad_plus.h"
#include "SysMsg.h"
#include "FileDialog.h"
#include "resource.h"
#include "printer.h"
//#include "Process.h"
#include "FileNameStringSplitter.h"
#include "lesDlgs.h"
#include "Utf8_16.h"

const char Notepad_plus::_className[32] = NOTEPAD_PP_CLASS_NAME;

Notepad_plus::Notepad_plus(): Window(), _mainWindowStatus(0), _pDocTab(NULL), _pEditView(NULL),
	_pMainSplitter(NULL), _hTabPopupMenu(NULL), _hTabPopupDropMenu(NULL) 
{

	TiXmlDocument *nativeLangDocRoot = (NppParameters::getInstance())->getNativeLang();
	if (nativeLangDocRoot)
	{
		_nativeLang =  nativeLangDocRoot->FirstChild("NotepadPlus");
		if (_nativeLang)
			_nativeLang = _nativeLang->FirstChild("Native-Langue");
	}
	else
		_nativeLang = NULL;
	
	TiXmlDocument *toolIconsDocRoot = (NppParameters::getInstance())->getToolIcons();
	if (toolIconsDocRoot)
	{
		_toolIcons =  toolIconsDocRoot->FirstChild("NotepadPlus");
		if (_toolIcons)
			if (_toolIcons = _toolIcons->FirstChild("ToolBarIcons"))
			{
				if (_toolIcons = _toolIcons->FirstChild("Theme"))
				{
					const char *themeDir = (_toolIcons->ToElement())->Attribute("pathPrefix");

					for (TiXmlNode *childNode = _toolIcons->FirstChildElement("Icon");
						 childNode ;
						 childNode = childNode->NextSibling("Icon") )
					{
						int iIcon;
						const char *res = (childNode->ToElement())->Attribute("id", &iIcon);
						if (res)
						{
							TiXmlNode *grandChildNode = childNode->FirstChildElement("normal");
							if (grandChildNode)
							{
								TiXmlNode *valueNode = grandChildNode->FirstChild();
								//putain, enfin!!!
								if (valueNode)
								{
									std::string locator = themeDir?themeDir:"";
									
									locator += valueNode->Value();
									_customIconVect.push_back(iconLocator(0, iIcon, locator));
								}
							}

							grandChildNode = childNode->FirstChildElement("hover");
							if (grandChildNode)
							{
								TiXmlNode *valueNode = grandChildNode->FirstChild();
								//putain, enfin!!!
								if (valueNode)
								{
									std::string locator = themeDir?themeDir:"";
									
									locator += valueNode->Value();
									_customIconVect.push_back(iconLocator(1, iIcon, locator));
								}
							}

							grandChildNode = childNode->FirstChildElement("disabled");
							if (grandChildNode)
							{
								TiXmlNode *valueNode = grandChildNode->FirstChild();
								//putain, enfin!!!
								if (valueNode)
								{
									std::string locator = themeDir?themeDir:"";
									
									locator += valueNode->Value();
									_customIconVect.push_back(iconLocator(2, iIcon, locator));
								}
							}
						}
					}
				}
			}
	}
	else
		_toolIcons = NULL;
}


void Notepad_plus::init(HINSTANCE hInst, HWND parent, const char *cmdLine)
{
	Window::init(hInst, parent);
    
	WNDCLASS nppClass;

	nppClass.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	nppClass.lpfnWndProc = Notepad_plus_Proc;
	nppClass.cbClsExtra = 0;
	nppClass.cbWndExtra = 0;
	nppClass.hInstance = _hInst;
	nppClass.hIcon = ::LoadIcon(_hInst, MAKEINTRESOURCE(IDI_M30ICON));
	nppClass.hCursor = NULL;
	nppClass.hbrBackground = ::CreateSolidBrush(::GetSysColor(COLOR_MENU));
	nppClass.lpszMenuName = MAKEINTRESOURCE(IDR_M30_MENU);
	nppClass.lpszClassName = _className;

	if (!::RegisterClass(&nppClass))
	{
		systemMessage("System Err");
		throw int(98);
	}
//::MessageBox(NULL, "Avant Choas", "npp", MB_OK);
	_hSelf = ::CreateWindowEx(
					WS_EX_ACCEPTFILES/*WS_EX_CLIENTEDGE*/,\
					_className,\
					"Notepad++",\
					WS_OVERLAPPEDWINDOW	| WS_CLIPCHILDREN,\
					CW_USEDEFAULT, CW_USEDEFAULT,\
					CW_USEDEFAULT, CW_USEDEFAULT,\
					_hParent,\
					NULL,\
					_hInst,\
					(LPVOID)this); // pass the ptr of this instantiated object
                                   // for retrive it in Notepad_plus_Proc from 
                                   // the CREATESTRUCT.lpCreateParams afterward.
//::MessageBox(NULL, "apres Choas", "npp", MB_OK);  
	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(777);
	}

    if (cmdLine)
    {
        FileNameStringSplitter fnss(cmdLine);
        char *pFn = NULL;
		
		LangType lt = (NppParameters::getInstance())->getDefLang();
        for (int i = 0 ; i < fnss.size() ; i++)
        {
            pFn = (char *)fnss.getFileName(i);
            doOpen((const char *)pFn);
			
			if (lt != L_TXT)
			{
				_pEditView->setCurrentDocType(lt);
				setLangStatus(lt);
			}
        }
		(NppParameters::getInstance())->setDefLang(L_TXT);
    }
	
	::GetModuleFileName(NULL, _nppPath, MAX_PATH);
	setTitle(_className);
	display();
	checkDocState();
}

bool Notepad_plus::doOpen(const char *fileName) 
{
	int i = - 1;
	if ( (i = _pDocTab->find(fileName)) != -1)
	{
		setTitleWith(_pDocTab->activate(i));
		_pEditView->getFocus();
		return false;
	}

	Utf8_16_Read UnicodeConvertor;

    bool isNewDoc2Close = false;
	FILE *fp = fopen(fileName, "rb");
    
	if (fp)
	{
        if ((_pEditView->getNbDoc() == 1) 
			&& Buffer::isUntitled(_pEditView->getCurrentTitle())
            && (!_pEditView->isCurrentDocDirty()) && (_pEditView->getCurrentDocLen() == 0))
        {
            //_pDocTab->closeCurrentDoc();
            isNewDoc2Close = true;
        }
		setTitleWith(_pDocTab->newDoc(fileName));

		// It's VERY IMPORTANT to reset the view
		_pEditView->execute(SCI_CLEARALL);

		const int blockSize = 128 * 1024;
		char data[blockSize];

		int lenFile = int(fread(data, 1, sizeof(data), fp));
		
		while (lenFile > 0) 
		{
			lenFile = UnicodeConvertor.convert(data, lenFile);
			_pEditView->execute(SCI_ADDTEXT, lenFile, reinterpret_cast<LPARAM>(UnicodeConvertor.getNewBuf()));
			lenFile = int(fread(data, 1, sizeof(data), fp));
		}
		fclose(fp);
		
		// 3 formats : WIN_FORMAT, UNIX_FORMAT and MAC_FORMAT
		(_pEditView->getCurrentBuffer()).determinateFormat(UnicodeConvertor.getNewBuf());
		_pEditView->execute(SCI_SETEOLMODE, _pEditView->getCurrentBuffer().getFormat());

		UniMode unicodeMode = static_cast<UniMode>(UnicodeConvertor.getEncoding());
		(_pEditView->getCurrentBuffer()).setUnicodeMode(unicodeMode);

		if (unicodeMode != uni8Bit)
			// Override the code page if Unicode
			_pEditView->execute(SCI_SETCODEPAGE, SC_CP_UTF8);
		

		//_pDocTab->activate(_pEditView->getCurrentDocIndex());
		_pEditView->getFocus();
		_pEditView->execute(SCI_SETSAVEPOINT);
		_pEditView->execute(EM_EMPTYUNDOBUFFER);

		// if file is read only, we set the view read only
		_pEditView->execute(SCI_SETREADONLY, _pEditView->isCurrentBufReadOnly());
        if (isNewDoc2Close)
            _pDocTab->closeDocAt(0);

		// Then replace the caret to the begining
		_pEditView->execute(SCI_GOTOPOS, 0);
		dynamicCheckMenuAndTB();

		return true;
	}
	else
	{
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		//strcat(msg, fullPath);
		strcat(msg, fileName);
		strcat(msg, "\".");
		::MessageBox(_hSelf, msg, "ERR", MB_OK);
		return false;
	}
}

void Notepad_plus::fileOpen()
{
    FileDialog fDlg(_hSelf, _hInst, true);
	
    fDlg.setExtFilter("All types", ".*", NULL);
    fDlg.setExtFilter("c/c++ src file", ".c", ".cpp", ".cxx", ".cc", ".h", NULL);
    fDlg.setExtFilter("Window Resource File", ".rc", NULL);

    fDlg.setExtFilter("Java src file", ".java", NULL);
    fDlg.setExtFilter("HTML file", ".html", ".htm", NULL);
    fDlg.setExtFilter("XML file", ".xml", NULL);
    fDlg.setExtFilter("Makefile", "makefile", "GNUmakefile", ".makefile", NULL);
    fDlg.setExtFilter("php file", ".php", ".phtml", NULL);
    fDlg.setExtFilter("asp file", ".asp", NULL);
    fDlg.setExtFilter("ini file", ".ini", NULL);
    fDlg.setExtFilter("nfo file", ".nfo", NULL);
    fDlg.setExtFilter("VB/VBS file", ".vb", ".vbs", NULL);
    fDlg.setExtFilter("SQL file", ".sql", NULL);
    fDlg.setExtFilter("Objective C file", ".m", ".h", NULL);
	if (stringVector *pfns = fDlg.doOpenDlg())
	{
		int sz = int(pfns->size());
		for (int i = 0 ; i < sz ; i++)
			doOpen((pfns->at(i)).c_str());
	}
}

bool Notepad_plus::doSave(const char *filename, UniMode mode)
{
	Utf8_16_Write UnicodeConvertor;
	UnicodeConvertor.setEncoding(static_cast<Utf8_16::encodingType>(mode));

	FILE *fp = UnicodeConvertor.fopen(filename, "wb");
	const int blockSize = 128 * 1024;
	if (fp)
	{
		char data[blockSize + 1];
		int lengthDoc = _pEditView->getCurrentDocLen();
		for (int i = 0; i < lengthDoc; i += blockSize)
		{
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize) 
				grabSize = blockSize;
			
			_pEditView->getText(data, i, i + grabSize);
			UnicodeConvertor.fwrite(data, grabSize);
		}
		UnicodeConvertor.fclose();
		//checkIfReadOnlyFile()
		_pEditView->updateCurrentBufTimeStamp();
		_pEditView->execute(SCI_SETSAVEPOINT);
		return true;
	}
	return false;
}

bool Notepad_plus::fileSave()
{
	if (_pEditView->isCurrentDocDirty())
	{
		const char *fn = _pEditView->getCurrentTitle();
		return (Buffer::isUntitled(fn))?fileSaveAs():doSave(fn, _pEditView->getCurrentBuffer().getUnicodeMode());
	}
	return false;
}

bool Notepad_plus::fileSaveAll() {

	int iCurrent = _pEditView->getCurrentDocIndex();

    if (_mainWindowStatus & TWO_VIEWS_MASK)
    {
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
        int iCur = _pEditView->getCurrentDocIndex();

	    for (int i = 0 ; i < _pEditView->getNbDoc() ; i++)
	    {
		    _pDocTab->activate(i);
		    fileSave();
	    }

        _pDocTab->activate(iCur);
        
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
    }
    
    for (int i = 0 ; i < _pEditView->getNbDoc() ; i++)
	{
		_pDocTab->activate(i);
		fileSave();
	}

	_pDocTab->activate(iCurrent);
	return true;
}

bool Notepad_plus::fileSaveAs()
{
/*	FileDialog fDlg(_hSelf, _hInst, false);*/

	FileDialog fDlg(_hSelf, _hInst, false, styleSave);

    fDlg.setExtFilter("All types", ".*", NULL);

    fDlg.setExtFilter("c src file", ".c", NULL);
    fDlg.setExtFilter("c++ src file", ".cpp", NULL);
    fDlg.setExtFilter("Window Resource File", ".rc", NULL);
    fDlg.setExtFilter("c/c++ header file", ".h", NULL);
    fDlg.setExtFilter("Java src file", ".java", NULL);
    fDlg.setExtFilter("HTML file", ".html", NULL);
    fDlg.setExtFilter("XML file", ".xml", NULL);
    fDlg.setExtFilter("php file", ".php",NULL);
    fDlg.setExtFilter("ini file", ".ini", NULL);
    fDlg.setExtFilter("Normal text file", ".txt", NULL);

	if (char *pfn = fDlg.doSaveDlg())
	{
		int i = _pEditView->findDocIndexByName(pfn);
		if ((i == -1) || (i == _pEditView->getCurrentDocIndex()))
		{
			doSave(pfn, _pEditView->getCurrentBuffer().getUnicodeMode());
			_pEditView->setCurrentTitle(pfn);
            _pEditView->setCurrentDocReadOnly(false);
            _pEditView->execute(SCI_SETREADONLY, false);
			_pDocTab->updateCurrentTabItem(PathFindFileName(pfn));
			setTitleWith(pfn);
			return true;
		}
		else
		{
			::MessageBox(_hSelf, "The file is already opened in the Notepad++.", "ERROR", MB_OK | MB_ICONSTOP);
			_pDocTab->activate(i);
			return false;
		}
        checkModifiedDocument();
	}
	else // cancel button is pressed
    {
        checkModifiedDocument();
		return false;
    }
}

void Notepad_plus::filePrint(bool showDialog)
{
	Printer printer;

	int startPos = int(_pEditView->execute(SCI_GETSELECTIONSTART));
	int endPos = int(_pEditView->execute(SCI_GETSELECTIONEND));
	
	printer.init(_hInst, _hSelf, showDialog, startPos, endPos);
	printer.doPrint(*_pEditView);
}

void Notepad_plus::enableCommand(int cmdID, bool doEnable, int which) const
{
	if (which & MENU)
	{
		enableMenu(cmdID, doEnable);
	}
	if (which & TOOLBAR)
	{
		_toolBar.enable(cmdID, doEnable);
	}
}

void Notepad_plus::checkClipboard() 
{
	bool hasSelection = _pEditView->execute(SCI_GETSELECTIONSTART) != _pEditView->execute(SCI_GETSELECTIONEND);
	bool canPaste = bool(_pEditView->execute(SCI_CANPASTE));
	enableCommand(IDM_EDIT_CUT, hasSelection, MENU | TOOLBAR); 
	enableCommand(IDM_EDIT_COPY, hasSelection, MENU | TOOLBAR);
	enableCommand(IDM_EDIT_PASTE, canPaste, MENU | TOOLBAR);
}

void Notepad_plus::checkDocState()
{
	bool isCurrentDirty = _pEditView->isCurrentDocDirty();
	bool isSeveralDirty = !_pEditView->isAllDocsClean();
	enableCommand(IDM_FILE_SAVE, isCurrentDirty, MENU | TOOLBAR);
	enableCommand(IDM_FILE_SAVEALL, isSeveralDirty, MENU | TOOLBAR);
	enableConvertMenuItems((_pEditView->getCurrentBuffer()).getFormat());
	checkLangsMenu(-1);
}

void Notepad_plus::checkUndoState()
{
	enableCommand(IDM_EDIT_UNDO, (bool)_pEditView->execute(SCI_CANUNDO), MENU | TOOLBAR);
	enableCommand(IDM_EDIT_REDO, (bool)_pEditView->execute(SCI_CANREDO), MENU | TOOLBAR);
}

void Notepad_plus::synchronise()
{
    Buffer & bufSrc = _pEditView->getCurrentBuffer();
    
    const char *fn = bufSrc.getFileName();

    int i = getNonCurrentDocTab()->find(fn);
    if (i != -1)
    {
        Buffer & bufDest = getNonCurrentEditView()->getBufferAt(i);
        bufDest.synchroniseWith(bufSrc);
        getNonCurrentDocTab()->updateTabItem(i);
    }
}

void Notepad_plus::setLangStatus(LangType langType)
{
    std::string str2Show;

    switch (langType)
    {
	case L_C:
		str2Show = "c source file"; break;

	case L_CPP:
		str2Show = "c++ source file"; break;

	case L_OBJC:
		str2Show = "Objective C source file"; break;

	case L_JAVA:
		str2Show = "Java source file"; break;

	case L_CS:
		str2Show = "C# source file"; break;

    case L_RC :
        str2Show = "Windows Resource file"; break;
    
    case L_MAKEFILE:
        str2Show = "Makefile"; break;

	case L_HTML:
        str2Show = "Hyper Text Markup Language file"; break;

    case L_XML:
        str2Show = "eXtensible Markup Language file"; break;

	case L_JS:
        str2Show = "Javascript file"; break;

	case L_PHP:
        str2Show = "Personal Home Page language file"; break;

	case L_ASP:
        str2Show = "Active Server Pages script file"; break;

	case L_CSS:
        str2Show = "Cascade Style Sheets File"; break;

	case L_LUA:
        str2Show = "Lua source File"; break;

    case L_NFO:
        str2Show = "M$ Dos Style"; break;

    case L_SQL:
        str2Show = "Structure Query Lanquage file"; break;

    case L_VB:
        str2Show = "Visual Basic file"; break;

    case L_BATCH :
        str2Show = "Batch file"; break;

	case L_PASCAL :
        str2Show = "Pascal source file"; break;

	case L_PERL :
        str2Show = "Perl source file"; break;

	case L_PYTHON :
        str2Show = "Python file"; break;

	case L_TEX : 
		str2Show = "TeX file"; break;

	case L_FORTRAN : 
		str2Show = "Fortran source file"; break;

	case L_BASH : 
		str2Show = "Unix script file"; break;

	case L_FLASH :
		str2Show = "Flash Action script file"; break;

	case L_NSIS :
		str2Show = "Nullsoft Scriptable Install System script file"; break;

	case L_USER:
	{
        str2Show = "User Define File";
		Buffer & currentBuf = _pEditView->getCurrentBuffer();
		if (currentBuf.isUserDefineLangExt())
		{
			str2Show += " - ";
			str2Show += currentBuf.getUserDefineLangName();
		}
        break;
	}

    default:
        str2Show = "Normal text file";

    }
    _statusBar.setText(str2Show.c_str(), STATUSBAR_DOC_TYPE);
}

void Notepad_plus::getApiFileName(LangType langType, std::string &fn)
{

    switch (langType)
    {
	case L_C: fn = "c";	break;

	case L_CPP:	fn = "cpp";	break;

	case L_OBJC: fn = "objC"; break;

	case L_JAVA: fn = "java"; break;

    case L_CS : fn = "cs"; break;

    case L_XML: fn = "xml"; break;

	case L_JS: fn = "javascript"; break;

	case L_PHP: fn = "php"; break;

	case L_VB:
	case L_ASP: fn = "vb"; break;

    case L_CSS: fn = "css"; break;

    case L_LUA: fn = "lua"; break;

    case L_PERL: fn = "perl"; break;

    case L_PASCAL: fn = "pascal"; break;

    case L_PYTHON: fn = "python"; break;

	case L_TEX : fn = "tex"; break;

	case L_FORTRAN : fn = "fortran"; break;

	case L_BASH : fn = "bash"; break;

	case L_FLASH :  fn = "flash"; break;

	case L_NSIS :  fn = "nsis"; break;

    default:
        fn = "";

    }
    //strcpy(fileName, fn.c_str());
}

void Notepad_plus::notify(SCNotification *notification)
{
  switch (notification->nmhdr.code) 
  {
	case SCN_DOUBLECLICK :
		//MessageBox(NULL, "DBL click", "SCN_DOUBLECLICK", MB_OK);
      break;

    case SCN_SAVEPOINTREACHED:
      _pEditView->setCurrentDocState(false);
	  _pDocTab->updateCurrentTabItem();
	  checkDocState();
      synchronise();
      break;

    case SCN_SAVEPOINTLEFT:
      _pEditView->setCurrentDocState(true);
	  _pDocTab->updateCurrentTabItem();
	  checkDocState();
      synchronise();
      break;
    
    case  SCN_MODIFYATTEMPTRO :
      // on fout rien
      break;
	
	case SCN_KEY:
      //MessageBox(NULL, "Putain, sa mere!", "toto", MB_OK);
      break;

	case TCN_TABDROPPEDOUTSIDE:
	case TCN_TABDROPPED:
	{
        TabBar *sender = reinterpret_cast<TabBar *>(notification->nmhdr.idFrom);
        int destIndex = sender->getTabDraggedIndex();
		int scrIndex  = sender->getSrcTabIndex();

		// if the dragNdrop tab is not the current view tab,
		// we have to set it to the current view tab
		if (notification->nmhdr.hwndFrom != _pDocTab->getHSelf())
			switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

        _pEditView->sortBuffer(destIndex, scrIndex);
        _pEditView->setCurrentIndex(destIndex);

        if (notification->nmhdr.code == TCN_TABDROPPEDOUTSIDE)
        {
            POINT p = sender->getDraggingPoint();

			//It's the coordinate of screen, so we can call 
			//"WindowFromPoint" function without converting the point
            HWND hWin = ::WindowFromPoint(p);
			if (hWin == _pEditView->getHSelf()) // In the same view group
			{
				if (!_hTabPopupDropMenu)
				{
					char goToView[64] = "Go to another View";
					char cloneToView[64] = "Clone to another View";
					const char *pGoToView = goToView;
					const char *pCloneToView = cloneToView;

					if (_nativeLang)
					{
						TiXmlNode *tabBarMenu = _nativeLang->FirstChild("Menu");
						tabBarMenu = tabBarMenu->FirstChild("TabBar");
						if (tabBarMenu)
						{
							for (TiXmlNode *childNode = tabBarMenu->FirstChildElement("Item");
								childNode ;
								childNode = childNode->NextSibling("Item") )
							{
								TiXmlElement *element = childNode->ToElement();
								int ordre;
								element->Attribute("order", &ordre);
								if (ordre == 5)
									pGoToView = element->Attribute("name");
								else if (ordre == 6)
									pCloneToView = element->Attribute("name");
							}
						}	
						if (!pGoToView || !pGoToView[0])
							pGoToView = goToView;
						if (!pCloneToView || !pCloneToView[0])
							pCloneToView = cloneToView;
					}
					_hTabPopupDropMenu = ::CreatePopupMenu();
					::InsertMenu(_hTabPopupDropMenu, 0, MF_BYPOSITION, IDC_DOC_GOTO_ANOTHER_VIEW, pGoToView);
					::InsertMenu(_hTabPopupDropMenu, 1, MF_BYPOSITION, IDC_DOC_CLONE_TO_ANOTHER_VIEW, pCloneToView);
				}
				::TrackPopupMenu(_hTabPopupDropMenu, TPM_LEFTALIGN, p.x, p.y, 0, _hSelf, NULL);
			}
			else if ((hWin == getNonCurrentDocTab()->getHSelf()) || 
				     (hWin == getNonCurrentEditView()->getHSelf())) // In the another view group
			{
				if (::GetKeyState(VK_LCONTROL) & 0x80000000)
					docGotoAnotherEditView(MODE_CLONE);
				else
					docGotoAnotherEditView(MODE_TRANSFER);
			}
			//else on fout rien!!! // It's non view group
				//::MessageBox(NULL, "chez qqn d'autre", "", MB_OK);
        }
		break;
	}

	case TCN_SELCHANGE:
	{
        char *fullPath = NULL;

        if (notification->nmhdr.hwndFrom == _mainDocTab.getHSelf())
		{
			fullPath = _mainDocTab.clickedUpdate();
            switchEditViewTo(MAIN_VIEW);
			
		}
		else if (notification->nmhdr.hwndFrom == _subDocTab.getHSelf())
		{
			fullPath = _subDocTab.clickedUpdate();
            switchEditViewTo(SUB_VIEW);
		}
        checkDocState();
		break;
	}

	case NM_CLICK :
    {        
		if (notification->nmhdr.hwndFrom == _statusBar.getHSelf())
        {
            LPNMMOUSE lpnm = (LPNMMOUSE)notification;
			if (lpnm->dwItemSpec == STATUSBAR_TYPING_MODE)
			{
				bool isOverTypeMode = _pEditView->execute(SCI_GETOVERTYPE);
				_pEditView->execute(SCI_SETOVERTYPE, !isOverTypeMode);
				_statusBar.setText((_pEditView->execute(SCI_GETOVERTYPE))?"OVR":"INS", STATUSBAR_TYPING_MODE);
			}
        }
		break;
	}

	case NM_DBLCLK :
    {        
		if (notification->nmhdr.hwndFrom == _statusBar.getHSelf())
        {
            LPNMMOUSE lpnm = (LPNMMOUSE)notification;
			if (lpnm->dwItemSpec == STATUSBAR_CUR_POS)
			{
				bool isFirstTime = !_goToLineDlg.isCreated();
				_goToLineDlg.doDialog();
				if (isFirstTime)
					changeDlgLang(DLG_GOTOLINE);
			}
        }
		break;
	}

    case NM_RCLICK :
    {        
		if (notification->nmhdr.hwndFrom == _mainDocTab.getHSelf())
		{
            switchEditViewTo(MAIN_VIEW);
		}
        else if (notification->nmhdr.hwndFrom == _subDocTab.getHSelf())
        {
            switchEditViewTo(SUB_VIEW);
        }
		else // From tool bar or Status Bar
			break;
        
		POINT p, clientPoint;
		::GetCursorPos(&p);
        clientPoint.x = p.x;
        clientPoint.y = p.y;
        
        if (!_hTabPopupMenu)
		{
			char close[32] = "Close me";
			char closeBut[32] = "Close all but me";
			char save[32] = "Save me";
			char saveAs[32] = "Save me As...";
			char print[32] = "Print me";
			char goToView[32] = "Go to another View";
			char cloneToView[32] = "Clone to another View";

			const char *pClose = close;
			const char *pCloseBut = closeBut;
			const char *pSave = save;
			const char *pSaveAs = saveAs;
			const char *pPrint = print;
			const char *pGoToView = goToView;
			const char *pCloneToView = cloneToView;

			if (_nativeLang)
			{
				TiXmlNode *tabBarMenu = _nativeLang->FirstChild("Menu");
				if (tabBarMenu) 
				{
					tabBarMenu = tabBarMenu->FirstChild("TabBar");
					if (tabBarMenu)
					{
						for (TiXmlNode *childNode = tabBarMenu->FirstChildElement("Item");
							childNode ;
							childNode = childNode->NextSibling("Item") )
						{
							TiXmlElement *element = childNode->ToElement();
							int ordre;
							element->Attribute("order", &ordre);
							switch (ordre)
							{
								case 0 :
									pClose = element->Attribute("name"); break;
								case 1 :
									pCloseBut = element->Attribute("name"); break;
								case 2 :
									pSave = element->Attribute("name"); break;
								case 3 :
									pSaveAs = element->Attribute("name"); break;
								case 4 :
									pPrint = element->Attribute("name"); break;
								case 5 :
									pGoToView = element->Attribute("name"); break;
								case 6 :
									pCloneToView = element->Attribute("name"); break;

							}
						}
					}	
				}
				if (!pClose || !pClose[0])
					pClose = close;
				if (!pCloseBut || !pCloseBut[0])
					pCloseBut = cloneToView;
				if (!pSave || !pSave[0])
					pSave = save;
				if (!pSaveAs || !pSaveAs[0])
					pSaveAs = saveAs;
				if (!pPrint || !pPrint[0])
					pPrint = print;
				if (!pGoToView || !pGoToView[0])
					pGoToView = goToView;
				if (!pCloneToView || !pCloneToView[0])
					pCloneToView = cloneToView;
			}
			_hTabPopupMenu = ::CreatePopupMenu();
			::InsertMenu(_hTabPopupMenu, 0, MF_BYPOSITION, IDM_FILE_CLOSE, pClose);
			::InsertMenu(_hTabPopupMenu, 1, MF_BYPOSITION, IDM_FILE_CLOSEALL_BUT_CURRENT, pCloseBut);
			::InsertMenu(_hTabPopupMenu, 2, MF_BYPOSITION, IDM_FILE_SAVE, pSave);
            ::InsertMenu(_hTabPopupMenu, 3, MF_BYPOSITION, IDM_FILE_SAVEAS, pSaveAs);
            ::InsertMenu(_hTabPopupMenu, 4, MF_BYPOSITION, IDM_FILE_PRINT, pPrint);
            
            ::InsertMenu(_hTabPopupMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
            ::InsertMenu(_hTabPopupMenu, 6, MF_BYPOSITION, IDC_DOC_GOTO_ANOTHER_VIEW, pGoToView);
			::InsertMenu(_hTabPopupMenu, 7, MF_BYPOSITION, IDC_DOC_CLONE_TO_ANOTHER_VIEW, pCloneToView);
		}


		//verifyPopMenuItem();
        ScreenToClient(_pDocTab->getHSelf(), &clientPoint);
        ::SendMessage(_pDocTab->getHSelf(), WM_LBUTTONDOWN, 0, MAKELONG(clientPoint.x, clientPoint.y));
        ::TrackPopupMenu(_hTabPopupMenu, TPM_LEFTALIGN, p.x, p.y, 0, _hSelf, NULL);
        break;
    }

    case SCN_MARGINCLICK: 
    {
        if (notification->nmhdr.hwndFrom == _mainEditView.getHSelf())
            switchEditViewTo(MAIN_VIEW);
			
		else if (notification->nmhdr.hwndFrom == _subEditView.getHSelf())
            switchEditViewTo(SUB_VIEW);
        
        if (notification->margin == ScintillaEditView::_SC_MARGE_FOLDER) 
        {
            _pEditView->marginClick(notification->position, notification->modifiers);
        }
        else if (notification->margin == ScintillaEditView::_SC_MARGE_SYBOLE)
        {
            
            int lineClick = int(_pEditView->execute(SCI_LINEFROMPOSITION, notification->position));
            bookmarkToggle(lineClick);
        
        }
		case SCN_CHARADDED:
			charAdded(static_cast<char>(notification->ch));
			break;
	}
	break;

    case SCN_UPDATEUI:
        braceMatch();
		updateStatusBar();
        break;

    case TTN_GETDISPINFO: 
    { 
        LPTOOLTIPTEXT lpttt; 

        lpttt = (LPTOOLTIPTEXT)notification; 
        lpttt->hinst = _hInst; 

        // Specify the resource identifier of the descriptive 
        // text for the given button. 
        int idButton = lpttt->hdr.idFrom; 

		const char *tip;
		
		tip = getNativeTip(idButton);

        switch (idButton) 
		{ 
            case IDM_FILE_NEW:
				lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"New document");
                break; 
            case IDM_FILE_OPEN: 
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Open file");
                break; 
            case IDM_FILE_SAVE: 
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Save current file");
                break;							  
            case IDM_FILE_SAVEALL:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Save all files");
                break; 
            case IDM_FILE_CLOSE: 
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Close current file");
                break;							  
            case IDM_FILE_CLOSEALL:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Close all files");
                break;
            case IDM_EDIT_CUT:					  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Cut");
                break;
            case IDM_EDIT_COPY:					  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Copy");
                break;
            case IDM_EDIT_PASTE:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Paste");
                break;
            case IDM_EDIT_UNDO:					  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Undo");
                break;
            case IDM_EDIT_REDO:					  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Redo");
                break;							  
            case IDM_SEARCH_FIND:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Find...");
                break;
            case IDM_SEARCH_REPLACE: 
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Replace...");
                break;							  
            case IDM_VIEW_ZOOMIN:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Zoom in");
                break;							  
            case IDM_VIEW_ZOOMOUT:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Zoom out");
                break;							  
            case IDM_VIEW_WRAP:					  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Wrap text");
                break;							  
            case IDM_VIEW_ALL_CHARACTERS:		  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Invisible characters");
                break;							  
            case IDM_VIEW_INDENT_GUIDE:			  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"show/hide indent guideline");
                break;
            case IDM_VIEW_USER_DLG: 
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Toggle User Language Define panel");
                break;							  
            case IDC_BUTTON_PRINT:				  
                lpttt->lpszText = (LPSTR)((tip && tip[0])?tip:"Print current document");
                break; 
        } 
        break; 
    } 
    break;

    case SCN_ZOOM:
        _pEditView->setLineNumberWidth(_pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_LINENUMBER));
		break;

	default :
		break;

  }
  //return TRUE;
}
void Notepad_plus::findMatchingBracePos(int & braceAtCaret, int & braceOpposite)
{
	int caretPos = _pEditView->execute(SCI_GETCURRENTPOS, 0, 0);
	braceAtCaret = -1;
	braceOpposite = -1;
	char charBefore = '\0';
	char styleBefore = '\0';
	int lengthDoc = _pEditView->execute(SCI_GETLENGTH, 0, 0);

	if ((lengthDoc > 0) && (caretPos > 0)) 
    {
		charBefore = _pEditView->execute(SCI_GETCHARAT, caretPos - 1, 0);
	}
	// Priority goes to character before caret
	if (charBefore && strchr("[](){}", charBefore))
    {
		braceAtCaret = caretPos - 1;
	}

	if (lengthDoc > 0  && (braceAtCaret < 0)) 
    {
		// No brace found so check other side
		char charAfter = _pEditView->execute(SCI_GETCHARAT, caretPos, 0);
		if (charAfter && strchr("[](){}", charAfter))
        {
			braceAtCaret = caretPos;
		}
	}
	if (braceAtCaret >= 0) 
		braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret, 0);
}

void Notepad_plus::braceMatch() 
{
	//if (!bracesCheck)
		//return;
	int braceAtCaret = -1;
	int braceOpposite = -1;
	findMatchingBracePos(braceAtCaret, braceOpposite);

	if ((braceAtCaret != -1) && (braceOpposite == -1))
    {
		_pEditView->execute(SCI_BRACEBADLIGHT, braceAtCaret);
		_pEditView->execute(SCI_SETHIGHLIGHTGUIDE);
	} 
    else 
    {
		_pEditView->execute(SCI_BRACEHIGHLIGHT, braceAtCaret, braceOpposite);

		if (_pEditView->isShownIndentGuide())
        {
            int columnAtCaret = _pEditView->execute(SCI_GETCOLUMN, braceAtCaret);
		    int columnOpposite = _pEditView->execute(SCI_GETCOLUMN, braceOpposite);
			_pEditView->execute(SCI_SETHIGHLIGHTGUIDE, (columnAtCaret < columnOpposite)?columnAtCaret:columnOpposite);
        }
    }
}

void Notepad_plus::charAdded(char chAdded)
{
	//if (indentMaintain)
		MaintainIndentation(chAdded);
}

void Notepad_plus::MaintainIndentation(char ch) {
	int eolMode = _pEditView->execute(SCI_GETEOLMODE);
	int curLine = _pEditView->getCurrentLineNumber();
	int lastLine = curLine - 1;
	int indentAmount = 0;

	if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
	        (eolMode == SC_EOL_CR && ch == '\r')) 
	{
		//if (props.GetInt("indent.automatic")) {
			while (lastLine >= 0 && _pEditView->getLineLength(lastLine) == 0)
				lastLine--;
		//}
		if (lastLine >= 0) {
			indentAmount = _pEditView->getLineIndent(lastLine);
		}
		if (indentAmount > 0) {
			_pEditView->setLineIndent(curLine, indentAmount);
		}
	}
}

void Notepad_plus::command(int id) 
{
	switch (id)
	{
		case IDM_FILE_NEW:
			fileNew();
			break;
		
		case IDM_FILE_OPEN:
			fileOpen();
			break;

		case IDM_FILE_CLOSE:
			fileClose();
			break;

		case IDM_FILE_CLOSEALL:
			fileCloseAll();
			break;

		case IDM_FILE_CLOSEALL_BUT_CURRENT :
			fileCloseAllButCurrent();
			break;

		case IDM_FILE_SAVE :
			fileSave();
			break;

		case IDM_FILE_SAVEALL :
			fileSaveAll();
			break;

		case IDM_FILE_SAVEAS :
			fileSaveAs();
			break;

		case IDC_BUTTON_PRINT :
			filePrint(false);
			break;

		case IDM_FILE_PRINT :
			filePrint(true);
			break;
/*
        case IDM_FILE_ASIAN_LANG :
            _pEditView->execute(SCI_SETCODEPAGE,  SC_CP_UTF8);
            break;
*/
		case IDM_FILE_EXIT:
			::SendMessage(_hSelf, WM_CLOSE, 0, 0);
			break;

		case IDM_EDIT_UNDO:
			_pEditView->execute(WM_UNDO);
			checkClipboard();
			checkUndoState();
			break;

		case IDM_EDIT_REDO:
			_pEditView->execute(SCI_REDO);
			checkClipboard();
			checkUndoState();
			break;

		case IDM_EDIT_CUT:
			_pEditView->execute(WM_CUT);
			break;

		case IDM_EDIT_COPY:
			_pEditView->execute(WM_COPY);
			checkClipboard();
			break;

		case IDM_EDIT_PASTE:
			_pEditView->execute(WM_PASTE);
			break;

		case IDM_EDIT_DELETE:
			_pEditView->execute(WM_CLEAR);
			break;

		case IDM_SEARCH_FIND :
		case IDM_SEARCH_REPLACE :
		{
			const int strSize = 256;
			char str[strSize];
			if (_pEditView->getSelectedText(str, strSize))
			{
				_findReplaceDlg.doDialog((id == IDM_SEARCH_FIND)?FIND:REPLACE);
				_findReplaceDlg.setSearchText(str);
				changeDlgLang(DLG_FIND);
			}
			else
				::MessageBox(_hSelf, "can not search over 256 characters", "error", MB_OK);
			break;
		}

		case IDM_SEARCH_FINDNEXT :
			_findReplaceDlg.processFindNext();
			break;

		
        case IDM_SEARCH_GOTOLINE :
		{
			bool isFirstTime = !_goToLineDlg.isCreated();
			_goToLineDlg.doDialog();
			if (isFirstTime)
				changeDlgLang(DLG_GOTOLINE);
			break;
		}

        case IDM_SEARCH_TOGGLE_BOOKMARK :
	        bookmarkToggle(-1);
            break;

	    case IDM_SEARCH_NEXT_BOOKMARK:
		    bookmarkNext(true);
		    break;

	    case IDM_SEARCH_PREV_BOOKMARK:
		    bookmarkNext(false);
		    break;

	    case IDM_SEARCH_CLEAR_BOOKMARKS:
			bookmarkClearAll();
		    break;
			
        case IDM_VIEW_USER_DLG :
        {
		    bool isUDDlgVisible = false;
                
		    UserDefineDialog *udd = _pEditView->getUserDefineDlg();
			//changeDlgLang(DLG_USERDIFINE);
		    if (!udd->isCreated())
		    {
			    _pEditView->doUserDefineDlg();
			    // the 1st time isUDDlgVisible should be false
		    }
			else
			{
				isUDDlgVisible = udd->isVisible();
				
				bool isUDDlgDocked = udd->isDocked();
				if ((isUDDlgDocked)&&(isUDDlgVisible))
				{
					::ShowWindow(_pMainSplitter->getHSelf(), SW_HIDE);

					if (_mainWindowStatus & TWO_VIEWS_MASK)
						_pMainWindow = &_subSplitter;
					else
						_pMainWindow = _pDocTab;

					RECT rc;
					getMainClientRect(rc);
					_pMainWindow->reSizeTo(rc);
					
					udd->display(false);
					_mainWindowStatus &= ~DOCK_MASK;
				}
				else if ((isUDDlgDocked)&&(!isUDDlgVisible))
				{
                    if (!_pMainSplitter)
                    {
                        _pMainSplitter = new SplitterContainer;
                        _pMainSplitter->init(_hInst, _hSelf);

                        Window *pWindow;
                        if (_mainWindowStatus & TWO_VIEWS_MASK)
                            pWindow = &_subSplitter;
                        else
                            pWindow = _pDocTab;

                        _pMainSplitter->create(pWindow, ScintillaEditView::getUserDefineDlg(), 8, RIGHT_FIX, 45); 
                    }

					_pMainWindow = _pMainSplitter;

					_pMainSplitter->setWin0((_mainWindowStatus & TWO_VIEWS_MASK)?(Window *)&_subSplitter:(Window *)_pDocTab);

					RECT rc;
					getMainClientRect(rc);
					_pMainWindow->reSizeTo(rc);
					_pMainWindow->display();

					_mainWindowStatus |= DOCK_MASK;
				}
				else if ((!isUDDlgDocked)&&(isUDDlgVisible))
				{
					udd->display(false);
				}
				else //((!isUDDlgDocked)&&(!isUDDlgVisible))
					udd->display();
			}
			checkMenuItem(IDM_VIEW_USER_DLG, !isUDDlgVisible);
			_toolBar.setCheck(IDM_VIEW_USER_DLG, !isUDDlgVisible);

            break;
        }

		case IDM_EDIT_SELECTALL:
			_pEditView->execute(SCI_SELECTALL);
			checkClipboard();
			break;

		case IDM_EDIT_INS_TAB:
			_pEditView->execute(SCI_TAB);
			break;

		case IDM_EDIT_RMV_TAB:
			_pEditView->execute(SCI_BACKTAB);
			break;

		case IDM_EDIT_DUP_LINE:
			_pEditView->execute(SCI_LINEDUPLICATE);
			break;

		case IDM_EDIT_TRANSPOSE_LINE:
			_pEditView->execute(SCI_LINETRANSPOSE);
			break;


		case IDM_EDIT_SPLIT_LINES:
			_pEditView->execute(SCI_TARGETFROMSELECTION);
			_pEditView->execute(SCI_LINESSPLIT);
			break;

		case IDM_EDIT_JOIN_LINES:
			_pEditView->execute(SCI_TARGETFROMSELECTION);
			_pEditView->execute(SCI_LINESJOIN);
			break;

		case IDM_EDIT_LINE_UP:
			_pEditView->currentLineUp();
			break;

		case IDM_EDIT_LINE_DOWN:
			_pEditView->currentLineDown();
			break;

		case IDM_VIEW_TOGGLE_FOLDALL:
		{
			foldAll();
		}
		break;

		case IDM_VIEW_TOOLBAR_HIDE:
		{
            int checkedID = getToolBarState();

            if (checkedID != IDM_VIEW_TOOLBAR_HIDE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.display(false);
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
				checkToolBarMenu(id);
            }
		}
		break;

		case IDM_VIEW_TOOLBAR_REDUCE:
		{
            int checkedID = getToolBarState();

            if (checkedID != IDM_VIEW_TOOLBAR_REDUCE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.reduce();
			    _toolBar.display();
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
				//changeToolBarIcons();
				checkToolBarMenu(id);
            }
		}
		break;

		case IDM_VIEW_TOOLBAR_ENLARGE:
		{
            int checkedID = getToolBarState();
            if (checkedID != IDM_VIEW_TOOLBAR_ENLARGE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.enlarge();
			    _toolBar.display();
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
				changeToolBarIcons();
				checkToolBarMenu(id);
            }
		}
		break;

		case IDM_VIEW_TOOLBAR_STANDARD:
		{            
			int checkedID = getToolBarState();
            if (checkedID != IDM_VIEW_TOOLBAR_STANDARD)
            {
				RECT rc;
				getClientRect(rc);
				_toolBar.setToUglyIcons();
				_toolBar.display();
				::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
				checkToolBarMenu(id);
			}
		}
		break;

		case IDM_VIEW_REDUCETABBAR :
		{
			_toReduceTabBar = !_toReduceTabBar;

			//Resize the  icon
			int iconSize = _toReduceTabBar?12:20;

			//Resize the tab height
			int tabHeight = _toReduceTabBar?20:25;

			//change the font
			int stockedFont = _toReduceTabBar?DEFAULT_GUI_FONT:SYSTEM_FONT;

			TabCtrl_SetItemSize(_mainDocTab.getHSelf(), 45, tabHeight);
			TabCtrl_SetItemSize(_subDocTab.getHSelf(), 45, tabHeight);

			_docTabIconList.setIconSize(iconSize);

			HFONT hf = (HFONT)::GetStockObject(stockedFont);

			if (hf)
			{
				::SendMessage(_mainDocTab.getHSelf(), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
				::SendMessage(_subDocTab.getHSelf(), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
			}
			RECT rc;
            
			getMainClientRect(rc);
            _pMainWindow->reSizeTo(rc);

			checkMenuItem(IDM_VIEW_REDUCETABBAR, _toReduceTabBar);

			break;
		}

        case IDM_VIEW_LOCKTABBAR:
		{
			bool isDrag = TabBar::doDragNDropOrNot();
            TabBar::doDragNDrop(!isDrag);
			checkMenuItem(IDM_VIEW_LOCKTABBAR, isDrag);
            break;
		}

		case IDM_VIEW_DRAWTABBAR_INACIVETAB:
		case IDM_VIEW_DRAWTABBAR_TOPBAR:
		{
			bool drawIt = (id == IDM_VIEW_DRAWTABBAR_TOPBAR)?TabBar::drawTopBar():TabBar::drawInactiveTab();
			if (id == IDM_VIEW_DRAWTABBAR_TOPBAR)
				TabBar::setDrawTopBar(!drawIt);
			else 
				TabBar::setDrawInactiveTab(!drawIt);

			checkMenuItem(id, !drawIt);
			break;
		}

        case IDM_VIEW_STATUSBAR:
            RECT rc;
			getClientRect(rc);
            _statusBar.display((bool)checkStatusBar());
            ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
            break;

        case IDM_VIEW_LINENUMBER:
        case IDM_VIEW_SYMBOLMARGIN:
        case IDM_VIEW_FOLDERMAGIN:
        {
            int margin;
            if (id == IDM_VIEW_LINENUMBER)
                margin = ScintillaEditView::_SC_MARGE_LINENUMBER;
            else if (id == IDM_VIEW_SYMBOLMARGIN)
                margin = ScintillaEditView::_SC_MARGE_SYBOLE;
            else 
                margin = ScintillaEditView::_SC_MARGE_FOLDER;

            if (_pEditView->hasMarginShowed(margin))
                _pEditView->showMargin(margin, false);
            else
                _pEditView->showMargin(margin);

            checkMenuItem(id, _pEditView->hasMarginShowed(margin));
            break;
        }

        case IDM_VIEW_FOLDERMAGIN_SIMPLE:
        case IDM_VIEW_FOLDERMAGIN_ARROW:
        case IDM_VIEW_FOLDERMAGIN_CIRCLE:
        case IDM_VIEW_FOLDERMAGIN_BOX:
        {
            int checkedID = getFolderMarginStyle();
            if (checkedID == id) return;
            folderStyle fStyle = (id == IDM_VIEW_FOLDERMAGIN_SIMPLE)?FOLDER_STYLE_SIMPLE:\
                ((id == IDM_VIEW_FOLDERMAGIN_ARROW)?FOLDER_STYLE_ARROW:\
                ((id == IDM_VIEW_FOLDERMAGIN_CIRCLE)?FOLDER_STYLE_CIRCLE:FOLDER_STYLE_BOX));
            _pEditView->setMakerStyle(fStyle);
            checkFolderMarginStyleMenu(id);
            break;
        }
		case IDM_VIEW_TAB_SPACE:
		case IDM_VIEW_EOL:
		case IDM_VIEW_ALL_CHARACTERS:
		{
			HMENU hMenu = ::GetMenu(_hSelf);
			bool isChecked = !(::GetMenuState(hMenu, id, MF_BYCOMMAND) == MF_CHECKED);
			if (id == IDM_VIEW_TAB_SPACE)
			{
				::CheckMenuItem(hMenu, IDM_VIEW_EOL, MF_BYCOMMAND | MF_UNCHECKED);
				::CheckMenuItem(hMenu, IDM_VIEW_ALL_CHARACTERS, MF_BYCOMMAND | MF_UNCHECKED);
				_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, !isChecked);
				_pEditView->showEOL(false);
				_pEditView->showWSAndTab(isChecked);
			}
			else if (id == IDM_VIEW_EOL)
			{
				::CheckMenuItem(hMenu, IDM_VIEW_TAB_SPACE, MF_BYCOMMAND | MF_UNCHECKED);
				::CheckMenuItem(hMenu, IDM_VIEW_ALL_CHARACTERS, MF_BYCOMMAND | MF_UNCHECKED);
				_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, !isChecked);
				_pEditView->showEOL(isChecked);
				_pEditView->showWSAndTab(false);
			}
			else
			{
				::CheckMenuItem(hMenu, IDM_VIEW_EOL, MF_BYCOMMAND | MF_UNCHECKED);
				::CheckMenuItem(hMenu, IDM_VIEW_TAB_SPACE, MF_BYCOMMAND | MF_UNCHECKED);
				_pEditView->showInvisibleChars(isChecked);
				_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, isChecked);
			}
			::CheckMenuItem(hMenu, id, MF_BYCOMMAND | (isChecked?MF_CHECKED:MF_UNCHECKED));
			break;
		}
			
		case IDM_VIEW_INDENT_GUIDE:
		{
			_pEditView->showIndentGuideLine(!_pEditView->isShownIndentGuide());
            _toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());
			checkMenuItem(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());
			break;
		}

		case IDM_VIEW_EDGEBACKGROUND:
		case IDM_VIEW_EDGELINE:
		{
			HMENU hMenu = ::GetMenu(_hSelf);
			bool isChecked = !(::GetMenuState(hMenu, id, MF_BYCOMMAND) == MF_CHECKED);
			int mode;
			if (id == IDM_VIEW_EDGELINE)
			{
				::CheckMenuItem(hMenu, IDM_VIEW_EDGEBACKGROUND, MF_BYCOMMAND | MF_UNCHECKED);
				::CheckMenuItem(hMenu, IDM_VIEW_EDGELINE, MF_BYCOMMAND | isChecked?MF_CHECKED:MF_UNCHECKED);
				mode = isChecked?EDGE_LINE:EDGE_NONE;
			}
			else 
			{
				::CheckMenuItem(hMenu, IDM_VIEW_EDGELINE, MF_BYCOMMAND | MF_UNCHECKED);
				::CheckMenuItem(hMenu, IDM_VIEW_EDGEBACKGROUND, MF_BYCOMMAND | isChecked?MF_CHECKED:MF_UNCHECKED);
				mode = isChecked?EDGE_BACKGROUND:EDGE_NONE;
			}
			_pEditView->execute(SCI_SETEDGEMODE, mode);
			break;
		}

		case IDM_VIEW_CURLINE_HILITING:
		{
            COLORREF colour = (NppParameters::getInstance())->getCurLineHilitingColour();
			_pEditView->setCurrentLineHiLiting(!_pEditView->isCurrentLineHiLiting(), colour);
			checkMenuItem(IDM_VIEW_CURLINE_HILITING, _pEditView->isCurrentLineHiLiting());
			break;
		}

		case IDM_VIEW_WRAP:
		{
			_pEditView->wrap(!_pEditView->isWrap());
            _toolBar.setCheck(IDM_VIEW_WRAP, _pEditView->isWrap());
			checkMenuItem(IDM_VIEW_WRAP, _pEditView->isWrap());
			break;
		}

		case IDM_VIEW_ZOOMIN:
		{
			_pEditView->execute(SCI_ZOOMIN);
			break;
		}
		case IDM_VIEW_ZOOMOUT:
			_pEditView->execute(SCI_ZOOMOUT);
			break;


		case IDM_EXECUTE:
		{
			bool isFirstTime = !_runDlg.isCreated();
			_runDlg.doDialog();
			if (isFirstTime)
				changeDlgLang(DLG_RUN);

			break;
		}

/*
        case IDC_DOCK_USERDEFINE_DLG :
            break;

        case IDC_UNDOCK_USERDEFINE_DLG :
            break;
*/
		case IDM_FORMAT_TODOS :
		case IDM_FORMAT_TOUNIX :
		case IDM_FORMAT_TOMAC :
		{
			int f = int((id == IDM_FORMAT_TODOS)?SC_EOL_CRLF:(id == IDM_FORMAT_TOUNIX)?SC_EOL_LF:SC_EOL_CR);
			_pEditView->execute(SCI_SETEOLMODE, f);
			_pEditView->execute(SCI_CONVERTEOLS, f);
			(_pEditView->getCurrentBuffer()).setFormat((formatType)f);
			enableConvertMenuItems((formatType)f);
			setDisplayFormat((formatType)f);
			break;
		}
/*		 
			_pEditView->execute(SCI_CONVERTEOLS, SC_EOL_LF);
			enableConvertMenuItems((_pEditView->getCurrentBuffer()).getFormat());
			break;

		
			_pEditView->execute(SCI_CONVERTEOLS, SC_EOL_CR);
			enableConvertMenuItems((_pEditView->getCurrentBuffer()).getFormat());
			break;

*/

		case IDM_FORMAT_ANSI :
		case IDM_FORMAT_UTF_8 :	
		case IDM_FORMAT_UCS_2BE :
		case IDM_FORMAT_UCS_2LE :
		case IDM_FORMAT_AS_UTF_8 :
		{

			//UniMode um = _pEditView->getCurrentBuffer().getUnicodeMode();
			UniMode um;
			bool isUnicodeMode = true;
			switch (id)
			{
				case IDM_FORMAT_ANSI:
					um = uni8Bit;
					isUnicodeMode = false;
					break;
				
				case IDM_FORMAT_UTF_8:
					um = uniUTF8;
					break;

				case IDM_FORMAT_UCS_2BE:
					um = uni16BE;
					break;

				case IDM_FORMAT_UCS_2LE:
					um = uni16LE;
					break;

				default : // IDM_FORMAT_AS_UTF_8
				{
					bool wasChecked = (_pEditView->getCurrentBuffer().getUnicodeMode() == uniCookie);
					if (wasChecked)
					{
						um = uni8Bit;
						isUnicodeMode = false;
					}
					else
					{
						um = uniCookie;
						checkMenuItem(IDM_FORMAT_AS_UTF_8, false);
					}
				}
			}
			_pEditView->getCurrentBuffer().setUnicodeMode(um);
			_pDocTab->updateCurrentTabItem();
			checkDocState();
			synchronise();

			_pEditView->execute(SCI_SETCODEPAGE, isUnicodeMode?SC_CP_UTF8:0);
			checkUnicodeMenuItems(um);
			setUniModeText(um);
			break;
		}

		case IDM_SETTING_TAB_REPLCESPACE:
		{
			NppGUI & nppgui = (NppGUI &)((NppParameters::getInstance())->getNppGUI());
			nppgui._tabReplacedBySpace = !nppgui._tabReplacedBySpace;
			_pEditView->execute(SCI_SETUSETABS, !nppgui._tabReplacedBySpace);
			checkMenuItem(IDM_SETTING_TAB_REPLCESPACE, nppgui._tabReplacedBySpace);
			break;
		}

		case IDM_SETTING_TAB_SIZE:
		{
			ValueDlg tabSizeDlg;
			NppGUI & nppgui = (NppGUI &)((NppParameters::getInstance())->getNppGUI());
			tabSizeDlg.init(_hInst, _hSelf, nppgui._tabSize, "Tab Size : ");
			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(_hParent, &p);
			int size = tabSizeDlg.doDialog(p);
			if (size != -1)
			{
				nppgui._tabSize = size;
				_pEditView->execute(SCI_SETTABWIDTH, nppgui._tabSize);
			}

			break;
		}

		case IDM_SETTING_HISTORY_SIZE :
		{
			ValueDlg nbHistoryDlg;
			NppParameters *pNppParam = NppParameters::getInstance();
			nbHistoryDlg.init(_hInst, _hSelf, pNppParam->getNbMaxFile(), "Max File : ");
			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(_hParent, &p);
			int size = nbHistoryDlg.doDialog(p);
			//changeDlgLang(DLG_NBHISTORY, nbHistoryDlg.getHSelf());

			if (size != -1)
			{
				if (size > NB_MAX_LRF_FILE)
					size = NB_MAX_LRF_FILE;
				pNppParam->setNbMaxFile(size);
			}
			break;
		}

		case IDM_SETTING_EDGE_SIZE :
		{
			ValueDlg nbColumnEdgeDlg;
			NppParameters *pNppParam = NppParameters::getInstance();
			nbColumnEdgeDlg.init(_hInst, _hSelf, pNppParam->getNbColumnEdge(), "Nb of column:");
			nbColumnEdgeDlg.setNBNumber(3);

			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(_hParent, &p);
			int size = nbColumnEdgeDlg.doDialog(p);
			//changeDlgLang(DLG_NBHISTORY, nbHistoryDlg.getHSelf());

			if (size != -1)
			{
				pNppParam->setNbColumnEdge(size);
				_mainEditView.execute(SCI_SETEDGECOLUMN, size);
				_subEditView.execute(SCI_SETEDGECOLUMN, size);
			}
			break;
		}

        case IDC_DOC_GOTO_ANOTHER_VIEW:
            docGotoAnotherEditView(MODE_TRANSFER);
            break;

        case IDC_DOC_CLONE_TO_ANOTHER_VIEW:
            docGotoAnotherEditView(MODE_CLONE);
            break;

        case IDM_ABOUT:
            _aboutDlg.doDialog();
			break;
// WebControl bug
            /*
        case IDM_TRY :
        {
            int textLen = _pEditView->getCurrentDocLen();
            char *htmlStr = new char[textLen];
            _pEditView->getText(htmlStr, textLen);
            _webBrowser.displayStr(htmlStr);
            _webBrowser.display();
            delete [] htmlStr;
            break;
        }
            */
		case IDC_AUTOCOMPLETE :
			showAutoComp();
			break;

        case IDM_LANGSTYLE_CONFIG_DLG :
		{
			bool isFirstTime = !_configStyleDlg.isCreated();
			_configStyleDlg.doDialog();
			if (isFirstTime)
				changeConfigLang();
			break;
		}

        case IDM_LANG_C	:
            setLanguage(id, L_C); 
            break;

        case IDM_LANG_CPP :
            setLanguage(id, L_CPP); 
            break;

        case IDM_LANG_JAVA :
            setLanguage(id, L_JAVA); 
            break;

        case IDM_LANG_CS :
            setLanguage(id, L_CS); 
            break;

        case IDM_LANG_HTML :
            setLanguage(id, L_HTML); 
            break;

        case IDM_LANG_XML :
            setLanguage(id, L_XML); 
            break;

        case IDM_LANG_JS :
            setLanguage(id, L_JS); 
            break;

        case IDM_LANG_PHP :
            setLanguage(id, L_PHP); 
            break;

        case IDM_LANG_ASP :
            setLanguage(id, L_ASP); 
            break;

        case IDM_LANG_CSS :
            setLanguage(id, L_CSS); 
            break;

        case IDM_LANG_LUA :
            setLanguage(id, L_LUA); 
            break;

        case IDM_LANG_PERL :
            setLanguage(id, L_PERL); 
            break;

        case IDM_LANG_PYTHON :
            setLanguage(id, L_PYTHON); 
            break;

        case IDM_LANG_PASCAL :
            setLanguage(id, L_PASCAL); 
            break;

        case IDM_LANG_BATCH :
            setLanguage(id, L_BATCH); 
            break;

        case IDM_LANG_OBJC :
            setLanguage(id, L_OBJC); 
            break;

        case IDM_LANG_VB :
            setLanguage(id, L_VB); 
            break;

        case IDM_LANG_SQL :
            setLanguage(id, L_SQL); 
            break;

        case IDM_LANG_ASCII :
            setLanguage(id, L_NFO); 
            break;

        case IDM_LANG_TEXT :
            setLanguage(id, L_TXT); 
            break;

        case IDM_LANG_RC :
            setLanguage(id, L_RC); 
            break;

        case IDM_LANG_MAKEFILE :
            setLanguage(id, L_MAKEFILE); 
            break;

        case IDM_LANG_INI :
            setLanguage(id, L_INI); 
            break;

        case IDM_LANG_TEX :
            setLanguage(id, L_TEX); 
            break;

        case IDM_LANG_FORTRAN :
            setLanguage(id, L_FORTRAN); 
            break;

        case IDM_LANG_SH :
            setLanguage(id, L_BASH); 
            break;

        case IDM_LANG_FLASH :
            setLanguage(id, L_FLASH); 
            break;

		case IDM_LANG_NSIS :
            setLanguage(id, L_NSIS); 
            break;

		case IDM_LANG_USER :
            setLanguage(id, L_USER); 
            break;

        case IDC_PREV_DOC :
            activateNextDoc(dirUp);
            break;

        case IDC_NEXT_DOC :
            activateNextDoc(dirDown);
            break;

		default :
			if (id > IDM_FILE_EXIT && id < (IDM_FILE_EXIT + _nbLRFile + 1))
			{
				char fn[256];
				::GetMenuString(::GetMenu(_hSelf), id, fn, sizeof(fn), MF_BYCOMMAND);
				if (!doOpen(fn))
                {
                    ::RemoveMenu(::GetMenu(_hSelf), id , MF_BYCOMMAND);
                }
				//else
				//{
					
				//}
			}
			else if ((id > IDM_LANG_USER) && (id < IDM_LANG_USER_LIMIT))
			{
				char langName[langNameLenMax];
				::GetMenuString(::GetMenu(_hSelf), id, langName, sizeof(langName), MF_BYCOMMAND);
				_pEditView->setCurrentDocUserType(langName);
				setLangStatus(L_USER);
				checkLangsMenu(id);
			}
	}
}

void Notepad_plus::setTitleWith(const char *filePath)
{
	if (!filePath || !strcmp(filePath, ""))
		return;

	char str2concat[MAX_PATH]; 
	strcat(strcpy(str2concat, _className), " - ");
	strcat(str2concat, filePath);
	::SetWindowText(_hSelf, str2concat);
}

void Notepad_plus::activateNextDoc(bool direction) 
{
    int nbDoc = _pEditView->getNbDoc();
    if (!nbDoc) return;

    int curIndex = _pEditView->getCurrentDocIndex();
    curIndex += (direction == dirUp)?-1:1;
    if (curIndex >= nbDoc)
        curIndex = 0;
    else if (curIndex < 0)
        curIndex = nbDoc - 1;

    setTitleWith(_pDocTab->activate(curIndex));
	checkDocState();
}


void Notepad_plus::dropFiles(HDROP hdrop) 
{
	if (hdrop)
	{
		// Determinate in which view the file(s) is (are) dropped
		POINT p;
		::DragQueryPoint(hdrop, &p);
		HWND hWin = ::ChildWindowFromPoint(_hSelf, p);
		if (hWin)
		{
			if ((_mainEditView.getHSelf() == hWin) || (_mainDocTab.getHSelf() == hWin))
				switchEditViewTo(MAIN_VIEW);
			else if ((_subEditView.getHSelf() == hWin) || (_subDocTab.getHSelf() == hWin))
				switchEditViewTo(SUB_VIEW);
		}

		int filesDropped = ::DragQueryFile(hdrop, 0xffffffff, NULL, 0);
		for (int i = 0 ; i < filesDropped ; ++i)
		{
			char pathDropped[MAX_PATH];
			::DragQueryFile(hdrop, i, pathDropped, sizeof(pathDropped));
			if (!doOpen(pathDropped))
			{
				break;
			}
            setLangStatus(_pEditView->getCurrentDocType());
		}
		::DragFinish(hdrop);
		// Put Notepad_plus to forefront
		// May not work for Win2k, but OK for lower versions
		// Note: how to drop a file to an iconic window?
		// Actually, it is the Send To command that generates a drop.
		if (::IsIconic(_hSelf))
		{
			::ShowWindow(_hSelf, SW_RESTORE);
		}
		::SetForegroundWindow(_hSelf);
	}
}

void Notepad_plus::checkModifiedDocument()
{
	const int NB_VIEW = 2;
	ScintillaEditView * pScintillaArray[NB_VIEW];
	DocTabView * pDocTabArray[NB_VIEW];

	// the oder (1.current view 2.non current view) is important
	// to synchronize with "hideCurrentView" function
	pScintillaArray[0] = _pEditView;
	pScintillaArray[1] = getNonCurrentEditView();

	pDocTabArray[0] = _pDocTab;
	pDocTabArray[1] = getNonCurrentDocTab();


	for (int j = 0 ; j < NB_VIEW ; j++)
	{
		for (int i = (pScintillaArray[j]->getNbDoc()-1) ; i >= 0  ; i--)
		{
			Buffer & docBuf = pScintillaArray[j]->getBufferAt(i);
			docFileStaus fStatus = docBuf.checkFileState();
			pDocTabArray[j]->updateTabItem(i);

			if (fStatus == MODIFIED_FROM_OUTSIDE)
			{
					// If npp is minimized, bring it up to the top
				if (::IsIconic(_hSelf))
					::ShowWindow(_hSelf, SW_SHOWNORMAL);

				if (doReloadOrNot(docBuf.getFileName()) == IDYES)
				{
					pDocTabArray[j]->activate(i);
					// if it's a non current view, make it as the current view
					if (j == 1)
						switchEditViewTo(getNonCurrentView());
					reload(docBuf.getFileName());
				}
				docBuf.updatTimeStamp();
			}
			else if (fStatus == FILE_DELETED)
			{
				if (::IsIconic(_hSelf))
					::ShowWindow(_hSelf, SW_SHOWNORMAL);

				if (doCloseOrNot(docBuf.getFileName()) == IDNO)
				{
					pDocTabArray[j]->activate(i);
					//_pDocTab->closeCurrentDoc();
					if ((pScintillaArray[j]->getNbDoc() == 1) && (_mainWindowStatus & TWO_VIEWS_MASK))
					{
						pDocTabArray[j]->closeCurrentDoc();
						hideCurrentView();
					}
					else
						pDocTabArray[j]->closeCurrentDoc();
				}
			}
	        
			bool isReadOnly = pScintillaArray[j]->isCurrentBufReadOnly();
			pScintillaArray[j]->execute(SCI_SETREADONLY, isReadOnly);
			//_pDocTab->updateCurrentTabItem();
		}
	}
}

void Notepad_plus::hideCurrentView()
{
	if (_mainWindowStatus & DOCK_MASK)
	{
		_pMainSplitter->setWin0(getNonCurrentDocTab());
		//_pMainWindow = _pMainSplitter;
	}
	else // otherwise the main window is the spltter container that we just created
		_pMainWindow = getNonCurrentDocTab();
	    
	_subSplitter.display(false);
	_pEditView->display(false);
	_pDocTab->display(false);

	// resize the main window
	RECT rc;
	getMainClientRect(rc);
	_pMainWindow->reSizeTo(rc);

	switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

	//setTitleWith(_pEditView->getCurrentTitle());

	_mainWindowStatus &= ~TWO_VIEWS_MASK;
}

bool Notepad_plus::fileClose()
{
	int res;
	bool isDirty = _pEditView->isCurrentDocDirty();
	if (isDirty)
	{
		if ((res = doSaveOrNot(_pEditView->getCurrentTitle())) == IDYES)
		{
			if (!fileSave()) // the cancel button of savdialog is pressed
				return false;
		}
		else if (res == IDCANCEL)
			return false;
		// else IDNO we continue
	}

	//process the fileNamePath into LRF
	const char *fileNamePath = _pEditView->getCurrentTitle();

	//si ce n'est pas untited(avec prefixe "new "), on fait le traitement
	if (!Buffer::isUntitled(fileNamePath))
	{
		//(NppParameters::getInstance())->writeHistory(fileNamePath);
		_lastRecentFileList.set(fileNamePath);
	}

	
	if ((_pEditView->getNbDoc() == 1) && (_mainWindowStatus & TWO_VIEWS_MASK))
	{
		_pDocTab->closeCurrentDoc();
		hideCurrentView();
		return true;
	}

	setTitleWith(_pDocTab->closeCurrentDoc());

	updateStatusBar();
	dynamicCheckMenuAndTB();
	return true;
}

bool Notepad_plus::fileCloseAll()
{
    if (_mainWindowStatus & TWO_VIEWS_MASK)
    {
        while (_pEditView->getNbDoc() > 1)
		    if (!fileClose())
			    return false;

	    if (!fileClose())
			return false;
    }

	while (_pEditView->getNbDoc() > 1)
		if (!fileClose())
			return false;
	return fileClose();
}

bool Notepad_plus::fileCloseAllButCurrent()
{
	int curIndex = _pEditView->getCurrentDocIndex();
	_pEditView->activateDocAt(0);

	for (int i = 0 ; i < curIndex ; i++)
		if (!fileClose())
			    return false;

	if (_pEditView->getNbDoc() > 1)
	{
		_pEditView->activateDocAt(1);
		while (_pEditView->getNbDoc() > 1)
			if (!fileClose())
			    return false;
	}
	if (_mainWindowStatus & TWO_VIEWS_MASK)
	{
		switchEditViewTo(getNonCurrentView());
		while (_pEditView->getNbDoc() > 1)
			if (!fileClose())
				return false;
		return fileClose();
	}
	return true;
}

void Notepad_plus::reload(const char *fileName)
{
	Utf8_16_Read UnicodeConvertor;

	FILE *fp = fopen(fileName, "rb");
	if (fp)
	{
		//setTitleWith(_pDocTab->newDoc(fileName));

		// It's VERY IMPORTANT to reset the view
		_pEditView->execute(SCI_CLEARALL);

		const int blockSize = 128 * 1024;
		char data[blockSize];

		int lenFile = int(fread(data, 1, sizeof(data), fp));
		while (lenFile > 0) {
			lenFile = UnicodeConvertor.convert(data, lenFile);
			//_pEditView->execute(SCI_ADDTEXT, lenFile, reinterpret_cast<LPARAM>(static_cast<char *>(data)));
			_pEditView->execute(SCI_ADDTEXT, lenFile, reinterpret_cast<LPARAM>(UnicodeConvertor.getNewBuf()));
			lenFile = int(fread(data, 1, sizeof(data), fp));
		}
		fclose(fp);

		UniMode unicodeMode = static_cast<UniMode>(UnicodeConvertor.getEncoding());
		(_pEditView->getCurrentBuffer()).setUnicodeMode(unicodeMode);

		if (unicodeMode != uni8Bit)
			// Override the code page if Unicode
			_pEditView->execute(SCI_SETCODEPAGE, SC_CP_UTF8);

		_pEditView->getFocus();
		_pEditView->execute(SCI_SETSAVEPOINT);
		_pEditView->execute(EM_EMPTYUNDOBUFFER);
//		return true;
	}
	else
	{
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		//strcat(msg, fullPath);
		strcat(msg, fileName);
		strcat(msg, "\".");
		::MessageBox(_hSelf, msg, "ERR", MB_OK);
//		return false;
	}
}

void Notepad_plus::getMainClientRect(RECT &rc) const
{
    Window::getClientRect(rc);
	rc.top += _toolBar.getHeight() + 2;
    rc.bottom -= _toolBar.getHeight() + 2 +_statusBar.getHeight();
}

void Notepad_plus::getToolBarClientRect(RECT &rc) const
{
    Window::getClientRect(rc);
    rc.bottom = _toolBar.getHeight();
}

void Notepad_plus::getStatusBarClientRect(RECT & rc) const
{
    RECT rectMain;
    
    getMainClientRect(rectMain);
    getClientRect(rc);
    rc.top = rectMain.top + rectMain.bottom;
    rc.bottom = rc.bottom - rc.top;
}

void Notepad_plus::dockUserDlg()
{
    if (!_pMainSplitter)
    {
        _pMainSplitter = new SplitterContainer;
        _pMainSplitter->init(_hInst, _hSelf);

        Window *pWindow;
        if (_mainWindowStatus & TWO_VIEWS_MASK)
            pWindow = &_subSplitter;
        else
            pWindow = _pDocTab;

        _pMainSplitter->create(pWindow, ScintillaEditView::getUserDefineDlg(), 8, RIGHT_FIX, 45); 
    }

    if (_mainWindowStatus & TWO_VIEWS_MASK)
        _pMainSplitter->setWin0(&_subSplitter);
    else 
        _pMainSplitter->setWin0(_pDocTab);

    RECT rc;
    
    getMainClientRect(rc);
    _pMainSplitter->reSizeTo(rc);
    _pMainSplitter->display();

    _mainWindowStatus |= DOCK_MASK;
    _pMainWindow = _pMainSplitter;
}

void Notepad_plus::undockUserDlg()
{
    // a cause de surchargement de "display"
    ::ShowWindow(_pMainSplitter->getHSelf(), SW_HIDE);

    if (_mainWindowStatus & TWO_VIEWS_MASK)
        _pMainWindow = &_subSplitter;
    else
        _pMainWindow = _pDocTab;
    
    RECT rc;
    getMainClientRect(rc);
    _pMainWindow->reSizeTo(rc);

    _mainWindowStatus &= ~DOCK_MASK;
    (ScintillaEditView::getUserDefineDlg())->display(); 
    //(_pEditView->getUserDefineDlg())->display();
}

void Notepad_plus::docGotoAnotherEditView(bool mode)
{
    if (!(_mainWindowStatus & TWO_VIEWS_MASK))
    {
        // if there's dock dialog, it means there's also a splitter container
        // we replace the right window by sub-spltter container that we just created
        if (_mainWindowStatus & DOCK_MASK)
        {
            _pMainSplitter->setWin0(&_subSplitter);
            _pMainWindow = _pMainSplitter;
        }
        else // otherwise the main window is the spltter container that we just created
            _pMainWindow = &_subSplitter;
        
        // resize the main window
        RECT rc;
		getMainClientRect(rc);
        _pMainWindow->reSizeTo(rc);

        getNonCurrentEditView()->display();
        getNonCurrentDocTab()->display();

        _pMainWindow->display();

        // update the main window status
        _mainWindowStatus |= TWO_VIEWS_MASK;
    }

    // Bon, define the source view and the dest view
    // source view
    DocTabView *pSrcDocTab;
    ScintillaEditView *pSrcEditView;
    if (getCurrentView() == MAIN_VIEW)
    {
        // make dest view
        switchEditViewTo(SUB_VIEW);

        // make source view
        pSrcDocTab = &_mainDocTab;
        pSrcEditView = &_mainEditView;

    }
    else
    {
        // make dest view : _pDocTab & _pEditView
        switchEditViewTo(MAIN_VIEW);

        // make source view
        pSrcDocTab = &_subDocTab;
        pSrcEditView = &_subEditView;
    }

    // Maintenant, we begin to manipulate the source and the dest:
    // 1. Save the current position of the source view to transfer
    pSrcEditView->saveCurrentPos();

    // 2. Retrieve the current buffer from the source
    Buffer & buf = pSrcEditView->getCurrentBuffer();

    // 3. See if the file to transfer exist in the dest view
    //    if so, we don't transfer the file(buffer) 
    //    but activate the opened document in the dest view then beat it
    int i;
    if ( (i = _pDocTab->find(buf.getFileName())) != -1)
	{
		setTitleWith(_pDocTab->activate(i));
		_pDocTab->getFocus();
		return;
	}

    // 4. Transfer the file (buffer) into the dest view
    bool isNewDoc2Close = false;

    if ((_pEditView->getNbDoc() == 1) 
		&& Buffer::isUntitled(_pEditView->getCurrentTitle())
        && (!_pEditView->isCurrentDocDirty()) && (_pEditView->getCurrentDocLen() == 0))
    {
        isNewDoc2Close = true;
    }

    setTitleWith(_pDocTab->newDoc(buf));
    _pDocTab->updateCurrentTabItem(NULL);
    
    if (isNewDoc2Close)
        _pDocTab->closeDocAt(0);

    // 5. If it's the clone mode, we keep the document to transfer
    //    in the source view (do nothing). If it's the transfer mode
    //    we remove the file (buffer) from the source view
    if (mode != MODE_CLONE)
    {
        // Make focus to the source view
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

        if (_pEditView->getNbDoc() == 1)
        {
            // close the current doc in the dest view
            _pDocTab->closeCurrentDoc();
			hideCurrentView();
        }
        else
        {
            // Make focus to the source view
            //switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

            // close the current doc in the dest view
            _pDocTab->closeCurrentDoc();

            // return to state where the focus is on dest view
            switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
        }
    }
}

void Notepad_plus::bookmarkNext(bool forwardScan) 
{
	int lineno = _pEditView->getCurrentLineNumber();
	int sci_marker = SCI_MARKERNEXT;
	int lineStart = lineno + 1;	//Scan starting from next line
	int lineRetry = 0;				//If not found, try from the beginning
	if (!forwardScan) 
    {
		lineStart = lineno - 1;		//Scan starting from previous line
		lineRetry = _pEditView->execute(SCI_GETLINECOUNT);	//If not found, try from the end
		sci_marker = SCI_MARKERPREVIOUS;
	}
	int nextLine = _pEditView->execute(sci_marker, lineStart, 1 << MARK_SYMBOLE);
	if (nextLine < 0)
		nextLine = _pEditView->execute(sci_marker, lineRetry, 1 << MARK_SYMBOLE);

    _pEditView->execute(SCI_ENSUREVISIBLEENFORCEPOLICY, nextLine);
	_pEditView->execute(SCI_GOTOLINE, nextLine);
}

void Notepad_plus::dynamicCheckMenuAndTB() const 
{
	// Visibility of 3 margins
    checkMenuItem(IDM_VIEW_LINENUMBER, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_LINENUMBER));
    checkMenuItem(IDM_VIEW_SYMBOLMARGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_SYBOLE));
    checkMenuItem(IDM_VIEW_FOLDERMAGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_FOLDER));
	// Folder margin style
	//changeCheckedItemFromTo(getFolderMarginStyle(), );
	checkFolderMarginStyleMenu(getFolderMaginStyleIDFrom(_pEditView->getFolderStyle()));

	// Visibility of invisible characters
	bool b = _pEditView->isInvisibleCharsShown();
	checkMenuItem(IDM_VIEW_ALL_CHARACTERS, b);
	_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, b);

	// Visibility of the indentation guide line 
	b = _pEditView->isShownIndentGuide();
	checkMenuItem(IDM_VIEW_INDENT_GUIDE, b);
	_toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, b);

	// Edge Line
	int mode = _pEditView->execute(SCI_GETEDGEMODE);
	checkMenuItem(IDM_VIEW_EDGEBACKGROUND, MF_BYCOMMAND | ((mode == EDGE_NONE)||(mode == EDGE_LINE))?MF_UNCHECKED:MF_CHECKED);
	checkMenuItem(IDM_VIEW_EDGELINE, MF_BYCOMMAND | ((mode == EDGE_NONE)||(mode == EDGE_BACKGROUND))?MF_UNCHECKED:MF_CHECKED);

	// Current Line Highlighting
	checkMenuItem(IDM_VIEW_CURLINE_HILITING, _pEditView->isCurrentLineHiLiting());

	// Wrap
	b = _pEditView->isWrap();
	checkMenuItem(IDM_VIEW_WRAP, b);
	_toolBar.setCheck(IDM_VIEW_WRAP, b);

	//Format conversion
	enableConvertMenuItems((_pEditView->getCurrentBuffer()).getFormat());
	checkUnicodeMenuItems((_pEditView->getCurrentBuffer()).getUnicodeMode());
}

void Notepad_plus::showAutoComp()
{
	int curPos = _pEditView->execute(SCI_GETCURRENTPOS);
	int line = _pEditView->getCurrentLineNumber();
	int debutLinePos = _pEditView->execute(SCI_POSITIONFROMLINE, line );
	int debutMotPos = curPos;


	char c = _pEditView->execute(SCI_GETCHARAT, debutMotPos-1);
	while ((debutMotPos>0)&&(debutMotPos>=debutLinePos)&&((isalnum(c))||(c=='_')))
	{
		debutMotPos--;
		c = _pEditView->execute(SCI_GETCHARAT, debutMotPos-1);
	}
	LangType langType = _pEditView->getCurrentDocType();
	if ((langType == L_RC) || (langType == L_HTML) || (langType == L_SQL))
	{
		int typeIndex = LANG_INDEX_INSTR;
		
		const char *pKeywords = (NppParameters::getInstance())->getWordList(langType, typeIndex);
		if (pKeywords)
		{
			_pEditView->execute(SCI_AUTOCSETSEPARATOR, WPARAM(' '));
			//_pEditView->execute(SCI_AUTOCSETTYPESEPARATOR, WPARAM('('));
			_pEditView->execute(SCI_AUTOCSETIGNORECASE, 3);
			_pEditView->execute(SCI_AUTOCSHOW, curPos-debutMotPos, WPARAM(pKeywords));
		}
	}
	else
	{
		char nppPath[MAX_PATH];
		strcpy(nppPath, _nppPath);
		PathRemoveFileSpec(nppPath);
		std::string fullFileName = nppPath;
		std::string fileName;
		getApiFileName(langType, fileName);
		fileName += ".api";
		fullFileName += "\\plugins\\APIs\\";
		fullFileName += fileName;

		FILE* f = fopen( fullFileName.c_str(), "r" );

		if (f)
		{
			fseek( f, 0, SEEK_END );
			size_t sz = ftell( f );
			fseek( f, 0, SEEK_SET );
			char* pData = new char[sz+1];
			size_t nbChar = fread(pData, 1, sz, f);
			pData[nbChar] = '\0';
			fclose( f );

			_pEditView->execute(SCI_AUTOCSETSEPARATOR, WPARAM('\n'));
			_pEditView->execute(SCI_AUTOCSETTYPESEPARATOR, WPARAM('('));
			_pEditView->execute(SCI_AUTOCSETIGNORECASE, 3);
			_pEditView->execute(SCI_AUTOCSHOW, curPos-debutMotPos, WPARAM(pData));
			delete[] pData;
		}
	}
}

void Notepad_plus::changeMenuLang()
{
	if (!_nativeLang) return;

	HMENU hMenu = ::GetMenu(_hSelf);
	TiXmlNode *mainMenu = _nativeLang->FirstChild("Menu");
	if (!mainMenu) return;

	mainMenu = mainMenu->FirstChild("Main");
	if (!mainMenu) return;

	TiXmlNode *entriesRoot = mainMenu->FirstChild("Entries");
	if (!entriesRoot) return;

	for (TiXmlNode *childNode = entriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		::ModifyMenu(hMenu, id, MF_BYPOSITION, 0, name);
	}

	TiXmlNode *menuCommandsRoot = mainMenu->FirstChild("Commands");

	for (TiXmlNode *childNode = menuCommandsRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		::ModifyMenu(hMenu, id, MF_BYCOMMAND, id, name);
	}

	TiXmlNode *subEntriesRoot = mainMenu->FirstChild("SubEntries");

	for (TiXmlNode *childNode = subEntriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int x, y;
		element->Attribute("posX", &x);
		element->Attribute("posY", &y);
		const char *name = element->Attribute("name");
		::ModifyMenu(::GetSubMenu(hMenu, x), y, MF_BYPOSITION, 0, name);
	}
	::DrawMenuBar(_hSelf);
}

const char * Notepad_plus::getNativeTip(int btnID)
{
	if (!_nativeLang) return NULL;

	TiXmlNode *tips = _nativeLang->FirstChild("Tips");
	if (!tips) return NULL;

	for (TiXmlNode *childNode = tips->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		element->Attribute("id", &id);
		if (id == btnID)
			return element->Attribute("name");
	}
	return NULL;
}
void Notepad_plus::changeConfigLang()
{
	if (!_nativeLang) return;

	TiXmlNode *styleConfDlgNode = _nativeLang->FirstChild("Dialog");
	if (!styleConfDlgNode) return;	
	
	styleConfDlgNode = styleConfDlgNode->FirstChild("StyleConfig");
	if (!styleConfDlgNode) return;

	HWND hDlg = _configStyleDlg.getHSelf();
	// Set Title
	const char *titre = (styleConfDlgNode->ToElement())->Attribute("title");
	if (titre && titre[0])
		::SetWindowText(hDlg, titre);

	for (TiXmlNode *childNode = styleConfDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
			::SetWindowText(::GetDlgItem(hDlg, id), name);
	}
	hDlg = _configStyleDlg.getHChildDlg();
	styleConfDlgNode = styleConfDlgNode->FirstChild("SubDialog");
	
	for (TiXmlNode *childNode = styleConfDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
			::SetWindowText(::GetDlgItem(hDlg, id), name);
	}
}

void Notepad_plus::changeStyleCtrlsLang(HWND hDlg, int *idArray, const char **translatedText)
{
	const int iColorStyle = 0;
	const int iForeground = 1;
	const int iBackground = 2;
	const int iFontStyle = 3;
	const int iFontName = 4;
	const int iFontSize = 5;
	const int iBold = 6;
	const int iItalic = 7;

	if (translatedText[iColorStyle] && translatedText[iColorStyle][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iColorStyle]), translatedText[iColorStyle]);

	if (translatedText[iForeground] && translatedText[iForeground][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iForeground]), translatedText[iForeground]);

	if (translatedText[iBackground] && translatedText[iBackground][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iBackground]), translatedText[iBackground]);

	if (translatedText[iFontStyle] && translatedText[iFontStyle][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iFontStyle]), translatedText[iFontStyle]);

	if (translatedText[iFontName] && translatedText[iFontName][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iFontName]), translatedText[iFontName]);

	if (translatedText[iFontSize] && translatedText[iFontSize][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iFontSize]), translatedText[iFontSize]);

	if (translatedText[iBold] && translatedText[iBold][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iBold]), translatedText[iBold]);

	if (translatedText[iItalic] && translatedText[iItalic][0])
		::SetWindowText(::GetDlgItem(hDlg, idArray[iItalic]), translatedText[iItalic]);
}

void Notepad_plus::changeUserDefineLang()
{
	if (!_nativeLang) return;

	TiXmlNode *userDefineDlgNode = _nativeLang->FirstChild("Dialog");
	if (!userDefineDlgNode) return;	
	
	userDefineDlgNode = userDefineDlgNode->FirstChild("UserDefine");
	if (!userDefineDlgNode) return;

	UserDefineDialog *userDefineDlg = _pEditView->getUserDefineDlg();

	HWND hDlg = userDefineDlg->getHSelf();
	// Set Title
	const char *titre = (userDefineDlgNode->ToElement())->Attribute("title");
	if (titre && titre[0])
		::SetWindowText(hDlg, titre);

	// pour ses propres controls 	
	const int nbControl = 8;
	const char *translatedText[nbControl];

	for (TiXmlNode *childNode = userDefineDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		
		if (sentinel && (name && name[0]))
		{
			if (id > 30)
				::SetWindowText(::GetDlgItem(hDlg, id), name);
			else
			{
				switch(id)
				{
					case 0: case 1: case 2: case 3: 
					case 4: case 5: case 6: case 7: 
						translatedText[id] = name; break;
				}
			}
		}
	}

	const int nbDlg = 4;
	HWND hDlgArrary[nbDlg];
	hDlgArrary[0] = userDefineDlg->getFolderHandle();
	hDlgArrary[1] = userDefineDlg->getKeywordsHandle();
	hDlgArrary[2] = userDefineDlg->getCommentHandle();
	hDlgArrary[3] = userDefineDlg->getSymbolHandle();
	
	const int nbGrpFolder = 3;
	int folderID[nbGrpFolder][nbControl] = {\
		{IDC_DEFAULT_COLORSTYLEGROUP_STATIC, IDC_DEFAULT_FG_STATIC, IDC_DEFAULT_BG_STATIC, IDC_DEFAULT_FONTSTYLEGROUP_STATIC, IDC_DEFAULT_FONTNAME_STATIC, IDC_DEFAULT_FONTSIZE_STATIC, IDC_DEFAULT_BOLD_CHECK, IDC_DEFAULT_ITALIC_CHECK},\
		{IDC_FOLDEROPEN_COLORSTYLEGROUP_STATIC, IDC_FOLDEROPEN_FG_STATIC, IDC_FOLDEROPEN_BG_STATIC, IDC_FOLDEROPEN_FONTSTYLEGROUP_STATIC, IDC_FOLDEROPEN_FONTNAME_STATIC, IDC_FOLDEROPEN_FONTSIZE_STATIC, IDC_FOLDEROPEN_BOLD_CHECK, IDC_FOLDEROPEN_ITALIC_CHECK},\
		{IDC_FOLDERCLOSE_COLORSTYLEGROUP_STATIC, IDC_FOLDERCLOSE_FG_STATIC, IDC_FOLDERCLOSE_BG_STATIC, IDC_FOLDERCLOSE_FONTSTYLEGROUP_STATIC, IDC_FOLDERCLOSE_FONTNAME_STATIC, IDC_FOLDERCLOSE_FONTSIZE_STATIC, IDC_FOLDERCLOSE_BOLD_CHECK, IDC_FOLDERCLOSE_ITALIC_CHECK}\
	};

	const int nbGrpKeywords = 4;
	int keywordsID[nbGrpKeywords][nbControl] = {\
		 {IDC_KEYWORD1_COLORSTYLEGROUP_STATIC, IDC_KEYWORD1_FG_STATIC, IDC_KEYWORD1_BG_STATIC, IDC_KEYWORD1_FONTSTYLEGROUP_STATIC, IDC_KEYWORD1_FONTNAME_STATIC, IDC_KEYWORD1_FONTSIZE_STATIC, IDC_KEYWORD1_BOLD_CHECK, IDC_KEYWORD1_ITALIC_CHECK},\
		{IDC_KEYWORD2_COLORSTYLEGROUP_STATIC, IDC_KEYWORD2_FG_STATIC, IDC_KEYWORD2_BG_STATIC, IDC_KEYWORD2_FONTSTYLEGROUP_STATIC, IDC_KEYWORD2_FONTNAME_STATIC, IDC_KEYWORD2_FONTSIZE_STATIC, IDC_KEYWORD2_BOLD_CHECK, IDC_KEYWORD2_ITALIC_CHECK},\
		{IDC_KEYWORD3_COLORSTYLEGROUP_STATIC, IDC_KEYWORD3_FG_STATIC, IDC_KEYWORD3_BG_STATIC, IDC_KEYWORD3_FONTSTYLEGROUP_STATIC, IDC_KEYWORD3_FONTNAME_STATIC, IDC_KEYWORD3_FONTSIZE_STATIC, IDC_KEYWORD3_BOLD_CHECK, IDC_KEYWORD3_ITALIC_CHECK},\
		{IDC_KEYWORD4_COLORSTYLEGROUP_STATIC, IDC_KEYWORD4_FG_STATIC, IDC_KEYWORD4_BG_STATIC, IDC_KEYWORD4_FONTSTYLEGROUP_STATIC, IDC_KEYWORD4_FONTNAME_STATIC, IDC_KEYWORD4_FONTSIZE_STATIC, IDC_KEYWORD4_BOLD_CHECK, IDC_KEYWORD4_ITALIC_CHECK}\
	};

	const int nbGrpComment = 3;
	int commentID[nbGrpComment][nbControl] = {\
		{IDC_COMMENT_COLORSTYLEGROUP_STATIC, IDC_COMMENT_FG_STATIC, IDC_COMMENT_BG_STATIC, IDC_COMMENT_FONTSTYLEGROUP_STATIC, IDC_COMMENT_FONTNAME_STATIC, IDC_COMMENT_FONTSIZE_STATIC, IDC_COMMENT_BOLD_CHECK, IDC_COMMENT_ITALIC_CHECK},\
		{IDC_NUMBER_COLORSTYLEGROUP_STATIC, IDC_NUMBER_FG_STATIC, IDC_NUMBER_BG_STATIC, IDC_NUMBER_FONTSTYLEGROUP_STATIC, IDC_NUMBER_FONTNAME_STATIC, IDC_NUMBER_FONTSIZE_STATIC, IDC_NUMBER_BOLD_CHECK, IDC_NUMBER_ITALIC_CHECK},\
		{IDC_COMMENTLINE_COLORSTYLEGROUP_STATIC, IDC_COMMENTLINE_FG_STATIC, IDC_COMMENTLINE_BG_STATIC, IDC_COMMENTLINE_FONTSTYLEGROUP_STATIC, IDC_COMMENTLINE_FONTNAME_STATIC, IDC_COMMENTLINE_FONTSIZE_STATIC, IDC_COMMENTLINE_BOLD_CHECK, IDC_COMMENTLINE_ITALIC_CHECK}\
	};

	const int nbGrpOperator = 1;
	int operatorID[nbGrpOperator][nbControl] = {\
		{IDC_SYMBOL_COLORSTYLEGROUP_STATIC, IDC_SYMBOL_FG_STATIC, IDC_SYMBOL_BG_STATIC, IDC_SYMBOL_FONTSTYLEGROUP_STATIC, IDC_SYMBOL_FONTNAME_STATIC, IDC_SYMBOL_FONTSIZE_STATIC, IDC_SYMBOL_BOLD_CHECK, IDC_SYMBOL_ITALIC_CHECK}\
	};
	
	int nbGpArray[nbDlg] = {nbGrpFolder, nbGrpKeywords, nbGrpComment, nbGrpOperator};
	const char nodeNameArray[nbDlg][16] = {"Folder", "Keywords", "Comment", "Operator"};

	//int **idArrays[nbDlg] = {(int **)folderID, (int **)keywordsID, (int **)commentID, (int **)operatorID};

	for (int i = 0 ; i < nbDlg ; i++)
	{
	
		for (int j = 0 ; j < nbGpArray[i] ; j++)
		{
			int **idArray;
			switch (i)
			{
				case 0 : changeStyleCtrlsLang(hDlgArrary[i], folderID[j], translatedText); break;
				case 1 : changeStyleCtrlsLang(hDlgArrary[i], keywordsID[j], translatedText); break;
				case 2 : changeStyleCtrlsLang(hDlgArrary[i], commentID[j], translatedText); break;
				case 3 : changeStyleCtrlsLang(hDlgArrary[i], operatorID[j], translatedText); break;
			}
			
//			changeStyleCtrlsLang(hDlgArrary[i], idArray[j], translatedText);
		}
		TiXmlNode *node = userDefineDlgNode->FirstChild(nodeNameArray[i]);
		
		if (node) 
		{
			// Set Title
			titre = (node->ToElement())->Attribute("title");
			if (titre &&titre[0])
				userDefineDlg->setTabName(i, titre);

			for (TiXmlNode *childNode = node->FirstChildElement("Item");
				childNode ;
				childNode = childNode->NextSibling("Item") )
			{
				TiXmlElement *element = childNode->ToElement();
				int id;
				const char *sentinel = element->Attribute("id", &id);
				const char *name = element->Attribute("name");
				if (sentinel && (name && name[0]))
					::SetWindowText(::GetDlgItem(hDlgArrary[i], id), name);
			}
		}
	}
}

void Notepad_plus::changeDlgLang(int dlgID)
{
	if (!_nativeLang) return;

	TiXmlNode *dlgNode = _nativeLang->FirstChild("Dialog");
	if (!dlgNode) return;
	
	HWND hDlg;
	char *dlgName;

	switch (dlgID)
	{
		case DLG_FIND :
			hDlg = _findReplaceDlg.getHSelf();
			dlgName = "Find";
			break;

		case DLG_GOTOLINE :
			hDlg = _goToLineDlg.getHSelf();
			dlgName = "GoToLine";
			break;

		case DLG_RUN :
			hDlg = _runDlg.getHSelf();
			dlgName = "Run";
			break;
/*
		case DLG_NBHISTORY :
			hDlg = hWnd;
			dlgName = "MaxFile";
			break;
*/

		default :
			return;
	}
	dlgNode = dlgNode->FirstChild(dlgName);
	if (!dlgNode) return;

	// Set Title
	const char *titre = (dlgNode->ToElement())->Attribute("title");
	if (titre && titre[0])
		::SetWindowText(hDlg, titre);

	// Set the text of child control
	for (TiXmlNode *childNode = dlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElement *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
			::SetWindowText(::GetDlgItem(hDlg, id), name);
	}
}

void Notepad_plus::changeToolBarIcons()
{
	if (!_toolIcons)
		return;
	for (int i = 0 ; i < _customIconVect.size() ; i++)
		_toolBar.changeIcons(_customIconVect[i].listIndex, _customIconVect[i].iconIndex, (_customIconVect[i].iconLocation).c_str());
}

ToolBarButtonUnit toolBarIcons[] = {
	{IDM_FILE_NEW,		IDI_NEW_OFF_ICON,		IDI_NEW_ON_ICON,		IDI_NEW_OFF_ICON, STD_FILENEW},
	{IDM_FILE_OPEN,		IDI_OPEN_OFF_ICON,		IDI_OPEN_ON_ICON,		IDI_NEW_OFF_ICON, STD_FILEOPEN},
	{IDM_FILE_SAVE,		IDI_SAVE_OFF_ICON,		IDI_SAVE_ON_ICON,		IDI_SAVE_DISABLE_ICON, STD_FILESAVE},
	{IDM_FILE_SAVEALL,	IDI_SAVEALL_OFF_ICON,	IDI_SAVEALL_ON_ICON,	IDI_SAVEALL_DISABLE_ICON, -1},
	{IDM_FILE_CLOSE,	IDI_CLOSE_OFF_ICON,		IDI_CLOSE_ON_ICON,		IDI_CLOSE_OFF_ICON, -1},
	{IDM_FILE_CLOSEALL,	IDI_CLOSEALL_OFF_ICON,	IDI_CLOSEALL_ON_ICON,	IDI_CLOSEALL_OFF_ICON, -1},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_EDIT_CUT,		IDI_CUT_OFF_ICON,		IDI_CUT_ON_ICON,		IDI_CUT_DISABLE_ICON, STD_CUT},
	{IDM_EDIT_COPY,		IDI_COPY_OFF_ICON,		IDI_COPY_ON_ICON,		IDI_COPY_DISABLE_ICON, STD_COPY},
	{IDM_EDIT_PASTE,	IDI_PASTE_OFF_ICON,		IDI_PASTE_ON_ICON,		IDI_PASTE_DISABLE_ICON, STD_PASTE},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_EDIT_UNDO,		IDI_UNDO_OFF_ICON,		IDI_UNDO_ON_ICON,		IDI_UNDO_DISABLE_ICON, STD_UNDO},
	{IDM_EDIT_REDO,		IDI_REDO_OFF_ICON,		IDI_REDO_ON_ICON,		IDI_REDO_DISABLE_ICON, STD_REDOW},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_SEARCH_FIND,		IDI_FIND_OFF_ICON,		IDI_FIND_ON_ICON,		IDI_FIND_OFF_ICON, -1},
	{IDM_SEARCH_REPLACE,  IDI_REPLACE_OFF_ICON,	IDI_REPLACE_ON_ICON,	IDI_REPLACE_OFF_ICON, -1},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_VIEW_ZOOMIN,	IDI_ZOOMIN_OFF_ICON,	IDI_ZOOMIN_ON_ICON,		IDI_ZOOMIN_OFF_ICON, -1},
	{IDM_VIEW_ZOOMOUT,	IDI_ZOOMOUT_OFF_ICON,	IDI_ZOOMOUT_ON_ICON,	IDI_ZOOMOUT_OFF_ICON, -1},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
 {IDM_VIEW_WRAP,  IDI_VIEW_WRAP_OFF_ICON,	IDI_VIEW_WRAP_ON_ICON,	IDI_VIEW_WRAP_OFF_ICON, -1},
 {IDM_VIEW_ALL_CHARACTERS,  IDI_VIEW_ALL_CHAR_OFF_ICON,	IDI_VIEW_ALL_CHAR_ON_ICON,	IDI_VIEW_ALL_CHAR_OFF_ICON, -1},
 {IDM_VIEW_INDENT_GUIDE,  IDI_VIEW_INDENT_OFF_ICON,	IDI_VIEW_INDENT_ON_ICON,	IDI_VIEW_INDENT_OFF_ICON, -1},
 {IDM_VIEW_USER_DLG,  IDI_VIEW_UD_DLG_OFF_ICON,	IDI_VIEW_UD_DLG_ON_ICON,	IDI_VIEW_UD_DLG_OFF_ICON, -1},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDC_BUTTON_PRINT,	IDI_PRINT_OFF_ICON,		IDI_PRINT_ON_ICON,		IDI_PRINT_OFF_ICON, STD_PRINT}
};    
					
int docTabIconIDs[] = {IDI_SAVED_ICON, IDI_UNSAVED_ICON, IDI_READONLY_ICON};

int stdIcons[] = {IDR_SAVEALL, IDR_CLOSEFILE, IDR_CLOSEALL, IDR_FIND, IDR_REPLACE, IDR_ZOOMIN, IDR_ZOOMOUT, IDR_WRAP, IDR_INVISIBLECHAR, IDR_INDENTGUIDE, IDR_SHOWPANNEL};

//static HWND hwndReBar;

LRESULT Notepad_plus::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_CREATE:
		{
			(NppParameters::getInstance())->setFontList(hwnd);
			const NppGUI & nppGUI = (NppParameters::getInstance())->getNppGUI();

			// Menu
			changeMenuLang();

            //-- Tool Bar Section --//
			toolBarStatusType tbStatus = nppGUI._toolBarStatus;

            // TB_LARGE par default
            int iconSize = 32;
            bool willBeShown = true;
            int menuID = IDM_VIEW_TOOLBAR_ENLARGE;

            if (tbStatus == TB_HIDE)
            {
                willBeShown = false;
                menuID = IDM_VIEW_TOOLBAR_HIDE;
            }
            else if (tbStatus == TB_SMALL)
            {
                iconSize = 16;
                menuID = IDM_VIEW_TOOLBAR_REDUCE;
            }
			else if (tbStatus == TB_STANDARD)
            {
                iconSize = 16;
                menuID = IDM_VIEW_TOOLBAR_STANDARD;
            }
			_toolBar.init(_hInst, hwnd, iconSize, toolBarIcons, sizeof(toolBarIcons)/sizeof(ToolBarButtonUnit), (tbStatus == TB_STANDARD), stdIcons, sizeof(stdIcons)/sizeof(int));
            _toolBar.display(willBeShown);
            checkToolBarMenu(menuID);

			
			changeToolBarIcons();

            _pDocTab = &_mainDocTab;
            _pEditView = &_mainEditView;

            const ScintillaViewParams & svp1 = (NppParameters::getInstance())->getSVP(SCIV_PRIMARY);
			const ScintillaViewParams & svp2 = (NppParameters::getInstance())->getSVP(SCIV_SECOND);

			_mainEditView.init(_hInst, hwnd);
            _subEditView.init(_hInst, hwnd);
			_mainEditView.display();


			// Configuration of 2 scintilla views
            _mainEditView.showMargin(ScintillaEditView::_SC_MARGE_LINENUMBER, svp1._lineNumberMarginShow);
			_subEditView.showMargin(ScintillaEditView::_SC_MARGE_LINENUMBER, svp2._lineNumberMarginShow);
            _mainEditView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, svp1._bookMarkMarginShow);
			_subEditView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, svp2._bookMarkMarginShow);

            _mainEditView.showIndentGuideLine(svp1._indentGuideLineShow);
            _subEditView.showIndentGuideLine(svp2._indentGuideLineShow);
			
			_configStyleDlg.init(_hInst, _hSelf);
            //Marker Margin config
            _mainEditView.setMakerStyle(svp1._folderStyle);
            _subEditView.setMakerStyle(svp2._folderStyle);

			_mainEditView.execute(SCI_SETCARETLINEVISIBLE, svp1._currentLineHilitingShow);
			_subEditView.execute(SCI_SETCARETLINEVISIBLE, svp2._currentLineHilitingShow);

			_mainEditView.wrap(svp1._doWrap);
			_subEditView.wrap(svp2._doWrap);
            //_toolBar.setCheck(IDM_VIEW_WRAP, _pEditView->isWrap());
			//checkMenuItem(IDM_VIEW_WRAP, _pEditView->isWrap());
			int nbCol = (NppParameters::getInstance())->getNbColumnEdge();
			_mainEditView.execute(SCI_SETEDGECOLUMN, nbCol);
			_mainEditView.execute(SCI_SETEDGEMODE, svp1._edgeMode);
			_subEditView.execute(SCI_SETEDGECOLUMN, nbCol);
			_subEditView.execute(SCI_SETEDGEMODE, svp2._edgeMode);

			_mainEditView.performGlobalStyles();
			_subEditView.performGlobalStyles();


			int tabBarStatus = nppGUI._tabStatus;
			_toReduceTabBar = bool(tabBarStatus & TAB_REDUCE);
			_docTabIconList.create(_toReduceTabBar?13:20, _hInst, docTabIconIDs, sizeof(docTabIconIDs)/sizeof(int));

            const char * str = _mainDocTab.init(_hInst, hwnd, &_mainEditView, &_docTabIconList);
			setTitleWith(str);
            _subDocTab.init(_hInst, hwnd, &_subEditView, &_docTabIconList);
			TabBar::doDragNDrop(true);
			
			if (_toReduceTabBar)
			{
				HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT/*SYSTEM_FONT*/);

				if (hf)
				{
					::SendMessage(_mainDocTab.getHSelf(), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
					::SendMessage(_subDocTab.getHSelf(), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
				}
				TabCtrl_SetItemSize(_mainDocTab.getHSelf(), 45, 20);
				TabCtrl_SetItemSize(_subDocTab.getHSelf(), 45, 20);
			}
			_mainDocTab.display();

			
			TabBar::doDragNDrop((tabBarStatus & TAB_DRAGNDROP) != 0);
			TabBar::setDrawTopBar((tabBarStatus & TAB_DRAWTOPBAR) != 0);
			TabBar::setDrawInactiveTab((tabBarStatus & TAB_DRAWINACTIVETAB) != 0);

			checkMenuItem(IDM_VIEW_REDUCETABBAR, _toReduceTabBar);
			checkMenuItem(IDM_VIEW_LOCKTABBAR, !TabBar::doDragNDropOrNot());
			checkMenuItem(IDM_VIEW_DRAWTABBAR_INACIVETAB, TabBar::drawInactiveTab());
			checkMenuItem(IDM_VIEW_DRAWTABBAR_TOPBAR, TabBar::drawTopBar());

            //--Splitter Section--//
			bool isVertical = (nppGUI._splitterPos == POS_VERTICAL);

            _subSplitter.init(_hInst, _hSelf);
            _subSplitter.create(&_mainDocTab, &_subDocTab, 8, DYNAMIC, 50, isVertical);

            //--Status Bar Section--//
			willBeShown = nppGUI._statusBarShow;
            _statusBar.init(_hInst, hwnd, 6);
			_statusBar.setPartWidth(STATUSBAR_DOC_SIZE, 80);
			_statusBar.setPartWidth(STATUSBAR_CUR_POS, 160);
			_statusBar.setPartWidth(STATUSBAR_EOF_FORMAT, 80);
			_statusBar.setPartWidth(STATUSBAR_UNICODE_TYPE, 100);
			_statusBar.setPartWidth(STATUSBAR_TYPING_MODE, 30);
            _statusBar.display(willBeShown);
			//
            checkMenuItem(IDM_VIEW_STATUSBAR, willBeShown);
			
            _findReplaceDlg.init(_hInst, hwnd, &_pEditView);
            _goToLineDlg.init(_hInst, hwnd, &_pEditView);
            _aboutDlg.init(_hInst, hwnd);
			_runDlg.init(_hInst, hwnd);
            
			checkMenuItem(IDM_SETTING_TAB_REPLCESPACE, nppGUI._tabReplacedBySpace);

            _pMainWindow = &_mainDocTab;

			//--Web Browser Section--//
            // WebControl bug
			//_webBrowser.init(_hInst, hwnd);

            //--User Define Dialog Section--//
			int uddStatus = nppGUI._userDefineDlgStatus;
		    UserDefineDialog *udd = _pEditView->getUserDefineDlg();
		    udd->create(IDD_GLOBAL_USERDEFINE_DLG);
			changeUserDefineLang();

			bool uddShow = false;
          switch (uddStatus)
            {
                case UDD_SHOW :                 // show & undocked
		            udd->display(true);
					uddShow = true;
                    break;
                case UDD_DOCKED : {              // hide & docked
                    udd->changeStyle();
                    udd->display(false);
                    break;}
                case (UDD_SHOW | UDD_DOCKED) :    // show & docked
		            udd->doDialog();
		            ::SendMessage(udd->getHSelf(), WM_COMMAND, IDC_DOCK_BUTTON, 0);
					uddShow = true;
                    break;

				default :                        // hide & undocked
                    udd->display(false);
            }
            		// UserDefine Dialog
			
			checkMenuItem(IDM_VIEW_USER_DLG, uddShow);
			_toolBar.setCheck(IDM_VIEW_USER_DLG, uddShow);

			dynamicCheckMenuAndTB();
			_mainEditView.defineDocType(L_TXT);
			HMENU hMenu = ::GetSubMenu(::GetMenu(_hSelf), 0);

			_nbLRFile = (NppParameters::getInstance())->getNbLRFile();
			int pos = 11;//GetMenuItemCount(hMenu) - 1 + 3/*separateurs*/;
            int k = 0;
			for (int i = 0, j = 0 ; i < _nbLRFile ; i++)
			{
				std::string * stdStr = (NppParameters::getInstance())->getLRFile(i);
                if (PathFileExists(stdStr->c_str()))
                {
				    _lastRecentFileList.add(stdStr->c_str());
				    ::InsertMenu(hMenu, pos + j , MF_BYPOSITION, IDM_FILE_EXIT + j + 1, stdStr->c_str());
                    j++;
                }
                else
                {
                    k++;
                }
			}
			if (_nbLRFile)
				::InsertMenu(hMenu, pos + _nbLRFile - k, MF_BYPOSITION, -1, 0);
			
			//updateStatusBar();
            _hExplorer = ::LoadLibrary(EXPLORER_PLUGIN_PATH);
            if (_hExplorer)
            {
                _doExplorer = (HWND (__cdecl *)(HINSTANCE, HWND))::GetProcAddress(_hExplorer, "doExplorer");
                if (_doExplorer)
                {
                    HWND hBrowser = _doExplorer(_hInst, _hSelf);
                    ::ShowWindow(hBrowser, SW_SHOW);
                }
            }
			// Add User Define Languages Entry
			hMenu = ::GetSubMenu(::GetMenu(_hSelf), 6);
			pos = GetMenuItemCount(hMenu) - 2;

			for (int i = 0 ; i < NppParameters::getInstance()->getNbUserLang() ; i++)
			{
				UserLangContainer & userLangContainer = NppParameters::getInstance()->getULCFromIndex(i);
				::InsertMenu(hMenu, pos + i , MF_BYPOSITION, IDM_LANG_USER + i + 1, userLangContainer.getName());
			}

			::SetWindowPos(_hSelf, HWND_TOP, nppGUI._appPos.left, nppGUI._appPos.top, nppGUI._appPos.right, nppGUI._appPos.bottom, SWP_SHOWWINDOW);
			//::MessageBox(NULL, "OK", "debug", MB_OK);

/*
	   hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,
	  REBARCLASSNAME,NULL,WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | RBS_VARHEIGHT | \
                  RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                             0,0,0,0,_hSelf,(HMENU)0,_hInst,NULL);
  REBARINFO rbi;
  REBARBANDINFO rbBand;

  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);

  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE ;
  rbBand.fStyle  =  RBBS_FIXEDSIZE ;
  //if (bIsAppThemed)
    rbBand.fStyle |= RBBS_CHILDEDGE;
  rbBand.hbmBack = NULL;
  rbBand.lpText     = "Toolbar";
  rbBand.hwndChild  = _toolBar.getHSelf();
  //rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(tbbMainWnd);
  //rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);
SetWindowPos(hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
*/

			return TRUE;
		}
		
		case WM_DRAWITEM :
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_TAB)
			{
				return ::SendMessage(dis->hwndItem, WM_DRAWITEM, wParam, lParam);
			}
		}

		case WM_DOCK_USERDEFINE_DLG:
		{
			dockUserDlg();
			return TRUE;
		}

        case WM_UNDOCK_USERDEFINE_DLG:
		{
            undockUserDlg();
			return TRUE;
		}

		case WM_REMOVE_USERLANG:
		{
            char name[256];
			strcpy(name, (char *)lParam);
			_mainEditView.removeUserLang(name);
			_subEditView.removeUserLang(name);
			return TRUE;
		}

        case WM_RENAME_USERLANG:
		{
            char oldName[256];
			char newName[256];
			strcpy(oldName, (char *)lParam);
			strcpy(newName, (char *)wParam);
			_mainEditView.renameUserLang(oldName, newName);
			_subEditView.renameUserLang(oldName, newName);
			return TRUE;
		}

		case WM_CLOSE_USERDEFINE_DLG :
		{
			checkMenuItem(IDM_VIEW_USER_DLG, false);
			_toolBar.setCheck(IDM_VIEW_USER_DLG, false);
			return TRUE;
		}

		case WM_SIZE:
		{
			RECT rc;
            getToolBarClientRect(rc);
            _toolBar.reSizeTo(rc);
			//SetWindowPos(hwndReBar,NULL,0,0,rc.right,rc.bottom,SWP_NOZORDER);

            getStatusBarClientRect(rc);
            _statusBar.reSizeTo(rc);			
			
			getMainClientRect(rc);
            _pMainWindow->reSizeTo(rc);

			return TRUE;
		}

		case WM_MOVE:
		{
			redraw();
			return TRUE;
		}

        case WM_COPYDATA :
        {
            COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT *)lParam;
			LangType lt = LangType(pCopyData->dwData);
            FileNameStringSplitter fnss((const char *)pCopyData->lpData);
            char *pFn = NULL;
            for (int i = 0 ; i < fnss.size() ; i++)
            {
                pFn = (char *)fnss.getFileName(i);
                doOpen((const char *)pFn);
				if (lt != L_TXT)
				{
					_pEditView->setCurrentDocType(lt);
					setLangStatus(lt);
				}
            }
            return TRUE;
        }

		case WM_COMMAND:
            if (HIWORD(wParam) == SCEN_SETFOCUS)
            {
                switchEditViewTo((lParam == (LPARAM)_mainEditView.getHSelf())?MAIN_VIEW:SUB_VIEW);
            }
            else
                command(LOWORD(wParam));

			return TRUE;

		case WM_NOTIFY:
		{
			checkClipboard();
			checkUndoState();
			notify(reinterpret_cast<SCNotification *>(lParam));
			return FALSE;
		}

		case WM_ACTIVATEAPP :
		{
			if (LOWORD(wParam))
			{
				checkModifiedDocument();
				return FALSE;
			}
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
		}


		case WM_ACTIVATE :
			_pEditView->getFocus();
			return TRUE;

		case WM_DROPFILES:
		{
			dropFiles(reinterpret_cast<HDROP>(wParam));
			return TRUE;
		}

		case WM_UPDATESCINTILLAS:
		{
			_mainEditView.defineDocType(_mainEditView.getCurrentDocType());
			_subEditView.defineDocType(_subEditView.getCurrentDocType());
			_mainEditView.performGlobalStyles();
			_subEditView.performGlobalStyles();
			return TRUE;
		}

		case WM_CLOSE:
		{
			if (fileCloseAll())
			{
				_lastRecentFileList.saveLRFL();
				saveScintillaParams(SCIV_PRIMARY);
				saveScintillaParams(SCIV_SECOND);
				saveGUIParams();
				saveUserDefineLangs();
				::DestroyWindow(hwnd);
			}
			return TRUE;
		}

		case WM_DESTROY:
		{	
			killAllChildren();	
			if (_hExplorer)
			{
				::FreeLibrary(_hExplorer);
			}
		
			::PostQuitMessage(0);
			return TRUE;
		}

		default:
		{
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
		}
	}
	return FALSE;
}

LRESULT CALLBACK Notepad_plus::Notepad_plus_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch(Message)
  {
    case WM_NCCREATE : // First message we get the ptr of instantiated object
                       // then stock it into GWL_USERDATA index in order to retrieve afterward
	{
		Notepad_plus *pM30ide = (Notepad_plus *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		pM30ide->_hSelf = hwnd;
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)pM30ide);

		return TRUE;
	}

    default :
    {
      return ((Notepad_plus *)::GetWindowLong(hwnd, GWL_USERDATA))->runProc(hwnd, Message, wParam, lParam);
    }
  }
}
