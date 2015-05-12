/*
 * tx81z.h - interface to TX81Z synthesizer
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
#ifndef TX81Z_H
#define TX81Z_H

#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef MIDI_MIDI_H
#   include "midi.h"
#endif

/*
 * Notification messages
 */
#define TXN_RECEIVED_ACED            1
#define TXN_RECEIVED_VCED            2
#define TXN_RECEIVED_PCED            3
#define TXN_VERSION_10_DETECTED      4
#define TXN_VERSION_OTHER_DETECTED   5

/*
 * Dump request type flags
 */
typedef enum {
    REQ_VCED         = 0x00000001,  /* 0 */
    REQ_PCED         = 0x00000002,  /* 1 */
    REQ_FX           = 0x00000004,  /* 2 */
    REQ_PC           = 0x00000008,  /* 3 */
    REQ_MTO          = 0x00000010,  /* 4 */
    REQ_MTF          = 0x00000020,  /* 5 */
    REQ_SYS          = 0x00000040,  /* 6 */
    REQ_VMEM         = 0x00000080,  /* 7 */
    REQ_PMEM         = 0x00000100,  /* 8 */
    REQ_PRESET_A     = 0x00000200,  /* 9 */
    REQ_PRESET_B     = 0x00000400,  /* 10 */
    REQ_PRESET_C     = 0x00000800,  /* 11 */
    REQ_PRESET_D     = 0x00001000,  /* 12 */
    REQ_ACED         = 0x00002000,  /* 13 */
    REQ_VOICE_MODE   = 0x00004000,  /* 14 */
    REQ_PFM_MODE     = 0x00008000,  /* 15 */
    REQ_AVCED        = REQ_VCED | REQ_ACED,
    REQ_EDIT_BUFFERS = REQ_ACED | REQ_VCED | REQ_PCED,
    REQ_EDITABLE     = REQ_ACED | REQ_VCED | REQ_PCED | REQ_FX
        | REQ_PC | REQ_MTO | REQ_MTF | REQ_SYS,
    REQ_PRESETS      = REQ_PRESET_A | REQ_PRESET_B | REQ_PRESET_C
        | REQ_PRESET_D,
    REQ_MODES        = REQ_VOICE_MODE | REQ_PFM_MODE,
    REQ_NEEDS_PROTECT_CHECK = REQ_FX | REQ_PC | REQ_MTO | REQ_MTF | REQ_SYS
        | REQ_VMEM | REQ_PMEM | REQ_PRESET_A | REQ_PRESET_B | REQ_PRESET_C
        | REQ_PRESET_D,
} REQUEST;

/*
 * Indexes of types in the TX81Z_meta array
 */
typedef enum {
    META_VCED        = 0,
    META_PCED        = 1,
    META_FX          = 2,
    META_PC          = 3,
    META_MTO         = 4,
    META_MTF         = 5,
    META_SYS         = 6,
    META_VMEM        = 7,
    META_PMEM        = 8,
    META_PRESET_A    = 9,
    META_PRESET_B    = 10,
    META_PRESET_C    = 11,
    META_PRESET_D    = 12,
    META_ACED        = 13,
    META_VOICE_MODE  = 14,
    META_PFM_MODE    = 15,
    META_CNT         = 16,
    META_BNDL        = 17,
} METAINDEX;

/*
 * Indexes of item groups in the snapshot flags arrays and list box
 */
typedef enum {
    SI_VCED     =   0,
    SI_PCED     =   1,
    SI_FX       =   2,
    SI_PC       =   3,
    SI_MTO      =   4,
    SI_MTF      =   5,
    SI_SYS      =   6,
    SI_VMEM     =   7,
    SI_PMEM     =  39,
    SI_PRESET_A =  63,
    SI_PRESET_B =  95,
    SI_PRESET_C = 127,
    SI_PRESET_D = 159,
    SI_CNT      = 191,
} SINDEX; /* snapshot item indexes */

