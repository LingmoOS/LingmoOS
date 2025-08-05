/*
 * libdpkg - Debian packaging suite library routines
 * strvec.h - string vector support
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

#ifndef LIBDPKG_STRVEC_H
#define LIBDPKG_STRVEC_H

#include <stddef.h>

#include <dpkg/macros.h>

DPKG_BEGIN_DECLS

/**
 * @defgroup strvec String vector functions
 * @ingroup dpkg-internal
 * @{
 */

struct strvec {
	char **vec;
	size_t size;
	size_t used;
};

struct strvec *
strvec_new(size_t size);

void
strvec_grow(struct strvec *sv, size_t need_size);

void
strvec_push(struct strvec *sv, char *str);

char *
strvec_pop(struct strvec *sv);

void
strvec_drop(struct strvec *sv);

const char *
strvec_peek(struct strvec *sv);

enum DPKG_ATTR_ENUM_FLAGS strvec_split_flags {
	STRVEC_SPLIT_DEFAULT		= 0,
	STRVEC_SPLIT_SKIP_DUP_SEP	= DPKG_BIT(0),
};

struct strvec *
strvec_split(const char *str, int sep, enum strvec_split_flags flags);

char *
strvec_join(struct strvec *sv, int sep);

void
strvec_free(struct strvec *sv);

/** @} */

DPKG_END_DECLS

#endif /* LIBDPKG_STRVEC_H */
