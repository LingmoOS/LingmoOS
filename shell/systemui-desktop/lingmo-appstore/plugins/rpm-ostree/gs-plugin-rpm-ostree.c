/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017-2020 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <lingmo-appstore.h>

#include <fcntl.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>
#include <ostree.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpmostree.h>

#include "gs-plugin-private.h"
#include "gs-plugin-rpm-ostree.h"
#include "gs-rpmostree-generated.h"

/*
 * SECTION:
 * Exposes rpm-ostree system updates and overlays.
 *
 * The plugin has a worker thread which all operations are delegated to, as
 * while the rpm-ostreed API is asynchronous over D-Bus, the plugin also needs
 * to use lower level libostree APIs which are entirely synchronous.
 * Message passing to the worker thread is by gs_worker_thread_queue().
 */

/* This shows up in the `rpm-ostree status` as the software that
 * initiated the update.
 */
#define GS_RPMOSTREE_CLIENT_ID PACKAGE_NAME

/* How long to wait between two consecutive requests, before considering
 * the connection to the rpm-ostree daemon inactive and disconnect from it.
 */
#define INACTIVE_TIMEOUT_SECONDS 60

G_DEFINE_AUTO_CLEANUP_FREE_FUNC(Header, headerFree, NULL)
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(rpmts, rpmtsFree, NULL);
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(rpmdbMatchIterator, rpmdbFreeIterator, NULL);

struct _GsPluginRpmOstree {
	GsPlugin		 parent;

	GsWorkerThread		*worker;  /* (owned) */

	GMutex			 mutex;
	GsRPMOSTreeOS		*os_proxy;
	GsRPMOSTreeSysroot	*sysroot_proxy;
	OstreeRepo		*ot_repo;
	OstreeSysroot		*ot_sysroot;
	gboolean		 update_triggered;
	guint			 inactive_timeout_id;

	GHashTable		*cached_sources; /* (nullable) (owned) (element-type utf8 GsApp); sources by id, each value is weak reffed */
	GMutex			 cached_sources_mutex;
};

G_DEFINE_TYPE (GsPluginRpmOstree, gs_plugin_rpm_ostree, GS_TYPE_PLUGIN)

#define assert_in_worker(self) \
	g_assert (gs_worker_thread_is_in_worker_context (self->worker))

static void
cached_sources_weak_ref_cb (gpointer user_data,
			    GObject *object)
{
	GsPluginRpmOstree *self = user_data;
	GHashTableIter iter;
	gpointer key, value;
	g_autoptr(GMutexLocker) locker = NULL;

	locker = g_mutex_locker_new (&self->cached_sources_mutex);

	g_assert (self->cached_sources != NULL);

	g_hash_table_iter_init (&iter, self->cached_sources);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		GObject *repo_object = value;
		if (repo_object == object) {
			g_hash_table_iter_remove (&iter);
			if (!g_hash_table_size (self->cached_sources))
				g_clear_pointer (&self->cached_sources, g_hash_table_unref);
			break;
		}
	}
}

static void
gs_plugin_rpm_ostree_dispose (GObject *object)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (object);

	g_clear_handle_id (&self->inactive_timeout_id, g_source_remove);
	g_clear_object (&self->os_proxy);
	g_clear_object (&self->sysroot_proxy);
	g_clear_object (&self->ot_sysroot);
	g_clear_object (&self->ot_repo);
	g_clear_object (&self->worker);

	if (self->cached_sources != NULL) {
		GHashTableIter iter;
		gpointer value;

		g_hash_table_iter_init (&iter, self->cached_sources);
		while (g_hash_table_iter_next (&iter, NULL, &value)) {
			GObject *app_repo = value;
			g_object_weak_unref (app_repo, cached_sources_weak_ref_cb, self);
		}

		g_clear_pointer (&self->cached_sources, g_hash_table_unref);
	}

	G_OBJECT_CLASS (gs_plugin_rpm_ostree_parent_class)->dispose (object);
}

static void
gs_plugin_rpm_ostree_finalize (GObject *object)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (object);

	g_mutex_clear (&self->mutex);
	g_mutex_clear (&self->cached_sources_mutex);

	G_OBJECT_CLASS (gs_plugin_rpm_ostree_parent_class)->finalize (object);
}

static void
gs_plugin_rpm_ostree_init (GsPluginRpmOstree *self)
{
	g_mutex_init (&self->mutex);
	g_mutex_init (&self->cached_sources_mutex);

	/* only works on OSTree */
	if (!g_file_test ("/run/ostree-booted", G_FILE_TEST_EXISTS)) {
		gs_plugin_set_enabled (GS_PLUGIN (self), FALSE);
		return;
	}

	/* open transaction */
	rpmReadConfigFiles (NULL, NULL);

	/* rpm-ostree is already a daemon with a DBus API; hence it makes
	 * more sense to use a custom plugin instead of using PackageKit.
	 */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_CONFLICTS, "packagekit");

	/* need pkgname */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "appstream");
}

static void
gs_rpmostree_error_convert (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return;

	/* parse remote RPM_OSTREED_ERROR */
	if (g_dbus_error_is_remote_error (error)) {
		g_autofree gchar *remote_error = g_dbus_error_get_remote_error (error);

		g_dbus_error_strip_remote_error (error);

		if (g_strcmp0 (remote_error, "org.projectatomic.rpmostreed.Error.NotAuthorized") == 0) {
			error->code = GS_PLUGIN_ERROR_NO_SECURITY;
		} else if (g_str_has_prefix (remote_error, "org.projectatomic.rpmostreed.Error")) {
			error->code = GS_PLUGIN_ERROR_FAILED;
		} else if (gs_utils_error_convert_gdbus (perror)) {
			return;
		} else {
			g_warning ("can't reliably fixup remote error %s", remote_error);
			error->code = GS_PLUGIN_ERROR_FAILED;
		}
		error->domain = GS_PLUGIN_ERROR;
		return;
	}

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gio (perror))
		return;

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gdbus (perror))
		return;
}

static void
gs_rpmostree_unregister_client_done_cb (GObject *source_object,
					GAsyncResult *result,
					gpointer user_data)
{
	g_autoptr(GError) error = NULL;

	if (!gs_rpmostree_sysroot_call_unregister_client_finish (GS_RPMOSTREE_SYSROOT (source_object), result, &error))
		g_debug ("Failed to unregister client: %s", error->message);
	else
		g_debug ("Unregistered client from the rpm-ostreed");
}

static gboolean
gs_rpmostree_inactive_timeout_cb (gpointer user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (user_data);
	g_autoptr(GMutexLocker) locker = NULL;

	if (g_source_is_destroyed (g_main_current_source ()))
		return G_SOURCE_REMOVE;

	locker = g_mutex_locker_new (&self->mutex);

	/* In case it gets destroyed before the lock is acquired */
	if (!g_source_is_destroyed (g_main_current_source ()) &&
	    self->inactive_timeout_id == g_source_get_id (g_main_current_source ())) {
		g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;

		if (self->sysroot_proxy) {
			g_autofree gchar *current_path = gs_rpmostree_sysroot_dup_active_transaction_path (self->sysroot_proxy);
			/* Do not unregister the client when there's an ongoing transaction */
			if (current_path != NULL && *current_path != '\0')
				return G_SOURCE_CONTINUE;
			sysroot_proxy = g_steal_pointer (&self->sysroot_proxy);
		}

		g_clear_object (&self->os_proxy);
		g_clear_object (&self->sysroot_proxy);
		g_clear_object (&self->ot_sysroot);
		g_clear_object (&self->ot_repo);
		self->inactive_timeout_id = 0;

		g_clear_pointer (&locker, g_mutex_locker_free);

		if (sysroot_proxy) {
			g_autoptr(GVariantBuilder) options_builder = NULL;
			options_builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
			g_variant_builder_add (options_builder, "{sv}", "id",
					       g_variant_new_string (GS_RPMOSTREE_CLIENT_ID));
			gs_rpmostree_sysroot_call_unregister_client (sysroot_proxy,
								     g_variant_builder_end (options_builder),
								     /* never interactive */
								     G_DBUS_CALL_FLAGS_NONE,
								     -1  /* timeout */,
								     NULL,
								     gs_rpmostree_unregister_client_done_cb,
								     NULL);
		}
	}

	return G_SOURCE_REMOVE;
}

/* Hold the plugin mutex when called */
static gboolean
gs_rpmostree_ref_proxies_locked (GsPluginRpmOstree *self,
				 gboolean interactive,
				 GsRPMOSTreeOS **out_os_proxy,
				 GsRPMOSTreeSysroot **out_sysroot_proxy,
				 GCancellable *cancellable,
				 GError **error)
{
	if (self->inactive_timeout_id) {
		g_source_remove (self->inactive_timeout_id);
		self->inactive_timeout_id = 0;
	}

	/* Create a proxy for sysroot */
	if (self->sysroot_proxy == NULL) {
		g_autoptr(GVariantBuilder) options_builder = NULL;

		self->sysroot_proxy = gs_rpmostree_sysroot_proxy_new_sync (gs_plugin_get_system_bus_connection (GS_PLUGIN (self)),
		                                                           G_DBUS_PROXY_FLAGS_NONE,
		                                                           "org.projectatomic.rpmostree1",
		                                                           "/org/projectatomic/rpmostree1/Sysroot",
		                                                           cancellable,
		                                                           error);
		if (self->sysroot_proxy == NULL) {
			gs_rpmostree_error_convert (error);
			return FALSE;
		}

		options_builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
		g_variant_builder_add (options_builder, "{sv}", "id",
				       g_variant_new_string (GS_RPMOSTREE_CLIENT_ID));
		/* Register as a client so that the rpm-ostree daemon doesn't exit */
		if (!gs_rpmostree_sysroot_call_register_client_sync (self->sysroot_proxy,
								     g_variant_builder_end (options_builder),
								     interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
								     -1  /* timeout */,
								     cancellable,
								     error)) {
			g_clear_object (&self->sysroot_proxy);
			gs_rpmostree_error_convert (error);
			return FALSE;
		}

		g_debug ("Registered client on the rpm-ostreed");
	}

	/* Create a proxy for currently booted OS */
	if (self->os_proxy == NULL) {
		g_autofree gchar *os_object_path = NULL;

		os_object_path = gs_rpmostree_sysroot_dup_booted (self->sysroot_proxy);
		if (os_object_path == NULL &&
		    !gs_rpmostree_sysroot_call_get_os_sync (self->sysroot_proxy,
		                                            "",
		                                            interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
		                                            -1  /* timeout */,
		                                            &os_object_path,
		                                            cancellable,
		                                            error)) {
			gs_rpmostree_error_convert (error);
			g_clear_object (&self->sysroot_proxy);
			return FALSE;
		}

		self->os_proxy = gs_rpmostree_os_proxy_new_sync (gs_plugin_get_system_bus_connection (GS_PLUGIN (self)),
		                                                 G_DBUS_PROXY_FLAGS_NONE,
		                                                 "org.projectatomic.rpmostree1",
		                                                 os_object_path,
		                                                 cancellable,
		                                                 error);
		if (self->os_proxy == NULL) {
			gs_rpmostree_error_convert (error);
			g_clear_object (&self->sysroot_proxy);
			return FALSE;
		}
	}

	/* Load ostree sysroot and repo */
	if (self->ot_sysroot == NULL) {
		g_autofree gchar *sysroot_path = NULL;
		g_autoptr(GFile) sysroot_file = NULL;

		sysroot_path = gs_rpmostree_sysroot_dup_path (self->sysroot_proxy);
		sysroot_file = g_file_new_for_path (sysroot_path);

		self->ot_sysroot = ostree_sysroot_new (sysroot_file);
		if (!ostree_sysroot_load (self->ot_sysroot, cancellable, error)) {
			gs_rpmostree_error_convert (error);
			g_clear_object (&self->sysroot_proxy);
			g_clear_object (&self->os_proxy);
			g_clear_object (&self->ot_sysroot);
			return FALSE;
		}

		if (!ostree_sysroot_get_repo (self->ot_sysroot, &self->ot_repo, cancellable, error)) {
			gs_rpmostree_error_convert (error);
			g_clear_object (&self->sysroot_proxy);
			g_clear_object (&self->os_proxy);
			g_clear_object (&self->ot_sysroot);
			return FALSE;
		}
	}

	self->inactive_timeout_id = g_timeout_add_seconds (INACTIVE_TIMEOUT_SECONDS,
		gs_rpmostree_inactive_timeout_cb, self);

	if (out_os_proxy)
		*out_os_proxy = g_object_ref (self->os_proxy);

	if (out_sysroot_proxy)
		*out_sysroot_proxy = g_object_ref (self->sysroot_proxy);

	return TRUE;
}

