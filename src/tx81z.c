/*
 * tx81z.c - interface to TX81Z synthesizer
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
#include "snapshot.h"
#include "txpack.h"
#include "TX81Z.h"

/*
 * Global constants
 */
const _TUCHAR *TX81Z_className = _T("TX81Z");
#include "tx81z_inits.c"

/*
 * Global procedures
 */
extern HWND TX81Z_Create(HWND parentWnd);
extern void TX81Z_Destroy(HWND tx81zWnd);
extern METAINDEX TX81Z_DetectSysexType(MIDI *midi, BYTE *data);
extern void TX81Z_InitPfm(MIDI *midi, SNAPSHOT *snapshot, const BYTE *pfmInit);
extern void TX81Z_InitVoice(MIDI *midi, SNAPSHOT *snapshot);
extern BOOL TX81Z_IsMemoryProtectOn(MIDI *midi, SNAPSHOT *snapshot);
extern void TX81Z_Panic(MIDI *midi, SNAPSHOT *snapshot);
extern void TX81Z_ProgramChange(MIDI *midi, int bank, int number);
extern BOOL TX81Z_Register(void);
extern BOOL TX81Z_RetrieveData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot);
extern void TX81Z_SendData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot);
extern void TX81Z_SendParamChange(MIDI *midi, int subgroup, int paramIndex, int value);
extern void TX81Z_SendPCParamChange(MIDI *midi, int pcNumber, int memNum);
extern void TX81Z_SendRemote(MIDI *midi, BYTE remoteBtn, REMOTEACTION action);
extern BOOL TX81Z_StorePfm(MIDI *midi, int destIndex);
extern BOOL TX81Z_StoreVoice(MIDI *midi, int destIndex);
extern void TX81Z_SwitchToPfmMode(MIDI *midi);
extern void TX81Z_SwitchToVoiceMode(MIDI *midi);

/*
 * Global variables.
 */
extern TX81ZMETA TX81Z_meta[META_CNT];

/*
 * Unit types/constants
 */
#define RETRIEVE_TIMER 2
#define RETRIEVE_TIMER_DURATION 2000
#define MSG_TIMER 3
#define MSG_DELAY_DEFAULT 70
#define REMOTE_DELAY 125
#define TRANSMIT_TIMER 4
#define TRANSMIT_TIMER_DURATION 125

typedef enum {
    MODE_UNKNOWN,
    MODE_SINGLE,
    MODE_SINGLE_EDIT,
    MODE_SINGLE_UTIL,
    MODE_PERFORM,
    MODE_PERFORM_EDIT,
    MODE_PERFORM_UTIL,
} MODE;

/*
 * Unit procedures
 */
