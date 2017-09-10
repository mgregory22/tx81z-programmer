/*
 * bndldlg.c - Unbundle dialog
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
#include "dlg/bndldlg.h"

/*
 * Global procedures
 */
extern BOOL BndlDlg_Create(HWND parentWnd, BYTE *bndlBuf);

/*
 * Unit constants
 */
static const _TUCHAR bd_placeBtnText[] = _T("Place");
#define bd_placeBtnTextLen 5
static const _TUCHAR bd_unplaceBtnText[] = _T("Unplace");
#define bd_unplaceBtnTextLen 7
static int bd_bundleLbxDivs = 28;
TXLBX_PROPERTIES bd_bundleLbxProps = { 1, &bd_bundleLbxDivs };

/*
 * Unit procedures
 */
static size_t bd_CalcBndlSize(BNDL_FORMAT bndlFmt);
static void bd_CopyIntoSnapshot(void);
static BOOL CALLBACK bd_DlgProc(HWND bndlDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void bd_InitControlValues(HWND bndlDlg);
static void bd_OnCommand(HWND bndlDlg, UINT cmdID, HWND ctrlID, UINT notify);
static void bd_OnDrawItem(HWND bndlDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL bd_OnInitDialog(HWND bndlDlg, HWND focusCtrl, LPARAM lParam);
static void bd_OnSize(HWND bndlDlg, UINT state, int cx, int cy);
static void bd_OnSizing(HWND bndlDlg, int edge, RECT *rect);
static void bd_PlaceItem(void);
static void bd_RemovePlacement(int bndlItemIdx);


/*
 * Unit variables
 */
static HWND bd_parentWnd;
static SIZE bd_minWnd;
static BYTE bd_bndlBuf[S_MAX_BNDL_SIZE];
static HWND bd_snapshotLbl;
static HWND bd_bundleLbx;
static HWND bd_snapshotLbx;
static HWND bd_placeBtn;
static HWND bd_unplaceBtn;
static HWND bd_okBtn;
static HWND bd_cancelBtn;
static AREA bd_snapshotLblArea;
static AREA bd_bundleLbxArea;
static AREA bd_snapshotLbxArea;
static AREA bd_placeBtnArea;
static AREA bd_okBtnArea;
static AREA bd_cancelBtnArea;
static int bd_lbxBtmYDiff;
static int bd_okBtnXDiff;
static int bd_cancelBtnXDiff;
static int bd_btnYDiff;
#define bd_maxPlacements 11
static int bd_placements[bd_maxPlacements];  /* The snapshot locations of the
                                                placed items.  A location of
                                                -1 means the item has not been
                                                placed. */
static int bd_placementCnt;
static _TUCHAR bd_bndlItemTextBackups[bd_maxPlacements][40];
static DWORD bd_bndlItemDataBackups[bd_maxPlacements];
static _TUCHAR bd_ssItemTextBackups[bd_maxPlacements][40];
static DWORD bd_ssItemDataBackups[bd_maxPlacements];


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL BndlDlg_Create(HWND parentWnd, BYTE *bndlBuf)
{
    BNDL_FORMAT bndlFmt = *(BNDL_FORMAT *) bndlBuf;

    bd_parentWnd = parentWnd;
    memcpy(bd_bndlBuf, bndlBuf, bd_CalcBndlSize(bndlFmt));

    return DialogBox(Prog_instance, (LPCTSTR) IDD_BNDLDLG, parentWnd
            , bd_DlgProc) == IDOK;
}

/*
 * CalcBndlSize()
 */
size_t bd_CalcBndlSize(BNDL_FORMAT bndlFmt)
{
    size_t size = sizeof(BNDL_FORMAT) + S_PMEM_DATA_SIZE;

    size += ((bndlFmt & BF_VOICE_MASK) + 1) * S_VMEM_DATA_SIZE;
    if (bndlFmt & BF_FX) {
        size += S_FX_DATA_SIZE;
    }
    if (bndlFmt & BF_MTO) {
        size += S_MTO_DATA_SIZE;
    }
    if (bndlFmt & BF_MTF) {
        size += S_MTF_DATA_SIZE;
    }
    return size;
}

/*
 * CopyIntoSnapshot() - Copies the placed bundle items into the snapshot.
 */
void bd_CopyIntoSnapshot(void)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    BNDL_FORMAT bndlFmt = *(BNDL_FORMAT *) bd_bndlBuf;
    BYTE *pfm = &bd_bndlBuf[sizeof bndlFmt];
    BYTE *next = &pfm[S_PMEM_DATA_SIZE];
    int voiceCnt;
    int pIdx;
    int i, j;
    SINDEX sIdx;
    int pfmVoices[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int pfmVoiceCnt = 0;
    int voiceNum;

    /*
     * Scan the performance instruments for the original voice numbers.
     */
    for (i = 0; i < 8; i++) {
        /*
         * Get the instrument's voice number.
         */
        voiceNum = pfm[i * 8 + 1] + ((pfm[i * 8] & 0x10) << 3);
        /*
         * Scan through the previously recorded voices to see if there's a
         * match.
         */
        for (j = 0; j < pfmVoiceCnt; j++) {
            if (pfmVoices[j] == voiceNum) {
                goto VoiceAlreadyRecorded;
            }
        }
        pfmVoices[pfmVoiceCnt++] = voiceNum;
VoiceAlreadyRecorded:
        ;
    }
    /*
     * Place the voices and adjust the voice numbers in the performance to
     * match.
     */
    voiceCnt = (bndlFmt & BF_VOICE_MASK) + 1;
    pIdx = 1; /* the performance is bd_placements[0], the voices start at 1 */
    for (i = 0; i < voiceCnt; i++) {
        if (bd_placements[pIdx] > -1) {
            /*
             * Copy the voice.
             */
            sIdx = TxLbx_GetItemData(bd_snapshotLbx, bd_placements[pIdx]);
            memcpy(snapshot->vmem.data[sIdx - SI_VMEM], next, S_VMEM_DATA_SIZE);
            Snapshot_DirtyItem(snapshot, sIdx);
            Snapshot_SetItemGroupLoaded(snapshot, sIdx);
        }
        next += S_VMEM_DATA_SIZE;
        pIdx++;
    }
    /*
     * Scan through the performance voices and update placed to their new
     * voice number.
     */
    for (i = 0; i < 8; i++) {
        voiceNum = pfm[i * 8 + 1] + ((pfm[i * 8] & 0x10) << 3);
        for (j = 1; j < pIdx; j++) {
            if (bd_placements[j] > -1) {
                if (voiceNum == pfmVoices[j - 1]) {
                    int newVoice = bd_placements[j] - ((bndlFmt & BF_FX) != 0)
                         - ((bndlFmt & BF_MTO) != 0) - ((bndlFmt & BF_MTF) != 0);

                    pfm[i * 8 + 1] = newVoice & 0x7F;
                    pfm[i * 8] &= ~0x10;
                    pfm[i * 8] |= (newVoice & 0x80) >> 3;
                }
            }
        }
    }
    /*
     * Place the performance.
     */
    if (bd_placements[0] > -1) {
        sIdx = TxLbx_GetItemData(bd_snapshotLbx, bd_placements[0]);
        memcpy(snapshot->pmem.data[sIdx - SI_PMEM], pfm, S_PMEM_DATA_SIZE);
        Snapshot_DirtyItem(snapshot, sIdx);
        Snapshot_SetItemGroupLoaded(snapshot, sIdx);
    }
    /*
     * Place the effects.
     */
    if (bndlFmt & BF_FX) {
        if (bd_placements[pIdx] > -1) {
            sIdx = TxLbx_GetItemData(bd_snapshotLbx, bd_placements[pIdx]);
            memcpy(snapshot->fx.data, next, S_FX_DATA_SIZE);
            Snapshot_DirtyItem(snapshot, sIdx);
            Snapshot_SetItemGroupLoaded(snapshot, sIdx);
        }
        next += S_FX_DATA_SIZE;
        pIdx++;
    }
    /*
     * Place the octave microtuning table.
     */
    if (bndlFmt & BF_MTO) {
        if (bd_placements[pIdx] > -1) {
            sIdx = TxLbx_GetItemData(bd_snapshotLbx, bd_placements[pIdx]);
            memcpy(snapshot->mto.data, next, S_MTO_DATA_SIZE);
            Snapshot_DirtyItem(snapshot, sIdx);
            Snapshot_SetItemGroupLoaded(snapshot, sIdx);
        }
        next += S_MTO_DATA_SIZE;
        pIdx++;
    }
    /*
     * Place the full microtuning table.
     */
    if (bndlFmt & BF_MTF) {
        if (bd_placements[pIdx] > -1) {
            sIdx = TxLbx_GetItemData(bd_snapshotLbx, bd_placements[pIdx]);
            memcpy(snapshot->mto.data, next, S_MTF_DATA_SIZE);
            Snapshot_DirtyItem(snapshot, sIdx);
            Snapshot_SetItemGroupLoaded(snapshot, sIdx);
        }
        next += S_MTF_DATA_SIZE;
        pIdx++;
    }
}

/*
 * DlgProc
 */
BOOL CALLBACK bd_DlgProc(HWND bndlDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(bndlDlg, WM_COMMAND, bd_OnCommand);
        HANDLE_MSG(bndlDlg, WM_DRAWITEM, bd_OnDrawItem);
        HANDLE_MSG(bndlDlg, WM_INITDIALOG, bd_OnInitDialog);
        HANDLE_MSG(bndlDlg, WM_MEASUREITEM, TxLbx_OnMeasureItem);
        HANDLE_MSG(bndlDlg, WM_SIZE, bd_OnSize);
        HANDLE_MSG(bndlDlg, WM_SIZING, bd_OnSizing);
    }

    return FALSE;
}

/*
 * InitControlValues()
 */
void bd_InitControlValues(HWND bndlDlg)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    BNDL_FORMAT bndlFmt = *((BNDL_FORMAT *) bd_bndlBuf);
    BYTE *pfm = &bd_bndlBuf[sizeof bndlFmt];
    BYTE *next = &pfm[S_PMEM_DATA_SIZE];
    const _TUCHAR *name;
    _TUCHAR typeStr[5];
    int lbxIdx;
    int voiceCnt;
    int lbxCnt;
    int bank, patch;
    int i;

    /*
     * Init the placements array.
     */
    for (i = 0; i < bd_maxPlacements; i++) {
        bd_placements[i] = -1;
    }
    bd_placementCnt = 1;
    /*
     * Init the bundle contents list box.
     */
    /*
     * Clear the list box.
     */
    ListBox_ResetContent(bd_bundleLbx);
    /*
     * Add the performance to the list box.
     */
    if (Snapshot_IsItemGroupLoaded(snapshot, META_PMEM)) {
        for (patch = 0; patch < 24; patch++) {
            if (memcmp(snapshot->pmem.data[0] + patch * S_PMEM_DATA_SIZE, pfm
                        , S_PMEM_DATA_SIZE) == 0)
            {
                _sntprintf(typeStr, 5, TX81Z_meta[META_PMEM].typeStr
                        , patch + 1);
                lbxIdx = TxLbx_AddSnapshotItem(bd_bundleLbx, typeStr
                        , &pfm[TX81Z_PMEM_NAME], META_PMEM);
                TxLbx_SetItemAttr(bd_bundleLbx, lbxIdx, TA_UNLOADED);
                goto AddVoices;
            }
        }
    }
    TxLbx_AddSnapshotItem(bd_bundleLbx, Prog_pMemStr, &pfm[TX81Z_PMEM_NAME]
            , META_PMEM);
AddVoices:
    /*
     * Find the number of voices and add each one to the list box.
     */
    voiceCnt = (bndlFmt & BF_VOICE_MASK) + 1;
    for (i = 0; i < voiceCnt; i++) {
        METAINDEX banks[5] = {
            META_VMEM,
            META_PRESET_A,
            META_PRESET_B,
            META_PRESET_C,
            META_PRESET_D
        };

        /*
         * Search the loaded banks to see if the voice already exists in
         * the snapshot.  If so, paint the item with gray text.
         */
        for (bank = 0; bank < 5; bank++) {
            METAINDEX bankIdx = banks[bank];

            if (Snapshot_IsItemGroupLoaded(snapshot, bankIdx)) {
                for (patch = 0; patch < 32; patch++) {
                    if (memcmp(TX81Z_meta[bankIdx].dataPtr
                                + patch * S_VMEM_DATA_SIZE, next
                                , S_VMEM_DATA_SIZE) == 0)
                    {
                        _TUCHAR typeStr[5];
                        int lbxIdx;

                        _sntprintf(typeStr, 5, TX81Z_meta[bankIdx].typeStr
                                , patch + 1);
                        lbxIdx = TxLbx_AddSnapshotItem(bd_bundleLbx, typeStr
                                , &next[TX81Z_VMEM_NAME], META_VMEM);
                        TxLbx_SetItemAttr(bd_bundleLbx, lbxIdx, TA_UNLOADED);
                        goto NextVoice;
                    }
                }
            }
        }
        TxLbx_AddSnapshotItem(bd_bundleLbx, Prog_vMemStr
                , &next[TX81Z_VMEM_NAME], META_VMEM);
NextVoice:
        next += S_VMEM_DATA_SIZE;
        bd_placementCnt++;
    }
    /*
     * If the bundle has effects settings, add them to the list.
     */
    if (bndlFmt & BF_FX) {
        lbxIdx = TxLbx_AddNonbankItem(bd_bundleLbx, TX81Z_meta[META_FX].typeStr
                , TX81Z_meta[META_FX].nameFmt, META_FX);
        if (Snapshot_IsItemGroupLoaded(snapshot, META_FX)) {
            if (memcmp(snapshot->fx.data, next, S_FX_DATA_SIZE) == 0) {
                TxLbx_SetItemAttr(bd_bundleLbx, lbxIdx, TA_UNLOADED);
            }
        }
        next += S_FX_DATA_SIZE;
        bd_placementCnt++;
    }
    /*
     * If the bundle has micro tune octave table, add it to the list.
     */
    if (bndlFmt & BF_MTO) {
        TxLbx_AddNonbankItem(bd_bundleLbx, TX81Z_meta[META_MTO].typeStr
                , TX81Z_meta[META_MTO].nameFmt, META_MTO);
        if (Snapshot_IsItemGroupLoaded(snapshot, META_MTO)) {
            if (memcmp(snapshot->mto.data, next, S_MTO_DATA_SIZE) == 0) {
                TxLbx_SetItemAttr(bd_bundleLbx, lbxIdx, TA_UNLOADED);
            }
        }
        next += S_MTO_DATA_SIZE;
        bd_placementCnt++;
    }
    /*
     * If the bundle has micro tune full table, add it to the list.
     */
    if (bndlFmt & BF_MTF) {
        TxLbx_AddNonbankItem(bd_bundleLbx, TX81Z_meta[META_MTF].typeStr
                , TX81Z_meta[META_MTF].nameFmt, META_MTF);
        if (Snapshot_IsItemGroupLoaded(snapshot, META_MTF)) {
            if (memcmp(snapshot->mtf.data, next, S_MTF_DATA_SIZE) == 0) {
                TxLbx_SetItemAttr(bd_bundleLbx, lbxIdx, TA_UNLOADED);
            }
        }
        next += S_MTF_DATA_SIZE;
        bd_placementCnt++;
    }

    /*
     * Init the snapshot list box.
     */
    /*
     * Clear the list box.
     */
    ListBox_ResetContent(bd_snapshotLbx);
    /*
     * Add the effect settings item to the list box if the bundle contains
     * them.
     */
    if (bndlFmt & BF_FX) {
        name = Snapshot_IsItemGroupLoaded(snapshot, META_FX)
            ? TX81Z_meta[META_FX].nameFmt : emptyStr;

        TxLbx_AddNonbankItem(bd_snapshotLbx, TX81Z_meta[META_FX].typeStr
                , name, SI_FX);
    }
    /*
     * Add the micro tune octave item to the list box if the bundle contains
     * it.
     */
    if (bndlFmt & BF_MTO) {
        name = Snapshot_IsItemGroupLoaded(snapshot, META_MTO)
            ? TX81Z_meta[META_MTO].nameFmt : emptyStr;

        TxLbx_AddNonbankItem(bd_snapshotLbx, TX81Z_meta[META_MTO].typeStr
                , name, SI_MTO);
    }
    /*
     * Add the micro tune full item to the list box if the bundle contains
     * it.
     */
    if (bndlFmt & BF_MTF) {
        name = Snapshot_IsItemGroupLoaded(snapshot, META_MTF)
            ? TX81Z_meta[META_MTF].nameFmt : emptyStr;

        TxLbx_AddNonbankItem(bd_snapshotLbx, TX81Z_meta[META_MTF].typeStr
                , name, SI_MTF);
    }
    /*
     * Add the internal voice banks to the list box.
     */
    for (i = SI_VMEM; i < SI_VMEM + 32; i++) {
        _TUCHAR typeStr[5];

        _sntprintf(typeStr, 5, _T("I%02d"), i - SI_VMEM + 1);
        TxLbx_AddSnapshotItem(bd_snapshotLbx, typeStr
            , &Prog_snapshot.vmem.data[i - SI_VMEM][TX81Z_VMEM_NAME], i);
    }
    /*
     * Add the internal performance banks to the list box.
     */
    for (i = SI_PMEM; i < SI_PMEM + 24; i++) {
        _TUCHAR typeStr[5];

        _sntprintf(typeStr, 5, _T("PF%02d"), i - SI_PMEM + 1);
        TxLbx_AddSnapshotItem(bd_snapshotLbx, typeStr
            , &Prog_snapshot.pmem.data[i - SI_PMEM][TX81Z_PMEM_NAME], i);
    }
    /*
     * Set unloaded/dirty status for snapshot items.
     */
    lbxCnt = ListBox_GetCount(bd_snapshotLbx);
    for (i = 0; i < lbxCnt; i++) {
        /*
         * Get the snapshot index of the next item.
         */
        DWORD ssIdx = ListBox_GetItemData(bd_snapshotLbx, i);
        /*
         * If the snapshot item is unloaded, use the unloaded color.
         * If it's dirty, use the dirty color.
         */
        if (!Snapshot_IsItemsGroupLoaded(snapshot, ssIdx)) {
            TxLbx_SetItemAttr(bd_snapshotLbx, i, TA_UNLOADED);
        } else if (Snapshot_IsItemDirty(snapshot, ssIdx)) {
            TxLbx_SetItemAttr(bd_snapshotLbx, i, TA_DIRTY);
        }
    }
    /*
     * Disable the Unbundle button.
     */
    EnableWindow(bd_placeBtn, FALSE);
}

