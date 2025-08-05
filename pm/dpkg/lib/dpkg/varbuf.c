/*
 * libdpkg - Debian packaging suite library routines
 * varbuf.c - variable length expandable buffer handling
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2015 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

struct varbuf *
varbuf_new(size_t size)
{
	struct varbuf *v;

	v = m_malloc(sizeof(*v));
	varbuf_init(v, size);

	return v;
}

void
varbuf_init(struct varbuf *v, size_t size)
{
	v->used = 0;
	v->size = size;
	if (size) {
		v->buf = m_malloc(size);
		v->buf[0] = '\0';
	} else {
		v->buf = NULL;
	}
}

void
varbuf_swap(struct varbuf *v, struct varbuf *o)
{
	struct varbuf m = *v;

	*v = *o;
	*o = m;
}

void
varbuf_grow(struct varbuf *v, size_t need_size)
{
	size_t new_size;

	/* Make sure the varbuf is in a sane state. */
	if (v->size < v->used)
		internerr("varbuf used(%zu) > size(%zu)", v->used, v->size);

	/* Check if we already have enough room. */
	if ((v->size - v->used) >= need_size)
		return;

	/* Check if we overflow. */
	new_size = (v->size + need_size) * 2;
	if (new_size < v->size)
		ohshit(_("cannot grow varbuf to size %zu; it would overflow"),
		       need_size);

	v->size = new_size;
	v->buf = m_realloc(v->buf, v->size);
}

void
varbuf_trunc(struct varbuf *v, size_t used_size)
{
	/* Make sure the caller does not claim more than available. */
	if (v->size < used_size)
		internerr("varbuf new_used(%zu) > size(%zu)", used_size, v->size);

	v->used = used_size;
	if (v->buf)
		v->buf[v->used] = '\0';
}

void
varbuf_reset(struct varbuf *v)
{
	v->used = 0;
	if (v->buf)
		v->buf[0] = '\0';
}

const char *
varbuf_str(struct varbuf *v)
{
	if (v->buf == NULL)
		return "";

	return v->buf;
}

char *
varbuf_array(const struct varbuf *v, size_t index)
{
	if (index > v->used)
		internerr("varbuf array access (%zu) > used (%zu)",
		          index, v->used);

	return v->buf + index;
}

void
varbuf_set_buf(struct varbuf *v, const void *buf, size_t size)
{
	varbuf_reset(v);
	varbuf_add_buf(v, buf, size);
}

void
varbuf_set_varbuf(struct varbuf *v, struct varbuf *other)
{
	varbuf_set_buf(v, other->buf, other->used);
}

int
varbuf_set_vfmt(struct varbuf *v, const char *fmt, va_list args)
{
	varbuf_reset(v);
	return varbuf_add_vfmt(v, fmt, args);
}

int
varbuf_set_fmt(struct varbuf *v, const char *fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = varbuf_set_vfmt(v, fmt, args);
	va_end(args);

	return n;
}

void
varbuf_add_varbuf(struct varbuf *v, const struct varbuf *other)
{
	varbuf_grow(v, other->used + 1);
	memcpy(v->buf + v->used, other->buf, other->used);
	v->used += other->used;
	v->buf[v->used] = '\0';
}

void
varbuf_add_char(struct varbuf *v, int c)
{
	varbuf_grow(v, 2);
	v->buf[v->used++] = c;
	v->buf[v->used] = '\0';
}

void
varbuf_dup_char(struct varbuf *v, int c, size_t n)
{
	if (n == 0)
		return;
	varbuf_grow(v, n + 1);
	memset(v->buf + v->used, c, n);
	v->used += n;
	v->buf[v->used] = '\0';
}

void
varbuf_map_char(struct varbuf *v, int c_src, int c_dst)
{
	size_t i;

	for (i = 0; i < v->used; i++)
		if (v->buf[i] == c_src)
			v->buf[i] = c_dst;
}

