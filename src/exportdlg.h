/*
 * exportdlg.h - Export dialog template module
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
#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_STDLIB
#   include <stdlib.h>
#endif
#ifndef TXLIB_H
#   include "txlib.h"
#endif


/*
 * Global procedures
 */
BOOL ExportDlg_Create(HWND parentWnd, TXLIB *txLib, int *libSelItems
        , int libSelItemCnt, int batchNumber, int batchStart
        , METAINDEX *exportType, _TUCHAR *initialDir, _TUCHAR *fileName
        , size_t fileNameLen);


#endif  /* #ifndef EXPORTDLG_H */
