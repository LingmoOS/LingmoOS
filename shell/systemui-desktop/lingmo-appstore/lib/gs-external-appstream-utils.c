 /* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Endless Mobile, Inc.
 *
 * Authors: Joaquim Rocha <jrocha@endlessm.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * SECTION:gs-external-appstream-urls
 * @short_description: Provides support for downloading external AppStream files.
 *
 * This downloads the set of configured external AppStream files, and caches
 * them locally.
 *
 * According to the `external-appstream-system-wide` GSetting, the files will
 * either be downloaded to a per-user cache, or to a system-wide cache. In the
 * case of a system-wide cache, they are downloaded to a temporary file writable
 * by the user, and then the suexec binary `lingmo-appstore-install-appstream` is
 * run to copy them to the system location.
 *
 * All the downloads are done in the default #GMainContext for the thread which
 * calls gs_external_appstream_refresh_async(). They are done in parallel and
 * the async refresh function will only complete once the last download is
 * complete.
 *
 * Progress data is reported via a callback, and gives the total progress of all
 * parallel downloads. Internally this is done by updating #ProgressTuple
 * structs as each download progresses. A periodic timeout callback sums these
 * and reports the total progress to the caller. That means that progress
 * reports from gs_external_appstream_refresh_async() are done at a constant
 * frequency.
 *
 * To test this code locally you will probably want to change your GSettings
 * configuration to add some external AppStream URIs:
 * ```
 * gsettings set org.lingmo.appstore external-appstream-urls '["https://example.com/appdata.xml.gz"]'
 * ```
 *
 * When you are done with development, run the following command to use the real
 * external AppStream list again:
 * ```
 * gsettings reset org.lingmo.appstore external-appstream-urls
 * ```
 *
 * Since: 42
 */

#include <errno.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <libsoup/soup.h>

#include "gs-external-appstream-utils.h"

#define APPSTREAM_SYSTEM_DIR LOCALSTATEDIR "/cache/swcatalog/xml"

G_DEFINE_QUARK (gs-external-appstream-error-quark, gs_external_appstream_error)

gchar *
gs_external_appstream_utils_get_file_cache_path (const gchar *file_name)
{
	g_autofree gchar *prefixed_file_name = g_strdup_printf (EXTERNAL_APPSTREAM_PREFIX "-%s",
								file_name);
	return g_build_filename (APPSTREAM_SYSTEM_DIR, prefixed_file_name, NULL);
}

/* To be able to delete old files, when the path changed */
gchar *
gs_external_appstream_utils_get_legacy_file_cache_path (const gchar *file_name)
{
	g_autofree gchar *prefixed_file_name = g_strdup_printf (EXTERNAL_APPSTREAM_PREFIX "-%s",
								file_name);
	return g_build_filename (LOCALSTATEDIR "/cache/app-info/xmls", prefixed_file_name, NULL);
}

const gchar *
gs_external_appstream_utils_get_system_dir (void)
{
	return APPSTREAM_SYSTEM_DIR;
}

static gboolean
gs_external_appstream_check (GFile   *appstream_file,
                             guint64  cache_age_secs)
{
	guint64 appstream_file_age = gs_utils_get_file_age (appstream_file);
	return appstream_file_age >= cache_age_secs;
}

static gboolean
gs_external_appstream_install (const gchar   *appstream_file,
                               GCancellable  *cancellable,
                               GError       **error)
{
	g_autoptr(GSubprocess) subprocess = NULL;
	const gchar *argv[] = { "pkexec",
				LIBEXECDIR "/lingmo-appstore-install-appstream",
				appstream_file, NULL};
	g_debug ("Installing the appstream file %s in the system",
		 appstream_file);
	subprocess = g_subprocess_newv (argv,
					G_SUBPROCESS_FLAGS_STDOUT_PIPE |
					G_SUBPROCESS_FLAGS_STDIN_PIPE, error);
	if (subprocess == NULL)
		return FALSE;
	return g_subprocess_wait_check (subprocess, cancellable, error);
}

static void download_replace_file_cb (GObject      *source_object,
				      GAsyncResult *result,
				      gpointer      user_data);
static void download_stream_cb (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data);