static gboolean
gs_rpmostree_ref_proxies (GsPluginRpmOstree *self,
			  gboolean interactive,
			  GsRPMOSTreeOS **out_os_proxy,
			  GsRPMOSTreeSysroot **out_sysroot_proxy,
			  GCancellable *cancellable,
			  GError **error)
{
	g_autoptr(GMutexLocker) locker = NULL;

	locker = g_mutex_locker_new (&self->mutex);

	return gs_rpmostree_ref_proxies_locked (self, interactive, out_os_proxy, out_sysroot_proxy, cancellable, error);
}

static gint
get_priority_for_interactivity (gboolean interactive)
{
	return interactive ? G_PRIORITY_DEFAULT : G_PRIORITY_LOW;
}

static void setup_thread_cb (GTask        *task,
                             gpointer      source_object,
                             gpointer      task_data,
                             GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_setup_async (GsPlugin            *plugin,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;

	g_debug ("rpm-ostree version: %s", RPM_OSTREE_VERSION_S);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_setup_async);

	/* Start up a worker thread to process all the plugin’s function calls. */
	self->worker = gs_worker_thread_new ("gs-plugin-rpm-ostree");

	/* Queue a job to set up the D-Bus proxies. While these could be set
	 * up from the main thread asynchronously, setting them up in the worker
	 * thread means their signal emissions will correctly be in the worker
	 * thread, and locking is simpler. */
	gs_worker_thread_queue (self->worker, G_PRIORITY_DEFAULT,
				setup_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
setup_thread_cb (GTask        *task,
                 gpointer      source_object,
                 gpointer      task_data,
                 GCancellable *cancellable)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (source_object);
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_rpmostree_ref_proxies (self, FALSE  /* not interactive */, NULL, NULL, cancellable, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_rpm_ostree_setup_finish (GsPlugin      *plugin,
                                   GAsyncResult  *result,
                                   GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void shutdown_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data);

static void
gs_plugin_rpm_ostree_shutdown_async (GsPlugin            *plugin,
                                     GCancellable        *cancellable,
                                     GAsyncReadyCallback  callback,
                                     gpointer             user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_shutdown_async);

	/* Stop checking for inactivity. */
	g_clear_handle_id (&self->inactive_timeout_id, g_source_remove);

	/* Stop the worker thread. */
	gs_worker_thread_shutdown_async (self->worker, cancellable, shutdown_cb, g_steal_pointer (&task));
}

static void
shutdown_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginRpmOstree *self = g_task_get_source_object (task);
	g_autoptr(GsWorkerThread) worker = NULL;
	g_autoptr(GError) local_error = NULL;

	worker = g_steal_pointer (&self->worker);

	if (!gs_worker_thread_shutdown_finish (worker, result, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_rpm_ostree_shutdown_finish (GsPlugin      *plugin,
                                      GAsyncResult  *result,
                                      GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
app_set_rpm_ostree_packaging_format (GsApp *app)
{
	gs_app_set_metadata (app, "LingmoAppstore::PackagingFormat", "RPM");
	gs_app_set_metadata (app, "LingmoAppstore::PackagingBaseCssColor", "error_color");
}

void
gs_plugin_adopt_app (GsPlugin *plugin, GsApp *app)
{
	if (gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_PACKAGE &&
	    gs_app_get_scope (app) == AS_COMPONENT_SCOPE_SYSTEM) {
		gs_app_set_management_plugin (app, plugin);
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
		app_set_rpm_ostree_packaging_format (app);
	}

	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_OPERATING_SYSTEM) {
		gs_app_set_management_plugin (app, plugin);
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
	}
}

typedef struct {
	GsPlugin *plugin;
	GError *error;
	GMainContext *context;
	GsApp *app;
	GsAppList *download_progress_list;
	gboolean complete;
	gboolean owner_changed;
} TransactionProgress;

static TransactionProgress *
transaction_progress_new (void)
{
	TransactionProgress *self;

	self = g_slice_new0 (TransactionProgress);
	self->context = g_main_context_ref_thread_default ();

	return self;
}

static void
transaction_progress_free (TransactionProgress *self)
{
	g_clear_object (&self->plugin);
	g_clear_error (&self->error);
	g_main_context_unref (self->context);
	g_clear_object (&self->app);
	g_clear_object (&self->download_progress_list);
	g_slice_free (TransactionProgress, self);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(TransactionProgress, transaction_progress_free);

static void
transaction_progress_end (TransactionProgress *self)
{
	self->complete = TRUE;
	if (self->download_progress_list)
		gs_app_list_override_progress (self->download_progress_list, GS_APP_PROGRESS_UNKNOWN);
	g_main_context_wakeup (self->context);
}

static void
on_transaction_progress (GDBusProxy *proxy,
                         gchar *sender_name,
                         gchar *signal_name,
                         GVariant *parameters,
                         gpointer user_data)
{
	TransactionProgress *tp = user_data;

	if (g_strcmp0 (signal_name, "PercentProgress") == 0) {
		const gchar *message = NULL;
		guint32 percentage;

		g_variant_get_child (parameters, 0, "&s", &message);
		g_variant_get_child (parameters, 1, "u", &percentage);
		g_debug ("PercentProgress: %u, %s\n", percentage, message);

		if (tp->app != NULL)
			gs_app_set_progress (tp->app, (guint) percentage);

		if (tp->app != NULL && tp->plugin != NULL) {
			GsPluginStatus plugin_status;

			switch (gs_app_get_state (tp->app)) {
			case GS_APP_STATE_INSTALLING:
				plugin_status = GS_PLUGIN_STATUS_INSTALLING;
				break;
			case GS_APP_STATE_REMOVING:
				plugin_status = GS_PLUGIN_STATUS_REMOVING;
				break;
			default:
				plugin_status = GS_PLUGIN_STATUS_DOWNLOADING;
				break;
			}
			gs_plugin_status_update (tp->plugin, tp->app, plugin_status);
		}
	} else if (g_strcmp0 (signal_name, "DownloadProgress") == 0) {
		guint32 percentage = 0;
		guint32 fetched, requested;
		g_autofree gchar *params = g_variant_print (parameters, TRUE);

		/* "content" arg */
		g_variant_get_child (parameters, 4, "(uu)", &fetched, &requested);
		if (requested > 0) {
			gdouble percentage_dbl = ((gdouble) fetched) * 100.0 / requested;
			percentage = (guint32) percentage_dbl;
		}
		g_debug ("%s: %s", signal_name, params);

		if (tp->app != NULL)
			gs_app_set_progress (tp->app, (guint) percentage);
		if (tp->download_progress_list)
			gs_app_list_override_progress (tp->download_progress_list, (guint) percentage);
		if (tp->app != NULL && tp->plugin != NULL)
			gs_plugin_status_update (tp->plugin, tp->app, GS_PLUGIN_STATUS_DOWNLOADING);
	} else if (g_strcmp0 (signal_name, "Finished") == 0) {
		if (tp->error == NULL) {
			g_autofree gchar *error_message = NULL;
			gboolean success = FALSE;

			g_variant_get (parameters, "(bs)", &success, &error_message);

			if (!success) {
				tp->error = g_dbus_error_new_for_dbus_error ("org.projectatomic.rpmostreed.Error.Failed",
				                                             error_message);
			}
		}

		transaction_progress_end (tp);
	} else {
		g_autofree gchar *params = g_variant_print (parameters, TRUE);
		g_debug ("Ignoring '%s' signal with params: %s", signal_name, params);
	}
}

static void
on_owner_notify (GObject    *obj,
                 GParamSpec *pspec,
                 gpointer    user_data)
{
	TransactionProgress *tp = user_data;

	tp->owner_changed = TRUE;

	/* Wake up the context so it can notice the server has disappeared. */
	g_main_context_wakeup (tp->context);
}

static void
cancelled_handler (GCancellable *cancellable,
                   gpointer user_data)
{
	GsRPMOSTreeTransaction *transaction = user_data;
	gs_rpmostree_transaction_call_cancel_sync (transaction,
						   /* never interactive */
						   G_DBUS_CALL_FLAGS_NONE,
						   -1  /* timeout */,
						   NULL, NULL);
}

static gboolean
gs_rpmostree_transaction_get_response_sync (GsRPMOSTreeSysroot *sysroot_proxy,
                                            const gchar *transaction_address,
                                            TransactionProgress *tp,
                                            gboolean interactive,
                                            GCancellable *cancellable,
                                            GError **error)
{
	GsRPMOSTreeTransaction *transaction = NULL;
	g_autoptr(GDBusConnection) peer_connection = NULL;
	gint cancel_handler = 0;
	gulong signal_handler = 0;
	gulong notify_handler = 0;
	gboolean success = FALSE;
	gboolean just_started = FALSE;
	gboolean saw_name_owner = FALSE;
	g_autofree gchar *name_owner = NULL;

	peer_connection = g_dbus_connection_new_for_address_sync (transaction_address,
	                                                          G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
	                                                          NULL,
	                                                          cancellable,
	                                                          error);

	if (peer_connection == NULL)
		goto out;

	transaction = gs_rpmostree_transaction_proxy_new_sync (peer_connection,
	                                                       G_DBUS_PROXY_FLAGS_NONE,
	                                                       NULL,
	                                                       "/",
	                                                       cancellable,
	                                                       error);
	if (transaction == NULL)
		goto out;

	if (cancellable) {
		/* setup cancel handler */
		cancel_handler = g_cancellable_connect (cancellable,
							G_CALLBACK (cancelled_handler),
							transaction, NULL);
	}

	signal_handler = g_signal_connect (transaction, "g-signal",
	                                   G_CALLBACK (on_transaction_progress),
	                                   tp);

	notify_handler = g_signal_connect (transaction, "notify::g-name-owner",
					   G_CALLBACK (on_owner_notify),
					   tp);

	/* Tell the server we're ready to receive signals. */
	if (!gs_rpmostree_transaction_call_start_sync (transaction,
	                                               interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
	                                               -1  /* timeout */,
	                                               &just_started,
	                                               cancellable,
	                                               error))
		goto out;

	/* Process all the signals until we receive the Finished signal or the
	 * daemon disappears (which can happen if it crashes).
	 *
	 * The property can be NULL right after connecting to it, before the D-Bus
	 * transfers the property value to the client. */
	while (!tp->complete &&
	       !g_cancellable_is_cancelled (cancellable) &&
	       ((name_owner = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (transaction))) != NULL ||
		(!saw_name_owner && !tp->owner_changed))) {
		saw_name_owner = saw_name_owner || name_owner != NULL;
		g_clear_pointer (&name_owner, g_free);
		g_main_context_iteration (tp->context, TRUE);
	}

	if (!g_cancellable_set_error_if_cancelled (cancellable, error)) {
		if (tp->error) {
			g_propagate_error (error, g_steal_pointer (&tp->error));
		} else if (!tp->complete && name_owner == NULL) {
			g_set_error_literal (error, G_DBUS_ERROR, G_DBUS_ERROR_NO_REPLY,
					     "Daemon disappeared");
		} else {
			success = TRUE;
		}
	}

out:
	if (cancel_handler)
		g_cancellable_disconnect (cancellable, cancel_handler);
	if (notify_handler != 0)
		g_signal_handler_disconnect (transaction, notify_handler);
	if (signal_handler)
		g_signal_handler_disconnect (transaction, signal_handler);
	if (transaction != NULL)
		g_object_unref (transaction);

	return success;
}

/* FIXME: Refactor this once rpmostree returns a specific error code
 * for ‘transaction in progress’, to avoid the slight race here where
 * lingmo-appstore could return from this function just as another client
 * starts a new transaction.
 * https://github.com/coreos/rpm-ostree/issues/3070 */
static gboolean
gs_rpmostree_wait_for_ongoing_transaction_end (GsRPMOSTreeSysroot *sysroot_proxy,
					       GCancellable *cancellable,
				               GError **error)
{
	g_autofree gchar *current_path = NULL;
	g_autoptr(GMainContext) main_context = NULL;
	gulong notify_handler, cancelled_handler = 0;

	current_path = gs_rpmostree_sysroot_dup_active_transaction_path (sysroot_proxy);
	if (current_path == NULL || *current_path == '\0')
		return TRUE;

	main_context = g_main_context_ref_thread_default ();

	notify_handler = g_signal_connect_swapped (sysroot_proxy, "notify::active-transaction-path",
						   G_CALLBACK (g_main_context_wakeup), main_context);
	if (cancellable) {
		/* Not using g_cancellable_connect() here for simplicity and because checking the state below anyway. */
		cancelled_handler = g_signal_connect_swapped (cancellable, "cancelled",
							      G_CALLBACK (g_main_context_wakeup), main_context);
	}

	while (!g_cancellable_set_error_if_cancelled (cancellable, error)) {
		g_clear_pointer (&current_path, g_free);
		current_path = gs_rpmostree_sysroot_dup_active_transaction_path (sysroot_proxy);
		if (current_path == NULL || *current_path == '\0') {
			g_clear_signal_handler (&notify_handler, sysroot_proxy);
			g_clear_signal_handler (&cancelled_handler, cancellable);
			return TRUE;
		}
		g_main_context_iteration (main_context, TRUE);
	}

	g_clear_signal_handler (&notify_handler, sysroot_proxy);
	g_clear_signal_handler (&cancelled_handler, cancellable);

	gs_rpmostree_error_convert (error);

	return FALSE;
}

static GsApp *
app_from_modified_pkg_variant (GsPlugin *plugin,
			       GVariant *variant,
			       GHashTable *packages_with_urgency)
{
	g_autoptr(GsApp) app = NULL;
	const char *name;
	const char *old_evr, *old_arch;
	const char *new_evr, *new_arch;
	g_autofree char *old_nevra = NULL;
	g_autofree char *new_nevra = NULL;

	g_variant_get (variant, "(us(ss)(ss))", NULL /* type*/, &name, &old_evr, &old_arch, &new_evr, &new_arch);
	old_nevra = g_strdup_printf ("%s-%s.%s", name, old_evr, old_arch);
	new_nevra = g_strdup_printf ("%s-%s.%s", name, new_evr, new_arch);

	app = gs_plugin_cache_lookup (plugin, old_nevra);
	if (app == NULL) {
		/* create new app */
		app = gs_app_new (NULL);
		gs_app_set_management_plugin (app, plugin);
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
		app_set_rpm_ostree_packaging_format (app);
		/* will be downloaded, but the size is unknown */
		gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWN, 0);
		gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
		gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
		gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);

		/* update or downgrade */
		gs_app_add_source (app, name);
		gs_app_set_version (app, old_evr);
		gs_app_set_update_version (app, new_evr);
		gs_app_set_state (app, GS_APP_STATE_UPDATABLE);

		g_debug ("!%s\n", old_nevra);
		g_debug ("=%s\n", new_nevra);

		gs_plugin_cache_add (plugin, old_nevra, app);
	}
	if (packages_with_urgency != NULL) {
		guint urgency = GPOINTER_TO_UINT (g_hash_table_lookup (packages_with_urgency, new_nevra));
		if (urgency > 0)
			gs_app_set_update_urgency (app, urgency);
	}
	return g_steal_pointer (&app);
}

static GsApp *
app_from_single_pkg_variant (GsPlugin *plugin,
			     GVariant *variant,
			     gboolean addition,
			     GHashTable *packages_with_urgency)
{
	g_autoptr(GsApp) app = NULL;
	const char *name;
	const char *evr;
	const char *arch;
	g_autofree char *nevra = NULL;

	g_variant_get (variant, "(usss)", NULL /* type*/, &name, &evr, &arch);
	nevra = g_strdup_printf ("%s-%s.%s", name, evr, arch);

	app = gs_plugin_cache_lookup (plugin, nevra);
	if (app == NULL) {
		/* create new app */
		app = gs_app_new (NULL);
		gs_app_set_management_plugin (app, plugin);
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
		app_set_rpm_ostree_packaging_format (app);
		/* will be downloaded, but the size is unknown */
		gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWN, 0);
		gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
		gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
		gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);

		if (addition) {
			/* addition */
			gs_app_add_source (app, name);
			gs_app_set_version (app, evr);
			gs_app_set_state (app, GS_APP_STATE_AVAILABLE);

			g_debug ("+%s\n", nevra);
		} else {
			/* removal */
			gs_app_add_source (app, name);
			gs_app_set_version (app, evr);
			gs_app_set_state (app, GS_APP_STATE_UNAVAILABLE);

			g_debug ("-%s\n", nevra);
		}

		gs_plugin_cache_add (plugin, nevra, app);
	}
	if (packages_with_urgency != NULL) {
		guint urgency = GPOINTER_TO_UINT (g_hash_table_lookup (packages_with_urgency, nevra));
		if (urgency > 0)
			gs_app_set_update_urgency (app, urgency);
	}
	return g_steal_pointer (&app);
}

