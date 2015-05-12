/*
 * voicemenu.h - Voice selection flyout window
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
#ifndef VOICEMENU_H
#define VOICEMENU_H

#ifndef _INC_WINDOWS
#   include <windows.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif

/*
 * Global constants
 */
extern const _TUCHAR *VoiceMenu_className;

/*
 * Global procedures
 */
HWND VoiceMenu_Create(HWND parentWnd, UINT voiceMenuCmd);
int VoiceMenu_GetLastSelection(void);
BOOL VoiceMenu_Register(void);
UINT VoiceMenu_Select(HWND voiceMenu, RECT *btnRect);
void VoiceMenu_SetLastSelection(int lastSelection);
void VoiceMenu_Update(HWND voiceMenu);


#endif  /* #ifndef VOICEMENU_H */
