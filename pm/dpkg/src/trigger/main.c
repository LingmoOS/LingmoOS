/*
 * dpkg-trigger - trigger management utility
 *
 * Copyright © 2007 Canonical Ltd.
 * Written by Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/types.h>

#include <fcntl.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/options.h>
#include <dpkg/trigdeferred.h>
#include <dpkg/triglib.h>
#include <dpkg/pkg-spec.h>

static const char printforhelp[] = N_(
"Type dpkg-trigger --help for help about this utility.");

static int
printversion(const char *const *argv)
{
	printf(_("Debian %s package trigger utility version %s.\n"),
	       dpkg_get_progname(), PACKAGE_RELEASE);

	printf(_(
"This is free software; see the GNU General Public License version 2 or\n"
"later for copying conditions. There is NO warranty.\n"));

	m_output(stdout, _("<standard output>"));

	return 0;
}

static int
usage(const char *const *argv)
{
	printf(_(
"Usage: %s [<option>...] <trigger-name>\n"
"       %s [<option>...] <command>\n"
"\n"), dpkg_get_progname(), dpkg_get_progname());

	printf(_(
"Commands:\n"
"  --check-supported                Check if the running dpkg supports triggers.\n"
"\n"));

	printf(_(
"  -?, --help                       Show this help message.\n"
"      --version                    Show the version.\n"
"\n"));

	printf(_(
"Options:\n"
"  --admindir=<directory>           Use <directory> instead of %s.\n"
"  --root=<directory>               Use <directory> instead of %s.\n"
"  --by-package=<package>           Override trigger awaiter (normally set\n"
"                                     by dpkg).\n"
"  --await                          Package needs to await the processing.\n"
"  --no-await                       No package needs to await the processing.\n"
"  --no-act                         Just test - don't actually change anything.\n"
"\n"), ADMINDIR, "/");

	m_output(stdout, _("<standard output>"));

	return 0;
}

static int opt_act = 1;
static int opt_await = 1;
static const char *opt_bypackage;

static const char *activate;
static bool done_trig, ctrig;

static void
yespackage(const char *awname)
{
	trigdef_update_printf(" %s", awname);
}

static const char *
parse_awaiter_package(void)
{
	struct dpkg_error err = DPKG_ERROR_INIT;
	struct pkginfo *pkg;

	if (!opt_await)
		opt_bypackage = "-";

	if (opt_bypackage == NULL) {
		const char *pkgname, *archname;

		pkgname = getenv("DPKG_MAINTSCRIPT_PACKAGE");
		archname = getenv("DPKG_MAINTSCRIPT_ARCH");
		if (pkgname == NULL || archname == NULL)
			badusage(_("must be called from a maintainer script"
			           " (or with a --by-package option)"));

		pkg = pkg_spec_find_pkg(pkgname, archname, &err);
	} else if (strcmp(opt_bypackage, "-") == 0) {
		pkg = NULL;
	} else {
		pkg = pkg_spec_parse_pkg(opt_bypackage, &err);
	}

	/* Normalize the bypackage name if there was no error. */
	if (pkg)
		opt_bypackage = pkg_name(pkg, pnaw_same);

	return err.str;
}

static void
tdm_add_trig_begin(const char *trig)
{
	ctrig = strcmp(trig, activate) == 0;
	trigdef_update_printf("%s", trig);
	if (!ctrig || done_trig)
		return;
	yespackage(opt_bypackage);
	done_trig = true;
}

static void
tdm_add_package(const char *awname)
{
	if (ctrig && strcmp(awname, opt_bypackage) == 0)
		return;
	yespackage(awname);
}

static void
tdm_add_trig_end(void)
{
	trigdef_update_printf("\n");
}

static const struct trigdefmeths tdm_add = {
	.trig_begin = tdm_add_trig_begin,
	.package = tdm_add_package,
	.trig_end = tdm_add_trig_end,
};

static int
do_trigger(const char *const *argv)
{
	const char *badname;
	enum trigdef_update_flags tduf;
	enum trigdef_update_status tdus;

	if (!*argv || argv[1])
		badusage(_("takes one argument, the trigger name"));

	badname = parse_awaiter_package();
	if (badname)
		badusage(_("illegal awaited package name '%.250s': %.250s"),
		         opt_bypackage, badname);

	activate = argv[0];
	badname = trig_name_is_illegal(activate);
	if (badname)
		badusage(_("invalid trigger name '%.250s': %.250s"),
		         activate, badname);

	trigdef_set_methods(&tdm_add);

	tduf = TDUF_NO_LOCK_OK;
	if (opt_act)
		tduf |= TDUF_WRITE | TDUF_WRITE_IF_EMPTY;
	tdus = trigdef_update_start(tduf);
	if (tdus >= 0) {
		trigdef_parse();
		if (!done_trig)
			trigdef_update_printf("%s %s\n", activate, opt_bypackage);
		trigdef_process_done();
	}

	return 0;
}

static int
do_check(const char *const *argv)
{
	enum trigdef_update_status uf;

	if (*argv)
		badusage(_("--%s takes no arguments"), "check-supported");

	uf = trigdef_update_start(TDUF_NO_LOCK_OK);
	switch (uf) {
	case TDUS_ERROR_NO_DIR:
		notice(_("triggers data directory not yet created"));
		return 1;
	case TDUS_ERROR_NO_DEFERRED:
		notice(_("trigger records not yet in existence"));
		return 1;
	case TDUS_OK:
	case TDUS_ERROR_EMPTY_DEFERRED:
		return 0;
	default:
		internerr("unknown trigdef_update_start return value '%d'", uf);
	}
}

static const struct cmdinfo cmdinfo_trigger =
	ACTION("trigger",          0,  0, do_trigger);

static const struct cmdinfo cmdinfos[] = {
	ACTION("check-supported",  0,  0, do_check),
	ACTION("help",            '?', 0, usage),
	ACTION("version",          0,  0, printversion),

	{ "admindir",        0,   1, NULL,       NULL,       set_admindir, 0 },
	{ "root",            0,   1, NULL,       NULL,       set_root, 0 },
	{ "by-package",      'f', 1, NULL,       &opt_bypackage },
	{ "await",           0,   0, &opt_await, NULL,       NULL, 1 },
	{ "no-await",        0,   0, &opt_await, NULL,       NULL, 0 },
	{ "no-act",          0,   0, &opt_act,   NULL,       NULL, 0 },
	{  NULL  }
};

int
main(int argc, const char *const *argv)
{
	int ret;

	dpkg_locales_init(PACKAGE);
	dpkg_program_init("dpkg-trigger");
	dpkg_options_parse(&argv, cmdinfos, printforhelp);

	debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

	if (!cipaction)
		setaction(&cmdinfo_trigger, NULL);

	ret = cipaction->action(argv);

	dpkg_program_done();
	dpkg_locales_done();

	return ret;
}
