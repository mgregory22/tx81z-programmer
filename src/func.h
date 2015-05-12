/*
 * msg/func.h - callback function type definitions
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
#ifndef MSG_FUNC_H
#define MSG_FUNC_H

/*
 * Compares two items for boolean equality.
 */
typedef BOOL (*BCMPFUNC)(const void *key, const void *record);

/*
 * Compares two items for strcmp-like equality.
 */
typedef int (*CMPFUNC)(const void *newItem, const void *listItem);

/*
 * Deletes a user-defined record in a container before deleting a node.
 */
typedef void (*DELRECFUNC)(void *data);

/*
 * Type for the function pointer parameter in MakeDump().
 */
typedef void (*DUMPFUNC)(_TUCHAR *line, size_t lineLen, unsigned long user);


#endif
