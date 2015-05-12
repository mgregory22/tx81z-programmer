/*
 * rpanel.c - Radio panel control - a rectangular bank of icons that are selected like radio buttons.
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
#include "rpanel.h"
#include "prog.h"
#include "resource.h"

/*
 * Unit types
 */
typedef struct {
    int value;
    int rows;
    int columns;
    BOOL highlight;
} RPANELDATA;

/*
 * Macros
 */
#define GET_RPANELDATA(rPanel)  (RPANELDATA *) GetWindowLong((rPanel), GWL_RPANELDATA)

/*
 * Unit constants
 */
#define RPANEL_EXTRA      4
#define GWL_RPANELDATA    0

/*
 * Global constants
 */
const _TUCHAR *RPanel_className = _T("RPanel");

/*
 * Global procedures
 */
extern BOOL RPanel_Register(void);

/*
 * Unit procedures
 */
static void rp_OnChar(HWND rPanel, _TUCHAR ch, int repeat);
static BOOL rp_OnCreate(HWND rPanel, LPCREATESTRUCT createStruct);
static void rp_OnDestroy(HWND rPanel);
static int rp_OnGetMax(HWND rPanel);
static int rp_OnGetValue(HWND rPanel);
static void rp_OnHighlight(HWND rPanel, BOOL highlight);
static void rp_OnKey(HWND rPanel, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void rp_OnKillFocus(HWND rPanel, HWND otherWnd);
static void rp_OnLButtonDown(HWND rPanel, BOOL dblClick, int x, int y, UINT keyFlags);
static void rp_OnPaint(HWND rPanel);
static void rp_OnSetFocus(HWND rPanel, HWND otherWnd);
static void rp_OnSetGeometry(HWND rPanel, int rows, int columns);
static void rp_OnSetValue(HWND rPanel, int value);
static LRESULT CALLBACK rp_WndProc(HWND rPanel, UINT message, WPARAM wParam, LPARAM lParam);


/*
 * Procedure definitions
 */

/*
 * Register() - Registers the window class for the control.  Returns true on
 *              success, false on failure.
 */
BOOL RPanel_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) rp_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = RPANEL_EXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = RPanel_className;
    classInfo.hIconSm = NULL;
    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering RPanel control"));
        return FALSE;
    }

    return TRUE;
}

/*
 * OnChar()
 */
void rp_OnChar(HWND rPanel, _TUCHAR ch, int repeat)
{
    if (isdigit(ch)) {
        RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
        int rows = rPanelData->rows;
        int columns = rPanelData->columns;
        int value = ch - '1';

        if (value >= 0 && value < rows * columns) {
            int r = value / columns;
            int c = value - r * columns;
            int columnSize, rowSize;
            int x, y;
            RECT clientRect;

            GetClientRect(rPanel, &clientRect);
            rowSize = RECT_H(clientRect) / rows;
            columnSize = RECT_W(clientRect) / columns;
            x = c * columnSize + 1;
            y = r * rowSize + 1;
            rp_OnLButtonDown(rPanel, FALSE, x, y, 0);
        }
    }
}

/*
 * OnCreate()
 */
BOOL rp_OnCreate(HWND rPanel, LPCREATESTRUCT createStruct)
{
    RPANELDATA *rPanelData = calloc(1, sizeof *rPanelData);

    if (!rPanelData) {
        Error_OnError(E_MALLOC_ERROR);
        return FALSE;
    }
    SetWindowLong(rPanel, GWL_RPANELDATA, (LONG) rPanelData);

    return TRUE;
}

/*
 * OnDestroy()
 */
void rp_OnDestroy(HWND rPanel)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);

    free(rPanelData);
}

/*
 * OnGetMax()
 */
int rp_OnGetMax(HWND rPanel)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);

    return rPanelData->rows * rPanelData->columns - 1;
}

/*
 * OnGetValue()
 */
int rp_OnGetValue(HWND rPanel)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);

    return rPanelData->value;
}

/*
 * OnHighlight()
 */
void rp_OnHighlight(HWND rPanel, BOOL highlight)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);

    rPanelData->highlight = highlight;
    RedrawWindowNow(rPanel);
}

/*
 * OnKey()
 */
void rp_OnKey(HWND rPanel, UINT vk, BOOL down, int repeat, UINT keyFlags)
{
    if (down) {
        RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
        int value = rPanelData->value;
        int rows = rPanelData->rows;
        int columns = rPanelData->columns;
        int maxValue = rows * columns - 1;
        int r, c, x, y, columnSize, rowSize;
        RECT clientRect;

        switch (vk) {
            case VK_UP:
                value--;
                goto CheckMin;
            case VK_DOWN:
                value++;
                goto CheckMax;
            case VK_PRIOR:
                value -= columns;
CheckMin:
                if (value < 0) {
                    value = 0;
                }
                break;
            case VK_NEXT:
                value += columns;
CheckMax:
                if (value > maxValue) {
                    value = maxValue;
                }
                break;
            case VK_HOME:
                value = 0;
                break;
            case VK_END:
                value = maxValue;
                break;
            default:
                return;
        }
        r = value / columns;
        c = value - r * columns;
        GetClientRect(rPanel, &clientRect);
        rowSize = RECT_H(clientRect) / rows;
        columnSize = RECT_W(clientRect) / columns;
        x = c * columnSize + 1;
        y = r * rowSize + 1;
        rp_OnLButtonDown(rPanel, FALSE, x, y, 0);
    }
}

