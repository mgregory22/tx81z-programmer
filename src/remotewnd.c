/*
 * remotewnd.c - TX81Z remote control module
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
#include "remotewnd.h"

/*
 * Data types
 */
typedef struct {
    HBITMAP *bmp;
    UINT btnID;
    int x;
    int y;
    int w;
    int h;
} BTNINIT;

/*
 * Global constants
 */
const _TUCHAR RemoteWnd_className[] = _T("RemoteWnd");

/*
 * Global procedures
 */
extern HWND RemoteWnd_Create(HWND parentWnd);
extern BOOL RemoteWnd_Register(void);

/*
 * Unit constants
 */
const BTNINIT btnInits[] = {
    { &Prog_resetBmp, IDC_REMOTE_RESET_BTN,              10,  8, 46, 46 },
    { &Prog_storeBmp, IDC_REMOTE_STORE_BTN,              66,  8, 46, 46 },
    { &Prog_utilityBmp, IDC_REMOTE_UTILITY_BTN,         113,  8, 46, 46 },
    { &Prog_editCmpBmp, IDC_REMOTE_EDITCMP_BTN,         160,  8, 46, 46 },
    { &Prog_playPfmBmp, IDC_REMOTE_PLAYPFM_BTN,         207,  8, 46, 46 },
    { &Prog_leftArrowBmp, IDC_REMOTE_PARAM_LEFT_BTN,    263,  8, 46, 46 },
    { &Prog_rightArrowBmp, IDC_REMOTE_PARAM_RIGHT_BTN,  310,  8, 46, 46 },
    { &Prog_decBmp, IDC_REMOTE_DEC_BTN,                 366,  8, 46, 46 },
    { &Prog_incBmp, IDC_REMOTE_INC_BTN,                 413,  8, 46, 46 },
    { &Prog_leftArrowBmp, IDC_REMOTE_VOLUME_LEFT_BTN,   470,  8, 46, 46 },
    { &Prog_rightArrowBmp, IDC_REMOTE_VOLUME_RIGHT_BTN, 517,  8, 46, 46 },
    { &Prog_cursorBmp, IDC_REMOTE_CURSOR_BTN,           573,  8, 46, 46 },
};
#define btnInitCnt (sizeof btnInits / sizeof btnInits[0])
#define REPEAT_TIMER_ID             1
#define REPEAT_TIMER_INITIAL_DELAY  300
#define REPEAT_TIMER_REPEAT_DELAY   25 

/*
 * Unit procedures
 */
static void rw_OnCommand(HWND remoteWnd, int cmdID, HWND ctrl, UINT notify);
static void rw_OnDestroy(HWND remoteWnd);
static void rw_OnDrawItem(HWND remoteWnd, const DRAWITEMSTRUCT *drawItem);
static void rw_OnSysCommand(HWND remoteWnd, UINT cmdID, int x, int y);
static void rw_StayOnTop(HWND remoteWnd);
static LRESULT CALLBACK rw_WndProc(HWND remoteWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK rw_BtnProc(HWND remoteWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
WNDPROC rw_origBtnProc;

/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
HWND RemoteWnd_Create(HWND parentWnd)
{
    HWND remoteWnd;

    if (Prog_remoteWnd) {
        if (IsIconic(Prog_remoteWnd)) {
            OpenIcon(Prog_remoteWnd);
        }
        BringWindowToTop(Prog_remoteWnd);
        remoteWnd = Prog_remoteWnd;
    } else {
        RECT wndRect = { 60, 60, 685, 120 };
        DWORD wndExStyles = WS_EX_LEFT;
        DWORD wndStyles = WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU;
        int i;

        if (!AdjustWindowRectEx(&wndRect, wndStyles, FALSE, wndExStyles)) {
            MsgBox_LastErrorF(parentWnd, _T("Error adjusting window rect"));
        }
        remoteWnd = CreateWindowEx(
                wndExStyles                               /* extended styles */
                , RemoteWnd_className                     /* window class */
                , _T("Remote")                            /* caption text */
                , wndStyles                               /* styles */
                , wndRect.left, wndRect.top               /* left, top */
                , RECT_W(wndRect), RECT_H(wndRect)        /* width, height */
                , NULL                                    /* parent window */
                , (HMENU) NULL                            /* menu */
                , Prog_instance                           /* program instance */
                , NULL                                    /* creation data */
            );
        if (!remoteWnd) {
            MsgBox_LastErrorF(parentWnd
                    , _T("Error creating remote control window"));
            return NULL;
        }
        /*
         * Add "Stay On Top" system menu item.
         */
        Window_AddSysMenuItem(remoteWnd, _T("Stay On Top"), IDM_STAY_ON_TOP
                , TRUE);
        rw_StayOnTop(remoteWnd);

        /*
         * Initialize buttons.
         */
        for (i = 0; i < btnInitCnt; i++) {
            HWND btn = CreateWindowEx(
                    0                                         /* extended styles */
                    , _T("BUTTON")                            /* window class */
                    , _T("TX81Z Remote")                      /* caption text */
                    , BS_OWNERDRAW | WS_CHILD | WS_VISIBLE    /* styles */
                    , btnInits[i].x, btnInits[i].y            /* left, top */
                    , btnInits[i].w, btnInits[i].h            /* width, height */
                    , remoteWnd                               /* parent window */
                    , (HMENU) btnInits[i].btnID               /* menu */
                    , Prog_instance                           /* program instance */
                    , NULL                                    /* creation data */
                );
            if (!btn) {
                MsgBox_LastErrorF(parentWnd
                        , _T("Error creating remote control button"));
                return NULL;
            }
            rw_origBtnProc = (WNDPROC) SetWindowLong(btn, GWL_WNDPROC
                    , (long) rw_BtnProc);
        }
        ShowWindow(remoteWnd, SW_SHOW);
    }

    return remoteWnd;
}

BOOL RemoteWnd_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) rw_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = 0;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = Prog_icon;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = Prog_wndTextBrush;
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = RemoteWnd_className;
    classInfo.hIconSm = NULL;
    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering remote control class"));
        return FALSE;
    }

    return TRUE;
}

