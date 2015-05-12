/*
 * minifont.h - A bitmap font based on the TX81Z LCD
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
#ifndef MINIFONT_H
#define MINIFONT_H

#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Global constants
 */
#define MINIFONT_CHARCNT  108

/*
 * Global procedures
 */
void MiniFont_DrawChar(HDC dC, int x, int y, _TUCHAR ch, COLORREF color);
void MiniFont_DrawChar2X(HDC dC, int x, int y, _TUCHAR ch, HBRUSH brush);
void MiniFont_DrawString(HDC dC, int x, int y, const _TUCHAR *str, int len
        , COLORREF color);
void MiniFont_DrawString2X(HDC dC, int x, int y, const _TUCHAR *str, int len
        , HBRUSH brush);
void MiniFont_DrawVerticalStringUp(HDC dC, int x, int y, const _TUCHAR *str
        , int len, COLORREF color);
void MiniFont_ReplaceZero(BOOL slashed);


/*
 * Global variables
 */
extern _TUCHAR MiniFont_bitmaps[MINIFONT_CHARCNT][7];

#endif
