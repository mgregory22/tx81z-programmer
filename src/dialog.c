/*
 * gui/dialog.c - Generic dialog routines
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
extern DIALOG *Dialog_Create(HWND dialogWnd);
extern void Dialog_Destroy(DIALOG *dialog);
extern BOOL Dialog_DoEvents(HWND dialogWnd);
extern void Dialog_FitToControls(DIALOG *dialog);
extern BOOL CALLBACK Dialog_GenericProc(HWND dlg, UINT message, WPARAM wParam
        , LPARAM lParam);
extern void Dialog_OnHScroll(DIALOG *dialog, HWND control, UINT code, int pos);
extern void Dialog_OnSize(DIALOG *dialog, UINT state, int cx, int cy);
extern void Dialog_OnVScroll(DIALOG *dialog, HWND control, UINT code, int pos);
extern void Dialog_Scroll(DIALOG *dialog, int dx, int dy);
extern void Dialog_UpdateScrollBars(DIALOG *dialog);

/*
 * Procedure definitions
 */

DIALOG *Dialog_Create(HWND dialogWnd)
{
    DIALOG *dialog = malloc(sizeof *dialog);
    RECT wndRect;

    if (!dialog) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    dialog->wnd = dialogWnd;
    GetClientRect(dialogWnd, &dialog->clientRect);
    GetWindowRect(dialogWnd, &wndRect);
    dialog->diffSize.cx = Rect_Width(&wndRect) - dialog->clientRect.right;
    dialog->diffSize.cy = Rect_Height(&wndRect) - dialog->clientRect.bottom;
    dialog->ctrlAreaSize = Window_CalcControlAreaSize(dialog->wnd);

    dialog->hScrollInfo.cbSize = sizeof(SCROLLINFO);
    dialog->hScrollInfo.fMask = SIF_ALL;
    dialog->hScrollInfo.nPage = dialog->clientRect.right;
    dialog->hScrollInfo.nMin = 0;
    dialog->hScrollInfo.nMax = dialog->ctrlAreaSize.cx;
    dialog->hScrollInfo.nPos = 0;
    dialog->vScrollInfo.cbSize = sizeof(SCROLLINFO);
    dialog->vScrollInfo.fMask = SIF_ALL;
    dialog->vScrollInfo.nPage = dialog->clientRect.bottom;
    dialog->vScrollInfo.nMin = 0;
    dialog->vScrollInfo.nMax = dialog->ctrlAreaSize.cy;
    dialog->vScrollInfo.nPos = 0;
    dialog->hideScrollBars = FALSE;
#ifdef DIALOG_SIZEBOX
    dialog->sizeBox = CreateWindowEx(
            0L
            , _T("ScrollBar")
            , (LPTSTR) NULL
            , WS_CHILD | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN
            , dialog->clientRect.left
            , dialog->clientRect.top
            , Rect_Width(&dialog->clientRect), Rect_Height(&dialog->clientRect)
            , dialog->wnd
            , (HMENU) 20
            , GetModuleHandle(NULL)
            , (LPVOID) NULL
        );
#endif

    return dialog;
}

void Dialog_Destroy(DIALOG *dialog)
{
    free(dialog);
}

