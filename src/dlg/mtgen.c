/*
 * mtgen.c - Generic routines for micro tune octave and micro tune full dialogs.
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
#include "dlg/mtgen.h"

/*
 * Global procedures
 */
extern void MTGen_FineLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
extern void MTGen_FullLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
extern void MTGen_InitControlValues(MTDLGDATA *mtDlgData);
extern void MTGen_NoteLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
extern void MTGen_OnKey(HWND mtDlg, UINT vk, BOOL down, int repeat
        , UINT keyFlags, UINT initCnt, DIALOG *dialog);
extern void CALLBACK MTGen_Redo(HWND mtDlg, CHANGE *change);
extern void CALLBACK MTGen_Undo(HWND mtDlg, CHANGE *change);

/*
 * Unit types
 */
typedef enum {
    SLIDER_TO_UNIT,
    SLIDER_TO_VISUAL,
    UNIT_TO_SLIDER,
    UNIT_TO_VISUAL,
    VISUAL_TO_SLIDER,
    VISUAL_TO_UNIT
} FINECVTTYPE;

typedef enum {
    MTCT_NOTE,
    MTCT_FINE,
    MTCT_FULL
} MTCTRLTYPE;

/*
 * Unit procedures
 */
static int mg_CvtFineValue(int value, FINECVTTYPE conversionType);
static void mg_SendParamChange(MTDLGDATA *mtDlgData, int key, int note, int fine);
static int mg_UpdateFineSlider(HWND fineSlider, BYTE *data, int noteIdx, int note, int fine);
static void mg_UpdateFreqDisplay(HWND mtDlg, int key, BYTE *data);

/*
 * Unit variables
 */
static BOOL mg_initializing;
static BOOL mg_fineUpdate;
static BOOL mg_fullUpdate;
static BOOL mg_noteUpdate;
static BOOL mg_undoUpdate;


/*
 * Procedure definitions
 */

/*
 * CvtFineValue() - The value of the fine frequency setting can be expressed in
 *                  terms of three different areas: in terms of the dialog
 *                  slider, in terms of the visual display of the value (which
 *                  is the same both in the dialog and on the unit), and in
 *                  terms of the TX81Z's internal representation.  This
 *                  function converts between the three and serves to document
 *                  the relationships between them.  The relationships are:
 *                  VISUAL : -31 .. -1 ,  0 .. 32
 *                  SLIDER :   0 .. 30 , 31 .. 63
 *                  UNIT   :  33 .. 63 ,  0 .. 32
 */
int mg_CvtFineValue(int value, FINECVTTYPE conversionType)
{
    int result;

    switch (conversionType) {
        case SLIDER_TO_UNIT:
            result = value <= 30 ? value + 33 : value - 31;
            break;
        case SLIDER_TO_VISUAL:
            result = value - 31;
            break;
        case UNIT_TO_SLIDER:
            result = value <= 32 ? value + 31 : value - 33;
            break;
        case UNIT_TO_VISUAL:
            result = value <= 32 ? value : value - 64;
            break;
        case VISUAL_TO_SLIDER:
            result = value + 31;
            break;
        case VISUAL_TO_UNIT:
            result = value >= 0 ? value : value + 64;
            break;
    }
    return result;
}

/*
 * FineLcdChange() - LCD callback function.
 */
