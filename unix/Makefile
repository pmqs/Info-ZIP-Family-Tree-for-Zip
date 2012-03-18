#==============================================================================
# unix/Makefile
# - For Zip, ZipNote, ZipCloak and ZipSplit            Last update: 17 Mar 2012
#==============================================================================

# INSTRUCTIONS (such as they are):
#
# "make -f unix/Makefile generic"         Make Zip on most systems using "cc"
#
# "make -f unix/Makefile generic CC=gcc"  Make Zip on most systems using "gcc"
#
# "make -f unix/Makefile list"            Lists all supported targets
#
# "make -f unix/Makefile help"            Limited advice on which targets to
#                                         try if problems occur.
#
# CF are flags for the C compiler.  LF are flags for the linker.  LF2 are more
# flags for the linker, if they need to be at the end of the line instead of
# at the beginning (for example, some libraries).  LOCAL_ZIP is a "make" macro
# that can be used to add default C flags to your compile without editing the
# Makefile (e.g., -DDEBUG_STRUC, or -FPi87 on PCs using Microsoft C).
#
# The "generic" target invokes the "unix/configure" script to test the compile
# and other parts of the environment to see what's supported.  The results are
# used to adjust the build procedure and to enable or disable some program
# features.  unix/configure builds a ./flags file with appropriate values for
# a number of make variables, which is used by this Makefile when performing
# the actual builds.  A separate flags file for building BZIP2 may also be
# created.  This Makefile recursively invokes this Makefile to perform the
# actual builds.
#
# While oriented towards building unix (and unix work-alike) ports of Zip, this
# file and/or unix/configure are also used by other ports of Zip.  See below.
#
# If at all possible, use the generic targets.  The other (older) targets are
# less well tested, and may need considerable help from the user to get
# optimal results (or any success at all).
#
# The "generic_docs" and "generic_man" targets also invoke "unix/configure",
# and generate the Zip documentation files.
#
# Optional Zip features are controlled by setting "make" macros as described
# below or by the results of tests done by configure (such as checking if the
# needed files are in an appropriate directory).
#
# Optional features include (see INSTALL for installation instructions):
#
# AES encryption - If you want AES encryption (WinZip/Gladman) support
#   included, unzip the AES source archive (iz_aes_wg10.zip, or whatever is
#   the latest version, available on the Info-ZIP ftp site) to create the
#   local ./aes_wg subdirectory, and set the "make" macro AES_WG to 1.
#   See the files README_AES_WG.txt and INSTALL for more information.
#
# LZMA compression - If you want LZMA compression support included, set the
#   "make" macro LZMA to 1.  See the files README_LZMA.txt and INSTALL for
#   more information.
#
# PPMd compression - If you want PPMd compression support included, set the
#   "make" macro PPMD to 1.  See the files README_LZMA.txt and INSTALL for
#   more information.
#
# Bzip2 compression - If you want bzip2 compression support included, there
#   are three options:
#   - Create a local build of bzip2 by unzipping the latest bzip2 source
#     archive into the ./bzip2 subdirectory and setting the "make" macro
#     IS_BZIP2 to bzip2.  unix/Makefile will attempt to properly build the
#     library using the bzip2 make file.
#   - Use an existing build of the bzip2 library by setting the "make" macro
#     IZ_BZIP2 to a directory containing the bzip2 header file and object
#     (and/or shared) library files.
#   - Use the system bzip2 library.  Ensure the local ./bzip2 subdirectory
#     exists and does not contain the files Makefile, libbz2.a, or bzlib.h.
#     Set the "make" macro IZ_BZIP2 to bzip2.  However, use one of the above
#     options first, if possible, to avoid an issue with error handling that
#     may not be corrected in the system library.
#   See the files README_BZIP2.txt and INSTALL for more information.
#
# Zlib deflation - If you want to use zlib for compressing instead of Zip's
#   built-in code, there are two options:
#   - Use an existing build of the zlib support by setting the "make" macro
#     IZ_ZLIB to a directory containing suitable zlib header and object/shared
#     library files.
#   - Use the system zlib support by ensuring the local ./zlib subdirectory
#     exists, and does not contain the files libz.a or bzlib.h. Set the "make"
#     macro IZ_BZIP2 to zlib.
#
# Zip Ports:
#
# This Makefile and/or unix/configure script support a number of non-unix Zip
# ports, where the Zip program build is under unix (or a unix work-alike), but
# the Zip program execution may be on another system or execution environment.
# The unix/configure script will be invoked from either unix/Makefile, or from
# a Makefile located in another port directory.
#
# The port may be explicitly identified by the PORT parameter, or implicitly
# detected by compiler configuration tests.
#
# Supported ports include:
#   msdos : DOS port built using the DJGPP (or Cygwin) cross-compiler
#   os2   : OS/2 port
#   qdos  : Sinclair Q-DOS port built using a gcc cross-compiler
#   zos   : z/OS MVS port built under z/OS UNIX
#   zvm   : z/VM CMS port built under z/VM OpenExtensions
#
# Be sure to test your new Zip (and ZipNote, ZipCloak and ZipSplit).  A
# successful build does not guarantee a working or fully functional program.


#####################
# DEFAULT TARGETS   #
#####################

# List of supported systems/targets in this version
GENERIC_TARGETS  = generic  generic_man  generic_docs  generic_pkg  flags
MAKE_ACTIONS  = clean  clean_bzip2  clean_exe  clean_docs

