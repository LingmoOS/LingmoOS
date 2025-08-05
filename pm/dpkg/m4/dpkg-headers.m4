# serial 1
# Copyright © 2023 Guillem Jover <guillem@debian.org>

# DPKG_CHECK_HEADER_SYS_PROCFS
# ----------------------------
# Check for the <sys/procfs.h> header.
AC_DEFUN([DPKG_CHECK_HEADER_SYS_PROCFS], [
  # On at least Solaris <= 11.3 we cannot use the new structured procfs API
  # while also using LFS, so we need to check whether including <sys/procfs.h>
  # is usable, and otherwise undefine _FILE_OFFSET_BITS, but only for the
  # code using this (that is s-s-d).
  dpkg_structured_procfs_supports_lfs=1
  AC_CHECK_HEADERS([sys/procfs.h], [], [
    # We need to reset the cached value, otherwise we will not be able to
    # check it again.
    AS_UNSET([ac_cv_header_sys_procfs_h])
    # If a bare include failed, check whether we need to disable LFS to be
    # able to use structured procfs.
    AC_CHECK_HEADERS([sys/procfs.h], [
        dpkg_structured_procfs_supports_lfs=0
      ], [], [[
#undef _FILE_OFFSET_BITS
#define _STRUCTURED_PROC 1
    ]])
  ], [[
#define _STRUCTURED_PROC 1
  ]])

  AC_DEFINE_UNQUOTED([DPKG_STRUCTURED_PROCFS_SUPPORTS_LFS],
    [$dpkg_structured_procfs_supports_lfs],
    [Whether structured procfs supports LFS])
])# DPKG_CHECK_HEADER_SYS_PROCFS