typedef enum {
    NF_NUMBER                  = 0x01,
    NF_DASH                    = 0x02,
    NF_NAME                    = 0x04,
    NF_NUMBER_AND_NAME         = NF_NUMBER | NF_NAME,
    NF_DASHED                  = NF_NUMBER | NF_DASH | NF_NAME,
    NF_TYPE                    = 0x08,
    NF_COMMENT                 = 0x10,
    NF_TYPE_NAME_AND_COMMENT   = NF_TYPE | NF_NAME | NF_COMMENT,
} NAME_FORMAT;

typedef enum {
    BF_VOICE_MASK = 0x07,
    BF_FX         = 0x80,
    BF_MTO        = 0x10,
    BF_MTF        = 0x20,
} BNDL_FORMAT;

/*
 * Snapshot size constants
 */
#define S_FTR_SIZE          2
#define S_ACED_HDR_SIZE    16
#define S_ACED_DATA_SIZE   23
#define S_ACED_SIZE         (S_ACED_HDR_SIZE + S_ACED_DATA_SIZE + S_FTR_SIZE)
#define S_VCED_HDR_SIZE     6
#define S_VCED_DATA_SIZE   93
#define S_VCED_SIZE         (S_VCED_HDR_SIZE + S_VCED_DATA_SIZE + S_FTR_SIZE)
#define S_PCED_HDR_SIZE    16
#define S_PCED_DATA_SIZE  110
#define S_PCED_SIZE         (S_PCED_HDR_SIZE + S_PCED_DATA_SIZE + S_FTR_SIZE)
#define S_FX_HDR_SIZE      16
#define S_FX_DATA_SIZE     55
#define S_FX_SIZE           (S_FX_HDR_SIZE + S_FX_DATA_SIZE + S_FTR_SIZE)
#define S_PC_HDR_SIZE      16
#define S_PC_DATA_SIZE     256
#define S_PC_SIZE           (S_PC_HDR_SIZE + S_PC_DATA_SIZE + S_FTR_SIZE)
#define S_MTO_HDR_SIZE     16
#define S_MTO_DATA_SIZE    24
#define S_MTO_SIZE          (S_MTO_HDR_SIZE + S_MTO_DATA_SIZE + S_FTR_SIZE)
#define S_MTF_HDR_SIZE     16
#define S_MTF_DATA_SIZE   256
#define S_MTF_SIZE          (S_MTF_HDR_SIZE + S_MTF_DATA_SIZE + S_FTR_SIZE)
#define S_SYS_HDR_SIZE     16
#define S_SYS_DATA_SIZE    27
#define S_SYS_SIZE          (S_SYS_HDR_SIZE + S_SYS_DATA_SIZE + S_FTR_SIZE)
#define S_VMEM_HDR_SIZE     6
#define S_VMEM_DATA_SIZE  128
#define S_VMEM_SIZE         (S_VMEM_HDR_SIZE + 32 * S_VMEM_DATA_SIZE + S_FTR_SIZE)
#define S_PMEM_HDR_SIZE    16
#define S_PMEM_DATA_SIZE   76
#define S_PMEM_SIZE         (S_PMEM_HDR_SIZE + 32 * S_PMEM_DATA_SIZE + S_FTR_SIZE)
#define S_MAX_BNDL_SIZE (sizeof(BNDL_FORMAT) + S_PMEM_DATA_SIZE \
            + 8 * S_VMEM_DATA_SIZE + S_FX_DATA_SIZE + S_MTF_DATA_SIZE)

/*
 * Parameter change subgroup constants
 */
#define TX81Z_SUBGRP_VCED     2
#define TX81Z_SUBGRP_ACED     3
#define TX81Z_SUBGRP_PCED     0
#define TX81Z_SUBGRP_FX       4
#define TX81Z_SUBGRP_SYS      5
#define TX81Z_SUBGRP_REMOTE   3
#define TX81Z_SUBGRP_MTO      6
#define TX81Z_SUBGRP_MTF      7

/*
 * Program change bank constants
 */
#define TX81Z_BANK_I          0
#define TX81Z_BANK_A          1
#define TX81Z_BANK_B          2
#define TX81Z_BANK_C          3
#define TX81Z_BANK_D          4
#define TX81Z_BANK_PF         5

