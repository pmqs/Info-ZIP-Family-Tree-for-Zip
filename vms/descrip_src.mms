#                                               13 December 2004.  SMS.
#
#    Zip 3.0 for VMS - MMS (or MMK) Source Description File.
#

# This description file is included by other description files.  It is
# not intended to be used alone.  Verify proper inclusion.

.IFDEF INCL_DESCRIP_SRC
.ELSE
$$$$ THIS DESCRIPTION FILE IS NOT INTENDED TO BE USED THIS WAY.
.ENDIF


# Define MMK architecture macros when using MMS.

.IFDEF __MMK__                  # __MMK__
.ELSE                           # __MMK__
ALPHA_X_ALPHA = 1
IA64_X_IA64 = 1
VAX_X_VAX = 1
.IFDEF $(MMS$ARCH_NAME)_X_ALPHA     # $(MMS$ARCH_NAME)_X_ALPHA
__ALPHA__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_ALPHA
.IFDEF $(MMS$ARCH_NAME)_X_IA64      # $(MMS$ARCH_NAME)_X_IA64
__IA64__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_IA64
.IFDEF $(MMS$ARCH_NAME)_X_VAX       # $(MMS$ARCH_NAME)_X_VAX
__VAX__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_VAX
.ENDIF                          # __MMK__

# Combine command-line VAX C compiler macros.

.IFDEF VAXC                     # VAXC
VAXC_OR_FORCE_VAXC = 1
.ELSE                           # VAXC
.IFDEF FORCE_VAXC                   # FORCE_VAXC
VAXC_OR_FORCE_VAXC = 1
.ENDIF                              # FORCE_VAXC
.ENDIF                          # VAXC

# Analyze architecture-related and option macros.

.IFDEF __ALPHA__                # __ALPHA__
DECC = 1
DESTM = ALPHA
.ELSE                           # __ALPHA__
.IFDEF __IA64__                     # __IA64__
DECC = 1
DESTM = IA64
.ELSE                               # __IA64__
.IFDEF __VAX__                          # __VAX__
.IFDEF VAXC_OR_FORCE_VAXC                   # VAXC_OR_FORCE_VAXC
DESTM = VAXV
.ELSE                                       # VAXC_OR_FORCE_VAXC
.IFDEF GNUC                                     # GNUC
CC = GCC
DESTM = VAXG
.ELSE                                           # GNUC
DECC = 1
DESTM = VAX
.ENDIF                                          # GNUC
.ENDIF                                      # VAXC_OR_FORCE_VAXC
.ELSE                                   # __VAX__
DESTM = UNK
UNK_DEST = 1
.ENDIF                                  # __VAX__
.ENDIF                              # __IA64__
.ENDIF                          # __ALPHA__

.IFDEF IM                       # IM
DESTI = I
.ELSE                           # IM
DESTI =
.ENDIF                          # IM

.IFDEF LARGE                    # LARGE
.IFDEF __VAX__                      # __VAX__
DESTL =
.ELSE                               # __VAX__
DESTL = L
.ENDIF                              # __VAX__
.ELSE                           # LARGE
DESTL =
.ENDIF                          # LARGE

DEST = $(DESTM)$(DESTI)$(DESTL)

# Library module name suffix for XXX_.OBJ with GNU C.

.IFDEF GNUC                     # GNUC
GCC_ = _
.ELSE                           # GNUC
GCC_ =
.ENDIF                          # GNUC

# Check for option problems.

.IFDEF __VAX__                  # __VAX__
.IFDEF LARGE                        # LARGE
LARGE_VAX = 1
.ENDIF                              # LARGE
.IFDEF VAXC_OR_FORCE_VAXC           # VAXC_OR_FORCE_VAXC
.IFDEF GNUC                             # GNUC
VAX_MULTI_CMPL = 1
.ENDIF                                  # GNUC
.ENDIF                              # VAXC_OR_FORCE_VAXC
.ELSE                           # __VAX__
.IFDEF VAXC_OR_FORCE_VAXC           # VAXC_OR_FORCE_VAXC
NON_VAX_CMPL = 1
.ELSE                               # VAXC_OR_FORCE_VAXC
.IFDEF GNUC                             # GNUC
NON_VAX_CMPL = 1
.ENDIF                                  # GNUC
.ENDIF                              # VAXC_OR_FORCE_VAXC
.ENDIF                          # __VAX__

# Complain if warranted.  Otherwise, show destination directory.
# Make the destination directory, if necessary.
				
