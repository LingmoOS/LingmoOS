/*
 * libdpkg - Debian packaging suite library routines
 * pkg-namevalue.c - name/value package tables
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
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

#include <dpkg/dpkg-db.h>
#include <dpkg/namevalue.h>

const struct namevalue booleaninfos[] = {
	NAMEVALUE_DEF("no",			false),
	NAMEVALUE_DEF("yes",			true),
	{ .name = NULL }
};

const struct namevalue multiarchinfos[] = {
	NAMEVALUE_DEF("no",			PKG_MULTIARCH_NO),
	NAMEVALUE_DEF("same",			PKG_MULTIARCH_SAME),
	NAMEVALUE_DEF("allowed",		PKG_MULTIARCH_ALLOWED),
	NAMEVALUE_DEF("foreign",		PKG_MULTIARCH_FOREIGN),
	{ .name = NULL }
};

const struct namevalue priorityinfos[] = {
	NAMEVALUE_DEF("required",		PKG_PRIO_REQUIRED),
	NAMEVALUE_DEF("important",		PKG_PRIO_IMPORTANT),
	NAMEVALUE_DEF("standard",		PKG_PRIO_STANDARD),
	NAMEVALUE_DEF("optional",		PKG_PRIO_OPTIONAL),
	NAMEVALUE_DEF("extra",			PKG_PRIO_EXTRA),
	NAMEVALUE_DEF("unknown",		PKG_PRIO_UNKNOWN),
	{ .name = NULL }
};

const struct namevalue wantinfos[] = {
	NAMEVALUE_DEF("unknown",		PKG_WANT_UNKNOWN),
	NAMEVALUE_DEF("install",		PKG_WANT_INSTALL),
	NAMEVALUE_DEF("hold",			PKG_WANT_HOLD),
	NAMEVALUE_DEF("deinstall",		PKG_WANT_DEINSTALL),
	NAMEVALUE_DEF("purge",			PKG_WANT_PURGE),
	{ .name = NULL }
};

const struct namevalue eflaginfos[] = {
	NAMEVALUE_DEF("ok",			PKG_EFLAG_OK),
	NAMEVALUE_DEF("reinstreq",		PKG_EFLAG_REINSTREQ),
	{ .name = NULL }
};

const struct namevalue statusinfos[] = {
	NAMEVALUE_DEF("not-installed",		PKG_STAT_NOTINSTALLED),
	NAMEVALUE_DEF("config-files",		PKG_STAT_CONFIGFILES),
	NAMEVALUE_DEF("half-installed",		PKG_STAT_HALFINSTALLED),
	NAMEVALUE_DEF("unpacked",		PKG_STAT_UNPACKED),
	NAMEVALUE_DEF("half-configured",	PKG_STAT_HALFCONFIGURED),
	NAMEVALUE_DEF("triggers-awaited",	PKG_STAT_TRIGGERSAWAITED),
	NAMEVALUE_DEF("triggers-pending",	PKG_STAT_TRIGGERSPENDING),
	NAMEVALUE_DEF("installed",		PKG_STAT_INSTALLED),
	{ .name = NULL }
};
