                        README_LZMA.txt
                        ---------------

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Description
      -----------

   The Info-ZIP programs UnZip (version 6.1 and later) and Zip (version
3.1 and later) offer optional support for LZMA compression (method 14). 
The PKWARE, Inc. APPNOTE describes this method as an Early Feature
Specification, hence subject to change.

      http://www.pkware.com/documents/casestudies/APPNOTE.TXT

   Our LZMA implementation uses public-domain code from the LZMA
Software Development Kit (SDK) provided by Igor Pavlov:

      http://www.7-zip.org/sdk.html

Only a small subset of the LZMA SDK is used by (and provided with) the
Info-ZIP programs, in an "lzma" subdirectory.

   A small change was made to the header file "lzma/LzFind.h" to
accommodate name length limitations on VMS systems.

   Additional changes were made to various LZMA files to allow LZMA to
be included in Zip.  See CHANGES and the comments in the files in the
lzma directory for more on what was changed.  The directory lzma/orig
has copies of the original unmodified files for comparison.

   Multi-threading support is not included in this release, though the
LZMA SDK has an initial version of it for Windows.  We may look at
adding multi-threading support in a later release.

   LZMA2, with better threading and other enhancements, is not currently
supported.  It may be looked at for a future release.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Building UnZip and Zip with LZMA Compression Support
      ----------------------------------------------------

   The build instructions (file INSTALL in the UnZip and Zip source kits)
describe how to build UnZip and Zip with LZMA compression.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Acknowledgement
      ---------------

   We're grateful to Igor Pavlov for providing the LZMA compression code.
Any problems involving LZMA compression in Info-ZIP programs should be
reported to the Info-ZIP team, not to Mr. Pavlov.  However, any questions
on LZMA compression algorithms, or regarding LZMA SDK code (except as we
modified and use it) should be addressed to Mr. Pavlov.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Document Revision History
      -------------------------

      2011-08-20  Add description of new Zip LZMA support and make other
                  updates.  (EG)
      2011-08-08  New.  LZMA SDK version 9.20.  (SMS)

