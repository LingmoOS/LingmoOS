/*
 * libdpkg - Debian packaging suite library routines
 * strvec.c - string vector support
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

#include <string.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/varbuf.h>
#include <dpkg/strvec.h>

struct strvec *
strvec_new(size_t size)
{
	struct strvec *sv;

	sv = m_malloc(sizeof(*sv));
	if (size == 0)
		sv->size = 16;
	else
		sv->size = size;
	sv->used = 0;
	sv->vec = m_calloc(sv->size, sizeof(char *));

	return sv;
}

void
strvec_grow(struct strvec *sv, size_t need_size)
{
	size_t new_size;

	if ((sv->size - sv->used) >= need_size)
		return;

	new_size = (sv->size + need_size) * 2;
	if (new_size < sv->size)
		ohshit(_("cannot grow strvec to size %zu; it would overflow"),
		       need_size);

	sv->size = new_size;
	sv->vec = m_realloc(sv->vec, sv->size * sizeof(char *));
}

void
strvec_push(struct strvec *sv, char *str)
{
	strvec_grow(sv, 1);
	sv->vec[sv->used++] = str;
}

char *
strvec_pop(struct strvec *sv)
{
	if (sv->used == 0)
		return NULL;

	return sv->vec[--sv->used];
}

void
strvec_drop(struct strvec *sv)
{
	if (sv->used == 0)
		return;

	free(sv->vec[--sv->used]);
}

const char *
strvec_peek(struct strvec *sv)
{
	if (sv->used == 0)
		return NULL;

	return sv->vec[sv->used - 1];
}

struct strvec *
strvec_split(const char *str, int sep, enum strvec_split_flags flags)
{
	struct strvec *sv;
	size_t ini = 0, end = 0;

	sv = strvec_new(0);

	do {
		if (str[end] == '\0' || str[end] == sep) {
			strvec_push(sv, m_strndup(&str[ini], end - ini));
			if (flags & STRVEC_SPLIT_SKIP_DUP_SEP) {
				while (str[end] == sep && str[end + 1] == sep)
					end++;
			}
			ini = end + 1;
		}
	} while (str[end++]);

	return sv;
}

char *
strvec_join(struct strvec *sv, int sep)
{
	struct varbuf vb = VARBUF_INIT;
	size_t i;

	for (i = 0; i < sv->used; i++) {
		if (i)
			varbuf_add_char(&vb, sep);
		varbuf_add_str(&vb, sv->vec[i]);
	}

	return varbuf_detach(&vb);
}

void
strvec_free(struct strvec *sv)
{
	size_t i;

	for (i = 0; i < sv->used; i++)
		free(sv->vec[i]);
	free(sv->vec);
	free(sv);
}
