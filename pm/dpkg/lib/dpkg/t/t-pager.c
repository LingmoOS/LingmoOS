/*
 * libdpkg - Debian packaging suite library routines
 * t-pager.c - test pager implementation
 *
 * Copyright © 2010-2012 Guillem Jover <guillem@debian.org>
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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/test.h>
#include <dpkg/pager.h>
#include <dpkg/dpkg.h>

static void
test_dup_file(int fd, const char *filename, int flags)
{
	int newfd;

	newfd = open(filename, flags);
	dup2(newfd, fd);
	close(newfd);
}

static void
test_pager_get_exec(void)
{
	const char *pager;
	int origfd = dup(STDOUT_FILENO);

	/* Test stdout being a tty. */
	test_todo_block("environment might not expose controlling terminal") {
		const char *default_pager;

		test_dup_file(STDOUT_FILENO, "/dev/tty", O_WRONLY);
		setenv("PAGER", "test-pager", 1);
		pager = pager_get_exec();
		unsetenv("PAGER");
		default_pager = pager_get_exec();
		dup2(origfd, STDOUT_FILENO);
		test_str(pager, ==, "test-pager");
		test_str(default_pager, ==, DPKG_DEFAULT_PAGER);
	}

	/* Test stdout not being a tty. */
	test_dup_file(STDOUT_FILENO, "/dev/null", O_WRONLY);
	pager = pager_get_exec();
	dup2(origfd, STDOUT_FILENO);
	test_str(pager, ==, CAT);
}

TEST_ENTRY(test)
{
	test_plan(3);

	test_pager_get_exec();
}