/*
 * OnCommand()
 */
void bd_OnCommand(HWND bndlDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    switch (cmdID) {
        case IDOK:
            bd_CopyIntoSnapshot();
        case IDCANCEL:
            EndDialog(bndlDlg, cmdID);
            break;
        /*
         * Controls
         */
        case IDC_PLACE_BTN:
            bd_PlaceItem();
            break;
        case IDC_UNPLACE_BTN:
            bd_RemovePlacement(ListBox_GetCurSel(bd_bundleLbx));
            break;
        case IDC_BUNDLE_LBX:
        case IDC_SNAPSHOT_LBX:
            if (notify == LBN_SELCHANGE) {
                int bndlItemIdx = ListBox_GetCurSel(bd_bundleLbx);
                int ssItemIdx = ListBox_GetCurSel(bd_snapshotLbx);
                METAINDEX bndlItemType = TxLbx_GetItemData(bd_bundleLbx
                        , bndlItemIdx);
                SINDEX snapshotIdx = TxLbx_GetItemData(bd_snapshotLbx
                        , ssItemIdx);
                METAINDEX snapshotItemType = Snapshot_GetItemType(snapshotIdx);

                /**
                 ** Swap the place and unplace buttons.
                 **/
                /*
                 * If the user has selected the two sides of a placed item.
                 */
                if (bndlItemIdx > -1 && ssItemIdx > -1
                        && bd_placements[bndlItemIdx] == ssItemIdx)
                {
                    ShowWindow(bd_placeBtn, SW_HIDE);
                    ShowWindow(bd_unplaceBtn, SW_SHOW);
                } else {
                    ShowWindow(bd_placeBtn, SW_SHOW);
                    ShowWindow(bd_unplaceBtn, SW_HIDE);
                    /*
                     * Enable/disable the Place button.
                     */
                    EnableWindow(bd_placeBtn
                            , bndlItemType == snapshotItemType);
                }
                break;
            }
    }
}

