/*
 * pfmdlg.c - TX81Z Performance Editor
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
#include "copyinstdlg.h"
#include "kybdctrl.h"
#include "lcdctrl.h"
#include "mainwnd.h"
#include "menubtn.h"
#include "minifont.h"
#include "pfmnav.h"
#include "prog.h"
#include "resource.h"
#include "snapshot.h"
#include "storedlg.h"
#include "tx81z.h"
#include "undo.h"
#include "voicemenu.h"
#include "pfmdlg.h"

/*
 * Macros
 */
#define INST_DIFF (TX81Z_PCED_INST2_MAX_NOTES - TX81Z_PCED_INST1_MAX_NOTES)

/*
 * Global constants
 */
const _TUCHAR *PfmDlg_className = _T("PfmDlg");

/*
 * Global procedures
 */
extern BOOL PfmDlg_Create(HWND parentWnd, PCED *pfm);
extern void PfmDlg_Update(HWND pfmDlg, PCED *pfm);

/*
 * Global variables
 */
_TUCHAR PfmDlg_voiceNumSettings[160][3];

/*
 * Unit procedures
 */
static int pd_AssignedNoteCnt(UINT ctrlID);
static void CALLBACK pd_ChangeParameter(UINT parameter, int value, CPSEND send);
static BOOL CALLBACK pd_DlgProc(HWND pfmDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void pd_DrawEdges(HDC dC);
static void pd_EnableInstrument(HWND pfmDlg, UINT ctrlID, BOOL enable);
static void pd_InitControlValues(HWND pfmDlg);
static void pd_OnActivate(HWND pfmDlg, UINT state, HWND otherWnd, BOOL minimized);
static void pd_OnCommand(HWND pfmDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void pd_OnDestroy(HWND pfmDlg);
static void pd_OnDrawItem(HWND pfmDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL pd_OnEraseBkgnd(HWND pfmDlg, HDC dC);
static void pd_OnGetMinMaxInfo(HWND pfmDlg, LPMINMAXINFO minMaxInfo);
static BOOL pd_OnInitDialog(HWND pfmDlg, HWND focusCtrl, LPARAM lParam);
static void pd_OnInitMenu(HWND pfmDlg, HMENU menu);
static void pd_OnKey(HWND pfmDlg, UINT vk, BOOL down, int repeat, UINT keyFlags);
static int pd_OnMaxNotesChange(HWND pfmDlg, UINT ctrlID, int value);
static UINT pd_OnNCHitTest(HWND pfmDlg, int x, int y);
static void pd_OnSize(HWND pfmDlg, UINT state, int cx, int cy);
static void pd_QuickEditCopy(HWND pfmDlg, int srcInst, int destInst, CPSEND send);
static void pd_QuickEditSwap(HWND pfmDlg, int srcInst, int destInst);
static void CALLBACK pd_Redo(HWND pfmDlg, CHANGE *change);
static BOOL CALLBACK pd_SubclassControls(HWND dlgCtrl, LPARAM lParam);
static void CALLBACK pd_Undo(HWND pfmDlg, CHANGE *change);
static void pd_UpdateLFOSettings(HWND pfmDlg);
static void pd_UpdateMicrotuneKeyLcd(HWND pfmDlg, int microtuneTable);
static void pd_UpdateQuickEditMenu(UINT ctrlID);
static BOOL pd_UpdateVoiceNameBtn(HWND pfmDlg, UINT ctrlID, int value);

/*
 * Unit constants
 */
#define INST_MAX_NOTES       0
#define INST_VOICE_NAME      1
#define INST_VOICE_NUM       2
#define INST_CHANNEL         3
#define INST_LIMIT_L         4
#define INST_LIMIT_H         5
#define INST_DETUNE          6
#define INST_NOTE_SHIFT      7
#define INST_VOLUME          8
#define INST_OUT_ASSIGN      9
#define INST_LFO_SELECT     10
#define INST_MICROTUNE      11
#define INST_QUICKEDIT_BTN  12

#define INST_GAP  1
#define INST_MARGIN  7

static const _TUCHAR pd_microtuneKeyCSettings[12][3] = {
    { _T(' '), _T(' '), _T('C') },
    { _T(' '), _T('D'), _T('b') },
    { _T(' '), _T(' '), _T('D') },
    { _T(' '), _T('E'), _T('b') },
    { _T(' '), _T(' '), _T('E') },
    { _T(' '), _T(' '), _T('F') },
    { _T(' '), _T('G'), _T('b') },
    { _T(' '), _T(' '), _T('G') },
    { _T(' '), _T('A'), _T('b') },
    { _T(' '), _T(' '), _T('A') },
    { _T(' '), _T('B'), _T('b') },
    { _T(' '), _T(' '), _T('B') }
};
static const _TUCHAR pd_microtuneKeyASettings[12][3] = {
    { _T(' '), _T(' '), _T('A') },
    { _T(' '), _T('A'), _T('#') },
    { _T(' '), _T(' '), _T('B') },
    { _T(' '), _T(' '), _T('C') },
    { _T(' '), _T('C'), _T('#') },
    { _T(' '), _T(' '), _T('D') },
    { _T(' '), _T('D'), _T('#') },
    { _T(' '), _T(' '), _T('E') },
    { _T(' '), _T(' '), _T('F') },
    { _T(' '), _T('F'), _T('#') },
    { _T(' '), _T(' '), _T('G') },
    { _T(' '), _T('G'), _T('#') }
};
static const _TUCHAR pd_microtuneKeyDisabledSettings[12][3] = {
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') },
    { _T(' '), _T('*'), _T(' ') }
};
static const _TUCHAR pd_outAssignSettings[4][3] = {
    { _T('o'), _T('f'), _T('f') },
    { _T(' '), _T(' '), _T('I') },
    { _T(' '), _T(' '), _T('\x88') },
    { _T(' '), _T('I'), _T('\x88') }
};
static _TUCHAR pd_lfoSelectSettings[4][3] = {
    { _T('o'), _T('f'), _T('f') },
    { _T(' '), _T(' '), _T('1') },
    { _T(' '), _T(' '), _T('2') },
    { _T('v'), _T('i'), _T('b') }
};

static const NUMLCDINIT pd_numLcdInits[] = {
    { IDC_INST1_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST2_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST3_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST4_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST5_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST6_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST7_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST8_MAX_NOTES,  3, 0,  8,   0, NULL },
    { IDC_INST1_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST2_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST3_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST4_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST5_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST6_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST7_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST8_DETUNE,     3, 0, 14,  -7, NULL },
    { IDC_INST1_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST2_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST3_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST4_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST5_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST6_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST7_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST8_NOTE_SHIFT, 3, 0, 48, -24, LcdCtrl_PageSize12 },
    { IDC_INST1_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST2_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST3_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST4_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST5_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST6_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST7_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_INST8_VOLUME,     3, 0, 99,   0, LcdCtrl_PageSizeSnap5 },
};
#define pd_numLcdInitCnt (sizeof pd_numLcdInits / sizeof pd_numLcdInits[0])

static const SPECIALLCDINIT pd_specialLcdInits[] = {
    { IDC_MICROTUNE_KEY,    3, 0,  11, pd_microtuneKeyDisabledSettings[0], NULL },
    { IDC_INST1_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST2_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST3_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST4_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST5_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST6_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST7_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST8_VOICE_NUM,  3, 0, 159,  PfmDlg_voiceNumSettings[0], LcdCtrl_PageSize8 },
    { IDC_INST1_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST2_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST3_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST4_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST5_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST6_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST7_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST8_CHANNEL,    3, 0,  16,       Prog_rcvChSettings[0], NULL },
    { IDC_INST1_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST2_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST3_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST4_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST5_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST6_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST7_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST8_LIMIT_L,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST1_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST2_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST3_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST4_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST5_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST6_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST7_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST8_LIMIT_H,    3, 0, 127,      Prog_keyNameStrings[0], LcdCtrl_PageSize12 },
    { IDC_INST1_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST2_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST3_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST4_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST5_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST6_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST7_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST8_OUT_ASSIGN, 3, 0,   3,     pd_outAssignSettings[0], NULL },
    { IDC_INST1_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST2_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST3_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST4_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST5_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST6_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST7_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
    { IDC_INST8_LFO_SELECT, 3, 0,   3,     pd_lfoSelectSettings[0], NULL },
};
#define pd_specialLcdInitCnt (sizeof pd_specialLcdInits / sizeof pd_specialLcdInits[0])

static const _TUCHAR pd_normal[] = _T("Normal");
static const _TUCHAR pd_alternate[] = _T("Alternate");
static struct {
    unsigned ctrlID;
    const _TUCHAR *modeOn;
    const _TUCHAR *modeOff;
} pd_toggleInits[] = {
    { IDC_ASSIGN_MODE, pd_alternate, pd_normal },
    { IDC_INST1_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST2_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST3_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST4_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST5_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST6_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST7_MICROTUNE, Prog_on, Prog_off },
    { IDC_INST8_MICROTUNE, Prog_on, Prog_off },
};
#define pd_toggleInitCnt (sizeof pd_toggleInits / sizeof pd_toggleInits[0])


/*
 * Unit variables
 */
static HWND pd_parentWnd;  /* always the main window */
static BYTE *pd_data;  /* the pced.data member of the snapshot */
static HWND pd_pfmDlg;  /* the performance editor window */
static HWND pd_voiceMenu;  /* the voice menu window */
static int pd_voiceMenuInst;  /* the instrument that's voice is currently being
                                 set with the voice menu */
static HMENU pd_menu;  /* the main menu of the performance editor */
static UINT pd_quickEdit;  /* the instrument that the quick edit menu is
                              currently open for */
static BOOL pd_dirty;  /* tracks whether or not a EN_CHANGE notification has
                          been sent to the main window */
static UNDO *pd_undo;  /* the undo list */
static BOOL pd_undoGroup;  /* true if the next item added to the undo list
                              should be the start of a group of related
                              changes */
static BOOL pd_undoFlag;  /* to prevent changes from getting added to the undo
                             list while undoing */
static int pd_minWidth, pd_height;  /* window size constraints */
static HWND pd_pfmNameGbx;
static HWND pd_instCtrls[8][13];
static HWND pd_assignModeLbl;
static HWND pd_assignModeBtn;
static HWND pd_effectLbl;
static HWND pd_effectCbx;
static HWND pd_microtuneLbl;
static HWND pd_microtuneCbx;
static HWND pd_microtuneKeyLbl;
static HWND pd_microtuneKeyLcd;
static RECT pd_pfmNameGbxRect;
static AREA pd_instCtrlAreas[8][14];
static AREA pd_assignModeLblArea;
static AREA pd_assignModeBtnArea;
static AREA pd_effectLblArea;
static AREA pd_effectCbxArea;
static AREA pd_microtuneLblArea;
static AREA pd_microtuneCbxArea;
static AREA pd_microtuneKeyLblArea;
static AREA pd_microtuneKeyLcdArea;
static float pd_microtuneLblRatio;
static float pd_microtuneCbxRatio;
static float pd_assignModeLblRatio;
static float pd_assignModeBtnRatio;
static float pd_effectLblRatio;
static float pd_effectCbxRatio;
static float pd_microtuneKeyLblRatio;
static float pd_microtuneKeyLcdRatio;
static float pd_microtuneKeyLcdWidthRatio;

/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL PfmDlg_Create(HWND parentWnd, PCED *pfm)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_pfmDlg) {
        PfmDlg_Update(Prog_pfmDlg, pfm);
        if (IsIconic(Prog_pfmDlg)) {
            OpenIcon(Prog_pfmDlg);
        }
        BringWindowToTop(Prog_pfmDlg);
    } else {
        pd_parentWnd = parentWnd;
        pd_data = pfm->data;
        Prog_pfmDlg = CreateDialogParam(Prog_instance, (LPCTSTR) IDD_PFMDLG
                , HWND_DESKTOP, pd_DlgProc, 0);
        if (!Prog_pfmDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog box.
 */
void PfmDlg_Update(HWND pfmDlg, PCED *pfm)
{
    if (pfm) {
        pd_data = pfm->data;
        pd_InitControlValues(pfmDlg);
    }
    VoiceMenu_Update(pd_voiceMenu);
}

/*
 * AssignedNoteCnt() - Returns the number of notes allocated to the instrument
 *                     associated with resource identifier ctrlID.
 */
int pd_AssignedNoteCnt(UINT ctrlID)
{
    return pd_data[(ctrlID - PCED_ID_FIRST) / INST_DIFF * INST_DIFF];
}

/*
 * ChangeParameter() - Sends parameter changes and updates snapshot data.
 */
void CALLBACK pd_ChangeParameter(UINT parameter, int value, CPSEND send)
{
    UINT ctrlID = parameter + PCED_ID_FIRST;
    int relativeParameter = -1;

    assert(parameter >= 0 && parameter <= 109);
    /*
     * Adjust controls that depend on the parameter.
     */
    if (parameter >= TX81Z_PCED_INST1_MAX_NOTES
            && parameter <= TX81Z_PCED_INST8_MICROTUNE)
    {
        relativeParameter = parameter % INST_DIFF;

        /*
         * If the control is a max notes control.
         */
        if (relativeParameter == 0) {
            value = pd_OnMaxNotesChange(pd_pfmDlg, ctrlID
                    , LcdCtrl_GetValue(GetDlgItem(pd_pfmDlg, ctrlID)));
        }
    }
#ifdef NO_REDUNDANT_SENDS
    if (value == pd_data[parameter]) {
        return;
    }
#endif
    /*
     * Add the change to the undo list.
     */
    if (!pd_undoFlag) {
        if (parameter < TX81Z_PCED_NAME) {
            int oldValue = pd_data[parameter];

            Undo_AddChange(pd_undo, ctrlID, sizeof(int), &oldValue, &value
                    , pd_undoGroup);
        } else {
            char newName[TX81Z_NAME_LEN];

            memcpy(newName, &pd_data[TX81Z_PCED_NAME], TX81Z_NAME_LEN);
            newName[parameter - TX81Z_PCED_NAME] = value;
            Undo_AddChange(pd_undo, IDC_PFM_NAME, TX81Z_NAME_LEN
                    , &pd_data[TX81Z_PCED_NAME], newName, pd_undoGroup);
        }
        pd_undoGroup = FALSE;
        MenuItem_Enable(pd_menu, IDM_UNDO);
        MenuItem_Disable(pd_menu, IDM_REDO);
    }
    /*
     * Update the unit.
     */
    if (send) {
        TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_PCED, parameter, value);
    }
    /*
     * Update the snapshot buffer.
     */
    pd_data[parameter] = value;
    /*
     * If the parameter was a Max Notes parameter then update the LFO
     * instrument numbers.
     */
    if (relativeParameter == 0) {
        pd_UpdateLFOSettings(pd_pfmDlg);
    }
    /*
     * Notify the main window of the modification status if it hasn't already
     * been done.
     */
    if (!pd_dirty) {
        SendNotification(Prog_mainWnd, IDD_PFMDLG, pd_pfmDlg, EDN_CHANGE);
        pd_dirty = TRUE;
    }
}

/*
 * DlgProc()
 */
BOOL CALLBACK pd_DlgProc(HWND pfmDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(pfmDlg, WM_ACTIVATE, pd_OnActivate);
        HANDLE_MSG(pfmDlg, WM_COMMAND, pd_OnCommand);
        HANDLE_MSG(pfmDlg, WM_DESTROY, pd_OnDestroy);
        HANDLE_MSG(pfmDlg, WM_DRAWITEM, pd_OnDrawItem);
        HANDLE_MSG(pfmDlg, WM_ERASEBKGND, pd_OnEraseBkgnd);
        HANDLE_MSG(pfmDlg, WM_GETMINMAXINFO, pd_OnGetMinMaxInfo);
        HANDLE_MSG(pfmDlg, WM_INITDIALOG, pd_OnInitDialog);
        HANDLE_MSG(pfmDlg, WM_INITMENU, pd_OnInitMenu);
        HANDLE_MSG(pfmDlg, WM_KEYDOWN, pd_OnKey);
        HANDLE_MSG(pfmDlg, WM_NCHITTEST, pd_OnNCHitTest);
        HANDLE_MSG(pfmDlg, WM_SIZE, pd_OnSize);
        case EDM_REFRESH:
            pd_InitControlValues(pfmDlg);
            return FALSE;
        case EDM_SAVED:
            pd_dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            pd_undoGroup = TRUE;
            break;
    }
    return FALSE;
}

/*
 * DrawEdges() - Draws pseudo-group boxes around the instrument controls.
 */
void pd_DrawEdges(HDC dC)
{
    RECT edgeRect;
    RECT fillRect;
    HPEN oldPen = SelectPen(dC, Prog_dlgPen);
    HBRUSH oldBrush = SelectBrush(dC, Prog_dlgBrush);
    RECT qeRect, mtRect;
    int i;

    /*
     * Fill between the voice name and assign mode controls.
     */
    fillRect.left = pd_pfmNameGbxRect.right;
    fillRect.top = 0;
    fillRect.right = pd_assignModeBtnArea.x;
    fillRect.bottom = pd_pfmNameGbxRect.bottom;
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill between the assign mode and effect select controls.
     */
    fillRect.left = AREA_R(pd_assignModeBtnArea);
    fillRect.right = pd_effectCbxArea.x;
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill between the effect select and microtune table contros.
     */
    fillRect.left = AREA_R(pd_effectCbxArea);
    fillRect.right = pd_microtuneCbxArea.x;
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill between the microtune table and microtune key controls.
     */
    fillRect.left = AREA_R(pd_microtuneCbxArea);
    fillRect.right = pd_microtuneKeyLcdArea.x;
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill to the right of the microtune key LCD.
     */
    fillRect.left = AREA_R(pd_microtuneKeyLcdArea);
    fillRect.right = fillRect.left + 5;
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill below the microtune key LCD.
     */
    fillRect.right = fillRect.left;
    fillRect.left = pd_microtuneKeyLcdArea.x;
    fillRect.top = AREA_B(pd_microtuneKeyLcdArea);
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    /*
     * Fill below the effect select combo box.
     */
    fillRect.left = pd_effectCbxArea.x;
    fillRect.top = AREA_B(pd_effectCbxArea);
    fillRect.right = AREA_R(pd_effectCbxArea);
    Rectangle(dC, PASS_RECT_FIELDS(fillRect));

    /*
     * For each instrument
     */
    for (i = 0; i < 8; i++) {
        /*
         * Get the rectangle of the quick edit button.
         */
        AreaToRect(&qeRect, &pd_instCtrlAreas[i][INST_QUICKEDIT_BTN]);
        /*
         * Get the rectangle of the microtune button.
         */
        AreaToRect(&mtRect, &pd_instCtrlAreas[i][INST_MICROTUNE]);
        /*
         * Draw top side of group box to the right of the button.
         */
        edgeRect.left = qeRect.right;
        edgeRect.top = Rect_VCenter(&qeRect);
        edgeRect.right = mtRect.right + 6;
        edgeRect.bottom = mtRect.bottom + 6;
        DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_TOP);
        /*
         * Draw bottom three sides of group box.
         */
        edgeRect.left = qeRect.left - 6;
        DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_LEFT | BF_BOTTOM | BF_RIGHT);
        /*
         * Draw top side of group box to the left of the button.
         */
        edgeRect.left++;
        edgeRect.right = qeRect.left;
        DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_TOP);
        /*
         * Fill the area between the group box and the controls on the left
         * side.
         */
        fillRect.left = qeRect.left - 4;
        fillRect.top = edgeRect.top + 2;
        fillRect.right = mtRect.left;
        fillRect.bottom = edgeRect.bottom - 2;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill the area between the group box and the controls on the right
         * side.
         */
        fillRect.left = mtRect.right;
        fillRect.right = fillRect.left + 4;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill to the right between instruments.
         */
        fillRect.left = fillRect.right + 2;
        fillRect.right = fillRect.left + 4;
        fillRect.top = qeRect.top;
        fillRect.bottom = edgeRect.bottom + 7;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill to the left of the instrument button above the group box.
         */
        fillRect.left = fillRect.right = qeRect.left;
        fillRect.left -= 6;
        fillRect.bottom = edgeRect.top;
        /*
         * Skip the first column on this next fill since the controls don't move.
         */
        if (i & 3) {
            Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        }
        /*
         * Fill to the right of the instrument button above the group box.
         */
        fillRect.left = qeRect.right;
        fillRect.right = mtRect.right + 6;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill to the right of the instrument button below the group box.
         */
        fillRect.top = edgeRect.top + 2;
        fillRect.bottom = qeRect.bottom;
        fillRect.right -= 2;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill the gap between the instrument button and the max notes slider.
         */
        fillRect.top = fillRect.bottom;
        fillRect.bottom = pd_instCtrlAreas[i][INST_MAX_NOTES].y;
        fillRect.left = mtRect.left;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
        /*
         * Fill the gap between the microtune button and the group box on the
         * bottom.
         */
        fillRect.top = mtRect.bottom;
        fillRect.bottom = edgeRect.bottom - 2;
        Rectangle(dC, PASS_RECT_FIELDS(fillRect));
    }
    SelectPen(dC, oldPen);
    SelectBrush(dC, oldBrush);
}