typedef enum {
	RPMOSTREE_OPTION_NONE		 = 0,
	RPMOSTREE_OPTION_REBOOT		 = (1 << 0),
	RPMOSTREE_OPTION_ALLOW_DOWNGRADE = (1 << 1),
	RPMOSTREE_OPTION_CACHE_ONLY	 = (1 << 2),
	RPMOSTREE_OPTION_DOWNLOAD_ONLY	 = (1 << 3),
	RPMOSTREE_OPTION_SKIP_PURGE	 = (1 << 4),
	RPMOSTREE_OPTION_NO_PULL_BASE	 = (1 << 5),
	RPMOSTREE_OPTION_DRY_RUN	 = (1 << 6),
	RPMOSTREE_OPTION_NO_OVERRIDES	 = (1 << 7)
} RpmOstreeOptions;

static GVariant *
make_rpmostree_options_variant (RpmOstreeOptions options)
{
	GVariantDict dict;
	g_variant_dict_init (&dict, NULL);
	g_variant_dict_insert (&dict, "reboot", "b", (options & RPMOSTREE_OPTION_REBOOT) != 0);
	g_variant_dict_insert (&dict, "allow-downgrade", "b", (options & RPMOSTREE_OPTION_ALLOW_DOWNGRADE) != 0);
	g_variant_dict_insert (&dict, "cache-only", "b", (options & RPMOSTREE_OPTION_CACHE_ONLY) != 0);
	g_variant_dict_insert (&dict, "download-only", "b", (options & RPMOSTREE_OPTION_DOWNLOAD_ONLY) != 0);
	g_variant_dict_insert (&dict, "skip-purge", "b", (options & RPMOSTREE_OPTION_SKIP_PURGE) != 0);
	g_variant_dict_insert (&dict, "no-pull-base", "b", (options & RPMOSTREE_OPTION_NO_PULL_BASE) != 0);
	g_variant_dict_insert (&dict, "dry-run", "b", (options & RPMOSTREE_OPTION_DRY_RUN) != 0);
	g_variant_dict_insert (&dict, "no-overrides", "b", (options & RPMOSTREE_OPTION_NO_OVERRIDES) != 0);
	return g_variant_ref_sink (g_variant_dict_end (&dict));
}

static GVariant *
make_refresh_md_options_variant (gboolean force)
{
	GVariantDict dict;
	g_variant_dict_init (&dict, NULL);
	g_variant_dict_insert (&dict, "force", "b", force);
	return g_variant_ref_sink (g_variant_dict_end (&dict));
}

static gboolean
make_rpmostree_modifiers_variant (const char *install_package,
                                  const char *uninstall_package,
                                  const char *install_local_package,
                                  GVariant **out_modifiers,
                                  GUnixFDList **out_fd_list,
                                  GError **error)
{
	GVariantDict dict;
	g_autoptr(GUnixFDList) fd_list = g_unix_fd_list_new ();

	g_variant_dict_init (&dict, NULL);

	if (install_package != NULL) {
		g_autoptr(GPtrArray) repo_pkgs = g_ptr_array_new ();

		g_ptr_array_add (repo_pkgs, (gpointer) install_package);

		g_variant_dict_insert_value (&dict, "install-packages",
		                             g_variant_new_strv ((const char *const*)repo_pkgs->pdata,
		                             repo_pkgs->len));

	}

	if (uninstall_package != NULL) {
		g_autoptr(GPtrArray) repo_pkgs = g_ptr_array_new ();

		g_ptr_array_add (repo_pkgs, (gpointer) uninstall_package);

		g_variant_dict_insert_value (&dict, "uninstall-packages",
		                             g_variant_new_strv ((const char *const*)repo_pkgs->pdata,
		                             repo_pkgs->len));

	}

	if (install_local_package != NULL) {
		g_auto(GVariantBuilder) builder;
		int fd;
		int idx;

		g_variant_builder_init (&builder, G_VARIANT_TYPE ("ah"));

		fd = openat (AT_FDCWD, install_local_package, O_RDONLY | O_CLOEXEC | O_NOCTTY);
		if (fd == -1) {
			g_set_error (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED,
			             "Failed to open %s", install_local_package);
			return FALSE;
		}

		idx = g_unix_fd_list_append (fd_list, fd, error);
		if (idx < 0) {
			close (fd);
			return FALSE;
		}

		g_variant_builder_add (&builder, "h", idx);
		g_variant_dict_insert_value (&dict, "install-local-packages",
		                             g_variant_new ("ah", &builder));
		close (fd);
	}

	*out_fd_list = g_steal_pointer (&fd_list);
	*out_modifiers = g_variant_ref_sink (g_variant_dict_end (&dict));
	return TRUE;
}

static gboolean
rpmostree_update_deployment (GsRPMOSTreeOS *os_proxy,
                             const char *install_package,
                             const char *uninstall_package,
                             const char *install_local_package,
                             GVariant *options,
                             gboolean interactive,
                             char **out_transaction_address,
                             GCancellable *cancellable,
                             GError **error)
{
	g_autoptr(GUnixFDList) fd_list = NULL;
	g_autoptr(GVariant) modifiers = NULL;

	if (!make_rpmostree_modifiers_variant (install_package,
	                                       uninstall_package,
	                                       install_local_package,
	                                       &modifiers, &fd_list, error))
		return FALSE;

	return gs_rpmostree_os_call_update_deployment_sync (os_proxy,
	                                                    modifiers,
	                                                    options,
	                                                    interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
	                                                    -1  /* timeout */,
	                                                    fd_list,
	                                                    out_transaction_address,
	                                                    NULL,
	                                                    cancellable,
	                                                    error);
}

static void refresh_metadata_thread_cb (GTask        *task,
                                        gpointer      source_object,
                                        gpointer      task_data,
                                        GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_refresh_metadata_async (GsPlugin                     *plugin,
                                             guint64                       cache_age_secs,
                                             GsPluginRefreshMetadataFlags  flags,
                                             GCancellable                 *cancellable,
                                             GAsyncReadyCallback           callback,
                                             gpointer                      user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_refresh_metadata_async);
	g_task_set_task_data (task, gs_plugin_refresh_metadata_data_new (cache_age_secs, flags), (GDestroyNotify) gs_plugin_refresh_metadata_data_free);

	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				refresh_metadata_thread_cb, g_steal_pointer (&task));
}

