/*
  zip.c - Zip 3

  Copyright (c) 1990-2006 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
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
#include <errno.h>
#ifdef VMS
#  include <stsdef.h>
#  include "vms/vmsmunch.h"
#  include "vms/vms.h"
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
#define ARCHIVE 4
local int action = ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
local int comadd = 0;   /* 1=add comments for new files */
local int zipedit = 0;  /* 1=edit zip comment and all file comments */
local int latest = 0;   /* 1=set zip file time to time of latest file */
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
  uzoff_t filesize64;
#endif
#endif

/* Temporary zip file name and file pointer */
#ifndef MACOS
/* local char *tempzip; */
local FILE *tempzf;
#else
/* char *tempzip; */
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
local void help_extended OF((void));
#endif /* !VMSCLI */
#endif /* !MACOS && !WINDLL */

/* prereading of arguments is not supported in new command
   line interpreter get_option() so read filters as arguments
   are processed and convert to expected array later */
local int add_filter OF((int flag, char *pattern));
local int filterlist_to_patterns OF((void));
/* not used
 local int get_filters OF((int argc, char **argv));
*/

/* list to store file arguments */
local int add_name OF((char *filearg));


local int DisplayRunningStats OF((void));

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

/* structure used by add_filearg to store file arguments */
struct filelist_struct {
  char *name;
  struct filelist_struct *next;
};
long filearg_count = 0;
struct filelist_struct *filelist = NULL;  /* start of list */
struct filelist_struct *lastfile = NULL;  /* last file in list */

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
    if (zfiles->oname)
      free((zvoid *)(zfiles->oname));
#ifdef UNICODE_SUPPORT
    if (zfiles->uname)
      free((zvoid *)(zfiles->uname));
