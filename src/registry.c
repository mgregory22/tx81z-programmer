/*
 * registry.c - MSW registry access - version 2
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
 * Unit constants
 */
#define ROOT_KEY_PATH          _T("Software\\MSG Software\\")
#define ROOT_KEY_PATH_LEN      22
#ifdef UNICODE
#   define REGISTRY_KEYMAX     256
#else
#   define REGISTRY_KEYMAX     512
#endif

/*
 * Unit types
 */
typedef struct {
    unsigned ID;
    size_t entryCnt;
    REGENTRY *entries;
} REGENTRYGROUP;

struct REGISTRY {
    _TUCHAR baseKeyPath[ROOT_KEY_PATH_LEN + REGISTRY_KEYMAX];
    size_t entryGroupCnt;
    REGENTRYGROUP *entryGroups;
};
#define PREGISTRY HREGISTRY

/*
 * Unit constants
 */
const _TUCHAR r_rootKeyPath[] = ROOT_KEY_PATH;
#define r_rootKeyPathLen   ROOT_KEY_PATH_LEN

/*
 * Global procedures
 */
extern BOOL Registry_AddEntry(PREGISTRY registry, unsigned groupID
        , const _TUCHAR *subKeyName, const _TUCHAR *entryName, DWORD entryType
        , void *buf, size_t bufSize);
extern BOOL Registry_AddEntryIndirect(PREGISTRY registry, unsigned groupID
        , REGENTRY *entry);
extern PREGISTRY Registry_Create(const _TUCHAR *baseKeyPath);
extern void Registry_Destroy(PREGISTRY registry);
extern void Registry_DeleteGroup(PREGISTRY registry, unsigned groupID);
extern long Registry_ReadGroup(PREGISTRY registry, unsigned groupID
        , BOOL create);
extern long Registry_WriteGroup(PREGISTRY registry, unsigned groupID
        , BOOL create);

/*
 * Unit procedures
 */

/*
 * Procedure definitions
 */
BOOL Registry_AddEntry(PREGISTRY registry, unsigned groupID
        , const _TUCHAR *subKeyName, const _TUCHAR *entryName, DWORD entryType
        , void *buf, size_t bufSize)
{
    REGENTRY entry = {
        subKeyName,
        entryName,
        entryType,
        buf,
        bufSize
    };
    return Registry_AddEntryIndirect(registry, groupID, &entry);
}

BOOL Registry_AddEntryIndirect(PREGISTRY registry, unsigned groupID
        , REGENTRY *entry)
{
    int g, groupCnt = registry->entryGroupCnt;

    /*
     * Find the group in the REGISTRY structure that groupID refers to, if any.
     */
    for (g = 0; g < groupCnt; g++) {
        if (registry->entryGroups[g].ID == groupID)
            goto GROUPFOUND;
    }

    /*
     * Add a new group to the REGISTRY structure.
     */
    {
        REGENTRYGROUP *newGroups = realloc(registry->entryGroups
                , (groupCnt + 1) * sizeof(*newGroups));

        if (!newGroups) {
            Error_OnError(E_MALLOC_ERROR);
            return FALSE;
        }
        newGroups[groupCnt].ID = groupID;
        newGroups[groupCnt].entryCnt = 0;
        newGroups[groupCnt].entries = NULL;
        registry->entryGroups = newGroups;
        registry->entryGroupCnt = groupCnt + 1;
        g = groupCnt;
    }

GROUPFOUND:
    /*
     * Add the new entry to the group.
     */
    {
        REGENTRYGROUP *group = &registry->entryGroups[g];
        int entryCnt = group->entryCnt;
        REGENTRY *newEntries = realloc(group->entries
                , (entryCnt + 1) * sizeof(*newEntries));

        if (!newEntries) {
            Error_OnError(E_MALLOC_ERROR);
            return FALSE;
        }
        newEntries[entryCnt] = *entry;
        group->entryCnt = entryCnt + 1;
        group->entries = newEntries;
    }

    return TRUE;
}

