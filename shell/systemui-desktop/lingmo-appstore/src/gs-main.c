/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <sys/stat.h>

#include "gs-application.h"
#include "gs-debug.h"

int
main (int argc, char **argv)
{
	int status = 0;
	g_autoptr(GDesktopAppInfo) appinfo = NULL;
	g_autoptr(GsApplication) application = NULL;
	g_autoptr(GsDebug) debug = gs_debug_new_from_environment ();

	setlocale (LC_ALL, "");

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Override the umask to 022 to make it possible to share files between
	 * the lingmo-appstore process and flatpak system helper process.
	 * Ideally this should be set when needed in the flatpak plugin, but
	 * umask is thread-unsafe so there is really no local way to fix this.
	 */
	umask (022);

	/* redirect logs */
	application = gs_application_new (debug);
	appinfo = g_desktop_app_info_new ("org.lingmo.Appstore.desktop");
	if (appinfo != NULL)
		g_set_application_name (g_app_info_get_name (G_APP_INFO (appinfo)));
	status = g_application_run (G_APPLICATION (application), argc, argv);
	return status;
}
