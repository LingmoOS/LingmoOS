/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016-2019 Endless Mobile, Inc
 *
 * Authors:
 *   Joaquim Rocha <jrocha@endlessm.com>
 *   Philip Withnall <withnall@endlessm.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Appstore Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Appstore
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <lingmo-appstore.h>
#include <gs-plugin.h>
#include <gs-utils.h>
#include <math.h>
#include <ostree.h>

#include "gs-eos-updater-generated.h"
#include "gs-plugin-eos-updater.h"
#include "gs-plugin-private.h"

/*
 * SECTION:
 * Plugin to poll for, download and apply OS updates using the `eos-updater`
 * service when running on Endless OS.
 *
 * This plugin is only useful on Endless OS.
 *
 * It creates a proxy for the `eos-updater` D-Bus service, which implements a
 * basic state machine which progresses through several states in order to
 * download updates: `Ready` (doing nothing) → `Poll` (checking for updates) →
 * `Fetch` (downloading an update) → `Apply` (deploying the update’s OSTree,
 * before a reboot). Any state may transition to the `Error` state at any time,
 * and the daemon may disappear at any time.
 *
 * This plugin follows the state transitions signalled by the daemon, and
 * updates the state of a single #GsApp instance (`os_upgrade`) to reflect the
 * OS update or upgrade in the UI.
 *
 * The #GsApp instance is returned by
 * `gs_plugin_eos_updater_list_distro_upgrades_async()` or
 * `gs_plugin_add_updates()` depending on whether it contains significant
 * user visible changes, as determined by the `update-is-user-visible` property
 * on the proxy. (This in turn is set from information on the release OSTree
 * commit.) The #GsApp will be returned by at most one of these vfuncs.
 *
 * Calling gs_plugin_eos_updater_refresh_metadata_async() will result in this
 * plugin calling the `Poll()` method on the `eos-updater` daemon to check for a
 * new update.
 *
 * Calling gs_plugin_app_upgrade_download() will result in this plugin calling
 * a sequence of methods on the `eos-updater` daemon to check for, download and
 * apply an update. Typically, gs_plugin_app_upgrade_download() should be called
 * once `eos-updater` is already in the `UpdateAvailable` state. It will report
 * progress information, with the first 75 percentage points of the progress
 * reporting the download progress, and the final 25 percentage points reporting
 * the OSTree deployment progress. The final 25 percentage points are currently
 * faked because we can’t get reasonable progress data out of OSTree.
 *
 * The proxy object (`updater_proxy`) uses the thread-default main context from
 * the gs_plugin_eos_updater_setup() function, which is currently the global default main
 * context from lingmo-appstore’s main thread. This means all the signal
 * callbacks from the proxy will be executed in the main thread, and *must not
 * block*.
 *
 * Asynchronous plugin vfuncs (such as
 * gs_plugin_eos_updater_refresh_metadata_async()) are run in lingmo-appstore’s
 * main thread and *must not block*. As they all call D-Bus methods, the work
 * they do is minimal and hence is OK to happen in the main thread.
 *
 * The other functions (gs_plugin_app_upgrade_download(),
 * etc.) are called in #GTask worker threads. They are allowed to call methods
 * on the proxy; the main thread is only allowed to receive signals and check
 * properties on the proxy, to avoid blocking.
 *
 * `updater_proxy`, `os_upgrade` and `cancellable` are only set in
 * gs_plugin_eos_updater_setup(), and are both internally thread-safe — so they can both be
 * dereferenced and have their methods called from any thread without any
 * locking.
 *
 * Cancellation of any operations on the `eos-updater` daemon (polling, fetching
 * or applying) is implemented by calling the `Cancel()` method on it. This is
 * permanently connected to the private `cancellable` #GCancellable instance,
 * which persists for the lifetime of the plugin. The #GCancellable instances
 * for various operations can be temporarily chained to it for the duration of
 * each operation.
 */

static const guint max_progress_for_update = 75;  /* percent */

typedef enum {
	EOS_UPDATER_STATE_NONE = 0,
	EOS_UPDATER_STATE_READY,
	EOS_UPDATER_STATE_ERROR,
	EOS_UPDATER_STATE_POLLING,
	EOS_UPDATER_STATE_UPDATE_AVAILABLE,
	EOS_UPDATER_STATE_FETCHING,
	EOS_UPDATER_STATE_UPDATE_READY,
	EOS_UPDATER_STATE_APPLYING_UPDATE,
	EOS_UPDATER_STATE_UPDATE_APPLIED,
} EosUpdaterState;
#define EOS_UPDATER_N_STATES (EOS_UPDATER_STATE_UPDATE_APPLIED + 1)

static const gchar *
eos_updater_state_to_str (EosUpdaterState state)
{
	const gchar * const eos_updater_state_str[] = {
		"None",
		"Ready",
		"Error",
		"Polling",
		"UpdateAvailable",
		"Fetching",
		"UpdateReady",
		"ApplyingUpdate",
		"UpdateApplied",
	};

	G_STATIC_ASSERT (G_N_ELEMENTS (eos_updater_state_str) == EOS_UPDATER_N_STATES);

	g_return_val_if_fail ((gint) state < EOS_UPDATER_N_STATES, "unknown");
	return eos_updater_state_str[state];
}

static void
gs_eos_updater_error_convert (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return;

	/* parse remote eos-updater error */
	if (g_dbus_error_is_remote_error (error)) {
		g_autofree gchar *remote_error = g_dbus_error_get_remote_error (error);

		g_dbus_error_strip_remote_error (error);

		if (g_str_equal (remote_error, "com.endlessm.Updater.Error.WrongState")) {
			error->code = GS_PLUGIN_ERROR_FAILED;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.LiveBoot") ||
			   g_str_equal (remote_error, "com.endlessm.Updater.Error.NotOstreeSystem") ||
			   g_str_equal (remote_error, "org.freedesktop.DBus.Error.ServiceUnknown")) {
			error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.WrongConfiguration")) {
			error->code = GS_PLUGIN_ERROR_FAILED;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.Fetching")) {
			error->code = GS_PLUGIN_ERROR_DOWNLOAD_FAILED;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.MalformedAutoinstallSpec") ||
			   g_str_equal (remote_error, "com.endlessm.Updater.Error.UnknownEntryInAutoinstallSpec") ||
			   g_str_equal (remote_error, "com.endlessm.Updater.Error.FlatpakRemoteConflict")) {
			error->code = GS_PLUGIN_ERROR_FAILED;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.MeteredConnection")) {
			error->code = GS_PLUGIN_ERROR_NO_NETWORK;
		} else if (g_str_equal (remote_error, "com.endlessm.Updater.Error.Cancelled")) {
			error->code = GS_PLUGIN_ERROR_CANCELLED;
		} else {
			g_warning ("Can’t reliably fixup remote error ‘%s’", remote_error);
			error->code = GS_PLUGIN_ERROR_FAILED;
		}
		error->domain = GS_PLUGIN_ERROR;
		return;
	}

	/* this is allowed for low-level errors */
	if (gs_utils_error_convert_gio (perror))
		return;

	/* this is allowed for low-level errors */
	if (gs_utils_error_convert_gdbus (perror))
		return;
}

