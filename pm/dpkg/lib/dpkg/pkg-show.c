/*
 * libdpkg - Debian packaging suite library routines
 * pkg-show.c - primitives for pkg information display
 *
 * Copyright © 1995,1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <string.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-show.h>

static bool
pkgbin_name_needs_arch(const struct pkgbin *pkgbin,
                       enum pkg_name_arch_when pnaw)
{
	if (pkgbin->arch->type == DPKG_ARCH_NONE ||
	    pkgbin->arch->type == DPKG_ARCH_EMPTY)
		return false;

	switch (pnaw) {
	case pnaw_never:
		break;
	case pnaw_same:
		if (pkgbin->multiarch == PKG_MULTIARCH_SAME)
			return true;
		return false;
	case pnaw_nonambig:
		if (pkgbin->multiarch == PKG_MULTIARCH_SAME)
			return true;
	/* Fall through. */
	case pnaw_foreign:
		if (pkgbin->arch->type == DPKG_ARCH_NATIVE ||
		    pkgbin->arch->type == DPKG_ARCH_ALL)
			break;
	/* Fall through. */
	case pnaw_always:
		return true;
	}

	return false;
}

/**
 * Add a string representation of the package name to a varbuf.
 *
 * Works exactly like pkgbin_name() but acts on the varbuf instead of
 * returning a string. It NUL terminates the varbuf.
 *
 * @param vb      The varbuf struct to modify.
 * @param pkg     The package to consider.
 * @param pkgbin  The binary package instance to consider.
 * @param pnaw    When to display the architecture qualifier.
 */
void
varbuf_add_pkgbin_name(struct varbuf *vb,
                       const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                       enum pkg_name_arch_when pnaw)
{
	varbuf_add_str(vb, pkg->set->name);
	if (pkgbin_name_needs_arch(pkgbin, pnaw))
		varbuf_add_archqual(vb, pkgbin->arch);
}

const char *
pkgbin_name_archqual(const struct pkginfo *pkg, const struct pkgbin *pkgbin)
{
	char *pkgname;

	if (pkgbin->arch->type == DPKG_ARCH_NONE ||
	    pkgbin->arch->type == DPKG_ARCH_EMPTY)
		return pkg->set->name;

	pkgname = nfmalloc(strlen(pkg->set->name) + 1 +
	                   strlen(pkgbin->arch->name) + 1);
	str_concat(pkgname, pkg->set->name, ":",
	                    pkgbin->arch->name, NULL);

	return pkgname;
}

/**
 * Return a string representation of the package name.
 *
 * The returned string must not be freed, and it's permanently allocated so
 * can be used as long as the non-freeing memory pool has not been freed.
 *
 * Note, that this const variant will "leak" a new non-freeing string on
 * each call if the internal cache has not been previously initialized,
 * so it is advised to use it only in error reporting code paths.
 *
 * The pnaw parameter should be one of pnaw_never (never print arch),
 * pnaw_foreign (print arch for foreign packages only), pnaw_nonambig (print
 * arch for non ambiguous cases) or pnaw_always (always print arch),
 *
 * @param pkg     The package to consider.
 * @param pkgbin  The binary package instance to consider.
 * @param pnaw    When to display the architecture qualifier.
 *
 * @return The string representation.
 */
const char *
pkgbin_name_const(const struct pkginfo *pkg, const struct pkgbin *pkgbin,
            enum pkg_name_arch_when pnaw)
{
	if (!pkgbin_name_needs_arch(pkgbin, pnaw))
		return pkg->set->name;

	/* Return a non-freeing package name representation, which
	 * is intended to be used in error-handling code, as we will keep
	 * "leaking" them until the next memory pool flush. */
	if (pkgbin->pkgname_archqual == NULL)
		return pkgbin_name_archqual(pkg, pkgbin);

	return pkgbin->pkgname_archqual;
}

/**
 * Return a string representation of the installed package name.
 *
 * This is equivalent to pkgbin_name_const() but just for its installed pkgbin.
 *
 * @param pkg   The package to consider.
 * @param pnaw  When to display the architecture qualifier.
 *
 * @return The string representation.
 */
