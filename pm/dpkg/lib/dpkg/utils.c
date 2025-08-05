/*
 * libdpkg - Debian packaging suite library routines
 * utils.c - helper functions for dpkg
 *
 * Copyright © 1995, 2008 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>

int
fgets_checked(char *buf, size_t bufsz, FILE *f, const char *fn)
{
	int l;

	if (!fgets(buf, bufsz, f)) {
		if (ferror(f))
			ohshite(_("read error in '%.250s'"), fn);
		return -1;
	}
	l = strlen(buf);
	if (l == 0)
		ohshit(_("fgets gave an empty string from '%.250s'"), fn);
	if (buf[--l] != '\n')
		ohshit(_("too-long line or missing newline in '%.250s'"), fn);
	buf[l] = '\0';

	return l;
}

int
fgets_must(char *buf, size_t bufsz, FILE *f, const char *fn)
{
	int l = fgets_checked(buf, bufsz, f, fn);

	if (l < 0)
		ohshit(_("unexpected end of file reading '%.250s'"), fn);

	return l;
}
