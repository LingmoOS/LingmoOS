/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-utils
 * @title: GsUtils
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Utilities that plugins can use
 *
 * These functions provide useful functionality that makes it easy to
 * add new plugin functions.
 */

#include "config.h"

#include <errno.h>
#include <fnmatch.h>
#include <math.h>
#include <string.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>

#if defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef HAVE_POLKIT
#include <polkit/polkit.h>
#endif

#include "gs-app.h"
#include "gs-app-private.h"
#include "gs-utils.h"
#include "gs-plugin.h"

#define MB_IN_BYTES (1024 * 1024)

/**
 * gs_mkdir_parent:
 * @path: A full pathname
 * @error: A #GError, or %NULL
 *
 * Creates any required directories, including any parent directories.
 *
 * Returns: %TRUE for success
 **/
gboolean
gs_mkdir_parent (const gchar *path, GError **error)
{
	g_autofree gchar *parent = NULL;

	parent = g_path_get_dirname (path);
	if (g_mkdir_with_parents (parent, 0755) == -1) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_FAILED,
			     "Failed to create '%s': %s",
			     parent, g_strerror (errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * gs_utils_get_file_age:
 * @file: A #GFile
 *
 * Gets a file age.
 *
 * Returns: The time in seconds since the file was modified, or %G_MAXUINT64 for error
 */
guint64
gs_utils_get_file_age (GFile *file)
{
	guint64 now;
	guint64 mtime;
	g_autoptr(GFileInfo) info = NULL;

	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_TIME_MODIFIED,
				  G_FILE_QUERY_INFO_NONE,
				  NULL,
				  NULL);
	if (info == NULL)
		return G_MAXUINT64;
	mtime = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
	now = (guint64) g_get_real_time () / G_USEC_PER_SEC;
	if (mtime > now)
		return G_MAXUINT64;
	if (now - mtime > G_MAXUINT64)
		return G_MAXUINT64;
	return (guint) (now - mtime);
}

static gchar *
gs_utils_filename_array_return_newest (GPtrArray *array)
{
	const gchar *filename_best = NULL;
	guint age_lowest = G_MAXUINT;
	guint i;
	for (i = 0; i < array->len; i++) {
		const gchar *fn = g_ptr_array_index (array, i);
		g_autoptr(GFile) file = g_file_new_for_path (fn);
		guint64 age_tmp = gs_utils_get_file_age (file);
		if (age_tmp < age_lowest) {
			age_lowest = age_tmp;
			filename_best = fn;
		}
	}
	return g_strdup (filename_best);
}

/**
 * gs_utils_get_cache_filename:
 * @kind: A cache kind, e.g. "fwupd" or "screenshots/123x456"
 * @resource: A resource, e.g. "system.bin" or "http://foo.bar/baz.bin"
 * @flags: Some #GsUtilsCacheFlags, e.g. %GS_UTILS_CACHE_FLAG_WRITEABLE
 * @error: A #GError, or %NULL
 *
 * Returns a filename that points into the cache.
 * This may be per-system or per-user, the latter being more likely
 * when %GS_UTILS_CACHE_FLAG_WRITEABLE is specified in @flags.
 *
 * If %GS_UTILS_CACHE_FLAG_USE_HASH is set in @flags then the returned filename
 * will contain the hashed version of @resource.
 *
 * If there is more than one match, the file that has been modified last is
 * returned.
 *
 * If a plugin requests a file to be saved in the cache it is the plugins
 * responsibility to remove the file when it is no longer valid or is too old
 * -- lingmo-appstore will not ever clean the cache for the plugin.
 * For this reason it is a good idea to use the plugin name as @kind.
 *
 * This function can only fail if %GS_UTILS_CACHE_FLAG_ENSURE_EMPTY or
 * %GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY are passed in @flags.
 *
 * Returns: The full path and filename, which may or may not exist, or %NULL
 **/
gchar *
gs_utils_get_cache_filename (const gchar *kind,
			     const gchar *resource,
			     GsUtilsCacheFlags flags,
			     GError **error)
{
	const gchar *tmp;
	g_autofree gchar *basename = NULL;
	g_autofree gchar *cachedir = NULL;
	g_autoptr(GFile) cachedir_file = NULL;
	g_autoptr(GPtrArray) candidates = g_ptr_array_new_with_free_func (g_free);
	g_autoptr(GError) local_error = NULL;

	/* in the self tests */
	tmp = g_getenv ("GS_SELF_TEST_CACHEDIR");
	if (tmp != NULL) {
		cachedir = g_build_filename (tmp, kind, NULL);
		cachedir_file = g_file_new_for_path (cachedir);

		if ((flags & GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY) &&
		    !g_file_make_directory_with_parents (cachedir_file, NULL, &local_error) &&
		    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_EXISTS)) {
			g_propagate_error (error, g_steal_pointer (&local_error));
			return NULL;
		}

		return g_build_filename (cachedir, resource, NULL);;
	}

	/* get basename */
	if (flags & GS_UTILS_CACHE_FLAG_USE_HASH) {
		g_autofree gchar *basename_tmp = g_path_get_basename (resource);
		g_autofree gchar *hash = g_compute_checksum_for_string (G_CHECKSUM_SHA1,
									resource, -1);
		basename = g_strdup_printf ("%s-%s", hash, basename_tmp);
	} else {
		basename = g_path_get_basename (resource);
	}

	/* not writable, so try the system cache first */
	if (!(flags & GS_UTILS_CACHE_FLAG_WRITEABLE)) {
		g_autofree gchar *cachefn = NULL;
		cachefn = g_build_filename (LOCALSTATEDIR,
					    "cache",
					    "lingmo-appstore",
		                            kind,
					    basename,
					    NULL);
		if (g_file_test (cachefn, G_FILE_TEST_EXISTS)) {
			g_ptr_array_add (candidates,
					 g_steal_pointer (&cachefn));
		}
	}

	/* create the cachedir in a per-release location, creating
	 * if it does not already exist */
	cachedir = g_build_filename (g_get_user_cache_dir (),
				     "lingmo-appstore",
				     kind,
				     NULL);
	cachedir_file = g_file_new_for_path (cachedir);
	if (g_file_query_exists (cachedir_file, NULL) &&
	    flags & GS_UTILS_CACHE_FLAG_ENSURE_EMPTY) {
		if (!gs_utils_rmtree (cachedir, error))
			return NULL;
	}
	if ((flags & GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY) &&
	    !g_file_query_exists (cachedir_file, NULL) &&
	    !g_file_make_directory_with_parents (cachedir_file, NULL, error))
		return NULL;
	g_ptr_array_add (candidates, g_build_filename (cachedir, basename, NULL));

	/* common case: we only have one option */
	if (candidates->len == 1)
		return g_strdup (g_ptr_array_index (candidates, 0));

	/* return the newest (i.e. one with least age) */
	return gs_utils_filename_array_return_newest (candidates);
}

