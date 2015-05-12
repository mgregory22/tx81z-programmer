/*
 * txpack.c - TX81Z parameter locations and ranges
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
#include "snapshot.h"
#include "tx81z.h"
#include "txpack.h"

/*
 * Global procedures
 */
extern void TXPack_AvcedToVmem(const BYTE *acedData, BYTE *vcedData, BYTE *vmemData);
extern void TXPack_PcedToPmem(const BYTE *pcedData, BYTE *pmemData);
extern void TXPack_PmemToPced(const BYTE *pmemData, PCED *pced);
extern void TXPack_ValidateBuf(BYTE *buf, METAINDEX type);
extern void TXPack_VmemToAvced(const BYTE *vmemData, AVCED *avced, BOOL combine);

/*
 * Global constants
 */
const TXPACK TXPack_vced[TXPack_vcedCnt] = {
/* Op 4 */
    { /*  0 */  0, 0, 5, 0, 31, _T("OP4 Attack Rate")              },
    { /*  1 */  1, 0, 5, 0, 31, _T("OP4 Decay 1 Rate")             },
    { /*  2 */  2, 0, 5, 0, 31, _T("OP4 Decay 2 Rate")             },
    { /*  3 */  3, 0, 4, 0, 15, _T("OP4 Release Rate")             },
    { /*  4 */  4, 0, 4, 0, 15, _T("OP4 Decay 1 Level")            },
    { /*  5 */  5, 0, 7, 0, 99, _T("OP4 Level Scaling")            },
    { /*  6 */  9, 3, 2, 0,  3, _T("OP4 Rate Scaling")             },
    { /*  7 */  6, 3, 3, 0,  7, _T("OP4 EG Bias Sensitivity")      },
    { /*  8 */  6, 6, 1, 0,  1, _T("OP4 Amplitude Modulation Enable") },
    { /*  9 */  6, 0, 3, 0,  7, _T("OP4 Key Velocity Sensitivity") },
    { /* 10 */  7, 0, 7, 0, 99, _T("OP4 Out Level")                },
    { /* 11 */  8, 0, 6, 0, 63, _T("OP4 Coarse Frequency")         },
    { /* 12 */  9, 0, 3, 0,  6, _T("OP4 Detune")                   },
/* Op 2 */
    { /* 13 */ 10, 0, 5, 0, 31, _T("OP2 Attack Rate")              },
    { /* 14 */ 11, 0, 5, 0, 31, _T("OP2 Decay 1 Rate")             },
    { /* 15 */ 12, 0, 5, 0, 31, _T("OP2 Decay 2 Rate")             },
    { /* 16 */ 13, 0, 4, 0, 15, _T("OP2 Release Rate")             },
    { /* 17 */ 14, 0, 4, 0, 15, _T("OP2 Decay 1 Level")            },
    { /* 18 */ 15, 0, 7, 0, 99, _T("OP2 Level Scaling")            },
    { /* 19 */ 19, 3, 2, 0,  3, _T("OP2 Rate Scaling")             },
    { /* 20 */ 16, 3, 3, 0,  7, _T("OP2 EG Bias Sensitivity")      },
    { /* 21 */ 16, 6, 1, 0,  1, _T("OP2 Amplitude Modulation Enable") },
    { /* 22 */ 16, 0, 3, 0,  7, _T("OP2 Key Velocity Sensitivity") },
    { /* 23 */ 17, 0, 7, 0, 99, _T("OP2 Out Level")                },
    { /* 24 */ 18, 0, 6, 0, 63, _T("OP2 Coarse Frequency")         },
    { /* 25 */ 19, 0, 3, 0,  6, _T("OP2 Detune")                   },
/* Op 3 */
    { /* 26 */ 20, 0, 5, 0, 31, _T("OP3 Attack Rate")              },
    { /* 27 */ 21, 0, 5, 0, 31, _T("OP3 Decay 1 Rate")             },
    { /* 28 */ 22, 0, 5, 0, 31, _T("OP3 Decay 2 Rate")             },
    { /* 29 */ 23, 0, 4, 0, 15, _T("OP3 Release Rate")             },
    { /* 30 */ 24, 0, 4, 0, 15, _T("OP3 Decay 1 Level")            },
    { /* 31 */ 25, 0, 7, 0, 99, _T("OP3 Level Scaling")            },
    { /* 32 */ 29, 3, 2, 0,  3, _T("OP3 Rate Scaling")             },
    { /* 33 */ 26, 3, 3, 0,  7, _T("OP3 EG Bias Sensitivity")      },
    { /* 34 */ 26, 6, 1, 0,  1, _T("OP3 Amplitude Modulation Enable") },
    { /* 35 */ 26, 0, 3, 0,  7, _T("OP3 Key Velocity Sensitivity") },
    { /* 36 */ 27, 0, 7, 0, 99, _T("OP3 Out Level")                },
    { /* 37 */ 28, 0, 6, 0, 63, _T("OP3 Coarse Frequency")         },
    { /* 38 */ 29, 0, 3, 0,  6, _T("OP3 Detune")                   },
/* Op 1 */
    { /* 39 */ 30, 0, 5, 0, 31, _T("OP1 Attack Rate")              },
    { /* 40 */ 31, 0, 5, 0, 31, _T("OP1 Decay 1 Rate")             },
    { /* 41 */ 32, 0, 5, 0, 31, _T("OP1 Decay 2 Rate")             },
    { /* 42 */ 33, 0, 4, 0, 15, _T("OP1 Release Rate")             },
    { /* 43 */ 34, 0, 4, 0, 15, _T("OP1 Decay 1 Level")            },
    { /* 44 */ 35, 0, 7, 0, 99, _T("OP1 Level Scaling")            },
    { /* 45 */ 39, 3, 2, 0,  3, _T("OP1 Rate Scaling")             },
    { /* 46 */ 36, 3, 3, 0,  7, _T("OP1 EG Bias Sensitivity")      },
    { /* 47 */ 36, 6, 1, 0,  1, _T("OP1 Amplitude Modulation Enable") },
    { /* 48 */ 36, 0, 3, 0,  7, _T("OP1 Key Velocity Sensitivity") },
    { /* 49 */ 37, 0, 7, 0, 99, _T("OP1 Out Level")                },
    { /* 50 */ 38, 0, 6, 0, 63, _T("OP1 Coarse Frequency")         },
    { /* 51 */ 39, 0, 3, 0,  6, _T("OP1 Detune")                   },
/* */
    { /* 52 */ 40, 0, 3,  0,   7, _T("Algorithm") },
    { /* 53 */ 40, 3, 3,  0,   7, _T("Feedback") },
    { /* 54 */ 41, 0, 7,  0,  99, _T("LFO Speed") },
    { /* 55 */ 42, 0, 7,  0,  99, _T("LFO Delay") },
    { /* 56 */ 43, 0, 7,  0,  99, _T("Pitch Modulation Depth") },
    { /* 57 */ 44, 0, 7,  0,  99, _T("Amplitude Modulation Depth") },
    { /* 58 */ 40, 7, 1,  0,   1, _T("LFO Key Sync") },
    { /* 59 */ 45, 0, 2,  0,   3, _T("LFO Wave") },
    { /* 60 */ 45, 4, 3,  0,   7, _T("Pitch Modulation Sensitivity") },
    { /* 61 */ 45, 2, 2,  0,   3, _T("Amplitude Modulation Sensitivity") },
    { /* 62 */ 46, 0, 6,  0,  48, _T("Transpose") },
    { /* 63 */ 48, 3, 1,  0,   1, _T("Poly/Mono Mode") },
    { /* 64 */ 47, 0, 4,  0,  12, _T("Pitch Bend Range") },
    { /* 65 */ 48, 0, 1,  0,   1, _T("Portamento Mode") },
    { /* 66 */ 49, 0, 7,  0,  99, _T("Portamento Time") },
    { /* 67 */ 50, 0, 7,  0,  99, _T("Foot Controller Volume") },
    { /* 68 */ 48, 2, 1,  0,   1, _T("Sustain") },
    { /* 69 */ 48, 1, 1,  0,   1, _T("Portamento") },
    { /* 70 */ 48, 4, 1,  0,   1, _T("Chorus") },
    { /* 71 */ 51, 0, 7,  0,  99, _T("Modulation Wheel Pitch") },
    { /* 72 */ 52, 0, 7,  0,  99, _T("Modulation Wheel Amplitude") },
    { /* 73 */ 53, 0, 7,  0,  99, _T("Breath Controller Pitch") },
    { /* 74 */ 54, 0, 7,  0,  99, _T("Breath Controller Amplitude") },
    { /* 75 */ 55, 0, 7,  0,  99, _T("Breath Controller Pitch Bias") },
    { /* 76 */ 56, 0, 7,  0,  99, _T("Breath Controller EG Bias") },
    { /* 77 */ 57, 0, 7, 32, 127, _T("Voice Name  1") },
    { /* 78 */ 58, 0, 7, 32, 127, _T("Voice Name  2") },
    { /* 79 */ 59, 0, 7, 32, 127, _T("Voice Name  3") },
    { /* 80 */ 60, 0, 7, 32, 127, _T("Voice Name  4") },
    { /* 81 */ 61, 0, 7, 32, 127, _T("Voice Name  5") },
    { /* 82 */ 62, 0, 7, 32, 127, _T("Voice Name  6") },
    { /* 83 */ 63, 0, 7, 32, 127, _T("Voice Name  7") },
    { /* 84 */ 64, 0, 7, 32, 127, _T("Voice Name  8") },
    { /* 85 */ 65, 0, 7, 32, 127, _T("Voice Name  9") },
    { /* 86 */ 66, 0, 7, 32, 127, _T("Voice Name 10") },
    { /* 87 */ 67, 0, 7,  0,  99, _T("PEG Rate 1 (DX21/V50 only)") },
    { /* 88 */ 68, 0, 7,  0,  99, _T("PEG Rate 2 (DX21/V50 only)") },
    { /* 89 */ 69, 0, 7,  0,  99, _T("PEG Rate 3 (DX21/V50 only)") },
    { /* 90 */ 70, 0, 7,  0,  99, _T("PEG Level 1 (DX21/V50 only)") },
    { /* 91 */ 71, 0, 7,  0,  99, _T("PEG Level 2 (DX21/V50 only)") },
    { /* 92 */ 72, 0, 7,  0,  99, _T("PEG Level 3 (DX21/V50 only)") },
    //{ 93,  0, 0, 0,  0,  15, _T("Operator 4-1 On/Off (bits 0-3) (doesn't exist in VMEM)") },
};

