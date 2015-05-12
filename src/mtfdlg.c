/*
 * mtfdlg.c - Full micro-tunings editor
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
#include "kybdctrl.h"
#include "lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "undo.h"
#include "mtgen.h"
#include "mtfdlg.h"

/*
 * Global constants
 */
const _TUCHAR *MtfDlg_className = _T("MtfDlg");

/*
 * Global procedures
 */
extern BOOL MtfDlg_Create(HWND parentWnd, MTF *mtf);
extern void MtfDlg_Update(HWND mtfDlg, MTF *mtf);

/*
 * Unit procedures
 */
static BOOL CALLBACK mfd_DlgProc(HWND mtfDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void mfd_OnActivate(HWND mtfDlg, UINT state, HWND otherWnd, BOOL minimized);
static void mfd_OnCommand(HWND mtfDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void mfd_OnDestroy(HWND mtfDlg);
static BOOL mfd_OnInitDialog(HWND mtfDlg, HWND focusCtrl, LPARAM lParam);
static void mfd_OnKey(HWND mtfDlg, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void mfd_OnSize(HWND mtfDlg, UINT state, int cx, int cy);

/*
 * Unit constants
 */
#define mfd_initCnt 128
#define IDC_STATIC (-1)

/*
 * Unit variables
 */
static MTDLGDATA mfd_dlgData = {
    NULL,
    IDD_MTFDLG,
    NULL,
    NULL,
    FALSE,
    mfd_initCnt,
    TX81Z_SUBGRP_MTF,
    NULL,
    TRUE
};
static HWND mfd_parentWnd;
static DIALOG *mfd_dialog;

/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL MtfDlg_Create(HWND parentWnd, MTF *mtf)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_mtfDlg) {
        MtfDlg_Update(Prog_mtfDlg, mtf);
        if (IsIconic(Prog_mtfDlg)) {
            OpenIcon(Prog_mtfDlg);
        }
        BringWindowToTop(Prog_mtfDlg);
    } else {
        mfd_parentWnd = parentWnd;
        mfd_dlgData.data = mtf->data;
        CreateDialogParam(Prog_instance, (LPCTSTR) IDD_MTFDLG, HWND_DESKTOP
                , mfd_DlgProc, 0);
        if (!Prog_mtfDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog.
 */
void MtfDlg_Update(HWND mtfDlg, MTF *mtf)
{
    if (mtf) {
        mfd_dlgData.wnd = mtfDlg;
        mfd_dlgData.data = mtf->data;
        MTGen_InitControlValues(&mfd_dlgData);
    }
}

/*
 * DlgProc()
 */
BOOL CALLBACK mfd_DlgProc(HWND mtfDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(mtfDlg, WM_ACTIVATE, mfd_OnActivate);
        HANDLE_MSG(mtfDlg, WM_COMMAND, mfd_OnCommand);
        HANDLE_MSG(mtfDlg, WM_DESTROY, mfd_OnDestroy);
        HANDLE_MSG(mtfDlg, WM_INITDIALOG, mfd_OnInitDialog);
        HANDLE_MSG(mtfDlg, WM_KEYDOWN, mfd_OnKey);
        HANDLE_MSG(mtfDlg, WM_SIZE, mfd_OnSize);
        HANDLE_MSG(mfd_dialog, WM_VSCROLL, Dialog_OnVScroll);
        case EDM_REFRESH:
            MTGen_InitControlValues(&mfd_dlgData);
            break;
        case EDM_SAVED:
            mfd_dlgData.dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            mfd_dlgData.undoGroup = TRUE;
            break;
    }
    return FALSE;
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void mfd_OnActivate(HWND mtfDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = mtfDlg;
}

/*
 * OnCommand()
 */
void mfd_OnCommand(HWND mtfDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_RETRIEVE_MTF:
            TX81Z_RetrieveData(Prog_midi, REQ_MTF, &Prog_snapshot);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_MTFDLG, EDN_CHANGE)
                    , (LPARAM) mtfDlg);
            MTGen_InitControlValues(&mfd_dlgData);
            break;
        case IDM_TRANSMIT_MTF:
            TX81Z_SendData(Prog_midi, REQ_MTF, &Prog_snapshot);
            break;
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(mtfDlg);
            return;
        case IDM_UNDO:
            Undo_Undo(mfd_dlgData.undo, MTGen_Undo, mtfDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(mfd_dlgData.undo, MTGen_Redo, mtfDlg);
UpdateUndoMenus:
            EnableMenuItem(mfd_dlgData.menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(mfd_dlgData.undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(mfd_dlgData.menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(mfd_dlgData.undo)
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
            Prog_OpenHelp(mtfDlg, _T("microtunings_editors.html"));
            return;
    }
    /*
     * Init microtunings menu items.
     */
    if (ctrlID >= IDM_EQUAL_TEMPERED && ctrlID <= IDM_1_8_TONE) {
        memcpy(&Prog_snapshot.mtf.data
                , &TX81Z_initMtf[ctrlID - IDM_EQUAL_TEMPERED][0]
                , mfd_initCnt << 1);
        TX81Z_SendData(Prog_midi, REQ_MTF, &Prog_snapshot);
        MTGen_InitControlValues(&mfd_dlgData);
        SendNotification(Prog_mainWnd, IDD_MTFDLG, mtfDlg, EDN_CHANGE);
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
            MTGen_FullLcdChange(&mfd_dlgData, ctrlID, value);
        /*
         * If the control is a note LCD.
         */
        } else if ((ctrlID & 1) == 0) {
            MTGen_NoteLcdChange(&mfd_dlgData, ctrlID, value);
        /*
         * Else the control is a fine frequency LCD.
         */
        } else {
            MTGen_FineLcdChange(&mfd_dlgData, ctrlID, value);
        }
    }
}

void mfd_OnDestroy(HWND mtfDlg)
{
    GetWindowPlacement(mtfDlg, &Prog_mtfDlgPlacement);
    Undo_Destroy(mfd_dlgData.undo);
    Dialog_Destroy(mfd_dialog);
    mfd_dialog = NULL;
    Prog_mtfDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnInitDialog()
 */
BOOL mfd_OnInitDialog(HWND mtfDlg, HWND focusCtrl, LPARAM lParam)
{
    HWND ctrl;
    int i;
    SPECIALLCDINIT noteLcdInit = { IDC_MT_NOTE_1,  3, 13, 109
        , Prog_keyNameStrings[13], NULL };
    NUMLCDINIT fineLcdInit = { IDC_MT_FINE_1,  3,  0,   63, -31, NULL };
    NUMLCDINIT fullLcdInit = { IDC_MT_FULL_1,  4,  0, 6143,   0, NULL };
    _TUCHAR noteName[5];

    mfd_dlgData.wnd = Prog_mtfDlg = mtfDlg;
    /*
     * Adjust the window position.
     */
    if (IsRectEmpty(&Prog_mtfDlgPlacement.rcNormalPosition)) {
        Window_Center(mtfDlg, mfd_parentWnd);
        GetWindowPlacement(mtfDlg, &Prog_mtfDlgPlacement);
    }
    /*
     * Set up dialog controls.
     */
    for (i = 0; i < mfd_initCnt; i++) {
        int topOffset = 34 * i;

        /*
         * Create note label.
         */
        KybdCtrl_KeyToText(i, noteName);
        ctrl = CreateWindow(
                _T("Static")                           /* window class */
                , noteName                             /* caption text */
                , WS_CHILD | WS_GROUP | WS_VISIBLE     /* styles */
                    | SS_LEFT
                , 3, 7 + topOffset                    /* left, top */
                , 26, 13                               /* width, height */
                , mtfDlg                               /* parent window */
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
                , mtfDlg                               /* parent window */
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
                , mtfDlg                               /* parent window */
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
                , mtfDlg                               /* parent window */
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
                , mtfDlg                               /* parent window */
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
    mfd_dlgData.menu = GetMenu(mtfDlg);
    mfd_dlgData.undo = Undo_Create();
    /*
     * Init control values.
     */
    MTGen_InitControlValues(&mfd_dlgData);
    /*
     * Set up the scrolling dialog module.
     */
    if (!(mfd_dialog = Dialog_Create(mtfDlg)))
        return FALSE;
    Dialog_UpdateScrollBars(mfd_dialog);
    /*
     * Adjust and display the window.
     */
#define RC Prog_mtfDlgPlacement.rcNormalPosition
    MoveWindow(mtfDlg, RC.left, RC.top
            , RECT_W(RC) + GetSystemMetrics(SM_CXHSCROLL)
            , RECT_H(RC) + GetSystemMetrics(SM_CYVSCROLL), TRUE);
    MoveWindow(mtfDlg, RC.left, RC.top, RECT_W(RC), RECT_H(RC), TRUE);
#undef RC
    ShowWindow(mtfDlg, SW_SHOWNORMAL);

    return TRUE;
}

/*
 * OnKey()
 */
void mfd_OnKey(HWND mtDlg, UINT vk, BOOL down, int repeat, UINT keyFlags)
{
    MTGen_OnKey(mtDlg, vk, down, repeat, keyFlags, mfd_initCnt, mfd_dialog);
}

/*
 * OnSize()
 */
void mfd_OnSize(HWND mtfDlg, UINT state, int cx, int cy)
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
    HDWP defer = BeginDeferWindowPos(mfd_initCnt * 4);
    int vScrollPos = mfd_dialog->vScrollInfo.nPos;

    ctrl = GetDlgItem(mtfDlg, IDC_MT_FULL_1 + mfd_initCnt - 1);
    Window_GetParentRelativeRect(ctrl, mtfDlg, &ctrlRect);
    if (ctrlRect.bottom > cy) {
        cx -= GetSystemMetrics(SM_CXVSCROLL);
        SetWindowLong(mtfDlg, GWL_STYLE
                , GetWindowLong(mtfDlg, GWL_STYLE) | WS_VSCROLL);
    }
    for (i = 0; i < mfd_initCnt; i++) {
        int topOffset = 34 * i;

        /*
         * Reposition Note LCD.
         */
        ctrl = GetDlgItem(mtfDlg, IDC_MT_NOTE_1 + (i << 1));
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
        ctrl = GetDlgItem(mtfDlg, IDC_MT_FINE_1 + (i << 1));
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
        ctrl = GetDlgItem(mtfDlg, IDC_MT_FULL_1 + i);
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
        ctrl = GetDlgItem(mtfDlg, IDC_MT_FREQ_1 + i);
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
    if (mfd_dialog) {
        mfd_dialog->ctrlAreaSize.cx = cx;
        Dialog_OnSize(mfd_dialog, state, cx, cy);
    }
}

