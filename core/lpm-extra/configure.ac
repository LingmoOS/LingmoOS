AC_INIT([lpm-contrib],[1.10.6],[lpm-contrib@lists.archlinux.org])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AM_INIT_AUTOMAKE([-Wall foreign])
CPPFLAGS="$CPPFLAGS -D_FORTIFY_SOURCE=2"

# Help line for root directory
AC_ARG_WITH(root-dir,
	AS_HELP_STRING([--with-root-dir=PATH], [set the location of the root operating directory]),
	[ROOTDIR=$withval], [ROOTDIR=/])

# Help line for vim runtime directory
AC_ARG_WITH(vim-dir,
	[AS_HELP_STRING([--with-vim-dir=PATH], [set the location of the vim runtime file directory])],
	[vim_dir=$withval], [vim_dir=$datadir/vim/vimfiles])

# Help line for documentation
AC_ARG_ENABLE(doc,
	AS_HELP_STRING([--disable-doc], [prevent make from looking at doc/ dir]),
	[wantdoc=$enableval], [wantdoc=yes])

# Help line for debug
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [enable debugging support]),
	[debug=$enableval], [debug=no])

# Help line for compiler warning flags
AC_ARG_ENABLE(warningflags,
	AS_HELP_STRING([--enable-warningflags], [enable extra compiler warning flags]),
	[warningflags=$enableval], [warningflags=no])

# Help line for using git version in lpm version string
AC_ARG_ENABLE(git-version,
	AS_HELP_STRING([--enable-git-version],
		[enable use of git version in version string if available]),
	[wantgitver=$enableval], [wantgitver=no])

# Checks for programs.
AC_PROG_CC
AC_PATH_PROGS([BASH_SHELL], [bash bash4], [false])

AS_IF([test "x$BASH_SHELL" = "xfalse"],
	AC_MSG_WARN([*** bash >= 4.1.0 is required for lpm scripts]),
	[bash_version_major=`$BASH_SHELL -c 'echo "${BASH_VERSINFO[[0]]}"'`
	bash_version_minor=`$BASH_SHELL -c 'echo "${BASH_VERSINFO[[1]]}"'`
	ok=yes
	if test "$bash_version_major" -lt 4; then
		ok=no
	fi
	if test "$bash_version_major" -eq 4 && test "$bash_version_minor" -lt 1; then
		ok=no
	fi
	if test "$ok" = "no"; then
		AC_MSG_ERROR([*** bash >= 4.1.0 is required for lpm scripts])
	fi
	unset bash_version_major bash_version_minor ok])

# Check for libarchive
PKG_CHECK_MODULES(LIBARCHIVE, [libarchive >= 2.8.0], ,
	AC_MSG_ERROR([*** libarchive >= 2.8.0 is needed to build lpm-contrib!]))

# Check for libalpm
PKG_CHECK_MODULES(LIBALPM, [libalpm], ,
	AC_MSG_ERROR([*** libalpm is needed to build lpm-contrib!]))

PKG_CHECK_VAR(LIBMAKEPKGDIR, [libmakepkg], [libmakepkgdir], ,
	AC_MSG_ERROR([*** libmakepkg is needed to build lpm-contrib!]))
AC_SUBST(LIBMAKEPKGDIR)

# Enable or disable use of git version in version string
AC_MSG_CHECKING(whether to use git version if available)
if test "x$wantgitver" = "xyes" ; then
	AC_CHECK_PROGS([GIT], [git])
	AC_CHECK_FILE([.git/], hasgitdir=yes)
	usegitver=$ac_cv_file__git_
	if test $GIT -a "x$hasgitdir" = "xyes"; then
		AC_DEFINE([USE_GIT_VERSION], , [Use GIT version in version string])
	fi
else
	AC_MSG_RESULT([no, disabled by configure])
	usegitver=no
fi
AM_CONDITIONAL(USE_GIT_VERSION, test "x$usegitver" = "xyes")

