/*
 * msg/msgerror.h - error handling routines
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
#ifndef MSG_MSGERROR_H
#define MSG_MSGERROR_H

#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Macros
 */
#ifdef _DEBUG
#define Error_Pause() { \
    extern int __cdecl _getch(void); \
    printf("Press any key to exit"); \
    _getch(); \
}
#else
#define Error_Pause()
#endif

/*
 * error codes
 */
#define E_SUCCESS                            0
#define E_SYSTEM_ERROR                       1
#define E_MALLOC_ERROR                       2
#define E_HELP_REQUEST                       3
#define E_VERSION_REQUEST                    4
#define E_UNKNOWN_OPTION                     5
#define E_MISSING_OPTION_ARGUMENT            6
#define E_MISSING_ARGUMENT                   7
#define E_SYMBOL_NOT_FOUND                   8
#define E_CONTAINER_FULL                     9
#define E_NOT_IMPLEMENTED                   10
#define E_CANNOT_APPEND_TO_ITEM             11
#define E_CANNOT_DELETE_END_ITEM            12
#define E_CANNOT_INSERT_BEYOND_END_ITEM     13
#define E_CANNOT_OPEN_FILE                  14
#define E_INVALID_FILE_HANDLE               15
#define E_OUT_OF_BOUNDS                     16
#define E_FILE_DOES_NOT_EXIST               17
#define E_FILE_READ_SHORT                   18
#define E_BUFFER_TOO_SMALL                  19
#define E_MISMATCHED_FIELD_SIZE             20
#define E_UNKNOWN_ENUM_ID                   21
#define E_UNKNOWN_ENUM_VALUE                22
#define E_UNKNOWN_STRUCT_FIELD              23
#define E_INVALID_XML                       24
#define E_INVALID_FIELD_TYPE                25
#define E_MISMATCHED_SERIAL_FIELD_TYPE      26
#define E_MISMATCHED_COUNT_FIELD            27

/*
 * The error handler used for the msg and gui libraries.  The error condition
 * can be cancelled in some situations if the handler returns true.
 */
extern void (*Error_OnError)(unsigned);

/*
 * The array of error message strings, indexed by error code.
 */
extern const _TUCHAR *Error_msg[];

#endif
