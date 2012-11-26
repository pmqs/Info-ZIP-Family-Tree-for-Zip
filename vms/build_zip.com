$! BUILD_ZIP.COM
$!
$!     Build procedure for VMS versions of Zip.
$!
$!     Last revised:  2012-11-19  SMS.
$!
$!     Command arguments:
$!     - suppress C compilation (re-link): "NOCOMPILE"
$!     - suppress linking executables: "NOLINK"
$!     - suppress help file processing: "NOHELP"
$!     - suppress message file processing: "NOMSG"
$!     - define DCL symbols: "SYMBOLS"  (Was default before Zip 3.1.)
$!     - select compiler environment: "VAXC", "DECC", "GNUC"
$!     - select AES (WinZip/Gladman) encryption support: "AES_WG"
$!     - select large-file support: "LARGE" (Non-VAX only)
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
$!     - select BZIP2 support: "IZ_BZIP2=dev:[dir]", where "dev:[dir]"
$!       (or a suitable logical name) tells where to find "bzlib.h".
$!       The BZIP2 object library (LIBBZ2_NS.OLB) is expected to be in
$!       a "[.dest]" directory under that one ("dev:[dir.ALPHAL]", for
$!       example), or in that directory itself.
$!     - select LZMA compression support: "LZMA" (Non-VAX only)
$!     - select PPMd compression support: "PPMD"
$!     - use ZLIB compression library: "IZ_ZLIB=dev:[dir]", where
$!       "dev:[dir]" (or a suitable logical name) tells where to find
$!       "zlib.h".  The ZLIB object library (LIBZ.OLB) is expected to be
$!       in a "[.dest]" directory under that one ("dev:[dir.ALPHAL]",
$!       for example), or in that directory itself.
$!     - build (and use) a callable-Zip library, LIBIZZIP.OLB: "LIBZIP"
$!     - choose a destination directory for architecture-specific
$!       product files (.EXE, .OBJ,.OLB, and so on): "PROD=subdir", to
$!       use "[.subdir]".  The default is a name automatically generated
$!       using rules defined below.
$!     - Create help output text files: "HELP_TEXT"
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
$!
$! Save the current default disk:[directory], and set default to the
$! directory above where this procedure is situated (which had better be
$! the main distribution directory).
$!
$ workdir = f$environment( "default")
$ here = f$parse( workdir, , , "device")+ f$parse( workdir, , , "directory")
$!
$ proc = f$environment( "procedure")
$ proc_dir = f$parse( proc, , , "device")+ f$parse( proc, , , "directory")
$ set default 'proc_dir'
$ set default [-]
$!
$ on error then goto error
$ on control_y then goto error
$ OLD_VERIFY = f$verify( 0)
$!
$ edit := edit                  ! override customized edit commands
$ say := write sys$output
$!
$! Analyze command-line options.
$!
$!##################### Read settings from environment ########################
$!
$ if (f$type( LOCAL_ZIP) .eqs. "")
$ then
$     LOCAL_ZIP = ""
$ else  ! Trim blanks and append comma if missing
$     LOCAL_ZIP = f$edit( LOCAL_ZIP, "TRIM")
$     if (f$extract( f$length( LOCAL_ZIP)- 1, 1, LOCAL_ZIP) .nes. ",")
$     then
$         LOCAL_ZIP = LOCAL_ZIP + ","
$     endif
$ endif
$!
$! Check for the presence of "VMSCLI" in LOCAL_ZIP.  If yes, we will
$! define the foreign command for "zip" to use the VMS CLI executable.
$!
$ len_local_zip = f$length( LOCAL_ZIP)
$!
$ pos_cli = f$locate( "VMSCLI", LOCAL_ZIP)
$ if (pos_cli .ne. len_local_zip)
$ then
$     CLI_IS_DEFAULT = 1
$     ! Remove "VMSCLI" macro from LOCAL_ZIP. The Zip executable
$     ! including the CLI interface is now created unconditionally.
$     LOCAL_ZIP = f$extract( 0, pos_cli, LOCAL_ZIP)+ -
       f$extract( pos_cli+7, len_local_zip- (pos_cli+ 7), LOCAL_ZIP)