/* A tuple to store the last-received progress data for a single download.
 * Each download (refresh_url_async()) has a pointer to the relevant
 * #ProgressTuple for its download. These are stored in an array in #RefreshData
 * and a timeout callback periodically sums them all and reports progress to the
 * caller. */
typedef struct {
	gsize bytes_downloaded;
	gsize total_download_size;
} ProgressTuple;

typedef struct {
	/* Input data. */
	gchar *url;  /* (not nullable) (owned) */
	GTask *task;  /* (not nullable) (owned) */
	GFile *output_file;  /* (not nullable) (owned) */
	ProgressTuple *progress_tuple;  /* (not nullable) */
	SoupSession *soup_session;  /* (not nullable) (owned) */
	gboolean system_wide;

	/* In-progress data. */
	gchar *last_etag;  /* (nullable) (owned) */
	GDateTime *last_modified_date;  /* (nullable) (owned) */
} DownloadAppStreamData;

static void
download_appstream_data_free (DownloadAppStreamData *data)
{
	g_free (data->url);
	g_clear_object (&data->task);
	g_clear_object (&data->output_file);
	g_clear_object (&data->soup_session);
	g_free (data->last_etag);
	g_clear_pointer (&data->last_modified_date, g_date_time_unref);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DownloadAppStreamData, download_appstream_data_free)

static void
refresh_url_progress_cb (gsize    bytes_downloaded,
                         gsize    total_download_size,
                         gpointer user_data)
{
	ProgressTuple *tuple = user_data;

	tuple->bytes_downloaded = bytes_downloaded;
	tuple->total_download_size = total_download_size;

	/* The timeout callback in progress_cb() periodically sums these. No
	 * need to notify of progress from here. */
}

