/*
  zip.h - Zip 3

/---------------------------------------------------------------------/

Info-ZIP Licence

This is version 2005-Feb-10 of the Info-ZIP copyright and license.
The definitive version of this document should be available at
ftp://ftp.info-zip.org/pub/infozip/license.html indefinitely.


Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

For the purposes of this copyright and license, "Info-ZIP" is defined as
the following set of individuals:

   Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,
   Jean-loup Gailly, Hunter Goatley, Ed Gordon, Ian Gorman, Chris Herborth,
   Dirk Haase, Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz,
   David Kirschbaum, Johnny Lee, Onno van der Linden, Igor Mandrichenko,
   Steve P. Miller, Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs,
   Kai Uwe Rommel, Steve Salisbury, Dave Smith, Steven M. Schweda,
   Christian Spieler, Cosmin Truta, Antoine Verheijen, Paul von Behren,
   Rich Wales, Mike White

This software is provided "as is," without warranty of any kind, express
or implied.  In no event shall Info-ZIP or its contributors be held liable
for any direct, indirect, incidental, special or consequential damages
arising out of the use of or inability to use this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. Redistributions of source code must retain the above copyright notice,
       definition, disclaimer, and this list of conditions.

    2. Redistributions in binary form (compiled executables) must reproduce
       the above copyright notice, definition, disclaimer, and this list of
       conditions in documentation and/or other materials provided with the
       distribution.  The sole exception to this condition is redistribution
       of a standard UnZipSFX binary (including SFXWiz) as part of a
       self-extracting archive; that is permitted without inclusion of this
       license, as long as the normal SFX banner has not been removed from
       the binary or disabled.

    3. Altered versions--including, but not limited to, ports to new operating
       systems, existing ports with new graphical interfaces, and dynamic,
       shared, or static library versions--must be plainly marked as such
       and must not be misrepresented as being the original source.  Such
       altered versions also must not be misrepresented as being Info-ZIP
       releases--including, but not limited to, labeling of the altered
       versions with the names "Info-ZIP" (or any variation thereof, including,
       but not limited to, different capitalizations), "Pocket UnZip," "WiZ"
       or "MacZip" without the explicit permission of Info-ZIP.  Such altered
       versions are further prohibited from misrepresentative use of the
       Zip-Bugs or Info-ZIP e-mail addresses or of the Info-ZIP URL(s).

    4. Info-ZIP retains the right to use the names "Info-ZIP," "Zip," "UnZip,"
       "UnZipSFX," "WiZ," "Pocket UnZip," "Pocket Zip," and "MacZip" for its
       own source and binary releases.

/---------------------------------------------------------------------/

*/

/*
 *  zip.h by Mark Adler
 */
#ifndef __zip_h
#define __zip_h 1

#define ZIP   /* for crypt.c:  include zip password functions, not unzip */

/* Set up portability */
#include "tailor.h"

#ifdef USE_ZLIB
#  include "zlib.h"
#endif

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#ifndef WSIZE
#  define WSIZE  (0x8000)
#endif
/* Maximum window size = 32K. If you are really short of memory, compile
 * with a smaller WSIZE but this reduces the compression ratio for files
 * of size > WSIZE. WSIZE must be a power of two in the current implementation.
 */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

/* Forget FILENAME_MAX (incorrectly = 14 on some System V) */
#ifdef DOS
#  define FNMAX 256
#else
#  define FNMAX 1024
#endif

#ifndef MATCH
#  define MATCH shmatch         /* Default for pattern matching: UNIX style */
#endif

/* Types centralized here for easy modification */
#define local static            /* More meaningful outside functions */
typedef unsigned char uch;      /* unsigned 8-bit value */
typedef unsigned short ush;     /* unsigned 16-bit value */
typedef unsigned long ulg;      /* unsigned 32-bit value */


/* Structure carrying extended timestamp information */
typedef struct iztimes {
   time_t atime;                /* new access time */
   time_t mtime;                /* new modification time */
   time_t ctime;                /* new creation time (!= Unix st.ctime) */
} iztimes;

/* Lengths of headers after signatures in bytes */
#define LOCHEAD 26
#define CENHEAD 42
#define ENDHEAD 18

