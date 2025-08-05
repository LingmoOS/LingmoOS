/*
 * libdpkg - Debian packaging suite library routines
 * t-fsys-dir.c - test filesystem handling
 *
 * Copyright © 2018 Guillem Jover <guillem@debian.org>
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
#include <dpkg/fsys.h>

static void
test_fsys_dir(void)
{
	const char *newdir;
	char *dir;

	test_str(dpkg_fsys_get_dir(), ==, "");

	newdir = dpkg_fsys_set_dir("/testdir//./");
	test_str(newdir, ==, "/testdir");
	test_str(dpkg_fsys_get_dir(), ==, "/testdir");

	newdir = dpkg_fsys_set_dir("/testdir");
	test_str(newdir, ==, "/testdir");
	test_str(dpkg_fsys_get_dir(), ==, "/testdir");

	newdir = dpkg_fsys_set_dir(newdir);
	test_str(newdir, ==, "/testdir");
	test_str(dpkg_fsys_get_dir(), ==, "/testdir");

	dir = dpkg_fsys_get_path("testfile");
	test_str(dir, ==, "/testdir/testfile");
	free(dir);

	dir = dpkg_fsys_get_path("/testfile");
	test_str(dir, ==, "/testdir/testfile");
	free(dir);

	setenv("DPKG_ROOT", "/testenvdir//./", 1);
	dpkg_fsys_set_dir(NULL);
	test_str(dpkg_fsys_get_dir(), ==, "/testenvdir");

	setenv("DPKG_ROOT", "/testenvdir", 1);
	dpkg_fsys_set_dir(NULL);
	test_str(dpkg_fsys_get_dir(), ==, "/testenvdir");

	dir = dpkg_fsys_get_path("testfile");
	test_str(dir, ==, "/testenvdir/testfile");
	free(dir);

	dir = dpkg_fsys_get_path("/testfile");
	test_str(dir, ==, "/testenvdir/testfile");
	free(dir);

	unsetenv("DPKG_ROOT");
	dpkg_fsys_set_dir(NULL);
	test_str(dpkg_fsys_get_dir(), ==, "");

	dir = dpkg_fsys_get_path("testfile");
	test_str(dir, ==, "/testfile");
	free(dir);

	dir = dpkg_fsys_get_path("/testfile");
	test_str(dir, ==, "/testfile");
	free(dir);
}

TEST_ENTRY(test)
{
	test_plan(16);

	test_fsys_dir();
}
