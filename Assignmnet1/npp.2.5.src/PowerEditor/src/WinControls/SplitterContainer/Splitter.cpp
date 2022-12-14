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


#include "SysMsg.h"
#include "Splitter.h"
//#include "resource.h"

bool Splitter::_isHorizontalRegistered = false;
bool Splitter::_isVerticalRegistered = false;
bool Splitter::_isHorizontalFixedRegistered = false;
bool Splitter::_isVerticalFixedRegistered = false;


#define SPLITTER_SIZE 8

Splitter::Splitter() : Window() 
{
	//hInstance = GetModuleHandle(NULL);
	_rect.left   = 0; // x axis 
	_rect.top    = 0; // y axis 
	_rect.right  = 0; // Width of the spliter.
	_rect.bottom = 0; // Height of the spliter
	_isFixed = false;
}


void Splitter::init( HINSTANCE hInst, HWND hPere, int splitterSize,
				int iSplitRatio, DWORD dwFlags)
{
	Window::init(hInst, hPere);
	_spiltterSize = splitterSize;

	WNDCLASSEX wcex;
	DWORD dwExStyle = 0L;
	DWORD dwStyle   = WS_CHILD | WS_VISIBLE;
	
	if (hPere == NULL)
	{
		::MessageBox(NULL, "pas de pere?", "Splitter::init", MB_OK);
		throw int(96);
	}
	if (iSplitRatio < 0)
	{
		::MessageBox(NULL, "it shoulds be 0 < ratio < 100", "Splitter::init", MB_OK);
		throw int(96);
	}
	_hParent = hPere;
	_dwFlags = dwFlags;
	
	if (_dwFlags & SV_FIXED)
	{
		//Fixed spliter
		_isFixed = true;
	}
	else
	{
		if (iSplitRatio >= 100)
		{
			//cant be 100 % or more 
			::MessageBox(NULL, "it shoulds be 0 < ratio < 100", "Splitter::init", MB_OK);
			throw int(96);
		}
	}

	_splitPercent = iSplitRatio;
	
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)staticWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _hInst;
	wcex.hIcon			= NULL;
	
	::GetClientRect(_hParent, &_rect);
	
	if (_dwFlags & SV_HORIZONTAL) //Horizontal spliter
	{
		_rect.top  = ((_rect.bottom * _splitPercent)/100);
		// y axis determined by the split% of the parent windows height
		
		_rect.left = 0;
		// x axis is always 0
		
		_rect.bottom = _spiltterSize;
		// the height of the spliter
		
		// the width of the splitter remains the same as the width of the parent window.
	}
	else //Vertical spliter
	{
		// y axis is 0 always 
		
		_rect.left = ((_rect.right * _splitPercent)/100); 
		// x axis determined by split% of the parent windows width.
		
		_rect.right = _spiltterSize; 
		// width of the spliter.			
		
		//height of the spliter remains the same as the height of the parent window
	}
	
	if (!_isFixed)
	{
		if ((_dwFlags & SV_ENABLERDBLCLK) || (_dwFlags & SV_ENABLELDBLCLK))
		{
			wcex.style = wcex.style | CS_DBLCLKS;
			// enable mouse double click messages.
		}
	}
	
	if (_isFixed)
	{
		wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
		// if fixed spliter then choose default cursor type.
        if (_dwFlags & SV_HORIZONTAL)
		    wcex.lpszClassName	= "fxdnsspliter";
        else
            wcex.lpszClassName	= "fxdwespliter";
	}
	else
	{
		if (_dwFlags & SV_HORIZONTAL)
		{
			//double sided arrow pointing north-south as cursor
			wcex.hCursor		= ::LoadCursor(NULL,IDC_SIZENS);
			wcex.lpszClassName	= "nsspliter";
		}
		else
		{
			// double sided arrow pointing east-west as cursor
			wcex.hCursor		= ::LoadCursor(NULL,IDC_SIZEWE);
			wcex.lpszClassName	= "wespliter";
		}
	}
	
	wcex.hbrBackground	= (HBRUSH)(COLOR_3DFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.hIconSm		= NULL;
	
	if ((_dwFlags & SV_HORIZONTAL)&&(!_isHorizontalRegistered))
	{
		RegisterClassEx(&wcex);
		_isHorizontalRegistered = true;
	}
	else if (isVertical()&&(!_isVerticalRegistered))
	{
		RegisterClassEx(&wcex);
		_isVerticalRegistered = true;
	}
    else if ((_dwFlags & SV_HORIZONTAL)&&(!_isHorizontalFixedRegistered))
    {
        RegisterClassEx(&wcex);
        _isHorizontalFixedRegistered = true;
    }
    else if (isVertical()&&(!_isVerticalFixedRegistered))
    {
        RegisterClassEx(&wcex);
        _isVerticalFixedRegistered = true;
    }

	_hSelf = CreateWindowEx(
				dwExStyle,
				wcex.lpszClassName,
				"",
				dwStyle,
				_rect.left, 
				_rect.top, 
				_rect.right, 
				_rect.bottom, 
				_hParent,
				NULL,
				_hInst,
				(LPVOID)this);
	
	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(345);
	}

	RECT rc;
	getClientRect(rc);
	//::GetClientRect(_hParent,&rc);

	_clickZone2TL.left = rc.left;
	_clickZone2TL.top = rc.top;

	int clickZoneWidth = getClickZone(WIDTH);
	int clickZoneHeight = getClickZone(HEIGHT);
	_clickZone2TL.right = clickZoneWidth;
	_clickZone2TL.bottom = clickZoneHeight;

	_clickZone2BR.left = rc.right - clickZoneWidth;
	_clickZone2BR.top = rc.bottom - clickZoneHeight;
	_clickZone2BR.right = clickZoneWidth;
	_clickZone2BR.bottom = clickZoneHeight;

	display();
	::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);

}
// determinated by (_dwFlags & SV_VERTICAL) && _splitterSize
int Splitter::getClickZone(WH which)
{
	if (_spiltterSize <= 8)
	{
		return isVertical()?(which==WIDTH?_spiltterSize:HIEGHT_MINIMAL)
							:(which==WIDTH?HIEGHT_MINIMAL:_spiltterSize);
	}
	else // (_spiltterSize > 8)
	{
		return isVertical()?(which==WIDTH? 8:15)
							:(which==WIDTH?15:8);
	}
}

