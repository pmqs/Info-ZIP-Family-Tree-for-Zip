/*
  zipfile.c - Zip 3

  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  zipfile.c by Mark Adler.
 */
#define __ZIPFILE_C

#include "zip.h"
#include "revision.h"

/* for realloc 2/6/2005 EG */
#include <stdlib.h>

#ifdef VMS
#  include "vms/vms.h"
#  include "vms/vmsmunch.h"
#  include "vms/vmsdefs.h"
#endif

#ifdef __RSXNT__
#  include <windows.h>
#endif

/*
 * XXX start of zipfile.h
 */
#ifdef THEOS
 /* Macros cause stack overflow in compiler */
 ush SH(uch* p) { return ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8)); }
 ulg LG(uch* p) { return ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16)); }
#else /* !THEOS */
 /* Macros for converting integers in little-endian to machine format */
# define SH(a) ((ush)(((ush)(uch)(a)[0]) | (((ush)(uch)(a)[1]) << 8)))
# define LG(a) ((ulg)SH(a) | ((ulg)SH((a)+2) << 16))
# ifdef ZIP64_SUPPORT           /* zip64 support 08/31/2003 R.Nausedat */
#  define LLG(a) ((zoff_t)LG(a) | ((zoff_t)LG((a)+4) << 32))
# endif
#endif /* ?THEOS */

/* Macros for writing machine integers to little-endian format */
#define PUTSH(a,f) {putc((char)((a) & 0xff),(f)); putc((char)((a) >> 8),(f));}
#define PUTLG(a,f) {PUTSH((a) & 0xffff,(f)) PUTSH((a) >> 16,(f))}

#ifdef ZIP64_SUPPORT           /* zip64 support 08/31/2003 R.Nausedat */
# define PUTLLG(a,f) {PUTLG((a) & 0xffffffff,(f)) PUTLG((a) >> 32,(f))}
#endif


/* -- Structure of a ZIP file -- */

/* Signatures for zip file information headers */
#define LOCSIG     0x04034b50L
#define CENSIG     0x02014b50L
#define ENDSIG     0x06054b50L
#define EXTLOCSIG  0x08074b50L

/* Offsets of values in headers */
/* local header */
#define LOCVER  0               /* version needed to extract */
#define LOCFLG  2               /* encrypt, deflate flags */
#define LOCHOW  4               /* compression method */
#define LOCTIM  6               /* last modified file time, DOS format */
#define LOCDAT  8               /* last modified file date, DOS format */
#define LOCCRC  10              /* uncompressed crc-32 for file */
#define LOCSIZ  14              /* compressed size in zip file */
#define LOCLEN  18              /* uncompressed size */
#define LOCNAM  22              /* length of filename */
#define LOCEXT  24              /* length of extra field */

/* extended local header (data descriptor) following file data (if bit 3 set) */
/* if Zip64 then all are 8 byte and not below - 11/1/03 EG */
#define EXTCRC  0               /* uncompressed crc-32 for file */
#define EXTSIZ  4               /* compressed size in zip file */
#define EXTLEN  8               /* uncompressed size */

/* central directory header */
#define CENVEM  0               /* version made by */
#define CENVER  2               /* version needed to extract */
#define CENFLG  4               /* encrypt, deflate flags */
#define CENHOW  6               /* compression method */
#define CENTIM  8               /* last modified file time, DOS format */
#define CENDAT  10              /* last modified file date, DOS format */
#define CENCRC  12              /* uncompressed crc-32 for file */
#define CENSIZ  16              /* compressed size in zip file */
#define CENLEN  20              /* uncompressed size */
#define CENNAM  24              /* length of filename */
#define CENEXT  26              /* length of extra field */
#define CENCOM  28              /* file comment length */
#define CENDSK  30              /* disk number start */
#define CENATT  32              /* internal file attributes */
#define CENATX  34              /* external file attributes */
#define CENOFF  38              /* relative offset of local header */

/* end of central directory record */
#define ENDDSK  0               /* number of this disk */
#define ENDBEG  2               /* number of the starting disk */
#define ENDSUB  4               /* entries on this disk */
#define ENDTOT  6               /* total number of entries */
#define ENDSIZ  8               /* size of entire central directory */
#define ENDOFF  12              /* offset of central on starting disk */
#define ENDCOM  16              /* length of zip file comment */

/* zip64 support 08/31/2003 R.Nausedat */

/* EOCDL_SIG used to detect Zip64 archive */
#define ZIP64_EOCDL_SIG                  0x07064b50
/* EOCDL size is used in the empty archive check */
#define ZIP64_EOCDL_OFS_SIZE                20

#ifdef ZIP64_SUPPORT
# define ZIP_UWORD16_MAX                 0xFFFF                        /* border value */
# define ZIP_UWORD32_MAX                 0xFFFFFFFF                    /* border value */
# define ZIP64_EXTCRC                    0                             /* uncompressed crc-32 for file */
# define ZIP64_EXTSIZ                    4                             /* compressed size in zip file */
# define ZIP64_EXTLEN                    12                            /* uncompressed size */
# define ZIP64_EOCD_SIG                  0x06064b50
# define ZIP64_EOCD_OFS_SIZE             40
# define ZIP64_EOCD_OFS_CD_START         48
# define ZIP64_EOCDL_OFS_SIZE                20
# define ZIP64_EOCDL_OFS_EOCD_START      8
# define ZIP64_EOCDL_OFS_TOTALDISKS      16
# define ZIP_EF_HEADER_SIZE              4                             /* size of pre-header of extra fields */
# define ZIP64_MIN_VER                   45                            /* min version to set in the CD extra records */
# define ZIP64_CENTRAL_DIR_TAIL_SIZE     (56 - 8 - 4)                  /* size of zip64 central dir tail */
# define ZIP64_CENTRAL_DIR_TAIL_SIG      0x06064B50L                   /* zip64 central dir tail signature */
# define ZIP64_CENTRAL_DIR_TAIL_END_SIG  0x07064B50L                   /* zip64 end of cen dir locator signature */
# define ZIP64_LARGE_FILE_HEAD_SIZE      32                            /* total size of zip64 extra field */
# define ZIP64_EF_TAG                    0x0001                        /* ID for zip64 extra field */
# define ZIP64_EFIELD_OFS_OSIZE          ZIP_EF_HEADER_SIZE            /* zip64 extra field: offset to original file size */
# define ZIP64_EFIELD_OFS_CSIZE          (ZIP64_EFIELD_OFS_OSIZE + 8)  /* zip64 extra field: offset to compressed file size */
# define ZIP64_EFIELD_OFS_OFS            (ZIP64_EFIELD_OFS_CSIZE + 8)  /* zip64 extra field: offset to offset in archive */
# define ZIP64_EFIELD_OFS_DISK           (ZIP64_EFIELD_OFS_OFS + 8)    /* zip64 extra field: offset to start disk # */
/* -------------------------------------------------------------------------------------------------------------------------- */
 local char *get_extra_field OF((ush, char *, unsigned));           /* zip64 */
 local void adjust_zip_local_entry OF((struct zlist far *));
 local void adjust_zip_central_entry OF((struct zlist far *));
 local int add_central_zip64_extra_field OF((struct zlist far *));
 local int add_local_zip64_extra_field OF((struct zlist far *));
#endif /* ZIP64_SUPPORT */

/* moved out of ZIP64_SUPPORT - 2/6/2005 EG */
local void write_ushort_to_mem OF((ush, char *));                      /* little endian conversions */
local void write_ulong_to_mem OF((ulg, char *));
#ifdef ZIP64_SUPPORT
local void write_int64_to_mem OF((uzoff_t, char *));
#endif /* def ZIP64_SUPPORT */

/* added these self allocators - 2/6/2005 EG */
local void append_ushort_to_mem OF((ush, char **, ulg *, ulg *));
local void append_ulong_to_mem OF((ulg, char **, ulg *, ulg *));
#ifdef ZIP64_SUPPORT
local void append_int64_to_mem OF((uzoff_t, char **, ulg *, ulg *));
#endif /* def ZIP64_SUPPORT */
local void append_string_to_mem OF((char *, int, char**, ulg *, ulg *));


/* Local functions */

local int zqcmp OF((ZCONST zvoid *, ZCONST zvoid *));
local int scanzipf_reg OF((FILE *f));
#ifndef UTIL
   local int rqcmp OF((ZCONST zvoid *, ZCONST zvoid *));
   local int zbcmp OF((ZCONST zvoid *, ZCONST zvoid far *));
   local void zipoddities OF((struct zlist far *));
   local int scanzipf_fix OF((FILE *f));
#  ifdef USE_EF_UT_TIME
     local int ef_scan_ut_time OF((char *ef_buf, extent ef_len, int ef_is_cent,
                                   iztimes *z_utim));
#  endif /* USE_EF_UT_TIME */
   local void cutpath OF((char *p, int delim));
#endif /* !UTIL */

/*
 * XXX end of zipfile.h
 */

/* Local data */

#ifdef HANDLE_AMIGA_SFX
   ulg amiga_sfx_offset;        /* place where size field needs updating */
#endif


local int zqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to zip entries */
/* Used by qsort() to compare entries in the zfile list.
 * Compares the internal names z->iname */
{
  return namecmp((*(struct zlist far **)a)->iname,
                 (*(struct zlist far **)b)->iname);
}

#ifndef UTIL

local int rqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to zip entries */
/* Used by qsort() to compare entries in the zfile list.
 * Compare the internal names z->iname, but in reverse order. */
{
  return namecmp((*(struct zlist far **)b)->iname,
                 (*(struct zlist far **)a)->iname);
}


local int zbcmp(n, z)
ZCONST zvoid *n;        /* string to search for */
ZCONST zvoid far *z;    /* pointer to a pointer to a zip entry */
/* Used by search() to compare a target to an entry in the zfile list. */
{
  return namecmp((char *)n, ((struct zlist far *)z)->zname);
}


struct zlist far *zsearch(n)
ZCONST char *n;         /* name to find */
/* Return a pointer to the entry in zfile with the name n, or NULL if
   not found. */
{
  zvoid far **p;        /* result of search() */

  if (zcount &&
      (p = search(n, (ZCONST zvoid far **)zsort, zcount, zbcmp)) != NULL)
    return *(struct zlist far **)p;
  else
    return NULL;
}

#endif /* !UTIL */

#ifndef VMS     /* See [.VMS]VMS.C for VMS-specific ziptyp(). */
#  ifndef PATHCUT
#    define PATHCUT '/'
#  endif

char *ziptyp(s)
char *s;                /* file name to force to zip */
/* If the file name *s has a dot (other than the first char), or if
   the -A option is used (adjust self-extracting file) then return
   the name, otherwise append .zip to the name.  Allocate the space for
   the name in either case.  Return a pointer to the new name, or NULL
   if malloc() fails. */
{
  char *q;              /* temporary pointer */
  char *t;              /* pointer to malloc'ed string */
#ifdef THEOS
  char *r;              /* temporary pointer */
  char *disk;
#endif

  if ((t = malloc(strlen(s) + 5)) == NULL)
    return NULL;
  strcpy(t, s);
#ifdef __human68k__
  _toslash(t);
#endif
#ifdef MSDOS
  for (q = t; *q; INCSTR(q))
    if (*q == '\\')
      *q = '/';
#endif /* MSDOS */
#ifdef __RSXNT__   /* RSXNT/EMX C rtl uses OEM charset */
  AnsiToOem(t, t);
#else
#ifdef WIN32_OEM
  AnsiToOem(t, t);
#endif
#endif
  if (adjust) return t;
#ifndef RISCOS
# ifndef QDOS
#  ifdef AMIGA
  if ((q = MBSRCHR(t, '/')) == NULL)
    q = MBSRCHR(t, ':');
  if (MBSRCHR((q ? q + 1 : t), '.') == NULL)
#  else /* !AMIGA */
#    ifdef THEOS
  /* the argument expansion add a dot to the end of file names when
   * there is no extension and at least one of a argument has wild cards.
   * So check for at least one character in the extension if there is a dot
   * in file name */
  if ((q = MBSRCHR((q = MBSRCHR(t, PATHCUT)) == NULL ? t : q + 1, '.')) == NULL
    || q[1] == '\0') {
#    else /* !THEOS */
#      ifdef TANDEM
  if (MBSRCHR((q = MBSRCHR(t, '.')) == NULL ? t : q + 1, ' ') == NULL)
#      else /* !TANDEM */
  if (MBSRCHR((q = MBSRCHR(t, PATHCUT)) == NULL ? t : q + 1, '.') == NULL)
#      endif /* ?TANDEM */
#    endif /* ?THEOS */
#  endif /* ?AMIGA */
#  ifdef CMS_MVS
    if (strncmp(t,"dd:",3) != 0 && strncmp(t,"DD:",3) != 0)
#  endif /* CMS_MVS */
#  ifdef THEOS
    /* insert .zip extension before disk name */
    if ((r = MBSRCHR(t, ':')) != NULL) {
        /* save disk name */
        if ((disk = strdup(r)) == NULL)
            return NULL;
        strcpy(r[-1] == '.' ? r - 1 : r, ".zip");
        strcat(t, disk);
        free(disk);
    } else {
        if (q != NULL && *q == '.')
          strcpy(q, ".zip");
        else
          strcat(t, ".zip");
    }
  }
#  else /* !THEOS */
#    ifdef TANDEM     /*  Tandem can't cope with extensions */
    strcat(t, " ZIP");
#    else /* !TANDEM */
    strcat(t, ".zip");
#    endif /* ?TANDEM */
#  endif /* ?THEOS */
# else /* QDOS */
  q = LastDir(t);
  if(MBSRCHR(q, '_') == NULL && MBSRCHR(q, '.') == NULL)
  {
      strcat(t, "_zip");
  }
# endif /* QDOS */
#endif /* !RISCOS */
  return t;
}
#endif  /* ndef VMS */

