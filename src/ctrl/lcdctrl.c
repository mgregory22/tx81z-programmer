/*
 * lcdctrl.c - LCD edit control
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
#include "ctrl/lcdctrl.h"

#ifdef max
#   undef max
#endif
#ifdef min
#   undef min
#endif

/*
 * Macros
 */
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

/*
 * Data types
 */
typedef enum {
    LF_NONE          = 0x00,
    LF_JUSTGOTFOCUS  = 0x01,  /* used to erase the contents of a numeric lcd
                              when a user clicks and types for the first time */
    LF_INSERTMODE    = 0x02,
    LF_SELECTED      = 0x04,
    LF_HIGHLIGHTED   = 0x08,  /* draw the LCD with the highlight color */
} LCDFLAGS;

typedef struct {
    int maxLen;              /* width of the LCD display in characters */
    int cursorPos;           /* the cursor position for text controls */
    _TUCHAR *data;             /* holds the currently displayed text -  created in lc_OnSetLength */
    int min;                 /* minimum scroll bar value */
    int max;                 /* maximum scroll bar value */
    int offset;              /* difference in the current value to display */
    const _TUCHAR *strings;    /* display string list for special LCDs */
    LCDPAGEFUNC LcdPageFunc; /* callback that returns the amount to change the value when paging */
    HWND scrollBar;          /* the child scroll bar control */
    LCDFLAGS flags;          /* internal status of the LCD */
    RECT lcdRect;            /* LCD display rectangle */
} LCDDATA;

typedef enum {
    DO_NONE = 0,
    DO_COPY = 1,
    DO_SWAP = 2,
} DRAGOPERATION;

/*
 * Global constants
 */
const _TUCHAR LcdCtrl_className[] = _T("LcdCtrl");

/*
 * Unit constants
 */
#define LCD_EXTRA        4
#define GWL_LCDDATA      0

#define LARGE_CURSOR_WIDTH   12
#define LARGE_CURSOR_HEIGHT  16
#define LARGE_DELTA          (LARGE_CURSOR_WIDTH - 1)
#define SMALL_CURSOR_WIDTH   7
#define SMALL_CURSOR_HEIGHT  11
#define SMALL_DELTA          (SMALL_CURSOR_WIDTH - 1)
#define LARGE_LCD_HEIGHT     20
#define SMALL_LCD_HEIGHT     13
#define SMALL_HORZ_PADDING   2
#define SMALL_VERT_PADDING   3
#define LARGE_HORZ_PADDING   3
#define LARGE_VERT_PADDING   3
#define LARGE_CURSOR_HORZ_ORG 4
#define LARGE_CURSOR_VERT_ORG 2
#define SMALL_CURSOR_HORZ_ORG 2
#define SMALL_CURSOR_VERT_ORG 1

static const WORD lc_copyOverlayBits[11] = {
    0x0000,
    0xC07F,
    0xC07F,
    0xC07B,
    0xC07B,
    0xC060,
    0xC07B,
    0xC07B,
    0xC07F,
    0xC07F,
    0x0000,
};
static const WORD lc_swapOverlayBits[11] = {
    0x0000,
    0xC07F,
    0x4071,
    0x406E,
    0x405C,
    0xC07F,
    0x4047,
    0xC04E,
    0xC051,
    0xC07F,
    0x0000,
};


/*
 * Macros
 */
#define GET_LCDDATA(hwnd) (LCDDATA *) GetWindowLong((hwnd), GWL_LCDDATA)
#define CHAR_DELTA(style) \
    ((style) & LCS_LARGE ? LARGE_DELTA : SMALL_DELTA)
#define LCD_HEIGHT(style) \
    ((style) & LCS_LARGE ? LARGE_LCD_HEIGHT : SMALL_LCD_HEIGHT)

/*
 * Global procedures
 */
extern void LcdCtrl_NumInit(HWND lcdCtrl, const NUMLCDINIT *init);
extern int LcdCtrl_PageSize8(UINT ctrlID, int dir, int value);
extern int LcdCtrl_PageSize12(UINT ctrlID, int dir, int value);
extern int LcdCtrl_PageSizeSnap5(UINT ctrlID, int dir, int value);
extern BOOL LcdCtrl_Register(void);
extern void LcdCtrl_SpecialInit(HWND lcdCtrl, const SPECIALLCDINIT *init);
extern void LcdCtrl_TextInit(HWND lcdCtrl, int maxLen);

/*
 * Unit procedures
 */
static void lc_AddEntryHoldChar(HWND lcdCtrl, _TUCHAR ch);
static void lc_ApplyEntryHold(_TUCHAR *applied, _TUCHAR *data, int maxLen);
static void lc_ClearEntryHold(HWND lcdCtrl);
static void lc_CreateCaret(HWND lcdCtrl, BOOL insertCaret);
static void lc_DrawLcd(HDC dC, HWND lcdCtrl, int style, RECT *lcdRect, HBRUSH bgBrush, COLORREF textColor);
static int lc_GetControlValue(_TUCHAR *data, size_t maxLen);
static void lc_GetLcdSize(HWND lcdCtrl, int *width, int *height);
static int lc_GetPageSize(int min, int max);
static void lc_OnBeginDrag(HWND lcdCtrl, int x, int y);
static void lc_OnCancelMode(HWND lcdCtrl);
static void lc_OnChar(HWND lcdCtrl, _TUCHAR ch, int repeat);
static BOOL lc_OnCreate(HWND lcdCtrl, LPCREATESTRUCT createStruct);
static void lc_OnDestroy(HWND lcdCtrl);
static void lc_OnEnable(HWND lcdCtrl, BOOL enable);
static _TUCHAR lc_OnGetChar(HWND lcdCtrl, int pos);
static int lc_OnGetCursorPos(HWND lcdCtrl);
static UINT lc_OnGetDlgCode(HWND lcdCtrl, LPMSG msg);
static void lc_OnGetLcdDlgRect(HWND lcdCtrl, RECT *rect);
static int lc_OnGetLength(HWND lcdCtrl);
static int lc_OnGetOffset(HWND lcdCtrl);
static DWORD lc_OnGetRange(HWND lcdCtrl);
static int lc_OnGetRangeMax(HWND lcdCtrl);
static int lc_OnGetRangeMin(HWND lcdCtrl);
static void lc_OnGetText(HWND lcdCtrl, _TUCHAR *text);
static int lc_OnGetValue(HWND lcdCtrl);
static void lc_OnHighlight(HWND lcdCtrl, BOOL highlight);
static void lc_OnHScroll(HWND lcdCtrl, HWND scrollBar, UINT code, int pos);
static void lc_OnKey(HWND lcdCtrl, UINT vk, BOOL down, int repeat, UINT flags);
static void lc_OnKillFocus(HWND lcdCtrl, HWND otherWnd);
static void lc_OnLButtonDown(HWND lcdCtrl, BOOL dblClick, int x, int y, UINT keyFlags);
static void lc_OnLButtonUp(HWND lcdCtrl, int x, int y, UINT keyFlags);
static void lc_OnMouseMove(HWND lcdCtrl, int x, int y, UINT keyFlags);
static void lc_OnMouseWheel(HWND lcdCtrl, int delta, int x, int y, UINT keyFlags);
static void lc_OnPaint(HWND lcdCtrl);
static void lc_OnSetCursorPos(HWND lcdCtrl, int cursorPos);
static void lc_OnSetFocus(HWND lcdCtrl, HWND otherWnd);
static void lc_OnSetLength(HWND lcdCtrl, int maxLen);
static void lc_OnSetOffset(HWND lcdCtrl, int offset);
static int lc_OnSetRange(HWND lcdCtrl, int min, int max);
static void lc_OnSetStrings(HWND lcdCtrl, _TUCHAR *strings);
static void lc_OnSetText(HWND lcdCtrl, _TUCHAR *text);
static void lc_OnSetValue(HWND lcdCtrl, int value);
static void lc_OnSize(HWND lcdCtrl, UINT state, int cx, int cy);
static void lc_OnVScroll(HWND lcdCtrl, HWND scrollBar, UINT code, int pos);
static void lc_RemoveEntryHoldChar(HWND lcdCtrl);
static void lc_SetDragCursor(DRAGOPERATION dragOperation);
static void lc_SetLcdRect(HWND lcdCtrl);
static void lc_SetScrollingLcd(HWND lcdCtrl, int value);
static void lc_SetScrollingLcdText(HWND lcdCtrl, int value);
static LRESULT CALLBACK lc_WndProc(HWND lcdCtrl, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK lc_WndProcScrollBar(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static WNDPROC lc_origSBWndProc;
static HIMAGELIST lc_imageList; /* for dragging */
static HCURSOR lc_dragCursor;
static DRAGOPERATION lc_dragOperation;
static HWND lc_entryHoldCtrl;
#define ENTRYHOLD_MAX 40
static _TUCHAR lc_entryHold[ENTRYHOLD_MAX];
static size_t lc_entryHoldCnt;


/*
 * Procedure definitions
 */

void LcdCtrl_Deinit(void)
{
    if (lc_dragCursor) {
        DestroyIcon(lc_dragCursor);
    }
}

void LcdCtrl_NumInit(HWND lcdCtrl, const NUMLCDINIT *init)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lc_OnSetLength(lcdCtrl, init->maxLen);
    lcdData->offset = init->offset;
    lcdData->min = init->min;
    lcdData->max = init->max;
    lcdData->LcdPageFunc = init->LcdPageFunc;
    ScrollBar_SetRange(lcdData->scrollBar, init->min, init->max);
}

/*
 * PageSize8() - Returns the page size of 8.
 */
int LcdCtrl_PageSize8(UINT ctrlID, int dir, int value)
{
    return 8;
}

/*
 * PageSize12() - Returns the page size of 12.
 */
int LcdCtrl_PageSize12(UINT ctrlID, int dir, int value)
{
    return 12;
}

/*
 * PageSizeSnap5() - Used for scroll bar page size.  Snaps to multiples of 5.
 */
int LcdCtrl_PageSizeSnap5(UINT ctrlID, int dir, int value)
{
    int size = 1;

    while ((value + dir * size) % 5 != 0) {
        size++;
    }
    return size;
}

/*
 * Register() - Registers the window class for the window.  Returns true on
 *              success, false on failure.
 */
BOOL LcdCtrl_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) lc_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = LCD_EXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = NULL;
    classInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = LcdCtrl_className;
    classInfo.hIconSm = NULL;

    if (RegisterClassEx(&classInfo) == 0) {
        MsgBox_LastErrorF(NULL, _T("Error registering LCD control"));
        return FALSE;
    }

    return TRUE;
}

