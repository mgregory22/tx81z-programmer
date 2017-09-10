/*
 * kybdctrl.c - Musical keyboard control for sending MIDI note data.
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
#include "ctrl/kybdctrl.h"

/*
 * Unit types
 */
typedef struct {
    SIZE white;
    SIZE black;
    int blackDeltas[5];
    int firstKey;
    int keyCnt;
    DWORD downKeys[4];
    DWORD disabledKeys[4];
    int lastKeyChanged;
    int lastVelocity;
    BOOL sustain;
    HWND focusVictim;
} KYBDDATA;

/*
 * Global constants
 */
const _TUCHAR *KybdCtrl_className = _T("KybdCtrl");

/*
 * Global procedures
 */
extern BOOL KybdCtrl_Register(void);
extern int KybdCtrl_HitTest(HWND kybdCtrl, int x, int y, int *velocity);
extern BOOL KybdCtrl_IsKeyDown(HWND kybdCtrl, int key);
extern _TUCHAR *KybdCtrl_KeyToText(int key, _TUCHAR *buf);

/*
 * Unit constants
 */
#define KYBD_EXTRA       4
#define GWL_KYBDDATA     0

/*
 * Unit procedures
 */
static int kc_BlackKeyIndex(int key);
static void kc_DrawKey(HWND kybdCtrl, int key, BOOL down);
static void kc_GetKeyPoly(KYBDDATA *kybdData, int key, POINT verts[8], int *vertCnt);
static int kc_HitTest(KYBDDATA *kybdData, int x, int y, int *velocity);
static BOOL kc_IsKeyBlack(int key);
static BOOL kc_IsKeyFlagSet(DWORD keyFlags[4], int key);
static void kc_OnClose(HWND kybdCtrl);
static BOOL kc_OnCreate(HWND kybdCtrl, LPCREATESTRUCT createStruct);
static void kc_OnDestroy(HWND kybdCtrl);
static DWORD kc_OnGetLastChanged(HWND kybdCtrl);
static long kc_OnGetRange(HWND kybdCtrl);
static void kc_OnKey(HWND kybdCtrl, UINT vk, BOOL down, int repeat, UINT flags);
static void kc_OnLButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y, UINT keyFlags);
static void kc_OnLButtonUp(HWND kybdCtrl, int x, int y, UINT keyFlags);
static void kc_OnMouseMove(HWND kybdCtrl, int x, int y, UINT keyFlags);
static void kc_OnPaint(HWND kybdCtrl);
static void kc_OnPushKey(HWND kybdCtrl, int key, int velocity);
static void kc_OnRButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y, UINT keyFlags);
static void kc_OnReleaseKey(HWND kybdCtrl, int key, int velocity);
static void kc_OnReleaseAllKeys(HWND kybdCtrl);
static BOOL kc_OnSetCursor(HWND kybdCtrl, HWND cursorWnd, UINT codeHitTest, UINT msg);
static void kc_OnSetRange(HWND kybdCtrl, int firstKey, int keyCnt);
static void kc_ReleaseAllKeys(HWND kybdCtrl);
static void kc_ResetKeyFlag(DWORD keyFlags[4], int key);
static void kc_SetKeyFlag(DWORD keyFlags[4], int key);
static int kc_WhiteKeyToKey(int whiteKey);
static int kc_WhiteKeyIndex(int key);
static LRESULT CALLBACK kc_WndProc(HWND kybdCtrl, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HCURSOR kc_cursor;


/*
 * Procedure definitions
 */

/*
 * HitTest() - Give it a mouse click, get back a key and velocity.  velocity
 *             can be NULL if you don't need it.
 */
int KybdCtrl_HitTest(HWND kybdCtrl, int x, int y, int *velocity)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int tmpVelocity;

    return kc_HitTest(kybdData, x, y, velocity ? velocity : &tmpVelocity);
}

/*
 * IsKeyDown()
 */
BOOL KybdCtrl_IsKeyDown(HWND kybdCtrl, int key)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);

    return kc_IsKeyFlagSet(kybdData->downKeys, key);
}

/*
 * KeyToText() - Converts a key number to plain text.  Key 0 is C-2.  The
 *               string is null terminated and buf must have room for 5 chars.
 */
