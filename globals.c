/*
  globals.c - Zip 3

  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  globals.c by Mark Adler
 */
#define __GLOBALS_C

#define GLOBALS         /* include definition of errors[] in zip.h */
#ifndef UTIL
#define UTIL            /* do not declare the read_buf variable */
#endif

#include "zip.h"


/* Handy place to build error messages */
char errbuf[FNMAX+81];

/* Argument processing globals */
int recurse = 0;        /* 1=recurse into directories encountered */
int dispose = 0;        /* 1=remove files after put in zip file */
int pathput = 1;        /* 1=store path with name */
#ifdef RISCOS
int scanimage = 1;      /* 1=scan through image files */
#endif
int method = BEST;      /* one of BEST, DEFLATE (only), or STORE (only) */
int dosify = 0;         /* 1=make new entries look like MSDOS */
int verbose = 0;        /* 1=report oddities in zip file structure */
int fix = 0;            /* 1=fix the zip file */
int adjust = 0;         /* 1=adjust offsets for sfx'd file (keep preamble) */
int level = 6;          /* 0=fastest compression, 9=best compression */
int translate_eol = 0;  /* Translate end-of-line LF -> CR LF */
#ifdef VMS
   int vmsver = 0;      /* 1=append VMS version number to file names */
   int vms_native = 0;  /* 1=store in VMS format */
   int vms_case_2 = 0;  /* ODS2 file name case in VMS.  -1: down. */
   int vms_case_5 = 0;  /* ODS5 file name case in VMS.  +1: preserve. */
#endif /* VMS */
#if defined(OS2) || defined(WIN32)
   int use_longname_ea = 0;   /* 1=use the .LONGNAME EA as the file's name */
#endif
/* 9/26/04 EG */
int no_wild = 0;              /* 1 = wildcards are disabled */
#ifdef WILD_STOP_AT_DIR
   int wild_stop_at_dir = 1;  /* default wildcards do not include / in matches */
#else
   int wild_stop_at_dir = 0;  /* default wildcards do include / in matches */
#endif

/* dots 10/20/04 EG */
int dot_size = 0;             /* buffers in deflate per dot, 0 = no dots */
int dot_count = 0;            /* buffers seen, recyles at dot_size */
/* status 10/30/04 EG */
int display_counts = 0;       /* display running file count */
int display_bytes = 0;        /* display running bytes remaining */
int display_usize = 0;        /* display uncompressed bytes */
ulg files_so_far = 0;         /* files processed so far */
ulg bad_files_so_far = 0;     /* bad files skipped so far */
ulg files_total = 0;          /* files total to process */
uzoff_t bytes_so_far = 0;     /* bytes processed so far (from initial scan) */
uzoff_t good_bytes_so_far = 0;/* good bytes read so far */
uzoff_t bad_bytes_so_far = 0; /* bad bytes skipped so far */
uzoff_t bytes_total = 0;      /* total bytes to process (from initial scan) */
/* logfile 6/5/05 EG */
int logall = 0;               /* 0 = warnings/errors, 1 = all */
FILE *logfile = NULL;         /* pointer to open logfile or NULL */
int logfile_append = 0;       /* append to existing logfile */
char *logfile_path = NULL;    /* pointer to path of logfile */

int hidden_files = 0;         /* process hidden and system files */
int volume_label = 0;         /* add volume label */
int dirnames = 1;             /* include directory entries by default */
int linkput = 0;              /* 1=store symbolic links as such */
int noisy = 1;                /* 0=quiet operation */
int extra_fields = 1;         /* 0=do not create extra fields */
int use_descriptors = 0;      /* 1=use data descriptors 12/29/04 EG */
int zip_to_stdout = 0;        /* output zipfile to stdout 12/30/04 EG */

int output_seekable = 1;      /* 1 = output seekable 3/13/05 EG */