void LcdCtrl_SpecialInit(HWND lcdCtrl, const SPECIALLCDINIT *init)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lcdData->strings = init->strings;
    lcdData->LcdPageFunc = init->LcdPageFunc;
    lc_OnSetLength(lcdCtrl, init->maxLen);
    lc_OnSetRange(lcdCtrl, init->min, init->max);
}

void LcdCtrl_TextInit(HWND lcdCtrl, int maxLen)
{
    lc_OnSetLength(lcdCtrl, maxLen);
}

/*
 * AddEntryHoldChar() - Adds a character to the entry hold and displays the
 *                      cursor.
 */
void lc_AddEntryHoldChar(HWND lcdCtrl, _TUCHAR ch)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    int i = lc_entryHoldCnt;

    if (i < ENTRYHOLD_MAX - 2) {
        lc_entryHoldCtrl = lcdCtrl;
        lc_entryHold[i++] = ch;
        lc_entryHold[i] = '\0';
        InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
        lc_entryHoldCnt = i;
    }
}

/*
 * ApplyEntryHold() - Overlays entry hold text over LCD data.
 */
void lc_ApplyEntryHold(_TUCHAR *applied, _TUCHAR *data, int maxLen)
{
    int i;
    _TUCHAR *s = lc_entryHold;

    for (i = 0; i < maxLen; i++) {
        if (*s == '\0') {
            s = data;
        }
        applied[i] = *s++;
    }
}

/*
 * ClearEntryHold() - Clears the entry hold, deletes the caret and removes
 *                    the selection color from the display.
 */
void lc_ClearEntryHold(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lc_entryHoldCtrl = NULL;
    lc_entryHold[0] = '\0';
    lc_entryHoldCnt = 0;
    DestroyCaret();
    InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
}

/*
 * CreateCaret() - Creates and displays the text caret.
 */
void lc_CreateCaret(HWND lcdCtrl, BOOL insertCaret)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);

    if (style & LCS_LARGE) {
        CreateCaret(lcdCtrl, NULL
                , insertCaret ? 2 : LARGE_CURSOR_WIDTH, LARGE_CURSOR_HEIGHT);
    } else {
        CreateCaret(lcdCtrl, NULL
                , insertCaret ? 1 : SMALL_CURSOR_WIDTH, SMALL_CURSOR_HEIGHT);
    }
    lc_OnSetCursorPos(lcdCtrl, lcdData->cursorPos);
    ShowCaret(lcdCtrl);
}

/*
 * DrawLcd() - Draws the lcd part of the control.  Used in OnPaint and
 *             OnLButtonDown() to create the drag image.
 */
void lc_DrawLcd(HDC dC, HWND lcdCtrl, int style, RECT *lcdRect, HBRUSH bgBrush
        , COLORREF textColor)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    _TUCHAR *data = lcdData->data;
    int maxLen = lcdData->maxLen;
    int i = 0, j;
    int lcdRight;
    int entryHoldSpaceSkip;
    BOOL entryHold = FALSE;
    //int maxLenMinus2 = lcdData->maxLen - 2;

    SelectBrush(dC, bgBrush);
    SelectPen(dC, Prog_wndTextPen);
    Rectangle(dC, PASS_RECT_FIELDS(*lcdRect));

    /*
     * If the entry hold is active, and it is being used for string matching,
     * it has to be drawn so it matches the underlying data, so first the
     * data needs to be scanned to see if it contains the first character
     * of the entry hold, then whatever position it was found is the number
     * of spaces that need to be drawn before drawing the entry hold string.
     */
    if (lcdCtrl == lc_entryHoldCtrl) {
        entryHold = TRUE;
    }
    if (entryHold) {
        for (i = entryHoldSpaceSkip = 0; i < maxLen; i++) {
            if (data[i] == ' ') {
                entryHoldSpaceSkip++;
            } else if (data[i] == lc_entryHold[0]) {
                break;
            }
        }
    }
    if (style & LCS_LARGE) {
        HBRUSH fgBrush;

        lcdRight = lcdRect->right - LARGE_HORZ_PADDING;
#if NEVER_APPLIES_TO_TX81Z_PROGRAM
        if (entryHold) {
            /*
             * Draw the inverted entry hold string.
             */
            RECT bgRect = { 0, 0, LARGE_CURSOR_WIDTH, LARGE_CURSOR_HEIGHT };
            int entryHoldMax = maxLen < ENTRYHOLD_MAX ? maxLen : ENTRYHOLD_MAX;

            fgBrush = Prog_wndBrush;
            for (i = j = 0; lc_entryHold[j] && i < entryHoldMax; i++) {
                FillRect(dC, &bgRect, Prog_wndTextBrush);
                OffsetRect(&bgRect, LARGE_DELTA, 0);
                if (i >= entryHoldSpaceSkip) {
                    MiniFont_DrawChar2X(dC
                            , lcdRight - ((maxLen - i) * LARGE_DELTA)
                            , LARGE_VERT_PADDING, lc_entryHold[j], fgBrush)
                    j++;
                }
            }
            /*
             * Set the cursor to the position after the entry hold string.
             */
            lcdData->cursorPos = i;
            lc_CreateCaret(lcdCtrl, TRUE);
        }
#endif
        fgBrush = CreateSolidBrush(textColor);
        for ( ; i < maxLen; i++) {
            MiniFont_DrawChar2X(dC, lcdRight - ((maxLen - i) * LARGE_DELTA)
                    , LARGE_VERT_PADDING, data[i], fgBrush);
        }
        DeleteBrush(fgBrush);
    } else {
        lcdRight = lcdRect->right - SMALL_HORZ_PADDING;
        if (entryHold) {
            /*
             * Draw the inverted entry hold string.
             */
            RECT bgRect = *lcdRect;
            int entryHoldMax = maxLen < ENTRYHOLD_MAX ? maxLen : ENTRYHOLD_MAX;

            bgRect.left++;
            bgRect.right = bgRect.left + SMALL_CURSOR_WIDTH;

            for (i = j = 0; lc_entryHold[j] && i < entryHoldMax; i++) {
                if (i == 1) {
                    bgRect.left++;
                }
                FillRect(dC, &bgRect, Prog_wndTextBrush);
                OffsetRect(&bgRect, SMALL_DELTA, 0);
                if (i >= entryHoldSpaceSkip) {
                    MiniFont_DrawChar(dC
                            , lcdRight - ((maxLen - i) * SMALL_DELTA)
                            , SMALL_VERT_PADDING, lc_entryHold[j]
                            , Prog_selTextColor);
                    j++;
                }
            }
            /*
             * Set the cursor to the position after the entry hold string.
             */
            lcdData->cursorPos = i;
            lc_CreateCaret(lcdCtrl, TRUE);
        }
        for ( ; i < maxLen; i++) {
            MiniFont_DrawChar(dC
                    , lcdRight - ((maxLen - i) * SMALL_DELTA)
                    , SMALL_VERT_PADDING, data[i], textColor);
        }
    }
}

/*
 * GetControlValue() - Calculates the value of a numeric LCD.
 */
int lc_GetControlValue(_TUCHAR *data, size_t maxLen)
{
    int value = 0;
    size_t i;
    BOOL negative = FALSE;

    for (i = 0; i < maxLen; i++) {
        if (isdigit(data[i])) {
            value *= 10;
            value += data[i] - '0';
        } else if (data[i] == '-') {
            negative = TRUE;
        }
    }
    return negative ? -value : value;
}

/*
 * GetLcdSize() - Returns the width and height of the LCD part of the control.
 */
void lc_GetLcdSize(HWND lcdCtrl, int *width, int *height)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int charDelta = CHAR_DELTA(style);
    int maxLen = lcdData->maxLen;
    RECT clientRect;
    int w, h;

    GetClientRect(lcdCtrl, &clientRect);
    w = clientRect.right;
    h = clientRect.bottom;
    if (style & LCS_BOTTOM_SB) {
        h = LCD_HEIGHT(style);
    } else if (style & LCS_LEFT_SB || style & LCS_RIGHT_SB) {
        w = charDelta * maxLen + ((style & LCS_LARGE ? LARGE_HORZ_PADDING
                : SMALL_HORZ_PADDING) << 1) + 1;
    }
    *width = w;
    *height = h;
}

/*
 * GetPageSize() - Returns the size of the scrolling page based on the range.
 */
