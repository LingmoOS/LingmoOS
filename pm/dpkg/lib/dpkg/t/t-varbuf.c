/*
 * libdpkg - Debian packaging suite library routines
 * t-varbuf.c - test varbuf implementation
 *
 * Copyright © 2009-2011, 2013-2015 Guillem Jover <guillem@debian.org>
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
#include <dpkg/varbuf.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static void
test_varbuf_init(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 0);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);

	varbuf_destroy(&vb);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
}

static void
test_varbuf_prealloc(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 10);
	test_pass(vb.used == 0);
	test_pass(vb.size >= 10);
	test_pass(vb.buf != NULL);

	varbuf_destroy(&vb);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
}

static void
test_varbuf_new(void)
{
	struct varbuf *vb;

	vb = varbuf_new(0);
	test_pass(vb != NULL);
	test_pass(vb->used == 0);
	test_pass(vb->size == 0);
	test_pass(vb->buf == NULL);
	varbuf_free(vb);

	vb = varbuf_new(10);
	test_pass(vb != NULL);
	test_pass(vb->used == 0);
	test_pass(vb->size >= 10);
	test_pass(vb->buf != NULL);
	varbuf_free(vb);
}

static void
test_varbuf_grow(void)
{
	struct varbuf vb;
	jmp_buf grow_jump;
	volatile size_t old_size;
	volatile bool grow_overflow;
	int i;

	varbuf_init(&vb, 10);

	/* Test that we grow when needed. */
	varbuf_grow(&vb, 100);
	test_pass(vb.used == 0);
	test_pass(vb.size >= 100);

	old_size = vb.size;

	/* Test that we are not leaking. */
	for (i = 0; i < 10; i++) {
		varbuf_grow(&vb, 100);
		test_pass(vb.used == 0);
		test_pass(vb.size >= 100);
		test_pass(vb.size == old_size);
	}

	/* Test that we grow when needed, with used space. */
	vb.used = 10;
	varbuf_grow(&vb, 100);
	test_pass(vb.used == 10);
	test_pass(vb.size >= 110);

	/* Test that we do not allow allocation overflows. */
	grow_overflow = false;
	old_size = vb.size;
	test_try(grow_jump) {
		varbuf_grow(&vb, SIZE_MAX - vb.size + 2);
	} test_catch {
		grow_overflow = true;
	} test_finally;
	test_pass(vb.size == old_size && grow_overflow);

	grow_overflow = false;
	old_size = vb.size;
	test_try(grow_jump) {
		varbuf_grow(&vb, (SIZE_MAX - vb.size - 2) / 2);
	} test_catch {
		grow_overflow = true;
	} test_finally;
	test_pass(vb.size == old_size && grow_overflow);

	varbuf_destroy(&vb);
}

static void
test_varbuf_trunc(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 50);

	/* Test that we truncate (grow). */
	varbuf_trunc(&vb, 20);
	test_pass(vb.used == 20);
	test_pass(vb.size >= 50);

	/* Test that we truncate (shrink). */
	varbuf_trunc(&vb, 10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= 50);

	varbuf_destroy(&vb);
}

static void
test_varbuf_set(void)
{
	struct varbuf vb, cb;

	varbuf_init(&vb, 10);
	varbuf_init(&cb, 10);

	varbuf_set_buf(&vb, "1234567890", 5);
	test_pass(vb.used == 5);
	test_mem(vb.buf, ==, "12345", 5);

	varbuf_set_buf(&vb, "abcd", 4);
	test_pass(vb.used == 4);
	test_mem(vb.buf, ==, "abcd", 4);

	varbuf_set_varbuf(&cb, &vb);
	test_pass(cb.used == 4);
	test_mem(cb.buf, ==, "abcd", 4);

	varbuf_set_str(&vb, "12345");
	test_pass(vb.used == 5);
	test_str(vb.buf, ==, "12345");

	varbuf_set_strn(&vb, "1234567890", 8);
	test_pass(vb.used == 8);
	test_str(vb.buf, ==, "12345678");

	varbuf_destroy(&cb);
	varbuf_destroy(&vb);
}

