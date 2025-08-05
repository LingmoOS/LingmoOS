/*
 * libdpkg - Debian packaging suite library routines
 * tarfn.c - tar archive extraction functions
 *
 * Copyright © 1995 Bruce Perens
 * Copyright © 2007-2011, 2013-2017 Guillem Jover <guillem@debian.org>
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

#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#endif
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/error.h>
#include <dpkg/sysuser.h>
#include <dpkg/tarfn.h>

#define TAR_MAGIC_USTAR "ustar\0" "00"
#define TAR_MAGIC_GNU   "ustar "  " \0"

#define TAR_TYPE_SIGNED(t)	(!((t)0 < (t)-1))

#define TAR_TYPE_MIN(t) \
	(TAR_TYPE_SIGNED(t) ? \
	 ~(t)TAR_TYPE_MAX(t) : \
	 (t)0)
#define TAR_TYPE_MAX(t) \
	(TAR_TYPE_SIGNED(t) ? \
	 ((((t)1 << (sizeof(t) * 8 - 2)) - 1) * 2 + 1) : \
	 ~(t)0)

#define TAR_ATOUL(str, type) \
	(type)tar_atoul(str, sizeof(str), TAR_TYPE_MAX(type))
#define TAR_ATOSL(str, type) \
	(type)tar_atosl(str, sizeof(str), TAR_TYPE_MIN(type), TAR_TYPE_MAX(type))

struct tar_header {
	char name[100] DPKG_ATTR_NONSTRING;
	char mode[8] DPKG_ATTR_NONSTRING;
	char uid[8] DPKG_ATTR_NONSTRING;
	char gid[8] DPKG_ATTR_NONSTRING;
	char size[12] DPKG_ATTR_NONSTRING;
	char mtime[12] DPKG_ATTR_NONSTRING;
	char checksum[8] DPKG_ATTR_NONSTRING;
	char linkflag;
	char linkname[100] DPKG_ATTR_NONSTRING;

	/* Only valid on ustar and gnu. */
	char magic[8] DPKG_ATTR_NONSTRING;
	char user[32] DPKG_ATTR_NONSTRING;
	char group[32] DPKG_ATTR_NONSTRING;
	char devmajor[8] DPKG_ATTR_NONSTRING;
	char devminor[8] DPKG_ATTR_NONSTRING;

	/* Only valid on ustar. */
	char prefix[155] DPKG_ATTR_NONSTRING;
};

static inline uintmax_t
tar_ret_errno(int err, uintmax_t ret)
{
	errno = err;
	return ret;
}

/**
 * Convert an ASCII octal string to an intmax_t.
 */
static uintmax_t
tar_atol8(const char *s, size_t size)
{
	const char *end = s + size;
	uintmax_t n = 0;

	/* Old implementations might precede the value with spaces. */
	while (s < end && *s == ' ')
		s++;

	if (s == end)
		return tar_ret_errno(EINVAL, 0);

	while (s < end) {
		if (*s == '\0' || *s == ' ')
			break;
		if (*s < '0' || *s > '7')
			return tar_ret_errno(ERANGE, 0);
		n = (n * 010) + (*s++ - '0');
	}

	while (s < end) {
		if (*s != '\0' && *s != ' ')
			return tar_ret_errno(EINVAL, 0);
		s++;
	}

	return tar_ret_errno(0, n);
}

/**
 * Convert a base-256 two-complement number to an intmax_t.
 */
static uintmax_t
tar_atol256(const char *s, size_t size, intmax_t min, uintmax_t max)
{
	uintmax_t n = 0;
	unsigned char c;
	int sign;

	/* The encoding always sets the first bit to one, so that it can be
	 * distinguished from the ASCII encoding. For positive numbers we
	 * need to reset it. For negative numbers we initialize n to -1. */
	c = *s++;
	if (c == 0x80)
		c = 0;
	else
		n = ~(uintmax_t)0;
	sign = c;

	/* Check for overflows. */
	while (size > sizeof(uintmax_t)) {
		if (c != sign)
			return tar_ret_errno(ERANGE, sign ? (uintmax_t)min : max);
		c = *s++;
		size--;
	}

	if ((c & 0x80) != (sign & 0x80))
		return tar_ret_errno(ERANGE, sign ? (uintmax_t)min : max);

	for (;;) {
		n = (n << 8) | c;
		if (--size == 0)
			break;
		c = *s++;
	}

	return tar_ret_errno(0, n);
}