/* ---------------------------------------------------- */

/* moved out of ZIP64_SUPPORT - 2/6/2005 EG */

/* 08/31/2003 R.Nausedat */

local void write_ushort_to_mem( OFT( ush) usValue,
                                OFT( char *)pPtr)
#ifdef NO_PROTO
  ush usValue;
  char *pPtr;
#endif /* def NO_PROTO */
{
  *pPtr++ = usValue & 0xff;
  *pPtr = ((usValue >> 8) & 0xff);
}

local void write_ulong_to_mem(uValue,pPtr)
ulg uValue;
char *pPtr;
{
  write_ushort_to_mem((ush)(uValue & 0xffff),pPtr);
  write_ushort_to_mem((ush)((uValue >> 16) & 0xffff),pPtr + 2);
}

#ifdef ZIP64_SUPPORT

local void write_int64_to_mem(l64Value,pPtr)
uzoff_t l64Value;
char *pPtr;
{
  write_ulong_to_mem((ulg)(l64Value & 0xffffffff),pPtr);
  write_ulong_to_mem((ulg)((l64Value >> 32) & 0xffffffff),pPtr + 4);
}

#endif /* def ZIP64_SUPPORT */


/* same as above but allocate memory as needed and keep track of current end
   using offset - 2/6/05 EG */

#if 0 /* ubyte version not used */
local void append_ubyte_to_mem( OFT( unsigned char) ubValue,
                                OFT( char **) pPtr,
                                OFT( ulg *) offset,
                                OFT( ulg *) blocksize)
#ifdef NO_PROTO
  unsigned char ubValue;  /* byte to append */
  char **pPtr;            /* start of block */
  ulg *offset;           /* next byte to write */
  ulg *blocksize;        /* current size of block */
#endif /* def NO_PROTO */
{
  if (*pPtr == NULL) {
    /* malloc a 1K block */
    (*blocksize) = 1024;
    *pPtr = (char *) malloc(*blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ubyte_to_mem");
    }
  }
  else if ((*offset) + 1 > (*blocksize) - 1) {
    /* realloc a bigger block in 1 K increments */
    (*blocksize) += 1024;
    *pPtr = realloc(*pPtr, *blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ubyte_to_mem");
    }
  }
  *(*pPtr + *offset) = ubValue;
  (*offset)++;
}
#endif

local void append_ushort_to_mem( OFT( ush) usValue,
                                 OFT( char **) pPtr,
                                 OFT( ulg *) offset,
                                 OFT( ulg *) blocksize)
#ifdef NO_PROTO
  ush usValue;
  char **pPtr;
  ulg *offset;
  ulg *blocksize;
#endif /* def NO_PROTO */
{
  if (*pPtr == NULL) {
    /* malloc a 1K block */
    (*blocksize) = 1024;
    *pPtr = (char *) malloc(*blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ushort_to_mem");
    }
  }
  else if ((*offset) + 2 > (*blocksize) - 1) {
    /* realloc a bigger block in 1 K increments */
    (*blocksize) += 1024;
    *pPtr = realloc(*pPtr, *blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ushort_to_mem");
    }
  }
  write_ushort_to_mem(usValue, (*pPtr) + (*offset));
  (*offset) += 2;
}

local void append_ulong_to_mem(uValue, pPtr, offset, blocksize)
  ulg uValue;
  char **pPtr;
  ulg *offset;
  ulg *blocksize;
{
  if (*pPtr == NULL) {
    /* malloc a 1K block */
    (*blocksize) = 1024;
    *pPtr = (char *) malloc(*blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ulong_to_mem");
    }
  }
  else if ((*offset) + 4 > (*blocksize) - 1) {
    /* realloc a bigger block in 1 K increments */
    (*blocksize) += 1024;
    *pPtr = realloc(*pPtr, *blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_ulong_to_mem");
    }
  }
  write_ulong_to_mem(uValue, (*pPtr) + (*offset));
  (*offset) += 4;
}

#ifdef ZIP64_SUPPORT

local void append_int64_to_mem(l64Value, pPtr, offset, blocksize)
  uzoff_t l64Value;
  char **pPtr;
  ulg *offset;
  ulg *blocksize;
{
  if (*pPtr == NULL) {
    /* malloc a 1K block */
    (*blocksize) = 1024;
    *pPtr = (char *) malloc(*blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_int64_to_mem");
    }
  }
  else if ((*offset) + 8 > (*blocksize) - 1) {
    /* realloc a bigger block in 1 K increments */
    (*blocksize) += 1024;
    *pPtr = realloc(*pPtr, *blocksize);
    if (*pPtr == NULL) {
      ziperr(ZE_MEM, "append_int64_to_mem");
    }
  }
  write_int64_to_mem(l64Value, (*pPtr) + (*offset));
  (*offset) += 8;
}

#endif /* def ZIP64_SUPPORT */

/* Append a string to the memory block. */
local void append_string_to_mem(strValue, strLength, pPtr, offset, blocksize)
  char *strValue;
  int  strLength;
  char **pPtr;
  ulg *offset;
  ulg *blocksize;
{
  if (strValue != NULL) {
    int bsize = 1024;
    int ssize = strLength;
    int i;

    if (ssize > bsize) {
      bsize = ssize;
    }
    if (*pPtr == NULL) {
      /* malloc a 1K block */
      (*blocksize) = bsize;
      *pPtr = (char *) malloc(*blocksize);
      if (*pPtr == NULL) {
        ziperr(ZE_MEM, "append_string_to_mem");
      }
    }
    else if ((*offset) + ssize > (*blocksize) - 1) {
      /* realloc a bigger block in 1 K increments */
      (*blocksize) += bsize;
      *pPtr = realloc(*pPtr, *blocksize);
      if (*pPtr == NULL) {
        ziperr(ZE_MEM, "append_string_to_mem");
      }
    }
    for (i = 0; i < ssize; i++) {
      *(*pPtr + *offset + i) = *(strValue + i);
    }
    (*offset) += ssize;
  }
}

/* ---------------------------------------------------- */


# ifdef ZIP64_SUPPORT           /* zip64 support 08/31/2003 R.Nausedat */

/* Searches pExtra for extra field with specified tag.
 * If it finds one it returns a pointer to it, else NULL.
 * Renamed and made generic.  10/3/03
 */
local char *get_extra_field( OFT( ush) tag,
                             OFT( char *) pExtra,
                             OFT( unsigned) iExtraLen)
#ifdef NO_PROTO
  ush tag;       /* tag to look for */
  char *pExtra;  /* pointer to extra field in memory */
  unsigned iExtraLen; /* length of extra field */
#endif /* def NO_PROTO */
{
  char  *pTemp;
  ush   usBlockTag;
  ush   usBlockSize;

  if( pExtra == NULL )
    return NULL;

  for (pTemp = pExtra; pTemp < pExtra  + iExtraLen - ZIP_EF_HEADER_SIZE;)
  {
    usBlockTag = SH(pTemp);       /* get tag */
    usBlockSize = SH(pTemp + 2);  /* get field data size */
    if (usBlockTag == tag)
      return pTemp;
    pTemp += (usBlockSize + ZIP_EF_HEADER_SIZE);
  }
  return NULL;
}

/* searches the cextra member of zlist for a zip64 extra field. if it finds one it  */
/* updates the len, siz and off members of zlist with the corresponding values of   */
/* the zip64 extra field, that is if either the len, siz or off member of zlist is  */
/* set to its max value we have to use the corresponding value from the zip64 extra */
/* field. as of now the dsk member of zlist is not much of interest since we should */
/* not modify multi volume archives at all.                                         */
local void adjust_zip_central_entry(pZipListEntry)
  struct zlist far *pZipListEntry;
{
  char  *pTemp;
  /* assume not using zip64 fields */
  zip64_entry = 0;

  /* check if we have a "large file" Zip64 extra field ... */
  pTemp = get_extra_field( ZIP64_EF_TAG, pZipListEntry->cextra, pZipListEntry->cext );
  if( pTemp == NULL )
    return;

  /* using zip64 field */
  zip64_entry = 1;
  pTemp += ZIP_EF_HEADER_SIZE;

  /* ... if so, update corresponding entries in struct zlist */
  if (pZipListEntry->len == ZIP_UWORD32_MAX)
  {
    pZipListEntry->len = LLG(pTemp);
    pTemp += 8;
  }

  if (pZipListEntry->siz == ZIP_UWORD32_MAX)
  {
    pZipListEntry->siz = LLG(pTemp);
    pTemp += 8;
  }

  if (pZipListEntry->off == ZIP_UWORD32_MAX)
    pZipListEntry->off = LLG(pTemp);
}

local void adjust_zip_local_entry(pZipListEntry)
  struct zlist far *pZipListEntry;
{
  char  *pTemp;
  /* assume not using zip64 fields */
  zip64_entry = 0;

  /* check if we have a "large file" Zip64 extra field ... */
  pTemp = get_extra_field( ZIP64_EF_TAG, pZipListEntry->extra, pZipListEntry->ext );
  if( pTemp == NULL )
    return;

  /* using zip64 field */
  zip64_entry = 1;
  pTemp += ZIP_EF_HEADER_SIZE;

  /* ... if so, update corresponding entries in struct zlist */
  if (pZipListEntry->len == ZIP_UWORD32_MAX)
  {
    pZipListEntry->len = LLG(pTemp);
    pTemp += 8;
  }

  if (pZipListEntry->siz == ZIP_UWORD32_MAX)
  {
    pZipListEntry->siz = LLG(pTemp);
    pTemp += 8;
  }
}

/* adds a zip64 extra field to the data the cextra member of zlist points to. If
 * there is already a zip64 extra field present delete it first.
 */
local int add_central_zip64_extra_field(pZipListEntry)
  struct zlist far *pZipListEntry;
{
  char   *pExtraFieldPtr;
  char   *pTemp;
  ush    usTemp;
  ush    efsize = 0;
  ush    esize;
  ush    oldefsize;
  extent len;

  /* get length of ef based on which fields exceed limits */
  /* AppNote says:
   *      The order of the fields in the ZIP64 extended
   *      information record is fixed, but the fields will
   *      only appear if the corresponding Local or Central
   *      directory record field is set to 0xFFFF or 0xFFFFFFFF.
   */
  efsize = ZIP_EF_HEADER_SIZE;             /* type + size */
  if (pZipListEntry->len > ZIP_UWORD32_MAX || force_zip64) {
    /* compressed size */
    efsize += 8;
  }
  if (pZipListEntry->siz > ZIP_UWORD32_MAX) {
    /* uncompressed size */
    efsize += 8;
  }
  if (pZipListEntry->off > ZIP_UWORD32_MAX) {
    /* offset */
    efsize += 8;
  }
  if (pZipListEntry->dsk > ZIP_UWORD16_MAX) {
    /* disk number */
    efsize += 4;
  }

  /* malloc zip64 extra field? */
  if( pZipListEntry->cextra == NULL )
  {
    if (efsize == ZIP_EF_HEADER_SIZE) {
      return ZE_OK;
    }
    if ((pExtraFieldPtr = pZipListEntry->cextra = (char *) malloc(efsize)) == NULL) {
      return ZE_MEM;
    }
    pZipListEntry->cext = efsize;
  }
  else
  {
    /* check if we have a "large file" extra field ... */
    pExtraFieldPtr = get_extra_field(ZIP64_EF_TAG, pZipListEntry->cextra, pZipListEntry->cext);
    if( pExtraFieldPtr == NULL )
    {
      /* ... we don't, so re-malloc enough memory for the old extra data plus
       * the size of the zip64 extra field
       */
      if ((pExtraFieldPtr = (char *) malloc(efsize + pZipListEntry->cext)) == NULL) {
        return ZE_MEM;
      }
      /* move the old extra field */
      memmove(pExtraFieldPtr, pZipListEntry->cextra, pZipListEntry->cext);
      free(pZipListEntry->cextra);
      pZipListEntry->cextra = pExtraFieldPtr;
      pExtraFieldPtr += pZipListEntry->cext;
      pZipListEntry->cext += efsize;
    }
    else
    {
      /* ... we have. sort out the existing zip64 extra field and remove it from
       * pZipListEntry->cextra, re-malloc enough memory for the old extra data
       * left plus the size of the zip64 extra field
       */
      usTemp = SH(pExtraFieldPtr + 2);
      /* if pZipListEntry->cextra == pExtraFieldPtr and pZipListEntry->cext == usTemp + efsize
       * we should have only one extra field, and this is a zip64 extra field. as some
       * zip tools seem to require fixed zip64 extra fields we have to check if
       * usTemp + ZIP_EF_HEADER_SIZE is equal to ZIP64_LARGE_FILE_HEAD_SIZE. if it
       * isn't, we free the old extra field and allocate memory for a new one
       */
      if( pZipListEntry->cext == (extent)(usTemp + ZIP_EF_HEADER_SIZE) )
      {
        /* just Zip64 extra field in extra field */
        if( pZipListEntry->cext != efsize )
        {
          /* wrong size */
          if ((pExtraFieldPtr = (char *) malloc(efsize)) == NULL) {
            return ZE_MEM;
          }
          free(pZipListEntry->cextra);
          pZipListEntry->cextra = pExtraFieldPtr;
          pZipListEntry->cext = efsize;
        }
      }
      else
      {
        /* get the old Zip64 extra field out and add new */
        oldefsize = usTemp + ZIP_EF_HEADER_SIZE;
        if ((pTemp = (char *) malloc(pZipListEntry->cext - oldefsize + efsize)) == NULL) {
          return ZE_MEM;
        }
        len = (extent)(pExtraFieldPtr - pZipListEntry->cextra);
        memcpy(pTemp, pZipListEntry->cextra, len);
        len = pZipListEntry->cext - oldefsize - len;
        memcpy(pTemp + len, pExtraFieldPtr + oldefsize, len);
        pZipListEntry->cext -= oldefsize;
        pExtraFieldPtr = pTemp + pZipListEntry->cext;
        pZipListEntry->cext += efsize;
        free(pZipListEntry->cextra);
        pZipListEntry->cextra = pTemp;
      }
    }
  }

