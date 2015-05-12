/*
 * txlib.c - TX81Z data library module
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
#include "exportdlg.h"
#include "importdlg.h"
#include "prog.h"
#include "snapshot.h"
#include "tx81z.h"
#include "txpack.h"
#include "txlib.h"

/*
 * Global procedures
 */
extern int TxLib_AddItem(TXLIB *txLib, METAINDEX type, BYTE *data, TXLIB_ADD_OPTIONS options);
extern void TxLib_Clean(TXLIB *txLib);
extern void TxLib_DeInit(TXLIB *txLib);
extern void TxLib_DeleteItem(TXLIB *txLib, int itemIndex);
extern void TxLib_Export(HWND mainWnd, TXLIB *txLib, int *libSelItems, int libSelItemCnt);
extern void TxLib_FormatName(TXLIB *txLib, NAME_FORMAT fmtFlags, _TUCHAR *dest);
extern __inline int TxLib_GetCount(TXLIB *txLib);
extern void TxLib_GetItemComment(TXLIB *txLib, _TUCHAR comment[TXLIBITEM_COMMENT_SIZE]);
extern __inline BYTE *TxLib_GetItemData(TXLIB *txLib);
extern void TxLib_GetItemName(TXLIB *txLib, _TUCHAR name[TXLIBITEM_NAME_SIZE]);
extern __inline int TxLib_GetItemOrder(TXLIB *txLib);
extern __inline METAINDEX TxLib_GetItemType(TXLIB *txLib);
extern void TxLib_Head(TXLIB *txLib);
extern void TxLib_Import(HWND mainWnd, TXLIB *txLib, IMPORT_OPTIONS importOptions, _TUCHAR *fileNames);
extern void TxLib_Init(TXLIB *txLib);
extern __inline BOOL TxLib_IsDirty(TXLIB *txLib);
extern BOOL TxLib_IsEmpty(TXLIB *txLib);
extern __inline BOOL TxLib_IsItemDirty(TXLIB *txLib);
extern BOOL TxLib_Load(HWND mainWnd, TXLIB *txLib);
extern void TxLib_New(TXLIB *txLib);
extern TXLIBITEM *TxLib_Next(TXLIB *txLib);
extern TXLIBITEM *TxLib_Prev(TXLIB *txLib);
extern void TxLib_ReplaceLib(TXLIB *destLib, TXLIB *srcLib);
extern BOOL TxLib_Save(HWND mainWnd, TXLIB *txLib);
extern void TxLib_SetItemComment(TXLIB *txLib, _TUCHAR *comment);
extern void TxLib_SetItemName(TXLIB *txLib, _TUCHAR *name);
extern TXLIBITEM *TxLib_SetPtr(TXLIB *txLib, int itemIndex);
extern void TxLib_Sort(TXLIB *txLib, TXLIB_SORT sort);
extern __inline TXLIBITEM *TxLib_This(TXLIB *txLib);

/*
 * Global variables
 */
char TxLib_searchStr[TXLIB_SEARCHSTR_LEN + 1];
TXLIB_SEARCH_TYPE TxLib_searchType;
int *TxLib_libSelItems;
int *TxLib_libSelItemCnt;

/*
 * Unit Types
 */
typedef int (*TXLIBITEMCMPFUNC)(const TXLIBITEM *newItem, const TXLIBITEM *listItem);

/*
 * Unit constants
 */
#define DUMMY_DATA_BYTE  1
#define ERRORMSG_LEN  (_MAX_PATH + 80)
#define TXLIBITEM_COMMENT_MAX (TXLIBITEM_COMMENT_SIZE - 1)
static const _TUCHAR *tx_fileHdr = _T("TX81Z1.0");

/*
 * Unit procedures
 */
