/*
 * aboutdlg.c - about box class
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
#include "prog.h"
#include "resource.h"
#include "dlg/aboutdlg.h"

/*
 * Unit procedures
 */
static void ad_DrawFocusRect(HWND aboutDlg);
static void ad_OnCommand(HWND aboutDlg, int cmdID, HWND control, UINT notify);
static BOOL ad_OnEraseBkgnd(HWND aboutDlg, HDC dC);
static BOOL ad_OnInitDialog(HWND aboutDlg, HWND focusCtrl, LPARAM lParam);
static void ad_OnLButtonDown(HWND aboutDlg, BOOL dblClick, int x, int y, UINT keyFlags);
static void ad_OnLButtonUp(HWND aboutDlg, int x, int y, UINT keyFlags);
static void ad_OnMouseMove(HWND aboutDlg, int x, int y, UINT keyFlags);
static void ad_OnPaint(HWND aboutDlg);
static BOOL ad_OnSetCursor(HWND aboutDlg, HWND cursorWnd, UINT codeHitTest, UINT msg);
static BOOL CALLBACK ad_DlgProc(HWND aboutDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HCURSOR ad_cursor;
static RECT ad_clientRect;
static _TUCHAR ad_prog[80];
static int ad_progLen;
static int ad_copyLen;
static const _TUCHAR ad_websiteLabel[] = _T("Website:");
#define ad_websiteLabelLen  STRSIZE(ad_websiteLabel)
static const _TUCHAR ad_website[] = _T("https://mgregory22.me/tx81z/programmer.html");
#define ad_websiteLen  STRSIZE(ad_website)
static const _TUCHAR ad_emailLabel[] = _T("Email:");
#define ad_emailLabelLen  STRSIZE(ad_emailLabel)
static const _TUCHAR ad_email[] = _T("mailto:mgregory22@gmail.com");
#define ad_emailLen  STRSIZE(ad_email)
static const _TUCHAR ad_ack1[] = _T("Special thanks to Fran Bourdeau and Jesse Hager!");
#define ad_ack1Len  STRSIZE(ad_ack1)
#define ad_websiteTop     132
#define ad_websiteBottom  (ad_websiteTop + 15)
#define ad_emailTop       152
#define ad_emailBottom    (ad_emailTop + 15)
#define ad_ack1Top        172
#define ad_ack1Bottom     (ad_ack1Top + 15)
static RECT ad_websiteRect = { 132, ad_websiteTop, 380, ad_websiteBottom };
static RECT ad_emailRect = { 132, ad_emailTop, 302, ad_emailBottom };
static BOOL ad_mouseDown;
static RECT *focusRect;

/*
 * Create - displays the about box
 */
void AboutDlg_Create(HWND mainWnd)
{
    focusRect = NULL;
    DialogBoxParam(Prog_instance, (LPCTSTR) IDD_ABOUTDLG, mainWnd, ad_DlgProc
            , (LPARAM) mainWnd);
}

/*
 * DrawFocusRect() - Draws a focus rect around the clicked URL.
 */
void ad_DrawFocusRect(HWND aboutDlg)
{
    if (focusRect) {
        HDC dC = GetDC(aboutDlg);

        DrawFocusRect(dC, focusRect);
        ReleaseDC(aboutDlg, dC);
    }
}

/*
 * OnCommand - handles OK and X buttons
 */
void ad_OnCommand(HWND aboutDlg, int cmdID, HWND control, UINT notify)
{
    switch (cmdID) {
        case IDOK:
        case IDCANCEL:
            EndDialog(aboutDlg, cmdID);
            return;
    }
}

/*
 * OnEraseBkgnd()
 */
BOOL ad_OnEraseBkgnd(HWND aboutDlg, HDC dC)
{
    FillRect(dC, &ad_clientRect, Prog_whiteBrush);

    return TRUE; /* background has been erased */
}

/*
 * OnInitDialog - centers the dialog in the main window
 */
BOOL ad_OnInitDialog(HWND aboutDlg, HWND focusCtrl, LPARAM lParam)
{
    /*
     * Get the window size.
     */
    GetClientRect(aboutDlg, &ad_clientRect);
    /*
     * Calculate the string lengths.
     */
    ad_progLen = _stprintf(ad_prog, _T("%s v%s"), Prog_name, Prog_version);
    ad_copyLen = _tcslen(Prog_copyright);

    return TRUE;
}

/*
 * OnLButtonDown()
 */
void ad_OnLButtonDown(HWND aboutDlg, BOOL dblClick, int x, int y, UINT keyFlags)
{
    if (dblClick) {
        return;
    }
    if (Rect_XYIn(&ad_websiteRect, x, y)) {
        if (focusRect == &ad_emailRect) {
            ad_DrawFocusRect(aboutDlg);
        }
        focusRect = &ad_websiteRect;
        ad_DrawFocusRect(aboutDlg);
        ad_mouseDown = TRUE;
    } else if (Rect_XYIn(&ad_emailRect, x, y)) {
        if (focusRect == &ad_websiteRect) {
            ad_DrawFocusRect(aboutDlg);
        }
        focusRect = &ad_emailRect;
        ad_DrawFocusRect(aboutDlg);
        ad_mouseDown = TRUE;
    } else if (focusRect) {
        ad_DrawFocusRect(aboutDlg);
        focusRect = NULL;
    }
}

/*
 * OnLButtonUp()
 */
void ad_OnLButtonUp(HWND aboutDlg, int x, int y, UINT keyFlags)
{
    if (ad_mouseDown) {
        if (Rect_XYIn(&ad_websiteRect, x, y)) {
            ShellExecute(NULL, _T("open"), ad_website, NULL, NULL, SW_SHOW);
        } else if (Rect_XYIn(&ad_emailRect, x, y)) {
            _TUCHAR link[128];

            _stprintf(link, _T("%s?subject=TX81Z%%20Programmer"), ad_email);
            ShellExecute(NULL, _T("open"), link, NULL, NULL, SW_SHOW);
        }
        ad_mouseDown = FALSE;
    }
}

/*
 * OnMouseMove()
 */
void ad_OnMouseMove(HWND aboutDlg, int x, int y, UINT keyFlags)
{
    if (Rect_XYIn(&ad_websiteRect, x, y) || Rect_XYIn(&ad_emailRect, x, y)) {
        ad_cursor = Prog_linkCursor;
    } else {
        ad_cursor = Prog_arrowCursor;
    }
}

/*
 * OnPaint()
 */
void ad_OnPaint(HWND aboutDlg)
{
    PAINTSTRUCT paint;
    HDC dC = BeginPaint(aboutDlg, &paint);
    HFONT oldFont;
    RECT textRect = { 0, 95, ad_clientRect.right, 110 };

    /*
     * Set up the DC.
     */
    oldFont = SelectFont(dC, Prog_tahomaBoldFont);
    SetTextColor(dC, BLACK);
    SetBkColor(dC, WHITE);
    /*
     * Draw the program name and version.
     */
    DrawText(dC, ad_prog, ad_progLen, &textRect, DT_CENTER | DT_SINGLELINE);
    /*
     * Draw the acknowledgements.
     */
    textRect.top = 180;
    textRect.bottom = 195;
    DrawText(dC, ad_ack1, ad_ack1Len, &textRect, DT_CENTER | DT_SINGLELINE);
    /*
     * Draw the copyright notice.
     */
    textRect.top = 110;
    textRect.bottom = 125;
    DrawText(dC, Prog_copyright, ad_copyLen, &textRect, DT_CENTER | DT_SINGLELINE);
    /*
     * Draw the website label.
     */
    RECT_SET(textRect, 72, ad_websiteTop, 122, ad_websiteBottom);
    DrawText(dC, ad_websiteLabel, ad_websiteLabelLen, &textRect, DT_RIGHT | DT_SINGLELINE);
    /*
     * Draw the email label.
     */
    RECT_SET(textRect, 72, ad_emailTop, 122, ad_emailBottom);
    DrawText(dC, ad_emailLabel, ad_emailLabelLen, &textRect, DT_RIGHT | DT_SINGLELINE);
    /*
     * Set up a hyperlink font.
     */
    SelectFont(dC, Prog_tahomaUnderlineFont);
    SetTextColor(dC, BLUE);
    DrawText(dC, ad_website, ad_websiteLen, &ad_websiteRect, DT_LEFT | DT_SINGLELINE);
    DrawText(dC, ad_email, ad_emailLen, &ad_emailRect, DT_LEFT | DT_SINGLELINE);
    /*
     * Draw the focus rect if a link is selected.
     */
    if (focusRect) {
        DrawFocusRect(dC, focusRect);
    }
    /*
     * Clean up.
     */
    SelectFont(dC, oldFont);
    EndPaint(aboutDlg, &paint);
}

/*
 * OnSetCursor()
 */
BOOL ad_OnSetCursor(HWND aboutDlg, HWND cursorWnd, UINT codeHitTest, UINT msg)
{
    SetCursor(ad_cursor);

    return TRUE;
}

/*
 * DlgProc
 */
BOOL CALLBACK ad_DlgProc(HWND aboutDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(aboutDlg, WM_ERASEBKGND, ad_OnEraseBkgnd);
        HANDLE_MSG(aboutDlg, WM_INITDIALOG, ad_OnInitDialog);
        HANDLE_MSG(aboutDlg, WM_COMMAND, ad_OnCommand);
        HANDLE_MSG(aboutDlg, WM_LBUTTONDOWN, ad_OnLButtonDown);
        HANDLE_MSG(aboutDlg, WM_LBUTTONUP, ad_OnLButtonUp);
        HANDLE_MSG(aboutDlg, WM_MOUSEMOVE, ad_OnMouseMove);
        HANDLE_MSG(aboutDlg, WM_PAINT, ad_OnPaint);
        HANDLE_MSG(aboutDlg, WM_SETCURSOR, ad_OnSetCursor);
    }
    return FALSE;
}