/*
 * OnDrawItem()
 */
void bd_OnDrawItem(HWND bndlDlg, const DRAWITEMSTRUCT *drawItem)
{
    if (drawItem->CtlType == ODT_LISTBOX) {
        TxLbx_OnDrawItem(bndlDlg, drawItem);
    } else if (drawItem->CtlType == ODT_BUTTON) {
        HDC dC = drawItem->hDC;
        HDC memDC = CreateCompatibleDC(dC);
        BOOL disabled = drawItem->itemState & ODS_DISABLED;
        int leftOffset = 2;
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
        if (drawItem->CtlID == IDC_PLACE_BTN) {
            SelectBitmap(memDC, disabled ? Prog_disabledPlaceBmp
                    : Prog_placeBmp);
            BitBlt(dC, drawItem->rcItem.left + leftOffset + 4
                    , drawItem->rcItem.top + topOffset, 24, 15
                    , memDC, 0, 0, SRCCOPY);
            if (disabled) {
                COLORREF savedColor = SetTextColor(dC, Prog_3dHighlightColor);

                SetBkMode(dC, TRANSPARENT);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35 + 1
                    , drawItem->rcItem.top + topOffset + 2, bd_placeBtnText
                    , bd_placeBtnTextLen);
                SetTextColor(dC, Prog_grayTextColor);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35
                    , drawItem->rcItem.top + topOffset + 1, bd_placeBtnText
                    , bd_placeBtnTextLen);
                SetTextColor(dC, savedColor);
            } else {
                COLORREF savedColor = SetTextColor(dC, Prog_dlgTextColor);
                TextOut(dC, drawItem->rcItem.left + leftOffset + 35
                    , drawItem->rcItem.top + topOffset + 1, bd_placeBtnText
                    , bd_placeBtnTextLen);
                SetTextColor(dC, savedColor);
            }
        } else if (drawItem->CtlID == IDC_UNPLACE_BTN) {
            SelectBitmap(memDC, Prog_unplaceBmp);
            BitBlt(dC, drawItem->rcItem.left + leftOffset + 4
                    , drawItem->rcItem.top + topOffset, 15, 15
                    , memDC, 0, 0, SRCCOPY);
            TextOut(dC, drawItem->rcItem.left + leftOffset + 25
                , drawItem->rcItem.top + topOffset + 1, bd_unplaceBtnText
                , bd_unplaceBtnTextLen);
        }
        DeleteDC(memDC);
    }
}

