/*
 * libdpkg - Debian packaging suite library routines
 * namevalue.c - name value structure handling
 *
 * Copyright © 2010-2011, 2014-2015 Guillem Jover <guillem@debian.org>
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

#include <stddef.h>
#include <string.h>

#include <dpkg/namevalue.h>

const struct namevalue *
namevalue_find_by_name(const struct namevalue *head, const char *str)
{
	const struct namevalue *nv;

	for (nv = head; nv->name; nv++) {
		if (strncasecmp(str, nv->name, nv->length) == 0)
			return nv;
	}

	return NULL;
}
