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

#include "Notepad_plus.h"
#include "SysMsg.h"
#include <exception>

static bool isInList(const char *token2Find, char *list2Clean) {
	char word[1024];
	
	for (int i = 0, j = 0 ;  i <= strlen(list2Clean) ; i++)
	{
		if ((list2Clean[i] == ' ') || (list2Clean[i] == '\0'))
		{
			if (j)
			{
				word[j] = '\0';
				j = 0;
				bool bingo = !strcmp(token2Find, word);
				//::MessageBox(NULL, word, "token2Find", MB_OK);
				if (bingo)
				{
					int wordLen = strlen(word);
					int prevPos = i - wordLen;

					for (i = i + 1 ;  i <= strlen(list2Clean) ; i++, prevPos++)
						list2Clean[prevPos] = list2Clean[i];

					list2Clean[prevPos] = '\0';
					
					return true;
				}
			}
		}
		else
		{
			word[j++] = list2Clean[i];
		}
	}
	return false;
};


static LangType getLangTypeFromParam(char *list2Clean) {
	char word[1024];
	bool checkDash = true;
	bool checkL = false;
	bool action = false;
	int pos2Erase;

	for (int i = 0, j = 0 ;  i <= strlen(list2Clean) ; i++)
	{
		if ((list2Clean[i] == ' ') || (list2Clean[i] == '\0'))
		{
			if (action)
			{
				word[j] = '\0';
				j = 0;
				action = false;
				//::MessageBox(NULL, word, "getLangTypeFromParam", MB_OK);
				for (i = i + 1 ;  i <= strlen(list2Clean) ; i++, pos2Erase++)
					list2Clean[pos2Erase] = list2Clean[i];
						
				list2Clean[pos2Erase] = '\0';

				return NppParameters::getLangIDFromStr(word);
			}
			checkDash = true;
		}
		if (action)
		{
			word[j++] =  list2Clean[i];
		}
		else if (checkDash)
		{
			if (list2Clean[i] == '-')
				checkL = true;
		            
			if (list2Clean[i] != ' ')
				checkDash = false;
		}
		else if (checkL)
		{
			if (list2Clean[i] == 'l')
			{
				action = true;
				pos2Erase = i-1;
			}
			checkL = false;
		}
	}
	return L_TXT;
}; 

const char FLAG_MULTI_INSTANCE[] = "-multiInst";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int nCmdShow)
{
	
	bool multiInstance = isInList(FLAG_MULTI_INSTANCE, lpszCmdLine);
	LangType langType = getLangTypeFromParam(lpszCmdLine);
	if (!multiInstance)
	{
		HWND hNotepad_plus = ::FindWindow(Notepad_plus::getClassName(), NULL);
		if (hNotepad_plus)
		{
			// First all, destroy static object NppParameters
			(NppParameters::getInstance())->destroyInstance();

			if (::IsIconic(hNotepad_plus))
				::OpenIcon(hNotepad_plus);
	
			::SetForegroundWindow(hNotepad_plus);
			if (lpszCmdLine[0])
			{
				COPYDATASTRUCT copyData;
				copyData.dwData = langType;//(ULONG_PTR);
				copyData.cbData = DWORD(strlen(lpszCmdLine) + 1);
				copyData.lpData = lpszCmdLine;
				::SendMessage(hNotepad_plus, WM_COPYDATA, (WPARAM)hInstance, (LPARAM)&copyData);
			}
			return 0;
		}
	}

	Notepad_plus notepad_plus_plus;
	MSG msg;
	msg.wParam = 0;

	// WebControl bug
	//OleInitialize(NULL);
	try {
        char *pPathNames = NULL;
        if (lpszCmdLine[0])
        {
            pPathNames = lpszCmdLine;
        }
		(NppParameters::getInstance())->setDefLang(langType);
		notepad_plus_plus.init(hInstance, NULL, pPathNames);

		HACCEL hAccTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_M30_ACCELERATORS));
		MSG msg;
		msg.wParam = 0;
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			// if the message doesn't belong to the notepad_plus_plus's dialog
			if (!notepad_plus_plus.isDlgsMsg(&msg))
			{
				if (::TranslateAccelerator(notepad_plus_plus.getHSelf(), hAccTable, &msg) == 0)
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	} catch(int i) {
		if (i == 106901)
			::MessageBox(NULL, "Scintilla.init is failled!", "106901", MB_OK);
		else {
			char str[50] = "God Damn Exception : ";
			char code[10];
			itoa(i, code, 10);
			::MessageBox(NULL, strcat(str, code), "int exception", MB_OK);
		}
	}
	
	catch(std::exception ex) {
		::MessageBox(NULL, ex.what(), "Exception", MB_OK);
	}
	catch(...) {
		systemMessage("System Err");
	}
	// WebControl bug
	//OleUninitialize();
	return (UINT)msg.wParam;
}