/*
 * OnInitDialog()
 */
BOOL bd_OnInitDialog(HWND bndlDlg, HWND focusCtrl, LPARAM lParam)
{
    RECT clientRect;
    RECT wndRect;

    /*
     * Adjust the window position.
     */
    Window_Center(bndlDlg, bd_parentWnd);
    /*
     * Get the minimum size limits for the window.
     */
    GetWindowRect(bndlDlg, &wndRect);
    bd_minWnd.cx = RECT_W(wndRect);
    bd_minWnd.cy = RECT_H(wndRect);
    /*
     * Cache some control handles.
     */
    bd_snapshotLbl = GetDlgItem(bndlDlg, IDC_SNAPSHOT_LBL);
    bd_bundleLbx = GetDlgItem(bndlDlg, IDC_BUNDLE_LBX);
    bd_snapshotLbx = GetDlgItem(bndlDlg, IDC_SNAPSHOT_LBX);
    bd_placeBtn = GetDlgItem(bndlDlg, IDC_PLACE_BTN);
    bd_unplaceBtn = GetDlgItem(bndlDlg, IDC_UNPLACE_BTN);
    bd_okBtn = GetDlgItem(bndlDlg, IDOK);
    bd_cancelBtn = GetDlgItem(bndlDlg, IDCANCEL);
    /*
     * Cache control areas and positions.
     */
    Window_GetParentRelativeArea(bd_snapshotLbl, bndlDlg, &bd_snapshotLblArea);
    Window_GetParentRelativeArea(bd_bundleLbx, bndlDlg, &bd_bundleLbxArea);
    Window_GetParentRelativeArea(bd_snapshotLbx, bndlDlg, &bd_snapshotLbxArea);
    Window_GetParentRelativeArea(bd_placeBtn, bndlDlg, &bd_placeBtnArea);
    Window_GetParentRelativeArea(bd_okBtn, bndlDlg, &bd_okBtnArea);
    Window_GetParentRelativeArea(bd_cancelBtn, bndlDlg, &bd_cancelBtnArea);
    GetClientRect(bndlDlg, &clientRect);
    bd_lbxBtmYDiff = clientRect.bottom - AREA_B(bd_snapshotLbxArea);
    bd_okBtnXDiff = clientRect.right - bd_okBtnArea.x;
    bd_cancelBtnXDiff = clientRect.right - bd_cancelBtnArea.x;
    bd_btnYDiff = clientRect.bottom - bd_okBtnArea.y;
    /*
     * Subclass the list boxes so they paint up correctly.
     */
    TxLbx_Init(bd_bundleLbx, &bd_bundleLbxProps);
    TxLbx_Init(bd_snapshotLbx, &bd_bundleLbxProps);
    /*
     * Hide the Unplace button.
     */
    ShowWindow(bd_unplaceBtn, SW_HIDE);
    /*
     * Initialize dialog controls.
     */
    bd_InitControlValues(bndlDlg);
    /*
     * Display the dialog.
     */
    bd_OnSize(bndlDlg, SIZE_RESTORED, clientRect.right, clientRect.bottom);
    ShowWindow(bndlDlg, SW_SHOW);

    return TRUE;
}

