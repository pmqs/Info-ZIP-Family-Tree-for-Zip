#
# Zip for VMS - MMS (or MMK) Source Dependency File.
#

# This description file is included by other description files.  It is
# not intended to be used alone.  Verify proper inclusion.

.IFDEF INCL_DESCRIP_DEPS
.ELSE
$$$$ THIS DESCRIPTION FILE IS NOT INTENDED TO BE USED THIS WAY.
.ENDIF

[.$(DEST)]CRC32.OBJ : []CRC32.C
[.$(DEST)]CRC32.OBJ : []ZIP.H
[.$(DEST)]CRC32.OBJ : []TAILOR.H
[.$(DEST)]CRC32.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]CRC32.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]CRC32.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]CRC32.OBJ : []ZIPERR.H
[.$(DEST)]CRC32.OBJ : []CRC32.H
[.$(DEST)]CRYPT.OBJ : []CRYPT.C
[.$(DEST)]CRYPT.OBJ : []ZIP.H
[.$(DEST)]CRYPT.OBJ : []TAILOR.H
[.$(DEST)]CRYPT.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]CRYPT.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]CRYPT.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]CRYPT.OBJ : []ZIPERR.H
[.$(DEST)]CRYPT.OBJ : []CRYPT.H
[.$(DEST)]CRYPT.OBJ : []TTYIO.H
[.$(DEST)]CRYPT.OBJ : []CRC32.H
[.$(DEST)]CRYPT_.OBJ : []CRYPT.C
[.$(DEST)]CRYPT_.OBJ : []ZIP.H
[.$(DEST)]CRYPT_.OBJ : []TAILOR.H
[.$(DEST)]CRYPT_.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]AES.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]CRYPT_.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]CRYPT_.OBJ : []ZIPERR.H
[.$(DEST)]CRYPT_.OBJ : []CRYPT.H
[.$(DEST)]CRYPT_.OBJ : []TTYIO.H
[.$(DEST)]CRYPT_.OBJ : []CRC32.H
[.$(DEST)]DEFLATE.OBJ : []DEFLATE.C
[.$(DEST)]DEFLATE.OBJ : []ZIP.H
[.$(DEST)]DEFLATE.OBJ : []TAILOR.H
[.$(DEST)]DEFLATE.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]AES.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]AES.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]DEFLATE.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]DEFLATE.OBJ : []ZIPERR.H
[.$(DEST)]FILEIO.OBJ : []FILEIO.C
[.$(DEST)]FILEIO.OBJ : []ZIP.H
[.$(DEST)]FILEIO.OBJ : []TAILOR.H
[.$(DEST)]FILEIO.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]FILEIO.OBJ : [.AES_WG]AES.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]AES.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]FILEIO.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]FILEIO.OBJ : []ZIPERR.H
[.$(DEST)]FILEIO.OBJ : []CRC32.H
[.$(DEST)]FILEIO.OBJ : [.VMS]VMS.H
[.$(DEST)]FILEIO_.OBJ : []FILEIO.C
[.$(DEST)]FILEIO_.OBJ : []ZIP.H
[.$(DEST)]FILEIO_.OBJ : []TAILOR.H
[.$(DEST)]FILEIO_.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]AES.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]AES.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]FILEIO_.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]FILEIO_.OBJ : []ZIPERR.H
[.$(DEST)]FILEIO_.OBJ : []CRC32.H
[.$(DEST)]FILEIO_.OBJ : [.VMS]VMS.H
[.$(DEST)]GLOBALS.OBJ : []GLOBALS.C
[.$(DEST)]GLOBALS.OBJ : []ZIP.H
[.$(DEST)]GLOBALS.OBJ : []TAILOR.H
[.$(DEST)]GLOBALS.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]AES.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]AES.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]GLOBALS.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]GLOBALS.OBJ : []ZIPERR.H
[.$(DEST)]TREES.OBJ : []TREES.C
[.$(DEST)]TREES.OBJ : []ZIP.H
[.$(DEST)]TREES.OBJ : []TAILOR.H
[.$(DEST)]TREES.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]TREES.OBJ : [.AES_WG]AES.H
[.$(DEST)]TREES.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]TREES.OBJ : [.AES_WG]AES.H
[.$(DEST)]TREES.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]TREES.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]TREES.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]TREES.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]TREES.OBJ : []ZIPERR.H
[.$(DEST)]TTYIO.OBJ : []TTYIO.C
[.$(DEST)]TTYIO.OBJ : []ZIP.H
[.$(DEST)]TTYIO.OBJ : []TAILOR.H
[.$(DEST)]TTYIO.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]TTYIO.OBJ : [.AES_WG]AES.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]AES.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]TTYIO.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]TTYIO.OBJ : []ZIPERR.H
[.$(DEST)]TTYIO.OBJ : []CRYPT.H
[.$(DEST)]TTYIO.OBJ : []TTYIO.H
[.$(DEST)]UTIL.OBJ : []UTIL.C
[.$(DEST)]UTIL.OBJ : []ZIP.H
[.$(DEST)]UTIL.OBJ : []TAILOR.H
[.$(DEST)]UTIL.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]UTIL.OBJ : [.AES_WG]AES.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]AES.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]UTIL.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]UTIL.OBJ : []ZIPERR.H
[.$(DEST)]UTIL.OBJ : []EBCDIC.H
[.$(DEST)]UTIL_.OBJ : []UTIL.C
[.$(DEST)]UTIL_.OBJ : []ZIP.H
[.$(DEST)]UTIL_.OBJ : []TAILOR.H
[.$(DEST)]UTIL_.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]UTIL_.OBJ : [.AES_WG]AES.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]AES.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]UTIL_.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]UTIL_.OBJ : []ZIPERR.H
[.$(DEST)]UTIL_.OBJ : []EBCDIC.H
[.$(DEST)]ZBZ2ERR.OBJ : []ZBZ2ERR.C
[.$(DEST)]ZBZ2ERR.OBJ : []ZIP.H
[.$(DEST)]ZBZ2ERR.OBJ : []TAILOR.H
[.$(DEST)]ZBZ2ERR.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZBZ2ERR.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZBZ2ERR.OBJ : []ZIPERR.H
[.$(DEST)]ZIP.OBJ : []ZIP.C
[.$(DEST)]ZIP.OBJ : []ZIP.H
[.$(DEST)]ZIP.OBJ : []TAILOR.H
[.$(DEST)]ZIP.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIP.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIP.OBJ : []ZIPERR.H
[.$(DEST)]ZIP.OBJ : []REVISION.H
[.$(DEST)]ZIP.OBJ : []CRC32.H
[.$(DEST)]ZIP.OBJ : []CRYPT.H
[.$(DEST)]ZIP.OBJ : []TTYIO.H
[.$(DEST)]ZIP.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]ZIP.OBJ : [.VMS]VMS.H
.IFDEF AES_WG
[.$(DEST)]ZIP.OBJ : [.AES_WG]AESOPT.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]BRG_ENDIAN.H
[.$(DEST)]ZIP.OBJ : [.AES_WG]IZ_AES_WG.H
.ENDIF # AES_WG
.IFDEF LZMA
[.$(DEST)]ZIP.OBJ : [.LZMA]SZVERSION.H
.ENDIF # LZMA
[.$(DEST)]ZIPCLI.OBJ : []ZIP.C
[.$(DEST)]ZIPCLI.OBJ : []ZIP.H
[.$(DEST)]ZIPCLI.OBJ : []TAILOR.H
[.$(DEST)]ZIPCLI.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPCLI.OBJ : []ZIPERR.H
[.$(DEST)]ZIPCLI.OBJ : []REVISION.H
[.$(DEST)]ZIPCLI.OBJ : []CRC32.H
[.$(DEST)]ZIPCLI.OBJ : []CRYPT.H
[.$(DEST)]ZIPCLI.OBJ : []TTYIO.H
[.$(DEST)]ZIPCLI.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]ZIPCLI.OBJ : [.VMS]VMS.H
.IFDEF AES_WG
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]AESOPT.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]BRG_ENDIAN.H
[.$(DEST)]ZIPCLI.OBJ : [.AES_WG]IZ_AES_WG.H
.ENDIF # AES_WG
.IFDEF LZMA
[.$(DEST)]ZIPCLI.OBJ : [.LZMA]SZVERSION.H
.ENDIF # LZMA
[.$(DEST)]ZIPCLOAK.OBJ : []ZIPCLOAK.C
[.$(DEST)]ZIPCLOAK.OBJ : []ZIP.H
[.$(DEST)]ZIPCLOAK.OBJ : []TAILOR.H
[.$(DEST)]ZIPCLOAK.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPCLOAK.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPCLOAK.OBJ : []ZIPERR.H
[.$(DEST)]ZIPCLOAK.OBJ : []REVISION.H
[.$(DEST)]ZIPCLOAK.OBJ : []CRC32.H
[.$(DEST)]ZIPCLOAK.OBJ : []CRYPT.H
[.$(DEST)]ZIPCLOAK.OBJ : []TTYIO.H
[.$(DEST)]ZIPFILE.OBJ : []ZIPFILE.C
[.$(DEST)]ZIPFILE.OBJ : []ZIP.H
[.$(DEST)]ZIPFILE.OBJ : []TAILOR.H
[.$(DEST)]ZIPFILE.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPFILE.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPFILE.OBJ : []ZIPERR.H
[.$(DEST)]ZIPFILE.OBJ : []REVISION.H
[.$(DEST)]ZIPFILE.OBJ : [.VMS]VMS.H
[.$(DEST)]ZIPFILE.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]ZIPFILE.OBJ : [.VMS]VMSDEFS.H
[.$(DEST)]ZIPFILE_.OBJ : []ZIPFILE.C
[.$(DEST)]ZIPFILE_.OBJ : []ZIP.H
[.$(DEST)]ZIPFILE_.OBJ : []TAILOR.H
[.$(DEST)]ZIPFILE_.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPFILE_.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPFILE_.OBJ : []ZIPERR.H
[.$(DEST)]ZIPFILE_.OBJ : []REVISION.H
[.$(DEST)]ZIPFILE_.OBJ : [.VMS]VMS.H
[.$(DEST)]ZIPFILE_.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]ZIPFILE_.OBJ : [.VMS]VMSDEFS.H
[.$(DEST)]ZIPNOTE.OBJ : []ZIPNOTE.C
[.$(DEST)]ZIPNOTE.OBJ : []ZIP.H
[.$(DEST)]ZIPNOTE.OBJ : []TAILOR.H
[.$(DEST)]ZIPNOTE.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPNOTE.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPNOTE.OBJ : []ZIPERR.H
[.$(DEST)]ZIPNOTE.OBJ : []REVISION.H
[.$(DEST)]ZIPSPLIT.OBJ : []ZIPSPLIT.C
[.$(DEST)]ZIPSPLIT.OBJ : []ZIP.H
[.$(DEST)]ZIPSPLIT.OBJ : []TAILOR.H
[.$(DEST)]ZIPSPLIT.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPSPLIT.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPSPLIT.OBJ : []ZIPERR.H
[.$(DEST)]ZIPSPLIT.OBJ : []REVISION.H
[.$(DEST)]ZIPUP.OBJ : []ZIPUP.C
[.$(DEST)]ZIPUP.OBJ : []ZIP.H
[.$(DEST)]ZIPUP.OBJ : []TAILOR.H
[.$(DEST)]ZIPUP.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]AES.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]ZIPUP.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]ZIPUP.OBJ : []ZIPERR.H
[.$(DEST)]ZIPUP.OBJ : []REVISION.H
[.$(DEST)]ZIPUP.OBJ : []CRC32.H
[.$(DEST)]ZIPUP.OBJ : []CRYPT.H
[.$(DEST)]ZIPUP.OBJ : [.VMS]ZIPUP.H
.IFDEF LZMA
[.$(DEST)]ZIPUP.OBJ : [.LZMA]ALLOC.H
[.$(DEST)]ZIPUP.OBJ : [.LZMA]SZFILE.H
.ENDIF # LZMA
[.$(DEST)]ZIPUP.OBJ : []ZIP.H
.IFDEF LZMA
[.$(DEST)]ZIPUP.OBJ : [.LZMA]TYPES.H
[.$(DEST)]ZIPUP.OBJ : [.LZMA]SZVERSION.H
[.$(DEST)]ZIPUP.OBJ : [.LZMA]LZMADEC.H
[.$(DEST)]ZIPUP.OBJ : [.LZMA]LZMAENC.H
.ENDIF # LZMA
.IFDEF AES_WG
[.$(DEST)]AESCRYPT.OBJ : [.AES_WG]AESCRYPT.C
[.$(DEST)]AESCRYPT.OBJ : [.AES_WG]AESOPT.H
[.$(DEST)]AESCRYPT.OBJ : [.AES_WG]AES.H
[.$(DEST)]AESCRYPT.OBJ : [.AES_WG]BRG_ENDIAN.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]AESKEY.OBJ : [.AES_WG]AESKEY.C
[.$(DEST)]AESKEY.OBJ : [.AES_WG]AESOPT.H
[.$(DEST)]AESKEY.OBJ : [.AES_WG]AES.H
[.$(DEST)]AESKEY.OBJ : [.AES_WG]BRG_ENDIAN.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]AESTAB.OBJ : [.AES_WG]AESTAB.C
[.$(DEST)]AESTAB.OBJ : [.AES_WG]AESOPT.H
[.$(DEST)]AESTAB.OBJ : [.AES_WG]AES.H
[.$(DEST)]AESTAB.OBJ : [.AES_WG]BRG_ENDIAN.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]FILEENC.OBJ : [.AES_WG]FILEENC.C
[.$(DEST)]FILEENC.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]FILEENC.OBJ : [.AES_WG]AES.H
[.$(DEST)]FILEENC.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]FILEENC.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]FILEENC.OBJ : [.AES_WG]PWD2KEY.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]HMAC.OBJ : [.AES_WG]HMAC.C
[.$(DEST)]HMAC.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]HMAC.OBJ : [.AES_WG]SHA1.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]PRNG.OBJ : [.AES_WG]PRNG.C
[.$(DEST)]PRNG.OBJ : [.AES_WG]PRNG.H
[.$(DEST)]PRNG.OBJ : [.AES_WG]SHA1.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]PWD2KEY.OBJ : [.AES_WG]PWD2KEY.C
[.$(DEST)]PWD2KEY.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]PWD2KEY.OBJ : [.AES_WG]SHA1.H
.ENDIF # AES_WG
.IFDEF AES_WG
[.$(DEST)]SHA1.OBJ : [.AES_WG]SHA1.C
[.$(DEST)]SHA1.OBJ : [.AES_WG]BRG_ENDIAN.H
[.$(DEST)]SHA1.OBJ : [.AES_WG]SHA1.H
.ENDIF # AES_WG
.IFDEF LZMA
[.$(DEST)]ALLOC.OBJ : [.LZMA]ALLOC.C
[.$(DEST)]ALLOC.OBJ : [.LZMA]ALLOC.H
.ENDIF # LZMA
.IFDEF LZMA
[.$(DEST)]LZFIND.OBJ : [.LZMA]LZFIND.C
[.$(DEST)]LZFIND.OBJ : [.LZMA]LZFIND.H
[.$(DEST)]LZFIND.OBJ : [.LZMA]TYPES.H
[.$(DEST)]LZFIND.OBJ : [.LZMA]LZHASH.H
.ENDIF # LZMA
.IFDEF LZMA
[.$(DEST)]LZMAENC.OBJ : [.LZMA]LZMAENC.C
.ENDIF # LZMA
[.$(DEST)]LZMAENC.OBJ : []ZIP.H
[.$(DEST)]LZMAENC.OBJ : []TAILOR.H
[.$(DEST)]LZMAENC.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]AES.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]AES.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]LZMAENC.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]LZMAENC.OBJ : []ZIPERR.H
.IFDEF LZMA
[.$(DEST)]LZMAENC.OBJ : [.LZMA]LZMAENC.H
[.$(DEST)]LZMAENC.OBJ : [.LZMA]TYPES.H
[.$(DEST)]LZMAENC.OBJ : [.LZMA]LZFIND.H
.ENDIF # LZMA
.IFDEF LZMA
[.$(DEST)]SZFILE.OBJ : [.LZMA]SZFILE.C
.ENDIF # LZMA
[.$(DEST)]SZFILE.OBJ : []ZIP.H
[.$(DEST)]SZFILE.OBJ : []TAILOR.H
[.$(DEST)]SZFILE.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]SZFILE.OBJ : [.AES_WG]AES.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]AES.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]SZFILE.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]SZFILE.OBJ : []ZIPERR.H
[.$(DEST)]SZFILE.OBJ : []CRYPT.H
.IFDEF LZMA
[.$(DEST)]SZFILE.OBJ : [.LZMA]SZFILE.H
[.$(DEST)]SZFILE.OBJ : [.LZMA]TYPES.H
.ENDIF # LZMA
[.$(DEST)]CMDLINE.OBJ : [.VMS]CMDLINE.C
[.$(DEST)]CMDLINE.OBJ : []ZIP.H
[.$(DEST)]CMDLINE.OBJ : []TAILOR.H
[.$(DEST)]CMDLINE.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]AES.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]AES.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]CMDLINE.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]CMDLINE.OBJ : []ZIPERR.H
[.$(DEST)]CMDLINE.OBJ : []CRYPT.H
[.$(DEST)]CMDLINE.OBJ : []REVISION.H
[.$(DEST)]VMS.OBJ : [.VMS]VMS.C
[.$(DEST)]VMS.OBJ : []ZIP.H
[.$(DEST)]VMS.OBJ : []TAILOR.H
[.$(DEST)]VMS.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]VMS.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMS.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]VMS.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMS.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]VMS.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]VMS.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]VMS.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]VMS.OBJ : []ZIPERR.H
[.$(DEST)]VMS.OBJ : [.VMS]ZIPUP.H
[.$(DEST)]VMS.OBJ : [.VMS]VMS_PK.C
[.$(DEST)]VMS.OBJ : []CRC32.H
[.$(DEST)]VMS.OBJ : [.VMS]VMS.H
[.$(DEST)]VMS.OBJ : [.VMS]VMSDEFS.H
[.$(DEST)]VMS.OBJ : [.VMS]VMS_IM.C
[.$(DEST)]VMSMUNCH.OBJ : [.VMS]VMSMUNCH.C
[.$(DEST)]VMSMUNCH.OBJ : []ZIP.H
[.$(DEST)]VMSMUNCH.OBJ : []TAILOR.H
[.$(DEST)]VMSMUNCH.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]VMSMUNCH.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]VMSMUNCH.OBJ : []ZIPERR.H
[.$(DEST)]VMSMUNCH.OBJ : [.VMS]VMS.H
[.$(DEST)]VMSMUNCH.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]VMSMUNCH.OBJ : [.VMS]VMSDEFS.H
[.$(DEST)]VMSZIP.OBJ : [.VMS]VMSZIP.C
[.$(DEST)]VMSZIP.OBJ : []ZIP.H
[.$(DEST)]VMSZIP.OBJ : []TAILOR.H
[.$(DEST)]VMSZIP.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]VMSZIP.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]VMSZIP.OBJ : []ZIPERR.H
[.$(DEST)]VMSZIP.OBJ : [.VMS]VMSMUNCH.H
[.$(DEST)]VMSZIP.OBJ : [.VMS]VMS.H
[.$(DEST)]VMS_.OBJ : [.VMS]VMS.C
[.$(DEST)]VMS_.OBJ : []ZIP.H
[.$(DEST)]VMS_.OBJ : []TAILOR.H
[.$(DEST)]VMS_.OBJ : [.VMS]OSDEP.H
.IFDEF AES_WG
[.$(DEST)]VMS_.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]FILEENC.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]AES.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]HMAC.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]SHA1.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]PWD2KEY.H
[.$(DEST)]VMS_.OBJ : [.AES_WG]PRNG.H
.ENDIF # AES_WG
[.$(DEST)]VMS_.OBJ : []ZIPERR.H
[.$(DEST)]VMS_.OBJ : [.VMS]ZIPUP.H
[.$(DEST)]VMS_.OBJ : [.VMS]VMS_PK.C
[.$(DEST)]VMS_.OBJ : []CRC32.H
[.$(DEST)]VMS_.OBJ : [.VMS]VMS.H
[.$(DEST)]VMS_.OBJ : [.VMS]VMSDEFS.H
[.$(DEST)]VMS_.OBJ : [.VMS]VMS_IM.C
