/*
 * dpkg-realpath - print the resolved pathname with DPKG_ROOT support
 *
 * Copyright © 2020 Helmut Grohne <helmut@subdivi.de>
 * Copyright © 2020-2024 Guillem Jover <guillem@debian.org>
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
#include <sys/stat.h>

#include <errno.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/string.h>
#include <dpkg/db-fsys.h>
#include <dpkg/options.h>

static const char printforhelp[] = N_(
"Use --help for help about this utility.");

static void DPKG_ATTR_NORET
printversion(const struct cmdinfo *cip, const char *value)
{
	printf(_("Debian %s version %s.\n"), dpkg_get_progname(),
	       PACKAGE_RELEASE);

	printf(_(
"This is free software; see the GNU General Public License version 2 or\n"
"later for copying conditions. There is NO warranty.\n"));

	m_output(stdout, _("<standard output>"));

	exit(0);
}

static void DPKG_ATTR_NORET
usage(const struct cmdinfo *cip, const char *value)
{
	printf(_(
"Usage: %s [<option>...] <pathname>\n"
"\n"), dpkg_get_progname());

	printf(_(
"Options:\n"
"  -z, --zero                   end output line with NUL, not newline.\n"
"      --instdir <directory>    set the root directory.\n"
"      --root <directory>       set the root directory.\n"
"      --version                show the version.\n"
"      --help                   show this help message.\n"
"\n"));

	m_output(stdout, _("<standard output>"));

	exit(0);
}

static int opt_eol = '\n';

static char *
realpath_relative_to(const char *pathname, const char *rootdir)
{
	struct varbuf root = VARBUF_INIT;
	struct varbuf src = VARBUF_INIT;
	struct varbuf dst = VARBUF_INIT;
	struct varbuf slink = VARBUF_INIT;
	struct varbuf result = VARBUF_INIT;
	struct varbuf prefix = VARBUF_INIT;
	int loop = 0;

	varbuf_init(&root, 32);
	varbuf_init(&src, 32);
	varbuf_init(&dst, 32);
	varbuf_init(&result, 32);

	varbuf_set_str(&root, rootdir);
	varbuf_set_str(&src, pathname);
	varbuf_set_str(&result, rootdir);

	/* Check whether the path is relative, make it absolute otherwise. */
	if (src.buf[0] != '/') {
		struct varbuf abs_src = VARBUF_INIT;

		file_getcwd(&abs_src);
		varbuf_add_char(&abs_src, '/');
		varbuf_add_varbuf(&abs_src, &src);

		varbuf_set_varbuf(&src, &abs_src);
		varbuf_destroy(&abs_src);

		varbuf_trim_varbuf_prefix(&src, &root);
	}

	/* Remove prefixed slashes. */
	varbuf_trim_char_prefix(&src, '/');

	while (src.used) {
		struct stat st;
		const char *slash;

		/* Get the first directory component. */
		varbuf_set_varbuf(&prefix, &src);
		slash = strchrnul(prefix.buf, '/');
		varbuf_trunc(&prefix, slash - prefix.buf);

		/* Remove the first directory component from src. */
		varbuf_trim_varbuf_prefix(&src, &prefix);

		/* Remove prefixed slashes. */
		varbuf_trim_char_prefix(&src, '/');

		varbuf_set_varbuf(&slink, &result);
		varbuf_add_char(&slink, '/');
		varbuf_add_varbuf(&slink, &prefix);

		/* Resolve the first directory component. */
		if (strcmp(prefix.buf, ".") == 0) {
			/* Ignore, stay at the same directory. */
		} else if (strcmp(prefix.buf, "..") == 0) {
			/* Go up one directory. */
			slash = strrchr(result.buf, '/');
			if (slash != NULL)
				varbuf_trunc(&result, slash - result.buf);

			if (root.used && !varbuf_has_prefix(&result, &root))
				varbuf_set_varbuf(&result, &root);
		} else if (lstat(slink.buf, &st) == 0 && S_ISLNK(st.st_mode)) {
			ssize_t linksize;

			loop++;
			if (loop > 25)
				ohshit(_("too many levels of symbolic links"));

			/* Resolve the symlink within result. */
			linksize = file_readlink(slink.buf, &dst, st.st_size);
			if (linksize < 0)
				ohshite(_("cannot read link '%s'"), slink.buf);
			else if ((off_t)linksize != st.st_size)
				ohshit(_("symbolic link '%s' size has changed from %jd to %zd"),
				       slink.buf, (intmax_t)st.st_size, linksize);

			if (dst.buf[0] == '/') {
				/* Absolute pathname, reset result back to
				 * root. */
				varbuf_set_varbuf(&result, &root);

				if (src.used) {
					varbuf_add_char(&dst, '/');
					varbuf_add_varbuf(&dst, &src);
				}
				varbuf_set_varbuf(&src, &dst);

				/* Remove prefixed slashes. */
				varbuf_trim_char_prefix(&src, '/');
			} else {
				/* Relative pathname. */
				if (src.used) {
					varbuf_add_char(&dst, '/');
					varbuf_add_varbuf(&dst, &src);
				}
				varbuf_set_varbuf(&src, &dst);
			}
		} else {
			/* Otherwise append the prefix. */
			varbuf_add_char(&result, '/');
			varbuf_add_varbuf(&result, &prefix);
		}
	}

	/* We are done, return the resolved pathname, w/o root. */
	varbuf_trim_varbuf_prefix(&result, &root);

	varbuf_destroy(&root);
	varbuf_destroy(&src);
	varbuf_destroy(&dst);
	varbuf_destroy(&slink);
	varbuf_destroy(&prefix);

	return varbuf_detach(&result);
}

static const struct cmdinfo cmdinfos[] = {
	{ "zero",       'z', 0,  &opt_eol,     NULL,      NULL,         '\0' },
	{ "instdir",    0,   1,  NULL,         NULL,      set_instdir,  0 },
	{ "root",       0,   1,  NULL,         NULL,      set_instdir,  0 },
	{ "version",    0,   0,  NULL,         NULL,      printversion  },
	{ "help",       '?', 0,  NULL,         NULL,      usage         },
	{  NULL,        0                                               }
};

int
main(int argc, const char *const *argv)
{
	const char *instdir;
	const char *pathname;
	char *real_pathname;

	dpkg_locales_init(PACKAGE);
	dpkg_program_init("dpkg-realpath");
	dpkg_options_parse(&argv, cmdinfos, printforhelp);

	debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

	pathname = argv[0];
	if (str_is_unset(pathname))
		badusage(_("need a pathname argument"));

	instdir = dpkg_fsys_get_dir();
	if (strlen(instdir) && strncmp(pathname, instdir, strlen(instdir)) == 0)
		badusage(_("link '%s' includes root prefix '%s'"),
		         pathname, instdir);

	real_pathname = realpath_relative_to(pathname, instdir);
	printf("%s%c", real_pathname, opt_eol);
	free(real_pathname);

	dpkg_program_done();
	dpkg_locales_done();

	return 0;
}
