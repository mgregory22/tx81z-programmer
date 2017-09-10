/*
 * storedlg.c - Dialog to select a memory number for storing an item.
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
#include "snapshot.h"
#include "tx81z.h"
#include "ctrl/txlbx.h"
#include "dlg/storedlg.h"

/*
 * Global procedures
 */
extern BOOL StoreDlg_Create(HWND parentWnd, SINDEX itemToStore, int *destIndex);

/*
 * Unit constants
 */
#define sd_memDivCnt 1
static int sd_memDivPos[sd_memDivCnt] = { 28 };
static TXLBX_PROPERTIES sd_txProps = { sd_memDivCnt, sd_memDivPos };

/*
 * Unit procedures
 */
static BOOL CALLBACK sd_DlgProc(HWND storeDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void sd_OnCommand(HWND storeDlg, UINT cmdID, HWND ctrl, UINT notify);
static void sd_OnDrawItem(HWND storeDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL sd_OnInitDialog(HWND storeDlg, HWND focusCtrl, LPARAM lParam);

/*
 * Unit variables
 */
static HWND sd_parentWnd;
static _TUCHAR sd_srcItemName[11];
static SINDEX sd_itemToStore;
static int *sd_destIndex;  /* index of the selected item if the user hits OK */
static HWND sd_nameLcd;
static HWND sd_lbx[2];


/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL StoreDlg_Create(HWND parentWnd, SINDEX itemToStore, int *destIndex)
{
    sd_parentWnd = parentWnd;
    Snapshot_FormatName(&Prog_snapshot, itemToStore, NF_NAME, sd_srcItemName);
    sd_itemToStore = itemToStore;
    sd_destIndex = destIndex;

    return DialogBoxParam(Prog_instance, (LPCTSTR) IDD_STOREDLG, parentWnd
            , sd_DlgProc, 0) == IDOK;
}

/*
 * DlgProc()
 */
BOOL CALLBACK sd_DlgProc(HWND storeDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(storeDlg, WM_COMMAND, sd_OnCommand);
        HANDLE_MSG(storeDlg, WM_DRAWITEM, TxLbx_OnDrawItem);
        HANDLE_MSG(storeDlg, WM_INITDIALOG, sd_OnInitDialog);
        HANDLE_MSG(storeDlg, WM_MEASUREITEM, TxLbx_OnMeasureItem);
    }
    return FALSE;
}

/*
 * OnCommand()
 */
void sd_OnCommand(HWND storeDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    int lbxIndex;
    int itemGroupIndex;
    int lbxCount;

    switch (cmdID) {
        case IDOK:
            if (sd_itemToStore == SI_VCED) {
                itemGroupIndex = SI_VMEM;
                lbxCount = 16;
            } else { /* (sd_itemToStore == SI_PCED) */
                itemGroupIndex = SI_PMEM;
                lbxCount = 12;
            }
            *sd_destIndex = ListBox_GetCurSel(sd_lbx[0]);
            if (*sd_destIndex == -1) {
                *sd_destIndex = lbxCount + ListBox_GetCurSel(sd_lbx[1]);
            }
            *sd_destIndex += itemGroupIndex;
        case IDCANCEL:
            EndDialog(storeDlg, cmdID);
            break;
        case IDC_STORE1_LBX:
            lbxIndex = 0;
            goto LBX_INDEX_SET;
        case IDC_STORE2_LBX:
            lbxIndex = 1;
LBX_INDEX_SET:
            if (notify == LBN_SETFOCUS) {
                /*
                 * Allow only one selection among both list boxes.
                 */
                ListBox_SetCurSel(sd_lbx[1 - lbxIndex], -1);
            } else if (notify == LBN_DBLCLK) {
                /*
                 * Double clicking an item is equivalent to clicking OK.
                 */
                PostMessage(storeDlg, WM_COMMAND, IDOK, 0);
            }
            break;
    }
}

/*
 * OnInitDialog()
 */
BOOL sd_OnInitDialog(HWND storeDlg, HWND focusCtrl, LPARAM lParam)
{
    int firstItem;
    int listItemCnt;
    int i, j;
    RECT clientRect;
    RECT lbxRect;
    RECT lbxPosRect;
    int lbxHeight;
    HWND okBtn;
    HWND cancelBtn;
    AREA otherCtrlArea;
    DWORD style;
    DWORD exStyle;
    BOOL loaded;

    /*
     * Set up the window.
     */
    Window_Center(storeDlg, sd_parentWnd);
    sd_nameLcd = GetDlgItem(storeDlg, IDC_NAME_LCD);
    LcdCtrl_TextInit(sd_nameLcd, TX81Z_NAME_LEN);

    /*
     * Cache control handles
     */
    okBtn = GetDlgItem(storeDlg, IDOK);
    cancelBtn = GetDlgItem(storeDlg, IDCANCEL);
    sd_lbx[0] = GetDlgItem(storeDlg, IDC_STORE1_LBX);
    sd_lbx[1] = GetDlgItem(storeDlg, IDC_STORE2_LBX);

    /*
     * Initialize the list boxes.
     */
    TxLbx_Init(sd_lbx[0], &sd_txProps);
    TxLbx_Init(sd_lbx[1], &sd_txProps);

    /*
     * Initialize control settings.
     */
    LcdCtrl_SetText(sd_nameLcd, sd_srcItemName);
    if (sd_itemToStore == SI_VCED) {
        firstItem = SI_VMEM;
        listItemCnt = 16;
    } else { /* sd_itemToStore == SI_PCED */
        firstItem = SI_PMEM;
        listItemCnt = 12;
    }
    /*
     * Resize list boxes to fit their items exactly.
     */
    GetClientRect(sd_lbx[0], &lbxRect);
    lbxRect.bottom = listItemCnt * 13;
    style = (DWORD) GetWindowLong(sd_lbx[0], GWL_STYLE);
    exStyle = (DWORD) GetWindowLong(sd_lbx[0], GWL_EXSTYLE);
    AdjustWindowRectEx(&lbxRect, style, FALSE, exStyle);
    lbxHeight = RECT_H(lbxRect);
    Window_GetParentRelativeRect(sd_lbx[0], storeDlg, &lbxPosRect);
#define SPACING 6
    /*
     * Move the OK button.
     */
    Window_GetParentRelativeArea(okBtn, storeDlg, &otherCtrlArea);
    SetWindowPos(okBtn, NULL, otherCtrlArea.x
            , lbxPosRect.top + lbxHeight + SPACING, 0, 0
            , SWP_NOSIZE | SWP_NOZORDER);
    /*
     * Move the cancel button.
     */
    Window_GetParentRelativeArea(cancelBtn, storeDlg, &otherCtrlArea);
    otherCtrlArea.y = lbxPosRect.top + lbxHeight + SPACING;
    SetWindowPos(cancelBtn, NULL, otherCtrlArea.x, otherCtrlArea.y, 0, 0
            , SWP_NOSIZE | SWP_NOZORDER);
    /*
     * Set the dialog position.
     */
    GetClientRect(storeDlg, &clientRect);
    style = (DWORD) GetWindowLong(storeDlg, GWL_STYLE);
    exStyle = (DWORD) GetWindowLong(storeDlg, GWL_EXSTYLE);
    clientRect.bottom = AREA_B(otherCtrlArea) + SPACING;
    AdjustWindowRectEx(&clientRect, style, FALSE, exStyle);
    SetWindowPos(storeDlg, NULL, 0, 0, RECT_W(clientRect), RECT_H(clientRect)
            , SWP_NOMOVE | SWP_NOZORDER);

    /*
     * For each list box.
     */
    loaded = Snapshot_IsItemGroupLoaded(&Prog_snapshot, firstItem);
    for (j = 0; j < 2; j++) {
        /*
         * Size the list box vertically to fit the number of items.
         */
        SetWindowPos(sd_lbx[j], NULL, 0, 0, RECT_W(lbxRect), lbxHeight
                , SWP_NOMOVE | SWP_NOZORDER);
        /*
         * Add the list items.
         */
        for (i = 0; i < listItemCnt; i++) {
            _TUCHAR text[40];
            int index = i + j * listItemCnt;
            TXLBX_ATTR itemAttr;

            Snapshot_FormatName(&Prog_snapshot, firstItem + index
                    , NF_NUMBER_AND_NAME, text);
            if (Snapshot_IsItemDirty(&Prog_snapshot, firstItem + index)) {
                itemAttr = TA_DIRTY;
            } else if (loaded) {
                itemAttr = TA_NORMAL;
            } else {
                itemAttr = TA_UNLOADED;
            }
            TxLbx_AddItem(sd_lbx[j], text, 0, itemAttr);
        }
    }

    return TRUE;
}

