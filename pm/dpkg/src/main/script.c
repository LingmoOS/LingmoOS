/*
 * dpkg - main program for package management
 * script.c - maintainer script routines
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2007-2014 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef WITH_LIBSELINUX
#include <selinux/selinux.h>
#endif

#include <dpkg/i18n.h>
#include <dpkg/debug.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/triglib.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"

void
post_postinst_tasks(struct pkginfo *pkg, enum pkgstatus new_status)
{
	if (new_status < PKG_STAT_TRIGGERSAWAITED)
		pkg_set_status(pkg, new_status);
	else if (pkg->trigaw.head)
		pkg_set_status(pkg, PKG_STAT_TRIGGERSAWAITED);
	else if (pkg->trigpend_head)
		pkg_set_status(pkg, PKG_STAT_TRIGGERSPENDING);
	else
		pkg_set_status(pkg, PKG_STAT_INSTALLED);
	modstatdb_note(pkg);

	debug(dbg_triggersdetail, "post_postinst_tasks - trig_incorporate");
	trig_incorporate(modstatdb_get_status());
}

static void
post_script_tasks(void)
{
	debug(dbg_triggersdetail, "post_script_tasks - ensure_diversions");
	ensure_diversions();

	debug(dbg_triggersdetail, "post_script_tasks - trig_incorporate");
	trig_incorporate(modstatdb_get_status());
}

static void
cu_post_script_tasks(int argc, void **argv)
{
	post_script_tasks();
}

static void
setexecute(const char *path, struct stat *stab)
{
	if ((stab->st_mode & 0555) == 0555)
		return;
	if (!chmod(path, 0755))
		return;
	ohshite(_("unable to set execute permissions on '%.250s'"), path);
}

/**
 * Returns the path to the script inside the chroot.
 */
static const char *
maintscript_pre_exec(struct command *cmd)
{
	const char *instdir = dpkg_fsys_get_dir();
	const char *admindir = dpkg_db_get_dir();
	const char *changedir;
	size_t instdirlen = strlen(instdir);

	if (instdirlen > 0 && in_force(FORCE_SCRIPT_CHROOTLESS))
		changedir = instdir;
	else
		changedir = "/";

	if (instdirlen > 0 && !in_force(FORCE_SCRIPT_CHROOTLESS)) {
		int rc;

		if (strncmp(admindir, instdir, instdirlen) != 0)
			ohshit(_("admindir must be inside instdir for dpkg to work properly"));
		if (setenv("DPKG_ADMINDIR", admindir + instdirlen, 1) < 0)
			ohshite(_("unable to setenv for subprocesses"));
		if (setenv("DPKG_ROOT", "", 1) < 0)
			ohshite(_("unable to setenv for subprocesses"));

		rc = chroot(instdir);
		if (rc && in_force(FORCE_NON_ROOT) && errno == EPERM)
			ohshit(_("not enough privileges to change root "
			         "directory with --force-not-root, consider "
			         "using --force-script-chrootless?"));
		else if (rc)
			ohshite(_("failed to chroot to '%.250s'"), instdir);
	}
	/* Switch to a known good directory to give the maintainer script
	 * a saner environment, also needed after the chroot(). */
	if (chdir(changedir))
		ohshite(_("failed to chdir to '%.255s'"), changedir);
	if (debug_has_flag(dbg_scripts)) {
		struct varbuf args = VARBUF_INIT;
		const char **argv = cmd->argv;

		while (*++argv) {
			varbuf_add_char(&args, ' ');
			varbuf_add_str(&args, *argv);
		}
		debug(dbg_scripts, "fork/exec %s (%s )", cmd->filename,
		      varbuf_str(&args));
		varbuf_destroy(&args);
	}
	if (instdirlen == 0 || in_force(FORCE_SCRIPT_CHROOTLESS))
		return cmd->filename;

	if (strlen(cmd->filename) < instdirlen)
		internerr("maintscript name '%s' length < instdir length %zd",
		          cmd->filename, instdirlen);

	return cmd->filename + instdirlen;
}

/**
 * Set a new security execution context for the maintainer script.
 *
 * Try to create a new execution context based on the current one and the
 * specific maintainer script filename. If it's the same as the current
 * one, use the given fallback.
 */
static int
maintscript_set_exec_context(struct command *cmd)
{
#ifdef WITH_LIBSELINUX
	return setexecfilecon(cmd->filename, "dpkg_script_t");
#else
	return 0;
#endif
}

static int
maintscript_exec(struct pkginfo *pkg, struct pkgbin *pkgbin,
                 struct command *cmd, struct stat *stab, int warn)
{
	pid_t pid;
	int rc;

	setexecute(cmd->filename, stab);

	push_cleanup(cu_post_script_tasks, ehflag_bombout, 0);

	pid = subproc_fork();
	if (pid == 0) {
		char *pkg_count;
		const char *maintscript_debug;

		pkg_count = str_fmt("%d", pkgset_installed_instances(pkg->set));

		maintscript_debug = debug_has_flag(dbg_scripts) ? "1" : "0";

		if (setenv("DPKG_MAINTSCRIPT_PACKAGE", pkg->set->name, 1) ||
		    setenv("DPKG_MAINTSCRIPT_PACKAGE_REFCOUNT", pkg_count, 1) ||
		    setenv("DPKG_MAINTSCRIPT_ARCH", pkgbin->arch->name, 1) ||
		    setenv("DPKG_MAINTSCRIPT_NAME", cmd->argv[0], 1) ||
		    setenv("DPKG_MAINTSCRIPT_DEBUG", maintscript_debug, 1) ||
		    setenv("DPKG_RUNNING_VERSION", PACKAGE_VERSION, 1))
			ohshite(_("unable to setenv for maintainer script"));

		cmd->filename = cmd->argv[0] = maintscript_pre_exec(cmd);

		if (maintscript_set_exec_context(cmd) < 0)
			ohshite(_("cannot set security execution context for "
			          "maintainer script"));

		command_exec(cmd);
	}
	subproc_signals_ignore(cmd->name);
	rc = subproc_reap(pid, cmd->name, warn);
	subproc_signals_restore();

