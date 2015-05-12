/*
 * txlbx.c - TX81Z list box
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
#include "minifont.h"
#include "prog.h"
#include "resource.h"
#include "snapshot.h"
#include "tx81z.h"
#include "txlbx.h"

/*
 * Global procedures
 */
extern int TxLbx_AddItem(HWND listBox, const _TUCHAR *text, DWORD itemData, TXLBX_ATTR attr);
extern int TxLbx_AddLibraryItem(HWND listBox, const _TUCHAR *type, const char *name, const char *comment, DWORD itemData);
extern int TxLbx_AddNonbankItem(HWND listBox, const _TUCHAR *type, const _TUCHAR *name, DWORD itemData);
extern int TxLbx_AddSnapshotItem(HWND listBox, const _TUCHAR *type, const char *name, DWORD itemData);
extern void TxLbx_Init(HWND listBox, TXLBX_PROPERTIES *txProps);
extern void TxLbx_InitColors(void);
extern int TxLbx_InsertItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData, TXLBX_ATTR attr);
extern void TxLbx_OnDrawItem(HWND wnd, const DRAWITEMSTRUCT *drawItem);
extern void TxLbx_OnMeasureItem(HWND wnd, MEASUREITEMSTRUCT *measureItem);
extern void TxLbx_ReplaceItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData, TXLBX_ATTR attr);
extern void TxLbx_SetItemAttr(HWND listBox, int lbxItem, TXLBX_ATTR attr);

/*
 * Global variables
 */
COLORREF *TxLbx_colors[4]; /* indexed by second highest 4 bits of TXLBX_ATTR */
HBRUSH *TxLbx_brushes[2];  /* indexed by upper 4 bits of TXLBX_ATTR */

/*
 * Unit constants
 */
#define ITEMNAME_LEN  80

/*
 * Unit procedures
 */
static LRESULT CALLBACK tl_LbxProc(HWND listBox, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static WNDPROC tl_origLbxProc; /* using a global for this assumes that all
                                  list boxes have the same original window
                                  procedure */

/*
 * Procedure definitions
 */

/*
 * AddItem()
 */
int TxLbx_AddItem(HWND listBox, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr)
{
    int lbxItem;

    lbxItem = ListBox_AddString(listBox, text);
    ListBox_SetItemData(listBox, lbxItem, attr | (itemData & 0x00FFFFFF));

    return lbxItem;
}

/*
 * AddLibraryItem()
 */
int TxLbx_AddLibraryItem(HWND listBox, const _TUCHAR *type, const char *name
        , const char *comment, DWORD itemData)
{
    _TUCHAR itemName[ITEMNAME_LEN];
    int lbxItem;

#ifdef _UNICODE
    _sntprintf(itemName, ITEMNAME_LEN, _T("%-4.4s %-10.10S %-40.40S"), type
            , name, comment);
#else
    _sntprintf(itemName, ITEMNAME_LEN, _T("%-4.4s %-10.10s %-40.40s"), type
            , name, comment);
#endif
    itemName[ITEMNAME_LEN - 1] = '\0';
    lbxItem = ListBox_AddString(listBox, itemName);
    ListBox_SetItemData(listBox, lbxItem, itemData & 0x00FFFFFF);

    return lbxItem;
}

/*
 * AddNonbankItem()
 */
int TxLbx_AddNonbankItem(HWND listBox, const _TUCHAR *type, const _TUCHAR *name
        , DWORD itemData)
{
    _TUCHAR itemName[ITEMNAME_LEN];
    int lbxItem;

    _sntprintf(itemName, ITEMNAME_LEN, _T("%-4.4s %s"), type, name);
    itemName[ITEMNAME_LEN - 1] = '\0';
    lbxItem = ListBox_AddString(listBox, itemName);
    ListBox_SetItemData(listBox, lbxItem, itemData & 0x00FFFFFF);

    return lbxItem;
}

/*
 * AddSnapshotItem()
 */
int TxLbx_AddSnapshotItem(HWND listBox, const _TUCHAR *type, const char *name
        , DWORD itemData)
{
    _TUCHAR itemName[ITEMNAME_LEN];
    int lbxItem;

#ifdef _UNICODE
    _sntprintf(itemName, ITEMNAME_LEN, _T("%-4.4s %-10.10S"), type, name);
#else
    _sntprintf(itemName, ITEMNAME_LEN, _T("%-4.4s %-10.10s"), type, name);
#endif
    itemName[ITEMNAME_LEN - 1] = '\0';
    lbxItem = ListBox_AddString(listBox, itemName);
    ListBox_SetItemData(listBox, lbxItem, itemData & 0x00FFFFFF);

    return lbxItem;
}

/*
 * Init() - Sets properties and window procedure for the TX list box.
 */
void TxLbx_Init(HWND listBox, TXLBX_PROPERTIES *txProps)
{
    SetWindowLong(listBox, GWL_USERDATA, (LONG) txProps);
    tl_origLbxProc = (WNDPROC) SetWindowLong(listBox, GWL_WNDPROC
            , (LONG) tl_LbxProc);
}

/*
 * InitColors() - Sets up color arrays.
 */
void TxLbx_InitColors(void)
{
    TxLbx_colors[0] = &Prog_wndTextColor;
    TxLbx_colors[1] = &Prog_unloadedColor;
    TxLbx_colors[2] = &Prog_dirtyColor;
    TxLbx_colors[3] = &Prog_selTextColor;
    TxLbx_brushes[0] = &Prog_wndBrush;
    TxLbx_brushes[1] = &Prog_hiBrush;
}

/*
 * InsertItem() - Inserts an item into the list box at a specific index.
 */
int TxLbx_InsertItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr)
{
    idx = ListBox_InsertString(listBox, idx, text);
    ListBox_SetItemData(listBox, idx, attr | (itemData & 0x00FFFFFF));

    return idx;
}