_TUCHAR *KybdCtrl_KeyToText(int key, _TUCHAR *buf)
{
    int octave = key / 12 - 2;
    int note = key % 12;
    BOOL isBlack = (note - (note >= 5)) & 1;
    int i = 1;

    note = (((note + (note >= 5)) >> 1) + 2) % 7 + 'A';
    buf[0] = note;
    if (isBlack) {
        buf[i++] = '#';
    }
    if (octave < 0) {
        buf[i++] = '-';
        octave = -octave;
    }
    buf[i++] = octave + '0';
    buf[i++] = '\0';

    return buf;
}

/*
 * Register() - Registers the window class for the control.  Returns true on
 *              success, false on failure.
 */
BOOL KybdCtrl_Register(void)
{
    WNDCLASSEX classInfo;

    kc_cursor = Prog_fingerCursor;
    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) kc_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = KYBD_EXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = NULL;
    classInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = KybdCtrl_className;
    classInfo.hIconSm = NULL;

    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering KybdCtrl control"));
        return FALSE;
    }

    return TRUE;
}

/*
 * BlackKeyIndex() - finds the number of black keys to the left of a key.
 */
int kc_BlackKeyIndex(int key)
{
    int octave = key / 12;
    int note = key % 12;

    return (octave * 5) + ((note - (note > 5)) >> 1);
}

/*
 * DrawKey()
 */
void kc_DrawKey(HWND kybdCtrl, int key, BOOL down)
{
    HDC dC = GetDC(kybdCtrl);
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    POINT verts[8];
    int vertCnt;

    if (key < firstKey || key > lastKey) {
        return;
    }
    SelectPen(dC, Prog_3dShadowPen);
    if (down) {
        SelectBrush(dC, Prog_hiBrush);
    } else if (kc_IsKeyBlack(key)) {
        SelectBrush(dC, Prog_wndTextBrush);
    } else {
        SelectBrush(dC, Prog_wndBrush);
    }
    kc_GetKeyPoly(kybdData, key, verts, &vertCnt);
    Polygon(dC, verts, vertCnt);

    ReleaseDC(kybdCtrl, dC);
}

/*
 * GetKeyPoly() - Calculates the bounding rectangle of a key from its index.
 */
void kc_GetKeyPoly(KYBDDATA *kybdData, int key, POINT verts[8], int *vertCnt)
{
    int whiteCx = kybdData->white.cx;
    int whiteCy = kybdData->white.cy - 1;
    int blackCx = kybdData->black.cx;
    int blackCy = kybdData->black.cy - 1;
    int *blackDeltas = kybdData->blackDeltas;
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    int keyboardDelta = kc_WhiteKeyIndex(firstKey) * whiteCx;
    int whiteKeyIndex = kc_WhiteKeyIndex(key);
    int blackKeyIndex = kc_BlackKeyIndex(key);
    int x, y, x0, y0;
    int octaveLeft = (key / 12) * whiteCx * 7;

    if (kc_IsKeyBlack(key)) {
        x = octaveLeft + blackDeltas[blackKeyIndex % 5] - keyboardDelta;
        y = 0;
        x0 = x + blackCx;
        y0 = blackCy;
    } else {
        x = whiteKeyIndex * whiteCx - keyboardDelta;
        y = 0;
        x0 = x + whiteCx;
        y0 = whiteCy;
        if (key <= lastKey) {
            /* if the key is a D, G, or A */
            int note = key % 12;
            if ((note == 2 || note == 7 || note == 9) && key != firstKey
                    && key != lastKey) {
                /* make an 8 point polygon */
                int blx = octaveLeft + blackDeltas[(blackKeyIndex - 1) % 5]
                    + blackCx - keyboardDelta;
                int brx = octaveLeft + blackDeltas[blackKeyIndex % 5]
                    - keyboardDelta;
                *vertCnt = 8;
                verts[0].x = blx; verts[0].y = y;
                verts[1].x = brx; verts[1].y = y;
                verts[2].x = brx; verts[2].y = blackCy;
                verts[3].x = x0;  verts[3].y = blackCy;
                verts[4].x = x0;  verts[4].y = y0;
                verts[5].x = x;   verts[5].y = y0;
                verts[6].x = x;   verts[6].y = blackCy;
                verts[7].x = blx; verts[7].y = blackCy;
            } else {
                /* make a 6 point polygon */
                *vertCnt = 6;
                if (note == 0 || note == 5 || key == firstKey) {
                    int brx = octaveLeft + blackDeltas[blackKeyIndex % 5]
                        - keyboardDelta;
                    if (key == lastKey) {
                        goto GUESS_I_WANT_A_RECTANGLE_AFTER_ALL;
                    }
                    verts[0].x = x;   verts[0].y = y;
                    verts[1].x = brx; verts[1].y = y;
                    verts[2].x = brx; verts[2].y = blackCy;
                    verts[3].x = x0;  verts[3].y = blackCy;
                    verts[4].x = x0;  verts[4].y = y0;
                    verts[5].x = x;   verts[5].y = y0;
                } else {
                    int blx = octaveLeft
                        + blackDeltas[(blackKeyIndex - 1) % 5] + blackCx
                        - keyboardDelta;
                    verts[0].x = blx; verts[0].y = y;
                    verts[1].x = x0;  verts[1].y = y;
                    verts[2].x = x0;  verts[2].y = y0;
                    verts[3].x = x;   verts[3].y = y0;
                    verts[4].x = x;   verts[4].y = blackCy;
                    verts[5].x = blx; verts[5].y = blackCy;
                }
            }
            return;
        }
    }
GUESS_I_WANT_A_RECTANGLE_AFTER_ALL:
    *vertCnt = 4;
    verts[0].x = x;  verts[0].y = y;
    verts[1].x = x0; verts[1].y = y;
    verts[2].x = x0; verts[2].y = y0;
    verts[3].x = x;  verts[3].y = y0;
}