void MTGen_FineLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value)
{
    int noteIdx, fineIdx;
    int note, fine;
    int key;
    BYTE *data = mtDlgData->data;

    if (mg_initializing || mg_fullUpdate)
        return;

    fineIdx = ctrlID - MT_ID_OFFSET;
    noteIdx = fineIdx - 1;
    key = noteIdx >> 1;
    note = data[noteIdx];
    fine = mg_CvtFineValue(value, SLIDER_TO_UNIT);
    /*
     * The note setting on the unit can be changed by the fine slider, so adjust
     * it if the visual value of the fine slider changed sign.
     */
    if (fine > 32 && data[fineIdx] <= 32) {
        note--;
    } else if (fine <= 32 && data[fineIdx] > 32) {
        note++;
    }
    /*
     * Add the change to the undo list.
     */
    if (!mg_undoUpdate) {
        int oldValue = mg_CvtFineValue(data[fineIdx], UNIT_TO_SLIDER);
        int newValue = mg_CvtFineValue(fine, UNIT_TO_SLIDER);

        Undo_AddChange(mtDlgData->undo, ctrlID, sizeof(int), &oldValue
                , &newValue, mtDlgData->undoGroup);
        mtDlgData->undoGroup = FALSE;
        /*
         * Update the dialog's undo menu items.
         */
        MenuItem_Enable(mtDlgData->menu, IDM_UNDO);
        MenuItem_Disable(mtDlgData->menu, IDM_REDO);
    }
    /*
     * Update the synth.
     */
    mg_SendParamChange(mtDlgData, key, note, fine);
    /*
     * Update the snapshot buffer.
     */
    data[noteIdx] = note;
    data[fineIdx] = fine;
    /*
     * Update the full tuning bar.
     */
    mg_fineUpdate = TRUE;
    LcdCtrl_SetValue(GetDlgItem(mtDlgData->wnd, key + MT_FULL_OFFSET)
            , ((note - 13) << 6) + fine);
    mg_fineUpdate = FALSE;
    /*
     * Update the frequency readout.
     */
    mg_UpdateFreqDisplay(mtDlgData->wnd, key, data);
}

/*
 * FullLcdChange() - LCD callback function.
 */
void MTGen_FullLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value)
{
    HWND noteSlider, fineSlider;
    int noteIdx, fineIdx;
    int note, fine;
    int key;
    BYTE *data = mtDlgData->data;

    if (mg_initializing || mg_fineUpdate || mg_noteUpdate)
        return;

    key = ctrlID - MT_FULL_OFFSET;
    noteIdx = key << 1;
    fineIdx = noteIdx + 1;
    // C#-1   D-1
    // 0..32  33..96
    // 0      1
    note = (value >> 6) + 13;
    fine = value - ((value >> 6) << 6);
    noteSlider = GetDlgItem(mtDlgData->wnd, noteIdx + MT_ID_OFFSET);
    fineSlider = GetDlgItem(mtDlgData->wnd, fineIdx + MT_ID_OFFSET);
    /*
     * Update the fine slider range.
     */
    mg_UpdateFineSlider(fineSlider, data, noteIdx, note + (fine > 32)
            , data[fineIdx]);
    /*
     * Add the change to the undo list.
     */
    if (!mg_undoUpdate) {
        int oldValue = ((data[noteIdx] - 13) << 6) + data[fineIdx];

        Undo_AddChange(mtDlgData->undo, ctrlID, sizeof(int), &oldValue, &value
                , mtDlgData->undoGroup);
        mtDlgData->undoGroup = FALSE;
        /*
         * Update the dialog's undo menu items.
         */
        MenuItem_Enable(mtDlgData->menu, IDM_UNDO);
        MenuItem_Disable(mtDlgData->menu, IDM_REDO);
    }
    /*
     * Update the synth.
     */
    mg_SendParamChange(mtDlgData, key, note, fine);
    /*
     * Update the snapshot buffer.
     */
    data[noteIdx] = note;
    data[fineIdx] = fine;
    /*
     * Update the LCD controls.
     */
    mg_fullUpdate = TRUE;
    LcdCtrl_SetValue(noteSlider, note + (fine > 32));
    LcdCtrl_SetValue(fineSlider, mg_CvtFineValue(fine, UNIT_TO_SLIDER));
    mg_fullUpdate = FALSE;
    /*
     * Update the frequency readout.
     */
    mg_UpdateFreqDisplay(mtDlgData->wnd, key, data);
}

/*
 * InitControlValues() - 
 */
