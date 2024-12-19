/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>
#include <ostree.h>

#include <glib/gi18n.h>

#include "gs-flatpak-app.h"
#include "gs-flatpak.h"
#include "gs-flatpak-utils.h"

void
gs_flatpak_error_convert (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return;

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gio (perror))
		return;

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gdbus (perror))
		return;

	/* this are allowed for network ops */
	if (gs_utils_error_convert_gresolver (perror))
		return;

	/* custom to this plugin */
	if (error->domain == FLATPAK_ERROR) {
		switch (error->code) {
		case FLATPAK_ERROR_ALREADY_INSTALLED:
		case FLATPAK_ERROR_NOT_INSTALLED:
			error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
			break;
		case FLATPAK_ERROR_OUT_OF_SPACE:
			error->code = GS_PLUGIN_ERROR_NO_SPACE;
			break;
		case FLATPAK_ERROR_INVALID_REF:
		case FLATPAK_ERROR_INVALID_DATA:
			error->code = GS_PLUGIN_ERROR_INVALID_FORMAT;
			break;
		default:
			error->code = GS_PLUGIN_ERROR_FAILED;
			break;
		}
	} else if (error->domain == OSTREE_GPG_ERROR) {
		error->code = GS_PLUGIN_ERROR_NO_SECURITY;
	} else {
		g_warning ("can't reliably fixup error from domain %s: %s",
			   g_quark_to_string (error->domain),
			   error->message);
		error->code = GS_PLUGIN_ERROR_FAILED;
	}
	error->domain = GS_PLUGIN_ERROR;
}

GsApp *
gs_flatpak_app_new_from_remote (GsPlugin *plugin,
				FlatpakRemote *xremote,
				gboolean is_user)
{
	g_autofree gchar *title = NULL;
	g_autofree gchar *url = NULL;
	g_autofree gchar *filter = NULL;
	g_autofree gchar *description = NULL;
	g_autofree gchar *comment = NULL;
	g_autoptr(GsApp) app = NULL;

	app = gs_flatpak_app_new (flatpak_remote_get_name (xremote));
	gs_app_set_kind (app, AS_COMPONENT_KIND_REPOSITORY);
	gs_app_set_state (app, flatpak_remote_get_disabled (xremote) ?
			  GS_APP_STATE_AVAILABLE : GS_APP_STATE_INSTALLED);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST,
			 flatpak_remote_get_name (xremote));
	gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_app_set_management_plugin (app, plugin);
	gs_flatpak_app_set_packaging_info (app);
	gs_app_set_scope (app, is_user ? AS_COMPONENT_SCOPE_USER : AS_COMPONENT_SCOPE_SYSTEM);

	gs_app_set_metadata (app, "LingmoAppstore::SortKey", "100");
	gs_app_set_metadata (app, "LingmoAppstore::InstallationKind",
		is_user ? _("User Installation") : _("System Installation"));
	if (!is_user)
		gs_app_add_quirk (app, GS_APP_QUIRK_PROVENANCE);

	/* title */
	title = flatpak_remote_get_title (xremote);
	if (title != NULL) {
		gs_app_set_summary (app, GS_APP_QUALITY_LOWEST, title);
		gs_app_set_name (app, GS_APP_QUALITY_NORMAL, title);
	}

	/* origin_ui on a remote is the repo dialogue section name,
	 * not the remote title */
	gs_app_set_origin_ui (app, _("Apps"));

	description = flatpak_remote_get_description (xremote);
	if (description != NULL)
		gs_app_set_description (app, GS_APP_QUALITY_NORMAL, description);

	/* url */
	url = flatpak_remote_get_url (xremote);
	if (url != NULL)
		gs_app_set_url (app, AS_URL_KIND_HOMEPAGE, url);

	filter = flatpak_remote_get_filter (xremote);
	if (filter != NULL)
		gs_flatpak_app_set_repo_filter (app, filter);

	comment = flatpak_remote_get_comment (xremote);
	if (comment != NULL)
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, comment);

	/* success */
	return g_steal_pointer (&app);
}

