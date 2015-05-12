/*
 * keynav.h - keyboard navigation routines for the editor dialogs
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
#ifndef KEYNAV_H
#define KEYNAV_H


/*
 * Notification message constants
 */
#define KNN_FOCUSCHANGED  (WM_USER + 200)


/*
 * Global types
 */
typedef enum {
    KN_CTRLID = 0,
    KN_LEFT   = 1,
    KN_UP     = 2,
    KN_RIGHT  = 3,
    KN_DOWN   = 4
} KEYNAVIDX;

typedef enum {
    KA_CTRLID = 0,
    KA_ALTID  = 1
} KEYALTIDX;

typedef UINT KEYNAV[5];
typedef UINT KEYALT[2];

/*
 * Global procedures
 */
HWND KeyNav_FindAdjacentCtrl(HWND dlg, KEYNAV *keyNav, size_t keyNavCnt
        , KEYALT *keyAlt, size_t keyAltCnt, UINT focusID, KEYNAVIDX toDir
        , KEYNAVIDX fromDir, UINT *outNavIndex, BOOL savePrevious);
void KeyNav_SubclassButton(HWND button);
void KeyNav_SubclassComboBox(HWND comboBox);
BOOL CALLBACK KeyNav_SubclassControls(HWND ctrl, LPARAM lParam);
void KeyNav_SubclassLcdCtrl(HWND lcdCtrl);
void KeyNav_SubclassRPanel(HWND rPanel);
void KeyNav_SubclassScrollBar(HWND scrollBar);


#endif  /* #ifndef KEYNAV_H */