/*
 * HitTest()
 */
int kc_HitTest(KYBDDATA *kybdData, int x, int y, int *velocity)
{
    int whiteCx = kybdData->white.cx;
    int whiteCy = kybdData->white.cy;
    int blackCx = kybdData->black.cx;
    int blackCy = kybdData->black.cy;
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    int keyboardDelta = kc_WhiteKeyIndex(firstKey) * whiteCx;
    DWORD *disabledKeys = kybdData->disabledKeys;
    int whiteKey;
    int key;

    /*
     * Find velocity based on the inverse of the y-position clicked.
     */
    *velocity = y < 0 ? 0 : y > whiteCy ? 127 : 127 * y / whiteCy;

    /*
     * Find which white key the mouse is over.
     */
    whiteKey = (x + keyboardDelta) / whiteCx;
    
    /*
     * See if the mouse is outside the black key's vertical area.
     */
    if (y > blackCy) {
        int whiteNote = whiteKey % 7;

        key = whiteKey + (whiteKey / 7 * 5) + whiteNote - (whiteNote > 2);
        return kc_IsKeyFlagSet(disabledKeys, key) ? -1 : key;
    }

    /*
     * Hit test the black keys.
     */
    key = kc_WhiteKeyToKey(whiteKey);
    if (kc_IsKeyBlack(key - 1)) {
        int octave = whiteKey / 7;
        int blackKeyLeft = octave * (whiteCx * 7)
            + kybdData->blackDeltas[kc_BlackKeyIndex(key - 1) % 5]
            - keyboardDelta;

        if (x >= blackKeyLeft && x < blackKeyLeft + blackCx
                && key - 1 >= firstKey) {
            key--;
            return kc_IsKeyFlagSet(disabledKeys, key) ? -1 : key;
        }
    }
    if (kc_IsKeyBlack(key + 1)) {
        int octave = whiteKey / 7;
        int blackKeyLeft = octave * (whiteCx * 7)
            + kybdData->blackDeltas[kc_BlackKeyIndex(key + 1) % 5]
            - keyboardDelta;

        if (x >= blackKeyLeft && x < blackKeyLeft + blackCx
                && key + 1 <= lastKey) {
            key++;
            return kc_IsKeyFlagSet(disabledKeys, key) ? -1 : key;
        }
    }
    return kc_IsKeyFlagSet(disabledKeys, key) ? -1 : key;
}

BOOL kc_IsKeyBlack(int key)
{
    key %= 12;
    return (key - (key >= 5)) & 1;
}

BOOL kc_IsKeyFlagSet(DWORD keyFlags[4], int key)
{
    return (keyFlags[key / 32] & (1 << (31 - (key & 0x1F)))) != 0;
}

/*
 * OnClose()
 */
