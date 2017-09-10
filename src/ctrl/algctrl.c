/*
 * algctrl.c - Algorithm selection control
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
#include "dlg/voicedlg.h"
#include "ctrl/algctrl.h"

/*
 * Global constants
 */
const _TUCHAR *AlgCtrl_className = _T("AlgCtrl");

/*
 * Global procedures
 */
extern BOOL AlgCtrl_Register(void);

/*
 * Unit constants
 */
#define ALG_EXTRA          8
#define GWL_ALG_NUMBER     0
#define GWL_ALG_SCROLLBAR  4

/*
 * Unit procedures
 */
static void ac_CreateAlgNumFont(HWND algCtrl);
static void ac_DrawOperator(HDC dC, int x, int y, int opNum);
static BOOL ac_OnCreate(HWND algCtrl, LPCREATESTRUCT createStruct);
static void ac_OnDestroy(HWND algCtrl);
static int ac_OnGetValue(HWND algCtrl);
static void ac_OnHScroll(HWND algCtrl, HWND control, UINT code, int pos);
static void ac_OnKey(HWND algCtrl, UINT vk, BOOL down, int repeat, UINT flags);
static void ac_OnLButtonDown(HWND algCtrl, BOOL dblClick, int x, int y, UINT keyFlags);
static void ac_OnMouseWheel(HWND algCtrl, int delta, int x, int y, UINT keyFlags);
static void ac_OnPaint(HWND algCtrl);
static void ac_OnSetFocus(HWND algCtrl, HWND otherWnd);
static void ac_OnSetValue(HWND algCtrl, int value);
static void ac_ScrollBarOnChar(HWND scrollBar, _TUCHAR ch, int repeat);
static void ac_ScrollBarOnKey(HWND scrollBar, UINT vk, BOOL down, int repeat, UINT flags);
static LRESULT CALLBACK ac_WndProc(HWND algCtrl, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ac_WndProcScrollBar(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HFONT ac_algNumFont;
static WNDPROC ac_origSBWndProc;

/*
 * Procedure definitions
 */

/*
 * Register() - Registers the window class for the control.  Returns true on
 *              success, false on failure.
 */
BOOL AlgCtrl_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) ac_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = ALG_EXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = AlgCtrl_className;
    classInfo.hIconSm = NULL;

    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering AlgCtrl control"));
        return FALSE;
    }

    return TRUE;
}

/*
 * CreateAlgNumFont() - Creates the font to display the algorithm number at
 *                      the bottom.
 */
void ac_CreateAlgNumFont(HWND algCtrl)
{
    HDC dC = GetDC(algCtrl);

    ac_algNumFont = Font_CreateBold(dC, 9, _T("Arial"));
    ReleaseDC(algCtrl, dC);
}

/*
 * DrawOperator() - Draws a single operator on the DC.
 */
void ac_DrawOperator(HDC dC, int x, int y, int opNum)
{
    RECT rect = { x, y, x + 13, y + 11 };

    if (opNum == 4) {
        /* draw feedback */
        RECT fbRect = { x + 6, y - 3, x + 16, y + 13 };
        FrameRect(dC, &fbRect, Prog_wndTextBrush);
    }
    FillRect(dC, &rect, Prog_wndTextBrush);
    MiniFont_DrawChar(dC, x + 4, y + 2, '0' + opNum, Prog_wndColor);
}

/*
 * OnCreate()
 */
BOOL ac_OnCreate(HWND algCtrl, LPCREATESTRUCT createStruct)
{
    RECT clientRect;
    HWND scrollBar;

    GetClientRect(algCtrl, &clientRect);
    scrollBar = CreateWindowEx(
            0L
            , _T("ScrollBar")
            , (LPTSTR) NULL
            , WS_CHILD | WS_VISIBLE | SBS_HORZ
            , 0
            , clientRect.bottom - Prog_hSliderH
            , RECT_W(clientRect)
            , Prog_hSliderH
            , algCtrl
            , (HMENU) IDC_ALGORITHM
            , Prog_instance
            , (LPVOID) NULL
        );
    if (!scrollBar) {
        MsgBox_LastErrorF(Prog_mainWnd, _T("Error creating algorithm control:"));
        return FALSE;
    }
    ScrollBar_SetRange(scrollBar, 0, 7);
    ScrollBar_SetPos(scrollBar, 0, FALSE);
    if (!ac_algNumFont) {
        ac_CreateAlgNumFont(algCtrl);
    }
    SetWindowLong(algCtrl, GWL_ALG_SCROLLBAR, (long) scrollBar);
    ac_origSBWndProc = (WNDPROC) SetWindowLong(scrollBar, GWLP_WNDPROC, (long) ac_WndProcScrollBar);

    return TRUE;
}