/* Structures for in-memory file information */
struct zlist {
  /* See central header in zipfile.c for what vem..off are */
  /* Do not rearrange these as less than smart coding in zipfile.c
     in scanzipf_reg() depends on u being set to ver and then stepping
     through as a byte array.  Ack.  Should be fixed.  5/25/2005 EG */
  ush vem, ver, flg, how;
  ulg tim, crc;
  uzoff_t siz, len;             /* zip64 support 08/29/2003 R.Nausedat */
  /* changed from extent to ush 3/10/2005 EG */
  ush nam, ext, cext, com;      /* offset of ext must be >= LOCHEAD */
  ulg dsk;                      /* disk number was ush but now ulg */
  ush att, lflg;                /* offset of lflg must be >= LOCHEAD */
  uzoff_t off;
  ulg atx;
  char *name;                   /* File name in zip file */
  char *extra;                  /* Extra field (set only if ext != 0) */
  char *cextra;                 /* Extra in central (set only if cext != 0) */
  char *comment;                /* Comment (set only if com != 0) */
  char *iname;                  /* Internal file name after cleanup (stored in archive) */
  char *zname;                  /* External version of internal name */
  char *oname;                  /* Display version of name used in messages */
#ifdef UNICODE_SUPPORT
  /* Unicode support - 10/11/05 EG */
  char *uname;                  /* UTF-8 version of iname */
#endif
  int mark;                     /* Marker for files to operate on */
  int trash;                    /* Marker for files to delete */
  int dosflag;                  /* Set to force MSDOS file attributes */
  struct zlist far *nxt;        /* Pointer to next header in list */
};
struct flist {
  char *name;                   /* Raw internal file name */
  char *iname;                  /* Internal file name after cleanup */
  char *zname;                  /* External version of internal name */
  char *oname;                  /* Display version of internal name */
#ifdef UNICODE_SUPPORT
  char *uname;                  /* UTF-8 name */
#endif
  int dosflag;                  /* Set to force MSDOS file attributes */
  uzoff_t usize;                /* usize from initial scan */
  struct flist far *far *lst;   /* Pointer to link pointing here */
  struct flist far *nxt;        /* Link to next name */
};
struct plist {
  char *zname;                  /* External version of internal name */
  int select;                   /* Selection flag ('i' or 'x') */
};

/* internal file attribute */
#define UNKNOWN (-1)
#define BINARY  0
#define ASCII   1
#define __EBCDIC 2

/* extra field definitions */
#define EF_VMCMS     0x4704   /* VM/CMS Extra Field ID ("G")*/
#define EF_MVS       0x470f   /* MVS Extra Field ID ("G")   */
#define EF_IZUNIX    0x5855   /* UNIX Extra Field ID ("UX") */
#define EF_IZUNIX2   0x7855   /* Info-ZIP's new Unix ("Ux") */
#define EF_TIME      0x5455   /* universal timestamp ("UT") */
#define EF_OS2EA     0x0009   /* OS/2 Extra Field ID (extended attributes) */
#define EF_ACL       0x4C41   /* ACL Extra Field ID (access control list, "AL") */
#define EF_NTSD      0x4453   /* NT Security Descriptor Extra Field ID, ("SD") */
#define EF_BEOS      0x6542   /* BeOS Extra Field ID ("Be") */
#define EF_ATHEOS    0x7441   /* AtheOS Extra Field ID ("At") */
#define EF_QDOS      0xfb4a   /* SMS/QDOS ("J\373") */
#define EF_AOSVS     0x5356   /* AOS/VS ("VS") */
#define EF_SPARK     0x4341   /* David Pilling's Acorn/SparkFS ("AC") */
#define EF_THEOS     0x6854   /* THEOS ("Th") */
#define EF_TANDEM    0x4154   /* Tandem NSK ("TA") */

/* Definitions for extra field handling: */
#define EF_SIZE_MAX  ((unsigned)0xFFFF) /* hard limit of total e.f. length */
#define EB_HEADSIZE       4     /* length of a extra field block header */
#define EB_ID             0     /* offset of block ID in header */
#define EB_LEN            2     /* offset of data length field in header */
#define EB_MEMCMPR_HSIZ   6     /* header length for memcompressed data */
#define EB_DEFLAT_EXTRA  10     /* overhead for 64kByte "undeflatable" data */

#define EB_UX_MINLEN      8     /* minimal "UX" field contains atime, mtime */
#define EB_UX_ATIME       0     /* offset of atime in "UX" extra field data */
#define EB_UX_MTIME       4     /* offset of mtime in "UX" extra field data */