$ else
$     CLI_IS_DEFAULT = 0
$ endif
$ delete /symbol /local pos_cli
$!
$! Check for the presence of "VMS_IM_EXTRA" in LOCAL_ZIP.  If yes, we
$! will (later) add "I" to the destination directory name.
$!
$ desti = ""
$ pos_im = f$locate( "VMS_IM_EXTRA", LOCAL_ZIP)
$ if (pos_im .ne. len_local_zip)
$ then
$    desti = "I"
$ endif
$!
$ delete /symbol /local len_local_zip
$!
$!##################### Customizing section #############################
$!
$ zipx_unx = "ZIP"
$ zipx_cli = "ZIP_CLI"
$ lib_zipcli_name = "ZIPCLI.OLB"
$ lib_ziputils_name = "ZIPUTILS.OLB"
$!
$ AES_WG = ""
$ CCOPTS = ""
$ IZ_BZIP2 = ""
$ IZ_ZLIB = ""
$ LINKOPTS = "/notraceback"
$ LISTING = " /nolist"
$ LARGE_FILE = 0
$ LIBZIP = 0
$ LZMA = 0
$ PPMD = 0
$ MAKE_EXE = 1
$ MAKE_HELP = 1
$ MAKE_HELP_TEXT = 0
$ MAKE_MSG = 1
$ MAKE_OBJ = 1
$ MAKE_SYM = 0
$ MAY_USE_DECC = 1
$ MAY_USE_GNUC = 0
$ PROD = ""
$!
$! Process command line parameters requesting optional features.
$!
$ arg_cnt = 1
$ argloop:
$     current_arg_name = "P''arg_cnt'"
$     curr_arg = f$edit( 'current_arg_name', "UPCASE")
$     if (curr_arg .eqs. "") then goto argloop_out
$!
$     if (f$extract( 0, 6, curr_arg) .eqs. "AES_WG")
$     then
$         AES_WG = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 5, curr_arg) .eqs. "CCOPT")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         CCOPTS = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 9, curr_arg) .eqs. "HELP_TEXT")
$     then
$         MAKE_HELP_TEXT = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 7, curr_arg) .eqs. "IZ_BZIP")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         IZ_BZIP2 = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 7, curr_arg) .eqs. "IZ_ZLIB")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         IZ_ZLIB = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 5, curr_arg) .eqs. "LARGE")
$     then
$         LARGE_FILE = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 6, curr_arg) .eqs. "LIBZIP")
$     then
$         LIBZIP = 1
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
$     if (f$extract( 0, 4, curr_arg) .eqs. "LIST")
$     then
$         LISTING = "/''curr_arg'"      ! But see below for mods.
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 4, curr_arg) .eqs. "LZMA")
$     then
$         LZMA = 1
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "NOCOMPILE")
$     then
$         MAKE_OBJ = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "NOLINK")
$     then
$         MAKE_EXE = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "NOHELP")
$     then
$         MAKE_HELP = 0
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "NOMSG")
$     then
$         MAKE_MSG = 0
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 4, curr_arg) .eqs. "PPMD")
$     then
$         PPMD = 1
$         goto argloop_end
$     endif
$!
$     if (f$extract( 0, 4, curr_arg) .eqs. "PROD")
$     then
$         opts = f$edit( curr_arg, "COLLAPSE")
$         eq = f$locate( "=", opts)
$         PROD = f$extract( (eq+ 1), 1000, opts)
$         goto argloop_end
$     endif
$!
$     if (curr_arg .eqs. "SYMBOLS")
$     then
$         MAKE_SYM = 1
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
$ destl = ""
$ destm = arch
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
$     defs = "''LOCAL_ZIP' VMS"
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
$     if (LZMA .ne. 0)
$     then
$        say "LZMA is not available on VAX."
$        LZMA = 0
$     endif
$     HAVE_DECC_VAX = (f$search( "SYS$SYSTEM:DECC$COMPILER.EXE") .nes. "")
$     HAVE_VAXC_VAX = (f$search( "SYS$SYSTEM:VAXC.EXE") .nes. "")
$     MAY_HAVE_GNUC = (f$trnlnm( "GNU_CC") .nes. "")
$     if (HAVE_DECC_VAX .and. MAY_USE_DECC)
$     then
$         ! We use DECC:
$         USE_DECC_VAX = 1
$         cc = "cc /decc /prefix = all"
$         defs = "''LOCAL_ZIP' VMS"
$     else
$         ! We use VAXC (or GNU C):
$         USE_DECC_VAX = 0
$         defs = "''LOCAL_ZIP' VMS"
$         if ((.not. HAVE_VAXC_VAX .and. MAY_HAVE_GNUC) .or. MAY_USE_GNUC)
$         then
$             cc = "gcc"
$             destm = "''destm'G"
$             cmpl = "GNU C"
$             opts = "GNU_CC:[000000]GCCLIB.OLB /LIBRARY,"
$         else
$             if (HAVE_DECC_VAX)
$             then
$                 cc = "cc /vaxc"
$             else
$                 cc = "cc"
$             endif
$             destm = "''destm'V"
$             cmpl = "VAC C"
$         endif
$         opts = "VAXC"
$     endif
$ endif
$!
$! Set the bzip2 (and/or zlib) directory.
$!
$ seek_bz = destm
$!
$! Change the destination directory, according to the VMS_IM_EXTRA and
$! large-file options, or the user's specification.
$!
$ if (LARGE_FILE .ne. 0)
$ then
$     destl = "L"
$ endif
$!
$ dest_std = destm+ desti+ destl
$ if (PROD .eqs. "")
$ then
$     dest = dest_std
$ else
$     dest = PROD
$ endif
$!
$ if (opts .eqs. "VAXC")
$ then
$     opts = "SYS$DISK:[.''dest']VAXCSHR.OPT /OPTIONS,"
$ endif
$!
$ if (LIBZIP .eq. 0)
$ then
$     lib_zip_name = "ZIP.OLB"
$ else
$     lib_zip_name = "LIBIZZIP.OLB"
$ endif
$ lib_zip = "SYS$DISK:[.''dest']''lib_zip_name'"
$ lib_zipcli = "SYS$DISK:[.''dest']''lib_zipcli_name'"
$ lib_ziputils = "SYS$DISK:[.''dest']''lib_ziputils_name'"
$ libzip_opt = "[.''dest']LIB_IZZIP.OPT"
$!
$! If BZIP2 support was selected, find the object library.
$! Complain if things fail.
$!
$ cc_incl = "[], [.VMS]"
$ lib_bzip2_opts = ""
$ if (IZ_BZIP2 .nes. "")
$ then
$     bz2_olb = "LIBBZ2_NS.OLB"
$     if (MAKE_OBJ .or. MAKE_EXE)
$     then
$         define incl_bzip2 'IZ_BZIP2'
$         defs = "''defs', BZIP2_SUPPORT"
$     endif
$!
$     if (MAKE_EXE)
$     then
$         @ [.VMS]FIND_BZIP2_LIB.COM 'IZ_BZIP2' 'seek_bz' 'bz2_olb' lib_bzip2
$         if (f$trnlnm( "lib_bzip2") .eqs. "")
$         then
$             say "Can't find BZIP2 object library.  Can't link."
$             goto error
$         else
$             lib_bzip2_opts = "LIB_BZIP2:''bz2_olb' /library, "
$         endif
$     endif
$ endif
$!
$! If ZLIB use was selected, find the object library.
$! Complain if things fail.
$!
$ lib_zlib_opts = ""
$ if (IZ_ZLIB .nes. "")
$ then
$     zlib_olb = "LIBZ.OLB"
$     define incl_zlib 'IZ_ZLIB'
$     defs = "''defs', USE_ZLIB"
$     @ [.VMS]FIND_BZIP2_LIB.COM 'IZ_ZLIB' 'seek_bz' 'zlib_olb' lib_zlib
$     if (f$trnlnm( "lib_zlib") .eqs. "")
$     then
$         say "Can't find ZLIB object library.  Can't link."
$         goto error
$     else
$         say "ZLIB dir:  ''f$trnlnm( "lib_zlib")'"
$         lib_zlib_opts = "LIB_ZLIB:''zlib_olb' /library, "
$     endif
$ endif
$!
$! Set AES_WG-related data.
$!
$ if (AES_WG .ne. 0)
$ then
$     defs = defs+ ", CRYPT_AES_WG"
$ endif
$!
$! Set LZMA-related data.
$!
$ if (LZMA .ne. 0)
$ then
$     defs = defs+ ", LZMA_SUPPORT"
$     if (arch .eqs. "VAX")
$     then
$         defs = defs+ ", _SZ_NO_INT_64"
$     endif
$ endif
$!
$! Set PPMd-related data.
$!
$ if (PPMD .ne. 0)
$ then
$     defs = defs+ ", PPMD_SUPPORT"
$     if ((arch .eqs. "VAX") .and. (LZMA .eq. 0))
$     then
$         defs = defs+ ", _SZ_NO_INT_64"
$     endif
$ endif
$!
$! Reveal the plan.  If compiling, set some compiler options.
$!
$ if (MAKE_OBJ)
$ then
$     say "Compiling on ''arch' using ''cmpl'."
$!
$     DEF_UNX = "/define = (''defs')"
$     DEF_CLI = "/define = (''defs', VMSCLI)"
$     DEF_UTIL = "/define = (''defs', UTIL)"
$     DEF_LIBZIP = "/define = (''defs', USE_ZIPMAIN)"
$ else
$     if (MAKE_EXE .or. MAKE_MSG)
$     then
$         say "Linking on ''arch' for ''cmpl'."
$     endif
$ endif
$!
$! If [.'dest'] does not exist, either complain (link-only) or make it.
$!
$ if (f$search( "''dest'.DIR;1") .eqs. "")
$ then
$     if (MAKE_OBJ)
$     then
$         create /directory [.'dest']
$     else
$         if (MAKE_EXE .or. MAKE_MSG)
$         then
$             say "Can't find directory ""[.''dest']"".  Can't link."
$             goto error
$         endif
$     endif
$ endif
$!
$ if (MAKE_OBJ)
$ then
$!
$! Arrange to get arch-specific list file placement, if LISTING, and if
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
$     cc = cc+ " /include = (''cc_incl')"+ LISTING+ CCOPTS
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
   (f$search( "[.''dest']VAXCSHR.OPT") .eqs. ""))
