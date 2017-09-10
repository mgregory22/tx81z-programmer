/*
 * filedlg.c - handles MS-Windows file dialogs.
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
extern BOOL FileDlg_Open(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr, const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen, DWORD flags);
extern BOOL FileDlg_ParseFileNames(_TUCHAR *multiString, _TUCHAR *nextPath, _TUCHAR **p);
extern BOOL FileDlg_Save(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr, const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen, DWORD flags);
extern void FileDlg_SaveInitialDir(_TUCHAR *initialDir, const _TUCHAR *fileName);

/*
 * Unit procedures
 */
static void fd_Init(OPENFILENAME *fileDlgInfo, HWND parentWnd, const _TUCHAR *initialDir, const _TUCHAR *filterStr, const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen, DWORD flags);

/*
 * Procedure definitions
 */
BOOL FileDlg_Open(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr
        , const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen
        , DWORD flags)
{
    OPENFILENAME fileDlgInfo;

    fd_Init(&fileDlgInfo, parentWnd, initialDir, filterStr, defaultExt
            , fileNameBuf, fileNameBufLen, flags);

	if (GetOpenFileName(&fileDlgInfo)) {
        FileDlg_SaveInitialDir(initialDir, fileNameBuf);

        return TRUE;
    }

    return FALSE;
}

BOOL FileDlg_ParseFileNames(_TUCHAR *multiString, _TUCHAR *nextPath, _TUCHAR **p)
{
    if (*p == multiString) {
        while (**p) {
            (*p)++;
        }
        if (!*(*p + 1)) {
            _tcscpy(nextPath, multiString);
            return TRUE;
        }
    }
    (*p)++;
    if (!**p) {
        return FALSE;
    }
    StrCons(nextPath, multiString, _T("\\"), *p, NULL);
    while (**p) {
        (*p)++;
    }
    return TRUE;
}

BOOL FileDlg_Save(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr
        , const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen
        , DWORD flags)
{
    OPENFILENAME fileDlgInfo;

    fd_Init(&fileDlgInfo, parentWnd, initialDir, filterStr, defaultExt
            , fileNameBuf, fileNameBufLen, flags);

	if (GetSaveFileName(&fileDlgInfo)) {
        FileDlg_SaveInitialDir(initialDir, fileNameBuf);

        return TRUE;
    }
    return FALSE;
}

void FileDlg_SaveInitialDir(_TUCHAR *initialDir, const _TUCHAR *fileName)
{
    /*
     * Save the directory in initialDir.
     */
    if (initialDir) {
        struct _stat statInfo = { 0 };
        _TUCHAR tempDir[_MAX_PATH];

        _tcscpy(tempDir, fileName);
        _tstat(tempDir, &statInfo);
        if ((statInfo.st_mode & _S_IFDIR) == 0) {
            _TUCHAR *p = (_TUCHAR *) FilePath_GetLeafPtr(tempDir);

            if (p > tempDir) {
                *(p - 1) = '\0';
                _tcscpy(initialDir, tempDir);
            }
        }
    }
}

void fd_Init(OPENFILENAME *fileDlgInfo, HWND parentWnd, const _TUCHAR *initialDir
        , const _TUCHAR *filterStr, const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf
        , size_t fileNameBufLen, DWORD flags)
{
	fileDlgInfo->lStructSize       = sizeof *fileDlgInfo;
	fileDlgInfo->hwndOwner         = parentWnd;
	fileDlgInfo->hInstance         = NULL;
	fileDlgInfo->lpstrFilter       = filterStr;
	fileDlgInfo->lpstrCustomFilter = NULL;
	fileDlgInfo->nMaxCustFilter    = 0;
	fileDlgInfo->nFilterIndex      = 0;
	fileDlgInfo->lpstrFile         = fileNameBuf;
	fileDlgInfo->nMaxFile          = fileNameBufLen;
	fileDlgInfo->lpstrFileTitle    = NULL;  // not used
	fileDlgInfo->nMaxFileTitle     = 0;     // not used
	fileDlgInfo->lpstrInitialDir   = initialDir;
	fileDlgInfo->lpstrTitle        = NULL;
	fileDlgInfo->Flags             = flags;
	fileDlgInfo->nFileOffset       = 0;
	fileDlgInfo->nFileExtension    = 0;
	fileDlgInfo->lpstrDefExt       = defaultExt;
	fileDlgInfo->lCustData         = 0L;
	fileDlgInfo->lpfnHook          = NULL;
	fileDlgInfo->lpTemplateName    = NULL;
}

