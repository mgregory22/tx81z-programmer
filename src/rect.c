/*
 * rect.c - rectangle functions
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
extern __inline long Rect_Area(const RECT *rect);
extern __inline POINT Rect_Center(const RECT *rect);
extern void Rect_CenterRect(RECT *moving, const RECT *stationary
        , CENTERRECT flags);
extern __inline void Rect_Clear(RECT *rect);
extern __inline void Rect_Draw(HDC dc, const RECT *rect);
extern __inline void Rect_Expand(RECT *rect1, const RECT *rect2);
extern __inline void Rect_ExpandPt(RECT *rect, POINT pt);
extern __inline void Rect_Fill(HDC dc, const RECT *rect);
extern __inline void Rect_FillXor(HDC dc, const RECT *rect);
extern void Rect_FitToDesktop(RECT *wndRect);
extern __inline void Rect_Focus(HDC dC, RECT *rect);
extern __inline void Rect_Frame(HDC dc, const RECT *rect);
extern __inline long Rect_HCenter(const RECT *rect);
extern __inline long Rect_Height(const RECT *rect);
extern __inline void Rect_IncSize(RECT *rect);
extern __inline int Rect_Perimeter(const RECT *rect);
extern __inline BOOL Rect_PtIn(const RECT *rect, POINT pt);
extern __inline BOOL Rect_PtIsAbove(const RECT *rect, int x, int y);
extern __inline BOOL Rect_PtIsBelow(const RECT *rect, int x, int y);
extern __inline BOOL Rect_PtIsToTheLeft(const RECT *rect, int x, int y);
extern __inline BOOL Rect_PtIsToTheRight(const RECT *rect, int x, int y);
extern BOOL Rect_RectIn(const RECT *rect1, const RECT *rect2);
extern __inline void Rect_SetFromPoints(RECT *rect, POINT p1, POINT p2);
extern __inline void Rect_SetInflated(RECT *dest, const RECT *src, int dx, int dy);
extern void Rect_SetNormalFromInts(RECT *rect, int x, int y, int x0, int y0);
extern __inline void Rect_DecSize(RECT *rect);
extern __inline long Rect_VCenter(const RECT *rect);
extern __inline long Rect_Width(const RECT *rect);


/*
 * Procedure definitions
 */

void Rect_CenterRect(RECT *moving, const RECT *stationary, CENTERRECT flags)
{
    if (flags & CR_HORIZONTAL) {
        int mWidth = moving->right - moving->left;
        int sWidth = stationary->right - stationary->left;
        moving->left = stationary->left + ((sWidth - mWidth) >> 1);
		moving->right = moving->left + mWidth;
    }
    if (flags & CR_VERTICAL) {
        int mHeight = moving->bottom - moving->top;
        int sHeight = stationary->bottom - stationary->top;
        moving->top = stationary->top + ((sHeight - mHeight) >> 1);
		moving->bottom = moving->top + mHeight;
    }
}

void Rect_FitToDesktop(RECT *wndRect)
{
    RECT deskRect;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);

    if (wndRect->left < 0) {
        wndRect->right -= wndRect->left;
        wndRect->left = 0;
    }
    if (wndRect->top < 0) {
        wndRect->bottom -= wndRect->top;
        wndRect->top = 0;
    }
    if (wndRect->right > deskRect.right) {
        int d = wndRect->right - deskRect.right;
        wndRect->right -= d;
        wndRect->left -= d;
    }
    if (wndRect->bottom > deskRect.bottom) {
        int d = wndRect->bottom - deskRect.bottom;
        wndRect->bottom -= d;
        wndRect->top -= d;
    }
}

BOOL Rect_RectIn(const RECT *rect1, const RECT *rect2)
{
    if (rect1->left <= rect2->left
            && rect1->right >= rect2->right
            && rect1->top <= rect2->top
            && rect1->bottom >= rect2->bottom)
        return TRUE;
    return FALSE;
}

void Rect_SetNormalFromInts(RECT *rect, int x, int y, int x0, int y0)
{
    if (x <= x0) {
        rect->left = x;
        rect->right = x0;
    } else {
        rect->left = x0;
        rect->right = x;
    }
    if (y <= y0) {
        rect->top = y;
        rect->bottom = y0;
    } else {
        rect->top = y0;
        rect->bottom = y;
    }
}

