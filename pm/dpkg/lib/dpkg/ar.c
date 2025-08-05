/*
 * libdpkg - Debian packaging suite library routines
 * ar.c - primitives for ar handling
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
#include <compat.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/ar.h>

struct dpkg_ar *
dpkg_ar_fdopen(const char *filename, int fd)
{
	struct dpkg_ar *ar;
	struct stat st;

	if (fstat(fd, &st) != 0)
		ohshite(_("failed to fstat archive"));

	ar = m_malloc(sizeof(*ar));
	ar->name = filename;
	ar->mode = st.st_mode;
	ar->size = st.st_size;
	ar->time = st.st_mtime;
	ar->fd = fd;

	return ar;
}

struct dpkg_ar *
dpkg_ar_open(const char *filename)
{
	int fd;

	if (strcmp(filename, "-") == 0)
		fd = STDIN_FILENO;
	else
		fd = open(filename, O_RDONLY);
	if (fd < 0)
		ohshite(_("failed to read archive '%.255s'"), filename);

	return dpkg_ar_fdopen(filename, fd);
}

struct dpkg_ar *
dpkg_ar_create(const char *filename, mode_t mode)
{
	int fd;

	fd = creat(filename, mode);
	if (fd < 0)
		ohshite(_("unable to create '%.255s'"), filename);

	return dpkg_ar_fdopen(filename, fd);
}

void
dpkg_ar_set_mtime(struct dpkg_ar *ar, intmax_t mtime)
{
	ar->time = mtime;
}

void
dpkg_ar_close(struct dpkg_ar *ar)
{
	if (close(ar->fd))
		ohshite(_("unable to close file '%s'"), ar->name);
	free(ar);
}

static void
dpkg_ar_member_init(struct dpkg_ar *ar, struct dpkg_ar_member *member,
                    const char *name, off_t size)
{
	member->name = name;
	member->size = size;
	member->time = ar->time;
	member->mode = 0100644;
	member->uid = 0;
	member->gid = 0;
}

void
dpkg_ar_normalize_name(struct dpkg_ar_hdr *arh)
{
	char *name = arh->ar_name;
	int i;

	/* Remove trailing spaces from the member name. */
	for (i = sizeof(arh->ar_name) - 1; i >= 0 && name[i] == ' '; i--)
		name[i] = '\0';

	/* Remove optional slash terminator (on GNU-style archives). */
	if (i >= 0 && name[i] == '/')
		name[i] = '\0';
}

off_t
dpkg_ar_member_get_size(struct dpkg_ar *ar, struct dpkg_ar_hdr *arh)
{
	const char *str = arh->ar_size;
	int len = sizeof(arh->ar_size);
	off_t size = 0;

	while (len && *str == ' ')
		str++, len--;

	while (len--) {
		if (*str == ' ')
			break;
		if (*str < '0' || *str > '9')
			ohshit(_("invalid character '%c' in archive '%.250s' "
			         "member '%.16s' size"),
			       *str, ar->name, arh->ar_name);

		size *= 10;
		size += *str++ - '0';
	}

	return size;
}

bool
dpkg_ar_member_is_illegal(struct dpkg_ar_hdr *arh)
{
	return memcmp(arh->ar_fmag, DPKG_AR_FMAG, sizeof(arh->ar_fmag)) != 0;
}

void
dpkg_ar_put_magic(struct dpkg_ar *ar)
{
	if (fd_write(ar->fd, DPKG_AR_MAGIC, strlen(DPKG_AR_MAGIC)) < 0)
		ohshite(_("unable to write file '%s'"), ar->name);
}

void
dpkg_ar_member_put_header(struct dpkg_ar *ar, struct dpkg_ar_member *member)
{
	char header[sizeof(struct dpkg_ar_hdr) + 1];
	int n;

	if (strlen(member->name) > 15)
		ohshit(_("ar member name '%s' length too long"), member->name);
	if (member->size > 9999999999L)
		ohshit(_("ar member size %jd too large"), (intmax_t)member->size);
	if (member->time > 999999999999L)
		ohshit(_("ar member time %jd too large"), (intmax_t)member->time);

	n = snprintf(header, sizeof(struct dpkg_ar_hdr) + 1,
	             "%-16s%-12jd%-6lu%-6lu%-8lo%-10jd`\n",
	             member->name, (intmax_t)member->time,
	             (unsigned long)member->uid, (unsigned long)member->gid,
	             (unsigned long)member->mode, (intmax_t)member->size);
	if (n != sizeof(struct dpkg_ar_hdr))
		ohshit(_("generated corrupt ar header for '%s'"), ar->name);

	if (fd_write(ar->fd, header, n) < 0)
		ohshite(_("unable to write file '%s'"), ar->name);
}

void
dpkg_ar_member_put_mem(struct dpkg_ar *ar,
                       const char *name, const void *data, size_t size)
{
	struct dpkg_ar_member member;

	dpkg_ar_member_init(ar, &member, name, size);
	dpkg_ar_member_put_header(ar, &member);

	/* Copy data contents. */
	if (fd_write(ar->fd, data, size) < 0)
		ohshite(_("unable to write file '%s'"), ar->name);

	if (size & 1)
		if (fd_write(ar->fd, "\n", 1) < 0)
			ohshite(_("unable to write file '%s'"), ar->name);
}

void
dpkg_ar_member_put_file(struct dpkg_ar *ar,
                        const char *name, int fd, off_t size)
{
	struct dpkg_error err;
	struct dpkg_ar_member member;

	if (size <= 0) {
		struct stat st;

		if (fstat(fd, &st))
			ohshite(_("failed to fstat ar member file (%s)"), name);
		size = st.st_size;
	}

	dpkg_ar_member_init(ar, &member, name, size);
	dpkg_ar_member_put_header(ar, &member);

	/* Copy data contents. */
	if (fd_fd_copy(fd, ar->fd, size, &err) < 0)
		ohshit(_("cannot append ar member file (%s) to '%s': %s"),
		       name, ar->name, err.str);

	if (size & 1)
		if (fd_write(ar->fd, "\n", 1) < 0)
			ohshite(_("unable to write file '%s'"), ar->name);
}