/*
 * OnCommand - handles OK and X buttons
 */
void rw_OnCommand(HWND remoteWnd, int cmdID, HWND ctrl, UINT notify)
{
    switch (cmdID) {
        case IDC_REMOTE_RESET_BTN:
        case IDC_REMOTE_STORE_BTN:
        case IDC_REMOTE_UTILITY_BTN:
        case IDC_REMOTE_EDITCMP_BTN:
        case IDC_REMOTE_PLAYPFM_BTN:
        case IDC_REMOTE_PARAM_LEFT_BTN:
        case IDC_REMOTE_PARAM_RIGHT_BTN:
        case IDC_REMOTE_DEC_BTN:
        case IDC_REMOTE_INC_BTN:
        case IDC_REMOTE_VOLUME_LEFT_BTN:
        case IDC_REMOTE_VOLUME_RIGHT_BTN:
        case IDC_REMOTE_CURSOR_BTN:
            if (notify == BN_PUSHED) {
                TX81Z_SendRemote(Prog_midi, (BYTE) (cmdID - REMOTE_ID_OFFSET)
                        , RA_DOWN);
            } else if (notify == BN_UNPUSHED) {
                TX81Z_SendRemote(Prog_midi, (BYTE) (cmdID - REMOTE_ID_OFFSET)
                        , RA_UP);
            }
            return;
    }
}

/*
 * OnDestroy()
 */
void rw_OnDestroy(HWND remoteWnd)
{
    Prog_remoteWnd = NULL;
}

/*
 * OnDrawItem()
 */
