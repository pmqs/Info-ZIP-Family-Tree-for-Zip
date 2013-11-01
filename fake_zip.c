#include "zip.h"

#include <stdio.h>

int ZipComment( char *cmnt)
{
    strcpy( cmnt, "This is my comment.");
    return TRUE;
}

int ZipPassword( char *pwd, int size, const char *prompt, const char *zfn)
{
    fprintf( stderr, "ZPW.  size = %d, zfn: >%s<\n", size, zfn);
    fprintf( stderr, "ZPW.   pr: >%s<\n", prompt);
    strncpy( pwd, "password", size);
    return IZ_PW_ENTERED;
}

int ZipPrint( char *buf, unsigned long size)
{
    fprintf( stderr, "ZP(%d): %s", size, buf);
    return size;
}


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

    ZipUserFunctions.comment = ZipComment;
    ZipUserFunctions.password = ZipPassword;
    ZipUserFunctions.print = ZipPrint;

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
