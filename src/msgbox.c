/*
 * msgbox.c - message box functions
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
 * Local constants
 */
#define BUFLEN 256

/*
 * Global procedures
 */
extern void MsgBox_Error(HWND parentWnd, DWORD errorID);
extern void MsgBox_ErrorFva(HWND parentWnd, DWORD errorID, const _TUCHAR *fmt
        , va_list argList);
extern void MsgBox_F(HWND parentWnd, const _TUCHAR *fmt, ...);
extern void MsgBox_Fva(HWND parentWnd, const _TUCHAR *fmt, va_list argList);
extern void MsgBox_LastError(HWND parentWnd);
extern void MsgBox_LastErrorF(HWND parentWnd, const _TUCHAR *fmt, ...);
extern void MsgBox_LastErrorFva(HWND parentWnd, const _TUCHAR *fmt
        , va_list argList);


/*
 * Procedure definitions
 */
void MsgBox_Error(HWND parentWnd, DWORD errorID)
{
    _TUCHAR *sysBuf;
    MSG msg;

    /*
     * Get the error message string from the system.
     */
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, errorID,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (_TUCHAR *) &sysBuf, 0, NULL);
    /*
     * Empty the message queue.
     */
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        ;
    }
    /*
     * Display the message box.
     */
    MessageBox(parentWnd, sysBuf, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
    /*
     * Free the FormatMessage buffer
     */
    LocalFree((void *) sysBuf);
}

void MsgBox_ErrorFva(HWND parentWnd, DWORD errorID, const _TUCHAR *fmt
        , va_list argList)
{
    _TUCHAR *sysBuf;
    _TUCHAR msgBuf[BUFLEN];
    unsigned sysBufLen;
    int msgBufLen;
    int catLen;

    msgBufLen = _vsntprintf(msgBuf, BUFLEN, fmt, argList);

    if (msgBufLen < 0) {
        /* an error occurred while formatting the message - it was probably
         * too long for the buffer */
        MessageBox(parentWnd
            , _T("An error occured while formatting an error message.")
            , NULL, MB_OK | MB_ICONERROR);
        msgBuf[0] = '\0';
        msgBufLen = 0;
    }
    /*
     * Get the error message string from the system.
     */
    sysBufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, errorID,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (_TUCHAR *) &sysBuf, 0, NULL);
    /*
     * Concatenate the system message to the user message.
     */
    catLen = (msgBufLen + 2 + sysBufLen + 1 <= BUFLEN)
        ?  sysBufLen + 1  :  BUFLEN - msgBufLen;
    msgBuf[msgBufLen++] = ':';
    msgBuf[msgBufLen++] = ' ';
    _tcsncpy(&msgBuf[msgBufLen], sysBuf, catLen);
    msgBuf[BUFLEN - 1] = '\0';
    /*
     * Display the message box.
     */
    MessageBox(parentWnd, msgBuf, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
    /*
     * Free the FormatMessage buffer.
     */
    LocalFree(sysBuf);
}

void MsgBox_F(HWND parentWnd, const _TUCHAR *fmt, ...)
{
    va_list argList;

    va_start(argList, fmt);
    MsgBox_Fva(parentWnd, fmt, argList);
    va_end(argList);
}

void MsgBox_Fva(HWND parentWnd, const _TUCHAR *fmt, va_list argList)
{
    _TUCHAR msgBuf[BUFLEN];

    _vsntprintf(msgBuf, sizeof(msgBuf) / sizeof(msgBuf[0]), fmt, argList);

    MessageBox(parentWnd, msgBuf, NULL, MB_ICONINFORMATION);
}

void MsgBox_LastError(HWND parentWnd)
{
    MsgBox_Error(parentWnd, GetLastError());
}

void MsgBox_LastErrorF(HWND parentWnd, const _TUCHAR *fmt, ...)
{
    va_list argList;

    va_start(argList, fmt);
    MsgBox_ErrorFva(parentWnd, GetLastError(), fmt, argList);
    va_end(argList);
}

void MsgBox_LastErrorFva(HWND parentWnd, const _TUCHAR *fmt, va_list argList)
{
    MsgBox_ErrorFva(parentWnd, GetLastError(), fmt, argList);
}

