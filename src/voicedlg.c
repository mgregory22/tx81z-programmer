/*
 * voicedlg.c - TX81Z voice editor dialog
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
#include "algctrl.h"
#include "egctrl.h"
#include "lcdctrl.h"
#include "mainwnd.h"
#include "menubtn.h"
#include "minifont.h"
#include "prog.h"
#include "resource.h"
#include "rpanel.h"
#include "snapshot.h"
#include "storedlg.h"
#include "tx81z.h"
#include "txpack.h"
#include "undo.h"
#include "voicenav.h"
#include "voicedlg.h"


/*
 * Global constants
 */
const _TUCHAR *VoiceDlg_className = _T("VoiceDlg");

/*
 * Global procedures
 */
extern BOOL VoiceDlg_Create(HWND parentWnd, AVCED *voice);
extern int VoiceDlg_FixedFreqSortCmp(const FIXEDFREQSORT *ffs1, const FIXEDFREQSORT *ffs2);
extern int VoiceDlg_FixedParamsToFreq(int range, int crs, int fine);
extern void VoiceDlg_Update(HWND voiceDlg, AVCED *voice);

/*
 * Global variables
 */
FIXEDFREQSORT VoiceDlg_fixedFreqSort[SIMPLE_FIXED_RANGE_MAX];
_TUCHAR VoiceDlg_simpleRatioSettings[SIMPLE_RATIO_RANGE_MAX][5];
_TUCHAR VoiceDlg_simpleFixedSettings[SIMPLE_FIXED_RANGE_MAX][5];
BOOL VoiceDlg_fastWaveSelection;
BOOL VoiceDlg_simpleFreqSelection;
BOOL VoiceDlg_hideEGLabels;
BOOL VoiceDlg_hideScrollBars;
MOUSEWHEEL VoiceDlg_mouseWheel;

/*
 * Unit types
 */
typedef struct {
    UINT ctrlID;
    HWND wnd;
} CTRLPAIR;

typedef enum {
    FCI_FREQSTC = 0,
    FCI_RANGE   = 1,
    FCI_CRS     = 2,
    FCI_FINE    = 3,
    FCI_SIMPLE  = 4,
    FCI_CNT     = 5
} FREQCTRLIDX;

typedef struct {
    CTRLPAIR ctrl[FCI_CNT];
    UINT fixedIdx;
} OPFREQCTRLS;

/*
 * Unit procedures
 */
