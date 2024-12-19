/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>

#include "gs-flatpak-app.h"

const gchar *
gs_flatpak_app_get_ref_name (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RefName");
}

const gchar *
gs_flatpak_app_get_ref_arch (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RefArch");
}

const gchar *
gs_flatpak_app_get_commit (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::Commit");
}

GsFlatpakAppFileKind
gs_flatpak_app_get_file_kind (GsApp *app)
{
	GVariant *tmp = gs_app_get_metadata_variant (app, "flatpak::FileKind");
	if (tmp == NULL)
		return GS_FLATPAK_APP_FILE_KIND_UNKNOWN;
	return g_variant_get_uint32 (tmp);
}

const gchar *
gs_flatpak_app_get_runtime_url (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RuntimeUrl");
}

FlatpakRefKind
gs_flatpak_app_get_ref_kind (GsApp *app)
{
	GVariant *tmp = gs_app_get_metadata_variant (app, "flatpak::RefKind");
	if (tmp == NULL)
		return FLATPAK_REF_KIND_APP;
	return g_variant_get_uint32 (tmp);
}

const gchar *
gs_flatpak_app_get_ref_kind_as_str (GsApp *app)
{
	FlatpakRefKind ref_kind = gs_flatpak_app_get_ref_kind (app);
	if (ref_kind == FLATPAK_REF_KIND_APP)
		return "app";
	if (ref_kind == FLATPAK_REF_KIND_RUNTIME)
		return "runtime";
	return NULL;
}

const gchar *
gs_flatpak_app_get_object_id (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::ObjectID");
}

const gchar *
gs_flatpak_app_get_repo_gpgkey (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RepoGpgKey");
}

const gchar *
gs_flatpak_app_get_repo_url (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RepoUrl");
}

gchar *
gs_flatpak_app_get_ref_display (GsApp *app)
{
	const gchar *ref_kind_as_str = gs_flatpak_app_get_ref_kind_as_str (app);
	const gchar *ref_name = gs_flatpak_app_get_ref_name (app);
	const gchar *ref_arch = gs_flatpak_app_get_ref_arch (app);
	const gchar *ref_branch = gs_app_get_branch (app);

	g_return_val_if_fail (ref_kind_as_str != NULL, NULL);
	g_return_val_if_fail (ref_name != NULL, NULL);
	g_return_val_if_fail (ref_arch != NULL, NULL);
	g_return_val_if_fail (ref_branch != NULL, NULL);

	return g_strdup_printf ("%s/%s/%s/%s",
	                        ref_kind_as_str,
	                        ref_name,
	                        ref_arch,
	                        ref_branch);
}

void
gs_flatpak_app_set_ref_name (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::RefName", val);
}

void
gs_flatpak_app_set_ref_arch (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::RefArch", val);
}

void
gs_flatpak_app_set_commit (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::Commit", val);
}

void
gs_flatpak_app_set_file_kind (GsApp *app, GsFlatpakAppFileKind file_kind)
{
	g_autoptr(GVariant) tmp = g_variant_new_uint32 (file_kind);
	gs_app_set_metadata_variant (app, "flatpak::FileKind", tmp);
}

void
gs_flatpak_app_set_runtime_url (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::RuntimeUrl", val);
}

void
gs_flatpak_app_set_ref_kind (GsApp *app, FlatpakRefKind ref_kind)
{
	g_autoptr(GVariant) tmp = g_variant_new_uint32 (ref_kind);
	gs_app_set_metadata_variant (app, "flatpak::RefKind", tmp);
}

void
gs_flatpak_app_set_object_id (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::ObjectID", val);
}

void
gs_flatpak_app_set_repo_gpgkey (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::RepoGpgKey", val);
}

void
gs_flatpak_app_set_repo_url (GsApp *app, const gchar *val)
{
	gs_app_set_metadata (app, "flatpak::RepoUrl", val);
}

GsApp *
gs_flatpak_app_new (const gchar *id)
{
	return GS_APP (g_object_new (GS_TYPE_APP, "id", id, NULL));
}

void
gs_flatpak_app_set_main_app_ref_name (GsApp *app, const gchar *main_app_ref)
{
	gs_app_set_metadata (app, "flatpak::mainApp", main_app_ref);
}

const gchar *
gs_flatpak_app_get_main_app_ref_name (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::mainApp");
}

void
gs_flatpak_app_set_repo_filter (GsApp *app, const gchar *filter)
{
	gs_app_set_metadata (app, "flatpak::RepoFilter", filter);
}

const gchar *
gs_flatpak_app_get_repo_filter (GsApp *app)
{
	return gs_app_get_metadata_item (app, "flatpak::RepoFilter");
}
