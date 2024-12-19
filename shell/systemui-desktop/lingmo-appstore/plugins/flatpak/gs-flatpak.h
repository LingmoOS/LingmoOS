/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Joaquim Rocha <jrocha@endlessm.com>
 * Copyright (C) 2016-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <lingmo-appstore.h>
#include <flatpak.h>

G_BEGIN_DECLS

#define GS_TYPE_FLATPAK (gs_flatpak_get_type ())

G_DECLARE_FINAL_TYPE (GsFlatpak, gs_flatpak, GS, FLATPAK, GObject)

typedef enum {
	GS_FLATPAK_FLAG_NONE			= 0,
	GS_FLATPAK_FLAG_IS_TEMPORARY		= 1 << 0,
	GS_FLATPAK_FLAG_LAST  /*< skip >*/
} GsFlatpakFlags;

GsFlatpak	*gs_flatpak_new			(GsPlugin		*plugin,
						 FlatpakInstallation	*installation,
						 GsFlatpakFlags		 flags);
FlatpakInstallation *gs_flatpak_get_installation (GsFlatpak		*self,
						  gboolean		 interactive);

GsApp		*gs_flatpak_ref_to_app		(GsFlatpak		*self,
						 const gchar		*ref,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);

AsComponentScope	gs_flatpak_get_scope		(GsFlatpak		*self);
const gchar	*gs_flatpak_get_id		(GsFlatpak		*self);
gboolean	gs_flatpak_setup		(GsFlatpak		*self,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_installed	(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_sources		(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_updates		(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_refresh		(GsFlatpak		*self,
						 guint64		 cache_age_secs,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_refine_app		(GsFlatpak		*self,
						 GsApp			*app,
						 GsPluginRefineFlags	flags,
						 gboolean		 interactive,
						 gboolean		 force_state_update,
						 GCancellable		*cancellable,
						 GError			**error);
void		gs_flatpak_refine_addons	(GsFlatpak *self,
						 GsApp *parent_app,
						 GsPluginRefineFlags flags,
						 GsAppState state,
						 gboolean interactive,
						 GCancellable *cancellable);
gboolean	gs_flatpak_refine_app_state	(GsFlatpak		*self,
						 GsApp			*app,
						 gboolean		 interactive,
						 gboolean		 force_state_update,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_refine_wildcard	(GsFlatpak		*self,
						 GsApp			*app,
						 GsAppList		*list,
						 GsPluginRefineFlags	 flags,
						 gboolean		 interactive,
						 GHashTable		**inout_components_by_id,
						 GHashTable		**inout_components_by_bundle,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_launch		(GsFlatpak		*self,
						 GsApp			*app,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_app_remove_source	(GsFlatpak		*self,
						 GsApp			*app,
						 gboolean		 is_remove,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_app_install_source	(GsFlatpak		*self,
						 GsApp			*app,
						 gboolean		 is_install,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
GsApp		*gs_flatpak_file_to_app_ref	(GsFlatpak		*self,
						 GFile			*file,
						 gboolean		 unrefined,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
GsApp		*gs_flatpak_file_to_app_bundle	(GsFlatpak		*self,
						 GFile			*file,
						 gboolean		 unrefined,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
GsApp		*gs_flatpak_find_source_by_url	(GsFlatpak		*self,
						 const gchar		*name,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_search		(GsFlatpak		*self,
						 const gchar * const	*values,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_search_developer_apps(GsFlatpak		*self,
						 const gchar * const	*values,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_refine_category_sizes(GsFlatpak		*self,
						 GPtrArray		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_category_apps	(GsFlatpak		*self,
						 GsCategory		*category,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_popular		(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_featured		(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_deployment_featured
						(GsFlatpak		*self,
						 GsAppList		*list,
						 gboolean		 interactive,
						 const gchar *const	*deployments,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_alternates	(GsFlatpak		*self,
						 GsApp			*app,
						 GsAppList		*list,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_add_recent		(GsFlatpak		*self,
						 GsAppList		*list,
						 guint64		 age,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
gboolean	gs_flatpak_url_to_app		(GsFlatpak		*self,
						 GsAppList		*list,
						 const gchar		*url,
						 gboolean		 interactive,
						 GCancellable		*cancellable,
						 GError			**error);
void		gs_flatpak_set_busy		(GsFlatpak		*self,
						 gboolean		 busy);
gboolean	gs_flatpak_get_busy		(GsFlatpak		*self);
gboolean	gs_flatpak_purge_sync		(GsFlatpak              *self,
						 GCancellable           *cancellable,
						 GError                **error);

G_END_DECLS