static void vd_AdjustFrigginEGControls(HWND voiceDlg);
static void vd_ChangeAcedParameter(HWND voiceDlg, int parameter, int value, BOOL send);
static void vd_ChangeControl(HWND voiceDlg, UINT ctrlID, int value, BOOL send);
static void vd_ChangeVcedParameter(HWND voiceDlg, int parameter, int value, BOOL send);
static int vd_CheckFreqSliderRange(HWND voiceDlg, int op);
static int vd_CtrlIdToOpNumber(UINT ctrlID);
BOOL CALLBACK vd_DlgProc(HWND voiceDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void vd_DrawBkgnd(HDC dC);
static void vd_EraseStretchingEdges(HDC dC);
static void vd_FastWaveSelection(HWND voiceDlg, BOOL fastOn);
static void vd_FocusCtrl(HWND voiceDlg, UINT ctrlID);
static void vd_InitControlValues(HWND voiceDlg);
static BOOL vd_IsOpFollowing(int a, int b);
static void vd_OnActivate(HWND voiceDlg, UINT state, HWND otherWnd, BOOL minimized);
static void vd_OnCommand(HWND voiceDlg, unsigned ctrlID, HWND ctrl, UINT notify);
static void vd_OnDestroy(HWND voiceDlg);
static void vd_OnDrawItem(HWND voiceDlg, const DRAWITEMSTRUCT *drawItem);
static void vd_OnEGOriginScroll(HWND voiceDlg, HWND ctrl, UINT code, int pos);
static BOOL vd_OnEraseBkgnd(HWND voiceDlg, HDC dC);
static BOOL vd_OnInitDialog(HWND voiceDlg, HWND focusCtrl, LPARAM lParam);
static void vd_OnInitMenu(HWND voiceDlg, HMENU menu);
static void vd_OnKey(HWND voiceDlg, UINT vk, BOOL down, int repeat, UINT flags);
static void vd_OnMeasureItem(HWND voiceDlg, MEASUREITEMSTRUCT *measureItem);
static void vd_OnMouseMove(HWND voiceDlg, int x, int y, UINT keyFlags);
static void vd_OnMouseWheel(HWND voiceDlg, int delta, int x, int y, UINT keyFlags);
static void vd_OnRButtonDown(HWND voiceDlg, BOOL dblClick, int x, int y, UINT keyFlags);
static int vd_OnRButtonUp(HWND voiceDlg, int x, int y, UINT keyFlags);
static void vd_OnSimpleFreqChange(HWND voiceDlg, UINT ctrlID, int value);
static void vd_OnSize(HWND voiceDlg, UINT state, int cx, int cy);
static LRESULT CALLBACK vd_OriginWndProc(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam);
static int vd_PageSizeCrsFreq(UINT ctrlID, int dir, int value);
static int vd_PageSizeSimpleFixed(UINT ctrlID, int dir, int value);
static int vd_PageSizeSimpleRatio(UINT ctrlID, int dir, int value);
static void vd_QuickEditCopy(HWND voiceDlg, UINT copyOpIdx, UINT quickEdit);
static void vd_QuickEditRand(HWND voiceDlg, int op, UINT quickEdit);
static void vd_QuickEditSwap(HWND voiceDlg, UINT swapOpIdx, UINT quickEdit);
static void vd_RandomizeVoice(HWND voiceDlg);
static void CALLBACK vd_Redo(HWND voiceDlg, CHANGE *change);
static void vd_SimpleFreqSelection(HWND voiceDlg, BOOL simpleOn);
static BOOL CALLBACK vd_SubclassControls(HWND dlgCtrl, LPARAM lParam);
static void CALLBACK vd_Undo(HWND voiceDlg, CHANGE *change);
static void vd_UpdateFixedRatioStatus(HWND voiceDlg, int op);
static void vd_UpdateFrequencyDisplay(HWND voiceDlg, int op);
static void vd_UpdateQuickEditMenu(UINT ctrlID);
static void vd_UpdateSimpleFreqValue(HWND voiceDlg, int op, BOOL simpleOn);


/*
 * Unit constants
 */
#define TEXT_LEN  80
static const _TUCHAR vd_fixedRangeSettings[40] = {
    '2', '5', '5', 'H', 'z',
    '5', '1', '0', 'H', 'z',
    ' ', '1', 'K', 'H', 'z',
    ' ', '2', 'K', 'H', 'z',
    ' ', '4', 'K', 'H', 'z',
    ' ', '8', 'K', 'H', 'z',
    '1', '6', 'K', 'H', 'z',
    '3', '2', 'K', 'h', 'z', 
};
static _TUCHAR vd_egShiftSettings[8] = {
    'o', 'f',
    '4', '8',
    '2', '4',
    '1', '2'
};
static _TUCHAR vd_reverbRateSettings[24] = {
    'o', 'f', 'f',
    ' ', '1', ' ',
    ' ', '2', ' ',
    ' ', '3', ' ',
    ' ', '4', ' ',
    ' ', '5', ' ',
    ' ', '6', ' ',
    ' ', '7', ' ',
};

static const NUMLCDINIT vd_numLcdInits[] = {
/* UINT ctrlID, int maxLen, int min, int max, int offset, LCDPAGEFUNC LcdPageFunc */
    { IDC_FEEDBACK,      1,   0,   7,   0,         NULL },
    { IDC_OP1_OUT,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP2_OUT,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP3_OUT,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP4_OUT,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_LFO_SPEED,     2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_LFO_DELAY,     2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_LFO_PMD,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_LFO_AMD,       2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_PMS,           1,   0,   7,   0,         NULL },
    { IDC_AMS,           1,   0,   3,   0,         NULL },
    { IDC_OP1_EBS,       1,   0,   7,   0,         NULL },
    { IDC_OP2_EBS,       1,   0,   7,   0,         NULL },
    { IDC_OP3_EBS,       1,   0,   7,   0,         NULL },
    { IDC_OP4_EBS,       1,   0,   7,   0,         NULL },
    { IDC_OP1_KVS,       1,   0,   7,   0,         NULL },
    { IDC_OP2_KVS,       1,   0,   7,   0,         NULL },
    { IDC_OP3_KVS,       1,   0,   7,   0,         NULL },
    { IDC_OP4_KVS,       1,   0,   7,   0,         NULL },
    { IDC_OP1_RANGE,     1,   0,   7,   0,         NULL },
    { IDC_OP2_RANGE,     1,   0,   7,   0,         NULL },
    { IDC_OP3_RANGE,     1,   0,   7,   0,         NULL },
    { IDC_OP4_RANGE,     1,   0,   7,   0,         NULL },
    { IDC_OP1_CRS,       2,   0,  63,   0, vd_PageSizeCrsFreq },
    { IDC_OP2_CRS,       2,   0,  63,   0, vd_PageSizeCrsFreq },
    { IDC_OP3_CRS,       2,   0,  63,   0, vd_PageSizeCrsFreq },
    { IDC_OP4_CRS,       2,   0,  63,   0, vd_PageSizeCrsFreq },
    { IDC_OP1_FINE,      2,   0,  15,   0,         NULL },
    { IDC_OP2_FINE,      2,   0,  15,   0,         NULL },
    { IDC_OP3_FINE,      2,   0,  15,   0,         NULL },
    { IDC_OP4_FINE,      2,   0,  15,   0,         NULL },
    { IDC_OP1_DET,       2,   0,   6,  -3,         NULL },
    { IDC_OP2_DET,       2,   0,   6,  -3,         NULL },
    { IDC_OP3_DET,       2,   0,   6,  -3,         NULL },
    { IDC_OP4_DET,       2,   0,   6,  -3,         NULL },
    { IDC_OP1_RS,        1,   0,   3,   0,         NULL },
    { IDC_OP2_RS,        1,   0,   3,   0,         NULL },
    { IDC_OP3_RS,        1,   0,   3,   0,         NULL },
    { IDC_OP4_RS,        1,   0,   3,   0,         NULL },
    { IDC_OP1_LS,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP2_LS,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP3_LS,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_OP4_LS,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_PORT_TIME,     2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_FC_VOL,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_FC_PITCH,      2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_FC_AMP,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_MW_PITCH,      2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_MW_AMP,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_BC_PITCH,      2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_BC_AMP,        2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_BC_PITCH_BIAS, 2, -50, +50,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_BC_EG_BIAS,    2,   0,  99,   0, LcdCtrl_PageSizeSnap5 },
    { IDC_PB_RANGE,      2,   0,  12,   0,         NULL },
    { IDC_OP1_AR,        2,   0,  31,   0,         NULL },
    { IDC_OP2_AR,        2,   0,  31,   0,         NULL },
    { IDC_OP3_AR,        2,   0,  31,   0,         NULL },
    { IDC_OP4_AR,        2,   0,  31,   0,         NULL },
    { IDC_OP1_D1R,       2,   0,  31,   0,         NULL },
    { IDC_OP2_D1R,       2,   0,  31,   0,         NULL },
    { IDC_OP3_D1R,       2,   0,  31,   0,         NULL },
    { IDC_OP4_D1R,       2,   0,  31,   0,         NULL },
    { IDC_OP1_D1L,       2,   0,  15,   0,         NULL },
    { IDC_OP2_D1L,       2,   0,  15,   0,         NULL },
    { IDC_OP3_D1L,       2,   0,  15,   0,         NULL },
    { IDC_OP4_D1L,       2,   0,  15,   0,         NULL },
    { IDC_OP1_D2R,       2,   0,  31,   0,         NULL },
    { IDC_OP2_D2R,       2,   0,  31,   0,         NULL },
    { IDC_OP3_D2R,       2,   0,  31,   0,         NULL },
    { IDC_OP4_D2R,       2,   0,  31,   0,         NULL },
    { IDC_OP1_RR,        2,   1,  15,   0,         NULL },
    { IDC_OP2_RR,        2,   1,  15,   0,         NULL },
    { IDC_OP3_RR,        2,   1,  15,   0,         NULL },
    { IDC_OP4_RR,        2,   1,  15,   0,         NULL },
    { IDC_BC_PITCH_BIAS, 3,   0, 100, -50, LcdCtrl_PageSizeSnap5 },
};
#define vd_numLcdInitCnt ARRAYSIZE(vd_numLcdInits)

static const SPECIALLCDINIT vd_specialLcdInits[] = {
/* UINT ctrlID, int maxLen, int min, int max, _TUCHAR *strings, LCDPAGEFUNC LcdPageFunc */
    { IDC_TRANSPOSE,   3, 0, 48, Prog_keyNameStrings[36], LcdCtrl_PageSize12 },
    { IDC_OP1_RANGE,   5, 0,  7,   vd_fixedRangeSettings, NULL },
    { IDC_OP2_RANGE,   5, 0,  7,   vd_fixedRangeSettings, NULL },
    { IDC_OP3_RANGE,   5, 0,  7,   vd_fixedRangeSettings, NULL },
    { IDC_OP4_RANGE,   5, 0,  7,   vd_fixedRangeSettings, NULL },
    { IDC_OP1_SHIFT,   2, 0,  0,      vd_egShiftSettings, NULL },
    { IDC_OP2_SHIFT,   2, 0,  3,      vd_egShiftSettings, NULL },
    { IDC_OP3_SHIFT,   2, 0,  3,      vd_egShiftSettings, NULL },
    { IDC_OP4_SHIFT,   2, 0,  3,      vd_egShiftSettings, NULL },
    { IDC_REVERB_RATE, 3, 0,  7,   vd_reverbRateSettings, NULL },
};
#define vd_specialLcdInitCnt ARRAYSIZE(vd_specialLcdInits)

#include "freqratios.c"

static const SPECIALLCDINIT vd_simpleRatioLcdInits[] = {
    { IDC_OP1_SIMPLE_FREQ, 5, 0, 991, VoiceDlg_simpleRatioSettings[0], vd_PageSizeSimpleRatio },
    { IDC_OP2_SIMPLE_FREQ, 5, 0, 991, VoiceDlg_simpleRatioSettings[0], vd_PageSizeSimpleRatio },
    { IDC_OP3_SIMPLE_FREQ, 5, 0, 991, VoiceDlg_simpleRatioSettings[0], vd_PageSizeSimpleRatio },
    { IDC_OP4_SIMPLE_FREQ, 5, 0, 991, VoiceDlg_simpleRatioSettings[0], vd_PageSizeSimpleRatio },
};
#define vd_simpleRatioLcdInitCnt ARRAYSIZE(vd_simpleRatioLcdInits)

static const SPECIALLCDINIT vd_simpleFixedLcdInits[] = {
    { IDC_OP1_SIMPLE_FREQ, 5, 0, 1983, VoiceDlg_simpleFixedSettings[0], vd_PageSizeSimpleFixed },
    { IDC_OP2_SIMPLE_FREQ, 5, 0, 1983, VoiceDlg_simpleFixedSettings[0], vd_PageSizeSimpleFixed },
    { IDC_OP3_SIMPLE_FREQ, 5, 0, 1983, VoiceDlg_simpleFixedSettings[0], vd_PageSizeSimpleFixed },
    { IDC_OP4_SIMPLE_FREQ, 5, 0, 1983, VoiceDlg_simpleFixedSettings[0], vd_PageSizeSimpleFixed },
};
#define vd_simpleFixedLcdInitCnt ARRAYSIZE(vd_simpleFixedLcdInits)

static const _TUCHAR vd_ratio[] = _T("Ratio");
static const _TUCHAR vd_fixed[] = _T("Fixed");
static const _TUCHAR vd_polyMode[] = _T("Poly Mode");
static const _TUCHAR vd_monoMode[] = _T("Mono Mode");
static const _TUCHAR vd_fullTime[] = _T("Full Time Porta");
static const _TUCHAR vd_fingered[] = _T("Fingered Porta");
static const _TUCHAR vd_absolute[] = _T("Absolute");
static const _TUCHAR vd_relative[] = _T("Relative");

static struct {
    UINT ctrlID;
    const _TUCHAR *modeOn;
    const _TUCHAR *modeOff;
} vd_toggleInits[] = {
    { IDC_POLY_MODE, vd_monoMode, vd_polyMode },
    { IDC_PORT_MODE, vd_fingered, vd_fullTime },
    { IDC_LFO_SYNC, NULL, NULL },
    { IDC_OP1_AME, NULL, NULL },
    { IDC_OP2_AME, NULL, NULL },
    { IDC_OP3_AME, NULL, NULL },
    { IDC_OP4_AME, NULL, NULL },
    { IDC_OP1_FIXED, vd_fixed, vd_ratio },
    { IDC_OP2_FIXED, vd_fixed, vd_ratio },
    { IDC_OP3_FIXED, vd_fixed, vd_ratio },
    { IDC_OP4_FIXED, vd_fixed, vd_ratio },
};
#define vd_toggleInitCnt ARRAYSIZE(vd_toggleInits)

static const _TUCHAR vd_opWaveInits[8][4] = {
    { 'W', '1', _T('\x80'), _T('\x81') },
    { 'W', '2', _T('\x82'), _T('\x83') },
    { 'W', '3', _T('\x80'), _T('-') },
    { 'W', '4', _T('\x82'), _T('-') },
    { 'W', '5', _T('\x84'), _T('-') },
    { 'W', '6', _T('\x85'), _T('-') },
    { 'W', '7', _T('\x86'), _T('-') },
    { 'W', '8', _T('\x87'), _T('-') }
};

static const int vd_opVcedOffsets[4] = {
    TX81Z_VCED_OP1_AR,
    TX81Z_VCED_OP2_AR,
    TX81Z_VCED_OP3_AR,
    TX81Z_VCED_OP4_AR,
};
static const int vd_opAcedOffsets[4] = {
    TX81Z_ACED_OP1_FIXED,
    TX81Z_ACED_OP2_FIXED,
    TX81Z_ACED_OP3_FIXED,
    TX81Z_ACED_OP4_FIXED,
};
static const int vd_opSimpleOffsets[4] = {
    IDC_OP1_SIMPLE_FREQ,
    IDC_OP2_SIMPLE_FREQ,
    IDC_OP3_SIMPLE_FREQ,
    IDC_OP4_SIMPLE_FREQ,
};
static const int vd_opRPanelOffsets[4] = {
    IDC_OP1_WAVE_RPANEL,
    IDC_OP2_WAVE_RPANEL,
    IDC_OP3_WAVE_RPANEL,
    IDC_OP4_WAVE_RPANEL,
};

#define vd_egCtrlIDCnt  8
static const UINT vd_egCtrlIDs[vd_egCtrlIDCnt][4] = {
    { IDC_OP1_AR,    IDC_OP2_AR,    IDC_OP3_AR,    IDC_OP4_AR    },
    { IDC_OP1_D1R,   IDC_OP2_D1R,   IDC_OP3_D1R,   IDC_OP4_D1R   },
    { IDC_OP1_D1L,   IDC_OP2_D1L,   IDC_OP3_D1L,   IDC_OP4_D1L   },
    { IDC_OP1_D2R,   IDC_OP2_D2R,   IDC_OP3_D2R,   IDC_OP4_D2R   },
    { IDC_OP1_RR,    IDC_OP2_RR,    IDC_OP3_RR,    IDC_OP4_RR    },
    { IDC_OP1_SHIFT, IDC_OP2_SHIFT, IDC_OP3_SHIFT, IDC_OP4_SHIFT },
    { IDC_OP1_EBS,   IDC_OP2_EBS,   IDC_OP3_EBS,   IDC_OP4_EBS   },
    { IDC_OP1_EG,    IDC_OP2_EG,    IDC_OP3_EG,    IDC_OP4_EG    },
};

#define vd_freqCtrlIDCnt  6
static const UINT vd_freqCtrlIDs[vd_freqCtrlIDCnt][4] = {
    { IDC_OP1_CRS,   IDC_OP2_CRS,   IDC_OP3_CRS,   IDC_OP4_CRS   },
    { IDC_OP1_DET,   IDC_OP2_DET,   IDC_OP3_DET,   IDC_OP4_DET   }, 
    { IDC_OP1_FIXED, IDC_OP2_FIXED, IDC_OP3_FIXED, IDC_OP4_FIXED }, 
    { IDC_OP1_RANGE, IDC_OP2_RANGE, IDC_OP3_RANGE, IDC_OP4_RANGE }, 
    { IDC_OP1_FINE,  IDC_OP2_FINE,  IDC_OP3_FINE,  IDC_OP4_FINE  }, 
    { IDC_OP1_WAVE,  IDC_OP2_WAVE,  IDC_OP3_WAVE,  IDC_OP4_WAVE  },
};

#define vd_outputCtrlIDCnt  4
static const UINT vd_outputCtrlIDs[vd_outputCtrlIDCnt][4] = {
    { IDC_OP1_LS,  IDC_OP2_LS,  IDC_OP3_LS,  IDC_OP4_LS }, 
    { IDC_OP1_RS,  IDC_OP2_RS,  IDC_OP3_RS,  IDC_OP4_RS },
    { IDC_OP1_KVS, IDC_OP2_KVS, IDC_OP3_KVS, IDC_OP4_KVS }, 
    { IDC_OP1_OUT, IDC_OP2_OUT, IDC_OP3_OUT, IDC_OP4_OUT },
};

static const BOOL vd_algModulators[8][4] = {
    { FALSE,  TRUE,  TRUE,  TRUE },
    { FALSE,  TRUE,  TRUE,  TRUE },
    { FALSE,  TRUE,  TRUE,  TRUE },
    { FALSE,  TRUE,  TRUE,  TRUE },
    { FALSE,  TRUE, FALSE,  TRUE },
    { FALSE, FALSE, FALSE,  TRUE },
    { FALSE, FALSE, FALSE,  TRUE },
    { FALSE, FALSE, FALSE, FALSE }
};
static const _TUCHAR *vd_optionMenuHeaders[] = {
    _T("Frequency Options:"),
    _T("EG Options:"),
    _T("Mouse Options:"),
    _T("Window Options:"),
};
#define vd_optionMenuHeaderCnt ARRAYSIZE(vd_optionMenuHeaders)

/*
 * Unit variables
 */
static HWND vd_parentWnd;
static BYTE *vd_acedData;
static BYTE *vd_vcedData;
static HMENU vd_menu;
static UNDO *vd_undo;  /* the undo list */
static BOOL vd_undoGroup;  /* true if the next item added to the undo list
                              should be the start of a group of related
                              changes */
static BOOL vd_undoFlag;  /* to prevent changes from getting added to the undo
                             list while undoing */
static BOOL vd_simpleUpdateFlag; /* to prevent the simple controls from being
                                    continually reset when the user is trying
                                    to change them */
static int vd_optionMenuHeaderWidth;
static DIALOG *vd_dialog;
static WNDPROC vd_origOriginWndProc;
static int vd_opEnable;
static int vd_egLengths[4];
static int vd_mDragLastX, vd_mDragLastY;
static BOOL vd_dirty; /* if this is false, any change in the editor should
                         send a signal to the main window to dirty the item */
static unsigned vd_opFollow;
static BOOL vd_opFollowRecursion;
static BOOL vd_carrierFollow;
static BOOL vd_modulatorFollow;
static BOOL vd_absoluteFollow;
static UINT vd_quickEdit; /* indicates which quickedit menu was active when
                             a quickedit command was issued */
static HWND vd_algQuickEditBtn;
static HWND vd_egQuickEditBtn;
static HWND vd_freqQuickEditBtn;
static HWND vd_outputQuickEditBtn;
static HWND vd_feedbackLcd;
static HWND vd_op4LSLcd;
static HWND vd_op4DetLcd;
static HWND vd_egGraphLbl;
static HWND vd_egOrigin;
static HWND vd_egGraphCtrls[4];
static HWND vd_highlightedCtrls[4];
static int vd_egCtrlX[6];
static RECT vd_algQuickEditBtnRect;
static RECT vd_egQuickEditBtnRect;
static RECT vd_freqQuickEditBtnRect;
static RECT vd_outputQuickEditBtnRect;
static RECT vd_feedbackLcdRect;
static RECT vd_op4LSLcdRect;
static RECT vd_op4DetLcdRect;
static RECT vd_kvsLblRect;
static RECT vd_op4KvsLcdRect;
static AREA vd_egGraphLblArea;
static AREA vd_egOriginArea;
static AREA vd_egGraphCtrlAreas[4];
static AREA vd_baseCtrlArea;
static int vd_algorithm;
static int vd_simpleFreqValues[4];
static OPFREQCTRLS vd_freqCtrls[4] = {
    {
        { { IDC_OP1_FREQ },
        { IDC_OP1_RANGE },
        { IDC_OP1_CRS },
        { IDC_OP1_FINE },
        { IDC_OP1_SIMPLE_FREQ } },
        TX81Z_ACED_OP1_FIXED
    }, {
        { { IDC_OP2_FREQ },
        { IDC_OP2_RANGE },
        { IDC_OP2_CRS },
        { IDC_OP2_FINE },
        { IDC_OP2_SIMPLE_FREQ } },
        TX81Z_ACED_OP2_FIXED
    }, {
        { { IDC_OP3_FREQ },
        { IDC_OP3_RANGE },
        { IDC_OP3_CRS },
        { IDC_OP3_FINE },
        { IDC_OP3_SIMPLE_FREQ } },
        TX81Z_ACED_OP3_FIXED
    }, {
        { { IDC_OP4_FREQ },
        { IDC_OP4_RANGE },
        { IDC_OP4_CRS },
        { IDC_OP4_FINE },
        { IDC_OP4_SIMPLE_FREQ } },
        TX81Z_ACED_OP4_FIXED
    }
};


/*
 * Procedure definitions
 */

/*
 * Create - displays the dialog box
 */
BOOL VoiceDlg_Create(HWND parentWnd, AVCED *voice)
{
    /*
     * If the editor window already exists, just re-initialize it.
     */
    if (Prog_voiceDlg) {
        VoiceDlg_Update(Prog_voiceDlg, voice);
        if (IsIconic(Prog_voiceDlg)) {
            OpenIcon(Prog_voiceDlg);
        }
        BringWindowToTop(Prog_voiceDlg);
    } else {
        vd_parentWnd = parentWnd;
        vd_acedData = voice->aced.data;
        vd_vcedData = voice->vced.data;
        Prog_voiceDlg = CreateDialogParam(Prog_instance, (LPCTSTR) IDD_VOICEDLG
                , HWND_DESKTOP, vd_DlgProc, 0);
        if (!Prog_voiceDlg) {
            MsgBox_LastError(parentWnd);
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * FixedFreqSortCmp() - Comparison routine used for qsort in
 *                      InitSpecialLcdDisplayStrings().
 */
int VoiceDlg_FixedFreqSortCmp(const FIXEDFREQSORT *ffs1, const FIXEDFREQSORT *ffs2)
{
    return ffs1->freq - ffs2->freq;
}

/*
 * FixedParamsToFreq() - Converts fixed frequency slider values to a frequency.
 */
int VoiceDlg_FixedParamsToFreq(int range, int crs, int fine)
{
    return (((crs & 0x3C) << 2) + fine + (crs < 4 ? 8 : 0)) << range;
}

/*
 * Update() - Updates the contents of the dialog box.
 */
void VoiceDlg_Update(HWND voiceDlg, AVCED *voice)
{
    if (voice) {
        vd_acedData = voice->aced.data;
        vd_vcedData = voice->vced.data;
        vd_InitControlValues(voiceDlg);
    }
}

/*
 * AdjustFrigginEGControls()
 */
void vd_AdjustFrigginEGControls(HWND voiceDlg)
{
    AREA anchorArea;
    HWND ctrl;
    AREA movingCtrlArea;
    int i, op;

    /*
     * Get the OP1 AR control to use as a reference for the adjustments to all
     * the other EG controls.
     */
    ctrl = GetDlgItem(voiceDlg, vd_egCtrlIDs[0][0]);
    Window_GetParentRelativeArea(ctrl, voiceDlg, &anchorArea);
    /*
     * For each operator's set of EG controls.
     */
    for (op = 0; op < 4; op++) {
        /*
         * Six controls: D1R, D1L, D2R, RR, SHFT and the EG control.
         */
        for (i = 0; i < vd_egCtrlIDCnt - 1; i++) {
            /*
             * Skip over the EBS controls.
             */
            if (i == vd_egCtrlIDCnt - 2) {
                continue;
            }
            /*
             * The first time around, get the X coordinates for the six sliders
             * to use for drawing the EG labels.
             */
            if (op == 0) {
                vd_egCtrlX[i] = anchorArea.x;
            }
            /*
             * Get the next EG control to the right.
             */
            ctrl = GetDlgItem(voiceDlg
                    , vd_egCtrlIDs[i + (i == vd_egCtrlIDCnt - 3 ? 2 : 1)][op]);
            Window_GetParentRelativeArea(ctrl, voiceDlg, &movingCtrlArea);
            /*
             * Adjust the coordinates relative to the control to the left of it.
             */
            movingCtrlArea.x = AREA_R(anchorArea);
            movingCtrlArea.y = anchorArea.y;
            /*
             * Move it.
             */
            SetWindowPos(ctrl, NULL, PASS_AREA_FIELDS(movingCtrlArea)
                    , SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
            /*
             * Make this control's area the anchor area for the next control.
             */
            anchorArea = movingCtrlArea;
        }
        /*
         * First time around, move the EG origin scroll bar.
         */
        if (op == 0) {
            ctrl = GetDlgItem(voiceDlg, IDC_EG_ORIGIN);
            Window_GetParentRelativeArea(ctrl, voiceDlg, &movingCtrlArea);
            movingCtrlArea.x = anchorArea.x;
            movingCtrlArea.y = anchorArea.y - movingCtrlArea.h;
            SetWindowPos(ctrl, NULL, PASS_AREA_FIELDS(movingCtrlArea)
                    , SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
        }
        /*
         * For the first three ops, set the first anchor area for the next op.
         */
        if (op < 3) {
            ctrl = GetDlgItem(voiceDlg, vd_egCtrlIDs[0][op + 1]);
            Window_GetParentRelativeArea(ctrl, voiceDlg, &movingCtrlArea);
            movingCtrlArea.y = AREA_B(anchorArea) - 1;
            SetWindowPos(ctrl, NULL, PASS_AREA_FIELDS(movingCtrlArea)
                    , SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
            anchorArea = movingCtrlArea;
        }
    }
}

/*
 * ChangeAcedParameter()
 */
void vd_ChangeAcedParameter(HWND voiceDlg, int parameter, int value, BOOL send)
{
    int relativeParameter;
    int op = vd_CtrlIdToOpNumber(parameter + ACED_ID_FIRST);
    int i;
    int relativeDelta;

    assert(parameter >= 0 && parameter <= 22);
#ifdef NO_REDUNDANT_SENDS
    if (value == vd_acedData[parameter]) {
        return;
    }
#endif
    /*
     * Operator 1 EG shift cannot be changed.
     */
    if (parameter == TX81Z_ACED_OP1_SHIFT) {
        return;
    }
    /*
     * Add the change to the undo list.
     */
    if (!vd_undoFlag) {
        int oldValue = vd_acedData[parameter];

        Undo_AddChange(vd_undo, parameter + ACED_ID_FIRST, sizeof(int)
                , &oldValue, &value, vd_undoGroup);
        vd_undoGroup = FALSE;
        MenuItem_Enable(vd_menu, IDM_UNDO);
        MenuItem_Disable(vd_menu, IDM_REDO);
    }
    /*
     * Update the synth.
     */
    if (send) {
        TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_ACED, parameter, value);
    }
    /*
     * Find the delta value for relative follow.
     */
    if (!vd_absoluteFollow) {
        relativeDelta = value - vd_acedData[parameter];
    }
    /*
     * Update the snapshot data.
     */
    vd_acedData[parameter] = value;
    /*
     * Do operator follow editing.
     */
    if (op != -1 && !vd_opFollowRecursion && !vd_undoFlag) {
        vd_opFollowRecursion = TRUE;
        /*
         * Find the parameter number relative to the operator.
         */
        relativeParameter = parameter - vd_opAcedOffsets[op];
        /*
         * If opFollow is on for any operator, make the change to that operator.
         */
        for (i = 0; i < 4; i++) {
            if (vd_IsOpFollowing(i, op)) {
                int parameter = vd_opAcedOffsets[i] + relativeParameter;
                UINT ctrlID;

                if (relativeParameter == IDC_OP4_WAVE - ACED_ID_FIRST) {
                    ctrlID = parameter + ACED_ID_FIRST + WAVE_RPANEL_DIFF;
                } else {
                    ctrlID = parameter + ACED_ID_FIRST;
                }
                vd_ChangeControl(voiceDlg, ctrlID
                        , vd_absoluteFollow ? value
                            : vd_acedData[parameter] + relativeDelta
                        , send);
            }
        }
        vd_opFollowRecursion = FALSE;
    }
    /*
     * Update controls that depend on the changed parameter.
     */
    switch (parameter) {
        case TX81Z_ACED_OP1_FIXED:
        case TX81Z_ACED_OP2_FIXED:
        case TX81Z_ACED_OP3_FIXED:
        case TX81Z_ACED_OP4_FIXED:
            vd_UpdateFrequencyDisplay(voiceDlg, op);
            vd_UpdateFixedRatioStatus(voiceDlg, op);
            break;
        case TX81Z_ACED_OP1_RANGE:
        case TX81Z_ACED_OP2_RANGE:
        case TX81Z_ACED_OP3_RANGE:
        case TX81Z_ACED_OP4_RANGE:
        case TX81Z_ACED_OP1_FINE:
        case TX81Z_ACED_OP2_FINE:
        case TX81Z_ACED_OP3_FINE:
        case TX81Z_ACED_OP4_FINE:
            vd_UpdateFrequencyDisplay(voiceDlg, op);
            vd_UpdateSimpleFreqValue(voiceDlg, op, VoiceDlg_simpleFreqSelection);
            break;
        case TX81Z_ACED_OP1_SHIFT:
        case TX81Z_ACED_OP2_SHIFT:
        case TX81Z_ACED_OP3_SHIFT:
        case TX81Z_ACED_OP4_SHIFT:
            InvalidateRect(vd_egGraphCtrls[op], NULL, TRUE);
            break;
    }
    /*
     * Update the dirty status of the item in the snapshot list.
     */
    if (!vd_dirty) {
        SendNotification(Prog_mainWnd, IDD_VOICEDLG, voiceDlg, EDN_CHANGE);
        vd_dirty = TRUE;
    }
}

/*
 * ChangeControl() - Does automated editing by manipulating a control.
 */
void vd_ChangeControl(HWND voiceDlg, UINT ctrlID, int value, BOOL send)
{
    HWND ctrl = GetDlgItem(voiceDlg, ctrlID);
    _TUCHAR className[30];

    /*
     * Figure out what type of control ctrlID refers to.
     */
    GetClassName(ctrl, className, 30);
    if (StriEq(className, LcdCtrl_className)) {
        int min = LcdCtrl_GetRangeMin(ctrl);

        if (value < min) {
            value = min;
        } else {
            int max = LcdCtrl_GetRangeMax(ctrl);

            if (value >= max) {
                value = max;
            }
        }
        LcdCtrl_SetValue(ctrl, value);
        if (ctrlID == IDC_OP1_CRS || ctrlID == IDC_OP2_CRS
                || ctrlID == IDC_OP3_CRS || ctrlID == IDC_OP4_CRS)
        {
            vd_CheckFreqSliderRange(voiceDlg, vd_CtrlIdToOpNumber(ctrlID));
        }
    } else if (StriEq(className, RPanel_className)) {
        if (value < 0) {
            value = 0;
        } else {
            int max = RPanel_GetMax(ctrl);

            if (value > max) {
                value = max;
            }
        }
        RPanel_SetValue(ctrl, value);
        /*
         * Convert the ctrlID to the combo box.
         */
        ctrlID -= WAVE_RPANEL_DIFF;
        ComboBox_SetCurSel(GetDlgItem(voiceDlg, ctrlID), value);
    } else if (StriEq(className, Prog_comboBoxClassName)) {
        if (value < 0) {
            value = 0;
        } else {
            int max = ComboBox_GetCount(ctrl) - 1;

            if (value > max) {
                value = max;
            }
        }
        ComboBox_SetCurSel(ctrl, value);
        RPanel_SetValue(GetDlgItem(voiceDlg, ctrlID + WAVE_RPANEL_DIFF), value);
    } else if (StriEq(className, Prog_buttonClassName)) {
        if (value != Button_IsChecked(ctrl)) {
            Button_Click(ctrl);
        }
    }
    /*
     * Figure out the parameter number and send the parameter change message.
     */
    if (ctrlID >= ACED_ID_FIRST && ctrlID <= ACED_ID_LAST) {
        vd_ChangeAcedParameter(voiceDlg, ctrlID - ACED_ID_FIRST, value, send);
    } else if (ctrlID >= VCED_ID_FIRST && ctrlID <= VCED_ID_LAST) {
        vd_ChangeVcedParameter(voiceDlg, ctrlID - VCED_ID_FIRST, value, send);
    }
}

/*
 * ChangeVcedParameter()
 */
void vd_ChangeVcedParameter(HWND voiceDlg, int parameter, int value, BOOL send)
{
    int relativeParameter;
    int op = vd_CtrlIdToOpNumber(parameter + VCED_ID_FIRST);
    int i;
    int relativeDelta;

    assert(parameter >= 0 && parameter <= 93);
#ifdef NO_REDUNDANT_SENDS
    if (value == vd_vcedData[parameter]) {
        return;
    }
#endif
    /*
     * Add the change to the undo list.
     */
    if (!vd_undoFlag) {
        if (parameter < TX81Z_VCED_NAME) {
            int oldValue = vd_vcedData[parameter];

            Undo_AddChange(vd_undo, parameter + VCED_ID_FIRST, sizeof(int)
                    , &oldValue, &value, vd_undoGroup);
        } else {
            char newName[TX81Z_VCED_NAME];

            memcpy(newName, &vd_vcedData[TX81Z_VCED_NAME], TX81Z_VCED_NAME);
            newName[parameter - TX81Z_VCED_NAME] = value;
            Undo_AddChange(vd_undo, IDC_VOICE_NAME
                    , TX81Z_VCED_NAME
                    , &vd_vcedData[TX81Z_VCED_NAME], newName
                    , vd_undoGroup);
        }
        vd_undoGroup = FALSE;
        MenuItem_Enable(vd_menu, IDM_UNDO);
        MenuItem_Disable(vd_menu, IDM_REDO);
    }
    /*
     * Update the synth.
     */
    if (send) {
        TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_VCED, parameter, value);
    }
    if (!vd_absoluteFollow) {
        relativeDelta = value - vd_vcedData[parameter];
    }
    vd_vcedData[parameter] = value;
    /*
     * Do operator follow editing.
     */
    if (op != -1 && !vd_opFollowRecursion && !vd_undoFlag) {
        vd_opFollowRecursion = TRUE;
        /*
         * Find the parameter number relative to the operator.
         */
        relativeParameter = parameter - vd_opVcedOffsets[op];
        /*
         * If opFollow is on for any operator, make the change to that operator.
         */
        for (i = 0; i < 4; i++) {
            if (vd_IsOpFollowing(i, op)) {
                int parameter = vd_opVcedOffsets[i] + relativeParameter;

                vd_ChangeControl(voiceDlg, parameter + VCED_ID_FIRST
                        , vd_absoluteFollow ? value
                            : vd_vcedData[parameter] + relativeDelta
                        , send);
            }
        }
        vd_opFollowRecursion = FALSE;
    }
    /*
     * Update controls that depend on the changed parameter.
     */
    switch (parameter) {
        case TX81Z_VCED_OP1_CRS:
        case TX81Z_VCED_OP2_CRS:
        case TX81Z_VCED_OP3_CRS:
        case TX81Z_VCED_OP4_CRS:
            vd_UpdateFrequencyDisplay(voiceDlg, op);
            vd_UpdateSimpleFreqValue(voiceDlg, op, VoiceDlg_simpleFreqSelection);
            break;
        case TX81Z_VCED_OP1_AR:
        case TX81Z_VCED_OP1_D1R:
        case TX81Z_VCED_OP1_D1L:
        case TX81Z_VCED_OP1_D2R:
        case TX81Z_VCED_OP1_RR:
        case TX81Z_VCED_OP2_AR:
        case TX81Z_VCED_OP2_D1R:
        case TX81Z_VCED_OP2_D1L:
        case TX81Z_VCED_OP2_D2R:
        case TX81Z_VCED_OP2_RR:
        case TX81Z_VCED_OP3_AR:
        case TX81Z_VCED_OP3_D1R:
        case TX81Z_VCED_OP3_D1L:
        case TX81Z_VCED_OP3_D2R:
        case TX81Z_VCED_OP3_RR:
        case TX81Z_VCED_OP4_AR:
        case TX81Z_VCED_OP4_D1R:
        case TX81Z_VCED_OP4_D1L:
        case TX81Z_VCED_OP4_D2R:
        case TX81Z_VCED_OP4_RR:
            InvalidateRect(vd_egGraphCtrls[op], NULL, TRUE);
            break;
    }
    /*
     * Update the dirty status of the item in the snapshot list.
     */
    if (!vd_dirty) {
        SendNotification(Prog_mainWnd, IDD_VOICEDLG, voiceDlg, EDN_CHANGE);
        vd_dirty = TRUE;
    }
}

/*
 * CheckFreqSliderRange() - If the coarse frequency setting is less than 1.00,
 *                          then it's not possible to set the fine frequency
 *                          above 7, so this function changes the range on the
 *                          fine frequency slider to account for that.
 *                          Returns the new fine value if the change in range
 *                          change it, otherwise it returns -1.
 */
int vd_CheckFreqSliderRange(HWND voiceDlg, int op)
{
    int crsValue;
    int fineMax;
    HWND fineCtrl;

    /*
     * Get the relevant information from the controls.
     */
    crsValue = LcdCtrl_GetValue(vd_freqCtrls[op].ctrl[FCI_CRS].wnd);
    fineCtrl = vd_freqCtrls[op].ctrl[FCI_FINE].wnd;
    fineMax = LcdCtrl_GetRangeMax(fineCtrl);
    /*
     * Set the range of the fine frequency slider to match the
     * coarse slider.
     */
    if (crsValue < 4) {
        if (fineMax == 15) {
            if (LcdCtrl_SetRange(fineCtrl, 0, 7)) {
                return LcdCtrl_GetValue(fineCtrl);
            }
        }
    } else if (fineMax == 7) {
        LcdCtrl_SetRange(fineCtrl, 0, 15);
    }
    return -1;
}

/*
 * CtrlIdToOpNumber() - Returns the operator number that a control affects.
 */
int vd_CtrlIdToOpNumber(UINT ctrlID)
{
    switch (ctrlID) {
        case IDC_OP1_FREQ:
        case IDC_OP1_SIMPLE_FREQ:
        case IDC_OP1_ENABLE:
        case IDC_OP1_WAVE_RPANEL:
            return 0;
        case IDC_OP2_FREQ:
        case IDC_OP2_SIMPLE_FREQ:
        case IDC_OP2_ENABLE:
        case IDC_OP2_WAVE_RPANEL:
            return 1;
        case IDC_OP3_FREQ:
        case IDC_OP3_SIMPLE_FREQ:
        case IDC_OP3_ENABLE:
        case IDC_OP3_WAVE_RPANEL:
            return 2;
        case IDC_OP4_FREQ:
        case IDC_OP4_SIMPLE_FREQ:
        case IDC_OP4_ENABLE:
        case IDC_OP4_WAVE_RPANEL:
            return 3;
    }
    if ((ctrlID >= IDC_OP1_AR && ctrlID <= IDC_OP1_DET)
            || (ctrlID >= IDC_OP1_FIXED && ctrlID <= IDC_OP1_SHIFT))
    {
        return 0;
    }
    if ((ctrlID >= IDC_OP2_AR && ctrlID <= IDC_OP2_DET)
            || (ctrlID >= IDC_OP2_FIXED && ctrlID <= IDC_OP2_SHIFT))
    {
        return 1;
    }
    if ((ctrlID >= IDC_OP3_AR && ctrlID <= IDC_OP3_DET)
            || (ctrlID >= IDC_OP3_FIXED && ctrlID <= IDC_OP3_SHIFT))
    {
        return 2;
    }
    if ((ctrlID >= IDC_OP4_AR && ctrlID <= IDC_OP4_DET)
            || (ctrlID >= IDC_OP4_FIXED && ctrlID <= IDC_OP4_SHIFT))
    {
        return 3;
    }
    return -1;
}

/*
 * DlgProc
 */
BOOL CALLBACK vd_DlgProc(HWND voiceDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(voiceDlg, WM_ACTIVATE, vd_OnActivate);
        HANDLE_MSG(voiceDlg, WM_COMMAND, vd_OnCommand);
        HANDLE_MSG(voiceDlg, WM_DESTROY, vd_OnDestroy);
        HANDLE_MSG(voiceDlg, WM_DRAWITEM, vd_OnDrawItem);
        HANDLE_MSG(voiceDlg, WM_ERASEBKGND, vd_OnEraseBkgnd);
        HANDLE_MSG(voiceDlg, WM_INITDIALOG, vd_OnInitDialog);
        HANDLE_MSG(voiceDlg, WM_INITMENU, vd_OnInitMenu);
        HANDLE_MSG(voiceDlg, WM_KEYDOWN, vd_OnKey);
        HANDLE_MSG(voiceDlg, WM_MEASUREITEM, vd_OnMeasureItem);
        HANDLE_MSG(voiceDlg, WM_MOUSEMOVE, vd_OnMouseMove);
        HANDLE_MSG(voiceDlg, WM_MOUSEWHEEL, vd_OnMouseWheel);
        HANDLE_MSG(voiceDlg, WM_RBUTTONDBLCLK, vd_OnRButtonDown);
        HANDLE_MSG(voiceDlg, WM_RBUTTONDOWN, vd_OnRButtonDown);
        HANDLE_MSG(voiceDlg, WM_SIZE, vd_OnSize);
        HANDLE_MSG(vd_dialog, WM_VSCROLL, Dialog_OnVScroll);
        case WM_RBUTTONUP:
            return vd_OnRButtonUp(voiceDlg, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam));
        case WM_HSCROLL:
            if (lParam == (LPARAM) vd_egOrigin) {
                HANDLE_WM_HSCROLL(voiceDlg, wParam, lParam, vd_OnEGOriginScroll);
            } else {
                HANDLE_WM_HSCROLL(vd_dialog, wParam, lParam, Dialog_OnHScroll);
            }
            break;
        case EDM_REFRESH:
            vd_InitControlValues(voiceDlg);
            break;
        case EDM_SAVED:
            vd_dirty = FALSE;
            break;
        case KNN_FOCUSCHANGED:
            vd_undoGroup = TRUE;
            break;
    }
    return FALSE;
}

/*
 * DrawBkgnd() - Draws pseudo group boxes.
 */
void vd_DrawBkgnd(HDC dC)
{
    RECT edgeRect;
    int scrollX = vd_dialog->hScrollInfo.nPos;
    int scrollY = vd_dialog->vScrollInfo.nPos;
    int left, right, bottom;

    /*
     * Draw the Algorithm group box.
     */
    edgeRect.left = 0;
    edgeRect.top = Rect_VCenter(&vd_algQuickEditBtnRect);
    edgeRect.right = vd_feedbackLcdRect.right + 7;
    edgeRect.bottom = vd_feedbackLcdRect.bottom + 7;
    OffsetRect(&edgeRect, -scrollX, -scrollY);
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_RECT);
    /*
     * Draw the Output group polygon.
     */
    /* draw top, left and bottom of the tall box */
    edgeRect.left = -scrollX;
    edgeRect.top = Rect_VCenter(&vd_outputQuickEditBtnRect) - scrollY;
    edgeRect.right = vd_feedbackLcdRect.right + 7 - scrollX;
    edgeRect.bottom = vd_op4LSLcdRect.bottom + 7 - scrollY;
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_TOP | BF_LEFT | BF_BOTTOM);
    /* draw top right of the tall box */
    bottom = edgeRect.bottom; // save the bottom
    edgeRect.bottom = vd_kvsLblRect.top - 7 - scrollY; // top of short box
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_RIGHT);
    /* draw short box */
    edgeRect.top = edgeRect.bottom; // top of short box
    edgeRect.bottom = vd_op4KvsLcdRect.bottom + 9 - scrollY; // bottom of short box
    left = edgeRect.left; // save left side of tall box
    right = edgeRect.right; // save right side of tall box
    edgeRect.left = right - 2; // left side of short box
    edgeRect.right = vd_op4KvsLcdRect.right + 9 - scrollX; // right side of short box
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_TOP | BF_RIGHT | BF_BOTTOM);
    /* bottom right of the tall box */
    edgeRect.left = left;
    edgeRect.right = right;
    edgeRect.top = edgeRect.bottom - 1; // bottom of short box
    edgeRect.bottom = bottom - 1;
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_RIGHT);
    /*
     * Draw the Frequency group box.
     */
    edgeRect.left = vd_freqQuickEditBtnRect.left - 7;
    edgeRect.top = Rect_VCenter(&vd_freqQuickEditBtnRect);
    edgeRect.right = vd_op4DetLcdRect.right + 6;
    edgeRect.bottom = vd_op4DetLcdRect.bottom + 5;
    OffsetRect(&edgeRect, -scrollX, -scrollY);
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_RECT);
    /*
     * Draw the EG group box.
     */
    edgeRect.left = vd_egQuickEditBtnRect.left - 5;
    edgeRect.top = Rect_VCenter(&vd_egQuickEditBtnRect);
    edgeRect.right = AREA_R(vd_egGraphCtrlAreas[3]) + 5;
    edgeRect.bottom = AREA_B(vd_egGraphCtrlAreas[3]) + 6;
    OffsetRect(&edgeRect, -scrollX, -scrollY);
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_RECT);
    /*
     * Draw the EG control labels.
     */
