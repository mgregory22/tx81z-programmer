/*
 * mainwnd.h - main window class
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
#ifndef MAINWND_H
#define MAINWND_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Global constants
 */
extern const _TUCHAR *MainWnd_className;

/*
 * Global procedures
 */
void MainWnd_AddToLib(HWND mainWnd, int libIndex, const int *snapshotItemIdxs
        , int snapshotItemIdxCnt);
void MainWnd_BundleToLib(HWND mainWnd, int libIndex
        , const int *snapshotItemIdxs, int snapshotItemIdxCnt);
HWND MainWnd_Create(void);
BOOL MainWnd_Register(void);
BOOL MainWnd_StoreItem(int srcItemIndex, int destItemIndex);


#endif  /* #ifndef MAINWND_H */
