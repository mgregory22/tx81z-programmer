/*
 * undo.c - Undo module for TX81Z editor
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
#include "undo.h"

/*
 * Global procedures
 */
extern BOOL Undo_AnyRedoes(UNDO *undo);
extern BOOL Undo_AnyUndoes(UNDO *undo);
extern CHANGE *Undo_AddChange(UNDO *undo, UINT ctrlID, size_t size, void *oldValue, void *newValue, BOOL groupStart);
extern void Undo_BeginChangeGroup(UNDO *undo);
extern void Undo_Clear(UNDO *undo);
extern UNDO *Undo_Create(void);
extern void Undo_Destroy(UNDO *undo);
extern void Undo_Redo(UNDO *undo, DOFUNC DoFunc, HWND wnd);
extern void Undo_Undo(UNDO *undo, DOFUNC DoFunc, HWND wnd);

/*
 * Unit procedures
 */
static void u_DeleteTail(UNDO *undo);
static CHANGE *u_Redo(UNDO *undo);
static CHANGE *u_Undo(UNDO *undo);

/*
 * Procedure definitions
 */

/*
 * AnyRedoes() - Returns true if there are any undone changes that can be
 *               redone.
 */
BOOL Undo_AnyRedoes(UNDO *undo)
{
    return undo->ptr != &undo->head;
}

/*
 * AnyUndoes() - Returns true if there are any changes that can be undone.
 */
BOOL Undo_AnyUndoes(UNDO *undo)
{
    return undo->ptr->prev != &undo->head;
}

/*
 * AddChange() - Deletes the tail of the list starting at the current node and
 *               appends a change to the undo list.  <oldValue> and <newValue>
 *               are assumed to be buffers of size <size>, but they can be
 *               NULL.  Two buffers of size <size> are malloc()'ed and if the
 *               passed buffers are valid, they are memcpy()'ed into the change
 *               structure.  If true is passed in groupStart, then the change
 *               is marked as the start of a new change group.
 */
CHANGE *Undo_AddChange(UNDO *undo, UINT ctrlID, size_t size, void *oldValue
        , void *newValue, BOOL groupStart)
{
    UNDOLISTNODE *node;
    
    /*
     * If an undo action has been taken, or the list is empty, then override
     * the groupStart setting and start a new group.
     */
    if ((undo->ptr != &undo->head) || (undo->head.next == &undo->head)
            || (oldValue == NULL) || (newValue == NULL))
    {
        groupStart = 1;
    }
    /*
     * If this change isn't starting a new group, search the current group for
     * a change with the same ctrlID number.  (<node> is being pre-recycled
     * as a temporary pointer here)
     */
    if (!groupStart) {
        node = &undo->head;
        do {
            node = node->prev;
            if (node->change.ctrlID == ctrlID) {
                /*
                 * Replace the node's new value with the new new value and return.
                 * The plan is that the undo function will restore the control's
                 * original value, which is what it was at the point in time when
                 * the control received the input focus.
                 */
                memcpy(node->change.newValue, newValue, size);
                return &node->change;
            }
        } while (!node->change.groupStart);
    }
    /*
     * Create a new node for the list.
     */
    node = malloc(sizeof *node);
    if (!node) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    /*
     * Replace all of the items from undo->ptr to the end with the new node.
     */
    u_DeleteTail(undo);
    /*
     * Set the node's values.
     */
    node->change.ctrlID = ctrlID;
    node->change.size = size;
    node->change.oldValue = malloc(size);
    if (!node->change.oldValue) {
        Error_OnError(E_MALLOC_ERROR);
        free(node);
        return NULL;
    }
    if (oldValue) {
        memcpy(node->change.oldValue, oldValue, size);
    }
    node->change.newValue = malloc(size);
    if (!node->change.newValue) {
        Error_OnError(E_MALLOC_ERROR);
        free(node->change.oldValue);
        free(node);
        return NULL;
    }
    if (newValue) {
        memcpy(node->change.newValue, newValue, size);
    }
    node->change.groupStart = groupStart;
    /*
     * Append the node to the list.
     */
    node->next = &undo->head;
    node->prev = undo->head.prev;
    node->prev->next = node;
    undo->head.prev = node;

    return &node->change;
}

/*
 * Clear() - Clears the undo list.
 */
void Undo_Clear(UNDO *undo)
{
    undo->ptr = undo->head.next;
    u_DeleteTail(undo);
}

/*
 * Create() - Allocates an undo object and returns a pointer to it, or null on
 *            failure.
 */
UNDO *Undo_Create(void)
{
    UNDO *undo = malloc(sizeof *undo);

    if (!undo) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    undo->head.next = undo->head.prev = undo->ptr = &undo->head;
    undo->head.change.ctrlID = undo->head.change.size = 0;
    undo->head.change.oldValue = undo->head.change.newValue = NULL;
    undo->head.change.groupStart = FALSE;

    return undo;
}

/*
 * Destroy() - Frees an undo object.
 */
void Undo_Destroy(UNDO *undo)
{
    if (!undo) {
        return;
    }
    Undo_Clear(undo);
    free(undo);
}

/*
 * Redo() - Calls DoFunc() for each change in the current redo change set.
 */
void Undo_Redo(UNDO *undo, DOFUNC DoFunc, HWND wnd)
{
    CHANGE *change;

    while (change = u_Redo(undo)) {
        DoFunc(wnd, change);
        if (undo->ptr->change.groupStart) {
            break;
        }
    }
}

/*
 * Undo() - Calls DoFunc() for each change in the current undo change set.
 */
void Undo_Undo(UNDO *undo, DOFUNC DoFunc, HWND wnd)
{
    CHANGE *change;

    while (change = u_Undo(undo)) {
        DoFunc(wnd, change);
        if (change->groupStart) {
            break;
        }
    }
}

/*
 * DeleteTail() - Deletes all changes from the current position onward.
 */
void u_DeleteTail(UNDO *undo)
{
    UNDOLISTNODE *p = undo->ptr;
    UNDOLISTNODE *tmp;

    if (p == &undo->head) {
        return;
    }
    /*
     * Disconnect the tail from the rest of the list, leaving p pointing to
     * the tail.
     */
    undo->ptr = p->prev;
    undo->ptr->next = &undo->head;
    undo->head.prev = undo->ptr;
    /*
     * Walk through the nodes at p and delete them.
     */
    while (p != &undo->head) {
        tmp = p;
        p = p->next;
        free(tmp->change.newValue);
        free(tmp->change.oldValue);
        free(tmp);
    }
    /*
     * Set the pointer to the head node.
     */
    undo->ptr = p;
}

/*
 * Redo() - Returns the next change of a redo action, or null if there are
 *          no more changes in the undo group.
 */
CHANGE *u_Redo(UNDO *undo)
{
    if (undo->ptr == &undo->head) {
        return NULL;
    }
    undo->ptr = undo->ptr->next;

    return &undo->ptr->prev->change;
}

/*
 * Undo() - Returns the next change of an undo action, or null if there are
 *          no more changes in the undo group.
 */
CHANGE *u_Undo(UNDO *undo)
{
    if (undo->ptr->prev == &undo->head) {
        return NULL;
    }
    undo->ptr = undo->ptr->prev;

    return &undo->ptr->change;
}