#define DLGBASEX 3
#define DLGBASEY 2
    MiniFont_DrawString(dC, vd_egCtrlX[0] + 3 - scrollX, 33 - scrollY
            , Prog_arStr, 2, Prog_wndTextColor);
    MiniFont_DrawString(dC, vd_egCtrlX[1] - scrollX, 24 - scrollY, Prog_d1rStr
            , 3, Prog_wndTextColor);
    MiniFont_DrawString(dC, vd_egCtrlX[2] - scrollX, 33 - scrollY, Prog_d1lStr
            , 3, Prog_wndTextColor);
    MiniFont_DrawString(dC, vd_egCtrlX[3] - scrollX, 24 - scrollY, Prog_d2rStr
            , 3, Prog_wndTextColor);
    MiniFont_DrawString(dC, vd_egCtrlX[4] + 3 - scrollX, 33 - scrollY
            , Prog_rrStr, 2, Prog_wndTextColor);
    MiniFont_DrawString(dC, vd_egCtrlX[5] - 4 - scrollX, 24 - scrollY
            , Prog_shftStr, 4, Prog_wndTextColor);

}

/*
 * EraseStretchingEdges() - Cleans up pseudo group boxes when the window is
 *                          resized.
 */
void vd_EraseStretchingEdges(HDC dC)
{
    RECT edgeRect;
    int scrollX = vd_dialog->hScrollInfo.nPos;
    int scrollY = vd_dialog->vScrollInfo.nPos;

    /*
     * Erase the right edge of the EG group box.
     */
    edgeRect.top = Rect_VCenter(&vd_egQuickEditBtnRect) + 2;
    edgeRect.right = AREA_R(vd_egGraphCtrlAreas[3]) + 5;
    edgeRect.left = edgeRect.right - 3;
    edgeRect.bottom = AREA_B(vd_egGraphCtrlAreas[3]) + 6 - 2;
    OffsetRect(&edgeRect, -scrollX, -scrollY);
    FillRect(dC, &edgeRect, Prog_dlgBrush);
    /*
     * Redraw the little remnants of erasure.
     */
    edgeRect.top -= 2;
    edgeRect.bottom += 2;
    edgeRect.left -= 5;
    DrawEdge(dC, &edgeRect, EDGE_ETCHED, BF_TOP | BF_BOTTOM);
}

/*
 * FastWaveSelection() - Swaps out combo box and radio panel controls for
 *                       operator and LFO wave type selection.
 */
void vd_FastWaveSelection(HWND voiceDlg, BOOL fastOn)
{
    static const UINT comboIds[] = {
        IDC_OP1_WAVE,
        IDC_OP2_WAVE,
        IDC_OP3_WAVE,
        IDC_OP4_WAVE,
        IDC_LFO_WAVE
    };
#define comboIdCnt (sizeof comboIds / sizeof comboIds[0])
    static const UINT rPanelIds[] = {
        IDC_OP1_WAVE_RPANEL,
        IDC_OP2_WAVE_RPANEL,
        IDC_OP3_WAVE_RPANEL,
        IDC_OP4_WAVE_RPANEL,
        IDC_LFO_WAVE_RPANEL
    };
#define rPanelIdCnt (sizeof rPanelIds / sizeof rPanelIds[0])
    int i;
    HWND ctrl;
    HWND focusCtrl = GetFocus();
    int focusIdx = -1;

    /*
     * Update the values of and show/hide wave selection controls.
     */
    for (i = 0; i < comboIdCnt; i++) {
        ctrl = GetDlgItem(voiceDlg, comboIds[i]);
        if (ctrl == focusCtrl) {
            focusIdx = i;
        }
        ComboBox_SetCurSel(ctrl
                , (i < comboIdCnt - 1)
                    ? vd_acedData[comboIds[i] - ACED_ID_FIRST]
                    : vd_vcedData[TX81Z_VCED_LFO_WAVE]);
        ShowWindow(ctrl, !fastOn ? SW_SHOW : SW_HIDE);
    }
    for (i = 0; i < rPanelIdCnt; i++) {
        ctrl = GetDlgItem(voiceDlg, rPanelIds[i]);
        if (ctrl == focusCtrl) {
            focusIdx = i;
        }
        RPanel_SetValue(ctrl
                , (i < rPanelIdCnt - 1)
                    ? vd_acedData[rPanelIds[i] - WAVE_RPANEL_DIFF
                            - ACED_ID_FIRST]
                    : vd_vcedData[TX81Z_VCED_LFO_WAVE]);
        ShowWindow(ctrl, fastOn ? SW_SHOW : SW_HIDE);
    }
    if (focusIdx != -1) {
        if (fastOn) {
            SetFocus(GetDlgItem(voiceDlg, rPanelIds[focusIdx]));
        } else {
            SetFocus(GetDlgItem(voiceDlg, comboIds[focusIdx]));
        }
    }
}

/*
 * FocusCtrl() - Sets focus to a control, taking into account swapped controls.
 */
