/*
 * libdpkg - Debian packaging suite library routines
 * t-varbuf.c - test varbuf C++ implementation
 *
 * Copyright © 2009-2024 Guillem Jover <guillem@debian.org>
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
	varbuf vb;

	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);

	vb.destroy();
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
}

static void
test_varbuf_prealloc(void)
{
	varbuf vb(10);

	test_pass(vb.used == 0);
	test_pass(vb.size >= 10);
	test_pass(vb.buf != nullptr);

	vb.destroy();
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
}

static void
test_varbuf_new(void)
{
	varbuf *vb;

	vb = new varbuf;
	test_pass(vb != nullptr);
	test_pass(vb->used == 0);
	test_pass(vb->size == 0);
	test_pass(vb->buf == nullptr);
	delete vb;

	vb = new varbuf(10);
	test_pass(vb != nullptr);
	test_pass(vb->used == 0);
	test_pass(vb->size >= 10);
	test_pass(vb->buf != nullptr);
	delete vb;
}

static void
test_varbuf_grow(void)
{
	varbuf vb(10);
	jmp_buf grow_jump;
	volatile size_t old_size;
	volatile bool grow_overflow;
	int i;

	/* Test that we grow when needed. */
	vb.grow(100);
	test_pass(vb.used == 0);
	test_pass(vb.size >= 100);

	old_size = vb.size;

	/* Test that we are not leaking. */
	for (i = 0; i < 10; i++) {
		vb.grow(100);
		test_pass(vb.used == 0);
		test_pass(vb.size >= 100);
		test_pass(vb.size == old_size);
	}

	/* Test that we grow when needed, with used space. */
	vb.used = 10;
	vb.grow(100);
	test_pass(vb.used == 10);
	test_pass(vb.size >= 110);

	/* Test that we do not allow allocation overflows. */
	grow_overflow = false;
	old_size = vb.size;
	test_try(grow_jump) {
		vb.grow(SIZE_MAX - vb.size + 2);
	} test_catch {
		grow_overflow = true;
	} test_finally;
	test_pass(vb.size == old_size && grow_overflow);

	grow_overflow = false;
	old_size = vb.size;
	test_try(grow_jump) {
		vb.grow((SIZE_MAX - vb.size - 2) / 2);
	} test_catch {
		grow_overflow = true;
	} test_finally;
	test_pass(vb.size == old_size && grow_overflow);

	vb.destroy();
}

static void
test_varbuf_trunc(void)
{
	varbuf vb(50);

	/* Test that we truncate (grow). */
	vb.trunc(20);
	test_pass(vb.used == 20);
	test_pass(vb.size >= 50);

	/* Test that we truncate (shrink). */
	vb.trunc(10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= 50);
}

static void
test_varbuf_set(void)
{
	varbuf vb(10), cb(10);

	vb.set_buf("1234567890", 5);
	test_pass(vb.used == 5);
	test_mem(vb.buf, ==, "12345", 5);

	vb.set_buf("abcd", 4);
	test_pass(vb.used == 4);
	test_mem(vb.buf, ==, "abcd", 4);

	cb.set(vb);
	test_pass(cb.used == 4);
	test_mem(cb.buf, ==, "abcd", 4);

	vb.set("12345");
	test_pass(vb.used == 5);
	test_str(vb.buf, ==, "12345");

	vb.set("1234567890", 8);
	test_pass(vb.used == 8);
	test_str(vb.buf, ==, "12345678");
}

static void
test_varbuf_add_varbuf(void)
{
	varbuf vb(5), cb;

	vb.set("1234567890");
	cb.add(vb);
	test_pass(cb.used == 10);
	test_pass(cb.size >= cb.used);
	test_mem(cb.buf, ==, "1234567890", 10);

	vb.set("abcde");
	cb.add(vb);
	test_pass(cb.used == 15);
	test_pass(cb.size >= cb.used);
	test_mem(cb.buf, ==, "1234567890abcde", 15);
}

static void
test_varbuf_add_buf(void)
{
	varbuf vb(5);

	vb.add_buf("1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890", 10);

	vb.add_buf("abcde", 5);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890abcde", 15);
}

static void
test_varbuf_add_str(void)
{
	varbuf vb(5);

	vb.add("1234567890");
	test_str(vb.buf, ==, "1234567890");

	vb.add("abcd");
	test_str(vb.buf, ==, "1234567890abcd");

	vb.add("1234567890", 5);
	test_str(vb.buf, ==, "1234567890abcd12345");

	vb.add("abcd", 0);
	test_str(vb.buf, ==, "1234567890abcd12345");
}

