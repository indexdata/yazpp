# Copyright (C) 1993-2004, Index Data ApS
# All rights reserved.
# $Id: makefile,v 1.5 2004-04-11 17:53:03 adam Exp $

###########################################################
############### Parameters 
###########################################################

DEBUG=0   # 0 for release, 1 for debug

# YAZ
YAZ_DIR=c:\yaz

default: all

all: dirs generated dll zclient yaz

NSIS="c:\program files\nsis\makensis-bz2.exe"

nsis: all
	$(NSIS) yazpp.nsi

nsishelp:
	$(NSIS)

###########################################################
############### Directories
###########################################################
# The current directory is supposed to be something like
# ..../yaz/win, everything is relative to that
ROOTDIR=..   # The home of yaz++

INCLDIR=$(ROOTDIR)\include  # our includes
LIBDIR=$(ROOTDIR)\lib       # We produce .lib, .exp etc there
BINDIR=$(ROOTDIR)\bin       # We produce exes and dlls there
WINDIR=$(ROOTDIR)\win       # all these Win make things
OBJDIR=$(WINDIR)\obj        # where we store intermediate files
SRCDIR=$(ROOTDIR)\src       # for the case we move them under src
PROXYDIR=$(ROOTDIR)\proxy
ZOOMDIR=$(ROOTDIR)\zoom

TMPDIR=$(ROOTDIR)\win\tmp
TMP=$(TMPDIR)

###########################################################
############### Targets - what to make
###########################################################

YAZPP_DLL=$(BINDIR)\yazpp.dll
YAZPP_IMPLIB=$(LIBDIR)\yazpp.lib
ZCLIENT=$(BINDIR)\zclient.exe

# shortcut names defined here
dll: $(YAZPP_DLL)
zclient: $(ZCLIENT)

###########################################################
############### Compiler and linker options 
###########################################################

# YAZ includes & libs
YAZ_LIB="$(YAZ_DIR)\lib\yaz.lib"
YAZ_DEF=/I"$(YAZ_DIR)\include"
yaz: $(BINDIR)\yaz.dll $(BINDIR)\yaz.dll

$(BINDIR)\yaz.dll: $(YAZ_DIR)\bin\yaz.dll
  copy "$(YAZ_DIR)\bin\yaz.dll" $(BINDIR)

COMMON_C_OPTIONS=          \
  /nologo /W3 /GX /FD /c   \
  $(YAZ_DEF)               \
  /D "_WINDOWS"            \
  /D "WIN32"               \
  /D "HAVE_WCHAR_H=1"      \
  /FR"$(OBJDIR)\\"         \
  /Fo"$(OBJDIR)\\"         \
  /Fd"$(OBJDIR)\\" 

COMMON_C_INCLUDES= \
  /I"$(ROOTDIR)\include"

DEBUG_C_OPTIONS=  \
  /D "_DEBUG"      \
  /MDd  /Od /YX /Zi /Gm

RELEASE_C_OPTIONS=  \
  /D "NDEBUG"        \
  /MD /O2

# /W3  = warning level
# /GX  = Enable exception handling
# /FD  = Generate file dependencies (what ever they are)
# /c   = compile without linking
# /FR  = Generate browse info (.sbr file that gets combined into .bsc)
# /Fo  = object file name (or at least path)
# /Fd  = debug database name (or path)
# /MD  = Runtime library: Multithread DLL
# /MDd = Runtime library: Multithread DLL (debug)
# /Od  = Disable optimising (debug)
# /O2  = Optimize for speed
# /YX  = Automatic use of precomipled headers
# /Gm  = Minimal rebuild (some cpp class stuff)
# /Zi  = Program database for debuggers
# /ZI  = Pgm database with special "edit&continue" stuff - not available in C5


### Linker options
LINK=link.exe

LINK_LIBS= kernel32.lib user32.lib   gdi32.lib   \
           advapi32.lib uuid.lib \
           wsock32.lib  advapi32.lib \
           $(YAZ_LIB) $(ICONV_LIB) $(LIBXML2_LIB) $(LIBXSLT_LIB)

COMMON_LNK_OPTIONS= /nologo \
                    /subsystem:windows \
                    /machine:i386 \
			  /incremental:no

DEBUG_LNK_OPTIONS= /debug 

RELEASE_LNK_OPTIONS=

DLL_LINK_OPTIONS= /dll  
CLIENT_LINK_OPTIONS = /subsystem:console  