static gboolean
gs_plugin_rpm_ostree_refresh_metadata_in_worker (GsPluginRpmOstree *self,
						 GsPluginRefreshMetadataData *data,
						 GsRPMOSTreeOS *os_proxy,
						 GsRPMOSTreeSysroot *sysroot_proxy,
						 GCancellable *cancellable,
						 GError **error)
{
	GsPlugin *plugin = GS_PLUGIN (self);
	g_autoptr(GError) local_error = NULL;
	gboolean done;
	gboolean interactive = data->flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE;

	assert_in_worker (self);

	/* refresh metadata */
	{
		g_autofree gchar *transaction_address = NULL;
		g_autoptr(GsApp) progress_app = NULL;
		g_autoptr(GVariant) options = NULL;
		g_autoptr(TransactionProgress) tp = NULL;

		if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
			return FALSE;

		progress_app = gs_app_new (gs_plugin_get_name (plugin));
		tp = transaction_progress_new ();
		tp->app = g_object_ref (progress_app);
		tp->plugin = g_object_ref (plugin);

		options = make_refresh_md_options_variant (FALSE /* force */);
		done = FALSE;
		while (!done) {
			done = TRUE;
			if (!gs_rpmostree_os_call_refresh_md_sync (os_proxy,
								   options,
								   interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
								   -1  /* timeout */,
								   &transaction_address,
								   cancellable,
								   &local_error)) {
				if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
					g_clear_error (&local_error);
					if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
						return FALSE;
					done = FALSE;
					continue;
				}

				g_propagate_error (error, g_steal_pointer (&local_error));
				gs_rpmostree_error_convert (error);
				return FALSE;
			}
		}

		if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
								 transaction_address,
								 tp,
								 interactive,
								 cancellable,
								 error)) {
			gs_rpmostree_error_convert (error);
			return FALSE;
		}
	}

	if (data->cache_age_secs == G_MAXUINT64)
		return TRUE;

	/* check what can be updated */
	{
		g_autofree gchar *transaction_address = NULL;
		g_autoptr(GsApp) progress_app = gs_app_new (gs_plugin_get_name (plugin));
		g_autoptr(GVariant) options = NULL;
		GVariantDict dict;
		g_autoptr(TransactionProgress) tp = transaction_progress_new ();

		if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
			return FALSE;

		tp->app = g_object_ref (progress_app);
		tp->plugin = g_object_ref (plugin);

		g_variant_dict_init (&dict, NULL);
		g_variant_dict_insert (&dict, "mode", "s", "check");
		options = g_variant_ref_sink (g_variant_dict_end (&dict));

		done = FALSE;
		while (!done) {
			done = TRUE;
			if (!gs_rpmostree_os_call_automatic_update_trigger_sync (os_proxy,
										 options,
										 interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
										 -1  /* timeout */,
										 NULL,
										 &transaction_address,
										 cancellable,
										 &local_error)) {
				if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
					g_clear_error (&local_error);
					if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
						return FALSE;
					done = FALSE;
					continue;
				}
				g_propagate_error (error, g_steal_pointer (&local_error));
				gs_rpmostree_error_convert (error);
				return FALSE;
			}
		}

		if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
		                                                 transaction_address,
		                                                 tp,
		                                                 interactive,
		                                                 cancellable,
		                                                 error)) {
			gs_rpmostree_error_convert (error);
			return FALSE;
		}
	}

	/* update UI */
	gs_plugin_updates_changed (plugin);

	return TRUE;
}

static void
refresh_metadata_thread_cb (GTask        *task,
                            gpointer      source_object,
                            gpointer      task_data,
                            GCancellable *cancellable)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	GsPluginRefreshMetadataData *data = task_data;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	gboolean interactive = data->flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (gs_plugin_rpm_ostree_refresh_metadata_in_worker (self, data, os_proxy, sysroot_proxy, cancellable, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, g_steal_pointer (&local_error));
}

static gboolean
gs_plugin_rpm_ostree_refresh_metadata_finish (GsPlugin      *plugin,
                                              GAsyncResult  *result,
                                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gs_plugin_add_updates (GsPlugin *plugin,
		       GsAppList *list,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GVariant) cached_update = NULL;
	g_autoptr(GVariant) rpm_diff = NULL;
	g_autoptr(GVariant) advisories = NULL;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GHashTable) packages_with_urgency = NULL;
	g_autoptr(GError) local_error = NULL;
	const gchar *checksum = NULL;
	const gchar *version = NULL;
	g_auto(GVariantDict) cached_update_dict;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error)) {
		g_debug ("Failed to ref proxies to get updates: %s", local_error->message);
		return TRUE;
	}

	/* ensure D-Bus properties are updated before reading them */
	if (!gs_rpmostree_sysroot_call_reload_sync (sysroot_proxy,
						    interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						    -1  /* timeout */,
						    cancellable,
						    &local_error)) {
		g_debug ("Failed to call reload to get updates: %s", local_error->message);
		return TRUE;
	}

	cached_update = gs_rpmostree_os_dup_cached_update (os_proxy);
	g_variant_dict_init (&cached_update_dict, cached_update);

	if (!g_variant_dict_lookup (&cached_update_dict, "checksum", "&s", &checksum))
		return TRUE;
	if (!g_variant_dict_lookup (&cached_update_dict, "version", "&s", &version))
		return TRUE;

	g_debug ("got CachedUpdate version '%s', checksum '%s'", version, checksum);

	advisories = g_variant_dict_lookup_value (&cached_update_dict, "advisories", G_VARIANT_TYPE ("a(suuasa{sv})"));
	if (advisories != NULL) {
		GVariantIter iter;
		GVariant *child;

		packages_with_urgency = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

		g_variant_iter_init (&iter, advisories);
		while ((child = g_variant_iter_next_value (&iter)) != NULL) {
			GVariantIter *packages_iter = NULL;
			guint severity = 0;
			g_variant_get (child, "(suuasa{sv})", NULL /* id */, NULL /* kind */, &severity, &packages_iter, NULL /* metadata */);
			if (packages_iter) {
				guint urgency = AS_URGENCY_KIND_UNKNOWN; /* RPM_OSTREE_ADVISORY_SEVERITY_NONE */
				switch (severity) {
				case 1: /* RPM_OSTREE_ADVISORY_SEVERITY_LOW */
					urgency = AS_URGENCY_KIND_LOW;
					break;
				case 2: /* RPM_OSTREE_ADVISORY_SEVERITY_MODERATE */
					urgency = AS_URGENCY_KIND_MEDIUM;
					break;
				case 3: /* RPM_OSTREE_ADVISORY_SEVERITY_IMPORTANT */
					urgency = AS_URGENCY_KIND_HIGH;
					break;
				case 4: /* RPM_OSTREE_ADVISORY_SEVERITY_CRITICAL */
					urgency = AS_URGENCY_KIND_CRITICAL;
					break;
				default:
					break;
				}
				if (urgency != AS_URGENCY_KIND_UNKNOWN) {
					const gchar *pkgname = NULL;
					while (g_variant_iter_loop (packages_iter, "s", &pkgname)) {
						if (pkgname != NULL && *pkgname != '\0') {
							if (GPOINTER_TO_UINT (g_hash_table_lookup (packages_with_urgency, pkgname)) < urgency)
								g_hash_table_insert (packages_with_urgency, g_strdup (pkgname), GUINT_TO_POINTER (urgency));
						}
						pkgname = NULL;
					}
				}
			}
			g_variant_unref (child);
		}
	}

	rpm_diff = g_variant_dict_lookup_value (&cached_update_dict, "rpm-diff", G_VARIANT_TYPE ("a{sv}"));
	if (rpm_diff != NULL) {
		GVariantIter iter;
		GVariant *child;
		g_autoptr(GVariant) upgraded = NULL;
		g_autoptr(GVariant) downgraded = NULL;
		g_autoptr(GVariant) removed = NULL;
		g_autoptr(GVariant) added = NULL;
		g_auto(GVariantDict) rpm_diff_dict;
		g_variant_dict_init (&rpm_diff_dict, rpm_diff);

		upgraded = g_variant_dict_lookup_value (&rpm_diff_dict, "upgraded", G_VARIANT_TYPE ("a(us(ss)(ss))"));
		if (upgraded == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_INVALID_FORMAT,
			                     "no 'upgraded' in rpm-diff dict");
			return FALSE;
		}
		downgraded = g_variant_dict_lookup_value (&rpm_diff_dict, "downgraded", G_VARIANT_TYPE ("a(us(ss)(ss))"));
		if (downgraded == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_INVALID_FORMAT,
			                     "no 'downgraded' in rpm-diff dict");
			return FALSE;
		}
		removed = g_variant_dict_lookup_value (&rpm_diff_dict, "removed", G_VARIANT_TYPE ("a(usss)"));
		if (removed == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_INVALID_FORMAT,
			                     "no 'removed' in rpm-diff dict");
			return FALSE;
		}
		added = g_variant_dict_lookup_value (&rpm_diff_dict, "added", G_VARIANT_TYPE ("a(usss)"));
		if (added == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_INVALID_FORMAT,
			                     "no 'added' in rpm-diff dict");
			return FALSE;
		}

		/* iterate over all upgraded packages and add them */
		g_variant_iter_init (&iter, upgraded);
		while ((child = g_variant_iter_next_value (&iter)) != NULL) {
			g_autoptr(GsApp) app = app_from_modified_pkg_variant (plugin, child, packages_with_urgency);
			if (app != NULL)
				gs_app_list_add (list, app);
			g_variant_unref (child);
		}

		/* iterate over all downgraded packages and add them */
		g_variant_iter_init (&iter, downgraded);
		while ((child = g_variant_iter_next_value (&iter)) != NULL) {
			g_autoptr(GsApp) app = app_from_modified_pkg_variant (plugin, child, packages_with_urgency);
			if (app != NULL)
				gs_app_list_add (list, app);
			g_variant_unref (child);
		}

		/* iterate over all removed packages and add them */
		g_variant_iter_init (&iter, removed);
		while ((child = g_variant_iter_next_value (&iter)) != NULL) {
			g_autoptr(GsApp) app = app_from_single_pkg_variant (plugin, child, FALSE, packages_with_urgency);
			if (app != NULL)
				gs_app_list_add (list, app);
			g_variant_unref (child);
		}

		/* iterate over all added packages and add them */
		g_variant_iter_init (&iter, added);
		while ((child = g_variant_iter_next_value (&iter)) != NULL) {
			g_autoptr(GsApp) app = app_from_single_pkg_variant (plugin, child, TRUE, packages_with_urgency);
			if (app != NULL)
				gs_app_list_add (list, app);
			g_variant_unref (child);
		}
	}

	return TRUE;
}

static gboolean
trigger_rpmostree_update (GsPluginRpmOstree *self,
                          GsApp *app,
			  GsRPMOSTreeOS *os_proxy,
			  GsRPMOSTreeSysroot *sysroot_proxy,
			  gboolean interactive,
			  GCancellable *cancellable,
                          GError **error)
{
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariant) options = NULL;
	g_autoptr(TransactionProgress) tp = transaction_progress_new ();
	g_autoptr(GError) local_error = NULL;
	gboolean done;

	/* if we can process this online do not require a trigger */
	if (gs_app_get_state (app) != GS_APP_STATE_UPDATABLE)
		return TRUE;

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, GS_PLUGIN (self)))
		return TRUE;

	/* already in correct state */
	if (self->update_triggered)
		return TRUE;

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	/* trigger the update */
	options = make_rpmostree_options_variant (RPMOSTREE_OPTION_CACHE_ONLY);
	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!gs_rpmostree_os_call_upgrade_sync (os_proxy,
							options,
							interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							-1  /* timeout */,
							NULL /* fd list */,
							&transaction_address,
							NULL /* fd list out */,
							cancellable,
							&local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
					return FALSE;
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			return FALSE;
		}
	}

	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);
		return FALSE;
	}

	self->update_triggered = TRUE;

	/* success */
	return TRUE;
}