void kc_OnClose(HWND kybdCtrl)
{
    PostMessage(GetParent(kybdCtrl), WM_COMMAND
            , MAKEWPARAM(GetWindowLong(kybdCtrl, GWL_ID), KCN_CLOSED)
            , (LPARAM) kybdCtrl);
}

/*
 * OnCreate()
 */
BOOL kc_OnCreate(HWND kybdCtrl, LPCREATESTRUCT createStruct)
{
    KYBDDATA *kybdData;
    RECT clientRect;
    int i;

    kybdData = malloc(sizeof *kybdData);
    if (!kybdData) {
        Error_OnError(E_MALLOC_ERROR);
        return FALSE;
    }
    SetWindowLong(kybdCtrl, GWL_KYBDDATA, (long) kybdData);

    GetClientRect(kybdCtrl, &clientRect);
    kybdData->white.cy = clientRect.bottom;                   /* 5 5/16  134mm  */
#define ENGLISH
#ifdef METRIC
    kybdData->white.cx = kybdData->white.cy * 23 / 134;           /* 15/16   23mm   */
    kybdData->black.cy = kybdData->white.cy * 87 / 134;           /* 3 7/16  87mm   */
    kybdData->black.cx = kybdData->white.cx * 14 / 23;            /* 9/16    14mm   */
    kybdData->blackDeltas[0] = kybdData->white.cx * 12 / 23;      /* 1/2     12mm   */
    kybdData->blackDeltas[1] = kybdData->white.cx * 41 / 23;      /* 1 5/8   41mm   */
    kybdData->blackDeltas[2] = kybdData->white.cx * 82 / 23;      /* 3 1/4   82mm   */
    kybdData->blackDeltas[3] = kybdData->white.cx * 108 / 23;     /* 4 9/32  108mm  */
    kybdData->blackDeltas[4] = kybdData->white.cx * 134 / 23;     /* 5 19/64 134mm  */
#else
    kybdData->white.cx = kybdData->white.cy * 3 / 17;             /* 15/16   23mm   */
    kybdData->black.cy = kybdData->white.cy * 11 / 17;            /* 3 7/16  87mm   */
    kybdData->black.cx = kybdData->white.cx * 3 / 5;              /* 9/16    14mm   */
    kybdData->blackDeltas[0] = kybdData->white.cx * 17 / 30 + 1;   /* 1/2     12mm   */
    kybdData->blackDeltas[1] = kybdData->white.cx * 26 / 15 + 1;  /* 1 5/8   41mm   */
    kybdData->blackDeltas[2] = kybdData->white.cx * 52 / 15 + 2;  /* 3 1/4   82mm   */
    kybdData->blackDeltas[3] = kybdData->white.cx * 137 / 30 + 2; /* 4 9/32  108mm  */
    kybdData->blackDeltas[4] = kybdData->white.cx * 113 / 20 + 2; /* 5 19/64 134mm  */
#endif
    for (i = 0; i < 4; i++) {
        kybdData->downKeys[i] = 0;
        kybdData->disabledKeys[i] = 0;
    }
    kybdData->lastKeyChanged = -1;
    kybdData->sustain = FALSE;
    kybdData->focusVictim = NULL;

    return TRUE;
}

/*
 * OnDestroy()
 */
void kc_OnDestroy(HWND kybdCtrl)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);

    free(kybdData);
}

/*
 * OnEnableRange() - Enables or disables key ranges.
 */
void kc_OnEnableRange(HWND kybdCtrl, int firstKey, int lastKey, BOOL enable)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    DWORD *flags = kybdData->disabledKeys;
    int i;

    if (enable) {
        for (i = firstKey; i <= lastKey; i++) {
            kc_ResetKeyFlag(flags, i);
        }
    } else {
        for (i = firstKey; i <= lastKey; i++) {
            kc_SetKeyFlag(flags, i);
        }
    }
}

/*
 * OnGetLastChanged() - Returns the last key whose status was changed by a
 *                      mouse click or release of the shift key.
 */
DWORD kc_OnGetLastChanged(HWND kybdCtrl)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    return MAKELRESULT(kybdData->lastKeyChanged, kybdData->lastVelocity);
}

/*
 * OnGetRange() - Returns the key range packed in a long.
 */
long kc_OnGetRange(HWND kybdCtrl)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    return MAKELRESULT(kybdData->firstKey, kybdData->keyCnt);
}

/*
 * OnKey()
 */