const TXPACK TXPack_aced[TXPack_acedCnt] = {
/* Op 4 */
    { /*  0 */ 73, 3, 1, 0,  1, _T("OP4 Fixed Frequency On/Off") },
    { /*  1 */ 73, 0, 3, 0,  7, _T("OP4 Fixed Frequency Range") },
    { /*  2 */ 74, 0, 4, 0, 15, _T("OP4 Frequency Range Fine") },
    { /*  3 */ 74, 4, 3, 0,  7, _T("OP4 Waveform") },
    { /*  4 */ 73, 4, 2, 0,  3, _T("OP4 EG Shift") },
/* Op 2 */
    { /*  5 */ 75, 3, 1, 0,  1, _T("OP2 Fixed Frequency On/Off") },
    { /*  6 */ 75, 0, 3, 0,  7, _T("OP2 Fixed Frequency Range") },
    { /*  7 */ 76, 0, 4, 0, 15, _T("OP2 Frequency Range Fine") },
    { /*  8 */ 76, 4, 3, 0,  7, _T("OP2 Waveform") },
    { /*  9 */ 75, 4, 2, 0,  3, _T("OP2 EG Shift") },
/* Op 3 */
    { /* 10 */ 77, 3, 1, 0,  1, _T("OP3 Fixed Frequency On/Off") },
    { /* 11 */ 77, 0, 3, 0,  7, _T("OP3 Fixed Frequency Range") },
    { /* 12 */ 78, 0, 4, 0, 15, _T("OP3 Frequency Range Fine") },
    { /* 13 */ 78, 4, 3, 0,  7, _T("OP3 Waveform") },
    { /* 14 */ 77, 4, 2, 0,  3, _T("OP3 EG Shift") },
/* Op 1 */
    { /* 15 */ 79, 3, 1, 0,  1, _T("OP1 Fixed Frequency On/Off") },
    { /* 16 */ 79, 0, 3, 0,  7, _T("OP1 Fixed Frequency Range") },
    { /* 17 */ 80, 0, 4, 0, 15, _T("OP1 Frequency Range Fine") },
    { /* 18 */ 80, 4, 3, 0,  7, _T("OP1 Waveform") },
    { /* 19 */ 79, 4, 2, 0,  3, _T("OP1 EG Shift") },
/* */
    { /* 20 */ 81, 0, 3, 0,  7, _T("Reverb Rate")  },
    { /* 21 */ 82, 0, 7, 0, 99, _T("Foot Controller Pitch")     },
    { /* 22 */ 83, 0, 7, 0, 99, _T("Foot Controller Amplitude") },
};