static void update_apps_thread_cb (GTask        *task,
                                   gpointer      source_object,
                                   gpointer      task_data,
                                   GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_update_apps_async (GsPlugin                           *plugin,
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
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

	task = gs_plugin_update_apps_data_new_task (plugin, apps, flags,
						    progress_callback, progress_user_data,
						    app_needs_user_action_callback, app_needs_user_action_data,
						    cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_update_apps_async);

	/* Queue a job to update the apps. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				update_apps_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
update_apps_thread_cb (GTask        *task,
                       gpointer      source_object,
                       gpointer      task_data,
                       GCancellable *cancellable)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (source_object);
	GsPluginUpdateAppsData *data = task_data;
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	gboolean interactive = data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE;

	assert_in_worker (self);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (!(data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD)) {
		GsPlugin *plugin = GS_PLUGIN (self);
		g_autofree gchar *transaction_address = NULL;
		g_autoptr(GVariant) options = NULL;
		g_autoptr(TransactionProgress) tp = transaction_progress_new ();
		gboolean done;

		if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		tp->download_progress_list = g_object_ref (data->apps);
		tp->plugin = g_object_ref (plugin);

		options = make_rpmostree_options_variant (RPMOSTREE_OPTION_DOWNLOAD_ONLY);
		done = FALSE;
		while (!done) {
			done = TRUE;
			if (!gs_rpmostree_os_call_upgrade_sync (os_proxy,
								options,
								interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
								-1  /* timeout */,
								NULL /* fd list */,
								&transaction_address,
								NULL /* fd list out */,
								cancellable,
								&local_error)) {
				if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
					g_clear_error (&local_error);
					if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, &local_error)) {
						g_task_return_error (task, g_steal_pointer (&local_error));
						return;
					}
					done = FALSE;
					continue;
				}
				gs_rpmostree_error_convert (&local_error);
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}

		if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
		                                                 transaction_address,
		                                                 tp,
								 interactive,
		                                                 cancellable,
		                                                 &local_error)) {
			gs_app_list_override_progress (data->apps, GS_APP_PROGRESS_UNKNOWN);
			gs_rpmostree_error_convert (&local_error);
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		gs_app_list_override_progress (data->apps, GS_APP_PROGRESS_UNKNOWN);

		for (guint i = 0; i < gs_app_list_length (data->apps); i++) {
			GsApp *app = gs_app_list_index (data->apps, i);
			GsAppList *related = gs_app_get_related (app);

			if (!gs_app_has_quirk (app, GS_APP_QUIRK_IS_PROXY) &&
			    gs_app_has_management_plugin (app, plugin))
				gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);

			for (guint j = 0; j < gs_app_list_length (related); j++) {
				GsApp *app_tmp = gs_app_list_index (related, j);

				if (gs_app_has_management_plugin (app_tmp, plugin))
					gs_app_set_size_download (app_tmp, GS_SIZE_TYPE_VALID, 0);
			}
		}
	}

	/* Doing updates is only a case of triggering the deploy of a
	 * pre-downloaded update, so there’s no need to bother with progress updates. */
	if (data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	self->update_triggered = FALSE;

	for (guint i = 0; !self->update_triggered && i < gs_app_list_length (data->apps); i++) {
		GsApp *app = gs_app_list_index (data->apps, i);
		GsAppList *related = gs_app_get_related (app);

		/* we don't currently put all updates in the OsUpdate proxy app */
		if (!gs_app_has_quirk (app, GS_APP_QUIRK_IS_PROXY)) {
			if (!trigger_rpmostree_update (self, app, os_proxy, sysroot_proxy, interactive, cancellable, &local_error)) {
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}

		/* try to trigger each related app */
		for (guint j = 0; !self->update_triggered && j < gs_app_list_length (related); j++) {
			GsApp *app_tmp = gs_app_list_index (related, j);

			if (!trigger_rpmostree_update (self, app_tmp, os_proxy, sysroot_proxy, interactive, cancellable, &local_error)) {
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_rpm_ostree_update_apps_finish (GsPlugin      *plugin,
                                         GAsyncResult  *result,
                                         GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gs_plugin_app_upgrade_trigger (GsPlugin *plugin,
                               GsApp *app,
                               GCancellable *cancellable,
                               GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	const char *packages[] = { NULL };
	g_autofree gchar *new_refspec = NULL;
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariant) options = NULL;
	g_autoptr(TransactionProgress) tp = transaction_progress_new ();
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GError) local_error = NULL;
	gboolean done;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	/* check is distro-upgrade */
	if (gs_app_get_kind (app) != AS_COMPONENT_KIND_OPERATING_SYSTEM)
		return TRUE;

	gs_app_set_state (app, GS_APP_STATE_PENDING_INSTALL);
	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error)) {
		gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
		return FALSE;
	}

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
		gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
		return FALSE;
	}

	/* construct new refspec based on the distro version we're upgrading to */
	new_refspec = g_strdup_printf ("ostree://fedora/%s/x86_64/silverblue",
	                               gs_app_get_version (app));

	/* trigger the upgrade */
	options = make_rpmostree_options_variant (RPMOSTREE_OPTION_ALLOW_DOWNGRADE |
	                                          RPMOSTREE_OPTION_CACHE_ONLY);
	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!gs_rpmostree_os_call_rebase_sync (os_proxy,
						       options,
						       new_refspec,
						       packages,
						       interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						       -1  /* timeout */,
						       NULL /* fd list */,
						       &transaction_address,
						       NULL /* fd list out */,
						       cancellable,
						       &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
			return FALSE;
		}
	}

	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);

		if (g_strrstr ((*error)->message, "Old and new refs are equal")) {
			/* don't error out if the correct tree is already deployed */
			g_debug ("ignoring rpm-ostree error: %s", (*error)->message);
			g_clear_error (error);
		} else {
			gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
			return FALSE;
		}
	}

	gs_app_set_state (app, GS_APP_STATE_UPDATABLE);

	/* success */
	return TRUE;
}

static gboolean
gs_rpmostree_repo_enable (GsPlugin *plugin,
			  GsApp *app,
			  gboolean enable,
			  GsRPMOSTreeOS *os_proxy,
			  GsRPMOSTreeSysroot *sysroot_proxy,
			  gboolean interactive,
			  GCancellable *cancellable,
			  GError **error)
{
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariantBuilder) options_builder = NULL;
	g_autoptr(TransactionProgress) tp = NULL;
	g_autoptr(GError) local_error = NULL;
	gboolean done;

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	if (enable)
		gs_app_set_state (app, GS_APP_STATE_INSTALLING);
	else
		gs_app_set_state (app, GS_APP_STATE_REMOVING);

	done = FALSE;
	while (!done) {
		done = TRUE;
		g_clear_pointer (&options_builder, g_variant_builder_unref);
		options_builder = g_variant_builder_new (G_VARIANT_TYPE ("a{ss}"));
		g_variant_builder_add (options_builder, "{ss}", "enabled", enable ? "1" : "0");
		if (!gs_rpmostree_os_call_modify_yum_repo_sync (os_proxy,
								gs_app_get_id (app),
								g_variant_builder_end (options_builder),
								interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
								-1  /* timeout */,
								&transaction_address,
								cancellable,
								&local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					gs_app_set_state_recover (app);
					gs_utils_error_add_origin_id (error, app);
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			gs_app_set_state_recover (app);
			gs_utils_error_add_origin_id (error, app);
			return FALSE;
		}
	}

	tp = transaction_progress_new ();
	tp->app = g_object_ref (app);
	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);
		gs_app_set_state_recover (app);
		gs_utils_error_add_origin_id (error, app);
		return FALSE;
	}


	/* state is known */
	if (enable)
		gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	else
		gs_app_set_state (app, GS_APP_STATE_AVAILABLE);

	gs_plugin_repository_changed (plugin, app);

	return TRUE;
}

gboolean
gs_plugin_app_install (GsPlugin *plugin,
                       GsApp *app,
                       GCancellable *cancellable,
                       GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	const gchar *install_package = NULL;
	g_autofree gchar *local_filename = NULL;
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariant) options = NULL;
	g_autoptr(TransactionProgress) tp = transaction_progress_new ();
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GError) local_error = NULL;
	gboolean done;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	/* enable repo, handled by dedicated function */
	g_return_val_if_fail (gs_app_get_kind (app) != AS_COMPONENT_KIND_REPOSITORY, FALSE);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error))
		return FALSE;

	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		if (gs_app_get_source_default (app) == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			                     "no source set");
			return FALSE;
		}

		install_package = gs_app_get_source_default (app);
		break;
	case GS_APP_STATE_AVAILABLE_LOCAL:
		if (gs_app_get_local_file (app) == NULL) {
			g_set_error_literal (error,
			                     GS_PLUGIN_ERROR,
			                     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			                     "local package, but no filename");
			return FALSE;
		}

		local_filename = g_file_get_path (gs_app_get_local_file (app));
		break;
	default:
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_NOT_SUPPORTED,
		             "do not know how to install app in state %s",
		             gs_app_state_to_string (gs_app_get_state (app)));
		return FALSE;
	}

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	gs_app_set_state (app, GS_APP_STATE_INSTALLING);
	tp->app = g_object_ref (app);

	options = make_rpmostree_options_variant (RPMOSTREE_OPTION_NO_PULL_BASE);
	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!rpmostree_update_deployment (os_proxy,
						  install_package,
						  NULL /* remove package */,
						  local_filename,
						  options,
						  interactive,
						  &transaction_address,
						  cancellable,
						  &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					gs_app_set_state_recover (app);
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			gs_app_set_state_recover (app);
			return FALSE;
		}
	}

	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);
		gs_app_set_state_recover (app);
		return FALSE;
	}

	/* state is known */
	gs_app_set_state (app, GS_APP_STATE_PENDING_INSTALL);

	/* get the new icon from the package */
	gs_app_set_local_file (app, NULL);
	gs_app_remove_all_icons (app);

	/* no longer valid */
	gs_app_clear_source_ids (app);

	return TRUE;
}

gboolean
gs_plugin_app_remove (GsPlugin *plugin,
                      GsApp *app,
                      GCancellable *cancellable,
                      GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariant) options = NULL;
	g_autoptr(TransactionProgress) tp = transaction_progress_new ();
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GError) local_error = NULL;
	gboolean done;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error))
		return FALSE;

	/* disable repo, handled by dedicated function */
	g_return_val_if_fail (gs_app_get_kind (app) != AS_COMPONENT_KIND_REPOSITORY, FALSE);

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	gs_app_set_state (app, GS_APP_STATE_REMOVING);
	tp->app = g_object_ref (app);

	options = make_rpmostree_options_variant (RPMOSTREE_OPTION_CACHE_ONLY |
						  RPMOSTREE_OPTION_NO_PULL_BASE);
	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!rpmostree_update_deployment (os_proxy,
						  NULL /* install package */,
						  gs_app_get_source_default (app),
						  NULL /* install local package */,
						  options,
						  interactive,
						  &transaction_address,
						  cancellable,
						  &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					gs_app_set_state_recover (app);
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			gs_app_set_state_recover (app);
			return FALSE;
		}
	}

	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);
		gs_app_set_state_recover (app);
		return FALSE;
	}

	if (gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT)) {
		gs_app_set_state (app, GS_APP_STATE_PENDING_REMOVE);
	} else {
		/* state is not known: we don't know if we can re-install this app */
		gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
	}

	return TRUE;
}

static gboolean
gs_rpm_ostree_has_launchable (GsApp *app)
{
	const gchar *desktop_id;
	GDesktopAppInfo *desktop_appinfo;

	if (gs_app_has_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE) ||
	    gs_app_has_quirk (app, GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE))
		return FALSE;

	desktop_id = gs_app_get_launchable (app, AS_LAUNCHABLE_KIND_DESKTOP_ID);
	if (!desktop_id)
		desktop_id = gs_app_get_id (app);
	if (!desktop_id)
		return FALSE;

	desktop_appinfo = gs_utils_get_desktop_app_info (desktop_id);
	if (!desktop_appinfo)
		return FALSE;

	return TRUE;
}

static gboolean
resolve_installed_packages_app (GsPlugin *plugin,
                                GHashTable *packages,
                                GHashTable *layered_packages,
                                GHashTable *layered_local_packages,
                                GsApp *app)
{
	RpmOstreePackage *pkg;

	if (!gs_app_get_source_default (app))
		return FALSE;

	pkg = g_hash_table_lookup (packages, gs_app_get_source_default (app));

	if (pkg) {
		gs_app_set_version (app, rpm_ostree_package_get_evr (pkg));
		gs_app_set_metadata (app, "LingmoAppstore::packagename-value", rpm_ostree_package_get_nevra (pkg));
		if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN) {
			/* Kind of hack, pending installs do not have available the desktop file */
			if (gs_app_get_kind (app) != AS_COMPONENT_KIND_DESKTOP_APP || gs_rpm_ostree_has_launchable (app))
				gs_app_set_state (app, GS_APP_STATE_INSTALLED);
			else
				gs_app_set_state (app, GS_APP_STATE_PENDING_INSTALL);
		}
		if ((rpm_ostree_package_get_name (pkg) &&
		     g_hash_table_contains (layered_packages, rpm_ostree_package_get_name (pkg))) ||
		    (rpm_ostree_package_get_nevra (pkg) &&
		     g_hash_table_contains (layered_local_packages, rpm_ostree_package_get_nevra (pkg)))) {
			/* layered packages can always be removed */
			gs_app_remove_quirk (app, GS_APP_QUIRK_COMPULSORY);
		} else {
			/* can't remove packages that are part of the base system */
			gs_app_add_quirk (app, GS_APP_QUIRK_COMPULSORY);
		}
		if (gs_app_get_origin (app) == NULL)
			gs_app_set_origin (app, "rpm-ostree");
		if (gs_app_get_name (app) == NULL)
			gs_app_set_name (app, GS_APP_QUALITY_LOWEST, rpm_ostree_package_get_name (pkg));
		return TRUE /* found */;
	}

	return FALSE /* not found */;
}

