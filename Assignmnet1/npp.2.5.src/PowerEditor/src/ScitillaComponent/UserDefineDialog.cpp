//this file is part of Notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
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

#include "UserDefineDialog.h"
#include "ScintillaEditView.h"
#include "Parameters.h"
#include "resource.h"

UserLangContainer * SharedParametersDialog::_pUserLang = NULL;
ScintillaEditView * SharedParametersDialog::_pScintilla = NULL;

void SharedParametersDialog::initControls()
{
    for (int i = 0 ; i < _nbGroup ; i++)
    {
        HWND hFgColourStaticText = ::GetDlgItem(_hSelf, _fgStatic[i]);
        HWND hBgColourStaticText = ::GetDlgItem(_hSelf, _bgStatic[i]);
        
        _pFgColour[i] = new ColourPicker;
        _pFgColour[i]->init(_hInst, _hSelf);
        _pFgColour[i]->setColour(black);

        _pBgColour[i] = new ColourPicker;
        _pBgColour[i]->init(_hInst, _hSelf);
		_pBgColour[i]->setColour(white);

        POINT p1, p2;

        alignWith(hFgColourStaticText, _pFgColour[i]->getHSelf(), ALIGNPOS_RIGHT, p1);
        alignWith(hBgColourStaticText, _pBgColour[i]->getHSelf(), ALIGNPOS_RIGHT, p2);

        p1.x = p2.x = ((p1.x > p2.x)?p1.x:p2.x) + 10;
        p1.y -= 4; p2.y -= 4;

        ::MoveWindow(_pFgColour[i]->getHSelf(), p1.x, p1.y, 25, 25, TRUE);
        ::MoveWindow(_pBgColour[i]->getHSelf(), p2.x, p2.y, 25, 25, TRUE);
        _pFgColour[i]->display();
        _pBgColour[i]->display();
        
        //for the font size combos
        HWND hFontSizeCombo = ::GetDlgItem(_hSelf, _fontSizeCombo[i]);
        for(int j = 0 ; j < sizeof(fontSizeStrs)/3 ; j++)
        {
            ::SendMessage(hFontSizeCombo, CB_ADDSTRING, 0, (LPARAM)fontSizeStrs[j]);
        }
        
        //for the font name combos
        HWND hFontNameCombo = ::GetDlgItem(_hSelf, _fontNameCombo[i]);
        const std::vector<std::string> & fontlist = (NppParameters::getInstance())->getFontList();
        for (int j = 0 ; j < fontlist.size() ; j++)
        {
            int k = ::SendMessage(hFontNameCombo, CB_ADDSTRING, 0, (LPARAM)fontlist[j].c_str());
            ::SendMessage(hFontNameCombo, CB_SETITEMDATA, k, (LPARAM)fontlist[j].c_str());
        }
    }
}

void SharedParametersDialog::styleUpdate(const Style & style, ColourPicker *pFgColourPicker, ColourPicker *pBgColourPicker, 
										 int fontComboId, int fontSizeComboId, int boldCheckId, int italicCheckId)
{
	pFgColourPicker->setColour((style._fgColor == -1)?black:style._fgColor);
	pFgColourPicker->redraw();
	pBgColourPicker->setColour((style._bgColor == -1)?white:style._bgColor);
	pBgColourPicker->redraw();

	HWND hFontCombo = ::GetDlgItem(_hSelf, fontComboId);
	int i = ::SendMessage(hFontCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)style._fontName);
	if (i == CB_ERR)
		i = 0;
	::SendMessage(hFontCombo, CB_SETCURSEL, i, 0);

	char size[10];
	if (style._fontSize == -1)
		size[0] = '\0';
	else
		itoa(style._fontSize, size, 10);

	hFontCombo = ::GetDlgItem(_hSelf, fontSizeComboId);
	i = ::SendMessage(hFontCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)size);
	if (i != CB_ERR)
		::SendMessage(hFontCombo, CB_SETCURSEL, i, 0);
	int isBold = 0;
	int isItalic = 0;
	int isUnderline = 0;
	if (style._fontStyle != -1)
	{
		isBold = (style._fontStyle & FONTSTYLE_BOLD)?BST_CHECKED:BST_UNCHECKED;
		isItalic = (style._fontStyle & FONTSTYLE_ITALIC)?BST_CHECKED:BST_UNCHECKED;
		//isUnderline = (style._fontStyle & FONTSTYLE_UNDERLINE)?BST_CHECKED:BST_UNCHECKED;
	}
	::SendDlgItemMessage(_hSelf, boldCheckId, BM_SETCHECK, isBold, 0);
	::SendDlgItemMessage(_hSelf, italicCheckId, BM_SETCHECK, isItalic, 0);
	//::SendDlgItemMessage(_hSelf, underlineCheckId, BM_SETCHECK, isUnderline, 0);
}

int fgStatic[] = {IDC_DEFAULT_FG_STATIC, IDC_FOLDEROPEN_FG_STATIC, IDC_FOLDERCLOSE_FG_STATIC};
int bgStatic[] = {IDC_DEFAULT_BG_STATIC, IDC_FOLDEROPEN_BG_STATIC, IDC_FOLDERCLOSE_BG_STATIC};
int fontSizeCombo[] = {IDC_DEFAULT_FONTSIZE_COMBO, IDC_FOLDEROPEN_FONTSIZE_COMBO, IDC_FOLDERCLOSE_FONTSIZE_COMBO};
int fontNameCombo[] = {IDC_DEFAULT_FONT_COMBO, IDC_FOLDEROPEN_FONT_COMBO, IDC_FOLDERCLOSE_FONT_COMBO};

FolderStyleDialog::FolderStyleDialog() : SharedParametersDialog(3) 
{
	memcpy(_fgStatic, fgStatic, sizeof(fgStatic));
	memcpy(_bgStatic, bgStatic, sizeof(bgStatic));
	memcpy(_fontSizeCombo, fontSizeCombo, sizeof(fontSizeCombo));
	memcpy(_fontNameCombo, fontNameCombo, sizeof(fontNameCombo));
}

void FolderStyleDialog::setKeywords2List(int ctrlID) 
{
    int index;
    if (ctrlID == IDC_FOLDEROPEN_EDIT)
        index = 1;
    else if (ctrlID == IDC_FOLDERCLOSE_EDIT)
        index = 2;
    else
        index = -1;
        
    if (index != -1)
		::GetDlgItemText(_hSelf, ctrlID, _pUserLang->_keywordLists[index], max_char);
}

