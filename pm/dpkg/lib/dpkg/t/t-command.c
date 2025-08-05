/*
 * libdpkg - Debian packaging suite library routines
 * t-command.c - test command implementation
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/test.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/dpkg.h>

static void
test_command_init(void)
{
	struct command cmd;

	command_init(&cmd, "/absolute/path/to/progname", NULL);
	test_str(cmd.filename, ==, "/absolute/path/to/progname");
	test_str(cmd.name, ==, "progname");
	test_pass(cmd.argc == 0);
	test_pass(cmd.argv[0] == NULL);

	command_destroy(&cmd);
	test_pass(cmd.filename == NULL);
	test_pass(cmd.name == NULL);
	test_pass(cmd.argc == 0);
	test_pass(cmd.argv == NULL);

	command_init(&cmd, "progname", NULL);
	test_str(cmd.filename, ==, "progname");
	test_str(cmd.name, ==, "progname");
	test_pass(cmd.argc == 0);
	test_pass(cmd.argv[0] == NULL);

	command_destroy(&cmd);

	command_init(&cmd, "progname", "description");
	test_str(cmd.filename, ==, "progname");
	test_str(cmd.name, ==, "description");
	test_pass(cmd.argc == 0);
	test_pass(cmd.argv[0] == NULL);

	command_destroy(&cmd);
}

static void
test_command_grow_argv(void)
{
	struct command cmd;
	int argv_size, i;

	command_init(&cmd, "test", NULL);

	argv_size = cmd.argv_size + 4;
	for (i = 0; i < argv_size; i++)
		command_add_arg(&cmd, "arg");

	test_pass(cmd.argc == argv_size);
	test_pass(cmd.argv_size >= argv_size);
	test_str(cmd.argv[0], ==, "arg");
	test_str(cmd.argv[argv_size - 1], ==, "arg");
	test_pass(cmd.argv[argv_size] == NULL);

	command_destroy(&cmd);
}

static void
test_command_add_arg(void)
{
	struct command cmd;

	command_init(&cmd, "test", NULL);

	command_add_arg(&cmd, "arg 0");
	test_pass(cmd.argc == 1);
	test_str(cmd.argv[0], ==, "arg 0");
	test_pass(cmd.argv[1] == NULL);

	command_add_arg(&cmd, "arg 1");
	test_pass(cmd.argc == 2);
	test_str(cmd.argv[0], ==, "arg 0");
	test_str(cmd.argv[1], ==, "arg 1");
	test_pass(cmd.argv[2] == NULL);

	command_add_arg(&cmd, "arg 2");
	test_pass(cmd.argc == 3);
	test_str(cmd.argv[0], ==, "arg 0");
	test_str(cmd.argv[1], ==, "arg 1");
	test_str(cmd.argv[2], ==, "arg 2");
	test_pass(cmd.argv[3] == NULL);

	command_destroy(&cmd);
}

static void
test_command_add_argl(void)
{
	struct command cmd;
	const char *args[] = {
		"arg 1",
		"arg 2",
		"arg 3",
		NULL,
	};

	command_init(&cmd, "test", NULL);

	command_add_arg(&cmd, "arg 0");

	command_add_argl(&cmd, args);
	test_pass(cmd.argc == 4);
	test_str(cmd.argv[0], ==, "arg 0");
	test_str(cmd.argv[1], ==, "arg 1");
	test_str(cmd.argv[2], ==, "arg 2");
	test_str(cmd.argv[3], ==, "arg 3");
	test_pass(cmd.argv[4] == NULL);

	command_destroy(&cmd);
}

static void
test_command_add_args(void)
{
	struct command cmd;

	command_init(&cmd, "test", NULL);

	command_add_arg(&cmd, "arg 0");

	command_add_args(&cmd, "arg 1", "arg 2", "arg 3", NULL);
	test_pass(cmd.argc == 4);
	test_str(cmd.argv[0], ==, "arg 0");
	test_str(cmd.argv[1], ==, "arg 1");
	test_str(cmd.argv[2], ==, "arg 2");
	test_str(cmd.argv[3], ==, "arg 3");
	test_pass(cmd.argv[4] == NULL);

	command_destroy(&cmd);
}

static void
test_command_exec(void)
{
	struct command cmd;
	pid_t pid;
	int ret;

	command_init(&cmd, "true", "exec test");

	command_add_arg(&cmd, "true");
	command_add_arg(&cmd, "arg 0");
	command_add_arg(&cmd, "arg 1");

	pid = subproc_fork();

	if (pid == 0)
		command_exec(&cmd);

	ret = subproc_reap(pid, "command exec test", 0);
	test_pass(ret == 0);

	command_destroy(&cmd);
}

static void
test_command_in_path(void)
{
	char *path_a, *path_b, *path_noexec;
	char *oldpath, *newpath = NULL;
	bool ret;
	int rc;

	path_a = test_data_file("command/path-a");
	path_b = test_data_file("command/path-b");
	path_noexec = test_data_file("command/path-noexec");

	oldpath = getenv("PATH");

	setenv("PATH", "", 1);
	ret = command_in_path("cmd");
	test_pass(ret == false);
	ret = command_in_path("cmd-a");
	test_pass(ret == false);
	ret = command_in_path("cmd-b");
	test_pass(ret == false);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	setenv("PATH", "/nonexistent", 1);
	ret = command_in_path("cmd");
	test_pass(ret == false);
	ret = command_in_path("cmd-a");
	test_pass(ret == false);
	ret = command_in_path("cmd-b");
	test_pass(ret == false);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	setenv("PATH", path_a, 1);
	ret = command_in_path("cmd");
	test_pass(ret == true);
	ret = command_in_path("cmd-a");
	test_pass(ret == true);
	ret = command_in_path("cmd-b");
	test_pass(ret == false);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	setenv("PATH", path_b, 1);
	ret = command_in_path("cmd");
	test_pass(ret == true);
	ret = command_in_path("cmd-a");
	test_pass(ret == false);
	ret = command_in_path("cmd-b");
	test_pass(ret == true);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	setenv("PATH", path_noexec, 1);
	ret = command_in_path("cmd");
	test_pass(ret == false);
	ret = command_in_path("cmd-a");
	test_pass(ret == false);
	ret = command_in_path("cmd-b");
	test_pass(ret == false);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	rc = asprintf(&newpath, "/nonexistent:%s:%s:%s", path_a, path_b, path_noexec);
	if (rc < 0)
		test_bail("cannot allocate new PATH variable");
	setenv("PATH", newpath, 1);
	ret = command_in_path("cmd");
	test_pass(ret == true);
	ret = command_in_path("cmd-a");
	test_pass(ret == true);
	ret = command_in_path("cmd-b");
	test_pass(ret == true);
	ret = command_in_path("cmd-noexec");
	test_pass(ret == false);

	setenv("PATH", oldpath, 1);

	free(path_a);
	free(path_b);
	free(path_noexec);
	free(newpath);
}

static void
test_command_shell(void)
{
	pid_t pid;
	int ret;

	pid = subproc_fork();
	if (pid == 0)
		command_shell("true", "command shell pass test");
	ret = subproc_reap(pid, "command shell pass test", 0);
	test_pass(ret == 0);

	pid = subproc_fork();
	if (pid == 0)
		command_shell("false", "command shell fail test");
	ret = subproc_reap(pid, "command shell fail test", SUBPROC_RETERROR);
	test_fail(ret == 0);

	unsetenv("SHELL");
	pid = subproc_fork();
	if (pid == 0)
		command_shell("true", "command default shell test");
	ret = subproc_reap(pid, "command default shell test", 0);
	test_pass(ret == 0);
}

TEST_ENTRY(test)
{
	test_plan(73);

	test_command_init();
	test_command_grow_argv();
	test_command_add_arg();
	test_command_add_argl();
	test_command_add_args();
	test_command_exec();
	test_command_in_path();
	test_command_shell();
}
