/*
 * mtgen.h - Generic routines for micro tune octave and micro tune full dialogs.
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
#ifndef MTGEN_H
#define MTGEN_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef GUI_DIALOG_H
#   include <gui/dialog.h>
#endif
#ifndef UNDO_H
#   include "undo.h"
#endif


/*
 * Global types
 */
typedef struct {
    HWND wnd;
    UINT ctrlID;
    HMENU menu;
    BYTE *data;
    BOOL dirty;
    int initCnt;
    int subGrp;
    UNDO *undo;
    BOOL undoGroup;
} MTDLGDATA;

/*
 * Global procedures
 */
void MTGen_FineLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
void MTGen_FullLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
void MTGen_InitControlValues(MTDLGDATA *mtDlgData);
void MTGen_NoteLcdChange(MTDLGDATA *mtDlgData, UINT ctrlID, int value);
void MTGen_OnKey(HWND mtDlg, UINT vk, BOOL down, int repeat, UINT keyFlags
        , UINT initCnt, DIALOG *dialog);
void CALLBACK MTGen_Redo(HWND mtDlg, CHANGE *change);
void CALLBACK MTGen_Undo(HWND mtDlg, CHANGE *change);


#endif  /* #ifndef MTGEN_H */
