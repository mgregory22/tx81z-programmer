/*
 * pfmdlg.h - TX81Z Performance Editor
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
#ifndef PFMDLG_H
#define PFMDLG_H

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
extern const _TUCHAR *PfmDlg_className;

/*
 * Global procedures
 */
BOOL PfmDlg_Create(HWND parentWnd, PCED *pfm);
void PfmDlg_Update(HWND pfmDlg, PCED *pfm);

/*
 * Global variables
 */
extern _TUCHAR PfmDlg_voiceNumSettings[160][3];


#endif