.IFDEF UNK_DEST                 # UNK_DEST
.FIRST
	@ write sys$output -
 "   Unknown system architecture."
.IFDEF __MMK__                      # __MMK__
	@ write sys$output -
 "   MMK on IA64?  Try adding ""/MACRO = __IA64__""."
.ELSE                               # __MMK__
	@ write sys$output -
 "   MMS too old?  Try adding ""/MACRO = MMS$ARCH_NAME=ALPHA"","
	@ write sys$output -
 "   or ""/MACRO = MMS$ARCH_NAME=IA64"", or ""/MACRO = MMS$ARCH_NAME=VAX"","
	@ write sys$output -
 "   as appropriate.  (Or try a newer version of MMS.)"
.ENDIF                              # __MMK__
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                           # UNK_DEST
.IFDEF VAX_MULTI_CMPL               # VAX_MULTI_CMPL
.FIRST
	@ write sys$output -
 "   Macro ""GNUC"" is incompatible with ""VAXC"" or ""FORCE_VAXC""."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                               # VAX_MULTI_CMPL
.IFDEF NON_VAX_CMPL                     # NON_VAX_CMPL
.FIRST
	@ write sys$output -
 "   Macros ""GNUC"", ""VAXC"", and ""FORCE_VAXC"" are valid only on VAX."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                                   # NON_VAX_CMPL
.IFDEF LARGE_VAX                            # LARGE_VAX
.FIRST
	@ write sys$output -
 "   Macro ""LARGE"" is invalid on VAX."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                                       # LARGE_VAX
.FIRST
	@ write sys$output "   Destination: [.$(DEST)]"
	@ write sys$output ""
	if (f$search( "$(DEST).DIR;1") .eqs. "") then -
	 create /directory [.$(DEST)]
.ENDIF                                      # LARGE_VAX
.ENDIF                                  # NON_VAX_CMPL
.ENDIF                              # VAX_MULTI_CMPL
.ENDIF                          # UNK_DEST

# DBG options.

.IFDEF DBG                      # DBG
CFLAGS_DBG = /debug /nooptimize
LINKFLAGS_DBG = /debug /traceback
.ELSE                           # DBG
CFLAGS_DBG =
LINKFLAGS_DBG = /notraceback
.ENDIF                          # DBG

# "IM" scheme for storing VMS/RMS file attributes.

.IFDEF IM                       # IM
CDEFS_IM = , VMS_IM_EXTRA
.ELSE                           # IM
CDEFS_IM =
.ENDIF                          # IM

# Large-file options.

.IFDEF LARGE                    # LARGE
CDEFS_LARGE = , LARGE_FILE_SUPPORT
.ELSE                           # LARGE
CDEFS_LARGE =
.ENDIF                          # LARGE

# C compiler defines.

.IFDEF LOCAL_ZIP
C_LOCAL_ZIP = , $(LOCAL_ZIP)
.ELSE
C_LOCAL_ZIP =
.ENDIF

CDEFS = VMS $(CDEFS_IM) $(CDEFS_LARGE) $(C_LOCAL_ZIP)

CDEFS_UNX = /define = ($(CDEFS))

CDEFS_CLI = /define = ($(CDEFS), VMSCLI)

CDEFS_UTIL = /define = ($(CDEFS), UTIL)

# Other C compiler options.

CFLAGS_INCL = /include = []

.IFDEF DECC                             # DECC
CFLAGS_ARCH = /decc /prefix = (all)
.ELSE                                   # DECC
.IFDEF FORCE_VAXC                           # FORCE_VAXC
CFLAGS_ARCH = /vaxc
.IFDEF VAXC                                     # VAXC
.ELSE                                           # VAXC
VAXC = 1
.ENDIF                                          # VAXC
.ELSE                                       # FORCE_VAXC
CFLAGS_ARCH =
.ENDIF                                      # FORCE_VAXC
.ENDIF                                  # DECC

.IFDEF VAXC_OR_FORCE_VAXC               # VAXC_OR_FORCE_VAXC
OPT_FILE = [.$(DEST)]vaxcshr.opt
LFLAGS_ARCH = $(OPT_FILE) /options, 
.ELSE                                   # VAXC_OR_FORCE_VAXC
.IFDEF GNUC                                 # GNUC
OPT_FILE = [.$(DEST)]vaxcshr.opt
LFLAGS_GNU = GNU_CC:[000000]GCCLIB.OLB /LIBRARY
LFLAGS_ARCH = $(LFLAGS_GNU), SYS$DISK:$(OPT_FILE) /options, 
.ELSE                                       # GNUC
OPT_FILE =
LFLAGS_ARCH =
.ENDIF                                      # GNUC
.ENDIF                                  # VAXC_OR_FORCE_VAXC

