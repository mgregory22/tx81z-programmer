/*
 * mainwnd.c - main window class
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
#include "aboutdlg.h"
#include "bndldlg.h"
#include "datadlg.h"
#include "diffdlg.h"
#include "exportdlg.h"
#include "fxdlg.h"
#include "importdlg.h"
#include "kybddlg.h"
#include "mainwnd.h"
#include "menubtn.h"
#include "minifont.h"
#include "mtfdlg.h"
#include "mtodlg.h"
#include "pcdlg.h"
#include "pfmdlg.h"
#include "prog.h"
#include "recommentdlg.h"
#include "remotewnd.h"
#include "renamedlg.h"
#include "resource.h"
#include "searchdlg.h"
#include "snapshot.h"
#include "storedlg.h"
#include "sysdlg.h"
#include "tx81z.h"
#include "txlbx.h"
#include "txpack.h"
#include "voicedlg.h"

/*
 * Global constants
 */
const _TUCHAR *MainWnd_className = _T("MainWnd");

/*
 * Global procedures
 */
extern void MainWnd_AddToLib(HWND mainWnd, int libIdx, const int *sIdxs, int sIdxCnt);
extern void MainWnd_BundleToLib(HWND mainWnd, int libIdx, const int *sIdxs, int sIdxCnt);
extern HWND MainWnd_Create(void);
extern BOOL MainWnd_Register(void);
extern BOOL MainWnd_StoreItem(int srcItem, int destItem);

/*
 * Unit types
 */
/* WF_LIB1 and WF_LIB2 are used as indexes into Prog_lib[] */
typedef enum {
    WF_LIB1     = 0,
    WF_LIB2     = 1,
    WF_SNAPSHOT = 2
} WHICHFILE;

typedef enum {
    UL_TBX = 0x01,
    UL_LBX = 0x02,
    UL_ALL = 0x03,
} UPDATELBX;

typedef enum {
    SELECT_NONE,
    SELECT_PREVIOUS,
    SELECT_UPDATED
} SELECTWHAT;

typedef struct {
    HBITMAP *bmp;
    HBITMAP *disabledBmp;
    int bmpWidth;
    _TUCHAR *text;
    size_t textLen;
    int textX;
} DRAWBTN;

/*
 * Unit constants
 */
#define SS_LBX_COL1_X 28
#define mw_ssLbxDivCnt 1
static int mw_ssDivPos[mw_ssLbxDivCnt] = { SS_LBX_COL1_X };
static TXLBX_PROPERTIES mw_ssTXProps = { mw_ssLbxDivCnt, mw_ssDivPos };

#define LIB_LBX_COL1_X  28
#define LIB_LBX_COL2_X  94
#define mw_libLbxDivCnt 2
static int mw_libDivPos[mw_libLbxDivCnt] = { LIB_LBX_COL1_X, LIB_LBX_COL2_X };
static TXLBX_PROPERTIES mw_libTXProps = { mw_libLbxDivCnt, mw_libDivPos };

#define mw_multiFileNameLen 4096

/* kludge for calling TX81Z_ProgramChange() with the item type instead of the
 * bank number */
static const BYTE mw_typeToBank[] = {
    -1,             /* META_VCED */
    -1,             /* META_PCED */
    -1,             /* META_FX */
    -1,             /* META_PC */
    -1,             /* META_MTO */
    -1,             /* META_MTF */
    -1,             /* META_SYS */
    TX81Z_BANK_I,   /* META_VMEM */
    TX81Z_BANK_PF,  /* META_PMEM */
    TX81Z_BANK_A,   /* META_PRESET_A */
    TX81Z_BANK_B,   /* META_PRESET_B */
    TX81Z_BANK_C,   /* META_PRESET_C */
    TX81Z_BANK_D    /* META_PRESET_D */
};
static const DRAWBTN mw_drawBtns[6] = {
    { &Prog_deleteBmp, &Prog_disabledDeleteBmp, 15, _T("Delete"), 6, 32 },
    { &Prog_renameBmp, &Prog_disabledRenameBmp, 16, _T("Rename"), 6, 27 },
    { &Prog_recommentBmp, &Prog_disabledRecommentBmp, 15, _T("Recomment"), 9, 19 },
    { &Prog_diffBmp, &Prog_disabledDiffBmp, 15, _T("Compare"), 7, 26 },
    { &Prog_copyLeftBmp, &Prog_disabledCopyLeftBmp, 24, _T("Copy"), 4, 34 },
    { &Prog_copyRightBmp, &Prog_disabledCopyRightBmp, 24, _T("Copy"), 4, 34 },
};
#define DRAWBTN_COPY_LEFT_INDEX 4
#define DRAWBTN_COPY_RIGHT_INDEX 5
const _TUCHAR mw_libDupeMsg[] = _T("Some items were not added because they are ")
            _T("duplicates of existing items.");
const _TUCHAR mw_libDupeTitle[] = _T("Duplicate Warning");
const _TUCHAR *mw_optionMenuHeaders[] = {
    _T("Startup Options:"),
    _T("Main Window Options:"),
    _T("Snapshot Options:"),
    _T("Library Options:"),
    _T("Global Options:"),
    _T("TX81Z Version:"),
};
#define mw_optionMenuHeaderCnt ARRAYSIZE(mw_optionMenuHeaders)

/*
 * Unit procedures
 */
