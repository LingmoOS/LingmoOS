/*
 * libdpkg - Debian packaging suite library routines
 * pager.c - pager execution support
 *
 * Copyright © 2018 Guillem Jover <guillem@debian.org>
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/string.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/pager.h>

static bool pager_enabled = true;

void
pager_enable(bool enable)
{
	pager_enabled = enable;
}

/**
 * Get a suitable pager.
 *
 * @return A string representing a pager.
 */
const char *
pager_get_exec(void)
{
	const char *envvars[] = {
		"DPKG_PAGER",
		"PAGER",
	};
	const char *fallback[] = {
		DPKG_DEFAULT_PAGER,
		LESS,
		MORE,
	};
	size_t i;

	if (!isatty(0) || !isatty(1))
		return CAT;

	for (i = 0; i < array_count(envvars); i++) {
		const char *pager = getenv(envvars[i]);

		if (str_is_set(pager) && command_in_path(pager))
			return pager;
	}

	for (i = 0; i < array_count(fallback); i++)
		if (command_in_path(fallback[i]))
			return fallback[i];

	return CAT;
}

struct pager {
	bool used;
	const char *desc;
	pid_t pid;
	struct sigaction sigpipe;
	int stdout_old;
	int pipe[2];
};

struct pager *
pager_spawn(const char *desc)
{
	struct sigaction sa;
	struct pager *pager;
	const char *exec;

	pager = m_calloc(1, sizeof(*pager));
	pager->used = isatty(0) && isatty(1);
	pager->desc = desc;

	exec = pager_get_exec();
	if (strcmp(exec, CAT) == 0)
		pager->used = false;

	if (!pager_enabled)
		pager->used = false;

	if (!pager->used)
		return pager;

	m_pipe(pager->pipe);

	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;

	sigaction(SIGPIPE, &sa, &pager->sigpipe);

	pager->pid = subproc_fork();
	if (pager->pid == 0) {
		/* Set better defaults for less if not already set. */
		setenv("LESS", "-FRSXMQ", 0);

		m_dup2(pager->pipe[0], 0);
		close(pager->pipe[0]);
		close(pager->pipe[1]);

		command_shell(exec, desc);
	}

	pager->stdout_old = m_dup(1);
	m_dup2(pager->pipe[1], 1);
	close(pager->pipe[0]);
	close(pager->pipe[1]);

	/* Force the output to fully buffered, because originally stdout was
	 * a tty, so it was set as line buffered. This way we send as much as
	 * possible to the pager, which will handle the output by itself. */
	setvbuf(stdout, NULL, _IOFBF, 0);

	return pager;
}

void
pager_reap(struct pager *pager)
{
	if (!pager->used)
		return;

	m_dup2(pager->stdout_old, 1);
	subproc_reap(pager->pid, pager->desc, SUBPROC_NOPIPE);

	sigaction(SIGPIPE, &pager->sigpipe, NULL);

	free(pager);
}