#endif
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

  /* close logfile */
  if (logfile) {
    fclose(logfile);
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
  if (in_file != NULL)
  {
    fclose(in_file);
    in_file = NULL;
  }
  if (in_path != NULL)
  {
    free((zvoid *)in_path);
    in_path = NULL;
  }
  if (out_path != NULL)
  {
    free((zvoid *)out_path);
    out_path = NULL;
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
ZCONST char *h;         /* message about how it happened */
/* Issue a message for the error, clean up files and memory, and exit. */
{
#ifndef WINDLL
#ifndef MACOS
  static int error_level = 0;
#endif

  if (error_level++ > 0)
     /* avoid recursive ziperr() printouts (his should never happen) */
     EXIT(ZE_LOGIC);  /* ziperr recursion is an internal logic error! */
#endif /* !WINDLL */

  if (h != NULL) {
    if (PERR(c))
      fprintf(mesg, "zip I/O error: %s", strerror(errno));
      /* perror("zip I/O error"); */
    fflush(mesg);
    fprintf(mesg, "\nzip error: %s (%s)\n", ziperrors[c-1], h);
#ifdef DOS
    check_for_windows("Zip");
#endif
    if (logfile) {
      if (PERR(c))
        fprintf(logfile, "zip I/O error: %s\n", strerror(errno));
      fprintf(logfile, "\nzip error: %s (%s)\n", ziperrors[c-1], h);
    }
  }
  if (tempzip != NULL)
  {
    if (tempzip != zipfile) {

      /*
      if (tempzf != NULL)
      */
#ifdef SPLIT_FILE
      if (current_local_file)
        fclose(current_local_file);
#endif
      if (y != NULL)
        fclose(y);
#ifndef DEBUG
      destroy(tempzip);
#endif
      free((zvoid *)tempzip);
    } else {
      /* -g option, attempt to restore the old file */

      /* zip64 support 09/05/2003 R.Nausedat */
      uzoff_t k = 0;                        /* keep count for end header */
      uzoff_t cb = cenbeg;                  /* get start of central */

      struct zlist far *z;  /* steps through zfiles linked list */

      fprintf(mesg, "attempting to restore %s to its previous state\n",
         zipfile);
      if (logfile)
        fprintf(logfile, "attempting to restore %s to its previous state\n",
           zipfile);

      zfseeko(y, cenbeg, SEEK_SET);

      tempzn = cenbeg;
      for (z = zfiles; z != NULL; z = z->nxt)
      {
        putcentral(z);
        tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
        k++;
      }
      putend(k, tempzn - cb, cb, zcomlen, zcomment);
      fclose(y);
      y = NULL;
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
  if (out_path != NULL) {
    free((zvoid *)out_path);
    out_path = NULL;
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
  ZCONST char *h;
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
  putc('\n', mesg);
#endif /* !MSDOS */
#endif /* AMIGA && __SASC */
  ziperr(ZE_ABORT, "aborting");
  s++;                                  /* keep some compilers happy */
}
#endif /* !MACOS && !WINDLL */

void zipmessage(a, b)
ZCONST char *a, *b;     /* message strings juxtaposed in output */
/* Print a message to mesg and return. */
{
  if (noisy) {
    if (mesg_line_started)
      fprintf(mesg, "\n");
    fprintf(mesg, "%s%s\n", a, b);
    mesg_line_started = 0;
  }
  if (logfile) fprintf(logfile, "%s%s\n", a, b);
  fflush(mesg);
}

void zipwarn(a, b)
ZCONST char *a, *b;     /* message strings juxtaposed in output */
/* Print a warning message to mesg (usually stderr) and return. */
{
  if (noisy) {
    if (mesg_line_started)
      fprintf(mesg, "\n");
    fprintf(mesg, "\tzip warning: %s%s\n", a, b);
    mesg_line_started = 0;
  }
  if (logfile) fprintf(logfile, "\tzip warning: %s%s\n", a, b);
  fflush(mesg);
}

#ifndef WINDLL
local void license()
/* Print license information to stdout. */
{
  extent i;             /* counter for copyright array */

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
#ifdef VMS
"Zip %s (%s). Usage: zip == \"$ disk:[dir]zip.exe\"",
#else
"Zip %s (%s). Usage:",
#endif
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
," -h2  show more help",
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
"  -C   preserve case of file names  -C-  down-case all file names",
"  -C2  preserve case of ODS2 names  -C2- down-case ODS2 file names* (*=default)",
"  -C5  preserve case of ODS5 names* -C5- down-case ODS5 file names",
"  -V   save VMS file attributes (-VV also save allocated blocks past EOF)",
"  -w   store file version numbers\
   -ww  store file version numbers as \".nnn\"",
#endif /* def VMS */
#ifdef NTSD_EAS
"  -!   use privileges (if granted) to obtain all aspects of WinNT security",
#endif /* NTSD_EAS */
#ifdef OS2
"  -E   use the .LONGNAME Extended attribute (if found) as filename",
#endif /* OS2 */
#ifdef S_IFLNK
"  -y   store symbolic links as the link instead of the referenced file",
#endif /* !S_IFLNK */
/*
"  -R   PKZIP recursion (see manual)",
*/
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
,"  -h2  show more help               -I   don't scan thru Image files"
#else
,"  -h2  show more help"
#endif
#endif /* ?MACOS */
#ifdef VMS
,"  (Must quote upper-case options, like \"-V\", unless SET PROC/PARSE=EXTEND)"
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
"",
"See the Zip Manual for more detailed help",
"",
"BASICS (Read This):",
"",
"Zip stores files in zip archives.  Some examples:",
"  Add file.txt to z.zip (create z if needed):  zip z file.txt",
"  Zip all files in current dir:                zip z *",
"  Zip files in current dir and subdirs also:   zip -r z .",
/*
"  Pipe output from another program to zip:     program | zip z -",
"  Stream output of zip to another program:     zip - file1 file2 | program",
"  Use zip as filter (same as zip - -):         program1 | zip | program2",
*/
"",
"Basic command line:",
"  zip options archive_name file file ...",
"",
"Basic modes (default is Update):",
" External modes (selects files from file system):",
"  -u    Update   - Add new files and update existing files in archive",
"  -f    Freshen  - Update existing files only (no files added)",
"  -m    Move     - Add files, then delete originals (move into archive)",
" Internal modes (selects entries in archive):",
"  -d    Delete   - Delete files from archive",
"  -U    Archive  - Select files in archive to copy (use with --out)",
"",
"Some basic options:",
"  -0             - store the file",
"  -1 to -9       - compress fastest to compress best",
"  -r             - recurse into directories (see Recursion below)",
"  -j             - junk directory names (store just file names)",
"  -q             - quiet operation",
"  -v             - verbose operation (alone show version information)",
"",
"MORE DETAILS (Read As Needed):",
"",
"Syntax:",
"  The full command line syntax is:",
"",
"    zip [-shortopts ...] [--longopt ...] [zipfile path path ...] [-xi list]",
"",
"  Any number of short option and long option arguments are allowed",
"  (within limits) as well as any number of path arguments for files",
"  to zip up.  If zipfile is existing zip archive the archive is read in.",
"  If zipfile is '-', stream to stdout.  If any path is '-', zip stdin.",
"",
"Options and Values:",
"  For short options that take values use -ovalue or -o value or -o=value",
"  For long option values use either --longoption=value or --longoption value.",
"    zip -ds 10 --temp-dir=path zipfile path1 path2 --exclude pattern pattern",
"  Avoid -ovalue (no space between) to avoid confusion",
"  Be aware of 2-character options, for example",
"    -d -s is read as (delete, split size) and -ds as (dot size)",
"  Usually better to break short options across multiple arguments by function",
"    zip -r -dbdcds 10m -lilalf logfile archive input_directory -ll",
"",
"  All arguments after -- are read verbatim as paths and not options.",
"    zip zipfile path path ... -- verbatimpath verbatimpath ...",
"",
"Wildcards:",
"  Internally zip supports the following wildcards:",
"    ?       (or %% or #, depending on OS) matches any single character",
"    *       matches any number of characters, including zero",
"    [list]  matches char in list (regex), can do range [ac-f], all but [!bf]",
"  If port supports [], must escape [ as [[] or use -nw to turn off wildcards",
"  Normally * crosses dir bounds in path, e.g. 'a*b' can match 'ac/db'.  If",
"   -W option used, * does not cross dir bounds but ** does",
"  For shells that expand wildcards, escape (\\* or '*') so zip can recurse",
"    zip zipfile -r . -i '*.h'",
"",
"Include and Exclude:",
"  -i pattern pattern ...   include files that match a pattern",
"  -x pattern pattern ...   exclude files that match a pattern",    
"  Patterns are paths with optional wildcards and match paths as stored in",
"  archive.  Exclude and include lists end at next option, @, or end of line.",
"    zip -x pattern pattern @ zipfile path path ...",
"",
"End Of Line Translation:",
"  -l        translate CR or LF (depending on OS) line end to CR LF",
"  -ll       translate CR LF to CR or LF (depending on OS) line end",
"  If first buffer read from file contains binary the translation is skipped",
"",
"Recursion:",
"  -r path path ...         recurse paths, include files in subdirectories",
"  -R pattern pattern ...   recurse current directory and match patterns",
"   use -i and -x with either to include or exclude paths",
"  Path root in archive starts at current dir, so if /a/b/c/file and",
"    current dir is /a/b, 'zip -r archive *' puts c/file in archive",
"",
"Deletion:",
"  -d pattern pattern ...   delete files",
"  Delete archive entries matching internal archive paths in list",
"  Can use -t and -tt to select files in archive, but NOT -x or -i, so",
"    zip archive -d '*' -t 2005-12-27",
"  deletes all files from archive.zip with date of 27 Dec 2005 and later",
"  Note the * (escape as '*' on Unix) to select all files in archive",
"",
"Encryption:",
"  -e        use standard (weak) PKZip 2.0 encryption, prompt for password",
"  -P pswd   use standard encryption, password is pswd",
"",
"Splits (archives created as a set of split files):",
"  -s ssize  create split archive with splits of size ssize, where ssize nm",
"              n number and m multiplier (kmgt, default m), 100k -> 100 kB",
"  -sp       pause after each split closed to allow changing disks",
"      WARNING:  Archives created with -sp use data descriptors and should",
"                work with most unzips but may not work with some",
"  -sv       be verbose about creating splits",
"      Split archives CANNOT be updated, but see --out and Copy Mode below",
"",
"Using --out (output to new archive):",
"  --out oa  Instead of updating old archive, create new output archive oa",
"            Result is same as without --out but in new archive",
"      WARNING:  --out ALWAYS overwrites any existing output file",
"  Create new_archive like old_archive but add newfile1 and newfile2",
"    zip old_archive newfile1 newfile2 --out new_archive",
"  Cannot update split archive, so use --out to out new archive",
"    zip in_split_archive newfile1 newfile2 --out out_split_archive",
"  If input is split, output will default to same split size",
"  Use -s=0 or -s- to turn off splitting to convert split to single file",
"    zip in_split_archive -s 0 --out out_single_file_archive",
"      WARNING:  If overwriting old split archive but need less splits,",
"                old splits not overwritten are not needed but remain",
"",
"Copy Mode (copying from archive to archive):",
"  Copy Mode copies entries from old to new archive with --out",
"  Used when either no input files on command line or -U (--copy) used",
"  -U pattern pattern ...   select entries in archive to copy (reverse delete)",
"  For example, copy only files matching *.c into new archive, excluding foo.c",
"    zip old_archive --copy '*.c' --out new_archive -x foo.c",
"  If no input files and --out, Copy Mode assumed",
"    zip old_archive --out new_archive",
"",
"Streaming:",
"  prog1 | zip -ll z -      zip output of prog1 to zipfile z, converting CR LF",
"  zip - -R '*.c' | prog2   zip *.c files in current dir and stream to prog2 ",
"  prog1 | zip | prog2      zip in pipe with no in or out acts like zip - -",
"  If Zip is Zip64 enabled, streaming stdin to a non-seekable device creates",
"    Zip64 archives by default that need PKZip 4.5 unzipper like UnZip 6.0",
"  WARNING:  Some archives created with streaming use data descriptors and",
"            should work with most unzips but may not work with some",
"",
"Dots, counts:",
"  -db       display a running count of bytes processed and bytes to go",
"              (uncompressed size, except delete and copy show stored size)",
"  -dc       display a running count of entries done and entries to go",
"  -dd       display dots every 10 MB (or dot size) while processing files",
"  -dg       display dots globally for archive instead of for each file",
"    zip -qdgds 10m   will turn off most output except dots every 10 MB",
"  -ds siz   each dot is siz processed where siz is nm as splits (0 no dots)",
"  -du       display original uncompressed size for each entry as added",
"      Dot size is approximate, especially for dot sizes less than 1 MB",
"",
"Logging:",
"  -lf path  open file at path as logfile (overwrite existing file)",
"  -la       append to existing logfile",
"  -li       include info messages (default just warnings and errors)",
"",
"DOS Archive bit (Windows only):",
"  -AS       include only files with the DOS Archive bit set",
"  -AC       after archive created, clear archive bit of included files",
"      WARNING: Once the archive bits are cleared they are cleared",
"               Use -T to test the archive before the bits are cleared",
"               Can also use -sf to save file list before zipping files",
"",
"More option highlights (see manual for additional options and details):",
"  -b dir    when creating or updating archive, create the temp archive in",
"             dir, which allows using seekable temp file when writing to a",
"             write once CD, such archives compatible with more unzips",
"             (could require additional file copy if on another device)",
"  -MM       input patterns must match at least one file and matched files",
"             must be readable or exit with OPEN error and abort archive",
"             (without -MM, both are warnings only, and if unreadable files",
"             are skipped OPEN error (18) returned after archive created)",
"  -nw       no wildcards (wildcards are like any other character)",
"  -sc       show command line arguments as processed and exit",
"  -sf       show files that would be operated on and exit",
"  -so       show all available options on this system",
"  -t date   include files modified on this date and later",
"  -tt date  include files before date (dates are mmddyyyy or yyyy-mm-dd)",
"  -W        wildcards don't span directory boundaries in paths",
""
  };

  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i]);
    putchar('\n');
  }
#ifdef DOS
  check_for_windows("Zip");
#endif
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
#if defined(WIN32) && defined(NO_W32TIMES_IZFIX)
    "NO_W32TIMES_IZFIX",
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
#ifdef WIN32_OEM
    "WIN32_OEM",
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
#ifdef UNICODE_SUPPORT
    "UNICODE_SUPPORT",
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
#ifdef USE_ZLIB
  if (strcmp(ZLIB_VERSION, zlibVersion()) == 0)
    printf("\tUSE_ZLIB [zlib version %s]\n", ZLIB_VERSION);
  else
    printf("\tUSE_ZLIB [compiled with version %s, using version %s]\n",
      ZLIB_VERSION, zlibVersion());
  i++;  /* zlib use means there IS at least one compilation option */
#endif
#if CRYPT
  printf("\t[encryption, version %d.%d%s of %s]\n\n",
            CR_MAJORVER, CR_MINORVER, CR_BETA_VER, CR_VERSION_DATE);
  for (i = 0; i < sizeof(cryptnote)/sizeof(char *); i++)
  {
    printf(cryptnote[i]);
    putchar('\n');
  }
  ++i;  /* crypt support means there IS at least one compilation option */
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
#ifdef DOS
  check_for_windows("Zip");
#endif
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
  mesg = stderr;
  if (isatty(1))
    ziperr(ZE_PARMS, "cannot write zip file to terminal");
  if ((zipfile = malloc(4)) == NULL)
    ziperr(ZE_MEM, "was processing arguments");
  strcpy(zipfile, "-");
  /*
  if ((r = readzipfile()) != ZE_OK)
    ziperr(r, zipfile);
  */
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

   if ((zipnam = (char *)malloc(strlen(zipname) + 3)) == NULL)
     ziperr(ZE_MEM, "was creating unzip zipnam");

# ifdef MSDOS
   /* Add quotes for MSDOS.  8/11/04 */
  strcpy(zipnam, "\"");    /* accept spaces in name and path */
  strcat(zipnam, zipname);
  strcat(zipnam, "\"");
# else
  strcpy(zipnam, zipname);
# endif

  /* Here is where we need to check for the version of unzip the user
   * has.  If creating a Zip64 archive need UnZip 6 or may fail.
   */

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

       /* Also need to check unzip version here */

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
   /* remove path limit and bug when exceeded - 11/08/04 */
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
   result = system(cmd);
# ifdef VMS
   /* Convert success severity to 0, others to non-zero. */
   result = ((result & STS$M_SEVERITY) != STS$M_SUCCESS);
# endif /* def VMS */
  free(cmd);
  if (result) {
#endif /* ?((MSDOS && !__GO32__) || __human68k__) */
     fprintf(mesg, "test of %s FAILED\n", zipfile);
     ziperr(ZE_TEST, "original files unmodified");
   }
  if (noisy)
     fprintf(mesg, "test of %s OK\n", zipfile);
  if (logfile)
     fprintf(logfile, "test of %s OK\n", zipfile);
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
  if ((patterns = (struct plist *) malloc((pcount + 1) * sizeof(struct plist)))
      == NULL) {
    ZIPERR(ZE_MEM, "was creating pattern list");
  }

  for (i = 0; i < pcount && filterlist != NULL; i++) {
    switch (filterlist->flag) {
      case 'i':
        icount++;
        break;
      case 'R':
        Rcount++;
        break;
    }
    patterns[i].select = filterlist->flag;
    patterns[i].zname = filterlist->pattern;
    next = filterlist->next;
    free(filterlist);
    filterlist = next;
  }

  return pcount;
}


/* add a file argument to linked list */
local int add_name(filearg)
  char *filearg;
{
  char *name = NULL;
  struct filelist_struct *fileentry = NULL;

  if ((fileentry = (struct filelist_struct *) malloc(sizeof(struct filelist_struct))) == NULL) {
    ZIPERR(ZE_MEM, "adding file");
  }
  if ((name = malloc(strlen(filearg) + 1)) == NULL) {
    ZIPERR(ZE_MEM, "adding file");
  }
  strcpy(name, filearg);
  fileentry->next = NULL;
  fileentry->name = name;
  if (filelist == NULL) {
    /* first file argument */
    filelist = fileentry;         /* start of list */
    lastfile = fileentry;
  } else {
    lastfile->next = fileentry;   /* link to last filter in list */
    lastfile = fileentry;
  }
  filearg_count++;

  return filearg_count;
}


/* Running Stats
   10/30/04 */

local int DisplayRunningStats()
{
  char tempstrg[100];

  if (display_counts) {
    if (noisy) {
      fprintf(mesg, "%3ld/%3ld ", files_so_far, files_total - files_so_far);
      mesg_line_started = 1;
    }
    if (logall)
      fprintf(logfile, "%3ld/%3ld ", files_so_far, files_total - files_so_far);
  }
  if (display_bytes) {
    /* since file sizes can change as we go, use bytes_so_far from
       initial scan so all adds up */
    WriteNumString(bytes_so_far, tempstrg);
    if (noisy) {
      fprintf(mesg, "[%4s", tempstrg);
      mesg_line_started = 1;
    }
    if (logall)
      fprintf(logfile, "[%4s", tempstrg);
    if (bytes_total > bytes_so_far) {
      WriteNumString(bytes_total - bytes_so_far, tempstrg);
      if (noisy)
        fprintf(mesg, "/%4s] ", tempstrg);
      if (logall)
        fprintf(logfile, "/%4s] ", tempstrg);
    } else {
      WriteNumString(bytes_so_far - bytes_total, tempstrg);
      if (noisy)
        fprintf(mesg, "-%4s] ", tempstrg);
      if (logall)
        fprintf(logfile, "-%4s] ", tempstrg);
    }
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


/* rename a split
 * A split has a tempfile name until it is closed, then
 * here rename it as out_path the final name for the split.
 */
int rename_split(tempname, out_path)
  char *tempname;
  char *out_path;
{
  int r;
  /* Replace old zip file with new zip file, leaving only the new one */
  if ((r = replace(out_path, tempname)) != ZE_OK)
  {
    zipwarn("new zip file left as: ", tempname);
    free((zvoid *)tempzip);
    tempzip = NULL;
    ZIPERR(r, "was replacing split file");
  }
  if (zip_attributes) {
    setfileattr(out_path, zip_attributes);
  }
  return ZE_OK;
}


int set_filetype(out_path)
  char *out_path;
{
#ifdef __BEOS__
  /* Set the filetype of the zipfile to "application/zip" */
  setfiletype( out_path, "application/zip" );
#endif

#ifdef __ATHEOS__
  /* Set the filetype of the zipfile to "application/x-zip" */
  setfiletype(out_path, "application/x-zip");
#endif

#ifdef MACOS
  /* Set the Creator/Type of the zipfile to 'IZip' and 'ZIP ' */
  setfiletype(out_path, 'IZip', 'ZIP ');
#endif

#ifdef RISCOS
  /* Set the filetype of the zipfile to &DDC */
  setfiletype(out_path, 0xDDC);
#endif
  return ZE_OK;
}


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
       name         - short description of option which is
                        returned on some errors and when options
                        are listed with -so option, can be NULL
*/

/* Most option IDs are set to the shortopt char.  For
   multichar short options set to arbitrary unused constant. */
#define o_AC            0x101
#define o_AS            0x102
#define o_C2            0x103
#define o_C5            0x104
#define o_db            0x105
#define o_dc            0x106
#define o_dd            0x107
#define o_ds            0x108
#define o_des           0x109
#define o_df            0x110
#define o_dg            0x111
#define o_du            0x112
#define o_FF            0x113
#define o_h2            0x114
#define o_jj            0x115
#define o_la            0x116
#define o_lf            0x117
#define o_li            0x118
#define o_ll            0x119
#define o_mm            0x120
#define o_MM            0x121
#define o_nw            0x122
#define o_sc            0x123
#define o_sd            0x124
#define o_sf            0x125
#define o_so            0x126
#define o_sp            0x127
#define o_sv            0x128
#define o_tt            0x129
#define o_VV            0x130
#define o_ww            0x131
#define o_XX            0x132
#define o_z64           0x133

/* the below is mainly from the old main command line
   switch with a few changes */
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
    {"0",  "store",       o_NO_VALUE,       o_NOT_NEGATABLE, '0',  "store"},
    {"1",  "compress-1",  o_NO_VALUE,       o_NOT_NEGATABLE, '1',  "compress 1"},
    {"2",  "compress-2",  o_NO_VALUE,       o_NOT_NEGATABLE, '2',  "compress 2"},
    {"3",  "compress-3",  o_NO_VALUE,       o_NOT_NEGATABLE, '3',  "compress 3"},
    {"4",  "compress-4",  o_NO_VALUE,       o_NOT_NEGATABLE, '4',  "compress 4"},
    {"5",  "compress-5",  o_NO_VALUE,       o_NOT_NEGATABLE, '5',  "compress 5"},
    {"6",  "compress-6",  o_NO_VALUE,       o_NOT_NEGATABLE, '6',  "compress 6"},
    {"7",  "compress-7",  o_NO_VALUE,       o_NOT_NEGATABLE, '7',  "compress 7"},
    {"8",  "compress-8",  o_NO_VALUE,       o_NOT_NEGATABLE, '8',  "compress 8"},
    {"9",  "compress-9",  o_NO_VALUE,       o_NOT_NEGATABLE, '9',  "compress 9"},
    {"A",  "adjust-sfx",  o_NO_VALUE,       o_NOT_NEGATABLE, 'A',  "adjust self extractor"},
#if defined(WIN32)
    {"AC", "archive-clear", o_NO_VALUE,     o_NOT_NEGATABLE, o_AC, "clear DOS archive bit of included files"},
    {"AS", "archive-set", o_NO_VALUE,       o_NOT_NEGATABLE, o_AS, "include only files with archive bit set"},
#endif
    {"b",  "temp-path",   o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'b',  "temp dir"},
    {"c",  "entry-comments", o_NO_VALUE,    o_NOT_NEGATABLE, 'c',  "comments for each entry"},
#ifdef VMS
    {"C",  "preserve-case",   o_NO_VALUE,   o_NEGATABLE,     'C',  "Preserve (C-: down-) case all on VMS"},
    {"C2", "preserve-case-2", o_NO_VALUE,   o_NEGATABLE,     o_C2, "Preserve (C2-: down-) case ODS2 on VMS"},
    {"C5", "preserve-case-5", o_NO_VALUE,   o_NEGATABLE,     o_C5, "Preserve (C5-: down-) case ODS5 on VMS"},
#endif /* VMS */
    {"d",  "delete",      o_NO_VALUE,       o_NOT_NEGATABLE, 'd',  "delete"},
    {"db", "display-bytes", o_NO_VALUE,     o_NOT_NEGATABLE, o_db, "display running bytes"},
    {"dc", "display-counts", o_NO_VALUE,    o_NOT_NEGATABLE, o_dc, "display running file count"},
    {"dd", "display-dots", o_NO_VALUE,      o_NOT_NEGATABLE, o_dd, "display dots as process each file"},
    {"dg", "display-globaldots",o_NO_VALUE, o_NOT_NEGATABLE, o_dg, "display dots for archive instead of files"},
    {"ds", "dot-size",    o_REQUIRED_VALUE, o_NOT_NEGATABLE, o_ds, "set dot size in MB - default 10M bytes"},
    {"du", "display-usize", o_NO_VALUE,     o_NOT_NEGATABLE, o_du, "display uncompressed size in bytes"},
#ifdef MACOS
    {"df", "datafork",    o_NO_VALUE,       o_NOT_NEGATABLE, o_df, "save datafork"},
#endif /* MACOS */
    {"D",  "no-dir-entries", o_NO_VALUE,    o_NOT_NEGATABLE, 'D',  "no entries for dirs themselves (-x */)"},
    {"e",  "encrypt",     o_NO_VALUE,       o_NOT_NEGATABLE, 'e',  "encrypt entries, ask for password"},
#ifdef OS2
    {"E",  "longnames",   o_NO_VALUE,       o_NOT_NEGATABLE, 'E',  "use OS2 longnames"},
#endif
    {"F",  "fix",         o_NO_VALUE,       o_NOT_NEGATABLE, 'F',  "fix"},
    {"FF", "fixfix",      o_NO_VALUE,       o_NOT_NEGATABLE, o_FF, "fixfix"},
    {"f",  "freshen",     o_NO_VALUE,       o_NOT_NEGATABLE, 'f',  "freshen existing archive entries"},
    {"fd", "force-descriptors", o_NO_VALUE, o_NOT_NEGATABLE, o_des,"force data descriptors as if streaming"},
#ifdef ZIP64_SUPPORT
    {"fz", "force-zip64", o_NO_VALUE,       o_NOT_NEGATABLE, o_z64,"force use of Zip64 format"},
#endif
    {"g",  "grow",        o_NO_VALUE,       o_NOT_NEGATABLE, 'g',  "grow existing archive instead of replace"},
#ifndef WINDLL
    {"h",  "help",        o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"H",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"?",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'h',  "help"},
    {"h2", "more-help",   o_NO_VALUE,       o_NOT_NEGATABLE, o_h2, "extended help"},
#endif /* !WINDLL */
    {"i",  "include",     o_VALUE_LIST,     o_NOT_NEGATABLE, 'i',  "include only files matching patterns"},
#ifdef RISCOS
    {"I",  "no-image",    o_NO_VALUE,       o_NOT_NEGATABLE, 'I',  "no image"},
#endif
    {"j",  "junk-paths",  o_NO_VALUE,       o_NOT_NEGATABLE, 'j',  "strip paths and just store file names"},
#ifdef MACOS
    {"jj", "absolute-path", o_NO_VALUE,     o_NOT_NEGATABLE, o_jj, "MAC absolute path"},
#endif /* ?MACOS */
    {"J",  "junk-sfx",    o_NO_VALUE,       o_NOT_NEGATABLE, 'J',  "strip self extractor from archive"},
    {"k",  "DOS-names",   o_NO_VALUE,       o_NOT_NEGATABLE, 'k',  "force use of 8.3 DOS names"},
    {"l",  "to-crlf",     o_NO_VALUE,       o_NOT_NEGATABLE, 'l',  "convert text file line ends - LF->CRLF"},
    {"ll", "from-crlf",   o_NO_VALUE,       o_NOT_NEGATABLE, o_ll, "convert text file line ends - CRLF->LF"},
    {"lf", "logfile-path",o_REQUIRED_VALUE, o_NOT_NEGATABLE, o_lf, "log to log file at path (default overwrite)"},
    {"la", "log-append",  o_NO_VALUE,       o_NEGATABLE,     o_la, "append to existing log file"},
    {"li", "log-info",    o_NO_VALUE,       o_NEGATABLE,     o_li, "include informational messages in log"},
#ifndef WINDLL
    {"L",  "license",     o_NO_VALUE,       o_NOT_NEGATABLE, 'L',  "display licence"},
#endif
    {"m",  "move",        o_NO_VALUE,       o_NOT_NEGATABLE, 'm',  "add files to archive then delete"},
    {"mm", "",            o_NO_VALUE,       o_NOT_NEGATABLE, o_mm, "not used"},
    {"MM", "must-match",  o_NO_VALUE,       o_NOT_NEGATABLE, o_MM, "error if input not matched or not readable"},
    {"n",  "suffixes",    o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'n',  "suffixes to not compress: .gz:.zip"},
    {"nw", "no-wild",     o_NO_VALUE,       o_NOT_NEGATABLE, o_nw, "no wildcards during add or update"},
#if defined(AMIGA) || defined(MACOS)
    {"N",  "notes",       o_NO_VALUE,       o_NOT_NEGATABLE, 'N',  "add notes as entry comments"},
#endif
    {"o",  "latest-time", o_NO_VALUE,       o_NOT_NEGATABLE, 'o',  "use latest entry time as archive time"},
    {"O",  "output-file", o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'O',  "set out zip different than in zip"},
    {"p",  "paths",       o_NO_VALUE,       o_NOT_NEGATABLE, 'p',  "store paths"},
    {"P",  "password",    o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'P',  "encrypt entries, option value is password"},
#if defined(QDOS) || defined(QLZIP)
    {"Q",  "Q-flag",      o_NUMBER_VALUE,   o_NOT_NEGATABLE, 'Q',  "Q flag"},
#endif
    {"q",  "quiet",       o_NO_VALUE,       o_NOT_NEGATABLE, 'q',  "quiet"},
    {"r",  "recurse-paths", o_NO_VALUE,     o_NOT_NEGATABLE, 'r',  "recurse down listed paths"},
    {"R",  "recurse-patterns", o_NO_VALUE,  o_NOT_NEGATABLE, 'R',  "recurse current dir and match patterns"},
    {"s",  "split-size",  o_REQUIRED_VALUE, o_NOT_NEGATABLE, 's',  "do splits, set split size (-s=0 no splits)"},
    {"sp", "split-pause", o_NO_VALUE,       o_NOT_NEGATABLE, o_sp, "pause while splitting to select destination"},
    {"sv", "split-verbose", o_NO_VALUE,     o_NOT_NEGATABLE, o_sv, "be verbose about creating splits"},
    {"sc", "show-command",o_NO_VALUE,       o_NOT_NEGATABLE, o_sc, "show command line"},
    {"sd", "show-debug",  o_NO_VALUE,       o_NOT_NEGATABLE, o_sd, "show debug"},
    {"sf", "show-files",  o_NO_VALUE,       o_NOT_NEGATABLE, o_sf, "show files to operate on and exit"},
    {"so", "show-options",o_NO_VALUE,       o_NOT_NEGATABLE, o_so, "show options"},
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(ATARI)
    {"S",  "",            o_NO_VALUE,       o_NOT_NEGATABLE, 'S',  "include system and hidden"},
#endif /* MSDOS || OS2 || WIN32 || ATARI */
    {"t",  "from-date",   o_REQUIRED_VALUE, o_NOT_NEGATABLE, 't',  "exclude before date"},
    {"tt", "before-date", o_REQUIRED_VALUE, o_NOT_NEGATABLE, o_tt, "include before date"},
    {"T",  "test",        o_NO_VALUE,       o_NOT_NEGATABLE, 'T',  "test updates before replacing archive"},
    {"u",  "update",      o_NO_VALUE,       o_NOT_NEGATABLE, 'u',  "update existing entries and add new"},
    {"U",  "copy-entries", o_NO_VALUE,      o_NOT_NEGATABLE, 'U',  "select from archive instead of file system"},
    {"v",  "verbose",     o_NO_VALUE,       o_NOT_NEGATABLE, 'v',  "display additional information"},
#ifdef VMS
    {"V",  "VMS-portable", o_NO_VALUE,      o_NOT_NEGATABLE, 'V',  "Store VMS attributes, portable file format"},
    {"VV", "VMS-specific", o_NO_VALUE,      o_NOT_NEGATABLE, o_VV, "Store VMS attributes, VMS specific format"},
    {"w",  "VMS-versions", o_NO_VALUE,      o_NOT_NEGATABLE, 'w',  "store VMS versions"},
    {"ww", "VMS-dot-versions", o_NO_VALUE,  o_NOT_NEGATABLE, o_ww, "store VMS versions as \".nnn\""},
#endif /* VMS */
    {"W",  "wild-stop-dirs", o_NO_VALUE,    o_NOT_NEGATABLE, 'W',  "* stops at /, ** includes any /"},
    {"x",  "exclude",     o_VALUE_LIST,     o_NOT_NEGATABLE, 'x',  "exclude files matching patterns"},
/*    {"X",  "no-extra",    o_NO_VALUE,       o_NOT_NEGATABLE, 'X',  "no extra"},
*/
    {"X",  "extra-strip-old", o_NO_VALUE,   o_NOT_NEGATABLE, 'X',  "strip most old extra fields"},
    {"XX", "extra-strip-most", o_NO_VALUE,  o_NOT_NEGATABLE, o_XX, "strip all but critical extra fields"},
#ifdef S_IFLNK
    {"y",  "symlinks",    o_NO_VALUE,       o_NOT_NEGATABLE, 'y',  "store symbolic links"},
#endif /* S_IFLNK */
    {"z",  "archive-comment", o_NO_VALUE,   o_NOT_NEGATABLE, 'z',  "ask for archive comment"},
    {"Z",  "compression-method", o_REQUIRED_VALUE, o_NOT_NEGATABLE, 'Z', "compression method"},
#if defined(MSDOS) || defined(OS2)
    {"$",  "volume-label", o_NO_VALUE,      o_NOT_NEGATABLE, '$',  "store volume label"},
#endif
#ifndef MACOS
    {"@",  "names-stdin", o_NO_VALUE,       o_NOT_NEGATABLE, '@',  "get file names from stdin, one per line"},
#endif /* !MACOS */
#ifdef NTSD_EAS
    {"!",  "use-privileges", o_NO_VALUE,    o_NOT_NEGATABLE, '!',  "use privileges"},
#endif
#ifdef RISCOS
    {"/",  "exts-to-swap", o_REQUIRED_VALUE, o_NOT_NEGATABLE, '/',  "override Zip$Exts"},
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
  int d;                /* true if just adding to a zip file */
  char *e;              /* malloc'd comment buffer */
  struct flist far *f;  /* steps through found linked list */
  int i;                /* arg counter, root directory flag */
  int kk;               /* next arg type (formerly another re-use of "k") */

  /* zip64 support 09/05/2003 R.Nausedat */
  uzoff_t c;            /* start of central directory */
  uzoff_t t;            /* length of central directory */
  zoff_t k;             /* marked counter, comment size, entry count */
  uzoff_t n;            /* total of entry len's */

  int o;                /* true if there were any ZE_OPEN errors */
  char *p;              /* steps through option arguments */
  char *pp;             /* temporary pointer */
  ulg *cmptime = NULL;  /* pointer to 'before' or 'after' */
  int r;                /* temporary variable */
  int s;                /* flag to read names from stdin */
  uzoff_t csize;        /* compressed file size for stats */
  uzoff_t usize;        /* uncompressed file size for stats */
  ulg tf;               /* file time */
  int first_listarg = 0;/* index of first arg of "process these files" list */
  struct zlist far *v;  /* temporary variable */
  struct zlist far * far *w;    /* pointer to last link in zfiles list */
  FILE *x /*, *y */;    /* input and output zip files (y global) */
  struct zlist far *z;  /* steps through zfiles linked list */
  int bad_open_is_error = 0; /* if read fails, 0=warning, 1=error */
#ifdef WINDLL
  int retcode;          /* return code for dll */
#endif /* WINDLL */
#if (!defined(VMS) && !defined(CMS_MVS))
  char *zipbuf;         /* stdio buffer for the zip file */
#endif /* !VMS && !CMS_MVS */
  FILE *comment_stream; /* set to stderr if anything is read from stdin */

  struct filelist_struct *filearg;

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
  int key_needed = 0;   /* prompt for encryption key */
  int have_out = 0;     /* if set in_path and out_path different archive */
  int show_files = 0;   /* show files to operate on and exit */

  char **args = NULL;  /* could be wide argv */


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
  action = ADD; /* one of ADD, UPDATE, FRESHEN, DELETE, or ARCHIVE */
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
#if defined(OS2) || defined(WIN32)
  use_longname_ea = 0; /* 1=use the .LONGNAME EA as the file's name */
#endif
#ifdef NTSD_EAS
  use_privileges = 0;  /* 1=use security privileges overrides */
#endif
  hidden_files = 0;    /* process hidden and system files */
  volume_label = 0;    /* add volume label */
  dirnames = 1;        /* include directory entries by default */
#if defined(WIN32)
  only_archive_set = 0; /* only include if DOS archive bit set */
  clear_archive_bit = 0; /* clear DOS archive bit of included files */
#endif
  linkput = 0;         /* 1=store symbolic links as such */
  noisy = 1;           /* 0=quiet operation */
  extra_fields = 1;    /* 0=do not create extra fields */
  special = ".Z:.zip:.zoo:.arc:.lzh:.arj"; /* List of special suffixes */
  key = NULL;          /* Scramble password if scrambling */
  key_needed = 0;      /* Need scramble password */
  tempath = NULL;      /* Path for temporary files */
  found = NULL;        /* where in found, or new found entry */
  fnxt = &found;
  patterns = NULL;     /* List of patterns to be matched */
  pcount = 0;          /* number of patterns */
  icount = 0;          /* number of include only patterns */
  Rcount = 0;          /* number of -R include patterns */
  /* status 10/30/04 */
  files_so_far = 0;    /* count of files processed so far */
  bad_files_so_far = 0;/* files skipped so far */
  files_total = 0;     /* files to get through */
  bytes_so_far = 0;    /* bytes processed so far */
  good_bytes_so_far = 0;/* bytes read so far */
  bad_bytes_so_far = 0;/* bytes skipped so far */
  bytes_total = 0;     /* bytes to get through */
  /* display 5/15/05 */
  display_bytes = 0;   /* display running bytes */
  display_counts = 0;  /* display running counts */
  display_usize = 0;   /* display uncompressed size */

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

  zipfile = NULL;      /* path of usual in and out zipfile */
  in_file = NULL;      /* current input file for splits */
  in_split_path = NULL; /* current in split path */
  in_path = NULL;      /* used by splits to track changing split locations */
  out_path = NULL;     /* if set, use -O out_path as output */
  have_out = 0;        /* if set, in_path and out_path not the same archive */

#ifdef UNICODE_SUPPORT
  use_wide_to_mb_default = 0;
#endif

#ifdef ZIP64_SUPPORT
  force_zip64 = 0;     /* if 1 force entries to be zip64 */
  zip64_entry = 0;     /* current entry needs Zip64 */
  zip64_archive = 0;   /* if 1 then at least 1 entry needs zip64 */
#endif

  current_disk = 0;              /* current disk number */
  cd_start_disk = -1;            /* central directory start disk */
  cd_start_offset = 0;           /* offset of start of cd on cd start disk */
  cd_entries_this_disk = 0;      /* cd entries this disk */
  total_cd_entries = 0;          /* total cd entries */
  zip64_eocd_disk = 0;           /* disk with Zip64 End Of Central Directory Record */
  zip64_eocd_offset = 0;         /* offset for Zip64 EOCD Record */

  /* for split method 1 (keep split with local header open and update) */
  current_local_tempname = NULL; /* name of temp file */
  current_local_file = NULL;     /* file pointer for current local header */
  current_local_offset = 0;      /* offset to start of current local header */

  /* global */
  bytes_this_split = 0;          /* bytes written to the current split */
  read_split_archive = 0;        /* 1=scanzipf_reg detected spanning signature */
  split_method = 0;              /* 0=no splits, 1=update LHs, 2=data descriptors */
  split_size = 0;                /* how big each split should be */
  bytes_prev_splits = 0;         /* total bytes written to all splits before this */
  bytes_this_entry = 0;          /* bytes written for this entry across all splits */
  noisy_splits = 0;              /* be verbose about creating splits */
  mesg_line_started = 0;         /* 1=started writing a line to mesg */

  filelist = NULL;
  filearg_count = 0;
  allow_empty_archive = 0;       /* if no files, allow creation of empty archive anyway */
  bad_open_is_error = 0;         /* if read fails, 0=warning, 1=error */
  show_files = 0;                /* show files to operate on and exit */


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
  /* test if we can support large files - 9/29/04 */
  if (sizeof(zoff_t) < 8) {
    ZIPERR(ZE_COMPERR, "LARGE_FILE_SUPPORT enabled but OS not supporting it");
  }
#endif
  /* test if sizes are the same - 12/30/04 */
  if (sizeof(uzoff_t) != sizeof(zoff_t)){
    ZIPERR(ZE_COMPERR, "uzoff_t not same size as zoff_t");
  }

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

  /*    Substitutes the extended command line argument list produced by
   *    the MKS Korn Shell in place of the command line info from DOS.
   */

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
    EXIT(ZE_OK);
  }
  else if (argc == 2 && strcmp(argv[1], "-v") == 0 &&
           /* only "-v" as argument, and */
           (isatty(1) || isatty(0)))
           /* stdout or stdin is connected to console device */
  {                             /* show diagnostic version info */
    version_info();
    EXIT(ZE_OK);
  }
# ifndef VMS
#   ifndef RISCOS
  envargs(&argc, &argv, "ZIPOPT", "ZIP");  /* get options from environment */
#   else /* RISCOS */
  envargs(&argc, &argv, "ZIPOPT", "Zip$Options");  /* get options from environment */
  getRISCOSexts("Zip$Exts");        /* get the extensions to swap from environment */
#   endif /* ? RISCOS */
# else /* VMS */
  envargs(&argc, &argv, "ZIPOPT", "ZIP_OPTS");  /* 4th arg for unzip compat. */
# endif /* ?VMS */
#endif /* !WINDLL */

  zipfile = tempzip = NULL;
  y = NULL;
  d = 0;                        /* disallow adding to a zip file */
#if (!defined(MACOS) && !defined(WINDLL) && !defined(NLM))
  signal(SIGINT, handler);
#ifdef SIGTERM                  /* AMIGADOS and others have no SIGTERM */
  signal(SIGTERM, handler);
#endif
# if defined(SIGABRT) && !(defined(AMIGA) && defined(__SASC))
   signal(SIGABRT, handler);
# endif
# ifdef SIGBREAK
   signal(SIGBREAK, handler);
# endif
# ifdef SIGBUS
   signal(SIGBUS, handler);
# endif
# ifdef SIGILL
   signal(SIGILL, handler);
# endif
# ifdef SIGSEGV
   signal(SIGSEGV, handler);
# endif
#endif /* !MACOS && !WINDLL && !NLM */
#ifdef NLM
  NLMsignals();
#endif
  /* make copy of args that can use with insert_arg() */
  args = copy_args(argv, 0);

  kk = 0;                       /* Next non-option argument type */
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
         args    - usually same as argv if no argument file support
         argcnt  - current argc for args
         value   - char* to value (free() when done with it) or NULL if no value
         negated - option was negated with trailing -
  */

  while ((option = get_option(&args, &argcnt, &argnum,
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
#if defined(WIN32)
        case o_AC:
          clear_archive_bits = 1; break;
        case o_AS:
          only_archive_set = 1; break;
#endif /* MSDOS || OS2 || WIN32 */
        case 'b':   /* Specify path for temporary file */
          tempdir = 1;
          tempath = value;
          break;
        case 'c':   /* Add comments for new files in zip file */
          comadd = 1;  break;

        /* -C, -C2, and -C5 are with -V */

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
        case o_dd:
          /* display dots */
          display_globaldots = 0;
          /* set default dot size if dot_size not set (dot_count = 0) */
          if (dot_count == 0)
            /* default to 10 MB */
            dot_size = 10 * 0x100000;
          dot_count = -1;
          break;
        case o_dg:
          /* display dots globally for archive instead of for each file */
          display_globaldots = 1;
          /* set default dot size if dot_size not set (dot_count = 0) */
          if (dot_count == 0)
            dot_size = 10 * 0x100000;
          dot_count = -1;
          break;
        case o_ds:
          /* input dot_size is now actual dot size to account for different buffer sizes */
          if (value == NULL)
            dot_size = 10 * 0x100000;
          else if (value[0] == '\0') {
            /* default to 10 MB */
            dot_size = 10 * 0x100000;
            free(value);
          } else {
            dot_size = ReadNumString(value);
            if (dot_size == (uzoff_t)-1) {
              sprintf(errbuf, "option -ds (--dot-size) has bad size:  '%s'", value);
              free(value);
              ZIPERR(ZE_PARMS, errbuf);
            }
            if (dot_size < 0x400) {
              /* < 1 KB there is no multiplier, assume MB */
              dot_size *= 0x100000;

            } else if (dot_size < 0x400 * 32) {
              /* 1K <= dot_size < 32K */
              sprintf(errbuf, "dot size must be at least 32 KB:  '%s'", value);
              free(value);
              ZIPERR(ZE_PARMS, errbuf);

            } else {
              /* 32K <= dot_size */
            }
            free(value);
          }
          dot_count = -1;
          break;
        case o_du:
          display_usize = 1;
          break;
        case 'D':   /* Do not add directory entries */
          dirnames = 0; break;
        case 'e':   /* Encrypt */
#if !CRYPT
          ZIPERR(ZE_PARMS, "encryption not supported");
#else /* CRYPT */
          if (key)
            free(key);
          key_needed = 1;
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

        /* -i is with -x */
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
        case o_lf:
          /* open a logfile */
          /* allow multiple use of option but only last used */
          if (logfile_path) {
            free(logfile_path);
          }
          logfile_path = value;
          break;
        case o_la:
          /* append to existing logfile */
          if (negated)
            logfile_append = 0;
          else
            logfile_append = 1;
          break;
        case o_li:
          /* log all including informational messages */
          if (negated)
            logall = 0;
          else
            logall = 1;
          break;
#ifndef WINDLL
        case 'L':   /* Show license */
          license();
          RETURN(finish(ZE_OK));
#endif
        case 'm':   /* Delete files added or updated in zip file */
          dispose = 1;  break;
        case o_mm:  /* To prevent use of -mm for -MM */
          ZIPERR(ZE_PARMS, "-mm not supported, Must_Match is -MM");
          dispose = 1;  break;
        case o_MM:  /* Exit with error if input file can't be read */
          bad_open_is_error = 1; break;
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
        case 'O':   /* Set output file different than input archive */
          out_path = ziptyp(value);
          free(value);
          have_out = 1;
          break;
        case 'p':   /* Store path with name */
          break;            /* (do nothing as annoyance avoidance) */
        case 'P':   /* password for encryption */
          if (key != NULL) {
            free(key);
          }
#if CRYPT
          key = value;
          key_needed = 0;
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
        case o_sf:  /* show files to operate on */
          show_files = 1; break;
        case o_so:  /* show all options */
          show_options = 1; break;

        case 's':   /* enable split archives */
          /* get the split size from value */
          if (strcmp(value, "-") == 0) {
            /* -s- do not allow splits */
            split_method = -1;
          } else {
            split_size = ReadNumString(value);
            if (split_size == (uzoff_t)-1) {
              sprintf(errbuf, "bad split size:  '%s'", value);
              ZIPERR(ZE_PARMS, errbuf);
            }
            if (split_size == 0) {
              /* do not allow splits */
              split_method = -1;
            } else {
              if (split_method == 0) {
                split_method = 1;
              }
              if (split_size < 0x400) {
                /* < 1 KB there is no multiplier, assume MB */
                split_size *= 0x100000;
              }
              /* By setting the minimum split size to 64 KB we avoid
                 not having enough room to write a header unsplit
                 which is required */
              if (split_size < 0x400 * 64) {
                /* split_size < 64K */
                sprintf(errbuf, "minimum split size is 64 KB:  '%s'", value);
                free(value);
                ZIPERR(ZE_PARMS, errbuf);
              }
            }
          }
          free(value);
          break;
        case o_sp:  /* enable split select - pause splitting between splits */
          use_descriptors = 1;
          split_method = 2;
          break;
        case o_sv:  /* be verbose about creating splits */
          noisy_splits = 1;
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
        case o_tt:  /* Exclude files at or after specified date */
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
        case 'U':   /* Select archive entries to keep or operate on */
          if (action != ADD) {
            ZIPERR(ZE_PARMS, "specify just one action");
          }
          action = ARCHIVE;
          break;
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
        case 'C':  /* Preserve case (- = down-case) all. */
          if (negated)
          { /* Down-case all. */
            if ((vms_case_2 > 0) || (vms_case_5 > 0))
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C-)");
            }
            vms_case_2 = -1;
            vms_case_5 = -1;
          }
          else
          { /* Not negated.  Preserve all. */
            if ((vms_case_2 < 0) || (vms_case_5 < 0))
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C)");
            }
            vms_case_2 = 1;
            vms_case_5 = 1;
          }
          break;
        case o_C2:  /* Preserve case (- = down-case) ODS2. */
          if (negated)
          { /* Down-case ODS2. */
            if (vms_case_2 > 0)
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C2-)");
            }
            vms_case_2 = -1;
          }
          else
          { /* Not negated.  Preserve ODS2. */
            if (vms_case_2 < 0)
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C2)");
            }
            vms_case_2 = 1;
          }
          break;
        case o_C5:  /* Preserve case (- = down-case) ODS5. */
          if (negated)
          { /* Down-case ODS5. */
            if (vms_case_5 > 0)
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C5-)");
            }
            vms_case_5 = -1;
          }
          else
          { /* Not negated.  Preserve ODS5. */
            if (vms_case_5 < 0)
            {
              ZIPERR( ZE_PARMS, "Conflicting case directives (-C5)");
            }
            vms_case_5 = 1;
          }
          break;
        case 'V':   /* Store in VMS format.  (Record multiples.) */
          vms_native = 1; break;
          /* below does work with new parser but doesn't allow tracking
             -VV separately, like adding a separate description */
          /* vms_native++; break; */
        case o_VV:  /* Store in VMS specific format */
          vms_native = 2; break;
        case 'w':   /* Append the VMS version number */
          vmsver |= 1;  break;
        case o_ww:   /* Append the VMS version number as ".nnn". */
          vmsver |= 3;  break;