#define EB_UX_FULLSIZE    12    /* full "UX" field (atime, mtime, uid, gid) */
#define EB_UX_UID         8     /* byte offset of UID in "UX" field data */
#define EB_UX_GID         10    /* byte offset of GID in "UX" field data */

#define EB_UT_MINLEN      1     /* minimal UT field contains Flags byte */
#define EB_UT_FLAGS       0     /* byte offset of Flags field */
#define EB_UT_TIME1       1     /* byte offset of 1st time value */
#define EB_UT_FL_MTIME    (1 << 0)      /* mtime present */
#define EB_UT_FL_ATIME    (1 << 1)      /* atime present */
#define EB_UT_FL_CTIME    (1 << 2)      /* ctime present */
#define EB_UT_LEN(n)      (EB_UT_MINLEN + 4 * (n))

#define EB_UX2_MINLEN     4     /* minimal Ux field contains UID/GID */
#define EB_UX2_UID        0     /* byte offset of UID in "Ux" field data */
#define EB_UX2_GID        2     /* byte offset of GID in "Ux" field data */
#define EB_UX2_VALID      (1 << 8)      /* UID/GID present */

/* ASCII definitions for line terminators in text files: */
#define LF     10        /* '\n' on ASCII machines; must be 10 due to EBCDIC */
#define CR     13        /* '\r' on ASCII machines; must be 13 due to EBCDIC */
#define CTRLZ  26        /* DOS & OS/2 EOF marker (used in fileio.c, vms.c) */

/* return codes of password fetches (negative: user abort; positive: error) */
#define IZ_PW_ENTERED   0       /* got some PWD string, use/try it */
#define IZ_PW_CANCEL    -1      /* no password available (for this entry) */
#define IZ_PW_CANCELALL -2      /* no password, skip any further PWD request */
#define IZ_PW_ERROR     5       /* = PK_MEM2 : failure (no mem, no tty, ...) */
#define IZ_PW_SKIPVERIFY IZ_PW_CANCEL   /* skip encrypt. passwd verification */

/* mode flag values of password prompting function */
#define ZP_PW_ENTER     0       /* request for encryption password */
#define ZP_PW_VERIFY    1       /* request for reentering password */

/* Error return codes and PERR macro */
#include "ziperr.h"

#if 0            /* Optimization: use the (const) result of crc32(0L,NULL,0) */
#  define CRCVAL_INITIAL  crc32(0L, (uch *)NULL, 0)
#else
#  define CRCVAL_INITIAL  0L
#endif

#define DOSTIME_MINIMUM         ((ulg)0x00210000L)
#define DOSTIME_2038_01_18      ((ulg)0x74320000L)


/* Public globals */
extern uch upper[256];          /* Country dependent case map table */
extern uch lower[256];
#ifdef EBCDIC
extern ZCONST uch ascii[256];   /* EBCDIC <--> ASCII translation tables */
extern ZCONST uch ebcdic[256];
#endif /* EBCDIC */

/* Are these ever used?  6/12/05 EG */
#ifdef IZ_ISO2OEM_ARRAY         /* ISO 8859-1 (Win CP 1252) --> OEM CP 850 */
extern ZCONST uch Far iso2oem[128];
#endif
#ifdef IZ_OEM2ISO_ARRAY         /* OEM CP 850 --> ISO 8859-1 (Win CP 1252) */
extern ZCONST uch Far oem2iso[128];
#endif

extern char errbuf[FNMAX+81];   /* Handy place to build error messages */
extern int recurse;             /* Recurse into directories encountered */
extern int dispose;             /* Remove files after put in zip file */
extern int pathput;             /* Store path with name */

#ifdef RISCOS
extern int scanimage;           /* Scan through image files */
#endif

#define BEST -1                 /* Use best method (deflation or store) */
#define STORE 0                 /* Store method */
#define DEFLATE 8               /* Deflation method*/
extern int method;              /* Restriction on compression method */

extern int dosify;              /* Make new entries look like MSDOS */
extern char *special;           /* Don't compress special suffixes */
extern int verbose;             /* Report oddities in zip file structure */
extern int fix;                 /* Fix the zip file */
extern int adjust;              /* Adjust the unzipsfx'd zip file */
extern int level;               /* Compression level */
extern int translate_eol;       /* Translate end-of-line LF -> CR LF */
#ifdef VMS
   extern int vmsver;           /* Append VMS version number to file names */
   extern int vms_native;       /* Store in VMS format */
   extern int vms_case_2;       /* ODS2 file name case in VMS. -1: down. */
   extern int vms_case_5;       /* ODS5 file name case in VMS. +1: preserve. */
