/*
  Copyright (c) 1990-2012 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-02 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------

  izzip_example.c

  Example main program illustrating how to use the libizzip object
  library (Unix: libizzip.a, VMS: LIBIZZIP.OLB).

  Basic build procedure, Unix:

    cc izzip_example.c -DUSE_ZIPMAIN -IZip_Source_Dir -o izzip_example \
     -LZip_Object_Dir -lizzip

  (On Unix, the default Zip_Object_Dir is the same as the
  Zip_Source_Dir.)

  Basic build procedure, VMS:

    cc izzip_example.c /define = USE_ZIPMAIN=1 -
     /include = (Zip_Source_Dir, Zip_Source_Vms_Dir)
    link izzip_example.obj, Zip_Object_Dir:libizzip.olb /library

  If the Zip library was built with bzip2 support, then the bzip2
  object library must be added to the link command.

  On Unix, add the appropriate -L (if needed) and -l options (plus any
  other system-specific options which may be needed).  For example:

    cc izzip_example.c -DUSE_ZIPMAIN -IZip_Source_Dir -o izzip_example \
     -LZip_Object_Dir -LBzip2_Object_Dir -lizzip -lbz2 -lizzip

  Additional -L and/or -l options may be needed to supply other external
  libraries, such as zlib, if these are used.

  On VMS, a link options file, LIB_IZZIP.OPT, is generated along with
  the object library (in the same directory), and it can be used to
  simplify the LINK command.  (LIB_IZZIP.OPT contains comments showing
  how to define the logical names which it uses.)

    define LIB_IZZIP Dir        ! See comments in LIB_IZZIP.OPT.
    define LIB_other Dir        ! See comments in LIB_IZZIP.OPT.
    link izzip_example.obj, Zip_Object_Dir:lib_izzip.opt /options

  ---------------------------------------------------------------------------*/

#include "zip.h"                /* Zip specifics. */

#include <stdio.h>

/*
 * MyZipComment(): Archive comment call-back function.
 * Maximum length: 65534.
 */

int MyZipComment( char *cmnt)
{
    strcpy( cmnt, "This is my comment.");
    return TRUE;
}

/*
 * MyZipPassword(): Encryption password call-back function.
 */

int MyZipPassword( char *pwd, int size, const char *prompt, const char *zfn)
{
    fprintf( stderr, "ZPW.  size = %d, zfn: >%s<\n", size, zfn);
    fprintf( stderr, "ZPW.   pr: >%s<\n", prompt);
    strncpy( pwd, "password", size);
    return IZ_PW_ENTERED;
}

/*
 * MyZipPrint(): Message output call-back function.
 */

int MyZipPrint( char *buf, unsigned long size)
{
    fprintf( stderr, "ZP(%d): %s", size, buf);
    return size;
}


/*
 * main(): Example main program.
 */

int main( int argc, char **argv)
{
    int sts;
#ifdef __VMS
    int vsts;
#endif
    ZpVer zip_ver;

    ZIPUSERFUNCTIONS ZipUserFunctions;
    char cmnt_buf[ 1024];

    szCommentBuf = cmnt_buf;

    ZipUserFunctions.comment = MyZipComment;
    ZipUserFunctions.password = MyZipPassword;
    ZipUserFunctions.print = MyZipPrint;

    sts = ZpInit( &ZipUserFunctions);

    fprintf( stderr, " ZipInit() sts = %d (%%x%08x).\n", sts, sts);

    sts = zipmain( argc, argv);

    fprintf( stderr, " Raw sts = %d (%%x%08x).\n", sts, sts);
#ifdef __VMS
    vsts = vms_status( sts);
    fprintf( stderr, " VMS sts = %d (%%x%08x).\n", vsts, vsts);
#endif

    ZpVersion( &zip_ver);
    fprintf( stderr, " Zip version %d.%d%d%s\n",
     zip_ver.zip.major, zip_ver.zip.minor, zip_ver.zip.patchlevel,
     zip_ver.betalevel);
    fprintf( stderr, "%s\n", zip_ver.szFeatures);
}
