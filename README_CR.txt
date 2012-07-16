README_CR.txt - Readme file for Info-ZIP Traditional Encryption (ZCRYPT)

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Description
      -----------

   A modified version of Traditional encryption (the original encryption
in PKZIP) has been available for Zip and UnZip for some time as the
separate package ZCRYPT.  As of 2002, changes to US export laws (as
detailed below) has allowed us to bundle the traditional encryption
code as part of the source code distributions from Zip 2.31 on and from
UnZip 5.50 on.  This means that the separate ZCRYPT encryption package
is not needed for these later Zip 2.x and UnZip 5.x releases.  The
ZCRYPT code is also included in Zip 3.x and UnZip 6.x.

  ZCRYPT is still needed for adding Traditional encryption to earlier
releases.  For these releases the ZCRYPT 2.9 package is required and is
available on our ftp site.  More details are provided in the file
README.CR included in the ZCRYPT 2.9 package.  We're no longer including
that file in the main source distributions.

   This file describes the Info-ZIP ZCRYPT 3.0 source kit which contains
the current implementation of the ZCRYPT code in Zip and UnZip.  The
source code is already integrated in the current versions of Zip and
UnZip.  This kit merely includes the current Traditional encryption
implementation in Zip and UnZip and these extracted files are mainly
provided for review of the implementation of the encryption and
decryption algorithms.  DO NOT USE THIS KIT TO IMPLEMENT ENCRYPTION OR
DECRYPTION IN ANY VERSION OF Zip OR UnZip.

   We do not expect to update this kit as long as the Traditional
encryption and decryption algorithms do not change.  However, be sure
to check for the latest version of this kit, which should be available
at:

      ftp://ftp.info-zip.org/pub/infozip/crypt/zcrypt.zip

Older versions (such as ZCRYPT 2.9) should be available there as well.
The latest version of this document should be available at:

     ftp://ftp.info-zip.org/pub/infozip/crypt/README_CR.txt

   As of ZCRYPT version 2.9, this encryption source code is copyrighted
by Info-ZIP; see the enclosed LICENSE.txt file for details.  Older
versions remain in the public domain.  The ZCRYPT code was originally
written in Europe and, as of April 2000, can be freely distributed from
the US as well as other countries.  The latest version of the Info-ZIP
license should be available at:

      http://www.info-zip.org/license.html

   The ability to export from the US is new (as of 2002) and is due to a
change in the regulations now administered by the Bureau of Industry and
Security (U.S. Department of Commerce), as published in Volume 65,
Number 10, of the Federal Register [14 January 2000].  Info-ZIP
initially filed the required notification via e-mail on 9 April 2000. 
The enclosed USexport.msg file shows the current notification
information.  The latest notification should be available at:

      ftp://ftp.info-zip.org/pub/infozip/crypt/USexport.msg

As of June 2002, this code can now be freely distributed in both source
and object forms from the USA under License Exception TSU of the U.S.
Export Administration Regulations (section 740.13(e)) of 6 June 2002).
Export is subject to the constraints of these regulations.  Though we
believe it can now be freely distributed from any country, check your
local laws and regulations for guidance.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Implementation
      --------------

   This traditional zip encryption code is a direct transcription of the
algorithm from Roger Schlafly, described originally by Phil Katz in the
file:

      http://www.pkware.com/documents/casestudies/APPNOTE.TXT

Note that this encryption will probably resist attacks by amateurs if
the password is well chosen and long enough (at least 8 characters) but
it will probably not resist attacks by experts.  A Web search should
find various discussions of zip encryption weaknesses.  Short passwords
consisting of lowercase letters only can be recovered in a few hours on
any workstation.  But for casual cryptography designed to keep your
mother from reading your mail, it may be adequate.

   For stronger encryption, UnZip version 6.1 (or later) and Zip version
3.1 (or later) offer optional support for AES encryption, using a
separate IZ_AES_WG (Info-ZIP AES WinZip/Gladman) source kit (different
from this ZCRYPT kit).  Traditional encryption must be enabled to enable
AES encryption.  Many public-key encryption programs are also available,
including GNU Privacy Guard (GnuPG, GPG).

   Zip 2.3x and UnZip 5.5x and later are compatible with PKZIP 2.04g. 
(Thanks to Phil Katz for accepting our suggested minor changes to the
zipfile format.)

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Compatibility
      -------------

   Though this kit includes the same Traditional encryption and
decryption algorithms and essentially the same source code implementations
as what's in Zip 3.1 and UnZip 6.1, the source code in this kit may not
be directly compatible with any specific version of Zip and UnZip.
Newer versions should already include this code and older versions, for
example, UnZip 5.4, Zip 2.3, and WiZ 5.0, should use the ZCRYPT 2.9 code.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Information of Historical Interest
      ----------------------------------

   IMPORTANT NOTE:

   Zip archives produced by Zip 2.0 or later must not be *updated* by
Zip 1.1 or PKZIP 1.10 or PKZIP 1.93a, if they contain encrypted members
or if they have been produced in a pipe or on a non-seekable device. The
old versions of Zip or PKZIP would destroy the zip structure.  The old
versions can list the contents of the zipfile but cannot extract it
anyway (because of the new compression algorithm).  If you do not use
encryption and compress regular disk files, you need not worry about
this problem.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Version History
      ---------------

      3.0  2011-09-03  Revised documentation.  The ZCRYPT code now
                       includes some infrastructure for AES encryption,
                       but all the actual AES code is packaged in a
                       separate (IZ_AES_WG) source kit.