const char *
pkg_name_const(const struct pkginfo *pkg, enum pkg_name_arch_when pnaw)
{
	return pkgbin_name_const(pkg, &pkg->installed, pnaw);
}

/**
 * Return a string representation of the package name.
 *
 * The returned string must not be freed, and it's permanently allocated so
 * can be used as long as the non-freeing memory pool has not been freed.
 *
 * The pnaw parameter should be one of pnaw_never (never print arch),
 * pnaw_foreign (print arch for foreign packages only), pnaw_nonambig (print
 * arch for non ambiguous cases) or pnaw_always (always print arch),
 *
 * @param pkg     The package to consider.
 * @param pkgbin  The binary package instance to consider.
 * @param pnaw    When to display the architecture qualifier.
 *
 * @return The string representation.
 */
const char *
pkgbin_name(struct pkginfo *pkg, struct pkgbin *pkgbin,
            enum pkg_name_arch_when pnaw)
{
	if (!pkgbin_name_needs_arch(pkgbin, pnaw))
		return pkg->set->name;

	/* Cache the package name representation, for later reuse. */
	if (pkgbin->pkgname_archqual == NULL)
		pkgbin->pkgname_archqual = pkgbin_name_archqual(pkg, pkgbin);

	return pkgbin->pkgname_archqual;
}

/**
 * Return a string representation of the installed package name.
 *
 * This is equivalent to pkgbin_name() but just for its installed pkgbin.
 *
 * @param pkg   The package to consider.
 * @param pnaw  When to display the architecture qualifier.
 *
 * @return The string representation.
 */
const char *
pkg_name(struct pkginfo *pkg, enum pkg_name_arch_when pnaw)
{
	return pkgbin_name(pkg, &pkg->installed, pnaw);
}

/**
 * Return a string representation of the package synopsis.
 *
 * The returned string must not be freed, and it's permanently allocated so
 * can be used as long as the non-freeing memory pool has not been freed.
 *
 * The package synopsis is the short description, but it is not NUL terminated,
 * so the output len argument should be used to limit the string length.
 *
 * @param pkg      The package to consider.
 * @param pkgbin   The binary package instance to consider.
 * @param[out] len The length of the synopsis string within the description.
 *
 * @return The string representation.
 */
const char *
pkgbin_synopsis(const struct pkginfo *pkg, const struct pkgbin *pkgbin, int *len)
{
	const char *pdesc;

	pdesc = pkgbin->description;
	if (!pdesc)
		pdesc = _("(no description available)");

	*len = strcspn(pdesc, "\n");

	return pdesc;
}

/**
 * Return a string representation of the package synopsis.
 *
 * The returned string must not be freed, and it's permanently allocated so
 * can be used as long as the non-freeing memory pool has not been freed.
 *
 * It will try to use the installed version, otherwise it will fallback to
 * use the available version.
 *
 * The package synopsis is the short description, but it is not NUL terminated,
 * so the output len argument should be used to limit the string length.
 *
 * @param pkg      The package to consider.
 * @param[out] len The length of the synopsis string within the description.
 *
 * @return The string representation.
 */
const char *
pkg_synopsis(const struct pkginfo *pkg, int *len)
{
	const char *pdesc;

	pdesc = pkg->installed.description;
	if (!pdesc)
		pdesc = pkg->available.description;
	if (!pdesc)
		pdesc = _("(no description available)");

	*len = strcspn(pdesc, "\n");

	return pdesc;
}

/**
 * Return a character abbreviated representation of the package want status.
 *
 * @param pkg The package to consider.
 *
 * @return The character abbreviated representation.
 */
int
pkg_abbrev_want(const struct pkginfo *pkg)
{
	return "uihrp"[pkg->want];
}

/**
 * Return a character abbreviated representation of the package current status.
 *
 * @param pkg The package to consider.
 *
 * @return The character abbreviated representation.
 */
int
pkg_abbrev_status(const struct pkginfo *pkg)
{
	return "ncHUFWti"[pkg->status];
}