/*
 * OnSize()
 */
void bd_OnSize(HWND bndlDlg, UINT state, int cx, int cy)
{
    HDWP defer;
    int lbxTop = bd_bundleLbxArea.y;

    defer = BeginDeferWindowPos(7);
    /*
     * Move the OK button.
     */
    bd_okBtnArea.x = cx - bd_okBtnXDiff;
    bd_okBtnArea.y = cy - bd_btnYDiff;
    DeferWindowPos(defer, bd_okBtn, NULL, PASS_AREA_FIELDS(bd_okBtnArea)
            , SWP_NOZORDER);
    /*
     * Move the Cancel button.
     */
    bd_cancelBtnArea.x = cx - bd_cancelBtnXDiff;
    bd_cancelBtnArea.y = cy - bd_btnYDiff;
    DeferWindowPos(defer, bd_cancelBtn, NULL
            , PASS_AREA_FIELDS(bd_cancelBtnArea), SWP_NOZORDER);
    /*
     * Move the place and unplace buttons.
     */
    bd_placeBtnArea.x = (cx - bd_placeBtnArea.w) >> 1;
    bd_placeBtnArea.y = lbxTop + (bd_bundleLbxArea.h >> 1)
        - (bd_placeBtnArea.h >> 1);
    if (bd_placeBtnArea.y < lbxTop)
        bd_placeBtnArea.y = lbxTop;
    DeferWindowPos(defer, bd_placeBtn, NULL
            , PASS_AREA_FIELDS(bd_placeBtnArea), SWP_NOZORDER);
    DeferWindowPos(defer, bd_unplaceBtn, NULL
            , PASS_AREA_FIELDS(bd_placeBtnArea), SWP_NOZORDER);
    /*
     * Resize the bundle list box.
     */
    bd_bundleLbxArea.w = bd_placeBtnArea.x - bd_bundleLbxArea.x - 2;   
    bd_bundleLbxArea.h = cy - lbxTop - bd_lbxBtmYDiff;
    DeferWindowPos(defer, bd_bundleLbx, NULL
            , PASS_AREA_FIELDS(bd_bundleLbxArea), SWP_NOZORDER);
    /*
     * Move the snapshot label.
     */
    bd_snapshotLblArea.x = AREA_R(bd_placeBtnArea) + 2;
    DeferWindowPos(defer, bd_snapshotLbl, NULL
            , PASS_AREA_FIELDS(bd_snapshotLblArea), SWP_NOZORDER);
    /*
     * Move and resize the snapshot list box.
     */
    bd_snapshotLbxArea.x = bd_snapshotLblArea.x;
    bd_snapshotLbxArea.w = cx - bd_snapshotLblArea.x;
    bd_snapshotLbxArea.h = cy - lbxTop - bd_lbxBtmYDiff;
    DeferWindowPos(defer, bd_snapshotLbx, NULL
            , PASS_AREA_FIELDS(bd_snapshotLbxArea), SWP_NOZORDER);
    /*
     * Do the move.
     */
    EndDeferWindowPos(defer);
}

