# serial 2
# Copyright © 2005 Scott James Remnant <scott@netsplit.com>
# Copyright © 2006-2023 Guillem Jover <guillem@debian.org>

# _DPKG_ARCHITECTURE([DEB_VAR], [sh_var])
# ------------------
# Use dpkg-architecture from the source tree to set sh_var using DEB_VAR for
# the target architecture, to avoid duplicating its logic.
AC_DEFUN([_DPKG_ARCHITECTURE], [
  AC_REQUIRE([DPKG_PROG_PERL])dnl
  AC_REQUIRE([AC_CANONICAL_HOST])dnl
  $2=$(PERL=$PERL ${CONFIG_SHELL-/bin/sh} "$srcdir/build-aux/run-script" scripts/dpkg-architecture.pl -t$host -q$1 2>/dev/null)
])# _DPKG_ARCHITECTURE

# DPKG_CPU_TYPE
# -------------
# Parse the host cpu name and check it against the cputable to determine
# the Debian name for it.  Sets ARCHITECTURE_CPU.
AC_DEFUN([DPKG_CPU_TYPE], [
  AC_MSG_CHECKING([dpkg cpu type])
  _DPKG_ARCHITECTURE([DEB_HOST_ARCH_CPU], [cpu_type])
  AS_IF([test -z "$cpu_type"], [
    cpu_type=$host_cpu
    AC_MSG_RESULT([$cpu_type])
    AC_MSG_WARN([$host_cpu not found in cputable])
  ], [
    AC_MSG_RESULT([$cpu_type])
  ])
  AC_DEFINE_UNQUOTED([ARCHITECTURE_CPU], ["${cpu_type}"],
    [Set this to the canonical dpkg CPU name.])
])# DPKG_CPU_TYPE

# DPKG_OS_TYPE
# ------------
# Parse the host operating system name and check it against a list of
# special cases to determine what type it is.  Sets ARCHITECTURE_OS.
AC_DEFUN([DPKG_OS_TYPE], [
  AC_MSG_CHECKING([dpkg operating system type])
  _DPKG_ARCHITECTURE([DEB_HOST_ARCH_OS], [os_type])
  AS_IF([test -z "$os_type"], [
    os_type=$host_os
    AC_MSG_RESULT([$os_type])
    AC_MSG_WARN([$host_os not found in ostable])
  ], [
    AC_MSG_RESULT([$os_type])
  ])
  AC_DEFINE_UNQUOTED([ARCHITECTURE_OS], ["${os_type}"],
    [Set this to the canonical dpkg system name.])
])# DPKG_OS_TYPE

# DPKG_ARCHITECTURE
# -----------------
# Determine the Debian name for the host operating system,
# sets ARCHITECTURE.
AC_DEFUN([DPKG_ARCHITECTURE], [
  DPKG_CPU_TYPE
  DPKG_OS_TYPE
  AC_MSG_CHECKING([dpkg architecture name])
  _DPKG_ARCHITECTURE([DEB_HOST_ARCH], [dpkg_arch])
  AS_IF([test -z "$dpkg_arch"], [
    AC_MSG_ERROR([cannot determine host dpkg architecture])
  ], [
    AC_MSG_RESULT([$dpkg_arch])
  ])
  AC_DEFINE_UNQUOTED([ARCHITECTURE], ["${dpkg_arch}"],
    [Set this to the canonical dpkg architecture name.])
])# DPKG_ARCHITECTURE

# DPKG_ARCH_ABI
# -------------
# Determine parts of the Debian architecture ABI for the host system,
# so that it can be printed during configure.
AC_DEFUN([DPKG_ARCH_ABI], [
  AC_CHECK_SIZEOF([short])
  AC_CHECK_SIZEOF([int])
  AC_CHECK_SIZEOF([long])
  AC_CHECK_SIZEOF([long long])
  AC_CHECK_SIZEOF([float])
  AC_CHECK_SIZEOF([double])
  AC_CHECK_SIZEOF([long double])
  AC_CHECK_SIZEOF([void *])
  AC_CHECK_SIZEOF([off_t], [], [[
#undef _TIME_BITS
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#include <sys/types.h>
]])
  AC_CHECK_SIZEOF([time_t], [], [[
#undef _TIME_BITS
#include <time.h>
]])

  AC_CHECK_ALIGNOF([max_align_t])

  AC_C_CHAR_UNSIGNED
  AS_IF([test "$ac_cv_c_char_unsigned" = yes],
    [dpkg_char_sign=unsigned],
    [dpkg_char_sign=signed]
  )

  AC_C_BIGENDIAN
  AS_CASE([$ac_cv_c_bigendian],
    [yes], [dpkg_arch_endian=big],
    [no], [dpkg_arch_endian=little],
    [dpkg_arch_endian=$ac_cv_c_bigendian]
  )

  AC_COMPUTE_INT([dpkg_char_bits], [CHAR_BIT], [#include <limits.h>])
  dpkg_arch_bits=$((ac_cv_sizeof_void_p * dpkg_char_bits))
])# DPKG_ARCH_ABI