PREGISTRY Registry_Create(const _TUCHAR *baseKeyPath)
{
    PREGISTRY registry = malloc(sizeof *registry);

    if (!registry) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    _tcscpy(registry->baseKeyPath, r_rootKeyPath);
    _tcsncpy(&registry->baseKeyPath[r_rootKeyPathLen], baseKeyPath
            , REGISTRY_KEYMAX);
    registry->entryGroupCnt = 0;
    registry->entryGroups = NULL;

    return registry;
}

void Registry_Destroy(PREGISTRY registry)
{
    size_t i;

    for (i = 0; i < registry->entryGroupCnt; i++) {
        free(registry->entryGroups[i].entries);
    }
    free(registry->entryGroups);
    free(registry);
}

void Registry_DeleteGroup(PREGISTRY registry, unsigned groupID)
{
    HKEY baseKey;
    int g, groupCnt = registry->entryGroupCnt;
    int e, entryCnt;
    long error;

    error = RegOpenKeyEx(HKEY_CURRENT_USER, registry->baseKeyPath, 0
            , KEY_QUERY_VALUE, &baseKey);

    for (g = 0; g < groupCnt; g++) {
        REGENTRYGROUP *group = &registry->entryGroups[g];

        if (groupID == REGISTRY_ALL_GROUPS || group->ID == groupID) {
            entryCnt = group->entryCnt;

            for (e = 0; e < entryCnt; e++) {
                REGENTRY *entry = &group->entries[e];

                if (entry->subKeyName) {
                    RegDeleteKey(baseKey, entry->subKeyName);
                    /*
                     * Just ignore errors - they're probably file not found
                     * errors.
                     */
                }
            }
        }
    }
    RegCloseKey(baseKey);
    if (groupID == REGISTRY_ALL_GROUPS) {
        RegDeleteKey(HKEY_CURRENT_USER, registry->baseKeyPath);
    }
}

long Registry_ReadGroup(PREGISTRY registry, unsigned groupID, BOOL create)
{
    HKEY baseKey;
    DWORD disposition;
    int g, groupCnt = registry->entryGroupCnt;
    int e, entryCnt;
    long error;

    if (create) {
        error = RegCreateKeyEx(HKEY_CURRENT_USER, registry->baseKeyPath, 0
                , NULL, REG_OPTION_NON_VOLATILE
                , KEY_CREATE_SUB_KEY | KEY_QUERY_VALUE, NULL, &baseKey
                , &disposition);
    } else {
        error = RegOpenKeyEx(HKEY_CURRENT_USER, registry->baseKeyPath, 0
                , KEY_QUERY_VALUE, &baseKey);
    }
    if (error) {
        return error;
    }
    for (g = 0; g < groupCnt; g++) {
        REGENTRYGROUP *group = &registry->entryGroups[g];

        if (groupID == REGISTRY_ALL_GROUPS || group->ID == groupID) {
            HKEY subKey = NULL;
            BOOL didOpenSubKey = FALSE;

            entryCnt = group->entryCnt;
            for (e = 0; e < entryCnt; e++) {
                REGENTRY *entry = &group->entries[e];

                if (entry->subKeyName) {
                    /*
                     * If the right subkey is not already open . . .
                     */
                    if ((didOpenSubKey == FALSE) || (e == 0)
                            || (group->entries[e - 1].subKeyName == NULL)
                            || (StrNotEq(entry->subKeyName
                                    , group->entries[e - 1].subKeyName)))
                    {
                        if (didOpenSubKey && subKey != baseKey) {
                            RegCloseKey(subKey);
                        }
                        if (create) {
                            error = RegCreateKeyEx(baseKey
                                    , entry->subKeyName, 0
                                    , NULL, REG_OPTION_NON_VOLATILE
                                    , KEY_CREATE_SUB_KEY | KEY_QUERY_VALUE
                                    , NULL, &subKey, NULL);
                        } else {
                            error = RegOpenKeyEx(baseKey
                                    , entry->subKeyName, 0
                                    , KEY_QUERY_VALUE
                                    , &subKey);
                        }
                        if (error) {
                            RegCloseKey(baseKey);
                            return error;
                        }
                        didOpenSubKey = TRUE;
                    }
                } else {
                    subKey = baseKey;
                }
                if (!create || disposition == REG_OPENED_EXISTING_KEY) {
                    DWORD bufSize = entry->bufSize;
                    BYTE *buf = malloc(bufSize);

                    if (buf) {
                        if (RegQueryValueEx(subKey, entry->entryName, 0, NULL
                                    , buf, &bufSize) == ERROR_SUCCESS)
                        {
                            if (bufSize == entry->bufSize
                                    || ((entry->entryType == REG_SZ
                                          || entry->entryType == REG_EXPAND_SZ)
                                        && bufSize < entry->bufSize)) {
                                memcpy(entry->buf, buf, bufSize);
                            } else {
                                /* not enough data was read from the registry */
                            }
                        }
                        free(buf);
                    }
                    /* deliberately ignoring errors here for both the
                     * allocation of buf and the result of RegQueryValueEx() */
                }
            }
            if (didOpenSubKey && subKey != baseKey) {
                RegCloseKey(subKey);
            }
        }
    }
    RegCloseKey(baseKey);
    return 0;
}

