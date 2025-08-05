/*
 * libdpkg - Debian packaging suite library routines
 * t-version.c - test version handling
 *
 * Copyright © 2009-2014 Guillem Jover <guillem@debian.org>
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
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

static void
test_version_blank(void)
{
	struct dpkg_version a;

	dpkg_version_blank(&a);
	test_pass(a.epoch == 0);
	test_pass(a.version == NULL);
	test_pass(a.revision == NULL);
}

static void
test_version_is_informative(void)
{
	struct dpkg_version a;

	dpkg_version_blank(&a);
	test_fail(dpkg_version_is_informative(&a));

	a.epoch = 1;
	test_pass(dpkg_version_is_informative(&a));

	dpkg_version_blank(&a);
	a.version = "1";
	test_pass(dpkg_version_is_informative(&a));

	dpkg_version_blank(&a);
	a.revision = "1";
	test_pass(dpkg_version_is_informative(&a));
}

static void
test_version_compare(void)
{
	struct dpkg_version a, b;

	dpkg_version_blank(&a);
	dpkg_version_blank(&b);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	a.epoch = 1;
	b.epoch = 2;
	test_fail(dpkg_version_compare(&a, &b) == 0);

	a = DPKG_VERSION_OBJECT(0, "1", "1");
	b = DPKG_VERSION_OBJECT(0, "2", "1");
	test_fail(dpkg_version_compare(&a, &b) == 0);

	a = DPKG_VERSION_OBJECT(0, "1", "1");
	b = DPKG_VERSION_OBJECT(0, "1", "2");
	test_fail(dpkg_version_compare(&a, &b) == 0);

	/* Test for version equality. */
	a = b = DPKG_VERSION_OBJECT(0, "0", "0");
	test_pass(dpkg_version_compare(&a, &b) == 0);

	a = DPKG_VERSION_OBJECT(0, "0", "00");
	b = DPKG_VERSION_OBJECT(0, "00", "0");
	test_pass(dpkg_version_compare(&a, &b) == 0);

	a = b = DPKG_VERSION_OBJECT(1, "2", "3");
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test for epoch difference. */
	a = DPKG_VERSION_OBJECT(0, "0", "0");
	b = DPKG_VERSION_OBJECT(1, "0", "0");
	test_pass(dpkg_version_compare(&a, &b) < 0);
	test_pass(dpkg_version_compare(&b, &a) > 0);

	/* Test for version component difference. */
	a = DPKG_VERSION_OBJECT(0, "a", "0");
	b = DPKG_VERSION_OBJECT(0, "b", "0");
	test_pass(dpkg_version_compare(&a, &b) < 0);
	test_pass(dpkg_version_compare(&b, &a) > 0);

	/* Test for revision component difference. */
	a = DPKG_VERSION_OBJECT(0, "0", "a");
	b = DPKG_VERSION_OBJECT(0, "0", "b");
	test_pass(dpkg_version_compare(&a, &b) < 0);
	test_pass(dpkg_version_compare(&b, &a) > 0);

	/* TODO: Complete. */
}

static void
test_version_relate(void)
{
	struct dpkg_version a, b;

	dpkg_version_blank(&a);
	dpkg_version_blank(&b);
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_NONE, &b));

	a = DPKG_VERSION_OBJECT(0, "1", "1");
	b = DPKG_VERSION_OBJECT(0, "1", "1");
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_EQ, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_LT, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_LE, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_GT, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_GE, &b));

	a = DPKG_VERSION_OBJECT(0, "1", "1");
	b = DPKG_VERSION_OBJECT(0, "2", "1");
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_EQ, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_LT, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_LE, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_GT, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_GE, &b));

	a = DPKG_VERSION_OBJECT(0, "2", "1");
	b = DPKG_VERSION_OBJECT(0, "1", "1");
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_EQ, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_LT, &b));
	test_fail(dpkg_version_relate(&a, DPKG_RELATION_LE, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_GT, &b));
	test_pass(dpkg_version_relate(&a, DPKG_RELATION_GE, &b));
}

