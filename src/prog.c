/*
 * prog.c - TX81Z editor
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
#include "ctrl/algctrl.h"
#include "ctrl/egctrl.h"
#include "ctrl/kybdctrl.h"
#include "ctrl/lcdctrl.h"
#include "ctrl/rpanel.h"
#include "ctrl/txlbx.h"
#include "ctrl/voicemenu.h"
#include "dlg/fxdlg.h"
#include "dlg/kybddlg.h"
#include "dlg/mtfdlg.h"
#include "dlg/mtodlg.h"
#include "dlg/pcdlg.h"
#include "dlg/pfmdlg.h"
#include "dlg/voicedlg.h"
#include "gui/menubtn.h"
#include "wnd/mainwnd.h"
#include "wnd/remotewnd.h"
#include "resource.h"
#include "snapshot.h"
#include "tx81z.h"
#include "txlib.h"
#define DEFINE_GLOBALS
#include "prog.h"

/*
 * Data types
 */
typedef struct {
    HCURSOR *cursor;
    UINT cursorID;
} CURSORLOAD;

typedef struct {
    HBITMAP *bmp;
    UINT bmpID;
    int w;
    int h;
} BMPLOAD;

/*
 * Global procedures
 */
extern void Prog_ChangeControl(HWND ctrl, UINT parameter, void *valuePtr, CPSEND send, CHANGEPARAMFUNC ChangeParameter);
extern void Prog_CheckRegistration(_TUCHAR *userName, _TUCHAR *serial);
extern void Prog_CreateFonts(HWND wnd);
extern void Prog_DeleteFonts(void);
extern void Prog_OpenHelp(HWND wnd, _TUCHAR *topicUrl);
extern BOOL CALLBACK Prog_SetBoldFont(HWND dlgCtrl, LPARAM lParam);
extern void Prog_UpdateSysColors(void);

/*
 * Unit constants
 */
static const CURSORLOAD p_cursorLoads[] = {
    { &Prog_splitUpDownCursor, IDC_SPLITUPDOWN },
    { &Prog_splitDownCursor, IDC_SPLITDOWN },
    { &Prog_splitUpCursor, IDC_SPLITUP },
    { &Prog_splitLeftRightCursor, IDC_SPLITLEFTRIGHT },
    { &Prog_splitLeftCursor, IDC_SPLITLEFT },
    { &Prog_splitRightCursor, IDC_SPLITRIGHT },
    { &Prog_panCursor, IDC_PAN },
    { &Prog_fingerCursor, IDC_FINGER },
    { &Prog_linkCursor, IDC_LINK },
};
#define p_cursorLoadCnt ARRAYSIZE(p_cursorLoads)

static const BMPLOAD p_bmpLoads[] = {
    { &Prog_logoBmp, IDB_LOGO_BMP, Prog_logoWidth, Prog_logoHeight },
    { &Prog_fileBmp, IDB_FILE_BMP, 14, 14 },
    { &Prog_sortBmp, IDB_SORT_BMP, 14, 15 },
    { &Prog_copyLeftBmp, IDB_COPY_LEFT_BMP, 24, 15 },
    { &Prog_copyRightBmp, IDB_COPY_RIGHT_BMP, 24, 15 },
    { &Prog_deleteBmp, IDB_DELETE_BMP, 15, 15 },
    { &Prog_renameBmp, IDB_RENAME_BMP, 16, 15 },
    { &Prog_recommentBmp, IDB_RECOMMENT_BMP, 15, 15 },
    { &Prog_diffBmp, IDB_DIFF_BMP, 15, 15 },
    { &Prog_setBmp, IDB_SET_BMP, 24, 15 },
    { &Prog_placeBmp, IDB_PLACE_BMP, 24, 15 },
    { &Prog_unplaceBmp, IDB_UNPLACE_BMP, 15, 15 },
    { &Prog_disabledCopyLeftBmp, IDB_DISABLED_COPY_LEFT_BMP, 24, 15 },
    { &Prog_disabledCopyRightBmp, IDB_DISABLED_COPY_RIGHT_BMP, 24, 15 },
    { &Prog_disabledDeleteBmp, IDB_DISABLED_DELETE_BMP, 15, 15 },
    { &Prog_disabledRenameBmp, IDB_DISABLED_RENAME_BMP, 16, 15 },
    { &Prog_disabledRecommentBmp, IDB_DISABLED_RECOMMENT_BMP, 15, 15 },
    { &Prog_disabledDiffBmp, IDB_DISABLED_DIFF_BMP, 15, 15 },
    { &Prog_disabledSetBmp, IDB_DISABLED_SET_BMP, 24, 15 },
    { &Prog_disabledPlaceBmp, IDB_DISABLED_PLACE_BMP, 24, 15 },
    { &Prog_resetBmp, IDB_RESET_BMP, 40, 40 },
    { &Prog_storeBmp, IDB_STORE_BMP, 40, 40 },
    { &Prog_utilityBmp, IDB_UTILITY_BMP, 40, 40 },
    { &Prog_editCmpBmp, IDB_EDITCMP_BMP, 40, 40 },
    { &Prog_playPfmBmp, IDB_PLAYPFM_BMP, 40, 40 },
    { &Prog_leftArrowBmp, IDB_LEFT_ARROW_BMP, 40, 40 },
    { &Prog_rightArrowBmp, IDB_RIGHT_ARROW_BMP, 40, 40 },
    { &Prog_decBmp, IDB_DEC_BMP, 40, 40 },
    { &Prog_incBmp, IDB_INC_BMP, 40, 40 },
    { &Prog_cursorBmp, IDB_CURSOR_BMP, 40, 40 },
};
#define p_bmpLoadCnt ARRAYSIZE(p_bmpLoads)

