# Microsoft Developer Studio Generated NMAKE File, Based on zip.dsp
!IF "$(CFG)" == ""
CFG=zip - Win32 ASM Debug
!MESSAGE No configuration specified. Defaulting to zip - Win32 ASM Debug.
!ENDIF 

!IF "$(CFG)" != "zip - Win32 ASM Release" && "$(CFG)" != "zip - Win32 ASM Debug" && "$(CFG)" != "zip - Win32 Release" && "$(CFG)" != "zip - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zip.mak" CFG="zip - Win32 ASM Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zip - Win32 ASM Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zip - Win32 ASM Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "zip - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zip - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "zip - Win32 ASM Release"

OUTDIR=.\zip___Win32_ASM_Release
INTDIR=.\zip___Win32_ASM_Release
# Begin Custom Macros
OutDir=.\zip___Win32_ASM_Release
# End Custom Macros

ALL : "$(OUTDIR)\zip.exe"


CLEAN :
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\win32zip.obj"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(OUTDIR)\zip.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "ASM_CRC" /D "ASMV" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\zip.pdb" /machine:I386 /out:"$(OUTDIR)\zip.exe" 
LINK32_OBJS= \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\nt.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\win32zip.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj" \
	"$(INTDIR)\crc_i386.obj" \
	"$(INTDIR)\match32.obj"

"$(OUTDIR)\zip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"

OUTDIR=.\zip___Win32_ASM_Debug
INTDIR=.\zip___Win32_ASM_Debug
# Begin Custom Macros
OutDir=.\zip___Win32_ASM_Debug
# End Custom Macros

ALL : "$(OUTDIR)\zip.exe"


CLEAN :
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\win32zip.obj"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(OUTDIR)\zip.exe"
	-@erase "$(OUTDIR)\zip.ilk"
	-@erase "$(OUTDIR)\zip.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "ASM_CRC" /D "ASMV" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\zip.pdb" /debug /machine:I386 /out:"$(OUTDIR)\zip.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\nt.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\win32zip.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj" \
	"$(INTDIR)\crc_i386.obj" \
	"$(INTDIR)\match32.obj"

"$(OUTDIR)\zip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zip - Win32 Release"

OUTDIR=.\zip___Win32_Release
INTDIR=.\zip___Win32_Release
# Begin Custom Macros
OutDir=.\zip___Win32_Release
# End Custom Macros

ALL : "$(OUTDIR)\zip.exe"


CLEAN :
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\win32zip.obj"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(OUTDIR)\zip.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NO_ASM" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\zip.pdb" /machine:I386 /out:"$(OUTDIR)\zip.exe" 
LINK32_OBJS= \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\nt.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\win32zip.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj"

"$(OUTDIR)\zip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zip - Win32 Debug"

OUTDIR=.\zip___Win32_Debug
INTDIR=.\zip___Win32_Debug
# Begin Custom Macros
OutDir=.\zip___Win32_Debug
# End Custom Macros

ALL : "$(OUTDIR)\zip.exe" "$(OUTDIR)\zip.bsc"


CLEAN :
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crc32.sbr"
	-@erase "$(INTDIR)\crctab.obj"
	-@erase "$(INTDIR)\crctab.sbr"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\crypt.sbr"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\deflate.sbr"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\fileio.sbr"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\globals.sbr"
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\nt.sbr"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\trees.sbr"
	-@erase "$(INTDIR)\ttyio.obj"
	-@erase "$(INTDIR)\ttyio.sbr"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32.sbr"
	-@erase "$(INTDIR)\win32i64.obj"
	-@erase "$(INTDIR)\win32i64.sbr"
	-@erase "$(INTDIR)\win32zip.obj"
	-@erase "$(INTDIR)\win32zip.sbr"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zip.sbr"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipfile.sbr"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(INTDIR)\zipup.sbr"
	-@erase "$(OUTDIR)\zip.bsc"
	-@erase "$(OUTDIR)\zip.exe"
	-@erase "$(OUTDIR)\zip.ilk"
	-@erase "$(OUTDIR)\zip.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "NO_ASM" /D "WIN32" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\crc32.sbr" \
	"$(INTDIR)\crctab.sbr" \
	"$(INTDIR)\crypt.sbr" \
	"$(INTDIR)\deflate.sbr" \
	"$(INTDIR)\fileio.sbr" \
	"$(INTDIR)\globals.sbr" \
	"$(INTDIR)\nt.sbr" \
	"$(INTDIR)\trees.sbr" \
	"$(INTDIR)\ttyio.sbr" \
	"$(INTDIR)\util.sbr" \
	"$(INTDIR)\win32.sbr" \
	"$(INTDIR)\win32i64.sbr" \
	"$(INTDIR)\win32zip.sbr" \
	"$(INTDIR)\zip.sbr" \
	"$(INTDIR)\zipfile.sbr" \
	"$(INTDIR)\zipup.sbr"