#endif /* VMS */
#if defined(OS2) || defined(WIN32)
   extern int use_longname_ea;   /* use the .LONGNAME EA as the file's name */
#endif
#if defined (QDOS) || defined(QLZIP)
extern short qlflag;
#endif
/* 9/26/04 EG */
extern int no_wild;             /* wildcards are disabled */
extern int wild_stop_at_dir;    /* wildcards do not include / in matches */
/* 10/20/04 EG */
extern zoff_t dot_size;         /* if not 0 then display dots every size buffers */
extern zoff_t dot_count;        /* if dot_size not 0 counts buffers */
/* status 10/30/04 EG */
extern int display_counts;      /* display running file count */
extern int display_bytes;       /* display running bytes remaining */
extern int display_globaldots;  /* display dots for archive instead of for each file */
extern int display_usize;       /* display uncompressed bytes */
extern ulg files_so_far;        /* files processed so far */
extern ulg bad_files_so_far;    /* files skipped so far */
extern ulg files_total;         /* files total to process */
extern uzoff_t bytes_so_far;    /* bytes processed so far (from initial scan) */
extern uzoff_t good_bytes_so_far;/* good bytes read so far */
extern uzoff_t bad_bytes_so_far;/* bad bytes skipped so far */
extern uzoff_t bytes_total;     /* total bytes to process (from initial scan) */
/* logfile 6/5/05 EG */
extern int logall;          /* 0 = warnings/errors, 1 = all */
extern FILE *logfile;           /* pointer to open logfile or NULL */
extern int logfile_append;      /* append to existing logfile */
extern char *logfile_path;      /* pointer to path of logfile */
#ifdef UNICODE_SUPPORT
/* Unicode 10/12/05 EG */
extern int use_wide_to_mb_default;/* use the default MB char instead of escape */
#endif

extern int hidden_files;        /* process hidden and system files */
extern int volume_label;        /* add volume label */
extern int dirnames;            /* include directory names */
extern int linkput;             /* Store symbolic links as such */
extern int noisy;               /* False for quiet operation */
extern int extra_fields;        /* do not create extra fields */
#ifdef NTSD_EAS
 extern int use_privileges;     /* use security privilege overrides */
#endif
extern int use_descriptors;     /* use data descriptors (extended headings) */
extern int allow_empty_archive; /* if no files, create empty archive anyway */
extern int zip_to_stdout;       /* output to stdout */
extern int output_seekable;     /* 1 = output seekable 3/13/05 EG */
#ifdef ZIP64_SUPPORT            /* zip64 globals 10/4/03 E. Gordon */
 extern int force_zip64;        /* force use of zip64 when streaming from stdin */
 extern int zip64_entry;        /* current entry needs Zip64 */
 extern int zip64_archive;      /* at least 1 entry needs zip64 */
#endif
 
extern char *tempzip;           /* temp file name */
extern FILE *y;                 /* output file now global for splits */

extern ulg before;              /* 0=ignore, else exclude files before this time */
extern ulg after;               /* 0=ignore, else exclude files newer than this time */

extern ulg    current_local_disk; /* disk with current local header */

#ifdef SPLIT_SUPPORT
 extern ulg     current_disk;     /* current disk number */
 extern int     cd_start_disk;    /* central directory start disk */
 extern uzoff_t cd_start_offset;  /* offset of start of cd on cd start disk */
 extern uzoff_t cd_entries_this_disk; /* cd entries this disk */
 extern uzoff_t total_cd_entries; /* total cd entries */
 extern ulg     zip64_eocd_disk;  /* disk with Zip64 EOCD Record */
 extern uzoff_t zip64_eocd_offset; /* offset of Zip64 EOCD Record */
  /* for split method 1 (keep split with local header open and update) */
 extern char *current_local_tempname; /* name of temp file */
 extern FILE  *current_local_file; /* file pointer for current local header */
 extern uzoff_t current_local_offset; /* offset to start of current local header */
  /* global */
 extern uzoff_t bytes_this_split; /* bytes written to current split */
 extern int read_split_archive; /* 1=scanzipf_reg detected spanning signature */
 extern int split_method;       /* 0=no splits, 1=update LHs, 2=data descriptors */
 extern uzoff_t split_size;     /* how big each split should be */
 extern uzoff_t bytes_prev_splits; /* total bytes written to all splits before this */
 extern uzoff_t bytes_this_entry; /* bytes written for this entry across all splits */
 extern int noisy_splits;       /* note when splits are being created */
