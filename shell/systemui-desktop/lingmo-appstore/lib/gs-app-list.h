/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-app.h"

G_BEGIN_DECLS

/**
 * GsAppListFilterFlags:
 * @GS_APP_LIST_FILTER_FLAG_NONE:		No flags set
 * @GS_APP_LIST_FILTER_FLAG_KEY_ID:		Filter by ID
 * @GS_APP_LIST_FILTER_FLAG_KEY_SOURCE:		Filter by default source
 * @GS_APP_LIST_FILTER_FLAG_KEY_VERSION:	Filter by version
 * @GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED:	Prefer installed applications
 * @GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES:	Filter using the provides ID
 *
 * Flags to use when filtering. The priority of each #GsApp is used to choose
 * which application object to keep.
 *
 * Since: 40
 **/
typedef enum {
	GS_APP_LIST_FILTER_FLAG_NONE		= 0,
	GS_APP_LIST_FILTER_FLAG_KEY_ID		= 1 << 0,
	GS_APP_LIST_FILTER_FLAG_KEY_SOURCE	= 1 << 1,
	GS_APP_LIST_FILTER_FLAG_KEY_VERSION	= 1 << 2,
	GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED= 1 << 3,
	GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES	= 1 << 4,
	GS_APP_LIST_FILTER_FLAG_LAST,  /*< skip >*/
	GS_APP_LIST_FILTER_FLAG_MASK		= G_MAXUINT64
} GsAppListFilterFlags;

/* All the properties which use #GsAppListFilterFlags are guint64s. */
G_STATIC_ASSERT (sizeof (GsAppListFilterFlags) == sizeof (guint64));

#define GS_TYPE_APP_LIST (gs_app_list_get_type ())

G_DECLARE_FINAL_TYPE (GsAppList, gs_app_list, GS, APP_LIST, GObject)

/**
 * GsAppListSortFunc:
 * @app1:
 * @app2:
 * @user_data: user data passed into the sort function
 *
 * A version of #GCompareFunc which is specific to #GsApps.
 *
 * Returns: zero if @app1 and @app2 are equal, a negative value if @app1 comes
 *     before @app2, or a positive value if @app1 comes after @app2
 * Since: 41
 */
typedef gint	 (*GsAppListSortFunc)		(GsApp		*app1,
						 GsApp		*app2,
						 gpointer	 user_data);
typedef gboolean (*GsAppListFilterFunc)		(GsApp		*app,
						 gpointer	 user_data);

GsAppList	*gs_app_list_new		(void);
GsAppList	*gs_app_list_copy		(GsAppList	*list);
void		 gs_app_list_add		(GsAppList	*list,
						 GsApp		*app);
void		 gs_app_list_add_list		(GsAppList	*list,
						 GsAppList	*donor);
gboolean	 gs_app_list_remove		(GsAppList	*list,
						 GsApp		*app);
void		 gs_app_list_remove_all		(GsAppList	*list);
GsApp		*gs_app_list_index		(GsAppList	*list,
						 guint		 idx);
GsApp		*gs_app_list_lookup		(GsAppList	*list,
						 const gchar	*unique_id);
guint		 gs_app_list_length		(GsAppList	*list);
void		 gs_app_list_sort		(GsAppList	*list,
						 GsAppListSortFunc func,
						 gpointer	 user_data);
void		 gs_app_list_filter		(GsAppList	*list,
						 GsAppListFilterFunc func,
						 gpointer	 user_data);
void		 gs_app_list_override_progress	(GsAppList	*list,
						 guint		 progress);

G_END_DECLS