int lc_GetPageSize(int min, int max)
{
    double scrollRange = max - min + 1;

    return scrollRange < 10 ? 3 : (int) sqrt(scrollRange);
}

/*
 * OnBeginDrag()
 */
void lc_OnBeginDrag(HWND lcdCtrl, int x, int y)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int lcdX, lcdY, lcdW, lcdH;
    HBITMAP dragBmp, maskBmp;
    HDC dC = GetDC(lcdCtrl);
    HDC memDC = CreateCompatibleDC(dC);
    RECT lcdRect = lcdData->lcdRect;
    POINT pt = { x, y };

    ClientToScreen(lcdCtrl, &pt);
    /*
     * Get the lcd size.
     */
    lcdW = RECT_W(lcdRect);
    lcdH = RECT_H(lcdRect);
    lcdX = lcdRect.left;
    lcdY = lcdRect.top;
    if (style & LCS_LEFT_SB) {
        /*
         * Adjust x for hot spot.
         */
        x -= lcdX;
        lcdRect.left -= lcdX;
        lcdRect.right -= lcdX;
    }
    /*
     * Create a bitmap of the lcd.
     */
    dragBmp = CreateCompatibleBitmap(dC, lcdW, lcdH);
    SelectBitmap(memDC, dragBmp);
    lc_DrawLcd(memDC, lcdCtrl, style, &lcdRect, Prog_wndBrush
            , Prog_wndTextColor);
    /*
     * Create a monochrome mask image.
     */
    maskBmp = CreateBitmap(lcdW, lcdH, 1, 1, NULL);
    SelectBitmap(memDC, maskBmp);
    lc_DrawLcd(memDC, lcdCtrl, style, &lcdRect, Prog_wndBrush
            , Prog_wndTextColor);
    /*
     * Create an image list.
     */
    lc_imageList = ImageList_Create(lcdW, lcdH, ILC_MASK, 1, 1);
    /*
     * Add the bitmaps to the list.
     */
    ImageList_Add(lc_imageList, dragBmp, NULL);
    /*
     * Delete the bitmaps and DC's.
     */
    DeleteBitmap(dragBmp);
    DeleteBitmap(maskBmp);
    DeleteDC(memDC);
    ReleaseDC(lcdCtrl, dC);
    /*
     * Begin dragging the image.
     */
    ImageList_BeginDrag(lc_imageList, 0, x, y);
    ImageList_DragEnter(HWND_DESKTOP, pt.x, pt.y);
    SetCapture(lcdCtrl);
    DestroyCaret();
    lc_SetDragCursor(lc_dragOperation);
}

/*
 * OnCancelMode()
 */
void lc_OnCancelMode(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    ScrollBar_Enable(lcdData->scrollBar, ESB_DISABLE_BOTH);
    InvalidateRect(lcdCtrl, NULL, FALSE);
}

/*
 * OnChar()
 */
