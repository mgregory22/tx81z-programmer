/*
 * msgerror.c - error handling routines
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
 * Unit constants
 */
const _TUCHAR *Error_msg[] = {
    NULL,                                   /*  0 - E_SUCCESS */
	NULL,                                   /*  1 - E_SYSTEM_ERROR */
    _T("Memory allocation failure"),        /*  2 - E_MALLOC_ERROR */
    NULL,                                   /*  3 - E_HELP_REQUEST */
    NULL,                                   /*  4 - E_VERSION_REQUEST */
    NULL,                                   /*  5 - E_UNKNOWN_OPTION */
    NULL,                                   /*  6 - E_MISSING_OPTION_ARGUMENT */
    NULL,                                   /*  7 - E_MISSING_ARGUMENT */
    NULL,                                   /*  8 - E_SYMBOL_NOT_FOUND */
    NULL,                                   /*  9 - E_CONTAINER_FULL */
    NULL,                                   /* 10 - E_NOT_IMPLEMENTED */
    NULL,                                               
                                            /* 11 - E_CANNOT_APPEND_TO_ITEM */
    NULL,                                               
                                            /* 12 - E_CANNOT_DELETE_END_ITEM */
    NULL,                                     
                                            /* 13 - E_CANNOT_INSERT_BEYOND_END_ITEM */
    _T("Cannot open file"),                 /* 14 - E_CANNOT_OPEN_FILE */
    NULL,                                   /* 15 - E_INVALID_FILE_HANDLE */
    NULL,                                   /* 16 - E_OUT_OF_BOUNDS */
    _T("File does not exist"),              /* 17 - E_FILE_DOES_NOT_EXIST */
    _T("The file was truncated on load"),   /* 18 - E_FILE_READ_SHORT */
    NULL,
                                            /* 19 - E_BUFFER_TOO_SMALL */
    NULL,
                                            /* 20 - E_MISMATCHED_FIELD_SIZE */
    NULL,                                   /* 21 - E_UNKNOWN_ENUM_ID */
    NULL,                                   /* 22 - E_UNKNOWN_ENUM_VALUE */
    NULL,                                   /* 23 - E_UNKNOWN_STRUCT_FIELD */
    NULL,                                   /* 24 - E_INVALID_XML */
    NULL,                                   /* 25 - E_INVALID_FIELD_TYPE */
    NULL,                                   /* 26 - E_MISMATCHED_SERIAL_FIELD_TYPE */
    NULL,
                                            /* 27 - E_MISMATCHED_COUNT_FIELD */
};

/*
 * Global procedures
 */


/*
 * Global variables
 */
void (*Error_OnError)(unsigned) = NULL;


/*
 * Procedure definitions
 */
