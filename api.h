/*
  api.h - Zip 3

  Copyright (c) 1990-2013 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-2 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/

/* api.h and api.c are a callable interface to Zip.  The following entry points
   are defined:

     ZpVersion   - Returns version of the DLL or LIB
     ZpInit      - Initializes the DLL or LIB
     ZpArchive   - Main entry point for the DLL and LIB (does the actual work)
     zipmain     - When used as a LIB on some platforms, zipmain (in zip.c) called
                    directly rather than via ZpArchive() - also used when the
                    Zip source is included directly (USE_ZIPMAIN_NO_API)

   In addition, these callback functions are used:

     DLLPRNT             - Called to print strings
     DLLPASSWORD         - Called to request a password
     DLLSERVICE          - Called as each entry is processed
                           Setting return code to 1 aborts zip operation
     DLLSERVICE_NO_INT64 - A version of DLLSERVICE that does not use 64-bit args,
                           but instead uses high and low 32-bit parts - this is
                           needed for Visual Basic, for instance
     DLLSPLIT            - Called to request split destination if splitting
     DLLCOMMENT          - Called to request a comment
     DLLPROGRESS         - (Optional) Called to report progress, returns:
                             (entry name, % all bytes processed, % entry procd)
                           Setting return code to 1 aborts zip operation

   The ZPOPT structure is used to pass settings, such as root directory, to Zip
   via the ZPArchive call.  If ZPArchive is not used, the values will need to
   be set using options passed to zipmain().  Not all fields in ZPOPT may be
   settable using command line options.

   Only the Windows DLL is currently supported */

/* There are basically three ways to include Zip functionality:
     DLL     - Compile Zip as a Dynamic Load Library (DLL) and call it from the
                application
     LIB     - Compile Zip as a Static Load Library (LIB), link it in, and call it
                from the application
     in line - Include the Zip source in the application directly and call Zip
                via zipmain (the user will need to handle any reentry issues)

   The following macros determine how this interface is compiled:
     WINDLL               - Compile for WIN32 DLL, mainly for MS Visual Studio
                            WINDLL implies WIN32 and ZIPDLL
                            Set by the build environment
     WIN32                - Running on WIN32
                            The compiler usually sets this
     ZIPDLL               - Compile the Zip DLL (WINDLL implies this)
                            Set by the build environment
     WIN32 and ZIPDLL     - A DLL for WIN32 (except MS C++ - use WINDLL instead)
                             - this is used for creating DLLs for other ports
                            (such as OS2), but only the MSVS version is currently
                            supported
     USE_ZIPMAIN          - Use zipmain() instead of main() - for DLL and LIB, but
                             could also be used to compile the source into another
                             application directly
                            Usually set by ZIPDLL or ZIPLIB, but can be set in the
                             build environment in unique situations
     ZIPLIB               - Compile as static library (LIB)
                            For MSVS, WINDLL and ZIPLIB are both defined to create
                             the LIB
     ZIP_DLL_LIB          - Set in zip.h when either ZIPDLL or ZIPLIB are defined
     NO_ZPARCHIVE         - Set if not using ZpArchive - used with ZIPLIB (DLL has
                             to call an established entry point so can't call
                             zipmain() directly - a new entry point that can pass
                             arguments directly to zipmain() is in the works)

   DLL_ZIPAPI is deprecated.  USE_ZIPMAIN && DLL_ZIPAPI for UNIX and VMS is replaced
   by ZIPLIB && NO_ZPARCHIVE (as they call zipmain() directly instead).
*/


#ifndef _ZIPAPI_H
# define _ZIPAPI_H


# include "zip.h"


/* =================
 * This needs to be looked at
 */
#  ifdef WIN32
#   ifndef PATH_MAX
#    define PATH_MAX 260
#   endif
# endif

# if 0
# ifdef __cplusplus
   extern "C"
   {
      void  EXPENTRY ZpVersion(ZpVer far *);
      int   EXPENTRY ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);
      int   EXPENTRY ZpArchive(ZCL C, LPZPOPT Opts);
   }
# endif
# endif /* 0 */

