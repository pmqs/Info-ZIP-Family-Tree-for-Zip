#                                               31 December 2012.  SMS.
#
#    Zip 3.1 for VMS - MMS (or MMK) Source Description File.
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
.ENDIF                          # IM

.IFDEF LARGE                    # LARGE
.IFDEF __VAX__                      # __VAX__
.ELSE                               # __VAX__
DESTL = L
.ENDIF                              # __VAX__
.ENDIF                          # LARGE

DEST_STD = $(DESTM)$(DESTI)$(DESTL)
.IFDEF PROD                     # PROD
DEST = $(PROD)
.ELSE                           # PROD
DEST = $(DEST_STD)
.ENDIF                          # PROD
SEEK_BZ = $(DESTM)

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
.IFDEF LZMA                         # LZMA
LZMA_VAX = 1
.ENDIF                              # LZMA
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

# Complain about any problems (and die) if warranted.  Otherwise, show
# optional package directories being used, and the destination
# directory.  Make the destination directory, if necessary.

.FIRST
.IFDEF __MMK__                  # __MMK__
	@ write sys$output ""
.ENDIF                          # __MMK__
.IFDEF UNK_DEST                 # UNK_DEST
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
	@ write sys$output -
 "   Macro ""GNUC"" is incompatible with ""VAXC"" or ""FORCE_VAXC""."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                               # VAX_MULTI_CMPL
.IFDEF NON_VAX_CMPL                     # NON_VAX_CMPL
	@ write sys$output -
 "   Macros ""GNUC"", ""VAXC"", and ""FORCE_VAXC"" are valid only on VAX."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                                   # NON_VAX_CMPL
.IFDEF LARGE_VAX                            # LARGE_VAX
	@ write sys$output -
 "   Macro ""LARGE"" is invalid on VAX."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                                       # LARGE_VAX
.IFDEF LZMA_VAX                                 # LZMA_VAX
	@ write sys$output -
 "   Macro ""LZMA"" is invalid on VAX."
	@ write sys$output ""
	I_WILL_DIE_NOW.  /$$$$INVALID$$$$
.ELSE                                           # LZMA_VAX
.IFDEF IZ_BZIP2                                     # IZ_BZIP2
	@ @[.VMS]FIND_BZIP2_LIB.COM $(IZ_BZIP2) $(SEEK_BZ) -
	   LIBBZ2_NS.OLB lib_bzip2
	@ if (f$trnlnm( "lib_bzip2") .eqs. "") then -
	   write sys$output "   Can not find BZIP2 object library."
	@ if (f$trnlnm( "lib_bzip2") .eqs. "") then -
	   write sys$output ""
	@ if (f$trnlnm( "lib_bzip2") .eqs. "") then -
	   I_WILL_DIE_NOW.  /$$$$INVALID$$$$
	@ write sys$output "   BZIP2 dir: ''f$trnlnm( "lib_bzip2")'"
	@ write sys$output ""
	@ define incl_bzip2 $(IZ_BZIP2)
.ENDIF                                              # IZ_BZIP2
.IFDEF IZ_ZLIB                                      # IZ_ZLIB
	@ @[.VMS]FIND_BZIP2_LIB.COM $(IZ_ZLIB) $(SEEK_BZ) LIBZ.OLB lib_zlib
	@ if (f$trnlnm( "lib_zlib") .eqs. "") then -
	   write sys$output "   Can not find ZLIB object library."
	@ if (f$trnlnm( "lib_zlib") .eqs. "") then -
	   write sys$output ""
	@ if (f$trnlnm( "lib_zlib") .eqs. "") then -
	   I_WILL_DIE_NOW.  /$$$$INVALID$$$$
	@ write sys$output "   ZLIB dir:  ''f$trnlnm( "lib_zlib")'"
	@ write sys$output ""
	@ define incl_zlib $(IZ_ZLIB)
