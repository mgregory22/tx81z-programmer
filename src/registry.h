/*
 * gui/registry.h - MSW registry access - version 2
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
#ifndef GUI_REGISTRY_H
#define GUI_REGISTRY_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_LIMITS
#   include <limits.h>
#endif
#ifndef _INC_STDDEF
#   include <stddef.h>
#endif

/*
 * Constants
 */
#define REGISTRY_ALL_GROUPS    UINT_MAX

/*
 * Data types
 */
typedef struct {
    const _TUCHAR *subKeyName;
    const _TUCHAR *entryName;
    DWORD entryType;
    void *buf;
    size_t bufSize;
} REGENTRY;

typedef struct REGISTRY *HREGISTRY;

/*
 * Global procedures
 */

/*
 * Registry_AddEntry() and Registry_AddEntryIndirect() - 
 *       Adds a registry setting description to the specified group in a
 *       registry object.  subKeyName is a subkey under the base key given
 *       in the Registry_Create() call.  It can be NULL if you want the entry
 *       to go in the base key.  entryName is the name of the registry value.
 *       entryType is one of the REG_DWORD, REG_SZ, REG_BINARY, etc. constants.
 *       buf and bufSize refers to the variable to be stored.  groupID is a
 *       user-defined constant that allows you to read and write to the
 *       registry in multiple batches, each of which has a separate ID.
 */
BOOL Registry_AddEntry(HREGISTRY registry, unsigned groupID
        , const _TUCHAR *subKeyName, const _TUCHAR *entryName, DWORD entryType
        , void *buf, size_t bufSize);

BOOL Registry_AddEntryIndirect(HREGISTRY registry, unsigned groupID
        , REGENTRY *entry);

/*
 * Registry_Create() - Creates a registry object and returns a handle to it.
 *                     baseKeyPath should probably be the program name.
 */
HREGISTRY Registry_Create(const _TUCHAR *baseKeyPath);

/*
 * Registry_DeleteGroup() - Deletes a group of subkeys from the registry.  If
 *                          REGISTRY_ALL_GROUPS is passed as the groupID, then
 *                          the root key is deleted as well.
 */
void Registry_DeleteGroup(HREGISTRY registry, unsigned groupID);

/*
 * Registry_Destroy() - Frees a registry object.
 */
void Registry_Destroy(HREGISTRY registry);

/*
 * Registry_ReadGroup() - Reads a group of registry settings that were defined
 *                        the Registry_AddEntry() or Registry_AddEntryIndirect().
 *                        groupID is the number of the group to read.  If
 *                        groupID is REGISTRY_ALL_GROUPS, then all defined
 *                        groups are read.  The function returns 0 on success
 *                        or a winerror.h constant if there's an error.  If
 *                        there's an error reading an entry, then the
 *                        destination buffer will be unchanged.
 */
long Registry_ReadGroup(HREGISTRY registry, unsigned groupID, BOOL create);

/*
 * Registry_WriteGroup() - Writes a group of registry settings.  groupID is
 *                         the number of the group to write.  If groupID is
 *                         REGISTRY_ALL_GROUPS, then all defined groups are
 *                         written.  The function returns 0 on success
 *                         or a winerror.h constant if there's an error.
 */
long Registry_WriteGroup(HREGISTRY registry, unsigned groupID, BOOL create);


#endif
