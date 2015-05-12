/*
 * egctrl.c - Envelope generator display for TX81Z
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
#include "egctrl.h"
#include "minifont.h"
#include "prog.h"
#include "resource.h"

/*
 * Types
 */
typedef struct {
    char *valuePtr;
    char *shiftPtr;
    int origin;
    int len;
    int keyUp;
    BOOL hideLabels;
} EGDATA;

typedef struct {
    HDC dC;
    int keyUpX;
    BOOL steep;
} LINEDDAINFO;

/*
 * Global constants
 */
const _TUCHAR *EGCtrl_className = _T("EGCtrl");

/*
 * Global procedures
 */
extern BOOL EGCtrl_Register(void);

/*
 * Unit constants
 */
#define EG_EXTRA    4
#define GWL_EGDATA  0
static const int ec_decayAngleRadiuses[11] = {
      0,  /* D1R = 0 (not drawn) */
    340,  /* D1R = 1  +57 */
    240,  /* D1R = 2  +47 */
    175,  /* D1R = 3  +38 */
    114,  /* D1R = 4  +30 */
     78,  /* D1R = 5  +23 */
     54,  /* D1R = 6  +17 */
     37,  /* D1R = 7  +12 */
     25,  /* D1R = 8   +5 */
     20,  /* D1R = 9   +5 */
     15   /* D1R = 10 */
};
static const int ec_releaseAngleRadiuses[7] = {
      0,  /* RR = 0 (not drawn) */
    121,  /* RR = 1  +47 */
     74,  /* RR = 2  +30 */
     44,  /* RR = 3  +17 */
     27,  /* RR = 4   +7 */
     20,  /* RR = 5   +5 */
     15,  /* RR = 6 */
};

/*
 * Unit procedures
 */
static int ec_GetARRateLength(int setting, int height, int origHeight);
static int ec_GetDRRateLength(int setting, int height, int origHeight);
static int ec_GetRRRateLength(int setting, int height, int origHeight);
static void CALLBACK ec_LineFunc(int x, int y, LPARAM lParam);
static void CALLBACK ec_CalcKeyUpY(int x, int y, LPARAM lParam);
static BOOL ec_OnCreate(HWND egCtrl, LPCREATESTRUCT createStruct);
static void ec_OnDestroy(HWND egCtrl);
static int ec_OnGetKeyUp(HWND egCtrl);
static int ec_OnGetLength(HWND egCtrl);
static void ec_OnGetPtrs(HWND egCtrl, char **valuePtrPtr, char **shiftPtrPtr);
static void ec_OnLButtonDown(HWND egCtrl, BOOL dblClick, int x, int y, UINT keyFlags);
static void ec_OnLButtonUp(HWND egCtrl, int x, int y, UINT keyFlags);
static void ec_OnMouseMove(HWND egCtrl, int x, int y, UINT keyFlags);
static void ec_OnPaint(HWND egCtrl);
static void ec_OnSetKeyUp(HWND egCtrl, int keyUp);
static void ec_OnSetOrigin(HWND egCtrl, int origin);
static void ec_OnSetPtrs(HWND egCtrl, char *valuePtr, char *shiftPtr);
static void ec_OnHideLabels(HWND egCtrl, BOOL hide);
static void ec_OnSize(HWND egCtrl, UINT state, int cx, int cy);
static LRESULT CALLBACK ec_WndProc(HWND egCtrl, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static int ec_dashTrack;
#define ec_dashLen  6
#define ec_halfDashLen 3
static int ec_keyUpY;


/*
 * Procedure definitions
 */

/*
 * Register() - Registers the window class for the control.  Returns true on
 *              success, false on failure.
 */
BOOL EGCtrl_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) ec_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = EG_EXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = NULL;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) NULL;
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = EGCtrl_className;
    classInfo.hIconSm = NULL;

    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering EGCtrl control"));
        return FALSE;
    }
    return TRUE;
}

#define RR_ADJUSTMENT(setting) (4. + ((setting) - 1.) * 10. / 7.)
#define AR_FACTOR 1.298
#define DR_FACTOR 1.41

/* disable double to int conversion warnings for the Get*RateLength() funcs */
#pragma warning(push)
#pragma warning(disable:4244)
/*
 * GetRateLength() - Calculates the width of an EG stage.
 */
