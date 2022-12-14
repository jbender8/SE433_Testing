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
#include <stdarg.h>
#include "FileDialog.h"

FileDialog *FileDialog::staticThis = NULL;

FileDialog::FileDialog(HWND hwnd, HINSTANCE hInst, bool isMultiSel, bool genre) 
	: _nbCharFileExt(0), _isMultiSel(isMultiSel), _nbExt(0)
{staticThis = this;
    for (int i = 0 ; i < nbExtMax ; i++)
        _extArray[i][0] = '\0';

    memset(_fileExt, 0x00, sizeof(_fileExt));
	_fileName[0] = '\0';
 
	_ofn.lStructSize = sizeof(_ofn);     
	_ofn.hwndOwner = hwnd; 
	_ofn.hInstance = hInst;
	_ofn.lpstrFilter = _fileExt;
	_ofn.lpstrCustomFilter = (LPTSTR) NULL;
	_ofn.nMaxCustFilter = 0L;
	_ofn.nFilterIndex = 1L;
	_ofn.lpstrFile = _fileName;
	_ofn.nMaxFile = sizeof(_fileName);
	_ofn.lpstrFileTitle = NULL;
	_ofn.nMaxFileTitle = 0;
	_ofn.lpstrInitialDir = NULL;
	_ofn.lpstrTitle = NULL;
	_ofn.nFileOffset  = 0;
	_ofn.nFileExtension = 0;
	_ofn.lpstrDefExt = NULL;  // No default extension
	_ofn.lCustData = 0;
	_ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_LONGNAMES | DS_CENTER;

    if (genre == styleSave)
    {
        _ofn.Flags |= /*OFN_ENABLEHOOK |*/ OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
        //_ofn.lpfnHook = OFNHookProc;
    }
    else 
    {
        _ofn.Flags |= OFN_FILEMUSTEXIST | _isMultiSel?OFN_ALLOWMULTISELECT:0;
    }
}

// This function set and concatenate the filter into the list box of FileDialog.
// The 1st parameter is the description of the file type, the 2nd .. Nth parameter(s) is (are)
// the file extension which should be ".WHATEVER", otherwise it (they) will be considered as
// a file name to filter. Since the nb of arguments is variable, you have to add NULL at the end.
// example : 
// FileDialog.setExtFilter("c/c++ src file", ".c", ".cpp", ".cxx", ".h", NULL);
// FileDialog.setExtFilter("Makeile", "makefile", "GNUmakefile", NULL);
void FileDialog::setExtFilter(const char *extText, const char *ext, ...)
{
    // fill out the ext array for save as file dialog
    if (_nbExt < nbExtMax)
        strcpy(_extArray[_nbExt++], ext);
    // 
    std::string extFilter = extText;
   
    va_list pArg;
    va_start(pArg, ext);

    std::string exts;

	if (ext[0] == '.')
		exts += "*";
    exts += ext;
    exts += ";";

    const char *ext2Concat;

    while (ext2Concat = va_arg(pArg, const char *))
	{
        if (ext2Concat[0] == '.')
            exts += "*";
        exts += ext2Concat;
        exts += ";";
	}
	va_end(pArg);

	// remove the last ';'
    exts = exts.substr(0, exts.length()-1);

    extFilter += " (";
    extFilter += exts + ")";
    
    char *pFileExt = _fileExt + _nbCharFileExt;
    memcpy(pFileExt, extFilter.c_str(), extFilter.length() + 1);
    _nbCharFileExt += extFilter.length() + 1;
    
    pFileExt = _fileExt + _nbCharFileExt;
    memcpy(pFileExt, exts.c_str(), exts.length() + 1);
    _nbCharFileExt += exts.length() + 1;
}

stringVector * FileDialog::doOpenDlg()
{
	if (::GetOpenFileName(&_ofn))
	{
		if (_isMultiSel)
		{
			char fn[MAX_PATH];
			char *pFn = _fileName + strlen(_fileName) + 1;
			if (!(*pFn))
				_fileNames.push_back(std::string(_fileName));
			else
				strcat(strcpy(fn, _fileName), "\\");
			int term = int(strlen(fn));
			while (*pFn)
			{
				fn[term] = '\0';
				strcat(fn, pFn);
				_fileNames.push_back(std::string(fn));
				pFn += strlen(pFn) + 1;
			}
		}
		else
		{
			_fileNames.push_back(std::string(_fileName));
		}
		return &_fileNames;
	}
	else
		return NULL;
}

UINT_PTR CALLBACK FileDialog::OFNHookProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG :
        {
			::SetWindowLong(hWnd, GWL_USERDATA, (long)staticThis);
            //pStaticDlg->run_dlgProc(message, wParam, lParam);
			return TRUE;
		}

		default :
		{
			FileDialog *pFileDialog = reinterpret_cast<FileDialog *>(::GetWindowLong(hWnd, GWL_USERDATA));
			if (!pFileDialog)
				return FALSE;
			return pFileDialog->run(hWnd, uMsg, wParam, lParam);
		}
    }
    //::OFNHookProc(hWnd, uMsg, wParam, lParam);
    return FALSE;
}