# Final opt variables
!if $(DEBUG)
COPT=   $(COMMON_C_OPTIONS)   $(DEBUG_C_OPTIONS)     $(COMMON_C_INCLUDES)
MTLOPT= $(COMMON_MTL_OPTIONS) $(DEBUG_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(DEBUG_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(DEBUG_LNK_OPTIONS)   $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)

!else
COPT=   $(COMMON_C_OPTIONS)   $(RELEASE_C_OPTIONS)   $(COMMON_C_INCLUDES) 
MTLOPT= $(COMMON_MTL_OPTIONS) $(RELEASE_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(RELEASE_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(RELEASE_LNK_OPTIONS) $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)
!endif

###########################################################
###############  Source and object modules
###########################################################

YAZPP_DLL_OBJS = \
   "$(OBJDIR)\yaz-socket-manager.obj" \
   "$(OBJDIR)\yaz-pdu-assoc.obj" \
   "$(OBJDIR)\yaz-z-assoc.obj" \
   "$(OBJDIR)\yaz-z-query.obj" \
   "$(OBJDIR)\yaz-ir-assoc.obj" \
   "$(OBJDIR)\yaz-z-server.obj" \
   "$(OBJDIR)\yaz-pdu-assoc-thread.obj" \
   "$(OBJDIR)\yaz-z-server-sr.obj" \
   "$(OBJDIR)\yaz-z-server-ill.obj" \
   "$(OBJDIR)\yaz-z-server-update.obj" \
   "$(OBJDIR)\yaz-z-databases.obj" \
   "$(OBJDIR)\yaz-z-cache.obj" \
   "$(OBJDIR)\yaz-cql2rpn.obj" \
   "$(OBJDIR)\zexcept.obj" \
   "$(OBJDIR)\zconn.obj" \
   "$(OBJDIR)\zquery.obj" \
   "$(OBJDIR)\zrs.obj" \
   "$(OBJDIR)\zrec.obj"

ZCLIENT_OBJS = \
   "$(OBJDIR)\zclient.obj"

###########################################################
############### Compiling 
###########################################################

# Note: This defines where to look for the necessary
# source files. Funny way of doing it, but it works.

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(ZOOMDIR)}.cpp{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

###########################################################
############### Resources
###########################################################

### The RC compiler (resource files)
RSC=rc.exe
COMMON_RC_OPTIONS= /l 0x406 /i"$(ROOTDIR)" 
DEBUG_RC_OPTIONS=/d "_DEBUG"
RELEASE_RC_OPTIONS=/d "NDEBUG"

YAZ_RES=$(OBJDIR)\yaz.res
YAZ_RC=$(WINDIR)\yaz.rc

!if $(DEBUG)
RSOPT=/d_DEBUG
!else
RSOPT=/d_NDEBUG
!endif

$(YAZ_RES): $(YAZ_RC)
	$(RSC) $(RSOPT) /fo"$(YAZ_RES)" $(YAZ_RC) 

###########################################################
############### Linking
###########################################################

$(YAZPP_DLL) $(YAZPP_IMPLIB): "$(BINDIR)" $(YAZPP_DLL_OBJS)
	@echo Linking $(YAZPP_DLL)
	$(LINK) @<<
		$(LNKOPT) 
		$(LINK_LIBS) 
		$(DLL_LINK_OPTIONS)
		$(YAZPP_DLL_OBJS)
		/out:"$(YAZPP_DLL)"
		/implib:"$(YAZPP_IMPLIB)"
		/map:"$(LIBDIR)\yazpp.map"  
<<

$(ZCLIENT) : "$(BINDIR)" $(ZCLIENT_OBJS) $(YAZPP_IMPLIB)
	@echo Linking $(ZCLIENT)
	$(LINK) @<<
        $(LNKOPT) 
		$(CLIENT_LINK_OPTIONS)
		$(LINK_LIBS) 
            $(YAZPP_IMPLIB)
		$(ZCLIENT_OBJS)
		/map:"$(LIBDIR)\zclient.map"
		/out:$(ZCLIENT)
<<


###########################################################
############### Generated Source files
###########################################################

generated: $(ZOOMDIR)\zoom.h

"$(ZOOMDIR)\zoom.h": "$(ZOOMDIR)\master-header"
	sed "s/^*	/	/; s/^*/ /" "$(ZOOMDIR)\master-header" >"$(ZOOMDIR)\zoom.h"

############## clean
clean:
	-del $(BINDIR)\*.exe
	-del $(BINDIR)\*.dll
	-del $(TMPDIR)\*.
	-del $(LIBDIR)\*.MAP
	-del $(LIBDIR)\*.LIB
	-del $(OBJDIR)\*.OBJ

realclean: clean
	-del $(ZOOMDIR)\zoom.h

# Because DOS del will only accept one file name to delete,
# the _H_ files work only on sets that have just one file.
# Z3950_H_FILES had to be spelled out. One more point for MS!

########### check directories and create if needed
dirs: $(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR)

$(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR):
	if not exist "$@/$(NUL)" mkdir "$@"

