/*
 * msg/str.h - string functions and macros
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

#ifndef MSG_STR_H
#define MSG_STR_H

#ifndef _INC_STDLIB
#   include <stdlib.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef MSG_FUNC_H
#   include "func.h"
#endif

/*
 * Macros
 */
#define StrEq(s1, s2) (_tcscmp((s1), (s2)) == 0)
#define StrNotEq(s1, s2) (_tcscmp((s1), (s2)) != 0)
#define StrLt(s1, s2) (_tcscmp((s1), (s2)) < 0)
#define StrGt(s1, s2) (_tcscmp((s1), (s2)) > 0)
#define StrEqN(s1, s2, n) (_tcsncmp((s1), (s2), (n)) == 0)
#define StrNotEqN(s1, s2, n) (_tcsncmp((s1), (s2), (n)) != 0)
#define StrLtN(s1, s2, n) (_tcsncmp((s1), (s2), (n)) < 0)
#define StrGtN(s1, s2, n) (_tcsncmp((s1), (s2), (n)) > 0)
#define StriEq(s1, s2) (_tcsicmp((s1), (s2)) == 0)
#define StriNotEq(s1, s2) (_tcsicmp((s1), (s2)) != 0)
#define StriLt(s1, s2) (_tcsicmp((s1), (s2)) < 0)
#define StrIGt(s1, s2) (_tcsicmp((s1), (s2)) > 0)
#define StriEqN(s1, s2, n) (_tcsnicmp((s1), (s2), (n)) == 0)
#define StriNotEqN(s1, s2, n) (_tcsnicmp((s1), (s2), (n)) != 0)
#define StriLtN(s1, s2, n) (_tcsnicmp((s1), (s2), (n)) < 0)
#define StriGtN(s1, s2, n) (_tcsnicmp((s1), (s2), (n)) > 0)
#define IsTokenEnd(c) (c == '\0' || isspace(c))

/*
 * AnsiToUnicodeDupe() - Converts an Ansi string to a wide character string by
 *                       copying it into a malloc'ed buffer.
 */
wchar_t *AnsiToUnicodeDupe(const char *src);

/*
 * AnsiToUnicodeCopy() - Converts an Ansi string to a wide character string by
 *                       copying it into the dest buffer.  dest is returned.
 */
wchar_t *AnsiToUnicodeCopy(wchar_t *dest, const char *src);

/*
 * AnsiToUnicodeNCopy() - Converts len characters of an Ansi string to a wide
 *                        character string by copying it into the dest buffer.
 *                        dest is returned.
 */
wchar_t *AnsiToUnicodeNCopy(wchar_t *dest, const char *src, size_t len);

/*
 * BinaryStr() - Converts an integer to a binary representation.
 */
_TUCHAR *BinaryStr(_TUCHAR *dest, long value, int bits, _TUCHAR onChar
		, _TUCHAR offChar);

/*
 * ByteToHexPair() - Converts an 8 bit byte to a 2 digit hexadecimal string.
 *                   dest is returned.
 */
_TUCHAR *ByteToHexPair(_TUCHAR dest[3], unsigned char value);

/*
 * CToPChar() - Converts a char to a C-style constant suitable for printing.
 *              The dest parameter must point to a valid string at least 5
 *              chars long.  If <spaces> is true, then spaces are converted to
 *              '\s' sequences.  Tabs are always converted to '\t'.
 */
_TUCHAR *CToPChar(_TUCHAR *dest, _TUCHAR c, BOOL spaces);

/*
 * Chomp() - Removes all trailing newlines from a string.  The length of the
 *           new string is returned.  If the string has no newlines, it is
 *           unchanged and its length is returned.
 */
size_t Chomp(_TUCHAR *str);

/*
 * FromAnsiCopy() - Converts an ANSI string to _TUCHAR and copies it to dest.
 *                  The number of characters copied including the null is
 *                  returned.
 */
size_t FromAnsiCopy(_TUCHAR *dest, const char *src);

/*
 * FromAnsiNCat() - Converts an ANSI string to _TUCHAR and concatenates it onto
 *                  dest.  The final string will have no more than len
 *                  characters, including the null.
 */
size_t FromAnsiNCat(_TUCHAR *dest, const char *src, size_t len);

