/*
  zip.c - Zip 3

  Copyright (c) 1990-2004 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2003-May-08 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  zip.c by Mark Adler.
 */
#define __ZIP_C

#include "zip.h"
#include <time.h>       /* for tzset() declaration */
#ifdef WINDLL
#  include <windows.h>
#  include <setjmp.h>
#  include "windll/windll.h"
#endif
#define DEFCPYRT        /* main module: enable copyright string defines! */
#include "revision.h"
#include "crypt.h"
#include "ttyio.h"
#include <ctype.h>
#ifdef VMS
#  include "vms/vmsmunch.h"
#endif

#ifdef MACOS
#  include "macglob.h"
   extern MacZipGlobals MacZip;
   extern int error_level;
#endif

#if (defined(MSDOS) && !defined(__GO32__)) || defined(__human68k__)
#  include <process.h>
#  if (!defined(P_WAIT) && defined(_P_WAIT))
#    define P_WAIT _P_WAIT
#  endif
#endif

#include <signal.h>

#define MAXCOM 256      /* Maximum one-line comment size */


/* Local option flags */
#ifndef DELETE
#define DELETE  0
#endif
#define ADD     1
#define UPDATE  2
#define FRESHEN 3
local int action = ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
local int comadd = 0;   /* 1=add comments for new files */
local int zipedit = 0;  /* 1=edit zip comment and all file comments */
local int latest = 0;   /* 1=set zip file time to time of latest file */
local ulg before = 0;   /* 0=ignore, else exclude files before this time */
local ulg after = 0;    /* 0=ignore, else exclude files newer than this time */
local int test = 0;     /* 1=test zip file with unzip -t */
local int tempdir = 0;  /* 1=use temp directory (-b) */
local int junk_sfx = 0; /* 1=junk the sfx prefix */
#if defined(AMIGA) || defined(MACOS)
local int filenotes = 0; /* 1=take comments from AmigaDOS/MACOS filenotes */
#endif

#ifdef EBCDIC
int aflag = __EBCDIC;   /* Convert EBCDIC to ASCII or stay EBCDIC ? */
#endif
#ifdef CMS_MVS
int bflag = 0;          /* Use text mode as default */
#endif

#ifdef QDOS
char _version[] = VERSION;
#endif

#ifdef WINDLL
jmp_buf zipdll_error_return;
#ifdef ZIP64_SUPPORT
  unsigned long low, high; /* returning 64 bit values for systems without an _int64 */
  zoff_t filesize64;
#endif
#endif

/* Temporary zip file name and file pointer */
#ifndef MACOS
local char *tempzip;
local FILE *tempzf;
#else
char *tempzip;
FILE *tempzf;
#endif

#if CRYPT
/* Pointer to crc_table, needed in crypt.c */
# if (!defined(USE_ZLIB) || defined(USE_OWN_CRCTAB))
ZCONST ulg near *crc_32_tab;
# else
ZCONST uLongf *crc_32_tab;
# endif
#endif /* CRYPT */

/* Local functions */

local void freeup  OF((void));
local int  finish  OF((int));
#if (!defined(MACOS) && !defined(WINDLL))
local void handler OF((int));
local void license OF((void));
#ifndef VMSCLI
local void help    OF((void));
#endif /* !VMSCLI */
#endif /* !MACOS && !WINDLL */

/* prereading of arguments is not supported in new command
   line interpreter get_option() so read filters as arguments
   are processed and convert to expected array later*/
local int add_filter OF((int flag, char *pattern));
local int filterlist_to_patterns();
/* not used
 local int get_filters OF((int argc, char **argv));
*/

#if (!defined(MACOS) && !defined(WINDLL))
local void check_zipfile OF((char *zipname, char *zippath));
local void version_info OF((void));
local void zipstdout OF((void));
#endif /* !MACOS && !WINDLL */

/* structure used by add_filter to store filters */
struct filterlist_struct {
  char flag;
  char *pattern;
  struct filterlist_struct *next;
};
struct filterlist_struct *filterlist = NULL;  /* start of list */
struct filterlist_struct *lastfilter = NULL;  /* last filter in list */

local void freeup()
/* Free all allocations in the 'found' list, the 'zfiles' list and
   the 'patterns' list. */
{
  struct flist far *f;  /* steps through found list */
  struct zlist far *z;  /* pointer to next entry in zfiles list */

  for (f = found; f != NULL; f = fexpel(f))
    ;
  while (zfiles != NULL)
  {
    z = zfiles->nxt;
    if (zfiles->zname && zfiles->zname != zfiles->name)
      free((zvoid *)(zfiles->zname));
    if (zfiles->name)
      free((zvoid *)(zfiles->name));
    if (zfiles->iname)
      free((zvoid *)(zfiles->iname));
    if (zfiles->cext && zfiles->cextra && zfiles->cextra != zfiles->extra)
      free((zvoid *)(zfiles->cextra));
    if (zfiles->ext && zfiles->extra)
      free((zvoid *)(zfiles->extra));
    if (zfiles->com && zfiles->comment)
      free((zvoid *)(zfiles->comment));
    farfree((zvoid far *)zfiles);
    zfiles = z;
    zcount--;
  }

  if (patterns != NULL) {
    while (pcount-- > 0) {
      if (patterns[pcount].zname != NULL)
        free((zvoid *)(patterns[pcount].zname));
    }
    free((zvoid *)patterns);
    patterns = NULL;
  }
}

local int finish(e)
int e;                  /* exit code */
/* Process -o and -m options (if specified), free up malloc'ed stuff, and
   exit with the code e. */
{
  int r;                /* return value from trash() */
  ulg t;                /* latest time in zip file */
  struct zlist far *z;  /* pointer into zfile list */

  /* If latest, set time to zip file to latest file in zip file */
  if (latest && zipfile && strcmp(zipfile, "-"))
  {
    diag("changing time of zip file to time of latest file in it");
    /* find latest time in zip file */
    if (zfiles == NULL)
       zipwarn("zip file is empty, can't make it as old as latest entry", "");
    else {
      t = 0;
      for (z = zfiles; z != NULL; z = z->nxt)
        /* Ignore directories in time comparisons */
#ifdef USE_EF_UT_TIME
        if (z->iname[z->nam-1] != (char)0x2f)   /* ascii '/' */
        {
          iztimes z_utim;
          ulg z_tim;

          z_tim = ((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
                   unix2dostime(&z_utim.mtime) : z->tim);
          if (t < z_tim)
            t = z_tim;
        }
#else /* !USE_EF_UT_TIME */
        if (z->iname[z->nam-1] != (char)0x2f    /* ascii '/' */
            && t < z->tim)
          t = z->tim;
#endif /* ?USE_EF_UT_TIME */
      /* set modified time of zip file to that time */
      if (t != 0)
        stamp(zipfile, t);
      else
        zipwarn(
         "zip file has only directories, can't make it as old as latest entry",
         "");
    }
  }
  if (tempath != NULL)
  {
    free((zvoid *)tempath);
    tempath = NULL;
  }
  if (zipfile != NULL)
  {
    free((zvoid *)zipfile);
    zipfile = NULL;
  }
  if (zcomment != NULL)
  {
    free((zvoid *)zcomment);
    zcomment = NULL;
  }


  /* If dispose, delete all files in the zfiles list that are marked */
  if (dispose)
  {
    diag("deleting files that were added to zip file");
    if ((r = trash()) != ZE_OK)
      ZIPERR(r, "was deleting moved files and directories");
  }


  /* Done! */
  freeup();
  return e;
}

void ziperr(c, h)
int c;                  /* error code from the ZE_ class */
char *h;                /* message about how it happened */
/* Issue a message for the error, clean up files and memory, and exit. */
{
#ifndef WINDLL
#ifndef MACOS
  static int error_level = 0;
#endif

  if (error_level++ > 0)
     EXIT(0);  /* avoid recursive ziperr() */
#endif /* !WINDLL */

  if (h != NULL) {
    if (PERR(c))
      perror("zip I/O error");
    fflush(mesg);
    fprintf(stderr, "\nzip error: %s (%s)\n", ziperrors[c-1], h);
  }
  if (tempzip != NULL)
  {
    if (tempzip != zipfile) {
      if (tempzf != NULL)
        fclose(tempzf);
#ifndef DEBUG
      destroy(tempzip);
#endif
      free((zvoid *)tempzip);
    } else {
      /* -g option, attempt to restore the old file */

      /* zip64 support 09/05/2003 R.Nausedat */
      zoff_t k = 0;                        /* keep count for end header */
      zoff_t cb = cenbeg;                  /* get start of central */

      struct zlist far *z;  /* steps through zfiles linked list */

      fprintf(stderr, "attempting to restore %s to its previous state\n",
         zipfile);

      zfseeko(tempzf, cenbeg, SEEK_SET);

      tempzn = cenbeg;
      for (z = zfiles; z != NULL; z = z->nxt)
      {
        putcentral(z, tempzf);
        tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
        k++;
      }
      putend(k, tempzn - cb, cb, zcomlen, zcomment, tempzf);
      fclose(tempzf);
      tempzf = NULL;
    }
  }
  if (key != NULL) {
    free((zvoid *)key);
    key = NULL;
  }
  if (tempath != NULL) {
    free((zvoid *)tempath);
    tempath = NULL;
  }
  if (zipfile != NULL) {
    free((zvoid *)zipfile);
    zipfile = NULL;
  }
  if (zcomment != NULL) {
    free((zvoid *)zcomment);
    zcomment = NULL;
  }
  freeup();
#ifndef WINDLL
  EXIT(c);
#else
  longjmp(zipdll_error_return, c);
#endif
}


void error(h)
  char *h;
/* Internal error, should never happen */
{
  ziperr(ZE_LOGIC, h);
}

#if (!defined(MACOS) && !defined(WINDLL))
local void handler(s)
int s;                  /* signal number (ignored) */
/* Upon getting a user interrupt, turn echo back on for tty and abort
   cleanly using ziperr(). */
{
#if defined(AMIGA) && defined(__SASC)
   _abort();
#else
#if !defined(MSDOS) && !defined(__human68k__) && !defined(RISCOS)
  echon();
  putc('\n', stderr);
#endif /* !MSDOS */
#endif /* AMIGA && __SASC */
  ziperr(ZE_ABORT, "aborting");
  s++;                                  /* keep some compilers happy */
}
#endif /* !MACOS && !WINDLL */

void zipwarn(a, b)
char *a, *b;            /* message strings juxtaposed in output */
/* Print a warning message to stderr and return. */
{
  if (noisy) fprintf(stderr, "\tzip warning: %s%s\n", a, b);
}

#ifndef WINDLL
local void license()
/* Print license information to stdout. */
{
  extent i;             /* counter for copyright array */

#if 0
  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++) {
    printf(copyright[i], "zip");
    putchar('\n');
  }
#endif
  for (i = 0; i < sizeof(swlicense)/sizeof(char *); i++)
    puts(swlicense[i]);
}

