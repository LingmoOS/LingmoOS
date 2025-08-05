/*
 * libdpkg - Debian packaging suite library routines
 * pkg.c - primitives for pkg handling
 *
 * Copyright © 1995, 1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2009-2014 Guillem Jover <guillem@debian.org>
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

#include <string.h>

#include <dpkg/ehandle.h>
#include <dpkg/string.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>

/**
 * Set the package installation status.
 */
void
pkg_set_status(struct pkginfo *pkg, enum pkgstatus status)
{
	if (pkg->status == status)
		return;
	else if (pkg->status == PKG_STAT_NOTINSTALLED)
		pkg->set->installed_instances++;
	else if (status == PKG_STAT_NOTINSTALLED)
		pkg->set->installed_instances--;

	if (pkg->set->installed_instances < 0)
		internerr("pkgset %s went into negative installed instances %d",
		          pkg->set->name, pkg->set->installed_instances);

	pkg->status = status;
	pkg->status_dirty = true;
}

/**
 * Set the specified flags to 1 in the package error flags.
 */
void
pkg_set_eflags(struct pkginfo *pkg, enum pkgeflag eflag)
{
	pkg->eflag |= eflag;
}

/**
 * Clear the specified flags to 0 in the package error flags.
 */
void
pkg_clear_eflags(struct pkginfo *pkg, enum pkgeflag eflag)
{
	pkg->eflag &= ~eflag;
}

/**
 * Reset the package error flags to 0.
 */
void
pkg_reset_eflags(struct pkginfo *pkg)
{
	pkg->eflag = PKG_EFLAG_OK;
}

/**
 * Copy the package error flags to another package.
 */
void
pkg_copy_eflags(struct pkginfo *pkg_dst, struct pkginfo *pkg_src)
{
	pkg_dst->eflag = pkg_src->eflag;
}

/**
 * Set the package selection status.
 */
void
pkg_set_want(struct pkginfo *pkg, enum pkgwant want)
{
	pkg->want = want;
}

void
pkgbin_blank(struct pkgbin *pkgbin)
{
	pkgbin->essential = false;
	pkgbin->is_protected = false;
	pkgbin->depends = NULL;
	pkgbin->pkgname_archqual = NULL;
	pkgbin->description = NULL;
	pkgbin->maintainer = NULL;
	pkgbin->source = NULL;
	pkgbin->installedsize = NULL;
	pkgbin->bugs = NULL;
	pkgbin->origin = NULL;
	dpkg_version_blank(&pkgbin->version);
	pkgbin->conffiles = NULL;
	pkgbin->arbs = NULL;
}

void
pkg_blank(struct pkginfo *pkg)
{
	pkg->status = PKG_STAT_NOTINSTALLED;
	pkg->status_dirty = false;
	pkg->eflag = PKG_EFLAG_OK;
	pkg->want = PKG_WANT_UNKNOWN;
	pkg->priority = PKG_PRIO_UNKNOWN;
	pkg->otherpriority = NULL;
	pkg->section = NULL;
	dpkg_version_blank(&pkg->configversion);
	pkg->files_list_valid = false;
	pkg->files_list_phys_offs = 0;
	pkg->files = NULL;
	pkg->archives = NULL;
	pkg->clientdata = NULL;
	pkg->trigaw.head = NULL;
	pkg->trigaw.tail = NULL;
	pkg->othertrigaw_head = NULL;
	pkg->trigpend_head = NULL;
	pkgbin_blank(&pkg->installed);
	pkgbin_blank(&pkg->available);

	/* The architectures are reset here (instead of in pkgbin_blank),
	 * because they are part of the package specification, and needed
	 * for selections. */
	pkg->installed.arch = dpkg_arch_get(DPKG_ARCH_NONE);
	pkg->installed.multiarch = PKG_MULTIARCH_NO;
	pkg->available.arch = dpkg_arch_get(DPKG_ARCH_NONE);
	pkg->available.multiarch = PKG_MULTIARCH_NO;
}

void
pkgset_blank(struct pkgset *set)
{
	set->name = NULL;
	set->depended.available = NULL;
	set->depended.installed = NULL;
	pkg_blank(&set->pkg);
	set->installed_instances = 0;
	set->pkg.set = set;
	set->pkg.arch_next = NULL;
}

/**
 * Link a pkginfo instance into a package set.
 *
 * @param set The package set to use.
 * @param pkg The package to link into the set.
 */
void
pkgset_link_pkg(struct pkgset *set, struct pkginfo *pkg)
{
	pkg->set = set;
	pkg->arch_next = set->pkg.arch_next;
	set->pkg.arch_next = pkg;
}

/**
 * Get the number of installed package instances in a package set.
 *
 * @param set The package set to use.
 *
 * @return The count of installed packages.
 */
int
pkgset_installed_instances(struct pkgset *set)
{
	return set->installed_instances;
}

/**
 * Check if a pkg is informative.
 *
 * Used by dselect and dpkg query options as an aid to decide whether to
 * display things, and by dump to decide whether to write them out.
 */
bool
pkg_is_informative(struct pkginfo *pkg, struct pkgbin *pkgbin)
{
	/* We ignore Section and Priority, as these tend to hang around. */
	if (pkgbin == &pkg->installed &&
	    (pkg->want != PKG_WANT_UNKNOWN ||
	     pkg->eflag != PKG_EFLAG_OK ||
	     pkg->status != PKG_STAT_NOTINSTALLED ||
	     dpkg_version_is_informative(&pkg->configversion)))
		return true;

	if (pkgbin->depends ||
	    str_is_set(pkgbin->description) ||
	    str_is_set(pkgbin->maintainer) ||
	    str_is_set(pkgbin->origin) ||
	    str_is_set(pkgbin->bugs) ||
	    str_is_set(pkgbin->installedsize) ||
	    str_is_set(pkgbin->source) ||
	    dpkg_version_is_informative(&pkgbin->version) ||
	    pkgbin->conffiles ||
	    pkgbin->arbs)
		return true;

	return false;
}