static void tx_OnCommand(HWND tx81zWnd, UINT cmdID, HWND ctrl, UINT notify);
static void tx_OnMidiError(HWND tx81zWnd, UINT error);
static void tx_OnSysexMsg(HWND tx81zWnd, MIDIHDR *midiHdr, MIDI *midi);
static void tx_OnTimer(HWND tx81zWnd, UINT timerID);
static void tx_SendLong(MIDI *midi, BYTE *buffer, size_t bufSize);
static void tx_SendFirstPresetBankRequest(MIDI *midi);
static void tx_SendNextDump(MIDI *midi);
static void tx_SendNextDumpMsg(MIDI *midi);
static void tx_SendNextPresetBankRequest(MIDI *midi);
#ifdef SHOW_MODE
static int tx_lineNum;
#define tx_ShowMode()  tx_lineNum = __LINE__; tx_ShowModeAux();
static void tx_ShowModeAux(void);
#endif  /* #ifdef SHOW_MODE */
static BOOL tx_ShowRxProgressDlg(MIDI *midi, REQUEST reqFlags);
static BOOL tx_ShowTxProgressDlg(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot);
static void tx_SwitchMode(MIDI *midi);
static void tx_UpdateMode(BYTE remoteBtn);
static void tx_UpdatePfmVoices(MIDI *midi, int bank, int number);
static LRESULT CALLBACK tx_WndProc(HWND tx81zWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Unit variables
 */
static HWND tx_progressBar;
static HWND tx_msgLbl;
static DWORD tx_reqFlags;
static DWORD tx_msgDelay = MSG_DELAY_DEFAULT;
static DWORD tx_lastMsgTime;
static CDVLIST *tx_outMsgList;
static const _TUCHAR tx_vmemName[] = _T("32 internal voices");
static const _TUCHAR tx_pmemName[] = _T("32 internal performances");
static const _TUCHAR tx_bankNameFmt[] = _T("%-10.10s");
static const _TUCHAR tx_bankAName[] = _T("voice bank A");
static const _TUCHAR tx_bankBName[] = _T("voice bank B");
static const _TUCHAR tx_bankCName[] = _T("voice bank C");
static const _TUCHAR tx_bankDName[] = _T("voice bank D");
static SNAPSHOT tx_tempSnapshot;
static MODE tx_curMode;
static BOOL tx_receivedPartialBuffer;
static BOOL tx_rxTimerActive;
static METAINDEX tx_partialBufferType;
static BOOL tx_retry;
static BOOL tx_transmitTimerFired;
static BOOL tx_hasSystemSetupBeenReceivedThisRun;

#include "tx81z_meta.c"
static const METAINDEX tx_reqOrder[META_CNT] = {
    META_ACED,
    META_VCED,
    META_PCED,
    META_FX,
    META_PC,
    META_MTO,
    META_MTF,
    META_SYS,
    META_VMEM,
    META_PMEM,
    META_VOICE_MODE,
    META_PFM_MODE,
    META_PRESET_A,
    META_PRESET_B,
    META_PRESET_C,
    META_PRESET_D
};


/*
 * Procedure definitions
 */

/*
 * Create() - Creates the TX81Z window, output message queue, etc.
 */
HWND TX81Z_Create(HWND parentWnd)
{
    HWND tx81zWnd;

    tx_outMsgList = CDVList_Create();
    if (!tx_outMsgList)
        return NULL;
    tx_lastMsgTime = GetTickCount();
    tx_curMode = MODE_UNKNOWN;
#ifdef SHOW_MODE
    tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */

    tx81zWnd = CreateDialog(Prog_instance, MAKEINTRESOURCE(IDD_TX81ZDLG)
            , NULL, NULL);
    if (!tx81zWnd) {
        MsgBox_LastErrorF(parentWnd, _T("Error creating TX81Z window"));
    }
    tx_progressBar = GetDlgItem(tx81zWnd, IDC_PROGRESSBAR);
    tx_msgLbl = GetDlgItem(tx81zWnd, IDC_MSG_LBL);

    return tx81zWnd;
}

/*
 * Destroy() - Cleans up the window and stuff.
 */
void TX81Z_Destroy(HWND tx81zWnd)
{
    CDVList_Destroy(tx_outMsgList);
    DestroyWindow(tx81zWnd);
}

/*
 * DetectSysexType() - Tries to figure out what kind of dump *data contains.
 *                     Returns -1 if the buffer doesn't start with a valid
 *                     TX81Z sysex header.  The <midi> parameter is used to
 *                     filter the channel.  If it's null, then the channel is
 *                     ignored.
 */
METAINDEX TX81Z_DetectSysexType(MIDI *midi, BYTE *data)
{
    METAINDEX i;

    for (i = 0; i < META_CNT; i++) {
        BYTE *dumpHdr = TX81Z_meta[i].dumpHdr;
        size_t dumpHdrLen = TX81Z_meta[i].dumpHdrLen;

        if (dumpHdr) {
            int channelByte = (dumpHdr[2] & 0xF0)
                | ((midi ? midi->inChannel : data[2]) & 0x0F);
            /*
             * Compare the dump headers with the file data.
             */
            if (data[0] == dumpHdr[0] && data[1] == dumpHdr[1]
                    && data[2] == channelByte
                    && memcmp(&dumpHdr[3], &data[3], dumpHdrLen - 3) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

/*
 * InitPfm() - Initializes the performance edit buffer with the data in pfmInit.
 */
void TX81Z_InitPfm(MIDI *midi, SNAPSHOT *snapshot, const BYTE *pfmInit)
{
    TXPack_PmemToPced(pfmInit, &snapshot->pced);
    if (midi->outPort == -1) {
        return;
    }
    TX81Z_SendData(midi, REQ_PCED, snapshot);
}

/*
 * InitVoice() - Initializes the voice edit buffer and retrieves it.
 */
void TX81Z_InitVoice(MIDI *midi, SNAPSHOT *snapshot)
{
    TXPack_VmemToAvced(TX81Z_initVMem, &snapshot->avced, TRUE);
    if (midi->outPort == -1) {
        return;
    }
    TX81Z_SendData(midi, REQ_AVCED, snapshot);
}

/*
 * IsMemoryProtectOn() - Checks memory protect and prompts user to turn it off.
 */
BOOL TX81Z_IsMemoryProtectOn(MIDI *midi, SNAPSHOT *snapshot)
{
    if (midi->outPort == -1) {
        return FALSE;
    }
    if (!tx_hasSystemSetupBeenReceivedThisRun) {
        TX81Z_RetrieveData(midi, REQ_SYS, snapshot);
    }
    if (snapshot->sys.data[TX81Z_SYS_MEMPROT_SW]) {
        if (MessageBox(GetForegroundWindow(), _T("Memory protect is on.  ")
                _T("Do you want me to turn it off and continue?")
                , _T("Memory Protect")
                , MB_YESNO) == IDNO)
        {
            return TRUE;
        }
        snapshot->sys.data[TX81Z_SYS_MEMPROT_SW] = 0;
        TX81Z_SendParamChange(midi, TX81Z_SUBGRP_SYS, TX81Z_SYS_MEMPROT_SW, 0);
        TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
        tx_curMode = MODE_SINGLE;
#ifdef SHOW_MODE
        tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
    }
    return FALSE;
}

/*
 * Panic() - Cuts off all sound on the synth by sending a poly/mono mode
 *           message.  Note: this switches the unit to SINGLE->EDIT mode.
 */
void TX81Z_Panic(MIDI *midi, SNAPSHOT *snapshot)
{
    if (midi->outPort == -1) {
        return;
    }
    TX81Z_SendParamChange(midi, TX81Z_SUBGRP_VCED, TX81Z_VCED_POLY_MODE
            , snapshot->avced.vced.data[TX81Z_VCED_POLY_MODE]);
    tx_curMode = MODE_SINGLE_EDIT;
#ifdef SHOW_MODE
    tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
}

/*
 * ProgramChange() - Changes to the specified bank and voice/performance number.
 */
void TX81Z_ProgramChange(MIDI *midi, int bank, int number)
{
    int i;
    BYTE *pcTable = Prog_snapshot.pc.data;
    int memNum = bank * 32 + number;
    int msb = memNum >> 7;
    int lsb = memNum & 0x7F;
    MIDIMSG pcMsg;
    int originalMode = tx_curMode;

    /*
     * Do nothing if MIDI is disabled.
     */
    if (midi->outPort == -1) {
        return;
    }
    /*
     * Get the program change table if it's not loaded.  Note that it might be
     * wrong if the snapshot was loaded from a file instead of the unit.
     */
    if (!Snapshot_IsItemsGroupLoaded(&Prog_snapshot, memNum)) {
        TX81Z_RetrieveData(midi, REQ_PC, &Prog_snapshot);
    }

    if (tx_curMode != MODE_SINGLE && tx_curMode != MODE_PERFORM)  {
        /*
         * Switch the unit to either play or performance mode, it doesn't
         * matter which.
         */
        TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
        originalMode = tx_curMode;
    }
    /*
     * Search the program change table for the requested memory number and if
     * found send the change message.
     */
    for (i = 0; i < 256; i += 2) {
        if (pcTable[i + 1] == lsb) {
            if (pcTable[i] == msb) {
                /*
                 * #1: The unit ignores performance program changes when the
                 * program change mode is set to "ind", so switch the unit to
                 * voice mode before sending the program change.  I don't know
                 * why it's like that.
                 */
                if (tx_curMode >= MODE_PERFORM
                        && Snapshot_IsPCModeSetToInd(&Prog_snapshot))
                {
                    TX81Z_SwitchToVoiceMode(midi);
                }
                Midi_ProgramChange(midi, i >> 1);
                /*
                 * Check for and update the performance.
                 */
                tx_UpdatePfmVoices(midi, bank, number);
                /*
                 * Update the mode.
                 */
                if (bank == TX81Z_BANK_PF) {
                    tx_curMode = MODE_PERFORM;
                } else {
                    tx_curMode = MODE_SINGLE;
                }
#ifdef SHOW_MODE
                tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
                return;
            }
        }
    }
    /*
     * The patch didn't exist in the table, so temporarily change the table to
     * perform the change.
     */
    TX81Z_SendPCParamChange(midi, 0, memNum);
    /*
     * The unit is now in utility mode, change it back to play mode to do the
     * program change, otherwise it won't have any effect.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    /*
     * Check the original mode so that individual program changes will affect
     * the performance instead of changing the single program.  See comment
     * labeled #1 above about the Prog_snapshot part.  In short, I don't want
     * to change it to performance mode here.
     */
    if (originalMode >= MODE_PERFORM
            && Prog_snapshot.sys.data[TX81Z_SYS_PROGCH_SW] != 2) {
        TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    }
    /*
     * Use tx_SendLong to send the program change message so the messages
     * don't arrive out of order.
     */
    pcMsg.c[0] = 0xC0 | midi->outChannel;
    pcMsg.c[1] = 0;
    tx_SendLong(midi, pcMsg.c, 2);
    TX81Z_SendPCParamChange(midi, 0
            , (Prog_snapshot.pc.data[0] << 7) + Prog_snapshot.pc.data[1]);
    /*
     * In utility mode again, change it back to play mode.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    /*
     * originalMode is for individual performance changes, bank is for changing
     * the mode to performance if the program change is a performance.
     */
    if (originalMode >= MODE_PERFORM || bank == TX81Z_BANK_PF) {
        /*
         * Switch to performance mode
         */
        TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    }
}

/*
 * Register()
 */
BOOL TX81Z_Register(void)
{
    WNDCLASSEX classInfo;

    classInfo.cbSize = sizeof(classInfo);
    classInfo.style = 0;
    classInfo.lpfnWndProc = (WNDPROC) tx_WndProc;
    classInfo.cbClsExtra = 0;
    classInfo.cbWndExtra = DLGWINDOWEXTRA;
    classInfo.hInstance = Prog_instance;
    classInfo.hIcon = Prog_icon;
    classInfo.hCursor = Prog_arrowCursor;
    classInfo.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
    classInfo.lpszMenuName = NULL;
    classInfo.lpszClassName = TX81Z_className;
    classInfo.hIconSm = NULL;

    if (!RegisterClassEx(&classInfo)) {
        MsgBox_LastErrorF(NULL, _T("Error registering TX81Z window"));
        return FALSE;
    }
    return TRUE;
}

/*
 * RetrieveData() - Requests and retrieves bulk dumps from the synth with a
 *                  progress dialog.
 */
BOOL TX81Z_RetrieveData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot)
{
    int i;

    if (midi->outPort == -1 || midi->inPort == -1) {
        return FALSE;
    }
    /*
     * Set the output channel for the dump requests.
     */
    for (i = 0; i < META_CNT; i++) {
        int req = tx_reqOrder[i];
        if ((reqFlags & TX81Z_meta[req].reqFlag) && TX81Z_meta[req].reqStr) {
            TX81Z_meta[req].reqStr[2] &= 0xF0;
            TX81Z_meta[req].reqStr[2] |= midi->outChannel & 0x0F;
        }
    }

    /*
     * Show the progress dialog and get the dumps specified in reqFlags.
     */
    if (tx_ShowRxProgressDlg(midi, reqFlags)) {
        /*
         * If all the dumps were successfully read, copy them to the working
         * snapshot.
         */
        for (i = 0; i < META_CNT; i++) {
            TX81ZMETA *meta = &TX81Z_meta[i];

            if ((reqFlags & meta->reqFlag) && meta->buf) {
                Snapshot_LoadSysex(snapshot, i, meta->buf);
            }
        }
    } else {
        return FALSE;
    }
    if (reqFlags & REQ_SYS) {
        tx_hasSystemSetupBeenReceivedThisRun = TRUE;
    }

    return TRUE;
}

/*
 * SendData() - Sends a bulk dump to the synth.
 */
void TX81Z_SendData(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot)
{
    if (midi->outPort == -1) {
        return;
    }
    if (reqFlags & REQ_NEEDS_PROTECT_CHECK) {
        if (TX81Z_IsMemoryProtectOn(midi, snapshot)) {
            return;
        }
    }
    Snapshot_UpdateSysexWrappers(snapshot);
    tx_ShowTxProgressDlg(midi, reqFlags, snapshot);
}

/*
 * SendParamChange() - Sends a parameter change message to the synth w/ speed
 *                     throttling.
 */
void TX81Z_SendParamChange(MIDI *midi, int subgroup, int paramIndex, int value)
{
    if (midi->outPort == -1) {
        return;
    }
    if (subgroup <= 3) {
        BYTE parmChMsg[] = {
            '\xF0',
            '\x43',
            '\x10' | (midi->outChannel & 0x0F),
            '\x10' | subgroup,
            paramIndex & 0x7F,
            value & 0x7F,
            '\xF7'
        };
        tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
        tx_curMode = (subgroup == TX81Z_SUBGRP_PCED) ? MODE_PERFORM_EDIT
            : MODE_SINGLE_EDIT;
    } else if (subgroup == TX81Z_SUBGRP_SYS) {
        BYTE parmChMsg[] = {
            '\xF0',
            '\x43',
            '\x10' | (midi->outChannel & 0x0F),
            '\x10',
            '\x7B',
            paramIndex & 0x7F,
            value & 0x7F,
            '\xF7'
        };
        tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
        tx_curMode = MODE_SINGLE_UTIL;
    } else if (subgroup == TX81Z_SUBGRP_FX) {
        BYTE parmChMsg[] = {
            '\xF0',
            '\x43',
            '\x10' | (midi->outChannel & 0x0F),
            '\x10',
            '\x7C',
            paramIndex & 0x7F,
            value & 0x7F,
            '\xF7'
        };
        tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
        tx_curMode = MODE_SINGLE_UTIL;
    } else if (subgroup == TX81Z_SUBGRP_MTO) {
        BYTE parmChMsg[] = {
            '\xF0',
            '\x43',
            '\x10' | (midi->outChannel & 0x0F),
            '\x10',
            '\x7D',
            paramIndex & 0x7F,
            (value >> 8) & 0x7F,
            value & 0x7F,
            '\xF7'
        };
        tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
        tx_curMode = MODE_SINGLE_UTIL;
    } else if (subgroup == TX81Z_SUBGRP_MTF) {
        BYTE parmChMsg[] = {
            '\xF0',
            '\x43',
            '\x10' | (midi->outChannel & 0x0F),
            '\x10',
            '\x7E',
            paramIndex & 0x7F,
            (value >> 8) & 0x7F,
            value & 0x7F,
            '\xF7'
        };
        tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
        tx_curMode = MODE_SINGLE_UTIL;
    }
#ifdef SHOW_MODE
    tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
}

/*
 * SendPCParamChange() - Sends a program change parameter change message.
 */
void TX81Z_SendPCParamChange(MIDI *midi, int pcNumber, int memNum)
{
    BYTE parmChMsg[] = {
        '\xF0',
        '\x43',
        '\x10' | (midi->outChannel & 0x0F),
        '\x10',
        '\x7F',
        pcNumber & 0x7F,
        memNum >> 7,
        memNum & 0x7F,
        '\xF7'
    };

    if (midi->outPort == -1) {
        return;
    }
    tx_SendLong(midi, parmChMsg, sizeof parmChMsg);
    tx_curMode = MODE_SINGLE_UTIL;
#ifdef SHOW_MODE
    tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
}

/*
 * SendRemote() - Sends a remote control command to the synth.
 */
void TX81Z_SendRemote(MIDI *midi, BYTE remoteBtn, REMOTEACTION action)
{
    BYTE remoteMsg[] = {
        '\xF0',
        '\x43',
        '\x10' | (midi->outChannel & 0x0F),
        '\x13',
        remoteBtn,
        '\x7F',
        '\xF7'
    };

    if (midi->outPort == -1) {
        return;
    }
    if (action == RA_DOWN || action == RA_CLICK) {
        tx_SendLong(midi, remoteMsg, sizeof remoteMsg);
        tx_UpdateMode(remoteBtn);
    }
#ifdef SHOW_MODE
    tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
    if (action == RA_UP || action == RA_CLICK) {
        remoteMsg[5] = '\x00';
        tx_SendLong(midi, remoteMsg, sizeof remoteMsg);
    }
}

BOOL TX81Z_StorePfm(MIDI *midi, int destIndex)
{
    if (midi->outPort == -1) {
        return FALSE;
    }
    if (TX81Z_IsMemoryProtectOn(Prog_midi, &Prog_snapshot)) {
        return FALSE;
    }
    /*
     * Update the sysex headers and footers in the snapshot.
     */
    Snapshot_UpdateSysexWrappers(&Prog_snapshot);
    /*
     * Retrieve the program change table from the unit.
     */
    TX81Z_RetrieveData(midi, REQ_PC, &Prog_snapshot);
    /*
     * Make sure the unit is in performance mode.
     */
    TX81Z_SwitchToPfmMode(midi);
    /*
     * Change the unit to the first performance, so the program can store the
     * item in the right position.
     */
    TX81Z_ProgramChange(midi, TX81Z_BANK_PF, destIndex);
    /*
     * Send the performance data, since the program change will probably have
     * wiped it out on the unit.
     */
    TX81Z_SendData(midi, REQ_PCED, &Prog_snapshot);
    /*
     * Send a store button down message to the unit.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_STORE, RA_DOWN);
    /*
     * Select the patch number for storing.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);
    /*
     * Send a release store button message.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_STORE, RA_UP);
    /*
     * Send a yes button click.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);

    return TRUE;
}

/*
 * StoreVoice() - Stores a voice in the unit.
 */
BOOL TX81Z_StoreVoice(MIDI *midi, int destIndex)
{
    if (midi->outPort == -1) {
        return FALSE;
    }
    if (TX81Z_IsMemoryProtectOn(Prog_midi, &Prog_snapshot)) {
        return FALSE;
    }
    Snapshot_UpdateSysexWrappers(&Prog_snapshot);
    /*
     * Retrieve the program change table from the unit.
     */
    TX81Z_RetrieveData(midi, REQ_PC, &Prog_snapshot);
    /*
     * Switch to voice mode.
     */
    TX81Z_SwitchToVoiceMode(midi);
    /*
     * Change to the destination patch.
     */
    TX81Z_ProgramChange(midi, TX81Z_BANK_I, destIndex);
    /*
     * Send the voice data again, since it got wiped out.
     */
    TX81Z_SendData(midi, REQ_AVCED, &Prog_snapshot);
    /*
     * Push the store button.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_STORE, RA_DOWN);
    /*
     * Select the patch number to store the voice in.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);
    /*
     * Release the store button.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_STORE, RA_UP);
    /*
     * Click yes to store.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);

    return TRUE;
}

/*
 * SwitchToPfmMode() - Switches the unit to performance mode.
 */
void TX81Z_SwitchToPfmMode(MIDI *midi)
{
    /*
    if (tx_curMode != MODE_PERFORM) {
        TX81Z_RetrieveData(midi, REQ_PFM_MODE, NULL);
    }
    */
    TX81Z_Panic(midi, &Prog_snapshot);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
}

/*
 * SwitchToVoiceMode() - Switches the unit to single mode.
 */
void TX81Z_SwitchToVoiceMode(MIDI *midi)
{
    /*
     * A dump won't be received if the unit is in Utility mode.
     */
    /*
    if (tx_curMode == MODE_SINGLE_UTIL) {
        tx_SwitchMode(midi);
    } else if (tx_curMode != MODE_SINGLE) {
        TX81Z_RetrieveData(midi, REQ_VOICE_MODE, NULL);
    }
    */
    TX81Z_Panic(midi, &Prog_snapshot);
    SleepEx(MSG_DELAY_DEFAULT, FALSE);
    Dialog_DoEvents(midi->wnd);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    SleepEx(MSG_DELAY_DEFAULT, FALSE);
    Dialog_DoEvents(midi->wnd);
}

/*
 * OnCommand() - Cancel button.
 */
void tx_OnCommand(HWND tx81zWnd, UINT cmdID, HWND ctrl, UINT notify)
{
    if (cmdID == IDCANCEL || cmdID == IDOK) {
        tx_reqFlags = 0;
        tx_receivedPartialBuffer = FALSE;
        //Midi_ClearQueue(Prog_midi);
        if (cmdID == IDCANCEL) {
            CDVList_Clear(tx_outMsgList);
        }
        KillTimer(tx81zWnd, RETRIEVE_TIMER);
        tx_rxTimerActive = FALSE;
        /*
         * Just hide the window, don't actually destroy it until the program
         * ends, so it can continue to handle MIDI messages.
         */
        EnableWindow(GetParent(tx81zWnd), TRUE);
        ShowWindow(tx81zWnd, SW_HIDE);
        SetWindowLong(tx81zWnd, GWL_HWNDPARENT, (LONG) HWND_DESKTOP);
    }
}

/*
 * OnMidiError() - Displays an error message box.
 */
void tx_OnMidiError(HWND tx81zWnd, UINT error)
{
    SendMessage(tx81zWnd, WM_COMMAND, (WPARAM) IDCANCEL, (LPARAM) 0L);
    Error_MidiError(tx81zWnd, _T("Error receiving MIDI data"), error);
}

/*
 * OnSysexMsg() - Deals with bulk dump requests.
 */
void tx_OnSysexMsg(HWND tx81zWnd, MIDIHDR *midiHdr, MIDI *midi)
{
    METAINDEX i;
    /* type is used to identify the type of the dump received, which is used to
     * keep track of what mode the unit is in */
    METAINDEX type;
    /* nextReq is the next request to be made in a series of dump requests */
    METAINDEX nextReq;

    type = TX81Z_DetectSysexType(midi, midiHdr->lpData);
    /*
     * If the data received doesn't contain a dump header, or the MIDI channel
     * doesn't match the receive channel.
     */
    if (type == -1) {
        /*
         * If a partial dump was received then tx_partialBufferType will be
         * set to the type.
         */
        if (tx_receivedPartialBuffer) {
            type = tx_partialBufferType;
        } else {
            /*
             * The channels didn't match, or garbage was received -- ignore
             * the dump.
             */
            goto AbortDump;
        }
    }
    /*
     * If a dump was received spontaneously.
     */
    if (tx_reqFlags == 0) {
        WORD notification;

        /*
         * Ignore the dump if the unit is in edit mode because if the user
         * does an edit when the unit is in a different mode from the editing
         * mode, the unit changes modes and sends a dump, which undoes the
         * last change made by the user.
         */
        if (tx_curMode == MODE_SINGLE_EDIT || tx_curMode == MODE_PERFORM_EDIT)
        {
            goto AbortDump;
        }
        /*
         * If the user pressed the PLAY/PERFORM button and switched the unit
         * to single mode.
         */
        if (type == META_ACED) {
            nextReq = META_ACED;
            tx_reqFlags = REQ_ACED;
            notification = TXN_RECEIVED_ACED;
        } else if (type == META_VCED) {
            nextReq = META_VCED;
            tx_reqFlags = REQ_VCED;
            notification = TXN_RECEIVED_VCED;
        /*
         * If the user pressed the PLAY/PERFORM button and switched the unit
         * to performance mode.
         */
        } else if (type == META_PCED) {
            nextReq = META_PCED;
            tx_reqFlags = REQ_PCED;
            notification = TXN_RECEIVED_PCED;
        /*
         * Else the user must have hit cancel when requesting a dump, or sent
         * a manual dump from the unit.
         */
        } else {
            goto AbortDump;
        }
        TX81Z_meta[type].recorded = 0;
        PostMessage(Prog_mainWnd, WM_COMMAND
                , MAKEWPARAM(IDD_TX81ZDLG, notification)
                , (LPARAM) tx81zWnd);
        /*
         * Track the mode that the unit is in.
         */
        if (type == META_ACED || type == META_VCED) {
            if (tx_curMode >= MODE_PERFORM || tx_curMode == MODE_UNKNOWN) {
                tx_curMode = MODE_SINGLE;
            }
        } else if (type == META_PCED) {
            if (tx_curMode < MODE_PERFORM) {
                tx_curMode = MODE_PERFORM;
            }
        }
#ifdef SHOW_MODE
        tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
    } else {
        //SetTimer(tx81zWnd, RETRIEVE_TIMER, RETRIEVE_TIMER_DURATION, NULL);
        //tx_rxTimerActive = TRUE;
        for (i = 0; i < META_CNT; i++) {
            nextReq = tx_reqOrder[i];
            if (tx_reqFlags & TX81Z_meta[nextReq].reqFlag) {
                break;
            }
        }
    }
    /*
     * If the next request is not a mode change.
     */
    if (nextReq != META_VOICE_MODE && nextReq != META_PFM_MODE) {
#ifdef _DEBUG
        if (midiHdr->dwBytesRecorded > TX81Z_meta[nextReq].bufSize) {
            MsgBox_F(Prog_tx81zWnd
                    , _T("Dump size does not fit destination buffer!"));
        }
#endif
        /*
         * If the type received was expected, or this is the continuation of
         * a partial reception, or the type is VMEM and a preset bank was
         * requested.
         */
        if ((tx_reqFlags & TX81Z_meta[type].reqFlag)
                || tx_receivedPartialBuffer
                || (type == META_VMEM && (tx_reqFlags & REQ_PRESETS)))
        {
            /*
             * Copy the dump data from the MIDI buffer to the dump reception
             * area of the TX81Z_meta array.
             */
            /*
             * If this is a system dump and the dump length was 44 bytes
             * instead of 45.
             */
            if (type == META_SYS && midiHdr->dwBytesRecorded == 44)
            {
                /*
                 * Convert the 44 byte dump to a regular 45 byte.  Copy the
                 * first (16 + 10) bytes, skip the AT byte, then copy the rest.
                 */
                memcpy(&TX81Z_meta[nextReq].buf[0], midiHdr->lpData, 26);
                TX81Z_meta[nextReq].buf[26] = Prog_snapshot.sys.data[26];
                memcpy(&TX81Z_meta[nextReq].buf[27], &midiHdr->lpData[26], 18);
                /*
                 * Change the recorded bytes to 45 to fool the rest of the
                 * routine.
                 */
                midiHdr->dwBytesRecorded = 45;
                /*
                 * If the TX81Z version is not set to 1.0, set it
                 * automatically.
                 */
                if (Prog_tx81zVersion != 10) {
                    PostNotification(Prog_mainWnd, IDD_TX81ZDLG, tx81zWnd
                            , TXN_VERSION_10_DETECTED);
                }
            } else {
                memcpy(&TX81Z_meta[nextReq].buf[TX81Z_meta[nextReq].recorded]
                        , midiHdr->lpData, midiHdr->dwBytesRecorded);
                /*
                 * If the TX81Z version is set to 1.0 and a 45 byte system
                 * dump was received, set the version to > 1.0.
                 */
                if (Prog_tx81zVersion == 10 && type == META_SYS
                        && midiHdr->dwBytesRecorded == 45)
                {
                    PostNotification(Prog_mainWnd, IDD_TX81ZDLG, tx81zWnd
                            , TXN_VERSION_OTHER_DETECTED);
                }
            }
            TX81Z_meta[nextReq].recorded += midiHdr->dwBytesRecorded;
            /*
             * Update progress bar.
             */
            ProgressBar_SetPos(tx_progressBar, TX81Z_meta[nextReq].recorded);
            /*
             * If the whole dump was received.
             */
            if (TX81Z_meta[nextReq].recorded == TX81Z_meta[nextReq].bufSize) {
                tx_receivedPartialBuffer = FALSE;
                /*
                 * Clear the request flag of the received dump.
                 */
                tx_reqFlags &= ~TX81Z_meta[nextReq].reqFlag;
                /*
                 * Get the unit out of utility mode after requesting
                 * preset banks.
                 */
                if (nextReq == META_PRESET_D) {
                    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
                }
                /*
                 * VCED's always follow ACED's so no need to request it.
                 */
                if (nextReq != META_ACED) {
                    /*
                     * Send the next dump request.
                     */
                    tx_SendNextDumpMsg(midi);
                } else {
                    /*
                     * TODO: Look into why ACED or VCED dump reception always
                     * switches the unit to single mode.
                     */
                    if (tx_curMode == MODE_UNKNOWN
                            || tx_curMode >= MODE_PERFORM)
                    {
                        tx_curMode = MODE_SINGLE;
#ifdef SHOW_MODE
                        tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
                    }
                }
            } else {
                tx_receivedPartialBuffer = TRUE;
                tx_partialBufferType = type;
                SetTimer(tx81zWnd, RETRIEVE_TIMER, RETRIEVE_TIMER_DURATION
                        , NULL);
                tx_rxTimerActive = TRUE;
            }
        }
    /*
     * If the next request is a mode change to voice mode.
     */
    } else if (nextReq == META_VOICE_MODE) {
        /*
         * Check the sysex buffer to see what was just received.
         */
        if (type == META_PCED) {
            if (CDVList_IsEmpty(tx_outMsgList)) {
                /*
                 * PCED dump received -- the machine is in performance mode.
                 * Send another request to switch into single mode.
                 */
                tx_SwitchMode(midi);
            }
        } else if (type == META_ACED) {
            /*
             * First half of voice dump received (ACED) -- wait for the
             * other half.
             */
        } else if (type == META_VCED) {
            /*
             * Voice dump received (ACED & VCED) -- the machine is now in
             * single mode.
             */
            tx_reqFlags &= ~REQ_VOICE_MODE;
            tx_curMode = MODE_SINGLE;
            tx_SendNextDumpMsg(midi);
#ifdef SHOW_MODE
        tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
        }
    } else if (nextReq == META_PFM_MODE) {
        /*
         * Check the buffer to see if it doesn't contain a PCED dump.
         */
        if (type == META_PCED) {
            /*
             * PCED dump received -- the machine is now in performance mode.
             */
            tx_reqFlags &= ~REQ_PFM_MODE;
            tx_curMode = MODE_PERFORM;
            tx_SendNextDumpMsg(midi);
#ifdef SHOW_MODE
        tx_ShowMode();
#endif  /* #ifdef SHOW_MODE */
        } else if (type == META_ACED) {
            /*
             * First half of voice dump received (ACED) - wait for the
             * other half of the dump.
             */
        } else if (type == META_VCED) {
            if (CDVList_IsEmpty(tx_outMsgList)) {
                /*
                 * Voice dump received (ACED & VCED) -- the machine is in
                 * single mode -- send another request to switch to
                 * performance mode.
                 */
                tx_SwitchMode(midi);
            }
        }
    }
AbortDump:
    Midi_BufferDone(midi, midiHdr);
}

/*
 * OnTimer() - Used for message throttling and tries to deal with message
 *             failure.
 */
void tx_OnTimer(HWND tx81zWnd, UINT timerID)
{
    int i;

    switch (timerID) {
        case RETRIEVE_TIMER:
            KillTimer(tx81zWnd, RETRIEVE_TIMER);
            tx_rxTimerActive = FALSE;
            for (i = 0; i < META_CNT; i++) {
                if (tx_reqFlags & TX81Z_meta[tx_reqOrder[i]].reqFlag) {
                    i = tx_reqOrder[i];
                    if (TX81Z_meta[i].recorded < TX81Z_meta[i].bufSize) {
                        TX81Z_meta[i].recorded = 0;
                        ProgressBar_SetPos(tx_progressBar, 0);
                    }
                    break;
                }
            }
            if (i == META_VOICE_MODE || i == META_PFM_MODE) {
                tx_SwitchMode(Prog_midi);
                return;
            }
            /*
             * Displaying a message box here, but letting tx_ShowRxProgressDlg()
             * handle the result.  IDCANCEL will end up cancelling out of both
             * dialogs.
             */
            PostMessage(tx81zWnd, WM_COMMAND
                    , MessageBox(tx81zWnd
                        , _T("Operation timed out requesting data from the synth.")
                        , _T("Timeout"), MB_RETRYCANCEL | MB_ICONINFORMATION)
                    , 0);
            break;
        case MSG_TIMER:
        {
            CDVNODE *txMsg = CDVList_First(tx_outMsgList);

            if (txMsg) {
                Midi_SendLong(Prog_midi, txMsg->data, txMsg->size);
                /*
                 * The effect 2 pan range, level scaling, and performance voice
                 * number seem to take a long time to set, so increase the
                 * message delay for these messages.
                 */
                if ((txMsg->data[4] == 124 && txMsg->data[5] == 6)
                        || (txMsg->data[3] == 0x12 && txMsg->data[4] < 45
                            && txMsg->data[4] % 13 == 5)
                        || (txMsg->data[3] == 0x10 && txMsg->data[4] < 87
                            && txMsg->data[4] % 12 == 1)) {
                    tx_msgDelay = 250;
                } else if (txMsg->size > 4000) {
                    tx_msgDelay = 800;
                } else if (txMsg->size > 2000) {
                    tx_msgDelay = 400;
                } else {
                    tx_msgDelay = MSG_DELAY_DEFAULT;
                }
                /*
                {
                    FILE *fp = fopen("log.txt", "a+");
                    fprintf(fp, "%d\n", txMsg->size);
                    fclose(fp);
                }
                if (txMsg->size < 8) {
                    tx_msgDelay = 50;
                } else if (txMsg->size < 12) {
                    tx_msgDelay = 150;
                } else if (txMsg->size < 16) {
                    tx_msgDelay = 250;
                } else if (txMsg->size < 20) {
                    tx_msgDelay = 200;
                }
                */
                CDVList_Delete(tx_outMsgList); // delete the first node
            }
            if (CDVList_IsEmpty(tx_outMsgList)) {
                KillTimer(tx81zWnd, MSG_TIMER);
            } else {
                SetTimer(tx81zWnd, MSG_TIMER, tx_msgDelay, NULL);
                if (tx_rxTimerActive) {
                    SetTimer(tx81zWnd, RETRIEVE_TIMER, RETRIEVE_TIMER_DURATION
                            , NULL);
                }
            }
            break;
        }
        case TRANSMIT_TIMER:
            tx_transmitTimerFired = TRUE;
            break;
    }
}

/*
 * SendLong() - Sends a sysex message with speed throttling.
 */
void tx_SendLong(MIDI *midi, BYTE *buffer, size_t bufSize)
{
    int diff = GetTickCount() - (tx_lastMsgTime + tx_msgDelay);

    if (!CDVList_IsEmpty(tx_outMsgList) || diff < 0) {
        CDVNODE *txMsg;
        
        CDVList_Head(tx_outMsgList);
        while (txMsg = CDVList_Prev(tx_outMsgList)) {
            /*
             * If a message of this size exists in the list.
             */
            if (txMsg->size == bufSize) {
                /*
                 * If the message is a voice or performance parameter message.
                 */
                if (bufSize == 7 && (buffer[3] != '\x13' || buffer[4] <= 22)) {
                    /*
                     * If the parameter number is the same.
                     */
                    if (txMsg->data[4] == buffer[4]) {
                        /*
                         * Replace the list message.
                         */
                        txMsg->data[5] = buffer[5];
                        return;
                    }
                /*
                 * If the message is a system or effect change message.
                 */
                } else if (bufSize == 8) {
                    /*
                     * If the parameter number is the same.
                     */
                    if (txMsg->data[4] == buffer[4]
                            && txMsg->data[5] == buffer[5])
                    {
                        /*
                         * Replace the list message.
                         */
                        txMsg->data[6] = buffer[6];
                        return;
                    }
                /*
                 * If the message is a micro tuning message.
                 */
                } else if (bufSize == 9
                        && (buffer[4] == '\x7D' || buffer[4] == '\x7E')) {
                    /*
                     * If the key number is the same.
                     */
                    if (txMsg->data[4] == buffer[4]
                            && txMsg->data[5] == buffer[5])
                    {
                        /*
                         * Replace the list message.
                         */
                        txMsg->data[6] = buffer[6];
                        txMsg->data[7] = buffer[7];
                        return;
                    }
                }
            }
        }
        /*
         * Add the message to the message list.
         */
        txMsg = CDVList_Append(tx_outMsgList, bufSize);
        memcpy(txMsg->data, buffer, bufSize);
        /*
         * Set a timer to send the message later.
         */
        if (diff < 0) {
            SetTimer(midi->wnd, MSG_TIMER, -diff, NULL);
        }
    } else {
        Midi_SendLong(midi, buffer, bufSize);
        tx_lastMsgTime = GetTickCount();
        if (bufSize == 4104) {
            tx_lastMsgTime += 1500;
        }
    }
}

/*
 * SendFirstPresetBankRequest() - Sends remote commands to get the synth to
 *                                send preset bank A.
 */
void tx_SendFirstPresetBankRequest(MIDI *midi)
{
    _TUCHAR dlgMsg[80];

    if (tx_retry) {
        TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);
        tx_retry = FALSE;
        return;
    }
    ProgressBar_SetPos(tx_progressBar, 0);
    /*
     * Get the unit into utility mode.
     */
    TX81Z_SendParamChange(midi, TX81Z_SUBGRP_SYS
            , TX81Z_SYS_NOTE_SW, Prog_snapshot.sys.data[TX81Z_SYS_NOTE_SW]);
    /*
     * Navigate to the bank dump menu.
     */
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PARAM_RIGHT, RA_CLICK);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PARAM_RIGHT, RA_CLICK);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_DEC, RA_CLICK);
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);
    /*
     * Update the dialog box.
     */
    _stprintf(dlgMsg, _T("Retrieving %s")
            , TX81Z_meta[META_PRESET_A].dumpName);
    Static_SetText(tx_msgLbl, dlgMsg);
    ProgressBar_SetRange(tx_progressBar, 0
            , TX81Z_meta[META_PRESET_A].bufSize);
    ProgressBar_SetPos(tx_progressBar, 0);
}

/*
 * SendNextDumpMsg() - Sends a request to the synth.
 */
void tx_SendNextDumpMsg(MIDI *midi)
{
    HWND tx81zWnd = midi->wnd;
    int i;

    KillTimer(tx81zWnd, RETRIEVE_TIMER);
    tx_rxTimerActive = FALSE;
    /*
     * Step through each request type in request order.
     */
    for (i = 0; i < META_CNT; i++) {
        int req = tx_reqOrder[i];
        TX81ZMETA *meta = &TX81Z_meta[req];

        /*
         * If the current request type was requested.
         */
        if ((tx_reqFlags & meta->reqFlag) && req != META_ACED) {
            _TUCHAR dlgMsg[80];

            if (req == META_VOICE_MODE) {
                _tcscpy(dlgMsg, _T("Switching the synth to single mode"));
            } else if (req == META_PFM_MODE) {
                _tcscpy(dlgMsg, _T("Switching the synth to performance mode"));
            } else if (req == META_PRESET_A) {
                _tcscpy(dlgMsg, _T("Navigating to the preset bank dump function on the unit"));
            } else {
                _stprintf(dlgMsg, _T("Retrieving %s"), meta->dumpName);
            }
            Static_SetText(tx_msgLbl, dlgMsg);
            ProgressBar_SetRange(tx_progressBar, 0, meta->bufSize);
            ProgressBar_SetPos(tx_progressBar, 0);
            SleepEx(MSG_DELAY_DEFAULT, FALSE);
            if (req <= META_PMEM) {
                tx_SendLong(midi, meta->reqStr, meta->reqStrLen);
            } else if (req == META_VOICE_MODE || req == META_PFM_MODE) {
                tx_SwitchMode(midi);
                //return;
            } else if (req == META_PRESET_A) {
                tx_SendFirstPresetBankRequest(midi);
            } else if (req >= META_PRESET_B && req <= META_PRESET_D) {
                tx_SendNextPresetBankRequest(midi);
            }
            SetTimer(tx81zWnd, RETRIEVE_TIMER, RETRIEVE_TIMER_DURATION, NULL);
            tx_rxTimerActive = TRUE;
            tx_receivedPartialBuffer = FALSE;
            meta->recorded = 0;
            Dialog_DoEvents(tx81zWnd);
            return;
        }
    }
    PostMessage(tx81zWnd, WM_COMMAND, (WPARAM) IDOK, (LPARAM) 0L);
}

/*
 * SendNextPresetBankRequest() - Sends remote control commands to retrieve
 *                               the next preset bank after A.
 */
void tx_SendNextPresetBankRequest(MIDI *midi)
{
    if (!tx_retry) {
        TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_DEC, RA_CLICK);
    }
    TX81Z_SendRemote(midi, TX81Z_REMOTE_DATA_INC, RA_CLICK);
    tx_retry = FALSE;
}