/*
 * OnDestroy()
 */
void ac_OnDestroy(HWND algCtrl)
{
}

/*
 * OnGetValue()
 */
int ac_OnGetValue(HWND algCtrl)
{
    return GetWindowLong(algCtrl, GWL_ALG_NUMBER);
}

/*
 * OnHScroll()
 */
void ac_OnHScroll(HWND algCtrl, HWND control, UINT code, int pos)
{
    UINT ctrlID = GetWindowLong(algCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(algCtrl, GWLP_HWNDPARENT);
    int value = ScrollBar_GetPos(control);
    int oldValue = value;
#define scrollMax 7
#define pageSize 1

    switch (code) {
        case SB_ENDSCROLL:
            break;
		case SB_LEFT:
            value = 0;
            break;
		case SB_RIGHT:
            value = scrollMax;
            break;
		case SB_LINELEFT:
            if (value > 0)
                value--;
            break;
		case SB_LINERIGHT:
            if (value < scrollMax)
                value++;
            break;
		case SB_PAGELEFT:
            value -= pageSize;
            if (value < 0)
                value = 0;
            break;
		case SB_PAGERIGHT:
            value += pageSize;
            if (value > scrollMax)
                value = scrollMax;
            break;
        case SB_THUMBPOSITION:
            value = pos;
            break;
		case SB_THUMBTRACK:
            value = pos;
            break;
    }
#undef scrollMax
#undef pageSize
    if (value != oldValue) {
        RECT algRect;

        /*
         * Redraw the algorithm display, carefully avoiding invalidating the
         * scroll bar.
         */
        GetClientRect(algCtrl, &algRect);
        algRect.bottom -= Prog_hSliderH;
        InvalidateRect(algCtrl, &algRect, TRUE);
        /*
         * Set the scroll bar position.
         */
        ScrollBar_SetPos(control, value, TRUE);
        /*
         * Save the value in the window.
         */
        SetWindowLong(algCtrl, GWL_ALG_NUMBER, value);
        /*
         * Notify parent window.
         */
        PostNotification(parentWnd, ctrlID, algCtrl, ACN_VALUECHANGED);
    }
}

/*
 * OnKey()
 */
void ac_OnKey(HWND algCtrl, UINT vk, BOOL down, int repeat, UINT flags)
{
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            FORWARD_WM_KEYDOWN(GetParent(algCtrl), vk, repeat, flags
                    , SendMessage);
            return;
    }
}

/*
 * OnLButtonDown()
 */
void ac_OnLButtonDown(HWND algCtrl, BOOL dblClick, int x, int y, UINT keyFlags)
{
    SetFocus(algCtrl);
}

/*
 * OnMouseWheel()
 */
void ac_OnMouseWheel(HWND algCtrl, int delta, int x, int y, UINT keyFlags)
{
    if (VoiceDlg_mouseWheel == MW_EDIT) {
        ac_OnHScroll(algCtrl, (HWND) GetWindowLong(algCtrl, GWL_ALG_SCROLLBAR)
                , delta < 0 ? SB_LINELEFT : SB_LINERIGHT, 0);
    } else {
        FORWARD_WM_MOUSEWHEEL(Prog_voiceDlg, delta, x, y, keyFlags, PostMessage);
    }
}

/*
 * OnPaint()
 */
