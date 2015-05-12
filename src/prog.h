/*
 * prog.h - main program header
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
#ifndef PROG_H
#define PROG_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_STDLIB
#   include <stdlib.h>
#endif
#ifndef _INC_TIME
#   include <time.h>
#endif
#ifndef GUI_REGISTRY_H
#   include "registry.h"
#endif
#ifndef MIDI_MIDI_H
#   include "midi.h"
#endif
#ifndef TX81Z_H
#   include "tx81z.h"
#endif
#ifndef TXLIB_H
#   include "txlib.h"
#endif


/*
 * Global types
 */
typedef enum {
    REG_PROG,
    REG_MAINWND,
    REG_VOICEDLG,
    REG_PFMDLG,
    REG_PCDLG,
    REG_MTODLG,
    REG_MTFDLG,
} REGGROUP;

typedef enum {
    CP_NOSEND  = 0x00,
    CP_SEND    = 0x01,
} CPSEND;

typedef void (CALLBACK *CHANGEPARAMFUNC)(UINT parameter, int value
        , CPSEND send);

/*
 * Global constants
 */
#define Prog_recentFileMax    4
#define Prog_keyNameStringCnt 128
#define Prog_menuFlags  (TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD \
            | TPM_LEFTBUTTON | TPM_HORIZONTAL)
#define Prog_logoWidth  347
#define Prog_logoHeight 51
/*
 * Window messages and notifications
 */
#define EDM_REFRESH    (WM_APP + 1)  /* Refresh editor dialog */
#define EDM_SAVED      (WM_APP + 2)  /* Notify the dialog that the file has
                                        been saved--turns off the dirty flag */
#define EDN_CHANGE     1             /* Editor dialog changed */

/*
 * Global procedures
 */
void Prog_ChangeControl(HWND ctrl, UINT parameter, void *valuePtr, CPSEND send
        , CHANGEPARAMFUNC ChangeParameter);
void Prog_CheckRegistration(_TUCHAR *userName, _TUCHAR *serial);
void Prog_CreateFonts(HWND wnd);
void Prog_DeleteFonts(void);
void Prog_OpenHelp(HWND wnd, _TUCHAR *topicUrl);
BOOL CALLBACK Prog_SetBoldFont(HWND dlgCtrl, LPARAM lParam);
void Prog_UpdateSysColors(void);

/*
 * Global variables
 */
#ifdef DEFINE_GLOBALS

const _TUCHAR Prog_name[] = _T("TX81Z Programmer");
const _TUCHAR Prog_copyright[] = _T("Copyright (c) 2005-2015 Matt Gregory");
const _TUCHAR Prog_version[] = _T("1.22");
HINSTANCE Prog_instance;
HREGISTRY Prog_registry;
HWND Prog_mainWnd;
HWND Prog_tx81zWnd;
HWND Prog_kybdDlg;
HWND Prog_remoteWnd;
HWND Prog_voiceDlg;
HWND Prog_pfmDlg;
HWND Prog_fxDlg;
HWND Prog_pcDlg;
HWND Prog_mtoDlg;
HWND Prog_mtfDlg;
HWND Prog_sysDlg;
HWND Prog_activeWnd;
const _TUCHAR Prog_buttonClassName[] = _T("Button");
const _TUCHAR Prog_comboBoxClassName[] = _T("ComboBox");
const _TUCHAR Prog_scrollBarClassName[] = _T("ScrollBar");

/*
 * GDI cache and resources
 */