SYS_TARGETS  = att6300nodir coherent cray_v3 cygwin lynx minix qnx qnxnto solaris

# What you can make ...
help:
	@echo ''
	@echo 'Make what?  You must specify which system to make Zip for.'
	@echo 'The "make" generic target choices are:'
	@echo ''
	@echo "  $(GENERIC_TARGETS)"
	@echo ''
	@echo 'Additional "make" actions include:'
	@echo ''
	@echo "  $(MAKE_ACTIONS)"
	@echo ''
	@echo 'For a list including specific targets for older systems:'
	@echo '   make -f unix/Makefile list'
	@echo ''
	@echo 'Optional overrides on "make" command (or environment variables):'
	@echo '    "CC=gcc"          Use "gcc" instead of "cc" for C compiler'
	@echo '    "AES_WG=1"        Add AES encryption (WinZip/Gladman)'
	@echo '    "LZMA=1"          Add LZMA compression'
	@echo '    "PPMD=1"          Add PPMd compression'
	@echo '    "IZ_BZIP2=bz2dir" Add BZIP2 compression from dir bz2dir'
	@echo '    "IZ_ZLIB=zdir"    Use ZLIB from dir zdir for deflate'
	@echo '    "LOCAL_ZIP=opts"  Add C compiler options and macro defines'
	@echo '    "LIST=1"          Generate C compiler and linker listing'
	@echo ''
	@echo 'Examples:'
	@echo '   make -f unix/Makefile generic'
	@echo '    - make Zip programs using cc'
	@echo '   make -f unix/Makefile generic CC=gcc AES_WG=1'
	@echo '    - make Zip programs using gcc, with AES_WG encryption'
	@echo ''
	@echo 'First, try "make -f unix/Makefile generic" (to use "cc")'
	@echo '(or "make -f unix/Makefile generic CC=gcc" (to use "gcc")),'
	@echo 'as "generic" should autodetect and set the proper flags.'
	@echo ''
	@echo 'Use the flags target to rebuild the system-specific flags used'
	@echo 'by Makefile.  However, you probably want to run clean first.'
	@echo 'As the generic targets automatically make flags, you should'
	@echo 'not need to make flags directly.'
	@echo ''
	@echo 'To make the manuals use "make -f unix/Makefile generic_man" to'
	@echo 'autodetect and use the proper flags.'
	@echo ''
	@echo 'Use generic_docs to make the plain text docs.  These are the'
	@echo 'files, like zip.txt, that should already be in the Zip root'
	@echo 'directory.  To recreate these files, clear out the originals'
	@echo 'using the clean_docs target, then use the generic_docs target.'
	@echo ''
	@echo 'See the files INSTALL and zip.txt, and the comments in'
	@echo 'unix/Makefile, for more information.'
	@echo ''

# list
# - Display list of supported systems/targets in this version
# - The SYSTEM variable is no longer supported; use
#   "make -f unix/Makefile <target>" instead.
list:
	@echo ''
	@echo 'Type "make -f unix/Makefile <target>", where <target> is one of'
	@echo 'the following:'
	@echo ''
	@echo 'Generic targets:'
	@echo "  $(GENERIC_TARGETS)"
	@echo ''
	@echo 'Make actions:'
	@echo "  $(MAKE_ACTIONS)"
	@echo ''
	@echo 'Specific targets:'
	@echo "  $(SYS_TARGETS)"
	@echo ''
	@echo 'Use one of the generic targets if possible, as the specific'
	@echo 'targets tend to be woefully out of date.'
	@echo ''
	@echo 'For further (very useful) information, please read the comments'
	@echo 'in unix/Makefile.'
	@echo ''


#####################
# MACRO DEFINITIONS #
#####################

#----------------------------------------------------------------------------
# For most systems:
# - The generic target invokes unix/configure to build ./flags and
#   ./flags_bz containing system derived settings that are used by
#   this makefile.
# - Use "make" macro LOCAL_ZIP to add C compiler flags (such as
#   -DDOSWILD).
#----------------------------------------------------------------------------

# This file name as make parameter, for recursion
MAKEF = '-f unix/Makefile'

# Build Directories
# - Root    build directory is "."
# - Manuals build directory is "./manout"

# Build Dependencies for make
# - Contains archive library name, if enabled
#    AES (WinZip/Gladman) encryption
#    LZMA compression
#    PPMd compression
#    BZIP2 compression
#    ZLIB compression
LIB_AES_WG  =
LIB_LZMA    =
LIB_PPMD    =
LIB_BZ      =
LIB_Z       =

# Build makefiles: bzip2 compression
MK_BZ   = Makefile

# OS-dependent files
# - Default:  Unix port.
# - Compilation and execution are both for Unix (or a Unix-like) environment.
#   The files in the Zip root and unix directories are used.
# - The vast majority of you out there will likely use this configuration, and
#   do not have to ever worry about this.
# - Where required by a specific Zip port, a generic target (in this Makefile
#   or a Makefile located in another port directory) uses the PORT macro to
#   set the correct OS-dependent files via unix/configure.  Or you can (very
#   carefully) override as required.
#    C header files
#    C object files
#    Assembler object files
OSDEP_H  = unix/osdep.h
OSDEP_OC = ./unix.o
OSDEP_OS =