static uintmax_t
tar_atol(const char *s, size_t size, intmax_t min, uintmax_t max)
{
	const unsigned char *a = (const unsigned char *)s;

	/* Check if it is a long two-complement base-256 number, positive or
	 * negative. */
	if (*a == 0xff || *a == 0x80)
		return tar_atol256(s, size, min, max);
	else
		return tar_atol8(s, size);
}

uintmax_t
tar_atoul(const char *s, size_t size, uintmax_t max)
{
	uintmax_t n = tar_atol(s, size, 0, UINTMAX_MAX);

	if (n > max)
		return tar_ret_errno(ERANGE, UINTMAX_MAX);

	return n;
}

intmax_t
tar_atosl(const char *s, size_t size, intmax_t min, intmax_t max)
{
	intmax_t n = tar_atol(s, size, INTMAX_MIN, INTMAX_MAX);

	if (n < min)
		return tar_ret_errno(ERANGE, INTMAX_MIN);
	if (n > max)
		return tar_ret_errno(ERANGE, INTMAX_MAX);

	return n;
}

static char *
tar_header_get_prefix_name(struct tar_header *h)
{
	struct varbuf path = VARBUF_INIT;

	varbuf_add_strn(&path, h->prefix, sizeof(h->prefix));
	varbuf_add_char(&path, '/');
	varbuf_add_strn(&path, h->name, sizeof(h->name));

	return path.buf;
}

static mode_t
tar_header_get_unix_mode(struct tar_header *h)
{
	mode_t mode;
	enum tar_filetype type;

	type = (enum tar_filetype)h->linkflag;

	switch (type) {
	case TAR_FILETYPE_FILE0:
	case TAR_FILETYPE_FILE:
	case TAR_FILETYPE_HARDLINK:
		mode = S_IFREG;
		break;
	case TAR_FILETYPE_SYMLINK:
		mode = S_IFLNK;
		break;
	case TAR_FILETYPE_DIR:
		mode = S_IFDIR;
		break;
	case TAR_FILETYPE_CHARDEV:
		mode = S_IFCHR;
		break;
	case TAR_FILETYPE_BLOCKDEV:
		mode = S_IFBLK;
		break;
	case TAR_FILETYPE_FIFO:
		mode = S_IFIFO;
		break;
	default:
		mode = 0;
		break;
	}

	mode |= TAR_ATOUL(h->mode, mode_t) & 07777;

	return mode;
}

static long
tar_header_checksum(struct tar_header *h)
{
	unsigned char *s = (unsigned char *)h;
	unsigned int i;
	const size_t checksum_offset = offsetof(struct tar_header, checksum);
	long sum;

	/* Treat checksum field as all blank. */
	sum = ' ' * sizeof(h->checksum);

	for (i = checksum_offset; i > 0; i--)
		sum += *s++;

	/* Skip the real checksum field. */
	s += sizeof(h->checksum);

	for (i = TARBLKSZ - checksum_offset - sizeof(h->checksum); i > 0; i--)
		sum += *s++;

	return sum;
}

static int
tar_header_decode(struct tar_header *h, struct tar_entry *d, struct dpkg_error *err)
{
	long checksum;

	errno = 0;

	if (memcmp(h->magic, TAR_MAGIC_GNU, 6) == 0)
		d->format = TAR_FORMAT_GNU;
	else if (memcmp(h->magic, TAR_MAGIC_USTAR, 6) == 0)
		d->format = TAR_FORMAT_USTAR;
	else
		d->format = TAR_FORMAT_OLD;

	d->type = (enum tar_filetype)h->linkflag;
	if (d->type == TAR_FILETYPE_FILE0)
		d->type = TAR_FILETYPE_FILE;

	/* Concatenate prefix and name to support ustar style long names. */
	if (d->format == TAR_FORMAT_USTAR && h->prefix[0] != '\0')
		d->name = tar_header_get_prefix_name(h);
	else
		d->name = m_strndup(h->name, sizeof(h->name));
	d->linkname = m_strndup(h->linkname, sizeof(h->linkname));
	d->stat.mode = tar_header_get_unix_mode(h);
	/* Even though off_t is signed, we use an unsigned parser here because
	 * negative offsets are not allowed. */
	d->size = TAR_ATOUL(h->size, off_t);
	if (errno)
		return dpkg_put_errno(err, _("invalid tar header size field"));
	d->mtime = TAR_ATOSL(h->mtime, time_t);
	if (errno)
		return dpkg_put_errno(err, _("invalid tar header mtime field"));

	if (d->type == TAR_FILETYPE_CHARDEV || d->type == TAR_FILETYPE_BLOCKDEV)
		d->dev = makedev(TAR_ATOUL(h->devmajor, dev_t),
		                 TAR_ATOUL(h->devminor, dev_t));
	else
		d->dev = makedev(0, 0);

	if (*h->user)
		d->stat.uname = m_strndup(h->user, sizeof(h->user));
	else
		d->stat.uname = NULL;
	d->stat.uid = TAR_ATOUL(h->uid, uid_t);
	if (errno)
		return dpkg_put_errno(err, _("invalid tar header uid field"));

	if (*h->group)
		d->stat.gname = m_strndup(h->group, sizeof(h->group));
	else
		d->stat.gname = NULL;
	d->stat.gid = TAR_ATOUL(h->gid, gid_t);
	if (errno)
		return dpkg_put_errno(err, _("invalid tar header gid field"));

	checksum = tar_atol8(h->checksum, sizeof(h->checksum));
	if (errno)
		return dpkg_put_errno(err, _("invalid tar header checksum field"));

	if (tar_header_checksum(h) != checksum)
		return dpkg_put_error(err, _("invalid tar header checksum"));

	return 0;
}

