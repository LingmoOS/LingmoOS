/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2023 Endless OS Foundation LLC
 *
 * Authors:
 *  - Georges Basile Stavracas Neto <georges@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-icon-downloader
 * @title: Icon downloader
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Utility object to download remote icons
 *
 * #GsIconDownloader is a helper object that is responsible for downloading
 * remote icons of #GsApp instances. Plugins can put apps in the queue to
 * download using gs_icon_downloader_queue_app(). The actual download may
 * happen at any arbitrary time in the future.
 *
 * Since: 44
 */

#include "gs-icon-downloader.h"

#include "gs-app-private.h"
#include "gs-remote-icon.h"
#include "gs-worker-thread.h"

struct _GsIconDownloader
{
	GObject 	 parent_instance;

	guint		 maximum_size_px;
	SoupSession	*soup_session; /* (owned) */

	GsWorkerThread	*worker; /* (owned) */
	GCancellable	*cancellable; /* (owned) */
};

G_DEFINE_FINAL_TYPE (GsIconDownloader, gs_icon_downloader, G_TYPE_OBJECT)

typedef enum {
	PROP_MAXIMUM_SIZE = 1,
	PROP_SOUP_SESSION,
} GsIconDownloaderProperty;

static GParamSpec *properties [PROP_SOUP_SESSION + 1] = { NULL, };

static void
gs_icon_downloader_finalize (GObject *object)
{
	GsIconDownloader *self = (GsIconDownloader *)object;

	g_cancellable_cancel (self->cancellable);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->worker);
	g_clear_object (&self->soup_session);

	G_OBJECT_CLASS (gs_icon_downloader_parent_class)->finalize (object);
}

static void
gs_icon_downloader_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	GsIconDownloader *self = GS_ICON_DOWNLOADER (object);

	switch ((GsIconDownloaderProperty) prop_id) {
	case PROP_MAXIMUM_SIZE:
		g_value_set_uint (value, self->maximum_size_px);
		break;
	case PROP_SOUP_SESSION:
		g_value_set_object (value, self->soup_session);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
gs_icon_downloader_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	GsIconDownloader *self = GS_ICON_DOWNLOADER (object);

	switch ((GsIconDownloaderProperty) prop_id) {
	case PROP_MAXIMUM_SIZE:
		g_assert (self->maximum_size_px == 0);
		self->maximum_size_px = g_value_get_uint (value);
		g_assert (self->maximum_size_px != 0);
		break;
	case PROP_SOUP_SESSION:
		g_assert (self->soup_session == NULL);
		self->soup_session = g_value_dup_object (value);
		g_assert (self->soup_session != NULL);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  }
}