void vd_FocusCtrl(HWND voiceDlg, UINT ctrlID)
{
    switch (ctrlID) {
        case IDC_OP1_CRS:
        case IDC_OP2_CRS:
        case IDC_OP3_CRS:
        case IDC_OP4_CRS:
        case IDC_OP1_FINE:
        case IDC_OP2_FINE:
        case IDC_OP3_FINE:
        case IDC_OP4_FINE:
        case IDC_OP1_RANGE:
        case IDC_OP2_RANGE:
        case IDC_OP3_RANGE:
        case IDC_OP4_RANGE:
            if (VoiceDlg_simpleFreqSelection) {
                SetFocus(vd_freqCtrls[vd_CtrlIdToOpNumber(ctrlID)]
                        .ctrl[FCI_SIMPLE].wnd);
                return;
            }
            break;
        case IDC_LFO_WAVE:
        case IDC_OP1_WAVE:
        case IDC_OP2_WAVE:
        case IDC_OP3_WAVE:
        case IDC_OP4_WAVE:
            if (VoiceDlg_fastWaveSelection) {
                ctrlID += WAVE_RPANEL_DIFF;
            }
            break;
    }
    SetFocus(GetDlgItem(voiceDlg, ctrlID));
}

/*
 * InitControlValues()
 */
void vd_InitControlValues(HWND voiceDlg)
{
    _TUCHAR name[10];
    UINT ctrlID;
    HWND ctrl;
    int i, op;
    int value;

    FromAnsiNCopy(name, &vd_vcedData[TX81Z_VCED_NAME], 10);
    LcdCtrl_SetText(GetDlgItem(voiceDlg, IDC_VOICE_NAME), name);

    ctrl = GetDlgItem(voiceDlg, IDC_ALGORITHM);
    vd_algorithm = vd_vcedData[TX81Z_VCED_ALGORITHM];
    AlgCtrl_SetValue(ctrl, vd_algorithm);

    /*
     * Numeric LCD's
     */
    /* first double check the minimum on the RR's */
    if (vd_vcedData[TX81Z_VCED_OP4_RR] < 1)
        vd_vcedData[TX81Z_VCED_OP4_RR] = 1;
    if (vd_vcedData[TX81Z_VCED_OP3_RR] < 1)
        vd_vcedData[TX81Z_VCED_OP3_RR] = 1;
    if (vd_vcedData[TX81Z_VCED_OP2_RR] < 1)
        vd_vcedData[TX81Z_VCED_OP2_RR] = 1;
    if (vd_vcedData[TX81Z_VCED_OP1_RR] < 1)
        vd_vcedData[TX81Z_VCED_OP1_RR] = 1;

    for (i = 0; i < vd_numLcdInitCnt; i++) {
        ctrlID = vd_numLcdInits[i].ctrlID;
        LcdCtrl_SetValue(GetDlgItem(voiceDlg, ctrlID)
                , ((ctrlID < ACED_ID_FIRST)
                    ? vd_vcedData[ctrlID - VCED_ID_FIRST]
                    : vd_acedData[ctrlID - ACED_ID_FIRST]));
    }
    /*
     * Special LCD's
     */
    for (i = 0; i < vd_specialLcdInitCnt; i++) {
        ctrlID = vd_specialLcdInits[i].ctrlID;
        LcdCtrl_SetValue(GetDlgItem(voiceDlg, ctrlID)
                , ((ctrlID < ACED_ID_FIRST)
                    ? vd_vcedData[ctrlID - VCED_ID_FIRST]
                    : vd_acedData[ctrlID - ACED_ID_FIRST]));
    }
    /*
     * Disable OP1 shift.
     */
    EnableWindow(GetDlgItem(voiceDlg, IDC_OP1_SHIFT), FALSE);

    /*
     * Toggle buttons
     */
    for (i = 0; i < vd_toggleInitCnt; i++) {
        BOOL btnOn;

        /*
         * Get the ID of the next control.
         */
        ctrlID = vd_toggleInits[i].ctrlID;
        /*
         * Get the status of the button from the snapshot data.
         */
        if (ctrlID < ACED_ID_FIRST) {
            btnOn = vd_vcedData[ctrlID - VCED_ID_FIRST] != 0;
        } else {
            btnOn = vd_acedData[ctrlID - ACED_ID_FIRST] != 0;
        }
        /*
         * Set the checked status of the button.
         */
        ctrl = GetDlgItem(voiceDlg, ctrlID);
        Button_SetCheck(ctrl, btnOn);
        /*
         * Set the text of the button.  The poly mode/port mode buttons have
         * a weird relationship.  The port mode can show "Full Time" on the
         * display, yet it will be set the "Fingered" internally in the unit,
         * as evinced from getting a sysex dump - See OnCommand() for more.
         */
        if (vd_toggleInits[i].modeOn) {
            if (ctrlID == IDC_PORT_MODE
                    && vd_vcedData[TX81Z_VCED_POLY_MODE] == 0)
            {
                btnOn = FALSE;
            }
            Button_SetText(ctrl, btnOn ? vd_toggleInits[i].modeOn
                    : vd_toggleInits[i].modeOff);
        }
    }
    /*
     * Initialize the frequency controls.
     */
    for (op = 0; op < 4; op++) {
        vd_CheckFreqSliderRange(voiceDlg, op);
        vd_UpdateFixedRatioStatus(voiceDlg, op);
        vd_UpdateFrequencyDisplay(voiceDlg, op);
        vd_UpdateSimpleFreqValue(voiceDlg, op, VoiceDlg_simpleFreqSelection);
    }

    /*
     * LFO Wave controls
     */
    value = vd_vcedData[TX81Z_VCED_LFO_WAVE];
    ComboBox_SetCurSel(GetDlgItem(voiceDlg, IDC_LFO_WAVE), value);
    RPanel_SetValue(GetDlgItem(voiceDlg, IDC_LFO_WAVE_RPANEL), value);

    /*
     * Op wave combos
     */
    for (op = 0; op < 4; op++) {
        value = vd_acedData[TX81Z_ACED_OP4_WAVE + op * 5];
        ComboBox_SetCurSel(GetDlgItem(voiceDlg, IDC_OP4_WAVE + op * 5), value);
        RPanel_SetValue(GetDlgItem(voiceDlg, IDC_OP4_WAVE_RPANEL + op * 5)
                , value);
    }

    vd_opEnable = 0x0F;
    ctrl = GetDlgItem(voiceDlg, IDC_OP1_ENABLE);
    Button_SetCheck(ctrl, vd_opEnable & 0x08);
    ctrl = GetDlgItem(voiceDlg, IDC_OP2_ENABLE);
    Button_SetCheck(ctrl, vd_opEnable & 0x04);
    ctrl = GetDlgItem(voiceDlg, IDC_OP3_ENABLE);
    Button_SetCheck(ctrl, vd_opEnable & 0x02);
    ctrl = GetDlgItem(voiceDlg, IDC_OP4_ENABLE);
    Button_SetCheck(ctrl, vd_opEnable & 0x01);
    /* 
     * I'd like to send this message so the editor will be in sync with the
     * unit, but it lamely switches the LCD to the algorithm display.
     *
     * TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_VCED
     *         , TX81Z_VCED_OP_ENABLE, vd_opEnable);
     */

    /*
     * Initialize the EG controls and scrollbar.
     */
    ScrollBar_SetRange(vd_egOrigin, 0, 100);
    ScrollBar_SetPos(vd_egOrigin, 0, FALSE);
    for (op = 0; op < 4; op++) {
        vd_egLengths[op] = 0;
        EGCtrl_SetOrigin(vd_egGraphCtrls[op], 0);
    }
    /*
     * Check menu items.
     */
    if (VoiceDlg_fastWaveSelection) {
        MenuItem_Check(vd_menu, IDM_FAST_WAVE_SELECTION);
    }
    if (VoiceDlg_simpleFreqSelection) {
        MenuItem_Check(vd_menu, IDM_SIMPLE_FREQ_SELECTION);
    }
    if (VoiceDlg_hideEGLabels) {
        MenuItem_Check(vd_menu, IDM_HIDE_EG_LABELS);
    }
    MenuItem_RadioCheck(vd_menu, IDM_MOUSEWHEEL_OFF, IDM_MOUSEWHEEL_EDIT
            , IDM_MOUSEWHEEL_OFF + VoiceDlg_mouseWheel - MW_OFF);
    if (VoiceDlg_hideScrollBars) {
        MenuItem_Check(vd_menu, IDM_HIDE_SCROLLBARS);
    }
    InvalidateRect(voiceDlg, NULL, TRUE);
    vd_dirty = FALSE;
    Undo_Clear(vd_undo);
    vd_undoGroup = TRUE;
    MenuItem_Disable(vd_menu, IDM_UNDO);
    MenuItem_Disable(vd_menu, IDM_REDO);
}

/*
 * IsOpFollowing() - Returns true if operator a is following operator b.
 */