HICON Prog_icon;
HACCEL Prog_accels;
HCURSOR Prog_arrowCursor;
HCURSOR Prog_waitCursor;
HCURSOR Prog_splitUpDownCursor;
HCURSOR Prog_splitDownCursor;
HCURSOR Prog_splitUpCursor;
HCURSOR Prog_splitLeftRightCursor;
HCURSOR Prog_splitLeftCursor;
HCURSOR Prog_splitRightCursor;
HCURSOR Prog_panCursor;
HCURSOR Prog_fingerCursor;
HCURSOR Prog_linkCursor;
HBITMAP Prog_logoBmp;
HBITMAP Prog_fileBmp;
HBITMAP Prog_sortBmp;
HBITMAP Prog_copyLeftBmp;
HBITMAP Prog_copyRightBmp;
HBITMAP Prog_deleteBmp;
HBITMAP Prog_renameBmp;
HBITMAP Prog_recommentBmp;
HBITMAP Prog_diffBmp;
HBITMAP Prog_setBmp;
HBITMAP Prog_placeBmp;
HBITMAP Prog_unplaceBmp;
HBITMAP Prog_disabledCopyLeftBmp;
HBITMAP Prog_disabledCopyRightBmp;
HBITMAP Prog_disabledDeleteBmp;
HBITMAP Prog_disabledRenameBmp;
HBITMAP Prog_disabledRecommentBmp;
HBITMAP Prog_disabledDiffBmp;
HBITMAP Prog_disabledSetBmp;
HBITMAP Prog_disabledPlaceBmp;
HBITMAP Prog_resetBmp;
HBITMAP Prog_storeBmp;
HBITMAP Prog_utilityBmp;
HBITMAP Prog_editCmpBmp;
HBITMAP Prog_playPfmBmp;
HBITMAP Prog_leftArrowBmp;
HBITMAP Prog_rightArrowBmp;
HBITMAP Prog_incBmp;
HBITMAP Prog_decBmp;
HBITMAP Prog_cursorBmp;
HMENU Prog_snapshotFileMenu;
HMENU Prog_libFileMenu;
HMENU Prog_libSortMenu;
HMENU Prog_voiceQuickEditMenu;
HMENU Prog_pfmQuickEditMenu;
COLORREF Prog_3dHighlightColor;
COLORREF Prog_3dLightColor;
COLORREF Prog_3dShadowColor;
COLORREF Prog_dirtyColor;
COLORREF Prog_dlgColor;
COLORREF Prog_dlgTextColor;
COLORREF Prog_grayTextColor;
COLORREF Prog_hiColor = DKRED;
COLORREF Prog_custColors[16] = {
    DKRED, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE
};
COLORREF Prog_infoTextColor;
COLORREF Prog_selColor;
COLORREF Prog_selTextColor;
COLORREF Prog_unloadedColor;
COLORREF Prog_wndColor;
COLORREF Prog_wndTextColor;
HBRUSH Prog_dlgBrush;
HBRUSH Prog_infoBkBrush;
HBRUSH Prog_hiBrush;
HBRUSH Prog_ltGrayBrush;
HBRUSH Prog_selBrush;
HBRUSH Prog_selTextBrush;
HBRUSH Prog_whiteBrush;
HBRUSH Prog_wndBrush;
HBRUSH Prog_wndTextBrush;
HPEN Prog_3dLightPen;
HPEN Prog_3dShadowPen;
HPEN Prog_blackPen;
HPEN Prog_dlgPen;
HPEN Prog_grayTextPen;
HPEN Prog_hiPen;
HPEN Prog_ltGrayPen;
HPEN Prog_selTextPen;
HPEN Prog_whitePen;
HPEN Prog_wndPen;
HPEN Prog_wndTextPen;
HFONT Prog_tahomaFont;
HFONT Prog_tahomaBoldFont;
HFONT Prog_tahomaUnderlineFont;
int Prog_hSliderH;
int Prog_vSliderW;

/*
 * Program data
 */
SNAPSHOT Prog_snapshot;
TXLIB Prog_lib[2];

/*
 * Program options / registry settings
 */