void lc_OnChar(HWND lcdCtrl, _TUCHAR ch, int repeat)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    UINT ctrlID = GetDlgCtrlID(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    HWND parentWnd = GetParent(lcdCtrl);
    const _TUCHAR *strings = lcdData->strings;
    _TUCHAR *data = lcdData->data;
    int maxLen = lcdData->maxLen;
    LCDFLAGS flags = lcdData->flags;
    int i;

#ifdef _DEBUG
    if (maxLen == 0) {
        MessageBox(Prog_mainWnd
                , _T("A size must be set with the LM_SETLENGTH message\r\n")
                  _T("before using an LCD control.")
                , _T("Design-Time Error")
                , MB_ICONERROR | MB_OK);
        return;
    }
#endif

    /*
     * If the control is a special LCD.
     */
    if (strings) {
        /*
         * If the user backspaces while the entry hold is active, adjust the
         * entry hold or delete it.
         */
        if (ch == '\b') {
            if (lc_entryHoldCnt) {
                lc_RemoveEntryHoldChar(lcdCtrl);
            }
            return;
        } else if (isprint(ch)) {
            /*
             * The plan is this: the user should be able to type in the
             * complete entry, or he can step through the possibilities by
             * hitting the first character multiple times.  So, what needs to
             * happen is, the first character typed is searched for in the
             * string list, starting from the position of the current LCD
             * value, and if a string whose first character matches the typed
             * character is found, the character is added to the entry hold and
             * the LCD is changed to the position where it was found.  Then
             * when the user types the next character, it searches the string
             * list, starting from the current position again, the new one,
             * this time for the two characters, the one in the entry hold and
             * the typed one, and if they are found, the new character is added
             * to the entry hold.  Etc.  That's the pattern for the user typing
             * a complete entry correctly.  If the user doesn't type a complete
             * entry correctly, then either he made a mistake, or he is
             * stepping through the list.
             *
             * Workaround: if the new key is the same as the previous key,
             * automatically do key stepping and forego the search.  Say
             * there's a list of numbers, (1, 2, 3...10, 11, 12, 13) and the
             * current selection is on 2, if the user types a 1, it will go to
             * 10, which is good, but if he keeps typing 1's it will go from
             * 10 to 11 to 12 back to 11 again, because he typed 2 1's, so
             * the thing will get stuck in a loop of 11 and 12.
             *
             * Another possibility is to search from the beginning every time
             * with the complete search string, but then single key stepping
             * wouldn't be possible.
             */
            int i, j, k;
            int min = lcdData->min;
            int max = lcdData->max;
            const _TUCHAR *str;
            int value = lc_OnGetValue(lcdCtrl);
            BOOL keyStepSearch = FALSE;
            _TUCHAR searchStr[ENTRYHOLD_MAX];
            int searchStrLen;
            int searchStart;
            _TUCHAR spacelessEntry[ENTRYHOLD_MAX];
            int spacelessMaxLen;

            ch = toupper(ch);
            /*
             * Create the search string: the entry hold + the newly typed key.
             */
            if (lc_entryHoldCnt < ENTRYHOLD_MAX - 2) {
                _tcscpy(searchStr, lc_entryHold);
                searchStr[lc_entryHoldCnt] = ch;
                searchStrLen = lc_entryHoldCnt + 1;
                searchStr[searchStrLen] = '\0';
            } else {
                MessageBeep(-1);
                return;
            }

            /*
             * Search for the entry hold string in the strings list.  If the
             * control just gain focus, start the search from the beginning.
             */
            if (lcdData->flags & LF_JUSTGOTFOCUS) {
                if (min == value) {
                    goto Nevermind;
                } else {
                    searchStart = min;
                }
            } else {
Nevermind:
                searchStart = value + 1;
            }
KeyStepSearch:
            for (i = searchStart; ; i++) {
                /*
                 * Wrap around the end of the array.
                 */
                if (i > max) {
                    i = min;
                    if (searchStart == min) {
                        goto SearchStopCheck;
                    }
                }
                /*
                 * Create an entry value that contains no spaces.
                 */
                str = &strings[(i - min) * maxLen];
                for (k = spacelessMaxLen = 0; k < maxLen; k++) {
                    if (str[k] != ' ') {
                        spacelessEntry[spacelessMaxLen++] = str[k];
                    }
                }
                spacelessEntry[spacelessMaxLen] = '\0';
                /*
                 * The the current LCD position is reached, then the string was
                 * not found.
                 */
                if (i == searchStart - 1) {
SearchStopCheck:
                    /*
                     * If the letter matches the string that the control is
                     * currently on consider the search successful afterall.
                     */
                    if (ch == spacelessEntry[lc_entryHoldCnt]) {
                        searchStr[lc_entryHoldCnt] = ch;
                        searchStrLen = lc_entryHoldCnt + 1;
                        searchStr[searchStrLen] = '\0';
                    /*
                     * If the letter matches the previous letter, switch to
                     * key step searching.
                     */
                    } else if (ch == searchStr[lc_entryHoldCnt - 1]) {
                        searchStr[lc_entryHoldCnt] = '\0';
                        searchStrLen--;
                        keyStepSearch = TRUE;
                        goto KeyStepSearch;
                    } else {
                        break;
                    }
                }
                /*
                 * If the search string matches the spaceless entry.
                 */
                if (StriEqN(searchStr, spacelessEntry, searchStrLen)) {
                    /*
                     * If the search also reached the end of the trial
                     * string, then the user typed the complete entry.
                     */
                    BOOL complete = (searchStrLen == spacelessMaxLen);

                    if (!complete) {
                        /*
                         * If the current LCD value is the only possible match
                         * for the entry hold, then consider the entry complete.
                         */
                        for (j = i + 1; ; j++) {
                            if (j > max) {
                                j = min;
                            }
                            /*
                             * No alternate match found.
                             */
                            if (j == i) {
                                complete = TRUE;
                                break;
                            }
                            str = &strings[(j - min) * maxLen];
                            for (k = spacelessMaxLen = 0; k < maxLen ; k++) {
                                if (str[k] != ' ') {
                                    spacelessEntry[spacelessMaxLen++] = str[k];
                                }
                            }
                            spacelessEntry[spacelessMaxLen] = '\0';
                            if (StriEqN(searchStr, spacelessEntry
                                        , searchStrLen))
                            {
                                break;
                            }
                        }
                    }
                    if (complete) {
                        /*
                         * Reset the entry hold.
                         */
                        lc_ClearEntryHold(lcdCtrl);
                    }
                    /*
                     * Set the new value and send the change
                     * notification.
                     */
                    DestroyCaret();
                    lc_SetScrollingLcd(lcdCtrl, i);
                    SendNotification(parentWnd, ctrlID, lcdCtrl
                            , LCN_SELCHANGE);
                    /*
                     * Return if the entry hold was cleared.
                     */
                    if (complete) {
                        goto Return;
                    }
                    /*
                     * If this is not a repeat character search, add
                     * the newly typed character.
                     */
                    if (!keyStepSearch) {
                        lc_AddEntryHoldChar(lcdCtrl, ch);
                    }
                    goto Return;
                }
            }
            /*
             * The match failed, so remove the last character from the search
             * string and if the typed key is the same as the previous key,
             * repeat the search that occurred on the previous keypress.
             */
            if (!keyStepSearch) {
                searchStr[lc_entryHoldCnt] = '\0';
                if (toupper(ch) == toupper(searchStr[lc_entryHoldCnt - 1])) {
                    keyStepSearch = TRUE;
                    goto KeyStepSearch;
                }
            }
            /*
             * Utter failure - the user made a typing mistake.
             */
            MessageBeep(-1);
        }
    /*
     * Else if it is numeric.
     */
    } else if (style & LCS_NUMERIC) {
        int min = lcdData->min;
        int max = lcdData->max;
        int offset = lcdData->offset;
        int apparentValue;
        int value;
        int oldValue;

        if (lc_entryHold[0]) {
            _TUCHAR applied[80];

            lc_ApplyEntryHold(applied, data, maxLen);
            apparentValue = lc_GetControlValue(applied, maxLen);
        } else {
            apparentValue = lc_GetControlValue(data, maxLen);
        }
        value = apparentValue - offset;
        oldValue = value;
        /*
         * If the user typed a backspace, delete the ones digit.
         */
        if (ch == '\b') {
            value /= 10;
        /*
         * If the user typed a negative sign.
         */
        } else if (ch == '-') {
            /*
             * If the LCD value is positive, try to negate it without going
             * out of range.
             */
            if (apparentValue > 0) {
                if (-apparentValue - offset >= min) {
                    value = -apparentValue - offset;
                } else {
                    MessageBeep(-1);
                    return;
                }
            /*
             * If the value is zero, negation can't be applied, so add the
             * minus to the entry hold and wait for additional characters
             * before taking any action.
             */
            } else if (apparentValue == 0) {
                lc_entryHold[0] = '\0';
                lc_AddEntryHoldChar(lcdCtrl, '-');
                goto Return;
            /*
             * Else the value is already negative, just ignore the key.
             */
            } else {
                goto Return;
            }
        /*
         * If the user typed a positive sign.
         */
        } else if (ch == '+') {
            /*
             * If the LCD contains a value of zero, then plus doesn't really
             * apply, so add the character to the entry hold.
             */
            if (apparentValue == 0) {
                lc_entryHold[0] = '\0';
                lc_AddEntryHoldChar(lcdCtrl, '+');
                goto Return;
            /*
             * If the current value is negative, negate the negation.
             */
            } else if (apparentValue < 0 && -apparentValue - offset <= max) {
                value = -apparentValue - offset;
            /*
             * Else the value is already positive, just ignore the key.
             */
            } else {
                goto Return;
            }
        /*
         * If the user typed a digit.
         */
        } else if (isdigit(ch)) {
            int digit = ch - '0';
            int newValue;
            int newApparentValue = apparentValue;
            int negativeMultiplier = 1;
            
            /*
             * If the user just clicks on the LCD and types a digit, then start
             * data entry with a clean slate by replacing all the text in the
             * LCD with the digit.
             */
            if (flags & LF_JUSTGOTFOCUS) {
                /*
                 * Replace the contents of the LCD with the new digit.
                 */
                goto ReplaceContents;
            /*
             * Else the user has done some previous typing, so add his current
             * typing as a continuation from that.
             */
            } else {
                /*
                 * Try appending a digit to the end of the number.
                 */
                newApparentValue *= 10;
                if (apparentValue < 0
                        || (apparentValue == 0 && lc_entryHold[0] == '-'))
                {
                    newApparentValue -= digit;
                } else {
                    newApparentValue += digit;
                }
                newValue = newApparentValue - offset;

                /*
                 * If the number is out of range.
                 */
                if (newValue < min || newValue > max) {
                    /*
                     * Try removing the leading digit of the new number.
                     */
                    int subValue;

                    for (i = 0; !isdigit(data[i]); i++) {
                        if (data[i] == '-') {
                            negativeMultiplier = -1;
                        } 
                    }
                    subValue = (data[i] - '0') * negativeMultiplier;
                    for ( ; i < maxLen; i++) {
                        subValue *= 10;
                    }
                    newApparentValue -= subValue;
                    newValue = newApparentValue - offset;
                    /*
                     * If the number is still out of range.
                     */
                    if (newValue < min || newValue > max) {
#ifdef REPLACE_DIGITS
                        /*
                         * Try replacing the last digit instead.
                         */
                        newApparentValue = apparentValue;
                        if (newApparentValue >= 0) {
                            newApparentValue = newApparentValue
                                - (newApparentValue % 10) + digit;
                        } else {
                            newApparentValue = newApparentValue
                                - (newApparentValue % 10) - digit;
                        }
                        newValue = newApparentValue - offset;
                        /*
                         * If the number is still out of range.
                         */
                        if (newValue < min || newValue > max) {
#endif
                            /*
                             * Replace the whole damn thing with the digit.
                             */
ReplaceContents:
                            newValue = digit * negativeMultiplier - offset;
                            /*
                             * If the number is STILL out of the fucking
                             * range, then sigh and give up in resignation.
                             */
                            if (newValue < min || newValue > max) {
                                /*
                                 * If the digit is a zero, then put the zero
                                 * in the entry hold.  The RR range is 1 to 15,
                                 * and without this feature, typing 05 while
                                 * the control is set from 10..15 will never
                                 * get to 5.  It will change from 10 then to 15.
                                 */
                                if (newValue == 0) {
                                    lc_entryHold[0] = '\0';
                                    lc_AddEntryHoldChar(lcdCtrl, '0');
                                    newValue = min;
                                    goto DontClearEntryHold;
                                } else {
                                    /*
                                     * Now give up.
                                     */
                                    MessageBeep(-1);
                                    goto Return;
                                }
                            }
#ifdef REPLACE_DIGITS
                        }
#endif
                    }
                }
            }
            lc_ClearEntryHold(lcdCtrl);
DontClearEntryHold:
            value = newValue;
        } else {
            MessageBeep(-1);
            return; /* don't clear the LF_JUSTGOTFOCUS flag yet */
        }
        if (value != oldValue) {
            lc_SetScrollingLcd(lcdCtrl, value);
            SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_SELCHANGE);
        }
    /*
     * Else the control is a text LCD.
     */
    } else {
        int cursorPos = lcdData->cursorPos;

        if (ch == '\b') {
            if (cursorPos > 0) {
                cursorPos--;
                ch = ' ';
                for (i = maxLen - 1; i >= cursorPos; i--) {
                    int t = data[i];
                    data[i] = ch;
                    lcdData->cursorPos = i;
                    SendNotification(parentWnd, ctrlID, lcdCtrl
                            , LCN_EDITUPDATE);
                    ch = t;
                }
                lcdData->cursorPos = cursorPos;
            }
        } else if (isprint(ch)) {
            if (flags & LF_INSERTMODE) {
                for (i = maxLen - 1; i > cursorPos; i--) {
                    data[i] = data[i - 1];
                    SendNotification(parentWnd, ctrlID, lcdCtrl
                            , LCN_EDITUPDATE);
                }
            }
            data[cursorPos] = ch;
            SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_EDITUPDATE);
            if (cursorPos < maxLen - 1) {
                cursorPos++;
            }
        } else {
            goto Return;
        }
        RedrawWindowNow(lcdCtrl);
        lc_OnSetCursorPos(lcdCtrl, cursorPos);
        SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_EDITCHANGE);
    }
Return:
    lcdData->flags &= ~LF_JUSTGOTFOCUS;
}

/*
 * OnCreate()
 */
BOOL lc_OnCreate(HWND lcdCtrl, LPCREATESTRUCT createStruct)
{
    LCDDATA *lcdData = calloc(sizeof *lcdData, 1);

    if (!lcdData) {
        Error_OnError(E_MALLOC_ERROR);
        return FALSE;
    }
    SetWindowLong(lcdCtrl, GWL_LCDDATA, (long) lcdData);

    return TRUE;
}

/*
 * OnDestroy()
 */
void lc_OnDestroy(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    if (lcdData) {
        if (lcdData->data) {
            free(lcdData->data);
        }
        free(lcdData);
    }
}

/*
 * OnEnable()
 */
void lc_OnEnable(HWND lcdCtrl, BOOL enable)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    if (enable) {
        ScrollBar_Enable(lcdData->scrollBar, ESB_ENABLE_BOTH);
    } else {
        ScrollBar_Enable(lcdData->scrollBar, ESB_DISABLE_BOTH);
    }
    InvalidateRect(lcdCtrl, NULL, FALSE);
}

/*
 * OnGetChar()
 */
_TUCHAR lc_OnGetChar(HWND lcdCtrl, int pos)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    assert(pos >= 0 && pos <= lcdData->maxLen);
    return lcdData->data[pos];
}

/*
 * OnGetCursorPos()
 */
int lc_OnGetCursorPos(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    return lcdData->cursorPos;
}

/*
 * OnGetDlgCode()
 */
UINT lc_OnGetDlgCode(HWND lcdCtrl, LPMSG msg)
{
    return DLGC_WANTCHARS | DLGC_WANTARROWS;
}

/*
 * OnGetLcdDlgRect()
 */
