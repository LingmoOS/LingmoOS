/*
 * libdpkg - Debian packaging suite library routines
 * t-fsys-hash.c - test fsys-hash implementation
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
#include <dpkg/fsys.h>

static void
test_fsys_nodes(void)
{
	struct fsys_namenode *fnn;
	struct fsys_hash_iter *iter;
	const char *name;

	test_pass(fsys_hash_entries() == 0);

	fsys_hash_init();

	fnn = fsys_hash_find_node("/nonexistent", FHFF_NO_NEW);
	test_pass(fnn == NULL);
	test_pass(fsys_hash_entries() == 0);

	name = "/test/path/aa";
	fnn = fsys_hash_find_node(name, FHFF_NO_COPY);
	test_pass(fnn != NULL);
	test_pass(fsys_hash_entries() == 1);
	test_pass(fnn->name == name);
	test_str(fnn->name, ==, "/test/path/aa");
	test_pass(fnn->flags == 0);
	test_pass(fnn->oldhash == NULL);
	test_pass(fnn->newhash == NULL);

	fnn = fsys_hash_find_node("//./test/path/bb", FHFF_NONE);
	test_pass(fnn != NULL);
	test_pass(fsys_hash_entries() == 2);
	test_str(fnn->name, ==, "/test/path/bb");
	test_pass(fnn->flags == 0);
	test_pass(fnn->oldhash == NULL);
	test_pass(fnn->newhash == NULL);

	fnn = fsys_hash_find_node("/test/path/cc", FHFF_NONE);
	test_pass(fnn != NULL);
	test_pass(fsys_hash_entries() == 3);
	test_str(fnn->name, ==, "/test/path/cc");
	test_pass(fnn->flags == 0);
	test_pass(fnn->oldhash == NULL);
	test_pass(fnn->newhash == NULL);

	iter = fsys_hash_iter_new();
	while ((fnn = fsys_hash_iter_next(iter))) {
		if (strcmp(fnn->name, "/test/path/aa") == 0)
			test_str(fnn->name, ==, "/test/path/aa");
		else if (strcmp(fnn->name, "/test/path/bb") == 0)
			test_str(fnn->name, ==, "/test/path/bb");
		else if (strcmp(fnn->name, "/test/path/cc") == 0)
			test_str(fnn->name, ==, "/test/path/cc");
		else
			test_fail("unknown fsys_namenode");
	}
	fsys_hash_iter_free(iter);

	fsys_hash_init();
	test_pass(fsys_hash_entries() == 3);
	fnn = fsys_hash_find_node("/test/path/aa", FHFF_NO_NEW);
	test_pass(fnn != NULL);
	fnn = fsys_hash_find_node("/test/path/bb", FHFF_NO_NEW);
	test_pass(fnn != NULL);
	fnn = fsys_hash_find_node("/test/path/cc", FHFF_NO_NEW);
	test_pass(fnn != NULL);
	test_pass(fsys_hash_entries() == 3);

	fsys_hash_reset();
	test_pass(fsys_hash_entries() == 0);
	fnn = fsys_hash_find_node("/test/path/aa", FHFF_NO_NEW);
	test_pass(fnn == NULL);
	fnn = fsys_hash_find_node("/test/path/bb", FHFF_NO_NEW);
	test_pass(fnn == NULL);
	fnn = fsys_hash_find_node("/test/path/cc", FHFF_NO_NEW);
	test_pass(fnn == NULL);
	test_pass(fsys_hash_entries() == 0);
}

TEST_ENTRY(test)
{
	test_plan(35);

	test_fsys_nodes();
}
