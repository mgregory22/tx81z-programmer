/*
 * kybddlg.h - Keyboard control window
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
#ifndef KYBDDLG_H
#define KYBDDLG_H

/*
 * Global constants
 */
extern const _TUCHAR *KybdDlg_className;

/*
 * Global procedures
 */
BOOL KybdDlg_Create(HWND parentWnd);


#endif  /* #ifndef KYBDDLG_H */
