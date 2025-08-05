/*
 * dpkg - main program for package management
 * force.h - forced operation support
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2019 Guillem Jover <guillem@debian.org>
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

#ifndef DPKG_FORCE_H
#define DPKG_FORCE_H

#include <stdbool.h>

#include <dpkg/dpkg.h>
#include <dpkg/options.h>

enum DPKG_ATTR_ENUM_FLAGS force_flags {
	FORCE_ARCHITECTURE = DPKG_BIT(0),
	FORCE_BAD_PATH = DPKG_BIT(1),
	FORCE_BAD_VERIFY = DPKG_BIT(2),
	FORCE_BAD_VERSION = DPKG_BIT(3),
	FORCE_BREAKS = DPKG_BIT(4),
	FORCE_CONFF_ASK = DPKG_BIT(5),
	FORCE_CONFF_DEF = DPKG_BIT(6),
	FORCE_CONFF_MISS = DPKG_BIT(7),
	FORCE_CONFF_NEW = DPKG_BIT(8),
	FORCE_CONFF_OLD = DPKG_BIT(9),
	FORCE_CONFIGURE_ANY = DPKG_BIT(10),
	FORCE_CONFLICTS = DPKG_BIT(11),
	FORCE_DEPENDS = DPKG_BIT(12),
	FORCE_DEPENDS_VERSION = DPKG_BIT(13),
	FORCE_DOWNGRADE = DPKG_BIT(14),
	FORCE_HOLD = DPKG_BIT(15),
	FORCE_NON_ROOT = DPKG_BIT(16),
	FORCE_OVERWRITE = DPKG_BIT(17),
	FORCE_OVERWRITE_DIR = DPKG_BIT(18),
	FORCE_OVERWRITE_DIVERTED = DPKG_BIT(19),
	FORCE_REMOVE_ESSENTIAL = DPKG_BIT(20),
	FORCE_REMOVE_REINSTREQ = DPKG_BIT(21),
	FORCE_SCRIPT_CHROOTLESS = DPKG_BIT(22),
	FORCE_UNSAFE_IO = DPKG_BIT(23),
	FORCE_STATOVERRIDE_ADD = DPKG_BIT(24),
	FORCE_STATOVERRIDE_DEL = DPKG_BIT(25),
	FORCE_SECURITY_MAC = DPKG_BIT(26),
	FORCE_REMOVE_PROTECTED = DPKG_BIT(27),
	FORCE_ALL = 0xffffffff,
};

bool
in_force(int flags);
void
set_force(int flags);
void
reset_force(int flags);

char *
get_force_string(void);

void
parse_force(const char *value, bool set);

void
set_force_default(int mask);
void
set_force_option(const struct cmdinfo *cip, const char *value);
void
reset_force_option(const struct cmdinfo *cip, const char *value);

void
forcibleerr(int forceflag, const char *format, ...) DPKG_ATTR_PRINTF(2);
int
forcible_nonroot_error(int rc);

#endif /* DPKG_FORCE_H */
