# FLM_PROG_TRY_DOXYGEN(["quiet"])
# ------------------------------
# FLM_PROG_TRY_DOXYGEN tests for an existing doxygen source
# documentation program. It sets or uses the environment 
# variable DOXYGEN. 
#
# If no arguments are given to this macro, and no doxygen 
# program can be found, it prints a warning message to STDOUT
# and to the config.log file. If the "quiet" argument is passed, 
# then only the normal "check" line is displayed. Any other
# argument is considered by autoconf to be an error at expansion
# time.
#
# Makes the DOXYGEN variable precious to Autoconf. You can 
# use the DOXYGEN variable in your Makefile.in files with 
# @DOXYGEN@.
#
# Author:   John Calcote <john.calcote@gmail.com>
# Modified: 2009-08-30
# License:  AllPermissive
#
AC_DEFUN([FLM_PROG_TRY_DOXYGEN],
[AC_ARG_VAR([DOXYGEN], [Doxygen source doc generation program])dnl
AC_CHECK_PROGS([DOXYGEN], [doxygen])
m4_if([$1],,
[if test -z "$DOXYGEN"; then
  AC_MSG_WARN([doxygen not found - continuing without Doxygen support])
  fi], [$1], [quiet],, [m4_fatal([Invalid option '$1' in $0])])
  ])# FLM_PROG_TRY_DOXYGEN
