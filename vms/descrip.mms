#                                               26 July 2005.  SMS.
#
#    Zip 3.0 for VMS - MMS (or MMK) Description File.
#
# Usage:
#
#    MMS /DESCRIP = [.VMS]DESCRIP.MMS [/MACRO = (<see_below>)] [target]
#
# Note that this description file must be used from the main
# distribution directory, not from the [.VMS] subdirectory.
#
# Optional macros:
#
#    CCOPTS=xxx     Compile with CC options xxx.  For example:
#                   CCOPTS=/ARCH=HOST
#
#    DBG=1          Compile with /DEBUG /NOOPTIMIZE.
#                   Link with /DEBUG /TRACEBACK.
#                   (Default is /NOTRACEBACK.)
#
#    IM=1           Use the old "IM" scheme for storing VMS/RMS file
#                   atributes, instead of the newer "PK" scheme.
#
#    LARGE=1        Enable large-file (>2GB) support.  Non-VAX only.
#
#    LINKOPTS=xxx   Link with LINK options xxx.  For example:
#                   LINKOPTS=/NOINFO   
#
#    LIST=1         Compile with /LIST /SHOW = (ALL, NOMESSAGES).
#                   Link with /MAP /CROSS_REFERENCE /FULL.
#
#    "LOCAL_ZIP=c_macro_1=value1 [, c_macro_2=value2 [...]]"
#                   Compile with these additional C macros defined.
#
# VAX-specific optional macros:
#
#    VAXC=1         Use the VAX C compiler, assuming "CC" runs it.
#                   (That is, DEC C is not installed, or else DEC C is
#                   installed, but VAX C is the default.)
#
#    FORCE_VAXC=1   Use the VAX C compiler, assuming "CC /VAXC" runs it.
#                   (That is, DEC C is installed, and it is the
#                   default, but you want VAX C anyway, you fool.)
#
#    GNUC=1         Use the GNU C compiler.  (Seriously under-tested.)
#
#
# The default target, ALL, builds the selected product executables and
# help files.
#
# Other targets:
#
#    CLEAN      deletes architecture-specific files, but leaves any
#               individual source dependency files and the help files.
#
#    CLEAN_ALL  deletes all generated files, except the main (collected)
#               source dependency file.
#
#    CLEAN_EXE  deletes only the architecture-specific executables. 
#               Handy if all you wish to do is re-link the executables.
#
# Example commands:
#
# To build the conventional small-file product using the DEC/Compaq/HP C
# compiler (Note: DESCRIP.MMS is the default description file name.):
#
#    MMS /DESCRIP = [.VMS]
#
# To get the large-file executables (on a non-VAX system):
#
#    MMS /DESCRIP = [.VMS] /MACRO = (LARGE=1)
#
# To delete the architecture-specific generated files for this system
# type:
#
#    MMS /DESCRIP = [.VMS] /MACRO = (LARGE=1) CLEAN     ! Large-file.
# or
#    MMS /DESCRIP = [.VMS] CLEAN                        ! Small-file.
#
# To build a complete small-file product for debug with compiler
# listings and link maps:
#
#    MMS /DESCRIP = [.VMS] CLEAN
#    MMS /DESCRIP = [.VMS] /MACRO = (DBG=1, LIST=1)
#
########################################################################

# Include primary product description file.

INCL_DESCRIP_SRC = 1
.INCLUDE [.vms]descrip_src.mms

# Object library names.

LIB_ZIP = [.$(DEST)]zip.olb
LIB_ZIPCLI = [.$(DEST)]zipcli.olb
LIB_ZIPUTILS = [.$(DEST)]ziputils.olb

# Help file names.

ZIP_HELP = zip.hlp zip_cli.hlp


# TARGETS.

# Default target, ALL.  Build All Zip executables, utility executables,
# and help files.

ALL : $(ZIP) $(ZIP_CLI) $(ZIPUTILS) $(ZIP_HELP)
	@ write sys$output "Done."

# CLEAN target.  Delete the [.$(DEST)] directory and everything in it.

