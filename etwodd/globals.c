/*
  globals.c - Zip 3

  Copyright (c) 1990-2013 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-2 or later
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
char errbuf[FNMAX+4081];

/* Argument processing globals */
int recurse = 0;        /* 1=recurse into directories encountered */
int dispose = 0;        /* 1=remove files after put in zip file */
int pathput = 1;        /* 1=store path with name */
#if defined( UNIX) && defined( __APPLE__)
int data_fork_only = 0; /* 1=no AppleDouble supplement file. */
int sequester = 0;      /* 1=sequester AppleDouble files in __MACOSX. */
#endif /* defined( UNIX) && defined( __APPLE__) */
#ifdef RISCOS
int scanimage = 1;      /* 1=scan through image files */
#endif
int method = BEST;      /* one of BEST, DEFLATE (only), or STORE (only) */
int dosify = 0;         /* 1=make new entries look like MSDOS */
int verbose = 0;        /* 1=report oddities in zip file structure */
int fix = 0;            /* 1=fix the zip file, 2=FF, 3=ZipNote */
int filesync = 0;       /* 1=file sync, delete entries not on file system */
int adjust = 0;         /* 1=adjust offsets for sfx'd file (keep preamble) */
int translate_eol = 0;  /* Translate end-of-line LF -> CR LF */
int level = 6;          /* 0=fastest compression, 9=best compression */
int levell;             /* Compression level, adjusted by method, suffix. */
#if defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT)
int etwodd;             /* Encrypt Traditional without data descriptor. */
#endif /* defined( IZ_CRYPT_TRAD) && defined( ETWODD_SUPPORT) */

#ifdef VMS
   int prsrv_vms = 0;   /* 1=preserve idiosyncratic VMS file names. */
   int vmsver = 0;      /* 1=append VMS version number to file names */
   int vms_native = 0;  /* 1=store in VMS format */
   int vms_case_2 = 0;  /* ODS2 file name case in VMS.  -1: down. */
   int vms_case_5 = 0;  /* ODS5 file name case in VMS.  +1: preserve. */
#endif /* VMS */
#if defined(OS2) || defined(WIN32)
   int use_longname_ea = 0;  /* 1=use the .LONGNAME EA as the file's name */
#endif
/* 9/26/04 */
int no_wild = 0;             /* 1 = wildcards are disabled */
int allow_regex = 0;         /* 1 = allow [list] matching */
#ifdef WILD_STOP_AT_DIR
   int wild_stop_at_dir = 1; /* default wildcards do not include / in matches */
#else
   int wild_stop_at_dir = 0; /* default wildcards do include / in matches */
#endif

#ifdef UNICODE_SUPPORT
   int using_utf8 = 0;       /* 1 if current character set UTF-8 */
# ifdef WIN32
   int no_win32_wide = -1;   /* 1 = no wide functions, like GetFileAttributesW() */
# endif
#endif

ulg skip_this_disk = 0;
int des_good = 0;             /* Good data descriptor found */
ulg des_crc = 0;              /* Data descriptor CRC */
uzoff_t des_csize = 0;        /* Data descriptor csize */
uzoff_t des_usize = 0;        /* Data descriptor usize */

/* dots 10/20/04 */
zoff_t dot_size = 0;          /* bytes processed in deflate per dot, 0 = no dots */
zoff_t dot_count = 0;         /* buffers seen, recyles at dot_size */
/* status 10/30/04 */
int display_counts = 0;       /* display running file count */
int display_bytes = 0;        /* display running bytes remaining */
int display_globaldots = 0;   /* display dots for archive instead of each file */
int display_volume = 0;       /* display current input and output volume (disk) numbers */
int display_usize = 0;        /* display uncompressed bytes */
int display_time = 0;         /* display time start each entry */
int display_est_to_go = 0;    /* display estimated time to go */
int display_zip_rate = 0;     /* display bytes per second rate */

ulg files_so_far = 0;         /* files processed so far */
ulg bad_files_so_far = 0;     /* bad files skipped so far */
ulg files_total = 0;          /* files total to process */
uzoff_t bytes_so_far = 0;     /* bytes processed so far (from initial scan) */
uzoff_t good_bytes_so_far = 0;/* good bytes read so far */
uzoff_t bad_bytes_so_far = 0; /* bad bytes skipped so far */
uzoff_t bytes_total = 0;      /* total bytes to process (from initial scan) */

