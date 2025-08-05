/*
 * libdpkg - Debian packaging suite library routines
 * i18n.c - i18n support
 *
 * Copyright © 2013 Guillem Jover <guillem@debian.org>
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <dpkg/i18n.h>

#ifdef HAVE_USELOCALE
#ifdef HAVE_XLOCALE_H
#include <xlocale.h>
#endif
static locale_t dpkg_C_locale;
#endif

static bool
dpkg_use_nls(void)
{
	const char *env;

	/* We mimic the behavior of the Dpkg::Gettext perl module. */
	env = getenv("DPKG_NLS");
	if (env == NULL)
		return true;

	if (strcmp(env, "0") == 0 || env[0] == '\0')
		return false;

	return true;
}

void
dpkg_locales_init(const char *package)
{
	if (dpkg_use_nls())
		setlocale(LC_ALL, "");
	bindtextdomain(package, LOCALEDIR);
	textdomain(package);

#ifdef HAVE_USELOCALE
	dpkg_C_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
#endif

#if defined(__APPLE__) && defined(__MACH__)
	/*
	 * On macOS, the libintl code needs to call into the CoreFoundation
	 * framework, which is internally threaded, to initialize some caches.
	 * This is a problem when that first call is done after a fork(3),
	 * because per POSIX, only one thread will survive, leaving the
	 * process in a very inconsistent state, leading to crashes.
	 *
	 * XXX: To workaround this, we try to force the caches initialization
	 * at program startup time, by performing a dummy gettext() call.
	 */
	gettext("");
#endif
}

void
dpkg_locales_done(void)
{
#ifdef HAVE_USELOCALE
	freelocale(dpkg_C_locale);
	dpkg_C_locale = (locale_t)0;
#endif
}

struct dpkg_locale
dpkg_locale_switch_C(void)
{
	struct dpkg_locale loc;

#ifdef HAVE_USELOCALE
	loc.oldloc = uselocale(dpkg_C_locale);
#else
	loc.oldloc = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "C");
#endif

	return loc;
}

void
dpkg_locale_switch_back(struct dpkg_locale loc)
{
#ifdef HAVE_USELOCALE
	uselocale(loc.oldloc);
#else
	setlocale(LC_ALL, loc.oldloc);
#endif
}
