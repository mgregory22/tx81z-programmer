/*
 * pcdlg.c - Program change table editor
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
#include "ctrl/txlbx.h"
#include "undo.h"
#include "dlg/pcdlg.h"


/*
 * Global constants
 */
const _TUCHAR *PcDlg_className = _T("PcDlg");

/*
 * Global procedures
 */
extern BOOL PcDlg_Create(HWND parentWnd, PC *pc);
extern void PcDlg_Update(HWND pcDlg, PC *pc);

/*
 * Unit constants
 */
#define TEXT_LEN 40

#define SEND_WHOLE_TABLE_THRESHOLD 10
static const _TUCHAR pd_setBtnText[] = _T("Set");
#define pd_setBtnTextLen 3

#define PC_LBX_COL1_X 22
#define pd_pcDivCnt 1
static int pd_pcDivPos[pd_pcDivCnt] = { PC_LBX_COL1_X };
static TXLBX_PROPERTIES pd_pcTXProps = { pd_pcDivCnt, pd_pcDivPos };

#define MEM_LBX_COL1_X 28
#define pd_memDivCnt 1
static int pd_memDivPos[pd_memDivCnt] = { MEM_LBX_COL1_X };
static TXLBX_PROPERTIES pd_memTXProps = { pd_memDivCnt, pd_memDivPos };

/*
 * Unit procedures
 */