BOOL CALLBACK SharedParametersDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
		case WM_INITDIALOG :
		{
			initControls();
            return TRUE;
		}

		case WM_COMMAND : 
		{
            if (HIWORD(wParam) == EN_CHANGE)
            {
                setKeywords2List(LOWORD(wParam));

                if (_pScintilla->getCurrentDocType() == L_USER)
                    _pScintilla->defineDocType(L_USER);

                return TRUE;
            }
			else if (HIWORD(wParam) == CBN_SELCHANGE)
            {
				bool isFontSize;
				int k = getGroupIndexFromCombo(LOWORD(wParam), isFontSize);

				if (k != -1)
				{
					int i = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETCURSEL, 0, 0);
					Style & style = _pUserLang->_styleArray.getStyler(k);
					if (isFontSize)
					{
						char intStr[5];
						if (i != 0)
						{
							::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETLBTEXT, i, (LPARAM)intStr);
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
					}
					else
					{
						style._fontName = (char *)::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETITEMDATA, i, 0);
					}
					if (_pScintilla->getCurrentDocType() == L_USER)
						_pScintilla->defineDocType(L_USER);
					return TRUE;
				}
			
			}
			else if (HIWORD(wParam) == CPN_COLOURPICKED)
			{
                bool isFG;
                ColourPicker *pCP;
                int index = getStylerIndexFromCP((HWND)lParam, isFG, &pCP);
                if (index != -1)
                {
                    Style & style = _pUserLang->_styleArray.getStyler(index);
                    if (isFG)
                        style._fgColor = pCP->getColour();
                    else
                        style._bgColor = pCP->getColour();
                }

				// A cause de "#define CPN_COLOURPICKED (BN_CLICKED)"
				// Nous sommes obligés de mettre ce bloc ici !!!
				// A modifier !!!
				else
				{
					bool isBold;
					int k = getGroupeIndexFromCheck(wParam, isBold);

					if (k != -1)
					{
						Style & style = _pUserLang->_styleArray.getStyler(k);
						if (style._fontStyle == -1)
								style._fontStyle = 0;
						style._fontStyle ^= (isBold)?BOLD_MASK:ITALIC_MASK;
						//::MessageBox(NULL, "Bingo!!!", "", MB_OK);
					}
				}
				if (_pScintilla->getCurrentDocType() == L_USER)
					_pScintilla->defineDocType(L_USER);
                return TRUE;
			}
			return FALSE;
		}
		/*
		case WM_SIZE :
		{
			redraw();
			return TRUE;
		}
		*/
		case WM_DESTROY:
		{
			for (int i = 0 ; i < _nbGroup ; i++)
			{
				_pFgColour[i]->destroy();
				_pBgColour[i]->destroy();
				
				delete _pFgColour[i];
				delete _pBgColour[i];
			}
			return TRUE;
		}
	}
	return FALSE;
}

void FolderStyleDialog::updateDlg() 
{
	::SendDlgItemMessage(_hSelf, IDC_FOLDEROPEN_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_FOLDER_OPEN_INDEX]));
	::SendDlgItemMessage(_hSelf, IDC_FOLDERCLOSE_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_FOLDER_CLOSE_INDEX]));

	Style & defaultStyle = _pUserLang->_styleArray.getStyler(STYLE_DEFAULT_INDEX);
	styleUpdate(defaultStyle, _pFgColour[0], _pBgColour[0], IDC_DEFAULT_FONT_COMBO, 
		IDC_DEFAULT_FONTSIZE_COMBO, IDC_DEFAULT_BOLD_CHECK, IDC_DEFAULT_ITALIC_CHECK);

	Style & foStyle = _pUserLang->_styleArray.getStyler(STYLE_BLOCK_OPEN_INDEX);
	styleUpdate(foStyle, _pFgColour[1], _pBgColour[1], IDC_FOLDEROPEN_FONT_COMBO, 
		IDC_FOLDEROPEN_FONTSIZE_COMBO, IDC_FOLDEROPEN_BOLD_CHECK, IDC_FOLDEROPEN_ITALIC_CHECK);

	Style & fcStyle = _pUserLang->_styleArray.getStyler(STYLE_BLOCK_CLOSE_INDEX);
	styleUpdate(fcStyle, _pFgColour[2], _pBgColour[2], IDC_FOLDERCLOSE_FONT_COMBO, 
		IDC_FOLDERCLOSE_FONTSIZE_COMBO, IDC_FOLDERCLOSE_BOLD_CHECK, IDC_FOLDERCLOSE_ITALIC_CHECK);
}

int FolderStyleDialog::getStylerIndexFromCP(HWND hWnd, bool & isFG, ColourPicker **ppCP) const
{
    for (int i = 0 ; i < _nbGroup ; i++)
    {
        if (hWnd == _pFgColour[i]->getHSelf())
        {
            *ppCP = _pFgColour[i];
            isFG = true;
            return i;
        }
        if (hWnd == _pBgColour[i]->getHSelf())
        {
            *ppCP = _pBgColour[i];
            isFG = false;
            return i;
        }
    }
    return -1;
}

int fgStatic2[] = {IDC_KEYWORD1_FG_STATIC, IDC_KEYWORD2_FG_STATIC, IDC_KEYWORD3_FG_STATIC, IDC_KEYWORD4_FG_STATIC};
int bgStatic2[] = {IDC_KEYWORD1_BG_STATIC, IDC_KEYWORD2_BG_STATIC, IDC_KEYWORD3_BG_STATIC, IDC_KEYWORD4_BG_STATIC};
int fontSizeCombo2[] = {IDC_KEYWORD1_FONTSIZE_COMBO, IDC_KEYWORD2_FONTSIZE_COMBO, IDC_KEYWORD3_FONTSIZE_COMBO, IDC_KEYWORD4_FONTSIZE_COMBO};
int fontNameCombo2[] = {IDC_KEYWORD1_FONT_COMBO, IDC_KEYWORD2_FONT_COMBO, IDC_KEYWORD3_FONT_COMBO, IDC_KEYWORD4_FONT_COMBO};

KeyWordsStyleDialog::KeyWordsStyleDialog() : SharedParametersDialog(4) 
{
	memcpy(_fgStatic, fgStatic2, sizeof(fgStatic2));
	memcpy(_bgStatic, bgStatic2, sizeof(bgStatic2));
	memcpy(_fontSizeCombo, fontSizeCombo2, sizeof(fontSizeCombo2));
	memcpy(_fontNameCombo, fontNameCombo2, sizeof(fontNameCombo2));
}

