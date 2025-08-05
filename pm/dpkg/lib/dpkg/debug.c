/*
 * libdpkg - Debian packaging suite library routines
 * debug.c - debugging support
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2011 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/report.h>
#include <dpkg/debug.h>

static int debug_mask = 0;
static FILE *debug_output = NULL;

/**
 * Set the debugging output file.
 *
 * Marks the file descriptor as close-on-exec.
 */
void
debug_set_output(FILE *output, const char *filename)
{
	setcloexec(fileno(output), filename);
	dpkg_set_report_buffer(output);
	debug_output = output;
}

/**
 * Set the debugging mask.
 *
 * The mask determines what debugging flags are going to take effect at
 * run-time. The output will be set to stderr if it has not been set before.
 */
void
debug_set_mask(int mask)
{
	debug_mask = mask;
	if (!debug_output)
		debug_output = stderr;
}

/**
 * Parse the debugging mask.
 *
 * The mask is parsed from the specified string and sets the global debugging
 * mask. If there is any error while parsing a negative number is returned.
 */
int
debug_parse_mask(const char *str)
{
	char *endp;
	long mask;

	errno = 0;
	mask = strtol(str, &endp, 8);
	if (str == endp || *endp || mask < 0 || errno == ERANGE)
		return -1;

	debug_set_mask(mask);

	return mask;
}

/**
 * Check if a debugging flag is currently set on the debugging mask.
 */
bool
debug_has_flag(int flag)
{
	return debug_mask & flag;
}

/**
 * Output a debugging message.
 *
 * The message will be printed to the previously specified output if the
 * specified flag is present in the current debugging mask.
 */
void
debug(int flag, const char *fmt, ...)
{
	va_list args;

	if (!debug_has_flag(flag))
		return;

	fprintf(debug_output, "D0%05o: ", flag);
	va_start(args, fmt);
	vfprintf(debug_output, fmt, args);
	va_end(args);
	putc('\n', debug_output);
}

/**
 * Initialize the debugging support.
 */
void
dpkg_debug_init(void)
{
	const char envvar[] = "DPKG_DEBUG";
	const char *env;

	env = getenv(envvar);
	if (env == NULL)
		return;

	if (debug_parse_mask(env) < 0)
		warning(_("cannot parse debug mask from environment variable %s"),
		        envvar);
}