/**
 * Decode a GNU longlink or longname from the tar archive.
 *
 * The way the GNU long{link,name} stuff works is like this:
 *
 * - The first header is a “dummy” header that contains the size of the
 *   filename (GNU tar includes the terminating NUL character in the size,
 *   but other implementations do not).
 * - The next N headers contain the filename (GNU tar terminates the string
 *   with a NUL character, but other implementations do not).
 * - After the headers with the filename comes the “real” header with a
 *   bogus name or link.
 *
 * To be robust against any input, we need to always terminate the filename
 * with a NUL character.
 */
static int
tar_gnu_long(struct tar_archive *tar, struct tar_entry *te, char **longp)
{
	char buf[TARBLKSZ];
	char *bp;
	int status = 0;
	int long_read;

	free(*longp);
	*longp = bp = m_malloc(te->size + 1);

	for (long_read = te->size; long_read > 0; long_read -= TARBLKSZ) {
		int copysize;

		status = tar->ops->read(tar, buf, TARBLKSZ);
		if (status == TARBLKSZ)
			status = 0;
		else {
			/* Read partial header record? */
			if (status > 0) {
				errno = 0;
				status = dpkg_put_error(&tar->err,
				                        _("partially read tar header"));
			}

			/* If we didn't get TARBLKSZ bytes read, punt. */
			break;
		}

		copysize = min(long_read, TARBLKSZ);
		memcpy(bp, buf, copysize);
		bp += copysize;
	}
	*bp = '\0';

	return status;
}

static void
tar_entry_copy(struct tar_entry *dst, struct tar_entry *src)
{
	memcpy(dst, src, sizeof(struct tar_entry));

	dst->name = m_strdup(src->name);
	dst->linkname = m_strdup(src->linkname);

	if (src->stat.uname)
		dst->stat.uname = m_strdup(src->stat.uname);
	if (src->stat.gname)
		dst->stat.gname = m_strdup(src->stat.gname);
}

static void
tar_entry_destroy(struct tar_entry *te)
{
	free(te->name);
	free(te->linkname);
	free(te->stat.uname);
	free(te->stat.gname);

	memset(te, 0, sizeof(*te));
}

struct tar_symlink_entry {
	struct tar_symlink_entry *next;
	struct tar_entry h;
};

/**
 * Update the tar entry from system information.
 *
 * Normalize UID and GID relative to the current system.
 */
void
tar_entry_update_from_system(struct tar_entry *te)
{
	struct passwd *passwd;
	struct group *group;

	if (te->stat.uname) {
		passwd = dpkg_sysuser_from_name(te->stat.uname);
		if (passwd)
			te->stat.uid = passwd->pw_uid;
	}
	if (te->stat.gname) {
		group = dpkg_sysgroup_from_name(te->stat.gname);
		if (group)
			te->stat.gid = group->gr_gid;
	}
}