$ then
$     open /write opt_file_ln [.'dest']VAXCSHR.OPT
$     write opt_file_ln "SYS$SHARE:VAXCRTL.EXE /SHARE"
$     close opt_file_ln
$ endif
$!
$! Show interesting facts.
$!
$ say ""
$ say "   architecture = ''arch' (destination = [.''dest'])"
$ if (MAKE_OBJ)
$ then
$     say "   cc = ''cc'"
$ endif
$!
$ if (MAKE_EXE)
$ then
$     say "   link = ''link'"
$ endif
$!
$ if (.not. MAKE_HELP)
$ then
$     say "   Not making new help files."
$ endif
$!
$ if (.not. MAKE_MSG)
$ then
$     say "   Not making new message files."
$ endif
$!
$ if (IZ_BZIP2 .nes. "")
$ then
$     if (MAKE_EXE)
$     then
$         say "   BZIP2 include dir: ''f$trnlnm( "incl_bzip2")'"
$     endif
$     say "   BZIP2 library dir: ''f$trnlnm( "lib_bzip2")'"
$ endif
$!
$ if (IZ_ZLIB .nes. "")
$ then
$     if (MAKE_EXE)
$     then
$         say "   ZLIB include dir:  ''f$trnlnm( "incl_zlib")'"
$     endif
$     say "   ZLIB library dir:  ''f$trnlnm( "lib_zlib")'"
$ endif
$ say ""
$!
$ tmp = f$verify( 1)    ! Turn echo on to see what's happening.
$!
$!-------------------------------- Zip section -------------------------------
$!
$ if (MAKE_HELP)
$ then
$!
$! Process the Unix-style help file.
$!
$     runoff /out = ZIP.HLP [.VMS]VMS_ZIP.RNH
$!
$ endif
$!
$ if (MAKE_HELP .and. MAKE_HELP_TEXT)
$ then
$!
$! Make the Unix-style help output text file.
$!
$     help_temp_name = "help_temp_"+ f$getjpi( 0, "PID")
$     if (f$search( help_temp_name+ ".HLB") .nes. "") then -
       delete 'help_temp_name'.HLB;*
