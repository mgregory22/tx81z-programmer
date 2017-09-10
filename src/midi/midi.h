/*
 * midi/midi.h - MIDI device module
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
#ifndef MIDI_MIDI_H
#define MIDI_MIDI_H

#ifndef _INC_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#ifndef _INC_MMSYSTEM
#   include <mmsystem.h>
#endif
#ifndef _INC_TCHAR
#   include <tchar.h>
#endif
#ifndef _INC_STDLIB
#   include <stdlib.h>
#endif
#ifndef MIDI_ERROR_H
#   include "midi/midierror.h"
#endif


/*
 * Constants
 */
#define MIDI_PORT_NAME_MAX   80  /* length of port name strings */


/* wParam = midi message,  lParam = time stamp */
#define MDM_SHORTMSG    (WM_USER + 90)   /* MIDI short message received */
/* wParam = address of MIDIHDR structure,  lParam = address of MIDI structure */
#define MDM_SYSEXMSG    (WM_USER + 91)   /* MIDI sysex data received */
/* wParam = 0, lParam = error */
#define MDM_ERROR       (WM_USER + 92)

#define HANDLE_MDM_SHORTMSG(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (DWORD)(wParam), (DWORD)(lParam)), 0L)
#define FORWARD_MDM_SHORTMSG(hwnd, midiMsg, timeStamp, fn) \
    (void)(fn)((hwnd), MDM_SHORTMSG, (WPARAM)(DWORD)(midiMsg), (LPARAM)(DWORD)(timeStamp))

#define HANDLE_MDM_SYSEXMSG(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (MIDIHDR *)(wParam), (MIDI *)(lParam)), 0L)
#define FORWARD_MDM_SYSEXMSG(hwnd, midiHdr, midi, fn) \
    (void)(fn)((hwnd), MDM_SYSEXMSG, (WPARAM)(DWORD)(midiHdr), (LPARAM)(DWORD)(midi))

#define HANDLE_MDM_ERROR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam)), 0L)
#define FORWARD_MDM_ERROR(hwnd, error, fn) \
    (void)(fn)((hwnd), MDM_ERROR, (WPARAM)(UINT)(error), 0)

/*
 * Dialog option constants
 */
#define MDO_ALLOW_OMNI_IN            0x01
#define MDO_ALLOW_OMNI_OUT           0x02
#define MDO_ALLOW_OMNI               0x03

/*
 * Extra MIDI errors
 */
#define MIDIERR_QUEUE_FULL (MIDIERR_LASTERROR + 1)

/*
 * Data types
 */
typedef struct {
    HWND wnd;        /* window to send MIDI message window messages */
    int outPort;
    _TUCHAR outPortName[MIDI_PORT_NAME_MAX];
    int outChannel;
    int inPort;
    _TUCHAR inPortName[MIDI_PORT_NAME_MAX];
    int inChannel;
    int masterInPort;
    _TUCHAR masterInPortName[MIDI_PORT_NAME_MAX];
    int bufSize;
    int bufCnt;
    HMIDIOUT midiOut;
    HANDLE outEvent;
    HMIDIIN midiIn;
    HMIDIIN midiMasterIn;
    BYTE *sysexBufs;
    MIDIHDR *midiHdrs;
} MIDI;

typedef union {
    BYTE c[4];
    DWORD dw;
} MIDIMSG;

/*
 * Function macros
 */

/*
 * AllNotesOff() - Sends an 'all notes off' message to the device.
 */
#define Midi_AllNotesOff(midi)                 Midi_SendShort((midi), 0x7B, 0, 0);

/*
 * ModWheel() - Sends a mod wheel controller message.
 */
#define Midi_ModWheel(midi, value)             Midi_SendShort((midi), 0xB0, 0x01, (value))

/*
 * NoteOn() - Sends a note on message.
 */
#define Midi_NoteOn(midi, note, velocity)      Midi_SendShort((midi), 0x90, (note), (velocity))

/*
 * NoteOff() - Sends a note off message.
 */
#define Midi_NoteOff(midi, note)               Midi_SendShort((midi), 0x80, (note), 0x00)

/*
 * NoteOffV() - Sends a 'note off' message with release velocity.
 */
#define Midi_NoteOffV(midi, note, velocity)    Midi_SendShort((midi), 0x80, (note), (velocity))

/*
 * ProgramChange() - Sends a program change message.
 */
#define Midi_ProgramChange(midi, program)      Midi_SendShort((midi), 0xC0, (program), 0x00)

/*
 * BufferDone() - Call at the end of the sysex message handler to return the
 *                buffer to the system. 
 */
MMRESULT Midi_BufferDone(MIDI *midi, MIDIHDR *midiHdr);

/*
 * Create() - Allocates and initializes a new MIDI object.  It doesn't open
 *            any ports - it's just for bookkeeping.
 */
MIDI *Midi_Create(HWND wnd, int bufSize, int bufCnt);

/*
 * Destroy() - Destroys a MIDI object.  All ports need to be closed before this
 *             function is called or your program will die a miserable death.
 */
void Midi_Destroy(MIDI *midi);

/*
 * InClose() - Closes a device opened with InOpen().
 */
MMRESULT Midi_InClose(MIDI *midi);

/*
 * InOpen() - Opens a MIDI device for listening.  The window specified by
 *            midi->wnd will be sent status messages when MIDI messages
 *            arrive.
 */
MMRESULT Midi_InOpen(MIDI *midi);

/*
 * LookupPortIndexes() -
 *      Converts port names to the port indexes that are used by the system.
 *      Returns TRUE if both ports were found.
 */
BOOL Midi_LookupPortIndexes(MIDI *midi);

/*
 * MasterInClose() - Closes the master controller port.
 */
MMRESULT Midi_MasterInClose(MIDI *midi);

/*
 * MasterInOpen() - Opens the master controller port.
 */
MMRESULT Midi_MasterInOpen(MIDI *midi);

/*
 * OutClose() - Closes the output port.
 */
MMRESULT Midi_OutClose(MIDI *midi);

/*
 * OutOpen() - Opens the output port.
 */
MMRESULT Midi_OutOpen(MIDI *midi);

/*
 * SendLong() - Sends a sysex message to the MIDI device and return the error
 *              status.
 */
MMRESULT Midi_SendLong(MIDI *midi, BYTE *buffer, size_t bufSize);

/*
 * SendShort() - Sends a short message to the MIDI device and returns the
 *               error status.
 */
MMRESULT Midi_SendShort(MIDI *midi, int status, int data1, int data2);

/*
 * SetupDialog() -
 *      Brings up a port setup dialog.  options specifies whether or not Omni
 *      mode can be selected using the MDO_ constants above.  caption is
 *      the window caption.  msg is for a static control in which instructions
 *      are displayed.
 */
BOOL Midi_SetupDialog(MIDI *dest, HWND parentWnd, HINSTANCE instance
        , DWORD options, _TUCHAR *caption, _TUCHAR *msg);


#endif  /* #ifndef MIDI_MIDI_H */