  /* set zip64 extra field members */
  write_ushort_to_mem(ZIP64_EF_TAG, pExtraFieldPtr);
  write_ushort_to_mem((ush) (efsize - ZIP_EF_HEADER_SIZE), pExtraFieldPtr + 2);
  esize = ZIP_EF_HEADER_SIZE;
  if (pZipListEntry->len > ZIP_UWORD32_MAX || force_zip64) {
    write_int64_to_mem(pZipListEntry->len, pExtraFieldPtr + esize);
    esize += 8;
  }
  if (pZipListEntry->siz > ZIP_UWORD32_MAX) {
    write_int64_to_mem(pZipListEntry->siz, pExtraFieldPtr + esize);
    esize += 8;
  }
  if (pZipListEntry->off > ZIP_UWORD32_MAX) {
    write_int64_to_mem(pZipListEntry->off, pExtraFieldPtr + esize);
    esize += 8;
  }
  if (pZipListEntry->dsk > ZIP_UWORD16_MAX) {
    write_ulong_to_mem(pZipListEntry->dsk, pExtraFieldPtr + esize);
  }

  /* un' wech */
  return ZE_OK;
}

/* Add Zip64 extra field to local header
 * 10/5/03 EG
 */
local int add_local_zip64_extra_field(pZEntry)
  struct zlist far *pZEntry;
{
  char  *pZ64Extra;
  char  *pOldZ64Extra;
  char  *pOldTemp;
  char  *pTemp;
  ush   newEFSize;
  ush   usTemp;
  ush   blocksize;
  ush   Z64LocalLen = ZIP_EF_HEADER_SIZE +  /* tag + EF Data Len */
                      8 +                   /* original uncompressed length of file */
                      8;                    /* compressed size of file */

  /* malloc zip64 extra field? */
  /* after the below pZ64Extra should point to start of Zip64 extra field */
  if (pZEntry->ext == 0 || pZEntry->extra == NULL)
  {
    /* get new extra field */
    pZ64Extra = pZEntry->extra = (char *) malloc(Z64LocalLen);
    if (pZEntry->extra == NULL) {
      ziperr( ZE_MEM, "Zip64 local extra field" );
    }
    pZEntry->ext = Z64LocalLen;
  }
  else
  {
    /* check if we have a Zip64 extra field ... */
    pOldZ64Extra = get_extra_field( ZIP64_EF_TAG, pZEntry->extra, pZEntry->ext );
    if (pOldZ64Extra == NULL)
    {
      /* ... we don't, so re-malloc enough memory for the old extra data plus */
      /* the size of the zip64 extra field */
      pZ64Extra = (char *) malloc( Z64LocalLen + pZEntry->ext );
      if (pZ64Extra == NULL)
        ziperr( ZE_MEM, "Zip64 Extra Field" );
      /* move old extra field and update pointer and length */
      memmove( pZ64Extra, pZEntry->extra, pZEntry->ext);
      free( pZEntry->extra );
      pZEntry->extra = pZ64Extra;
      pZ64Extra += pZEntry->ext;
      pZEntry->ext += Z64LocalLen;
    }
    else
    {
      /* ... we have. Sort out the existing zip64 extra field and remove it
       * from pZEntry->extra, re-malloc enough memory for the old extra data
       * left plus the size of the zip64 extra field */
      blocksize = SH( pOldZ64Extra + 2 );
      /* If the right length then go with it, else get rid of it and add a new extra field
       * to existing block. */
      if (blocksize == Z64LocalLen - ZIP_EF_HEADER_SIZE)
      {
        /* looks good */
        pZ64Extra = pOldZ64Extra;
      }
      else
      {
        newEFSize = pZEntry->ext - (blocksize + ZIP_EF_HEADER_SIZE) + Z64LocalLen;
        pZ64Extra = (char *) malloc( newEFSize );
        if( pZ64Extra == NULL )
          ziperr(ZE_MEM, "Zip64 Extra Field");
        /* move all before Zip64 EF */
        usTemp = (extent) (pOldZ64Extra - pZEntry->extra);
        pTemp = pZ64Extra;
        memcpy( pTemp, pZEntry->extra, usTemp );
        /* move all after old Zip64 EF */
        pTemp = pZ64Extra + usTemp;
        pOldTemp = pOldZ64Extra + ZIP_EF_HEADER_SIZE + blocksize;
        usTemp = pZEntry->ext - usTemp - blocksize;
        memcpy( pTemp, pOldTemp, usTemp);
        /* replace extra fields */
        pZEntry->ext = newEFSize;
        free(pZEntry->extra);
        pZEntry->extra = pZ64Extra;
        pZ64Extra = pTemp + usTemp;
      }
    }
  }
  /* set/update zip64 extra field members */
  write_ushort_to_mem(ZIP64_EF_TAG, pZ64Extra);
  write_ushort_to_mem((ush) (Z64LocalLen - ZIP_EF_HEADER_SIZE), pZ64Extra + 2);
  write_int64_to_mem(pZEntry->len, pZ64Extra + 2 + 2);
  write_int64_to_mem(pZEntry->siz, pZ64Extra + 2 + 2 + 8);

  return ZE_OK;
}

# endif /* ZIP64_SUPPORT */


zoff_t ffile_size OF((FILE *));


/* 2004-12-06 SMS.
 * ffile_size() returns reliable file size or EOF.
 * May be used to detect large files in a small-file program.
 */
zoff_t ffile_size( file)
FILE *file;
{
  int sts;
  size_t siz;
  zoff_t ofs;
  char waste[ 4];

  /* Seek to actual EOF. */
  sts = zfseeko( file, 0, SEEK_END);
  if (sts != 0)
  {
    /* fseeko() failed.  (Unlikely.) */
    ofs = EOF;
  }
  else
  {
    /* Get apparent offset at EOF. */
    ofs = zftello( file);
    if (ofs < 0)
    {
      /* Offset negative (overflow).  File too big. */
      ofs = EOF;
    }
    else
    {
      /* Seek to apparent EOF offset.
         Won't be at actual EOF if offset was truncated.
      */
      sts = zfseeko( file, ofs, SEEK_SET);
      if (sts != 0)
      {
        /* fseeko() failed.  (Unlikely.) */
        ofs = EOF;
      }
      else
      {
        /* Read a byte at apparent EOF.  Should set EOF flag. */
        siz = fread( waste, 1, 1, file);
        if (feof( file) == 0)
        {
          /* Not at EOF, but should be.  File too big. */
          ofs = EOF;
        }
      }
    }
  }
  sts = zfseeko( file, 0, SEEK_SET);
  return ofs;
}


#ifndef UTIL

local void zipoddities(z)
struct zlist far *z;
{
    if ((z->vem >> 8) >= NUM_HOSTS)
    {
        sprintf(errbuf, "made by version %d.%d on system type %d: ",
                (ush)(z->vem & 0xff) / (ush)10, (ush)(z->vem & 0xff) % (ush)10,
                z->vem >> 8);
        zipwarn(errbuf, z->zname);
    }
    if (z->ver != 10 && z->ver != 11 && z->ver != 20)
    {
        sprintf(errbuf, "needs unzip %d.%d on system type %d: ",
                (ush)(z->ver & 0xff) / (ush)10,
                (ush)(z->ver & 0xff) % (ush)10, z->ver >> 8);
        zipwarn(errbuf, z->zname);
    }
    if (z->flg != z->lflg)
    {
        sprintf(errbuf, "local flags = 0x%04x, central = 0x%04x: ",
                z->lflg, z->flg);
        zipwarn(errbuf, z->zname);
    }
    else if (z->flg & ~0xf)
    {
        sprintf(errbuf, "undefined bits used in flags = 0x%04x: ", z->flg);
        zipwarn(errbuf, z->zname);
    }
    if (z->how > DEFLATE)
    {
        sprintf(errbuf, "unknown compression method %u: ", z->how);
        zipwarn(errbuf, z->zname);
    }
    if (z->dsk)
    {
        sprintf(errbuf, "starts on disk %lu: ", z->dsk);
        zipwarn(errbuf, z->zname);
    }
    if (z->att!=ASCII && z->att!=BINARY && z->att!=__EBCDIC)
    {
        sprintf(errbuf, "unknown internal attributes = 0x%04x: ", z->att);
        zipwarn(errbuf, z->zname);
    }
# if 0
/* This test is ridiculous, it produces an error message for almost every */
/* platform of origin other than MS-DOS, Unix, VMS, and Acorn!  Perhaps   */
/* we could test "if (z->dosflag && z->atx & ~0xffL)", but what for?      */
    if (((n = z->vem >> 8) != 3) && n != 2 && n != 13 && z->atx & ~0xffL)
    {
        sprintf(errbuf, "unknown external attributes = 0x%08lx: ", z->atx);
        zipwarn(errbuf, z->zname);
    }
# endif

    /* This test is just annoying, as Zip itself does not write the same
       extra fields to both the local and central headers.  It's much more
       complicated than this test implies.  3/17/05 EG */
#if 0
    if (z->ext || z->cext)
    {
        if (z->ext && z->cext && z->extra != z->cextra)
        {
          sprintf(errbuf,
                  "local extra (%ld bytes) != central extra (%ld bytes): ",
                  (ulg)z->ext, (ulg)z->cext);
          if (noisy) fprintf(stderr, "\tzip info: %s%s\n", errbuf, z->zname);
        }
# if (!defined(RISCOS) && !defined(CMS_MVS))
        /* in noisy mode, extra field sizes are always reported */
        else if (noisy)
# else /* RISCOS || CMS_MVS */
/* avoid warnings for zipfiles created on the same type of OS system! */
/* or, was this warning really intended (eg. OS/2)? */
        /* Only give info if extra bytes were added by another system */
        else if (noisy && ((z->vem >> 8) != (OS_CODE >> 8)))
# endif /* ?(RISCOS || CMS_MVS) */
        {
            fprintf(stderr, "zip info: %s has %ld bytes of %sextra data\n",
                    z->zname, z->ext ? (ulg)z->ext : (ulg)z->cext,
                    z->ext ? (z->cext ? "" : "local ") : "central ");
        }
    }
#endif /* 0 */
}


/*
 * scanzipf_fix is called with zip -F or zip -FF
 * read the file from front to back and pick up the pieces
 * NOTE: there are still checks missing to see if the header
 *       that was found is *VALID*
 *
 * Still much work to do so can handle more cases.  1/18/04 EG
 */
local int scanzipf_fix(f)
  FILE *f;                      /* zip file */
/*
   The name of the zip file is pointed to by the global "zipfile".  The globals
   zipbeg, cenbeg, zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
    ulg a = 0L;                 /* attributes returned by filetime() */
    char b[CENHEAD];            /* buffer for central headers */
    ush flg;                    /* general purpose bit flag */
    int m;                      /* mismatch flag */
    extent n;                   /* length of name */
    uzoff_t p;                  /* current file offset */
    uzoff_t s;                  /* size of data, start of central */
    struct zlist far * far *x;  /* pointer last entry's link */
    struct zlist far *z;        /* current zip entry structure */

#ifndef ZIP64_SUPPORT

/* 2004-12-06 SMS.
 * Check for too-big file before doing any serious work.
 */
    if (ffile_size( f) == EOF)
      return ZE_ZIP64;

#endif /* ndef ZIP64_SUPPORT */


    /* Get any file attribute valid for this OS, to set in the central
     * directory when fixing the archive:
     */
# ifndef UTIL
    filetime(zipfile, &a, (zoff_t*)&s, NULL);
# endif
    x = &zfiles;                        /* first link */
    p = 0;                              /* starting file offset */
# ifdef HANDLE_AMIGA_SFX
    amiga_sfx_offset = 0L;
# endif

    /* Find start of zip structures */
    for (;;) {
      /* look for signature */
      while ((m = getc(f)) != EOF && m != 0x50)    /* 0x50 == 'P' */
      {
# ifdef HANDLE_AMIGA_SFX
        if (p == 0 && m == 0)
          amiga_sfx_offset = 1L;
        else if (amiga_sfx_offset) {
          if ((p == 1 && m != 0) || (p == 2 && m != 3)
                                 || (p == 3 && (uch) m != 0xF3))
            amiga_sfx_offset = 0L;
        }
# endif /* HANDLE_AMIGA_SFX */
        p++;
      }
      /* found a P */
      b[0] = (char) m;
      /* local - 11/2/03 EG */
      if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG)
        break;
      /* why search for ENDSIG if doing only local - 11/2/03 EG
      if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == ENDSIG)
        break;
      */
      /* back up */
      if (zfseeko(f, -3L, SEEK_CUR))
        return ferror(f) ? ZE_READ : ZE_EOF;
      /* move 1 byte forward */
      p++;
    }
    zipbeg = p;