void KeyWordsStyleDialog::setKeywords2List(int id) 
{
	int index;
	switch (id)
	{
		case IDC_KEYWORD1_EDIT : index = 5; break;
		case IDC_KEYWORD2_EDIT : index = 6; break;
		case IDC_KEYWORD3_EDIT : index = 7; break;
		case IDC_KEYWORD4_EDIT : index = 8; break;
		default : index = -1;
	}
    if (index != -1)
		::GetDlgItemText(_hSelf, id, _pUserLang->_keywordLists[index], max_char);
}

int KeyWordsStyleDialog::getStylerIndexFromCP(HWND hWnd, bool & isFG, ColourPicker **ppCP) const
{
    for (int i = 0 ; i < _nbGroup ; i++)
    {
        if (hWnd == _pFgColour[i]->getHSelf())
        {
            *ppCP = _pFgColour[i];
            isFG = true;
            return i+3;
        }
        if (hWnd == _pBgColour[i]->getHSelf())
        {
            *ppCP = _pBgColour[i];
            isFG = false;
            return i+3;
        }
    }
    return -1;
}

void KeyWordsStyleDialog::updateDlg() 
{
	::SendDlgItemMessage(_hSelf, IDC_KEYWORD1_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_KW1_INDEX]));
	::SendDlgItemMessage(_hSelf, IDC_KEYWORD2_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_KW2_INDEX]));
	::SendDlgItemMessage(_hSelf, IDC_KEYWORD3_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_KW3_INDEX]));
	::SendDlgItemMessage(_hSelf, IDC_KEYWORD4_EDIT, WM_SETTEXT, 0, (LPARAM)(_pUserLang->_keywordLists[KWL_KW4_INDEX]));

	Style & w1Style = _pUserLang->_styleArray.getStyler(STYLE_WORD1_INDEX);
	styleUpdate(w1Style, _pFgColour[0], _pBgColour[0], IDC_KEYWORD1_FONT_COMBO, 
		IDC_KEYWORD1_FONTSIZE_COMBO, IDC_KEYWORD1_BOLD_CHECK, IDC_KEYWORD1_ITALIC_CHECK);

	Style & w2Style = _pUserLang->_styleArray.getStyler(STYLE_WORD2_INDEX);
	styleUpdate(w2Style, _pFgColour[1], _pBgColour[1], IDC_KEYWORD2_FONT_COMBO, 
		IDC_KEYWORD2_FONTSIZE_COMBO, IDC_KEYWORD2_BOLD_CHECK, IDC_KEYWORD2_ITALIC_CHECK);

	Style & w3Style = _pUserLang->_styleArray.getStyler(STYLE_WORD3_INDEX);
	styleUpdate(w3Style, _pFgColour[2], _pBgColour[2], IDC_KEYWORD3_FONT_COMBO, 
		IDC_KEYWORD3_FONTSIZE_COMBO, IDC_KEYWORD3_BOLD_CHECK, IDC_KEYWORD3_BOLD_CHECK);

	Style & w4Style = _pUserLang->_styleArray.getStyler(STYLE_WORD4_INDEX);
	styleUpdate(w4Style, _pFgColour[3], _pBgColour[3], IDC_KEYWORD4_FONT_COMBO, 
		IDC_KEYWORD4_FONTSIZE_COMBO, IDC_KEYWORD4_BOLD_CHECK, IDC_KEYWORD4_ITALIC_CHECK);
}

int fgStatic3[] = {IDC_COMMENT_FG_STATIC, IDC_COMMENTLINE_FG_STATIC, IDC_NUMBER_FG_STATIC};
int bgStatic3[] = {IDC_COMMENT_BG_STATIC, IDC_COMMENTLINE_BG_STATIC, IDC_NUMBER_BG_STATIC};
int fontSizeCombo3[] = {IDC_COMMENT_FONTSIZE_COMBO, IDC_COMMENTLINE_FONTSIZE_COMBO, IDC_NUMBER_FONTSIZE_COMBO};
int fontNameCombo3[] = {IDC_COMMENT_FONT_COMBO, IDC_COMMENTLINE_FONT_COMBO, IDC_NUMBER_FONT_COMBO};

CommentStyleDialog::CommentStyleDialog() : SharedParametersDialog(3)
{
    memcpy(_fgStatic, fgStatic3, sizeof(fgStatic3));
	memcpy(_bgStatic, bgStatic3, sizeof(bgStatic3));
	memcpy(_fontSizeCombo, fontSizeCombo3, sizeof(fontSizeCombo3));
	memcpy(_fontNameCombo, fontNameCombo3, sizeof(fontNameCombo3));
}

void CommentStyleDialog::setKeywords2List(int id) 
{
    int i;
    switch (id)
    {
        case IDC_COMMENTOPEN_EDIT : 
        case IDC_COMMENTCLOSE_EDIT : 
        case IDC_COMMENTLINE_EDIT : 
            i = 4;
            break;
        default : i = -1;
    }
    if (i != -1)
    {
        char commentOpen[max_char];
        char commentClose[max_char];
        char commentLine[max_char];
        char newList[max_char] = "";
        ::GetDlgItemText(_hSelf, IDC_COMMENTOPEN_EDIT, commentOpen, max_char);
        ::GetDlgItemText(_hSelf, IDC_COMMENTCLOSE_EDIT, commentClose, max_char);
        ::GetDlgItemText(_hSelf, IDC_COMMENTLINE_EDIT, commentLine, max_char);
        convertTo(newList, commentOpen, '1');
        convertTo(newList, commentClose, '2');
        convertTo(newList, commentLine, '0');
        strcpy(_pUserLang->_keywordLists[i], newList);
    }
}

int CommentStyleDialog::getStylerIndexFromCP(HWND hWnd, bool & isFG, ColourPicker **ppCP) const
{
    for (int i = 0 ; i < _nbGroup ; i++)
    {
        if (hWnd == _pFgColour[i]->getHSelf())
        {
            *ppCP = _pFgColour[i];
            isFG = true;
            return i+7;
        }
        if (hWnd == _pBgColour[i]->getHSelf())
        {
            *ppCP = _pBgColour[i];
            isFG = false;
            return i+7;
        }
    }
    return -1;
}

