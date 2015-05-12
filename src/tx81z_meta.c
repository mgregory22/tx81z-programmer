/*
 * tx81z_meta.c
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
static BYTE tx_vcedReqStr[16] = "\xF0\x43\x20\x7ELM  8976AE\xF7";
static BYTE tx_pcedReqStr[16] = "\xF0\x43\x20\x7ELM  8976PE\xF7";
static BYTE tx_fxReqStr[16]   = "\xF0\x43\x20\x7ELM  8976S2\xF7";
static BYTE tx_pcReqStr[16]   = "\xF0\x43\x20\x7ELM  8976S1\xF7";
static BYTE tx_mtoReqStr[16]  = "\xF0\x43\x20\x7ELM  MCRTE0\xF7";
static BYTE tx_mtfReqStr[16]  = "\xF0\x43\x20\x7ELM  MCRTE1\xF7";
static BYTE tx_sysReqStr[16]  = "\xF0\x43\x20\x7ELM  8976S0\xF7";
static BYTE tx_vmemReqStr[6]  = "\xF0\x43\x20\x04\xF7";
static BYTE tx_pmemReqStr[16] = "\xF0\x43\x20\x7ELM  8976PM\xF7";

TX81ZMETA TX81Z_meta[META_CNT] = {
    {
        REQ_VCED,                              /* reqFlag    */
        SI_VCED,                               /* sIndex     */
        _T("voice edit buffer"),               /* dumpName   */
        _T("Voice Edit Buffer"),               /* nameFmt    */
        _T("the voice edit buffer"),           /* proseName  */
        _T("VCED"),                            /* typeStr    */
        tx_vcedReqStr,                         /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x03\x00\x5D",            /* dumpHdr    */
        6,                                     /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.avced.vced,  /* buf        */
        S_VCED_SIZE,                           /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, avced.vced),        /* offset     */
        Prog_snapshot.avced.vced.data,         /* dataPtr    */
        0                                      /* libSize    */
    }, {
        REQ_PCED,                              /* reqFlag    */
        SI_PCED,                               /* sIndex     */
        _T("performance edit buffer"),         /* dumpName   */
        _T("Performance Edit Buffer"),         /* nameFmt    */
        _T("the performance edit buffer"),     /* proseName  */
        _T("PCED"),                            /* typeStr    */
        tx_pcedReqStr,                         /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x00\x78LM  8976PE",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.pced,        /* buf        */
        S_PCED_SIZE,                           /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, pced),              /* offset     */
        Prog_snapshot.pced.data,               /* dataPtr    */
        0                                      /* libSize    */
    }, {
        REQ_FX,                                /* reqFlag    */
        SI_FX,                                 /* sIndex     */
        _T("effect settings"),                 /* dumpName   */
        _T("Effect Settings"),                 /* nameFmt    */
        _T("the effect settings"),             /* proseName  */
        _T("FX  "),                            /* typeStr    */
        tx_fxReqStr,                           /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x00\x41LM  8976S2",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.fx,          /* buf        */
        S_FX_SIZE,                             /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, fx),                /* offset     */
        Prog_snapshot.fx.data,                 /* dataPtr    */
        sizeof Prog_snapshot.fx.data           /* libSize    */
    }, {
        REQ_PC,                                /* reqFlag    */
        SI_PC,                                 /* sIndex     */
        _T("program change table"),            /* dumpName   */
        _T("Program Change Table"),            /* nameFmt    */
        _T("the program change table"),        /* proseName  */
        _T("PC  "),                            /* typeStr    */
        tx_pcReqStr,                           /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x02\x0ALM  8976S1",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.pc,          /* buf        */
        S_PC_SIZE,                             /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, pc),                /* offset     */
        Prog_snapshot.pc.data,                 /* dataPtr    */
        sizeof Prog_snapshot.pc.data           /* libSize    */
    }, {
        REQ_MTO,                               /* reqFlag    */
        SI_MTO,                                /* sIndex     */
        _T("microtunings (octave)"),           /* dumpName   */
        _T("Microtunings (Octave)"),           /* nameFmt    */
        _T("the octave microtunings"),         /* proseName  */
        _T("MTO "),                            /* typeStr    */
        tx_mtoReqStr,                          /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x00\x22LM  MCRTE0",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.mto,         /* buf        */
        S_MTO_SIZE,                            /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, mto),               /* offset     */
        Prog_snapshot.mto.data,                /* dataPtr    */
        sizeof Prog_snapshot.mto.data          /* libSize    */
    }, {
        REQ_MTF,                                /* reqFlag    */
        SI_MTF,                                 /* sIndex     */
        _T("microtunings (full kbd)"),          /* dumpName   */
        _T("Microtunings (Full Kbd)"),          /* nameFmt    */
        _T("the full keyboard microtunings"),   /* proseName  */
        _T("MTF "),                             /* typeStr    */
        tx_mtfReqStr,                           /* reqStr     */
        15,                                     /* reqStrLen  */
        "\xF0\x43\x00\x7E\x02\x0ALM  MCRTE1",   /* dumpHdr    */
        16,                                     /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.mtf,          /* buf        */
        S_MTF_SIZE,                             /* bufSize    */
        0,                                      /* recorded   */
        offsetof(SNAPSHOT, mtf),                /* offset     */
        Prog_snapshot.mtf.data,                 /* dataPtr    */
        sizeof Prog_snapshot.mtf.data           /* libSize    */
    }, {
        REQ_SYS,                               /* reqFlag    */
        SI_SYS,                                /* sIndex     */
        _T("system setup"),                    /* dumpName   */
        _T("System Setup"),                    /* nameFmt    */
        _T("the system setup"),                /* proseName  */
        _T("SYS "),                            /* typeStr    */
        tx_sysReqStr,                          /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x00\x25LM  8976S0",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.sys,         /* buf        */
        S_SYS_SIZE,                            /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, sys),               /* offset     */
        Prog_snapshot.sys.data,                /* dataPtr    */
        sizeof Prog_snapshot.sys.data          /* libSize    */
    }, {
        REQ_VMEM,                          /* reqFlag    */
        SI_VMEM,                           /* sIndex     */
        tx_vmemName,                       /* dumpName   */
        tx_bankNameFmt,                    /* nameFmt    */
        tx_vmemName,                       /* proseName  */
        _T("I%02d "),                      /* typeStr    */
        tx_vmemReqStr,                     /* reqStr     */
        5,                                 /* reqStrLen  */
        "\xF0\x43\x00\x04\x20\x00",        /* dumpHdr    */
        6,                                 /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.vmem,    /* buf        */
        S_VMEM_SIZE,                       /* bufSize    */
        0,                                 /* recorded   */
        offsetof(SNAPSHOT, vmem),          /* offset     */
        Prog_snapshot.vmem.data[0],        /* dataPtr    */
        sizeof Prog_snapshot.vmem.data[0]  /* libSize    */
    }, {
        REQ_PMEM,                              /* reqFlag    */
        SI_PMEM,                               /* sIndex     */
        tx_pmemName,                           /* dumpName   */
        tx_bankNameFmt,                        /* nameFmt    */
        tx_pmemName,                           /* proseName  */
        _T("PF%02d"),                          /* typeStr    */
        tx_pmemReqStr,                         /* reqStr     */
        15,                                    /* reqStrLen  */
        "\xF0\x43\x00\x7E\x13\x0ALM  8976PM",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.pmem,        /* buf        */
        S_PMEM_SIZE,                           /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, pmem),              /* offset     */
        Prog_snapshot.pmem.data[0],            /* dataPtr    */
        sizeof Prog_snapshot.pmem.data[0]      /* libSize    */
    }, {
        REQ_PRESET_A,                           /* reqFlag    */
        SI_PRESET_A,                            /* sIndex     */
        tx_bankAName,                           /* dumpName   */
        tx_bankNameFmt,                         /* nameFmt    */
        tx_bankAName,                           /* proseName  */
        _T("A%02d "),                           /* typeStr    */
        NULL,                                   /* reqStr     */
        0,                                      /* reqStrLen  */
        "\xF0\x43\x00\x04\x20\x00",             /* dumpHdr    */
        6,                                      /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.preset[0],    /* buf        */
        S_VMEM_SIZE,                            /* bufSize    */
        0,                                      /* recorded   */
        offsetof(SNAPSHOT, preset[0]),          /* offset     */
        Prog_snapshot.preset[0].data[0],        /* dataPtr    */
        sizeof Prog_snapshot.preset[0].data[0]  /* libSize    */
    }, {
        REQ_PRESET_B,                           /* reqFlag    */
        SI_PRESET_B,                            /* sIndex     */
        tx_bankBName,                           /* dumpName   */
        tx_bankNameFmt,                         /* nameFmt    */
        tx_bankBName,                           /* proseName  */
        _T("B%02d "),                           /* typeStr    */
        NULL,                                   /* reqStr     */
        0,                                      /* reqStrLen  */
        "\xF0\x43\x00\x04\x20\x00",             /* dumpHdr    */
        6,                                      /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.preset[1],    /* buf        */
        S_VMEM_SIZE,                            /* bufSize    */
        0,                                      /* recorded   */
        offsetof(SNAPSHOT, preset[1]),          /* offset     */
        Prog_snapshot.preset[1].data[0],        /* dataPtr    */
        sizeof Prog_snapshot.preset[1].data[0]  /* libSize    */
    }, {
        REQ_PRESET_C,                           /* reqFlag    */
        SI_PRESET_C,                            /* sIndex     */
        tx_bankCName,                           /* dumpName   */
        tx_bankNameFmt,                         /* nameFmt    */
        tx_bankCName,                           /* proseName  */
        _T("C%02d "),                           /* typeStr    */
        NULL,                                   /* reqStr     */
        0,                                      /* reqStrLen  */
        "\xF0\x43\x00\x04\x20\x00",             /* dumpHdr    */
        6,                                      /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.preset[2],    /* buf        */
        S_VMEM_SIZE,                            /* bufSize    */
        0,                                      /* recorded   */
        offsetof(SNAPSHOT, preset[2]),          /* offset     */
        Prog_snapshot.preset[2].data[0],        /* dataPtr    */
        sizeof Prog_snapshot.preset[2].data[0]  /* libSize    */
    }, {
        REQ_PRESET_D,                           /* reqFlag    */
        SI_PRESET_D,                            /* sIndex     */
        tx_bankDName,                           /* dumpName   */
        tx_bankNameFmt,                         /* nameFmt    */
        tx_bankDName,                           /* proseName  */
        _T("D%02d "),                           /* typeStr    */
        NULL,                                   /* reqStr     */
        0,                                      /* reqStrLen  */
        "\xF0\x43\x00\x04\x20\x00",             /* dumpHdr    */
        6,                                      /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.preset[3],    /* buf        */
        S_VMEM_SIZE,                            /* bufSize    */
        0,                                      /* recorded   */
        offsetof(SNAPSHOT, preset[3]),          /* offset     */
        Prog_snapshot.preset[3].data[0],        /* dataPtr    */
        sizeof Prog_snapshot.preset[3].data[0]  /* libSize    */
    }, {
        REQ_ACED,                              /* reqFlag    */
        -1,                                    /* sIndex     */
        NULL,                                  /* dumpName   */
        NULL,                                  /* nameFmt    */
        NULL,                                  /* proseName  */
        NULL,                                  /* typeStr    */
        NULL,                                  /* reqStr     */
        0,                                     /* reqStrLen  */
        "\xF0\x43\x00\x7E\x00\x21LM  8976AE",  /* dumpHdr    */
        16,                                    /* dumpHdrLen */
        (BYTE *) &tx_tempSnapshot.avced.aced,  /* buf        */
        S_ACED_SIZE,                           /* bufSize    */
        0,                                     /* recorded   */
        offsetof(SNAPSHOT, avced.aced),        /* offset     */
        Prog_snapshot.avced.aced.data,         /* dataPtr    */
        0                                      /* libSize    */
    }, {
        REQ_VOICE_MODE,                        /* reqFlag    */
        -1,                                    /* sIndex     */
        NULL,                                  /* dumpName   */
        NULL,                                  /* nameFmt    */
        NULL,                                  /* proseName  */
        NULL,                                  /* typeStr    */
        NULL,                                  /* reqStr     */
        0,                                     /* reqStrLen  */
        NULL,                                  /* dumpHdr    */
        0,                                     /* dumpHdrLen */
        NULL,                                  /* buf        */
        0,                                     /* bufSize    */
        0,                                     /* recorded   */
        0,                                     /* offset     */
        NULL,                                  /* dataPtr    */
        0                                      /* libSize    */
    }, {
        REQ_PFM_MODE,                          /* reqFlag    */
        -1,                                    /* sIndex     */
        NULL,                                  /* dumpName   */
        NULL,                                  /* nameFmt    */
        NULL,                                  /* proseName  */
        NULL,                                  /* typeStr    */
        NULL,                                  /* reqStr     */
        0,                                     /* reqStrLen  */
        NULL,                                  /* dumpHdr    */
        0,                                     /* dumpHdrLen */
        NULL,                                  /* buf        */
        0,                                     /* bufSize    */
        0,                                     /* recorded   */
        0,                                     /* offset     */
        NULL,                                  /* dataPtr    */
        0                                      /* libSize    */
    }
};