/*
 * Parameter index constants
 */
#define TX81Z_VMEM_NAME                 57
#define TX81Z_PMEM_NAME                 66
#define TX81Z_NAME_LEN                  10
#define TX81Z_STARTUP_MSG_LEN           16
#define TX81Z_VCED_OP_SIZE              13
#define TX81Z_VCED_OP4_AR                0
#define TX81Z_VCED_OP4_D1R               1
#define TX81Z_VCED_OP4_D2R               2
#define TX81Z_VCED_OP4_RR                3
#define TX81Z_VCED_OP4_D1L               4
#define TX81Z_VCED_OP4_LS                5
#define TX81Z_VCED_OP4_RS                6
#define TX81Z_VCED_OP4_EBS               7
#define TX81Z_VCED_OP4_AME               8
#define TX81Z_VCED_OP4_KVS               9
#define TX81Z_VCED_OP4_OUT              10
#define TX81Z_VCED_OP4_CRS              11
#define TX81Z_VCED_OP4_DET              12
#define TX81Z_VCED_OP2_AR               13
#define TX81Z_VCED_OP2_D1R              14
#define TX81Z_VCED_OP2_D2R              15
#define TX81Z_VCED_OP2_RR               16
#define TX81Z_VCED_OP2_D1L              17
#define TX81Z_VCED_OP2_LS               18
#define TX81Z_VCED_OP2_RS               19
#define TX81Z_VCED_OP2_EBS              20
#define TX81Z_VCED_OP2_AME              21
#define TX81Z_VCED_OP2_KVS              22
#define TX81Z_VCED_OP2_OUT              23
#define TX81Z_VCED_OP2_CRS              24
#define TX81Z_VCED_OP2_DET              25
#define TX81Z_VCED_OP3_AR               26
#define TX81Z_VCED_OP3_D1R              27
#define TX81Z_VCED_OP3_D2R              28
#define TX81Z_VCED_OP3_RR               29
#define TX81Z_VCED_OP3_D1L              30
#define TX81Z_VCED_OP3_LS               31
#define TX81Z_VCED_OP3_RS               32
#define TX81Z_VCED_OP3_EBS              33
#define TX81Z_VCED_OP3_AME              34
#define TX81Z_VCED_OP3_KVS              35
#define TX81Z_VCED_OP3_OUT              36
#define TX81Z_VCED_OP3_CRS              37
#define TX81Z_VCED_OP3_DET              38
#define TX81Z_VCED_OP1_AR               39
#define TX81Z_VCED_OP1_D1R              40
#define TX81Z_VCED_OP1_D2R              41
#define TX81Z_VCED_OP1_RR               42
#define TX81Z_VCED_OP1_D1L              43
#define TX81Z_VCED_OP1_LS               44
#define TX81Z_VCED_OP1_RS               45
#define TX81Z_VCED_OP1_EBS              46
#define TX81Z_VCED_OP1_AME              47
#define TX81Z_VCED_OP1_KVS              48
#define TX81Z_VCED_OP1_OUT              49
#define TX81Z_VCED_OP1_CRS              50
#define TX81Z_VCED_OP1_DET              51
#define TX81Z_VCED_ALGORITHM            52
#define TX81Z_VCED_FEEDBACK             53
#define TX81Z_VCED_LFO_SPEED            54
#define TX81Z_VCED_LFO_DELAY            55
#define TX81Z_VCED_LFO_PMD              56
#define TX81Z_VCED_LFO_AMD              57
#define TX81Z_VCED_LFO_SYNC             58
#define TX81Z_VCED_LFO_WAVE             59
#define TX81Z_VCED_PMS                  60
#define TX81Z_VCED_AMS                  61
#define TX81Z_VCED_TRANSPOSE            62
#define TX81Z_VCED_POLY_MODE            63
#define TX81Z_VCED_PB_RANGE             64
#define TX81Z_VCED_PORT_MODE            65
#define TX81Z_VCED_PORT_TIME            66
#define TX81Z_VCED_FC_VOL               67
#define TX81Z_VCED_SUSTAIN              68
#define TX81Z_VCED_PORTAMENTO           69
#define TX81Z_VCED_CHORUS               70
#define TX81Z_VCED_MW_PITCH             71
#define TX81Z_VCED_MW_AMP               72
#define TX81Z_VCED_BC_PITCH             73
#define TX81Z_VCED_BC_AMP               74
#define TX81Z_VCED_BC_PITCH_BIAS        75
#define TX81Z_VCED_BC_EG_BIAS           76
#define TX81Z_VCED_NAME                 77
#define TX81Z_VCED_OP_ENABLE            93
#define TX81Z_ACED_OP_SIZE               5
#define TX81Z_ACED_OP4_FIXED             0
#define TX81Z_ACED_OP4_RANGE             1
#define TX81Z_ACED_OP4_FINE              2
#define TX81Z_ACED_OP4_WAVE              3
#define TX81Z_ACED_OP4_SHIFT             4
#define TX81Z_ACED_OP2_FIXED             5
#define TX81Z_ACED_OP2_RANGE             6
#define TX81Z_ACED_OP2_FINE              7
#define TX81Z_ACED_OP2_WAVE              8
#define TX81Z_ACED_OP2_SHIFT             9
#define TX81Z_ACED_OP3_FIXED            10
#define TX81Z_ACED_OP3_RANGE            11
#define TX81Z_ACED_OP3_FINE             12
#define TX81Z_ACED_OP3_WAVE             13
#define TX81Z_ACED_OP3_SHIFT            14
#define TX81Z_ACED_OP1_FIXED            15
#define TX81Z_ACED_OP1_RANGE            16
#define TX81Z_ACED_OP1_FINE             17
#define TX81Z_ACED_OP1_WAVE             18
#define TX81Z_ACED_OP1_SHIFT            19
#define TX81Z_ACED_REVERB_RATE          20
#define TX81Z_ACED_FC_PITCH             21
#define TX81Z_ACED_FC_AMP               22
#define TX81Z_PCED_INST1_MAX_NOTES       0
#define TX81Z_PCED_INST1_VOICE_MSB       1
#define TX81Z_PCED_INST1_VOICE_LSB       2
#define TX81Z_PCED_INST1_CHANNEL         3
#define TX81Z_PCED_INST1_LIMIT_L         4
#define TX81Z_PCED_INST1_LIMIT_H         5
#define TX81Z_PCED_INST1_DETUNE          6
#define TX81Z_PCED_INST1_NOTE_SHIFT      7
#define TX81Z_PCED_INST1_VOLUME          8
#define TX81Z_PCED_INST1_OUT_ASSIGN      9
#define TX81Z_PCED_INST1_LFO_SELECT     10
#define TX81Z_PCED_INST1_MICROTUNE      11
#define TX81Z_PCED_INST2_MAX_NOTES      12
#define TX81Z_PCED_INST2_VOICE_MSB      13
#define TX81Z_PCED_INST2_VOICE_LSB      14
#define TX81Z_PCED_INST2_CHANNEL        15
#define TX81Z_PCED_INST2_LIMIT_L        16
#define TX81Z_PCED_INST2_LIMIT_H        17
#define TX81Z_PCED_INST2_DETUNE         18
#define TX81Z_PCED_INST2_NOTE_SHIFT     19
#define TX81Z_PCED_INST2_VOLUME         20
#define TX81Z_PCED_INST2_OUT_ASSIGN     21
#define TX81Z_PCED_INST2_LFO_SELECT     22
#define TX81Z_PCED_INST2_MICROTUNE      23
#define TX81Z_PCED_INST3_MAX_NOTES      24
#define TX81Z_PCED_INST3_VOICE_MSB      25
#define TX81Z_PCED_INST3_VOICE_LSB      26
#define TX81Z_PCED_INST3_CHANNEL        27
#define TX81Z_PCED_INST3_LIMIT_L        28
#define TX81Z_PCED_INST3_LIMIT_H        29
#define TX81Z_PCED_INST3_DETUNE         30
#define TX81Z_PCED_INST3_NOTE_SHIFT     31
#define TX81Z_PCED_INST3_VOLUME         32
#define TX81Z_PCED_INST3_OUT_ASSIGN     33
#define TX81Z_PCED_INST3_LFO_SELECT     34
#define TX81Z_PCED_INST3_MICROTUNE      35
#define TX81Z_PCED_INST4_MAX_NOTES      36
#define TX81Z_PCED_INST4_VOICE_MSB      37
#define TX81Z_PCED_INST4_VOICE_LSB      38
#define TX81Z_PCED_INST4_CHANNEL        39
#define TX81Z_PCED_INST4_LIMIT_L        40
#define TX81Z_PCED_INST4_LIMIT_H        41
#define TX81Z_PCED_INST4_DETUNE         42
#define TX81Z_PCED_INST4_NOTE_SHIFT     43
#define TX81Z_PCED_INST4_VOLUME         44
#define TX81Z_PCED_INST4_OUT_ASSIGN     45
#define TX81Z_PCED_INST4_LFO_SELECT     46
#define TX81Z_PCED_INST4_MICROTUNE      47
#define TX81Z_PCED_INST5_MAX_NOTES      48
#define TX81Z_PCED_INST5_VOICE_MSB      49
#define TX81Z_PCED_INST5_VOICE_LSB      50
#define TX81Z_PCED_INST5_CHANNEL        51
#define TX81Z_PCED_INST5_LIMIT_L        52
#define TX81Z_PCED_INST5_LIMIT_H        53
#define TX81Z_PCED_INST5_DETUNE         54
#define TX81Z_PCED_INST5_NOTE_SHIFT     55
#define TX81Z_PCED_INST5_VOLUME         56
#define TX81Z_PCED_INST5_OUT_ASSIGN     57
#define TX81Z_PCED_INST5_LFO_SELECT     58
#define TX81Z_PCED_INST5_MICROTUNE      59
#define TX81Z_PCED_INST6_MAX_NOTES      60
#define TX81Z_PCED_INST6_VOICE_MSB      61
#define TX81Z_PCED_INST6_VOICE_LSB      62
#define TX81Z_PCED_INST6_CHANNEL        63
#define TX81Z_PCED_INST6_LIMIT_L        64
#define TX81Z_PCED_INST6_LIMIT_H        65
#define TX81Z_PCED_INST6_DETUNE         66
#define TX81Z_PCED_INST6_NOTE_SHIFT     67
#define TX81Z_PCED_INST6_VOLUME         68
#define TX81Z_PCED_INST6_OUT_ASSIGN     69
#define TX81Z_PCED_INST6_LFO_SELECT     70
#define TX81Z_PCED_INST6_MICROTUNE      71
#define TX81Z_PCED_INST7_MAX_NOTES      72
#define TX81Z_PCED_INST7_VOICE_MSB      73
#define TX81Z_PCED_INST7_VOICE_LSB      74
#define TX81Z_PCED_INST7_CHANNEL        75
#define TX81Z_PCED_INST7_LIMIT_L        76
#define TX81Z_PCED_INST7_LIMIT_H        77
#define TX81Z_PCED_INST7_DETUNE         78
#define TX81Z_PCED_INST7_NOTE_SHIFT     79
#define TX81Z_PCED_INST7_VOLUME         80
#define TX81Z_PCED_INST7_OUT_ASSIGN     81
#define TX81Z_PCED_INST7_LFO_SELECT     82
#define TX81Z_PCED_INST7_MICROTUNE      83
#define TX81Z_PCED_INST8_MAX_NOTES      84
#define TX81Z_PCED_INST8_VOICE_MSB      85
#define TX81Z_PCED_INST8_VOICE_LSB      86
#define TX81Z_PCED_INST8_CHANNEL        87
#define TX81Z_PCED_INST8_LIMIT_L        88
#define TX81Z_PCED_INST8_LIMIT_H        89
#define TX81Z_PCED_INST8_DETUNE         90
#define TX81Z_PCED_INST8_NOTE_SHIFT     91
#define TX81Z_PCED_INST8_VOLUME         92
#define TX81Z_PCED_INST8_OUT_ASSIGN     93
#define TX81Z_PCED_INST8_LFO_SELECT     94
#define TX81Z_PCED_INST8_MICROTUNE      95
#define TX81Z_PCED_MICROTUNE_TABLE      96
#define TX81Z_PCED_ASSIGN_MODE          97
#define TX81Z_PCED_EFFECT_SELECT        98
#define TX81Z_PCED_MICROTUNE_KEY        99
#define TX81Z_PCED_NAME                100
#define TX81Z_PCED_INST_DIFF  (TX81Z_PCED_INST2_MAX_NOTES - TX81Z_PCED_INST1_MAX_NOTES)
#define TX81Z_SYS_MASTER_TUNE            0
#define TX81Z_SYS_RCV_CH                 1
#define TX81Z_SYS_TRANS_CH               2
#define TX81Z_SYS_PROGCH_SW              3
#define TX81Z_SYS_CC_SW                  4
#define TX81Z_SYS_PB_SW                  5
#define TX81Z_SYS_NOTE_SW                6
#define TX81Z_SYS_SYSEX_SW               7
#define TX81Z_SYS_MEMPROT_SW             8
#define TX81Z_SYS_COMBINE_SW             9
#define TX81Z_SYS_AT_SW                 10
#define TX81Z_SYS_STARTUP_MSG           11
#define TX81Z_FX_DELAY_TIME              0
#define TX81Z_FX_PITCH_SHIFT             1
#define TX81Z_FX_DELAY_FEEDBACK          2
#define TX81Z_FX_EFFECT_LEVEL            3
#define TX81Z_FX_PAN_SELECT              4
#define TX81Z_FX_PAN_DIR                 5
#define TX81Z_FX_PAN_RANGE               6
#define TX81Z_FX_CHORD_C3                7
#define TX81Z_FX_CHORD_CSH3             11
#define TX81Z_FX_CHORD_D3               15
#define TX81Z_FX_CHORD_DSH3             19
#define TX81Z_FX_CHORD_E3               23
#define TX81Z_FX_CHORD_F3               27
#define TX81Z_FX_CHORD_FSH3             31
#define TX81Z_FX_CHORD_G3               35
#define TX81Z_FX_CHORD_GSH3             39
#define TX81Z_FX_CHORD_A3               43
#define TX81Z_FX_CHORD_ASH3             47
#define TX81Z_FX_CHORD_B3               51
#define TX81Z_REMOTE_POWER              64
#define TX81Z_REMOTE_STORE              65
#define TX81Z_REMOTE_UTILITY            66
#define TX81Z_REMOTE_COMPARE            67
#define TX81Z_REMOTE_PLAY               68
#define TX81Z_REMOTE_PARAM_LEFT         69
#define TX81Z_REMOTE_PARAM_RIGHT        70
#define TX81Z_REMOTE_DATA_DEC           71
#define TX81Z_REMOTE_DATA_INC           72
#define TX81Z_REMOTE_CURSOR_LEFT        73
#define TX81Z_REMOTE_CURSOR_RIGHT       74
#define TX81Z_REMOTE_CURSOR             75