static void
test_varbuf_add_varbuf(void)
{
	struct varbuf vb, cb;

	varbuf_init(&vb, 5);
	varbuf_init(&cb, 0);

	varbuf_set_str(&vb, "1234567890");
	varbuf_add_varbuf(&cb, &vb);
	test_pass(cb.used == 10);
	test_pass(cb.size >= cb.used);
	test_mem(cb.buf, ==, "1234567890", 10);

	varbuf_set_str(&vb, "abcde");
	varbuf_add_varbuf(&cb, &vb);
	test_pass(cb.used == 15);
	test_pass(cb.size >= cb.used);
	test_mem(cb.buf, ==, "1234567890abcde", 15);

	varbuf_destroy(&cb);
	varbuf_destroy(&vb);
}

static void
test_varbuf_add_buf(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 5);

	varbuf_add_buf(&vb, "1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890", 10);

	varbuf_add_buf(&vb, "abcde", 5);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890abcde", 15);

	varbuf_destroy(&vb);
}

static void
test_varbuf_add_str(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 5);

	varbuf_add_str(&vb, "1234567890");
	test_str(vb.buf, ==, "1234567890");

	varbuf_add_str(&vb, "abcd");
	test_str(vb.buf, ==, "1234567890abcd");

	varbuf_add_strn(&vb, "1234567890", 5);
	test_str(vb.buf, ==, "1234567890abcd12345");

	varbuf_add_strn(&vb, "abcd", 0);
	test_str(vb.buf, ==, "1234567890abcd12345");

	varbuf_destroy(&vb);
}

static void
test_varbuf_add_char(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 1);

	varbuf_add_char(&vb, 'a');
	test_pass(vb.used == 1);
	test_pass(vb.size >= vb.used);
	test_pass(vb.buf[0] == 'a');

	varbuf_add_char(&vb, 'b');
	test_pass(vb.used == 2);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "ab", 2);

	varbuf_add_char(&vb, 'c');
	test_pass(vb.used == 3);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abc", 3);

	varbuf_add_char(&vb, 'd');
	test_pass(vb.used == 4);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abcd", 4);

	varbuf_destroy(&vb);
}

static void
test_varbuf_dup_char(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 5);

	varbuf_dup_char(&vb, 'z', 10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "zzzzzzzzzz", 10);

	varbuf_dup_char(&vb, 'y', 5);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "zzzzzzzzzzyyyyy", 15);

	varbuf_destroy(&vb);
}

static void
test_varbuf_map_char(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 5);

	varbuf_add_buf(&vb, "1234a5678a9012a", 15);

	varbuf_map_char(&vb, 'a', 'z');
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234z5678z9012z", 15);

	varbuf_destroy(&vb);
}

static void
test_varbuf_add_dir(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 10);

	varbuf_add_dir(&vb, "");
	test_str(vb.buf, ==, "/");
	varbuf_add_dir(&vb, "");
	test_str(vb.buf, ==, "/");
	varbuf_add_dir(&vb, "aa");
	test_str(vb.buf, ==, "/aa/");
	varbuf_add_dir(&vb, "");
	test_str(vb.buf, ==, "/aa/");

	varbuf_reset(&vb);

	varbuf_add_dir(&vb, "/foo/bar");
	test_str(vb.buf, ==, "/foo/bar/");

	varbuf_reset(&vb);

	varbuf_add_dir(&vb, "/foo/bar/");
	test_str(vb.buf, ==, "/foo/bar/");
	varbuf_add_dir(&vb, "quux");
	test_str(vb.buf, ==, "/foo/bar/quux/");
	varbuf_add_dir(&vb, "zoo");
	test_str(vb.buf, ==, "/foo/bar/quux/zoo/");

	varbuf_destroy(&vb);
}

static void
test_varbuf_end_str(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 10);

	varbuf_add_buf(&vb, "1234567890X", 11);
	test_pass(vb.used == 11);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890X", 11);

	varbuf_trunc(&vb, 10);

	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used + 1);
	test_pass(vb.buf[10] == '\0');
	test_str(vb.buf, ==, "1234567890");

	varbuf_destroy(&vb);
}

