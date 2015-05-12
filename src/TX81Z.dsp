# Microsoft Developer Studio Project File - Name="TX81Z" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TX81Z - Win32 ReleaseU
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TX81Z.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TX81Z.mak" CFG="TX81Z - Win32 LogFile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TX81Z - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TX81Z - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "TX81Z - Win32 ReleaseU" (based on "Win32 (x86) Application")
!MESSAGE "TX81Z - Win32 DebugU" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TX81Z - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINDOWS=0x0410 /Yu"stdafx.h" /J /FD /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib winmm.lib comctl32.lib shlwapi.lib htmlhelp.lib shell32.lib /nologo /pdb:none /map /machine:I386

!ELSEIF  "$(CFG)" == "TX81Z - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINDOWS=0x0410 /Yu"stdafx.h" /J /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib winmm.lib comctl32.lib shlwapi.lib htmlhelp.lib shell32.lib /nologo /incremental:no /debug /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TX81Z - Win32 ReleaseU"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseU"
# PROP Intermediate_Dir "ReleaseU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /O2 /Ob2 /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINDOWS=0x0410 /Yu"stdafx.h" /FD /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib winmm.lib comctl32.lib shlwapi.lib htmlhelp.lib shell32.lib /nologo /machine:I386
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "TX81Z - Win32 DebugU"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Z7 /Od /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINDOWS=0x0410 /Yu"stdafx.h" /FD /GZ /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib winmm.lib comctl32.lib shlwapi.lib htmlhelp.lib shell32.lib /nologo /profile /map /debug /machine:I386

!ENDIF 

# Begin Target

# Name "TX81Z - Win32 Release"
# Name "TX81Z - Win32 Debug"
# Name "TX81Z - Win32 ReleaseU"
# Name "TX81Z - Win32 DebugU"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aboutdlg.c
# End Source File
# Begin Source File

SOURCE=.\algctrl.c
# End Source File
# Begin Source File

SOURCE=.\area.c
# End Source File
# Begin Source File

SOURCE=.\bndldlg.c
# End Source File
# Begin Source File

SOURCE=.\cdvlist.c
# End Source File
# Begin Source File

SOURCE=.\copyinstdlg.c
# End Source File
# Begin Source File

SOURCE=.\datadlg.c
# End Source File
# Begin Source File

SOURCE=.\dialog.c
# End Source File
# Begin Source File

SOURCE=.\diffdlg.c
# End Source File
# Begin Source File

SOURCE=.\egctrl.c
# End Source File
# Begin Source File

SOURCE=.\exportdlg.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\filedlg.c
# End Source File
# Begin Source File

SOURCE=.\filepath.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\fxdlg.c
# End Source File
# Begin Source File

SOURCE=.\guierror.c
# End Source File
# Begin Source File

SOURCE=.\importdlg.c
# End Source File
# Begin Source File

SOURCE=.\keynav.c
# End Source File
# Begin Source File

SOURCE=.\kybdctrl.c
# End Source File
# Begin Source File

SOURCE=.\kybddlg.c
# End Source File
# Begin Source File

SOURCE=.\lcdctrl.c
# End Source File
# Begin Source File

SOURCE=.\mainwnd.c
# End Source File
# Begin Source File

SOURCE=.\menubtn.c
# End Source File
# Begin Source File

SOURCE=.\midi.c
# End Source File
# Begin Source File

SOURCE=.\midierror.c
# End Source File
# Begin Source File

SOURCE=.\minifont.c
# End Source File
# Begin Source File

SOURCE=.\msgbox.c
# End Source File
# Begin Source File

SOURCE=.\msgerror.c
# End Source File
# Begin Source File

SOURCE=.\mtfdlg.c
# End Source File
# Begin Source File

SOURCE=.\mtgen.c
# End Source File
# Begin Source File

SOURCE=.\mtodlg.c
# End Source File
# Begin Source File

SOURCE=.\pcdlg.c
# End Source File
# Begin Source File

SOURCE=.\pfmdlg.c
# End Source File
# Begin Source File

SOURCE=.\pfmnav.c
# End Source File
# Begin Source File

SOURCE=.\prog.c
# End Source File
# Begin Source File

SOURCE=.\random.c
# End Source File
# Begin Source File

SOURCE=.\recommentdlg.c
# End Source File
# Begin Source File

SOURCE=.\rect.c
# End Source File
# Begin Source File

SOURCE=.\registry.c
# End Source File
# Begin Source File

SOURCE=.\remotewnd.c
# End Source File
# Begin Source File

SOURCE=.\renamedlg.c
# End Source File
# Begin Source File

SOURCE=.\rpanel.c
# End Source File
# Begin Source File

SOURCE=.\searchdlg.c
# End Source File
# Begin Source File

SOURCE=.\snapshot.c
# End Source File
# Begin Source File

SOURCE=.\stdafx.c
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\storedlg.c
# End Source File
# Begin Source File

SOURCE=.\str.c
# End Source File
# Begin Source File

SOURCE=.\sysdlg.c
# End Source File
# Begin Source File

SOURCE=.\tx81z.c
# End Source File
# Begin Source File

SOURCE=.\Tx81z.rc
# End Source File
# Begin Source File

SOURCE=.\txlbx.c
# End Source File
# Begin Source File

SOURCE=.\txlib.c
# End Source File
# Begin Source File

SOURCE=.\txpack.c
# End Source File
# Begin Source File

SOURCE=.\undo.c
# End Source File
# Begin Source File

SOURCE=.\voicedlg.c
# End Source File
# Begin Source File

SOURCE=.\voicemenu.c
# End Source File
# Begin Source File

SOURCE=.\voicenav.c
# End Source File
# Begin Source File