.ENDIF                                              # IZ_ZLIB
	@ write sys$output "   Destination: [.$(DEST)]"
	@ write sys$output ""
	if (f$search( "$(DEST).DIR;1") .eqs. "") then -
	 create /directory [.$(DEST)]
.ENDIF                                          # LZMA_VAX
.ENDIF                                      # LARGE_VAX
.ENDIF                                  # NON_VAX_CMPL
.ENDIF                              # VAX_MULTI_CMPL
.ENDIF                          # UNK_DEST

# AES_WG options.

.IFDEF AES_WG                   # AES_WG
CDEFS_AES = , CRYPT_AES_WG
.ENDIF                          # AES_WG

# BZIP2 options.

.IFDEF IZ_BZIP2                 # IZ_BZIP2
CDEFS_BZ = , BZIP2_SUPPORT
LIB_BZIP2_OPTS = LIB_BZIP2:LIBBZ2_NS.OLB /library,
.ENDIF                          # IZ_BZIP2

# LZMA options.

.IFDEF LZMA                     # LZMA
LZMA_PPMD = 1
.IFDEF __VAX__                      # __VAX__
CDEFS_LZMA = , LZMA_SUPPORT, _SZ_NO_INT_64
.ELSE                               # __VAX__
CDEFS_LZMA = , LZMA_SUPPORT
.ENDIF                              # __VAX__
.ENDIF                          # LZMA

# PPMd options.

.IFDEF PPMD                     # PPMD
.IFDEF LZMA                         # LZMA
.IFDEF __VAX__                          # __VAX__
.IFDEF VAXC_OR_FORCE_VAXC                   # VAXC_OR_FORCE_VAXC
CDEFS_PPMD = , PPMD_SUPPORT, NO_SIGNED_CHAR
.ELSE                                       # VAXC_OR_FORCE_VAXC
CDEFS_PPMD = , PPMD_SUPPORT
.ENDIF                                      # VAXC_OR_FORCE_VAXC
.ELSE                                   # __VAX__
CDEFS_PPMD = , PPMD_SUPPORT
.ENDIF                                  # __VAX__
.ELSE                               # LZMA
LZMA_PPMD = 1
.IFDEF __VAX__                          # __VAX__
.IFDEF VAXC_OR_FORCE_VAXC                   # VAXC_OR_FORCE_VAXC
CDEFS_PPMD = , PPMD_SUPPORT, NO_SIGNED_CHAR, _SZ_NO_INT_64
.ELSE                                       # VAXC_OR_FORCE_VAXC
CDEFS_PPMD = , PPMD_SUPPORT, _SZ_NO_INT_64
.ENDIF                                      # VAXC_OR_FORCE_VAXC
.ELSE                                   # __VAX__
CDEFS_PPMD = , PPMD_SUPPORT
.ENDIF                                  # __VAX__
.ENDIF                              # LZMA
.ENDIF                          # PPMD

# ZLIB options.

.IFDEF IZ_ZLIB                  # IZ_ZLIB
CDEFS_ZL = , USE_ZLIB
LIB_ZLIB_OPTS = LIB_ZLIB:LIBZ.OLB /library,
.ENDIF                          # IZ_ZLIB

CFLAGS_INCL = /INCLUDE = ([], [.VMS])

# DBG, TRC options.

.IFDEF DBG                      # DBG
CFLAGS_DBG = /debug /nooptimize
LINKFLAGS_DBG = /debug /traceback
.ELSE                           # DBG
CFLAGS_DBG =
.IFDEF TRC                          # TRC
LINKFLAGS_DBG = /traceback
.ELSE                               # TRC
LINKFLAGS_DBG = /notraceback
.ENDIF                              # TRC
.ENDIF                          # DBG

# "IM" scheme for storing VMS/RMS file attributes.

.IFDEF IM                       # IM
CDEFS_IM = , VMS_IM_EXTRA
.ENDIF                          # IM

# Large-file options.

.IFDEF LARGE                    # LARGE
CDEFS_LARGE = , LARGE_FILE_SUPPORT
.ENDIF                          # LARGE

# C compiler defines.

