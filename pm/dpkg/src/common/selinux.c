/*
 * dpkg - main program for package management
 * selinux.c - SELinux support
 *
 * Copyright © 2007-2015 Guillem Jover <guillem@debian.org>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#ifdef WITH_LIBSELINUX
#include <selinux/selinux.h>
#include <selinux/avc.h>
#include <selinux/label.h>
#endif

#include "force.h"
#include "security-mac.h"

#ifdef WITH_LIBSELINUX
static struct selabel_handle *sehandle;
#endif

#ifdef WITH_LIBSELINUX
static int DPKG_ATTR_PRINTF(2)
log_callback(int type, const char *fmt, ...)
{
	va_list ap;
	char *msg;

	switch (type) {
	case SELINUX_ERROR:
	case SELINUX_WARNING:
	case SELINUX_AVC:
		break;
	default:
		return 0;
	}

	va_start(ap, fmt);
	m_vasprintf(&msg, fmt, ap);
	va_end(ap);

	warning("selinux: %s", msg);
	free(msg);

	return 0;
}
#endif

void
dpkg_selabel_load(void)
{
#ifdef WITH_LIBSELINUX
	static int selinux_enabled = -1;

	if (selinux_enabled < 0) {
		int rc;

		/* Set selinux_enabled if it is not already set (singleton). */
		selinux_enabled = (in_force(FORCE_SECURITY_MAC) &&
		                   is_selinux_enabled() > 0);
		if (!selinux_enabled)
			return;

		/* Open the SELinux status notification channel, with fallback
		 * enabled for older kernels. */
		rc = selinux_status_open(1);
		if (rc < 0)
			ohshit(_("cannot open security status notification channel"));

		selinux_set_callback(SELINUX_CB_LOG, (union selinux_callback) {
			.func_log = log_callback,
		});
	} else if (selinux_enabled && selinux_status_updated()) {
		/* The SELinux policy got updated in the kernel, usually after
		 * upgrading the package shipping it, we need to reload. */
		selabel_close(sehandle);
	} else {
		/* SELinux is either disabled or it does not need a reload. */
		return;
	}

	sehandle = selabel_open(SELABEL_CTX_FILE, NULL, 0);
	if (sehandle == NULL && security_getenforce() == 1)
		ohshite(_("cannot get security labeling handle"));
#endif
}

void
dpkg_selabel_set_context(const char *matchpath, const char *path, mode_t mode)
{
#ifdef WITH_LIBSELINUX
	char *scontext = NULL;
	int ret;

	/* If SELinux is not enabled just do nothing. */
	if (sehandle == NULL)
		return;

	/*
	 * We use the _raw function variants here so that no translation
	 * happens from computer to human readable forms, to avoid issues
	 * when mcstransd has disappeared during the unpack process.
	 */

	/* Do nothing if we can't figure out what the context is, or if it has
	 * no context; in which case the default context shall be applied. */
	ret = selabel_lookup_raw(sehandle, &scontext, matchpath, mode & S_IFMT);
	if (ret < 0 || (ret == 0 && scontext == NULL))
		return;

	ret = lsetfilecon_raw(path, scontext);
	if (ret < 0 && errno != ENOTSUP)
		ohshite(_("cannot set security context for file object '%s'"),
		        path);

	freecon(scontext);
#endif /* WITH_LIBSELINUX */
}

void
dpkg_selabel_close(void)
{
#ifdef WITH_LIBSELINUX
	if (sehandle == NULL)
		return;

	selinux_status_close();
	selabel_close(sehandle);
	sehandle = NULL;
#endif
}