void CommentStyleDialog::updateDlg()
{
	char commentOpen[256] = "";
	char commentClose[256] = "";
	char commentLine[256] = "";

	retrieve(commentOpen, _pUserLang->_keywordLists[KWL_COMMENT_INDEX], '1');
	retrieve(commentClose, _pUserLang->_keywordLists[KWL_COMMENT_INDEX], '2');
	retrieve(commentLine, _pUserLang->_keywordLists[KWL_COMMENT_INDEX], '0');

	::SendDlgItemMessage(_hSelf, IDC_COMMENTOPEN_EDIT, WM_SETTEXT, 0, (LPARAM)commentOpen);
	::SendDlgItemMessage(_hSelf, IDC_COMMENTCLOSE_EDIT, WM_SETTEXT, 0, (LPARAM)commentClose);
	::SendDlgItemMessage(_hSelf, IDC_COMMENTLINE_EDIT, WM_SETTEXT, 0, (LPARAM)commentLine);

	Style & commentStyle = _pUserLang->_styleArray.getStyler(STYLE_COMMENT_INDEX);
	styleUpdate(commentStyle, _pFgColour[0], _pBgColour[0], IDC_COMMENT_FONT_COMBO, 
		IDC_COMMENT_FONTSIZE_COMBO, IDC_COMMENT_BOLD_CHECK, IDC_COMMENT_ITALIC_CHECK);

	Style & commentLineStyle = _pUserLang->_styleArray.getStyler(STYLE_COMMENTLINE_INDEX);
	styleUpdate(commentLineStyle, _pFgColour[1], _pBgColour[1], IDC_COMMENTLINE_FONT_COMBO, 
		IDC_COMMENTLINE_FONTSIZE_COMBO, IDC_COMMENTLINE_BOLD_CHECK, IDC_COMMENTLINE_ITALIC_CHECK);

	Style & numberStyle = _pUserLang->_styleArray.getStyler(STYLE_NUMBER_INDEX);
	styleUpdate(numberStyle, _pFgColour[2], _pBgColour[2], IDC_NUMBER_FONT_COMBO, 
		IDC_NUMBER_FONTSIZE_COMBO, IDC_NUMBER_BOLD_CHECK, IDC_NUMBER_ITALIC_CHECK);

}

char symbolesArray[] = "+-*/.?!:;,%^$&\"'(_)=}]@\\`|[{#~<>";
const bool SymbolsStyleDialog::ADD = true;
const bool SymbolsStyleDialog::REMOVE = false;

int SymbolsStyleDialog::getStylerIndexFromCP(HWND hWnd, bool & isFG, ColourPicker **ppCP) const
{
	if (hWnd == _pFgColour[0]->getHSelf())
    {
        *ppCP = _pFgColour[0];
        isFG = true;
        return 10;
    }
    if (hWnd == _pBgColour[0]->getHSelf())
    {
        *ppCP = _pBgColour[0];
        isFG = false;
        return 10;
    }

    return -1;
}
void SymbolsStyleDialog::symbolAction(bool action)
{
	int id2Add, id2Remove;
	int idButton2Disable, idButton2Enable;
	if (action == ADD)
	{
		id2Add = IDC_ACTIVATED_SYMBOL_LIST;
		id2Remove = IDC_AVAILABLE_SYMBOLS_LIST;
		idButton2Enable = IDC_REMOVE_BUTTON;
		idButton2Disable = IDC_ADD_BUTTON;
		
	}
	else
	{
		id2Add = IDC_AVAILABLE_SYMBOLS_LIST;
		id2Remove = IDC_ACTIVATED_SYMBOL_LIST;
		idButton2Enable = IDC_ADD_BUTTON;
		idButton2Disable = IDC_REMOVE_BUTTON;
	}
	int i = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCURSEL, 0, 0);
	char s[2];
	::SendDlgItemMessage(_hSelf, id2Remove, LB_GETTEXT, i, (LPARAM)s);

	::SendDlgItemMessage(_hSelf, id2Add, LB_ADDSTRING, 0, (LPARAM)s);
	::SendDlgItemMessage(_hSelf, id2Remove, LB_DELETESTRING, i, 0);
	int count = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCOUNT, 0, 0);
	if (i == count)
		i -= 1;
		
	::SendDlgItemMessage(_hSelf, id2Remove, LB_SETCURSEL, i, 0);
	count = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCOUNT, 0, 0);

	// If there's no symbol, we activate another side
	if (!count)
	{
		::SendDlgItemMessage(_hSelf, id2Add, LB_SETCURSEL, 0, 0);
		::EnableWindow(::GetDlgItem(_hSelf, idButton2Enable), TRUE);
		::EnableWindow(::GetDlgItem(_hSelf, idButton2Disable), FALSE);
	}

	// Get the operators list
	count = ::SendDlgItemMessage(_hSelf, IDC_ACTIVATED_SYMBOL_LIST, LB_GETCOUNT, 0, 0);

	int j = 0;
	for (int i = 0 ; i < count ; i++)
	{
		::SendDlgItemMessage(_hSelf, IDC_ACTIVATED_SYMBOL_LIST, LB_GETTEXT, i, (LPARAM)s);
		_pUserLang->_keywordLists[3][j++] = s[0];
		_pUserLang->_keywordLists[3][j++] = ' ';
	}
	_pUserLang->_keywordLists[3][--j] = '\0';
	
	if (_pScintilla->getCurrentDocType() == L_USER)
		_pScintilla->defineDocType(L_USER);
}