static void
test_version_parse(void)
{
	struct dpkg_error err;
	struct dpkg_version a, b;
	const char *p;
	char *verstr;

	/* Test 0 versions. */
	dpkg_version_blank(&a);
	b = DPKG_VERSION_OBJECT(0, "0", "");

	test_pass(parseversion(&a, "0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	test_pass(parseversion(&a, "0:0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(0, "0", "0");
	test_pass(parseversion(&a, "0:0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(0, "0.0", "0.0");
	test_pass(parseversion(&a, "0:0.0-0.0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test epoched versions. */
	b = DPKG_VERSION_OBJECT(1, "0", "");
	test_pass(parseversion(&a, "1:0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(5, "1", "");
	test_pass(parseversion(&a, "5:1", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test multiple hyphens. */
	b = DPKG_VERSION_OBJECT(0, "0-0", "0");
	test_pass(parseversion(&a, "0:0-0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(0, "0-0-0", "0");
	test_pass(parseversion(&a, "0:0-0-0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test multiple colons. */
	b = DPKG_VERSION_OBJECT(0, "0:0", "0");
	test_pass(parseversion(&a, "0:0:0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(0, "0:0:0", "0");
	test_pass(parseversion(&a, "0:0:0:0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test multiple hyphens and colons. */
	b = DPKG_VERSION_OBJECT(0, "0:0-0", "0");
	test_pass(parseversion(&a, "0:0:0-0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	b = DPKG_VERSION_OBJECT(0, "0-0:0", "0");
	test_pass(parseversion(&a, "0:0-0:0-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test valid characters in upstream version. */
	b = DPKG_VERSION_OBJECT(0, "09azAZ.-+~:", "0");
	test_pass(parseversion(&a, "0:09azAZ.-+~:-0", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test valid characters in revision. */
	b = DPKG_VERSION_OBJECT(0, "0", "azAZ09.+~");
	test_pass(parseversion(&a, "0:0-azAZ09.+~", NULL) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test version with leading and trailing spaces. */
	b = DPKG_VERSION_OBJECT(0, "0", "1");
	test_pass(parseversion(&a, "  	0:0-1", &err) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);
	test_pass(parseversion(&a, "0:0-1	  ", &err) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);
	test_pass(parseversion(&a, "	  0:0-1  	", &err) == 0);
	test_pass(dpkg_version_compare(&a, &b) == 0);

	/* Test empty version. */
	test_pass(parseversion(&a, "", &err) != 0);
	test_error(err);
	test_pass(parseversion(&a, "  ", &err) != 0);
	test_error(err);

	/* Test empty upstream version after epoch. */
	test_fail(parseversion(&a, "0:", &err) == 0);
	test_error(err);

	/* Test empty epoch in version. */
	test_fail(parseversion(&a, ":1.0", &err) == 0);
	test_error(err);

	/* Test empty revision in version. */
	test_fail(parseversion(&a, "1.0-", &err) == 0);
	test_error(err);

	/* Test version with embedded spaces. */
	test_fail(parseversion(&a, "0:0 0-1", &err) == 0);
	test_error(err);

	/* Test version with negative epoch. */
	test_fail(parseversion(&a, "-1:0-1", &err) == 0);
	test_error(err);

	/* Test version with huge epoch. */
	test_fail(parseversion(&a, "999999999999999999999999:0-1", &err) == 0);
	test_error(err);

	/* Test invalid characters in epoch. */
	test_fail(parseversion(&a, "a:0-0", &err) == 0);
	test_error(err);
	test_fail(parseversion(&a, "A:0-0", &err) == 0);
	test_error(err);

	/* Test invalid empty upstream version. */
	test_fail(parseversion(&a, "-0", &err) == 0);
	test_error(err);
	test_fail(parseversion(&a, "0:-0", &err) == 0);
	test_error(err);

	/* Test upstream version not starting with a digit */
	test_fail(parseversion(&a, "0:abc3-0", &err) == 0);
	test_warn(err);

	/* Test invalid characters in upstream version. */
	verstr = test_alloc(strdup("0:0a-0"));
	for (p = "!#@$%&/|\\<>()[]{};,_=*^'"; *p; p++) {
		verstr[3] = *p;
		test_fail(parseversion(&a, verstr, &err) == 0);
		test_warn(err);
	}
	free(verstr);

	/* Test invalid characters in revision. */
	test_fail(parseversion(&a, "0:0-0:0", &err) == 0);
	test_warn(err);

	verstr = test_alloc(strdup("0:0-0"));
	for (p = "!#@$%&/|\\<>()[]{}:;,_=*^'"; *p; p++) {
		verstr[4] = *p;
		test_fail(parseversion(&a, verstr, &err) == 0);
		test_warn(err);
	}
	free(verstr);

	/* TODO: Complete. */
}

TEST_ENTRY(test)
{
	test_plan(196);

	test_version_blank();
	test_version_is_informative();
	test_version_compare();
	test_version_relate();
	test_version_parse();
}