#ifdef SHOW_MODE
void tx_ShowModeAux(void)
{
    static const _TUCHAR *modeNames[7] = {
        _T("MODE_UNKNOWN"),
        _T("MODE_SINGLE"),
        _T("MODE_SINGLE_EDIT"),
        _T("MODE_SINGLE_UTIL"),
        _T("MODE_PERFORM"),
        _T("MODE_PERFORM_EDIT"),
        _T("MODE_PERFORM_UTIL")
    };
#if 0
    FILE *fp = fopen("log.txt", "a+");
    fprintf(fp, "%d - %s\n", tx_lineNum, modeNames[tx_curMode]);
    fclose(fp);
#endif  /* #if 0 */
    SetWindowText(Prog_mainWnd, modeNames[tx_curMode]);
}
#endif  /* #ifdef SHOW_MODE */

/*
 * ShowRxProgressDlg() - Shows the progress dialog and retrieves bulk dumps.
 */
BOOL tx_ShowRxProgressDlg(MIDI *midi, REQUEST reqFlags)
{
    HWND tx81zWnd = midi->wnd;
    size_t i;
    MSG msg;
    BOOL result = TRUE;
    HWND parentWnd = GetActiveWindow();

    /*
     * Adjust window position.
     */
    SetWindowLong(tx81zWnd, GWL_HWNDPARENT, (LONG) parentWnd);
    Window_CenterInParent(tx81zWnd);
    /*
     * Initialize controls.
     */
    ProgressBar_SetStep(tx_progressBar, 1);

    /*
     * Reset the recorded byte counts.
     */
    for (i = 0; i < META_CNT; i++) {
        TX81Z_meta[i].recorded = 0;
    }

    /*
     * Send the first dump request to the synth.
     */
    tx_reqFlags = reqFlags;
    ShowWindow(tx81zWnd, SW_SHOW);
    EnableWindow(parentWnd, FALSE);
    tx_SendNextDumpMsg(midi);

    /*
     * Drive the dialog until all the dumps are retrieved or the user hits
     * the cancel button.
     */
    for (;;) {
        int gmResult = GetMessage(&msg, NULL, 0U, 0U);

        if (gmResult > 0) {
            if (!IsDialogMessage(tx81zWnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else if (gmResult == -1) {
            MsgBox_LastErrorF(tx81zWnd, _T("Error processing message"));
            result = FALSE;
            break;
        } else if (gmResult == 0) {
            PostQuitMessage(0);
            result = FALSE;
            break;
        }
        if (msg.message == WM_COMMAND) {
            WORD cmdID = LOWORD(msg.wParam);

            if (cmdID == IDCANCEL) {
                result = FALSE;
                break;
            } else if (cmdID == IDOK) {
                result = TRUE;
                break;
            } else if (cmdID == IDRETRY) {
                tx_retry = TRUE;
                tx_SendNextDumpMsg(midi);
            }
        }
    }
    /*
     * Just hide the window, don't actually destroy it until the program
     * ends, so it can continue to handle MIDI messages.
     */
    EnableWindow(parentWnd, TRUE);
    ShowWindow(tx81zWnd, SW_HIDE);
    SetWindowLong(tx81zWnd, GWL_HWNDPARENT, (LONG) HWND_DESKTOP);
    return result;
}

/*
 * ShowTxProgressDlg() - Shows the progress dialog and transmits bulk dumps.
 */
BOOL tx_ShowTxProgressDlg(MIDI *midi, REQUEST reqFlags, SNAPSHOT *snapshot)
{
    HWND tx81zWnd = midi->wnd;
    size_t i;
    int dumpCnt = 0;
    int progress = 0;
    MSG msg;
    BOOL result = TRUE;
    HWND parentWnd = GetActiveWindow();
    BYTE *buf = (BYTE *) &snapshot->avced;

    /*
     * Adjust window position.
     */
    SetWindowLong(tx81zWnd, GWL_HWNDPARENT, (LONG) parentWnd);
    Window_CenterInParent(tx81zWnd);
    /*
     * Reset the progress bar and set the range to the total number of dumps.
     */
    ProgressBar_SetStep(tx_progressBar, 1);
    ProgressBar_SetPos(tx_progressBar, 0);
    for (i = 0; i <= META_PMEM; i++) {
        if (reqFlags & (1 << i)) {
            dumpCnt++;
        }
    }
    ProgressBar_SetRange(tx_progressBar, 0, dumpCnt);

    /*
     * Send the first dump to the synth.
     */
    ShowWindow(tx81zWnd, SW_SHOW);
    EnableWindow(parentWnd, FALSE);

    for (i = 0; i <= META_PMEM; i++) {
        /*
         * If the current request type was requested.
         */
        if (reqFlags & TX81Z_meta[i].reqFlag) {
            _TUCHAR dlgMsg[80];

            /*
             * Set the dialog label message.
             */
            _stprintf(dlgMsg, _T("Transmitting %s"), TX81Z_meta[i].dumpName);
            Static_SetText(tx_msgLbl, dlgMsg);
            /*
             * Set the channel in the dump.
             */
            Dialog_DoEvents(tx81zWnd);
            if (i == META_VCED) {
                buf[2] &= 0xF0;
                buf[2] |= midi->outChannel & 0x0F;
                buf[TX81Z_meta[META_VCED].offset + 2] &= 0xF0;
                buf[TX81Z_meta[META_VCED].offset + 2] |= midi->outChannel & 0x0F;
                tx_SendLong(midi, buf, S_ACED_SIZE + S_VCED_SIZE);
            } else {
                buf[TX81Z_meta[i].offset + 2] &= 0xF0;
                buf[TX81Z_meta[i].offset + 2] |= midi->outChannel & 0x0F;
                /*
                 * If the dump is a system dump and version 1.0 tweaks are on,
                 * construct a version 1.0 system dump to send.
                 */
                if (i == META_SYS && Prog_tx81zVersion == 10) {
                    BYTE v10SysBuf[44];
                    BYTE *buf = snapshot->sys.header;

                    /*
                     * Copy the parts around the missing byte into a temporary
                     * buffer and calculate its checksum.
                     */
                    memcpy(v10SysBuf, buf, 26);
                    memcpy(&v10SysBuf[26], &buf[27], 18);
                    v10SysBuf[44 - 2] = Snapshot_Checksum(&v10SysBuf[6], 44-8);
                    /*
                     * Send the 44 byte system dump.
                     */
                    tx_SendLong(midi, v10SysBuf, 44);
                } else {
                    tx_SendLong(midi, buf + TX81Z_meta[i].offset
                            , TX81Z_meta[i].bufSize);
                }
            }
            ProgressBar_SetPos(tx_progressBar, ++progress);

            /*
             * Drive the dialog until all the dumps are transmitted or the
             * user hits the cancel button.
             */
            SetTimer(tx81zWnd, TRANSMIT_TIMER, TRANSMIT_TIMER_DURATION, NULL);
            tx_transmitTimerFired = FALSE;
            while (!tx_transmitTimerFired) {
                int gmResult = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);

                if (gmResult > 0) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                } else if (gmResult == -1) {
                    MsgBox_LastErrorF(tx81zWnd, _T("Error processing message"));
                    result = FALSE;
                    goto AbortTransmission;
                }
                if (msg.message == WM_QUIT) {
                    PostQuitMessage(0);
                    result = FALSE;
                    goto AbortTransmission;
                }
                if (msg.message == WM_COMMAND) {
                    WORD cmdID = LOWORD(msg.wParam);

                    if (cmdID == IDCANCEL) {
                        result = FALSE;
                        goto AbortTransmission;
                    } else if (cmdID == IDOK) {
                        result = TRUE;
                        goto AbortTransmission;
                    }
                }
            }
        }
    }
AbortTransmission:
    KillTimer(tx81zWnd, TRANSMIT_TIMER);
    /*
     * Just hide the window, don't actually destroy it until the program
     * ends, so it can continue to handle MIDI messages.
     */
    EnableWindow(parentWnd, TRUE);
    ShowWindow(tx81zWnd, SW_HIDE);
    SetWindowLong(tx81zWnd, GWL_HWNDPARENT, (LONG) HWND_DESKTOP);
    return result;
}

/*
 * SwitchMode() - Changes the play mode on the synth with remote control
 *                commands.
 */
void tx_SwitchMode(MIDI *midi)
{
    if (tx_curMode == MODE_SINGLE_UTIL || tx_curMode == MODE_PERFORM_UTIL) {
        TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
    }
    TX81Z_SendRemote(midi, TX81Z_REMOTE_PLAY, RA_CLICK);
}

/*
 * UpdateMode() - Returns the new mode based on the current mode and remote
 *                button pressed.
 */
void tx_UpdateMode(BYTE remoteBtn)
{
    static const MODE modeMap[3][7] = {
        /* UTILITY */
        {
            MODE_UNKNOWN,       /* MODE_UNKNOWN */
            MODE_SINGLE_UTIL,   /* MODE_SINGLE */
            MODE_SINGLE_UTIL,   /* MODE_SINGLE_EDIT */
            MODE_SINGLE_UTIL,   /* MODE_SINGLE_UTIL */
            MODE_PERFORM_UTIL,  /* MODE_PERFORM */
            MODE_PERFORM_UTIL,  /* MODE_PERFORM_EDIT */
            MODE_PERFORM_UTIL,  /* MODE_PERFORM_UTIL */
        },
        /* EDIT/COMPARE */
        {
            MODE_UNKNOWN,       /* MODE_UNKNOWN */
            MODE_SINGLE_EDIT,   /* MODE_SINGLE */
            MODE_SINGLE_EDIT,   /* MODE_SINGLE_EDIT */
            MODE_SINGLE_EDIT,   /* MODE_SINGLE_UTIL */
            MODE_PERFORM_EDIT,  /* MODE_PERFORM */
            MODE_PERFORM_EDIT,  /* MODE_PERFORM_EDIT */
            MODE_PERFORM_EDIT,  /* MODE_PERFORM_UTIL */
        },
        /* PLAY/PERFORM */
        {
            MODE_UNKNOWN,       /* MODE_UNKNOWN */
            MODE_PERFORM,       /* MODE_SINGLE */
            MODE_SINGLE,        /* MODE_SINGLE_EDIT */
            MODE_SINGLE,        /* MODE_SINGLE_UTIL */
            MODE_SINGLE,        /* MODE_PERFORM */
            MODE_PERFORM,       /* MODE_PERFORM_EDIT */
            MODE_PERFORM,       /* MODE_PERFORM_UTIL */
        }
    };
    if (remoteBtn < TX81Z_REMOTE_UTILITY || remoteBtn > TX81Z_REMOTE_PLAY) {
        return;
    }
    tx_curMode = modeMap[remoteBtn - TX81Z_REMOTE_UTILITY][tx_curMode];
}

/*
 * UpdatePfmVoices() - Updates voice numbers in the active performance in
 *                     response to an individual program change.
 */
void tx_UpdatePfmVoices(MIDI *midi, int bank, int number)
{
    /*
     * If the unit is in performance mode and the program change
     * mode is set to individual (2) then update the voice numbers
     * in the performance.
     */
    if (tx_curMode >= MODE_PERFORM
            && Prog_snapshot.sys.data[TX81Z_SYS_PROGCH_SW] == 2)
    {
        int i;
        BOOL did = FALSE;
#define INST_DATA_LEN (TX81Z_PCED_INST2_MAX_NOTES - TX81Z_PCED_INST1_MAX_NOTES)
#define CHANNEL_OFFSET (TX81Z_PCED_INST1_CHANNEL - TX81Z_PCED_INST1_MAX_NOTES)
#define MSB_OFFSET (TX81Z_PCED_INST1_VOICE_MSB - TX81Z_PCED_INST1_MAX_NOTES)
#define LSB_OFFSET (TX81Z_PCED_INST1_VOICE_LSB - TX81Z_PCED_INST1_MAX_NOTES)

        /*
         * For each voice.
         */
        for (i = 0; i < 8; i++) {
            int instIdx = TX81Z_PCED_INST1_MAX_NOTES + i * INST_DATA_LEN;
#define maxNotesIdx instIdx
            int channelIdx = instIdx + CHANNEL_OFFSET; 
            /*
             * If the voice's channel is set to the active channel.
             */
            if (Prog_snapshot.pced.data[maxNotesIdx] > 0
                    && Prog_snapshot.pced.data[channelIdx] == midi->outChannel)
            {
                /*
                 * Set the voice number to the program change value.
                 */
                int value = bank * 32 + number;
                int msb = (value & 0x80) >> 7;
                int lsb = value & 0x7F;

                Prog_snapshot.pced.data[instIdx + LSB_OFFSET] = lsb;
                Prog_snapshot.pced.data[instIdx + MSB_OFFSET] = msb;
                did = TRUE;
            }
#undef maxNotesIdx
        }
        if (did && Prog_pfmDlg) {
            SendMessage(Prog_pfmDlg, EDM_REFRESH, 0, 0);
        }
    }
}

/*
 * WndProc()
 */
LRESULT CALLBACK tx_WndProc(HWND tx81zWnd, UINT message, WPARAM wParam
        , LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(tx81zWnd, MDM_ERROR, tx_OnMidiError);
        HANDLE_MSG(tx81zWnd, MDM_SYSEXMSG, tx_OnSysexMsg);
        HANDLE_MSG(tx81zWnd, WM_COMMAND, tx_OnCommand);
        HANDLE_MSG(tx81zWnd, WM_TIMER, tx_OnTimer);
    }
    return DefWindowProc(tx81zWnd, message, wParam, lParam);
}