/*
 * EnableInstrument() - Enables or disables all of the controls for an
 *                      instrument.
 */
void pd_EnableInstrument(HWND pfmDlg, UINT ctrlID, BOOL enable)
{
    HWND ctrl;
    int i;
    int value;

    /*
     * Voice name
     */
    EnableWindow(GetDlgItem(pfmDlg, ctrlID + 1), enable);

    /*
     * Everything else
     */
    for (i = 2; i < INST_DIFF; i++) {
        ctrl = GetDlgItem(pfmDlg, ctrlID + i);
        if (i == 2) {
            value = (pd_data[ctrlID - PCED_ID_FIRST + 1] << 7)
                | pd_data[ctrlID - PCED_ID_FIRST + 2];
        } else {
            value = pd_data[ctrlID - PCED_ID_FIRST + i];
        }
        LcdCtrl_SetValue(ctrl, value);
        if (!enable) {
            LcdCtrl_SetText(ctrl, Prog_disabled);
        }
        EnableWindow(ctrl, enable);
    }
}

/*
 * InitControlValues() - Sets all controls values from the snapshot data.
 */
void pd_InitControlValues(HWND pfmDlg)
{
    UINT ctrlID;
    HWND ctrl;
    int i, idx;
    int value;
    _TUCHAR pfmName[11];

    /*
     * Performance name.
     */
    FromAnsiNCopy(pfmName, &pd_data[TX81Z_PCED_NAME], TX81Z_NAME_LEN);
    LcdCtrl_SetText(GetDlgItem(pfmDlg, IDC_PFM_NAME), pfmName);

    /*
     * Effect select combo
     */
    ComboBox_SetCurSel(pd_effectCbx, pd_data[TX81Z_PCED_EFFECT_SELECT]);

    /*
     * Micro tune table combo
     */
    value = pd_data[TX81Z_PCED_MICROTUNE_TABLE];
    ComboBox_SetCurSel(pd_microtuneCbx, value);
    pd_UpdateMicrotuneKeyLcd(pfmDlg, value);

    /*
     * Init numeric LCD's.
     */
    for (i = 0; i < pd_numLcdInitCnt; i++) {
        ctrlID = pd_numLcdInits[i].ctrlID;
        ctrl = GetDlgItem(pfmDlg, ctrlID);
        LcdCtrl_SetValue(ctrl, pd_data[ctrlID - PCED_ID_FIRST]);
    }
    /*
     * Init special LCD's
     */
    for (i = 0; i < pd_specialLcdInitCnt; i++) {
        ctrlID = pd_specialLcdInits[i].ctrlID;
        ctrl = GetDlgItem(pfmDlg, ctrlID);
        idx = ctrlID - PCED_ID_FIRST;
        /*
         * If this is a voice control
         */
        if (pd_specialLcdInits[i].max == 159) {
            /*
             * Combine the MSB and the num portions of the value.
             */
            value = (pd_data[idx - 1] << 7) | pd_data[idx];
            LcdCtrl_SetValue(ctrl, value);
            pd_UpdateVoiceNameBtn(pfmDlg, ctrlID, value);
        } else {
            LcdCtrl_SetValue(ctrl, pd_data[idx]);
        }
    }
    /*
     * Toggle buttons
     */
    for (i = 0; i < pd_toggleInitCnt; i++) {
        ctrlID = pd_toggleInits[i].ctrlID;
        ctrl = GetDlgItem(pfmDlg, ctrlID);
        value = pd_data[ctrlID - PCED_ID_FIRST] != 0;
        Button_SetCheck(ctrl, value != 0);
        Button_SetText(ctrl, value ? pd_toggleInits[i].modeOn
                : pd_toggleInits[i].modeOff);
    }
    /*
     * Enable instruments.
     */
    for (ctrlID = IDC_INST1_MAX_NOTES; ctrlID <= IDC_INST8_MAX_NOTES
            ; ctrlID += INST_DIFF) {
        value = pd_data[ctrlID - PCED_ID_FIRST];
        ctrl = GetDlgItem(pfmDlg, ctrlID);
        pd_EnableInstrument(pfmDlg, ctrlID, value);
    }
    pd_UpdateLFOSettings(pfmDlg);
    /*
     * Initialize the voice menu control.
     */
    VoiceMenu_Update(pd_voiceMenu);
    /*
     * Reset the change notification flag.
     */
    pd_dirty = FALSE;
    /*
     * Reset the undo state.
     */
    Undo_Clear(pd_undo);
    pd_undoGroup = TRUE;
    MenuItem_Disable(pd_menu, IDM_UNDO);
    MenuItem_Disable(pd_menu, IDM_REDO);
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void pd_OnActivate(HWND pfmDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = pfmDlg;
}

/*
 * OnCommand - handles OK and X buttons
 */
void pd_OnCommand(HWND pfmDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    int value;

    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(pfmDlg);
            return;
        case IDM_RETRIEVE_PFM:
            TX81Z_RetrieveData(Prog_midi, REQ_PCED, &Prog_snapshot);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_PFMDLG, EDN_CHANGE)
                    , (LPARAM) pfmDlg);
            pd_InitControlValues(pfmDlg);
            return;
        case IDM_TRANSMIT_PFM:
            TX81Z_SendData(Prog_midi, REQ_PCED, &Prog_snapshot);
            return;
        case IDM_STORE_PFM: {
            int destIndex;

            if (StoreDlg_Create(pfmDlg, META_PCED, &destIndex)) {
                MainWnd_StoreItem(SI_PCED, destIndex);
                /*
                 * Storing the performance loads up PF01 into the editor as it
                 * tries to figure out what patch on the unit is currently
                 * dialed up, so this request is necessary to restore the
                 * editor to its original state.
                 */
                TX81Z_RetrieveData(Prog_midi, REQ_PCED, &Prog_snapshot);
            }
            return;
        }
        case IDM_INIT_SINGLE_PFM:
        {
            const BYTE *initPfm;
            initPfm = TX81Z_initPMemSingle;
            goto InitPfm;
        case IDM_INIT_DUAL_PFM:
            initPfm = TX81Z_initPMemDual;
            goto InitPfm;
        case IDM_INIT_SPLIT_PFM:
            initPfm = TX81Z_initPMemSplit;
            goto InitPfm;
        case IDM_INIT_MONO8_PFM:
            initPfm = TX81Z_initPMemMono8;
            goto InitPfm;
        case IDM_INIT_POLY4_PFM:
            initPfm = TX81Z_initPMemPoly4;
InitPfm:
            TX81Z_InitPfm(Prog_midi, &Prog_snapshot, initPfm);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_PFMDLG, EDN_CHANGE), (LPARAM) pfmDlg);
            pd_InitControlValues(pfmDlg);
            return;
        }
        case IDM_ADD_TO_LIB_1:
        case IDM_ADD_TO_LIB_2:
        {
            int itemIdx = SI_PCED;
            int libIdx = ctrlID - IDM_ADD_TO_LIB_1;  /* assumes IDM_ADD_TO_LIB1
                                                       and IDM_ADD_TO_LIB2 are
                                                       adjacent */

            MainWnd_AddToLib(Prog_mainWnd, libIdx, &itemIdx, 1);
            return;
        }
        case IDM_BUNDLE_TO_LIB_1:
        case IDM_BUNDLE_TO_LIB_2:
        {
            int itemIdx = SI_PCED;
            int libIdx = ctrlID - IDM_BUNDLE_TO_LIB_1; /* assumes
                                                         IDM_BUNDLE_TO_LIB1 and
                                                         IDM_BUNDLE_TO_LIB2 are
                                                         adjacent */

            MainWnd_BundleToLib(Prog_mainWnd, libIdx, &itemIdx, 1);
            return;
        }
        case IDM_UNDO:
            Undo_Undo(pd_undo, pd_Undo, pfmDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(pd_undo, pd_Redo, pfmDlg);
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
        case IDM_FXDLG:
        case IDM_PCDLG:
        case IDM_MTODLG:
        case IDM_MTFDLG:
        case IDM_SYSDLG:
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_HELP:
            Prog_OpenHelp(pfmDlg, _T("performance_editor.html"));
            return;
        /*
         * Performance editing commands
         */
        case IDC_PFM_NAME:
        {
            if (notify == LCN_EDITUPDATE) {
                int cursorPos = LcdCtrl_GetCursorPos(ctrl);

                ctrlID = IDC_PFM_NAME + cursorPos;
                value = LcdCtrl_GetChar(ctrl, cursorPos);
                /*
                 * Send a notification to the main window so it can update the
                 * performance in the snapshot list.
                 */
                SendNotification(Prog_mainWnd, IDD_PFMDLG, pfmDlg, EDN_CHANGE);
            } else {
                return;
            }
            break;
        }
        case IDC_EFFECT_SELECT:
            if (notify == CBN_SELCHANGE) {
                value = ComboBox_GetCurSel(ctrl);
                break;
            }
            return;
        case IDC_MICROTUNE_TABLE:
            if (notify == CBN_SELCHANGE) {
                value = ComboBox_GetCurSel(ctrl);
                pd_UpdateMicrotuneKeyLcd(pfmDlg, value);
                break;
            }
            return;
        case IDC_INST1_MAX_NOTES:
        case IDC_INST2_MAX_NOTES:
        case IDC_INST3_MAX_NOTES:
        case IDC_INST4_MAX_NOTES:
        case IDC_INST5_MAX_NOTES:
        case IDC_INST6_MAX_NOTES:
        case IDC_INST7_MAX_NOTES:
        case IDC_INST8_MAX_NOTES:
            if (notify == LCN_SELCHANGE) {
                //value = pd_OnMaxNotesChange(pfmDlg, ctrlID
                //        , LcdCtrl_GetValue(ctrl));
                value = LcdCtrl_GetValue(ctrl);
                pd_ChangeParameter(ctrlID - PCED_ID_FIRST, value, CP_SEND);
            }
            return;
        case IDC_INST1_VOICE_NAME:
        case IDC_INST2_VOICE_NAME:
        case IDC_INST3_VOICE_NAME:
        case IDC_INST4_VOICE_NAME:
        case IDC_INST5_VOICE_NAME:
        case IDC_INST6_VOICE_NAME:
        case IDC_INST7_VOICE_NAME:
        case IDC_INST8_VOICE_NAME:
            if (notify == MBN_PUSHED) {
                SetFocus(GetDlgItem(pfmDlg, ctrlID + 1));
                pd_voiceMenuInst = (ctrlID - IDC_INST1_VOICE_NAME) / INST_DIFF;
                //VoiceMenu_Update(pd_voiceMenu);
                value = LcdCtrl_GetValue(
                        pd_instCtrls[pd_voiceMenuInst][INST_VOICE_NUM]
                    );
                VoiceMenu_SetLastSelection(value);
            }
            return;
        case IDM_VOICEMENU_SELECTION:
            value = VoiceMenu_GetLastSelection();
            LcdCtrl_SetValue(pd_instCtrls[pd_voiceMenuInst][INST_VOICE_NUM]
                    , value);
            ctrlID = IDC_INST1_VOICE_NUM + pd_voiceMenuInst * INST_DIFF;
            goto DoVoiceNameAsNum;
        case IDC_INST1_VOICE_NUM:
        case IDC_INST2_VOICE_NUM:
        case IDC_INST3_VOICE_NUM:
        case IDC_INST4_VOICE_NUM:
        case IDC_INST5_VOICE_NUM:
        case IDC_INST6_VOICE_NUM:
        case IDC_INST7_VOICE_NUM:
        case IDC_INST8_VOICE_NUM:
            if (notify == LCN_SELCHANGE) {
                value = LcdCtrl_GetValue(ctrl);
DoVoiceNameAsNum:
                if (pd_UpdateVoiceNameBtn(pfmDlg, ctrlID, value)) {
                    /*
                     * Send the parameter change (voice number has to come
                     * first, apparently, otherwise it ignores the MSB!)
                     */
                    pd_ChangeParameter(ctrlID - PCED_ID_FIRST, value & 0x7F
                            , CP_SEND);
                    /*
                     * Let the normal parameter change call set the second
                     * byte.
                     */
                    ctrlID--;
                    value = (value & 0x80) >> 7;
                } else {
                    return;
                }
                break;
            }
            return;
        case IDC_ASSIGN_MODE:
        case IDC_INST1_MICROTUNE:
        case IDC_INST2_MICROTUNE:
        case IDC_INST3_MICROTUNE:
        case IDC_INST4_MICROTUNE:
        case IDC_INST5_MICROTUNE:
        case IDC_INST6_MICROTUNE:
        case IDC_INST7_MICROTUNE:
        case IDC_INST8_MICROTUNE:
        {
            int i;

            for (i = 0; i < pd_toggleInitCnt; i++) {
                if (pd_toggleInits[i].ctrlID == ctrlID)
                    break;
            }
            value = Button_IsChecked(ctrl);
            Button_SetText(ctrl, value ? pd_toggleInits[i].modeOn
                    : pd_toggleInits[i].modeOff);
            break;
        }
        case IDC_INST1_QUICKEDIT_BTN:
        case IDC_INST2_QUICKEDIT_BTN:
        case IDC_INST3_QUICKEDIT_BTN:
        case IDC_INST4_QUICKEDIT_BTN:
        case IDC_INST5_QUICKEDIT_BTN:
        case IDC_INST6_QUICKEDIT_BTN:
        case IDC_INST7_QUICKEDIT_BTN:
        case IDC_INST8_QUICKEDIT_BTN:
            if (notify == MBN_PUSHED
                    && pd_quickEdit != ctrlID - IDC_INST1_QUICKEDIT_BTN)
            {
                pd_UpdateQuickEditMenu(ctrlID);
                pd_quickEdit = ctrlID - IDC_INST1_QUICKEDIT_BTN;
            }
            return;
        case IDM_COPY_TO_MULTIPLE_INSTS:
        {
            int result = CopyInstDlg_Create(pfmDlg, pd_quickEdit);
            int i;

            if (result) {
                for (i = 0; i < 8; i++) {
                    if (result & (1 << i)) {
                        pd_QuickEditCopy(pfmDlg, pd_quickEdit, i, FALSE);
                    }
                }
                TX81Z_SendData(Prog_midi, REQ_PCED, &Prog_snapshot);
            }
            return;
        }
        default:
            if (ctrlID >= IDM_COPY_INST && ctrlID < IDM_COPY_INST + 99) {
                int copyInstIdx = ctrlID - IDM_COPY_INST;

                pd_QuickEditCopy(pfmDlg, copyInstIdx / 10, copyInstIdx % 10
                        , TRUE);
                return;
            } else if (ctrlID >= IDM_SWAP_INST && ctrlID < IDM_SWAP_INST + 99) {
                int swapInstIdx = ctrlID - IDM_SWAP_INST;

                pd_QuickEditSwap(pfmDlg, swapInstIdx / 10, swapInstIdx % 10);
                return;
            }
            if (notify == LCN_SELCHANGE) {
                value = LcdCtrl_GetValue(ctrl);
                break;
            }
            return;
    }
    pd_ChangeParameter(ctrlID - PCED_ID_FIRST, value, CP_SEND);
}

