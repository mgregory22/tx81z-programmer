/*
 * algctrl.h - Algorithm selection control
 *
 * Copyright (c) 2006 Matt Gregory
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
#ifndef ALGCTRL_H
#define ALGCTRL_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif


/*
 * AlgCtrl message constants
 */
/* wParam = 0,   lParam = 0 */
#define ACM_GETVALUE      (WM_USER + 31)
/* wParam = 0,   lParam = value */
#define ACM_SETVALUE      (WM_USER + 32)
/* notification code */
#define ACN_VALUECHANGED  1


/*
 * Message macros
 */
#define AlgCtrl_GetValue(hwnd)                ((int)SendMessage((hwnd), ACM_GETVALUE, 0, 0))
#define AlgCtrl_SetValue(hwnd, value)         ((void)SendMessage((hwnd), ACM_SETVALUE, 0, (LPARAM)(int)(value)))


/*
void _OnSetValue(HWND Wnd, int value)
*/
#define HANDLE_ACM_SETVALUE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(lParam)), 0L)
#define FORWARD_ACM_SETVALUE(hwnd, value, fn) \
    (void)(fn)((hwnd), ACM_SETVALUE, 0L, (LPARAM)(int)(value))

/*
int _OnGetValue(HWND Wnd)
*/
#define HANDLE_ACM_GETVALUE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd))
#define FORWARD_ACM_GETVALUE(hwnd, fn) \
    (int)(fn)((hwnd), ACM_GETVALUE, 0L, 0L)

/*
 * Global constants
 */
extern const _TUCHAR *AlgCtrl_className;

/*
 * Global procedures
 */
BOOL AlgCtrl_Register(void);


#endif  /* #ifndef ALGCTRL_H */
