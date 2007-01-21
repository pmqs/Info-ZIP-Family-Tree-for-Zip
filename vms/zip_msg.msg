!    VMS Error Message File for Zip
!
! Ideally, this file would be generated automatically from, say, data in
! ziperr.h (or vice versa), but currently the IDENT symbols aren't there
! as strings, and the severity information is hidden in [.VMS]vms.c.
!
! Because the facility code was formally assigned by HP, the .FACILITY
! directive below specifies /SYSTEM.  Because the messages are, in
! general, specific to Zip, this file is not compiled with /SHARED.
! For example:
!
!    MESSAGE /OBJECT = [.dest]ZIP_MSG.OBJ /NOSYMBOLS [.VMS]ZIP_MSG.MSG
!
!    LINK /SHAREABLE = [.dest]ZIP_MSG.EXE [.dest]ZIP_MSG.OBJ
!
!-----------------------------------------------------------------------
!
!    Revision History:
!
! 2007-01-20 SMS.  New.
!
!-----------------------------------------------------------------------
!

.TITLE  Info-ZIP ZIP Error Messages
.IDENT 'V3.0-001'
.FACILITY IZ_ZIP, 1955 /SYSTEM

.BASE 0
OK      /SUCCESS        <Normal successful completion>
.BASE 2
EOF     /FATAL          <Unexpected end of zip file>
FORM    /ERROR          <Zip file structure invalid>
MEM     /FATAL          <Out of memory>
LOGIC   /FATAL          <Internal logic error>
BIG     /ERROR          <Entry too big to split, read, or write>
NOTE    /ERROR          <Invalid comment format>
TEST    /FATAL          <Zip file invalid, could not spawn unzip, or wrong unzip>
ABORT   /ERROR          <Interrupted>
TEMP    /FATAL          <Temporary file failure>
READ    /FATAL          <Input file read failure>
NONE    /WARNING        <Nothing to do>
NAME    /ERROR          <Missing or empty zip file>
WRITE   /FATAL          <Output file write failure>
CREAT   /FATAL          <Could not create output file>
PARMS   /ERROR          <Invalid command arguments>
.BASE 18
OPEN    /ERROR          <File not found or no read permission>
COMPERR /FATAL          <Not supported>
ZIP64   /FATAL          <Attempt to read unsupported Zip64 archive>

.END