void kc_OnKey(HWND kybdCtrl, UINT vk, BOOL down, int repeat, UINT flags)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    HWND focusVictim = kybdData->focusVictim;

    if (!down && (vk == VK_SHIFT || vk == VK_RSHIFT)) {
        /*
         * Released all sustained keys.
         */
        kc_ReleaseAllKeys(kybdCtrl);

        kybdData->sustain = FALSE;
        ReleaseCapture();

        /*
         * Return focus to its original owner.
         */
        if (focusVictim) {
            SetFocus(focusVictim);
            kybdData->focusVictim = NULL;
        }
    }
}

/*
 * OnLButtonDown()
 */
void kc_OnLButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y, UINT keyFlags)
{
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    int key;
    int velocity;

    /*
     * Grab the mouse input.
     */
    SetCapture(kybdCtrl);

    /*
     * Check for out-of-rangeness.
     */
    if ((key = kc_HitTest(kybdData, x, y, &velocity)) < firstKey
            || key > lastKey) {
        return;
    }

    /*
     * If the key is already down and sustain isn't activated, release the
     * key before playing it again.
     */
    if (!kybdData->sustain && kc_IsKeyFlagSet(kybdData->downKeys, key)) {
        kc_DrawKey(kybdCtrl, key, FALSE);
        kybdData->lastKeyChanged = key;
        kybdData->lastVelocity = velocity;
        SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
        kc_ResetKeyFlag(kybdData->downKeys, key);
    }

    /*
     * Play the key.
     */
    kc_DrawKey(kybdCtrl, key, TRUE);
    kybdData->lastKeyChanged = key;
    kybdData->lastVelocity = velocity;
    SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYDOWN);
    kc_SetKeyFlag(kybdData->downKeys, key);
}

/*
 * OnLButtonUp()
 */
void kc_OnLButtonUp(HWND kybdCtrl, int x, int y, UINT keyFlags)
{
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int lastKeyChanged = kybdData->lastKeyChanged;

    /*
     * Do nothing if sustain is activated.
     */
    if (!kybdData->sustain) {
        /*
         * If the shift key is being held when the mouse button is down, grab
         * the input focus so sustain can be released with the shift key.
         */
        if (keyFlags & MK_SHIFT) {
            kybdData->sustain = TRUE;
            if (GetFocus() != kybdCtrl) {
                kybdData->focusVictim = SetFocus(kybdCtrl);
            }
        }

        if (!kybdData->sustain) {
            /*
             * Release the key.
             */
            kc_DrawKey(kybdCtrl, lastKeyChanged, FALSE);
            SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
            kc_ResetKeyFlag(kybdData->downKeys, lastKeyChanged);
            kybdData->lastKeyChanged = -1;
            /*
             * Give up mouse input.
             */
            ReleaseCapture();
        }
    }
}

/*
 * OnMouseMove()
 */
void kc_OnMouseMove(HWND kybdCtrl, int x, int y, UINT keyFlags)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int velocity;

    /*
     * If the mouse button isn't down.
     */
    if (keyFlags & MK_LBUTTON) {
        UINT ctrlID = GetDlgCtrlID(kybdCtrl);
        HWND parentWnd = GetParent(kybdCtrl);
        int firstKey = kybdData->firstKey;
        int lastKey = firstKey + kybdData->keyCnt - 1;
        int lastKeyChanged = kybdData->lastKeyChanged;
        RECT clientRect;
        int key;

        /*
         * If the shift key was down for this message, activate sustain and
         * grab the input focus.
         */
        if (!kybdData->sustain) {
            if (keyFlags & MK_SHIFT) {
                kybdData->sustain = TRUE;
                if (GetFocus() != kybdCtrl) {
                    kybdData->focusVictim = SetFocus(kybdCtrl);
                }
            }
        }
        /*
         * Find the key the cursor is pointing to.
         */
        key = kc_HitTest(kybdData, x, y, &velocity);

        /*
         * Do nothing if there was no change from the last mouse message.
         */
        if (key == lastKeyChanged)
            return;

        /*
         * Release the previous key, if sustain isn't activated.
         */
        if (lastKeyChanged > -1 && !kybdData->sustain) {
            kc_DrawKey(kybdCtrl, lastKeyChanged, FALSE);
            SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
            kc_ResetKeyFlag(kybdData->downKeys, lastKeyChanged);
            kybdData->lastKeyChanged = -1;
        }

        /*
         * Do a little range checking on the new key.
         */
        GetClientRect(kybdCtrl, &clientRect);
        if (x < 0 || x >= clientRect.right)
            return;
        if (key < firstKey || key > lastKey)
            return;

        /*
         * Play the new key.
         */
        kc_DrawKey(kybdCtrl, key, TRUE);
        kybdData->lastKeyChanged = key;
        kybdData->lastVelocity = velocity;
        SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYDOWN);
        kc_SetKeyFlag(kybdData->downKeys, key);
    } else {
        if (kc_HitTest(kybdData, x, y, &velocity) == -1) {
            kc_cursor = Prog_arrowCursor;
        } else {
            kc_cursor = Prog_fingerCursor;
        }
        SetCursor(kc_cursor);
    }
}

