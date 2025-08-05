/*
 * libdpkg - Debian packaging suite library routines
 * b-fsys-hash.c - test fsys database load and hash performance
 *
 * Copyright © 2009-2019 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include <dpkg/perf.h>

#include <dpkg/db-fsys.h>

static const char *admindir;

int
main(int argc, const char *const *argv)
{
	struct perf_slot ps;

	push_error_context();
	setvbuf(stdout, NULL, _IONBF, 0);

	admindir = dpkg_db_set_dir(admindir);

	perf_ts_mark_print("init");

	perf_ts_slot_start(&ps);
	fsys_hash_init();
	perf_ts_slot_stop(&ps);

	perf_ts_slot_print(&ps, "fsys_hash_init");

	perf_ts_slot_start(&ps);
	modstatdb_open(msdbrw_readonly | msdbrw_available_readonly);
	perf_ts_slot_stop(&ps);

	perf_ts_slot_print(&ps, "modstatdb_init");

	perf_ts_slot_start(&ps);
	ensure_allinstfiles_available_quiet();
	perf_ts_slot_stop(&ps);

	perf_ts_slot_print(&ps, "load .list");

	if (test_is_verbose()) {
		pkg_hash_report(stdout);
		fsys_hash_report(stdout);
	}

	modstatdb_shutdown();
	pop_error_context(ehflag_normaltidy);

	perf_ts_mark_print("shutdown");

	return 0;
}
