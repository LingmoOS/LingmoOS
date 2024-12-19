/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation, Inc
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-remote-icon
 * @short_description: A #GIcon implementation for remote icons
 *
 * #GsRemoteIcon is a #GIcon implementation which represents remote icons —
 * icons which have an HTTP or HTTPS URI. It provides a well-known local filename
 * for a cached copy of the icon, accessible as #GFileIcon:file, and a method
 * to download the icon to the cache, gs_remote_icon_ensure_cached().
 *
 * Constructing a #GsRemoteIcon does not guarantee that the icon is cached. Call
 * gs_remote_icon_ensure_cached() for that.
 *
 * #GsRemoteIcon is immutable after construction and hence is entirely thread
 * safe.
 *
 * FIXME: Currently does no cache invalidation.
 *
 * Since: 40
 */

#include "config.h"

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libsoup/soup.h>

#include "gs-remote-icon.h"
#include "gs-utils.h"

/* FIXME: Work around the fact that GFileIcon is not derivable, by deriving from
 * it anyway by copying its `struct GFileIcon` definition inline here. This will
 * work as long as the size of `struct GFileIcon` doesn’t change within GIO.
 * There’s no way of knowing if that’s the case.
 *
 * See https://github.com/LingmoOS/glib/-/issues/2345 for why this is
 * necessary. */
struct _GsRemoteIcon
{
	/* struct GFileIcon { */
	GObject		 grandparent;
	GFile		*file;
	/* } */

	gchar		*uri;  /* (owned), immutable after construction */
};

G_DEFINE_TYPE (GsRemoteIcon, gs_remote_icon, G_TYPE_FILE_ICON)

typedef enum {
	PROP_URI = 1,
} GsRemoteIconProperty;

static GParamSpec *obj_props[PROP_URI + 1] = { NULL, };

