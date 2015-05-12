/*
 * searchdlg.c - Comment editor
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
#include "searchdlg.h"

/*
 * Global procedures
 */
extern BOOL SearchDlg_Create(HWND parentWnd);

/*
 * Unit procedures
 */
static BOOL CALLBACK sd_DlgProc(HWND searchDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void sd_OnCommand(HWND searchDlg, UINT cmdID, HWND ctrl, UINT notify);
static BOOL sd_OnInitDialog(HWND searchDlg, HWND focusCtrl, LPARAM lParam);

/*
 * Unit variables
 */
static HWND sd_searchLcd;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL SearchDlg_Create(HWND parentWnd)
{
    return DialogBox(Prog_instance, (LPCTSTR) IDD_SEARCHDLG, parentWnd
            , sd_DlgProc) == IDOK;
}

/*
 * DlgProc()
 */
BOOL CALLBACK sd_DlgProc(HWND searchDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(searchDlg, WM_COMMAND, sd_OnCommand);
        HANDLE_MSG(searchDlg, WM_INITDIALOG, sd_OnInitDialog);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void sd_OnCommand(HWND searchDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    _TUCHAR text[80];

    switch (cmdID) {
        case IDOK:
            LcdCtrl_GetText(sd_searchLcd, text);
            ToAnsiCopy(TxLib_searchStr, text);
            if (IsDlgButtonChecked(searchDlg, IDC_NAME_RDO)) {
                TxLib_searchType = TST_NAME;
            } else if (IsDlgButtonChecked(searchDlg, IDC_COMMENT_RDO)) {
                TxLib_searchType = TST_COMMENT;
            } else {
                TxLib_searchType = TST_NAME_AND_COMMENT;
            }
        case IDCANCEL:
            EndDialog(searchDlg, cmdID);
            return;
    }
}

/*
 * OnInitDialog()
 */
BOOL sd_OnInitDialog(HWND searchDlg, HWND focusCtrl, LPARAM lParam)
{
    _TUCHAR text[TXLIB_SEARCHSTR_LEN];

    /*
     * Adjust the window position.
     */
    Window_CenterInParent(searchDlg);

    /*
     * Initialize control settings.
     */
    FromAnsiNCopy(text, TxLib_searchStr, TXLIB_SEARCHSTR_LEN);
    sd_searchLcd = GetDlgItem(searchDlg, IDC_SEARCH_LCD);
    LcdCtrl_TextInit(sd_searchLcd, TXLIB_SEARCHSTR_LEN);
    LcdCtrl_SetText(sd_searchLcd, text);
    CheckRadioButton(searchDlg, IDC_NAME_RDO, IDC_NAME_AND_COMMENT_RDO
            , IDC_NAME_RDO + TxLib_searchType);

    return TRUE;
}