int ec_GetARRateLength(int setting, int height, int origHeight)
{
    return (int) pow(AR_FACTOR, 26 - setting);
}

int ec_GetDRRateLength(int setting, int height, int origHeight)
{
    return (int) pow(DR_FACTOR, 26 - setting) * ((double) height / (double) origHeight);
}

int ec_GetRRRateLength(int setting, int height, int origHeight)
{
    return (int) pow(DR_FACTOR, 26 - RR_ADJUSTMENT(setting))
        * ((double) height / (double) origHeight);
}
#pragma warning(pop)

/*
 * LineFunc() - Callback for LinaDDA() -- Draws the EG envelope.
 */
void CALLBACK ec_LineFunc(int x, int y, LPARAM lParam)
{
#define lineDDAInfo ((LINEDDAINFO *) lParam)
    HDC dC = lineDDAInfo->dC;

    if (x < lineDDAInfo->keyUpX || ec_dashTrack % ec_dashLen < ec_halfDashLen) {
        if (lineDDAInfo->steep) {
            SetPixelV(dC, x, y, Prog_hiColor);
            SetPixelV(dC, x + 1, y, Prog_hiColor);
        } else {
            SetPixelV(dC, x, y, Prog_hiColor);
            SetPixelV(dC, x, y + 1, Prog_hiColor);
        }
    }
    ec_dashTrack++;
#undef lineDDAInfo
}

/*
 * CalcKeyUpY() - Callback for LinaDDA() to grab the keyUpY value from LineDDA,
 *                since I don't know how to calculate it correctly.
 */
void CALLBACK ec_CalcKeyUpY(int x, int y, LPARAM lParam)
{
    if (x == lParam) {
        ec_keyUpY = y;
    }
}

/*
 * OnCreate()
 */
BOOL ec_OnCreate(HWND egCtrl, LPCREATESTRUCT createStruct)
{
    EGDATA *egData = malloc(sizeof *egData);

    if (!egData) {
        Error_OnError(E_MALLOC_ERROR);
        return FALSE;
    }
    egData->valuePtr = NULL;
    egData->shiftPtr = NULL;
    egData->origin = 0;
    egData->len = 0;
    egData->keyUp = createStruct->cx / 2;
    egData->hideLabels = FALSE;
    SetWindowLong(egCtrl, GWL_EGDATA, (DWORD) egData);

    return TRUE;
}

/*
 * OnDestroy()
 */
void ec_OnDestroy(HWND egCtrl)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    free(egData);
}

int ec_OnGetKeyUp(HWND egCtrl)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    return egData->keyUp;
}

int ec_OnGetLength(HWND egCtrl)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    return egData->len;
}

void ec_OnGetPtrs(HWND egCtrl, char **valuePtrPtr, char **shiftPtrPtr)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    *valuePtrPtr = egData->valuePtr;
    *shiftPtrPtr = egData->shiftPtr;
}

/*
 * OnLButtonDown()
 */
void ec_OnLButtonDown(HWND egCtrl, BOOL dblClick, int x, int y, UINT keyFlags)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    egData->keyUp = x + egData->origin;
    InvalidateRect(egCtrl, NULL, FALSE);
    SetCapture(egCtrl);
    PostNotification(GetParent(egCtrl), GetDlgCtrlID(egCtrl), egCtrl
            , ECN_KEYUPCHANGED);
}

/*
 * OnLButtonUp()
 */
void ec_OnLButtonUp(HWND egCtrl, int x, int y, UINT keyFlags)
{
    ReleaseCapture();
}

/*
 * OnMouseMove()
 */
void ec_OnMouseMove(HWND egCtrl, int x, int y, UINT keyFlags)
{
    if (keyFlags & MK_LBUTTON) {
        EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

        egData->keyUp = x + egData->origin;
        InvalidateRect(egCtrl, NULL, FALSE);
        PostNotification(GetParent(egCtrl), GetDlgCtrlID(egCtrl), egCtrl
                , ECN_KEYUPCHANGED);
    }
}

/*
 * OnPaint()
 */