void MTGen_InitControlValues(MTDLGDATA *mtDlgData)
{
    HWND ctrl;
    int noteIdx, fineIdx;
    int note, fine, full;
    int i;
    HWND mtDlg = mtDlgData->wnd;
    BYTE *data = mtDlgData->data;
    int initCnt = mtDlgData->initCnt;

    mg_initializing = TRUE;
    /*
     * Init LCD values.
     */
    for (i = 0; i < initCnt; i++) {
        noteIdx = i << 1;
        fineIdx = noteIdx + 1;
        ctrl = GetDlgItem(mtDlg, IDC_MT_NOTE_1 + noteIdx);
        fine = data[fineIdx];
        /*
         * Bump the note up if the fine value is negative (33..63 in unit
         * terms).
         */
        note = data[noteIdx] + (fine > 32);
        /*
         * Set the note LCD.
         */
        LcdCtrl_SetValue(ctrl, note);
        /*
         * Set the fine LCD.
         */
        ctrl = GetDlgItem(mtDlg, IDC_MT_FINE_1 + noteIdx);
        if (note == 109) {
            LcdCtrl_SetRange(ctrl, 0, 30);
        } else if (note == 13) {
            LcdCtrl_SetRange(ctrl, 31, 63);
        }
        LcdCtrl_SetValue(ctrl, mg_CvtFineValue(fine, UNIT_TO_SLIDER));
        /*
         * Set the full LCD.
         */
        ctrl = GetDlgItem(mtDlg, IDC_MT_FULL_1 + i);
        full = ((note - 13) << 6) + mg_CvtFineValue(fine, UNIT_TO_VISUAL);
        LcdCtrl_SetValue(ctrl, full);
        /*
         * Update the frequency readout.
         */
        mg_UpdateFreqDisplay(mtDlg, i, data);
    }
    InvalidateRect(mtDlg, NULL, TRUE);
    mtDlgData->dirty = FALSE;
    Undo_Clear(mtDlgData->undo);
    mtDlgData->undoGroup = TRUE;
    MenuItem_Disable(mtDlgData->menu, IDM_UNDO);
    MenuItem_Disable(mtDlgData->menu, IDM_REDO);
    mg_initializing = FALSE;
}

/*
 * NoteLcdChange() - LCD callback function.
 */
void MTGen_NoteLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value)
{
    HWND mtDlg = mtDlgData->wnd;
    BYTE *data = mtDlgData->data;
    int noteIdx = ctrlID - MT_ID_OFFSET;
    int fineIdx = noteIdx + 1;
    int key = noteIdx >> 1;
    int fine = data[fineIdx];
    int note;
    HWND fineSlider = GetDlgItem(mtDlg, ctrlID + 1);

    /*
     * Update the fine slider range.
     */
    fine = mg_UpdateFineSlider(fineSlider, data, noteIdx, value, fine);
    if (mg_initializing || mg_fineUpdate || mg_fullUpdate) {
        return;
    }
    /* convert the note slider value to the unit coarse frequency setting */
    note = value - (fine > 32);
    /*
     * Add the change to the undo list.
     */
    if (!mg_undoUpdate) {
        int oldValue = data[noteIdx] + (fine > 32);

        Undo_AddChange(mtDlgData->undo, ctrlID, sizeof(int), &oldValue, &value
                , mtDlgData->undoGroup);
        mtDlgData->undoGroup = FALSE;
        /*
         * Update the dialog's undo menu items.
         */
        MenuItem_Enable(mtDlgData->menu, IDM_UNDO);
        MenuItem_Disable(mtDlgData->menu, IDM_REDO);
    }
    /*
     * Update the synth.
     */
    mg_SendParamChange(mtDlgData, key, note, fine);
    /*
     * Update the snapshot buffer.
     */
    data[fineIdx] = fine;
    data[noteIdx] = note;
    /*
     * Update the full tuning bar.
     */
    mg_noteUpdate = TRUE;
    LcdCtrl_SetValue(GetDlgItem(mtDlg, key + MT_FULL_OFFSET)
            , ((value - 13) << 6) + mg_CvtFineValue(fine, UNIT_TO_VISUAL));
    mg_noteUpdate = FALSE;
    /*
     * Update the frequency readout.
     */
    mg_UpdateFreqDisplay(mtDlg, key, data);
}

/*
 * OnKey()
 */
