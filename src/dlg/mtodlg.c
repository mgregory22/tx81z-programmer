/*
 * mtodlg.c - Octave micro-tunings editor
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
#include "undo.h"
#include "dlg/mtgen.h"
#include "dlg/mtodlg.h"

/*
 * Global constants
 */
const _TUCHAR *MtoDlg_className = _T("MtoDlg");

/*
 * Global procedures
 */
extern BOOL MtoDlg_Create(HWND parentWnd, MTO *mto);
extern void MtoDlg_Update(HWND mtoDlg, MTO *mto);

/*
 * Unit procedures
 */
static BOOL CALLBACK mod_DlgProc(HWND mtoDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void mod_OnActivate(HWND mtoDlg, UINT state, HWND otherWnd, BOOL minimized);
static void mod_OnCommand(HWND mtoDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void mod_OnDestroy(HWND mtoDlg);
static BOOL mod_OnInitDialog(HWND mtoDlg, HWND focusCtrl, LPARAM lParam);
static void mod_OnKey(HWND mtoDlg, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void mod_OnSize(HWND mtoDlg, UINT state, int cx, int cy);

/*
 * Unit constants
 */
#define mod_initCnt 12
#define IDC_STATIC (-1)

/*
 * Unit variables
 */
static MTDLGDATA mod_dlgData = {
    NULL,
    IDD_MTODLG,
    NULL,
    NULL,
    FALSE,
    mod_initCnt,
    TX81Z_SUBGRP_MTO,
    NULL,
    TRUE
};
static HWND mod_parentWnd;
static DIALOG *mod_dialog;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL MtoDlg_Create(HWND parentWnd, MTO *mto)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_mtoDlg) {
        MtoDlg_Update(Prog_mtoDlg, mto);
        if (IsIconic(Prog_mtoDlg)) {
            OpenIcon(Prog_mtoDlg);
        }
        BringWindowToTop(Prog_mtoDlg);
    } else {
        mod_parentWnd = parentWnd;
        mod_dlgData.data = mto->data;
        CreateDialogParam(Prog_instance, (LPCTSTR) IDD_MTODLG, HWND_DESKTOP
                , mod_DlgProc, 0);
        if (!Prog_mtoDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog.
 */
void MtoDlg_Update(HWND mtoDlg, MTO *mto)
{
    if (mto) {
        mod_dlgData.wnd = mtoDlg;
        mod_dlgData.data = mto->data;
        MTGen_InitControlValues(&mod_dlgData);
    }
}

/*
 * DlgProc()
 */
BOOL CALLBACK mod_DlgProc(HWND mtoDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(mtoDlg, WM_ACTIVATE, mod_OnActivate);
        HANDLE_MSG(mtoDlg, WM_COMMAND, mod_OnCommand);
        HANDLE_MSG(mtoDlg, WM_DESTROY, mod_OnDestroy);
        HANDLE_MSG(mtoDlg, WM_INITDIALOG, mod_OnInitDialog);
        HANDLE_MSG(mtoDlg, WM_KEYDOWN, mod_OnKey);
        HANDLE_MSG(mtoDlg, WM_SIZE, mod_OnSize);
        HANDLE_MSG(mod_dialog, WM_VSCROLL, Dialog_OnVScroll);
        case EDM_REFRESH:
            MTGen_InitControlValues(&mod_dlgData);
            break;
        case EDM_SAVED:
            mod_dlgData.dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            mod_dlgData.undoGroup = TRUE;
            break;
    }
    return FALSE;
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void mod_OnActivate(HWND mtoDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = mtoDlg;
}

/*
 * OnCommand()
 */
void mod_OnCommand(HWND mtoDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_RETRIEVE_MTO:
            TX81Z_RetrieveData(Prog_midi, REQ_MTO, &Prog_snapshot);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_MTODLG, EDN_CHANGE)
                    , (LPARAM) mtoDlg);
            MTGen_InitControlValues(&mod_dlgData);
            break;
        case IDM_TRANSMIT_MTO:
            TX81Z_SendData(Prog_midi, REQ_MTO, &Prog_snapshot);
            break;
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(mtoDlg);
            return;
        case IDM_UNDO:
            Undo_Undo(mod_dlgData.undo, MTGen_Undo, mtoDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(mod_dlgData.undo, MTGen_Redo, mtoDlg);
UpdateUndoMenus:
            EnableMenuItem(mod_dlgData.menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(mod_dlgData.undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(mod_dlgData.menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(mod_dlgData.undo)
                        ? MF_ENABLED : MF_GRAYED));
            return;
        case IDM_KYBDDLG:
        case IDM_REMOTEWND:
        case IDM_MAINWND:
        case IDM_VOICEDLG:
        case IDM_PFMDLG:
        case IDM_FXDLG:
        case IDM_PCDLG:
        case IDM_MTODLG:
        case IDM_MTFDLG:
        case IDM_SYSDLG:
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_HELP:
            Prog_OpenHelp(mtoDlg, _T("res/microtunings_editors.html"));
            return;
    }
    /*
     * Init microtunings menu items.
     */
    if (ctrlID >= IDM_EQUAL_TEMPERED && ctrlID <= IDM_1_8_TONE) {
        memcpy(&Prog_snapshot.mto.data
                , &TX81Z_initMtf[ctrlID - IDM_EQUAL_TEMPERED][120]
                , mod_initCnt << 1);
        TX81Z_SendData(Prog_midi, REQ_MTO, &Prog_snapshot);
        MTGen_InitControlValues(&mod_dlgData);
        SendNotification(Prog_mainWnd, IDD_MTODLG, mtoDlg, EDN_CHANGE);
    }
    /*
     * LCD Controls
     */
    if (ctrlID >= IDC_MT_NOTE_1 && ctrlID < IDC_MT_FREQ_1
            && notify == LCN_SELCHANGE)
    {
        int value = LcdCtrl_GetValue(ctrl);

        /*
         * If the control is a full tuning LCD.
         */
        if (ctrlID >= IDC_MT_FULL_1 && ctrlID < IDC_MT_FREQ_1) {
            MTGen_FullLcdChange(&mod_dlgData, ctrlID, value);
        /*
         * If the control is a note LCD.
         */
        } else if ((ctrlID & 1) == 0) {
            MTGen_NoteLcdChange(&mod_dlgData, ctrlID, value);
        /*
         * Else the control is a fine frequency LCD.
         */
        } else {
            MTGen_FineLcdChange(&mod_dlgData, ctrlID, value);
        }
    }
}

void mod_OnDestroy(HWND mtoDlg)
{
    GetWindowPlacement(mtoDlg, &Prog_mtoDlgPlacement);
    Undo_Destroy(mod_dlgData.undo);
    Dialog_Destroy(mod_dialog);
    mod_dialog = NULL;
    Prog_mtoDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnInitDialog()
 */
BOOL mod_OnInitDialog(HWND mtoDlg, HWND focusCtrl, LPARAM lParam)
{
    HWND ctrl;
    int i;
    SPECIALLCDINIT noteLcdInit = { IDC_MT_NOTE_1,  3, 13, 109
        , Prog_keyNameStrings[13], NULL };
    NUMLCDINIT fineLcdInit = { IDC_MT_FINE_1,  3,  0,   63, -31, NULL };
    NUMLCDINIT fullLcdInit = { IDC_MT_FULL_1,  4,  0, 6143,   0, NULL };
    _TUCHAR noteName[5];

    mod_dlgData.wnd = Prog_mtoDlg = mtoDlg;
    /*
     * Adjust the window position.
     */
    if (IsRectEmpty(&Prog_mtoDlgPlacement.rcNormalPosition)) {
        Window_Center(mtoDlg, mod_parentWnd);
        GetWindowPlacement(mtoDlg, &Prog_mtoDlgPlacement);
    }
    /*
     * Set up dialog controls.
     */
    for (i = 0; i < mod_initCnt; i++) {
        int topOffset = 34 * i;

        /*
         * Create note label.
         */
        KybdCtrl_KeyToText(i + 60, noteName);
        ctrl = CreateWindow(
                _T("Static")                           /* window class */
                , noteName                             /* caption text */
                , WS_CHILD | WS_GROUP | WS_VISIBLE     /* styles */
                    | SS_LEFT
                , 3, 7 + topOffset                    /* left, top */
                , 26, 13                               /* width, height */
                , mtoDlg                               /* parent window */
                , (HMENU) IDC_STATIC                   /* control ID */
                , Prog_instance                        /* program instance */
                , NULL                                 /* creation data */
            );
        if (!ctrl) {
            Error_LastErrorF(_T("Error creating %s note label"), noteName);
            return FALSE;
        }
        SetWindowFont(ctrl, Prog_tahomaFont, FALSE);
        /*
         * Create Note LCD.
         */
        ctrl = CreateWindow(
                _T("LcdCtrl")                          /* window class */
                , NULL                                 /* caption text */
                , WS_CHILD | WS_TABSTOP | WS_VISIBLE   /* styles */
                    | LCS_SMALL | LCS_TEXT | LCS_LEFT_SB
                , 32, topOffset                        /* left, top */
                , 324, 13                              /* width, height */
                , mtoDlg                               /* parent window */
                , (HMENU) (IDC_MT_NOTE_1 + (i << 1))   /* control ID */
                , Prog_instance                        /* program instance */
                , NULL                                 /* creation data */
            );
        if (!ctrl) {
            Error_LastErrorF(_T("Error creating %s note LCD"), noteName);
            return FALSE;
        }
        LcdCtrl_SpecialInit(ctrl, &noteLcdInit);
        KeyNav_SubclassLcdCtrl(ctrl);
        /*
         * Create fine LCD.
         */
        ctrl = CreateWindow(
                _T("LcdCtrl")                          /* window class */
                , NULL                                 /* caption text */
                , WS_CHILD | WS_TABSTOP | WS_VISIBLE   /* styles */
                    | LCS_SMALL | LCS_NUMERIC | LCS_SHOWPLUS | LCS_LEFT_SB
                , 360, topOffset                       /* left, top */
                , 143, 13                              /* width, height */
                , mtoDlg                               /* parent window */
                , (HMENU) (IDC_MT_FINE_1 + (i << 1))   /* control ID */
                , Prog_instance                        /* program instance */
                , NULL                                 /* creation data */
            );
        if (!ctrl) {
            Error_LastErrorF(_T("Error creating %s fine LCD"), noteName);
            return FALSE;
        }
        LcdCtrl_NumInit(ctrl, &fineLcdInit);
        KeyNav_SubclassLcdCtrl(ctrl);
        /*
         * Create full range LCD.
         */
        ctrl = CreateWindow(
                _T("LcdCtrl")                          /* window class */
                , NULL                                 /* caption text */
                , WS_CHILD | WS_TABSTOP | WS_VISIBLE   /* styles */
                    | LCS_SMALL | LCS_NUMERIC | LCS_LEFT_SB
                , 32, 15 + topOffset                   /* left, top */
                , 471, 13                              /* width, height */
                , mtoDlg                               /* parent window */
                , (HMENU) (IDC_MT_FULL_1 + i)          /* control ID */
                , Prog_instance                        /* program instance */
                , NULL                                 /* creation data */
            );
        if (!ctrl) {
            Error_LastErrorF(_T("Error creating %s full range LCD"), noteName);
            return FALSE;
        }
        LcdCtrl_NumInit(ctrl, &fullLcdInit);
        KeyNav_SubclassLcdCtrl(ctrl);
        /*
         * Create frequency static control.
         */
        ctrl = CreateWindow(
                _T("Static")                           /* window class */
                , NULL                                 /* caption text */
                , WS_CHILD | WS_VISIBLE | SS_RIGHT     /* styles */
                    | SS_SUNKEN
                , 508, 7 + topOffset                   /* left, top */
                , 40, 16                               /* width, height */
                , mtoDlg                               /* parent window */
                , (HMENU) (IDC_MT_FREQ_1 + i)          /* control ID */
                , Prog_instance                        /* program instance */
                , NULL                                 /* creation data */
            );
        if (!ctrl) {
            Error_LastErrorF(_T("Error creating %s frequency display"), noteName);
            return FALSE;
        }
        SetWindowFont(ctrl, Prog_tahomaFont, FALSE);
    }
    /*
     * Set up the undo infrastructure.
     */
    mod_dlgData.menu = GetMenu(mtoDlg);
    mod_dlgData.undo = Undo_Create();
    /*
     * Init control values.
     */
    MTGen_InitControlValues(&mod_dlgData);
    /*
     * Set up the scrolling dialog module.
     */
    if (!(mod_dialog = Dialog_Create(mtoDlg)))
        return FALSE;
    Dialog_UpdateScrollBars(mod_dialog);
    /*
     * Adjust and display the window.
     */
#define RC Prog_mtoDlgPlacement.rcNormalPosition
    MoveWindow(mtoDlg, RC.left, RC.top
            , RECT_W(RC) + GetSystemMetrics(SM_CXHSCROLL)
            , RECT_H(RC) + GetSystemMetrics(SM_CYVSCROLL), TRUE);
    MoveWindow(mtoDlg, RC.left, RC.top, RECT_W(RC), RECT_H(RC), TRUE);
#undef RC
    ShowWindow(mtoDlg, SW_SHOWNORMAL);

    return TRUE;
}

/*
 * OnKey()
 */
void mod_OnKey(HWND mtDlg, UINT vk, BOOL down, int repeat, UINT keyFlags)
{
    MTGen_OnKey(mtDlg, vk, down, repeat, keyFlags, mod_initCnt, mod_dialog);
}

/*
 * OnSize()
 */
void mod_OnSize(HWND mtoDlg, UINT state, int cx, int cy)
{
    HWND ctrl;
    RECT ctrlRect;
    int freqDisplayLeft = cx - 43;
    int fullSliderRight = freqDisplayLeft - 6;
    int fullSliderWidth = fullSliderRight - 32;
#define fineSliderRight fullSliderRight
    int fineSliderLeft = fineSliderRight - (fullSliderWidth - 3) * 64 / 160;
#define fineSliderWidth (fullSliderRight - fineSliderLeft)
#define noteSliderWidth (fineSliderLeft - 3 - 32)
    int i;
    HDWP defer = BeginDeferWindowPos(mod_initCnt * 4);
    int vScrollPos = mod_dialog->vScrollInfo.nPos;

    ctrl = GetDlgItem(mtoDlg, IDC_MT_FULL_1 + mod_initCnt - 1);
    Window_GetParentRelativeRect(ctrl, mtoDlg, &ctrlRect);
    if (ctrlRect.bottom > cy) {
        cx -= GetSystemMetrics(SM_CXVSCROLL);
        SetWindowLong(mtoDlg, GWL_STYLE
                , GetWindowLong(mtoDlg, GWL_STYLE) | WS_VSCROLL);
    }
    for (i = 0; i < mod_initCnt; i++) {
        int topOffset = 34 * i;

        /*
         * Reposition Note LCD.
         */
        ctrl = GetDlgItem(mtoDlg, IDC_MT_NOTE_1 + (i << 1));
        defer = DeferWindowPos(
                defer                         /* handle to internal structure */
                , ctrl                        /* handle to window to position */
                , NULL                        /* placement order handle */
                , 0, 0                        /* left, top */
                , noteSliderWidth, 13         /* width, height */
                , SWP_NOZORDER | SWP_NOMOVE   /* window positioning options */
            );
        /*
         * Reposition Fine LCD.
         */
        ctrl = GetDlgItem(mtoDlg, IDC_MT_FINE_1 + (i << 1));
        defer = DeferWindowPos(
                defer                         /* handle to internal structure */
                , ctrl                        /* handle to window to position */
                , NULL                        /* placement order handle */
                , fineSliderLeft, topOffset - vScrollPos   /* left, top */
                , fineSliderWidth, 13         /* width, height */
                , SWP_NOZORDER                /* window positioning options */
            );
        /*
         * Reposition Full LCD.
         */
        ctrl = GetDlgItem(mtoDlg, IDC_MT_FULL_1 + i);
        defer = DeferWindowPos(
                defer                         /* handle to internal structure */
                , ctrl                        /* handle to window to position */
                , NULL                        /* placement order handle */
                , 0, 0                        /* left, top */
                , fullSliderWidth, 13         /* width, height */
                , SWP_NOZORDER | SWP_NOMOVE   /* window positioning options */
            );
        /*
         * Reposition Frequency display.
         */
        ctrl = GetDlgItem(mtoDlg, IDC_MT_FREQ_1 + i);
        defer = DeferWindowPos(
                defer                         /* handle to internal structure */
                , ctrl                        /* handle to window to position */
                , NULL                        /* placement order handle */
                , freqDisplayLeft, 7 + topOffset - vScrollPos /* left, top */
                , 40, 16                      /* width, height */
                , SWP_NOZORDER | SWP_NOSIZE   /* window positioning options */
            );
    }
    EndDeferWindowPos(defer);
    if (mod_dialog) {
        mod_dialog->ctrlAreaSize.cx = cx;
        Dialog_OnSize(mod_dialog, state, cx, cy);
    }
}

