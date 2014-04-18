/*
  zipup.c - Zip 3

  Copyright (c) 1990-2013 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-2 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  zipup.c by Mark Adler and Jean-loup Gailly.
 */
#define __ZIPUP_C

/* Found that for at least unix port zip.h has to be first or ctype.h will
   define off_t and when using 64-bit file environment off_t in other files
   is 8 bytes while off_t here is 4 bytes, and this makes the zlist struct
   different sizes and needless to say leads to segmentation faults.  Putting
   zip.h first seems to fix this.  8/14/04 EG */
#include "zip.h"
#include <ctype.h>
#include <errno.h>

#ifndef UTIL            /* This module contains no code for Zip Utilities */

#include "revision.h"
#include "crc32.h"
#include "crypt.h"
#ifdef USE_ZLIB
#  include "zlib.h"
#endif
#ifdef BZIP2_SUPPORT
#  ifdef BZIP2_USEBZIP2DIR
#    include "bzip2/bzlib.h"
#  else
#    include "bzlib.h"
#  endif
#endif

#ifdef OS2
#  include "os2/os2zip.h"
#endif

#if defined(MMAP)
#  include <sys/mman.h>
#  ifndef PAGESIZE   /* used to be SYSV, what about pagesize on SVR3 ? */
#    define PAGESIZE getpagesize()
#  endif
#  if defined(NO_VALLOC) && !defined(valloc)
#    define valloc malloc
#  endif
#endif

/* Use the raw functions for MSDOS and Unix to save on buffer space.
   They're not used for VMS since it doesn't work (raw is weird on VMS).
 */

#ifdef AMIGA
#  include "amiga/zipup.h"
#endif /* AMIGA */

#ifdef AOSVS
#  include "aosvs/zipup.h"
#endif /* AOSVS */

#ifdef ATARI
#  include "atari/zipup.h"
#endif

#ifdef __BEOS__
#  include "beos/zipup.h"
#endif

#ifdef __ATHEOS__
#  include "atheos/zipup.h"
#endif /* __ATHEOS__ */

#ifdef __human68k__
#  include "human68k/zipup.h"
#endif /* __human68k__ */

#ifdef MACOS
#  include "macos/zipup.h"
#endif

#ifdef DOS
#  include "msdos/zipup.h"
#endif /* DOS */

#ifdef NLM
#  include "novell/zipup.h"
#  include <nwfattr.h>
#endif

#ifdef OS2
#  include "os2/zipup.h"
#endif /* OS2 */

#ifdef RISCOS
#  include "acorn/zipup.h"
#endif

#ifdef TOPS20
#  include "tops20/zipup.h"
#endif

#ifdef UNIX
#  include "unix/zipup.h"
#  ifdef __APPLE__
#    include "unix/macosx.h"
#  endif /* def __APPLE__ */
#endif

#ifdef CMS_MVS
#  include "zipup.h"
#endif /* CMS_MVS */

#ifdef TANDEM
#  include "zipup.h"
#endif /* TANDEM */

#ifdef VMS
#  include "vms/zipup.h"
#endif /* VMS */

#ifdef QDOS
#  include "qdos/zipup.h"
#endif /* QDOS */

#ifdef WIN32
#  include "win32/zipup.h"
#endif

#ifdef THEOS
#  include "theos/zipup.h"
#endif

/* Local functions */
#ifndef RISCOS
   local int suffixes OF((char *, char *));
#else
   local int filetypes OF((char *, char *));
#endif
local unsigned iz_file_read OF((char *buf, unsigned size));
#ifdef USE_ZLIB
  local int zl_deflate_init OF((int pack_level));
#else /* !USE_ZLIB */
# ifdef ZP_NEED_MEMCOMPR
    local unsigned mem_read OF((char *buf, unsigned size));
# endif
#endif /* ?USE_ZLIB */

#ifdef IZ_CRYPT_AES_WG
# include "aes_wg/prng.h"
#endif

#ifdef IZ_CRYPT_AES_WG_NEW
# include "aesnew/ccm.h"
#endif

/* zip64 support 08/29/2003 R.Nausedat */
local zoff_t filecompress OF((struct zlist far *z_entry, int *cmpr_method));

#ifdef BZIP2_SUPPORT
local zoff_t bzfilecompress OF((struct zlist far *z_entry, int *cmpr_method));
#endif

#if defined( LZMA_SUPPORT) || defined( PPMD_SUPPORT)

#include "szip/Types.h"

static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

#endif /* defined( LZMA_SUPPORT) || defined( PPMD_SUPPORT) */

#ifdef LZMA_SUPPORT

# ifdef WIN32
#  define _CRT_SECURE_NO_WARNINGS
# endif /* def WIN32 */

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "szip/SzVersion.h"
# include "szip/LzmaEnc.h"

 const char *kCantReadMessage = "Can not read input file";
 const char *kCantWriteMessage = "Can not write output file";
 const char *kCantAllocateMessage = "Can not allocate memory";
 const char *kDataErrorMessage = "Data error";

 local zoff_t lzma_filecompress OF((struct zlist far *z_entry,
  int *cmpr_method));

#endif

#ifdef PPMD_SUPPORT

# include "szip/Ppmd8.h"

 local zoff_t ppmd_filecompress OF((struct zlist far *z_entry,
  int *cmpr_method));

#endif /* def PPMD_SUPPORT */


/* Deflate "internal" global data (currently not in zip.h) */
#if defined(MMAP) || defined(BIG_MEM)
# ifdef USE_ZLIB
    local uch *window = NULL;   /* Used to read all input file at once */
    local ulg window_size;      /* size of said window */
# else /* !USE_ZLIB */
    extern uch *window;         /* Used to read all input file at once */
#endif /* ?USE_ZLIB */
#endif /* MMAP || BIG_MEM */
#ifndef USE_ZLIB
  extern ulg window_size;       /* size of said window */

  unsigned (*read_buf) OF((char *buf, unsigned size)) = iz_file_read;
  /* Current input function. Set to mem_read for in-memory compression */
#endif /* !USE_ZLIB */


/* Local data */
local ulg crc;                  /* crc on uncompressed file data */
local ftype ifile;              /* file to compress */
#if defined(MMAP) || defined(BIG_MEM)
  local ulg remain;
  /* window bytes not yet processed.
   *  special value "(ulg)-1L" reserved to signal normal reads.
   */
#endif /* MMAP || BIG_MEM */

/* Show-what-doing compression level message storage. */
local char l_str[ 4];
local char *l_str_p;


/* 2012-02-07 SMS.
 * Pulled these declarations out of bzip2- and zlib-specific blocks into
 * this once-for-all set.  Changed the type from char to unsigned char,
 * to accommodate the PPMd8 code, where signed char causes trouble.
 */
#if defined( USE_ZLIB) || defined( BZIP2_SUPPORT) || defined( PPMD_SUPPORT)
# define NEED_IO_BUFFERS
#endif

# ifdef NEED_IO_BUFFERS
local unsigned char *f_ibuf = NULL;
local unsigned char *f_obuf = NULL;
#endif /* def NEED_IO_BUFFERS */

#ifdef USE_ZLIB
  local int deflInit = FALSE;   /* flag: zlib deflate is initialized */
  local z_stream zstrm;         /* zlib's data interface structure */
#else /* !USE_ZLIB */
  local char file_outbuf[1024]; /* output buffer for compression to file */

# ifdef ZP_NEED_MEMCOMPR
    local char *in_buf;
    /* Current input buffer, in_buf is used only for in-memory compression. */
    local unsigned in_offset;
    /* Current offset in input buffer. in_offset is used only for in-memory
     * compression. On 16 bit machines, the buffer is limited to 64K.
     */
    local unsigned in_size;     /* size of current input buffer */
# endif /* ZP_NEED_MEMCOMPR */
#endif /* ?USE_ZLIB */

#if defined( UNIX) && defined( __APPLE__)
    char btrbslash;             /* Saved character had better be a slash. */
    unsigned char *file_read_fake_buf;
    size_t file_read_fake_len;
    local int translate_eol_lcl;

   /* Use special AppleDouble-modified "-l[l]" flag. */
#  define TRANSLATE_EOL translate_eol_lcl
#else
   /* Use normal (unmodified) "-l[l]" flag. */
#  define TRANSLATE_EOL translate_eol
#endif /* defined( UNIX) && defined( __APPLE__) */

#ifdef BZIP2_SUPPORT
    local int bzipInit;         /* flag: bzip2lib is initialized */
    local bz_stream bstrm;      /* zlib's data interface structure */
#endif /* BZIP2_SUPPORT */

#ifdef DEBUG
    zoff_t isize;               /* input file size. global only for debugging */
#else /* !DEBUG */
    local zoff_t isize;         /* input file size. global only for debugging */
#endif /* ?DEBUG */
  /* If iz_file_read detects binary it sets this flag - 12/16/04 EG */
  local int file_binary = 0;        /* first buf */
  /* iz_file_read_bt() sets this one.   2012-02-23 SMS. */
  local int file_binary_final = 0;  /* for bzip2 for entire file.  assume text until find binary */


/* moved check to function 3/14/05 EG */
int is_seekable(y)
  FILE *y;
{
  zoff_t pos;

#ifdef BROKEN_FSEEK
  if (!fseekable(y)) {
    return 0;
  }
#endif

  /* 2013-10-16 SMS.
   * "zip - stuff >> archive.zip" fails (corrupt archive) because the
   * seek-tell tests below misleadingly succeed.  Thus, if possible, we
   * check for append access, and, if true, conclude non-seekable.
   */
#ifdef O_APPEND
  {
    int sts;

    sts = fcntl( fileno( y), F_GETFL);  /* Get flags and access modes. */
    if ((sts != -1) && (sts& O_APPEND))
    {
      return 0;                 /* fcntl() succeeded, and O_APPEND is set. */
    }
  }
#endif /* def O_APPEND */

  pos = zftello(y);
  if (zfseeko(y, pos, SEEK_SET)) {
    return 0;
  }

  return 1;
}


int percent(n, m)
  uzoff_t n;
  uzoff_t m;                    /* n is the original size, m is the new size */
/* Return the percentage compression from n to m using only integer
   operations */
{
  zoff_t p;

#if 0
  if (n > 0xffffffL)            /* If n >= 16M */
  {                             /*  then divide n and m by 256 */
    n += 0x80;  n >>= 8;
    m += 0x80;  m >>= 8;
  }
  return n > m ? (int)(1 + (200 * (n - m)/n)) / 2 : 0;
#endif

/* 2004-12-01 SMS.
 * Changed to do big-n test only for small zoff_t.
 * Changed big-n arithmetic to accomodate apparently negative values
 * when a small zoff_t value exceeds 2G.
 * Increased the reduction divisor from 256 to 512 to avoid the sign bit
 * in a reduced intermediate, allowing signed arithmetic for the final
 * result (which is no longer artificially limited to non-negative
 * values).
 * Note that right shifts must be on unsigned values to avoid undesired
 * sign extension.
 */

/* Handle n = 0 case and account for int maybe being 16-bit.  12/28/2004 EG
 */

#define PC_MAX_SAFE 0x007fffffL    /* 9 clear bits at high end. */
#define PC_MAX_RND  0xffffff00L    /* 8 clear bits at low end. */

  if (sizeof(uzoff_t) < 8)          /* Don't fiddle with big zoff_t. */
  {
    if ((ulg)n > PC_MAX_SAFE)       /* Reduce large values.  (n > m) */
    {
      if ((ulg)n < PC_MAX_RND)      /* Divide n by 512 with rounding, */
        n = ((ulg)n + 0x100) >> 9;  /* if boost won't overflow. */
      else                          /* Otherwise, use max value. */
        n = PC_MAX_SAFE;

      if ((ulg)m < PC_MAX_RND)      /* Divide m by 512 with rounding, */
        m = ((ulg)m + 0x100) >> 9;  /* if boost won't overflow. */
      else                          /* Otherwise, use max value. */
        m = PC_MAX_SAFE;
    }
  }
  if (n != 0)
    if (n > m)
      p = ((200 * ((zoff_t)n - (zoff_t)m) / (zoff_t)n) + 1) / 2;
    else
      p = ((200 * ((zoff_t)n - (zoff_t)m) / (zoff_t)n) - 1) / 2;
  else
    p = 0;
  return (int)p;  /* Return (rounded) % reduction. */
}


#ifndef RISCOS

local int suffixes(fname, sufx_list)
  char *fname;                  /* name to check suffix of */
  char *sufx_list;              /* list of suffixes separated by : or ; */
/* Return true if fname ends in any of the suffixes in sufx_list. */
{
  int match;                    /* true if suffix matches so far */
  char *slp;                    /* pointer into sufx_list */
  char *fnp;                    /* pointer into fname */

  if (sufx_list == NULL)        /* Nothing matches a null suffix list. */
    return 0;

#ifdef QDOS
  short dlen = devlen(fname);
  fname = fname + dlen;
#endif

  match = 1;
#ifdef VMS
  if ((fnp = strrchr(fname,';')) != NULL)       /* Cut out VMS file version. */
    --fnp;
  else
    fnp = fname + strlen(fname) - 1;
#else /* !VMS */
  fnp = fname + strlen(fname) - 1;
#endif /* ?VMS */
  for (slp = sufx_list + strlen(sufx_list) - 1; slp >= sufx_list; slp--)
    if (*slp == ':' || *slp == ';')
    {
      if (match)
        return 1;
      else
      {
        match = 1;
#ifdef VMS
        if ((fnp = strrchr(fname,';')) != NULL) /* Cut out VMS file version. */
          --fnp;
        else
          fnp = fname + strlen(fname) - 1;
#else /* !VMS */
        fnp = fname + strlen(fname) - 1;
#endif /* ?VMS */
      }
    }
    else
    {
      match = match && fnp >= fname && case_map(*slp) == case_map(*fnp);
      fnp--;
    }
  return match;
}

