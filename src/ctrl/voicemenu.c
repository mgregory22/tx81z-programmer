/*
 * voicemenu.c - Voice selection pseudo-menu
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
#include "snapshot.h"
#include "ctrl/voicemenu.h"

/*
 * Global constants
 */
const _TUCHAR *VoiceMenu_className = _T("VoiceMenu");

/*
 * Global procedures
 */
extern HWND VoiceMenu_Create(HWND parentWnd, UINT voiceMenuCmd);
extern int VoiceMenu_GetLastSelection(void);
extern BOOL VoiceMenu_Register(void);
extern UINT VoiceMenu_Select(HWND voiceMenu, RECT *btnRect);
extern void VoiceMenu_SetLastSelection(int lastSelection);
extern void VoiceMenu_Update(HWND voiceMenu);

/*
 * Unit constants
 */
#define BANK_CNT        5
#define BANK_ITEM_CNT  32
#define CHAR_WIDTH      6
#define CHAR_HEIGHT     8
#define VOICE_NAME_LEN 16
#define VOICE_WIDTH    (CHAR_WIDTH * VOICE_NAME_LEN + 10)
#define VOICE_HEIGHT   (CHAR_HEIGHT + 4)
#define WND_WIDTH      (BANK_CNT * VOICE_WIDTH)
#define WND_HEIGHT     (BANK_ITEM_CNT * VOICE_HEIGHT)

/*
 * Unit procedures
 */
static BOOL vm_DoEvents(HWND voiceMenu);
static void vm_OnCommand(HWND voiceMenu, UINT cmdID, HWND control, UINT notify);
static void vm_OnDestroy(HWND voiceMenu);
static void vm_OnKey(HWND voiceMenu, UINT vk, BOOL down, int repeat, UINT keyFlags);
static void vm_OnLButtonDown(HWND voiceMenu, BOOL dblClick, int x, int y, UINT keyFlags);
static void vm_OnLButtonUp(HWND voiceMenu, int x, int y, UINT keyFlags);
static void vm_OnMouseMove(HWND voiceMenu, int x, int y, UINT keyFlags);
static void vm_OnPaint(HWND voiceMenu);
static LRESULT CALLBACK vm_WndProc(HWND voiceMenu, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static UINT vm_voiceMenuCmd;
static RECT vm_wndRect;
static HBITMAP vm_bitmap;
static int vm_result;
static BOOL vm_quitLoop;
static int vm_highlightItem;
static int vm_lastSelection;
static BOOL vm_gotClick;

/*
 * Procedure definitions
 */

/*
 * Create() - Creates and initializes the main window.
 */
HWND VoiceMenu_Create(HWND parentWnd, UINT voiceMenuCmd)
{
    HWND voiceMenu;
#define VM_STYLES (WS_POPUP | WS_DLGFRAME)
#define VM_EXSTYLES (WS_EX_LEFT)

    vm_voiceMenuCmd = voiceMenuCmd;
    /*
     * Create the window.
     */
    vm_wndRect.left = 0;
    vm_wndRect.top = 0;
    vm_wndRect.right = WND_WIDTH - 1;
    vm_wndRect.bottom = WND_HEIGHT - 1;
    AdjustWindowRectEx(&vm_wndRect, VM_STYLES, FALSE, VM_EXSTYLES);
    voiceMenu = CreateWindowEx(
            VM_EXSTYLES                               /* extended styles */
            , VoiceMenu_className                     /* window class */
            , NULL                                    /* caption text */
            , VM_STYLES                               /* styles */
            , vm_wndRect.left, vm_wndRect.top         /* left, top */
            , RECT_W(vm_wndRect), RECT_H(vm_wndRect)  /* width, height */
            , parentWnd                               /* parent window */
            , (HMENU) NULL                            /* menu */
            , Prog_instance                           /* program instance */
            , NULL                                    /* creation data */
        );
    if (!voiceMenu) {
        Error_LastErrorF(_T("Error creating voice menu"));
        return NULL;
    }
    /*
     * Create a monochrome bitmap for the voice menu display.
     */
    vm_bitmap = CreateBitmap(WND_WIDTH, WND_HEIGHT, 1, 1, NULL);
    if (!vm_bitmap) {
        Error_LastErrorF(_T("Error creating voice bitmap"));
    }

    return voiceMenu;
}

/*
 * GetLastSelection()
 */
int VoiceMenu_GetLastSelection(void)
{
    return vm_lastSelection;
}

/*
 * Register() - Registers the window class for the window.
 */
BOOL VoiceMenu_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = CS_OWNDC | CS_SAVEBITS;
    classInfo.lpfnWndProc = (WNDPROC) vm_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = 0;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = VoiceMenu_className;
    classInfo.hIconSm = NULL;
    if (!RegisterClassEx(&classInfo)) {
        Error_LastErrorF(_T("Error registering window class"));
        return FALSE;
    }
    return TRUE;
}

