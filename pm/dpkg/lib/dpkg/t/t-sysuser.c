/*
 * libdpkg - Debian packaging suite library routines
 * t-sysuser.c - test sysuser handling code
 *
 * Copyright © 2025 Guillem Jover <guillem@debian.org>
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

#include <dpkg/test.h>
#include <dpkg/file.h>
#include <dpkg/sysuser.h>

#define test_user(name, pass, uid, gid, gecos, dir, shell) \
	do { \
		test_fail(pw == NULL); \
		if (pw == NULL) \
			continue; \
		test_str(pw->pw_name, ==, name); \
		test_str(pw->pw_passwd, ==, pass); \
		test_pass(pw->pw_uid == uid); \
		test_pass(pw->pw_gid == gid); \
		test_str(pw->pw_gecos, ==, gecos); \
		test_str(pw->pw_dir, ==, dir); \
		test_str(pw->pw_shell, ==, shell); \
	} while (0)

#define test_user_from_name(name, pass, uid, gid, gecos, dir, shell) \
	pw = dpkg_sysuser_from_name(name); \
	test_user(name, pass, uid, gid, gecos, dir, shell); \
	/* EOM. */

#define test_user_from_uid(name, pass, uid, gid, gecos, dir, shell) \
	pw = dpkg_sysuser_from_uid(uid); \
	test_user(name, pass, uid, gid, gecos, dir, shell); \
	/* EOM. */

static void
test_sysuser(void)
{
	char *passwdfile, *passwdfile_abs;
	struct passwd *pw;

	passwdfile = test_data_file("sysuser/passwd.base");
	passwdfile_abs = file_realpath(passwdfile);
	setenv("DPKG_PATH_PASSWD", passwdfile_abs, 1);
	free(passwdfile_abs);
	free(passwdfile);

	test_user_from_name("root", "x", 0, 0, "root", "/root", "/bin/sh");
	test_user_from_name("daemon", "x", 1, 1, "daemon", "/usr/sbin", "/usr/sbin/nologin");
	test_user_from_name("bin", "x", 2, 2, "bin", "/bin", "/usr/sbin/nologin");
	test_user_from_name("sys", "x", 3, 3, "sys", "/dev", "/usr/sbin/nologin");
	test_user_from_name("sync", "x", 4, 65534, "sync", "/bin", "/bin/sync");
	test_user_from_name("_sysuser", "x", 100, 65534, "", "/nonexistent", "/usr/sbin/nologin");
	test_user_from_name("someuser", "x", 1000, 1000, "Some User,,,", "/home/someuser", "/bin/bash");
	test_user_from_name("otheruser", "x", 1001, 1001, "Other User,,,", "/home/otheruser", "/bin/sh");
	pw = dpkg_sysuser_from_name("invented");
	test_pass(pw == NULL);

	test_user_from_uid("root", "x", 0, 0, "root", "/root", "/bin/sh");
	test_user_from_uid("daemon", "x", 1, 1, "daemon", "/usr/sbin", "/usr/sbin/nologin");
	test_user_from_uid("bin", "x", 2, 2, "bin", "/bin", "/usr/sbin/nologin");
	test_user_from_uid("sys", "x", 3, 3, "sys", "/dev", "/usr/sbin/nologin");
	test_user_from_uid("sync", "x", 4, 65534, "sync", "/bin", "/bin/sync");
	test_user_from_uid("_sysuser", "x", 100, 65534, "", "/nonexistent", "/usr/sbin/nologin");
	test_user_from_uid("someuser", "x", 1000, 1000, "Some User,,,", "/home/someuser", "/bin/bash");
	test_user_from_uid("otheruser", "x", 1001, 1001, "Other User,,,", "/home/otheruser", "/bin/sh");
	pw = dpkg_sysuser_from_uid(12345);
	test_pass(pw == NULL);
}

#define test_group(name, pass, gid, mem) \
	do { \
		test_fail(gr == NULL); \
		if (gr == NULL) \
			continue; \
		test_str(gr->gr_name, ==, name); \
		test_str(gr->gr_passwd, ==, pass); \
		test_pass(gr->gr_gid == gid); \
	} while (0)

#define test_group_from_name(name, pass, gid, mem) \
	gr = dpkg_sysgroup_from_name(name); \
	test_group(name, pass, gid, mem); \
	/* EOM. */

#define test_group_from_gid(name, pass, gid, mem) \
	gr = dpkg_sysgroup_from_gid(gid); \
	test_group(name, pass, gid, mem); \
	/* EOM. */

static void
test_sysgroup(void)
{
	char *groupfile, *groupfile_abs;
	struct group *gr;

	groupfile = test_data_file("sysuser/group.base");
	groupfile_abs = file_realpath(groupfile);
	setenv("DPKG_PATH_GROUP", groupfile_abs, 1);
	free(groupfile_abs);
	free(groupfile);

	test_group_from_name("root", "x", 0, NULL);
	test_group_from_name("daemon", "x", 1, NULL);
	test_group_from_name("bin", "x", 2, NULL);
	test_group_from_name("sys", "x", 3, NULL);
	test_group_from_name("adm", "x", 4, NULL);
	test_group_from_name("users", "x", 100, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_str(gr->gr_mem[1], ==, "otheruser");
	test_pass(gr->gr_mem[2] == NULL);
	test_group_from_name("someuser", "x", 1000, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_pass(gr->gr_mem[1] == NULL);
	test_group_from_name("otheruser", "x", 1001, NULL);
	test_str(gr->gr_mem[0], ==, "otheruser");
	test_pass(gr->gr_mem[1] == NULL);
	gr = dpkg_sysgroup_from_name("invented");
	test_pass(gr == NULL);

	test_group_from_gid("root", "x", 0, NULL);
	test_group_from_gid("daemon", "x", 1, NULL);
	test_group_from_gid("bin", "x", 2, NULL);
	test_group_from_gid("sys", "x", 3, NULL);
	test_group_from_gid("adm", "x", 4, NULL);
	test_group_from_gid("users", "x", 100, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_str(gr->gr_mem[1], ==, "otheruser");
	test_pass(gr->gr_mem[2] == NULL);
	test_group_from_gid("someuser", "x", 1000, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_pass(gr->gr_mem[1] == NULL);
	test_group_from_gid("otheruser", "x", 1001, NULL);
	test_str(gr->gr_mem[0], ==, "otheruser");
	test_pass(gr->gr_mem[1] == NULL);
	gr = dpkg_sysgroup_from_gid(12345);
	test_pass(gr == NULL);
}

TEST_ENTRY(test)
{
	test_plan(210);

	test_sysuser();
	test_sysgroup();
}
