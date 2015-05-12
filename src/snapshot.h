/*
 * snapshot.h - Module to manage TX81Z snapshot data.
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
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_STDLIB
#   include <stdlib.h>
#endif
#ifndef TX81Z_H
#   include "tx81z.h"
#endif


/*
 * Global procedures
 */
void Snapshot_BundlePerformance(SNAPSHOT *snapshot, SINDEX sIdx
        , BYTE bndlBuf[S_MAX_BNDL_SIZE]);
BYTE Snapshot_Checksum(BYTE *buf, size_t bufSize);
void Snapshot_Clean(SNAPSHOT *snapshot);
void Snapshot_DirtyItem(SNAPSHOT *snapshot, SINDEX sIdx);
void Snapshot_FormatName(SNAPSHOT *snapshot, SINDEX sIdx, NAME_FORMAT fmtFlags
        , _TUCHAR *dest);
SINDEX Snapshot_GetBankIndex(SINDEX sIdx);
void Snapshot_GetItemName(SNAPSHOT *snapshot, SINDEX sIdx, _TUCHAR *dest);
METAINDEX Snapshot_GetItemType(SINDEX sIdx);
void Snapshot_Init(SNAPSHOT *snapshot);
BOOL Snapshot_IsBankItem(SINDEX sIdx);
BOOL Snapshot_IsBankType(type);
#define Snapshot_IsCombineModeOn(snapshot) ((snapshot)->sys.data[TX81Z_SYS_COMBINE_SW] == 1)
BOOL Snapshot_IsDirty(SNAPSHOT *snapshot);
BOOL Snapshot_IsItemDirty(SNAPSHOT *snapshot, SINDEX sIdx);
BOOL Snapshot_IsItemGroupLoaded(SNAPSHOT *snapshot, METAINDEX type);
BOOL Snapshot_IsItemsGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx);
BOOL Snapshot_IsNamedItem(SINDEX sIdx);
#define Snapshot_IsPCModeSetToInd(snapshot) ((snapshot)->sys.data[TX81Z_SYS_PROGCH_SW] == 2)
BOOL Snapshot_Load(HWND mainWnd, SNAPSHOT *snapshot);
size_t Snapshot_LoadSysex(SNAPSHOT *snapshot, METAINDEX type, const BYTE *buf);
SINDEX Snapshot_MemNumToSnapshotIndex(int memNum);
BOOL Snapshot_Save(HWND mainWnd, SNAPSHOT *snapshot);
void Snapshot_SetItemDirty(SNAPSHOT *snapshot, SINDEX sIdx);
void Snapshot_SetItemGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx);
void Snapshot_UpdateSysexWrappers(SNAPSHOT *snapshot);


#endif