static void
gs_icon_downloader_class_init (GsIconDownloaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gs_icon_downloader_finalize;
	object_class->get_property = gs_icon_downloader_get_property;
	object_class->set_property = gs_icon_downloader_set_property;

	/**
	 * GsIconDownloader:maximum-size:
	 *
	 * The maximum size of the icon, in pixels.
	 *
	 * Since: 44
	 */
	properties[PROP_MAXIMUM_SIZE] =
		g_param_spec_uint ("maximum-size", NULL, NULL,
				   0, G_MAXUINT, 0,
				   G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsIconDownloader:soup-session:
	 *
	 * The #SoupSession to use to download remote icons.
	 *
	 * Since: 44
	 */
	properties[PROP_SOUP_SESSION] =
		g_param_spec_object ("soup-session", NULL, NULL,
				     SOUP_TYPE_SESSION,
				     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (properties), properties);
}

static void
gs_icon_downloader_init (GsIconDownloader *self)
{
	self->worker = gs_worker_thread_new ("gs-icon-downloader");
}

/**
 * gs_icon_downloader_new:
 * @soup_session: a #SoupSession
 * @maximum_size_px: the maximum size of the icons, in pixels
 *
 * Creates a new #GsIconDownloader.
 *
 * Since: 44
 */
GsIconDownloader *
gs_icon_downloader_new (SoupSession *soup_session,
                        guint        maximum_size_px)
{
	return g_object_new (GS_TYPE_ICON_DOWNLOADER,
			     "soup-session", soup_session,
			     "maximum-size", maximum_size_px,
			     NULL);
}


static void download_remote_icons_of_the_app_cb (GTask        *task,
                                                 gpointer      source_object,
                                                 gpointer      task_data,
                                                 GCancellable *cancellable);

static void app_remote_icons_download_finished (GObject      *source_object,
                                                GAsyncResult *result,
                                                gpointer      user_data);

/**
 * gs_icon_downloader_queue_app:
 * @self: a #GsIconDownloader
 * @app: (transfer none): a #GsApp
 * @interactive: whether this icon download was triggered by user action
 *
 * Puts @app in the queue to download icons.
 *
 * Since: 44
 */
void
gs_icon_downloader_queue_app (GsIconDownloader *self,
			      GsApp            *app,
			      gboolean          interactive)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(GPtrArray) icons = NULL;
	gboolean has_remote_icon = FALSE;

	g_return_if_fail (GS_IS_ICON_DOWNLOADER (self));
	g_return_if_fail (GS_IS_APP (app));

	icons = gs_app_dup_icons (app);

	for (guint j = 0; icons && j < icons->len; j++) {
		has_remote_icon |= GS_IS_REMOTE_ICON (g_ptr_array_index (icons, j));
		if (has_remote_icon)
			break;
	}

	/* Nothing to download */
	if (!has_remote_icon) {
		gs_app_set_icons_state (app, GS_APP_ICONS_STATE_AVAILABLE);
		return;
	}

	gs_app_set_icons_state (app, GS_APP_ICONS_STATE_PENDING_DOWNLOAD);

	task = g_task_new (self, self->cancellable, app_remote_icons_download_finished, NULL);
	g_task_set_task_data (task, g_object_ref (app), g_object_unref);
	g_task_set_source_tag (task, gs_icon_downloader_queue_app);

	gs_worker_thread_queue (self->worker, interactive ? G_PRIORITY_DEFAULT : G_PRIORITY_LOW,
				download_remote_icons_of_the_app_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
download_remote_icons_of_the_app_cb (GTask        *task,
                                     gpointer      source_object,
                                     gpointer      task_data,
                                     GCancellable *cancellable)
{
	GsIconDownloader *self = GS_ICON_DOWNLOADER (source_object);
	g_autoptr(GPtrArray) remote_icons = NULL;
	g_autoptr(GPtrArray) icons = NULL;
	GsApp *app;

	g_assert (gs_worker_thread_is_in_worker_context (self->worker));

	app = GS_APP (task_data);
	icons = gs_app_dup_icons (app);
	remote_icons = g_ptr_array_new_full (icons ? icons->len : 0, g_object_unref);

	for (guint j = 0; icons && j < icons->len; j++) {
		GObject *icon = g_ptr_array_index (icons, j);

		if (GS_IS_REMOTE_ICON (icon))
			g_ptr_array_add (remote_icons, g_object_ref (icon));
	}

	g_assert (remote_icons->len > 0);

	g_debug ("Downloading %u icons for app %s", remote_icons->len, gs_app_get_id (app));

	gs_app_set_icons_state (app, GS_APP_ICONS_STATE_DOWNLOADING);

	for (guint j = 0; j < remote_icons->len; j++) {
		GObject *icon = g_ptr_array_index (remote_icons, j);
		g_autoptr(GError) local_error = NULL;

		gs_remote_icon_ensure_cached (GS_REMOTE_ICON (icon),
					      self->soup_session,
					      self->maximum_size_px,
					      cancellable,
					      &local_error);

		if (local_error)
			g_debug ("Error downloading remote icon: %s", local_error->message);

		if (g_task_return_error_if_cancelled (task)) {
			gs_app_set_icons_state (app, GS_APP_ICONS_STATE_AVAILABLE);
			return;
		}
	}

	gs_app_set_icons_state (app, GS_APP_ICONS_STATE_AVAILABLE);

	g_task_return_boolean (task, TRUE);
}

static void
app_remote_icons_download_finished (GObject      *source_object,
                                    GAsyncResult *result,
                                    gpointer      user_data)
{
	g_autoptr(GError) error = NULL;

	g_assert (g_task_is_valid (result, source_object));

	if (!g_task_propagate_boolean (G_TASK (result), &error) &&
	    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		g_warning ("Failed to download icons of one app: %s", error->message);
}

static void shutdown_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data);

/**
 * gs_icon_downloader_shutdown_async:
 * @self: a #GsIconDownloader
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback for once the asynchronous operation is complete
 * @user_data: data to pass to @callback
 *
 * Shut down the icon downloader.
 *
 * This will shut down the internal worker thread that @self uses to
 * queue app downloads.
 *
 * This is a no-op if called subsequently.
 *
 * Since: 44
 */
void
gs_icon_downloader_shutdown_async (GsIconDownloader    *self,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;

	g_return_if_fail (GS_IS_ICON_DOWNLOADER (self));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_icon_downloader_shutdown_async);

	gs_worker_thread_shutdown_async (self->worker, cancellable, shutdown_cb,
					 g_steal_pointer (&task));
}

static void
shutdown_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	g_autoptr(GTask) task = G_TASK (user_data);
	GsIconDownloader *self = g_task_get_source_object (user_data);
	g_autoptr(GError) local_error = NULL;
	gboolean success;

	success = gs_worker_thread_shutdown_finish (self->worker, result, &local_error);

	if (local_error)
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, success);
}

/**
 * gs_icon_downloader_shutdown_finish:
 * @self: a #GsIconDownloader
 * @result: a #GAsyncResult
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous shutdown operation started with
 * gs_icon_downloader_shutdown_async();
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 44
 */
gboolean
gs_icon_downloader_shutdown_finish (GsIconDownloader  *self,
                                    GAsyncResult      *result,
                                    GError           **error)
{
	g_return_val_if_fail (GS_IS_ICON_DOWNLOADER (self), FALSE);
	g_return_val_if_fail (g_async_result_is_tagged (result, gs_icon_downloader_shutdown_async), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}