static void mw_AddRecentFile(_TUCHAR recentList[Prog_recentFileMax][_MAX_PATH], _TUCHAR fileName[_MAX_PATH]);
static void mw_ChooseColor(HWND mainWnd);
static void mw_CleanLbx(HWND listBox);
static BOOL mw_ConfirmBankTransfer(HWND mainWnd, REQUEST reqFlags, BOOL retrieve);
static void mw_CopyFromLib(HWND mainWnd);
static void mw_CopyLibItems(HWND mainWnd);
static BOOL mw_DeleteLibItems(HWND mainWnd);
static BOOL mw_DirtyPrompt(HWND mainWnd, WHICHFILE whichFile);
static int mw_DoesUserWantToSave(HWND mainWnd, WHICHFILE whichFile);
static void mw_DrawEdges(HDC dC);
static void mw_EditItem(HWND mainWnd, BOOL listBoxDblClk);
static void mw_EraseEdges(HDC dC);
static void mw_ExportLib(HWND mainWnd, int libIndex);
static void mw_GetSelItems(HWND listBox);
static void mw_ImportLib(HWND mainWnd, int libIndex);
static void mw_InitMenuButtons(void);
static BOOL mw_NewLib(HWND mainWnd, int libIndex);
static BOOL mw_NewSnapshot(HWND mainWnd);
static void mw_OnActivate(HWND mainWnd, UINT state, HWND otherWnd, BOOL minimized);
static void mw_OnClose(HWND mainWnd);
static void mw_OnCommand(HWND mainWnd, UINT ctrlID, HWND ctrl, UINT notify);
static BOOL mw_OnCreate(HWND mainWnd, LPCREATESTRUCT createStruct);
static HBRUSH mw_OnCtlColor(HWND mainWnd, HDC dC, HWND childWnd, int type);
static void mw_OnDestroy(HWND mainWnd);
static void mw_OnDrawItem(HWND mainWnd, const DRAWITEMSTRUCT *drawItem);
static BOOL mw_OnEraseBkgnd(HWND mainWnd, HDC dC);
static void mw_OnLButtonDown(HWND mainWnd, BOOL dblClick, int x, int y, UINT keyFlags);
static void mw_OnLButtonUp(HWND mainWnd, int x, int y, UINT keyFlags);
static void mw_OnMeasureItem(HWND mainWnd, MEASUREITEMSTRUCT *measureItem);
static void mw_OnMouseMove(HWND mainWnd, int x, int y, UINT keyFlags);
static BOOL mw_OnSetCursor(HWND mainWnd, HWND cursorWnd, UINT codeHitTest, UINT msg);
static void mw_OnSize(HWND mainWnd, UINT state, int cx, int cy);
static void mw_OnSizing(HWND mainWnd, int edge, RECT *rect);
static void mw_OnSysColorChange(HWND mainWnd);
static BOOL mw_OpenLibraryFile(HWND mainWnd, int libIndex, const _TUCHAR *fileName);
static BOOL mw_OpenSnapshotFile(HWND mainWnd, const _TUCHAR *fileName, BOOL isCopy);
static BOOL mw_RetrieveAndUpdate(REQUEST reqFlags);
static void mw_SaveFile(HWND mainWnd, WHICHFILE whichFile, BOOL showDlg);
static void mw_SaveSnapshotCopy(HWND mainWnd);
static void mw_SendLibItem(HWND mainWnd, int libIndex);
static void mw_SnapshotRxData(HWND mainWnd);
static void mw_SnapshotTxData(HWND mainWnd);
static void mw_SnapshotRxItems(HWND mainWnd);
static void mw_SnapshotTxItems(HWND mainWnd);
static void mw_SortLib(HWND mainWnd, int libIndex, TXLIB_SORT sort);
static void mw_UpdateButtonStates(void);
static void mw_UpdateCopyFromLibBtn(void);
static void mw_UpdateEditors(REQUEST reqFlags);
static void mw_UpdateLibLbx(int libIndex, UPDATELBX update);
static void mw_UpdateRecentMenu(HMENU menu, _TUCHAR recentList[Prog_recentFileMax][_MAX_PATH], int originalItemCnt, _TUCHAR *curPath, UINT firstItemID);
static void mw_UpdateSnapshotLbx(UPDATELBX update);
static void mw_UpdateSnapshotLbxItem(BOOL notifyEditorDlg, SINDEX sIdx, SELECTWHAT selectWhat);
static LRESULT CALLBACK mw_WndProc(HWND mainWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HMENU mw_menu;
static int mw_optionMenuHeaderWidth;
static HCURSOR mw_cursor;
static RECT mw_clientRect;
static HWND mw_snapshotLbl;
static HWND mw_snapshotTbx;
static HWND mw_snapshotFileBtn;
static HWND mw_snapshotSelectedLbl;
static HWND mw_snapshotSelectedStc;
static HWND mw_editBtn;
static HWND mw_sendPCBtn;
static HWND mw_addToLibBtn[2];
static HWND mw_bundleToLibBtn[2];
static HWND mw_copyFromLibBtn;
static HWND mw_snapshotRxItemsBtn;
static HWND mw_snapshotTxItemsBtn;
static HWND mw_storeBtn;
static HWND mw_snapshotRxBtn;
static HWND mw_snapshotTxBtn;
static HWND mw_snapshotLbx;
static HWND mw_libLbl[2];
static HWND mw_libTbx[2];
static HWND mw_libLbx[2];
static HWND mw_libFileBtn[2];
static HWND mw_libSortBtn[2];
static HWND mw_libCountLbl[2];
static HWND mw_libCountStc[2];
static HWND mw_libSelectedLbl[2];
static HWND mw_libSelectedStc[2];
static HWND mw_copyBtn;
static HWND mw_deleteBtn;
static HWND mw_renameBtn;
static HWND mw_recommentBtn;
static HWND mw_diffBtn;

static AREA mw_snapshotLblArea;
static AREA mw_snapshotTbxArea;
static AREA mw_snapshotLbxArea;
static AREA mw_snapshotFileBtnArea;
static AREA mw_snapshotSelectedLblArea;
static AREA mw_snapshotSelectedStcArea;

static AREA mw_snapshotRxItemsBtnArea;
static AREA mw_snapshotTxItemsBtnArea;
static AREA mw_storeBtnArea;
static AREA mw_snapshotRxBtnArea;
static AREA mw_snapshotTxBtnArea;

static AREA mw_libLblArea[2];
static AREA mw_libTbxArea[2];
static AREA mw_libLbxArea[2];
static AREA mw_libFileBtnArea[2];
static AREA mw_libSortBtnArea[2];
static AREA mw_libSelectedLblArea[2];
static AREA mw_libSelectedStcArea[2];
static AREA mw_libCountLblArea[2];
static AREA mw_libCountStcArea[2];
static AREA mw_copyBtnArea;
static AREA mw_deleteBtnArea;
static AREA mw_renameBtnArea;
static AREA mw_recommentBtnArea;
static AREA mw_diffBtnArea;
static int mw_buttonGroupTop;
static int mw_buttonGroupBottom;
/*
 * Window size variables
 */
static SIZE mw_minClient;
static SIZE mw_minWnd;
static SIZE mw_wndClientDiff; /* the different in size between the window and
                                 the client area */
static int mw_minLibLbxW;
static int mw_bottomCtrlH;
static int mw_rightBtnDelta; /* the distance between the right snapshot
                                function buttons' x coordinate and the
                                width of the client area of MainWnd */
/*
 * Splitter tracking variables
 */
static BOOL mw_isDraggingHSplit;
static int mw_hSplitTop, mw_hSplitBottom, mw_hSplitterH;
static int mw_hSplitOrg;
static int mw_hSplitMin, mw_hSplitMax;
static int mw_hSplitDragY;
static BOOL mw_isDraggingVSplit;
static int mw_vSplitLeft, mw_vSplitRight;
static int mw_vSplitMin, mw_vSplitMax;
static int mw_vSplitMaxFromClientRight;
static int mw_vSplitDragX;
/* These act as extra arguments to OnSize().  They are the distances the
 * vertical and horizontal splitters need to be moved while resizing.
 * They are zeroed at the end of OnSize(). */
static int mw_splitterDx, mw_splitterDy;
static int mw_lastWndState;
/* When the position of a splitter changes, its new position is saved along
 * with the corresponding size of the client area so that the proportion
 * between the two can be preserved when resizing the window */
static int mw_hSplitClientH;
static int mw_vSplitClientW;
/* Used to detect when the cursor leaves the splitter area */
static BOOL mw_isCaptureSet = FALSE;
/*
 * Edge drawing variables
 */
#define mw_edgeCnt 6
static RECT mw_edgeRects[mw_edgeCnt];
/*
 * Item selection tracking variables.
 */
static int *mw_snapshotSelItems;
static int mw_snapshotSelItemMax;
static int mw_snapshotSelItemCnt;
static int mw_originalSnapshotMenuItemCnt;
static int *mw_libSelItems;      /* list of selected library items */
static int mw_libSelItemMax;     /* amount of memory allocated for mw_libSelItems */
static int mw_libSelItemCnt;     /* number of library items selected */
static int mw_libSelIndex = -1;  /* indicates which library has items selected */
static int mw_libMenuIndex = -1; /* indicates which library menu was activated */
static int mw_originalLibMenuItemCnt;
/*
 * Snapshot item update pseudo-constant array - used in UpdateSnapshotLbx() to send
 * messages to the editor of the changed item.
 */
static HWND *mw_dlgPtrs[7] = {
    &Prog_voiceDlg,
    &Prog_pfmDlg,
/* these are set in MainWnd_Register to get around the fact that they are not constants */
    &Prog_fxDlg,
    &Prog_pcDlg,
    &Prog_mtoDlg,
    &Prog_mtfDlg,
    &Prog_sysDlg
};


/*
 * Procedure definitions
 */

/*
 * AddToLib() - Inserts items from the snapshot into a library.
 */
void MainWnd_AddToLib(HWND mainWnd, int libIdx, const int *sIdxs, int sIdxCnt)
{
    BOOL duplicatesWereFound = FALSE;
    int i;

    /*
     * For each selected item in the snapshot.
     */
    for (i = 0; i < sIdxCnt; i++) {
        int sIdx = sIdxs[i];
        METAINDEX type = Snapshot_GetItemType(sIdx);
        _TUCHAR text[80];
        int libItemIdx;

        /*
         * Add the item to the library.  Pack it first if it's an edit buffer.
         */
        if (type == META_VCED) {
            BYTE tempVmem[sizeof Prog_snapshot.vmem.data[0]];

            TXPack_AvcedToVmem(Prog_snapshot.avced.aced.data
                    , Prog_snapshot.avced.vced.data, tempVmem);
            /*
             * Add the item to the library and get the one-based index.
             */
            libItemIdx = TxLib_AddItem(&Prog_lib[libIdx], META_VMEM
                    , tempVmem, Prog_libraryNoDuplicates);
        } else if (type == META_PCED) {
            BYTE tempPmem[sizeof Prog_snapshot.pmem.data[0]];

            TXPack_PcedToPmem(Prog_snapshot.pced.data, tempPmem);
            /*
             * Add the item to the library and get the one-based index.
             */
            libItemIdx = TxLib_AddItem(&Prog_lib[libIdx], META_PMEM
                    , tempPmem, Prog_libraryNoDuplicates);
        } else if (Snapshot_IsBankItem(sIdx)) {
            int bankIdx = Snapshot_GetBankIndex(sIdx);
            METAINDEX libItemType = type >= META_PRESET_A ? META_VMEM : type;

            /*
             * Add the item to the library and get the one-based index.
             */
            libItemIdx = TxLib_AddItem(&Prog_lib[libIdx], libItemType
                    , TX81Z_meta[type].dataPtr + bankIdx
                        * TX81Z_meta[type].libSize, Prog_libraryNoDuplicates);
        } else {
            /*
             * Add the item to the library and get the one-based index.
             */
            libItemIdx = TxLib_AddItem(&Prog_lib[libIdx], type
                    , TX81Z_meta[type].dataPtr, Prog_libraryNoDuplicates);
        }
        /*
         * If the item was added (if there wasn't an identical item in the
         * library), add it to the library list.
         */
        if (libItemIdx > 0) {
            /*
             * Get the item display string.
             */
            TxLib_FormatName(&Prog_lib[libIdx], NF_TYPE_NAME_AND_COMMENT, text);
            /*
             * Add the item to the list box.
             */
            TxLbx_InsertItem(mw_libLbx[libIdx], libItemIdx - 1, text, 0
                    , TA_DIRTY);
            /*
             * Redraw the file name text box so it is drawn dirty.
             */
            InvalidateRect(mw_libTbx[libIdx], NULL, TRUE);
        } else if (libItemIdx < 0) {
            duplicatesWereFound = TRUE;
        } else {
            /*
             * Out of memory error -- an error message was already generated
             * by CDVList_Append.
             */
            return;
        }
    }
    /*
     * Notify the user if any of the items failed to get added.
     */
    if (duplicatesWereFound) {
        MessageBox(mainWnd, mw_libDupeMsg, mw_libDupeTitle
                , MB_OK | MB_ICONINFORMATION);
    }
}

/*
 * BundleToLib() - Creates performance bundles and adds them to the library.
 */
void MainWnd_BundleToLib(HWND mainWnd, int libIdx, const int *sIdxs
        , int sIdxCnt)
{
    BOOL duplicatesWereFound = FALSE;
    int i;
    SNAPSHOT *snapshot = &Prog_snapshot;

    /*
     * For each item selected in the library.
     */
    for (i = 0; i < sIdxCnt; i++) {
        int sIdx = sIdxs[i];
        _TUCHAR text[80];
        int libItemIdx;
        BYTE bndlBuf[S_MAX_BNDL_SIZE];

        /*
         * The behavior of the buttons in the main window should prevent
         * anything but performances from being in the selection, so there's
         * no real reason to check the type here.
         */
        /*
         * Create the performance bundle.
         */
        Snapshot_BundlePerformance(snapshot, sIdx, bndlBuf);
        /*
         * Add the item to the library and get the one-based index.
         */
        libItemIdx = TxLib_AddItem(&Prog_lib[libIdx], META_BNDL, bndlBuf
                , Prog_libraryNoDuplicates);
        /*
         * If the item was added (if there wasn't an identical item in the
         * library), add it to the library list.
         */
        if (libItemIdx > 0) {
            /*
             * Get the item display string.
             */
            TxLib_FormatName(&Prog_lib[libIdx], NF_TYPE_NAME_AND_COMMENT, text);
            /*
             * Add the item to the list box.
             */
            TxLbx_InsertItem(mw_libLbx[libIdx], libItemIdx - 1, text, 0
                    , TA_DIRTY);
            /*
             * Redraw the file name text box so it is drawn dirty.
             */
            InvalidateRect(mw_libTbx[libIdx], NULL, TRUE);
        } else if (libItemIdx < 0) {
            duplicatesWereFound = TRUE;
        } else {
            /*
             * Out of memory error -- an error message was already generated
             * by CDVList_Append.
             */
            return;
        }
    }
    /*
     * Notify the user if any of the items failed to get added.
     */
    if (duplicatesWereFound) {
        MessageBox(mainWnd, mw_libDupeMsg, mw_libDupeTitle
                , MB_OK | MB_ICONINFORMATION);
    }
}

/*
 * Create() - Creates and initializes the main window.
 */
HWND MainWnd_Create(void)
{
    HWND mainWnd;
    HWND iconSetterWnd;
    _TUCHAR title[80];
    WINDOWPLACEMENT minPlacement = { sizeof minPlacement };
    int i;
    UINT request = 0;
    MENUITEMINFO menuItemInfo;
    HDC dC;

    /*
     * Set the caption icon for all generic dialogs.
     */
    iconSetterWnd = CreateDialog(
            Prog_instance                   /* program instance */
            , MAKEINTRESOURCE(IDD_WAITDLG)  /* dialog resource ID */
            , HWND_DESKTOP                  /* parent window */
            , NULL                          /* dialog proc */
        );
    SetClassLong(iconSetterWnd, GCL_HICON, (long) Prog_icon);
    DestroyWindow(iconSetterWnd);

    /*
     * Create the main window.
     */
    mainWnd = CreateDialog(
            Prog_instance                   /* program instance */
            , MAKEINTRESOURCE(IDD_MAINWND)  /* dialog resource ID */
            , HWND_DESKTOP                  /* parent window */
            , NULL                          /* dlg proc set in window class */
        );
#ifdef _DEBUG
    if (!mainWnd) {
        MsgBox_LastErrorF(NULL, _T("Error creating main window"));
        return NULL;
    }
#endif
    /*
     * Create a window title and put it in the window.
     */
    _stprintf(title, _T("Main Window - %s"), Prog_name);
    SetWindowText(mainWnd, title);
    /*
     * Set up the menus.
     */
    mw_menu = GetMenu(mainWnd);
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_TYPE;
    menuItemInfo.fType = MFT_OWNERDRAW;
    menuItemInfo.dwTypeData = 0;
    SetMenuItemInfo(mw_menu, IDM_START_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(mw_menu, IDM_MAINWND_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(mw_menu, IDM_SNAPSHOT_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(mw_menu, IDM_LIBRARY_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(mw_menu, IDM_GLOBAL_OPTIONS, FALSE, &menuItemInfo);
    SetMenuItemInfo(mw_menu, IDM_VERSION_OPTIONS, FALSE, &menuItemInfo);
    dC = GetDC(mainWnd);
    for (i = 0; i < mw_optionMenuHeaderCnt; i++) {
        SIZE size;

        GetTextExtentPoint32(dC, mw_optionMenuHeaders[i], 19, &size);
        if (size.cx > mw_optionMenuHeaderWidth) {
            mw_optionMenuHeaderWidth = size.cx;
        }
    }
    ReleaseDC(mainWnd, dC);
    /*
     * Cache all the control handles.  No, I've never heard of an array.
     */
    mw_snapshotLbl = GetDlgItem(mainWnd, IDC_SNAPSHOT_LBL);
    mw_snapshotTbx = GetDlgItem(mainWnd, IDC_SNAPSHOT_TBX);
    mw_snapshotFileBtn = GetDlgItem(mainWnd, IDC_SNAPSHOT_FILE_BTN);
    mw_snapshotSelectedLbl = GetDlgItem(mainWnd, IDC_SNAPSHOT_SELECTED_LBL);
    mw_snapshotSelectedStc = GetDlgItem(mainWnd, IDC_SNAPSHOT_SELECTED_STC);
    mw_snapshotLbx = GetDlgItem(mainWnd, IDC_SNAPSHOT_LBX);
    mw_editBtn = GetDlgItem(mainWnd, IDC_ITEM_EDIT_BTN);
    mw_sendPCBtn = GetDlgItem(mainWnd, IDC_TRANSMIT_PC_BTN);
    mw_addToLibBtn[0] = GetDlgItem(mainWnd, IDC_ITEM_ADD_TO_LIB0_BTN);
    mw_addToLibBtn[1] = GetDlgItem(mainWnd, IDC_ITEM_ADD_TO_LIB1_BTN);
    mw_bundleToLibBtn[0] = GetDlgItem(mainWnd, IDC_ITEM_BNDL_TO_LIB0_BTN);
    mw_bundleToLibBtn[1] = GetDlgItem(mainWnd, IDC_ITEM_BNDL_TO_LIB1_BTN);
    mw_copyFromLibBtn = GetDlgItem(mainWnd, IDC_ITEM_COPY_FROM_LIB_BTN);
    mw_snapshotRxItemsBtn = GetDlgItem(mainWnd, IDC_SNAPSHOT_RX_ITEMS_BTN);
    mw_snapshotTxItemsBtn = GetDlgItem(mainWnd, IDC_SNAPSHOT_TX_ITEMS_BTN);
    mw_storeBtn = GetDlgItem(mainWnd, IDC_ITEM_STORE_BTN);
    mw_snapshotRxBtn = GetDlgItem(mainWnd, IDC_SNAPSHOT_RX_BTN);
    mw_snapshotTxBtn = GetDlgItem(mainWnd, IDC_SNAPSHOT_TX_BTN);
    for (i = 0; i < 2; i++) {
        int idOffset = i * (IDC_LIB1_LBL - IDC_LIB0_LBL);

        mw_libLbl[i] = GetDlgItem(mainWnd, IDC_LIB0_LBL + idOffset);
        mw_libTbx[i] = GetDlgItem(mainWnd, IDC_LIB0_TBX + idOffset);
        mw_libLbx[i] = GetDlgItem(mainWnd, IDC_LIB0_LBX + idOffset);
        mw_libFileBtn[i] = GetDlgItem(mainWnd, IDC_LIB0_FILE_BTN + idOffset);
        mw_libSortBtn[i] = GetDlgItem(mainWnd, IDC_LIB0_SORT_BTN + idOffset);
        mw_libSelectedLbl[i] = GetDlgItem(mainWnd, IDC_LIB0_SELECTED_LBL + idOffset);
        mw_libSelectedStc[i] = GetDlgItem(mainWnd, IDC_LIB0_SELECTED_STC + idOffset);
        mw_libCountLbl[i] = GetDlgItem(mainWnd, IDC_LIB0_COUNT_LBL + idOffset);
        mw_libCountStc[i] = GetDlgItem(mainWnd, IDC_LIB0_COUNT_STC + idOffset);
    }
    mw_copyBtn = GetDlgItem(mainWnd, IDC_COPY_BTN);
    mw_deleteBtn = GetDlgItem(mainWnd, IDC_DELETE_BTN);
    mw_renameBtn = GetDlgItem(mainWnd, IDC_RENAME_BTN);
    mw_recommentBtn = GetDlgItem(mainWnd, IDC_RECOMMENT_BTN);
    mw_diffBtn = GetDlgItem(mainWnd, IDC_DIFF_BTN);
    /*
     * Cache all the control areas since they will be moved around a lot.
     */
    Window_GetParentRelativeArea(mw_snapshotLbl, mainWnd, &mw_snapshotLblArea);
    Window_GetParentRelativeArea(mw_snapshotTbx, mainWnd, &mw_snapshotTbxArea);
    Window_GetParentRelativeArea(mw_snapshotFileBtn, mainWnd
            , &mw_snapshotFileBtnArea);
    Window_GetParentRelativeArea(mw_snapshotSelectedLbl, mainWnd
            , &mw_snapshotSelectedLblArea);
    Window_GetParentRelativeArea(mw_snapshotSelectedStc, mainWnd
            , &mw_snapshotSelectedStcArea);
    Window_GetParentRelativeArea(mw_snapshotLbx, mainWnd
            , &mw_snapshotLbxArea);

    Window_GetParentRelativeArea(mw_snapshotRxItemsBtn, mainWnd
            , &mw_snapshotRxItemsBtnArea);
    Window_GetParentRelativeArea(mw_snapshotTxItemsBtn, mainWnd
            , &mw_snapshotTxItemsBtnArea);
    Window_GetParentRelativeArea(mw_storeBtn, mainWnd, &mw_storeBtnArea);
    Window_GetParentRelativeArea(mw_snapshotRxBtn, mainWnd
            , &mw_snapshotRxBtnArea);
    Window_GetParentRelativeArea(mw_snapshotTxBtn, mainWnd
            , &mw_snapshotTxBtnArea);

    for (i = 0; i < 2; i++) {
        Window_GetParentRelativeArea(mw_libLbl[i], mainWnd, &mw_libLblArea[i]);
        Window_GetParentRelativeArea(mw_libTbx[i], mainWnd, &mw_libTbxArea[i]);
        Window_GetParentRelativeArea(mw_libLbx[i], mainWnd, &mw_libLbxArea[i]);
        Window_GetParentRelativeArea(mw_libFileBtn[i], mainWnd
                , &mw_libFileBtnArea[i]);
        Window_GetParentRelativeArea(mw_libSortBtn[i], mainWnd
                , &mw_libSortBtnArea[i]);
        Window_GetParentRelativeArea(mw_libCountLbl[i], mainWnd
                , &mw_libCountLblArea[i]);
        Window_GetParentRelativeArea(mw_libCountStc[i], mainWnd
                , &mw_libCountStcArea[i]);
        Window_GetParentRelativeArea(mw_libSelectedLbl[i], mainWnd
                , &mw_libSelectedLblArea[i]);
        Window_GetParentRelativeArea(mw_libSelectedStc[i], mainWnd
                , &mw_libSelectedStcArea[i]);
    }
    Window_GetParentRelativeArea(mw_copyBtn, mainWnd, &mw_copyBtnArea);
    /*
     * Need to adjust the button positions a bit since I can't get it right in
     * the dialog editor.
     */
    mw_snapshotFileBtnArea.y--;
    mw_deleteBtnArea = mw_copyBtnArea;
    mw_deleteBtnArea.y += mw_copyBtnArea.h;
    mw_renameBtnArea = mw_deleteBtnArea;
    mw_renameBtnArea.y += mw_deleteBtnArea.h;
    mw_recommentBtnArea = mw_renameBtnArea;
    mw_recommentBtnArea.y += mw_renameBtnArea.h;
    mw_diffBtnArea = mw_recommentBtnArea;
    mw_diffBtnArea.y += mw_recommentBtnArea.h;
    MoveWindow(mw_snapshotFileBtn, PASS_AREA_FIELDS(mw_snapshotFileBtnArea)
            , TRUE);
    MoveWindow(mw_deleteBtn, PASS_AREA_FIELDS(mw_deleteBtnArea), TRUE);
    MoveWindow(mw_renameBtn, PASS_AREA_FIELDS(mw_renameBtnArea), TRUE);
    MoveWindow(mw_recommentBtn, PASS_AREA_FIELDS(mw_recommentBtnArea), TRUE);
    MoveWindow(mw_diffBtn, PASS_AREA_FIELDS(mw_diffBtnArea), TRUE);
    /*
     * The dialog template is set up to use the smallest possible window size,
     * so get the default window placement to calculate the minimum window
     * sizes.
     */
    GetWindowPlacement(mainWnd, &minPlacement);
    GetClientRect(mainWnd, &mw_clientRect);
    /*
     * Set the initial vertical splitter position.
     */
    mw_vSplitLeft = mw_copyBtnArea.x;
    mw_vSplitRight = mw_vSplitLeft + mw_copyBtnArea.w + 2;
    mw_vSplitClientW = mw_clientRect.right;
    /*
     * Set the minimum allowable client width, which happens to be the right
     * side of the library 2 list box.
     */
    mw_minClient.cx = AREA_R(mw_libLbxArea[1]);
    /*
     * Set the minimum allowable window width.
     */
    if (mw_clientRect.right != mw_minClient.cx) {
        minPlacement.rcNormalPosition.right -= mw_clientRect.right
            - mw_minClient.cx;
    }
    /*
     * Save the minimum window size - used in mw_OnSizing()
     */
    mw_minWnd.cx = RECT_W(minPlacement.rcNormalPosition);
    mw_minWnd.cy = RECT_H(minPlacement.rcNormalPosition);
    mw_wndClientDiff.cx = mw_minWnd.cx - mw_clientRect.right;
    mw_wndClientDiff.cy = mw_minWnd.cy - mw_clientRect.bottom;
    /*
     * Save the right button delta value.
     */
    mw_rightBtnDelta = mw_clientRect.right - mw_snapshotRxItemsBtnArea.x;
    /*
     * Save the minimum width of a library list box for constraining the
     * vertical splitter.
     */
    mw_minLibLbxW = mw_libLbxArea[0].w;
    /*
     * Save the vertical splitter constraints.
     */
    mw_vSplitMin = mw_minLibLbxW + 1;
    mw_vSplitMaxFromClientRight = mw_minLibLbxW + 1 + mw_copyBtnArea.w;
    mw_vSplitMax = mw_clientRect.right - mw_vSplitMaxFromClientRight;
    /*
     * Set the initial horizontal splitter position.
     */
    mw_hSplitTop = mw_hSplitOrg = AREA_B(mw_snapshotLbxArea) + 1;
    mw_hSplitBottom = mw_libTbxArea[0].y - 2;
    mw_hSplitterH = mw_hSplitBottom - mw_hSplitTop;
    mw_hSplitClientH = mw_clientRect.bottom;
    /*
     * Save the height of the controls below the splitter - this is used for
     * constraining the splitter and the window height.
     */
    mw_bottomCtrlH = (mw_copyBtnArea.h * 5 + 1) + (mw_libLbxArea[0].y
            - mw_hSplitTop);
    /*
     * Save the minimum client height.
     */
    mw_minClient.cy = mw_hSplitTop + mw_bottomCtrlH + 4;
    /*
     * Save the horizontal splitter constraints.
     */
    mw_hSplitMin = AREA_B(mw_snapshotTxBtnArea) + 1;
    mw_hSplitMax = mw_clientRect.bottom - mw_bottomCtrlH - 4;
    /*
     * Set the cursor.
     */
    mw_cursor = Prog_arrowCursor;
    /*
     * Create and set some fonts.
     */
    Prog_CreateFonts(mainWnd);
    SetWindowFont(mw_snapshotLbl, Prog_tahomaBoldFont, TRUE);
    /*
     * Set up the item selection caches.
     */
    mw_snapshotSelItemMax = 32;
    mw_snapshotSelItems = malloc(sizeof(*mw_snapshotSelItems)
            * mw_snapshotSelItemMax);
    mw_snapshotSelItemCnt = 0;
    mw_libSelItemMax = 32;
    mw_libSelItems = malloc(sizeof(*mw_libSelItems) * mw_libSelItemMax);
    mw_libSelItemCnt = 0;
    /*
     * Initialize the list boxes.
     */
    TxLbx_Init(mw_snapshotLbx, &mw_ssTXProps);
    for (i = 0; i < 2; i++) {
        TxLbx_Init(mw_libLbx[i], &mw_libTXProps);
        SetWindowFont(mw_libLbl[i], Prog_tahomaBoldFont, TRUE);
    }
    mw_InitMenuButtons();
    /*
     * Save the number of items in the unmodified file menus.
     */
    mw_originalSnapshotMenuItemCnt = GetMenuItemCount(Prog_snapshotFileMenu);
    mw_originalLibMenuItemCnt = GetMenuItemCount(Prog_libFileMenu);
    /*
     * Check the menu items of saved options.
     */
    if (Prog_showFullPaths) {
        MenuItem_Check(mw_menu, IDM_SHOW_FULL_PATHS);
    }
    if (Prog_dblClkingBankDoesntOpen) {
        MenuItem_Check(mw_menu, IDM_DBLCLKING_BANK_DOESNT_OPEN);
    }
    if (Prog_mainWndToFront) {
        MenuItem_Check(mw_menu, IDM_MAINWND_TO_FRONT);
    }
    if (Prog_snapshotAutoSave) {
        MenuItem_Check(mw_menu, IDM_SNAPSHOT_AUTOSAVE);
    }
    if (Prog_libraryNoDuplicates) {
        MenuItem_Check(mw_menu, IDM_LIBRARY_NO_DUPLICATES);
    }
    if (Prog_unslashedZeros) {
        MenuItem_Check(mw_menu, IDM_UNSLASHED_ZEROS);
        MiniFont_ReplaceZero(FALSE);
    }
    MenuItem_RadioCheck(mw_menu, IDM_VERSION_10, IDM_VERSION_OTHER
            , IDM_VERSION_ORG + Prog_tx81zVersion);
    /*
     * Set the initial splitter position.
     */
    mw_splitterDy = Prog_mainWndHSplitterPos;
    Prog_mainWndHSplitterPos = 0; /* this will be set again in OnSize() */
    mw_lastWndState = SIZE_RESTORED;
    /*
     * Show the window.
     */
    if (IsRectEmpty(&Prog_mainWndPlacement.rcNormalPosition)) {
        /*
         * If the placement hasn't been saved yet, just show the window.
         */
        ShowWindow(mainWnd, SW_SHOWNORMAL);
    } else {
        /*
         * Set the saved window placement and show the window.
         */
        Prog_mainWndPlacement.showCmd = SW_SHOWNORMAL;
        SetWindowPlacement(mainWnd, &Prog_mainWndPlacement);
    }
    /*
     * Draw the snapshot window to make sure it's not blank.
     */
    mw_UpdateSnapshotLbx(UL_ALL);
    /*
     * Load previously open files.
     */
    if (Prog_openPrevious) {
        MenuItem_Check(mw_menu, IDM_OPEN_PREVIOUS);
        /*
         * If a snapshot file name was stored in the registry.
         */
        if (Prog_snapshotRecent[0][0] != '\0') {
RetrySnapshot:
            if (!File_Exists(Prog_snapshotRecent[0])) {
                _TUCHAR msg[_MAX_PATH + 80];

                _stprintf(msg, _T("The snapshot file %s could not be found.")
                        , Prog_snapshotRecent[0]);
                if (IDRETRY == MessageBox(mainWnd, msg, _T("File Error")
                            , MB_ICONWARNING | MB_RETRYCANCEL))
                {
                    goto RetrySnapshot;
                } else {
                    goto InitSnapshot;
                }
            }
            mw_OpenSnapshotFile(mainWnd, Prog_snapshotRecent[0], FALSE);
        } else {
InitSnapshot:
            Snapshot_Init(&Prog_snapshot);
        }
        for (i = 0; i < 2; i++) {
            /*
             * If a library file name was stored in the registry.
             */
            if (Prog_libRecent[i][0][0] != '\0') {
RetryLib:
                if (!File_Exists(Prog_libRecent[i][0])) {
                    _TUCHAR msg[_MAX_PATH + 80];

                    _stprintf(msg
                            , _T("The library file %s could not be found.")
                            , Prog_libRecent[i][0]);
                    if (IDRETRY == MessageBox(mainWnd, msg, _T("File Error")
                                , MB_ICONWARNING | MB_RETRYCANCEL))
                    {
                        goto RetryLib;
                    } else {
                        goto InitLib;
                    }
                }
                mw_OpenLibraryFile(mainWnd, i, Prog_libRecent[i][0]);
            } else {
InitLib:
                TxLib_Init(&Prog_lib[i]);
            }
        }
    } else {
        /*
         * Initialize the meat.
         */
        Snapshot_Init(&Prog_snapshot);
        TxLib_Init(&Prog_lib[0]);
        TxLib_Init(&Prog_lib[1]);
    }
    /*
     * If "Request Edit Buffers On Startup" is checked.
     */
    if (Prog_startupEditBufferRequest) {
        /*
         * Get the edit buffers and system setup from the unit.
         */
        MenuItem_Check(mw_menu, IDM_STARTUP_EDIT_BUFFER_REQUEST);
        request = REQ_EDIT_BUFFERS;
    }
    if (request) {
        mw_RetrieveAndUpdate(request);
    }

    return mainWnd;
}

/*
 * Register() - Registers the window class for the main window.  Returns true
 *              on success, false on failure.
 */
BOOL MainWnd_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = CS_OWNDC;
    classInfo.lpfnWndProc = (WNDPROC) mw_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = DLGWINDOWEXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = Prog_icon;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) NULL;
    classInfo.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN_MENU);
    classInfo.lpszClassName = MainWnd_className;
    classInfo.hIconSm = NULL;

    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering main window class"));
        return FALSE;
    }

    return TRUE;
}

/*
 * StoreItem() - stores an edit buffer in the TX81Z and snapshot.
 */
BOOL MainWnd_StoreItem(int srcItemIndex, int destItemIndex)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    HWND listBox = mw_snapshotLbx;
    _TUCHAR text[40];
    METAINDEX srcType = Snapshot_GetItemType(srcItemIndex);
    int destBankIndex = Snapshot_GetBankIndex(destItemIndex);
    REQUEST update = 0;

    if (srcType == META_VCED) {
        assert(Snapshot_GetItemType(destItemIndex) == META_VMEM);
        /*
         * Try to store the voice in the TX81Z.  If memory protect was on and
         * the user didn't want to turn it off, abort the operation.
         */
        if (!TX81Z_StoreVoice(Prog_midi, destBankIndex)) {
            /*
             * Operation failed.
             */
            return FALSE;
        }
        /*
         * Convert and copy the snapshot edit buffer to the internal bank of
         * the snapshot.
         */
        TXPack_AvcedToVmem(snapshot->avced.aced.data
                , snapshot->avced.vced.data
                , snapshot->vmem.data[destBankIndex]);
        update = REQ_VMEM;
    } else {
        /*
         * Try to store the performance.  If memory protect was on and the user
         * didn't want to turn it off, abort the operation.
         */
        if (!TX81Z_StorePfm(Prog_midi, destBankIndex)) {
            /*
             * Operation failed.
             */
            return FALSE;
        }
        /*
         * Convert and copy the snapshot edit buffer to the internal bank of
         * the snapshot.
         */
        TXPack_PcedToPmem(snapshot->pced.data
                , snapshot->pmem.data[destBankIndex]);
        update = REQ_PMEM;
    }
    /*
     * Replace the item in the list box.
     */
    Snapshot_FormatName(snapshot, destItemIndex, NF_NUMBER_AND_NAME, text);
    TxLbx_ReplaceItem(listBox, destItemIndex, text, 0, TA_DIRTY);
    /*
     * Select the bank item where the voice was stored.
     */
    ListBox_SetSel(listBox, FALSE, -1);
    ListBox_SetSel(listBox, TRUE, destItemIndex);
    ListBox_SetCaretIndex(listBox, destItemIndex);
    mw_GetSelItems(listBox);
    mw_UpdateButtonStates();
    mw_UpdateEditors(update);

    /*
     * Operation successful.
     */
    return TRUE;
}

