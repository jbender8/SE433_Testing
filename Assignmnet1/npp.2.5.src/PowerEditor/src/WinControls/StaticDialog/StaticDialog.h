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

#ifndef STATIC_DIALOG_H
#define STATIC_DIALOG_H

//#include "resource.h"
#include "Window.h"

enum PosAlign{ALIGNPOS_LEFT, ALIGNPOS_RIGHT, ALIGNPOS_TOP, ALIGNPOS_BOTTOM};

class StaticDialog : public Window
{
public :
	StaticDialog() : Window() {};
	~StaticDialog(){
	};
	virtual void create(int dialogID);

    virtual bool isCreated() const {
		return (_hSelf != NULL);
	};

    void destroy() {
		::DestroyWindow(_hSelf);
	};

protected :
	static BOOL CALLBACK dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) = 0;

    void alignWith(HWND handle, HWND handle2Align, PosAlign pos, POINT & point);
};

#endif //STATIC_DIALOG_H
