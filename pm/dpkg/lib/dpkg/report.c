/*
 * libdpkg - Debian packaging suite library routines
 * report.c - message reporting
 *
 * Copyright © 2004-2005 Scott James Remnant <scott@netsplit.com>
 * Copyright © 2008-2013 Guillem Jover <guillem@debian.org>
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/progname.h>
#include <dpkg/color.h>
#include <dpkg/report.h>

static int piped_mode = _IOLBF;

void
dpkg_set_report_piped_mode(int mode)
{
	piped_mode = mode;
}

void
dpkg_set_report_buffer(FILE *fp)
{
	if (isatty(fileno(fp)))
		setvbuf(fp, NULL, _IONBF, 0);
	else
		setvbuf(fp, NULL, piped_mode, 0);
}

void
dpkg_warning_printer(const char *msg, void *data)
{
	fprintf(stderr, "%s%s:%s %s%s:%s %s\n",
	        color_get(COLOR_PROG), dpkg_get_progname(), color_reset(),
	        color_get(COLOR_WARN), _("warning"), color_reset(), msg);
}

static dpkg_warning_printer_func *warning_printer_func = dpkg_warning_printer;
static void *warning_printer_data;

void
dpkg_set_warning_printer(dpkg_warning_printer_func *printer, void *data)
{
	warning_printer_func = printer;
	warning_printer_data = data;
}

static int warn_count = 0;

int
warning_get_count(void)
{
	return warn_count;
}

void
warningv(const char *fmt, va_list args)
{
	char *buf = NULL;

	warn_count++;

	m_vasprintf(&buf, fmt, args);
	warning_printer_func(buf, warning_printer_data);
	free(buf);
}

void
warning(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	warningv(fmt, args);
	va_end(args);
}

void
notice(const char *fmt, ...)
{
	char *buf = NULL;
	va_list args;

	va_start(args, fmt);
	m_vasprintf(&buf, fmt, args);
	va_end(args);

	fprintf(stderr, "%s%s:%s %s\n",
	        color_get(COLOR_PROG), dpkg_get_progname(), color_reset(), buf);

	free(buf);
}

void
info(const char *fmt, ...)
{
	char *buf;
	va_list args;

	va_start(args, fmt);
	m_vasprintf(&buf, fmt, args);
	va_end(args);

	printf("%s%s:%s %s\n",
	       color_get(COLOR_PROG), dpkg_get_progname(), color_reset(), buf);

	free(buf);
}

static bool hints_enabled = true;

void
dpkg_set_hints_enable(bool enable)
{
	hints_enabled = enable;
}

void
hint(const char *fmt, ...)
{
	char *buf = NULL;
	va_list args;

	if (!hints_enabled)
		return;

	va_start(args, fmt);
	m_vasprintf(&buf, fmt, args);
	va_end(args);

	printf("%s%s:%s %s%s:%s %s\n",
	       color_get(COLOR_PROG), dpkg_get_progname(), color_reset(),
	       color_get(COLOR_HINT), _("hint"), color_reset(), buf);

	free(buf);
}