.IFDEF LOCAL_ZIP
C_LOCAL_ZIP = , $(LOCAL_ZIP)
.ENDIF

CDEFS = VMS $(CDEFS_AES) $(CDEFS_BZ) $(CDEFS_IM) $(CDEFS_LARGE) \
 $(CDEFS_LZMA) $(CDEFS_PPMD) $(CDEFS_ZL) $(C_LOCAL_ZIP)

CDEFS_UNX = /define = ($(CDEFS))

CDEFS_CLI = /define = ($(CDEFS), VMSCLI)

CDEFS_UTIL = /define = ($(CDEFS), UTIL)

CDEFS_LIBZIP = /define = ($(CDEFS), USE_ZIPMAIN)

# Other C compiler options.

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

# LINK options.

.IFDEF VAXC_OR_FORCE_VAXC               # VAXC_OR_FORCE_VAXC
OPT_FILE = [.$(DEST)]VAXCSHR.OPT
LFLAGS_ARCH = $(OPT_FILE) /options, 
.ELSE                                   # VAXC_OR_FORCE_VAXC
.IFDEF GNUC                                 # GNUC
OPT_FILE = [.$(DEST)]VAXCSHR.OPT
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
CFLAGS_LIST = /nolist
LINKFLAGS_LIST = /nomap
.ENDIF                          # LIST

# Common CFLAGS and LINKFLAGS.

CFLAGS = \
 $(CFLAGS_ARCH) $(CFLAGS_DBG) $(CFLAGS_INCL) $(CFLAGS_LIST) $(CCOPTS) \
 /object = $(MMS$TARGET)

CFLAGS_DEP = $(CFLAGS_ARCH) $(CFLAGS_INCL) $(CCOPTS)

LINKFLAGS = \
 $(LINKFLAGS_DBG) $(LINKFLAGS_LIST) $(LINKOPTS) \
 /executable = $(MMS$TARGET)

# Object library module=object lists.

#    Primary object library, [].

MODS_OBJS_LIB_ZIP_N = \
 CRC32=[.$(DEST)]CRC32.OBJ \
 CRYPT=[.$(DEST)]CRYPT.OBJ \
 DEFLATE=[.$(DEST)]DEFLATE.OBJ \
 FILEIO=[.$(DEST)]FILEIO.OBJ \
 GLOBALS=[.$(DEST)]GLOBALS.OBJ \
 TREES=[.$(DEST)]TREES.OBJ \
 TTYIO=[.$(DEST)]TTYIO.OBJ \
 UTIL=[.$(DEST)]UTIL.OBJ \
 ZBZ2ERR=[.$(DEST)]ZBZ2ERR.OBJ \
 ZIPFILE=[.$(DEST)]ZIPFILE.OBJ \
 ZIPUP=[.$(DEST)]ZIPUP.OBJ

.IFDEF LIBZIP                   # LIBZIP
MODS_OBJS_LIB_LIBZIP_N = \
 API$(GCC_)=[.$(DEST)]API_.OBJ \
 ZIP$(GCC_)=[.$(DEST)]ZIP_.OBJ
.ENDIF                          # LIBZIP

#    Primary object library, [.VMS].
                    
MODS_OBJS_LIB_ZIP_V = \
 VMS=[.$(DEST)]VMS.OBJ \
 VMSMUNCH=[.$(DEST)]VMSMUNCH.OBJ \
 VMSZIP=[.$(DEST)]VMSZIP.OBJ

#    Primary object library, [.AES_WG].

.IFDEF AES_WG                   # AES_WG
MODS_OBJS_LIB_ZIP_AES = \
 AESCRYPT=[.$(DEST)]AESCRYPT.OBJ \
 AESKEY=[.$(DEST)]AESKEY.OBJ \
 AESTAB=[.$(DEST)]AESTAB.OBJ \
 FILEENC=[.$(DEST)]FILEENC.OBJ \
 HMAC=[.$(DEST)]HMAC.OBJ \
 PRNG=[.$(DEST)]PRNG.OBJ \
 PWD2KEY=[.$(DEST)]PWD2KEY.OBJ \
 SHA1=[.$(DEST)]SHA1.OBJ