# OS-dependent file extensions
# - This default may be customized by unix/configure based on PORT
#    Executable programs
#    DLL (.so) (currently not used)
#    man page
PGMEXT =
DLLEXT = .so
MANEXT = 1

# OS-dependent installation directories: executables and man pages
# - This default may be customized by unix/configure based on PORT
BINDIR = /usr/local/bin
MANDIR = /usr/local/man/man$(MANEXT)

# OS-dependent installation file permissions: executables and man pages
# - This default may be customized by unix/configure based on PORT
BINPERMS = 755
MANPERMS = 644

# C compiler and flags: core and bzip2
# - CC and LD must match, else "unresolved symbol:  ___main" is possible.
CC     = cc
CC_BZ  = $(CC)

CF_NOOPT = -I. -DUNIX $(LOCAL_ZIP)
CFLAGS = -O
CF     = $(CFLAGS) $(CF_NOOPT)         # C compiler flags (core components)
CF_BZ  = $(CF)                         # C compiler flags (BZIP2 components)

# C preprocessor
CPP    = $(CC) -E

# Assembler and assembler flags
AS     = as
AF     = $(LOCAL_ZIP)

# Linker and linker flags (early and late)
# - CC and LD must match, else "unresolved symbol:  ___main" is possible)
LD     = $(CC)
LF1    =
LF2    = -s

# Other utility programs
AR     = ar cq
CHMOD  = chmod
LN     = ln
RANLIB = ranlib
RM     = rm -f
SHELL  = /bin/sh
STRIP  = strip
ZIP    = zip

# Default for dependency on auto-configure result, is an empty symbol
# so that the static non-autoconfigure targets continue to work
ACONF_DEP =

#----------------------------------------------------------------------------
#  Build configuration:  header files
#----------------------------------------------------------------------------

# OSDEP_H is port-dependent and can be updated by unix/configure

# C header files: zip
H_ZIP    = zip.h            \
           ziperr.h         \
           tailor.h         \
           $(OSDEP_H)       \
           $(ACONF_DEP)

# C header files: AES (WinZip/Gladman) encryption
H_AES    = aes_wg/aes.h     \
           aes_wg/aesopt.h  \
           aes_wg/fileenc.h \
           aes_wg/hmac.h    \
           aes_wg/prng.h    \
           aes_wg/pwd2key.h \
           aes_wg/sha1.h

# C header files: LZMA compression
H_LZMA   = lzma/SzVersion.h \
           lzma/LzFind.h    \
           lzma/LzHash.h    \
           lzma/LzmaEnc.h   \
           lzma/Types.h

# C header files: PPMd compression
H_PPMD   = lzma/CpuArch.h   \
           lzma/Ppmd.h      \
           lzma/Ppmd8.h     \
           lzma/Types.h

#----------------------------------------------------------------------------
#  Build configuration:  object files
#----------------------------------------------------------------------------

# OSDEP_OC and OSDEP_OS are port-dependent and can be updated by unix/configure

# object files: zip
O_ZIP    = zip.o       \
           crc32.o     \
           crypt.o     \
           deflate.o   \
           fileio.o    \
           globals.o   \
           trees.o     \
           ttyio.o     \
           util.o      \
           zbz2err.o   \
           zipfile.o   \
           zipup.o     \
           $(OSDEP_OC) \
           $(OSDEP_OS)

# object files: zipcloak
O_CLOAK  = zipcloak.o  \
           crc32.o     \
           crypt_.o    \
           fileio_.o   \
           globals.o   \
           ttyio.o     \
           util_.o     \
           zipfile_.o  \
           $(OSDEP_OC:.o=_.o) \
           $(OSDEP_OS)

# object files: zipnote
O_NOTE   = zipnote.o   \
           crc32.o     \
           fileio_.o   \
           globals.o   \
           util_.o     \
           zipfile_.o  \
           $(OSDEP_OC:.o=_.o) \
           $(OSDEP_OS)

# object files: zipsplit
O_SPLIT  = zipsplit.o \
           crc32.o    \
           fileio_.o  \
           globals.o  \
           util_.o    \
           zipfile_.o \
           $(OSDEP_OC:.o=_.o) \
           $(OSDEP_OS)

# object files: Assembler Match (Source: match.S)
# - All OS-dependent, by unix/configure based on PORT
O_MCH_I  = ./mch_i86.o
O_MCH_G  = ./mch_gcc.o
O_MCH_SV = ./mch_sysv.o

# object files: Assembler CRC (Source: msdos/crc_i86.S)
# - All OS-dependent, by unix/configure based on PORT
O_CRC_I  = ./crc_i86.o

# object files: Assembler CRC (Source: crc_i386.S)
# - All OS-dependent, by unix/configure based on PORT
O_CRC_G  = ./crc_gcc.o
O_CRC_SV = ./crc_sysv.o

# object files: AES (WinZip/Gladman) encryption
O_AES    = aescrypt.o \
           aeskey.o   \
           aestab.o   \
           fileenc.o  \
           hmac.o     \
           prng.o     \
           pwd2key.o  \
           sha1.o

# object files: LZMA compression
O_LZMA   = LzFind.o   \
           LzmaEnc.o

# object files: PPMd compression
O_PPMD   = Ppmd8.o   \
           Ppmd8Enc.o

# object files: ZLIB compression
# - N/A. ZLIB is not built as part of Zip