void lc_OnGetLcdDlgRect(HWND lcdCtrl, RECT *rect)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    HWND parentWnd = GetParent(lcdCtrl);

    *rect = lcdData->lcdRect;
    MapWindowPoints(lcdCtrl, parentWnd, (POINT *) rect, 2);
}

/*
 * OnGetLength()
 */
int lc_OnGetLength(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    return lcdData->maxLen;
}

/*
 * OnGetOffset()
 */
int lc_OnGetOffset(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    return lcdData->offset;
}

/*
 * OnGetRange()
 */
DWORD lc_OnGetRange(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    return MAKELRESULT(lcdData->min, lcdData->max);
}

int lc_OnGetRangeMax(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    return lcdData->max;
}

int lc_OnGetRangeMin(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    return lcdData->min;
}

/*
 * OnGetText()
 */
void lc_OnGetText(HWND lcdCtrl, _TUCHAR *text)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    _TUCHAR *data = lcdData->data;
    int maxLen = lcdData->maxLen;
    int i;

    _tcsncpy(text, data, maxLen);
    for (i = maxLen - 1; i >= 0; i--) {
        if (text[i] == ' ' || text[i] == '\0') {
            text[i] = '\0';
        } else {
            break;
        }
    }
}

/*
 * OnGetValue()
 */
int lc_OnGetValue(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);

#ifdef _DEBUG
    if (lcdData->maxLen == 0) {
        MessageBox(Prog_mainWnd
                , _T("A size must be set with the LM_SETLENGTH message\r\n")
                  _T("before using an LCD control.")
                , _T("Design-Time Error")
                , MB_ICONERROR | MB_OK);
        return 0;
    }
#endif

    if (style & LCS_ANY_SB) {
        HWND scrollBar = lcdData->scrollBar;
        int min = lcdData->min;
        int max = lcdData->max;

        if (style & LCS_BOTTOM_SB) {
            return ScrollBar_GetRPos(scrollBar, min, max);
        } else {
            return ScrollBar_GetPos(scrollBar);
        }
    } else if (style & LCS_NUMERIC) {
        return lc_GetControlValue(lcdData->data, lcdData->maxLen);
    }

    return 0;
}

/*
 * OnHighlight() - Turns off/on highlighting for the LCD display.
 */
void lc_OnHighlight(HWND lcdCtrl, BOOL highlight)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    if (highlight) {
        lcdData->flags |= LF_HIGHLIGHTED;
    } else {
        lcdData->flags &= ~LF_HIGHLIGHTED;
    }
    RedrawWindow(lcdCtrl, &lcdData->lcdRect, NULL
            , RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW);
}

/*
 * OnHScroll()
 */
void lc_OnHScroll(HWND lcdCtrl, HWND scrollBar, UINT code, int pos)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    UINT ctrlID = GetDlgCtrlID(lcdCtrl);
    HWND parentWnd = GetParent(lcdCtrl);
    int min = lcdData->min;
    int max = lcdData->max;
    int value = ScrollBar_GetPos(scrollBar);
    int oldValue = value;
    LCDPAGEFUNC LcdPageFunc = lcdData->LcdPageFunc;

    switch (code) {
        case SB_ENDSCROLL:
            break;
		case SB_LEFT:
            value = min;
            break;
		case SB_RIGHT:
            value = max;
            break;
		case SB_LINELEFT:
            if (value > min)
                value--;
            break;
		case SB_LINERIGHT:
            if (value < max)
                value++;
            break;
		case SB_PAGELEFT:
            if (LcdPageFunc) {
                value -= LcdPageFunc(ctrlID, -1, value);
            } else {
                value -= lc_GetPageSize(min, max);
            }
            if (value < min)
                value = min;
            break;
		case SB_PAGERIGHT:
            if (LcdPageFunc) {
                value += LcdPageFunc(ctrlID, +1, value);
            } else {
                value += lc_GetPageSize(min, max);
            }
            if (value > max)
                value = max;
            break;
		case SB_THUMBTRACK:
            value = pos;
            break;
    }
    if (lc_entryHoldCnt) {
        lc_ClearEntryHold(lcdCtrl);
    }
    if (value != oldValue) {
        lc_SetScrollingLcd(lcdCtrl, value);
        SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_SELCHANGE);
    }
}

/*
 * OnKey()
 */
void lc_OnKey(HWND lcdCtrl, UINT vk, BOOL down, int repeat, UINT flags)
{
    if (lc_dragOperation) {
        if (vk == VK_SHIFT) {
            lc_dragOperation = down ? DO_SWAP : DO_COPY;
            lc_SetDragCursor(lc_dragOperation);
        }
    } else if (down) {
        LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
        UINT ctrlID = GetDlgCtrlID(lcdCtrl);
        DWORD style = GetWindowStyle(lcdCtrl);
        HWND parentWnd = GetParent(lcdCtrl);
        LCDPAGEFUNC LcdPageFunc = lcdData->LcdPageFunc;
        _TUCHAR *data = lcdData->data;
        int maxLen = lcdData->maxLen;

        if (style & LCS_ANY_SB) {
            int min = lcdData->min;
            int max = lcdData->max;
            HWND scrollBar = lcdData->scrollBar;
            int value = (style & LCS_BOTTOM_SB)
                ? ScrollBar_GetRPos(scrollBar, min, max)
                : ScrollBar_GetPos(scrollBar);
            int oldValue = value;

            switch (vk) {
                case VK_HOME:
                    value = max;
                    break;
                case VK_END:
                    value = min;
                    break;
                case VK_PRIOR:
                    if (LcdPageFunc) {
                        value += LcdPageFunc(ctrlID, +1, value);
                    } else {
                        value += lc_GetPageSize(min, max);
                    }
                    if (value > max) {
                        value = max;
                    }
                    break;
                case VK_NEXT:
                    if (LcdPageFunc) {
                        value -= LcdPageFunc(ctrlID, -1, value);
                    } else {
                        value -= lc_GetPageSize(min, max);
                    }
                    if (value < min) {
                        value = min;
                    }
                    break;
                case VK_UP:
                    if (value < max) {
                        value++;
                    }
                    break;
                case VK_DOWN:
                    if (value > min) {
                        value--;
                    }
                    break;
                case VK_ESCAPE:
                case VK_RETURN:
                    break;
                default:
                    return;
            }
            if (lc_entryHoldCnt) {
                lc_ClearEntryHold(lcdCtrl);
            }
            if (value != oldValue) {
                lc_SetScrollingLcd(lcdCtrl, value);
                SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_SELCHANGE);
            }
        } else {
            int cursorPos = lcdData->cursorPos;
            int i;

            switch (vk) {
                case VK_LEFT:
                    if (cursorPos > 0) {
                        cursorPos--;
                    }
                    goto SetCursorPos;
                case VK_RIGHT:
                    if (cursorPos < maxLen - 1) {
                        cursorPos++;
                    }
                    goto SetCursorPos;
                case VK_HOME:
                    cursorPos = 0;
                    goto SetCursorPos;
                case VK_END:
                    for (i = maxLen - 1; i > 0; i--) {
                        if (data[i] && data[i] != ' ') {
                            break;
                        }
                    }
                    cursorPos = i + 1;
                    if (cursorPos > maxLen - 1) {
                        cursorPos = maxLen - 1;
                    }
SetCursorPos:
                    lc_OnSetCursorPos(lcdCtrl, cursorPos);
                    break;
                case VK_INSERT:
                    DestroyCaret();
                    lcdData->flags ^= LF_INSERTMODE;
                    lc_OnSetFocus(lcdCtrl, lcdCtrl);
                    break;
                case VK_DELETE: {
                    int ch = ' ';

                    for (i = maxLen - 1; i >= cursorPos; i--) {
                        int t = data[i];

                        data[i] = ch;
                        /*
                         * Set the cursor position to the character that was
                         * just changed so the notification handler can access
                         * it.
                         */
                        lcdData->cursorPos = i;
                        SendNotification(parentWnd, ctrlID, lcdCtrl
                                , LCN_EDITUPDATE);
                        ch = t;
                    }
                    lcdData->cursorPos = cursorPos;
                    SendNotification(parentWnd, ctrlID, lcdCtrl
                            , LCN_EDITCHANGE);
                    RedrawWindowNow(lcdCtrl);
                    break;
                }
            }
        }
    }
}

/*
 * OnKillFocus()
 */
void lc_OnKillFocus(HWND lcdCtrl, HWND otherWnd)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lcdData->flags &= ~(LF_SELECTED | LF_JUSTGOTFOCUS);
    lc_ClearEntryHold(lcdCtrl);
    RedrawWindowNow(lcdCtrl);
    SendNotification(GetParent(lcdCtrl), GetDlgCtrlID(lcdCtrl), lcdCtrl
            , LCN_KILLFOCUS);
}

/*
 * OnLButtonDown()
 */
void lc_OnLButtonDown(HWND lcdCtrl, BOOL dblClick, int x, int y, UINT keyFlags)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int charDelta = CHAR_DELTA(style);
    POINT pt = { x, y };

    if (!(style & LCS_ANY_SB)) {
        lcdData->cursorPos = (x - (style & LCS_LARGE ? LARGE_HORZ_PADDING
                    : SMALL_HORZ_PADDING)) / charDelta;
    }
    if (GetFocus() == lcdCtrl) {
        /*
         * Call OnSetFocus() directly to be sure that the cursor code runs.
         * SetFocus() won't call it if the control already has focus.
         */
        lc_OnSetFocus(lcdCtrl, lcdCtrl);
    } else {
        SetFocus(lcdCtrl);
    }
    Window_DoEvents(lcdCtrl, Prog_accels);
    if (style & LCS_ANY_SB) {
        ClientToScreen(lcdCtrl, &pt);
        if (DragDetect(lcdCtrl, pt)) {
            lc_dragOperation = (keyFlags & MK_SHIFT) == 0 ? DO_COPY : DO_SWAP;
            PostMessage(lcdCtrl, LCM_BEGINDRAG, x, y);
        }
    }
}

