/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <lingmo-appstore.h>
#include <flatpak.h>

G_BEGIN_DECLS

typedef enum {
	GS_FLATPAK_APP_FILE_KIND_UNKNOWN,
	GS_FLATPAK_APP_FILE_KIND_REPO,
	GS_FLATPAK_APP_FILE_KIND_REF,
	GS_FLATPAK_APP_FILE_KIND_BUNDLE,
	GS_FLATPAK_APP_FILE_KIND_LAST,
} GsFlatpakAppFileKind;

GsApp			*gs_flatpak_app_new			(const gchar	*id);

const gchar		*gs_flatpak_app_get_ref_name		(GsApp		*app);
const gchar		*gs_flatpak_app_get_ref_arch		(GsApp		*app);
FlatpakRefKind		 gs_flatpak_app_get_ref_kind		(GsApp		*app);
const gchar		*gs_flatpak_app_get_ref_kind_as_str	(GsApp		*app);
gchar			*gs_flatpak_app_get_ref_display		(GsApp		*app);

const gchar		*gs_flatpak_app_get_commit		(GsApp		*app);
const gchar		*gs_flatpak_app_get_object_id		(GsApp		*app);
const gchar		*gs_flatpak_app_get_repo_gpgkey		(GsApp		*app);
const gchar		*gs_flatpak_app_get_repo_url		(GsApp		*app);
GsFlatpakAppFileKind	 gs_flatpak_app_get_file_kind		(GsApp		*app);
const gchar		*gs_flatpak_app_get_runtime_url		(GsApp		*app);

void			 gs_flatpak_app_set_ref_name		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_ref_arch		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_ref_kind		(GsApp		*app,
								 FlatpakRefKind	ref_kind);

void			 gs_flatpak_app_set_commit		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_object_id		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_repo_gpgkey		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_repo_url		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_file_kind		(GsApp		*app,
								 GsFlatpakAppFileKind	file_kind);
void			 gs_flatpak_app_set_runtime_url		(GsApp		*app,
								 const gchar	*val);
void			 gs_flatpak_app_set_main_app_ref_name	(GsApp		*app,
								 const gchar	*main_app_ref);
const gchar		*gs_flatpak_app_get_main_app_ref_name	(GsApp		*app);
void			 gs_flatpak_app_set_repo_filter		(GsApp		*app,
								 const gchar	*filter);
const gchar		*gs_flatpak_app_get_repo_filter		(GsApp		*app);

G_END_DECLS