# ifdef WIN32
#  include <windows.h>

/* Porting definations between Win 3.1x and Win32 */
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
# endif

# if 0
#  define USE_STATIC_LIB
# endif /* 0 */

# ifdef VMS
    /* Get a realistic value for PATH_MAX. */
    /* 2012-12-31 SMS.
     * Use <nam.h> instead of <namdef.h> to avoid conflicts on some old
     * systems (like, say, VMS V5.5-2, DEC C V4.0-000), where <nam.h>
     * and <namdef.h> are incompatible, and [.vms]vms.h gets <rms.h>,
     * which gets <nam.h>.  (On modern systems, <nam.h> is a wrapper for
     * <namdef.h>.)
     */
#  include <nam.h>
#  undef PATH_MAX
   /* Some compilers may complain about the "$" in NAML$C_MAXRSS. */
#  ifdef NAML$C_MAXRSS
#   define PATH_MAX (NAML$C_MAXRSS+1)
#  else
#   define PATH_MAX (NAM$C_MAXRSS+1)
#  endif
# endif /* def VMS */

# ifndef PATH_MAX
#  ifdef MAXPATHLEN
#   define PATH_MAX      MAXPATHLEN    /* in <sys/param.h> on some systems */
#  else /* def MAXPATHLEN */
#   ifdef _MAX_PATH
#    define PATH_MAX    _MAX_PATH
#   else /* def _MAX_PATH */
#    if FILENAME_MAX > 255
#     define PATH_MAX  FILENAME_MAX    /* used like PATH_MAX on some systems */
#    else /* FILENAME_MAX > 255 */
#     define PATH_MAX  1024
#    endif /* FILENAME_MAX > 255 [else] */
#   endif /* def _MAX_PATH [else] */
#  endif /* def MAXPATHLEN [else] */
# endif /* ndef PATH_MAX */

# ifndef WIN32
   /* Adapt Windows-specific code to normal C RTL. */
#  define far
#  define _far
#  define __far
#  define _msize sizeof
#  define _strnicmp strncasecmp
#  define lstrcat strcat
#  define lstrcpy strcpy
#  define lstrlen strlen
#  define GlobalAlloc( a, b) malloc( b)
#  define GlobalFree( a) free( a)
#  define GlobalLock( a) (a)
#  define GlobalUnlock( a)
#  define BOOL int
#  define DWORD size_t
#  define EXPENTRY
#  define HANDLE void *
#  define LPSTR char *
#  define LPCSTR const char *
#  define WINAPI
# endif /* not WIN32 */


# if defined(ZIPDLL) || defined(ZIPLIB)

/* The below are used to interface with the DLL and LIB */


/*---------------------------------------------------------------------------
    Prototypes for public Zip API (DLL and LIB) functions.
  ---------------------------------------------------------------------------*/

#  define ZPVER_LEN    sizeof(ZpVer)
/* These defines are set to zero for now, until OS/2 comes out
   with a dll.
 */
#  define D2_MAJORVER 0
#  define D2_MINORVER 0
#  define D2_PATCHLEVEL 0

   /* intended to be a private struct: */
   typedef struct _zip_ver {
    uch major;              /* e.g., integer 3 */
    uch minor;              /* e.g., 1 */
    uch patchlevel;         /* e.g., 0 */
    uch not_used;
   } _zip_version_type;

   /* this is what is returned for version information */
   typedef struct _ZpVer {
    ulg structlen;          /* length of the struct being passed */
    ulg flag;               /* bit 0: is_beta   bit 1: uses_zlib */
    char betalevel[10];     /* e.g., "g BETA" or "" */
    char date[20];          /* e.g., "4 Sep 95" (beta) or "4 September 1995" */
    char zlib_version[10];  /* e.g., "0.95" or NULL */
    BOOL fEncryption;       /* TRUE if encryption enabled, FALSE otherwise */
    _zip_version_type zip;     /* the Zip version the DLL is compiled from */
    _zip_version_type os2dll;
    _zip_version_type windll;  /* now only updated when dll interface changes */
    /* new */
    ulg opt_struct_size;    /* Expected size of the option structure */
    LPSTR szFeatures;       /* Comma separated list of enabled features */
   } ZpVer;

