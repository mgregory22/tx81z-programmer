/*
 * datadlg.c - Data options dialog
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
#include "snapshot.h"
#include "tx81z.h"
#include "datadlg.h"

/*
 * Global procedures
 */
extern BOOL DataDlg_Create(HWND parentWnd, DATADLGMODE mode);
extern BOOL CALLBACK DataDlg_DlgProc(HWND dataDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit constants
 */
#define CHECKBOX_CNT 10
static const REQUEST dd_btnReqFlags[] = {
    REQ_AVCED,
    REQ_PCED,
    REQ_FX,
    REQ_PC,
    REQ_MTO,
    REQ_MTF,
    REQ_SYS,
    REQ_VMEM,
    REQ_PMEM,
    REQ_VOICE_MODE | REQ_PRESETS
};
static int *dd_optionsVars[] = {
    &Prog_rxOptions,
    &Prog_txOptions,
    &Prog_exportOptions
};

/*
 * Unit procedures
 */
static void dd_OnCommand(HWND dataDlg, int cmdID, HWND ctrl, UINT notify);
static BOOL dd_OnInitDialog(HWND dataDlg, HWND focusCtrl, LPARAM lParam);
static LRESULT dd_OnNotify(HWND dataDlg, int ctrlID, LPNMHDR notifyInfo);


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL DataDlg_Create(HWND parentWnd, DATADLGMODE mode)
{
    return DialogBoxParam(Prog_instance, (LPCTSTR) IDD_DATADLG, parentWnd
            , DataDlg_DlgProc, (LPARAM) mode) == IDOK;
}

/*
 * DlgProc
 */
BOOL CALLBACK DataDlg_DlgProc(HWND dataDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(dataDlg, WM_INITDIALOG, dd_OnInitDialog);
        HANDLE_MSG(dataDlg, WM_COMMAND, dd_OnCommand);
    }
    return FALSE;
}

/*
 * OnCommand - handles OK and X buttons
 */
void dd_OnCommand(HWND dataDlg, int cmdID, HWND ctrl, UINT notify)
{
    int i;
    DATADLGMODE mode = GetWindowLong(dataDlg, DWL_USER);
    int *optionsVar = dd_optionsVars[mode];
    BOOL check = FALSE;

    switch (cmdID) {
        case IDOK:
            *optionsVar = 0;
            for (i = 0; i < CHECKBOX_CNT - (mode == DDM_TRANSMIT); i++) {
                if (IsDlgButtonChecked(dataDlg, IDC_FIRST_CHK + i)) {
                    *optionsVar |= dd_btnReqFlags[i];
                }
            }
        case IDCANCEL:
            EndDialog(dataDlg, cmdID);
            return;
        case IDC_CHECK_ALL_BTN:
            check = TRUE;
        case IDC_CLEAR_ALL_BTN:
            for (i = 0; i < CHECKBOX_CNT - (mode == DDM_TRANSMIT); i++) {
                CheckDlgButton(dataDlg, IDC_FIRST_CHK + i, check);
            }
            return;
    }
}

/*
 * OnInitDialog - centers the dialog in the main window
 */
BOOL dd_OnInitDialog(HWND dataDlg, HWND focusCtrl, LPARAM lParam)
{
    DATADLGMODE mode = (DATADLGMODE) (lParam > 2)
        ? ((OPENFILENAME *) lParam)->lCustData : lParam;
    static const _TUCHAR msgFmt[] = _T("Select which types of data you want to %s.");
#define MSG_LEN  256
    _TUCHAR msg[MSG_LEN];
    int *optionsVar = dd_optionsVars[mode];
    int i;

    /*
     * Adjust the window position.
     */
    Window_CenterInParent(dataDlg);
    /*
     * Set the message caption.
     */
    if (mode == DDM_RECEIVE || mode == DDM_TRANSMIT) {
        _sntprintf(msg, MSG_LEN, msgFmt
                , mode == DDM_RECEIVE ? _T("retrieve") : _T("transmit"));
        msg[MSG_LEN - 1] = '\0';
        Static_SetText(GetDlgItem(dataDlg, IDC_MSG), msg);
    }

    /*
     * Initialize the check boxes.
     */
    for (i = 0; i < CHECKBOX_CNT - (mode == DDM_TRANSMIT); i++) {
        if (mode == DDM_RECEIVE
                || Snapshot_IsItemGroupLoaded(&Prog_snapshot, i))
        {
            if (*optionsVar & dd_btnReqFlags[i]) {
                CheckDlgButton(dataDlg, IDC_FIRST_CHK + i, BST_CHECKED);
            }
        } else {
            EnableWindow(GetDlgItem(dataDlg, IDC_FIRST_CHK + i), FALSE);
        }
    }
    if (mode == DDM_TRANSMIT) {
        EnableWindow(GetDlgItem(dataDlg, IDC_PRESET_CHK), FALSE);
    }
    SetWindowLong(dataDlg, DWL_USER, mode);

    return TRUE;
}

