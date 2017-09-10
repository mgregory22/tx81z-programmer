/*
 * stdafx.h - precompiled header module
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tchar.h>
#include <cderr.h>
#include <commctrl.h>
#include <commdlg.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <mmsystem.h>
#include <htmlhelp.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "msg/cdvlist.h"
#include "msg/msgerror.h"
#include "msg/file.h"
#include "dlg/filedlg.h"
#include "msg/filepath.h"
#include "msg/func.h"
#include "msg/macros.h"
#include "msg/random.h"
#include "msg/str.h"
#include "gui/colors.h"
#include "gui/dialog.h"
#include "gui/guierror.h"
#include "gui/font.h"
#include "gui/minifont.h"
#include "gui/msgbox.h"
#include "gui/rect.h"
#include "gui/registry.h"
#include "gui/window.h"
#include "gui/winx.h"
#include "midi/midi.h"

//#include "aggopt.h"
/*
 * Change "variable was initialized but not referenced" and "unreferenced
 * local variable" warnings to errors
 */
#pragma warning(1:4189 4101)