#----------------------------------------------------------------------------
#  Build configuration:  object archive libraries
#----------------------------------------------------------------------------

# object archive: AES (WinZip/Gladman) encryption
AR_AES  = libaes.a

# object archive: BZIP2 compression
AR_BZ   = bzip2/libbz2.a

# object archive: LZMA compression
AR_LZMA = liblzma.a

# object archive: PPMd compression
AR_PPMD = libppmd.a

# object archive: ZLIB compression
AR_Z    = zlib/libz.a

#----------------------------------------------------------------------------
#  Build configuration:  Executable programs
#----------------------------------------------------------------------------
PGM_ZIP    = zip$(PGMEXT)
PGM_CLOAK  = zipcloak$(PGMEXT)
PGM_NOTE   = zipnote$(PGMEXT)
PGM_SPLIT  = zipsplit$(PGMEXT)

ZIP_PGMS   = $(PGM_ZIP)   \
             $(PGM_CLOAK) \
             $(PGM_NOTE)  \
             $(PGM_SPLIT)

#----------------------------------------------------------------------------
#  Build configuration:  Manuals
#----------------------------------------------------------------------------
MAN_ZIP    = manout/zip.$(MANEXT)
MAN_CLOAK  = manout/zipcloak.$(MANEXT)
MAN_NOTE   = manout/zipnote.$(MANEXT)
MAN_SPLIT  = manout/zipsplit.$(MANEXT)

ZIP_MANS   = $(MAN_ZIP)   \
             $(MAN_CLOAK) \
             $(MAN_NOTE)  \
             $(MAN_SPLIT)


# Plain text versions go in the root directory for easy user access

DOC_ZIP    = zip.txt
DOC_CLOAK  = zipcloak.txt
DOC_NOTE   = zipnote.txt
DOC_SPLIT  = zipsplit.txt

ZIP_DOCS   = $(DOC_ZIP)   \
             $(DOC_CLOAK) \
             $(DOC_NOTE)  \
             $(DOC_SPLIT)

#----------------------------------------------------------------------------
#  Build configuration:  Installation
#----------------------------------------------------------------------------

# (probably can change next two to `install' and `install -d' if you have it)
INSTALL   = cp
INSTALL_PROGRAM = $(INSTALL)
INSTALL_D = mkdir -p

BIN_ZIP   = $(BINDIR)/zip$(PGMEXT)
BIN_CLOAK = $(BINDIR)/zipcloak$(PGMEXT)
BIN_NOTE  = $(BINDIR)/zipnote$(PGMEXT)
BIN_SPLIT = $(BINDIR)/zipsplit$(PGMEXT)

INSTALLEDBIN = $(BIN_ZIP)   \
               $(BIN_CLOAK) \
               $(BIN_NOTE)  \
               $(BIN_SPLIT)

INSTALLEDMAN = $(MANDIR)/zip.$(MANEXT)      \
               $(MANDIR)/zipcloak.$(MANEXT) \
               $(MANDIR)/zipnote.$(MANEXT)  \
               $(MANDIR)/zipsplit.$(MANEXT)

# Solaris 2.x package stuff:
PKGDIR  = IZunzip
VERSION = Version 3.1


###############################################
# BASIC COMPILE INSTRUCTIONS AND DEPENDENCIES #
###############################################

#----------------------------------------------------------------------------
#  Build rules: File suffixes
#  - Remove all built-in rules
#  - Only explicit 1:1 rules are used
#----------------------------------------------------------------------------
.SUFFIXES:

#----------------------------------------------------------------------------
#  Build rules: C compile (.c -> .o)
#  - Explicit 1:1 rules are used
#  - Across various make utilities, pattern rule support is inconsistent
#  - Sys V make has issues with $< when input and output names differ
#----------------------------------------------------------------------------

# C Compile rules: zip

# A) Core zip C files

zip.o:       zip.c      $(H_ZIP) crc32.h crypt.h revision.h ttyio.h
	$(CC) -c $(CF) -o $@ zip.c

crc32.o:     crc32.c    $(H_ZIP) crc32.h
	$(CC) -c $(CF) -o $@ crc32.c

crypt.o:     crypt.c    $(H_ZIP) crc32.h crypt.h ttyio.h
	$(CC) -c $(CF) -o $@ crypt.c

deflate.o:   deflate.c  $(H_ZIP)
	$(CC) -c $(CF) -o $@ deflate.c

fileio.o:    fileio.c   $(H_ZIP) crc32.h
	$(CC) -c $(CF) -o $@ fileio.c

globals.o:   globals.c  $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT)
	$(CC) -c $(CF) -o $@ globals.c

trees.o:     trees.c    $(H_ZIP)
	$(CC) -c $(CF) -o $@ trees.c

ttyio.o:     ttyio.c    $(H_ZIP) $(H_CLOAK) crypt.h ttyio.h
	$(CC) -c $(CF) -o $@ ttyio.c

util.o:      util.c     $(H_ZIP)
	$(CC) -c $(CF) -o $@ util.c

zbz2err.o:   zbz2err.c  $(H_ZIP)
	$(CC) -c $(CF) -o $@ zbz2err.c

zipfile.o:   zipfile.c  $(H_ZIP) crc32.h
	$(CC) -c $(CF) -o $@ zipfile.c