void SymbolsStyleDialog::listboxsRemoveAll()
{
	int count = ::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_GETCOUNT, 0, 0);
	for (int i = count-1 ; i >= 0 ; i--)
	{
		::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_DELETESTRING, i, 0);
	}
	count = ::SendDlgItemMessage(_hSelf, IDC_ACTIVATED_SYMBOL_LIST, LB_GETCOUNT, 0, 0);
	for (int i = count-1 ; i >= 0 ; i--)
	{
		::SendDlgItemMessage(_hSelf, IDC_ACTIVATED_SYMBOL_LIST, LB_DELETESTRING, i, 0);
	}
}
void SymbolsStyleDialog::updateDlg() 
{
	listboxsReInit();

	//char symbols[256] = "";
	//getActiveSymbol(symbols, _pUserLang->_keywordLists[KWL_OPERATOR_INDEX]);
	const char *symbols = _pUserLang->_keywordLists[KWL_OPERATOR_INDEX];

	for (int i = 0 ; i < strlen(symbols) ; i++)
	{
		if (symbols[i] != ' ')
		{
			char s[2];
			s[0] = symbols[i];
			s[1] = '\0';
			int index = ::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_FINDSTRING, -1, (LPARAM)s);
			if (index == LB_ERR)
				//::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_SETCURSEL, index, 0);
				continue;

			int id2Add = IDC_ACTIVATED_SYMBOL_LIST;
			int id2Remove = IDC_AVAILABLE_SYMBOLS_LIST;
			int idButton2Enable = IDC_REMOVE_BUTTON;
			int idButton2Disable = IDC_ADD_BUTTON;

			//int i = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCURSEL, 0, 0);
			//char s[2];
			//::SendDlgItemMessage(_hSelf, id2Remove, LB_GETTEXT, index, (LPARAM)s);

			::SendDlgItemMessage(_hSelf, id2Add, LB_ADDSTRING, 0, (LPARAM)s);
			::SendDlgItemMessage(_hSelf, id2Remove, LB_DELETESTRING, index, 0);
			int count = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCOUNT, 0, 0);
			if (index == count)
				index -= 1;

			::SendDlgItemMessage(_hSelf, id2Remove, LB_SETCURSEL, index, 0);
			count = ::SendDlgItemMessage(_hSelf, id2Remove, LB_GETCOUNT, 0, 0);

			// If there's no symbol, we activate another side
			if (!count)
			{
				::SendDlgItemMessage(_hSelf, id2Add, LB_SETCURSEL, 0, 0);
				::EnableWindow(::GetDlgItem(_hSelf, idButton2Enable), TRUE);
				::EnableWindow(::GetDlgItem(_hSelf, idButton2Disable), FALSE);
			}
		}
	}
	
	Style & opStyle = _pUserLang->_styleArray.getStyler(STYLE_OPERATOR_INDEX);
	styleUpdate(opStyle, _pFgColour[0], _pBgColour[0], IDC_SYMBOL_FONT_COMBO, 
		IDC_SYMBOL_FONTSIZE_COMBO, IDC_SYMBOL_BOLD_CHECK, IDC_SYMBOL_ITALIC_CHECK);
}

void SymbolsStyleDialog::listboxsInit() 
{
	for (int i = 0 ; i < sizeof(symbolesArray)-1 ; i++)
	{
		char s[2];
		s[0] = symbolesArray[i];
		s[1] = '\0';
		::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_ADDSTRING, 0, (LPARAM)s);
	}
}

BOOL CALLBACK SymbolsStyleDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
		case WM_INITDIALOG :
		{
			// 2 listBoxes
			listboxsInit();

			::SendDlgItemMessage(_hSelf, IDC_AVAILABLE_SYMBOLS_LIST, LB_SETCURSEL, 0, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_REMOVE_BUTTON), FALSE);

			// font style and color style controls
			HWND hFgColourStaticText = ::GetDlgItem(_hSelf, IDC_SYMBOL_FG_STATIC);
			HWND hBgColourStaticText = ::GetDlgItem(_hSelf, IDC_SYMBOL_BG_STATIC);
	        
			_pFgColour[0] = new ColourPicker;
			_pFgColour[0]->init(_hInst, _hSelf);
	        _pFgColour[0]->setColour(black);
		
			_pBgColour[0] = new ColourPicker;
			_pBgColour[0]->init(_hInst, _hSelf);
			_pBgColour[0]->setColour(white);

			POINT p1, p2;

			alignWith(hFgColourStaticText, _pFgColour[0]->getHSelf(), ALIGNPOS_RIGHT, p1);
			alignWith(hBgColourStaticText, _pBgColour[0]->getHSelf(), ALIGNPOS_RIGHT, p2);

			p1.x = p2.x = ((p1.x > p2.x)?p1.x:p2.x) + 10;
			p1.y -= 4; p2.y -= 4;

			::MoveWindow(_pFgColour[0]->getHSelf(), p1.x, p1.y, 25, 25, TRUE);
			::MoveWindow(_pBgColour[0]->getHSelf(), p2.x, p2.y, 25, 25, TRUE);
			_pFgColour[0]->display();
			_pBgColour[0]->display();
	        
			//for the font size combos
			HWND hFontSizeCombo = ::GetDlgItem(_hSelf, IDC_SYMBOL_FONTSIZE_COMBO);
			for(int j = 0 ; j < sizeof(fontSizeStrs)/3 ; j++)
			{
				::SendMessage(hFontSizeCombo, CB_ADDSTRING, 0, (LPARAM)fontSizeStrs[j]);
			}
	        
			//for the font name combos
			HWND hFontNameCombo = ::GetDlgItem(_hSelf, IDC_SYMBOL_FONT_COMBO);
			const std::vector<std::string> & fontlist = (NppParameters::getInstance())->getFontList();
			for (int j = 0 ; j < fontlist.size() ; j++)
			{
				int k = ::SendMessage(hFontNameCombo, CB_ADDSTRING, 0, (LPARAM)fontlist[j].c_str());
				::SendMessage(hFontNameCombo, CB_SETITEMDATA, k, (LPARAM)fontlist[j].c_str());
			}
			return TRUE;
		}

		case WM_COMMAND : 
		{
			if ((wParam == IDC_ADD_BUTTON) || (wParam == IDC_REMOVE_BUTTON))
			{
				symbolAction((wParam == IDC_ADD_BUTTON)?ADD:REMOVE);
				if (_pScintilla->getCurrentDocType() == L_USER)
					_pScintilla->defineDocType(L_USER);
				return TRUE;
			}
			else if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE))
            {
				if ((LOWORD(wParam) == IDC_ACTIVATED_SYMBOL_LIST) || (LOWORD(wParam) == IDC_AVAILABLE_SYMBOLS_LIST))
				{
					int idButton2Enable;
					int idButton2Disable;

					if (LOWORD(wParam) == IDC_AVAILABLE_SYMBOLS_LIST)
					{
						idButton2Enable = IDC_ADD_BUTTON;
						idButton2Disable = IDC_REMOVE_BUTTON;
					}
					else
					{
						idButton2Enable = IDC_REMOVE_BUTTON;
						idButton2Disable = IDC_ADD_BUTTON;
					}

					int i = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), LB_GETCURSEL, 0, 0);
					if (i != LB_ERR)
					{
						::EnableWindow(::GetDlgItem(_hSelf, idButton2Enable), TRUE);
						int idListbox2Disable = (LOWORD(wParam)== IDC_AVAILABLE_SYMBOLS_LIST)?IDC_ACTIVATED_SYMBOL_LIST:IDC_AVAILABLE_SYMBOLS_LIST;
						::SendDlgItemMessage(_hSelf, idListbox2Disable, LB_SETCURSEL, -1, 0);
						::EnableWindow(::GetDlgItem(_hSelf, idButton2Disable), FALSE);
					}
					return TRUE;
				}

				bool isFontSize;
				int k = getGroupIndexFromCombo(LOWORD(wParam), isFontSize);

				if (k != -1)
				{
					int i = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETCURSEL, 0, 0);
					Style & style = _pUserLang->_styleArray.getStyler(k);
					if (isFontSize)
					{
						char intStr[5];
						if (i != 0)
						{
							::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETLBTEXT, i, (LPARAM)intStr);
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
					}
					else
					{
						style._fontName = (char *)::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETITEMDATA, i, 0);
					}
					if (_pScintilla->getCurrentDocType() == L_USER)
						_pScintilla->defineDocType(L_USER);
					return TRUE;
				}
			}
			else if (HIWORD(wParam) == CPN_COLOURPICKED)
			{
                bool isFG;
                ColourPicker *pCP;
                int index = getStylerIndexFromCP((HWND)lParam, isFG, &pCP);
                if (index != -1)
                {
                    Style & style = _pUserLang->_styleArray.getStyler(index);
                    if (isFG)
                        style._fgColor = pCP->getColour();
                    else
                        style._bgColor = pCP->getColour();
                }

				// A cause de "#define CPN_COLOURPICKED (BN_CLICKED)"
				// Nous sommes obligés de mettre ce bloc ici !!!
				// A modifier !!!
				else
				{
					
					bool isBold;
					int k = getGroupeIndexFromCheck(wParam, isBold);

					if (k != -1)
					{
						Style & style = _pUserLang->_styleArray.getStyler(k);
						if (style._fontStyle == -1)
								style._fontStyle = 0;
						style._fontStyle ^= (isBold)?BOLD_MASK:ITALIC_MASK;
					}
				}
				if (_pScintilla->getCurrentDocType() == L_USER)
					_pScintilla->defineDocType(L_USER);
                return TRUE;
			}
			return TRUE;
		}

		case WM_DESTROY:
		{
			_pFgColour[0]->destroy();
			_pBgColour[0]->destroy();
				
			delete _pFgColour[0];
			delete _pBgColour[0];
			return TRUE;
		}
	}
	return FALSE;
}

