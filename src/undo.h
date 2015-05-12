/*
 * undo.h - Undo module for TX81Z editor
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
#ifndef UNDO_H
#define UNDO_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * Global types
 */
typedef struct {
    UINT ctrlID;
    size_t size;
    void *oldValue;
    void *newValue;
    BOOL groupStart;
} CHANGE;

typedef struct tagUNDOLISTNODE {
    struct tagUNDOLISTNODE *next;
    struct tagUNDOLISTNODE *prev;
    CHANGE change;
} UNDOLISTNODE;

typedef struct {
    UNDOLISTNODE head;
    UNDOLISTNODE *ptr; /* points to the next change that will be undone if
                          NextChange() is called */
} UNDO;

typedef void (CALLBACK *DOFUNC)(HWND wnd, CHANGE *change);

/*
 * Global procedures
 */
BOOL Undo_AnyRedoes(UNDO *undo);
BOOL Undo_AnyUndoes(UNDO *undo);
CHANGE *Undo_AddChange(UNDO *undo, UINT ctrlID, size_t size, void *oldValue
        , void *newValue, BOOL groupStart);
void Undo_Clear(UNDO *undo);
UNDO *Undo_Create(void);
void Undo_Destroy(UNDO *undo);
void Undo_Redo(UNDO *undo, DOFUNC DoFunc, HWND wnd);
void Undo_Undo(UNDO *undo, DOFUNC DoFunc, HWND wnd);


#endif  /* #ifndef UNDO_H */
