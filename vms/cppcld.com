$!                                              6 August 2011.  SMS.
$!
$! Info-ZIP VMS accessory procedure.
$!
$! P1 = CC command
$! P2 = Command definition input file (".CLD") with CPP directives.
$! P3 = Command definition output file.
$! P4 = Comma-separated list of C macros.
$!
$!
$ cc = f$edit( p1, "trim")
$ if (cc .eqs. "")
$ then
$     cc = "cc"
$ endif
$!
$! C preprocessing.
$!
$ 'cc' /preprocess_only = 'p3' 'p2' /define = ('p4') /undefine = (VMS)
$!
$! Strip out the added file indentification line.
$!
$ line_nr = 0
$ open /read /error = error_r in_file 'p3'
$ open /write /error = error_w out_file 'p3'
$ loop_top:
$     read /error = error_c in_file line
$     if ((line_nr .ne. 0) .or. (f$extract( 0, 1, line) .nes. "#"))
$     then
$         write /error = error_c out_file line
$     endif
$     line_nr = 1
$ goto loop_top
$!
$ error_c:
$ close out_file
$ error_w:
$ close in_file
$ error_r:
$!
