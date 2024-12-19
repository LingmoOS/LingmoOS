/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <gio/gio.h>
#include <stdlib.h>

#define GS_BINARY_NAME			"lingmo-appstore"
#define GS_DBUS_BUS_NAME		"org.lingmo.Appstore"
#define GS_DBUS_OBJECT_PATH		"/org/lingmo/Appstore"
#define GS_DBUS_INTERFACE_NAME		"org.gtk.Actions"

typedef struct {
	GMainLoop	*loop;
	GDBusConnection	*connection;
	gboolean	 is_running;
	gboolean	 timed_out;
} GsRestarterPrivate;

static void
gs_restarter_on_name_appeared_cb (GDBusConnection *connection,
				  const gchar *name,
				  const gchar *name_owner,
				  gpointer user_data)
{
	GsRestarterPrivate *priv = (GsRestarterPrivate *) user_data;
	priv->is_running = TRUE;
	g_debug ("%s appeared", GS_DBUS_BUS_NAME);
	if (g_main_loop_is_running (priv->loop))
		g_main_loop_quit (priv->loop);
}

static void
gs_restarter_on_name_vanished_cb (GDBusConnection *connection,
				  const gchar *name,
				  gpointer user_data)
{
	GsRestarterPrivate *priv = (GsRestarterPrivate *) user_data;
	priv->is_running = FALSE;
	g_debug ("%s vanished", GS_DBUS_BUS_NAME);
	if (g_main_loop_is_running (priv->loop))
		g_main_loop_quit (priv->loop);
}

static gboolean
gs_restarter_loop_timeout_cb (gpointer user_data)
{
	GsRestarterPrivate *priv = (GsRestarterPrivate *) user_data;
	priv->timed_out = TRUE;
	g_main_loop_quit (priv->loop);
	return TRUE;
}

static gboolean
gs_restarter_wait_for_timeout (GsRestarterPrivate *priv,
			       guint timeout_ms,
			       GError **error)
{
	guint timer_id;
	priv->timed_out = FALSE;
	timer_id = g_timeout_add (timeout_ms, gs_restarter_loop_timeout_cb, priv);
	g_main_loop_run (priv->loop);
	g_source_remove (timer_id);
	if (priv->timed_out) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_TIMED_OUT,
			     "Waited for %ums", timeout_ms);
		return FALSE;
	}
	return TRUE;
}

static GsRestarterPrivate *
gs_restarter_private_new (void)
{
	GsRestarterPrivate *priv = g_new0 (GsRestarterPrivate, 1);
	priv->loop = g_main_loop_new (NULL, FALSE);
	return priv;
}

static void
gs_restarter_private_free (GsRestarterPrivate *priv)
{
	if (priv->connection != NULL)
		g_object_unref (priv->connection);
	g_main_loop_unref (priv->loop);
	g_free (priv);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GsRestarterPrivate, gs_restarter_private_free)

static gboolean
gs_restarter_create_new_process (GsRestarterPrivate *priv, GError **error)
{
	g_autofree gchar *binary_filename = NULL;

	/* start executable */
	binary_filename = g_build_filename (BINDIR, GS_BINARY_NAME, NULL);
	g_debug ("starting new binary %s", binary_filename);
	if (!g_spawn_command_line_async (binary_filename, error))
		return FALSE;

	/* wait for the bus name to appear */
	if (!gs_restarter_wait_for_timeout (priv, 15000, error)) {
		g_prefix_error (error, "%s did not appear: ", GS_DBUS_BUS_NAME);
		return FALSE;
	}

	return TRUE;
}

static gboolean
gs_restarter_destroy_old_process (GsRestarterPrivate *priv, GError **error)
{
	GVariantBuilder args_params;
	GVariantBuilder args_platform_data;
	g_autoptr(GVariant) reply = NULL;

	/* call a GtkAction */
	g_variant_builder_init (&args_params, g_variant_type_new ("av"));
	g_variant_builder_init (&args_platform_data, g_variant_type_new ("a{sv}"));
	reply = g_dbus_connection_call_sync (priv->connection,
					     GS_DBUS_BUS_NAME,
					     GS_DBUS_OBJECT_PATH,
					     GS_DBUS_INTERFACE_NAME,
					     "Activate",
					     g_variant_new ("(sava{sv})",
							    "shutdown",
							    &args_params,
							    &args_platform_data),
					     NULL,
					     G_DBUS_CALL_FLAGS_NO_AUTO_START,
					     5000,
					     NULL,
					     error);
	if (reply == NULL) {
		g_prefix_error (error, "Failed to send RequestShutdown: ");
		return FALSE;
	}

	/* wait for the name to disappear from the bus */
	if (!gs_restarter_wait_for_timeout (priv, 30000, error)) {
		g_prefix_error (error, "Failed to see %s vanish: ", GS_DBUS_BUS_NAME);
		return FALSE;
	}

	return TRUE;
}

static gboolean
gs_restarter_setup_watcher (GsRestarterPrivate *priv, GError **error)
{
	/* watch the name appear and vanish */
	priv->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);
	if (priv->connection == NULL) {
		g_prefix_error (error, "Failed to get D-Bus connection: ");
		return FALSE;
	}
	g_bus_watch_name_on_connection (priv->connection,
					GS_DBUS_BUS_NAME,
					G_BUS_NAME_WATCHER_FLAGS_NONE,
					gs_restarter_on_name_appeared_cb,
					gs_restarter_on_name_vanished_cb,
					priv,
					NULL);

	/* wait for one of the callbacks to be called */
	if (!gs_restarter_wait_for_timeout (priv, 50, error)) {
		g_prefix_error (error, "Failed to watch %s: ", GS_DBUS_BUS_NAME);
		return FALSE;
	}

	return TRUE;
}

int
main (int argc, char **argv)
{
	g_autoptr(GsRestarterPrivate) priv = NULL;
	g_autoptr(GError) error = NULL;

	/* show all debugging */
	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);

	/* set up the watcher */
	priv = gs_restarter_private_new ();
	if (!gs_restarter_setup_watcher (priv, &error)) {
		g_warning ("Failed to set up: %s", error->message);
		return EXIT_FAILURE;
	}

	/* kill the old process */
	if (priv->is_running) {
		if (!gs_restarter_destroy_old_process (priv, &error)) {
			g_warning ("Failed to quit service: %s", error->message);
			return EXIT_FAILURE;
		}
	}

	/* start a new process */
	if (!gs_restarter_create_new_process (priv, &error)) {
		g_warning ("Failed to start service: %s", error->message);
		return EXIT_FAILURE;
	}

	/* success */
	g_debug ("%s process successfully restarted", GS_DBUS_BUS_NAME);
	return EXIT_SUCCESS;
}
