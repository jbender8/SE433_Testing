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

#ifndef WORD_STYLE_H
#define WORD_STYLE_H

#include "Window.h"
#include "ColourPicker.h"
#include "StaticDialog.h"
#include "WordStyleDlgRes.h"
#include "TabBar.h"
#include "Parameters.h"

#define WM_UPDATESCINTILLAS  (WM_USER + 69) //GlobalStyleDlg's msg 2 send 2 its parent
#define WM_DATA_MODIFIED     (WM_USER + 96) //WordStyleDlg's msg 2 send 2 its parent(GlobalStyleDlg)

enum fontStyleType {BOLD_STATUS, ITALIC_STATUS, UNDERLINE_STATUS};

const bool C_FOREGROUND = false;
const bool C_BACKGROUND = true;



class WordStyleDlg : public StaticDialog
{
public :
    void init(HINSTANCE hInst, HWND parent, LexerStylerArray *pLsa, StyleArray *pSa)
	{
        Window::init(hInst, parent);
		_pLexerStylerArray = pLsa;
        _pStylerArray = pSa;
	};

    void doDialog() {
    	if (!isCreated())
		create(IDD_STYLER_DLG);
	    display();
    };

    virtual void redraw() const {
        _pFgColour->redraw();
        _pBgColour->redraw();
    };

    void setStyleComboFromLexer(int index);
    void setVisualFromStyleCombo();

private :
    ColourPicker *_pFgColour;
    ColourPicker *_pBgColour;

    int _currentLexerIndex;
    HWND _hStyleCombo;
    HWND _hCheckBold;
    HWND _hCheckItalic;
	HWND _hCheckUnderline;
    HWND _hFontNameCombo;
    HWND _hFontSizeCombo;

	HWND _hFgColourStaticText;
	HWND _hBgColourStaticText;
	HWND _hFontNameStaticText;
	HWND _hFontSizeStaticText;


    LexerStylerArray *_pLexerStylerArray;
    StyleArray *_pStylerArray;

	BOOL CALLBACK run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam);


	Style & getCurrentStyler() {
		int styleIndex = int(::SendMessage(_hStyleCombo, CB_GETCURSEL, 0, 0));
        if (_currentLexerIndex == 0)
            return _pStylerArray->getStyler(styleIndex);
        else
        {
		    LexerStyler & lexerStyler = _pLexerStylerArray->getLexerFromIndex(_currentLexerIndex - 1);
		    return lexerStyler.getStyler(styleIndex);
        }
	};

	void updateColour(bool which);
	void updateFontStyleStatus(fontStyleType whitchStyle);
	void updateExtension();
	void updateFontName();
	void updateFontSize();
	void updateUserKeywords();

	void enableFg(bool isEnable) {
		::EnableWindow(_pFgColour->getHSelf(), isEnable);
		::EnableWindow(_hFgColourStaticText, isEnable);
	};

	void enableBg(bool isEnable) {
		::EnableWindow(_pBgColour->getHSelf(), isEnable);
		::EnableWindow(_hBgColourStaticText, isEnable);
	};

	void enableFontName(bool isEnable) {
		::EnableWindow(_hFontNameCombo, isEnable);
		::EnableWindow(_hFontNameStaticText, isEnable);
	};

	void enableFontSize(bool isEnable) {
		::EnableWindow(_hFontSizeCombo, isEnable);
		::EnableWindow(_hFontSizeStaticText, isEnable);
	};

	void enableFontStyle(bool isEnable) {
		::EnableWindow(_hCheckBold, isEnable);
		::EnableWindow(_hCheckItalic, isEnable);
		::EnableWindow(_hCheckUnderline, isEnable);
	};
    int notifyParentDataModified() {
        return ::SendMessage(_hParent, WM_DATA_MODIFIED, 0, 0);
    }
};

class GlobalStyleDlg : public StaticDialog
{
public :
	GlobalStyleDlg():_isDirty(false), _isSync(true){};
	virtual void create(int dialogID);

    void doDialog() {
    	if (!isCreated())
			create(IDD_GLOBAL_STYLER_DLG);
		display();
    };
	HWND getHChildDlg() const {
		return _stylesControls.getHSelf();
	};
private :
	TabBar _styleTab;
	WordStyleDlg _stylesControls;

    LexerStylerArray _lsArray;
    StyleArray _globalStyles;

	bool _isDirty;
    bool _isSync;

    BOOL CALLBACK run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
};

#endif //WORD_STYLE_H
