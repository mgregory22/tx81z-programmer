/*
 * exportdlg.c - Export dialog template module
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
#include "tx81z.h"
#include "txlib.h"
#include "dlg/exportdlg.h"

/*
 * Global procedures
 */
extern BOOL ExportDlg_Create(HWND parentWnd, TXLIB *txLib, int *libSelItems, int libSelItemCnt, int batchNumber, int batchStart, METAINDEX *exportType, _TUCHAR *initialDir, _TUCHAR *fileName, size_t fileNameLen);

/*
 * Unit constants
 */
#define LBX_COL1_X  30
const _TUCHAR *ed_exportTypeStrings[2] = {
    _T("Voices"),
    _T("Performances")
};

/*
 * Unit procedures
 */
static void ed_ChangeModes(HWND exportDlg);
static BOOL CALLBACK ed_DlgProc(HWND exportDlg, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL ed_ItemsOfTheExportTypeExist(void);
static void ed_OnCommand(HWND exportDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void ed_OnDrawItem(HWND exportDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL ed_OnInitDialog(HWND exportDlg, HWND focusCtrl, LPARAM lParam);
static void ed_OnMeasureItem(HWND exportDlg, MEASUREITEMSTRUCT *measureItem);
static LRESULT ed_OnNotify(HWND exportDlg, int ctrlID, LPNMHDR notifyInfo);
static void ed_UpdateLbx(HWND listBox);

/*
 * Unit variables
 */
static HWND ed_selectedOnlyChk;
static HWND ed_sysexHeadersChk;
static HWND ed_appendFileChk;
static HWND ed_exportLbx;
static METAINDEX ed_exportType;
static TXLIB *ed_txLib;
static int *ed_libSelItems;
static int ed_libSelItemCnt;
static int ed_batchNumber;
static int ed_batchStart;
static BOOL ed_selectedOnly;
static METAINDEX ed_savedExportType;
static int ed_savedBatchNumber;
static BOOL ed_savedAppendFile;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL ExportDlg_Create(HWND parentWnd, TXLIB *txLib, int *libSelItems
        , int libSelItemCnt, int batchNumber, int batchStart
        , METAINDEX *exportType, _TUCHAR *initialDir, _TUCHAR *fileName
        , size_t fileNameLen)
{
    OPENFILENAME ofn;
    _TUCHAR title[80];
    BOOL result;

    /*
     * Set the dialog box title, based on the item types and batch number.
     */
    _stprintf(title, _T("Export %s - Batch %d")
            , ed_exportTypeStrings[*exportType != META_VMEM], batchNumber);
    /*
     * Set up unit variables.
     */
    ed_txLib = txLib;
    ed_libSelItems = libSelItems;
    ed_libSelItemCnt = libSelItemCnt;
    ed_batchNumber = batchNumber;
    ed_batchStart = batchStart;
    ed_exportType = *exportType;
    ed_savedExportType = 0;
    ed_savedBatchNumber = 0;
    /*
     * Set up the save dialog options.
     */
    ofn.lStructSize       = sizeof ofn;
    ofn.hwndOwner         = parentWnd;
    ofn.hInstance         = Prog_instance;
    ofn.lpstrFilter       = Prog_syxFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = fileName;
    ofn.nMaxFile          = fileNameLen;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = title;
    ofn.Flags             = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_EXPLORER;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = Prog_syxExt;
    ofn.lCustData         = (LPARAM) 0;
    ofn.lpfnHook          = ed_DlgProc;
    ofn.lpTemplateName    = MAKEINTRESOURCE(IDD_EXPORTDLG);
    fileName[0] = '\0';
    /*
     * Show the file name dialog.
     */
    result = GetSaveFileName(&ofn);
    /*
     * Set the export type in case it has changed.
     */
    *exportType = ed_exportType;
    if (result) {
        FileDlg_SaveInitialDir(initialDir, fileName);
    }

    return result;
}

/*
 * ChangeModes()
 */
void ed_ChangeModes(HWND exportDlg)
{
    _TUCHAR title[80];

    if (ed_savedExportType == 0) {
        ed_savedExportType = ed_exportType;
        ed_exportType = META_PMEM;
        ed_savedBatchNumber = ed_batchNumber;
        ed_batchNumber = 1;
    } else {
        ed_exportType = ed_savedExportType;
        ed_savedExportType = 0;
        ed_batchNumber = ed_savedBatchNumber;
        ed_savedBatchNumber = 0;
    }
    _stprintf(title, _T("Export %s - Batch %d")
            , ed_exportTypeStrings[ed_exportType != META_VMEM]
            , ed_batchNumber);
    SetWindowText(GetParent(exportDlg), title);
}

/*
 * DlgProc()
 */
BOOL CALLBACK ed_DlgProc(HWND exportDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(exportDlg, WM_COMMAND, ed_OnCommand);
        HANDLE_MSG(exportDlg, WM_DRAWITEM, ed_OnDrawItem);
        HANDLE_MSG(exportDlg, WM_INITDIALOG, ed_OnInitDialog);
        HANDLE_MSG(exportDlg, WM_MEASUREITEM, ed_OnMeasureItem);
        HANDLE_MSG(exportDlg, WM_NOTIFY, ed_OnNotify);
    }
    return FALSE;
}

/*
 * ItemsOfTheExportTypeExist() - Returns true if there are items in the
 *                               selection that match the export type.
 */
BOOL ed_ItemsOfTheExportTypeExist(void)
{
    int selIndex;
    BOOL itemsOfTheExportTypeExist = FALSE;

    /*
     * If there are no items of the export type in the selection.
     */
    for (selIndex = 0; selIndex < ed_libSelItemCnt; selIndex++) {
        TXLIBITEM *item = TxLib_SetPtr(ed_txLib, ed_libSelItems[selIndex]);
        if (item->type == ed_exportType) {
            itemsOfTheExportTypeExist = TRUE;
            break;
        }
    }
    return itemsOfTheExportTypeExist;
}

/*
 * OnCommand()
 */
void ed_OnCommand(HWND exportDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    if (ctrlID == IDC_SELECTED_ONLY_CHK && notify == BN_CLICKED) {
        if (Button_IsChecked(ed_selectedOnlyChk)) {
            /*
             * Repopulate the export list box with selected items.
             */
            ed_selectedOnly = TRUE;
            if (!ed_ItemsOfTheExportTypeExist()) {
                /*
                 * Change the dialog to performance mode and try again.
                 */
                ed_ChangeModes(exportDlg);
            }
        } else {
            /*
             * Repopulate the export list box with all items.
             */
            ed_selectedOnly = FALSE;
            if (ed_savedExportType != 0) {
                /*
                 * Change the dialog back to the previous mode.
                 */
                ed_ChangeModes(exportDlg);
            }
        }
        ed_UpdateLbx(ed_exportLbx);
    } else if (ctrlID == IDC_SYSEX_HEADERS_CHK && notify == BN_CLICKED) {
        if (Button_IsChecked(ed_sysexHeadersChk)) {
            EnableWindow(ed_appendFileChk, TRUE);
            if (ed_savedAppendFile) {
                Button_Check(ed_appendFileChk);
                ed_savedAppendFile = FALSE;
            }
        } else {
            ed_savedAppendFile = Button_IsChecked(ed_appendFileChk);
            Button_Uncheck(ed_appendFileChk);
            EnableWindow(ed_appendFileChk, FALSE);
        }
    }
}

/*
 * OnDrawItem()
 */
void ed_OnDrawItem(HWND exportDlg, const DRAWITEMSTRUCT *drawItem)
{
    if (drawItem->CtlType == ODT_LISTBOX) {
        HDC dC = drawItem->hDC;
        RECT itemRect = drawItem->rcItem;
        HWND listBox = drawItem->hwndItem;
        int itemNumX;
        int itemTextY;
        int itemNameX;
        int itemIndex = drawItem->itemID;
        _TUCHAR text[80];
        size_t textLen;

        if (drawItem->itemAction == ODA_FOCUS) {
            /*
             * Draw focus rectangle.
             */
            DrawFocusRect(dC, &itemRect);
        } else {
            /*
             * Draw the item's background.
             */
            FillRect(dC, &itemRect, Prog_dlgBrush);

            /*
             * Adjust column rectangles.
             */
            itemNumX = itemRect.left + 2;
            itemTextY = itemRect.top + 3;
            itemNameX = itemNumX + LBX_COL1_X + 3;

            if (itemIndex > -1) {
                /*
                 * Draw item numbers based on data type (voice/performance/other).
                 */
                const _TUCHAR *typeStr;

                /*
                 * Scan for selected items if Selected Only is checked.
                 */
                if (ed_selectedOnly) {
                    if (ed_batchStart + itemIndex < ed_libSelItemCnt) {
                        TxLib_SetPtr(ed_txLib
                                , ed_libSelItems[ed_batchStart + itemIndex]);
                        /*
                    } else {
                         * If there are no patches left to display, leave the
                         * pointer at the last valid item and use that item's
                         * type to display the "INIT VOICE" items.
                         */
                    }
                } else {
                    /*
                     * If there are patches left to display in the batch, set
                     * the library pointer to the next patch.
                     */
                    if (ed_batchStart + itemIndex < ed_txLib->count) {
                        TxLib_SetPtr(ed_txLib, ed_batchStart + itemIndex);
                        /*
                    } else {
                         * See comment above.
                         */
                    }
                }
                if (ed_exportType == META_VMEM) {
                    typeStr = Prog_vMemStr;
                } else /* if (ed_exportType == META_PMEM) */ {
                    typeStr = Prog_pMemStr;
                }
                MiniFont_DrawString(dC, itemNumX, itemTextY, typeStr
                        , _tcslen(typeStr), Prog_wndTextColor);
                /*
                 * Draw item name.
                 */
                textLen = ListBox_GetTextLen(listBox, itemIndex);
                if (textLen) {
                    ListBox_GetText(listBox, itemIndex, text);
                    MiniFont_DrawString(dC, itemNameX, itemTextY, text
                            , textLen, Prog_wndTextColor);
                }
                /*
                 * Draw column separator.
                 */
                MoveToEx(dC, LBX_COL1_X, itemRect.top, NULL);
                LineTo(dC, LBX_COL1_X, itemRect.bottom);
            }
        }
    }
}

/*
 * OnInitDialog()
 */
BOOL ed_OnInitDialog(HWND exportDlg, HWND focusCtrl, LPARAM lParam)
{
    HWND saveDlg = GetParent(exportDlg);

    /*
     * Cache controls handles.
     */
    ed_selectedOnlyChk = GetDlgItem(exportDlg, IDC_SELECTED_ONLY_CHK);
    ed_sysexHeadersChk = GetDlgItem(exportDlg, IDC_SYSEX_HEADERS_CHK);
    ed_appendFileChk = GetDlgItem(exportDlg, IDC_APPEND_FILE_CHK);
    ed_exportLbx = GetDlgItem(exportDlg, IDC_EXPORT_LBX);
    /*
     * Set the "Save" button text.
     */
    CommDlg_OpenSave_SetControlText(saveDlg, IDOK, _T("Export"));
    /*
     * Initialize the check boxes.
     */
    if (ed_libSelItemCnt > 0) {
        if (Prog_exportOptions & EO_SELECTED_ONLY) {
            Button_Check(ed_selectedOnlyChk);
            ed_selectedOnly = TRUE;
            if (!ed_ItemsOfTheExportTypeExist()) {
                if (ed_exportType == META_VMEM) {
                    ed_ChangeModes(exportDlg);
                } else {
                    EndDialog(saveDlg, FALSE);
                }
            }
        } else {
            ed_selectedOnly = FALSE;
        }
    } else {
        EnableWindow(ed_selectedOnlyChk, FALSE);
    }
    if (ed_batchNumber > 1) {
        EnableWindow(ed_selectedOnlyChk, FALSE);
    }
    if (Prog_exportOptions & EO_SYSEX_HEADERS) {
        Button_Check(ed_sysexHeadersChk);
    }
    if (Prog_exportOptions & EO_APPEND_FILE) {
        Button_Check(ed_appendFileChk);
    }
    ed_UpdateLbx(ed_exportLbx);

    return TRUE;
}

/*
 * OnMeasureItem()
 */
void ed_OnMeasureItem(HWND exportDlg, MEASUREITEMSTRUCT *measureItem)
{
    if (measureItem->CtlType == ODT_LISTBOX) {
        /* assumes the minifont is being used for the item font */
        measureItem->itemHeight = 13;
    }
}

LRESULT ed_OnNotify(HWND exportDlg, int ctrlID, LPNMHDR notifyInfo)
{
    /*
     * Center the dialog in the parent window before it's displayed.
     */
    if (notifyInfo->code == CDN_INITDONE) {
        Window_CenterInParent(GetParent(exportDlg));
    /*
     * If the user pressed the OK button.
     */
    } else if (notifyInfo->code == CDN_FILEOK) {
        /*
         * Save the state of the check boxes in the Prog_exportOptions bitmap.
         */
        Prog_exportOptions = 0;
        if (Button_IsChecked(ed_selectedOnlyChk)) {
            Prog_exportOptions |= EO_SELECTED_ONLY;
        }
        if (Button_IsChecked(ed_sysexHeadersChk)) {
            Prog_exportOptions |= EO_SYSEX_HEADERS;
        }
        if (Button_IsChecked(ed_appendFileChk)) {
            Prog_exportOptions |= EO_APPEND_FILE;
        } else {
            OFNOTIFY *ofNotifyInfo = (OFNOTIFY *) notifyInfo;

            /*
             * Ask the user if he wants to replace the file if it already
             * exists.
             */
            if (File_Exists(ofNotifyInfo->lpOFN->lpstrFile)) {
                _TUCHAR msg[_MAX_PATH + 80];
                HWND saveDlg = GetParent(exportDlg);

                _stprintf(msg, _T("%s already exists.\r\n")
                        _T("Do you want to replace it?")
                        , ofNotifyInfo->lpOFN->lpstrFile);
                if (MessageBox(saveDlg, msg, ofNotifyInfo->lpOFN->lpstrTitle
                            , MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2)
                        == IDNO)
                {
                    SetWindowLong(exportDlg, DWL_MSGRESULT, 1);
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
 * UpdateLbx() - Populates the "Items to be exported" list box.  Returns TRUE
 *               if the export list is empty.
 */
void ed_UpdateLbx(HWND listBox)
{
    int i;
    int itemIndex;
    int libItemIndex;
    int batchIndex = 0;
    _TUCHAR text[40];
    TXLIBITEM *item;

    /*
     * Clear the list box.
     */
    ListBox_ResetContent(listBox);
    /*
     * Initialize the lib item index and batch index.
     */
    if (ed_selectedOnly) {
        batchIndex = -1;
        do {
            batchIndex++;
            libItemIndex = ed_libSelItems[batchIndex];
        } while (libItemIndex < ed_batchStart);
    } else {
        libItemIndex = ed_batchStart;
    }
    item = TxLib_SetPtr(ed_txLib, libItemIndex);
    /*
     * Make sure the item is the right type.
     */
    while (item && item->type != ed_exportType) {
        /*
         * Advance the lib item index and batch index.
         */
        batchIndex++;
        if (ed_selectedOnly) {
            libItemIndex = ed_libSelItems[batchIndex];
        } else {
            libItemIndex = ed_batchStart + batchIndex;
        }
        /*
         * Set the library's item pointer.
         */
        item = TxLib_SetPtr(ed_txLib, libItemIndex);
    }
    /*
     * For each slot to populate in the export list box.
     */
    for (i = 0; i < 32; i++) {
        /*
         * If there are item left to display in the export batch, get the next
         * item's name.
         */
        if (item) {
            TxLib_GetItemName(ed_txLib, text);
        /*
         * Else if voices are being exported, set the name to INIT VOICE.
         */
        } else if (ed_exportType == META_VMEM) {
            _tcscpy(text, _T("INIT VOICE"));
        /*
         * Else performances are being exported, set the name to SINGLE.
         * TODO: That reminds me, if both VMEMs and PMEMs are being exported,
         *       there needs to be a way to separate them!
         */
        } else {
            _tcscpy(text, _T("SINGLE"));
        }
        /*
         * Add the name to the list box.
         */
        itemIndex = ListBox_AddString(listBox, text);
        /*
         * Set the item data to the library item index.
         */
        ListBox_SetItemData(listBox, itemIndex, libItemIndex);
        /*
         * Advance to the next item in the batch.
         */
        do {
            batchIndex++;
            /*
             * If there are more batch items to display, advance the library
             * pointer and get the next item's library index.
             */
            if ((ed_selectedOnly && batchIndex < ed_libSelItemCnt)
                    || (!ed_selectedOnly
                        && ed_batchStart + batchIndex < ed_txLib->count))
            {
                libItemIndex = ed_selectedOnly
                    ? ed_libSelItems[batchIndex]
                    : ed_batchStart + batchIndex;
                item = TxLib_SetPtr(ed_txLib, libItemIndex);
            /*
             * Else set a sentinel value.
             */
            } else {
                libItemIndex = -1;
                item = NULL;
            }
        } while (item && item->type != ed_exportType);
    }
}