_TUCHAR Prog_snapshotRecent[Prog_recentFileMax][_MAX_PATH];
_TUCHAR Prog_libRecent[2][Prog_recentFileMax][_MAX_PATH];
_TUCHAR Prog_snapshotRecentDir[_MAX_PATH];
_TUCHAR Prog_libRecentDir[_MAX_PATH];
_TUCHAR Prog_importRecentDir[_MAX_PATH];
_TUCHAR Prog_exportRecentDir[_MAX_PATH];
WINDOWPLACEMENT Prog_mainWndPlacement = { sizeof(WINDOWPLACEMENT) };
int Prog_mainWndHSplitterPos;
int Prog_mainWndZoomedHSplitterPos;
WINDOWPLACEMENT Prog_kybdDlgPlacement = { sizeof(WINDOWPLACEMENT) };
WINDOWPLACEMENT Prog_voiceDlgPlacement = { sizeof(WINDOWPLACEMENT) };
WINDOWPLACEMENT Prog_pfmDlgPlacement = { sizeof(WINDOWPLACEMENT) };
WINDOWPLACEMENT Prog_pcDlgPlacement = { sizeof(WINDOWPLACEMENT) };
WINDOWPLACEMENT Prog_mtoDlgPlacement = { sizeof(WINDOWPLACEMENT) };
WINDOWPLACEMENT Prog_mtfDlgPlacement = { sizeof(WINDOWPLACEMENT) };
MIDI *Prog_midi;
BOOL Prog_openPrevious;
BOOL Prog_startupEditBufferRequest;
BOOL Prog_showFullPaths;
BOOL Prog_dblClkingBankDoesntOpen;
BOOL Prog_mainWndToFront;
BOOL Prog_snapshotAutoSave;
BOOL Prog_libraryNoDuplicates;
BOOL Prog_unslashedZeros;
int Prog_tx81zVersion = 11;
BOOL Prog_remoteStayOnTop;
BOOL Prog_kybdStayOnTop;
int Prog_kybdChannel;
BOOL Prog_fixedVelEnable;
int Prog_fixedVelValue = 90;
DWORD Prog_rxOptions;
DWORD Prog_txOptions;
DWORD Prog_importOptions;
DWORD Prog_exportOptions;
DWORD Prog_diffView;

/*
 * Special LCD display string arrays
 */
_TUCHAR Prog_keyNameStrings[Prog_keyNameStringCnt][3];
_TUCHAR Prog_rcvChSettings[17][3];

/*
 * Other junk
 */
const _TUCHAR Prog_on[] = _T("On");
const _TUCHAR Prog_off[] = _T("Off");
const _TUCHAR Prog_disabled[] = _T(" * ");
const _TUCHAR Prog_vMemStr[] = _T("VMEM");
const _TUCHAR Prog_pMemStr[] = _T("PMEM");
const _TUCHAR Prog_bndlStr[] = _T("BNDL");
const _TUCHAR Prog_fileOpenErrorFmt[] = _T("Error opening file %s: ");
const _TUCHAR Prog_fileReadErrorFmt[] = _T("Error reading file %s: ");
const _TUCHAR Prog_fileWriteErrorFmt[] = _T("Error writing to file %s: ");
const _TUCHAR Prog_allFileFilter[] = _T("All Files (*.*)\0*.*\0");
const _TUCHAR Prog_libFilter[] = _T("TX81Z Library Files (*.tx8)\0*.tx8\0")
    _T("All Files (*.*)\0*.*\0");
const _TUCHAR Prog_libExt[] = _T("tx8");
const _TUCHAR Prog_syxFilter[] = _T("Sysex Files (*.syx)\0*.syx\0")
        _T("All Files (*.*)\0*.*\0");
const _TUCHAR Prog_syxExt[] = _T("syx");
const _TUCHAR Prog_txtFilter[] = _T("Text Files (*.txt)\0*.txt\0")
        _T("All Files (*.*)\0*.*\0");
const _TUCHAR Prog_txtExt[] = _T("txt");
const _TUCHAR Prog_untitled[] = _T("untitled");
const _TUCHAR Prog_zero[] = _T("0");
const _TUCHAR Prog_arStr[] = _T("AR");
const _TUCHAR Prog_d1rStr[] = _T("D1R");
const _TUCHAR Prog_d1lStr[] = _T("D1L");
const _TUCHAR Prog_d2rStr[] = _T("D2R");
const _TUCHAR Prog_rrStr[] = _T("RR");
const _TUCHAR Prog_shftStr[] = _T("SHFT");
#ifdef PROG_LOGFILE
FILE *Prog_logFile;
#endif

#else

extern const _TUCHAR Prog_name[];
extern const _TUCHAR Prog_copyright[];
extern const _TUCHAR Prog_version[];
extern HINSTANCE Prog_instance;
extern HREGISTRY Prog_registry;
extern HWND Prog_mainWnd;
extern HWND Prog_tx81zWnd;
extern HWND Prog_kybdDlg;
extern HWND Prog_remoteWnd;
extern HWND Prog_voiceDlg;
extern HWND Prog_pfmDlg;
extern HWND Prog_fxDlg;
extern HWND Prog_pcDlg;
extern HWND Prog_mtoDlg;
extern HWND Prog_mtfDlg;
extern HWND Prog_sysDlg;
extern HWND Prog_activeWnd;
extern const _TUCHAR Prog_buttonClassName[];
extern const _TUCHAR Prog_comboBoxClassName[];
extern const _TUCHAR Prog_scrollBarClassName[];