const TXPACK TXPack_pced[TXPack_pcedCnt] = {
/* Inst 1 */
    { /*  0 */  0, 0, 4, 0,   8, _T("Inst 1 Maximum Notes") },
    { /*  1 */  0, 4, 1, 0,   1, _T("Inst 1 Voice Number MSB") },
    { /*  2 */  1, 0, 7, 0, 127, _T("Inst 1 Voice Number") },
    { /*  3 */  2, 0, 5, 0,  16, _T("Inst 1 Receive Channel") },
    { /*  4 */  3, 0, 7, 0, 127, _T("Inst 1 Low Note Limit") },
    { /*  5 */  4, 0, 7, 0, 127, _T("Inst 1 High Note Limit") },
    { /*  6 */  5, 0, 4, 0,  14, _T("Inst 1 Detune") },
    { /*  7 */  6, 0, 6, 0,  48, _T("Inst 1 Note Shift") },
    { /*  8 */  7, 0, 7, 0,  99, _T("Inst 1 Volume") },
    { /*  9 */  0, 5, 2, 0,   3, _T("Inst 1 Output Assign") },
    { /* 10 */  2, 5, 2, 0,   3, _T("Inst 1 LFO Select") },
    { /* 11 */  6, 6, 1, 0,   1, _T("Inst 1 Microtune Enable") },
/* Inst 2 */
    { /* 12 */  8, 0, 4, 0,   8, _T("Inst 2 Maximum Notes") },
    { /* 13 */  8, 4, 1, 0,   1, _T("Inst 2 Voice Number MSB") },
    { /* 14 */  9, 0, 7, 0, 127, _T("Inst 2 Voice Number") },
    { /* 15 */ 10, 0, 5, 0,  16, _T("Inst 2 Receive Channel") },
    { /* 16 */ 11, 0, 7, 0, 127, _T("Inst 2 Low Note Limit") },
    { /* 17 */ 12, 0, 7, 0, 127, _T("Inst 2 High Note Limit") },
    { /* 18 */ 13, 0, 4, 0,  14, _T("Inst 2 Detune") },
    { /* 19 */ 14, 0, 6, 0,  48, _T("Inst 2 Note Shift") },
    { /* 20 */ 15, 0, 7, 0,  99, _T("Inst 2 Volume") },
    { /* 21 */  8, 5, 2, 0,   3, _T("Inst 2 Output Assign") },
    { /* 22 */ 10, 5, 2, 0,   3, _T("Inst 2 LFO Select") },
    { /* 23 */ 14, 6, 1, 0,   1, _T("Inst 2 Microtune Enable") },
/* Inst 3 */
    { /* 24 */ 16, 0, 4, 0,   8, _T("Inst 3 Maximum Notes") },
    { /* 25 */ 16, 4, 1, 0,   1, _T("Inst 3 Voice Number MSB") },
    { /* 26 */ 17, 0, 7, 0, 127, _T("Inst 3 Voice Number") },
    { /* 27 */ 18, 0, 5, 0,  16, _T("Inst 3 Receive Channel") },
    { /* 28 */ 19, 0, 7, 0, 127, _T("Inst 3 Low Note Limit") },
    { /* 29 */ 20, 0, 7, 0, 127, _T("Inst 3 High Note Limit") },
    { /* 30 */ 21, 0, 4, 0,  14, _T("Inst 3 Detune") },
    { /* 31 */ 22, 0, 6, 0,  48, _T("Inst 3 Note Shift") },
    { /* 32 */ 23, 0, 7, 0,  99, _T("Inst 3 Volume") },
    { /* 33 */ 16, 5, 2, 0,   3, _T("Inst 3 Output Assign") },
    { /* 34 */ 18, 5, 2, 0,   3, _T("Inst 3 LFO Select") },
    { /* 35 */ 22, 6, 1, 0,   1, _T("Inst 3 Microtune Enable") },
/* Inst 4 */
    { /* 36 */ 24, 0, 4, 0,   8, _T("Inst 4 Maximum Notes") },
    { /* 37 */ 24, 4, 1, 0,   1, _T("Inst 4 Voice Number MSB") },
    { /* 38 */ 25, 0, 7, 0, 127, _T("Inst 4 Voice Number") },
    { /* 39 */ 26, 0, 5, 0,  16, _T("Inst 4 Receive Channel") },
    { /* 40 */ 27, 0, 7, 0, 127, _T("Inst 4 Low Note Limit") },
    { /* 41 */ 28, 0, 7, 0, 127, _T("Inst 4 High Note Limit") },
    { /* 42 */ 29, 0, 4, 0,  14, _T("Inst 4 Detune") },
    { /* 43 */ 30, 0, 6, 0,  48, _T("Inst 4 Note Shift") },
    { /* 44 */ 31, 0, 7, 0,  99, _T("Inst 4 Volume") },
    { /* 45 */ 24, 5, 2, 0,   3, _T("Inst 4 Output Assign") },
    { /* 46 */ 26, 5, 2, 0,   3, _T("Inst 4 LFO Select") },
    { /* 47 */ 30, 6, 1, 0,   1, _T("Inst 4 Microtune Enable") },
/* Inst 5 */
    { /* 48 */ 32, 0, 4, 0,   8, _T("Inst 5 Maximum Notes") },
    { /* 49 */ 32, 4, 1, 0,   1, _T("Inst 5 Voice Number MSB") },
    { /* 50 */ 33, 0, 7, 0, 127, _T("Inst 5 Voice Number") },
    { /* 51 */ 34, 0, 5, 0,  16, _T("Inst 5 Receive Channel") },
    { /* 52 */ 35, 0, 7, 0, 127, _T("Inst 5 Low Note Limit") },
    { /* 53 */ 36, 0, 7, 0, 127, _T("Inst 5 High Note Limit") },
    { /* 54 */ 37, 0, 4, 0,  14, _T("Inst 5 Detune") },
    { /* 55 */ 38, 0, 6, 0,  48, _T("Inst 5 Note Shift") },
    { /* 56 */ 39, 0, 7, 0,  99, _T("Inst 5 Volume") },
    { /* 57 */ 32, 5, 2, 0,   3, _T("Inst 5 Output Assign") },
    { /* 58 */ 34, 5, 2, 0,   3, _T("Inst 5 LFO Select") },
    { /* 59 */ 38, 6, 1, 0,   1, _T("Inst 5 Microtune Enable") },
/* Inst 6 */
    { /* 60 */ 40, 0, 4, 0,   8, _T("Inst 6 Maximum Notes") },
    { /* 61 */ 40, 4, 1, 0,   1, _T("Inst 6 Voice Number MSB") },
    { /* 62 */ 41, 0, 7, 0, 127, _T("Inst 6 Voice Number") },
    { /* 63 */ 42, 0, 5, 0,  16, _T("Inst 6 Receive Channel") },
    { /* 64 */ 43, 0, 7, 0, 127, _T("Inst 6 Low Note Limit") },
    { /* 65 */ 44, 0, 7, 0, 127, _T("Inst 6 High Note Limit") },
    { /* 66 */ 45, 0, 4, 0,  14, _T("Inst 6 Detune") },
    { /* 67 */ 46, 0, 6, 0,  48, _T("Inst 6 Note Shift") },
    { /* 68 */ 47, 0, 7, 0,  99, _T("Inst 6 Volume") },
    { /* 69 */ 40, 5, 2, 0,   3, _T("Inst 6 Output Assign") },
    { /* 70 */ 42, 5, 2, 0,   3, _T("Inst 6 LFO Select") },
    { /* 71 */ 46, 6, 1, 0,   1, _T("Inst 6 Microtune Enable") },
/* Inst 7 */
    { /* 72 */ 48, 0, 4, 0,   8, _T("Inst 7 Maximum Notes") },
    { /* 73 */ 48, 4, 1, 0,   1, _T("Inst 7 Voice Number MSB") },
    { /* 74 */ 49, 0, 7, 0, 127, _T("Inst 7 Voice Number") },
    { /* 75 */ 50, 0, 5, 0,  16, _T("Inst 7 Receive Channel") },
    { /* 76 */ 51, 0, 7, 0, 127, _T("Inst 7 Low Note Limit") },
    { /* 77 */ 52, 0, 7, 0, 127, _T("Inst 7 High Note Limit") },
    { /* 78 */ 53, 0, 4, 0,  14, _T("Inst 7 Detune") },
    { /* 79 */ 54, 0, 6, 0,  48, _T("Inst 7 Note Shift") },
    { /* 80 */ 55, 0, 7, 0,  99, _T("Inst 7 Volume") },
    { /* 81 */ 48, 5, 2, 0,   3, _T("Inst 7 Output Assign") },
    { /* 82 */ 50, 5, 2, 0,   3, _T("Inst 7 LFO Select") },
    { /* 83 */ 54, 6, 1, 0,   1, _T("Inst 7 Microtune Enable") },
/* Inst 8 */
    { /* 84 */ 56, 0, 4, 0,   8, _T("Inst 8 Maximum Notes") },
    { /* 85 */ 56, 4, 1, 0,   1, _T("Inst 8 Voice Number MSB") },
    { /* 86 */ 57, 0, 7, 0, 127, _T("Inst 8 Voice Number") },
    { /* 87 */ 58, 0, 5, 0,  16, _T("Inst 8 Receive Channel") },
    { /* 88 */ 59, 0, 7, 0, 127, _T("Inst 8 Low Note Limit") },
    { /* 89 */ 60, 0, 7, 0, 127, _T("Inst 8 High Note Limit") },
    { /* 90 */ 61, 0, 4, 0,  14, _T("Inst 8 Detune") },
    { /* 91 */ 62, 0, 6, 0,  48, _T("Inst 8 Note Shift") },
    { /* 92 */ 63, 0, 7, 0,  99, _T("Inst 8 Volume") },
    { /* 93 */ 56, 5, 2, 0,   3, _T("Inst 8 Output Assign") },
    { /* 94 */ 58, 5, 2, 0,   3, _T("Inst 8 LFO Select") },
    { /* 95 */ 62, 6, 1, 0,   1, _T("Inst 8 Microtune Enable") },
/*  */
    { /*  96 */ 64, 0, 4,  0,  12, _T("Microtune Table") },
    { /*  97 */ 65, 0, 1,  0,   1, _T("Assign Mode") },
    { /*  98 */ 65, 1, 2,  0,   3, _T("Effect Select") },
    { /*  99 */ 65, 3, 4,  0,  11, _T("Microtune Key") },
    { /* 100 */ 66, 0, 7, 32, 127, _T("Performance Name  1") },
    { /* 101 */ 67, 0, 7, 32, 127, _T("Performance Name  2") },
    { /* 102 */ 68, 0, 7, 32, 127, _T("Performance Name  3") },
    { /* 103 */ 69, 0, 7, 32, 127, _T("Performance Name  4") },
    { /* 104 */ 70, 0, 7, 32, 127, _T("Performance Name  5") },
    { /* 105 */ 71, 0, 7, 32, 127, _T("Performance Name  6") },
    { /* 106 */ 72, 0, 7, 32, 127, _T("Performance Name  7") },
    { /* 107 */ 73, 0, 7, 32, 127, _T("Performance Name  8") },
    { /* 108 */ 74, 0, 7, 32, 127, _T("Performance Name  9") },
    { /* 109 */ 75, 0, 7, 32, 127, _T("Performance Name 10") },
};
const TXPACK TXPack_fx[TXPack_fxCnt] = {
    { /*  0 */  0, 0, 7, 0, 127, _T("Delay Time") },
    { /*  1 */  1, 0, 6, 0,  48, _T("Delay Pitch Shift") },
    { /*  2 */  2, 0, 3, 0,   7, _T("Delay Feedback") },
    { /*  3 */  3, 0, 7, 0,  99, _T("Delay Level") },
    { /*  4 */  4, 0, 2, 0,   2, _T("Pan Select") },
    { /*  5 */  5, 0, 1, 0,   1, _T("Pan Direction") },
    { /*  6 */  6, 0, 7, 0,  99, _T("Pan Range") },
    { /*  7 */  7, 0, 6, 0,  49, _T("C3 Chord Note 1") },
    { /*  8 */  8, 0, 6, 0,  49, _T("C3 Chord Note 2") },
    { /*  9 */  9, 0, 6, 0,  49, _T("C3 Chord Note 3") },
    { /* 10 */ 10, 0, 6, 0,  49, _T("C3 Chord Note 4") },
    { /* 11 */ 11, 0, 6, 0,  49, _T("C#3 Chord Note 1") },
    { /* 12 */ 12, 0, 6, 0,  49, _T("C#3 Chord Note 2") },
    { /* 13 */ 13, 0, 6, 0,  49, _T("C#3 Chord Note 3") },
    { /* 14 */ 14, 0, 6, 0,  49, _T("C#3 Chord Note 4") },
    { /* 15 */ 15, 0, 6, 0,  49, _T("D3 Chord Note 1") },
    { /* 16 */ 16, 0, 6, 0,  49, _T("D3 Chord Note 2") },
    { /* 17 */ 17, 0, 6, 0,  49, _T("D3 Chord Note 3") },
    { /* 18 */ 18, 0, 6, 0,  49, _T("D3 Chord Note 4") },
    { /* 19 */ 19, 0, 6, 0,  49, _T("D#3 Chord Note 1") },
    { /* 20 */ 20, 0, 6, 0,  49, _T("D#3 Chord Note 2") },
    { /* 21 */ 21, 0, 6, 0,  49, _T("D#3 Chord Note 3") },
    { /* 22 */ 22, 0, 6, 0,  49, _T("D#3 Chord Note 4") },
    { /* 23 */ 23, 0, 6, 0,  49, _T("E3 Chord Note 1") },
    { /* 24 */ 24, 0, 6, 0,  49, _T("E3 Chord Note 2") },
    { /* 25 */ 25, 0, 6, 0,  49, _T("E3 Chord Note 3") },
    { /* 26 */ 26, 0, 6, 0,  49, _T("E3 Chord Note 4") },
    { /* 27 */ 27, 0, 6, 0,  49, _T("F3 Chord Note 1") },
    { /* 28 */ 28, 0, 6, 0,  49, _T("F3 Chord Note 2") },
    { /* 29 */ 29, 0, 6, 0,  49, _T("F3 Chord Note 3") },
    { /* 30 */ 30, 0, 6, 0,  49, _T("F3 Chord Note 4") },
    { /* 31 */ 31, 0, 6, 0,  49, _T("F#3 Chord Note 1") },
    { /* 32 */ 32, 0, 6, 0,  49, _T("F#3 Chord Note 2") },
    { /* 33 */ 33, 0, 6, 0,  49, _T("F#3 Chord Note 3") },
    { /* 34 */ 34, 0, 6, 0,  49, _T("F#3 Chord Note 4") },
    { /* 35 */ 35, 0, 6, 0,  49, _T("G3 Chord Note 1") },
    { /* 36 */ 36, 0, 6, 0,  49, _T("G3 Chord Note 2") },
    { /* 37 */ 37, 0, 6, 0,  49, _T("G3 Chord Note 3") },
    { /* 38 */ 38, 0, 6, 0,  49, _T("G3 Chord Note 4") },
    { /* 39 */ 39, 0, 6, 0,  49, _T("G#3 Chord Note 1") },
    { /* 40 */ 40, 0, 6, 0,  49, _T("G#3 Chord Note 2") },
    { /* 41 */ 41, 0, 6, 0,  49, _T("G#3 Chord Note 3") },
    { /* 42 */ 42, 0, 6, 0,  49, _T("G#3 Chord Note 4") },
    { /* 43 */ 43, 0, 6, 0,  49, _T("A3 Chord Note 1") },
    { /* 44 */ 44, 0, 6, 0,  49, _T("A3 Chord Note 2") },
    { /* 45 */ 45, 0, 6, 0,  49, _T("A3 Chord Note 3") },
    { /* 46 */ 46, 0, 6, 0,  49, _T("A3 Chord Note 4") },
    { /* 47 */ 47, 0, 6, 0,  49, _T("A#3 Chord Note 1") },
    { /* 48 */ 48, 0, 6, 0,  49, _T("A#3 Chord Note 2") },
    { /* 49 */ 49, 0, 6, 0,  49, _T("A#3 Chord Note 3") },
    { /* 50 */ 50, 0, 6, 0,  49, _T("A#3 Chord Note 4") },
    { /* 51 */ 51, 0, 6, 0,  49, _T("B3 Chord Note 1") },
    { /* 52 */ 52, 0, 6, 0,  49, _T("B3 Chord Note 2") },
    { /* 53 */ 53, 0, 6, 0,  49, _T("B3 Chord Note 3") },
    { /* 54 */ 54, 0, 6, 0,  49, _T("B3 Chord Note 4") },
};
const TXPACK TXPack_sys[TXPack_sysCnt] = {
    { /*  0 */  0, 0, 7, 0, 172, _T("Master Tune") },
    { /*  1 */  1, 0, 5, 0,  16, _T("Basic Receive Channel") },
    { /*  2 */  2, 0, 4, 0,  15, _T("Transmit Channel") },
    { /*  3 */  3, 0, 2, 0,   2, _T("Program Change Switch") },
    { /*  4 */  4, 0, 5, 0,  17, _T("Controller Change Switch") },
    { /*  5 */  5, 0, 5, 0,  17, _T("Pitch Bend Switch") },
    { /*  6 */  6, 0, 2, 0,   2, _T("Note Switch") },
    { /*  7 */  7, 0, 1, 0,   1, _T("System Exclusive") },
    { /*  8 */  8, 0, 1, 0,   1, _T("Memory Protect") },
    { /*  9 */  9, 0, 1, 0,   1, _T("Combine") },
    { /* 10 */ 10, 0, 1, 0,   1, _T("AT -> BC") },
    { /* 11 */ 11, 0, 7, 32, 127, _T("Startup Message  1") },
    { /* 12 */ 12, 0, 7, 32, 127, _T("Startup Message  2") },
    { /* 13 */ 13, 0, 7, 32, 127, _T("Startup Message  3") },
    { /* 14 */ 14, 0, 7, 32, 127, _T("Startup Message  4") },
    { /* 15 */ 15, 0, 7, 32, 127, _T("Startup Message  5") },
    { /* 16 */ 16, 0, 7, 32, 127, _T("Startup Message  6") },
    { /* 17 */ 17, 0, 7, 32, 127, _T("Startup Message  7") },
    { /* 18 */ 18, 0, 7, 32, 127, _T("Startup Message  8") },
    { /* 19 */ 19, 0, 7, 32, 127, _T("Startup Message  9") },
    { /* 20 */ 20, 0, 7, 32, 127, _T("Startup Message 10") },
    { /* 21 */ 21, 0, 7, 32, 127, _T("Startup Message 11") },
    { /* 22 */ 22, 0, 7, 32, 127, _T("Startup Message 12") },
    { /* 23 */ 23, 0, 7, 32, 127, _T("Startup Message 13") },
    { /* 24 */ 24, 0, 7, 32, 127, _T("Startup Message 14") },
    { /* 25 */ 25, 0, 7, 32, 127, _T("Startup Message 15") },
    { /* 26 */ 26, 0, 7, 32, 127, _T("Startup Message 16") },
};
const BYTE TXPack_masks[8] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

