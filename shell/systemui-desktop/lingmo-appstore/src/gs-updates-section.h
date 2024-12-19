/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "gs-app-list.h"
#include "gs-plugin-loader.h"
#include "gs-page.h"

G_BEGIN_DECLS

#define GS_TYPE_UPDATES_SECTION (gs_updates_section_get_type ())

G_DECLARE_FINAL_TYPE (GsUpdatesSection, gs_updates_section, GS, UPDATES_SECTION, GtkBox)

typedef enum {
	GS_UPDATES_SECTION_KIND_OFFLINE_FIRMWARE,
	GS_UPDATES_SECTION_KIND_OFFLINE,
	GS_UPDATES_SECTION_KIND_ONLINE,
	GS_UPDATES_SECTION_KIND_ONLINE_FIRMWARE,
	GS_UPDATES_SECTION_KIND_LAST
} GsUpdatesSectionKind;

GsUpdatesSection	*gs_updates_section_new			(GsUpdatesSectionKind	 kind,
								 GsPluginLoader		*plugin_loader,
								 GsPage			*page);
GsAppList		*gs_updates_section_get_list		(GsUpdatesSection	*self);
void			 gs_updates_section_add_app		(GsUpdatesSection	*self,
								 GsApp			*app);
void			 gs_updates_section_remove_all		(GsUpdatesSection	*self);
void			 gs_updates_section_set_size_groups	(GsUpdatesSection	*self,
								 GtkSizeGroup		*name,
								 GtkSizeGroup		*button_label,
								 GtkSizeGroup		*button_image,
								 GtkSizeGroup		*header);
gboolean		 gs_updates_section_get_is_narrow	(GsUpdatesSection	*self);
void			 gs_updates_section_set_is_narrow	(GsUpdatesSection	*self,
								 gboolean		 is_narrow);

G_END_DECLS
