/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <lingmo-appstore.h>
#include <packagekit-glib2/packagekit.h>

G_BEGIN_DECLS

#define GS_TYPE_PACKAGEKIT_HELPER (gs_packagekit_helper_get_type ())

G_DECLARE_FINAL_TYPE (GsPackagekitHelper, gs_packagekit_helper, GS, PACKAGEKIT_HELPER, GObject)

GsPackagekitHelper *gs_packagekit_helper_new		(GsPlugin		*plugin);
GsPlugin	*gs_packagekit_helper_get_plugin	(GsPackagekitHelper	*self);
void		 gs_packagekit_helper_add_app		(GsPackagekitHelper	*self,
							 GsApp			*app);
void		 gs_packagekit_helper_set_progress_app	(GsPackagekitHelper	*self,
							 GsApp			*progress_app);
void		 gs_packagekit_helper_set_progress_list	(GsPackagekitHelper	*self,
							 GsAppList		*progress_list);
void		 gs_packagekit_helper_set_allow_emit_updates_changed
							(GsPackagekitHelper	*self,
							 gboolean		 allow_emit_updates_changed);
GsApp		*gs_packagekit_helper_get_app_by_id	(GsPackagekitHelper	*self,
							 const gchar		*package_id);
void		 gs_packagekit_helper_cb		(PkProgress		*progress,
							 PkProgressType		 type,
							 gpointer		 user_data);


G_END_DECLS