void ec_OnPaint(HWND egCtrl)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);
    DWORD style = GetWindowStyle(egCtrl);
    PAINTSTRUCT paint;
    HDC paintDC = BeginPaint(egCtrl, &paint);
    RECT clientRect;
    char *values = egData->valuePtr;
    int ar = values[0];
    int d1r = values[1];
    int d2r = values[2];
    int rr = values[3];
    int d1l = values[4];
    int arW, d1rW, d2rW, rrW, d1lH;
    int shift = *(egData->shiftPtr);
    int shiftedB;
    int shiftedH;
    int keyUpX = egData->keyUp > 1 ? egData->keyUp : 1;
    int keyUpY;
    BOOL doDrawKeyUp = TRUE;
    HPEN oldPen = (HPEN) GetCurrentObject(paintDC, OBJ_PEN);
    HPEN dottedPen = CreatePen(PS_DOT, 1, Prog_wndTextColor);
    int w, h;
    int lft, rgt, top, btm;
    int logRgt;
    int r;
    int egLen;
    HRGN clipRgn;
    LINEDDAINFO lineDDAInfo = { paintDC, keyUpX };
    int curPosX, curPosY;

    GetClientRect(egCtrl, &clientRect);
    SelectBrush(paintDC, Prog_wndBrush);
    SelectPen(paintDC, Prog_wndTextPen);
    Rectangle(paintDC, PASS_RECT_FIELDS(clientRect));

    lft = clientRect.left + 1;
    top = clientRect.top + 1;
    rgt = clientRect.right - 1;
    btm = clientRect.bottom - 1;
    logRgt = rgt + egData->origin;
    clipRgn = CreateRectRgn(lft, top, rgt, btm);
    SelectClipRgn(paintDC, clipRgn);

    w = rgt - lft;
    h = btm - top;
    shiftedB = btm - (shift * (h >> 2));
    shiftedH = shiftedB - top;
    arW = ec_GetARRateLength(ar, shiftedH, btm) + lft;
    d1lH = top + (15 - d1l) * shiftedH / 15;
    d1rW = arW + ((d1l == 15) ? 0 : ec_GetDRRateLength(d1r, d1lH, shiftedB));
    d2rW = d1rW + ec_GetDRRateLength(d2r, shiftedH - d1lH, shiftedB);

    /*
     * Calculate the keyUpY value the long and hard way.
     */
    curPosX = lft;
    curPosY = shiftedB;
    ec_keyUpY = -1;
    /*
     * If the attack rate is zero, the whole thing is flat (no sound at all).
     */
    if (ar == 0) {
        ec_keyUpY = shiftedB;
    } else {
        /*
         * AR
         */
        if (keyUpX < arW) {
            LineDDA(curPosX, curPosY, arW, top, ec_CalcKeyUpY, keyUpX);
        }
        if (ec_keyUpY != -1) {
            goto GotKeyUpY;
        }
        curPosX = arW;
        curPosY = top;
        /*
         * D1R
         */
        if (d1r == 0 && d1l == 15) {
            ; /* the D1R line has zero length */
        } else if (d1r == 0) {
            if (keyUpX < d1rW) {
                LineDDA(curPosX, curPosY, keyUpX, top, ec_CalcKeyUpY, keyUpX);
            }
            if (ec_keyUpY != -1) {
                goto GotKeyUpY;
            }
            ec_keyUpY = top;
            goto GotKeyUpY;
        } else {
            if (keyUpX < d1rW) {
                LineDDA(curPosX, curPosY, d1rW, d1lH, ec_CalcKeyUpY, keyUpX);
            }
            if (ec_keyUpY != -1) {
                goto GotKeyUpY;
            }
            curPosX = d1rW;
            curPosY = d1lH;
        }
        /*
         * D2R
         */
        if (d2r == 0) {
            if (keyUpX < d2rW) {
                LineDDA(curPosX, curPosY, keyUpX, d1lH, ec_CalcKeyUpY, keyUpX);
            }
            if (ec_keyUpY != -1) {
                goto GotKeyUpY;
            }
            ec_keyUpY = d1lH;
        } else {
            if (keyUpX < d2rW) {
                LineDDA(curPosX, curPosY, d2rW, shiftedB, ec_CalcKeyUpY, keyUpX);
            }
        }
        if (ec_keyUpY == -1) {
            ec_keyUpY = shiftedB;
        }
    }