zipup.o:     zipup.c    $(H_ZIP) crc32.h crypt.h revision.h unix/zipup.h
	$(CC) -c $(CF) -o $@ zipup.c

# B) C files for specific ports
#    - Files under msdos and os2 are used when cross compiling for those ports
#    - Generated when referenced by OSDEP_OC
#    - OSDEP_OC is customized by unix/configure, based on PORT macro

./unix.o: unix/unix.c   $(H_ZIP)
	$(CC) -c $(CF) -o $@ unix/unix.c

./msdos.o: msdos/msdos.c $(H_ZIP)
	$(CC) -c $(CF) -o $@ msdos/msdos.c

./os2.o: os2/os2.c      $(H_ZIP)
	$(CC) -c $(CF) -o $@ os2/os2.c


#----------------------------------------------------------------------------

# C Compile rules: Zip utilities

# A) Core Zip utility C files

zipcloak.o:  zipcloak.c $(H_ZIP) $(H_CLOAK) crc32.h crypt.h revision.h ttyio.h
	$(CC) -c $(CF) -DUTIL -o $@ zipcloak.c

zipnote.o:   zipnote.c  $(H_ZIP) $(H_NOTE) revision.h
	$(CC) -c $(CF) -DUTIL -o $@ zipnote.c

zipsplit.o:  zipsplit.c $(H_ZIP) $(H_SPLIT) revision.h
	$(CC) -c $(CF) -DUTIL -o $@ zipsplit.c

crc32_.o:   crc32.c     $(H_ZIP) $(H_CLOAK) crc32.h
	$(CC) -c $(CF) -DUTIL -o $@ crc32.c

crypt_.o:   crypt.c     $(H_ZIP) $(H_CLOAK) crc32.h crypt.h ttyio.h
	$(CC) -c $(CF) -DUTIL -o $@ crypt.c

fileio_.o:  fileio.c    $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT) crc32.h
	$(CC) -c $(CF) -DUTIL -o $@ fileio.c

util_.o:    util.c      $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT)
	$(CC) -c $(CF) -DUTIL -o $@ util.c

zipfile_.o: zipfile.c   $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT) crc32.h
	$(CC) -c $(CF) -DUTIL -o $@ zipfile.c

# B) C files for specific ports
#    - Generated when referenced by OSDEP_OC
#    - OSDEP_OC is customized by unix/configure, based on PORT macro

./unix_.o: unix/unix.c  $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT)
	$(CC) -c $(CF) -DUTIL -o $@ unix/unix.c

./msdos_.o: msdos/msdos.c $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT)
	$(CC) -c $(CF) -DUTIL -o $@ msdos/msdos.c

./os2_.o: os2/os2.c     $(H_ZIP) $(H_CLOAK) $(H_NOTE) $(H_SPLIT)
	$(CC) -c $(CF) -DUTIL -o $@ os2/os2.c


#----------------------------------------------------------------------------

# C Compile rules: AES (WinZip/Gladman) encryption

aescrypt.o: aes_wg/aescrypt.c $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/aescrypt.c

aeskey.o:   aes_wg/aeskey.c   $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/aeskey.c

aestab.o:   aes_wg/aestab.c   $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/aestab.c

fileenc.o:  aes_wg/fileenc.c  $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/fileenc.c

hmac.o:     aes_wg/hmac.c     $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/hmac.c

prng.o:     aes_wg/prng.c     $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/prng.c

pwd2key.o:  aes_wg/pwd2key.c  $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/pwd2key.c

sha1.o:     aes_wg/sha1.c     $(H_AES)
	$(CC) -c $(CF) -o $@ aes_wg/sha1.c


#----------------------------------------------------------------------------

# C Compile rules: LZMA compression

LzFind.o:   lzma/LzFind.c     $(H_LZMA)
	$(CC) -c $(CF) -o $@ lzma/LzFind.c

LzmaEnc.o:  lzma/LzmaEnc.c    $(H_LZMA)
	$(CC) -c $(CF) -o $@ lzma/LzmaEnc.c


#----------------------------------------------------------------------------

# C Compile rules: PPMd compression

Ppmd8.o:  lzma/Ppmd8.c          $(H_PPMD)
	$(CC) -c $(CF) -o $@ lzma/Ppmd8.c

Ppmd8Enc.o:  lzma/Ppmd8Enc.c    $(H_PPMD)
	$(CC) -c $(CF) -o $@ lzma/Ppmd8Enc.c


#----------------------------------------------------------------------------
#  Build rules: Assemble (.S -> .o)
#  - Optional assembler replacements for C routines
#  - All are port-specific (for instance, msdos rule only used for msdos)
#    - Generated when referenced by OSDEP_OS
#    - OSDEP_OS is customized by unix/configure, based on PORT macro
#----------------------------------------------------------------------------

# Assembler rule: Match

# 1) Assembler rule: Match, 16-bit only, MSDOS
$(O_MCH_I):  msdos/match.asm
	$(AS) $(AF) msdos/match.asm $(ASEOL)

# 2) Assembler rule: Match 32bit, GNU AS
$(O_MCH_G):  match.S $(ACONF_DEP)
	$(AS) $(AF) -x assembler-with-cpp -c -o $@ match.S

# 3) Assembler rule: Match 32bit, SysV AS
$(O_MCH_SV): match.S $(ACONF_DEP)
	$(CPP) $(AF) match.S > _matchs.s
	$(AS) -o $@ matchs.s
	$(RM) matchs.s


