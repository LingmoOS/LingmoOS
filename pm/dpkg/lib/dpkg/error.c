/*
 * libdpkg - Debian packaging suite library routines
 * error.c - error message reporting
 *
 * Copyright © 2011-2015 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <stdlib.h>

#include <dpkg/dpkg.h>
#include <dpkg/varbuf.h>
#include <dpkg/error.h>

static void DPKG_ATTR_VPRINTF(4)
dpkg_error_set(struct dpkg_error *err, enum dpkg_msg_type type, int syserrno,
               const char *fmt, va_list args)
{
	struct varbuf str = VARBUF_INIT;

	if (err == NULL)
		return;

	err->type = type;
	err->syserrno = syserrno;

	varbuf_add_vfmt(&str, fmt, args);
	if (syserrno)
		varbuf_add_fmt(&str, " (%s)", strerror(syserrno));

	err->str = str.buf;
}

bool
dpkg_has_error(struct dpkg_error *err)
{
	return err != NULL && err->type != DPKG_MSG_NONE;
}

int
dpkg_put_warn(struct dpkg_error *err, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	dpkg_error_set(err, DPKG_MSG_WARN, 0, fmt, args);
	va_end(args);

	return -1;
}

int
dpkg_put_error(struct dpkg_error *err, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	dpkg_error_set(err, DPKG_MSG_ERROR, 0, fmt, args);
	va_end(args);

	return -1;
}

int
dpkg_put_errno(struct dpkg_error *err, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	dpkg_error_set(err, DPKG_MSG_ERROR, errno, fmt, args);
	va_end(args);

	return -1;
}

void
dpkg_error_print(struct dpkg_error *err, const char *fmt, ...)
{
	va_list args;
	char *str;

	va_start(args, fmt);
	m_vasprintf(&str, fmt, args);
	va_end(args);

	if (err->type == DPKG_MSG_WARN)
		warning("%s: %s", str, err->str);
	else
		ohshit("%s: %s", str, err->str);

	free(str);
}

void
dpkg_error_move(struct dpkg_error *dst, struct dpkg_error *src)
{
	dst->type = src->type;
	src->type = DPKG_MSG_NONE;
	dst->str = src->str;
	src->str = NULL;
}

void
dpkg_error_destroy(struct dpkg_error *err)
{
	err->type = DPKG_MSG_NONE;
	err->syserrno = 0;
	free(err->str);
	err->str = NULL;
}
