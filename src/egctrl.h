/*
 * egctrl.h - Envelope generator display for TX81Z
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
#ifndef EGCTRL_H
#define EGCTRL_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * EGCtrl style constants
 */
#define ECS_KEYUPLABEL     0x0001

/*
 * EGCtrl message constants
 */
/* wParam = 0, lParam = 0 */
#define ECM_GETKEYUP       (WM_USER + 60)
/* wParam = 0, lParam = 0 */
#define ECM_GETLENGTH      (WM_USER + 61)
/* wParam = pointer to 5 byte receive buffer, as above
 * lParam = ptr to shift value */
#define ECM_GETPTRS        (WM_USER + 62)
/* wParam = 0,  lParam = origin */
#define ECM_SETKEYUP       (WM_USER + 63)
/* wParam = 0,  lParam = origin */
#define ECM_SETORIGIN      (WM_USER + 64)
/* wParam = pointer to 5 byte string: { AR, D1R, D1L, D2R, RR }
 * lParam = shift */
#define ECM_SETPTRS        (WM_USER + 65)
/* wParam = 0,  lParam = hide */
#define ECM_HIDELABELS     (WM_USER + 66)

/* WM_COMMAND - wParam = MAKEWPARAM(id, ECN_KEYUPCHANGED), lParam = HWND */
#define ECN_KEYUPCHANGED   1
/* WM_COMMAND - wParam = MAKEWPARAM(id, ECN_LENGTHCHANGED), lParam = HWND */
#define ECN_LENGTHCHANGED  2

/*
 * EGCtrl message crackers
 */
#define EGCtrl_GetKeyUp(hwnd) \
    (int)SendMessage((hwnd), ECM_GETKEYUP, 0L, 0L)
#define EGCtrl_GetLength(hwnd) \
    (int)SendMessage((hwnd), ECM_GETLENGTH, 0L, 0L)
#define EGCtrl_GetPtrs(hwnd, valuePtrPtr, shiftPtrPtr)  \
    (void)SendMessage((hwnd), ECM_GETPTRS, (WPARAM)(char **)(valuePtrPtr), (LPARAM)(char **)(shiftPtrPtr))
#define EGCtrl_SetKeyUp(hwnd, keyUp) \
    (void)SendMessage((hwnd), ECM_SETKEYUP, 0L, (LPARAM)(keyUp))
#define EGCtrl_SetOrigin(hwnd, origin) \
    (void)SendMessage((hwnd), ECM_SETORIGIN, 0L, (LPARAM)(origin))
#define EGCtrl_SetPtrs(hwnd, valuePtr, shiftPtr) \
    (void)SendMessage((hwnd), ECM_SETPTRS, (WPARAM)(char *)(valuePtr), (LPARAM)(char *)(shiftPtr))
#define EGCtrl_HideLabels(hwnd, hide) \
    (void)SendMessage((hwnd), ECM_HIDELABELS, 0L, (LPARAM)(BOOL)(hide))

/*
void _OnGetKeyUp(HWND Wnd)
*/
#define HANDLE_ECM_GETKEYUP(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)(hwnd)
#define FORWARD_ECM_GETKEYUP(hwnd, fn) \
    (int)(DWORD)(fn)((hwnd), ECM_GETKEYUP, 0L, 0L)

/*
void _OnGetLength(HWND Wnd)
*/
#define HANDLE_ECM_GETLENGTH(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)(hwnd)
#define FORWARD_ECM_GETLENGTH(hwnd, fn) \
    (int)(DWORD)(fn)((hwnd), ECM_GETLENGTH, 0L, 0L)

/*
void _OnGetPtrs(HWND Wnd, char **valuePtrPtr, char **shiftPtrPtr)
*/
#define HANDLE_ECM_GETPTRS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (char **)(wParam), (char **)(lParam)), 0L)
#define FORWARD_ECM_GETPTRS(hwnd, valuePtrPtr, shiftPtrPtr, fn) \
    (void)(fn)((hwnd), ECM_GETPTRS, (WPARAM)(char **)(valuePtrPtr), (LPARAM)(char **)(shiftPtrPtr))

/*
void _OnSetKeyUp(HWND Wnd, int keyUp)
*/
#define HANDLE_ECM_SETKEYUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(lParam)), 0L)
#define FORWARD_ECM_SETKEYUP(hwnd, keyUp, fn) \
    (void)(fn)((hwnd), ECM_SETKEYUP, (WPARAM) 0L, (LPARAM)(keyUp))

/*
void _OnSetOrigin(HWND Wnd, int origin)
*/
#define HANDLE_ECM_SETORIGIN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(lParam)), 0L)
#define FORWARD_ECM_SETORIGIN(hwnd, origin, fn) \
    (void)(fn)((hwnd), ECM_SETORIGIN, (WPARAM) 0L, (LPARAM)(origin))

/*
void _OnSetPtrs(HWND Wnd, char *valuePtr, char *shiftPtr)
*/
#define HANDLE_ECM_SETPTRS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (char *)(wParam), (char *)(lParam)), 0L)
#define FORWARD_ECM_SETPTRS(hwnd, valuePtr, shiftPtr, fn) \
    (void)(fn)((hwnd), ECM_SETPTRS, (WPARAM)(char *)(valuePtr), (LPARAM)(char *)(shiftPtr))

/*
void _OnHideLabels(HWND Wnd, BOOL hide)
*/
#define HANDLE_ECM_HIDELABELS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(lParam)), 0L)
#define FORWARD_ECM_HIDELABELS(hwnd, hide, fn) \
    (void)(fn)((hwnd), ECM_HIDELABELS, (WPARAM) 0L, (LPARAM)(hide))

/*
 * Global constants
 */
extern const _TUCHAR *EGCtrl_className;

/*
 * Global procedures
 */
BOOL EGCtrl_Register(void);


#endif  /* #ifndef EGCTRL_H */