void ac_OnPaint(HWND algCtrl)
{
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(algCtrl, &paint);
    RECT clientRect;
    RECT algNumRect;
    int algNum = GetWindowLong(algCtrl, GWL_ALG_NUMBER);
    _TUCHAR algNumText = '1' + algNum;

    SelectBrush(paintDC, Prog_wndBrush);
    SelectPen(paintDC, Prog_wndTextPen);
    SelectFont(paintDC, ac_algNumFont);

    GetClientRect(algCtrl, &clientRect);
    Rectangle(paintDC, PASS_RECT_FIELDS(clientRect));
    algNumRect = clientRect;
    algNumRect.bottom -= Prog_hSliderH;
    algNumRect.top = algNumRect.bottom - 16;
    SelectBrush(paintDC, Prog_wndTextBrush);
    Rectangle(paintDC, PASS_RECT_FIELDS(algNumRect));
    SetTextColor(paintDC, Prog_wndColor);
    SetBkColor(paintDC, Prog_wndTextColor);
    algNumRect.bottom -= 2;  // the text is a little low
    DrawText(paintDC, &algNumText, 1, &algNumRect, DT_CENTER | DT_VCENTER
            | DT_SINGLELINE);

    {
        int col2 = Rect_HCenter(&clientRect) - 8;
        int col1 = col2 - 18;
        int col3 = col2 + 18;
        int col4 = col3 + 18;
        int bottom = algNumRect.top - 3;
        int row1 = bottom - 15;
        int row2 = row1 - 16;
        int row3 = row2 - 16;
        int row4 = row3 - 16;
        POINT opPoses[8][4] = {
            { { col2, row1 }, { col2, row2 }, { col2, row3 }, { col2, row4 } },
            { { col2, row1 }, { col2, row2 }, { col2, row3 }, { col3, row3 } },
            { { col2, row1 }, { col2, row2 }, { col2, row3 }, { col3, row2 } },
            { { col2, row1 }, { col2, row2 }, { col3, row2 }, { col3, row3 } },
            { { col2 - 9, row1 }, { col2 - 9, row2 }, { col3 - 9, row1 }
                , { col3 - 9, row2 } },
            { { col1, row1 }, { col2, row1 }, { col3, row1 }, { col2, row2 } },
            { { col1, row1 }, { col2, row1 }, { col3, row1 }, { col3, row2 } },
            { { col1 - 9, row1 }, { col2 - 9, row1 }, { col3 - 9, row1 }
                , { col4 - 9, row1 } }
        };
        int i;

        col1 += 6;
        col2 += 6;
        col3 += 6;
        col4 += 6;
        switch (algNum) {
            case 0:
                MoveToEx(paintDC, col2, bottom, NULL);
                LineTo(paintDC, col2, row4);
                break;
            case 1:
                MoveToEx(paintDC, col2, bottom, NULL);
                LineTo(paintDC, col2, row3);
                MoveToEx(paintDC, col2, row2 - 2, NULL);
                LineTo(paintDC, col3, row2 - 2);
                LineTo(paintDC, col3, row3);
                break;
            case 2:
                MoveToEx(paintDC, col2, bottom, NULL);
                LineTo(paintDC, col2, row3);
                MoveToEx(paintDC, col2, row1 - 2, NULL);
                LineTo(paintDC, col3, row1 - 2);
                LineTo(paintDC, col3, row2);
                break;
            case 3:
                MoveToEx(paintDC, col2, bottom, NULL);
                LineTo(paintDC, col2, row2);
                MoveToEx(paintDC, col2, row1 - 2, NULL);
                LineTo(paintDC, col3, row1 - 2);
                LineTo(paintDC, col3, row3);
                break;
            case 4:
                MoveToEx(paintDC, col2 - 9, row2, NULL);
                LineTo(paintDC, col2 - 9, bottom);
                LineTo(paintDC, col3 - 9, bottom);
                LineTo(paintDC, col3 - 9, row2);
                break;
            case 5:
                MoveToEx(paintDC, col1, row1 - 2, NULL);
                LineTo(paintDC, col1, bottom);
                LineTo(paintDC, col3, bottom);
                LineTo(paintDC, col3, row1 - 2);
                LineTo(paintDC, col1, row1 - 2);
                MoveToEx(paintDC, col2, row2, NULL);
                LineTo(paintDC, col2, bottom);
                break;
            case 6:
                MoveToEx(paintDC, col1, row1, NULL);
                LineTo(paintDC, col1, bottom);
                LineTo(paintDC, col3, bottom);
                LineTo(paintDC, col3, row2);
                MoveToEx(paintDC, col2, bottom, NULL);
                LineTo(paintDC, col2, row1);
                break;
            case 7:
                MoveToEx(paintDC, col1 - 9, row1, NULL);
                LineTo(paintDC, col1 - 9, bottom);
                LineTo(paintDC, col4 - 9, bottom);
                LineTo(paintDC, col4 - 9, row1);
                MoveToEx(paintDC, col2 - 9, row1, NULL);
                LineTo(paintDC, col2 - 9, bottom);
                MoveToEx(paintDC, col3 - 9, row1, NULL);
                LineTo(paintDC, col3 - 9, bottom);
                break;
        }
        for (i = 0; i < 4; i++) {
            ac_DrawOperator(paintDC, opPoses[algNum][i].x
                    , opPoses[algNum][i].y, i + 1);
        }
    }
    EndPaint(algCtrl, &paint);
}