/*
 * AddRecentFile() - Adds a new file name to a recent file list.  If the file already
 *                   exists in the list, it is moved to the top.
 */
void mw_AddRecentFile(_TUCHAR recentList[Prog_recentFileMax][_MAX_PATH]
        , _TUCHAR fileName[_MAX_PATH])
{
    int i;
    int existPos = Prog_recentFileMax - 1;

    /*  fn = blii
     *   0 - blaa
     *   1 - blee
     *   2 - blii  <- existPos
     *   3 - bloo
     */
    /*
     * Search for the file name in the list.
     */
    for (i = 0; i < existPos; i++) {
        if (StrEqN(fileName, recentList[i], _MAX_PATH)) {
            existPos = i;
        }
    }
    /*
     * Move old file names down.
     */
    for (i = existPos; i >= 1; i--) {
        _tcsncpy(recentList[i], recentList[i - 1], _MAX_PATH);
    }
    /*
     * Put the new file name in the list.
     */
    _tcsncpy(recentList[0], fileName, _MAX_PATH);
}

/*
 * ChooseColor() - Show the color dialog to pick the highlight color.
 */
void mw_ChooseColor(HWND mainWnd)
{
    CHOOSECOLOR chooseColor;

    /*
     * Set up the dialog structure to pick the highlight color.
     */
    chooseColor.lStructSize = sizeof(CHOOSECOLOR);
    chooseColor.hwndOwner = mainWnd;
    chooseColor.hInstance = NULL;
    chooseColor.rgbResult = Prog_hiColor;
    chooseColor.lpCustColors = Prog_custColors;
    chooseColor.Flags = CC_SOLIDCOLOR | CC_RGBINIT;
    chooseColor.lCustData = 0;
    chooseColor.lpfnHook = NULL;
    chooseColor.lpTemplateName = NULL;

    if (ChooseColor(&chooseColor)) {
        Prog_hiColor = chooseColor.rgbResult;
        SendMessage(mainWnd, WM_SYSCOLORCHANGE, 0, 0);
        /*
         * Update windows.
         */
        InvalidateRect(mainWnd, NULL, TRUE);
        if (Prog_voiceDlg) {
            InvalidateRect(Prog_voiceDlg, NULL, TRUE);
        }
        if (Prog_fxDlg) {
            InvalidateRect(Prog_fxDlg, NULL, TRUE);
        }
    }
}

/*
 * CleanLbx() - Resets the color of all items in the list box.
 */
void mw_CleanLbx(HWND listBox)
{
    int i;
    int itemCnt = ListBox_GetCount(listBox);

    for (i = 0; i < itemCnt; i++) {
        ListBox_SetItemData(listBox, i, Prog_wndTextColor);
    }
}

/*
 * ConfirmBankTransfer() - Asks the user if it's ok to transfer an entire bank
 *                         to or from the TX81Z.
 */
BOOL mw_ConfirmBankTransfer(HWND mainWnd, REQUEST reqFlags, BOOL retrieve)
{
    _TUCHAR message[256];
    _TUCHAR *action = retrieve ? _T("retrieve") : _T("transmit");
    _TUCHAR fmt[] = _T("You are about to %s the entire user %s.\r\n")
        _T("Are you sure?");

    if ((reqFlags & REQ_VMEM) && (reqFlags & REQ_PMEM)) {
        _stprintf(message, fmt, action, _T("voice bank I01..I32 and the ")
                _T("entire user performance bank PF01..PF24"));
    } else if (reqFlags & REQ_VMEM) {
        _stprintf(message, fmt, action, _T("voice bank I01..I32"));
    } else if (reqFlags & REQ_PMEM) {
        _stprintf(message, fmt, action, _T("performance bank PF01..PF24"));
    }
    return IDYES == MessageBox(mainWnd, message, _T("Confirm Bank Transfer")
                , MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
}

/*
 * CopyFromLib() - Copies the selected library items into the snapshot.
 */
void mw_CopyFromLib(HWND mainWnd)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    int libItemIndex;
    SINDEX sIdx;
    int bankIndex;
    BYTE *libItemData;
    METAINDEX libItemType;
    int i;
    TXLIB *txLib = &Prog_lib[mw_libSelIndex];

    /*
     * If the user is using the copy from lib button to unpack a bundle.
     */
    if (mw_libSelItemCnt == 1) {
        TxLib_SetPtr(txLib, mw_libSelItems[0]);
        if (TxLib_GetItemType(txLib) == META_BNDL) {
            libItemData = TxLib_GetItemData(txLib);
            if (BndlDlg_Create(mainWnd, libItemData)) {
                mw_UpdateSnapshotLbx(UL_ALL);
            }
            return;
        }
    }
    /*
     * For each selected item in the snapshot.
     */
    for (i = 0; i < mw_snapshotSelItemCnt; i++) {
        /*
         * Get the index of the next selected snapshot item.
         */
        sIdx = mw_snapshotSelItems[i];
        /*
         * Get the index of the next select library item.
         */
        libItemIndex = mw_libSelItems[i];
        /*
         * Fetch the library item's information.
         */
        TxLib_SetPtr(txLib, libItemIndex);
        libItemData = TxLib_GetItemData(txLib);
        libItemType = TxLib_GetItemType(txLib);
        /*
         * Do a raw copy of the library item data into the snapshot.
         */
        switch (libItemType) {
            case META_VMEM:
                if (sIdx == SI_VCED) {
                    TXPack_VmemToAvced(libItemData, &snapshot->avced, FALSE);
                } else {
                    bankIndex = Snapshot_GetBankIndex(sIdx);
                    memcpy(snapshot->vmem.data[bankIndex], libItemData
                            , sizeof snapshot->vmem.data[bankIndex]);
                }
                break;
            case META_PMEM:
                if (sIdx == SI_PCED) {
                    TXPack_PmemToPced(libItemData, &snapshot->pced);
                } else {
                    bankIndex = Snapshot_GetBankIndex(sIdx);
                    memcpy(snapshot->pmem.data[bankIndex], libItemData
                            , sizeof snapshot->pmem.data[bankIndex]);
                }
                break;
            case META_FX:
                memcpy(snapshot->fx.data, libItemData
                        , sizeof snapshot->fx.data);
                break;
            case META_PC:
                memcpy(snapshot->pc.data, libItemData
                        , sizeof snapshot->pc.data);
                break;
            case META_MTO:
                memcpy(snapshot->mto.data, libItemData
                        , sizeof snapshot->mto.data);
                break;
            case META_MTF:
                memcpy(snapshot->mtf.data, libItemData
                        , sizeof snapshot->mtf.data);
                break;
            case META_SYS:
                memcpy(snapshot->sys.data, libItemData
                        , sizeof snapshot->sys.data);
                break;
            default:
                assert(0);
                return;
        }
        /*
         * Mark the item as loaded and dirty.
         */
        Snapshot_SetItemGroupLoaded(snapshot, sIdx);
        Snapshot_DirtyItem(snapshot, sIdx);
    }
    mw_UpdateSnapshotLbx(UL_ALL);
}

/*
 * CopyLibItems() - Copies the selected items from one library to the other.
 */
void mw_CopyLibItems(HWND mainWnd)
{
    int destLibIndex = 1 - mw_libSelIndex;
    TXLIB *srcTxLib = &Prog_lib[mw_libSelIndex];
    TXLIB *destTxLib = &Prog_lib[destLibIndex];
    HWND srcLbx = mw_libLbx[mw_libSelIndex];
    HWND destLbx = mw_libLbx[destLibIndex];
    BYTE *libItemData;
    METAINDEX libItemType;
    int srcIndex, destIndex;
    int *newLibSelItems;
    int newLibSelItemCnt = 0;
    _TUCHAR comment[TXLIBITEM_COMMENT_SIZE];
    _TUCHAR lbxItemText[80];
    _TUCHAR itemCntText[10];
    int i;

    newLibSelItems = malloc(mw_libSelItemMax * sizeof(int *));
    if (!newLibSelItems) {
        Error_OnError(E_MALLOC_ERROR);
        return;
    }
    for (i = 0; i < mw_libSelItemCnt; i++) {
        /*
         * Get the source item information.
         */
        srcIndex = mw_libSelItems[i];
        TxLib_SetPtr(srcTxLib, srcIndex);
        libItemData = TxLib_GetItemData(srcTxLib);
        libItemType = TxLib_GetItemType(srcTxLib);
        TxLib_GetItemComment(srcTxLib, comment);
        ListBox_SetSel(srcLbx, FALSE, srcIndex);
        /*
         * Add the source item to the destination library.
         */
        destIndex = TxLib_AddItem(destTxLib, libItemType, libItemData
                , Prog_libraryNoDuplicates);
        /*
         * If the item was added (wasn't a duplicate).
         */
        if (destIndex > 0) {
            /*
             * Set the item's comment.
             */
            TxLib_SetItemComment(destTxLib, comment);
            /*
             * Insert the item into the list box.
             */
            TxLib_FormatName(destTxLib, NF_TYPE_NAME_AND_COMMENT, lbxItemText);
            destIndex = TxLbx_InsertItem(destLbx, destIndex - 1, lbxItemText, 0
                    , TA_DIRTY);
            /*
             * Redraw the file name text box to show it as dirty.
             */
            InvalidateRect(mw_libTbx[destLibIndex], NULL, TRUE);
        } else if (destIndex < 0) {
            /*
             * Convert the negative, one-based duplicate index to a positive,
             * zero-based index.
             */
            destIndex = -destIndex - 1;
        } else {
            /*
             * Out of memory error -- an error message was already generated
             * by CDVList_Append.
             */
            return;
        }
        /*
         * Update the libSelItems array (the copied items will be selected
         * in the destination list box after the copy).
         */
        newLibSelItems[newLibSelItemCnt++] = destIndex;
    }
    /*
     * Update the count static of the destination list box.
     */
    _stprintf(itemCntText, _T("%d"), TxLib_GetCount(destTxLib));
    Static_SetText(mw_libCountStc[destLibIndex], itemCntText);
    /*
     * Replace the library selection array with the new one.
     */
    free(mw_libSelItems);
    mw_libSelItems = newLibSelItems;
    mw_libSelItemCnt = newLibSelItemCnt;
    mw_libSelIndex = destLibIndex;
    /*
     * Re-sort the destination library and select the copied items.
     * TODO: Why am I sorting this again when the items are inserted in sorted
     * order?
     */
    if (destTxLib->sort == TS_ORDER_ADDED) {
        for (i = 0; i < (signed) newLibSelItemCnt; i++) {
            ListBox_SetSel(destLbx, TRUE, newLibSelItems[i]);
        }
    } else {
        mw_SortLib(mainWnd, destLibIndex, destTxLib->sort);
    }
    /*
     * Flip the icon on the copy button so it points to the other library.
     */
    InvalidateRect(mw_copyBtn, NULL, FALSE);
}

/*
 * DeleteLibItems() - Deletes the selected items from a library.
 */
BOOL mw_DeleteLibItems(HWND mainWnd)
{
    TXLIB *txLib = &Prog_lib[mw_libSelIndex];
    HWND listBox = mw_libLbx[mw_libSelIndex];
    _TUCHAR msg[80];
    _TUCHAR itemCntText[10];
    int i;

    assert(mw_libSelIndex == 0 || mw_libSelIndex == 1);
    _stprintf(msg, _T("Are you sure you want to delete the items that are ")
            _T("selected in Library %d?"), mw_libSelIndex + 1);
    if (IDNO == MessageBox(mainWnd, msg, _T("Confirm Delete")
                , MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
    {
        return FALSE;
    }
    assert(mw_libSelItemCnt <= Prog_lib[mw_libSelIndex].count);
    for (i = mw_libSelItemCnt - 1; i >= 0; i--) {
        ListBox_DeleteString(listBox, mw_libSelItems[i]);
        TxLib_DeleteItem(txLib, mw_libSelItems[i]);
        InvalidateRect(mw_libTbx[mw_libSelIndex], NULL, TRUE);
    }
    /*
     * Update the count static.
     */
    _stprintf(itemCntText, _T("%d"), TxLib_GetCount(txLib));
    Static_SetText(mw_libCountStc[mw_libSelIndex], itemCntText);
    mw_libSelItemCnt = 0;
    mw_libSelIndex = -1;

    return TRUE;
}

/*
 * DirtyPrompt() - Checks a file for dirtiness and if so prompts user to save
 *                 with a Yes/No/Cancel dialog.  If the user selects Cancel,
 *                 the function returns TRUE.
 */
BOOL mw_DirtyPrompt(HWND mainWnd, WHICHFILE whichFile)
{
    int promptResult;

    if (whichFile == WF_SNAPSHOT) {
        if (Snapshot_IsDirty(&Prog_snapshot)) {
            if (Prog_snapshotAutoSave
                    && Prog_snapshot.extra.fileName[0] != '\0')
            {
                goto SaveFile;
            }
            promptResult = mw_DoesUserWantToSave(mainWnd, WF_SNAPSHOT);
            if (promptResult == IDCANCEL) {
                return TRUE;
            } else if (promptResult == IDYES) {
SaveFile:
                mw_SaveFile(mainWnd, WF_SNAPSHOT, FALSE);
            }
        }
    } else {
        if (TxLib_IsDirty(&Prog_lib[whichFile])) {
            promptResult = mw_DoesUserWantToSave(mainWnd, whichFile);
            if (promptResult == IDCANCEL) {
                return TRUE;
            } else if (promptResult == IDYES) {
                mw_SaveFile(mainWnd, whichFile, FALSE);
            }
        }
    }
    return FALSE;
}

/*
 * DoesUserWantToSave() - Asks the user if he wants to save a dirty file.
 */
int mw_DoesUserWantToSave(HWND mainWnd, WHICHFILE whichFile)
{
    _TUCHAR prompt[80];
    static const _TUCHAR *promptFmt
        = _T("Do you want to save the changes that were made to %s?");
    const _TUCHAR *fileCategory;

    if (whichFile == WF_SNAPSHOT) {
        fileCategory = _T("the snapshot");
    } else if (whichFile == WF_LIB1) {
        fileCategory = _T("Library 1");
    } else {
        fileCategory = _T("Library 2");
    }
    _stprintf(prompt, promptFmt, fileCategory);
    return MessageBox(mainWnd, prompt, Prog_name
            , MB_YESNOCANCEL | MB_ICONWARNING);
}

/*
 * DrawEdges() - Draws the graphical edges separating the different parts of
 *               the window.
 */
void mw_DrawEdges(HDC dC)
{
    RECT *edgeRect;
    RECT topEdgeRect;

    /*
     * Draw top horizontal edge.  The top edge never moves, so it never needs
     * to be erased, so it's position doesn't need to be saved in the
     * mw_edgeRects array.
     */
    topEdgeRect.left = 0;
    topEdgeRect.right = mw_snapshotLblArea.x;
    topEdgeRect.top = 7;
    topEdgeRect.bottom = topEdgeRect.top + 2;
    DrawEdge(dC, &topEdgeRect, EDGE_ETCHED, BF_TOP);
    topEdgeRect.left = AREA_R(mw_snapshotLblArea);
    topEdgeRect.right = mw_clientRect.right;
    DrawEdge(dC, &topEdgeRect, EDGE_ETCHED, BF_TOP);
    /*
     * Draw middle horizontal edge.
     */
    edgeRect = &mw_edgeRects[0];
    edgeRect->left = AREA_R(mw_libLblArea[1]) + 1;
    edgeRect->right = mw_clientRect.right;
    edgeRect->top = mw_hSplitTop + 7;
    edgeRect->bottom = edgeRect->top + 2;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_TOP);
    edgeRect = &mw_edgeRects[1];
    edgeRect->left = AREA_R(mw_libLblArea[0]) + 1;
    edgeRect->right = mw_libLblArea[1].x - 1;
    edgeRect->top = mw_hSplitTop + 7;
    edgeRect->bottom = edgeRect->top + 2;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_TOP);
    /*
     * Draw top center vertical edge.
     */
    edgeRect = &mw_edgeRects[2];
    edgeRect->left = Area_HCenter(&mw_copyBtnArea);
    edgeRect->right = edgeRect->left + 2;
    edgeRect->top = mw_hSplitTop + 8;
    edgeRect->bottom = mw_buttonGroupTop;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_LEFT);
    /*
     * Draw horizontal edge above the library buttons.
     */
    edgeRect = &mw_edgeRects[3];
    edgeRect->left = AREA_R(mw_libLbxArea[0]) + 2;
    edgeRect->right = mw_libLbxArea[1].x - 2;
    edgeRect->top = mw_buttonGroupTop;
    edgeRect->bottom = edgeRect->top + 2;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_TOP);
    /*
     * Draw horizontal edge below the library buttons.
     */
    edgeRect = &mw_edgeRects[4];
    edgeRect->left = AREA_R(mw_libLbxArea[0]) + 2;
    edgeRect->right = mw_libLbxArea[1].x - 2;
    edgeRect->top = mw_buttonGroupBottom;
    edgeRect->bottom = edgeRect->top + 2;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_TOP);
    /*
     * Draw bottom center vertical edge.
     */
    edgeRect = &mw_edgeRects[5];
    edgeRect->left = Area_HCenter(&mw_copyBtnArea);
    edgeRect->right = edgeRect->left + 2;
    edgeRect->top = mw_buttonGroupBottom + 1;
    edgeRect->bottom = mw_clientRect.bottom;
    DrawEdge(dC, edgeRect, EDGE_ETCHED, BF_LEFT);
}

/*
 * EditItem() - Opens the editor for the selected item in the snapshot list box.
 */
void mw_EditItem(HWND mainWnd, BOOL listBoxDblClk)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    int bankIndex;
    int sIdx = mw_snapshotSelItems[0];
    METAINDEX type = Snapshot_GetItemType(sIdx);
    
    switch (type) {
        case META_VCED:
            TX81Z_SwitchToVoiceMode(Prog_midi);
            VoiceDlg_Create(mainWnd, &snapshot->avced);
            break;
        case META_PCED:
            TX81Z_SwitchToPfmMode(Prog_midi);
            PfmDlg_Create(mainWnd, &snapshot->pced);
            break;
        case META_FX:
            FxDlg_Create(mainWnd, &snapshot->fx);
            break;
        case META_PC:
            PcDlg_Create(mainWnd, &snapshot->pc);
            break;
        case META_MTO:
            MtoDlg_Create(mainWnd, &snapshot->mto);
            break;
        case META_MTF:
            MtfDlg_Create(mainWnd, &snapshot->mtf);
            break;
        case META_SYS:
            SysDlg_Create(mainWnd, &snapshot->sys);
            break;
        case META_VMEM:
        case META_PRESET_A:
        case META_PRESET_B:
        case META_PRESET_C:
        case META_PRESET_D:
            bankIndex = Snapshot_GetBankIndex(sIdx);
            TX81Z_ProgramChange(Prog_midi, mw_typeToBank[type], bankIndex);
            TXPack_VmemToAvced(((VMEM *) (((BYTE *) snapshot)
                        + TX81Z_meta[type].offset))->data[bankIndex]
                    , &snapshot->avced, Snapshot_IsCombineModeOn(snapshot));
            Snapshot_DirtyItem(snapshot, SI_VCED);
            mw_UpdateSnapshotLbxItem(listBoxDblClk ? TRUE : FALSE, SI_VCED
                    , SELECT_PREVIOUS);
            if (!listBoxDblClk || !Prog_dblClkingBankDoesntOpen) {
                VoiceDlg_Create(mainWnd, &snapshot->avced);
            }
            break;
        case META_PMEM:
            bankIndex = Snapshot_GetBankIndex(sIdx);
            TX81Z_ProgramChange(Prog_midi, TX81Z_BANK_PF, bankIndex);
            TXPack_PmemToPced(snapshot->pmem.data[bankIndex]
                    , &snapshot->pced);
            Snapshot_DirtyItem(snapshot, SI_PCED);
            mw_UpdateSnapshotLbxItem(listBoxDblClk ? TRUE : FALSE, SI_PCED
                    , SELECT_PREVIOUS);
            if (!listBoxDblClk || !Prog_dblClkingBankDoesntOpen) {
                PfmDlg_Create(mainWnd, &snapshot->pced);
            }
            break;
    }
}