void rw_OnDrawItem(HWND remoteWnd, const DRAWITEMSTRUCT *drawItem)
{
    HDC dC = drawItem->hDC;
    RECT rect = drawItem->rcItem;
    HDC memDC = CreateCompatibleDC(dC);
    UINT ctrlID = drawItem->CtlID;
    HWND wnd = drawItem->hwndItem;
    UINT state = drawItem->itemState;
    int leftOffset = 3;
    int topOffset = 2;
    int i;

    rect.right--;
    rect.bottom--;
    FillRect(dC, &rect, Prog_wndTextBrush);
    if (ctrlID == IDC_REMOTE_STORE_BTN) {
        if (GetWindowLong(wnd, GWL_USERDATA) == BST_CHECKED) {
            state |= ODS_SELECTED;
        }
    }
    if (state & ODS_SELECTED) {
        SelectPen(dC, Prog_3dLightPen);
        MoveToEx(dC, rect.right, rect.top, NULL);
        LineTo(dC, rect.right, rect.bottom);
        LineTo(dC, rect.left, rect.bottom);
        SelectPen(dC, Prog_3dShadowPen);
        LineTo(dC, rect.left, rect.top);
        LineTo(dC, rect.right, rect.top);
        leftOffset++;
        topOffset++;
    } else {
        SelectPen(dC, Prog_3dShadowPen);
        MoveToEx(dC, rect.right, rect.top, NULL);
        LineTo(dC, rect.right, rect.bottom);
        LineTo(dC, rect.left, rect.bottom);
        SelectPen(dC, Prog_3dLightPen);
        LineTo(dC, rect.left, rect.top);
        LineTo(dC, rect.right, rect.top);
    }
    for (i = 0; i < btnInitCnt; i++) {
        if (ctrlID == btnInits[i].btnID) {
            SelectBitmap(memDC, *btnInits[i].bmp);
            break;
        }
    }
    BitBlt(dC, rect.left + leftOffset, rect.top + topOffset, 40, 40
            , memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
}

/*
 * OnSysCommand()
 */
void rw_OnSysCommand(HWND remoteWnd, UINT cmdID, int x, int y)
{
    if (cmdID == IDM_STAY_ON_TOP) {
        Prog_remoteStayOnTop ^= TRUE;
        rw_StayOnTop(remoteWnd);
    } else {
        FORWARD_WM_SYSCOMMAND(remoteWnd, cmdID, x, y, DefWindowProc);
    }
}

/*
 * StayOnTop() - Updates the "Stay On Top" menu item and window z-order.
 */
void rw_StayOnTop(HWND remoteWnd)
{
    HMENU sysMenu = GetSystemMenu(remoteWnd, FALSE);
    if (Prog_remoteStayOnTop) {
        SetWindowPos(remoteWnd, HWND_TOPMOST, 0, 0, 0, 0
                , SWP_NOMOVE | SWP_NOSIZE);
        MenuItem_Check(sysMenu, IDM_STAY_ON_TOP);
    } else {
        SetWindowPos(remoteWnd, HWND_NOTOPMOST, 0, 0, 0, 0
                , SWP_NOMOVE | SWP_NOSIZE);
        MenuItem_Uncheck(sysMenu, IDM_STAY_ON_TOP);
    }
}

/*
 * WndProc
 */
LRESULT CALLBACK rw_WndProc(HWND remoteWnd, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(remoteWnd, WM_DESTROY, rw_OnDestroy);
        HANDLE_MSG(remoteWnd, WM_DRAWITEM, rw_OnDrawItem);
        HANDLE_MSG(remoteWnd, WM_COMMAND, rw_OnCommand);
        HANDLE_MSG(remoteWnd, WM_SYSCOMMAND, rw_OnSysCommand);
    }
    return DefWindowProc(remoteWnd, message, wParam, lParam);
}

LRESULT CALLBACK rw_BtnProc(HWND btn, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    static unsigned downBtnID = 0;
    unsigned btnID = GetDlgCtrlID(btn);
    HWND parentWnd = GetParent(btn);

    if (message == WM_LBUTTONDBLCLK) {
        return SendMessage(btn, WM_LBUTTONDOWN, wParam, lParam);
    }
    if (message == WM_LBUTTONDOWN) {
        if (btnID == IDC_REMOTE_STORE_BTN) {
            if (GetWindowLong(btn, GWL_USERDATA) == BST_CHECKED) {
                SetWindowLong(btn, GWL_USERDATA, BST_UNCHECKED);
                SendMessage(parentWnd, WM_COMMAND
                        , MAKEWPARAM(btnID, BN_UNPUSHED), (LPARAM) btn);
            } else {
                SetWindowLong(btn, GWL_USERDATA, BST_CHECKED);
                SendMessage(parentWnd, WM_COMMAND
                        , MAKEWPARAM(btnID, BN_PUSHED), (LPARAM) btn);
            }
        } else {
            SendMessage(parentWnd, WM_COMMAND, MAKEWPARAM(btnID, BN_PUSHED)
                    , (LPARAM) btn);
        }
        if (btnID >= IDC_REMOTE_PARAM_LEFT_BTN
                && btnID <= IDC_REMOTE_VOLUME_RIGHT_BTN) {
            downBtnID = btnID;
            SetTimer(btn, REPEAT_TIMER_ID, REPEAT_TIMER_INITIAL_DELAY, NULL);
        }
    } else if (message == WM_LBUTTONUP) {
        if (btnID != IDC_REMOTE_STORE_BTN) {
            SendMessage(parentWnd, WM_COMMAND, MAKEWPARAM(btnID, BN_UNPUSHED)
                    , (LPARAM) btn);
        }
        if (btnID >= IDC_REMOTE_PARAM_LEFT_BTN
                && btnID <= IDC_REMOTE_VOLUME_RIGHT_BTN) {
            KillTimer(btn, REPEAT_TIMER_ID);
            downBtnID = 0;
        }
    } else if (message == WM_TIMER) {
        SendMessage(parentWnd, WM_COMMAND, MAKEWPARAM(downBtnID, BN_PUSHED)
                , (LPARAM) btn);
        SetTimer(btn, REPEAT_TIMER_ID, REPEAT_TIMER_REPEAT_DELAY, NULL);
    }

    return CallWindowProc(rw_origBtnProc, btn, message, wParam, lParam);
}