#  ifndef EXPENTRY
#   define EXPENTRY WINAPI
#  endif

#  ifndef DEFINED_ONCE
#   define DEFINED_ONCE
    typedef int (WINAPI DLLPRNT) (LPSTR, unsigned long);
    typedef int (WINAPI DLLPASSWORD) (LPSTR, int, LPCSTR, LPCSTR);
#  endif
#  ifdef ENABLE_DLL_PROGRESS
    typedef int (WINAPI DLLPROGRESS) (LPSTR, long, long);
    /* name, % all input bytes processed * 100, % this entry processed * 100 */
#  endif
#  ifdef ZIP64_SUPPORT
    typedef int (WINAPI DLLSERVICE) (LPCSTR, unsigned __int64);
    typedef int (WINAPI DLLSERVICE_NO_INT64) (LPCSTR, unsigned long, unsigned long);
#  else
    typedef int (WINAPI DLLSERVICE) (LPCSTR, unsigned long);
#  endif
   typedef int (WINAPI DLLSPLIT) (LPSTR);
   typedef int (WINAPI DLLCOMMENT)(LPSTR);

/* Structures */

   typedef struct {        /* zip options */
    LPSTR ExcludeBeforeDate;/* Exclude if file date before this, or NULL */
    LPSTR IncludeBeforeDate;/* Include if file date before this, or NULL */
    LPSTR szRootDir;        /* Directory to use as base for zipping, or NULL */
    LPSTR szTempDir;        /* Temporary directory used during zipping, or NULL */
   /* BOOL fTemp;             Use temporary directory '-b' during zipping */
   /* BOOL fSuffix;           include suffixes (not implemented) */

    int  fUnicode;          /* Unicode flags (was "include suffixes", fMisc) */
    /*  Add values to set flags (currently 2 and 4 are exclusive)
          1 = (was include suffixes (not implemented), now not used)
          2 = no UTF8          Ignore UTF-8 information (except native)
          4 = native UTF8      Store UTF-8 as native character set
    */

    int  fEncrypt;          /* encrypt method (was "encrypt files") */
                            /* See zip.h for xxx_ENCRYPTION macros. */

    BOOL fSystem;           /* include system and hidden files */
    BOOL fVolume;           /* Include volume label */
    BOOL fExtra;            /* Exclude extra attributes */
    BOOL fNoDirEntries;     /* Do not add directory entries */
    BOOL fVerbose;          /* Mention oddities in zip file structure */
    BOOL fQuiet;            /* Quiet operation */
    BOOL fCRLF_LF;          /* Translate CR/LF to LF */
    BOOL fLF_CRLF;          /* Translate LF to CR/LF */
    BOOL fJunkDir;          /* Junk directory names */
    BOOL fGrow;             /* Allow appending to a zip file */
    BOOL fForce;            /* Make entries using DOS names (k for Katz) */
    BOOL fMove;             /* Delete files added or updated in zip file */
    BOOL fDeleteEntries;    /* Delete files from zip file */
    BOOL fUpdate;           /* Update zip file--overwrite only if newer */
    BOOL fFreshen;          /* Freshen zip file--overwrite only */
    BOOL fJunkSFX;          /* Junk SFX prefix */
    BOOL fLatestTime;       /* Set zip file time to time of latest file in it */
    BOOL fComment;          /* Put comment in zip file */
    BOOL fOffsets;          /* Update archive offsets for SFX files */
    BOOL fPrivilege;        /* Use privileges (WIN32 only) */
    BOOL fEncryption;       /* TRUE if encryption supported (compiled in), else FALSE.
                               this is a read only flag */
    LPSTR szSplitSize;      /* This string contains the size that you want to
                               split the archive into. i.e. 100 for 100 bytes,
                               2K for 2 k bytes, where K is 1024, m for meg
                               and g for gig. If this string is not NULL it
                               will automatically be assumed that you wish to
                               split an archive. */
    LPSTR szIncludeList;    /* Pointer to include file list string (for VB) */
    long IncludeListCount;  /* Count of file names in the include list array */
    char **IncludeList;     /* Pointer to include file list array. Note that the last
                               entry in the array must be NULL */
    LPSTR szExcludeList;    /* Pointer to exclude file list (for VB) */
    long ExcludeListCount;  /* Count of file names in the include list array */
    char **ExcludeList;     /* Pointer to exclude file list array. Note that the last
                               entry in the array must be NULL */
    int  fRecurse;          /* Recurse into subdirectories. 1 => -r, 2 => -R */
    int  fRepair;           /* Repair archive. 1 => -F, 2 => -FF */
    char fLevel;            /* Compression level (0 - 9) */
    LPSTR szCompMethod;     /* Compression method string (e.g. "bzip2"), or NULL */
   /* ProgressSize should be added as an option to support the zipmain() interface */
#  ifdef ENABLE_DLL_PROGRESS
     LPSTR szProgressSize;  /* Bytes read in between progress reports (-ds format)
                               Set to NULL for no reports.  If used, must define
                               DLLPROGRESS. */
#  endif
    long int fluff[8];      /* not used, for later expansion */
   } ZPOPT, _far *LPZPOPT;

   typedef struct {
     int  argc;            /* Count of files to zip */
     LPSTR lpszZipFN;      /* name of archive to create/update */
     char **FNV;           /* array of file names to zip up */
     LPSTR lpszAltFNL;     /* pointer to a string containing a list of file
                              names to zip up, separated by whitespace. Intended
                              for use only by VB users, all others should set this
                              to NULL. */
   } ZCL, _far *LPZCL;

   typedef struct {
     DLLPRNT *print;
     DLLCOMMENT *comment;
     DLLPASSWORD *password;
     DLLSPLIT *split;      /* This MUST be set to NULL unless you want to be queried
                               for a destination for each split archive. */
#  ifdef ZIP64_SUPPORT
     DLLSERVICE *ServiceApplication64;
     DLLSERVICE_NO_INT64 *ServiceApplication64_No_Int64;
#  else
     DLLSERVICE *ServiceApplication;
#  endif
#  ifdef ENABLE_DLL_PROGRESS
     DLLPROGRESS *ProgressReport;
#  endif
   } ZIPUSERFUNCTIONS, far * LPZIPUSERFUNCTIONS;

  extern LPZIPUSERFUNCTIONS lpZipUserFunctions;