static void
test_varbuf_add_char(void)
{
	varbuf vb(1);

	vb.add('a');
	test_pass(vb.used == 1);
	test_pass(vb.size >= vb.used);
	test_pass(vb.buf[0] == 'a');

	vb.add('b');
	test_pass(vb.used == 2);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "ab", 2);

	vb.add('c');
	test_pass(vb.used == 3);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abc", 3);

	vb.add('d');
	test_pass(vb.used == 4);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abcd", 4);
}

static void
test_varbuf_dup_char(void)
{
	varbuf vb(5);

	vb.dup('z', 10);
	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "zzzzzzzzzz", 10);

	vb.dup('y', 5);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "zzzzzzzzzzyyyyy", 15);
}

static void
test_varbuf_map_char(void)
{
	varbuf vb(5);

	vb.add_buf("1234a5678a9012a", 15);

	vb.map('a', 'z');
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234z5678z9012z", 15);
}

static void
test_varbuf_add_dir(void)
{
	varbuf vb(10);

	vb.add_dir("");
	test_str(vb.buf, ==, "/");
	vb.add_dir("");
	test_str(vb.buf, ==, "/");
	vb.add_dir("aa");
	test_str(vb.buf, ==, "/aa/");
	vb.add_dir("");
	test_str(vb.buf, ==, "/aa/");

	vb.reset();

	vb.add_dir("/foo/bar");
	test_str(vb.buf, ==, "/foo/bar/");

	vb.reset();

	vb.add_dir("/foo/bar/");
	test_str(vb.buf, ==, "/foo/bar/");
	vb.add_dir("quux");
	test_str(vb.buf, ==, "/foo/bar/quux/");
	vb.add_dir("zoo");
	test_str(vb.buf, ==, "/foo/bar/quux/zoo/");
}

static void
test_varbuf_end_str(void)
{
	varbuf vb(10);

	vb.add_buf("1234567890X", 11);
	test_pass(vb.used == 11);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "1234567890X", 11);

	vb.trunc(10);

	test_pass(vb.used == 10);
	test_pass(vb.size >= vb.used + 1);
	test_pass(vb.buf[10] == '\0');
	test_str(vb.buf, ==, "1234567890");
}

static void
test_varbuf_str(void)
{
	varbuf vb(10);
	const char *str;

	vb.add_buf("1234567890", 10);
	str = vb.str();
	test_pass(vb.buf == str);
	test_pass(vb.used == 10);
	test_pass(vb.buf[vb.used] == '\0');
	test_pass(str[vb.used] == '\0');
	test_str(vb.buf, ==, "1234567890");
	test_str(str, ==, "1234567890");

	vb.add_buf("abcde", 5);
	str = vb.str();
	test_pass(vb.buf == str);
	test_pass(vb.used == 15);
	test_pass(vb.buf[vb.used] == '\0');
	test_pass(str[vb.used] == '\0');
	test_str(vb.buf, ==, "1234567890abcde");
	test_str(str, ==, "1234567890abcde");
}

static void
test_varbuf_has(void)
{
	varbuf vb;
	varbuf vb_prefix;
	varbuf vb_suffix;

	test_pass(vb.has_prefix(vb_prefix));
	test_pass(vb.has_suffix(vb_suffix));

	vb_prefix.set("prefix");
	vb_suffix.set("suffix");

	test_fail(vb.has_prefix(vb_prefix));
	test_fail(vb.has_suffix(vb_suffix));

	vb.set("prefix and some text");
	test_pass(vb.has_prefix(vb_prefix));
	test_fail(vb.has_prefix(vb_suffix));
	test_fail(vb.has_suffix(vb_prefix));
	test_fail(vb.has_suffix(vb_suffix));

	vb.set("some text with suffix");
	test_fail(vb.has_prefix(vb_prefix));
	test_fail(vb.has_prefix(vb_suffix));
	test_fail(vb.has_suffix(vb_prefix));
	test_pass(vb.has_suffix(vb_suffix));

	vb.set("prefix and some text with suffix");
	test_pass(vb.has_prefix(vb_prefix));
	test_fail(vb.has_prefix(vb_suffix));
	test_fail(vb.has_suffix(vb_prefix));
	test_pass(vb.has_suffix(vb_suffix));
}

static void
test_varbuf_trim(void)
{
	varbuf vb;
	varbuf vb_prefix;
	varbuf vb_suffix;

	vb_prefix.set("prefix");
	vb_suffix.set("suffix");

	vb.set("some text");
	vb.trim_prefix(vb_prefix);
	vb.trim_prefix('a');
	test_str(vb.buf, ==, "some text");

	vb.set("prefix and some text");
	vb.trim_prefix(vb_prefix);
	test_str(vb.buf, ==, " and some text");

	vb.set("       and some text");
	vb.trim_prefix(' ');
	test_str(vb.buf, ==, "and some text");
}

