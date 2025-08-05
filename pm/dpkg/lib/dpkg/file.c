/*
 * libdpkg - Debian packaging suite library routines
 * file.c - file handling functions
 *
 * Copyright © 1994, 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/pager.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/path.h>
#include <dpkg/file.h>
#include <dpkg/execname.h>

/**
 * Get the current working directory.
 *
 */
void
file_getcwd(struct varbuf *cwd)
{
	varbuf_reset(cwd);
	varbuf_grow(cwd, 64);
	while (getcwd(cwd->buf, cwd->size) == NULL)
		varbuf_grow(cwd, cwd->size * 2);
	varbuf_trunc(cwd, strlen(cwd->buf));
}

/*
 * Handle pre-POSIX-1.2008 realpath() semantics, by using a fixed size buffer
 * based on PATH_MAX, which we expect to be defined on the systems that have
 * no proper behavior for this function.
 */
#ifdef PATH_MAX
static char *
file_realpath_legacy(const char *pathname)
{
	char resolved_path_buf[PATH_MAX];
	char *resolved_path;

	resolved_path = realpath(pathname, resolved_path_buf);
	if (resolved_path == NULL && errno != ENOENT)
		ohshite(_("cannot canonicalize pathname %s"), pathname);

	if (resolved_path)
		return m_strdup(resolved_path);
	return NULL;
}
#endif

char *
file_realpath(const char *pathname)
{
	char *resolved_path;

	resolved_path = realpath(pathname, NULL);
	if (resolved_path == NULL && errno != ENOENT) {
#ifdef PATH_MAX
		if (errno == EINVAL)
			return file_realpath_legacy(pathname);
#endif
		ohshite(_("cannot canonicalize pathname %s"), pathname);
	}

	return resolved_path;
}

/**
 * Canonicalize a pathname (physically or lexically).
 *
 * Try to canonicalize a pathname based on what is on the filesystem. If
 * the pathname does not exist, then try a lexical canonicalization.
 *
 * @param pathname The pathname to canonicalize.
 *
 * @return The allocated canonicalized pathname.
 */
char *
file_canonicalize(const char *pathname)
{
	char *canon_path;

	errno = 0;
	canon_path = file_realpath(pathname);
	if (canon_path == NULL)
		canon_path = path_canonicalize(pathname);

	return canon_path;
}

/**
 * Read the symlink content into a varbuf.
 *
 */
ssize_t
file_readlink(const char *slink, struct varbuf *content, size_t content_len)
{
	ssize_t linksize;

	varbuf_reset(content);
	varbuf_grow(content, content_len + 1);

	linksize = readlink(slink, content->buf, content->size);
	if (linksize < 0)
		return linksize;

	varbuf_trunc(content, linksize);

	return linksize;
}

/**
 * Check whether a filename is executable.
 *
 * @param filename The filename to check.
 */
bool
file_is_exec(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) < 0)
		return false;

	if (!S_ISREG(st.st_mode))
		return false;

	return st.st_mode & 0111;
}

/**
 * Copy file ownership and permissions from one file to another.
 *
 * @param src The source filename.
 * @param dst The destination filename.
 */
void
file_copy_perms(const char *src, const char *dst)
{
	struct stat stab;

	if (stat(src, &stab) < 0) {
		if (errno == ENOENT)
			return;
		ohshite(_("unable to stat source file '%.250s'"), src);
	}

	if (chown(dst, stab.st_uid, stab.st_gid) < 0)
		ohshite(_("unable to change ownership of target file '%.250s'"),
		        dst);

	if (chmod(dst, (stab.st_mode & ~S_IFMT)) < 0)
		ohshite(_("unable to set mode of target file '%.250s'"), dst);
}

static int
file_slurp_fd(int fd, const char *filename, struct varbuf *vb,
              struct dpkg_error *err)
{
	struct stat st;

	if (fstat(fd, &st) < 0)
		return dpkg_put_errno(err, _("cannot stat %s"), filename);

	if (!S_ISREG(st.st_mode))
		return dpkg_put_error(err, _("%s is not a regular file"),
		                      filename);

	if (st.st_size == 0)
		return 0;

	varbuf_init(vb, st.st_size + 1);
	if (fd_read(fd, vb->buf, st.st_size) < 0)
		return dpkg_put_errno(err, _("cannot read %s"), filename);
	varbuf_trunc(vb, st.st_size);