static const _TUCHAR p_helpMsg[] = _T("\
TX81Z.EXE [/1 | /2 | /3 | /4 | /5 | /6 | /7 | /8 | /9]\n\
\n\
/1, /2, /3, /4, /5, /6, /7, /8, /9\n\
  --  A switch consisting of a single digit opens the editor with the\n\
      specified set of MIDI settings.  Only one set can be specified, so\n\
      only one digit switch can be present.\n");
#define p_helpMsgLen STRSIZE(p_helpMsg)

static const _TUCHAR *p_regSubkeys[10] = {
    NULL,
    _T("Set1"),
    _T("Set2"),
    _T("Set3"),
    _T("Set4"),
    _T("Set5"),
    _T("Set6"),
    _T("Set7"),
    _T("Set8"),
    _T("Set9"),
};
static const _TUCHAR p_helpPrefix[] = _T("tx81z.chm::/");
#define p_helpPrefixLen STRSIZE(p_helpPrefix)
static const _TUCHAR p_helpSitePrefix[] = _T("https://the-all.org/tx81z/programmer/docs.php?");
#define p_helpSitePrefixLen STRSIZE(p_helpSitePrefix)
#define p_helpPathMax (_MAX_PATH * 2)
#define p_helpSiteMax 1024

/*
 * Unit procedures
 */
static int CALLBACK p_EnumFontProc(const LOGFONT *lplf, const TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData);
static void p_InitSpecialArrays(void);
static int p_ParseCmdLine(char *cmdLine);

/*
 * Unit variables
 */
static _TUCHAR p_helpPath[p_helpPathMax];
static int p_helpPathLen;
static int p_regSet;
static BOOL p_tahomaExists;

/*
 * Procedure definitions
 */

/*
 * Program entry point
 */
int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInst, LPSTR cmdLine,
        int cmdShow)
{
    INITCOMMONCONTROLSEX initCommonControlStruct = {
        sizeof initCommonControlStruct,
        ICC_WIN95_CLASSES
    };
    MSG msg = {0};
    DWORD error;
    int i;
#ifdef _DEBUG
    // Get current flag
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;

    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );
    //_CrtSetBreakAlloc(2160);
#endif
#ifdef PROG_LOGFILE
    /*
     * Open the log file.
     */
    Prog_logFile = fopen("prog_logfile.txt", "w");
    if (!Prog_logFile) {
        MessageBox(NULL, _T("Cannot open prog_logfile.txt"), NULL, MB_OK);
        return EXIT_FAILURE;
    }
