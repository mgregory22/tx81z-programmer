/*
 * window.c - convenience functions for windows
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
extern void Window_AddSysMenuItem(HWND wnd, const _TUCHAR *itemText, unsigned cmdID, BOOL separator);
extern SIZE Window_CalcControlAreaSize(HWND wnd);
extern BOOL Window_CalcPositionAroundRect(RECT *wndRect, const RECT *btnRect);
extern void Window_Center(HWND topWnd, HWND bottomWnd);
extern void Window_CenterInParent(HWND wnd);
extern void Window_ClearQueue(HWND wnd);
extern BOOL Window_DoEvents(HWND wnd, HACCEL accels);
extern void Window_GetParentRelativeArea(HWND child, HWND parent, AREA *area);
extern void Window_GetParentRelativeRect(HWND child, HWND parent, RECT *rect);

/*
 * Unit procedures
 */
static BOOL CALLBACK w_AddChildSize(HWND childWnd, LPARAM lParam);


/*
 * Procedure definitions
 */

/*
 * AddSysMenuItem() - Adds an item to the window's system menu.  If separator
 *                    is TRUE, the procedure will insert a separator before
 *                    the item.
 */
void Window_AddSysMenuItem(HWND wnd, const _TUCHAR *itemText, unsigned cmdID
        , BOOL separator)
{
    HMENU sysMenu = GetSystemMenu(wnd, FALSE);
    if (separator) {
        AppendMenu(sysMenu, MF_SEPARATOR, 0, NULL);
    }
    AppendMenu(sysMenu, MF_STRING, cmdID, itemText);
}

/*
 * CalcControlAreaSize() - Calculates the size the client area of the window
 *                         needed to fully display all the child controls in
 *                         the window.
 */
SIZE Window_CalcControlAreaSize(HWND wnd)
{
    SIZE totalSize = { 0 };

    EnumChildWindows(wnd, w_AddChildSize, (LPARAM) &totalSize);

    return totalSize;
}

/*
 * CalcPositionAroundRect() - Calculates the rectangle for a window that
 *                            touches btnRect, but doesn't overlap it nor falls
 *                            off the screen.  The size of wndRect doesn't
 *                            change.  The function returns true if successful
 *                            or false if the criteria can't be met.
 */
BOOL Window_CalcPositionAroundRect(RECT *wndRect, const RECT *btnRect)
{
    int wndW = RECT_W(*wndRect);
    int wndH = RECT_H(*wndRect);
    HDC dC = GetDC(HWND_DESKTOP);
    int screenW = GetDeviceCaps(dC, HORZRES);
    int screenH = GetDeviceCaps(dC, VERTRES);
    //RECT screenRect = { 0, 0, screenW, screenH };

    ReleaseDC(HWND_DESKTOP, dC);
    /*
     * Try positioning the window against the bottom edge of the button.
     */
    if (btnRect->bottom + wndH < screenH) {
        int left;
        if (btnRect->left + wndW < screenW) {
            left = btnRect->left - wndRect->left;
        } else if (btnRect->right - wndW >= 0) {
            left = btnRect->right - wndW - wndRect->left;
        } else {
            left = -wndRect->left;
        }
        OffsetRect(wndRect, left, btnRect->bottom - wndRect->top);
        return TRUE;
    }
    /*
     * Try the right edge of the button.
     */
    if (btnRect->right + wndW < screenW) {
        int top;
        if (btnRect->top + wndH < screenH) {
            top = btnRect->top - wndRect->top;
        } else if (btnRect->bottom - wndH >= 0) {
            top = btnRect->bottom - wndH - wndRect->top;
        } else {
            top = -wndRect->top;
        }
        OffsetRect(wndRect, btnRect->right - wndRect->left, top);
        return TRUE;
    }
    /*
     * Try the top edge of the button
     */
    if (btnRect->top - wndH >= 0) {
        int left;
        if (btnRect->left + wndW < screenW) {
            left = btnRect->left - wndRect->left;
        } else if (btnRect->right - wndW >= 0) {
            left = btnRect->right - wndW - wndRect->left;
        } else {
            left = -wndRect->left;
        }
        OffsetRect(wndRect, left, btnRect->top - wndH - wndRect->top);
        return TRUE;
    }
    /*
     * Try the left edge of the button.
     */
    if (btnRect->left - wndW >= 0) {
        int top;
        if (btnRect->top + wndH < screenH) {
            top = btnRect->top - wndRect->top;
        } else if (btnRect->bottom - wndH >= 0) {
            top = btnRect->bottom - wndH - wndRect->top;
        } else {
            top = -wndRect->top;
        }
        OffsetRect(wndRect, btnRect->left - wndW - wndRect->left, top);
        return TRUE;
    }

    return FALSE;
}