#ifdef ZIP64_SUPPORT          /* zip64 support 10/4/03 E. Gordon */
  int force_zip64 = 0;        /* if 1 force entries to be zip64 */
                              /* mainly for streaming from stdin */
  zoff_t Z64_EFPos = 0;       /* file position of Zip64 Extra Field */
  int zip64_entry = 0;        /* current entry needs Zip64 */
  int zip64_archive = 0;      /* if 1 then at least 1 entry needs zip64 */
#endif

#ifdef NTSD_EAS
  int use_privileges = 0;     /* 1=use security privilege overrides */
#endif
#ifndef RISCOS
#ifndef QDOS
#ifndef TANDEM
char *special = ".Z:.zip:.zoo:.arc:.lzh:.arj"; /* List of special suffixes */
#else /* TANDEM */
char *special = " Z: zip: zoo: arc: lzh: arj"; /* List of special suffixes */
#endif
#else /* QDOS */
char *special = "_Z:_zip:_zoo:_arc:_lzh:_arj"; /* List of special suffixes */
#endif
#else /* RISCOS */
char *special = "DDC:D96:68E";
#endif /* ?RISCOS */
char *key = NULL;       /* Scramble password if scrambling */
char *tempath = NULL;   /* Path for temporary files */
FILE *mesg;             /* stdout by default, stderr for piping */

/* Zip file globals */
char *zipfile;          /* New or existing zip archive (zip file) */

/* zip64 support 08/31/2003 R.Nausedat */
uzoff_t zipbeg;               /* Starting offset of zip structures */
uzoff_t cenbeg;               /* Starting offset of central directory */
uzoff_t tempzn;               /* Count of bytes written to output zip file */

#ifdef SPLIT_SUPPORT          /* splits 8/7/04 EG */
  ulg    current_disk = 0;          /* current disk number */
  ulg    cd_start_disk = 0;         /* central directory start disk */
  uzoff_t cd_start_offset = 0;      /* offset of start of cd on cd start disk */
  uzoff_t cd_entries_this_disk = 0; /* cd entries this disk */
  uzoff_t total_cd_entries = 0;     /* total cd entries */
  /* for split method 1 (keep split with local header open and update) */
  FILE  *current_local_file = NULL; /* file pointer for current local header */
  ulg    current_local_disk = 0;    /* disk with current local header */
  uzoff_t current_local_offset = 0; /* offset to start of current local header */
  /* global */
  int read_split_archive = 0;       /* 1=scanzipf_reg detected spanning signature */
  int split_method = 0;             /* 0=no splits, 1=update LHs, 2=data descriptors */
  uzoff_t split_size = 0;           /* how big each split should be */
  uzoff_t bytes_prev_splits = 0;    /* total bytes written to all splits before this */
  uzoff_t bytes_this_split_entry = 0;/* bytes written for this entry in this split */
#endif

struct zlist far *zfiles = NULL;  /* Pointer to list of files in zip file */
/* The limit for number of files using the Zip64 format is 2^64 - 1 (8 bytes)
   but extent is used for many internal sorts and other tasks and is generally
   long on 32-bit systems.  Because of that, but more because of various memory
   utilization issues limiting the practical number of central directory entries
   that can be sorted, the number of actual entries that can be stored probably
   can't exceed roughly 2^30 on 32-bit systems so extent is probably sufficient. */
extent zcount;                    /* Number of files in zip file */
ush zcomlen;                      /* Length of zip file comment */
char *zcomment = NULL;            /* Zip file comment (not zero-terminated) */
struct zlist far **zsort;         /* List of files sorted by name */

/* Files to operate on that are not in zip file */
struct flist far *found = NULL;   /* List of names found */
struct flist far * far *fnxt = &found;
                                  /* Where to put next name in found list */
extent fcount;                    /* Count of files in list */

/* Patterns to be matched */
struct plist *patterns = NULL;  /* List of patterns to be matched */
unsigned pcount = 0;            /* number of patterns */
unsigned icount = 0;            /* number of include only patterns */
unsigned Rcount = 0;            /* number of -R include patterns */

#ifdef IZ_CHECK_TZ
int zp_tz_is_valid;     /* signals "timezone info is available" */
#endif