/*
 * FromAnsiNCopy() - Copies len number of characters from an ANSI string into
 *                   a _TUCHAR string.  dest is returned.
 */
_TUCHAR *FromAnsiNCopy(_TUCHAR *dest, const char *src, size_t len);

/*
 * FromAnsiDupe() - Duplicates an ANSI string, converting it to _TUCHAR.
 */
_TUCHAR *FromAnsiDupe(const char *src);

/*
 * FromAnsiNDupe() - Duplicates len characters of src, converting them to
 *                   _TUCHAR.  The function always allocates an extra character
 *                   and appends a null terminator.
 */
_TUCHAR *FromAnsiNDupe(const char *src, size_t len);

/*
 * FromUnicodeCopy() - Copies a wide character string, converting it to _TUCHAR.
 *                     The number of characters copied (including the null) is
 *                     returned.
 */
size_t FromUnicodeCopy(_TUCHAR *dest, const wchar_t *src);

/*
 * FromUnicodeDupe() - Duplicates a wide character string, converting it to
 *                     _TUCHAR.
 */
_TUCHAR *FromUnicodeDupe(const wchar_t *src);

/*
 * FromUnicodeDupe() - Duplicates len characters of src, converting them to
 *                     _TUCHAR.  The function always allocates an extra character
 *                     and appends a null terminator.
 */
_TUCHAR *FromUnicodeNDupe(const wchar_t *src, size_t len);

/*
 * GCvtAlignDecimal() - Converts a double to a string.  The number is align so
 *                      the decimal is in the character position at decPos.
 *                      The string is padded with spaces up to destLen.  The
 *                      number of digits after the decimal point is the minimum
 *                      needed to express the value.
 */
_TUCHAR *GCvtAlignDecimal(double d, size_t decPos, _TUCHAR *dest, size_t destLen);

/*
 * HexCharConst() - Converts a character to a C-style hex char constant.
 */
_TUCHAR *HexCharConst(_TUCHAR *dest, _TUCHAR c);

/*
 * HexPairToByte() - Converts a 2 digit hex string to a byte.  True is
 *                   returned on success.
 */
BOOL HexPairToByte(_TUCHAR src[3], unsigned char *dest);

/*
 * LCaseStr() - Converts a string to lower case in place.
 */
_TUCHAR *LCaseStr(_TUCHAR *str);

/*
 * LongToDecStr() - Converts a long to a decimal string representation.
 */
_TUCHAR *LongToDecStr(_TUCHAR *str, long value);

/*
 * MakeDump() - Converts a binary buffer into a hex/ascii dump.  It sends each
 *              line to DumpFunc.  Each line has 16 bytes and does not end in
 *              a newline.  user gets passed as-is to DumpFunc.
 */
void MakeDump(void *buffer, size_t bufSize, DUMPFUNC DumpFunc
        , unsigned long user);

/*
 * MakeSpacer() - Fills a string with a number of characters and appends a null.
 */
_TUCHAR *MakeSpacer(_TUCHAR *dest, _TUCHAR c, int cnt);

/*
 * NibbleToHexDigit() - Takes the lowest four bits of an integer and returns
 *                      its representation as a hexadecimal digit.
 */
_TUCHAR NibbleToHexDigit(int n);

/*
 * ParseMultiString() - Given a string pointer p that points to a string
 *                      composed of multiple null terminated strings that end
 *                      in a double null, the first string in the list is
 *                      returned and p is advanced to the next string in the
 *                      list.  When the function returns null, then p is at
 *                      the end of the list.
 */
_TUCHAR *ParseMultiString(_TUCHAR **p);

/*
 * QuoteStr() - Puts double quotes around a string in place.  The buffer must
 *              be big enough to hold 2 more characters.
 */
void QuoteStr(_TUCHAR *str);

/*
 * StrCmpToChar() - Compares two strings until it reaches a null terminator or
 *                  <stopChar>.  Returns zero if the strings are equal.
 */
int StrCmpToChar(const _TUCHAR *str1, const _TUCHAR *str2, _TUCHAR stopChar);

/*
 * StrCons() - Constructs a string by concatenating multiple strings.  The
 *             destination must be large enough to hold all the strings.  The
 *             final argument must be null.  Returns dest.
 */
