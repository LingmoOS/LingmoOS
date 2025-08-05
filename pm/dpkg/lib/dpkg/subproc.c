/*
 * libdpkg - Debian packaging suite library routines
 * subproc.c - subprocess helper routines
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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
#include <sys/wait.h>

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/subproc.h>

static int signo_ignores[] = { SIGQUIT, SIGINT };
static struct sigaction sa_save[array_count(signo_ignores)];

static void
subproc_reset_signal(int sig, struct sigaction *sa_old)
{
	if (sigaction(sig, sa_old, NULL)) {
		fprintf(stderr, _("error un-catching signal %s: %s\n"),
		        strsignal(sig), strerror(errno));
		onerr_abort++;
	}
}

static void
subproc_set_signal(int sig, struct sigaction *sa, struct sigaction *sa_old,
                   const char *name)
{
	if (sigaction(sig, sa, sa_old))
		ohshite(_("unable to ignore signal %s before running %.250s"),
		        strsignal(sig), name);
}

void
subproc_signals_ignore(const char *name)
{
	struct sigaction sa;
	size_t i;

	onerr_abort++;
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;

	for (i = 0; i < array_count(signo_ignores); i++)
		subproc_set_signal(signo_ignores[i], &sa, &sa_save[i], name);

	push_cleanup(subproc_signals_cleanup, ~0, 0);
	onerr_abort--;
}

void
subproc_signals_cleanup(int argc, void **argv)
{
	size_t i;

	for (i = 0; i < array_count(signo_ignores); i++)
		subproc_reset_signal(signo_ignores[i], &sa_save[i]);
}

void
subproc_signals_restore(void)
{
	pop_cleanup(ehflag_normaltidy);
}

static void
print_subproc_error(const char *emsg, const void *data)
{
	fprintf(stderr, _("%s (subprocess): %s\n"), dpkg_get_progname(), emsg);
}

pid_t
subproc_fork(void)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		onerr_abort++;
		ohshite(_("fork failed"));
	}
	if (pid > 0)
		return pid;

	/* Push a new error context, so that we don't do the other cleanups,
	 * because they'll be done by/in the parent process. */
	push_error_context_func(catch_fatal_error, print_subproc_error, NULL);

	return pid;
}

static int
subproc_check(int status, const char *desc, enum subproc_flags flags)
{
	void (*out)(const char *fmt, ...) DPKG_ATTR_PRINTF(1);
	int n;

	if (flags & SUBPROC_WARN)
		out = warning;
	else
		out = ohshit;

	if (WIFEXITED(status)) {
		n = WEXITSTATUS(status);
		if (!n)
			return 0;
		if (flags & SUBPROC_RETERROR)
			return n;

		out(_("%s subprocess returned error exit status %d"), desc, n);
	} else if (WIFSIGNALED(status)) {
		n = WTERMSIG(status);
		if (!n)
			return 0;
		if ((flags & SUBPROC_NOPIPE) && n == SIGPIPE)
			return 0;
		if (flags & SUBPROC_RETSIGNO)
			return n;

		if (n == SIGINT)
			out(_("%s subprocess was interrupted"), desc);
		else
			out(_("%s subprocess was killed by signal (%s)%s"),
			    desc, strsignal(n),
			    WCOREDUMP(status) ? _(", core dumped") : "");
	} else {
		if (flags & SUBPROC_RETERROR)
			return -1;

		out(_("%s subprocess failed with wait status code %d"), desc,
		    status);
	}

	return -1;
}

static int
subproc_wait(pid_t pid, const char *desc)
{
	pid_t dead_pid;
	int status;

	while ((dead_pid = waitpid(pid, &status, 0)) < 0 && errno == EINTR) ;

	if (dead_pid != pid) {
		onerr_abort++;
		ohshite(_("wait for %s subprocess failed"), desc);
	}

	return status;
}

int
subproc_reap(pid_t pid, const char *desc, enum subproc_flags flags)
{
	int status, rc;

	status = subproc_wait(pid, desc);

	if (flags & SUBPROC_NOCHECK)
		rc = status;
	else
		rc = subproc_check(status, desc, flags);

	return rc;
}