#endif
extern int adding_msg_pos;      /* "adding:" message position/state. */

extern char *key;               /* Scramble password or NULL */
extern char *tempath;           /* Path for temporary files */
extern FILE *mesg;              /* Where informational output goes */
extern char *zipfile;           /* New or existing zip archive (zip file) */
extern char *out_path;          /* Name of output file, usually same as zipfile */
extern int zip_attributes;

/* zip64 support 08/31/2003 R.Nausedat */
extern uzoff_t zipbeg;          /* Starting offset of zip structures */
extern uzoff_t cenbeg;          /* Starting offset of central directory */
extern uzoff_t tempzn;          /* Count of bytes written to output zip file */

/* NOTE: zcount and fcount cannot exceed "size_t" (resp. "extent") range.
   This is an internal limitation built into Zip's action handling:
   Zip keeps "{z|f}count * struct {z|f}list" arrays in (flat) memory,
   for sorting, file matching, and building the central-dir structures.
 */

extern struct zlist far *zfiles;/* Pointer to list of files in zip file */
extern extent zcount;           /* Number of files in zip file */
extern ush zcomlen;             /* Length of zip file comment */
extern char *zcomment;          /* Zip file comment (not zero-terminated) */
extern struct zlist far **zsort;/* List of files sorted by name */
extern struct flist far *found; /* List of names found */
extern struct flist far *far *fnxt;     /* Where to put next in found list */
extern extent fcount;           /* Count of names in found list */

extern struct plist *patterns;  /* List of patterns to be matched */
extern unsigned pcount;         /* number of patterns */
extern unsigned icount;         /* number of include only patterns */
extern unsigned Rcount;         /* number of -R include patterns */

#ifdef IZ_CHECK_TZ
extern int zp_tz_is_valid;      /* signals "timezone info is available" */
#endif
#if (defined(MACOS) || defined(WINDLL))
extern int zipstate;            /* flag "zipfile has been stat()'ed */
#endif

/* Diagnostic functions */
#ifdef DEBUG
# ifdef MSDOS
#  undef  stderr
#  define stderr stdout
# endif
#  define diag(where) fprintf(stderr, "zip diagnostic: %s\n", where)
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
# ifdef THEOS
#  define Trace(x) _fprintf x
#  define Tracev(x) {if (verbose) _fprintf x ;}
#  define Tracevv(x) {if (verbose>1) _fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) _fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) _fprintf x ;}
# else
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
# endif
#else
#  define diag(where)
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifdef DEBUGNAMES
#  define free(x) { int *v;Free(x); v=x;*v=0xdeadbeef;x=(void *)0xdeadbeef; }
#endif

/* Public function prototypes */

#ifndef UTIL
#ifdef USE_ZIPMAIN
int zipmain OF((int, char **));
#else
int main OF((int, char **));
#endif /* USE_ZIPMAIN */
#endif

#ifdef EBCDIC
extern int aflag;
#endif /* EBCDIC */
#ifdef CMS_MVS
extern int bflag;
#endif /* CMS_MVS */
void zipmessage OF((ZCONST char *, ZCONST char *));
void zipwarn OF((ZCONST char *, ZCONST char *));
void ziperr OF((int, ZCONST char *));
#ifdef UTIL
#  define error(msg)    ziperr(ZE_LOGIC, msg)
#else
   void error OF((ZCONST char *));
#  ifdef VMSCLI
     void help OF((void));
#  endif
   int encr_passwd OF((int, char *, int, ZCONST char *));
#endif

        /* in zipup.c */
#ifndef UTIL
  /* zip64 support 08/31/2003 R.Nausedat */
   int percent OF((uzoff_t, uzoff_t));

   int zipup OF((struct zlist far *));
#  ifdef USE_ZLIB
     void zl_deflate_free OF((void));
#  else
     void flush_outbuf OF((char *, unsigned *));
     int seekable OF((void));
     extern unsigned (*read_buf) OF((char *, unsigned int));