time_t clocktime;             /* current time */
#ifdef ENABLE_ENTRY_TIMING
  uzoff_t start_zip_time = 0; /* when start zipping files (after scan) in usec */
  uzoff_t current_time = 0;   /* current time in usec */
#endif


/* logfile 6/5/05 */
int logall = 0;               /* 0 = warnings/errors, 1 = all */
FILE *logfile = NULL;         /* pointer to open logfile or NULL */
int logfile_append = 0;       /* append to existing logfile */
char *logfile_path = NULL;    /* pointer to path of logfile */
int use_outpath_for_log = 0;  /* 1 = use output archive path for log */
int log_utf8 = 0;             /* log names as UTF-8 */

int hidden_files = 0;         /* process hidden and system files */
int volume_label = 0;         /* add volume label */
int dirnames = 1;             /* include directory entries by default */
int filter_match_case = 1;    /* 1=match case when filter() */
char *label = NULL;           /* Volume label. */

/* diff and backup */
int diff_mode = 0;            /* 1=diff mode - only store changed and add */
#if defined(WIN32)
  int only_archive_set = 0;   /* include only files with DOS archive bit set */
  int clear_archive_bits = 0; /* clear DOS archive bit of included files */
#endif
#ifdef BACKUP_SUPPORT
char *backup_path = NULL;     /* path to save backup archives and control */
int backup_type = 0;          /* 0=no,1=full backup,2=diff,3=incr */
char *backup_start_datetime = NULL; /* date/time stamp of start of backup */
char *backup_control_path = NULL; /* control file used to store backup state */
char *backup_full_path = NULL; /* full archive of backup set */
char *backup_output_path = NULL; /* path of output archive before finalizing */
#endif

uzoff_t cd_total_entries;     /* num of entries as read from (Zip64) EOCDR */
uzoff_t total_cd_total_entries; /* num of entries across all archives */


int linkput = 0;              /* 1=store symbolic links as such */
int noisy = 1;                /* 0=quiet operation */
int extra_fields = 1;         /* 0=create minimum, 1=don't copy old, 2=keep old */
int use_descriptors = 0;      /* 1=use data descriptors 12/29/04 */
int zip_to_stdout = 0;        /* output zipfile to stdout 12/30/04 */
int allow_empty_archive = 0;  /* if no files, create empty archive anyway 12/28/05 */
int copy_only = 0;            /* 1=copying archive entries only */
int allow_fifo = 0;           /* 1=allow reading Unix FIFOs, waiting if pipe open */
int show_files = 0;           /* show files to operate on and exit (=2 log only) */

int sf_usize = 0;             /* include usize in -sf listing */

int output_seekable = 1;      /* 1 = output seekable 3/13/05 EG */

#ifdef ZIP64_SUPPORT          /* zip64 support 10/4/03 */
  int force_zip64 = -1;       /* if 1 force entries to be zip64, 0 force not zip64 */
                              /* mainly for streaming from stdin */
  int zip64_entry = 0;        /* current entry needs Zip64 */
  int zip64_archive = 0;      /* if 1 then at least 1 entry needs zip64 */
#endif

/* encryption */
char *key = NULL;             /* Scramble password if scrambling */
int encryption_method = 0;    /* See definitions in zip.h */
ush aes_vendor_version;
uch aes_strength;
int force_ansi_key = 1;       /* Only ANSI characters for password (32 - 126) */
#ifdef IZ_CRYPT_AES_WG
  int key_size = 0;
  fcrypt_ctx zctx;
  unsigned char *zpwd;
  int zpwd_len;
  unsigned char *zsalt;
  unsigned char zpwd_verifier[PWD_VER_LENGTH];

  prng_ctx aes_rnp;           /* the context for the random number pool */
  unsigned char auth_code[20]; /* returned authentication code */
#endif

#ifdef IZ_CRYPT_AES_WG_NEW
  int key_size = 0;
  ccm_ctx aesnew_ctx;
  unsigned char *zpwd;
  int zpwd_len;
  unsigned char *zsalt;
  unsigned char zpwd_verifier[PWD_VER_LENGTH];
  unsigned char auth_code[20]; /* returned authentication code */
#endif

#ifdef NTSD_EAS
  int use_privileges = 0;     /* 1=use security privilege overrides */
#endif

