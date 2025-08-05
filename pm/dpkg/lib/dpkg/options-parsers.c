/*
 * libdpkg - Debian packaging suite library routines
 * options-parsers.c - command-line options parser helpers
 *
 * Copyright © 2014 Guillem Jover <guillem@debian.org>
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

#include <dpkg/i18n.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/error.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/options.h>

/**
 * Parse an argument as a package name.
 *
 * The string is parsed as a singleton package name, and a #pkginfo instance
 * is returned.
 *
 * @param cmd  The command information structure currently parsed.
 * @param name The package name argument
 *
 * @return A package instance.
 */
struct pkginfo *
dpkg_options_parse_pkgname(const struct cmdinfo *cmd, const char *name)
{
	struct pkginfo *pkg;
	struct dpkg_error err;

	pkg = pkg_spec_parse_pkg(name, &err);
	if (pkg == NULL)
		badusage(_("--%s needs a valid package name but '%.250s' is not: %s"),
		         cmd->olong, name, err.str);

	return pkg;
}
