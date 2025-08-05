/*
 * libdpkg - Debian packaging suite library routines
 * t-progname.c - test program name handling
 *
 * Copyright © 2011 Guillem Jover <guillem@debian.org>
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

#include <dpkg/test.h>
#include <dpkg/progname.h>

static void
test_progname(void)
{
	const char *progname;

	/* Test initially empty progname. */
	progname = dpkg_get_progname();
	/* Handle libtool executables. */
	if (strncmp(progname, "lt-", 3) == 0)
		progname += 3;
	test_str(progname, ==, "t-progname");

	/* Test setting a new progname. */
	dpkg_set_progname("newname");
	test_str(dpkg_get_progname(), ==, "newname");

	/* Test setting a new progname with path. */
	dpkg_set_progname("path/newprogname");
	test_str(dpkg_get_progname(), ==, "newprogname");
}

TEST_ENTRY(test)
{
	test_plan(3);

	test_progname();
}