/* STORE method file name suffixes. */
#ifndef RISCOS
# ifndef QDOS
#  ifndef TANDEM
#   define MTHD_SUFX_0 ".Z:.zip:.zoo:.arc:.lzh:.arj"    /* Normal. */
#  else /* ndef TANDEM */
#   define MTHD_SUFX_0 " Z: zip: zoo: arc: lzh: arj";   /* Tandem. */
#  endif /* ndef TANDEM [else] */
# else /* ndef QDOS */
#  define MTHD_SUFX_0 "_Z:_zip:_zoo:_arc:_lzh:_arj";    /* QDOS. */
# endif /* ndef QDOS [else] */
#else /* ndef RISCOS */
# define MTHD_SUFX_0 "DDC:D96:68E";                     /* RISCOS. */
#endif /* ndef RISCOS [else] */

/* Compression method and level (with file name suffixes). */
mthd_lvl_t mthd_lvl[] = {
/* method, level, level_sufx, method_str, suffixes. */
 { STORE, -1, -1, "store", MTHD_SUFX_0 },   /* STORE.  (Must be element 0.) */
 { DEFLATE, -1, -1, "deflate", NULL },      /* DEFLATE. */
#ifdef BZIP2_SUPPORT
 { BZIP2, -1, -1, "bzip2", NULL },          /* Bzip2. */
#endif /* def BZIP2_SUPPORT */
#ifdef LZMA_SUPPORT
 { LZMA, -1, -1, "lzma", NULL },            /* LZMA. */
#endif /* def LZMA_SUPPORT */
#ifdef PPMD_SUPPORT
 { PPMD, -1, -1, "ppmd", NULL },            /* PPMd. */
#endif /* def PPMD_SUPPORT */
 { -1, -1, -1, NULL, NULL } };              /* List terminator. */

char *tempath = NULL;   /* Path for temporary files */
FILE *mesg;             /* stdout by default, stderr for piping */

char **args = NULL;     /* Copy of argv that can be updated and freed */

char *path_prefix = NULL; /* Prefix to add to all new archive entries */
int path_prefix_mode = 0; /* 0=Prefix all paths, 1=Prefix only added/updated paths */

int all_ascii = 0;      /* Skip binary check and handle all files as text */

#ifdef UNICODE_SUPPORT
 int utf8_native = 0;   /* 1=force storing UTF-8 as standard per AppNote bit 11 */
#endif
int unicode_escape_all = 0; /* 1=escape all non-ASCII characters in paths */
int unicode_mismatch = 1; /* unicode mismatch is 0=error, 1=warn, 2=ignore, 3=no */

int mvs_mode = 0;       /* 0=lastdot (default), 1=dots, 2=slashes */

time_t scan_delay = 5;  /* seconds before display Scanning files message */
time_t scan_dot_time = 2; /* time in seconds between Scanning files dots */
time_t scan_start = 0;  /* start of scan */
time_t scan_last = 0;   /* time of last message */
int scan_started = 0;   /* scan has started */
uzoff_t scan_count = 0; /* Used for Scanning files ... message */

ulg before = 0;         /* 0=ignore, else exclude files before this time */
ulg after = 0;          /* 0=ignore, else exclude files newer than this time */

/* Zip file globals */
char *zipfile;          /* New or existing zip archive (zip file) */

/* zip64 support 08/31/2003 R.Nausedat */
/* all are across splits - subtract bytes_prev_splits to get offsets for current disk */
uzoff_t zipbeg;               /* Starting offset of zip structures */
uzoff_t cenbeg;               /* Starting offset of central dir */
uzoff_t tempzn;               /* Count of bytes written to output zip files */

/* 10/28/05 */
char *tempzip = NULL;         /* name of temp file */
FILE *y = NULL;               /* output file now global so can change in splits */
FILE *in_file = NULL;         /* current input file for splits */
char *in_path = NULL;         /* base name of input archive file */
char *in_split_path = NULL;   /* in split path */
char *out_path = NULL;        /* base name of output file, usually same as zipfile */
int zip_attributes = 0;
char *old_in_path = NULL;     /* used to save in_path when doing incr archives */

/* in split globals */

ulg     total_disks = 0;        /* total disks in archive */
ulg     current_in_disk = 0;    /* current read split disk */
uzoff_t current_in_offset = 0;  /* current offset in current read disk */
ulg     skip_current_disk = 0;  /* if != 0 and fix then skip entries on this disk */


