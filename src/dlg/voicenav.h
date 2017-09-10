/*
 * voicenav.h - Keyboard navigation array for the voice editor
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
#ifndef VOICENAV_H
#define VOICENAV_H

#include "dlg/keynav.h"

/*
 * Global variables
 */
extern KEYNAV VoiceNav_nav[];
extern const int VoiceNav_navCnt;
extern KEYALT VoiceNav_alt[];
extern const int VoiceNav_altCnt;

#endif  /* #ifndef VOICENAV_H */