#ifdef VMSCLI
void help()
#else
local void help()
#endif
/* Print help (along with license info) to stdout. */
{
  extent i;             /* counter for help array */

  /* help array */
  static ZCONST char *text[] = {
"Zip %s (%s). Usage:",
#ifdef MACOS
"zip [-options] [-b fm] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
"  The default action is to add or replace zipfile entries from list.",
" ",
"  -f   freshen: only changed files  -u   update: only changed or new files",
"  -d   delete entries in zipfile    -m   move into zipfile (delete files)",
"  -r   recurse into directories     -j   junk (don't record) directory names",
"  -0   store only                   -l   convert LF to CR LF (-ll CR LF to LF)",
"  -1   compress faster              -9   compress better",
"  -q   quiet operation              -v   verbose operation/print version info",
"  -c   add one-line comments        -z   add zipfile comment",
"                                    -o   make zipfile as old as latest entry",
"  -F   fix zipfile (-FF try harder) -D   do not add directory entries",
"  -T   test zipfile integrity       -X   eXclude eXtra file attributes",
#  if CRYPT
"  -e   encrypt                      -n   don't compress these suffixes"
#  else
"  -h   show this help               -n   don't compress these suffixes"
#  endif
," -h2  show extended help",
"  Macintosh specific:",
"  -jj  record Fullpath (+ Volname)  -N store finder-comments as comments",
"  -df  zip only datafork of a file  -S include finder invisible/system files"
#else /* !MACOS */
#ifdef VM_CMS
"zip [-options] [-b fm] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
#else  /* !VM_CMS */
"zip [-options] [-b path] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
#endif /* ?VM_CMS */
"  The default action is to add or replace zipfile entries from list, which",
"  can include the special name - to compress standard input.",
"  If zipfile and list are omitted, zip compresses stdin to stdout.",
"  -f   freshen: only changed files  -u   update: only changed or new files",
"  -d   delete entries in zipfile    -m   move into zipfile (delete files)",
"  -r   recurse into directories     -j   junk (don't record) directory names",
#ifdef THEOS
"  -0   store only                   -l   convert CR to CR LF (-ll CR LF to CR)",
#else
"  -0   store only                   -l   convert LF to CR LF (-ll CR LF to LF)",
#endif
"  -1   compress faster              -9   compress better",
"  -q   quiet operation              -v   verbose operation/print version info",
"  -c   add one-line comments        -z   add zipfile comment",
"  -@   read names from stdin        -o   make zipfile as old as latest entry",
"  -x   exclude the following names  -i   include only the following names",
#ifdef EBCDIC
#ifdef CMS_MVS
"  -a   translate to ASCII           -B   force binary read (text is default)",
#else  /* !CMS_MVS */
"  -a   translate to ASCII",
#endif /* ?CMS_MVS */
#endif /* EBCDIC */
#ifdef TANDEM
"                                    -Bn  set Enscribe formatting options",
#endif
"  -F   fix zipfile (-FF try harder) -D   do not add directory entries",
"  -A   adjust self-extracting exe   -J   junk zipfile prefix (unzipsfx)",
"  -T   test zipfile integrity       -X   eXclude eXtra file attributes",
#ifdef VMS
"  -V   save VMS file attributes (-VV also save allocated blocks past EOF)",
"  -w   append version number to stored name",
#endif /* def VMS */
#ifdef WIN32
"  -!   use privileges (if granted) to obtain all aspects of WinNT security",
#endif /* WIN32 */
#ifdef OS2
"  -E   use the .LONGNAME Extended attribute (if found) as filename",
#endif /* OS2 */
#ifdef S_IFLNK
"  -y   store symbolic links as the link instead of the referenced file",
#endif /* !S_IFLNK */
"  -R   PKZIP recursion (see manual)",
#if defined(MSDOS) || defined(OS2)
"  -$   include volume label         -S   include system and hidden files",
#endif
#ifdef AMIGA
#  if CRYPT
"  -N   store filenotes as comments  -e   encrypt",
"  -h   show this help               -n   don't compress these suffixes"
#  else
"  -N   store filenotes as comments  -n   don't compress these suffixes"
#  endif
#else /* !AMIGA */
#  if CRYPT
"  -e   encrypt                      -n   don't compress these suffixes"
#  else
"  -h   show this help               -n   don't compress these suffixes"
#  endif
#endif /* ?AMIGA */
#ifdef RISCOS
,"  -h2  show extended help           -I   don't scan thru Image files"
#else
,"  -h2  show extended help"
#endif
#endif /* ?MACOS */
#ifdef VMS
,"  (Must quote upper-case options, like \"-V\", unless SET PROC/PARSE=EXTEND.)"
#endif /* def VMS */
,"  "
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zip");
    putchar('\n');
  }
  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i], VERSION, REVDATE);
    putchar('\n');
  }
}

#ifdef VMSCLI
void help_extended()
#else
local void help_extended()
#endif
/* Print extended help to stdout. */
{
  extent i;             /* counter for help array */

  /* help array */
  static ZCONST char *text[] = {
"",
"Extended Help for Zip",
"  The full command line syntax is:",
"",
"zip [-shortoptions ...] [--longoption ...] [zipfile path path ...] [-xi list]",
"",
"  Any number of short option and long option arguments are allowed",
"  (within limits) as well as any number of path arguments for files",
"  to zip up.  If zipfile is a zip archive the archive is read in.",
"",
"zip zipfile path path ... -- verbatimpath verbatimpath ...",
"  All arguments after -- are read verbatim as paths and not options.",
"",
"zip -x pattern pattern @ zipfile path path ...",
"  Patterns are paths with optional wildcards matching internal archive paths.",
"  Exclude and include lists end at next option, @, or end of line.",
"",
"zip -dd 1000 --temp-dir=path zipfile path1 path2 --exclude pattern pattern",
"  For short options that take values use either -ovalue or -o value",
"  For long option values use either --longoption=value or --longoption value.",
"",
"Recursion:",
"  -r path path ...         recurse paths",
"  -R pattern pattern ...   recurse current directory and match patterns",
"   use -i and -x with either to include or exclude paths",
"",
"Dots, counts:",
"  -db       display a running count of bytes processed and bytes to go",
"  -dc       display a running count of entries done and entries to go",
"  -dd size  display dots every size MB while processing files (0 no dots)",
"",
"More option highlights (see manual for additional options and details):",
"  -b dir    when creating or updating archive create temp archive in dir",
"             (could require additional file copy if on another device)",
"  -nw       no wildcards",
"  -sc       show command line arguments as processed and exit",
"  -so       show all available options on this system",
"  -t date   exclude before date (dates are mmddyyyy or yyyy-mm-dd)",
"  -tt date  exclude after and including date",
"  -W        wildcards don't span directory boundaries in paths",
""
  };

  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i]);
    putchar('\n');
  }
}

/*
 * XXX version_info() in a separate file
 */
local void version_info()
/* Print verbose info about program version and compile time options
   to stdout. */
{
  extent i;             /* counter in text arrays */
  char *envptr;

  /* Options info array */
  static ZCONST char *comp_opts[] = {
#ifdef ASM_CRC
    "ASM_CRC",
#endif
#ifdef ASMV
    "ASMV",
#endif
#ifdef DYN_ALLOC
    "DYN_ALLOC",
#endif
#ifdef MMAP
    "MMAP",
#endif
#ifdef BIG_MEM
    "BIG_MEM",
#endif
#ifdef MEDIUM_MEM
    "MEDIUM_MEM",
#endif
#ifdef SMALL_MEM
    "SMALL_MEM",
#endif
#ifdef DEBUG
    "DEBUG",
#endif
#ifdef USE_EF_UT_TIME
    "USE_EF_UT_TIME",
#endif
#ifdef NTSD_EAS
    "NTSD_EAS",
#endif
#ifdef VMS
#ifdef VMSCLI
    "VMSCLI",
#endif
#ifdef VMS_IM_EXTRA
    "VMS_IM_EXTRA",
#endif
#ifdef VMS_PK_EXTRA
    "VMS_PK_EXTRA",
#endif
#endif /* VMS */
#ifdef WILD_STOP_AT_DIR
    "WILD_STOP_AT_DIR",
#endif
#ifdef USE_ZLIB
    "USE_ZLIB",
#endif
#ifdef LARGE_FILE_SUPPORT
# ifdef USING_DEFAULT_LARGE_FILE_SUPPORT
    "LARGE_FILE_SUPPORT (default settings)",
# else
    "LARGE_FILE_SUPPORT",
# endif
#endif
#ifdef ZIP64_SUPPORT
    "ZIP64_SUPPORT",
#endif
#if CRYPT && defined(PASSWD_FROM_STDIN)
    "PASSWD_FROM_STDIN",
#endif /* CRYPT & PASSWD_FROM_STDIN */
    NULL
  };

  static ZCONST char *zipenv_names[] = {
#ifndef VMS
#  ifndef RISCOS
    "ZIP"
#  else /* RISCOS */
    "Zip$Options"
#  endif /* ?RISCOS */
#else /* VMS */
    "ZIP_OPTS"
#endif /* ?VMS */
    ,"ZIPOPT"
#ifdef AZTEC_C
    ,     /* extremely lame compiler bug workaround */
#endif
#ifndef __RSXNT__
# ifdef __EMX__
    ,"EMX"
    ,"EMXOPT"
# endif
# if (defined(__GO32__) && (!defined(__DJGPP__) || __DJGPP__ < 2))
    ,"GO32"
    ,"GO32TMP"
# endif
# if (defined(__DJGPP__) && __DJGPP__ >= 2)
    ,"TMPDIR"
# endif
#endif /* !__RSXNT__ */
#ifdef RISCOS
    ,"Zip$Exts"
#endif
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zip");
    putchar('\n');
  }

  for (i = 0; i < sizeof(versinfolines)/sizeof(char *); i++)
  {
    printf(versinfolines[i], "Zip", VERSION, REVDATE);
    putchar('\n');
  }

  version_local();

  puts("Zip special compilation options:");
#if WSIZE != 0x8000
  printf("\tWSIZE=%u\n", WSIZE);
#endif
  for (i = 0; (int)i < (int)(sizeof(comp_opts)/sizeof(char *) - 1); i++)
  {
    printf("\t%s\n",comp_opts[i]);
  }
#if CRYPT
  printf("\t[encryption, version %d.%d%s of %s]\n\n",
            CR_MAJORVER, CR_MINORVER, CR_BETA_VER, CR_VERSION_DATE);

  /* new notice - 2/27/2004 EG */
  printf("Encryption notice:\n");
  printf("\tThe encryption code of this program is not copyrighted and is\n");
  printf("\tput in the public domain. It was originally written in Europe\n");
  printf("\tand, to the best of our knowledge, can be freely distributed\n");
  printf("\tin both source and object forms from any country, including\n");
  printf("\tthe USA under License Exception TSU of the U.S. Export\n");
  printf("\tAdministration Regulations (section 740.13(e)) of 6 June 2002.\n");

  ++i;
#endif /* CRYPT */
  if (i == 0)
      puts("\t[none]");

  puts("\nZip environment options:");
  for (i = 0; i < sizeof(zipenv_names)/sizeof(char *); i++)
  {
    envptr = getenv(zipenv_names[i]);
    printf("%16s:  %s\n", zipenv_names[i],
           ((envptr == (char *)NULL || *envptr == 0) ? "[none]" : envptr));
  }
}
#endif /* !WINDLL */


#ifndef PROCNAME
#  define PROCNAME(n) procname(n, (action == DELETE || action == FRESHEN))
#endif /* PROCNAME */

#ifndef WINDLL
#ifndef MACOS
local void zipstdout()
/* setup for writing zip file on stdout */
{
  int r;
  mesg = stderr;
  if (isatty(1))
    ziperr(ZE_PARMS, "cannot write zip file to terminal");
  if ((zipfile = malloc(4)) == NULL)
    ziperr(ZE_MEM, "was processing arguments");
  strcpy(zipfile, "-");
  if ((r = readzipfile()) != ZE_OK)
    ziperr(r, zipfile);
}
#endif /* !MACOS */

