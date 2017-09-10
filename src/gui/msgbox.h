/*
 * gui/msgbox.h - message box functions
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
#ifndef GUI_MSGBOX_H
#define GUI_MSGBOX_H

#if !defined(_INC_STDARG) && !defined(_STDARG_H_)
#   include <stdarg.h>
#endif
#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Displays a message box with an error message associated with the given ID.
 */
void MsgBox_Error(HWND parentWnd, DWORD errorID);

/*
 * Same as MsgBox_ErrorF, but takes a va_list instead of an ellipsis.
 */
void MsgBox_ErrorFva(HWND parentWnd, DWORD errorID, const _TUCHAR *fmt
        , va_list argList);

/*
 * Displays a message box with formatted text.
 */
void MsgBox_F(HWND parentWnd, const _TUCHAR *fmt, ...);

/*
 * Same as MsgBox_F but takes a va_list instead of an ellipsis.
 */
void MsgBox_Fva(HWND parentWnd, const _TUCHAR *fmt, va_list argList);

/*
 * Calls MsgBox_Error with GetLastError() for the errorID.
 */
void MsgBox_LastError(HWND parentWnd);

/*
 * Displays a message box of the last error (by calling GetLastError)
 * with formatted text.
 */
void MsgBox_LastErrorF(HWND parentWnd, const _TUCHAR *fmt, ...);

void MsgBox_LastErrorFva(HWND parentWnd, const _TUCHAR *fmt, va_list argList);


#endif