static void
test_varbuf_str(void)
{
	struct varbuf vb;
	const char *str;

	varbuf_init(&vb, 10);

	varbuf_add_buf(&vb, "1234567890", 10);
	str = varbuf_str(&vb);
	test_pass(vb.buf == str);
	test_pass(vb.used == 10);
	test_pass(vb.buf[vb.used] == '\0');
	test_pass(str[vb.used] == '\0');
	test_str(vb.buf, ==, "1234567890");
	test_str(str, ==, "1234567890");

	varbuf_add_buf(&vb, "abcde", 5);
	str = varbuf_str(&vb);
	test_pass(vb.buf == str);
	test_pass(vb.used == 15);
	test_pass(vb.buf[vb.used] == '\0');
	test_pass(str[vb.used] == '\0');
	test_str(vb.buf, ==, "1234567890abcde");
	test_str(str, ==, "1234567890abcde");

	varbuf_destroy(&vb);
}

static void
test_varbuf_has(void)
{
	struct varbuf vb = VARBUF_OBJECT;
	struct varbuf vb_prefix = VARBUF_OBJECT;
	struct varbuf vb_suffix = VARBUF_OBJECT;

	test_pass(varbuf_has_prefix(&vb, &vb_prefix));
	test_pass(varbuf_has_suffix(&vb, &vb_suffix));

	varbuf_set_str(&vb_prefix, "prefix");
	varbuf_set_str(&vb_suffix, "suffix");

	test_fail(varbuf_has_prefix(&vb, &vb_prefix));
	test_fail(varbuf_has_suffix(&vb, &vb_suffix));

	varbuf_set_str(&vb, "prefix and some text");
	test_pass(varbuf_has_prefix(&vb, &vb_prefix));
	test_fail(varbuf_has_prefix(&vb, &vb_suffix));
	test_fail(varbuf_has_suffix(&vb, &vb_prefix));
	test_fail(varbuf_has_suffix(&vb, &vb_suffix));

	varbuf_set_str(&vb, "some text with suffix");
	test_fail(varbuf_has_prefix(&vb, &vb_prefix));
	test_fail(varbuf_has_prefix(&vb, &vb_suffix));
	test_fail(varbuf_has_suffix(&vb, &vb_prefix));
	test_pass(varbuf_has_suffix(&vb, &vb_suffix));

	varbuf_set_str(&vb, "prefix and some text with suffix");
	test_pass(varbuf_has_prefix(&vb, &vb_prefix));
	test_fail(varbuf_has_prefix(&vb, &vb_suffix));
	test_fail(varbuf_has_suffix(&vb, &vb_prefix));
	test_pass(varbuf_has_suffix(&vb, &vb_suffix));

	varbuf_destroy(&vb_prefix);
	varbuf_destroy(&vb_suffix);
	varbuf_destroy(&vb);
}

static void
test_varbuf_trim(void)
{
	struct varbuf vb = VARBUF_OBJECT;
	struct varbuf vb_prefix = VARBUF_OBJECT;
	struct varbuf vb_suffix = VARBUF_OBJECT;

	varbuf_set_str(&vb_prefix, "prefix");
	varbuf_set_str(&vb_suffix, "suffix");

	varbuf_set_str(&vb, "some text");
	varbuf_trim_varbuf_prefix(&vb, &vb_prefix);
	varbuf_trim_char_prefix(&vb, 'a');
	test_str(vb.buf, ==, "some text");

	varbuf_set_str(&vb, "prefix and some text");
	varbuf_trim_varbuf_prefix(&vb, &vb_prefix);
	test_str(vb.buf, ==, " and some text");

	varbuf_set_str(&vb, "       and some text");
	varbuf_trim_char_prefix(&vb, ' ');
	test_str(vb.buf, ==, "and some text");

	varbuf_destroy(&vb_prefix);
	varbuf_destroy(&vb_suffix);
	varbuf_destroy(&vb);
}