SOURCE=.\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aboutdlg.h
# End Source File
# Begin Source File

SOURCE=.\algctrl.h
# End Source File
# Begin Source File

SOURCE=.\area.h
# End Source File
# Begin Source File

SOURCE=.\bndldlg.h
# End Source File
# Begin Source File

SOURCE=.\cdvlist.h
# End Source File
# Begin Source File

SOURCE=.\colors.h
# End Source File
# Begin Source File

SOURCE=.\copyinstdlg.h
# End Source File
# Begin Source File

SOURCE=.\datadlg.h
# End Source File
# Begin Source File

SOURCE=.\dialog.h
# End Source File
# Begin Source File

SOURCE=.\diffdlg.h
# End Source File
# Begin Source File

SOURCE=.\egctrl.h
# End Source File
# Begin Source File

SOURCE=.\exportdlg.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\filedlg.h
# End Source File
# Begin Source File

SOURCE=.\filepath.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\func.h
# End Source File
# Begin Source File

SOURCE=.\fxdlg.h
# End Source File
# Begin Source File

SOURCE=.\guierror.h
# End Source File
# Begin Source File

SOURCE=.\importdlg.h
# End Source File
# Begin Source File

SOURCE=.\keynav.h
# End Source File
# Begin Source File

SOURCE=.\kybdctrl.h
# End Source File
# Begin Source File

SOURCE=.\kybddlg.h
# End Source File
# Begin Source File

SOURCE=.\lcdctrl.h
# End Source File
# Begin Source File

SOURCE=.\macros.h
# End Source File
# Begin Source File

SOURCE=.\mainwnd.h
# End Source File
# Begin Source File

SOURCE=.\menubtn.h
# End Source File
# Begin Source File

SOURCE=.\midi.h
# End Source File
# Begin Source File

SOURCE=.\midierror.h
# End Source File
# Begin Source File

SOURCE=.\minifont.h
# End Source File
# Begin Source File

SOURCE=.\msgbox.h
# End Source File
# Begin Source File

SOURCE=.\msgerror.h
# End Source File
# Begin Source File

SOURCE=.\mtfdlg.h
# End Source File
# Begin Source File

SOURCE=.\mtgen.h
# End Source File
# Begin Source File

SOURCE=.\mtodlg.h
# End Source File
# Begin Source File

SOURCE=.\pcdlg.h
# End Source File
# Begin Source File

SOURCE=.\pfmdlg.h
# End Source File
# Begin Source File

SOURCE=.\pfmnav.h
# End Source File
# Begin Source File

SOURCE=.\prog.h
# End Source File
# Begin Source File

SOURCE=.\random.h
# End Source File
# Begin Source File

SOURCE=.\recommentdlg.h
# End Source File
# Begin Source File

SOURCE=.\rect.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\remotewnd.h
# End Source File
# Begin Source File

SOURCE=.\renamedlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rpanel.h
# End Source File
# Begin Source File

SOURCE=.\searchdlg.h
# End Source File
# Begin Source File

SOURCE=.\snapshot.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\storedlg.h
# End Source File
# Begin Source File

SOURCE=.\str.h
# End Source File
# Begin Source File

SOURCE=.\sysdlg.h
# End Source File
# Begin Source File

SOURCE=.\tx81z.h
# End Source File
# Begin Source File

SOURCE=.\txlbx.h
# End Source File
# Begin Source File

SOURCE=.\txlib.h
# End Source File
# Begin Source File

SOURCE=.\txpack.h
# End Source File
# Begin Source File

SOURCE=.\undo.h
# End Source File
# Begin Source File

SOURCE=.\voicedlg.h
# End Source File
# Begin Source File

SOURCE=.\voicemenu.h
# End Source File
# Begin Source File

SOURCE=.\voicenav.h
# End Source File
# Begin Source File

SOURCE=.\window.h
# End Source File
# Begin Source File

SOURCE=.\winx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\disabledrecomment.bmp
# End Source File
# Begin Source File

SOURCE=.\finger.cur
# End Source File
# Begin Source File

SOURCE=.\res\finger.cur
# End Source File
# Begin Source File

SOURCE=.\hand.cur
# End Source File
# Begin Source File

SOURCE=.\res\hand.cur
# End Source File
# Begin Source File

SOURCE=.\left_arr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\link.cur
# End Source File
# Begin Source File

SOURCE=.\res\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\menubtn.bmp
# End Source File
# Begin Source File

SOURCE=.\menubtn.ico
# End Source File
# Begin Source File

SOURCE=.\recomment.bmp
# End Source File
# Begin Source File

SOURCE=.\right_arr.bmp
# End Source File
# Begin Source File

SOURCE=.\split.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitD.cur
# End Source File
# Begin Source File

SOURCE=.\splitD.cur
# End Source File
# Begin Source File

SOURCE=.\splitdown.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitL.cur
# End Source File
# Begin Source File

SOURCE=.\splitL.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitLR.cur
# End Source File
# Begin Source File

SOURCE=.\splitLR.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitR.cur
# End Source File
# Begin Source File

SOURCE=.\splitR.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitU.cur
# End Source File
# Begin Source File

SOURCE=.\splitU.cur
# End Source File
# Begin Source File

SOURCE=.\res\splitUD.cur
# End Source File
# Begin Source File

SOURCE=.\splitUD.cur
# End Source File
# Begin Source File

SOURCE=.\splitup.cur
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z1.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z2.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z3.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z3.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z4.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z4.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z5.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z5.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z6.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z6.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z7.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z7.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z8.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z8.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tx81z9.ico
# End Source File
# Begin Source File

SOURCE=.\Tx81z9.ico
# End Source File
# End Group
# End Target
# End Project