# ifdef HANDLE_AMIGA_SFX
    if (amiga_sfx_offset && zipbeg >= 12 && (zipbeg & 3) == 0
        && fseek(f, -12L, SEEK_CUR) == 0 && fread(b, 12, 1, f) == 1
        && LG(b + 4) == 0xF1030000 /* 1009 in Motorola byte order */)
      amiga_sfx_offset = zipbeg - 4;
    else
      amiga_sfx_offset = 0L;
# endif /* HANDLE_AMIGA_SFX */

    /* Read local headers */
    while (LG(b) == LOCSIG)
    {
      if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL ||
          zcount + 1 < zcount)
        return ZE_MEM;
      if (fread(b, LOCHEAD, 1, f) != 1) {
          farfree((zvoid far *)z);
          break;
      }

      z->ver = SH(LOCVER + b);
      z->vem = (ush)(dosify ? 20 : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER);
      z->dosflag = dosify;
      flg = z->flg = z->lflg = SH(LOCFLG + b);
      z->how = SH(LOCHOW + b);
      z->tim = LG(LOCTIM + b);          /* time and date into one long */
      z->crc = LG(LOCCRC + b);
      z->siz = LG(LOCSIZ + b);
      z->len = LG(LOCLEN + b);
      n = z->nam = SH(LOCNAM + b);
      z->cext = z->ext = SH(LOCEXT + b);

      z->com = 0;
      z->dsk = 0;
      z->att = 0;
      z->atx = dosify ? a & 0xff : a;     /* Attributes from filetime() */
      z->mark = 0;
      z->trash = 0;

      /* attention: this one breaks the VC optimizer (Release Build) */
      /* may be fixed - 11/1/03 EG */
      s = fix > 1 ? 0L : z->siz; /* discard compressed size with -FF */

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name and extra field and skip data */
      if (n == 0)
      {
        sprintf(errbuf, "%lu", (ulg)zcount + 1);
        zipwarn("zero-length name for entry #", errbuf);
# ifndef DEBUG
        return ZE_FORM;
# endif
      }
      if ((z->iname = malloc(n+1)) ==  NULL ||
          (z->ext && (z->extra = malloc(z->ext)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, n, 1, f) != 1 ||
          (z->ext && fread(z->extra, z->ext, 1, f) != 1))
        return ferror(f) ? ZE_READ : ZE_EOF;

#  ifdef ZIP64_SUPPORT
      /* adjust/update siz,len and off (to come: dsk) entries */
      /* PKZIP does not care of the version set in a CDH: if  */
      /* there is a zip64 extra field assigned to a CDH PKZIP */
      /* uses it, we should do so, too.                       */
      adjust_zip_local_entry(z);
      /* z->siz may be updated */
      s = fix > 1 ? 0L : z->siz; /* discard compressed size with -FF */
#  endif

      if (s && zfseeko(f, (zoff_t)s, SEEK_CUR))
        return ferror(f) ? ZE_READ : ZE_EOF;
      /* If there is an extended local header, s is either 0 or
       * the correct compressed size.
       */
      z->iname[n] = '\0';               /* terminate name */
      z->zname = in2ex(z->iname);       /* convert to external name */
      if (z->zname == NULL)
        return ZE_MEM;
      z->name = z->zname;
      z->cextra = z->extra;
      if (noisy) fprintf(mesg, "zip: reading %s\n", z->zname);

      /* Save offset, update for next header */
      z->off = p;
      p += 4 + LOCHEAD + n + z->ext + s;
      zcount++;

      /* Skip extended local header if there is one */
      if ((flg & 8) != 0) {
        /* Skip the compressed data if compressed size is unknown.
         * For safety, we should use the central directory.
         */
        if (s == 0) {
          for (;;) {
            while ((m = getc(f)) != EOF && m != 0x50) ;  /* 0x50 == 'P' */
            b[0] = (char) m;
            if (fread(b+1, 15, 1, f) != 1 || LG(b) == EXTLOCSIG)
              break;
            if (zfseeko(f, -15L, SEEK_CUR))
              return ferror(f) ? ZE_READ : ZE_EOF;
          }
# ifdef ZIP64_SUPPORT
          if (zip64_entry) {        /* from extra field */
            /* all are 8 bytes */
            s = LG(4 + ZIP64_EXTSIZ + b);
          } else {
            s = LG(4 + EXTSIZ + b);
          }
# else
          s = LG(4 + EXTSIZ + b);
# endif
          p += s;
          if ((uzoff_t) zftello(f) != p+16L) {
            zipwarn("bad extended local header for ", z->zname);
            return ZE_FORM;
          }
        } else {
          /* compressed size non-zero, assume that it is valid: */
          Assert(p == zftello(f), "bad compressed size with extended header");

          if (zfseeko(f, p, SEEK_SET) || fread(b, 16, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(b) != EXTLOCSIG) {
            zipwarn("extended local header not found for ", z->zname);
            return ZE_FORM;
          }
        }
        /* overwrite the unknown values of the local header: */

        /* already in host format */
# ifdef ZIP64_SUPPORT
        z->crc = LG(4 + ZIP64_EXTCRC + b);
        z->siz = s;
        z->len = LG(4 + ZIP64_EXTLEN + b);
# else
        z->crc = LG(4 + EXTCRC + b);
        z->siz = s;
        z->len = LG(4 + EXTLEN + b);
# endif

        p += 16L;
      }
      else if (fix > 1) {
        /* Don't trust the compressed size */
        for (;;) {
          while ((m = getc(f)) != EOF && m != 0x50) p++; /* 0x50 == 'P' */
          b[0] = (char) m;
          if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == CENSIG)
            break;
          if (zfseeko(f, -3L, SEEK_CUR))
            return ferror(f) ? ZE_READ : ZE_EOF;
          p++;
        }
        s = p - (z->off + 4 + LOCHEAD + n + z->ext);
        if (s != z->siz) {
          fprintf(mesg, " compressed size %s, actual size %s for %s\n",
                  zip_fzofft(z->siz, NULL, "u"), zip_fzofft(s, NULL, "u"),
                  z->zname);
          z->siz = s;
        }
        /* next LOCSIG already read at this point, don't read it again: */
        continue;
      }

      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          break;
    }

    s = p;                              /* save start of central */

    if (LG(b) != CENSIG && noisy) {
      fprintf(mesg, "zip warning: %s %s truncated.\n", zipfile,
              fix > 1 ? "has been" : "would be");

      if (fix == 1) {
        fprintf(mesg,
   "Retry with option -qF to truncate, with -FF to attempt full recovery\n");
        ZIPERR(ZE_FORM, NULL);
      }
    }

    cenbeg = s;

    if (zipbeg && noisy)
      fprintf(mesg, "%s: adjusting offsets for a preamble of %s bytes\n",
              zipfile, zip_fzofft(zipbeg, NULL, "u"));
    return ZE_OK;
}

#endif /* !UTIL */

/*
 * scanzipf_reg starts searching for the End Signature at the end of the file
 * The End Signature points to the Central Directory Signature which points
 * to the Local Directory Signature
 * XXX probably some more consistency checks are needed
 */
local int scanzipf_reg(f)
  FILE *f;                      /* zip file */
/*
   The name of the zip file is pointed to by the global "zipfile".  The globals
   zipbeg, cenbeg, zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
    char b[CENHEAD];            /* buffer for central headers */
    extent n;                   /* length of name */
    struct zlist far * far *x;  /* pointer last entry's link */
    struct zlist far *z;        /* current zip entry structure */
    char *t;                    /* temporary pointer */
    char far *u;                /* temporary variable */
    int found;
    char *buf;                  /* temp buffer for reading zipfile */
# ifdef ZIP64_SUPPORT
    ulg u4;                     /* unsigned 4 byte variable */
    char bf[8];
    uzoff_t u8;                 /* unsigned 8 byte variable */
    uzoff_t censiz;             /* size of central directory */
    uzoff_t z64eocd;            /* Zip64 End Of Central Directory record byte offset */
# else
    ush flg;                    /* general purpose bit flag */
    int m;                      /* mismatch flag */
# endif
    zoff_t deltaoff = 0;


#ifndef ZIP64_SUPPORT

/* 2004-12-06 SMS.
 * Check for too-big file before doing any serious work.
 */
    if (ffile_size( f) == EOF)
      return ZE_ZIP64;

#endif /* ndef ZIP64_SUPPORT */


    buf = malloc(4096 + 4);
    if (buf == NULL)
      return ZE_MEM;

#ifdef HANDLE_AMIGA_SFX
    amiga_sfx_offset = (fread(buf, 1, 4, f) == 4 && LG(buf) == 0xF3030000);
    /* == 1 if this file is an Amiga executable (presumably UnZipSFX) */
#endif
#ifdef SPLIT_SUPPORT
    /* detect spanning signature */
    zfseeko(f, 0, SEEK_SET);
    read_split_archive = (fread(buf, 1, 4, f) == 4 && LG(buf) == 0x08074b50L);
#endif
    found = 0;
    t = &buf[4096];
    t[1] = '\0';
    t[2] = '\0';
    t[3] = '\0';
    /* back up as much as 4k from end */
    /* zip64 support 08/31/2003 R.Nausedat */
    if (zfseeko(f, -4096L, SEEK_END) == 0) {
      zipbeg = (uzoff_t) (zftello(f) + 4096L);
      /* back up 4k blocks and look for End Of CD signature */
      while (!found && zipbeg >= 4096) {
        zipbeg -= 4096L;
        buf[4096] = t[1];
        buf[4097] = t[2];
        buf[4098] = t[3];
/*
 * XXX error check ??
 */
        fread(buf, 1, 4096, f);
        zfseeko(f, -8192L, SEEK_CUR);
        t = &buf[4095];
/*
 * XXX far pointer arithmetic in DOS
 */
        while (t >= buf) {
          /* Check for ENDSIG ("PK\5\6" in ASCII) */
          if (LG(t) == ENDSIG) {
            found = 1;
/*
 * XXX error check ??
 * XXX far pointer arithmetic in DOS
 */
            zipbeg += (uzoff_t) (t - buf);
            zfseeko(f, (zoff_t) zipbeg + 4L, SEEK_SET);
            break;
          }
          --t;
        }
      }
    }
    else
      /* file less than 4k bytes */
      zipbeg = 4096L;
/*
 * XXX warn: garbage at the end of the file ignored
 */
    if (!found && zipbeg > 0) {
      size_t s;

      zfseeko(f, 0L, SEEK_SET);
      clearerr(f);
      s = fread(buf, 1, (size_t) zipbeg, f);
      /* add 0 bytes at end */
      buf[s] = t[1];
      buf[s + 1] = t[2];
      buf[s + 2] = t[3];
      t = &buf[s - 1];
/*
 * XXX far pointer comparison in DOS
 */
      while (t >= buf) {
        /* Check for ENDSIG ("PK\5\6" in ASCII) */
        if (LG(t) == ENDSIG) {
          found = 1;
/*
 * XXX far pointer arithmetic in DOS
 */
          zipbeg = (ulg) (t - buf);
          zfseeko(f, (zoff_t) zipbeg + 4L, SEEK_SET);
          break;
        }
        --t;
      }
    }
    free(buf);
    if (!found) {
      zipwarn("missing end signature--probably not a zip file (did you", "");
      zipwarn("remember to use binary mode when you transferred it?)", "");
      return ZE_FORM;
    }

/*
 * Check for a Zip64 EOCD Locator signature - 12/10/04 EG
 */
#ifndef ZIP64_SUPPORT
    /* If Zip64 not enabled check if archive being read is Zip64 */
    /* back up 24 bytes (size of Z64 EOCDL and ENDSIG) */
    if (zfseeko(f, -24, SEEK_CUR) != 0) {
        perror("fseek");
        return ZE_FORM; /* XXX */
    }
    /* read Z64 EOCDL if there */
    if (fread(b, 20, 1, f) != 1) {
      return ZE_READ;
    }
    /* first 4 bytes are the signature if there */
    if (LG(b) == ZIP64_EOCDL_SIG) {
      zipwarn("found Zip64 signature - this may be a Zip64 archive", "");
      zipwarn("PKZIP 4.5 or later needed - set ZIP64_SUPPORT in Zip 3", "");
      return ZE_ZIP64;
    }

    /* now should be back at the EOCD signature */
    if (fread(b, 4, 1, f) != 1) {
      zipwarn("unable to read after relative seek", "");
      return ZE_READ;
    }
    if (LG(b) != ENDSIG) {
      zipwarn("unable to relative seek in archive", "");
      return ZE_FORM;
    }
#if 0
    if (fseek(f, -4, SEEK_CUR) != 0) {
        perror("fseek");
        return ZE_FORM; /* XXX */
    }
