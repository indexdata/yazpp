# Microsoft Developer Studio Project File - Name="yazserver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=yazserver - Win32 Debug URSULA
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "yazserver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yazserver.mak" CFG="yazserver - Win32 Debug URSULA"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yazserver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yazserver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "yazserver - Win32 Debug URSULA" (based on "Win32 (x86) Console Application")
!MESSAGE "yazserver - Win32 Release URSULA" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yazserver - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "NDEBUG" /D HAVE_YAZ_URSULA_H=0 /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/yazmyserver.exe" /libpath:"../yazxx/release" /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\Release
ProjDir=.
TargetName=yazmyserver
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "yazserver - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D HAVE_YAZ_URSULA_H=0 /YX /FD /c
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/yazmyserver.exe" /pdbtype:sept /libpath:"../yazxx/debug" /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\Debug
ProjDir=.
TargetName=yazmyserver
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "yazserver - Win32 Debug URSULA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "yazserver___Win32_Debug_URSULA"
# PROP BASE Intermediate_Dir "yazserver___Win32_Debug_URSULA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "yazserver___Win32_Debug_URSULA"
# PROP Intermediate_Dir "yazserver___Win32_Debug_URSULA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D HAVE_YAZ_URSULA_H=1 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D HAVE_YAZ_URSULA_H=1 /YX /FD /c
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/yazmyserver.exe" /pdbtype:sept /libpath:"../yazxx/debug" /libpath:"../../../yaz/lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/yazmyserver.exe" /pdbtype:sept /libpath:"../yazxx/debug" /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\yazserver___Win32_Debug_URSULA
ProjDir=.
TargetName=yazmyserver
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "yazserver - Win32 Release URSULA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "yazserver___Win32_Release_URSULA"
# PROP BASE Intermediate_Dir "yazserver___Win32_Release_URSULA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "yazserver___Win32_Release_URSULA"
# PROP Intermediate_Dir "yazserver___Win32_Release_URSULA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "NDEBUG" /D HAVE_YAZ_URSULA=1 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /I "../../../yaz/include" /I "../../../yaz-ursula/include" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "NDEBUG" /D HAVE_YAZ_URSULA_H=1 /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/yazmyserver.exe" /libpath:"../yazxx/release" /libpath:"../../../yaz/lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib yazxx.lib yaz.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/yazmyserver.exe" /libpath:"../yazxx/release" /libpath:"../../../yaz/lib"
# Begin Special Build Tool
OutDir=.\yazserver___Win32_Release_URSULA
ProjDir=.
TargetName=yazmyserver
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe $(ProjDir)\..\..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "yazserver - Win32 Release"
# Name "yazserver - Win32 Debug"
# Name "yazserver - Win32 Debug URSULA"
# Name "yazserver - Win32 Release URSULA"
# Begin Source File

SOURCE="..\..\src\yaz-marc-sample.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\src\yaz-my-server.cpp"
# End Source File
# End Target
# End Project