/*
 * GDI cache and resources
 */
extern HICON Prog_icon;
extern HACCEL Prog_accels;
extern HCURSOR Prog_arrowCursor;
extern HCURSOR Prog_waitCursor;
extern HCURSOR Prog_splitUpDownCursor;
extern HCURSOR Prog_splitDownCursor;
extern HCURSOR Prog_splitUpCursor;
extern HCURSOR Prog_splitLeftRightCursor;
extern HCURSOR Prog_splitLeftCursor;
extern HCURSOR Prog_splitRightCursor;
extern HCURSOR Prog_panCursor;
extern HCURSOR Prog_fingerCursor;
extern HCURSOR Prog_linkCursor;
extern HBITMAP Prog_logoBmp;
extern HBITMAP Prog_fileBmp;
extern HBITMAP Prog_sortBmp;
extern HBITMAP Prog_copyLeftBmp;
extern HBITMAP Prog_copyRightBmp;
extern HBITMAP Prog_deleteBmp;
extern HBITMAP Prog_renameBmp;
extern HBITMAP Prog_recommentBmp;
extern HBITMAP Prog_diffBmp;
extern HBITMAP Prog_setBmp;
extern HBITMAP Prog_placeBmp;
extern HBITMAP Prog_unplaceBmp;
extern HBITMAP Prog_disabledCopyLeftBmp;
extern HBITMAP Prog_disabledCopyRightBmp;
extern HBITMAP Prog_disabledDeleteBmp;
extern HBITMAP Prog_disabledRenameBmp;
extern HBITMAP Prog_disabledRecommentBmp;
extern HBITMAP Prog_disabledDiffBmp;
extern HBITMAP Prog_disabledSetBmp;
extern HBITMAP Prog_disabledPlaceBmp;
extern HBITMAP Prog_resetBmp;
extern HBITMAP Prog_storeBmp;
extern HBITMAP Prog_utilityBmp;
extern HBITMAP Prog_editCmpBmp;
extern HBITMAP Prog_playPfmBmp;
extern HBITMAP Prog_leftArrowBmp;
extern HBITMAP Prog_rightArrowBmp;
extern HBITMAP Prog_incBmp;
extern HBITMAP Prog_decBmp;
extern HBITMAP Prog_cursorBmp;
extern HMENU Prog_snapshotFileMenu;
extern HMENU Prog_libFileMenu;
extern HMENU Prog_libSortMenu;
extern HMENU Prog_voiceQuickEditMenu;
extern HMENU Prog_pfmQuickEditMenu;
extern COLORREF Prog_3dHighlightColor;
extern COLORREF Prog_3dLightColor;
extern COLORREF Prog_3dShadowColor;
extern COLORREF Prog_dirtyColor;
extern COLORREF Prog_dlgColor;
extern COLORREF Prog_dlgTextColor;
extern COLORREF Prog_grayTextColor;
extern COLORREF Prog_hiColor;
extern COLORREF Prog_custColors[16];
extern COLORREF Prog_infoTextColor;
extern COLORREF Prog_selColor;
extern COLORREF Prog_selTextColor;
extern COLORREF Prog_unloadedColor;
extern COLORREF Prog_wndColor;
extern COLORREF Prog_wndTextColor;
extern HBRUSH Prog_dlgBrush;
extern HBRUSH Prog_infoBkBrush;
extern HBRUSH Prog_hiBrush;
extern HBRUSH Prog_ltGrayBrush;
extern HBRUSH Prog_selBrush;
extern HBRUSH Prog_selTextBrush;
extern HBRUSH Prog_whiteBrush;
extern HBRUSH Prog_wndBrush;
extern HBRUSH Prog_wndTextBrush;
extern HPEN Prog_3dLightPen;
extern HPEN Prog_3dShadowPen;
extern HPEN Prog_blackPen;
extern HPEN Prog_dlgPen;
extern HPEN Prog_grayTextPen;
extern HPEN Prog_hiPen;
extern HPEN Prog_ltGrayPen;
extern HPEN Prog_selTextPen;
extern HPEN Prog_whitePen;
extern HPEN Prog_wndPen;
extern HPEN Prog_wndTextPen;
extern HFONT Prog_tahomaFont;
extern HFONT Prog_tahomaBoldFont;
extern HFONT Prog_tahomaUnderlineFont;
extern int Prog_hSliderH;
extern int Prog_vSliderW;