/*
 * Select() - Displays window and allows user to select a voice.
 */
UINT VoiceMenu_Select(HWND voiceMenu, RECT *btnRect)
{
    DWORD pos;
    POINT pt;
    MSG msg;
    HWND parentWnd = GetParent(voiceMenu);
    HWND activeWnd;

    /*
     * Position the window somewhere around btnRect.
     */
    Window_CalcPositionAroundRect(&vm_wndRect, btnRect);
    /*
     * Show the window, but don't activate it.
     */
    SetWindowPos(voiceMenu, HWND_TOPMOST, vm_wndRect.left, vm_wndRect.top
            , 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    vm_DoEvents(voiceMenu);
    /*
     * Set up status variables.
     */
    vm_highlightItem = vm_lastSelection;
    vm_gotClick = FALSE;
    vm_quitLoop = FALSE;
    /*
     * Highlight the hover item.
     */
    pos = GetMessagePos();
    pt.x = GET_X_LPARAM(pos);
    pt.y = GET_Y_LPARAM(pos);
    MapWindowPoints(HWND_DESKTOP, voiceMenu, &pt, 1);
    vm_OnMouseMove(voiceMenu, pt.x, pt.y, 0);
    /*
     *  We want to receive all mouse messages, but since
     *  only the active window can capture the mouse,
     *  we have to set the capture to our owner window,
     *  and then steal the mouse messages out from under him.
     */
    SetCapture(parentWnd);
    /*
     * Do modal message loop.
     */
    while (GetMessage(&msg, NULL, 0, 0)) {
        /*
         *  If something happened that caused us to stop, then stop.
         */
        if (vm_quitLoop) {
            break;
        }
        /*
         *  If our owner stopped being the active window (e.g., the user
         *  Alt+Tab'd to another window in the meantime), then stop.
         */
        activeWnd = GetActiveWindow();
        if ((activeWnd != parentWnd && !IsChild(activeWnd, parentWnd))
                || (GetCapture() != parentWnd))
        {
            break;
        }
        /*
         *  At this point, we get to snoop at all input messages before they
         *  get dispatched.  This allows us to route all input to our popup
         *  window even if really belongs to somebody else.
         *
         *  All mouse messages are remunged and directed at our popup menu.
         *  If the mouse message arrives as client coordinates, then we have
         *  to convert it from the client coordinates of the original target
         *  to the client coordinates of the new target.
         */
        switch (msg.message) {
            /*
             *  These mouse messages arrive in client coordinates,
             *  so in addition to stealing the message, we also
             *  need to convert the coordinates.
             */
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
                pt.x = (short)LOWORD(msg.lParam);
                pt.y = (short)HIWORD(msg.lParam);
                MapWindowPoints(msg.hwnd, voiceMenu, &pt, 1);
                msg.lParam = MAKELPARAM(pt.x, pt.y);
                msg.hwnd = voiceMenu;
                break;

            /*
             *  These mouse messages arrive in screen coordinates,
             *  so we just need to steal the message.
             */
            case WM_NCMOUSEMOVE:
            case WM_NCLBUTTONDOWN:
            case WM_NCLBUTTONUP:
            case WM_NCLBUTTONDBLCLK:
            case WM_NCRBUTTONDOWN:
            case WM_NCRBUTTONUP:
            case WM_NCRBUTTONDBLCLK:
            case WM_NCMBUTTONDOWN:
            case WM_NCMBUTTONUP:
            case WM_NCMBUTTONDBLCLK:
                msg.hwnd = voiceMenu;
                break;

            /*
             *  Steal all keyboard messages, too.
             */
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
                msg.hwnd = voiceMenu;
                break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        /*
         *  If something happened that caused us to stop, then stop.
         */
        if (vm_quitLoop) {
            break;
        }
        /*
         *  If our owner stopped being the active window (e.g., the user
         *  Alt+Tab'd to another window in the meantime), then stop.
         */
        activeWnd = GetActiveWindow();
        if ((activeWnd != parentWnd && !IsChild(activeWnd, parentWnd))
                || (GetCapture() != parentWnd))
        {
            break;
        }
    }
    ReleaseCapture();
    ShowWindow(voiceMenu, SW_HIDE);
    /*
     *  If we got a WM_QUIT message, then re-post it so the caller's
     *  message loop will see it.
     */
    if (msg.message == WM_QUIT) {
        PostQuitMessage((int)msg.wParam);
    }
    /*
     * Set the last selection.
     */
    vm_lastSelection = vm_result;

    return vm_result == -1 ? 0 : vm_voiceMenuCmd;
}

/*
 * SetLastSelection()
 */
void VoiceMenu_SetLastSelection(int lastSelection)
{
    vm_lastSelection = lastSelection;
}

/*
 * Update() - Updates the monochrome bitmap with the voice names.
 */
void VoiceMenu_Update(HWND voiceMenu)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    HDC dC = GetDC(voiceMenu);
    HDC memDC = CreateCompatibleDC(dC);
    RECT bmpRect = { 0, 0, WND_WIDTH - 1, WND_HEIGHT - 1 };
    _TUCHAR voiceName[VOICE_NAME_LEN + 1];
    int bank, item;
    int x, y;

    ReleaseDC(voiceMenu, dC);
    SelectBitmap(memDC, vm_bitmap);
    SelectBrush(memDC, Prog_wndBrush);
    SelectPen(memDC, Prog_wndPen);
    Rectangle(memDC, PASS_RECT_FIELDS(bmpRect));
    SelectPen(memDC, Prog_wndTextPen);
    /*
     * Draw the grid lines.
     */
    for (bank = 1; bank < BANK_CNT; bank++) {
        x = VOICE_WIDTH * bank - 1;
        MoveToEx(memDC, x, 0, NULL);
        LineTo(memDC, x, WND_HEIGHT - 1);
    }
    for (item = 1; item < BANK_ITEM_CNT; item++) {
        y = VOICE_HEIGHT * item - 1;
        MoveToEx(memDC, 0, y, NULL);
        LineTo(memDC, WND_WIDTH - 1, y);
    }
    /*
     * Draw the voice names.
     */
    for (bank = 0; bank < BANK_CNT; bank++) {
        for (item = 0; item < BANK_ITEM_CNT; item++) {
            int sIdx = Snapshot_MemNumToSnapshotIndex(bank * BANK_ITEM_CNT
                    + item);

            Snapshot_FormatName(snapshot, sIdx, NF_DASHED, voiceName);
            MiniFont_DrawString(memDC, VOICE_WIDTH * bank + 2
                    , VOICE_HEIGHT * item + 2, voiceName, VOICE_NAME_LEN
                    , Prog_wndTextColor);
        }
    }
    DeleteDC(memDC);
}

/*
 * DoEvents()
 */
BOOL vm_DoEvents(HWND voiceMenu)
{
    MSG msg = {0};
    BOOL gotMsg = FALSE;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!TranslateAccelerator(msg.hwnd, Prog_accels, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        gotMsg = TRUE;
    }
    return gotMsg;
}

/*
 * OnCommand()
 */
void vm_OnCommand(HWND voiceMenu, UINT cmdID, HWND control, UINT notify)
{
    switch (cmdID) {
        case WM_CLOSE:
            vm_quitLoop = TRUE;
            break;
    }
}

/*
 * OnDestroy()
 */
void vm_OnDestroy(HWND voiceMenu)
{
    DeleteBitmap(vm_bitmap);
}

/*
 * OnKey()
 */
void vm_OnKey(HWND voiceMenu, UINT vk, BOOL down, int repeat, UINT keyFlags)
{
    if (vk == VK_ESCAPE) {
        vm_result = -1;
        vm_quitLoop = TRUE;
    }
}

/*
 * OnLButtonDown()
 */
void vm_OnLButtonDown(HWND voiceMenu, BOOL dblClick, int x, int y
        , UINT keyFlags)
{
    vm_gotClick = TRUE;
    if (x < 0 || x > WND_WIDTH - 1 || y < 0 || y > WND_HEIGHT - 1) {
        vm_result = -1;
        vm_quitLoop = TRUE;
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
    }
}

/*
 * OnLButtonUp()
 */
void vm_OnLButtonUp(HWND voiceMenu, int x, int y, UINT keyFlags)
{
    if (x < 0 || x > WND_WIDTH - 1 || y < 0 || y > WND_HEIGHT - 1) {
        if (!vm_gotClick) {
            return;
        }
        vm_result = -1;
    } else {
        vm_result = (x / VOICE_WIDTH) * BANK_ITEM_CNT + (y / VOICE_HEIGHT);
    }
    vm_gotClick = FALSE;
    vm_quitLoop = TRUE;
}

/*
 * OnMouseMove()
 */
void vm_OnMouseMove(HWND voiceMenu, int x, int y, UINT keyFlags)
{
    int hoverItem = (x / VOICE_WIDTH) * BANK_ITEM_CNT + (y / VOICE_HEIGHT);
    HDC dC = GetDC(voiceMenu);
    RECT itemRect;

    /*
     * Unhighlight highlighted item.
     */
    if (vm_highlightItem > -1) {
        itemRect.left = (vm_highlightItem / BANK_ITEM_CNT) * VOICE_WIDTH;
        itemRect.top = (vm_highlightItem % BANK_ITEM_CNT) * VOICE_HEIGHT;
        itemRect.right = itemRect.left + VOICE_WIDTH - 1;
        itemRect.bottom = itemRect.top + VOICE_HEIGHT - 1;
        InvertRect(dC, &itemRect);
    }
    if (x < 0 || x > WND_WIDTH - 1 || y < 0 || y > WND_HEIGHT - 1) {
        vm_highlightItem = -1;
    } else {
        /*
         * Highlight the hover item.
         */
        itemRect.left = (hoverItem / BANK_ITEM_CNT) * VOICE_WIDTH;
        itemRect.top = (hoverItem % BANK_ITEM_CNT) * VOICE_HEIGHT;
        itemRect.right = itemRect.left + VOICE_WIDTH - 1;
        itemRect.bottom = itemRect.top + VOICE_HEIGHT - 1;
        InvertRect(dC, &itemRect);
        vm_highlightItem = hoverItem;
    }
    ReleaseDC(voiceMenu, dC);
}

/*
 * OnPaint()
 */
void vm_OnPaint(HWND voiceMenu)
{
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(voiceMenu, &paint);
    HDC memDC = CreateCompatibleDC(paintDC);

    SelectBitmap(memDC, vm_bitmap);
    BitBlt(paintDC, 0, 0, WND_WIDTH - 1, WND_HEIGHT - 1, memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
    EndPaint(voiceMenu, &paint);
}

/*
 * WndProc()
 */
LRESULT CALLBACK vm_WndProc(HWND voiceMenu, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(voiceMenu, WM_COMMAND, vm_OnCommand);
        HANDLE_MSG(voiceMenu, WM_DESTROY, vm_OnDestroy);
        HANDLE_MSG(voiceMenu, WM_KEYDOWN, vm_OnKey);
        HANDLE_MSG(voiceMenu, WM_LBUTTONDOWN, vm_OnLButtonDown);
        HANDLE_MSG(voiceMenu, WM_LBUTTONUP, vm_OnLButtonUp);
        HANDLE_MSG(voiceMenu, WM_MOUSEMOVE, vm_OnMouseMove);
        HANDLE_MSG(voiceMenu, WM_PAINT, vm_OnPaint);
    }
    return DefWindowProc(voiceMenu, message, wParam, lParam);
}