$     library /create /help 'help_temp_name'.HLB ZIP.HLP
$     help /library = sys$disk:[]'help_temp_name'.HLB -
       /output = 'help_temp_name'.OUT zip...
$     delete 'help_temp_name'.HLB;*
$     create /fdl = [.VMS]STREAM_LF.FDL ZIP.HTX
$     open /append help_temp ZIP.HTX
$     copy 'help_temp_name'.OUT help_temp
$     close help_temp
$     delete 'help_temp_name'.OUT;*
$!
$ endif
$!
$ if (MAKE_MSG)
$ then
$!
$! Process the message file.
$!
$! Create the message source file first, if it's not found.
$!
$     if (f$search( "[.VMS]ZIP_MSG.MSG") .eqs. "")
$     then
$         cc /object = [.'dest']VMS_MSG_GEN.OBJ [.VMS]VMS_MSG_GEN.C
$         link /executable = [.'dest']VMS_MSG_GEN.EXE -
           [.'dest']VMS_MSG_GEN.OBJ
$         create /fdl = [.VMS]STREAM_LF.FDL [.VMS]ZIP_MSG.MSG
$         define /user_mode sys$output [.VMS]ZIP_MSG.MSG
$         run [.'dest']VMS_MSG_GEN.EXE
$         purge [.VMS]ZIP_MSG.MSG
$         delete [.'dest']VMS_MSG_GEN.EXE;*, -
           [.'dest']VMS_MSG_GEN.OBJ;*
