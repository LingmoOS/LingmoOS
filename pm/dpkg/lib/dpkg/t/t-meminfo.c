/*
 * libdpkg - Debian packaging suite library routines
 * t-meminfo.c - test memory information handling code
 *
 * Copyright © 2022 Guillem Jover <guillem@debian.org>
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
#include <dpkg/meminfo.h>

static void
test_meminfo(void)
{
	char *pathname;
	uint64_t mem;
	int rc;

	mem = 0;
	pathname = test_data_file("meminfo-no-file");
	rc = meminfo_get_available_from_file(pathname, &mem);
	test_pass(rc == MEMINFO_NO_FILE);
	test_pass(mem == 0);
	free(pathname);

	mem = 0;
	pathname = test_data_file("meminfo-no-data");
	rc = meminfo_get_available_from_file(pathname, &mem);
	test_pass(rc == MEMINFO_NO_DATA);
	test_pass(mem == 0);
	free(pathname);

	mem = 0;
	pathname = test_data_file("meminfo-no-unit");
	rc = meminfo_get_available_from_file(pathname, &mem);
	test_pass(rc == MEMINFO_NO_UNIT);
	test_pass(mem == 0);
	free(pathname);

	mem = 0;
	pathname = test_data_file("meminfo-no-info");
	rc = meminfo_get_available_from_file(pathname, &mem);
	test_pass(rc == MEMINFO_NO_INFO);
	test_pass(mem == 0);
	free(pathname);

	mem = 0;
	pathname = test_data_file("meminfo-ok");
	rc = meminfo_get_available_from_file(pathname, &mem);
	test_pass(rc == MEMINFO_OK);
	test_pass(mem == 3919974400UL);
	free(pathname);
}

TEST_ENTRY(test)
{
	test_plan(10);

	test_meminfo();
}