/**
 * Return a character abbreviated representation of the package eflag status.
 *
 * @param pkg The package to consider.
 *
 * @return The character abbreviated representation.
 */
int
pkg_abbrev_eflag(const struct pkginfo *pkg)
{
	return " R"[pkg->eflag];
}

/**
 * Return a string representation of the package want status name.
 *
 * @param pkg The package to consider.
 *
 * @return The string representation.
 */
const char *
pkg_want_name(const struct pkginfo *pkg)
{
	return wantinfos[pkg->want].name;
}

/**
 * Return a string representation of the package eflag status name.
 *
 * @param pkg The package to consider.
 *
 * @return The string representation.
 */
const char *
pkg_eflag_name(const struct pkginfo *pkg)
{
	return eflaginfos[pkg->eflag].name;
}

/**
 * Return a string representation of the package current status name.
 *
 * @param pkg The package to consider.
 *
 * @return The string representation.
 */
const char *
pkg_status_name(const struct pkginfo *pkg)
{
	return statusinfos[pkg->status].name;
}

/**
 * Return a string representation of the package priority name.
 *
 * @param pkg The package to consider.
 *
 * @return The string representation.
 */
const char *
pkg_priority_name(const struct pkginfo *pkg)
{
	if (pkg->priority == PKG_PRIO_OTHER)
		return pkg->otherpriority;
	else
		return priorityinfos[pkg->priority].name;
}

/**
 * Compare a package to be sorted by non-ambiguous name and architecture.
 *
 * @param a A pointer of a pointer to a struct pkginfo.
 * @param b A pointer of a pointer to a struct pkginfo.
 *
 * @return An integer with the result of the comparison.
 * @retval -1 a is earlier than b.
 * @retval 0 a is equal to b.
 * @retval 1 a is later than b.
 */
int
pkg_sorter_by_nonambig_name_arch(const void *a, const void *b)
{
	const struct pkginfo *pa = *(const struct pkginfo **)a;
	const struct pkginfo *pb = *(const struct pkginfo **)b;
	const struct pkgbin *pbina = &pa->installed;
	const struct pkgbin *pbinb = &pb->installed;
	int res;

	res = strcmp(pa->set->name, pb->set->name);
	if (res)
		return res;

	if (pbina->arch == pbinb->arch)
		return 0;

	if (pkgbin_name_needs_arch(pbina, pnaw_nonambig)) {
		if (pkgbin_name_needs_arch(pbinb, pnaw_nonambig))
			return strcmp(pbina->arch->name, pbinb->arch->name);
		else
			return 1;
	} else {
		return -1;
	}
}

/**
 * Add a string representation of the source package version to a varbuf.
 *
 * It parses the Source field (if present), and extracts the optional
 * version enclosed in parenthesis. Otherwise it falls back to use the
 * binary package version. It NUL terminates the varbuf.
 *
 * @param vb      The varbuf struct to modify.
 * @param pkg     The package to consider.
 * @param pkgbin  The binary package instance to consider.
 */
void
varbuf_add_source_version(struct varbuf *vb,
                          const struct pkginfo *pkg, const struct pkgbin *pkgbin)
{
	struct dpkg_version version = DPKG_VERSION_INIT;

	pkg_source_version(&version, pkg, pkgbin);
	varbufversion(vb, &version, vdew_nonambig);
}

void
pkg_source_version(struct dpkg_version *version,
                   const struct pkginfo *pkg, const struct pkgbin *pkgbin)
{
	const char *version_str;

	if (pkgbin->source)
		version_str = strchr(pkgbin->source, '(');
	else
		version_str = NULL;

	if (version_str == NULL) {
		*version = pkgbin->version;
	} else {
		struct dpkg_error err;
		struct varbuf vb = VARBUF_INIT;
		size_t len;

		version_str++;
		len = strcspn(version_str, ")");
		varbuf_add_buf(&vb, version_str, len);

		if (parseversion(version, varbuf_str(&vb), &err) < 0)
			ohshit(_("version '%s' has bad syntax: %s"),
			       varbuf_str(&vb), err.str);

		varbuf_destroy(&vb);
	}
}
