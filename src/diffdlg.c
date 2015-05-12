/*
 * diffdlg.c - Displays the differences between two library items.
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
#include "txlbx.h"
#include "txpack.h"
#include "diffdlg.h"

/*
 * Data types
 */
typedef struct {
    _TUCHAR name[2][11];
    BYTE *item[2];
    int type;
} DLGDATA;

/*
 * Global procedures
 */
extern void DiffDlg_Create(HWND parentWnd, BYTE *item1, BYTE *item2, int type);

/*
 * Unit Constants
 */
#define LBX_COL1_X  (6 * 33)
#define LBX_COL2_X  (LBX_COL1_X + 5 + 6 * 10)

#define dd_divCnt 2
static int dd_divPos[dd_divCnt] = { LBX_COL1_X, LBX_COL2_X };
static TXLBX_PROPERTIES dd_txProps = { dd_divCnt, dd_divPos };

/*
 * Unit procedures
 */
static BOOL CALLBACK dd_DlgProc(HWND diffDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void dd_OnCommand(HWND diffDlg, UINT cmdID, HWND ctrl, UINT notify);
static BOOL dd_OnInitDialog(HWND diffDlg, HWND focusCtrl, LPARAM lParam);
static void dd_UpdateLbx(void);

/*
 * Unit variables
 */
static HWND dd_lbx;
static DLGDATA dd_dlgData;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
void DiffDlg_Create(HWND parentWnd, BYTE *item1, BYTE *item2, int type)
{
    dd_dlgData.item[0] = item1;
    dd_dlgData.item[1] = item2;
    dd_dlgData.type = type;

    DialogBoxParam(Prog_instance, (LPCTSTR) IDD_DIFFDLG, parentWnd
            , dd_DlgProc, 0);
}

/*
 * DlgProc()
 */
BOOL CALLBACK dd_DlgProc(HWND diffDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(diffDlg, WM_COMMAND, dd_OnCommand);
        HANDLE_MSG(diffDlg, WM_DRAWITEM, TxLbx_OnDrawItem);
        HANDLE_MSG(diffDlg, WM_INITDIALOG, dd_OnInitDialog);
        HANDLE_MSG(diffDlg, WM_MEASUREITEM, TxLbx_OnMeasureItem);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void dd_OnCommand(HWND diffDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    switch (cmdID) {
        case IDM_CLOSE:
        case IDCANCEL:
            EndDialog(diffDlg, cmdID);
            break;
        case IDM_VIEW_ALL:
        case IDM_VIEW_DIFFS:
        case IDM_VIEW_SAMES:
            CheckMenuRadioItem(GetMenu(diffDlg), IDM_VIEW_ALL, IDM_VIEW_SAMES
                    , cmdID, MF_BYCOMMAND);
            Prog_diffView = cmdID;
            dd_UpdateLbx();
            break;
    }
}

/*
 * OnInitDialog()
 */
BOOL dd_OnInitDialog(HWND diffDlg, HWND focusCtrl, LPARAM lParam)
{
    _TUCHAR itemName[11];
    int i;
    RECT lbxRect;
    int scrollBarW = GetSystemMetrics(SM_CXVSCROLL);

    itemName[10] = '\0';
    /*
     * Set up the window.
     */
    dd_lbx = GetDlgItem(diffDlg, IDC_DIFF_LBX);
    /*
     * Make sure the display area of the list box is big enough to show the
     * data.
     */
    if (scrollBarW != 16) {
        RECT wndRect;
        int sizeDiff = scrollBarW - 16;

        /*
         * Resize the dialog.
         */
        GetWindowRect(diffDlg, &wndRect);
        wndRect.right += sizeDiff;
        SetWindowPos(diffDlg, NULL, 0, 0, RECT_W(wndRect), RECT_H(wndRect)
                , SWP_NOMOVE | SWP_NOZORDER);
        /*
         * Resize the list box.
         */
        GetWindowRect(dd_lbx, &lbxRect);
        lbxRect.right += sizeDiff;
        SetWindowPos(dd_lbx, NULL, 0, 0, RECT_W(lbxRect), RECT_H(lbxRect)
                , SWP_NOMOVE | SWP_NOZORDER);
    }
    Window_CenterInParent(diffDlg);
    /*
     * Initialize the TX list box.
     */
    TxLbx_Init(dd_lbx, &dd_txProps);
    /*
     * Initialize control settings.
     */
    if (Prog_diffView == 0) {
        Prog_diffView = IDM_VIEW_ALL;
    }
    CheckMenuRadioItem(GetMenu(diffDlg), IDM_VIEW_ALL, IDM_VIEW_SAMES
            , Prog_diffView, MF_BYCOMMAND);
    for (i = 0; i < 2; i++) {
        if (dd_dlgData.type == META_VMEM) {
            FromAnsiNCopy(dd_dlgData.name[i]
                    , &dd_dlgData.item[i][TX81Z_VMEM_NAME], 10);
        } else if (dd_dlgData.type == META_PMEM) {
            FromAnsiNCopy(dd_dlgData.name[i]
                    , &dd_dlgData.item[i][TX81Z_PMEM_NAME], 10);
        }
    }
    dd_UpdateLbx();

    return TRUE;
}

/*
 * UpdateLbx()
 */
void dd_UpdateLbx(void)
{
    int i;
    int maxParam;
    int nameOffset;
    int nameLen;
    const TXPACK *txPack = NULL;

    /*
     * Set up some type-based parameters.
     */
    if (dd_dlgData.type == META_VMEM) {
        maxParam = TXPack_vcedCnt;
        nameOffset = TX81Z_VMEM_NAME;
        nameLen = 10;
        txPack = TXPack_vced;
    } else if (dd_dlgData.type == META_PMEM) {
        maxParam = TXPack_pcedCnt;
        nameOffset = TX81Z_PMEM_NAME;
        nameLen = 10;
        txPack = TXPack_pced;
    } else if (dd_dlgData.type == META_FX) {
        maxParam = TXPack_fxCnt;
        nameOffset = -1;
        nameLen = 0;
        txPack = TXPack_fx;
    } else if (dd_dlgData.type == META_SYS) {
        maxParam = TXPack_sysCnt;
        nameOffset = 11;
        nameLen = 16;
        txPack = TXPack_sys;
    }
    /*
     * Clear the list box.
     */
    ListBox_ResetContent(dd_lbx);
    /*
     * Go through each parameter and add them to the list box.
     */
DO_ACED:
    for (i = 0; i < maxParam; i++) {
        const TXPACK *pack = &txPack[i];
        BYTE data;
        BYTE shift;
        BYTE mask;
        int value1, value2;
#define TEXT_LEN  80
        _TUCHAR text[TEXT_LEN];

        /*
         * Get the parameter parameters.
         */
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        /*
         * Find the first item's parameter value.
         */
        data = dd_dlgData.item[0][pack->memOffset];
        value1 = ((data >> shift) & mask);
        /*
         * Find the second item's parameter value.
         */
        data = dd_dlgData.item[1][pack->memOffset];
        value2 = ((data >> shift) & mask);
        if (Prog_diffView == IDM_VIEW_ALL
                || (Prog_diffView == IDM_VIEW_DIFFS && value1 != value2)
                || (Prog_diffView == IDM_VIEW_SAMES && value1 == value2))
        {
            /*
             * Create the string for the list box item.
             */
            if (pack->memOffset >= nameOffset
                    && pack->memOffset < nameOffset + nameLen)
            {
                static const _TUCHAR quoteStr[2] = _T("\'");

                _sntprintf(text, TEXT_LEN, _T("%-32.32s %8s%c\' %8s%c\'")
                        , pack->title, quoteStr, value1, quoteStr, value2);
            } else {
                _sntprintf(text, TEXT_LEN, _T("%-32.32s %10d %10d")
                        , pack->title, value1, value2);
            }
            text[TEXT_LEN - 1] = '\0';
            /*
             * Add the string to the list box.
             */
            TxLbx_AddItem(dd_lbx, text, 0, (value1 == value2) ? TA_NORMAL
                    : TA_HIGHLIGHT);
        }
    }
    /*
     * VMEM's have another set of parameters...
     */
    if (dd_dlgData.type == META_VMEM && nameOffset > 0) {
        maxParam = TXPack_acedCnt;
        nameOffset = -1;
        txPack = TXPack_aced;
        goto DO_ACED;
    }
}

