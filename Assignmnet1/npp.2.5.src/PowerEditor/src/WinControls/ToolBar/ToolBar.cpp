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

//#include "..\..\resource.h"
#include "ToolBar.h"
#include "SysMsg.h"

const bool ToolBar::REDUCED = true;
const bool ToolBar::ENLARGED = false;

bool ToolBar::init(HINSTANCE hInst, HWND hPere, int iconSize, 
				   ToolBarButtonUnit *buttonUnitArray, int arraySize,
				   bool doUglyStandardIcon, int *bmpArray, int bmpArraySize)
{
	Window::init(hInst, hPere);
	_state = doUglyStandardIcon?TB_STANDARD:(iconSize >= 32?TB_LARGE:TB_SMALL);
	_bmpArray = bmpArray;
	_bmpArraySize = bmpArraySize;

	_toolBarIcons.init(buttonUnitArray, arraySize);
	_toolBarIcons.create(_hInst, iconSize);
	
//	_state = (iconSize < 32)?REDUCED:ENLARGED;

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);

	_hSelf = ::CreateWindowEx(
	               WS_EX_PALETTEWINDOW ,
	               TOOLBARCLASSNAME,
	               "",
	               WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
	               TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | CCS_TOP | BTNS_AUTOSIZE ,
	               0, 0,
	               0, 0,
	               _hParent,
				   NULL,
	               _hInst,
	               0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(9);
	}

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
	// backward compatibility.
	::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	
	if (!doUglyStandardIcon)
	{
		setDefaultImageList();
		setHotImageList();
		setDisableImageList();
	}
	else
	{
		::SendMessage(_hSelf, TB_LOADIMAGES, IDB_STD_SMALL_COLOR, reinterpret_cast<LPARAM>(HINST_COMMCTRL));

		TBADDBITMAP addbmp = {_hInst, 0};
		if (bmpArray)
		{
			for (int i = 0 ; i < _bmpArraySize ; i++)
			{
				addbmp.nID = _bmpArray[i];
				::SendMessage(_hSelf, TB_ADDBITMAP, 1, (LPARAM)&addbmp);
			}
		}
		
	}
	int nbElement = _toolBarIcons.getNbCommand();
	
	_pTBB = new TBBUTTON[nbElement];
	int inc = 1;

	for (int i = 0, j = 0; i < nbElement ; i++)
	{
		int cmd = 0;
		int bmpIndex, style;

		if ((cmd = _toolBarIcons.getCommandAt(i)) != 0)
		{
			if (doUglyStandardIcon)
			{
				int ibmp = _toolBarIcons.getUglyIconAt(i);
				bmpIndex = (ibmp == -1)?(STD_PRINT + (inc++)):ibmp;
			}
			else
				bmpIndex = j++;

			style = BTNS_BUTTON;
		}
		else
		{
			bmpIndex = 0;
			style = BTNS_SEP;
		}
		_pTBB[i].iBitmap = bmpIndex;
		_pTBB[i].idCommand = cmd;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = style; 
		_pTBB[i].dwData = 0; 
		_pTBB[i].iString = 0;

	}

	setButtonSize(iconSize, iconSize);

	::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nbElement, (LPARAM)_pTBB); 
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);

	return true;
}

void ToolBar::reset() 
{
	setDefaultImageList();
	setHotImageList();
	setDisableImageList();

	if (_state == TB_STANDARD)
	{
		int nbElement = _toolBarIcons.getNbCommand();
		for (int i = 0, j = 0, k = nbElement-1 ; i < nbElement ; i++, k--)
		{
			int cmd = 0;
			int bmpIndex, style;

			::SendMessage(_hSelf, TB_DELETEBUTTON, k, 0);

			if ((cmd = _toolBarIcons.getCommandAt(i)) != 0)
			{
				bmpIndex = j++;
				style = BTNS_BUTTON;
			}
			else
			{
				bmpIndex = 0;
				style = BTNS_SEP;
			}
			_pTBB[i].iBitmap = bmpIndex;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = style; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;

		}

		::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nbElement, (LPARAM)_pTBB); 
	}

	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
}

void ToolBar::setToUglyIcons() 
{
	if (_state == TB_STANDARD) 
		return;

	// Due to the drawback of toolbar control (in-coexistence of Imagelist - custom icons and Bitmap - Std icons),
	// We have to destroy the control then re-initialize it
	::DestroyWindow(_hSelf);

	//_state = REDUCED;

	_hSelf = ::CreateWindowEx(
	               WS_EX_PALETTEWINDOW ,
	               TOOLBARCLASSNAME,
	               "",
	               WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
	               TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | CCS_TOP | BTNS_AUTOSIZE ,
	               0, 0,
	               0, 0,
	               _hParent,
				   NULL,
	               _hInst,
	               0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(9);
	}

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
	// backward compatibility.
	::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	::SendMessage(_hSelf, TB_LOADIMAGES, IDB_STD_SMALL_COLOR, reinterpret_cast<LPARAM>(HINST_COMMCTRL));

	TBADDBITMAP addbmp = {_hInst, 0};
	if (_bmpArray)
	{
		for (int i = 0 ; i < _bmpArraySize ; i++)
		{
			addbmp.nID = _bmpArray[i];
			::SendMessage(_hSelf, TB_ADDBITMAP, 1, (LPARAM)&addbmp);
		}
	}

	int nbElement = _toolBarIcons.getNbCommand();
	int inc = 1;

	for (int i = 0, j = 0; i < nbElement ; i++)
	{
		int cmd = 0;
		int bmpIndex, style;

		if ((cmd = _toolBarIcons.getCommandAt(i)) != 0)
		{
			int ibmp = _toolBarIcons.getUglyIconAt(i);
			bmpIndex = (ibmp == -1)?(STD_PRINT + (inc++)):ibmp;
			style = BTNS_BUTTON;
		}
		else
		{
			bmpIndex = 0;
			style = BTNS_SEP;
		}
		_pTBB[i].iBitmap = bmpIndex;
		_pTBB[i].idCommand = cmd;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = style; 
		_pTBB[i].dwData = 0; 
		_pTBB[i].iString = 0;

	}

	setButtonSize(16, 16);

	::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nbElement, (LPARAM)_pTBB); 
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
	_state = TB_STANDARD;
}

