/*
 * txlib.h - TX81Z data library module
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
#ifndef TXLIB_H
#define TXLIB_H

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
#ifndef TX81Z_H
#   include "tx81z.h"
#endif

/*
 * Constants
 */
#define TXLIBITEM_COMMENT_SIZE  41
#define TXLIBITEM_NAME_SIZE     11
#define TXLIB_SEARCHSTR_LEN     40
#define TXLIB_ADDITEM_MEMORY_ERROR  0

/*
 * Data types
 */
typedef enum {
    IO_NONE             = 0x00,
    IO_NO_DUPLICATES    = 0x01,
    IO_UNIQUE_NAMES     = 0x02,
    IO_CHECK_EVERY_POS  = 0x04,
    IO_VMEMS            = 0x08,
    IO_PMEMS            = 0x10,
} IMPORT_OPTIONS;

typedef enum {
    EO_NONE                = 0x00,
    EO_SELECTED_ONLY       = 0x01,
    EO_SYSEX_HEADERS       = 0x02,
    EO_APPEND_FILE         = 0x04,
} EXPORT_OPTIONS;

typedef enum {
    TAO_NONE            = 0x00,
    TAO_NO_DUPLICATES   = 0x01,
    TAO_UNIQUE_NAMES    = 0x02,
} TXLIB_ADD_OPTIONS;

typedef enum {
    TS_ORDER_ADDED,
    TS_TYPE,
    TS_NAME,
    TS_COMMENT,
    TS_SELECTION,
    TS_SEARCH,
} TXLIB_SORT;

typedef enum {
    TST_NAME,
    TST_COMMENT,
    TST_NAME_AND_COMMENT,
} TXLIB_SEARCH_TYPE;

typedef struct {
    size_t size;
    int order;
    METAINDEX type;
    BOOL dirty;
    char comment[TXLIBITEM_COMMENT_SIZE];
    BYTE data[1];  /* BYTE data[size - typeSize] */
} TXLIBITEM;

typedef struct tagTXLIBNODE {
    struct tagTXLIBNODE *prev;
    struct tagTXLIBNODE *next;
    TXLIBITEM item;
} TXLIBNODE;

typedef struct {
    _TUCHAR fileName[_MAX_PATH];
    TXLIBNODE head;
    TXLIBNODE *ptr;
    int ptrIndex;
    int count;
    int nextOrder;
    TXLIB_SORT sort;
    BOOL dirty;
} TXLIB;


int TxLib_AddItem(TXLIB *txLib, METAINDEX type, BYTE *data
        , TXLIB_ADD_OPTIONS options);
void TxLib_Clean(TXLIB *txLib);
void TxLib_DeInit(TXLIB *txLib);
void TxLib_DeleteItem(TXLIB *txLib, int itemIndex);
void TxLib_Export(HWND mainWnd, TXLIB *txLib, int *libSelItems
        , int libSelItemCnt);
void TxLib_FormatName(TXLIB *txLib, NAME_FORMAT fmtFlags, _TUCHAR *dest);
__inline int TxLib_GetCount(TXLIB *txLib);
void TxLib_GetItemComment(TXLIB *txLib, _TUCHAR comment[TXLIBITEM_COMMENT_SIZE]);
__inline BYTE *TxLib_GetItemData(TXLIB *txLib);
void TxLib_GetItemName(TXLIB *txLib, _TUCHAR name[TXLIBITEM_NAME_SIZE]);
__inline int TxLib_GetItemOrder(TXLIB *txLib);
__inline METAINDEX TxLib_GetItemType(TXLIB *txLib);
void TxLib_Head(TXLIB *txLib);
void TxLib_Import(HWND mainWnd, TXLIB *txLib, IMPORT_OPTIONS importOptions
        , _TUCHAR *fileNames);
void TxLib_Init(TXLIB *txLib);
__inline BOOL TxLib_IsDirty(TXLIB *txLib);
__inline BOOL TxLib_IsEmpty(TXLIB *txLib);
__inline BOOL TxLib_IsItemDirty(TXLIB *txLib);
BOOL TxLib_Load(HWND mainWnd, TXLIB *txLib);
void TxLib_New(TXLIB *txLib);
TXLIBITEM *TxLib_Next(TXLIB *txLib);
TXLIBITEM *TxLib_Prev(TXLIB *txLib);
void TxLib_ReplaceLib(TXLIB *destLib, TXLIB *srcLib);
BOOL TxLib_Save(HWND mainWnd, TXLIB *txLib);
void TxLib_SetItemComment(TXLIB *txLib, _TUCHAR *comment);
void TxLib_SetItemName(TXLIB *txLib, _TUCHAR *name);
TXLIBITEM *TxLib_SetPtr(TXLIB *txLib, int itemIndex);
void TxLib_Sort(TXLIB *txLib, TXLIB_SORT sort);
__inline TXLIBITEM *TxLib_This(TXLIB *txLib);

/*
 * Global variables
 */
extern char TxLib_searchStr[TXLIB_SEARCHSTR_LEN + 1];
extern TXLIB_SEARCH_TYPE TxLib_searchType;
extern int *TxLib_libSelItems;
extern int *TxLib_libSelItemCnt;

/*
 * Inline definitions
 */

/*
 * GetCount() - Gets the count of items in the library.
 */
int TxLib_GetCount(TXLIB *txLib)
{
    return txLib->count;
}

/*
 * GetItemData() - Gets the current item's data.
 */
BYTE *TxLib_GetItemData(TXLIB *txLib)
{
    return TxLib_This(txLib)->data;
}

/*
 * GetItemOrder() - Returns the current item's insert order.
 */
int TxLib_GetItemOrder(TXLIB *txLib)
{
    return TxLib_This(txLib)->order;
}

/*
 * GetItemType() - Returns the current item's type.
 */
METAINDEX TxLib_GetItemType(TXLIB *txLib)
{
    return TxLib_This(txLib)->type;
}

/*
 * IsDirty() - Returns true if any of the items in the library are dirty.
 */
BOOL TxLib_IsDirty(TXLIB *txLib)
{
    return txLib->dirty;
}

/*
 * IsEmpty() - Returns true if the library is empty.
 */
BOOL TxLib_IsEmpty(TXLIB *txLib)
{
    return txLib->count == 0;
}

/*
 * IsItemDirty() - Returns true if the current item is dirty.
 */
BOOL TxLib_IsItemDirty(TXLIB *txLib)
{
    return TxLib_This(txLib)->dirty;
}

/*
 * This() - Returns the current item.
 */
TXLIBITEM *TxLib_This(TXLIB *txLib)
{
    return &txLib->ptr->item;
}

#endif  /* #ifndef TXLIB_H */