#  endif /* !USE_ZLIB */
#  ifdef ZP_NEED_MEMCOMPR
     ulg memcompress OF((char *, ulg, char *, ulg));
#  endif
#endif /* !UTIL */

        /* in zipfile.c */
#ifndef UTIL
   struct zlist far *zsearch OF((ZCONST char *));
#  ifdef USE_EF_UT_TIME
     int get_ef_ut_ztime OF((struct zlist far *, iztimes *));
#  endif /* USE_EF_UT_TIME */
   int trash OF((void));
#endif /* !UTIL */
char *ziptyp OF((char *));
int readzipfile OF((void));
int putlocal OF((struct zlist far *, int));
int putextended OF((struct zlist far *));
int putcentral OF((struct zlist far *));
/* zip64 support 09/05/2003 R.Nausedat */
int putend OF((uzoff_t, uzoff_t, uzoff_t, ush, char *));
/* moved seekable to separate function 3/14/05 EG */
int is_seekable OF((FILE *));
int zipcopy OF((struct zlist far *, FILE *));

        /* in fileio.c */
#ifndef UTIL
   char *getnam OF((FILE *));
   struct flist far *fexpel OF((struct flist far *));
   char *last OF((char *, int));
   char *msname OF((char *));
   int check_dup OF((void));
   int filter OF((char *, int));
   int newname OF((char *, int, int));
#endif /* !UTIL */
#if (!defined(UTIL) || defined(W32_STATROOT_FIX))
   time_t dos2unixtime OF((ulg));
#endif
#ifndef UTIL
   ulg dostime OF((int, int, int, int, int, int));
   ulg unix2dostime OF((time_t *));
   int issymlnk OF((ulg a));
#  ifdef S_IFLNK
#    define rdsymlnk(p,b,n) readlink(p,b,n)
/*   extern int readlink OF((char *, char *, int)); */
#  else /* !S_IFLNK */
#    define rdsymlnk(p,b,n) (0)
#  endif /* !S_IFLNK */
#endif /* !UTIL */

int destroy OF((char *));
int replace OF((char *, char *));
int getfileattr OF((char *));
int setfileattr OF((char *, int));
char *tempname OF((char *));

/* for splits */
int close_split OF((int, FILE *, char *));
char *get_split_path OF((int));
int rename_split OF((char *, char *));
int set_filetype OF(());

int bfcopy OF((FILE *, uzoff_t));

int fcopy OF((FILE *, FILE *, uzoff_t));

#ifdef ZMEM
   char *memset OF((char *, int, unsigned int));
   char *memcpy OF((char *, char *, unsigned int));
   int memcmp OF((char *, char *, unsigned int));
#endif /* ZMEM */

        /* in system dependent fileio code (<system>.c) */
#ifndef UTIL
#  ifdef PROCNAME
     int wild OF((char *));
#  endif
   char *in2ex OF((char *));
   char *ex2in OF((char *, int, int *));
   int procname OF((char *, int));
   void stamp OF((char *, ulg));

   ulg filetime OF((char *, ulg *, zoff_t *, iztimes *));

#if !(defined(VMS) && defined(VMS_PK_EXTRA))
   int set_extra_field OF((struct zlist far *, iztimes *));
#endif /* ?(VMS && VMS_PK_EXTRA) */
   int deletedir OF((char *));
#ifdef MY_ZCALLOC
     zvoid far *zcalloc OF((unsigned int, unsigned int));
     zvoid zcfree       OF((zvoid far *));
#endif /* MY_ZCALLOC */
#endif /* !UTIL */
void version_local OF((void));

        /* in util.c */
#ifndef UTIL
int   fseekable    OF((FILE *));
char *isshexp      OF((char *));
int   shmatch      OF((ZCONST char *, ZCONST char *, int));
#if defined(DOS) || defined(WIN32)
   int dosmatch    OF((ZCONST char *, ZCONST char *, int));
#endif /* DOS || WIN32 */
#endif /* !UTIL */

/* functions to convert zoff_t to a string */
char *zip_fuzofft      OF((uzoff_t, char *, char*));
char *zip_fzofft       OF((zoff_t, char *, char*));

/* read and write number strings like 10M */
int DisplayNumString OF ((FILE *file, uzoff_t i));
int WriteNumString OF((uzoff_t num, char *outstring));
uzoff_t ReadNumString OF((char *numstring));