static gboolean
resolve_appstream_source_file_to_package_name (GsPlugin *plugin,
                                               GsApp *app,
                                               GsPluginRefineFlags flags,
                                               GCancellable *cancellable,
                                               GError **error)
{
	Header h;
	const gchar *fn;
	gint rc;
	g_auto(rpmdbMatchIterator) mi = NULL;
	g_auto(rpmts) ts = NULL;

	/* open db readonly */
	ts = rpmtsCreate();
	rpmtsSetRootDir (ts, NULL);
	rc = rpmtsOpenDB (ts, O_RDONLY);
	if (rc != 0) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_NOT_SUPPORTED,
		             "Failed to open rpmdb: %i", rc);
		return FALSE;
	}

	/* look for a specific file */
	fn = gs_app_get_metadata_item (app, "appstream::source-file");
	if (fn == NULL)
		return TRUE;

	mi = rpmtsInitIterator (ts, RPMDBI_INSTFILENAMES, fn, 0);
	if (mi == NULL) {
		g_debug ("rpm: no search results for %s", fn);
		return TRUE;
	}

	/* process any results */
	g_debug ("rpm: querying for %s with %s", gs_app_get_id (app), fn);
	while ((h = rpmdbNextIterator (mi)) != NULL) {
		const gchar *name;

		/* add default source */
		name = headerGetString (h, RPMTAG_NAME);
		if (gs_app_get_source_default (app) == NULL) {
			const gchar *nevra = headerGetString (h, RPMTAG_NEVRA);
			g_debug ("rpm: setting source to '%s' with nevra '%s'", name, nevra);
			gs_app_add_source (app, name);
			gs_app_set_metadata (app, "LingmoAppstore::packagename-value", nevra);
			gs_app_set_management_plugin (app, plugin);
			gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
			app_set_rpm_ostree_packaging_format (app);
			gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
		}
	}

	return TRUE;
}

static gboolean
gs_rpm_ostree_refine_apps (GsPlugin *plugin,
			   GsAppList *list,
			   GsPluginRefineFlags flags,
			   GCancellable *cancellable,
			   GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GHashTable) packages = NULL;
	g_autoptr(GHashTable) layered_packages = NULL;
	g_autoptr(GHashTable) layered_local_packages = NULL;
	g_autoptr(GHashTable) lookup_apps = NULL; /* name ~> GsApp */
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GPtrArray) pkglist = NULL;
	g_autoptr(GVariant) default_deployment = NULL;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(OstreeRepo) ot_repo = NULL;
	g_auto(GStrv) layered_packages_strv = NULL;
	g_auto(GStrv) layered_local_packages_strv = NULL;
	g_autofree gchar *checksum = NULL;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	locker = g_mutex_locker_new (&self->mutex);

	if (!gs_rpmostree_ref_proxies_locked (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error))
		return FALSE;

	ot_repo = g_object_ref (self->ot_repo);

	g_clear_pointer (&locker, g_mutex_locker_free);

	/* ensure D-Bus properties are updated before reading them */
	if (!gs_rpmostree_sysroot_call_reload_sync (sysroot_proxy,
						    interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						    -1  /* timeout */,
						    cancellable,
						    error)) {
		gs_rpmostree_error_convert (error);
		return FALSE;
	}

	default_deployment = gs_rpmostree_os_dup_default_deployment (os_proxy);
	g_assert (g_variant_lookup (default_deployment,
	                            "packages", "^as",
	                            &layered_packages_strv));
	g_assert (g_variant_lookup (default_deployment,
	                            "requested-local-packages", "^as",
	                            &layered_local_packages_strv));
	g_assert (g_variant_lookup (default_deployment,
	                            "checksum", "s",
	                            &checksum));

	pkglist = rpm_ostree_db_query_all (ot_repo, checksum, cancellable, error);
	if (pkglist == NULL) {
		gs_rpmostree_error_convert (error);
		return FALSE;
	}

	packages = g_hash_table_new (g_str_hash, g_str_equal);
	layered_packages = g_hash_table_new (g_str_hash, g_str_equal);
	layered_local_packages = g_hash_table_new (g_str_hash, g_str_equal);

	for (guint ii = 0; ii < pkglist->len; ii++) {
		RpmOstreePackage *pkg = g_ptr_array_index (pkglist, ii);
		if (rpm_ostree_package_get_name (pkg))
			g_hash_table_insert (packages, (gpointer) rpm_ostree_package_get_name (pkg), pkg);
	}

	for (guint ii = 0; layered_packages_strv && layered_packages_strv[ii]; ii++) {
		g_hash_table_add (layered_packages, layered_packages_strv[ii]);
	}

	for (guint ii = 0; layered_local_packages_strv && layered_local_packages_strv[ii]; ii++) {
		g_hash_table_add (layered_local_packages, layered_local_packages_strv[ii]);
	}

	lookup_apps = g_hash_table_new (g_str_hash, g_str_equal);

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD))
			continue;
		/* set management plugin for apps where appstream just added the source package name in refine() */
		if (gs_app_has_management_plugin (app, NULL) &&
		    gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_PACKAGE &&
		    gs_app_get_scope (app) == AS_COMPONENT_SCOPE_SYSTEM &&
		    gs_app_get_source_default (app) != NULL) {
			gs_app_set_management_plugin (app, plugin);
			gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
			app_set_rpm_ostree_packaging_format (app);
		}
		/* resolve the source package name based on installed appdata/desktop file name */
		if (gs_app_has_management_plugin (app, NULL) &&
		    gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_UNKNOWN &&
		    gs_app_get_scope (app) == AS_COMPONENT_SCOPE_SYSTEM &&
		    gs_app_get_source_default (app) == NULL) {
			if (!resolve_appstream_source_file_to_package_name (plugin, app, flags, cancellable, error))
				return FALSE;
		}
		if (!gs_app_has_management_plugin (app, plugin))
			continue;
		if (gs_app_get_source_default (app) == NULL)
			continue;

		/* first try to resolve from installed packages and
		   if we didn't find anything, try resolving from available packages */
		if (!resolve_installed_packages_app (plugin, packages, layered_packages, layered_local_packages, app))
			g_hash_table_insert (lookup_apps, (gpointer) gs_app_get_source_default (app), app);
	}

	if (g_hash_table_size (lookup_apps) > 0) {
		g_autofree gpointer *names = NULL;
		g_autoptr(GError) local_error = NULL;
		g_autoptr(GVariant) var_packages = NULL;

		names = g_hash_table_get_keys_as_array (lookup_apps, NULL);
		if (gs_rpmostree_os_call_get_packages_sync (os_proxy,
							    (const gchar * const *) names,
							    interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							    -1  /* timeout */,
							    &var_packages,
							    cancellable,
							    &local_error)) {
			gsize n_children = g_variant_n_children (var_packages);
			for (gsize i = 0; i < n_children; i++) {
				g_autoptr(GVariant) value = g_variant_get_child_value (var_packages, i);
				g_autoptr(GVariantDict) dict = g_variant_dict_new (value);
				GsApp *app;
				const gchar *key = NULL;
				const gchar *evr = NULL;
				const gchar *nevra = NULL;
				const gchar *reponame = NULL;
				const gchar *name = NULL;
				const gchar *summary = NULL;

				if (!g_variant_dict_lookup (dict, "key", "&s", &key) ||
				    !g_variant_dict_lookup (dict, "evr", "&s", &evr) ||
				    !g_variant_dict_lookup (dict, "nevra", "&s", &nevra) ||
				    !g_variant_dict_lookup (dict, "reponame", "&s", &reponame) ||
				    !g_variant_dict_lookup (dict, "name", "&s", &name) ||
				    !g_variant_dict_lookup (dict, "summary", "&s", &summary)) {
					continue;
				}

				app = g_hash_table_lookup (lookup_apps, key);
				if (app == NULL)
					continue;

				gs_app_set_version (app, evr);
				if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN)
					gs_app_set_state (app, GS_APP_STATE_AVAILABLE);

				gs_app_set_metadata (app, "LingmoAppstore::packagename-value", nevra);

				/* anything not part of the base system can be removed */
				gs_app_remove_quirk (app, GS_APP_QUIRK_COMPULSORY);

				/* set origin */
				if (gs_app_get_origin (app) == NULL)
					gs_app_set_origin (app, reponame);

				/* set more metadata for packages that don't have appstream data */
				gs_app_set_name (app, GS_APP_QUALITY_LOWEST, name);
				gs_app_set_summary (app, GS_APP_QUALITY_LOWEST, summary);

				/* set hide-from-search quirk for available apps we don't want to show; results for non-installed desktop apps
				 * are intentionally hidden (as recommended by Matthias Clasen) by a special quirk because app layering
				 * should be intended for power users and not a common practice on Fedora Silverblue */
				if (!gs_app_is_installed (app)) {
					switch (gs_app_get_kind (app)) {
					case AS_COMPONENT_KIND_DESKTOP_APP:
					case AS_COMPONENT_KIND_WEB_APP:
					case AS_COMPONENT_KIND_CONSOLE_APP:
						gs_app_add_quirk (app, GS_APP_QUIRK_HIDE_FROM_SEARCH);
						break;
					default:
						break;
					}
				}
			}
		} else {
			g_debug ("Failed to get packages: %s", local_error->message);
		}
	}

	return TRUE;
}

static void refine_thread_cb (GTask        *task,
                              gpointer      source_object,
                              gpointer      task_data,
                              GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_refine_async (GsPlugin            *plugin,
                                   GsAppList           *list,
                                   GsPluginRefineFlags  flags,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = gs_plugin_has_flags (GS_PLUGIN (self), GS_PLUGIN_FLAGS_INTERACTIVE);

	task = gs_plugin_refine_data_new_task (plugin, list, flags, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_refine_async);

	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				refine_thread_cb, g_steal_pointer (&task));
}

static void
refine_thread_cb (GTask        *task,
                  gpointer      source_object,
                  gpointer      task_data,
                  GCancellable *cancellable)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	GsPluginRefineData *data = task_data;
	GsAppList *list = data->list;
	GsPluginRefineFlags flags = data->flags;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_rpm_ostree_refine_apps (plugin, list, flags, cancellable, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_rpm_ostree_refine_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gs_plugin_app_upgrade_download (GsPlugin *plugin,
                                GsApp *app,
                                GCancellable *cancellable,
                                GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	const char *packages[] = { NULL };
	g_autofree gchar *new_refspec = NULL;
	g_autofree gchar *transaction_address = NULL;
	g_autoptr(GVariant) options = NULL;
	g_autoptr(TransactionProgress) tp = transaction_progress_new ();
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GError) local_error = NULL;
	gboolean done;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	/* check is distro-upgrade */
	if (gs_app_get_kind (app) != AS_COMPONENT_KIND_OPERATING_SYSTEM)
		return TRUE;

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error))
		return FALSE;

	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	/* construct new refspec based on the distro version we're upgrading to */
	new_refspec = g_strdup_printf ("ostree://fedora/%s/x86_64/silverblue",
	                               gs_app_get_version (app));

	options = make_rpmostree_options_variant (RPMOSTREE_OPTION_ALLOW_DOWNGRADE |
	                                          RPMOSTREE_OPTION_DOWNLOAD_ONLY);
	gs_app_set_state (app, GS_APP_STATE_DOWNLOADING);
	tp->app = g_object_ref (app);

	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!gs_rpmostree_os_call_rebase_sync (os_proxy,
						       options,
						       new_refspec,
						       packages,
						       interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
						       -1  /* timeout */,
						       NULL /* fd list */,
						       &transaction_address,
						       NULL /* fd list out */,
						       cancellable,
						       &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					gs_app_set_state_recover (app);
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			if (local_error)
				g_propagate_error (error, g_steal_pointer (&local_error));
			gs_rpmostree_error_convert (error);
			gs_app_set_state_recover (app);
			return FALSE;
		}
	}

	if (!gs_rpmostree_transaction_get_response_sync (sysroot_proxy,
	                                                 transaction_address,
	                                                 tp,
	                                                 interactive,
	                                                 cancellable,
	                                                 error)) {
		gs_rpmostree_error_convert (error);

		if (g_strrstr ((*error)->message, "Old and new refs are equal")) {
			/* don't error out if the correct tree is already deployed */
			g_debug ("ignoring rpm-ostree error: %s", (*error)->message);
			g_clear_error (error);
		} else {
			gs_app_set_state_recover (app);
			return FALSE;
		}
	}

	/* state is known */
	gs_app_set_state (app, GS_APP_STATE_UPDATABLE);
	return TRUE;
}