/*
 * OnDestroy()
 */
void pd_OnDestroy(HWND pfmDlg)
{
    GetWindowPlacement(pfmDlg, &Prog_pfmDlgPlacement);
    Undo_Destroy(pd_undo);
    DestroyWindow(pd_voiceMenu);
    pd_voiceMenu = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
    Prog_pfmDlg = NULL;
}

/*
 * OnDrawItem() - Draws a voice name button.
 */
void pd_OnDrawItem(HWND pfmDlg, const DRAWITEMSTRUCT *drawItem)
{
    UINT ctrlType = drawItem->CtlType;
    UINT ctrlID = drawItem->CtlID;

    if (ctrlType == ODT_BUTTON) {
        if (ctrlID >= FIRST_PFM_QUICKEDIT_BTN
                && ctrlID <= LAST_PFM_QUICKEDIT_BTN)
        {
            MenuBtn_DrawButton(pfmDlg, drawItem);
            MenuBtn_DrawArrow(pfmDlg, drawItem);
        } else {
            HDC dC = drawItem->hDC;
            RECT itemRect = drawItem->rcItem;
            BOOL disabled = drawItem->itemState & ODS_DISABLED;
            int leftOffset = 5;
            int topOffset = 3;
            _TUCHAR voiceName[40];
            int voiceNameLen;

            /*
             * Draw the button border.
             */
            if (drawItem->itemState & ODS_SELECTED) {
                DrawEdge(dC, &itemRect, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
                leftOffset++;
                topOffset++;
            } else {
                DrawEdge(dC, &itemRect, EDGE_RAISED, BF_RECT | BF_MIDDLE);
            }
            /*
             * Draw the focus rectangle.
             */
            if (drawItem->itemState & ODS_FOCUS) {
                RECT focusRect;

                focusRect.left = itemRect.left + 2;
                focusRect.top = itemRect.top + 2;
                focusRect.right = itemRect.right - 3;
                focusRect.bottom = itemRect.bottom - 3;
                DrawFocusRect(dC, &focusRect);
            }
            /*
             * Draw the button text.
             */
            itemRect.left += leftOffset;
            itemRect.top += topOffset + 1;
            voiceNameLen = Button_GetText(drawItem->hwndItem, voiceName, 17);
            if (disabled) {
                MiniFont_DrawString(dC, itemRect.left + 1, itemRect.top + 1
                        , voiceName, 16, Prog_3dHighlightColor);
                MiniFont_DrawString(dC, itemRect.left, itemRect.top, voiceName
                        , 16, Prog_grayTextColor);
            } else {
                MiniFont_DrawString(dC, itemRect.left, itemRect.top, voiceName
                        , 16, Prog_dlgTextColor);
            }
            /*
             * Remove trailing spaces from the voice name length so the next
             * test doesn't hide the little arrow before it really needs to.
             */
            while (voiceNameLen) {
                if (voiceName[voiceNameLen - 1] == ' ') {
                    voiceNameLen--;
                } else {
                    break;
                }
            }
            /*
             * Draw the menu arrow if there's enough room for it.
             */
            if (RECT_W(itemRect) > voiceNameLen * 6 + 8) {
                MenuBtn_DrawArrow(pfmDlg, drawItem);
            }
        }
    }
}

/*
 * OnEraseBkgnd()
 */
BOOL pd_OnEraseBkgnd(HWND pfmDlg, HDC dC)
{
    RECT rect;

    GetClientRect(pfmDlg, &rect);
    FillRect(dC, &rect, Prog_dlgBrush);

    pd_DrawEdges(dC);

    return TRUE;
}

/*
 * OnGetMinMaxInfo()
 */
void pd_OnGetMinMaxInfo(HWND pfmDlg, LPMINMAXINFO minMaxInfo)
{
    minMaxInfo->ptMinTrackSize.x = pd_minWidth;
    minMaxInfo->ptMinTrackSize.y = pd_height;
    minMaxInfo->ptMaxTrackSize.y = pd_height;
}

/*
 * OnInitDialog() - Adjusts the window position, caches control handles and
 *                  areas for the window stretching maneuver, sets up and
 *                  populates controls, and subclasses controls for keyboard
 *                  navigation.
 */
BOOL pd_OnInitDialog(HWND pfmDlg, HWND focusCtrl, LPARAM lParam)
{
    UINT ctrlID;
    HWND ctrl;
    int i, inst;
    RECT wndRect;
    RECT clientRect;
    float clientW;

    pd_pfmDlg = pfmDlg;
    pd_quickEdit = (UINT) -1;
    /*
     * Create the voice menu window.
     */
    pd_voiceMenu = VoiceMenu_Create(pfmDlg, IDM_VOICEMENU_SELECTION);
    /*
     * Save the minimum size of the default window.
     */
    GetWindowRect(pfmDlg, &wndRect);
    pd_minWidth = RECT_W(wndRect);
    pd_height = RECT_H(wndRect);
    GetClientRect(pfmDlg, &clientRect);
    clientW = (float) RECT_W(clientRect);
    /*
     * Adjust and save the window position is it's never been saved.
     */
    if (IsRectEmpty(&Prog_pfmDlgPlacement.rcNormalPosition)) {
        Window_Center(pfmDlg, pd_parentWnd);
        GetWindowPlacement(pfmDlg, &Prog_pfmDlgPlacement);
    }
    /*
     * Cache control handles.
     */
    pd_pfmNameGbx = GetDlgItem(pfmDlg, IDC_PFM_NAME_GBX);
    for (inst = 0; inst < 8; inst++) {
        for (i = 0; i < INST_QUICKEDIT_BTN; i++) {
            ctrl = GetDlgItem(pfmDlg, IDC_INST1_MAX_NOTES + inst * 12 + i);
            pd_instCtrls[inst][i] = ctrl;
        }
        ctrl = GetDlgItem(pfmDlg, IDC_INST1_QUICKEDIT_BTN + inst);
        pd_instCtrls[inst][INST_QUICKEDIT_BTN] = ctrl;
    }
    pd_microtuneLbl = GetDlgItem(pfmDlg, IDC_MICROTUNE_TABLE_LBL);
    pd_microtuneCbx = GetDlgItem(pfmDlg, IDC_MICROTUNE_TABLE);
    pd_assignModeLbl = GetDlgItem(pfmDlg, IDC_ASSIGN_MODE_LBL);
    pd_assignModeBtn = GetDlgItem(pfmDlg, IDC_ASSIGN_MODE);
    pd_effectLbl = GetDlgItem(pfmDlg, IDC_EFFECT_SELECT_LBL);
    pd_effectCbx = GetDlgItem(pfmDlg, IDC_EFFECT_SELECT);
    pd_microtuneKeyLbl = GetDlgItem(pfmDlg, IDC_MICROTUNE_KEY_LBL);
    pd_microtuneKeyLcd = GetDlgItem(pfmDlg, IDC_MICROTUNE_KEY);
    /*
     * Get areas for controls.
     */
    Window_GetParentRelativeRect(pd_pfmNameGbx, pfmDlg, &pd_pfmNameGbxRect);
    for (inst = 0; inst < 8; inst++) {
        for (i = 0; i < 13; i++) {
            Window_GetParentRelativeArea(pd_instCtrls[inst][i], pfmDlg
                    , &pd_instCtrlAreas[inst][i]);
        }
    }
    Window_GetParentRelativeArea(pd_microtuneLbl, pfmDlg
            , &pd_microtuneLblArea);
    Window_GetParentRelativeArea(pd_microtuneCbx, pfmDlg
            , &pd_microtuneCbxArea);
    Window_GetParentRelativeArea(pd_assignModeLbl, pfmDlg
            , &pd_assignModeLblArea);
    Window_GetParentRelativeArea(pd_assignModeBtn, pfmDlg
            , &pd_assignModeBtnArea);
    Window_GetParentRelativeArea(pd_effectLbl, pfmDlg
            , &pd_effectLblArea);
    Window_GetParentRelativeArea(pd_effectCbx, pfmDlg
            , &pd_effectCbxArea);
    Window_GetParentRelativeArea(pd_microtuneKeyLbl, pfmDlg
            , &pd_microtuneKeyLblArea);
    Window_GetParentRelativeArea(pd_microtuneKeyLcd, pfmDlg
            , &pd_microtuneKeyLcdArea);
    pd_microtuneLblRatio = (float) pd_microtuneLblArea.x / clientW;
    pd_microtuneCbxRatio = (float) pd_microtuneCbxArea.x / clientW;
    pd_assignModeLblRatio = (float) pd_assignModeLblArea.x / clientW;
    pd_assignModeBtnRatio = (float) pd_assignModeBtnArea.x / clientW;
    pd_effectLblRatio = (float) pd_effectLblArea.x / clientW;
    pd_effectCbxRatio = (float) pd_effectCbxArea.x / clientW;
    pd_microtuneKeyLblRatio = (float) pd_microtuneKeyLblArea.x / clientW;
    pd_microtuneKeyLcdRatio = (float) pd_microtuneKeyLcdArea.x / clientW;
    pd_microtuneKeyLcdWidthRatio = (float) pd_microtuneKeyLcdArea.w / clientW;
    /*
     * Bold group box labels
     */
    Prog_SetBoldFont(pd_pfmNameGbx, 0);
    /*
     * Init voice name and quick edit menu buttons.
     */
    for (inst = 0; inst < 8; inst++) {
        ctrl = pd_instCtrls[inst][INST_QUICKEDIT_BTN];
        MenuBtn_Init(ctrl, Prog_pfmQuickEditMenu, NULL, Prog_menuFlags);
        Prog_SetBoldFont(ctrl, 0);
        ctrl = pd_instCtrls[inst][INST_VOICE_NAME];
        MenuBtn_Init(ctrl, pd_voiceMenu, VoiceMenu_Select, 0);
    }
    /*
     * Effect select combo
     */
    ComboBox_AddString(pd_effectCbx, _T("off"));
    ComboBox_AddString(pd_effectCbx, _T("delay"));
    ComboBox_AddString(pd_effectCbx, _T("pan"));
    ComboBox_AddString(pd_effectCbx, _T("chord"));
    /*
     * Micro tune table combo
     */
    ComboBox_AddString(pd_microtuneCbx, _T("User-Defined Octave"));
    ComboBox_AddString(pd_microtuneCbx, _T("User-Defined Full Keyboard"));
    ComboBox_AddString(pd_microtuneCbx, _T("1 - Equal Tempered"));
    ComboBox_AddString(pd_microtuneCbx, _T("2 - Pure Major"));
    ComboBox_AddString(pd_microtuneCbx, _T("3 - Pure Minor"));
    ComboBox_AddString(pd_microtuneCbx, _T("4 - Mean Tone"));
    ComboBox_AddString(pd_microtuneCbx, _T("5 - Pythagorean"));
    ComboBox_AddString(pd_microtuneCbx, _T("6 - Werckmeister"));
    ComboBox_AddString(pd_microtuneCbx, _T("7 - Kirnberger"));
    ComboBox_AddString(pd_microtuneCbx, _T("8 - Valloti & Young"));
    ComboBox_AddString(pd_microtuneCbx, _T("9 - 1/4 Shift Equal"));
    ComboBox_AddString(pd_microtuneCbx, _T("10 - 1/4 Tone"));
    ComboBox_AddString(pd_microtuneCbx, _T("11 - 1/8 Tone"));
    /*
     * Init performance name LCD.
     */
    LcdCtrl_TextInit(GetDlgItem(pfmDlg, IDC_PFM_NAME), TX81Z_NAME_LEN);
    /*
     * Init numeric LCD's.
     */
    for (i = 0; i < pd_numLcdInitCnt; i++) {
        ctrlID = pd_numLcdInits[i].ctrlID;
        ctrl = GetDlgItem(pfmDlg, ctrlID);
        LcdCtrl_NumInit(ctrl, &pd_numLcdInits[i]);
    }
    /*
     * Init special LCD's
     */
    for (i = 0; i < pd_specialLcdInitCnt; i++) {
        ctrlID = pd_specialLcdInits[i].ctrlID;
        LcdCtrl_SpecialInit(GetDlgItem(pfmDlg, ctrlID), &pd_specialLcdInits[i]);
    }
    /*
     * Subclass controls for keyboard navigation.
     */
    Prog_pfmDlg = pfmDlg;
    EnumChildWindows(pfmDlg, pd_SubclassControls, 0);
    /*
     * Set up the undo infrastructure (must be done before calling
     * InitControlValues).
     */
    pd_menu = GetMenu(pfmDlg);
    pd_undo = Undo_Create();
    /*
     * Initialize control settings.
     */
    pd_InitControlValues(pfmDlg);
    /*
     * Set the saved window position.
     */
#define RC Prog_pfmDlgPlacement.rcNormalPosition
    MoveWindow(pfmDlg, RC.left, RC.top, RECT_W(RC), RECT_H(RC), TRUE);
#undef RC
    ShowWindow(pfmDlg, SW_SHOWNORMAL);

    return TRUE;
}

/*
 * OnInitMenu() - Sets the file names for the "Add To Library n" and
 *                "Bundle to Library n" menu items.
 */
void pd_OnInitMenu(HWND pfmDlg, HMENU menu)
{
    HMENU editorMenu = GetSubMenu(menu, 0);
    MENUITEMINFO menuItemInfo;
#define ITEMSTRLEN  (_MAX_PATH + 40)
    _TUCHAR itemStr[ITEMSTRLEN];
    const _TUCHAR *fileName;
    
    /*
     * Set library 1 name.
     */
    fileName = FilePath_GetLeafPtr(Prog_lib[0].fileName);
    if (fileName[0] == '\0') {
        fileName = Prog_untitled;
    }
    _sntprintf(itemStr, ITEMSTRLEN, _T("Add To Library &1 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_ADD_TO_LIB_1;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_ADD_TO_LIB_1, FALSE, &menuItemInfo);
    _sntprintf(itemStr, ITEMSTRLEN, _T("Bundle To Library &1 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_BUNDLE_TO_LIB_1;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_BUNDLE_TO_LIB_1, FALSE, &menuItemInfo);
    /*
     * Set library 2 name.
     */
    fileName = FilePath_GetLeafPtr(Prog_lib[1].fileName);
    if (fileName[0] == '\0') {
        fileName = Prog_untitled;
    }
    _sntprintf(itemStr, ITEMSTRLEN, _T("Add To Library &2 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_ADD_TO_LIB_2;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_ADD_TO_LIB_2, FALSE, &menuItemInfo);
    _sntprintf(itemStr, ITEMSTRLEN, _T("Bundle To Library &2 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_BUNDLE_TO_LIB_2;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_BUNDLE_TO_LIB_2, FALSE, &menuItemInfo);
}

/*
 * OnKey() - Handles fancy keyboard navigation.
 */
void pd_OnKey(HWND pfmDlg, UINT vk, BOOL down, int repeat, UINT keyFlags)
{
    HWND focusCtrl = GetFocus();
    UINT focusID = GetDlgCtrlID(focusCtrl);
    HWND destCtrl;
    UINT navIndex;

    if (focusID == IDC_LCD_SCROLLBAR) {
        focusCtrl = GetParent(focusCtrl);
        focusID = GetDlgCtrlID(focusCtrl);
    }

    switch (vk) {
        case VK_LEFT:
            destCtrl = KeyNav_FindAdjacentCtrl(pfmDlg, PfmNav_nav
                    , PfmNav_navCnt, NULL, 0
                    , focusID, KN_LEFT, KN_RIGHT, &navIndex, FALSE);
            break;
        case VK_UP:
            destCtrl = KeyNav_FindAdjacentCtrl(pfmDlg, PfmNav_nav
                    , PfmNav_navCnt, NULL, 0
                    , focusID, KN_UP, KN_DOWN, &navIndex, FALSE);
            break;
        case VK_RIGHT:
            destCtrl = KeyNav_FindAdjacentCtrl(pfmDlg, PfmNav_nav
                    , PfmNav_navCnt, NULL, 0
                    , focusID, KN_RIGHT, KN_LEFT, &navIndex, FALSE);
            break;
        case VK_DOWN:
            destCtrl = KeyNav_FindAdjacentCtrl(pfmDlg, PfmNav_nav
                    , PfmNav_navCnt, NULL, 0
                    , focusID, KN_DOWN, KN_UP, &navIndex, FALSE);
            break;
        default:
            return;
    }
    if (destCtrl == NULL) {
        return;
    }
    SetFocus(destCtrl);
}

/*
 * OnMaxNotesChange() - Validates the note assignment and updates the LFO select
 *                     strings and controls.
 */
int pd_OnMaxNotesChange(HWND pfmDlg, UINT ctrlID, int value)
{
    UINT parameter = ctrlID - PCED_ID_FIRST;
    int assigned = pd_data[parameter];
    int reserved = 0;
    UINT i;

    /*
     * If the user is trying to increase the number of notes allocated for this
     * instrument.
     */
    if (value > assigned) {
        /*
         * Count up the total reserved notes in all instruments.
         */
        for (i = TX81Z_PCED_INST1_MAX_NOTES; i <= TX81Z_PCED_INST8_MAX_NOTES
                ; i += INST_DIFF)
        {
            if (pd_data[i] > 0) {
                reserved += pd_data[i];
            }
        }
        /*
         * Increase the number of assigned notes by the number of notes left
         * to assign.
         */
        assigned += 8 - reserved;
        /*
         * If there aren't enough assigned notes to meet the user's request.
         */
        if (assigned < value) {
            /*
             * Go through the instruments from the last up to the one after
             * the current instrument and rob notes from them to try and meet
             * the request.
             */
            for (i = TX81Z_PCED_INST8_MAX_NOTES; i > parameter; i -= INST_DIFF)
            {
                /*
                 * If the victim has notes.
                 */
                if (pd_data[i] > 0) {
                    int zero = 0;

                    /*
                     * Add them to the assigned count of the request.
                     */
                    assigned += pd_data[i];
                    /*
                     * Remove the notes from the victim.
                     */
                    Prog_ChangeControl(GetDlgItem(pfmDlg, i + PCED_ID_FIRST)
                            , i, &zero, CP_NOSEND, pd_ChangeParameter);
                    /*
                     * Disable the victim instrument.
                     */
                    pd_EnableInstrument(pfmDlg, i + PCED_ID_FIRST, FALSE);
                    /*
                     * If the assigned notes now meet the request, quit robbing
                     * notes.
                     */
                    if (assigned >= value) {
                        break;
                    }
                }
            }
            /*
             * If there still aren't enough notes, assign the notes that can
             * be assigned.
             */
            if (assigned < value) {
                Prog_ChangeControl(GetDlgItem(pfmDlg, ctrlID), parameter
                        , &assigned, CP_NOSEND, pd_ChangeParameter);
                value = assigned;
            }
        }
    }
    /*
     * Enable the instrument if notes are assigned to it, otherwise disable it.
     */
    pd_EnableInstrument(pfmDlg, ctrlID, value > 0);

    return value;
}

/*
 * OnNCHitTest() - Restricts window stretching to the left and right edges.
 */
UINT pd_OnNCHitTest(HWND pfmDlg, int x, int y)
{
    LRESULT hitTest = DefWindowProc(pfmDlg, WM_NCHITTEST, 0, MAKELPARAM(x, y));

    switch (hitTest) {
        case HTTOPLEFT:
        case HTBOTTOMLEFT:
            hitTest = HTLEFT;
            goto Override;
        case HTTOPRIGHT:
        case HTBOTTOMRIGHT:
            hitTest = HTRIGHT;
            goto Override;
        case HTBOTTOM:
        case HTTOP:
            hitTest = HTNOWHERE;
            goto Override;
    }
    return FALSE;

Override:
    SetWindowLong(pfmDlg, DWL_MSGRESULT, hitTest);
    return TRUE;
}

/*
 * OnSize() - Flicker-free OnSize() for pfmdlg.c
 */
void pd_OnSize(HWND pfmDlg, UINT state, int cx, int cy)
{
    HDC dC;
    int inst;
    int instCtrl;
    long instGbxW = (cx - pd_instCtrlAreas[0][INST_MAX_NOTES].x + 8
            - 3 * INST_GAP) / 4;
    long instCtrlW = instGbxW - INST_MARGIN * 2;
    long x;

    /*
     * Adjust the instrument group boxes.
     */
    x = pd_instCtrlAreas[0][INST_QUICKEDIT_BTN].x + instGbxW + INST_GAP;
    for (inst = 1; inst < 4; inst++) {
        pd_instCtrlAreas[inst][INST_QUICKEDIT_BTN].x = x;
        pd_instCtrlAreas[inst + 4][INST_QUICKEDIT_BTN].x = x;
        x += instGbxW + INST_GAP;
    }
    /*
     * Adjust the instrument controls.
     */
    for (instCtrl = 0; instCtrl < INST_QUICKEDIT_BTN; instCtrl++) {
        pd_instCtrlAreas[0][instCtrl].w = instCtrlW;
        pd_instCtrlAreas[4][instCtrl].w = instCtrlW;
    }
    x = pd_instCtrlAreas[0][0].x + instCtrlW + INST_MARGIN * 2
        + INST_GAP;
    for (inst = 1; inst < 4; inst++) {
        for (instCtrl = 0; instCtrl < INST_QUICKEDIT_BTN; instCtrl++) {
            pd_instCtrlAreas[inst][instCtrl].x = x;
            pd_instCtrlAreas[inst][instCtrl].w = instCtrlW;
            pd_instCtrlAreas[inst + 4][instCtrl].x = x;
            pd_instCtrlAreas[inst + 4][instCtrl].w = instCtrlW;
        }
        x += instCtrlW + INST_MARGIN * 2 + INST_GAP;
    }
    pd_microtuneLblArea.x = (long)((float) cx * pd_microtuneLblRatio);
    pd_microtuneCbxArea.x = (long)((float) cx * pd_microtuneCbxRatio);
    pd_assignModeLblArea.x = (long)((float) cx * pd_assignModeLblRatio);
    pd_assignModeBtnArea.x = (long)((float) cx * pd_assignModeBtnRatio);
    pd_effectLblArea.x = (long)((float) cx * pd_effectLblRatio);
    pd_effectCbxArea.x = (long)((float) cx * pd_effectCbxRatio);
    pd_microtuneKeyLblArea.x = (long)((float) cx * pd_microtuneKeyLblRatio);
    pd_microtuneKeyLblArea.w = cx - pd_microtuneKeyLblArea.x;
    pd_microtuneKeyLcdArea.x = (long)((float) cx * pd_microtuneKeyLcdRatio);
    pd_microtuneKeyLcdArea.w = (long)((float) cx * pd_microtuneKeyLcdWidthRatio);

    /*
     * Set up the deferred window move.
     */
    //defer = BeginDeferWindowPos(120);
    for (inst = 0; inst < 8; inst++) {
        InvalidateRect(pd_instCtrls[inst][INST_QUICKEDIT_BTN], NULL, FALSE);
        SetWindowPos(pd_instCtrls[inst][INST_QUICKEDIT_BTN], NULL
                , PASS_AREA_FIELDS(pd_instCtrlAreas[inst][INST_QUICKEDIT_BTN])
                , SWP_NOZORDER | SWP_NOREDRAW);
        for (instCtrl = 0; instCtrl < INST_QUICKEDIT_BTN; instCtrl++) {
            InvalidateRect(pd_instCtrls[inst][instCtrl], NULL, FALSE);
            SetWindowPos(pd_instCtrls[inst][instCtrl], NULL
                    , PASS_AREA_FIELDS(pd_instCtrlAreas[inst][instCtrl])
                    , SWP_NOZORDER | SWP_NOREDRAW);
        }
    }
    InvalidateRect(pd_microtuneLbl, NULL, FALSE);
    SetWindowPos(pd_microtuneLbl, NULL
            , PASS_AREA_FIELDS(pd_microtuneLblArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_microtuneCbx, NULL, FALSE);
    SetWindowPos(pd_microtuneCbx, NULL
            , PASS_AREA_FIELDS(pd_microtuneCbxArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_assignModeLbl, NULL, FALSE);
    SetWindowPos(pd_assignModeLbl, NULL
            , PASS_AREA_FIELDS(pd_assignModeLblArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_assignModeBtn, NULL, FALSE);
    SetWindowPos(pd_assignModeBtn, NULL
            , PASS_AREA_FIELDS(pd_assignModeBtnArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_effectLbl, NULL, FALSE);
    SetWindowPos(pd_effectLbl, NULL
            , PASS_AREA_FIELDS(pd_effectLblArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_effectCbx, NULL, FALSE);
    SetWindowPos(pd_effectCbx, NULL
            , PASS_AREA_FIELDS(pd_effectCbxArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_microtuneKeyLbl, NULL, FALSE);
    SetWindowPos(pd_microtuneKeyLbl, NULL
            , PASS_AREA_FIELDS(pd_microtuneKeyLblArea), SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(pd_microtuneKeyLcd, NULL, FALSE);
    SetWindowPos(pd_microtuneKeyLcd, NULL
            , PASS_AREA_FIELDS(pd_microtuneKeyLcdArea), SWP_NOZORDER | SWP_NOREDRAW);
    /*
     * Do the deferred window move.
     */
    /*
    if (!EndDeferWindowPos(defer)) {
        Error_LastErrorF(_T("EndDeferWindowPos()"));
        return;
    }*/
    /*
     * Manually erase the invalid pieces of the background.
     */
    // nah
    /*
     * Manually invalidate the controls.
     */
    for (inst = 0; inst < 8; inst++) {
        InvalidateRect(pd_instCtrls[inst][INST_QUICKEDIT_BTN], NULL, FALSE);
        for (instCtrl = 0; instCtrl < INST_QUICKEDIT_BTN; instCtrl++) {
            InvalidateRect(pd_instCtrls[inst][instCtrl], NULL, FALSE);
        }
    }
    InvalidateRect(pd_microtuneLbl, NULL, FALSE);
    InvalidateRect(pd_microtuneCbx, NULL, FALSE);
    InvalidateRect(pd_assignModeLbl, NULL, FALSE);
    InvalidateRect(pd_assignModeBtn, NULL, FALSE);
    InvalidateRect(pd_effectLbl, NULL, FALSE);
    InvalidateRect(pd_effectCbx, NULL, FALSE);
    InvalidateRect(pd_microtuneKeyLbl, NULL, FALSE);
    InvalidateRect(pd_microtuneKeyLcd, NULL, FALSE);

    dC = GetDC(pfmDlg);
    pd_DrawEdges(dC);
    ReleaseDC(pfmDlg, dC);
}

/*
 * QuickEditCopy() - Copies one instrument to another.
 */
void pd_QuickEditCopy(HWND pfmDlg, int srcInst, int destInst, CPSEND send)
{
    int srcOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * srcInst + 1;
    int destOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * destInst + 1;
    int srcValue;
    int i;

    /*
     * Copying the parameters backwards here because if it's done forwards
     * and the max notes is zero, the control is disabled before all of its
     * controls are set, which leaves the disabled parameters LCDs without
     * asterisks.
     */
    for (i = 1; i < INST_DIFF; i++, srcOffset++, destOffset++) {
        srcValue = pd_data[srcOffset];
        /*
         * Don't call Prog_ChangeControl for the voice MSB, just copy it,
         * otherwise it will open up the voice menu.
         */
        if (i == 1) {
            pd_data[destOffset] = srcValue;
        } else {
            if (i == 2) {
                srcValue |= pd_data[destOffset - 1] << 7;
            }
            Prog_ChangeControl(GetDlgItem(pfmDlg, PCED_ID_FIRST + destOffset)
                    , destOffset, &srcValue, send, pd_ChangeParameter);
            if (i == 2) {
                pd_UpdateVoiceNameBtn(pfmDlg, PCED_ID_FIRST + destOffset
                        , srcValue);
            }
        }
    }
    /*
     * Copy the Max Notes parameter.
     */
    srcOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * srcInst;
    destOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * destInst;
    srcValue = pd_data[srcOffset];
    Prog_ChangeControl(GetDlgItem(pfmDlg, PCED_ID_FIRST + destOffset)
            , destOffset, &srcValue, send, pd_ChangeParameter);
    /*
    if (send) {
        TX81Z_SendData(Prog_midi, REQ_PCED, &Prog_snapshot);
    }
    */
}

/*
 * QuickEditSwap() - Swaps two instruments.
 */
void pd_QuickEditSwap(HWND pfmDlg, int srcInst, int destInst)
{
    int srcOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * srcInst;
    int destOffset = TX81Z_PCED_INST1_MAX_NOTES + INST_DIFF * destInst;
    int i, t;

    for (i = 0; i < INST_DIFF; i++, srcOffset++, destOffset++) {
        int value;

        if (i == 1) {
            pd_data[destOffset] = pd_data[srcOffset];
        } else if (i == INST_DIFF - 2) {
            continue;
        } else {
            if (i == 2) {
                t = pd_data[srcOffset] | (pd_data[srcOffset - 1] << 7);
                value = pd_data[destOffset] | (pd_data[destOffset - 1] << 7);
            } else {
                t = pd_data[srcOffset];
                value = pd_data[destOffset];
            }
            Prog_ChangeControl(GetDlgItem(pfmDlg, PCED_ID_FIRST + srcOffset)
                    , srcOffset, &value, CP_NOSEND, pd_ChangeParameter);
            Prog_ChangeControl(GetDlgItem(pfmDlg, PCED_ID_FIRST + destOffset)
                    , destOffset, &t, CP_NOSEND, pd_ChangeParameter);
            if (i == 2) {
                pd_UpdateVoiceNameBtn(pfmDlg, PCED_ID_FIRST + srcOffset, value);
                pd_UpdateVoiceNameBtn(pfmDlg, PCED_ID_FIRST + destOffset, t);
            }
        }
    }
    TX81Z_SendData(Prog_midi, REQ_PCED, &Prog_snapshot);
}

/*
 * Redo() - Redoes a previously undone edit.
 */
void CALLBACK pd_Redo(HWND pfmDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(pfmDlg, change->ctrlID);
    int srcOffset = change->ctrlID - PCED_ID_FIRST;
    int relativeParameter = srcOffset % INST_DIFF;
    int srcValue = *(int *) change->newValue;

    pd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    if (relativeParameter == 1) {
        pd_data[srcOffset] = *(int *) change->oldValue;
    } else {
        if (relativeParameter == 2) {
            srcValue |= pd_data[srcOffset - 1] << 7;
        }
        Prog_ChangeControl(ctrl, srcOffset, &srcValue, CP_SEND
                , pd_ChangeParameter);
        if (relativeParameter == 2) {
            pd_UpdateVoiceNameBtn(pfmDlg, change->ctrlID, srcValue);
        }
    }
    pd_undoFlag = FALSE;
}

/*
 * SubclassControls() - Sets up standard windows controls for keyboard
 *                      navigation and middle mouse button panning.
 */
BOOL CALLBACK pd_SubclassControls(HWND ctrl, LPARAM lParam)
{
    _TUCHAR className[30];

    GetClassName(ctrl, className, 30);
    if (StriEq(className, Prog_buttonClassName)) {
        UINT ctrlID = GetDlgCtrlID(ctrl);
        if (ctrlID < 6600 && (ctrlID == IDC_ASSIGN_MODE
                    || ((ctrlID - PCED_ID_FIRST) % INST_DIFF) != 1))
        {
            KeyNav_SubclassButton(ctrl);
        }
    } else if (StriEq(className, Prog_comboBoxClassName)) {
        KeyNav_SubclassComboBox(ctrl);
    } else if (StriEq(className, LcdCtrl_className)) {
        KeyNav_SubclassLcdCtrl(ctrl);
    }
    return TRUE;
}

/*
 * Undo() - Undoes an edit done to a control.
 */
void CALLBACK pd_Undo(HWND pfmDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(pfmDlg, change->ctrlID);
    int srcOffset = change->ctrlID - PCED_ID_FIRST;
    int relativeParameter = srcOffset % INST_DIFF;
    int srcValue = *(int *) change->oldValue;

    pd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    if (relativeParameter == 1) {
        pd_data[srcOffset] = *(int *) change->oldValue;
    } else {
        if (relativeParameter == 2) {
            srcValue |= pd_data[srcOffset - 1] << 7;
        }
        Prog_ChangeControl(ctrl, srcOffset, &srcValue, CP_SEND
                , pd_ChangeParameter);
        if (relativeParameter == 2) {
            pd_UpdateVoiceNameBtn(pfmDlg, change->ctrlID, srcValue);
        }
    }
    pd_undoFlag = FALSE;
}

/*
 * UpdateLFOSettings() - Updates the LFO selection strings.
 */
void pd_UpdateLFOSettings(HWND pfmDlg)
{
    int i;
    int lfo = 1;
    int lfoSelectValue;

    /*
     * While the instruments are being stepped through, the first
     * two instruments with notes assigned are the ones that get
     * displayed in the LFO selection LCD, so update the two
     * strings.
     */
    for (i = TX81Z_PCED_INST1_MAX_NOTES; i <= TX81Z_PCED_INST8_MAX_NOTES
            ; i += INST_DIFF)
    {
        /*
         * If the instrument has notes assigned.
         */
        if (pd_data[i] > 0) {
            /*
             * Only two different LFO's from instruments can be used in a
             * performance.
             */
            if (lfo <= 2) {
                /*
                 * Set the LFO string to indicate the current instrument
                 * number.
                 */
                pd_lfoSelectSettings[lfo][0] = ' ';
                pd_lfoSelectSettings[lfo][1] = ' ';
                pd_lfoSelectSettings[lfo][2] = '1'
                    + (i - TX81Z_PCED_INST1_MAX_NOTES) / INST_DIFF;
                lfo++;
            }
        }
        /*
         * Refresh the LFO select displays that might be affected by the
         * string changes.
         */
        lfoSelectValue = pd_data[i + INST_LFO_SELECT];
        if (lfoSelectValue == 1 || lfoSelectValue == 2) {
            InvalidateRect(
                    GetDlgItem(pfmDlg, i + INST_LFO_SELECT + PCED_ID_FIRST)
                    , NULL
                    , FALSE
                );
        }
    }
    /*
     * If there aren't two instruments with notes assigned, then the LFO
     * select LCD displays dashes.
     */
    while (lfo <= 2) {
        for (i = 0; i < 3; i++) {
            pd_lfoSelectSettings[lfo][i] = '-';
        }
        lfo++;
    }
}

/*
 * UpdateMicrotuneKeyLcd() -
 */
void pd_UpdateMicrotuneKeyLcd(HWND pfmDlg, int microtuneTable)
{
    /*
     * Set the microtune key control strings based on the table
     * selection.
     */
    if (microtuneTable == 3 || microtuneTable == 5 || microtuneTable == 6) {
        LcdCtrl_SetStrings(pd_microtuneKeyLcd, pd_microtuneKeyCSettings);
    } else if (microtuneTable == 4) {
        LcdCtrl_SetStrings(pd_microtuneKeyLcd, pd_microtuneKeyASettings);
    } else {
        LcdCtrl_SetStrings(pd_microtuneKeyLcd, pd_microtuneKeyDisabledSettings);
    }
}

/*
 * UpdateQuickEditMenu() - Replaces the names of the quickedit submenus.
 */
void pd_UpdateQuickEditMenu(UINT ctrlID)
{
    static _TUCHAR *preps[2] = { _T("To"), _T("With") };
    static int iDs[2] = { IDM_COPY_INST, IDM_SWAP_INST };
    MENUITEMINFO menuItemInfo;
    _TUCHAR text[18];
    int instNum = ctrlID - IDC_INST1_QUICKEDIT_BTN;
    int i, j, n;
    HMENU subMenu;
    
    /*
     * Update the copy and swap menus.
     */
    for (j = 0; j < 2; j++) {
        /*
         * Change all the submenu items to "To Instrument #" and
         * "With Instrument #".
         */
        subMenu = GetSubMenu(Prog_pfmQuickEditMenu, j);
        for (i = n = 0; i < 8; i++) {
            if (i != instNum) {
                menuItemInfo.cbSize = sizeof(MENUITEMINFO);
                menuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
                menuItemInfo.fType = MFT_STRING;
                _sntprintf(text, 18, _T("%s Instrument %d"), preps[j], i + 1);
                menuItemInfo.dwTypeData = text;
                /*
                 * Copy inst 1 to inst 2  = 31301
                 * Copy inst 1 to inst 3  = 31302
                 * ...
                 * Copy inst 8 to inst 7  = 31376
                 *
                 * Swap inst 1 and inst 2 = 31401
                 * Swap inst 1 and inst 3 = 31402
                 * ...
                 * Copy inst 8 to inst 7  = 31476
                 */
                menuItemInfo.wID = iDs[j] + instNum * 10 + i;
                SetMenuItemInfo(subMenu, n++, MF_BYPOSITION, &menuItemInfo);
            }
        }
    }
}

/*
 * UpdateVoiceNameBtn() - Set the voice name button text and returns true if
 *                        there are voices assigned to the instrument.  CtrlID
 *                        is the ID of the voice number slider, which the 
 *                        value the voice name button is always based on.
 */
BOOL pd_UpdateVoiceNameBtn(HWND pfmDlg, UINT ctrlID, int value)
{
    /*
     * Update the corresponding voice name button.
     */
    int voiceNameCtrlID = ctrlID - 1;
    HWND voiceNameCtrl = GetDlgItem(pfmDlg, voiceNameCtrlID);
    BOOL hasNotesAssigned = pd_AssignedNoteCnt(ctrlID) != 0;
    _TUCHAR voiceName[17];

    Snapshot_FormatName(&Prog_snapshot, Snapshot_MemNumToSnapshotIndex(value)
            , NF_DASHED, voiceName);
    Button_SetText(voiceNameCtrl, voiceName);
    InvalidateRect(voiceNameCtrl, NULL, FALSE);
    EnableWindow(voiceNameCtrl, hasNotesAssigned);

    return hasNotesAssigned;
}

