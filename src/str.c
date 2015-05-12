/*
 * str.c - string functions
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
extern size_t FromAnsiNCat(_TUCHAR *dest, const char *src, size_t len);
extern _TUCHAR *FromAnsiNCopy(_TUCHAR *dest, const char *src, size_t len);
extern _TUCHAR *MakeSpacer(_TUCHAR *dest, _TUCHAR c, int cnt);
extern _TUCHAR *StrCons(_TUCHAR *dest, const _TUCHAR *first, ...);
extern _TUCHAR *SubStr(_TUCHAR *dest, const _TUCHAR *src, int start, int end);
extern _TUCHAR *SubStrLeft(_TUCHAR *dest, const _TUCHAR *src, int n);
extern size_t ToAnsiCopy(char *dest, const _TUCHAR *src);
extern size_t ToAnsiNCopy(char *dest, const _TUCHAR *src, size_t len);

/*
 * Global variables
 */
const char emptyAnsiStr[] = "";
const wchar_t emptyUnicodeStr[] = L"";
#ifdef UNICODE
const _TUCHAR *emptyStr = emptyUnicodeStr;
#else
const _TUCHAR *emptyStr = emptyAnsiStr;
#endif


/*
 * Procedure definitions
 */
size_t FromAnsiNCat(_TUCHAR *dest, const char *src, size_t len)
{
    size_t i = _tcslen(dest);

    while (i < len && (dest[i++] = *src++))
        ;
    
    return i;
}

_TUCHAR *FromAnsiNCopy(_TUCHAR *dest, const char *src, size_t len)
{
    size_t i = 0;

    while (i < len && (dest[i] = src[i++]))
        ;

    return dest;
}

_TUCHAR *MakeSpacer(_TUCHAR *dest, _TUCHAR c, int cnt)
{
    _TUCHAR *p = dest;

    while (--cnt >= 0)
        *p++ = c;
    *p = '\0';

    return dest;
}

_TUCHAR *StrCons(_TUCHAR *dest, const _TUCHAR *first, ...)
{
    va_list args;
    const _TUCHAR *nextStr = first;

    va_start(args, first);
    dest[0] = '\0';
    do {
        _tcscat(dest, nextStr);
    } while ((nextStr = va_arg(args, const _TUCHAR *)) != NULL);
    va_end(args);

    return dest;
}

_TUCHAR *StrDupe(const _TUCHAR *src)
{
    _TUCHAR *dest;

    if (!src) {
        return NULL;
    }
    if (!(dest = malloc((_tcslen(src) + 1) * sizeof(_TUCHAR)))) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    _tcscpy(dest, src);

    return dest;
}

_TUCHAR *SubStr(_TUCHAR *dest, const _TUCHAR *src, int start, int end)
{
    if (start > end)
        return NULL;
    if (dest != src) {
        if ((dest > src && dest < src + end)
            || (src > dest && src < dest + end))
            goto MoveMem;
        _tcsncpy(dest, src + start, end - start + 1);
    } else {
MoveMem:
        memmove(dest, src + start, (end - start + 1) * sizeof(_TUCHAR));
    }
    dest[end - start + 1] = '\0';
    return dest;
}

_TUCHAR *SubStrLeft(_TUCHAR *dest, const _TUCHAR *src, int n)
{
    if (dest != src) {
        if ((dest > src && dest < src + n)
            || (src > dest && src < dest + n))
            memmove(dest, src, n * sizeof(_TUCHAR));
        _tcsncpy(dest, src, n);
    }
    dest[n] = '\0';
    return dest;
}

size_t ToAnsiCopy(char *dest, const _TUCHAR *src)
{
    char *p = dest;

    do {
        *p = (char) *src++;
    } while (*p++);

    return p - dest;
}

size_t ToAnsiNCopy(char *dest, const _TUCHAR *src, size_t len)
{
    size_t i;

    for (i=0; i<len; i++) {
        if ((dest[i] = (char) src[i]) == '\0')
            break;
    }

    return i;
}

