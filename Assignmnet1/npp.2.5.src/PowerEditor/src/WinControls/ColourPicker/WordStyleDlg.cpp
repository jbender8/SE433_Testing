/*
this file is part of notepad++
Copyright (C)2003 Don HO < donho@altern.org >

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "WordStyleDlg.h"
#include "ScintillaEditView.h"
#include "SysMsg.h"

//const char fontSizeStrs[][3] = {"", "8", "9", "10", "11", "12", "14", "16", "18", "20", "22", "24", "26", "28"};

static WNDPROC oldPrc = NULL;
static HFONT _hFont = NULL;
BOOL CALLBACK colourStaticProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_PAINT:
        {

	        RECT		rect;
            ::GetClientRect(hwnd, &rect);

            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
    		
            ::SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
            ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
    		// Create a font 
		    if(_hFont == 0)
		    {
			    // Get the default GUI font
			    LOGFONT lf;
                HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

			    // Add UNDERLINE attribute
				::GetObject(hf, sizeof lf, &lf);
                //lf.lfUnderline = TRUE;
    			
			    // Create a new font
                _hFont = ::CreateFontIndirect(&lf);
		    }

			HANDLE hOld = SelectObject(hdc, _hFont);

		    // Draw the text!
            char text[_MAX_PATH];
            ::GetWindowText(hwnd, text, sizeof(text));
            ::DrawText(hdc, text, -1, &rect, DT_CENTER);
    		
            ::SelectObject(hdc, hOld);

            ::EndPaint(hwnd, &ps);

		    return 0;
        }
    }
    return ::CallWindowProc(oldPrc, hwnd, Message, wParam, lParam);
}

BOOL CALLBACK WordStyleDlg::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_INITDIALOG :
		{
            _hStyleCombo = ::GetDlgItem(_hSelf, IDC_STYLETYPE_COMBO);
            _hCheckBold = ::GetDlgItem(_hSelf, IDC_BOLD_CHECK);
            _hCheckItalic = ::GetDlgItem(_hSelf, IDC_ITALIC_CHECK);
			_hCheckUnderline = ::GetDlgItem(_hSelf, IDC_UNDERLINE_CHECK);
			_hFontNameCombo = ::GetDlgItem(_hSelf, IDC_FONT_COMBO);
			_hFontSizeCombo = ::GetDlgItem(_hSelf, IDC_FONTSIZE_COMBO);

			_hFgColourStaticText = ::GetDlgItem(_hSelf, IDC_FG_STATIC);
			_hBgColourStaticText = ::GetDlgItem(_hSelf, IDC_BG_STATIC);
			_hFontNameStaticText = ::GetDlgItem(_hSelf, IDC_FONTNAME_STATIC);
			_hFontSizeStaticText = ::GetDlgItem(_hSelf, IDC_FONTSIZE_STATIC);
            oldPrc = (WNDPROC)::SetWindowLong(::GetDlgItem(_hSelf, IDC_STYLEDEFAULT_WARNING_STATIC), GWL_WNDPROC, (LONG)colourStaticProc);

			for(int i = 0 ; i < sizeof(fontSizeStrs)/3 ; i++)
				::SendMessage(_hFontSizeCombo, CB_ADDSTRING, 0, (LPARAM)fontSizeStrs[i]);

			const std::vector<std::string> & fontlist = (NppParameters::getInstance())->getFontList();
			for (int i = 0 ; i < fontlist.size() ; i++)
			{
				int j = ::SendMessage(_hFontNameCombo, CB_ADDSTRING, 0, (LPARAM)fontlist[i].c_str());
				::SendMessage(_hFontNameCombo, CB_SETITEMDATA, j, (LPARAM)fontlist[i].c_str());
			}

			_pFgColour = new ColourPicker;
			_pBgColour = new ColourPicker;
			_pFgColour->init(_hInst, _hSelf);
			_pBgColour->init(_hInst, _hSelf);

            POINT p1, p2;

            alignWith(_hFgColourStaticText, _pFgColour->getHSelf(), ALIGNPOS_RIGHT, p1);
            alignWith(_hBgColourStaticText, _pBgColour->getHSelf(), ALIGNPOS_RIGHT, p2);

            p1.x = p2.x = ((p1.x > p2.x)?p1.x:p2.x) + 10;
            p1.y -= 4; p2.y -= 4;

            ::MoveWindow((HWND)_pFgColour->getHSelf(), p1.x, p1.y, 25, 25, TRUE);
            ::MoveWindow((HWND)_pBgColour->getHSelf(), p2.x, p2.y, 25, 25, TRUE);
			_pFgColour->display();
			_pBgColour->display();
			return TRUE;
		}

		case WM_DESTROY:
		{
			_pFgColour->destroy();
			_pBgColour->destroy();
			delete _pFgColour;
			delete _pBgColour;
			return TRUE;
		}
		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
            {
				int editID = LOWORD(wParam);
				if (editID == IDC_USER_EXT_EDIT)
				{
					updateExtension();
					notifyParentDataModified();
				}
				else if (editID == IDC_USER_KEYWORDS_EDIT)
				{
					updateUserKeywords();
					notifyParentDataModified();
				}
			}
			else
			{
				switch (wParam)
				{
					case IDC_BOLD_CHECK :
						updateFontStyleStatus(BOLD_STATUS);
						notifyParentDataModified();
						break;

					case IDC_ITALIC_CHECK :
						updateFontStyleStatus(ITALIC_STATUS);
						notifyParentDataModified();
						break;

					case IDC_UNDERLINE_CHECK :
						updateFontStyleStatus(UNDERLINE_STATUS);
						notifyParentDataModified();
						break;
					default:
						switch (HIWORD(wParam))
						{
							case CBN_SELCHANGE :
							{
								switch (LOWORD(wParam))
								{
									case IDC_STYLETYPE_COMBO :
										setVisualFromStyleCombo();
										break;
									case IDC_FONT_COMBO :
										updateFontName();
										notifyParentDataModified();
										break;
									case IDC_FONTSIZE_COMBO :
										updateFontSize();
										notifyParentDataModified();
										break;
								}
								return TRUE;
							}

							case CPN_COLOURPICKED:	
							{
								if ((HWND)lParam == _pFgColour->getHSelf())
								{
									updateColour(C_FOREGROUND);
									notifyParentDataModified();
									return TRUE;
								}
								else if ((HWND)lParam == _pBgColour->getHSelf())
								{
									updateColour(C_BACKGROUND);
									notifyParentDataModified();
									return TRUE;
								}
								else
									return FALSE;
							}

							default :
							{
								return FALSE;
							}
						}
						return TRUE;
				}
			}

		}
		default :
			return FALSE;
	}
	return FALSE;
}
void WordStyleDlg::updateColour(bool which)
{
	Style & style = getCurrentStyler();
	if (which == C_FOREGROUND)
	{
		style._fgColor = _pFgColour->getColour();
	}
	else //(which == C_BACKGROUND)
	{
		style._bgColor = _pBgColour->getColour();
	}
}

void WordStyleDlg::updateFontSize()
{
	Style & style = getCurrentStyler();
	int iFontSizeSel = ::SendMessage(_hFontSizeCombo, CB_GETCURSEL, 0, 0);

	char intStr[5];
	if (iFontSizeSel != 0)
	{
		::SendMessage(_hFontSizeCombo, CB_GETLBTEXT, iFontSizeSel, (LPARAM)intStr);
		if ((!intStr) || (!intStr[0])) 
			style._fontSize = -1;
		else
		{
			char *finStr;
			style._fontSize = strtol(intStr, &finStr, 10);
			if (*finStr != '\0')
				style._fontSize = -1;
		}
	}
	else
		style._fontSize = 0;
}

void WordStyleDlg::updateExtension()
{
	const int NB_MAX = 256;
	char ext[NB_MAX];
	::SendDlgItemMessage(_hSelf, IDC_USER_EXT_EDIT, WM_GETTEXT, NB_MAX, (LPARAM)ext);
	_pLexerStylerArray->getLexerFromIndex(_currentLexerIndex - 1).setLexerUserExt(ext);
}

void WordStyleDlg::updateUserKeywords()
{
	Style & style = getCurrentStyler();
	const int NB_MAX = 2048;
	char kw[NB_MAX];
	::SendDlgItemMessage(_hSelf, IDC_USER_KEYWORDS_EDIT, WM_GETTEXT, NB_MAX, (LPARAM)kw);
	style.setKeywords(kw);
}

void WordStyleDlg::updateFontName()
{
    Style & style = getCurrentStyler();
	int iFontSel = ::SendMessage(_hFontNameCombo, CB_GETCURSEL, 0, 0);
	char *fnStr = (char *)::SendMessage(_hFontNameCombo, CB_GETITEMDATA, iFontSel, 0);
	style._fontName = fnStr;
}

void WordStyleDlg::updateFontStyleStatus(fontStyleType whitchStyle)
{
    Style & style = getCurrentStyler();
	if (style._fontStyle == -1)
		style._fontStyle = 0;

	int fontStyle = FONTSTYLE_UNDERLINE;
	HWND hWnd = _hCheckUnderline;

	if (whitchStyle == BOLD_STATUS)
	{
		fontStyle = FONTSTYLE_BOLD;
		hWnd = _hCheckBold;
	}
	if (whitchStyle == ITALIC_STATUS)
	{
		fontStyle = FONTSTYLE_ITALIC;
		hWnd = _hCheckItalic;
	}

	int isChecked = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
	if (isChecked != BST_INDETERMINATE)
	{
		if (isChecked == BST_CHECKED)
			style._fontStyle |= fontStyle;
		else
			style._fontStyle &= ~fontStyle;
	}
}

void WordStyleDlg::setStyleComboFromLexer(int index)
{
    _currentLexerIndex = index;

    // Fill out Styles Combobox
    // Before filling out, we clean it
    ::SendMessage(_hStyleCombo, CB_RESETCONTENT, 0, 0);

	if (index)
	{
		const char *langName = _pLexerStylerArray->getLexerNameFromIndex(index - 1);
		const char *ext = NppParameters::getInstance()->getLangExtFromName(langName);
		const char *userExt = (_pLexerStylerArray->getLexerStylerByName(langName))->getLexerUserExt();
		::SendDlgItemMessage(_hSelf, IDC_DEF_EXT_EDIT, WM_SETTEXT, 0, (LPARAM)(ext));
		::SendDlgItemMessage(_hSelf, IDC_USER_EXT_EDIT, WM_SETTEXT, 0, (LPARAM)(userExt));
	}
	::ShowWindow(::GetDlgItem(_hSelf, IDC_DEF_EXT_EDIT), index?SW_SHOW:SW_HIDE);
    ::ShowWindow(::GetDlgItem(_hSelf, IDC_DEF_EXT_STATIC), index?SW_SHOW:SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_USER_EXT_EDIT), index?SW_SHOW:SW_HIDE);
    ::ShowWindow(::GetDlgItem(_hSelf, IDC_USER_EXT_STATIC), index?SW_SHOW:SW_HIDE);

	StyleArray & lexerStyler = index?_pLexerStylerArray->getLexerFromIndex(index-1):*_pStylerArray;

    for (int i = 0 ; i < lexerStyler.getNbStyler() ; i++)
    {
        Style & style = lexerStyler.getStyler(i);
        ::SendMessage(_hStyleCombo, CB_ADDSTRING, 0, (LPARAM)style._styleDesc);	
    }
    ::SendMessage(_hStyleCombo, CB_SETCURSEL, 0, 0);
    setVisualFromStyleCombo();
}

void WordStyleDlg::setVisualFromStyleCombo() 
{

    Style & style = getCurrentStyler();

    //--Warning text
    int showWarning = ((_currentLexerIndex == 0) && (style._styleID == STYLE_DEFAULT))?SW_SHOW:SW_HIDE;
    ::ShowWindow(::GetDlgItem(_hSelf, IDC_STYLEDEFAULT_WARNING_STATIC), showWarning);

	//-- 2 couleurs : fg et bg
	bool isEnable = false;
	if (HIBYTE(HIWORD(style._fgColor)) != 0xFF)
	{
		_pFgColour->setColour(style._fgColor);
		isEnable = true;
	}
	enableFg(isEnable);

	isEnable = false;
	if (HIBYTE(HIWORD(style._bgColor)) != 0xFF)
	{
		_pBgColour->setColour(style._bgColor);
		isEnable = true;
	}
	enableBg(isEnable);

	//-- font name
	isEnable = false;
	int iFontName;
	if (style._fontName != NULL)
	{
		iFontName = ::SendMessage(_hFontNameCombo, CB_FINDSTRING, 1, (LPARAM)style._fontName);
		if (iFontName == CB_ERR)
			iFontName = 0;
		isEnable = true;
	}
	else
	{
		iFontName = 0;
	}
	::SendMessage(_hFontNameCombo, CB_SETCURSEL, iFontName, 0);
	enableFontName(isEnable);

	//-- font size
	isEnable = false;
	char intStr[5] = "";
	int iFontSize = 0;
	if (style._fontSize != -1)
	{
		sprintf(intStr, "%d", style._fontSize);
		iFontSize = ::SendMessage(_hFontSizeCombo, CB_FINDSTRING, 1, (LPARAM)intStr);
		isEnable = true;
	}
	::SendMessage(_hFontSizeCombo, CB_SETCURSEL, iFontSize, 0);
	enableFontSize(isEnable);

	//-- font style : bold et italic
	isEnable = false;
	int isBold, isItalic, isUnderline;
	if (style._fontStyle != -1)
	{
		isBold = (style._fontStyle & FONTSTYLE_BOLD)?BST_CHECKED:BST_UNCHECKED;
		isItalic = (style._fontStyle & FONTSTYLE_ITALIC)?BST_CHECKED:BST_UNCHECKED;
		isUnderline = (style._fontStyle & FONTSTYLE_UNDERLINE)?BST_CHECKED:BST_UNCHECKED;
		::SendMessage(_hCheckBold, BM_SETCHECK, isBold, 0);
		::SendMessage(_hCheckItalic, BM_SETCHECK, isItalic, 0);
		::SendMessage(_hCheckUnderline, BM_SETCHECK, isUnderline, 0);
		isEnable = true;
	}
	else // -1 est comme 0
	{
		::SendMessage(_hCheckBold, BM_SETCHECK, BST_UNCHECKED, 0);
		::SendMessage(_hCheckItalic, BM_SETCHECK, BST_UNCHECKED, 0);
		::SendMessage(_hCheckUnderline, BM_SETCHECK, BST_UNCHECKED, 0);
	}

    enableFontStyle(isEnable);


	//-- Default Keywords
	bool shouldBeDisplayed = style._keywordClass != -1;
	if (shouldBeDisplayed)
	{
		LexerStyler & lexerStyler = _pLexerStylerArray->getLexerFromIndex(_currentLexerIndex - 1);

		NppParameters *pNppParams = NppParameters::getInstance();
		LangType lType = pNppParams->getLangIDFromStr(lexerStyler.getLexerName());
		const char *kws = pNppParams->getWordList(lType, style._keywordClass);
		if (!kws)
			kws = "";
		::SendDlgItemMessage(_hSelf, IDC_DEF_KEYWORDS_EDIT, WM_SETTEXT, 0, (LPARAM)(kws));

		const char *ckwStr = (style._keywords)?style._keywords->c_str():"";
		::SendDlgItemMessage(_hSelf, IDC_USER_KEYWORDS_EDIT, WM_SETTEXT, 0, (LPARAM)(ckwStr));
	}
	int showOption = shouldBeDisplayed?SW_SHOW:SW_HIDE;
	::ShowWindow(::GetDlgItem(_hSelf, IDC_DEF_KEYWORDS_EDIT), showOption);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_USER_KEYWORDS_EDIT),showOption);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_DEF_KEYWORDS_STATIC), showOption);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_USER_KEYWORDS_STATIC),showOption);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_PLUSSYMBOL_STATIC),showOption);

    redraw();
}

void GlobalStyleDlg::create(int dialogID)
{
	StaticDialog::create(dialogID);

	if ((NppParameters::getInstance())->isTransparentAvailable())
	{
		::ShowWindow(::GetDlgItem(_hSelf, IDC_SC_TRANSPARENT_CHECK), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_SC_PERCENTAGE_SLIDER), SW_SHOW);
		
		::SendDlgItemMessage(_hSelf, IDC_SC_PERCENTAGE_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(20, 200));
		::SendDlgItemMessage(_hSelf, IDC_SC_PERCENTAGE_SLIDER, TBM_SETPOS, TRUE, 150);
		if (!(BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_SC_PERCENTAGE_SLIDER, BM_GETCHECK, 0, 0)))
			::EnableWindow(::GetDlgItem(_hSelf, IDC_SC_PERCENTAGE_SLIDER), FALSE);
	}
}

BOOL CALLBACK GlobalStyleDlg::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_INITDIALOG :
		{
			RECT rc;
			_styleTab.init(_hInst, _hSelf, false, true, true);

			_stylesControls.init(_hInst, _hSelf, &_lsArray, &_globalStyles);
			_stylesControls.doDialog();

            _lsArray = (NppParameters::getInstance())->getLStylerArray();
            _globalStyles = (NppParameters::getInstance())->getGlobalStylers();

            // Global Style
            _styleTab.insertAtEnd("Global Styles");

            // All the lexers
            for (int i = 0 ; i < _lsArray.getNbLexer() ; i++)
            {
			    //_styleTab.insertAtEnd(_lsArray.getLexerNameFromIndex(i));
				_styleTab.insertAtEnd(_lsArray.getLexerDescFromIndex(i));
            }
			
			getClientRect(rc);
			
            HWND hButton = ::GetDlgItem(_hSelf, IDCANCEL);
            POINT p;
            StaticDialog::alignWith(hButton, hButton, ALIGNPOS_TOP, p);
            rc.bottom = p.y;

            _styleTab.reSizeTo(rc);
            rc.top += 10;
            rc.bottom -= 100;
            rc.left += 5;
            rc.right -= 20;
            _stylesControls.reSizeTo(rc);

            _stylesControls.setStyleComboFromLexer(0);

			::EnableWindow(::GetDlgItem(_hSelf, IDOK), _isDirty);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_SAVECLOSE_BUTTON), !_isSync);

			_styleTab.display();
			_stylesControls.display();
			return TRUE;
		}

		case WM_DESTROY:
		{
			_styleTab.destroy();
			_stylesControls.destroy();
			return TRUE;
		}

        case WM_NOTIFY :
        {
            NMHDR *nmhdr = (NMHDR *)lParam;
            if (nmhdr->code == TCN_SELCHANGE)
	        {
                int indexClicked = int(::SendMessage(_styleTab.getHSelf(), TCM_GETCURSEL, 0, 0));
                _stylesControls.setStyleComboFromLexer(indexClicked);
            }
            return TRUE;
        }

		case WM_DATA_MODIFIED:
		{
			_isDirty = true;
			::EnableWindow(::GetDlgItem(_hSelf, IDOK), TRUE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_SAVECLOSE_BUTTON), TRUE);
			return TRUE;
		}

		case WM_HSCROLL :
		{
			if ((HWND)lParam == ::GetDlgItem(_hSelf, IDC_SC_PERCENTAGE_SLIDER))
			{
				int percent = ::SendDlgItemMessage(_hSelf, IDC_SC_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0);
				(NppParameters::getInstance())->SetTransparent(_hSelf, percent/*HIWORD(wParam)*/);
			}
			return TRUE;
		}

		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDCANCEL :
					if (_isDirty)
					{
						_lsArray = (NppParameters::getInstance())->getLStylerArray();
						_globalStyles = (NppParameters::getInstance())->getGlobalStylers();
						_isDirty = false;
                        _stylesControls.setVisualFromStyleCombo();
					}
					::EnableWindow(::GetDlgItem(_hSelf, IDOK), FALSE);
			        ::EnableWindow(::GetDlgItem(_hSelf, IDC_SAVECLOSE_BUTTON), !_isSync);
					display(false);
					return TRUE;

				case IDOK : //_isDirty == true;
                {
					LexerStylerArray & lsa = (NppParameters::getInstance())->getLStylerArray();
                    StyleArray & globalStyles = (NppParameters::getInstance())->getGlobalStylers();

					lsa = _lsArray;
                    globalStyles = _globalStyles;

					::EnableWindow(::GetDlgItem(_hSelf, IDOK), FALSE);
					_isDirty = false;
                    _isSync = false;
					::SendMessage(_hParent, WM_UPDATESCINTILLAS, 0, 0);
                    return TRUE;
                }

				case IDC_SAVECLOSE_BUTTON :
				{
                    if (_isDirty)
                    {
                        LexerStylerArray & lsa = (NppParameters::getInstance())->getLStylerArray();
                        StyleArray & globalStyles = (NppParameters::getInstance())->getGlobalStylers();

					    lsa = _lsArray;
                        globalStyles = _globalStyles;

                        ::EnableWindow(::GetDlgItem(_hSelf, IDOK), FALSE);
					    _isDirty = false;
                    }
                    (NppParameters::getInstance())->writeStyles(_lsArray, _globalStyles);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SAVECLOSE_BUTTON), FALSE);
                    _isSync = true;
					display(false);
					::SendMessage(_hParent, WM_UPDATESCINTILLAS, 0, 0);
					return TRUE;
				}
				
				case IDC_SC_TRANSPARENT_CHECK :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_SC_TRANSPARENT_CHECK, BM_GETCHECK, 0, 0));
					if (isChecked)
					{
						int percent = ::SendDlgItemMessage(_hSelf, IDC_SC_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0);
						(NppParameters::getInstance())->SetTransparent(_hSelf, percent);
					}
					else
						(NppParameters::getInstance())->removeTransparent(_hSelf);

					::EnableWindow(::GetDlgItem(_hSelf, IDC_SC_PERCENTAGE_SLIDER), isChecked);
					return TRUE;
				}

				default :
				{
					
					return FALSE;
				}
			}
		}

		default :
			return FALSE;
	}
	return FALSE;
}