$     endif
$!
$     message /object = [.'dest']ZIP_MSG.OBJ /nosymbols [.VMS]ZIP_MSG.MSG
$     link /shareable = [.'dest']ZIP_MSG.EXE [.'dest']ZIP_MSG.OBJ
$!
$ endif
$!
$ if (MAKE_OBJ)
$ then
$!
$! Compile the sources.
$!
$     cc 'DEF_UNX' /object = [.'dest']ZIP.OBJ ZIP.C
$     cc 'DEF_UNX' /object = [.'dest']CRC32.OBJ CRC32.C
$     cc 'DEF_UNX' /object = [.'dest']CRYPT.OBJ CRYPT.C
$     cc 'DEF_UNX' /object = [.'dest']DEFLATE.OBJ DEFLATE.C
$     cc 'DEF_UNX' /object = [.'dest']FILEIO.OBJ FILEIO.C
$     cc 'DEF_UNX' /object = [.'dest']GLOBALS.OBJ GLOBALS.C
$     cc 'DEF_UNX' /object = [.'dest']TREES.OBJ TREES.C
$     cc 'DEF_UNX' /object = [.'dest']TTYIO.OBJ TTYIO.C
$     cc 'DEF_UNX' /object = [.'dest']UTIL.OBJ UTIL.C
$     cc 'DEF_UNX' /object = [.'dest']ZBZ2ERR.OBJ ZBZ2ERR.C
$     cc 'DEF_UNX' /object = [.'dest']ZIPFILE.OBJ ZIPFILE.C
$     cc 'DEF_UNX' /object = [.'dest']ZIPUP.OBJ ZIPUP.C
$     cc 'DEF_UNX' /object = [.'dest']VMS.OBJ [.VMS]VMS.C
$     cc 'DEF_UNX' /object = [.'dest']VMSMUNCH.OBJ [.VMS]VMSMUNCH.C
$     cc 'DEF_UNX' /object = [.'dest']VMSZIP.OBJ [.VMS]VMSZIP.C
$!
$     if (LIBZIP .ne. 0)
$     then
$         cc 'DEF_LIBZIP' /object = [.'dest']API_.OBJ API.C
$         cc 'DEF_LIBZIP' /object = [.'dest']ZIP_.OBJ ZIP.C
$     endif
$!
$     if (AES_WG .ne. 0)
$     then
$         cc 'DEF_UNX' /object = [.'dest']AESCRYPT.OBJ [.AES_WG]AESCRYPT.C
$         cc 'DEF_UNX' /object = [.'dest']AESKEY.OBJ [.AES_WG]AESKEY.C
$         cc 'DEF_UNX' /object = [.'dest']AESTAB.OBJ [.AES_WG]AESTAB.C
$         cc 'DEF_UNX' /object = [.'dest']FILEENC.OBJ [.AES_WG]FILEENC.C
$         cc 'DEF_UNX' /object = [.'dest']HMAC.OBJ [.AES_WG]HMAC.C
$         cc 'DEF_UNX' /object = [.'dest']PRNG.OBJ [.AES_WG]PRNG.C
$         cc 'DEF_UNX' /object = [.'dest']PWD2KEY.OBJ [.AES_WG]PWD2KEY.C
$         cc 'DEF_UNX' /object = [.'dest']SHA1.OBJ [.AES_WG]SHA1.C
$     endif
$!
$     if (LZMA .ne. 0)
$     then
$         cc 'DEF_UNX' /object = [.'dest']LZFIND.OBJ [.SZIP]LZFIND.C
$         cc 'DEF_UNX' /object = [.'dest']LZMAENC.OBJ [.SZIP]LZMAENC.C
$     endif
$!
$     if (PPMD .ne. 0)
$     then
$         cc 'DEF_UNX' /object = [.'dest']PPMD8.OBJ [.SZIP]PPMD8.C
$         cc 'DEF_UNX' /object = [.'dest']PPMD8ENC.OBJ [.SZIP]PPMD8ENC.C
$     endif
$!
$! Create the callable library link options file.
$!
$     if (LIBZIP .ne. 0)
$     then
$         def_dev_dir_orig = f$environment( "default")
$         set default [.'dest']
$         def_dev_dir = f$environment( "default")
$         set default 'def_dev_dir_orig'
$         create /fdl = [.VMS]STREAM_LF.FDL 'libzip_opt'
$         open /append opt_file_lib 'libzip_opt'
$         write opt_file_lib "! DEFINE LIB_IZZIP ''def_dev_dir'"
$         if (IZ_BZIP2 .nes. "")
$         then
$             write opt_file_lib "! DEFINE LIB_BZIP2 ''f$trnlnm( "lib_bzip2")'"
$         endif
$         if (IZ_ZLIB .nes. "")
$         then
$             write opt_file_lib "! DEFINE LIB_ZLIB ''f$trnlnm( "lib_zlib")'"
$         endif
$         write opt_file_lib "LIB_IZZIP:''lib_zip_name' /library"
$         if (IZ_BZIP2 .nes. "")
$         then
$             write opt_file_lib "''lib_bzip2_opts'" - ", " - ","
$         endif
$         write opt_file_lib "LIB_IZZIP:''lib_zip_name' /library"
$         if (IZ_ZLIB .nes. "")
$         then
$             write opt_file_lib "''lib_zlib_opts'" - ", " - ","
$         endif
$         close opt_file_lib
$     endif
$!
$ endif
$!
$ if (MAKE_EXE)
$ then
$!
$! Create the primary object library.
$!
$     if (f$search( lib_zip) .eqs. "") then -
       libr /object /create 'lib_zip'
