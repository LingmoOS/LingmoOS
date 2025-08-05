/*
 * libdpkg - Debian packaging suite library routines
 * fdio.c - safe file descriptor based input/output
 *
 * Copyright © 2009-2010 Guillem Jover <guillem@debian.org>
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
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include <dpkg/fdio.h>
#include <dpkg/ehandle.h>

ssize_t
fd_read(int fd, void *buf, size_t len)
{
	ssize_t total = 0;
	char *ptr = buf;

	if (len > SSIZE_MAX)
		internerr("len=%zu exceeds SSIZE_MAX=%zd", len, SSIZE_MAX);

	while (len > 0) {
		ssize_t n;

		n = read(fd, ptr + total, len);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return total ? -total : n;
		}
		if (n == 0)
			break;

		total += n;
		len -= n;
	}

	return total;
}

ssize_t
fd_write(int fd, const void *buf, size_t len)
{
	ssize_t total = 0;
	const char *ptr = buf;

	if (len > SSIZE_MAX)
		internerr("len=%zu exceeds SSIZE_MAX=%zd", len, SSIZE_MAX);

	while (len > 0) {
		ssize_t n;

		n = write(fd, ptr + total, len);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return total ? -total : n;
		}
		if (n == 0)
			break;

		total += n;
		len -= n;
	}

	return total;
}

#ifdef USE_DISK_PREALLOCATE
#ifdef HAVE_F_PREALLOCATE
static void
fd_preallocate_setup(fstore_t *fs, int flags, off_t offset, off_t len)
{
	fs->fst_flags = flags;
	fs->fst_posmode = F_PEOFPOSMODE;
	fs->fst_offset = offset;
	fs->fst_length = len;
	fs->fst_bytesalloc = 0;
}
#endif

/**
 * Request the kernel to allocate the specified size for a file descriptor.
 *
 * We only want to send a hint that we will be using the requested size. But
 * we do not want to unnecessarily write the file contents. That is why we
 * are not using posix_fallocate(3) directly if possible, and not at all
 * on glibc based systems (except on GNU/kFreeBSD).
 */
int
fd_allocate_size(int fd, off_t offset, off_t len)
{
	int rc;

	/* Do not preallocate on very small files as that degrades performance
	 * on some filesystems. */
	if (len < (4 * 4096) - 1)
		return 0;

#if defined(HAVE_F_PREALLOCATE)
	/* On macOS. */
	fstore_t fs;

	fd_preallocate_setup(&fs, F_ALLOCATECONTIG, offset, len);
	rc = fcntl(fd, F_PREALLOCATE, &fs);
	if (rc < 0 && errno == ENOSPC) {
		/* If we cannot get a contiguous allocation, then try
		 * non-contiguous. */
		fd_preallocate_setup(&fs, F_ALLOCATEALL, offset, len);
		rc = fcntl(fd, F_PREALLOCATE, &fs);
	}
#elif defined(HAVE_F_ALLOCSP64)
	/* On Solaris. */
	struct flock64 fl;

	fl.l_whence = SEEK_SET;
	fl.l_start = offset;
	fl.l_len = len;

	rc = fcntl(fd, F_ALLOCSP64, &fl);
#elif defined(HAVE_FALLOCATE)
	/* On Linux. */
	do {
		rc = fallocate(fd, 0, offset, len);
	} while (rc < 0 && errno == EINTR);
#elif defined(HAVE_POSIX_FALLOCATE) && \
      ((defined(__GLIBC__) && defined(__FreeBSD_kernel__)) || \
       !defined(__GLIBC__))
	/*
	 * On BSDs, newer GNU/kFreeBSD and other non-glibc based systems
	 * we can use posix_fallocate(2) which should be a simple syscall
	 * wrapper. But not on other glibc systems, as there the function
	 * will try to allocate the size by writing a '\0' to each block
	 * if the syscall is not implemented or not supported by the
	 * kernel or the filesystem, which we do not want.
	 */
	rc = posix_fallocate(fd, offset, len);
#else
	errno = ENOSYS;
	rc = -1;
#endif

	return rc;
}
#else
int
fd_allocate_size(int fd, off_t offset, off_t len)
{
	errno = ENOSYS;
	return -1;
}
#endif
