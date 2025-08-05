/*
 * libdpkg - Debian packaging suite library routines
 * c-trigdeferred.c - test triggered deferred file parser
 *
 * Copyright © 2016 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/dpkg-db.h>
#include <dpkg/ehandle.h>
#include <dpkg/trigdeferred.h>

static void
test_tdm_begin(const char *trig)
{
	printf("<T='%s'", trig);
}

static void
test_tdm_package(const char *awname)
{
	printf(" P='%s'", awname);
}

static void
test_tdm_end(void)
{
	printf(" E>\n");
}

static const struct trigdefmeths test_tdm = {
	.trig_begin = test_tdm_begin,
	.package = test_tdm_package,
	.trig_end = test_tdm_end,
};

static int
test_trigdeferred_parser(const char *admindir)
{
	enum trigdef_update_flags tduf;
	enum trigdef_update_status tdus;

	dpkg_db_set_dir(admindir);

	trigdef_set_methods(&test_tdm);

	tduf = TDUF_NO_LOCK | TDUF_WRITE_IF_EMPTY | TDUF_WRITE_IF_ENOENT;
	tdus = trigdef_update_start(tduf);
	if (tdus < TDUS_OK)
		return -tdus + TDUS_OK;

	trigdef_parse();

	trigdef_process_done();

	return 0;
}

int
main(int argc, char **argv)
{
	const char *admindir = argv[1];
	int ret;

	setvbuf(stdout, NULL, _IOLBF, 0);

	push_error_context();

	if (admindir == NULL)
		ohshit("missing triggers deferred admindir");

	ret = test_trigdeferred_parser(admindir);

	pop_error_context(ehflag_normaltidy);

	return ret;
}