/*
 * OnSizing()
 */
void bd_OnSizing(HWND bndlDlg, int edge, RECT *rect)
{
    int rectW = RECT_W(*rect);
    int rectH = RECT_H(*rect);

    /*
     * Constrain the window width.
     */
    if (rectW < bd_minWnd.cx) {
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        if (edge == WMSZ_RIGHT || edge == WMSZ_TOPRIGHT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->right = rect->left + bd_minWnd.cx;
        }
        else if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT
                || edge == WMSZ_BOTTOMLEFT)
        {
            rect->left = rect->right - bd_minWnd.cx;
        }
    }
    /*
     * Constrain the window height.
     */
    if (rectH < bd_minWnd.cy) {
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        if (edge == WMSZ_BOTTOM || edge == WMSZ_BOTTOMLEFT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->bottom = rect->top + bd_minWnd.cy;
        }
        else if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT
                || edge == WMSZ_TOPRIGHT)
        {
            rect->top = rect->bottom - bd_minWnd.cy;
        }
    }
}

/*
 * PlaceItem() - Places an item in the snapshot for copying.
 */
void bd_PlaceItem(void)
{
    int bndlItemIdx = ListBox_GetCurSel(bd_bundleLbx);
    int ssItemIdx = ListBox_GetCurSel(bd_snapshotLbx);
    _TUCHAR bndlItemText[40] = {0};
    _TUCHAR ssItemText[40] = {0};
    DWORD bndlItemData;
    DWORD ssItemData;
    int oldBndlItemIdx;

    /*
     * If the selected snapshot item already has a bundle item placed in it,
     * remove the placement.
     */
    for (oldBndlItemIdx = 0; oldBndlItemIdx < bd_maxPlacements
            ; oldBndlItemIdx++)
    {
        if (bd_placements[oldBndlItemIdx] == ssItemIdx) {
            bd_RemovePlacement(oldBndlItemIdx);
            break;
        }
    }
    /*
     * If the selected bundle item already has a placement, remove it from
     * the snapshot list box.
     */
    if (bd_placements[bndlItemIdx] > -1) {
        bd_RemovePlacement(bndlItemIdx);
    }
    bndlItemData = ListBox_GetItemData(bd_bundleLbx, bndlItemIdx);
    ssItemData = ListBox_GetItemData(bd_snapshotLbx, ssItemIdx);
    /*
     * Get the list box item strings.
     */
    ListBox_GetText(bd_bundleLbx, bndlItemIdx, bndlItemText);   
    ListBox_GetText(bd_snapshotLbx, ssItemIdx, ssItemText);   
    /*
     * Save a backup of the bundle and snapshot item information in case the
     * user changes his mind.
     */
    _tcscpy(bd_bndlItemTextBackups[bndlItemIdx], bndlItemText);
    bd_bndlItemDataBackups[bndlItemIdx] = bndlItemData;
    _tcscpy(bd_ssItemTextBackups[bndlItemIdx], ssItemText);
    bd_ssItemDataBackups[bndlItemIdx] = ssItemData;
    /*
     * Set the bundle list item's type to the snapshot item's type.
     */
    _tcsncpy(bndlItemText, ssItemText, 4);
    ListBox_DeleteString(bd_bundleLbx, bndlItemIdx);
    ListBox_InsertString(bd_bundleLbx, bndlItemIdx, bndlItemText);
    /*
     * Set the snapshot item's name to the bundle item's.
     */
    _tcsncpy(&ssItemText[5], &bndlItemText[5], 20);
    ListBox_DeleteString(bd_snapshotLbx, ssItemIdx);
    ListBox_InsertString(bd_snapshotLbx, ssItemIdx, ssItemText);
    /*
     * Paint the bundle and snapshot items "placed" color.
     */
    ListBox_SetItemData(bd_bundleLbx, bndlItemIdx, bndlItemData);
    TxLbx_SetItemAttr(bd_bundleLbx, bndlItemIdx, TA_HIGHLIGHT);
    ListBox_SetItemData(bd_snapshotLbx, ssItemIdx, ssItemData);
    TxLbx_SetItemAttr(bd_snapshotLbx, ssItemIdx, TA_HIGHLIGHT);
    /*
     * Update the placements array.
     */
    bd_placements[bndlItemIdx] = ssItemIdx;
    /*
     * Deselect the items and disable the Place button.
     */
    ListBox_SetCurSel(bd_bundleLbx, -1);
    ListBox_SetCurSel(bd_snapshotLbx, -1);
    EnableWindow(bd_placeBtn, FALSE);
}

