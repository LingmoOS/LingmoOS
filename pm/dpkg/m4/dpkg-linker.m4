# serial 1
# Copyright © 2004 Scott James Remnant <scott@netsplit.com>.
# Copyright © 2010, 2014, 2016 Guillem Jover <guillem@debian.org>

# DPKG_LINKER_OPTIMIZATIONS
# -------------------------
# Add configure option to disable linker optimizations.
AC_DEFUN([DPKG_LINKER_OPTIMIZATIONS], [
  AC_ARG_ENABLE([linker-optimizations],
    [AS_HELP_STRING([--disable-linker-optimizations],
      [Disable (detected) linker optimizations])],
    [], [enable_linker_optimizations=yes])

  AS_IF([test "$enable_linker_optimizations" = "no"], [
    LDFLAGS=$(echo "$LDFLAGS" | $SED -e "s/ -Wl,-O[[0-9]]*\b//g")
  ], [
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,-O1"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[]], [[]])
    ], [], [
      LDFLAGS="$save_LDFLAGS"
    ])
  ])
])

# DPKG_LINKER_AS_NEEDED
# ---------------------
AC_DEFUN([DPKG_LINKER_AS_NEEDED], [
  AC_CACHE_CHECK([for --as-needed linker flag], [dpkg_cv_linker_as_needed], [
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,--as-needed"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([], [])
    ], [
      dpkg_cv_linker_as_needed=yes
    ], [
      dpkg_cv_linker_as_needed=no
    ])
    LDFLAGS="$save_LDFLAGS"
  ])
  AM_CONDITIONAL([HAVE_LINKER_AS_NEEDED],
    [test "$dpkg_cv_linker_as_needed" = "yes"])
])

# DPKG_LINKER_VERSION_SCRIPT
# --------------------------
AC_DEFUN([DPKG_LINKER_VERSION_SCRIPT], [
  AC_CACHE_CHECK([for --version-script linker flag], [dpkg_cv_version_script], [
    echo "{ global: symbol; local: *; };" >conftest.map
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.map"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
extern int symbol(void);
int symbol(void) { return 0; }
]], [[
]])
    ], [
      dpkg_cv_version_script=yes
    ], [
      dpkg_cv_version_script=no
    ])
    LDFLAGS="$save_LDFLAGS"
    rm -f conftest.map
  ])
  AM_CONDITIONAL([HAVE_LINKER_VERSION_SCRIPT],
    [test "$dpkg_cv_version_script" = "yes"])
])
