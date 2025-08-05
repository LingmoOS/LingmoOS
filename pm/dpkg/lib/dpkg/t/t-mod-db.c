/*
 * libdpkg - Debian packaging suite library routines
 * t-mod-db.c - test database implementation
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

#include <stdlib.h>

#include <dpkg/test.h>
#include <dpkg/dpkg-db.h>

static void
test_db_dir(void)
{
	char *dir;

	test_str(dpkg_db_get_dir(), ==, ADMINDIR);

	dpkg_db_set_dir("testdir");
	test_str(dpkg_db_get_dir(), ==, "testdir");

	setenv("DPKG_ADMINDIR", "testenvdir", 1);
	dpkg_db_set_dir(NULL);
	test_str(dpkg_db_get_dir(), ==, "testenvdir");

	unsetenv("DPKG_ADMINDIR");
	dpkg_db_set_dir(NULL);
	test_str(dpkg_db_get_dir(), ==, ADMINDIR);

	dir = dpkg_db_get_path("testfile");
	test_str(dir, ==, ADMINDIR "/testfile");
	free(dir);
}

TEST_ENTRY(test)
{
	test_plan(5);

	test_db_dir();
}