/* out split globals */

ulg     current_local_disk = 0;   /* disk with current local header */

ulg     current_disk = 0;         /* current disk number */
ulg     cd_start_disk = (ulg)-1;  /* central directory start disk */
uzoff_t cd_start_offset = 0;      /* offset of start of cd on cd start disk */
uzoff_t cd_entries_this_disk = 0; /* cd entries this disk */
uzoff_t total_cd_entries = 0;     /* total cd entries in new/updated archive */
ulg     zip64_eocd_disk = 0;      /* disk with Zip64 End Of Central Directory Record */
uzoff_t zip64_eocd_offset = 0;    /* offset for Zip64 EOCD Record */

/* for split method 1 (keep split with local header open and update) */
char *current_local_tempname = NULL; /* name of temp file */
FILE  *current_local_file = NULL; /* file pointer for current local header */
uzoff_t current_local_offset = 0; /* offset to start of current local header */

/* global */
uzoff_t bytes_this_split = 0;     /* bytes written to the current split */
int read_split_archive = 0;       /* 1=scanzipf_reg detected spanning signature */
int split_method = 0;             /* 0=no splits, 1=seekable, 2=data desc, -1=no */
uzoff_t split_size = 0;           /* how big each split should be */
int split_bell = 0;               /* when pause for next split ring bell */
uzoff_t bytes_prev_splits = 0;    /* total bytes written to all splits before this */
uzoff_t bytes_this_entry = 0;     /* bytes written for this entry across all splits */
int noisy_splits = 0;             /* note when splits are being created */
int mesg_line_started = 0;        /* 1=started writing a line to mesg */
int logfile_line_started = 0;     /* 1=started writing a line to logfile */

/* for progress reports */
uzoff_t bytes_read_this_entry = 0; /* bytes read from current input file */
uzoff_t bytes_expected_this_entry = 0; /* scanned uncompressed size */
char *entry_name = NULL;           /* used by DLL to pass z->zname to file_read() */
#ifdef ENABLE_DLL_PROGRESS
 uzoff_t progress_chunk_size = 0;  /* how many bytes before next progress report */
 uzoff_t last_progress_chunk = 0;  /* used to determine when to send next report */
#endif
int show_what_doing = 0;           /* show what doing */

/* For user-triggered progress reports. */
#ifdef ENABLE_USER_PROGRESS
int u_p_phase = 0;
char *u_p_task = NULL;
char *u_p_name = NULL;
#endif /* def ENABLE_USER_PROGRESS */

#ifdef WIN32
  int nonlocal_name = 0;          /* Name has non-local characters */
  int nonlocal_path = 0;          /* Path has non-local characters */
#endif
#ifdef UNICODE_SUPPORT
  int use_wide_to_mb_default = 0;
#endif

struct zlist far *zfiles = NULL;  /* Pointer to list of files in zip file */
/* The limit for number of files using the Zip64 format is 2^64 - 1 (8 bytes)
   but extent is used for many internal sorts and other tasks and is generally
   long on 32-bit systems.  Because of that, but more because of various memory
   utilization issues limiting the practical number of central directory entries
   that can be sorted, the number of actual entries that can be stored probably
   can't exceed roughly 2^30 on 32-bit systems so extent is probably sufficient. */
struct zlist far * far *zfilesnext = NULL;     /* Pointer to end of zfiles */
extent zcount;                    /* Number of files in zip file */
int zipfile_exists = 0;           /* 1 if zipfile exists */
ush zcomlen;                      /* Length of zip file comment */
char *zcomment = NULL;            /* Zip file comment (not zero-terminated) */
struct zlist far **zsort;         /* List of files sorted by name */
#ifdef UNICODE_SUPPORT
  struct zlist far **zusort;      /* List of files sorted by zuname */
#endif

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

/* 2011-12-04 SMS.
 *
 * Old VMS versions (V5.4 with VAX C V3.1-051, for example) may need
 * help to get this stuff linked in.  Using LINK /INCLUDE = GLOBALS
 * would work, except when compiling with CC /NAMES = AS_IS, and the
 * module name "GLOBALS" becomes "globals".  Adding this useless
 * function seems to solve the problem without requiring a right-case
 * LINK /INCLUDE option.  (See also zip.c, where it's referenced.)
 */
#ifdef VMS
void globals_dummy( void)
{
    int dmy = 0;
}
#endif /* def VMS */