/*
 * OnLButtonUp()
 */
void lc_OnLButtonUp(HWND lcdCtrl, int x, int y, UINT keyFlags)
{
    if (lc_dragOperation) {
        POINT pt = { x, y };
        HWND targetWnd;
        HWND parentWnd = GetParent(lcdCtrl);

		ImageList_DragLeave(lcdCtrl);
		ImageList_EndDrag();
		ReleaseCapture();
        SetCursor(Prog_arrowCursor);
		ImageList_Destroy(lc_imageList);
		lc_imageList = NULL;
        /*
         * Find out if there's an lcd under the cursor.
         */
        MapWindowPoints(lcdCtrl, parentWnd, &pt, 1);
        targetWnd = ChildWindowFromPointEx(parentWnd, pt
                , CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
        if (targetWnd && targetWnd != lcdCtrl) {
            _TUCHAR className[30];

            GetClassName(targetWnd, className, 30);
            if (StriNotEq(className, LcdCtrl_className)) {
                targetWnd = GetParent(targetWnd);
                GetClassName(targetWnd, className, 30);
            }
            if (StriEq(className, LcdCtrl_className)) {
                LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
                LCDDATA *targetLcdData = GET_LCDDATA(targetWnd);
                UINT targetCtrlID = GetDlgCtrlID(targetWnd);
                HWND targetParentWnd = GetParent(targetWnd);
                int value = lc_OnGetValue(lcdCtrl);

                if (lc_dragOperation == DO_COPY) {
                    /*
                     * Set the target control's value.
                     */
                    LcdCtrl_SetValue(targetWnd
                            , value + lcdData->offset - targetLcdData->offset);
                } else {
                    /*
                     * Swap the source and target control values.
                     */
                    int t = LcdCtrl_GetValue(lcdCtrl);
                    UINT ctrlID = GetDlgCtrlID(lcdCtrl);

                    LcdCtrl_SetValue(lcdCtrl, LcdCtrl_GetValue(targetWnd));
                    SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_SELCHANGE);
                    LcdCtrl_SetValue(targetWnd, t);
                }
                /*
                 * Send a notification to the control's parent window on
                 * behalf of the control.
                 */
                SendNotification(targetParentWnd, targetCtrlID, targetWnd
                        , LCN_SELCHANGE);
                SetFocus(targetWnd);
            }
        }
        lc_dragOperation = DO_NONE;
    }
}

/*
 * OnMouseMove()
 */
void lc_OnMouseMove(HWND lcdCtrl, int x, int y, UINT keyFlags)
{
    if (lc_dragOperation) {
        POINT pt = { x, y };

        ClientToScreen(lcdCtrl, &pt);
        ImageList_DragMove(pt.x, pt.y);
        /*
         * TODO: Detect LCD's underneath and select them to indicate the item
         * can be dropped.
         */
    }
}

/*
 * OnMouseWheel()
 */
void lc_OnMouseWheel(HWND lcdCtrl, int delta, int x, int y, UINT keyFlags)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    
    if (style & LCS_BOTTOM_SB) {
        if (delta < 0) {
            lc_OnVScroll(lcdCtrl, lcdData->scrollBar, SB_LINEDOWN, 0);
        } else if (delta > 0) {
            lc_OnVScroll(lcdCtrl, lcdData->scrollBar, SB_LINEUP, 0);
        }
    } else if (style & LCS_ANY_SB) {
        if (delta < 0) {
            lc_OnHScroll(lcdCtrl, lcdData->scrollBar, SB_LINELEFT, 0);
        } else if (delta > 0) {
            lc_OnHScroll(lcdCtrl, lcdData->scrollBar, SB_LINERIGHT, 0);
        }
    }
}

/*
 * OnPaint()
 */
void lc_OnPaint(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(lcdCtrl, &paint);
    HBRUSH bgBrush;
    COLORREF textColor;
    const _TUCHAR *strings = lcdData->strings;
    BOOL enabled = IsWindowEnabled(lcdCtrl);

    /*
     * If it's a special LCD, refresh the value.
     */
    if (strings && enabled) {
        int value = lc_OnGetValue(lcdCtrl);
        int maxLen = lcdData->maxLen;

        _tcsncpy(lcdData->data, &strings[(value - lcdData->min) * maxLen]
                , maxLen);
    }
    if ((lcdData->flags & LF_SELECTED) && !lc_entryHold[0]) {
        bgBrush = Prog_wndTextBrush;
        textColor = Prog_wndColor;
    } else {
        if (!enabled) {
            bgBrush = Prog_dlgBrush;
            textColor = Prog_grayTextColor;
        } else if (lcdData->flags & LF_HIGHLIGHTED) {
            textColor = Prog_selTextColor;
            bgBrush = Prog_hiBrush;
        } else {
            textColor = Prog_wndTextColor;
            bgBrush = Prog_wndBrush;
        }
    }
    lc_DrawLcd(paintDC, lcdCtrl, style, &lcdData->lcdRect, bgBrush, textColor);
    EndPaint(lcdCtrl, &paint);
}

/*
 * OnSetCursorPos() - Sets the caret position with the LCD display.
 */
void lc_OnSetCursorPos(HWND lcdCtrl, int cursorPos)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int charDelta = CHAR_DELTA(style);
    int hPadding = (style & LCS_LARGE ? LARGE_HORZ_PADDING
            : SMALL_HORZ_PADDING);
    int vPadding = (style & LCS_LARGE ? LARGE_VERT_PADDING
            : SMALL_VERT_PADDING);
    int maxLen = lcdData->maxLen;
    int lcdRight = lcdData->lcdRect.right;
    int lcdTop = lcdData->lcdRect.top;

    SetCaretPos(lcdRight - hPadding - (maxLen - cursorPos) * charDelta - 1
            , lcdTop + vPadding - 1);
    lcdData->cursorPos = cursorPos;
}

/*
 * OnSetFocus()
 */
void lc_OnSetFocus(HWND lcdCtrl, HWND otherWnd)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);

    /*
     * Don't allow focus on read-only controls.
     */
    if (style & LCS_READONLY) {
        SetFocus(otherWnd);
        return;
    }
    /*
     * Clear the entry hold.
     */
    lc_entryHold[0] = '\0';
    if (style & LCS_ANY_SB) {
        /*
         * Change the background color of the lcd for scrollbared controls.
         */
        lcdData->flags |= LF_SELECTED | LF_JUSTGOTFOCUS;
        SetFocus(lcdData->scrollBar);
        RedrawWindowNow(lcdCtrl);
    } else {
        /*
         * Create caret.
         */
        lc_CreateCaret(lcdCtrl, (lcdData->flags & LF_INSERTMODE) != 0);
    }
    SendNotification(GetParent(lcdCtrl), GetDlgCtrlID(lcdCtrl), lcdCtrl
            , LCN_SETFOCUS);
}

/*
 * OnSetLength()
 */
void lc_OnSetLength(HWND lcdCtrl, int maxLen)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    _TUCHAR *data = lcdData->data;
    int i;

    if (maxLen != lcdData->maxLen) {
        if (data) {
            free(data);
            lcdData->data = NULL;
        }
        if (maxLen == 0) {
            lcdData->maxLen = 0;
            return;
        }
        data = malloc((maxLen + 1) * sizeof(_TUCHAR));
        if (!data) {
            Error_OnError(E_MALLOC_ERROR);
            return;
        }
    }
    for (i = 0; i < maxLen; i++) {
        data[i] = ' ';
    }
    data[maxLen] = '\0';
    if (style & LCS_NUMERIC) {
        data[maxLen - 1] = '0';
    }

    lcdData->data = data;
    lcdData->maxLen = maxLen;

    if (style & LCS_ANY_SB) {
        HWND scrollBar = lcdData->scrollBar;

        if (!scrollBar) {
            int sbStyle;
            WNDPROC origWndProc;

            if (style & LCS_BOTTOM_SB) {
                sbStyle = SBS_VERT;
            } else {
                sbStyle = SBS_HORZ;
            }
            scrollBar = CreateWindowEx(
                    0L
                    , _T("ScrollBar")
                    , (LPTSTR) NULL
                    , WS_CHILD | WS_VISIBLE | sbStyle
                    , 0, 0
                    , 10, 10
                    , lcdCtrl
                    , (HMENU) IDC_LCD_SCROLLBAR
                    , HINST_THISCOMPONENT
                    , (LPVOID) NULL
                );
            if (scrollBar) {
                origWndProc = SubclassWindow(scrollBar, lc_WndProcScrollBar);
                if (!lc_origSBWndProc) {
                    lc_origSBWndProc = origWndProc;
                }
                lcdData->scrollBar = scrollBar;
            }
        }
        if (scrollBar) {
            RECT clientRect;

            ScrollBar_SetRange(scrollBar, lcdData->min, lcdData->max);
            GetClientRect(lcdCtrl, &clientRect);
            lc_OnSize(lcdCtrl, SIZE_RESTORED, clientRect.right
                    , clientRect.bottom);
        }
    }
}