/*
 * Unit constants
 */
static const int tp_acedFuncParamIdxs[] = {
    TX81Z_ACED_FC_PITCH,
    TX81Z_ACED_FC_AMP
};
#define tp_acedFuncParamIdxCnt (sizeof tp_acedFuncParamIdxs / sizeof tp_acedFuncParamIdxs[0])

static const int tp_vcedFuncParamIdxs[] = {
    TX81Z_VCED_PB_RANGE,
    TX81Z_VCED_PORT_TIME,
    TX81Z_VCED_FC_VOL,
    TX81Z_VCED_MW_PITCH,
    TX81Z_VCED_MW_AMP,
    TX81Z_VCED_BC_PITCH,
    TX81Z_VCED_BC_AMP,
    TX81Z_VCED_BC_PITCH_BIAS,
    TX81Z_VCED_BC_EG_BIAS,
    TX81Z_VCED_POLY_MODE,
    TX81Z_VCED_PORT_MODE
};
#define tp_vcedFuncParamIdxCnt (sizeof tp_vcedFuncParamIdxs / sizeof tp_vcedFuncParamIdxs[0])

/*
 * Procedure definitions
 */

/*
 * AvcedToVmem() - Converts a voice edit buffer to a bank/library stored voice.
 */
void TXPack_AvcedToVmem(const BYTE *acedData, BYTE *vcedData, BYTE *vmemData)
{
    int i, mask, shift, value;

    memset(vmemData, 0, TX81Z_meta[META_VMEM].libSize);
    for (i = 0; i < TXPack_vcedCnt; i++) {
        const TXPACK *pack = &TXPack_vced[i];

        value = vcedData[i];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        vmemData[pack->memOffset] |= (value & mask) << shift;
    }
    for (i = 0; i < TXPack_acedCnt; i++) {
        const TXPACK *pack = &TXPack_aced[i];

        value = acedData[i];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        vmemData[pack->memOffset] |= (value & mask) << shift;
    }
}