/*
 * OnPaint()
 */
void kc_OnPaint(HWND kybdCtrl)
{
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(kybdCtrl, &paint);
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    DWORD *disabledKeys = kybdData->disabledKeys;
    DWORD *downKeys = kybdData->downKeys;
    POINT verts[8];
    int vertCnt;
    int key;

    /*
     * Draw disabled keys first.
     */
    SelectBrush(paintDC, Prog_dlgBrush);
    SelectPen(paintDC, Prog_dlgPen);
    for (key = firstKey; key <= lastKey; key++) {
        if (kc_IsKeyFlagSet(disabledKeys, key)) {
            kc_GetKeyPoly(kybdData, key, verts, &vertCnt);
            Polygon(paintDC, verts, vertCnt);
        }
    }
    /*
     * Draw enabled keys.
     */
    SelectPen(paintDC, Prog_3dLightPen);
    for (key = firstKey; key <= lastKey; key++) {
        if (!kc_IsKeyFlagSet(disabledKeys, key)) {
            if (kc_IsKeyFlagSet(downKeys, key)) {
                SelectBrush(paintDC, Prog_hiBrush);
            } else if (kc_IsKeyBlack(key)) {
                SelectBrush(paintDC, Prog_wndTextBrush);
            } else {
                SelectBrush(paintDC, Prog_wndBrush);
            }
            kc_GetKeyPoly(kybdData, key, verts, &vertCnt);
            Polygon(paintDC, verts, vertCnt);
        }
    }

    EndPaint(kybdCtrl, &paint);
}

void kc_OnPushKey(HWND kybdCtrl, int key, int velocity)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);

    kc_DrawKey(kybdCtrl, key, TRUE);
    SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYDOWN);
    kc_SetKeyFlag(kybdData->downKeys, key);
}

/*
 * OnRButtonDown()
 */
void kc_OnRButtonDown(HWND kybdCtrl, BOOL dblClick, int x, int y, UINT keyFlags)
{
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    int key;
    int velocity;

    /*
     * Check for out-of-rangeness.
     */
    if ((key = kc_HitTest(kybdData, x, y, &velocity)) < firstKey
            || key > lastKey) {
        return;
    }

    if (kc_IsKeyFlagSet(kybdData->downKeys, key)) {
        /*
         * Release the key.
         */
        kc_DrawKey(kybdCtrl, key, FALSE);
        kybdData->lastKeyChanged = key;
        kybdData->lastVelocity = velocity;
        SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
        kc_ResetKeyFlag(kybdData->downKeys, key);
    } else {
        /*
         * Play the key.
         */
        kc_DrawKey(kybdCtrl, key, TRUE);
        kybdData->lastKeyChanged = key;
        kybdData->lastVelocity = velocity;
        SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYDOWN);
        kc_SetKeyFlag(kybdData->downKeys, key);
    }
}

void kc_OnReleaseKey(HWND kybdCtrl, int key, int velocity)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);

    kc_ResetKeyFlag(kybdData->downKeys, key);
    kc_DrawKey(kybdCtrl, key, FALSE);
    SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
}

/*
 * OnSetCursor()
 */
BOOL kc_OnSetCursor(HWND kybdCtrl, HWND cursorWnd, UINT codeHitTest, UINT msg)
{
    SetCursor(kc_cursor);

    return TRUE;
}

/*
 * OnSetRange() - Sets the size of the kybdData in terms of the number of keys.
 */
void kc_OnSetRange(HWND kybdCtrl, int firstKey, int keyCnt)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);

    kybdData->firstKey = firstKey;
    kybdData->keyCnt = keyCnt;
}