/*
 * OnKillFocus()
 */
void rp_OnKillFocus(HWND rPanel, HWND otherWnd)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
    int value = rPanelData->value;
    int rows = rPanelData->rows;
    int columns = rPanelData->columns;
    int rowSize, columnSize;
    int r, c;
    RECT clientRect;

    GetClientRect(rPanel, &clientRect);
    rowSize = RECT_H(clientRect) / rows;
    columnSize = RECT_W(clientRect) / columns;
    r = value / columns;
    c = value % columns;
    HideCaret(rPanel);
    InvalidateRect(rPanel, NULL, FALSE);
    SendNotification(GetParent(rPanel), GetDlgCtrlID(rPanel), rPanel
            , RPN_KILLFOCUS);
}

/*
 * OnLButtonDown()
 */
void rp_OnLButtonDown(HWND rPanel, BOOL dblClick, int x, int y, UINT keyFlags)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
    HWND parentWnd = (HWND) GetWindowLong(rPanel, GWL_HWNDPARENT);
    UINT ctrlID = (UINT) GetWindowLong(rPanel, GWL_ID);
    int value = rPanelData->value;
    int rows = rPanelData->rows;
    int columns = rPanelData->columns;
    int rowSize, columnSize;
    int r, c;
    int newR, newC;
    RECT clientRect;
    HDC dC = GetDC(rPanel);
    DRAWITEMSTRUCT drawItem = {
        ODT_RPANEL,           /* CtlType */
        ctrlID,               /* CtlID */
        value,                /* itemID */
        ODA_SELECT,           /* itemAction */
        ODS_FOCUS,            /* itemState */
        rPanel,               /* hwndItem */
        dC                    /* hDC */
                              /* rcItem */
                              /* itemData */
    };

    GetClientRect(rPanel, &clientRect);
    rowSize = RECT_H(clientRect) / rows;
    columnSize = RECT_W(clientRect) / columns;
    r = value / columns;
    c = value % columns;
    drawItem.rcItem.left = c * columnSize + 1;
    drawItem.rcItem.top = r * rowSize + 1;
    drawItem.rcItem.right = drawItem.rcItem.left + columnSize - 1;
    drawItem.rcItem.bottom = drawItem.rcItem.top + rowSize - 1;
    newR = y / rowSize;
    newC = x / columnSize;
    /*
     * Filter out clicks on the very edge, which make the cursor disappear.
     */
    if (newR >= rows || newC >= columns) {
        return;
    }
    DestroyCaret();
    SendMessage(parentWnd, WM_DRAWITEM, (WPARAM) ctrlID, (LPARAM) &drawItem);
    value = newR * columns + newC;
    rPanelData->value = value;
    drawItem.rcItem.left = newC * columnSize + 1;
    drawItem.rcItem.top = newR * rowSize + 1;
    drawItem.rcItem.right = drawItem.rcItem.left + columnSize - 1;
    drawItem.rcItem.bottom = drawItem.rcItem.top + rowSize - 1;
    drawItem.itemState |= ODS_SELECTED;
    drawItem.itemID = value;
    SendMessage(parentWnd, WM_DRAWITEM, (WPARAM) ctrlID, (LPARAM) &drawItem);
    ReleaseDC(rPanel, dC);
    PostNotification(parentWnd, ctrlID, rPanel, RPN_CHANGE);
    if (GetFocus() == rPanel) {
        rp_OnSetFocus(rPanel, NULL);
    } else {
        SetFocus(rPanel);
    }
}

/*
 * OnPaint()
 */
