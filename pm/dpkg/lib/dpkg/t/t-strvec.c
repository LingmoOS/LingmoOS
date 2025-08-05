/*
 * libdpkg - Debian packaging suite library routines
 * t-strvec.c - test string vector handling
 *
 * Copyright © 2024 Guillem Jover <guillem@debian.org>
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
#include <dpkg/strvec.h>

static void
test_strvec_stack(void)
{
	struct strvec *sv;
	char *str;

	sv = strvec_new(0);
	strvec_push(sv, m_strdup("aa"));
	strvec_push(sv, m_strdup("bb"));
	strvec_push(sv, m_strdup("cc"));
	test_pass(sv->size >= 3);
	test_pass(sv->used == 3);
	test_str(sv->vec[0], ==, "aa");
	test_str(sv->vec[1], ==, "bb");
	test_str(sv->vec[2], ==, "cc");
	strvec_free(sv);

	sv = strvec_new(0);
	strvec_push(sv, m_strdup("ini"));
	strvec_push(sv, m_strdup("tmp"));
	str = strvec_pop(sv);
	strvec_push(sv, m_strdup("end"));
	test_pass(sv->size >= 2);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "ini");
	test_str(sv->vec[1], ==, "end");
	test_str(str, ==, "tmp");
	free(str);
	strvec_free(sv);

	sv = strvec_new(0);
	strvec_push(sv, m_strdup("aa"));
	test_str(strvec_peek(sv), ==, "aa");
	strvec_push(sv, m_strdup("bb"));
	test_str(strvec_peek(sv), ==, "bb");
	strvec_push(sv, m_strdup("cc"));
	test_str(strvec_peek(sv), ==, "cc");
	str = strvec_pop(sv);
	test_str(strvec_peek(sv), ==, "bb");
	test_pass(sv->size >= 2);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "aa");
	test_str(sv->vec[1], ==, "bb");
	test_str(str, ==, "cc");
	free(str);
	strvec_free(sv);
}

static void
test_strvec_parts(void)
{
	struct strvec *sv;
	char *str;

	sv = strvec_split("", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 1);
	test_str(sv->vec[0], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "");
	free(str);
	strvec_free(sv);

	sv = strvec_split("one", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 1);
	test_str(sv->vec[0], ==, "one");
	str = strvec_join(sv, '/');
	test_str(str, ==, "one");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":end", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "end");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/end");
	free(str);
	strvec_free(sv);

	sv = strvec_split("ini:", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "ini");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "ini/");
	free(str);
	strvec_free(sv);

	sv = strvec_split("foo:bar:quux:foo::end:", ':', STRVEC_SPLIT_DEFAULT);
	test_pass(sv != NULL);
	test_pass(sv->used == 7);
	test_str(sv->vec[0], ==, "foo");
	test_str(sv->vec[1], ==, "bar");
	test_str(sv->vec[2], ==, "quux");
	test_str(sv->vec[3], ==, "foo");
	test_str(sv->vec[4], ==, "");
	test_str(sv->vec[5], ==, "end");
	test_str(sv->vec[6], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "foo/bar/quux/foo//end/");
	free(str);
	strvec_free(sv);
}

static void
test_strvec_parts_skip_dup_sep(void)
{
	struct strvec *sv;
	char *str;

	sv = strvec_split("", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 1);
	test_str(sv->vec[0], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "");
	free(str);
	strvec_free(sv);

	sv = strvec_split("one", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 1);
	test_str(sv->vec[0], ==, "one");
	str = strvec_join(sv, '/');
	test_str(str, ==, "one");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":::", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":end", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "end");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/end");
	free(str);
	strvec_free(sv);

	sv = strvec_split(":::end", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "");
	test_str(sv->vec[1], ==, "end");
	str = strvec_join(sv, '/');
	test_str(str, ==, "/end");
	free(str);
	strvec_free(sv);

	sv = strvec_split("ini:", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "ini");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "ini/");
	free(str);
	strvec_free(sv);

	sv = strvec_split("ini:::", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 2);
	test_str(sv->vec[0], ==, "ini");
	test_str(sv->vec[1], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "ini/");
	free(str);
	strvec_free(sv);

	sv = strvec_split("foo:bar::quux:foo::::end::::", ':', STRVEC_SPLIT_SKIP_DUP_SEP);
	test_pass(sv != NULL);
	test_pass(sv->used == 6);
	test_str(sv->vec[0], ==, "foo");
	test_str(sv->vec[1], ==, "bar");
	test_str(sv->vec[2], ==, "quux");
	test_str(sv->vec[3], ==, "foo");
	test_str(sv->vec[4], ==, "end");
	test_str(sv->vec[5], ==, "");
	str = strvec_join(sv, '/');
	test_str(str, ==, "foo/bar/quux/foo/end/");
	free(str);
	strvec_free(sv);
}

TEST_ENTRY(test)
{
	test_plan(99);

	test_strvec_stack();
	test_strvec_parts();
	test_strvec_parts_skip_dup_sep();
}
