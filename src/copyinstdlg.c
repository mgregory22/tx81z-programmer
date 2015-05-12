/*
 * copyinstdlg.c - Copy to multiple instruments dialog
 *
 * Copyright (c) 2006, 2015 Matt Gregory
 *
 * This file is part of TX81Z Programmer.
 *
 * TX81Z Programmer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * TX81Z Programmer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with TX81Z Programmer.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "prog.h"
#include "resource.h"
#include "copyinstdlg.h"

/*
 * Global procedures
 */
extern int CopyInstDlg_Create(HWND parentWnd, int srcInst);

/*
 * Unit procedures
 */
static BOOL CALLBACK cd_DlgProc(HWND copyInstDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void cd_OnCommand(HWND copyInstDlg, UINT ctrlID, HWND ctrl, UINT notify);
static BOOL cd_OnInitDialog(HWND copyInstDlg, HWND focusCtrl, LPARAM lParam);

/*
 * Unit variables
 */
static int cd_srcInst;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
int CopyInstDlg_Create(HWND parentWnd, int srcInst)
{
    int result;

    cd_srcInst = srcInst;
    result = DialogBox(Prog_instance, (LPCTSTR) IDD_COPYINSTDLG, parentWnd
            , cd_DlgProc);
    if (result == -1) {
        Error_LastError();
        result = 0;
    }
    return result;
}

/*
 * DlgProc()
 */
BOOL CALLBACK cd_DlgProc(HWND copyInstDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(copyInstDlg, WM_COMMAND, cd_OnCommand);
        HANDLE_MSG(copyInstDlg, WM_INITDIALOG, cd_OnInitDialog);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void cd_OnCommand(HWND copyInstDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    int i;
    UINT result = 0;

    switch (ctrlID) {
        case IDOK:
            for (i = 0; i < 8; i++) {
                HWND checkBox = GetDlgItem(copyInstDlg, IDC_INST1_CHK + i);
                if (Button_IsChecked(checkBox)) {
                    result |= 1 << i;
                }
            }
        case IDCANCEL:
            EndDialog(copyInstDlg, result);
            break;
        default:
            break;
    }
}

/*
 * OnInitDialog()
 */
BOOL cd_OnInitDialog(HWND copyInstDlg, HWND focusCtrl, LPARAM lParam)
{
    HWND parentWnd = GetParent(copyInstDlg);
#define TEXT_LEN 80
    _TUCHAR text[TEXT_LEN];

    /*
     * Set up the window.
     */
    Window_Center(copyInstDlg, parentWnd);

    /*
     * Initialize control settings.
     */
    _sntprintf(text, TEXT_LEN, _T("Select the instruments you want to ")
            _T("copy the settings of instrument %d to."), cd_srcInst + 1);
    Static_SetText(GetDlgItem(copyInstDlg, IDC_COPY_INST_LBL), text);
    EnableWindow(GetDlgItem(copyInstDlg, IDC_INST1_CHK + cd_srcInst), FALSE);

    return TRUE;
}
