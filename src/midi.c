/*
 * midi.c
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
#include "resource.h"

/*
 * Unit types
 */
typedef struct {
    DWORD options;
    _TUCHAR *caption;
    _TUCHAR *msg;
    HWND parentWnd;
} DLGPARAMS;

/*
 * Global procedures
 */
extern MMRESULT Midi_BufferDone(MIDI *midi, MIDIHDR *midiHdr);
extern MIDI *Midi_Create(HWND wnd, int bufSize, int bufCnt);
extern void Midi_Destroy(MIDI *midi);
extern MMRESULT Midi_InClose(MIDI *midi);
extern MMRESULT Midi_InOpen(MIDI *midi);
extern BOOL Midi_LookupPortIndexes(MIDI *midi);
extern MMRESULT Midi_MasterInClose(MIDI *midi);
extern MMRESULT Midi_MasterInOpen(MIDI *midi);
extern MMRESULT Midi_OutClose(MIDI *midi);
extern MMRESULT Midi_OutOpen(MIDI *midi);
extern MMRESULT Midi_SendLong(MIDI *midi, BYTE *buffer
        , size_t bufSize);
extern MMRESULT Midi_SendShort(MIDI *midi, int status, int data1, int data2);
extern BOOL Midi_SetupDialog(MIDI *dest, HWND parentWnd, HINSTANCE instance
        , DWORD options, _TUCHAR *caption, _TUCHAR *msg);

/*
 * Unit constants
 */
static const _TUCHAR *m_disabledStr = _T("Disabled");
static const _TUCHAR *m_omniStr = _T("omni");

/*
 * Unit procedures
 */