BOOL vd_IsOpFollowing(int a, int b)
{
    BOOL aIsModulator, bIsModulator;

    if (a == b) {
        return FALSE;
    }
    if (vd_opFollow & (1 << a)) {
        return TRUE;
    }
    aIsModulator = vd_algModulators[vd_algorithm][a];
    bIsModulator = vd_algModulators[vd_algorithm][b];
    if (vd_carrierFollow && !aIsModulator && !bIsModulator) {
        return TRUE;
    }
    if (vd_modulatorFollow && aIsModulator && bIsModulator) {
        return TRUE;
    }
    return FALSE;
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void vd_OnActivate(HWND voiceDlg, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = voiceDlg;
}

/*
 * OnCommand()
 */
void vd_OnCommand(HWND voiceDlg, unsigned ctrlID, HWND ctrl, UINT notify)
{
    int value = -1;
    unsigned i;

    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_EXIT:
            PostMessage(Prog_mainWnd, WM_COMMAND, IDM_EXIT, 0L);
        case IDM_CLOSE:
        case IDCANCEL:
            DestroyWindow(voiceDlg);
            return;
        case IDM_RETRIEVE_VOICE:
            TX81Z_RetrieveData(Prog_midi, REQ_AVCED, &Prog_snapshot);
            SendNotification(Prog_mainWnd, IDD_VOICEDLG, voiceDlg, EDN_CHANGE);
            vd_InitControlValues(voiceDlg);
            return;
        case IDM_TRANSMIT_VOICE:
            TX81Z_SendData(Prog_midi, REQ_AVCED, &Prog_snapshot);
            return;
        case IDM_STORE_VOICE: {
            int destIndex;

            if (StoreDlg_Create(voiceDlg, META_VCED, &destIndex)) {
                MainWnd_StoreItem(SI_VCED, destIndex);
            }
            return;
        }
        case IDM_INIT_VOICE:
            if (MessageBox(voiceDlg, _T("INIT VOICE: This cannot be undone.  Are you sure?")
                        , _T("Confirm Voice Init"), MB_YESNO | MB_ICONQUESTION)
                    == IDYES)
            {
                TX81Z_InitVoice(Prog_midi, &Prog_snapshot);
                SendNotification(Prog_mainWnd, IDD_VOICEDLG, voiceDlg, EDN_CHANGE);
                vd_InitControlValues(voiceDlg);
            }
            return;
        case IDM_ADD_TO_LIB_1:
        case IDM_ADD_TO_LIB_2:
        {
            int itemIdx = SI_VCED;
            int libIdx = ctrlID - IDM_ADD_TO_LIB_1;  /* assumes IDM_ADD_TO_LIB1
                                                       and IDM_ADD_TO_LIB2 are
                                                       adjacent */

            MainWnd_AddToLib(Prog_mainWnd, libIdx, &itemIdx, 1);
            return;
        }
        case IDM_UNDO:
            Undo_Undo(vd_undo, vd_Undo, voiceDlg);
            goto UpdateUndoMenus;
        case IDM_REDO:
            Undo_Redo(vd_undo, vd_Redo, voiceDlg);
UpdateUndoMenus:
            EnableMenuItem(vd_menu, IDM_UNDO
                    , MF_BYCOMMAND | (Undo_AnyUndoes(vd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(vd_menu, IDM_REDO
                    , MF_BYCOMMAND | (Undo_AnyRedoes(vd_undo)
                        ? MF_ENABLED : MF_GRAYED));
            return;
        case IDM_FIT_TO_CONTROLS:
            Dialog_FitToControls(vd_dialog);
            return;
        case IDM_KYBDDLG:
        case IDM_REMOTEWND:
        case IDM_MAINWND:
        case IDM_VOICEDLG:
        case IDM_PFMDLG:
        case IDM_FXDLG:
        case IDM_PCDLG:
        case IDM_MTODLG:
        case IDM_MTFDLG:
        case IDM_SYSDLG:
            SendMessage(Prog_mainWnd, WM_COMMAND, ctrlID, 0);
            return;
        case IDM_FAST_WAVE_SELECTION:
            MenuItem_Toggle(vd_menu, IDM_FAST_WAVE_SELECTION
                    , &VoiceDlg_fastWaveSelection);
            vd_FastWaveSelection(voiceDlg, VoiceDlg_fastWaveSelection);
            return;
        case IDM_SIMPLE_FREQ_SELECTION:
            MenuItem_Toggle(vd_menu, IDM_SIMPLE_FREQ_SELECTION
                    , &VoiceDlg_simpleFreqSelection);
            vd_SimpleFreqSelection(voiceDlg, VoiceDlg_simpleFreqSelection);
            return;
        case IDM_HIDE_EG_LABELS:
            MenuItem_Toggle(vd_menu, IDM_HIDE_EG_LABELS
                    , &VoiceDlg_hideEGLabels);
            for (i = 0; i < 4; i++) {
                EGCtrl_HideLabels(vd_egGraphCtrls[i], VoiceDlg_hideEGLabels);
            }
            return;
        case IDM_MOUSEWHEEL_OFF:
        case IDM_MOUSEWHEEL_SCROLL:
        case IDM_MOUSEWHEEL_EDIT:
            VoiceDlg_mouseWheel = ctrlID - IDM_MOUSEWHEEL_OFF;
            MenuItem_RadioCheck(vd_menu, IDM_MOUSEWHEEL_OFF
                    , IDM_MOUSEWHEEL_EDIT
                    , IDM_MOUSEWHEEL_OFF + VoiceDlg_mouseWheel - MW_OFF);
            return;
        case IDM_HIDE_SCROLLBARS:
            MenuItem_Toggle(vd_menu, IDM_HIDE_SCROLLBARS
                    , &VoiceDlg_hideScrollBars);
            vd_dialog->hideScrollBars = VoiceDlg_hideScrollBars;
            Dialog_UpdateScrollBars(vd_dialog);
            return;
        case IDM_HELP:
            Prog_OpenHelp(voiceDlg, _T("voice_editor.html"));
            return;
        case IDC_PANIC:
            /*
             * Send a poly mode change message to turn off all notes.
             */
            TX81Z_Panic(Prog_midi, &Prog_snapshot);
            return;
        /*
         * Voice editing commands
         */
        case IDC_VOICE_NAME:
            if (notify == LCN_EDITUPDATE) {
                i = LcdCtrl_GetCursorPos(ctrl);
                value = LcdCtrl_GetChar(ctrl, i);
                ctrlID += i;
            } else if (notify == LCN_EDITCHANGE) {
                SendNotification(Prog_mainWnd, IDD_VOICEDLG, voiceDlg
                        , EDN_CHANGE);
                return;
            }
            break;
        case IDC_ALGORITHM:
            value = vd_algorithm = AlgCtrl_GetValue(ctrl);
            break;
        case IDC_LFO_WAVE:
        case IDC_OP1_WAVE:
        case IDC_OP2_WAVE:
        case IDC_OP3_WAVE:
        case IDC_OP4_WAVE:
            if (notify == CBN_SELCHANGE) {
                value = ComboBox_GetCurSel(ctrl);
                break;
            } else if (notify == CBN_SETFOCUS || notify == CBN_KILLFOCUS) {
                break;
            }
            return;
        case IDC_LFO_WAVE_RPANEL:
        case IDC_OP1_WAVE_RPANEL:
        case IDC_OP2_WAVE_RPANEL:
        case IDC_OP3_WAVE_RPANEL:
        case IDC_OP4_WAVE_RPANEL:
            if (notify == RPN_CHANGE) {
                value = RPanel_GetValue(ctrl);
                ctrlID -= WAVE_RPANEL_DIFF;
            }
            break;
        case IDC_OP1_ENABLE:
        case IDC_OP2_ENABLE:
        case IDC_OP3_ENABLE:
        case IDC_OP4_ENABLE:
            if (Button_GetCheck(ctrl) == BST_CHECKED) {
                vd_opEnable |= 1 << (ctrlID - IDC_OP4_ENABLE);
            } else {
                vd_opEnable &= ~(1 << (ctrlID - IDC_OP4_ENABLE));
            }
            TX81Z_SendParamChange(Prog_midi, TX81Z_SUBGRP_VCED
                    , TX81Z_VCED_OP_ENABLE, vd_opEnable);
            return; /* return here because Op Enable doesn't exist in the
                       VCED format */
        case IDC_OP1_FIXED:
        case IDC_OP2_FIXED:
        case IDC_OP3_FIXED:
        case IDC_OP4_FIXED:
        case IDC_POLY_MODE:
        case IDC_PORT_MODE:
        {
            BOOL monoModeOn = Button_IsChecked(GetDlgItem(voiceDlg
                        , IDC_POLY_MODE));
            HWND portModeBtn = GetDlgItem(voiceDlg, IDC_PORT_MODE);

            /*
             * Find the toggleInit structure for the button.
             */
            for (i = 0; i < vd_toggleInitCnt; i++) {
                if (vd_toggleInits[i].ctrlID == ctrlID)
                    break;
            }
            /*
             * Find the checked status of the button.
             */
            value = Button_IsChecked(ctrl);
            /*
             * If poly mode is on, don't allow the port mode to be changed.
             */
            if (!monoModeOn && ctrlID == IDC_PORT_MODE) {
                Button_Toggle(ctrl);
                value = FALSE;
            }
            /*
             * If the port mode button is checked and the user changes the
             * poly mode.
             */
            if (ctrlID == IDC_POLY_MODE && Button_IsChecked(portModeBtn)) {
                /*
                 * Change the text on the port mode button without altering
                 * the checked status.
                 */
                Button_SetText(portModeBtn
                        , monoModeOn ? vd_fingered : vd_fullTime);
            }
            /*
             * Toggle the text on the button the user clicked.
             */
            Button_SetText(ctrl, value ? vd_toggleInits[i].modeOn
                    : vd_toggleInits[i].modeOff);
            break;
        }
        case IDC_LFO_SYNC:
        case IDC_OP1_AME:
        case IDC_OP2_AME:
        case IDC_OP3_AME:
        case IDC_OP4_AME:
            value = Button_IsChecked(ctrl);
            break;
        case IDC_OP1_RANGE:
        case IDC_OP1_CRS:
        case IDC_OP1_FINE:
        case IDC_OP2_RANGE:
        case IDC_OP2_CRS:
        case IDC_OP2_FINE:
        case IDC_OP3_RANGE:
        case IDC_OP3_CRS:
        case IDC_OP3_FINE:
        case IDC_OP4_RANGE:
        case IDC_OP4_CRS:
        case IDC_OP4_FINE:
            if (notify == LCN_SELCHANGE) {
                int op = vd_CtrlIdToOpNumber(ctrlID);
                int fineValue = vd_CheckFreqSliderRange(voiceDlg, op);

                if (fineValue != -1) {
                    vd_ChangeAcedParameter(voiceDlg
                            , vd_freqCtrls[op].ctrl[FCI_FINE].ctrlID
                                - ACED_ID_FIRST
                            , fineValue, TRUE);
                }
                value = LcdCtrl_GetValue(ctrl);
            }
            break;
        case IDC_OP1_EG:
        case IDC_OP2_EG:
        case IDC_OP3_EG:
        case IDC_OP4_EG:
            if (notify == ECN_LENGTHCHANGED) {
                int longest = 0;
                SCROLLINFO scrollInfo = { sizeof scrollInfo, SIF_ALL };
                int scrollMax;

                vd_egLengths[ctrlID - IDC_OP1_EG] = EGCtrl_GetLength(ctrl);
                for (i = 0; i < 4; i++) {
                    int len = vd_egLengths[i];
                    if (len > longest) {
                        longest = len;
                    }
                }
                GetScrollInfo(vd_egOrigin, SB_CTL, &scrollInfo);
                scrollInfo.nPage = vd_egOriginArea.w;
                scrollInfo.nMax = longest;
                scrollMax = scrollInfo.nMax - (signed) scrollInfo.nPage;
                if (scrollMax > 0 && scrollInfo.nPos > scrollMax) {
                    scrollInfo.nPos = scrollMax;
                    for (i = 0; i < 4; i++) {
                        EGCtrl_SetOrigin(vd_egGraphCtrls[i], scrollInfo.nPos);
                    }
                }
                SetScrollInfo(vd_egOrigin, SB_CTL, &scrollInfo, TRUE);
            } else if (notify == ECN_KEYUPCHANGED) {
                int keyUp = EGCtrl_GetKeyUp(ctrl);

                for (i = 0; i < 4; i++) {
                    if (vd_egGraphCtrls[i] != ctrl) {
                        EGCtrl_SetKeyUp(vd_egGraphCtrls[i], keyUp);
                    }
                }
            }
            return;
        case IDC_OP1_SIMPLE_FREQ:
        case IDC_OP2_SIMPLE_FREQ:
        case IDC_OP3_SIMPLE_FREQ:
        case IDC_OP4_SIMPLE_FREQ:
            if (notify == LCN_SELCHANGE) {
                vd_OnSimpleFreqChange(voiceDlg, ctrlID, LcdCtrl_GetValue(ctrl));
            }
            break;
        case IDC_OP1_FOLLOW:
        case IDC_OP2_FOLLOW:
        case IDC_OP3_FOLLOW:
        case IDC_OP4_FOLLOW:
            vd_opFollow ^= (1 << (ctrlID - IDC_OP1_FOLLOW));
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_CARRIER_FOLLOW));
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_MODULATOR_FOLLOW));
            vd_carrierFollow = vd_modulatorFollow = FALSE;
            return;
        case IDC_CARRIER_FOLLOW:
            vd_carrierFollow ^= TRUE;
            goto ClearOpFollows;
        case IDC_MODULATOR_FOLLOW:
            vd_modulatorFollow ^= TRUE;
ClearOpFollows:
            for (i = 0; i < 4; i++) {
                Button_Uncheck(GetDlgItem(voiceDlg, IDC_OP1_FOLLOW + i));
            }
            vd_opFollow = 0;
            return;
        case IDC_FOLLOW_ON:
            for (i = 0; i < 4; i++) {
                Button_Check(GetDlgItem(voiceDlg, IDC_OP1_FOLLOW + i));
            }
            vd_opFollow = 0x0F;
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_CARRIER_FOLLOW));
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_MODULATOR_FOLLOW));
            vd_carrierFollow = vd_modulatorFollow = FALSE;
            return;
        case IDC_RELATIVE_FOLLOW:
            vd_absoluteFollow ^= TRUE;
            Button_SetText(ctrl, vd_absoluteFollow ? vd_absolute : vd_relative);
            return;
        case IDC_FOLLOW_OFF:
            for (i = 0; i < 4; i++) {
                Button_Uncheck(GetDlgItem(voiceDlg, IDC_OP1_FOLLOW + i));
            }
            vd_opFollow = 0;
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_CARRIER_FOLLOW));
            Button_Uncheck(GetDlgItem(voiceDlg, IDC_MODULATOR_FOLLOW));
            ctrl = GetDlgItem(voiceDlg, IDC_RELATIVE_FOLLOW);
            Button_Uncheck(ctrl);
            Button_SetText(ctrl, vd_relative);
            vd_carrierFollow = vd_modulatorFollow = vd_absoluteFollow = FALSE;
            /*
             * Set the notify flag and fall through to unhighlight the LCDs.
             */
            notify = LCN_KILLFOCUS;
            break;
        case IDC_ALG_QUICKEDIT_BTN:
        case IDC_FREQ_QUICKEDIT_BTN:
        case IDC_EG_QUICKEDIT_BTN:
        case IDC_OUTPUT_QUICKEDIT_BTN:
            if (notify == MBN_PUSHED && vd_quickEdit != ctrlID) {
                vd_UpdateQuickEditMenu(ctrlID);
                vd_quickEdit = ctrlID;
            }
            return;
        case IDC_COPY_OP1_2:
        case IDC_COPY_OP1_3:
        case IDC_COPY_OP1_4:
        case IDC_COPY_OP2_1:
        case IDC_COPY_OP2_3:
        case IDC_COPY_OP2_4:
        case IDC_COPY_OP3_1:
        case IDC_COPY_OP3_2:
        case IDC_COPY_OP3_4:
        case IDC_COPY_OP4_1:
        case IDC_COPY_OP4_2:
        case IDC_COPY_OP4_3:
            vd_QuickEditCopy(voiceDlg, ctrlID - IDC_COPY_OP1_2, vd_quickEdit);
            return;
        case IDC_SWAP_OP1_2:
        case IDC_SWAP_OP1_3:
        case IDC_SWAP_OP1_4:
        case IDC_SWAP_OP2_3:
        case IDC_SWAP_OP2_4:
        case IDC_SWAP_OP3_4:
            vd_QuickEditSwap(voiceDlg, ctrlID - IDC_SWAP_OP1_2, vd_quickEdit);
            return;
        case IDC_RAND_OP1:
        case IDC_RAND_OP2:
        case IDC_RAND_OP3:
        case IDC_RAND_OP4:
        case IDC_RAND_ALL:
            vd_QuickEditRand(voiceDlg, ctrlID - IDC_RAND_OP1, vd_quickEdit);
            return;
        case 0x0000FFFF: /* IDC_STATIC */
            return;
        default:
            if (notify == LCN_SELCHANGE) {
                value = LcdCtrl_GetValue(ctrl);
            }
            break;
    }
    /* RPN_SETFOCUS is the same as LCN_SETFOCUS so I'm not checking it here */
    if ((notify == LCN_SETFOCUS || notify == CBN_SETFOCUS)
            && (vd_opFollow || vd_carrierFollow || vd_modulatorFollow))
    {
        /*
         * Find the following LCDs and paint them red.
         */
        int i;
        int op = vd_CtrlIdToOpNumber(ctrlID);
        int relativeParameter;
        const int *offsets = NULL;

        if (op == -1) {
            return;
        }
        if (ctrlID <= VCED_ID_LAST) {
            relativeParameter = ctrlID - vd_opVcedOffsets[op];
            offsets = vd_opVcedOffsets;
        } else if (ctrlID <= ACED_ID_LAST) {
            relativeParameter = ctrlID - vd_opAcedOffsets[op];
            offsets = vd_opAcedOffsets;
        } else if (ctrlID >= IDC_OP4_SIMPLE_FREQ
                && ctrlID <= IDC_OP1_SIMPLE_FREQ)
        {
            relativeParameter = 0;
            offsets = vd_opSimpleOffsets;
        } else if (ctrlID >= IDC_OP4_WAVE_RPANEL
                && ctrlID <= IDC_OP1_WAVE_RPANEL)
        {
            relativeParameter = 0;
            offsets = vd_opRPanelOffsets;
        }
        for (i = 0; i < 4; i++) {
            if (vd_IsOpFollowing(i, op)) {
                HWND followingCtrl = GetDlgItem(voiceDlg
                        , offsets[i] + relativeParameter);
                _TUCHAR className[30];

                GetClassName(ctrl, className, 30);
                if (StriEq(className, LcdCtrl_className)) {
                    LcdCtrl_Highlight(followingCtrl, TRUE);
                    goto SetHighlightCtrl;
                } else if (StriEq(className, RPanel_className)) {
                    RPanel_Highlight(followingCtrl, TRUE);
                    goto SetHighlightCtrl;
                } else if (StriEq(className, Prog_comboBoxClassName)) {
                    SetWindowLong(followingCtrl, GWL_USERDATA, TRUE);
                    InvalidateRect(followingCtrl, NULL, TRUE);
SetHighlightCtrl:
                    vd_highlightedCtrls[i] = followingCtrl;
                }
            }
        }
        return;
    /* RPN_KILLFOCUS is the same as LCN_KILLFOCUS so I'm not checking it here */
    } else if ((notify == LCN_KILLFOCUS || notify == CBN_KILLFOCUS)
            && (vd_opFollow || vd_carrierFollow || vd_modulatorFollow))
    {
        /*
         * Unhighlight the following LCDs.
         */
        for (i = 0; i < 4; i++) {
            HWND followingCtrl = vd_highlightedCtrls[i];

            if (followingCtrl) {
                _TUCHAR className[30];

                GetClassName(ctrl, className, 30);
                if (StriEq(className, LcdCtrl_className)) {
                    LcdCtrl_Highlight(followingCtrl, FALSE);
                    goto DehighlightCtrl;
                } else if (StriEq(className, RPanel_className)) {
                    RPanel_Highlight(followingCtrl, FALSE);
                    goto DehighlightCtrl;
                } else if (StriEq(className, Prog_comboBoxClassName)) {
                    SetWindowLong(followingCtrl, GWL_USERDATA, FALSE);
                    InvalidateRect(followingCtrl, NULL, TRUE);
DehighlightCtrl:
                    vd_highlightedCtrls[i] = NULL;
                }
            }
        }
        return;
    }
    if (value == -1) {
        return;
    }
    assert((ctrlID >= VCED_ID_FIRST && ctrlID <= VCED_ID_LAST)
                || (ctrlID >= ACED_ID_FIRST && ctrlID <= ACED_ID_LAST));
    /*
     * Transmit the parameter change.
     */
    if (ctrlID < ACED_ID_FIRST) {
        vd_ChangeVcedParameter(voiceDlg, ctrlID - VCED_ID_FIRST, value, TRUE);
    } else {
        vd_ChangeAcedParameter(voiceDlg, ctrlID - ACED_ID_FIRST, value, TRUE);
    }
}

/*
 * OnDestroy()
 */
void vd_OnDestroy(HWND voiceDlg)
{
    Dialog_Destroy(vd_dialog);
    vd_dialog = NULL;
    Undo_Destroy(vd_undo);
    GetWindowPlacement(voiceDlg, &Prog_voiceDlgPlacement);
    Prog_voiceDlg = NULL;
    if (Prog_mainWndToFront) {
        SetForegroundWindow(Prog_mainWnd);
    }
}

/*
 * OnDrawItem() - Draws the wave combo boxes and radio panel controls.
 */
