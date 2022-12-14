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

#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include "Window.h"

//#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
//#endif //_WIN32_IE

#include <commctrl.h>

#ifndef TB_SETIMAGELIST
#define TB_SETIMAGELIST	(WM_USER+48)
#endif

#ifndef TB_SETHOTIMAGELIST
#define TB_SETHOTIMAGELIST	(WM_USER+52)
#endif

#ifndef TB_SETDISABLEDIMAGELIST
#define TB_SETDISABLEDIMAGELIST (WM_USER+54)
#endif

enum toolBarStatusType {TB_HIDE, TB_SMALL, TB_LARGE, TB_STANDARD};

#include "ImageListSet.h"

class ToolBar : public Window
{
public :
	const static bool REDUCED;
	const static bool ENLARGED;
	ToolBar():Window(), _pTBB(NULL){};
	virtual ~ToolBar(){};

	virtual bool init(HINSTANCE hInst, HWND hPere, int iconSize, 
		ToolBarButtonUnit *buttonUnitArray, int arraySize,
		bool doUglyStandardIcon = false, int *bmpArray = NULL, int bmpArraySize = 0);

	virtual void destroy() {
		delete [] _pTBB;
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
		_toolBarIcons.destroy();
	};
	void enable(int cmdID, bool doEnable) const {
		::SendMessage(_hSelf, TB_ENABLEBUTTON, cmdID, (LPARAM)doEnable);
	};

	int getHeight() const {
		if (!::IsWindowVisible(_hSelf))
			return 0;
		return Window::getHeight();
	};

	void reduce() {
		if (_state == TB_SMALL)
			return;
		// I really don't know why we have to enlarge then reduce 
		// to take the effect.
		if (_state == TB_STANDARD)
			_toolBarIcons.resizeIcon(32);

		_toolBarIcons.resizeIcon(16);
		reset();

		Window::redraw();
		_state = TB_SMALL;
	};
	void enlarge() {
		if (_state == TB_LARGE)
			return;
		_toolBarIcons.resizeIcon(32);
		reset();
		Window::redraw();
		_state = TB_LARGE;
	};

	void display(bool toShow = true) {
		Window::display(toShow);
		if (!toShow)
			_state = TB_HIDE;
	};

	bool getCheckState(int ID2Check) const {
		return bool(::SendMessage(_hSelf, TB_GETSTATE, (WPARAM)ID2Check, 0) & TBSTATE_CHECKED);
	};

	void setCheck(int ID2Check, bool willBeChecked) const {
		::SendMessage(_hSelf, TB_CHECKBUTTON, (WPARAM)ID2Check, (LPARAM)MAKELONG(willBeChecked, 0));
	};

	toolBarStatusType getState() const {return _state;};

	void setToUglyIcons();

	bool changeIcons(int whichLst, int iconIndex, const char *iconLocation){
		return _toolBarIcons.replaceIcon(whichLst, iconIndex, iconLocation);
	};

private :
	TBBUTTON *_pTBB;
	ToolBarIcons _toolBarIcons;
	toolBarStatusType _state;
	int *_bmpArray;
	int _bmpArraySize;


	void setDefaultImageList() {
		::SendMessage(_hSelf, TB_SETIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons.getDefaultLst());
	};
	void setHotImageList() {
		::SendMessage(_hSelf, TB_SETHOTIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons.getHotLst());
	};
	void setDisableImageList() {
		::SendMessage(_hSelf, TB_SETDISABLEDIMAGELIST, (WPARAM)0, (LPARAM)_toolBarIcons.getDisableLst());
	};



	void setButtonSize(int w, int h) {
		::SendMessage(_hSelf, TB_SETBUTTONSIZE , (WPARAM)0, (LPARAM)MAKELONG (w, h));
	};
	
	void reset();
	
};

#endif // TOOL_BAR_H