/* the percentage of the progress bar to use for applying the OS upgrade;
 * we need to fake the progress in this percentage because applying the OS upgrade
 * can take a long time and we don't want the user to think that the upgrade has
 * stalled */
static const guint upgrade_apply_progress_range = 100 - max_progress_for_update; /* percent */
static const gfloat upgrade_apply_max_time = 600.0; /* sec */
static const gfloat upgrade_apply_step_time = 0.250; /* sec */

static void sync_state_from_updater (GsPluginEosUpdater *self);

struct _GsPluginEosUpdater
{
	GsPlugin parent;

	/* These members are only set once in gs_plugin_eos_updater_setup(), and are
	 * internally thread-safe, so can be accessed without any locking. */
	GsEosUpdater *updater_proxy;  /* (owned) */
	GsApp *os_upgrade;  /* (owned); represents both large upgrades and small updates */
	GCancellable *cancellable;  /* (owned) */
	gulong cancelled_id;

	/* These members must only ever be accessed from the main thread, so
	 * can be accessed without any locking. */
	gfloat upgrade_fake_progress;
	guint upgrade_fake_progress_handler;
};

G_DEFINE_TYPE (GsPluginEosUpdater, gs_plugin_eos_updater, GS_TYPE_PLUGIN)

static void
os_upgrade_cancelled_cb (GCancellable *cancellable,
                         gpointer      user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (user_data);

	g_debug ("%s: Cancelling upgrade", G_STRFUNC);
	gs_eos_updater_call_cancel (self->updater_proxy,
				    /* never interactive */
				    G_DBUS_CALL_FLAGS_NONE,
				    -1  /* timeout */,
				    NULL, NULL, NULL);
}

static gboolean
should_add_os_update_or_upgrade (GsAppState state)
{
	switch (state) {
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
	case GS_APP_STATE_UPDATABLE_LIVE:
		return TRUE;
	case GS_APP_STATE_UNKNOWN:
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_UNAVAILABLE:
	case GS_APP_STATE_REMOVING:
	default:
		return FALSE;
	}
}

static gboolean
should_add_os_upgrade (GsPluginEosUpdater *self)
{
	return (should_add_os_update_or_upgrade (gs_app_get_state (self->os_upgrade)) &&
	        gs_app_has_quirk (self->os_upgrade, GS_APP_QUIRK_IS_PROXY));
}

static gboolean
should_add_os_update (GsPluginEosUpdater *self)
{
	return (should_add_os_update_or_upgrade (gs_app_get_state (self->os_upgrade)) &&
	        !gs_app_has_quirk (self->os_upgrade, GS_APP_QUIRK_IS_PROXY));
}

/* Wrapper around gs_app_set_state() which ensures we also notify of update
 * changes if we change between non-upgradable and upgradable states, so that
 * the app is notified to appear in the UI. */
static void
app_set_state (GsPlugin   *plugin,
               GsApp      *app,
               GsAppState  new_state)
{
	GsAppState old_state = gs_app_get_state (app);

	if (new_state == old_state)
		return;

	gs_app_set_state (app, new_state);

	if (should_add_os_update_or_upgrade (old_state) !=
	    should_add_os_update_or_upgrade (new_state)) {
		g_debug ("%s: Calling gs_plugin_updates_changed()", G_STRFUNC);
		gs_plugin_updates_changed (plugin);
	}
}

static gboolean
eos_updater_error_is_cancelled (const gchar *error_name)
{
	return (g_strcmp0 (error_name, "com.endlessm.Updater.Error.Cancelled") == 0);
}

static void
app_set_update_is_user_visible (GsApp    *app,
                                gboolean  update_is_user_visible)
{
	g_debug ("%s: Setting OS update as %s", G_STRFUNC,
		 update_is_user_visible ? "containing significant user visible changes" : "only containing non-user visible changes");

	/* If the update contains significant user visible changes, we want to
	 * show it using the OS upgrade banner (#GsUpgradeBanner), which means
	 * it needs a certain set of metadata set on it.
	 *
	 * If it doesn’t contain significant user visible changes, we want to
	 * show it as a normal update row (a row in a #GsUpdatesSection), which
	 * means it needs different metadata.
	 *
	 * Other parts of the code in this plugin use the presence of
	 * #GS_APP_QUIRK_IS_PROXY to distinguish these two states. */
	if (update_is_user_visible) {
		gs_app_add_quirk (app, GS_APP_QUIRK_IS_PROXY);
		gs_app_set_special_kind (app, GS_APP_SPECIAL_KIND_OS_UPDATE);
	} else {
		gs_app_remove_quirk (app, GS_APP_QUIRK_IS_PROXY);
		gs_app_set_special_kind (app, GS_APP_SPECIAL_KIND_NONE);
	}
}

/* This will be invoked in the main thread. */
static void
updater_state_changed (GsPluginEosUpdater *self)
{
	g_debug ("%s", G_STRFUNC);

	sync_state_from_updater (self);
}

/* This will be invoked in the main thread. */
static void
updater_downloaded_bytes_changed (GsPluginEosUpdater *self)
{
	sync_state_from_updater (self);
}

/* This will be invoked in the main thread. */
static void
updater_version_changed (GsPluginEosUpdater *self)
{
	const gchar *version = gs_eos_updater_get_version (self->updater_proxy);

	/* If eos-updater goes away, we want to retain the previously set value
	 * of the version, for use in error messages. */
	if (version != NULL)
		gs_app_set_version (self->os_upgrade, version);
}

/* This will be invoked in the main thread. */
static void
updater_update_is_user_visible_changed (GsPluginEosUpdater *self)
{
	gboolean update_is_user_visible = gs_eos_updater_get_update_is_user_visible (self->updater_proxy);

	app_set_update_is_user_visible (self->os_upgrade, update_is_user_visible);
}

