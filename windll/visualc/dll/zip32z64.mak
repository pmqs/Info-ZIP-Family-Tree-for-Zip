# Microsoft Developer Studio Generated NMAKE File, Based on zip32z64.dsp
!IF "$(CFG)" == ""
CFG=zip32z64 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to zip32z64 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "zip32z64 - Win32 Release" && "$(CFG)" != "zip32z64 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zip32z64.mak" CFG="zip32z64 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zip32z64 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zip32z64 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "zip32z64 - Win32 Release"

OUTDIR=.\..\Release\app
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\..\Release\app
# End Custom Macros

ALL : "$(OUTDIR)\zip32z64.dll"


CLEAN :
	-@erase "$(INTDIR)\api.obj"
	-@erase "$(INTDIR)\crc32.obj"
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
	-@erase "$(INTDIR)\windll.obj"
	-@erase "$(INTDIR)\windll.res"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(OUTDIR)\zip32z64.dll"
	-@erase "$(OUTDIR)\zip32z64.exp"
	-@erase "$(OUTDIR)\zip32z64.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Zp4 /MT /W3 /GX /O2 /I "..\..\.." /I "..\..\..\WINDLL" /I "..\..\..\WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "NO_ASM" /D "WINDLL" /D "MSDOS" /D "USE_ZIPMAIN" /Fp"$(INTDIR)\zip32z64.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\windll.res" /d "NDEBUG" /d "WIN32" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip32z64.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\zip32z64.pdb" /machine:I386 /def:"..\..\windll32.def" /out:"$(OUTDIR)\zip32z64.dll" /implib:"$(OUTDIR)\zip32z64.lib" 
DEF_FILE= \
	"..\..\windll32.def"
LINK32_OBJS= \
	"$(INTDIR)\api.obj" \
	"$(INTDIR)\crc32.obj" \
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
	"$(INTDIR)\windll.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj" \
	"$(INTDIR)\windll.res"

"$(OUTDIR)\zip32z64.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"

OUTDIR=.\..\Debug\app
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\Debug\app
# End Custom Macros

ALL : "$(OUTDIR)\zip32z64.dll" "$(OUTDIR)\zip32z64.bsc"


CLEAN :
	-@erase "$(INTDIR)\api.obj"
	-@erase "$(INTDIR)\api.sbr"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crc32.sbr"
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
	-@erase "$(INTDIR)\windll.obj"
	-@erase "$(INTDIR)\windll.res"
	-@erase "$(INTDIR)\windll.sbr"
	-@erase "$(INTDIR)\zip.obj"
	-@erase "$(INTDIR)\zip.sbr"
	-@erase "$(INTDIR)\zipfile.obj"
	-@erase "$(INTDIR)\zipfile.sbr"
	-@erase "$(INTDIR)\zipup.obj"
	-@erase "$(INTDIR)\zipup.sbr"
	-@erase "$(OUTDIR)\zip32z64.bsc"
	-@erase "$(OUTDIR)\zip32z64.dll"
	-@erase "$(OUTDIR)\zip32z64.exp"
	-@erase "$(OUTDIR)\zip32z64.ilk"
	-@erase "$(OUTDIR)\zip32z64.lib"
	-@erase "$(OUTDIR)\zip32z64.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\ZIP" /I "..\..\..\WINDLL" /I "..\..\..\WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "NO_ASM" /D "WINDLL" /D "MSDOS" /D "USE_ZIPMAIN" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\zip32z64.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\windll.res" /d "_DEBUG" /d "WIN32" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zip32z64.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\api.sbr" \
	"$(INTDIR)\crc32.sbr" \
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
	"$(INTDIR)\windll.sbr" \
	"$(INTDIR)\zip.sbr" \
	"$(INTDIR)\zipfile.sbr" \
	"$(INTDIR)\zipup.sbr"

"$(OUTDIR)\zip32z64.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\zip32z64.pdb" /debug /machine:I386 /def:"..\..\windll32.def" /out:"$(OUTDIR)\zip32z64.dll" /implib:"$(OUTDIR)\zip32z64.lib" /pdbtype:sept 
DEF_FILE= \
	"..\..\windll32.def"
LINK32_OBJS= \
	"$(INTDIR)\api.obj" \
	"$(INTDIR)\crc32.obj" \
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
	"$(INTDIR)\windll.obj" \
	"$(INTDIR)\zip.obj" \
	"$(INTDIR)\zipfile.obj" \
	"$(INTDIR)\zipup.obj" \
	"$(INTDIR)\windll.res"

"$(OUTDIR)\zip32z64.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("zip32z64.dep")
!INCLUDE "zip32z64.dep"
!ELSE 
!MESSAGE Warning: cannot find "zip32z64.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zip32z64 - Win32 Release" || "$(CFG)" == "zip32z64 - Win32 Debug"
SOURCE=..\..\..\api.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\api.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\api.obj"	"$(INTDIR)\api.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\crc32.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\crc32.obj"	"$(INTDIR)\crc32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\crypt.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\crypt.obj"	"$(INTDIR)\crypt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\deflate.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\deflate.obj"	"$(INTDIR)\deflate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\fileio.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\fileio.obj"	"$(INTDIR)\fileio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\globals.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\globals.obj"	"$(INTDIR)\globals.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\win32\nt.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\nt.obj"	"$(INTDIR)\nt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\trees.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\trees.obj"	"$(INTDIR)\trees.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\ttyio.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\ttyio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\ttyio.obj"	"$(INTDIR)\ttyio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\util.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\win32\win32.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\win32.obj"	"$(INTDIR)\win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\win32\win32i64.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\win32i64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\win32i64.obj"	"$(INTDIR)\win32i64.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\win32\win32zip.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\win32zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\win32zip.obj"	"$(INTDIR)\win32zip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\windll\windll.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\windll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\windll.obj"	"$(INTDIR)\windll.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\windll\windll.rc

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\windll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\windll.res" /i "\aZip3\Zip3\windll" /d "NDEBUG" /d "WIN32" $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\windll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\windll.res" /i "\aZip3\Zip3\windll" /d "_DEBUG" /d "WIN32" $(SOURCE)


!ENDIF 

SOURCE=..\..\..\zip.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\zip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\zip.obj"	"$(INTDIR)\zip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\zipfile.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\zipfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\zipfile.obj"	"$(INTDIR)\zipfile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\zipup.c

!IF  "$(CFG)" == "zip32z64 - Win32 Release"


"$(INTDIR)\zipup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "zip32z64 - Win32 Debug"


"$(INTDIR)\zipup.obj"	"$(INTDIR)\zipup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