static BOOL CALLBACK pd_DlgProc(HWND pcDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void pd_InitControlValues(HWND pcDlg);
static void pd_InitPCTable(HWND pcDlg);
static void pd_OnActivate(HWND pcDlg, UINT state, HWND otherWnd, BOOL minimized);
static void pd_OnCommand(HWND pcDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void pd_OnDestroy(HWND pcDlg);
static void pd_OnDrawItem(HWND pcDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL pd_OnInitDialog(HWND pcDlg, HWND focusCtrl, LPARAM lParam);
static void pd_OnSize(HWND pcDlg, UINT state, int cx, int cy);
static void pd_OnSizing(HWND pcDlg, int edge, RECT *rect);
static void CALLBACK pd_Redo(HWND pcDlg, CHANGE *change);
static void pd_Set(HWND pcDlg);
static void pd_SetPCListItem(int pcNumber, int patchNumber);
static void CALLBACK pd_Undo(HWND pcDlg, CHANGE *change);


/*
 * Unit variables
 */
static HWND pd_parentWnd;
static SIZE pd_minWnd;
static BYTE *pd_data;
static HWND pd_memLbl;
static HWND pd_pcLbx;
static HWND pd_memLbx;
static HWND pd_setBtn;
static HWND pd_selectedPCStc;
static HWND pd_selectedMemNumStc;
static AREA pd_memLblArea;
static AREA pd_pcLbxArea;
static AREA pd_memLbxArea;
static AREA pd_setBtnArea;
static AREA pd_selectedPCStcArea;
static AREA pd_selectedMemNumStcArea;
static int pd_selectedPCs;
static int pd_selectedMemNums;
static BOOL pd_dirty;
static HMENU pd_menu;
static UNDO *pd_undo;
static BOOL pd_undoFlag;


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL PcDlg_Create(HWND parentWnd, PC *pc)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_pcDlg) {
        PcDlg_Update(Prog_pcDlg, pc);
        if (IsIconic(Prog_pcDlg)) {
            OpenIcon(Prog_pcDlg);
        }
        BringWindowToTop(Prog_pcDlg);
    } else {
        pd_parentWnd = parentWnd;
        pd_data = pc->data;
        Prog_pcDlg = CreateDialogParam(Prog_instance, (LPCTSTR) IDD_PCDLG
                , HWND_DESKTOP, pd_DlgProc, 0);
        if (!Prog_pcDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog.
 */
void PcDlg_Update(HWND pcDlg, PC *pc)
{
    if (pc) {
        pd_data = pc->data;
        pd_InitControlValues(pcDlg);
    }
}

/*
 * DlgProc
 */
BOOL CALLBACK pd_DlgProc(HWND pcDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(pcDlg, WM_ACTIVATE, pd_OnActivate);
        HANDLE_MSG(pcDlg, WM_COMMAND, pd_OnCommand);
        HANDLE_MSG(pcDlg, WM_DESTROY, pd_OnDestroy);
        HANDLE_MSG(pcDlg, WM_DRAWITEM, pd_OnDrawItem);
        HANDLE_MSG(pcDlg, WM_INITDIALOG, pd_OnInitDialog);
        HANDLE_MSG(pcDlg, WM_MEASUREITEM, TxLbx_OnMeasureItem);
        HANDLE_MSG(pcDlg, WM_SIZE, pd_OnSize);
        HANDLE_MSG(pcDlg, WM_SIZING, pd_OnSizing);
        case EDM_REFRESH:
            pd_InitControlValues(pcDlg);
            break;
        case EDM_SAVED:
            pd_dirty = FALSE;
            break;
    }

    return FALSE;
}

/*
 * InitControlValues()
 */
void pd_InitControlValues(HWND pcDlg)
{
    int memNum;
    _TUCHAR text[TEXT_LEN];
    int i;

    /*
     * Init the program change table list box.
     */
    /*
     * Clear the list box.
     */
    ListBox_ResetContent(pd_pcLbx);
    /*
     * Add the items to the list box.
     */
    for (i = 0; i < 256; i += 2) {
        /*
         * Get the memory number from the current program change table.
         */
        memNum = (pd_data[i] << 7) | pd_data[i + 1];

        /*
         * Get the name of the item at that memory number, if it's available.
         */
        _sntprintf(text, TEXT_LEN, _T("%3d "), (i >> 1) + 1);
        text[TEXT_LEN - 1] = '\0';
        Snapshot_FormatName(&Prog_snapshot
                , Snapshot_MemNumToSnapshotIndex(memNum), NF_DASHED, &text[4]);
        /*
         * Add the item name to the list box.
         */
        ListBox_AddString(pd_pcLbx, text);
    }

    /*
     * Init the memory number list box.
     */
    /*
     * Clear the list box.
     */
    ListBox_ResetContent(pd_memLbx);
    /*
     * Add the items to the list box.
     */
    for (i = 0; i < 184; i++) {
        /*
         * Get the memory number from the current program change table.
         */

        /*
         * Get the name of the item at that memory number, if it's available.
         */
        Snapshot_FormatName(&Prog_snapshot, Snapshot_MemNumToSnapshotIndex(i)
                , NF_NUMBER_AND_NAME, text);
        /*
         * Add the item name to the list box.
         */
        ListBox_AddString(pd_memLbx, text);
    }
    /*
     * Set the selection counts to zero.
     */
    pd_selectedPCs = pd_selectedMemNums = 0;
    Static_SetText(pd_selectedPCStc, Prog_zero);
    Static_SetText(pd_selectedMemNumStc, Prog_zero);
    /*
     * Disable the Set button.
     */
    EnableWindow(pd_setBtn, FALSE);
    pd_dirty = FALSE;
    Undo_Clear(pd_undo);
    MenuItem_Disable(pd_menu, IDM_UNDO);
    MenuItem_Disable(pd_menu, IDM_REDO);
}

/*
 * InitPCTable() - Initializes the program change table and transmits it to
 *                 the unit.
 */
void pd_InitPCTable(HWND pcDlg)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    int i;
    
    for (i = 0; i < 256; i += 2) {
        snapshot->pc.data[i] = 0;
        snapshot->pc.data[i + 1] = i >> 1;
    }
    TX81Z_SendData(Prog_midi, REQ_PC, snapshot);
    pd_InitControlValues(pcDlg);
    SendMessage(Prog_mainWnd, WM_COMMAND, MAKEWPARAM(IDD_PCDLG, EDN_CHANGE)
            , (LPARAM) pcDlg);
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void pd_OnActivate(HWND pcDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = pcDlg;
}

/*
 * OnCommand()
 */
void pd_OnCommand(HWND pcDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_RETRIEVE_PC:
            TX81Z_RetrieveData(Prog_midi, REQ_PC, &Prog_snapshot);
            pd_InitControlValues(pcDlg);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_PCDLG, EDN_CHANGE)
                    , (LPARAM) pcDlg);
            break;
        case IDM_TRANSMIT_PC:
            TX81Z_SendData(Prog_midi, REQ_PC, &Prog_snapshot);
            break;
        case IDM_INIT_PC:
            pd_InitPCTable(pcDlg);
            break;
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(pcDlg);
            return;
        case IDM_UNDO:
            Undo_Undo(pd_undo, pd_Undo, pcDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(pd_undo, pd_Redo, pcDlg);
UpdateUndoMenus:
            EnableMenuItem(pd_menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(pd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(pd_menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(pd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            return;
        case IDM_KYBDDLG:
        case IDM_REMOTEWND:
        case IDM_MAINWND:
        case IDM_VOICEDLG:
        case IDM_PFMDLG:
        case IDM_FXDLG:
        case IDM_MTODLG:
        case IDM_MTFDLG:
        case IDM_SYSDLG:
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_HELP:
            Prog_OpenHelp(pcDlg, _T("res/pctable_editor.html"));
            return;
        /*
         * Controls
         */
        case IDC_SET_BTN:
            pd_Set(pcDlg);
            return;
        case IDC_PC_LBX:
        case IDC_MEM_LBX:
            if (notify == LBN_SELCHANGE) {
                _TUCHAR text[TEXT_LEN];

                /*
                 * Update the selected count static controls.
                 */
                pd_selectedPCs = ListBox_GetSelCount(pd_pcLbx);
                _sntprintf(text, TEXT_LEN, _T("%3d"), pd_selectedPCs);
                text[TEXT_LEN - 1] = '\0';
                Static_SetText(pd_selectedPCStc, text);
                pd_selectedMemNums = ListBox_GetSelCount(pd_memLbx);
                _sntprintf(text, TEXT_LEN, _T("%3d"), pd_selectedMemNums);
                text[TEXT_LEN - 1] = '\0';
                Static_SetText(pd_selectedMemNumStc, text);
                /*
                 * Enable/disable the set button.
                 */
                EnableWindow(pd_setBtn
                        , pd_selectedPCs && pd_selectedPCs == pd_selectedMemNums);
                return;
            }
    }
}

/*
 * OnDestroy()
 */
void pd_OnDestroy(HWND pcDlg)
{
    GetWindowPlacement(pcDlg, &Prog_pcDlgPlacement);
    Undo_Destroy(pd_undo);
    Prog_pcDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnDrawItem()
 */
void pd_OnDrawItem(HWND pcDlg, const DRAWITEMSTRUCT *drawItem)
{
    if (drawItem->CtlType == ODT_LISTBOX) {
        TxLbx_OnDrawItem(pcDlg, drawItem);
    } else if (drawItem->CtlType == ODT_BUTTON) {
        HDC dC = drawItem->hDC;
        HDC memDC = CreateCompatibleDC(dC);
        BOOL disabled = drawItem->itemState & ODS_DISABLED;
        int leftOffset = 3;
        int topOffset = 4;

        if (drawItem->itemState & ODS_SELECTED) {
            DrawEdge(dC, (RECT *) &drawItem->rcItem, EDGE_SUNKEN
                    , BF_RECT | BF_MIDDLE);
            leftOffset++;
            topOffset++;
        } else {
            DrawEdge(dC, (RECT *) &drawItem->rcItem, EDGE_RAISED
                    , BF_RECT | BF_MIDDLE);
        }
        if (drawItem->CtlID == IDC_SET_BTN) {
            SelectBitmap(memDC, disabled ? Prog_disabledSetBmp
                    : Prog_setBmp);
            BitBlt(dC, drawItem->rcItem.left + leftOffset + 4
                    , drawItem->rcItem.top + topOffset, 24, 15
                    , memDC, 0, 0, SRCCOPY);
            DeleteDC(memDC);
            if (disabled) {
                COLORREF savedColor = SetTextColor(dC, Prog_3dHighlightColor);

                SetBkMode(dC, TRANSPARENT);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35 + 1
                    , drawItem->rcItem.top + topOffset + 2, pd_setBtnText
                    , pd_setBtnTextLen);
                SetTextColor(dC, Prog_3dShadowColor);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35
                    , drawItem->rcItem.top + topOffset + 1, pd_setBtnText
                    , pd_setBtnTextLen);
                SetTextColor(dC, savedColor);
            } else {
                COLORREF savedColor = SetTextColor(dC, Prog_dlgTextColor);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35
                    , drawItem->rcItem.top + topOffset + 1, pd_setBtnText
                    , pd_setBtnTextLen);
                SetTextColor(dC, savedColor);
            }
        }
    }
}

/*
 * OnInitDialog()
 */
BOOL pd_OnInitDialog(HWND pcDlg, HWND focusCtrl, LPARAM lParam)
{
    RECT wndRect;

    /*
     * Adjust and save the window position is it's never been saved.
     */
    if (IsRectEmpty(&Prog_pcDlgPlacement.rcNormalPosition)) {
        Window_Center(pcDlg, pd_parentWnd);
        GetWindowPlacement(pcDlg, &Prog_pcDlgPlacement);
    }
    /*
     * Get the minimum size limits for the window.
     */
    GetWindowRect(pcDlg, &wndRect);
    pd_minWnd.cx = RECT_W(wndRect);
    pd_minWnd.cy = RECT_H(wndRect);
    /*
     * Initialize variables.
     */
    pd_selectedPCs = 0;
    pd_selectedMemNums = 0;
    /*
     * Cache moveable control handles.
     */
    pd_memLbl = GetDlgItem(pcDlg, IDC_MEM_LBL);
    pd_pcLbx = GetDlgItem(pcDlg, IDC_PC_LBX);
    pd_memLbx = GetDlgItem(pcDlg, IDC_MEM_LBX);
    pd_setBtn = GetDlgItem(pcDlg, IDC_SET_BTN);
    pd_selectedPCStc = GetDlgItem(pcDlg, IDC_SELECTED_PCS);
    pd_selectedMemNumStc = GetDlgItem(pcDlg, IDC_SELECTED_MEMNUMS);
    /*
     * Cache moveable control areas.
     */
    Window_GetParentRelativeArea(pd_memLbl, pcDlg, &pd_memLblArea);
    Window_GetParentRelativeArea(pd_pcLbx, pcDlg, &pd_pcLbxArea);
    Window_GetParentRelativeArea(pd_memLbx, pcDlg, &pd_memLbxArea);
    Window_GetParentRelativeArea(pd_setBtn, pcDlg, &pd_setBtnArea);
    Window_GetParentRelativeArea(pd_selectedPCStc, pcDlg
            , &pd_selectedPCStcArea);
    Window_GetParentRelativeArea(pd_selectedMemNumStc, pcDlg
            , &pd_selectedMemNumStcArea);
    /*
     * Set bold font for the labels.
     */
    Prog_SetBoldFont(GetDlgItem(pcDlg, IDC_PC_LBL), 0);
    Prog_SetBoldFont(pd_memLbl, 0);
    /*
     * Initialize the list boxes.
     */
    TxLbx_Init(pd_pcLbx, &pd_pcTXProps);
    TxLbx_Init(pd_memLbx, &pd_memTXProps);
    /*
     * Set up the undo infrastructure (must be done before calling
     * InitControlValues).
     */
    pd_menu = GetMenu(pcDlg);
    pd_undo = Undo_Create();
    /*
     * Initialize control settings.
     */
    pd_InitControlValues(pcDlg);
    /*
     * Set the saved window position.
     */
#define RC Prog_pcDlgPlacement.rcNormalPosition
    MoveWindow(pcDlg, RC.left, RC.top, RECT_W(RC), RECT_H(RC), TRUE);
#undef RC
    ShowWindow(pcDlg, SW_SHOWNORMAL);

    return TRUE;
}

/*
 * OnSize()
 */
void pd_OnSize(HWND pcDlg, UINT state, int cx, int cy)
{
    HDWP defer;
    int lbxTop = pd_pcLbxArea.y;

    defer = BeginDeferWindowPos(6);
    /*
     * Move the set button.
     */
    pd_setBtnArea.x = (cx - pd_setBtnArea.w) >> 1;
    pd_setBtnArea.y = (cy - lbxTop - pd_setBtnArea.h) >> 1;
    if (pd_setBtnArea.y < lbxTop)
        pd_setBtnArea.y = lbxTop;
    DeferWindowPos(defer, pd_setBtn, NULL, PASS_AREA_FIELDS(pd_setBtnArea)
            , SWP_NOZORDER);
    /*
     * Move the memory number label.
     */
    pd_memLblArea.x = AREA_R(pd_setBtnArea) + 2;
    DeferWindowPos(defer, pd_memLbl, NULL, PASS_AREA_FIELDS(pd_memLblArea)
            , SWP_NOZORDER);
    /*
     * Resize the program change list box.
     */
    pd_pcLbxArea.w = pd_setBtnArea.x - pd_pcLbxArea.x - 2;   
    pd_pcLbxArea.h = cy - lbxTop;
    DeferWindowPos(defer, pd_pcLbx, NULL, PASS_AREA_FIELDS(pd_pcLbxArea)
            , SWP_NOZORDER);
    /*
     * Move the selected program change count static control.
     */
    pd_selectedPCStcArea.x = AREA_R(pd_pcLbxArea) - pd_selectedPCStcArea.w;
    DeferWindowPos(defer, pd_selectedPCStc, NULL
            , PASS_AREA_FIELDS(pd_selectedPCStcArea), SWP_NOZORDER);
    /*
     * Move and resize the memory number list box.
     */
    pd_memLbxArea.x = pd_memLblArea.x;
    pd_memLbxArea.w = cx - pd_memLblArea.x;
    pd_memLbxArea.h = cy - lbxTop;
    DeferWindowPos(defer, pd_memLbx, NULL, PASS_AREA_FIELDS(pd_memLbxArea)
            , SWP_NOZORDER);
    /*
     * Move the selected memory number count static control.
     */
    pd_selectedMemNumStcArea.x = AREA_R(pd_memLbxArea)
        - pd_selectedMemNumStcArea.w;
    DeferWindowPos(defer, pd_selectedMemNumStc, NULL
            , PASS_AREA_FIELDS(pd_selectedMemNumStcArea), SWP_NOZORDER);
    /*
     * Do the move.
     */
    EndDeferWindowPos(defer);
}

/*
 * OnSizing()
 */
void pd_OnSizing(HWND pcDlg, int edge, RECT *rect)
{
    int rectW = RECT_W(*rect);
    int rectH = RECT_H(*rect);

    /*
     * Constrain the window width.
     */
    if (rectW < pd_minWnd.cx) {
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        if (edge == WMSZ_RIGHT || edge == WMSZ_TOPRIGHT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->right = rect->left + pd_minWnd.cx;
        }
        else if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT
                || edge == WMSZ_BOTTOMLEFT)
        {
            rect->left = rect->right - pd_minWnd.cx;
        }
    }
    /*
     * Constrain the window height.
     */
    if (rectH < pd_minWnd.cy) {
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        if (edge == WMSZ_BOTTOM || edge == WMSZ_BOTTOMLEFT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->bottom = rect->top + pd_minWnd.cy;
        }
        else if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT
                || edge == WMSZ_TOPRIGHT)
        {
            rect->top = rect->bottom - pd_minWnd.cy;
        }
    }
}

/*
 * Redo() - Redoes a previously undone set of changes made to the program
 *          change table.
 */
void CALLBACK pd_Redo(HWND pcDlg, CHANGE *change)
{
    BYTE *newValue = change->newValue;
    size_t changeCnt = change->size >> 1;
    size_t i;

    ListBox_SetSel(pd_pcLbx, FALSE, -1);
    for (i = 0; i < changeCnt; i++) {
        int pcNumber = newValue[(i << 1) + 1];
        int patchNumber = newValue[i << 1];
        int pcBaseIndex = pcNumber << 1;

        pd_data[pcBaseIndex] = patchNumber > 0x7F;
        pd_data[pcBaseIndex + 1] = patchNumber & 0x7F;
        pd_SetPCListItem(pcNumber, patchNumber);
        if (changeCnt < SEND_WHOLE_TABLE_THRESHOLD) {
            TX81Z_SendPCParamChange(Prog_midi, pcNumber, patchNumber);
        }
    }
    if (changeCnt >= SEND_WHOLE_TABLE_THRESHOLD) {
        /*
         * Prepare the sysex message. -- The only way the header could be
         * changed is if a new PC table gets loaded, but if that happens the
         * undo list will be cleared, thus this point will never be reached,
         * so there's no point in initializing the PC table header again,
         * unless there was a change made to the transmit MIDI channel, in
         * which case the channel wasn't updated about either.
         */
        //memcpy(Prog_snapshot.pc.header, TX81Z_meta[META_PC].dumpHdr
        //        , TX81Z_meta[META_PC].dumpHdrLen);
        //Prog_snapshot.pc.footer[0] = Snapshot_Checksum(pd_data, 256);
        //Prog_snapshot.pc.footer[1] = 0xF7;
        /*
         * Send the program change table to the unit.
         */
        TX81Z_SendData(Prog_midi, REQ_PC, &Prog_snapshot);
    }
}

/*
 * Set() - Copies memory numbers from the memory number list to the program
 *         change table when the Set button is clicked.
 */
void pd_Set(HWND pcDlg)
{
    int selItemCnt = ListBox_GetSelCount(pd_memLbx);
    int *memSelItems;
    int *pcSelItems;
    int i;
    int saveTopIndex = ListBox_GetTopIndex(pd_pcLbx);
    CHANGE *change;
    BYTE *oldValue;
    BYTE *newValue;

    /*
     * Check memory protect.
     */
    if (TX81Z_IsMemoryProtectOn(Prog_midi, &Prog_snapshot)) {
        return;
    }
    /*
     * Create buffer to receive indexes of selected memory number list box
     * items.
     */
    memSelItems = malloc(selItemCnt * sizeof(int));
    if (!memSelItems) {
        Error_OnError(E_MALLOC_ERROR);
        return;
    }
    /*
     * Create buffer to receive indexes of selected program change list box
     * items.
     */
    pcSelItems = malloc(selItemCnt * sizeof(int));
    if (!pcSelItems) {
        free(memSelItems);
        Error_OnError(E_MALLOC_ERROR);
        return;
    }
    /*
     * Retrieve item selections.
     */
    ListBox_GetSelItems(pd_pcLbx, selItemCnt, pcSelItems);
    ListBox_GetSelItems(pd_memLbx, selItemCnt, memSelItems);
    /*
     * Create the undo record to get the buffers it allocates.
     */
    change = Undo_AddChange(pd_undo, IDC_PC_LBX, selItemCnt << 1, NULL, NULL
            , TRUE);
    oldValue = change->oldValue;
    newValue = change->newValue;
    /*
     * Go through all the selected items, copying them and updating the
     * program change list box.
     */
    for (i = 0; i < selItemCnt; i++) {
        int pcNumber = pcSelItems[i];
        int patchNumber = memSelItems[i]; 
        int pcBaseIndex = pcNumber << 1;  /* each pc entry is 2 bytes */

        /*
         * Copy the old and new values into the undo record.  The buffers are
         * arrays of two byte records.  Each record in each array holds the
         * patch number in the first byte of each record and the program change
         * number in the second byte.
         */
        oldValue[i << 1] = (pd_data[pcBaseIndex] << 7)
            | pd_data[pcBaseIndex + 1];
        oldValue[(i << 1) + 1] = pcNumber;
        newValue[i << 1] = patchNumber;
        newValue[(i << 1) + 1] = pcNumber;
        /*
         * Copy the memory number into the snapshot.
         */
        pd_data[pcBaseIndex] = patchNumber > 0x7F;
        pd_data[pcBaseIndex + 1] = patchNumber & 0x7F;
        pd_SetPCListItem(pcNumber, patchNumber);
        /*
         * Send the parameter change message, unless the number of changes
         * exceeds the "send whole table" threshold, in which case the whole
         * table is sent below.
         */
        if (selItemCnt < SEND_WHOLE_TABLE_THRESHOLD) {
            TX81Z_SendPCParamChange(Prog_midi, pcNumber, patchNumber);
        }
    }
    /*
     * Send the program change table if it hasn't already been sent with
     * individual parameter changes.
     */
    if (selItemCnt >= SEND_WHOLE_TABLE_THRESHOLD) {
        /*
         * Prepare the sysex message.
         */
        memcpy(Prog_snapshot.pc.header, TX81Z_meta[META_PC].dumpHdr
                , TX81Z_meta[META_PC].dumpHdrLen);
        Prog_snapshot.pc.footer[0] = Snapshot_Checksum(pd_data, 256);
        Prog_snapshot.pc.footer[1] = 0xF7;
        /*
         * Send.
         */
        TX81Z_SendData(Prog_midi, REQ_PC, &Prog_snapshot);
    }
    /*
     * Restore the program change list box to its former state.
     */
    ListBox_SetTopIndex(pd_pcLbx, saveTopIndex);
    /*
     * Clean up temps.
     */
    free(memSelItems);
    free(pcSelItems);
    /*
     * Update the undo menu.
     */
    MenuItem_Enable(pd_menu, IDM_UNDO);
    MenuItem_Disable(pd_menu, IDM_REDO);
    /*
     * Send a change notification to main window if this is the first change
     * made since opening the editor.
     */
    if (!pd_dirty) {
        SendNotification(Prog_mainWnd, IDD_PCDLG, pcDlg, EDN_CHANGE);
        pd_dirty = TRUE;
    }
}

/*
 * SetPCListItem() - Replaces an item in the program change list box.
 */
void pd_SetPCListItem(int pcNumber, int patchNumber)
{
    _TUCHAR text[TEXT_LEN];

    /*
     * Prepare the program change list box entry (program change number,
     * patch number, patch name).
     */
    _sntprintf(text, TEXT_LEN, _T("%3d "), pcNumber + 1);
    text[TEXT_LEN - 1] = '\0';
    Snapshot_FormatName(&Prog_snapshot
            , Snapshot_MemNumToSnapshotIndex(patchNumber)
            , NF_DASHED, &text[4]);
    /*
     * Replace the old item with the new.
     */
    TxLbx_ReplaceItem(pd_pcLbx, pcNumber, text, 0, TA_NORMAL);
    ListBox_SetSel(pd_pcLbx, TRUE, pcNumber);
}

/*
 * Undo() - Undoes a set of changes made to the program change table.
 */
void CALLBACK pd_Undo(HWND pcDlg, CHANGE *change)
{
    BYTE *oldValue = change->oldValue;
    size_t changeCnt = change->size >> 1;
    size_t i;

    ListBox_SetSel(pd_pcLbx, FALSE, -1);
    for (i = 0; i < changeCnt; i++) {
        int pcNumber = oldValue[(i << 1) + 1];
        int patchNumber = oldValue[i << 1];
        int pcBaseIndex = pcNumber << 1;

        pd_data[pcBaseIndex] = patchNumber > 0x7F;
        pd_data[pcBaseIndex + 1] = patchNumber & 0x7F;
        pd_SetPCListItem(pcNumber, patchNumber);
        if (changeCnt < SEND_WHOLE_TABLE_THRESHOLD) {
            TX81Z_SendPCParamChange(Prog_midi, pcNumber, patchNumber);
        }
    }
    if (changeCnt >= SEND_WHOLE_TABLE_THRESHOLD) {
        /*
         * Prepare the sysex message. -- The only way the header could be
         * changed is if a new PC table gets loaded, but if that happens the
         * undo list will be cleared, thus this point will never be reached,
         * so there's no point in initializing the PC table header again,
         * unless there was a change made to the transmit MIDI channel, in
         * which case the channel wasn't updated about either.
         */
        //memcpy(Prog_snapshot.pc.header, TX81Z_meta[META_PC].dumpHdr
        //        , TX81Z_meta[META_PC].dumpHdrLen);
        //Prog_snapshot.pc.footer[0] = Snapshot_Checksum(pd_data, 256);
        //Prog_snapshot.pc.footer[1] = 0xF7;
        /*
         * Send the program change table to the unit.
         */
        TX81Z_SendData(Prog_midi, REQ_PC, &Prog_snapshot);
    }
}