LRESULT CALLBACK Splitter::staticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_NCCREATE:
		{
			Splitter * pSplitter = (Splitter *)((LPCREATESTRUCT)lParam)->lpCreateParams;
			pSplitter->_hSelf = hWnd;
			::SetWindowLong(hWnd, GWL_USERDATA, (long)pSplitter);
			return TRUE;
		}
		default:
		{
			Splitter * pSplitter = (Splitter *)::GetWindowLong(hWnd, GWL_USERDATA);
			if (!pSplitter)
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);

			return pSplitter->spliterWndProc(uMsg, wParam, lParam);
		}
	}
}

LRESULT CALLBACK Splitter::spliterWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDBLCLK:
		{

		}
		return 0;

	case WM_RBUTTONDBLCLK:
		{

		}
		return 0;

	case WM_LBUTTONDOWN:
		{
			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			if ((isInLeftTopZone(p))&&(wParam == MK_LBUTTON))
			{
				gotoTopLeft();
				return TRUE;
			}

			if ((isInRightBottomZone(p))&&(wParam == MK_LBUTTON))
			{
				gotoRightBouuom();
				return TRUE;
			}

			if (!_isFixed)
			{
				::SetCapture(_hSelf);
				_isDraged = true;
			}
		}
		return 0;

	case WM_RBUTTONDOWN :
		::SendMessage(_hParent, WM_USER + 1, wParam, lParam);
		return TRUE;

	case WM_MOUSEMOVE:
		{
			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			if (isInLeftTopZone(p) || isInRightBottomZone(p))
			{
				//::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_UP_ARROW)));
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				return TRUE;
			}

			if ((!_isFixed) && (wParam == MK_LBUTTON))
			{
				POINT pt; RECT rt;
				::GetClientRect(_hParent, &rt);

				::GetCursorPos(&pt);
				::ScreenToClient(_hParent, &pt);

				if (_dwFlags & SV_HORIZONTAL)
				{
					if (pt.y <= 1)
					{
						_rect.top = 1;
						_splitPercent = 1;
					}
					else
					{
						if (pt.y <= (rt.bottom - 5))
						{
							_rect.top = pt.y;
							_splitPercent = ((pt.y * 100 / rt.bottom*100) / 100);
						}
						else
						{
							_rect.top = rt.bottom - 5;
							_splitPercent = 99;
						}
					}
				}
				else
				{
					if (pt.x <= 1)
					{
						_rect.left = 1;
						_splitPercent = 1;
					}
					else
					{
						if (pt.x <= (rt.right - 5))
						{
							_rect.left = pt.x;
							_splitPercent = ((pt.x*100/rt.right*100)/100);
						}
						else
						{
							_rect.left = rt.right - 5;
							_splitPercent = 99;
						}
					}
				}

				::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);
				::MoveWindow(_hSelf, _rect.left, _rect.top, _rect.right, _rect.bottom, FALSE);
				redraw();
			}
			return 0;
		}
	case WM_LBUTTONUP:
		{
			if (!_isFixed)
			{
				ReleaseCapture();
			}
			return 0;
		}
	case WM_CAPTURECHANGED:
		{
			if (_isDraged)
			{
				::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);
				::MoveWindow(_hSelf, _rect.left, _rect.top, _rect.right, _rect.bottom, TRUE);
				_isDraged = false;
			}
			return 0;
		}

	case WM_PAINT :
		drawSplitter();
		return 0;

	case WM_CLOSE:
		destroy();
		return 0;
	}
	return ::DefWindowProc(_hSelf, uMsg, wParam, lParam);
}

