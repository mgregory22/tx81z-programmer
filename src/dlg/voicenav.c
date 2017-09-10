/*
 * voicenav.c - Keyboard navigation array for the voice editor
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
#include "resource.h"
#include "dlg/voicenav.h"

/*
 * Global variables
 */
KEYNAV VoiceNav_nav[] = {
    /* ctrlID, left, up, right, down */
    { IDC_VOICE_NAME,  0,               0,               IDC_OP1_AR,      IDC_ALGORITHM     },
    { IDC_ALGORITHM,   0,               IDC_VOICE_NAME,  IDC_OP1_WAVE,    IDC_FEEDBACK      },
    { IDC_FEEDBACK,    0,               IDC_ALGORITHM,   IDC_OP1_RANGE,   IDC_OP4_OUT       },
    { IDC_OP1_OUT,     0,               IDC_FEEDBACK,    IDC_OP2_OUT,     IDC_OP1_ENABLE    },
    { IDC_OP2_OUT,     IDC_OP1_OUT,     IDC_FEEDBACK,    IDC_OP3_OUT,     IDC_OP2_ENABLE    },
    { IDC_OP3_OUT,     IDC_OP2_OUT,     IDC_FEEDBACK,    IDC_OP4_OUT,     IDC_OP3_ENABLE    },
    { IDC_OP4_OUT,     IDC_OP3_OUT,     IDC_FEEDBACK,    IDC_OP1_RANGE,   IDC_OP4_ENABLE    },
    { IDC_OP1_ENABLE,  0,               IDC_OP1_OUT,     IDC_OP2_ENABLE,  IDC_OP1_RS        },
    { IDC_OP2_ENABLE,  IDC_OP1_ENABLE,  IDC_OP2_OUT,     IDC_OP3_ENABLE,  IDC_OP2_RS        },
    { IDC_OP3_ENABLE,  IDC_OP2_ENABLE,  IDC_OP3_OUT,     IDC_OP4_ENABLE,  IDC_OP3_RS        },
    { IDC_OP4_ENABLE,  IDC_OP3_ENABLE,  IDC_OP4_OUT,     IDC_OP1_KVS,     IDC_OP4_RS        },
    { IDC_OP1_KVS,     IDC_OP4_ENABLE,  IDC_OP1_DET,     IDC_OP2_KVS,     IDC_LFO_WAVE      },
    { IDC_OP2_KVS,     IDC_OP1_KVS,     IDC_OP1_DET,     IDC_OP3_KVS,     IDC_LFO_WAVE      },
    { IDC_OP3_KVS,     IDC_OP2_KVS,     IDC_OP1_DET,     IDC_OP4_KVS,     IDC_LFO_WAVE      },
    { IDC_OP4_KVS,     IDC_OP3_KVS,     IDC_OP1_DET,     IDC_TRANSPOSE,   IDC_LFO_WAVE      },
    { IDC_OP1_RS,      0,               IDC_OP1_ENABLE,  IDC_OP2_RS,      IDC_OP1_LS        },
    { IDC_OP2_RS,      IDC_OP1_RS,      IDC_OP2_ENABLE,  IDC_OP3_RS,      IDC_OP2_LS        },
    { IDC_OP3_RS,      IDC_OP2_RS,      IDC_OP3_ENABLE,  IDC_OP4_RS,      IDC_OP3_LS        },
    { IDC_OP4_RS,      IDC_OP3_RS,      IDC_OP4_ENABLE,  IDC_OP1_KVS,     IDC_OP4_LS        },
    { IDC_OP1_LS,      0,               IDC_OP1_RS,      IDC_OP2_LS,      0                 },
    { IDC_OP2_LS,      IDC_OP1_LS,      IDC_OP2_RS,      IDC_OP3_LS,      0                 },
    { IDC_OP3_LS,      IDC_OP2_LS,      IDC_OP3_RS,      IDC_OP4_LS,      0                 },
    { IDC_OP4_LS,      IDC_OP3_LS,      IDC_OP4_RS,      IDC_LFO_SYNC,    0                 },
    { IDC_OP1_WAVE,    IDC_ALGORITHM,   IDC_VOICE_NAME,  IDC_OP2_WAVE,    IDC_OP1_FIXED     },
    { IDC_OP2_WAVE,    IDC_OP1_WAVE,    IDC_VOICE_NAME,  IDC_OP3_WAVE,    IDC_OP2_FIXED     },
    { IDC_OP3_WAVE,    IDC_OP2_WAVE,    IDC_VOICE_NAME,  IDC_OP4_WAVE,    IDC_OP3_FIXED     },
    { IDC_OP4_WAVE,    IDC_OP3_WAVE,    IDC_VOICE_NAME,  IDC_OP1_AR,      IDC_OP4_FIXED     },
    { IDC_OP1_FIXED,   IDC_ALGORITHM,   IDC_OP1_WAVE,    IDC_OP2_FIXED,   IDC_OP1_CRS       },
    { IDC_OP2_FIXED,   IDC_OP1_FIXED,   IDC_OP2_WAVE,    IDC_OP3_FIXED,   IDC_OP2_CRS       },
    { IDC_OP3_FIXED,   IDC_OP2_FIXED,   IDC_OP3_WAVE,    IDC_OP4_FIXED,   IDC_OP3_CRS       },
    { IDC_OP4_FIXED,   IDC_OP3_FIXED,   IDC_OP4_WAVE,    IDC_OP1_AR,      IDC_OP4_CRS       },
    { IDC_OP1_RANGE,   IDC_FEEDBACK,    IDC_OP1_FIXED,   IDC_OP1_CRS,     IDC_OP1_DET       },
    { IDC_OP1_CRS,     IDC_OP1_RANGE,   IDC_OP1_FIXED,   IDC_OP1_FINE,    IDC_OP1_DET       },
    { IDC_OP1_FINE,    IDC_OP1_CRS,     IDC_OP1_FIXED,   IDC_OP2_RANGE,   IDC_OP1_DET       },
    { IDC_OP2_RANGE,   IDC_OP1_FINE,    IDC_OP2_FIXED,   IDC_OP2_CRS,     IDC_OP2_DET       },
    { IDC_OP2_CRS,     IDC_OP2_RANGE,   IDC_OP2_FIXED,   IDC_OP2_FINE,    IDC_OP2_DET       },
    { IDC_OP2_FINE,    IDC_OP2_CRS,     IDC_OP2_FIXED,   IDC_OP3_RANGE,   IDC_OP2_DET       },
    { IDC_OP3_RANGE,   IDC_OP2_FINE,    IDC_OP3_FIXED,   IDC_OP3_CRS,     IDC_OP3_DET       },
    { IDC_OP3_CRS,     IDC_OP3_RANGE,   IDC_OP3_FIXED,   IDC_OP3_FINE,    IDC_OP3_DET       },
    { IDC_OP3_FINE,    IDC_OP3_CRS,     IDC_OP3_FIXED,   IDC_OP4_RANGE,   IDC_OP3_DET       },
    { IDC_OP4_RANGE,   IDC_OP3_FINE,    IDC_OP4_FIXED,   IDC_OP4_CRS,     IDC_OP4_DET       },
    { IDC_OP4_CRS,     IDC_OP4_RANGE,   IDC_OP4_FIXED,   IDC_OP4_FINE,    IDC_OP4_DET       },
    { IDC_OP4_FINE,    IDC_OP4_CRS,     IDC_OP4_FIXED,   IDC_OP2_AR,      IDC_OP4_DET       },
    { IDC_OP1_DET,     IDC_OP4_OUT,     IDC_OP1_CRS,     IDC_OP2_DET,     IDC_OP4_KVS       },
    { IDC_OP2_DET,     IDC_OP1_DET,     IDC_OP2_CRS,     IDC_OP3_DET,     IDC_TRANSPOSE     },
    { IDC_OP3_DET,     IDC_OP2_DET,     IDC_OP3_CRS,     IDC_OP4_DET,     IDC_TRANSPOSE     },
    { IDC_OP4_DET,     IDC_OP3_DET,     IDC_OP4_CRS,     IDC_OP3_AR,      IDC_TRANSPOSE     },
    { IDC_TRANSPOSE,   IDC_OP4_KVS,     IDC_OP4_DET,     IDC_OP4_AR,      IDC_REVERB_RATE   },
    { IDC_PB_RANGE,    IDC_OP4_KVS,     IDC_TRANSPOSE,   IDC_REVERB_RATE, IDC_POLY_MODE     },
    { IDC_REVERB_RATE, IDC_PB_RANGE,    IDC_TRANSPOSE,   IDC_OP4_AR,      IDC_PORT_MODE     },
    { IDC_OP1_AR,      IDC_VOICE_NAME,  0,               IDC_OP1_D1R,     IDC_OP2_AR        },
    { IDC_OP2_AR,      IDC_OP4_FINE,    IDC_OP1_AR,      IDC_OP2_D1R,     IDC_OP3_AR        },
    { IDC_OP3_AR,      IDC_OP4_DET,     IDC_OP2_AR,      IDC_OP3_D1R,     IDC_OP4_AR        },
    { IDC_OP4_AR,      IDC_OP4_DET,     IDC_OP3_AR,      IDC_OP4_D1R,     IDC_PORT_TIME     },
    { IDC_OP1_D1R,     IDC_OP1_AR,      0,               IDC_OP1_D1L,     IDC_OP2_D1R       },
    { IDC_OP2_D1R,     IDC_OP2_AR,      IDC_OP1_D1R,     IDC_OP2_D1L,     IDC_OP3_D1R       },
    { IDC_OP3_D1R,     IDC_OP3_AR,      IDC_OP2_D1R,     IDC_OP3_D1L,     IDC_OP4_D1R       },
    { IDC_OP4_D1R,     IDC_OP4_AR,      IDC_OP3_D1R,     IDC_OP4_D1L,     IDC_PORT_TIME     },
    { IDC_OP1_D1L,     IDC_OP1_D1R,     0,               IDC_OP1_D2R,     IDC_OP2_D1L       },
    { IDC_OP2_D1L,     IDC_OP2_D1R,     IDC_OP1_D1L,     IDC_OP2_D2R,     IDC_OP3_D1L       },
    { IDC_OP3_D1L,     IDC_OP3_D1R,     IDC_OP2_D1L,     IDC_OP3_D2R,     IDC_OP4_D1L       },
    { IDC_OP4_D1L,     IDC_OP4_D1R,     IDC_OP3_D1L,     IDC_OP4_D2R,     IDC_PORT_TIME     },
    { IDC_OP1_D2R,     IDC_OP1_D1L,     0,               IDC_OP1_RR,      IDC_OP2_D2R       },
    { IDC_OP2_D2R,     IDC_OP2_D1L,     IDC_OP1_D2R,     IDC_OP2_RR,      IDC_OP3_D2R       },
    { IDC_OP3_D2R,     IDC_OP3_D1L,     IDC_OP2_D2R,     IDC_OP3_RR,      IDC_OP4_D2R       },
    { IDC_OP4_D2R,     IDC_OP4_D1L,     IDC_OP3_D2R,     IDC_OP4_RR,      IDC_PORT_TIME     },
    { IDC_OP1_RR,      IDC_OP1_D2R,     0,               IDC_OP1_SHIFT,   IDC_OP2_RR        },
    { IDC_OP2_RR,      IDC_OP2_D2R,     IDC_OP1_RR,      IDC_OP2_SHIFT,   IDC_OP3_RR        },
    { IDC_OP3_RR,      IDC_OP3_D2R,     IDC_OP2_RR,      IDC_OP3_SHIFT,   IDC_OP4_RR        },
    { IDC_OP4_RR,      IDC_OP4_D2R,     IDC_OP3_RR,      IDC_OP4_SHIFT,   IDC_PORT_TIME     },
    { IDC_OP1_SHIFT,   IDC_OP1_RR,      0,               0,               IDC_OP2_SHIFT     },
    { IDC_OP2_SHIFT,   IDC_OP2_RR,      IDC_OP1_RR,      0,               IDC_OP3_SHIFT     },
    { IDC_OP3_SHIFT,   IDC_OP3_RR,      IDC_OP2_SHIFT,   0,               IDC_OP4_SHIFT     },
    { IDC_OP4_SHIFT,   IDC_OP4_RR,      IDC_OP3_SHIFT,   0,               IDC_PORT_TIME     },
    { IDC_POLY_MODE,   IDC_LFO_WAVE,    IDC_PB_RANGE,    IDC_PORT_MODE,   IDC_PMS           },
    { IDC_PORT_MODE,   IDC_POLY_MODE,   IDC_REVERB_RATE, IDC_PORT_TIME,   IDC_AMS           },
    { IDC_PORT_TIME,   IDC_PORT_MODE,   IDC_OP4_RR,      IDC_BC_PITCH_BIAS, IDC_OP4_EBS     },
    { IDC_LFO_WAVE,    IDC_OP4_RS,      IDC_OP3_KVS,     IDC_PMS,         IDC_LFO_SYNC      },
    { IDC_LFO_SYNC,    IDC_OP4_LS,      IDC_LFO_WAVE,    IDC_PMS,         IDC_LFO_SPEED     },
    { IDC_LFO_SPEED,   IDC_OP4_LS,      IDC_LFO_SYNC,    IDC_LFO_DELAY,   0                 },
    { IDC_LFO_DELAY,   IDC_LFO_SPEED,   IDC_LFO_SYNC,    IDC_LFO_PMD,     0                 },
    { IDC_PMS,         IDC_LFO_SYNC,    IDC_PORT_MODE,   IDC_AMS,         IDC_BC_PITCH      },
    { IDC_LFO_PMD,     IDC_LFO_DELAY,   IDC_PMS,         IDC_MW_PITCH,    0                 },
    { IDC_MW_PITCH,    IDC_LFO_PMD,     IDC_PMS,         IDC_FC_PITCH,    0                 },
    { IDC_FC_PITCH,    IDC_MW_PITCH,    IDC_PMS,         IDC_BC_PITCH,    0                 },
    { IDC_BC_PITCH,    IDC_FC_PITCH,    IDC_PMS,         IDC_LFO_AMD,     0                 },
    { IDC_AMS,         IDC_PMS,         IDC_PORT_MODE,   IDC_OP1_AME,     IDC_MW_AMP        },
    { IDC_OP1_AME,     IDC_AMS,         IDC_PORT_TIME,   IDC_OP2_AME,     IDC_FC_AMP        },
    { IDC_OP2_AME,     IDC_OP1_AME,     IDC_PORT_TIME,   IDC_OP3_AME,     IDC_FC_AMP        },
    { IDC_OP3_AME,     IDC_OP2_AME,     IDC_PORT_TIME,   IDC_OP4_AME,     IDC_BC_AMP        },
    { IDC_OP4_AME,     IDC_OP3_AME,     IDC_PORT_TIME,   IDC_FC_VOL,      IDC_BC_AMP        },
    { IDC_LFO_AMD,     IDC_BC_PITCH,    IDC_AMS,         IDC_MW_AMP,      0                 },
    { IDC_MW_AMP,      IDC_LFO_AMD,     IDC_AMS,         IDC_FC_AMP,      0                 },
    { IDC_FC_AMP,      IDC_MW_AMP,      IDC_OP1_AME,     IDC_BC_AMP,      0                 },
    { IDC_BC_AMP,      IDC_FC_AMP,      IDC_OP3_AME,     IDC_FC_VOL,      0                 },
    { IDC_FC_VOL,      IDC_OP4_AME,     IDC_PORT_TIME,   IDC_BC_EG_BIAS,  0                 },
    { IDC_BC_EG_BIAS,  IDC_FC_VOL,      IDC_PORT_TIME,   IDC_OP1_EBS,     0                 },
    { IDC_OP1_EBS,     IDC_BC_EG_BIAS,  IDC_PORT_TIME,   IDC_OP2_EBS,     0                 },
    { IDC_OP2_EBS,     IDC_OP1_EBS,     IDC_PORT_TIME,   IDC_OP3_EBS,     0                 },
    { IDC_OP3_EBS,     IDC_OP2_EBS,     IDC_PORT_TIME,   IDC_OP4_EBS,     0                 },
    { IDC_OP4_EBS,     IDC_OP3_EBS,     IDC_PORT_TIME,   IDC_BC_PITCH_BIAS, 0               },
    { IDC_BC_PITCH_BIAS, IDC_OP4_EBS,   IDC_PORT_TIME,   0,               0                 },
};
const int VoiceNav_navCnt = ARRAYSIZE(VoiceNav_nav);

