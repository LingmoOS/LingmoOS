/*
 * libdpkg - Debian packaging suite library routines
 * pkg-spec.c - primitives for pkg specifier handling
 *
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
 * Copyright © 2011-2015 Guillem Jover <guillem@debian.org>
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

#include <stdlib.h>
#include <fnmatch.h>
#include <string.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>
#include <dpkg/pkg-spec.h>

static void
pkg_spec_blank(struct pkg_spec *ps)
{
	ps->name = NULL;
	ps->arch = NULL;

	ps->name_is_pattern = false;
	ps->arch_is_pattern = false;
}

static void
pkg_spec_iter_blank(struct pkg_spec *ps)
{
	ps->pkg_iter = NULL;
	ps->pkg_next = NULL;
}

void
pkg_spec_init(struct pkg_spec *ps, enum pkg_spec_flags flags)
{
	ps->flags = flags;

	pkg_spec_blank(ps);
	pkg_spec_iter_blank(ps);
}

const char *
pkg_spec_is_illegal(struct pkg_spec *ps)
{
	static char msg[1024];
	const char *emsg;

	if (!ps->name_is_pattern &&
	    (emsg = pkg_name_is_illegal(ps->name))) {
		const char *arch_sep;

		/* Only check for DPKG_ARCH_NONE, because for everything else
		 * we want to see the passed package specification, even if
		 * the architecture is empty. */
		if (ps->arch->type == DPKG_ARCH_NONE)
			arch_sep = "";
		else
			arch_sep = ":";

		snprintf(msg, sizeof(msg),
		         _("illegal package name in specifier '%s%s%s': %s"),
		         ps->name, arch_sep, ps->arch->name, emsg);
		return msg;
	}

	if ((!ps->arch_is_pattern && ps->arch->type == DPKG_ARCH_ILLEGAL) ||
	    ps->arch->type == DPKG_ARCH_EMPTY) {
		emsg = dpkg_arch_name_is_illegal(ps->arch->name);
		snprintf(msg, sizeof(msg),
		         _("illegal architecture name in specifier '%s:%s': %s"),
		         ps->name, ps->arch->name, emsg);
		return msg;
	}

	/* If we have been requested a single instance, check that the
	 * package does not contain other instances. */
	if (!ps->arch_is_pattern && ps->flags & PKG_SPEC_ARCH_SINGLE) {
		struct pkgset *set;

		set = pkg_hash_find_set(ps->name);

		/* Single instancing only applies with no architecture. */
		if (ps->arch->type == DPKG_ARCH_NONE &&
		    pkgset_installed_instances(set) > 1) {
			snprintf(msg, sizeof(msg),
			         _("ambiguous package name '%s' with more "
			           "than one installed instance"), ps->name);
			return msg;
		}
	}

	return NULL;
}

static const char *
pkg_spec_prep(struct pkg_spec *ps, char *pkgname, const char *archname)
{
	ps->name = pkgname;
	ps->arch = dpkg_arch_find(archname);

	ps->name_is_pattern = false;
	ps->arch_is_pattern = false;

	/* Detect if we have patterns and/or illegal names. */
	if ((ps->flags & PKG_SPEC_PATTERNS) && strpbrk(ps->name, "*[?\\"))
		ps->name_is_pattern = true;

	if ((ps->flags & PKG_SPEC_PATTERNS) && strpbrk(ps->arch->name, "*[?\\"))
		ps->arch_is_pattern = true;

	return pkg_spec_is_illegal(ps);
}

const char *
pkg_spec_set(struct pkg_spec *ps, const char *pkgname, const char *archname)
{
	return pkg_spec_prep(ps, m_strdup(pkgname), archname);
}

const char *
pkg_spec_parse(struct pkg_spec *ps, const char *str)
{
	char *pkgname, *archname;

	archname = strchr(str, ':');
	if (archname == NULL) {
		pkgname = m_strdup(str);
	} else {
		pkgname = m_strndup(str, archname - str);
		archname++;
	}

	return pkg_spec_prep(ps, pkgname, archname);
}

static bool
pkg_spec_match_name(struct pkg_spec *ps, const char *name)
{
	if (ps->name_is_pattern)
		return (fnmatch(ps->name, name, 0) == 0);
	else
		return (strcmp(ps->name, name) == 0);
}