/*
 * PcedToPmem() - Converts a performance edit buffer to a bank/library stored
 *                performance.
 */
void TXPack_PcedToPmem(const BYTE *pcedData, BYTE *pmemData)
{
    int i, mask, shift, value;

    memset(pmemData, 0, TX81Z_meta[META_PMEM].libSize);
    for (i = 0; i < TXPack_pcedCnt; i++) {
        const TXPACK *pack = &TXPack_pced[i];

        value = pcedData[i];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        pmemData[pack->memOffset] |= (value & mask) << shift;
    }
}

/*
 * PMemToPced() - Converts a bank/library stored performance to a performance
 *                edit buffer.
 */
void TXPack_PmemToPced(const BYTE *pmemData, PCED *pced)
{
    int i, mask, shift, value;
    BYTE *pcedData = pced->data;

    /*
     * Remove the "unloaded" header mark (should probably copy the whole
     * header from the TX81Z_meta structure, but I don't feel like it)
     */
    pced->header[0] = '\xF0';
    /*
     * Unpack the PMEM data into the performance edit buffer.
     */
    for (i = 0; i < TXPack_pcedCnt; i++) {
        const TXPACK *pack = &TXPack_pced[i];

        value = pmemData[pack->memOffset];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        pcedData[i] = (value >> shift) & mask;
    }
}

