/*
 * gui/font.c - simplified font creation functions
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
extern HFONT Font_CreateBold(HDC dc, int pointSize, const _TUCHAR *faceName);


/*
 * Procedure definitions
 */
HFONT Font_CreateBold(HDC dC, int pointSize, const _TUCHAR *faceName)
{
    int height = -MulDiv(pointSize, GetDeviceCaps(dC, LOGPIXELSY), 72);
    LOGFONT lf = {
        height,                         /* lfHeight */
        0,                              /* lfWidth */
        0,                              /* lfEscapement */
        0,                              /* lfOrientation */
        FW_BOLD,                        /* lfWeight */
        0,                              /* lfItalic */
        0,                              /* lfUnderline */
        0,                              /* lfStrikeOut */
        DEFAULT_CHARSET,                /* lfCharSet */
        OUT_TT_PRECIS,                  /* lfOutPrecision */
        CLIP_DEFAULT_PRECIS,            /* lfClipPrecision */
        DEFAULT_QUALITY,                /* lfQuality */
        DEFAULT_PITCH | FF_DONTCARE     /* lfPitchAndFamily */
    };
    if (faceName)
        _tcsncpy(lf.lfFaceName, faceName, LF_FACESIZE);

	return CreateFontIndirect(&lf);
}

