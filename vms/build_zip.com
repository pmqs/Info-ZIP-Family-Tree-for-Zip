$! BUILD_ZIP.COM
$!
$!     Build procedure for VMS versions of Zip.
$!
$!     last revised:  2004-12-13  SMS.
$!
$!     Command arguments:
$!     - suppress help file processing: "NOHELP"
$!     - select link-only: "LINK"
$!     - select compiler environment: "VAXC", "DECC", "GNUC"
$!     - select large-file support: "LARGE"
$!     - select compiler listings: "LIST"  Note that the whole argument
$!       is added to the compiler command, so more elaborate options
$!       like "LIST/SHOW=ALL" (quoted or space-free) may be specified.
$!     - supply additional compiler options: "CCOPTS=xxx"  Allows the
$!       user to add compiler command options like /ARCHITECTURE or
$!       /[NO]OPTIMIZE.  For example, CCOPTS=/ARCH=HOST/OPTI=TUNE=HOST
$!       or CCOPTS=/DEBUG/NOOPTI.  These options must be quoted or
$!       space-free.
$!     - supply additional linker options: "LINKOPTS=xxx"  Allows the
$!       user to add linker command options like /DEBUG or /MAP.  For
$!       example: LINKOPTS=/DEBUG or LINKOPTS=/MAP/CROSS.  These options
$!       must be quoted or space-free.  Default is
$!       LINKOPTS=/NOTRACEBACK, but if the user specifies a LINKOPTS
$!       string, /NOTRACEBACK will not be included unless specified by
$!       the user.
$!     - select installation of CLI interface version of zip:
$!       "VMSCLI" or "CLI"
$!     - force installation of UNIX interface version of zip
$!       (override LOCAL_ZIP environment): "NOVMSCLI" or "NOCLI"
$!
$!     To specify additional options, define the global symbol
$!     LOCAL_ZIP as a comma-separated list of the C macros to be
$!     defined, and then run BUILD_ZIP.COM.  For example:
$!
$!             $ LOCAL_ZIP == "VMS_IM_EXTRA"
$!             $ @ [.VMS]BUILD_ZIP.COM
$!
$!     Valid VMS-specific options include VMS_PK_EXTRA and VMS_IM_EXTRA. 
$!     See the INSTALL file for other options.  (VMS_PK_EXTRA is the
$!     default.)
$!
$!     If editing this procedure to set LOCAL_ZIP, be sure to use only
$!     one "=", to avoid affecting other procedures.  For example:
$!             $ LOCAL_ZIP = "VMS_IM_EXTRA"
$!
$!     Note: This command procedure always generates both the "default"
$!     Zip having the UNIX style command interface and the "VMSCLI" Zip
$!     having the CLI compatible command interface.  There is no need to
$!     add "VMSCLI" to the LOCAL_ZIP symbol.  (The only effect of
$!     "VMSCLI" now is the selection of the CLI style Zip executable in
$!     the foreign command definition.)
$!
$!
$ on error then goto error
$ on control_y then goto error
$ OLD_VERIFY = f$verify( 0)
$!
$ edit := edit                  ! override customized edit commands
$ say := write sys$output
$!
$!##################### Read settings from environment ########################
$!
$ if (f$type( LOCAL_ZIP) .eqs. "")
$ then
$     local_zip = ""
$ else  ! Trim blanks and append comma if missing
$     local_zip = f$edit( local_zip, "TRIM")
$     if (f$extract( f$length( local_zip)- 1, 1, local_zip) .nes. ",")
$     then
$         local_zip = local_zip + ","
$     endif
$ endif
$!
$! Check for the presence of "VMSCLI" in local_zip.  If yes, we will
$! define the foreign command for "zip" to use the executable
$! containing the CLI interface.
$!
$ pos_cli = f$locate( "VMSCLI", local_zip)
$ len_local_zip = f$length( local_zip)
$ if (pos_cli .ne. len_local_zip)
$ then
$     CLI_IS_DEFAULT = 1
$     ! Remove "VMSCLI" macro from local_zip. The Zip executable
$     ! including the CLI interface is now created unconditionally.
$     local_zip = f$extract( 0, pos_cli, local_zip)+ -
       f$extract( pos_cli+7, len_local_zip- (pos_cli+ 7), local_zip)
