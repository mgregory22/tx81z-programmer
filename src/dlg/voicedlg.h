/*
 * voicedlg.h - TX81Z voice editor dialog
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
#ifndef VOICEDLG_H
#define VOICEDLG_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef SNAPSHOT_H
#   include "snapshot.h"
#endif


/*
 * Global types
 */
typedef struct {
    BYTE range, crs, fine;
    unsigned short freq;
} FIXEDFREQSORT;

typedef enum {
    MW_OFF,
    MW_SCROLL,
    MW_EDIT,
} MOUSEWHEEL;

/*
 * Global constants
 */
#define SIMPLE_FIXED_RANGE_MAX 1984
#define SIMPLE_RATIO_RANGE_MAX 992
extern const _TUCHAR *VoiceDlg_className;
extern const _TUCHAR VoiceDlg_freqRatios[1024][5];
extern const BYTE VoiceDlg_freqRatioSort[SIMPLE_RATIO_RANGE_MAX][2];
extern const _TUCHAR VoiceDlg_crsFreqPageSizes[64][2];

/*
 * Global procedures
 */
BOOL VoiceDlg_Create(HWND parentWnd, AVCED *voice);
int VoiceDlg_FixedFreqSortCmp(const FIXEDFREQSORT *ffs1
        , const FIXEDFREQSORT *ffs2);
int VoiceDlg_FixedParamsToFreq(int range, int crs, int fine);
void VoiceDlg_Update(HWND voiceDlg, AVCED *voice);

/*
 * Global variables
 */
extern FIXEDFREQSORT VoiceDlg_fixedFreqSort[SIMPLE_FIXED_RANGE_MAX];
extern _TUCHAR VoiceDlg_simpleRatioSettings[SIMPLE_RATIO_RANGE_MAX][5];
extern _TUCHAR VoiceDlg_simpleFixedSettings[SIMPLE_FIXED_RANGE_MAX][5];
extern BOOL VoiceDlg_fastWaveSelection;
extern BOOL VoiceDlg_simpleFreqSelection;
extern BOOL VoiceDlg_hideEGLabels;
extern BOOL VoiceDlg_hideScrollBars;
extern MOUSEWHEEL VoiceDlg_mouseWheel;


#endif