$!
$     libr /object /replace 'lib_zip' -
       [.'dest']CRC32.OBJ, -
       [.'dest']CRYPT.OBJ, -
       [.'dest']DEFLATE.OBJ, -
       [.'dest']FILEIO.OBJ, -
       [.'dest']GLOBALS.OBJ, -
       [.'dest']TREES.OBJ, -
       [.'dest']TTYIO.OBJ, -
       [.'dest']UTIL.OBJ, -
       [.'dest']ZBZ2ERR.OBJ, -
       [.'dest']ZIPFILE.OBJ, -
       [.'dest']ZIPUP.OBJ, -
       [.'dest']VMS.OBJ, -
       [.'dest']VMSMUNCH.OBJ, -
       [.'dest']VMSZIP.OBJ
$!
$     if (LIBZIP .ne. 0)
$     then
$         libr /object /replace 'lib_zip' -
           [.'dest']API_.OBJ, -
           [.'dest']ZIP_.OBJ
$     endif
$!
$     if (AES_WG .ne. 0)
$     then
$         libr /object /replace 'lib_zip' -
           [.'dest']AESCRYPT.OBJ, -
           [.'dest']AESKEY.OBJ, -
           [.'dest']AESTAB.OBJ, -
           [.'dest']FILEENC.OBJ, -
           [.'dest']HMAC.OBJ, -
           [.'dest']PRNG.OBJ, -
           [.'dest']PWD2KEY.OBJ, -
           [.'dest']SHA1.OBJ
$     endif
$!
$     if (LZMA .ne. 0)
$     then
$         libr /object /replace 'lib_zip' -
           [.'dest']LZFIND.OBJ, -
           [.'dest']LZMAENC.OBJ
$     endif
$!
$     if (PPMD .ne. 0)
$     then
$         libr /object /replace 'lib_zip; -
           [.'dest']PPMD8.OBJ, -
           [.'dest']PPMD8ENC.OBJ
$     endif
$!
$ endif
$!
$ if (MAKE_EXE)
$ then
$!
$! Create the module ID options file.
$!
$     optgen_verify = f$verify( 0)
$     @ [.vms]optgen.com Zip iz_zip_versn
$     open /write opt_file_ln SYS$DISK:[.'dest']ZIP.OPT
$     write opt_file_ln "Ident = ""Zip ''f$trnlnm( "iz_zip_versn")'"""
$     close opt_file_ln
$     deassign iz_zip_versn
$     tmp = f$verify( optgen_verify)
$!
$! Link the executable.
$!
$     link /executable = [.'dest']'ZIPX_UNX'.EXE -
       SYS$DISK:[.'dest']ZIP.OBJ, -
       'lib_zip' /library, -
       'lib_bzip2_opts' -
       'lib_zip' /library, -
       'lib_zlib_opts' -
       'opts' -
       SYS$DISK:[.'dest']ZIP.OPT /options