# Set root directory
AC_SUBST(ROOTDIR)
AC_DEFINE_UNQUOTED([ROOTDIR], "$ROOTDIR", [The location of the root operating directory])
# Set vim directory
AC_SUBST(vim_dir)
AC_DEFINE_UNQUOTED([vim_dir], "$vim_dir", [The location of the vim runtime file directory])

SIZECMD="stat -c %s"
AC_SUBST(SIZECMD)

# Check for documentation support and status
AC_CHECK_PROGS([ASCIIDOC], [asciidoc])
AC_MSG_CHECKING([for building documentation])
if test "x$wantdoc" = "xyes" ; then
	if test $ASCIIDOC ; then
		AC_MSG_RESULT([yes, enabled by configure])
	else
		asciidoc="(warning : asciidoc not installed)"
		AC_MSG_RESULT([yes $asciidoc])
	fi
	wantdoc=yes
else
	AC_MSG_RESULT([no, disabled by configure])
	wantdoc=no
fi
AM_CONDITIONAL(WANT_DOC, test "x$wantdoc" = "xyes")

# Enable or disable debug code
if test "x$debug" = "xyes" ; then
	AC_MSG_RESULT(yes)
	WARNING_CFLAGS="-g -Wall -Werror"
else
	AC_MSG_RESULT(no)
	WARNING_CFLAGS="-Wall"
fi

# Enable or disable compiler warning flags
AC_MSG_CHECKING(for excessive compiler warning flags)
if test "x$warningflags" = "xyes" ; then
	AC_MSG_RESULT(yes)
	CFLAGS_ADD([-Wcast-align], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wclobbered], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wempty-body], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wfloat-equal], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wformat-nonliteral], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wformat-security], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wignored-qualifiers], [WARNING_CFLAGS])
	CFLAGS_ADD([-Winit-self], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wlogical-op], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wmissing-declarations], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wmissing-field-initializers], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wmissing-parameter-type], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wmissing-prototypes], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wold-style-declaration], [WARNING_CFLAGS])
	CFLAGS_ADD([-Woverride-init], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wpointer-arith], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wredundant-decls], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wshadow], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wsign-compare], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wstrict-aliasing], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wstrict-overflow=5], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wstrict-prototypes], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wtype-limits], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wuninitialized], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wunused-but-set-parameter], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wunused-parameter], [WARNING_CFLAGS])
	CFLAGS_ADD([-Wwrite-strings], [WARNING_CFLAGS])
else
	AC_MSG_RESULT(no)
fi

AC_CONFIG_FILES([
	completions/Makefile
	lib/Makefile
	src/Makefile
	doc/Makefile
	test/Makefile
	Makefile
])
AC_OUTPUT

echo "
${PACKAGE_NAME}:

  Build information:
    prefix                 : ${prefix}
    sysconfdir             : $(eval echo ${sysconfdir})
       conf file           : $(eval echo ${sysconfdir})/lpm.conf
    localstatedir          : $(eval echo ${localstatedir})
       database dir        : $(eval echo ${localstatedir})/lib/lpm/
       cache dir           : $(eval echo ${localstatedir})/cache/lpm/pkg/
    libmakepkg dir         : $(eval echo ${LIBMAKEPKGDIR})

    compiler               : ${CC}
    compiler flags         : ${WARNING_CFLAGS} ${CFLAGS}
    linker flags           : ${LDFLAGS} ${LIBALPM_LIBS} ${LIBARCHIVE_LIBS}

    Filesize command       : ${SIZECMD}

    using git version      : ${usegitver}

  Directory and file information:
    root working directory : ${ROOTDIR}

  Compilation options:
    Run make in doc/ dir   : ${wantdoc} ${asciidoc}
    debug support          : ${debug}
    extra warning flags    : ${warningflags}
    use git version        : ${wantgitver}
"