/*
 * ReleaseAllKeys()
 */
void kc_ReleaseAllKeys(HWND kybdCtrl)
{
    KYBDDATA *kybdData = (KYBDDATA *) GetWindowLong(kybdCtrl, GWL_KYBDDATA);
    UINT ctrlID = GetWindowLong(kybdCtrl, GWL_ID);
    HWND parentWnd = (HWND) GetWindowLong(kybdCtrl, GWL_HWNDPARENT);
    int firstKey = kybdData->firstKey;
    int lastKey = firstKey + kybdData->keyCnt - 1;
    DWORD *downKeys = kybdData->downKeys;
    int key;

    for (key = firstKey; key <= lastKey; key++) {
        if (kc_IsKeyFlagSet(downKeys, key)) {
            kybdData->lastKeyChanged = key;
            kc_DrawKey(kybdCtrl, key, FALSE);
            SendNotification(parentWnd, ctrlID, kybdCtrl, KCN_KEYUP);
            kc_ResetKeyFlag(downKeys, key);
        }
    }
    kybdData->lastKeyChanged = -1;
}

/*
 * ResetKeyFlag() - Clear a key down flag.
 */
void kc_ResetKeyFlag(DWORD keyFlags[4], int key)
{
    keyFlags[key / 32] &= ~(1 << (31 - (key & 0x1F)));
}

/*
 * SetKeyFlag() - Set a key down flag.
 */
void kc_SetKeyFlag(DWORD keyFlags[4], int key)
{
    keyFlags[key / 32] |= 1 << (31 - (key & 0x1F));
}

/*
 * WhiteKeyIndex() - finds the number of white keys to the left of a key.
 */
int kc_WhiteKeyIndex(int key)
{
    int octave = key / 12;
    int note = key % 12;

    return key - (octave * 5) - ((note - (note > 5)) >> 1);
}

/*
 * WhiteKeyToKey() - Converts a white key index to a full key index (the
 *                   inverse of WhiteKeyIndex().
 */
int kc_WhiteKeyToKey(int whiteKey)
{
    int octave = whiteKey / 7;
    int note = whiteKey % 7;

    return octave * 12 + (note << 1) - (note >= 3);
}

/*
 * WndProc()
 */
LRESULT CALLBACK kc_WndProc(HWND kybdCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(kybdCtrl, KCM_ENABLERANGE, kc_OnEnableRange);
        HANDLE_MSG(kybdCtrl, KCM_GETLASTCHANGED, kc_OnGetLastChanged);
        HANDLE_MSG(kybdCtrl, KCM_GETRANGE, kc_OnGetRange);
        HANDLE_MSG(kybdCtrl, KCM_PUSHKEY, kc_OnPushKey);
        HANDLE_MSG(kybdCtrl, KCM_RELEASEKEY, kc_OnReleaseKey);
        HANDLE_MSG(kybdCtrl, KCM_RELEASEALLKEYS, kc_ReleaseAllKeys);
        HANDLE_MSG(kybdCtrl, KCM_SETRANGE, kc_OnSetRange);
        HANDLE_MSG(kybdCtrl, WM_CLOSE, kc_OnClose);
        HANDLE_MSG(kybdCtrl, WM_CREATE, kc_OnCreate);
        HANDLE_MSG(kybdCtrl, WM_DESTROY, kc_OnDestroy);
        HANDLE_MSG(kybdCtrl, WM_KEYDOWN, kc_OnKey);
        HANDLE_MSG(kybdCtrl, WM_KEYUP, kc_OnKey);
        HANDLE_MSG(kybdCtrl, WM_LBUTTONDOWN, kc_OnLButtonDown);
        HANDLE_MSG(kybdCtrl, WM_LBUTTONUP, kc_OnLButtonUp);
        HANDLE_MSG(kybdCtrl, WM_MOUSEMOVE, kc_OnMouseMove);
        HANDLE_MSG(kybdCtrl, WM_PAINT, kc_OnPaint);
        HANDLE_MSG(kybdCtrl, WM_RBUTTONDOWN, kc_OnRButtonDown);
        HANDLE_MSG(kybdCtrl, WM_SETCURSOR, kc_OnSetCursor);
    }
    return DefWindowProc(kybdCtrl, message, wParam, lParam);
}