#----------------------------------------------------------------------------

# Assembler rule: CRC-32

# 1) Assembler rule: CRC, 16-bit only, MSDOS
$(O_CRC_I):  msdos/crc_i86.asm
	$(AS) $(AF) msdos/crc_i86.asm $(ASEOL)

# 2) Assembler rule: CRC 32bit, GNU AS
$(O_CRC_G):  crc_i386.S $(ACONF_DEP)
	$(AS) $(AF) -x assembler-with-cpp -c -o $@ crc_i386.S

# 3) Assembler rule: CRC 32bit, SysV AS
$(O_CRC_SV): crc_i386.S $(ACONF_DEP)
	$(CPP) $(AF) crc_i386.S > _crc_i386s.s
	$(AS) -o $@ crc_i386s.s
	$(RM) crc_i386s.s


#----------------------------------------------------------------------------
#  Build rules: Object archives (.o -> .a)
#----------------------------------------------------------------------------

# Object archive rule: AES (WinZip/Gladman) encryption

$(AR_AES): $(O_AES)
	@echo 'Building/updating AES (WinZip/Gladman) object library...'
	-$(RM) libaes.a
	$(AR) libaes.a $(O_AES)
	-$(RANLIB) libaes.a


# Object archive rule: LZMA compression

$(AR_LZMA): $(O_LZMA)
	@echo 'Building/updating LZMA object library...'
	-$(RM) liblzma.a
	$(AR) liblzma.a $(O_LZMA)
	-$(RANLIB) liblzma.a


# Object archive rule: PPMd compression

$(AR_PPMD): $(O_PPMD)
	@echo 'Building/updating PPMd object library...'
	-$(RM) libppmd.a
	$(AR) libppmd.a $(O_PPMD)
	-$(RANLIB) libppmd.a


# Object archive rule: BZIP2 compression

$(AR_BZ):
	@echo 'Building/updating BZIP2 object library...'
	@if [ -f flags_bz ]; then \
	         ( cd bzip2/; \
	    echo "Using flags_bz overrides"; \
	    eval $(MAKE) -f $(MK_BZ) libbz2.a ACONF_DEP=flags_bz `cat ../flags_bz`; \
	  ); \
	else \
	  ( cd bzip2/; \
	    eval $(MAKE) -f $(MK_BZ) CC="$(CC_BZ)" CFLAGS="$(CF_BZ)" \
	                 RANLIB="$(RANLIB)" RM="rm -f" libbz2.a; \
	  ); \
	fi


# Object archive rule: ZLIB compression

$(AR_Z):
	@echo 'Building/updating ZLIB object library...'
	( cd zlib/; \
	  eval $(MAKE) -f Makefile libz.a ACONF_DEP=flags `cat ../flags`; \
	)


#----------------------------------------------------------------------------
#  Build rules: Link programs (.o + .a -> zips)
#----------------------------------------------------------------------------

# Link rule: zip

$(PGM_ZIP): $(O_ZIP) $(LIB_AES_WG) $(LIB_BZ) $(LIB_LZMA) \
            $(LIB_PPMD) $(LIB_Z)
	$(LD) -o $@ $(LF1) $(O_ZIP) $(LF2)


# Link rule: zipnote

$(PGM_NOTE): $(O_NOTE)
	$(LD) -o $@ $(LF1) $(O_NOTE) $(LF2)


# Link rule: zipcloak

$(PGM_CLOAK): $(O_CLOAK) $(LIB_AES_WG)
	$(LD) -o $@ $(LF1) $(O_CLOAK) $(LF2)


# Link rule: zipsplit

$(PGM_SPLIT): $(O_SPLIT)
	$(LD) -o $@ $(LF1) $(O_SPLIT) $(LF2)


#----------------------------------------------------------------------------
#  Build rules: Documentation
#  - Explicit 1:1 rules are used
#  - Across various make utilities, pattern rule support is inconsistent
#  - Sys V make has issues with $< when input and output names differ
#----------------------------------------------------------------------------

# Zip documentation output directory rules
# - Creates manout directory if unix/configure not executed

manout:
	mkdir manout


# Zip text documentation generation rules

$(DOC_ZIP):   man/zip.1
	man man/zip.1      | col -bx | uniq | expand > $@

$(DOC_CLOAK): man/zipcloak.1
	man man/zipcloak.1 | col -bx | uniq | expand > $@

$(DOC_NOTE):  man/zipnote.1
	man man/zipnote.1  | col -bx | uniq | expand > $@

$(DOC_SPLIT): man/zipsplit.1
	man man/zipsplit.1 | col -bx | uniq | expand > $@


# Zip man page generation rule
# - Requires nroff, or GNU groff package

$(MAN_ZIP):   man/zip.1
	nroff -man man/zip.1      | col -bx | uniq > $@

$(MAN_CLOAK): man/zipcloak.1
	nroff -man man/zipcloak.1 | col -bx | uniq > $@

$(MAN_NOTE):  man/zipnote.1
	nroff -man man/zipnote.1  | col -bx | uniq > $@

$(MAN_SPLIT): man/zipsplit.1
	nroff -man man/zipsplit.1 | col -bx | uniq > $@


#----------------------------------------------------------------------------
#  Build rules: Standard build targets
#----------------------------------------------------------------------------

