/* 
 * winx.h - some additions and alterations to windowsx.h
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

#ifndef GUI_WINX_H
#define GUI_WINX_H

#ifndef _INC_WINDOWSX
#   include <windowsx.h>
#endif


#define     SetWindowStyle(hwnd, style)  ((DWORD)SetWindowLong((hwnd), GWL_STYLE, (style)))

#define     SendNotification(hwnd, ctrlID, hwndCtrl, notify) \
    SendMessage((hwnd), WM_COMMAND, MAKEWPARAM((UINT)(ctrlID),(UINT)(notify)) \
            , (LPARAM)(HWND)(hwndCtrl))

#define     PostNotification(hwnd, ctrlID, hwndCtrl, notify) \
    PostMessage((hwnd), WM_COMMAND, MAKEWPARAM((UINT)(ctrlID),(UINT)(notify)) \
            , (LPARAM)(HWND)(hwndCtrl))

#define     RedrawWindowNow(hwnd) \
    RedrawWindow((hwnd), NULL, NULL \
            , RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW)

/****** Message crackers ****************************************************/

/*
void _OnSizing(HWND Wnd, int edge, RECT *rect)
*/
#define HANDLE_WM_SIZING(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (RECT *)(lParam)), TRUE)
#define FORWARD_WM_SIZING(hwnd, edge, rect, fn) \
    (void)(fn)((hwnd), WM_SIZING, (WPARAM)(int)(edge), (LPARAM)(RECT *)(rect))

/*
void _OnMouseWheel(HWND Wnd, int delta, int x, int y, UINT keyFlags)
*/
#undef HANDLE_WM_MOUSEWHEEL
#define HANDLE_WM_MOUSEWHEEL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)HIWORD(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(short)LOWORD(wParam)), 0L)
#undef FORWARD_WM_MOUSEWHEEL
#define FORWARD_WM_MOUSEWHEEL(hwnd, delta, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_MOUSEWHEEL, MAKEWPARAM((keyFlags), (delta)), MAKELPARAM((x), (y)))

/****** Button control message APIs ******************************************/

#define Button_IsChecked(hwndCtl)           (((BOOL)(DWORD)SNDMSG((hwndCtl), BM_GETCHECK, 0L, 0L)) == BST_CHECKED)
#define Button_Check(hwndCtl)               ((void)SNDMSG((hwndCtl), BM_SETCHECK, (WPARAM)(int)(BST_CHECKED), 0L))
#define Button_Uncheck(hwndCtl)             ((void)SNDMSG((hwndCtl), BM_SETCHECK, (WPARAM)(int)(BST_UNCHECKED), 0L))
#define Button_Toggle(hwndCtl)              Button_SetCheck((hwndCtl), Button_IsChecked(hwndCtl) ? BST_UNCHECKED : BST_CHECKED)
#define Button_Click(hwndCtl)               ((void)SNDMSG((hwndCtl), BM_CLICK, 0L, 0L))

#define Button_IsPushed(hwndCtl)            ((((BOOL)(DWORD)SNDMSG((hwndCtl), BM_GETSTATE, 0L, 0L)) & BST_PUSHED) != 0)
#define Button_Push(hwndCtl)                ((void)SNDMSG((hwndCtl), BM_SETSTATE, (WPARAM)(int)(Button_GetState(hwndCtl) | BST_PUSHED), 0L))
#define Button_Release(hwndCtl)             ((void)SNDMSG((hwndCtl), BM_SETSTATE, (WPARAM)(int)(Button_GetState(hwndCtl) & ~BST_PUSHED), 0L))

#define Button_GetImage(hwndCtl, type)          ((HANDLE)(DWORD)SNDMSG((hwndCtl), BM_GETIMAGE, (WPARAM)(type), (LPARAM) 0))
#define Button_GetBitmap(hwndCtl)               ((HANDLE)(DWORD)SNDMSG((hwndCtl), BM_GETIMAGE, (WPARAM)(IMAGE_BITMAP), (LPARAM) 0))
#define Button_SetImage(hwndCtl, type, image)   ((HANDLE)(DWORD)SNDMSG((hwndCtl), BM_SETIMAGE, (WPARAM)(type), (LPARAM)(HANDLE)(image)))
#define Button_SetIcon(hwndCtl, image)          ((HANDLE)(DWORD)SNDMSG((hwndCtl), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)(image)))
#define Button_SetBitmap(hwndCtl, image)        ((HANDLE)(DWORD)SNDMSG((hwndCtl), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HBITMAP)(image)))

/****** Edit control message APIs ********************************************/

