# Process this file with autoconf to produce a configure script.

m4_pattern_forbid([^_?DPKG_])

AC_PREREQ(2.60)
AC_INIT([dpkg], m4_esyscmd([build-aux/get-version]),
        [debian-dpkg@lists.debian.org], [dpkg],
        [https://wiki.debian.org/Teams/Dpkg])
DPKG_DIST_IS_RELEASE
AC_SUBST([PACKAGE_COPYRIGHT_HOLDER], ['Dpkg Developers'])
AC_SUBST([PACKAGE_VCS_ID], m4_esyscmd([build-aux/get-vcs-id]))
AC_SUBST([PACKAGE_VCS_TYPE], [git])
AC_SUBST([PACKAGE_VCS_URL], [https://git.dpkg.org/git/dpkg/dpkg.git])
AC_SUBST([PACKAGE_VCS_WEB], [https://git.dpkg.org/cgit/dpkg/dpkg.git])
AC_SUBST([PACKAGE_BUG_WEB], [https://bugs.debian.org/src:dpkg])
AC_SUBST([PACKAGE_CPAN_NAME], [Dpkg])
AC_CONFIG_SRCDIR([lib/dpkg/dpkg.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE(
  [1.11]
  [-Wall]
  [foreign]
  [nostdinc]
  [subdir-objects]
  [tar-ustar no-dist-gzip dist-xz]
)
AM_SILENT_RULES([yes])

# Require at least this gettext version, but will take any later version too.
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.8])
# XXX: We cannot remove the following macro due to backwards compatibility
# reasons. The above macro is set unconditionally to the minimal version
# required, and the below is set to 0.19.6, the first version introducing
# the AM_GNU_GETTEXT_REQUIRE_VERSION macro.
AM_GNU_GETTEXT_VERSION([0.19.6])
AM_GNU_GETTEXT([external])
DPKG_DIST_CHECK([test "$GMSGFMT" = ":" && test "$USE_NLS" = "yes"],
  [gettext required when NLS support enabled])

# Shared libraries are disabled on purpose, currently there is no ABI stability
# guarantee, and it will be broken at will. The infrastructure is in place just
# to be able to test that its future activation will work.
AM_PROG_AR
LT_INIT([disable-shared])
DPKG_BUILD_SHARED_LIBS
DPKG_LINKER_AS_NEEDED
DPKG_LINKER_VERSION_SCRIPT

# Allow compilation without optional programs
DPKG_BUILD_PROG([dselect])
DPKG_BUILD_PROG([start-stop-daemon])
DPKG_BUILD_PROG([update-alternatives])

DPKG_BUILD_RELEASE_DATE
DPKG_BUILD_DEVEL_DOCS

# Allow alternate directories
DPKG_WITH_DIR([devlibdir], [${libdir}],
  [dpkg development library directory [LIBDIR]])
DPKG_WITH_DIR([pkgconfdir], [${sysconfdir}/${PACKAGE_TARNAME}],
  [dpkg configuration directory [SYSCONFDIR/dpkg]])
DPKG_WITH_DIR([docspecdir], [${docdir}/spec],
  [dpkg specifications directory [DOCDIR/spec]])
DPKG_WITH_DIR([methodsdir], [${libexecdir}/${PACKAGE_TARNAME}/methods],
  [dpkg download methods directory [LIBEXECDIR/dpkg/methods]])
DPKG_WITH_DIR([admindir], [${localstatedir}/lib/${PACKAGE_TARNAME}],
  [dpkg database directory [LOCALSTATEDIR/lib/dpkg]])
DPKG_WITH_DIR([backupsdir], [${localstatedir}/backups],
  [dpkg database backups directory [LOCALSTATEDIR/backups]])
DPKG_WITH_DIR([logdir], [${localstatedir}/log],
  [system logging directory [LOCALSTATEDIR/log]])
DPKG_WITH_DIR([pkgconfigdir], [${devlibdir}/pkgconfig],
  [pkg-config .pc fragments directory [DEVLIBDIR/pkgconfig]])
DPKG_WITH_DIR([aclocaldir], [${datadir}/aclocal],
  [aclocal m4 fragments files directory [DATADIR/aclocal]])
DPKG_WITH_DIR([polkitactionsdir], [${datadir}/polkit-1/actions],
  [polkit .policy actions directory [DATADIR/polkit-1/actions]])
DPKG_WITH_DIR([bashcompletionsdir], [${datadir}/bash-completion/completions],
  [bash completions directory [DATADIR/bash-completion/completions]])
DPKG_WITH_DIR([zshcompletionsdir], [${datadir}/zsh/vendor-completions],
  [zsh vendor completions directory [DATADIR/zsh/vendor-completions]])

# Set default dpkg-deb values
DPKG_DEB_COMPRESSOR([xz])

# Checks for programs.
DPKG_PROG_SHELL
DPKG_PROG_PAGER
AC_PROG_SED
AC_PROG_GREP
AC_PROG_EGREP
AC_PROG_CC
DPKG_C_STD([99])
AC_PROG_CXX
DPKG_CXX_STD([14])
DPKG_PROG_PATCH
AC_CHECK_PROGS([DOXYGEN], [doxygen])
AC_CHECK_PROG([HAVE_DOT], [dot], [YES], [NO])
DPKG_PROG_TAR
DPKG_PROG_PO4A
DPKG_PROG_PERL
DPKG_PROG_POD2MAN
DPKG_CODE_COVERAGE

# Checks for operating system services and capabilities.
AC_SYS_LARGEFILE

# Checks for libraries.
DPKG_LIB_RT
DPKG_LIB_MD
DPKG_LIB_Z
DPKG_LIB_BZ2
DPKG_LIB_LZMA
DPKG_LIB_ZSTD
DPKG_LIB_SELINUX
AS_IF([test "$build_dselect" = "yes"], [
  DPKG_LIB_CURSES
])
AS_IF([test "$build_start_stop_daemon" = "yes"], [
  DPKG_LIB_SOCKET
  DPKG_LIB_PS
  DPKG_LIB_KVM
])

# Checks for header files.
AC_CHECK_HEADERS([\
  error.h \
  err.h \
  locale.h \
  xlocale.h \
  libintl.h \
  kvm.h \
  sys/sysmacros.h \
  sys/param.h \
  sys/syscall.h \
  sys/user.h \
  sys/mkdev.h \
  sys/pstat.h \
  linux/fiemap.h \
])
AS_CASE([$host_os],
  [linux-gnu*], [
    # The glibc project has deprecated the <sys/sysctl.h> header and emits
    # warnings on its usage, and removed it in 2.32. The Linux kernel removed
    # support for it in 5.5.
  ], [
    # On other systems, we need to check whether to use it.
    AC_CHECK_HEADERS([\
      sys/sysctl.h \
    ])
  ]
)
DPKG_CHECK_HEADER_SYS_PROCFS
AC_CHECK_HEADERS([sys/proc.h], [], [], [
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
])

AS_IF([test "$build_dselect" = "yes"], [
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([cxxabi.h])
  AC_LANG_POP([C++])
])

# Checks for typedefs, structures, and compiler characteristics.
DPKG_ARCH_ABI
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
DPKG_TYPE_PTRDIFF_T
DPKG_TYPE_STRUCT_PSINFO
DPKG_DECL_SYS_SIGLIST
DPKG_DECL_SYS_ERRLIST

# Checks for library functions.
DPKG_FUNC_VA_COPY
DPKG_FUNC_C99_SNPRINTF
DPKG_FUNC_FSYNC_DIR
DPKG_CHECK_DECL([offsetof], [stddef.h])
DPKG_CHECK_DECL([makedev], [sys/types.h])
DPKG_CHECK_DECL([WCOREDUMP], [sys/wait.h])
DPKG_CHECK_DECL([TIOCNOTTY], [sys/ioctl.h])
DPKG_CHECK_DECL([O_NOFOLLOW], [fcntl.h])
DPKG_CHECK_DECL([F_ALLOCSP64], [fcntl.h])
DPKG_CHECK_DECL([F_PREALLOCATE], [fcntl.h])
DPKG_CHECK_DECL([P_tmpdir], [stdio.h])
DPKG_CHECK_PROGNAME
DPKG_CHECK_COMPAT_FUNCS([\
  getopt \
  getopt_long \
  obstack_free \
  strchrnul \
  strnlen \
  strndup \
  strerror \
  strsignal \
  asprintf \
  fgetpwent \
  fgetgrent \
  scandir \
  alphasort \
  unsetenv \
])
AC_CHECK_FUNCS([\
  lchown \
], [], [AC_MSG_ERROR([missing required function])])
AC_CHECK_FUNCS([\
  setsid \
  getdtablesize \
  closefrom \
  close_range \
  getprocs64 \
  getprogname \
  getexecname \
  lutimes \
  fallocate \
  posix_fallocate \
  posix_fadvise \
  uselocale \
])

AS_IF([test "$build_dselect" = "yes"], [
  AC_LANG_PUSH([C++])
  AC_CHECK_FUNCS([__cxa_pure_virtual])
  AC_LANG_POP([C++])
])

DPKG_USE_MMAP
DPKG_USE_DISK_PREALLOCATE

# Checks for the build machinery.
AC_DEFINE([LIBDPKG_VOLATILE_API], [1], [Acknowledge the volatility of the API.])
DPKG_COMPILER_DIALECT
DPKG_COMPILER_HARDENING
DPKG_COMPILER_WARNINGS
DPKG_COMPILER_SANITIZER
DPKG_COMPILER_ANALYZER
DPKG_COMPILER_OPTIMIZATIONS
DPKG_LINKER_OPTIMIZATIONS
DPKG_ARCHITECTURE
AC_DEFINE([PACKAGE_RELEASE], [PACKAGE_VERSION " (" ARCHITECTURE ")"],
  [Define the project release information, version and architecture])

AC_CONFIG_TESTDIR([src/at])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_CONFIG_FILES([
  Makefile
  dselect/Makefile
  dselect/methods/Makefile
  dselect/po/Makefile.in
  lib/Makefile
  lib/compat/Makefile
  lib/dpkg/Makefile
  lib/dpkg/libdpkg.pc
  doc/Doxyfile
  man/Makefile
  po/Makefile.in
  scripts/Build.PL
  scripts/Makefile
  scripts/README.cpan
  scripts/mk/Makefile
  scripts/po/Makefile.in
  src/Makefile
  src/at/atlocal
  utils/Makefile
])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT

# Print the current configuration
cat <<CONFIG

Release:
  Version . . . . . : $PACKAGE_VERSION
  VCS id  . . . . . : $PACKAGE_VCS_ID

Configuration:
  Arch attributes:
    cpu name  . . . . . . . . . . : ${cpu_type}
    os name . . . . . . . . . . . : ${os_type}
    arch name . . . . . . . . . . : ${dpkg_arch}
    arch bits . . . . . . . . . . : ${dpkg_arch_bits}
    arch endian . . . . . . . . . : ${dpkg_arch_endian}
    char bits . . . . . . . . . . : ${dpkg_char_bits}
    char sign . . . . . . . . . . : ${dpkg_char_sign}
    sizeof(short) . . . . . . . . : ${ac_cv_sizeof_short}
    sizeof(int) . . . . . . . . . : ${ac_cv_sizeof_int}
    sizeof(long)  . . . . . . . . : ${ac_cv_sizeof_long}
    sizeof(long long) . . . . . . : ${ac_cv_sizeof_long_long}
    sizeof(float) . . . . . . . . : ${ac_cv_sizeof_float}
    sizeof(double)  . . . . . . . : ${ac_cv_sizeof_double}
    sizeof(long double) . . . . . : ${ac_cv_sizeof_long_double}
    sizeof(void *)  . . . . . . . : ${ac_cv_sizeof_void_p}
    sizeof(off_t) . . . . . . . . : ${ac_cv_sizeof_off_t}
    sizeof(time_t)  . . . . . . . : ${ac_cv_sizeof_time_t}
    alignof(max_align_t)  . . . . : ${ac_cv_alignof_max_align_t}

  Features:
    native language support . . . : $USE_NLS
    unicode support . . . . . . . : $USE_UNICODE
    development documentation . . : $build_devel_docs
    code coverage . . . . . . . . : $enable_coverage
    build shared libraries  . . . : $enable_shared
    mmap loaders  . . . . . . . . : $enable_mmap
    disk pre-allocation . . . . . : $enable_disk_preallocate
    default dpkg-deb compressor . : $with_deb_compressor
    default shell interpreter . . : $DPKG_SHELL
    default pager program . . . . : $DPKG_PAGER

  Paths:
    devlibdir . . . . . . . . . . : $devlibdir
    pkgconfdir  . . . . . . . . . : $pkgconfdir
    docspecdir  . . . . . . . . . : $docspecdir
    methodsdir  . . . . . . . . . : $methodsdir
    admindir  . . . . . . . . . . : $admindir
    backupsdir  . . . . . . . . . : $backupsdir
    logdir  . . . . . . . . . . . : $logdir
    pkg-config dir  . . . . . . . : $pkgconfigdir
    aclocal dir   . . . . . . . . : $aclocaldir
    polkit actions dir  . . . . . : $polkitactionsdir
    bash completions dir  . . . . : $bashcompletionsdir
    zsh completions dir . . . . . : $zshcompletionsdir
    perl interpreter  . . . . . . : $PERL
    perl libdir . . . . . . . . . : $PERL_LIBDIR

  Programs:
    update-alternatives . . . . . : $build_update_alternatives
    start-stop-daemon . . . . . . : $build_start_stop_daemon
    dselect . . . . . . . . . . . : $build_dselect

  System Libraries:
    librt . . . . . . . . . . . . : $have_librt
    libsocket . . . . . . . . . . : ${have_libsocket:-no}
    libps . . . . . . . . . . . . : ${have_libps:-no}
    libkvm  . . . . . . . . . . . : ${have_libkvm:-no}
    libselinux  . . . . . . . . . : $have_libselinux
    libmd . . . . . . . . . . . . : $have_libmd
    libz  . . . . . . . . . . . . : $have_libz_impl
    liblzma . . . . . . . . . . . : $have_liblzma
    libzstd . . . . . . . . . . . : $have_libzstd
    libbz2  . . . . . . . . . . . : $have_libbz2
    libcurses . . . . . . . . . . : ${have_libcurses:-no}
CONFIG
