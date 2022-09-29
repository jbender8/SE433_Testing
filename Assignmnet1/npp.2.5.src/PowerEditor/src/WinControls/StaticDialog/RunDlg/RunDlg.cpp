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

#include "RunDlg.h"
#include "process.h"
#include "FileDialog.h"

static void extractArgs(char *cmd2Exec, char *args, const char *cmdEntier)
{
	int i = 0;
	for ( ; i < strlen(cmdEntier) ; i++)
	{
		if (cmdEntier[i] != ' ')
			cmd2Exec[i] = cmdEntier[i];
		else
			break;
	}
	cmd2Exec[i] = '\0';
	
	if (i < strlen(cmdEntier))
	{
		for ( ; (i < strlen(cmdEntier)) && (cmdEntier[i] == ' ') ; i++);
		if (i < strlen(cmdEntier))
		{
			for (int k = 0 ; i <= strlen(cmdEntier) ; i++, k++)
			{
				args[k] = cmdEntier[i];
			}
		}

		int l = strlen(args);
		if (args[l-1] == ' ')
		{
			for (l -= 2 ; (l > 0) && (args[l] == ' ') ; l--);
			args[l+1] = '\0';
		}

	}
	else
		args[0] = '\0';
};

BOOL CALLBACK RunDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			getClientRect(_rc);
			return TRUE;
		}

		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDCANCEL :
					display(false);
					return TRUE;
				
				case IDOK :
				{
					const int PATH_MAX_LEN = 256;
					char cmd[PATH_MAX_LEN*2];
					char cmdPure[PATH_MAX_LEN];
					char cmd2Exec[PATH_MAX_LEN];
					char args[PATH_MAX_LEN];
					char args2Exec[PATH_MAX_LEN];
					
					//char *pCmd = cmd;
					::GetDlgItemText(_hSelf, IDC_COMBO_RUN_PATH, cmd, PATH_MAX_LEN);
					
					extractArgs(cmdPure, args, cmd);
					::ExpandEnvironmentStrings(cmdPure, cmd2Exec, sizeof(cmd2Exec));
					::ExpandEnvironmentStrings(args, args2Exec, sizeof(args));

					
					//Process proggy(cmd, ".");
					//if (proggy.run() == TRUE)
					
					//::MessageBox(NULL, cmd2Exec, "", MB_OK);
					HINSTANCE hInst = ::ShellExecute(_hSelf, "open", cmd2Exec, args2Exec, ".", SW_SHOW);
					if (int(hInst) > 32)
					{
						addTextToCombo(cmd);
						display(false);
					}
					else
					{
						removeTextFromCombo(cmd);
					}
					return TRUE;
				}

				case IDC_BUTTON_FILE_BROWSER :
				{
					FileDialog fd(_hSelf, _hInst);
					fd.setExtFilter("Executable file : ", ".exe", ".com", ".cmd", ".bat", NULL);
					fd.setExtFilter("All files : ", ".*", NULL);

					if (stringVector *pfns = fd.doOpenDlg())
						addTextToCombo((pfns->at(0)).c_str());
					return TRUE;
				}

				default :
					break;
			}
		}
	}
	return FALSE;	
}

void RunDlg::addTextToCombo(const char *txt2Add) const
{
	HWND handle = ::GetDlgItem(_hSelf, IDC_COMBO_RUN_PATH);
	int i = ::SendMessage(handle, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)txt2Add);
	if (i == CB_ERR)
		i = ::SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)txt2Add);
	::SendMessage(handle, CB_SETCURSEL, i, 0);
}
void RunDlg::removeTextFromCombo(const char *txt2Remove) const
{
	HWND handle = ::GetDlgItem(_hSelf, IDC_COMBO_RUN_PATH);
	int i = ::SendMessage(handle, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)txt2Remove);
	if (i == CB_ERR)
		return;
	::SendMessage(handle, CB_DELETESTRING, i, 0);
}

void RunDlg::doDialog()
{
	if (!isCreated())
		create(IDD_RUN_DLG);

    // Adjust the position in the center
    RECT rc;
    ::GetClientRect(_hParent, &rc);
    POINT center;
    center.x = rc.left + (rc.right - rc.left)/2;
    center.y = rc.top + (rc.bottom - rc.top)/2;
    ::ClientToScreen(_hParent, &center);

    int x = center.x - _rc.right/2;
    int y = center.y - _rc.bottom/2;

    ::SetWindowPos(_hSelf, HWND_TOP, x, y, _rc.right, _rc.bottom, SWP_SHOWWINDOW);
	::SetFocus(::GetDlgItem(_hSelf, IDC_COMBO_RUN_PATH));

};