_TUCHAR *StrCons(_TUCHAR *dest, const _TUCHAR *first, ...);

/*
 * Same as StrCons but returns a malloc'ed buffer that must be freed by
 * the caller.
 */
_TUCHAR *StrConsM(const _TUCHAR *first, ...);

/*
 * StrCopyToChar() - Copies a string until it reaches stopChar and adds a null.
 *                   Returns the number of characters copied, excluding the
 *                   null.
 */
size_t StrCopyToChar(_TUCHAR *dest, const _TUCHAR *src, _TUCHAR stopChar);

/*
 * StrCopyLen() - Copies a string and returns its length.
 */
size_t StrCopyLen(_TUCHAR *dest, const _TUCHAR *src);

/*
 * Converts all unprintable chars in a string to printable chars.  dest
 * must be large enough to hold the output, which could be up to four
 * times the size of src.
 */
_TUCHAR *StrCToPChar(_TUCHAR *dest, const _TUCHAR *src, BOOL spaces);

/*
 * Converts all unprintable chars in a string to printable chars.  dest
 * must be large enough to hold the output, which could be up to four
 * times the size of src.
 */
_TUCHAR *StrCToPCharN(_TUCHAR *dest, const _TUCHAR *src, int len, BOOL spaces);

/*
 * Copies a string into a malloc'ed buffer and returns it.
 */
_TUCHAR *StrDupe(const _TUCHAR *src);

/*
 * Copies len characters of string src into a malloc'ed buffer, adds a null
 * terminator and returns it.
 */
_TUCHAR *StrNDupe(const _TUCHAR *src, int len);

/*
 * Copies <src> to <dest> while replacing the first occurrence of <oldSubStr>
 * in <src> with <newSubStr>.  <src> and <dest> cannot overlap.  If <oldSubStr>
 * is not found, <dest> will contain an exact copy of <src>.  All strings
 * except <dest> must be null terminated.  <len> is the length of the <dest>
 * buffer.  <dest> is returned and may not be null terminated if <src> is
 * longer than <len>.
 */
_TUCHAR *StrRepl(_TUCHAR *dest, int len, const _TUCHAR *src, const _TUCHAR *oldSubStr
		, const _TUCHAR *newSubStr);

/*
 * Reverses a string.
 */
_TUCHAR *StrRev(_TUCHAR *str);

/*
 * Replaces all occurences of delimiter in str to null characters and returns
 * an array of pointers to every null-terminated string this creates.  The
 * number of strings is returned in count if it is not null.  The last entry
 * in the array is set to null.  The caller is responsible for freeing the
 * array.  The function will return null if the allocation of the array fails.
 */
_TUCHAR **StrSplit(_TUCHAR *str, _TUCHAR delimiter, int *count);

/*
 * Copies <src> from character <start> to character <end> inclusive into
 * <dest>.  <dest> must be large enough to hold the substring.
 */
_TUCHAR *SubStr(_TUCHAR *dest, const _TUCHAR *src, int start, int end);

/*
 * Copies <n> characters from <src> into <dest> starting from the beginning of
 * <src>.  <dest> must be large enough to hold <n> characters + the terminating
 * null.  <dest> is returned.
 */
_TUCHAR *SubStrLeft(_TUCHAR *dest, const _TUCHAR *src, int n);

/*
 * Copies <n> characters from <src> into <dest> starting from <start>.  <dest>
 * must be large enough to hold <n> characters + the terminating null. <dest>
 * is returned.
 */
_TUCHAR *SubStrN(_TUCHAR *dest, const _TUCHAR *src, int start, int n);

/*
 * Copies <n> characters from the right of <src> into <dest>.  <dest> must
 * be large enough to hold the characters + a null and <dest> is returned.
 */
_TUCHAR *SubStrRight(_TUCHAR *dest, const _TUCHAR *src, int n);

/*
 * Same as SubStrRight except the length of <src> is given instead of
 * calculated within the function.
 */
_TUCHAR *SubStrRightN(_TUCHAR *dest, const _TUCHAR *src, int len, int n);

/*
 * Converts a string to title case (first letter capitalized, the rest lower
 * cased) in place.
 */
_TUCHAR *TitleCaseStr(_TUCHAR *str);

/*
 * Copies a string, converting it to ANSI.  The number of characters copied
 * (including the null) is returned.
 */