/* This will be invoked in the main thread. */
static void
updater_release_notes_uri_changed (GsPluginEosUpdater *self)
{
	const gchar *release_notes_uri = gs_eos_updater_get_release_notes_uri (self->updater_proxy);

	/* @release_notes_uri may be the empty string, in which case we want to remove the URL */
	if (release_notes_uri != NULL && *release_notes_uri == '\0')
		release_notes_uri = NULL;

	gs_app_set_url (self->os_upgrade, AS_URL_KIND_HOMEPAGE, release_notes_uri);
}

/* This will be invoked in the main thread. */
static gboolean
fake_os_upgrade_progress_cb (gpointer user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (user_data);
	gfloat normal_step;
	guint new_progress;
	const gfloat fake_progress_max = 99.0;

	if (gs_eos_updater_get_state (self->updater_proxy) != EOS_UPDATER_STATE_APPLYING_UPDATE ||
	    self->upgrade_fake_progress > fake_progress_max) {
		self->upgrade_fake_progress = 0;
		self->upgrade_fake_progress_handler = 0;
		return G_SOURCE_REMOVE;
	}

	normal_step = (gfloat) upgrade_apply_progress_range /
		      (upgrade_apply_max_time / upgrade_apply_step_time);

	self->upgrade_fake_progress += normal_step;

	new_progress = max_progress_for_update +
		       (guint) round (self->upgrade_fake_progress);
	gs_app_set_progress (self->os_upgrade,
			     MIN (new_progress, (guint) fake_progress_max));

	g_debug ("OS upgrade fake progress: %f", self->upgrade_fake_progress);

	return G_SOURCE_CONTINUE;
}

/* This method deals with the synchronization between the EOS updater's states
 * (D-Bus service) and the OS upgrade's states (GsApp), in order to show the user
 * what is happening and what they can do. */
static void
sync_state_from_updater (GsPluginEosUpdater *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);
	GsApp *app = self->os_upgrade;
	EosUpdaterState state;
	GsAppState previous_app_state = gs_app_get_state (app);
	GsAppState current_app_state;

	/* in case the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_debug ("%s: Updater disabled", G_STRFUNC);
		return;
	}

	state = gs_eos_updater_get_state (self->updater_proxy);
	g_debug ("EOS Updater state changed: %s", eos_updater_state_to_str (state));

	switch (state) {
	case EOS_UPDATER_STATE_NONE:
	case EOS_UPDATER_STATE_READY: {
		app_set_state (plugin, app, GS_APP_STATE_UNKNOWN);
		break;
	} case EOS_UPDATER_STATE_POLLING: {
		/* Nothing to do here. */
		break;
	} case EOS_UPDATER_STATE_UPDATE_AVAILABLE: {
		gint64 total_size;

		app_set_update_is_user_visible (app, gs_eos_updater_get_update_is_user_visible (self->updater_proxy));
		app_set_state (plugin, app, GS_APP_STATE_AVAILABLE);

		/* The property returns -1 to indicate unknown size */
		total_size = gs_eos_updater_get_download_size (self->updater_proxy);
		if (total_size >= 0)
			gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, total_size);
		else
			gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWN, 0);

		break;
	}
	case EOS_UPDATER_STATE_FETCHING: {
		gint64 total_size = 0;
		gint64 downloaded = 0;
		guint progress = 0;

		/* FIXME: Set to QUEUED_FOR_INSTALL if we’re waiting for metered
		 * data permission. */
		app_set_state (plugin, app, GS_APP_STATE_DOWNLOADING);

		downloaded = gs_eos_updater_get_downloaded_bytes (self->updater_proxy);
		total_size = gs_eos_updater_get_download_size (self->updater_proxy);

		if (total_size == 0) {
			g_debug ("OS upgrade %s total size is 0!",
				 gs_app_get_unique_id (app));
			progress = GS_APP_PROGRESS_UNKNOWN;
		} else if (downloaded < 0 || total_size < 0) {
			/* Both properties return -1 to indicate unknown */
			progress = GS_APP_PROGRESS_UNKNOWN;
		} else {
			/* set progress only up to a max percentage, leaving the
			 * remaining for applying the update */
			progress = (gfloat) downloaded / (gfloat) total_size *
				   (gfloat) max_progress_for_update;
		}
		gs_app_set_progress (app, progress);

		break;
	}
	case EOS_UPDATER_STATE_UPDATE_READY: {
		app_set_state (plugin, app, GS_APP_STATE_UPDATABLE);

		/* Nothing further to download. */
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);

		break;
	}
	case EOS_UPDATER_STATE_APPLYING_UPDATE: {
		/* set as 'installing' because if it is applying the update, we
		 * want to show the progress bar */
		app_set_state (plugin, app, GS_APP_STATE_INSTALLING);
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);

		/* set up the fake progress to inform the user that something
		 * is still being done (we don't get progress reports from
		 * deploying updates) */
		if (self->upgrade_fake_progress_handler != 0)
			g_source_remove (self->upgrade_fake_progress_handler);
		self->upgrade_fake_progress = 0;
		self->upgrade_fake_progress_handler =
			g_timeout_add ((guint) (1000.0 * upgrade_apply_step_time),
				       (GSourceFunc) fake_os_upgrade_progress_cb,
				       self);

		break;
	}
	case EOS_UPDATER_STATE_UPDATE_APPLIED: {
		app_set_state (plugin, app, GS_APP_STATE_UPDATABLE);
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);

		break;
	}
	case EOS_UPDATER_STATE_ERROR: {
		const gchar *error_name;
		const gchar *error_message;

		error_name = gs_eos_updater_get_error_name (self->updater_proxy);
		error_message = gs_eos_updater_get_error_message (self->updater_proxy);

		/* unless the error is because the user cancelled the upgrade,
		 * we should make sure it gets in the journal */
		if (!eos_updater_error_is_cancelled (error_name))
			g_warning ("Got OS upgrade error state with name '%s': %s",
				   error_name, error_message);

		/* We can’t recover the app state since eos-updater needs to
		 * go through the ready → poll → fetch → apply loop again in
		 * order to recover its state. So go back to ‘unknown’. */
		app_set_state (plugin, app, GS_APP_STATE_UNKNOWN);

		/* Cancelling anything in the updater will result in a
		 * transition to the Error state. Use that as a cue to reset
		 * our #GCancellable ready for next time. */
		g_cancellable_reset (self->cancellable);

		break;
	}
	default:
		g_warning ("Encountered unknown eos-updater state: %u", state);
		break;
	}

	current_app_state = gs_app_get_state (app);

	g_debug ("%s: Old app state: %s; new app state: %s",
		 G_STRFUNC, gs_app_state_to_string (previous_app_state),
		 gs_app_state_to_string (current_app_state));

	/* if the state changed from or to 'unknown', we need to notify that a
	 * new update should be shown */
	if (should_add_os_update_or_upgrade (previous_app_state) !=
	    should_add_os_update_or_upgrade (current_app_state)) {
		g_debug ("%s: Calling gs_plugin_updates_changed()", G_STRFUNC);
		gs_plugin_updates_changed (plugin);
	}
}

