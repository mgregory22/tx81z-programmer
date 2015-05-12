/*
 * midi/error.c - MIDI error message module
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

/*
 * Global constants
 */
const _TUCHAR Error_cannotOpenMidiOut[] = _T("Cannot open MIDI out port");
const _TUCHAR Error_cannotOpenMidiIn[] = _T("Cannot open MIDI in port");
const _TUCHAR Error_cannotOpenMidiMasterIn[] = _T("Cannot open master controller port");

/*
 * Unit constants
 */
static const _TUCHAR *e_sysErrMsg[] = {
    _T("No error."),                                         /* MMSYSERR_NOERROR */
    _T("Unspecified error."),                                /* MMSYSERR_ERROR */
    _T("The specified device identifier is out of range."),  /* MMSYSERR_BADDEVICEID */
    _T("Driver failed enable."),                             /* MMSYSERR_NOTENABLED */
    _T("The specified resource is already allocated."),      /* MMSYSERR_ALLOCATED */
    _T("The specified device handle is invalid."),           /* MMSYSERR_INVALHANDLE */
    _T("No device driver present."),                         /* MMSYSERR_NODRIVER */
    _T("The system is unable to allocate or lock memory."),  /* MMSYSERR_NOMEM */
    _T("Function isn't supported."),                         /* MMSYSERR_NOTSUPPORTED */
    _T("Error value out of range."),                         /* MMSYSERR_BADERRNUM */
    _T("The flags specified by dwFlags are invalid."),       /* MMSYSERR_INVALFLAG */
    _T("The specified pointer or structure is invalid."),    /* MMSYSERR_INVALPARAM */
    _T("The handle is being used."),                         /* MMSYSERR_HANDLEBUSY */
    _T("The specified alias was not found."),                /* MMSYSERR_INVALIDALIAS */
    _T("Bad registry database."),                            /* MMSYSERR_BADDB */
    _T("Registry key not found."),                           /* MMSYSERR_KEYNOTFOUND */
    _T("Registry read error."),                              /* MMSYSERR_READERROR */
    _T("Registry write error."),                             /* MMSYSERR_WRITEERROR */
    _T("Registry delete error."),                            /* MMSYSERR_DELETEERROR */
    _T("Registry value not found."),                         /* MMSYSERR_VALNOTFOUND */
    _T("Driver does not call DriverCallback.")               /* MMSYSERR_NODRIVERCB */
};

static const _TUCHAR *e_midiErrMsg[] = {
    _T("The buffer pointed to by lpMidiInHdr has not been prepared."), /* MIDIERR_UNPREPARED */
    _T("The buffer pointed to by lpMidiInHdr is still in the queue."), /* MIDIERR_STILLPLAYING */
    _T("No configured instruments."),                    /* MIDIERR_NOMAP */
    _T("The hardware is busy with other data."),         /* MIDIERR_NOTREADY */
    _T("No MIDI port was found. This error occurs only when the mapper is opened."), /* MIDIERR_NODEVICE */
    _T("Invalid MIF."),                                  /* MIDIERR_INVALIDSETUP */
    _T("Operation unsupported w/ open mode."),           /* MIDIERR_BADOPENMODE */
    _T("Thru device 'eating' a message.")                /* MIDIERR_DONT_CONTINUE */
};


/*
 * Global prodecures
 */
void Error_MidiError(HWND wnd, const _TUCHAR *msg, MMRESULT error);


/*
 * Procedure definitions
 */

/*
 * MidiError() - Displays a MIDI related error message.
 */
void Error_MidiError(HWND wnd, const _TUCHAR *msg, MMRESULT error)
{
    if (error <= MMSYSERR_LASTERROR) {
        MsgBox_F(wnd, _T("%s: %s"), msg, e_sysErrMsg[error]);
    } else if (error >= MIDIERR_BASE && error <= MIDIERR_LASTERROR) {
        MsgBox_F(wnd, _T("%s: %s"), msg, e_midiErrMsg[error - MIDIERR_BASE]);
    } else {
        MsgBox_F(wnd, _T("Unknown multimedia error."));
    }
}