"$(OUTDIR)\zip.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\zip.pdb" /debug /machine:I386 /out:"$(OUTDIR)\zip.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\crctab.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\nt.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\ttyio.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\win32i64.obj" \
	"$(INTDIR)\win32zip.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj"

"$(OUTDIR)\zip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("zip.dep")
!INCLUDE "zip.dep"
!ELSE 
!MESSAGE Warning: cannot find "zip.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zip - Win32 ASM Release" || "$(CFG)" == "zip - Win32 ASM Debug" || "$(CFG)" == "zip - Win32 Release" || "$(CFG)" == "zip - Win32 Debug"
SOURCE=..\..\crc32.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\crc32.obj"	"$(INTDIR)\crc32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\crctab.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\crctab.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\crctab.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\crctab.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\crctab.obj"	"$(INTDIR)\crctab.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\crypt.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\crypt.obj"	"$(INTDIR)\crypt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\deflate.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\deflate.obj"	"$(INTDIR)\deflate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\fileio.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\fileio.obj"	"$(INTDIR)\fileio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\globals.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\globals.obj"	"$(INTDIR)\globals.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\nt.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\nt.obj"	"$(INTDIR)\nt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\trees.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\trees.obj"	"$(INTDIR)\trees.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\ttyio.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\ttyio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\ttyio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\ttyio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\ttyio.obj"	"$(INTDIR)\ttyio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\util.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\win32.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\win32.obj"	"$(INTDIR)\win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\win32i64.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\win32i64.obj"	"$(INTDIR)\win32i64.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\win32zip.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\win32zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\win32zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\win32zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\win32zip.obj"	"$(INTDIR)\win32zip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\zip.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\zip.obj"	"$(INTDIR)\zip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\zipfile.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\zipfile.obj"	"$(INTDIR)\zipfile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\zipup.c

!IF  "$(CFG)" == "zip - Win32 ASM Release"


"$(INTDIR)\zipup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"


"$(INTDIR)\zipup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Release"


"$(INTDIR)\zipup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip - Win32 Debug"


"$(INTDIR)\zipup.obj"	"$(INTDIR)\zipup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\crc_i386.asm

!IF  "$(CFG)" == "zip - Win32 ASM Release"

IntDir=.\zip___Win32_ASM_Release
InputPath=..\crc_i386.asm
InputName=crc_i386

"$(INTDIR)\crc_i386.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	ml /nologo /c /Cx /coff /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"
<< 
	

!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"

IntDir=.\zip___Win32_ASM_Debug
InputPath=..\crc_i386.asm
InputName=crc_i386

"$(INTDIR)\crc_i386.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	ml /nologo /c /Cx /coff /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"
<< 
	

!ELSEIF  "$(CFG)" == "zip - Win32 Release"

!ELSEIF  "$(CFG)" == "zip - Win32 Debug"

!ENDIF 

SOURCE=..\match32.asm

!IF  "$(CFG)" == "zip - Win32 ASM Release"

IntDir=.\zip___Win32_ASM_Release
InputPath=..\match32.asm
InputName=match32

"$(INTDIR)\match32.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	ml /nologo /c /Cx /coff /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"
<< 
	

!ELSEIF  "$(CFG)" == "zip - Win32 ASM Debug"

IntDir=.\zip___Win32_ASM_Debug
InputPath=..\match32.asm
InputName=match32

"$(INTDIR)\match32.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	ml /nologo /c /Cx /coff /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"
<< 
	

!ELSEIF  "$(CFG)" == "zip - Win32 Release"

!ELSEIF  "$(CFG)" == "zip - Win32 Debug"

!ENDIF 


!ENDIF 