.ENDIF                          # AES_WG

#    Primary object library, [.SZIP], LZMA.

.IFDEF LZMA                     # LZMA
MODS_OBJS_LIB_ZIP_LZMA = \
 LZFIND=[.$(DEST)]LZFIND.OBJ \
 LZMAENC=[.$(DEST)]LZMAENC.OBJ
.ENDIF                          # LZMA

#    Primary object library, [.SZIP], PPMd.

.IFDEF PPMD                     # PPMD
MODS_OBJS_LIB_ZIP_PPMD = \
 PPMD8=[.$(DEST)]PPMD8.OBJ \
 PPMD8ENC=[.$(DEST)]PPMD8ENC.OBJ
.ENDIF                          # PPMD

MODS_OBJS_LIB_ZIP = $(MODS_OBJS_LIB_ZIP_N) $(MODS_OBJS_LIB_LIBZIP_N) \
 $(MODS_OBJS_LIB_ZIP_V) \
 $(MODS_OBJS_LIB_ZIP_AES) $(MODS_OBJS_LIB_ZIP_LZMA) \
 $(MODS_OBJS_LIB_ZIP_PPMD)

#    Utility object library, normal, [].

MODS_OBJS_LIB_ZIPUTILS_N = \
 CRC32=[.$(DEST)]CRC32.OBJ \
 GLOBALS=[.$(DEST)]GLOBALS.OBJ \
 TTYIO=[.$(DEST)]TTYIO.OBJ

#    Utility object library, variant, [].

MODS_OBJS_LIB_ZIPUTILS_U = \
 CRYPT$(GCC_)=[.$(DEST)]CRYPT_.OBJ \
 FILEIO$(GCC_)=[.$(DEST)]FILEIO_.OBJ \
 UTIL$(GCC_)=[.$(DEST)]UTIL_.OBJ \
 ZIPFILE$(GCC_)=[.$(DEST)]ZIPFILE_.OBJ

#    Utility object library, normal, [.VMS].

MODS_OBJS_LIB_ZIPUTILS_N_V = \
 VMSMUNCH=[.$(DEST)]VMSMUNCH.OBJ

#    Utility object library, variant, [.VMS].

MODS_OBJS_LIB_ZIPUTILS_U_V = \
 VMS$(GCC_)=[.$(DEST)]VMS_.OBJ

MODS_OBJS_LIB_ZIPUTILS = $(MODS_OBJS_LIB_ZIPUTILS_N) \
 $(MODS_OBJS_LIB_ZIPUTILS_U) \
 $(MODS_OBJS_LIB_ZIPUTILS_N_V) \
 $(MODS_OBJS_LIB_ZIPUTILS_U_V) \
 $(MODS_OBJS_LIB_ZIP_AES) \
 $(MODS_OBJS_LIB_ZIP_LZMA) \
 $(MODS_OBJS_LIB_ZIP_PPMD)

#    CLI object library, [.VMS].

MODS_OBJS_LIB_ZIPCLI_C_V = \
 CMDLINE=[.$(DEST)]CMDLINE.OBJ

MODS_OBJS_LIB_ZIPCLI_CLD_V = \
 ZIP_CLITABLE=[.$(DEST)]ZIP_CLI.OBJ

MODS_OBJS_LIB_ZIPCLI = \
 $(MODS_OBJS_LIB_ZIPCLI_C_V) \
 $(MODS_OBJS_LIB_ZIPCLI_CLD_V)

# Executables.

ZIP = [.$(DEST)]ZIP.EXE

ZIP_CLI = [.$(DEST)]ZIP_CLI.EXE

ZIPUTILS = \
 [.$(DEST)]ZIPCLOAK.EXE \
 [.$(DEST)]ZIPNOTE.EXE \
 [.$(DEST)]ZIPSPLIT.EXE