/*
 * RemovePlacement() - Removes a placement on an item in both the bundle and
 *                     snapshot list boxes and restores its previous status
 *                     and information.
 */
void bd_RemovePlacement(int bndlItemIdx)
{
    int ssItemIdx = bd_placements[bndlItemIdx];

    /*
     * Remove placement on the bundle item.
     */
    ListBox_DeleteString(bd_bundleLbx, bndlItemIdx);
    ListBox_InsertString(bd_bundleLbx, bndlItemIdx
            , bd_bndlItemTextBackups[bndlItemIdx]);
    ListBox_SetItemData(bd_bundleLbx, bndlItemIdx
            , bd_bndlItemDataBackups[bndlItemIdx]);
    /*
     * Remove placement on the snapshot item.
     */
    ListBox_DeleteString(bd_snapshotLbx, ssItemIdx);
    ListBox_InsertString(bd_snapshotLbx, ssItemIdx
            , bd_ssItemTextBackups[bndlItemIdx]);
    ListBox_SetItemData(bd_snapshotLbx, ssItemIdx
            , bd_ssItemDataBackups[bndlItemIdx]);
    bd_placements[bndlItemIdx] = -1;
    ShowWindow(bd_placeBtn, SW_SHOW);
    ShowWindow(bd_unplaceBtn, SW_HIDE);
    EnableWindow(bd_placeBtn, FALSE);
}