static void proxy_new_cb (GObject      *source_object,
                          GAsyncResult *result,
                          gpointer      user_data);

/* This is called in the main thread, so will end up creating an @updater_proxy
 * which is tied to the main thread’s #GMainContext. */
static void
gs_plugin_eos_updater_setup_async (GsPlugin            *plugin,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_eos_updater_setup_async);

	g_debug ("%s", G_STRFUNC);

	self->cancellable = g_cancellable_new ();
	self->cancelled_id =
		g_cancellable_connect (self->cancellable,
				       G_CALLBACK (os_upgrade_cancelled_cb),
				       self, NULL);

	/* Check that the proxy exists (and is owned; it should auto-start) so
	 * we can disable the plugin for systems which don’t have eos-updater.
	 * Throughout the rest of the plugin, errors from the daemon
	 * (particularly where it has disappeared off the bus) are ignored, and
	 * the poll/fetch/apply sequence is run through again to recover from
	 * the error. This is the only point in the plugin where we consider an
	 * error from eos-updater to be fatal to the plugin. */
	gs_eos_updater_proxy_new (gs_plugin_get_system_bus_connection (plugin),
				  G_DBUS_PROXY_FLAGS_NONE,
				  "com.endlessm.Updater",
				  "/com/endlessm/Updater",
				  cancellable,
				  proxy_new_cb,
				  g_steal_pointer (&task));
}

static void
proxy_new_cb (GObject      *source_object,
              GAsyncResult *result,
              gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginEosUpdater *self = g_task_get_source_object (task);
	g_autofree gchar *name_owner = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GIcon) ic = NULL;
	g_autofree gchar *background_filename = NULL;
	g_autofree gchar *css = NULL;
	g_autofree gchar *summary = NULL;
	g_autofree gchar *version = NULL;
	gboolean update_is_user_visible = FALSE;
	g_autoptr(GsOsRelease) os_release = NULL;
	g_autoptr(GError) local_error = NULL;
	const gchar *os_name, *os_logo;

	self->updater_proxy = gs_eos_updater_proxy_new_finish (result, &local_error);
	if (self->updater_proxy == NULL) {
		gs_eos_updater_error_convert (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	name_owner = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (self->updater_proxy));

	if (name_owner == NULL) {
		g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED,
					 "Couldn’t create EOS Updater proxy: couldn’t get name owner");
		return;
	}

	g_signal_connect_object (self->updater_proxy, "notify::state",
				 G_CALLBACK (updater_state_changed),
				 self, G_CONNECT_SWAPPED);
	g_signal_connect_object (self->updater_proxy,
				 "notify::downloaded-bytes",
				 G_CALLBACK (updater_downloaded_bytes_changed),
				 self, G_CONNECT_SWAPPED);
	g_signal_connect_object (self->updater_proxy, "notify::version",
				 G_CALLBACK (updater_version_changed),
				 self, G_CONNECT_SWAPPED);
	g_signal_connect_object (self->updater_proxy, "notify::update-is-user-visible",
				 G_CALLBACK (updater_update_is_user_visible_changed),
				 self, G_CONNECT_SWAPPED);
	g_signal_connect_object (self->updater_proxy, "notify::release-notes-uri",
				 G_CALLBACK (updater_release_notes_uri_changed),
				 self, G_CONNECT_SWAPPED);

	/* prepare EOS upgrade app + sync initial state */

	/* Check for a background image in the standard location. */
	background_filename = gs_utils_get_upgrade_background (NULL);

	if (background_filename != NULL)
		css = g_strconcat ("background: url('file://", background_filename, "');"
				   "background-size: 100% 100%;", NULL);

	os_release = gs_os_release_new (&local_error);
	if (local_error) {
		g_warning ("Failed to get OS release information: %s", local_error->message);
		/* Just a fallback, do not localize */
		os_name = "Endless OS";
		os_logo = NULL;
		g_clear_error (&local_error);
	} else {
		os_name = gs_os_release_get_name (os_release);
		os_logo = gs_os_release_get_logo (os_release);
	}

	g_object_get (G_OBJECT (self->updater_proxy),
		"version", &version,
		"update-is-user-visible", &update_is_user_visible,
		"update-message", &summary,
		NULL);

	if (summary == NULL || *summary == '\0') {
		g_clear_pointer (&summary, g_free);
		g_object_get (G_OBJECT (self->updater_proxy),
			"update-label", &summary,
			NULL);
	}

	if (summary == NULL || *summary == '\0') {
		g_clear_pointer (&summary, g_free);
		/* Translators: The '%s' is replaced with the OS name, like "Endless OS" */
		summary = g_strdup_printf (_("%s update with new features and fixes."), os_name);
	}

	/* use stock icon */
	ic = g_themed_icon_new ((os_logo != NULL) ? os_logo : "system-component-os-updates");

	/* create the OS upgrade */
	app = gs_app_new ("com.endlessm.EOS.upgrade");
	gs_app_add_icon (app, ic);
	gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
	gs_app_set_kind (app, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, os_name);
	gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, summary);
	gs_app_set_version (app, version == NULL ? "" : version);
	app_set_update_is_user_visible (app, update_is_user_visible);
	gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
	gs_app_add_quirk (app, GS_APP_QUIRK_PROVENANCE);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_REVIEWABLE);
	gs_app_set_management_plugin (app, GS_PLUGIN (self));
	gs_app_set_metadata (app, "LingmoAppstore::UpgradeBanner-css", css);

	self->os_upgrade = g_steal_pointer (&app);

	/* sync initial state */
	sync_state_from_updater (self);

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_eos_updater_setup_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_eos_updater_init (GsPluginEosUpdater *self)
{
}

