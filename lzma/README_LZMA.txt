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
Info-ZIP programs, in an "lzma" subdirectory.  A small change was made
to the header file "lzma/LzFind.h" to accommodate name length
limitations on VMS systems.  Files whose names begin with a numeric
character were changed to begin with an alphabetic character, to
accommodate some IBM operating systems:

      7zFile.c     ->  SzFile.c
      7zFile.h     ->  SzFile.h
      7zVersion.h  ->  SzVersion.h

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      Building UnZip and Zip with LZMA Compression Support
      ----------------------------------------------------

   The build instructions (in the file INSTALL, in the UnZip and Zip
source kits) describe how to build UnZip and Zip with LZMA compression.

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

      2011-08-14  New.  LZMA SDK version 9.20.  (SMS)