void Splitter::resizeSpliter()
{
	RECT rect;
	
	::GetClientRect(_hParent,&rect);
	
	if (_dwFlags & SV_HORIZONTAL)
	{
		// for a Horizontal spliter the width remains the same 
		// as the width of the parent window, so get the new width of the parent.
		_rect.right = rect.right;
		
		//if resizeing should be done proportionately.
		if (_dwFlags & SV_RESIZEWTHPERCNT)
			_rect.top  = ((rect.bottom * _splitPercent)/100);
		else // soit la fenetre en haut soit la fenetre en bas qui est fixee
			_rect.top = getSplitterFixPosY();
	}
	else
	{
		// for a (default) Vertical spliter the height remains the same 
		// as the height of the parent window, so get the new height of the parent.
		_rect.bottom = rect.bottom;
		
		//if resizeing should be done proportionately.
		if (_dwFlags & SV_RESIZEWTHPERCNT) 
        {
			_rect.left = ((rect.right * _splitPercent)/100);
        }
        else // soit la fenetre gauche soit la fenetre droit qui est fixee
			_rect.left = getSplitterFixPosX();
        
	}
	::MoveWindow(_hSelf, _rect.left, _rect.top, _rect.right, _rect.bottom, TRUE);
	::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);
	
	RECT rc;
	getClientRect(rc);	
	_clickZone2BR.right = getClickZone(WIDTH);
	_clickZone2BR.bottom = getClickZone(HEIGHT);
	_clickZone2BR.left = rc.right - _clickZone2BR.right;
	_clickZone2BR.top = rc.bottom - _clickZone2BR.bottom;


	//force the window to repaint, to make sure the splitter is visible
	// in the new position.
	redraw();
}

