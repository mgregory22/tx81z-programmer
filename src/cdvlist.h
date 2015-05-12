/*
 * msg/cdvlist.h - a generic, circular, doubly linked list with variable
 *                 sized nodes
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
#ifndef MSG_CDVLIST_H
#define MSG_CDVLIST_H

#include <assert.h>
#ifndef MSG_FUNC_H
#   include "func.h"
#endif


typedef struct tagCDVLISTNODE {
	struct tagCDVLISTNODE *prev;
	struct tagCDVLISTNODE *next;
	size_t size;
} CDVLISTNODE;

typedef struct tagCDVLIST {
	CDVLISTNODE head;
	CDVLISTNODE *ptr;
} CDVLIST;

typedef struct tagCDVNODE {
	size_t size;           /* this size is the same as the size in CDVLISTNODE
                              (they overlap) */
	unsigned char data[1]; /* the size of this array is actually <size> bytes */
} CDVNODE;

/*
 * Add() - Inserts a node created by NewNode() into the list in sorted order.
 *         Returns the position of the node in the list, starting with 0.
 */
size_t CDVList_Add(CDVLIST *list, CDVLISTNODE *node, CMPFUNC Cmp);

/*
 * Append() - Allocates memory for a new record, appends the record to the
 *            list, sets the internal pointer to the new node, and returns a
 *            pointer to the uninitialized record.  Returns null on failure.
 */
CDVNODE *CDVList_Append(CDVLIST *list, size_t size);

/*
 * Clear() - Removes and frees all nodes in the list.
 */
void CDVList_Clear(CDVLIST *list);

/*
 * Create() - Creates a new list and returns a handle to it.
 */
CDVLIST *CDVList_Create(void);

/*
 * Delete() - Deletes the node at the internal pointer and frees its record.
 *            Returns TRUE on success, FALSE if the internal pointer was at
 *            the end of the list.
 */
BOOL CDVList_Delete(CDVLIST *list);

/*
 * Destroy() - Completely destroys a list, head node and all.
 */
void CDVList_Destroy(CDVLIST *list);

/*
 * First() - Sets the internal pointer to the first record in the list and
 *           returns a pointer to that record.  Returns null if the list is
 *           empty.
 */
CDVNODE *CDVList_First(CDVLIST *list);

/*
 * Head() - Sets the internal pointer to the head node so all the nodes can be
 *          conveniently accessed with CDVList_Next() or CDVList_Prev().
 */
__inline void CDVList_Head(CDVLIST *list);

/*
 * IsEmpty() - Returns TRUE if the list is empty
 */
__inline BOOL CDVList_IsEmpty(CDVLIST *list);

/*
 * Last() - Same as CDVList_First but points to the last record.
 */
CDVNODE *CDVList_Last(CDVLIST *list);

/*
 * NewNode() - Creates and initializes a list node and returns it.  Note that
 *             the type is a list node and not just the data part of it.
 */
CDVLISTNODE *CDVList_NewNode(size_t size);

/*
 * Next() - Advances the internal pointer to the next record and returns a
 *          pointer to that record.  Returns null if it reaches the end of the
 *          list.
 */
CDVNODE *CDVList_Next(CDVLIST *list);

/*
 * Prepend() - Same as CDVList_Append but inserts at the beginning.
 */
CDVNODE *CDVList_Prepend(CDVLIST *list, size_t size);

/*
 * Prev() - Same as CDVList_Next but goes backwards.
 */
CDVNODE *CDVList_Prev(CDVLIST *list);

/*
 * Sort() - Sorts the list.
 */
void CDVList_Sort(CDVLIST *list, CMPFUNC Cmp);

/*
 * This() - Returns the current record or null if the internal pointer is not
 *          pointing to a valid node.
 */
__inline CDVNODE *CDVList_This(CDVLIST *list);


/*
 * Inline procedures
 */
void CDVList_Head(CDVLIST *list)
{
	assert(list);

	/*
	 * Set internal pointer to the head node.
	 */
	list->ptr = &list->head;
}

BOOL CDVList_IsEmpty(CDVLIST *list)
{
	/*
	 * If head->next is pointing to head, then the list is empty (checking
	 * head->prev is redundant).
	 */
	if (list->head.next == &list->head)
		return TRUE;

	return FALSE;
}

CDVNODE *CDVList_This(CDVLIST *list)
{
	assert(list);

	/*
	 * Never return a pointer to the head node.
	 */
	if (list->ptr == &list->head)
		return NULL;

	return (CDVNODE *)(&list->ptr->size);
}

#endif