static void
gs_plugin_eos_updater_dispose (GObject *object)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (object);

	if (self->upgrade_fake_progress_handler != 0) {
		g_source_remove (self->upgrade_fake_progress_handler);
		self->upgrade_fake_progress_handler = 0;
	}

	if (self->updater_proxy != NULL) {
		g_signal_handlers_disconnect_by_func (self->updater_proxy,
						      G_CALLBACK (updater_state_changed),
						      self);
		g_signal_handlers_disconnect_by_func (self->updater_proxy,
						      G_CALLBACK (updater_downloaded_bytes_changed),
						      self);
		g_signal_handlers_disconnect_by_func (self->updater_proxy,
						      G_CALLBACK (updater_version_changed),
						      self);
		g_signal_handlers_disconnect_by_func (self->updater_proxy,
						      G_CALLBACK (updater_update_is_user_visible_changed),
						      self);
		g_signal_handlers_disconnect_by_func (self->updater_proxy,
						      G_CALLBACK (updater_release_notes_uri_changed),
						      self);
	}

	g_cancellable_cancel (self->cancellable);
	if (self->cancellable != NULL && self->cancelled_id != 0)
		g_cancellable_disconnect (self->cancellable, self->cancelled_id);
	g_clear_object (&self->cancellable);

	g_clear_object (&self->updater_proxy);

	g_clear_object (&self->os_upgrade);

	G_OBJECT_CLASS (gs_plugin_eos_updater_parent_class)->dispose (object);
}

static void poll_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data);

/* Called in the main thread. */
static void
gs_plugin_eos_updater_refresh_metadata_async (GsPlugin                     *plugin,
                                              guint64                       cache_age_secs,
                                              GsPluginRefreshMetadataFlags  flags,
                                              GCancellable                 *cancellable,
                                              GAsyncReadyCallback           callback,
                                              gpointer                      user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);
	EosUpdaterState updater_state;
	g_autoptr(GTask) task = NULL;
	gboolean interactive = flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_eos_updater_refresh_metadata_async);

	/* We let the eos-updater daemon do its own caching, so ignore the
	 * @cache_age_secs, unless it’s %G_MAXUINT64, which signifies startup of g-s.
	 * In that case, it’s probably just going to load the system too much to
	 * do an update check now. We can wait. */
	g_debug ("%s: cache_age_secs: %" G_GUINT64_FORMAT, G_STRFUNC, cache_age_secs);

	if (cache_age_secs == G_MAXUINT64) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* check if the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_debug ("%s: Updater disabled", G_STRFUNC);
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* poll in the error/none/ready states to check if there's an
	 * update available */
	updater_state = gs_eos_updater_get_state (self->updater_proxy);
	switch (updater_state) {
	case EOS_UPDATER_STATE_ERROR:
	case EOS_UPDATER_STATE_NONE:
	case EOS_UPDATER_STATE_READY:
		gs_eos_updater_call_poll (self->updater_proxy,
					  interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
					  -1  /* timeout */,
					  cancellable,
					  poll_cb,
					  g_steal_pointer (&task));
		return;
	default:
		g_debug ("%s: Updater in state %s; not polling",
			 G_STRFUNC, eos_updater_state_to_str (updater_state));
		g_task_return_boolean (task, TRUE);
		return;
	}
}

static void
poll_cb (GObject      *source_object,
         GAsyncResult *result,
         gpointer      user_data)
{
	GsEosUpdater *updater_proxy = GS_EOS_UPDATER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_eos_updater_call_poll_finish (updater_proxy, result, &local_error)) {
		gs_eos_updater_error_convert (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
	} else {
		g_task_return_boolean (task, TRUE);
	}
}

static gboolean
gs_plugin_eos_updater_refresh_metadata_finish (GsPlugin      *plugin,
                                               GAsyncResult  *result,
                                               GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

/* Called in the main thread. */
static void
gs_plugin_eos_updater_list_distro_upgrades_async (GsPlugin                        *plugin,
                                                  GsPluginListDistroUpgradesFlags  flags,
                                                  GCancellable                    *cancellable,
                                                  GAsyncReadyCallback              callback,
                                                  gpointer                         user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GsAppList) list = gs_app_list_new ();

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_eos_updater_list_distro_upgrades_async);

	g_debug ("%s", G_STRFUNC);

	/* if we are testing the plugin, then always add the OS upgrade */
	if (g_getenv ("GS_PLUGIN_EOS_TEST") != NULL) {
		gs_app_set_state (self->os_upgrade, GS_APP_STATE_AVAILABLE);
		gs_app_list_add (list, self->os_upgrade);
		g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
		return;
	}

	/* check if the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_debug ("%s: Updater disabled", G_STRFUNC);
		g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
		return;
	}

	if (should_add_os_upgrade (self)) {
		g_debug ("Adding EOS upgrade as user visible OS upgrade: %s",
			 gs_app_get_unique_id (self->os_upgrade));
		gs_app_list_add (list, self->os_upgrade);
	} else {
		g_debug ("Not adding EOS upgrade as user visible OS upgrade");
	}

	g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
}

static GsAppList *
gs_plugin_eos_updater_list_distro_upgrades_finish (GsPlugin      *plugin,
                                                   GAsyncResult  *result,
                                                   GError       **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

/* This is called in a #GTask worker thread.
 *
 * It’s used to check for non-significant or non-user-visible updates, in
 * contrast to the significant user visible updates checked for by
 * gs_plugin_eos_updater_list_distro_upgrades_async(). The polling process is
 * the same in either case, though. */
gboolean
gs_plugin_add_updates (GsPlugin      *plugin,
                       GsAppList     *list,
                       GCancellable  *cancellable,
                       GError       **error)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);

	g_debug ("%s", G_STRFUNC);

	/* check if the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_debug ("%s: Updater disabled", G_STRFUNC);
		return TRUE;
	}

	if (should_add_os_update (self)) {
		g_debug ("Adding EOS upgrade as non-user visible OS update: %s",
			 gs_app_get_unique_id (self->os_upgrade));
		gs_app_list_add (list, self->os_upgrade);
	} else {
		g_debug ("Not adding EOS upgrade as non-user visible OS update");
	}

	return TRUE;
}

typedef struct {
	EosUpdaterState old_state;
	gulong notify_id;
	gulong cancelled_id;
	guint idle_id;
} WaitForStateChangeData;

static void
wait_for_state_change_data_free (WaitForStateChangeData *data)
{
	/* These two should have been cleared already */
	g_assert (data->notify_id == 0);
	g_assert (data->cancelled_id == 0);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (WaitForStateChangeData, wait_for_state_change_data_free);

static void wait_for_state_change_cb (GTask *task);
static void wait_for_state_change_notify_cb (GObject    *object,
                                             GParamSpec *pspec,
                                             gpointer    user_data);
static void wait_for_state_change_cancelled_cb (GCancellable *cancellable,
                                                gpointer      user_data);

static void
object_unref_closure (gpointer  data,
                      GClosure *closure)
{
	GObject *obj = G_OBJECT (data);
	g_object_unref (obj);
}

