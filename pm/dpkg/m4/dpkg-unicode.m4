# serial 1
# Copyright © 1995-2003, 2005-2006 Free Software Foundation, Inc.
# Copyright © 2009 Yuri Vasilevski <yvasilev@gentoo.org>
# Copyright © 2010 Guillem Jover <guillem@debian.org>
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# DPKG_UNICODE()
# ------------
# Add configure option to disable Unicode support.
AC_DEFUN([DPKG_UNICODE], [
  AC_MSG_CHECKING([whether Unicode is requested])
  dnl Default: Unicode is enabled.
  AC_ARG_ENABLE([unicode],
    [AS_HELP_STRING([--disable-unicode],
      [do not use Unicode (wide chars) support])],
    [USE_UNICODE=$enableval],
    [USE_UNICODE=yes])
  AC_MSG_RESULT([$USE_UNICODE])
  AC_SUBST([USE_UNICODE])
]) # DPKG_UNICODE