/*
 * OnSetOffset()
 */
void lc_OnSetOffset(HWND lcdCtrl, int offset)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lcdData->offset = offset;
}

/*
 * OnSetRange() - Sets the range of the LCD control but doesn't check or
 *                manipulate the current value.  If the range change changes
 *                the value, the old value or something other than the old
 *                value is returned, otherwise zero.
 */
int lc_OnSetRange(HWND lcdCtrl, int min, int max)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    HWND scrollBar = lcdData->scrollBar;
    int oldMin = lcdData->min;
    int oldMax = lcdData->max;
    int value;
    int result = 0;

    assert(style & LCS_ANY_SB);
    if (style & LCS_BOTTOM_SB) {
        value = ScrollBar_GetRPos(scrollBar, oldMin, oldMax);
    } else {
        value = ScrollBar_GetPos(scrollBar);
    }
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    lcdData->min = min;
    lcdData->max = max;
    result = ScrollBar_SetRangeRedraw(scrollBar, min, max);
    lc_SetScrollingLcd(lcdCtrl, value);

    return result;
}

/*
 * OnSetStrings()
 */
void lc_OnSetStrings(HWND lcdCtrl, _TUCHAR *strings)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);

    lcdData->strings = strings;
    InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
}

/*
 * OnSetText()
 */
void lc_OnSetText(HWND lcdCtrl, _TUCHAR *text)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    _TUCHAR *data = lcdData->data;
    int maxLen = lcdData->maxLen;
    int i;

    for (i = 0; i < maxLen && text[i]; i++) {
        data[i] = text[i];
    }
    for ( ; i < maxLen; i++) {
        data[i] = ' ';
    }
    InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
}

/*
 * OnSetValue()
 */
void lc_OnSetValue(HWND lcdCtrl, int value)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    int min = lcdData->min;
    int max = lcdData->max;
#ifdef _DEBUG
    int maxLen = lcdData->maxLen;

    if (maxLen == 0) {
        MessageBox(Prog_mainWnd
                , _T("A size must be set with the LM_SETLENGTH message\r\n")
                  _T("before using an LCD control.")
                , _T("Design-Time Error")
                , MB_ICONERROR | MB_OK);
        return;
    }
#endif
    if (value < min) {
        value = min;
    }
    if (value > max) {
        value = max;
    }
    lc_SetScrollingLcd(lcdCtrl, value);
}

/*
 * OnSize()
 */
void lc_OnSize(HWND lcdCtrl, UINT state, int cx, int cy)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    HWND scrollBar = lcdData->scrollBar;
    int sbX = 0;
    int sbY = 0;
    int sbW = cx;
    int sbH = cy;
    int lcdW, lcdH;

    lc_SetLcdRect(lcdCtrl);
    if (scrollBar) {
        lcdW = RECT_W(lcdData->lcdRect);
        lcdH = RECT_H(lcdData->lcdRect);
        if (style & LCS_BOTTOM_SB) {
            sbY += lcdH;
            sbH -= lcdH;
        } else if (style & LCS_LEFT_SB) {
            sbW -= lcdW - 1;
        } else { // style & LCS_RIGHT_SB
            sbX += lcdW;
            sbW -= lcdW - 1;
        }
        SetWindowPos(
                scrollBar
                , NULL
                , sbX, sbY
                , sbW, sbH
                , SWP_NOZORDER | SWP_NOREDRAW
            );
        InvalidateRect(lcdCtrl, NULL, FALSE);
    }
    if (GetFocus() == scrollBar) {
        if (!(style & LCS_ANY_SB)) {
            lc_OnSetCursorPos(lcdCtrl, lcdData->cursorPos);
        } else if (style & LCS_LEFT_SB || style & LCS_RIGHT_SB) {
            /*
             * The caret falls off the thumb when the scrollbar is resized.
             */
            SendMessage(scrollBar, WM_SETFOCUS, (WPARAM) lcdCtrl, 0);
        }
    }
}

/*
 * OnVScroll()
 */
void lc_OnVScroll(HWND lcdCtrl, HWND scrollBar, UINT code, int pos)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    UINT ctrlID = GetDlgCtrlID(lcdCtrl);
    HWND parentWnd = GetParent(lcdCtrl);
    int min = lcdData->min;
    int max = lcdData->max;
    int value = ScrollBar_GetRPos(scrollBar, min, max);
    int oldValue = value;
    LCDPAGEFUNC LcdPageFunc = lcdData->LcdPageFunc;

    switch (code) {
        case SB_BOTTOM:
            value = min;
            break;
        case SB_ENDSCROLL:
            break;
        case SB_LINEDOWN:
            if (value > min)
                value--;
            break;
        case SB_LINEUP:
            if (value < max)
                value++;
            break;
        case SB_PAGEDOWN:
            if (LcdPageFunc) {
                value -= LcdPageFunc(ctrlID, -1, value);
            } else {
                value -= lc_GetPageSize(min, max);
            }
            if (value < min)
                value = min;
            break;
        case SB_PAGEUP:
            if (LcdPageFunc) {
                value += LcdPageFunc(ctrlID, +1, value);
            } else {
                value += lc_GetPageSize(min, max);
            }
            if (value > max)
                value = max;
            break;
		case SB_THUMBTRACK:
            value = max - pos + min;
            break;
        case SB_TOP:
            value = max;
            break;
    }
    if (lc_entryHoldCnt) {
        lc_ClearEntryHold(lcdCtrl);
    }
    if (value != oldValue) {
        lc_SetScrollingLcd(lcdCtrl, value);
        SendNotification(parentWnd, ctrlID, lcdCtrl, LCN_SELCHANGE);
    }
}

/*
 * RemoveEntryHoldChar() - Removes the last character of the entry hold and
 *                         updates the cursor and LCD display.  Returns the
 *                         new last character as an optimization in OnChar().
 */
void lc_RemoveEntryHoldChar(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    int i = lc_entryHoldCnt;

    /*
     * If there's only one character, clear the entry hold.
     */
    if (i <= 1) {
        lc_ClearEntryHold(lcdCtrl);
    } else {
        /*
         * Bloop!
         */
        lc_entryHold[--i] = '\0';
        /*
         * Draw the LCD so the background is displayed properly.
         */
        InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
        lc_entryHoldCnt = i;
    }
}

/*
 * SetDragCursor() - Creates a copy or swap cursor for drag and drop operations
 *                   and uses it to set the current cursor.
 */
void lc_SetDragCursor(DRAGOPERATION dragOperation)
{
    ICONINFO iconInfo;
    HDC memDC;
    HDC overlayDC;
    HBITMAP oldBmp;
    HBITMAP oldOverlayBmp;
    BITMAP bmp;
    HBITMAP overlayBmp;
    int overlayX, overlayY;
    int x, y;

    if (dragOperation == DO_NONE) {
        SetCursor(Prog_arrowCursor);
        return;
    }
    memDC = CreateCompatibleDC(NULL);
    overlayDC = CreateCompatibleDC(NULL);
    /*
     * Get the information on the standard arrow cursor.
     */
    GetIconInfo(Prog_arrowCursor, &iconInfo);
    /*
     * Find the bottom right pixel of the arrow stem to figure out where to
     * draw the overlay.
     */
    GetObject(iconInfo.hbmMask, sizeof bmp, &bmp);
    /*
     * Find the lower right corner of the visible cursor.
     */
    oldBmp = SelectBitmap(memDC, iconInfo.hbmMask);
    /*
     * Find the right edge of the mask bitmap.
     */
    for (x = bmp.bmWidth; --x >= 0; ) {
        for (y = bmp.bmWidth; --y >= 0; ) {
            if (GetPixel(memDC, x, y) == BLACK) {
                overlayX = x + 1;
                if (overlayX > bmp.bmWidth - 12) {
                    overlayX = bmp.bmWidth - 12;
                }
                goto GotX;
            }
        }
    }
GotX:
    /*
     * Find the bottom edge of the mask bitmap.
     */
    for (y = bmp.bmWidth; --y >= 0; ) {
        for (x = bmp.bmWidth; --x >= 0; ) {
            if (GetPixel(memDC, x, y) == BLACK) {
                overlayY = y;
                if (overlayY > bmp.bmWidth - 12) {
                    overlayY = bmp.bmWidth - 12;
                }
                goto GotY;
            }
        }
    }
GotY:
    /*
     * Set up the overlay icon.
     */
    overlayBmp = CreateBitmap(11, 11, 1, 1, (dragOperation == DO_COPY)
            ? lc_copyOverlayBits : lc_swapOverlayBits);
#if _DEBUG
    if (!overlayBmp) {
        Error_LastError();
        return;
    }
#endif
    oldOverlayBmp = SelectBitmap(overlayDC, overlayBmp);
    /*
     * Copy the copy overlay icon into the cursor bitmaps.
     */
    if (iconInfo.hbmColor) {
        /*
         * Modify the color bitmap.
         */
        SelectBitmap(memDC, iconInfo.hbmColor);
        BitBlt(memDC, overlayX, overlayY, 11, 11, overlayDC, 0, 0, SRCCOPY);
        /*
         * Modify the mask bitmap.
         */
        SelectBitmap(memDC, iconInfo.hbmMask);
        BitBlt(memDC, overlayX, overlayY, 11, 11, overlayDC, 0, 0, BLACKNESS);
    } else {
        /*
         * Modify the AND mask.
         */
        SelectBitmap(memDC, iconInfo.hbmMask);
        BitBlt(memDC, overlayX, overlayY, 11, 11, overlayDC, 0, 0, BLACKNESS);
        /*
         * Modify the XOR mask.
         */
        BitBlt(memDC, overlayX, overlayY + bmp.bmWidth, 11, 11, overlayDC, 0, 0
                , SRCCOPY);
    }
    /*
     * Need to get the cursor bitmap out of the DC before calling
     * CreateIconIndirect().
     */
    SelectBitmap(memDC, oldBmp);
    /*
     * Create the copy cursor from the bitmaps.
     */
    if (lc_dragCursor) {
        SetCursor(Prog_arrowCursor);
        if (!DestroyIcon(lc_dragCursor)) {
            Error_LastError();
        }
    }
    lc_dragCursor = (HCURSOR) CreateIconIndirect(&iconInfo);
    SetCursor(lc_dragCursor);
    /*
     * Delete the bitmaps and DCs.
     */
    SelectBitmap(overlayDC, oldOverlayBmp);
    DeleteBitmap(overlayBmp);
    DeleteBitmap(iconInfo.hbmColor);
    DeleteBitmap(iconInfo.hbmMask);
    DeleteDC(memDC);
    DeleteDC(overlayDC);
}