static int txl_Add(TXLIB *txLib, TXLIBNODE *node, TXLIBITEMCMPFUNC Cmp);
static TXLIBITEM *txl_Append(TXLIB *txLib, size_t itemSize);
static BOOL txl_CheckBufForPmem(BYTE *buf);
static BOOL txl_CheckBufForVmem(BYTE *buf);
static void txl_Clear(TXLIB *txLib);
static int txl_CmpComment(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static int txl_CmpName(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static int txl_CmpOrder(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static int txl_CmpSearch(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static int txl_CmpSelection(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static int txl_CmpType(const TXLIBITEM *newItem, const TXLIBITEM *listItem);
static BOOL txl_DeleteItem(TXLIB *txLib);
static int txl_FindDuplicateItem(TXLIB *txLib, TXLIBITEM *item);
static void txl_GetItemName(TXLIBITEM *txLib, _TUCHAR name[TXLIBITEM_NAME_SIZE]);
static void txl_IncrementNameSuffix(_TUCHAR name[TXLIBITEM_NAME_SIZE]);
static void txl_MakeNameUnique(TXLIB *txLib, TXLIBITEM *item);
static TXLIBNODE *txl_NewNode(size_t size);
static int txl_ReadVoiceBank(FILE *inFile, TXLIB *txLib, IMPORT_OPTIONS importOptions);
static void txl_Sort(TXLIB *txLib, TXLIBITEMCMPFUNC Cmp);


static TXLIBITEMCMPFUNC txl_cmpFuncs[] = {
    txl_CmpOrder,
    txl_CmpType,
    txl_CmpName,
    txl_CmpComment,
    txl_CmpSelection,
    txl_CmpSearch
};
#define txl_cmpFuncCnt  ARRAYSIZE(txl_cmpFuncs)


/*
 * Procedure definitions
 */

/*
 * AddItem() - Adds an item to the library, inserted according to the current
 *             sort order.  The return value is the one-based index where the
 *             item was inserted, the index is the negative one-based index if
 *             the item was a duplicate, or the function returns zero if there
 *             was a memory allocation failure.
 */
int TxLib_AddItem(TXLIB *txLib, METAINDEX type, BYTE *data
        , TXLIB_ADD_OPTIONS options)
{
    size_t itemSize;
    BNDL_FORMAT bndlFmt;
    
    /*
     * Calculate the amount of memory to allocate for the item.
     */
    assert(type < META_CNT || type == META_BNDL);
    if (type < META_CNT) {
        itemSize = TX81Z_meta[type].libSize;
    } else {
        bndlFmt = *((BNDL_FORMAT *) data);

        itemSize = sizeof(BNDL_FORMAT) + S_PMEM_DATA_SIZE
            + ((bndlFmt & BF_VOICE_MASK) + 1) * S_VMEM_DATA_SIZE
            + ((bndlFmt & BF_FX) != 0) * S_FX_DATA_SIZE
            + ((bndlFmt & BF_MTO) != 0) * S_MTO_DATA_SIZE
            + ((bndlFmt & BF_MTF) != 0) * S_MTF_DATA_SIZE;
        /* Warning: The BF_MTO and BF_MTF flags were meant to be mutually
         * exclusive, but I'm not checking that property like I should. */
    }

    if (txLib->sort == TS_ORDER_ADDED) {
        TXLIBITEM *item = txl_Append(txLib, itemSize);

        if (!item) {
            return TXLIB_ADDITEM_MEMORY_ERROR;
        }
        /*
         * Set the new item's fields.
         */
        item->order = txLib->nextOrder;
        item->type = type;
        item->dirty = TRUE;
        memset(item->comment, 0, TXLIBITEM_COMMENT_SIZE);
        if (type == META_BNDL) {
            int n = sprintf(item->comment, "%d VMEM"
                    , (bndlFmt & BF_VOICE_MASK) + 1);
            if (bndlFmt & BF_FX) {
                n += sprintf(&item->comment[n], ", FX");
            }
            if (bndlFmt & BF_MTO) {
                n += sprintf(&item->comment[n], ", MTO");
            }
            if (bndlFmt & BF_MTF) {
                sprintf(&item->comment[n], ", MTF");
            }
        }
        memcpy(item->data, data, item->size);
        /*
         * Search the list for a duplicate item.
         */
        if (options & TAO_NO_DUPLICATES) {
            int duplicate;

            /*
             * Search for a duplicate in the list.
             */
            duplicate = txl_FindDuplicateItem(txLib, item);
            /*
             * If a duplicate was found.
             */
            if (duplicate) {
                /*
                 * Delete the new item from the list.
                 */
                txl_DeleteItem(txLib);
                /*
                 * Set the list pointer to the item that was duplicated.
                 */
                TxLib_SetPtr(txLib, duplicate);
                /*
                 * Return the negative one-based index of the duplicate.
                 */
                return -duplicate;
            }
        }
        /*
         * Search the list for a duplicate name.
         */
        if (options & TAO_UNIQUE_NAMES) {
            txl_MakeNameUnique(txLib, item);
        }
        /*
         * Do some bookkeeping for the list.
         */
        txLib->nextOrder++;
        txLib->dirty |= TRUE;
        /*
         * Return the one-based index of the new item.
         */
        return txLib->count;
    } else {
        TXLIBNODE *itemNode = txl_NewNode(itemSize);
        TXLIBITEM *item;
        
        if (!itemNode) {
            return TXLIB_ADDITEM_MEMORY_ERROR;
        }
        item = &itemNode->item;

        /*
         * Set the new item's fields.
         */
        item->order = txLib->nextOrder;
        item->type = type;
        item->dirty = TRUE;
        memset(item->comment, 0, TXLIBITEM_COMMENT_SIZE);
        if (type == META_BNDL) {
            int n = sprintf(item->comment, "%d VMEM"
                    , (bndlFmt & BF_VOICE_MASK) + 1);
            if (bndlFmt & BF_FX) {
                n += sprintf(&item->comment[n], ", FX");
            }
            if (bndlFmt & BF_MTO) {
                n += sprintf(&item->comment[n], ", MTO");
            }
            if (bndlFmt & BF_MTF) {
                sprintf(&item->comment[n], ", MTF");
            }
        }
        memcpy(item->data, data, item->size);
        /*
         * Search the list and see if an identical item already exists.
         */
        if (options & TAO_NO_DUPLICATES) {
            int duplicate = txl_FindDuplicateItem(txLib, item);

            if (duplicate) {
                /*
                 * Don't add the item if one was found.
                 */
                free(itemNode);
                return -duplicate;
            }
        }
        /*
         * Search the list for a duplicate name.
         */
        if (options & TAO_UNIQUE_NAMES) {
            txl_MakeNameUnique(txLib, item);
        }
        /*
         * Do some bookkeeping for the library.
         */
        txLib->nextOrder++;
        txLib->dirty |= TRUE;
        /*
         * Add the node to the list and return its one-based index.
         */
        assert(txLib->sort < txl_cmpFuncCnt);
        return 1 + txl_Add(txLib, itemNode, txl_cmpFuncs[txLib->sort]);
    }
    return TXLIB_ADDITEM_MEMORY_ERROR;
}

/*
 * Clean() - Clears dirty flags on all items.
 */
void TxLib_Clean(TXLIB *txLib)
{
    TXLIBITEM *item;

    TxLib_Head(txLib);
    while (item = TxLib_Next(txLib)) {
        item->dirty = FALSE;
    }
    txLib->dirty = FALSE;
}

/*
 * DeInit() - Destroys a TXLIB object.
 */
void TxLib_DeInit(TXLIB *txLib)
{
	TXLIBNODE *tmp = &txLib->head;
	TXLIBNODE *p = tmp->next;

    /*
     * If the library was never initialized, return.
     */
    if (tmp->next == NULL) {
        return;
    }
	/*
	 * Walk through the list and delete all the nodes.
	 */
	while (p != &txLib->head) {
		tmp = p;
		p = p->next;
		free(tmp);
	}
}

/*
 * DeleteItem() - Deletes the item at itemIndex.
 */
void TxLib_DeleteItem(TXLIB *txLib, int itemIndex)
{
    TxLib_SetPtr(txLib, itemIndex);
    txl_DeleteItem(txLib);
    txLib->dirty = TRUE;
}

/*
 * Export() - Exports items from a file.
 */
void TxLib_Export(HWND mainWnd, TXLIB *txLib, int *libSelItems
        , int libSelItemCnt)
{
    _TUCHAR fileName[_MAX_PATH];
    int batchNumber;
    int batchStart;
    TXLIBNODE *savedPtr;
    int savedPtrIndex;
    int selIndex; /* the current selected item */
    TXLIBITEM *item;
    FILE *outFile;
    int fileItemCnt;  /* the current file item */
    _TUCHAR errorMsg[ERRORMSG_LEN];
    size_t errorMsgLen;
    BOOL selectedOnly = FALSE;
    BOOL sysexHeaders;
    BOOL appendFile;
    METAINDEX exportType = META_VMEM;
    unsigned char checksum;
    int i;

    /*
     * For each export type (first VMEMs, then PMEMs).
     */
    for (exportType = META_VMEM; exportType <= META_PMEM; exportType++) {
        batchNumber = 1;
        selIndex = 0;
        TxLib_Head(txLib);
        batchStart = -1;
        /*
         * Find the first item that has the same type as the type being
         * exported.
         */
        do {
            item = TxLib_Next(txLib);
            batchStart++;
        } while (item && item->type != exportType);
        /*
         * While items are left and either the current item is less than the
         * number of items in the library, or if selected items are being
         * exported, the selected index is less than the number of selected
         * items.
         */
        while (item && ((selectedOnly && selIndex < libSelItemCnt)
                    || batchStart < txLib->count))
        {
            /*
             * Show export dialog and get the next batch of patches and the
             * file name.
             */
            savedPtr = txLib->ptr;
            savedPtrIndex = txLib->ptrIndex;
            if (ExportDlg_Create(mainWnd, txLib, libSelItems, libSelItemCnt
                        , batchNumber, batchStart, &exportType
                        , Prog_exportRecentDir, fileName, _MAX_PATH))
            {
                batchNumber++;
                selectedOnly = (Prog_exportOptions & EO_SELECTED_ONLY) != 0;
                sysexHeaders = (Prog_exportOptions & EO_SYSEX_HEADERS) != 0;
                appendFile = (Prog_exportOptions & EO_APPEND_FILE) != 0;
                /*
                 * If selected items are being exported, advance the library
                 * pointer to the next selected item;
                 */
                if (selectedOnly && batchStart != libSelItems[selIndex]) {
                    batchStart = libSelItems[selIndex];
                    item = TxLib_SetPtr(txLib, batchStart);
                } else {
                    txLib->ptr = savedPtr;
                    txLib->ptrIndex = savedPtrIndex;
                    item = TxLib_This(txLib);
                }
                selIndex++;
                while (item && item->type != exportType)
                {
                    if (selectedOnly) {
                        batchStart = libSelItems[selIndex];
                        item = TxLib_SetPtr(txLib, batchStart);
                        selIndex++;
                    } else {
                        batchStart++;
                        item = TxLib_Next(txLib);
                    }
                }
                /*
                 * Open the next file.
                 */
                outFile = _tfopen(fileName, appendFile ? _T("ab") : _T("wb"));
                if (!outFile) {
                    errorMsgLen = _sntprintf(errorMsg, ERRORMSG_LEN
                            , Prog_fileOpenErrorFmt, fileName);
                    FromAnsiNCopy(&errorMsg[errorMsgLen]
                            , strerror(errno), ERRORMSG_LEN - errorMsgLen);
                    errorMsg[ERRORMSG_LEN - 1] = '\0';
                    MsgBox_F(mainWnd, errorMsg);
                    return;
                }
                if (sysexHeaders) {
                    fwrite(TX81Z_meta[exportType].dumpHdr, 1
                            , TX81Z_meta[exportType].dumpHdrLen, outFile);
                    checksum = (exportType == META_VMEM) ? 0 : 0x54;
                }
                /*
                 * For each item to be written to the file.
                 */
                for (fileItemCnt = 0; fileItemCnt < 32; fileItemCnt++) {
                    /*
                     * If there is an item left in the library, write it
                     * to the file.
                     */
                    if (item != NULL) {
                        signed dataSize = item->size;

                        fwrite(item->data, dataSize, 1, outFile);
                        if (sysexHeaders) {
                            for (i = 0; i < dataSize; i++) {
                                checksum += item->data[i];
                            }
                        }
                    /*
                     * Else there are no items left.  Write an init patch
                     * to the file.
                     */
                    } else if (exportType == META_VMEM) {
                        fwrite(TX81Z_initVMem, TX81Z_initVMemLen, 1, outFile);
                        if (sysexHeaders) {
                            for (i = 0; i < TX81Z_initVMemLen; i++) {
                                checksum += TX81Z_initVMem[i];
                            }
                        }
                    } else {
                        fwrite(TX81Z_initPMemSingle, TX81Z_initPMemLen, 1
                                , outFile);
                        if (sysexHeaders) {
                            for (i = 0; i < TX81Z_initPMemLen; i++) {
                                checksum += TX81Z_initPMemSingle[i];
                            }
                        }
                    }
                    /*
                     * If items are left in the library, find the next item.
                     */
                    if (item != NULL) {
                        do {
                            /*
                             * Get next item and break out of the while loop
                             * if there are none left.
                             */
                            if (selectedOnly) {
                                if (selIndex >= libSelItemCnt) {
                                    item = NULL;
                                    break;
                                }
                                batchStart = libSelItems[selIndex];
                                item = TxLib_SetPtr(txLib, batchStart);
                                selIndex++;
                            } else {
                                batchStart++;
                                if ((item = TxLib_Next(txLib)) == NULL) {
                                    break;
                                }
                            }
                        } while (item->type != exportType);
                    }
                }
                if (sysexHeaders) {
                    putc((0x100 - checksum) & 0x7F, outFile);
                    putc(0xF7, outFile);
                }
                fclose(outFile);
            /*
             * Bail out of the export operation when user hits cancel.
             */
            } else {
                return;
            }
        }
    }
}

/*
 * FormatName() - Formats a display string for the current item.
 */
void TxLib_FormatName(TXLIB *txLib, NAME_FORMAT fmtFlags, _TUCHAR *dest)
{
    int i = 0;

    if (fmtFlags & NF_TYPE) {
        METAINDEX type = TxLib_GetItemType(txLib);
        const _TUCHAR *typeStr;

        if (type == META_VMEM) {
            typeStr = Prog_vMemStr;
        } else if (type == META_PMEM) {
            typeStr = Prog_pMemStr;
        } else if (type == META_BNDL) {
            typeStr = Prog_bndlStr;
        } else {
            typeStr = TX81Z_meta[type].typeStr;
        }
        _tcscpy(dest, typeStr);
        dest[4] = ' ';
        dest[5] = '\0';
        i += 5;
    }
    if (fmtFlags & NF_NAME) {
        TxLib_GetItemName(txLib, &dest[i]);
        i += 10;
    }
    dest[i] = ' ';
    i++;
    dest[i] = '\0';
    if (fmtFlags & NF_COMMENT) {
        TxLib_GetItemComment(txLib, &dest[i]);
    }
}

/*
 * GetItemComment() - Copies the comment of the current item into the buffer.
 */
void TxLib_GetItemComment(TXLIB *txLib, _TUCHAR comment[TXLIBITEM_COMMENT_SIZE])
{
    TXLIBITEM *item = TxLib_This(txLib);

    FromAnsiNCopy(comment, item->comment, TXLIBITEM_COMMENT_SIZE - 1);
    comment[TXLIBITEM_COMMENT_SIZE - 1] = '\0';
}

/*
 * GetItemName() - Copies the name of the current item into the buffer.
 */
void TxLib_GetItemName(TXLIB *txLib, _TUCHAR name[TXLIBITEM_NAME_SIZE])
{
    TXLIBITEM *item = TxLib_This(txLib);

    txl_GetItemName(item, name);
}

/*
 * Head() - Sets the internal list pointer to the head node.
 */
void TxLib_Head(TXLIB *txLib)
{
    txLib->ptr = &txLib->head;
    txLib->ptrIndex = -1;
}

/*
 * Import() - Imports items from a file.
 */
void TxLib_Import(HWND mainWnd, TXLIB *txLib, IMPORT_OPTIONS importOptions
        , _TUCHAR *fileNames)
{
    _TUCHAR path[_MAX_PATH], *p = fileNames;
    BOOL voiceCheck = (importOptions & IO_VMEMS) != 0;
    BOOL pfmCheck = (importOptions & IO_PMEMS) != 0;
    BOOL checkEveryPos = (importOptions & IO_CHECK_EVERY_POS) != 0;

    /*
     * For each filename.
     */
    while (FileDlg_ParseFileNames(fileNames, path, &p)) {
        BYTE *buf;
        size_t bufSize;
        _TUCHAR errorMsg[ERRORMSG_LEN];
        size_t i, j;

        /*
         * Open the next file.
         */
        buf = File_Load(path, &bufSize, errorMsg, ERRORMSG_LEN);
        if (!buf) {
            MsgBox_F(mainWnd, errorMsg);
            return;
        }
        /*
         * Make sure the file is big enough to possibly hold an item.
         */
        if (bufSize >= S_PCED_SIZE) {
            /*
             * Scan each file byte-by-byte.
             */
            for (i = 0; i <= bufSize - S_PCED_SIZE; i++) {
                /*
                 * A valid item will start with a sysex status byte and a
                 * Yamaha identifier.  The top half of the third byte will
                 * also be clear (the lower half contains the channel number,
                 * which could be anything, I suppose).
                 */
                if (buf[i] == 0xF0 && buf[i + 1] == 0x43
                        && (buf[i + 2] & 0xF0) == 0)
                {
                    /*
                     * Check for a VMEM bank header.
                     */
                    if (voiceCheck && buf[i + 3] == 0x04
                            && i <= bufSize - S_VMEM_SIZE
                            && buf[i + 4] == 0x20
                            && buf[i + 5] == 0x00)
                    {
                        /*
                         * Load 32 VMEM voices.
                         */
                        i += S_VMEM_HDR_SIZE;
                        for (j = 0; j < 32; j++) {
                            if (TxLib_AddItem(txLib, META_VMEM, &buf[i]
                                    , importOptions)
                                        == TXLIB_ADDITEM_MEMORY_ERROR)
                            {
                                return;
                            }
                            i += S_VMEM_DATA_SIZE;
                        }
                        /*
                         * Skip over the sysex footer.
                         */
                        i += 2;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    /*
                     * Check for a PMEM bank header.
                     */
                    } else if (pfmCheck && buf[i + 3] == 0x7E
                            && i <= bufSize - S_PMEM_SIZE
                            && buf[i + 4] == 0x13 && buf[i + 5] == 0x0A
                            && strncmp(&buf[i + 6], "LM  8976PM", 10) == 0)
                    {
                        /*
                         * Load 24 PMEM performances.
                         */
                        i += S_PMEM_HDR_SIZE;
                        for (j = 0; j < 24; j++) {
                            if (TxLib_AddItem(txLib, META_PMEM, &buf[i]
                                    , importOptions)
                                        == TXLIB_ADDITEM_MEMORY_ERROR)
                            {
                                return;
                            }
                            i += S_PMEM_DATA_SIZE;
                        }
                        /*
                         * Skip over the 8 empty performances and the sysex
                         * footer.
                         */
                        i += S_PMEM_DATA_SIZE * 8 + 2;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    /*
                     * Check for an ACED header and also a VCED following.
                     */
                    } else if (voiceCheck
                            && i <= bufSize - S_ACED_SIZE - S_VCED_SIZE
                            && buf[i + 3] == 0x7E
                            && buf[i + 4] == 0x00 && buf[i + 5] == 0x21
                            && strncmp(&buf[i + 6], "LM  8976AE", 10) == 0
                            && buf[i + S_ACED_SIZE] == 0xF0
                            && buf[i + S_ACED_SIZE + 1] == 0x43
                            && (buf[i + S_ACED_SIZE + 2] & 0xF0) == 0
                            && buf[i + S_ACED_SIZE + 3] == 0x03
                            && buf[i + S_ACED_SIZE + 4] == 0x00
                            && buf[i + S_ACED_SIZE + 5] == 0x5D)
                    {
                        /*
                         * Convert the AVCED to a VMEM and load it.
                         */
                        BYTE vmemData[S_VMEM_SIZE];

                        TXPack_AvcedToVmem(&buf[i + S_ACED_HDR_SIZE]
                                , &buf[i + S_ACED_SIZE + S_VCED_HDR_SIZE]
                                , vmemData);
                        if (TxLib_AddItem(txLib, META_VMEM, vmemData
                                    , importOptions)
                                        == TXLIB_ADDITEM_MEMORY_ERROR)
                        {
                            return;
                        }
                        /*
                         * Skip over the AVCED.
                         */
                        i += S_ACED_SIZE + S_VCED_SIZE;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    /*
                     * Check for a PCED header.
                     */
                    } else if (pfmCheck && buf[i + 3] == 0x7E
                            && buf[i + 4] == 0x00 && buf[i + 5] == 0x78
                            && strncmp(&buf[i + 6], "LM  8976PE", 10) == 0)
                    {
                        /*
                         * Convert the PCED to a PMEM and load it.
                         */
                        BYTE pmemData[S_PMEM_SIZE];

                        TXPack_PcedToPmem(&buf[i + S_PCED_HDR_SIZE], pmemData);
                        if (TxLib_AddItem(txLib, META_PMEM, pmemData
                                    , importOptions)
                                        == TXLIB_ADDITEM_MEMORY_ERROR)
                        {
                            return;
                        }
                        /*
                         * Skip over the PCED.
                         */
                        i += S_PCED_SIZE;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    }
                } else if (voiceCheck && txl_CheckBufForVmem(&buf[i])) {
                    if (TxLib_AddItem(txLib, META_VMEM, &buf[i]
                            , importOptions) == TXLIB_ADDITEM_MEMORY_ERROR)
                    {
                        return;
                    }
                    if (!checkEveryPos) {
                        /*
                         * Skip over the VMEM data.
                         */
                        i += S_VMEM_DATA_SIZE;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    }
                } else if (pfmCheck && txl_CheckBufForPmem(&buf[i])) {
                    if (TxLib_AddItem(txLib, META_PMEM, &buf[i]
                            , importOptions) == TXLIB_ADDITEM_MEMORY_ERROR)
                    {
                        return;
                    }
                    if (!checkEveryPos) {
                        /*
                         * Skip over the PMEM data.
                         */
                        i += S_PMEM_DATA_SIZE;
                        /*
                         * Decrement by one to anticipate the next loop
                         * increment.
                         */
                        i--;
                    }
                }
            }
        }
        free(buf);
    }
}

/*
 * Init() - Initializes a TXLIB object.
 */
void TxLib_Init(TXLIB *txLib)
{
    txLib->head.prev = txLib->head.next = txLib->ptr = &txLib->head;
    TxLib_New(txLib);
}

/*
 * Load() - Loads a library from a file.
 */
BOOL TxLib_Load(HWND mainWnd, TXLIB *txLib)
{
    _TUCHAR errorMsg[ERRORMSG_LEN];
    size_t errorMsgLen;
    char fileHdr[20];
    size_t hdrLen = 0;
    char progNameVerStr[20];
    _TUCHAR *path = txLib->fileName;
    int c;
    FILE *inFile = _tfopen(path, _T("rb"));

    if (!inFile) {
        errorMsgLen = _sntprintf(errorMsg, ERRORMSG_LEN, Prog_fileOpenErrorFmt
                , path);
        goto FILEERROR;
    }
    /*
     * Read the version header from the file.
     */
    while ((c = getc(inFile)) != EOF) {
        fileHdr[hdrLen++] = c;
        if (c == 0)
            break;
    }
    /*
     * Check the file version.
     */
    ToAnsiCopy(progNameVerStr, tx_fileHdr);
    if (strcmp(fileHdr, progNameVerStr) != 0) {
        MsgBox_F(mainWnd, _T("This file was made by a different version of the program,\r\n")
                _T("or maybe even a different program altogether, or the file is corrupt.\r\n")
                _T("In other words, this file can't be read."));
        return FALSE;
    }

    /*
     * If there is already a library open, clear it.
     */
    if (!TxLib_IsEmpty(txLib)) {
        txl_Clear(txLib);
    }
    txLib->count = 0;
    txLib->nextOrder = 0;
    txLib->sort = TS_ORDER_ADDED;

    /*
     * Read the items.
     */
    for (;;) {
        size_t dataSize;
        METAINDEX type;
        TXLIBITEM *item;

        /*
         * Read the data size.
         */
        if (fread(&dataSize, 1, sizeof(size_t), inFile) != sizeof(size_t))
            break;
        /*
         * If the data size is greater than the largest possible item (a
         * performance bundle with 8 VMEM's, FX, and MTF table), then the
         * file is corrupted.
         */
        if (dataSize > S_MAX_BNDL_SIZE) {
            goto BadFile;
        }
        /*
         * Read the data type.
         */
        if ((type = getc(inFile)) == EOF)
            break;
        /*
         * Verify the type.
         */
        if (type < 0 || type > META_BNDL) {
            goto BadFile;
        }
        /*
         * Check the size to see if it matches the type.  The bundle type is
         * too complicated and can't be checked until the bundle format has
         * been read in it can't be checked here.
         */
        if (type < META_CNT) {
            if (dataSize != TX81Z_meta[type].libSize) {
                goto BadFile;
            }
        }
        /*
         * Create a new node for the list.
         */
        item = txl_Append(txLib, dataSize);
        if (!item) {
            fclose(inFile);
            return FALSE;
        }
        item->order = txLib->nextOrder++;
        item->type = type;
        item->dirty = FALSE;
        
        if (fread(item->comment, 1, TXLIBITEM_COMMENT_MAX, inFile)
                != TXLIBITEM_COMMENT_MAX)
        {
            break;
        }
        if (fread(item->data, 1, dataSize, inFile) != dataSize) {
            break;
        }
        /*
         * Check the size on bundles.
         */
        if (type == META_BNDL) {
            BNDL_FORMAT *bndlFmt = (BNDL_FORMAT *) item->data;
            size_t bndlSize;

            bndlSize = sizeof(BNDL_FORMAT) + S_PMEM_DATA_SIZE
                + (((*bndlFmt & BF_VOICE_MASK) + 1) * S_VMEM_DATA_SIZE)
                + (((*bndlFmt & BF_FX) != 0) * S_FX_DATA_SIZE)
                + (((*bndlFmt & BF_MTO) != 0) * S_MTO_DATA_SIZE)
                + (((*bndlFmt & BF_MTF) != 0) * S_MTF_DATA_SIZE);
            if (bndlSize != dataSize) {
BadFile:
                MsgBox_F(mainWnd, _T("There was problem loading %s.\r\n")
                        _T("It may be corrupted."), path);
                fclose(inFile);
                return FALSE;
            }
        }
    }
    if (ferror(inFile)) {
        goto READERROR;
    }
    fclose(inFile);
    txLib->dirty = FALSE;

    return TRUE;

READERROR:
    errorMsgLen = _sntprintf(errorMsg, ERRORMSG_LEN, Prog_fileReadErrorFmt
            , path);
FILEERROR:
    FromAnsiNCat(errorMsg, strerror(errno), ERRORMSG_LEN - errorMsgLen);
    errorMsg[ERRORMSG_LEN - 1] = '\0';
    MsgBox_F(mainWnd, errorMsg);
    if (inFile) {
        fclose(inFile);
    }

    return FALSE;
}

/*
 * New() - Resets a library.
 */
void TxLib_New(TXLIB *txLib)
{
    txl_Clear(txLib);
    txLib->fileName[0] = '\0';
    txLib->count = 0;
    txLib->nextOrder = 0;
    txLib->sort = TS_ORDER_ADDED;
    txLib->dirty = FALSE;
}

/*
 * Next() - Set the list pointer to the next node.
 */
TXLIBITEM *TxLib_Next(TXLIB *txLib)
{
	/*
	 * Advance internal pointer.
	 */
	txLib->ptr = txLib->ptr->next;
    /*
     * Increment the pointer index.
     */
    txLib->ptrIndex++;
	/*
	 * If the pointer circles around back to the head, then the whole list
	 * has been traversed.
	 */
	if (txLib->ptr == &txLib->head) {
        txLib->ptrIndex = -1;
		return NULL;
    }
	/*
	 * Return a pointer to the library item.
	 */
	return &txLib->ptr->item;
}

/*
 * Prev() - Moves the list pointer to the previous node.
 */
TXLIBITEM *TxLib_Prev(TXLIB *txLib)
{
    /*
     * Move internal pointer.
     */
    txLib->ptr = txLib->ptr->prev;
    /*
     * If the pointer was pointing to the head node, set the index to the
     * node count.
     */
    if (txLib->ptrIndex == -1) {
        txLib->ptrIndex = txLib->count;
    }
    /*
     * Decrement the pointer index.
     */
    txLib->ptrIndex--;
	/*
	 * If the pointer circles around back to the head, then the whole list
	 * has been traversed.
	 */
	if (txLib->ptr == &txLib->head) {
		return NULL;
    }
	/*
	 * Return a pointer to the library item.
	 */
	return &txLib->ptr->item;
}

/*
 * ReplaceLib() - Destroys the destination library and replaces it with the
 *                source library.
 */
void TxLib_ReplaceLib(TXLIB *destLib, TXLIB *srcLib)
{
    TXLIBNODE *oldHead = &srcLib->head;

    /*
     * Destroy the destination library.
     */
    TxLib_DeInit(destLib);
    /*
     * Copy the TXLIB structure fields.
     */
    memcpy(destLib, srcLib, sizeof(TXLIB));
    /*
     * Patch up the pointers pointing to <head>.
     */
    if (destLib->head.next == oldHead) {
        destLib->head.next = &destLib->head;
    }
    if (destLib->head.prev == oldHead) {
        destLib->head.prev = &destLib->head;
    }
    destLib->head.next->prev = destLib->head.prev->next = destLib->ptr
        = &destLib->head;
    destLib->ptrIndex = -1;
}

/*
 * Save() - Saves a library to a file.
 */
BOOL TxLib_Save(HWND mainWnd, TXLIB *txLib)
{
    _TUCHAR errorMsg[ERRORMSG_LEN];
    size_t errorMsgLen;
    char fileHdr[10];
    size_t hdrLen;
    _TUCHAR *path = txLib->fileName;
    FILE *outFile = _tfopen(path, _T("wb"));
    TXLIBITEM *item;
    int i;

    if (!outFile) {
        errorMsgLen = _sntprintf(errorMsg, ERRORMSG_LEN, Prog_fileOpenErrorFmt
                , path);
        goto FILEERROR;
    }

    /*
     * Write a version header to the file.
     */
    hdrLen = ToAnsiCopy(fileHdr, tx_fileHdr);
    fwrite(fileHdr, 1, hdrLen, outFile);

    /*
     * Write the items to the file in insert order.
     */
    TxLib_Head(txLib);
    for (i = 0; i < txLib->nextOrder; i++) {
        TXLIBITEM *breakItem = item = TxLib_Next(txLib);

        /*
         * If the previous loop stopped at the last item in the list then
         * item will be NULL, which crashes the program.
         */
        if (!breakItem) {
            breakItem = item = TxLib_Next(txLib);
        }
        do {
            if (item->order == i) {
                /*
                 * Write the item to the file.
                 */
                size_t dataSize = item->size;

                /*
                 * Write the size of the item (4 bytes).
                 */
                if (fwrite(&dataSize, 1, sizeof(size_t), outFile)
                        != sizeof(size_t))
                {
                    goto WRITEERROR;
                }
                /*
                 * Write the type of the item (1 byte).
                 */
                if (putc(item->type, outFile) == EOF) {
                    goto WRITEERROR;
                }
                /*
                 * Write the comment (40 bytes).
                 */
                if (fwrite(item->comment, 1, TXLIBITEM_COMMENT_MAX, outFile)
                        != TXLIBITEM_COMMENT_MAX)
                {
                    goto WRITEERROR;
                }
                /*
                 * Write the data (variable length).
                 */
                if (fwrite(item->data, 1, dataSize, outFile) != dataSize) {
                    goto WRITEERROR;
                }
                /*
                 * Clear the item's dirty flag.
                 */
                item->dirty = FALSE;
                goto NextI;
            }
            item = TxLib_Next(txLib);
            if (!item) {
                item = TxLib_Next(txLib);
            }
        } while (item != breakItem);
NextI:  
        ;
    }
    fclose(outFile);
    txLib->dirty = FALSE;

    return TRUE;

WRITEERROR:
    errorMsgLen = _sntprintf(errorMsg, ERRORMSG_LEN, Prog_fileWriteErrorFmt
            , path);
FILEERROR:
    FromAnsiNCat(errorMsg, strerror(errno), ERRORMSG_LEN - errorMsgLen);
    errorMsg[ERRORMSG_LEN - 1] = '\0';
    MsgBox_F(mainWnd, errorMsg);
    if (outFile) {
        fclose(outFile);
    }

    return FALSE;
}

/*
 * SetItemComment() - Sets the current item's comment.
 */
void TxLib_SetItemComment(TXLIB *txLib, _TUCHAR *comment)
{
    TXLIBITEM *item = TxLib_This(txLib);
    int i;

    /*
     * Compare the comments and bail out if they are the same to avoid
     * unnecessarily dirtying the item.
     */
    for (i = 0; i < TXLIBITEM_COMMENT_SIZE - 1; i++) {
        if (item->comment[i] != comment[i])
            goto SetComment;
    }
    return;
SetComment:
    /*
     * Set the item's comment.
     */
    ToAnsiNCopy(item->comment, comment, TXLIBITEM_COMMENT_SIZE - 1);
    /*
     * Dirty the item and the library.
     */
    item->dirty = txLib->dirty = TRUE;
}

/*
 * SetItemName() - Sets the current item's name.
 */
void TxLib_SetItemName(TXLIB *txLib, _TUCHAR *name)
{
    TXLIBITEM *item = TxLib_This(txLib);
    size_t nameOffset;
    int i;

    switch (item->type) {
        case META_VMEM:
            nameOffset = TX81Z_VMEM_NAME;
            break;
        case META_PMEM:
            nameOffset = TX81Z_PMEM_NAME;
            break;
        case META_BNDL:
            nameOffset = sizeof(BNDL_FORMAT) + TX81Z_PMEM_NAME;
            break;
        default:
            /*
             * None of the other item types have a name.
             */
            return;
    }
    /*
     * Compare the names and bail out if they are the same to avoid
     * unnecessarily dirtying the item.
     */
    for (i = 0; i < TXLIBITEM_NAME_SIZE - 1; i++) {
        if (item->data[nameOffset + i] != name[i])
            goto SetName;
    }
    return;
SetName:
    /*
     * Copy the name.
     */
    ToAnsiNCopy(&item->data[nameOffset], name, TXLIBITEM_NAME_SIZE - 1);
    /*
     * Dirty the item and the library.
     */
    item->dirty = txLib->dirty = TRUE;
}

/*
 * SetPtr() - Sets the internal item pointer to the <itemIndex>th item.
 */
TXLIBITEM *TxLib_SetPtr(TXLIB *txLib, int itemIndex)
{
    TXLIBITEM *item;
    int ptrIndex = txLib->ptrIndex;
    int count = txLib->count;

    /*
     * If the requested item is the same as the current item, return the
     * current item.
     */
    if (ptrIndex == itemIndex) {
        return TxLib_This(txLib);
    }
    /*
     * If the pointer needs to be advanced forward.
     */
    if (ptrIndex < itemIndex) {
        /*
         * If it would be faster starting at the end and going backwards,
         * then do it that way.
         */
        if (count - itemIndex < itemIndex - ptrIndex) {
            TxLib_Head(txLib);
            do {
                item = TxLib_Prev(txLib);
            } while (item && txLib->ptrIndex != itemIndex);
        } else {
            do {
                item = TxLib_Next(txLib);
            } while (item && txLib->ptrIndex != itemIndex);
        }
    /*
     * Else the pointer needs to go backwards.
     */
    } else {
        /*
         * If it would be faster starting at the beginning and going forwards,
         * then do it that way.
         */
        if (itemIndex - 0 < ptrIndex - itemIndex) {
            TxLib_Head(txLib);
            do {
                item = TxLib_Next(txLib);
            } while (item && txLib->ptrIndex != itemIndex);
        } else {
            do {
                item = TxLib_Prev(txLib);
            } while (item && txLib->ptrIndex != itemIndex);
        }
    }

    return item;
}

/*
 * Sort() - Sorts the items.
 */
void TxLib_Sort(TXLIB *txLib, TXLIB_SORT sort)
{
    assert(sort >= 0 && sort < txl_cmpFuncCnt);
    txLib->sort = sort;
    txl_Sort(txLib, txl_cmpFuncs[sort]);
}

/*
 * Add() - Adds a node to the list in sorted order and returns its index.
 */
int txl_Add(TXLIB *txLib, TXLIBNODE *node, TXLIBITEMCMPFUNC Cmp)
{
    TXLIBNODE *p = &txLib->head;
    int index = 0;

    while ((p = p->next) != &txLib->head && Cmp(&node->item, &p->item) >= 0) {
        index++;
    }
    node->next = p;
    node->prev = p->prev;
    node->prev->next = node;
    p->prev = node;
    txLib->count++;
    txLib->ptr = node;
    txLib->ptrIndex = index;

    return index;
}

/*
 * Append() - Appends an item to the library list.
 */
TXLIBITEM *txl_Append(TXLIB *txLib, size_t size)
{
	TXLIBNODE *node;

	/*
	 * Create a new list node.
	 */
    node = txl_NewNode(size);
	if (!node) {
		return NULL;
	}
	/*
	 * Insert the node into the list.
	 */
	node->next = &txLib->head;
	node->prev = txLib->head.prev;
	txLib->head.prev->next = node;
	txLib->head.prev = node;
	/*
	 * Set internal pointer to the new node.
	 */
	txLib->ptr = node;
    /*
     * Set the pointer index to the item count - 1, but it's going to be
     * incremented, so plain old count is fine here.
     */
    txLib->ptrIndex = txLib->count;
    /*
     * Increment the item count.
     */
    txLib->count++;
	/*
	 * Return the uninitialized record.
	 */
	return &node->item;
}

/*
 * CheckBufForPmem() - Scans a sysex buffer and returns true if it contains
 *                     a performance bank.
 */
BOOL txl_CheckBufForPmem(BYTE *buf)
{
    int i;

    for (i = 0; i < TXPack_pcedCnt; i++) {
        const TXPACK *pack = &TXPack_pced[i];
        BYTE value = (buf[pack->memOffset] >> pack->memFirstBit)
            & TXPack_masks[pack->memBitCnt];

        if (value < pack->loLimit || value > pack->hiLimit)
            return FALSE;
    }
    return TRUE;
}

/*
 * CheckBufForVmem() - Scans a sysex buffer and returns true if it contains
 *                     a voice bank.
 */
BOOL txl_CheckBufForVmem(BYTE *buf)
{
    int i;

    for (i = 0; i < TXPack_vcedCnt; i++) {
        const TXPACK *pack = &TXPack_vced[i];
        BYTE value = (buf[pack->memOffset] >> pack->memFirstBit)
            & TXPack_masks[pack->memBitCnt];

        if (value < pack->loLimit || value > pack->hiLimit)
            return FALSE;
    }
    for (i = 0; i < TXPack_acedCnt; i++) {
        const TXPACK *pack = &TXPack_aced[i];
        BYTE value = (buf[pack->memOffset] >> pack->memFirstBit)
            & TXPack_masks[pack->memBitCnt];

        if (value < pack->loLimit || value > pack->hiLimit)
            return FALSE;
    }
    return TRUE;
}

/*
 * Clear() - Deletes all nodes from the list.
 */
void txl_Clear(TXLIB *txLib)
{
    txLib->ptr = txLib->head.next;
	while (txl_DeleteItem(txLib))
		;
    txLib->ptrIndex = -1;
}

/*
 * CmpComment() - Does a string comparison of two item comments for sorting.
 */
int txl_CmpComment(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    /*
     * Return the result of the comparison only if either comment exists.
     */
    if (newItem->comment[0] || listItem->comment[0]) {
        return strncmp(newItem->comment, listItem->comment
                , TXLIBITEM_COMMENT_SIZE);
    }
    return newItem->order - listItem->order;
}

/*
 * CmpName() - Does a string comparison of two item names for sorting.
 */
int txl_CmpName(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    const char *newName = newItem->type == META_VMEM
        ? &newItem->data[TX81Z_VMEM_NAME]
        : newItem->type == META_PMEM
        ? &newItem->data[TX81Z_PMEM_NAME]
        : emptyAnsiStr;
    const char *listItemName = listItem->type == META_VMEM
        ? &listItem->data[TX81Z_VMEM_NAME]
        : listItem->type == META_PMEM
        ? &listItem->data[TX81Z_PMEM_NAME]
        : emptyAnsiStr;

    return strncmp(newName, listItemName, TXLIBITEM_NAME_SIZE);
}

/*
 * CmpOrder() - Does a comparison of two item order values for sorting.
 */
int txl_CmpOrder(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    return newItem->order - listItem->order;
}

/*
 * CmpSearch() - Scans item comments and names to determine sorting based on
 *               the search string TxLib_searchStr.
 */
int txl_CmpSearch(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    const char *newName;
    const char *listItemName;
    const char *newComment;
    const char *listItemComment;
    const char *newSearch;
    const char *listItemSearch;

    newName = newItem->type == META_VMEM
        ? &newItem->data[TX81Z_VMEM_NAME]
        : newItem->type == META_PMEM
        ? &newItem->data[TX81Z_PMEM_NAME]
        : emptyAnsiStr;
    listItemName = listItem->type == META_VMEM
        ? &listItem->data[TX81Z_VMEM_NAME]
        : listItem->type == META_PMEM
        ? &listItem->data[TX81Z_PMEM_NAME]
        : emptyAnsiStr;
    if (TxLib_searchType == TST_NAME) {
        newSearch = StrStrIA(newName, TxLib_searchStr);
        listItemSearch = StrStrIA(listItemName, TxLib_searchStr);
    } else if (TxLib_searchType == TST_COMMENT) {
        newComment = newItem->comment;
        listItemComment = listItem->comment;
        newSearch = StrStrIA(newComment, TxLib_searchStr);
        listItemSearch = StrStrIA(listItemComment, TxLib_searchStr);
    } else {
        newComment = newItem->comment;
        listItemComment = listItem->comment;
        newSearch = StrStrIA(newName, TxLib_searchStr);
        if (!newSearch) {
            newSearch = StrStrIA(newComment, TxLib_searchStr);
        }
        listItemSearch = StrStrIA(listItemName, TxLib_searchStr);
        if (!listItemSearch) {
            listItemSearch = StrStrIA(listItemComment, TxLib_searchStr);
        }
    }
    /*
     * If newItem contains search string and listItem doesn't.
     */
    if (newSearch && listItemSearch == NULL) {
        return -1;
    /*
     * If listItem contains search string and newItem doesn't.
     */
    } else if (listItemSearch && newSearch == NULL) {
        return 1;
    /*
     * Else return straight order comparison.
     */
    } else {
        return newItem->order - listItem->order;
    }
    return 0;
}

/*
 * CmpSelection() - Compares items with selection array for sorting.
 */
int txl_CmpSelection(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    int i;
    int ni = -1;
    int li = -1;

    /*
     * Find the items in the selection array.
     */
    for (i = 0; i < *TxLib_libSelItemCnt; i++) {
        if (newItem->order == TxLib_libSelItems[i]) {
            ni = i;
            if (li != -1) {
                break;
            }
        }
        if (listItem->order == TxLib_libSelItems[i]) {
            li = i;
            if (ni != -1) {
                break;
            }
        }
    }
    if (ni > -1 && li > -1) {
        return ni - li;
    } else if (ni == -1) {
        return 1;
    } else if (li == -1) {
        return -1;
    }
    return newItem->order - listItem->order;
}

/*
 * CmpType() - Compares item types for sorting.
 */
int txl_CmpType(const TXLIBITEM *newItem, const TXLIBITEM *listItem)
{
    METAINDEX newType = newItem->type;
    METAINDEX listType = listItem->type;

    if (newType == listType) {
        return newItem->order - listItem->order;
    }

    return newType - listType;
}

/*
 * DeleteItem() - Deletes a library item from the list.
 */
BOOL txl_DeleteItem(TXLIB *txLib)
{
    TXLIBNODE *tmp = txLib->ptr;

	/*
	 * Check for empty list.
	 */
	if (tmp == &txLib->head) {
		return FALSE;
    }
	/*
	 * Connect the previous and next nodes to each other.
	 */
	tmp->prev->next = tmp->next;
	tmp->next->prev = tmp->prev;
	/*
	 * Set the list pointer to the node after the deleted node, which puts it
     * at the same <ptrIndex> as before, unless it was the last item in the
     * list that was deleted.
	 */
	txLib->ptr = tmp->next;
    if (txLib->ptr == &txLib->head) {
        txLib->ptrIndex = -1;
    }
    txLib->count--;
	/*
	 * Free the node.
	 */
	free(tmp);

	return TRUE;
}

/*
 * FindDuplicateItem() - Scans the library for the item and returns the
 *                       one-based index of the duplicate if one is found.
 */
int txl_FindDuplicateItem(TXLIB *txLib, TXLIBITEM *item)
{
    TXLIBITEM *listItem;
    METAINDEX itemType = item->type;
    size_t itemSize = item->size;
    BYTE *itemData = item->data;
    TXLIBNODE *savedPtr = txLib->ptr;
    int savedPtrIndex = txLib->ptrIndex;
    int duplicate;

    TxLib_Head(txLib);
    if (itemType == META_VMEM || itemType == META_PMEM) {
        size_t nameStart = itemType == META_VMEM ? TX81Z_VMEM_NAME
            : TX81Z_PMEM_NAME;
        BYTE *postNamePtr = &itemData[nameStart + 10];
        size_t postNameLen = itemSize - nameStart - 10;

        while (listItem = TxLib_Next(txLib)) {
            if (listItem != item) {
                if (listItem->size == itemSize && listItem->type == itemType) {
                    /*
                     * Compare the two items, ignoring the name.
                     */
                    if (memcmp(listItem->data, itemData, nameStart) == 0
                            && memcmp(&listItem->data[nameStart + 10]
                                    , postNamePtr, postNameLen) == 0)
                    {
                        break;
                    }
                }
            }
        }
    } else {
        while (listItem = TxLib_Next(txLib)) {
            if (listItem != item) {
                if (listItem->size == itemSize && listItem->type == itemType) {
                    if (memcmp(listItem->data, itemData, itemSize) == 0) {
                        break;
                    }
                }
            }
        }
    }
    duplicate = txLib->ptrIndex + 1;
    txLib->ptr = savedPtr;
    txLib->ptrIndex = savedPtrIndex;

    return duplicate;
}

/*
 * GetItemName() - Gets the name of an item.
 */
void txl_GetItemName(TXLIBITEM *item, _TUCHAR name[TXLIBITEM_NAME_SIZE])
{
    int i;

    switch (item->type) {
        case META_VMEM:
            i = TX81Z_VMEM_NAME;
            goto Copy;
        case META_PMEM:
            i = TX81Z_PMEM_NAME;
            goto Copy;
        case META_BNDL:
            i = TX81Z_PMEM_NAME + sizeof(BNDL_FORMAT);
Copy:
            /*
             * Copy the item's name into the buffer.
             */
            FromAnsiNCopy(name, &item->data[i], 10);
            name[10] = '\0';
            break;
        case META_FX:
            _tcscpy(name, _T("Effects   "));
            break;
        case META_PC:
            _tcscpy(name, _T("PC Table  "));
            break;
        case META_MTO:
            _tcscpy(name, _T("Micro Oct "));
            break;
        case META_MTF:
            _tcscpy(name, _T("Micro Full"));
            break;
        case META_SYS:
            _tcscpy(name, _T("System    "));
            break;
    }
}

/*
 * IncrementNameSuffix() - Increments the digits at the end of an item name, or
 *                         adds a digit if none exists.
 */
void txl_IncrementNameSuffix(_TUCHAR name[TXLIBITEM_NAME_SIZE])
{
    int i = TXLIBITEM_NAME_SIZE - 2;

    while (name[i] == ' ' && name[i] == '\0') {
        i--;
    }
    if (!isdigit(name[i])) {
        if (i == TXLIBITEM_NAME_SIZE - 2) {
            name[i] = '0';
        } else {
            i++;
            name[i] = '0';
        }
    } else {
        if (++name[i] > '9') {
            if (i == TXLIBITEM_NAME_SIZE - 2) {
REPEAT:
                name[i] = '0';
                i--;
                if (!isdigit(name[i])) {
                    name[i] = '1';
                } else if (++name[i] > '9') {
                    goto REPEAT;
                }
            }
        }
    }
}

/*
 * MakeNameUnique() - Creates a unique item name.
 */
void txl_MakeNameUnique(TXLIB *txLib, TXLIBITEM *item)
{
    TXLIBITEM *listItem;
    TXLIBNODE *savedPtr = txLib->ptr;
    int savedPtrIndex = txLib->ptrIndex;
    BOOL nameUnique = FALSE;
    _TUCHAR listItemName[TXLIBITEM_NAME_SIZE];
    _TUCHAR itemName[TXLIBITEM_NAME_SIZE];

    txl_GetItemName(item, itemName);
    while (!nameUnique) {
        TxLib_Head(txLib);
        while (listItem = TxLib_Next(txLib)) {
            if (listItem != item) {
                if (listItem->type == item->type) {
                    txl_GetItemName(listItem, listItemName);
                    if (StrEq(listItemName, itemName)) {
                        txl_IncrementNameSuffix(itemName);
                        ToAnsiNCopy(&item->data[item->type == META_VMEM
                                ? TX81Z_VMEM_NAME : TX81Z_PMEM_NAME]
                                , itemName, 10);
                        break;
                    }
                }
            }
        }
        if (listItem == NULL) {
            nameUnique = TRUE;
        }
    }
    txLib->ptr = savedPtr;
    txLib->ptrIndex = savedPtrIndex;
}

/*
 * NewNode() - Creates a new list node.
 */
TXLIBNODE *txl_NewNode(size_t size)
{
	TXLIBNODE *node = malloc(sizeof(TXLIBNODE) - DUMMY_DATA_BYTE + size);

	if (!node) {
		Error_OnError(E_MALLOC_ERROR);
		return NULL;
	}
	node->item.size = size;
    return node;
}

/*
 * ReadVoiceBank() - Reads a voice bank from a file.  Returns nonzero on
 *                   success, or 0 if there was a memory error.
 */
int txl_ReadVoiceBank(FILE *inFile, TXLIB *txLib
        , IMPORT_OPTIONS importOptions)
{
#define BANK_SIZE (S_VMEM_HDR_SIZE + 32 * S_VMEM_DATA_SIZE + S_FTR_SIZE)
    unsigned char buf[BANK_SIZE];
    long filePos = ftell(inFile);
    int i;

    if (fread(buf, 1, BANK_SIZE, inFile) != BANK_SIZE) {
        /*
         * If the read is incomplete, restore the previous file position 
         * (incremented by 1 so it doesn't go in circles reading the same
         * thing over and over) and continue the normal search.
         */
        fseek(inFile, SEEK_SET, filePos + 1);
        return 1;
    }
    for (i = 0; i < 32; i++) {
        if (TxLib_AddItem(txLib, META_VMEM
                , &buf[S_VMEM_HDR_SIZE + i * S_VMEM_DATA_SIZE]
                , importOptions) == TXLIB_ADDITEM_MEMORY_ERROR)
        {
            return 0;
        }
    }
    return 1;
}

/*
 * Sort() - Sorts the library with the given comparison function.
 */
void txl_Sort(TXLIB *txLib, TXLIBITEMCMPFUNC Cmp)
{
    TXLIBNODE *nextUnsortedNode = txLib->head.next;
    TXLIBNODE *node;
    TXLIBNODE *listNode;

    while (nextUnsortedNode != &txLib->head) {
        /*
         * Unlink the next node from the list.
         */
        node = nextUnsortedNode;
        nextUnsortedNode = nextUnsortedNode->next;
        nextUnsortedNode->prev = node->prev;
        node->prev->next = nextUnsortedNode;
        /*
         * Scan through the sorted nodes for the node's new position.
         */
        listNode = &txLib->head;
        while ((listNode = listNode->next) != &txLib->head
                && listNode != nextUnsortedNode)
        {
            if (Cmp(&node->item, &listNode->item) < 0) {
                break;
            }
        }
        /*
         * Link the node into the list.
         */
        node->prev = listNode->prev;
        node->next = listNode;
        listNode->prev->next = node;
        listNode->prev = node;
    }
}