KEYALT VoiceNav_alt[] = {
    { IDC_OP1_WAVE,        IDC_OP1_WAVE_RPANEL },
    { IDC_OP2_WAVE,        IDC_OP2_WAVE_RPANEL },
    { IDC_OP3_WAVE,        IDC_OP3_WAVE_RPANEL },
    { IDC_OP4_WAVE,        IDC_OP4_WAVE_RPANEL },
    { IDC_LFO_WAVE,        IDC_LFO_WAVE_RPANEL },
    { IDC_OP1_CRS,         IDC_OP1_SIMPLE_FREQ },
    { IDC_OP1_RANGE,       IDC_OP1_SIMPLE_FREQ },
    { IDC_OP1_FINE,        IDC_OP1_SIMPLE_FREQ },
    { IDC_OP2_CRS,         IDC_OP2_SIMPLE_FREQ },
    { IDC_OP2_RANGE,       IDC_OP2_SIMPLE_FREQ },
    { IDC_OP2_FINE,        IDC_OP2_SIMPLE_FREQ },
    { IDC_OP3_CRS,         IDC_OP3_SIMPLE_FREQ },
    { IDC_OP3_RANGE,       IDC_OP3_SIMPLE_FREQ },
    { IDC_OP3_FINE,        IDC_OP3_SIMPLE_FREQ },
    { IDC_OP4_CRS,         IDC_OP4_SIMPLE_FREQ },
    { IDC_OP4_RANGE,       IDC_OP4_SIMPLE_FREQ },
    { IDC_OP4_FINE,        IDC_OP4_SIMPLE_FREQ },
};
const int VoiceNav_altCnt = ARRAYSIZE(VoiceNav_alt);