/*
 * Program data
 */
extern SNAPSHOT Prog_snapshot;
extern TXLIB Prog_lib[2];

/*
 * Program options / registry settings
 */
extern _TUCHAR Prog_snapshotRecent[Prog_recentFileMax][_MAX_PATH];
extern _TUCHAR Prog_libRecent[2][Prog_recentFileMax][_MAX_PATH];
extern _TUCHAR Prog_snapshotRecentDir[_MAX_PATH];
extern _TUCHAR Prog_libRecentDir[_MAX_PATH];
extern _TUCHAR Prog_importRecentDir[_MAX_PATH];
extern _TUCHAR Prog_exportRecentDir[_MAX_PATH];
extern WINDOWPLACEMENT Prog_mainWndPlacement;
extern int Prog_mainWndHSplitterPos;
extern int Prog_mainWndZoomedHSplitterPos;
extern WINDOWPLACEMENT Prog_kybdDlgPlacement;
extern WINDOWPLACEMENT Prog_voiceDlgPlacement;
extern WINDOWPLACEMENT Prog_pfmDlgPlacement;
extern WINDOWPLACEMENT Prog_pcDlgPlacement;
extern WINDOWPLACEMENT Prog_mtoDlgPlacement;
extern WINDOWPLACEMENT Prog_mtfDlgPlacement;
extern MIDI *Prog_midi;
extern BOOL Prog_openPrevious;
extern BOOL Prog_startupEditBufferRequest;
extern BOOL Prog_showFullPaths;
extern BOOL Prog_dblClkingBankDoesntOpen;
extern BOOL Prog_mainWndToFront;
extern BOOL Prog_snapshotAutoSave;
extern BOOL Prog_libraryNoDuplicates;
extern BOOL Prog_unslashedZeros;
extern int Prog_tx81zVersion;
extern BOOL Prog_remoteStayOnTop;
extern BOOL Prog_kybdStayOnTop;
extern int Prog_kybdChannel;
extern BOOL Prog_fixedVelEnable;
extern int Prog_fixedVelValue;
extern DWORD Prog_rxOptions;
extern DWORD Prog_txOptions;
extern DWORD Prog_importOptions;
extern DWORD Prog_exportOptions;
extern DWORD Prog_diffView;

/*
 * Special LCD display string arrays
 */
extern _TUCHAR Prog_keyNameStrings[Prog_keyNameStringCnt][3];
extern _TUCHAR Prog_rcvChSettings[17][3];

/*
 * Other junk
 */
extern const _TUCHAR Prog_on[];
extern const _TUCHAR Prog_off[];
extern const _TUCHAR Prog_disabled[];
extern const _TUCHAR Prog_vMemStr[];
extern const _TUCHAR Prog_pMemStr[];
extern const _TUCHAR Prog_bndlStr[];
extern const _TUCHAR Prog_fileOpenErrorFmt[];
extern const _TUCHAR Prog_fileReadErrorFmt[];
extern const _TUCHAR Prog_fileWriteErrorFmt[];
extern const _TUCHAR Prog_allFileFilter[];
extern const _TUCHAR Prog_libFilter[];
extern const _TUCHAR Prog_libExt[];
extern const _TUCHAR Prog_syxFilter[];
extern const _TUCHAR Prog_syxExt[];
extern const _TUCHAR Prog_txtFilter[];
extern const _TUCHAR Prog_txtExt[];
extern const _TUCHAR Prog_untitled[];
extern const _TUCHAR Prog_zero[];
extern const _TUCHAR Prog_arStr[];
extern const _TUCHAR Prog_d1rStr[];
extern const _TUCHAR Prog_d1lStr[];
extern const _TUCHAR Prog_d2rStr[];
extern const _TUCHAR Prog_rrStr[];
extern const _TUCHAR Prog_shftStr[];
#ifdef PROG_LOGFILE
extern FILE *Prog_logFile;
#endif

#endif /* #ifdef DEFINE_GLOBALS */


#endif /* #ifndef PROG_H */
