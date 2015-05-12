/*
 * gui/error.h - gui interface to the msg/error module
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
#ifndef GUI_ERROR_H
#define GUI_ERROR_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef MSG_ERROR_H
#   include "msgerror.h"
#endif

/*
 * Gui error codes
 */
#define E_FIRST_GUI_ERROR                  100
#define E_REGISTRY_BAD_TYPE                E_FIRST_GUI_ERROR
#define E_REGISTRY_NONEXISTENT_VALUE       E_FIRST_GUI_ERROR + 1
#define E_REGISTRY_KEY_ACCESS_DENIED       E_FIRST_GUI_ERROR + 2

/*
 * GuiInit() - Sets Error_GuiError as the error handler and sets wnd as the
 *             message box parent window.
 */
void Error_GuiInit(HWND wnd);

/*
 * GuiMsg() - Displays a simple message box with the error window as the
 *            parent.
 */
void Error_GuiMsg(const _TUCHAR *message);

/*
 * LastError() - Displays the message from GetLastError() with the error window
 *               as parent.
 */
void Error_LastError(void);
void Error_LastErrorF(const _TUCHAR *fmt, ...);


#endif  /* #ifndef GUI_ERROR_H */
