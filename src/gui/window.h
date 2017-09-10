/*
 * gui/window.h - convenience functions for windows
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
#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef GUI_AREA_H
#   include "area.h"
#endif

/*
 * Global procedures
 */
void Window_AddSysMenuItem(HWND wnd, const _TUCHAR *itemText, unsigned cmdID
        , BOOL separator);
SIZE Window_CalcControlAreaSize(HWND wnd);
BOOL Window_CalcPositionAroundRect(RECT *wndRect, const RECT *btnRect);
void Window_Center(HWND topWnd, HWND bottomWnd);
void Window_CenterInParent(HWND wnd);
void Window_ClearQueue(HWND wnd);
BOOL Window_DoEvents(HWND wnd, HACCEL accels);
void Window_GetParentRelativeArea(HWND child, HWND parent, AREA *area);
POINT Window_GetParentRelativePosition(HWND child, HWND parent);
void Window_GetParentRelativeRect(HWND child, HWND parent, RECT *rect);
_TUCHAR *Window_GetTextM(HWND wnd, const _TUCHAR *blankMsg, size_t *returnLen);


#endif
