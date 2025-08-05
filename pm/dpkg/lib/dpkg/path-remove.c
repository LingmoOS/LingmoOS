/*
 * libdpkg - Debian packaging suite library routines
 * path-remove.c - path removal functions
 *
 * Copyright © 1994-1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2007-2015 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/path.h>
#include <dpkg/debug.h>
#include <dpkg/subproc.h>

int
secure_unlink_statted(const char *pathname, const struct stat *stab)
{
	mode_t mode = stab->st_mode;

	if (S_ISREG(mode) ? (mode & 07000) :
	    !(S_ISLNK(mode) || S_ISDIR(mode) ||
	      S_ISFIFO(mode) || S_ISSOCK(mode))) {
		if (chmod(pathname, 0600))
			return -1;
	}

	if (unlink(pathname))
		return -1;

	return 0;
}

/**
 * Securely unlink a pathname.
 *
 * If the pathname to remove is:
 *
 * 1. a sticky or set-id file, or
 * 2. an unknown object (i.e., not a file, link, directory, fifo or socket)
 *
 * we change its mode so that a malicious user cannot use it, even if it's
 * linked to another file.
 */
int
secure_unlink(const char *pathname)
{
	struct stat stab;

	if (lstat(pathname, &stab))
		return -1;

	return secure_unlink_statted(pathname, &stab);
}

/**
 * Securely remove a pathname.
 *
 * This is a secure version of remove(3) using secure_unlink() instead of
 * unlink(2).
 *
 * @retval  0 On success.
 * @retval -1 On failure, just like unlink(2) & rmdir(2).
 */
int
secure_remove(const char *pathname)
{
	int rc, e;

	if (!rmdir(pathname)) {
		debug(dbg_eachfiledetail, "secure_remove '%s' rmdir OK",
		      pathname);
		return 0;
	}

	if (errno != ENOTDIR) {
		e = errno;
		debug(dbg_eachfiledetail, "secure_remove '%s' rmdir %s",
		      pathname, strerror(e));
		errno = e;
		return -1;
	}

	rc = secure_unlink(pathname);
	e = errno;
	debug(dbg_eachfiledetail, "secure_remove '%s' unlink %s",
	      pathname, rc ? strerror(e) : "OK");
	errno = e;

	return rc;
}

/**
 * Remove a pathname and anything below it.
 *
 * This function removes pathname and all its contents recursively.
 */
void
path_remove_tree(const char *pathname)
{
	pid_t pid;
	const char *u;

	u = path_skip_slash_dotslash(pathname);
	if (u[0] == '\0')
		internerr("pathname '%s' reduces to nothing", pathname);

	debug(dbg_eachfile, "%s '%s'", __func__, pathname);
	if (!rmdir(pathname))
		return; /* Deleted it OK, it was a directory. */
	if (errno == ENOENT || errno == ELOOP)
		return;
	if (errno == ENOTDIR) {
		/* Either it's a file, or one of the path components is. If
		 * one of the path components is this will fail again ... */
		if (secure_unlink(pathname) == 0)
			return; /* OK, it was. */
		if (errno == ENOTDIR)
			return;
	}
	/* Trying to remove a directory or a file on a read-only filesystem,
	 * even if non-existent, always returns EROFS. */
	if (errno == EROFS) {
		if (access(pathname, F_OK) < 0 && errno == ENOENT)
			return;
		errno = EROFS;
	}
	if (errno != ENOTEMPTY && errno != EEXIST) /* Huh? */
		ohshite(_("unable to securely remove '%.255s'"), pathname);

	pid = subproc_fork();
	if (pid == 0) {
		execlp(RM, "rm", "-rf", "--", pathname, NULL);
		ohshite(_("unable to execute %s (%s)"),
		        _("rm command for cleanup"), RM);
	}
	debug(dbg_eachfile, "%s running rm -rf '%s'", __func__, pathname);
	subproc_reap(pid, _("rm command for cleanup"), 0);
}