/*
 * EraseEdges() - Erases the graphical edges separating the different parts of
 *                the window.
 */
void mw_EraseEdges(HDC dC)
{
    int i;

    for (i = 0; i < mw_edgeCnt; i++) {
        FillRect(dC, &mw_edgeRects[i], Prog_dlgBrush);
    }
}

/*
 * ExportLib()
 */
void mw_ExportLib(HWND mainWnd, int libIndex)
{
    TxLib_Export(mainWnd, &Prog_lib[libIndex], mw_libSelItems
            , mw_libSelItemCnt);
}

/*
 * GetSelItems() - Puts the selected item numbers of a list box into
 *                 mw_snapshotSelItems or mw_libSelItems, which is realloced
 *                 if the items won't fit.
 */
void mw_GetSelItems(HWND listBox)
{
    if (listBox == mw_snapshotLbx) {
        mw_snapshotSelItemCnt = ListBox_GetSelCount(listBox);
        if (mw_snapshotSelItemCnt > mw_snapshotSelItemMax) {
            mw_snapshotSelItemMax = mw_snapshotSelItemCnt + 16;
            mw_snapshotSelItems = realloc(mw_snapshotSelItems
                    , sizeof(int) * mw_snapshotSelItemMax);
        }
        ListBox_GetSelItems(listBox, mw_snapshotSelItemCnt, mw_snapshotSelItems);
    } else {
        mw_libSelItemCnt = ListBox_GetSelCount(listBox);
        if (mw_libSelItemCnt == 0) {
            mw_libSelIndex = -1;
        } else {
            mw_libSelIndex = listBox == mw_libLbx[0] ? 0 : 1;
            if (mw_libSelItemCnt > mw_libSelItemMax) {
                mw_libSelItemMax = mw_libSelItemCnt + 16;
                mw_libSelItems = realloc(mw_libSelItems
                        , sizeof(int) * mw_libSelItemMax);
            }
            ListBox_GetSelItems(listBox, mw_libSelItemCnt, mw_libSelItems);
        }
    }
}

/*
 * ImportLib()
 */
void mw_ImportLib(HWND mainWnd, int libIndex)
{
    _TUCHAR multiFileNames[mw_multiFileNameLen];

    if (ImportDlg_Create(mainWnd, Prog_importRecentDir, multiFileNames
                , mw_multiFileNameLen))
    {
        TxLib_Import(mainWnd, &Prog_lib[libIndex], Prog_importOptions
                , multiFileNames);
        mw_UpdateLibLbx(libIndex, UL_LBX);
        InvalidateRect(mw_libTbx[libIndex], NULL, TRUE);
    }
}

/*
 * InitMenuButtons()
 */
void mw_InitMenuButtons(void)
{
    int i;

    /*
     * Link the button icons and menus to the file and sort buttons.
     */
    MenuBtn_Deinit(mw_snapshotFileBtn);
    Button_SetBitmap(mw_snapshotFileBtn, Prog_fileBmp);
    MenuBtn_Init(mw_snapshotFileBtn, Prog_snapshotFileMenu, NULL
            , Prog_menuFlags);
    for (i = 0; i < 2; i++) {
        MenuBtn_Deinit(mw_libFileBtn[i]);
        Button_SetBitmap(mw_libFileBtn[i], Prog_fileBmp);
        MenuBtn_Init(mw_libFileBtn[i], Prog_libFileMenu, NULL, Prog_menuFlags);
        MenuBtn_Deinit(mw_libSortBtn[i]);
        Button_SetBitmap(mw_libSortBtn[i], Prog_sortBmp);
        MenuBtn_Init(mw_libSortBtn[i], Prog_libSortMenu, NULL, Prog_menuFlags);
    }
}

/*
 * NewLib() - Called when user selects New from one of the library file menus.
 */
BOOL mw_NewLib(HWND mainWnd, int libIndex)
{
    if (mw_DirtyPrompt(mainWnd, libIndex)) {
        return FALSE;
    }
    TxLib_New(&Prog_lib[libIndex]);
    mw_UpdateLibLbx(libIndex, UL_ALL);

    return TRUE;
}

/*
 * NewSnapshot() - Called when the user selects New from the snapshot file menu.
 */
BOOL mw_NewSnapshot(HWND mainWnd)
{
    /*
     * If the snapshot is dirty, prompt the user if he wants to save it.
     */
    if (mw_DirtyPrompt(mainWnd, WF_SNAPSHOT)) {
        return FALSE;
    }
    /*
     * Clear out the snapshot.
     */
    Snapshot_Init(&Prog_snapshot);
    /*
     * Clear the snapshot selections.
     */
    mw_snapshotSelItemCnt = 0;
    /*
     * Update the listbox.
     */
    mw_UpdateSnapshotLbx(UL_ALL);

    return TRUE;
}

/*
 * OnActivate() - Sets the active window variable so the main message loop can 
 *                handle accelerators properly.
 */
void mw_OnActivate(HWND mainWnd, UINT state, HWND otherWnd, BOOL minimized)
{
    Prog_activeWnd = mainWnd;
}

/*
 * OnClose() - Prompts to save before exiting.
 */
void mw_OnClose(HWND mainWnd)
{
    if (mw_DirtyPrompt(mainWnd, WF_SNAPSHOT)
            || mw_DirtyPrompt(mainWnd, WF_LIB1)
            || mw_DirtyPrompt(mainWnd, WF_LIB2))
    {
        return;
    }
    DestroyWindow(mainWnd);
}

/*
 * OnCommand()
 */
