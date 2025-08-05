/*
 * libcompat - system compatibility library
 *
 * Copyright © 2010 Guillem Jover <guillem@debian.org>
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
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat.h"

int
vasprintf(char **strp, char const *fmt, va_list args)
{
	va_list args_copy;
	int needed, n;
	char *str;

	va_copy(args_copy, args);
	needed = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (needed < 0 || needed >= INT_MAX) {
		if (needed >= INT_MAX)
			errno = EOVERFLOW;
		*strp = NULL;
		return -1;
	}

	str = malloc(needed + 1);
	if (str == NULL) {
		*strp = NULL;
		return -1;
	}

	n = vsnprintf(str, needed + 1, fmt, args);
	if (n < 0) {
		free(str);
		str = NULL;
	}

	*strp = str;

	return n;
}