GotKeyUpY:
    keyUpY = ec_keyUpY;
    rrW = keyUpX + ec_GetRRRateLength(rr, shiftedH - keyUpY, shiftedB);
    /*
     * The length of the entire envelope is the longer of the D1R, D2R, or RR.
     */
    /*
     * The virtual length should be set to the longest possible EG length,
     * which should be either the d2rW, or if the slope of the release rate is
     * lower, it should be whatever the length of the EG is when the key up
     * line is at the top.
     *
     * The idea is to try and prevent the virtual size from changing as the
     * user moves the key up line back and forth, because it screws up
     * sometimes and the key up line can end up way over to the right of the
     * mouse pointer.
     */
    egLen = arW + ec_GetRRRateLength(rr, shiftedH, shiftedB);
    if (d2rW > egLen) {
        egLen = d2rW;
    }
    if (d1rW > egLen) {
        egLen = d1rW;
    }
    /*
     * If the key up line is beyond the end of the envelope (and therefore at
     * zero), or the key up level at that position is below the zero point for
     * whatever reason, then don't draw the key up level line or the RR stage.
     */
    if (keyUpX > egLen || keyUpY >= shiftedB) {
        doDrawKeyUp = FALSE;
    }

    SetViewportOrgEx(paintDC, 0, 0, NULL);

    /*
     * Draw the shift line.
     */
    SelectPen(paintDC, Prog_wndTextPen);
    MoveToEx(paintDC, lft, shiftedB + 1, NULL);
    LineTo(paintDC, rgt, shiftedB + 1);
    if (!egData->hideLabels) {
        MiniFont_DrawString(paintDC, rgt - 24, shiftedB + 2, Prog_shftStr, 4
                , Prog_wndTextColor);
    }

    /*
     * Draw the D1L level line.
     */
    SelectPen(paintDC, dottedPen);
    MoveToEx(paintDC, lft, d1lH, NULL);
    LineTo(paintDC, rgt, d1lH);
    if (!egData->hideLabels) {
        MiniFont_DrawString(paintDC, rgt - 20
                , (d1l < 14) ? d1lH - 9 : d1lH + 2, Prog_d1lStr, 3, Prog_wndTextColor);
    }

    SetViewportOrgEx(paintDC, -egData->origin, 0, NULL);

    /*
     * Draw the key up line.
     */
    MoveToEx(paintDC, keyUpX, top, NULL);
    LineTo(paintDC, keyUpX, btm);
    if ((style & ECS_KEYUPLABEL) && !egData->hideLabels) {
        MiniFont_DrawVerticalStringUp(paintDC, keyUpX - 9, 37, _T("Key Up")
                , 6, Prog_wndTextColor);
    }

    /*
     * Draw the key up level line.
     */
    if (doDrawKeyUp && !egData->hideLabels) {
        if (rr < 7) {
            r = ec_releaseAngleRadiuses[rr];
        } else {
            r = 10;
        }
        MoveToEx(paintDC, keyUpX - 20, keyUpY, NULL);
        LineTo(paintDC, keyUpX + r + 6, keyUpY);
    }

    if (!egData->hideLabels) {
        /*
         * Draw the AR angle arc.
         */
        if (ar > 0) {
            SelectPen(paintDC, Prog_wndTextPen);
            r = 10 + ((ar < 6) ? 5 * (6 - ar) : 0);
            Arc(paintDC, lft - r + 1, shiftedB - r, lft + r, shiftedB + r
                    , lft + r, shiftedB, arW, top);
            MiniFont_DrawString(paintDC, lft + r + 3, shiftedB - 9, Prog_arStr
                    , 2, Prog_wndTextColor);
        }

        /*
         * Draw the D1R angle arc.
         */
        if (d1l < 14) {
            if (ar > 0 && d1r > 0 && d1l < 15) {
                if (d1r < 11) {
                    r = ec_decayAngleRadiuses[d1r];
                } else {
                    r = 10;
                }
                Arc(paintDC, arW - r + 1, top - r, arW + r, top + r
                        , d1rW, d1lH, arW + r, top);
                MiniFont_DrawString(paintDC, arW + r + 3, top + 1, Prog_d1rStr
                        , 3, Prog_wndTextColor);
            }
        }

        /*
         * Draw the D2R angle arc.
         */
        if (ar > 0 && (d1r > 0 || d1l == 15) && d2r > 0 && d1l > 0) {
            if (d2r < 11) {
                r = ec_decayAngleRadiuses[d2r];
            } else {
                r = 10;
            }
            Arc(paintDC, d1rW - r + 1, d1lH - r, d1rW + r, d1lH + r
                    , d2rW, shiftedB, d1rW + r, d1lH);
            MiniFont_DrawString(paintDC
                    , d1rW + r + 3
                    , d1lH + 2
                    , Prog_d2rStr, 3, Prog_wndTextColor);
        }

        /*
         * Draw the RR angle arc.
         */
        if (doDrawKeyUp && ar > 0) {
            if (rr < 7) {
                r = ec_releaseAngleRadiuses[rr];
            } else {
                r = 12;
            }
            Arc(paintDC, keyUpX - r + 1, keyUpY - r, keyUpX + r, keyUpY + r
                    , rrW, shiftedB, keyUpX + r, keyUpY);
            MiniFont_DrawString(paintDC
                    , keyUpX + r + 3
                    , keyUpY + 2
                    , Prog_rrStr, 2, Prog_wndTextColor);
        }
    }

    /*
     * Draw the envelope.
     */
    curPosX = lft;
    curPosY = shiftedB;
    ec_dashTrack = 0;
    /*
     * If the attack rate is zero, the whole thing is flat (no sound at all).
     */
    if (ar == 0) {
        lineDDAInfo.steep = 0;
        LineDDA(lft, shiftedB - 2, logRgt, shiftedB - 2, ec_LineFunc
                , (LPARAM) &lineDDAInfo);
        egLen = w;
    } else {
        /**
         ** AR
         **/
        /*
         *  Bottom left corner to the top of the control.
         */
        lineDDAInfo.steep = abs(top - curPosY) > abs(arW - curPosX);
        LineDDA(curPosX, curPosY, arW, top - 1, ec_LineFunc
                , (LPARAM) &lineDDAInfo);
        curPosX = arW;
        curPosY = top;
        /**
         ** D1R
         **/
        /*
         * If the D1R rate is zero, and the D1L is 15, then the D1L is reached
         * immediately and there is no drawing to do.
         */
        if (d1r == 0 && d1l == 15) {
            ;
        /*
         * If the D1R is zero, the D1L is never reached, so draw a line
         * straight across the top all the way to the end - none of the other
         * stages are activated.
         */
        } else if (d1r == 0) {
            lineDDAInfo.steep = 0;
            LineDDA(curPosX, curPosY, logRgt, top, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            egLen = arW + w;
            goto EGDONE;
        /*
         * Normally, the D1R starts at the top where the AR left off and goes
         * down to the D1L.
         */
        } else {
            lineDDAInfo.steep = abs(d1lH - curPosY) > abs(d1rW - curPosX);
            LineDDA(curPosX, curPosY, d1rW, d1lH, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            curPosX = d1rW;
            curPosY = d1lH;
        }
        /**
         ** D2R
         **/
        /*
         * If the D2R is zero, then it just goes straight across along the D1L
         * line to the end.
         */
        if (d2r == 0) {
            lineDDAInfo.steep = 0;
            LineDDA(curPosX, curPosY, logRgt, d1lH, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            egLen = d1rW + w;
            curPosX = logRgt;
            curPosY = d1lH;
        /*
         * Normally, the D2R stage starts at the D1L line and falls to zero.
         */
        } else {
            lineDDAInfo.steep = abs(shiftedB - curPosY) > abs(d2rW - curPosX);
            LineDDA(curPosX, curPosY, d2rW, shiftedB, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            curPosX = d2rW;
            curPosY = shiftedB;
            /*
             * Once it reaches zero, I want a line going across the bottom to 
             * indicate its zeroness.
             */
            lineDDAInfo.steep = 0;
            LineDDA(curPosX, curPosY, logRgt, shiftedB, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            curPosX = logRgt;
            curPosY = shiftedB;
        }
EGDONE:;
        /**
         ** RR
         **/
        /*
         * If the key up line isn't off screen to the right somewhere, draw
         * the RR stage from the keyUpX, keyUpY position to the zero level.
         */
        lineDDAInfo.keyUpX = logRgt;
        if (doDrawKeyUp) {
            lineDDAInfo.steep = abs(shiftedB - keyUpY) > abs(rrW - keyUpX);
            LineDDA(keyUpX, keyUpY, rrW, shiftedB, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
            curPosX = rrW;
            curPosY = shiftedB;
        }
        /*
         * Draw a line along the zero line to the end.
         */
        if (curPosX < logRgt) {
            lineDDAInfo.steep = abs(shiftedB - curPosY) > abs(logRgt - curPosX);
            LineDDA(curPosX, curPosY, logRgt, shiftedB, ec_LineFunc
                    , (LPARAM) &lineDDAInfo);
        }
    }

    SelectPen(paintDC, oldPen);
    DeletePen(dottedPen);
    SelectClipRgn(paintDC, NULL);
    DeleteRgn(clipRgn);
    EndPaint(egCtrl, &paint);
    if (egLen != egData->len) {
        egData->len = egLen;
        PostNotification(GetParent(egCtrl), GetDlgCtrlID(egCtrl), egCtrl
                , ECN_LENGTHCHANGED);
    }
}

void ec_OnSetKeyUp(HWND egCtrl, int keyUp)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    egData->keyUp = keyUp;
    InvalidateRect(egCtrl, NULL, FALSE);
}

void ec_OnSetOrigin(HWND egCtrl, int origin)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    egData->origin = origin;
    InvalidateRect(egCtrl, NULL, FALSE);
}

void ec_OnSetPtrs(HWND egCtrl, char *valuePtr, char *shiftPtr)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    egData->valuePtr = valuePtr;
    egData->shiftPtr = shiftPtr;
}

/*
 * OnHideLabels() -
 */
void ec_OnHideLabels(HWND egCtrl, BOOL hide)
{
    EGDATA *egData = (EGDATA *) GetWindowLong(egCtrl, GWL_EGDATA);

    egData->hideLabels = hide;
    InvalidateRect(egCtrl, NULL, FALSE);
}

/*
 * OnSize()
 */
void ec_OnSize(HWND egCtrl, UINT state, int cx, int cy)
{
    InvalidateRect(egCtrl, NULL, FALSE);
}

/*
 * WndProc()
 */
LRESULT CALLBACK ec_WndProc(HWND egCtrl, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(egCtrl, ECM_GETKEYUP, ec_OnGetKeyUp);
        HANDLE_MSG(egCtrl, ECM_GETLENGTH, ec_OnGetLength);
        HANDLE_MSG(egCtrl, ECM_GETPTRS, ec_OnGetPtrs);
        HANDLE_MSG(egCtrl, ECM_SETKEYUP, ec_OnSetKeyUp);
        HANDLE_MSG(egCtrl, ECM_SETORIGIN, ec_OnSetOrigin);
        HANDLE_MSG(egCtrl, ECM_SETPTRS, ec_OnSetPtrs);
        HANDLE_MSG(egCtrl, ECM_HIDELABELS, ec_OnHideLabels);
        HANDLE_MSG(egCtrl, WM_CREATE, ec_OnCreate);
        HANDLE_MSG(egCtrl, WM_DESTROY, ec_OnDestroy);
        HANDLE_MSG(egCtrl, WM_LBUTTONDOWN, ec_OnLButtonDown);
        HANDLE_MSG(egCtrl, WM_LBUTTONUP, ec_OnLButtonUp);
        HANDLE_MSG(egCtrl, WM_MOUSEMOVE, ec_OnMouseMove);
        HANDLE_MSG(egCtrl, WM_PAINT, ec_OnPaint);
        HANDLE_MSG(egCtrl, WM_SIZE, ec_OnSize);
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND parentWnd = GetParent(egCtrl);

            MapWindowPoints(egCtrl, parentWnd, &pt, 1);
            PostMessage(parentWnd, message, wParam, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
    }
    return DefWindowProc(egCtrl, message, wParam, lParam);
}

