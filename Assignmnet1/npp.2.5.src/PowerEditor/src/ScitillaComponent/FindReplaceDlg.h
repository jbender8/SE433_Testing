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

#ifndef FIND_REPLACE_DLG_H
#define FIND_REPLACE_DLG_H

#include "StaticDialog.h"
#include "FindReplaceDlg_rc.h"

class ScintillaEditView;

typedef bool DIALOG_TYPE;
#define REPLACE true
#define FIND false

#define DIR_DOWN true
#define DIR_UP false

#define FIND_REPLACE_STR_MAX 256

const int REPLACE_ALL = 0;
const int MARK_ALL = 1;
const int COUNT_ALL = 2;

const int DISPLAY_POS_TOP = 2;
const int DISPLAY_POS_MIDDLE = 1;
const int DISPLAY_POS_BOTTOM = 0;

class FindReplaceDlg : public StaticDialog
{
public :
	FindReplaceDlg() : StaticDialog() {};
	void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView) {
		Window::init(hInst, hPere);
		if (!ppEditView)
			throw int(9900);
		_ppEditView = ppEditView;
	};

	virtual void create(int dialogID);
	
	void initOptionsFromDlg()	{
		_isWholeWord = isCheckedOrNot(IDWHOLEWORD);
		_isMatchCase = isCheckedOrNot(IDMATCHCASE);
		_isRegExp = isCheckedOrNot(IDREGEXP);
		_isWrapAround = isCheckedOrNot(IDWRAP);
		
		// Set Direction : Down by default
		_whitchDirection = DIR_DOWN;
		::SendMessage(::GetDlgItem(_hSelf, IDDIRECTIONDOWN), BM_SETCHECK, BST_CHECKED, 0);

		_doPurge = isCheckedOrNot(IDC_PURGE_CHECK);
		_doMarkLine = isCheckedOrNot(IDC_MARKLINE_CHECK);
		_doStyleFoundToken = isCheckedOrNot(IDC_STYLEFOUND_CHECK);

		::EnableWindow(::GetDlgItem(_hSelf, IDCMARKALL), (_doMarkLine || _doStyleFoundToken));
		::SendMessage(::GetDlgItem(_hSelf, IDC_DISPLAYPOS_BOTTOM), BM_SETCHECK, BST_CHECKED, 0);
	};


    void doFindDlg() {
		doDialog(FIND);
	};

	void doReplaceDlg() {
		doDialog(REPLACE);
	};

	void doDialog(DIALOG_TYPE witchType) {
		if (!isCreated())
			create(IDD_FIND_REPLACE_DLG);
		enableReplceFunc(witchType);
		::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
		display();
	};
	bool processFindNext();
	bool processReplace();
	//void processReplaceAll();
	void processAll(int op);

	void setSearchText(const char * txt2find) {
		if (!strcmp(txt2find, "")) return;
		HWND handle = ::GetDlgItem(_hSelf, IDFINDWHAT);
		int i = ::SendMessage(handle, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)txt2find);
		if (i == CB_ERR)
			i = ::SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)txt2find);
		::SendMessage(handle, CB_SETCURSEL, i, 0);
	};
protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
	DIALOG_TYPE _currentStatus;

	char _text2Find[FIND_REPLACE_STR_MAX];
	char _replaceText[FIND_REPLACE_STR_MAX];

	bool _isWholeWord;
	bool _isMatchCase;
	bool _isRegExp;
	bool _isWrapAround;
	bool _whitchDirection;

	bool _doPurge;
	bool _doMarkLine;
	bool _doStyleFoundToken;

	RECT _replaceCancelPos, _findCancelPos;

	ScintillaEditView **_ppEditView;

	void enableReplceFunc(bool isEnable) {
		int hideOrShow = isEnable?SW_SHOW:SW_HIDE;
		::ShowWindow(::GetDlgItem(_hSelf, ID_STATICTEXT_REPLACE),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEWITH),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEINSEL),hideOrShow);

		::ShowWindow(::GetDlgItem(_hSelf, IDCCOUNTALL),!hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_STATIC),!hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDCMARKALL),!hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_MARKLINE_CHECK),!hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_STYLEFOUND_CHECK),!hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_PURGE_CHECK),!hideOrShow);

		RECT *pRect = (!isEnable)?&_replaceCancelPos:&_findCancelPos;
		::MoveWindow(::GetDlgItem(_hSelf, IDCANCEL), pRect->left, pRect->top, pRect->right, pRect->bottom, TRUE);

		::SetWindowText(_hSelf, isEnable?"Replace":"Find");
		_currentStatus = isEnable;
	};

	void getSearchTexts() {
		getComboTextFrom(IDFINDWHAT);
	};

	void getReplaceTexts() {
		getComboTextFrom(IDREPLACEWITH);
	};

	void getComboTextFrom(int ID)
	{
		char *str;
		if (ID == IDFINDWHAT) str = _text2Find;
		else if (ID == IDREPLACEWITH) str = _replaceText;
		else return;

		::GetDlgItemText(_hSelf, ID, str, FIND_REPLACE_STR_MAX);
		if (_replaceText[0])
		{
			HWND handle = ::GetDlgItem(_hSelf, ID);
			if (CB_ERR == ::SendMessage(handle, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)str))
				::SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)str);
		}
	};

	bool isCheckedOrNot(int checkControlID) const {
		return (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, checkControlID), BM_GETCHECK, 0, 0));
	};

	int getDisplayPos() const {
		if (isCheckedOrNot(IDC_DISPLAYPOS_TOP))
			return DISPLAY_POS_TOP;
		else if (isCheckedOrNot(IDC_DISPLAYPOS_MIDDLE))
			return DISPLAY_POS_MIDDLE;
		else //IDC_DISPLAYPOS_BOTTOM
			return DISPLAY_POS_BOTTOM;
	};
};

#endif //FIND_REPLACE_DLG_H