/*
 * SetLcdRect() - Caches the lcd rectangle.
 */
void lc_SetLcdRect(HWND lcdCtrl)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int charDelta = CHAR_DELTA(style);
    int maxLen = lcdData->maxLen;
    int w;

    GetClientRect(lcdCtrl, &lcdData->lcdRect);
    if (style & LCS_BOTTOM_SB) {
        lcdData->lcdRect.bottom = LCD_HEIGHT(style);
    } else if (style & LCS_LEFT_SB || style & LCS_RIGHT_SB) {
        w = charDelta * maxLen + ((style & LCS_LARGE ? LARGE_HORZ_PADDING
                    : SMALL_HORZ_PADDING) << 1) + 1;
        if (style & LCS_LEFT_SB) {
            lcdData->lcdRect.left = lcdData->lcdRect.right - w;
        } else {
            lcdData->lcdRect.right = w;
        }
    }
}

/*
 * SetScrollingLcd() - Sets the text and scroll bar position of a numeric or
 *                     special LCD.
 */
void lc_SetScrollingLcd(HWND lcdCtrl, int value)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    HWND scrollBar = lcdData->scrollBar;

    lc_SetScrollingLcdText(lcdCtrl, value);
    if (scrollBar) {
        BOOL redraw = IsWindowEnabled(scrollBar);

        if (style & LCS_BOTTOM_SB) {
            ScrollBar_SetRPos(scrollBar, value, lcdData->min, lcdData->max
                    , redraw);
        } else {
            ScrollBar_SetPos(scrollBar, value, redraw);
        }
    }
}

/*
 * SetScrollingLcdText() - Sets the text of a numeric or special LCD control.
 */
void lc_SetScrollingLcdText(HWND lcdCtrl, int value)
{
    LCDDATA *lcdData = GET_LCDDATA(lcdCtrl);
    DWORD style = GetWindowStyle(lcdCtrl);
    int maxLen = lcdData->maxLen;
    const _TUCHAR *strings = lcdData->strings;
    _TUCHAR *data = lcdData->data;
    BOOL sign = FALSE;
    int i;

    /*
     * If the LCD has strings.
     */
    if (strings) {
        _tcsncpy(data, &strings[(value - lcdData->min) * maxLen], maxLen);
    /*
     * Else if it's numeric.
     */
    } else if (style & LCS_NUMERIC) {
        value += lcdData->offset;
        if (value < 0) {
            data[0] = '-';
            sign = TRUE;
            value = -value;
        } else if (value > 0 && style & LCS_SHOWPLUS) {
            data[0] = '+';
            sign = TRUE;
        }

        for (i = maxLen - 1; i >= sign; i--) {
            int digit = value % 10;

            if (value == 0) {
                if (i == (signed) maxLen - 1) {
                    data[i] = '0';
                } else {
                    for ( ; i >= sign; i--) {
                        data[i] = ' ';
                    }
                    break;
                }
            } else {
                data[i] = digit + '0';
            }
            value /= 10;
        }
    /*
     * Else this should never happen.
     */
    } else {
        return;
    }
    InvalidateRect(lcdCtrl, &lcdData->lcdRect, FALSE);
}

/*
 * WndProc()
 */
LRESULT CALLBACK lc_WndProc(HWND lcdCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(lcdCtrl, LCM_BEGINDRAG, lc_OnBeginDrag);
        HANDLE_MSG(lcdCtrl, LCM_GETCHAR, lc_OnGetChar);
        HANDLE_MSG(lcdCtrl, LCM_GETCURSORPOS, lc_OnGetCursorPos);
        HANDLE_MSG(lcdCtrl, LCM_GETLCDDLGRECT, lc_OnGetLcdDlgRect);
        HANDLE_MSG(lcdCtrl, LCM_GETLENGTH, lc_OnGetLength);
        HANDLE_MSG(lcdCtrl, LCM_GETOFFSET, lc_OnGetOffset);
        HANDLE_MSG(lcdCtrl, LCM_GETRANGE, lc_OnGetRange);
        HANDLE_MSG(lcdCtrl, LCM_GETRANGEMAX, lc_OnGetRangeMax);
        HANDLE_MSG(lcdCtrl, LCM_GETRANGEMIN, lc_OnGetRangeMin);
        HANDLE_MSG(lcdCtrl, LCM_GETTEXT, lc_OnGetText);
        HANDLE_MSG(lcdCtrl, LCM_GETVALUE, lc_OnGetValue);
        HANDLE_MSG(lcdCtrl, LCM_HIGHLIGHT, lc_OnHighlight);
        HANDLE_MSG(lcdCtrl, LCM_SETCURSORPOS, lc_OnSetCursorPos);
        HANDLE_MSG(lcdCtrl, LCM_SETLENGTH, lc_OnSetLength);
        HANDLE_MSG(lcdCtrl, LCM_SETOFFSET, lc_OnSetOffset);
        HANDLE_MSG(lcdCtrl, LCM_SETRANGE, lc_OnSetRange);
        HANDLE_MSG(lcdCtrl, LCM_SETSTRINGS, lc_OnSetStrings);
        HANDLE_MSG(lcdCtrl, LCM_SETTEXT, lc_OnSetText);
        HANDLE_MSG(lcdCtrl, LCM_SETVALUE, lc_OnSetValue);
        HANDLE_MSG(lcdCtrl, WM_CANCELMODE, lc_OnCancelMode);
        HANDLE_MSG(lcdCtrl, WM_CHAR, lc_OnChar);
        HANDLE_MSG(lcdCtrl, WM_CREATE, lc_OnCreate);
        HANDLE_MSG(lcdCtrl, WM_DESTROY, lc_OnDestroy);
        HANDLE_MSG(lcdCtrl, WM_ENABLE, lc_OnEnable);
        HANDLE_MSG(lcdCtrl, WM_GETDLGCODE, lc_OnGetDlgCode);
        HANDLE_MSG(lcdCtrl, WM_HSCROLL, lc_OnHScroll);
        HANDLE_MSG(lcdCtrl, WM_KEYDOWN, lc_OnKey);
        HANDLE_MSG(lcdCtrl, WM_KEYUP, lc_OnKey);
        HANDLE_MSG(lcdCtrl, WM_KILLFOCUS, lc_OnKillFocus);
        HANDLE_MSG(lcdCtrl, WM_LBUTTONDOWN, lc_OnLButtonDown);
        HANDLE_MSG(lcdCtrl, WM_LBUTTONUP, lc_OnLButtonUp);
        HANDLE_MSG(lcdCtrl, WM_MOUSEMOVE, lc_OnMouseMove);
        HANDLE_MSG(lcdCtrl, WM_MOUSEWHEEL, lc_OnMouseWheel);
        HANDLE_MSG(lcdCtrl, WM_PAINT, lc_OnPaint);
        HANDLE_MSG(lcdCtrl, WM_SETFOCUS, lc_OnSetFocus);
        HANDLE_MSG(lcdCtrl, WM_SIZE, lc_OnSize);
        HANDLE_MSG(lcdCtrl, WM_VSCROLL, lc_OnVScroll);
        case WM_RBUTTONDOWN:
            if (lc_dragOperation) {
                lc_dragOperation ^= DO_COPY | DO_SWAP;
                lc_SetDragCursor(lc_dragOperation);
            }
            return 0;
    }
    return DefWindowProc(lcdCtrl, message, wParam, lParam);
}

/*
 * WndProcScrollBar()
 */
LRESULT CALLBACK lc_WndProcScrollBar(HWND scrollBar, UINT message
        , WPARAM wParam, LPARAM lParam)
{
    HWND lcdCtrl = GetParent(scrollBar);

    switch (message) {
        HANDLE_MSG(lcdCtrl, WM_CHAR, lc_OnChar);
        HANDLE_MSG(lcdCtrl, WM_KEYDOWN, lc_OnKey);
        HANDLE_MSG(lcdCtrl, WM_KEYUP, lc_OnKey);
        case WM_KILLFOCUS:
        {
            lc_OnKillFocus(lcdCtrl, (HWND) wParam);
            break;
        }
        case WM_LBUTTONDOWN:
            lc_OnSetFocus(lcdCtrl, scrollBar);
            break;
        case WM_SETFOCUS:
            lc_OnSetFocus(lcdCtrl, scrollBar);
            break;
    }
    return CallWindowProc(lc_origSBWndProc, scrollBar, message, wParam
            , lParam);
}
