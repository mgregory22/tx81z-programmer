/*
 * gui/rect.h - rectangle functions
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
#ifndef GUI_RECT_H
#define GUI_RECT_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

/*
 * Macros
 */
#define RECT_W(rect) ((rect).right - (rect).left)
#define RECT_H(rect) ((rect).bottom - (rect).top)
#define MAKE_RECT_TEMPS(rect) \
    long x = (rect).left, \
         y = (rect).top, \
         x0 = (rect).right, \
         y0 = (rect).bottom
#define PASS_RECT_FIELDS(rect) (rect).left, (rect).top, (rect).right, (rect).bottom
#define PASS_RECT_FIELDS_AS_AREA(rect) (rect).left, (rect).top, (rect).right - (rect).left, (rect).bottom - (rect).top
#define RECT_SET(rect, x, y, x0, y0) \
    rect.left = x; \
    rect.top = y; \
    rect.right = x0; \
    rect.bottom = y0

/*
 * Types
 */
typedef enum {
    CR_HORIZONTAL = 1,
    CR_VERTICAL = 2
} CENTERRECT;


/*
 * Procedures
 */

/*
 * Returns the area of a rectangle.
 */
__inline long Rect_Area(const RECT *rect);

/*
 * Returns the center of a rectangle.
 */
__inline POINT Rect_Center(const RECT *rect);

/*
 * Centers <moving> in or over <stationary>, vertically, horizontally or both
 */
void Rect_CenterRect(RECT *moving, const RECT *stationary, CENTERRECT flags);

/*
 * Sets a rectangle's coordinates to zeros.
 */
__inline void Rect_Clear(RECT *rect);

/*
 * Draws and fills the rectangle with the current pen and brush.
 */
__inline void Rect_Draw(HDC dC, const RECT *rect);

/*
 * Expands rect1 with the coordinates of rect2.  If rect1 is empty, it is
 * set to the coordinates of rect2.
 */
__inline void Rect_Expand(RECT *rect1, const RECT *rect2);

/*
 * Expands rect1 to include the specified point.
 */
__inline void Rect_ExpandPt(RECT *rect, POINT pt);

/*
 * Same as FillRect, but draws the rectangle with the current brush and doesn't
 * exclude the right and bottom pixels.
 */
__inline void Rect_Fill(HDC dC, const RECT *rect);

/*
 * Draws a xor mode filled rectangle.
 */
__inline void Rect_FillXor(HDC dC, const RECT *rect);

/*
 * Moves a rectangle so it fits completely within the desktop work area.
 */
void Rect_FitToDesktop(RECT *wndRect);

/*
 * Draws a focus rectangle.
 */
__inline void Rect_Focus(HDC dC, RECT *rect);

/*
 * Same as FrameRect, but draws the rectangle with the current pen and doesn't
 * exclude the right and bottom pixels.
 */
__inline void Rect_Frame(HDC dC, const RECT *rect);

/*
 * Draws a rectangle outline in NOTXORPEN raster mode.
 */
void Rect_FrameXor(HDC dC, const RECT *rect, HPEN pen);

/*
 * Returns the x-coordinate of the horizontal center of the rectangle.
 */
__inline long Rect_HCenter(const RECT *rect);

/*
 * Returns the height of a rectangle.
 */
__inline long Rect_Height(const RECT *rect);

/*
 * Adjust the rectangle by adding 1 to the right and bottom.
 */
__inline void Rect_IncSize(RECT *rect);

/*
 * Swaps left and right and/or top and bottom if the rect is backwards.
 */
void Rect_Normalize(RECT *rect);

/*
 * Copies a rect, swapping left and right and/or top and bottom if the
 * rect is backwards.
 */
void Rect_NormalizedCopy(RECT *dest, const RECT *src);

/*
 * Returns true if any area of rect1 overlaps with rect2.
 */
BOOL Rect_Overlap(const RECT *rect1, const RECT *rect2);

/*
 * Returns the perimeter of the rectangle.
 */
__inline int Rect_Perimeter(const RECT *rect);

/*
 * Same as PtInRect without the pixel exclusion.
 */
__inline BOOL Rect_PtIn(const RECT *rect, POINT pt);

/*
 * Returns TRUE if the point is directly above the rect.
 */
__inline BOOL Rect_PtIsAbove(const RECT *rect, int x, int y);

/*
 * Returns TRUE if the point is directly below the rect.
 */
__inline BOOL Rect_PtIsBelow(const RECT *rect, int x, int y);

/*
 * Returns TRUE if the point is directly to the left of the rect.
 */
__inline BOOL Rect_PtIsToTheLeft(const RECT *rect, int x, int y);

/*
 * Returns TRUE if the point is directly to the right of the rect.
 */
__inline BOOL Rect_PtIsToTheRight(const RECT *rect, int x, int y);

/*
 * RectIn() - Returns TRUE if rect1 fully encompasses rect2.
 */
BOOL Rect_RectIn(const RECT *rect1, const RECT *rect2);

/*
 * Sets a rectangle from two points.
 */
__inline void Rect_SetFromPoints(RECT *rect, POINT p1, POINT p2);

/*
 * Sets a rectangle by inflating another rectangle.
 */
__inline void Rect_SetInflated(RECT *dest, const RECT *src, int dx, int dy);

/*
 * Sets a normalized rectangle from four coordinates.
 */
void Rect_SetNormalFromInts(RECT *rect, int x, int y, int x0, int y0);

/*
 * Sets a normalized rectangle from two points.
 */
void Rect_SetNormalFromPoints(RECT *rect, POINT p1, POINT p2);