static gboolean
plugin_rpmostree_pick_rpm_desktop_file_cb (GsPlugin *plugin,
					   GsApp *app,
					   const gchar *filename,
					   GKeyFile *key_file)
{
	return strstr (filename, "/snapd/") == NULL &&
	       strstr (filename, "/snap/") == NULL &&
	       strstr (filename, "/flatpak/") == NULL &&
	       g_key_file_has_group (key_file, "Desktop Entry") &&
	       !g_key_file_has_key (key_file, "Desktop Entry", "X-Flatpak", NULL) &&
	       !g_key_file_has_key (key_file, "Desktop Entry", "X-SnapInstanceName", NULL);
}

gboolean
gs_plugin_launch (GsPlugin *plugin,
                  GsApp *app,
                  GCancellable *cancellable,
                  GError **error)
{
	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	return gs_plugin_app_launch_filtered (plugin, app, plugin_rpmostree_pick_rpm_desktop_file_cb, NULL, error);
}

static void
add_quirks_from_package_name (GsApp *app, const gchar *package_name)
{
	/* these packages don't have a .repo file in their file lists, but
	 * instead install one through rpm scripts / cron job */
	const gchar *packages_with_repos[] = {
		"google-chrome-stable",
		"google-earth-pro-stable",
		"google-talkplugin",
		NULL };

	if (g_strv_contains (packages_with_repos, package_name))
		gs_app_add_quirk (app, GS_APP_QUIRK_HAS_SOURCE);
}

gboolean
gs_plugin_file_to_app (GsPlugin *plugin,
		       GsAppList *list,
		       GFile *file,
		       GCancellable *cancellable,
		       GError **error)
{
	gboolean ret = FALSE;
	FD_t rpmfd = NULL;
	guint64 epoch;
	guint64 size;
	const gchar *name;
	const gchar *version;
	const gchar *release;
	const gchar *license;
	g_auto(Header) h = NULL;
	g_auto(rpmts) ts = NULL;
	g_autofree gchar *evr = NULL;
	g_autofree gchar *filename = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) tmp_list = NULL;

	filename = g_file_get_path (file);
	if (!g_str_has_suffix (filename, ".rpm")) {
		ret = TRUE;
		goto out;
	}

	ts = rpmtsCreate ();
	rpmtsSetVSFlags (ts, _RPMVSF_NOSIGNATURES);

	/* librpm needs Fopenfd */
	rpmfd = Fopen (filename, "r.fdio");
	if (rpmfd == NULL) {
		g_set_error (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED,
		             "Opening %s failed", filename);
		goto out;
	}
	if (Ferror (rpmfd)) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "Opening %s failed: %s",
		             filename,
		             Fstrerror (rpmfd));
		goto out;
	}

	if (rpmReadPackageFile (ts, rpmfd, filename, &h) != RPMRC_OK) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "Verification of %s failed",
		             filename);
		goto out;
	}

	app = gs_app_new (NULL);
	gs_app_set_metadata (app, "LingmoAppstore::Creator", gs_plugin_get_name (plugin));
	gs_app_set_management_plugin (app, plugin);
	if (h) {
		const gchar *str;

		str = headerGetString (h, RPMTAG_NAME);
		if (str && *str)
			gs_app_set_name (app, GS_APP_QUALITY_HIGHEST, str);

		str = headerGetString (h, RPMTAG_SUMMARY);
		if (str && *str)
			gs_app_set_summary (app, GS_APP_QUALITY_HIGHEST, str);

		str = headerGetString (h, RPMTAG_DESCRIPTION);
		if (str && *str)
			gs_app_set_description (app, GS_APP_QUALITY_HIGHEST, str);

		str = headerGetString (h, RPMTAG_NEVRA);
		if (str && *str)
			gs_app_set_metadata (app, "LingmoAppstore::packagename-value", str);
	}
	gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
	app_set_rpm_ostree_packaging_format (app);
	gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
	gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);

	/* add default source */
	name = headerGetString (h, RPMTAG_NAME);
	g_debug ("rpm: setting source to %s", name);
	gs_app_add_source (app, name);

	/* add version */
	epoch = headerGetNumber (h, RPMTAG_EPOCH);
	version = headerGetString (h, RPMTAG_VERSION);
	release = headerGetString (h, RPMTAG_RELEASE);
	if (epoch > 0) {
		evr = g_strdup_printf ("%" G_GUINT64_FORMAT ":%s-%s",
		                       epoch, version, release);
	} else {
		evr = g_strdup_printf ("%s-%s",
		                       version, release);
	}
	g_debug ("rpm: setting version to %s", evr);
	gs_app_set_version (app, evr);

	/* set size */
	size = headerGetNumber (h, RPMTAG_SIZE);
	gs_app_set_size_installed (app, GS_SIZE_TYPE_VALID, size);

	/* set license */
	license = headerGetString (h, RPMTAG_LICENSE);
	if (license != NULL) {
		g_autofree gchar *license_spdx = NULL;
		license_spdx = as_license_to_spdx_id (license);
		gs_app_set_license (app, GS_APP_QUALITY_NORMAL, license_spdx);
		g_debug ("rpm: setting license to %s", license_spdx);
	}

	add_quirks_from_package_name (app, name);

	tmp_list = gs_app_list_new ();
	gs_app_list_add (tmp_list, app);

	if (gs_rpm_ostree_refine_apps (plugin, tmp_list, 0, cancellable, error)) {
		if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN)
			gs_app_set_state (app, GS_APP_STATE_AVAILABLE_LOCAL);

		gs_app_list_add (list, app);
		ret = TRUE;
	}

out:
	if (rpmfd != NULL)
		(void) Fclose (rpmfd);
	return ret;
}

static gchar **
what_provides_decompose (GsAppQueryProvidesType  provides_type,
                         const gchar            *provides_tag)
{
	g_autoptr(GPtrArray) array = g_ptr_array_new ();

	/* The provides_tag possibly already contains the prefix, thus use it as is */
	if (provides_type != GS_APP_QUERY_PROVIDES_UNKNOWN &&
	    g_str_has_suffix (provides_tag, ")") &&
	    strchr (provides_tag, '(') != NULL)
		provides_type = GS_APP_QUERY_PROVIDES_PACKAGE_NAME;

	/* Wrap the @provides_tag with the appropriate Fedora prefix */
	switch (provides_type) {
	case GS_APP_QUERY_PROVIDES_PACKAGE_NAME:
		g_ptr_array_add (array, g_strdup (provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_GSTREAMER:
		g_ptr_array_add (array, g_strdup_printf ("gstreamer0.10(%s)", provides_tag));
		g_ptr_array_add (array, g_strdup_printf ("gstreamer1(%s)", provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_FONT:
		g_ptr_array_add (array, g_strdup_printf ("font(%s)", provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_MIME_HANDLER:
		g_ptr_array_add (array, g_strdup_printf ("mimehandler(%s)", provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_PS_DRIVER:
		g_ptr_array_add (array, g_strdup_printf ("postscriptdriver(%s)", provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_PLASMA:
		g_ptr_array_add (array, g_strdup_printf ("plasma4(%s)", provides_tag));
		g_ptr_array_add (array, g_strdup_printf ("plasma5(%s)", provides_tag));
		break;
	case GS_APP_QUERY_PROVIDES_UNKNOWN:
	default:
		g_assert_not_reached ();
	}

	g_ptr_array_add (array, NULL);

	return (gchar **) g_ptr_array_free (g_steal_pointer (&array), FALSE);
}

static void list_apps_thread_cb (GTask        *task,
                                 gpointer      source_object,
                                 gpointer      task_data,
                                 GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_list_apps_async (GsPlugin              *plugin,
                                      GsAppQuery            *query,
                                      GsPluginListAppsFlags  flags,
                                      GCancellable          *cancellable,
                                      GAsyncReadyCallback    callback,
                                      gpointer               user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

	task = gs_plugin_list_apps_data_new_task (plugin, query, flags,
						  cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_list_apps_async);

	/* Queue a job to get the apps. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				list_apps_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
list_apps_thread_cb (GTask        *task,
                     gpointer      source_object,
                     gpointer      task_data,
                     GCancellable *cancellable)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (source_object);
	g_autoptr(GsAppList) list = gs_app_list_new ();
	GsPluginListAppsData *data = task_data;
	const gchar *provides_tag = NULL;
	GsAppQueryProvidesType provides_type = GS_APP_QUERY_PROVIDES_UNKNOWN;
	g_autoptr(GError) local_error = NULL;
	g_auto(GStrv) provides = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GVariant) packages = NULL;
	gsize n_children;
	gboolean done;
	gboolean interactive = data->flags & GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE;

	assert_in_worker (self);

	if (data->query != NULL) {
		provides_type = gs_app_query_get_provides (data->query, &provides_tag);
	}

	/* Currently only support a subset of query properties, and only one set at once. */
	if (provides_tag == NULL ||
	    gs_app_query_get_n_properties_set (data->query) != 1) {
		g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
					 "Unsupported query");
		return;
	}

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error) ||
	    !gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	provides = what_provides_decompose (provides_type, provides_tag);
	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!gs_rpmostree_os_call_what_provides_sync (os_proxy,
							      (const gchar * const *) provides,
							      interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							      -1  /* timeout */,
							      &packages,
							      cancellable,
							      &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, &local_error)) {
					gs_rpmostree_error_convert (&local_error);
					g_task_return_error (task, g_steal_pointer (&local_error));
					return;
				}
				done = FALSE;
				continue;
			}
			gs_rpmostree_error_convert (&local_error);
			/*  Ignore error when the corresponding D-Bus method does not exist */
			if (g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
				g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
				return;
			}
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}
	n_children = g_variant_n_children (packages);
	for (gsize i = 0; i < n_children; i++) {
		g_autoptr(GVariant) value = g_variant_get_child_value (packages, i);
		g_autoptr(GVariantDict) dict = g_variant_dict_new (value);
		g_autoptr(GsApp) app = NULL;
		const gchar *name = NULL;
		const gchar *nevra = NULL;

		if (!g_variant_dict_lookup (dict, "nevra", "&s", &nevra) ||
		    !g_variant_dict_lookup (dict, "name", "&s", &name))
			continue;

		app = gs_plugin_cache_lookup (GS_PLUGIN (self), nevra);
		if (app != NULL) {
			gs_app_list_add (list, app);
			continue;
		}

		/* create new app */
		app = gs_app_new (NULL);
		gs_app_set_metadata (app, "LingmoAppstore::Creator", gs_plugin_get_name (GS_PLUGIN (self)));
		gs_app_set_management_plugin (app, GS_PLUGIN (self));
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
		app_set_rpm_ostree_packaging_format (app);
		gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
		gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
		gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
		gs_app_add_source (app, name);
		gs_app_set_metadata (app, "LingmoAppstore::packagename-value", nevra);

		gs_plugin_cache_add (GS_PLUGIN (self), nevra, app);
		gs_app_list_add (list, app);
	}

	g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
}

static GsAppList *
gs_plugin_rpm_ostree_list_apps_finish (GsPlugin      *plugin,
                                       GAsyncResult  *result,
                                       GError       **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

gboolean
gs_plugin_add_sources (GsPlugin *plugin,
		       GsAppList *list,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GVariant) repos = NULL;
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GError) local_error = NULL;
	gsize n_children;
	gboolean done;
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, error))
		return FALSE;
	if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error))
		return FALSE;

	done = FALSE;
	while (!done) {
		done = TRUE;
		if (!gs_rpmostree_os_call_list_repos_sync (os_proxy,
							   interactive ? G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION : G_DBUS_CALL_FLAGS_NONE,
							   -1  /* timeout */,
							   &repos,
							   cancellable,
							   &local_error)) {
			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_BUSY)) {
				g_clear_error (&local_error);
				if (!gs_rpmostree_wait_for_ongoing_transaction_end (sysroot_proxy, cancellable, error)) {
					return FALSE;
				}
				done = FALSE;
				continue;
			}
			gs_rpmostree_error_convert (&local_error);
			/*  Ignore error when the corresponding D-Bus method does not exist */
			if (g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED))
				return TRUE;
			g_propagate_error (error, g_steal_pointer (&local_error));
			return FALSE;
		}
	}

	locker = g_mutex_locker_new (&self->cached_sources_mutex);
	if (self->cached_sources == NULL)
		self->cached_sources = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	n_children = g_variant_n_children (repos);
	for (gsize i = 0; i < n_children; i++) {
		g_autoptr(GVariant) value = g_variant_get_child_value (repos, i);
		g_autoptr(GVariantDict) dict = g_variant_dict_new (value);
		g_autoptr(GsApp) app = NULL;
		const gchar *id = NULL;
		const gchar *description = NULL;
		gboolean is_enabled = FALSE;
		gboolean is_devel = FALSE;
		gboolean is_source = FALSE;

		if (!g_variant_dict_lookup (dict, "id", "&s", &id))
			continue;
		if (g_variant_dict_lookup (dict, "is-devel", "b", &is_devel) && is_devel)
			continue;
		/* hide these from the user */
		if (g_variant_dict_lookup (dict, "is-source", "b", &is_source) && is_source)
			continue;
		if (!g_variant_dict_lookup (dict, "description", "&s", &description))
			continue;
		if (!g_variant_dict_lookup (dict, "is-enabled", "b", &is_enabled))
			is_enabled = FALSE;

		app = g_hash_table_lookup (self->cached_sources, id);
		if (app == NULL) {
			app = gs_app_new (id);
			gs_app_set_management_plugin (app, plugin);
			gs_app_set_kind (app, AS_COMPONENT_KIND_REPOSITORY);
			gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
			gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
			gs_app_set_state (app, is_enabled ? GS_APP_STATE_INSTALLED : GS_APP_STATE_AVAILABLE);
			gs_app_set_name (app, GS_APP_QUALITY_LOWEST, description);
			gs_app_set_summary (app, GS_APP_QUALITY_LOWEST, description);
			gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
			gs_app_set_metadata (app, "LingmoAppstore::SortKey", "200");
			gs_app_set_origin_ui (app, _("Operating System (OSTree)"));
		} else {
			g_object_ref (app);
			/* The repo-related apps are those installed; due to re-using
			   cached app, make sure the list is populated from fresh data. */
			gs_app_list_remove_all (gs_app_get_related (app));
		}

		gs_app_list_add (list, app);
	}

	return TRUE;
}