void MTGen_OnKey(HWND mtDlg, UINT vk, BOOL down, int repeat, UINT keyFlags
        , UINT initCnt, DIALOG *dialog)
{
    HWND focusCtrl = GetFocus();
    UINT focusID = GetDlgCtrlID(focusCtrl);
    UINT destID = 0;
    MTCTRLTYPE ctrlType;
    RECT mtDlgRect;
    UINT destGroupID;
    RECT noteRect;
    RECT fullRect;

    if (focusID == IDC_LCD_SCROLLBAR) {
        focusCtrl = GetParent(focusCtrl);
        focusID = GetDlgCtrlID(focusCtrl);
    }
    if (focusID >= IDC_MT_FULL_1) {
        ctrlType = MTCT_FULL;
    } else if (focusID % 2 == 0) {
        ctrlType = MTCT_NOTE;
    } else {
        ctrlType = MTCT_FINE;
    }

    switch (vk) {
        case VK_LEFT:
            if (ctrlType == MTCT_FULL) {
                destID = ((focusID - IDC_MT_FULL_1) << 1) + IDC_MT_NOTE_1;
            } else if (ctrlType == MTCT_FINE) {
                destID = focusID - 1;
            }
            break;
        case VK_UP:
            if (ctrlType == MTCT_FULL) {
                destID = ((focusID - IDC_MT_FULL_1) << 1)
                        + IDC_MT_FINE_1;
            } else if (ctrlType == MTCT_NOTE) {
                if (focusID > IDC_MT_FINE_1) {
                    destID = focusID - 2;
                }
            } else {
                if (focusID > IDC_MT_FINE_1) {
                    destID = ((focusID - IDC_MT_FINE_1 - 1) >> 1)
                            + IDC_MT_FULL_1;
                }
            }
            break;
        case VK_RIGHT:
            if (ctrlType == MTCT_NOTE) {
                destID = focusID + 1;
            }
            break;
        case VK_DOWN:
            if (ctrlType == MTCT_FULL) {
                if (focusID < IDC_MT_FULL_1 + initCnt - 1) {
                    destID = ((focusID - IDC_MT_FULL_1 + 1) << 1)
                            + IDC_MT_FINE_1;
                }
            } else if (ctrlType == MTCT_NOTE) {
                if (focusID < IDC_MT_NOTE_1 + ((initCnt - 1) << 1)) {
                    destID = focusID + 2;
                }
            } else {
                destID = ((focusID - IDC_MT_FINE_1 + 1) >> 1)
                        + IDC_MT_FULL_1;
            }
            break;
        default:
            return;
    }
    if (destID == 0) {
        return;
    }
    SetFocus(GetDlgItem(mtDlg, destID));
    /*
     * Scroll window so control is fully visible, if necessary.
     */
    GetClientRect(mtDlg, &mtDlgRect);
    if (destID >= IDC_MT_FULL_1) {
        destGroupID = destID - IDC_MT_FULL_1;
    } else {
        destGroupID = (destID - IDC_MT_NOTE_1) >> 1;
    }
    Window_GetParentRelativeRect(GetDlgItem(mtDlg, IDC_MT_FULL_1
                + destGroupID), mtDlg, &fullRect);
    Window_GetParentRelativeRect(GetDlgItem(mtDlg, IDC_MT_NOTE_1
                + (destGroupID << 1)), mtDlg, &noteRect);

    if (!Rect_RectIn(&mtDlgRect, &noteRect)
            || !Rect_RectIn(&mtDlgRect, &fullRect))
    {
        int dy = 0;

        if (noteRect.top < mtDlgRect.top) {
            dy = noteRect.top - mtDlgRect.top - 5;
        }
        if (fullRect.bottom > mtDlgRect.bottom) {
            dy = fullRect.bottom - mtDlgRect.bottom + 5;
        }
        if (dy) {
            Dialog_Scroll(dialog, 0, dy);
            SetScrollInfo(mtDlg, SB_VERT, &dialog->vScrollInfo, TRUE);
        }
    }
}

/*
 * Redo() - Callback for Undo_Redo().
 */
void CALLBACK MTGen_Redo(HWND mtDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(mtDlg, change->ctrlID);

    if (change->groupStart) {
        SetFocus(ctrl);
    }
    LcdCtrl_SetValue(ctrl, *(int *) change->newValue);
    mg_undoUpdate = TRUE;
    FORWARD_WM_COMMAND(mtDlg, change->ctrlID, ctrl, LCN_SELCHANGE, SendMessage);
    mg_undoUpdate = FALSE;
}

/*
 * Undo() - Callback for Undo_Undo().
 */
void CALLBACK MTGen_Undo(HWND mtDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(mtDlg, change->ctrlID);

    if (change->groupStart) {
        SetFocus(ctrl);
    }
    LcdCtrl_SetValue(ctrl, *(int *) change->oldValue);
    mg_undoUpdate = TRUE;
    FORWARD_WM_COMMAND(mtDlg, change->ctrlID, ctrl, LCN_SELCHANGE, SendMessage);
    mg_undoUpdate = FALSE;
}

/*
 * SendParamChange() - Implements rules for sending MIDI messages.
 */
