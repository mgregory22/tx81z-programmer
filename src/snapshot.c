/*
 * snapshot.c - Module to manage TX81Z snapshot data.
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
#include "prog.h"
#include "tx81z.h"
#include "txpack.h"
#include "snapshot.h"

/*
 * Global procedures
 */
extern void Snapshot_BundlePerformance(SNAPSHOT *snapshot, SINDEX sIdx, BYTE bndlBuf[S_MAX_BNDL_SIZE]);
extern BYTE Snapshot_Checksum(BYTE *buf, size_t bufSize);
extern void Snapshot_Clean(SNAPSHOT *snapshot);
extern void Snapshot_DirtyItem(SNAPSHOT *snapshot, SINDEX sIdx);
extern void Snapshot_FormatName(SNAPSHOT *snapshot, SINDEX sIdx, NAME_FORMAT fmtFlags, _TUCHAR *dest);
extern SINDEX Snapshot_GetBankIndex(SINDEX sIdx);
extern void Snapshot_GetItemName(SNAPSHOT *snapshot, SINDEX sIdx, _TUCHAR *dest);
extern METAINDEX Snapshot_GetItemType(SINDEX sIdx);
extern void Snapshot_Init(SNAPSHOT *snapshot);
extern BOOL Snapshot_IsBankItem(SINDEX sIdx);
extern BOOL Snapshot_IsBankType(METAINDEX type);
extern BOOL Snapshot_IsDirty(SNAPSHOT *snapshot);
extern BOOL Snapshot_IsItemDirty(SNAPSHOT *snapshot, SINDEX sIdx);
extern BOOL Snapshot_IsItemGroupLoaded(SNAPSHOT *snapshot, METAINDEX type);
extern BOOL Snapshot_IsItemsGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx);
extern BOOL Snapshot_IsNamedItem(SINDEX sIdx);
extern BOOL Snapshot_Load(HWND mainWnd, SNAPSHOT *snapshot);
extern size_t Snapshot_LoadSysex(SNAPSHOT *snapshot, METAINDEX type, const BYTE *buf);
extern SINDEX Snapshot_MemNumToSnapshotIndex(int memNum);
extern BOOL Snapshot_Save(HWND mainWnd, SNAPSHOT *snapshot);
extern void Snapshot_SetItemDirty(SNAPSHOT *snapshot, SINDEX sIdx);
extern void Snapshot_SetItemGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx);
extern void Snapshot_UpdateSysexWrappers(SNAPSHOT *snapshot);

/*
 * Unit constants
 */
#define ERRORMSG_LEN  (_MAX_PATH + 80)

/*
 * Unit procedures
 */


/*
 * Procedure definitions
 */

/*
 * BundlePerformance() - Creates a performance bundle.  sIdx must be the
 *                       index of a performance in the snapshot.
 */
