#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(lingmo-interface,1.0,liuhao@kylinos.cn)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

# Checks for libraries.

PKG_CHECK_MODULES(GLIB_2, glib-2.0 >= 1.00)
PKG_CHECK_MODULES(GIO_2, gio-2.0 >= 1.00)
PKG_CHECK_MODULES(GIO_UNIX_2, gio-unix-2.0 >= 1.00)
PKG_CHECK_MODULES(QT5CORE, Qt5Core >= 5.6.1)

# FIXME: Replace `main' with a function in `-lgsettings':
AC_CHECK_LIB([gsettings], [main])
# FIXME: Replace `main' with a function in `-lprint':
AC_CHECK_LIB([print], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 po/Makefile
                 src/Makefile
                 src/accounts/Makefile
                 src/accounts/user/Makefile
                 src/common/Makefile
                 src/date/Makefile
                 src/date/dttm/Makefile
                 src/hardware/Makefile
                 src/hardware/keyboard/Makefile
                 src/hardware/mouse/Makefile
                 src/hardware/touchpad/Makefile
                 src/hardware/power/Makefile
                 src/language/Makefile
                 src/network/Makefile
                 src/network/network/Makefile
                 src/os/Makefile
                 src/os/subversion/Makefile
                 src/os/sysinfo/Makefile
                 src/other/Makefile
                 src/personal/Makefile
                 src/personal/background/Makefile
                 src/personal/desktop/Makefile
                 src/personal/font/Makefile
                 src/personal/marcogeneral/Makefile
                 src/security/Makefile
                 src/software/Makefile
                 src/software/defaultprograms/Makefile
                 src/updates/Makefile])
AC_OUTPUT