local void check_zipfile(zipname, zippath)
  char *zipname;
  char *zippath;
  /* Invoke unzip -t on the given zip file */
{
#if (defined(MSDOS) && !defined(__GO32__)) || defined(__human68k__)
   int status, len;
   char *path, *p;
   char *zipnam;

   if ((zipnam = malloc(strlen(zipname) + 3)) == NULL)
     ziperr(ZE_MEM, "was creating unzip zipnam");

# ifdef MSDOS
   /* Add quotes for MSDOS.  8/11/04 EG */
   strcpy(zipnam, "\"");    /* accept spaces in name and path */
   strcat(zipnam, zipname);
   strcat(zipnam, "\"");
# else
   strcpy(zipnam, zipname);
# endif

   status = spawnlp(P_WAIT, "unzip", "unzip", verbose ? "-t" : "-tqq",
                    zipnam, NULL);
# ifdef __human68k__
   if (status == -1)
     perror("unzip");
# else
/*
 * unzip isn't in PATH range, assume an absolute path to zip in argv[0]
 * and hope that unzip is in the same directory.
 */
   if (status == -1) {
     p = MBSRCHR(zippath, '\\');
     path = MBSRCHR((p == NULL ? zippath : p), '/');
     if (path != NULL)
       p = path;
     if (p != NULL) {
       len = (int)(p - zippath) + 1;
       if ((path = malloc(len + sizeof("unzip.exe"))) == NULL)
         ziperr(ZE_MEM, "was creating unzip path");
       memcpy(path, zippath, len);
       strcpy(&path[len], "unzip.exe");
       status = spawnlp(P_WAIT, path, "unzip", verbose ? "-t" : "-tqq",
                        zipnam, NULL);
       free(path);
     }
     if (status == -1)
       perror("unzip");
   }
# endif /* ?__human68k__ */
   free(zipnam);
   if (status != 0) {
#else /* (MSDOS && !__GO32__) || __human68k__ */
   /* remove path limit and bug when exceeded - 11/08/04 EG */
   char *cmd;
   /*
   char cmd[FNMAX+16];
   */
   int result;

   if ((cmd = malloc(20 + strlen(zipname))) == NULL) {
     ziperr(ZE_MEM, "building command string for testing archive");
   }
   
   /* Tell picky compilers to shut up about unused variables */
   zippath = zippath;

   strcpy(cmd, "unzip -t ");
# ifdef QDOS
   strcat(cmd, "-Q4 ");
# endif
   if (!verbose) strcat(cmd, "-qq ");
   /*
   if ((int)strlen(zipname) > FNMAX) {
     error("zip filename too long");
   }
   */
# ifdef UNIX
   strcat(cmd, "'");    /* accept space or $ in name */
   strcat(cmd, zipname);
   strcat(cmd, "'");
# else
   strcat(cmd, zipname);
# endif
# ifdef VMS
   result = !system(cmd);
# else
   result = system(cmd);
# endif
  free(cmd);
  if (result) { 
#endif /* ?((MSDOS && !__GO32__) || __human68k__) */
     fprintf(mesg, "test of %s FAILED\n", zipfile);
     ziperr(ZE_TEST, "original files unmodified");
   }
   if (noisy)
     fprintf(mesg, "test of %s OK\n", zipfile);
}
#endif /* !WINDLL */

/* get_filters() is replaced by the following
local int get_filters(argc, argv)
*/

/* The filter patterns for options -x, -i, and -R are
   returned by get_option() one at a time, so use a linked
   list to store until all args are processed.  Then convert
   to array for processing.
 */

/* add a filter to the linked list */
local int add_filter(flag, pattern)
  int flag;
  char *pattern;
{
  char *iname, *p = NULL;
  FILE *fp;
  struct filterlist_struct *filter = NULL;

  /* should never happen */
  if (flag != 'R' && flag != 'x' && flag != 'i') {
    ZIPERR(ZE_LOGIC, "bad flag to add_filter");
  }
  if (pattern == NULL) {
    ZIPERR(ZE_LOGIC, "null pattern to add_filter");
  }

  if (pattern[0] == '@') {
    /* read file with 1 pattern per line */
    if (pattern[1] == '\0') {
      ZIPERR(ZE_PARMS, "missing file after @");
    }
    fp = fopen(pattern + 1, "r");
    if (fp == NULL) {
      sprintf(errbuf, "%c pattern file '%s'", flag, pattern);
      ZIPERR(ZE_OPEN, errbuf);
    }
    while ((p = getnam(fp)) != NULL) {
      if ((filter = (struct filterlist_struct *) malloc(sizeof(struct filterlist_struct))) == NULL) {
        ZIPERR(ZE_MEM, "adding filter");
      }
      if (filterlist == NULL) {
        /* first filter */
        filterlist = filter;         /* start of list */
        lastfilter = filter;
      } else {
        lastfilter->next = filter;   /* link to last filter in list */
        lastfilter = filter;
      }
      iname = ex2in(p, 0, (int *)NULL);
      free(p);
      if (iname != NULL) {
        lastfilter->pattern = in2ex(iname);
        free(iname);
      } else {
        lastfilter->pattern = NULL;
      }
      lastfilter->flag = flag;
      if (flag != 'x')
        icount++;
      pcount++;
      lastfilter->next = NULL;
    }
    fclose(fp);
  } else {
    /* single pattern */
    if ((filter = (struct filterlist_struct *) malloc(sizeof(struct filterlist_struct))) == NULL) {
      ZIPERR(ZE_MEM, "adding filter");
    }
    if (filterlist == NULL) {
      /* first pattern */
      filterlist = filter;         /* start of list */
      lastfilter = filter;
    } else {
      lastfilter->next = filter;   /* link to last filter in list */
      lastfilter = filter;
    }
    iname = ex2in(pattern, 0, (int *)NULL);
    if (iname != NULL) {
       lastfilter->pattern = in2ex(iname);
       free(iname);
    } else {
      lastfilter->pattern = NULL;
    }
    lastfilter->flag = flag;
    if (flag != 'x')
      icount++;
    pcount++;
    lastfilter->next = NULL;
  }

  return pcount;
}

/* convert list to patterns array */
local int filterlist_to_patterns()
{
  unsigned i;
  struct filterlist_struct *next = NULL;

  if (pcount == 0) {
    patterns = NULL;
    return 0;
  }
  if ((patterns = (struct plist *) malloc((pcount + 1) * sizeof(struct plist))) == NULL) {
    ZIPERR(ZE_MEM, "was creating pattern list");
  }
  
  for (i = 0; i < pcount && filterlist != NULL; i++) {
    patterns[i].select = filterlist->flag;
    patterns[i].zname = filterlist->pattern;
    next = filterlist->next;
    free(filterlist);
    filterlist = next;
  }
  
  return pcount;
}

/* Running Stats
   10/30/04 EG */

int DisplayRunningStats()
{
  char tempstrg[100];

  if (display_counts) {
    fprintf(mesg, "%3d/%3d ", files_so_far, files_total - files_so_far);
  }
  if (display_bytes) {
    WriteNumString(bytes_so_far, tempstrg);
    fprintf(mesg, "[%4s/", tempstrg);
    WriteNumString(bytes_total - bytes_so_far, tempstrg);
    fprintf(mesg, "%4s] ", tempstrg);
  }

  return 0;
}

#if CRYPT
#ifndef WINDLL
int encr_passwd(modeflag, pwbuf, size, zfn)
int modeflag;
char *pwbuf;
int size;
ZCONST char *zfn;
{
    char *prompt;

    /* Tell picky compilers to shut up about unused variables */
    zfn = zfn;

    prompt = (modeflag == ZP_PW_VERIFY) ?
              "Verify password: " : "Enter password: ";

    if (getp(prompt, pwbuf, size) == NULL) {
      ziperr(ZE_PARMS, "stderr is not a tty");
    }
    return IZ_PW_ENTERED;
}
#endif /* !WINDLL */
#else /* !CRYPT */
int encr_passwd(modeflag, pwbuf, size, zfn)
int modeflag;
char *pwbuf;
int size;
ZCONST char *zfn;
{
    /* Tell picky compilers to shut up about unused variables */
    modeflag = modeflag; pwbuf = pwbuf; size = size; zfn = zfn;

    return ZE_LOGIC;    /* This function should never be called! */
}
#endif /* CRYPT */

/*
  -------------------------------------------------------
  Command Line Options
  -------------------------------------------------------

  Valid command line options.

  The function get_option() uses this table to check if an
  option is valid and if it takes a value (also called an
  option argument).  To add an option to zip just add it
  to this table and add a case in the main switch to handle
  it.  If either shortopt or longopt not used set to "".

   The fields:
       shortopt     - short option name (1 or 2 chars)
       longopt      - long option name
       value_type   - see zip.h for constants
       negatable    - option is negatable with trailing -
       ID           - unsigned long int returned for option
       name         - short description of option returned
                        on some errors, can be NULL
*/

/* Most option IDs are set to the shortopt char.  For
   multichar short options set to arbitrary unused constant. */
#define o_db            0x101
#define o_dc            0x102
#define o_df            0x103
#define o_dots          0x104
#define o_FF            0x105
#define o_h2            0x106
#define o_jj            0x107
#define o_ll            0x108
#define o_nw            0x109
#define o_sc            0x110
#define o_sd            0x111
#define o_so            0x112
#define o_sp            0x113
#define o_tt            0x114
#define o_VV            0x115
#define o_z64           0x116

/* the below is mainly from the main command line
   switch with few changes */
struct option_struct options[] = {
  /* short longopt        value_type        negatable        ID    name */
#ifdef EBCDIC
    {"a",  "ascii",       o_NO_VALUE,       o_NOT_NEGATABLE, 'a',  "to ascii"},
#endif /* EBCDIC */
#ifdef CMS_MVS
    {"B",  "binary",      o_NO_VALUE,       o_NOT_NEGATABLE, 'B',  "binary"},
#endif /* CMS_MVS */
#ifdef TANDEM
    {"B",  "",            o_NUMBER_VALUE,   o_NOT_NEGATABLE, 'B',  "nsk"},
#endif
    {"0",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '0',  "store"},
    {"1",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '1',  "compress 1"},
    {"2",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '2',  "compress 2"},
    {"3",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '3',  "compress 3"},
    {"4",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '4',  "compress 4"},
    {"5",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '5',  "compress 5"},
    {"6",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '6',  "compress 6"},
    {"7",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '7',  "compress 7"},
    {"8",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '8',  "compress 8"},
    {"9",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '9',  "compress 9"},
    {"A",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'A',  "adjust sfx"},
    {"b",  "temp-path",   o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'b',  "temp dir"},
    {"c",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'c',  "comments"},
    {"d",  "delete",      o_NO_VALUE,       o_NOT_NEGATABLE, 'd',  "delete"},
    {"db", "display-bytes", o_NO_VALUE,     o_NOT_NEGATABLE, o_db, "display running bytes"},
    {"dc", "display-counts", o_NO_VALUE,    o_NOT_NEGATABLE, o_dc, "display running file count"},
    {"dd", "dotsize",     o_OPTIONAL_VALUE, o_NOT_NEGATABLE, o_dots,"display dot each size MBs"},
#ifdef MACOS
    {"df", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_df, "datafork"},
#endif /* MACOS */
    {"D",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'D',  "no dir entries"},
    {"e",  "encrypt",     o_NO_VALUE,       o_NOT_NEGATABLE, 'e',  "encrypt"},
#ifdef OS2
    {"E",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'E',  "OS2 longnames"},
#endif
    {"F",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'F',  "fix"},
    {"FF", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_FF, "fixfix"},
    {"f",  "freshen",     o_NO_VALUE,       o_NOT_NEGATABLE, 'f',  "freshen"},
    {"g",  "grow",        o_NO_VALUE,       o_NOT_NEGATABLE, 'g',  "grow"},
#ifndef WINDLL
    {"h",  "help",        o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"H",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"?",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"h2", "more-help",   o_NO_VALUE,       o_NOT_NEGATABLE, o_h2, "more help"},
#endif /* !WINDLL */
    {"i",  "include",     o_VALUE_LIST,     o_NOT_NEGATABLE, 'i',  "include"},
#ifdef RISCOS
    {"I",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'I',  "no image"},
#endif
    {"j",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'j',  "junk paths"},
#ifdef MACOS
    {"jj", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_jj, "MAC abs path"},
#endif /* ?MACOS */
    {"J",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'J',  "junk sfx"},
    {"k",  "DOS-names",   o_NO_VALUE,       o_NOT_NEGATABLE, 'k',  "DOS names"},
    {"l",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'l',  "LF->CRLF"},
    {"ll", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_ll, "CRLF->LF"},
#ifndef WINDLL
    {"L",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'L',  "licence"},
#endif
    {"m",  "move",        o_NO_VALUE,       o_NOT_NEGATABLE, 'm',  "move"},
    {"n",  "suffixes",    o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'n',  "suffixes"},
    {"nw", "no-wild",     o_NO_VALUE,       o_NOT_NEGATABLE, o_nw, "no wildcards during add or update"},
#if defined(AMIGA) || defined(MACOS)
    {"N",  "notes",       o_NO_VALUE,       o_NOT_NEGATABLE, 'N',  "notes"},
#endif
    {"o",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'o',  "use latest time"},
    {"p",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'p',  "store paths"},
    {"P",  "password",    o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'P',  "encryption password"},
#if defined(QDOS) || defined(QLZIP)
    {"Q",  "",            o_NUMBER_VALUE,   o_NOT_NEGATABLE, 'Q',  "Q flag"},
#endif
    {"q",  "quiet",       o_NO_VALUE,       o_NOT_NEGATABLE, 'q',  "quiet"},
    {"r",  "recurse-paths", o_NO_VALUE,     o_NOT_NEGATABLE, 'r',  "recurse -r (paths)"},
    {"R",  "recurse-patterns", o_NO_VALUE,  o_NOT_NEGATABLE, 'R',  "recurse -R (patterns)"},
    {"s",  "split-size",  o_REQUIRED_VALUE, o_NOT_NEGATABLE, 's',  "enable splitting and set split size"},
    {"sc", "show-command",o_NO_VALUE,       o_NOT_NEGATABLE, o_sc, "show command line"},
    {"sd", "show-debug",  o_NO_VALUE,       o_NOT_NEGATABLE, o_sd, "show debug"},
    {"so", "show-options",o_NO_VALUE,       o_NOT_NEGATABLE, o_so, "show options"},
    {"sp", "split-pause", o_NO_VALUE,       o_NOT_NEGATABLE, o_sp, "pause and select split destination"},
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(ATARI)
    {"S",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'S',  "include system and hidden"},
#endif /* MSDOS || OS2 || WIN32 || ATARI */
    {"t",  "",            o_REQUIRED_VALUE, o_NOT_NEGATABLE, 't',  "exclude before"},
    {"tt", "",            o_REQUIRED_VALUE, o_NOT_NEGATABLE, o_tt, "include before"},
    {"T",  "test",        o_NO_VALUE,       o_NOT_NEGATABLE, 'T',  "test"},
    {"u",  "update",      o_NO_VALUE,       o_NOT_NEGATABLE, 'u',  "update"},
    {"v",  "verbose",     o_NO_VALUE,       o_NOT_NEGATABLE, 'v',  "verbose"},
#ifdef VMS
    {"V",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'V',  "Store VMS attributes"},
    {"VV", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_VV, "VMS specific format"},
    {"w",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'w',  "VMS version"},
#endif /* VMS */
    {"W",  "wild-respects-dirs", o_NO_VALUE,o_NOT_NEGATABLE, 'W',  "wild respects dirs"},
    {"x",  "exclude",     o_VALUE_LIST,     o_NOT_NEGATABLE, 'x',  "exclude"},
    {"X",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'X',  "no extra"},
#ifdef S_IFLNK
    {"y",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'y',  "store symbolic links"},
#endif /* S_IFLNK */
    {"z",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'z',  "file comment"},
#if defined(MSDOS) || defined(OS2)
    {"$",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '$',  "volume label"},
#endif
#ifndef MACOS
    {"@",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '@',  "names from stdin"},
#endif /* !MACOS */
#ifdef WIN32
    {"!",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, '!',  "use privileges"},
#endif
#ifdef RISCOS
    {"/",  "",            o_REQUIRED_VALUE, o_NOT_NEGATABLE, '/',  "override Zip$Exts"},
#endif
#ifdef ZIP64_SUPPORT
    {"",  "force-zip64",  o_NO_VALUE,       o_NOT_NEGATABLE, o_z64,"force Zip64"},
#endif
    /* the end of the list */
    {NULL, NULL,          o_NO_VALUE,       o_NOT_NEGATABLE, 0,    NULL} /* end has option_ID = 0 */
  };



#ifndef USE_ZIPMAIN
int main(argc, argv)
#else
int zipmain(argc, argv)
#endif
int argc;               /* number of tokens in command line */
char **argv;            /* command line tokens */
/* Add, update, freshen, or delete zip entries in a zip file.  See the
   command help in help() above. */
{
  int a;                /* attributes of zip file */
  int d;                /* true if just adding to a zip file */
  char *e;              /* malloc'd comment buffer */
  struct flist far *f;  /* steps through found linked list */
  int i;                /* arg counter, root directory flag */

  /* zip64 support 09/05/2003 R.Nausedat */
  zoff_t c;             /* start of central directory */
  zoff_t t;             /* length of central directory */
  zoff_t k;             /* next argument type, marked counter,
                           comment size, entry count */
  zoff_t n;             /* total of entry len's */

  int o;                /* true if there were any ZE_OPEN errors */
  char *p;              /* steps through option arguments */
  char *pp;             /* temporary pointer */
  ulg *cmptime = NULL;  /* pointer to 'before' or 'after' */
  int r;                /* temporary variable */
  int s;                /* flag to read names from stdin */
  zoff_t usize;         /* uncompressed file size 10/30/04 EG */
  ulg tf;               /* file time */
  int first_listarg = 0;/* index of first arg of "process these files" list */
  struct zlist far *v;  /* temporary variable */
  struct zlist far * far *w;    /* pointer to last link in zfiles list */
  FILE *x, *y;          /* input and output zip files */
  struct zlist far *z;  /* steps through zfiles linked list */
#ifdef WINDLL
  int retcode;          /* return code for dll */
#endif /* WINDLL */
#if (!defined(VMS) && !defined(CMS_MVS))
  char *zipbuf;         /* stdio buffer for the zip file */
#endif /* !VMS && !CMS_MVS */
  FILE *comment_stream; /* set to stderr if anything is read from stdin */

/* used by get_option */
  unsigned long option; /* option ID returned by get_option */
  int argcnt = 0;       /* current argcnt in args */
  int argnum = 0;       /* arg number */
  int optchar = 0;      /* option state */
  char *value = NULL;   /* non-option arg, option value or NULL */
  int negated = 0;      /* 1 = option negated */
  int fna = 0;          /* current first non-opt arg */
  int optnum = 0;       /* index in table */

  int show_options = 0; /* show options */
  int show_what_doing = 0; /* show what doing */
  int show_args = 0;    /* show command line */
  int seen_doubledash = 0; /* seen -- argument */


#ifdef THEOS
  /* the argument expansion from the standard library is full of bugs */
  /* use mine instead */
  _setargv(&argc, &argv);
  setlocale(LC_CTYPE,"I");
#else
  SETLOCALE(LC_CTYPE,"");
#endif

#if defined(__IBMC__) && defined(__DEBUG_ALLOC__)
  {
    extern void DebugMalloc(void);
    atexit(DebugMalloc);
  }
#endif

#ifdef QDOS
  {
    extern void QDOSexit(void);
    atexit(QDOSexit);
  }
#endif

#ifdef NLM
  {
    extern void NLMexit(void);
    atexit(NLMexit);
  }
#endif

#ifdef RISCOS
  set_prefix();
#endif

#ifdef __human68k__
  fflush(stderr);
  setbuf(stderr, NULL);
#endif

/* Re-initialize global variables to make the zip dll re-entrant. It is
 * possible that we could get away with not re-initializing all of these
 * but better safe than sorry.
 */
#if defined(MACOS) || defined(WINDLL)
  action = ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
  comadd = 0;   /* 1=add comments for new files */
  zipedit = 0;  /* 1=edit zip comment and all file comments */
  latest = 0;   /* 1=set zip file time to time of latest file */
  before = 0;   /* 0=ignore, else exclude files before this time */
  after = 0;    /* 0=ignore, else exclude files newer than this time */
  test = 0;     /* 1=test zip file with unzip -t */
  tempdir = 0;  /* 1=use temp directory (-b) */
  junk_sfx = 0; /* 1=junk the sfx prefix */
#if defined(AMIGA) || defined(MACOS)
  filenotes = 0;/* 1=take comments from AmigaDOS/MACOS filenotes */
#endif
  zipstate = -1;
  tempzip = NULL;
  fcount = 0;
  recurse = 0;         /* 1=recurse into directories; 2=match filenames */
  dispose = 0;         /* 1=remove files after put in zip file */
  pathput = 1;         /* 1=store path with name */
  method = BEST;       /* one of BEST, DEFLATE (only), or STORE (only) */
  dosify = 0;          /* 1=make new entries look like MSDOS */
  verbose = 0;         /* 1=report oddities in zip file structure */
  fix = 0;             /* 1=fix the zip file */
  adjust = 0;          /* 1=adjust offsets for sfx'd file (keep preamble) */
  level = 6;           /* 0=fastest compression, 9=best compression */
  translate_eol = 0;   /* Translate end-of-line LF -> CR LF */
#ifdef WIN32
  use_longname_ea = 0; /* 1=use the .LONGNAME EA as the file's name */
  use_privileges = 0;  /* 1=use security privileges overrides */
#endif
  hidden_files = 0;    /* process hidden and system files */
  volume_label = 0;    /* add volume label */
  dirnames = 1;        /* include directory entries by default */
  linkput = 0;         /* 1=store symbolic links as such */
  noisy = 1;           /* 0=quiet operation */
  extra_fields = 1;    /* 0=do not create extra fields */
  special = ".Z:.zip:.zoo:.arc:.lzh:.arj"; /* List of special suffixes */
  key = NULL;          /* Scramble password if scrambling */
  tempath = NULL;      /* Path for temporary files */
  found = NULL;        /* where in found, or new found entry */
  fnxt = &found;
  patterns = NULL;     /* List of patterns to be matched */
  pcount = 0;          /* number of patterns */
  icount = 0;          /* number of include only patterns */
  /* status 10/30/04 EG */
  files_so_far = 0;    /* count of files processed so far */
  files_total = 0;     /* files to get through */
  bytes_so_far = 0;    /* bytes processed so far */
  bytes_total = 0;     /* bytes to get through */

  /* used by get_option */
  argcnt = 0;          /* size of args */
  argnum = 0;          /* current arg number */
  optchar = 0;         /* option state */
  value = NULL;        /* non-option arg, option value or NULL */
  negated = 0;         /* 1 = option negated */
  fna = 0;             /* current first nonopt arg */
  optnum = 0;          /* option index */

  show_options = 0;    /* 1 = show options */
  show_what_doing = 0; /* 1 = show what zip doing */
  show_args = 0;       /* 1 = show command line */
  seen_doubledash = 0; /* seen -- argument */


#ifndef MACOS
  retcode = setjmp(zipdll_error_return);
  if (retcode) {
    return retcode;
  }
#endif /* !MACOS */
#endif /* MACOS || WINDLL */

  mesg = (FILE *) stdout; /* cannot be made at link time for VMS */
  comment_stream = (FILE *)stdin;

  init_upper();           /* build case map table */

#ifdef LARGE_FILE_SUPPORT
  /* test if we can support large files - 9/29/04 EG */
  if (sizeof(zoff_t) < 8) {
    ZIPERR(ZE_COMPERR, "LARGE_FILE_SUPPORT enabled but OS not supporting it");
  }
#endif

#if (defined(WIN32) && defined(USE_EF_UT_TIME))
  /* For the Win32 environment, we may have to "prepare" the environment
     prior to the tzset() call, to work around tzset() implementation bugs.
   */
  iz_w32_prepareTZenv();
#endif

#if (defined(IZ_CHECK_TZ) && defined(USE_EF_UT_TIME))
#  ifndef VALID_TIMEZONE
#     define VALID_TIMEZONE(tmp) \
             (((tmp = getenv("TZ")) != NULL) && (*tmp != '\0'))
#  endif
  zp_tz_is_valid = VALID_TIMEZONE(p);
#if (defined(AMIGA) || defined(DOS))
  if (!zp_tz_is_valid)
    extra_fields = 0;     /* disable storing "UT" time stamps */
#endif /* AMIGA || DOS */
#endif /* IZ_CHECK_TZ && USE_EF_UT_TIME */

/* For systems that do not have tzset() but supply this function using another
   name (_tzset() or something similar), an appropiate "#define tzset ..."
   should be added to the system specifc configuration section.  */
#if (!defined(TOPS20) && !defined(VMS))
#if (!defined(RISCOS) && !defined(MACOS) && !defined(QDOS))
#if (!defined(BSD) && !defined(MTS) && !defined(CMS_MVS) && !defined(TANDEM))
  tzset();
#endif
#endif
#endif

#ifdef VMSCLI
    {
        ulg status = vms_zip_cmdline(&argc, &argv);
        if (!(status & 1))
            return status;
    }
#endif /* VMSCLI */

  /* extract extended argument list from environment */
  expand_args(&argc, &argv);


#ifndef WINDLL
  /* Process arguments */
  diag("processing arguments");
  /* First, check if just the help or version screen should be displayed */
  if (argc == 1 && isatty(1))   /* no arguments, and output screen available */
  {                             /* show help screen */
#ifdef VMSCLI
    VMSCLI_help();
#else
    help();
#endif
    EXIT(0);
  }
  else if (argc == 2 && strcmp(argv[1], "-v") == 0 &&
           /* only "-v" as argument, and */
           (isatty(1) || isatty(0)))
           /* stdout or stdin is connected to console device */
  {                             /* show diagnostic version info */
    version_info();
    EXIT(0);
  }
#ifndef VMS
#  ifndef RISCOS
  envargs(&argc, &argv, "ZIPOPT", "ZIP");  /* get options from environment */
#  else /* RISCOS */
  envargs(&argc, &argv, "ZIPOPT", "Zip$Options");  /* get options from environment */
  getRISCOSexts("Zip$Exts");        /* get the extensions to swap from environment */
#  endif /* ? RISCOS */
#else /* VMS */
  envargs(&argc, &argv, "ZIPOPT", "ZIP_OPTS");  /* 4th arg for unzip compat. */
#endif /* ?VMS */
#endif /* !WINDLL */

  zipfile = tempzip = NULL;
  tempzf = NULL;
  d = 0;                        /* disallow adding to a zip file */
#if (!defined(MACOS) && !defined(WINDLL) && !defined(NLM))
  signal(SIGINT, handler);
#ifdef SIGTERM                  /* AMIGADOS and others have no SIGTERM */
  signal(SIGTERM, handler);
#endif
#endif /* !MACOS && !WINDLL && !NLM */
#ifdef NLM
  NLMsignals();
#endif
  k = 0;                        /* Next non-option argument type */
  s = 0;                        /* set by -@ */

  /*
  -------------------------------------------
  Process command line using get_option
  -------------------------------------------

  Each call to get_option() returns either a command
  line option and possible value or a non-option argument.
  Arguments are permuted so that all options (-r, -b temp)
  are returned before non-option arguments (zipfile).
  Returns 0 when nothing left to read.
  */
    
  /* set argnum = 0 on first call to init get_option */
  argnum = 0;

  /* get_option returns the option ID and updates parameters:
         argv    - same argv if no argument file support
         argcnt  - current argc for argv
         value   - char* to value (free() when done with it) or NULL if no value
         negated - option was negated with trailing -
  */

  while ((option = get_option(&argv, &argcnt, &argnum,
                              &optchar, &value, &negated,
                              &fna, &optnum, 0)))
  {
    switch (option)
    {
#ifdef EBCDIC
      case 'a':
        aflag = ASCII;
        printf("Translating to ASCII...\n");
        break;
#endif /* EBCDIC */
#ifdef CMS_MVS
        case 'B':
          bflag = 1;
          printf("Using binary mode...\n");
          break;
#endif /* CMS_MVS */
#ifdef TANDEM
        case 'B':
          nskformatopt(value);
          free(value);
          break;
#endif

        case '0':
          method = STORE; level = 0; break;
        case '1':  case '2':  case '3':  case '4':
        case '5':  case '6':  case '7':  case '8':  case '9':
          /* Set the compression efficacy */
          level = option - '0';  break;
        case 'A':   /* Adjust unzipsfx'd zipfile:  adjust offsets only */
          adjust = 1; break;
        case 'b':   /* Specify path for temporary file */
          tempdir = 1;
          tempath = value;
          break;
        case 'c':   /* Add comments for new files in zip file */
          comadd = 1;  break;
        case 'd':   /* Delete files from zip file */
          if (action != ADD) {
            ZIPERR(ZE_PARMS, "specify just one action");
          }
          action = DELETE;
          break;
#ifdef MACOS
        case o_df:
          MacZip.DataForkOnly = true;
          break;
#endif /* MACOS */
        case o_db:
          display_bytes = 1;
          break;
        case o_dc:
          display_counts = 1;
          break;
        case o_dots:
          /* dot_size = 1 is about 1 MB, 2 about 2 MB */
          if (value == NULL)
            dot_size = 10;
          else {
            if (isdigit(value[0])) {
              dot_size = atoi(value);
            } else {
              sprintf(errbuf, "option -dd (--dotsize) has bad size '%s'", value);
              ZIPERR(ZE_PARMS, errbuf);
            }
          }
          /* result is about 1 dot per dot_size MB input read */
          dot_size = dot_size * 32;
          dot_count = -1;
          free(value);
          break;
        case 'D':   /* Do not add directory entries */
          dirnames = 0; break;
        case 'e':   /* Encrypt */
#if !CRYPT
          ZIPERR(ZE_PARMS, "encryption not supported");
#else /* CRYPT */
          if (key == NULL) {
            if ((key = malloc(IZ_PWLEN+1)) == NULL) {
              ZIPERR(ZE_MEM, "was getting encryption password");
            }
            r = encr_passwd(ZP_PW_ENTER, key, IZ_PWLEN+1, zipfile);
            if (r != IZ_PW_ENTERED) {
              if (r < IZ_PW_ENTERED)
                r = ZE_PARMS;
              ZIPERR(r, "was getting encryption password");
            }
            if (*key == '\0') {
              ZIPERR(ZE_PARMS, "zero length password not allowed");
            }
            if ((e = malloc(IZ_PWLEN+1)) == NULL) {
              ZIPERR(ZE_MEM, "was verifying encryption password");
            }
            r = encr_passwd(ZP_PW_VERIFY, e, IZ_PWLEN+1, zipfile);
            if (r != IZ_PW_ENTERED && r != IZ_PW_SKIPVERIFY) {
              free((zvoid *)e);
              if (r < ZE_OK) r = ZE_PARMS;
              ZIPERR(r, "was verifying encryption password");
            }
            r = ((r == IZ_PW_SKIPVERIFY) ? 0 : strcmp(key, e));
            free((zvoid *)e);
            if (r) {
              ZIPERR(ZE_PARMS, "password verification failed");
            }
          }
#endif /* !CRYPT */
          break;
        case 'F':   /* fix the zip file */
          fix = 1; break;
        case o_FF:  /* try harder to fix file */
          fix = 2; break;
        case 'f':   /* Freshen zip file--overwrite only */
          if (action != ADD) {
            ZIPERR(ZE_PARMS, "specify just one action");
          }
          action = FRESHEN;
          break;
        case 'g':   /* Allow appending to a zip file */
          d = 1;  break;
#ifndef WINDLL
        case 'h': case 'H': case '?':  /* Help */
#ifdef VMSCLI
          VMSCLI_help();
#else
          help();
#endif
          RETURN(finish(ZE_OK));
#endif /* !WINDLL */

#ifndef WINDLL
        case o_h2:  /* Extended Help */
          help_extended();
          RETURN(finish(ZE_OK));
#endif /* !WINDLL */

#ifdef RISCOS
        case 'I':   /* Don't scan through Image files */
          scanimage = 0;
          break;
#endif
#ifdef MACOS
        case o_jj:   /* store absolute path including volname */
            MacZip.StoreFullPath = true;
            break;
#endif /* ?MACOS */
        case 'j':   /* Junk directory names */
          pathput = 0;  break;
        case 'J':   /* Junk sfx prefix */
          junk_sfx = 1;  break;
        case 'k':   /* Make entries using DOS names (k for Katz) */
          dosify = 1;  break;
        case 'l':   /* Translate end-of-line */
          translate_eol = 1; break;
        case o_ll:
          translate_eol = 2; break;
#ifndef WINDLL
        case 'L':   /* Show license */
          license();
          RETURN(finish(ZE_OK));
#endif
        case 'm':   /* Delete files added or updated in zip file */
          dispose = 1;  break;
        case 'n':   /* Don't compress files with a special suffix */
          special = value;
          /* special = NULL; */ /* will be set at next argument */
          break;
        case o_nw:  /* no wildcards - wildcards are handled like other characters */
          no_wild = 1;
          break;
#if defined(AMIGA) || defined(MACOS)
        case 'N':   /* Get zipfile comments from AmigaDOS/MACOS filenotes */
          filenotes = 1; break;
#endif
        case 'o':   /* Set zip file time to time of latest file in it */
          latest = 1;  break;
        case 'p':   /* Store path with name */
          break;            /* (do nothing as annoyance avoidance) */
        case 'P':   /* password for encryption */
          if (key != NULL) {
            ZIPERR(ZE_PARMS, "can only have one -P");
          }
#if CRYPT
          key = value;
#else
          ZIPERR(ZE_PARMS, "encryption not supported");
#endif /* CRYPT */
          break;
#if defined(QDOS) || defined(QLZIP)
        case 'Q':
          qlflag  = strtol(value, NULL, 10);
       /* qlflag  = strtol((p+1), &p, 10); */
       /* p--; */
          if (qlflag == 0) qlflag = 4;
          free(value);
          break;
#endif
        case 'q':   /* Quiet operation */
          noisy = 0;
#ifdef MACOS
          MacZip.MacZip_Noisy = false;
#endif  /* MACOS */
          if (verbose) verbose--;
          break;
        case 'r':   /* Recurse into subdirectories, match full path */
          if (recurse == 2) {
            ZIPERR(ZE_PARMS, "do not specify both -r and -R");
          }
          recurse = 1;  break;
        case 'R':   /* Recurse into subdirectories, match filename */
          if (recurse == 1) {
            ZIPERR(ZE_PARMS, "do not specify both -r and -R");
          }
          recurse = 2;  break;
        case o_sc:  /* show command line args */
          show_args = 1; break;
        case o_sd:  /* show debugging */
          show_what_doing = 1; break;
        case o_so:  /* show all options */
          show_options = 1; break;
        case 's':   /* enable split archives */

          ZIPERR(ZE_PARMS, "-s not yet supported");

          /* get the split size from value */
          
          free(value);
          break;
        case o_sp:  /* enable split select - pause splitting between splits */

          ZIPERR(ZE_PARMS, "-sp not yet supported");

          break;
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(ATARI)
        case 'S':
          hidden_files = 1; break;
#endif /* MSDOS || OS2 || WIN32 || ATARI */
#ifdef MACOS
        case 'S':
          MacZip.IncludeInvisible = true; break;
#endif /* MACOS */
        case 't':   /* Exclude files earlier than specified date */
        case o_tt:  /* Exclude files after specified date */
          if (option == o_tt) {
            cmptime = &after;
          } else {
            cmptime = &before;
          }
          if (*cmptime) {
            ZIPERR(ZE_PARMS, (cmptime == &after ?
                   "can only have one -tt" : "can only have one -t"));
          }
          {
            int yyyy, mm, dd;       /* results of sscanf() */

            /* Support ISO 8601 & American dates */
            if ((sscanf(value, "%4d-%2d-%2d", &yyyy, &mm, &dd) != 3 &&
                 sscanf(value, "%2d%2d%4d", &mm, &dd, &yyyy) != 3) ||
                mm < 1 || mm > 12 || dd < 1 || dd > 31) {
              ZIPERR(ZE_PARMS, (cmptime == &after ?
                     "invalid date entered for -tt option - use mmddyyyy or yyyy-mm-dd" :
                     "invalid date entered for -t option - use mmddyyyy or yyyy-mm-dd"));
            }
            *cmptime = dostime(yyyy, mm, dd, 0, 0, 0);
          }
          free(value);
          break;
        case 'T':   /* test zip file */
          test = 1; break;
        case 'u':   /* Update zip file--overwrite only if newer */
          if (action != ADD) {
            ZIPERR(ZE_PARMS, "specify just one action");
          }
          action = UPDATE;
          break;
        case 'v':   /* Mention oddities in zip file structure */
          noisy = 1;
          verbose++;
          break;
#ifdef VMS
        case 'V':   /* Store in VMS format.  (Record multiples.) */
          vms_native = 1; break;
          /* below does work with new parser but doesn't allow tracking
             -VV separately, like adding a separate description */
          /* vms_native++; break; */
        case o_VV:  /* Store in VMS specific format */
          vms_native = 2; break;
        case 'w':   /* Append the VMS version number */
          vmsver = 1;  break;
#endif /* VMS */
        case 'W':   /* Wildcards do not include directory boundaries in matches */
          wild_stop_at_dir = 1;
          break;

        case 'i':   /* Include only the following files */
        case 'x':   /* Exclude following files */
          add_filter((char) option, value);
          free(value);
          break;
#ifdef S_IFLNK
        case 'y':   /* Store symbolic links as such */
          linkput = 1;  break;
#endif /* S_IFLNK */
        case 'z':   /* Edit zip file comment */
          zipedit = 1;  break;
#if defined(MSDOS) || defined(OS2)
        case '$':   /* Include volume label */
          volume_label = 1; break;
#endif
#ifndef MACOS
        case '@':   /* read file names from stdin */
          comment_stream = NULL;
          s = 1;          /* defer -@ until after zipfile read */
          break;
#endif /* !MACOS */
        case 'X':
          extra_fields = 0;
          break;
#ifdef OS2
        case 'E':
          /* use the .LONGNAME EA (if any) as the file's name. */
          use_longname_ea = 1;
          break;
#endif
#ifdef WIN32
        case '!':
          /* use security privilege overrides */
          use_privileges = 1;
          break;
#endif
#ifdef RISCOS
        case '/':
          exts2swap = value; /* override Zip$Exts */
          break;
#endif
#ifdef ZIP64_SUPPORT
        case o_z64:   /* Force creation of Zip64 entries */
          force_zip64 = 1; break;
#endif

        case o_NON_OPTION_ARG:
          /* not an option */
          /* no more options as permuting */
          /* just dash also ends up here */

          if (recurse != 2 && k == 0 && patterns == NULL) {
            /* have all filters so convert filterlist to patterns array
               as PROCNAME needs patterns array */
            filterlist_to_patterns();
          }

          /* -- stops arg processing for remaining args - 7/25/04 EG */
          /*
          printf("value = '%s' k=%d\n", value, k);
          */
          /* ignore only first -- */
          if (strcmp(value, "--") == 0 && seen_doubledash == 0) {
            /* -- */
            seen_doubledash = 1;
            if (k == 0) {
              ZIPERR(ZE_PARMS, "can't use -- before archive name");
            }

            /* just ignore as just marks what follows as non-option arguments */

          } else if (k == 6) {
            /* value is R pattern */
            add_filter('R', value);
            free(value);
            if (first_listarg == 0) {
              first_listarg = argnum;
            }
          } else switch (k)
          {
            case 0:
              if (show_what_doing) {
                fprintf(stderr, "zipfile name '%s'\n", value);
                fflush(stderr);
              }
              /* first non-option arg is zipfile name */
#if (!defined(MACOS) && !defined(WINDLL))
              if (strcmp(value, "-") == 0) {  /* output zipfile is dash */
                /* just a dash */
                zipstdout();
              } else
#endif /* !MACOS && !WINDLL */
              {
               if ((zipfile = ziptyp(value)) == NULL) {
                 ZIPERR(ZE_MEM, "was processing arguments");
               }
               if ((r = readzipfile()) != ZE_OK) {
                 ZIPERR(r, zipfile);
               }
              }
              k = 3;
              if (s)
              {
                /* do -@ and get names from stdin */
                /* should be able to read names from
                   stdin and output to stdout, but
                   this was not allowed in old code.
                   This check moved to k = 3 case to fix. */
                /* if (strcmp(zipfile, "-") == 0) {
                  ZIPERR(ZE_PARMS, "can't use - and -@ together");
                }
                */
                while ((pp = getnam(stdin)) != NULL)
                {
                  k = 4;
                  if (recurse == 2) {
                    /* reading patterns from stdin */
                    add_filter('R', pp);
                  } else
                  if ((r = PROCNAME(pp)) != ZE_OK) {
                    if (r == ZE_MISS)
                      zipwarn("name not matched: ", pp);
                    else {
                      ZIPERR(r, pp);
                    }
                  }
                  free(pp);
                }
                s = 0;
              }
              if (recurse == 2) {
                /* rest are -R patterns */
                k = 6;
              }
              break;

            case 3:  case 4:
              /* no recurse and -r file names */
              /* can't read filenames -@ and input - from stdin at
                 same time */
              if (s == 1 && strcmp(value, "-") == 0) {
                ZIPERR(ZE_PARMS, "can't read input (-) and filenames (-@) both from stdin");
              }
              if ((r = PROCNAME(value)) != ZE_OK) {
                if (r == ZE_MISS)
                  zipwarn("name not matched: ", value);
                else {
                  ZIPERR(r, value);
                }
              }
              if (k == 3) {
                first_listarg = argnum;
                k = 4;
              }
              break;

            } /* switch k */
            break;

        default:
          /* should never get here as get_option will exit if not in table */
          sprintf(errbuf, "no such option ID: %ld", option);
          ZIPERR(ZE_PARMS, errbuf);

     }  /* switch */
  }

  if (show_what_doing) {
    fprintf(stderr, "Command line read\n");
    fflush(stderr);
  }
  /* show command line args */
  if (show_args) {
    fprintf(stderr, "command line:\n");
    for (i = 0; i < argcnt; i++) {
      fprintf(stderr, "'%s'  ", argv[i]);
    }
    fprintf(stderr, "\n");
    ZIPERR(ZE_ABORT, "show command line");
  }
  /* show all options */
  if (show_options) {
    printf("available options:\n");
    printf(" %-22s %-2s %-4s %-3s %-30s\n", "long", "sh", "val", "neg", "description");
    printf(" %-22s %-2s %-4s %-3s %-30s\n", "----", "--", "---", "---", "-----------");
    for (i = 0; options[i].option_ID; i++) {
      printf(" %-22s %-2s ", options[i].longopt, options[i].shortopt);
      switch (options[i].value_type) {
        case o_NO_VALUE:
          printf("%-4s ", "");
          break;
        case o_REQUIRED_VALUE:
          printf("%-4s ", "req");
          break;
        case o_OPTIONAL_VALUE:
          printf("%-4s ", "opt");
          break;
        case o_VALUE_LIST:
          printf("%-4s ", "list");
          break;
        case o_ONE_CHAR_VALUE:
          printf("%-4s ", "char");
          break;
        case o_NUMBER_VALUE:
          printf("%-4s ", "num");
          break;
        default:
          printf("%-4s ", "unk");
      }
      switch (options[i].negatable) {
        case o_NEGATABLE:
          printf("%-3s ", "neg");
          break;
        case o_NOT_NEGATABLE:
          printf("%-3s ", "");
          break;
        default:
          printf("%-3s ", "unk");
      }
      if (options[i].name)
        printf("%-30s\n", options[i].name);
      else
        printf("\n");
    }
    RETURN(finish(ZE_OK));
  }

  if (verbose && (dot_size == 0) && (dot_count == 0)) {
    /* show all dots as before if verbose set and dot_size not set */
    dot_size = -1;
  }

  /* done getting -R filters so convert filterlist if not done */
  if (pcount && patterns == NULL) {
    filterlist_to_patterns();
  }

#if (defined(MSDOS) || defined(OS2)) && !defined(WIN32)
  if ((k == 3 || k == 4) && volume_label == 1) {
    /* read volume label */
    PROCNAME(NULL);
    k = 4;
  }
#endif

  if ((recurse == 2 || pcount) && first_listarg == 0 &&
      (k < 3 || (action != UPDATE && action != FRESHEN))) {
    ZIPERR(ZE_PARMS, "nothing to select from");
  }

  /* recurse from current directory for -R */
  if (recurse == 2) {
#ifdef AMIGA
    if ((r = PROCNAME("")) != ZE_OK)
#else
    if ((r = PROCNAME(".")) != ZE_OK)
#endif
    {
      if (r == ZE_MISS)
        zipwarn("name not matched: ", "current directory for -R");
      else {
        ZIPERR(r, "-R");
      }
    }
  }

/*
  -------------------------------------
  end of new command line code
  -------------------------------------
*/

#if (!defined(MACOS) && !defined(WINDLL))
  if (k < 3) {               /* zip used as filter */
    zipstdout();
    comment_stream = NULL;
    if ((r = procname("-", 0)) != ZE_OK) {
      if (r == ZE_MISS)
        zipwarn("name not matched: ", "-");
      else {
        ZIPERR(r, "-");
      }
    }
    k = 4;
    if (s) {
      ZIPERR(ZE_PARMS, "can't use - and -@ together");
    }
  }
#endif /* !MACOS && !WINDLL */

  if (show_what_doing) {
    fprintf(stderr, "Apply filters\n");
    fflush(stderr);
  }
  /* Clean up selections ("3 <= k <= 5" now) */
  if (k != 4 && first_listarg == 0 &&
      (action == UPDATE || action == FRESHEN)) {
    /* if -u or -f with no args, do all, but, when present, apply filters */
    for (z = zfiles; z != NULL; z = z->nxt) {
      z->mark = pcount ? filter(z->zname, 0) : 1;
#ifdef DOS
      if (z->mark) z->dosflag = 1;      /* force DOS attribs for incl. names */
#endif
    }
  }
  if (show_what_doing) {
    fprintf(stderr, "Check dups\n");
    fflush(stderr);
  }
  if ((r = check_dup()) != ZE_OK) {     /* remove duplicates in found list */
    if (r == ZE_PARMS) {
      ZIPERR(r, "cannot repeat names in zip file");
    }
    else {
      ZIPERR(r, "was processing list of files");
    }
  }

  if (zcount)
    free((zvoid *)zsort);

  /* Check option combinations */
  if (special == NULL) {
    ZIPERR(ZE_PARMS, "missing suffix list");
  }
  if (level == 9 || !strcmp(special, ";") || !strcmp(special, ":"))
    special = NULL; /* compress everything */

  if (action == DELETE && (method != BEST || dispose || recurse ||
      key != NULL || comadd || zipedit)) {
    zipwarn("invalid option(s) used with -d; ignored.","");
    /* reset flags - needed? */
    method  = BEST;
    dispose = 0;
    recurse = 0;
    if (key != NULL) {
      free((zvoid *)key);
      key   = NULL;
    }
    comadd  = 0;
    zipedit = 0;
  }
  if (linkput && dosify)
    {
      zipwarn("can't use -y with -k, -y ignored", "");
      linkput = 0;
    }
  if (fix && adjust)
    {
      zipwarn("can't use -F with -A, -F ignored", "");
    }
  if (test && !strcmp(zipfile, "-")) {
    test = 0;
    zipwarn("can't use -T on stdout, -T ignored", "");
  }
  if ((action != ADD || d) && !strcmp(zipfile, "-")) {
    ZIPERR(ZE_PARMS, "can't use -d,-f,-u or -g on stdout\n");
  }
#if defined(EBCDIC)  && !defined(OS390)
  if (aflag==ASCII && !translate_eol) {
    /* Translation to ASCII implies EOL translation!
     * (on OS390, consistent EOL translation is controlled separately)
     * The default translation mode is "UNIX" mode (single LF terminators).
     */
    translate_eol = 2;
  }
#endif
#ifdef CMS_MVS
  if (aflag==ASCII && bflag)
    ZIPERR(ZE_PARMS, "can't use -a with -B");
#endif
#ifdef VMS
  if (!extra_fields && vms_native)
    {
      zipwarn("can't use -V with -X, -V ignored", "");
      vms_native = 0;
    }
  if (vms_native && translate_eol)
    ZIPERR(ZE_PARMS, "can't use -V with -l or -ll");
#endif
  if (zcount == 0 && (action != ADD || d)) {
    zipwarn(zipfile, " not found or empty");
  }

/*
 * XXX make some kind of mktemppath() function for each OS.
 */

#ifndef VM_CMS
/* For CMS, leave tempath NULL.  A-disk will be used as default. */
  /* If -b not specified, make temporary path the same as the zip file */
#if defined(MSDOS) || defined(__human68k__) || defined(AMIGA)
  if (tempath == NULL && ((p = MBSRCHR(zipfile, '/')) != NULL ||
#  ifdef MSDOS
                          (p = MBSRCHR(zipfile, '\\')) != NULL ||
#  endif /* MSDOS */
                          (p = MBSRCHR(zipfile, ':')) != NULL))
  {
    if (*p == ':')
      p++;
#else
#ifdef RISCOS
  if (tempath == NULL && (p = MBSRCHR(zipfile, '.')) != NULL)
  {
#else
#ifdef QDOS
  if (tempath == NULL && (p = LastDir(zipfile)) != NULL)
  {
#else
  if (tempath == NULL && (p = MBSRCHR(zipfile, '/')) != NULL)
  {
#endif /* QDOS */
#endif /* RISCOS */
#endif /* MSDOS || __human68k__ || AMIGA */
    if ((tempath = malloc((int)(p - zipfile) + 1)) == NULL) {
      ZIPERR(ZE_MEM, "was processing arguments");
    }
    r = *p;  *p = 0;
    strcpy(tempath, zipfile);
    *p = (char)r;
  }
#endif /* VM_CMS */

#if (defined(IZ_CHECK_TZ) && defined(USE_EF_UT_TIME))
  if (!zp_tz_is_valid && action != DELETE) {
    zipwarn("TZ environment variable not found, cannot use UTC times!!","");
  }
#endif /* IZ_CHECK_TZ && USE_EF_UT_TIME */

  /* For each marked entry, if not deleting, check if it exists, and if
     updating or freshening, compare date with entry in old zip file.
     Unmark if it doesn't exist or is too old, else update marked count. */
  if (show_what_doing) {
    fprintf(stderr, "Scanning files to add\n");
    fflush(stderr);
  }
#ifdef MACOS
  PrintStatProgress("Getting file information ...");
#endif
  diag("stating marked entries");
  k = 0;                        /* Initialize marked count */
  for (z = zfiles; z != NULL; z = z->nxt)
    if (z->mark) {
#ifdef USE_EF_UT_TIME
      iztimes f_utim, z_utim;
#endif /* USE_EF_UT_TIME */
      Trace((stderr, "zip diagnostics: marked file=%s\n", z->zname));

      if (action != DELETE &&
#ifdef USE_EF_UT_TIME
          ((tf = filetime(z->name, (ulg *)NULL, &usize, &f_utim))
#else /* !USE_EF_UT_TIME */
          ((tf = filetime(z->name, (ulg *)NULL, &usize, (iztimes *)NULL))
#endif /* ?USE_EF_UT_TIME */
              == 0 ||
           tf < before || (after && tf >= after) ||
           ((action == UPDATE || action == FRESHEN) &&
#ifdef USE_EF_UT_TIME
            ((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
             f_utim.mtime <= ROUNDED_TIME(z_utim.mtime) : tf <= z->tim)
#else /* !USE_EF_UT_TIME */
            tf <= z->tim
#endif /* ?USE_EF_UT_TIME */
           )))
      {
        z->mark = comadd ? 2 : 0;
        z->trash = tf && tf >= before &&
                   (after == 0 || tf < after);   /* delete if -um or -fm */
        if (verbose) {
          fprintf(mesg, "zip diagnostic: %s %s\n", z->zname,
                 z->trash ? "up to date" : "missing or early");
        }
      }
      else if (action != DELETE) {
        files_total++;
        bytes_total += usize;
        k++;
      }
    } else {
      /* not marked */
      if (action == DELETE) {
        files_total++;
        bytes_total += z->len;
      }

    }

  /* Remove entries from found list that do not exist or are too old */
  if (show_what_doing) {
    fprintf(stderr, "fcount = %u\n", (unsigned)fcount);
    fflush(stderr);
  }
  diag("stating new entries");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL;) {
    Trace((stderr, "zip diagnostic: new file=%s\n", f->zname));
    if (action == DELETE || action == FRESHEN ||
        (tf = filetime(f->name, (ulg *)NULL, &usize, (iztimes *)NULL)) == 0 ||
         tf < before || (after && tf >= after) ||
          (namecmp(f->zname, zipfile) == 0 && strcmp(zipfile, "-"))
        )
      f = fexpel(f);
    else {
      /* ??? */
      f->usize = usize;
      files_total++;
      bytes_total += usize;
      f = f->nxt;
    }
  }
#ifdef MACOS
  PrintStatProgress("done");
#endif

  /* Make sure there's something left to do */
  if (k == 0 && found == NULL &&
      !(zfiles != NULL &&
        (latest || fix || adjust || junk_sfx || comadd || zipedit))) {
    if (test && (zfiles != NULL || zipbeg != 0)) {
#ifndef WINDLL
      check_zipfile(zipfile, argv[0]);
#endif
      RETURN(finish(ZE_OK));
    }
    if (action == UPDATE || action == FRESHEN) {
      RETURN(finish(ZE_NONE));
    }
    else if (zfiles == NULL && (latest || fix || adjust || junk_sfx)) {
      ZIPERR(ZE_NAME, zipfile);
    }
#ifndef WINDLL
    else if (recurse && (pcount == 0) && (first_listarg > 0)) {
#ifdef VMS
      strcpy(errbuf, "try: zip \"");
      for (i = 1; i < (first_listarg - 1); i++)
        strcat(strcat(errbuf, argv[i]), "\" ");
      strcat(strcat(errbuf, argv[i]), " *.* -i");
#else /* !VMS */
      strcpy(errbuf, "try: zip");
      for (i = 1; i < first_listarg; i++)
        strcat(strcat(errbuf, " "), argv[i]);
#  ifdef AMIGA
      strcat(errbuf, " \"\" -i");
#  else
      strcat(errbuf, " . -i");
#  endif
#endif /* ?VMS */
      for (i = first_listarg; i < argc; i++)
        strcat(strcat(errbuf, " "), argv[i]);
      ZIPERR(ZE_NONE, errbuf);
    }
#endif /* !WINDLL */
    else {
      ZIPERR(ZE_NONE, zipfile);
    }
  }
  d = (d && k == 0 && (zipbeg || zfiles != NULL)); /* d true if appending */

#if CRYPT
  /* Initialize the crc_32_tab pointer, when encryption was requested. */
  if (key != NULL) {
    crc_32_tab = get_crc_table();
#ifdef EBCDIC
    /* convert encryption key to ASCII (ISO variant for 8-bit ASCII chars) */
    strtoasc(key, key);
#endif /* EBCDIC */
  }
#endif /* CRYPT */

  /* Before we get carried away, make sure zip file is writeable. This
   * has the undesired side effect of leaving one empty junk file on a WORM,
   * so when the zipfile does not exist already and when -b is specified,
   * the writability check is made in replace().
   */
  if (strcmp(zipfile, "-"))
  {
    if (tempdir && zfiles == NULL && zipbeg == 0) {
      a = 0;
    } else {
      x = zfiles == NULL && zipbeg == 0 ? zfopen(zipfile, FOPW) :
                                          zfopen(zipfile, FOPM);
      /* Note: FOPW and FOPM expand to several parameters for VMS */
      if (x == NULL) {
        ZIPERR(ZE_CREAT, zipfile);
      }
      fclose(x);
      a = getfileattr(zipfile);
      if (zfiles == NULL && zipbeg == 0)
        destroy(zipfile);
    }
  }
  else
    a = 0;

  /* Throw away the garbage in front of the zip file for -J */
  if (junk_sfx) zipbeg = 0;

  /* Open zip file and temporary output file */
  if (show_what_doing) {
    fprintf(stderr, "open zip file and create temp file\n");
    fflush(stderr);
  }
  diag("opening zip file and creating temporary zip file");
  x = NULL;
  tempzn = 0;
  if (strcmp(zipfile, "-") == 0)
  {
#ifdef MSDOS
    /* It is nonsense to emit the binary data stream of a zipfile to
     * the (text mode) console.  This case should already have been caught
     * in a call to zipstdout() far above.  Therefore, if the following
     * failsafe check detects a console attached to stdout, zip is stopped
     * with an "internal logic error"!  */
    if (isatty(fileno(stdout)))
      ZIPERR(ZE_LOGIC, "tried to write binary zipfile data to console!");
    /* Set stdout mode to binary for MSDOS systems */
#  ifdef __HIGHC__
    setmode(stdout, _BINARY);
#  else
    setmode(fileno(stdout), O_BINARY);
#  endif
    tempzf = y = zfdopen(fileno(stdout), FOPW);
#else
    tempzf = y = stdout;
#endif
    /* tempzip must be malloced so a later free won't barf */
    tempzip = malloc(4);
    if (tempzip == NULL) {
      ZIPERR(ZE_MEM, "allocating temp filename");
    }
    strcpy(tempzip, "-");
  }
  else if (d) /* d true if just appending (-g) */
  {
    if ((y = zfopen(zipfile, FOPM)) == NULL) {
      ZIPERR(ZE_NAME, zipfile);
    }
    tempzip = zipfile;
    tempzf = y;

    if (zfseeko(y, cenbeg, SEEK_SET)) {
      ZIPERR(ferror(y) ? ZE_READ : ZE_EOF, zipfile);
    }
    tempzn = cenbeg;
  }
  else
  {
    if ((zfiles != NULL || zipbeg) && (x = zfopen(zipfile, FOPR_EX)) == NULL) {
      ZIPERR(ZE_NAME, zipfile);
    }
    if ((tempzip = tempname(zipfile)) == NULL) {
      ZIPERR(ZE_MEM, "allocating temp filename");
    }
    if ((tempzf = y = zfopen(tempzip, FOPW_TMP)) == NULL) {
      ZIPERR(ZE_TEMP, tempzip);
    }
  }

#if (!defined(VMS) && !defined(CMS_MVS))
  /* Use large buffer to speed up stdio: */
#if (defined(_IOFBF) || !defined(BUFSIZ))
  zipbuf = (char *)malloc(ZBSZ);
#else
  zipbuf = (char *)malloc(BUFSIZ);
#endif
  if (zipbuf == NULL) {
    ZIPERR(ZE_MEM, tempzip);
  }
# ifdef _IOFBF
  setvbuf(y, zipbuf, _IOFBF, ZBSZ);
# else
  setbuf(y, zipbuf);
# endif /* _IOBUF */
#endif /* !VMS  && !CMS_MVS */

  if (strcmp(zipfile, "-") != 0 && !d)  /* this must go *after* set[v]buf */
  {
    if (zipbeg && (r = fcopy(x, y, zipbeg)) != ZE_OK) {
      ZIPERR(r, r == ZE_TEMP ? tempzip : zipfile);
      }
    tempzn = zipbeg;
  }

  o = 0;                                /* no ZE_OPEN errors yet */


  /* Process zip file, updating marked files */
#ifdef DEBUG
  if (zfiles != NULL)
    diag("going through old zip file");
#endif
  if (zfiles != NULL && show_what_doing) {
    fprintf(stderr, "Going through old zip file\n");
    fflush(stderr);
  }
  w = &zfiles;
  while ((z = *w) != NULL) {
    if (z->mark == 1)
    {
      /* if not deleting, zip it up */
      if (action != DELETE)
      {
        if (noisy)
        {
          DisplayRunningStats();
          if (action == FRESHEN)
             fprintf(mesg, "freshening: %s", z->zname);
          else
             fprintf(mesg, "updating: %s", z->zname);
          fflush(mesg);
        }
        if ((r = zipup(z, y)) != ZE_OK && r != ZE_OPEN && r != ZE_MISS)
        {
          if (noisy)
          {
#if (!defined(MACOS) && !defined(WINDLL))
            putc('\n', mesg);
            fflush(mesg);
#else
            fprintf(stdout, "\n");
#endif
          }
          sprintf(errbuf, "was zipping %s", z->name);
          ZIPERR(r, errbuf);
        }
        if (r == ZE_OPEN || r == ZE_MISS)
        {
          o = 1;
          if (noisy)
          {
#if (!defined(MACOS) && !defined(WINDLL))
            putc('\n', mesg);
            fflush(mesg);
#else
            fprintf(stdout, "\n");
#endif
          }
          if (r == ZE_OPEN) {
            perror(z->zname);
            zipwarn("could not open for reading: ", z->zname);
          } else {
            zipwarn("file and directory with the same name: ", z->zname);
          }
          zipwarn("will just copy entry over: ", z->zname);
          if ((r = zipcopy(z, x, y)) != ZE_OK)
          {
            sprintf(errbuf, "was copying %s", z->zname);
            ZIPERR(r, errbuf);
          }
          z->mark = 0;
        }
        files_so_far++;
        bytes_so_far += z->len;
        w = &z->nxt;
      }
      else
      {
        if (noisy)
        {
          fprintf(mesg, "deleting: %s\n", z->zname);
          fflush(mesg);
        }
#ifdef WINDLL
#ifdef ZIP64_SUPPORT
        /* int64 support in caller */
        if (lpZipUserFunctions->ServiceApplication64 != NULL) 
        {
          if ((*lpZipUserFunctions->ServiceApplication64)(z->zname, z->siz))
                    ZIPERR(ZE_ABORT, "User terminated operation");
        }
        else
        {
          /* no int64 support in caller */
          filesize64 = z->siz;
          low = (unsigned long)(filesize64 & 0x00000000FFFFFFFF);
          high = (unsigned long)((filesize64 >> 32) & 0x00000000FFFFFFFF);
          if (lpZipUserFunctions->ServiceApplication64_No_Int64 != NULL) {
            if ((*lpZipUserFunctions->ServiceApplication64_No_Int64)(z->zname, low, high))
                      ZIPERR(ZE_ABORT, "User terminated operation");
          }
        }
#else
        if (lpZipUserFunctions->ServiceApplication != NULL) {
          if ((*lpZipUserFunctions->ServiceApplication)(z->zname, z->siz))
                  ZIPERR(ZE_ABORT, "User terminated operation");
        }
#endif //ZIP64_SUPPORT
#endif //WINDLL

        v = z->nxt;                     /* delete entry from list */
        free((zvoid *)(z->iname));
        free((zvoid *)(z->zname));
        if (z->ext)
          free((zvoid *)(z->extra));
        if (z->cext && z->cextra != z->extra)
          free((zvoid *)(z->cextra));
        if (z->com)
          free((zvoid *)(z->comment));
        farfree((zvoid far *)z);
        *w = v;
        zcount--;
      }
    }
    else
    {
      /* copy the original entry verbatim */
      if (!d && (r = zipcopy(z, x, y)) != ZE_OK)
      {
        sprintf(errbuf, "was copying %s", z->zname);
        ZIPERR(r, errbuf);
      }
      files_so_far++;
      bytes_so_far += z->len;
      w = &z->nxt;
    }
  }


  /* Process the edited found list, adding them to the zip file */
  if (show_what_doing) {
    fprintf(stderr, "zipping up new entries\n");
    fflush(stderr);
  }
  diag("zipping up new entries, if any");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL; f = fexpel(f))
  {
    /* add a new zfiles entry and set the name */
    if ((z = (struct zlist far *)farmalloc(sizeof(struct zlist))) == NULL) {
      ZIPERR(ZE_MEM, "was adding files to zip file");
    }
    z->nxt = NULL;
    z->name = f->name;
    f->name = NULL;
    z->iname = f->iname;
    f->iname = NULL;
    z->zname = f->zname;
    f->zname = NULL;
    z->ext = z->cext = z->com = 0;
    z->extra = z->cextra = NULL;
    z->mark = 1;
    z->dosflag = f->dosflag;
    /* zip it up */
    if (noisy)
    {
      DisplayRunningStats();
      fprintf(mesg, "  adding: %s", z->zname);
      fflush(mesg);
    }
    if ((r = zipup(z, y)) != ZE_OK  && r != ZE_OPEN && r != ZE_MISS)
    {
      if (noisy)
      {
#if (!defined(MACOS) && !defined(WINDLL))
        putc('\n', mesg);
        fflush(mesg);
#else
        fprintf(stdout, "\n");
#endif
      }
      sprintf(errbuf, "was zipping %s", z->zname);
      ZIPERR(r, errbuf);
    }
    if (r == ZE_OPEN || r == ZE_MISS)
    {
      o = 1;
      if (noisy)
      {
#if (!defined(MACOS) && !defined(WINDLL))
        putc('\n', mesg);
        fflush(mesg);
#else
        fprintf(stdout, "\n");
#endif
      }
      if (r == ZE_OPEN) {
        perror("zip warning");
        zipwarn("could not open for reading: ", z->zname);
      } else {
        zipwarn("file and directory with the same name: ", z->zname);
      }
      free((zvoid *)(z->name));
      free((zvoid *)(z->iname));
      free((zvoid *)(z->zname));
      farfree((zvoid far *)z);
    }
    else
    {
      files_so_far++;
      bytes_so_far += z->len;
      *w = z;
      w = &z->nxt;
      zcount++;
    }
  }
  if (key != NULL)
  {
    free((zvoid *)key);
    key = NULL;
  }


  /* Get one line comment for each new entry */
  if (show_what_doing) {
    fprintf(stderr, "Get comment if any\n");
    fflush(stderr);
  }
#if defined(AMIGA) || defined(MACOS)
  if (comadd || filenotes)
  {
    if (comadd)
#else
  if (comadd)
  {
#endif
    {
      if (comment_stream == NULL) {
#ifndef RISCOS
        comment_stream = (FILE*)fdopen(fileno(stderr), "r");
#else
        comment_stream = stderr;
#endif
      }
      if ((e = malloc(MAXCOM + 1)) == NULL) {
        ZIPERR(ZE_MEM, "was reading comment lines");
      }
    }
#ifdef __human68k__
    setmode(fileno(comment_stream), O_TEXT);
#endif
#ifdef MACOS
    if (noisy) fprintf(mesg, "\nStart commenting files ...\n");
#endif
    for (z = zfiles; z != NULL; z = z->nxt)
      if (z->mark)
#if defined(AMIGA) || defined(MACOS)
        if (filenotes && (p = GetComment(z->zname)))
        {
          if (z->comment = malloc(k = strlen(p)+1))
          {
            z->com = k;
            strcpy(z->comment, p);
          }
          else
          {
            free((zvoid *)e);
            ZIPERR(ZE_MEM, "was reading filenotes");
          }
        }
        else if (comadd)
#endif /* AMIGA || MACOS */
        {
          if (noisy)
            fprintf(mesg, "Enter comment for %s:\n", z->zname);
          if (fgets(e, MAXCOM+1, comment_stream) != NULL)
          {
            if ((p = malloc((extent)(k = strlen(e))+1)) == NULL)
            {
              free((zvoid *)e);
              ZIPERR(ZE_MEM, "was reading comment lines");
            }
            strcpy(p, e);
            if (p[k-1] == '\n')
              p[--k] = 0;
            z->comment = p;
            /* zip64 support 09/05/2003 R.Nausedat */
            z->com = (extent)k;
          }
        }
#ifdef MACOS
    if (noisy) fprintf(mesg, "\n...done");
#endif
#if defined(AMIGA) || defined(MACOS)
    if (comadd)
      free((zvoid *)e);
    GetComment(NULL);           /* makes it free its internal storage */
#else
    free((zvoid *)e);
#endif
  }

  /* Get multi-line comment for the zip file */
  if (zipedit)
  {
#ifndef WINDLL
    if (comment_stream == NULL) {
#ifndef RISCOS
      comment_stream = (FILE*)fdopen(fileno(stderr), "r");
#else
      comment_stream = stderr;
#endif
    }
    if ((e = malloc(MAXCOM + 1)) == NULL) {
      ZIPERR(ZE_MEM, "was reading comment lines");
    }
    if (noisy && zcomlen)
    {
      fputs("current zip file comment is:\n", mesg);
      fwrite(zcomment, 1, zcomlen, mesg);
      if (zcomment[zcomlen-1] != '\n')
        putc('\n', mesg);
      free((zvoid *)zcomment);
    }
    if ((zcomment = malloc(1)) == NULL)
      ZIPERR(ZE_MEM, "was setting comments to null");
    zcomment[0] = '\0';
    if (noisy)
      fputs("enter new zip file comment (end with .):\n", mesg);
#if (defined(AMIGA) && (defined(LATTICE)||defined(__SASC)))
    flushall();  /* tty input/output is out of sync here */
#endif
#ifdef __human68k__
    setmode(fileno(comment_stream), O_TEXT);
#endif
#ifdef MACOS
    printf("\n enter new zip file comment \n");
    if (fgets(e, MAXCOM+1, comment_stream) != NULL) {
        if ((p = malloc((k = strlen(e))+1)) == NULL) {
            free((zvoid *)e);
            ZIPERR(ZE_MEM, "was reading comment lines");
        }
        strcpy(p, e);
        if (p[k-1] == '\n') p[--k] = 0;
        zcomment = p;
    }
#else /* !MACOS */
    while (fgets(e, MAXCOM+1, comment_stream) != NULL && strcmp(e, ".\n"))
    {
      if (e[(r = strlen(e)) - 1] == '\n')
        e[--r] = 0;
      if ((p = malloc((*zcomment ? strlen(zcomment) + 3 : 1) + r)) == NULL)
      {
        free((zvoid *)e);
        ZIPERR(ZE_MEM, "was reading comment lines");
      }
      if (*zcomment)
        strcat(strcat(strcpy(p, zcomment), "\r\n"), e);
      else
        strcpy(p, *e ? e : "\r\n");
      free((zvoid *)zcomment);
      zcomment = p;
    }
#endif /* ?MACOS */
    free((zvoid *)e);
#else /* WINDLL */
    comment(zcomlen);
    if ((p = malloc(strlen(szCommentBuf)+1)) == NULL) {
      ZIPERR(ZE_MEM, "was setting comments to null");
    }
    if (szCommentBuf[0] != '\0')
       lstrcpy(p, szCommentBuf);
    else
       p[0] = '\0';
    free((zvoid *)zcomment);
    GlobalUnlock(hStr);
    GlobalFree(hStr);
    zcomment = p;
#endif /* WINDLL */
    zcomlen = strlen(zcomment);
  }


  /* Write central directory and end header to temporary zip */
  diag("writing central directory");
  k = 0;                        /* keep count for end header */
  c = tempzn;                   /* get start of central */
  n = t = 0;
  for (z = zfiles; z != NULL; z = z->nxt)
  {
    if ((r = putcentral(z, y)) != ZE_OK) {
      ZIPERR(r, tempzip);
    }
    tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
    n += z->len;
    t += z->siz;
    k++;
  }
  if (k == 0)
    zipwarn("zip file empty", "");
  if (verbose) {
    fprintf( mesg, "total bytes=%s, compressed=%s -> %d%% savings\n",
             fzofft(n, NULL, "u"), fzofft(t, NULL, "u"), percent(n, t));
  }
  t = tempzn - c;               /* compute length of central */
  diag("writing end of central directory");
  if ((r = putend(k, t, c, zcomlen, zcomment, y)) != ZE_OK) {
    ZIPERR(r, tempzip);
  }
  tempzf = NULL;
  if (fclose(y)) {
    ZIPERR(d ? ZE_WRITE : ZE_TEMP, tempzip);
  }
  if (x != NULL)
    fclose(x);

  /* Free some memory before spawning unzip */
#ifdef USE_ZLIB
  zl_deflate_free();
#else
  lm_free();
#endif

#ifndef WINDLL
  /* Test new zip file before overwriting old one or removing input files */
  if (test)
    check_zipfile(tempzip, argv[0]);
#endif
  /* Replace old zip file with new zip file, leaving only the new one */
  if (strcmp(zipfile, "-") && !d)
  {
    diag("replacing old zip file with new zip file");
    if ((r = replace(zipfile, tempzip)) != ZE_OK)
    {
      zipwarn("new zip file left as: ", tempzip);
      free((zvoid *)tempzip);
      tempzip = NULL;
      ZIPERR(r, "was replacing the original zip file");
    }
    free((zvoid *)tempzip);
  }
  tempzip = NULL;
  if (a && strcmp(zipfile, "-")) {
    setfileattr(zipfile, a);
#ifdef VMS
    /* If the zip file existed previously, restore its record format: */
    if (x != NULL)
      (void)VMSmunch(zipfile, RESTORE_RTYPE, NULL);
#endif
  }

#ifdef __BEOS__
  /* Set the filetype of the zipfile to "application/zip" */
  setfiletype( zipfile, "application/zip" );
#endif

#ifdef __ATHEOS__
  /* Set the filetype of the zipfile to "application/x-zip" */
  setfiletype(zipfile, "application/x-zip");
#endif

#ifdef MACOS
  /* Set the Creator/Type of the zipfile to 'IZip' and 'ZIP ' */
  setfiletype(zipfile, 'IZip', 'ZIP ');
#endif

#ifdef RISCOS
  /* Set the filetype of the zipfile to &DDC */
  setfiletype(zipfile, 0xDDC);
#endif

  /* Finish up (process -o, -m, clean up).  Exit code depends on o. */
#if (!defined(VMS) && !defined(CMS_MVS))
  free((zvoid *) zipbuf);
#endif /* !VMS && !CMS_MVS */
  RETURN(finish(o ? ZE_OPEN : ZE_OK));
}