CLEAN :
	if (f$search( "[.$(DEST)]*.*") .nes. "") then -
	 delete /noconfirm [.$(DEST)]*.*;*
	if (f$search( "$(DEST).dir") .nes. "") then -
	 set protection = w:d $(DEST).dir;*
	if (f$search( "$(DEST).dir") .nes. "") then -
	 delete /noconfirm $(DEST).dir;*

# CLEAN_ALL target.  Delete:
#    The [.$(DEST)] directories and everything in them.
#    All help-related derived files,
#    All individual C dependency files.
# Also mention:
#    Comprehensive dependency file.

CLEAN_ALL :
	if (f$search( "[.ALPHA*]*.*") .nes. "") then -
	 delete /noconfirm [.ALPHA*]*.*;*
	if (f$search( "ALPHA*.dir") .nes. "") then -
	 set protection = w:d ALPHA*.dir;*
	if (f$search( "ALPHA*.dir") .nes. "") then -
	 delete /noconfirm ALPHA*.dir;*
	if (f$search( "[.IA64*]*.*") .nes. "") then -
	 delete /noconfirm [.IA64*]*.*;*
	if (f$search( "IA64*.dir") .nes. "") then -
	 set protection = w:d IA64*.dir;*
	if (f$search( "IA64*.dir") .nes. "") then -
	 delete /noconfirm IA64*.dir;*
	if (f$search( "[.VAX*]*.*") .nes. "") then -
	 delete /noconfirm [.VAX*]*.*;*
	if (f$search( "VAX*.dir") .nes. "") then -
	 set protection = w:d VAX*.dir;*
	if (f$search( "VAX*.dir") .nes. "") then -
	 delete /noconfirm VAX*.dir;*
	if (f$search( "[.vms]ZIP_CLI.RNH") .nes. "") then -
	 delete /noconfirm [.vms]ZIP_CLI.RNH;*
	if (f$search( "ZIP_CLI.HLP") .nes. "") then -
	 delete /noconfirm ZIP_CLI.HLP;*
	if (f$search( "ZIP.HLP") .nes. "") then -
	 delete /noconfirm ZIP.HLP;*
	if (f$search( "*.MMSD") .nes. "") then -
	 delete /noconfirm *.MMSD;*
	if (f$search( "[.vms]*.MMSD") .nes. "") then -
	 delete /noconfirm [.vms]*.MMSD;*
	@ write sys$output ""
	@ write sys$output "Note:  This procedure will not"
	@ write sys$output "   DELETE [.VMS]DESCRIP_DEPS.MMS;*"
	@ write sys$output -
 "You may choose to, but a recent version of MMS (V3.5 or newer?) is"
	@ write sys$output -
 "needed to regenerate it.  (It may also be recovered from the original"
	@ write sys$output -
 "distribution kit.)  See [.VMS]DESCRIP_MKDEPS.MMS for instructions on"
	@ write sys$output -
 "generating [.VMS]DESCRIP_DEPS.MMS."
	@ write sys$output ""

# CLEAN_EXE target.  Delete the executables in [.$(DEST)].

CLEAN_EXE :
	if (f$search( "[.$(DEST)]*.exe") .nes. "") then -
	 delete /noconfirm [.$(DEST)]*.exe;*


# Object library module dependencies.

$(LIB_ZIP) : $(LIB_ZIP)($(MODS_OBJS_LIB_ZIP))
	@ write sys$output "$(MMS$TARGET) updated."

$(LIB_ZIPCLI) : $(LIB_ZIPCLI)($(MODS_OBJS_LIB_ZIPCLI))
	@ write sys$output "$(MMS$TARGET) updated."

$(LIB_ZIPUTILS) : $(LIB_ZIPUTILS)($(MODS_OBJS_LIB_ZIPUTILS))
	@ write sys$output "$(MMS$TARGET) updated."

# Module ID options file.

OPT_ID = [.vms]zip.opt

# Default C compile rule.

.C.OBJ :
	$(CC) $(CFLAGS) $(CDEFS_UNX) $(MMS$SOURCE)


# Normal sources in [.VMS].

[.$(DEST)]vms.obj : [.vms]vms.c
[.$(DEST)]vmsmunch.obj : [.vms]vmsmunch.c
[.$(DEST)]vmszip.obj : [.vms]vmszip.c

# Command-line interface files.

