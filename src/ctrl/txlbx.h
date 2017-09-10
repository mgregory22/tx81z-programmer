/*
 * txlbx.h - TX81Z list box
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
 *
 * Usage:
 *   1. Call TxLbx_InitColors() at the beginning of the program to initialize
 *      the color arrays.  The array consists of pointers to Prog_*Color and
 *      Prog_*Brush variables.
 *   2. For each list box, create a non-temporary TXLBX_PROPERTIES structure
 *      to specify the number of column dividers and their horizontal positions
 *      in the list box.
 *   3. Call TxLbx_Init() after the list box is created to set up the window
 *      procedure and set the properties.
 *   4. Populate the list box with the TxLbx_Add*Item() functions.
 *   5. Use TxLbx_OnDrawItem() and TxLbx_OnMeasureItem() in the window
 *      procedure for the parent window.  If WM_DRAWITEM is needed for other
 *      controls, TxLbx_OnDrawItem() can be called inside the regular
 *      WM_DRAWITEM handler.
 *   6. If you want to use user item data for the list box items, mask off the
 *      upper 8 bits, which contain the color attributes for the item.
 */
#ifndef TXLBX_H
#define TXLBX_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif


/*
 * Global types
 */
typedef struct {
    int divCnt;           /* number of dividers */
    int *divPos;          /* array of the X positions of each column divider */
} TXLBX_PROPERTIES;

typedef enum {
    TA_FG_NORMAL      = 0x00000000,
    TA_FG_UNLOADED    = 0x01000000,
    TA_FG_DIRTY       = 0x02000000,
    TA_FG_HIGHLIGHT   = 0x03000000,
    TA_BG_WND         = 0x00000000,
    TA_BG_HIGHLIGHT   = 0x10000000,
    TA_NORMAL         = TA_FG_NORMAL | TA_BG_WND,
    TA_UNLOADED       = TA_FG_UNLOADED | TA_BG_WND,
    TA_DIRTY          = TA_FG_DIRTY | TA_BG_WND,
    TA_HIGHLIGHT      = TA_FG_HIGHLIGHT | TA_BG_HIGHLIGHT,
} TXLBX_ATTR;

/*
 * Global procedures
 */
int TxLbx_AddItem(HWND listBox, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr);
int TxLbx_AddLibraryItem(HWND listBox, const _TUCHAR *type, const char *name
        , const char *comment, DWORD itemData);
int TxLbx_AddNonbankItem(HWND listBox, const _TUCHAR *type, const _TUCHAR *name
        , DWORD itemData);
int TxLbx_AddSnapshotItem(HWND listBox, const _TUCHAR *type, const char *name
        , DWORD itemData);
__inline DWORD TxLbx_GetItemData(HWND listBox, int index);
void TxLbx_Init(HWND listBox, TXLBX_PROPERTIES *txProps);
void TxLbx_InitColors(void);
int TxLbx_InsertItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr);
void TxLbx_OnDrawItem(HWND wnd, const DRAWITEMSTRUCT *drawItem);
void TxLbx_OnMeasureItem(HWND wnd, MEASUREITEMSTRUCT *measureItem);
void TxLbx_ReplaceItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr);
void TxLbx_SetItemAttr(HWND listBox, int lbxItem, TXLBX_ATTR attr);

/*
 * Global variables
 */
extern COLORREF *TxLbx_colors[4]; /* indexed by second highest 4 bits of TXLBX_ATTR */
extern HBRUSH *TxLbx_brushes[2];  /* indexed by upper 4 bits of TXLBX_ATTR */


/*
 * Inline procedure definitions
 */
DWORD TxLbx_GetItemData(HWND listBox, int index)
{
    return (ListBox_GetItemData(listBox, index) & 0x00FFFFFF);
}

#endif  /* #ifndef TXLBX_H */
