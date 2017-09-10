/*
 * lcdctrl.h - LCD edit control
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
#ifndef LCDCTRL_H
#define LCDCTRL_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * Data types
 */
typedef int (*LCDPAGEFUNC)(UINT ctrlID, int dir, int value);

typedef struct {
    UINT ctrlID;
    int maxLen;
    int min;
    int max;
    int offset;
    LCDPAGEFUNC LcdPageFunc;
} NUMLCDINIT;

typedef struct {
    UINT ctrlID;
    int maxLen;
    int min;
    int max;
    const _TUCHAR *strings;  /* an array of (max - min + 1) non-null terminated
                              _TUCHAR strings of length maxLen.  Used for
                              displaying the value and interpreting keystrokes */
    LCDPAGEFUNC LcdPageFunc;
} SPECIALLCDINIT;

typedef struct TEXTLCDINIT {
    UINT ctrlID;
    int maxLen;
} TEXTLCDINIT;

/*
 * LCD style constants
 */
/* Dialog sizes TEXT: large: height: 12, width: 10ch = 77
 *                    small: height:  8, width: 10ch = 43   ==> 4 * cch + 3
 *            NUMBER: large: height: 12, width:  2ch = 18   ==> 7 * cch + 4
 *                    small: height:  8, width:  2ch = 11
 */
#define LCS_SMALL      0x0000L
#define LCS_LARGE      0x0001L
#define LCS_TEXT       0x0000L
#define LCS_NUMERIC    0x0002L
#define LCS_SHOWPLUS   0x0004L
#define LCS_NOTIFY     0x0008L
#define LCS_BOTTOM_SB  0x0010L
#define LCS_LEFT_SB    0x0020L
#define LCS_RIGHT_SB   0x0040L
#define LCS_READONLY   0x0080L

/*
 * The following is meant for testing whether or not an LCD has a scroll bar.
 * An LCD control can only have one scroll bar!
 */
#define LCS_ANY_SB  (LCS_BOTTOM_SB | LCS_LEFT_SB | LCS_RIGHT_SB)

/*
 * There are three types of LCD's: text, numeric and special, which is a text
 * LCD with a scrollbar.  The special LCD's call the LcdChangeFunc callback
 * to allow the user to change the text of the control before it is painted.
 * When the cursorPos parameter of the callback is -1, then it was signalled
 * by a SB_ENDSCROLL message.
 */

/*
 * LCD message constants
 */
/* wParam = x, lParam = y */
#define LCM_BEGINDRAG      (WM_USER + 40)

/* wParam = pos,   lParam = 0 */
#define LCM_GETCHAR        (WM_USER + 41)

/* wParam = 0,   lParam = 0 */
#define LCM_GETCURSORPOS   (WM_USER + 42)

/* wParam = lpRect,   lParam = 0 */
#define LCM_GETLCDDLGRECT  (WM_USER + 43)

/* wParam = 0,   lParam = 0 */
#define LCM_GETLENGTH      (WM_USER + 44)

/* wParam = 0,   lParam = 0 */
#define LCM_GETOFFSET      (WM_USER + 45)

/* wParam = 0,   lParam = 0 */
#define LCM_GETRANGE       (WM_USER + 46)

/* wParam = 0,   lParam = 0 */
#define LCM_GETRANGEMAX    (WM_USER + 47)

/* wParam = 0,   lParam = 0 */
#define LCM_GETRANGEMIN    (WM_USER + 48)

/* wParam = text,   lParam = 0 */
#define LCM_GETTEXT        (WM_USER + 49)

/* wParam = 0,   lParam = 0 */
#define LCM_GETVALUE       (WM_USER + 50)

/* wParam = highlight,  lParam = 0 */
#define LCM_HIGHLIGHT      (WM_USER + 51)

/* wParam = pos,   lParam = 0 */
#define LCM_SETCURSORPOS   (WM_USER + 52)

/* wParam = maxLen,   lParam = 0 */
#define LCM_SETLENGTH      (WM_USER + 53)

/* wParam = offset,   lParam = 0 */
#define LCM_SETOFFSET      (WM_USER + 54)

/* wParam = min, lParam = max */
#define LCM_SETRANGE       (WM_USER + 55)

/* wParam = strings,   lParam = 0 */
#define LCM_SETSTRINGS     (WM_USER + 56)

/* wParam = text,   lParam = 0 */
#define LCM_SETTEXT        (WM_USER + 57)

/* wParam = value,   lParam = 0 */
#define LCM_SETVALUE       (WM_USER + 58)

/*
 * Notification constants
 */
#define LCN_SELCHANGE    101
#define LCN_EDITUPDATE   102
#define LCN_EDITCHANGE   103
#define LCN_SCROLLTRACK  104
#define LCN_SETFOCUS     105
#define LCN_KILLFOCUS    106

/*
 * Scroll bar resource ID
 */
#define IDC_LCD_SCROLLBAR  99

/*
 * Message macros
 */
