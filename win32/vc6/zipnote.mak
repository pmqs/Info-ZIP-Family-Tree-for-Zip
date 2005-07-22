# Microsoft Developer Studio Generated NMAKE File, Based on zipnote.dsp
!IF "$(CFG)" == ""
CFG=zipnote - Win32 Debug
!MESSAGE No configuration specified. Defaulting to zipnote - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "zipnote - Win32 Release" && "$(CFG)" != "zipnote - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zipnote.mak" CFG="zipnote - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zipnote - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zipnote - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zipnote - Win32 Release"

OUTDIR=.\zipnote___Win32_Release
INTDIR=.\zipnote___Win32_Release
# Begin Custom Macros
OutDir=.\zipnote___Win32_Release
# End Custom Macros

ALL : "$(OUTDIR)\zipnote.exe"


CLEAN :
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipnote.obj"
	-@erase "$(OUTDIR)\zipnote.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "UTIL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipnote.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\zipnote.pdb" /machine:I386 /out:"$(OUTDIR)\zipnote.exe" 
LINK32_OBJS= \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipnote.obj"

"$(OUTDIR)\zipnote.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zipnote - Win32 Debug"

OUTDIR=.\zipnote___Win32_Debug
INTDIR=.\zipnote___Win32_Debug
# Begin Custom Macros
OutDir=.\zipnote___Win32_Debug
# End Custom Macros

ALL : "$(OUTDIR)\zipnote.exe"


CLEAN :
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipnote.obj"
	-@erase "$(OUTDIR)\zipnote.exe"
	-@erase "$(OUTDIR)\zipnote.ilk"
	-@erase "$(OUTDIR)\zipnote.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "UTIL" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipnote.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\zipnote.pdb" /debug /machine:I386 /out:"$(OUTDIR)\zipnote.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipnote.obj"

"$(OUTDIR)\zipnote.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("zipnote.dep")
!INCLUDE "zipnote.dep"
!ELSE 
!MESSAGE Warning: cannot find "zipnote.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zipnote - Win32 Release" || "$(CFG)" == "zipnote - Win32 Debug"
SOURCE=..\..\fileio.c

"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\globals.c

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\win32i64.c

"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\zipfile.c

"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\zipnote.c

"$(INTDIR)\zipnote.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