/*
 * ValidateBuf() - Corrects any values that fall outside of the valid parameter
 *                 range in a sysex buffer.
 */
void TXPack_ValidateBuf(BYTE *buf, METAINDEX type)
{
    BYTE *data;
    int i;

    switch (type) {
        case META_VCED:
            data = &buf[S_VCED_HDR_SIZE];
            for (i = 0; i < TXPack_vcedCnt; i++) {
                if (data[i] < TXPack_vced[i].loLimit) {
                    data[i] = TXPack_vced[i].loLimit;
                }
                if (data[i] > TXPack_vced[i].hiLimit) {
                    data[i] = TXPack_vced[i].hiLimit;
                }
            }
            break;
        case META_PCED:
            data = &buf[S_PCED_HDR_SIZE];
            for (i = 0; i < TXPack_pcedCnt; i++) {
                if (data[i] < TXPack_pced[i].loLimit) {
                    data[i] = TXPack_pced[i].loLimit;
                }
                if (data[i] > TXPack_pced[i].hiLimit) {
                    data[i] = TXPack_pced[i].hiLimit;
                }
            }
            break;
        case META_FX:
            data = &buf[S_FX_HDR_SIZE];
            for (i = 0; i < TXPack_fxCnt; i++) {
                if (data[i] < TXPack_fx[i].loLimit) {
                    data[i] = TXPack_fx[i].loLimit;
                }
                if (data[i] > TXPack_fx[i].hiLimit) {
                    data[i] = TXPack_fx[i].hiLimit;
                }
            }
            break;
        case META_PC:
            data = &buf[S_PC_HDR_SIZE];
            for (i = 0; i < S_PC_DATA_SIZE; i += 2) {
                if (data[i] > 1) {
                    data[i] = 1;
                }
                if (data[i + 1] > 127) {
                    data[i + 1] = 127;
                }
            }
            break;
        case META_MTO:
            data = &buf[S_MTO_HDR_SIZE];
            for (i = 0; i < S_MTO_DATA_SIZE; i += 2) {
                if (data[i] < 13) {
                    data[i] = 13;
                }
                if (data[i] > 108) {
                    data[i] = 108;
                }
                if (data[i + 1] > 63) {
                    data[i + 1] = 63;
                }
            }
            break;
        case META_MTF:
            data = &buf[S_MTF_HDR_SIZE];
            for (i = 0; i < S_MTF_DATA_SIZE; i += 2) {
                if (data[i] < 13) {
                    data[i] = 13;
                }
                if (data[i] > 108) {
                    data[i] = 108;
                }
                if (data[i + 1] > 63) {
                    data[i + 1] = 63;
                }
            }
            break;
        case META_SYS:
            data = &buf[S_SYS_HDR_SIZE];
            for (i = 0; i < TXPack_sysCnt; i++) {
                if (data[i] < TXPack_sys[i].loLimit) {
                    data[i] = TXPack_sys[i].loLimit;
                }
                if (data[i] > TXPack_sys[i].hiLimit) {
                    data[i] = TXPack_sys[i].hiLimit;
                }
            }
            break;
        case META_VMEM:
            data = &buf[S_VMEM_HDR_SIZE];
            for (i = 0; i < TXPack_vcedCnt; i++) {
                const TXPACK *pack = &TXPack_vced[i];
                int shift = pack->memFirstBit;
                int mask = TXPack_masks[pack->memBitCnt];
                int value = (data[pack->memOffset] >> shift) & mask;

                if (value < pack->loLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->loLimit & mask) << shift;
                }
                if (value > pack->hiLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->hiLimit & mask) << shift;
                }
            }
            for (i = 0; i < TXPack_acedCnt; i++) {
                const TXPACK *pack = &TXPack_aced[i];
                int shift = pack->memFirstBit;
                int mask = TXPack_masks[pack->memBitCnt];
                int value = (data[pack->memOffset] >> shift) & mask;

                if (value < pack->loLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->loLimit & mask) << shift;
                }
                if (value > pack->hiLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->hiLimit & mask) << shift;
                }
            }
            break;
        case META_PMEM:
            data = &buf[S_PMEM_HDR_SIZE];
            for (i = 0; i < TXPack_pcedCnt; i++) {
                const TXPACK *pack = &TXPack_pced[i];
                int shift = pack->memFirstBit;
                int mask = TXPack_masks[pack->memBitCnt];
                int value = (data[pack->memOffset] >> shift) & mask;

                if (value < pack->loLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->loLimit & mask) << shift;
                }
                if (value > pack->hiLimit) {
                    data[pack->memOffset] &= ~(mask << shift);
                    data[pack->memOffset] |= (pack->hiLimit & mask) << shift;
                }
            }
            break;
        case META_ACED:
            data = &buf[S_ACED_HDR_SIZE];
            for (i = 0; i < TXPack_acedCnt; i++) {
                if (data[i] < TXPack_aced[i].loLimit) {
                    data[i] = TXPack_aced[i].loLimit;
                }
                if (data[i] > TXPack_aced[i].hiLimit) {
                    data[i] = TXPack_aced[i].hiLimit;
                }
            }
            break;
    }
}