int
tar_extractor(struct tar_archive *tar)
{
	int status;
	char buffer[TARBLKSZ];
	struct tar_entry h;

	char *next_long_name, *next_long_link;
	struct tar_symlink_entry *symlink_head, *symlink_tail, *symlink_node;

	next_long_name = NULL;
	next_long_link = NULL;
	symlink_tail = symlink_head = NULL;

	h.name = NULL;
	h.linkname = NULL;
	h.stat.uname = NULL;
	h.stat.gname = NULL;

	while ((status = tar->ops->read(tar, buffer, TARBLKSZ)) == TARBLKSZ) {
		int name_len;

		if (tar_header_decode((struct tar_header *)buffer, &h, &tar->err) < 0) {
			if (h.name[0] == '\0') {
				/* The checksum failed on the terminating
				 * End Of Tape block entry of zeros. */
				dpkg_error_destroy(&tar->err);

				/* End Of Tape. */
				status = 0;
			} else {
				status = -1;
			}
			tar_entry_destroy(&h);
			break;
		}
		if (h.type != TAR_FILETYPE_GNU_LONGLINK &&
		    h.type != TAR_FILETYPE_GNU_LONGNAME) {
			if (next_long_name) {
				free(h.name);
				h.name = next_long_name;
			}

			if (next_long_link) {
				free(h.linkname);
				h.linkname = next_long_link;
			}

			next_long_link = NULL;
			next_long_name = NULL;
		}

		if (h.name[0] == '\0') {
			status = dpkg_put_error(&tar->err,
			                        _("invalid tar header with empty name field"));
			errno = 0;
			tar_entry_destroy(&h);
			break;
		}

		name_len = strlen(h.name);

		switch (h.type) {
		case TAR_FILETYPE_FILE:
			/* Compatibility with pre-ANSI ustar. */
			if (h.name[name_len - 1] != '/') {
				status = tar->ops->extract_file(tar, &h);
				break;
			}
			/* Else, fall through. */
		case TAR_FILETYPE_DIR:
			if (h.name[name_len - 1] == '/') {
				h.name[name_len - 1] = '\0';
			}
			status = tar->ops->mkdir(tar, &h);
			break;
		case TAR_FILETYPE_HARDLINK:
			status = tar->ops->link(tar, &h);
			break;
		case TAR_FILETYPE_SYMLINK:
			symlink_node = m_malloc(sizeof(*symlink_node));
			symlink_node->next = NULL;
			tar_entry_copy(&symlink_node->h, &h);

			if (symlink_head)
				symlink_tail->next = symlink_node;
			else
				symlink_head = symlink_node;
			symlink_tail = symlink_node;
			status = 0;
			break;
		case TAR_FILETYPE_CHARDEV:
		case TAR_FILETYPE_BLOCKDEV:
		case TAR_FILETYPE_FIFO:
			status = tar->ops->mknod(tar, &h);
			break;
		case TAR_FILETYPE_GNU_LONGLINK:
			status = tar_gnu_long(tar, &h, &next_long_link);
			break;
		case TAR_FILETYPE_GNU_LONGNAME:
			status = tar_gnu_long(tar, &h, &next_long_name);
			break;
		case TAR_FILETYPE_GNU_VOLUME:
		case TAR_FILETYPE_GNU_MULTIVOL:
		case TAR_FILETYPE_GNU_SPARSE:
		case TAR_FILETYPE_GNU_DUMPDIR:
			status = dpkg_put_error(&tar->err,
			                        _("unsupported GNU tar header type '%c'"),
			                        h.type);
			errno = 0;
			break;
		case TAR_FILETYPE_SOLARIS_EXTENDED:
		case TAR_FILETYPE_SOLARIS_ACL:
			status = dpkg_put_error(&tar->err,
			                        _("unsupported Solaris tar header type '%c'"),
			                        h.type);
			errno = 0;
			break;
		case TAR_FILETYPE_PAX_GLOBAL:
		case TAR_FILETYPE_PAX_EXTENDED:
			status = dpkg_put_error(&tar->err,
			                        _("unsupported PAX tar header type '%c'"),
			                        h.type);
			errno = 0;
			break;
		default:
			status = dpkg_put_error(&tar->err,
			                        _("unknown tar header type '%c'"),
			                        h.type);
			errno = 0;
		}
		tar_entry_destroy(&h);
		if (status != 0)
			/* Pass on status from coroutine. */
			break;
	}

	while (symlink_head) {
		symlink_node = symlink_head->next;
		if (status == 0)
			status = tar->ops->symlink(tar, &symlink_head->h);
		tar_entry_destroy(&symlink_head->h);
		free(symlink_head);
		symlink_head = symlink_node;
	}
	/* Make sure we free the long names, in case of a bogus or truncated
	 * tar archive with long entries not followed by a normal entry. */
	free(next_long_name);
	free(next_long_link);

	if (status > 0) {
		status = dpkg_put_error(&tar->err,
		                        _("partially read tar header"));
		errno = 0;
	}

	/* Return whatever I/O function returned. */
	return status;
}