/*
 * OnDrawItem() - WM_DRAWITEM handler to draw list box items.
 */
void TxLbx_OnDrawItem(HWND wnd, const DRAWITEMSTRUCT *drawItem)
{
    HDC dC = drawItem->hDC;
    RECT itemRect = drawItem->rcItem;
    HWND listBox = drawItem->hwndItem;
    TXLBX_PROPERTIES *txProps = (TXLBX_PROPERTIES *) GetWindowLong(listBox
            , GWL_USERDATA);
    int itemIndex = drawItem->itemID;
    COLORREF textColor;
    HBRUSH bgBrush;
    TXLBX_ATTR itemAttr = drawItem->itemData >> 24;
    int i;

    if (drawItem->itemAction == ODA_FOCUS) {
        /*
         * Draw focus rectangle.
         */
        DrawFocusRect(dC, &itemRect);
    } else {
        /*
         * Set up colors based on selection and copy state.
         */
        if (drawItem->itemState & ODS_SELECTED) {
            textColor = *TxLbx_colors[itemAttr & 0x0F];
            if (textColor == Prog_wndTextColor) {
                textColor = Prog_selTextColor;
            }
            bgBrush = Prog_selBrush;
        } else {
            textColor = *TxLbx_colors[itemAttr & 0x0F];
            bgBrush = *TxLbx_brushes[itemAttr >> 4];
        }
        /*
         * Draw the item's background.
         */
        FillRect(dC, &itemRect, bgBrush);
        if (itemIndex > -1) {
            _TUCHAR text[80];
            int textLen = ListBox_GetText(listBox, itemIndex, text);

            /*
             * Draw the string.
             */
            MiniFont_DrawString(dC, itemRect.left + 2, itemRect.top + 3, text
                    , textLen, textColor);
            /*
             * Draw column separators.
             */
            for (i = 0; i < txProps->divCnt; i++) {
                int x = txProps->divPos[i];
                HPEN oldPen = SelectPen(dC, Prog_wndTextPen);

                MoveToEx(dC, x, itemRect.top, NULL);
                LineTo(dC, x, itemRect.bottom);
                SelectPen(dC, oldPen);
            }
        }
    }
}

/*
 * OnMeasureItem()
 */
void TxLbx_OnMeasureItem(HWND wnd, MEASUREITEMSTRUCT *measureItem)
{
    if (measureItem->CtlType == ODT_LISTBOX) {
        /* assumes the minifont is being used for the item font */
        measureItem->itemHeight = 13;
    }
}

/*
 * ReplaceItem() - 
 */
void TxLbx_ReplaceItem(HWND listBox, int idx, const _TUCHAR *text, DWORD itemData
        , TXLBX_ATTR attr)
{
    ListBox_DeleteString(listBox, idx);
    ListBox_InsertString(listBox, idx, text);
    ListBox_SetItemData(listBox, idx, attr | (itemData & 0x00FFFFFF));
}

/*
 * SetItemAttr()
 */
void TxLbx_SetItemAttr(HWND listBox, int lbxItem, TXLBX_ATTR attr)
{
    DWORD data = ListBox_GetItemData(listBox, lbxItem) & 0x00FFFFFF;

    data |= attr;
    ListBox_SetItemData(listBox, lbxItem, data);
}

/*
 * LbxProc() - List box window procedure.  Draws the dividers on empty slots,
 *             makes sure the mouse cursor is an arrow, and sets focus to the
 *             list box when the user clicks on the scrollbar.
 */
LRESULT CALLBACK tl_LbxProc(HWND listBox, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_PAINT:
        {
            TXLBX_PROPERTIES *txProps
                = (TXLBX_PROPERTIES *) GetWindowLong(listBox, GWL_USERDATA);
            HDC dC;
            RECT clientRect;
            int i;

            /*
             * Draw items.
             */
            CallWindowProc(tl_origLbxProc, listBox, message, wParam
                    , lParam);

            /*
             * Draw column separators.
             */
            dC = GetDC(listBox);
            GetClientRect(listBox, &clientRect);
            for (i = 0; i < txProps->divCnt; i++) {
                int x = txProps->divPos[i];
                HPEN oldPen = SelectPen(dC, Prog_wndTextPen);

                MoveToEx(dC, x, 0, NULL);
                LineTo(dC, x, clientRect.bottom);
                SelectPen(dC, oldPen);
            }
            ReleaseDC(listBox, dC);
            return 0;
        }
        case WM_MOUSEMOVE:
            SetCursor(Prog_arrowCursor);
            break;
        case WM_NCLBUTTONDOWN:
            if (GetFocus() != listBox) {
                SetFocus(listBox);
            }
            break;
    }
    return CallWindowProc(tl_origLbxProc, listBox, message, wParam
            , lParam);
}

