/*
 * mtfdlg.h - Full micro-tunings editor
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
#ifndef MTFDLG_H
#define MTFDLG_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef SNAPSHOT_H
#   include "snapshot.h"
#endif


/*
 * Global constants
 */
extern const _TUCHAR *MtfDlg_className;

/*
 * Global procedures
 */
BOOL MtfDlg_Create(HWND parentWnd, MTF *mtf);
void MtfDlg_Update(HWND mtfDlg, MTF *mtf);


#endif  /* #ifndef MTFDLG_H */
