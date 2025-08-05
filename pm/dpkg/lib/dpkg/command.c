/*
 * libdpkg - Debian packaging suite library routines
 * command.c - command execution support
 *
 * Copyright © 2010-2012 Guillem Jover <guillem@debian.org>
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

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/string.h>
#include <dpkg/varbuf.h>
#include <dpkg/file.h>
#include <dpkg/path.h>
#include <dpkg/command.h>

/**
 * Initialize a command structure.
 *
 * If name is NULL, then the last component of the filename path will be
 * used to initialize the name member.
 *
 * @param cmd The command structure to initialize.
 * @param filename The filename of the command to execute.
 * @param name The description of the command to execute.
 */
void
command_init(struct command *cmd, const char *filename, const char *name)
{
	cmd->filename = filename;
	if (name == NULL)
		cmd->name = path_basename(filename);
	else
		cmd->name = name;
	cmd->argc = 0;
	cmd->argv_size = 10;
	cmd->argv = m_malloc(cmd->argv_size * sizeof(cmd->argv[0]));
	cmd->argv[0] = NULL;
}

/**
 * Destroy a command structure.
 *
 * Free the members managed by the command functions (i.e. the argv pointer
 * array), and zero all members of a command structure.
 *
 * @param cmd The command structure to free.
 */
void
command_destroy(struct command *cmd)
{
	cmd->filename = NULL;
	cmd->name = NULL;
	cmd->argc = 0;
	cmd->argv_size = 0;
	free(cmd->argv);
	cmd->argv = NULL;
}

static void
command_grow_argv(struct command *cmd, int need)
{
	/* We need a ghost byte for the NUL character. */
	need++;

	/* Check if we already have enough room. */
	if ((cmd->argv_size - cmd->argc) >= need)
		return;

	cmd->argv_size = (cmd->argv_size + need) * 2;
	cmd->argv = m_realloc(cmd->argv, cmd->argv_size * sizeof(cmd->argv[0]));
}

/**
 * Append an argument to the command's argv.
 *
 * @param cmd The command structure to act on.
 * @param arg The argument to append to argv.
 */
void
command_add_arg(struct command *cmd, const char *arg)
{
	command_grow_argv(cmd, 1);

	cmd->argv[cmd->argc++] = arg;
	cmd->argv[cmd->argc] = NULL;
}

/**
 * Append an argument array to the command's argv.
 *
 * @param cmd The command structure to act on.
 * @param argv The NULL terminated argument array to append to argv.
 */
void
command_add_argl(struct command *cmd, const char **argv)
{
	int i, add_argc = 0;

	while (argv[add_argc] != NULL)
		add_argc++;

	command_grow_argv(cmd, add_argc);

	for (i = 0; i < add_argc; i++)
		cmd->argv[cmd->argc++] = argv[i];

	cmd->argv[cmd->argc] = NULL;
}

/**
 * Append a va_list of argument to the command's argv.
 *
 * @param cmd The command structure to act on.
 * @param args The NULL terminated va_list of argument array to append to argv.
 */
void
command_add_argv(struct command *cmd, va_list args)
{
	va_list args_copy;
	int i, add_argc = 0;

	va_copy(args_copy, args);
	while (va_arg(args_copy, const char *) != NULL)
		add_argc++;
	va_end(args_copy);

	command_grow_argv(cmd, add_argc);

	for (i = 0; i < add_argc; i++)
		cmd->argv[cmd->argc++] = va_arg(args, const char *);

	cmd->argv[cmd->argc] = NULL;
}

/**
 * Append a variable list of argument to the command's argv.
 *
 * @param cmd The command structure to act on.
 * @param ... The NULL terminated variable list of argument to append to argv.
 */
void
command_add_args(struct command *cmd, ...)
{
	va_list args;

	va_start(args, cmd);
	command_add_argv(cmd, args);
	va_end(args);
}

/**
 * Execute the command specified.
 *
 * The command is executed searching the PATH if the filename does not
 * contain any slashes, or using the full path if it's either a relative or
 * absolute pathname. This functions does not return.
 *
 * @param cmd The command structure to act on.
 */
void
command_exec(struct command *cmd)
{
	execvp(cmd->filename, (char * const *)cmd->argv);
	ohshite(_("unable to execute %s (%s)"), cmd->name, cmd->filename);
}

/**
 * Execute a shell with a possible command.
 *
 * @param cmd The command string to execute, if it's NULL an interactive
 *            shell will be executed instead.
 * @param name The description of the command to execute.
 */
void
command_shell(const char *cmd, const char *name)
{
	const char *shell;
	const char *mode;

	if (cmd == NULL) {
		mode = "-i";
		shell = getenv("SHELL");
	} else {
		mode = "-c";
		shell = NULL;
	}

	if (str_is_unset(shell))
		shell = DPKG_DEFAULT_SHELL;

#if HAVE_DPKG_SHELL_WITH_DASH_DASH
	execlp(shell, shell, mode, "--", cmd, NULL);
#else
	execlp(shell, shell, mode, cmd, NULL);
#endif
	ohshite(_("unable to execute %s (%s)"), name, cmd);
}

/**
 * Check whether a command can be found in PATH.
 *
 * @param cmd The command name to check. This is a relative pathname.
 *
 * @return A boolean denoting whether the command has been found.
 */
bool
command_in_path(const char *cmd)
{
	struct varbuf filename = VARBUF_INIT;
	const char *path_list;
	const char *path, *path_end;

	if (cmd[0] == '/')
		return file_is_exec(cmd);

	path_list = getenv("PATH");
	if (!path_list)
		ohshit(_("PATH is not set"));

	for (path = path_list; path; path = *path_end ? path_end + 1 : NULL) {
		size_t path_len;

		path_end = strchrnul(path, ':');
		path_len = (size_t)(path_end - path);

		varbuf_set_buf(&filename, path, path_len);
		if (path_len)
			varbuf_add_char(&filename, '/');
		varbuf_add_str(&filename, cmd);

		if (file_is_exec(filename.buf)) {
			varbuf_destroy(&filename);
			return true;
		}
	}

	varbuf_destroy(&filename);
	return false;
}
