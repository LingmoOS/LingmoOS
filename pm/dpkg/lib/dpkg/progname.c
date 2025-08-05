/*
 * libdpkg - Debian packaging suite library routines
 * progname.c - program name handling functions
 *
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

#ifdef HAVE_GETPROCS64
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <procinfo.h>
#endif
#include <errno.h>
#include <stdlib.h>

#include <dpkg/path.h>
#include <dpkg/progname.h>

static const char *progname;

/**
 * Set the program name.
 *
 * This function will set the program name which will be used by error
 * reporting functions, among others.
 *
 * @param name The new program name.
 */
void
dpkg_set_progname(const char *name)
{
	progname = path_basename(name);
}

#if defined(HAVE___PROGNAME)
extern const char *__progname;
#endif

/**
 * Get the program name.
 *
 * The program name might have been set previously by dpkg_set_progname(),
 * but if not this function will try to initialize it by system dependent
 * means, so it's half safe to not call dpkg_set_progname() at all. At worst
 * the function might return NULL in that case.
 *
 * @return A pointer to a static buffer with the program name.
 */
const char *
dpkg_get_progname(void)
{
	if (progname == NULL) {
#if defined(HAVE_PROGRAM_INVOCATION_SHORT_NAME)
		progname = program_invocation_short_name;
#elif defined(HAVE___PROGNAME)
		progname = __progname;
#elif defined(HAVE_GETPROGNAME)
		progname = getprogname();
#elif defined(HAVE_GETEXECNAME)
		/* getexecname(3) returns an absolute path, normalize it. */
		dpkg_set_progname(getexecname());
#elif defined(HAVE_GETPROCS64)
		struct procentry64 pe;
		pid_t pid = getpid();

		if (getprocs64(&pe, sizeof(pe), NULL, 0, &pid, 1) >= 0)
			progname = strdup(pe.pi_comm);
#endif
	}

	return progname;
}