static void
gs_remote_icon_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	GsRemoteIcon *self = GS_REMOTE_ICON (object);

	switch ((GsRemoteIconProperty) prop_id) {
	case PROP_URI:
		g_value_set_string (value, gs_remote_icon_get_uri (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_remote_icon_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	GsRemoteIcon *self = GS_REMOTE_ICON (object);

	switch ((GsRemoteIconProperty) prop_id) {
	case PROP_URI:
		/* Construct only */
		g_assert (self->uri == NULL);
		self->uri = g_value_dup_string (value);
		g_assert (g_str_has_prefix (self->uri, "http:") ||
			  g_str_has_prefix (self->uri, "https:"));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_remote_icon_finalize (GObject *object)
{
	GsRemoteIcon *self = GS_REMOTE_ICON (object);

	g_free (self->uri);

	G_OBJECT_CLASS (gs_remote_icon_parent_class)->finalize (object);
}

static void
gs_remote_icon_class_init (GsRemoteIconClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_remote_icon_get_property;
	object_class->set_property = gs_remote_icon_set_property;
	object_class->finalize = gs_remote_icon_finalize;

	/**
	 * GsRemoteIcon:uri: (not nullable)
	 *
	 * Remote URI of the icon. This must be an HTTP or HTTPS URI; it is a
	 * programmer error to provide other URI schemes.
	 *
	 * Since: 40
	 */
	obj_props[PROP_URI] =
		g_param_spec_string ("uri", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

static void
gs_remote_icon_init (GsRemoteIcon *self)
{
}

/* Use a hash-prefixed filename to avoid cache clashes.
 * This can only fail if @create_directory is %TRUE. */
static gchar *
gs_remote_icon_get_cache_filename (const gchar  *uri,
                                   gboolean      create_directory,
                                   GError      **error)
{
	g_autofree gchar *uri_checksum = NULL;
	g_autofree gchar *uri_basename = NULL;
	g_autofree gchar *cache_basename = NULL;
	GsUtilsCacheFlags flags;

	uri_checksum = g_compute_checksum_for_string (G_CHECKSUM_SHA1,
						      uri,
						      -1);
	uri_basename = g_path_get_basename (uri);

	/* convert filename from jpg to png, as we always convert to PNG on
	 * download */
	if (g_str_has_suffix (uri_basename, ".jpg"))
		memcpy (uri_basename + strlen (uri_basename) - 4, ".png", 4);

	cache_basename = g_strdup_printf ("%s-%s", uri_checksum, uri_basename);

	flags = GS_UTILS_CACHE_FLAG_WRITEABLE;
	if (create_directory)
		flags |= GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY;

	return gs_utils_get_cache_filename ("icons",
					    cache_basename,
					    flags,
					    error);
}

/**
 * gs_remote_icon_new:
 * @uri: remote URI of the icon
 *
 * Create a new #GsRemoteIcon representing @uri. The #GFileIcon:file of the
 * resulting icon will represent the local cache location for the icon.
 *
 * Returns: (transfer full): a new remote icon
 * Since: 40
 */
GIcon *
gs_remote_icon_new (const gchar *uri)
{
	g_autofree gchar *cache_filename = NULL;
	g_autoptr(GFile) file = NULL;

	g_return_val_if_fail (uri != NULL, NULL);

	/* The file is the expected cached location of the icon, once it’s
	 * downloaded. By setting it as the #GFileIcon:file property, existing
	 * code (particularly in GTK) which operates on #GFileIcons will work
	 * transparently with this.
	 *
	 * Ideally, #GFileIcon would be an interface rather than a class, which
	 * would make this implementation cleaner, but this is what we’re stuck
	 * with.
	 *
	 * See https://github.com/LingmoOS/glib/-/issues/2345 */
	cache_filename = gs_remote_icon_get_cache_filename (uri, FALSE, NULL);
	g_assert (cache_filename != NULL);
	file = g_file_new_for_path (cache_filename);

	return g_object_new (GS_TYPE_REMOTE_ICON,
			     "file", file,
			     "uri", uri,
			     NULL);
}

/**
 * gs_remote_icon_get_uri:
 * @self: a #GsRemoteIcon
 *
 * Gets the value of #GsRemoteIcon:uri.
 *
 * Returns: (not nullable): remote URI of the icon
 * Since: 40
 */
const gchar *
gs_remote_icon_get_uri (GsRemoteIcon *self)
{
	g_return_val_if_fail (GS_IS_REMOTE_ICON (self), NULL);

	return self->uri;
}

static GdkPixbuf *
gs_icon_download (SoupSession   *session,
                  const gchar   *uri,
                  const gchar   *destination_path,
                  guint          max_size,
                  GCancellable  *cancellable,
                  GError       **error)
{
	guint status_code;
	g_autoptr(SoupMessage) msg = NULL;
	g_autoptr(GInputStream) stream = NULL;
	g_autoptr(GdkPixbuf) pixbuf = NULL;
	g_autoptr(GdkPixbuf) scaled_pixbuf = NULL;

	/* Create the request */
	msg = soup_message_new (SOUP_METHOD_GET, uri);
	if (msg == NULL) {
		g_set_error_literal (error,
				     G_IO_ERROR,
				     G_IO_ERROR_INVALID_DATA,
				     "Icon has an invalid URL");
		return NULL;
	}

	/* Send request synchronously and start reading the response. */
	stream = soup_session_send (session, msg, cancellable, error);

#if SOUP_CHECK_VERSION(3, 0, 0)
	status_code = soup_message_get_status (msg);
#else
	status_code = msg->status_code;
#endif
	if (stream == NULL) {
		return NULL;
	} else if (status_code != SOUP_STATUS_OK) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_FAILED,
			     "Failed to download icon %s: %s",
			     uri, soup_status_get_phrase (status_code));
		return NULL;
	}

	/* Typically these icons are 64x64px PNG files. If not, resize down
	 * so it’s at most @max_size square, to minimise the size of the on-disk
	 * cache.*/
	pixbuf = gdk_pixbuf_new_from_stream (stream, cancellable, error);
	if (pixbuf == NULL)
		return NULL;

	if ((guint) gdk_pixbuf_get_height (pixbuf) <= max_size &&
	    (guint) gdk_pixbuf_get_width (pixbuf) <= max_size) {
		scaled_pixbuf = g_object_ref (pixbuf);
	} else {
		scaled_pixbuf = gdk_pixbuf_scale_simple (pixbuf, max_size, max_size,
							 GDK_INTERP_BILINEAR);
	}

	/* write file */
	if (!gdk_pixbuf_save (scaled_pixbuf, destination_path, "png", error, NULL))
		return NULL;

	return g_steal_pointer (&scaled_pixbuf);
}

/**
 * gs_remote_icon_ensure_cached:
 * @self: a #GsRemoteIcon
 * @soup_session: a #SoupSession to use to download the icon
 * @maximum_icon_size: maximum size (in device pixels) of the icon to save
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Ensure the given icon is present in the local cache, potentially downloading
 * it from its remote server if needed. This will do network and disk I/O.
 *
 * @maximum_icon_size specifies the maximum size (in device pixels) of the icon
 * which should be saved to the cache. This is the maximum size that the icon
 * can ever be used at, as icons can be downscaled but never upscaled. Typically
 * this will be 160px multiplied by the device scale
 * (`gtk_widget_get_scale_factor()`).
 *
 * This can be called from any thread, as #GsRemoteIcon is immutable and hence
 * thread-safe.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 40
 */
gboolean
gs_remote_icon_ensure_cached (GsRemoteIcon  *self,
                              SoupSession   *soup_session,
                              guint          maximum_icon_size,
                              GCancellable  *cancellable,
                              GError       **error)
{
	const gchar *uri;
	g_autofree gchar *cache_filename = NULL;
	g_autoptr(GdkPixbuf) cached_pixbuf = NULL;
	GStatBuf stat_buf;

	g_return_val_if_fail (GS_IS_REMOTE_ICON (self), FALSE);
	g_return_val_if_fail (SOUP_IS_SESSION (soup_session), FALSE);
	g_return_val_if_fail (maximum_icon_size > 0, FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	uri = gs_remote_icon_get_uri (self);

	/* Work out cache filename. */
	cache_filename = gs_remote_icon_get_cache_filename (uri, TRUE, error);
	if (cache_filename == NULL)
		return FALSE;

	/* Already in cache and not older than 30 days */
	if (g_stat (cache_filename, &stat_buf) != -1 &&
	    S_ISREG (stat_buf.st_mode) &&
	    (g_get_real_time () / G_USEC_PER_SEC) - stat_buf.st_mtim.tv_sec < (60 * 60 * 24 * 30)) {
		gint width = 0, height = 0;
		/* Ensure the downloaded image dimensions are stored on the icon */
		if (!g_object_get_data (G_OBJECT (self), "width") &&
		    gdk_pixbuf_get_file_info (cache_filename, &width, &height)) {
			g_object_set_data (G_OBJECT (self), "width", GINT_TO_POINTER (width));
			g_object_set_data (G_OBJECT (self), "height", GINT_TO_POINTER (height));
		}
		return TRUE;
	}

	cached_pixbuf = gs_icon_download (soup_session, uri, cache_filename, maximum_icon_size, cancellable, error);
	if (cached_pixbuf == NULL)
		return FALSE;

	/* Ensure the dimensions are set correctly on the icon. */
	g_object_set_data (G_OBJECT (self), "width", GUINT_TO_POINTER (gdk_pixbuf_get_width (cached_pixbuf)));
	g_object_set_data (G_OBJECT (self), "height", GUINT_TO_POINTER (gdk_pixbuf_get_height (cached_pixbuf)));

	return TRUE;
}