#define Edit_GetSelDw(hwndCtl, lpStart, lpEnd)  ((DWORD)SNDMSG((hwndCtl), EM_GETSEL, (WPARAM)(DWORD *)lpStart, (LPARAM)(DWORD *)lpEnd))

/****** ListBox control message APIs *****************************************/

#define ListBox_GetCurItemData(hwndCtl)             ((LRESULT)(DWORD)SNDMSG((hwndCtl), LB_GETITEMDATA, (WPARAM)(SNDMSG((hwndCtl), LB_GETCURSEL, 0L, 0L)), 0L))
#define ListBox_ItemFromPoint(hwndCtl, lpPoint)    ((int)(DWORD)SNDMSG((hwndCtl), LB_ITEMFROMPOINT, 0L, MAKELPARAM((lpPoint->x), (lpPoint->y))))
#define ListBox_ItemFromLParam(hwndCtl, lParam)    ((int)(DWORD)SNDMSG((hwndCtl), LB_ITEMFROMPOINT, 0L, lParam))

/****** Menu APIs ************************************************************/
#define MenuItem_IsChecked(hmenu, cmdID)        ((GetMenuState((hmenu), (cmdID), MF_BYCOMMAND) & MF_CHECKED) != 0)
#define MenuItem_Check(hmenu, cmdID)            (CheckMenuItem((hmenu), (cmdID), MF_BYCOMMAND | MF_CHECKED))
#define MenuItem_Uncheck(hmenu, cmdID)          (CheckMenuItem((hmenu), (cmdID), MF_BYCOMMAND | MF_UNCHECKED))
#define MenuItem_RadioCheck(hmenu, firstID, lastID, cmdID)       (CheckMenuRadioItem((hmenu), (firstID), (lastID), (cmdID), MF_BYCOMMAND))
#define MenuItem_Disable(hmenu, cmdID)          EnableMenuItem((hmenu), (cmdID), MF_BYCOMMAND | MF_GRAYED)
#define MenuItem_Enable(hmenu, cmdID)           EnableMenuItem((hmenu), (cmdID), MF_BYCOMMAND | MF_ENABLED)
/* MenuItem_Toggle() returns TRUE if the item was toggled on.  flagPtr is a
 * pointer to BOOL. */
#define MenuItem_Toggle(hmenu, cmdID, flagPtr)  (!CheckMenuItem((hmenu), (cmdID), MF_BYCOMMAND | ((*(flagPtr) ^= TRUE) ? MF_CHECKED : MF_UNCHECKED)))

/****** Progress bar control message APIs **********************************/
#define ProgressBar_SetRange(hwndCtl, nMinRange, nMaxRange)   ((DWORD)SNDMSG((hwndCtl), PBM_SETRANGE, (WPARAM) 0L, MAKELPARAM((nMinRange), (nMaxRange))))
#define ProgressBar_SetStep(hwndCtl, nStepInc)                ((int)SNDMSG((hwndCtl), PBM_SETSTEP, (WPARAM) (nStepInc), (LPARAM) 0L))
#define ProgressBar_StepIt(hwndCtl)                           ((int)SNDMSG((hwndCtl), PBM_STEPIT, (WPARAM) 0L, (LPARAM) 0L))
#define ProgressBar_SetPos(hwndCtl, nPos)                     ((int)SNDMSG((hwndCtl), PBM_SETPOS, (WPARAM) (nPos), (LPARAM) 0L))

/****** ScrollBar control message APIs ***************************************/

#undef ScrollBar_SetRange
#define ScrollBar_SetRange(hwndCtl, posMin, posMax)  SendMessage(hwndCtl, SBM_SETRANGE, (posMin), (posMax))
#define ScrollBar_SetRangeRedraw(hwndCtl, posMin, posMax)  SendMessage(hwndCtl, SBM_SETRANGEREDRAW, (posMin), (posMax))

/* Reversed range scroll bars */
#define ScrollBar_GetRPos(hwnd, min, max)         (max - ScrollBar_GetPos(hwnd) + min)
#define ScrollBar_SetRPos(hwnd, value, min, max, redraw)  (ScrollBar_SetPos(hwnd, max - value + min, redraw))
#define ScrollBar_GetRInfo(hwnd, scrollInfo) \
    GetScrollInfo(hwnd, SB_CTL, &scrollInfo); \
    scrollInfo.nPos = scrollInfo.nMax - scrollInfo.nPos + scrollInfo.nMin;
#define ScrollBar_SetRInfo(hwnd, scrollInfo, redraw) \
    scrollInfo.nPos = scrollInfo.nMax - scrollInfo.nPos + scrollInfo.nMin; \
    SetScrollInfo(hwnd, SB_CTL, &scrollInfo, redraw); \
    scrollInfo.nPos = scrollInfo.nMax - scrollInfo.nPos + scrollInfo.nMin;

