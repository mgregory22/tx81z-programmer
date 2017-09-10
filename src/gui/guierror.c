/*
 * guierror.c - gui interface to the msg/error module
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

/*
 * Unit constants
 */
const _TUCHAR *Gui_error_msg[] = {
    _T("The registry entry is the wrong data type"),   /* E_REGISTRY_BAD_TYPE */
    _T("The specified registry entry does not exist"), /* E_REGISTRY_NONEXISTENT_VALUE */
    _T("The specified registry key is inaccessible"),  /* E_REGISTRY_KEY_ACCESS_DENIED */
};

/*
 * Global procedures
 */
extern void Error_GuiInit(HWND wnd);
extern void Error_GuiMsg(const _TUCHAR *message);
extern void Error_LastError(void);
extern void Error_LastErrorF(const _TUCHAR *fmt, ...);

/*
 * Local variables
 */
static HWND e_errorWnd;


void Error_GuiError(unsigned errorCode)
{
    if (errorCode >= E_FIRST_GUI_ERROR)
        MsgBox_F(e_errorWnd, Gui_error_msg[errorCode - E_FIRST_GUI_ERROR]);
    else if (errorCode == E_SYSTEM_ERROR)
		MsgBox_LastError(e_errorWnd);
	else
        MsgBox_F(e_errorWnd, Error_msg[errorCode]);
}

void Error_GuiInit(HWND wnd)
{
    e_errorWnd = wnd;
    Error_OnError = Error_GuiError;
}

void Error_GuiMsg(const _TUCHAR *message)
{
    MsgBox_F(e_errorWnd, message);
}

void Error_LastError(void)
{
    MsgBox_LastError(e_errorWnd);
}

void Error_LastErrorF(const _TUCHAR *fmt, ...)
{
    va_list argList;

    va_start(argList, fmt);
    MsgBox_LastErrorFva(e_errorWnd, fmt, argList);
    va_end(argList);
}

