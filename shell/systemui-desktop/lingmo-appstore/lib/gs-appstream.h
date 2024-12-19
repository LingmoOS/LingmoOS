/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2015-2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <lingmo-appstore.h>
#include <xmlb.h>

G_BEGIN_DECLS

GsApp		*gs_appstream_create_app		(GsPlugin	*plugin,
							 XbSilo		*silo,
							 XbNode		*component,
							 const gchar	*appstream_source_file,
							 AsComponentScope default_scope,
							 GError		**error);
gboolean	 gs_appstream_refine_app		(GsPlugin	*plugin,
							 GsApp		*app,
							 XbSilo		*silo,
							 XbNode		*component,
							 GsPluginRefineFlags flags,
							 GHashTable	*installed_by_desktopid,
							 const gchar	*appstream_source_file,
							 AsComponentScope default_scope,
							 GError		**error);
gboolean	 gs_appstream_search			(GsPlugin	*plugin,
							 XbSilo		*silo,
							 const gchar * const *values,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_search_developer_apps	(GsPlugin	*plugin,
							 XbSilo		*silo,
							 const gchar * const *values,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_refine_category_sizes	(XbSilo		*silo,
							 GPtrArray	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_category_apps		(GsPlugin	*plugin,
							 XbSilo		*silo,
							 GsCategory	*category,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_installed		(GsPlugin	*plugin,
							 XbSilo		*silo,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_popular		(XbSilo		*silo,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_featured		(XbSilo		*silo,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_deployment_featured	(XbSilo		*silo,
							 const gchar * const *deployments,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_alternates		(XbSilo		*silo,
							 GsApp		*app,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_add_recent		(GsPlugin	*plugin,
							 XbSilo		*silo,
							 GsAppList	*list,
							 guint64	 age,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	gs_appstream_url_to_app			(GsPlugin	*plugin,
							 XbSilo		*silo,
							 GsAppList	*list,
							 const gchar	*url,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_appstream_load_desktop_files	(XbBuilder	*builder,
							 const gchar	*path,
							 gboolean	*out_any_loaded,
							 GFileMonitor  **out_file_monitor,
							 GCancellable	*cancellable,
							 GError		**error);
GPtrArray	*gs_appstream_get_appstream_data_dirs	(void);
void		 gs_appstream_add_current_locales	(XbBuilder	*builder);
void		 gs_appstream_add_data_merge_fixup	(XbBuilder	*builder,
							 GPtrArray	*appstream_paths,
							 GPtrArray	*desktop_paths,
							 GCancellable	*cancellable);
void		 gs_appstream_component_add_extra_info	(XbBuilderNode	*component);
void		 gs_appstream_component_add_keyword	(XbBuilderNode	*component,
							 const gchar	*str);
void		 gs_appstream_component_add_category	(XbBuilderNode	*component,
							 const gchar	*str);
void		 gs_appstream_component_add_icon	(XbBuilderNode	*component,
							 const gchar	*str);
void		 gs_appstream_component_add_provide	(XbBuilderNode	*component,
							 const gchar	*str);
void		 gs_appstream_component_fix_url		(XbBuilderNode  *component,
							 const gchar    *baseurl);

G_END_DECLS