/****** Toolbar control message APIs ****************************************/

#define Toolbar_ButtonStructSize(hwndCtl, size)     ((void)SNDMSG((hwndCtl), TB_BUTTONSTRUCTSIZE, (WPARAM) size, 0L))
#define Toolbar_SetImageList(hwndCtl, imgList)      ((HIMAGELIST)SNDMSG((hwndCtl), TB_SETIMAGELIST, 0L, (LPARAM) imgList))
#define Toolbar_GetImageList(hwndCtl)               ((HIMAGELIST)SNDMSG((hwndCtl), TB_GETIMAGELIST, 0L, 0L))
#define Toolbar_AddButtons(hwndCtl, btnCnt, btnStruct)  ((BOOL)SNDMSG((hwndCtl), TB_ADDBUTTONS, (WPARAM) btnCnt, (LPARAM) btnStruct))
#define Toolbar_AddString(hwndCtl, buf)             ((int)SNDMSG((hwndCtl), TB_ADDSTRING, (WPARAM) NULL, (LPARAM) buf))
#define Toolbar_AutoSize(hwndCtl)                   ((void)SNDMSG((hwndCtl), TB_AUTOSIZE, 0L, 0L))

#define Toolbar_CheckButton(hwndCtl, btnID)         ((BOOL)SNDMSG((hwndCtl), TB_CHECKBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(TRUE, 0)))
#define Toolbar_UncheckButton(hwndCtl, btnID)       ((BOOL)SNDMSG((hwndCtl), TB_CHECKBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(FALSE, 0)))
#define Toolbar_PressButton(hwndCtl, btnID)         ((BOOL)SNDMSG((hwndCtl), TB_PRESSBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(TRUE, 0)))
#define Toolbar_ReleaseButton(hwndCtl, btnID)       ((BOOL)SNDMSG((hwndCtl), TB_PRESSBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(FALSE, 0)))
#define Toolbar_IsButtonChecked(hwndCtl, btnID)     ((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONCHECKED, (WPARAM)(btnID), 0L))
#define Toolbar_IsButtonPressed(hwndCtl, btnID)     ((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONPRESSED, (WPARAM)(btnID), 0L))
#define Toolbar_EnableButton(hwndCtl, btnID)        ((BOOL)SNDMSG((hwndCtl), TB_ENABLEBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(TRUE, 0)))
#define Toolbar_DisableButton(hwndCtl, btnID)       ((BOOL)SNDMSG((hwndCtl), TB_ENABLEBUTTON, (WPARAM)(btnID), (LPARAM) MAKELONG(FALSE, 0)))

/****** Track bar control message APIs **********************************/
#define TrackBar_SetRange(hwndCtl, nMinRange, nMaxRange, fRedraw)   ((void)SNDMSG((hwndCtl), TBM_SETRANGE, (WPARAM)(BOOL)(fRedraw), MAKELPARAM((nMinRange), (nMaxRange))))
#define TrackBar_SetPos(hwndCtl, nPos, fRedraw)                     ((void)SNDMSG((hwndCtl), TBM_SETPOS, (WPARAM)(BOOL)(fRedraw), (LPARAM)(nPos)))
#define TrackBar_GetPos(hwndCtl)                                    ((int)SNDMSG((hwndCtl), TBM_GETPOS, (WPARAM) 0L, (LPARAM) 0L))

/****** UpDown control message APIs ****************************************/
#define UpDown_SetRange(hwndCtl, iLow, iHigh)       ((void)SNDMSG((hwndCtl), UDM_SETRANGE, (WPARAM) 0L, (LPARAM) MAKELONG((iHigh), (iLow))))
#define UpDown_SetPos(hwndCtl, iPos)                ((int)SNDMSG((hwndCtl), UDM_SETPOS, 0L, (LPARAM)(iPos)))
#define UpDown_GetPos(hwndCtl)                      ((int)SNDMSG((hwndCtl), UDM_GETPOS, 0L, 0L))
#define UpDown_SetRange32(hwndCtl, iLow, iHigh)     ((void)SNDMSG((hwndCtl), UDM_SETRANGE32, (WPARAM) (iLow), (LPARAM) (iHigh)))
#define UpDown_SetPos32(hwndCtl, iPos)              ((int)SNDMSG((hwndCtl), UDM_SETPOS32, 0L, (LPARAM)(iPos)))
#define UpDown_GetPos32(hwndCtl, pfError)           ((int)SNDMSG((hwndCtl), UDM_GETPOS32, 0L, (LPARAM)(BOOL *) pfError))

#endif  /* #ifndef GUI_WINX_H */
