/*
  fileio.c - Zip 3

  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  fileio.c by Mark Adler
 */
#define __FILEIO_C

#include "zip.h"

#ifdef MACOS
#  include "helpers.h"
#endif

#include <time.h>

#ifdef NO_MKTIME
time_t mktime OF((struct tm *));
#endif

#ifdef OSF
#define EXDEV 18   /* avoid a bug in the DEC OSF/1 header files. */
#else
#include <errno.h>
#endif

#ifdef NO_ERRNO
extern int errno;
#endif

/* -----------------------
   For long option support
   ----------------------- */
#include <ctype.h>


#if defined(VMS) || defined(TOPS20)
#  define PAD 5
#else
#  define PAD 0
#endif

#ifdef NO_RENAME
int rename OF((ZCONST char *, ZCONST char *));
#endif


/* Local functions */
local int optionerr OF((char *, ZCONST char *, int, int));
local unsigned long get_shortopt OF((char **, int, int *, int *, char **, int *, int));
local unsigned long get_longopt OF((char **, int, int *, int *, char **, int *, int));

#ifndef UTIL    /* the companion #endif is a bit of ways down ... */

local int fqcmp  OF((ZCONST zvoid *, ZCONST zvoid *));
local int fqcmpz OF((ZCONST zvoid *, ZCONST zvoid *));

/* Local module level variables. */
char *label = NULL;                /* global, but only used in `system'.c */
local z_stat zipstatb;             /* now use z_stat globally - 7/24/04 EG */
#if (!defined(MACOS) && !defined(WINDLL))
local int zipstate = -1;
#else
int zipstate;
#endif
/* -1 unknown, 0 old zip file exists, 1 new zip file */

#if 0
char *getnam(n, fp)
char *n;                /* where to put name (must have >=FNMAX+1 bytes) */
#endif

/* converted to return string pointer from malloc to avoid
   size limitation - 11/8/04 EG */
#define GETNAM_MAX 9000 /* hopefully big enough for now */
char *getnam(fp)
  FILE *fp;
  /* Read a \n or \r delimited name from stdin into n, and return
     n.  If EOF, then return NULL.  Also, if problem return NULL. */
{
  char name[GETNAM_MAX + 1];
  int c;                /* last character read */
  char *p;              /* pointer into name area */


  p = name;
  while ((c = getc(fp)) == '\n' || c == '\r')
    ;
  if (c == EOF)
    return NULL;
  do {
    if (p - name >= GETNAM_MAX)
      return NULL;
    *p++ = (char) c;
    c = getc(fp);
  } while (c != EOF && (c != '\n' && c != '\r'));
#ifdef WIN32
/*
 * WIN32 strips off trailing spaces and periods in filenames
 * XXX what about a filename that only consists of spaces ?
 *     Answer: on WIN32, a filename must contain at least one non-space char
 */
  while (p > name) {
    if ((c = p[-1]) != ' ' && c != '.')
      break;
    --p;
  }
#endif
  *p = 0;
  /* malloc a copy */
  if ((p = malloc(strlen(name) + 1)) == NULL) {
    return NULL;
  }
  strcpy(p, name);
  return p;
}

struct flist far *fexpel(f)
struct flist far *f;    /* entry to delete */
/* Delete the entry *f in the doubly-linked found list.  Return pointer to
   next entry to allow stepping through list. */
{
  struct flist far *t;  /* temporary variable */

  t = f->nxt;
  *(f->lst) = t;                        /* point last to next, */
  if (t != NULL)
    t->lst = f->lst;                    /* and next to last */
  if (f->name != NULL)                  /* free memory used */
    free((zvoid *)(f->name));
  if (f->zname != NULL)
    free((zvoid *)(f->zname));
  if (f->iname != NULL)
    free((zvoid *)(f->iname));
  farfree((zvoid far *)f);
  fcount--;                             /* decrement count */
  return t;                             /* return pointer to next */
}


local int fqcmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to found entries */
/* Used by qsort() to compare entries in the found list by name. */
{
  return strcmp((*(struct flist far **)a)->name,
                (*(struct flist far **)b)->name);
}