void init_upper    OF((void));
int  namecmp       OF((ZCONST char *string1, ZCONST char *string2));

#ifdef EBCDIC
char *strtoasc     OF((char *str1, ZCONST char *str2));
char *strtoebc     OF((char *str1, ZCONST char *str2));
char *memtoasc     OF((char *mem1, ZCONST char *mem2, unsigned len));
char *memtoebc     OF((char *mem1, ZCONST char *mem2, unsigned len));
#endif /* EBCDIC */
#ifdef IZ_ISO2OEM_ARRAY
char *str_iso_to_oem    OF((char *dst, ZCONST char *src));
#endif
#ifdef IZ_OEM2ISO_ARRAY
char *str_oem_to_iso    OF((char *dst, ZCONST char *src));
#endif

zvoid far **search OF((ZCONST zvoid *, ZCONST zvoid far **, extent,
                       int (*)(ZCONST zvoid *, ZCONST zvoid far *)));
void envargs       OF((int *, char ***, char *, char *));
void expand_args   OF((int *, char ***));

int  is_text_buf   OF((ZCONST char *buf_ptr, unsigned buf_size));

#ifndef USE_ZLIB
#ifndef UTIL
        /* in crc32.c */
ulg  crc32         OF((ulg, ZCONST uch *, extent));
#endif /* !UTIL */

        /* in crctab.c */
ZCONST ulg near *get_crc_table OF((void));
#ifdef DYNALLOC_CRCTAB
void free_crc_table OF((void));
#endif
#endif /* !USE_ZLIB */

#ifndef UTIL
#ifndef USE_ZLIB
        /* in deflate.c */
void lm_init OF((int, ush *));
void lm_free OF((void));

uzoff_t deflate OF((void));

        /* in trees.c */
void     ct_init      OF((ush *, int *));
int      ct_tally     OF((int, int));
uzoff_t  flush_block  OF((char far *, ulg, int));
void     bi_init      OF((char *, unsigned int, int));
#endif /* !USE_ZLIB */
#endif /* !UTIL */

        /* in system specific assembler code, replacing C code in trees.c */
#if defined(ASMV) && defined(RISCOS)
  void     send_bits    OF((int, int));
  unsigned bi_reverse   OF((unsigned int, int));
#endif /* ASMV && RISCOS */

/*---------------------------------------------------------------------------
    VMS-only functions:
  ---------------------------------------------------------------------------*/
#ifdef VMS
   int    vms_stat        OF((char *, stat_t *));              /* vms.c */
   void   vms_exit        OF((int));                           /* vms.c */
#ifndef UTIL
#ifdef VMSCLI
   ulg    vms_zip_cmdline OF((int *, char ***));                /* cmdline.c */
   void   VMSCLI_help     OF((void));                           /* cmdline.c */
#endif /* VMSCLI */
#endif /* !UTIL */
#endif /* VMS */

/*
#ifdef ZIP64_SUPPORT
   update_local_Zip64_extra_field OF((struct zlist far *, FILE *));
#endif
*/

/*---------------------------------------------------------------------------
    WIN32-only functions:
  ---------------------------------------------------------------------------*/
#ifdef WIN32
   int    ZipIsWinNT         OF((void));                         /* win32.c */
#endif /* WIN32 */

#if (defined(WINDLL) || defined(DLL_ZIPAPI))
/*---------------------------------------------------------------------------
    Prototypes for public Zip API (DLL) functions.
  ---------------------------------------------------------------------------*/
#include "api.h"
#endif /* WINDLL || DLL_ZIPAPI */



/*---------------------------------------------------------------------
    Unicode Support
    28 August 2005
  ---------------------------------------------------------------------*/
#ifdef UNICODE_SUPPORT

  /* Default character when a zwchar too big for wchar_t */
# define zwchar_to_wchar_t_default_char '_'

  /* Default character string when wchar_t does not convert to mb */
# define wide_to_mb_default_string "_"

  /* wide character type */
  typedef unsigned long zwchar;

  /* check if string is all ASCII */
  int is_ascii_string OF((char *));
  
  /* convert UTF-8 string to multi-byte string */
  char *utf8_to_local_string OF((char *));

  /* convert UTF-8 string to wide string */
  zwchar *utf8_to_wide_string OF((char *));

  /* convert wide string to multi-byte string */
  char *wide_to_local_string OF((zwchar *));

  /* convert local string to multi-byte display string */
  char *local_to_display_string OF((char *));