all:      generic_msg generic
zips:     $(ZIP_PGMS)
objs:     $(O_ZIP)
docs:     manout $(ZIP_DOCS)
manuals:  manout $(ZIP_MANS)
zipsman:  zips manuals
zipsdoc:  zips docs


#----------------------------------------------------------------------------
#  Build rules: Configuration flags
#  - Creates files containing system-dependent flag values
#  - Generated by unix/configure based on build utility program, platform and
#    features selected
#----------------------------------------------------------------------------

# Main flags

flags: unix/configure
	@echo "Running:  unix/configure"	
	@sh unix/configure \
	        "PROD=${PROD}" \
	        "LIST=${LIST}" \
	        "CC=${CC}" \
	        "CFLAGS=${CF_NOOPT}" \
	        "AES_WG=${AES_WG}" \
	        "LZMA=${LZMA}" \
	        "PPMD=${PPMD}" \
	        "IZ_BZIP2=${IZ_BZIP2}" \
	        "IZ_ZLIB=${IZ_ZLIB}"


# BZIP2 flags

flags_bz: flags


# These symbols, when #defined using -D have these effects on compilation:
# ZMEM                  - includes C language versions of memset(), memcpy(),
#                         and memcmp() (util.c).
# HAVE_DIRENT_H         - use <dirent.h> instead of <sys/dir.h>
# NODIR                 - for 3B1, which has neither getdents() nor opendir().
# HAVE_NDIR_H           - use <ndir.h> (unix/unix.c).
# HAVE_SYS_DIR_H        - use <sys/dir.h>
# HAVE_SYS_NDIR_H       - use <sys/ndir.h>
# UTIL                  - select routines for utilities (note, cloak, split)
# NO_RMDIR              - remove directories using a system("rmdir ...") call.
# NO_PROTO              - cannot handle ANSI prototypes
# NO_CONST              - cannot handle ANSI const
# NO_LARGE_FILE_SUPPORT - do not enable Large File support even if available.
# NO_ZIP64_SUPPORT      - do not enable Zip64 archive support even if available.
# NO_UNICODE_SUPPORT    - do not enable Unicode support even if available.
# NO_BZIP2_SUPPORT      - do not compile in bzip2 code even if available.


#----------------------------------------------------------------------------
#  Build rules: Clean-up targets
#  - Used to remove old output files before performing a new build
#  - Used when incompatible build options (or source changes) are required
#----------------------------------------------------------------------------

clean:
	rm -f *.o *.a $(ZIP_PGMS) flags flags_bz
	rm -rf manout/*
	rm -rf $(PKGDIR)/*

clean_bzip2 :
	@if [ -f bzip2/${MK_BZ} ]; then \
	  ( cd bzip2; make -f ${MK_BZ} clean ); \
	else \
	  echo "No bzip2 make file found: bzip2/${MK_BZ}"; \
	fi;

clean_exe :
	rm -f $(ZIPS_PGMS)

clean_docs:
	rm -f $(ZIP_DOCS)


#----------------------------------------------------------------------------
#  Build rules: Installation-related
#----------------------------------------------------------------------------

install: zips manuals
	-$(INSTALL_D) $(BINDIR)
	$(INSTALL_PROGRAM) $(ZIP_PGMS) $(BINDIR)
	-cd $(BINDIR); $(CHMOD) $(BINPERMS) $(ZIP_PGMS)
	-$(INSTALL_D) $(MANDIR)
	$(INSTALL) $(ZIP_MANS) $(MANDIR)
	$(CHMOD) $(BINPERMS) $(INSTALLEDBIN)
	$(CHMOD) $(MANPERMS) $(INSTALLEDMAN)

uninstall:
	$(RM) $(INSTALLEDBIN) $(INSTALLEDMAN)


#----------------------------------------------------------------------------
#  Build rules: SVR4 Package generation Interface
#  - Originally tested under Sun Solaris 2.x (by JBush)
#  - Other SVr4s may be very similar, and could possibly use this
#  Note:  Expects version info to be stored in VERSION macro variable
#         See "README" under ./unix/Packaging
#----------------------------------------------------------------------------

svr4package: zips
	@echo "Creating SVR4 package for Unix ..."
	-@rm -rf ./$(PKGDIR) ./$(PKGDIR)_`uname -p`.pkg
	-@sed -e "s/.VERSION./$(VERSION)/g" \
	      -e "s/.PSTAMP./$(LOGNAME)_`date | tr  ' ' '_'`/g" \
	      -e "s/.ARCH./Solaris_`uname -rp | tr ' ' ','`/g" \
	      ./unix/Packaging/pkginfo.in > ./unix/Packaging/pkginfo
	-@sed -e "s/.ARCH./`uname -p`/g" \
	      ./unix/Packaging/preinstall.in > ./unix/Packaging/preinstall
	/usr/bin/pkgmk -d . -b . -r . -f ./unix/Packaging/prototype $(PKGDIR)
	/usr/bin/pkgtrans -o -s . $(PKGDIR)_`uname -p`.pkg $(PKGDIR)
	@echo " "
	@echo "To install, copy $(PKGDIR)_`uname -p`.pkg to the target system, and"
	@echo "issue the command (as root):  pkgadd -d $(PKGDIR)_`uname -p`.pkg"
	@echo " "


#----------------------------------------------------------------------------
#  Build rules: Make a distribution
#----------------------------------------------------------------------------

dist: $(DOC_ZIP)
	eval zip -r9 zip`sed -e '/VERSION/!d' -e 's/.*"\(.*\)".*/\1/' \
	         -e 's/[.]//g' -e 's/ .*//g' -e q revision.h` *


