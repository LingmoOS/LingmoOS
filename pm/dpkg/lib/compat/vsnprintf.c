/*
 * libcompat - system compatibility library
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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

#include <sys/types.h>

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#include "compat.h"

int
vsnprintf(char *buf, size_t maxsize, const char *fmt, va_list args)
{
	static FILE *file = NULL;
	static pid_t file_pid;

	size_t want, nr;
	int total;

	if (maxsize != 0 && buf == NULL)
		return -1;

	/* Avoid race conditions from children after a fork(2). */
	if (file_pid > 0 && file_pid != getpid()) {
		fclose(file);
		file = NULL;
	}

	if (!file) {
		file = tmpfile();
		if (!file)
			return -1;
		file_pid = getpid();
	} else {
		if (fseek(file, 0, 0))
			return -1;
		if (ftruncate(fileno(file), 0))
			return -1;
	}

	total = vfprintf(file, fmt, args);
	if (total < 0)
		return -1;
	if (maxsize == 0)
		return total;
	if (total >= (int)maxsize)
		want = maxsize - 1;
	else
		want = total;
	if (fflush(file))
		return -1;
	if (fseek(file, 0, SEEK_SET))
		return -1;

	nr = fread(buf, 1, want, file);
	if (nr != want)
		return -1;
	buf[want] = '\0';

	return total;
}
