/*
 * cdvlist.c - a generic, circular, doubly linked list with variable sized
 *             nodes
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

/*
 * Global procedures
 */
extern CDVNODE *CDVList_Append(CDVLIST *list, size_t size);
extern void CDVList_Clear(CDVLIST *list);
extern CDVLIST *CDVList_Create(void);
extern BOOL CDVList_Delete(CDVLIST *list);
extern void CDVList_Destroy(CDVLIST *list);
extern CDVNODE *CDVList_First(CDVLIST *list);
extern __inline void CDVList_Head(CDVLIST *list);
extern __inline int CDVList_IsEmpty(CDVLIST *list);
extern CDVNODE *CDVList_Prev(CDVLIST *list);
extern __inline CDVNODE *CDVList_This(CDVLIST *list);


/*
 * Procedure definitions
 */
CDVNODE *CDVList_Append(CDVLIST *list, size_t size)
{
	CDVLISTNODE *node;

	assert(list);

	/*
	 * Create a CDVLISTNODE structure (the next and prev pointers), but cheat
	 * and add enough bytes to hold a user record after it.
	 */
	node = malloc(sizeof(CDVLISTNODE) + size);
	if (!node) {
		Error_OnError(E_MALLOC_ERROR);
		return NULL;
	}
	node->size = size;

	/*
	 * Insert the node into the list.
	 */
	node->next = &list->head;
	node->prev = list->head.prev;
	list->head.prev->next = node;
	list->head.prev = node;

	/*
	 * Set internal pointer to the new node.
	 */
	list->ptr = node;

	/*
	 * Return the uninitialized record.
	 */
	return (CDVNODE *)(&node->size);
}

void CDVList_Clear(CDVLIST *list)
{
	CDVList_First(list);
	while (CDVList_Delete(list))
		;

	return;
}

CDVLIST *CDVList_Create(void)
{
	/*
	 * Create a head node and return it as a handle.
	 */
	CDVLIST *list;
	size_t size = sizeof(CDVLIST);
	list = malloc(size);
	if (!list) {
		Error_OnError(E_MALLOC_ERROR);
		return NULL;
	}

	list->head.prev = list->head.next = list->ptr = &list->head;
	list->head.size = 0;

	return list;
}

BOOL CDVList_Delete(CDVLIST *list)
{
	CDVLISTNODE *tmp = list->ptr;

	assert(tmp);

	/*
	 * Check for empty list.
	 */
	if (tmp == &list->head)
		return FALSE;

	/*
	 * Connect the previous and next nodes to each other.
	 */
	tmp->prev->next = tmp->next;
	tmp->next->prev = tmp->prev;

	/*
	 * Advance the internal pointer.
	 */
	list->ptr = tmp->next;

	/*
	 * Free the node.
	 */
	free(tmp);

	return TRUE;
}

void CDVList_Destroy(CDVLIST *list)
{
	CDVLISTNODE *tmp = &list->head;
	CDVLISTNODE *p = tmp->next;

	assert(list);

	/*
	 * Walk through the list and delete all the nodes.
	 */
	while (p != &list->head) {
		tmp = p;
		p = p->next;
		free(tmp);
	}

	/*
	 * Delete the head node.
	 */
	free(list);
}

CDVNODE *CDVList_First(CDVLIST *list)
{
	assert(list);

	/*
	 * Set internal pointer to the first node in the list.
	 */
	list->ptr = list->head.next;

	/*
	 * Check for empty list.
	 */
	if (list->ptr == &list->head)
		return NULL;

	/*
	 * Return a pointer to the record, which is the address of the node
	 * plus enough bytes to skip over the prev and next pointers.
	 */
	return (CDVNODE *)(&list->ptr->size);
}

CDVNODE *CDVList_Prev(CDVLIST *list)
{
	assert(list);

	/*
	 * Advance internal pointer.
	 */
	list->ptr = list->ptr->prev;

	/*
	 * If the pointer circles around back to the head, then the whole list
	 * has been traversed.
	 */
	if (list->ptr == &list->head)
		return NULL;

	/*
	 * Return a pointer to the record, which is the address of the node
	 * plus enough bytes to skip over the prev and next pointers.
	 */
	return (CDVNODE *)(&list->ptr->size);
}