#else /* RISCOS */

local int filetypes(fname, sufx_list)
char *fname;                    /* extra field of file to check filetype of */
char *sufx_list;                /* list of filetypes separated by : or ; */
/* Return true if fname is any of the filetypes in sufx_list. */
{
 char *slp;                     /* pointer into sufx_list */
 char typestr[4];               /* filetype hex string taken from fname */

 if ((((unsigned*)fname)[2] & 0xFFF00000) != 0xFFF00000) {
 /* The file is not filestamped, always try to compress it */
   return 0;
 }

 if (sufx_list == NULL)         /* Nothing matches a null suffix list. */
   return 0;

 sprintf(typestr,"%.3X",(((unsigned*)fname)[2] & 0x000FFF00) >> 8);

 for (slp = sufx_list; slp <= sufx_list+ strlen(sufx_list)- 3; slp += 3) {
   /* slp+=3 to skip 3 hex type */
   while (*slp==':' || *slp==';')
     slp++;

   if (typestr[0] == toupper(slp[0]) &&
       typestr[1] == toupper(slp[1]) &&
       typestr[2] == toupper(slp[2]))
     return 1;
 }
 return 0;
}
#endif /* ?RISCOS */


/* SMSd. */
#if defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT)
/*
 * zread_file(): Pre-read a (non-directory) file and calculate its CRC.
 */
int zread_file( z, l)
struct zlist far *z;    /* Zip entry being processed. */
int l;                  /* True if this file is a symbolic link. */
{
  char *b;              /* Malloc'ed file buffer. */
  int sts = ZE_OK;      /* Return value. */

#ifndef NO_SYMLINKS
  int k = 0;            /* Result of zread().  (ssize_t?) */

  if (l)
  {
    /* Symlink.  Use special read function. */
    /* Allocate a read buffer.  (Might be smarter to do this less often.) */
    if ((b = malloc(SBSZ)) == NULL)
      return ZE_MEM;
    k = rdsymlnk( z->name, b, SBSZ);
    if (k < 0)
      sts = ZE_OPEN;
    else
      crc = crc32( crc, (uch *) b, k);
  }
  else
#endif /* ndef NO_SYMLINKS */
  {
    /* Normal file.  Read whole file.  (iz_file_read() calculates CRC.) */
    /* Check input seekability.  Can't rewind if not.
     * (zrewind() should also fail for a terminal.)
     */
    if (zrewind( ifile) < 0)
      return ZE_READ;

    /* Allocate a read buffer.  (Might be smarter to do this less often.) */
    if ((b = malloc(SBSZ)) == NULL)
      return ZE_MEM;
    while ((k = iz_file_read( b, SBSZ)) > 0 && k != (extent)EOF);

#if defined( UNIX) && defined( __APPLE__)
    file_read_fake_len = 0;             /* Reset Mac-specific size. */
#endif /* defined( UNIX) && defined( __APPLE__) */

    /* Rewind file, if appropriate. */
    if (ifile != fbad)                  /* Necessary test? */
    {
      if (zrewind( ifile) < 0)
        sts = ZE_READ;
    }
  }

  /* Free the buffer. */
  free( (zvoid *)b);

/* SMSd. */
/*
fprintf( stderr, " zread_file().  crc = %08x .\n", crc);
*/

  return sts;
}
#endif /* defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT) */


/* Note: a zip "entry" includes a local header (which includes the file
   name), an encryption header if encrypting, the compressed data
   and possibly an extended local header. */

int zipup(z)
struct zlist far *z;    /* zip entry to compress */
/* Compress the file z->name into the zip entry described by *z and write
   it to the file *y.  Encrypt if requested.  Return an error code in the
   ZE_ class.  Also, update tempzn by the number of bytes written. */