#  if 0
#  ifndef __cplusplus
    void  EXPENTRY ZpVersion(ZpVer far *);
    int   EXPENTRY ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);
    int   EXPENTRY ZpArchive(ZCL C, LPZPOPT Opts);
#  endif
#  endif /* 0 */

#  if defined(ZIPLIB) || defined(COM_OBJECT)
#   define ydays zp_ydays
#  endif


/* Functions not yet supported */
#  if 0
    int      EXPENTRY ZpMain            (int argc, char **argv);
    int      EXPENTRY ZpAltMain         (int argc, char **argv, ZpInit *init);
#  endif

#  ifdef ZIPDLL
#   define printf  ZPprintf
#   define fprintf ZPfprintf
#   define perror  ZPperror

    extern int __far __cdecl printf(const char *format, ...);
    extern int __far __cdecl fprintf(FILE *file, const char *format, ...);
    extern void __far __cdecl perror(const char *);
#  endif /* ZIPDLL */


/* Interface function prototypes. */
#  ifdef __cplusplus
    extern "C"
    {
#  endif /* def __cplusplus */
  void  EXPENTRY ZpVersion(ZpVer far *);
  int   EXPENTRY ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);
  int   EXPENTRY ZpArchive(ZCL C, LPZPOPT Opts);
#  ifdef __cplusplus
    }
#  endif


#  ifndef WINDLL

/* windll.[ch] stuff for non-Windows systems. */

extern LPSTR szCommentBuf;
extern HANDLE hStr;

void comment( unsigned int);

#  endif /* ndef WINDLL */

# endif /* ZIPDLL || ZIPLIB */


#endif /* _ZIPAPI_H */