#endif
    /*
     * Parse command line options.
     */
    if (p_ParseCmdLine(cmdLine)) {
        return EXIT_FAILURE;
    }
    /*
     * Save the program instance handle.
     */
    Prog_instance = instance;
    /*
     * Get the path to the help file.
     */
    p_helpPathLen = GetModuleFileName(instance, p_helpPath, _MAX_PATH);
    while (p_helpPath[p_helpPathLen] != '\\') {
        p_helpPathLen--;
    }
    p_helpPath[++p_helpPathLen] = '\0';
    _tcscpy(&p_helpPath[p_helpPathLen], p_helpPrefix);
    p_helpPathLen += p_helpPrefixLen;
    /*
     * Set up error handlers until the main window gets created.
     */
    Error_GuiInit(HWND_DESKTOP);
    /*
     * Create MIDI object - needs to be done before the registry is read.
     */
    if (!(Prog_midi = Midi_Create(HWND_DESKTOP, 512, 32))) {
        return EXIT_FAILURE;
    }
    /*
     * Set up and load registry settings.
     */
    Prog_registry = Registry_Create(Prog_name);
    /*
     * Recent file lists
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Snapshot Recent 1"), REG_SZ, Prog_snapshotRecent[0]
            , sizeof Prog_snapshotRecent[0]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Snapshot Recent 2"), REG_SZ, Prog_snapshotRecent[1]
            , sizeof Prog_snapshotRecent[1]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Snapshot Recent 3"), REG_SZ, Prog_snapshotRecent[2]
            , sizeof Prog_snapshotRecent[2]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Snapshot Recent 4"), REG_SZ, Prog_snapshotRecent[3]
            , sizeof Prog_snapshotRecent[3]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 1 Recent 1"), REG_SZ, Prog_libRecent[0][0]
            , sizeof Prog_libRecent[0][0]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 1 Recent 2"), REG_SZ, Prog_libRecent[0][1]
            , sizeof Prog_libRecent[0][1]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 1 Recent 3"), REG_SZ, Prog_libRecent[0][2]
            , sizeof Prog_libRecent[0][2]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 1 Recent 4"), REG_SZ, Prog_libRecent[0][3]
            , sizeof Prog_libRecent[0][3]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 2 Recent 1"), REG_SZ, Prog_libRecent[1][0]
            , sizeof Prog_libRecent[1][0]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 2 Recent 2"), REG_SZ, Prog_libRecent[1][1]
            , sizeof Prog_libRecent[1][1]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 2 Recent 3"), REG_SZ, Prog_libRecent[1][2]
            , sizeof Prog_libRecent[1][2]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Lib 2 Recent 4"), REG_SZ, Prog_libRecent[1][3]
            , sizeof Prog_libRecent[1][3]);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Snapshot Recent Dir"), REG_SZ, Prog_snapshotRecentDir
            , sizeof Prog_snapshotRecentDir);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Library Recent Dir"), REG_SZ, Prog_libRecentDir
            , sizeof Prog_libRecentDir);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Import Recent Dir"), REG_SZ, Prog_importRecentDir
            , sizeof Prog_importRecentDir);
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("Export Recent Dir"), REG_SZ, Prog_exportRecentDir
            , sizeof Prog_exportRecentDir);
    /*
     * Window positions
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , MainWnd_className, REG_BINARY, &Prog_mainWndPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("MainWnd HSplitter"), REG_DWORD
            , &Prog_mainWndHSplitterPos, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , _T("MainWnd Zoomed HSplitter"), REG_DWORD
            , &Prog_mainWndZoomedHSplitterPos, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, p_regSubkeys[p_regSet]
            , KybdDlg_className, REG_BINARY, &Prog_kybdDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, p_regSubkeys[p_regSet]
            , VoiceDlg_className, REG_BINARY, &Prog_voiceDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_PFMDLG, p_regSubkeys[p_regSet]
            , PfmDlg_className, REG_BINARY, &Prog_pfmDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_PCDLG, p_regSubkeys[p_regSet]
            , PcDlg_className, REG_BINARY, &Prog_pcDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_MTODLG, p_regSubkeys[p_regSet]
            , MtoDlg_className, REG_BINARY, &Prog_mtoDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    Registry_AddEntry(Prog_registry, REG_MTFDLG, p_regSubkeys[p_regSet]
            , MtfDlg_className, REG_BINARY, &Prog_mtfDlgPlacement
            , sizeof(WINDOWPLACEMENT));
    /*
     * MIDI settings
     */
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("In Port"), REG_SZ, &Prog_midi->inPortName
            , sizeof Prog_midi->inPortName);
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("In Channel"), REG_DWORD, &Prog_midi->inChannel
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("Out Port"), REG_SZ, &Prog_midi->outPortName
            , sizeof Prog_midi->outPortName);
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("Out Channel"), REG_DWORD, &Prog_midi->outChannel
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("Master In Port"), REG_SZ, &Prog_midi->masterInPortName
            , sizeof Prog_midi->masterInPortName);
    /*
     * Program options
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Open Previous"), REG_DWORD, &Prog_openPrevious
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Startup Edit Buffer Request"), REG_DWORD, &Prog_startupEditBufferRequest
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Show Full Paths"), REG_DWORD, &Prog_showFullPaths
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Snapshot Double Click"), REG_DWORD, &Prog_dblClkingBankDoesntOpen
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("MainWnd To Front"), REG_DWORD, &Prog_mainWndToFront
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Highlight Color"), REG_DWORD, &Prog_hiColor
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Custom Colors"), REG_BINARY, &Prog_custColors
            , sizeof(Prog_custColors));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Snapshot Autosave"), REG_DWORD, &Prog_snapshotAutoSave
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Disallow Library Duplicates"), REG_DWORD
            , &Prog_libraryNoDuplicates, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("Unslashed Zeros"), REG_DWORD, &Prog_unslashedZeros
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("TX81Z Version"), REG_DWORD, &Prog_tx81zVersion
            , sizeof(DWORD));
    /*
     * Remote control window options
     */
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("Remote Stay On Top"), REG_DWORD, &Prog_remoteStayOnTop
            , sizeof(DWORD));
    /*
     * Keyboard window options
     */
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("Keyboard Stay On Top"), REG_DWORD, &Prog_kybdStayOnTop
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, p_regSubkeys[p_regSet]
            , _T("Keyboard Channel"), REG_DWORD, &Prog_kybdChannel
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("Fixed Velocity Enable"), REG_DWORD, &Prog_fixedVelEnable
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_PROG, NULL
            , _T("Fixed Velocity Value"), REG_DWORD, &Prog_fixedVelValue
            , sizeof(DWORD));
    /*
     * Retrieve and transmit dialog options
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Rx Options"), REG_DWORD, &Prog_rxOptions
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Tx Options"), REG_DWORD, &Prog_txOptions
            , sizeof(DWORD));
    /*
     * Import and export options
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Import Options"), REG_DWORD, &Prog_importOptions
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Export Options"), REG_DWORD, &Prog_exportOptions
            , sizeof(DWORD));
    /*
     * Diff window options
     */
    Registry_AddEntry(Prog_registry, REG_MAINWND, NULL
            , _T("Diff View"), REG_DWORD, &Prog_diffView
            , sizeof(DWORD));
    /*
     * Voice editor options
     */
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, NULL
            , _T("One Click Wave Selection"), REG_DWORD
            , &VoiceDlg_fastWaveSelection, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, NULL
            , _T("Simple Frequency Selection"), REG_DWORD
            , &VoiceDlg_simpleFreqSelection, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, NULL
            , _T("Hide EG Labels"), REG_DWORD, &VoiceDlg_hideEGLabels
            , sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, NULL
            , _T("Hide Voice Editor Scroll Bars"), REG_DWORD
            , &VoiceDlg_hideScrollBars, sizeof(DWORD));
    Registry_AddEntry(Prog_registry, REG_VOICEDLG, NULL
            , _T("Mouse Wheel"), REG_DWORD
            , &VoiceDlg_mouseWheel, sizeof(DWORD));
    /*
     * Read'em in
     */
    if (error = Registry_ReadGroup(Prog_registry, REGISTRY_ALL_GROUPS, TRUE)) {
        MsgBox_Error(NULL, error);
    }
    /*
     * Load resources.
     */
    Prog_icon = LoadIcon(instance, MAKEINTRESOURCE(IDI_PROGICON + p_regSet));
    Prog_accels = LoadAccelerators(instance, MAKEINTRESOURCE(IDA_PROGACCEL));
    Prog_arrowCursor = LoadCursor(NULL, IDC_ARROW);
    Prog_waitCursor = LoadCursor(NULL, IDC_WAIT);
    for (i = 0; i < p_cursorLoadCnt; i++) {
        *p_cursorLoads[i].cursor = LoadCursor(instance
                , MAKEINTRESOURCE(p_cursorLoads[i].cursorID));
    }
    Prog_snapshotFileMenu = GetSubMenu(LoadMenu(instance
                , MAKEINTRESOURCE(IDM_SNAPSHOT_FILE_MENU)), 0);
    Prog_libFileMenu = GetSubMenu(LoadMenu(instance
                , MAKEINTRESOURCE(IDM_LIB_FILE_MENU)), 0);
    Prog_libSortMenu = GetSubMenu(LoadMenu(instance
                , MAKEINTRESOURCE(IDM_LIB_SORT_MENU)), 0);
    Prog_voiceQuickEditMenu = GetSubMenu(LoadMenu(instance
                , MAKEINTRESOURCE(IDM_VOICE_QUICKEDIT_MENU)), 0);
    Prog_pfmQuickEditMenu = GetSubMenu(LoadMenu(instance
                , MAKEINTRESOURCE(IDM_PFM_QUICKEDIT_MENU)), 0);
    /*
     * Cache GDI objects.
     */
    Prog_unloadedColor = LTGRAY;
    Prog_ltGrayBrush = GetStockObject(LTGRAY_BRUSH);
    Prog_whiteBrush = GetStockObject(WHITE_BRUSH);
    Prog_blackPen = GetStockObject(BLACK_PEN);
    Prog_ltGrayPen = CreatePen(PS_SOLID, 1, LTGRAY);
    Prog_whitePen = GetStockObject(WHITE_PEN);
    Prog_hSliderH = GetSystemMetrics(SM_CYHSCROLL);
    Prog_vSliderW = GetSystemMetrics(SM_CXVSCROLL);
    Prog_UpdateSysColors();
    /*
     * Init TX list box colors.
     */
    TxLbx_InitColors();
    /*
     * Init LCD display string and other arrays.
     */
    p_InitSpecialArrays();
    /*
     * Init common control libs.
     */
    InitCommonControlsEx(&initCommonControlStruct);
    /*
     * Initialize random number seed.
     */
    Randomize();
    /*
     * Register window classes.
     */
    if (!MainWnd_Register()
        || !AlgCtrl_Register()
        || !EGCtrl_Register()
        || !LcdCtrl_Register()
        || !RPanel_Register()
        || !TX81Z_Register()
        || !KybdCtrl_Register()
        || !RemoteWnd_Register()
        || !VoiceMenu_Register())
    {
        return EXIT_FAILURE;
    }
    /*
     * Create the main window.
     */
    if ((Prog_mainWnd = MainWnd_Create()) == NULL) {
        return EXIT_FAILURE;
    }
    /*
     * Set up the main error handler.
     */
    Error_GuiInit(Prog_mainWnd);
    /*
     * Main message loop
     */
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (!TranslateAccelerator(Prog_activeWnd, Prog_accels, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    /*
     * Free resources and exit.
     */
    Registry_WriteGroup(Prog_registry, REGISTRY_ALL_GROUPS, TRUE);
    Registry_Destroy(Prog_registry);
    Midi_Destroy(Prog_midi);
    LcdCtrl_Deinit();
    DeletePen(Prog_dlgPen);
    DeletePen(Prog_grayTextPen);
    DeletePen(Prog_hiPen);
    DeletePen(Prog_ltGrayPen);
    DeletePen(Prog_selTextPen);
    DeletePen(Prog_wndPen);
    DeletePen(Prog_wndTextPen);
    DeleteBrush(Prog_hiBrush);
    Prog_DeleteFonts();
    for (i = 0; i < p_bmpLoadCnt; i++) {
        DeleteBitmap(*p_bmpLoads[i].bmp);
    }
#ifdef PROG_LOGFILE
    fclose(Prog_logFile);
#endif

    return msg.wParam;
}

/*
 * ChangeControl() - Does automated editing by manipulating a control.
 */
void Prog_ChangeControl(HWND ctrl, UINT parameter, void *valuePtr, CPSEND send
        , CHANGEPARAMFUNC ChangeParameter)
{
    _TUCHAR className[30];
    int value;

    /*
     * Figure out what type of control ctrlID refers to.
     */
    GetClassName(ctrl, className, 30);
    if (StriEq(className, LcdCtrl_className)) {
        if (GetWindowStyle(ctrl) & LCS_ANY_SB) {
            int min = LcdCtrl_GetRangeMin(ctrl);

            value = *(int *) valuePtr;
            if (value < min) {
                value = min;
            } else {
                int max = LcdCtrl_GetRangeMax(ctrl);

                if (value >= max) {
                    value = max;
                }
            }
            LcdCtrl_SetValue(ctrl, value);
        } else {
            size_t i, size;
            char text[80];

            LcdCtrl_GetText(ctrl, text);
            if (parameter == TX81Z_PCED_NAME) {
                size = TX81Z_NAME_LEN;
            } else if (parameter == TX81Z_SYS_STARTUP_MSG) {
                size = TX81Z_STARTUP_MSG_LEN;
            } else {
                assert(0);
            }
            for (i = 0; i < size; i++) {
                char c = ((char *) valuePtr)[i];

                if (c != text[i]) {
                    ChangeParameter(parameter + i, c, send);
                    LcdCtrl_SetCursorPos(ctrl, i);
                }
            }
            LcdCtrl_SetText(ctrl, (char *) valuePtr);
            return;
        }
    } else if (StriEq(className, Prog_comboBoxClassName)) {
        value = *(int *) valuePtr;
        ComboBox_SetCurSel(ctrl, value);
    } else if (StriEq(className, Prog_buttonClassName)) {
        value = *(int *) valuePtr;
        if (value != Button_IsChecked(ctrl)) {
            Button_Click(ctrl);
        }
    }
    /*
     * Send the parameter change message.
     */
    ChangeParameter(parameter, value, send);
}

/*
 * CreateFonts() - Creates fonts to use for dialog controls.
 */
void Prog_CreateFonts(HWND wnd)
{
    LOGFONT logFont = { 0 };
    HDC dC = GetDC(wnd);

    /*
     * Return if the fonts have already been initialized (when the splash
     * screen module initialized them and the main window module is now calling
     * this function).
     */
    if (Prog_tahomaFont) {
        return;
    }
    logFont.lfHeight = -MulDiv(8, GetDeviceCaps(dC, LOGPIXELSY), 72);
    _tcsncpy(logFont.lfFaceName, _T("Tahoma"), LF_FACESIZE);
    EnumFontFamiliesEx(dC, &logFont, p_EnumFontProc, 0, 0);
    if (!p_tahomaExists) {
        _tcsncpy(logFont.lfFaceName, _T("MS Sans Serif"), LF_FACESIZE);
    }
    ReleaseDC(wnd, dC);
    /*
     * Create a regular version of the font.
     */
    Prog_tahomaFont = CreateFontIndirect(&logFont);
    /*
     * Create a bold version.
     */
    logFont.lfWeight = FW_BOLD;
    Prog_tahomaBoldFont = CreateFontIndirect(&logFont);
    /*
     * Create an underline version.
     */
    logFont.lfWeight = FW_NORMAL;
    logFont.lfUnderline = TRUE;
    Prog_tahomaUnderlineFont = CreateFontIndirect(&logFont);
}

/*
 * DeleteFonts()
 */
void Prog_DeleteFonts(void)
{
    DeleteFont(Prog_tahomaFont);
    DeleteFont(Prog_tahomaBoldFont);
    DeleteFont(Prog_tahomaUnderlineFont);
}

/*
 * OpenHelp() - Opens the help file to a topic.
 */
void Prog_OpenHelp(HWND wnd, _TUCHAR *topicUrl)
{
    _tcsncpy(&p_helpPath[p_helpPathLen], topicUrl, p_helpPathMax);
    /*
     * Just go to the help page on the website if the help file is missing
     */
    if (!HtmlHelp(NULL, p_helpPath, HH_DISPLAY_TOPIC, 0)) {
        _TUCHAR helpSite[p_helpSiteMax];
        _tcsncpy(helpSite, p_helpSitePrefix, p_helpSiteMax);
        _tcsncat(helpSite, topicUrl, p_helpSiteMax);
        ShellExecute(NULL, _T("open"), helpSite, NULL, NULL, SW_SHOW);
    }
}

/*
 * SetBoldFont() - Callback to set controls to bold font.
 */
BOOL CALLBACK Prog_SetBoldFont(HWND dlgCtrl, LPARAM lParam)
{
    UINT ctrlID = GetWindowLong(dlgCtrl, GWL_ID);

    if (ctrlID >= 6000 && ctrlID <= 7000) {
        SetWindowFont(dlgCtrl, Prog_tahomaBoldFont, FALSE);
    }
    return TRUE;
}

/*
 * UpdateSysColors() - Updates system color cache after a system settings
 *                     change.
 */
void Prog_UpdateSysColors(void)
{
    int i;

    for (i = 0; i < p_bmpLoadCnt; i++) {
        DeleteBitmap(*p_bmpLoads[i].bmp);
        *p_bmpLoads[i].bmp = LoadImage(Prog_instance
                , MAKEINTRESOURCE(p_bmpLoads[i].bmpID)
                , IMAGE_BITMAP
                , p_bmpLoads[i].w
                , p_bmpLoads[i].h
                , LR_LOADMAP3DCOLORS);
    }
    Prog_dirtyColor = Prog_hiColor;
    Prog_3dHighlightColor = GetSysColor(COLOR_3DHIGHLIGHT);
    Prog_3dLightColor = GetSysColor(COLOR_3DLIGHT);
    Prog_3dShadowColor = GetSysColor(COLOR_3DSHADOW);
    Prog_dlgColor = GetSysColor(COLOR_3DFACE);
    Prog_dlgTextColor = GetSysColor(COLOR_BTNTEXT);
    Prog_grayTextColor = GetSysColor(COLOR_GRAYTEXT);
    Prog_selColor = GetSysColor(COLOR_HIGHLIGHT);
    Prog_selTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
    Prog_infoTextColor = GetSysColor(COLOR_INFOTEXT);
    Prog_wndColor = GetSysColor(COLOR_WINDOW);
    Prog_wndTextColor = GetSysColor(COLOR_WINDOWTEXT);

    Prog_dlgBrush = GetSysColorBrush(COLOR_3DFACE);
    Prog_selBrush = GetSysColorBrush(COLOR_HIGHLIGHT);
    Prog_selTextBrush = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
    Prog_infoBkBrush = GetSysColorBrush(COLOR_INFOBK);
    Prog_wndBrush = GetSysColorBrush(COLOR_WINDOW);
    Prog_wndTextBrush = GetSysColorBrush(COLOR_WINDOWTEXT);

    if (Prog_hiBrush) DeleteBrush(Prog_hiBrush);
    Prog_hiBrush = CreateSolidBrush(Prog_hiColor);

    if (Prog_3dLightPen) DeletePen(Prog_3dLightPen);
    Prog_3dLightPen = CreatePen(PS_SOLID, 1, Prog_3dLightColor);

    if (Prog_3dShadowPen) DeletePen(Prog_3dShadowPen);
    Prog_3dShadowPen = CreatePen(PS_SOLID, 1, Prog_3dShadowColor);

    if (Prog_dlgPen) DeletePen(Prog_dlgPen);
    Prog_dlgPen = CreatePen(PS_SOLID, 1, Prog_dlgColor);

    if (Prog_grayTextPen) DeletePen(Prog_grayTextPen);
    Prog_grayTextPen = CreatePen(PS_SOLID, 1, Prog_grayTextColor);

    if (Prog_hiPen) DeletePen(Prog_hiPen);
    Prog_hiPen = CreatePen(PS_SOLID, 1, Prog_hiColor);

    if (Prog_selTextPen) DeletePen(Prog_selTextPen);
    Prog_selTextPen = CreatePen(PS_SOLID, 1, Prog_selTextColor);

    if (Prog_wndPen) DeletePen(Prog_wndPen);
    Prog_wndPen = CreatePen(PS_SOLID, 1, Prog_wndColor);

    if (Prog_wndTextPen) DeletePen(Prog_wndTextPen);
    Prog_wndTextPen = CreatePen(PS_SOLID, 1, Prog_wndTextColor);
    
    /*
     * Update the colors on the voice menu in the performance editor.
     */
    PfmDlg_Update(Prog_pfmDlg, NULL);
}

/*
 * EnumFontProc()
 */
int CALLBACK p_EnumFontProc(const LOGFONT *lplf, const TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData)
{
    p_tahomaExists = TRUE;
    return 0;
}

/*
 * InitSpecialArrays() - Initializes arrays of strings for special LCD's and
 *                       other information displays.
 */
void p_InitSpecialArrays(void)
{
    _TUCHAR octaveNumberChars[11] = {
        (_TUCHAR) 0x8A, (_TUCHAR) 0x89, '0', '1', '2', '3', '4', '5', '6', '7', '8'
    };
    _TUCHAR voiceNumChars[5] = {
        'I', 'A', 'B', 'C', 'D'
    };
    _TUCHAR text[6];
    int octave, note;
    BOOL isBlack;
    int range, crs, fine;
    int i, n;

    /*
     * Key name strings.  Used for the transpose LCD in the voice editor; the
     * low limit, high limit LCD's in the performance editor; the chord LCD's
     * in the effects editor; and the coarse frequency sliders in the
     * microtune editors.
     */
    for (i = 0; i < Prog_keyNameStringCnt; i++) {
        octave = i / 12;
        note = i % 12;
        isBlack = (note - (note >= 5)) & 1;
        note = (((note + (note >= 5)) >> 1) + 2) % 7 + 'A';
        Prog_keyNameStrings[i][0] = isBlack ? note : ' ';
        Prog_keyNameStrings[i][1] = isBlack ? '#' : note;
        Prog_keyNameStrings[i][2] = octaveNumberChars[octave];
    }
    /*
     * Receive channel settings.  Used for the performance instrument channels
     * and the basic receive and transmit channels.
     */
    for (i = 0; i < 16; i++) {
        Prog_rcvChSettings[i][0] = ' ';
        Prog_rcvChSettings[i][1] = (i + 1) >= 10 ? '1' : ' ';
        Prog_rcvChSettings[i][2] = (i + 1) % 10 + '0';
    }
    _tcsncpy(Prog_rcvChSettings[16], _T("omn"), 3);
    /*
     * Voice number strings for the voice number LCD's in the performance editor.
     */
    for (i = 0; i < 5; i++) {
        for (n = 0; n < 32; n++) {
            PfmDlg_voiceNumSettings[i * 32 + n][0] = voiceNumChars[i];
            PfmDlg_voiceNumSettings[i * 32 + n][1] = '0' + (n + 1) / 10;
            PfmDlg_voiceNumSettings[i * 32 + n][2] = '0' + (n + 1) % 10;
        }
    }

    /*
     * Delay time LCD in effects editor.
     */
    for (i = 0; i < 128; i++) {
        n = i + 1;
        FxDlg_delayTimeLcdStrings[i][0] = '0' + (n > 99);
        FxDlg_delayTimeLcdStrings[i][1] = '.';
        FxDlg_delayTimeLcdStrings[i][2] = n / 10 % 10 + '0';
        FxDlg_delayTimeLcdStrings[i][3] = n % 10 + '0';
        FxDlg_delayTimeLcdStrings[i][4] = 's';
    }

    /*
     * Initialize the fixed frequency sort array.
     */
    i = 0;
    for (range = 0; range < 8; range++) {
        for (crs = 0; crs < 64; crs += 4) {
            for (fine = 0; fine < (crs == 0 ? 8 : 16); fine++) {
                VoiceDlg_fixedFreqSort[i].range = range;
                VoiceDlg_fixedFreqSort[i].crs = crs;
                VoiceDlg_fixedFreqSort[i].fine = fine;
                VoiceDlg_fixedFreqSort[i].freq
                        = VoiceDlg_FixedParamsToFreq(range, crs, fine);
                i++;
            }
        }
    }
    qsort(VoiceDlg_fixedFreqSort, 1984, sizeof(FIXEDFREQSORT)
            , VoiceDlg_FixedFreqSortCmp);

    /*
     * Initialize the ratio simple frequency settings.
     */
    for (i = 0; i < 992; i++) {
        n = VoiceDlg_freqRatioSort[i][0] * 16 + VoiceDlg_freqRatioSort[i][1];
        _tcsncpy(VoiceDlg_simpleRatioSettings[i], VoiceDlg_freqRatios[n], 5);
    }
    /*
     * Initialize the fixed simple frequency settings.
     */
    for (i = 0; i < 1984; i++) {
        _stprintf(text, _T("%05d"), VoiceDlg_fixedFreqSort[i].freq);
        _tcsncpy(VoiceDlg_simpleFixedSettings[i], text, 5);
    }
}

/*
 * ParseCmdLine() - Parses command line options.
 */
int p_ParseCmdLine(char *cmdLine)
{
    int i = 0;
    char c = cmdLine[i];
    BOOL parseOption = FALSE;
    BOOL parseSnapshotName = FALSE;
    BOOL parseLibName = FALSE;

    while (c) {
        if (parseOption) {
            if (c >= '1' && c <= '9') {
                if (p_regSet == 0) {
                    p_regSet = c - '0';
                } else {
                    MsgBox_F(NULL
                            , _T("Only one MIDI set switch can be specified"));
                    goto ShowHelp;
                }
            } else {
ShowHelp:
                MessageBox(NULL, p_helpMsg
                        , _T("TX81Z Command Line Options"), MB_OK);
                return 1;
            }
        } else if (parseSnapshotName) {
        } else if (parseLibName) {
        } else if (c == '/' || c == '-') {
            parseOption = TRUE;
        }
        c = cmdLine[++i];
    }
    return 0;
}