################################
# INDIVIDUAL MACHINE MAKERULES #
################################

#----------------------------------------------------------------------------
#  Generic targets
#  - These invoke the configure script to determine configuration.
#----------------------------------------------------------------------------

# generic_msg
# - Issue warning message before generic build

generic_msg:
	@echo ''
	@echo '  Attempting "make generic" now.  If this fails for some reason, type'
	@echo '  "make help" for suggestions.  If none of the targets work for you,'
	@echo '  contact us using the information in README so we can work the'
	@echo '  problems.'
	@echo ''


# generic
# - Generate flags file (if doesn't exist), then Zip modules

generic: flags
	eval $(MAKE) $(MAKEF) zips PROD="$(PROD)" `cat flags`


# generic_man
# - Generate flags file (if doesn't exist), then man pages

generic_man: flags
	eval $(MAKE) $(MAKEF) manuals PROD="$(PROD)" `cat flags`


# generic_docs
# - Generate flags file (if doesn't exist), then plain text docs

generic_docs: flags
	eval $(MAKE) $(MAKEF) docs ACONF_DEP=flags`cat flags`


# generic_pkg
# - Generate flags file (if doesn't exist), then Zip modules, then perform
#   SVR4 package-creation

generic_pkg: generic svr4package


#----------------------------------------------------------------------------
#  Old static generic targets
#  - For those systems which do not work with the generic targets.
#  - Can not assume that the make utility groks "$(MAKE)"
#----------------------------------------------------------------------------

# AT&T 6300 PLUS

# - Do not know yet how to allocate 64K bytes:
att6300nodir:
	$(MAKE) $(MAKEF) zips LF1="-Ml -s" \
	CF="-DUNIX -I. -O -Ml -DNO_RMDIR -DDYN_ALLOC -DMEDIUM_MEM \
-DWSIZE=16384 -DNO_STDLIB_H -DNO_STDDEF_H -DNO_RENAME \
-DNO_MKTIME -DNO_SIZE_T -DNO_VOID -DNO_PROTO -DNO_DIR \
-DNO_CONST -DHAVE_TERMIO_H" \
	"LF2="


# Coherent

# - AS definition not needed for gcc
coherent:
	$(MAKE) $(MAKEF) zips CF="-DUNIX -I. -O -DDIRENT -DASMV" \
	 AS="as -gx" OBJA=match.o


# Cray Unicos 6.1, Standard C compiler 3.0
# - All routines except trees.c may be compiled with vector3
# - Internal compiler bug in 3.0.2.3 and earlier requires vector2 for trees.c

cray_v3:
	$(MAKE) $(MAKEF) zips CC="scc" \
	            CF="-DUNIX -I. -O -h vector2 -h scalar3 -DHAVE_DIRENT_H"


# Cygwin

cygwin:
	$(MAKE) $(MAKEF) generic CC="gcc" CPP="gcc -E" PGMEXE=".exe"


# LynxOS

lynx:
	$(MAKE) $(MAKEF) generic CC=gcc CPP="gcc -E" CF="$(CF) \
	 -DNO_UNDERLINE -DLynx -DLYNX LF2="$LF2 -lc_p"


# MINIX 1.5.10
# - With Bruce Evans 386 patches and gcc/GNU make

minix:
	$(MAKE) $(MAKEF) zips CC=gcc CF="-DUNIX -I. -O -DDIRENT -DMINIX"
	chmem =262144 zip


# QNX
# - /bin/sh is ksh and it doesn't grok the configure script properly,
#   generating a bad flags file.  [cjh]

qnx:
	$(MAKE) $(MAKEF) zips LN=ln CC=cc CF="-DUNIX -I. -O \
	 -DHAVE_DIRENT_H -DHAVE_TERMIOS_H -DNO_MKTEMP"


# QNX/Neutrino
# - No native development tools, so cross-build environement is used.
# - Use gcc and set ARCH as follows:
#      "x86"    to produce x86
#      "ppcbe"             PowerPC (big-endian)
#      "ppcle"             PowerPC (little-endian)
#      "mipsbe"            MIPS    (big-endian)
#      "mipsle"            MIPS    (little-endian)

qnxnto:
	@if [ "$(ARCH)" = "" ] ; then \
	  echo "You didn't set ARCH; I'll assume you meant ARCH=x86..." ; \
	  echo "" ; \
	  $(MAKE) $(MAKEF) zips LN=ln CC="qcc -Vgcc_ntox86" \
	  CF="-g -DUNIX -I. -O -DHAVE_DIRENT_H -DHAVE_TERMIOS_H -DNO_MKTEMP" \
	  LF2=-g ; \
	else \
	  echo "Making Zip for $(ARCH)..." ; \
	  echo "" ; \
	  $(MAKE) $(MAKEF) zips LN=ln CC="qcc -Vgcc_nto$(ARCH)" \
	  CF="-g -DUNIX -I. -O -DHAVE_DIRENT_H -DHAVE_TERMIOS_H -DNO_MKTEMP" \
	  LF2=-g ; \
	fi


# Solaris
# - Generic unix build, plus generation of installable package.

solaris: generic svr4package