BOOL Dialog_DoEvents(HWND dialogWnd)
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        if (!IsDialogMessage(dialogWnd, &msg)) {
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

void Dialog_FitToControls(DIALOG *dialog)
{
    RECT wndRect;
    DWORD style = GetWindowLong(dialog->wnd, GWL_STYLE);
    RECT workAreaRect;
    int workAreaH, workAreaW;
    int heightAdjustment = 0;
    int widthAdjustment = 0;

    /*
     * Reset the scroll position and remove the scroll bars.
     */
    if (dialog->hScrollInfo.nPos > 0 || dialog->vScrollInfo.nPos > 0) {
        Dialog_Scroll(dialog, dialog->hScrollInfo.nPos
                , dialog->vScrollInfo.nPos);
    }
    if (style & WS_HSCROLL) {
        style &= ~WS_HSCROLL;
        dialog->clientRect.bottom += GetSystemMetrics(SM_CYHSCROLL);
    }
    if (style & WS_VSCROLL) {
        style &= ~WS_VSCROLL;
        dialog->clientRect.right += GetSystemMetrics(SM_CXVSCROLL);
    }
    SetWindowLong(dialog->wnd, GWL_STYLE, style);

    /*
     * Reset the window size.
     */
    GetWindowRect(dialog->wnd, &wndRect);
    wndRect.right = wndRect.left + dialog->ctrlAreaSize.cx
            + dialog->diffSize.cx;
    wndRect.bottom = wndRect.top + dialog->ctrlAreaSize.cy
            + dialog->diffSize.cy;
    /*
     * Get the current work area and make sure the window fits entirely 
     * inside it.
     */
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
    workAreaW = RECT_W(workAreaRect);
    workAreaH = RECT_H(workAreaRect);
    if (wndRect.left < workAreaRect.left) {
        int dx = workAreaRect.left - wndRect.left;

        wndRect.left += dx;
        wndRect.right += dx;
    }
    if (wndRect.right > workAreaW) {
        if (wndRect.left > workAreaRect.left) {
            int dx = wndRect.left - workAreaRect.left;

            wndRect.left -= dx;
            wndRect.right -= dx;
        }
        if (wndRect.right > workAreaW) {
            widthAdjustment = wndRect.right - workAreaRect.right;
            wndRect.right -= widthAdjustment;
        }
    }
    if (wndRect.top < workAreaRect.top) {
        int dy = workAreaRect.top - wndRect.top;

        wndRect.top += dy;
        wndRect.bottom += dy;
    }
    if (wndRect.bottom > workAreaH) {
        if (wndRect.top > workAreaRect.top) {
            int dy = wndRect.top - workAreaRect.top;

            wndRect.top -= dy;
            wndRect.bottom -= dy;
        }
        if (wndRect.bottom > workAreaH) {
            heightAdjustment = wndRect.bottom - workAreaRect.bottom;
            wndRect.bottom -= heightAdjustment;
        }
    }
    MoveWindow(dialog->wnd, wndRect.left, wndRect.top, Rect_Width(&wndRect)
            , Rect_Height(&wndRect), TRUE);

    dialog->clientRect.right = dialog->ctrlAreaSize.cx - heightAdjustment;
    dialog->clientRect.bottom = dialog->ctrlAreaSize.cy - widthAdjustment;
    /*
     * Make sure the window isn't falling off the screen.
     */
    SendMessage(dialog->wnd, DM_REPOSITION, (WPARAM) 0, (LPARAM) 0);
    Dialog_UpdateScrollBars(dialog);
}

/*
 * GenericProc()
 */
BOOL CALLBACK Dialog_GenericProc(HWND dlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    WORD cmdID;

    switch (message) {
        case WM_COMMAND:
            cmdID = LOWORD(wParam);
            if (cmdID == IDOK || cmdID == IDCANCEL) {
                EndDialog(dlg, cmdID);
            }
            break;
        case WM_INITDIALOG:
            Window_CenterInParent(dlg);
            ShowWindow(dlg, SW_SHOW);
            return TRUE;
    }
    return FALSE;
}

/*
 * OnHScroll()
 */
void Dialog_OnHScroll(DIALOG *dialog, HWND control, UINT code, int pos)
{
    SCROLLINFO *hScrollInfo = &dialog->hScrollInfo;
    int dx = 0;

    switch (code) {
        case SB_ENDSCROLL:
            break;
		case SB_LEFT:
            dx = -hScrollInfo->nPos;
            break;
		case SB_RIGHT:
            dx = hScrollInfo->nMax;
            break;
		case SB_LINELEFT:
            dx = -20;
            break;
		case SB_LINERIGHT:
            dx = 20;
            break;
		case SB_PAGELEFT:
            dx = -((signed) hScrollInfo->nPage);
            break;
		case SB_PAGERIGHT:
            dx = hScrollInfo->nPage;
            break;
        case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
            dx = pos - hScrollInfo->nPos;
            break;
    }
    if (dx) {
        Dialog_Scroll(dialog, dx, 0);
        SetScrollInfo(dialog->wnd, SB_HORZ, hScrollInfo, TRUE);
    }
}

/*
 * OnSize()
 */
void Dialog_OnSize(DIALOG *dialog, UINT state, int cx, int cy)
{
    RECT *clientRect = &dialog->clientRect;

    if (cx > clientRect->right) {
        RECT invalidRect = { clientRect->right, 0, cx, cy };
        InvalidateRect(dialog->wnd, &invalidRect, TRUE);
    }
    if (cy > clientRect->bottom) {
        RECT invalidRect = { 0, clientRect->bottom, cx, cy };
        InvalidateRect(dialog->wnd, &invalidRect, TRUE);
    }
    clientRect->right = cx;
    clientRect->bottom = cy;
    Dialog_UpdateScrollBars(dialog);
}

/*
 * OnVScroll()
 */
void Dialog_OnVScroll(DIALOG *dialog, HWND control, UINT code, int pos)
{
    SCROLLINFO *vScrollInfo = &dialog->vScrollInfo;
    int dy = 0;

    switch (code) {
        case SB_BOTTOM:
            dy = vScrollInfo->nMax;
            break;
        case SB_ENDSCROLL:
            break;
        case SB_LINEDOWN:
            dy = 20;
            break;
        case SB_LINEUP:
            dy = -20;
            break;
        case SB_PAGEDOWN:
            dy = vScrollInfo->nPage;
            break;
        case SB_PAGEUP:
            dy = -((signed) vScrollInfo->nPage);
            break;
        case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
            dy = pos - vScrollInfo->nPos;
            break;
        case SB_TOP:
            dy = -vScrollInfo->nPos;
            break;
    }
    if (dy) {
        Dialog_Scroll(dialog, 0, dy);
        SetScrollInfo(dialog->wnd, SB_VERT, vScrollInfo, TRUE);
    }
}

void Dialog_Scroll(DIALOG *dialog, int dx, int dy)
{
    SCROLLINFO *hScrollInfo = &dialog->hScrollInfo;
    SCROLLINFO *vScrollInfo = &dialog->vScrollInfo;
    int max = hScrollInfo->nMax;
    int pos = hScrollInfo->nPos;
    int scrollMax = max - hScrollInfo->nPage + 1;

    if (pos + dx > scrollMax) {
        dx = scrollMax - pos;
    }
    if (pos + dx < 0) {
        dx = -pos;
    }

    max = vScrollInfo->nMax;
    pos = vScrollInfo->nPos;
    scrollMax = max - vScrollInfo->nPage + 1;
    if (pos + dy > scrollMax) {
        dy = scrollMax - pos;
    }
    if (pos + dy < 0) {
        dy = -pos;
    }
    if (dx || dy) {
        ScrollWindowEx(dialog->wnd, -dx, -dy, NULL, NULL
                , NULL, NULL, SW_SCROLLCHILDREN | SW_ERASE | SW_INVALIDATE);
        hScrollInfo->nPos += dx;
        vScrollInfo->nPos += dy;
    }
#ifdef DIALOG_SIZEBOX
    if (IsWindowVisible(dialog->sizeBox)) {
        RECT sizeBoxRect;

        sizeBoxRect.left = sizeBoxRect.right = dialog->clientRect.right;
        sizeBoxRect.top = sizeBoxRect.bottom = dialog->clientRect.bottom;
        sizeBoxRect.left -= GetSystemMetrics(SM_CXVSCROLL);
        sizeBoxRect.top -= GetSystemMetrics(SM_CYHSCROLL);

        SetWindowPos(dialog->sizeBox, HWND_TOP, sizeBoxRect.left
                , sizeBoxRect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        UpdateWindow(dialog->wnd);
        RedrawWindow(dialog->sizeBox, NULL, NULL, RDW_ERASE | RDW_INVALIDATE
                | RDW_INTERNALPAINT | RDW_UPDATENOW);
    }
#endif
}

void Dialog_UpdateScrollBars(DIALOG *dialog)
{
    DWORD oldStyle = GetWindowLong(dialog->wnd, GWL_STYLE);
    DWORD style = oldStyle & ~(WS_HSCROLL | WS_VSCROLL);
    RECT wndRect;
    int noSBWidth;   /* width of client area with no vertical scrollbar */
    int sbWidth;     /* width of client area with vertical scrollbar */
    int noSBHeight;  /* height of client area with no horizontal scrollbar */
    int sbHeight;    /* height of client area with horizontal scrollbar */
    int clientWidth;
    int clientHeight;
    int ctrlAreaCx = dialog->ctrlAreaSize.cx;
    int ctrlAreaCy = dialog->ctrlAreaSize.cy;
    SCROLLINFO *hScrollInfo = &dialog->hScrollInfo;
    SCROLLINFO *vScrollInfo = &dialog->vScrollInfo;
    int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
    int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

    /*
     * Check the window size and activate or deactivate scroll bars.
     */
    GetWindowRect(dialog->wnd, &wndRect);
    noSBWidth = Rect_Width(&wndRect) - dialog->diffSize.cx;
    sbWidth = noSBWidth - cxVScroll;
    noSBHeight = Rect_Height(&wndRect) - dialog->diffSize.cy;
    sbHeight = noSBHeight - cyHScroll;

    clientWidth = noSBWidth;
    clientHeight = noSBHeight;
    if (!dialog->hideScrollBars) {
        if (noSBWidth < ctrlAreaCx) {
            style |= WS_HSCROLL;
            clientWidth = sbWidth;
        }
        if (noSBHeight < ctrlAreaCy) {
            style |= WS_VSCROLL;
            clientHeight = sbHeight;
        }
        if (sbWidth < ctrlAreaCx && (style & WS_VSCROLL)) {
            style |= WS_HSCROLL;
            clientWidth = sbWidth;
        }
        if (sbHeight < ctrlAreaCy && (style & WS_HSCROLL)) {
            style |= WS_VSCROLL;
            clientHeight = sbHeight;
        }
    }
    SetWindowLong(dialog->wnd, GWL_STYLE, style);

    /*
     * If the window has scroll bars, update them to reflect the ratio
     * between the window size and its virtual size.
     */
    hScrollInfo->nMin = 0;
    hScrollInfo->nPage = clientWidth;
    hScrollInfo->nMax = ctrlAreaCx;
    if (hScrollInfo->nPos + (signed) clientWidth > hScrollInfo->nMax)
    {
        Dialog_Scroll(dialog, hScrollInfo->nPos + clientWidth - ctrlAreaCx, 0);
    }
    if (style & WS_HSCROLL) {
        SetScrollInfo(dialog->wnd, SB_HORZ, hScrollInfo, TRUE);
    }
    vScrollInfo->nMin = 0;
    vScrollInfo->nPage = clientHeight;
    vScrollInfo->nMax = ctrlAreaCy;
    if (vScrollInfo->nPos + (signed) clientHeight > ctrlAreaCy) {
        Dialog_Scroll(dialog, 0
                , dialog->vScrollInfo.nPos + clientHeight - ctrlAreaCy);
    }
    if (style & WS_VSCROLL) {
        SetScrollInfo(dialog->wnd, SB_VERT, &dialog->vScrollInfo, TRUE);
    }
    SetWindowPos(dialog->wnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED
            | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
#ifdef DIALOG_SIZEBOX
    if (style & (WS_HSCROLL | WS_VSCROLL)) {
        ShowWindow(dialog->sizeBox, SW_HIDE);
    } else {
        SetWindowPos(dialog->sizeBox, HWND_TOP
                , clientWidth - cxVScroll, clientHeight - cyHScroll
                , 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    }
#endif
}