void
varbuf_add_dir(struct varbuf *v, const char *dirname)
{
	varbuf_add_str(v, dirname);
	if (v->used == 0 || v->buf[v->used - 1] != '/')
		varbuf_add_char(v, '/');
}

void
varbuf_add_buf(struct varbuf *v, const void *s, size_t size)
{
	if (size == 0)
		return;
	varbuf_grow(v, size + 1);
	memcpy(v->buf + v->used, s, size);
	v->used += size;
	v->buf[v->used] = '\0';
}

bool
varbuf_has_prefix(struct varbuf *v, struct varbuf *prefix)
{
	if (prefix->used > v->used)
		return false;

	if (prefix->used == 0)
		return true;
	if (v->used == 0)
		return false;

	return strncmp(v->buf, prefix->buf, prefix->used) == 0;
}

bool
varbuf_has_suffix(struct varbuf *v, struct varbuf *suffix)
{
	const char *slice;

	if (suffix->used > v->used)
		return false;

	if (suffix->used == 0)
		return true;
	if (v->used == 0)
		return false;

	slice = v->buf + v->used - suffix->used;

	return strcmp(slice, suffix->buf) == 0;
}

void
varbuf_trim_varbuf_prefix(struct varbuf *v, struct varbuf *prefix)
{
	if (!varbuf_has_prefix(v, prefix))
		return;

	memmove(v->buf, v->buf + prefix->used, v->used - prefix->used);
	varbuf_trunc(v, v->used - prefix->used);
}

void
varbuf_trim_char_prefix(struct varbuf *v, int prefix)
{
	const char *str = v->buf;
	size_t len = v->used;

	while (str[0] == prefix && len > 0) {
		str++;
		len--;
	}
	if (str != v->buf) {
		memmove(v->buf, str, len);
		varbuf_trunc(v, len);
	}
}

int
varbuf_add_vfmt(struct varbuf *v, const char *fmt, va_list args)
{
	va_list args_copy;
	int needed, n;

	va_copy(args_copy, args);
	needed = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (needed < 0)
		ohshite(_("error formatting string into varbuf variable"));

	varbuf_grow(v, needed + 1);

	n = vsnprintf(v->buf + v->used, needed + 1, fmt, args);
	if (n < 0)
		ohshite(_("error formatting string into varbuf variable"));

	v->used += n;

	return n;
}

int
varbuf_add_fmt(struct varbuf *v, const char *fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = varbuf_add_vfmt(v, fmt, args);
	va_end(args);

	return n;
}

void
varbuf_snapshot(struct varbuf *v, struct varbuf_state *vs)
{
	vs->v = v;
	vs->used = v->used;
}

void
varbuf_rollback(struct varbuf_state *vs)
{
	varbuf_trunc(vs->v, vs->used);
}

size_t
varbuf_rollback_len(struct varbuf_state *vs)
{
	if (vs->used > vs->v->used)
		internerr("varbuf state_used(%zu) > used(%zu)",
		          vs->used, vs->v->used);
	return vs->v->used - vs->used;
}

const char *
varbuf_rollback_end(struct varbuf_state *vs)
{
	if (vs->v->buf == NULL) {
		if (vs->used)
			internerr("varbuf buf(NULL) state_used(%zu) > 0",
			          vs->used);
		return varbuf_str(vs->v);
	}
	return vs->v->buf + vs->used;
}

char *
varbuf_detach(struct varbuf *v)
{
	char *buf = v->buf;

	v->buf = NULL;
	v->size = 0;
	v->used = 0;

	if (buf == NULL)
		buf = m_strdup("");

	return buf;
}

void
varbuf_destroy(struct varbuf *v)
{
	free(v->buf);
	v->buf = NULL;
	v->size = 0;
	v->used = 0;
}

void
varbuf_free(struct varbuf *v)
{
	free(v->buf);
	free(v);
}