$ else
$     CLI_IS_DEFAULT = 0
$ endif
$ delete /symbol /local pos_cli
$ delete /symbol /local len_local_zip
$!
$!##################### Customizing section #############################
$!
$ zipx_unx = "zip"
$ zipx_cli = "zip_cli"
$!
$ CCOPTS = ""
$ LINKOPTS = "/notraceback"
$ LINK_ONLY = 0
$ LISTING = " /nolist"
$ LARGE_FILE = 0
$ MAKE_HELP = 1
$ MAY_USE_DECC = 1
$ MAY_USE_GNUC = 0
$!
$! Process command line parameters requesting optional features.
$!
$ arg_cnt = 1
$ argloop:
$     current_arg_name = "P''arg_cnt'"
$     curr_arg = f$edit( 'current_arg_name', "UPCASE")
$     if (curr_arg .eqs. "") then goto argloop_out
$!
$     if (f$extract( 0, 5, curr_arg) .eqs. "CCOPT")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         CCOPTS = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 5, curr_arg) .eqs. "LARGE")
$     then
$         LARGE_FILE = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 7, curr_arg) .eqs. "LINKOPT")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         LINKOPTS = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$! Note: LINK test must follow LINKOPTS test.
$!
$     if (f$extract( 0, 4, curr_arg) .eqs. "LINK")
$     then
$         LINK_ONLY = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 4, curr_arg) .eqs. "LIST")
$     then
$         LISTING = "/''curr_arg'"      ! But see below for mods.
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "NOHELP")
$     then
$         MAKE_HELP = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "VAXC")
$     then
$         MAY_USE_DECC = 0
$         MAY_USE_GNUC = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "DECC")
$     then
$         MAY_USE_DECC = 1
$         MAY_USE_GNUC = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "GNUC")
$     then
$         MAY_USE_DECC = 0
$         MAY_USE_GNUC = 1
$         goto argloop_end
$     endif
$!
$     if ((curr_arg .eqs. "VMSCLI") .or. (curr_arg .eqs. "CLI"))
$     then
$         CLI_IS_DEFAULT = 1
$         goto argloop_end
$     endif
$!
$     if ((curr_arg .eqs. "NOVMSCLI") .or. (curr_arg .eqs. "NOCLI"))
$     then
$         CLI_IS_DEFAULT = 0
$         goto argloop_end
$     endif
$!
$     say "Unrecognized command-line option: ''curr_arg'"
$     goto error
$!
$     argloop_end:
$     arg_cnt = arg_cnt + 1
$ goto argloop
$ argloop_out:
$!
$ if (CLI_IS_DEFAULT)
$ then
$     ZIPEXEC = zipx_cli
$ else
$     ZIPEXEC = zipx_unx
$ endif
$!
$!#######################################################################
$!
$! Find out current disk, directory, compiler and options
$!
$ workdir = f$environment( "default")
$ here = f$parse( workdir, , , "device")+ f$parse( workdir, , , "directory")
$!
$! Sense the host architecture (Alpha, Itanium, or VAX).
$!
$ if (f$getsyi( "HW_MODEL") .lt. 1024)
$ then
$     arch = "VAX"
$ else
$     if (f$getsyi( "ARCH_TYPE") .eq. 2)
$     then
$         arch = "ALPHA"
$     else
$         if (f$getsyi( "ARCH_TYPE") .eq. 3)
$         then
$             arch = "IA64"
$         else
$             arch = "unknown_arch"
$         endif
$     endif
$ endif
$!
$ dest = arch
$ cmpl = "DEC/Compaq/HP C"
$ opts = ""
$ if (arch .nes. "VAX")
$ then
$     HAVE_DECC_VAX = 0
$     USE_DECC_VAX = 0
$!
$     if (MAY_USE_GNUC)
$     then
$         say "GNU C is not supported for ''arch'."
$         say "You must use DEC/Compaq/HP C to build Zip."
$         goto error
$     endif
$!
$     if (.not. MAY_USE_DECC)
$     then
$         say "VAX C is not supported for ''arch'."
$         say "You must use DEC/Compaq/HP C to build Zip."
$         goto error
$     endif
$!
$     cc = "cc /standard = relax /prefix = all /ansi"
$     defs = "''local_zip' VMS"
$     if (LARGE_FILE .ne. 0)
$     then
$         defs = "LARGE_FILE_SUPPORT, ''defs'"
$     endif
$ else
$     if (LARGE_FILE .ne. 0)
$     then
$        say "LARGE_FILE_SUPPORT is not available on VAX."
$        LARGE_FILE = 0
$     endif
$     HAVE_DECC_VAX = (f$search( "SYS$SYSTEM:DECC$COMPILER.EXE") .nes. "")
$     HAVE_VAXC_VAX = (f$search( "SYS$SYSTEM:VAXC.EXE") .nes. "")
$     MAY_HAVE_GNUC = (f$trnlnm( "GNU_CC") .nes. "")
$     if (HAVE_DECC_VAX .and. MAY_USE_DECC)
$     then
$         ! We use DECC:
$         USE_DECC_VAX = 1
$         cc = "cc /decc /prefix = all"
$         defs = "''local_zip' VMS"
$     else
$         ! We use VAXC (or GNU C):
$         USE_DECC_VAX = 0
$         defs = "''local_zip' VMS"
$         if ((.not. HAVE_VAXC_VAX .and. MAY_HAVE_GNUC) .or. MAY_USE_GNUC)
$         then
$             cc = "gcc"
$             opts = "GNU_CC:[000000]GCCLIB.OLB /LIBRARY,"
$             dest = "''dest'G"
$             cmpl = "GNU C"
$         else
$             if (HAVE_DECC_VAX)
$             then
$                 cc = "cc /vaxc"
$             else
$                 cc = "cc"
$             endif
$             dest = "''dest'V"
$             cmpl = "VAC C"
$         endif
$         opts = "''opts' SYS$DISK:[.''dest']VAXCSHR.OPT /OPTIONS,"
$     endif
$ endif
$!
$! Reveal the plan.  If compiling, set some compiler options.
$!
$ if (LINK_ONLY)
$ then
$     say "Linking on ''arch' for ''cmpl'."
$ else
$     say "Compiling on ''arch' using ''cmpl'."
$!
$     DEF_UNX = "/define = (''defs')"
$     DEF_CLI = "/define = (''defs', VMSCLI)"
$     DEF_UTIL = "/define = (''defs', UTIL)"
$ endif
$!
$! Change the destination directory, if the large-file option is enabled.
$!
$ if (LARGE_FILE .ne. 0)
$ then
$     dest = "''dest'L"
$ endif
$!
$! If [.'dest'] does not exist, either complain (link-only) or make it.
$!
$ if (f$search( "''dest'.dir;1") .eqs. "")
$ then
$     if (LINK_ONLY)
$     then
$         say "Can't find directory ""[.''dest']"".  Can't link."
$         goto error
$     else
$         create /directory [.'dest']
$     endif
$ endif
$!
$ if (.not. LINK_ONLY)
$ then
$!
$! Arrange to get arch-specific list file placement, if listing, and if
$! the user didn't specify a particular "/LIST =" destination.
$!
$     L = f$edit( LISTING, "COLLAPSE")
$     if ((f$extract( 0, 5, L) .eqs. "/LIST") .and. -
       (f$extract( 4, 1, L) .nes. "="))
