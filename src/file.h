/*
 * msg/file.h - file i/o wrapper module
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
#ifndef MSG_FILE_H
#define MSG_FILE_H

#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_IO
#   include <io.h>
#endif
#ifndef _INC_STDIO
#   include <stdio.h>
#endif

/*
 * File helper macros
 */
#define File_Exists(fileName) (_taccess((fileName), 0) == 0)


void *File_Load(_TUCHAR *fileName, size_t *fileSize, _TUCHAR *errorMsg
        , size_t errorMsgLen);


#endif /* #ifndef MSG_FILE_H */