extern const _TUCHAR *TX81Z_className;

/*
 * Init voice constants
 */
#define TX81Z_initVMemLen 128
extern const BYTE TX81Z_initVMem[TX81Z_initVMemLen];
#define TX81Z_initPMemLen 76
extern const BYTE TX81Z_initPMemSingle[TX81Z_initPMemLen];
extern const BYTE TX81Z_initPMemDual[TX81Z_initPMemLen];
extern const BYTE TX81Z_initPMemSplit[TX81Z_initPMemLen];
extern const BYTE TX81Z_initPMemMono8[TX81Z_initPMemLen];
extern const BYTE TX81Z_initPMemPoly4[TX81Z_initPMemLen];
extern const BYTE TX81Z_initMtf[55][256];

/*
 * Global types
 */
#pragma pack(push, 1)
typedef struct {
    BYTE header[S_ACED_HDR_SIZE];
    BYTE data[S_ACED_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} ACED;

typedef struct {
    BYTE header[S_VCED_HDR_SIZE];
    BYTE data[S_VCED_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} VCED;

typedef struct {
    BYTE header[S_PCED_HDR_SIZE];
    BYTE data[S_PCED_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} PCED;

typedef struct {
    ACED aced;
    VCED vced;   
} AVCED;

typedef struct {
    BYTE header[S_VMEM_HDR_SIZE];
    BYTE data[32][S_VMEM_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} VMEM;

typedef struct {
    BYTE header[S_PMEM_HDR_SIZE];
    BYTE data[32][S_PMEM_DATA_SIZE]; /* 24 user performances + 8 initial */
    BYTE footer[S_FTR_SIZE];
} PMEM;

typedef struct {
    BYTE header[S_FX_HDR_SIZE];
    BYTE data[S_FX_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} FX;

typedef struct {
    BYTE header[S_PC_HDR_SIZE];
    BYTE data[S_PC_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} PC;

typedef struct {
    BYTE header[S_MTO_HDR_SIZE];
    BYTE data[S_MTO_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} MTO;

typedef struct {
    BYTE header[S_MTF_HDR_SIZE];
    BYTE data[S_MTF_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} MTF;

typedef struct {
    BYTE header[S_SYS_HDR_SIZE];
    BYTE data[S_SYS_DATA_SIZE];
    BYTE footer[S_FTR_SIZE];
} SYS;

typedef struct {
    _TUCHAR fileName[_MAX_PATH];
    REQUEST loaded;
    DWORD itemDirty[7];
} EXTRA;

typedef struct {
    AVCED avced;
    PCED pced;
    FX fx;
    PC pc;
    MTO mto;
    MTF mtf;
    SYS sys;
    VMEM vmem;
    PMEM pmem;
    VMEM preset[4];
    EXTRA extra;
} SNAPSHOT;
#pragma pack(pop)

typedef struct {
    REQUEST reqFlag;
    SINDEX sIndex; /* snapshot flag/list box item index */
    const _TUCHAR *dumpName;
    const _TUCHAR *nameFmt;
    const _TUCHAR *proseName;
    const _TUCHAR *typeStr;
    BYTE *reqStr;
    size_t reqStrLen;
    BYTE *dumpHdr;
    size_t dumpHdrLen;
    BYTE *buf;
    size_t bufSize;
    size_t recorded;
    size_t offset;
    BYTE *dataPtr;
    size_t libSize;
} TX81ZMETA;

typedef enum {
    RA_DOWN,
    RA_UP,
    RA_CLICK,
} REMOTEACTION;

/*
 * Global procedures
 */
HWND TX81Z_Create(HWND parentWnd);
void TX81Z_Destroy(HWND tx81zWnd);
METAINDEX TX81Z_DetectSysexType(MIDI *midi, BYTE *data);
void TX81Z_InitPfm(MIDI *midi, SNAPSHOT *snapshot, const BYTE *pfmInit);
void TX81Z_InitVoice(MIDI *midi, SNAPSHOT *snapshot);
BOOL TX81Z_IsMemoryProtectOn(MIDI *midi, SNAPSHOT *snapshot);
void TX81Z_Panic(MIDI *midi, SNAPSHOT *snapshot);
void TX81Z_ProgramChange(MIDI *midi, int bank, int number);
BOOL TX81Z_Register(void);
BOOL TX81Z_RetrieveData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot);
void TX81Z_SendData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot);
void TX81Z_SendParamChange(MIDI *midi, int subgroup, int paramIndex, int value);
void TX81Z_SendPCParamChange(MIDI *midi, int pcNumber, int memNum);
void TX81Z_SendPresetBankRequest(MIDI *midi);
void TX81Z_SendRemote(MIDI *midi, BYTE function, REMOTEACTION action);
BOOL TX81Z_StorePfm(MIDI *midi, int destIndex);
BOOL TX81Z_StoreVoice(MIDI *midi, int destIndex);
void TX81Z_SwitchToPfmMode(MIDI *midi);
void TX81Z_SwitchToVoiceMode(MIDI *midi);

/*
 * Global variables
 */
extern TX81ZMETA TX81Z_meta[META_CNT];

#endif