#define LcdCtrl_GetChar(hwnd, pos)            ((int)SendMessage((hwnd), LCM_GETCHAR, (pos), 0))
#define LcdCtrl_GetCursorPos(hwnd)            ((int)SendMessage((hwnd), LCM_GETCURSORPOS, 0, 0))
#define LcdCtrl_GetLcdDlgRect(hwnd, lpRect)   ((void)SendMessage((hwnd), LCM_GETLCDDLGRECT, (WPARAM)(lpRect), 0))
#define LcdCtrl_GetLength(hwnd)               ((int)SendMessage((hwnd), LCM_GETLENGTH, 0, 0))
#define LcdCtrl_GetOffset(hwnd)               ((int)SendMessage((hwnd), LCM_GETOFFSET, 0, 0))
#define LcdCtrl_GetRange(hwnd)                ((DWORD)SendMessage((hwnd), LCM_GETRANGE, 0, 0))
#define LcdCtrl_GetRangeMax(hwnd)             ((int)SendMessage((hwnd), LCM_GETRANGEMAX, 0, 0))
#define LcdCtrl_GetRangeMin(hwnd)             ((int)SendMessage((hwnd), LCM_GETRANGEMIN, 0, 0))
#define LcdCtrl_GetText(hwnd, text)           ((void)SendMessage((hwnd), LCM_GETTEXT, (WPARAM)(text), 0))
#define LcdCtrl_GetValue(hwnd)                ((int)SendMessage((hwnd), LCM_GETVALUE, 0, 0))
#define LcdCtrl_Highlight(hwnd, highlight)    ((void)SendMessage((hwnd), LCM_HIGHLIGHT, (WPARAM)(BOOL)(highlight), 0))
#define LcdCtrl_SetCursorPos(hwnd, pos)       ((void)SendMessage((hwnd), LCM_SETCURSORPOS, (WPARAM)(int)(pos), 0))
#define LcdCtrl_SetLength(hwnd, maxLen)       ((void)SendMessage((hwnd), LCM_SETLENGTH, (WPARAM)(int)(maxLen), 0))
#define LcdCtrl_SetOffset(hwnd, offset)       ((void)SendMessage((hwnd), LCM_SETOFFSET, (WPARAM)(int)(offset), 0))
#define LcdCtrl_SetRange(hwnd, min, max)      ((int)SendMessage((hwnd), LCM_SETRANGE, (WPARAM)(int)(min), (LPARAM)(int)(max)))
#define LcdCtrl_SetStrings(hwnd, strings)     ((void)SendMessage((hwnd), LCM_SETSTRINGS, (WPARAM)(_TUCHAR *)(strings), 0))
#define LcdCtrl_SetText(hwnd, text)           ((void)SendMessage((hwnd), LCM_SETTEXT, (WPARAM)(_TUCHAR *)(text), 0))
#define LcdCtrl_SetValue(hwnd, value)         ((void)SendMessage((hwnd), LCM_SETVALUE, (WPARAM)(int)(value), 0))

/*
void _OnBeginDrag(HWND lcdCtrl, int x, int y)
*/
#define HANDLE_LCM_BEGINDRAG(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (int)(lParam)), 0L)
#define FORWARD_LCM_BEGINDRAG(hwnd, x, y, fn) \
    (void)(fn)((hwnd), LCM_BEGINDRAG, (WPARAM)(x), (LPARAM)(y))

/*
_TUCHAR _OnGetChar(HWND lcdCtrl, int pos)
*/
#define HANDLE_LCM_GETCHAR(hwnd, wParam, lParam, fn) \
    (LRESULT)(_TUCHAR)(fn)((hwnd), (int)(wParam))
#define FORWARD_LCM_GETCHAR(hwnd, pos, fn) \
    (_TUCHAR)(fn)((hwnd), LCM_GETCURSORPOS, (WPARAM)(int)(pos), 0)

/*
int _OnGetCursorPos(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETCURSORPOS(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETCURSORPOS(hwnd, fn) \
    (int)(fn)((hwnd), LCM_GETCURSORPOS, 0, 0)

/*
void _OnGetLcdDlgRect(HWND lcdCtrl, LPRECT lpRect)
 */
#define HANDLE_LCM_GETLCDDLGRECT(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd, (LPRECT)(wParam)), 0)
#define FORWARD_LCM_GETLCDDLGRECT(hwnd, lpRect, fn) \
    (void)(fn)((hwnd), LCM_GETLCDDLGRECT, (WPARAM)(LPRECT)(lpRect), 0)

/*
int _OnGetLength(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETLENGTH(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETLENGTH(hwnd, fn) \
    (int)(fn)((hwnd), LCM_GETLENGTH, 0, 0)

/*
int _OnGetOffset(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETOFFSET(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETOFFSET(hwnd, fn) \
    (int)(fn)((hwnd), LCM_GETOFFSET, 0, 0)

/*
DWORD _OnGetRange(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETRANGE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)(hwnd)
#define FORWARD_LCM_GETRANGE(hwnd, fn) \
    (DWORD)(fn)((hwnd), LCM_GETRANGE, 0, 0)

/*
int _OnGetRangeMax(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETRANGEMAX(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETRANGEMAX(hwnd, fn) \
    (int)(fn)((hwnd), LCM_GETRANGEMAX, 0, 0)

/*
int _OnGetRangeMin(HWND lcdCtrl)
 */
