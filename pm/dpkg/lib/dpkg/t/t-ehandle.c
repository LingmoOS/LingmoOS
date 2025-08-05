/*
 * libdpkg - Debian packaging suite library routines
 * t-ehandle.c - test error handling implementation
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

#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include <dpkg/test.h>
#include <dpkg/ehandle.h>

static jmp_buf global_handler_jump;

static void
printer_empty(const char *msg, const void *data)
{
}

static void
handler_func(void)
{
	longjmp(global_handler_jump, 1);
}

static void
test_error_handler_func(void)
{
	bool pass;

	if (setjmp(global_handler_jump)) {
		pass = true;
		pop_error_context(ehflag_normaltidy);
	} else {
		pass = false;
		push_error_context_func(handler_func, printer_empty, "test func");
		ohshit("test func error");
		test_bail("ohshit() is not supposed to return");
	}
	test_pass(pass);
}

static void
test_error_handler_jump(void)
{
	jmp_buf handler_jump;
	bool pass;

	if (setjmp(handler_jump)) {
		pass = true;
		pop_error_context(ehflag_normaltidy);
	} else {
		pass = false;
		push_error_context_jump(&handler_jump, printer_empty, "test jump");
		ohshit("test jump error");
		test_bail("ohshit() is not supposed to return");
	}
	test_pass(pass);
}

static void
cleanup_error(int argc, void **argv)
{
	ohshit("cleanup error");
}

static void
test_cleanup_error(void)
{
	jmp_buf handler_jump;
	bool pass;

	if (setjmp(handler_jump)) {
		/* The ohshit() is not supposed to get us here, as it should
		 * be caught by the internal recursive error context. */
		pass = false;

		pop_cleanup(ehflag_normaltidy);
		pop_error_context(ehflag_normaltidy);
	} else {
		push_error_context_jump(&handler_jump, printer_empty, "test cleanup");
		push_cleanup(cleanup_error, ~ehflag_normaltidy, 0);
		pop_error_context(ehflag_bombout);

		/* We should have recovered from the cleanup handler failing,
		 * and arrived here correctly. */
		pass = true;
	}

	test_pass(pass);
}

TEST_ENTRY(test)
{
	test_plan(3);

	if (!test_is_verbose()) {
		int fd;

		/* Shut up stderr, we do not want the error output. */
		fd = open("/dev/null", O_RDWR);
		if (fd < 0)
			test_bail("cannot open /dev/null");
		dup2(fd, 2);
	}

	test_error_handler_func();
	test_error_handler_jump();
	test_cleanup_error();
}
