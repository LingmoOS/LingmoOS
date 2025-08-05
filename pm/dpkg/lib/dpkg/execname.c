/*
 * libdpkg - Debian packaging suite library routines
 * execname.c - executable name handling functions
 *
 * Copyright © 2024 Guillem Jover <guillem@debian.org>
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

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#if defined(_AIX) && defined(HAVE_SYS_PROCFS_H)
#include <sys/procfs.h>
#endif

#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#if defined(__GNU__)
#include <hurd.h>
#include <ps.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <libproc.h>
#endif

#include <dpkg/dpkg.h>
#if defined(__sun)
#include <dpkg/file.h>
#endif
#include <dpkg/execname.h>

#if defined(_AIX) && defined(HAVE_STRUCT_PSINFO)
static bool
proc_get_psinfo(pid_t pid, struct psinfo *psinfo)
{
	char filename[64];
	FILE *fp;

	snprintf(filename, sizeof(filename), "/proc/%d/psinfo", pid);
	fp = fopen(filename, "r");
	if (!fp)
		return false;
	if (fread(psinfo, sizeof(*psinfo), 1, fp) == 0) {
		fclose(fp);
		return false;
	}
	if (ferror(fp)) {
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}
#endif

/**
 * Get the executable name for a PID.
 *
 * Tries to obtain the executable name or process name for a specific PID,
 * if the executable name cannot be obtained then it will return NULL.
 *
 * @return A pointer to an allocated string with the executable name, or NULL.
 */
char *
dpkg_get_pid_execname(pid_t pid)
{
	char *execname = NULL;
#if defined(__linux__)
	char lname[32];
	char lcontents[_POSIX_PATH_MAX + 1];
	int nread;

	snprintf(lname, sizeof(lname), "/proc/%d/exe", pid);
	nread = readlink(lname, lcontents, sizeof(lcontents) - 1);
	if (nread == -1)
		return NULL;

	lcontents[nread] = '\0';
	execname = lcontents;
#elif defined(__GNU__)
	struct ps_context *pc;
	struct proc_stat *ps;
	error_t err;

	if (pid < 0)
		return NULL;

	err = ps_context_create(getproc(), &pc);
	if (err)
		return NULL;

	err = ps_context_find_proc_stat(pc, pid, &ps);
	if (err)
		return NULL;

	/* On old Hurd systems we have to use the argv[0] value, because
	 * there is nothing better. */
	if (proc_stat_set_flags(ps, PSTAT_ARGS) == 0 &&
	    (proc_stat_flags(ps) & PSTAT_ARGS))
		execname = proc_stat_args(ps);

#ifdef PSTAT_EXE
	/* On new Hurd systems we can use the correct value, as long
	 * as it's not NULL nor empty, as it was the case on the first
	 * implementation. */
	if (proc_stat_set_flags(ps, PSTAT_EXE) == 0 &&
	    proc_stat_flags(ps) & PSTAT_EXE &&
	    proc_stat_exe(ps) != NULL &&
	    proc_stat_exe(ps)[0] != '\0')
		execname = proc_stat_exe(ps);
#endif

	ps_context_free(pc);
#elif defined(__sun)
	char filename[64];
	struct varbuf vb = VARBUF_INIT;

	snprintf(filename, sizeof(filename), "/proc/%d/execname", pid);
	if (file_slurp(filename, &vb, NULL) < 0)
		return NULL;

	return varbuf_detach(&vb);
#elif defined(__APPLE__) && defined(__MACH__)
	char pathname[_POSIX_PATH_MAX];

	if (proc_pidpath(pid, pathname, sizeof(pathname)) < 0)
		return NULL;

	execname = pathname;
#elif defined(_AIX) && defined(HAVE_STRUCT_PSINFO)
	char filename[64];
	struct psinfo psi;

	snprintf(filename, sizeof(filename), "/proc/%d/psinfo", pid);
	if (!proc_get_psinfo(pid, &psi))
		return NULL;

	execname = psi.pr_fname;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
	int error, mib[4];
	size_t len;
	char pathname[PATH_MAX];

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = pid;
	len = sizeof(pathname);

	error = sysctl(mib, 4, pathname, &len, NULL, 0);
	if (error != 0 && errno != ESRCH)
		return NULL;
	if (len == 0)
		pathname[0] = '\0';
	execname = pathname;
#else
	return execname;
#endif

	return m_strdup(execname);
}