/*
 * Center() - Centers topWnd in front of bottomWnd.
 */
void Window_Center(HWND topWnd, HWND bottomWnd)
{
    RECT topWndRect, bottomWndRect;

    GetWindowRect(topWnd, &topWndRect);
    GetWindowRect(bottomWnd, &bottomWndRect);

    Rect_CenterRect(&topWndRect, &bottomWndRect, CR_HORIZONTAL | CR_VERTICAL);
    Rect_FitToDesktop(&topWndRect);
    MoveWindow(topWnd, topWndRect.left, topWndRect.top
            , Rect_Width(&topWndRect), Rect_Height(&topWndRect), TRUE);
}

/*
 * CenterInParent() - Centers topWnd in front of its parent.
 */
void Window_CenterInParent(HWND wnd)
{
    HWND parentWnd = GetParent(wnd);
    RECT wndRect, parentRect;

    if (!parentWnd)
        return;
    GetWindowRect(wnd, &wndRect);
    GetWindowRect(parentWnd, &parentRect);

    Rect_CenterRect(&wndRect, &parentRect, CR_HORIZONTAL | CR_VERTICAL);
    Rect_FitToDesktop(&wndRect);
    MoveWindow(wnd, wndRect.left, wndRect.top
            , Rect_Width(&wndRect), Rect_Height(&wndRect), TRUE);
}

/*
 * ClearQueue() - Clears a window's message queue.
 */
void Window_ClearQueue(HWND wnd)
{
    MSG msg;

    while (PeekMessage(&msg, wnd, 0, 0, PM_REMOVE)) {
        ;
    }
}

/*
 * DoEvents() - Dispatches current events for a window.  Returns TRUE if the
 *              WM_QUIT message was received.
 */
BOOL Window_DoEvents(HWND wnd, HACCEL accels)
{
    MSG msg;

    while (PeekMessage(&msg, wnd, 0U, 0U, PM_REMOVE)) {
        if (!TranslateAccelerator(msg.hwnd, accels, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    if (msg.message == WM_QUIT) {
        PostQuitMessage(msg.wParam);
        return TRUE;
    }
    return FALSE;
}

/*
 * GetParentRelativeArea()
 */
void Window_GetParentRelativeArea(HWND child, HWND parent, AREA *area)
{
    GetWindowRect(child, (RECT *) area);
    area->w -= area->x;
    area->h -= area->y;
    MapWindowPoints(HWND_DESKTOP, parent, (POINT *) area, 1);
}

/*
 * GetParentRelativeRect()
 */
void Window_GetParentRelativeRect(HWND child, HWND parent, RECT *rect)
{
    GetWindowRect(child, rect);
    MapWindowPoints(HWND_DESKTOP, parent, (POINT *) rect, 2);
}

BOOL CALLBACK w_AddChildSize(HWND childWnd, LPARAM lParam)
{
    SIZE *totalSize = (SIZE *) lParam;
    RECT wndRect;
    HWND parentWnd = (HWND) GetWindowLong(childWnd, GWL_HWNDPARENT);

    GetWindowRect(childWnd, &wndRect);
    MapWindowPoints(HWND_DESKTOP, parentWnd, (POINT *) &wndRect, 2);

    if (wndRect.right > totalSize->cx)
        totalSize->cx = wndRect.right;
    if (wndRect.bottom > totalSize->cy)
        totalSize->cy = wndRect.bottom;

    return TRUE;
}

