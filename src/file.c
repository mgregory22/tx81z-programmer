/*
 * file.c - file i/o wrapper module
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
extern void *File_Load(_TUCHAR *fileName, size_t *fileSize, _TUCHAR *errorMsg
        , size_t errorMsgLen);

/*
 * Procedure definitions
 */

/*
 * Load() - Loads a file into a malloc'ed buffer and returns it.  If <fileSize>
 *          is not null, the size of the buffer/file is returned.  If an error
 *          occurs, a message will be placed into errorMsg.
 */
void *File_Load(_TUCHAR *fileName, size_t *fileSize, _TUCHAR *errorMsg
        , size_t errorMsgLen)
{
    FILE *inFile;
    struct _stat statInfo;
    _off_t bytesRead;
    void *fileData;

    /*
     * Get the size of the file.
     */
    if (_tstat(fileName, &statInfo) == -1) {
        goto ErrorOut;
    }
             
    /*
     * Allocate a buffer to hold the file.
     */
    fileData = malloc(statInfo.st_size);
    if (!fileData) {
        errno = ENOMEM;
        goto ErrorOut;
    }
    
    /*
     * Open the file.
     */
    inFile = _tfopen(fileName, _T("rb"));
    if (!inFile) {
        free(fileData);
        goto ErrorOut;
    }

    /*
     * Read the file into the buffer.
     */
    bytesRead = fread(fileData, 1, statInfo.st_size, inFile);

    /*
     * Close the file.
     */
    fclose(inFile);

    if (bytesRead != statInfo.st_size) {
        free(fileData);
        goto ErrorOut;
    }
    if (fileSize) {
        *fileSize = statInfo.st_size;
    }

    return fileData;

ErrorOut:
#ifdef UNICODE
    _sntprintf(errorMsg, errorMsgLen, _T("Error opening %s: %S"), fileName
            , strerror(errno));
#else
    _sntprintf(errorMsg, errorMsgLen, _T("Error opening %s: %s"), fileName
            , strerror(errno));
#endif

    return NULL;
}