	pop_cleanup(ehflag_normaltidy);

	return rc;
}

static int
vmaintscript_installed(struct pkginfo *pkg, const char *scriptname,
                       const char *desc, va_list args)
{
	struct command cmd;
	const char *scriptpath;
	struct stat stab;
	char *buf;

	scriptpath = pkg_infodb_get_file(pkg, &pkg->installed, scriptname);
	m_asprintf(&buf, _("installed %s package %s script"),
	           pkg_name(pkg, pnaw_nonambig), desc);

	command_init(&cmd, scriptpath, buf);
	command_add_arg(&cmd, scriptname);
	command_add_argv(&cmd, args);

	if (stat(scriptpath, &stab)) {
		command_destroy(&cmd);

		if (errno == ENOENT) {
			debug(dbg_scripts,
			      "vmaintscript_installed nonexistent %s",
			      scriptname);
			free(buf);
			return 0;
		}
		ohshite(_("unable to stat %s '%.250s'"), buf, scriptpath);
	}
	maintscript_exec(pkg, &pkg->installed, &cmd, &stab, 0);

	command_destroy(&cmd);
	free(buf);

	return 1;
}

/*
 * All ...'s in maintscript_* are const char *'s.
 */

int
maintscript_installed(struct pkginfo *pkg, const char *scriptname,
                      const char *desc, ...)
{
	va_list args;
	int rc;

	va_start(args, desc);
	rc = vmaintscript_installed(pkg, scriptname, desc, args);
	va_end(args);

	if (rc)
		post_script_tasks();

	return rc;
}

int
maintscript_postinst(struct pkginfo *pkg, ...)
{
	va_list args;
	int rc;

	va_start(args, pkg);
	rc = vmaintscript_installed(pkg, POSTINSTFILE, "post-installation", args);
	va_end(args);

	if (rc)
		ensure_diversions();

	return rc;
}

int
maintscript_new(struct pkginfo *pkg, const char *scriptname,
                const char *desc, const char *cidir, char *cidirrest, ...)
{
	struct command cmd;
	struct stat stab;
	va_list args;
	char *buf;

	strcpy(cidirrest, scriptname);
	m_asprintf(&buf, _("new %s package %s script"),
	           pkg_name(pkg, pnaw_nonambig), desc);

	va_start(args, cidirrest);
	command_init(&cmd, cidir, buf);
	command_add_arg(&cmd, scriptname);
	command_add_argv(&cmd, args);
	va_end(args);

	if (stat(cidir, &stab)) {
		command_destroy(&cmd);

		if (errno == ENOENT) {
			debug(dbg_scripts,
			      "maintscript_new nonexistent %s '%s'",
			      scriptname, cidir);
			free(buf);
			return 0;
		}
		ohshite(_("unable to stat %s '%.250s'"), buf, cidir);
	}
	maintscript_exec(pkg, &pkg->available, &cmd, &stab, 0);

	command_destroy(&cmd);
	free(buf);
	post_script_tasks();

	return 1;
}

int
maintscript_fallback(struct pkginfo *pkg,
                     const char *scriptname, const char *desc,
                     const char *cidir, char *cidirrest,
                     const char *ifok, const char *iffallback)
{
	struct command cmd;
	const char *oldscriptpath;
	struct stat stab;
	char *buf;

	oldscriptpath = pkg_infodb_get_file(pkg, &pkg->installed, scriptname);
	m_asprintf(&buf, _("old %s package %s script"),
	           pkg_name(pkg, pnaw_nonambig), desc);

	command_init(&cmd, oldscriptpath, buf);
	command_add_args(&cmd, scriptname, ifok,
	                 versiondescribe(&pkg->available.version, vdew_nonambig),
	                 NULL);

	if (stat(oldscriptpath, &stab)) {
		if (errno == ENOENT) {
			debug(dbg_scripts,
			      "maintscript_fallback nonexistent %s '%s'",
			      scriptname, oldscriptpath);
			command_destroy(&cmd);
			free(buf);
			return 0;
		}
		warning(_("unable to stat %s '%.250s': %s"),
		        cmd.name, oldscriptpath, strerror(errno));
	} else if (!maintscript_exec(pkg, &pkg->installed, &cmd, &stab, SUBPROC_WARN)) {
		command_destroy(&cmd);
		free(buf);
		post_script_tasks();
		return 1;
	}
	notice(_("trying script from the new package instead ..."));

	strcpy(cidirrest, scriptname);
	m_asprintf(&buf, _("new %s package %s script"),
	           pkg_name(pkg, pnaw_nonambig), desc);

	command_destroy(&cmd);
	command_init(&cmd, cidir, buf);
	command_add_args(&cmd, scriptname, iffallback,
	                 versiondescribe(&pkg->installed.version, vdew_nonambig),
	                 versiondescribe(&pkg->available.version, vdew_nonambig),
	                 NULL);

	if (stat(cidir, &stab)) {
		command_destroy(&cmd);

		if (errno == ENOENT)
			ohshit(_("there is no script in the new version of the package - giving up"));
		else
			ohshite(_("unable to stat %s '%.250s'"), buf, cidir);
	}

	maintscript_exec(pkg, &pkg->available, &cmd, &stab, 0);
	notice(_("... it looks like that went OK"));

	command_destroy(&cmd);
	free(buf);
	post_script_tasks();

	return 1;
}
