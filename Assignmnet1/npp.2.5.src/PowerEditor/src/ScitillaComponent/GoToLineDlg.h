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

#ifndef GOTILINE_DLG_H
#define GOTILINE_DLG_H

#include "StaticDialog.h"
#include "..\resource.h"

#include "ScintillaEditView.h"

class GoToLineDlg : public StaticDialog
{
public :
	GoToLineDlg() : StaticDialog() {};
	void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView) {
		Window::init(hInst, hPere);
		if (!ppEditView)
			throw int(9900);
		_ppEditView = ppEditView;
	};

	virtual void create(int dialogID) {
		StaticDialog::create(dialogID);
		
	/*	_currentStatus = REPLACE;

		initOptionsFromDlg();

		POINT p;
		//get screen coordonnees (x,y)
		::GetWindowRect(::GetDlgItem(_hSelf, IDREPLACE), &_replaceCancelPos);
		// set point (x,y)
		p.x = _replaceCancelPos.left;
		p.y = _replaceCancelPos.top;
		// convert screen coordonnees to client coordonnees
		::ScreenToClient(_hSelf, &p);
		// get the width and height
		::GetClientRect(::GetDlgItem(_hSelf, IDREPLACE), &_replaceCancelPos);
		// fill out _replaceCancelPos
		_replaceCancelPos.left = p.x;
		_replaceCancelPos.top = p.y;
		
		::GetWindowRect(::GetDlgItem(_hSelf, IDCANCEL), &_findCancelPos);
		p.x = _findCancelPos.left;
		p.y = _findCancelPos.top;
		::ScreenToClient(_hSelf, &p);
		::GetClientRect(::GetDlgItem(_hSelf, IDCANCEL), &_findCancelPos);
		_findCancelPos.left = p.x;
		_findCancelPos.top = p.y;
*/
	};
	

	void doDialog() {
		if (!isCreated())
			create(IDD_GOLINE);
		display();
	};
    virtual void display(bool toShow = true) const {
        //updateLinesNumbers();
        Window::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, ID_GOLINE_EDIT));
    };

protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :

    ScintillaEditView **_ppEditView;

    void updateLinesNumbers() const {
        ::SetDlgItemInt(_hSelf, ID_CURRLINE, (*_ppEditView)->getCurrentLineNumber() + 1, FALSE);
        ::SetDlgItemInt(_hSelf, ID_LASTLINE, (*_ppEditView)->execute(SCI_GETLINECOUNT), FALSE);
    };
    
    void cleanLineEdit() const {
        ::SetDlgItemText(_hSelf, ID_GOLINE_EDIT, "");
    };

    int getLine() const {
        BOOL isSuccessful;
        int line = ::GetDlgItemInt(_hSelf, ID_GOLINE_EDIT, &isSuccessful, FALSE);
        return (isSuccessful?line:-1);
    };

};

#endif //GOTILINE_DLG_H