char styleName[][32] = {"DEFAULT", "FOLDEROPEN", "FOLDERCLOSE", "KEYWORD1", "KEYWORD2", "KEYWORD3", "KEYWORD4", "COMMENT", "COMMENT LINE", "NUMBER", "OPERATOR"};


UserDefineDialog::UserDefineDialog(): SharedParametersDialog(), _status(UNDOCK), _isDirty(false), _yScrollPos(0), _prevHightVal(0)
{
	_pCurrentUserLang = new UserLangContainer();

    // @REF #01 NE CHANGER PAS D'ORDRE !!!
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_IDENTIFIER, styleName[0]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_BLOCK_OPERATOR_OPEN, styleName[1]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_BLOCK_OPERATOR_CLOSE, styleName[2]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_WORD1, styleName[3]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_WORD2, styleName[4]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_WORD3, styleName[5]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_WORD4, styleName[6]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_COMMENT, styleName[7]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_COMMENTLINE, styleName[8]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_NUMBER, styleName[9]);
	_pCurrentUserLang->_styleArray.addStyler(SCE_USER_OPERATOR, styleName[10]);
}

UserDefineDialog::~UserDefineDialog()
{
	delete _pCurrentUserLang;
}

void UserDefineDialog::changeStyle()
{
    display(false);
    _status = !_status;
    ::SetDlgItemText(_hSelf, IDC_DOCK_BUTTON, (_status == DOCK)?"Undock":"Dock");

    long style = ::GetWindowLong(_hSelf, GWL_STYLE);
    if (!style)
        ::MessageBox(NULL,"echou GetWindowLong", "", MB_OK);

    style = (_status == DOCK)?
        ((style & ~WS_POPUP) & ~DS_MODALFRAME & ~WS_CAPTION) | WS_CHILD :
        (style & ~WS_CHILD) | WS_POPUP | DS_MODALFRAME | WS_CAPTION;

    long result = ::SetWindowLong(_hSelf, GWL_STYLE, style);
    if (!result)
        ::MessageBox(NULL,"echou SetWindowLong", "", MB_OK);    

    if (_status == DOCK)
        getActualPosSize();
    else
        restorePosSize();

    ::SetWindowPos(_hSelf, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOREPOSITION);
    ::SetParent(_hSelf, (_status == DOCK)?_hParent:NULL);
}

void UserDefineDialog::enableLangAndControlsBy(int index)
{
	_pUserLang = (index == 0)?_pCurrentUserLang:&((NppParameters::getInstance())->getULCFromIndex(index - 1));
	if (index != 0)
		::SetWindowText(::GetDlgItem(_hSelf, IDC_EXT_EDIT), _pUserLang->_ext);

	::ShowWindow(::GetDlgItem(_hSelf, IDC_EXT_STATIC), (index == 0)?SW_HIDE:SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_EXT_EDIT), (index == 0)?SW_HIDE:SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_RENAME_BUTTON), (index == 0)?SW_HIDE:SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REMOVELANG_BUTTON), (index == 0)?SW_HIDE:SW_SHOW);
}