#ifdef WIN32
  /* convert local string to oem string */
  char *local_to_oem_string OF((char *, char *));
#endif

  /* convert wide character to escape string */
  char *wide_to_escape_string OF((unsigned long));

  /* convert escape string to wide character */
  unsigned long escape_string_to_wide OF((char *));

  /* convert local to UTF-8 */
  char *local_to_utf8_string OF ((char *));

  /* convert local to wide string */
  zwchar *local_to_wide_string OF ((char *));

  /* convert wide string to UTF-8 */
  char *wide_to_utf8_string OF((zwchar *));

#endif /* UNICODE_SUPPORT */


/*---------------------------------------------------
 * Split archives
 *
 * 10/20/05 EG
 */
  
#define BFWRITE_DATA 0
#define BFWRITE_LOCALHEADER 1
#define BFWRITE_CENTRALHEADER 2
#define BFWRITE_HEADER 3

size_t bfwrite OF((ZCONST void *buffer, size_t size, size_t count,
                   int));

/* for putlocal() */
#define PUTLOCAL_WRITE 0
#define PUTLOCAL_REWRITE 1


/*--------------------------------------------------------------------
    Long option support
    23 August 2003
    See fileio.c
  --------------------------------------------------------------------*/

/* The below is for use in the caller-provided options table */

/* value_type - value is always returned as a string. */
#define o_NO_VALUE        0   /* this option does not take a value */
#define o_REQUIRED_VALUE  1   /* this option requires a value */
#define o_OPTIONAL_VALUE  2   /* value is optional (see get_option() for details) */
#define o_VALUE_LIST      3   /* this option takes a list of values */
#define o_ONE_CHAR_VALUE  4   /* next char is value (does not end short opt string) */
#define o_NUMBER_VALUE    5   /* value is integer (does not end short opt string) */


/* negatable - a dash following the option (but before any value) sets negated. */
#define o_NOT_NEGATABLE   0   /* trailing '-' to negate either starts value or generates error */
#define o_NEGATABLE       1   /* trailing '-' sets negated to TRUE */


/* option_num can be this when option not in options table */
#define o_NO_OPTION_MATCH     -1

/* special values returned by get_option - do not use these as option IDs */
#define o_NON_OPTION_ARG      ((unsigned long) 0xFFFF)    /* returned for non-option
                                                             args */
#define o_ARG_FILE_ERR        ((unsigned long) 0xFFFE)    /* internal recursion
                                                             return (user never sees) */

/* options array is set in zip.c */
struct option_struct {
  char *shortopt;           /* char * to sequence of char that is short option */
  char *longopt;            /* char * to long option string */
  int  value_type;          /* from above */
  int  negatable;           /* from above */
  unsigned long option_ID;  /* value returned by get_option when this option is found */
  char *name;               /* optional string for option returned on some errors */
};
extern struct option_struct options[];


/* moved here from fileio.c to make global - 10/6/05 EG */

/* If will support wide for Unicode then need to add */
  /* multi-byte */
#ifdef _MBCS
# ifndef MULTIBYTE_GETOPTNS
#   define MULTIBYTE_GETOPTNS
# endif
#endif
#ifdef MULTIBYTE_GETOPTNS
  int mb_clen OF((ZCONST char *));
# define MB_CLEN(ptr) mb_clen(ptr)
# define MB_NEXTCHAR(ptr) ((ptr) += MB_CLEN(ptr))
#else
  /* no multi-byte */
# define MB_CLEN(ptr) (1)
# define MB_NEXTCHAR(ptr) ((ptr)++)
#endif


/* function prototypes */

/* get the next option from args */
unsigned long get_option OF((char ***pargs, int *argc, int *argnum, int *optchar,
                             char **value, int *negated, int *first_nonopt_arg,
                             int *option_num, int recursion_depth));

/* copy args - copy an args array, allocating space as needed */
char **copy_args OF((char **args, int max_args));

/* free args - free args created with one of these functions */
int free_args OF ((char **args));

/* insert arg - copy an arg into args */
int insert_arg OF ((char ***args, char *arg, int insert_at, int free_args));


/*--------------------------------------------------------------------
    End of Long option support
  --------------------------------------------------------------------*/


#endif /* !__zip_h */
/* end of zip.h */
