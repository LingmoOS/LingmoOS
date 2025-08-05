/*
 * libdpkg - Debian packaging suite library routines
 * t-buffer.c - test buffer handling
 *
 * Copyright © 2009-2011 Guillem Jover <guillem@debian.org>
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
#include <dpkg/buffer.h>
#include <dpkg/dpkg.h>

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static const char str_empty[] = "";
static const char ref_hash_empty[] = "d41d8cd98f00b204e9800998ecf8427e";
static const char str_test[] = "this is a test string\n";
static const char ref_hash_test[] = "475aae3b885d70a9130eec23ab33f2b9";

static void
test_buffer_hash(void)
{
	char hash[MD5HASHLEN + 1];

	buffer_md5(str_empty, hash, strlen(str_empty));
	test_str(hash, ==, ref_hash_empty);

	buffer_md5(str_test, hash, strlen(str_test));
	test_str(hash, ==, ref_hash_test);
}

static void
test_fdio_hash(void)
{
	char hash[MD5HASHLEN + 1];
	char *test_file;
	int fd;

	test_file = test_alloc(strdup("test.XXXXXX"));
	fd = mkstemp(test_file);
	test_pass(fd >= 0);

	test_pass(fd_md5(fd, hash, -1, NULL) >= 0);
	test_str(hash, ==, ref_hash_empty);

	test_pass(write(fd, str_test, strlen(str_test)) == (ssize_t)strlen(str_test));
	test_pass(lseek(fd, 0, SEEK_SET) == 0);

	test_pass(fd_md5(fd, hash, -1, NULL) >= 0);
	test_str(hash, ==, ref_hash_test);

	test_pass(unlink(test_file) == 0);

	free(test_file);
}

TEST_ENTRY(test)
{
	test_plan(10);

	test_buffer_hash();
	test_fdio_hash();
}