$     then
$         LISTING = " /LIST = [.''dest']"+ f$extract( 5, 1000, LISTING)
$     endif
$!
$! Define compiler command.
$!
$     cc = cc+ LISTING+ CCOPTS
$!
$ endif
$!
$! Define linker command.
$!
$ link = "link ''LINKOPTS'"
$!
$! Make a VAXCRTL options file for GNU C or VAC C, if needed.
$!
$ if ((opts .nes. "") .and. -
   (f$locate( "VAXCSHR", f$edit( opts, "UPCASE")) .lt. f$length( opts)) .and. -
   (f$search( "[.''dest']vaxcshr.opt") .eqs. ""))
$ then
$     open /write opt_file_ln [.'dest']vaxcshr.opt
$     write opt_file_ln "SYS$SHARE:VAXCRTL.EXE /SHARE"
$     close opt_file_ln
$ endif
$!
$! Show interesting facts.
$!
$ say "   architecture = ''arch' (destination = [.''dest'])"
$ if (.not. LINK_ONLY)
$ then
$     say "   cc = ''cc'"
$ endif
$ say "   link = ''link'"
$ if (.not. MAKE_HELP)
$ then
$     say "   Not making new help files."
$ endif
$ say ""
$!
$ tmp = f$verify( 1)    ! Turn echo on to see what's happening.
$!
$!-------------------------------- Zip section -------------------------------
$!
$ if (.not. LINK_ONLY)
$ then
$!
$! Process the help file, if desired.
$!
$     if (MAKE_HELP)
$     then
$         runoff /out = zip.hlp [.vms]vms_zip.rnh
$     endif
$!
$! Compile the sources.
$!
$     cc 'DEF_UNX' /object = [.'dest']zip.obj zip.c
$     cc 'DEF_UNX' /object = [.'dest']crc32.obj crc32.c
$     cc 'DEF_UNX' /object = [.'dest']crctab.obj crctab.c
$     cc 'DEF_UNX' /object = [.'dest']crypt.obj crypt.c
$     cc 'DEF_UNX' /object = [.'dest']deflate.obj deflate.c
$     cc 'DEF_UNX' /object = [.'dest']fileio.obj fileio.c
$     cc 'DEF_UNX' /object = [.'dest']globals.obj globals.c
$     cc 'DEF_UNX' /object = [.'dest']trees.obj trees.c
$     cc 'DEF_UNX' /object = [.'dest']ttyio.obj ttyio.c
$     cc 'DEF_UNX' /object = [.'dest']util.obj util.c
$     cc 'DEF_UNX' /object = [.'dest']zipfile.obj zipfile.c
$     cc 'DEF_UNX' /object = [.'dest']zipup.obj zipup.c
$     cc /include = []'DEF_UNX' /object = [.'dest']vms.obj -
       [.vms]vms.c