void mg_SendParamChange(MTDLGDATA *mtDlgData, int key, int note, int fine)
{
#ifdef NO_REDUNDANT_SENDS
    int noteIdx = key << 1;
    int fineIdx = noteIdx + 1;

    if (note == mtDlgData->data[noteIdx] && fine == mtDlgData->data[fineIdx]) {
        return;
    }
#endif
    if (mg_initializing) {
        return;
    }
    TX81Z_SendParamChange(Prog_midi, mtDlgData->subGrp, key
            , ((note & 0x7F) << 8) | (fine & 0x3F));
    if (!mtDlgData->dirty) {
        SendNotification(Prog_mainWnd, mtDlgData->ctrlID, mtDlgData->wnd
                , EDN_CHANGE);
        mtDlgData->dirty = TRUE;
    }
}

/*
 * UpdateFineSlider() - Updates the range of the fine slider when the note
 *                      slider reaches the ends of its range.
 */
int mg_UpdateFineSlider(HWND fineSlider, BYTE *data, int noteIdx, int note
        , int fine)
{
    int oldNote;

    /*
     * The way the unit works is, each key has two settings for the frequency,
     * coarse and fine.  The fine setting uses only 6 bits, and the most
     * significant bit is used for the sign.  Each coarse setting can refer to
     * one of two notes, depending on whether the fine value is negative or
     * positive.  For example, say the slider for the coarse setting goes from
     * 13 to 108 and the fine slider goes from -31 to 32.  If the user slides
     * the fine slider thumb from -31 all the way across to 32, the note will
     * have gone up half a step, so the coarse setting will now refer to a
     * different note.  This is too confusing in practice, so what I've done
     * is translated this so that each coarse setting only refers to one note.
     * This means that when the coarse slider is at one of the ends of its
     * range, the fine slider range is cut in half (i.e. at note C#-1, the fine
     * range only goes from 0 to 32, and at C#7 it goes from -31 to -1).
     * Further complicating this is the fact that I've made my own fine slider
     * go from 0 to 63 with a visual offset of -31, so 0 to 30 on the slider
     * refers to 33 to 63 on the unit, and 31 to 63 refers to 0 to 32 on the
     * unit.  This means that there is a visual representation of -31 to 32,
     * an internal representation for the slider of 0 to 63, and an internal
     * representation of the unit setting which goes from 33 to 63, 0 to 32.
     * So, in the dialog, the coarse slider, when set to 13, refers to
     * C#-1 and the fine slider is restricted to 0 to 32 (0 to 32 visually).
     * I've added an extra setting, 109, to the range to include the extra
     * note, C#7, so when the coarse is set to 109, the fine is restricted to
     * 33 to 63 (-31 to -1 visually).
     */
    oldNote = data[noteIdx] + (fine > 32);
    if (note == 13 && oldNote != 13)
    {
        LcdCtrl_SetRange(fineSlider, 31, 63);
        if (!mg_initializing && fine > 32) {
            LcdCtrl_SetValue(fineSlider, 31);
            fine = 0;
        }
    }
    else if (note == 109 && oldNote != 109)
    {
        LcdCtrl_SetRange(fineSlider, 0, 30);
        if (!mg_initializing && fine <= 32) {
            LcdCtrl_SetValue(fineSlider, 30);
            fine = 63;
        }
    }
    else if (note != 13 && note != 109 && (oldNote == 13 || oldNote == 109))
    {
        LcdCtrl_SetRange(fineSlider, 0, 63);
    }

    return fine;
}

/*
 * UpdateFreqDisplay() - Updates a particular frequency display with the
 *                       frequency of the current parameters.
 */
void mg_UpdateFreqDisplay(HWND mtDlg, int key, BYTE *data)
{
    UINT freqId = key + MT_FREQ_OFFSET;
    int noteIdx = key << 1;
    int fineIdx = noteIdx + 1;
    HWND freqDisplay = GetDlgItem(mtDlg, freqId);
    int note = data[noteIdx];
    int fine = data[fineIdx];
#define TEXT_LEN 10
    _TUCHAR text[TEXT_LEN];
    double freq = (440.0 / 32.0)
        * pow(2, ((double) ((note - 9) * 64 + fine)) / (12.0 * 64.0));
    
    _sntprintf(text, TEXT_LEN, _T("%4.1f"), freq);
    text[TEXT_LEN - 1] = '\0';
    SetWindowText(freqDisplay, text);
}