/**
 * gs_utils_get_user_hash:
 * @error: A #GError, or %NULL
 *
 * This SHA1 hash is composed of the contents of machine-id and your
 * username and is also salted with a hardcoded value.
 *
 * This provides an identifier that can be used to identify a specific
 * user on a machine, allowing them to cast only one vote or perform
 * one review on each app.
 *
 * There is no known way to calculate the machine ID or username from
 * the machine hash and there should be no privacy issue.
 *
 * Returns: The user hash, or %NULL on error
 */
gchar *
gs_utils_get_user_hash (GError **error)
{
	g_autofree gchar *data = NULL;
	g_autofree gchar *salted = NULL;

	if (!g_file_get_contents ("/etc/machine-id", &data, NULL, error))
		return NULL;

	salted = g_strdup_printf ("lingmo-appstore[%s:%s]",
				  g_get_user_name (), data);
	return g_compute_checksum_for_string (G_CHECKSUM_SHA1, salted, -1);
}

/**
 * gs_utils_get_permission:
 * @id: A PolicyKit ID, e.g. "org.lingmo.Desktop"
 * @cancellable: A #GCancellable, or %NULL
 * @error: A #GError, or %NULL
 *
 * Gets a permission object for an ID.
 *
 * Returns: a #GPermission, or %NULL if this if not possible.
 **/
GPermission *
gs_utils_get_permission (const gchar *id, GCancellable *cancellable, GError **error)
{
#ifdef HAVE_POLKIT
	g_autoptr(GPermission) permission = NULL;
	permission = polkit_permission_new_sync (id, NULL, cancellable, error);
	if (permission == NULL) {
		g_prefix_error (error, "failed to create permission %s: ", id);
		gs_utils_error_convert_gio (error);
		return NULL;
	}
	return g_steal_pointer (&permission);
#else
	g_set_error (error,
		     GS_PLUGIN_ERROR,
		     GS_PLUGIN_ERROR_NOT_SUPPORTED,
		     "no PolicyKit, so can't return GPermission for %s", id);
	return NULL;
#endif
}

/**
 * gs_utils_get_permission_async:
 * @id: a polkit action ID, for example `org.freedesktop.packagekit.trigger-offline-update`
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback for when the asynchronous operation is complete
 * @user_data: data to pass to @callback
 *
 * Asynchronously gets a #GPermission object representing the given polkit
 * action @id.
 *
 * Since: 42
 */
void
gs_utils_get_permission_async (const gchar         *id,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
	g_return_if_fail (id != NULL);
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

#ifdef HAVE_POLKIT
	polkit_permission_new (id, NULL, cancellable, callback, user_data);
#else
	g_task_report_new_error (NULL, callback, user_data, gs_utils_get_permission_async,
				 GS_PLUGIN_ERROR,
				 GS_PLUGIN_ERROR_NOT_SUPPORTED,
				 "no PolicyKit, so can't return GPermission for %s", id);
#endif
}

/**
 * gs_utils_get_permission_finish:
 * @result: result of the asynchronous operation
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous operation started with gs_utils_get_permission_async().
 *
 * Returns: (transfer full): a #GPermission representing the given action ID
 * Since: 42
 */
GPermission *
gs_utils_get_permission_finish (GAsyncResult  *result,
                                GError       **error)
{
	g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

#ifdef HAVE_POLKIT
	return polkit_permission_new_finish (result, error);
#else
	return g_task_propagate_pointer (G_TASK (result), error);
#endif
}

/**
 * gs_utils_get_content_type:
 * @file: A GFile
 * @cancellable: A #GCancellable, or %NULL
 * @error: A #GError, or %NULL
 *
 * Gets the standard content type for a file.
 *
 * Returns: the content type, or %NULL, e.g. "text/plain"
 */
gchar *
gs_utils_get_content_type (GFile *file,
			   GCancellable *cancellable,
			   GError **error)
{
	const gchar *tmp;
	g_autoptr(GFileInfo) info = NULL;

	/* get content type */
	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
				  G_FILE_QUERY_INFO_NONE,
				  cancellable,
				  error);
	if (info == NULL)
		return NULL;
	tmp = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	if (tmp == NULL)
		return NULL;
	return g_strdup (tmp);
}

/**
 * gs_utils_strv_fnmatch:
 * @strv: A NUL-terminated list of strings
 * @str: A string
 *
 * Matches a string against a list of globs.
 *
 * Returns: %TRUE if the list matches
 */
gboolean
gs_utils_strv_fnmatch (gchar **strv, const gchar *str)
{
	guint i;

	/* empty */
	if (strv == NULL)
		return FALSE;

	/* look at each one */
	for (i = 0; strv[i] != NULL; i++) {
		if (fnmatch (strv[i], str, 0) == 0)
			return TRUE;
	}
	return FALSE;
}

/**
 * gs_utils_sort_key:
 * @str: A string to convert to a sort key
 *
 * Useful to sort strings in a locale-sensitive, presentational way.
 * Case is ignored and utf8 collation is used (e.g. accents are ignored).
 *
 * Returns: a newly allocated string sort key
 */
gchar *
gs_utils_sort_key (const gchar *str)
{
	g_autofree gchar *casefolded = g_utf8_casefold (str, -1);
	return g_utf8_collate_key (casefolded, -1);
}

/**
 * gs_utils_sort_strcmp:
 * @str1: (nullable): A string to compare
 * @str2: (nullable): A string to compare
 *
 * Compares two strings in a locale-sensitive, presentational way.
 * Case is ignored and utf8 collation is used (e.g. accents are ignored). %NULL
 * is sorted before all non-%NULL strings, and %NULLs compare equal.
 *
 * Returns: < 0 if str1 is before str2, 0 if equal, > 0 if str1 is after str2
 */
gint
gs_utils_sort_strcmp (const gchar *str1, const gchar *str2)
{
	g_autofree gchar *key1 = (str1 != NULL) ? gs_utils_sort_key (str1) : NULL;
	g_autofree gchar *key2 = (str2 != NULL) ? gs_utils_sort_key (str2) : NULL;
	return g_strcmp0 (key1, key2);
}

/**
 * gs_utils_get_desktop_app_info:
 * @id: A desktop ID, e.g. "gimp.desktop"
 *
 * Gets a a #GDesktopAppInfo taking into account the kde4- prefix.
 * If the given @id doesn not have a ".desktop" suffix, it will add one to it
 * for convenience.
 *
 * Returns: a #GDesktopAppInfo for a specific ID, or %NULL
 */
