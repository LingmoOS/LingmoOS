/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>

#include "gs-app.h"

G_BEGIN_DECLS

/**
 * GsUtilsCacheFlags:
 * @GS_UTILS_CACHE_FLAG_NONE:		No flags set
 * @GS_UTILS_CACHE_FLAG_WRITEABLE:	A writable directory is required
 * @GS_UTILS_CACHE_FLAG_USE_HASH:	Prefix a hash to the filename
 * @GS_UTILS_CACHE_FLAG_ENSURE_EMPTY:	Clear existing cached items
 * @GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY:	Create the cache directory (Since: 40)
 *
 * The cache flags.
 **/
typedef enum {
	GS_UTILS_CACHE_FLAG_NONE		= 0,
	GS_UTILS_CACHE_FLAG_WRITEABLE		= 1 << 0,
	GS_UTILS_CACHE_FLAG_USE_HASH		= 1 << 1,
	GS_UTILS_CACHE_FLAG_ENSURE_EMPTY	= 1 << 2,
	GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY	= 1 << 3,
	GS_UTILS_CACHE_FLAG_LAST  /*< skip >*/
} GsUtilsCacheFlags;

guint64		 gs_utils_get_file_age		(GFile		*file);
gchar		*gs_utils_get_content_type	(GFile		*file,
						 GCancellable	*cancellable,
						 GError		**error);
gboolean	 gs_utils_symlink		(const gchar	*target,
						 const gchar	*linkpath,
						 GError		**error);
gboolean	 gs_utils_unlink		(const gchar	*filename,
						 GError		**error);
gboolean	 gs_mkdir_parent		(const gchar	*path,
						 GError		**error);
gchar		*gs_utils_get_cache_filename	(const gchar	*kind,
						 const gchar	*resource,
						 GsUtilsCacheFlags flags,
						 GError		**error);
gchar		*gs_utils_get_user_hash		(GError		**error);
GPermission	*gs_utils_get_permission	(const gchar	*id,
						 GCancellable	*cancellable,
						 GError		**error);
void		 gs_utils_get_permission_async	(const gchar		*id,
						 GCancellable		*cancellable,
						 GAsyncReadyCallback	 callback,
						 gpointer		 user_data);
GPermission	*gs_utils_get_permission_finish	(GAsyncResult	*result,
						 GError		**error);
gboolean	 gs_utils_strv_fnmatch		(gchar		**strv,
						 const gchar	*str);
gchar           *gs_utils_sort_key		(const gchar    *str);
gint             gs_utils_sort_strcmp		(const gchar    *str1,
						 const gchar	*str2);
GDesktopAppInfo *gs_utils_get_desktop_app_info	(const gchar	*id);
gboolean	 gs_utils_rmtree		(const gchar	*directory,
						 GError		**error);
gint		 gs_utils_get_wilson_rating	(guint64	 star1,
						 guint64	 star2,
						 guint64	 star3,
						 guint64	 star4,
						 guint64	 star5);
void		 gs_utils_error_add_app_id	(GError		**error,
						 GsApp		*app);
void		 gs_utils_error_add_origin_id	(GError		**error,
						 GsApp		*origin);
gchar		*gs_utils_error_strip_app_id	(GError		*error);
gchar		*gs_utils_error_strip_origin_id	(GError		*error);
gboolean	 gs_utils_error_convert_gio	(GError		**perror);
gboolean	 gs_utils_error_convert_gresolver (GError	**perror);
gboolean	 gs_utils_error_convert_gdbus	(GError		**perror);
gboolean	 gs_utils_error_convert_gdk_pixbuf(GError	**perror);
gboolean	 gs_utils_error_convert_appstream (GError	**perror);

gchar		*gs_utils_get_url_scheme	(const gchar	*url);
gchar		*gs_utils_get_url_path		(const gchar	*url);
const gchar	*gs_user_agent			(void);
void		 gs_utils_append_key_value	(GString	*str,
						 gsize		 align_len,
						 const gchar	*key,
						 const gchar	*value);
guint		 gs_utils_get_memory_total	(void);
gboolean	 gs_utils_parse_evr		(const gchar	 *evr,
						 gchar		**out_epoch,
						 gchar		**out_version,
						 gchar		**out_release);

gchar		*gs_utils_unique_id_compat_convert	(const gchar	*data_id);

gchar		*gs_utils_build_unique_id	(AsComponentScope scope,
						 AsBundleKind bundle_kind,
						 const gchar *origin,
						 const gchar *cid,
						 const gchar *branch);

void		 gs_utils_pixbuf_blur		(GdkPixbuf	*src,
						 guint		radius,
						 guint		iterations);

/**
 * GsFileSizeIncludeFunc:
 * @filename: file name to check
 * @file_kind: the file kind, one of #GFileTest enums
 * @user_data: a user data passed to the gs_utils_get_file_size()
 *
 * Check whether include the @filename in the size calculation.
 * The @filename is a relative path to the file name passed to
 * the #GsFileSizeIncludeFunc.
 *
 * Returns: Whether to include the @filename in the size calculation
 *
 * Since: 41
 **/
typedef gboolean (*GsFileSizeIncludeFunc)	(const gchar		*filename,
						 GFileTest		 file_kind,
						 gpointer		 user_data);

guint64		 gs_utils_get_file_size		(const gchar		*filename,
						 GsFileSizeIncludeFunc	 include_func,
						 gpointer		 user_data,
						 GCancellable		*cancellable);
gchar *		 gs_utils_get_file_etag		(GFile			*file,
						 GDateTime		**last_modified_date_out,
						 GCancellable		*cancellable);
gboolean	 gs_utils_set_file_etag		(GFile			*file,
						 const gchar		*etag,
						 GCancellable		*cancellable);

gchar		*gs_utils_get_upgrade_background (const gchar		*version);

gint		 gs_utils_app_sort_name		(GsApp			*app1,
						 GsApp			*app2,
						 gpointer		 user_data);
gint		 gs_utils_app_sort_match_value	(GsApp			*app1,
						 GsApp			*app2,
						 gpointer		 user_data);
gint		 gs_utils_app_sort_priority	(GsApp			*app1,
						 GsApp			*app2,
						 gpointer		 user_data);
void		 gs_utils_gstring_replace	(GString		*str,
						 const gchar		*find,
						 const gchar		*replace);

G_END_DECLS
