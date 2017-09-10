/*
 * fxdlg.c - TX81Z Effects Editor
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
#include "keynav.h"
#include "ctrl/kybdctrl.h"
#include "ctrl/lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "snapshot.h"
#include "tx81z.h"
#include "undo.h"
#include "dlg/fxdlg.h"

/*
 * Global procedures
 */
extern BOOL FxDlg_Create(HWND parentWnd, FX *fx);
extern void FxDlg_Update(HWND fxDlg, FX *fx);

/*
 * Global variables
 */
_TUCHAR FxDlg_delayTimeLcdStrings[128][5];

/*
 * Unit procedures
 */
static void CALLBACK fd_ChangeParameter(UINT parameter, int value, CPSEND send);
static BOOL CALLBACK fd_DlgProc(HWND fxDlg, UINT message, WPARAM wParam, LPARAM lParam);
static _TUCHAR *fd_GetChordNoteText(UINT ctrlID);
static void fd_InitControlValues(HWND fxDlg);
static void fd_KybdOnLButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y
        , UINT keyFlags);
static LRESULT CALLBACK fd_KybdProc(HWND kybdCtrl, UINT message, WPARAM wParam, LPARAM lParam);
static void fd_OnActivate(HWND fxDlg, UINT state, HWND otherWnd, BOOL minimized);
static void fd_OnCommand(HWND fxDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void fd_OnDestroy(HWND fxDlg);
static void fd_OnDrawItem(HWND fxDlg, const DRAWITEMSTRUCT *drawItem);
static BOOL fd_OnInitDialog(HWND fxDlg, HWND focusCtrl, LPARAM lParam);
static void fd_OnKey(HWND fxDlg, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void fd_PushChordBtn(HWND fxDlg, UINT ctrlID);
static void CALLBACK fd_Redo(HWND fxDlg, CHANGE *change);
static void CALLBACK fd_Undo(HWND fxDlg, CHANGE *change);
static void fd_UpdateNoteLcds(HWND fxDlg);

/*
 * Unit constants
 */
static const _TUCHAR fd_directionBtnText[7] = {
    'I', (_TUCHAR) 0x7E, (_TUCHAR) 0x88, (_TUCHAR) 0x88, (_TUCHAR) 0x7E, 'I'
};

static const NUMLCDINIT fd_numLcdInits[] = {
/* UINT ctrlID, int maxLen, int min, int max, int offset, LCDPAGEFUNC LcdPageFunc */
    { IDC_PITCH_SHIFT,    3, 0, 48, -24, NULL },
    { IDC_DELAY_FEEDBACK, 3, 0,  7,   0, NULL },
    { IDC_EFFECT_LEVEL,   3, 0, 99,   0, NULL },
    { IDC_PAN_RANGE,      3, 0, 99,   0, NULL },
};
#define fd_numLcdInitCnt (sizeof fd_numLcdInits / sizeof fd_numLcdInits[0])


static const SPECIALLCDINIT fd_specialLcdInits[] = {
/* UINT ctrlID, int maxLen, int min, int max, _TUCHAR *strings, LCDPAGEFUNC LcdPageFunc */
    { IDC_DELAY_TIME, 5, 0, 127, &FxDlg_delayTimeLcdStrings[0][0], NULL },
};
#define fd_specialLcdInitCnt (sizeof fd_specialLcdInits / sizeof fd_specialLcdInits[0])

static UINT fd_chordBtns[] = {
    IDC_CHORD_C3,
    IDC_CHORD_CSH3,
    IDC_CHORD_D3,
    IDC_CHORD_DSH3,
    IDC_CHORD_E3,
    IDC_CHORD_F3,
    IDC_CHORD_FSH3,
    IDC_CHORD_G3,
    IDC_CHORD_GSH3,
    IDC_CHORD_A3,
    IDC_CHORD_ASH3,
    IDC_CHORD_B3
};
#define fd_chordBtnCnt (sizeof fd_chordBtns / sizeof fd_chordBtns[0])

/*
 * Unit variables
 */
static HWND fd_parentWnd;
static BYTE *fd_data;
static HWND fd_kybdCtrl;
static WNDPROC fd_origKybdProc;
static UINT fd_curChord;
static BOOL fd_dirty;
static HMENU fd_menu;
static UNDO *fd_undo;
static BOOL fd_undoGroup;
static BOOL fd_undoFlag;
static KEYNAV fd_nav[] = {
    /* ctrlID, left, up, right, down */
    { IDC_DELAY_TIME,   0,              0,                0,              IDC_PITCH_SHIFT  },
    { IDC_PITCH_SHIFT,  0,              IDC_DELAY_TIME,   0,              IDC_DELAY_FEEDBACK },
    { IDC_DELAY_FEEDBACK, 0,            IDC_PITCH_SHIFT,  0,              IDC_EFFECT_LEVEL },
    { IDC_EFFECT_LEVEL, 0,              IDC_DELAY_FEEDBACK, 0,            IDC_PAN_RANGE    },
    { IDC_PAN_SELECT,   0,              IDC_EFFECT_LEVEL, IDC_PAN_DIR,    IDC_CHORD_C3     },
    { IDC_PAN_DIR,      IDC_PAN_SELECT, IDC_EFFECT_LEVEL, IDC_PAN_RANGE,  IDC_CHORD_DSH3   },
    { IDC_PAN_RANGE,    IDC_PAN_DIR,    IDC_EFFECT_LEVEL, 0,              IDC_CHORD_B3     },
    { IDC_CHORD_C3,     0,              IDC_PAN_SELECT,   IDC_CHORD_CSH3, 0                },
    { IDC_CHORD_CSH3,   IDC_CHORD_C3,   IDC_PAN_SELECT,   IDC_CHORD_D3,   0                },
    { IDC_CHORD_D3,     IDC_CHORD_CSH3, IDC_PAN_DIR,      IDC_CHORD_DSH3, 0                },
    { IDC_CHORD_DSH3,   IDC_CHORD_D3,   IDC_PAN_DIR,      IDC_CHORD_E3,   0                },
    { IDC_CHORD_E3,     IDC_CHORD_DSH3, IDC_PAN_DIR,      IDC_CHORD_F3,   0                },
    { IDC_CHORD_F3,     IDC_CHORD_E3,   IDC_PAN_RANGE,    IDC_CHORD_FSH3, 0                },
    { IDC_CHORD_FSH3,   IDC_CHORD_F3,   IDC_PAN_RANGE,    IDC_CHORD_G3,   0                },
    { IDC_CHORD_G3,     IDC_CHORD_FSH3, IDC_PAN_RANGE,    IDC_CHORD_GSH3, 0                },
    { IDC_CHORD_GSH3,   IDC_CHORD_G3,   IDC_PAN_RANGE,    IDC_CHORD_A3,   0                },
    { IDC_CHORD_A3,     IDC_CHORD_GSH3, IDC_PAN_RANGE,    IDC_CHORD_ASH3, 0                },
    { IDC_CHORD_ASH3,   IDC_CHORD_A3,   IDC_PAN_RANGE,    IDC_CHORD_B3,   0                },
    { IDC_CHORD_B3,     IDC_CHORD_ASH3, IDC_PAN_RANGE,    0,              0                },
};
#define fd_navCnt  ARRAYSIZE(fd_nav)


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL FxDlg_Create(HWND parentWnd, FX *fx)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_fxDlg) {
        FxDlg_Update(Prog_fxDlg, fx);
        if (IsIconic(Prog_fxDlg)) {
            OpenIcon(Prog_fxDlg);
        }
        BringWindowToTop(Prog_fxDlg);
    } else {
        fd_parentWnd = parentWnd;
        fd_data = fx->data;
        Prog_fxDlg = CreateDialogParam(Prog_instance, (LPCTSTR) IDD_FXDLG
                , HWND_DESKTOP, fd_DlgProc, 0);
        if (!Prog_fxDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog.
 */
void FxDlg_Update(HWND fxDlg, FX *fx)
{
    if (fx) {
        fd_data = fx->data;
        fd_InitControlValues(fxDlg);
    }
}
/*
 * ChangeParameter()
 */
void CALLBACK fd_ChangeParameter(UINT parameter, int value, CPSEND send)
{
#ifdef NO_REDUNDANT_SENDS
    if (value == fd_data[parameter]) {
        return;
    }
#endif
    /*
     * Add the change to the undo list.
     */
    if (!fd_undoFlag) {
        int oldValue = fd_data[parameter];

        Undo_AddChange(fd_undo, parameter + FX_ID_OFFSET, sizeof(int)
                , &oldValue, &value, fd_undoGroup);
        fd_undoGroup = FALSE;
        MenuItem_Enable(fd_menu, IDM_UNDO);
        MenuItem_Disable(fd_menu, IDM_REDO);
    }
    /*
     * Update the snapshot buffer.
     */
    fd_data[parameter] = value;
    /*
     * Update the unit - the manual says that parameter 4 is the pan direction
     * and 5 is pan select, which is right for parameter changes, but the two
     * are reversed in the dumps, so I'm reversing them in the headers to be
     * consistent with my convention of numbering everything in relation to the
     * dump and unreversing them here for the parameter changes.
     */
    if (send) {
        if (parameter == TX81Z_FX_PAN_DIR) {
            parameter = TX81Z_FX_PAN_SELECT;
        } else if (parameter == TX81Z_FX_PAN_SELECT) {
            parameter = TX81Z_FX_PAN_DIR;
        }
        TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_FX, parameter, value);
    }
    /*
     * Notify the main window of the modification status if it hasn't already
     * been done.
     */
    if (!fd_dirty) {
        SendNotification(Prog_mainWnd, IDD_FXDLG, Prog_fxDlg, EDN_CHANGE);
        fd_dirty = TRUE;
    }
}

/*
 * DlgProc
 */
BOOL CALLBACK fd_DlgProc(HWND fxDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(fxDlg, WM_ACTIVATE, fd_OnActivate);
        HANDLE_MSG(fxDlg, WM_COMMAND, fd_OnCommand);
        HANDLE_MSG(fxDlg, WM_DESTROY, fd_OnDestroy);
        HANDLE_MSG(fxDlg, WM_DRAWITEM, fd_OnDrawItem);
        HANDLE_MSG(fxDlg, WM_INITDIALOG, fd_OnInitDialog);
        HANDLE_MSG(fxDlg, WM_KEYDOWN, fd_OnKey);
        case EDM_REFRESH:
            fd_InitControlValues(fxDlg);
            break;
        case EDM_SAVED:
            fd_dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            fd_undoGroup = TRUE;
            break;
    }
    return FALSE;
}

/*
 * GetChordNoteText()
 */
_TUCHAR *fd_GetChordNoteText(UINT ctrlID)
{
    static _TUCHAR noteText[4] = { 0 }; /* return value */
    int index = ctrlID - CHORD_NOTE_ID_OFFSET;
    int note = fd_data[index];

    if (note == 49) {
        noteText[0] = ' ';
        noteText[1] = '*';
        noteText[2] = ' ';
    } else {
        int key = note + 36 + ((index - 7) >> 2);

        memcpy(noteText, Prog_keyNameStrings[key], 3 * sizeof(_TUCHAR));
    }
    return noteText;
}

/*
 * InitControlValues() - Set editor controls via the snapshot data.
 */
void fd_InitControlValues(HWND fxDlg)
{
    BYTE *data = fd_data;
    UINT ctrlID;
    HWND ctrl;
    int i;

    /*
     * Init chord note LCD's
     */
    for (i = IDC_CHORD_C3_N1; i <= IDC_CHORD_B3_N4; i++) {
        LcdCtrl_SetText(GetDlgItem(fxDlg, i), fd_GetChordNoteText(i));
    }

    /*
     * Init numeric LCD's.
     */
    for (i = 0; i < fd_numLcdInitCnt; i++) {
        ctrlID = fd_numLcdInits[i].ctrlID;
        ctrl = GetDlgItem(fxDlg, ctrlID);
        LcdCtrl_SetValue(ctrl, data[ctrlID - FX_ID_OFFSET]);
    }
    /*
     * Init special LCD's
     */
    for (i = 0; i < fd_specialLcdInitCnt; i++) {
        ctrlID = fd_specialLcdInits[i].ctrlID;
        ctrl = GetDlgItem(fxDlg, ctrlID);
        LcdCtrl_SetValue(ctrl, data[ctrlID - FX_ID_OFFSET]);
    }
    /*
     * Init pan select combo box.
     */
    ComboBox_SetCurSel(GetDlgItem(fxDlg, IDC_PAN_SELECT)
            , data[TX81Z_FX_PAN_SELECT]);
    /*
     * Init pan direction button.
     */
    SetWindowLong(GetDlgItem(fxDlg, IDC_PAN_DIR), GWL_USERDATA
            , data[TX81Z_FX_PAN_DIR]);
    /*
     * Init chord button note displays.
     */
    for (i = 0; i < fd_chordBtnCnt; i++) {
        if (Button_IsChecked(GetDlgItem(fxDlg, fd_chordBtns[i]))) {
            fd_PushChordBtn(fxDlg, fd_chordBtns[i]);
            break;
        }
    }
    /*
     * Reset the change notification flag.
     */
    fd_dirty = FALSE;
    /*
     * Reset the undo state.
     */
    Undo_Clear(fd_undo);
    fd_undoGroup = TRUE;
    MenuItem_Disable(fd_menu, IDM_UNDO);
    MenuItem_Disable(fd_menu, IDM_REDO);
}

/*
 * KybdOnLButtonDown()
 */
void fd_KybdOnLButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y
        , UINT keyFlags)
{
    int key;

    /*
     * Push/release the key, then fill in the chord with the pressed keys
     * from bottom to top.
     */
    if ((key = KybdCtrl_HitTest(kybdCtrl, x, y, NULL)) == -1) {
        return;
    }
    if (KybdCtrl_IsKeyDown(kybdCtrl, key)) {
        KybdCtrl_ReleaseKey(kybdCtrl, key, 1);
    } else {
        int i;

        /*
         * Check to see if the currently selected chord is full.
         */
        for (i = 0; i < 4; i++) {
            int index = fd_curChord - CHORD_BTN_ID_OFFSET + i;

            if (fd_data[index] == 49) {
                goto FoundFreeNote;
            }
        }
        return;
FoundFreeNote:
        KybdCtrl_PushKey(kybdCtrl, key, 1);
    }
    fd_UpdateNoteLcds(Prog_fxDlg);
    InvalidateRect(kybdCtrl, NULL, FALSE);
}