GDesktopAppInfo *
gs_utils_get_desktop_app_info (const gchar *id)
{
	GDesktopAppInfo *app_info;
	g_autofree gchar *desktop_id = NULL;

	/* for convenience, if the given id doesn't have the required .desktop
	 * suffix, we add it here */
	if (!g_str_has_suffix (id, ".desktop")) {
		desktop_id = g_strconcat (id, ".desktop", NULL);
		id = desktop_id;
	}

	/* try to get the standard app-id */
	app_info = g_desktop_app_info_new (id);

	/* KDE is a special project because it believes /usr/share/applications
	 * isn't KDE enough. For this reason we support falling back to the
	 * "kde4-" prefixed ID to avoid educating various self-righteous
	 * upstreams about the correct ID to use in the AppData file. */
	if (app_info == NULL) {
		g_autofree gchar *kde_id = NULL;
		kde_id = g_strdup_printf ("%s-%s", "kde4", id);
		app_info = g_desktop_app_info_new (kde_id);
	}

	return app_info;
}

/**
 * gs_utils_symlink:
 * @target: the full path of the symlink to create
 * @linkpath: where the symlink should point to
 * @error: A #GError, or %NULL
 *
 * Creates a symlink that can cross filesystem boundaries.
 * Any parent directories needed for target to exist are also created.
 *
 * Returns: %TRUE for success
 **/
gboolean
gs_utils_symlink (const gchar *target, const gchar *linkpath, GError **error)
{
	if (!gs_mkdir_parent (target, error))
		return FALSE;
	if (symlink (target, linkpath) != 0) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_WRITE_FAILED,
			     "failed to create symlink from %s to %s",
			     linkpath, target);
		return FALSE;
	}
	return TRUE;
}

/**
 * gs_utils_unlink:
 * @filename: A full pathname to delete
 * @error: A #GError, or %NULL
 *
 * Deletes a file from disk.
 *
 * Returns: %TRUE for success
 **/
gboolean
gs_utils_unlink (const gchar *filename, GError **error)
{
	if (g_unlink (filename) != 0) {
		gint err = errno;
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_DELETE_FAILED,
			     _("Failed to delete file “%s”: %s"), filename, g_strerror (err));
		return FALSE;
	}
	return TRUE;
}

static gboolean
gs_utils_rmtree_real (const gchar *directory, GError **error)
{
	const gchar *filename;
	g_autoptr(GDir) dir = NULL;

	/* try to open */
	dir = g_dir_open (directory, 0, error);
	if (dir == NULL)
		return FALSE;

	/* find each */
	while ((filename = g_dir_read_name (dir))) {
		g_autofree gchar *src = NULL;
		src = g_build_filename (directory, filename, NULL);
		if (g_file_test (src, G_FILE_TEST_IS_DIR) &&
		    !g_file_test (src, G_FILE_TEST_IS_SYMLINK)) {
			if (!gs_utils_rmtree_real (src, error))
				return FALSE;
		} else {
			if (g_unlink (src) != 0) {
				gint err = errno;
				g_set_error (error,
					     GS_PLUGIN_ERROR,
					     GS_PLUGIN_ERROR_DELETE_FAILED,
					     _("Failed to delete file “%s”: %s"), src, g_strerror (err));
				return FALSE;
			}
		}
	}

	if (g_rmdir (directory) != 0) {
		gint err = errno;
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_DELETE_FAILED,
			     _("Failed to delete directory “%s”: %s"), directory, g_strerror (err));
		return FALSE;
	}
	return TRUE;
}

/**
 * gs_utils_rmtree:
 * @directory: A full directory pathname to delete
 * @error: A #GError, or %NULL
 *
 * Deletes a directory from disk and all its contents.
 *
 * Returns: %TRUE for success
 **/
gboolean
gs_utils_rmtree (const gchar *directory, GError **error)
{
	g_debug ("recursively removing directory '%s'", directory);
	return gs_utils_rmtree_real (directory, error);
}

static gdouble
pnormaldist (gdouble qn)
{
	static gdouble b[11] = { 1.570796288,      0.03706987906,   -0.8364353589e-3,
				-0.2250947176e-3,  0.6841218299e-5,  0.5824238515e-5,
				-0.104527497e-5,   0.8360937017e-7, -0.3231081277e-8,
				 0.3657763036e-10, 0.6936233982e-12 };
	gdouble w1, w3;
	guint i;

	if (qn < 0 || qn > 1)
		return 0; // This is an error case
	if (qn == 0.5)
		return 0;

	w1 = qn;
	if (qn > 0.5)
		w1 = 1.0 - w1;
	w3 = -log (4.0 * w1 * (1.0 - w1));
	w1 = b[0];
	for (i = 1; i < 11; i++)
		w1 = w1 + (b[i] * pow (w3, i));

	if (qn > 0.5)
		return sqrt (w1 * w3);
	else
		return -sqrt (w1 * w3);
}

static gdouble
wilson_score (gdouble value, gdouble n, gdouble power)
{
	gdouble z, phat;
	if (value == 0)
		return 0;
	z = pnormaldist (1 - power / 2);
	phat = value / n;
	return (phat + z * z / (2 * n) -
		z * sqrt ((phat * (1 - phat) + z * z / (4 * n)) / n)) /
		(1 + z * z / n);
}

/**
 * gs_utils_get_wilson_rating:
 * @star1: The number of 1 star reviews
 * @star2: The number of 2 star reviews
 * @star3: The number of 3 star reviews
 * @star4: The number of 4 star reviews
 * @star5: The number of 5 star reviews
 *
 * Returns the lower bound of Wilson score confidence interval for a
 * Bernoulli parameter. This ensures small numbers of ratings don't give overly
 * high scores.
 * See https://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval
 * for details.
 *
 * Returns: Wilson rating percentage, or -1 for error
 **/
gint
gs_utils_get_wilson_rating (guint64 star1,
			    guint64 star2,
			    guint64 star3,
			    guint64 star4,
			    guint64 star5)
{
	gdouble val;
	guint64 star_sum = star1 + star2 + star3 + star4 + star5;
	if (star_sum == 0)
		return -1;

	/* get score */
	val =  (wilson_score ((gdouble) star1, (gdouble) star_sum, 0.2) * -2);
	val += (wilson_score ((gdouble) star2, (gdouble) star_sum, 0.2) * -1);
	val += (wilson_score ((gdouble) star4, (gdouble) star_sum, 0.2) * 1);
	val += (wilson_score ((gdouble) star5, (gdouble) star_sum, 0.2) * 2);

	/* normalize from -2..+2 to 0..5 */
	val += 3;

	/* multiply to a percentage */
	val *= 20;

	/* return rounded up integer */
	return (gint) ceil (val);
}