void Splitter::gotoTopLeft() 
{
	if ((_dwFlags & SV_ENABLELDBLCLK) && (!_isFixed) && (_splitPercent > 1))
	{
		if (_dwFlags & SV_HORIZONTAL)
			_rect.top   = 1;
		else
			_rect.left   = 1;
		_splitPercent = 1;
		
		::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);
		::MoveWindow(_hSelf, _rect.left, _rect.top, _rect.right, _rect.bottom, TRUE);
		redraw();
	}
}

void Splitter::gotoRightBouuom() 
{
	if ((_dwFlags & SV_ENABLERDBLCLK) && (!_isFixed) && (_splitPercent < 99))
	{
		RECT rt;
		GetClientRect(_hParent,&rt);
		
		if (_dwFlags & SV_HORIZONTAL)
			_rect.top   = rt.bottom - _spiltterSize;
		else
			_rect.left   = rt.right - _spiltterSize;

		_splitPercent = 99;
		
		::SendMessage(_hParent, WM_USER, _rect.left, _rect.top);
		::MoveWindow(_hSelf, _rect.left, _rect.top, _rect.right, _rect.bottom, TRUE);
		redraw();
	}
}

void Splitter::drawSplitter() 
{
	PAINTSTRUCT ps;
	RECT rc, rcToDraw1, rcToDraw2, TLrc, BRrc;

	HDC hdc = ::BeginPaint(_hSelf, &ps);
	getClientRect(rc);

	if ((_spiltterSize >= 4) && (_dwFlags & SV_RESIZEWTHPERCNT))
	{
		adjustZoneToDraw(TLrc, TOP_LEFT);
		adjustZoneToDraw(BRrc, BOTTOM_RIGHT);
		paintArrow(hdc, TLrc, isVertical()?ARROW_LEFT:ARROW_UP);
	}

	if (isVertical())
	{
		rcToDraw2.top = (_dwFlags & SV_RESIZEWTHPERCNT)?_clickZone2TL.bottom:0;
		rcToDraw2.bottom = rcToDraw2.top + 2;

		rcToDraw1.top = rcToDraw2.top + 1;
		rcToDraw1.bottom = rcToDraw1.top + 2;
	}
	else
	{
		rcToDraw2.top = 1;
		rcToDraw2.bottom = 3;

		rcToDraw1.top = 2;
		rcToDraw1.bottom = 4;
	}

	int bottom = 0;
	if (_dwFlags & SV_RESIZEWTHPERCNT)
		bottom = (isVertical() ? rc.bottom - _clickZone2BR.bottom : rc.bottom);
	else
		bottom = rc.bottom;

	while (rcToDraw1.bottom <= bottom)
	{
		if (isVertical())
		{
			rcToDraw2.left = 1;
			rcToDraw2.right = 3;

			rcToDraw1.left = 2;
			rcToDraw1.right = 4;
		}
		else
		{
			rcToDraw2.left = _clickZone2TL.right;
			rcToDraw2.right = rcToDraw2.left + 2;

			rcToDraw1.left = rcToDraw2.left;
			rcToDraw1.right = rcToDraw1.left + 2;
		}

		while (rcToDraw1.right <= (isVertical() ? rc.right : rc.right - _clickZone2BR.right))
		{
			::FillRect(hdc, &rcToDraw1, (HBRUSH)(RGB(0xFF, 0xFF, 0xFF)));
			::FillRect(hdc, &rcToDraw2, (HBRUSH)(COLOR_3DSHADOW+1));

			rcToDraw2.left += 4;
			rcToDraw2.right += 4;
			rcToDraw1.left += 4;
			rcToDraw1.right += 4;
		}
		rcToDraw2.top += 4;
		rcToDraw2.bottom += 4;
		rcToDraw1.top += 4;
		rcToDraw1.bottom += 4;
	}

	if ((_spiltterSize >= 4) && (_dwFlags & SV_RESIZEWTHPERCNT))
		paintArrow(hdc, BRrc, isVertical()?ARROW_RIGHT:ARROW_DOWN);

	::EndPaint(_hSelf, &ps);
}