static void
test_varbuf_add_fmt(void)
{
	varbuf vb(5);

	/* Test normal format printing. */
	vb.add_fmt("format %s number %d", "string", 10);
	test_pass(vb.used == strlen("format string number 10"));
	test_pass(vb.size >= vb.used);
	test_str(vb.buf, ==, "format string number 10");

	vb.reset();

	/* Test concatenated format printing. */
	vb.add_fmt("format %s number %d", "string", 10);
	vb.add_fmt(" extra %s", "string");
	test_pass(vb.used == strlen("format string number 10 extra string"));
	test_pass(vb.size >= vb.used);
	test_str(vb.buf, ==, "format string number 10 extra string");
}

static void
test_varbuf_reset(void)
{
	varbuf vb(10);

	vb.add_buf("1234567890", 10);

	vb.reset();
	test_pass(vb.used == 0);
	test_pass(vb.size >= vb.used);

	vb.add_buf("abcdefghijklmno", 15);
	test_pass(vb.used == 15);
	test_pass(vb.size >= vb.used);
	test_mem(vb.buf, ==, "abcdefghijklmno", 15);
}

static void
test_varbuf_snapshot(void)
{
	varbuf vb;
	varbuf_state vbs;

	test_pass(vb.used == 0);
	vbs.snapshot(vb);
	test_pass(vb.used == 0);
	test_pass(vb.used == vbs.used);
	test_pass(vbs.rollback_len() == 0);
	test_str(vbs.rollback_end(), ==, "");

	vb.add_buf("1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(vbs.rollback_len() == 10);
	test_str(vbs.rollback_end(), ==, "1234567890");
	vbs.rollback();
	test_pass(vb.used == 0);
	test_pass(vbs.rollback_len() == 0);
	test_str(vbs.rollback_end(), ==, "");

	vb.add_buf("1234567890", 10);
	test_pass(vb.used == 10);
	test_pass(vbs.rollback_len() == 10);
	test_str(vbs.rollback_end(), ==, "1234567890");
	vbs.snapshot(vb);
	test_pass(vb.used == 10);
	test_pass(vbs.rollback_len() == 0);
	test_str(vbs.rollback_end(), ==, "");

	vb.add_buf("1234567890", 10);
	test_pass(vb.used == 20);
	test_pass(vbs.rollback_len() == 10);
	test_str(vbs.rollback_end(), ==, "1234567890");
	vbs.rollback();
	test_pass(vb.used == 10);
	test_pass(vbs.rollback_len() == 0);
	test_str(vbs.rollback_end(), ==, "");
}

static void
test_varbuf_detach(void)
{
	varbuf vb;
	char *str;

	vb.init(0);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
	str = vb.detach();
	test_str(str, ==, "");
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
	free(str);

	vb.init(0);
	vb.add_buf(nullptr, 0);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
	str = vb.detach();
	test_str(str, ==, "");
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
	free(str);

	vb.init(0);
	vb.add_buf("1234567890", 10);
	str = vb.detach();
	test_mem(str, ==, "1234567890", 10);
	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);
	free(str);
}

static void
test_varbuf_operator(void)
{
	varbuf vb;
	varbuf cb("some text");

	test_pass(vb.used == 0);
	test_pass(vb.size == 0);
	test_pass(vb.buf == nullptr);

	test_pass(cb.used == 9);
	test_pass(cb.size >= 9);
	test_mem(cb.buf, ==, "some text", 9);

	vb = cb;
	test_pass(vb.used == 9);
	test_pass(vb.size >= 9);
	test_mem(vb.buf, ==, "some text", 9);

	vb += '&';
	test_pass(vb.used == 10);
	test_pass(vb.size >= 10);
	test_mem(vb.buf, ==, "some text&", 10);

	vb += cb;
	test_pass(vb.used == 19);
	test_pass(vb.size >= 19);
	test_mem(vb.buf, ==, "some text&some text", 19);

	vb += "$end$";
	test_pass(vb.used == 24);
	test_pass(vb.size >= 24);
	test_mem(vb.buf, ==, "some text&some text$end$", 24);

	vb = cb;
	test_pass(vb.used == 9);
	test_pass(vb.size >= 9);
	test_mem(vb.buf, ==, "some text", 9);
}

TEST_ENTRY(test)
{
	test_plan(226);

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
	test_varbuf_operator();

	/* TODO: Complete. */
}