/**
 * gs_utils_error_add_app_id:
 * @error: a #GError
 * @app: a #GsApp
 *
 * Adds app unique ID prefix to the error.
 *
 * Since: 3.30
 **/
void
gs_utils_error_add_app_id (GError **error, GsApp *app)
{
	g_return_if_fail (GS_APP (app));
	if (error == NULL || *error == NULL)
		return;
	g_prefix_error (error, "{%s} ", gs_app_get_unique_id (app));
}

/**
 * gs_utils_error_add_origin_id:
 * @error: a #GError
 * @origin: a #GsApp
 *
 * Adds origin unique ID prefix to the error.
 *
 * Since: 3.30
 **/
void
gs_utils_error_add_origin_id (GError **error, GsApp *origin)
{
	g_return_if_fail (GS_APP (origin));
	if (error == NULL || *error == NULL)
		return;
	g_prefix_error (error, "[%s] ", gs_app_get_unique_id (origin));
}

/**
 * gs_utils_error_strip_app_id:
 * @error: a #GError
 *
 * Removes a possible app ID prefix from the error, and returns the removed
 * app ID.
 *
 * Returns: A newly allocated string with the app ID
 *
 * Since: 3.30
 **/
gchar *
gs_utils_error_strip_app_id (GError *error)
{
	g_autofree gchar *app_id = NULL;
	g_autofree gchar *msg = NULL;

	if (error == NULL || error->message == NULL)
		return FALSE;

	if (g_str_has_prefix (error->message, "{")) {
		const gchar *endp = strstr (error->message + 1, "} ");
		if (endp != NULL) {
			app_id = g_strndup (error->message + 1,
			                    endp - (error->message + 1));
			msg = g_strdup (endp + 2);
		}
	}

	if (msg != NULL) {
		g_free (error->message);
		error->message = g_steal_pointer (&msg);
	}

	return g_steal_pointer (&app_id);
}

/**
 * gs_utils_error_strip_origin_id:
 * @error: a #GError
 *
 * Removes a possible origin ID prefix from the error, and returns the removed
 * origin ID.
 *
 * Returns: A newly allocated string with the origin ID
 *
 * Since: 3.30
 **/
gchar *
gs_utils_error_strip_origin_id (GError *error)
{
	g_autofree gchar *origin_id = NULL;
	g_autofree gchar *msg = NULL;

	if (error == NULL || error->message == NULL)
		return FALSE;

	if (g_str_has_prefix (error->message, "[")) {
		const gchar *endp = strstr (error->message + 1, "] ");
		if (endp != NULL) {
			origin_id = g_strndup (error->message + 1,
			                       endp - (error->message + 1));
			msg = g_strdup (endp + 2);
		}
	}

	if (msg != NULL) {
		g_free (error->message);
		error->message = g_steal_pointer (&msg);
	}

	return g_steal_pointer (&origin_id);
}

/**
 * gs_utils_error_convert_gdbus:
 * @perror: a pointer to a #GError, or %NULL
 *
 * Converts the #GDBusError to an error with a GsPluginError domain.
 *
 * Returns: %TRUE if the error was converted, or already correct
 **/
