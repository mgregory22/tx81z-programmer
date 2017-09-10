/*
 * sysdlg.c - TX81Z system settings editor
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
#include "ctrl/lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "snapshot.h"
#include "tx81z.h"
#include "undo.h"
#include "dlg/sysdlg.h"

/*
 * Global procedures
 */
extern BOOL SysDlg_Create(HWND parentWnd, SYS *sys);
extern void SysDlg_Update(HWND sysDlg, SYS *sys);

/*
 * Unit constants
 */
static const _TUCHAR sd_ccPbSettings[18][4] = {
    { 'o', 'f', 'f', ' ' },
    { 'n', 'o', 'r', 'm' },
    { 'G', ' ', '1', ' ' },
    { 'G', ' ', '2', ' ' },
    { 'G', ' ', '3', ' ' },
    { 'G', ' ', '4', ' ' },
    { 'G', ' ', '5', ' ' },
    { 'G', ' ', '6', ' ' },
    { 'G', ' ', '7', ' ' },
    { 'G', ' ', '8', ' ' },
    { 'G', ' ', '9', ' ' },
    { 'G', '1', '0', ' ' },
    { 'G', '1', '1', ' ' },
    { 'G', '1', '2', ' ' },
    { 'G', '1', '3', ' ' },
    { 'G', '1', '4', ' ' },
    { 'G', '1', '5', ' ' },
    { 'G', '1', '6', ' ' }
};
static const _TUCHAR sd_pcSettings[3][3] = {
    { 'o', 'f', 'f' },
    { 'c', 'o', 'm' },
    { 'i', 'n', 'd' }
};
static const _TUCHAR sd_noteSwSettings[3][4] = {
    { 'a', 'l', 'l', ' ' },
    { 'o', 'd', 'd', ' ' },
    { 'e', 'v', 'e', 'n' }
};

/*
 * Unit procedures
 */