local int fqcmpz(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to found entries */
/* Used by qsort() to compare entries in the found list by iname. */
{
  return strcmp((*(struct flist far **)a)->iname,
                (*(struct flist far **)b)->iname);
}


char *last(p, c)
char *p;                /* sequence of path components */
int c;                  /* path components separator character */
/* Return a pointer to the start of the last path component. For a directory
 * name terminated by the character in c, the return value is an empty string.
 */
{
  char *t;              /* temporary variable */

  if ((t = strrchr(p, c)) != NULL)
    return t + 1;
  else
#ifndef AOS_VS
    return p;
#else
/* We want to allow finding of end of path in either AOS/VS-style pathnames
 * or Unix-style pathnames.  This presents a few little problems ...
 */
  {
    if (*p == '='  ||  *p == '^')      /* like ./ and ../ respectively */
      return p + 1;
    else
      return p;
  }
#endif
}


char *msname(n)
char *n;
/* Reduce all path components to MSDOS upper case 8.3 style names. */
{
  int c;                /* current character */
  int f;                /* characters in current component */
  char *p;              /* source pointer */
  char *q;              /* destination pointer */

  p = q = n;
  f = 0;
  while ((c = (unsigned char)*POSTINCSTR(p)) != 0)
    if (c == ' ' || c == ':' || c == '"' || c == '*' || c == '+' ||
        c == ',' || c == ';' || c == '<' || c == '=' || c == '>' ||
        c == '?' || c == '[' || c == ']' || c == '|')
      continue;                         /* char is discarded */
    else if (c == '/')
    {
      *POSTINCSTR(q) = (char)c;
      f = 0;                            /* new component */
    }
#ifdef __human68k__
    else if (ismbblead(c) && *p)
    {
      if (f == 7 || f == 11)
        f++;
      else if (*p && f < 12 && f != 8)
      {
        *q++ = c;
        *q++ = *p++;
        f += 2;
      }
    }
#endif /* __human68k__ */
    else if (c == '.')
    {
      if (f == 0)
        continue;                       /* leading dots are discarded */
      else if (f < 9)
      {
        *POSTINCSTR(q) = (char)c;
        f = 9;                          /* now in file type */
      }
      else
        f = 12;                         /* now just excess characters */
    }
    else
      if (f < 12 && f != 8)
      {
        f += CLEN(p);                   /* do until end of name or type */
        *POSTINCSTR(q) = (char)(to_up(c));
      }
  *q = 0;
  return n;
}

int check_dup()
/* Sort the found list and remove duplicates.
   Return an error code in the ZE_ class. */
{
  struct flist far *f;          /* steps through found linked list */
  extent j, k;                  /* indices for s */
  struct flist far **s;         /* sorted table */
  struct flist far **nodup;     /* sorted table without duplicates */

  /* sort found list, remove duplicates */
  if (fcount)
  {
    extent fl_size = fcount * sizeof(struct flist far *);
    if ((fl_size / sizeof(struct flist far *)) != fcount ||
        (s = (struct flist far **)malloc(fl_size)) == NULL)
      return ZE_MEM;
    for (j = 0, f = found; f != NULL; f = f->nxt)
      s[j++] = f;
    /* Check names as given (f->name) */
    qsort((char *)s, fcount, sizeof(struct flist far *), fqcmp);
    for (k = j = fcount - 1; j > 0; j--)
      if (strcmp(s[j - 1]->name, s[j]->name) == 0)
        /* remove duplicate entry from list */
        fexpel(s[j]);           /* fexpel() changes fcount */
      else
        /* copy valid entry into destination position */
        s[k--] = s[j];
    s[k] = s[0];                /* First entry is always valid */
    nodup = &s[k];              /* Valid entries are at end of array s */

    /* sort only valid items and check for unique internal names (f->iname) */
    qsort((char *)nodup, fcount, sizeof(struct flist far *), fqcmpz);
    for (j = 1; j < fcount; j++)
      if (strcmp(nodup[j - 1]->iname, nodup[j]->iname) == 0)
      {
        zipwarn("  first full name: ", nodup[j - 1]->name);
        zipwarn(" second full name: ", nodup[j]->name);
#ifdef EBCDIC
        strtoebc(nodup[j]->iname, nodup[j]->iname);
#endif
        zipwarn("name in zip file repeated: ", nodup[j]->iname);
#ifdef EBCDIC
        strtoasc(nodup[j]->iname, nodup[j]->iname);
#endif
        return ZE_PARMS;
      }
    free((zvoid *)s);
  }
  return ZE_OK;
}

int filter(name, casesensitive)
  char *name;
  int casesensitive;
  /* Scan the -R, -i and -x lists for matches to the given name.
     Return TRUE if the name must be included, FALSE otherwise.
     Give precedence to -x over -i and -R.
     Note that if both R and i patterns are given then must
     have a match for both.
     This routine relies on the following global variables:
       patterns                 array of match pattern structures
       pcount                   total number of patterns
       icount                   number of -i patterns
       Rcount                   number of -R patterns
     These data are set up by the command line parsing code.
   */
{
   unsigned int n;
   int slashes;
   char *p, *q;
   /* without -i patterns, every name matches the "-i select rules" */
   int imatch = (icount == 0);
   /* without -R patterns, every name matches the "-R select rules" */
   int Rmatch = (Rcount == 0);

   if (pcount == 0) return TRUE;

   for (n = 0; n < pcount; n++) {
      if (!patterns[n].zname[0])        /* it can happen... */
         continue;
      p = name;
      switch (patterns[n].select) {
       case 'R':
         if (Rmatch)
            /* one -R match is sufficient, skip this pattern */
            continue;
         /* With -R patterns, if the pattern has N path components (that is,
            N-1 slashes), then we test only the last N components of name.
          */
         slashes = 0;
         for (q = patterns[n].zname; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
            slashes++;
         /* The name may have M path components (M-1 slashes) */
         for (q = p; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
            slashes--;
         /* Now, "slashes" contains the difference "N-M" between the number
            of path components in the pattern (N) and in the name (M).
          */
         if (slashes < 0)
            /* We found "M > N"
                --> skip the first (M-N) path components of the name.
             */
            for (q = p; (q = MBSCHR(q, '/')) != NULL; INCSTR(q))
               if (++slashes == 0) {
                  p = q + 1;    /* q points at '/', mblen("/") is 1 */
                  break;
               }
         break;
       case 'i':
         if (imatch)
            /* one -i match is sufficient, skip this pattern */
            continue;
         break;
      }
      if (MATCH(patterns[n].zname, p, casesensitive)) {
         switch (patterns[n].select) {
            case 'x':
               /* The -x match takes precedence over everything else */
               return FALSE;
            case 'R':
               Rmatch = TRUE;
               break;
            default:
               /* this must be a type -i match */
               imatch = TRUE;
               break;
         }
      }
   }
   return imatch && Rmatch;
}

int newname(name, isdir, casesensitive)
char *name;             /* name to add (or exclude) */
int  isdir;             /* true for a directory */
int  casesensitive;     /* true for case-sensitive matching */
/* Add (or exclude) the name of an existing disk file.  Return an error
   code in the ZE_ class. */
{
  char *iname, *zname;  /* internal name, external version of iname */
  char *undosm;         /* zname version with "-j" and "-k" options disabled */
  struct flist far *f;  /* where in found, or new found entry */
  struct zlist far *z;  /* where in zfiles (if found) */
  int dosflag;

  /* Search for name in zip file.  If there, mark it, else add to
     list of new names to do (or remove from that list). */
  if ((iname = ex2in(name, isdir, &dosflag)) == NULL)
    return ZE_MEM;

  /* Discard directory names with zip -rj */
  if (*iname == '\0') {
#ifndef AMIGA
/* A null string is a legitimate external directory name in AmigaDOS; also,
 * a command like "zip -r zipfile FOO:" produces an empty internal name.
 */
# ifndef RISCOS
 /* If extensions needs to be swapped, we will have empty directory names
    instead of the original directory. For example, zipping 'c.', 'c.main'
    should zip only 'main.c' while 'c.' will be converted to '\0' by ex2in. */

    if (pathput && !recurse) error("empty name without -j or -r");

# endif /* !RISCOS */
#endif /* !AMIGA */
    free((zvoid *)iname);
    return ZE_OK;
  }
  undosm = NULL;
  if (dosflag || !pathput) {
    int save_dosify = dosify, save_pathput = pathput;
    dosify = 0;
    pathput = 1;
    /* zname is temporarly mis-used as "undosmode" iname pointer */
    if ((zname = ex2in(name, isdir, NULL)) != NULL) {
      undosm = in2ex(zname);
      free(zname);
    }
    dosify = save_dosify;
    pathput = save_pathput;
  }
  if ((zname = in2ex(iname)) == NULL)
    return ZE_MEM;
  if (undosm == NULL)
    undosm = zname;
  if ((z = zsearch(zname)) != NULL) {
    if (pcount && !filter(undosm, casesensitive)) {
      /* Do not clear z->mark if "exclude", because, when "dosify || !pathput"
       * is in effect, two files with different filter options may hit the
       * same z entry.
       */
      if (verbose)
        fprintf(mesg, "excluding %s\n", zname);
      free((zvoid *)iname);
      free((zvoid *)zname);
    } else {
      z->mark = 1;
      if ((z->name = malloc(strlen(name) + 1 + PAD)) == NULL) {
        if (undosm != zname)
          free((zvoid *)undosm);
        free((zvoid *)iname);
        free((zvoid *)zname);
        return ZE_MEM;
      }
      strcpy(z->name, name);
      z->dosflag = dosflag;

#ifdef FORCE_NEWNAME
      free((zvoid *)(z->iname));
      z->iname = iname;
#else
      /* Better keep the old name. Useful when updating on MSDOS a zip file
       * made on Unix.
       */
      free((zvoid *)iname);
      free((zvoid *)zname);
#endif /* ? FORCE_NEWNAME */
    }
    if (name == label) {
       label = z->name;
    }
  } else if (pcount == 0 || filter(undosm, casesensitive)) {

    /* Check that we are not adding the zip file to itself. This
     * catches cases like "zip -m foo ../dir/foo.zip".
     */
#ifndef CMS_MVS
/* Version of stat() for CMS/MVS isn't complete enough to see if       */
/* files match.  Just let ZIP.C compare the filenames.  That's good    */
/* enough for CMS anyway since there aren't paths to worry about.      */
    z_stat statb;      /* now use structure z_stat and function zstat globally 7/24/04 EG */

    if (zipstate == -1)
       zipstate = strcmp(zipfile, "-") != 0 &&
                   zstat(zipfile, &zipstatb) == 0;

    if (zipstate == 1 && (statb = zipstatb, zstat(name, &statb) == 0
      && zipstatb.st_mode  == statb.st_mode
#ifdef VMS
      && memcmp(zipstatb.st_ino, statb.st_ino, sizeof(statb.st_ino)) == 0
      && strcmp(zipstatb.st_dev, statb.st_dev) == 0
      && zipstatb.st_uid   == statb.st_uid
#else /* !VMS */
      && zipstatb.st_ino   == statb.st_ino
      && zipstatb.st_dev   == statb.st_dev
      && zipstatb.st_uid   == statb.st_uid
      && zipstatb.st_gid   == statb.st_gid
#endif /* ?VMS */
      && zipstatb.st_size  == statb.st_size
      && zipstatb.st_mtime == statb.st_mtime
      && zipstatb.st_ctime == statb.st_ctime)) {
      /* Don't compare a_time since we are reading the file */
         if (verbose)
           fprintf(mesg, "file matches zip file -- skipping\n");
         if (undosm != zname)
           free((zvoid *)zname);
         if (undosm != iname)
           free((zvoid *)undosm);
         free((zvoid *)iname);
         return ZE_OK;
    }
#endif  /* CMS_MVS */

    /* allocate space and add to list */
    if ((f = (struct flist far *)farmalloc(sizeof(struct flist))) == NULL ||
        fcount + 1 < fcount ||
        (f->name = malloc(strlen(name) + 1 + PAD)) == NULL)
    {
      if (f != NULL)
        farfree((zvoid far *)f);
      if (undosm != zname)
        free((zvoid *)undosm);
      free((zvoid *)iname);
      free((zvoid *)zname);
      return ZE_MEM;
    }
    strcpy(f->name, name);
    f->iname = iname;
    f->zname = zname;
    f->dosflag = dosflag;
    *fnxt = f;
    f->lst = fnxt;
    f->nxt = NULL;
    fnxt = &f->nxt;
    fcount++;
    if (name == label) {
      label = f->name;
    }
  }
  if (undosm != zname)
    free((zvoid *)undosm);
  return ZE_OK;
}


ulg dostime(y, n, d, h, m, s)
int y;                  /* year */
int n;                  /* month */
int d;                  /* day */
int h;                  /* hour */
int m;                  /* minute */
int s;                  /* second */
/* Convert the date y/n/d and time h:m:s to a four byte DOS date and
   time (date in high two bytes, time in low two bytes allowing magnitude
   comparison). */
{
  return y < 1980 ? DOSTIME_MINIMUM /* dostime(1980, 1, 1, 0, 0, 0) */ :
        (((ulg)y - 1980) << 25) | ((ulg)n << 21) | ((ulg)d << 16) |
        ((ulg)h << 11) | ((ulg)m << 5) | ((ulg)s >> 1);
}


ulg unix2dostime(t)
time_t *t;              /* unix time to convert */
/* Return the Unix time t in DOS format, rounded up to the next two
   second boundary. */
{
  time_t t_even;
  struct tm *s;         /* result of localtime() */

  t_even = (time_t)(((unsigned long)(*t) + 1) & (~1));
                                /* Round up to even seconds. */
  s = localtime(&t_even);       /* Use local time since MSDOS does. */
  if (s == (struct tm *)NULL) {
      /* time conversion error; use current time as emergency value
         (assuming that localtime() does at least accept this value!) */
      t_even = (time_t)(((unsigned long)time(NULL) + 1) & (~1));
      s = localtime(&t_even);
  }
  return dostime(s->tm_year + 1900, s->tm_mon + 1, s->tm_mday,
                 s->tm_hour, s->tm_min, s->tm_sec);
}

int issymlnk(a)
ulg a;                  /* Attributes returned by filetime() */
/* Return true if the attributes are those of a symbolic link */
{
#ifndef QDOS
#ifdef S_IFLNK
#ifdef __human68k__
  int *_dos_importlnenv(void);

  if (_dos_importlnenv() == NULL)
    return 0;
#endif
  return ((a >> 16) & S_IFMT) == S_IFLNK;
#else /* !S_IFLNK */
  return (int)a & 0;    /* avoid warning on unused parameter */
#endif /* ?S_IFLNK */
#else
  return 0;
#endif
}

#endif /* !UTIL */


#if (!defined(UTIL) && !defined(ZP_NEED_GEN_D2U_TIME))
   /* There is no need for dos2unixtime() in the ZipUtils' code. */
#  define ZP_NEED_GEN_D2U_TIME
#endif
#if ((defined(OS2) || defined(VMS)) && defined(ZP_NEED_GEN_D2U_TIME))
   /* OS/2 and VMS use a special solution to handle time-stams of files. */
#  undef ZP_NEED_GEN_D2U_TIME
#endif
#if (defined(W32_STATROOT_FIX) && !defined(ZP_NEED_GEN_D2U_TIME))
   /* The Win32 stat()-bandaid to fix stat'ing root directories needs
    * dos2unixtime() to calculate the time-stamps. */
#  define ZP_NEED_GEN_D2U_TIME
#endif

#ifdef ZP_NEED_GEN_D2U_TIME

time_t dos2unixtime(dostime)
ulg dostime;            /* DOS time to convert */
/* Return the Unix time_t value (GMT/UTC time) for the DOS format (local)
 * time dostime, where dostime is a four byte value (date in most significant
 * word, time in least significant word), see dostime() function.
 */
{
  struct tm *t;         /* argument for mktime() */
  ZCONST time_t clock = time(NULL);

  t = localtime(&clock);
  t->tm_isdst = -1;     /* let mktime() determine if DST is in effect */
  /* Convert DOS time to UNIX time_t format */
  t->tm_sec  = (((int)dostime) <<  1) & 0x3e;
  t->tm_min  = (((int)dostime) >>  5) & 0x3f;
  t->tm_hour = (((int)dostime) >> 11) & 0x1f;
  t->tm_mday = (int)(dostime >> 16) & 0x1f;
  t->tm_mon  = ((int)(dostime >> 21) & 0x0f) - 1;
  t->tm_year = ((int)(dostime >> 25) & 0x7f) + 80;

  return mktime(t);
}

#undef ZP_NEED_GEN_D2U_TIME
#endif /* ZP_NEED_GEN_D2U_TIME */


#ifndef MACOS
int destroy(f)
char *f;                /* file to delete */
/* Delete the file *f, returning non-zero on failure. */
{
  return unlink(f);
}


int replace(d, s)
char *d, *s;            /* destination and source file names */
/* Replace file *d by file *s, removing the old *s.  Return an error code
   in the ZE_ class. This function need not preserve the file attributes,
   this will be done by setfileattr() later.
 */
{
  z_stat t;         /* results of stat() */
#if defined(CMS_MVS)
  /* cmsmvs.h defines FOPW_TEMP as memory(hiperspace).  Since memory is
   * lost at end of run, always do copy instead of rename.
   */
  int copy = 1;
#else
  int copy = 0;
#endif
  int d_exists;

#if defined(VMS) || defined(CMS_MVS)
  /* stat() is broken on VMS remote files (accessed through Decnet).
   * This patch allows creation of remote zip files, but is not sufficient
   * to update them or compress remote files */
  unlink(d);
#else /* !(VMS || CMS_MVS) */
  d_exists = (LSTAT(d, &t) == 0);
  if (d_exists)
  {
    /*
     * respect existing soft and hard links!
     */
    if (t.st_nlink > 1
# ifdef S_IFLNK
        || (t.st_mode & S_IFMT) == S_IFLNK
# endif
        )
       copy = 1;
    else if (unlink(d))
       return ZE_CREAT;                 /* Can't erase zip file--give up */
  }
#endif /* ?(VMS || CMS_MVS) */
#ifndef CMS_MVS
  if (!copy) {
      if (rename(s, d)) {               /* Just move s on top of d */
          copy = 1;                     /* failed ? */
#if !defined(VMS) && !defined(ATARI) && !defined(AZTEC_C)
#if !defined(CMS_MVS) && !defined(RISCOS) && !defined(QDOS)
    /* For VMS, ATARI, AMIGA Aztec, VM_CMS, MVS, RISCOS,
       always assume that failure is EXDEV */
          if (errno != EXDEV
#  ifdef THEOS
           && errno != EEXIST
#  else
#    ifdef ENOTSAM
           && errno != ENOTSAM /* Used at least on Turbo C */
#    endif
#  endif
              ) return ZE_CREAT;
#endif /* !CMS_MVS && !RISCOS */
#endif /* !VMS && !ATARI && !AZTEC_C */
      }
  }
#endif /* !CMS_MVS */

  if (copy) {
    FILE *f, *g;        /* source and destination files */
    int r;              /* temporary variable */

#ifdef RISCOS
    if (SWI_OS_FSControl_26(s,d,0xA1)!=NULL) {
#endif

    /* Use zfopen for almost all opens where fopen is used.  For
       most OS that support large files we use the 64-bit file
       environment and zfopen maps to fopen, but this allows
       tweeking ports that don't do that.  7/24/04 EG */
    if ((f = zfopen(s, FOPR)) == NULL) {
      fprintf(stderr," replace: can't open %s\n", s);
      return ZE_TEMP;
    }
    if ((g = zfopen(d, FOPW)) == NULL)
    {
      fclose(f);
      return ZE_CREAT;
    }

    r = fcopy(f, g, (ulg)-1L);
    fclose(f);
    if (fclose(g) || r != ZE_OK)
    {
      unlink(d);
      return r ? (r == ZE_TEMP ? ZE_WRITE : r) : ZE_WRITE;
    }
    unlink(s);
#ifdef RISCOS
    }
#endif
  }
  return ZE_OK;
}
#endif /* !MACOS */


int getfileattr(f)
char *f;                /* file path */
/* Return the file attributes for file f or 0 if failure */
{
#ifdef __human68k__
  struct _filbuf buf;

  return _dos_files(&buf, f, 0xff) < 0 ? 0x20 : buf.atr;
#else
  z_stat s;

  return SSTAT(f, &s) == 0 ? (int) s.st_mode : 0;
#endif
}


int setfileattr(f, a)
char *f;                /* file path */
int a;                  /* attributes returned by getfileattr() */
/* Give the file f the attributes a, return non-zero on failure */
{
#if defined(TOPS20) || defined (CMS_MVS)
  return 0;
#else
#ifdef __human68k__
  return _dos_chmod(f, a) < 0 ? -1 : 0;
#else
  return chmod(f, a);
#endif
#endif
}


/* tempname */

#ifndef VMS /* VMS-specific function is in VMS.C. */

char *tempname(zip)
char *zip;              /* path name of zip file to generate temp name for */

/* Return a temporary file name in its own malloc'ed space, using tempath. */
{
  char *t = zip;   /* malloc'ed space for name (use zip to avoid warning) */

# ifdef CMS_MVS
  if ((t = malloc(strlen(tempath)+L_tmpnam+2)) == NULL)
    return NULL;

#  ifdef VM_CMS
  tmpnam(t);
  /* Remove filemode and replace with tempath, if any. */
  /* Otherwise A-disk is used by default */
  *(strrchr(t, ' ')+1) = '\0';
  if (tempath!=NULL)
     strcat(t, tempath);
  return t;
#  else   /* !VM_CMS */
  /* For MVS */
  tmpnam(t);
  if (tempath != NULL)
  {
    int l1 = strlen(t);
    char *dot;
    if (*t == '\'' && *(t+l1-1) == '\'' && (dot = strchr(t, '.')))
    {
      /* MVS and not OE.  tmpnam() returns quoted string of 5 qualifiers.
       * First is HLQ, rest are timestamps.  User can only replace HLQ.
       */
      int l2 = strlen(tempath);
      if (strchr(tempath, '.') || l2 < 1 || l2 > 8)
        ziperr(ZE_PARMS, "On MVS and not OE, tempath (-b) can only be HLQ");
      memmove(t+1+l2, dot, l1+1-(dot-t));  /* shift dot ready for new hlq */
      memcpy(t+1, tempath, l2);            /* insert new hlq */
    }
    else
    {
      /* MVS and probably OE.  tmpnam() returns filename based on TMPDIR,
       * no point in even attempting to change it.  User should modify TMPDIR
       * instead.
       */
      zipwarn("MVS, assumed to be OE, change TMPDIR instead of option -b: ",
              tempath);
    }
  }
  return t;
#  endif  /* !VM_CMS */

# else /* !CMS_MVS */

#  ifdef TANDEM
  char cur_subvol [FILENAME_MAX];
  char temp_subvol [FILENAME_MAX];
  char *zptr;
  char *ptr;
  char *cptr = &cur_subvol[0];
  char *tptr = &temp_subvol[0];
  short err;
  FILE *tempf;
  int attempts;

  t = (char *)malloc(NAMELEN); /* malloc here as you cannot free */
                               /* tmpnam allocated storage later */

  zptr = strrchr(zip, TANDEM_DELIMITER);

  if (zptr != NULL) {
    /* ZIP file specifies a Subvol so make temp file there so it can just
       be renamed at end */

    *tptr = *cptr = '\0';
    strcat(cptr, getenv("DEFAULTS"));

    strncat(tptr, zip, _min(FILENAME_MAX, (zptr - zip)) ); /* temp subvol */
    strncat(t, zip, _min(NAMELEN, ((zptr - zip) + 1)) );   /* temp stem   */

    err = chvol(tptr);
    ptr = t + strlen(t);  /* point to end of stem */
  }
  else
    ptr = t;

  /* If two zips are running in same subvol then we can get contention problems
     with the temporary filename.  As a work around we attempt to create
     the file here, and if it already exists we get a new temporary name */

  attempts = 0;
  do {
    attempts++;
    tmpnam(ptr);  /* Add filename */
    tempf = zfopen(ptr, FOPW_TMP);    /* Attempt to create file */
  } while (tempf == NULL && attempts < 100);

  if (attempts >= 100) {
    ziperr(ZE_TEMP, "Could not get unique temp file name");
  }

  fclose(tempf);

  if (zptr != NULL) {
    err = chvol(cptr);  /* Put ourself back to where we came in */
  }

  return t;

#  else /* !CMS_MVS && !TANDEM */
/*
 * Do something with TMPDIR, TMP, TEMP ????
 */
  if (tempath != NULL)
  {
    if ((t = malloc(strlen(tempath)+12)) == NULL)
      return NULL;
    strcpy(t, tempath);

#   if (!defined(VMS) && !defined(TOPS20))
#    ifdef MSDOS
    {
      char c = (char)lastchar(t);
      if (c != '/' && c != ':' && c != '\\')
        strcat(t, "/");
    }
#    else

#     ifdef AMIGA
    {
      char c = (char)lastchar(t);
      if (c != '/' && c != ':')
        strcat(t, "/");
    }
#     else /* !AMIGA */
#      ifdef RISCOS
    if (lastchar(t) != '.')
      strcat(t, ".");
#      else /* !RISCOS */

#       ifdef QDOS
    if (lastchar(t) != '_')
      strcat(t, "_");
#       else
    if (lastchar(t) != '/')
      strcat(t, "/");
#       endif /* ?QDOS */
#      endif /* ?RISCOS */
#     endif  /* ?AMIGA */
#    endif /* ?MSDOS */
#   endif /* !VMS && !TOPS20 */
  }
  else
  {
    if ((t = malloc(12)) == NULL)
      return NULL;
    *t = 0;
  }
#   ifdef NO_MKTEMP
  {
    char *p = t + strlen(t);
    sprintf(p, "%08lx", (ulg)time(NULL));
    return t;
  }
#   else
  strcat(t, "ziXXXXXX"); /* must use lowercase for Linux dos file system */
  return mktemp(t);
#   endif /* NO_MKTEMP */
#  endif /* TANDEM */
# endif /* CMS_MVS */
}
#endif /* !VMS */

int fcopy(f, g, n)
  FILE *f, *g;            /* source and destination files */
  /* now use uzoff_t for all file sizes 5/14/05 CS */
  uzoff_t n;               /* number of bytes to copy or -1 for all */
/* Copy n bytes from file *f to file *g, or until EOF if (zoff_t)n == -1.
   Return an error code in the ZE_ class. */
{
  char *b;              /* malloc'ed buffer for copying */
  extent k;             /* result of fread() */
  uzoff_t m;            /* bytes copied so far */

  if ((b = malloc(CBSZ)) == NULL)
    return ZE_MEM;
  m = 0;
  while (n == (uzoff_t)(-1L) || m < n)
  {
    if ((k = fread(b, 1, n == (uzoff_t)(-1) ?
                   CBSZ : (n - m < CBSZ ? (extent)(n - m) : CBSZ), f)) == 0)
    {
      if (ferror(f))
      {
        free((zvoid *)b);
        return ZE_READ;
      }
      else
        break;
    }
    if (fwrite(b, 1, k, g) != k)
    {
      free((zvoid *)b);
      fprintf(stderr," fcopy: write error\n");
      return ZE_TEMP;
    }
    m += k;
  }
  free((zvoid *)b);
  return ZE_OK;
}

#ifdef NO_RENAME
int rename(from, to)
ZCONST char *from;
ZCONST char *to;
{
    unlink(to);
    if (link(from, to) == -1)
        return -1;
    if (unlink(from) == -1)
        return -1;
    return 0;
}

#endif /* NO_RENAME */


#ifdef ZMEM

/************************/
/*  Function memset()   */
/************************/

/*
 * memset - for systems without it
 *  bill davidsen - March 1990
 */

char *
memset(buf, init, len)
register char *buf;     /* buffer loc */
register int init;      /* initializer */
register unsigned int len;   /* length of the buffer */
{
    char *start;

    start = buf;
    while (len--) *(buf++) = init;
    return(start);
}


/************************/
/*  Function memcpy()   */
/************************/

char *
memcpy(dst,src,len)             /* v2.0f */
register char *dst, *src;
register unsigned int len;
{
    char *start;

    start = dst;
    while (len--)
        *dst++ = *src++;
    return(start);
}


/************************/
/*  Function memcmp()   */
/************************/

int
memcmp(b1,b2,len)                     /* jpd@usl.edu -- 11/16/90 */
register char *b1, *b2;
register unsigned int len;
{

    if (len) do {       /* examine each byte (if any) */
      if (*b1++ != *b2++)
        return (*((uch *)b1-1) - *((uch *)b2-1));  /* exit when miscompare */
    } while (--len);

    return(0);          /* no miscompares, yield 0 result */
}

#endif  /* ZMEM */

/*---------------------------------------------------------------
 *  Long option support
 *  8/23/2003
 *
 *  Defines function get_option() to get and process the command
 *  line options and arguments from argv[].  The caller calls
 *  get_option() in a loop to get either one option and possible
 *  value or a non-option argument each loop.
 *
 *  This version does not include argument file support and can
 *  work directly on argv.  The argument file code complicates things and
 *  it seemed best to leave it out for now.  If argument file support (reading in
 *  command line arguments stored in a file and inserting into
 *  command line where @filename is found) is added later the arguments
 *  can change and a freeable copy of argv will be needed and can be
 *  created using copy_args in the left out code.
 *
 *  Supports short and long options as defined in the array options[]
 *  in zip.c, multiple short options in an argument (like -jlv), long
 *  option abbreviation (like --te for --temp-file if --te unique),
 *  short and long option values (like -b filename or --temp-file filename
 *  or --temp-file=filename), optional and required values, option negation
 *  by trailing - (like -S- to not include hidden and system files in MSDOS),
 *  value lists (like -x a b c), argument permuting (returning all options
 *  and values before any non-option arguments), and argument files (where any
 *  non-option non-value argument in form @path gets substituted with the
 *  white space separated arguments in the text file at path).  In this
 *  version argument file support has been removed to simplify development but
 *  may be added later.
 *
 *  E. Gordon
 */


/* message output - char casts are needed to handle constants */
#define oWARN(message) zipwarn((char *) message, "")
#define oERR(err,message) ZIPERR(err, (char *) message)

/* Although the below provides some support for multibyte characters
   the proper thing to do may be to use wide characters and support
   Unicode.  May get to it soon.  EG
 */
#ifdef _MBCS
#  ifndef MULTIBYTE_GETOPTNS
#    define MULTIBYTE_GETOPTNS
#  endif
#endif
/* multibyte character set support
   Multibyte characters use typically two or more sequential bytes
   to represent additional characters than can fit in a single byte
   character set.  The code used here is based on the ANSI mblen function. */
#ifdef MULTIBYTE_GETOPTNS
  local int mb_clen OF((ZCONST char *));  /* declare proto first */
  local int mb_clen(ptr)
    ZCONST char *ptr;
  {
    /* return the number of bytes that the char pointed to is.  Return 1 if
       null character or error like not start of valid multibyte character. */
    int cl;

    cl = mblen(ptr, MB_CUR_MAX);
    return (cl > 0) ? cl : 1;
  }
# define MB_CLEN(ptr) mb_clen(ptr)
# define MB_NEXTCHAR(ptr) ((ptr) += MB_CLEN(ptr))
#else
# define MB_CLEN(ptr) (1)
# define MB_NEXTCHAR(ptr) ((ptr)++)
#endif


/* constants */

/* function get_args_from_arg_file() can return this in depth parameter */
#define ARG_FILE_ERR -1

/* internal settings for optchar */
#define SKIP_VALUE_ARG   -1
#define THIS_ARG_DONE    -2
#define START_VALUE_LIST -3
#define IN_VALUE_LIST    -4
#define NON_OPTION_ARG   -5
#define STOP_VALUE_LIST  -6
/* 7/25/04 EG */
#define READ_REST_ARGS_VERBATIM -7


/* global veriables */

int enable_permute = 1;                     /* yes - return options first */
/* 7/25/04 EG */
int doubledash_ends_options = 1;            /* when -- what follows are not options */

/* buffer for error messages (this sizing is a guess but must hold 2 paths) */
#define OPTIONERR_BUF_SIZE (FNMAX * 2 + 200)
char optionerrbuf[OPTIONERR_BUF_SIZE + 1];

/* error messages */
static ZCONST char Far op_not_neg_err[] = "option %s not negatable";
static ZCONST char Far op_req_val_err[] = "option %s requires a value";
static ZCONST char Far op_no_allow_val_err[] = "option %s does not allow a value";
static ZCONST char Far sh_op_not_sup_err[] = "short option '%c' not supported";
static ZCONST char Far oco_req_val_err[] = "option %s requires one character value";
static ZCONST char Far oco_no_mbc_err[] = "option %s does not support multibyte values";
static ZCONST char Far num_req_val_err[] = "option %s requires number value";
static ZCONST char Far long_op_ambig_err[] = "long option '%s' ambiguous";
static ZCONST char Far long_op_not_sup_err[] = "long option '%s' not supported";

static ZCONST char Far no_arg_files_err[] = "argument files not enabled\n";


/* below removed as only used for processing argument files */

/* copy_args */
/* free_args */
/* insert_arg */
/* get_nextarg */
/* get_args_from_string */
/* insert_args */
/* get_args_from_arg_file */


/* copy error, option name, and option description if any to buf */
local int optionerr(buf, err, optind, islong)
  char *buf;
  ZCONST char *err;
  int optind;
  int islong;
{
  char optname[50];

  if (options[optind].name && options[optind].name[0] != '\0') {
    if (islong)
      sprintf(optname, "'%s' (%s)", options[optind].longopt, options[optind].name);
    else
      sprintf(optname, "'%s' (%s)", options[optind].shortopt, options[optind].name);
  } else {
    if (islong)
      sprintf(optname, "'%s'", options[optind].longopt);
    else
      sprintf(optname, "'%s'", options[optind].shortopt);
  }
  sprintf(buf, err, optname);
  return 0;
}


/* get_shortopt
 *
 * Get next short option from arg.  The state is stored in argnum, optchar, and
 * option_num so no static storage is used.  Returns the option_ID.
 *
 * parameters:
 *    args        - argv array of arguments
 *    argnum      - index of current arg in args
 *    optchar     - pointer to index of next char to process.  Can be 0 or
 *                  const defined at top of this file like THIS_ARG_DONE
 *    negated     - on return pointer to int set to 1 if option negated or 0 otherwise
 *    value       - on return pointer to string set to value of option if any or NULL
 *                  if none.  If value is returned then the caller should free()
 *                  it when not needed anymore.
 *    option_num  - pointer to index in options[] of returned option or
 *                  o_NO_OPTION_MATCH if none.  Do not change as used by
 *                  value lists.
 *    depth       - recursion depth (0 at top level, 1 or more in arg files)
 */
local unsigned long get_shortopt(args, argnum, optchar, negated, value, option_num, depth)
  char **args;
  int argnum;
  int *optchar;
  int *negated;
  char **value;
  int *option_num;
  int depth;
{
  char *shortopt;
  int clen;
  char *nextchar;
  char *s;
  char *start;
  int op;
  char *arg;
  long match = -1;


  /* get arg */
  arg = args[argnum];
  /* current char in arg */
  nextchar = arg + (*optchar);
  clen = MB_CLEN(nextchar);
  /* next char in arg */
  (*optchar) +=  clen;
  /* get first char of short option */
  shortopt = arg + (*optchar);
  /* no value */
  *value = NULL;

  if (*shortopt == '\0') {
    /* no more options in arg */
    *optchar = 0;
    *option_num = o_NO_OPTION_MATCH;
    return 0;
  }

  /* look for match in options */
  clen = MB_CLEN(shortopt);
  for (op = 0; options[op].option_ID; op++) {
    s = options[op].shortopt;
    if (s && s[0] == shortopt[0]) {
      if (s[1] == '\0' && clen == 1) {
        /* single char match */
        match = op;
      } else {
        /* 2 wide short opt.  Could support more chars but should use long opts instead */
        if (s[1] == shortopt[1]) {
          /* match 2 char short opt or 2 byte char */
          match = op;
          if (clen == 1) (*optchar)++;
          break;
        }
      }
    }
  }

  if (match > -1) {
    /* match */
    clen = MB_CLEN(shortopt);
    nextchar = arg + (*optchar) + clen;
    /* check for trailing dash negating option */
    if (*nextchar == '-') {
      /* negated */
      if (options[match].negatable == o_NOT_NEGATABLE) {
        if (options[match].value_type == o_NO_VALUE) {
          optionerr(optionerrbuf, op_not_neg_err, match, 0);
          if (depth > 0) {
            /* unwind */
            oWARN(optionerrbuf);
            return o_ARG_FILE_ERR;
          } else {
            oERR(ZE_PARMS, optionerrbuf);
          }
        }
      } else {
        *negated = 1;
        /* set up to skip negating dash */
        (*optchar) += clen;
        clen = 1;
      }
    }

    /* value */
    clen = MB_CLEN(arg + (*optchar));
    /* optional value, one char value, and number value must follow option */
    if (options[match].value_type == o_ONE_CHAR_VALUE) {
      /* one char value */
      if (arg[(*optchar) + clen]) {
        /* has value */
        if (MB_CLEN(arg + (*optchar) + clen) > 1) {
          /* multibyte value not allowed for now */
          optionerr(optionerrbuf, oco_no_mbc_err, match, 0);
          if (depth > 0) {
            /* unwind */
            oWARN(optionerrbuf);
            return o_ARG_FILE_ERR;
          } else {
            oERR(ZE_PARMS, optionerrbuf);
          }
        }
        if ((*value = (char *) malloc(2)) == NULL) {
          oERR(ZE_MEM, "gso");
        }
        (*value)[0] = *(arg + (*optchar) + clen);
        (*value)[1] = '\0';
        *optchar += clen;
        clen = 1;
      } else {
        /* one char values require a value */
        optionerr(optionerrbuf, oco_req_val_err, match, 0);
        if (depth > 0) {
          oWARN(optionerrbuf);
          return o_ARG_FILE_ERR;
        } else {
          oERR(ZE_PARMS, optionerrbuf);
        }
      }
    } else if (options[match].value_type == o_NUMBER_VALUE) {
      /* read chars until end of number */
      start = arg + (*optchar) + clen;
      if (*start == '+' || *start == '-') {
        start++;
      }
      s = start;
      for (; isdigit(*s); MB_NEXTCHAR(s)) ;
      if (s == start) {
        /* no digits */
        optionerr(optionerrbuf, num_req_val_err, match, 0);
        if (depth > 0) {
          oWARN(optionerrbuf);
          return o_ARG_FILE_ERR;
        } else {
          oERR(ZE_PARMS, optionerrbuf);
        }
      }
      start = arg + (*optchar) + clen;
      if ((*value = (char *) malloc((int)(s - start) + 1)) == NULL) {
        oERR(ZE_MEM, "gso");
      }
      *optchar += (int)(s - start);
      strncpy(*value, start, (int)(s - start));
      (*value)[(int)(s - start)] = '\0';
      clen = MB_CLEN(s);
    } else if (options[match].value_type == o_OPTIONAL_VALUE) {
      /* optional value */
      /* This was inconsistent so now if no value attached to argument look to next
         argument if that argument is not an option for option value - 11/12/04 EG */
      if (arg[(*optchar) + clen]) {
        /* has value */
        /* add support for optional = - 2/6/05 EG */
        if (arg[(*optchar) + clen] == '=') {
          /* skip = */
          clen++;
        }
        if (arg[(*optchar) + clen]) {
          if ((*value = (char *) malloc(strlen(arg + (*optchar) + clen) + 1)) == NULL) {
            oERR(ZE_MEM, "gso");
          }
          strcpy(*value, arg + (*optchar) + clen);
        }
        *optchar = THIS_ARG_DONE;
      } else if (args[argnum + 1] && args[argnum + 1][0] != '-') {
        /* use next arg for value */
        if ((*value = (char *) malloc(strlen(args[argnum + 1]) + 1)) == NULL) {
          oERR(ZE_MEM, "gso");
        }
        /* using next arg as value */
        strcpy(*value, args[argnum + 1]);
        *optchar = SKIP_VALUE_ARG;
      }
    } else if (options[match].value_type == o_REQUIRED_VALUE ||
               options[match].value_type == o_VALUE_LIST) {
      /* see if follows option */
      if (arg[(*optchar) + clen]) {
        /* has value following option as -ovalue */
        /* add support for optional = - 6/5/05 EG */
        if (arg[(*optchar) + clen] == '=') {
          /* skip = */
          clen++;
        }
        if ((*value = (char *) malloc(strlen(arg + (*optchar) + clen) + 1)) == NULL) {
          oERR(ZE_MEM, "gso");
        }
        strcpy(*value, arg + (*optchar) + clen);
        *optchar = THIS_ARG_DONE;
      } else {
        /* use next arg for value */
        if (args[argnum + 1]) {
          if ((*value = (char *) malloc(strlen(args[argnum + 1]) + 1)) == NULL) {
            oERR(ZE_MEM, "gso");
          }
          /* using next arg as value */
          strcpy(*value, args[argnum + 1]);
          if (options[match].value_type == o_VALUE_LIST) {
            *optchar = START_VALUE_LIST;
          } else {
            *optchar = SKIP_VALUE_ARG;
          }
        } else {
          /* no value found */
          optionerr(optionerrbuf, op_req_val_err, match, 0);
          if (depth > 0) {
            oWARN(optionerrbuf);
            return o_ARG_FILE_ERR;
          } else {
            oERR(ZE_PARMS, optionerrbuf);
          }
        }
      }
    }

    *option_num = match;
    return options[match].option_ID;
  }
  sprintf(optionerrbuf, sh_op_not_sup_err, *shortopt);
  if (depth > 0) {
    /* unwind */
    oWARN(optionerrbuf);
    return o_ARG_FILE_ERR;
  } else {
    oERR(ZE_PARMS, optionerrbuf);
  }
  return 0;
}


/* get_longopt
 *
 * Get the long option in args array at argnum.  Parameters same as for get_shortopt.
 */

local unsigned long get_longopt(args, argnum, optchar, negated, value, option_num, depth)
  char **args;
  int argnum;
  int *optchar;
  int *negated;
  char **value;
  int *option_num;
  int depth;
{
  char *longopt;
  char *lastchr;
  char *valuestart;
  int op;
  char *arg;
  int match = -1;
  *value = NULL;

  if (args == NULL) {
    *option_num = o_NO_OPTION_MATCH;
    return 0;
  }
  if (args[argnum] == NULL) {
    *option_num = o_NO_OPTION_MATCH;
    return 0;
  }
  /* copy arg so can chop end if value */
  if ((arg = (char *) malloc(strlen(args[argnum]) + 1)) == NULL) {
    oERR(ZE_MEM, "glo");
  }
  strcpy(arg, args[argnum]);

  /* get option */
  longopt = arg + 2;
  /* no value */
  *value = NULL;

  /* find = */
  for (lastchr = longopt, valuestart = longopt;
       *valuestart && *valuestart != '=';
       lastchr = valuestart, MB_NEXTCHAR(valuestart)) ;
  if (*valuestart) {
    /* found =value */
    *valuestart = '\0';
    valuestart++;
  } else {
    valuestart = NULL;
  }

  if (*lastchr == '-') {
    /* option negated */
    *negated = 1;
    *lastchr = '\0';
  } else {
    *negated = 0;
  }

  /* look for long option match */
  for (op = 0; options[op].option_ID; op++) {
    if (options[op].longopt && strcmp(options[op].longopt, longopt) == 0) {
      /* exact match */
      match = op;
      break;
    }
    if (options[op].longopt && strncmp(options[op].longopt, longopt, strlen(longopt)) == 0) {
      if (match > -1) {
        sprintf(optionerrbuf, long_op_ambig_err, longopt);
        if (depth > 0) {
          /* unwind */
          oWARN(optionerrbuf);
          return o_ARG_FILE_ERR;
        } else {
          oERR(ZE_PARMS, optionerrbuf);
        }
      }
      match = op;
    }
  }

  if (match == -1) {
    sprintf(optionerrbuf, long_op_not_sup_err, longopt);
    free(arg);
    if (depth > 0) {
      oWARN(optionerrbuf);
      return o_ARG_FILE_ERR;
    } else {
      oERR(ZE_PARMS, optionerrbuf);
    }
  }

  /* one long option an arg */
  *optchar = THIS_ARG_DONE;

  /* if negated then see if allowed */
  if (*negated && options[match].negatable == o_NOT_NEGATABLE) {
    optionerr(optionerrbuf, op_not_neg_err, match, 1);
    free(arg);
    if (depth > 0) {
      /* unwind */
      oWARN(optionerrbuf);
      return o_ARG_FILE_ERR;
    } else {
      oERR(ZE_PARMS, optionerrbuf);
    }
  }
  /* get value */
  if (options[match].value_type == o_OPTIONAL_VALUE) {
    /* optional value in form option=value */
    if (valuestart) {
      /* option=value */
      if ((*value = (char *) malloc(strlen(valuestart) + 1)) == NULL) {
        free(arg);
        oERR(ZE_MEM, "glo");
      }
      strcpy(*value, valuestart);
    }
  } else if (options[match].value_type == o_REQUIRED_VALUE ||
             options[match].value_type == o_NUMBER_VALUE ||
             options[match].value_type == o_ONE_CHAR_VALUE ||
             options[match].value_type == o_VALUE_LIST) {
    /* handle long option one char and number value as required value */
    if (valuestart) {
      /* option=value */
      if ((*value = (char *) malloc(strlen(valuestart) + 1)) == NULL) {
        free(arg);
        oERR(ZE_MEM, "glo");
      }
      strcpy(*value, valuestart);
    } else {
      /* use next arg */
      if (args[argnum + 1]) {
        if ((*value = (char *) malloc(strlen(args[argnum + 1]) + 1)) == NULL) {
          free(arg);
          oERR(ZE_MEM, "glo");
        }
        /* using next arg as value */
        strcpy(*value, args[argnum + 1]);
        if (options[match].value_type == o_VALUE_LIST) {
          *optchar = START_VALUE_LIST;
        } else {
          *optchar = SKIP_VALUE_ARG;
        }
      } else {
        /* no value found */
        optionerr(optionerrbuf, op_req_val_err, match, 1);
        free(arg);
        if (depth > 0) {
          /* unwind */
          oWARN(optionerrbuf);
          return o_ARG_FILE_ERR;
        } else {
          oERR(ZE_PARMS, optionerrbuf);
        }
      }
    }
  } else if (options[match].value_type == o_NO_VALUE) {
    /* this option does not accept a value */
    if (valuestart) {
      /* --option=value */
      optionerr(optionerrbuf, op_no_allow_val_err, match, 1);
      if (depth > 0) {
        oWARN(optionerrbuf);
        return o_ARG_FILE_ERR;
      } else {
        oERR(ZE_PARMS, optionerrbuf);
      }
    }
  }
  free(arg);

  *option_num = match;
  return options[match].option_ID;
}



/* get_option
 *
 * Main interface for user.  Use this function to get options, values and
 * non-option arguments from a command line provided in argv form.
 *
 * To use get_option() first define valid options by setting
 * the global variable options[] to an array of option_struct.  Also
 * either change defaults below or make variables global and set elsewhere.
 * Zip uses below defaults.
 *
 * Call get_option() to get an option (like -b or --temp-file) and any
 * value for that option (like filename for -b) or a non-option argument
 * (like archive name) each call.  If *value* is not NULL after calling
 * get_option() it is a returned value and the caller should either store
 * the char pointer or free() it before calling get_option() again to avoid
 * leaking memory.  If a non-option non-value argument is returned get_option()
 * returns o_NON_OPTION_ARG and value is set to the entire argument.  When there
 * are no more arguments get_option() returns 0.
 *
 * The parameters argnum (after set to 0 on initial call),
 * optchar, first_nonopt_arg, option_num, and depth (after initial
 * call) are set and maintained by get_option() and should not be
 * changed.  The parameters argc, negated, and value are outputs and
 * can be used by the calling program.  get_option() returns either the
 * option_ID for the current option, a special value defined in
 * zip.h, or 0 when no more arguments.
 *
 * The value returned by get_option() is the ID value in the options
 * table.  This value can be duplicated in the table if different
 * options are really the same option.  The index into the options[]
 * table is given by option_num, though the ID should be used as
 * option numbers change when the table is changed.  The ID must
 * not be 0 for any option as this ends the table.  If get_option()
 * finds an option not in the table it calls oERR to post an
 * error and exit.  Errors also result if the option requires a
 * value that is missing, a value is present but the option does
 * not take one, and an option is negated but is not
 * negatable.  Non-option arguments return o_NON_OPTION_ARG
 * with the entire argument in value.
 *
 * For Zip, permuting is on and all options and their values are
 * returned before any non-option arguments like archive name.
 *
 * The arguments "-" alone and "--" alone return as non-option arguments.
 * Note that "-" should not be used as part of a short option
 * entry in the table but can be used in the middle of long
 * options such as in the long option "a-long-option".  This version
 * does not handle "--" uniquely, such as stopping argument processing.
 *
 * Argument file support is removed from this version. It may be added later.
 *
 * After each call:
 *   argc       is set to the current size of args[] but should not change
 *                with argument file support removed,
 *   argnum     is the index of the current arg,
 *   value      is either the value of the returned option or non-option
 *                argument or NULL if option with no value,
 *   negated    is set if the option was negated by a trailing dash (-)
 *   option_num is set to either the index in options[] for the option or
 *                o_NO_OPTION_MATCH if no match.
 * Negation is checked before the value is read if the option is negatable so that
 * the - is not included in the value.  If the option is not negatable but takes
 * a value then the - will start the value.  If permuting then argnum and
 * first_nonopt_arg are unreliable and should not be used.
 *
 * Command line is read from left to right.  As get_option() finds non-option
 * arguments (arguments not starting with - and that are not values to options) it
 * moves later options and values in front of the non-option arguments.  This
 * permuting is turned off by setting enable_permute to 0.  Then get_option() will
 * return options and non-option arguments in the order found.  Currently
 * permuting is only done after an argument is completely processed so
 * that any value can be moved with options they go with.  All state information is
 * stored in the parameters argnum, optchar, first_nonopt_arg and option_num.  You
 * should not change these after the first call to get_option().  If you need to
 * back up to a previous arg then set argnum to that arg (remembering that
 * args may have been permuted) and set optchar = 0 and first_nonopt_arg to
 * the first non-option argument if permuting.  After all arguments are returned
 * the next call to get_option() returns 0.  The caller can then call
 * free_args(args) if appropriate.
 *
 * get_option() accepts arguments in the following forms:
 *  short options
 *       of 1 and 2 characters, e.g. a, b, cc, d, and ba, after a single
 *       leading -, as in -abccdba.  In this example if 'b' is followed by 'a'
 *       it matches short option 'ba' else it is interpreted as short option
 *       b followed by another option.  The character - is not legal as a
 *       short option or as part of a 2 character short option.
 *
 *       If a short option has a value it immediately follows the option or
 *       if that option is the end of the arg then the next arg is used as
 *       the value.  So if short option e has a value, it can be given as
 *             -evalue
 *       or
 *             -e value
 *       and now
 *             -e=value
 *       but now that = is optional a leading = is stripped for the first.
 *       This change allows optional short option values to be defaulted as
 *             -e=
 *       Either optional or required values can be specified.  Optional values
 *       now use both forms as ignoring the later got confusing.  Any
 *       non-value short options can preceed a valued short option as in
 *             -abevalue
 *       Some value types (one_char and number) allow options after the value
 *       so if oc is an option that takes a character and n takes a number
 *       then
 *             -abocVccn42evalue
 *       returns value V for oc and value 42 for n.  All values are strings
 *       so programs may have to convert the "42" to a number.  See long
 *       options below for how value lists are handled.
 *
 *       Any short option can be negated by following it with -.  Any - is
 *       handled and skipped over before any value is read unless the option
 *       is not negatable but takes a value and then - starts the value.
 *
 *       If the value for an optional value is just =, then treated as no
 *       value.
 *
 *  long options
 *       of arbitrary length are assumed if an arg starts with -- but is not
 *       exactly --.  Long options are given one per arg and can be abbreviated
 *       if the abbreviation uniquely matches one of the long options.
 *       Exact matches always match before partial matches.  If ambiguous an
 *       error is generated.
 *
 *       Values are specified either in the form
 *             --longoption=value
 *       or can be the following arg if the value is required as in
 *             --longoption value
 *       Optional values to long options must be in the first form.
 *
 *       Value lists are specified by o_VALUE_LIST and consist of an option
 *       that takes a value followed by one or more value arguments.
 *       The two forms are
 *             --option=value
 *       or
 *             -ovalue
 *       for a single value or
 *             --option value1 value2 value3 ... --option2
 *       or
 *             -o value1 value2 value3 ...
 *       for a list of values.  The list ends at the next option, the
 *       end of the command line, or at a single "@" argument.
 *       Each value is treated as if it was preceeded by the option, so
 *             --option1 val1 val2
 *       with option1 value_type set to o_VALUE_LIST is the same as
 *             --option1=val1 --option1=val2
 *
 *       Long options can be negated by following the option with - as in
 *             --longoption-
 *       Long options with values can also be negated if this makes sense for
 *       the caller as:
 *             --longoption-=value
 *       If = is not followed by anything it is treated as no value.
 *
 *  @path
 *       Argument files support removed from this version.  It may be added
 *       back later.
 *
 *  non-option argument
 *       is any argument not given above.  If enable_permute is 1 then
 *       these are returned after all options, otherwise all options and
 *       args are returned in order.  Returns option ID o_NON_OPTION_ARG
 *       and sets value to the argument.
 *
 *
 * Arguments to get_option:
 *  char ***pargs          - pointer to arg array in the argv form
 *  int *argc              - returns the current argc for args including args[0]
 *  int *argnum            - the index of the current argument (caller
 *                            should set = 0 on first call and not change after that)
 *  int *optchar           - index of next short opt in arg or special
 *  int *first_nonopt_arg  - used by get_option to permute args
 *  int *negated           - option was negated (had trailing -)
 *  char *value            - value of option if any (free when done with it) or NULL
 *  int *option_num        - the index in options of the last option returned
 *                            (can be o_NO_OPTION_MATCH)
 *  int recursion_depth    - current depth of recursion (always set to 0 by caller)
 *                            (always 0 with argument files support removed)
 *
 *  Caller should only read the returned option ID and the value, negated,
 *  and option_num (if required) parameters after each call.
 *
 *  Ed Gordon
 *  8/24/2003 (last updated 2/5/2005 EG)
 *
 */
unsigned long get_option(pargs, argc, argnum, optchar, value,
                         negated, first_nonopt_arg, option_num, recursion_depth)
  char ***pargs;
  int *argc;
  int *argnum;
  int *optchar;
  char **value;
  int *negated;
  int *first_nonopt_arg;
  int *option_num;
  int recursion_depth;
{
  char **args;
  unsigned long option_ID;

  int argcnt;
  int first_nonoption_arg;
  char *arg = NULL;
  int h;
  int optc;
  int argn;
  int j;
  int v;
  int read_rest_args_verbatim = 0;  /* 7/25/04 - ignore options and arg files for rest args */

  /* value is outdated.  The caller should free value before
     calling get_option again. */
  *value = NULL;

  /* if args NULL done */
  if (pargs == NULL) {
    *argc = 0;
    return 0;
  }
  args = *pargs;
  if (args == NULL) {
    *argc = 0;
    return 0;
  }

  /* count args */
  for (argcnt = 0; args[argcnt]; argcnt++) ;

  /* if no provided args nothing to do */
  if (argcnt < 1 || (recursion_depth == 0 && argcnt < 2)) {
    *argc = argcnt;
    return 0;
  }

  *negated = 0;
  first_nonoption_arg = *first_nonopt_arg;
  argn = *argnum;
  optc = *optchar;

  if (optc == READ_REST_ARGS_VERBATIM) {
    read_rest_args_verbatim = 1;
  }

  if (argn == -1 || (recursion_depth == 0 && argn == 0)) {
    /* first call */
    /* if depth = 0 then args[0] is argv[0] so skip */
    *option_num = o_NO_OPTION_MATCH;
    optc = THIS_ARG_DONE;
    first_nonoption_arg = -1;
  }

  /* last option_ID is returned if continuing value list */
  option_ID = 0;
  if (*option_num != o_NO_OPTION_MATCH) {
    option_ID = options[*option_num].option_ID;
  }

  /* get next option if any */
  for (;;)  {
    if (read_rest_args_verbatim) {
      /* rest after -- are non-option args if doubledash_ends_options set */
      argn++;
      if (argn > argcnt || args[argn] == NULL) {
        /* done */
        option_ID = 0;
        break;
      }
      arg = args[argn];
      if ((*value = (char *) malloc(strlen(arg) + 1)) == NULL) {
        oERR(ZE_MEM, "go");
      }
      strcpy(*value, arg);
      *option_num = o_NO_OPTION_MATCH;
      option_ID = o_NON_OPTION_ARG;
      break;

    /* permute non-option args after option args */
    } else if (enable_permute) {
      if (optc == SKIP_VALUE_ARG || optc == THIS_ARG_DONE ||
          optc == START_VALUE_LIST || optc == IN_VALUE_LIST ||
          optc == STOP_VALUE_LIST) {
        /* moved to new arg */
        if (first_nonoption_arg > -1 && args[first_nonoption_arg]) {
          /* move non-options after this option */
          /* if option and value separate args or starting list skip option */
          if (optc == SKIP_VALUE_ARG || optc == START_VALUE_LIST) {
            v = 1;
          } else {
            v = 0;
          }
          for (h = first_nonoption_arg; h < argn; h++) {
            arg = args[first_nonoption_arg];
            for (j = first_nonoption_arg; j < argn + v; j++) {
              args[j] = args[j + 1];
            }
            args[j] = arg;
          }
          first_nonoption_arg += 1 + v;
        }
      }
    } else if (optc == NON_OPTION_ARG) {
      /* if not permuting then already returned arg */
      optc = THIS_ARG_DONE;
    }

    /* value lists */
    if (optc == STOP_VALUE_LIST) {
      optc = THIS_ARG_DONE;
    }

    if (optc == START_VALUE_LIST || optc == IN_VALUE_LIST) {
      if (optc == START_VALUE_LIST) {
        /* already returned first value */
        argn ++;
        optc = IN_VALUE_LIST;
      }
      argn++;
      arg = args[argn];
      /* if end of args and still in list and non-option args then terminate list */
      if (arg == NULL && (optc == START_VALUE_LIST || optc == IN_VALUE_LIST)
          && first_nonoption_arg > -1) {
        /* terminate value list with @ */
        /* this is only needed for argument files */
        /*
        argcnt = insert_arg(&args, "@", first_nonoption_arg, 1);
        argn++;
        if (first_nonoption_arg > -1) {
          first_nonoption_arg++;
        }
        */
      }

      arg = args[argn];
      if (arg && arg[0] == '@' && arg[1] == '\0') {
          /* inserted arguments terminator */
          optc = STOP_VALUE_LIST;
          continue;
      } else if (arg && arg[0] != '-') {  /* not option */
        /* - and -- are not allowed in value lists unless escaped */
        /* another value in value list */
        if ((*value = (char *) malloc(strlen(args[argn]) + 1)) == NULL) {
          oERR(ZE_MEM, "go");
        }
        strcpy(*value, args[argn]);
        break;

      } else {
        argn--;
        optc = THIS_ARG_DONE;
      }
    }

    /* move to next arg */
    if (optc == SKIP_VALUE_ARG) {
      argn += 2;
      optc = 0;
    } else if (optc == THIS_ARG_DONE) {
      argn++;
      optc = 0;
    }
    if (argn > argcnt) {
      break;
    }
    if (args[argn] == NULL) {
      /* done unless permuting and non-option args */
      if (first_nonoption_arg > -1 && args[first_nonoption_arg]) {
        /* return non-option arguments at end */
        if (optc == NON_OPTION_ARG) {
          first_nonoption_arg++;
        }
        /* after first pass args are permuted but skipped over non-option args */
        /* swap so argn points to first non-option arg */
        j = argn;
        argn = first_nonoption_arg;
        first_nonoption_arg = j;
      }
      if (argn > argcnt || args[argn] == NULL) {
        /* done */
        option_ID = 0;
        break;
      }
    }

    /* after swap first_nonoption_arg points to end which is NULL */
    if (first_nonoption_arg > -1 && (args[first_nonoption_arg] == NULL)) {
      /* only non-option args left */
      if (optc == NON_OPTION_ARG) {
        argn++;
      }
      if (argn > argcnt || args[argn] == NULL) {
        /* done */
        option_ID = 0;
        break;
      }
      if ((*value = (char *) malloc(strlen(args[argn]) + 1)) == NULL) {
        oERR(ZE_MEM, "go");
      }
      strcpy(*value, args[argn]);
      optc = NON_OPTION_ARG;
      option_ID = o_NON_OPTION_ARG;
      break;
    }

    arg = args[argn];

    /* is it an option */
    if (arg[0] == '-') {
      /* option */
      if (arg[1] == '\0') {
        /* arg = - */
        /* treat like non-option arg */
        *option_num = o_NO_OPTION_MATCH;
        if (enable_permute) {
          /* permute args to move all non-option args to end */
          if (first_nonoption_arg < 0) {
            first_nonoption_arg = argn;
          }
          argn++;
        } else {
          /* not permute args so return non-option args when found */
          if ((*value = (char *) malloc(strlen(arg) + 1)) == NULL) {
            oERR(ZE_MEM, "go");
          }
          strcpy(*value, arg);
          optc = NON_OPTION_ARG;
          option_ID = o_NON_OPTION_ARG;
          break;
        }

      } else if (arg[1] == '-') {
        /* long option */
        if (arg[2] == '\0') {
          /* arg = -- */
          if (doubledash_ends_options) {
            /* Now -- stops permuting and forces the rest of
               the command line to be read verbetim - 7/25/04 EG */

            /* never permute args after -- and return as non-option args */
            if (first_nonoption_arg < 1) {
              /* -- is first non-option argument - 8/7/04 EG */
              argn--;
            } else {
              /* go back to start of non-option args - 8/7/04 EG */
              argn = first_nonoption_arg - 1;
            }

            /* disable permuting and treat remaining arguments as not options */
            read_rest_args_verbatim = 1;
            optc = READ_REST_ARGS_VERBATIM;

          } else {
            /* treat like non-option arg */
            *option_num = o_NO_OPTION_MATCH;
            if (enable_permute) {
              /* permute args to move all non-option args to end */
              if (first_nonoption_arg < 0) {
                first_nonoption_arg = argn;
              }
              argn++;
            } else {
              /* not permute args so return non-option args when found */
              if ((*value = (char *) malloc(strlen(arg) + 1)) == NULL) {
                oERR(ZE_MEM, "go");
              }
              strcpy(*value, arg);
              optc = NON_OPTION_ARG;
              option_ID = o_NON_OPTION_ARG;
              break;
            }
          }

        } else {
          option_ID = get_longopt(args, argn, &optc, negated, value, option_num, recursion_depth);
          if (option_ID == o_ARG_FILE_ERR) {
            /* unwind as only get this if recursion_depth > 0 */
            return option_ID;
          }
          break;
        }

      } else {
        /* short option */
        option_ID = get_shortopt(args, argn, &optc, negated, value, option_num, recursion_depth);

        if (option_ID == o_ARG_FILE_ERR) {
          /* unwind as only get this if recursion_depth > 0 */
          return option_ID;
        }

        if (optc == 0) {
          /* if optc = 0 then ran out of short opts this arg */
          optc = THIS_ARG_DONE;
        } else {
          break;
        }
      }

#if 0
    /* argument file code left out
       so for now let filenames start with @
    */

    } else if (allow_arg_files && arg[0] == '@') {
      /* arg file */
      oERR(ZE_PARMS, no_arg_files_err);
#endif

    } else {
      /* non-option */
      if (enable_permute) {
        /* permute args to move all non-option args to end */
        if (first_nonoption_arg < 0) {
          first_nonoption_arg = argn;
        }
        argn++;
      } else {
        /* no permute args so return non-option args when found */
        if ((*value = (char *) malloc(strlen(arg) + 1)) == NULL) {
          oERR(ZE_MEM, "go");
        }
        strcpy(*value, arg);
        *option_num = o_NO_OPTION_MATCH;
        optc = NON_OPTION_ARG;
        option_ID = o_NON_OPTION_ARG;
        break;
      }

    }
  }

  *pargs = args;
  *argc = argcnt;
  *first_nonopt_arg = first_nonoption_arg;
  *argnum = argn;
  *optchar = optc;

  return option_ID;
}
