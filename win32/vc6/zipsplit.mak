# Microsoft Developer Studio Generated NMAKE File, Based on zipsplit.dsp
!IF "$(CFG)" == ""
CFG=zipsplit - Win32 Debug
!MESSAGE No configuration specified. Defaulting to zipsplit - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "zipsplit - Win32 Release" && "$(CFG)" != "zipsplit - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zipsplit.mak" CFG="zipsplit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zipsplit - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zipsplit - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "zipsplit - Win32 Release"

OUTDIR=.\zipsplit___Win32_Release
INTDIR=.\zipsplit___Win32_Release
# Begin Custom Macros
OutDir=.\zipsplit___Win32_Release
# End Custom Macros

ALL : "$(OUTDIR)\zipsplit.exe"


CLEAN :
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipsplit.obj"
	-@erase "$(OUTDIR)\zipsplit.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "UTIL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipsplit.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\zipsplit.pdb" /machine:I386 /out:"$(OUTDIR)\zipsplit.exe" 
LINK32_OBJS= \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipsplit.obj"

"$(OUTDIR)\zipsplit.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"

OUTDIR=.\zipsplit___Win32_Debug
INTDIR=.\zipsplit___Win32_Debug
# Begin Custom Macros
OutDir=.\zipsplit___Win32_Debug
# End Custom Macros

ALL : "$(OUTDIR)\zipsplit.exe" "$(OUTDIR)\zipsplit.bsc"


CLEAN :
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\fileio.sbr"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\globals.sbr"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32.sbr"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\win32i64.sbr"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipfile.sbr"
	-@erase "$(INTDIR)\zipsplit.obj"
	-@erase "$(INTDIR)\zipsplit.sbr"
	-@erase "$(OUTDIR)\zipsplit.bsc"
	-@erase "$(OUTDIR)\zipsplit.exe"
	-@erase "$(OUTDIR)\zipsplit.ilk"
	-@erase "$(OUTDIR)\zipsplit.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "UTIL" /D "WIN32" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zipsplit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\fileio.sbr" \
	"$(INTDIR)\globals.sbr" \
	"$(INTDIR)\util.sbr" \
	"$(INTDIR)\win32.sbr" \
	"$(INTDIR)\win32i64.sbr" \
	"$(INTDIR)\zipfile.sbr" \
	"$(INTDIR)\zipsplit.sbr"

"$(OUTDIR)\zipsplit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\zipsplit.pdb" /debug /machine:I386 /out:"$(OUTDIR)\zipsplit.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipsplit.obj"

"$(OUTDIR)\zipsplit.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("zipsplit.dep")
!INCLUDE "zipsplit.dep"
!ELSE 
!MESSAGE Warning: cannot find "zipsplit.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zipsplit - Win32 Release" || "$(CFG)" == "zipsplit - Win32 Debug"
SOURCE=..\..\fileio.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\fileio.obj"	"$(INTDIR)\fileio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\globals.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\globals.obj"	"$(INTDIR)\globals.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\util.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\win32.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\win32.obj"	"$(INTDIR)\win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\win32i64.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\win32i64.obj"	"$(INTDIR)\win32i64.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\zipfile.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\zipfile.obj"	"$(INTDIR)\zipfile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\zipsplit.c

!IF  "$(CFG)" == "zipsplit - Win32 Release"


"$(INTDIR)\zipsplit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zipsplit - Win32 Debug"


"$(INTDIR)\zipsplit.obj"	"$(INTDIR)\zipsplit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