static void enable_repository_thread_cb (GTask        *task,
					 gpointer      source_object,
					 gpointer      task_data,
					 GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_enable_repository_async (GsPlugin                     *plugin,
					      GsApp			   *repository,
                                              GsPluginManageRepositoryFlags flags,
                                              GCancellable	 	   *cancellable,
                                              GAsyncReadyCallback	    callback,
                                              gpointer			    user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE);

	task = gs_plugin_manage_repository_data_new_task (plugin, repository, flags, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_enable_repository_async);

	/* only process this app if it was created by this plugin */
	if (!gs_app_has_management_plugin (repository, plugin)) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	g_assert (gs_app_get_kind (repository) == AS_COMPONENT_KIND_REPOSITORY);

	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				enable_repository_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
enable_repository_thread_cb (GTask        *task,
			     gpointer      source_object,
			     gpointer      task_data,
			     GCancellable *cancellable)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (source_object);
	GsPluginManageRepositoryData *data = task_data;
	GsPluginRefreshMetadataData refresh_data = { 0 };
	gboolean interactive = (data->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE);
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (!gs_rpmostree_repo_enable (GS_PLUGIN (self), data->repository, TRUE, os_proxy, sysroot_proxy, interactive, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	refresh_data.flags = interactive ? GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE : GS_PLUGIN_REFRESH_METADATA_FLAGS_NONE;
	refresh_data.cache_age_secs = 1;

	if (!gs_plugin_rpm_ostree_refresh_metadata_in_worker (self, &refresh_data, os_proxy, sysroot_proxy, cancellable, &local_error))
		g_debug ("Failed to refresh after repository enable: %s", local_error->message);

	/* This can fail silently, it's only to update necessary caches, to provide
	 * up-to-date information after the successful repository enable/install.
	 */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_rpm_ostree_enable_repository_finish (GsPlugin      *plugin,
					       GAsyncResult  *result,
					       GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void disable_repository_thread_cb (GTask        *task,
					  gpointer      source_object,
					  gpointer      task_data,
					  GCancellable *cancellable);

static void
gs_plugin_rpm_ostree_disable_repository_async (GsPlugin                     *plugin,
					       GsApp			    *repository,
                                               GsPluginManageRepositoryFlags flags,
                                               GCancellable	 	    *cancellable,
                                               GAsyncReadyCallback	     callback,
                                               gpointer			     user_data)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE);

	task = gs_plugin_manage_repository_data_new_task (plugin, repository, flags, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_rpm_ostree_disable_repository_async);

	/* only process this app if it was created by this plugin */
	if (!gs_app_has_management_plugin (repository, plugin)) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	g_assert (gs_app_get_kind (repository) == AS_COMPONENT_KIND_REPOSITORY);

	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				disable_repository_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
disable_repository_thread_cb (GTask        *task,
			      gpointer      source_object,
			      gpointer      task_data,
			      GCancellable *cancellable)
{
	GsPluginRpmOstree *self = GS_PLUGIN_RPM_OSTREE (source_object);
	GsPluginManageRepositoryData *data = task_data;
	g_autoptr(GsRPMOSTreeOS) os_proxy = NULL;
	g_autoptr(GsRPMOSTreeSysroot) sysroot_proxy = NULL;
	gboolean interactive = data->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_rpmostree_ref_proxies (self, interactive, &os_proxy, &sysroot_proxy, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (gs_rpmostree_repo_enable (GS_PLUGIN (self), data->repository, FALSE, os_proxy, sysroot_proxy, interactive, cancellable, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, g_steal_pointer (&local_error));
}

static gboolean
gs_plugin_rpm_ostree_disable_repository_finish (GsPlugin      *plugin,
					        GAsyncResult  *result,
					        GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static const gchar *
find_char_on_line (const gchar *txt,
		   gchar chr)
{
	while (*txt != '\n' && *txt != '\0' && *txt != chr)
		txt++;
	return *txt == chr ? txt : NULL;
}

static void
sanitize_update_history_text (gchar *text)
{
	gchar *read_pos = text, *write_pos = text;
	gsize text_len = strlen (text);

	#define skip_after(_chr) G_STMT_START { \
		while (*read_pos != '\0' && *read_pos != '\n' && *read_pos != (_chr)) { \
			if (read_pos != write_pos) \
				*write_pos = *read_pos; \
			read_pos++; \
			write_pos++; \
		} \
		if (*read_pos == (_chr)) { \
			if (read_pos != write_pos) \
				*write_pos = *read_pos; \
			read_pos++; \
			write_pos++; \
		} \
	} G_STMT_END
	#define skip_whitespace() G_STMT_START { \
		while (*read_pos != '\0' && *read_pos != '\n' && g_ascii_isspace (*read_pos)) { \
			if (read_pos != write_pos) \
				*write_pos = *read_pos; \
			read_pos++; \
			write_pos++; \
		} \
	} G_STMT_END

	/* The first two lines begin with "ostree diff commit from/to:" - skip them. */
	if (g_ascii_strncasecmp (read_pos, "ostree diff", strlen ("ostree diff")) == 0)
		skip_after ('\n');
	if (g_ascii_strncasecmp (read_pos, "ostree diff", strlen ("ostree diff")) == 0)
		skip_after ('\n');
	write_pos = text;

	while (*read_pos != '\0') {
		skip_whitespace ();

		/* Hide email addresses */
		if (*read_pos == '*') {
			const gchar *start, *end;

			start = find_char_on_line (read_pos, '<');
			if (start != NULL) {
				end = find_char_on_line (start, '>');
				if (end != NULL) {
					while (read_pos < start) {
						if (read_pos != write_pos)
							*write_pos = *read_pos;
						read_pos++;
						write_pos++;
					}
					read_pos += end - read_pos;
					if (*read_pos == '>' && g_ascii_isspace (read_pos[1]))
						read_pos += 2;
				}
			}
		}

		skip_after ('\n');
	}

	#undef skip_until
	#undef skip_whitespace

	if (read_pos != write_pos)
		*write_pos = '\0';

	/* The logs can have thousands kilobytes of data, which is not good for GtkLabel,
	   which has (together with Pango) a hard time to process it and show it (high CPU
	   use for seconds or even minutes).

	   Cut the log in 4KB, which is not so big and not so small part of the log.
	   This will be extended to parse the output and split the texts by package in the future. */
	if (write_pos - text + strlen ("…") > 4096) {
		write_pos = g_utf8_offset_to_pointer (text, g_utf8_strlen (text, 4096));
		*write_pos = '\0';
		if (write_pos - text + strlen ("…") < text_len - 1)
			strcat (write_pos, "…");
	}
}

gboolean
gs_plugin_add_updates_historical (GsPlugin *plugin,
				  GsAppList *list,
				  GCancellable *cancellable,
				  GError **error)
{
	g_autoptr(GSubprocess) subprocess = NULL;
	g_autofree gchar *stdout_data = NULL;

	subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, error,
				       "rpm-ostree",
				       "db",
				       "diff",
				       "--changelogs",
				       "--format=block",
				       NULL);
	if (subprocess == NULL)
		return FALSE;
	if (!g_subprocess_communicate_utf8 (subprocess, NULL, cancellable, &stdout_data, NULL, error))
		return FALSE;
	if (stdout_data != NULL && *stdout_data != '\0') {
		g_autoptr(GsApp) app = NULL;
		g_autoptr(GIcon) ic = NULL;

		sanitize_update_history_text (stdout_data);

		/* create new */
		app = gs_app_new ("org.lingmo.Appstore.RpmostreeUpdate");
		gs_app_set_management_plugin (app, plugin);
		gs_app_set_state (app, GS_APP_STATE_INSTALLED);
		gs_app_set_name (app,
				 GS_APP_QUALITY_NORMAL,
				 /* TRANSLATORS: this is a group of updates that are not
				  * packages and are not shown in the main list */
				 _("System Updates"));
		gs_app_set_summary (app,
				    GS_APP_QUALITY_NORMAL,
				    /* TRANSLATORS: this is a longer description of the
				     * "System Updates" string */
				    _("General system updates, such as security or bug fixes, and performance improvements."));
		gs_app_set_description (app,
					GS_APP_QUALITY_NORMAL,
					gs_app_get_summary (app));
		gs_app_set_update_details_text (app, stdout_data);
		ic = g_themed_icon_new ("system-component-os-updates");
		gs_app_add_icon (app, ic);

		gs_app_list_add (list, app);

		return TRUE;
	}

	return FALSE;
}

static void
gs_plugin_rpm_ostree_class_init (GsPluginRpmOstreeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_rpm_ostree_dispose;
	object_class->finalize = gs_plugin_rpm_ostree_finalize;

	plugin_class->setup_async = gs_plugin_rpm_ostree_setup_async;
	plugin_class->setup_finish = gs_plugin_rpm_ostree_setup_finish;
	plugin_class->shutdown_async = gs_plugin_rpm_ostree_shutdown_async;
	plugin_class->shutdown_finish = gs_plugin_rpm_ostree_shutdown_finish;
	plugin_class->refine_async = gs_plugin_rpm_ostree_refine_async;
	plugin_class->refine_finish = gs_plugin_rpm_ostree_refine_finish;
	plugin_class->refresh_metadata_async = gs_plugin_rpm_ostree_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_rpm_ostree_refresh_metadata_finish;
	plugin_class->enable_repository_async = gs_plugin_rpm_ostree_enable_repository_async;
	plugin_class->enable_repository_finish = gs_plugin_rpm_ostree_enable_repository_finish;
	plugin_class->disable_repository_async = gs_plugin_rpm_ostree_disable_repository_async;
	plugin_class->disable_repository_finish = gs_plugin_rpm_ostree_disable_repository_finish;
	plugin_class->list_apps_async = gs_plugin_rpm_ostree_list_apps_async;
	plugin_class->list_apps_finish = gs_plugin_rpm_ostree_list_apps_finish;
	plugin_class->update_apps_async = gs_plugin_rpm_ostree_update_apps_async;
	plugin_class->update_apps_finish = gs_plugin_rpm_ostree_update_apps_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_RPM_OSTREE;
}