static MMRESULT m_AddInBuffer(MIDI *midi);
static BOOL CALLBACK m_DlgProc(HWND setupDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void CALLBACK m_MidiInProc(HMIDIIN midiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
//#define MIDI_THRU
#ifndef MIDI_THRU
static void CALLBACK m_MidiMasterInProc(HMIDIIN midiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
#endif
static void m_OnCommand(HWND setupDlg, int cmdID, HWND control, UINT notify);
static BOOL m_OnInitDialog(HWND setupDlg, HWND focusCtrl, LPARAM lParam);

/*
 * Unit variables
 */
static HWND m_inPortCbo;
static HWND m_outPortCbo;
static HWND m_masterInPortCbo;
static HWND m_inChannelCbo;
static HWND m_outChannelCbo;
static MIDI *m_midi;
static BOOL m_closing;


/*
 * Procedure definitions
 */
MMRESULT Midi_BufferDone(MIDI *midi, MIDIHDR *midiHdr)
{
    MMRESULT error;

    midiHdr->dwBytesRecorded = 0;
    error = midiInAddBuffer(midi->midiIn, midiHdr, sizeof(MIDIHDR));

    return error;
}

MIDI *Midi_Create(HWND wnd, int bufSize, int bufCnt)
{
    MIDI *midi = malloc(sizeof *midi);
    int i;

    if (!midi) {
        Error_OnError(E_MALLOC_ERROR);
        return NULL;
    }
    /*
     * Create the sysex data buffers.
     */
    midi->sysexBufs = calloc(bufSize, bufCnt);
    if (!midi->sysexBufs) {
        Error_OnError(E_MALLOC_ERROR);
        free(midi);
        return NULL;
    }
    /*
     * Create the midi header array.
     */
    midi->midiHdrs = calloc(sizeof(MIDIHDR), bufCnt);
    if (!midi->midiHdrs) {
        Error_OnError(E_MALLOC_ERROR);
        free(midi->sysexBufs);
        free(midi);
        return NULL;
    }
    /*
     * Set the midi header array data pointers.
     */
    for (i = 0; i < bufCnt; i++) {
        midi->midiHdrs[i].lpData = &midi->sysexBufs[bufSize * i];
        midi->midiHdrs[i].dwBufferLength = bufSize;
    }
    midi->bufSize = bufSize;
    midi->bufCnt = bufCnt;
    midi->wnd = wnd;
    midi->outPort = -1;  /* -1 = need to look up the port number from the name */
    midi->outPortName[0] = '\0';
    midi->outChannel = 0;
    midi->inPort = -1;  /* -1 = need to look up the port number from the name */
    midi->inPortName[0] = '\0';
    midi->inChannel = 0;
    midi->masterInPort = -1;
    midi->masterInPortName[0] = '\0';
    midi->midiOut = NULL;
    midi->outEvent = NULL;
    midi->midiIn = NULL;
    midi->midiMasterIn = NULL;

    return midi;
}

void Midi_Destroy(MIDI *midi)
{
    free(midi->midiHdrs);
    free(midi->sysexBufs);
    free(midi);
}

MMRESULT Midi_InClose(MIDI *midi)
{
    MMRESULT error;
    int i;

    if (midi->inPort == -1) {
        return MMSYSERR_NOERROR;
    }
    m_closing = TRUE;
    error = midiInReset(midi->midiIn);
    if (error != MMSYSERR_NOERROR) {
        return error;
    }
    /*
     * Unprepare the pending midi buffer header.
     */
    for (i = 0; i < midi->bufCnt; i++) {
UnprepareHeader:
        error = midiInUnprepareHeader(midi->midiIn, &midi->midiHdrs[i]
                , sizeof(MIDIHDR));
        if (error == MIDIERR_STILLPLAYING) {
            midiInReset(midi->midiIn);
            SleepEx(50, FALSE);
            goto UnprepareHeader;
        }
    }
CloseMidi:
    error = midiInClose(midi->midiIn);
    if (error == MIDIERR_STILLPLAYING) {
        midiInReset(midi->midiIn);
        SleepEx(50, FALSE);
        goto CloseMidi;
    }
    midi->midiIn = NULL;

    return error;
}

MMRESULT Midi_InOpen(MIDI *midi)
{
    MMRESULT error;
    int i;

    if (midi->inPort == -1) {
        return MMSYSERR_NOERROR;
    }
    /*
     * Open the port.
     */
    m_closing = FALSE;
    error = midiInOpen(&midi->midiIn, midi->inPort
            , (DWORD) m_MidiInProc, (DWORD) midi, CALLBACK_FUNCTION);
    if (error != MMSYSERR_NOERROR) {
        return error;
    }
    /*
     * Initialize the midi header array.
     */
    for (i = 0; i < midi->bufCnt; i++) {
        error = midiInPrepareHeader(midi->midiIn, &midi->midiHdrs[i]
                , sizeof(MIDIHDR));
        if (error != MMSYSERR_NOERROR) {
            for (i--; i >= 0; i--) {
                midiInUnprepareHeader(midi->midiIn, &midi->midiHdrs[i]
                        , sizeof(MIDIHDR));
            }
            midiInClose(midi->midiIn);
            return error;
        }
        error = midiInAddBuffer(midi->midiIn, &midi->midiHdrs[i]
                , sizeof(MIDIHDR));
        if (error != MMSYSERR_NOERROR) {
            midiInReset(midi->midiIn);
            for ( ; i >= 0; i--) {
                midiInUnprepareHeader(midi->midiIn, &midi->midiHdrs[i]
                        , sizeof(MIDIHDR));
            }
            midiInClose(midi->midiIn);
            return error;
        }
    }
    /*
     * Initiate MIDI input.
     */
    error = midiInStart(midi->midiIn);
    if (error != MMSYSERR_NOERROR) {
        midiInReset(midi->midiIn);
        midiInClose(midi->midiIn);
        return error;
    }

    return error;
}

BOOL Midi_LookupPortIndexes(MIDI *midi)
{
    BOOL setupValid = TRUE;
    int midiInDevCnt = midiInGetNumDevs();
    int midiOutDevCnt = midiOutGetNumDevs();
    int i;
    
    if (StrEq(midi->inPortName, _T("Disabled"))) {
        midi->inPort = -1;
        goto InPortFound;
    }
    for (i = 0; i < midiInDevCnt; i++) {
        MIDIINCAPS midiDevInfo;

        midiInGetDevCaps(i, &midiDevInfo, sizeof midiDevInfo);
        if (StrEq(midi->inPortName, midiDevInfo.szPname)) {
            midi->inPort = i;
            goto InPortFound;
        }
    }
    setupValid = FALSE;
InPortFound:
    if (StrEq(midi->outPortName, _T("Disabled"))) {
        midi->outPort = -1;
        goto OutPortFound;
    }
    for (i = 0; i < midiOutDevCnt; i++) {
        MIDIOUTCAPS midiDevInfo;

        midiOutGetDevCaps(i, &midiDevInfo, sizeof midiDevInfo);
        if (StrEq(midi->outPortName, midiDevInfo.szPname)) {
            midi->outPort = i;
            goto OutPortFound;
        }
    }
    setupValid = FALSE;
OutPortFound:
    if (StrEq(midi->masterInPortName, _T("Disabled"))) {
        midi->masterInPort = -1;
        goto MasterInPortFound;
    }
    for (i = 0; i < midiInDevCnt; i++) {
        MIDIINCAPS midiDevInfo;

        midiInGetDevCaps(i, &midiDevInfo, sizeof midiDevInfo);
        if (StrEq(midi->masterInPortName, midiDevInfo.szPname)) {
            midi->masterInPort = i;
            goto MasterInPortFound;
        }
    }
MasterInPortFound:

    return setupValid;
}

MMRESULT Midi_MasterInClose(MIDI *midi)
{
    MMRESULT error = MMSYSERR_NOERROR;

    if (!midi->midiMasterIn) {
        return MMSYSERR_NOERROR;
    }
#ifdef MIDI_THRU
    if (midi->midiOut) {
        midiDisconnect((HMIDI) midi->midiMasterIn, midi->midiOut, NULL);
    }
#endif
CloseMidi:
    error = midiInClose(midi->midiMasterIn);
    if (error == MIDIERR_STILLPLAYING) {
        midiInReset(midi->midiMasterIn);
        SleepEx(50, FALSE);
        goto CloseMidi;
    }
    midi->midiMasterIn = NULL;

    return error;
}

MMRESULT Midi_MasterInOpen(MIDI *midi)
{
    MMRESULT error;

    if (midi->masterInPort == -1 || midi->outPort == -1) {
        return MMSYSERR_NOERROR;
    }
    if (midi->masterInPort == midi->inPort) {
        return MMSYSERR_NOERROR;
    }
#ifdef MIDI_THRU
    /*
     * Open the port.
     */
    error = midiInOpen(&midi->midiMasterIn, midi->masterInPort, 0, 0
            , CALLBACK_NULL);
    if (error != MMSYSERR_NOERROR) {
        return error;
    }
    /*
     * Connect the master in port to the output port.
     */
    error = midiConnect((HMIDI) midi->midiMasterIn, midi->midiOut, NULL);
    if (error != MMSYSERR_NOERROR) {
        midiInClose(midi->midiMasterIn);
        return error;
    }
#else
    /*
     * Open the port.
     */
    error = midiInOpen(&midi->midiMasterIn, midi->masterInPort
            , (DWORD) m_MidiMasterInProc, (DWORD) midi, CALLBACK_FUNCTION);
    if (error != MMSYSERR_NOERROR) {
        return error;
    }
#endif
    error = midiInStart(midi->midiMasterIn);
    if (error != MMSYSERR_NOERROR) {
        midiInReset(midi->midiMasterIn);
        midiInClose(midi->midiMasterIn);
        return error;
    }

    return error;
}

MMRESULT Midi_OutClose(MIDI *midi)
{
    MMRESULT error = MMSYSERR_NOERROR;

    if (midi->outPort == -1) {
        return MMSYSERR_NOERROR;
    }
    if (midi->midiOut) {
#ifdef MIDI_THRU
        if (midi->midiMasterIn) {
            midiDisconnect((HMIDI) midi->midiMasterIn, midi->midiOut, NULL);
        }
#endif
        error = midiOutClose(midi->midiOut);
        if (error != MMSYSERR_NOERROR) {
            return error;
        }
        midi->midiOut = NULL;
    }
    if (midi->outEvent) {
        CloseHandle(midi->outEvent);
        midi->outEvent = NULL;
    }
    return error;
}

MMRESULT Midi_OutOpen(MIDI *midi)
{
    MMRESULT error;

    if (midi->outPort == -1) {
        return MMSYSERR_NOERROR;
    }
    midi->outEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!midi->outEvent) {
        MsgBox_LastError(midi->wnd);
        return MMSYSERR_NOERROR;
    }

    error = midiOutOpen(&midi->midiOut, midi->outPort
            , (DWORD) midi->outEvent, 0, CALLBACK_EVENT);
    if (error != MMSYSERR_NOERROR) {
        CloseHandle(midi->outEvent);
        midi->outEvent = NULL;
        return error;
    }
    return error;
}

MMRESULT Midi_SendLong(MIDI *midi, BYTE *buffer, size_t bufSize)
{
    HMIDIOUT midiOut = midi->midiOut;
    MIDIHDR outMsgHdr = {
        buffer,      /* sysex message */
        bufSize,     /* sysex message length */
        bufSize
    };
    MMRESULT error = MMSYSERR_NOERROR;

    if (midi->outPort == -1) {
        return MMSYSERR_NOERROR;
    }
    error = midiOutPrepareHeader(midiOut, &outMsgHdr, sizeof outMsgHdr);
    if (error)
        return error;
    error = midiOutLongMsg(midiOut, &outMsgHdr, sizeof outMsgHdr);
    if (error)
        goto UnprepareHeader;
    if (WaitForSingleObject(midi->outEvent, 500) == WAIT_TIMEOUT) {
        // ?
        goto UnprepareHeader;
    }
    ResetEvent(midi->outEvent);
UnprepareHeader:
    error = midiOutUnprepareHeader(midiOut, &outMsgHdr, sizeof outMsgHdr);
    if (error == MIDIERR_STILLPLAYING) {
        SleepEx(20, FALSE);
        goto UnprepareHeader;
    }

    return error;
}

MMRESULT Midi_SendShort(MIDI *midi, int status, int data1, int data2)
{
    MIDIMSG msg = {
        (status & 0xF0) | (midi->outChannel & 0x0F),
        data1 & 0x7F,
        data2 & 0x7F,
        0x00
    };

    if (midi->outPort == -1) {
        return MMSYSERR_NOERROR;
    }
    return midiOutShortMsg(midi->midiOut, msg.dw);
}

BOOL Midi_SetupDialog(MIDI *midi, HWND parentWnd, HINSTANCE instance
        , DWORD options, _TUCHAR *caption, _TUCHAR *msg)
{
    DLGPARAMS dlgParams = { options, caption, msg, parentWnd };
    MMRESULT error;
    MIDI copyOfMidi;

    /*
     * Create a copy of the important midi information in case the user cancels.
     */
    copyOfMidi.outPort = midi->outPort;
    _tcscpy(copyOfMidi.outPortName, midi->outPortName);
    copyOfMidi.outChannel = midi->outChannel;
    copyOfMidi.inPort = midi->inPort;
    _tcscpy(copyOfMidi.inPortName, midi->inPortName);
    copyOfMidi.inChannel = midi->inChannel;
    copyOfMidi.masterInPort = midi->masterInPort;
    _tcscpy(copyOfMidi.masterInPortName, midi->masterInPortName);
    /*
     * The midi structure will be changed as the user edits the midi settings.
     */
    m_midi = midi;
    /*
     * Open the setup dialog.
     */
    if (!DialogBoxParam(instance, MAKEINTRESOURCE(IDD_MIDIDLG)
            , parentWnd, m_DlgProc, (LPARAM) &dlgParams))
    {
        /*
         * If the user cancels, close the midi ports.
         */
        if (midi->midiIn) {
            Midi_InClose(midi);
        }
        if (midi->midiMasterIn) {
            Midi_MasterInClose(midi);
        }
        if (midi->midiOut) {
            Midi_OutClose(midi);
        }
        /*
         * Restore the backup copy of the midi structure.
         */
        midi->inPort = copyOfMidi.inPort;
        midi->outPort = copyOfMidi.outPort;
        midi->masterInPort = copyOfMidi.masterInPort;
        midi->inChannel = copyOfMidi.inChannel;
        midi->outChannel = copyOfMidi.outChannel;
        _tcscpy(midi->inPortName, copyOfMidi.inPortName);
        _tcscpy(midi->outPortName, copyOfMidi.outPortName);
        _tcscpy(midi->masterInPortName, copyOfMidi.masterInPortName);
        /*
         * Reopen the midi ports.
         */
        if (error = Midi_InOpen(midi)) {
            Error_MidiError(parentWnd, Error_cannotOpenMidiIn, error);
        }
        if (error = Midi_OutOpen(midi)) {
            Error_MidiError(parentWnd, Error_cannotOpenMidiOut, error);
        }
        if (error = Midi_MasterInOpen(midi)) {
            Error_MidiError(parentWnd, Error_cannotOpenMidiMasterIn, error);
        }
    }

    return TRUE;
}

BOOL CALLBACK m_DlgProc(HWND setupDlg, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(setupDlg, WM_INITDIALOG, m_OnInitDialog);
        HANDLE_MSG(setupDlg, WM_COMMAND, m_OnCommand);
    }
    return FALSE;
}

void CALLBACK m_MidiInProc(HMIDIIN midiIn, UINT wMsg, DWORD dwInstance
        , DWORD dwParam1, DWORD dwParam2)
{
#define midi ((MIDI *) dwInstance)
#define dwMidiMessage dwParam1
#define lpMidiHdr     ((MIDIHDR *) dwParam1)
#define dwTimeStamp   dwParam2

    switch (wMsg) {
        case MIM_DATA:
            if (midi->inPort == midi->masterInPort && midi->midiOut) {
                midiOutShortMsg(midi->midiOut, dwMidiMessage);
            }
            /*
            PostMessage(midi->wnd, MDM_SHORTMSG, (WPARAM) dwMidiMessage
                    , (LPARAM) dwTimeStamp);
            */
            break;
        case MIM_LONGDATA:
            if (m_closing) {
                return;
            }
            PostMessage(midi->wnd, MDM_SYSEXMSG, (WPARAM) lpMidiHdr
                    , (LPARAM) midi);
            break;
    }
#undef midi
#undef dwMidiMessage
#undef lpMidiHdr
#undef dwTimeStamp
}

#ifndef MIDI_THRU
void CALLBACK m_MidiMasterInProc(HMIDIIN midiIn, UINT wMsg, DWORD dwInstance
        , DWORD dwParam1, DWORD dwParam2)
{
#define midi ((MIDI *) dwInstance)
#define dwMidiMessage dwParam1
#define lpMidiHdr     ((MIDIHDR *) dwParam1)
#define dwTimeStamp   dwParam2

    if (wMsg == MIM_DATA) {
        if (midi->midiOut) {
            midiOutShortMsg(midi->midiOut, dwMidiMessage);
        }
    }
#undef midi
#undef dwMidiMessage
#undef lpMidiHdr
#undef dwTimeStamp
}
#endif

/*
 * OnCommand()
 */
void m_OnCommand(HWND setupDlg, int cmdID, HWND control, UINT notify)
{
    MMRESULT error;

    switch (cmdID) {
        case IDC_MIDI_TEST_BTN:
            if (Midi_NoteOn(m_midi, 60, 100) == MMSYSERR_NOERROR) {
                SleepEx(500, FALSE);
                Midi_NoteOff(m_midi, 60);
                SleepEx(50, FALSE);
            }
            break;
        case IDOK:
            /*
             * Copy the port names into the MIDI structure.  The combo boxes
             * need to be set again in case a midi open error occurred and
             * wound up disabling one of the ports.
             */
            ComboBox_SetCurSel(m_inPortCbo, m_midi->inPort + 1);
            ComboBox_SetCurSel(m_outPortCbo, m_midi->outPort + 1);
            ComboBox_SetCurSel(m_masterInPortCbo, m_midi->masterInPort + 1);
            ComboBox_GetText(m_inPortCbo, m_midi->inPortName
                    , MIDI_PORT_NAME_MAX);
            ComboBox_GetText(m_outPortCbo, m_midi->outPortName
                    , MIDI_PORT_NAME_MAX);
            ComboBox_GetText(m_masterInPortCbo, m_midi->masterInPortName
                    , MIDI_PORT_NAME_MAX);
            /*
             * Fall through.
             */
        case IDCANCEL:
            /*
             * Close the midi setup dialog.
             */
            EndDialog(setupDlg, cmdID == IDOK);
            break;
        case IDC_MIDI_IN_PORT_CBO:
            if (notify == CBN_SELCHANGE) {
                /*
                 * Close the midi in port.
                 */
                if (m_midi->midiIn) {
                    Midi_InClose(m_midi);
                }
                /*
                 * Get the port information from the in port combo box.
                 */
                m_midi->inPort = ComboBox_GetCurSel(m_inPortCbo) - 1;
                /*
                 * Disable/enable the channel combo box if the port is
                 * disabled/enabled.
                 */
                EnableWindow(m_inChannelCbo, m_midi->inPort == -1 ? FALSE
                        : TRUE);
                /*
                 * Reopen the midi in port with the new setting.
                 */
                if (error = Midi_InOpen(m_midi)) {
                    Error_MidiError(setupDlg, Error_cannotOpenMidiIn, error);
                    m_midi->inPort = -1;
                    ComboBox_SetCurSel(m_inPortCbo, 0);
                }
            }
            break;
        case IDC_MIDI_OUT_PORT_CBO:
            if (notify == CBN_SELCHANGE) {
                /*
                 * Close the midi out port.
                 */
                if (m_midi->midiOut) {
                    Midi_OutClose(m_midi);
                }
                /*
                 * Get the output port from the out port combo box.
                 */
                m_midi->outPort = ComboBox_GetCurSel(m_outPortCbo) - 1;
                /*
                 * Disable/enable the output channel depending on the output
                 * port.
                 */
                EnableWindow(m_outChannelCbo, m_midi->outPort == -1 ? FALSE
                        : TRUE);
                /*
                 * Reopen the output port with the new setting.
                 */
                if (error = Midi_OutOpen(m_midi)) {
                    Error_MidiError(setupDlg, Error_cannotOpenMidiOut, error);
                    m_midi->outPort = -1;
                    ComboBox_SetCurSel(m_outPortCbo, 0);
                }
            }
            break;
        case IDC_MIDI_MASTER_PORT_CBO:
            if (notify == CBN_SELCHANGE) {
                /*
                 * Close the master controller port.
                 */
                if (m_midi->midiMasterIn) {
                    Midi_MasterInClose(m_midi);
                }
                /*
                 * Get the new master controller port from the combo box.
                 */
                m_midi->masterInPort = ComboBox_GetCurSel(m_masterInPortCbo)
                    - 1;
                /*
                 * Open the master controller port with the new setting.
                 */
                if (error = Midi_MasterInOpen(m_midi)) {
                    Error_MidiError(setupDlg, Error_cannotOpenMidiMasterIn
                            , error);
                    m_midi->masterInPort = -1;
                    ComboBox_SetCurSel(m_masterInPortCbo, 0);
                }
            }
            break;
        case IDC_MIDI_IN_CHANNEL_CBO:
            if (notify == CBN_SELCHANGE) {
                /*
                 * Store the midi in channel in the midi structure as the user
                 * changes it.
                 */
                m_midi->inChannel = ComboBox_GetCurSel(m_inChannelCbo);
            }
            break;
        case IDC_MIDI_OUT_CHANNEL_CBO:
            if (notify == CBN_SELCHANGE) {
                /*
                 * Store the midi out channel in the midi structure as the user
                 * changes it.
                 */
                m_midi->outChannel = ComboBox_GetCurSel(m_outChannelCbo);
            }
            break;
    }
}

/*
 * OnInitDialog()
 */
BOOL m_OnInitDialog(HWND setupDlg, HWND focusCtrl, LPARAM lParam)
{
    DLGPARAMS *dlgParams = (DLGPARAMS *) lParam;
    HWND msgLabel = GetDlgItem(setupDlg, IDC_MSG_LBL);
    int midiInDevCnt, midiOutDevCnt;
    int i;

    /*
     * Center the window.
     */
    Window_Center(setupDlg, dlgParams->parentWnd);
    /*
     * Cache control handles.
     */
    m_inPortCbo = GetDlgItem(setupDlg, IDC_MIDI_IN_PORT_CBO);
    m_inChannelCbo = GetDlgItem(setupDlg, IDC_MIDI_IN_CHANNEL_CBO);
    m_outPortCbo = GetDlgItem(setupDlg, IDC_MIDI_OUT_PORT_CBO);
    m_outChannelCbo = GetDlgItem(setupDlg, IDC_MIDI_OUT_CHANNEL_CBO);
    m_masterInPortCbo = GetDlgItem(setupDlg, IDC_MIDI_MASTER_PORT_CBO);
    /*
     * Set the window caption and general message text.
     */
    if (dlgParams->caption) {
        SetWindowText(setupDlg, dlgParams->caption);
    }
    if (dlgParams->msg) {
        SetWindowText(msgLabel, dlgParams->msg);
    }
    /*
     * Init the In Port and Master Port combos.
     */
    midiInDevCnt = midiInGetNumDevs();
    ComboBox_AddString(m_inPortCbo, m_disabledStr);
    ComboBox_AddString(m_masterInPortCbo, m_disabledStr);
    for (i = 0; i < midiInDevCnt; i++) {
        MIDIINCAPS midiDevInfo;

        midiInGetDevCaps(i, &midiDevInfo, sizeof midiDevInfo);
        ComboBox_AddString(m_inPortCbo, midiDevInfo.szPname);
        ComboBox_AddString(m_masterInPortCbo, midiDevInfo.szPname);
    }
    ComboBox_SetCurSel(m_inPortCbo, m_midi->inPort + 1);
    ComboBox_SetCurSel(m_masterInPortCbo, m_midi->masterInPort + 1);
    /*
     * Init the In Channel combo.
     */
    for (i = 1; i <= 16; i++) {
        _TUCHAR chanStr[3];

        _stprintf(chanStr, _T("%d"), i);
        ComboBox_AddString(m_inChannelCbo, chanStr);
    }
    if (dlgParams->options & MDO_ALLOW_OMNI_IN) {
        ComboBox_AddString(m_inChannelCbo, m_omniStr);
    }
    ComboBox_SetCurSel(m_inChannelCbo, m_midi->inChannel);
    EnableWindow(m_inChannelCbo, m_midi->inPort == -1 ? FALSE : TRUE);
    /*
     * Init the Out Port combo.
     */
    midiOutDevCnt = midiOutGetNumDevs();
    ComboBox_AddString(m_outPortCbo, m_disabledStr);
    for (i = 0; i < midiOutDevCnt; i++) {
        MIDIOUTCAPS midiDevInfo;

        midiOutGetDevCaps(i, &midiDevInfo, sizeof midiDevInfo);
        ComboBox_AddString(m_outPortCbo, midiDevInfo.szPname);
    }
    ComboBox_SetCurSel(m_outPortCbo, m_midi->outPort + 1);
    /*
     * Init the Out Channel combo.
     */
    for (i = 1; i <= 16; i++) {
        _TUCHAR chanStr[3];

        _stprintf(chanStr, _T("%d"), i);
        ComboBox_AddString(m_outChannelCbo, chanStr);
    }
    if (dlgParams->options & MDO_ALLOW_OMNI_OUT) {
        ComboBox_AddString(m_outChannelCbo, m_omniStr);
    }
    ComboBox_SetCurSel(m_outChannelCbo, m_midi->outChannel);
    EnableWindow(m_outChannelCbo, m_midi->outPort == -1 ? FALSE : TRUE);

    return TRUE;
}

