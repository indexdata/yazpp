# Microsoft Developer Studio Project File - Name="yazxx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=yazxx - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "yazxx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yazxx.mak" CFG="yazxx - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yazxx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "yazxx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yazxx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../include" /I "../../../yaz/include" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yaz.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\Release
ProjDir=.
TargetName=yazxx
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).dll $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "yazxx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../yaz++/include" /I "../../include" /I "../../../yaz/include" /I "../../../yaz/include/yaz" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yaz.lib wsock32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\Debug
ProjDir=.
TargetName=yazxx
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).dll $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "yazxx - Win32 Release"
# Name "yazxx - Win32 Debug"
# Begin Source File

SOURCE="..\..\src\yaz-ir-assoc.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-ir-assoc.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-pdu-assoc-thread.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-pdu-assoc.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-pdu-assoc.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-pdu-observer.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-proxy.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-proxy.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-socket-manager.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-socket-manager.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-socket-observer.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-assoc.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-z-assoc.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-cache.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-databases.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-z-databases.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-query.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-z-query.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-server-ill.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-server-sr.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-server-update.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-z-server.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\include\yaz++\yaz-z-server.h"
# End Source File
# Begin Source File

SOURCE=..\..\zoom\zconn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\zoom\zexcept.cpp
# End Source File
# Begin Source File

SOURCE=..\..\zoom\zquery.cpp
# End Source File
# Begin Source File

SOURCE=..\..\zoom\zrec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\zoom\zrs.cpp
# End Source File
# End Target
# End Project
