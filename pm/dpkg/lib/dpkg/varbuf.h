/*
 * libdpkg - Debian packaging suite library routines
 * varbuf.h - variable length expandable buffer handling
 *
 * Copyright © 1994, 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#ifndef LIBDPKG_VARBUF_H
#define LIBDPKG_VARBUF_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <dpkg/macros.h>

DPKG_BEGIN_DECLS

/**
 * @defgroup varbuf Variable length buffers
 * @ingroup dpkg-public
 * @{
 */

/**
 * varbuf_init must be called exactly once before the use of each varbuf
 * (including before any call to varbuf_destroy), or the variable must be
 * initialized with VARBUF_INIT.
 *
 * However, varbufs allocated ‘static’ are properly initialized anyway and
 * do not need varbuf_init; multiple consecutive calls to varbuf_init before
 * any use are allowed.
 *
 * varbuf_destroy must be called after a varbuf is finished with, if anything
 * other than varbuf_init has been done. After this you are allowed but
 * not required to call varbuf_init again if you want to start using the
 * varbuf again.
 *
 * Callers using C++ need not worry about any of this.
 */
struct varbuf {
	size_t used, size;
	char *buf;

#ifdef __cplusplus
	explicit varbuf(size_t _size = 0);
	explicit varbuf(const char *str);
	varbuf(const varbuf &v);
	varbuf(varbuf &&v);
	~varbuf();

	void init(size_t _size = 0);
	void grow(size_t need_size);
	void trunc(size_t used_size);
	char *detach();
	void reset();
	void destroy();

	void set_buf(const void *buf, size_t _size);
	void set(varbuf &other);
	void set(const char *str);
	void set(const char *str, size_t len);

	int set_vfmt(const char *fmt, va_list args)
		DPKG_ATTR_VPRINTF(2);
	int set_fmt(const char *fmt, ...)
		DPKG_ATTR_PRINTF(2);

	void add_buf(const void *str, size_t _size);
	void add(const varbuf &other);
	void add(int c);
	void add(const char *str);
	void add(const char *str, size_t len);
	void add_dir(const char *dirname);

	int add_vfmt(const char *fmt, va_list args)
		DPKG_ATTR_VPRINTF(2);
	int add_fmt(const char *fmt, ...)
		DPKG_ATTR_PRINTF(2);

	void dup(int c, size_t n);
	void map(int c_src, int c_dst);

	bool has_prefix(varbuf &prefix);
	bool has_suffix(varbuf &suffix);
	void trim_prefix(varbuf &prefix);
	void trim_prefix(int prefix);

	varbuf &operator=(const varbuf &v);
	varbuf &operator=(varbuf &&v);
	varbuf &operator+=(const varbuf &v);
	varbuf &operator+=(int c);
	varbuf &operator+=(const char *str);

	const char &operator[](int i) const;
	char &operator[](int i);

	size_t len() const;
	const char *str();
#endif
};

#define VARBUF_INIT { 0, 0, NULL }

#define VARBUF_OBJECT (struct varbuf)VARBUF_INIT

struct varbuf *varbuf_new(size_t size);
void varbuf_init(struct varbuf *v, size_t size);
void varbuf_grow(struct varbuf *v, size_t need_size);
void varbuf_swap(struct varbuf *v, struct varbuf *other);
void varbuf_trunc(struct varbuf *v, size_t used_size);
char *varbuf_detach(struct varbuf *v);
void varbuf_reset(struct varbuf *v);
void varbuf_destroy(struct varbuf *v);
void varbuf_free(struct varbuf *v);

const char *varbuf_str(struct varbuf *v);
char *varbuf_array(const struct varbuf *v, size_t index);

void varbuf_set_varbuf(struct varbuf *v, struct varbuf *other);
void varbuf_set_buf(struct varbuf *v, const void *buf, size_t size);
#define varbuf_set_str(v, s) varbuf_set_buf(v, s, strlen(s))
#define varbuf_set_strn(v, s, n) varbuf_set_buf(v, s, strnlen(s, n))
int varbuf_set_vfmt(struct varbuf *v, const char *fmt, va_list args)
	DPKG_ATTR_VPRINTF(2);
int varbuf_set_fmt(struct varbuf *v, const char *fmt, ...)
	DPKG_ATTR_PRINTF(2);

void varbuf_add_varbuf(struct varbuf *v, const struct varbuf *other);
void varbuf_add_char(struct varbuf *v, int c);
void varbuf_dup_char(struct varbuf *v, int c, size_t n);
void varbuf_map_char(struct varbuf *v, int c_src, int c_dst);
#define varbuf_add_str(v, s) varbuf_add_buf(v, s, strlen(s))
#define varbuf_add_strn(v, s, n) varbuf_add_buf(v, s, strnlen(s, n))
void varbuf_add_dir(struct varbuf *v, const char *dirname);
void varbuf_add_buf(struct varbuf *v, const void *s, size_t size);
int varbuf_add_fmt(struct varbuf *v, const char *fmt, ...)
	DPKG_ATTR_PRINTF(2);
int varbuf_add_vfmt(struct varbuf *v, const char *fmt, va_list args)
	DPKG_ATTR_VPRINTF(2);

bool varbuf_has_prefix(struct varbuf *v, struct varbuf *prefix);
bool varbuf_has_suffix(struct varbuf *v, struct varbuf *suffix);
void varbuf_trim_varbuf_prefix(struct varbuf *v, struct varbuf *prefix);
void varbuf_trim_char_prefix(struct varbuf *v, int prefix);

struct varbuf_state {
	struct varbuf *v;
	size_t used;

#ifdef __cplusplus
	varbuf_state();
	explicit varbuf_state(varbuf &vb);
	~varbuf_state();

