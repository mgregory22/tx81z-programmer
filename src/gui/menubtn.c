/*
 * menubtn.c - Menu button control
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
#include "menubtn.h"
#include "prog.h"

/*
 * Unit types
 */
typedef struct {
    HANDLE menu;
    TRACKFUNC TrackFunc;
    UINT trackFuncFlags;
    WNDPROC origWndProc;
    DWORD style;
} MENUBTNINFO;

/*
 * Unit constants
 */
#define GWL_MENUBTNDATA    GWL_USERDATA

/*
 * Macros
 */
#define GET_MENUBTNINFO(menuBtn) ((MENUBTNINFO *) GetWindowLong(menuBtn, GWL_MENUBTNDATA))

/*
 * Global procedures
 */
extern void MenuBtn_Deinit(HWND btnCtrl);
extern void MenuBtn_DrawArrow(HWND parentWnd, const DRAWITEMSTRUCT *drawItem);
extern void MenuBtn_DrawButton(HWND parentWnd, const DRAWITEMSTRUCT *drawItem);
extern BOOL MenuBtn_Init(HWND btnCtrl, HANDLE menu, TRACKFUNC TrackFunc
        , UINT trackFuncFlags);

/*
 * Unit constants
 */
static const WORD mb_arrowBits[3] = {
    0x0000, 0x0088, 0x00D8
};

/*
 * Unit procedures
 */
