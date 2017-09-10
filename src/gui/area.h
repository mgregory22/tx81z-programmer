/*
 * gui/area.h - area functions
 *
 * Copyright (c) 2006 Matt Gregory
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
#ifndef GUI_AREA_H
#define GUI_AREA_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#define AREA_R(area) ((area).x + (area).w)
#define AREA_B(area) ((area).y + (area).h)
#define PASS_AREA_FIELDS(area) (area).x, (area).y, (area).w, (area).h
#define Area_XCenter Area_HCenter
#define Area_YCenter Area_VCenter

typedef struct {
    long x;
    long y;
    long w;
    long h;
} AREA;

__inline long Area_HCenter(const AREA *area);
void AreaToRect(RECT *rect, const AREA *area);
__inline long Area_VCenter(const AREA *area);

/*
 * Inline definitions
 */
long Area_HCenter(const AREA *area)
{
    return (area->w >> 1) + area->x;
}

long Area_VCenter(const AREA *area)
{
    return (area->h >> 1) + area->y;
}

#endif  /* #ifndef GUI_AREA_H */

