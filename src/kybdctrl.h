/*
 * kybdctrl.h - Musical keyboard control for sending MIDI note data.
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
#ifndef KYBDCTRL_H
#define KYBDCTRL_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * KybdCtrl messages and notifications
 */
/* wParam = WPARAM(firstKey, lastKey), lParam = fEnable */
#define KCM_ENABLERANGE      (WM_USER + 87)
/* wParam = 0, lParam = 0 - returns MAKELRESULT(key, velocity) */
#define KCM_GETLASTCHANGED   (WM_USER + 88)
/* wParam = 0, lParam = 0 - returns MAKELRESULT(firstKey, keyCnt) */
#define KCM_GETRANGE         (WM_USER + 89)
/* wParam = key, lParam = velocity */
#define KCM_PUSHKEY          (WM_USER + 90)
/* wParam = key, lParam = velocity */
#define KCM_RELEASEKEY       (WM_USER + 91)
/* wParam = 0, lParam = 0 */
#define KCM_RELEASEALLKEYS   (WM_USER + 92)
/* wParam = firstKey, lParam = KeyCnt */
#define KCM_SETRANGE         (WM_USER + 93)

/* wParam = WPARAM(control ID, KCN_KEYDOWN), lParam = HWND */
#define KCN_KEYDOWN      1
/* wParam = WPARAM(control ID, KCN_KEYUP), lParam = HWND */
#define KCN_KEYUP        2
/* wParam = WPARAM(control ID, KCN_CLOSED), lParam = HWND */
#define KCN_CLOSED       3


/*
void _OnEnableRange(HWND Wnd, int firstKey, int lastKey, BOOL enable)
 */
#define HANDLE_KCM_ENABLERANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), LOWORD(wParam), HIWORD(wParam), (lParam)), 0L)
#define FORWARD_KCM_ENABLERANGE(hwnd, firstKey, lastKey, enable, fn) \
    (fn)((hwnd), MAKEWPARAM((int)(firstKey), (int)(lastKey)), (LPARAM)(BOOL)(enable))

/*
long _OnGetRange(HWND Wnd)
 */
#define HANDLE_KCM_GETRANGE(hwnd, wParam, lParam, fn) \
    (long)(fn)(hwnd)
#define FORWARD_KCM_GETRANGE(hwnd, fn) \
    (long)(fn)((hwnd), KCM_GETRANGE, 0L, 0L)

/*
int _OnGetLastChanged(HWND Wnd)
 */
#define HANDLE_KCM_GETLASTCHANGED(hwnd, wParam, lParam, fn) \
    (DWORD)(fn)(hwnd)
#define FORWARD_KCM_GETLASTCHANGED(hwnd, fn) \
    (DWORD)(fn)((hwnd), KCM_GETLASTCHANGED, 0L, 0L)

/*
void _OnPushKey(HWND Wnd, int key, int velocity)
*/
#define HANDLE_KCM_PUSHKEY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (int)(lParam)), 0L)
#define FORWARD_KCM_PUSHKEY(hwnd, key, velocity, fn) \
    (void)(fn)((hwnd), KCM_PUSHKEY, (WPARAM)(key), (LPARAM)(velocity))

/*
void _OnReleaseKey(HWND Wnd, int key, int velocity)
*/
#define HANDLE_KCM_RELEASEKEY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (int)(lParam)), 0L)
#define FORWARD_KCM_RELEASEKEY(hwnd, key, velocity, fn) \
    (void)(fn)((hwnd), KCM_RELEASEKEY, (WPARAM)(key), (LPARAM)(velocity))

/*
void _OnReleaseAllKeys(HWND Wnd)
 */
#define HANDLE_KCM_RELEASEALLKEYS(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_KCM_RELEASEALLKEYS(hwnd, fn) \
    (void)(fn)((hwnd), KCM_RELEASEALLKEYS, 0L, 0L)

/*
void _OnSetRange(HWND Wnd, int firstKey, int keyCnt)
*/
#define HANDLE_KCM_SETRANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (int)(lParam)), 0L)
#define FORWARD_KCM_SETRANGE(hwnd, firstKey, keyCnt, fn) \
    (void)(fn)((hwnd), KCM_SETRANGE, (WPARAM)(firstKey), (LPARAM)(keyCnt))


#define KybdCtrl_EnableRange(hwnd, firstKey, lastKey, enable)   (void)SendMessage((hwnd), KCM_ENABLERANGE, MAKEWPARAM((int)(firstKey), (int)(lastKey)), (LPARAM)(BOOL)(enable))
#define KybdCtrl_GetRange(hwnd)                    (long)SendMessage((hwnd), KCM_GETRANGE, 0, 0)
#define KybdCtrl_GetLastChanged(hwnd)              (int)SendMessage((hwnd), KCM_GETLASTCHANGED, 0, 0)
#define KybdCtrl_PushKey(hwnd, key, velocity)      (void)SendMessage((hwnd), KCM_PUSHKEY, (key), (velocity))
#define KybdCtrl_ReleaseKey(hwnd, key, velocity)   (void)SendMessage((hwnd), KCM_RELEASEKEY, (key), (velocity))
#define KybdCtrl_ReleaseAllKeys(hwnd)              (void)SendMessage((hwnd), KCM_RELEASEALLKEYS, 0, 0)
#define KybdCtrl_SetRange(hwnd, firstKey, keyCnt)  (void)SendMessage((hwnd), KCM_SETRANGE, (WPARAM)(int)(firstKey), (LPARAM)(int)(keyCnt))

/*
 * Global constants
 */
extern const _TUCHAR *KybdCtrl_className;


/*
 * Global procedures
 */

int KybdCtrl_HitTest(HWND kybdCtrl, int x, int y, int *velocity);
BOOL KybdCtrl_IsKeyDown(HWND kybdCtrl, int key);
_TUCHAR *KybdCtrl_KeyToText(int key, _TUCHAR *buf);
BOOL KybdCtrl_Register(void);


#endif
