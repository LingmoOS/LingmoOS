# serial 2
# Copyright © 2005 Scott James Remnant <scott@netsplit.com>
# Copyright © 2009-2023 Guillem Jover <guillem@debian.org>

# DPKG_TYPE_PTRDIFF_T
# -------------------
# Check for the ptrdiff_t type, defining to int if not defined
AC_DEFUN([DPKG_TYPE_PTRDIFF_T], [
  AC_CHECK_TYPE([ptrdiff_t], [], [
    AC_DEFINE_UNQUOTED([ptrdiff_t], [int],
      [Define to 'int' if <malloc.h> does not define.])
  ])
])# DPKG_TYPE_PTRDIFF_T

# DPKG_TYPE_STRUCT_PSINFO
# -----------------------
# Check for the struct psinfo type.
AC_DEFUN([DPKG_TYPE_STRUCT_PSINFO], [
  AC_REQUIRE([DPKG_CHECK_HEADER_SYS_PROCFS])

  AC_CHECK_TYPES([struct psinfo], [], [], [[
#if !DPKG_STRUCTURED_PROCFS_SUPPORTS_LFS
#undef _FILE_OFFSET_BITS
#endif
#define _STRUCTURED_PROC 1
#ifdef HAVE_SYS_PROCFS_H
#include <sys/procfs.h>
#endif
  ]])
])# DPKG_TYPE_STRUCT_PSINFO

# DPKG_DECL_SYS_SIGLIST
# ---------------------
# Check for the sys_siglist variable in either signal.h or unistd.h
AC_DEFUN([DPKG_DECL_SYS_SIGLIST], [
  AC_CHECK_HEADERS([unistd.h])
  AC_CHECK_DECLS([sys_siglist], [], [], [[
#include <signal.h>
/* NetBSD declares sys_siglist in unistd.h.  */
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
  ]])
])# DPKG_DECL_SYS_SIGLIST

# DPKG_DECL_SYS_ERRLIST
# ---------------------
# Check for the sys_errlist and sys_nerr variables in either errno.h or
# stdio.h
AC_DEFUN([DPKG_DECL_SYS_ERRLIST], [
  AC_CHECK_DECLS([sys_errlist, sys_nerr], [], [], [[
#include <errno.h>
/* glibc declares sys_errlist in stdio.h.  */
#include <stdio.h>
  ]])
  AM_CONDITIONAL([HAVE_SYS_ERRLIST],
    [test "$ac_cv_have_decl_sys_errlist" = "yes" && \
     test "$ac_cv_have_decl_sys_nerr" = "yes"])
])# DPKG_DECL_SYS_SIGLIST

# DPKG_CHECK_DECL([DECL], [HEADER])
# ---------------
# Define HAVE_DECL to 1 if declared in HEADER
AC_DEFUN([DPKG_CHECK_DECL], [
  AC_CHECK_DECL([$1], [
    AC_DEFINE(AS_TR_CPP([HAVE_$1]), [1],
      [Define to 1 if ']$1[' is declared in <$2>])
  ], [], [[#include <$2>]])
])# DPKG_CHECK_DECL
