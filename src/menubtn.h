/*
 * menubtn.h - Menu button control
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
#ifndef MENUBTN_H
#define MENUBTN_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * Global constants
 */
#define MENUBTN_TEXT_MAX  40

/*
 * Notification messages
 */
#define MBN_PUSHED    1
#define MBN_RELEASED  2

/*
 * Global types
 */
typedef UINT (*TRACKFUNC)(HANDLE menu, RECT *btnRect);

/*
 * Global procedures
 */
void MenuBtn_Deinit(HWND btnCtrl);
void MenuBtn_DrawArrow(HWND parentWnd, const DRAWITEMSTRUCT *drawItem);
void MenuBtn_DrawButton(HWND parentWnd, const DRAWITEMSTRUCT *drawItem);
BOOL MenuBtn_Init(HWND btnCtrl, HANDLE menu, TRACKFUNC TrackFunc
        , UINT trackFuncFlags);


#endif  /* #ifndef MENUBTN_H */