/*
 * Decrements the right and bottom parameters of a RECT by one.
 */
__inline void Rect_DecSize(RECT *rect);

/*
 * Returns the y-coordinate of the vertical center of the rectangle.
 */
__inline long Rect_VCenter(const RECT *rect);

/*
 * Returns the width of a rectangle.
 */
__inline long Rect_Width(const RECT *rect);

/*
 * Returns true if the rectangle contains the point at (x, y).
 */
__inline BOOL Rect_XYIn(const RECT *rect, int x, int y);

/*
 * Aliases for HCenter and VCenter
 */
#define Rect_XCenter Rect_HCenter
#define Rect_YCenter Rect_VCenter

/*
 * Inline definitions
 */
long Rect_Area(const RECT *rect)
{
    return (rect->right - rect->left) * (rect->bottom - rect->top);
}

void Rect_IncSize(RECT *rect)
{
    rect->right++;
    rect->bottom++;
}

POINT Rect_Center(const RECT *rect)
{
    POINT c = {
        (rect->right + rect->left) >> 1,
        (rect->bottom + rect->top) >> 1
    };
    return c;
}

void Rect_Clear(RECT *rect)
{
    memset(rect, 0, sizeof(*rect));
}

void Rect_Draw(HDC dC, const RECT *rect)
{
    int x = rect->left;
    int y = rect->top;
    int x0 = rect->right + 1;
    int y0 = rect->bottom + 1;
    Rectangle(dC, x, y, x0, y0);
}

void Rect_Expand(RECT *rect1, const RECT *rect2)
{
    if (rect1->left == rect1->right || rect1->top == rect1->bottom) {
        *rect1 = *rect2;
        return;
    }
    if (rect2->left < rect1->left)
        rect1->left = rect2->left;
    if (rect2->top < rect1->top)
        rect1->top = rect2->top;
    if (rect2->right > rect1->right)
        rect1->right = rect2->right;
    if (rect2->bottom > rect1->bottom)
        rect1->bottom = rect2->bottom;
}

void Rect_ExpandPt(RECT *rect, POINT pt)
{
    if (pt.x < rect->left)
        rect->left = pt.x;
    if (pt.x > rect->right)
        rect->right = pt.x;
    if (pt.y < rect->top)
        rect->top = pt.y;
    if (pt.y > rect->bottom)
        rect->bottom = pt.y;
}

void Rect_Fill(HDC dC, const RECT *rect)
{
    PatBlt(dC, rect->left, rect->top, Rect_Width(rect), Rect_Height(rect)
            , PATCOPY);
}

void Rect_FillXor(HDC dC, const RECT *rect)
{
    PatBlt(dC, rect->left, rect->top, Rect_Width(rect), Rect_Height(rect)
            , PATINVERT);
}

void Rect_Focus(HDC dC, RECT *rect)
{
    Rect_IncSize(rect);
    DrawFocusRect(dC, rect);
    Rect_DecSize(rect);
}

void Rect_Frame(HDC dC, const RECT *rect)
{
    int x = rect->left, y = rect->top, x0 = rect->right, y0 = rect->bottom;
    POINT verts[5] = {
        { x, y },
        { x, y0 },
        { x0, y0 },
        { x0, y },
        { x, y }
    };
    Polyline(dC, verts, 5);
}

long Rect_HCenter(const RECT *rect)
{
    return (rect->right + rect->left) >> 1;
}

long Rect_Height(const RECT *rect)
{
    return rect->bottom - rect->top;
}

int Rect_Perimeter(const RECT *rect)
{
    return (Rect_Width(rect) << 1) + (Rect_Height(rect) << 1);
}

BOOL Rect_PtIn(const RECT *rect, POINT pt)
{
    if ((pt.x < rect->left)
            || (pt.x > rect->right)
            || (pt.y < rect->top)
            || (pt.y > rect->bottom))
        return FALSE;
    return TRUE;
}

BOOL Rect_PtIsAbove(const RECT *rect, int x, int y)
{
    return ((y < rect->top) && (x >= rect->left) && (x <= rect->right));
}

BOOL Rect_PtIsBelow(const RECT *rect, int x, int y)
{
    return ((y > rect->bottom) && (x >= rect->left) && (x <= rect->right));
}

BOOL Rect_PtIsToTheLeft(const RECT *rect, int x, int y)
{
    return ((x < rect->left) && (y >= rect->top) && (y <= rect->bottom));
}

BOOL Rect_PtIsToTheRight(const RECT *rect, int x, int y)
{
    return ((x > rect->right) && (y >= rect->top) && (y <= rect->bottom));
}


void Rect_SetFromPoints(RECT *rect, POINT p1, POINT p2)
{
    rect->left = p1.x;
    rect->top = p1.y;
    rect->right = p2.x;
    rect->bottom = p2.y;
}

void Rect_SetInflated(RECT *dest, const RECT *src, int dx, int dy)
{
    dest->left = src->left - dx;
    dest->top = src->top - dy;
    dest->right = src->right + dx;
    dest->bottom = src->bottom + dy;
}

void Rect_DecSize(RECT *rect)
{
    rect->right--;
    rect->bottom--;
}

long Rect_VCenter(const RECT *rect)
{
    return (rect->bottom + rect->top) >> 1;
}

long Rect_Width(const RECT *rect)
{
    return rect->right - rect->left;
}

BOOL Rect_XYIn(const RECT *rect, int x, int y)
{
    if ((x < rect->left)
            || (x > rect->right)
            || (y < rect->top)
            || (y > rect->bottom))
        return FALSE;
    return TRUE;
}

#endif
