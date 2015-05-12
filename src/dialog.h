/*
 * gui/dialog.h - Generic dialog routines
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
#ifndef GUI_DIALOG_H
#define GUI_DIALOG_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif


/*
 * Global types
 */
typedef struct {
    HWND wnd;
    SCROLLINFO hScrollInfo;
    SCROLLINFO vScrollInfo;
    RECT clientRect;
    SIZE diffSize;
    SIZE ctrlAreaSize;
    BOOL hideScrollBars;
#ifdef DIALOG_SIZEBOX
    HWND sizeBox;
#endif
} DIALOG;

/*
 * Create() - Allocates and initializes a new dialog object, but not the
 *            window.  The actual window should be created normally and
 *            partially managed by this module.
 */
DIALOG *Dialog_Create(HWND dialogWnd);

/*
 * Destroy() - Frees a dialog object (but not the window it refres to).
 */
void Dialog_Destroy(DIALOG *dlg);

/*
 * DoEvents() - Dispatches current events for a dialog.  Returns TRUE if the
 *              WM_QUIT message was received.
 */
BOOL Dialog_DoEvents(HWND dialogWnd);

/*
 * FitToControls() - Resizes the window to fit its controls.
 */
void Dialog_FitToControls(DIALOG *dialog);

/*
 * GenericProc() - A generic procedure for modal dialogs that handles OK and
 *                 cancel buttons.
 */
BOOL CALLBACK Dialog_GenericProc(HWND dlg, UINT message, WPARAM wParam
        , LPARAM lParam);

/*
 * OnHScroll() - WM_HSCROLL handler.  Specify Dialog_OnHScroll in HANDLE_MSG
 *               using a DIALOG* as the first parameter instead of an HWND.
 */
void Dialog_OnHScroll(DIALOG *dialog, HWND control, UINT code, int pos);

/*
 * OnSize() - WM_SIZE handler.  Specify Dialog_OnSize in HANDLE_MSG
 *            using a DIALOG* as the first parameter instead of an HWND.
 */
void Dialog_OnSize(DIALOG *dialog, UINT state, int cx, int cy);

/*
 * OnVScroll() - WM_VSCROLL handler.  Specify Dialog_OnVScroll in HANDLE_MSG.
 *               using a DIALOG* as the first parameter instead of an HWND.
 */
void Dialog_OnVScroll(DIALOG *dialog, HWND control, UINT code, int pos);

#ifdef DIALOG_SIZEBOX
/*
 * PaintSizer() - Repaints the sizer.
 */
__inline void Dialog_PaintSizeBox(DIALOG *dialog);
#endif

/*
 * Scroll() - Scrolls a dialog window with appropriate limits, etc.
 */
void Dialog_Scroll(DIALOG *dlg, int dx, int dy);

/*
 * UpdateScrollBars() - Updates scroll bars after a window resize and shows or
 *                      hides scroll bars based on the window size and area of
 *                      the child controls.
 */
void Dialog_UpdateScrollBars(DIALOG *dialog);


/*
 * Inline definitions
 */
#ifdef DIALOG_SIZEBOX
void Dialog_PaintSizeBox(DIALOG *dialog)
{
    if (IsWindowVisible(dialog->sizeBox)) {
        RedrawWindow(dialog->sizeBox, NULL, NULL, RDW_ERASE | RDW_INVALIDATE
                | RDW_INTERNALPAINT | RDW_UPDATENOW);
    }
}
#endif


#endif  /* #ifndef GUI_DIALOG_H */