gboolean
gs_utils_error_convert_gdbus (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return FALSE;
	if (error->domain == GS_PLUGIN_ERROR)
		return TRUE;
	if (error->domain != G_DBUS_ERROR)
		return FALSE;
	switch (error->code) {
	case G_DBUS_ERROR_FAILED:
	case G_DBUS_ERROR_NO_REPLY:
	case G_DBUS_ERROR_TIMEOUT:
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	case G_DBUS_ERROR_IO_ERROR:
	case G_DBUS_ERROR_NAME_HAS_NO_OWNER:
	case G_DBUS_ERROR_NOT_SUPPORTED:
	case G_DBUS_ERROR_SERVICE_UNKNOWN:
	case G_DBUS_ERROR_UNKNOWN_INTERFACE:
	case G_DBUS_ERROR_UNKNOWN_METHOD:
	case G_DBUS_ERROR_UNKNOWN_OBJECT:
	case G_DBUS_ERROR_UNKNOWN_PROPERTY:
		error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
		break;
	case G_DBUS_ERROR_NO_MEMORY:
		error->code = GS_PLUGIN_ERROR_NO_SPACE;
		break;
	case G_DBUS_ERROR_ACCESS_DENIED:
	case G_DBUS_ERROR_AUTH_FAILED:
		error->code = GS_PLUGIN_ERROR_NO_SECURITY;
		break;
	case G_DBUS_ERROR_NO_NETWORK:
		error->code = GS_PLUGIN_ERROR_NO_NETWORK;
		break;
	case G_DBUS_ERROR_INVALID_FILE_CONTENT:
		error->code = GS_PLUGIN_ERROR_INVALID_FORMAT;
		break;
	default:
		g_warning ("can't reliably fixup error code %i in domain %s: %s",
			   error->code, g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	}
	error->domain = GS_PLUGIN_ERROR;
	return TRUE;
}

/**
 * gs_utils_error_convert_gio:
 * @perror: a pointer to a #GError, or %NULL
 *
 * Converts the #GIOError to an error with a GsPluginError domain.
 *
 * Returns: %TRUE if the error was converted, or already correct
 **/
gboolean
gs_utils_error_convert_gio (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return FALSE;
	if (error->domain == GS_PLUGIN_ERROR)
		return TRUE;
	if (error->domain != G_IO_ERROR)
		return FALSE;
	switch (error->code) {
	case G_IO_ERROR_FAILED:
	case G_IO_ERROR_NOT_FOUND:
	case G_IO_ERROR_EXISTS:
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	case G_IO_ERROR_TIMED_OUT:
		error->code = GS_PLUGIN_ERROR_TIMED_OUT;
		break;
	case G_IO_ERROR_NOT_SUPPORTED:
		error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
		break;
	case G_IO_ERROR_CANCELLED:
		error->code = GS_PLUGIN_ERROR_CANCELLED;
		break;
	case G_IO_ERROR_NO_SPACE:
		error->code = GS_PLUGIN_ERROR_NO_SPACE;
		break;
	case G_IO_ERROR_PERMISSION_DENIED:
		error->code = GS_PLUGIN_ERROR_NO_SECURITY;
		break;
	case G_IO_ERROR_HOST_NOT_FOUND:
	case G_IO_ERROR_HOST_UNREACHABLE:
	case G_IO_ERROR_CONNECTION_REFUSED:
	case G_IO_ERROR_PROXY_FAILED:
	case G_IO_ERROR_PROXY_AUTH_FAILED:
	case G_IO_ERROR_PROXY_NOT_ALLOWED:
		error->code = GS_PLUGIN_ERROR_DOWNLOAD_FAILED;
		break;
	case G_IO_ERROR_NETWORK_UNREACHABLE:
		error->code = GS_PLUGIN_ERROR_NO_NETWORK;
		break;
	default:
		g_warning ("can't reliably fixup error code %i in domain %s: %s",
			   error->code, g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	}
	error->domain = GS_PLUGIN_ERROR;
	return TRUE;
}

/**
 * gs_utils_error_convert_gresolver:
 * @perror: a pointer to a #GError, or %NULL
 *
 * Converts the #GResolverError to an error with a GsPluginError domain.
 *
 * Returns: %TRUE if the error was converted, or already correct
 **/
gboolean
gs_utils_error_convert_gresolver (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return FALSE;
	if (error->domain == GS_PLUGIN_ERROR)
		return TRUE;
	if (error->domain != G_RESOLVER_ERROR)
		return FALSE;
	switch (error->code) {
	case G_RESOLVER_ERROR_INTERNAL:
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	case G_RESOLVER_ERROR_NOT_FOUND:
	case G_RESOLVER_ERROR_TEMPORARY_FAILURE:
		error->code = GS_PLUGIN_ERROR_DOWNLOAD_FAILED;
		break;
	default:
		g_warning ("can't reliably fixup error code %i in domain %s: %s",
			   error->code, g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	}
	error->domain = GS_PLUGIN_ERROR;
	return TRUE;
}

/**
 * gs_utils_error_convert_gdk_pixbuf:
 * @perror: a pointer to a #GError, or %NULL
 *
 * Converts the #GdkPixbufError to an error with a GsPluginError domain.
 *
 * Returns: %TRUE if the error was converted, or already correct
 **/
gboolean
gs_utils_error_convert_gdk_pixbuf (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return FALSE;
	if (error->domain == GS_PLUGIN_ERROR)
		return TRUE;
	if (error->domain != GDK_PIXBUF_ERROR)
		return FALSE;
	switch (error->code) {
	case GDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION:
	case GDK_PIXBUF_ERROR_UNKNOWN_TYPE:
		error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
		break;
	case GDK_PIXBUF_ERROR_FAILED:
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	case GDK_PIXBUF_ERROR_CORRUPT_IMAGE:
		error->code = GS_PLUGIN_ERROR_INVALID_FORMAT;
		break;
	default:
		g_warning ("can't reliably fixup error code %i in domain %s: %s",
			   error->code, g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
		break;
	}
	error->domain = GS_PLUGIN_ERROR;
	return TRUE;
}

/**
 * gs_utils_error_convert_appstream:
 * @perror: a pointer to a #GError, or %NULL
 *
 * Converts the various AppStream error types to an error with a GsPluginError
 * domain.
 *
 * Returns: %TRUE if the error was converted, or already correct
 **/
gboolean
gs_utils_error_convert_appstream (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return FALSE;
	if (error->domain == GS_PLUGIN_ERROR)
		return TRUE;

	/* custom to this plugin */
	if (error->domain == AS_METADATA_ERROR) {
		switch (error->code) {
		case AS_METADATA_ERROR_PARSE:
		case AS_METADATA_ERROR_FORMAT_UNEXPECTED:
		case AS_METADATA_ERROR_NO_COMPONENT:
			error->code = GS_PLUGIN_ERROR_INVALID_FORMAT;
			break;
		case AS_METADATA_ERROR_FAILED:
		default:
			error->code = GS_PLUGIN_ERROR_FAILED;
			break;
		}
	} else if (error->domain == AS_POOL_ERROR) {
		switch (error->code) {
		case AS_POOL_ERROR_FAILED:
		default:
			error->code = GS_PLUGIN_ERROR_FAILED;
			break;
		}
	} else if (error->domain == G_FILE_ERROR) {
		switch (error->code) {
		case G_FILE_ERROR_EXIST:
		case G_FILE_ERROR_ACCES:
		case G_FILE_ERROR_PERM:
			error->code = GS_PLUGIN_ERROR_NO_SECURITY;
			break;
		case G_FILE_ERROR_NOSPC:
			error->code = GS_PLUGIN_ERROR_NO_SPACE;
			break;
		default:
			error->code = GS_PLUGIN_ERROR_FAILED;
			break;
		}
	} else {
		g_warning ("can't reliably fixup error code %i in domain %s: %s",
			   error->code, g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
	}
	error->domain = GS_PLUGIN_ERROR;
	return TRUE;
}

/**
 * gs_utils_get_url_scheme:
 * @url: A URL, e.g. "appstream://gimp.desktop"
 *
 * Gets the scheme from the URL string.
 *
 * Returns: the URL scheme, e.g. "appstream"
 */
gchar *
gs_utils_get_url_scheme	(const gchar *url)
{
	g_autoptr(GUri) uri = NULL;

	/* no data */
	if (url == NULL)
		return NULL;

	/* create URI from URL */
	uri = g_uri_parse (url, SOUP_HTTP_URI_FLAGS, NULL);
	if (!uri)
		return NULL;

	/* success */
	return g_strdup (g_uri_get_scheme (uri));
}

/**
 * gs_utils_get_url_path:
 * @url: A URL, e.g. "appstream://gimp.desktop"
 *
 * Gets the path from the URL string, removing any leading slashes.
 *
 * Returns: the URL path, e.g. "gimp.desktop"
 */
gchar *
gs_utils_get_url_path (const gchar *url)
{
	g_autoptr(GUri) uri = NULL;
	const gchar *host;
	const gchar *path;

	uri = g_uri_parse (url, SOUP_HTTP_URI_FLAGS, NULL);
	if (!uri)
		return NULL;

	/* foo://bar -> scheme: foo, host: bar, path: / */
	/* foo:bar -> scheme: foo, host: (empty string), path: /bar */
	host = g_uri_get_host (uri);
	path = g_uri_get_path (uri);
	if (host != NULL && *host != '\0')
		path = host;

	/* trim any leading slashes */
	while (*path == '/')
		path++;

	/* success */
	return g_strdup (path);
}

/**
 * gs_user_agent:
 *
 * Gets the user agent to use for remote requests.
 *
 * Returns: the user-agent, e.g. "lingmo-appstore/3.22.1"
 */
const gchar *
gs_user_agent (void)
{
	return PACKAGE_NAME "/" PACKAGE_VERSION;
}

/**
 * gs_utils_append_key_value:
 * @str: A #GString
 * @align_len: The alignment of the @value compared to the @key
 * @key: The text to use as a title
 * @value: The text to use as a value
 *
 * Adds a line to an existing string, padding the key to a set number of spaces.
 *
 * Since: 3.26
 */
void
gs_utils_append_key_value (GString *str, gsize align_len,
			   const gchar *key, const gchar *value)
{
	gsize len = 0;

	g_return_if_fail (str != NULL);
	g_return_if_fail (value != NULL);

	if (key != NULL) {
		len = strlen (key) + 2;
		g_string_append (str, key);
		g_string_append (str, ": ");
	}
	for (gsize i = len; i < align_len + 1; i++)
		g_string_append (str, " ");
	g_string_append (str, value);
	g_string_append (str, "\n");
}

guint
gs_utils_get_memory_total (void)
{
#if defined(__linux__)
	struct sysinfo si = { 0 };
	sysinfo (&si);
	if (si.mem_unit > 0)
		return si.totalram / MB_IN_BYTES / si.mem_unit;
	return 0;
#elif defined(__FreeBSD__)
	unsigned long physmem;
	sysctl ((int[]){ CTL_HW, HW_PHYSMEM }, 2, &physmem, &(size_t){ sizeof (physmem) }, NULL, 0);
	return physmem / MB_IN_BYTES;
#else
#error "Please implement gs_utils_get_memory_total for your system."
#endif
}

/**
 * gs_utils_unique_id_compat_convert:
 * @data_id: (nullable): A string that may be a unique component ID
 *
 * Converts the unique ID string from its legacy 6-part form into
 * a new-style 5-part AppStream data-id.
 * Does nothing if the string is already valid.
 *
 * See !583 for the history of this conversion.
 *
 * Returns: (nullable): A newly allocated string with the new-style data-id, or %NULL if input was no valid ID.
 *
 * Since: 40
 **/
gchar*
gs_utils_unique_id_compat_convert (const gchar *data_id)
{
	g_auto(GStrv) parts = NULL;
	if (data_id == NULL)
		return NULL;

	/* check for the most common case first: data-id is already valid */
	if (as_utils_data_id_valid (data_id))
		return g_strdup (data_id);

	parts = g_strsplit (data_id, "/", -1);
	if (g_strv_length (parts) != 6)
		return NULL;
	return g_strdup_printf ("%s/%s/%s/%s/%s",
				parts[0],
				parts[1],
				parts[2],
				parts[4],
				parts[5]);
}

static const gchar *
_fix_data_id_part (const gchar *value)
{
	if (!value || !*value)
		return "*";

	return value;
}

/**
 * gs_utils_build_unique_id:
 * @scope: Scope of the metadata as #AsComponentScope e.g. %AS_COMPONENT_SCOPE_SYSTEM
 * @bundle_kind: Bundling system providing this data, e.g. 'package' or 'flatpak'
 * @origin: Origin string, e.g. 'os' or 'lingmo-apps-nightly'
 * @cid: AppStream component ID, e.g. 'org.freedesktop.appstream.cli'
 * @branch: Branch, e.g. '3-20' or 'master'
 *
 * Builds an identifier string unique to the individual dataset using the supplied information.
 * It's similar to as_utils_build_data_id(), except it respects the @origin for the packages.
 *
 * Returns: (transfer full): a unique ID, free with g_free(), when no longer needed.
 *
 * Since: 41
 */
gchar *
gs_utils_build_unique_id (AsComponentScope scope,
			  AsBundleKind bundle_kind,
			  const gchar *origin,
			  const gchar *cid,
			  const gchar *branch)
{
	const gchar *scope_str = NULL;
	const gchar *bundle_str = NULL;

	if (scope != AS_COMPONENT_SCOPE_UNKNOWN)
		scope_str = as_component_scope_to_string (scope);
	if (bundle_kind != AS_BUNDLE_KIND_UNKNOWN)
		bundle_str = as_bundle_kind_to_string (bundle_kind);

	return g_strdup_printf ("%s/%s/%s/%s/%s",
				_fix_data_id_part (scope_str),
				_fix_data_id_part (bundle_str),
				_fix_data_id_part (origin),
				_fix_data_id_part (cid),
				_fix_data_id_part (branch));
}

static void
gs_pixbuf_blur_private (GdkPixbuf *src, GdkPixbuf *dest, guint radius, guint8 *div_kernel_size)
{
	gint width, height, src_rowstride, dest_rowstride, n_channels;
	guchar *p_src, *p_dest, *c1, *c2;
	gint x, y, i, i1, i2, width_minus_1, height_minus_1, radius_plus_1;
	gint r, g, b;
	guchar *p_dest_row, *p_dest_col;

	width = gdk_pixbuf_get_width (src);
	height = gdk_pixbuf_get_height (src);
	n_channels = gdk_pixbuf_get_n_channels (src);
	radius_plus_1 = radius + 1;

	/* horizontal blur */
	p_src = gdk_pixbuf_get_pixels (src);
	p_dest = gdk_pixbuf_get_pixels (dest);
	src_rowstride = gdk_pixbuf_get_rowstride (src);
	dest_rowstride = gdk_pixbuf_get_rowstride (dest);
	width_minus_1 = width - 1;
	for (y = 0; y < height; y++) {

		/* calc the initial sums of the kernel */
		r = g = b = 0;
		for (i = -radius; i <= (gint) radius; i++) {
			c1 = p_src + (CLAMP (i, 0, width_minus_1) * n_channels);
			r += c1[0];
			g += c1[1];
			b += c1[2];
		}

		p_dest_row = p_dest;
		for (x = 0; x < width; x++) {
			/* set as the mean of the kernel */
			p_dest_row[0] = div_kernel_size[r];
			p_dest_row[1] = div_kernel_size[g];
			p_dest_row[2] = div_kernel_size[b];
			p_dest_row += n_channels;

			/* the pixel to add to the kernel */
			i1 = x + radius_plus_1;
			if (i1 > width_minus_1)
				i1 = width_minus_1;
			c1 = p_src + (i1 * n_channels);

			/* the pixel to remove from the kernel */
			i2 = x - radius;
			if (i2 < 0)
				i2 = 0;
			c2 = p_src + (i2 * n_channels);

			/* calc the new sums of the kernel */
			r += c1[0] - c2[0];
			g += c1[1] - c2[1];
			b += c1[2] - c2[2];
		}

		p_src += src_rowstride;
		p_dest += dest_rowstride;
	}

	/* vertical blur */
	p_src = gdk_pixbuf_get_pixels (dest);
	p_dest = gdk_pixbuf_get_pixels (src);
	src_rowstride = gdk_pixbuf_get_rowstride (dest);
	dest_rowstride = gdk_pixbuf_get_rowstride (src);
	height_minus_1 = height - 1;
	for (x = 0; x < width; x++) {

		/* calc the initial sums of the kernel */
		r = g = b = 0;
		for (i = -radius; i <= (gint) radius; i++) {
			c1 = p_src + (CLAMP (i, 0, height_minus_1) * src_rowstride);
			r += c1[0];
			g += c1[1];
			b += c1[2];
		}

		p_dest_col = p_dest;
		for (y = 0; y < height; y++) {
			/* set as the mean of the kernel */

			p_dest_col[0] = div_kernel_size[r];
			p_dest_col[1] = div_kernel_size[g];
			p_dest_col[2] = div_kernel_size[b];
			p_dest_col += dest_rowstride;

			/* the pixel to add to the kernel */
			i1 = y + radius_plus_1;
			if (i1 > height_minus_1)
				i1 = height_minus_1;
			c1 = p_src + (i1 * src_rowstride);

			/* the pixel to remove from the kernel */
			i2 = y - radius;
			if (i2 < 0)
				i2 = 0;
			c2 = p_src + (i2 * src_rowstride);

			/* calc the new sums of the kernel */
			r += c1[0] - c2[0];
			g += c1[1] - c2[1];
			b += c1[2] - c2[2];
		}

		p_src += n_channels;
		p_dest += n_channels;
	}
}

/**
 * gs_utils_pixbuf_blur:
 * @src: the GdkPixbuf.
 * @radius: the pixel radius for the gaussian blur, typical values are 1..3
 * @iterations: Amount to blur the image, typical values are 1..5
 *
 * Blurs an image. Warning, this method is s..l..o..w... for large images.
 **/
void
gs_utils_pixbuf_blur (GdkPixbuf *src, guint radius, guint iterations)
{
	gint kernel_size;
	gint i;
	g_autofree guchar *div_kernel_size = NULL;
	g_autoptr(GdkPixbuf) tmp = NULL;

	tmp = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (src),
			      gdk_pixbuf_get_has_alpha (src),
			      gdk_pixbuf_get_bits_per_sample (src),
			      gdk_pixbuf_get_width (src),
			      gdk_pixbuf_get_height (src));
	kernel_size = 2 * radius + 1;
	div_kernel_size = g_new (guchar, 256 * kernel_size);
	for (i = 0; i < 256 * kernel_size; i++)
		div_kernel_size[i] = (guchar) (i / kernel_size);

	while (iterations-- > 0)
		gs_pixbuf_blur_private (src, tmp, radius, div_kernel_size);
}

/**
 * gs_utils_get_file_size:
 * @filename: a file name to get the size of; it can be a file or a directory
 * @include_func: (nullable) (scope call): optional callback to limit what files to count
 * @user_data: user data passed to the @include_func
 * @cancellable: (nullable): an optional #GCancellable or %NULL
 *
 * Gets the size of the file or a directory identified by @filename.
 *
 * When the @include_func is not %NULL, it can limit which files are included
 * in the resulting size. When it's %NULL, all files and subdirectories are included.
 *
 * Returns: disk size of the @filename; or 0 when not found
 *
 * Since: 41
 **/
guint64
gs_utils_get_file_size (const gchar *filename,
			GsFileSizeIncludeFunc include_func,
			gpointer user_data,
			GCancellable *cancellable)
{
	guint64 size = 0;

	g_return_val_if_fail (filename != NULL, 0);

	if (g_file_test (filename, G_FILE_TEST_IS_DIR)) {
		GSList *dirs_to_do = NULL;
		gsize base_len = strlen (filename);

		/* The `include_func()` expects a path relative to the `filename`, without
		   a leading dir separator. As the `dirs_to_do` contains the full path,
		   constructed with `g_build_filename()`, the added dir separator needs
		   to be skipped, when it's not part of the `filename` already. */
		if (!g_str_has_suffix (filename, G_DIR_SEPARATOR_S))
			base_len++;

		dirs_to_do = g_slist_prepend (dirs_to_do, g_strdup (filename));
		while (dirs_to_do != NULL && !g_cancellable_is_cancelled (cancellable)) {
			g_autofree gchar *path = NULL;
			g_autoptr(GDir) dir = NULL;

			/* Steal the top `path` out of the `dirs_to_do`. */
			path = dirs_to_do->data;
			dirs_to_do = g_slist_remove (dirs_to_do, path);

			dir = g_dir_open (path, 0, NULL);
			if (dir) {
				const gchar *name;
				while (name = g_dir_read_name (dir), name != NULL && !g_cancellable_is_cancelled (cancellable)) {
					g_autofree gchar *full_path = g_build_filename (path, name, NULL);
					GStatBuf st;

					if (g_stat (full_path, &st) == 0 && (include_func == NULL ||
					    include_func (full_path + base_len,
							  g_file_test (full_path, G_FILE_TEST_IS_SYMLINK) ? G_FILE_TEST_IS_SYMLINK :
							  S_ISDIR (st.st_mode) ? G_FILE_TEST_IS_DIR :
							  G_FILE_TEST_IS_REGULAR,
							  user_data))) {
						if (S_ISDIR (st.st_mode)) {
							/* Skip symlinks, they can point to a shared storage */
							if (!g_file_test (full_path, G_FILE_TEST_IS_SYMLINK))
								dirs_to_do = g_slist_prepend (dirs_to_do, g_steal_pointer (&full_path));
						} else {
							size += st.st_size;
						}
					}
				}
			}
		}
		g_slist_free_full (dirs_to_do, g_free);
	} else {
		GStatBuf st;

		if (g_stat (filename, &st) == 0)
			size = st.st_size;
	}

	return size;
}

#define METADATA_ETAG_ATTRIBUTE "xattr::lingmo-appstore::etag"

/**
 * gs_utils_get_file_etag:
 * @file: a file to get the ETag for
 * @last_modified_date_out: (out callee-allocates) (transfer full) (optional) (nullable):
 *   return location for the last modified date of the file (%NULL to ignore),
 *   or %NULL if unknown
 * @cancellable: (nullable): an optional #GCancellable or %NULL
 *
 * Gets the ETag for the @file, previously stored by
 * gs_utils_set_file_etag().
 *
 * Returns: (nullable) (transfer full): The ETag stored for the @file,
 *    or %NULL, when the file does not exist, no ETag is stored for it
 *    or other error occurs.
 *
 * Since: 43
 **/
gchar *
gs_utils_get_file_etag (GFile         *file,
                        GDateTime    **last_modified_date_out,
                        GCancellable  *cancellable)
{
	g_autoptr(GFileInfo) info = NULL;
	const gchar *attributes;
	g_autoptr(GError) local_error = NULL;

	g_return_val_if_fail (G_IS_FILE (file), NULL);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), NULL);

	if (last_modified_date_out == NULL)
		attributes = METADATA_ETAG_ATTRIBUTE;
	else
		attributes = METADATA_ETAG_ATTRIBUTE "," G_FILE_ATTRIBUTE_TIME_MODIFIED;

	info = g_file_query_info (file, attributes, G_FILE_QUERY_INFO_NONE, cancellable, &local_error);

	if (info == NULL) {
		g_debug ("Error getting attribute ‘%s’ for file ‘%s’: %s",
			 METADATA_ETAG_ATTRIBUTE, g_file_peek_path (file), local_error->message);

		if (last_modified_date_out != NULL)
			*last_modified_date_out = NULL;

		return NULL;
	}

	if (last_modified_date_out != NULL)
		*last_modified_date_out = g_file_info_get_modification_date_time (info);

	return g_strdup (g_file_info_get_attribute_string (info, METADATA_ETAG_ATTRIBUTE));
}

/**
 * gs_utils_set_file_etag:
 * @file: a file to get the ETag for
 * @etag: (nullable): an ETag to set
 * @cancellable: (nullable): an optional #GCancellable or %NULL
 *
 * Sets the ETag for the @file. When the @etag is %NULL or an empty
 * string, then unsets the ETag for the @file. The ETag can be read
 * back with gs_utils_get_file_etag().
 *
 * The @file should exist, otherwise the function fails.
 *
 * Returns: whether succeeded.
 *
 * Since: 42
 **/
gboolean
gs_utils_set_file_etag (GFile        *file,
                        const gchar  *etag,
                        GCancellable *cancellable)
{
	g_autoptr(GError) local_error = NULL;

	g_return_val_if_fail (G_IS_FILE (file), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);

	if (etag == NULL || *etag == '\0') {
		if (!g_file_set_attribute (file, METADATA_ETAG_ATTRIBUTE, G_FILE_ATTRIBUTE_TYPE_INVALID,
					   NULL, G_FILE_QUERY_INFO_NONE, cancellable, &local_error)) {
			g_debug ("Error clearing attribute ‘%s’ on file ‘%s’: %s",
				 METADATA_ETAG_ATTRIBUTE, g_file_peek_path (file), local_error->message);
			return FALSE;
		}

		return TRUE;
	}

	if (!g_file_set_attribute_string (file, METADATA_ETAG_ATTRIBUTE, etag, G_FILE_QUERY_INFO_NONE, cancellable, &local_error)) {
		g_debug ("Error setting attribute ‘%s’ to ‘%s’ on file ‘%s’: %s",
			 METADATA_ETAG_ATTRIBUTE, etag, g_file_peek_path (file), local_error->message);
		return FALSE;
	}

	return TRUE;
}

/**
 * gs_utils_get_upgrade_background:
 * @version: (nullable): version string of the upgrade (which must be non-empty
 *   if provided), or %NULL if unknown
 *
 * Get the path to a background image to display as the background for a banner
 * advertising an upgrade to the given @version.
 *
 * If a path is returned, it’s guaranteed to exist on the file system.
 *
 * Vendors can drop their customised backgrounds in this directory for them to
 * be used by lingmo-appstore. See `doc/vendor-customisation.md`.
 *
 * Returns: (transfer full) (type filename) (nullable): path to an upgrade
 *   background image to use, or %NULL if a suitable one didn’t exist
 * Since: 42
*/
gchar *
gs_utils_get_upgrade_background (const gchar *version)
{
	g_autofree gchar *filename = NULL;
	g_autofree gchar *os_id = g_get_os_info (G_OS_INFO_KEY_ID);

	g_return_val_if_fail (version == NULL || *version != '\0', NULL);

	if (version != NULL) {
		filename = g_strdup_printf (DATADIR "/lingmo-appstore/backgrounds/%s-%s.png", os_id, version);
		if (g_file_test (filename, G_FILE_TEST_EXISTS))
			return g_steal_pointer (&filename);
		g_clear_pointer (&filename, g_free);
	}

	filename = g_strdup_printf (DATADIR "/lingmo-appstore/backgrounds/%s.png", os_id);
	if (g_file_test (filename, G_FILE_TEST_EXISTS))
		return g_steal_pointer (&filename);
	g_clear_pointer (&filename, g_free);

	return NULL;
}

/**
 * gs_utils_app_sort_name:
 * @app1: a #GsApp
 * @app2: another #GsApp
 * @user_data: data passed to the sort function
 *
 * Comparison function to sort apps in increasing alphabetical order of name.
 *
 * This is suitable for passing to gs_app_list_sort().
 *
 * Returns: a strcmp()-style sort value comparing @app1 to @app2
 * Since: 43
 */
gint
gs_utils_app_sort_name (GsApp    *app1,
                        GsApp    *app2,
                        gpointer  user_data)
{
	return gs_utils_sort_strcmp (gs_app_get_name (app1), gs_app_get_name (app2));
}

/**
 * gs_utils_app_sort_match_value:
 * @app1: a #GsApp
 * @app2: another #GsApp
 * @user_data: data passed to the sort function
 *
 * Comparison function to sort apps in decreasing order of match value
 * (#GsApp:match-value).
 *
 * This is suitable for passing to gs_app_list_sort().
 *
 * Returns: a strcmp()-style sort value comparing @app1 to @app2
 * Since: 43
 */
gint
gs_utils_app_sort_match_value (GsApp    *app1,
                               GsApp    *app2,
                               gpointer  user_data)
{
	return gs_app_get_match_value (app2) - gs_app_get_match_value (app1);
}

/**
 * gs_utils_app_sort_priority:
 * @app1: a #GsApp
 * @app2: another #GsApp
 * @user_data: data passed to the sort function
 *
 * Comparison function to sort apps in increasing order of their priority
 * (#GsApp:priority).
 *
 * This is suitable for passing to gs_app_list_sort().
 *
 * Returns: a strcmp()-style sort value comparing @app1 to @app2
 * Since: 43
 */
gint
gs_utils_app_sort_priority (GsApp    *app1,
                            GsApp    *app2,
                            gpointer  user_data)
{
	return gs_app_compare_priority (app1, app2);
}

/**
 * gs_utils_gstring_replace:
 * @str: a #GString to replace the text in
 * @find: a text to find
 * @replace: a text to replace the found text with
 *
 * Replaces all @find occurrences in @str with @replace.
 *
 * Since: 45
 **/
void
gs_utils_gstring_replace (GString *str,
			  const gchar *find,
			  const gchar *replace)
{
#if AS_CHECK_VERSION(1, 0, 0)
	as_gstring_replace (str, find, replace, 0);
#else
	as_gstring_replace2 (str, find, replace, 0);
#endif
}
