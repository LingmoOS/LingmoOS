/*
 * libdpkg - Debian packaging suite library routines
 * program.c - dpkg-based program support
 *
 * Copyright © 2013 Guillem Jover <guillem@debian.org>
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

#include <sys/stat.h>

#include <stdio.h>

#include <dpkg/progname.h>
#include <dpkg/debug.h>
#include <dpkg/report.h>
#include <dpkg/ehandle.h>
#include <dpkg/program.h>

/**
 * Standard initializations when starting a dpkg-based program.
 *
 * @param progname The program name.
 */
void
dpkg_program_init(const char *progname)
{
	dpkg_set_progname(progname);
	dpkg_set_report_buffer(stdout);
	dpkg_debug_init();

	push_error_context();

	/* Set sane default permissions for newly created files. */
	umask(022);
}

/**
 * Standard cleanups before terminating a dpkg-based program.
 */
void
dpkg_program_done(void)
{
	pop_error_context(ehflag_normaltidy);
}
