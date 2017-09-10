/*
 * renamedlg.c - Name editor
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
#include "ctrl/lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "dlg/renamedlg.h"

/*
 * Global procedures
 */
extern BOOL RenameDlg_Create(HWND parentWnd, RENAMEINFO *renameInfo);

/*
 * Unit procedures
 */
static BOOL CALLBACK rd_DlgProc(HWND renameDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void rd_OnCommand(HWND renameDlg, UINT cmdID, HWND ctrl, UINT notify);
static BOOL rd_OnInitDialog(HWND renameDlg, HWND focusCtrl, LPARAM lParam);
static LRESULT rd_OnNotify(HWND renameDlg, UINT ctrlID, LPNMHDR notifyInfo);
static void rd_OnVScroll(HWND renameDlg, HWND ctrl, UINT code, int pos);
static void rd_ShowCountInLcd(_TUCHAR *lcdText);
static void rd_UpdateRenameInfo(void);

/*
 * Unit variables
 */
static HWND rd_nameLcd;
static HWND rd_useCountChk;
static HWND rd_countPosLbl;
static HWND rd_countPosTrk;
static HWND rd_countStartLbl;
static HWND rd_countStartSpn;
static _TUCHAR rd_realName[11];
static RENAMEINFO *rd_renameInfo;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL RenameDlg_Create(HWND parentWnd, RENAMEINFO *renameInfo)
{
    rd_renameInfo = renameInfo;
    return DialogBoxParam(Prog_instance, (LPCTSTR) IDD_RENAMEDLG, parentWnd
            , rd_DlgProc, 0) == IDOK;
}

/*
 * DlgProc()
 */
BOOL CALLBACK rd_DlgProc(HWND renameDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(renameDlg, WM_COMMAND, rd_OnCommand);
        HANDLE_MSG(renameDlg, WM_INITDIALOG, rd_OnInitDialog);
        HANDLE_MSG(renameDlg, WM_NOTIFY, rd_OnNotify);
        HANDLE_MSG(renameDlg, WM_VSCROLL, rd_OnVScroll);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void rd_OnCommand(HWND renameDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    switch (cmdID) {
        case IDC_NAME_LCD:
            if (notify == LCN_EDITUPDATE) {
                int cursorPos = LcdCtrl_GetCursorPos(ctrl);
                _TUCHAR text[TX81Z_NAME_LEN + 1];

                rd_realName[cursorPos] = LcdCtrl_GetChar(ctrl, cursorPos);
                if (Button_IsChecked(rd_useCountChk)) {
                    rd_UpdateRenameInfo();
                    LcdCtrl_GetText(ctrl, text);
                    rd_ShowCountInLcd(text);
                    LcdCtrl_SetText(ctrl, text);
                }
            }
            break;
        case IDC_USE_COUNT_CHK:
            if (Button_IsChecked(rd_useCountChk)) {
                EnableWindow(rd_countPosLbl, TRUE);
                EnableWindow(rd_countPosTrk, TRUE);
                EnableWindow(rd_countStartLbl, TRUE);
                EnableWindow(rd_countStartSpn, TRUE);
            } else {
                EnableWindow(rd_countPosLbl, FALSE);
                EnableWindow(rd_countPosTrk, FALSE);
                EnableWindow(rd_countStartLbl, FALSE);
                EnableWindow(rd_countStartSpn, FALSE);
            }
            rd_UpdateRenameInfo();
            rd_ShowCountInLcd(NULL);
            break;
        case IDOK:
            _tcsncpy(rd_renameInfo->name, rd_realName, TX81Z_NAME_LEN);
        case IDCANCEL:
            EndDialog(renameDlg, cmdID);
            break;
    }
}

/*
 * OnNotify()
 */
LRESULT rd_OnNotify(HWND renameDlg, UINT ctrlID, LPNMHDR notifyInfo)
{
    switch (notifyInfo->idFrom) {
        case IDC_COUNT_POS_TRK:
            rd_UpdateRenameInfo();
            rd_ShowCountInLcd(NULL);
            break;
    }
    return 0;
}

/*
 * OnVScroll()
 */
void rd_OnVScroll(HWND renameDlg, HWND ctrl, UINT code, int pos)
{
    rd_UpdateRenameInfo();
    rd_ShowCountInLcd(NULL);
}

/*
 * OnInitDialog()
 */
BOOL rd_OnInitDialog(HWND renameDlg, HWND focusCtrl, LPARAM lParam)
{
    /*
     * Adjust the window position.
     */
    Window_CenterInParent(renameDlg);

    /*
     * Cache control handles
     */
    rd_nameLcd = GetDlgItem(renameDlg, IDC_NAME_LCD);
    rd_useCountChk = GetDlgItem(renameDlg, IDC_USE_COUNT_CHK);
    rd_countPosLbl = GetDlgItem(renameDlg, IDC_COUNT_POS_LBL);
    rd_countPosTrk = GetDlgItem(renameDlg, IDC_COUNT_POS_TRK);
    rd_countStartLbl = GetDlgItem(renameDlg, IDC_COUNT_START_LBL);
    rd_countStartSpn = GetDlgItem(renameDlg, IDC_COUNT_START_SPN);

    /*
     * Init controls.
     */
    LcdCtrl_TextInit(rd_nameLcd, TX81Z_NAME_LEN);
    _tcsncpy(rd_realName, rd_renameInfo->name, TX81Z_NAME_LEN);
    LcdCtrl_SetText(rd_nameLcd, rd_realName);
    TrackBar_SetRange(rd_countPosTrk, 0, 9, FALSE);
    TrackBar_SetPos(rd_countPosTrk, 9, TRUE);
    UpDown_SetRange(rd_countStartSpn, 0, 9999);
    UpDown_SetPos(rd_countStartSpn, 1);
    if (!rd_renameInfo->useCount) {
        EnableWindow(rd_useCountChk, FALSE);
    }
    EnableWindow(rd_countPosLbl, FALSE);
    EnableWindow(rd_countPosTrk, FALSE);
    EnableWindow(rd_countStartLbl, FALSE);
    EnableWindow(rd_countStartSpn, FALSE);

    return TRUE;
}

void rd_ShowCountInLcd(_TUCHAR *lcdText)
{
    _TUCHAR tmpName[TX81Z_NAME_LEN + 1];
    _TUCHAR *dest;

    if (lcdText) {
        dest = lcdText;
    } else {
        dest = tmpName;
    }
    _tcsncpy(dest, rd_realName, TX81Z_NAME_LEN);
    dest[TX81Z_NAME_LEN] = '\0';
    if (rd_renameInfo->useCount) {
        int count = rd_renameInfo->countStart;
        int i;

        for (i = rd_renameInfo->countPos; i >= 0; i--) {
            dest[i] = '0' + count % 10;
            count /= 10;
            if (count == 0) {
                break;
            }
        }
    }
    if (!lcdText) {
        LcdCtrl_SetText(rd_nameLcd, tmpName);
        InvalidateRect(rd_nameLcd, NULL, FALSE);
    }
}

void rd_UpdateRenameInfo(void)
{
    rd_renameInfo->useCount = Button_IsChecked(rd_useCountChk);
    rd_renameInfo->countPos = TrackBar_GetPos(rd_countPosTrk);
    rd_renameInfo->countStart = UpDown_GetPos(rd_countStartSpn);
}