/*
 * VMemToAvced() - Converts a bank/library stored voice to a voice edit buffer.
 */
void TXPack_VmemToAvced(const BYTE *vmemData, AVCED *avced, BOOL combine)
{
    int i, n, mask, shift, value;
    BYTE *acedData = avced->aced.data;
    BYTE *vcedData = avced->vced.data;
    BYTE funcParams[13];

    /*
     * Save function parameter values if combine mode is off.
     */
    if (!combine) {
        for (i = n = 0; i < tp_acedFuncParamIdxCnt; i++, n++) {
            funcParams[n] = avced->aced.data[tp_acedFuncParamIdxs[i]];
        }
        for (i = 0; i < tp_vcedFuncParamIdxCnt; i++, n++) {
            funcParams[n] = avced->vced.data[tp_vcedFuncParamIdxs[i]];
        }
    }
    /*
     * Unpack the VMEM data into the voice edit buffer.
     */
    for (i = 0; i < TXPack_vcedCnt; i++) {
        const TXPACK *pack = &TXPack_vced[i];

        value = vmemData[pack->memOffset];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        vcedData[i] = (value >> shift) & mask;
    }
    for (i = 0; i < TXPack_acedCnt; i++) {
        const TXPACK *pack = &TXPack_aced[i];

        value = vmemData[pack->memOffset];
        shift = pack->memFirstBit;
        mask = TXPack_masks[pack->memBitCnt];
        acedData[i] = (value >> shift) & mask;
    }
    /*
     * Restore function parameter values if combine mode is off.
     */
    if (!combine) {
        for (i = n = 0; i < tp_acedFuncParamIdxCnt; i++, n++) {
            avced->aced.data[tp_acedFuncParamIdxs[i]] = funcParams[n];
        }
        for (i = 0; i < tp_vcedFuncParamIdxCnt; i++, n++) {
            avced->vced.data[tp_vcedFuncParamIdxs[i]] = funcParams[n];
        }
    }
}