long Registry_WriteGroup(PREGISTRY registry, unsigned groupID, BOOL create)
{
    HKEY baseKey;
    int g, groupCnt = registry->entryGroupCnt;
    int e, entryCnt;
    long error;

    if (create) {
        error = RegCreateKeyEx(HKEY_CURRENT_USER, registry->baseKeyPath, 0
                , NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY
                    | KEY_SET_VALUE, NULL, &baseKey, NULL);
    } else {
        error = RegOpenKeyEx(HKEY_CURRENT_USER, registry->baseKeyPath, 0
                , KEY_SET_VALUE, &baseKey);
    }
    if (error) {
        return error;
    }
    for (g = 0; g < groupCnt; g++) {
        REGENTRYGROUP *group = &registry->entryGroups[g];

        if (group->ID == groupID || groupID == REGISTRY_ALL_GROUPS) {
            HKEY subKey = NULL;
            BOOL didOpenSubKey = FALSE;

            entryCnt = group->entryCnt;
            for (e = 0; e < entryCnt; e++) {
                REGENTRY *entry = &group->entries[e];
                size_t trueLen = entry->bufSize;

                if (entry->subKeyName) {
                    /*
                     * If the right subkey is not already open . . .
                     */
                    if ((didOpenSubKey == FALSE) || (e == 0)
                            || (group->entries[e - 1].subKeyName == NULL)
                            || (StrNotEq(entry->subKeyName
                                    , group->entries[e - 1].subKeyName)))
                    {
                        if (didOpenSubKey && subKey != baseKey) {
                            RegCloseKey(subKey);
                        }
                        if (create) {
                            error = RegCreateKeyEx(baseKey
                                    , entry->subKeyName, 0
                                    , NULL, REG_OPTION_NON_VOLATILE
                                    , KEY_CREATE_SUB_KEY | KEY_SET_VALUE
                                    , NULL, &subKey, NULL);
                        } else {
                            error = RegOpenKeyEx(baseKey
                                    , entry->subKeyName, 0
                                    , KEY_SET_VALUE
                                    , &subKey);
                        }
                        if (error) {
                            RegCloseKey(baseKey);
                            return error;
                        }
                        didOpenSubKey = TRUE;
                    }
                } else {
                    subKey = baseKey;
                }
                if (entry->entryType == REG_SZ
                        || entry->entryType == REG_EXPAND_SZ)
                {
                    trueLen = (_tcslen((_TUCHAR *) entry->buf) + 1)
                        * sizeof(_TUCHAR);
                    if (trueLen > entry->bufSize)
                        trueLen = entry->bufSize;
                }
                RegSetValueEx(subKey, entry->entryName, 0, entry->entryType
                        , entry->buf, trueLen);
                /* deliberately ignoring errors */
            }
            if (didOpenSubKey && subKey != baseKey) {
                RegCloseKey(subKey);
            }
        }
    }
    RegCloseKey(baseKey);
    return 0;
}