/*
 * KybdProc() - Keyboard control window procedure
 */
LRESULT CALLBACK fd_KybdProc(HWND kybdCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(kybdCtrl, WM_LBUTTONDOWN, fd_KybdOnLButtonDown);
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            return 0;
        case WM_MOUSEMOVE:
            /*
             * Forward the message without the keyFlags informations so the
             * cursor changes occur, but not mouse dragging.
             */
            wParam = 0;
            break;
    }
    return CallWindowProc(fd_origKybdProc, kybdCtrl, message, wParam, lParam);
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void fd_OnActivate(HWND fxDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = fxDlg;
}

/*
 * OnCommand - handles OK and X buttons
 */
void fd_OnCommand(HWND fxDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    int value;

    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_RETRIEVE_FX:
            TX81Z_RetrieveData(Prog_midi, REQ_FX, &Prog_snapshot);
            SendNotification(Prog_mainWnd, IDD_FXDLG, fxDlg, EDN_CHANGE);
            fd_InitControlValues(fxDlg);
            return;
        case IDM_TRANSMIT_FX:
            TX81Z_SendData(Prog_midi, REQ_FX, &Prog_snapshot);
            return;
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(fxDlg);
            return;
        case IDM_UNDO:
            Undo_Undo(fd_undo, fd_Undo, fxDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(fd_undo, fd_Redo, fxDlg);
UpdateUndoMenus:
            EnableMenuItem(fd_menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(fd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(fd_menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(fd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            return;
        case IDM_KYBDDLG:
        case IDM_REMOTEWND:
        case IDM_MAINWND:
        case IDM_VOICEDLG:
        case IDM_PFMDLG:
        case IDM_PCDLG:
        case IDM_MTODLG:
        case IDM_MTFDLG:
        case IDM_SYSDLG:
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_HELP:
            Prog_OpenHelp(fxDlg, _T("res/effects_editor.html"));
            return;
        /*
         * Controls
         */
        case IDC_PAN_DIR:
            if (notify == BN_CLICKED) {
                value = fd_data[TX81Z_FX_PAN_DIR];
                value = !value;
                SetWindowLong(ctrl, GWL_USERDATA, value);
                InvalidateRect(ctrl, NULL, FALSE);
            } else {
                return;
            }
            break;
        case IDC_PAN_SELECT:
            if (notify == CBN_SELCHANGE) {
                value = ComboBox_GetCurSel(ctrl);
            } else {
                return;
            }
            break;
        case IDC_CHORD_C3:
        case IDC_CHORD_CSH3:
        case IDC_CHORD_D3:
        case IDC_CHORD_DSH3:
        case IDC_CHORD_E3:
        case IDC_CHORD_F3:
        case IDC_CHORD_FSH3:
        case IDC_CHORD_G3:
        case IDC_CHORD_GSH3:
        case IDC_CHORD_A3:
        case IDC_CHORD_ASH3:
        case IDC_CHORD_B3:
            fd_PushChordBtn(fxDlg, ctrlID);
            return;
        default:
            if (notify == LCN_SELCHANGE) {
                value = LcdCtrl_GetValue(ctrl);
            } else {
                return;
            }
            break;
    }
    fd_ChangeParameter(ctrlID - FX_ID_OFFSET, value, CP_SEND);
}

/*
 * OnDestroy()
 */
void fd_OnDestroy(HWND fxDlg)
{
    Undo_Destroy(fd_undo);
    Prog_fxDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnDrawItem() - Draws the pan direction button.
 */
void fd_OnDrawItem(HWND fxDlg, const DRAWITEMSTRUCT *drawItem)
{
    RECT rect = drawItem->rcItem;
    int textX = Rect_HCenter(&rect) - 10;
    int textY = 7;
    int direction = fd_data[TX81Z_FX_PAN_DIR];

    if (drawItem->itemState & ODS_SELECTED) {
        DrawEdge(drawItem->hDC, &rect, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
        MiniFont_DrawString(drawItem->hDC, textX + 1, textY + 1
                , &fd_directionBtnText[direction * 3], 3, Prog_wndTextColor);
    } else {
        DrawEdge(drawItem->hDC, &rect, EDGE_RAISED, BF_RECT | BF_MIDDLE);
        MiniFont_DrawString(drawItem->hDC, textX, textY
                , &fd_directionBtnText[direction * 3], 3, Prog_wndTextColor);
    }
    if (drawItem->itemState & ODS_FOCUS) {
        InflateRect(&rect, -3, -3);
        rect.left++;
        rect.top++;
        DrawFocusRect(drawItem->hDC, &rect);
    }
}

/*
 * OnInitDialog - 
 */
BOOL fd_OnInitDialog(HWND fxDlg, HWND focusCtrl, LPARAM lParam)
{
    UINT ctrlID;
    HWND combo = GetDlgItem(fxDlg, IDC_PAN_SELECT);
    int i;

    /*
     * Set up the window.
     */
    Window_Center(fxDlg, fd_parentWnd);
    EnumChildWindows(fxDlg, Prog_SetBoldFont, 0);
    /*
     * Configure chord note LCD's
     */
    for (i = IDC_CHORD_C3_N1; i <= IDC_CHORD_B3_N4; i++) {
        LcdCtrl_TextInit(GetDlgItem(fxDlg, i), 3);
    }
    /*
     * Configure numeric LCD's.
     */
    for (i = 0; i < fd_numLcdInitCnt; i++) {
        ctrlID = fd_numLcdInits[i].ctrlID;
        LcdCtrl_NumInit(GetDlgItem(fxDlg, ctrlID), &fd_numLcdInits[i]);
    }
    /*
     * Configure special LCD's
     */
    for (i = 0; i < fd_specialLcdInitCnt; i++) {
        ctrlID = fd_specialLcdInits[i].ctrlID;
        LcdCtrl_SpecialInit(GetDlgItem(fxDlg, ctrlID), &fd_specialLcdInits[i]);
    }
    /*
     * Effect select combo
     */
    ComboBox_AddString(combo, _T("LFO"));
    ComboBox_AddString(combo, _T("velocity"));
    ComboBox_AddString(combo, _T("note"));
    /*
     * Keyboard control
     */
    fd_kybdCtrl = GetDlgItem(fxDlg, IDC_KYBDCTRL);
    KybdCtrl_SetRange(fd_kybdCtrl, 36, 60);
    fd_origKybdProc = SubclassWindow(fd_kybdCtrl, fd_KybdProc);
    /*
     * Chord buttons
     */
    Button_SetCheck(GetDlgItem(fxDlg, IDC_CHORD_C3), BST_CHECKED);
    fd_curChord = IDC_CHORD_C3;
    /*
     * Subclass controls for keyboard navigation.
     */
    EnumChildWindows(fxDlg, KeyNav_SubclassControls, 0);
    /*
     * Set up the undo infrastructure (must be done before calling
     * InitControlValues).
     */
    fd_menu = GetMenu(fxDlg);
    fd_undo = Undo_Create();
    /*
     * Initialize control settings.
     */
    fd_InitControlValues(fxDlg);
    /*
     * Show the window.
     */
    ShowWindow(fxDlg, SW_SHOW);

    return TRUE;
}

/*
 * OnKey() - Handles fancy keyboard navigation.
 */
void fd_OnKey(HWND fxDlg, UINT vk, BOOL down, int repeat, UINT keyFlags)
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
            destCtrl = KeyNav_FindAdjacentCtrl(fxDlg, fd_nav, fd_navCnt, NULL
                    , 0, focusID, KN_LEFT, KN_RIGHT, &navIndex, TRUE);
            break;
        case VK_UP:
            destCtrl = KeyNav_FindAdjacentCtrl(fxDlg, fd_nav, fd_navCnt, NULL
                    , 0, focusID, KN_UP, KN_DOWN, &navIndex, TRUE);
            break;
        case VK_RIGHT:
            destCtrl = KeyNav_FindAdjacentCtrl(fxDlg, fd_nav, fd_navCnt, NULL
                    , 0, focusID, KN_RIGHT, KN_LEFT, &navIndex, TRUE);
            break;
        case VK_DOWN:
            destCtrl = KeyNav_FindAdjacentCtrl(fxDlg, fd_nav, fd_navCnt, NULL
                    , 0, focusID, KN_DOWN, KN_UP, &navIndex, TRUE);
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
 * PushChordBtn() - Updates and adjusts the keyboard control as the chord
 *                  buttons are pressed.
 */
void fd_PushChordBtn(HWND fxDlg, UINT ctrlID)
{
    int i;
    int firstKey = 36 + ((ctrlID - IDC_CHORD_C3) >> 2);
    int index;

    for (i = 0; i < 12; i++) {
        if (fd_chordBtns[i] != ctrlID) {
            Button_Uncheck(GetDlgItem(fxDlg, fd_chordBtns[i]));
        }
    }
    Button_Check(GetDlgItem(fxDlg, ctrlID));
    KybdCtrl_ReleaseAllKeys(fd_kybdCtrl);
    for (i = 0; i < 4; i++) {
        index = ctrlID - CHORD_BTN_ID_OFFSET + i;
        if (fd_data[index] != 49) {
            KybdCtrl_PushKey(fd_kybdCtrl, fd_data[index] + firstKey, 1);
        }
    }
    fd_curChord = ctrlID;
    KybdCtrl_EnableRange(fd_kybdCtrl, 36, firstKey - 1, FALSE);
    KybdCtrl_EnableRange(fd_kybdCtrl, firstKey, firstKey + 48, TRUE);
    KybdCtrl_EnableRange(fd_kybdCtrl, firstKey + 49, 96, FALSE);
    InvalidateRect(fd_kybdCtrl, NULL, FALSE);
}

/*
 * Redo() - Redoes a previously undone edit.
 */
void CALLBACK fd_Redo(HWND fxDlg, CHANGE *change)
{
    UINT ctrlID = change->ctrlID;
    HWND ctrl = GetDlgItem(fxDlg, ctrlID);

    fd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    if (ctrlID == IDC_PAN_DIR) {
        if (GetWindowLong(ctrl, GWL_USERDATA)) {
            Button_Click(ctrl);
        }
        fd_ChangeParameter(TX81Z_FX_PAN_DIR, *(int *) change->newValue
                , CP_SEND);
        InvalidateRect(GetDlgItem(fxDlg, IDC_PAN_DIR), NULL, FALSE);
    } else if (ctrlID >= IDC_CHORD_C3_N1 && ctrlID <= IDC_CHORD_B3_N4) {
        /*
         * Update the keyboard control.
         */
        HWND kybdCtrl = GetDlgItem(fxDlg, IDC_KYBDCTRL);
        UINT chordBtnID = ((ctrlID - CHORD_NOTE_ID_OFFSET - 7) & 0xFFFFFFFC)
            + 7 + CHORD_BTN_ID_OFFSET;
        int firstKey;
        int oldKey = *(int *) change->oldValue;
        int newKey = *(int *) change->newValue;

        /*
         * Make sure the right chord button is pressed.
         */
        if (!Button_IsChecked(GetDlgItem(fxDlg, chordBtnID))) {
            fd_PushChordBtn(fxDlg, chordBtnID);
        }
        firstKey = 36 + ((fd_curChord - IDC_CHORD_C3) >> 2);
        /*
         * Turn off the old key and turn on the new.
         */
        if (oldKey < 49) {
            KybdCtrl_ReleaseKey(kybdCtrl, oldKey + firstKey, 1);
        }
        if (newKey < 49) {
            KybdCtrl_PushKey(kybdCtrl, newKey + firstKey, 1);
        }
        fd_ChangeParameter(ctrlID - FX_ID_OFFSET, newKey, CP_SEND);
        /*
         * The only way to know if this is the last change in the set of undo
         * changes is to check the internal pointer to see if it's at the end
         * of the undo list, or check the node's groupStart flag.
         */
        if (fd_undo->ptr == &fd_undo->head || fd_undo->ptr->change.groupStart)
        {
            fd_UpdateNoteLcds(fxDlg);
            InvalidateRect(kybdCtrl, NULL, FALSE);
        }
    } else {
        Prog_ChangeControl(ctrl, ctrlID - FX_ID_OFFSET
                , change->newValue, CP_SEND, fd_ChangeParameter);
    }
    fd_undoFlag = FALSE;
}

/*
 * Undo() - Undoes an edit done to a control.
 */
void CALLBACK fd_Undo(HWND fxDlg, CHANGE *change)
{
    UINT ctrlID = change->ctrlID;
    HWND ctrl = GetDlgItem(fxDlg, ctrlID);

    fd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    if (ctrlID == IDC_PAN_DIR) {
        if (GetWindowLong(ctrl, GWL_USERDATA)) {
            Button_Click(ctrl);
        }
        fd_ChangeParameter(ctrlID - FX_ID_OFFSET, *(int *) change->oldValue
                , CP_SEND);
        InvalidateRect(GetDlgItem(fxDlg, IDC_PAN_DIR), NULL, FALSE);
    } else if (ctrlID >= IDC_CHORD_C3_N1 && ctrlID <= IDC_CHORD_B3_N4) {
        /*
         * Update the keyboard control.
         */
        HWND kybdCtrl = GetDlgItem(fxDlg, IDC_KYBDCTRL);
        UINT chordBtnID = ((ctrlID - CHORD_NOTE_ID_OFFSET - 7) & 0xFFFFFFFC)
            + 7 + CHORD_BTN_ID_OFFSET;
        int firstKey;
        int oldKey = *(int *) change->oldValue;
        int newKey = *(int *) change->newValue;

        /*
         * Make sure the right chord button is pressed.
         */
        if (!Button_IsChecked(GetDlgItem(fxDlg, chordBtnID))) {
            fd_PushChordBtn(fxDlg, chordBtnID);
        }
        firstKey = 36 + ((fd_curChord - IDC_CHORD_C3) >> 2);
        /*
         * Turn off the new key and turn on the old.
         */
        if (newKey < 49) {
            KybdCtrl_ReleaseKey(kybdCtrl, newKey + firstKey, 1);
        }
        if (oldKey < 49) {
            KybdCtrl_PushKey(kybdCtrl, oldKey + firstKey, 1);
        }
        fd_ChangeParameter(ctrlID - FX_ID_OFFSET, oldKey, CP_SEND);
        if (change->groupStart) {
            fd_UpdateNoteLcds(fxDlg);
            InvalidateRect(kybdCtrl, NULL, FALSE);
        }
    } else {
        Prog_ChangeControl(ctrl, ctrlID - FX_ID_OFFSET, change->oldValue
                , CP_SEND, fd_ChangeParameter);
    }
    fd_undoFlag = FALSE;
}


/*
 * UpdateNoteLcds() - Update note LCD's.
 */
void fd_UpdateNoteLcds(HWND fxDlg)
{
    HWND kybdCtrl = GetDlgItem(fxDlg, IDC_KYBDCTRL);
    int index = fd_curChord - CHORD_BTN_ID_OFFSET;
    int i = 0;
    int key;
    HWND noteLcd;
    int value;

    for (key = 0; key <= 60; key++) {
        if (KybdCtrl_IsKeyDown(kybdCtrl, key + 36)) {
            if (i < 4) {
                noteLcd = GetDlgItem(fxDlg, index + i + CHORD_NOTE_ID_OFFSET);
                value = key - (index >> 2) + 1;
                fd_ChangeParameter(index + i, value, CP_SEND);
                LcdCtrl_SetText(noteLcd, Prog_keyNameStrings[key + 36]);
                InvalidateRect(noteLcd, NULL, FALSE);
                i++;
            } else {
                KybdCtrl_ReleaseKey(kybdCtrl, key + 36, 1);
            }
        }
    }
    while (i < 4) {
        noteLcd = GetDlgItem(fxDlg, index + i + CHORD_NOTE_ID_OFFSET);
        fd_ChangeParameter(index + i, 49, CP_SEND);
        LcdCtrl_SetText(noteLcd, Prog_disabled);
        InvalidateRect(noteLcd, NULL, FALSE);
        i++;
    }
}