void vd_OnDrawItem(HWND voiceDlg, const DRAWITEMSTRUCT *drawItem)
{
    UINT ctrlType = drawItem->CtlType;
    UINT ctrlID = drawItem->CtlID;
    HDC dC = drawItem->hDC;

    if (ctrlType == ODT_BUTTON && ctrlID >= FIRST_VOICE_QUICKEDIT_BTN
            && ctrlID <= LAST_VOICE_QUICKEDIT_BTN)
    {
        MenuBtn_DrawButton(voiceDlg, drawItem);
        MenuBtn_DrawArrow(voiceDlg, drawItem);
    } else if (ctrlType == ODT_MENU) {
        COLORREF oldTextColor = SetTextColor(dC, Prog_wndTextColor);
        RECT itemRect = drawItem->rcItem;
        HFONT oldFont;
        
        ctrlID = drawItem->itemID;
        itemRect.left += GetSystemMetrics(SM_CXMENUCHECK)
                + GetSystemMetrics(SM_CXDLGFRAME);
        oldFont = SelectFont(dC, Prog_tahomaBoldFont);
        DrawText(dC, vd_optionMenuHeaders[ctrlID - IDM_FREQ_OPTIONS], -1
                , &itemRect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
        SetTextColor(dC, oldTextColor);
        SelectFont(dC, oldFont);
    } else {
        HBRUSH brush, waveBrush;
        HPEN pen, oldPen;
        const RECT *rect = &drawItem->rcItem;
        int item = drawItem->itemID;
        int state = drawItem->itemState;
        int left = ctrlType == ODT_COMBOBOX ? rect->left + 5 : rect->left + 3;
        int top = ctrlType == ODT_COMBOBOX ? rect->top + 3 : rect->top + 1;
        int right = ctrlType == ODT_COMBOBOX ? rect->left + 40 : rect->right - 4;
        int bottom = ctrlType == ODT_COMBOBOX ? rect->bottom - 3 : rect->bottom - 2;
        int center = left + ((right - left) >> 1);
        int middle = top + ((bottom - top) >> 1);
        COLORREF textColor, oldTextColor;
        COLORREF bkColor, oldBkColor;

        if (ctrlType == ODT_COMBOBOX
                && GetWindowLong(drawItem->hwndItem, GWL_USERDATA))
        {
            state |= ODS_SELECTED | ODS_HIGHLIGHT;
        }
        if ((state & ODS_FOCUS) || (state & ODS_SELECTED)) {
            textColor = Prog_wndColor;
            bkColor = Prog_wndTextColor;
            pen = Prog_wndPen;
            waveBrush = Prog_wndBrush;
            if (state & ODS_HIGHLIGHT) {
                brush = Prog_hiBrush;
            } else {
                brush = Prog_wndTextBrush;
            }
        } else {
            textColor = Prog_wndTextColor;
            pen = Prog_wndTextPen;
            waveBrush = Prog_wndTextBrush;
            bkColor = Prog_wndColor;
            brush = Prog_wndBrush;
        }
        oldTextColor = SetTextColor(dC, textColor);
        oldBkColor = SetBkColor(dC, bkColor);
        oldPen = SelectPen(dC, pen);
        FillRect(dC, rect, brush);

        if (ctrlID == IDC_LFO_WAVE) {
            if (item == 0) {
                POINT verts[] = {
                    { left, bottom }, { right, top }, { right, bottom }
                };
                Polyline(dC, verts, 3);
            } else if (item == 1) {
                POINT verts[] = {
                    { left, middle }, { left, top }, { center, top },
                        { center, bottom }, { right, bottom }, { right, middle }
                };
                Polyline(dC, verts, 6);
            } else if (item == 2) {
                POINT verts[] = {
                    { left, bottom }, { center, top }, { right, bottom }
                };
                Polyline(dC, verts, 3);
            } else if (item == 3) {
                POINT verts[] = {
                    { left, middle }, { left, bottom }, { left + 8, bottom },
                    { left + 8, top + 2 }, { left + 16, top + 2 },
                    { left + 16, bottom - 3 }, { left + 24, bottom - 3 },
                    { left + 24, middle }, { left + 32, middle },
                    { left + 32, bottom - 2 }, { left + 40, bottom - 2 },
                    { left + 40, top }
                };
                Polyline(dC, verts, 12);
            };
        } else if (ctrlID == IDC_LFO_WAVE_RPANEL) {
            if (item == 0) {
                POINT verts[] = {
                    { left, bottom }, { right, top }, {right, bottom }
                };
                Polyline(dC, verts, 3);
            } else if (item == 1) {
                POINT verts[] = {
                    { left, middle }, { left, top }, { center, top },
                        { center, bottom }, { right, bottom }, { right, middle }
                };
                Polyline(dC, verts, 6);
            } else if (item == 2) {
                POINT verts[] = {
                    { left, bottom }, { center, top }, { right, bottom }
                };
                Polyline(dC, verts, 3);
            } else if (item == 3) {
                POINT verts[] = {
                    { ++left, middle }, { left, bottom }, { left + 5, bottom },
                    { left + 5, top + 1 }, { left + 10, top + 1 },
                    { left + 10, bottom - 2 }, { left + 15, bottom - 2 },
                    { left + 15, middle }, { left + 20, middle },
                    { left + 20, bottom - 1 }, { left + 25, bottom - 1 },
                    { left + 25, top }
                };
                Polyline(dC, verts, 12);
            }
        }
        else if (ctrlID == IDC_OP1_WAVE || ctrlID == IDC_OP2_WAVE
                || ctrlID == IDC_OP3_WAVE || ctrlID == IDC_OP4_WAVE)
        {
            const _TUCHAR *waveStr = &vd_opWaveInits[item][2];

            MiniFont_DrawString(dC, rect->left + 2, rect->top + 6
                    , vd_opWaveInits[item], 2, textColor);
            MiniFont_DrawString2X(dC, rect->left + 16, rect->top + 2, waveStr, 2
                    , waveBrush);
        }
        else if (ctrlID == IDC_OP1_WAVE_RPANEL || ctrlID == IDC_OP2_WAVE_RPANEL
                || ctrlID == IDC_OP3_WAVE_RPANEL || ctrlID == IDC_OP4_WAVE_RPANEL)
        {
            const _TUCHAR *waveStr = &vd_opWaveInits[item][2];

            MiniFont_DrawString(dC, rect->left + 2, rect->top + 1
                    , waveStr, 2, textColor);
        }
        SelectPen(dC, oldPen);
        SetTextColor(dC, oldTextColor);
        SetBkColor(dC, oldBkColor);
    }
}

/*
 * OnEGOriginScroll()
 */
void vd_OnEGOriginScroll(HWND voiceDlg, HWND ctrl, UINT code, int pos)
{
    SCROLLINFO scrollInfo = { sizeof scrollInfo, SIF_ALL };
    int origPos, scrollMax;
	int i;

    GetScrollInfo(ctrl, SB_CTL, &scrollInfo);
    origPos = scrollInfo.nPos;
    scrollMax = scrollInfo.nMax - scrollInfo.nPage + 1;

    switch (code) {
        case SB_ENDSCROLL:
            break;
		case SB_LEFT:
            scrollInfo.nPos = 0;
            break;
		case SB_RIGHT:
            scrollInfo.nPos = scrollMax;
            break;
		case SB_LINELEFT:
            scrollInfo.nPos -= 20;
            break;
		case SB_LINERIGHT:
            scrollInfo.nPos += 20;
            break;
		case SB_PAGELEFT:
            scrollInfo.nPos -= scrollInfo.nPage;
            break;
		case SB_PAGERIGHT:
            scrollInfo.nPos += scrollInfo.nPage;
            break;
        case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
            scrollInfo.nPos = pos;
            break;
    }
    if (scrollInfo.nPos < 0)
        scrollInfo.nPos = 0;
    if (scrollInfo.nPos > scrollMax)
        scrollInfo.nPos = scrollMax;
    SetScrollInfo(ctrl, SB_CTL, &scrollInfo, TRUE);
    for (i = 0; i < 4; i++) {
        EGCtrl_SetOrigin(vd_egGraphCtrls[i], scrollInfo.nPos);
    }
}

/*
 * OnEraseBkgnd() - Fills the background and draws the group box edges.
 */
BOOL vd_OnEraseBkgnd(HWND voiceDlg, HDC dC)
{
    RECT rect;

    GetClientRect(voiceDlg, &rect);
    FillRect(dC, &rect, Prog_dlgBrush);

    vd_DrawBkgnd(dC);

    return TRUE;
}

/*
 * OnInitDialog - Does one-time initializations for the voice editor.
 */
BOOL vd_OnInitDialog(HWND voiceDlg, HWND focusCtrl, LPARAM lParam)
{
    HWND ctrl;
    int i, j;
    BOOL newPosition = FALSE;
    MENUITEMINFO menuItemInfo;
    HDC dC;

    Prog_voiceDlg = voiceDlg;
    /*
     * Set up the menus.
     */
    vd_menu = GetMenu(voiceDlg);
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_OWNERDRAW;
    SetMenuItemInfo(vd_menu, IDM_FREQ_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(vd_menu, IDM_EG_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(vd_menu, IDM_MOUSE_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(vd_menu, IDM_WINDOW_OPTIONS, FALSE, &menuItemInfo);
    dC = GetDC(voiceDlg);
    for (i = 0; i < vd_optionMenuHeaderCnt; i++) {
        SIZE size;

        GetTextExtentPoint32(dC, vd_optionMenuHeaders[i], 19, (SIZE *) &size);
        if (size.cx > vd_optionMenuHeaderWidth) {
            vd_optionMenuHeaderWidth = size.cx; 
        }
    }
    ReleaseDC(voiceDlg, dC);
    /*
     * Init menu buttons.
     */
    for (i = FIRST_VOICE_QUICKEDIT_BTN; i <= LAST_VOICE_QUICKEDIT_BTN; i++) {
        ctrl = GetDlgItem(voiceDlg, i);
        MenuBtn_Init(ctrl, Prog_voiceQuickEditMenu, NULL, Prog_menuFlags);
    }
    /*
     * Set up some arrays.
     */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < FCI_CNT; j++) {
            vd_freqCtrls[i].ctrl[j].wnd
                = GetDlgItem(voiceDlg, vd_freqCtrls[i].ctrl[j].ctrlID);
        }
    }
    /*
     * Adjust the window position if the position has never been saved.
     */
    if (IsRectEmpty(&Prog_voiceDlgPlacement.rcNormalPosition)) {
        Window_Center(voiceDlg, vd_parentWnd);
        newPosition = TRUE;
    }
    /*
     * Adjust the friggin' EG controls.
     */
    vd_AdjustFrigginEGControls(voiceDlg);
    /*
     * Cache control handles and areas for calculating the positions of the
     * pseudo group boxes.
     */
    vd_algQuickEditBtn = GetDlgItem(voiceDlg, IDC_ALG_QUICKEDIT_BTN);
    vd_egQuickEditBtn = GetDlgItem(voiceDlg, IDC_EG_QUICKEDIT_BTN);
    vd_freqQuickEditBtn = GetDlgItem(voiceDlg, IDC_FREQ_QUICKEDIT_BTN);
    vd_outputQuickEditBtn = GetDlgItem(voiceDlg, IDC_OUTPUT_QUICKEDIT_BTN);
    vd_feedbackLcd = GetDlgItem(voiceDlg, IDC_FEEDBACK);
    vd_op4LSLcd = GetDlgItem(voiceDlg, IDC_OP4_LS);
    vd_op4DetLcd = GetDlgItem(voiceDlg, IDC_OP4_DET);
    vd_egGraphLbl = GetDlgItem(voiceDlg, IDC_EG_GRAPH_LBL);
    vd_egOrigin = GetDlgItem(voiceDlg, IDC_EG_ORIGIN);
    for (i = 0; i < 4; i++) {
        vd_egGraphCtrls[i] = GetDlgItem(voiceDlg
                , vd_egCtrlIDs[vd_egCtrlIDCnt - 1][i]);
    }
    Window_GetParentRelativeRect(vd_algQuickEditBtn, voiceDlg
            , &vd_algQuickEditBtnRect);
    Window_GetParentRelativeRect(vd_egQuickEditBtn, voiceDlg
            , &vd_egQuickEditBtnRect);
    Window_GetParentRelativeRect(vd_freqQuickEditBtn, voiceDlg
            , &vd_freqQuickEditBtnRect);
    Window_GetParentRelativeRect(vd_outputQuickEditBtn, voiceDlg
            , &vd_outputQuickEditBtnRect);
    Window_GetParentRelativeRect(vd_feedbackLcd, voiceDlg, &vd_feedbackLcdRect);
    Window_GetParentRelativeRect(vd_op4LSLcd, voiceDlg, &vd_op4LSLcdRect);
    Window_GetParentRelativeRect(vd_op4DetLcd, voiceDlg, &vd_op4DetLcdRect);
    Window_GetParentRelativeRect(GetDlgItem(voiceDlg, IDC_KVS_LBL), voiceDlg
            , &vd_kvsLblRect);
    Window_GetParentRelativeRect(GetDlgItem(voiceDlg, IDC_OP4_KVS), voiceDlg
            , &vd_op4KvsLcdRect);
    Window_GetParentRelativeArea(vd_egGraphLbl, voiceDlg, &vd_egGraphLblArea);
    Window_GetParentRelativeArea(vd_egOrigin, voiceDlg, &vd_egOriginArea);
    for (i = 0; i < 4; i++) {
        Window_GetParentRelativeArea(vd_egGraphCtrls[i], voiceDlg
                , &vd_egGraphCtrlAreas[i]);
    }
    /*
     * Bold up the group box labels.
     */
    EnumChildWindows(voiceDlg, Prog_SetBoldFont, 0);
    /*
     * LFO Wave combo
     */
    ctrl = GetDlgItem(voiceDlg, IDC_LFO_WAVE);
    ComboBox_AddString(ctrl, _T("Saw Up"));
    ComboBox_AddString(ctrl, _T("Square"));
    ComboBox_AddString(ctrl, _T("Triangle"));
    ComboBox_AddString(ctrl, _T("S/Hold"));
    /*
     * LFO Wave radio panel
     */
    ctrl = GetDlgItem(voiceDlg, IDC_LFO_WAVE_RPANEL);
    RPanel_SetGeometry(ctrl, 2, 2);
    /*
     * Op Wave combos and radio panels
     */
    for (i = 0; i < 4; i++) {
        ctrl = GetDlgItem(voiceDlg, IDC_OP4_WAVE + i * 5);
        for (j = 0; j < 8; j++) {
            _TUCHAR str[3] = { vd_opWaveInits[j][0], vd_opWaveInits[j][1], '\0' };
            ComboBox_AddString(ctrl, str);
        }
        ctrl = GetDlgItem(voiceDlg, IDC_OP4_WAVE_RPANEL + i * 5);
        RPanel_SetGeometry(ctrl, 2, 4);
    }
    /*
     * EG Graph controls
     */
    for (i = 0; i < 4; i++) {
        /*
         * Give each EG control a pointer to the beginning of the VCED EG
         * parameters for its operator and a pointer to the beginning of the
         * ACED EG parameters for its operator.
         */
        EGCtrl_SetPtrs(vd_egGraphCtrls[i]
                , &vd_vcedData[vd_egCtrlIDs[0][i] - VCED_ID_FIRST]
                , &vd_acedData[vd_egCtrlIDs[5][i] - ACED_ID_FIRST]);
        /*
         * Hide the EG labels if that option is set.
         */
        EGCtrl_HideLabels(vd_egGraphCtrls[i], VoiceDlg_hideEGLabels);
    }
    /*
     * Voice name LCD
     */
    LcdCtrl_TextInit(GetDlgItem(voiceDlg, IDC_VOICE_NAME), TX81Z_NAME_LEN);
    /*
     * Numeric LCDs
     */
    for (i = 0; i < vd_numLcdInitCnt; i++) {
        LcdCtrl_NumInit(
                GetDlgItem(voiceDlg, vd_numLcdInits[i].ctrlID)
                , &vd_numLcdInits[i]
            );
    }
    /*
     * Special LCDs
     */
    for (i = 0; i < vd_specialLcdInitCnt; i++) {
        LcdCtrl_SpecialInit(
                GetDlgItem(voiceDlg, vd_specialLcdInits[i].ctrlID)
                , &vd_specialLcdInits[i]
            );
    }
    /*
     * Simple LCDs
     */
    for (i = 0; i < 4; i++) {
        LcdCtrl_SpecialInit(vd_freqCtrls[i].ctrl[FCI_SIMPLE].wnd
                , &vd_simpleRatioLcdInits[i]);
    }
    /*
     * Subclass controls for keyboard navigation and middle button scrolling.
     */
    EnumChildWindows(voiceDlg, vd_SubclassControls, 0);
    /*
     * Subclass the EG origin scroll bar so it can't gain focus.
     */
    vd_origOriginWndProc = SubclassWindow(vd_egOrigin, vd_OriginWndProc);
    /*
     * Turn on the simple frequency option.
     */
    vd_SimpleFreqSelection(voiceDlg, VoiceDlg_simpleFreqSelection);
    /*
     * Set up the undo infrastructure (must be done before calling
     * InitControlValues).
     */
    vd_undo = Undo_Create();
    /*
     * Initialize control settings.
     */
    vd_InitControlValues(voiceDlg);

    /*
     * Turn on/off one click wave selection.
     */
    vd_FastWaveSelection(voiceDlg, VoiceDlg_fastWaveSelection);

    if (!(vd_dialog = Dialog_Create(voiceDlg)))
        return FALSE;
    vd_dialog->ctrlAreaSize.cx += 7; /* leave room for the EG group box */
    vd_dialog->hideScrollBars = VoiceDlg_hideScrollBars;
    if (VoiceDlg_hideScrollBars) {
        DWORD style = GetWindowLong(voiceDlg, GWL_STYLE);
        style &= ~(WS_HSCROLL | WS_VSCROLL);
        SetWindowLong(voiceDlg, GWL_STYLE, style);
        MenuItem_Check(vd_menu, IDM_HIDE_SCROLLBARS);
    }
    if (newPosition) {
        Dialog_FitToControls(vd_dialog);
        GetWindowPlacement(voiceDlg, &Prog_voiceDlgPlacement);
    }
    vd_baseCtrlArea.w = vd_dialog->ctrlAreaSize.cx;
    vd_baseCtrlArea.h = vd_dialog->ctrlAreaSize.cy;
    /*
     * Reset follow functions.
     */
    vd_opFollow = vd_carrierFollow = vd_modulatorFollow = vd_absoluteFollow
            = FALSE;
    for (i = 0; i < 4; i++) {
        vd_highlightedCtrls[i] = NULL;
    }
    /*
     * Set the saved window position.  Moving the window twice here is a hack
     * to fake out the dialog creation routine so the scrollbars appear in
     * the correct state.  It doesn't pay attention to changes in the scroll
     * bar styles in WM_INITDIALOG apparently, and I can't get anything else
     * to work.
     */
    Dialog_UpdateScrollBars(vd_dialog);
#define RC Prog_voiceDlgPlacement.rcNormalPosition
    MoveWindow(voiceDlg, RC.left, RC.top
            , RECT_W(RC) + GetSystemMetrics(SM_CXHSCROLL)
            , RECT_H(RC) + GetSystemMetrics(SM_CYVSCROLL), TRUE);
    MoveWindow(voiceDlg, RC.left, RC.top, RECT_W(RC), RECT_H(RC), TRUE);
//    SetWindowPos(voiceDlg, HWND_TOP, RC.left, RC.top, RECT_W(RC), RECT_H(RC)
//            , SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    
#undef RC
    ShowWindow(voiceDlg, SW_SHOWNORMAL);

    return TRUE;
}

/*
 * OnInitMenu() - Sets the file names for the "Add To Library n" menu items.
 */
void vd_OnInitMenu(HWND voiceDlg, HMENU menu)
{
    HMENU editorMenu = GetSubMenu(menu, 0);
    MENUITEMINFO menuItemInfo;
#define ITEMSTRLEN (_MAX_PATH + 40)
    _TUCHAR itemStr[ITEMSTRLEN];
    const _TUCHAR *fileName;
    
    /*
     * Set library 1 name.
     */
    fileName = FilePath_GetLeafPtr(Prog_lib[0].fileName);
    if (fileName[0] == '\0') {
        fileName = Prog_untitled;
    }
    _sntprintf(itemStr, ITEMSTRLEN, _T("Add To Library &1 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_ADD_TO_LIB_1;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_ADD_TO_LIB_1, FALSE, &menuItemInfo);
    /*
     * Set library 2 name.
     */
    fileName = FilePath_GetLeafPtr(Prog_lib[1].fileName);
    if (fileName[0] == '\0') {
        fileName = Prog_untitled;
    }
    _sntprintf(itemStr, ITEMSTRLEN, _T("Add To Library &2 (%.260s)")
            , fileName);
    menuItemInfo.cbSize = sizeof menuItemInfo;
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.wID = IDM_ADD_TO_LIB_2;
    menuItemInfo.dwTypeData = itemStr;
    SetMenuItemInfo(editorMenu, IDM_ADD_TO_LIB_2, FALSE, &menuItemInfo);
}

/*
 * OnKey() - Changes control focus for keyboard navigation and scrolls the
 *           window.
 */
void vd_OnKey(HWND voiceDlg, UINT vk, BOOL down, int repeat, UINT flags)
{
    HWND focusCtrl = GetFocus();
    UINT focusID = GetDlgCtrlID(focusCtrl);
    HWND destCtrl;
    UINT navIndex;
    RECT voiceDlgRect;
    RECT destFocusRect;

    if (focusID == IDC_LCD_SCROLLBAR) {
        focusCtrl = GetParent(focusCtrl);
        focusID = GetDlgCtrlID(focusCtrl);
    }

    switch (vk) {
        case VK_LEFT:
            destCtrl = KeyNav_FindAdjacentCtrl(voiceDlg, VoiceNav_nav
                    , VoiceNav_navCnt, VoiceNav_alt, VoiceNav_altCnt
                    , focusID, KN_LEFT, KN_RIGHT, &navIndex, TRUE);
            break;
        case VK_UP:
            destCtrl = KeyNav_FindAdjacentCtrl(voiceDlg, VoiceNav_nav
                    , VoiceNav_navCnt, VoiceNav_alt, VoiceNav_altCnt
                    , focusID, KN_UP, KN_DOWN, &navIndex, TRUE);
            break;
        case VK_RIGHT:
            destCtrl = KeyNav_FindAdjacentCtrl(voiceDlg, VoiceNav_nav
                    , VoiceNav_navCnt, VoiceNav_alt, VoiceNav_altCnt
                    , focusID, KN_RIGHT, KN_LEFT, &navIndex, TRUE);
            break;
        case VK_DOWN:
            destCtrl = KeyNav_FindAdjacentCtrl(voiceDlg, VoiceNav_nav
                    , VoiceNav_navCnt, VoiceNav_alt, VoiceNav_altCnt
                    , focusID, KN_DOWN, KN_UP, &navIndex, TRUE);
            break;
        default:
            return;
    }
    if (destCtrl == NULL) {
        return;
    }
    SetFocus(destCtrl);
    /*
     * Scroll window so control is fully visible, if necessary.
     */
    GetClientRect(voiceDlg, &voiceDlgRect);
    Window_GetParentRelativeRect(destCtrl, voiceDlg, &destFocusRect);
    if (!Rect_RectIn(&voiceDlgRect, &destFocusRect)) {
        int dx = 0, dy = 0;

        if (destFocusRect.left < voiceDlgRect.left) {
            /*
             * If the user is navigating to a control on the far left, scroll
             * all the way to the left.
             */
            if (VoiceNav_nav[navIndex][KN_LEFT] == 0) {
                dx = -vd_dialog->hScrollInfo.nPos;
            } else {
                dx = destFocusRect.left - voiceDlgRect.left - 5;
            }
        }
        if (destFocusRect.right > voiceDlgRect.right) {
            /*
             * If the user is navigating to a control on the far right, scroll
             * all the way to the right.
             */
            if (VoiceNav_nav[navIndex][KN_RIGHT] == 0) {
                dx = vd_dialog->hScrollInfo.nMax;
            } else {
                dx = destFocusRect.right - voiceDlgRect.right + 5;
            }
        }
        if (destFocusRect.top < voiceDlgRect.top) {
            /*
             * If the user is navigating to a control at the very top, scroll
             * all the way to the top.
             */
            if (VoiceNav_nav[navIndex][KN_UP] == 0) {
                dy = -vd_dialog->vScrollInfo.nPos;
            } else {
                dy = destFocusRect.top - voiceDlgRect.top - 5;
            }
        }
        if (destFocusRect.bottom > voiceDlgRect.bottom) {
            /*
             * If the user is navigating to a control at the very bottom,
             * scroll all the way to the bottom.
             */
            if (VoiceNav_nav[navIndex][KN_DOWN] == 0) {
                dy = vd_dialog->vScrollInfo.nMax;
            } else {
                dy = destFocusRect.bottom - voiceDlgRect.bottom + 5;
            }
        }
        if (dx || dy) {
            Dialog_Scroll(vd_dialog, dx, dy);
            if (!vd_dialog->hideScrollBars) {
                SetScrollInfo(voiceDlg, SB_HORZ, &vd_dialog->hScrollInfo, TRUE);
                SetScrollInfo(voiceDlg, SB_VERT, &vd_dialog->vScrollInfo, TRUE);
            }
        }
    }
}

/*
 * OnMeasureItem()
 */
void vd_OnMeasureItem(HWND voiceDlg, MEASUREITEMSTRUCT *measureItem)
{
    if (measureItem->CtlType == ODT_MENU) {
        measureItem->itemWidth = vd_optionMenuHeaderWidth;
    }
}

/*
 * OnMouseMove() - Scrolls the window when panning.
 */
void vd_OnMouseMove(HWND voiceDlg, int x, int y, UINT keyFlags)
{
    if (keyFlags & MK_RBUTTON) {
        int dx = vd_mDragLastX - x;
        int dy = vd_mDragLastY - y;

        Dialog_Scroll(vd_dialog, dx, dy);
        if (!vd_dialog->hideScrollBars) {
            SetScrollInfo(voiceDlg, SB_HORZ, &vd_dialog->hScrollInfo, TRUE);
            SetScrollInfo(voiceDlg, SB_VERT, &vd_dialog->vScrollInfo, TRUE);
        }
        vd_mDragLastX = x;
        vd_mDragLastY = y;
    }
}

/*
 * OnMouseWheel()
 */
void vd_OnMouseWheel(HWND voiceDlg, int delta, int x, int y, UINT keyFlags)
{
    if (VoiceDlg_mouseWheel == MW_SCROLL) {
        Dialog_Scroll(vd_dialog, 0
                , (vd_dialog->ctrlAreaSize.cy / 3) * (delta > 0 ? -1 : 1));
        if (!vd_dialog->hideScrollBars) {
            SetScrollInfo(voiceDlg, SB_HORZ, &vd_dialog->hScrollInfo, TRUE);
            SetScrollInfo(voiceDlg, SB_VERT, &vd_dialog->vScrollInfo, TRUE);
        }
    } else if (VoiceDlg_mouseWheel == MW_EDIT) {
        FORWARD_WM_MOUSEWHEEL(GetFocus(), delta, x, y, keyFlags, SendMessage);
    }
}

/*
 * OnRButtonDown() - Sets the hand cursor and begins the panning operation.
 */
void vd_OnRButtonDown(HWND voiceDlg, BOOL dblClick, int x, int y, UINT keyFlags)
{
    HWND captureWnd = GetCapture();

    if (!captureWnd) {
        SetCursor(Prog_panCursor);
        SetClassLong(voiceDlg, GCL_HCURSOR, (long) Prog_panCursor);
        SetCapture(voiceDlg);
        vd_mDragLastX = x;
        vd_mDragLastY = y;
    }
}

/*
 * OnRButtonUp() - Resets the cursor to an arrow and ends panning.
 */
int vd_OnRButtonUp(HWND voiceDlg, int x, int y, UINT keyFlags)
{
    HWND captureWnd = GetCapture();

    if (captureWnd == voiceDlg) {
        SetCursor(Prog_arrowCursor);
        SetClassLong(voiceDlg, GCL_HCURSOR, (long) Prog_arrowCursor);
        ReleaseCapture();
        return 1;
    }
    return 0;
}

/*
 * OnSimpleFreqChange() - Updates frequency controls when a simplified
 *                        frequency control changes.
 */
void vd_OnSimpleFreqChange(HWND voiceDlg, UINT ctrlID, int value)
{
    UINT crsID;
    UINT fineID;
    UINT rangeID;
    BOOL fixed;
    int op = vd_CtrlIdToOpNumber(ctrlID);
    int range, crs, fine;
    int i;
    int relativeDiff;
    BOOL savedOpFollowRecursion; /* Yay! Flag nightmare time! */
    
    vd_simpleUpdateFlag = TRUE;
    /*
     * If relative follow is on, get the amount the control has changed.
     */
    if (!vd_absoluteFollow) {
        relativeDiff = value - vd_simpleFreqValues[op];
        savedOpFollowRecursion = vd_opFollowRecursion;
        vd_opFollowRecursion = TRUE;
    }
    vd_simpleFreqValues[op] = value;
    fixed = vd_acedData[vd_freqCtrls[op].fixedIdx] != 0;
    crsID = vd_freqCtrls[op].ctrl[FCI_CRS].ctrlID;
    fineID = vd_freqCtrls[op].ctrl[FCI_FINE].ctrlID;

    /*
     * If the operator frequency is set to fixed, change the fixed controls.
     */
    if (fixed) {
        int curCrsLowBits = vd_vcedData[crsID - VCED_ID_FIRST] & 0x03;

        range = VoiceDlg_fixedFreqSort[value].range;
        crs = (VoiceDlg_fixedFreqSort[value].crs & 0xFC) | curCrsLowBits;
        fine = VoiceDlg_fixedFreqSort[value].fine;
        rangeID = vd_freqCtrls[op].ctrl[FCI_RANGE].ctrlID;
        LcdCtrl_SetValue(GetDlgItem(voiceDlg, rangeID), range);
        vd_ChangeAcedParameter(voiceDlg, rangeID - ACED_ID_FIRST, range, TRUE);
    /*
     * Else change the ratio controls.
     */
    } else {
        crs = VoiceDlg_freqRatioSort[value][0];
        fine = VoiceDlg_freqRatioSort[value][1];
    }
    LcdCtrl_SetValue(GetDlgItem(voiceDlg, crsID), crs);
    LcdCtrl_SetValue(GetDlgItem(voiceDlg, fineID), fine);
    vd_ChangeVcedParameter(voiceDlg, crsID - VCED_ID_FIRST, crs, TRUE);
    vd_ChangeAcedParameter(voiceDlg, fineID - ACED_ID_FIRST, fine, TRUE);
    /*
     * Do op follow for the simple frequency controls. (the parameters have
     * already been changed by vd_Change*cedParameter).
     */
    if (!savedOpFollowRecursion) {
        for (i = 0; i < 4; i++) {
            if (vd_IsOpFollowing(i, op)) {
                if (vd_absoluteFollow) {
                    LcdCtrl_SetValue(vd_freqCtrls[i].ctrl[FCI_SIMPLE].wnd
                            , value);
                    vd_simpleFreqValues[i] = value;
                } else {
                    int followValue = vd_simpleFreqValues[i] + relativeDiff;
                    HWND followCtrl = vd_freqCtrls[i].ctrl[FCI_SIMPLE].wnd;
                    int min = LcdCtrl_GetRangeMin(followCtrl);

                    if (followValue < min) {
                        followValue = min;
                    } else {
                        int max = LcdCtrl_GetRangeMax(followCtrl);

                        if (followValue > max) {
                            followValue = max;
                        }
                    }
                    LcdCtrl_SetValue(vd_freqCtrls[i].ctrl[FCI_SIMPLE].wnd
                            , followValue);
                    vd_OnSimpleFreqChange(voiceDlg
                            , vd_freqCtrls[i].ctrl[FCI_SIMPLE].ctrlID
                            , followValue);
                    vd_simpleFreqValues[i] = followValue;
                }
            }
        }
    }
    if (!vd_absoluteFollow) {
        vd_opFollowRecursion = savedOpFollowRecursion;
    }
    vd_simpleUpdateFlag = FALSE;
}

/*
 * OnSize()
 */
void vd_OnSize(HWND voiceDlg, UINT state, int cx, int cy)
{
    /*
     * If the the width of the window is different than the current dialog
     * control area and is greater than the base control area, stretch the
     * EG controls to fit the window, but don't shrink them smaller than the
     * base control area.
     */
    assert(vd_dialog);
    if (cx != vd_dialog->ctrlAreaSize.cx && cx > vd_baseCtrlArea.w) {
        HDC dC = GetDC(voiceDlg);
        HDWP defer = BeginDeferWindowPos(6);
        RECT invalidRect;
        int i;

        vd_EraseStretchingEdges(dC);
        ReleaseDC(voiceDlg, dC);
        vd_egOriginArea.w = cx - vd_egOriginArea.x - 5;
        if (AREA_R(vd_egOriginArea) + 5 < vd_baseCtrlArea.w) {
            vd_egOriginArea.w = vd_baseCtrlArea.w - vd_egOriginArea.x - 5;
        }
        DeferWindowPos(defer, vd_egOrigin, NULL, 0, 0, vd_egOriginArea.w,
                vd_egOriginArea.h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
        vd_egGraphLblArea.x = vd_egOriginArea.x + ((vd_egOriginArea.w
                    - vd_egGraphLblArea.w) >> 1);
        DeferWindowPos(defer, vd_egGraphLbl, NULL
                , vd_egGraphLblArea.x - vd_dialog->hScrollInfo.nPos
                , vd_egGraphLblArea.y - vd_dialog->vScrollInfo.nPos, 0, 0
                , SWP_NOSIZE | SWP_NOZORDER);
        for (i = 0; i < 4; i++) {
            vd_egGraphCtrlAreas[i].w = vd_egOriginArea.w;
            DeferWindowPos(defer, vd_egGraphCtrls[i], NULL, 0, 0
                    , vd_egOriginArea.w, vd_egGraphCtrlAreas[i].h
                    , SWP_NOMOVE | SWP_NOZORDER);
        }
        EndDeferWindowPos(defer);
        vd_dialog->ctrlAreaSize.cx = vd_egOriginArea.x + vd_egOriginArea.w + 5;
        Rect_SetNormalFromInts(&invalidRect, AREA_R(vd_egOriginArea), 0
                , cx, cy);
        InvalidateRect(voiceDlg, &invalidRect, TRUE);
        InvalidateRect(vd_egOrigin, NULL, FALSE);
    }
    Dialog_OnSize(vd_dialog, state, cx, cy);
    UpdateWindow(voiceDlg);
}

/*
 * OriginWndProc() - Sends focus back to the previous holder (when the EG
 *                   origin scroll bar is clicked).
 */
LRESULT CALLBACK vd_OriginWndProc(HWND scrollBar, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    if (message == WM_SETFOCUS) {
        DestroyCaret();
        return 0;
    }
    return CallWindowProc(vd_origOriginWndProc, scrollBar, message, wParam
            , lParam);
}

/*
 * PageSizeCrsFreq() - Returns the amount to page for the coarse frequency LCDs.
 */
int vd_PageSizeCrsFreq(UINT ctrlID, int dir, int value)
{
    int op = vd_CtrlIdToOpNumber(ctrlID);

    if (vd_acedData[vd_freqCtrls[op].fixedIdx]) {
        int size = 1;

        while ((value + dir * size) % 4 != 0) {
            size++;
        }
        return size;
    }

    return VoiceDlg_crsFreqPageSizes[value][dir == -1 ? 0 : 1];
}

/*
 * PageSizeSimpleFixed() - Used to page the simple ratio frequency controls.
 */
int vd_PageSizeSimpleFixed(UINT ctrlID, int dir, int value)
{
    // TODO: Find double or half the current frequency.
    return 16;
}

/*
 * PageSizeSimpleRatio() - Used to page the simple ratio frequency controls.
 */
int vd_PageSizeSimpleRatio(UINT ctrlID, int dir, int value)
{
    int crs, fine;
    int pageSize;
    int op = vd_CtrlIdToOpNumber(ctrlID);
    int i;

    /*
     * Find the page size that fits the coarse and leave the fine value alone,
     * unless the thumb is on the last possible scroll page in the range in
     * either direction.
     */
    crs = VoiceDlg_freqRatioSort[value][0];
    fine = VoiceDlg_freqRatioSort[value][1];
    crs += vd_PageSizeCrsFreq(vd_freqCtrls[op].ctrl[FCI_CRS].ctrlID, dir, crs)
        * dir;
    for (i = 0; i < SIMPLE_RATIO_RANGE_MAX; i++) {
        if (VoiceDlg_freqRatioSort[i][0] == crs && VoiceDlg_freqRatioSort[i][1] == fine)
        {
            pageSize = (i > value) ? i - value : value - i;
            break;
        }
    }
    if (pageSize == 0) {
        pageSize = (value < 496) ? value : SIMPLE_RATIO_RANGE_MAX - value;
    }
    return pageSize;
}

/*
 * QuickEditCopy() - Copies parameters from one operator to another.
 */
void vd_QuickEditCopy(HWND voiceDlg, UINT copyOpIdx, UINT quickEdit)
{
    int srcOp;
    int destOp;
    int srcOffset;
    int destOffset;
    const UINT (*ctrlIDs)[4] = NULL;
    int ctrlIDCnt;
    int i;

    /*
     * Convert copyOpIdx to source and destination operator indexes.
     */
    srcOp = copyOpIdx / 3;
    destOp = copyOpIdx % 3;
    destOp += (destOp >= srcOp) ? 1 : 0;
    /*
     * Start a new undo group.
     */
    vd_undoGroup = TRUE;
    /*
     * Set the follow resursion flag to temporarily disable op following.
     */
    vd_opFollowRecursion = TRUE;
    /*
     * Switch on the quick edit menu that was activated.
     */
    if (quickEdit == IDC_ALG_QUICKEDIT_BTN) {
        /*
         * Find the base offsets of the operators in the VCED.
         */
        srcOffset = vd_opVcedOffsets[srcOp];
        destOffset = vd_opVcedOffsets[destOp];
        /*
         * Copy the source operator's VCED data to the destination operator.
         */
        for (i = 0; i < TX81Z_VCED_OP_SIZE; i++) {
            vd_ChangeControl(voiceDlg, VCED_ID_FIRST + destOffset + i
                    , vd_vcedData[srcOffset + i], FALSE);
        }
        /*
         * Find the base offsets of the operators in the ACED.
         */
        srcOffset = vd_opAcedOffsets[srcOp];
        destOffset = vd_opAcedOffsets[destOp];
        /*
         * Copy the source operator's ACED data to the destination operator.
         */
        for (i = 0; i < TX81Z_ACED_OP_SIZE; i++) {
            vd_ChangeControl(voiceDlg, ACED_ID_FIRST + destOffset + i
                    , vd_acedData[srcOffset + i], FALSE);
        }
        /*
         * Update controls.
         */
        goto SendVoice;
    } else if (quickEdit == IDC_EG_QUICKEDIT_BTN) {
        ctrlIDs = vd_egCtrlIDs;
        ctrlIDCnt = vd_egCtrlIDCnt - 1;
    } else if (quickEdit == IDC_FREQ_QUICKEDIT_BTN) {
        ctrlIDs = vd_freqCtrlIDs;
        ctrlIDCnt = vd_freqCtrlIDCnt;
    } else if (quickEdit == IDC_OUTPUT_QUICKEDIT_BTN) {
        ctrlIDs = vd_outputCtrlIDs;
        ctrlIDCnt = vd_outputCtrlIDCnt;
    }
    for (i = 0; i < ctrlIDCnt; i++) {
        if (ctrlIDs[i][destOp] < ACED_ID_FIRST) {
            vd_ChangeControl(voiceDlg, ctrlIDs[i][destOp]
                    , vd_vcedData[ctrlIDs[i][srcOp] - VCED_ID_FIRST]
                    , FALSE);
        } else {
            vd_ChangeControl(voiceDlg, ctrlIDs[i][destOp]
                    , vd_acedData[ctrlIDs[i][srcOp] - ACED_ID_FIRST]
                    , FALSE);
        }
    }
    /*
     * Refresh the unit.
     */
SendVoice:
    vd_UpdateFrequencyDisplay(voiceDlg, destOp);
    TX81Z_SendData(Prog_midi, REQ_AVCED, &Prog_snapshot);
    /*
     * Clear the op follow recursion flag.
     */
    vd_opFollowRecursion = FALSE;
}

/*
 * QuickEditRand() - Randomizes a control group.
 */
void vd_QuickEditRand(HWND voiceDlg, int op, UINT quickEdit)
{
    int firstOp = op == 4 ? 0 : op;
    int lastOp = op == 4 ? 3 : op;
    int opOffset, firstOffset, lastOffset;
    const UINT (*ctrlIDs)[4] = NULL;
    int ctrlIDCnt;
    int i;

    /*
     * Start a new undo group.
     */
    vd_undoGroup = TRUE;
    /*
     * Set the follow resursion flag to temporarily disable op following.
     */
    vd_opFollowRecursion = TRUE;
    for (op = firstOp; op <= lastOp; op++) {
        /*
         * Switch on the quick edit menu that was activated.
         */
        if (quickEdit == IDC_ALG_QUICKEDIT_BTN) {
            /*
             * Find the base offset of the operator in the VCED.
             */
            opOffset = vd_opVcedOffsets[op];
            firstOffset = opOffset;
            lastOffset = opOffset + TX81Z_VCED_OP_SIZE - 1;
            /*
             * Randomize the operator's VCED data.
             */
            for (i = firstOffset; i <= lastOffset; i++) {
                vd_ChangeControl(voiceDlg, VCED_ID_FIRST + i
                        , RandRange(TXPack_vced[i].loLimit
                            , TXPack_vced[i].hiLimit)
                        , FALSE);
            }
            /*
             * Find the base offset of the operator in the ACED.
             */
            opOffset = vd_opAcedOffsets[op];
            firstOffset = opOffset;
            lastOffset = opOffset + TX81Z_ACED_OP_SIZE - 1;
            /*
             * Copy the source operator's ACED data to the destination operator.
             */
            for (i = firstOffset; i <= lastOffset; i++) {
                vd_ChangeControl(voiceDlg, ACED_ID_FIRST + i
                        , RandRange(TXPack_aced[i].loLimit
                            , TXPack_aced[i].hiLimit)
                        , FALSE);
            }
            /*
             * Update controls.
             */
            vd_UpdateFrequencyDisplay(voiceDlg, op);
            continue;
        } else if (quickEdit == IDC_EG_QUICKEDIT_BTN) {
            ctrlIDs = vd_egCtrlIDs;
            ctrlIDCnt = vd_egCtrlIDCnt - 1;
        } else if (quickEdit == IDC_FREQ_QUICKEDIT_BTN) {
            ctrlIDs = vd_freqCtrlIDs;
            ctrlIDCnt = vd_freqCtrlIDCnt;
        } else if (quickEdit == IDC_OUTPUT_QUICKEDIT_BTN) {
            ctrlIDs = vd_outputCtrlIDs;
            ctrlIDCnt = vd_outputCtrlIDCnt;
        }
        for (i = 0; i < ctrlIDCnt; i++) {
            if (ctrlIDs[i][op] < ACED_ID_FIRST) {
                int parameter = ctrlIDs[i][op] - VCED_ID_FIRST;

                vd_ChangeControl(voiceDlg, ctrlIDs[i][op]
                        , RandRange(TXPack_vced[parameter].loLimit
                            , TXPack_vced[parameter].hiLimit)
                        , FALSE);
            } else {
                int parameter = ctrlIDs[i][op] - ACED_ID_FIRST;

                vd_ChangeControl(voiceDlg, ctrlIDs[i][op]
                        , RandRange(TXPack_aced[parameter].loLimit
                            , TXPack_aced[parameter].hiLimit)
                        , FALSE);
            }
        }
    }
    /*
     * Refresh the unit.
     */
    TX81Z_SendData(Prog_midi, REQ_AVCED, &Prog_snapshot);
    /*
     * Clear the op follow recursion flag.
     */
    vd_opFollowRecursion = FALSE;
}

/*
 * QuickEditSwap() - Swaps operator parameters.
 */
void vd_QuickEditSwap(HWND voiceDlg, UINT swapOpIdx, UINT quickEdit)
{
    BYTE srcAndDestOps[6] = { '\x01', '\x02', '\x03', '\x12', '\x13', '\x23' };
    int srcOp;
    int destOp;
    int srcID;
    int destID;
    int srcOffset;
    int destOffset;
    const UINT (*ctrlIDs)[4] = NULL;
    int ctrlIDCnt;
    int i;
    BYTE tempAced[S_ACED_DATA_SIZE];
    BYTE tempVced[S_VCED_DATA_SIZE];

    /*
     * Convert copyOpIdx to source and destination operator indexes.
     */
    srcOp = srcAndDestOps[swapOpIdx] >> 4;
    destOp = srcAndDestOps[swapOpIdx] & 0x0F;
    /*
     * Make a copy of the voice data to work around the range changes screwing
     * up the swap.
     */
    memcpy(tempAced, vd_acedData, S_ACED_DATA_SIZE);
    memcpy(tempVced, vd_vcedData, S_VCED_DATA_SIZE);
    /*
     * Start a new undo group.
     */
    vd_undoGroup = TRUE;
    /*
     * Set the follow resursion flag to temporarily disable op following.
     */
    vd_opFollowRecursion = TRUE;
    /*
     * Switch on the quick edit menu that was activated.
     */
    if (quickEdit == IDC_ALG_QUICKEDIT_BTN) {
        /*
         * Find the base offsets of the operators in the VCED.
         */
        srcOffset = vd_opVcedOffsets[srcOp];
        destOffset = vd_opVcedOffsets[destOp];
        /*
         * Swap the source and destination operators' VCED data.
         */
        for (i = 0; i < TX81Z_VCED_OP_SIZE; i++, srcOffset++, destOffset++) {
            vd_ChangeControl(voiceDlg, VCED_ID_FIRST + srcOffset
                    , tempVced[destOffset], FALSE);
            vd_ChangeControl(voiceDlg, VCED_ID_FIRST + destOffset
                    , tempVced[srcOffset], FALSE);
        }
        /*
         * Find the base offsets of the operators in the ACED.
         */
        srcOffset = vd_opAcedOffsets[srcOp];
        destOffset = vd_opAcedOffsets[destOp];
        /*
         * Swap the source and destination operators' ACED data.
         */
        for (i = 0; i < TX81Z_ACED_OP_SIZE; i++, srcOffset++, destOffset++) {
            vd_ChangeControl(voiceDlg, ACED_ID_FIRST + srcOffset
                    , tempAced[destOffset], FALSE);
            vd_ChangeControl(voiceDlg, ACED_ID_FIRST + destOffset
                    , tempAced[srcOffset], FALSE);
        }
        goto SendVoice;
    } else if (quickEdit == IDC_EG_QUICKEDIT_BTN) {
        ctrlIDs = vd_egCtrlIDs;
        ctrlIDCnt = vd_egCtrlIDCnt - 1;
    } else if (quickEdit == IDC_FREQ_QUICKEDIT_BTN) {
        ctrlIDs = vd_freqCtrlIDs;
        ctrlIDCnt = vd_freqCtrlIDCnt;
    } else if (quickEdit == IDC_OUTPUT_QUICKEDIT_BTN) {
        ctrlIDs = vd_outputCtrlIDs;
        ctrlIDCnt = vd_outputCtrlIDCnt;
    } 
    for (i = 0; i < ctrlIDCnt; i++) {
        if (ctrlIDs[i][destOp] < ACED_ID_FIRST) {
            srcID = ctrlIDs[i][srcOp];
            destID = ctrlIDs[i][destOp];
            srcOffset = srcID - VCED_ID_FIRST;
            destOffset = destID - VCED_ID_FIRST;
            vd_ChangeControl(voiceDlg, srcID, tempVced[destOffset], FALSE);
            vd_ChangeControl(voiceDlg, destID, tempVced[srcOffset], FALSE);
        } else {
            srcID = ctrlIDs[i][srcOp];
            destID = ctrlIDs[i][destOp];
            srcOffset = srcID - ACED_ID_FIRST;
            destOffset = destID - ACED_ID_FIRST;
            vd_ChangeControl(voiceDlg, srcID, tempAced[destOffset], FALSE);
            vd_ChangeControl(voiceDlg, destID, tempAced[srcOffset], FALSE);
        }
    }
    /*
     * Refresh the unit.
     */
SendVoice:
    TX81Z_SendData(Prog_midi, REQ_AVCED, &Prog_snapshot);
    /*
     * Clear the op follow recursion flag.
     */
    vd_opFollowRecursion = FALSE;
}

/*
 * RandomizeVoice() - An absolutely hideous voice randomizer.
 */
void vd_RandomizeVoice(HWND voiceDlg)
{
    int i;

    for (i = 0; i < TXPack_vcedCnt; i++) {
        const TXPACK *pack = &TXPack_vced[i];

        vd_vcedData[i] = RandRange(pack->loLimit, pack->hiLimit);
    }
    for (i = 0; i < TXPack_acedCnt; i++) {
        const TXPACK *pack = &TXPack_aced[i];

        vd_acedData[i] = RandRange(pack->loLimit, pack->hiLimit);
    }
    TX81Z_SendData(Prog_midi, REQ_AVCED, &Prog_snapshot);
    vd_InitControlValues(voiceDlg);
}

/*
 * Redo() - Redoes a previously undone edit.
 */
void CALLBACK vd_Redo(HWND voiceDlg, CHANGE *change)
{
    vd_undoFlag = TRUE;
    if (change->groupStart) {
        vd_FocusCtrl(voiceDlg, change->ctrlID);
    }
    vd_ChangeControl(voiceDlg, change->ctrlID, *(int *)change->newValue
            , CP_SEND);
    vd_undoFlag = FALSE;
}

/*
 * SimpleFreqSelection() - Updates dialog controls when the "Simple Frequency"
 *                         status changes.
 */
void vd_SimpleFreqSelection(HWND voiceDlg, BOOL simpleOn)
{
    static const UINT cmplxFreqIds[] = {
        IDC_OP1_FREQ,
        IDC_OP1_RANGE,
        IDC_OP1_CRS,
        IDC_OP1_FINE,
        IDC_OP2_FREQ,
        IDC_OP2_RANGE,
        IDC_OP2_CRS,
        IDC_OP2_FINE,
        IDC_OP3_FREQ,
        IDC_OP3_RANGE,
        IDC_OP3_CRS,
        IDC_OP3_FINE,
        IDC_OP4_FREQ,
        IDC_OP4_RANGE,
        IDC_OP4_CRS,
        IDC_OP4_FINE,
        IDC_RANGE_LBL,
        IDC_CRS_LBL,
        IDC_FINE_LBL,
        IDC_DETUNE_LBL,
    };
#define cmplxFreqIdCnt (sizeof cmplxFreqIds / sizeof cmplxFreqIds[0])
    static const UINT simpleFreqIds[] = {
        IDC_OP1_SIMPLE_FREQ,
        IDC_OP2_SIMPLE_FREQ,
        IDC_OP3_SIMPLE_FREQ,
        IDC_OP4_SIMPLE_FREQ,
    };
#define simpleFreqIdCnt (sizeof simpleFreqIds / sizeof simpleFreqIds[0])
    int i, op;
    HWND focusCtrl = GetParent(GetFocus());

    /*
     * Set the simple frequency LCD value.
     */
    if (simpleOn) {
        for (op = 0; op < 4; op++) {
            vd_UpdateSimpleFreqValue(voiceDlg, op, VoiceDlg_simpleFreqSelection);
        }
    }
    /*
     * Show and hide frequency controls.
     */
    for (i = 0; i < cmplxFreqIdCnt; i++) {
        HWND ctrl = GetDlgItem(voiceDlg, cmplxFreqIds[i]); 

        ShowWindow(ctrl, simpleOn ? SW_HIDE : SW_SHOW);
        if (simpleOn && focusCtrl == ctrl) {
            SetFocus(GetDlgItem(voiceDlg, simpleFreqIds[i / 4]));
            focusCtrl = NULL;
        }
    }
    for (i = 0; i < simpleFreqIdCnt; i++) {
        HWND ctrl = GetDlgItem(voiceDlg, simpleFreqIds[i]);

        ShowWindow(ctrl, simpleOn ? SW_SHOW : SW_HIDE);
        if (!simpleOn && focusCtrl == ctrl) {
            SetFocus(GetDlgItem(voiceDlg, cmplxFreqIds[i * 4 + 2]));
        }
    }
}

/*
 * SubclassControls() - Sets up standard windows controls for keyboard
 *                      navigation and middle mouse button panning.
 */
BOOL CALLBACK vd_SubclassControls(HWND ctrl, LPARAM lParam)
{
    _TUCHAR className[30];

    GetClassName(ctrl, className, 30);
    if (StriEq(className, Prog_buttonClassName)) {
        if (GetDlgCtrlID(ctrl) < 6500) {
            KeyNav_SubclassButton(ctrl);
        }
    } else if (StriEq(className, Prog_comboBoxClassName)) {
        KeyNav_SubclassComboBox(ctrl);
    } else if (StriEq(className, LcdCtrl_className)) {
        KeyNav_SubclassLcdCtrl(ctrl);
    } else if (StriEq(className, Prog_scrollBarClassName)) {
        UINT ctrlID = GetDlgCtrlID(ctrl);

        if (ctrlID == IDC_EG_ORIGIN) {
            KeyNav_SubclassScrollBar(ctrl);
        }
    } else if (StriEq(className, RPanel_className)) {
        KeyNav_SubclassRPanel(ctrl);
    }
    return TRUE;
}

/*
 * Undo() - Undoes an edit done to a control.
 */
void CALLBACK vd_Undo(HWND voiceDlg, CHANGE *change)
{
    vd_undoFlag = TRUE;
    if (change->groupStart) {
        vd_FocusCtrl(voiceDlg, change->ctrlID);
    }
    vd_ChangeControl(voiceDlg, change->ctrlID, *(int *) change->oldValue
            , CP_SEND);
    vd_undoFlag = FALSE;
}

/*
 * UpdateFixedRatioStatus() - Updates the frequency controls based on the fixed
 *                            status.
 */
void vd_UpdateFixedRatioStatus(HWND voiceDlg, int op)
{
    BOOL fixed = vd_acedData[vd_freqCtrls[op].fixedIdx] != 0;

    /*
     * Enable/disable the fixed range slider.
     */
    EnableWindow(vd_freqCtrls[op].ctrl[FCI_RANGE].wnd, fixed);
    if (VoiceDlg_simpleFreqSelection) {
        HWND simpleCtrl = vd_freqCtrls[op].ctrl[FCI_SIMPLE].wnd;

        /*
         * Reinitialize the simplified frequency slider with fixed/ratio
         * parameters.
         */
        if (fixed) {
            LcdCtrl_SpecialInit(simpleCtrl, &vd_simpleFixedLcdInits[op]);
        } else {
            LcdCtrl_SpecialInit(simpleCtrl, &vd_simpleRatioLcdInits[op]);
        }
        vd_UpdateSimpleFreqValue(voiceDlg, op, VoiceDlg_simpleFreqSelection);
    }
}

/*
 * UpdateFrequencyDisplay() - Refreshes a frequency display static control.
 */
void vd_UpdateFrequencyDisplay(HWND voiceDlg, int op)
{
    BOOL fixed;
    int crs, fine;
    int index;
    _TUCHAR text[20];

    fixed = vd_acedData[vd_freqCtrls[op].fixedIdx] != 0;
    crs = vd_vcedData[vd_freqCtrls[op].ctrl[FCI_CRS].ctrlID - VCED_ID_FIRST];
    fine = vd_acedData[vd_freqCtrls[op].ctrl[FCI_FINE].ctrlID - ACED_ID_FIRST];
    if (crs < 4 && fine > 7) {
        vd_CheckFreqSliderRange(voiceDlg, op);
        fine
            = vd_acedData[vd_freqCtrls[op].ctrl[FCI_FINE].ctrlID - ACED_ID_FIRST]
            = 7;
    }
    if (fixed) {
        int range = vd_acedData[vd_freqCtrls[op].ctrl[FCI_RANGE].ctrlID
            - ACED_ID_FIRST];

        /*
         * Convert the frequency settings to a formatted frequency string.
         */
        _sntprintf(text, 20, _T("%5dHz")
                , VoiceDlg_FixedParamsToFreq(range, crs, fine));
    } else {
        /*
         * Copy the frequency from the frequency ratio array.
         */
        index = (crs << 4) | (fine & 0x0F);
        _tcsncpy(text, VoiceDlg_freqRatios[index], 5);
        text[5] = '\0';
    }
    /*
     * Set the frequency display text.
     */
    Static_SetText(vd_freqCtrls[op].ctrl[FCI_FREQSTC].wnd, text);
}

/*
 * UpdateQuickEditMenu() - Replaces the names of the quickedit submenus and
 *                         sets the flag that indicates which quickedit menu
 *                         button was pressed.
 */
void vd_UpdateQuickEditMenu(UINT ctrlID)
{
    MENUITEMINFO menuItemInfo;
    _TUCHAR text[TEXT_LEN];
    int i, m;
    static const int subItemCnts[3] = { 15, 15, 6 };

    for (m = 0; m < 3; m++) {
        HMENU subMenu = GetSubMenu(Prog_voiceQuickEditMenu, m);
        _TUCHAR *varTextStart;

        for (i = 0; i < subItemCnts[m]; i++) {
            menuItemInfo.cbSize = sizeof(MENUITEMINFO);
            menuItemInfo.fMask = MIIM_TYPE;
            menuItemInfo.fType = MFT_STRING;
            menuItemInfo.dwTypeData = text;
            menuItemInfo.cch = TEXT_LEN;
            GetMenuItemInfo(subMenu, i, MF_BYPOSITION, &menuItemInfo);
            if ((menuItemInfo.fType & MFT_SEPARATOR) == 0) {
                varTextStart = _tcschr(text, '(') + 1;
                switch (ctrlID) {
                    case IDC_ALG_QUICKEDIT_BTN:
                        _tcscpy(varTextStart, _T("Entire Operator)"));
                        break;
                    case IDC_FREQ_QUICKEDIT_BTN:
                        _tcscpy(varTextStart, _T("Frequency Parameters)"));
                        break;
                    case IDC_EG_QUICKEDIT_BTN:
                        _tcscpy(varTextStart, _T("EG Parameters, including EBS)"));
                        break;
                    case IDC_OUTPUT_QUICKEDIT_BTN:
                        _tcscpy(varTextStart, _T("Output Parameters)"));
                        break;
                }
                SetMenuItemInfo(subMenu, i, MF_BYPOSITION, &menuItemInfo);
            }
        }
    }
}

/*
 * UpdateSimpleFreqValue() - Updates the display of a simple frequency control.
 */
void vd_UpdateSimpleFreqValue(HWND voiceDlg, int op, BOOL simpleOn)
{
    BOOL fixed = vd_acedData[vd_freqCtrls[op].fixedIdx] != 0;

    if (simpleOn && !vd_simpleUpdateFlag) {
        int crs, fine, range;
        int value;
        HWND simpleCtrl = vd_freqCtrls[op].ctrl[FCI_SIMPLE].wnd;

        /*
         * Derive the new value from the range, coarse and frequency
         * controls.
         */
        crs = vd_vcedData[vd_freqCtrls[op].ctrl[FCI_CRS].ctrlID - VCED_ID_FIRST];
        fine = vd_acedData[vd_freqCtrls[op].ctrl[FCI_FINE].ctrlID - ACED_ID_FIRST];
        range = vd_acedData[vd_freqCtrls[op].ctrl[FCI_RANGE].ctrlID - ACED_ID_FIRST];
        if (fixed) {
            FIXEDFREQSORT key = { range, crs, fine
                , VoiceDlg_FixedParamsToFreq(range, crs, fine) };

            /*
             * Initialize the simple control with the fixed init parameters.
             */
            if (LcdCtrl_GetRangeMax(simpleCtrl) != SIMPLE_FIXED_RANGE_MAX - 1) {
                LcdCtrl_SpecialInit(simpleCtrl, &vd_simpleFixedLcdInits[op]);
            }
            /*
             * Find the value to set the slider to.
             */
            value = ((FIXEDFREQSORT *) bsearch(
                        &key
                        , VoiceDlg_fixedFreqSort
                        , SIMPLE_FIXED_RANGE_MAX
                        , sizeof(FIXEDFREQSORT)
                        , VoiceDlg_FixedFreqSortCmp)
                    ) - VoiceDlg_fixedFreqSort;
        } else {
            /*
             * Initialize the simple control with the ratio init parameters.
             */
            if (LcdCtrl_GetRangeMax(simpleCtrl) != SIMPLE_RATIO_RANGE_MAX - 1) {
                LcdCtrl_SpecialInit(simpleCtrl, &vd_simpleRatioLcdInits[op]);
            }
            /*
             * Find the value to set the slider to.
             */
            for (value = 0; value < SIMPLE_RATIO_RANGE_MAX; value++) {
                if (VoiceDlg_freqRatioSort[value][0] == crs
                        && VoiceDlg_freqRatioSort[value][1] == fine)
                {
                    break;
                }
            }
        }
        /*
         * Update the LCD value.
         */
        LcdCtrl_SetValue(simpleCtrl, value);
        /*
         * Update the cached values for the relative follow function.
         */
        vd_simpleFreqValues[op] = value;
    }
}