static bool
pkg_spec_match_arch(struct pkg_spec *ps, struct pkginfo *pkg,
                    const struct dpkg_arch *arch)
{
	if (ps->arch_is_pattern)
		return (fnmatch(ps->arch->name, arch->name, 0) == 0);
	else if (ps->arch->type != DPKG_ARCH_NONE) /* !arch_is_pattern */
		return (ps->arch == arch);

	/* No arch specified. */
	switch (ps->flags & PKG_SPEC_ARCH_MASK) {
	case PKG_SPEC_ARCH_SINGLE:
		return pkgset_installed_instances(pkg->set) <= 1;
	case PKG_SPEC_ARCH_WILDCARD:
		return true;
	default:
		internerr("unknown PKG_SPEC_ARCH_* flags %d in pkg_spec",
		          ps->flags & PKG_SPEC_ARCH_MASK);
	}
}

bool
pkg_spec_match_pkg(struct pkg_spec *ps, struct pkginfo *pkg,
                   struct pkgbin *pkgbin)
{
	return (pkg_spec_match_name(ps, pkg->set->name) &&
	        pkg_spec_match_arch(ps, pkg, pkgbin->arch));
}

static struct pkginfo *
pkg_spec_get_pkg(struct pkg_spec *ps)
{
	if (ps->arch->type == DPKG_ARCH_NONE)
		return pkg_hash_find_singleton(ps->name);
	else
		return pkg_hash_find_pkg(ps->name, ps->arch);
}

struct pkginfo *
pkg_spec_parse_pkg(const char *str, struct dpkg_error *err)
{
	struct pkg_spec ps;
	struct pkginfo *pkg;
	const char *emsg;

	pkg_spec_init(&ps, PKG_SPEC_ARCH_SINGLE);
	emsg = pkg_spec_parse(&ps, str);
	if (emsg) {
		dpkg_put_error(err, "%s", emsg);
		pkg = NULL;
	} else {
		pkg = pkg_spec_get_pkg(&ps);
	}
	pkg_spec_destroy(&ps);

	return pkg;
}

struct pkginfo *
pkg_spec_find_pkg(const char *pkgname, const char *archname,
                  struct dpkg_error *err)
{
	struct pkg_spec ps;
	struct pkginfo *pkg;
	const char *emsg;

	pkg_spec_init(&ps, PKG_SPEC_ARCH_SINGLE);
	emsg = pkg_spec_set(&ps, pkgname, archname);
	if (emsg) {
		dpkg_put_error(err, "%s", emsg);
		pkg = NULL;
	} else {
		pkg = pkg_spec_get_pkg(&ps);
	}
	pkg_spec_destroy(&ps);

	return pkg;
}

void
pkg_spec_iter_init(struct pkg_spec *ps)
{
	if (ps->name_is_pattern)
		ps->pkg_iter = pkg_hash_iter_new();
	else
		ps->pkg_next = &pkg_hash_find_set(ps->name)->pkg;
}

static struct pkginfo *
pkg_spec_iter_next_pkgname(struct pkg_spec *ps)
{
	struct pkginfo *pkg;

	while ((pkg = pkg_hash_iter_next_pkg(ps->pkg_iter))) {
		if (pkg_spec_match_pkg(ps, pkg, &pkg->installed))
			return pkg;
	}

	return NULL;
}

static struct pkginfo *
pkg_spec_iter_next_pkgarch(struct pkg_spec *ps)
{
	struct pkginfo *pkg;

	while ((pkg = ps->pkg_next)) {
		ps->pkg_next = pkg->arch_next;

		if (pkg_spec_match_arch(ps, pkg, pkg->installed.arch))
			return pkg;
	}

	return NULL;
}

struct pkginfo *
pkg_spec_iter_next_pkg(struct pkg_spec *ps)
{
	if (ps->name_is_pattern)
		return pkg_spec_iter_next_pkgname(ps);
	else
		return pkg_spec_iter_next_pkgarch(ps);
}

void
pkg_spec_iter_destroy(struct pkg_spec *ps)
{
	pkg_hash_iter_free(ps->pkg_iter);
	pkg_spec_iter_blank(ps);
}

void
pkg_spec_destroy(struct pkg_spec *ps)
{
	free(ps->name);
	pkg_spec_blank(ps);
	pkg_spec_iter_destroy(ps);
}
