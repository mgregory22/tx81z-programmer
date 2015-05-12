/*
 * txpack.h - TX81Z parameter locations and ranges
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
#ifndef TXPACK_H
#define TXPACK_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef SNAPSHOT_H
#   include "snapshot.h"
#endif
#ifndef TX81Z_H
#   include "tx81z.h"
#endif


/*
 * Data types
 */
typedef struct {
    BYTE memOffset;
    BYTE memFirstBit;
    BYTE memBitCnt;
    BYTE loLimit;
    BYTE hiLimit;
    _TUCHAR *title;
} TXPACK;

/*
 * Constants
 */
#define TXPack_vcedCnt  S_VCED_DATA_SIZE
#define TXPack_acedCnt  S_ACED_DATA_SIZE
#define TXPack_pcedCnt  S_PCED_DATA_SIZE
#define TXPack_fxCnt    S_FX_DATA_SIZE
#define TXPack_sysCnt   S_SYS_DATA_SIZE
extern const TXPACK TXPack_vced[TXPack_vcedCnt];
extern const TXPACK TXPack_aced[TXPack_acedCnt];
extern const TXPACK TXPack_pced[TXPack_pcedCnt];
extern const TXPACK TXPack_fx[TXPack_fxCnt];
extern const TXPACK TXPack_sys[TXPack_sysCnt];
extern const BYTE TXPack_masks[8];


/*
 * Global procedures
 */

void TXPack_AvcedToVmem(const BYTE *acedData, BYTE *vcedData, BYTE *vmemData);
void TXPack_PcedToPmem(const BYTE *pcedData, BYTE *pmemData);
void TXPack_PmemToPced(const BYTE *pmemData, PCED *pced);
void TXPack_ValidateBuf(BYTE *buf, METAINDEX type);
void TXPack_VmemToAvced(const BYTE *vmemData, AVCED *avced, BOOL combine);


#endif  /* #ifndef TXPACK_H */
