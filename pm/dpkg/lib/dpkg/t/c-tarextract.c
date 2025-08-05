/*
 * libdpkg - Debian packaging suite library routines
 * c-tarextract.c - test tar extractor
 *
 * Copyright © 2014 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/types.h>
#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h> /* Needed on AIX for major()/minor(). */
#endif
#ifdef HAVE_SYS_MKDEV_H
#include <sys/mkdev.h> /* Needed on Solaris for major()/minor(). */
#endif

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/ehandle.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/tarfn.h>

struct tar_context {
	int tar_fd;
};

static int
tar_read(struct tar_archive *tar, char *buffer, int size)
{
	struct tar_context *tc = tar->ctx;

	return fd_read(tc->tar_fd, buffer, size);
}

static int
tar_object_skip(struct tar_archive *tar, struct tar_entry *te)
{
	struct tar_context *tc = tar->ctx;
	off_t size;

	size = (te->size + TARBLKSZ - 1) / TARBLKSZ * TARBLKSZ;
	if (size == 0)
		return 0;

	return fd_skip(tc->tar_fd, size, NULL);
}

static int
tar_object(struct tar_archive *tar, struct tar_entry *te)
{
	printf("%s mode=%o time=%jd.%.9d uid=%d gid=%d", te->name,
	       te->stat.mode, te->mtime, 0, te->stat.uid, te->stat.gid);
	if (te->stat.uname)
		printf(" uname=%s", te->stat.uname);
	if (te->stat.gname)
		printf(" gname=%s", te->stat.gname);

	switch (te->type) {
	case TAR_FILETYPE_FILE0:
	case TAR_FILETYPE_FILE:
		tar_object_skip(tar, te);
		printf(" type=file size=%jd", (intmax_t)te->size);
		break;
	case TAR_FILETYPE_HARDLINK:
		printf(" type=hardlink linkto=%s size=%jd",
		       te->linkname, (intmax_t)te->size);
		break;
	case TAR_FILETYPE_SYMLINK:
		printf(" type=symlink linkto=%s size=%jd",
		       te->linkname, (intmax_t)te->size);
		break;
	case TAR_FILETYPE_DIR:
		printf(" type=dir");
		break;
	case TAR_FILETYPE_CHARDEV:
	case TAR_FILETYPE_BLOCKDEV:
		printf(" type=device id=%d.%d", major(te->dev), minor(te->dev));
		break;
	case TAR_FILETYPE_FIFO:
		printf(" type=fifo");
		break;
	default:
		ohshit("unexpected tar entry type '%c'", te->type);
	}

	printf("\n");

	return 0;
}

static struct tar_operations tar_ops = {
	.read = tar_read,
	.extract_file = tar_object,
	.link = tar_object,
	.symlink = tar_object,
	.mkdir = tar_object,
	.mknod = tar_object,
};

int
main(int argc, char **argv)
{
	struct tar_archive tar;
	struct tar_context tar_ctx;
	const char *tar_name = argv[1];

	setvbuf(stdout, NULL, _IOLBF, 0);

	push_error_context();

	if (tar_name) {
		tar_ctx.tar_fd = open(tar_name, O_RDONLY);
		if (tar_ctx.tar_fd < 0)
			ohshite("cannot open file '%s'", tar_name);
	} else {
		tar_ctx.tar_fd = STDIN_FILENO;
	}

	tar.err = DPKG_ERROR_OBJECT;
	tar.ctx = &tar_ctx;
	tar.ops = &tar_ops;

	if (tar_extractor(&tar))
		ohshite("extracting tar");

	dpkg_error_destroy(&tar.err);

	if (tar_name)
		close(tar_ctx.tar_fd);

	pop_error_context(ehflag_normaltidy);

	return 0;
}