#endif /* VMS */
        case 'W':   /* Wildcards do not include directory boundaries in matches */
          wild_stop_at_dir = 1;
          break;

        case 'i':   /* Include only the following files */
          /* if nothing matches include list then still create an empty archive */
          allow_empty_archive = 1;
        case 'x':   /* Exclude following files */
          add_filter((int) option, value);
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
          s = 1;          /* defer -@ until have zipfile name */
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
#ifdef NTSD_EAS
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
        case o_des:
          use_descriptors = 1;
          break;

#ifdef ZIP64_SUPPORT
        case o_z64:   /* Force creation of Zip64 entries */
          force_zip64 = 1; break;
#endif

        case o_NON_OPTION_ARG:
          /* not an option */
          /* no more options as permuting */
          /* just dash also ends up here */

          if (recurse != 2 && kk == 0 && patterns == NULL) {
            /* have all filters so convert filterlist to patterns array
               as PROCNAME needs patterns array */
            filterlist_to_patterns();
          }

          /* "--" stops arg processing for remaining args - 7/25/04 */
          /*
          printf("value = '%s' kk=%d\n", value, kk);
          */
          /* ignore only first -- */
          if (strcmp(value, "--") == 0 && seen_doubledash == 0) {
            /* -- */
            seen_doubledash = 1;
            if (kk == 0) {
              ZIPERR(ZE_PARMS, "can't use -- before archive name");
            }

            /* just ignore as just marks what follows as non-option arguments */

          } else if (kk == 6) {
            /* value is R pattern */
            add_filter((int)'R', value);
            free(value);
            if (first_listarg == 0) {
              first_listarg = argnum;
            }
          } else switch (kk)
          {
            case 0:
              if (show_what_doing) {
                fprintf(mesg, "zipfile name '%s'\n", value);
                fflush(mesg);
              }
              /* first non-option arg is zipfile name */
#if (!defined(MACOS) && !defined(WINDLL))
              if (strcmp(value, "-") == 0) {  /* output zipfile is dash */
                /* just a dash */
                zipstdout();
              } else
#endif /* !MACOS && !WINDLL */
              {
                /* name of zipfile */
                if ((zipfile = ziptyp(value)) == NULL) {
                  ZIPERR(ZE_MEM, "was processing arguments");
                }
                /* read zipfile if exists */
                /*
                if ((r = readzipfile()) != ZE_OK) {
                  ZIPERR(r, zipfile);
                }
                */
                free(value);
              }
              /* if in_path not set, use zipfile path as usual for input */
              /* in_path is used as the base path to find splits */
              if (in_path == NULL) {
                if ((in_path = malloc(strlen(zipfile) + 1)) == NULL) {
                  ZIPERR(ZE_MEM, "was processing arguments");
                }
                strcpy(in_path, zipfile);
              }
              /* if out_path not set, use zipfile path as usual for output */
              /* out_path is where the output archive is written */
              if (out_path == NULL) {
                if ((out_path = malloc(strlen(zipfile) + 1)) == NULL) {
                  ZIPERR(ZE_MEM, "was processing arguments");
                }
                strcpy(out_path, zipfile);
              }
              kk = 3;
              if (s)
              {
                /* do -@ and get names from stdin */
                /* should be able to read names from
                   stdin and output to stdout, but
                   this was not allowed in old code.
                   This check moved to kk = 3 case to fix. */
                /* if (strcmp(zipfile, "-") == 0) {
                  ZIPERR(ZE_PARMS, "can't use - and -@ together");
                }
                */
                while ((pp = getnam(stdin)) != NULL)
                {
                  kk = 4;
                  if (recurse == 2) {
                    /* reading patterns from stdin */
                    add_filter((int)'R', pp);
                  } else {
                    /* file argument now processed later */
                    add_name(pp);
                  }
                  /*
                  if ((r = PROCNAME(pp)) != ZE_OK) {
                    if (r == ZE_MISS)
                      zipwarn("name not matched: ", pp);
                    else {
                      ZIPERR(r, pp);
                    }
                  }
                  */
                  free(pp);
                }
                s = 0;
              }
              if (recurse == 2) {
                /* rest are -R patterns */
                kk = 6;
              }
              break;

            case 3:  case 4:
              /* no recurse and -r file names */
              /* can't read filenames -@ and input - from stdin at
                 same time */
              if (s == 1 && strcmp(value, "-") == 0) {
                ZIPERR(ZE_PARMS, "can't read input (-) and filenames (-@) both from stdin");
              }
              /* add name to list for later processing */
              add_name(value);
              /*
              if ((r = PROCNAME(value)) != ZE_OK) {
                if (r == ZE_MISS)
                  zipwarn("name not matched: ", value);
                else {
                  ZIPERR(r, value);
                }
              }
              */
              if (kk == 3) {
                first_listarg = argnum;
                kk = 4;
              }
              break;

            } /* switch kk */
            break;

        default:
          /* should never get here as get_option will exit if not in table */
          sprintf(errbuf, "no such option ID: %ld", option);
          ZIPERR(ZE_PARMS, errbuf);

     }  /* switch */
  }


  /* do processing of command line and one-time tasks */

  /* Key not yet specified.  If needed, get/verify it now. */
  if (key_needed) {
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
  if (key) {
    /* if -P "" could get here */
    if (*key == '\0') {
      ZIPERR(ZE_PARMS, "zero length password not allowed");
    }
  }

  if (show_what_doing) {
    fprintf(mesg, "Command line read\n");
    fflush(mesg);
  }

  /* show command line args */
  if (show_args) {
    fprintf(mesg, "command line:\n");
    for (i = 0; i < argcnt; i++) {
      fprintf(mesg, "'%s'  ", argv[i]);
    }
    fprintf(mesg, "\n");
    ZIPERR(ZE_ABORT, "show command line");
  }

  /* show all options */
  if (show_options) {
    printf("available options:\n");
    printf(" %-2s  %-18s %-4s %-3s %-30s\n", "sh", "long", "val", "neg", "description");
    printf(" %-2s  %-18s %-4s %-3s %-30s\n", "--", "----", "---", "---", "-----------");
    for (i = 0; options[i].option_ID; i++) {
      printf(" %-2s  %-18s ", options[i].shortopt, options[i].longopt);
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

  
  /* open log file */
  if (logfile_path) {
    char mode[10];
    char *p;
    char *lastp;

    /* if no extension add .log */
    p = logfile_path;
    /* find last / */
    lastp = NULL;
    for (p = logfile_path; (p = MBSRCHR(p, '/')) != NULL; p++) {
      lastp = p;
    }
    if (lastp == NULL)
      lastp = logfile_path;
    if (MBSRCHR(lastp, '.') == NULL) {
      /* add .log */
      if ((p = malloc(strlen(logfile_path) + 5)) == NULL) {
        ZIPERR(ZE_MEM, "logpath");
      }
      strcpy(p, logfile_path);
      strcat(p, ".log");
      free(logfile_path);
      logfile_path = p;
    }

    if (logfile_append) {
      sprintf(mode, "a");
    } else {
      sprintf(mode, "w");
    }
    if ((logfile = zfopen(logfile_path, mode)) == NULL) {
      sprintf(errbuf, "could not open logfile '%s'", logfile_path);
      ZIPERR(ZE_PARMS, errbuf);
    }
    {
      /* At top put start time and command line */

      /* get current time */
      struct tm *now;
      time_t clocktime;

      time(&clocktime);
      now = localtime(&clocktime);

      fprintf(logfile, "---------\n");
      fprintf(logfile, "Zip log opened %s", asctime(now));
      fprintf(logfile, "command line arguments:\n ");
      for (i = 1; i < argcnt; i++) {
        fprintf(logfile, "%s ", argv[i]);
      }
      fprintf(logfile, "\n\n");
    }
  } else {
    /* only set logall if logfile open */
    logall = 0;
  }


  if (split_method && out_path) {
    /* if splitting, the archive name must have .zip extension */
    int plen = strlen(out_path);
    char *out_path_ext;

#ifdef VMS
    /* On VMS, adjust plen (and out_path_ext) to avoid the file version. */
    plen -= strlen( vms_file_version( out_path));
#endif /* def VMS */
    out_path_ext = out_path+ plen- 4;

    if (plen < 4 ||
        out_path_ext[0] != '.' ||
        toupper(out_path_ext[1]) != 'Z' ||
        toupper(out_path_ext[2]) != 'I' ||
        toupper(out_path_ext[3]) != 'P') {
      ZIPERR(ZE_PARMS, "archive name must end in .zip for splits");
    }
  }


  if (verbose && (dot_size == 0) && (dot_count == 0)) {
    /* show all dots as before if verbose set and dot_size not set (dot_count = 0) */
    dot_size = -1;
  }

  /* done getting -R filters so convert filterlist if not done */
  if (pcount && patterns == NULL) {
    filterlist_to_patterns();
  }

#if (defined(MSDOS) || defined(OS2)) && !defined(WIN32)
  if ((kk == 3 || kk == 4) && volume_label == 1) {
    /* read volume label */
    PROCNAME(NULL);
    kk = 4;
  }
#endif

  if (have_out && kk == 3) {
    copy_only = 1;
    action = ARCHIVE;
  }

  if (action != ARCHIVE && (recurse == 2 || pcount) && first_listarg == 0 &&
      (kk < 3 || (action != UPDATE && action != FRESHEN))) {
    ZIPERR(ZE_PARMS, "nothing to select from");
  }
  
/*
  -------------------------------------
  end of new command line code
  -------------------------------------
*/

#if (!defined(MACOS) && !defined(WINDLL))
  if (kk < 3) {               /* zip used as filter */
    zipstdout();
    comment_stream = NULL;
    if ((r = procname("-", 0)) != ZE_OK) {
      if (r == ZE_MISS)
        if (bad_open_is_error) {
          zipwarn("name not matched: ", "-");
          ZIPERR(ZE_OPEN, "-");
        } else {
          zipwarn("name not matched: ", "-");
        }
      else {
        ZIPERR(r, "-");
      }
    }
    kk = 4;
    if (s) {
      ZIPERR(ZE_PARMS, "can't use - and -@ together");
    }
  }
#endif /* !MACOS && !WINDLL */

  if (zipfile && !strcmp(zipfile, "-")) {
    if (show_what_doing) {
      printf("zipping to stdout\n");
    }
    zip_to_stdout = 1;
  }

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
  if (action == ARCHIVE && (method != BEST || dispose || recurse ||
      comadd || zipedit)) {
    zipwarn("can't set method, move, recurse, or comments with copy mode.","");
    /* reset flags - needed? */
    method  = BEST;
    dispose = 0;
    recurse = 0;
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
  if (test && zip_to_stdout) {
    test = 0;
    zipwarn("can't use -T on stdout, -T ignored", "");
  }
  if (split_method && (fix || adjust)) {
    ZIPERR(ZE_PARMS, "can't create split archive while fixing\n");
  }
  if (split_method && (d || zip_to_stdout)) {
    ZIPERR(ZE_PARMS, "can't create split archive with -d or -g or on stdout\n");
  }
  if ((action != ADD || d) && zip_to_stdout) {
    ZIPERR(ZE_PARMS, "can't use -d,-f,-u, -U, or -g on stdout\n");
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

  if (noisy_splits && split_size > 0)
    zipmessage("splitsize = ", zip_fuzofft(split_size, NULL, NULL));

  /* Read old archive */

  /* Now read the zip file here instead of when doing args above */
  /* Only read the central directory and build zlist */
  if (show_what_doing) {
    fprintf(mesg, "Reading archive\n");
    fflush(mesg);
  }
  /* read zipfile if exists */
  if ((r = readzipfile()) != ZE_OK) {
    ZIPERR(r, zipfile);
  }

  if (zcount == 0 && (action != ADD || d)) {
    zipwarn(zipfile, " not found or empty");
  }

  if (have_out && kk == 3) {
    /* no input paths so assume copy mode and match everything if --out */
    for (z = zfiles; z != NULL; z = z->nxt) {
      z->mark = pcount ? filter(z->zname, 0) : 1;
    }
  }

  /* Scan for new files */

  /* Process file arguments from command line */
  if (filelist) {
    if (action == ARCHIVE) {
      /* find in archive */
      if (show_what_doing) {
        fprintf(mesg, "Scanning archive entries\n");
        fflush(mesg);
      }
      for (; filelist; ) {
        if ((r = proc_archive_name(filelist->name, 1)) != ZE_OK) {
          if (r == ZE_MISS)
            zipwarn("name not matched: ", filelist->name);
          else {
            ZIPERR(r, filelist->name);
          }
        }
        free(filelist->name);
        filearg = filelist;
        filelist = filelist->next;
        free(filearg);
      }
    } else {
      /* try find files then find in archive */
      if (show_what_doing) {
        fprintf(mesg, "Scanning files\n");
        fflush(mesg);
      }
      for (; filelist; ) {
        if ((r = PROCNAME(filelist->name)) != ZE_OK) {
          if (r == ZE_MISS)
            if (bad_open_is_error) {
              zipwarn("name not matched: ", filelist->name);
              ZIPERR(ZE_OPEN, filelist->name);
            } else {
              zipwarn("name not matched: ", filelist->name);
            }
          else {
            ZIPERR(r, filelist->name);
          }
        }
        free(filelist->name);
        filearg = filelist;
        filelist = filelist->next;
        free(filearg);
      }
    }
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
        if (bad_open_is_error) {
          zipwarn("name not matched: ", "current directory for -R");
          ZIPERR(ZE_OPEN, "-R");
        } else {
          zipwarn("name not matched: ", "current directory for -R");
        }
      else {
        ZIPERR(r, "-R");
      }
    }
  }


  if (show_what_doing) {
    fprintf(mesg, "Apply filters\n");
    fflush(mesg);
  }
  /* Clean up selections ("3 <= kk <= 5" now) */
  if (kk != 4 && first_listarg == 0 &&
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
    fprintf(mesg, "Check dups\n");
    fflush(mesg);
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
    if ((tempath = (char *)malloc((int)(p - zipfile) + 1)) == NULL) {
      ZIPERR(ZE_MEM, "was processing arguments");
    }
    r = *p;  *p = 0;
    strcpy(tempath, zipfile);
    *p = (char)r;
  }
#endif /* VM_CMS */

#if (defined(IZ_CHECK_TZ) && defined(USE_EF_UT_TIME))
  if (!zp_tz_is_valid) {
    zipwarn("TZ environment variable not found, cannot use UTC times!!","");
  }
#endif /* IZ_CHECK_TZ && USE_EF_UT_TIME */

  /* For each marked entry, if not deleting, check if it exists, and if
     updating or freshening, compare date with entry in old zip file.
     Unmark if it doesn't exist or is too old, else update marked count. */
  if (show_what_doing) {
    fprintf(mesg, "Scanning files to update and add\n");
    fflush(mesg);
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
      ulg z_tim;
#endif /* USE_EF_UT_TIME */
      Trace((stderr, "zip diagnostics: marked file=%s\n", z->oname));

      csize = z->siz;
      usize = z->len;
      if (action == DELETE) {
        /* only delete files in date range */
#ifdef USE_EF_UT_TIME
        z_tim = (get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
                unix2dostime(&z_utim.mtime) : z->tim;
#else /* !USE_EF_UT_TIME */
#       define z_tim  z->tim
#endif /* ?USE_EF_UT_TIME */
        if (z_tim < before || (after && z_tim >= after)) {
          /* include in archive */
          z->mark = 0;
        } else {
          /* delete file */
          files_total++;
          /* ignore len in old archive and update to current size */
          z->len = usize;
          if (csize < (uzoff_t) -1)
            bytes_total += csize;
          k++;
        }
      } else if (action == ARCHIVE) {
        /* only keep files in date range */
#ifdef USE_EF_UT_TIME
        z_tim = (get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
                unix2dostime(&z_utim.mtime) : z->tim;
#else /* !USE_EF_UT_TIME */
#       define z_tim  z->tim
#endif /* ?USE_EF_UT_TIME */
        if (z_tim < before || (after && z_tim >= after)) {
          /* exclude from archive */
          z->mark = 0;
        } else {
          /* keep file */
          files_total++;
          /* ignore len in old archive and update to current size */
          z->len = usize;
          if (csize < (uzoff_t) -1)
            bytes_total += csize;
          k++;
        }
      } else if (
#ifdef USE_EF_UT_TIME
          ((tf = filetime(z->name, (ulg *)NULL, (zoff_t *)&usize, &f_utim))
#else /* !USE_EF_UT_TIME */
          ((tf = filetime(z->name, (ulg *)NULL, (zoff_t *)&usize, NULL))
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
        if (verbose)
          fprintf(mesg, "zip diagnostic: %s %s\n", z->oname,
                 z->trash ? "up to date" : "missing or early");
        if (logfile)
          fprintf(logfile, "zip diagnostic: %s %s\n", z->oname,
                 z->trash ? "up to date" : "missing or early");
      }
      else {
        files_total++;
        /* ignore len in old archive and update to current size */
        z->len = usize;
        if (usize < (uzoff_t) -1)
          bytes_total += usize;
        k++;
      }
    }

  /* Remove entries from found list that do not exist or are too old */
  if (show_what_doing) {
    fprintf(mesg, "fcount = %u\n", (unsigned)fcount);
    fflush(mesg);
  }
  diag("stating new entries");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL;) {
    Trace((stderr, "zip diagnostic: new file=%s\n", f->oname));
    if (action == DELETE || action == FRESHEN ||
        (tf = filetime(f->name, (ulg *)NULL, (zoff_t *)&usize, NULL)) == 0 ||
        tf < before || (after && tf >= after) ||
        (namecmp(f->zname, zipfile) == 0 && !zip_to_stdout)
       )
      f = fexpel(f);
    else {
      /* ??? */
      files_total++;
      f->usize = 0;
      if (usize < (uzoff_t) -1) {
        bytes_total += usize;
        f->usize = usize;
      }
      f = f->nxt;
    }
  }
#ifdef MACOS
  PrintStatProgress("done");
#endif

  if (show_files) {
    uzoff_t count = 0;

    if (action == DELETE)
      fprintf(stdout, "Would Delete:\n");
    else if (action == FRESHEN)
      fprintf(stdout, "Would Freshen:\n");
    else if (action == ARCHIVE)
      fprintf(stdout, "Would Copy:\n");
    else
      fprintf(stdout, "Would Add/Update:\n");

    for (z = zfiles; z != NULL; z = z->nxt) {
      if (z->mark) {
        count++;
        fprintf(stdout, "  %s\n", z->name);
      }
    }
    fprintf(stdout, "Total %s entries (%s bytes)\n",
                                          zip_fuzofft(count, NULL, NULL),
                                          zip_fuzofft(bytes_total, NULL, NULL));
    RETURN(finish(ZE_OK));
  }
  
  /* Make sure there's something left to do */
  if (k == 0 && found == NULL && !(zfiles == NULL && allow_empty_archive) &&
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
    else {
      ZIPERR(ZE_NONE, zipfile);
    }
#endif /* !WINDLL */
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

  /* Just ignore the spanning signature if a multi-disk archive */
  if (zfiles && total_disks != 1 && zipbeg == 4) {
    zipbeg = 0;
  }

  /* Before we get carried away, make sure zip file is writeable. This
   * has the undesired side effect of leaving one empty junk file on a WORM,
   * so when the zipfile does not exist already and when -b is specified,
   * the writability check is made in replace().
   */
  if (strcmp(zipfile, "-"))
  {
    if (tempdir && zfiles == NULL && zipbeg == 0) {
      zip_attributes = 0;
    } else {
      x = (have_out || (zfiles == NULL && zipbeg == 0)) ? zfopen(out_path, FOPW) :
                                                          zfopen(out_path, FOPM);
      /* Note: FOPW and FOPM expand to several parameters for VMS */
      if (x == NULL) {
        ZIPERR(ZE_CREAT, out_path);
      }
      fclose(x);
      zip_attributes = getfileattr(out_path);
      if (zfiles == NULL && zipbeg == 0)
        destroy(out_path);
    }
  }
  else
    zip_attributes = 0;

  /* Throw away the garbage in front of the zip file for -J */
  if (junk_sfx) zipbeg = 0;

  /* Open zip file and temporary output file */
  if (show_what_doing) {
    fprintf(mesg, "open zip file and create temp file\n");
    fflush(mesg);
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
    y = zfdopen(fileno(stdout), FOPW);
#else
    y = stdout;
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
    /*
    tempzf = y;
    */

    if (zfseeko(y, cenbeg, SEEK_SET)) {
      ZIPERR(ferror(y) ? ZE_READ : ZE_EOF, zipfile);
    }
    tempzn = cenbeg;
  }
  else
  {
    if (show_what_doing) {
      printf("creating new zip file\n");
    }
    /* See if there is something at beginning of disk 1 to copy.
       If not, do nothing as zipcopy() will open files to read
       as needed. */
    if (zipbeg) {
      in_split_path = get_in_split_path(in_path, 1);

      while ((in_file = zfopen(in_split_path, FOPR_EX)) == NULL) {
        /* could not open split */

        /* Ask for directory with split.  Updates in_path */
        if (ask_for_split_read_path(1) == 0) {
          ZIPERR(ZE_ABORT, "could not open archive to read");
        }
        free(in_split_path);
        in_split_path = get_in_split_path(in_path, 1);
      }
    }
    if ((tempzip = tempname(zipfile)) == NULL) {
      ZIPERR(ZE_MEM, "allocating temp filename");
    }
    if ((y = zfopen(tempzip, FOPW_TMP)) == NULL) {
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

  /* If not seekable set some flags 3/14/05 EG */
  output_seekable = 1;
  if (!is_seekable(y)) {
    output_seekable = 0;
    use_descriptors = 1;
  }

  /* Not needed.  Only need Zip64 when input file is larger than 2 GB or reading
     stdin.  This is set in putlocal() for each file.  12/28/05 EG */
#if 0
  /* If using descriptors and Zip64 enabled force Zip64 3/13/05 EG */
#ifdef ZIP64_SUPPORT
  if (use_descriptors) {
    force_zip64 = 1;
  }
#endif
#endif

  /* if archive exists, not streaming and not deleting or growing, copy
     any bytes at beginning */
  if (strcmp(zipfile, "-") != 0 && !d)  /* this must go *after* set[v]buf */
  {
    /* copy anything before archive */
    if (in_file && zipbeg && (r = bfcopy(zipbeg)) != ZE_OK) {
      ZIPERR(r, r == ZE_TEMP ? tempzip : zipfile);
    }
    if (in_file) {
      fclose(in_file);
      in_file = NULL;
      free(in_split_path);
    }
    tempzn = zipbeg;
    if (split_method) {
      /* add spanning signature */
      if (show_what_doing) {
        printf("adding spanning/splitting signature at top of archive\n");
      }
      /* write the spanning signature at the top of the archive */
      errbuf[0] = 'P';
      errbuf[1] = 'K';
      errbuf[2] = 7;
      errbuf[3] = 8;
      bfwrite(errbuf, 1, 4, BFWRITE_DATA);
      /* tempzn updated below */
      tempzn += 4;
    }
  }

  o = 0;                                /* no ZE_OPEN errors yet */


  /* Process zip file, updating marked files */
#ifdef DEBUG
  if (zfiles != NULL)
    diag("going through old zip file");
#endif
  if (zfiles != NULL && show_what_doing) {
    fprintf(mesg, "Going through old zip file\n");
    fflush(mesg);
  }
  w = &zfiles;
  while ((z = *w) != NULL) {
    if (z->mark == 1)
    {
      uzoff_t len;
      len = z->len;

      /* if not deleting, zip it up */
      if (action != ARCHIVE && action != DELETE)
      {
        DisplayRunningStats();
        if (noisy)
        {
          if (action == FRESHEN)
             fprintf(mesg, "freshening: %s", z->oname);
          else
             fprintf(mesg, "updating: %s", z->oname);
          fflush(mesg);
          mesg_line_started = 1;
        }
        if (logall)
        {
          if (action == FRESHEN)
             fprintf(logfile, "freshening: %s", z->oname);
          else
             fprintf(logfile, "updating: %s", z->oname);
        }
        if ((r = zipup(z)) != ZE_OK && r != ZE_OPEN && r != ZE_MISS)
        {
          if (noisy)
          {
#if (!defined(MACOS) && !defined(WINDLL))
            putc('\n', mesg);
            fflush(mesg);
#else
            fprintf(stdout, "\n");
#endif
            mesg_line_started = 0;
            if (logall)
              fprintf(logfile, "\n");
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
            mesg_line_started = 0;
            if (logall)
              fprintf(logfile, "\n");
          }
          if (r == ZE_OPEN) {
            perror(z->oname);
            zipwarn("could not open for reading: ", z->oname);
            if (bad_open_is_error) {
              sprintf(errbuf, "was zipping %s", z->name);
              ZIPERR(r, errbuf);
            }
          } else {
            zipwarn("file and directory with the same name: ", z->oname);
          }
          zipwarn("will just copy entry over: ", z->oname);
          if ((r = zipcopy(z)) != ZE_OK)
          {
            sprintf(errbuf, "was copying %s", z->oname);
            ZIPERR(r, errbuf);
          }
          z->mark = 0;
        }
        files_so_far++;
        good_bytes_so_far += z->len;
        bytes_so_far += len;
        w = &z->nxt;
      }
      else if (action == ARCHIVE)
      {
        DisplayRunningStats();
        if (noisy)
        {
          fprintf(mesg, "copying: %s", z->oname);
          if (display_usize) {
            fprintf(mesg, " (");
            DisplayNumString(mesg, z->len );
            fprintf(mesg, ")");
          }
          fflush(mesg);
        }
        if (logall)
        {
          fprintf(logfile, "copying: %s", z->oname);
          if (display_usize) {
            fprintf(logfile, " (");
            DisplayNumString(logfile, z->len );
            fprintf(logfile, ")");
          }
        }
        if ((r = zipcopy(z)) != ZE_OK)
        {
          sprintf(errbuf, "was copying %s", z->oname);
          ZIPERR(r, errbuf);
        }
        if (noisy)
        {
          fprintf(mesg, "\n");
        }
        if (logall)
        {
          fprintf(logfile, "\n");
        }
        files_so_far++;
        good_bytes_so_far += z->siz;
        bytes_so_far += z->siz;
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
#endif /* ZIP64_SUPPORT - I added comments around // comments - does that help below? EG */
/* strange but true: if I delete this and put these two endifs adjacent to
   each other, the Aztec Amiga compiler never sees the second endif!  WTF?? PK */
#endif /* WINDLL */
        w = &z->nxt;
      }
      else
      {
        DisplayRunningStats();
        if (noisy)
        {
          fprintf(mesg, "deleting: %s", z->oname);
          if (display_usize) {
            fprintf(mesg, " (");
            DisplayNumString(mesg, z->len );
            fprintf(mesg, ")");
          }
          fflush(mesg);
          fprintf(mesg, "\n");
        }
        if (logall)
        {
          fprintf(logfile, "deleting: %s", z->oname);
          if (display_usize) {
            fprintf(logfile, " (");
            DisplayNumString(logfile, z->len );
            fprintf(logfile, ")");
          }
          fprintf(logfile, "\n");
        }
        files_so_far++;
        good_bytes_so_far += z->siz;
        bytes_so_far += z->siz;
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
#endif /* ZIP64_SUPPORT - I added comments around // comments - does that help below? EG */
/* strange but true: if I delete this and put these two endifs adjacent to
   each other, the Aztec Amiga compiler never sees the second endif!  WTF?? PK */
#endif /* WINDLL */

        v = z->nxt;                     /* delete entry from list */
        free((zvoid *)(z->iname));
        free((zvoid *)(z->zname));
        free(z->oname);
#ifdef UNICODE_SUPPORT
        if (z->uname) free(z->uname);
#endif /* def UNICODE_SUPPORT */
        if (z->ext)
          /* don't have local extra until zipcopy reads it */
          if (z->extra) free((zvoid *)(z->extra));
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
      if (action == ARCHIVE) {
        v = z->nxt;                     /* delete entry from list */
        free((zvoid *)(z->iname));
        free((zvoid *)(z->zname));
        free(z->oname);
#ifdef UNICODE_SUPPORT
        if (z->uname) free(z->uname);
#endif /* def UNICODE_SUPPORT */
        if (z->ext)
          /* don't have local extra until zipcopy reads it */
          if (z->extra) free((zvoid *)(z->extra));
        if (z->cext && z->cextra != z->extra)
          free((zvoid *)(z->cextra));
        if (z->com)
          free((zvoid *)(z->comment));
        farfree((zvoid far *)z);
        *w = v;
        zcount--;
      }
      else
      {
        /* copy the original entry */
        if (!d && (r = zipcopy(z)) != ZE_OK)
        {
          sprintf(errbuf, "was copying %s", z->oname);
          ZIPERR(r, errbuf);
        }
        w = &z->nxt;
      }
    }
  }


  /* Process the edited found list, adding them to the zip file */
  if (show_what_doing) {
    fprintf(mesg, "zipping up new entries\n");
    fflush(mesg);
  }
  diag("zipping up new entries, if any");
  Trace((stderr, "zip diagnostic: fcount=%u\n", (unsigned)fcount));
  for (f = found; f != NULL; f = fexpel(f))
  {
    uzoff_t len;
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
#ifdef UNICODE_SUPPORT
    z->uname = f->uname;
    f->uname = NULL;
#endif
    z->oname = f->oname;
    f->oname = NULL;
    z->ext = z->cext = z->com = 0;
    z->extra = z->cextra = NULL;
    z->mark = 1;
    z->dosflag = f->dosflag;
    /* zip it up */
    DisplayRunningStats();
    if (noisy)
    {
      fprintf(mesg, "  adding: %s", z->oname);
      fflush(mesg);
      mesg_line_started = 1;
    }
    if (logall)
    {
      fprintf(logfile, "  adding: %s", z->oname);
    }
    /* initial scan */
    len = f->usize;
    if ((r = zipup(z)) != ZE_OK  && r != ZE_OPEN && r != ZE_MISS)
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
      mesg_line_started = 0;
      if (logall)
        fprintf(logfile, "\n");
      sprintf(errbuf, "was zipping %s", z->oname);
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
        mesg_line_started = 0;
      }
      if (logall)
        fprintf(logfile, "\n");
      if (r == ZE_OPEN) {
        perror("zip warning");
        if (logfile)
          fprintf(logfile, "zip warning: %s\n", strerror(errno));
        zipwarn("could not open for reading: ", z->oname);
        if (bad_open_is_error) {
          sprintf(errbuf, "was zipping %s", z->name);
          ZIPERR(r, errbuf);
        }
      } else {
        zipwarn("file and directory with the same name: ", z->oname);
      }
      files_so_far++;
      bytes_so_far += len;
      bad_files_so_far++;
      bad_bytes_so_far += len;
      free((zvoid *)(z->name));
      free((zvoid *)(z->iname));
      free((zvoid *)(z->zname));
      free(z->oname);
#ifdef UNICODE_SUPPORT
      free(z->uname);
#endif
      farfree((zvoid far *)z);
    }
    else
    {
      files_so_far++;
      /* current size of file (just before reading) */
      good_bytes_so_far += z->len;
      /* size of file on initial scan */
      bytes_so_far += len;
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

  /* final status 3/17/05 EG */
  if (noisy && bad_files_so_far)
  {
    char tempstrg[100];

    fprintf(mesg, "\nzip warning: Not all files were readable\n");
    fprintf(mesg, "  files/entries read:  %lu", files_total - bad_files_so_far);
    WriteNumString(good_bytes_so_far, tempstrg);
    fprintf(mesg, " (%s bytes)", tempstrg);
    fprintf(mesg, "  skipped:  %lu", bad_files_so_far);
    WriteNumString(bad_bytes_so_far, tempstrg);
    fprintf(mesg, " (%s bytes)\n", tempstrg);
    fflush(mesg);
  }
  if (logfile && bad_files_so_far)
  {
    char tempstrg[100];

    fprintf(logfile, "\nzip warning: Not all files were readable\n");
    fprintf(logfile, "  files/entries read:  %lu", files_total - bad_files_so_far);
    WriteNumString(good_bytes_so_far, tempstrg);
    fprintf(logfile, " (%s bytes)", tempstrg);
    fprintf(logfile, "  skipped:  %lu", bad_files_so_far);
    WriteNumString(bad_bytes_so_far, tempstrg);
    fprintf(logfile, " (%s bytes)", tempstrg);
  }

  /* Get one line comment for each new entry */
  if (show_what_doing) {
    fprintf(mesg, "Get comment if any\n");
    fflush(mesg);
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
            fprintf(mesg, "Enter comment for %s:\n", z->oname);
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

  if (display_globaldots) {
#ifndef WINDLL
    putc('\n', mesg);
#else
    fprintf(stdout,"%c",'\n');
#endif
    mesg_line_started = 0;
  }

  /* Write central directory and end header to temporary zip */
  diag("writing central directory");
  k = 0;                        /* keep count for end header */
  c = tempzn;                   /* get start of central */
  n = t = 0;
  for (z = zfiles; z != NULL; z = z->nxt)
  {
    if ((r = putcentral(z)) != ZE_OK) {
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
    fprintf(mesg, "total bytes=%s, compressed=%s -> %d%% savings\n",
            zip_fzofft(n, NULL, "u"), zip_fzofft(t, NULL, "u"), percent(n, t));
    if (logall)
      fprintf(logfile, "total bytes=%s, compressed=%s -> %d%% savings\n",
              zip_fzofft(n, NULL, "u"), zip_fzofft(t, NULL, "u"), percent(n, t));
  }
  t = tempzn - c;               /* compute length of central */
  diag("writing end of central directory");
  if ((r = putend(k, t, c, zcomlen, zcomment)) != ZE_OK) {
    ZIPERR(r, tempzip);
  }
  /*
  tempzf = NULL;
  */
  if (fclose(y)) {
    ZIPERR(d ? ZE_WRITE : ZE_TEMP, tempzip);
  }
  if (in_file != NULL) {
    fclose(in_file);
    in_file = NULL;
  }
  /*
  if (x != NULL)
    fclose(x);
  */

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
    if ((r = replace(out_path, tempzip)) != ZE_OK)
    {
      zipwarn("new zip file left as: ", tempzip);
      free((zvoid *)tempzip);
      tempzip = NULL;
      ZIPERR(r, "was replacing the original zip file");
    }
    free((zvoid *)tempzip);
  }
  tempzip = NULL;
  if (zip_attributes && strcmp(zipfile, "-")) {
    setfileattr(out_path, zip_attributes);
#ifdef VMS
    /* If the zip file existed previously, restore its record format: */
    if (x != NULL)
      (void)VMSmunch(out_path, RESTORE_RTYPE, NULL);
#endif
  }

  set_filetype(out_path);
  
#if defined(WIN32)
  /* All looks good so, if requested, clear the DOS archive bits */
  if (clear_archive_bits) {
    if (noisy)
      zipmessage("Clearing archive bits...", "");
    for (z = zfiles; z != NULL; z = z->nxt)
    {
      if (!ClearArchiveBit(z->name)){
        zipwarn("Could not clear archive bit for: ", z->name);
      }
    }
  }
#endif

  /* finish logfile (it gets closed in freeup() called by finish()) */
  if (logfile) {
      struct tm *now;
      time_t clocktime;

      fprintf(logfile, "\nTotal %ld entries (", files_total);
      if (good_bytes_so_far != bytes_total) {
        fprintf(logfile, "planned ");
        DisplayNumString(logfile, bytes_total);
        fprintf(logfile, " bytes, actual ");
        DisplayNumString(logfile, good_bytes_so_far);
        fprintf(logfile, " bytes)");
      } else {
        DisplayNumString(logfile, bytes_total);
        fprintf(logfile, " bytes)");
      }

      /* get current time */

      time(&clocktime);
      now = localtime(&clocktime);
      fprintf(logfile, "\nDone %s", asctime(now));
  }

  /* Finish up (process -o, -m, clean up).  Exit code depends on o. */
#if (!defined(VMS) && !defined(CMS_MVS))
  free((zvoid *) zipbuf);
#endif /* !VMS && !CMS_MVS */
  RETURN(finish(o ? ZE_OPEN : ZE_OK));
}

