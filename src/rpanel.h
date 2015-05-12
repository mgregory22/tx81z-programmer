/*
 * rpanel.h - Radio panel control - a rectangular bank of icons that are selected like radio buttons.
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
#ifndef RPANEL_H
#define RPANEL_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * RPanel state constants
 */
#define ODS_HIGHLIGHT    0x0040

/*
 * RPanel message constants
 */
/* wParam = 0,   lParam = 0 */
#define RPM_GETMAX          (WM_USER + 100)
/* wParam = 0,   lParam = 0 */
#define RPM_GETVALUE        (WM_USER + 101)
/* wParam = highlight,  lParam = 0 */
#define RPM_HIGHLIGHT       (WM_USER + 102)
/* wParam = rows,   lParam = columns */
#define RPM_SETGEOMETRY     (WM_USER + 103)
/* wParam = value,   lParam = 0 */
#define RPM_SETVALUE        (WM_USER + 104)

/*
 * RPanel notifications
 */
#define RPN_CHANGE          101
#define RPN_SETFOCUS        105
#define RPN_KILLFOCUS       106

/*
 * RPanel message macros
 */
#define RPanel_GetMax(hwnd)                         ((int)SendMessage((hwnd), RPM_GETMAX, 0, 0))
#define RPanel_GetValue(hwnd)                       ((int)SendMessage((hwnd), RPM_GETVALUE, 0, 0))
#define RPanel_Highlight(hwnd, highlight)           ((void)SendMessage((hwnd), RPM_HIGHLIGHT, (WPARAM)(BOOL)(highlight), 0))
#define RPanel_SetGeometry(hwnd, rows, columns)     ((void)SendMessage((hwnd), RPM_SETGEOMETRY, (WPARAM)(int)(rows), (LPARAM)(int)(columns)))
#define RPanel_SetValue(hwnd, value)                ((void)SendMessage((hwnd), RPM_SETVALUE, (WPARAM)(int)(value), 0))

/*
 * RPanel message crackers
 */
/*
int _OnGetMax(HWND rPanel)
*/
#define HANDLE_RPM_GETMAX(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd))
#define FORWARD_RPM_GETMAX(hwnd, fn) \
    (int)(fn)((hwnd), RPM_GETMAX, 0L, 0L)

/*
int _OnGetValue(HWND rPanel)
*/
#define HANDLE_RPM_GETVALUE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd))
#define FORWARD_RPM_GETVALUE(hwnd, fn) \
    (int)(fn)((hwnd), RPM_GETVALUE, 0L, 0L)

/*
void _OnHighlight(HWND rPanel, BOOL highlight)
*/
#define HANDLE_RPM_HIGHLIGHT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam)), 0)
#define FORWARD_RPM_HIGHLIGHT(hwnd, highlight, fn) \
    (void)(fn)((hwnd), RPM_HIGHLIGHT, (WPARAM)(BOOL)(highlight), 0)

/*
void _OnSetGeometry(HWND rPanel, int rows, int columns)
*/
#define HANDLE_RPM_SETGEOMETRY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (int)(lParam)), 0L)
#define FORWARD_RPM_SETGEOMETRY(hwnd, value, fn) \
    (void)(fn)((hwnd), RPM_SETGEOMETRY, (LPARAM)(int)(rows), (WPARAM)(int)(columns))

/*
void _OnSetValue(HWND rPanel, int value)
*/
#define HANDLE_RPM_SETVALUE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0L)
#define FORWARD_RPM_SETVALUE(hwnd, value, fn) \
    (void)(fn)((hwnd), RPM_SETVALUE, (LPARAM)(int)(value), 0L)


/*
 * Global constants
 */
#define ODT_RPANEL 100
extern const _TUCHAR *RPanel_className;

/*
 * Global procedures
 */
BOOL RPanel_Register(void);


#endif  /* #ifndef RPANEL_H */