static void
refresh_url_async (GSettings           *settings,
                   const gchar         *url,
                   SoupSession         *soup_session,
                   guint64              cache_age_secs,
                   ProgressTuple       *progress_tuple,
                   GCancellable        *cancellable,
                   GAsyncReadyCallback  callback,
                   gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autofree gchar *basename = NULL;
	g_autofree gchar *basename_url = g_path_get_basename (url);
	/* make sure different uris with same basenames differ */
	g_autofree gchar *hash = NULL;
	g_autofree gchar *target_file_path = NULL;
	g_autoptr(GFile) target_file = NULL;
	g_autoptr(GFile) tmp_file_parent = NULL;
	g_autoptr(GFile) tmp_file = NULL;
	g_autoptr(GsApp) app_dl = gs_app_new ("external-appstream");
	g_autoptr(GError) local_error = NULL;
	DownloadAppStreamData *data;
	gboolean system_wide;

	task = g_task_new (NULL, cancellable, callback, user_data);
	g_task_set_source_tag (task, refresh_url_async);

	/* Calculate the basename of the target file. */
	hash = g_compute_checksum_for_string (G_CHECKSUM_SHA1, url, -1);
	if (hash == NULL) {
		g_task_return_new_error (task,
					 GS_EXTERNAL_APPSTREAM_ERROR,
					 GS_EXTERNAL_APPSTREAM_ERROR_DOWNLOADING,
					 "Failed to hash URI ‘%s’", url);
		return;
	}
	basename = g_strdup_printf ("%s-%s", hash, basename_url);

	/* Are we downloading for the user, or the system? */
	system_wide = g_settings_get_boolean (settings, "external-appstream-system-wide");

	/* Check cache file age. */
	if (system_wide) {
		target_file_path = gs_external_appstream_utils_get_file_cache_path (basename);
	} else {
		g_autofree gchar *legacy_file_path = NULL;

		target_file_path = g_build_filename (g_get_user_data_dir (),
						     "swcatalog",
						     "xml",
						     basename,
						     NULL);

		/* Delete an old file, from a legacy location */
		legacy_file_path = g_build_filename (g_get_user_data_dir (),
						     "app-info",
						     "xmls",
						     basename,
						     NULL);

		if (g_unlink (legacy_file_path) == -1) {
			int errn = errno;
			if (errn != ENOENT)
				g_debug ("Failed to unlink '%s': %s", legacy_file_path, g_strerror (errn));

		}
	}

	target_file = g_file_new_for_path (target_file_path);

	if (!gs_external_appstream_check (target_file, cache_age_secs)) {
		g_debug ("skipping updating external appstream file %s: "
			 "cache age is older than file",
			 target_file_path);
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* If downloading system wide, write the download contents into a
	 * temporary file that will be copied into the system location later. */
	if (system_wide) {
		g_autofree gchar *tmp_file_path = NULL;

		tmp_file_path = gs_utils_get_cache_filename ("external-appstream",
							     basename,
							     GS_UTILS_CACHE_FLAG_WRITEABLE |
							     GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
							     &local_error);
		if (tmp_file_path == NULL) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		tmp_file = g_file_new_for_path (tmp_file_path);
	} else {
		tmp_file = g_object_ref (target_file);
	}

	gs_app_set_summary_missing (app_dl,
				    /* TRANSLATORS: status text when downloading */
				    _("Downloading extra metadata files…"));

	data = g_new0 (DownloadAppStreamData, 1);
	data->url = g_strdup (url);
	data->task = g_object_ref (task);
	data->output_file = g_object_ref (tmp_file);
	data->progress_tuple = progress_tuple;
	data->soup_session = g_object_ref (soup_session);
	data->system_wide = system_wide;
	g_task_set_task_data (task, data, (GDestroyNotify) download_appstream_data_free);

	/* Create the destination file’s directory.
	 * FIXME: This should be made async; it hasn’t done for now as it’s
	 * likely to be fast. */
	tmp_file_parent = g_file_get_parent (tmp_file);

	if (tmp_file_parent != NULL &&
	    !g_file_make_directory_with_parents (tmp_file_parent, cancellable, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_EXISTS)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_clear_error (&local_error);

	/* Query the ETag and modification date of the target file, if the file already exists. For
	 * system-wide installations, this is the ETag of the AppStream file installed system-wide.
	 * For local installations, this is just the local output file. */
	data->last_etag = gs_utils_get_file_etag (target_file, &data->last_modified_date, cancellable);
	g_debug ("Queried ETag of file %s: %s", g_file_peek_path (target_file), data->last_etag);

	/* Create the output file */
	g_file_replace_async (tmp_file,
			      NULL,  /* ETag */
			      FALSE,  /* make_backup */
			      G_FILE_CREATE_PRIVATE | G_FILE_CREATE_REPLACE_DESTINATION,
			      G_PRIORITY_LOW,
			      cancellable,
			      download_replace_file_cb,
			      g_steal_pointer (&task));
}

static void
download_replace_file_cb (GObject      *source_object,
                          GAsyncResult *result,
                          gpointer      user_data)
{
	GFile *output_file = G_FILE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GCancellable *cancellable = g_task_get_cancellable (task);
	DownloadAppStreamData *data = g_task_get_task_data (task);
	g_autoptr(GFileOutputStream) output_stream = NULL;
	g_autoptr(GError) local_error = NULL;

	output_stream = g_file_replace_finish (output_file, result, &local_error);

	if (output_stream == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Do the download. */
	gs_download_stream_async (data->soup_session,
				  data->url,
				  G_OUTPUT_STREAM (output_stream),
				  data->last_etag,
				  data->last_modified_date,
				  G_PRIORITY_LOW,
				  refresh_url_progress_cb,
				  data->progress_tuple,
				  cancellable,
				  download_stream_cb,
				  g_steal_pointer (&task));
}

static void
download_stream_cb (GObject      *source_object,
                    GAsyncResult *result,
                    gpointer      user_data)
{
	SoupSession *soup_session = SOUP_SESSION (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GCancellable *cancellable = g_task_get_cancellable (task);
	DownloadAppStreamData *data = g_task_get_task_data (task);
	g_autoptr(GError) local_error = NULL;
	g_autofree gchar *new_etag = NULL;

	if (!gs_download_stream_finish (soup_session, result, &new_etag, NULL, &local_error)) {
		if (data->system_wide && g_error_matches (local_error, GS_DOWNLOAD_ERROR, GS_DOWNLOAD_ERROR_NOT_MODIFIED)) {
			g_debug ("External AppStream file not modified, removing temporary download file %s",
				 g_file_peek_path (data->output_file));

			/* System-wide installs should delete the empty file created when preparing to
			 * download the external AppStream file. */
			g_file_delete_async (data->output_file, G_PRIORITY_LOW, NULL, NULL, NULL);
			g_task_return_boolean (task, TRUE);
		} else if (!g_network_monitor_get_network_available (g_network_monitor_get_default ())) {
			g_task_return_new_error (task,
						 GS_EXTERNAL_APPSTREAM_ERROR,
						 GS_EXTERNAL_APPSTREAM_ERROR_NO_NETWORK,
						 "External AppStream could not be downloaded due to being offline");
		} else {
			g_task_return_new_error (task,
						 GS_EXTERNAL_APPSTREAM_ERROR,
						 GS_EXTERNAL_APPSTREAM_ERROR_DOWNLOADING,
						 "Server returned no data for external AppStream file: %s",
						 local_error->message);
		}
		return;
	}

	g_debug ("Downloaded appstream file %s", g_file_peek_path (data->output_file));

	gs_utils_set_file_etag (data->output_file, new_etag, cancellable);

	if (data->system_wide) {
		/* install file systemwide */
		if (!gs_external_appstream_install (g_file_peek_path (data->output_file),
						    cancellable,
						    &local_error)) {
			g_task_return_new_error (task,
						 GS_EXTERNAL_APPSTREAM_ERROR,
						 GS_EXTERNAL_APPSTREAM_ERROR_INSTALLING_ON_SYSTEM,
						 "Error installing external AppStream file on system: %s", local_error->message);
			return;
		}
		g_debug ("Installed appstream file %s", g_file_peek_path (data->output_file));
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
refresh_url_finish (GAsyncResult  *result,
                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void refresh_cb (GObject      *source_object,
                        GAsyncResult *result,
                        gpointer      user_data);
static gboolean progress_cb (gpointer user_data);
static void finish_refresh_op (GTask  *task,
                               GError *error);

typedef struct {
	/* Input data. */
	guint64 cache_age_secs;

	/* In-progress data. */
	guint n_pending_ops;
	GError *error;  /* (nullable) (owned) */
	gsize n_appstream_urls;
	GsDownloadProgressCallback progress_callback;  /* (nullable) */
	gpointer progress_user_data;  /* (closure progress_callback) */
	ProgressTuple *progress_tuples;  /* (array length=n_appstream_urls) (owned) */
	GSource *progress_source;  /* (owned) */
} RefreshData;

static void
refresh_data_free (RefreshData *data)
{
	g_assert (data->n_pending_ops == 0);

	/* If this was set it should have been stolen for g_task_return_error()
	 * by now. */
	g_assert (data->error == NULL);

	/* Similarly, progress reporting should have been stopped by now. */
	g_assert (g_source_is_destroyed (data->progress_source));
	g_source_unref (data->progress_source);

	g_free (data->progress_tuples);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (RefreshData, refresh_data_free)

/**
 * gs_external_appstream_refresh_async:
 * @cache_age_secs: cache age, in seconds, as passed to #GsPluginClass.refresh_metadata_async()
 * @progress_callback: (nullable): callback to call with progress information
 * @progress_user_data: (nullable) (closure progress_callback): data to pass
 *   to @progress_callback
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function call when the asynchronous operation is complete
 * @user_data: data to pass to @callback
 *
 * Refresh any configured external appstream files, if the cache is too old.
 *
 * Since: 42
 */
void
gs_external_appstream_refresh_async (guint64                     cache_age_secs,
                                     GsDownloadProgressCallback  progress_callback,
                                     gpointer                    progress_user_data,
                                     GCancellable               *cancellable,
                                     GAsyncReadyCallback         callback,
                                     gpointer                    user_data)
{
	g_autoptr(GSettings) settings = NULL;
	g_auto(GStrv) appstream_urls = NULL;
	gsize n_appstream_urls;
	g_autoptr(SoupSession) soup_session = NULL;
	g_autoptr(GTask) task = NULL;
	RefreshData *data;
	g_autoptr(RefreshData) data_owned = NULL;

	/* Chosen to allow a few UI updates per second without updating the
	 * progress label so often it’s unreadable. */
	const guint progress_update_period_ms = 300;

	task = g_task_new (NULL, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_external_appstream_refresh_async);

	settings = g_settings_new ("org.lingmo.appstore");
	soup_session = gs_build_soup_session ();
	appstream_urls = g_settings_get_strv (settings,
					      "external-appstream-urls");
	n_appstream_urls = g_strv_length (appstream_urls);

	data = data_owned = g_new0 (RefreshData, 1);
	data->progress_callback = progress_callback;
	data->progress_user_data = progress_user_data;
	data->n_appstream_urls = n_appstream_urls;
	data->progress_tuples = g_new0 (ProgressTuple, n_appstream_urls);
	data->progress_source = g_timeout_source_new (progress_update_period_ms);
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) refresh_data_free);

	/* Set up the progress timeout. This periodically sums up the progress
	 * tuples in `data->progress_tuples` and reports them to the calling
	 * function via @progress_callback, giving an overall progress for all
	 * the parallel operations. */
	g_source_set_callback (data->progress_source, progress_cb, g_object_ref (task), g_object_unref);
	g_source_attach (data->progress_source, g_main_context_get_thread_default ());

	/* Refresh all the URIs in parallel. */
	data->n_pending_ops = 1;

	for (gsize i = 0; i < n_appstream_urls; i++) {
		if (!g_str_has_prefix (appstream_urls[i], "https")) {
			g_warning ("Not considering %s as an external "
				   "appstream source: please use an https URL",
				   appstream_urls[i]);
			continue;
		}

		data->n_pending_ops++;
		refresh_url_async (settings,
				   appstream_urls[i],
				   soup_session,
				   cache_age_secs,
				   &data->progress_tuples[i],
				   cancellable,
				   refresh_cb,
				   g_object_ref (task));
	}

	finish_refresh_op (task, NULL);
}

static void
refresh_cb (GObject      *source_object,
            GAsyncResult *result,
            gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	refresh_url_finish (result, &local_error);
	finish_refresh_op (task, g_steal_pointer (&local_error));
}

static gboolean
progress_cb (gpointer user_data)
{
	GTask *task = G_TASK (user_data);
	RefreshData *data = g_task_get_task_data (task);
	gsize parallel_bytes_downloaded = 0, parallel_total_download_size = 0;

	/* Sum up the progress numerator and denominator for all parallel
	 * downloads. */
	for (gsize i = 0; i < data->n_appstream_urls; i++) {
		const ProgressTuple *progress_tuple = &data->progress_tuples[i];

		if (!g_size_checked_add (&parallel_bytes_downloaded,
					 parallel_bytes_downloaded,
					 progress_tuple->bytes_downloaded))
			parallel_bytes_downloaded = G_MAXSIZE;
		if (!g_size_checked_add (&parallel_total_download_size,
					 parallel_total_download_size,
					 progress_tuple->total_download_size))
			parallel_total_download_size = G_MAXSIZE;
	}

	/* Report progress to the calling function. */
	if (data->progress_callback != NULL)
		data->progress_callback (parallel_bytes_downloaded,
					 parallel_total_download_size,
					 data->progress_user_data);

	return G_SOURCE_CONTINUE;
}

/* @error is (transfer full) if non-%NULL */
static void
finish_refresh_op (GTask  *task,
                   GError *error)
{
	RefreshData *data = g_task_get_task_data (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);

	if (data->error == NULL && error_owned != NULL)
		data->error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while refreshing external appstream: %s", error_owned->message);

	g_assert (data->n_pending_ops > 0);
	data->n_pending_ops--;

	if (data->n_pending_ops > 0)
		return;

	/* Emit one final progress update, then stop any further ones. */
	progress_cb (task);
	g_source_destroy (data->progress_source);

	/* All complete. */
	if (data->error != NULL)
		g_task_return_error (task, g_steal_pointer (&data->error));
	else
		g_task_return_boolean (task, TRUE);
}

/**
 * gs_external_appstream_refresh_finish:
 * @result: a #GAsyncResult
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous refresh operation started with
 * gs_external_appstream_refresh_async().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_external_appstream_refresh_finish (GAsyncResult  *result,
                                      GError       **error)
{
	g_return_val_if_fail (g_task_is_valid (result, NULL), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}