static void
test_varbuf_add_fmt(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 5);

	/* Test normal format printing. */
	varbuf_add_fmt(&vb, "format %s number %d", "string", 10);
	test_pass(vb.used == strlen("format string number 10"));
	test_pass(vb.size >= vb.used);
	test_str(vb.buf, ==, "format string number 10");

	varbuf_reset(&vb);

	/* Test concatenated format printing. */
	varbuf_add_fmt(&vb, "format %s number %d", "string", 10);
	varbuf_add_fmt(&vb, " extra %s", "string");
	test_pass(vb.used == strlen("format string number 10 extra string"));
	test_pass(vb.size >= vb.used);
	test_str(vb.buf, ==, "format string number 10 extra string");

	varbuf_destroy(&vb);
}

static void
test_varbuf_reset(void)
{
	struct varbuf vb;

	varbuf_init(&vb, 10);

	varbuf_add_buf(&vb, "1234567890", 10);

	varbuf_reset(&vb);
	test_pass(vb.used == 0);
	test_pass(vb.size >= vb.used);

	varbuf_add_buf(&vb, "abcdefghijklmno", 15);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abcdefghijklmno", 15);

	varbuf_destroy(&vb);
}

static void
test_varbuf_snapshot(void)
{
	struct varbuf vb;
	struct varbuf_state vbs;

	varbuf_init(&vb, 0);

	test_pass(vb.used == 0);
	varbuf_snapshot(&vb, &vbs);
	test_pass(vb.used == 0);
	test_pass(vb.used == vbs.used);
	test_pass(varbuf_rollback_len(&vbs) == 0);
	test_str(varbuf_rollback_end(&vbs), ==, "");

	varbuf_add_buf(&vb, "1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(varbuf_rollback_len(&vbs) == 10);
	test_str(varbuf_rollback_end(&vbs), ==, "1234567890");
	varbuf_rollback(&vbs);
	test_pass(vb.used == 0);
	test_pass(varbuf_rollback_len(&vbs) == 0);
	test_str(varbuf_rollback_end(&vbs), ==, "");

	varbuf_add_buf(&vb, "1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(varbuf_rollback_len(&vbs) == 10);
	test_str(varbuf_rollback_end(&vbs), ==, "1234567890");
	varbuf_snapshot(&vb, &vbs);
	test_pass(vb.used == 10);
	test_pass(varbuf_rollback_len(&vbs) == 0);
	test_str(varbuf_rollback_end(&vbs), ==, "");

	varbuf_add_buf(&vb, "1234567890", 10);
	test_pass(vb.used == 20);
	test_pass(varbuf_rollback_len(&vbs) == 10);
	test_str(varbuf_rollback_end(&vbs), ==, "1234567890");
	varbuf_rollback(&vbs);
	test_pass(vb.used == 10);
	test_pass(varbuf_rollback_len(&vbs) == 0);
	test_str(varbuf_rollback_end(&vbs), ==, "");

	varbuf_destroy(&vb);
}

static void
test_varbuf_detach(void)
{
	struct varbuf vb;
	char *str;

	varbuf_init(&vb, 0);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
	str = varbuf_detach(&vb);
	test_str(str, ==, "");
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
	free(str);

	varbuf_init(&vb, 0);
	varbuf_add_buf(&vb, NULL, 0);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
	str = varbuf_detach(&vb);
	test_str(str, ==, "");
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
	free(str);

	varbuf_init(&vb, 0);
	varbuf_add_buf(&vb, "1234567890", 10);
	str = varbuf_detach(&vb);
	test_mem(str, ==, "1234567890", 10);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == NULL);
	free(str);
}

TEST_ENTRY(test)
{
	test_plan(205);

	test_varbuf_init();
	test_varbuf_prealloc();
	test_varbuf_new();
	test_varbuf_grow();
	test_varbuf_trunc();
	test_varbuf_set();
	test_varbuf_add_varbuf();
	test_varbuf_add_buf();
	test_varbuf_add_str();
	test_varbuf_add_char();
	test_varbuf_dup_char();
	test_varbuf_map_char();
	test_varbuf_add_dir();
	test_varbuf_end_str();
	test_varbuf_str();
	test_varbuf_has();
	test_varbuf_trim();
	test_varbuf_add_fmt();
	test_varbuf_reset();
	test_varbuf_snapshot();
	test_varbuf_detach();

	/* TODO: Complete. */
}