void Snapshot_BundlePerformance(SNAPSHOT *snapshot, SINDEX sIdx
        , BYTE bndlBuf[S_MAX_BNDL_SIZE])
{
    BYTE *next = bndlBuf;
    BNDL_FORMAT *bndlFmt;
    BYTE *pfm;
    int voices[8];
    int voiceCnt;
    int i;
    BOOL microtune = FALSE;

    /*
     * Save a pointer to the bundle format.
     */
    bndlFmt = (BNDL_FORMAT *) next;
    next += sizeof(BNDL_FORMAT);
    /*
     * Save a pointer to the performance.
     */
    pfm = next;
    /*
     * If the selected item is the edit buffer, pack it into the buffer.
     * Otherwise it will be a PMEM and it can just be copied.
     */
    if (sIdx == SI_PCED) {
        TXPack_PcedToPmem(Prog_snapshot.pced.data, next);
    } else {
        memcpy(next, snapshot->pmem.data[sIdx - SI_PMEM], S_PMEM_DATA_SIZE);
    }
    next += S_PMEM_DATA_SIZE;
    /*
     * Scan through the eight Max Notes parameters of the performance and
     * figure out how many voices it uses and what those voices are.  Ignore
     * duplicate voices.
     *
     * Each instrument in a PMEM takes up 8 bytes.  The Max Notes parameter
     * is in the low 4 bits of the first byte of each instrument.  The voice
     * number is in the second byte.
     *
     * Also set the microtune flag if any of the voices use micro tunings.
     */
    voiceCnt = 0;
    for (i = 0; i < 64; i += 8) {
        /*
         * If the voice has notes allocated.
         */
        if (pfm[i] & 0x0F) {
            int voiceNum = ((pfm[i] & 0x10) << 3) + pfm[i + 1];
            int j;

            /*
             * If the voice already exists in the list, don't add a duplicate.
             */
            for (j = 0; j < voiceCnt; j++) {
                if (voiceNum == voices[j]) {
                    goto Dupe;
                }
            }
            /*
             * Add the voice number to the list.
             */
            voices[voiceCnt] = voiceNum;
            voiceCnt++;
Dupe:
            /*
             * If micro tuning is turned on for this voice.
             */
            if (pfm[i + 6] & 0x40) {
                microtune = TRUE;
            }
        }
    }
    /*
     * Copy the voice count into the bundle format area.
     */
    *bndlFmt = voiceCnt - 1;
    /*
     * Copy the data for each voice into the bundle.
     */
    for (i = 0; i < voiceCnt; i++) {
        /*
         * Find the address of the source data and perform the copy.
         */
        int voiceNum = voices[i];
        int bank = voiceNum / 32;
        BYTE *vmem;
        
        voiceNum %= 32;
        vmem = (bank == 0) ? snapshot->vmem.data[voiceNum]
                : snapshot->preset[bank - 1].data[voiceNum];
        memcpy(next, vmem, S_VMEM_DATA_SIZE);
        next += S_VMEM_DATA_SIZE;
    }
    /*
     * If the effects are turned on in the performance, add the effect
     * settings to the bundle.
     */
    if (pfm[65] & 0x06) {
        *bndlFmt |= BF_FX;
        memcpy(next, snapshot->fx.data, S_FX_DATA_SIZE);
        next += S_FX_DATA_SIZE;
    }
    /*
     * If the any of the voices use micro tuning, and the table is set to
     * user-defined octave, copy the octave table.
     */
    if (microtune) {
        int microtuneTable = pfm[64] & 0x0F;

        if (microtuneTable == 0) {
            *bndlFmt |= BF_MTO;
            memcpy(next, snapshot->mto.data, S_MTO_DATA_SIZE);
            next += S_MTO_DATA_SIZE;
        } else if (microtuneTable == 1) {
            *bndlFmt |= BF_MTF;
            memcpy(next, snapshot->mtf.data, S_MTF_DATA_SIZE);
            next += S_MTF_DATA_SIZE;
        }
    }
}

/*
 * Checksum() - Calculates the checksum for a dump.
 */
BYTE Snapshot_Checksum(BYTE *buf, size_t bufSize)
{
    int checksum = 0;
    size_t i;

    for (i = 0; i < bufSize; i++, buf++) {
        checksum = (checksum + *buf) & 0xFF;
    }
    return (0x100 - checksum) & 0x7F;
}

/*
 * Clean() - Clears all dirty flags.
 */
void Snapshot_Clean(SNAPSHOT *snapshot)
{
    int i;

    for (i = 0; i < 7; i++) {
        snapshot->extra.itemDirty[i] = 0;
    }
}

/*
 * DirtyItem() - Sets a group's dirty flag.
 */
void Snapshot_DirtyItem(SNAPSHOT *snapshot, SINDEX sIdx)
{
    assert(sIdx >= 0 && sIdx < SI_CNT);
    //Snapshot_SetItemGroupLoaded(snapshot, sIdx);
    snapshot->extra.loaded |= 1 << Snapshot_GetItemType(sIdx);
    snapshot->extra.itemDirty[sIdx >> 5] |= 1 << (sIdx & 0x1F);
}

/*
 * FormatName() - Creates display strings for snapshot items.
 */
