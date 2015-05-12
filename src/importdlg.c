/*
 * importdlg.c - Import dialog template module
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
#include "prog.h"
#include "resource.h"
#include "txlib.h"
#include "importdlg.h"

/*
 * Global procedures
 */
extern BOOL ImportDlg_Create(HWND parentWnd, _TUCHAR *initialDir, _TUCHAR *multiFileNames, size_t multiFileNameLen);

/*
 * Unit constants
 */
#define IO_BOTH  (IO_VMEMS | IO_PMEMS)

/*
 * Unit procedures
 */
static BOOL CALLBACK id_DlgProc(HWND importDlg, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL id_OnInitDialog(HWND importDlg, HWND focusCtrl, LPARAM lParam);
static LRESULT id_OnNotify(HWND importDlg, int ctrlID, LPNMHDR notifyInfo);


/*
 * Procedure definitions
 */

/*
 * Create()
 */
BOOL ImportDlg_Create(HWND parentWnd, _TUCHAR *initialDir, _TUCHAR *multiFileNames
        , size_t multiFileNameLen)
{
    OPENFILENAME ofn = {
        sizeof ofn,            /* DWORD         lStructSize;  */
        parentWnd,             /* HWND          hwndOwner;  */
        Prog_instance,         /* HINSTANCE     hInstance;  */
        Prog_allFileFilter,    /* LPCTSTR       lpstrFilter;  */
        NULL,                  /* LPTSTR        lpstrCustomFilter;  */
        0,                     /* DWORD         nMaxCustFilter;  */
        0,                     /* DWORD         nFilterIndex;  */
        multiFileNames,        /* LPTSTR        lpstrFile;  */
        multiFileNameLen,      /* DWORD         nMaxFile;  */
        NULL,                  /* LPTSTR        lpstrFileTitle;  */
        0,                     /* DWORD         nMaxFileTitle;  */
        initialDir,            /* LPCTSTR       lpstrInitialDir;  */
        _T("Import File"),     /* LPCTSTR       lpstrTitle;  */
        OFN_ALLOWMULTISELECT   /* DWORD         Flags;  */
            | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_EXPLORER
            | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        0,                     /* WORD          nFileOffset;  */
        0,                     /* WORD          nFileExtension;  */
        Prog_syxExt,           /* LPCTSTR       lpstrDefExt;  */
        0,                     /* LPARAM        lCustData;  */
        id_DlgProc,            /* LPOFNHOOKPROC lpfnHook;  */
        MAKEINTRESOURCE(IDD_IMPORTDLG)   /* LPCTSTR       lpTemplateName;  */
    };
    multiFileNames[0] = '\0';

    if (GetOpenFileName(&ofn)) {
        FileDlg_SaveInitialDir(initialDir, multiFileNames);

        return TRUE;
    }

    return FALSE;
}

/*
 * DlgProc()
 */
BOOL CALLBACK id_DlgProc(HWND importDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(importDlg, WM_INITDIALOG, id_OnInitDialog);
        HANDLE_MSG(importDlg, WM_NOTIFY, id_OnNotify);
    }
    return FALSE;
}

/*
 * OnInitDialog()
 */
BOOL id_OnInitDialog(HWND importDlg, HWND focusCtrl, LPARAM lParam)
{
    /*
     * Set the "Open" button text.
     */
    CommDlg_OpenSave_SetControlText(GetParent(importDlg), IDOK, _T("Import"));
    /*
     * Initialize the check boxes.
     */
    if ((Prog_importOptions & IO_BOTH) == IO_BOTH) {
        CheckRadioButton(importDlg, IDC_VMEM_RDO, IDC_BOTH_RDO, IDC_BOTH_RDO);
    } else if (Prog_importOptions & IO_PMEMS) {
        CheckRadioButton(importDlg, IDC_VMEM_RDO, IDC_BOTH_RDO, IDC_PMEM_RDO);
    } else {
        CheckRadioButton(importDlg, IDC_VMEM_RDO, IDC_BOTH_RDO, IDC_VMEM_RDO);
    }
    if (Prog_importOptions & IO_NO_DUPLICATES) {
        CheckDlgButton(importDlg, IDC_NO_DUPLICATES_CHK, BST_CHECKED);
    }
    if (Prog_importOptions & IO_UNIQUE_NAMES) {
        CheckDlgButton(importDlg, IDC_UNIQUE_NAMES_CHK, BST_CHECKED);
    }
    if (Prog_importOptions & IO_CHECK_EVERY_POS) {
        CheckDlgButton(importDlg, IDC_CHECK_EVERY_POS_CHK, BST_CHECKED);
    }

    return TRUE;
}

LRESULT id_OnNotify(HWND importDlg, int ctrlID, LPNMHDR notifyInfo)
{
    if (notifyInfo->code == CDN_INITDONE) {
        Window_CenterInParent(GetParent(importDlg));
    } else if (notifyInfo->code == CDN_FILEOK) {
        /*
         * Save the state of the check boxes.
         */
#define SET_IF_CHECKED(ctrlID, flag) \
        if (IsDlgButtonChecked(importDlg, ctrlID) == BST_CHECKED) \
            Prog_importOptions |= flag

        Prog_importOptions = 0;
        SET_IF_CHECKED(IDC_VMEM_RDO, IO_VMEMS);
        SET_IF_CHECKED(IDC_PMEM_RDO, IO_PMEMS);
        SET_IF_CHECKED(IDC_BOTH_RDO, IO_BOTH);
        SET_IF_CHECKED(IDC_NO_DUPLICATES_CHK, IO_NO_DUPLICATES);
        SET_IF_CHECKED(IDC_UNIQUE_NAMES_CHK, IO_UNIQUE_NAMES);
        SET_IF_CHECKED(IDC_CHECK_EVERY_POS_CHK, IO_CHECK_EVERY_POS);
    }
    return 0;
}