static void
wait_for_state_change_async (GsEosUpdater        *updater_proxy,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(WaitForStateChangeData) data_owned = NULL;
	WaitForStateChangeData *data;

	task = g_task_new (updater_proxy, cancellable, callback, user_data);
	g_task_set_source_tag (task, wait_for_state_change_async);

	/* Store the initial state to compare against later. */
	data = data_owned = g_new0 (WaitForStateChangeData, 1);
	data->old_state = gs_eos_updater_get_state (updater_proxy);
	g_debug ("%s: Old state ‘%s’", G_STRFUNC, eos_updater_state_to_str (data->old_state));

	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) wait_for_state_change_data_free);

	/* Listen for state changes. Connect late in the emission process so
	 * that the callback is invoked after the main updater_state_changed(),
	 * because that updates a load of internal state which the function
	 * calling this one might need. */
	data->notify_id = g_signal_connect_data (updater_proxy, "notify::state",
						 G_CALLBACK (wait_for_state_change_notify_cb),
						 g_object_ref (task), object_unref_closure,
						 G_CONNECT_AFTER);
	data->cancelled_id = g_cancellable_connect (cancellable,
						    G_CALLBACK (wait_for_state_change_cancelled_cb),
						    g_object_ref (task), g_object_unref);
}

static void
wait_for_state_change_cb (GTask *task_unowned)
{
	g_autoptr(GTask) task = g_object_ref (task_unowned);
	WaitForStateChangeData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsEosUpdater *updater_proxy = g_task_get_source_object (task);
	EosUpdaterState old_state, new_state;

	old_state = GPOINTER_TO_INT (g_task_get_task_data (task));
	new_state = gs_eos_updater_get_state (updater_proxy);

	if (new_state == old_state &&
	    !g_cancellable_is_cancelled (cancellable))
		return;

	/* State has changed, or the wait has been cancelled. Disconnect, and
	 * return. */
	g_clear_signal_handler (&data->notify_id, updater_proxy);
	g_cancellable_disconnect (cancellable, data->cancelled_id);
	data->cancelled_id = 0;

	if (data->idle_id != 0) {
		g_source_remove (data->idle_id);
		data->idle_id = 0;
	}

	if (g_task_return_error_if_cancelled (task)) {
		g_debug ("%s: Cancelled", G_STRFUNC);
	} else {
		g_debug ("%s: New state ‘%s’", G_STRFUNC, eos_updater_state_to_str (new_state));
		g_task_return_boolean (task, TRUE);
	}
}

static void
wait_for_state_change_notify_cb (GObject    *object,
                                 GParamSpec *pspec,
                                 gpointer    user_data)
{
	GTask *task = G_TASK (user_data);
	wait_for_state_change_cb (task);
}

static gboolean
wait_for_state_change_idle_cb (gpointer user_data)
{
	g_autoptr(GTask) task = G_TASK (user_data);
	WaitForStateChangeData *data = g_task_get_task_data (task);
	/* it can be zeroed when the "state change" had been called meanwhile;
	   in that case the task is finished already */
	if (data->idle_id != 0) {
		data->idle_id = 0;
		wait_for_state_change_cb (task);
	}
	return G_SOURCE_REMOVE;
}

static void
wait_for_state_change_cancelled_cb (GCancellable *cancellable,
                                    gpointer      user_data)
{
	GTask *task = G_TASK (user_data);
	WaitForStateChangeData *data = g_task_get_task_data (task);
	if (data->idle_id != 0)
		return;
	/* cannot call wait_for_state_change_cb() from the GCancellable::cancelled signal,
	   because it calls g_cancellable_disconnect(), which leads to deadlock, thus
	   postpone this to an idle callback */
	data->idle_id = g_idle_add_full (G_PRIORITY_HIGH_IDLE,
					 wait_for_state_change_idle_cb,
					 g_object_ref (task),
					 g_object_unref);
}

static gboolean
wait_for_state_change_finish (GsEosUpdater  *updater_proxy,
                              GAsyncResult  *result,
                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

/* Could be executed in any thread. No need to hold a lock since we don’t
 * access anything which is not thread-safe. */
static void
cancelled_cb (GCancellable *ui_cancellable,
              gpointer      user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (user_data);

	/* Chain cancellation. */
	g_debug ("Propagating OS download cancellation from %p to %p",
		 ui_cancellable, self->cancellable);
	g_cancellable_cancel (self->cancellable);
}

/* State tracking for a single call to a D-Bus method on the updater proxy.
 *
 * This is designed so that multiple different async calls can all end up back
 * in download_iterate_state_machine_cb(), which then advances the updater’s
 * state machine.
 *
 * Given that different async calls have different `*_finish()` functions, a
 * pointer to the finish function is needed: `finish_func()`. This is called
 * to get the results of each async call. */
typedef struct {
	/* Input arguments. */
	GsApp *app;  /* (not nullable) (owned) */
	GCancellable *cancellable;  /* (nullable) (not owned) */
	gulong cancelled_id;
	gboolean interactive;

	/* State. */
	gboolean done;
	gboolean allow_restart;

	/* Completion callback. */
	gboolean (*finish_func) (GsEosUpdater  *updater_proxy,
	                         GAsyncResult  *result,
	                         GError       **error);  /* (nullable) */
} UpgradeDownloadState;

static void
upgrade_download_state_free (UpgradeDownloadState *data)
{
	g_clear_object (&data->app);

	if (data->cancellable != NULL && data->cancelled_id != 0) {
		g_debug ("Disconnecting cancellable %p", data->cancellable);
		g_cancellable_disconnect (data->cancellable, data->cancelled_id);
		data->cancellable = NULL;
		data->cancelled_id = 0;
	}

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (UpgradeDownloadState, upgrade_download_state_free)

static void download_iterate_state_machine_cb (GObject      *source_object,
                                               GAsyncResult *result,
                                               gpointer      user_data);

/* Called in a #GTask worker thread or in the main thread. */
static void
gs_plugin_eos_updater_app_upgrade_download_async (GsPluginEosUpdater  *self,
                                                  GsApp               *app,
                                                  gboolean             interactive,
                                                  GCancellable        *cancellable,
                                                  GAsyncReadyCallback  callback,
                                                  gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(UpgradeDownloadState) data_owned = NULL;
	UpgradeDownloadState *data;
	EosUpdaterState state;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_eos_updater_app_upgrade_download_async);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, GS_PLUGIN (self))) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* if the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED,
					 "The OS upgrade has been disabled in the EOS plugin");
		return;
	}

	g_assert (app == self->os_upgrade);

	/* Set up some state. */
	data = data_owned = g_new0 (UpgradeDownloadState, 1);
	data->app = g_object_ref (app);
	data->interactive = interactive;
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) upgrade_download_state_free);

	/* Set up cancellation. */
	g_debug ("Chaining cancellation from %p to %p", cancellable, self->cancellable);
	if (cancellable != NULL) {
		data->cancellable = cancellable;  /* ref is held by @task */
		data->cancelled_id = g_cancellable_connect (cancellable,
							    G_CALLBACK (cancelled_cb),
							    self, NULL);
	}

	/* Step through the state machine until we are finished downloading and
	 * applying the update, or until an error occurs.
	 *
	 * Each step is a call to download_iterate_state_machine_cb(). The first
	 * call is below, and subsequent calls come from async function
	 * completions.
	 *
	 * `data->done` is %TRUE once we reach the `UPDATE_APPLIED` state.
	 *
	 * `data->allow_restart` indicates whether the state machine can be
	 * restarted to clear an error condition, or whether the error should be
	 * propagated (because the machine has already been restarted). This
	 * prevents infinite loops. */
	state = gs_eos_updater_get_state (self->updater_proxy);

	data->done = FALSE;
	data->allow_restart = (state == EOS_UPDATER_STATE_NONE ||
			       state == EOS_UPDATER_STATE_READY ||
			       state == EOS_UPDATER_STATE_ERROR);

	download_iterate_state_machine_cb (G_OBJECT (self->updater_proxy), NULL, g_steal_pointer (&task));
}