/*
 * OnSetFocus()
 */
void ac_OnSetFocus(HWND algCtrl, HWND otherWnd)
{
    SetFocus((HWND) GetWindowLong(algCtrl, GWL_ALG_SCROLLBAR));
}

/*
 * OnSetValue()
 */
void ac_OnSetValue(HWND algCtrl, int value)
{
    HWND scrollBar = (HWND) GetWindowLong(algCtrl, GWL_ALG_SCROLLBAR);

    if (value >= 0 && value <= 7) {
        ScrollBar_SetPos(scrollBar, value, TRUE);
        SetWindowLong(algCtrl, GWL_ALG_NUMBER, value);
    }
}

/*
 * ScrollBarOnChar() - Set the algorithm to the number pressed.
 */
void ac_ScrollBarOnChar(HWND scrollBar, _TUCHAR ch, int repeat)
{
    if (isdigit(ch)) {
        HWND algCtrl = GetParent(scrollBar);
        HWND parentWnd = GetParent(algCtrl);
        UINT ctrlID = GetWindowLong(algCtrl, GWL_ID);
        int value = ch - '1';

        if (value >= 0 && value <= 7) {
            ac_OnSetValue(algCtrl, value);
            PostNotification(parentWnd, ctrlID, algCtrl, ACN_VALUECHANGED);
            InvalidateRect(algCtrl, NULL, TRUE);
        }
    }
}

/*
 * ScrollBarOnKey() - Handle cursor keys.
 */
void ac_ScrollBarOnKey(HWND scrollBar, UINT vk, BOOL down, int repeat
        , UINT flags)
{
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            FORWARD_WM_KEYDOWN(GetParent(scrollBar), vk, repeat, flags
                    , SendMessage);
            return;
        case VK_INSERT:
            vk = VK_DOWN;
            break;
        case VK_DELETE:
            vk = VK_UP;
            break;
        case VK_HOME:
            vk = VK_END;
            break;
        case VK_END:
            vk = VK_HOME;
            break;
        case VK_NEXT:
            vk = VK_PRIOR;
            break;
        case VK_PRIOR:
            vk = VK_NEXT;
            break;
    }
    CallWindowProc(ac_origSBWndProc, scrollBar, WM_KEYDOWN, (WPARAM) vk
            , MAKELPARAM(repeat, flags));
}

/*
 * WndProc()
 */
LRESULT CALLBACK ac_WndProc(HWND algCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(algCtrl, ACM_GETVALUE, ac_OnGetValue);
        HANDLE_MSG(algCtrl, ACM_SETVALUE, ac_OnSetValue);
        HANDLE_MSG(algCtrl, WM_CREATE, ac_OnCreate);
        HANDLE_MSG(algCtrl, WM_DESTROY, ac_OnDestroy);
        HANDLE_MSG(algCtrl, WM_HSCROLL, ac_OnHScroll);
        HANDLE_MSG(algCtrl, WM_KEYDOWN, ac_OnKey);
        HANDLE_MSG(algCtrl, WM_LBUTTONDOWN, ac_OnLButtonDown);
        HANDLE_MSG(algCtrl, WM_MOUSEWHEEL, ac_OnMouseWheel);
        HANDLE_MSG(algCtrl, WM_PAINT, ac_OnPaint);
        HANDLE_MSG(algCtrl, WM_SETFOCUS, ac_OnSetFocus);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(algCtrl);

            MapWindowPoints(algCtrl, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
    }
    return DefWindowProc(algCtrl, message, wParam, lParam);
}

/*
 * WndProcScrollBar()
 */
LRESULT CALLBACK ac_WndProcScrollBar(HWND scrollBar, UINT message
        , WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(scrollBar, WM_CHAR, ac_ScrollBarOnChar);
        HANDLE_MSG(scrollBar, WM_KEYDOWN, ac_ScrollBarOnKey);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND grandparentWnd = GetParent(GetParent(scrollBar));

            MapWindowPoints(scrollBar, grandparentWnd, &pt, 1);
            PostMessage(grandparentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_LBUTTONDOWN:
            if (GetFocus() != scrollBar) {
                SetFocus(scrollBar);
            }
            break;
    }
    return CallWindowProc(ac_origSBWndProc, scrollBar, message, wParam
            , lParam);
}
