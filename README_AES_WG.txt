                        README_AES_WG.txt
                        -----------------

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Description
      -----------

   This note describes the IZ_AES_WG source kit which may be used to add
support for the WinZip form of AES (strong) encryption (a build-time
option) to the Info-ZIP programs UnZip (version 6.1 and later) and Zip
(version 3.1 and later).

   The WinZip AES scheme is described in:

      http://www.winzip.com/aes_info.htm

Briefly, the WinZip form of AES encryption in a Zip archive uses
compression method 99 to indicate AES-encrypted entries, and a new extra
field (type 0x9901) to hold additional data. 

   The IZ_AES_WG source kit is based on an AES source kit provided by
Brian Gladman:

      http://gladman.plushost.co.uk/oldsite/cryptography_technology/
      fileencrypt/files.zip

with one header file ("brg_endian.h") from a newer kit:

      http://gladman.plushost.co.uk/oldsite/AES/aes-src-11-01-11.zip

(Non-Windows users should use "unzip -a" when unpacking those kits.)

   The original Gladman code has been modified in minor ways to improve
its portability.  The changes were:

      Use <string.h> instead of <memory.h>.

      Use "brg_endian.h" for endian determination.

      Change "brg_endian.h" to work with GNU C on non-Linux systems.

Comments in the code identify the changes.  (Look for "Info-ZIP".)  The
original files are preserved in an "original_files" subdirectory, for
reference.

   The name "IZ_AES_WG" (Info-ZIP AES WinZip/Gladman) is used by
Info-ZIP to identify our implementation of the WinZip form of AES
encryption, using encryption code supplied by Dr. Gladman.  WinZip is a
registered trademark of WinZip International LLC.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Building UnZip and Zip with AES_WG Encryption Support
      -----------------------------------------------------

   First, obtain the IZ_AES_WG source kit, which is packaged separately
from the the basic UnZip and Zip source kits (but should be found
nearby).  For example, on the Info-ZIP FTP server, these kits might be:

      ftp://ftp.info-zip.org/pub/infozip/src/iz_aes_wg10.zip
      ftp://ftp.info-zip.org/pub/infozip/src/unzip61.zip
      ftp://ftp.info-zip.org/pub/infozip/src/zip31.zip

(Version suffixes on the kits may vary, and ".tgz" tar+gzip kits may
also be available.)

   After unpacking an UnZip or Zip source kit, move into the UnZip or
Zip source directory and unpack the IZ_AES_WG source kit there.  For
example, to build UnZip on a UNIX(-like) or Windows system, the commands
might look like these:

      unzip unzip61.zip   # Or (on UNIX): gzip -dc unzip61.tgz | tar xf -
      cd unzip61
      unzip ../iz_aes_wg10.zip

Or, on VMS:

      unzip unzip61.zip
      set default [.unzip61]
      unzip [-]iz_aes_wg10.zip

This should populate an "aes_wg" subdirectory, where the UnZip and Zip
builders expect to find the IZ_AES_WG source code.  (The IZ_AES_WG
source kit should contain a copy of this README document, possibly a
newer version.  Check the Version History at the bottom.)

   If no UnZip program is available to unpack the IZ_AES_WG kit, then
build (or otherwise obtain) an UnZip executable without AES_WG support,
and use that UnZip executable to unpack the IZ_AES_WG kit.  (After
building a non-AES_WG UnZip, save that UnZip executable somewhere, and
use some method like "make clean" to ensure a fresh start when building
UnZip again with AES_WG support.)

   Build procedures vary among platforms, but on UNIX(-like) systems,
one normally specifies the "make" macro "AES_WG".  For example:

      make -f unix/Makefile AES_WG=1 generic

On VMS, it's an MMS/MMK macro or a DCL command-line parameter:

      mms /descrip = [.vms] /macro = (LARGE=1, AES_WG=1)
or:
      @ [.vms]build_unzip.com LARGE AES_WG

   The procedure is similar when building Zip with AES_WG support.
Unpack the Zip source kit, move into the Zip source directory, unpack
the IZ_AES_WG kit there, and build Zip, requesting AES_WG support.

   Wnen built with AES_WG support, the UnZip and Zip "-v" (or VMS CLI
/VERBOSE) reports should include a line like:

        CRYPT_AES_WG         (AES encryption (WinZip/Gladman), ver 1.0)

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Acknowledgements
      ----------------

   We're grateful to Dr. Gladman for providing the AES encryption code.
Any problems involving AES_WG encryption in Info-ZIP programs should be
reported to the Info-ZIP team, not to Dr. Gladman.

   We're grateful to WinZip for making the WinZip AES specification
available, and providing the detailed information needed to implement
it.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Version History
      ---------------

      0.1  2011-06-17  Initial alpha version.