static gboolean
is_wrong_state_error (const GError *error)
{
	g_autofree gchar *remote_error = NULL;

	if (!g_dbus_error_is_remote_error (error))
		return FALSE;

	remote_error = g_dbus_error_get_remote_error (error);

	return g_str_equal (remote_error, "com.endlessm.Updater.Error.WrongState");
}

static void
download_iterate_state_machine_cb (GObject      *source_object,
                                   GAsyncResult *result,
                                   gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginEosUpdater *self = g_task_get_source_object (task);
	UpgradeDownloadState *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	EosUpdaterState state;

	/* Call the finish function from the asynchronous method call which has
	 * just completed and brought us back into
	 * download_iterate_state_machine_cb().
	 *
	 * This may be %NULL if the state machine is just being started. */
	if (data->finish_func != NULL) {
		g_autoptr(GError) local_error = NULL;

		if (!data->finish_func (self->updater_proxy, result, &local_error)) {
			/* Ignore WrongState errors, since we explicitly synchronise
			 * to the daemon’s state again below, so should be able
			 * to recover from them. The user can’t do anything
			 * about them anyway. */
			if (is_wrong_state_error (local_error)) {
				g_debug ("Got WrongState error from eos-updater daemon; ignoring.");
				g_clear_error (&local_error);
			} else {
				gs_eos_updater_error_convert (&local_error);
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}

		data->finish_func = NULL;
	} else {
		g_assert (result == NULL);
	}

	/* Iterate the state machine one step. */
	while (!data->done && !g_cancellable_is_cancelled (cancellable)) {
		state = gs_eos_updater_get_state (self->updater_proxy);
		g_debug ("%s: State ‘%s’", G_STRFUNC, eos_updater_state_to_str (state));

		switch (state) {
		case EOS_UPDATER_STATE_NONE:
		case EOS_UPDATER_STATE_READY: {
			/* Poll for an update. This typically only happens if
			 * we’ve drifted out of sync with the updater process
			 * due to it dying. In that case, only restart once
			 * before giving up, so we don’t end up in an endless
			 * loop (say, if eos-updater always died 50% of the way
			 * through a download). */
			if (data->allow_restart) {
				data->allow_restart = FALSE;
				g_debug ("Restarting OS upgrade from none/ready state");

				data->finish_func = gs_eos_updater_call_poll_finish;
				gs_eos_updater_call_poll (self->updater_proxy,
							  data->interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							  -1  /* timeout */,
							  cancellable,
							  download_iterate_state_machine_cb,
							  g_steal_pointer (&task));
				return;
			} else {
				/* Display an error to the user. */
				g_autoptr(GError) error_local = NULL;
				g_autoptr(GsPluginEvent) event = NULL;

				g_set_error_literal (&error_local, GS_PLUGIN_ERROR,
						     GS_PLUGIN_ERROR_FAILED,
						     _("EOS update service could not fetch and apply the update."));
				gs_eos_updater_error_convert (&error_local);

				event = gs_plugin_event_new ("app", data->app,
							     "action", GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
							     "error", error_local,
							     NULL);
				gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
				if (data->interactive)
					gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
				gs_plugin_report_event (GS_PLUGIN (self), event);

				/* Error out. */
				data->done = TRUE;
			}

			break;
		} case EOS_UPDATER_STATE_POLLING: {
			/* Nothing to do here. */
			break;
		} case EOS_UPDATER_STATE_UPDATE_AVAILABLE: {
			g_auto(GVariantDict) options_dict = G_VARIANT_DICT_INIT (NULL);

			/* when the OS upgrade was started by the user and the
			 * updater reports an available update, (meaning we were
			 * polling before), we should readily call fetch */
			g_variant_dict_insert (&options_dict, "force", "b", TRUE);

			data->finish_func = gs_eos_updater_call_fetch_full_finish;
			gs_eos_updater_call_fetch_full (self->updater_proxy,
							g_variant_dict_end (&options_dict),
							data->interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							-1  /* timeout */,
							cancellable,
							download_iterate_state_machine_cb,
							g_steal_pointer (&task));
			return;
		}
		case EOS_UPDATER_STATE_FETCHING: {
			/* Nothing to do here. */
			break;
		}
		case EOS_UPDATER_STATE_UPDATE_READY: {
			/* if there's an update ready to deployed, and it was started by
			 * the user, we should proceed to applying the upgrade */
			gs_app_set_progress (data->app, max_progress_for_update);

			/* Nothing further to download. */
			gs_app_set_size_download (data->app, GS_SIZE_TYPE_VALID, 0);

			data->finish_func = gs_eos_updater_call_apply_finish;
			gs_eos_updater_call_apply (self->updater_proxy,
						   data->interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						   -1  /* timeout */,
						   cancellable,
						   download_iterate_state_machine_cb,
						   g_steal_pointer (&task));
			return;
		}
		case EOS_UPDATER_STATE_APPLYING_UPDATE: {
			/* Nothing to do here. */
			break;
		}
		case EOS_UPDATER_STATE_UPDATE_APPLIED: {
			/* Done! */
			data->done = TRUE;
			break;
		}
		case EOS_UPDATER_STATE_ERROR: {
			const gchar *error_name;
			const gchar *error_message;
			g_autoptr(GError) error_local = NULL;

			error_name = gs_eos_updater_get_error_name (self->updater_proxy);
			error_message = gs_eos_updater_get_error_message (self->updater_proxy);
			error_local = g_dbus_error_new_for_dbus_error (error_name, error_message);

			/* Display an error to the user, unless they cancelled
			 * the download. */
			if (!eos_updater_error_is_cancelled (error_name)) {
				g_autoptr(GsPluginEvent) event = NULL;

				gs_eos_updater_error_convert (&error_local);

				event = gs_plugin_event_new ("app", data->app,
							     "action", GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
							     "error", error_local,
							     NULL);
				gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
				if (data->interactive)
					gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
				gs_plugin_report_event (GS_PLUGIN (self), event);
			}

			/* Unconditionally call Poll() to get the updater out
			 * of the error state and to allow the update to be
			 * displayed in the UI again and retried. Exit the
			 * state change loop immediately by setting data->done,
			 * though, to prevent possible endless loops between the
			 * Poll/Error states. */
			data->allow_restart = FALSE;
			data->done = TRUE;
			g_debug ("Restarting OS upgrade on error");

			data->finish_func = gs_eos_updater_call_poll_finish;
			gs_eos_updater_call_poll (self->updater_proxy,
						  data->interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						  -1  /* timeout */,
						  cancellable,
						  download_iterate_state_machine_cb,
						  g_steal_pointer (&task));
			return;
		}
		default:
			g_warning ("Encountered unknown eos-updater state: %u", state);
			break;
		}

		/* Block on the next state change. */
		if (!data->done) {
			data->finish_func = wait_for_state_change_finish;
			wait_for_state_change_async (self->updater_proxy, cancellable, download_iterate_state_machine_cb, g_steal_pointer (&task));
			return;
		}
	}

	/* Process the final state. */
	if (gs_eos_updater_get_state (self->updater_proxy) == EOS_UPDATER_STATE_ERROR) {
		const gchar *error_name;
		const gchar *error_message;
		g_autoptr(GError) error_local = NULL;

		error_name = gs_eos_updater_get_error_name (self->updater_proxy);
		error_message = gs_eos_updater_get_error_message (self->updater_proxy);
		error_local = g_dbus_error_new_for_dbus_error (error_name, error_message);
		gs_eos_updater_error_convert (&error_local);

		g_task_return_error (task, g_steal_pointer (&error_local));
	} else if (!g_task_return_error_if_cancelled (task)) {
		g_task_return_boolean (task, TRUE);
	}
}

static gboolean
gs_plugin_eos_updater_app_upgrade_download_finish (GsPluginEosUpdater  *self,
                                                   GAsyncResult        *result,
                                                   GError             **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
async_result_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GAsyncResult **result_out = user_data;

	g_assert (result_out != NULL && *result_out == NULL);
	*result_out = g_object_ref (result);
	g_main_context_wakeup (g_main_context_get_thread_default ());
}

/* This is called in a #GTask worker thread.
 *
 * It’s used to download the update if it’s been listed in the UI as a major
 * upgrade. The download process is the same. */
gboolean
gs_plugin_app_upgrade_download (GsPlugin *plugin,
				GsApp *app,
			        GCancellable *cancellable,
				GError **error)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);
	g_autoptr(GMainContext) context = g_main_context_new ();
	g_autoptr(GMainContextPusher) pusher = g_main_context_pusher_new (context);
	g_autoptr(GAsyncResult) result = NULL;

	/* FIXME: This should eventually be moved to the new plugin API. For now,
	 * since this old function runs in a worker thread, run in a loop until
	 * the async download completes. */
	gs_plugin_eos_updater_app_upgrade_download_async (self, app, interactive, cancellable, async_result_cb, &result);
	while (result == NULL)
		g_main_context_iteration (context, TRUE);
	return gs_plugin_eos_updater_app_upgrade_download_finish (self, result, error);
}