void Splitter::rotate()
{
	if (!_isFixed)
	{
		destroy();
		if (_dwFlags & SV_HORIZONTAL)
		{
			_dwFlags ^= SV_HORIZONTAL;
			_dwFlags |= SV_VERTICAL;
		}
		else //SV_VERTICAL
		{
			_dwFlags ^= SV_VERTICAL;
			_dwFlags |= SV_HORIZONTAL;
		}
		init(_hInst, _hParent, _spiltterSize, _splitPercent, _dwFlags);
	}
}

void Splitter::paintArrow(HDC hdc, const RECT &rect, Arrow arrowDir)
{
	RECT rc;
	rc.left = rect.left; rc.top = rect.top;
	rc.right = rect.right; rc.bottom = rect.bottom;
	if (arrowDir == ARROW_LEFT)
	{
		int x = rc.right;
		int y = rc.top;

		//::MoveToEx(hdc, x, y, NULL);
		for (; (x > rc.left) && (y != rc.bottom) ; x--)
		{
			::MoveToEx(hdc, x, y++, NULL);
			::LineTo(hdc, x, rc.bottom--);
		}
	}
	else if (arrowDir == ARROW_RIGHT)
	{
		int x = rc.left;
		int y = rc.top;

		//::MoveToEx(hdc, x, y, NULL);
		for (; (x < rc.right) && (y != rc.bottom) ; x++)
		{
			::MoveToEx(hdc, x, y++, NULL);
			::LineTo(hdc, x, rc.bottom--);
		}
	}
	else if (arrowDir == ARROW_UP)
	{
		int x = rc.left;
		int y = rc.bottom;

		//::MoveToEx(hdc, x, y, NULL);
		for (; (y > rc.top) && (x != rc.right) ; y--)
		{
			::MoveToEx(hdc, x++, y, NULL);
			::LineTo(hdc, rc.right--, y);
		}
	}
	else if (arrowDir == ARROW_DOWN)
	{
		int x = rc.left;
		int y = rc.top;

		//::MoveToEx(hdc, x, y, NULL);
		for (; (y < rc.bottom) && (x != rc.right) ; y++)
		{
			::MoveToEx(hdc, x++, y, NULL);
			::LineTo(hdc, rc.right--, y);
		}
	}
}
void Splitter::adjustZoneToDraw(RECT & rc2def, ZONE_TYPE whichZone) 
{
	if (_spiltterSize < 4) return;
	int x0, y0, x1, y1, w, h;
	if ((4 <= _spiltterSize) && (_spiltterSize <= 8))
	{
		w = (isVertical()?4:7);
		h = (isVertical()?7:4);
	}
	else // (_spiltterSize > 8)
	{
		w = (isVertical()?6:11);
		h = (isVertical()?11:6);
	}

	if (isVertical())
	{//w=4 h=7
		if (whichZone == TOP_LEFT)
		{
			x0 = 0;
			y0 = (_clickZone2TL.bottom - h) / 2;
		}
		else // whichZone == BOTTOM_RIGHT
		{
			x0 = _clickZone2BR.left + _clickZone2BR.right - w;
			y0 = (_clickZone2BR.bottom - h) / 2 + _clickZone2BR.top;
		}
		x1 = x0 + w;
		y1 = y0 + h;
	}
	else // Horizontal
	{//w=7 h=4
		if (whichZone == TOP_LEFT)
		{
			x0 = (_clickZone2TL.right - w) / 2;
			y0 = 0;
		}
		else // whichZone == BOTTOM_RIGHT
		{
			x0 = ((_clickZone2BR.right - w) / 2) + _clickZone2BR.left;
			y0 = _clickZone2BR.top + _clickZone2BR.bottom - h;
		}
		x1 = x0 + w;
		y1 = y0 + h;
	}
	rc2def.left = x0;
	rc2def.top = y0;
	rc2def.right = x1;
	rc2def.bottom = y1;
}
