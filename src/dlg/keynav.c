/*
 * keynav.c - keyboard navigation routines
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
#include "ctrl/lcdctrl.h"
#include "prog.h"
#include "resource.h"
#include "dlg/voicedlg.h"
#include "dlg/keynav.h"


/*
 * Global procedures
 */
extern HWND KeyNav_FindAdjacentCtrl(HWND dlg, KEYNAV *keyNav, size_t keyNavCnt
        , KEYALT *keyAlt, size_t keyAltCnt, UINT focusID, KEYNAVIDX toDir
        , KEYNAVIDX fromDir, UINT *outNavIndex, BOOL savePrevious);
extern void KeyNav_SubclassButton(HWND button);
extern void KeyNav_SubclassComboBox(HWND comboBox);
extern BOOL CALLBACK KeyNav_SubclassControls(HWND ctrl, LPARAM lParam);
extern void KeyNav_SubclassLcdCtrl(HWND lcdCtrl);
extern void KeyNav_SubclassRPanel(HWND rPanel);
extern void KeyNav_SubclassScrollBar(HWND scrollBar);


/*
 * Unit procedures
 */
static void kn_BtnOnKey(HWND button, UINT vk, BOOL down, int repeat, UINT flags);
static void kn_BtnOnMouseWheel(HWND button, int delta, int x, int y, UINT keyFlags);
static LRESULT CALLBACK kn_BtnWndProc(HWND button, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK kn_CboWndProc(HWND comboBox, UINT message, WPARAM wParam, LPARAM lParam);
static UINT kn_GetAlternateID(KEYALT *keyAlt, size_t keyAltCnt, UINT ctrlID);
static UINT kn_IdToNavIndex(KEYNAV *keyNav, size_t keyNavCnt, UINT ctrlID);
static void kn_LcdOnKey(HWND lcdCtrl, UINT vk, BOOL down, int repeat, UINT flags);
static void kn_LcdSbOnKey(HWND ctrl, UINT vk, BOOL down, int repeat, UINT flags);
static LRESULT CALLBACK kn_LcdSbWndProc(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK kn_LcdWndProc(HWND lcdCtrl, UINT message, WPARAM wParam, LPARAM lParam);
static void kn_OnKey(HWND ctrl, UINT vk, BOOL down, int repeat, UINT flags, WNDPROC origWndProc);
static void kn_OnMouseWheel(HWND ctrl, int delta, int x, int y, UINT keyFlags, WNDPROC origWndProc);
static LRESULT CALLBACK kn_RpcWndProc(HWND rPanel, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK kn_SbWndProc(HWND comboBox, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static WNDPROC kn_origBtnWndProc;
static WNDPROC kn_origCboWndProc;
static WNDPROC kn_origLcdWndProc;
static WNDPROC kn_origLcdSbWndProc;
static WNDPROC kn_origRpcWndProc;
static WNDPROC kn_origSbWndProc;


/*
 * Procedure definitions
 */

/*
 * FindAdjacentCtrl() - Finds an adjacent control to move to with keyboard
 *                      navigation.
 */
HWND KeyNav_FindAdjacentCtrl(HWND dlg, KEYNAV *keyNav, size_t keyNavCnt
        , KEYALT *keyAlt, size_t keyAltCnt, UINT focusID, KEYNAVIDX toDir
        , KEYNAVIDX fromDir, UINT *outNavIndex, BOOL savePrevious)
{
    int navIndex;
    UINT destID;
    HWND destCtrl;
    UINT origFocusID = focusID;
    
    /*
     * Get the index of the focused control in the nav table.
     */
    navIndex = kn_IdToNavIndex(keyNav, keyNavCnt, focusID);
    /*
     * If the control doesn't exist in the table.
     */
    if (navIndex == UINT_MAX) {
        /*
         * Look for its alternate's original control.
         */
        focusID = kn_GetAlternateID(keyAlt, keyAltCnt, focusID);
        if (!focusID) {
            return NULL;
        }
        navIndex = kn_IdToNavIndex(keyNav, keyNavCnt, focusID);
        if (navIndex == UINT_MAX) {
            return NULL;
        }
    }
    /*
     * Find the adjacent control.
     */
    do {
        /*
         * Get the ID of the control adjacent to the focused control
         * from the nav array.
         */
        destID = keyNav[navIndex][toDir];
        /*
         * If there is no adjacent control.
         */
        if (!destID) {
            /*
             * Try for an alternate???
             */
            destID = kn_GetAlternateID(keyAlt, keyAltCnt, destID);
            if (!destID) {
                return NULL;
            }
        }
        destCtrl = GetDlgItem(dlg, destID);
        /*
         * Get the navIndex of the adjacent control so the nav table can be
         * updated and so the scrolling of the window will be correct by
         * using the nav array to check for edge controls.
         */
        navIndex = kn_IdToNavIndex(keyNav, keyNavCnt, destID);
        /*
         * If the control is hidden, get the alternate control.
         */
        if (!IsWindowVisible(destCtrl)) {
            UINT altID = kn_GetAlternateID(keyAlt, keyAltCnt, destID);

            /*
             * If the control has an alternate, and the alternate is not the
             * same as the original -- that is, for example, we're not moving
             * left from a fine freq control to a coarse freq control where the
             * alternate would be the same for both.
             */
            if (altID && altID != origFocusID) {
                /*
                 * Set the destination to the alternate control and get its
                 * index into the nav table.
                 */
                destID = altID;
                destCtrl = GetDlgItem(dlg, destID);
                navIndex = kn_IdToNavIndex(keyNav, keyNavCnt, destID);
            }
        }
        /*
         * If the control that was found is disabled, go through this procedure
         * again to find the control adjacent to the disabled one.
         */
    } while (!IsWindowEnabled(destCtrl) || !IsWindowVisible(destCtrl));

    /*
     * If no control could be found, see if the focused control has
     * an alternate.
     */
    assert(destID != origFocusID);

    /*
     * Save the focused control in the nav array so that going right
     * from the new control will return the cursor to where it was.
     */
    if (navIndex == -1) {
        /*
         * Try to get a valid <navIndex>.
         */
        navIndex = kn_IdToNavIndex(keyNav, keyNavCnt
                , kn_GetAlternateID(keyAlt, keyAltCnt, destID));
    } else if (savePrevious) {
        keyNav[navIndex][fromDir] = focusID;
    }
    *outNavIndex = navIndex;

    return destCtrl;
}

/*
 * SubclassButton() - Installs keyboard navigation into a button control.
 */
void KeyNav_SubclassButton(HWND button)
{
    WNDPROC origBtnWndProc = SubclassWindow(button, kn_BtnWndProc);

    if (!kn_origBtnWndProc) {
        kn_origBtnWndProc = origBtnWndProc;
    }
}

/*
 * SubclassComboBox() - Installs keyboard navigation into a combobox control.
 */
void KeyNav_SubclassComboBox(HWND comboBox)
{
    WNDPROC origCboWndProc = SubclassWindow(comboBox, kn_CboWndProc);

    if (!kn_origCboWndProc) {
        kn_origCboWndProc = origCboWndProc;
    }
}

/*
 * SubclassControls() - Sets up standard windows controls for keyboard
 *                      navigation.
 */
BOOL CALLBACK KeyNav_SubclassControls(HWND ctrl, LPARAM lParam)
{
    _TUCHAR className[30];

    GetClassName(ctrl, className, 30);

    if (StriEq(className, Prog_buttonClassName)) {
        KeyNav_SubclassButton(ctrl);
    } else if (StriEq(className, Prog_comboBoxClassName)) {
        KeyNav_SubclassComboBox(ctrl);
    } else if (StriEq(className, LcdCtrl_className)) {
        KeyNav_SubclassLcdCtrl(ctrl);
    }

    return TRUE;
}

/*
 * SubclassLcdCtrl() - Installs keyboard navigation into an LCD control.
 */
void KeyNav_SubclassLcdCtrl(HWND lcdCtrl)
{
    WNDPROC origLcdWndProc, origLcdSbWndProc;
    HWND scrollBar;

    origLcdWndProc = SubclassWindow(lcdCtrl, kn_LcdWndProc);
    scrollBar = GetDlgItem(lcdCtrl, IDC_LCD_SCROLLBAR);
    origLcdSbWndProc = SubclassWindow(scrollBar, kn_LcdSbWndProc);
    if (!kn_origLcdWndProc) {
        kn_origLcdWndProc = origLcdWndProc;
    }
    if (!kn_origLcdSbWndProc) {
        kn_origLcdSbWndProc = origLcdSbWndProc;
    }
}

/*
 * SubclassRPanel() - Installs keyboard navigation into a radio panel
 *                        control.
 */
void KeyNav_SubclassRPanel(HWND rPanel)
{
    WNDPROC origRpcWndProc = SubclassWindow(rPanel, kn_RpcWndProc);

    if (!kn_origRpcWndProc) {
        kn_origRpcWndProc = origRpcWndProc;
    }
}

/*
 * SubclassScrollBar() - Installs keyboard navigation into a scrollbar control.
 */
void KeyNav_SubclassScrollBar(HWND scrollBar)
{
    WNDPROC origSbWndProc = SubclassWindow(scrollBar, kn_SbWndProc);

    if (!kn_origSbWndProc) {
        kn_origSbWndProc = origSbWndProc;
    }
}

/*
 * BtnOnKey() - Key handler for subclassed buttons.
 */
void kn_BtnOnKey(HWND button, UINT vk, BOOL down, int repeat, UINT flags)
{
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            if (down) {
                FORWARD_WM_KEYDOWN(GetParent(button), vk, repeat, flags
                        , SendMessage);
            }
            return;
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_RETURN:
        case VK_DELETE:
        case VK_INSERT: {
            short scan = VkKeyScan(_T(' '));
            vk = VK_SPACE;
            flags = MAKEWORD(scan, HIBYTE(flags));
            break;
        }
    }
    CallWindowProc(kn_origBtnWndProc, button, down ? WM_KEYDOWN : WM_KEYUP
            , (WPARAM) vk, MAKELPARAM(repeat, flags));
}

/*
 * BtnOnMouseWheel() - Mouse wheel handler for subclassed buttons.
 */
void kn_BtnOnMouseWheel(HWND button, int delta, int x, int y, UINT keyFlags)
{
    if (VoiceDlg_mouseWheel == MW_SCROLL) {
        FORWARD_WM_MOUSEWHEEL(GetParent(button), delta, x, y, keyFlags
                , SendMessage);
    } else if (VoiceDlg_mouseWheel == MW_EDIT) {
        CallWindowProc(kn_origBtnWndProc, button, WM_KEYDOWN
                , (WPARAM) VK_SPACE, 0);
        CallWindowProc(kn_origBtnWndProc, button, WM_KEYUP
                , (WPARAM) VK_SPACE, 0);
    }
}

/*
 * BtnWndProc() - Window procedure for subclassed buttons.
 */
LRESULT CALLBACK kn_BtnWndProc(HWND button, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(button, WM_KEYDOWN, kn_BtnOnKey);
        HANDLE_MSG(button, WM_KEYUP, kn_BtnOnKey);
        HANDLE_MSG(button, WM_MOUSEWHEEL, kn_BtnOnMouseWheel);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(button);

            MapWindowPoints(button, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(button), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origBtnWndProc, button, message, wParam, lParam);
}

/*
 * CboWndProc() - Window procedure for subclasses combo boxes.
 */
LRESULT CALLBACK kn_CboWndProc(HWND comboBox, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_KEYDOWN:
            kn_OnKey(comboBox, (UINT) wParam, TRUE, LOWORD(lParam)
                    , (UINT) HIWORD(lParam), kn_origCboWndProc);
            return 0;
        case WM_MOUSEWHEEL:
            kn_OnMouseWheel(comboBox, (int)(short)HIWORD(wParam)
                    , (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)
                    , (UINT)(short)LOWORD(wParam), kn_origCboWndProc);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(comboBox);

            MapWindowPoints(comboBox, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(comboBox), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origCboWndProc, comboBox, message, wParam, lParam);
}

/*
 * GetAlternateID() - Gets a control's alternate.  Used for user-swappable
 *                    controls.
 */
UINT kn_GetAlternateID(KEYALT *keyAlt, size_t keyAltCnt, UINT ctrlID)
{
    size_t i, j;

    for (i = 0; i < keyAltCnt; i++) {
        for (j = 0; j < 2; j++) {
            if (keyAlt[i][j] == ctrlID) {
                return keyAlt[i][1 - j];
            }
        }
    }

    return 0;
}

/*
 * IdToNavIndex() - Scans through the keyNav array and returns the index at
 *                  which the control was found or returns UINT_MAX if not
 *                  found.
 */
UINT kn_IdToNavIndex(KEYNAV *keyNav, size_t keyNavCnt, UINT ctrlID)
{
    size_t i;

    for (i = 0; i < keyNavCnt; i++) {
        if (keyNav[i][KN_CTRLID] == ctrlID) {
            return i;
        }
    }

    return UINT_MAX;
}

/*
 * LcdOnKey() - Key handler for text LCD controls.
 */
void kn_LcdOnKey(HWND lcdCtrl, UINT vk, BOOL down, int repeat, UINT flags)
{
    switch (vk) {
        case VK_LEFT:
            if (LcdCtrl_GetCursorPos(lcdCtrl) > 0) {
                break;
            }
            goto ForwardKey;
        case VK_RIGHT:
            if (LcdCtrl_GetCursorPos(lcdCtrl) < LcdCtrl_GetLength(lcdCtrl) - 1)
            {
                break;
            }
            goto ForwardKey;
        case VK_UP:
        case VK_DOWN:
ForwardKey:
            FORWARD_WM_KEYDOWN(GetParent(lcdCtrl), vk, repeat, flags
                    , SendMessage);
            return;
        default:
            break;
    }
    CallWindowProc(kn_origLcdWndProc, lcdCtrl, WM_KEYDOWN, (WPARAM) vk
            , MAKELPARAM(repeat, flags));
}

/*
 * LcdSbOnKey() - Key handler for numeric LCD scroll bars.
 */
void kn_LcdSbOnKey(HWND ctrl, UINT vk, BOOL down, int repeat, UINT flags)
{
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            FORWARD_WM_KEYDOWN(GetParent(GetParent(ctrl)), vk, repeat, flags
                    , SendMessage);
            return;
        case VK_INSERT:
            vk = VK_UP;
            break;
        case VK_DELETE:
            vk = VK_DOWN;
            break;
    }
    CallWindowProc(kn_origLcdSbWndProc, ctrl, WM_KEYDOWN, (WPARAM) vk
            , MAKELPARAM(repeat, flags));
}

/*
 * LcdSbWndProc() - Handles keyboard navigation for LCD scroll bars.
 */
LRESULT CALLBACK kn_LcdSbWndProc(HWND scrollBar, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_KEYDOWN:
            kn_LcdSbOnKey(scrollBar, (UINT) wParam, TRUE, LOWORD(lParam)
                    , (UINT) HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            kn_OnMouseWheel(scrollBar, (int)(short)HIWORD(wParam)
                    , (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)
                    , (UINT)(short)LOWORD(wParam), kn_origLcdSbWndProc);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(scrollBar);

            MapWindowPoints(scrollBar, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(GetParent(scrollBar)), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origLcdSbWndProc, scrollBar, message, wParam
            , lParam);
}

/*
 * LcdWndProc() - Handles keyboard navigation for LCD controls.
 */
LRESULT CALLBACK kn_LcdWndProc(HWND lcdCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_KEYDOWN:
            kn_LcdOnKey(lcdCtrl, (UINT) wParam, TRUE, LOWORD(lParam)
                    , (UINT) HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            kn_OnMouseWheel(lcdCtrl, (int)(short)HIWORD(wParam)
                    , (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)
                    , (UINT)(short)LOWORD(wParam), kn_origLcdWndProc);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(lcdCtrl);

            MapWindowPoints(lcdCtrl, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(lcdCtrl), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origLcdWndProc, lcdCtrl, message, wParam, lParam);
}

/*
 * OnKey() - Key handler for scroll bars and combo boxes.
 */
void kn_OnKey(HWND ctrl, UINT vk, BOOL down, int repeat, UINT flags
        , WNDPROC origWndProc)
{
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            FORWARD_WM_KEYDOWN(GetParent(ctrl), vk, repeat, flags, SendMessage);
            return;
        case VK_INSERT:
            vk = VK_UP;
            break;
        case VK_DELETE:
            vk = VK_DOWN;
            break;
    }
    CallWindowProc(origWndProc, ctrl, WM_KEYDOWN, (WPARAM) vk
            , MAKELPARAM(repeat, flags));
}

/*
 * OnMouseWheel() - Mouse wheel handler for LCD scroll bars.
 */
void kn_OnMouseWheel(HWND ctrl, int delta, int x, int y, UINT keyFlags
        , WNDPROC origWndProc)
{
    if (VoiceDlg_mouseWheel == MW_SCROLL) {
        FORWARD_WM_MOUSEWHEEL(GetParent(ctrl), delta, x, y, keyFlags
                , SendMessage);
    } else if (VoiceDlg_mouseWheel == MW_EDIT) {
        UINT vk;

        if (delta < 0) {
            vk = VK_DOWN;
        } else if (delta > 0) {
            vk = VK_UP;
        } else {
            return;
        }

        CallWindowProc(origWndProc, ctrl, WM_KEYDOWN, (WPARAM) vk, 0);
    }
}

/*
 * RpcWndProc() - Handles middle mouse button for radio panel controls.
 */
LRESULT CALLBACK kn_RpcWndProc(HWND rPanel, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_KEYDOWN:
            kn_OnKey(rPanel, (UINT) wParam, TRUE, LOWORD(lParam)
                    , (UINT) HIWORD(lParam), kn_origRpcWndProc);
            return 0;
        case WM_MOUSEWHEEL:
            kn_OnMouseWheel(rPanel, (int)(short)HIWORD(wParam)
                    , (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)
                    , (UINT)(short)LOWORD(wParam), kn_origRpcWndProc);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(rPanel);

            MapWindowPoints(rPanel, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(rPanel), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origRpcWndProc, rPanel, message, wParam, lParam);
}

/*
 * SbWndProc() - Handles middle mouse button for scroll bars.
 */
LRESULT CALLBACK kn_SbWndProc(HWND scrollBar, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        case WM_KEYDOWN:
            kn_LcdSbOnKey(scrollBar, (UINT) wParam, TRUE, LOWORD(lParam)
                    , (UINT) HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            kn_OnMouseWheel(scrollBar, (int)(short)HIWORD(wParam)
                    , (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)
                    , (UINT)(short)LOWORD(wParam), kn_origSbWndProc);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(scrollBar);

            MapWindowPoints(scrollBar, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        case WM_SETFOCUS:
            SendMessage(GetParent(scrollBar), KNN_FOCUSCHANGED, 0, 0);
            break;
    }
    return CallWindowProc(kn_origSbWndProc, scrollBar, message, wParam, lParam);
}