BOOL CALLBACK UserDefineDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
        case WM_INITDIALOG :
        {
			_ctrlTab.init(_hInst, _hSelf, false);

			_folderStyleDlg.init(_hInst, _hSelf);
			_folderStyleDlg.create(IDD_FOLDER_STYLE_DLG);
			_folderStyleDlg.display();

			_keyWordsStyleDlg.init(_hInst, _hSelf);
			_keyWordsStyleDlg.create(IDD_KEYWORD_STYLE_DLG);
			_keyWordsStyleDlg.display(false);

			_commentStyleDlg.init(_hInst, _hSelf);
			_commentStyleDlg.create(IDD_COMMENT_STYLE_DLG);
			_commentStyleDlg.display(false);

			_symbolsStyleDlg.init(_hInst, _hSelf);
			_symbolsStyleDlg.create(IDD_SYMBOL_STYLE_DLG);
			_symbolsStyleDlg.display(false);

			_wVector.push_back(DlgEtName(&_folderStyleDlg, "Folder && Default"));
			_wVector.push_back(DlgEtName(&_keyWordsStyleDlg, "Keywords Lists"));
			_wVector.push_back(DlgEtName(&_commentStyleDlg, "Comment && Number"));
			_wVector.push_back(DlgEtName(&_symbolsStyleDlg, "Operators"));
			//_ctrlTab.init(_hInst, _hSelf, false, _wVector);
			_ctrlTab.createTabs(_wVector);
			_ctrlTab.display();
			RECT rc;
			getClientRect(rc);
			rc.top += 100;	
			rc.bottom -= 100;
			_ctrlTab.reSizeTo(rc);

			_folderStyleDlg.reSizeTo(rc);
			_keyWordsStyleDlg.reSizeTo(rc);
			_commentStyleDlg.reSizeTo(rc);
			_symbolsStyleDlg.reSizeTo(rc);
			//_folderStyleDlg.display(false);
			//_folderStyleDlg.display();

			::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_ADDSTRING, 0, (LPARAM)"User Define Language");
			for (int i = 0 ; i < NppParameters::getInstance()->getNbUserLang() ; i++)
			{
				UserLangContainer & userLangContainer = NppParameters::getInstance()->getULCFromIndex(i);
				::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_ADDSTRING, 0, (LPARAM)userLangContainer.getName());
			}
			::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_SETCURSEL, 0, 0);
			enableLangAndControlsBy(0);

			_pUserLang = _pCurrentUserLang;
			//_ctrlTab.display(false);

			if ((NppParameters::getInstance())->isTransparentAvailable())
			{
				::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_TRANSPARENT_CHECK), SW_SHOW);
				::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER), SW_SHOW);
				
				::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(20, 200));
				::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, TBM_SETPOS, TRUE, 150);
				if (!(BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, BM_GETCHECK, 0, 0)))
					::EnableWindow(::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER), FALSE);
			}
			SCROLLINFO si;
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE; //| SIF_PAGE; 
			si.nMin   = 0; 
			si.nMax   = 0; 
			//si.nPage  = _currentHight; 
			//si.nPos = 0;
			::SetScrollInfo(_hSelf, SB_VERT, &si, TRUE);
            return TRUE;
        }

		case WM_NOTIFY :		  
		{
			NMHDR *nmhdr = (NMHDR *)lParam;
			if (nmhdr->code == TCN_SELCHANGE)
			{
				if (nmhdr->hwndFrom == _ctrlTab.getHSelf())
				{
					_ctrlTab.clickedUpdate();
					return TRUE;
				}
			}
			break;
		}

		case WM_HSCROLL :
		{
			if ((HWND)lParam == ::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER))
			{
				int percent = ::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0);
				(NppParameters::getInstance())->SetTransparent(_hSelf, percent/*HIWORD(wParam)*/);
			}
			return TRUE;
		}

		case WM_COMMAND : 
		{
            if (HIWORD(wParam) == EN_CHANGE)
            {
				::SendDlgItemMessage(_hSelf, IDC_EXT_EDIT, WM_GETTEXT, extsLenMax, (LPARAM)(_pUserLang->_ext));
                return TRUE;
            }
            else if (HIWORD(wParam) == CBN_SELCHANGE)
            {
				if (LOWORD(wParam) == IDC_LANGNAME_COMBO)
				{
					int i = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETCURSEL, 0, 0);
					enableLangAndControlsBy(i);
					updateDlg();
				}
                return TRUE;
            }
            else
            {
			    switch (wParam)
			    {
				    case IDC_DOCK_BUTTON :
                    {
						int msg = WM_UNDOCK_USERDEFINE_DLG;
						
						if (_status == UNDOCK)
						{
							if ((NppParameters::getInstance())->isTransparentAvailable())
							{
								(NppParameters::getInstance())->removeTransparent(_hSelf);
								::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_TRANSPARENT_CHECK), SW_HIDE);
								::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER), SW_HIDE);
							}
							msg = WM_DOCK_USERDEFINE_DLG;
						}

                        changeStyle();

						if (_status == UNDOCK)
						{
							if ((NppParameters::getInstance())->isTransparentAvailable())
							{
								bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_UD_TRANSPARENT_CHECK, BM_GETCHECK, 0, 0));
								if (isChecked)
								{
									int percent = ::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0);
									(NppParameters::getInstance())->SetTransparent(_hSelf, percent);
								}
								::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_TRANSPARENT_CHECK), SW_SHOW);
								::ShowWindow(::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER), SW_SHOW);
							}
						}
                        ::SendMessage(_hParent, msg, 0, 0);
					    return TRUE;
                    }
    				case IDCANCEL :
						::SendMessage(_hParent, WM_CLOSE_USERDEFINE_DLG, 0, 0);
					    display(false);
					    return TRUE;

				    case IDC_REMOVELANG_BUTTON :
                    {
						int result = ::MessageBox(_hSelf, "Are you sure?", "Remove the current language", MB_YESNO);
						if (result == IDYES)
						{
							int i = ::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETCURSEL, 0, 0);
							char langName[256];
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETLBTEXT, i, (LPARAM)langName);

							//remove current language from combobox
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_DELETESTRING, i, 0);
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_SETCURSEL, i-1, 0);
							::SendMessage(_hSelf, WM_COMMAND, MAKELONG(IDC_LANGNAME_COMBO, CBN_SELCHANGE), (LPARAM)::GetDlgItem(_hSelf, IDC_LANGNAME_COMBO));

							//remove current language from userLangArray
							NppParameters::getInstance()->removeUserLang(i-1);

							//remove current language from langMenu
							HWND hNpp = ::GetParent(_hSelf);
							::RemoveMenu(::GetSubMenu(::GetMenu(hNpp), 6), IDM_LANG_USER + i, MF_BYCOMMAND);
							::DrawMenuBar(hNpp);
							::SendMessage(_hParent, WM_REMOVE_USERLANG, 0, (LPARAM)langName);
						}
					    return TRUE;
                    }
				    case IDC_RENAME_BUTTON :
                    {
						char langName[256];
						int i = ::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETCURSEL, 0, 0);
						::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETLBTEXT, i, (LPARAM)langName);

						StringDlg strDlg;
						strDlg.init(_hInst, _hSelf, "Rename Current Language Name", "Name : ", langName);

						char *newName = (char *)strDlg.doDialog();

						if (newName)
						{
							if (NppParameters::getInstance()->isExistingUserLangName(newName))
							{
								::MessageBox(_hSelf, "This name is used by another language,\rplease give another one.", "Err", MB_OK);
								::PostMessage(_hSelf, WM_COMMAND, IDC_RENAME_BUTTON, 0);
								return TRUE;
							}
							//rename current language name in combobox
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_DELETESTRING, i, 0);
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_INSERTSTRING, i, (LPARAM)newName);
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_SETCURSEL, i, 0);
							
							//rename current language name in userLangArray
							UserLangContainer & userLangContainer = NppParameters::getInstance()->getULCFromIndex(i-1);
							strcpy(userLangContainer._name, newName);

							//rename current language name in langMenu
							HWND hNpp = ::GetParent(_hSelf);
							::ModifyMenu(::GetSubMenu(::GetMenu(hNpp), 6), IDM_LANG_USER + i, MF_BYCOMMAND, IDM_LANG_USER + i, newName);
							::DrawMenuBar(hNpp);
							::SendMessage(_hParent, WM_RENAME_USERLANG, (WPARAM)newName, (LPARAM)langName);
						}

					    return TRUE;
                    }

					case IDC_ADDNEW_BUTTON :
					case IDC_SAVEAS_BUTTON :
                    {
						char langName[256];
						int i = ::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETCURSEL, 0, 0);
						//::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_GETLBTEXT, i, (LPARAM)langName);
						if (i == 0)
							wParam = IDC_ADDNEW_BUTTON;

						StringDlg strDlg;
						if (wParam == IDC_SAVEAS_BUTTON)
							strDlg.init(_hInst, _hSelf, "Save Current Language Name As...", "Name : ", "");
						else
							strDlg.init(_hInst, _hSelf, "Create New Language...", "Name : ", "");

						char *newName = (char *)strDlg.doDialog();

						if (newName)
						{
							if (NppParameters::getInstance()->isExistingUserLangName(newName))
							{
								::MessageBox(_hSelf, "This name is used by another language,\rplease give another one.", "Err", MB_OK);
								::PostMessage(_hSelf, WM_COMMAND, IDC_RENAME_BUTTON, 0);
								return TRUE;
							}
							//add current language in userLangArray at the end as a new lang
							UserLangContainer & userLang = (wParam == IDC_SAVEAS_BUTTON)?NppParameters::getInstance()->getULCFromIndex(i-1):*_pCurrentUserLang;
							int newIndex = NppParameters::getInstance()->addUserLangToEnd(userLang, newName);
							//UserLangContainer & newUserLang = NppParameters::getInstance()->getULCFromIndex(newIndex);
							//strcpy(newUserLang._name, newName);

							//add new language name in combobox
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_ADDSTRING, 0, LPARAM(newName));
							::SendDlgItemMessage(_hSelf, IDC_LANGNAME_COMBO, CB_SETCURSEL, newIndex + 1, 0);
							::SendMessage(_hSelf, WM_COMMAND, MAKELONG(IDC_LANGNAME_COMBO, CBN_SELCHANGE), (LPARAM)::GetDlgItem(_hSelf, IDC_LANGNAME_COMBO));

							//add new language name in langMenu
							HWND hNpp = ::GetParent(_hSelf);
							::InsertMenu(::GetSubMenu(::GetMenu(hNpp), 6), IDM_LANG_USER + newIndex /*+ 1*/, MF_BYCOMMAND, IDM_LANG_USER + newIndex + 1, newName);
							::DrawMenuBar(hNpp);
						}

					    return TRUE;
                    }
				
					case IDC_UD_TRANSPARENT_CHECK :
					{
						bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_UD_TRANSPARENT_CHECK, BM_GETCHECK, 0, 0));
						if (isChecked)
						{
							int percent = ::SendDlgItemMessage(_hSelf, IDC_UD_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0);
							(NppParameters::getInstance())->SetTransparent(_hSelf, percent);
						}
						else
							(NppParameters::getInstance())->removeTransparent(_hSelf);

						::EnableWindow(::GetDlgItem(_hSelf, IDC_UD_PERCENTAGE_SLIDER), isChecked);
						return TRUE;
					}

				    default :
					    break;
			    }
            }
			return FALSE;
		}

		case WM_DESTROY:
		{
			_folderStyleDlg.destroy();
			_keyWordsStyleDlg.destroy();
			_commentStyleDlg.destroy();
			_symbolsStyleDlg.destroy();

			_ctrlTab.destroy();
			return TRUE;
		}

		case WM_SIZE: 
		{
			int originalHight = _dlgPos.bottom; //- ((_status == DOCK)?_dlgPos.top:0);
			_currentHight = HIWORD (lParam);

			int diff = _currentHight - _prevHightVal;
			_prevHightVal = _currentHight;

			 int maxPos = originalHight - _currentHight;
			// Set the vertical scrolling range and page size
			SCROLLINFO si;
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_PAGE; 
			si.nMin   = 0; 
			si.nMax   = (_status == UNDOCK)?0:originalHight; 
			si.nPage  = _currentHight; 
			//si.nPos = 0;
			::SetScrollInfo(_hSelf, SB_VERT, &si, TRUE);

			if ((_yScrollPos >= maxPos) && (_currentHight < originalHight))
			{
				int nDelta = min(max(maxPos/10,5), maxPos - _yScrollPos);
				if (_yScrollPos > 0)
				{
					_yScrollPos -= diff;
					::SetScrollPos(_hSelf, SB_VERT, _yScrollPos, TRUE);
					::ScrollWindow(_hSelf, 0, diff, NULL, NULL);
				}
			}
			return TRUE; 
		}

		case WM_VSCROLL :
		{
			int originalHight = _dlgPos.bottom;// - _dlgPos.top;
			int nDelta;
			int maxPos = originalHight - _currentHight;

			switch (LOWORD (wParam))
			{
				 // user clicked the top arrow
				 case SB_LINEUP:
					 if (_yScrollPos <= 0)
						return FALSE;
					 nDelta = -min(max(maxPos/20, 5), _yScrollPos);
					  //si.nPos -= 1;
					  break;
					  
				 // user clicked the bottom arrow
				 case SB_LINEDOWN:
					 if (_yScrollPos >= maxPos)
						 return FALSE;
					 nDelta = min(max(maxPos/10,5), maxPos - _yScrollPos);
					  //si.nPos += 1;
					  break;
/*
				case SB_PAGEDOWN:
					if (_yScrollPos >= maxPos)
						return FALSE;
					nDelta = min(max(maxPos/10,5), maxPos - _yScrollPos);
					break;

				case SB_PAGEUP:
					if (_yScrollPos <= 0)
						return FALSE;
					nDelta = -min(max(maxPos/10,5), _yScrollPos);
					break;
*/
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:
					nDelta = (int)HIWORD(wParam) - _yScrollPos;
					break;
				default :
					return FALSE;
			}
			 _yScrollPos += nDelta;
			 ::SetScrollPos(_hSelf, SB_VERT, _yScrollPos, TRUE);
			 ::ScrollWindow(_hSelf, 0, -nDelta, NULL, NULL);
		}
    }
	
	return FALSE;
}