#define HANDLE_LCM_GETRANGEMIN(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETRANGEMIN(hwnd, fn) \
    (int)(fn)((hwnd), LCM_GETRANGEMIN, 0, 0)

/*
void _OnGetText(HWND lcdCtrl, _TUCHAR *text)
*/
#define HANDLE_LCM_GETTEXT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (_TUCHAR *)(wParam)), 0)
#define FORWARD_LCM_GETTEXT(hwnd, text, fn) \
    (void)(fn)((hwnd), LCM_GETTEXT, (WPARAM)(_TUCHAR *)(text), 0)

/*
int _OnGetValue(HWND lcdCtrl)
*/
#define HANDLE_LCM_GETVALUE(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)(hwnd)
#define FORWARD_LCM_GETVALUE(hwnd, value, fn) \
    (int)(fn)((hwnd), LCM_GETVALUE, 0, 0)

/*
void _OnHighlight(HWND lcdCtrl, BOOL highlight)
*/
#define HANDLE_LCM_HIGHLIGHT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam)), 0)
#define FORWARD_LCM_HIGHLIGHT(hwnd, highlight, fn) \
    (void)(fn)((hwnd), LCM_HIGHLIGHT, (WPARAM)(BOOL)(highlight), 0)

/*
void _OnSetCursorPos(HWND lcdCtrl, int pos)
*/
#define HANDLE_LCM_SETCURSORPOS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0)
#define FORWARD_LCM_SETCURSORPOS(hwnd, pos, fn) \
    (void)(fn)((hwnd), LCM_SETCURSORPOS, (WPARAM)(int)(pos), 0)

/*
void _OnSetLength(HWND lcdCtrl, int maxLen)
*/
#define HANDLE_LCM_SETLENGTH(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0)
#define FORWARD_LCM_SETLENGTH(hwnd, maxLen, fn) \
    (void)(fn)((hwnd), LCM_SETLENGTH, (WPARAM)(int)(maxLen), 0)

/*
void _OnSetOffset(HWND lcdCtrl, int offset)
*/
#define HANDLE_LCM_SETOFFSET(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0)
#define FORWARD_LCM_SETOFFSET(hwnd, offset, fn) \
    (void)(fn)((hwnd), LCM_SETOFFSET, (WPARAM)(int)(offset), 0)

/*
int _OnSetRange(HWND lcdCtrl, int min, int max)
*/
#define HANDLE_LCM_SETRANGE(hwnd, wParam, lParam, fn) \
    (LRESULT)(int)(fn)((hwnd), (int)(wParam), (int)(lParam))
#define FORWARD_LCM_SETRANGE(hwnd, min, max, fn) \
    (int)(fn)((hwnd), LCM_SETRANGE, (WPARAM)(int)(min), (LPARAM)(int)(max))

/*
void _OnSetStrings(HWND lcdCtrl, _TUCHAR *strings)
*/
#define HANDLE_LCM_SETSTRINGS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (_TUCHAR *)(wParam)), 0)
#define FORWARD_LCM_SETSTRINGS(hwnd, strings, fn) \
    (void)(fn)((hwnd), LCM_SETSTRINGS, (WPARAM)(_TUCHAR *)(strings), 0)

/*
void _OnSetText(HWND lcdCtrl, _TUCHAR *text)
*/
#define HANDLE_LCM_SETTEXT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (_TUCHAR *)(wParam)), 0)
#define FORWARD_LCM_SETTEXT(hwnd, text, fn) \
    (void)(fn)((hwnd), LCM_SETTEXT, (WPARAM)(_TUCHAR *)(text), 0)

/*
void _OnSetValue(HWND lcdCtrl, int value)
*/
#define HANDLE_LCM_SETVALUE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0)
#define FORWARD_LCM_SETVALUE(hwnd, value, fn) \
    (void)(fn)((hwnd), LCM_SETVALUE, (WPARAM)(int)(value), 0)


/*
 * Global constants
 */
extern const _TUCHAR LcdCtrl_className[];

/*
 * Global procedures
 */
void LcdCtrl_Deinit(void);
void LcdCtrl_NumInit(HWND lcdCtrl, const NUMLCDINIT *init);
int LcdCtrl_PageSize8(UINT ctrlID, int dir, int value);
int LcdCtrl_PageSize12(UINT ctrlID, int dir, int value);
int LcdCtrl_PageSizeSnap5(UINT ctrlID, int dir, int value);
BOOL LcdCtrl_Register(void);
void LcdCtrl_SpecialInit(HWND lcdCtrl, const SPECIALLCDINIT *init);
void LcdCtrl_TextInit(HWND lcdCtrl, int maxLen);


#endif
