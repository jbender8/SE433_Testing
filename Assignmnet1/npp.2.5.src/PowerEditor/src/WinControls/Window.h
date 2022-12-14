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

#ifndef WINDOW_CONTROL_H
#define WINDOW_CONTROL_H

#include <windows.h>

class Window
{
public:
	Window(): _hInst(NULL), _hParent(NULL), _hSelf(NULL){};
	virtual ~Window() {};

	virtual void init(HINSTANCE hInst, HWND parent)
	{
		_hInst = hInst;
		_hParent = parent;
	}

	virtual void destroy() = 0;

	virtual void display(bool toShow = true) const {
		::ShowWindow(_hSelf, toShow?SW_SHOW:SW_HIDE);
	};
	
	virtual void reSizeTo(RECT & rc) // should NEVER be const !!!
	{ 
		::MoveWindow(_hSelf, rc.left, rc.top, rc.right, rc.bottom, TRUE);
		redraw();
	};

	virtual void redraw() const {
		::InvalidateRect(_hSelf, NULL, TRUE);
		::UpdateWindow(_hSelf);
	};
	
    virtual void getClientRect(RECT & rc) const {
		::GetClientRect(_hSelf, &rc);
	};
	
	virtual int getWidth() const {
		RECT rc;
		::GetClientRect(_hSelf, &rc);
		return (rc.right - rc.left);
	};

	virtual int getHeight() const {
		RECT rc;
		::GetClientRect(_hSelf, &rc);
		return (rc.bottom - rc.top);
	};
	
    virtual bool isVisible() const {
        return bool(::IsWindowVisible(_hSelf));
	};

	HWND getHSelf() const {
		if (!_hSelf)
		{
			::MessageBox(NULL, "_hSelf == NULL", "class Window", MB_OK);
			throw int(999);
		}
		return _hSelf;
	};

	void getFocus() const {
		::SetFocus(_hSelf);
	};

    HINSTANCE getHinst() const {
		if (!_hInst)
		{
			::MessageBox(NULL, "_hInst == NULL", "class Window", MB_OK);
			throw int(1999);
		}
		return _hInst;
	};
protected:
	HINSTANCE _hInst;
	HWND _hParent;
	HWND _hSelf;
};

#endif //WINDOW_CONTROL_H