[.$(DEST)]cmdline.obj : [.vms]cmdline.c
	$(CC) $(CFLAGS) $(CDEFS_CLI) $(MMS$SOURCE)

[.$(DEST)]zipcli.obj : zip.c
	$(CC) $(CFLAGS) $(CDEFS_CLI) $(MMS$SOURCE)

[.$(DEST)]zip_cli.obj : [.vms]zip_cli.cld

# Utility variant sources.

[.$(DEST)]crypt_.obj : crypt.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]fileio_.obj : fileio.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]util_.obj : util.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]zipfile_.obj : zipfile.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]vms_.obj : [.vms]vms.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

# Utility main sources.

[.$(DEST)]zipcloak.obj : zipcloak.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]zipnote.obj : zipnote.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

[.$(DEST)]zipsplit.obj : zipsplit.c
	$(CC) $(CFLAGS) $(CDEFS_UTIL) $(MMS$SOURCE)

# VAX C LINK options file.

.IFDEF OPT_FILE
$(OPT_FILE) :
        open /write opt_file_ln  $(OPT_FILE)
        write opt_file_ln "SYS$SHARE:VAXCRTL.EXE /SHARE"
        close opt_file_ln
.ENDIF

# Normal Zip executable.

$(ZIP) : [.$(DEST)]zip.obj $(LIB_ZIP) $(OPT_FILE)
	$(LINK) $(LINKFLAGS) $(MMS$SOURCE), -
	 $(LIB_ZIP) /include = (GLOBALS) /library,  -
	 $(LFLAGS_ARCH) -
	 $(OPT_ID) /options

# CLI Zip executable.

$(ZIP_CLI) : [.$(DEST)]zipcli.obj \
             $(LIB_ZIPCLI) $(OPT_ID) $(OPT_FILE)
	$(LINK) $(LINKFLAGS) $(MMS$SOURCE), -
	 $(LIB_ZIPCLI) /library, -
	 $(LIB_ZIP) /include = (GLOBALS) /library, -
	 $(LFLAGS_ARCH) -
	 $(OPT_ID) /options

# Utility executables.

[.$(DEST)]zipcloak.exe : [.$(DEST)]zipcloak.obj \
                         $(LIB_ZIPUTILS) $(LIB_ZIP) \
                         $(OPT_ID) $(OPT_FILE)
	$(LINK) $(LINKFLAGS) $(MMS$SOURCE), -
	 $(LIB_ZIPUTILS) /include = (GLOBALS) /library, -
	 $(LFLAGS_ARCH) -
	 $(OPT_ID) /options

[.$(DEST)]zipnote.exe : [.$(DEST)]zipnote.obj \
                         $(LIB_ZIPUTILS) $(LIB_ZIP) \
                         $(OPT_ID) $(OPT_FILE)
	$(LINK) $(LINKFLAGS) $(MMS$SOURCE), -
	 $(LIB_ZIPUTILS) /include = (GLOBALS) /library, -
	 $(LFLAGS_ARCH) -
	 $(OPT_ID) /options

[.$(DEST)]zipsplit.exe : [.$(DEST)]zipsplit.obj \
                         $(LIB_ZIPUTILS) $(LIB_ZIP) \
                         $(OPT_ID) $(OPT_FILE)
	$(LINK) $(LINKFLAGS) $(MMS$SOURCE), -
	 $(LIB_ZIPUTILS) /include = (GLOBALS) /library, -
	 $(LFLAGS_ARCH) -
	 $(OPT_ID) /options

# Help files.

zip.hlp : [.vms]vms_zip.rnh
	runoff /output = $(MMS$TARGET) $(MMS$SOURCE)

zip_cli.hlp : [.vms]zip_cli.help [.vms]cvthelp.tpu
	edit := edit
	edit /tpu /nosection /nodisplay /command = [.vms]cvthelp.tpu -
	 $(MMS$SOURCE)
	rename /noconfirm zip_cli.rnh; [.vms];
	purge /noconfirm /nolog /keep = 1 [.vms]zip_cli.rnh
	runoff /output = $(MMS$TARGET) [.vms]zip_cli.rnh

# Include generated source dependencies.

INCL_DESCRIP_DEPS = 1
.INCLUDE [.vms]descrip_deps.mms

