/*
 * libdpkg - Debian packaging suite library routines
 * t-file.c - test file functions
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

#include <dpkg/test.h>
#include <dpkg/dpkg.h>
#include <dpkg/file.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static const char ref_data[] =
	"this is a test string\n"
	"within a test file\n"
	"containing multiple lines\n"
;

static void
test_file_getcwd(void)
{
	char *env;
	struct varbuf cwd = VARBUF_INIT;

	env = getenv("abs_builddir");
	file_getcwd(&cwd);

	test_str(env, ==, cwd.buf);

	varbuf_destroy(&cwd);
}

static void
test_file_realpath(void)
{
	char *path;

	path = file_realpath("/");
	test_str(path, ==, "/");
	free(path);

	path = file_realpath("///");
	test_str(path, ==, "/");
	free(path);

	path = file_realpath("//./..///../././..///..");
	test_str(path, ==, "/");
	free(path);
}

static void
test_file_canonicalize(void)
{
	char *path;

	/* Canonicalize filenames that exist. */
	path = file_canonicalize("/");
	test_str(path, ==, "/");
	free(path);

	path = file_canonicalize("///");
	test_str(path, ==, "/");
	free(path);

	path = file_canonicalize("//./..///../././..///..");
	test_str(path, ==, "/");
	free(path);

	/* Canonicalize filenames that do not exist. */
	path = file_canonicalize("/./nonexistent/..///./bar///../././../..///end//.");
	test_str(path, ==, "/end");
	free(path);

	path = file_canonicalize("foo/../bar/../quux////fox");
	test_str(path, ==, "quux/fox");
	free(path);

	path = file_canonicalize(".//aa/../bb///cc/./../dd//");
	test_str(path, ==, "bb/dd");
	free(path);

	path = file_canonicalize("..////../aa/../..///bb/../../cc/dd//");
	test_str(path, ==, "cc/dd");
	free(path);
}

static void
test_file_slurp(void)
{
	struct varbuf vb = VARBUF_INIT;
	struct dpkg_error err = DPKG_ERROR_INIT;
	char *test_file;
	char *test_dir;
	int fd;

	test_pass(file_slurp("/nonexistent", &vb, &err) < 0);
	test_pass(vb.used == 0);
	test_pass(vb.buf == NULL);
	test_pass(err.syserrno == ENOENT);
	test_error(err);
	varbuf_destroy(&vb);

	test_dir = test_alloc(strdup("test.XXXXXX"));
	test_pass(mkdtemp(test_dir) != NULL);
	test_pass(file_slurp(test_dir, &vb, &err) < 0);
	test_pass(vb.used == 0);
	test_pass(vb.buf == NULL);
	test_pass(err.syserrno == 0);
	test_error(err);
	varbuf_destroy(&vb);
	test_pass(rmdir(test_dir) == 0);

	test_file = test_alloc(strdup("test.XXXXXX"));
	fd = mkstemp(test_file);
	test_pass(fd >= 0);

	test_pass(file_slurp(test_file, &vb, &err) == 0);
	test_pass(vb.used == 0);
	test_pass(vb.buf == NULL);
	test_pass(err.syserrno == 0);
	test_pass(err.type == DPKG_MSG_NONE);
	varbuf_destroy(&vb);

	test_pass(write(fd, ref_data, strlen(ref_data)) == (ssize_t)strlen(ref_data));
	test_pass(lseek(fd, 0, SEEK_SET) == 0);

	test_pass(file_slurp(test_file, &vb, &err) == 0);
	test_pass(vb.used == strlen(ref_data));
	test_mem(vb.buf, ==, ref_data, min(vb.used, strlen(ref_data)));
	test_pass(err.syserrno == 0);
	test_pass(err.type == DPKG_MSG_NONE);
	varbuf_destroy(&vb);

	test_fail(file_is_exec(test_dir));
	test_fail(file_is_exec(test_file));
	test_pass(chmod(test_file, 0755) == 0);
	test_pass(file_is_exec(test_file));
	test_pass(chmod(test_file, 0750) == 0);
	test_pass(file_is_exec(test_file));

	test_pass(unlink(test_file) == 0);
	free(test_file);
	free(test_dir);
}

TEST_ENTRY(test)
{
	test_plan(43);

	test_file_getcwd();
	test_file_realpath();
	test_file_canonicalize();
	test_file_slurp();
}