/* y is now global */
{
  iztimes f_utim;       /* UNIX GMT timestamps, filled by filetime() */
  ulg tim;              /* time returned by filetime() */
  ulg a = 0L;           /* attributes returned by filetime() */
  char *b;              /* malloc'ed file buffer */
  extent k = 0;         /* result of zread */
  int j;
  int l = 0;            /* true if this file is a symbolic link */
  int mthd;             /* Method for this entry. */
  int mthd_adj;         /* Method for this entry, adjusted. */
  int lvl;

  zoff_t o = 0, p;      /* offsets in zip file */
  zoff_t q = (zoff_t) -3; /* size returned by filetime */
  uzoff_t uq;           /* unsigned q */
  zoff_t s = 0;         /* size of compressed data */

  int r;                /* temporary variable */
  int isdir;            /* set for a directory name */
  int set_type = 0;     /* set if file type (ascii/binary) unknown */
  zoff_t last_o;        /* used to detect wrap around */
  int sufx_i;           /* Method-by-suffix index. */

  ush tempext = 0;      /* temp copies of extra fields */
  ush tempcext = 0;
  char *tempextra = NULL;
  char *tempcextra = NULL;

#ifdef IZ_CRYPT_AES_WG
  uch salt_len;
  uch auth_len;
#endif

#ifdef IZ_CRYPT_AES_WG_NEW
  uch salt_len;
  uch auth_len;
#endif

#ifdef ENABLE_DLL_PROGRESS
    long percent_all_entries_processed;
    long percent_this_entry_processed;
    uzoff_t bytetotal;
#endif

#ifdef WINDLL
# ifdef ZIP64_SUPPORT
  extern uzoff_t filesize64;
  extern unsigned long low;
  extern unsigned long high;
# endif
#endif

  /* local variable may be updated later */
  z->encrypt_method = encryption_method;

  z->nam = strlen(z->iname);
  isdir = z->iname[z->nam-1] == (char)0x2f; /* ascii[(unsigned)('/')] */

  file_binary = -1;      /* not set, set after first read */
  file_binary_final = 0; /* not set, set after first read */

  /* used for progress tracking */
  bytes_read_this_entry = 0;
  entry_name = z->zname;

#ifdef ENABLE_DLL_PROGRESS
  last_progress_chunk = 0;

  /* Initial progress callback.  Updates in iz_file_read(). */
/*  if (progress_chunk_size > 0) { */
  if (progress_chunk_size > 0 && lpZipUserFunctions->ProgressReport != NULL) {
    /* Callback parameters:
          name, total bytes so far, estimated total bytes to be read,
          this entry bytes so far, total bytes this entry
    */
    if (bytes_total > 0) {
      if (bytes_total < ((uzoff_t)1 << (sizeof(uzoff_t) * 8 - 15))) {
        percent_all_entries_processed = (long)((10000 * bytes_so_far) / bytes_total);
      } else {
        /* avoid overflow, but this is inaccurate for small numbers */
        bytetotal = bytes_total / 10000;
        if (bytetotal == 0) bytetotal = 1;
        percent_all_entries_processed = (long)(bytes_so_far / bytetotal);
      }
    } else{
      percent_all_entries_processed = 0;
    }
    percent_this_entry_processed = 0;
    /* This is a kluge.  The numbers should add up. */
    if (percent_all_entries_processed > 10000) {
      percent_all_entries_processed = 10000;
    }

/*
    printf("\n  progress %5.2f%% all  %5.2f%%  %s\n", (float)percent_all_entries_processed/100,
                                                      (float)percent_this_entry_processed/100,
                                                       entry_name);
*/

    if ((*lpZipUserFunctions->ProgressReport)(entry_name,
                                              percent_all_entries_processed,
                                              percent_this_entry_processed)) {
      ZIPERR(ZE_ABORT, "User terminated operation");
    }
  }
#endif

#if defined(UNICODE_SUPPORT) && defined(WIN32)
  if ((!no_win32_wide) && (z->namew != NULL))
    tim = filetimew(z->namew, &a, &q, &f_utim);
  else
    tim = filetime(z->name, &a, &q, &f_utim);
#else
  tim = filetime(z->name, &a, &q, &f_utim);
#endif

#if defined( UNIX) && defined( __APPLE__)
  /* Special treatment for AppleDouble "._" file:
   * Never translate EOL in an (always binary) AppleDouble file.
   * Add AppleDouble header size to AppleDouble resource fork file size,
   * because data for both will be stored in the AppleDouble "._" file.
   * (Determining this size requires analysis of the extended
   * attributes, where they are available.)
   */
  if ((z->flags& FLAGS_APLDBL) == 0)
  {
    /* Normal file, not an AppleDouble "._" file. */
    translate_eol_lcl = translate_eol;  /* Translate EOL normally. */
    file_read_fake_len = 0;             /* No fake AppleDouble file data. */
  }
  else
  {
    /* AppleDouble "._" file. */
    /* Never translate EOL in an (always binary) AppleDouble file. */
    translate_eol_lcl = 0;

    /* Truncate name at "/rsrc" for getattrlist(). */
    btrbslash = z->name[ strlen( z->name)- strlen( APL_DBL_SUFX)];
    z->name[ strlen( z->name)- strlen( APL_DBL_SUFX)] = '\0';

    /* Create the AppleDouble header (in allocated storage).  It will
     * contain the Finder info, and, if available, any extended
     * attributes.
     */
    r = make_apl_dbl_header( z->name, &j);

    /* Restore name suffix ("/rsrc"). */
    z->name[ strlen( z->name)] = btrbslash;

    /* Increment the AppleDouble file size by the size of the header. */
    q += j;
  } /* if ((z->flags& FLAGS_APLDBL) == 0) [else] */
#endif /* defined( UNIX) && defined( __APPLE__) */

  if (tim == 0 || q == (zoff_t) -3)
    return ZE_OPEN;

/* SMSd. */
#if 0
fprintf( stderr, " isdir = %d, a = %08x , q = %lld.\n", isdir, a, q);
#endif /* 0 */

  /* q is set to -1 if the input file is a device, -2 for a volume label */
  if (q == (zoff_t) -2) {
     isdir = 1;
     q = 0;
  } else if (isdir != ((a & MSDOS_DIR_ATTR) != 0)) {
     /* don't overwrite a directory with a file and vice-versa */
     return ZE_MISS;
  }
  /* reset dot_count for each file */
  if (!display_globaldots)
    dot_count = -1;

  /* display uncompressed size */
  uq = ((uzoff_t) q > (uzoff_t) -3) ? 0 : (uzoff_t) q;
  if (noisy && display_usize) {
    fprintf(mesg, " (");
    DisplayNumString( mesg, uq );
    fprintf(mesg, ")");
    mesg_line_started = 1;
    fflush(mesg);
  }
  if (logall && display_usize) {
    fprintf(logfile, " (");
    DisplayNumString( logfile, uq );
    fprintf(logfile, ")");
    logfile_line_started = 1;
    fflush(logfile);
  }

  /* initial z->len so if error later have something */
  z->len = uq;
  bytes_expected_this_entry = z->len;

#if defined( UNIX) && defined( __APPLE__)
  if (z->flags& FLAGS_APLDBL) {
    z->att = (ush)BINARY;       /* AppleDouble files are always binary. */
  }
  else {
    z->att = (ush)UNKNOWN; /* will be changed later */
  }
#else /* defined( UNIX) && defined( __APPLE__) */
  z->att = (ush)UNKNOWN; /* will be changed later */
#endif /* defined( UNIX) && defined( __APPLE__) [else] */

  z->atx = 0; /* may be changed by set_extra_field() */

  /* Free the old extra fields which are probably obsolete */
  /* Should probably read these and keep any we don't update.  12/30/04 EG */
  if (extra_fields == 2) {
    /* If keeping extra fields, make copy before clearing for set_extra_field()
       A better approach is to modify the port code, but maybe later */
    if (z->ext) {
      if ((tempextra = malloc(z->ext)) == NULL) {
        ZIPERR(ZE_MEM, "extra fields copy");
      }
      memcpy(tempextra, z->extra, z->ext);
      tempext = z->ext;
    }
    if (z->cext) {
      if ((tempcextra = malloc(z->cext)) == NULL) {
        ZIPERR(ZE_MEM, "extra fields copy");
      }
      memcpy(tempcextra, z->cextra, z->cext);
      tempcext = z->cext;
    }
  }
  if (z->ext) {
    free((zvoid *)(z->extra));
  }
  if (z->cext && z->extra != z->cextra) {
    free((zvoid *)(z->cextra));
  }
  z->extra = z->cextra = NULL;
  z->ext = z->cext = 0;

#if defined(MMAP) || defined(BIG_MEM)
  remain = (ulg)-1L; /* changed only for MMAP or BIG_MEM */
#endif /* MMAP || BIG_MEM */
#if (!defined(USE_ZLIB) || defined(MMAP) || defined(BIG_MEM))
  window_size = 0L;
#endif /* !USE_ZLIB || MMAP || BIG_MEM */

  /* Select method and level based on the global method and the file
   * name suffix.  Note: RISCOS must set m after setting extra field.
   */
  mthd = method;        /* Everyone starts with the global (-Z) method. */
  mthd_adj = mthd;      /* Adjusted global method,             */
  if (mthd_adj < 0)     /* with (misnomer) BEST (-1) converted */
    mthd_adj = DEFLATE; /* to its eventual value, DEFLATE.     */
  levell = level;       /* and the global (-0, ..., -9) level. */

#ifndef RISCOS
  /* Scan for a by-suffix compression method (with by-suffix level?). */

  lvl = -1;
  for (sufx_i = 0; mthd_lvl[ sufx_i].method >= 0; sufx_i++)
  {
    /* Note: suffixes() checks for a null suffix list. */
    if (suffixes( z->name, mthd_lvl[ sufx_i].suffixes))
    {
      /* Found a match for this method. */
      mthd = mthd_adj = mthd_lvl[ sufx_i].method;

      if (mthd_lvl[ sufx_i].level_sufx >= 0)
      {
        /* Use the compression level specified for this method by suffix. */
        lvl = levell = mthd_lvl[ sufx_i].level_sufx;
      }
      break;
    }
  }

  if (lvl < 0)
  {
    /* Scan for a by-method compression level, and use that, if found. */
    for (j = 0; mthd_lvl[ j].method >= 0; j++)
    {
      if (mthd_lvl[ j].method == mthd_adj)
      {
        /* Use the compression level for this method (if specified). */
        if (mthd_lvl[ j].level >= 0)
        {
          levell = mthd_lvl[ j].level;
        }
        break;
      }
    }
  }

#endif /* ndef RISCOS */

  /* For now force deflate if using descriptors.  Instead zip and unzip
     could check bytes read against compressed size in each data descriptor
     found and skip over any that don't match.  This is how at least one
     other zipper does it.  To be added later.  Until then it
     probably doesn't hurt to force deflation when streaming.  12/30/04 EG
  */

  /* Now is a good time.  For now allow storing for testing.  12/16/05 EG */
  /* By release need to force deflation based on reports some inflate
     streamed data to find the end of the data */
  /* Need to handle bzip2 */
#ifdef NO_STREAMING_STORE
  if (use_descriptors && m == STORE)
  {
      m = DEFLATE;
  }
#endif

  /* Open file to zip up unless it is stdin */
  if (strcmp(z->name, "-") == 0)
  {
    ifile = (ftype)zstdin;
#if defined(MSDOS) || defined(__human68k__)
    if (isatty(zstdin) == 0)  /* keep default mode if stdin is a terminal */
      setmode(zstdin, O_BINARY);
#endif
    z->tim = tim;
  }
  else
  {
#if !(defined(VMS) && defined(VMS_PK_EXTRA))
    if (extra_fields) {
      /* create extra field and change z->att and z->atx if desired */
      set_extra_field(z, &f_utim);
# ifdef QLZIP
      if(qlflag)
          a |= (S_IXUSR) << 16;   /* Cross compilers don't set this */
# endif
# ifdef RISCOS
  for (sufx_i = 0; mthd_lvl[ sufx_i].method >= 0; sufx_i++)
  {
    /* Note: filetypes() checks for a null suffix list. */
    if (filetypes( z->extra, mthd_lvl[ sufx_i].suffixes))
    {
      /* Found a match for this method. */
      mthd = mthd_lvl[ sufx_i].method;
      break;
    }
  }
# endif /* def RISCOS */

      /* For now allow store for testing */
#ifdef NO_STREAMING_STORE
      /* For now force deflation if using data descriptors. */
      if (use_descriptors && (mthd == STORE))
      {
        mthd = DEFLATE;
      }
#endif

    }
#endif /* !(VMS && VMS_PK_EXTRA) */
    l = issymlnk(a);
    if (l) {
      ifile = fbad;
      mthd = STORE;
    }
    else if (isdir) { /* directory */
      ifile = fbad;
      mthd = STORE;
      q = 0;
    }
#ifdef THEOS
    else if (((a >> 16) & S_IFMT) == S_IFLIB) {   /* library */
      ifile = fbad;
      mthd = STORE;
      q = 0;
    }
#endif
    else {
#ifdef CMS_MVS
      if (bflag) {
        if ((ifile = zopen(z->name, fhowb)) == fbad)
           return ZE_OPEN;
      }
      else
#endif /* CMS_MVS */
#if defined(UNICODE_SUPPORT) && defined(WIN32)
      if (!no_win32_wide) {
        if ((ifile = zwopen(z->namew, fhow)) == fbad)
          return ZE_OPEN;
      } else {
        if ((ifile = zopen(z->name, fhow)) == fbad)
          return ZE_OPEN;
      }
#else /* defined(UNICODE_SUPPORT) && defined(WIN32) */
      if ((ifile = zopen(z->name, fhow)) == fbad)
        return ZE_OPEN;
#endif /* defined(UNICODE_SUPPORT) && defined(WIN32) [else] */
    }

    z->tim = tim;

#if defined(VMS) && defined(VMS_PK_EXTRA)
    /* vms_get_attributes must be called after vms_open() */
    if (extra_fields) {
      /* create extra field and change z->att and z->atx if desired */
      vms_get_attributes(ifile, z, &f_utim);
    }
#endif /* VMS && VMS_PK_EXTRA */

#if defined(MMAP) || defined(BIG_MEM)
    /* Map ordinary files but not devices. This code should go in fileio.c */
    if (!TRANSLATE_EOL && m != STORE && q != -1L && (ulg)q > 0 &&
        (ulg)q + MIN_LOOKAHEAD > (ulg)q) {
# ifdef MMAP
      /* Map the whole input file in memory */
      if (window != NULL)
        free(window);  /* window can't be a mapped file here */
      window_size = (ulg)q + MIN_LOOKAHEAD;
      remain = window_size & (PAGESIZE-1);
      /* If we can't touch the page beyond the end of file, we must
       * allocate an extra page.
       */
      if (remain > MIN_LOOKAHEAD) {
        window = (uch*)mmap(0, window_size, PROT_READ, MAP_PRIVATE, ifile, 0);
      } else {
        window = (uch*)valloc(window_size - remain + PAGESIZE);
        if (window != NULL) {
          window = (uch*)mmap((char*)window, window_size - remain, PROT_READ,
                        MAP_PRIVATE | MAP_FIXED, ifile, 0);
        } else {
          window = (uch*)(-1);
        }
      }
      if (window == (uch*)(-1)) {
        Trace((mesg, " mmap failure on %s\n", z->name));
        window = NULL;
        window_size = 0L;
        remain = (ulg)-1L;
      } else {
        remain = (ulg)q;
      }
# else /* !MMAP, must be BIG_MEM */
      /* Read the whole input file at once */
      window_size = (ulg)q + MIN_LOOKAHEAD;
      window = window ? (uch*) realloc(window, (unsigned)window_size)
                      : (uch*) malloc((unsigned)window_size);
      /* Just use normal code if big malloc or realloc fails: */
      if (window != NULL) {
        remain = (ulg)zread(ifile, (char*)window, q+1);
        if (remain != (ulg)q) {
          fprintf(mesg, " q=%lu, remain=%lu ", (ulg)q, remain);
          error("can't read whole file at once");
        }
      } else {
        window_size = 0L;
      }
# endif /* ?MMAP */
    }
#endif /* MMAP || BIG_MEM */

  } /* strcmp(z->name, "-") == 0 */

  if (extra_fields == 2) {
    unsigned len;
    char *p;

    /* step through old extra fields and copy over any not already
       in new extra fields */
    p = copy_nondup_extra_fields(tempextra, tempext, z->extra, z->ext, &len);
    free(z->extra);
    z->ext = len;
    z->extra = p;
    p = copy_nondup_extra_fields(tempcextra, tempcext, z->cextra, z->cext, &len);
    free(z->cextra);
    z->cext = len;
    z->cextra = p;

    if (tempext)
      free(tempextra);
    if (tempcext)
      free(tempcextra);
  }

  if (q == 0)
    mthd = STORE;
  if (mthd == BEST)
    mthd = DEFLATE;

  /* Do not create STORED files with extended local headers if the
   * input size is not known, because such files could not be extracted.
   * So if the zip file is not seekable and the input file is not
   * on disk, obey the -0 option by forcing deflation with stored block.
   * Note however that using "zip -0" as filter is not very useful...
   * ??? to be done.
   */

  /* An alternative used by others is to allow storing but on reading do
   * a second check when a signature is found.  This is simply to check
   * the compressed size to the bytes read since the start of the file data.
   * If this is the right signature then the compressed size should match
   * the size of the compressed data to that point.  If not look for the
   * next signature.  We should do this.  12/31/04 EG
   *
   * For reading and testing we should do this, but should not write
   * stored streamed data unless for testing as finding the end of
   * streamed deflated data can be done by inflating.  6/26/06 EG
   */

  /* Fill in header information and write local header to zip file.
   * This header will later be re-written since compressed length and
   * crc are not yet known.
   */

  /* (Assume ext, cext, com, and zname already filled in.) */
#if defined(OS2) || defined(WIN32)
# ifdef WIN32_OEM
  /* When creating OEM-coded names on Win32, the entries must always be marked
     as "created on MSDOS" (OS_CODE = 0), because UnZip needs to handle archive
     entry names just like those created by Zip's MSDOS port.
   */
  z->vem = (ush)(dosify ? 20 : 0 + Z_MAJORVER * 10 + Z_MINORVER);
# else
  z->vem = (ush)(z->dosflag ? (dosify ? 20 : /* Made under MSDOS by PKZIP 2.0 */
                               (0 + Z_MAJORVER * 10 + Z_MINORVER))
                 : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER);
  /* For a plain old (8+3) FAT file system, we cheat and pretend that the file
   * was not made on OS2/WIN32 but under DOS. unzip is confused otherwise.
   */
# endif
#else /* !(OS2 || WIN32) */
  z->vem = (ush)(dosify ? 20 : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER);
#endif /* ?(OS2 || WIN32) */

  /* Need PKUNZIP 2.0 except for store */
  z->ver = (ush)(mthd == STORE ? 10 : 20);

#if defined( LZMA_SUPPORT) || defined( PPMD_SUPPORT)
  if ((mthd == LZMA) || (mthd == PPMD))
    z->ver = (ush)(mthd == STORE ? 10 : 63);
#endif
#ifdef BZIP2_SUPPORT
  if (mthd == BZIP2)
    z->ver = (ush)(mthd == STORE ? 10 : 46);
#endif

  /* standard says directories need minimum version 20 */
  if (isdir && z->ver == 10)
    z->ver = 20;

  z->crc = 0;  /* to be updated later */

  /* Set extended header (data descriptor) and encryption flag bits for
   * non-directory entries.  (Directories are not encrypted, and need no
   * extended headers.  zip.c does "z->flg = 0".)
   */
  if (!isdir)
  {
    /* Use extended header (data descriptor)? */
    /* Check output seekability here? */

/* Define (zip,h?) and use macros instead of numbers for flag bits?
 * #define FLG_ENCRYPT       0x0001
 * #define FLG_EXT_HEADER    0x0008
 * [...]
 * Or GPF_ (for General-Purpose Flags)?
 */
    if (use_descriptors)
    {
      z->flg |= 8;              /* Explicit request for extended header. */
    }
#ifdef IZ_CRYPT_TRAD
    else
    {
      if (z->encrypt_method == TRADITIONAL_ENCRYPTION)
      {
        int have_crc = 0;
# ifdef ETWODD_SUPPORT
        if (etwodd)             /* Encrypt Trad without extended header. */
        {
          /* Pre-read the file to get the real CRC. */
          crc = CRCVAL_INITIAL;         /* Set the initial CRC value. */
          r = zread_file( z, l);        /* Read the file. */
          /* Need some error handling here. */
          z->crc = crc;                 /* Save the real CRC. */
          have_crc = 1;
        }
# endif /* def ETWODD_SUPPORT */

        if (have_crc == 0)      /* We want/need to use an extended header. */
        {
          z->flg |= 8;          /* Encrypt Trad with extended header. */
          /* Traditional encryption with an extended header implies that
           * we use (the low 16 bits of) the MS-DOS modification time
           * instead of the real (unknown) CRC as the "CRC" for the
           * pseudo-random seed datum.  (The high 16 bits of the "CRC"
           * are used in the Traditional encryption header.)
           */
          z->crc = z->tim << 16;
        }
      } /* z->encrypt_method == TRADITIONAL_ENCRYPTION */
    } /* use_descriptors [else] */
#endif /* def IZ_CRYPT_TRAD */

#ifdef IZ_CRYPT_ANY
    /* Encrypt (any method)? */
    if (key != NULL) {
      z->flg |= 1;              /* Set the encrypt flag bit. */
    }
#endif /* def IZ_CRYPT_ANY */
  } /* !isdir */

  z->lflg = z->flg;
  z->how = (ush)mthd;                           /* may be changed later  */
  z->siz = (zoff_t)(mthd == STORE && q >= 0 ? q : 0); /* will be changed later */
  z->len = (zoff_t)(q != -1L ? q : 0);          /* may be changed later  */
  if (z->att == (ush)UNKNOWN) {
      z->att = BINARY;                    /* set sensible value in header */
      set_type = 1;
  }
  /* Attributes from filetime(), flag bits from set_extra_field(): */
#if defined(DOS) || defined(OS2) || defined(WIN32)
  z->atx = z->dosflag ? a & 0xff : a | (z->atx & 0x0000ff00);
#else
  z->atx = dosify ? a & 0xff : a | (z->atx & 0x0000ff00);
#endif /* DOS || OS2 || WIN32 */

#ifdef IZ_CRYPT_AES_WG
  /* Initialize AES encryption
   *
   * Data size: this value is currently 7, but vendors should not assume
   * that it will always remain 7.
   * Vendor ID: the vendor ID field should always be set to the two
   * ASCII characters "AE".
   * Vendor version: the vendor version for AE-1 is 0x0001. The vendor
   * version for AE-2 is 0x0002.  (The handling of the CRC value is the
   * only difference between the AE-1 and AE-2 formats.)
   * Encryption strength: the mode values (encryption strength) for AE-1
   * and AE-2 are:
   *     Value  Strength
   *     0x01   128-bit encryption key
   *     0x02   192-bit encryption key
   *     0x03   256-bit encryption key
   * Compression method: the compression method is the one that would
   * otherwise have been stored.
   */
  if (encryption_method >= AES_MIN_ENCRYPTION)
  {
    if (q <= 0)
    {
      /* don't encrypt empty files */
      z->encrypt_method = NO_ENCRYPTION;
    }
    else
    {
      int ret;

      zpwd = (unsigned char *)key;
      zpwd_len = strlen(key);

      /* these values probably need tweeking */

      /* check if password length supports requested encryption strength */
      if (encryption_method == AES_192_ENCRYPTION && zpwd_len < 20) {
        ZIPERR(ZE_CRYPT, "AES192 requires minimum 20 character password");
      } else if (encryption_method == AES_256_ENCRYPTION && zpwd_len < 24) {
        ZIPERR(ZE_CRYPT, "AES256 requires minimum 24 character password");
      }
      if (encryption_method == AES_128_ENCRYPTION) {
        aes_strength = 0x01;
        key_size = 128;
      } else if (encryption_method == AES_192_ENCRYPTION) {
        aes_strength = 0x02;
        key_size = 192;
      } else if (encryption_method == AES_256_ENCRYPTION) {
        aes_strength = 0x03;
        key_size = 256;
      } else {
        ZIPERR(ZE_CRYPT, "Bad encryption method");
      }

      salt_len = SALT_LENGTH(aes_strength);
      auth_len = MAC_LENGTH(aes_strength);

      /* get the salt */
      prng_rand(zsalt, salt_len, &aes_rnp);

      /* initialize encryption context for this file */
      ret = fcrypt_init(
       aes_strength,            /* extra data value indicating key size */
       zpwd,                    /* the password */
       zpwd_len,                /* number of bytes in password */
       zsalt,                   /* the salt */
       zpwd_verifier,           /* on return contains password verifier */
       &zctx);                  /* encryption context */
      if (ret == PASSWORD_TOO_LONG) {
        ZIPERR(ZE_CRYPT, "Password too long");
      } else if (ret == BAD_MODE) {
        ZIPERR(ZE_CRYPT, "Bad mode");
      }
    }
  }
#endif


#ifdef IZ_CRYPT_AES_WG_NEW
  /* Initialize AES encryption
   *
   * Data size: this value is currently 7, but vendors should not assume
   * that it will always remain 7.
   * Vendor ID: the vendor ID field should always be set to the two
   * ASCII characters "AE".
   * Vendor version: the vendor version for AE-1 is 0x0001. The vendor
   * version for AE-2 is 0x0002.  (The handling of the CRC value is the
   * only difference between the AE-1 and AE-2 formats.)
   * Encryption strength: the mode values (encryption strength) for AE-1
   * and AE-2 are:
   *     Value  Strength
   *     0x01   128-bit encryption key
   *     0x02   192-bit encryption key
   *     0x03   256-bit encryption key
   * Compression method: the compression method is the one that would
   * otherwise have been stored.
   */
  if (encryption_method >= AES_MIN_ENCRYPTION)
  {
    if (q <= 0)
    {
      /* don't encrypt empty files */
      z->encrypt_method = NO_ENCRYPTION;
    }
    else
    {
      int ret;

      zpwd = (unsigned char *)key;
      zpwd_len = strlen(key);

      /* these values probably need tweeking */

      /* check if password length supports requested encryption strength */
      if (encryption_method == AES_192_ENCRYPTION && zpwd_len < 20) {
        ZIPERR(ZE_CRYPT, "AES192 requires minimum 20 character password");
      } else if (encryption_method == AES_256_ENCRYPTION && zpwd_len < 24) {
        ZIPERR(ZE_CRYPT, "AES256 requires minimum 24 character password");
      }
      if (encryption_method == AES_128_ENCRYPTION) {
        aes_strength = 0x01;
        key_size = 128;
      } else if (encryption_method == AES_192_ENCRYPTION) {
        aes_strength = 0x02;
        key_size = 192;
      } else if (encryption_method == AES_256_ENCRYPTION) {
        aes_strength = 0x03;
        key_size = 256;
      } else {
        ZIPERR(ZE_CRYPT, "Bad encryption method");
      }

      salt_len = SALT_LENGTH(aes_strength);
      auth_len = MAC_LENGTH(aes_strength);

      ccm_init_message(                 /* initialise for a new message */
       const unsigned char iv[],        /* the initialisation vector    */
       unsigned long iv_len,            /* the nonce length             */
       length_t hdr_len,                /* the associated data length   */
       length_t msg_len,                /* message data length          */
       unsigned long tag_len,           /* authentication field length  */
       ccm_ctx ctx[1]);                 /* the mode context             */

      /* get the salt */
      prng_rand(zsalt, salt_len, &aes_rnp);

      /* initialize encryption context for this file */
      ret = fcrypt_init(
       aes_strength,            /* extra data value indicating key size */
       zpwd,                    /* the password */
       zpwd_len,                /* number of bytes in password */
       zsalt,                   /* the salt */
       zpwd_verifier,           /* on return contains password verifier */
       &zctx);                  /* encryption context */
      if (ret == PASSWORD_TOO_LONG) {
        ZIPERR(ZE_CRYPT, "Password too long");
      } else if (ret == BAD_MODE) {
        ZIPERR(ZE_CRYPT, "Bad mode");
      }
    }
  }
#endif


  if ((r = putlocal(z, PUTLOCAL_WRITE)) != ZE_OK) {
    if (ifile != fbad)
      zclose(ifile);
    return r;
  }

  /* now get split information set by bfwrite() */
  z->off = current_local_offset;

  /* disk local header was written to */
  z->dsk = current_local_disk;

  tempzn += 4 + LOCHEAD + z->nam + z->ext;


#ifdef IZ_CRYPT_ANY
  /* write out encryption header at top of file data */
  if (!isdir && (key != NULL) && (z->encrypt_method != NO_ENCRYPTION)) {
# ifdef IZ_CRYPT_AES_WG
    if (z->encrypt_method >= AES_MIN_ENCRYPTION) {
      aes_crypthead(zsalt, salt_len, zpwd_verifier);

      z->siz += salt_len + 2 + auth_len;  /* to be updated later */
      tempzn += salt_len + 2;
    } else {
# endif
# ifdef IZ_CRYPT_TRAD
      crypthead(key, z->crc);   /* Trad encrypt hdr with real or fake CRC. */
      z->siz += RAND_HEAD_LEN;  /* to be updated later */
      tempzn += RAND_HEAD_LEN;
# else /* def IZ_CRYPT_TRAD */
      /* Do something in case the impossible happens here? */
# endif /* def IZ_CRYPT_TRAD [else] */
# ifdef IZ_CRYPT_AES_WG
    }
# endif /* def IZ_CRYPT_AES_WG */
  }
#endif /* def IZ_CRYPT_ANY */
  if (ferror(y)) {
    if (ifile != fbad)
      zclose(ifile);
    ZIPERR(ZE_WRITE, "unexpected error on zip file");
  }

  last_o = o;
  o = zftello(y); /* for debugging only, ftell can fail on pipes */
  if (ferror(y))
    clearerr(y);

  if (o != -1 && last_o > o) {
    fprintf(mesg, "last %s o %s\n", zip_fzofft(last_o, NULL, NULL),
                                    zip_fzofft(o, NULL, NULL));
    ZIPERR(ZE_BIG, "seek wrap - zip file too big to write");
  }

  /* Write stored or deflated file to zip file */
  isize = 0L;
  crc = CRCVAL_INITIAL;

  if (isdir) {

    /* dir */

    /* nothing to write */
  }
  else if (mthd != STORE) {

    /* compress */

    if (set_type) z->att = (ush)UNKNOWN;
    /* ... is finally set in file compression routine */
#ifdef BZIP2_SUPPORT
    if (mthd == BZIP2) {
      s = bzfilecompress(z, &mthd);
    }
    else
#endif /* BZIP2_SUPPORT */
#ifdef LZMA_SUPPORT
    if (mthd == LZMA) {
      s = lzma_filecompress(z, &mthd);
    }
    else
#endif /* LZMA_SUPPORT */
#ifdef PPMD_SUPPORT
    if (mthd == PPMD) {
      s = ppmd_filecompress(z, &mthd);
    }
    else
#endif /* PPMD_SUPPORT */
    {
      s = filecompress(z, &mthd);
    }
    /* not sure why this is here */
    /* fflush(y); */
#ifndef PGP
    if (z->att == (ush)BINARY && TRANSLATE_EOL && file_binary) {
      if (TRANSLATE_EOL == 1)
        zipwarn("has binary so -l ignored", "");
      else
        zipwarn("has binary so -ll ignored", "");
    }
    else if (z->att == (ush)BINARY && TRANSLATE_EOL) {
      if (TRANSLATE_EOL == 1)
        zipwarn("-l used on binary file - corrupted?", "");
      else
        zipwarn("-ll used on binary file - corrupted?", "");
    }
#endif
  }
  else
  {
    /* store */

    if ((b = malloc(SBSZ)) == NULL)
       return ZE_MEM;

    if (l) {
      k = rdsymlnk(z->name, b, SBSZ);
/*
 * compute crc first because zfwrite will alter the buffer b points to !!
 */
      crc = crc32(crc, (uch *) b, k);
      if (zfwrite(b, 1, k) != k)
      {
        free((zvoid *)b);
        return ZE_TEMP;
      }
      isize = k;

/* SMSd. */
      q = k;
#ifdef MINIX
#endif /* MINIX */
    }
    else
    {
      while ((k = iz_file_read(b, SBSZ)) > 0 && k != (extent) EOF)
      {
        if (zfwrite(b, 1, k) != k)
        {
          if (ifile != fbad)
            zclose(ifile);
          free((zvoid *)b);
          return ZE_TEMP;
        }

        /* store */

        /* Display progress dots. */
        if (!display_globaldots)
        {
          display_dot( 0, SBSZ);
        }
      }
    }
    free((zvoid *)b);
    s = isize;

  } /* store */

  if (ifile != fbad && zerr(ifile)) {
    perror("\nzip warning");
    if (logfile)
      fprintf(logfile, "\nzip warning: %s\n", strerror(errno));
    zipwarn("could not read input file: ", z->oname);
  }
  if (ifile != fbad)
    zclose(ifile);
#ifdef MMAP
  if (remain != (ulg)-1L) {
    munmap((caddr_t) window, window_size);
    window = NULL;
  }
#endif /*MMAP */

  tempzn += s;
  p = tempzn; /* save for future fseek() */

#if (!defined(MSDOS) || defined(OS2))
#if !defined(VMS) && !defined(CMS_MVS) && !defined(__mpexl)
  /* Check input size (but not in VMS -- variable record lengths mess it up)
   * and not on MSDOS -- diet in TSR mode reports an incorrect file size)
   */
#ifndef TANDEM /* Tandem EOF does not match byte count unless Unstructured */
  if (!TRANSLATE_EOL && q != -1L && isize != q)
  {
    Trace((mesg, " i=%lu, q=%lu ", isize, q));
    zipwarn(" file size changed while zipping ", z->name);
  }
#endif /* !TANDEM */
#endif /* !VMS && !CMS_MVS && !__mpexl */
#endif /* (!MSDOS || OS2) */

/* SMSd. */ /*
fprintf( stderr, " Done.          crc = %08x .\n", crc);
*/

  /* Check real CRC against pre-read CRC. */
#if defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT)
  if (etwodd)                   /* Encrypt Trad without extended header. */
  {
    if (!isdir && (z->crc != crc))
    {
      /* CRC mismatch on a non-directory file.  Complain. */
      fprintf( mesg, " pre-read: %08lx, read: %08lx ", z->crc, crc);
      error( "CRC mismatch");
      /* Revert to using an extended header (data descriptor). */
      z->flg |= 8;
      z->lflg |= 8;
    }
  }
#endif /* defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT) */

  if (isdir)
  {
    /* A directory */
    z->siz = 0;
    z->len = 0;
    z->how = STORE;
    z->ver = 20;  /* AppNote requires version 2.0 for a directory */
/* SMSd. */
#if 0
/* Following no longer needed? */
    /* never encrypt directory so don't need extended local header */
    z->flg &= ~8;
    z->lflg &= ~8;
#endif /* 0 */
  }
  else
  {
    /* Try to rewrite the local header with correct information */
    z->crc = crc;
    z->siz = s;
#ifdef IZ_CRYPT_ANY
    if ((key != NULL) && (z->encrypt_method != NO_ENCRYPTION)) {
# ifdef IZ_CRYPT_AES_WG
      if (z->encrypt_method >= AES_MIN_ENCRYPTION) {
        z->siz += salt_len + 2 + auth_len;
      } else {
# endif /* def IZ_CRYPT_AES_WG */
        z->siz += RAND_HEAD_LEN;
# ifdef IZ_CRYPT_AES_WG
      }
# endif /* def IZ_CRYPT_AES_WG */
    }
#endif /* def IZ_CRYPT_ANY */
    z->len = isize;

#ifdef IZ_CRYPT_AES_WG
    /* close encryption for this file */
    if (z->encrypt_method >= AES_MIN_ENCRYPTION)
    {
      int ret;

      ret = fcrypt_end( auth_code,   /* on return contains the authentication code */
                        &zctx);      /* encryption context */
      bfwrite(auth_code, 1, ret, BFWRITE_DATA);
      tempzn += ret;
    }
#endif /* def IZ_CRYPT_AES_WG */


    /* if can seek back to local header */
#ifdef BROKEN_FSEEK
    if ((z->flg & 8) || !fseekable(y) || zfseeko(y, z->off, SEEK_SET))
#else
    if ((z->flg & 8) || zfseeko(y, z->off, SEEK_SET))
#endif
    { /* Planning extended header (data descr), or unable to seek() back. */
      if (z->how != (ush) mthd)
         error("can't rewrite method");
      if (mthd == STORE && q < 0)
         ZIPERR(ZE_PARMS, "zip -0 not supported for I/O on pipes or devices");
      if ((r = putextended(z)) != ZE_OK)
        return r;
      /* if Zip64 and not seekable then Zip64 data descriptor */
#ifdef ZIP64_SUPPORT
      tempzn += (zip64_entry ? 24L : 16L);
#else
      tempzn += 16L;
#endif
      z->flg = z->lflg; /* if z->flg modified by deflate */
    }
    else
    { /* Not using an extended header (data descriptor). */
      uzoff_t expected_size = (uzoff_t)s;
#ifdef IZ_CRYPT_ANY
      if (key && (z->encrypt_method != NO_ENCRYPTION)) {
# ifdef IZ_CRYPT_AES_WG
        if (z->encrypt_method >= AES_MIN_ENCRYPTION) {
          expected_size += salt_len + 2 + auth_len;
        } else {
# endif /* def IZ_CRYPT_AES_WG */
          expected_size += 12;
# ifdef IZ_CRYPT_AES_WG
        }
# endif /* def IZ_CRYPT_AES_WG */
      }
#endif /* def IZ_CRYPT_ANY */

      /* ftell() not as useful across splits */
      if (bytes_this_entry != expected_size) {
        fprintf(mesg, " s=%s, actual=%s ",
                zip_fzofft(s, NULL, NULL), zip_fzofft(bytes_this_entry, NULL, NULL));
        error("incorrect compressed size");
      }
#if 0
       /* seek ok, ftell() should work, check compressed size */
# if !defined(VMS) && !defined(CMS_MVS)
      if (p - o != s) {
        fprintf(mesg, " s=%s, actual=%s ",
                zip_fzofft(s, NULL, NULL), zip_fzofft(p-o, NULL, NULL));
        error("incorrect compressed size");
      }
# endif /* !VMS && !CMS_MVS */
#endif /* 0 */
      z->how = (ush)mthd;
      switch (mthd)
      {
      case STORE:
        if (isdir)
          z->ver = 20;
        else
          z->ver = 10;
        break;
      /* Need PKUNZIP 2.0 for DEFLATE */
      case DEFLATE:
        z->ver = 20; break;
#ifdef BZIP2_SUPPORT
      case BZIP2:
        z->ver = 46; break;
#endif
      }
#if 0
#ifndef IZ_CRYPT_NOEXT
      /*
       * The encryption header needs the crc, but we don't have it
       * for a new file.  The file time is used instead and the encryption
       * header then used to encrypt the data.  The AppNote standard only
       * can be applied to a file that the crc is known, so that means
       * either an existing entry in an archive or get the crc before
       * creating the encryption header and then encrypt the data.
       */
      /* 2013-02-06 SMS.
       * The normal UnZip algorithm for Traditional decryption expects
       * the last byte (or two) of the encryption header to match that
       * part of the crc.  When an extended header is used, UnZip
       * expects the last byte of the encryption header to match that
       * part of the MS-DOS mod time instead of the crc.
       * To use the normal (non-extended-header) scheme, we would need
       * to know the crc when the encryption header is written, or else
       * we would need to know how to re-write the encryption header
       * after the crc has been determined.  (Or else fiddle something
       * to make the crc match what we did use.)
       */
      if ((z->flg & 1) == 0) {
        /* not encrypting so don't need extended local header */
        z->flg &= ~8;
      }
#endif /* ndef IZ_CRYPT_NOEXT */

#ifdef IZ_CRYPT_AES_WG
      if (z->encrypt_method >= AES_MIN_ENCRYPTION) {
        z->flg &= ~8;
      }
#endif /* def IZ_CRYPT_AES_WG */
#endif /* 0 */

      /* deflate may have set compression level bit markers in z->flg,
         and we can't think of any reason central and local flags should
         be different. */
      z->lflg = z->flg;

      /* Not using descriptors, so back up and rewrite local header. */
      if (split_method == 1 && current_local_file != y) {
        if (zfseeko(current_local_file, z->off, SEEK_SET))
          return ZE_READ;
      }

      /* if local header in another split, putlocal will close it */
      if ((r = putlocal(z, PUTLOCAL_REWRITE)) != ZE_OK)
        return r;

      if (zfseeko(y, bytes_this_split, SEEK_SET))
        return ZE_READ;

#if 0
#ifdef IZ_CRYPT_TRAD
      if (z->encrypt_method == TRADITIONAL_ENCRYPTION)
      {
/* #ifndef IZ_CRYPT_NOEXT */
        if (!etwodd)
        {
          if ((z->flg & 1) != 0)
          {
# ifdef IZ_CRYPT_AES_WG
            if (z->encrypt_method == TRADITIONAL_ENCRYPTION)
# endif /* def IZ_CRYPT_AES_WG */
            {
              /* Traditionally encrypted, so extended header still required. */
              if ((r = putextended(z)) != ZE_OK)
                return r;
            }
          }
# ifdef ZIP64_SUPPORT
          if (zip64_entry)
            tempzn += 24L;
          else
            tempzn += 16L;
# else
          tempzn += 16L;
# endif
        }
      }
#endif /* def IZ_CRYPT_TRAD */
#endif  /* 0 */
    } /* Put out extended header. [else] */
  } /* isdir [else] */
  /* Free the local extra field which is no longer needed */
  if (z->ext) {
    if (z->extra != z->cextra) {
      free((zvoid *)(z->extra));
      z->extra = NULL;
    }
    z->ext = 0;
  }

  /* Display statistics */


  if (noisy || logall)
  {
    l_str_p = "";
    if (show_what_doing || verbose)
      {
      if (mthd != STORE)
      {
        sprintf( l_str, "-%d", levell);
        l_str_p = l_str;
      }
    }
  }

  if (noisy)
  {
    if (verbose) {
      fprintf( mesg, "        (in=%s) (out=%s)",
               zip_fzofft(isize, NULL, "u"), zip_fzofft(s, NULL, "u"));
    }
#ifdef BZIP2_SUPPORT
    if (mthd == BZIP2)
      fprintf(mesg, " (bzipped%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
#ifdef LZMA_SUPPORT
    if (mthd == LZMA)
      fprintf(mesg, " (LZMAed%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
#ifdef PPMD_SUPPORT
    if (mthd == PPMD)
      fprintf(mesg, " (PPMded%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
    if (mthd == DEFLATE)
      fprintf(mesg, " (deflated%s %d%%)\n", l_str_p, percent(isize, s));
    else
      fprintf(mesg, " (stored 0%%)\n");
    mesg_line_started = 0;
    fflush(mesg);
  }
  if (logall)
  {
#ifdef BZIP2_SUPPORT
    if (mthd == BZIP2)
      fprintf(logfile, " (bzipped%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
#ifdef LZMA_SUPPORT
    if (mthd == LZMA)
      fprintf(logfile, " (LZMAed%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
#ifdef PPMD_SUPPORT
    if (mthd == PPMD)
      fprintf(logfile, " (PPMded%s %d%%)\n", l_str_p, percent(isize, s));
    else
#endif
    if (mthd == DEFLATE)
      fprintf(logfile, " (deflated%s %d%%)\n", l_str_p, percent(isize, s));
    else
      fprintf(logfile, " (stored 0%%)\n");
    logfile_line_started = 0;
    fflush(logfile);
  }

#ifdef WINDLL
# ifdef ZIP64_SUPPORT
   /* The DLL api has been updated and uses a different
      interface.  7/24/04 EG */
   if (lpZipUserFunctions->ServiceApplication64 != NULL)
    {
    if ((*lpZipUserFunctions->ServiceApplication64)(z->zname, isize))
                ZIPERR(ZE_ABORT, "User terminated operation");
    }
  else
   {
   filesize64 = isize;
   low = (unsigned long)(filesize64 & 0x00000000FFFFFFFF);
   high = (unsigned long)((filesize64 >> 32) & 0x00000000FFFFFFFF);
   if (lpZipUserFunctions->ServiceApplication64_No_Int64 != NULL) {
    if ((*lpZipUserFunctions->ServiceApplication64_No_Int64)(z->zname, low, high))
                ZIPERR(ZE_ABORT, "User terminated operation");
    }
   }
# else
  if (lpZipUserFunctions->ServiceApplication != NULL)
  {
    if ((*lpZipUserFunctions->ServiceApplication)(z->zname, isize))
    {
      ZIPERR(ZE_ABORT, "User terminated operation");
    }
  }
# endif
#endif

#ifdef ENABLE_DLL_PROGRESS
  /* Final progress callback. */
/*  if (progress_chunk_size > 0) { */
  if (progress_chunk_size > 0 && lpZipUserFunctions->ProgressReport != NULL) {
    /* Callback parameters:
          name, total bytes so far, estimated total bytes to be read,
          this entry bytes so far, total bytes this entry
    */
    if (bytes_total > 0) {
      if (bytes_total < ((uzoff_t)1 << (sizeof(uzoff_t) * 8 - 15))) {
        percent_all_entries_processed
          = (long)((10000 * (bytes_so_far + bytes_expected_this_entry)) / bytes_total);
      } else {
        /* avoid overflow, but this is inaccurate for small numbers */
        bytetotal = bytes_total / 10000;
        if (bytetotal == 0) bytetotal = 1;
        percent_all_entries_processed
          = (long)((bytes_so_far + bytes_expected_this_entry) / bytetotal);
      }
    } else{
      percent_all_entries_processed = 0;
    }
    percent_this_entry_processed = 10000;
    /* This is a kluge.  The numbers should add up. */
    if (percent_all_entries_processed > 10000) {
      percent_all_entries_processed = 10000;
    }

/*
    printf("\n  progress %5.2f%% all  %5.2f%%  %s\n", (float)percent_all_entries_processed/100,
                                                      (float)percent_this_entry_processed/100,
                                                       entry_name);
*/

    if ((*lpZipUserFunctions->ProgressReport)(entry_name,
                                              percent_all_entries_processed,
                                              percent_this_entry_processed)) {
      ZIPERR(ZE_ABORT, "User terminated operation");
    }
  }
#endif

  return ZE_OK;
}




local unsigned iz_file_read(buf, size)
  char *buf;
  unsigned size;
/* Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
{
  unsigned len;
  char *b;
  zoff_t isize_prev;    /* Previous isize.  Used for overflow check. */
#ifdef ENABLE_DLL_PROGRESS
  uzoff_t current_progress_chunk;
  long percent_all_entries_processed;
  long percent_this_entry_processed;
  uzoff_t bytetotal;
#endif

#if defined( UNIX) && defined( __APPLE__)
  /* Supply data from fake file buffer, if available.
   * Always binary, never translated.
   */
  if (file_read_fake_len > 0) {
    len = IZ_MIN( file_read_fake_len, size);
    memcpy( buf, file_read_fake_buf, len);
    file_read_fake_len -= len;
  } else
  /* Otherwise, read real data from the real file. */
#endif /* defined( UNIX) && defined( __APPLE__) */
#if defined(MMAP) || defined(BIG_MEM)
  if (remain == 0L) {
    return 0;
  } else if (remain != (ulg)-1L) {
    /* The window data is already in place. We still compute the crc
     * by 32K blocks instead of once on whole file to keep a certain
     * locality of reference.
     */
    Assert(buf == (char*)window + isize, "are you lost?");
    if ((ulg)size > remain) size = (unsigned)remain;
    if (size > WSIZE) size = WSIZE; /* don't touch all pages at once */
    remain -= (ulg)size;
    len = size;
  } else
#endif /* MMAP || BIG_MEM */
  if (TRANSLATE_EOL == 0) {
    len = zread(ifile, buf, size);
    bytes_read_this_entry += len;
    if (len == (unsigned)EOF || len == 0) return len;

#ifdef ZOS_UNIX
    /* 2012-08-02 SMS.
     * Added a binary-text test here (like those below for cases where
     * TRANSLATE_EOL != 0).  Use "-aa" (setting "all_ascii") to evade
     * the test in util.c:is_text_buf(), if the old (always-convert)
     * behavior is preferred.
     * Replaced old conversion code which looped until it found the
     * first '\0' in a multi-line buffer with code which stops at "len".
     * Zip Bugs forum topic: "BUG in zip 31c & 31d25 on z/OS Unix".  If
     * detection and/or elimination of '\0' characters is needed, then
     * it needs to be (much) smarter (multi-line, stateful, ...).
     */
    if (aflag == ASCII)
    {
      unsigned i;

      /* Check buf for binary (once, at the first read operation). */
      if (file_binary < 0)
      {
        file_binary = is_text_buf( buf, len) ? 0 : 1;
      }

      /* If text, then convert EBCDIC to ASCII. */
      if (file_binary == 0)
      {
        for (i = 0; i < len; i++)
        {
          buf[ i] = (char)ascii[ (uch)buf[ i]];
        }
      }
    }
#endif
  } else if (TRANSLATE_EOL == 1) {
    /* translate_eol == 1 */
    /* Transform LF to CR LF */
    size >>= 1;
    b = buf+size;
    size = len = zread(ifile, b, size);
    bytes_read_this_entry += len;
    if (len == (unsigned)EOF || len == 0) return len;

    /* Check b for binary (once, at the first read operation). */
    if (file_binary < 0)
    {
      file_binary = is_text_buf( b, len) ? 0 : 1;
    }

    /* If text, then convert EBCDIC to ASCII, and/or adjust line endings. */
    if (file_binary == 0)
    {
#ifdef EBCDIC
      if (aflag == ASCII)
      {
         do {
            char c;

            if ((c = *b++) == '\n') {
               *buf++ = CR; *buf++ = LF; len++;
            } else {
              *buf++ = (char)ascii[(uch)c];
            }
         } while (--size != 0);
      }
      else
#endif /* EBCDIC */
      {
         do {
            if ((*buf++ = *b++) == '\n') *(buf-1) = CR, *buf++ = LF, len++;
         } while (--size != 0);
      }
      buf -= len;
    } else { /* do not translate binary */
      memcpy(buf, b, len);
    }

  } else {
    /* translate_eol == 2 */
    /* Transform CR LF to LF and suppress final ^Z */
    b = buf;
    size = len = zread(ifile, buf, size-1);
    bytes_read_this_entry += len;
    if (len == (unsigned)EOF || len == 0) return len;

    /* Check buf for binary (once, at the first read operation). */
    if (file_binary < 0)
    {
      file_binary = is_text_buf( b, len) ? 0 : 1;
    }

    /* If text, then convert EBCDIC to ASCII, and/or adjust line endings. */
    if (file_binary == 0)
    {
      buf[len] = '\n'; /* I should check if next char is really a \n */
#ifdef EBCDIC
      if (aflag == ASCII)
      {
         do {
            char c;

            if ((c = *b++) == '\r' && *b == '\n') {
               len--;
            } else {
               *buf++ = (char)(c == '\n' ? LF : ascii[(uch)c]);
            }
         } while (--size != 0);
      }
      else
#endif /* EBCDIC */
      {
         do {
            if (( *buf++ = *b++) == CR && *b == LF) buf--, len--;
         } while (--size != 0);
      }
      if (len == 0) {
         zread(ifile, buf, 1); len = 1; /* keep single \r if EOF */
         bytes_read_this_entry += len;
#ifdef EBCDIC
         if (aflag == ASCII) {
            *buf = (char)(*buf == '\n' ? LF : ascii[(uch)(*buf)]);
         }
#endif
      } else {
         buf -= len;
         if (buf[len-1] == CTRLZ) len--; /* suppress final ^Z */
      }
    }
  }
  crc = crc32(crc, (uch *) buf, len);
  /* 2005-05-23 SMS.
     Increment file size.  A small-file program reading a large file may
     cause isize to overflow, so complain (and abort) if it goes
     negative or wraps around.  Awful things happen later otherwise.
  */
  isize_prev = isize;
  isize += (ulg)len;
  if (isize < isize_prev) {
    ZIPERR(ZE_BIG, "overflow in byte count");
  }

#ifdef ENABLE_DLL_PROGRESS
  /* If progress_chunk_size is defined and ProgressReport() exists,
     see if time to send user progress information. */
/*  if (progress_chunk_size > 0) { */
  if (progress_chunk_size > 0 && lpZipUserFunctions->ProgressReport != NULL) {
    current_progress_chunk = bytes_read_this_entry / progress_chunk_size;
    if (current_progress_chunk > last_progress_chunk) {
      /* Send progress report to application */
      last_progress_chunk = current_progress_chunk;
      /* Callback parameters:
            name, total bytes so far, estimated total bytes to be read,
            this entry bytes so far, total bytes this entry
      */
      if (bytes_total > 0) {
        if (bytes_total < ((uzoff_t)1 << (sizeof(uzoff_t) * 8 - 15))) {
          percent_all_entries_processed
            = (long)((10000 * (bytes_so_far + bytes_read_this_entry)) / bytes_total);
        } else {
          /* avoid overflow, but this is inaccurate for small numbers */
          bytetotal = bytes_total / 10000;
          if (bytetotal == 0) bytetotal = 1;
          percent_all_entries_processed
            = (long)((bytes_so_far + bytes_read_this_entry) / bytetotal);
        }
      } else{
        percent_all_entries_processed = 0;
      }
      if (bytes_expected_this_entry > 0) {
        if (bytes_expected_this_entry < ((uzoff_t)1 << (sizeof(uzoff_t) * 8 - 15))) {
          percent_this_entry_processed
            = (long)((10000 * bytes_read_this_entry) / bytes_expected_this_entry);
        } else {
          /* avoid overflow, but this is inaccurate for small numbers */
          bytetotal = bytes_expected_this_entry / 10000;
          if (bytetotal == 0) bytetotal = 1;
          percent_this_entry_processed = (long)(bytes_read_this_entry / bytetotal);
        }
      }
      else {
        percent_this_entry_processed = 0;
      }

      /* This is a kluge.  The numbers should add up. */
      if (percent_all_entries_processed > 10000) {
        percent_all_entries_processed = 10000;
      }

/*
      printf("\n  progress %5.2f%% all  %5.2f%%  %s\n", (float)percent_all_entries_processed/100,
                                                        (float)percent_this_entry_processed/100,
                                                         entry_name);
*/

      if ((*lpZipUserFunctions->ProgressReport)(entry_name,
                                                percent_all_entries_processed,
                                                percent_this_entry_processed)) {
        ZIPERR(ZE_ABORT, "User terminated operation");
      }
    }
  }
#endif

  return len;
}


/* Read a buffer using iz_file_read() and set file_binary_final
 * according to the buffer contents.
 */

unsigned iz_file_read_bt( buf, size)
  char *buf;
  unsigned size;
{
  unsigned cnt;

  cnt = iz_file_read( buf, size);
  if ((cnt > 0) && (file_binary_final == 0))
  {
    if (!is_text_buf( (char *)buf, cnt))
      file_binary_final = 1;
  }
  return cnt;
}


#ifdef USE_ZLIB

local int zl_deflate_init(pack_level)
    int pack_level;
{
    unsigned i;
    int windowBits;
    int err = Z_OK;
    int zp_err = ZE_OK;

    if (zlib_version[0] != ZLIB_VERSION[0]) {
        sprintf(errbuf, "incompatible zlib version (expected %s, found %s)",
              ZLIB_VERSION, zlib_version);
        zp_err = ZE_LOGIC;
    } else if (strcmp(zlib_version, ZLIB_VERSION) != 0) {
        fprintf(mesg,
         "        warning:  different zlib version (expected %s, using %s)\n",
         ZLIB_VERSION, zlib_version);
    }

    /* windowBits = log2(WSIZE) */
    for (i = ((unsigned)WSIZE), windowBits = 0; i != 1; i >>= 1, ++windowBits);

    zstrm.zalloc = (alloc_func)Z_NULL;
    zstrm.zfree = (free_func)Z_NULL;

    Trace((stderr, "initializing deflate()\n"));
    err = deflateInit2(&zstrm, pack_level, Z_DEFLATED, -windowBits, 8, 0);

    if (err == Z_MEM_ERROR) {
        sprintf(errbuf, "cannot initialize zlib deflate");
        zp_err = ZE_MEM;
    } else if (err != Z_OK) {
        sprintf(errbuf, "zlib deflateInit failure (%d)", err);
        zp_err = ZE_LOGIC;
    }

    deflInit = TRUE;
    return zp_err;
}


void zl_deflate_free()
{
    int err;

    if (f_obuf != NULL) {
        free(f_obuf);
        f_obuf = NULL;
    }
    if (f_ibuf != NULL) {
        free(f_ibuf);
        f_ibuf = NULL;
    }
    if (deflInit) {
        err = deflateEnd(&zstrm);
        if (err != Z_OK && err !=Z_DATA_ERROR) {
            ziperr(ZE_LOGIC, "zlib deflateEnd failed");
        }
        deflInit = FALSE;
    }
}

#else /* !USE_ZLIB */

# ifdef ZP_NEED_MEMCOMPR
/* ===========================================================================
 * In-memory read function. As opposed to iz_file_read(), this function
 * does not perform end-of-line translation, and does not update the
 * crc and input size.
 *    Note that the size of the entire input buffer is an unsigned long,
 * but the size used in mem_read() is only an unsigned int. This makes a
 * difference on 16 bit machines. mem_read() may be called several
 * times for an in-memory compression.
 */
local unsigned mem_read(b, bsize)
     char *b;
     unsigned bsize;
{
    if (in_offset < in_size) {
        ulg block_size = in_size - in_offset;
        if (block_size > (ulg)bsize) block_size = (ulg)bsize;
        memcpy(b, in_buf + in_offset, (unsigned)block_size);
        in_offset += (unsigned)block_size;
        return (unsigned)block_size;
    } else {
        return 0; /* end of input */
    }
}
# endif /* ZP_NEED_MEMCOMPR */


/* ===========================================================================
 * Flush the current output buffer.
 */
void flush_outbuf(o_buf, o_idx)
    char *o_buf;
    unsigned *o_idx;
{
    if (y == NULL) {
        error("output buffer too small for in-memory compression");
    }
    /* Encrypt and write the output buffer: */
    if (*o_idx != 0) {
        zfwrite(o_buf, 1, (extent)*o_idx);
        if (ferror(y)) ziperr(ZE_WRITE, "write error on zip file");
    }
    *o_idx = 0;
}

/* ===========================================================================
 * Return true if the zip file can be seeked. This is used to check if
 * the local header can be re-rewritten. This function always returns
 * true for in-memory compression.
 * IN assertion: the local header has already been written (ftell() > 0).
 */
int seekable()
{
    return fseekable(y);
}
#endif /* ?USE_ZLIB */


/* ===========================================================================
 * Compression to archive file.
 */
local zoff_t filecompress(z_entry, cmpr_method)
    struct zlist far *z_entry;
    int *cmpr_method;
{
#ifdef USE_ZLIB
    int err = Z_OK;
    unsigned mrk_cnt = 1;
    int maybe_stored = FALSE;
    ulg cmpr_size;
# if defined(MMAP) || defined(BIG_MEM)
    unsigned ibuf_sz = (unsigned)SBSZ;
# else
#   define ibuf_sz ((unsigned)SBSZ)
# endif
# ifndef OBUF_SZ
#  define OBUF_SZ ZBSZ
# endif

# if defined(MMAP) || defined(BIG_MEM)
    if (remain == (ulg)-1L && f_ibuf == NULL)
# else /* !(MMAP || BIG_MEM */
    if (f_ibuf == NULL)
# endif /* MMAP || BIG_MEM */
        f_ibuf = (unsigned char *)malloc(SBSZ);
    if (f_obuf == NULL)
        f_obuf = (unsigned char *)malloc(OBUF_SZ);
# if defined(MMAP) || defined(BIG_MEM)
    if ((remain == (ulg)-1L && f_ibuf == NULL) || f_obuf == NULL)
# else /* !(MMAP || BIG_MEM */
    if (f_ibuf == NULL || f_obuf == NULL)
# endif /* MMAP || BIG_MEM */
        ziperr(ZE_MEM, "allocating zlib file-I/O buffers");

    if (!deflInit) {
        err = zl_deflate_init(levell);
        if (err != ZE_OK)
            ziperr(err, errbuf);
    }

    if (levell <= 2) {
        z_entry->flg |= 4;
    } else if (levell >= 8) {
        z_entry->flg |= 2;
    }
# if defined(MMAP) || defined(BIG_MEM)
    if (remain != (ulg)-1L) {
        zstrm.next_in = (Bytef *)window;
        ibuf_sz = (unsigned)WSIZE;
    } else
# endif /* MMAP || BIG_MEM */
    {
        zstrm.next_in = (Bytef *)f_ibuf;
    }
    zstrm.avail_in = iz_file_read((char *)zstrm.next_in, ibuf_sz);
    if (zstrm.avail_in < ibuf_sz) {
        unsigned more = iz_file_read((char *)(zstrm.next_in + zstrm.avail_in),
                                  (ibuf_sz - zstrm.avail_in));
        if (more == (unsigned)EOF || more == 0) {
            maybe_stored = TRUE;
        } else {
            zstrm.avail_in += more;
        }
    }
    bytes_so_far += zstrm.avail_in;
    zstrm.next_out = (Bytef *)f_obuf;
    zstrm.avail_out = OBUF_SZ;

    if (!maybe_stored)
    while (zstrm.avail_in != 0 && zstrm.avail_in != (uInt)EOF) {
        err = deflate(&zstrm, Z_NO_FLUSH);
        if (err != Z_OK && err != Z_STREAM_END) {
            sprintf(errbuf, "unexpected zlib deflate error %d", err);
            ziperr(ZE_LOGIC, errbuf);
        }
        if (zstrm.avail_out == 0) {
            if (zfwrite(f_obuf, 1, OBUF_SZ) != OBUF_SZ) {
                ziperr(ZE_TEMP, "error writing to zipfile");
            }
            zstrm.next_out = (Bytef *)f_obuf;
            zstrm.avail_out = OBUF_SZ;
        }
        if (zstrm.avail_in == 0) {
            if (verbose || noisy)
                while((unsigned)(zstrm.total_in / (uLong)WSIZE) > mrk_cnt) {
                    mrk_cnt++;


                    /* deflate */

                    /* display dots */
                    if (!display_globaldots)
                    {
                      display_dot( 1, WSIZE);
                    }
                }
# if defined(MMAP) || defined(BIG_MEM)
            if (remain == (ulg)-1L)
                zstrm.next_in = (Bytef *)f_ibuf;
# else
            zstrm.next_in = (Bytef *)f_ibuf;
# endif
            zstrm.avail_in = iz_file_read((char *)zstrm.next_in, ibuf_sz);
            bytes_so_far += zstrm.avail_in;
        }
    }

    do {
        err = deflate(&zstrm, Z_FINISH);
        if (maybe_stored) {
            if (err == Z_STREAM_END && zstrm.total_out >= zstrm.total_in &&
                fseekable(y)) {
                /* deflation does not reduce size, switch to STORE method */
                unsigned len_out = (unsigned)zstrm.total_in;
                if (zfwrite(f_ibuf, 1, len_out) != len_out) {
                    ziperr(ZE_TEMP, "error writing to zipfile");
                }
                zstrm.total_out = (uLong)len_out;
                *cmpr_method = STORE;
                break;
            } else {
                maybe_stored = FALSE;
            }
        }
        if (zstrm.avail_out < OBUF_SZ) {
            unsigned len_out = OBUF_SZ - zstrm.avail_out;
            if (zfwrite(f_obuf, 1, len_out) != len_out) {
                ziperr(ZE_TEMP, "error writing to zipfile");
            }
            zstrm.next_out = (Bytef *)f_obuf;
            zstrm.avail_out = OBUF_SZ;
        }
    } while (err == Z_OK);

    if (err != Z_STREAM_END) {
        sprintf(errbuf, "unexpected zlib deflate error %d", err);
        ziperr(ZE_LOGIC, errbuf);
    }

    if (z_entry->att == (ush)UNKNOWN)
        z_entry->att = (ush)(zstrm.data_type == Z_ASCII ? ASCII : BINARY);
    cmpr_size = (ulg)zstrm.total_out;

    if ((err = deflateReset(&zstrm)) != Z_OK)
        ziperr(ZE_LOGIC, "zlib deflateReset failed");
    return cmpr_size;
#else /* !USE_ZLIB */

    /* Set the defaults for file compression. */
    read_buf = iz_file_read;

    /* Initialize deflate's internals and execute file compression. */
    bi_init(file_outbuf, sizeof(file_outbuf), TRUE);
    ct_init(&z_entry->att, cmpr_method);
    lm_init(levell, &z_entry->flg);
    return deflate();
#endif /* ?USE_ZLIB */
}

#ifdef ZP_NEED_MEMCOMPR
/* ===========================================================================
 * In-memory compression. This version can be used only if the entire input
 * fits in one memory buffer. The compression is then done in a single
 * call of memcompress(). (An extension to allow repeated calls would be
 * possible but is not needed here.)
 * The first two bytes of the compressed output are set to a short with the
 * method used (DEFLATE or STORE). The following four bytes contain the CRC.
 * The values are stored in little-endian order on all machines.
 * This function returns the byte size of the compressed output, including
 * the first six bytes (method and crc).
 */

ulg memcompress(tgt, tgtsize, src, srcsize)
    char *tgt, *src;       /* target and source buffers */
    ulg tgtsize, srcsize;  /* target and source sizes */
{
    ulg crc;
    unsigned out_total;
    int method   = DEFLATE;
#ifdef USE_ZLIB
    int err      = Z_OK;
#else
    ush att      = (ush)UNKNOWN;
    ush flags    = 0;
#endif

    if (tgtsize <= (ulg)6L) error("target buffer too small");
    out_total = 2 + 4;

#ifdef USE_ZLIB
    if (!deflInit) {
        err = zl_deflate_init(levell);
        if (err != ZE_OK)
            ziperr(err, errbuf);
    }

    zstrm.next_in = (Bytef *)src;
    zstrm.avail_in = (uInt)srcsize;
    zstrm.next_out = (Bytef *)(tgt + out_total);
    zstrm.avail_out = (uInt)tgtsize - (uInt)out_total;

    err = deflate(&zstrm, Z_FINISH);
    if (err != Z_STREAM_END)
        error("output buffer too small for in-memory compression");
    out_total += (unsigned)zstrm.total_out;

    if ((err = deflateReset(&zstrm)) != Z_OK)
        error("zlib deflateReset failed");
#else /* !USE_ZLIB */
    read_buf  = mem_read;
    in_buf    = src;
    in_size   = (unsigned)srcsize;
    in_offset = 0;
    window_size = 0L;

    bi_init(tgt + (2 + 4), (unsigned)(tgtsize - (2 + 4)), FALSE);
    ct_init(&att, &method);
    lm_init((levell != 0 ? levell : 1), &flags);
    out_total += (unsigned)deflate();
    window_size = 0L; /* was updated by lm_init() */
#endif /* ?USE_ZLIB */

    crc = CRCVAL_INITIAL;
    crc = crc32(crc, (uch *)src, (extent)srcsize);

    /* For portability, force little-endian order on all machines: */
    tgt[0] = (char)(method & 0xff);
    tgt[1] = (char)((method >> 8) & 0xff);
    tgt[2] = (char)(crc & 0xff);
    tgt[3] = (char)((crc >> 8) & 0xff);
    tgt[4] = (char)((crc >> 16) & 0xff);
    tgt[5] = (char)((crc >> 24) & 0xff);

    return (ulg)out_total;
}
#endif /* ZP_NEED_MEMCOMPR */


/* ===========================================================================
 * BZIP2 Compression
 */

#ifdef BZIP2_SUPPORT

local int bz_compress_init(pack_level)
int pack_level;
{
    int err = BZ_OK;
    int zp_err = ZE_OK;
    const char *bzlibVer;

    bzlibVer = BZ2_bzlibVersion();

    /* $TODO - Check BZIP2 LIB version? */

    bstrm.bzalloc = NULL;
    bstrm.bzfree = NULL;
    bstrm.opaque = NULL;

    Trace((stderr, "initializing bzlib compress()\n"));
    err = BZ2_bzCompressInit(&bstrm, pack_level, 0, 30);

    if (err == BZ_MEM_ERROR) {
        sprintf(errbuf, "cannot initialize bzlib compress");
        zp_err = ZE_MEM;
    } else if (err != BZ_OK) {
        sprintf(errbuf, "bzlib bzCompressInit failure (%d)", err);
        zp_err = ZE_LOGIC;
    }

    bzipInit = TRUE;
    return zp_err;
}


void bz_compress_free()
{
    int err;

    if (f_obuf != NULL) {
        free(f_obuf);
        f_obuf = NULL;
    }
    if (f_ibuf != NULL) {
        free(f_ibuf);
        f_ibuf = NULL;
    }
    if (bzipInit) {
        err = BZ2_bzCompressEnd(&bstrm);
        if (err != BZ_OK && err != BZ_DATA_ERROR) {
            ziperr(ZE_LOGIC, "bzlib bzCompressEnd failed");
        }
        bzipInit = FALSE;
    }
}


local zoff_t bzfilecompress(z_entry, cmpr_method)
struct zlist far *z_entry;
int *cmpr_method;
{
    int err = BZ_OK;
    unsigned mrk_cnt = 1;
    int maybe_stored = FALSE;
    zoff_t cmpr_size;
#if defined(MMAP) || defined(BIG_MEM)
    unsigned ibuf_sz = (unsigned)SBSZ;
#else
#   define ibuf_sz ((unsigned)SBSZ)
#endif
#ifndef OBUF_SZ
#  define OBUF_SZ ZBSZ
#endif

#if defined(MMAP) || defined(BIG_MEM)
    if (remain == (ulg)-1L && f_ibuf == NULL)
#else /* !(MMAP || BIG_MEM */
    if (f_ibuf == NULL)
#endif /* MMAP || BIG_MEM */
        f_ibuf = (unsigned char *)malloc(SBSZ);
    if (f_obuf == NULL)
        f_obuf = (unsigned char *)malloc(OBUF_SZ);
#if defined(MMAP) || defined(BIG_MEM)
    if ((remain == (ulg)-1L && f_ibuf == NULL) || f_obuf == NULL)
#else /* !(MMAP || BIG_MEM */
    if (f_ibuf == NULL || f_obuf == NULL)
#endif /* MMAP || BIG_MEM */
        ziperr(ZE_MEM, "allocating zlib/bzlib file-I/O buffers");

    if (!bzipInit) {
        err = bz_compress_init(levell);
        if (err != ZE_OK)
            ziperr(err, errbuf);
    }

#if defined(MMAP) || defined(BIG_MEM)
    if (remain != (ulg)-1L) {
        bstrm.next_in = (Bytef *)window;
        ibuf_sz = (unsigned)WSIZE;
    } else
#endif /* MMAP || BIG_MEM */
    {
        bstrm.next_in = (char *)f_ibuf;
    }

    bstrm.avail_in = iz_file_read_bt(bstrm.next_in, ibuf_sz);

    if (bstrm.avail_in < ibuf_sz) {
        /* Should this be iz_file_read_bt(), too? */
        unsigned more = iz_file_read(bstrm.next_in + bstrm.avail_in,
                                  (ibuf_sz - bstrm.avail_in));
        if (more == (unsigned) EOF || more == 0) {
            maybe_stored = TRUE;
        } else {
            bstrm.avail_in += more;
        }
    }
    bytes_so_far += bstrm.avail_in;

    bstrm.next_out = (char *)f_obuf;
    bstrm.avail_out = OBUF_SZ;

    if (!maybe_stored) {
      while (bstrm.avail_in != 0 && bstrm.avail_in != (unsigned) EOF) {
        err = BZ2_bzCompress(&bstrm, BZ_RUN);
        if (err != BZ_RUN_OK && err != BZ_STREAM_END) {
            sprintf(errbuf, "unexpected bzlib compress error %d", err);
            ziperr(ZE_LOGIC, errbuf);
        }
        if (bstrm.avail_out == 0) {
            if (zfwrite(f_obuf, 1, OBUF_SZ) != OBUF_SZ) {
                ziperr(ZE_TEMP, "error writing to zipfile");
            }
            bstrm.next_out = (char *)f_obuf;
            bstrm.avail_out = OBUF_SZ;
        }
        /* $TODO what about high 32-bits of total-in??? */
        if (bstrm.avail_in == 0) {
            if (verbose || noisy)
#ifdef LARGE_FILE_SUPPORT
                while((unsigned)((bstrm.total_in_lo32
                                  + (((zoff_t)bstrm.total_in_hi32) << 32))
                                 / (zoff_t)(ulg)WSIZE) > mrk_cnt) {
#else
                while((unsigned)(bstrm.total_in_lo32 / (ulg)WSIZE) > mrk_cnt) {
#endif
                    mrk_cnt++;


                    /* bzip2 */

                    /* display dots */
                    if (!display_globaldots)
                    {
                      display_dot( 1, WSIZE);
                    }
                }
#if defined(MMAP) || defined(BIG_MEM)
            if (remain == (ulg)-1L)
                bstrm.next_in = (char *)f_ibuf;
#else
            bstrm.next_in = (char *)f_ibuf;
#endif

            bstrm.avail_in = iz_file_read_bt( bstrm.next_in, ibuf_sz);
            bytes_so_far += bstrm.avail_in;
        }
      }
    }

    /* binary or text */
    if (file_binary_final)
      /* found binary in file */
      z_entry->att = (ush)BINARY;
    else
      /* text file */
      z_entry->att = (ush)ASCII;

    do {
        err = BZ2_bzCompress(&bstrm, BZ_FINISH);
        if (maybe_stored) {
            /* This code is only executed when the complete data stream fits
               into the input buffer (see above where maybe_stored gets set).
               So, it is safe to assume that total_in_hi32 (and total_out_hi32)
               are 0, because the input buffer size is well below the 32-bit
               limit.
             */
            if (err == BZ_STREAM_END
                && bstrm.total_out_lo32 >= bstrm.total_in_lo32
                && fseekable(y)) {
                /* BZIP2 compress does not reduce size,
                   switch to STORE method */
                unsigned len_out = (unsigned)bstrm.total_in_lo32;
                if (zfwrite(f_ibuf, 1, len_out) != len_out) {
                    ziperr(ZE_TEMP, "error writing to zipfile");
                }
                bstrm.total_out_lo32 = (ulg)len_out;
                *cmpr_method = STORE;
                break;
            } else {
                maybe_stored = FALSE;
            }
        }
        if (bstrm.avail_out < OBUF_SZ) {
            unsigned len_out = OBUF_SZ - bstrm.avail_out;
            if (zfwrite(f_obuf, 1, len_out) != len_out) {
                ziperr(ZE_TEMP, "error writing to zipfile");
            }
            bstrm.next_out = (char *)f_obuf;
            bstrm.avail_out = OBUF_SZ;
        }
    } while (err == BZ_FINISH_OK);

    if (err < BZ_OK) {
        sprintf(errbuf, "unexpected bzlib compress error %d", err);
        ziperr(ZE_LOGIC, errbuf);
    }

    if (z_entry->att == (ush)UNKNOWN)
        z_entry->att = (ush)BINARY;
#ifdef LARGE_FILE_SUPPORT
    cmpr_size = (zoff_t)bstrm.total_out_lo32
               + (((zoff_t)bstrm.total_out_hi32) << 32);
#else
    cmpr_size = (zoff_t)bstrm.total_out_lo32;
#endif

    if ((err = BZ2_bzCompressEnd(&bstrm)) != BZ_OK)
        ziperr(ZE_LOGIC, "zlib deflateReset failed");
    bzipInit = FALSE;
    return cmpr_size;
}

#endif /* BZIP2_SUPPORT */


/* ===========================================================================
 * LZMA Compression
 */

#ifdef LZMA_SUPPORT

/* 7-Zip-compatible I/O Read and Write functions. */

static SRes lzma_read( void *p, void *buf, size_t *size)
{
  int size0;

  errno = 0;                            /* iz_file_read_bt() sets errno. */
  size0 = *size;
  *size = iz_file_read_bt( buf, size0);

  return errno;
}


/* 2012-03-02 SMS.
 * We call this write function first, with the four-byte LZMA Zip
 * properties header (version, LZMA properties size).  LzmaEnc_Encode(),
 * calls it once with the five-byte (LzmaEnc.h:LZMA_PROPS_SIZE) LZMA
 * properties header, then as needed with up to 64KB
 * (LzmaEnc.c:RC_BUF_SIZE?).
 *
 * With no obvious (reliable) way to identify the last call, identifying
 * the case where reverting to the STORE method (all the data fit into
 * one buffer, and the "compressed" data are bigger than the original
 * data) would seem to require additional buffering (for at least the
 * header, and, perhaps, for the old input, which might get lost by the
 * time we would need it again), and a scheme to defer actual writing
 * for the (two-part) header and for the next (compressed data) buffer.
 */

local zoff_t lzma_compressed_size;      /* Output byte count. */

static size_t lzma_write( void *p, const void *buf, size_t size)
{
  int bytes_written;

  bytes_written = zfwrite( (void *)buf, 1, size);
  if (bytes_written > 0)
  {
    lzma_compressed_size += bytes_written;
  }
  return bytes_written;
}


/* LZMA progress function. */

int lzma_progress_function(void *v, UInt64 a, UInt64 b)
{
  UInt64 bufsize = 32819;

#if 0
  printf("progress  uncompressed read %I64d  compressed written %I64d\n", a, b);
#endif /* 0 */

  /* display dots every so many bytes */
  if (!display_globaldots)
  {
    display_dot( 0, (ulg)bufsize);
  }
  return 0;
}


local SRes LZMA_Encode(struct zlist far *z_entry,
 ISeqOutStream *outStream, ISeqInStream *inStream,
 UInt64 fileSize)
{
  CLzmaEncHandle enc;
  CLzmaEncProps props;
  ICompressProgress lzma_progress;
  SRes res;

  lzma_progress.Progress = lzma_progress_function;

  /* Create LZMA data structures. */
  enc = LzmaEnc_Create( &g_Alloc);
  if (enc == 0)
    return SZ_ERROR_MEM;

  /* Initialize LZMA properties. */
  LzmaEncProps_Init( &props);

  /* Set the LZMA level to the Zip level - 1, which seems roughly
     appropriate.  So the Zip default level 6 becomes the LZMA
     default level 5.  Zip level 0 (store) should not get here.

     This is just an initial mapping.  Initial testing suggests
     that it may be better to drift some from the standard LZMA
     level settings and change the settings in props directly to
     get a better range of results when setting Zip compression
     level from 1 to 9.  - EG
  */
  if (levell > 0)
    props.level = levell - 1;
  else
    props.level = 0;

  /* Set LZMA encoding parameters, using props.
   * Uses props.level to set various other props.
   */
  res = LzmaEnc_SetProps( enc, &props);

  if (res == SZ_OK)
  {
    Byte header[ LZMA_PROPS_SIZE];
    size_t headerSize = LZMA_PROPS_SIZE;

    /* Fill in the LZMA header. */
    res = LzmaEnc_WriteProperties( enc, header, &headerSize);

#if 0
    /* Apparently not included for LZMA entries in zip archives. */
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(fileSize >> (8 * i));
#endif

    if (outStream->Write(outStream, header, headerSize) != headerSize)
      res = SZ_ERROR_WRITE;
    else
    {
      if (res == SZ_OK)
        res = LzmaEnc_Encode(enc, outStream, inStream, &lzma_progress,
         &g_Alloc, &g_Alloc);
    }
  }

  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);

  /* Set the "internal file attributes" word to text or binary,
   * according to what is_text_buf() determined.
   */
  if (file_binary_final)
    z_entry->att = (ush)BINARY;
  else
    z_entry->att = (ush)ASCII;

  return res;
}


local zoff_t lzma_filecompress(z_entry, cmpr_method)
  struct zlist far *z_entry;
  int *cmpr_method;
{
  ISeqInStream inStream;
  ISeqOutStream outStream;
  int res;
  uzoff_t filesize = z_entry->len;

  unsigned char lzma_properties_header[4];

  inStream.Read = lzma_read;
  outStream.Write = lzma_write;
  lzma_compressed_size = 0;

  {
    size_t t4 = sizeof(UInt32);
    size_t t8 = sizeof(UInt64);
    if (t4 != 4 || t8 != 8)
      ZIPERR(ZE_COMPERR, "LZMA UInt32 or UInt64 wrong size");
  }

  /* Write out 4 bytes of LZMA Zip header: version, LZMA properties size. */
  lzma_properties_header[0] = MY_VER_MAJOR;
  lzma_properties_header[1] = MY_VER_MINOR;
  lzma_properties_header[2] = LZMA_PROPS_SIZE & 0xff;
  lzma_properties_header[3] = LZMA_PROPS_SIZE >> 8;

  lzma_write( NULL, lzma_properties_header, 4);

  /* LZMA encode.  (Puts out LZMA properties, and compressed data.) */
  res = LZMA_Encode( z_entry, &outStream, &inStream, filesize);
  if (res != SZ_OK)
  {
    if (res == SZ_ERROR_MEM) {
      sprintf(errbuf, "LZMA error %d, %s", res, kCantAllocateMessage);
      ZIPERR(ZE_COMPRESS, errbuf);
    }
    else if (res == SZ_ERROR_DATA) {
      sprintf(errbuf, "LZMA error %d, %s", res, kDataErrorMessage);
      ZIPERR(ZE_COMPRESS, errbuf);
    }
    else if (res == SZ_ERROR_WRITE) {
      sprintf(errbuf, "LZMA error %d, %s", res, kCantWriteMessage);
      ZIPERR(ZE_COMPRESS, errbuf);
    }
    else if (res == SZ_ERROR_READ) {
      sprintf(errbuf, "LZMA error %d, %s", res, kCantReadMessage);
      ZIPERR(ZE_COMPRESS, errbuf);
    }
    sprintf(errbuf, "LZMA error %d", res);
    ZIPERR(ZE_COMPRESS, errbuf);
  }
  return lzma_compressed_size;
}

#endif /* LZMA_SUPPORT */


/* ===========================================================================
 * PPMd Compression
 */

#ifdef PPMD_SUPPORT

/* 7-Zip I/O structure (reduced). */
typedef struct
{
  IByteOut ibo;
  int byte_count;
  int buf_size;
  zoff_t compressed_size;
} CByteOutToSeq;


/* 7-Zip-compatible I/O Write (one byte, blindly) function. */
static void ppmd_write_byte( void *pp, unsigned char uc)
{
  CByteOutToSeq *p = (CByteOutToSeq *)pp;

  if (p->byte_count >= p->buf_size)
  {
    zfwrite( f_obuf, 1, p->byte_count);
    p->compressed_size += p->byte_count;
    p->byte_count = 0;
  }
  f_obuf[ p->byte_count++] = uc;
}


local zoff_t ppmd_filecompress(z_entry, cmpr_method)
  struct zlist far *z_entry;
  int *cmpr_method;
{
  /* PPMd parameters. */
  unsigned order;
  unsigned memSize;
  unsigned restor;
  unsigned short ppmd_param_word;

  /* PPMd structure. */
  CPpmd8 ppmd8;

  /* 7-Zip-compatible I/O structure. */
  CByteOutToSeq cbots;

  int byte_cnt_in;
  int i;
  int sts;

#if defined(MMAP) || defined(BIG_MEM)
  unsigned ibuf_sz = (unsigned)SBSZ;
#else
#  define ibuf_sz ((unsigned)SBSZ)
#endif
#ifndef OBUF_SZ
#  define OBUF_SZ ZBSZ
#endif

#if defined(MMAP) || defined(BIG_MEM)
  if (remain == (ulg)-1L && f_ibuf == NULL)
#else /* !(MMAP || BIG_MEM */
  if (f_ibuf == NULL)
#endif /* MMAP || BIG_MEM */
    f_ibuf = (unsigned char *)malloc(SBSZ);
  if (f_obuf == NULL)
    f_obuf = (unsigned char *)malloc(OBUF_SZ);
#if defined(MMAP) || defined(BIG_MEM)
  if ((remain == (ulg)-1L && f_ibuf == NULL) || f_obuf == NULL)
#else /* !(MMAP || BIG_MEM */
  if (f_ibuf == NULL || f_obuf == NULL)
#endif /* MMAP || BIG_MEM */
    ziperr(ZE_MEM, "allocating PPMd file-I/O buffers");

  ppmd8.Stream.Out = &cbots.ibo;
  cbots.ibo.Write = ppmd_write_byte;
  cbots.byte_count = 0;
  cbots.compressed_size = 0;
  cbots.buf_size = OBUF_SZ;

  /* PPMD8_MIN_ORDER (= 2) <= order <= PPMD8_MAX_ORDER (= 16)
   * 1MB (= 2* 0) <= memSize <= 256MB (= 2^ 8)   (M = 2^ 20)
   * restor = 0 (PPMD8_RESTORE_METHOD_RESTART),
   *          1 (PPMD8_RESTORE_METHOD_CUT_OFF)
   *
   * Currently using 7-Zip-compatible values:
   *    order = 3 + level
   *    memory size = 2^ (level- 1) (MB)  (level 9 treated as level 8.)
   *    restoration method = 0 for level <= 6, 1 for level >= 7.
   *
   */
  order = 3+ levell;                        /* 4, 5, 6, ..., 12. */
  memSize = 1<< (IZ_MIN( levell, 8)- 1);    /* 1MB, 2MB, 4MB, ..., 128MB. */
  restor = (levell <= 6 ? 0 : 1);

  memSize <<= 20;                       /* Convert B to MB. */

  Ppmd8_Construct( &ppmd8);
  sts = Ppmd8_Alloc( &ppmd8, memSize, &g_Alloc);

  if (sts <= 0)
    ziperr( ZE_MEM, "allocating PPMd storage");

  Ppmd8_RangeEnc_Init( &ppmd8);
  Ppmd8_Init( &ppmd8, order, restor);

/* wPPMd = (Model order - 1) +
 *         ((Sub-allocator size - 1) << 4) +
 *         (Model restoration method << 12)
 *
 *  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 *  Mdl_Res_Mth ___Sub-allocator_size-1 Mdl_Order-1
 */

  /* Form the PPMd properties word.  Put out the bytes. */
  ppmd_param_word = ((order- 1)& 0xf)+
   ((((memSize>> 20)- 1)& 0xff)<< 4)+
   ((restor& 0xf)<< 12);

  ppmd_write_byte( ppmd8.Stream.Out, (ppmd_param_word& 0xff));
  ppmd_write_byte( ppmd8.Stream.Out, (ppmd_param_word>> 8));

  sts = 0;
  while (sts == 0)
  {
    byte_cnt_in = iz_file_read_bt( (char *)f_ibuf, ibuf_sz);
    if (byte_cnt_in > 0)
    {
      if (byte_cnt_in >= ibuf_sz)
      {
        /* Display progress dots. */
        if (!display_globaldots)
        {
          display_dot( 0, ibuf_sz);
        }
      }
      for (i = 0; i < byte_cnt_in; i++)
      {
        /* Note: Passing Ppmd8_EncodeSymbol() a _signed_ char (for its
         * "int" argument) causes 0xff to be treated as EOF (= -1),
         * which thoroughly confuses the PPMd code.
         */
        Ppmd8_EncodeSymbol( &ppmd8, f_ibuf[ i]);
      }
    }
    else
    {
      sts = -1;
      Ppmd8_EncodeSymbol( &ppmd8, -1);
      Ppmd8_RangeEnc_FlushData( &ppmd8);
    }
  }

  /* Revert to STORE if:
   *   the output buffer has not been flushed, and
   *   the uncompressed size is less than the "compressed" size.
   */
  if ((cbots.compressed_size == 0) && (isize < cbots.byte_count))
  {
    /* Revert to STORE.  Write out the input buffer, and use its size. */
    *cmpr_method = STORE;
    zfwrite( f_ibuf, 1, isize);
    cbots.compressed_size = isize;
  }
  else
  {
    /* Flush out any remaining expanded data. */
    if (cbots.byte_count > 0)
    {
      zfwrite( f_obuf, 1, cbots.byte_count);
      cbots.compressed_size += cbots.byte_count;
    }
  }

  if (f_ibuf != NULL)
  {
    free( f_ibuf);
    f_ibuf = NULL;
  }

  if (f_obuf != NULL)
  {
    free( f_obuf);
    f_obuf = NULL;
  }

  Ppmd8_Free( &ppmd8, &g_Alloc);

  /* Set the "internal file attributes" word to text or binary,
   * according to what is_text_buf() determined.
   */
  if (file_binary_final)
    z_entry->att = (ush)BINARY;
  else
    z_entry->att = (ush)ASCII;

  return cbots.compressed_size;
}

#endif /* def PPMD_SUPPORT */

#endif /* !UTIL */