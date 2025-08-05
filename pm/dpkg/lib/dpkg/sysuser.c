/*
 * libdpkg - Debian packaging suite library routines
 * sysuser.c - system user and group handling
 *
 * Copyright © 2023-2025 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <dpkg/string.h>
#include <dpkg/fsys.h>
#include <dpkg/sysuser.h>

#define DPKG_PATH_PASSWD "/etc/passwd"
#define DPKG_PATH_GROUP "/etc/group"

/* Cache UID 0 called "root" in most systems.
 * Cache GID 0 called "root" in most systems,
 *   or "wheel" on macOS,
 *   or "system" on AIX,
 *   although we do not cache the non-"root" ones.
 */

static struct passwd *root_pw;
static struct group *root_gr;

static FILE *
dpkg_sysdb_open(const char *envvar, const char *default_pathname)
{
	const char *pathname;
	char *root_pathname;
	FILE *fp;

	pathname = getenv(envvar);
	if (str_is_unset(pathname))
		pathname = default_pathname;
	root_pathname = dpkg_fsys_get_path(pathname);

	fp = fopen(root_pathname, "r");
	free(root_pathname);

	if (fp == NULL) {
		if (errno != ENOENT)
			return NULL;

		/*
		 * If there was no such file, we assume we might be in a
		 * bootstrapping scenario where the filesystem is empty,
		 * and we try to fallback potentially to the non-chroot
		 * file, which happens to be the historic behavior.
		 */
		fp = fopen(default_pathname, "r");
	}

	return fp;
}

/*
 * Copy a sysuser struct.
 *
 * Note: We only copy the portable subset that we care about, the rest will
 * be zeroed out.
 */
static struct passwd *
dpkg_sysuser_copy(struct passwd *pw_old)
{
	struct passwd *pw_new;

	pw_new = m_calloc(1, sizeof(*pw_new));
	pw_new->pw_name = m_strdup(pw_old->pw_name);
	pw_new->pw_uid = pw_old->pw_uid;
	pw_new->pw_gid = pw_old->pw_gid;
	pw_new->pw_gecos = m_strdup(pw_old->pw_gecos);
	pw_new->pw_dir = m_strdup(pw_old->pw_dir);
	pw_new->pw_shell = m_strdup(pw_old->pw_shell);

	return pw_new;
}

typedef bool dpkg_sysuser_match_func(struct passwd *pw, const void *match);

static struct passwd *
dpkg_sysuser_find(dpkg_sysuser_match_func *matcher, const void *match)
{
	FILE *fp;
	struct passwd *pw;

	fp = dpkg_sysdb_open("DPKG_PATH_PASSWD", DPKG_PATH_PASSWD);
	if (fp == NULL)
		return NULL;
	while ((pw = fgetpwent(fp)))
		if (matcher(pw, match))
			break;
	fclose(fp);

	return pw;
}

static bool
dpkg_sysuser_match_uname(struct passwd *pw, const void *match)
{
	const char *uname = match;

	return strcmp(pw->pw_name, uname) == 0;
}

struct passwd *
dpkg_sysuser_from_name(const char *uname)
{
	struct passwd *pw;
	bool is_root = strcmp(uname, "root") == 0;

	if (is_root && root_pw != NULL)
		return root_pw;

	pw = dpkg_sysuser_find(dpkg_sysuser_match_uname, uname);

	if (is_root && pw)
		root_pw = dpkg_sysuser_copy(pw);

	return pw;
}

static bool
dpkg_sysuser_match_uid(struct passwd *pw, const void *match)
{
	const uid_t *uid = match;

	return pw->pw_uid == *uid;
}

struct passwd *
dpkg_sysuser_from_uid(uid_t uid)
{
	return dpkg_sysuser_find(dpkg_sysuser_match_uid, &uid);
}

/*
 * Copy a sysgroup struct.
 *
 * Note: We only copy the portable subset that we care about, the rest will
 * be zeroed out.
 */
static struct group *
dpkg_sysgroup_copy(struct group *gr_old)
{
	struct group *gr_new;
	size_t n;

	gr_new = m_calloc(1, sizeof(*gr_new));
	gr_new->gr_name = m_strdup(gr_old->gr_name);
	gr_new->gr_gid = gr_old->gr_gid;

	for (n = 0; gr_old->gr_mem[n]; n++)
		;

	gr_new->gr_mem = m_calloc(n + 1, sizeof(char *));
	for (n = 0; gr_old->gr_mem[n]; n++)
		gr_new->gr_mem[n] = m_strdup(gr_old->gr_mem[n]);

	return gr_new;
}

typedef bool dpkg_sysgroup_match_func(struct group *gr, const void *match);

static struct group *
dpkg_sysgroup_find(dpkg_sysgroup_match_func *matcher, const void *match)
{
	FILE *fp;
	struct group *gr;

	fp = dpkg_sysdb_open("DPKG_PATH_GROUP", DPKG_PATH_GROUP);
	if (fp == NULL)
		return NULL;
	while ((gr = fgetgrent(fp)))
		if (matcher(gr, match))
			break;
	fclose(fp);

	return gr;
}

static bool
dpkg_sysgroup_match_gname(struct group *gr, const void *match)
{
	const char *gname = match;

	return strcmp(gr->gr_name, gname) == 0;
}

struct group *
dpkg_sysgroup_from_name(const char *gname)
{
	struct group *gr;
	bool is_root = strcmp(gname, "root") == 0;

	if (is_root && root_gr != NULL)
		return root_gr;

	gr = dpkg_sysgroup_find(dpkg_sysgroup_match_gname, gname);

	if (is_root && gr)
		root_gr = dpkg_sysgroup_copy(gr);

	return gr;
}

static bool
dpkg_sysgroup_match_gid(struct group *gr, const void *match)
{
	const gid_t *gid = match;

	return gr->gr_gid == *gid;
}

struct group *
dpkg_sysgroup_from_gid(gid_t gid)
{
	return dpkg_sysgroup_find(dpkg_sysgroup_match_gid, &gid);
}