#endif
#endif

    /* Read end header */
    if (fread(b, ENDHEAD, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;
    if (SH(ENDDSK + b) || SH(ENDBEG + b) ||
        SH(ENDSUB + b) != SH(ENDTOT + b))
      zipwarn("multiple disk information ignored", "");
    zcomlen = SH(ENDCOM + b);
    if (zcomlen)
    {
      if ((zcomment = malloc(zcomlen)) == NULL)
        return ZE_MEM;
      if (fread(zcomment, zcomlen, 1, f) != 1)
      {
        free((zvoid *)zcomment);
        zcomment = NULL;
        return ferror(f) ? ZE_READ : ZE_EOF;
      }
#ifdef EBCDIC
      if (zcomment)
         memtoebc(zcomment, zcomment, zcomlen);
#endif /* EBCDIC */
    }
#ifdef ZIP64_SUPPORT
    /* account for Zip64 EOCD Record and Zip64 EOCD Locator */

    /* Z64 EOCDL should be just before EOCD (unless this is an empty archive) */
    cenbeg = zipbeg - ZIP64_EOCDL_OFS_SIZE;
    /* check for empty archive */
    /* changed cenbeg to uzoff_t so instead of cenbeg >= 0 use new check - 5/23/05 EG */
    if (zipbeg >= ZIP64_EOCDL_OFS_SIZE) {
      /* look for signature */
      if (zfseeko(f, cenbeg, SEEK_SET)) {
        zipwarn("end of file seeking Z64EOCDL", "");
        return ZE_FORM;
      }
      if (fread(bf, 4, 1, f) != 1) {
        ziperr(ZE_FORM, "read error");
      }
      u4 = LG(bf);
      if (u4 == ZIP64_EOCDL_SIG) {
        /* found Zip64 EOCD Locator */
        /* check for disk information */
        zfseeko(f, cenbeg + ZIP64_EOCDL_OFS_TOTALDISKS, SEEK_SET);
        if (fread(bf, 4, 1, f) != 1) {
          ziperr(ZE_FORM, "read error");
        }
        u4 = LG(bf);
        if (u4 != 1) {
          ziperr(ZE_FORM, "multiple disk archives not yet supported");
        }

        /* look for Zip64 EOCD Record */
        zfseeko(f, cenbeg + ZIP64_EOCDL_OFS_EOCD_START, SEEK_SET);
        if (fread(bf, 8, 1, f) != 1) {
         ziperr(ZE_FORM, "read error");
        }
        z64eocd = LLG(bf);
        if (zfseeko(f, z64eocd, SEEK_SET)) {
          ziperr(ZE_FORM, "error searching for Z64 EOCD Record");
        }
        if (fread(bf, 4, 1, f) != 1) {
         ziperr(ZE_FORM, "read error");
        }
        u4 = LG(bf);
        if (u4 != ZIP64_EOCD_SIG) {
          ziperr(ZE_FORM, "Z64 EOCD not found but Z64 EOCD Locator exists");
        }
        /* get size of CD */
        zfseeko(f, z64eocd + ZIP64_EOCD_OFS_SIZE, SEEK_SET);
        if (fread(bf, 8, 1, f) != 1) {
         ziperr(ZE_FORM, "read error");
        }
        censiz = LLG(bf);
        /* get start of CD */
        zfseeko(f, z64eocd + ZIP64_EOCD_OFS_CD_START, SEEK_SET);
        if (fread(bf, 8, 1, f) == (size_t) -1) {
         ziperr(ZE_FORM, "read error");
        }
        cenbeg = LLG(bf);
        u8 = z64eocd - cenbeg;
        deltaoff = adjust ? u8 - censiz : 0L;
      } else {
        /* assume no Locator and no Zip64 EOCD Record */
        censiz = LG(ENDSIZ + b);
        cenbeg = LG(b + ENDOFF);
        u8 = zipbeg - censiz;
        deltaoff = adjust ? u8 - censiz : 0L;
      }
    }
#else
/*
 * XXX assumes central header immediately precedes end header
 */
    /* start of central directory */
    cenbeg = zipbeg - LG(ENDSIZ + b);
/*
printf("start of central directory cenbeg %ld\n", cenbeg);
*/

    /* offset to first entry of archive */
    deltaoff = adjust ? cenbeg - LG(b + ENDOFF) : 0L;
#endif

    if (zipbeg < ZIP64_EOCDL_OFS_SIZE) {
      /* zip file seems empty */
      return ZE_OK;
    }

    if (zfseeko(f, cenbeg, SEEK_SET) != 0) {
        perror("fseek");
        return ZE_FORM; /* XXX */
    }

    x = &zfiles;                        /* first link */

    if (fread(b, 4, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;

    while (LG(b) == CENSIG) {
      /* Read central header. The portion of the central header that should
         be in common with local header is read raw, for later comparison.
         (this requires that the offset of ext in the zlist structure
         be greater than or equal to LOCHEAD) */
      if (fread(b, CENHEAD, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL)
        return ZE_MEM;
      z->vem = SH(CENVEM + b);
      for (u = (char far *)(&(z->ver)), n = 0; n < (CENNAM-CENVER); n++)
        u[n] = b[CENVER + n];
      z->nam = SH(CENNAM + b);          /* used before comparing cen vs. loc */
      z->cext = SH(CENEXT + b);         /* may be different from z->ext */
      z->com = SH(CENCOM + b);
      z->dsk = SH(CENDSK + b);
      z->att = SH(CENATT + b);
      z->atx = LG(CENATX + b);
      z->off = LG(CENOFF + b) + deltaoff;
      z->dosflag = (z->vem & 0xff00) == 0;

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name, extra field and comment field */
      if (z->nam == 0)
      {
        sprintf(errbuf, "%lu", (ulg)zcount + 1);
        zipwarn("zero-length name for entry #", errbuf);
#ifndef DEBUG
        farfree((zvoid far *)z);
        return ZE_FORM;
#endif
      }
      if ((z->iname = malloc(z->nam+1)) ==  NULL ||
          (z->cext && (z->cextra = malloc(z->cext)) == NULL) ||
          (z->com && (z->comment = malloc(z->com)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, z->nam, 1, f) != 1 ||
          (z->cext && fread(z->cextra, z->cext, 1, f) != 1) ||
          (z->com && fread(z->comment, z->com, 1, f) != 1))
        return ferror(f) ? ZE_READ : ZE_EOF;
      z->iname[z->nam] = '\0';                  /* terminate name */
#ifdef EBCDIC
      if (z->com)
         memtoebc(z->comment, z->comment, z->com);
#endif /* EBCDIC */

#ifdef ZIP64_SUPPORT
      /* zip64 support 08/31/2003 R.Nausedat                          */
      /* here, we have to read the len, siz etc values from the CD    */
      /* entry as we might have to adjust them regarding their        */
      /* correspronding zip64 extra fields.                           */
      /* also, we cannot compare the values from the CD entries with  */
      /* the values from the LH as they might be different.           */
      z->len = LG(CENLEN + b);
      z->siz = LG(CENSIZ + b);
      z->crc = LG(CENCRC + b);
      z->tim = LG(CENTIM + b);   /* time and date into one long */
      z->how = SH(CENHOW + b);
      z->flg = SH(CENFLG + b);
      z->ver = SH(CENVER + b);
      /* adjust/update siz,len and off (to come: dsk) entries */
      /* PKZIP does not care of the version set in a CDH: if  */
      /* there is a zip64 extra field assigned to a CDH PKZIP */
      /* uses it, we should do so, too.                       */
      adjust_zip_central_entry(z);
#endif
      /* Update zipbeg offset, prepare for next header */
      if (z->off < zipbeg)
         zipbeg = z->off;
      zcount++;
      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          return ferror(f) ? ZE_READ : ZE_EOF;
    }

    /* Point to start of header list and read local headers */
    z = zfiles;
    while (z != NULL) {
      /* Read next signature */
      if (zfseeko(f, z->off, SEEK_SET) != 0 || fread(b, 4, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if (LG(b) == LOCSIG) {
        if (fread(b, LOCHEAD, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;

        z->lflg = SH(LOCFLG + b);
        n = SH(LOCNAM + b);
        z->ext = SH(LOCEXT + b);

        /* Compare name and extra fields */
        if (n != z->nam)
        {
#ifdef EBCDIC
          strtoebc(z->iname, z->iname);
#endif
          zipwarn("name lengths in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        if ((t = malloc(z->nam)) == NULL)
          return ZE_MEM;
        if (fread(t, z->nam, 1, f) != 1)
        {
          free((zvoid *)t);
          return ferror(f) ? ZE_READ : ZE_EOF;
        }
        if (memcmp(t, z->iname, z->nam))
        {
          free((zvoid *)t);
#ifdef EBCDIC
          strtoebc(z->iname, z->iname);
#endif
          zipwarn("names in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        free((zvoid *)t);
        if (z->ext)
        {
          if ((z->extra = malloc(z->ext)) == NULL)
            return ZE_MEM;
          if (fread(z->extra, z->ext, 1, f) != 1)
          {
            free((zvoid *)(z->extra));
            return ferror(f) ? ZE_READ : ZE_EOF;
          }
          if (z->ext == z->cext && memcmp(z->extra, z->cextra, z->ext) == 0)
          {
            free((zvoid *)(z->extra));
            z->extra = z->cextra;
          }
        }

#ifdef ZIP64_SUPPORT       /* zip64 support 09/02/2003 R.Nausedat */
/*
for now the below is left out if ZIP64_SUPPORT is defined as the fields
len, siz and off in struct zlist are type of int64 if ZIP64_SUPPORT
is defined. In either way, the values read from the central directory
should be valid. comments are welcome
*/
#else /* !ZIP64_SUPPORT */
        /* Check extended local header if there is one */
        /* bit 3 */
        if ((z->lflg & 8) != 0)
        {
          char buf2[16];
          ulg s;                        /* size of compressed data */

          s = LG(LOCSIZ + b);
          if (s == 0)
            s = LG((CENSIZ-CENVER) + (char far *)(&(z->ver)));
          if (zfseeko(f, (z->off + (4+LOCHEAD) + z->nam + z->ext + s), SEEK_SET)
              || (fread(buf2, 16, 1, f) != 1))
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(buf2) != EXTLOCSIG)
          {
# ifdef EBCDIC
            strtoebc(z->iname, z->iname);
# endif
            zipwarn("extended local header not found for ", z->iname);
            return ZE_FORM;
          }
          /* overwrite the unknown values of the local header: */
          for (n = 0; n < 12; n++)
            b[LOCCRC+n] = buf2[4+n];
        }

        /* Compare local header with that part of central header (except
           for the reserved bits in the general purpose flags and except
           for the already checked entry name length */
        /* If I have read this right we are stepping through the z struct
           here as a byte array.  Need to fix this.  5/25/2005 EG */
        u = (char far *)(&(z->ver));
        flg = SH((CENFLG-CENVER) + u);          /* Save central flags word */
        u[CENFLG-CENVER+1] &= 0x1f;             /* Mask reserved flag bits */
        b[LOCFLG+1] &= 0x1f;
        for (m = 0, n = 0; n < LOCNAM; n++) {
          if (b[n] != u[n])
          {
            if (!m)
            {
              zipwarn("local and central headers differ for ", z->zname);
              m = 1;
            }
            if (noisy)
            {
              sprintf(errbuf, " offset %u--local = %02x, central = %02x",
                      (unsigned)n, (uch)b[n], (uch)u[n]);
              zipwarn(errbuf, "");
            }
          }
        }
        if (m && !adjust)
          return ZE_FORM;

        /* Complete the setup of the zlist entry by translating the remaining
         * central header fields in memory, starting with the fields with
         * highest offset. This order of the conversion commands takes into
         * account potential buffer overlaps caused by structure padding.
         */
        z->len = LG((CENLEN-CENVER) + u);
        z->siz = LG((CENSIZ-CENVER) + u);
        z->crc = LG((CENCRC-CENVER) + u);
        z->tim = LG((CENTIM-CENVER) + u);   /* time and date into one long */
        z->how = SH((CENHOW-CENVER) + u);
        z->flg = flg;                       /* may be different from z->lflg */
        z->ver = SH((CENVER-CENVER) + u);
#endif /* ZIP64_SUPPORT */

        /* Clear actions */
        z->mark = 0;
        z->trash = 0;
#ifdef UTIL
/* We only need z->iname in the utils */
        z->name = z->iname;
#ifdef EBCDIC
/* z->zname is used for printing and must be coded in native charset */
        if ((z->zname = malloc(z->nam+1)) ==  NULL)
          return ZE_MEM;
        strtoebc(z->zname, z->iname);
#else
        z->zname = z->iname;
#endif
#else /* !UTIL */
        z->zname = in2ex(z->iname);       /* convert to external name */
        if (z->zname == NULL)
          return ZE_MEM;
        z->name = z->zname;
#endif /* ?UTIL */
      }
      else {
#ifdef EBCDIC
        strtoebc(z->iname, z->iname);
#endif
        zipwarn("local header not found for ", z->iname);
        return ZE_FORM;
      }
#ifndef UTIL
      if (verbose)
        zipoddities(z);
#endif
      z = z->nxt;
    }

    if (zipbeg && noisy)
      fprintf(mesg, "%s: %s a preamble of %s bytes\n",
              zipfile, adjust ? "adjusting offsets for" : "found",
              zip_fzofft(zipbeg, NULL, "u"));
#ifdef HANDLE_AMIGA_SFX
    if (zipbeg < 12 || (zipbeg & 3) != 0 /* must be longword aligned */)
      amiga_sfx_offset = 0;
    else if (amiga_sfx_offset) {
      char buf2[16];
      if (!fseek(f, zipbeg - 12, SEEK_SET) && fread(buf2, 12, 1, f) == 1) {
        if (LG(buf2 + 4) == 0xF1030000 /* 1009 in Motorola byte order */)
          /* could also check if LG(buf2) == 0xF2030000... no for now */
          amiga_sfx_offset = zipbeg - 4;
        else
          amiga_sfx_offset = 0L;
      }
    }
#endif /* HANDLE_AMIGA_SFX */
    return ZE_OK;
}


/*
 * readzipfile initializes the global variables that hold the zipfile
 * directory info and opens the zipfile. For the actual zipfile scan,
 * the subroutine scanzipf_reg() or scanzipf_fix() is called,
 * depending on the mode of operation (regular processing, or zipfix mode).
 */
int readzipfile()
/*
   The name of the zip file is pointed to by the global "zipfile".
   The globals zipbeg, zfiles, zcount, and zcomlen are initialized.
   Return an error code in the ZE_ class.
*/
{
  FILE *f;              /* zip file */
  int retval;           /* return code */
  int readable;         /* 1 if zipfile exists and is readable */

  /* Initialize zip file info */
  zipbeg = 0;
  zfiles = NULL;                        /* Points to first header */
  zcount = 0;                           /* number of files */
  zcomlen = 0;                          /* zip file comment length */
  retval = ZE_OK;
  f = NULL;                             /* shut up some compilers */

  /* If zip file exists, read headers and check structure */
#ifdef VMS
  if (zipfile == NULL || !(*zipfile) || !strcmp(zipfile, "-"))
    return ZE_OK;
  {
    int rtype;

    if ((VMSmunch(zipfile, GET_RTYPE, (char *)&rtype) == RMS$_NORMAL) &&
        (rtype == FAT$C_VARIABLE)) {
      fprintf(stderr,
     "\n     Error:  zipfile is in variable-length record format.  Please\n\
     run \"bilf b %s\" to convert the zipfile to fixed-length\n\
     record format.\n\n", zipfile);
      return ZE_FORM;
    }
  }
  readable = ((f = zfopen(zipfile, FOPR)) != NULL);
#else /* !VMS */
  readable = (zipfile != NULL && *zipfile && strcmp(zipfile, "-") &&
              (f = zfopen(zipfile, FOPR)) != NULL);
#endif /* ?VMS */
#ifdef MVS
  /* Very nasty special case for MVS.  Just because the zipfile has been
   * opened for reading does not mean that we can actually read the data.
   * Typical JCL to create a zipfile is
   *
   * //ZIPFILE  DD  DISP=(NEW,CATLG),DSN=prefix.ZIP,
   * //             SPACE=(CYL,(10,10))
   *
   * That creates a VTOC entry with an end of file marker (DS1LSTAR) of zero.
   * Alas the VTOC end of file marker is only used when the file is opened in
   * append mode.  When a file is opened in read mode, the "other" end of file
   * marker is used, a zero length data block signals end of file when reading.
   * With a brand new file which has not been written to yet, it is undefined
   * what you read off the disk.  In fact you read whatever data was in the same
   * disk tracks before the zipfile was allocated.  You would be amazed at the
   * number of application programmers who still do not understand this.  Makes
   * for interesting and semi-random errors, GIGO.
   *
   * Newer versions of SMS will automatically write a zero length block when a
   * file is allocated.  However not all sites run SMS or they run older levels
   * so we cannot rely on that.  The only safe thing to do is close the file,
   * open in append mode (we already know that the file exists), close it again,
   * reopen in read mode and try to read a data block.  Opening and closing in
   * append mode will write a zero length block where DS1LSTAR points, making
   * sure that the VTOC and internal end of file markers are in sync.  Then it
   * is safe to read data.  If we cannot read one byte of data after all that,
   * it is a brand new zipfile and must not be read.
   */
  if (readable)
  {
    char c;
    fclose(f);
    /* append mode */
    if ((f = zfopen(zipfile, "ab")) == NULL) {
      ZIPERR(ZE_OPEN, zipfile);
    }
    fclose(f);
    /* read mode again */
    if ((f = zfopen(zipfile, FOPR)) == NULL) {
      ZIPERR(ZE_OPEN, zipfile);
    }
    if (fread(&c, 1, 1, f) != 1) {
      /* no actual data */
      readable = 0;
      fclose(f);
    }
    else{
      fseek(f, 0, SEEK_SET);  /* at least one byte in zipfile, back to the start */
    }
  }
#endif /* MVS */
  if (readable)
  {
#ifndef UTIL
    retval = (fix && !adjust) ? scanzipf_fix(f) : scanzipf_reg(f);
#else
    retval = scanzipf_reg(f);
#endif

    /* Done with zip file for now */
    fclose(f);

    /* If one or more files, sort by name */
    if (zcount)
    {
      struct zlist far * far *x;    /* pointer into zsort array */
      struct zlist far *z;          /* pointer into zfiles linked list */
      extent zl_size = zcount * sizeof(struct zlist far *);

      if (zl_size / sizeof(struct zlist far *) != zcount ||
          (x = zsort = (struct zlist far **)malloc(zl_size)) == NULL)
        return ZE_MEM;
      for (z = zfiles; z != NULL; z = z->nxt)
        *x++ = z;
      qsort((char *)zsort, zcount, sizeof(struct zlist far *), zqcmp);
    }
  }
  return retval;
}


int putlocal(z, f)
  struct zlist far *z;    /* zip entry to write local header for */
  FILE *f;                /* file to write to */
/* Write a local header described by *z to file *f.  Return an error code
   in the ZE_ class. */
{
  /* If any of compressed size (siz), uncompressed size (len), offset(off), or
     disk number (dsk) is larger than can fit in the below standard fields then a
     Zip64 flag value is stored and a Zip64 extra field is created.
     Only siz and len are in the local header while all can be in the central
     directory header.

     For the local header if the extra field is created must store both
     uncompressed and compressed sizes.  10/6/03 EG

     This assumes that for large entries the compressed size won't need a
     Zip64 extra field if the uncompressed size did not.  12/30/04 EG

     If streaming or use_descriptors is set then always create a Zip64 extra field
     flagging the data descriptor as being in Zip64 format.  12/30/04 EG
   */
  char *block = NULL;   /* mem block to write to */
  ulg offset = 0;      /* offset into block */
  ulg blocksize = 0;   /* size of block */

#ifdef ZIP64_SUPPORT
  zip64_entry = 0;

  if (z->siz > ZIP_UWORD32_MAX || z->len > ZIP_UWORD32_MAX ||
      force_zip64)
  {
    zip64_entry = 1;        /* header of this entry has a field needing Zip64 */
    zip64_archive = 1;      /* this archive needs Zip64 (version 4.5 unzipper) */
  }
  if (zip64_entry) {
    z->ver = ZIP64_MIN_VER;
  }
#endif

  /* Instead of writing to the file as we go, to do splits we have to write it
     to memory and see if it will fit before writing the entire local header.
     If the local header doesn't fit we need to save it for the next disk.
     3/10/05 EG */

  append_ulong_to_mem(LOCSIG, &block, &offset, &blocksize);		/* local file header signature */
  append_ushort_to_mem(z->ver, &block, &offset, &blocksize);	/* version needed to extract */
  append_ushort_to_mem(z->lflg, &block, &offset, &blocksize);	/* general purpose bit flag */
  append_ushort_to_mem(z->how, &block, &offset, &blocksize);	/* compression method */
  append_ulong_to_mem(z->tim, &block, &offset, &blocksize);		/* last mod file date time */
  append_ulong_to_mem(z->crc, &block, &offset, &blocksize);		/* crc-32 */
#ifdef ZIP64_SUPPORT        /* zip64 support 09/02/2003 R.Nausedat */
                            /* changes 10/5/03 EG */
  if (zip64_entry) {
    append_ulong_to_mem(0xFFFFFFFF, &block, &offset, &blocksize);	/* compressed size */
    append_ulong_to_mem(0xFFFFFFFF, &block, &offset, &blocksize);	/* uncompressed size */
  } else {
    append_ulong_to_mem((ulg)z->siz, &block, &offset, &blocksize);/* compressed size */
    append_ulong_to_mem((ulg)z->len, &block, &offset, &blocksize);/* uncompressed size */
  }
#else
  append_ulong_to_mem(z->siz, &block, &offset, &blocksize);		/* compressed size */
  append_ulong_to_mem(z->len, &block, &offset, &blocksize);		/* uncompressed size */
#endif
  append_ushort_to_mem(z->nam, &block, &offset, &blocksize);	/* file name length */
#ifdef ZIP64_SUPPORT
  if (zip64_entry) {
    /* update extra field */
    add_local_zip64_extra_field( z );
  }
#endif
  append_ushort_to_mem(z->ext, &block, &offset, &blocksize);	/* extra field length */
  append_string_to_mem(z->iname, z->nam, &block, &offset, &blocksize); /* file name */
  if (z->ext) {
    append_string_to_mem(z->extra, z->ext, &block, &offset, &blocksize); /* extra field */
  }

  /* write the header */
  if (fwrite(block, 1, offset, f) != offset) {
    free(block);
    return ZE_TEMP;
  }
  free(block);
  return ZE_OK;
}

int putextended(z, f)
  struct zlist far *z;    /* zip entry to write local header for */
  FILE *f;                /* file to write to */
  /* This is the data descriptor.
   * Write an extended local header described by *z to file *f.
   * Return an error code in the ZE_ class. */
{
  /* write to mem block then write to file 3/10/2005 EG */
  char *block = NULL;   /* mem block to write to */
  ulg offset = 0;       /* offset into block */
  ulg blocksize = 0;    /* size of block */

  append_ulong_to_mem(EXTLOCSIG, &block, &offset, &blocksize);	/* extended local signature */
  append_ulong_to_mem(z->crc, &block, &offset, &blocksize);		/* crc-32 */
#ifdef ZIP64_SUPPORT
  if (zip64_entry) {
    /* use Zip64 entries */
    append_int64_to_mem(z->siz, &block, &offset, &blocksize);	/* compressed size */
    append_int64_to_mem(z->len, &block, &offset, &blocksize);	/* uncompressed size */
    /* This is rather klugy as the AppNote handles this poorly.  Typically
       we don't know at this point if we are writing a Zip64 archive or not,
       unless a file has needed Zip64.  This is particularly annoying here
       when deciding the size of the data descriptor (extended local header)
       fields as the appnote says the uncompressed and compressed sizes
       should be 8 bytes if the archive is Zip64 and 4 bytes if not.

       One interpretation is the version of the archive is determined from
       the Version Needed To Extract field in the Zip64 End Of Central Directory
       record and so either an archive should start as Zip64 and write all data
       descriptors with 8-byte fields or store everything until all the files
       are processed and then write everything to the archive as changing the
       sizes of the data descriptors is messy and just not feasible when
       streaming to standard output.  This is not easily workable and others
       use the different interpretation below.

       This was the old thought:
       We always write a standard data descriptor.  If the file has a large
       uncompressed or compressed size we set the field to the max field
       value, which we are defining as flagging the field as having a Zip64
       value that doesn't fit.  As the CRC happens before the variable size
       fields the CRC is still valid and can be used to check the file.  We
       always use deflate if streaming so signatures should not appear in
       the data and all local header signatures should be valid, allowing a
       streaming unzip to find entries by local header signatures, if max size
       values in the data descriptor sizes ignore them, and extract the file and
       check it using the CRC.  If not streaming the central directory is available
       so just use those values which are correct.

       After discussions with other groups this is the current thinking:

       Apparent industry interpretation for data descriptors:
       Data descriptor size is determined for each entry.  If the local header
       version needed to extract is 45 or higher then the entry can use Zip64
       data descriptors but more checking is needed.  If Zip64 extra field is
       present then assume data descriptor is Zip64 and local version needed
       to extract should be 45 or higher.  If standard data descriptor then
       local size fields are set to 0 and correct sizes are in standard data descriptor.
       If Zip64 data descriptor then local sizes are set to -1, Zip64 extra field
       sizes are set to 0, and the correct sizes are in the Zip64 data descriptor.

       So do this:
       If an entry is standard and the archive is updatable then seek back and
       update the local header.  No change.

       If an entry is zip64 and the archive is updatable assume the Zip64 extra
       field was created and update it.  No change.

       If data descriptors are needed then assume the archive is Zip64.  This is
       a change and means if ZIP64_SUPPORT is enabled that any non-updatable archive
       will be in Zip64 format and use Zip64 data deacriptors.  This should be
       compatible with other zippers that depend on the current (though not perfect)
       AppNote description.

       If anyone has some ideas on this I'd like to hear them.

       3/20/05 EG
    */
  }
#else
  append_ulong_to_mem(z->siz, &block, &offset, &blocksize);		/* compressed size */
  append_ulong_to_mem(z->len, &block, &offset, &blocksize);		/* uncompressed size */
#endif
  /* write the header */
  if (fwrite(block, 1, offset, f) != offset) {
    free(block);
    return ZE_TEMP;
  }
  free(block);
  return ZE_OK;
}

int putcentral(z, f)
  struct zlist far *z;    /* zip entry to write central header for */
  FILE *f;                /* file to write to */
/* Write a central header described by *z to file *f.  Return an error code
   in the ZE_ class. */
{
  /* If any of compressed size (siz), uncompressed size (len), offset(off), or
     disk number (dsk) is larger than can fit in the below standard fields then a
     Zip64 flag value is stored and a Zip64 extra field is created.
     Only siz and len are in the local header while all are in the central directory
     header.

     For the central directory header just store the fields required.  All previous fields
     must be stored though.  So can store none (no extra field), just uncompressed size
     (len), len then siz, len then siz then off, or len then siz then off then dsk, in
     those orders.  10/6/03 EG
   */

  /* write to mem block then write to file 3/10/2005 EG */
  char *block = NULL;   /* mem block to write to */
  ulg offset = 0;      /* offset into block */
  ulg blocksize = 0;   /* size of block */

#ifdef ZIP64_SUPPORT        /* zip64 support 09/02/2003 R.Nausedat */
  int iRes;

  if (z->siz > ZIP_UWORD32_MAX || z->len > ZIP_UWORD32_MAX ||
      z->off > ZIP_UWORD32_MAX || z->dsk > ZIP_UWORD16_MAX || force_zip64)
  {
    iRes = add_central_zip64_extra_field(z);
    if( iRes != ZE_OK )
      return iRes;
  }

  append_ulong_to_mem(CENSIG, &block, &offset, &blocksize);		/* central file header signature */
  append_ushort_to_mem(z->vem, &block, &offset, &blocksize);	/* version made by */
  append_ushort_to_mem(z->ver, &block, &offset, &blocksize);	/* version needed to extract */
  append_ushort_to_mem(z->flg, &block, &offset, &blocksize);	/* general purpose bit flag */
  append_ushort_to_mem(z->how, &block, &offset, &blocksize);	/* compression method */
  append_ulong_to_mem(z->tim, &block, &offset, &blocksize);		/* last mod file date time */
  append_ulong_to_mem(z->crc, &block, &offset, &blocksize);		/* crc-32 */
  if (z->siz > ZIP_UWORD32_MAX)
  {
    /* instead of z->siz */
    append_ulong_to_mem(ZIP_UWORD32_MAX, &block, &offset, &blocksize); /* compressed size */
  }
  else
  {
    append_ulong_to_mem((ulg)z->siz, &block, &offset, &blocksize); /* compressed size */
  }
  if (z->len > ZIP_UWORD32_MAX || force_zip64)	/* if forcing Zip64 just force first ef field */
  {
    /* instead of z->len */
    append_ulong_to_mem(ZIP_UWORD32_MAX, &block, &offset, &blocksize); /* uncompressed size */
  }
  else
  {
    append_ulong_to_mem((ulg)z->len, &block, &offset, &blocksize); /* uncompressed size */
  }
  append_ushort_to_mem(z->nam, &block, &offset, &blocksize);	/* file name length */
  append_ushort_to_mem(z->cext, &block, &offset, &blocksize);	/* extra field length */
  append_ushort_to_mem(z->com, &block, &offset, &blocksize);	/* file comment length */

  if (z->dsk > ZIP_UWORD16_MAX)
  {
    /* instead of z->dsk */
    append_ushort_to_mem((ush)ZIP_UWORD16_MAX, &block, &offset, &blocksize);	/* disk number start */
  }
  else
  {
    append_ushort_to_mem((ush)z->dsk, &block, &offset, &blocksize);	/* disk number start */
  }
  append_ushort_to_mem(z->att, &block, &offset, &blocksize);	/* internal file attributes */
  append_ulong_to_mem(z->atx, &block, &offset, &blocksize);		/* external file attributes */
  if (z->off > ZIP_UWORD32_MAX)
  {
    /* instead of z->off */
    append_ulong_to_mem(ZIP_UWORD32_MAX, &block, &offset, &blocksize); /* relative offset of local header */
  }
  else
  {
    append_ulong_to_mem((ulg)z->off, &block, &offset, &blocksize); /* relative offset of local header */
  }

#else /* !ZIP64_SUPPORT */

  append_ulong_to_mem(CENSIG, &block, &offset, &blocksize);		/* central file header signature */
  append_ushort_to_mem(z->vem, &block, &offset, &blocksize);	/* version made by */
  append_ushort_to_mem(z->ver, &block, &offset, &blocksize);	/* version needed to extract */
  append_ushort_to_mem(z->flg, &block, &offset, &blocksize);	/* general purpose bit flag */
  append_ushort_to_mem(z->how, &block, &offset, &blocksize);	/* compression method */
  append_ulong_to_mem(z->tim, &block, &offset, &blocksize);		/* last mod file date time */
  append_ulong_to_mem(z->crc, &block, &offset, &blocksize);		/* crc-32 */
  append_ulong_to_mem(z->siz, &block, &offset, &blocksize);		/* compressed size */
  append_ulong_to_mem(z->len, &block, &offset, &blocksize);		/* uncompressed size */
  append_ushort_to_mem(z->nam, &block, &offset, &blocksize);	/* file name length */
  append_ushort_to_mem(z->cext, &block, &offset, &blocksize);	/* extra field length */
  append_ushort_to_mem(z->com, &block, &offset, &blocksize);	/* file comment length */
  append_ushort_to_mem((ush)z->dsk, &block, &offset, &blocksize);	/* disk number start */
  append_ushort_to_mem(z->att, &block, &offset, &blocksize);	/* internal file attributes */
  append_ulong_to_mem(z->atx, &block, &offset, &blocksize);		/* external file attributes */
  append_ulong_to_mem(z->off, &block, &offset, &blocksize);		/* relative offset of local header */

#endif /* ZIP64_SUPPORT */

#ifdef EBCDIC
  if (z->com)
    memtoasc(z->comment, z->comment, z->com);
#endif /* EBCDIC */
  append_string_to_mem(z->iname, z->nam, &block, &offset, &blocksize);
  if (z->cext) {
    append_string_to_mem(z->cextra, z->cext, &block, &offset, &blocksize);
  }
  if (z->com) {
    append_string_to_mem(z->comment, z->com, &block, &offset, &blocksize);
  }

  /* write the header */
  if (fwrite(block, 1, offset, f) != offset) {
    free(block);
    return ZE_TEMP;
  }
  free(block);

  return ZE_OK;
}


#ifdef ZIP64_SUPPORT        /* zip64 support 09/05/2003 R.Nausedat */

/* Write the end of central directory data to file *f.  Return an error code
   in the ZE_ class. */

int putend( OFT( uzoff_t) n,
            OFT( uzoff_t) s,
            OFT( uzoff_t) c,
            OFT( ush) m,
            OFT( char *) z,
            OFT( FILE *) f)
#ifdef NO_PROTO
  uzoff_t n;                /* number of entries in central directory */
  uzoff_t s;                /* size of central directory */
  uzoff_t c;                /* offset of central directory */
  ush m;                    /* length of zip file comment (0 if none) */
  char *z;                  /* zip file comment if m != 0 */
  FILE *f;                  /* file to write to */
#endif /* def NO_PROTO */
{
  ush vem;          /* version made by */
  int iNeedZip64 = 0;

  char *block = NULL;   /* mem block to write to */
  ulg offset = 0;      /* offset into block */
  ulg blocksize = 0;   /* size of block */

  /* we have to create a zip64 archive if we have more than 64k - 1 entries,      */
  /* if the CD is > 4 GB or if the offset to the CD > 4 GB. even if the CD start  */
  /* is < 4 GB and CD start + CD size > 4GB we do not need a zip64 archive since  */
  /* the offset entry in the CD tail is still valid.                              */
  /* order of the zip/zip64 records in a zip64 archive:                           */
  /* central directory                                                            */
  /* zip64 end of central directory record                                        */
  /* zip64 end of central directory locator                                       */
  /* end of central directory record                                              */

  /* check zip64_archive instead of force_zip64 3/19/05 */

  if( n > ZIP_UWORD16_MAX || s > ZIP_UWORD32_MAX || c > ZIP_UWORD32_MAX ||
      zip64_archive )
  {
    ++iNeedZip64;
    /* write zip64 central dir tail:  */
    /*                                    */
    /* 4 bytes   zip64 end of central dir signature (0x06064b50) */
    append_ulong_to_mem((ulg)ZIP64_CENTRAL_DIR_TAIL_SIG, &block, &offset, &blocksize);
    /* 8 bytes   size of zip64 end of central directory record */
    /* a fixed size unless the end zip64 extensible data sector is used. - 3/19/05 EG */
    /* also note that AppNote 6.2 creates version 2 of this record for
       central directory encryption - 3/19/05 EG */
    append_int64_to_mem((zoff_t)ZIP64_CENTRAL_DIR_TAIL_SIZE, &block, &offset, &blocksize);

    /* 2 bytes   version made by */
    vem = OS_CODE + Z_MAJORVER * 10 + Z_MINORVER;
    append_ushort_to_mem(vem, &block, &offset, &blocksize);

    /* APPNOTE says that zip64 archives should have at least version 4.5
       in the "version needed to extract" field */
    /* 2 bytes   version needed to extract */
    append_ushort_to_mem(ZIP64_MIN_VER, &block, &offset, &blocksize);

    /* 4 bytes   number of this disk */
    append_ulong_to_mem(0, &block, &offset, &blocksize);
    /* 4 bytes   number of the disk with the start of the central directory */
    append_ulong_to_mem(0, &block, &offset, &blocksize);
    /* 8 bytes   total number of entries in the central directory on this disk */
    append_int64_to_mem(n, &block, &offset, &blocksize);
    /* 8 bytes   total number of entries in the central directory */
    append_int64_to_mem(n, &block, &offset, &blocksize);
    /* 8 bytes   size of the central directory */
    append_int64_to_mem(s, &block, &offset, &blocksize);
    /* 8 bytes   offset of start of central directory with respect to the starting disk number */
    append_int64_to_mem(c, &block, &offset, &blocksize);
    /* zip64 extensible data sector    (variable size), we don't use it... */

    /* write zip64 end of central directory locator:  */
    /*                                                    */
    /* 4 bytes   zip64 end of central dir locator  signature (0x07064b50) */
    append_ulong_to_mem(ZIP64_CENTRAL_DIR_TAIL_END_SIG, &block, &offset, &blocksize);
    /* 4 bytes   number of the disk with the start of the zip64 end of central directory */
    append_ulong_to_mem(0, &block, &offset, &blocksize);
    /* 8 bytes   relative offset of the zip64 end of central directory record, that is */
    /* offset of CD + CD size */
    append_int64_to_mem(c + s, &block, &offset, &blocksize);
    /* PUTLLG(l64Temp, f); */
    /* 4 bytes   total number of disks */
    append_ulong_to_mem(1, &block, &offset, &blocksize);
  }

  /* end of central dir signature */
  append_ulong_to_mem(ENDSIG, &block, &offset, &blocksize);
  if( !iNeedZip64 )
  {
    /* 2 bytes    number of this disk */
    append_ushort_to_mem(0, &block, &offset, &blocksize);
    /* 2 bytes    number of the disk with the start of the central directory */
    append_ushort_to_mem(0, &block, &offset, &blocksize);
    /* 2 bytes    total number of entries in the central directory on this disk */
    append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
    /* 2 bytes    total number of entries in the central directory */
    append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
    /* 4 bytes    size of the central directory */
    append_ulong_to_mem((ulg)s, &block, &offset, &blocksize);
    /* 4 bytes    offset of start of central directory with respect to the starting disk number */
    append_ulong_to_mem((ulg)c, &block, &offset, &blocksize);
    /* size of comment */
    append_ushort_to_mem(m, &block, &offset, &blocksize);
  }
  else
  {
    /* mv archives to come :)         */
    /* for now use n for all          */
    /* 2 bytes    number of this disk */
    append_ushort_to_mem(0, &block, &offset, &blocksize);
    /* 2 bytes    number of the disk with the start of the central directory */
    append_ushort_to_mem(0, &block, &offset, &blocksize);
    /* avoid overflows, give zip readers without zip64 support a chance */
    /* to extract as much files as possible */
    if( n > ZIP_UWORD16_MAX )
    {
      /* instead of n */
      append_ushort_to_mem(ZIP_UWORD16_MAX, &block, &offset, &blocksize);
      /* instead of n */
      append_ushort_to_mem(ZIP_UWORD16_MAX, &block, &offset, &blocksize);
    }
    else
    {
      /* 2 bytes    total number of entries in the central directory on this disk */
      append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
      /* 2 bytes    total number of entries in the central directory */
      append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
    }
    if( s > ZIP_UWORD32_MAX )
    {
      /* instead of s */
      append_ulong_to_mem(ZIP_UWORD32_MAX, &block, &offset, &blocksize);
    }
    else
    {
      /* 4 bytes    size of the central directory */
      append_ulong_to_mem((ulg)s, &block, &offset, &blocksize);
    }
    if( c > ZIP_UWORD32_MAX)
    {
      /* instead of c */
      append_ulong_to_mem(ZIP_UWORD32_MAX, &block, &offset, &blocksize);
    }
    else
    {
      /* 4 bytes    offset of start of central directory with respect to the starting disk number */
      append_ulong_to_mem((ulg)c, &block, &offset, &blocksize);
    }
    /* size of comment */
    append_ushort_to_mem(m, &block, &offset, &blocksize);
  }

/* Write the comment, if any */
#ifdef EBCDIC
  memtoasc(z, z, m);
#endif
  if (m) {
    append_string_to_mem(z, m, &block, &offset, &blocksize);
  }

  /* write the block */
  if (fwrite(block, 1, offset, f) != offset) {
    free(block);
    return ZE_TEMP;
  }
  free(block);

  return ZE_OK;
}

#else /* !ZIP64_SUPPORT */

  /* Write the end of central directory data to file *f.  Return an error code
     in the ZE_ class. */

int putend( OFT( uzoff_t) n,
            OFT( uzoff_t) s,
            OFT( uzoff_t) c,
            OFT( ush) m,
            OFT( char *) z,
            OFT( FILE *) f)
#ifdef NO_PROTO
  uzoff_t n;                /* number of entries in central directory */
  uzoff_t s;                /* size of central directory */
  uzoff_t c;                /* offset of central directory */
  ush m;                    /* length of zip file comment (0 if none) */
  char *z;                  /* zip file comment if m != 0 */
  FILE *f;                  /* file to write to */
#endif /* def NO_PROTO */
{
  char *block = NULL;   /* mem block to write to */
  ulg offset = 0;      /* offset into block */
  ulg blocksize = 0;   /* size of block */

  /* end of central dir signature */
  append_ulong_to_mem(ENDSIG, &block, &offset, &blocksize);
  /* 2 bytes    number of this disk */
  append_ushort_to_mem(0, &block, &offset, &blocksize);
  /* 2 bytes    number of the disk with the start of the central directory */
  append_ushort_to_mem(0, &block, &offset, &blocksize);
  /* 2 bytes    total number of entries in the central directory on this disk */
  append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
  /* 2 bytes    total number of entries in the central directory */
  append_ushort_to_mem((ush)n, &block, &offset, &blocksize);
  /* 4 bytes    size of the central directory */
  append_ulong_to_mem(s, &block, &offset, &blocksize);
  /* 4 bytes    offset of start of central directory with respect to the starting disk number */
  append_ulong_to_mem(c, &block, &offset, &blocksize);
  /* size of comment */
  append_ushort_to_mem(m, &block, &offset, &blocksize);
  /* Write the comment, if any */
#ifdef EBCDIC
  memtoasc(z, z, m);
#endif
  if (m) {
    append_string_to_mem(z, m, &block, &offset, &blocksize);
  }

  /* write the block */
  if (fwrite(block, 1, offset, f) != offset) {
    free(block);
    return ZE_TEMP;
  }
  free(block);

#ifdef HANDLE_AMIGA_SFX
  if (amiga_sfx_offset && zipbeg /* -J zeroes this */) {
    s = ftell(f);
    while (s & 3) s++, putc(0, f);   /* final marker must be longword aligned */
    PUTLG(0xF2030000 /* 1010 in Motorola byte order */, f);
    c = (s - amiga_sfx_offset - 4) / 4;  /* size of archive part in longwords */
    if (fseek(f, amiga_sfx_offset, SEEK_SET) != 0)
      return ZE_TEMP;
    c = ((c >> 24) & 0xFF) | ((c >> 8) & 0xFF00)
         | ((c & 0xFF00) << 8) | ((c & 0xFF) << 24);     /* invert byte order */
    PUTLG(c, f);
    fseek(f, 0, SEEK_END);                                    /* just in case */
  }
#endif
  return ZE_OK;
}

#endif /* ZIP64_SUPPORT */


/* Note: a zip "entry" includes a local header (which includes the file
   name), an encryption header if encrypting, the compressed data
   and possibly an extended local header. */

int zipcopy(z, x, y)
struct zlist far *z;    /* zip entry to copy */
FILE *x, *y;            /* source and destination files */
/* Copy the zip entry described by *z from file *x to file *y.  Return an
   error code in the ZE_ class.  Also update tempzn by the number of bytes
   copied. */
{
  uzoff_t n;           /* holds local header offset */

  Trace((stderr, "zipcopy %s\n", z->zname));
  n = (uzoff_t)((4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext);

  if (fix > 1) {
    if (zfseeko(x, z->off + n, SEEK_SET)) /* seek to compressed data */
      return ferror(x) ? ZE_READ : ZE_EOF;

    if (fix > 2) {
      /* Update length of entry's name, it may have been changed.  This is
         needed to support the ZipNote ability to rename archive entries. */
      z->nam = strlen(z->iname);
      n = (uzoff_t)((4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext);
    }

    /* do not trust the old compressed size */
    if (putlocal(z, y) != ZE_OK)
      return ZE_TEMP;

    z->off = tempzn;
    tempzn += n;
    n = z->siz;
  } else {
    if (zfseeko(x, z->off, SEEK_SET))     /* seek to local header */
      return ferror(x) ? ZE_READ : ZE_EOF;

    z->off = tempzn;
    n += z->siz;
  }
  /* copy the compressed data and the extended local header if there is one */
  if (z->lflg & 8) n += 16;
  tempzn += n;
  return fcopy(x, y, n);
}


#ifndef UTIL

#ifdef USE_EF_UT_TIME

local int ef_scan_ut_time(ef_buf, ef_len, ef_is_cent, z_utim)
char *ef_buf;                   /* buffer containing extra field */
extent ef_len;                  /* total length of extra field */
int ef_is_cent;                 /* flag indicating "is central extra field" */
iztimes *z_utim;                /* return storage: atime, mtime, ctime */
/* This function scans the extra field for EF_TIME or EF_IZUNIX blocks
 * containing Unix style time_t (GMT) values for the entry's access, creation
 * and modification time.
 * If a valid block is found, all time stamps are copied to the iztimes
 * structure.
 * The presence of an EF_TIME or EF_IZUNIX2 block results in ignoring
 * all data from probably present obsolete EF_IZUNIX blocks.
 * If multiple blocks of the same type are found, only the information from
 * the last block is used.
 * The return value is the EF_TIME Flags field (simulated in case of an
 * EF_IZUNIX block) or 0 in case of failure.
 */
{
  int flags = 0;
  unsigned eb_id;
  extent eb_len;
  int have_new_type_eb = FALSE;

  if (ef_len == 0 || ef_buf == NULL)
    return 0;

  Trace((stderr,"\nef_scan_ut_time: scanning extra field of length %u\n",
         (unsigned)ef_len));
  while (ef_len >= EB_HEADSIZE) {
    eb_id = SH(EB_ID + ef_buf);
    eb_len = SH(EB_LEN + ef_buf);

    if (eb_len > (ef_len - EB_HEADSIZE)) {
      /* Discovered some extra field inconsistency! */
      Trace((stderr,"ef_scan_ut_time: block length %u > rest ef_size %u\n",
             (unsigned)eb_len, (unsigned)(ef_len - EB_HEADSIZE)));
      break;
    }

    switch (eb_id) {
      case EF_TIME:
        flags &= ~0x00ff;       /* ignore previous IZUNIX or EF_TIME fields */
        have_new_type_eb = TRUE;
        if ( eb_len >= EB_UT_MINLEN && z_utim != NULL) {
           unsigned eb_idx = EB_UT_TIME1;
           Trace((stderr,"ef_scan_ut_time: Found TIME extra field\n"));
           flags |= (ef_buf[EB_HEADSIZE+EB_UT_FLAGS] & 0x00ff);
           if ((flags & EB_UT_FL_MTIME)) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->mtime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
                 Trace((stderr,"  Unix EF modtime = %ld\n", z_utim->mtime));
              } else {
                 flags &= ~EB_UT_FL_MTIME;
                 Trace((stderr,"  Unix EF truncated, no modtime\n"));
              }
           }
           if (ef_is_cent) {
              break;            /* central version of TIME field ends here */
           }
           if (flags & EB_UT_FL_ATIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->atime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
                 Trace((stderr,"  Unix EF acctime = %ld\n", z_utim->atime));
              } else {
                 flags &= ~EB_UT_FL_ATIME;
              }
           }
           if (flags & EB_UT_FL_CTIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->ctime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 /* eb_idx += 4; */  /* superfluous for now ... */
                 Trace((stderr,"  Unix EF cretime = %ld\n", z_utim->ctime));
              } else {
                 flags &= ~EB_UT_FL_CTIME;
              }
           }
        }
        break;

      case EF_IZUNIX2:
        if (!have_new_type_eb) {
           flags &= ~0x00ff;    /* ignore any previous IZUNIX field */
           have_new_type_eb = TRUE;
        }
        break;

      case EF_IZUNIX:
        if (eb_len >= EB_UX_MINLEN) {
           Trace((stderr,"ef_scan_ut_time: Found IZUNIX extra field\n"));
           if (have_new_type_eb) {
              break;            /* Ignore IZUNIX extra field block ! */
           }
           z_utim->atime = LG((EB_HEADSIZE+EB_UX_ATIME) + ef_buf);
           z_utim->mtime = LG((EB_HEADSIZE+EB_UX_MTIME) + ef_buf);
           Trace((stderr,"  Unix EF access time = %ld\n",z_utim->atime));
           Trace((stderr,"  Unix EF modif. time = %ld\n",z_utim->mtime));
           flags |= (EB_UT_FL_MTIME | EB_UT_FL_ATIME);  /* signal success */
        }
        break;

      case EF_THEOS:
/*      printf("Not implemented yet\n"); */
        break;

      default:
        break;
    }
    /* Skip this extra field block */
    ef_buf += (eb_len + EB_HEADSIZE);
    ef_len -= (eb_len + EB_HEADSIZE);
  }

  return flags;
}

int get_ef_ut_ztime(z, z_utim)
struct zlist far *z;
iztimes *z_utim;
{
  int r;

#ifdef IZ_CHECK_TZ
  if (!zp_tz_is_valid) return 0;
#endif

  /* First, scan local extra field. */
  r = ef_scan_ut_time(z->extra, z->ext, FALSE, z_utim);

  /* If this was not successful, try central extra field, but only if
     it is really different. */
  if (!r && z->cext > 0 && z->cextra != z->extra)
    r = ef_scan_ut_time(z->cextra, z->cext, TRUE, z_utim);

  return r;
}

#endif /* USE_EF_UT_TIME */


local void cutpath(p, delim)
char *p;                /* path string */
int delim;              /* path component separator char */
/* Cut the last path component off the name *p in place.
 * This should work on both internal and external names.
 */
{
  char *r;              /* pointer to last path delimiter */

#ifdef VMS                      /* change [w.x.y]z to [w.x]y.DIR */
  if ((r = MBSRCHR(p, ']')) != NULL)
  {
    *r = 0;
    if ((r = MBSRCHR(p, '.')) != NULL)
    {
      *r = ']';
      strcat(r, ".DIR;1");     /* this assumes a little padding--see PAD */
    } else {
      *p = 0;
    }
  } else {
    if ((r = MBSRCHR(p, delim)) != NULL)
      *r = 0;
    else
      *p = 0;
  }
#else /* !VMS */
  if ((r = MBSRCHR(p, delim)) != NULL)
    *r = 0;
  else
    *p = 0;
#endif /* ?VMS */
}

int trash()
/* Delete the compressed files and the directories that contained the deleted
   files, if empty.  Return an error code in the ZE_ class.  Failure of
   destroy() or deletedir() is ignored. */
{
  extent i;             /* counter on deleted names */
  extent n;             /* number of directories to delete */
  struct zlist far **s; /* table of zip entries to handle, sorted */
  struct zlist far *z;  /* current zip entry */

  /* Delete marked names and count directories */
  n = 0;
  for (z = zfiles; z != NULL; z = z->nxt)
    if (z->mark == 1 || z->trash)
    {
      z->mark = 1;
      if (z->iname[z->nam - 1] != (char)0x2f) { /* don't unlink directory */
        if (verbose)
          fprintf(mesg, "zip diagnostic: deleting file %s\n", z->name);
        if (destroy(z->name)) {
          zipwarn("error deleting ", z->name);
        }
        /* Try to delete all paths that lead up to marked names. This is
         * necessary only with the -D option.
         */
        if (!dirnames) {
          cutpath(z->name, '/');  /* XXX wrong ??? */
          /* Below apparently does not work for Russion OEM but
             '/' should be same as 0x2f for ascii and most ports so
             changed it.  Did not trace through the mappings but
             maybe 0x2F is mapped differently on OEM_RUSS - EG 2/28/2003 */
          /* CS, 5/14/2005: iname is the byte array read from and written
             to the zip archive; it MUST be ASCII (compatible)!!!
             If something goes wrong with OEM_RUSS, there is a charcode
             mapping error between external name (z->name) and iname somewhere
             in the in2ex & ex2in code. The charcode translation should be
             checked.
             This code line is changed back to the original code. */
          /* CS, 6/12/2005: What is handled here is the difference between
             ASCII charsets and non-ASCII charsets like the family of EBCDIC
             charsets.  On these systems, the slash character '/' is not coded
             as 0x2f but as 0x61 (the ASCII 'a'). The iname struct member holds
             the name as stored in the Zip file, which are ASCII or translated
             into ASCII for new entries, whereas the "name" struct member hold
             the external name, coded in the native charset of the system
             (EBCDIC on EBCDIC systems) */
          /* cutpath(z->iname, '/'); */ /* QQQ ??? */
          cutpath(z->iname, 0x2f); /* 0x2f = ascii['/'] */
          z->nam = strlen(z->iname);
          if (z->nam > 0) {
            z->iname[z->nam - 1] = (char)0x2f;
            z->iname[z->nam++] = '\0';
          }
          if (z->nam > 0) n++;
        }
      } else {
        n++;
      }
    }

  /* Construct the list of all marked directories. Some may be duplicated
   * if -D was used.
   */
  if (n)
  {
    if ((s = (struct zlist far **)malloc(n*sizeof(struct zlist far *))) ==
        NULL)
      return ZE_MEM;
    n = 0;
    for (z = zfiles; z != NULL; z = z->nxt) {
      if (z->mark && z->nam > 0 && z->iname[z->nam - 1] == (char)0x2f /* '/' */
          && (n == 0 || strcmp(z->name, s[n-1]->name) != 0)) {
        s[n++] = z;
      }
    }
    /* Sort the files in reverse order to get subdirectories first.
     * To avoid problems with strange naming conventions as in VMS,
     * we sort on the internal names, so x/y/z will always be removed
     * before x/y. On VMS, x/y/z > x/y but [x.y.z] < [x.y]
     */
    qsort((char *)s, n, sizeof(struct zlist far *), rqcmp);

    for (i = 0; i < n; i++) {
      char *p = s[i]->name;
      if (*p == '\0') continue;
      if (p[strlen(p) - 1] == '/') { /* keep VMS [x.y]z.dir;1 intact */
        p[strlen(p) - 1] = '\0';
      }
      if (i == 0 || strcmp(s[i]->name, s[i-1]->name) != 0) {
        if (verbose) {
          fprintf(mesg, "deleting directory %s (if empty)                \n",
                  s[i]->name);
        }
        deletedir(s[i]->name);
      }
    }
    free((zvoid *)s);
  }
  return ZE_OK;
}

#endif /* !UTIL */