	void snapshot(varbuf &vb);
	void rollback();
	size_t rollback_len();
	const char *rollback_end();
#endif
};

void varbuf_snapshot(struct varbuf *v, struct varbuf_state *vs);
void varbuf_rollback(struct varbuf_state *vs);
size_t varbuf_rollback_len(struct varbuf_state *vs);
const char *varbuf_rollback_end(struct varbuf_state *vs);

/** @} */

DPKG_END_DECLS

#ifdef __cplusplus
inline
varbuf::varbuf(size_t _size)
{
	varbuf_init(this, _size);
}

inline
varbuf::varbuf(const char *str)
{
	varbuf_init(this, 0);
	varbuf_set_str(this, str);
}

inline
varbuf::varbuf(const varbuf &v)
{
	varbuf_init(this, v.used);
	varbuf_set_buf(this, v.buf, v.used);
}

inline
varbuf::varbuf(varbuf &&v):
	varbuf()
{
	varbuf_swap(this, &v);
}

inline
varbuf::~varbuf()
{
	varbuf_destroy(this);
}

inline void
varbuf::init(size_t _size)
{
	varbuf_init(this, _size);
}

inline void
varbuf::grow(size_t need_size)
{
	varbuf_grow(this, need_size);
}

inline void
varbuf::trunc(size_t used_size)
{
	varbuf_trunc(this, used_size);
}

inline char *
varbuf::detach()
{
	return varbuf_detach(this);
}

inline void
varbuf::reset()
{
	varbuf_reset(this);
}

inline void
varbuf::destroy()
{
	varbuf_destroy(this);
}

inline void
varbuf::set_buf(const void *_buf, size_t _size)
{
	varbuf_set_buf(this, _buf, _size);
}

inline void
varbuf::set(varbuf &other)
{
	varbuf_set_varbuf(this, &other);
}

inline void
varbuf::set(const char *str)
{
	varbuf_set_str(this, str);
}

inline void
varbuf::set(const char *str, size_t len)
{
	varbuf_set_strn(this, str, len);
}

inline int
varbuf::set_vfmt(const char *fmt, va_list args)
{
	return varbuf_set_vfmt(this, fmt, args);
}

inline int
varbuf::set_fmt(const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = varbuf_set_vfmt(this, fmt, args);
	va_end(args);

	return rc;
}

inline void
varbuf::add_buf(const void *str, size_t _size)
{
	varbuf_add_buf(this, str, _size);
}

inline void
varbuf::add(const varbuf &other)
{
	varbuf_add_varbuf(this, &other);
}

inline void
varbuf::add(int c)
{
	varbuf_add_char(this, c);
}

inline void
varbuf::add(const char *str)
{
	varbuf_add_str(this, str);
}

inline void
varbuf::add(const char *str, size_t len)
{
	varbuf_add_strn(this, str, len);
}

inline void
varbuf::add_dir(const char *dirname)
{
	varbuf_add_dir(this, dirname);
}

inline int
varbuf::add_vfmt(const char *fmt, va_list args)
{
	return varbuf_add_vfmt(this, fmt, args);
}

inline int
varbuf::add_fmt(const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = varbuf_add_vfmt(this, fmt, args);
	va_end(args);

	return rc;
}

inline void
varbuf::dup(int c, size_t n)
{
	varbuf_dup_char(this, c, n);
}

inline void
varbuf::map(int c_src, int c_dst)
{
	varbuf_map_char(this, c_src, c_dst);
}

inline bool
varbuf::has_prefix(varbuf &prefix)
{
	return varbuf_has_prefix(this, &prefix);
}

inline bool
varbuf::has_suffix(varbuf &suffix)
{
	return varbuf_has_suffix(this, &suffix);
}

inline void
varbuf::trim_prefix(varbuf &prefix)
{
	varbuf_trim_varbuf_prefix(this, &prefix);
}

inline void
varbuf::trim_prefix(int prefix)
{
	varbuf_trim_char_prefix(this, prefix);
}

inline varbuf &
varbuf::operator=(const varbuf &v)
{
	varbuf_set_buf(this, v.buf, v.used);

	return *this;
}

inline varbuf &
varbuf::operator=(varbuf &&v)
{
	varbuf_swap(this, &v);

	return *this;
}

inline varbuf &
varbuf::operator+=(const varbuf &v)
{
	varbuf_add_varbuf(this, &v);
	return *this;
}

inline varbuf &
varbuf::operator+=(int c)
{
	varbuf_add_char(this, c);
	return *this;
}

inline varbuf &
varbuf::operator+=(const char *str)
{
	varbuf_add_str(this, str);
	return *this;
}

inline const char &
varbuf::operator[](int i) const
{
	return *varbuf_array(this, i);
}

inline char &
varbuf::operator[](int i)
{
	return *varbuf_array(this, i);
}

inline size_t
varbuf::len() const
{
	return this->used;
}

inline const char *
varbuf::str()
{
	return varbuf_str(this);
}

inline
varbuf_state::varbuf_state():
	v(nullptr), used(0)
{
}

inline
varbuf_state::varbuf_state(varbuf &vb)
{
	varbuf_snapshot(&vb, this);
}

inline
varbuf_state::~varbuf_state()
{
}

inline void
varbuf_state::snapshot(varbuf &vb)
{
	varbuf_snapshot(&vb, this);
}

inline void
varbuf_state::rollback()
{
	varbuf_rollback(this);
}

inline size_t
varbuf_state::rollback_len()
{
	return varbuf_rollback_len(this);
}

inline const char *
varbuf_state::rollback_end()
{
	return varbuf_rollback_end(this);
}
#endif

#endif /* LIBDPKG_VARBUF_H */