# LIST options.

.IFDEF LIST                     # LIST
.IFDEF DECC                         # DECC
CFLAGS_LIST = /list = $*.LIS /show = (all, nomessages)
.ELSE                               # DECC
CFLAGS_LIST = /list = $*.LIS /show = (all)
.ENDIF                              # DECC
LINKFLAGS_LIST = /map = $*.MAP /cross_reference /full
.ELSE                           # LIST
CFLAGS_LIST =
LINKFLAGS_LIST =
.ENDIF                          # LIST

# Common CFLAGS and LINKFLAGS.

CFLAGS = \
 $(CFLAGS_ARCH) $(CFLAGS_DBG) $(CFLAGS_INCL) $(CFLAGS_LIST) $(CCOPTS) \
 /object = $(MMS$TARGET)

LINKFLAGS = \
 $(LINKFLAGS_DBG) $(LINKFLAGS_LIST) $(LINKOPTS) \
 /executable = $(MMS$TARGET)

# Object library module=object lists.

#    Primary object library, [].

MODS_OBJS_LIB_ZIP_N = \
 crc32=[.$(DEST)]crc32.obj \
 crctab=[.$(DEST)]crctab.obj \
 crypt=[.$(DEST)]crypt.obj \
 deflate=[.$(DEST)]deflate.obj \
 fileio=[.$(DEST)]fileio.obj \
 globals=[.$(DEST)]globals.obj \
 trees=[.$(DEST)]trees.obj \
 ttyio=[.$(DEST)]ttyio.obj \
 util=[.$(DEST)]util.obj \
 zipfile=[.$(DEST)]zipfile.obj \
 zipup=[.$(DEST)]zipup.obj

#    Primary object library, [.vms].
                    
MODS_OBJS_LIB_ZIP_V = \
 vms=[.$(DEST)]vms.obj \
 vmsmunch=[.$(DEST)]vmsmunch.obj \
 vmszip=[.$(DEST)]vmszip.obj

MODS_OBJS_LIB_ZIP = $(MODS_OBJS_LIB_ZIP_N) $(MODS_OBJS_LIB_ZIP_V)

#    Utility object library, normal, [].

MODS_OBJS_LIB_ZIPUTILS_N = \
 crctab=[.$(DEST)]crctab.obj \
 globals=[.$(DEST)]globals.obj \
 ttyio=[.$(DEST)]ttyio.obj

#    Utility object library, variant, [].

MODS_OBJS_LIB_ZIPUTILS_U = \
 crypt$(GCC_)=[.$(DEST)]crypt_.obj \
 fileio$(GCC_)=[.$(DEST)]fileio_.obj \
 util$(GCC_)=[.$(DEST)]util_.obj \
 zipfile$(GCC_)=[.$(DEST)]zipfile_.obj

#    Utility object library, normal, [.vms].

MODS_OBJS_LIB_ZIPUTILS_N_V = \
 vmsmunch=[.$(DEST)]vmsmunch.obj

#    Utility object library, variant, [.vms].

MODS_OBJS_LIB_ZIPUTILS_U_V = \
 vms$(GCC_)=[.$(DEST)]vms_.obj

MODS_OBJS_LIB_ZIPUTILS = $(MODS_OBJS_LIB_ZIPUTILS_N) \
 $(MODS_OBJS_LIB_ZIPUTILS_U) \
 $(MODS_OBJS_LIB_ZIPUTILS_N_V) \
 $(MODS_OBJS_LIB_ZIPUTILS_U_V) \

#    CLI object library, [.vms].

MODS_OBJS_LIB_ZIPCLI_C_V = \
 cmdline=[.$(DEST)]cmdline.obj

MODS_OBJS_LIB_ZIPCLI_CLD_V = \
 ZIP_CLITABLE=[.$(DEST)]zip_cli.obj

MODS_OBJS_LIB_ZIPCLI = \
 $(MODS_OBJS_LIB_ZIPCLI_C_V) \
 $(MODS_OBJS_LIB_ZIPCLI_CLD_V)

# Executables.

ZIP = [.$(DEST)]zip.exe

ZIP_CLI = [.$(DEST)]zip_cli.exe

ZIPUTILS = \
 [.$(DEST)]zipcloak.exe \
 [.$(DEST)]zipnote.exe \
 [.$(DEST)]zipsplit.exe