$     cc /include = []'DEF_UNX' /object = [.'dest']vmsmunch.obj -
       [.vms]vmsmunch.c
$     cc /include = []'DEF_UNX' /object = [.'dest']vmszip.obj -
       [.vms]vmszip.c
$!
$! Create the object library.
$!
$     if (f$search( "[.''dest']zip.olb") .eqs. "") then -
       libr /object /create [.'dest']zip.olb
$!
$     libr /object /replace [.'dest']zip.olb -
       [.'dest']crc32.obj, -
       [.'dest']crctab.obj, -
       [.'dest']crypt.obj, -
       [.'dest']deflate.obj, -
       [.'dest']fileio.obj, -
       [.'dest']globals.obj, -
       [.'dest']trees.obj, -
       [.'dest']ttyio.obj, -
       [.'dest']util.obj, -
       [.'dest']zipfile.obj, -
       [.'dest']zipup.obj, -
       [.'dest']vms.obj, -
       [.'dest']vmsmunch.obj, -
       [.'dest']vmszip.obj
$!
$ endif
$!
$! Link the executable.
$!
$ link /executable = [.'dest']'zipx_unx'.exe -
   [.'dest']zip.obj, -
   [.'dest']zip.olb /include = (GLOBALS) /library, -
   'opts' -
   [.VMS]zip.opt /options
$!
$!----------------------- Zip (CLI interface) section ----------------------
$!
$ if (.not. LINK_ONLY)
$ then
$!
$! Process the CLI help file, if desired.
$!
$     if (MAKE_HELP)
$     then
$         set default [.vms]
$         edit /tpu /nosection /nodisplay /command = cvthelp.tpu -
           zip_cli.help
$         set default [-]
$         runoff /output = zip_cli.hlp [.vms]zip_cli.rnh
$     endif
$!
$! Compile the CLI sources.
$!
$     cc 'DEF_CLI' /object = [.'dest']zipcli.obj zip.c
$     cc /include = [] 'DEF_CLI' /object = [.'dest']cmdline.obj -
       [.vms]cmdline.c