$!
$ endif
$!
$!------------------------ Zip (CLI interface) section -----------------------
$!
$ if (MAKE_HELP)
$ then
$!
$! Process the CLI help file.
$!
$     set default [.VMS]
$     edit /tpu /nosection /nodisplay /command = cvthelp.tpu zip_cli.help
$     set default [-]
$     runoff /output = ZIP_CLI.HLP [.VMS]ZIP_CLI.RNH
$!
$ endif
$!
$ if (MAKE_HELP .and. MAKE_HELP_TEXT)
$ then
$!
$! Make the CLI help output text file.
$!
$     help_temp_name = "help_temp_"+ f$getjpi( 0, "PID")
$     if (f$search( help_temp_name+ ".HLB") .nes. "") then -
       delete 'help_temp_name'.HLB;*
$     library /create /help 'help_temp_name'.HLB ZIP_CLI.HLP
$     help /library = sys$disk:[]'help_temp_name'.HLB -
       /output = 'help_temp_name'.OUT zip...
$     delete 'help_temp_name'.HLB;*
$     create /fdl = [.VMS]STREAM_LF.FDL ZIP_CLI.HTX
$     open /append help_temp ZIP_CLI.HTX
$     copy 'help_temp_name'.OUT help_temp
$     close help_temp
$     delete 'help_temp_name'.OUT;*
$!
$ endif
$!
$ if (MAKE_OBJ)
$ then
$!
$! Compile the CLI sources.
$!
$     cc 'DEF_CLI' /object = [.'dest']ZIPCLI.OBJ ZIP.C
$     cc 'DEF_CLI' /object = [.'dest']CMDLINE.OBJ [.VMS]CMDLINE.C
$!
$! Create the command definition object file.
$!
$     cppcld_verify = f$verify( 0)
$     @ [.vms]cppcld.com "''cc'" [.VMS]ZIP_CLI.CLD -
       [.'dest']ZIP_CLI.CLD "''defs'"
$     tmp = f$verify( cppcld_verify)
$     set command /object = [.'dest']ZIP_CLI.OBJ [.'dest']ZIP_CLI.CLD
$!
$ endif
$!
$ if (MAKE_EXE)
$ then
$!
$! Create the CLI object library.
$!
$     if (f$search( lib_zipcli) .eqs. "") then -
       libr /object /create 'lib_zipcli'
$!
$     libr /object /replace 'lib_zipcli' -
       [.'dest']ZIPCLI.OBJ, -
       [.'dest']CMDLINE.OBJ, -
       [.'dest']ZIP_CLI.OBJ
$!
$ endif
$!
$! Link the CLI executable.
$!
$ if (MAKE_EXE)
$ then
$!
$     link /executable = [.'dest']'ZIPX_CLI'.EXE -
       SYS$DISK:[.'dest']ZIPCLI.OBJ, -
       'lib_zipcli' /library, -
       'lib_zip' /library, -
       'lib_bzip2_opts' -
       'lib_zip' /library, -
       'lib_zlib_opts' -
       'opts' -
       SYS$DISK:[.'dest']ZIP.OPT /options
$!
$ endif
$!
$!--------------------------- Zip utilities section --------------------------
$!
$ if (MAKE_OBJ)
$ then
$!
$! Compile the variant Zip utilities library sources.
$!
$     cc 'DEF_UTIL' /object = [.'dest']CRYPT_.OBJ CRYPT.C
$     cc 'DEF_UTIL' /object = [.'dest']FILEIO_.OBJ FILEIO.C
$     cc 'DEF_UTIL' /object = [.'dest']UTIL_.OBJ UTIL.C
$     cc 'DEF_UTIL' /object = [.'dest']ZIPFILE_.OBJ ZIPFILE.C
$     cc 'DEF_UTIL' /object = [.'dest']VMS_.OBJ [.VMS]VMS.C
$!
$! Compile the Zip utilities main program sources.
$!
$     cc 'DEF_UTIL' /object = [.'dest']ZIPCLOAK.OBJ ZIPCLOAK.C
$     cc 'DEF_UTIL' /object = [.'dest']ZIPNOTE.OBJ ZIPNOTE.C
$     cc 'DEF_UTIL' /object = [.'dest']ZIPSPLIT.OBJ ZIPSPLIT.C
$!
$ endif
$!
$ if (MAKE_EXE)
$ then
$!
$! Create the Zip utilities object library.
$!
$     if (f$search( lib_ziputils) .eqs. "") then -
       libr /object /create 'lib_ziputils'
