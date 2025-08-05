/*
 * libdpkg - Debian packaging suite library routines
 * trigname.c - trigger name handling
 *
 * Copyright © 2007 Canonical Ltd
 * Written by Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <dpkg/i18n.h>
#include <dpkg/triglib.h>

const char *
trig_name_is_illegal(const char *p)
{
	int c;

	if (!*p)
		return _("empty trigger names are not permitted");

	while ((c = *p++)) {
		if (c <= ' ' || c >= 0177)
			return _("trigger name contains invalid character");
	}

	return NULL;
}
