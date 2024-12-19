/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdlib.h>
#include <gtk/gtk.h>

#include "gs-plugin-loader-sync.h"
#include "gs-test.h"

/**
 * gs_test_init:
 *
 * Initializes the environment with the common settings for the test,
 * as a replacement for the g_test_init(), which is called as well.
 *
 * Since: 42
 **/
void
gs_test_init (gint *pargc,
	      gchar ***pargv)
{
	g_autoptr(GSettings) settings = NULL;

	g_setenv ("GSETTINGS_BACKEND", "memory", FALSE);
	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);

	/* To not download ODRS data during the test */
	settings = g_settings_new ("org.lingmo.appstore");
	g_settings_set_string (settings, "review-server", "");

	g_test_init (pargc, pargv,
		     G_TEST_OPTION_ISOLATE_DIRS,
		     NULL);

	/* only critical and error are fatal */
	g_log_set_fatal_mask (NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
}

gchar *
gs_test_get_filename (const gchar *testdatadir, const gchar *filename)
{
	gchar *tmp;
	char full_tmp[PATH_MAX];
	g_autofree gchar *path = NULL;
	path = g_build_filename (testdatadir, filename, NULL);
	g_debug ("looking in %s", path);
	tmp = realpath (path, full_tmp);
	if (tmp == NULL)
		return NULL;
	return g_strdup (full_tmp);
}

void
gs_test_flush_main_context (void)
{
	guint cnt = 0;
	while (g_main_context_iteration (NULL, FALSE)) {
		if (cnt == 0)
			g_debug ("clearing pending events...");
		cnt++;
	}
	if (cnt > 0)
		g_debug ("cleared %u events", cnt);
}

/**
 * gs_test_expose_icon_theme_paths:
 *
 * Calculate and set the `GS_SELF_TEST_ICON_THEME_PATH` environment variable
 * to include the current system icon theme paths. This is designed to be called
 * before calling `gs_test_init()`, which will clear the system icon theme paths.
 *
 * As this function calls `g_setenv()`, it must not be called after threads have
 * been spawned.
 *
 * Calling this function is an explicit acknowledgement that the code under test
 * should be accessing the icon theme.
 *
 * Since: 3.38
 */
void
gs_test_expose_icon_theme_paths (void)
{
	GdkDisplay *display = gdk_display_get_default ();
	const gchar * const *data_dirs;
	g_autoptr(GString) data_dirs_str = NULL;
	g_autofree gchar *data_dirs_joined = NULL;

	data_dirs = g_get_system_data_dirs ();
	data_dirs_str = g_string_new ("");
	for (gsize i = 0; data_dirs[i] != NULL; i++)
		g_string_append_printf (data_dirs_str, "%s%s/icons",
					(data_dirs_str->len > 0) ? ":" : "",
					data_dirs[i]);
	data_dirs_joined = g_string_free (g_steal_pointer (&data_dirs_str), FALSE);
	g_setenv ("GS_SELF_TEST_ICON_THEME_PATH", data_dirs_joined, TRUE);

	if (display) {
		GtkIconTheme *default_theme;
		default_theme = gtk_icon_theme_get_for_display (display);
		gtk_icon_theme_add_resource_path (default_theme, "/org/lingmo/Appstore/icons/");
	}
}

/**
 * gs_test_reinitialise_plugin_loader:
 * @plugin_loader: a #GsPluginLoader
 *
 * Calls setup on each plugin. This should only be used from the self tests
 * and in a controlled way.
 *
 * Since: 42
 */
void
gs_test_reinitialise_plugin_loader (GsPluginLoader      *plugin_loader,
                                    const gchar * const *allowlist,
                                    const gchar * const *blocklist)
{
	g_autoptr(GError) local_error = NULL;
#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec G_GNUC_UNUSED = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	/* Shut down */
	gs_plugin_loader_shutdown (plugin_loader, NULL);

	/* clear global cache */
	gs_plugin_loader_clear_caches (plugin_loader);

	/* remove any events */
	gs_plugin_loader_remove_events (plugin_loader);

	/* Start all the plugins setting up again in parallel. Use the blocking
	 * sync version of the function, just for the tests. */
	gs_plugin_loader_setup (plugin_loader, allowlist, blocklist, NULL, &local_error);
	g_assert_no_error (local_error);

#ifdef HAVE_SYSPROF
	if (plugin_loader->sysprof_writer != NULL) {
		sysprof_capture_writer_add_mark (plugin_loader->sysprof_writer,
						 begin_time_nsec,
						 sched_getcpu (),
						 getpid (),
						 SYSPROF_CAPTURE_CURRENT_TIME - begin_time_nsec,
						 "lingmo-appstore",
						 "setup-again",
						 NULL);
	}
#endif  /* HAVE_SYSPROF */
}