$!
$! Create the command definition object file.
$!
$     set command /object = [.'dest']zip_cli.obj [.vms]zip_cli.cld
$!
$! Create the CLI object library.
$!
$     if (f$search( "[.''dest']zipcli.olb") .eqs. "") then -
       libr /object /create [.'dest']zipcli.olb
$!
$     libr /object /replace [.'dest']zipcli.olb -
       [.'dest']zipcli.obj, -
       [.'dest']cmdline.obj, -
       [.'dest']zip_cli.obj
$!
$ endif
$!
$! Link the CLI executable.
$!
$ link /executable = [.'dest']'zipx_cli'.exe -
   [.'dest']zipcli.obj, -
   [.'dest']zipcli.olb /library, -
   [.'dest']zip.olb /include = (GLOBALS) /library, -
   'opts' -
   [.VMS]zip.opt /options
$!
$!------------------------ Zip utilities section -----------------------------
$!
$ if (.not. LINK_ONLY)
$ then
$!
$! Compile the variant Zip utilities library sources.
$!
$     cc 'DEF_UTIL' /object = [.'dest']crypt_.obj crypt.c
$     cc 'DEF_UTIL' /object = [.'dest']fileio_.obj fileio.c
$     cc 'DEF_UTIL' /object = [.'dest']util_.obj util.c
$     cc 'DEF_UTIL' /object = [.'dest']zipfile_.obj zipfile.c
$     cc 'DEF_UTIL' /include = [] /object = [.'dest']vms_.obj [.vms]vms.c
$!
$! Create the Zip utilities object library.
$!
$     if f$search( "[.''dest']ziputils.olb") .eqs. "" then -
       libr /object /create [.'dest']ziputils.olb
$!
$     libr /object /replace [.'dest']ziputils.olb -
       [.'dest']crctab.obj, -
       [.'dest']crypt_.obj, -
       [.'dest']fileio_.obj, -
       [.'dest']globals.obj, -
       [.'dest']ttyio.obj, -
       [.'dest']util_.obj, -
       [.'dest']zipfile_.obj, -
       [.'dest']vms_.obj, -
       [.'dest']vmsmunch.obj
$!
$! Compile the Zip utilities main program sources.
$!
$     cc 'DEF_UTIL' /object = [.'dest']zipcloak.obj zipcloak.c
$     cc 'DEF_UTIL' /object = [.'dest']zipnote.obj zipnote.c
$     cc 'DEF_UTIL' /object = [.'dest']zipsplit.obj zipsplit.c
$!
$ endif
$!
$! Link the Zip utilities executables.
$!
$ link /executable = [.'dest']zipcloak.exe -
   [.'dest']zipcloak.obj, -
   [.'dest']ziputils.olb /include = (GLOBALS) /library, -
   'opts' -
   [.VMS]zip.opt /options
$!
$ link /executable = [.'dest']zipnote.exe -
   [.'dest']zipnote.obj, -
   [.'dest']ziputils.olb /include = (GLOBALS) /library, -
   'opts' -
   [.VMS]zip.opt /options
$!
$ link /executable = [.'dest']zipsplit.exe -
   [.'dest']zipsplit.obj, -
   [.'dest']ziputils.olb /include = (GLOBALS) /library, -
   'opts' -
   [.VMS]zip.opt /options
$!
$!----------------------------- Symbols section ------------------------------
$!
$ there = here- "]"+ ".''dest']"
$!
$! Define the foreign command symbols.  Similar commands may be useful
$! in SYS$MANAGER:SYLOGIN.COM and/or users' LOGIN.COM.
$!
$ zip      == "$''there'''ZIPEXEC'.exe"
$ zipcloak == "$''there'zipcloak.exe"
$ zipnote  == "$''there'zipnote.exe"
$ zipsplit == "$''there'zipsplit.exe"
$!
$! Restore the original default directory and DCL verify status.
$!
$ error:
$!
$ if (f$type( here) .nes. "")
$ then
$     if (here .nes. "")
$     then
$         set default 'here'
$     endif
$ endif
$!
$ if (f$type( OLD_VERIFY) .nes. "")
$ then
$     tmp = f$verify( OLD_VERIFY)
$ endif
$!
$ exit
$!