void mw_OnCommand(HWND mainWnd, UINT ctrlID, HWND ctrl, UINT notify)
{
    TXLIB *txLib;
    int libIndex;
    _TUCHAR text[80];

    switch (ctrlID) {
        /*
         * Menus
         */
        case IDM_EXIT:
            SendMessage(mainWnd, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
            break;
        case IDM_MIDI_SETUP:
            Midi_SetupDialog(Prog_midi, mainWnd, Prog_instance, 0, NULL
                    , _T("Configure your MIDI connections to the TX81Z."));
            break;
        case IDM_SHOW_FULL_PATHS:
            MenuItem_Toggle(mw_menu, IDM_SHOW_FULL_PATHS, &Prog_showFullPaths);
            mw_UpdateSnapshotLbx(UL_TBX);
            mw_UpdateLibLbx(0, UL_TBX);
            mw_UpdateLibLbx(1, UL_TBX);
            break;
        case IDM_OPEN_PREVIOUS:
            MenuItem_Toggle(mw_menu, IDM_OPEN_PREVIOUS, &Prog_openPrevious);
            break;
        case IDM_STARTUP_EDIT_BUFFER_REQUEST:
            MenuItem_Toggle(mw_menu, IDM_STARTUP_EDIT_BUFFER_REQUEST
                    , &Prog_startupEditBufferRequest);
            break;
        case IDM_DBLCLKING_BANK_DOESNT_OPEN:
            MenuItem_Toggle(mw_menu, IDM_DBLCLKING_BANK_DOESNT_OPEN
                    , &Prog_dblClkingBankDoesntOpen);
            break;
        case IDM_MAINWND_TO_FRONT:
            MenuItem_Toggle(mw_menu, IDM_MAINWND_TO_FRONT
                    , &Prog_mainWndToFront);
            break;
        case IDM_SNAPSHOT_AUTOSAVE:
            MenuItem_Toggle(mw_menu, IDM_SNAPSHOT_AUTOSAVE
                    , &Prog_snapshotAutoSave);
            break;
        case IDM_LIBRARY_NO_DUPLICATES:
            MenuItem_Toggle(mw_menu, IDM_LIBRARY_NO_DUPLICATES
                    , &Prog_libraryNoDuplicates);
            break;
        case IDM_UNSLASHED_ZEROS:
            MenuItem_Toggle(mw_menu, IDM_UNSLASHED_ZEROS, &Prog_unslashedZeros);
            MiniFont_ReplaceZero(!Prog_unslashedZeros);
            InvalidateRect(mainWnd, NULL, FALSE);
            if (Prog_voiceDlg) {
                InvalidateRect(Prog_voiceDlg, NULL, FALSE);
            }
            if (Prog_pfmDlg) {
                InvalidateRect(Prog_pfmDlg, NULL, FALSE);
            }
            if (Prog_fxDlg) {
                InvalidateRect(Prog_fxDlg, NULL, FALSE);
            }
            if (Prog_pcDlg) {
                InvalidateRect(Prog_pcDlg, NULL, FALSE);
            }
            if (Prog_mtoDlg) {
                InvalidateRect(Prog_mtoDlg, NULL, FALSE);
            }
            if (Prog_mtfDlg) {
                InvalidateRect(Prog_mtfDlg, NULL, FALSE);
            }
            if (Prog_sysDlg) {
                InvalidateRect(Prog_sysDlg, NULL, FALSE);
            }
            break;
        case IDM_HIGHLIGHT_COLOR:
            mw_ChooseColor(mainWnd);
            break;
        case IDM_VERSION_10:
        case IDM_VERSION_OTHER:
            Prog_tx81zVersion = ctrlID - IDM_VERSION_ORG;
            MenuItem_RadioCheck(mw_menu, IDM_VERSION_10, IDM_VERSION_OTHER
                    , ctrlID);
            break;
        case IDM_KYBDDLG:
            KybdDlg_Create(mainWnd);
            break;
        case IDM_REMOTEWND:
            Prog_remoteWnd = RemoteWnd_Create(mainWnd);
            break;
        case IDM_MAINWND: /* the other windows send window menu messages here, so I'm putting this here even though it looks pointless */
            if (IsIconic(Prog_mainWnd)) {
                OpenIcon(Prog_mainWnd);
            }
            BringWindowToTop(Prog_mainWnd);
            return;
        case IDM_VOICEDLG:
            VoiceDlg_Create(mainWnd, &Prog_snapshot.avced);
            break;
        case IDM_PFMDLG:
            PfmDlg_Create(mainWnd, &Prog_snapshot.pced);
            break;
        case IDM_FXDLG:
            FxDlg_Create(mainWnd, &Prog_snapshot.fx);
            break;
        case IDM_PCDLG:
            PcDlg_Create(mainWnd, &Prog_snapshot.pc);
            break;
        case IDM_MTODLG:
            MtoDlg_Create(mainWnd, &Prog_snapshot.mto);
            break;
        case IDM_MTFDLG:
            MtfDlg_Create(mainWnd, &Prog_snapshot.mtf);
            break;
        case IDM_SYSDLG:
            SysDlg_Create(mainWnd, &Prog_snapshot.sys);
            break;
        case IDM_ABOUT:
            AboutDlg_Create(mainWnd);
            break;
        case IDM_HELP:
            Prog_OpenHelp(mainWnd, _T("index.html"));
            break;
        case IDM_SNAPSHOT_FILE_NEW:
            if (mw_NewSnapshot(mainWnd)) {
                mw_UpdateEditors(REQ_EDITABLE);
                goto UpdateSnapshotSelection;
            }
            break;
        case IDM_SNAPSHOT_FILE_OPEN:
            if (mw_OpenSnapshotFile(mainWnd, NULL, FALSE)) {
                mw_UpdateEditors(REQ_EDITABLE);
                goto UpdateSnapshotSelection;
            }
            break;
        case IDM_SNAPSHOT_FILE_SAVE:
            mw_SaveFile(mainWnd, WF_SNAPSHOT, FALSE);
            break;
        case IDM_SNAPSHOT_FILE_SAVE_AS:
            mw_SaveFile(mainWnd, WF_SNAPSHOT, TRUE);
            break;
        case IDM_SNAPSHOT_FILE_SAVE_COPY:
            mw_SaveSnapshotCopy(mainWnd);
            break;
        case IDM_SNAPSHOT_FILE_LOAD_COPY:
            mw_OpenSnapshotFile(mainWnd, NULL, TRUE);
            mw_UpdateEditors(REQ_EDITABLE);
            break;
        case IDM_LIB_FILE_NEW:
            libIndex = mw_libMenuIndex;
            if (mw_NewLib(mainWnd, libIndex)) {
                goto UpdateLibSelection;
            }
            break;
        case IDM_LIB_FILE_SAVE:
            mw_SaveFile(mainWnd, mw_libMenuIndex, FALSE);
            break;
        case IDM_LIB_FILE_SAVE_AS:
            mw_SaveFile(mainWnd, mw_libMenuIndex, TRUE);
            break;
        case IDM_LIB_FILE_OPEN:
            libIndex = mw_libMenuIndex;
            if (mw_OpenLibraryFile(mainWnd, libIndex, NULL)) {
                goto UpdateLibSelection;
            }
            break;
        case IDM_LIB_FILE_IMPORT:
            mw_ImportLib(mainWnd, mw_libMenuIndex);
            break;
        case IDM_LIB_FILE_EXPORT:
            mw_ExportLib(mainWnd, mw_libMenuIndex);
            break;
        case IDM_LIB_SORT_ORDER_ADDED:
            mw_SortLib(mainWnd, mw_libMenuIndex, TS_ORDER_ADDED);
            break;
        case IDM_LIB_SORT_TYPE:
            mw_SortLib(mainWnd, mw_libMenuIndex, TS_TYPE);
            break;
        case IDM_LIB_SORT_NAME:
            mw_SortLib(mainWnd, mw_libMenuIndex, TS_NAME);
            break;
        case IDM_LIB_SORT_COMMENT:
            mw_SortLib(mainWnd, mw_libMenuIndex, TS_COMMENT);
            break;
        case IDM_LIB_SORT_SELECTION:
            mw_SortLib(mainWnd, mw_libMenuIndex, TS_SELECTION);
            break;
        case IDM_LIB_SORT_SEARCH:
            if (SearchDlg_Create(mainWnd)) {
                mw_SortLib(mainWnd, mw_libMenuIndex, TS_SEARCH);
            }
            break;
        /*
         * Buttons
         */
        case IDC_ITEM_EDIT_BTN:
            mw_EditItem(mainWnd, FALSE);
            break;
        case IDC_TRANSMIT_PC_BTN:
        {
            int sIdx = mw_snapshotSelItems[0];
            METAINDEX type = Snapshot_GetItemType(sIdx);
            int bankIndex = Snapshot_GetBankIndex(sIdx);

            TX81Z_ProgramChange(Prog_midi, mw_typeToBank[type], bankIndex);
            break;
        }
        case IDC_ITEM_ADD_TO_LIB0_BTN:
            libIndex = 0;
            goto AddToLib;
        case IDC_ITEM_ADD_TO_LIB1_BTN:
            libIndex = 1;
AddToLib:
            MainWnd_AddToLib(mainWnd, libIndex, mw_snapshotSelItems
                    , mw_snapshotSelItemCnt);
            break;
        case IDC_ITEM_BNDL_TO_LIB0_BTN:
            libIndex = 0;
            goto BundleToLib;
        case IDC_ITEM_BNDL_TO_LIB1_BTN:
            libIndex = 1;
BundleToLib:
            MainWnd_BundleToLib(mainWnd, libIndex, mw_snapshotSelItems
                    , mw_snapshotSelItemCnt);
            break;
        case IDC_ITEM_COPY_FROM_LIB_BTN:
            mw_CopyFromLib(mainWnd);
            break;
        case IDC_SNAPSHOT_RX_ITEMS_BTN:
            mw_SnapshotRxItems(mainWnd);
            break;
        case IDC_SNAPSHOT_TX_ITEMS_BTN:
            mw_SnapshotTxItems(mainWnd);
            break;
        case IDC_ITEM_STORE_BTN:
        {
            int srcIndex = mw_snapshotSelItems[0];
            int destIndex;

            if (!Snapshot_IsItemGroupLoaded(&Prog_snapshot, META_VMEM)) {
                mw_RetrieveAndUpdate(REQ_VMEM);
            }
            if (StoreDlg_Create(mainWnd, Snapshot_GetItemType(srcIndex)
                    , &destIndex))
            {
                MainWnd_StoreItem(srcIndex, destIndex);
            }
            break;
        }
        case IDC_SNAPSHOT_RX_BTN:
            mw_SnapshotRxData(mainWnd);
            break;
        case IDC_SNAPSHOT_TX_BTN:
            mw_SnapshotTxData(mainWnd);
            break;
        case IDC_COPY_BTN:
            mw_CopyLibItems(mainWnd);
            break;
        case IDC_DELETE_BTN:
            libIndex = mw_libSelIndex;
            if (mw_DeleteLibItems(mainWnd)) {
                goto UpdateLibSelection;
            }
            break;
        case IDC_RENAME_BTN:
        {
            RENAMEINFO renameInfo;
            int itemIndex;

            renameInfo.name = text;
            renameInfo.useCount = mw_libSelItemCnt > 1;
            libIndex = mw_libSelIndex;
            txLib = &Prog_lib[libIndex];
            itemIndex = mw_libSelItems[0];
            /*
             * Get the first selected item's name.
             */
            TxLib_SetPtr(txLib, itemIndex);
            TxLib_GetItemName(txLib, text); /* text == renameinfo.name */
            /*
             * Show the name editor.
             */
            if (RenameDlg_Create(mainWnd, &renameInfo)) {
                int i;

                /*
                 * For each selected item.
                 */
                for (i = 0; i < mw_libSelItemCnt; i++) {
                    itemIndex = mw_libSelItems[i];

                    /*
                     * If the count option was enabled.
                     */
                    if (renameInfo.useCount) {
                        /*
                         * Insert the current count into the copy of the name.
                         */
                        int count = renameInfo.countStart + i;
                        int j;

                        for (j = renameInfo.countPos; j >= 0; j--) {
                            text[j] = '0' + count % 10;
                            count /= 10;
                            if (count == 0) {
                                break;
                            }
                        }
                    }
                    /*
                     * Set the library item's name.
                     */
                    TxLib_SetPtr(txLib, itemIndex);
                    TxLib_SetItemName(txLib, text);
                }
                goto SortAndUpdateLibLbx;
            }
            break;
        }
        case IDC_RECOMMENT_BTN:
        {
            _TUCHAR comment[TXLIBITEM_COMMENT_SIZE];
            int itemIndex;

            libIndex = mw_libSelIndex;
            txLib = &Prog_lib[libIndex];
            itemIndex = mw_libSelItems[0];
            /*
             * Get the text of the first selected item's comment.
             */
            TxLib_SetPtr(txLib, itemIndex);
            TxLib_GetItemComment(txLib, comment);
            /*
             * Use it to initialize the comment editor dialog.
             */
            if (RecommentDlg_Create(mainWnd, comment)) {
                int i;

                /*
                 * Go through all the selected items.
                 */
                for (i = 0; i < mw_libSelItemCnt; i++) {
                    itemIndex = mw_libSelItems[i];
                    /*
                     * Set the library item's comment.
                     */
                    TxLib_SetPtr(txLib, itemIndex);
                    TxLib_SetItemComment(txLib, comment);
                }
SortAndUpdateLibLbx:
                /*
                 * Invalidate the file name text box, since it's not being
                 * painted right.
                 */
                InvalidateRect(mw_libTbx[libIndex], NULL, TRUE);
                /*
                 * Resort the library if the changes will affect the sort order.
                 */
                if ((ctrlID == IDC_RENAME_BTN && (txLib->sort == TS_NAME
                                || (txLib->sort == TS_SEARCH
                                    && (TxLib_searchType == TST_NAME
                                        || TxLib_searchType
                                            == TST_NAME_AND_COMMENT))))
                        || (ctrlID == IDC_RECOMMENT_BTN
                            && (txLib->sort == TS_COMMENT
                                || (txLib->sort == TS_SEARCH
                                    && (TxLib_searchType == TST_COMMENT
                                        || TxLib_searchType
                                            == TST_NAME_AND_COMMENT)))))
                {
                    mw_SortLib(mainWnd, libIndex, txLib->sort);
                } else {
                    mw_UpdateLibLbx(libIndex, UL_LBX);
                }
            }
            break;
        }
        case IDC_DIFF_BTN:
        {
            BYTE *libItem1Data, *libItem2Data;
            TXLIB *txLib = &Prog_lib[mw_libSelIndex];

            TxLib_SetPtr(txLib, mw_libSelItems[0]);
            libItem1Data = TxLib_GetItemData(txLib);
            TxLib_SetPtr(txLib, mw_libSelItems[1]);
            libItem2Data = TxLib_GetItemData(txLib);
            DiffDlg_Create(mainWnd, libItem1Data, libItem2Data
                    , TxLib_GetItemType(txLib));
            break;
        }
        /*
         * Menu buttons
         */
        case IDC_SNAPSHOT_FILE_BTN:
            mw_UpdateRecentMenu(Prog_snapshotFileMenu, Prog_snapshotRecent
                    , mw_originalSnapshotMenuItemCnt, Prog_snapshot.extra.fileName
                    , IDM_SNAPSHOT_FILE_RECENT);
            break;
        case IDC_LIB0_FILE_BTN:
            mw_libMenuIndex = 0;
            goto UpdateLibFileMenu;
        case IDC_LIB1_FILE_BTN:
            mw_libMenuIndex = 1;
UpdateLibFileMenu:
            mw_UpdateRecentMenu(Prog_libFileMenu, Prog_libRecent[mw_libMenuIndex]
                    , mw_originalLibMenuItemCnt, Prog_lib[mw_libMenuIndex].fileName
                    , IDM_LIB_FILE_RECENT);
            break;
        case IDC_LIB0_SORT_BTN:
            mw_libMenuIndex = 0;
            goto UpdateSortMenu;
        case IDC_LIB1_SORT_BTN:
            mw_libMenuIndex = 1;
UpdateSortMenu:
            CheckMenuRadioItem(Prog_libSortMenu, 0, 5, Prog_lib[mw_libMenuIndex].sort
                    , MF_BYPOSITION);
            break;
        /*
         * List boxes
         */
        case IDC_SNAPSHOT_LBX:
            if (notify == LBN_DBLCLK) {
                mw_EditItem(mainWnd, TRUE);
            } else if (notify == LBN_SELCHANGE) {
UpdateSnapshotSelection:
                /*
                 * Update the item selection cache.
                 */
                mw_GetSelItems(mw_snapshotLbx);
                /*
                 * Update the selection count static.
                 */
                _stprintf(text, _T("%d"), mw_snapshotSelItemCnt);
                Static_SetText(mw_snapshotSelectedStc, text);
                /*
                 * Update the buttons.
                 */
                mw_UpdateButtonStates();
            }
            break;
        case IDC_LIB0_LBX:
            libIndex = 0;
            goto LibIndexSet;
        case IDC_LIB1_LBX:
            libIndex = 1;
LibIndexSet:
            if (notify == LBN_DBLCLK) {
                mw_SendLibItem(mainWnd, libIndex);
            } else if (notify == LBN_SELCHANGE) {
                goto UpdateLibSelection;
            } else if (notify == LBN_SETFOCUS) {
                /*
                 * Allow only one selection at a time among both lib list boxes.
                 */
                ListBox_SetSel(mw_libLbx[1 - libIndex], FALSE, -1);
UpdateLibSelection:
                /*
                 * Update the selection cache.
                 */
                mw_GetSelItems(mw_libLbx[libIndex]);
                /*
                 * Update the selection count static.
                 */
                _stprintf(text, _T("%d"), mw_libSelItemCnt);
                Static_SetText(mw_libSelectedStc[libIndex], text);
                Static_SetText(mw_libSelectedStc[1 - libIndex], _T("0"));
                /*
                 * Enable or disable buttons.
                 */
                mw_UpdateButtonStates();
            }
            break;
        /*
         * TX81Z notifications
         */
        case IDD_TX81ZDLG:
            switch (notify) {
                case TXN_RECEIVED_VCED:
                    Snapshot_LoadSysex(&Prog_snapshot, META_VCED
                            , TX81Z_meta[META_VCED].buf);
                    Snapshot_LoadSysex(&Prog_snapshot, META_ACED
                            , TX81Z_meta[META_ACED].buf);
                    mw_UpdateSnapshotLbxItem(TRUE, SI_VCED, SELECT_PREVIOUS);
                    goto RedrawTbx;
                case TXN_RECEIVED_PCED:
                    Snapshot_LoadSysex(&Prog_snapshot, META_PCED
                            , TX81Z_meta[META_PCED].buf);
                    mw_UpdateSnapshotLbxItem(TRUE, SI_PCED, SELECT_PREVIOUS);
                    goto RedrawTbx;
                case TXN_VERSION_10_DETECTED:
                    Prog_tx81zVersion = 10;
                    MenuItem_RadioCheck(mw_menu, IDM_VERSION_10
                            , IDM_VERSION_OTHER, IDM_VERSION_10);
                    break;
                case TXN_VERSION_OTHER_DETECTED:
                    Prog_tx81zVersion = 11;
                    MenuItem_RadioCheck(mw_menu, IDM_VERSION_10
                            , IDM_VERSION_OTHER, IDM_VERSION_OTHER);
                    break;
            }
            break;
        case IDD_VOICEDLG:
        case IDD_PFMDLG:
        case IDD_FXDLG:
        case IDD_PCDLG:
        case IDD_MTODLG:
        case IDD_MTFDLG:
        case IDD_SYSDLG:
            if (notify == EDN_CHANGE) {
                int sIdx = SI_VCED + (ctrlID - IDD_VOICEDLG);

                Snapshot_DirtyItem(&Prog_snapshot, sIdx);
                mw_UpdateSnapshotLbxItem(FALSE
                        , SI_VCED + (ctrlID - IDD_VOICEDLG), SELECT_PREVIOUS);
RedrawTbx:
                InvalidateRect(mw_snapshotTbx, NULL, TRUE);
            }
            break;
        default:
            if (ctrlID >= IDM_SNAPSHOT_FILE_RECENT
                    && ctrlID < IDM_SNAPSHOT_FILE_RECENT + Prog_recentFileMax)
            {
                mw_OpenSnapshotFile(mainWnd
                        , Prog_snapshotRecent[ctrlID - IDM_SNAPSHOT_FILE_RECENT]
                        , FALSE);
                mw_UpdateEditors(REQ_EDITABLE);
            } else if (ctrlID >= IDM_LIB_FILE_RECENT
                    && ctrlID < IDM_LIB_FILE_RECENT + Prog_recentFileMax)
            {
                mw_OpenLibraryFile(mainWnd, mw_libMenuIndex
                        , Prog_libRecent[mw_libMenuIndex][ctrlID - IDM_LIB_FILE_RECENT]);
            }
            break;
    }
}

/*
 * OnCreate()
 */
BOOL mw_OnCreate(HWND mainWnd, LPCREATESTRUCT createStruct)
{
    MMRESULT error;

    /*
     * Create the TX81Z message window.
     */
    Prog_mainWnd = mainWnd;
    if ((Prog_tx81zWnd = TX81Z_Create(mainWnd)) == NULL)
        return FALSE;

    /*
     * Set up the MIDI device structure and open it for output.
     */
    Prog_midi->wnd = Prog_tx81zWnd;
    if (!Midi_LookupPortIndexes(Prog_midi)) {
        Midi_SetupDialog(Prog_midi, mainWnd, Prog_instance, 0, NULL
                , _T("The MIDI port settings for the TX81Z can't be found ")
                _T("in the registry or aren't valid.  Please configure your ")
                _T("MIDI connections to the TX81Z."));
        return TRUE;
    }
    if (error = Midi_OutOpen(Prog_midi)) {
        Error_MidiError(mainWnd, Error_cannotOpenMidiOut, error);
        Error_GuiMsg(_T("Midi out will be disabled."));
        Prog_midi->outPort = -1;
    }
    if (error = Midi_InOpen(Prog_midi)) {
        Error_MidiError(mainWnd, Error_cannotOpenMidiIn, error);
        Error_GuiMsg(_T("Midi in will be disabled."));
        Prog_midi->inPort = -1;
    }
    if (error = Midi_MasterInOpen(Prog_midi)) {
        Error_MidiError(mainWnd, Error_cannotOpenMidiMasterIn, error);
        Error_GuiMsg(_T("The master controller port will be disabled."));
        Prog_midi->masterInPort = -1;
    }

    return TRUE;
}

/*
 * OnCtlColor()
 */
HBRUSH mw_OnCtlColor(HWND mainWnd, HDC dC, HWND childWnd, int type)
{
    COLORREF textColor = Prog_wndTextColor;

    if (childWnd == mw_snapshotTbx) {
        if (Snapshot_IsDirty(&Prog_snapshot)) {
            textColor = Prog_dirtyColor;
        }
    } else if (childWnd == mw_libTbx[0]) {
        if (TxLib_IsDirty(&Prog_lib[0])) {
            textColor = Prog_dirtyColor;
        }
    } else if (childWnd == mw_libTbx[1]) {
        if (TxLib_IsDirty(&Prog_lib[1])) {
            textColor = Prog_dirtyColor;
        }
    }
    SetTextColor(dC, textColor);
    SetBkColor(dC, Prog_dlgColor);

    return Prog_dlgBrush;
}

/*
 * OnDestroy()
 */
void mw_OnDestroy(HWND mainWnd)
{
    Midi_MasterInClose(Prog_midi);
    Midi_OutClose(Prog_midi);
    Midi_InClose(Prog_midi);
    free(mw_snapshotSelItems);
    free(mw_libSelItems);
    TX81Z_Destroy(Prog_tx81zWnd);
    TxLib_DeInit(&Prog_lib[0]);
    TxLib_DeInit(&Prog_lib[1]);
    GetWindowPlacement(mainWnd, &Prog_mainWndPlacement);
    PostQuitMessage(EXIT_SUCCESS);
}

/*
 * OnDrawItem()
 */
void mw_OnDrawItem(HWND mainWnd, const DRAWITEMSTRUCT *drawItem)
{
    HDC dC = drawItem->hDC;
    UINT ctrlType = drawItem->CtlType;
    UINT ctrlID = drawItem->CtlID;

    if (ctrlType == ODT_LISTBOX) {
        TxLbx_OnDrawItem(mainWnd, drawItem);
    } else if (ctrlType == ODT_MENU) {
        COLORREF oldTextColor = SetTextColor(dC, Prog_wndTextColor);
        RECT itemRect = drawItem->rcItem;
        HFONT oldFont;
        
        ctrlID = drawItem->itemID;
        itemRect.left += GetSystemMetrics(SM_CXMENUCHECK)
                + GetSystemMetrics(SM_CXDLGFRAME) + 1;
        oldFont = SelectFont(dC, Prog_tahomaBoldFont);
        DrawText(dC, mw_optionMenuHeaders[ctrlID - IDM_START_OPTIONS], -1
                , &itemRect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
        SetTextColor(dC, oldTextColor);
        SelectFont(dC, oldFont);
    } else if (ctrlID >= IDC_SNAPSHOT_FILE_BTN && ctrlID <= IDC_LIB1_SORT_BTN) {
        MenuBtn_DrawButton(mainWnd, drawItem);
        MenuBtn_DrawArrow(mainWnd, drawItem);
    } else if (ctrlType == ODT_BUTTON) {
        RECT itemRect = drawItem->rcItem;
        HDC memDC = CreateCompatibleDC(dC);
        BOOL disabled = drawItem->itemState & ODS_DISABLED;
        int leftOffset = 3;
        int topOffset = 4;
        const DRAWBTN *drawBtn;

        /*
         * Draw the button border.
         */
        if (drawItem->itemState & ODS_SELECTED) {
            DrawEdge(dC, &itemRect, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
            leftOffset++;
            topOffset++;
        } else {
            DrawEdge(dC, &itemRect, EDGE_RAISED, BF_RECT | BF_MIDDLE);
        }
        /*
         * Draw the button icon.
         */
        if (drawItem->CtlID == IDC_COPY_BTN) {
            drawBtn = &mw_drawBtns[mw_libSelIndex == 1
                ? DRAWBTN_COPY_LEFT_INDEX : DRAWBTN_COPY_RIGHT_INDEX];
        } else {
            drawBtn = &mw_drawBtns[ctrlID - IDC_DELETE_BTN];
        }
        SelectBitmap(memDC, disabled ? *(drawBtn->disabledBmp)
                : *(drawBtn->bmp));
        BitBlt(dC, itemRect.left + leftOffset + 2
                , itemRect.top + topOffset, drawBtn->bmpWidth, 15
                , memDC, 0, 0, SRCCOPY);
        DeleteDC(memDC);
        /*
         * Draw the button text.
         */
        SetBkMode(dC, TRANSPARENT);
        itemRect.left += leftOffset + drawBtn->textX;
        itemRect.top += topOffset + 1;
        if (disabled) {
            SetTextColor(dC, Prog_3dHighlightColor);
            TextOut(dC, itemRect.left + 1, itemRect.top + 1, drawBtn->text
                    , drawBtn->textLen);
            SetTextColor(dC, Prog_3dShadowColor);
            TextOut(dC, itemRect.left, itemRect.top, drawBtn->text
                    , drawBtn->textLen);
        } else {
            TextOut(dC, itemRect.left, itemRect.top, drawBtn->text
                    , drawBtn->textLen);
        }
    }
}

/*
 * OnEraseBkgnd()
 */
BOOL mw_OnEraseBkgnd(HWND mainWnd, HDC dC)
{
    RECT rect;

    GetClientRect(mainWnd, &rect);
    mw_clientRect.right = rect.right;
    mw_clientRect.bottom = rect.bottom;
    FillRect(dC, &mw_clientRect, Prog_dlgBrush);

    mw_DrawEdges(dC);

    return TRUE;
}

/*
 * OnLButtonDown()
 */
void mw_OnLButtonDown(HWND mainWnd, BOOL dblClick, int x, int y, UINT keyFlags)
{
    BringWindowToTop(mainWnd);
    /*
     * If the cursor is over the horizontal splitter.
     */
    if (y >= mw_hSplitTop && y <= mw_hSplitBottom) {
        /*
         * Flag the fact that the horizontal splitter is being dragged and
         * record the vertical cursor position relative to the splitter.
         */
        SetCapture(mainWnd);
        mw_isCaptureSet = TRUE;
        mw_isDraggingHSplit = TRUE;
        mw_hSplitDragY = y - mw_hSplitTop;
    /*
     * If the cursor is over the vertical splitter.
     */
    } else if (x >= mw_vSplitLeft && x <= mw_vSplitRight
            && y > mw_hSplitBottom) {
        /*
         * Flag the fact that the vertical splitter is being dragged and
         * record the horizontal cursor position relative to the splitter.
         */
        SetCapture(mainWnd);
        mw_isCaptureSet = TRUE;
        mw_isDraggingVSplit = TRUE;
        mw_vSplitDragX = x - mw_vSplitLeft;
    }
}

/*
 * OnLButtonUp()
 */
void mw_OnLButtonUp(HWND mainWnd, int x, int y, UINT keyFlags)
{
    /*
     * If the horizontal splitter was being dragged.
     */
    if (mw_isDraggingHSplit) {
        /*
         * Clear the dragging flag.
         */
        mw_isDraggingHSplit = FALSE;
        ReleaseCapture();
        mw_isCaptureSet = FALSE;
        /*
         * If the cursor is no longer over the horizontal splitter.  (The x
         * coordinate needs to be checked because the mouse is captured)
         */
        if (x < 0 || x > mw_clientRect.right
                || y < mw_hSplitTop || y > mw_hSplitBottom) {
            /*
             * Change it back to the arrow cursor.
             */
            mw_cursor = Prog_arrowCursor;
            SetCursor(mw_cursor);
        }
    } else if (mw_isDraggingVSplit) {
        /*
         * Clear the dragging flag.
         */
        mw_isDraggingVSplit = FALSE;
        ReleaseCapture();
        mw_isCaptureSet = FALSE;
        /*
         * If the cursor is no longer over the vertical splitter.
         */
        if (x < mw_vSplitLeft || x > mw_vSplitRight || y < mw_hSplitBottom
                || y > mw_clientRect.bottom
                || (y >= mw_buttonGroupTop && y <= mw_buttonGroupBottom))
        {
            /*
             * Change it back to the arrow cursor.
             */
            mw_cursor = Prog_arrowCursor;
            SetCursor(mw_cursor);
        }
    }
}

/*
 * OnMeasureItem()
 */
void mw_OnMeasureItem(HWND mainWnd, MEASUREITEMSTRUCT *measureItem)
{
    if (measureItem->CtlType == ODT_LISTBOX) {
        TxLbx_OnMeasureItem(mainWnd, measureItem);
    } else if (measureItem->CtlType == ODT_MENU) {
        measureItem->itemWidth = mw_optionMenuHeaderWidth;
    }
}

/*
 * OnMouseMove()
 */
void mw_OnMouseMove(HWND mainWnd, int x, int y, UINT keyFlags)
{
    /*
     * Need to use mouse capture so that mouse up can be detected when the
     * user is dragging a splitter.
     */

    if (mw_isDraggingHSplit) {
        /*
         * Find the new proposed position of the splitter.
         */
        int newSplitTop = y - mw_hSplitDragY;

        /*
         * Make sure it is within the confines of the controls.
         */
        if (newSplitTop < mw_hSplitMin)
            newSplitTop = mw_hSplitMin;
        if (newSplitTop > mw_hSplitMax)
            newSplitTop = mw_hSplitMax;
        if (mw_hSplitTop == newSplitTop)
            return;
        /*
         * Move the splitter.
         */
        mw_splitterDy = newSplitTop - mw_hSplitTop;
        mw_OnSize(mainWnd, IsZoomed(mainWnd) ? SIZE_MAXIMIZED : SIZE_RESTORED
                , mw_clientRect.right, mw_clientRect.bottom);
        /*
         * Set the splitter cursor.
         */
        if (mw_hSplitTop == mw_hSplitMin) {
            mw_cursor = Prog_splitDownCursor;
        } else if (mw_hSplitTop == mw_hSplitMax) {
            mw_cursor = Prog_splitUpCursor;
        } else {
            mw_cursor = Prog_splitUpDownCursor;
        }
        SetCursor(mw_cursor);
    } else if (mw_isDraggingVSplit) {
        /*
         * Find the new proposed position of the splitter.
         */
        int newSplitLeft = x - mw_vSplitDragX;

        /*
         * Make sure it is within the confines of the controls.
         */
        if (newSplitLeft < mw_vSplitMin)
            newSplitLeft = mw_vSplitMin;
        if (newSplitLeft > mw_vSplitMax)
            newSplitLeft = mw_vSplitMax;
        if (mw_vSplitLeft == newSplitLeft)
            return;
        /*
         * Move the splitter.
         */
        mw_splitterDx = newSplitLeft - mw_vSplitLeft;
        mw_OnSize(mainWnd, IsZoomed(mainWnd) ? SIZE_MAXIMIZED : SIZE_RESTORED
                , mw_clientRect.right, mw_clientRect.bottom);
        /*
         * Set the splitter cursor.
         */
        if (mw_vSplitLeft == mw_vSplitMin) {
            mw_cursor = Prog_splitRightCursor;
        } else if (mw_vSplitLeft == mw_vSplitMax) {
            mw_cursor = Prog_splitLeftCursor;
        } else {
            mw_cursor = Prog_splitLeftRightCursor;
        }
        SetCursor(mw_cursor);
    } else {
        /*
         * If the mouse is captured and it moves outside the splitter area,
         * release the capture and change the cursor back to an arrow.
         */
        if (mw_isCaptureSet) {
            POINT mousePos = { x, y };
            HWND hoverWnd = ChildWindowFromPoint(mainWnd, mousePos);

            /*
             * Check for hovering over MainWnd child windows that aren't a
             * part of the splitter.
             */
            if (hoverWnd != mainWnd && hoverWnd != mw_libLbl[0]
                    && hoverWnd != mw_libLbl[1]) {
                goto HoverWindowChange;
            } else {
                /*
                 * Check for hovering over a different window, like an editor
                 * window or a window from another application.
                 */
                ClientToScreen(mainWnd, &mousePos);
                hoverWnd = WindowFromPoint(mousePos);
                if (hoverWnd != mainWnd) {
HoverWindowChange:
                    mw_cursor = Prog_arrowCursor;
                    SetCursor(mw_cursor);
                    ReleaseCapture();
                    mw_isCaptureSet = FALSE;
                    return;
                }
            }
        }
        /*
         * If the cursor is inside the horizontal splitter area.  The x
         * coordinate needs to be checked because the mouse might be captured.
         */
        if (x >= 0 && x <= mw_clientRect.right && y >= mw_hSplitTop
                && y <= mw_hSplitBottom)
        {
            /*
             * If so, set the horizontal splitter cursor.
             */
            if (mw_hSplitTop == mw_hSplitMin && mw_hSplitTop == mw_hSplitMax) {
                mw_cursor = Prog_arrowCursor;
            } else if (mw_hSplitTop == mw_hSplitMin) {
                mw_cursor = Prog_splitDownCursor;
            } else if (mw_hSplitTop == mw_hSplitMax) {
                mw_cursor = Prog_splitUpCursor;
            } else {
                mw_cursor = Prog_splitUpDownCursor;
            }
            SetCursor(mw_cursor);
            if (!mw_isCaptureSet) {
                SetCapture(mainWnd);
                mw_isCaptureSet = TRUE;
            }
        /*
         * If it's inside the vertical splitter area.
         */
        } else if (x >= mw_vSplitLeft && x <= mw_vSplitRight
                && ((y > mw_hSplitBottom && y < mw_buttonGroupTop)
                    || (y > mw_buttonGroupBottom && y < mw_clientRect.bottom)))
        {
            /*
             * If so, set the vertical splitter cursor.
             */
            if (mw_vSplitLeft == mw_vSplitMin && mw_vSplitLeft == mw_vSplitMax) {
                mw_cursor = Prog_arrowCursor;
                if (mw_isCaptureSet) {
                    ReleaseCapture();
                    mw_isCaptureSet = FALSE;
                }
            } else {
                if (mw_vSplitLeft == mw_vSplitMin) {
                    mw_cursor = Prog_splitRightCursor;
                } else if (mw_vSplitLeft == mw_vSplitMax) {
                    mw_cursor = Prog_splitLeftCursor;
                } else {
                    mw_cursor = Prog_splitLeftRightCursor;
                }
                if (!mw_isCaptureSet) {
                    SetCapture(mainWnd);
                    mw_isCaptureSet = TRUE;
                }
            }
            SetCursor(mw_cursor);
        } else {
            /*
             * Set the arrow cursor.
             */
            mw_cursor = Prog_arrowCursor;
            if (mw_isCaptureSet) {
                SetCursor(mw_cursor);
                ReleaseCapture();
                mw_isCaptureSet = FALSE;
            }
        }
    }
}

/*
 * OnSetCursor()
 */
BOOL mw_OnSetCursor(HWND mainWnd, HWND cursorWnd, UINT codeHitTest, UINT msg)
{
    /*
     * If the cursor is in the window's client area.
     */
    if (codeHitTest == HTCLIENT) {
        /*
         * Set the cursor to the one set in OnMouseMove().
         */
        SetCursor(mw_cursor);
    } else {
        /*
         * Otherwise, let windows set it as per usual.
         */
        FORWARD_WM_SETCURSOR(mainWnd, cursorWnd, codeHitTest, msg
                , DefWindowProc);
    }
    return TRUE;
}

/*
 * OnSize() - Repositions controls and splitters and redraws graphical edges.
 */
void mw_OnSize(HWND mainWnd, UINT state, int cx, int cy)
{
    static int splitMoveKludge = 1;
    int buttonGroupHeight;
    int buttonGroupCenter;
    int buttonGroupTop;
    int listBoxTop;
    int tempTop;
    int x;
    int deltaX;
    int selectedDeltaX;
    int countCtrlWidth;
    int countCtrlSpaceWidth;
    int countDeltaX;
    int i;
    HDWP defer;
    HDC dC;

#ifdef PROG_LOGFILE
    static const char *states[5] = {
        "SIZE_RESTORED",
        "SIZE_MINIMIZED",
        "SIZE_MAXIMIZED",
        "SIZE_MAXSHOW",
        "SIZE_MAXHIDE"
    };
    if (state >= 0 && state <= 5) {
        fprintf(Prog_logFile, "WM_SIZE, state = %s\n", states[state]);
    } else {
        fprintf(Prog_logFile, "WM_SIZE, state = %d\n", state);
    }
#endif
    /*
     * No need to move controls if the window is minimized.
     */
    if (state == SIZE_MINIMIZED)
        return;
    /*
     * Update the splitter positions.
     */
    if (state == SIZE_RESTORED) {
        if (mw_lastWndState == SIZE_MAXIMIZED) {
            mw_splitterDy = Prog_mainWndHSplitterPos
                - Prog_mainWndZoomedHSplitterPos;
        } else {
            Prog_mainWndHSplitterPos += mw_splitterDy;
        }
    } else if (state == SIZE_MAXIMIZED) {
        if (mw_lastWndState == SIZE_RESTORED) {
            mw_splitterDy = Prog_mainWndZoomedHSplitterPos
                - Prog_mainWndHSplitterPos;
        } else {
            Prog_mainWndZoomedHSplitterPos += mw_splitterDy;
        }
    }
    mw_hSplitTop += mw_splitterDy;
    mw_lastWndState = state;
    mw_hSplitBottom = mw_hSplitTop + mw_hSplitterH;
    /*
     * Update control positions - calculate the vertical positions first.
     */
    mw_snapshotLbxArea.h = mw_hSplitTop - 1 - mw_snapshotLbxArea.y;
    for (i = 0; i < 2; i++) {
        mw_libLblArea[i].y         += mw_splitterDy;
        mw_libTbxArea[i].y         += mw_splitterDy;
        mw_libFileBtnArea[i].y     += mw_splitterDy;
        mw_libSortBtnArea[i].y     += mw_splitterDy;
        mw_libSelectedLblArea[i].y += mw_splitterDy;
        mw_libSelectedStcArea[i].y += mw_splitterDy;
        mw_libCountLblArea[i].y    += mw_splitterDy;
        mw_libCountStcArea[i].y    += mw_splitterDy;
    }
    /*
     * Calculate the lower list box positions.
     */
    listBoxTop = AREA_B(mw_libFileBtnArea[0]) + 1;
    mw_libLbxArea[0].y = mw_libLbxArea[1].y = listBoxTop;
    mw_libLbxArea[0].h = mw_libLbxArea[1].h = cy - listBoxTop;
    /*
     * Library operation buttons.
     */
    buttonGroupHeight = mw_copyBtnArea.h * 5 + 4;
    buttonGroupCenter = buttonGroupHeight >> 1;
    /* the buttons can't go past the top of the library list boxes */
    tempTop = (mw_hSplitBottom + ((cy - mw_hSplitBottom) >> 1))
        - buttonGroupCenter;
    buttonGroupTop = tempTop < listBoxTop ? listBoxTop : tempTop;
    /* set the button positions */
    mw_copyBtnArea.y = buttonGroupTop;
    mw_deleteBtnArea.y = AREA_B(mw_copyBtnArea) + 1;
    mw_renameBtnArea.y = AREA_B(mw_deleteBtnArea) + 1;
    mw_recommentBtnArea.y = AREA_B(mw_renameBtnArea) + 1;
    mw_diffBtnArea.y = AREA_B(mw_recommentBtnArea) + 1;
    /* set the mw_buttonGroup variables for the edge drawing */
    mw_buttonGroupTop = buttonGroupTop - 4;
    mw_buttonGroupBottom = AREA_B(mw_diffBtnArea) + 3;
    /* set the new height of the library list boxes */
    mw_libLbxArea[0].h = mw_libLbxArea[1].h = cy - mw_libLbxArea[0].y;
    /* set the lower horizontal splitter constraint */
    mw_hSplitMax = cy - mw_bottomCtrlH - 4;

    /*
     * Calculate the horizontal positions.
     */
    /*
     * Find the new x coordinate of the right snapshot function buttons.
     */
    x = cx - mw_rightBtnDelta;
    /*
     * Find the distance that the right buttons need to be moved.
     */
    deltaX = x - mw_snapshotRxItemsBtnArea.x;
    /*
     * Add that distance to the width of the snapshot text box and list box.
     */
    mw_snapshotTbxArea.w += deltaX;
    mw_snapshotLbxArea.w += deltaX;
    /*
     * Add the delta x to the positions of the snapshot selected count label
     * and static control.
     */
    mw_snapshotSelectedLblArea.x += deltaX;
    mw_snapshotSelectedStcArea.x += deltaX;
    /*
     * Place the right snapshot function buttons.
     */
    mw_snapshotRxItemsBtnArea.x = mw_snapshotTxItemsBtnArea.x
        = mw_storeBtnArea.x = mw_snapshotRxBtnArea.x = mw_snapshotTxBtnArea.x
        = x;
    /*
     * Now for the bottom controls, first calculate the new position of the
     * vertical splitter.  Moving the splitter causes it to change by
     * mw_splitterDx.  Resizing the window causes the splitter to move in
     * proportion with the last splitter position in relation to the previous
     * window size.
     */
    if (mw_splitterDx) {
        mw_vSplitLeft += mw_splitterDx;
        mw_vSplitRight += mw_splitterDx;
    }
    if (cx != mw_vSplitClientW) {
        deltaX = (mw_vSplitLeft * cx / mw_vSplitClientW) - mw_vSplitLeft
            + splitMoveKludge;
        splitMoveKludge ^= 1;
        mw_vSplitLeft += deltaX;
        if (mw_vSplitLeft < mw_vSplitMin) {
            mw_vSplitLeft = mw_vSplitMin;
        }
        mw_vSplitMax = cx - mw_vSplitMaxFromClientRight;
        if (mw_vSplitLeft > mw_vSplitMax) {
            mw_vSplitLeft = mw_vSplitMax;
        }
        mw_vSplitRight = mw_vSplitLeft + mw_copyBtnArea.w + 2;
        mw_vSplitClientW = cx;
    }
    /*
     * Apply the button delta x to the buttons.
     */
    mw_copyBtnArea.x = mw_deleteBtnArea.x = mw_renameBtnArea.x
        = mw_recommentBtnArea.x = mw_diffBtnArea.x = mw_vSplitLeft;
    /*
     * The new right edge of the buttons will determine where the left edge of
     * the library 2 controls is, so the delta x can be applied to its controls
     * that are left-aligned.
     */
    mw_libLblArea[1].x = mw_libTbxArea[1].x = mw_libLbxArea[1].x
        = mw_libFileBtnArea[1].x = mw_vSplitRight;
    mw_libSortBtnArea[1].x = AREA_R(mw_libFileBtnArea[1]) + 2;
    /*
     * The code to calculate the widths and right alignments for the library 2
     * controls can be used for the library 1 controls too.  The only
     * difference is the right edge coordinate, which will be changed at the
     * end of the loop.  The comments in this loop talk about the right side,
     * though.
     */
    for (i = 1; i >= 0; i--) {
        /*
         * The width of the library 2 text box and list box is governed by the
         * right edge of the window's client area.
         */
        mw_libTbxArea[i].w = cx - mw_libTbxArea[i].x;
        mw_libLbxArea[i].w = cx - mw_libLbxArea[i].x;
        /*
         * The selected label and static control, as a group, is right-aligned
         * against the window edge.  This needs a different delta x variable
         * since the right edge moves faster than the buttons.
         */
        selectedDeltaX = cx - AREA_R(mw_libSelectedStcArea[i]);
        mw_libSelectedLblArea[i].x += selectedDeltaX;
        mw_libSelectedStcArea[i].x += selectedDeltaX;
        /*
         * The library 2 item count label and static, as a group, sit centered
         * in between the sort button and the selected label.
         */
        countCtrlWidth = AREA_R(mw_libCountStcArea[i])
            - mw_libCountLblArea[i].x;
        countCtrlSpaceWidth = mw_libSelectedLblArea[i].x
            - AREA_R(mw_libSortBtnArea[i]);
        x = ((countCtrlSpaceWidth - countCtrlWidth) >> 1)
            + AREA_R(mw_libSortBtnArea[i]);
        countDeltaX = x - mw_libCountLblArea[i].x;
        mw_libCountLblArea[i].x += countDeltaX;
        mw_libCountStcArea[i].x += countDeltaX;
        /*
         * Change the right edge for the library 1 controls.
         */
        cx = mw_copyBtnArea.x - 2;
    }
    /*
     * Set up the deferred window move.
     */
    defer = BeginDeferWindowPos(32);
    DeferWindowPos(defer, mw_snapshotTbx, NULL
            , 0, 0, mw_snapshotTbxArea.w, mw_snapshotTbxArea.h
            , SWP_NOZORDER | SWP_NOMOVE);
    DeferWindowPos(defer, mw_snapshotSelectedLbl, NULL
            , mw_snapshotSelectedLblArea.x, mw_snapshotSelectedLblArea.y
            , 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_snapshotSelectedStc, NULL
            , mw_snapshotSelectedStcArea.x, mw_snapshotSelectedStcArea.y
            , 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_snapshotLbx, NULL
            , 0, 0, mw_snapshotLbxArea.w, mw_snapshotLbxArea.h
            , SWP_NOZORDER | SWP_NOMOVE);

    DeferWindowPos(defer, mw_snapshotRxItemsBtn, NULL
            , mw_snapshotRxItemsBtnArea.x, mw_snapshotRxItemsBtnArea.y
            , 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_snapshotTxItemsBtn, NULL
            , mw_snapshotTxItemsBtnArea.x, mw_snapshotTxItemsBtnArea.y
            , 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_storeBtn, NULL
            , mw_storeBtnArea.x, mw_storeBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_snapshotRxBtn, NULL
            , mw_snapshotRxBtnArea.x, mw_snapshotRxBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_snapshotTxBtn, NULL
            , mw_snapshotTxBtnArea.x, mw_snapshotTxBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);

    DeferWindowPos(defer, mw_copyBtn, NULL
            , mw_copyBtnArea.x, mw_copyBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_deleteBtn, NULL
            , mw_deleteBtnArea.x, mw_deleteBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_renameBtn, NULL
            , mw_renameBtnArea.x, mw_renameBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_recommentBtn, NULL
            , mw_recommentBtnArea.x, mw_recommentBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);
    DeferWindowPos(defer, mw_diffBtn, NULL
            , mw_diffBtnArea.x, mw_diffBtnArea.y, 0, 0
            , SWP_NOZORDER | SWP_NOSIZE);

    for (i = 0; i < 2; i++) {
        DeferWindowPos(defer, mw_libLbl[i], NULL
                , mw_libLblArea[i].x, mw_libLblArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libTbx[i], NULL
                , PASS_AREA_FIELDS(mw_libTbxArea[i]), SWP_NOZORDER);
        DeferWindowPos(defer, mw_libFileBtn[i], NULL
                , mw_libFileBtnArea[i].x, mw_libFileBtnArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libSortBtn[i], NULL
                , mw_libSortBtnArea[i].x, mw_libSortBtnArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libCountLbl[i], NULL
                , mw_libCountLblArea[i].x, mw_libCountLblArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libCountStc[i], NULL
                , mw_libCountStcArea[i].x, mw_libCountStcArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libSelectedLbl[i], NULL
                , mw_libSelectedLblArea[i].x, mw_libSelectedLblArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libSelectedStc[i], NULL
                , mw_libSelectedStcArea[i].x, mw_libSelectedStcArea[i].y, 0, 0
                , SWP_NOZORDER | SWP_NOSIZE);
        DeferWindowPos(defer, mw_libLbx[i], NULL
                , PASS_AREA_FIELDS(mw_libLbxArea[i]), SWP_NOZORDER);
    }
    /*
     * Set the minimum client height.
     */
    mw_minClient.cy = mw_hSplitTop + mw_bottomCtrlH + 4;
    /*
     * Erase the graphical edges.
     */
    dC = GetDC(mainWnd);
    mw_EraseEdges(dC);
    /*
     * Do the deferred window move.
     */
    EndDeferWindowPos(defer);
    /*
     * Redraw the edges.
     */
    mw_DrawEdges(dC);
    ReleaseDC(mainWnd, dC);
    /*
     * Reset the splitter delta variables.
     */
    mw_splitterDx = mw_splitterDy = 0;
}

/*
 * OnSizing()
 */
void mw_OnSizing(HWND mainWnd, int edge, RECT *rect)
{
    int rectW = RECT_W(*rect);
    int rectH = RECT_H(*rect);

    /*
     * Need to update these because mw_minClient changes with the splitters.
     */
    mw_minWnd.cx = mw_minClient.cx + mw_wndClientDiff.cx;
    mw_minWnd.cy = mw_minClient.cy + mw_wndClientDiff.cy;
    /*
     * Constrain the window width.
     */
    if (rectW < mw_minWnd.cx) {
        /*
         * If the splitter has some leeway, squash the splitter instead of
         * constraining the window.
         */
        if (mw_vSplitLeft > mw_vSplitMin) {
            /*
             * Figure out how far the user wants to move the window edge.
             */
            mw_splitterDx = rectW - mw_minWnd.cx;
            /*
             * If it's too far for the splitter.
             */
            if (mw_vSplitLeft + mw_splitterDx < mw_vSplitMin) {
                /*
                 * Move the window as far as the splitter can move.
                 */
                mw_splitterDx = mw_vSplitMin - mw_vSplitLeft;
            }
        }
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        if (edge == WMSZ_RIGHT || edge == WMSZ_TOPRIGHT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->right = rect->left + mw_minWnd.cx;
        }
        else if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT
                || edge == WMSZ_BOTTOMLEFT)
        {
            rect->left = rect->right - mw_minWnd.cx;
        }
    }
    /*
     * Constrain the window height.
     */
    if (rectH < mw_minWnd.cy) {
        /*
         * If the splitter has some leeway, squash the splitter instead of
         * constraining the window.
         */
        if (mw_hSplitTop > mw_hSplitMin) {
            mw_splitterDy = rectH - mw_minWnd.cy;
            if (mw_hSplitTop + mw_splitterDy < mw_hSplitMin) {
                mw_splitterDy = mw_hSplitMin - mw_hSplitTop;
            }
        }
        /*
         * It's necessary to explicitly state which edge should be constrained,
         * otherwise the window could move when an edge is being dragged.
         */
        else if (edge == WMSZ_BOTTOM || edge == WMSZ_BOTTOMLEFT
                || edge == WMSZ_BOTTOMRIGHT)
        {
            rect->bottom = rect->top + mw_minWnd.cy;
        }
        else if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT
                || edge == WMSZ_TOPRIGHT)
        {
            rect->top = rect->bottom - mw_minWnd.cy;
        }
    }
}

/*
 * OnSysColorChange() - Updates the GUI when a system color change occurs.
 */
void mw_OnSysColorChange(HWND mainWnd)
{
    Prog_UpdateSysColors();
    mw_InitMenuButtons();
}

/*
 * OpenLibraryFile()
 */
BOOL mw_OpenLibraryFile(HWND mainWnd, int libIndex, const _TUCHAR *fileName)
{
    TXLIB tempLib;

    /*
     * Ask user if he wants to save if the current file is dirty, and bail out if he
     * clicks Cancel.
     */
    if (mw_DirtyPrompt(mainWnd, libIndex)) {
        return FALSE;
    }
    /*
     * Create a temporary library to load the file into.
     */
    TxLib_Init(&tempLib);
    /*
     * If the file is to be opened automatically, a name will be passed to the function.
     * If the name is missing then prompt the user for a file name.
     */
    if (fileName == NULL || fileName[0] == '\0') {
        /*
         * Use the current file name as the default for the open dialog.  If there is no
         * current file name, then it's no big deal.
         */
        _tcscpy(tempLib.fileName, Prog_lib[libIndex].fileName);
        /*
         * Show the open file dialog.
         */
        if (!FileDlg_Open(mainWnd, Prog_libRecentDir, Prog_libFilter
                    , Prog_libExt, tempLib.fileName, _MAX_PATH
                    , OFN_HIDEREADONLY))
        {
            /*
             * Abandon ship if the user cancelled.
             */
            return FALSE;
        }
    } else {
        /*
         * Copy the passed file name into the temporary structure to tell TxLib_Load()
         * what file to load.
         */
        _tcscpy(tempLib.fileName, fileName);
    }
    /*
     * Load the library file and if it fails, destroy the temp and return.
     */
    if (!TxLib_Load(mainWnd, &tempLib)) {
        TxLib_DeInit(&tempLib);
        return FALSE;
    }
    /*
     * The load was successful, so destroy the old library and replace it with the temp.
     */
    TxLib_ReplaceLib(&Prog_lib[libIndex], &tempLib);
    /*
     * Add the file name to the library's recent file list.
     */
    mw_AddRecentFile(Prog_libRecent[libIndex], Prog_lib[libIndex].fileName);
    /*
     * Reset the scroll position so the update function doesn't restore it.
     */
    ListBox_SetTopIndex(mw_libLbx[libIndex], 0);
    /*
     * Update the library list box.
     */
    mw_UpdateLibLbx(libIndex, UL_ALL);

    return TRUE;
}

/*
 * OpenSnapshotFile()
 */
BOOL mw_OpenSnapshotFile(HWND mainWnd, const _TUCHAR *fileName, BOOL isCopy)
{
    SNAPSHOT tempSnapshot;

    /*
     * Ask user if he wants to save if the current file is dirty, and bail out
     * if he clicks Cancel.
     */
    if (mw_DirtyPrompt(mainWnd, WF_SNAPSHOT)) {
        return FALSE;
    }
    /*
     * Create a temporary snapshot to load the new file into.  This way if the
     * load fails, the old file will remain intact.
     */
    if (isCopy) {
        memcpy(&tempSnapshot, &Prog_snapshot, sizeof Prog_snapshot);
    } else {
        Snapshot_Init(&tempSnapshot);
    }
    /*
     * If the file is to be opened automatically, a name will be passed to the
     * function.  If the name is missing then prompt the user for a file name.
     */
    if (fileName == NULL || fileName[0] == '\0') {
        /*
         * Use the current file name as the default for the open dialog.  If
         * there is no current file name, then it's no big deal.
         */
        _tcscpy(tempSnapshot.extra.fileName, Prog_snapshot.extra.fileName);
        /*
         * Show the open file dialog.
         */
        if (!FileDlg_Open(mainWnd, Prog_snapshotRecentDir, Prog_syxFilter
                    , Prog_syxExt, tempSnapshot.extra.fileName, _MAX_PATH
                    , OFN_HIDEREADONLY))
        {
            /*
             * Abandon ship if the user cancelled.
             */
            return FALSE;
        }
    } else {
        /*
         * Copy the passed file name into the temporary structure to tell
         * Snapshot_Load() what file to load.
         */
        _tcscpy(tempSnapshot.extra.fileName, fileName);
    }
    /*
     * Load the snapshot file and if it fails just return.
     */
    tempSnapshot.extra.loaded = 0;
    if (!Snapshot_Load(mainWnd, &tempSnapshot)) {
        return FALSE;
    }
    /*
     * The load was successful, so...
     */
    if (isCopy) {
        int i;

        /*
         * Copy the snapshot, but preserve the file name.
         */
        memcpy(&Prog_snapshot, &tempSnapshot
                , sizeof(SNAPSHOT) - sizeof(EXTRA));
        Prog_snapshot.extra.loaded |= tempSnapshot.extra.loaded;
        for (i = 0; i < 7; i++) {
            Prog_snapshot.extra.itemDirty[i] |= tempSnapshot.extra.itemDirty[i];
        }
    } else {
        /*
         * Mark the snapshot as clean.
         */
        Snapshot_Clean(&tempSnapshot);
        /*
         * Copy the temp snapshot into the real snapshot.
         */
        memcpy(&Prog_snapshot, &tempSnapshot, sizeof tempSnapshot);
        /*
         * Add the new file name to the recent files list.
         */
        mw_AddRecentFile(Prog_snapshotRecent, Prog_snapshot.extra.fileName);
    }
    /*
     * Clear the snapshot list box selection array so the update function
     * doesn't re-select them.
     */
    mw_snapshotSelItemCnt = 0;
    /*
     * Reset the scroll position so the update function doesn't restore it.
     */
    ListBox_SetTopIndex(mw_snapshotLbx, 0);
    /*
     * Update the snapshot list box.
     */
    mw_UpdateSnapshotLbx(UL_ALL);

    return TRUE;
}

/*
 * RetrieveAndUpdate() - Retrieves data from the TX81Z and updates the
 *                       snapshot listbox and open editor windows.
 */
BOOL mw_RetrieveAndUpdate(REQUEST reqFlags)
{
    if (TX81Z_RetrieveData(Prog_midi, reqFlags, &Prog_snapshot)) {
        /*
         * Update snapshot listbox.
         */
        mw_UpdateSnapshotLbx(UL_ALL);
        /*
         * Update open editor dialogs.
         */
        mw_UpdateEditors(reqFlags);
        return TRUE;
    }
    return FALSE;
}

/*
 * SaveFile() - Saves a snapshot or library file.  Shows the file save dialog
 *              if showDlg is true and also allows the user to cancel saving.
 */
void mw_SaveFile(HWND mainWnd, WHICHFILE whichFile, BOOL showDlg)
{
    BOOL doSave = TRUE;
    BOOL hasNoFileName;
    _TUCHAR *initialDir;
    _TUCHAR *fileName;
    const _TUCHAR *filterStr;
    const _TUCHAR *extStr;

    if (whichFile == WF_SNAPSHOT) {
        initialDir = Prog_snapshotRecentDir;
        fileName = Prog_snapshot.extra.fileName;
        filterStr = Prog_syxFilter;
        extStr = Prog_syxExt;
    } else {
        initialDir = Prog_libRecentDir;
        fileName = Prog_lib[whichFile].fileName;
        filterStr = Prog_libFilter;
        extStr = Prog_libExt;
    }
    hasNoFileName = fileName[0] == '\0';

    if (showDlg || hasNoFileName) {
        doSave = FileDlg_Save(mainWnd, initialDir, filterStr, extStr, fileName
                , _MAX_PATH, OFN_OVERWRITEPROMPT);
    }
    if (doSave) {
        if (whichFile == WF_SNAPSHOT) {
            Snapshot_UpdateSysexWrappers(&Prog_snapshot);
            if (Snapshot_Save(mainWnd, &Prog_snapshot)) {
                /*
                 * Clear the dirty flags from the snapshot.
                 */
                Snapshot_Clean(&Prog_snapshot);
                /*
                 * Clear the dirty colors from the list box and update it.
                 */
                mw_CleanLbx(mw_snapshotLbx);
                mw_UpdateSnapshotLbx(UL_ALL);
                /*
                 * Add the new file name to the recent files list.
                 */
                mw_AddRecentFile(Prog_snapshotRecent
                        , Prog_snapshot.extra.fileName);
                /*
                 * Notify open editors of the save.
                 */
                if (Prog_voiceDlg) {
                    SendMessage(Prog_voiceDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_pfmDlg) {
                    SendMessage(Prog_pfmDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_fxDlg) {
                    SendMessage(Prog_fxDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_pcDlg) {
                    SendMessage(Prog_pcDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_mtoDlg) {
                    SendMessage(Prog_mtoDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_mtfDlg) {
                    SendMessage(Prog_mtfDlg, EDM_SAVED, 0, 0);
                }
                if (Prog_sysDlg) {
                    SendMessage(Prog_sysDlg, EDM_SAVED, 0, 0);
                }
            }
        } else {
            if (TxLib_Save(mainWnd, &Prog_lib[whichFile])) {
                /*
                 * Clear dirty flags from the library.
                 */
                TxLib_Clean(&Prog_lib[whichFile]);
                /*
                 * Clear dirty colors from the list box and update it.
                 */
                mw_CleanLbx(mw_libLbx[whichFile]);
                mw_UpdateLibLbx(whichFile, UL_ALL);
                /*
                 * Add the file name to the library's recent file list.
                 */
                mw_AddRecentFile(Prog_libRecent[whichFile]
                        , Prog_lib[whichFile].fileName);
            }
        }
    }
}

/*
 * SaveSnapshotCopy() - Saves a snapshot under a new name without changing the
 *                      current file name.
 */
void mw_SaveSnapshotCopy(HWND mainWnd)
{
    _TUCHAR oldFileName[_MAX_PATH];
    _TUCHAR *fileName = Prog_snapshot.extra.fileName;

    _tcscpy(oldFileName, Prog_snapshot.extra.fileName);
    if (FileDlg_Save(mainWnd, Prog_snapshotRecentDir, Prog_syxFilter
                , Prog_syxExt, fileName, _MAX_PATH, OFN_OVERWRITEPROMPT))
    {
        Snapshot_UpdateSysexWrappers(&Prog_snapshot);
        Snapshot_Save(mainWnd, &Prog_snapshot);
    }
    _tcscpy(Prog_snapshot.extra.fileName, oldFileName);
}

/*
 * SendLibItem() - Copies a library item to the edit buffer.
 */
void mw_SendLibItem(HWND mainWnd, int libIndex)
{
    SNAPSHOT *snapshot = &Prog_snapshot;
    TXLIB *txLib = &Prog_lib[libIndex];
    HWND listBox = mw_libLbx[libIndex];
    int itemIndex = ListBox_GetCurSel(listBox);
    BYTE *libItemData;
    METAINDEX libItemType;

    TxLib_SetPtr(txLib, itemIndex);
    libItemData = TxLib_GetItemData(txLib);
    libItemType = TxLib_GetItemType(txLib);
    if (libItemType == META_VMEM) {
        /*
         * Copy the patch to the edit buffer.
         */
        TXPack_VmemToAvced(libItemData, &snapshot->avced, TRUE);
        Snapshot_SetItemGroupLoaded(snapshot, SI_VCED);
        Snapshot_DirtyItem(snapshot, SI_VCED);
        mw_UpdateSnapshotLbxItem(TRUE, SI_VCED, SELECT_UPDATED);
        /*
         * Send it to the unit.
         */
        TX81Z_SendData(Prog_midi, REQ_AVCED, snapshot);
        if (!Prog_dblClkingBankDoesntOpen) {
            VoiceDlg_Create(mainWnd, &snapshot->avced);
        }
        /*
         * Switch the unit to voice mode.
         */
        TX81Z_SwitchToVoiceMode(Prog_midi);
    } else if (libItemType == META_PMEM) {
        TXPack_PmemToPced(libItemData, &snapshot->pced);
        Snapshot_DirtyItem(snapshot, SI_PCED);
        Snapshot_SetItemGroupLoaded(snapshot, SI_PCED);
        mw_UpdateSnapshotLbxItem(TRUE, SI_PCED, SELECT_UPDATED);
        TX81Z_SendData(Prog_midi, REQ_PCED, snapshot);
        if (!Prog_dblClkingBankDoesntOpen) {
            PfmDlg_Create(mainWnd, &snapshot->pced);
        }
    } else if (libItemType == META_BNDL) {
        if (BndlDlg_Create(mainWnd, libItemData)) {
            mw_UpdateSnapshotLbx(UL_LBX);
        }
    } else if (libItemType == META_FX) {
        memcpy(snapshot->fx.data, libItemData, sizeof snapshot->fx.data);
        TX81Z_SendData(Prog_midi, REQ_FX, snapshot);
        Snapshot_SetItemGroupLoaded(snapshot, SI_FX);
        Snapshot_DirtyItem(snapshot, SI_FX);
        mw_UpdateSnapshotLbxItem(TRUE, SI_FX, SELECT_UPDATED);
        if (!Prog_dblClkingBankDoesntOpen) {
            FxDlg_Create(mainWnd, &snapshot->fx);
        }
    } else if (libItemType == META_PC) {
        memcpy(snapshot->pc.data, libItemData, sizeof snapshot->pc.data);
        TX81Z_SendData(Prog_midi, REQ_PC, snapshot);
        Snapshot_SetItemGroupLoaded(snapshot, SI_PC);
        Snapshot_DirtyItem(snapshot, SI_PC);
        mw_UpdateSnapshotLbxItem(TRUE, SI_PC, SELECT_UPDATED);
        if (!Prog_dblClkingBankDoesntOpen) {
            PcDlg_Create(mainWnd, &snapshot->pc);
        }
    } else if (libItemType == META_MTO) {
        memcpy(snapshot->mto.data, libItemData, sizeof snapshot->mto.data);
        TX81Z_SendData(Prog_midi, REQ_MTO, snapshot);
        Snapshot_SetItemGroupLoaded(snapshot, SI_MTO);
        Snapshot_DirtyItem(snapshot, SI_MTO);
        mw_UpdateSnapshotLbxItem(TRUE, SI_MTO, SELECT_UPDATED);
        if (!Prog_dblClkingBankDoesntOpen) {
            MtoDlg_Create(mainWnd, &snapshot->mto);
        }
    } else if (libItemType == META_MTF) {
        memcpy(snapshot->mtf.data, libItemData, sizeof snapshot->mtf.data);
        TX81Z_SendData(Prog_midi, REQ_MTF, snapshot);
        Snapshot_SetItemGroupLoaded(snapshot, SI_MTF);
        Snapshot_DirtyItem(snapshot, SI_MTF);
        mw_UpdateSnapshotLbxItem(TRUE, SI_MTF, SELECT_UPDATED);
        if (!Prog_dblClkingBankDoesntOpen) {
            MtfDlg_Create(mainWnd, &snapshot->mtf);
        }
    } else if (libItemType == META_SYS) {
        memcpy(snapshot->sys.data, libItemData, sizeof snapshot->sys.data);
        TX81Z_SendData(Prog_midi, REQ_SYS, snapshot);
        Snapshot_SetItemGroupLoaded(snapshot, SI_SYS);
        Snapshot_DirtyItem(snapshot, SI_SYS);
        mw_UpdateSnapshotLbxItem(TRUE, SI_SYS, SELECT_UPDATED);
        if (!Prog_dblClkingBankDoesntOpen) {
            SysDlg_Create(mainWnd, &snapshot->sys);
        }
    }
}

/*
 * SnapshotRxData() - Receive data into snapshot.
 */
void mw_SnapshotRxData(HWND mainWnd)
{
    if (DataDlg_Create(mainWnd, DDM_RECEIVE) && Prog_rxOptions) {
        mw_RetrieveAndUpdate(Prog_rxOptions);
    }
}

/*
 * SnapshotRxItems() - Receive selected items into snapshot.
 */
void mw_SnapshotRxItems(HWND mainWnd)
{
    int i;
    REQUEST reqFlags = 0;

    /*
     * Go through and set the reqFlags bits according to what is selected.
     */
    for (i = 0; i < mw_snapshotSelItemCnt; i++) {
        int m;

        for (m = META_VCED; m <= META_PRESET_D; m++) {
            int n = mw_snapshotSelItems[i];

            if (n >= TX81Z_meta[m].sIndex && n < TX81Z_meta[m + 1].sIndex) {
                reqFlags |= TX81Z_meta[m].reqFlag;
                if (m == META_VCED) {
                    reqFlags |= REQ_ACED;
                }
                break;
            }
        }
    }
    /*
     * Get confirmation from the user if a bank is about to be retrieved.
     */
    if ((((reqFlags & REQ_VMEM)
                    && Snapshot_IsItemGroupLoaded(&Prog_snapshot, META_VMEM))
                || ((reqFlags & REQ_PMEM)
                    && Snapshot_IsItemGroupLoaded(&Prog_snapshot, META_PMEM)))
            && !mw_ConfirmBankTransfer(mainWnd, reqFlags, TRUE))
    {
        return;
    }
    mw_RetrieveAndUpdate(reqFlags);
}

/*
 * SnapshotTxData()
 */
void mw_SnapshotTxData(HWND mainWnd)
{
    if (DataDlg_Create(mainWnd, DDM_TRANSMIT) && Prog_txOptions) {
        TX81Z_SendData(Prog_midi, Prog_txOptions, &Prog_snapshot);
    }
}

/*
 * SnapshotTxItems()
 */
void mw_SnapshotTxItems(HWND mainWnd)
{
    int i;
    REQUEST reqFlags = 0;

    /*
     * Go through and set the reqFlags bits according to what is selected,
     * excluding the preset banks.
     */
    for (i = 0; i < mw_snapshotSelItemCnt; i++) {
        int m;

        for (m = META_VCED; m < META_PRESET_A; m++) {
            int n = mw_snapshotSelItems[i];

            if (n >= TX81Z_meta[m].sIndex && n < TX81Z_meta[m + 1].sIndex) {
                reqFlags |= TX81Z_meta[m].reqFlag;
                break;
            }
        }
    }
    /*
     * Get confirmation from the user if a bank is about to be retrieved.
     */
    if (((reqFlags & REQ_VMEM) || (reqFlags & REQ_PMEM))
            && !mw_ConfirmBankTransfer(mainWnd, reqFlags, FALSE))
    {
        return;
    }
    TX81Z_SendData(Prog_midi, reqFlags, &Prog_snapshot);
}

/*
 * SortLib() - Sorts a library.
 */
void mw_SortLib(HWND mainWnd, int libIndex, TXLIB_SORT sort)
{
    TXLIB *txLib = &Prog_lib[libIndex];
    int i;
    HWND waitDlg;
    HCURSOR oldCursor;

    waitDlg = CreateDialog(Prog_instance, MAKEINTRESOURCE(IDD_WAITDLG), mainWnd
            , Dialog_GenericProc);
    Window_Center(waitDlg, mainWnd);
    ShowWindow(waitDlg, SW_SHOW);
    SetCapture(waitDlg);
    oldCursor = SetCursor(Prog_waitCursor);
    Dialog_DoEvents(waitDlg);
    /*
     * If there are selected items in this list box.
     */
    if (mw_libSelIndex == libIndex) {
        /*
         * The selection of items is going to get shuffled, so replace the item
         * indexes in the selItems array with the item insert orders, since they
         * are unique.
         */
        for (i = 0; i < mw_libSelItemCnt; i++) {
            TxLib_SetPtr(txLib, mw_libSelItems[i]);
            mw_libSelItems[i] = TxLib_GetItemOrder(txLib);
        }
    }
    /*
     * Send the lib selection array to the TxLib module.
     */
    TxLib_libSelItems = mw_libSelItems;
    TxLib_libSelItemCnt = &mw_libSelItemCnt;
    /*
     * Sort the library.
     */
    TxLib_Sort(txLib, sort);
    /*
     * Update the list box.
     */
    mw_UpdateLibLbx(libIndex, UL_LBX);
    /*
     * Restore the item selections, if applicable.
     */
    if (mw_libSelIndex == libIndex) {
        /*
         * Find the previously selected items in the library and save their new
         * indexes in the selection array.
         */
        for (i = 0; i < mw_libSelItemCnt; i++) {
            int itemIndex = 0;

            TxLib_Head(txLib);
            while (TxLib_Next(txLib)->order != mw_libSelItems[i]) {
                itemIndex++;
            }
            mw_libSelItems[i] = itemIndex;
        }
        /*
         * Restore the item selections.
         */
        SetWindowRedraw(mw_libLbx[libIndex], FALSE);
        for (i = 0; i < mw_libSelItemCnt; i++) {
            ListBox_SetSel(mw_libLbx[libIndex], TRUE, mw_libSelItems[i]);
        }
        SetWindowRedraw(mw_libLbx[libIndex], TRUE);
    }
    ReleaseCapture();
    DestroyWindow(waitDlg);
    SetCursor(oldCursor);
}

/*
 * UpdateButtonStates()
 */
void mw_UpdateButtonStates(void)
{
    METAINDEX type, lastType = -1;
    int i, j;
    BOOL t;
    BOOL onlyPfmsAreSelected;
    BOOL anyLibItemsAreSelected = (mw_libSelItemCnt > 0);
    BOOL anyLibNamelessItemsAreSelected = FALSE;
    BOOL anyNonDiffableItemsAreSelected = FALSE;

    /*
     * Enable/disable buttons as the selection state changes.
     *   - The Edit Item button is enabled if there is exactly one item
     *     selected and it is loaded.
     *   - The Send Program Change button is enabled if there is exactly one
     *     bank item selected.
     *   - The Add To Lib # buttons are enabled if anything is selected.
     *   - The Bundle To Lib # buttons are enabled if only performances are
     *     selected.
     *   - The Receive Items button is enabled if anything is selected.
     *   - The Transmit Items button is enabled if any loaded items are
     *     selected.
     *   - The Store Item button is enabled if there is only one item selected
     *     and it is loaded and it is an edit buffer.
     */
    if (mw_snapshotSelItemCnt == 1) {
        type = Snapshot_GetItemType(mw_snapshotSelItems[0]);
        t = (type >= META_VMEM && type <= META_PRESET_D);
        EnableWindow(mw_sendPCBtn, t);
        EnableWindow(mw_snapshotRxItemsBtn, TRUE);
        if (Snapshot_IsItemsGroupLoaded(&Prog_snapshot
                    , mw_snapshotSelItems[0]))
        {
            EnableWindow(mw_editBtn, TRUE);
            t = (type == META_VCED || type == META_PCED);
            EnableWindow(mw_storeBtn, t);
            goto THERE_IS_A_LOADED_ITEM_IN_THE_SELECTION;
        } else {
            EnableWindow(mw_bundleToLibBtn[0], FALSE);
            EnableWindow(mw_bundleToLibBtn[1], FALSE);
            EnableWindow(mw_editBtn, FALSE);
            EnableWindow(mw_storeBtn, FALSE);
        }
    } else {
        EnableWindow(mw_bundleToLibBtn[0], FALSE);
        EnableWindow(mw_bundleToLibBtn[1], FALSE);
        EnableWindow(mw_sendPCBtn, FALSE);
        EnableWindow(mw_editBtn, FALSE);
        EnableWindow(mw_storeBtn, FALSE);
        t = (mw_snapshotSelItemCnt > 1);
        EnableWindow(mw_snapshotRxItemsBtn, t);
    }
    for (j = 0; j < mw_snapshotSelItemCnt; j++) {
        if (Snapshot_IsItemsGroupLoaded(&Prog_snapshot
                    , mw_snapshotSelItems[j]))
        {
THERE_IS_A_LOADED_ITEM_IN_THE_SELECTION:
            for (i = 0; i < 2; i++) {
                EnableWindow(mw_addToLibBtn[i], TRUE);
            }
            EnableWindow(mw_snapshotTxItemsBtn, TRUE);
            goto UPDATECOPYBUTTON;
        }
    }
    for (i = 0; i < 2; i++) {
        EnableWindow(mw_addToLibBtn[i], FALSE);
    }
    EnableWindow(mw_snapshotTxItemsBtn, FALSE);
UPDATECOPYBUTTON:
    mw_UpdateCopyFromLibBtn();
    /*
     * If any library items are selected, activate the library buttons.
     */
    EnableWindow(mw_copyBtn, anyLibItemsAreSelected);
    EnableWindow(mw_deleteBtn, anyLibItemsAreSelected);
    /*
     * The rename button requires that only named items are selected, and the
     * diff button requires certain types and that the selected items are the
     * same type.
     */
    for (i = 0; i < mw_libSelItemCnt; i++) {
        TxLib_SetPtr(&Prog_lib[mw_libSelIndex], mw_libSelItems[i]);
        type = TxLib_GetItemType(&Prog_lib[mw_libSelIndex]);
        if (i > 0 && type != lastType) {
            anyNonDiffableItemsAreSelected = TRUE;
        }
        switch (type) {
            case META_PC:
            case META_MTO:
            case META_MTF:
                anyNonDiffableItemsAreSelected = TRUE;
            case META_FX:
            case META_SYS:
                anyLibNamelessItemsAreSelected = TRUE;
                break;
            case META_BNDL:
                anyNonDiffableItemsAreSelected = TRUE;
                break;
        }
        lastType = type;
    }
    EnableWindow(mw_renameBtn, anyLibItemsAreSelected
                && !anyLibNamelessItemsAreSelected);
    EnableWindow(mw_recommentBtn, anyLibItemsAreSelected);
    /*
     * The diff button requires exactly two library items of the same type
     * to be selected, and they must be VMEM, PMEM, FX, or SYS.
     */
    if (mw_libSelItemCnt == 2 && !anyNonDiffableItemsAreSelected) {
        EnableWindow(mw_diffBtn, TRUE);
    } else {
        EnableWindow(mw_diffBtn, FALSE);
    }
    /*
     * If only performances exist in the selection, activate the bundle buttons.
     */
    if (mw_snapshotSelItemCnt > 0) {
        onlyPfmsAreSelected = TRUE;
        for (i = 0; i < mw_snapshotSelItemCnt; i++) {
            type = Snapshot_GetItemType(mw_snapshotSelItems[i]);
            if ((type != META_PCED && type != META_PMEM)
                    || !Snapshot_IsItemsGroupLoaded(&Prog_snapshot
                        , mw_snapshotSelItems[i])) {
                onlyPfmsAreSelected = FALSE;
                break;
            }
        }
    } else {
        onlyPfmsAreSelected = FALSE;
    }
    EnableWindow(mw_bundleToLibBtn[0], onlyPfmsAreSelected);
    EnableWindow(mw_bundleToLibBtn[1], onlyPfmsAreSelected);
}

/*
 * UpdateCopyFromLibBtn() - Enables or disables the Copy From Lib button
 *                          based on item selection criteria.
 */
void mw_UpdateCopyFromLibBtn(void)
{
    TXLIB *txLib = &Prog_lib[mw_libSelIndex];
    int i;

    /*
     * If a single bundle in a library is selected, then enable the button.
     */
    if (mw_libSelItemCnt == 1) {
        TxLib_SetPtr(txLib, mw_libSelItems[0]);
        if (TxLib_GetItemType(txLib) == META_BNDL) {
            EnableWindow(mw_copyFromLibBtn, TRUE);
            return;
        }
    }
    /*
     * If there are either no snapshot items selected or no library items
     * selected, or the number of selected items between the two do not match,
     * then disable the button.
     */
    if (mw_snapshotSelItemCnt == 0 || mw_libSelIndex == -1
            || mw_snapshotSelItemCnt != mw_libSelItemCnt)
    {
        EnableWindow(mw_copyFromLibBtn, FALSE);
        return;
    }
    for (i = 0; i < mw_snapshotSelItemCnt; i++) {
        METAINDEX snapshotItemType;
        METAINDEX libItemType;

        /*
         * Get the snapshot item type for the next item and adjust it for
         * edit buffers.
         */
        snapshotItemType = Snapshot_GetItemType(mw_snapshotSelItems[i]);
        if (snapshotItemType == META_VCED) {
            snapshotItemType = META_VMEM;
        } else if (snapshotItemType == META_PCED) {
            snapshotItemType = META_PMEM;
        }
        /*
         * Get the library item type for the next item.
         */
        TxLib_SetPtr(txLib, mw_libSelItems[i]);
        libItemType = TxLib_GetItemType(txLib);
        if (snapshotItemType != libItemType) {
            EnableWindow(mw_copyFromLibBtn, FALSE);
            return;
        }
    }
    EnableWindow(mw_copyFromLibBtn, TRUE);
}

/*
 * UpdateEditors() - Updates the contents of editor dialogs after receiving
 *                   dumps.
 */
void mw_UpdateEditors(REQUEST reqFlags)
{
    if (Prog_voiceDlg && (reqFlags & REQ_VCED)) {
        VoiceDlg_Update(Prog_voiceDlg, &Prog_snapshot.avced);
    }
    if (Prog_pfmDlg && (reqFlags & (REQ_PCED | REQ_VMEM | REQ_PRESETS))) {
        PfmDlg_Update(Prog_pfmDlg, &Prog_snapshot.pced);
    }
    if (Prog_fxDlg && (reqFlags & REQ_FX)) {
        FxDlg_Update(Prog_fxDlg, &Prog_snapshot.fx);
    }
    if (Prog_pcDlg && (reqFlags & (REQ_PC | REQ_EDIT_BUFFERS | REQ_VMEM
                    | REQ_PMEM | REQ_PRESETS)))
    {
        PcDlg_Update(Prog_pcDlg, &Prog_snapshot.pc);
    }
    if (Prog_mtoDlg && (reqFlags & REQ_MTO)) {
        MtoDlg_Update(Prog_mtoDlg, &Prog_snapshot.mto);
    }
    if (Prog_mtfDlg && (reqFlags & REQ_MTF)) {
        MtfDlg_Update(Prog_mtfDlg, &Prog_snapshot.mtf);
    }
    if (Prog_sysDlg && (reqFlags & REQ_SYS)) {
        SysDlg_Update(Prog_sysDlg, &Prog_snapshot.sys);
    }
}

/*
 * UpdateLibLbx() - Refreshes a lib list box with recently loaded data.
 */
void mw_UpdateLibLbx(int libIndex, UPDATELBX update)
{
    if (update & UL_LBX) {
        _TUCHAR text[80];
        int topIndex;
        HWND listBox = mw_libLbx[libIndex];
        TXLIB *txLib = &Prog_lib[libIndex];
        TXLBX_ATTR itemAttr;

        /*
         * Save the scroll position.
         */
        topIndex = ListBox_GetTopIndex(listBox);
        /*
         * Remove all items from the list box.
         */
        ListBox_ResetContent(listBox);
        SetWindowRedraw(listBox, FALSE);
        /*
         * Add library items to the list box.
         */
        TxLib_Head(txLib);
        while (TxLib_Next(txLib)) {
            if (TxLib_IsItemDirty(txLib)) {
                itemAttr = TA_DIRTY;
            } else {
                itemAttr = TA_NORMAL;
            }
            TxLib_FormatName(txLib, NF_TYPE_NAME_AND_COMMENT, text);
            TxLbx_AddItem(listBox, text, 0, itemAttr);
        }
        /*
         * Restore the scroll position.
         */
        ListBox_SetTopIndex(listBox, topIndex);
        SetWindowRedraw(listBox, TRUE);
        /*
         * Update the count static.
         */
        _stprintf(text, _T("%d"), TxLib_GetCount(txLib));
        Static_SetText(mw_libCountStc[libIndex], text);
    }
    if (update & UL_TBX) {
        _TUCHAR *path = Prog_lib[libIndex].fileName;
        HWND tbx = mw_libTbx[libIndex];

        /*
         * Update the file name text box.
         */
        if (Prog_showFullPaths) {
            Edit_SetText(tbx, path);
        } else {
            Edit_SetText(tbx, FilePath_GetLeafPtr(path));
        }
    }
    /*
     * The selection has been removed - disable the Copy From Lib button.
     */
    EnableWindow(mw_copyFromLibBtn, FALSE);
}

/*
 * UpdateRecentMenu() - Updates a snapshot or library file menu with recent items.
 */
void mw_UpdateRecentMenu(HMENU menu, _TUCHAR recentList[Prog_recentFileMax][_MAX_PATH]
        , int originalItemCnt, _TUCHAR *curPath, UINT firstItemID)
{
    int menuItemCnt;
    int i;
    int splitPos;
    const _TUCHAR *leafPtr;
    const _TUCHAR *itemStr;

    /*
     * Get the menu in a condition where the recent list can be appended to it - the
     * normal file items and a separator at the bottom with no items below it.
     */
    menuItemCnt = GetMenuItemCount(menu);
    while (menuItemCnt > originalItemCnt) {
        --menuItemCnt;
        DeleteMenu(menu, menuItemCnt, MF_BYPOSITION);
    }
    /*
     * If there are no files in the recent list, just return.
     */
    if (recentList[0][0] == '\0') {
        return;
    }
    /*
     * Add a separator to the menu.
     */
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    /*
     * Add the recent file list to the menu, abbreviating the file names that are in
     * the same directory as the current file name.
     */
    for (i = 0; recentList[i][0] && i < Prog_recentFileMax; i++) {
        /*
         * Find the position of the beginning of the file name of the recent file.
         */
        leafPtr = FilePath_GetLeafPtr(recentList[i]);
        splitPos = leafPtr - recentList[i];
        /*
         * If the path of the recent file is the same as the path of the current file,
         * set the itemStr pointer to the file name only.
         */
        if (splitPos > -1 && StrEqN(recentList[i], curPath, splitPos)
                && !Prog_showFullPaths)
        {
            itemStr = leafPtr;
        } else {
            itemStr = recentList[i];
        }
        /*
         * Add the item to the menu.
         */
        AppendMenu(menu, MF_STRING, firstItemID + i, itemStr);
    }
}

/*
 * UpdateSnapshotLbx() - Refreshes all the items in the snapshot list box.
 */
void mw_UpdateSnapshotLbx(UPDATELBX update)
{
    if (update & UL_LBX) {
        _TUCHAR text[80];
        int topIndex;
        HWND listBox = mw_snapshotLbx;
        SNAPSHOT *snapshot = &Prog_snapshot;
        int i, sIdx;
        TXLBX_ATTR itemAttr;

        /*
         * Save the scroll position.
         */
        topIndex = ListBox_GetTopIndex(listBox);
        SetWindowRedraw(listBox, FALSE);

        /*
         * Remove all items from the list box.
         */
        ListBox_ResetContent(listBox);
        
        /*
         * Add voice edit buffer to the list box.
         */
        Snapshot_FormatName(snapshot, SI_VCED, NF_NUMBER_AND_NAME, text);
        if (Snapshot_IsItemGroupLoaded(snapshot, META_VCED)) {
            if (Snapshot_IsItemDirty(snapshot, SI_VCED)) {
                itemAttr = TA_DIRTY;
            } else {
                itemAttr = TA_NORMAL;
            }
        } else {
            itemAttr = TA_UNLOADED;
        }
        TxLbx_AddItem(listBox, text, 0, itemAttr);

        /*
         * Add performance edit buffer.
         */
        Snapshot_FormatName(snapshot, SI_PCED, NF_NUMBER_AND_NAME, text);
        if (Snapshot_IsItemGroupLoaded(snapshot, META_PCED)) {
            if (Snapshot_IsItemDirty(snapshot, SI_PCED)) {
                itemAttr = TA_DIRTY;
            } else {
                itemAttr = TA_NORMAL;
            }
        } else {
            itemAttr = TA_UNLOADED;
        }
        TxLbx_AddItem(listBox, text, 0, itemAttr);

        /*
         * Add effects, program change table, system setup, and micro tune tables.
         */
        for (sIdx = SI_FX; sIdx <= SI_SYS; sIdx++) {
            METAINDEX type = Snapshot_GetItemType(sIdx);

            if (Snapshot_IsItemGroupLoaded(snapshot, type)) {
                if (Snapshot_IsItemDirty(snapshot, sIdx)) {
                    itemAttr = TA_DIRTY;
                } else {
                    itemAttr = TA_NORMAL;
                }
            } else {
                itemAttr = TA_UNLOADED;
            }
            Snapshot_FormatName(snapshot, sIdx, NF_NUMBER_AND_NAME, text);
            TxLbx_AddItem(listBox, text, 0, itemAttr);
        }

        /*
         * Add the internal voice bank.
         */
        if (Snapshot_IsItemGroupLoaded(snapshot, META_VMEM)) {
            itemAttr = TA_NORMAL;
        } else {
            itemAttr = TA_UNLOADED;
        }
        for (sIdx = SI_VMEM; sIdx < SI_VMEM + 32; sIdx++) {
            if (itemAttr != TA_UNLOADED) {
                if (Snapshot_IsItemDirty(snapshot, sIdx)) {
                    itemAttr = TA_DIRTY;
                } else {
                    itemAttr = TA_NORMAL;
                }
            }
            Snapshot_FormatName(snapshot, sIdx, NF_NUMBER_AND_NAME, text);
            TxLbx_AddItem(listBox, text, 0, itemAttr);
        }
        /*
         * Add the performance bank.
         */
        if (Snapshot_IsItemGroupLoaded(snapshot, META_PMEM)) {
            itemAttr = TA_NORMAL;
        } else {
            itemAttr = TA_UNLOADED;
        }
        for (sIdx = SI_PMEM; sIdx < SI_PMEM + 24; sIdx++) {
            if (itemAttr != TA_UNLOADED) {
                if (Snapshot_IsItemDirty(snapshot, sIdx)) {
                    itemAttr = TA_DIRTY;
                } else {
                    itemAttr = TA_NORMAL;
                }
            }
            Snapshot_FormatName(snapshot, sIdx, NF_NUMBER_AND_NAME, text);
            TxLbx_AddItem(listBox, text, 0, itemAttr);
        }
        /*
         * Add the preset voice banks if any of them are loaded.
         */
        for (i = 0; i < 4; i++) {
            if (Snapshot_IsItemGroupLoaded(snapshot, META_PRESET_A + i)) {
                goto ShowPresetBanks;
            }
        }
        goto SkipPresetBanks;
ShowPresetBanks:
        for (i = 0; i < 4; i++) {
            int j;
            int bankIdx = SI_PRESET_A + i * 32;

            if (Snapshot_IsItemGroupLoaded(snapshot, META_PRESET_A + i)) {
                itemAttr = TA_NORMAL;
            } else {
                itemAttr = TA_UNLOADED;
            }
            for (j = 0; j < 32; j++) {
                sIdx = bankIdx + j;
                if (itemAttr != TA_UNLOADED) {
                    if (Snapshot_IsItemDirty(snapshot, sIdx)) {
                        itemAttr = TA_DIRTY;
                    } else {
                        itemAttr = TA_NORMAL;
                    }
                }
                Snapshot_FormatName(snapshot, sIdx, NF_NUMBER_AND_NAME, text);
                TxLbx_AddItem(listBox, text, 0, itemAttr);
            }
        }
SkipPresetBanks:
        /*
         * Restore the previous selections.
         */
        for (i = 0; i < mw_snapshotSelItemCnt; i++) {
            ListBox_SetSel(listBox, TRUE, mw_snapshotSelItems[i]);
        }
        /*
         * Restore the scroll position.
         */
        ListBox_SetTopIndex(listBox, topIndex);
        SetWindowRedraw(listBox, TRUE);
    }
    if (update & UL_TBX) {
        _TUCHAR *path = Prog_snapshot.extra.fileName;

        /*
         * Update the file name text box.
         */
        if (Prog_showFullPaths) {
            Edit_SetText(mw_snapshotTbx, path);
        } else {
            Edit_SetText(mw_snapshotTbx, FilePath_GetLeafPtr(path));
        }
    }
    mw_UpdateButtonStates();
}

/*
 * UpdateSnapshotLbxItem() - Updates the name and color of a non-bank item in
 *                           the snapshot list box after it has been edited
 *                           or its sysex received.
 */
void mw_UpdateSnapshotLbxItem(BOOL notifyEditorDlg, SINDEX sIdx
        , SELECTWHAT selectWhat)
{
    _TUCHAR itemName[40];
    HWND listBox = mw_snapshotLbx;
    SNAPSHOT *snapshot = &Prog_snapshot;
    int i;
    TXLBX_ATTR itemAttr;

    /*
     * Get the item name.
     */
    Snapshot_FormatName(snapshot, sIdx, NF_NUMBER_AND_NAME, itemName);
    /*
     * Set the item's color.
     */
    if (!Snapshot_IsItemsGroupLoaded(snapshot, sIdx)) {
        itemAttr = TA_UNLOADED;
    } else {
        if (Snapshot_IsItemDirty(snapshot, sIdx)) {
            itemAttr = TA_DIRTY;
        } else {
            itemAttr = TA_NORMAL;
        }
    }
    /*
     * Replace the item in the list box.
     */
    TxLbx_ReplaceItem(listBox, sIdx, itemName, 0, itemAttr);
    /*
     * Notify the item's editor of the change.
     */
    if (notifyEditorDlg && *mw_dlgPtrs[sIdx]) {
        PostMessage(*mw_dlgPtrs[sIdx], EDM_REFRESH, 0L, 0L);
    }
    /*
     * Remove current selections.
     */
    ListBox_SetSel(listBox, FALSE, -1);
    /*
     * On the select option.
     */
    if (selectWhat == SELECT_PREVIOUS) {
        /*
         * Restore listbox selections.
         */
        for (i = 0; i < mw_snapshotSelItemCnt; i++) {
            ListBox_SetSel(listBox, TRUE, mw_snapshotSelItems[i]);
        }
    } else if (selectWhat == SELECT_UPDATED) {
        /*
         * Select the updated item.
         */
        ListBox_SetSel(listBox, TRUE, sIdx);
        mw_snapshotSelItems[0] = sIdx;
        mw_snapshotSelItemCnt = 1;
    }
    mw_UpdateButtonStates();
}

/*
 * WndProc()
 */
LRESULT CALLBACK mw_WndProc(HWND mainWnd, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(mainWnd, WM_ACTIVATE, mw_OnActivate);
        HANDLE_MSG(mainWnd, WM_CLOSE, mw_OnClose);
        HANDLE_MSG(mainWnd, WM_COMMAND, mw_OnCommand);
        HANDLE_MSG(mainWnd, WM_CREATE, mw_OnCreate);
        HANDLE_MSG(mainWnd, WM_CTLCOLORSTATIC, mw_OnCtlColor);
        HANDLE_MSG(mainWnd, WM_DESTROY, mw_OnDestroy);
        HANDLE_MSG(mainWnd, WM_DRAWITEM, mw_OnDrawItem);
        HANDLE_MSG(mainWnd, WM_ERASEBKGND, mw_OnEraseBkgnd);
        HANDLE_MSG(mainWnd, WM_MEASUREITEM, mw_OnMeasureItem);
        HANDLE_MSG(mainWnd, WM_LBUTTONDOWN, mw_OnLButtonDown);
        HANDLE_MSG(mainWnd, WM_LBUTTONUP, mw_OnLButtonUp);
        HANDLE_MSG(mainWnd, WM_MOUSEMOVE, mw_OnMouseMove);
        HANDLE_MSG(mainWnd, WM_SETCURSOR, mw_OnSetCursor);
        HANDLE_MSG(mainWnd, WM_SIZE, mw_OnSize);
        HANDLE_MSG(mainWnd, WM_SIZING, mw_OnSizing);
        HANDLE_MSG(mainWnd, WM_SYSCOLORCHANGE, mw_OnSysColorChange);
    }
    return DefWindowProc(mainWnd, message, wParam, lParam);
}