$!
$     libr /object /replace 'lib_ziputils' -
       [.'dest']CRC32.OBJ, -
       [.'dest']CRYPT_.OBJ, -
       [.'dest']FILEIO_.OBJ, -
       [.'dest']GLOBALS.OBJ, -
       [.'dest']TTYIO.OBJ, -
       [.'dest']UTIL_.OBJ, -
       [.'dest']ZIPFILE_.OBJ, -
       [.'dest']VMS_.OBJ, -
       [.'dest']VMSMUNCH.OBJ
$!
$     if (AES_WG .ne. 0)
$     then
$         libr /object /replace 'lib_ziputils' -
           [.'dest']AESCRYPT.OBJ, -
           [.'dest']AESKEY.OBJ, -
           [.'dest']AESTAB.OBJ, -
           [.'dest']FILEENC.OBJ, -
           [.'dest']HMAC.OBJ, -
           [.'dest']PRNG.OBJ, -
           [.'dest']PWD2KEY.OBJ, -
           [.'dest']SHA1.OBJ
$     endif
$!
$     if (LZMA .ne. 0)
$     then
$         libr /object /replace 'lib_zip' -
           [.'dest']LZFIND.OBJ, -
           [.'dest']LZMAENC.OBJ
$     endif
$!
$     if (PPMD .ne. 0)
$     then
$         libr /object /replace 'lib_zip' -
           [.'dest']PPMD8.OBJ, -
           [.'dest']PPMD8ENC.OBJ
$     endif
$!
$ endif
$!
$! Link the Zip utilities executables.
$!
$ if (MAKE_EXE)
$ then
$!
$     link /executable = [.'dest']ZIPCLOAK.EXE -
       SYS$DISK:[.'dest']ZIPCLOAK.OBJ, -
       'lib_ziputils' /library, -
       'lib_zlib_opts' -
       'opts' -
       SYS$DISK:[.'dest']ZIP.OPT /options
$!
$     link /executable = [.'dest']ZIPNOTE.EXE -
       SYS$DISK:[.'dest']ZIPNOTE.OBJ, -
       'lib_ziputils' /library, -
       'opts' -
       SYS$DISK:[.'dest']ZIP.OPT /OPTIONS
$!
$     LINK /EXECUTABLE = [.'DEST']ZIPSPLIT.EXE -
       SYS$DISK:[.'DEST']ZIPSPLIT.OBJ, -
       'lib_ziputils' /LIBRARY, -
       'opts' -
       SYS$DISK:[.'dest']ZIP.OPT /options
$!
$ endif
$!
$!------------------------------ Symbols section -----------------------------
$!
$ if (MAKE_SYM)
$ then
$!
$     there = here- "]"+ ".''dest']"
$!
$!    Define the foreign command symbols.  Similar commands may be
$!    useful in SYS$MANAGER:SYLOGIN.COM and/or users' LOGIN.COM.
$!
$     zip      == "$''there'''ZIPEXEC'.exe"
$     zipcloak == "$''there'zipcloak.exe"
$     zipnote  == "$''there'zipnote.exe"
$     zipsplit == "$''there'zipsplit.exe"
$!
$ endif
$!
$! Deassign the temporary process logical names, restore the original
$! default directory, and restore the DCL verify status.
$!
$ error:
$!
$ if (IZ_BZIP2 .nes. "")
$ then
$     if (f$trnlnm( "incl_bzip2", "LNM$PROCESS_TABLE") .nes. "")
$     then
$         deassign incl_bzip2
$     endif
$     if (f$trnlnm( "lib_bzip2", "LNM$PROCESS_TABLE") .nes. "")
$     then
$         deassign lib_bzip2
$     endif
$ endif
$!
$ if (IZ_ZLIB .nes. "")
$ then
$     if (f$trnlnm( "incl_zlib", "LNM$PROCESS_TABLE") .nes. "")
$     then
$         deassign incl_zlib
$     endif
$     if (f$trnlnm( "lib_zlib", "LNM$PROCESS_TABLE") .nes. "")
$     then
$         deassign lib_zlib
$     endif
$ endif
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
