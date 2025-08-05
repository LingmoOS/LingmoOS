/*
 * libdpkg - Debian packaging suite library routines
 * t-tar.c - test tar implementation
 *
 * Copyright © 2017 Guillem Jover <guillem@debian.org>
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

#include <errno.h>

#include <dpkg/test.h>
#include <dpkg/tarfn.h>

static void
test_tar_atol8(void)
{
	uintmax_t u;

	/* Test valid octal numbers. */
	u = tar_atoul("000000\0\0\0\0\0\0", 12, UINTMAX_MAX);
	test_pass(u == 0);
	u = tar_atoul("00000000000\0", 12, UINTMAX_MAX);
	test_pass(u == 0);
	u = tar_atoul("00000000001\0", 12, UINTMAX_MAX);
	test_pass(u == 1);
	u = tar_atoul("00000000777\0", 12, UINTMAX_MAX);
	test_pass(u == 511);
	u = tar_atoul("77777777777\0", 12, UINTMAX_MAX);
	test_pass(u == 8589934591);

	/* Test legacy formatted octal numbers. */
	u = tar_atoul("          0\0", 12, UINTMAX_MAX);
	test_pass(u == 0);
	u = tar_atoul("          1\0", 12, UINTMAX_MAX);
	test_pass(u == 1);
	u = tar_atoul("        777\0", 12, UINTMAX_MAX);
	test_pass(u == 511);

	/* Test extended octal numbers not terminated by space or NUL,
	 * (as is required by POSIX), but accepted by several implementations
	 * to get one byte larger values. */
	u = tar_atoul("000000000000", 12, UINTMAX_MAX);
	test_pass(u == 0);
	u = tar_atoul("000000000001", 12, UINTMAX_MAX);
	test_pass(u == 1);
	u = tar_atoul("000000000777", 12, UINTMAX_MAX);
	test_pass(u == 511);
	u = tar_atoul("777777777777", 12, UINTMAX_MAX);
	test_pass(u == 68719476735);

	/* Test invalid octal numbers. */
	errno = 0;
	u = tar_atoul("            ", 12, UINTMAX_MAX);
	test_pass(u == 0);
	test_pass(errno == EINVAL);

	errno = 0;
	u = tar_atoul("   11111aaa ", 12, UINTMAX_MAX);
	test_pass(u == 0);
	test_pass(errno == ERANGE);

	errno = 0;
	u = tar_atoul("          8 ", 12, UINTMAX_MAX);
	test_pass(u == 0);
	test_pass(errno == ERANGE);

	errno = 0;
	u = tar_atoul("         18 ", 12, UINTMAX_MAX);
	test_pass(u == 0);
	test_pass(errno == ERANGE);

	errno = 0;
	u = tar_atoul("    aa      ", 12, UINTMAX_MAX);
	test_pass(u == 0);
	test_pass(errno == ERANGE);
}

static void
test_tar_atol256(void)
{
	uintmax_t u;
	intmax_t i;

	/* Test positive numbers. */
	u = tar_atoul("\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, UINTMAX_MAX);
	test_pass(u == 0);
	u = tar_atoul("\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 12, UINTMAX_MAX);
	test_pass(u == 1);
	u = tar_atoul("\x80\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00", 12, UINTMAX_MAX);
	test_pass(u == 8589934592);
	u = tar_atoul("\x80\x00\x00\x00\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 12, UINTMAX_MAX);
	test_pass(u == INTMAX_MAX);

	/* Test overflow. */
	errno = 0;
	u = tar_atoul("\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00", 12, UINTMAX_MAX);
	test_pass(u == UINTMAX_MAX);
	test_pass(errno == ERANGE);

	errno = 0;
	u = tar_atoul("\x80\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, UINTMAX_MAX);
	test_pass(u == UINTMAX_MAX);
	test_pass(errno == ERANGE);

	/* Test negative numbers. */
	i = tar_atosl("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == -1);
	i = tar_atosl("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == -2);
	i = tar_atosl("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\x00\x00\x00\x00", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == -8589934592);
	i = tar_atosl("\xFF\xFF\xFF\xFF\x80\x00\x00\x00\x00\x00\x00\x00", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == INTMAX_MIN);

	/* Test underflow. */
	errno = 0;
	i = tar_atosl("\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == INTMAX_MIN);
	test_pass(errno == ERANGE);

	errno = 0;
	i = tar_atosl("\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, INTMAX_MIN, INTMAX_MAX);
	test_pass(i == INTMAX_MIN);
	test_pass(errno == ERANGE);
}

TEST_ENTRY(test)
{
	test_plan(38);

	test_tar_atol8();
	test_tar_atol256();
}
