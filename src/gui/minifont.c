/*
 * minifont.c - A bitmap font based on the TX81Z LCD
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
 * Global constants
 */

_TUCHAR MiniFont_bitmaps[MINIFONT_CHARCNT][7] = {
/* ascii bitmaps */
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00') }, // 32 - space 
    { _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x00'), _T('\x00'), _T('\x04') }, // 33 - !
    { _T('\x0A'), _T('\x0A'), _T('\x0A'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00') }, // 34 - "
    { _T('\x0A'), _T('\x0A'), _T('\x1F'), _T('\x0A'), _T('\x1F'), _T('\x0A'), _T('\x0A') }, // 35 - #
    { _T('\x04'), _T('\x0F'), _T('\x14'), _T('\x0E'), _T('\x05'), _T('\x1E'), _T('\x04') }, // 36 - $
    { _T('\x18'), _T('\x19'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x13'), _T('\x03') }, // 37 - %
    { _T('\x0C'), _T('\x12'), _T('\x14'), _T('\x08'), _T('\x15'), _T('\x12'), _T('\x0D') }, // 38 - &
    { _T('\x0C'), _T('\x04'), _T('\x08'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00') }, // 39 - '
    { _T('\x02'), _T('\x04'), _T('\x08'), _T('\x08'), _T('\x08'), _T('\x04'), _T('\x02') }, // 40 - (
    { _T('\x08'), _T('\x04'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x04'), _T('\x08') }, // 41 - )
    { _T('\x00'), _T('\x04'), _T('\x15'), _T('\x0E'), _T('\x15'), _T('\x04'), _T('\x00') }, // 42 - *
    { _T('\x00'), _T('\x04'), _T('\x04'), _T('\x1F'), _T('\x04'), _T('\x04'), _T('\x00') }, // 43 - +
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x0C'), _T('\x04'), _T('\x08') }, // 44 - ,
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x1F'), _T('\x00'), _T('\x00'), _T('\x00') }, // 45 - -
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x0C'), _T('\x0C') }, // 46 - .
    { _T('\x00'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x10'), _T('\x00') }, // 47 - /
    { _T('\x0E'), _T('\x11'), _T('\x13'), _T('\x15'), _T('\x19'), _T('\x11'), _T('\x0E') }, // 48 - 0
    { _T('\x04'), _T('\x0C'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x0E') }, // 49 - 1
    { _T('\x0E'), _T('\x11'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x1F') }, // 50 - 2
    { _T('\x1F'), _T('\x02'), _T('\x04'), _T('\x02'), _T('\x01'), _T('\x11'), _T('\x0E') }, // 51 - 3
    { _T('\x02'), _T('\x06'), _T('\x0A'), _T('\x12'), _T('\x1F'), _T('\x02'), _T('\x02') }, // 52 - 4
    { _T('\x1F'), _T('\x10'), _T('\x1E'), _T('\x01'), _T('\x01'), _T('\x11'), _T('\x0E') }, // 53 - 5
    { _T('\x06'), _T('\x08'), _T('\x10'), _T('\x1E'), _T('\x11'), _T('\x11'), _T('\x0E') }, // 54 - 6
    { _T('\x1F'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x08'), _T('\x08') }, // 55 - 7
    { _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x0E') }, // 56 - 8
    { _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x0F'), _T('\x01'), _T('\x02'), _T('\x0C') }, // 57 - 9
    { _T('\x00'), _T('\x0C'), _T('\x0C'), _T('\x00'), _T('\x0C'), _T('\x0C'), _T('\x00') }, // 58 - :
    { _T('\x00'), _T('\x0C'), _T('\x0C'), _T('\x00'), _T('\x0C'), _T('\x04'), _T('\x08') }, // 59 - ;
    { _T('\x02'), _T('\x04'), _T('\x08'), _T('\x10'), _T('\x08'), _T('\x04'), _T('\x02') }, // 60 - <
    { _T('\x00'), _T('\x00'), _T('\x1F'), _T('\x00'), _T('\x1F'), _T('\x00'), _T('\x00') }, // 61 - =
    { _T('\x08'), _T('\x04'), _T('\x02'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x08') }, // 62 - >
    { _T('\x0E'), _T('\x11'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x00'), _T('\x04') }, // 63 - ?
    { _T('\x0E'), _T('\x11'), _T('\x01'), _T('\x0D'), _T('\x15'), _T('\x15'), _T('\x0E') }, // 64 - @
    { _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x1F'), _T('\x11'), _T('\x11') }, // 65 - A
    { _T('\x1E'), _T('\x11'), _T('\x11'), _T('\x1E'), _T('\x11'), _T('\x11'), _T('\x1E') }, // 66 - B
    { _T('\x0E'), _T('\x11'), _T('\x10'), _T('\x10'), _T('\x10'), _T('\x11'), _T('\x0E') }, // 67 - C
    { _T('\x1C'), _T('\x12'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x12'), _T('\x1C') }, // 68 - D
    { _T('\x1F'), _T('\x10'), _T('\x10'), _T('\x1E'), _T('\x10'), _T('\x10'), _T('\x1F') }, // 69 - E
    { _T('\x1F'), _T('\x10'), _T('\x10'), _T('\x1E'), _T('\x10'), _T('\x10'), _T('\x10') }, // 70 - F
    { _T('\x0E'), _T('\x11'), _T('\x10'), _T('\x17'), _T('\x11'), _T('\x11'), _T('\x0F') }, // 71 - G
    { _T('\x11'), _T('\x11'), _T('\x11'), _T('\x1F'), _T('\x11'), _T('\x11'), _T('\x11') }, // 72 - H
    { _T('\x0E'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x0E') }, // 73 - I
    { _T('\x07'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x12'), _T('\x0C') }, // 74 - J
    { _T('\x11'), _T('\x12'), _T('\x14'), _T('\x18'), _T('\x14'), _T('\x12'), _T('\x11') }, // 75 - K
    { _T('\x10'), _T('\x10'), _T('\x10'), _T('\x10'), _T('\x10'), _T('\x10'), _T('\x1F') }, // 76 - L
    { _T('\x11'), _T('\x1B'), _T('\x15'), _T('\x15'), _T('\x11'), _T('\x11'), _T('\x11') }, // 77 - M
    { _T('\x11'), _T('\x11'), _T('\x19'), _T('\x15'), _T('\x13'), _T('\x11'), _T('\x11') }, // 78 - N
    { _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0E') }, // 79 - O
    { _T('\x1E'), _T('\x11'), _T('\x11'), _T('\x1E'), _T('\x10'), _T('\x10'), _T('\x10') }, // 80 - P
    { _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x15'), _T('\x12'), _T('\x0D') }, // 81 - Q
    { _T('\x1E'), _T('\x11'), _T('\x11'), _T('\x1E'), _T('\x14'), _T('\x12'), _T('\x11') }, // 82 - R
    { _T('\x0F'), _T('\x10'), _T('\x10'), _T('\x0E'), _T('\x01'), _T('\x01'), _T('\x1E') }, // 83 - S
    { _T('\x1F'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04') }, // 84 - T
    { _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0E') }, // 85 - U
    { _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0A'), _T('\x04') }, // 86 - V
    { _T('\x11'), _T('\x11'), _T('\x11'), _T('\x15'), _T('\x15'), _T('\x15'), _T('\x0A') }, // 87 - W
    { _T('\x11'), _T('\x11'), _T('\x0A'), _T('\x04'), _T('\x0A'), _T('\x11'), _T('\x11') }, // 88 - X
    { _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0A'), _T('\x04'), _T('\x04'), _T('\x04') }, // 89 - Y
    { _T('\x1F'), _T('\x01'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x10'), _T('\x1F') }, // 90 - Z
    { _T('\x0E'), _T('\x08'), _T('\x08'), _T('\x08'), _T('\x08'), _T('\x08'), _T('\x0E') }, // 91 - [
    { _T('\x11'), _T('\x0A'), _T('\x1F'), _T('\x04'), _T('\x1F'), _T('\x04'), _T('\x04') }, // 92 - (yen)
    { _T('\x0E'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x02'), _T('\x0E') }, // 93 - ]
    { _T('\x04'), _T('\x0A'), _T('\x11'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00') }, // 94 - ^
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x1F') }, // 95 - _
    { _T('\x08'), _T('\x04'), _T('\x02'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00') }, // 96 - `
    { _T('\x00'), _T('\x00'), _T('\x0E'), _T('\x01'), _T('\x0F'), _T('\x11'), _T('\x0F') }, // 97 - a
    { _T('\x10'), _T('\x10'), _T('\x16'), _T('\x19'), _T('\x11'), _T('\x11'), _T('\x1E') }, // 98 - b
    { _T('\x00'), _T('\x00'), _T('\x0E'), _T('\x10'), _T('\x10'), _T('\x11'), _T('\x0E') }, // 99 - c
    { _T('\x01'), _T('\x01'), _T('\x0D'), _T('\x13'), _T('\x11'), _T('\x11'), _T('\x0F') }, // 100 - d
    { _T('\x00'), _T('\x00'), _T('\x0E'), _T('\x11'), _T('\x1F'), _T('\x10'), _T('\x0E') }, // 101 - e
    { _T('\x06'), _T('\x09'), _T('\x08'), _T('\x1C'), _T('\x08'), _T('\x08'), _T('\x08') }, // 102 - f
    { _T('\x00'), _T('\x0F'), _T('\x11'), _T('\x11'), _T('\x0F'), _T('\x01'), _T('\x0E') }, // 103 - g
    { _T('\x10'), _T('\x10'), _T('\x16'), _T('\x19'), _T('\x11'), _T('\x11'), _T('\x11') }, // 104 - h
    { _T('\x04'), _T('\x00'), _T('\x0C'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x0E') }, // 105 - i
    { _T('\x02'), _T('\x00'), _T('\x06'), _T('\x02'), _T('\x02'), _T('\x12'), _T('\x0C') }, // 106 - j
    { _T('\x10'), _T('\x10'), _T('\x12'), _T('\x14'), _T('\x18'), _T('\x14'), _T('\x12') }, // 107 - k
    { _T('\x0C'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x0E') }, // 108 - l
    { _T('\x00'), _T('\x00'), _T('\x1A'), _T('\x15'), _T('\x15'), _T('\x11'), _T('\x11') }, // 109 - m
    { _T('\x00'), _T('\x00'), _T('\x16'), _T('\x19'), _T('\x11'), _T('\x11'), _T('\x11') }, // 110 - n
    { _T('\x00'), _T('\x00'), _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0E') }, // 111 - o
    { _T('\x00'), _T('\x00'), _T('\x1E'), _T('\x11'), _T('\x1E'), _T('\x10'), _T('\x10') }, // 112 - p
    { _T('\x00'), _T('\x00'), _T('\x0D'), _T('\x13'), _T('\x0F'), _T('\x01'), _T('\x01') }, // 113 - q
    { _T('\x00'), _T('\x00'), _T('\x16'), _T('\x19'), _T('\x10'), _T('\x10'), _T('\x10') }, // 114 - r
    { _T('\x00'), _T('\x00'), _T('\x0E'), _T('\x10'), _T('\x0E'), _T('\x01'), _T('\x1E') }, // 115 - s
    { _T('\x08'), _T('\x08'), _T('\x1C'), _T('\x08'), _T('\x08'), _T('\x09'), _T('\x06') }, // 116 - t
    { _T('\x00'), _T('\x00'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x13'), _T('\x0D') }, // 117 - u
    { _T('\x00'), _T('\x00'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0A'), _T('\x04') }, // 118 - v
    { _T('\x00'), _T('\x00'), _T('\x11'), _T('\x11'), _T('\x15'), _T('\x15'), _T('\x0A') }, // 119 - w
    { _T('\x00'), _T('\x00'), _T('\x11'), _T('\x0A'), _T('\x04'), _T('\x0A'), _T('\x11') }, // 120 - x
    { _T('\x00'), _T('\x00'), _T('\x11'), _T('\x11'), _T('\x0F'), _T('\x01'), _T('\x0E') }, // 121 - y
    { _T('\x00'), _T('\x00'), _T('\x1F'), _T('\x02'), _T('\x04'), _T('\x08'), _T('\x1F') }, // 122 - z
    { _T('\x02'), _T('\x04'), _T('\x04'), _T('\x08'), _T('\x04'), _T('\x04'), _T('\x02') }, // 123 - {
    { _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04'), _T('\x04') }, // 124 - |
    { _T('\x08'), _T('\x04'), _T('\x04'), _T('\x02'), _T('\x04'), _T('\x04'), _T('\x08') }, // 125 - }
    { _T('\x00'), _T('\x04'), _T('\x02'), _T('\x1F'), _T('\x02'), _T('\x04'), _T('\x00') }, // 126 - 0x7E (right arrow)
    { _T('\x00'), _T('\x04'), _T('\x08'), _T('\x1F'), _T('\x08'), _T('\x04'), _T('\x00') }, // 127 - 0x7F (left arrow)
/* waveform bitmaps */
    { _T('\x04'), _T('\x0A'), _T('\x11'), _T('\x10'), _T('\x00'), _T('\x00'), _T('\x00') }, // 128 - 0x80 left sine (up)
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x01'), _T('\x11'), _T('\x0A'), _T('\x04') }, // 129 - 0x81 right sine (down)
    { _T('\x04'), _T('\x04'), _T('\x0A'), _T('\x11'), _T('\x00'), _T('\x00'), _T('\x00') }, // 130 - 0x82 left triangle (up)
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x11'), _T('\x0A'), _T('\x04'), _T('\x04') }, // 131 - 0x83 right triangle (down)
    { _T('\x08'), _T('\x14'), _T('\x14'), _T('\x15'), _T('\x05'), _T('\x05'), _T('\x02') }, // 132 - 0x84 short sine
    { _T('\x08'), _T('\x08'), _T('\x14'), _T('\x15'), _T('\x05'), _T('\x02'), _T('\x02') }, // 133 - 0x85 short triangle
    { _T('\x0A'), _T('\x15'), _T('\x15'), _T('\x15'), _T('\x00'), _T('\x00'), _T('\x00') }, // 134 - 0x86 two sines up
    { _T('\x0A'), _T('\x0A'), _T('\x15'), _T('\x15'), _T('\x00'), _T('\x00'), _T('\x00') }, // 135 - 0x87 two triangles up
/* misc symbols */
    { _T('\x1F'), _T('\x0A'), _T('\x0A'), _T('\x0A'), _T('\x0A'), _T('\x0A'), _T('\x1F') }, // 136 - 0x88 'II' out assign setting
    { _T('\x02'), _T('\x06'), _T('\x02'), _T('\x1A'), _T('\x02'), _T('\x02'), _T('\x07') }, // 137 - 0x89 '-1' performance note limit
    { _T('\x02'), _T('\x05'), _T('\x01'), _T('\x19'), _T('\x02'), _T('\x04'), _T('\x07') }, // 138 - 0x8A '-2' performance note limit
    { _T('\x00'), _T('\x00'), _T('\x00'), _T('\x00'), _T('\x1F'), _T('\x0E'), _T('\x04') }, // 139 - 0x8B middle C marker
};

/*
 * Global procedures
 */
extern void MiniFont_DrawChar(HDC dC, int x, int y, _TUCHAR ch, COLORREF color);
extern void MiniFont_DrawChar2X(HDC dC, int x, int y, _TUCHAR ch, HBRUSH brush);
extern void MiniFont_DrawString(HDC dC, int x, int y, const _TUCHAR *str, int len, COLORREF color);
extern void MiniFont_DrawString2X(HDC dC, int x, int y, const _TUCHAR *str, int len, HBRUSH brush);
extern void MiniFont_DrawVerticalStringUp(HDC dC, int x, int y, const _TUCHAR *str, int len, COLORREF color);
extern void MiniFont_ReplaceZero(BOOL slashed);

/*
 * Unit constants
 */
static _TUCHAR mf_slashedZero[7] = {
    _T('\x0E'), _T('\x11'), _T('\x13'), _T('\x15'), _T('\x19'), _T('\x11'), _T('\x0E')
};
static _TUCHAR mf_unslashedZero[7] = {
    _T('\x0E'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0E')
    //_T('\x04'), _T('\x0A'), _T('\x11'), _T('\x11'), _T('\x11'), _T('\x0A'), _T('\x04')
};

/*
 * Procedure definitions
 */
void MiniFont_DrawChar(HDC dC, int x, int y, _TUCHAR ch, COLORREF color)
{
    int i, j;
    const _TUCHAR *mfc;
    
    if (ch < 32 || ch >= MINIFONT_CHARCNT + 32) {
        ch = 36;
    }
    mfc = MiniFont_bitmaps[ch - 32];
    for (j = 0; j < 7; j++) {
        int bmp = mfc[j];

        if (bmp) {
            for (i = 0; i < 5; i++) {
                if (bmp & (1 << (4 - i))) {
                    SetPixelV(dC, x + i, y + j, color);
                }
            }
        }
    }
}

void MiniFont_DrawChar2X(HDC dC, int x, int y, _TUCHAR ch, HBRUSH brush)
{
    int i, j;
    const _TUCHAR *mfc;
    
    if (ch < 32 || ch >= MINIFONT_CHARCNT + 32) {
        ch = 36;
    }
    mfc = MiniFont_bitmaps[ch - 32];
    for (j = 0; j < 7; j++) {
        int bmp = mfc[j];

        if (bmp) {
            for (i = 0; i < 5; i++) {
                if (bmp & (1 << (4 - i))) {
                    RECT dot = {
                        x + (i * 2),
                        y + (j * 2),
                        dot.left + 2,
                        dot.top + 2
                    };
                    FrameRect(dC, &dot, brush);
                }
            }
        }
    }
}

void MiniFont_DrawString(HDC dC, int x, int y, const _TUCHAR *str, int len
        , COLORREF color)
{
    int n = 0;
    
    for ( ; n < len; n++) {
        int i, j;
        int x0 = x + n * 6;
        int ch = str[n];
        const _TUCHAR *mfc;

        if (ch < 32 || ch >= MINIFONT_CHARCNT + 32) {
            ch = 36;
        }
        mfc = MiniFont_bitmaps[ch - 32];
        for (j = 0; j < 7; j++) {
            int bmp = mfc[j];

            if (bmp) {
                for (i = 0; i < 5; i++) {
                    if (bmp & (1 << (4 - i))) {
                        SetPixelV(dC, x0 + i, y + j, color);
                    }
                }
            }
        }
    }
}

void MiniFont_DrawString2X(HDC dC, int x, int y, const _TUCHAR *str, int len
        , HBRUSH brush)
{
    int n = 0;

    for ( ; n < len; n++) {
        int i, j;
        int x0 = x + n * 12;
        int ch = str[n];
        const _TUCHAR *mfc;

        if (ch < 32 || ch >= MINIFONT_CHARCNT + 32) {
            ch = 36;
        }
        mfc = MiniFont_bitmaps[ch - 32];
        for (j = 0; j < 7; j++) {
            int bmp = mfc[j];

            if (bmp) {
                for (i = 0; i < 5; i++) {
                    if (bmp & (1 << (4 - i))) {
                        RECT dot = {
                            x0 + (i * 2),
                            y + (j * 2),
                            dot.left + 2,
                            dot.top + 2
                        };
                        FrameRect(dC, &dot, brush);
                    }
                }
            }
        }
    }
}

void MiniFont_DrawVerticalStringUp(HDC dC, int x, int y, const _TUCHAR *str
        , int len, COLORREF color)
{
    int n = 0;

    for ( ; n < len; n++) {
        int i, j;
        int ch = str[n];
        const _TUCHAR *mfc;
        int y0 = y - n * 6;

        if (ch < 32 || ch >= MINIFONT_CHARCNT + 32) {
            ch = 36;
        }
        mfc = MiniFont_bitmaps[ch - 32];
        for (i = 0; i < 7; i++) {
            int c = mfc[i];
            if (c) {
                for (j = 0; j < 5; j++) {
                    if (c & (1 << (4 - j))) {
                        SetPixelV(dC, x + i, y0 - j, color);
                    }
                }
            }
        }
    }
}

void MiniFont_ReplaceZero(BOOL slashed)
{
    int i;

    for (i = 0; i < 7; i++) {
        MiniFont_bitmaps['0' - ' '][i] = slashed ? mf_slashedZero[i]
            : mf_unslashedZero[i];
    }
}

