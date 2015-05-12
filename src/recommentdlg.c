/*
 * recommentdlg.c - Comment editor
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
#include "lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "recommentdlg.h"

/*
 * Global procedures
 */
extern BOOL RecommentDlg_Create(HWND parentWnd, _TUCHAR *comment);

/*
 * Unit procedures
 */
static BOOL CALLBACK rd_DlgProc(HWND recommentDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void rd_OnCommand(HWND recommentDlg, UINT cmdID, HWND ctrl, UINT notify);
static BOOL rd_OnInitDialog(HWND recommentDlg, HWND focusCtrl, LPARAM lParam);

/*
 * Unit variables
 */
static HWND rd_commentLcd;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL RecommentDlg_Create(HWND parentWnd, _TUCHAR *comment)
{
    return DialogBoxParam(Prog_instance, (LPCTSTR) IDD_RECOMMENTDLG, parentWnd
            , rd_DlgProc, (LPARAM) comment) == IDOK;
}

/*
 * DlgProc()
 */
BOOL CALLBACK rd_DlgProc(HWND recommentDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(recommentDlg, WM_COMMAND, rd_OnCommand);
        HANDLE_MSG(recommentDlg, WM_INITDIALOG, rd_OnInitDialog);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void rd_OnCommand(HWND recommentDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    _TUCHAR *comment;

    switch (cmdID) {
        case IDOK:
            comment = (_TUCHAR *) GetWindowLong(recommentDlg, DWL_USER);
            LcdCtrl_GetText(rd_commentLcd, comment);
        case IDCANCEL:
            EndDialog(recommentDlg, cmdID);
            return;
    }
}

/*
 * OnInitDialog()
 */
BOOL rd_OnInitDialog(HWND recommentDlg, HWND focusCtrl, LPARAM lParam)
{
    _TUCHAR *comment = (_TUCHAR *) lParam;

    /*
     * Adjust the window position.
     */
    Window_CenterInParent(recommentDlg);

    /*
     * Initialize control settings.
     */
    rd_commentLcd = GetDlgItem(recommentDlg, IDC_COMMENT_LCD);
    LcdCtrl_TextInit(rd_commentLcd, 40);
    LcdCtrl_SetText(rd_commentLcd, comment);
    SetWindowLong(recommentDlg, DWL_USER, (long) comment);

    return TRUE;
}
