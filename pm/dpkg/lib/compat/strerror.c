/*
 * libcompat - system compatibility library
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <errno.h>
#include <stdio.h>
#include <gettext.h>

#include "compat.h"

#define _(str) gettext(str)

#if !HAVE_DECL_SYS_ERRLIST
extern const char *const sys_errlist[];
#endif
#if !HAVE_DECL_SYS_NERR
extern const int sys_nerr;
#endif

const char *
strerror(int e)
{
	static char buf[100];

	if (e >= 0 && e < sys_nerr)
		return sys_errlist[e];

	snprintf(buf, sizeof(buf), _("Unknown error %d"), e);

	return buf;
}