size_t ToAnsiCopy(char *dest, const _TUCHAR *src);

/*
 * Duplicates a string, converting it to ANSI.
 */
char *ToAnsiDupe(const _TUCHAR *src);

/*
 * Copies src into a fixed buffer of length len.  dest is guaranteed to be
 * null terminated.
 */
size_t ToAnsiFixedBufCopy(char *dest, size_t len, const _TUCHAR *src);

/*
 * ToAnsiNCat() - Converts src to ANSI and appends it to dest.  The final
 *                string won't be longer than len.
 */
size_t ToAnsiNCat(char *dest, const _TUCHAR *src, size_t len);

/*
 * ToAnsiNCopy() - Copies len characters of src into dest, converting to char.
 *                 dest might not be null terminated if src is longer than len.
 */
size_t ToAnsiNCopy(char *dest, const _TUCHAR *src, size_t len);

/*
 * Duplicates len characters of src, converting them to ANSI.  The function
 * always allocates an extra character and appends a null terminator.
 */
char *ToAnsiNDupe(const _TUCHAR *src, size_t len);

/*
 * Copies a string, converting it to Unicode.  The number of characters copied
 * (including the null) is returned.
 */
size_t ToUnicodeCopy(wchar_t *dest, const _TUCHAR *src);

/*
 * Duplicates a string, converting it to Unicode.
 */
wchar_t *ToUnicodeDupe(const _TUCHAR *src);

/*
 * Copies src into a fixed wchar_t buffer of length len.  dest is guaranteed
 * to be null terminated.
 */
size_t ToUnicodeFixedBufCopy(wchar_t *dest, size_t len, const _TUCHAR *src);

/*
 * Copies len characters of src into dest, converting to wchar_t.  dest might
 * not be null terminated if src is longer than len.
 */
size_t ToUnicodeNCopy(wchar_t *dest, const _TUCHAR *src, size_t len);

/*
 * Duplicates len characters of src, converting them to Unicode.  The function
 * always allocates an extra character and appends a null terminator.
 */
wchar_t *ToUnicodeNDupe(const _TUCHAR *src, size_t len);

/*
 * Trims leading and trailing whitespace from a string.  dest must point
 * to a buffer large enough to hold src.
 */
_TUCHAR *TrimWS(_TUCHAR *dest, const _TUCHAR *src);

/*
 * Trims leading and trailing whitespace from a non-null-terminated string.
 * <dest> must point to a buffer large enough to hold <src> + 1.
 */
_TUCHAR *TrimWSn(_TUCHAR *dest, const _TUCHAR *src, int len);

/*
 * Converts a string to upper case in place.
 */
_TUCHAR *UCaseStr(_TUCHAR *str);

/*
 * Converts an unsigned long to a binary string representation.
 */
_TUCHAR *UlongToBinStr(_TUCHAR *str, unsigned long value);

/*
 * Converts an unsigned long to a decimal string representation.
 */
_TUCHAR *UlongToDecStr(_TUCHAR *str, unsigned long value);

/*
 * Converts an unsigned long to a hexadecimal string representation.
 */
_TUCHAR *UlongToHexStr(_TUCHAR *str, unsigned long value);

/*
 * Converts an unsigned long to a octal string representation.
 */
_TUCHAR *UlongToOctStr(_TUCHAR *str, unsigned long value);

/*
 * Removes quotes and de-slashifies a string representing a string literal.
 * The buffer is modified (shortened) and returned.
 */
_TUCHAR *UnquoteStr(_TUCHAR *str);

/*
 * Concatenates a string with line wrapping.
 */
int WrapCat(_TUCHAR *dest, const _TUCHAR *src, int lineLen, const _TUCHAR *wrapChars
        , const _TUCHAR *newLine);

/*
 * Returns the position where a newline should be inserted for proper
 * word wrapping.  If wrapChars is null, it defaults to just wrapping at
 * spaces.
 */
int WrapStr(const _TUCHAR *s, int maxWrapLen, const _TUCHAR *wrapChars);


/*
 * Global variables
 */
extern const char emptyAnsiStr[];
extern const wchar_t emptyUnicodeStr[];
extern const _TUCHAR *emptyStr;


#endif  /* #ifndef MSG_STR_H */
