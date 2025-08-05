/*
 * libdpkg - Debian packaging suite library routines
 * depcon.c - dependency and conflict checking
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
 * Copyright © 2009 Canonical Ltd.
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

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>

bool
versionsatisfied(struct pkgbin *it, struct deppossi *against)
{
	return dpkg_version_relate(&it->version,
	                           against->verrel,
	                           &against->version);
}

/**
 * Check if the architecture qualifier in the dependency is satisfied.
 *
 * The rules are supposed to be:
 * - unqualified Depends/Pre-Depends/Recommends/Suggests are only
 *   satisfied by a package of a different architecture if the target
 *   package is Multi-Arch: foreign.
 * - Depends/Pre-Depends/Recommends/Suggests on pkg:any are satisfied by
 *   a package of a different architecture if the target package is
 *   Multi-Arch: allowed.
 * - all other Depends/Pre-Depends/Recommends/Suggests are only
 *   satisfied by packages of the same architecture.
 * - Architecture: all packages are treated the same as packages of the
 *   native architecture.
 * - Conflicts/Replaces/Breaks are assumed to apply to packages of any arch.
 */
bool
deparchsatisfied(struct pkgbin *it, const struct dpkg_arch *it_arch,
                 struct deppossi *against)
{
	const struct dpkg_arch *dep_arch, *pkg_arch;

	if (against->arch_is_implicit &&
	    it->multiarch == PKG_MULTIARCH_FOREIGN)
		return true;

	dep_arch = against->arch;
	if (dep_arch->type == DPKG_ARCH_WILDCARD &&
	    (it->multiarch == PKG_MULTIARCH_ALLOWED ||
	     against->up->type == dep_conflicts ||
	     against->up->type == dep_replaces ||
	     against->up->type == dep_breaks))
		return true;

	pkg_arch = it_arch;
	if (dep_arch->type == DPKG_ARCH_NONE || dep_arch->type == DPKG_ARCH_ALL)
		dep_arch = dpkg_arch_get(DPKG_ARCH_NATIVE);
	if (pkg_arch->type == DPKG_ARCH_NONE || pkg_arch->type == DPKG_ARCH_ALL)
		pkg_arch = dpkg_arch_get(DPKG_ARCH_NATIVE);

	return (dep_arch == pkg_arch);
}

bool
archsatisfied(struct pkgbin *it, struct deppossi *against)
{
	return deparchsatisfied(it, it->arch, against);
}

/**
 * Check if the dependency is satisfied by a virtual package.
 *
 * For versioned depends, we only check providers with #DPKG_RELATION_EQ. It
 * does not make sense to check ones without a version since we have nothing
 * to verify against. Also, it is way too complex to allow anything but an
 * equal in a provided version. A few examples below to deter you from trying:
 *
 * - pkg1 depends on virt (>= 0.6), pkg2 provides virt (<= 1.0).
 *   Should pass (easy enough).
 *
 * - pkg1 depends on virt (>= 0.7) and (<= 1.1), pkg2 provides virt (>= 1.2).
 *   Should fail (little harder).
 *
 * - pkg1 depends on virt (>= 0.4), pkg2 provides virt (<= 1.0) and (>= 0.5),
 *   IOW, inclusive of only those versions. This would require backchecking
 *   the other provided versions in the possi, which would make things sickly
 *   complex and overly time consuming. Should fail (very hard to implement).
 *
 * This could be handled by switching to a SAT solver, but that would imply
 * lots of work for very little gain. Packages can easily get around most of
 * these by providing multiple #DPKG_RELATION_EQ versions.
 */
bool
pkg_virtual_deppossi_satisfied(struct deppossi *dependee,
                               struct deppossi *provider)
{
	if (provider->verrel != DPKG_RELATION_NONE &&
	    provider->verrel != DPKG_RELATION_EQ)
		return false;

	if (provider->verrel == DPKG_RELATION_NONE &&
	    dependee->verrel != DPKG_RELATION_NONE)
		return false;

	return dpkg_version_relate(&provider->version,
	                           dependee->verrel,
	                           &dependee->version);
}
