/*
 * gui/filedlg.h - handles MS-Windows file dialogs.
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
#ifndef FILEDLG_H
#define FILEDLG_H

#ifndef _INC_WINDOWS
#define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_COMMDLG
#   include <commdlg.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Examples:
    _TUCHAR fileName[_MAX_PATH] = {0};
    if (FileDlg_Open(mainWnd
            , _T("Data Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0")
            , _T("dat")
            , fileName
            , _MAX_PATH
            , OFN_CREATEPROMPT | OFN_HIDEREADONLY))
    {
        // open file
    }
#define multiFileNameLen 4096
    _TUCHAR multiFileNames[multiFileNameLen] = {0};
    if (FileDlg_Open(mainWnd
            , _T("Data Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0")
            , _T("dat")
            , fileName
            , _MAX_PATH
            , OFN_ALLOWMULTISELECT | OFN_CREATEPROMPT | OFN_HIDEREADONLY))
    {
        _TUCHAR path[_MAX_PATH];
        _TUCHAR *p = multiFileNames;

        while (FileDlg_ParseFileNames(multiFileNames, path, &p)) {
            FILE *outFile = _tfopen(path, "w");
            ...
        }
    }
    _TUCHAR fileName[_MAX_PATH] = {0};
    if (FileDlg_Save(mainWnd
            , _T("Data Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0")
            , _T("dat")
            , fileName
            , _MAX_PATH
            , OFN_OVERWRITEPROMPT))
    {
        // save file
    }
 * return value:
 *      these functions return true if the user hits OK
 * notes:
 *      needs linkage with comdlg32.lib
 */
BOOL FileDlg_Open(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr
        , const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen
        , DWORD flags);
BOOL FileDlg_ParseFileNames(_TUCHAR *multiString, _TUCHAR *nextPath, _TUCHAR **p);
BOOL FileDlg_Save(HWND parentWnd, _TUCHAR *initialDir, const _TUCHAR *filterStr
        , const _TUCHAR *defaultExt, _TUCHAR *fileNameBuf, size_t fileNameBufLen
        , DWORD flags);
void FileDlg_SaveInitialDir(_TUCHAR *initialDir, const _TUCHAR *fileName);


#endif