static void upgrade_download_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);

/* Called in the main thread.
 *
 * It’s used to download the update if it’s been listed in the UI as a minor
 * update. The download process is the same. */
static void
gs_plugin_eos_updater_update_apps_async (GsPlugin                           *plugin,
                                         GsAppList                          *apps,
                                         GsPluginUpdateAppsFlags             flags,
                                         GsPluginProgressCallback            progress_callback,
                                         gpointer                            progress_user_data,
                                         GsPluginAppNeedsUserActionCallback  app_needs_user_action_callback,
                                         gpointer                            app_needs_user_action_data,
                                         GCancellable                       *cancellable,
                                         GAsyncReadyCallback                 callback,
                                         gpointer                            user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (plugin);
	g_autoptr(GTask) task = NULL;
	GsApp *app;
	guint n_managed_apps = 0;
	gboolean interactive = (flags & GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_eos_updater_update_apps_async);

	g_debug ("%s", G_STRFUNC);

	/* check if the OS upgrade has been disabled */
	if (self->updater_proxy == NULL) {
		g_debug ("%s: Updater disabled", G_STRFUNC);
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* Find the app for the OS upgrade in the list of apps. It might not be present. */
	for (guint i = 0; i < gs_app_list_length (apps); i++) {
		GsApp *app_i = gs_app_list_index (apps, i);

		if (gs_app_has_management_plugin (app_i, plugin)) {
			app = app_i;
			n_managed_apps++;
		}
	}

	if (n_managed_apps == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	g_assert (n_managed_apps == 1);

	if (!(flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD)) {
		/* Download the update.
		 * FIXME: Progress reporting */
		gs_plugin_eos_updater_app_upgrade_download_async (self, app, interactive, cancellable, upgrade_download_cb, g_steal_pointer (&task));
	} else {
		g_task_return_boolean (task, TRUE);
	}
}

static void
upgrade_download_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	GsPluginEosUpdater *self = GS_PLUGIN_EOS_UPDATER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_plugin_eos_updater_app_upgrade_download_finish (self, result, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_eos_updater_update_apps_finish (GsPlugin      *plugin,
                                          GAsyncResult  *result,
                                          GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_eos_updater_class_init (GsPluginEosUpdaterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_eos_updater_dispose;

	plugin_class->setup_async = gs_plugin_eos_updater_setup_async;
	plugin_class->setup_finish = gs_plugin_eos_updater_setup_finish;
	plugin_class->refresh_metadata_async = gs_plugin_eos_updater_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_eos_updater_refresh_metadata_finish;
	plugin_class->list_distro_upgrades_async = gs_plugin_eos_updater_list_distro_upgrades_async;
	plugin_class->list_distro_upgrades_finish = gs_plugin_eos_updater_list_distro_upgrades_finish;
	plugin_class->update_apps_async = gs_plugin_eos_updater_update_apps_async;
	plugin_class->update_apps_finish = gs_plugin_eos_updater_update_apps_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_EOS_UPDATER;
}
