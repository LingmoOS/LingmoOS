/*
 * libdpkg - Debian packaging suite library routines
 * t-compat-getent.c - test compat getent handling code
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

#undef fgetpwent
#define fgetpwent test_fgetpwent
#undef fgetgrent
#define fgetgrent test_fgetgrent

#include <config.h>
#include <compat.h>

#include <dpkg/test.h>
#include <dpkg/sysuser.h>

#define test_pwent(name, pass, uid, gid, gecos, dir, shell) \
	do { \
		pw = fgetpwent(fp); \
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

#define test_pwent_nis(name) \
	test_pwent(name, "", 0, 0, "", "", "");

static void
test_compat_fgetpwent(void)
{
	char *passwdfile;
	FILE *fp;
	struct passwd *pw;

	passwdfile = test_data_file("sysuser/passwd.base");

	fp = fopen(passwdfile, "r");
	if (fp == NULL) {
		printf("# cannot open %s\n", passwdfile);
		test_bail("cannot open sysuser/passwd.base");
	}

	test_pwent("root", "x", 0, 0, "root", "/root", "/bin/sh");
	test_pwent("daemon", "x", 1, 1, "daemon", "/usr/sbin", "/usr/sbin/nologin");
	test_pwent("bin", "x", 2, 2, "bin", "/bin", "/usr/sbin/nologin");
	test_pwent("sys", "x", 3, 3, "sys", "/dev", "/usr/sbin/nologin");
	test_pwent("sync", "x", 4, 65534, "sync", "/bin", "/bin/sync");
	test_pwent("_sysuser", "x", 100, 65534, "", "/nonexistent", "/usr/sbin/nologin");
	test_pwent("someuser", "x", 1000, 1000, "Some User,,,", "/home/someuser", "/bin/bash");
	test_pwent("otheruser", "x", 1001, 1001, "Other User,,,", "/home/otheruser", "/bin/sh");
	pw = fgetpwent(fp);
	test_pass(pw == NULL);

	fclose(fp);
	free(passwdfile);

	passwdfile = test_data_file("sysuser/passwd.nis");

	fp = fopen(passwdfile, "r");
	if (fp == NULL) {
		printf("# cannot open %s\n", passwdfile);
		test_bail("cannot open sysuser/passwd.base");
	}

	test_pwent("root", "x", 0, 0, "root", "/root", "/bin/sh");
	test_pwent("daemon", "x", 1, 1, "daemon", "/usr/sbin", "/usr/sbin/nologin");
	test_pwent("bin", "x", 2, 2, "bin", "/bin", "/usr/sbin/nologin");
	test_pwent("sys", "x", 3, 3, "sys", "/dev", "/usr/sbin/nologin");
	test_pwent("sync", "x", 4, 65534, "sync", "/bin", "/bin/sync");
	test_pwent_nis("+user-compat-1");
	test_pwent("_sysuser", "x", 100, 65534, "", "/nonexistent", "/usr/sbin/nologin");
	test_pwent_nis("-user-compat-2");
	test_pwent("someuser", "x", 1000, 1000, "Some User,,,", "/home/someuser", "/bin/bash");
	test_pwent("otheruser", "x", 1001, 1001, "Other User,,,", "/home/otheruser", "/bin/sh");
	pw = fgetpwent(fp);
	test_pass(pw == NULL);

	fclose(fp);
	free(passwdfile);
}

#define test_grent(name, pass, gid, mem) \
	do { \
		gr = fgetgrent(fp); \
		test_fail(gr == NULL); \
		if (gr == NULL) \
			continue; \
		test_str(gr->gr_name, ==, name); \
		test_str(gr->gr_passwd, ==, pass); \
		test_pass(gr->gr_gid == gid); \
	} while (0)

#define test_grent_nis(name) \
	test_grent(name, "", 0, NULL); \
	test_pass(gr->gr_mem[0] == NULL);

static void
test_compat_fgetgrent(void)
{
	char *groupfile;
	FILE *fp;
	struct group *gr;

	groupfile = test_data_file("sysuser/group.base");
	fp = fopen(groupfile, "r");
	if (fp == NULL) {
		printf("# cannot open %s\n", groupfile);
		test_bail("cannot open sysuser/group.base");
	}

	test_grent("root", "x", 0, NULL);
	test_grent("daemon", "x", 1, NULL);
	test_grent("bin", "x", 2, NULL);
	test_grent("sys", "x", 3, NULL);
	test_grent("adm", "x", 4, NULL);
	test_grent("users", "x", 100, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_str(gr->gr_mem[1], ==, "otheruser");
	test_pass(gr->gr_mem[2] == NULL);
	test_grent("someuser", "x", 1000, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_pass(gr->gr_mem[1] == NULL);
	test_grent("otheruser", "x", 1001, NULL);
	test_str(gr->gr_mem[0], ==, "otheruser");
	test_pass(gr->gr_mem[1] == NULL);
	gr = fgetgrent(fp);
	test_pass(gr == NULL);

	fclose(fp);
	free(groupfile);

	groupfile = test_data_file("sysuser/group.nis");
	fp = fopen(groupfile, "r");
	if (fp == NULL) {
		printf("# cannot open %s\n", groupfile);
		test_bail("cannot open sysuser/group.base");
	}

	test_grent("root", "x", 0, NULL);
	test_grent("daemon", "x", 1, NULL);
	test_grent("bin", "x", 2, NULL);
	test_grent("sys", "x", 3, NULL);
	test_grent("adm", "x", 4, NULL);
	test_grent_nis("+group-compat-1");
	test_grent("users", "x", 100, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_str(gr->gr_mem[1], ==, "otheruser");
	test_pass(gr->gr_mem[2] == NULL);
	test_grent_nis("-group-compat-2");
	test_grent("someuser", "x", 1000, NULL);
	test_str(gr->gr_mem[0], ==, "someuser");
	test_pass(gr->gr_mem[1] == NULL);
	test_grent("otheruser", "x", 1001, NULL);
	test_str(gr->gr_mem[0], ==, "otheruser");
	test_pass(gr->gr_mem[1] == NULL);
	gr = fgetgrent(fp);
	test_pass(gr == NULL);

	fclose(fp);
	free(groupfile);
}

TEST_ENTRY(test)
{
	test_plan(236);

	test_compat_fgetpwent();
	test_compat_fgetgrent();
}