GsApp *
gs_flatpak_app_new_from_repo_file (GFile *file,
				   GCancellable *cancellable,
				   GError **error)
{
	gchar *tmp;
	g_autofree gchar *basename = NULL;
	g_autofree gchar *filename = NULL;
	g_autofree gchar *repo_comment = NULL;
	g_autofree gchar *repo_default_branch = NULL;
	g_autofree gchar *repo_description = NULL;
	g_autofree gchar *repo_gpgkey = NULL;
	g_autofree gchar *repo_homepage = NULL;
	g_autofree gchar *repo_icon = NULL;
	g_autofree gchar *repo_id = NULL;
	g_autofree gchar *repo_title = NULL;
	g_autofree gchar *repo_url = NULL;
	g_autofree gchar *repo_filter = NULL;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GKeyFile) kf = NULL;
	g_autoptr(GsApp) app = NULL;

	/* read the file */
	kf = g_key_file_new ();
	filename = g_file_get_path (file);
	if (!g_key_file_load_from_file (kf, filename,
					G_KEY_FILE_NONE,
					&error_local)) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "failed to load flatpakrepo: %s",
			     error_local->message);
		return NULL;
	}

	/* get the ID from the basename */
	basename = g_file_get_basename (file);
	tmp = g_strrstr (basename, ".");
	if (tmp != NULL)
		*tmp = '\0';

	/* ensure this is valid for flatpak */
	if (ostree_validate_remote_name (basename, NULL)) {
		repo_id = g_steal_pointer (&basename);
	} else {
		repo_id = g_str_to_ascii (basename, NULL);

		for (guint i = 0; repo_id[i] != '\0'; i++) {
			if (!g_ascii_isalnum (repo_id[i]))
				repo_id[i] = '_';
		}
	}

	/* create source */
	repo_title = g_key_file_get_string (kf, "Flatpak Repo", "Title", NULL);
	repo_url = g_key_file_get_string (kf, "Flatpak Repo", "Url", NULL);
	if (repo_title == NULL || repo_url == NULL ||
	    repo_title[0] == '\0' || repo_url[0] == '\0') {
		g_set_error_literal (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "not enough data in file, "
				     "expected at least Title and Url");
		return NULL;
	}

	/* check version */
	if (g_key_file_has_key (kf, "Flatpak Repo", "Version", NULL)) {
		guint64 ver = g_key_file_get_uint64 (kf, "Flatpak Repo", "Version", NULL);
		if (ver != 1) {
			g_set_error (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "unsupported version %" G_GUINT64_FORMAT, ver);
			return NULL;
		}
	}

	/* create source */
	app = gs_flatpak_app_new (repo_id);
	gs_flatpak_app_set_file_kind (app, GS_FLATPAK_APP_FILE_KIND_REPO);
	gs_app_set_kind (app, AS_COMPONENT_KIND_REPOSITORY);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE_LOCAL);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
	gs_app_set_name (app, GS_APP_QUALITY_NORMAL, repo_title);
	gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_flatpak_app_set_repo_url (app, repo_url);
	gs_app_set_origin_ui (app, repo_title);
	gs_app_set_origin_hostname (app, repo_url);

	/* user specified a URL */
	repo_gpgkey = g_key_file_get_string (kf, "Flatpak Repo", "GPGKey", NULL);
	if (repo_gpgkey != NULL) {
		if (g_str_has_prefix (repo_gpgkey, "http://") ||
		    g_str_has_prefix (repo_gpgkey, "https://")) {
			g_set_error_literal (error,
					     GS_PLUGIN_ERROR,
					     GS_PLUGIN_ERROR_NOT_SUPPORTED,
					     "Base64 encoded GPGKey required, not URL");
			return NULL;
		}
		gs_flatpak_app_set_repo_gpgkey (app, repo_gpgkey);
	}

	/* optional data */
	repo_homepage = g_key_file_get_string (kf, "Flatpak Repo", "Homepage", NULL);
	if (repo_homepage != NULL)
		gs_app_set_url (app, AS_URL_KIND_HOMEPAGE, repo_homepage);
	repo_comment = g_key_file_get_string (kf, "Flatpak Repo", "Comment", NULL);
	if (repo_comment != NULL)
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, repo_comment);
	repo_description = g_key_file_get_string (kf, "Flatpak Repo", "Description", NULL);
	if (repo_description != NULL)
		gs_app_set_description (app, GS_APP_QUALITY_NORMAL, repo_description);
	repo_default_branch = g_key_file_get_string (kf, "Flatpak Repo", "DefaultBranch", NULL);
	if (repo_default_branch != NULL)
		gs_app_set_branch (app, repo_default_branch);
	repo_icon = g_key_file_get_string (kf, "Flatpak Repo", "Icon", NULL);
	if (repo_icon != NULL &&
	    (g_str_has_prefix (repo_icon, "http:") ||
	     g_str_has_prefix (repo_icon, "https:"))) {
		g_autoptr(GIcon) icon = gs_remote_icon_new (repo_icon);
		gs_app_add_icon (app, icon);
	}
	repo_filter = g_key_file_get_string (kf, "Flatpak Repo", "Filter", NULL);
	if (repo_filter != NULL && *repo_filter != '\0')
		gs_flatpak_app_set_repo_filter (app, repo_filter);

	/* success */
	return g_steal_pointer (&app);
}

void
gs_flatpak_app_set_packaging_info (GsApp *app)
{
	g_return_if_fail (GS_IS_APP (app));

	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_FLATPAK);
	gs_app_set_metadata (app, "LingmoAppstore::PackagingBaseCssColor", "flatpak_packaging_color");
	gs_app_set_metadata (app, "LingmoAppstore::PackagingIcon", "flatpak-symbolic");
	gs_app_set_metadata (app, "LingmoAppstore::packagename-title", _("App ID"));
}
