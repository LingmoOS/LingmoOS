/*
 * libdpkg - Debian packaging suite library routines
 * t-subproc.c - test sub-process module
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

#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <dpkg/test.h>
#include <dpkg/subproc.h>

static void
test_subproc_fork(void)
{
	struct sigaction sa;
	pid_t pid;
	int ret;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);

	/* Test exit(). */
	pid = subproc_fork();
	if (pid == 0)
		exit(0);
	ret = subproc_reap(pid, "subproc exit pass", SUBPROC_RETERROR);
	test_pass(ret == 0);

	pid = subproc_fork();
	if (pid == 0)
		exit(128);
	ret = subproc_reap(pid, "subproc exit fail", SUBPROC_RETERROR);
	test_pass(ret == 128);

	/* Test signals. */
	pid = subproc_fork();
	if (pid == 0)
		raise(SIGINT);
	ret = subproc_reap(pid, "subproc signal", SUBPROC_WARN);
	test_pass(ret < 0);

	pid = subproc_fork();
	if (pid == 0)
		raise(SIGTERM);
	ret = subproc_reap(pid, "subproc signal", SUBPROC_WARN);
	test_pass(ret < 0);

	pid = subproc_fork();
	if (pid == 0)
		raise(SIGPIPE);
	ret = subproc_reap(pid, "subproc SIGPIPE",
	                   SUBPROC_WARN | SUBPROC_NOPIPE);
	test_pass(ret == 0);

	pid = subproc_fork();
	if (pid == 0)
		raise(SIGPIPE);
	ret = subproc_reap(pid, "subproc SIGPIPE", SUBPROC_WARN);
	test_pass(ret < 0);
}

TEST_ENTRY(test)
{
	int fd;

	test_plan(6);

	/* XXX: Shut up stderr, we don't want the error output. */
	fd = open("/dev/null", O_RDWR);
	if (fd < 0)
		test_bail("cannot open /dev/null");
	dup2(fd, 2);

	test_subproc_fork();
}