static void CALLBACK sd_ChangeParameter(UINT parameter, int value, CPSEND send);
static BOOL CALLBACK sd_DlgProc(HWND sysDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void sd_InitControlValues(HWND sysDlg);
static void sd_OnActivate(HWND sysDlg, UINT state, HWND otherWnd, BOOL minimized);
static void sd_OnCommand(HWND sysDlg, UINT ctrlID, HWND ctrl, UINT notify);
static void sd_OnDestroy(HWND sysDlg);
static BOOL sd_OnInitDialog(HWND sysDlg, HWND focusCtrl, LPARAM lParam);
static void sd_OnKey(HWND sysDlg, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void CALLBACK sd_Redo(HWND sysDlg, CHANGE *change);
static void CALLBACK sd_Undo(HWND sysDlg, CHANGE *change);

/*
 * Unit constants
 */
static const NUMLCDINIT sd_numLcdInits[] = {
    { IDC_MASTER_TUNE, 3, 0, 127, -64, NULL },
    { IDC_TRANS_CH,    3, 0,  15,   1, NULL },
};
#define sd_numLcdInitCnt (sizeof sd_numLcdInits / sizeof sd_numLcdInits[0])

static const SPECIALLCDINIT sd_specialLcdInits[] = {
    { IDC_RCV_CH,  3, 0, 16, Prog_rcvChSettings[0], NULL },
    { IDC_PC_SW,   3, 0,  2,      sd_pcSettings[0], NULL },
    { IDC_CC_SW,   4, 0, 17,    sd_ccPbSettings[0], NULL },
    { IDC_PB_SW,   4, 0, 17,    sd_ccPbSettings[0], NULL },
    { IDC_NOTE_SW, 4, 0,  2,  sd_noteSwSettings[0], NULL },
};
#define sd_specialLcdInitCnt (sizeof sd_specialLcdInits / sizeof sd_specialLcdInits[0])

static struct {
    unsigned ctrlID;
    const _TUCHAR *modeOn;
    const _TUCHAR *modeOff;
} sd_toggleInits[] = {
    { IDC_SYSEX_SW,   Prog_on, Prog_off },
    { IDC_MEMPROT_SW, Prog_on, Prog_off },
    { IDC_COMBINE_SW, Prog_on, Prog_off },
    { IDC_AT_SW,      Prog_on, Prog_off },
};
#define sd_toggleInitCnt (sizeof sd_toggleInits / sizeof sd_toggleInits[0])


/*
 * Unit variables
 */
static HWND sd_parentWnd;  /* always the main window */
static BYTE *sd_data;  /* pointer to the sys.data field in the snapshot */
static BOOL sd_dirty;  /* tracks whether or not a EN_CHANGE notification has
                          been sent to the main window */
static HMENU sd_menu;  /* menu bar for the system settings editor window */
static UNDO *sd_undo;  /* the undo list */
static BOOL sd_undoGroup;  /* true if the next item added to the undo list
                              should be the start of a group of related
                              changes */
static BOOL sd_undoFlag;  /* to prevent changes from getting added to the undo
                             list while undoing */
static KEYNAV sd_nav[] = {  /* keyboard navigation map */
    /* ctrlID, left, up, right, down */
    { IDC_MASTER_TUNE, 0,              0,               0,              IDC_RCV_CH      },
    { IDC_RCV_CH,      0,              IDC_MASTER_TUNE, 0,              IDC_TRANS_CH    },
    { IDC_TRANS_CH,    0,              IDC_RCV_CH,      0,              IDC_PC_SW       },
    { IDC_PC_SW,       0,              IDC_TRANS_CH,    0,              IDC_NOTE_SW     },
    { IDC_NOTE_SW,     0,              IDC_PC_SW,       0,              IDC_PB_SW       },
    { IDC_PB_SW,       0,              IDC_NOTE_SW,     0,              IDC_CC_SW       },
    { IDC_CC_SW,       0,              IDC_PB_SW,       0,              IDC_AT_SW       },
    { IDC_COMBINE_SW,  0,              IDC_CC_SW,       IDC_AT_SW,      IDC_SYSEX_SW    },
    { IDC_AT_SW,       IDC_COMBINE_SW, IDC_CC_SW,       0,              IDC_MEMPROT_SW  },
    { IDC_SYSEX_SW,    0,              IDC_COMBINE_SW,  IDC_MEMPROT_SW, IDC_STARTUP_MSG },
    { IDC_MEMPROT_SW,  IDC_SYSEX_SW,   IDC_AT_SW,       0,              IDC_STARTUP_MSG },
    { IDC_STARTUP_MSG, 0,              IDC_SYSEX_SW,    0,              0               },
};
#define sd_navCnt  ARRAYSIZE(sd_nav)


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL SysDlg_Create(HWND parentWnd, SYS *sys)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_sysDlg) {
        SysDlg_Update(Prog_sysDlg, sys);
        if (IsIconic(Prog_sysDlg)) {
            OpenIcon(Prog_sysDlg);
        }
        BringWindowToTop(Prog_sysDlg);
    } else {
        sd_parentWnd = parentWnd;
        sd_data = sys->data;
        Prog_sysDlg = CreateDialogParam(Prog_instance, (LPCTSTR) IDD_SYSDLG
                , HWND_DESKTOP, sd_DlgProc, 0);
        if (!Prog_sysDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Update() - Updates the contents of the dialog.
 */
void SysDlg_Update(HWND sysDlg, SYS *sys)
{
    if (sys) {
        sd_data = sys->data;
        sd_InitControlValues(sysDlg);
    }
}

/*
 * ChangeParameter()
 */
void CALLBACK sd_ChangeParameter(UINT parameter, int value, CPSEND send)
{
#ifdef NO_REDUNDANT_SENDS
    if (value == sd_data[parameter]) {
        return;
    }
#endif
    /*
     * Add the change to the undo list.
     */
    if (!sd_undoFlag) {
        if (parameter < TX81Z_SYS_STARTUP_MSG) {
            int oldValue = sd_data[parameter];

            Undo_AddChange(sd_undo, parameter + SYS_ID_OFFSET, sizeof(int)
                    , &oldValue, &value, sd_undoGroup);
        } else {
            char newStartupMsg[TX81Z_STARTUP_MSG_LEN];

            memcpy(newStartupMsg, &sd_data[TX81Z_SYS_STARTUP_MSG]
                    , TX81Z_STARTUP_MSG_LEN);
            newStartupMsg[parameter - TX81Z_SYS_STARTUP_MSG] = value;
            Undo_AddChange(sd_undo, IDC_STARTUP_MSG
                    , TX81Z_STARTUP_MSG_LEN
                    , &sd_data[TX81Z_SYS_STARTUP_MSG], newStartupMsg
                    , sd_undoGroup);
        }
        sd_undoGroup = FALSE;
        MenuItem_Enable(sd_menu, IDM_UNDO);
        MenuItem_Disable(sd_menu, IDM_REDO);
    }
    /*
     * Update the unit.
     */
    if (send) {
        TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_SYS, parameter, value);
        if (parameter == TX81Z_SYS_RCV_CH && value < 16) {
            Prog_midi->outChannel = value;
        }
        if (parameter == TX81Z_SYS_TRANS_CH && value < 16) {
            Prog_midi->inChannel = value;
        }
    }
    /*
     * Update the snapshot buffer.
     */
    sd_data[parameter] = value;
    /*
     * Notify the main window of the modification status if it hasn't already
     * been done.
     */
    if (!sd_dirty) {
        SendNotification(Prog_mainWnd, IDD_SYSDLG, Prog_sysDlg, EDN_CHANGE);
        sd_dirty = TRUE;
    }
}

/*
 * DlgProc
 */
BOOL CALLBACK sd_DlgProc(HWND sysDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(sysDlg, WM_ACTIVATE, sd_OnActivate);
        HANDLE_MSG(sysDlg, WM_COMMAND, sd_OnCommand);
        HANDLE_MSG(sysDlg, WM_DESTROY, sd_OnDestroy);
        HANDLE_MSG(sysDlg, WM_INITDIALOG, sd_OnInitDialog);
        HANDLE_MSG(sysDlg, WM_KEYDOWN, sd_OnKey);
        case EDM_REFRESH:
            sd_InitControlValues(sysDlg);
            break;
        case EDM_SAVED:
            sd_dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            sd_undoGroup = TRUE;
            break;
    }

    return FALSE;
}

/*
 * InitControlValues() -
 */
void sd_InitControlValues(HWND sysDlg)
{
    _TUCHAR startupMsg[16];
    UINT ctrlID;
    HWND ctrl;
    int i;
    int index;

    /*
     * Startup message LCD.
     */
    FromAnsiNCopy(startupMsg, &sd_data[TX81Z_SYS_STARTUP_MSG], 16);
    LcdCtrl_SetText(GetDlgItem(sysDlg, IDC_STARTUP_MSG), startupMsg);
    /*
     * Init numeric LCD's.
     */
    for (i = 0; i < sd_numLcdInitCnt; i++) {
        ctrlID = sd_numLcdInits[i].ctrlID;
        ctrl = GetDlgItem(sysDlg, ctrlID);
        LcdCtrl_SetValue(ctrl, sd_data[ctrlID - SYS_ID_OFFSET]);
    }
    /*
     * Init special LCD's
     */
    for (i = 0; i < sd_specialLcdInitCnt; i++) {
        ctrlID = sd_specialLcdInits[i].ctrlID;
        ctrl = GetDlgItem(sysDlg, ctrlID);
        LcdCtrl_SetValue(ctrl, sd_data[ctrlID - SYS_ID_OFFSET]);
    }
    /*
     * Toggle buttons
     */
    for (i = 0; i < sd_toggleInitCnt; i++) {
        BOOL btnOn;

        /*
         * Get the ID of the next control.
         */
        ctrlID = sd_toggleInits[i].ctrlID;
        /*
         * Get the status of the button from the snapshot data.
         */
        index = ctrlID - SYS_ID_OFFSET;
        btnOn = sd_data[index] != 0;
        /*
         * Set the checked status of the button.
         */
        ctrl = GetDlgItem(sysDlg, ctrlID);
        Button_SetCheck(ctrl, btnOn);
        Button_SetText(ctrl, btnOn ? sd_toggleInits[i].modeOn
                : sd_toggleInits[i].modeOff);
    }
    sd_dirty = FALSE;
    Undo_Clear(sd_undo);
    sd_undoGroup = TRUE;
    MenuItem_Disable(sd_menu, IDM_UNDO);
    MenuItem_Disable(sd_menu, IDM_REDO);
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void sd_OnActivate(HWND sysDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = sysDlg;
}

/*
 * OnCommand - handles OK and X buttons
 */
void sd_OnCommand(HWND sysDlg, UINT ctrlID, HWND ctrl, UINT notify)
{
    int value;

    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_RETRIEVE_SYS:
            TX81Z_RetrieveData(Prog_midi, REQ_SYS, &Prog_snapshot);
            SendMessage(Prog_mainWnd, WM_COMMAND
                    , MAKEWPARAM(IDD_SYSDLG, EDN_CHANGE)
                    , (LPARAM) sysDlg);
            sd_InitControlValues(sysDlg);
            return;
        case IDM_TRANSMIT_SYS:
            TX81Z_SendData(Prog_midi, REQ_SYS, &Prog_snapshot);
            return;
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(sysDlg);
            return;
        case IDM_UNDO:
            Undo_Undo(sd_undo, sd_Undo, sysDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(sd_undo, sd_Redo, sysDlg);
UpdateUndoMenus:
            EnableMenuItem(sd_menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(sd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(sd_menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(sd_undo)
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
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_HELP:
            Prog_OpenHelp(sysDlg, _T("res/system_editor.html"));
            return;
        /*
         * Controls
         */
        case IDC_SYSEX_SW:
        case IDC_MEMPROT_SW:
        case IDC_COMBINE_SW:
        case IDC_AT_SW:
        {
            unsigned i;

            /*
             * Find the toggleInit structure for the button.
             */
            for (i = 0; i < sd_toggleInitCnt; i++) {
                if (sd_toggleInits[i].ctrlID == ctrlID)
                    break;
            }
            /*
             * Find the checked status of the button.
             */
            value = Button_IsChecked(ctrl);
            /*
             * Toggle the text on the button the user clicked.
             */
            Button_SetText(ctrl, value ? sd_toggleInits[i].modeOn
                    : sd_toggleInits[i].modeOff);
            break;
        }
        case IDC_STARTUP_MSG:
            if (notify == LCN_EDITUPDATE) {
                int cursorPos = LcdCtrl_GetCursorPos(ctrl);

                ctrlID += cursorPos;
                value = LcdCtrl_GetChar(ctrl, cursorPos);
            } else {
                return;
            }
            break;
        default:
            if (notify == LCN_SELCHANGE) {
                value = LcdCtrl_GetValue(ctrl);
            } else {
                return;
            }
            break;
    }
    sd_ChangeParameter(ctrlID - SYS_ID_OFFSET, value, CP_SEND);
}

/*
 * OnDestroy()
 */
void sd_OnDestroy(HWND sysDlg)
{
    Undo_Destroy(sd_undo);
    Prog_sysDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnInitDialog - centers the dialog in the main window
 */
BOOL sd_OnInitDialog(HWND sysDlg, HWND focusCtrl, LPARAM lParam)
{
    UINT ctrlID;
    HWND ctrl;
    int i;

    /*
     * Set up the window.
     */
    Window_Center(sysDlg, sd_parentWnd);
    /*
     * Startup message LCD
     */
    LcdCtrl_TextInit(GetDlgItem(sysDlg, IDC_STARTUP_MSG)
            , TX81Z_STARTUP_MSG_LEN);
    /*
     * Configure numeric LCD's.
     */
    for (i = 0; i < sd_numLcdInitCnt; i++) {
        ctrlID = sd_numLcdInits[i].ctrlID;
        ctrl = GetDlgItem(sysDlg, ctrlID);
        LcdCtrl_NumInit(ctrl, &sd_numLcdInits[i]);
    }
    /*
     * Configure special LCD's
     */
    for (i = 0; i < sd_specialLcdInitCnt; i++) {
        ctrlID = sd_specialLcdInits[i].ctrlID;
        ctrl = GetDlgItem(sysDlg, ctrlID);
        LcdCtrl_SpecialInit(ctrl, &sd_specialLcdInits[i]);
    }
    /*
     * Subclass controls for keyboard navigation.
     */
    EnumChildWindows(sysDlg, KeyNav_SubclassControls, 0);
    /*
     * Set up the undo infrastructure (must be done before calling
     * InitControlValues).
     */
    sd_menu = GetMenu(sysDlg);
    sd_undo = Undo_Create();
    /*
     * Initialize control settings.
     */
    sd_InitControlValues(sysDlg);
    /*
     * Show the window.
     */
    ShowWindow(sysDlg, SW_SHOW);

    return TRUE;
}

/*
 * OnKey() - Handles fancy keyboard navigation.
 */
void sd_OnKey(HWND sysDlg, UINT vk, BOOL down, int repeat, UINT keyFlags)
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
            destCtrl = KeyNav_FindAdjacentCtrl(sysDlg, sd_nav, sd_navCnt, NULL
                    , 0, focusID, KN_LEFT, KN_RIGHT, &navIndex, TRUE);
            break;
        case VK_UP:
            destCtrl = KeyNav_FindAdjacentCtrl(sysDlg, sd_nav, sd_navCnt, NULL
                    , 0, focusID, KN_UP, KN_DOWN, &navIndex, TRUE);
            break;
        case VK_RIGHT:
            destCtrl = KeyNav_FindAdjacentCtrl(sysDlg, sd_nav, sd_navCnt, NULL
                    , 0, focusID, KN_RIGHT, KN_LEFT, &navIndex, TRUE);
            break;
        case VK_DOWN:
            destCtrl = KeyNav_FindAdjacentCtrl(sysDlg, sd_nav, sd_navCnt, NULL
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
 * Redo() - Redoes a previously undone edit.
 */
void CALLBACK sd_Redo(HWND sysDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(sysDlg, change->ctrlID);

    sd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    Prog_ChangeControl(ctrl, change->ctrlID - SYS_ID_OFFSET, change->newValue
            , CP_SEND, sd_ChangeParameter);
    sd_undoFlag = FALSE;
}

/*
 * Undo() - Undoes an edit done to a control.
 */
void CALLBACK sd_Undo(HWND sysDlg, CHANGE *change)
{
    HWND ctrl = GetDlgItem(sysDlg, change->ctrlID);

    sd_undoFlag = TRUE;
    if (change->groupStart) {
        SetFocus(ctrl);
    }
    Prog_ChangeControl(ctrl, change->ctrlID - SYS_ID_OFFSET, change->oldValue
            , CP_SEND, sd_ChangeParameter);
    sd_undoFlag = FALSE;
}

