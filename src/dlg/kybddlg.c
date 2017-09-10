/*
* kybddlg.c - Keyboard control window
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
#include "ctrl/kybdctrl.h"
#include "ctrl/lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "dlg/kybddlg.h"

/*
* Global constants
*/
const _TUCHAR *KybdDlg_className = _T("KybdDlg");

/*
* Global procedures
*/
extern BOOL KybdDlg_Create(HWND parentWnd);

/*
* Global variables
*/

/*
* Unit procedures
*/
static BOOL CALLBACK kd_DlgProc(HWND kybdDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void kd_OnCommand(HWND kybdDlg, UINT cmdID, HWND ctrl, UINT notify);
static void kd_OnDestroy(HWND kybdDlg);
static BOOL kd_OnInitDialog(HWND kybdDlg, HWND focusCtrl, LPARAM lParam);
static void kd_OnPaint(HWND kybdDlg);
static void kd_OnSysCommand(HWND kybdDlg, UINT cmdID, int x, int y);
static void kd_StayOnTop(HWND kybdDlg);

/*
* Unit variables
*/
static HWND kd_parentWnd;
static HWND kd_velLcd;
static HWND kd_midiNote;
static POINT kd_middleCPos;

/*
* Procedure definitions
*/

/*
* Create()
*/
BOOL KybdDlg_Create(HWND parentWnd)
{
    if (Prog_kybdDlg) {
        if (IsIconic(Prog_kybdDlg)) {
            OpenIcon(Prog_kybdDlg);
        }
        BringWindowToTop(Prog_kybdDlg);
    } else {
        kd_parentWnd = parentWnd;
        Prog_kybdDlg = CreateDialog(Prog_instance, (LPCTSTR) IDD_KYBDDLG
                , NULL, kd_DlgProc);
        if (!Prog_kybdDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }
    return TRUE;
}

/*
* DlgProc()
*/
BOOL CALLBACK kd_DlgProc(HWND kybdDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(kybdDlg, WM_COMMAND, kd_OnCommand);
        HANDLE_MSG(kybdDlg, WM_DESTROY, kd_OnDestroy);
        HANDLE_MSG(kybdDlg, WM_INITDIALOG, kd_OnInitDialog);
        HANDLE_MSG(kybdDlg, WM_PAINT, kd_OnPaint);
        HANDLE_MSG(kybdDlg, WM_SYSCOMMAND, kd_OnSysCommand);
    }
    return FALSE;
}

/*
* OnCommand()
*/
void kd_OnCommand(HWND kybdDlg, UINT cmdID, HWND ctrl, UINT notify)
{
    switch (cmdID) {
        case IDOK:
        case IDCANCEL:
            DestroyWindow(kybdDlg);
            break;
        /*
         * Controls
         */
        case IDC_MOD_WHEEL:
        {
            int savedChannel = Prog_midi->outChannel;

            Prog_midi->outChannel = Prog_kybdChannel;
            Midi_ModWheel(Prog_midi, LcdCtrl_GetValue(ctrl));
            Prog_midi->outChannel = savedChannel;
            break;
        }
        case IDC_KYBD_CHANNEL:
            Prog_kybdChannel = LcdCtrl_GetValue(ctrl);
            break;
        case IDC_FIXED_VEL_ENABLE:
            if (notify == BN_CLICKED) {
                Prog_fixedVelEnable = IsDlgButtonChecked(kybdDlg
                        , IDC_FIXED_VEL_ENABLE);
                EnableWindow(kd_velLcd, Prog_fixedVelEnable);
            }
            break;
        case IDC_FIXED_VEL_VALUE:
            Prog_fixedVelValue = LcdCtrl_GetValue(ctrl);
            break;
        case IDC_KYBDCTRL:
        {
            DWORD noteInfo = KybdCtrl_GetLastChanged(ctrl);
            int key = LOWORD(noteInfo);
            int velocity = Prog_fixedVelEnable ? Prog_fixedVelValue
                : HIWORD(noteInfo);
            int savedChannel = Prog_midi->outChannel;
            _TUCHAR noteNum[5];

            /*
             * Switch the channel of the main midi struct long enough to
             * transmit a note message.
             */
            Prog_midi->outChannel = Prog_kybdChannel;
            noteNum[0] = '\0';
            if (notify == KCN_KEYDOWN) {
                Midi_NoteOn(Prog_midi, key, velocity);
                _stprintf(noteNum, _T("%d"), key);
            } else if (notify == KCN_KEYUP) {
                Midi_NoteOff(Prog_midi, key);
            }
            Static_SetText(kd_midiNote, noteNum);
            Prog_midi->outChannel = savedChannel;
            break;
        }
    }
}

/*
* OnDestroy()
*/
void kd_OnDestroy(HWND kybdDlg)
{
GetWindowPlacement(kybdDlg, &Prog_kybdDlgPlacement);
Prog_kybdDlg = NULL;
}

/*
* OnInitDialog()
*/
BOOL kd_OnInitDialog(HWND kybdDlg, HWND focusCtrl, LPARAM lParam)
{
HWND ctrl;
NUMLCDINIT modWheelInit = { 0, 3, 0, 127, 0 };
NUMLCDINIT channelInit =  { 0, 2, 0,  15, 1 };
NUMLCDINIT fixedVelInit = { 0, 3, 0, 127, 0 };
RECT clientRect, kybdRect;

/*
 * Set up the window.
 */
if (IsRectEmpty(&Prog_kybdDlgPlacement.rcNormalPosition)) {
    Window_Center(kybdDlg, kd_parentWnd);
    GetWindowPlacement(kybdDlg, &Prog_kybdDlgPlacement);
}
/*
 * Add "Stay On Top" system menu item.
 */
Window_AddSysMenuItem(kybdDlg, _T("Stay On Top"), IDM_STAY_ON_TOP, TRUE);
if (Prog_kybdStayOnTop) {
    MenuItem_Check(GetSystemMenu(kybdDlg, FALSE), IDM_STAY_ON_TOP);
}

/*
 * Initialize control settings.
 */

/*
 * Mod wheel LCD
 */
ctrl = GetDlgItem(kybdDlg, IDC_MOD_WHEEL);
LcdCtrl_NumInit(ctrl, &modWheelInit);
LcdCtrl_SetValue(ctrl, 0);

/*
 * Keyboard channel LCD
 */
ctrl = GetDlgItem(kybdDlg, IDC_KYBD_CHANNEL);
LcdCtrl_NumInit(ctrl, &channelInit);
LcdCtrl_SetValue(ctrl, Prog_kybdChannel);

/*
 * Fixed velocity check box
 */
CheckDlgButton(kybdDlg, IDC_FIXED_VEL_ENABLE, Prog_fixedVelEnable);

/*
 * Fixed velocity LCD
 */
kd_velLcd = GetDlgItem(kybdDlg, IDC_FIXED_VEL_VALUE);
LcdCtrl_NumInit(kd_velLcd, &fixedVelInit);
LcdCtrl_SetValue(kd_velLcd, Prog_fixedVelValue);
EnableWindow(kd_velLcd, Prog_fixedVelEnable);

/*
 * MIDI note number display
 */
kd_midiNote = GetDlgItem(kybdDlg, IDC_MIDI_NOTE);

/*
 * Middle C marker
 */
ctrl = GetDlgItem(kybdDlg, IDC_KYBDCTRL);
Window_GetParentRelativeRect(ctrl, kybdDlg, &kybdRect);
kd_middleCPos.x = Rect_XCenter(&kybdRect) - 5;
kd_middleCPos.y = kybdRect.top - 8;

/*
 * Keyboard control
 */
GetClientRect(ctrl, &clientRect);
SetWindowPos(ctrl, NULL, 0, 0
        , clientRect.right - 1, clientRect.bottom
        , SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
KybdCtrl_SetRange(ctrl, 12, 97);

/*
 * Display the dialog.
 */
SetWindowPos(kybdDlg, Prog_kybdStayOnTop ? HWND_TOPMOST : HWND_TOP
        , PASS_RECT_FIELDS_AS_AREA(Prog_kybdDlgPlacement.rcNormalPosition)
        , SWP_SHOWWINDOW);

return TRUE;
}

/*
* OnPaint()
*/
void kd_OnPaint(HWND kybdDlg)
{
PAINTSTRUCT paint;
HDC paintDC = BeginPaint(kybdDlg, &paint);

MiniFont_DrawChar(paintDC, kd_middleCPos.x, kd_middleCPos.y, (_TUCHAR) 0x8B
            , BLACK);

    EndPaint(kybdDlg, &paint);
}

/*
 * OnSysCommand()
 */
void kd_OnSysCommand(HWND kybdDlg, UINT cmdID, int x, int y)
{
    if (cmdID == IDM_STAY_ON_TOP) {
        Prog_kybdStayOnTop ^= TRUE;
        kd_StayOnTop(kybdDlg);
    } else {
        FORWARD_WM_SYSCOMMAND(kybdDlg, cmdID, x, y, DefWindowProc);
    }
}

/*
 * StayOnTop() - Updates the "Stay On Top" menu item and window z-order.
 */
void kd_StayOnTop(HWND kybdDlg)
{
    HMENU sysMenu = GetSystemMenu(kybdDlg, FALSE);

    if (Prog_kybdStayOnTop) {
        SetWindowPos(kybdDlg, HWND_TOPMOST, 0, 0, 0, 0
                , SWP_NOMOVE | SWP_NOSIZE);
        MenuItem_Check(sysMenu, IDM_STAY_ON_TOP);
    } else {
        SetWindowPos(kybdDlg, HWND_NOTOPMOST, 0, 0, 0, 0
                , SWP_NOMOVE | SWP_NOSIZE);
        MenuItem_Uncheck(sysMenu, IDM_STAY_ON_TOP);
    }
}