static void mb_OnDestroy(HWND menuBtn);
static void mb_OnLButtonDown(HWND menuBtn, BOOL dblClick, int x, int y, UINT keyFlags);
static void mb_OnLButtonUp(HWND menuBtn, int x, int y, UINT keyFlags);
static LRESULT CALLBACK mb_WndProc(HWND menuBtn, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HBITMAP mb_arrowBmp;


/*
 * Procedure definitions
 */

/*
 * Deinit() - Deinitializes a menu button.
 */
void MenuBtn_Deinit(HWND btnCtrl)
{
    MENUBTNINFO *menuBtnInfo;

    menuBtnInfo = (MENUBTNINFO *) GetWindowLong(btnCtrl, GWL_MENUBTNDATA);
    if (menuBtnInfo) {
        /*
         * The button was already initialized - deinit the button.
         */
        SetWindowStyle(btnCtrl, menuBtnInfo->style);
        SubclassWindow(btnCtrl, menuBtnInfo->origWndProc);
        free(menuBtnInfo);
        SetWindowLong(btnCtrl, GWL_MENUBTNDATA, (LONG) NULL);
    }
}

/*
 * DrawArrow() - Draws a menu-down-arrow thing on the button.
 */
void MenuBtn_DrawArrow(HWND parentWnd, const DRAWITEMSTRUCT *drawItem)
{
    HDC memDC;
    HDC dC = GetDC(drawItem->hwndItem); // drawItem->hDC;
    int btnArrowX, btnArrowY;
    COLORREF oldTextColor, oldBkColor;

    /*
     * Set up some drawing parameters.
     */
    btnArrowX = drawItem->rcItem.right - 9;
    btnArrowY = (drawItem->rcItem.bottom >> 1) - 1;
    if (drawItem->itemState & ODS_SELECTED) {
        btnArrowX++;
        btnArrowY++;
    }
    /*
     * Draw the button arrow.
     */
    memDC = CreateCompatibleDC(dC);
    SelectBitmap(memDC, mb_arrowBmp);
    oldTextColor = SetTextColor(dC, Prog_wndTextColor);
    oldBkColor = SetBkColor(dC, GetSysColor(COLOR_3DFACE));
    BitBlt(dC, btnArrowX, btnArrowY, 5, 3, memDC, 0, 0, SRCCOPY);
    SetTextColor(dC, oldTextColor);
    SetBkColor(dC, oldBkColor);
    DeleteDC(memDC);
    ReleaseDC(drawItem->hwndItem, dC);
}

/*
 * DrawButton() - Draws the button using the styles that it had before calling
 *                Init().  Should be called in WM_DRAWITEM before any
 *                custom drawing is done.
 */
void MenuBtn_DrawButton(HWND parentWnd, const DRAWITEMSTRUCT *drawItem)
{
#if 0
    /* Doesn't draw anything but the arrow */
    HWND menuBtn = drawItem->hwndItem;
    MENUBTNINFO *menuBtnInfo = GET_MENUBTNINFO(menuBtn);
    DWORD style = SetWindowStyle(menuBtn, menuBtnInfo->style);

    CallWindowProc(menuBtnInfo->origWndProc, menuBtn, WM_DRAWITEM
            , GetDlgCtrlID(menuBtn), (LPARAM) drawItem);
    SetWindowStyle(menuBtn, style);
#endif
#if 0
    /*
     * Swap the current button styles with the saved styles for the
     * CallWindowProc() call.
     */
    /* Crashes??? */
    HWND menuBtn = drawItem->hwndItem;
    MENUBTNINFO *menuBtnInfo = GET_MENUBTNINFO(menuBtn);
    DWORD style = SetWindowStyle(menuBtn, menuBtnInfo->style);
    WNDPROC WndProc = SubclassWindow(menuBtn, menuBtnInfo->origWndProc);

    RedrawWindowNow(menuBtn);
    SetWindowStyle(menuBtn, style);
    SubclassWindow(menuBtn, WndProc);
    SetWindowLong(menuBtn, GWL_MENUBTNDATA, (LONG) menuBtnInfo);
#endif
    HDC dC = drawItem->hDC;
    HWND menuBtn = drawItem->hwndItem;
    RECT itemRect = drawItem->rcItem;
    HBITMAP bmp;

    if (drawItem->itemState & ODS_SELECTED) {
        DrawEdge(dC, &itemRect, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
        itemRect.left++;
        itemRect.top++;
    } else {
        DrawEdge(dC, &itemRect, EDGE_RAISED, BF_RECT | BF_MIDDLE);
    }
    /* Doesn't draw bitmap buttons */
    if (bmp = Button_GetBitmap(menuBtn)) {
        HDC memDC = CreateCompatibleDC(dC);
        BITMAP bmpInfo;

        GetObject(bmp, sizeof bmpInfo, &bmpInfo);
        SelectBitmap(memDC, bmp);
        BitBlt(dC, itemRect.left + 4
                , itemRect.top + ((RECT_H(itemRect) - bmpInfo.bmHeight) >> 1)
                    , bmpInfo.bmWidth, bmpInfo.bmHeight, memDC, 0, 0, SRCCOPY);
        DeleteDC(memDC);
    } else {
        _TUCHAR text[MENUBTN_TEXT_MAX];
        int textLen = Button_GetTextLength(menuBtn);
        int oldBkMode = SetBkMode(dC, TRANSPARENT);

        Button_GetText(menuBtn, text, MENUBTN_TEXT_MAX);
        itemRect.left += 4;
        DrawText(dC, text, textLen, &itemRect
                , DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
        SetBkMode(dC, oldBkMode);
    }
}

/*
 * Init() - Create a menu button from a regular pushbutton.  The button
 *          must NOT have the BS_OWNERDRAW style, unless you're prepared to
 *          deal with the recursion that would ensue.
 */
BOOL MenuBtn_Init(HWND btnCtrl, HANDLE menu, TRACKFUNC TrackFunc
        , UINT trackFuncFlags)
{
    MENUBTNINFO *menuBtnInfo;
    UINT style;

    /*
     * Create the internal menu button info structure.
     */
    menuBtnInfo = malloc(sizeof *menuBtnInfo);

    if (!menuBtnInfo) {
        Error_OnError(E_MALLOC_ERROR);
        return FALSE;
    }
    SetWindowLong(btnCtrl, GWL_MENUBTNDATA, (LONG) menuBtnInfo);
    menuBtnInfo->menu = menu;
    menuBtnInfo->TrackFunc = TrackFunc;
    menuBtnInfo->trackFuncFlags = trackFuncFlags;
    menuBtnInfo->origWndProc = SubclassWindow(btnCtrl, mb_WndProc);
    /*
     * Create a bitmap for the little down arrow.
     */
    if (!mb_arrowBmp) {
        mb_arrowBmp = CreateBitmap(5, 3, 1, 1, mb_arrowBits);
    }
    /*
     * Save the button styles and change the button to owner-drawn.
     */
    style = menuBtnInfo->style = GetWindowStyle(btnCtrl);
    SetWindowStyle(btnCtrl, style | BS_OWNERDRAW);

    return TRUE;
}

/*
 * OnDestroy()
 */
void mb_OnDestroy(HWND menuBtn)
{
    MENUBTNINFO *menuBtnInfo = GET_MENUBTNINFO(menuBtn);
    WNDPROC origWndProc = menuBtnInfo->origWndProc;

    SubclassWindow(menuBtn, origWndProc);
    PostMessage(menuBtn, WM_DESTROY, 0, 0);
    free(menuBtnInfo);
}

/*
 * OnLButtonDown()
 */
void mb_OnLButtonDown(HWND menuBtn, BOOL dblClick, int x, int y, UINT keyFlags)
{
    MENUBTNINFO *menuBtnInfo = GET_MENUBTNINFO(menuBtn);
    TPMPARAMS tpmParams = { sizeof tpmParams };
    HWND parentWnd = GetParent(menuBtn);
    UINT ctrlID = GetDlgCtrlID(menuBtn);
    UINT menuSelection;
    DWORD msgPos;

    if (!Button_IsPushed(menuBtn)) {
        Button_Push(menuBtn);
        SendNotification(parentWnd, ctrlID, menuBtn, MBN_PUSHED);

        GetWindowRect(menuBtn, &tpmParams.rcExclude);
        if (menuBtnInfo->TrackFunc) {
#ifdef _DEBUG
            assert(!IsBadCodePtr((FARPROC) menuBtnInfo->TrackFunc));
#endif
            menuSelection = menuBtnInfo->TrackFunc(menuBtnInfo->menu
                    , &tpmParams.rcExclude);
        } else {
            menuSelection = TrackPopupMenuEx(menuBtnInfo->menu
                    , menuBtnInfo->trackFuncFlags
                    , tpmParams.rcExclude.left, tpmParams.rcExclude.bottom
                    , menuBtn, &tpmParams);
        }
        if (menuSelection) {
            FORWARD_WM_COMMAND(parentWnd, menuSelection, menuBtn, 0
                    , PostMessage);
            goto ReleaseButton;
        } else {
            /*
             * If the pointer is over the button when TrackPopupMenuEx()
             * returns, and there's a WM_LBUTTONDOWN message in the queue,
             * remove it before releasing the button, otherwise the button
             * will get pressed again when it should be toggled off.
             */
            msgPos = GetMessagePos();
            if (Rect_XYIn(&tpmParams.rcExclude, GET_X_LPARAM(msgPos)
                        , GET_Y_LPARAM(msgPos)))
            {
                MSG msg;

                PeekMessage(&msg, NULL, WM_LBUTTONDOWN, WM_LBUTTONDOWN
                            , PM_REMOVE); 
            }
        }
    }
ReleaseButton:
    Button_Release(menuBtn);
    SendNotification(parentWnd, ctrlID, menuBtn, MBN_RELEASED);
}

/*
 * OnLButtonUp()
 */
void mb_OnLButtonUp(HWND menuBtn, int x, int y, UINT keyFlags)
{
    /* disable button up processing */
}

/*
 * WndProc()
 */
LRESULT CALLBACK mb_WndProc(HWND menuBtn, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    MENUBTNINFO *menuBtnInfo;

    switch (message) {
        HANDLE_MSG(menuBtn, WM_DESTROY, mb_OnDestroy);
        HANDLE_MSG(menuBtn, WM_LBUTTONDOWN, mb_OnLButtonDown);
        HANDLE_MSG(menuBtn, WM_LBUTTONDBLCLK, mb_OnLButtonDown);
        HANDLE_MSG(menuBtn, WM_LBUTTONUP, mb_OnLButtonUp);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(menuBtn);

            MapWindowPoints(menuBtn, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
    }
    menuBtnInfo = GET_MENUBTNINFO(menuBtn);

    return CallWindowProc(menuBtnInfo->origWndProc, menuBtn, message, wParam
            , lParam);
}