void Snapshot_FormatName(SNAPSHOT *snapshot, SINDEX sIdx, NAME_FORMAT fmtFlags
        , _TUCHAR *dest)
{
    METAINDEX type = Snapshot_GetItemType(sIdx);
    int bankIndex = Snapshot_GetBankIndex(sIdx);
    int i = 0;

    if (fmtFlags & NF_NUMBER) {
        i += _stprintf(dest, TX81Z_meta[type].typeStr, bankIndex + 1);
        /*
         * Insert a dash or a space, but only if NF_NUMBER was specified.
         */
        if (fmtFlags & NF_DASH) {
            /*
             * Some type names have extra spaces after them, which are undesirable
             * in dashed format.
             */
            while (dest[i - 1] == ' ') {
                i--;
            }
            _tcscpy(&dest[i], _T(" - "));
            i += 3;
        } else {
            /*
             * Add a space for the divider.
             */
            _tcscpy(&dest[i], _T(" "));
            i++;
        }
    }
    if (fmtFlags & NF_NAME) {
        if (Snapshot_IsItemGroupLoaded(snapshot, type)) {
            if (Snapshot_IsNamedItem(sIdx)) {
                Snapshot_GetItemName(snapshot, sIdx, &dest[i]);
            } else {
                _tcscpy(&dest[i], TX81Z_meta[type].nameFmt);
            }
        } else {
            MakeSpacer(&dest[i], ' ', 10);
        }
    }
}

/*
 * GetBankIndex() - Returns the index of an item relative to the beginning of
 *                  the bank that it is in.
 */
SINDEX Snapshot_GetBankIndex(SINDEX sIdx)
{
    if (sIdx < SI_VMEM || sIdx >= SI_CNT)
        return -1;
    if (sIdx < SI_PMEM)
        return sIdx - SI_VMEM;
    if (sIdx < SI_PRESET_A)
        return sIdx - SI_PMEM;
    if (sIdx < SI_PRESET_B)
        return sIdx - SI_PRESET_A;
    if (sIdx < SI_PRESET_C)
        return sIdx - SI_PRESET_B;
    if (sIdx < SI_PRESET_D)
        return sIdx - SI_PRESET_C;
    return sIdx - SI_PRESET_D;
}

/*
 * GetItemName() -
 */
void Snapshot_GetItemName(SNAPSHOT *snapshot, SINDEX sIdx, _TUCHAR *dest)
{
    char *src;
    int i;

    if (sIdx == SI_VCED) {
        src = &snapshot->avced.vced.data[TX81Z_VCED_NAME];
    } else if (sIdx == SI_PCED) {
        src = &snapshot->pced.data[TX81Z_PCED_NAME];
    } else if (sIdx >= SI_VMEM && sIdx < SI_PMEM) {
        src = &snapshot->vmem.data[sIdx - SI_VMEM][TX81Z_VMEM_NAME];
    } else if (sIdx >= SI_PMEM && sIdx < SI_PRESET_A) {
        src = &snapshot->pmem.data[sIdx - SI_PMEM][TX81Z_PMEM_NAME];
    } else if (sIdx >= SI_PRESET_A && sIdx < SI_PRESET_B) {
        src = &snapshot->preset[0].data[sIdx - SI_PRESET_A][TX81Z_VMEM_NAME];
    } else if (sIdx >= SI_PRESET_B && sIdx < SI_PRESET_C) {
        src = &snapshot->preset[1].data[sIdx - SI_PRESET_B][TX81Z_VMEM_NAME];
    } else if (sIdx >= SI_PRESET_C && sIdx < SI_PRESET_D) {
        src = &snapshot->preset[2].data[sIdx - SI_PRESET_C][TX81Z_VMEM_NAME];
    } else if (sIdx >= SI_PRESET_D && sIdx < SI_CNT) {
        src = &snapshot->preset[3].data[sIdx - SI_PRESET_D][TX81Z_VMEM_NAME];
    } else {
        return;
    }
    for (i = 0; i < 10; i++) {
        dest[i] = src[i] ? src[i] : ' ';
    }
    dest[10] = '\0';
}

/*
 * GetItemType() - Returns the data type associated with a memory number.
 */
METAINDEX Snapshot_GetItemType(SINDEX sIdx)
{
    if (sIdx <= SI_VMEM)
        return sIdx;
    if (sIdx < SI_PMEM)
        return META_VMEM;
    if (sIdx < SI_PRESET_A)
        return META_PMEM;
    if (sIdx < SI_PRESET_B)
        return META_PRESET_A;
    if (sIdx < SI_PRESET_C)
        return META_PRESET_B;
    if (sIdx < SI_PRESET_D)
        return META_PRESET_C;
    return META_PRESET_D;
}

/*
 * Init() - Clears the snapshot and initializes housekeeping fields.
 */
