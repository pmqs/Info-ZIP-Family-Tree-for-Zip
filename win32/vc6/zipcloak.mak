# Microsoft Developer Studio Generated NMAKE File, Based on zipcloak.dsp
!IF "$(CFG)" == ""
CFG=zipcloak - Win32 Debug
!MESSAGE No configuration specified. Defaulting to zipcloak - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "zipcloak - Win32 Release" && "$(CFG)" != "zipcloak - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zipcloak.mak" CFG="zipcloak - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zipcloak - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zipcloak - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "zipcloak - Win32 Release"

OUTDIR=.\zipcloak___Win32_Release
INTDIR=.\zipcloak___Win32_Release
# Begin Custom Macros
OutDir=.\zipcloak___Win32_Release
# End Custom Macros

ALL : "$(OUTDIR)\zipcloak.exe"


CLEAN :
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\zipcloak.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(OUTDIR)\zipcloak.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "UTIL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipcloak.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\zipcloak.pdb" /machine:I386 /out:"$(OUTDIR)\zipcloak.exe" 
LINK32_OBJS= \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipcloak.obj" \
	"$(INTDIR)\zipfile.obj"

"$(OUTDIR)\zipcloak.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zipcloak - Win32 Debug"

OUTDIR=.\zipcloak___Win32_Debug
INTDIR=.\zipcloak___Win32_Debug
# Begin Custom Macros
OutDir=.\zipcloak___Win32_Debug
# End Custom Macros

ALL : "$(OUTDIR)\zipcloak.exe"


CLEAN :
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\zipcloak.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(OUTDIR)\zipcloak.exe"
	-@erase "$(OUTDIR)\zipcloak.ilk"
	-@erase "$(OUTDIR)\zipcloak.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "UTIL" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipcloak.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\zipcloak.pdb" /debug /machine:I386 /out:"$(OUTDIR)\zipcloak.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipcloak.obj" \
	"$(INTDIR)\zipfile.obj"

"$(OUTDIR)\zipcloak.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("zipcloak.dep")
!INCLUDE "zipcloak.dep"
!ELSE 
!MESSAGE Warning: cannot find "zipcloak.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zipcloak - Win32 Release" || "$(CFG)" == "zipcloak - Win32 Debug"
SOURCE=..\..\crctab.c

"$(INTDIR)\crctab.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\crypt.c

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\fileio.c

"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\globals.c

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\ttyio.c

"$(INTDIR)\ttyio.obj" : $(SOURCE) "$(INTDIR)"
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


SOURCE=..\..\zipcloak.c

"$(INTDIR)\zipcloak.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\zipfile.c

"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