	return 0;
}

int
file_slurp(const char *filename, struct varbuf *vb, struct dpkg_error *err)
{
	int fd;
	int rc;

	varbuf_init(vb, 0);

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return dpkg_put_errno(err, _("cannot open %s"), filename);

	rc = file_slurp_fd(fd, filename, vb, err);

	(void)close(fd);

	return rc;
}

static void
file_lock_setup(struct flock *fl, short type)
{
	fl->l_type = type;
	fl->l_whence = SEEK_SET;
	fl->l_start = 0;
	fl->l_len = 0;
	fl->l_pid = 0;
}

/**
 * Unlock a previously locked file.
 */
void
file_unlock(int lockfd, const char *lockfile, const char *lockdesc)
{
	struct flock fl;

	if (lockfd < 0)
		internerr("%s (%s) fd is %d < 0", lockdesc, lockfile, lockfd);

	file_lock_setup(&fl, F_UNLCK);

	if (fcntl(lockfd, F_SETLK, &fl) < 0)
		ohshite(_("unable to unlock %s"), lockdesc);
}

static void
file_unlock_cleanup(int argc, void **argv)
{
	int lockfd = *(int *)argv[0];
	const char *lockfile = argv[1];
	const char *lockdesc = argv[2];

	file_unlock(lockfd, lockfile, lockdesc);
}

/**
 * Check if a file has a lock acquired.
 *
 * @param lockfd The file descriptor for the lock.
 * @param filename The file name associated to the file descriptor.
 */
bool
file_is_locked(int lockfd, const char *filename)
{
	struct flock fl;

	file_lock_setup(&fl, F_WRLCK);

	if (fcntl(lockfd, F_GETLK, &fl) < 0)
		ohshit(_("unable to check file '%s' lock status"), filename);

	if (fl.l_type == F_WRLCK && fl.l_pid != getpid())
		return true;
	else
		return false;
}

/**
 * Lock a file.
 *
 * @param lockfd The pointer to the lock file descriptor. It must be allocated
 *        statically as its addresses is passed to a cleanup handler.
 * @param flags The lock flags specifying what type of locking to perform.
 * @param filename The name of the file to lock.
 * @param desc The description of the file to lock.
 */
void
file_lock(int *lockfd, enum file_lock_flags flags, const char *filename,
          const char *desc)
{
	struct flock fl;
	int lock_cmd;

	setcloexec(*lockfd, filename);

	file_lock_setup(&fl, F_WRLCK);

	if (flags == FILE_LOCK_WAIT)
		lock_cmd = F_SETLKW;
	else
		lock_cmd = F_SETLK;

	if (fcntl(*lockfd, lock_cmd, &fl) < 0) {
		const char *warnmsg;
		char *execname;

		if (errno != EACCES && errno != EAGAIN)
			ohshite(_("unable to lock %s"), desc);

		warnmsg = _("Note: removing the lock file is always wrong, "
		            "can damage the locked area\n"
		            "and the entire system. "
		            "See <https://wiki.debian.org/Teams/Dpkg/FAQ#db-lock>.");

		file_lock_setup(&fl, F_WRLCK);
		if (fcntl(*lockfd, F_GETLK, &fl) < 0)
			ohshit(_("%s was locked by another process\n%s"),
			       desc, warnmsg);

		execname = dpkg_get_pid_execname(fl.l_pid);

		ohshit(_("%s was locked by %s process with pid %d\n%s"),
		       desc, execname ? execname : C_("process", "<unknown>"),
		       fl.l_pid, warnmsg);
	}

	push_cleanup(file_unlock_cleanup, ~0, 3, lockfd, filename, desc);
}

void
file_show(const char *filename)
{
	struct pager *pager;
	struct dpkg_error err;
	int fd, rc;

	if (filename == NULL)
		internerr("filename is NULL");

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		ohshite(_("cannot open file %s"), filename);

	pager = pager_spawn(_("pager to show file"));
	rc = fd_fd_copy(fd, STDOUT_FILENO, -1, &err);
	pager_reap(pager);

	close(fd);

	if (rc < 0 && err.syserrno != EPIPE) {
		errno = err.syserrno;
		ohshite(_("cannot write file %s into the pager"), filename);
	}
}