void Snapshot_Init(SNAPSHOT *snapshot)
{
    /*
     * Clear the snapshot.
     */
    memset(snapshot, 0, sizeof *snapshot);
}

/*
 * IsBankItem() - Returns true if the item is a bank item.
 */
BOOL Snapshot_IsBankItem(SINDEX sIdx)
{
    METAINDEX type = Snapshot_GetItemType(sIdx);

    return Snapshot_IsBankType(type);
}

/*
 * IsBankType() - Returns true if the type index represents a bank.
 */
BOOL Snapshot_IsBankType(METAINDEX type)
{
    return ((type == META_VMEM) || (type == META_PMEM)
            || ((type >= META_PRESET_A) && (type <= META_PRESET_D)));
}

/*
 * IsDirty() - Returns true if any part of the snapshot is dirty.
 */
BOOL Snapshot_IsDirty(SNAPSHOT *snapshot)
{
    SINDEX sIdx;

    for (sIdx = 0; sIdx < SI_CNT; sIdx++) {
        if ((snapshot->extra.itemDirty[sIdx >> 5] & (1 << (sIdx & 0x1F))) != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * IsItemDirty() - Returns true if the item's group is marked dirty.
 */
BOOL Snapshot_IsItemDirty(SNAPSHOT *snapshot, SINDEX sIdx)
{
    assert(sIdx >= 0 && sIdx < SI_CNT);
    return (snapshot->extra.itemDirty[sIdx >> 5] & (1 << (sIdx & 0x1F))) != 0;
}

/*
 * IsItemGroupLoaded() - Returns true if the item group as identified by the
 *                       item type is loaded.
 */
BOOL Snapshot_IsItemGroupLoaded(SNAPSHOT *snapshot, METAINDEX type)
{
    if (type == META_ACED) {
        type = META_VCED;
    }
    return (snapshot->extra.loaded & (1 << type)) != 0; 
}

/*
 * IsItemsGroupLoaded() - Returns true if the item's group is marked loaded.
 */
BOOL Snapshot_IsItemsGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx)
{
    assert(sIdx >= 0 && sIdx < SI_CNT);

    return (snapshot->extra.loaded & (1 << Snapshot_GetItemType(sIdx))) != 0;
}

/*
 * IsNamedItem() - Returns true if the item has a user-defined name.
 */
BOOL Snapshot_IsNamedItem(SINDEX sIdx)
{
    METAINDEX type = Snapshot_GetItemType(sIdx);

    return Snapshot_IsBankType(type) || type == META_VCED || type == META_PCED;
}

/*
 * Load() - Loads a snapshot file from disk.
 */
BOOL Snapshot_Load(HWND mainWnd, SNAPSHOT *snapshot)
{
    _TUCHAR errorMsg[ERRORMSG_LEN];
    size_t fileSize;
    size_t i;
    int vmemsLoaded = 0;
    BYTE *data = File_Load(snapshot->extra.fileName, &fileSize, errorMsg
            , ERRORMSG_LEN);

    if (!data) {
        MsgBox_F(mainWnd, errorMsg);
        return FALSE;
    }
    /*
     * Search through file data for sysex headers.
     */
    for (i = 0; i < fileSize; i++) {
        if (data[i] == 0xF0 && data[i + 1] == 0x43) {
            METAINDEX type = TX81Z_DetectSysexType(NULL, &data[i]);

            if (type == -1) {
                    /*
                     * The buffer is not a valid sysex dump - skip a byte
                     * since it's already been determined that it's 0x43.
                     */
                    i++;
            } else if (type == META_VMEM) {
                /*
                 * The first VMEM found is loaded into the internal bank,
                 * the second is loaded into preset bank A, etc.  If more
                 * than five are found the rest are ignored.
                 */
                if (vmemsLoaded < 5) {
                    if (vmemsLoaded > 0) {
                        type = META_PRESET_A + vmemsLoaded - 1;
                    }
                    vmemsLoaded++;
                } else {
                    continue;
                }
                goto Load;
            } else {
Load:
                i += Snapshot_LoadSysex(snapshot, type, &data[i]) - 1;
            }
        }
    }
    /*
     * Delete temporary file data buffer.
     */
    free(data);

    return TRUE;
}

/*
 * LoadSysex() - Copies data into the snapshot structure and does housekeeping.
 */
size_t Snapshot_LoadSysex(SNAPSHOT *snapshot, METAINDEX type, const BYTE *buf)
{
    TX81ZMETA *meta = &TX81Z_meta[type];
    /* ssBuf is used as a cast to BYTE * for pointer arithmetic */
    BYTE *ssBuf = ((BYTE *) &snapshot->avced) + meta->offset;
    int identical = memcmp(ssBuf + 3, buf + 3, meta->bufSize - 3) == 0;
    BOOL notLoaded = !Snapshot_IsItemGroupLoaded(snapshot, type);

    /*
     * If the data is not the same as what already exists in the snapshot.
     */
    if (notLoaded || !identical) {
        /*
         * If the sysex message received is a bank of patches.
         */
        if (Snapshot_IsBankType(type)) {
            int i;
            int max = type == META_PMEM ? 24 : 32;
            int patchSize = type == META_PMEM ? S_PMEM_DATA_SIZE
                : S_VMEM_DATA_SIZE;

            /*
             * Step through each patch and figure out which ones are about to
             * change.
             */
            for (i = 0; i < max; i++) {
                int offset = meta->dumpHdrLen + i * patchSize;

                if (memcmp(ssBuf + offset, buf + offset, patchSize) != 0) {
                    Snapshot_DirtyItem(snapshot, meta->sIndex + i);
                }
            }
        /*
         * If the type of the data has a corresponding snapshot group.
         */
        } else if (meta->sIndex >= 0) {
            if (!identical) {
                /*
                 * Mark that group as dirty.
                 */
                Snapshot_DirtyItem(snapshot, meta->sIndex);
            }
        /*
         * If the type is ACED then dirty the VCED.
         */
        } else if (type == META_ACED) {
            if (!identical) {
                Snapshot_DirtyItem(snapshot, SI_VCED);
            }
        }
        /*
         * Copy the data from buf into the snapshot.
         */
        memcpy(ssBuf, buf, meta->bufSize);
        /*
         * Validate the sysex data.
         */
        TXPack_ValidateBuf(ssBuf, type);
        /*
         * Mark the group as loaded, unless it's an ACED, since loading an ACED
         * isn't a full voice edit buffer.
         */
        if (type != META_ACED) {
            Snapshot_SetItemGroupLoaded(snapshot, meta->sIndex);
        }
    }

    return meta->bufSize;
}

/*
 * MemNumToSnapshotIndex() - Converts a memory number from a dump to a snapshot
 *                           index.
 */
SINDEX Snapshot_MemNumToSnapshotIndex(int memNum)
{
    if (memNum <= 31) {
        return memNum + SI_VMEM;
    } else if (memNum <= 63) {
        return memNum - 32 + SI_PRESET_A;
    } else if (memNum <= 95) {
        return memNum - 64 + SI_PRESET_B;
    } else if (memNum <= 127) {
        return memNum - 96 + SI_PRESET_C;
    } else if (memNum <= 159) {
        return memNum - 128 + SI_PRESET_D;
    } else if (memNum <= 183) {
        return memNum - 160 + SI_PMEM;
    } else {
        return -1;
    }
}

/*
 * Save() - Saves the snapshot to disk.
 */
BOOL Snapshot_Save(HWND mainWnd, SNAPSHOT *snapshot)
{
    _TUCHAR errorMsg[ERRORMSG_LEN];
    _TUCHAR *path = snapshot->extra.fileName;
    FILE *outFile;
    METAINDEX group;

    Snapshot_UpdateSysexWrappers(&Prog_snapshot);
    outFile = _tfopen(path, _T("wb"));
    if (!outFile) {
        _stprintf(errorMsg, Prog_fileOpenErrorFmt, path);
        goto FILEERROR;
    }
    /*
     * Write the loaded items groups to the file.
     */
    for (group = META_VCED; group <= META_PRESET_D; group++) {
        if (Snapshot_IsItemGroupLoaded(snapshot, group)) {
            TX81ZMETA *meta = &TX81Z_meta[group];
            BYTE *ssBuf = ((BYTE *) &snapshot->avced) + meta->offset;

            /*
             * Write the ACED before the VCED.
             */
            if (group == META_VCED) {
                TX81ZMETA *meta = &TX81Z_meta[META_ACED];
                BYTE *ssBuf = ((BYTE *) &snapshot->avced) + meta->offset;

                if (fwrite(ssBuf, meta->bufSize, 1, outFile) != 1) {
                    goto FILEERROR;
                }
            }
            /*
             * Write the data.
             */
            if (fwrite(ssBuf, meta->bufSize, 1, outFile) != 1) {
                goto FILEERROR;
            }
        }
    }
    fclose(outFile);

    return TRUE;

FILEERROR:
    _stprintf(errorMsg, Prog_fileWriteErrorFmt, path);
    FromAnsiNCat(errorMsg, strerror(errno), ERRORMSG_LEN);
    MsgBox_F(mainWnd, errorMsg);
    if (outFile)
        fclose(outFile);

    return FALSE;
}

/*
 * SetItemDirty() - Sets an item's group as dirty.
 */
void Snapshot_SetItemDirty(SNAPSHOT *snapshot, SINDEX sIdx)
{
    snapshot->extra.itemDirty[sIdx >> 5] |= 1 << (sIdx & 0x1F);
}

/*
 * SetItemGroupLoaded() - Sets an item's group as loaded.
 */
void Snapshot_SetItemGroupLoaded(SNAPSHOT *snapshot, SINDEX sIdx)
{
    snapshot->extra.loaded |= 1 << Snapshot_GetItemType(sIdx);
}

/*
 * UpdateSysexWrappers() - Copies the sysex headers and footers and calculates
 *                         the checksum for all dumps in the snapshot.
 */
void Snapshot_UpdateSysexWrappers(SNAPSHOT *snapshot)
{
    int i;

    /*
     * Copy the sysex headers and footers.
     */
    for (i = 0; i <= META_ACED; i++) {
        BYTE *buf = (BYTE *) &snapshot->avced + TX81Z_meta[i].offset;

        memcpy(buf, TX81Z_meta[i].dumpHdr, TX81Z_meta[i].dumpHdrLen);
        buf += TX81Z_meta[i].bufSize - 1;
        *buf = '\xF7';
    }
    /*
     * Update ACED checksum.  The + 10 on the size is for the "LM  ??????"
     * string in the header, which is included in the checksum.
     */
    snapshot->avced.aced.footer[0] = Snapshot_Checksum(
            &snapshot->avced.aced.header[6]
            , sizeof snapshot->avced.aced.data + 10);

    /*
     * Update VCED checksum.
     */
    snapshot->avced.vced.footer[0] = Snapshot_Checksum(
            &snapshot->avced.vced.data[0]
            , sizeof snapshot->avced.vced.data);

    /*
     * Update PCED checksum.
     */
    snapshot->pced.footer[0] = Snapshot_Checksum(&snapshot->pced.header[6]
            , sizeof snapshot->pced.data + 10);

    /*
     * Update VMEM checksum.
     */
    snapshot->vmem.footer[0] = Snapshot_Checksum(&snapshot->vmem.data[0][0]
            , sizeof snapshot->vmem.data);

    /*
     * Update PMEM checksum.
     */
    snapshot->pmem.footer[0] = Snapshot_Checksum(&snapshot->pmem.header[6]
            , sizeof snapshot->pmem.data + 10);

    /*
     * Update system checksum.
     */
    snapshot->sys.footer[0] = Snapshot_Checksum(&snapshot->sys.header[6]
            , sizeof snapshot->sys.data + 10);

    /*
     * Update progCh checksum.
     */
    snapshot->pc.footer[0] = Snapshot_Checksum(&snapshot->pc.header[6]
            , sizeof snapshot->pc.data + 10);

    /*
     * Update effects checksum.
     */
    snapshot->fx.footer[0] = Snapshot_Checksum(&snapshot->fx.header[6]
            , sizeof snapshot->fx.data + 10);

    /*
     * Update microOctave checksum.
     */
    snapshot->mto.footer[0] = Snapshot_Checksum(&snapshot->mto.header[6]
            , sizeof snapshot->mto.data + 10);

    /*
     * Update microFull checksum.
     */
    snapshot->mtf.footer[0] = Snapshot_Checksum(&snapshot->mtf.header[6]
            , sizeof snapshot->mtf.data + 10);
}