void rp_OnPaint(HWND rPanel)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
    HWND parentWnd = (HWND) GetWindowLong(rPanel, GWL_HWNDPARENT);
    UINT ctrlID = (UINT) GetWindowLong(rPanel, GWL_ID);
    int value = rPanelData->value;
    int rows = rPanelData->rows;
    int columns = rPanelData->columns;
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(rPanel, &paint);
    RECT clientRect;
    DRAWITEMSTRUCT drawItem = {
        ODT_RPANEL,           /* CtlType */
        ctrlID,               /* CtlID */
        0,                    /* itemID */
        ODA_DRAWENTIRE,       /* itemAction */
        0,                    /* itemState */
        rPanel,               /* hwndItem */
        paintDC               /* hDC */
                              /* rcItem */
                              /* itemData */
    };
    int rowSize, columnSize;
    int r, c;
    BOOL gotFocus = (GetFocus() == rPanel);

    GetClientRect(rPanel, &clientRect);
    rowSize = RECT_H(clientRect) / rows;
    columnSize = RECT_W(clientRect) / columns;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < columns; c++) {
            drawItem.rcItem.left = columnSize * c;
            drawItem.rcItem.top = rowSize * r;
            drawItem.rcItem.right = columnSize * (c + 1) + 1;
            if (drawItem.rcItem.right > clientRect.right) {
                drawItem.rcItem.right = clientRect.right;
            }
            drawItem.rcItem.bottom = rowSize * (r + 1) + 1;
            if (drawItem.rcItem.bottom > clientRect.bottom) {
                drawItem.rcItem.bottom = clientRect.bottom;
            }
            drawItem.itemState = 0;
            drawItem.itemID = r * columns + c;
            if ((UINT) value == drawItem.itemID) {
                drawItem.itemState = ODS_SELECTED;
                if (rPanelData->highlight) {
                    drawItem.itemState |= ODS_HIGHLIGHT;
                }
            }
            if (gotFocus) {
                drawItem.itemState |= ODS_FOCUS;
            }
            FrameRect(paintDC, &drawItem.rcItem, Prog_wndTextBrush);
            InflateRect(&drawItem.rcItem, -1, -1);
            SendMessage(parentWnd, WM_DRAWITEM, (WPARAM) ctrlID
                    , (LPARAM) &drawItem);
        }
    }

    EndPaint(rPanel, &paint);
}

/*
 * OnSetFocus()
 */
void rp_OnSetFocus(HWND rPanel, HWND otherWnd)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
    int value = rPanelData->value;
    int rows = rPanelData->rows;
    int columns = rPanelData->columns;
    int rowSize, columnSize;
    int r, c;
    RECT clientRect;

    GetClientRect(rPanel, &clientRect);
    rowSize = RECT_H(clientRect) / rows;
    columnSize = RECT_W(clientRect) / columns;
    r = value / columns;
    c = value % columns;
    CreateCaret(rPanel, NULL, columnSize - 1, rowSize - 1);
    SetCaretPos(c * columnSize + 1, r * rowSize + 1);
    ShowCaret(rPanel);
    InvalidateRect(rPanel, NULL, FALSE);
    SendNotification(GetParent(rPanel), GetDlgCtrlID(rPanel), rPanel
            , RPN_SETFOCUS);
}

/*
 * OnSetGeometry()
 */
void rp_OnSetGeometry(HWND rPanel, int rows, int columns)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);

    rPanelData->rows = rows;
    rPanelData->columns = columns;
}

/*
 * OnSetValue()
 */
void rp_OnSetValue(HWND rPanel, int value)
{
    RPANELDATA *rPanelData = GET_RPANELDATA(rPanel);
    int maxValue = rPanelData->rows * rPanelData->columns;

    if (value < 0) {
        value = 0;
    } else if (value >= maxValue) {
        value = maxValue - 1;
    }
    rPanelData->value = value;
    /*
     * If the radio panel has focus when the value is set, the control needs
     * to be refocused so the cursor will move to the correct location.
     */
    if (GetFocus() == rPanel) {
        rp_OnSetFocus(rPanel, NULL);
    }
    InvalidateRect(rPanel, NULL, FALSE);
}

/*
 * WndProc()
 */
LRESULT CALLBACK rp_WndProc(HWND rPanel, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(rPanel, RPM_GETMAX, rp_OnGetMax);
        HANDLE_MSG(rPanel, RPM_GETVALUE, rp_OnGetValue);
        HANDLE_MSG(rPanel, RPM_HIGHLIGHT, rp_OnHighlight);
        HANDLE_MSG(rPanel, RPM_SETGEOMETRY, rp_OnSetGeometry);
        HANDLE_MSG(rPanel, RPM_SETVALUE, rp_OnSetValue);
        HANDLE_MSG(rPanel, WM_CHAR, rp_OnChar);
        HANDLE_MSG(rPanel, WM_CREATE, rp_OnCreate);
        HANDLE_MSG(rPanel, WM_DESTROY, rp_OnDestroy);
        HANDLE_MSG(rPanel, WM_KEYDOWN, rp_OnKey);
        HANDLE_MSG(rPanel, WM_KILLFOCUS, rp_OnKillFocus);
        HANDLE_MSG(rPanel, WM_LBUTTONDOWN, rp_OnLButtonDown);
        HANDLE_MSG(rPanel, WM_PAINT, rp_OnPaint);
        HANDLE_MSG(rPanel, WM_SETFOCUS, rp_OnSetFocus);
    }
    return DefWindowProc(rPanel, message, wParam, lParam);
}

