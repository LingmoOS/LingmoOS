/*
 * dpkg - main program for package management
 * actions.h - command action definition list
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2016 Guillem Jover <guillem@debian.org>
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

#ifndef DPKG_ACTIONS_H
#define DPKG_ACTIONS_H

enum action {
	act_unset,

	act_unpack,
	act_configure,
	act_install,
	act_triggers,
	act_remove,
	act_purge,
	act_verify,
	act_commandfd,

	act_status,
	act_listpackages,
	act_listfiles,
	act_searchfiles,
	act_controlpath,
	act_controllist,
	act_controlshow,

	act_cmpversions,

	act_arch_add,
	act_arch_remove,
	act_printarch,
	act_printforeignarches,

	act_assert_feature,

	act_validate_pkgname,
	act_validate_trigname,
	act_validate_archname,
	act_validate_version,

	act_audit,
	act_unpackchk,
	act_predeppackage,

	act_getselections,
	act_setselections,
	act_clearselections,

	act_avail,
	act_printavail,
	act_avclear,
	act_avreplace,
	act_avmerge,
	act_forgetold,

	act_help,
	act_version,
};

#endif /* DPKG_ACTIONS_H */
